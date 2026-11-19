// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_walls.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <queue>
#include <utility>
#include <stdexcept>

// Original499a60 pipeline; see local rms-wall117-native-prototype findings (2026-11-16).
namespace swgb {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
struct Point { int x, y; };
struct Node { int cell; float cost, priority; uint32_t order; };
struct Later {
    bool operator()(const Node& a, const Node& b) const {
        return a.priority != b.priority ? a.priority > b.priority : a.order < b.order;
    }
};
constexpr std::array<Point, 4> cardinal{{{-1,0},{1,0},{0,-1},{0,1}}};
constexpr std::array<int, 4> directionBits{{2,4,1,8}};
constexpr std::array<Point, 8> searchNeighbors{{{1,1},{-1,1},{1,-1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1}}};
constexpr std::array<Point, 8> pruneNeighbors{{{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1}}};

class Controller {
    const RmsWallInput& in;
    int w, h;
    std::vector<Point> starts;
    RmsWallBoundary result;
    std::vector<uint8_t> boundary, deposits;
    int index(int x, int y) const { return x*h+y; }
    bool inside(int x, int y) const { return x>=0 && y>=0 && x<w && y<h; }
    bool terrainPasses(int x, int y) const { return in.restrictions[in.terrain[y*w+x]] > 0; }
    void validate() {
        require(w==120 && h==120, "Only verified square120 profile is supported");
        require(in.terrain.size()==size_t(w*h), "Bad terrain size");
        require(in.restrictions.size()==53, "Expected actual53 terrain restriction columns");
        for (float value: in.restrictions) require(std::isfinite(value), "Nonfinite restriction");
        require(in.restrictions[0]>0, "Expected initially passable original terrain0 cache");
        for (uint8_t value: in.terrain) require(value<in.restrictions.size(), "Terrain index out of range");
        const auto distances=std::make_pair(in.minimumDistance,in.maximumDistance);
        require(distances==std::make_pair(10,20) || distances==std::make_pair(18,30) ||
                distances==std::make_pair(16,20) || distances==std::make_pair(18,22), "Unverified wall-distance profile");
        require(starts.size()==2 || starts.size()==3, "Expected two or three command centers");
        std::array<bool,3> seen{};
        auto geometry=[&](float x,float y,float rx,float ry) {
            require(std::isfinite(x)&&std::isfinite(y)&&std::isfinite(rx)&&std::isfinite(ry), "Nonfinite object geometry");
            require(x>=0 && y>=0 && x<w && y<h && rx>=0 && ry>=0 && rx<=w && ry<=h, "Invalid object geometry");
        };
        for (const auto& cc: in.commandCenters) {
            require(cc.owner>=1 && size_t(cc.owner)<=starts.size(), "Invalid command center owner");
            require(!seen[cc.owner-1], "Duplicate command center owner");seen[cc.owner-1]=true;
            geometry(cc.x,cc.y,cc.radiusX,cc.radiusY);
            require(cc.radiusX==2 && cc.radiusY==2, "Unverified command center radius");
            // 5d72a0/5d7300: truncation occurs both before and after radius subtraction.
            Point p{int(int(cc.x)-cc.radiusX-0.5),int(int(cc.y)-cc.radiusY-0.5)};
            require(inside(p.x,p.y), "Command center derived origin outside map");starts[cc.owner-1]=p;
        }
        require(!in.objects.empty() && in.objects.size()<=size_t(w*h*2), "Invalid object count");
        for (const auto& o: in.objects) {
            require(o.owner>=0 && size_t(o.owner)<=starts.size(), "Invalid object owner");geometry(o.x,o.y,o.radiusX,o.radiusY);
            require(o.obstructionType>=0 && o.obstructionType<=255 && o.obstructionClass>=0 && o.obstructionClass<=255, "Invalid obstruction metadata");
            require(o.unitClass>=0 && o.unitClass<=32767, "Invalid unit class");
            if(o.unitClass==29 || o.unitClass==30)
                require(o.radiusX==.5f && o.radiusY==.5f, "Unverified deposit registration footprint");
            require(o.obstructionType!=10, "Unverified type10 terrain callback");
            require((o.obstructionType<=2 || o.obstructionType==5), "Unverified obstruction type");
            if (o.obstructionType>0 && o.obstructionType<=2 && o.obstructionClass>0) {
                require(int(double(o.x)-o.radiusX)>=0 && int(double(o.y)-o.radiusY)>=0 && int(double(o.x)+o.radiusX)<=w && int(double(o.y)+o.radiusY)<=h, "Static footprint outside grid");
            }
        }
    }
    void makeObstruction() {
        result.obstruction.assign(w*h,0);deposits.assign(w*h,0);
        std::vector<uint8_t> refreshed(w*h,0);
        for (const auto& o: in.objects) {
            if(o.unitClass==29 || o.unitClass==30)deposits[index(int(o.x),int(o.y))]=1;
            if (o.obstructionType<=0 || o.obstructionType>2 || o.obstructionClass<=0) continue;
            const auto bounds=pathMapFootprint(o.x,o.y,o.radiusX,o.radiusY);
            const int x0=bounds.left,x1=bounds.right-1,y0=bounds.top,y1=bounds.bottom-1;
            for(int x=std::max(0,std::min(x0,x1)-1);x<=std::min(w-1,std::max(x0,x1)+1);++x)
                for(int y=std::max(0,std::min(y0,y1)-1);y<=std::min(h-1,std::max(y0,y1)+1);++y)refreshed[index(x,y)]=1;
            uint32_t word=uint32_t((o.obstructionClass&63)|(o.owner<<8)|0x40)<<8;
            if (o.obstructionClass==5) word|=0x8000;
            for (int y=bounds.top;y<bounds.bottom;++y)
                for (int x=bounds.left;x<bounds.right;++x) {
                    auto& cell=result.obstruction[y*w+x];cell=(cell&255)|word;
                }
        }
        result.directions.assign(w*h,0);
        auto pass=[&](int x,int y) { return inside(x,y) && terrainPasses(x,y) && !(result.obstruction[y*w+x]&0x4000); };
        // shortcut: verified startup refresh union; use event history if later terrain edits or unmatched removals affect cached cells.
        for (int x=0;x<w;++x) for (int y=0;y<h;++y) {
            const bool update=refreshed[index(x,y)]!=0;
            if(update && !pass(x,y))continue;
            for (size_t i=0;i<cardinal.size();++i) {
                const int nx=x+cardinal[i].x,ny=y+cardinal[i].y;
                if(update?pass(nx,ny):inside(nx,ny))result.directions[index(x,y)]|=directionBits[i];
            }
        }
    }
    void route(int x0,int y0,int x1,int y1,int side,uint8_t ownerBit) {
        if (!(x0<w && x1>=0 && y0<h && y1>0)) return;
        x0=std::max(x0,0);y0=std::max(y0,0);x1=std::min(x1,w-1);y1=std::min(y1,w-1);
        Point start=side==1 ? Point{x0,y1} : side==2 ? Point{x1,y0} : Point{x0,y0};
        std::vector<uint8_t> visited(w*h,0);
        std::vector<int> parents(w*h,-1);
        std::priority_queue<Node,std::vector<Node>,Later> queue;
        uint32_t order=0;
        auto enqueue=[&](int x,int y,int parent,float cost,float heuristic) {
            int cell=index(x,y);visited[cell]=1;parents[cell]=parent;
            queue.push(Node{cell,cost,float(cost+heuristic),++order});
        };
        enqueue(start.x,start.y,-1,0,0);
        while (!queue.empty()) {
            Node node=queue.top();queue.pop();int x=node.cell/h,y=node.cell%h;
            if ((side==2 || side==4) ? y==y1 : x==x1) {
                for (int p=node.cell;p!=-1;p=parents[p]) if (!(boundary[p]&ownerBit)) boundary[p]+=ownerBit;
                break;
            }
            for (size_t n=0;n<searchNeighbors.size();++n) {
                int nx=x+searchNeighbors[n].x,ny=y+searchNeighbors[n].y;
                if (nx<x0 || nx>x1 || ny<y0 || ny>y1 || visited[index(nx,ny)]) continue;
                bool edge0=(side==2 || side==4) ? ny==y0 : nx==x0;
                bool edge1=(side==2 || side==4) ? ny==y1 : nx==x1;
                int step;
                if (edge1) step=-100;
                else if (edge0) step=0;
                else {
                    int terrain=in.terrain[ny*w+nx];uint8_t flags=result.directions[index(nx,ny)];
                    if (terrain==35) step=600;
                    else if (nx==0 || ny==0 || nx==w-1 || ny==h-1) step=10;
                    else if (!flags) {
                        bool deposit=deposits[index(nx,ny)]!=0;
                        for(Point d:cardinal)if(inside(nx+d.x,ny+d.y))deposit|=deposits[index(nx+d.x,ny+d.y)]!=0;
                        step=deposit?300:10+(side==1 ? ny-y0 : side==2 ? nx-x0 : side==4 ? x1-nx : y1-ny);
                    } else step=(terrain==4 ? 300 : 100)+(n<4 ? 10 : 0);
                    if (boundary[index(nx,ny)]) step=2000;
                }
                // Original truncates stored parent float before adding the integer step.
                float cost=float(int(node.cost)+step);
                float heuristic=float((side==2 || side==4) ? y1-ny : x1-nx);
                enqueue(nx,ny,node.cell,cost,heuristic);
            }
        }
    }
    void reachable() {
        auto& scratch=result.reachableScratch;scratch.assign(w*h,0);
        for (int owner=0;owner<int(starts.size());++owner) {
            uint8_t bit=uint8_t(1<<owner);std::vector<Point> stack{starts[owner]};scratch[index(starts[owner].x,starts[owner].y)]+=bit;
            while (!stack.empty()) {
                Point p=stack.back();stack.pop_back();
                for (Point d: cardinal) {
                    int x=p.x+d.x,y=p.y+d.y;if (!inside(x,y)) continue;int i=index(x,y);
                    // Actual49a660(1): terrain-only cardinal reachability, not path bits.
                    if (!(scratch[i]&bit) && terrainPasses(x,y)) {
                        scratch[i]+=bit;if (!(boundary[i]&bit)) stack.push_back({x,y});
                    }
                }
            }
        }
        for (int i=0;i<w*h;++i) boundary[i]&=scratch[i];result.reachableBoundary=boundary;
    }
    void prune() {
        auto& scratch=result.prunedScratch;scratch.assign(w*h,0);
        struct Item { int x,y,cost; };
        for (int owner=0;owner<int(starts.size());++owner) {
            uint8_t bit=uint8_t(1<<owner);std::vector<Item> stack{{starts[owner].x,starts[owner].y,0}};scratch[index(starts[owner].x,starts[owner].y)]+=bit;
            while (!stack.empty()) {
                Item p=stack.back();stack.pop_back();int flags=result.directions[index(p.x,p.y)];
                if (!flags) ++p.cost;
                if (p.cost<=0 || p.cost>=3) {
                    for (size_t k=0;k<cardinal.size();++k) {
                        int x=p.x+cardinal[k].x,y=p.y+cardinal[k].y;if (!inside(x,y)) continue;int i=index(x,y);
                        if (!(scratch[i]&bit) && (flags&directionBits[k])) {
                            scratch[i]+=bit;stack.push_back({x,y,(boundary[i]&bit) ? 1 : 0});
                        }
                    }
                } else if (!flags || p.cost==1) {
                    for (Point d: pruneNeighbors) {
                        int x=p.x+d.x,y=p.y+d.y;if (!inside(x,y)) continue;int i=index(x,y);
                        if (!(scratch[i]&bit) && (boundary[i]&bit)) {scratch[i]+=bit;stack.push_back({x,y,p.cost});}
                    }
                }
            }
        }
        for (int i=0;i<w*h;++i) boundary[i]&=scratch[i];result.prunedBoundary=boundary;
    }
    void collectSegments() {
        auto& seen=result.segmentScratch;seen.assign(w*h,1);result.segments.resize(starts.size()+1);
        for (int x=0;x<w;++x) for (int y=0;y<w;++y) {
            int i=index(x,y);int value=boundary[i];if (!seen[i] || !value) continue;
            for (int owner=1;owner<=int(starts.size());++owner) {
                uint8_t bit=uint8_t(1<<(owner-1));if (!(value&bit)) continue;
                bool active=result.directions[i]!=0;int dx=0,dy=0;
                auto same=[&](int nx,int ny) {return inside(nx,ny) && boundary[index(nx,ny)]==bit && (result.directions[index(nx,ny)]!=0)==active;};
                if (same(x+1,y)) dx=1;
                else if (same(x+1,y+1)) dx=dy=1;
                else if (same(x,y+1)) dy=1;
                else if (same(x+1,y-1)) {dx=1;dy=-1;}
                int nx=x,ny=y;RmsWallSegment segment{owner,x,y,x,y,0,active};
                while ((boundary[index(nx,ny)]&bit) && (result.directions[index(nx,ny)]!=0)==active) {
                    seen[index(nx,ny)]=0;segment.toX=nx;segment.toY=ny;++segment.length;bool moved=false;
                    if (dx && nx<w-1) {++nx;moved=true;}
                    if (dy==1 && ny<h-1) {++ny;moved=true;}
                    if (dy==-1 && ny>0) {--ny;moved=true;}
                    if (!moved) break;
                }
                result.segments[owner].insert(result.segments[owner].begin(),segment);
            }
        }
    }
public:
    explicit Controller(const RmsWallInput& input):in(input),w(input.width),h(input.height),starts(input.commandCenters.size()) {}
    RmsWallBoundary run() {
        validate();makeObstruction();boundary.assign(w*h,0);
        for (int owner=0;owner<int(starts.size());++owner) {
            const auto p=starts[owner];const int lo=in.minimumDistance,hi=in.maximumDistance;uint8_t bit=uint8_t(1<<owner);
            route(p.x-hi,p.y-hi,p.x-lo,p.y+hi,2,bit);result.routeBoundaries.push_back(boundary);
            route(p.x+lo,p.y-hi,p.x+hi,p.y+hi,4,bit);result.routeBoundaries.push_back(boundary);
            route(p.x-hi,p.y-hi,p.x+hi,p.y-lo,1,bit);result.routeBoundaries.push_back(boundary);
            route(p.x-hi,p.y+lo,p.x+hi,p.y+hi,8,bit);result.routeBoundaries.push_back(boundary);
        }
        reachable();prune();collectSegments();return std::move(result);
    }
};
}
bool generateRmsWallBoundary(const RmsWallInput& input, RmsWallBoundary& output, std::string* error) {
    try {
        RmsWallBoundary pending=Controller(input).run();
        output=std::move(pending);
        if (error) error->clear();
        return true;
    } catch (const std::exception& failure) {
        if (error) *error=failure.what();
        return false;
    }
}
}
