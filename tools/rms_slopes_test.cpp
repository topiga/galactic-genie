// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_slopes.h"
#include <cstdlib>
#include <iostream>

static void check(bool ok) { if(!ok) { std::cerr<<"RMS original slope mismatch\n";std::exit(1); } }
int main() {
    std::vector<uint8_t> heights(9),slopes;
    std::string error;
    for (int center:{1,128,254}) {
        uint32_t hash=2166136261u;
        for(int code=0;code<6561;++code) {
            int digits=code;
            heights[4]=center;
            for(int i=8;i>=0;--i) if(i!=4) { heights[i]=center+digits%3-1;digits/=3; }
            check(swgb::generateRmsSlopes(3,3,heights,slopes,&error));
            hash=(hash^slopes[4])*16777619u;
        }
        check(hash==2776554441u);
    }
    const std::vector<std::vector<uint8_t>> inputs={
        {0,0,0,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255},
        {0,1,2,1,2,3,2,3,4},{2,0,2,0,1,0,2,0,2},{0,0,0,0,255,0,0,0,0}};
    const std::vector<std::vector<uint8_t>> expected={
        {0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},
        {15,15,7,15,15,7,8,8,0},{0,7,0,8,2,5,0,6,0},{0,0,0,0,0,0,0,0,0}};
    for(size_t i=0;i<inputs.size();++i) {
        heights=inputs[i];
        check(swgb::generateRmsSlopes(3,3,heights,heights,&error) && heights==expected[i]);
    }
    // Actual4925d0/495bb0/496170; final visual refresh callbacks omitted.
    const uint32_t smoothing[][5]={
        {1,1,1,808256u,84696351u},
        {1,1,2,84696351u,84696351u},
        {1,1,3,101473970u,84696351u},
        {1,1,4,51141113u,84696351u},
        {1,1,5,84696351u,84696351u},
        {1,1,6,17585875u,84696351u},
        {1,1,7,34363494u,84696351u},
        {1,1,8,67918732u,84696351u},
        {1,1,9,808256u,84696351u},
        {1,1,10,218917303u,84696351u},
        {1,1,11,118251589u,84696351u},
        {1,1,12,808256u,84696351u},
        {1,1,13,84696351u,84696351u},
        {1,1,14,101473970u,84696351u},
        {1,1,15,51141113u,84696351u},
        {1,1,16,84696351u,84696351u},
        {1,1,17,17585875u,84696351u},
        {1,1,18,34363494u,84696351u},
        {1,1,19,67918732u,84696351u},
        {1,1,20,17585875u,84696351u},
        {3,3,1,890597978u,1528984707u},
        {3,3,2,3517971239u,144701479u},
        {3,3,3,4150428292u,1386711205u},
        {3,3,4,1783933617u,2398570257u},
        {3,3,5,2380981024u,2937312068u},
        {3,3,6,337403231u,108029499u},
        {3,3,7,3985124827u,3289396484u},
        {3,3,8,2644845127u,2749826396u},
        {3,3,9,4150428292u,1386711205u},
        {3,3,10,1045892183u,2736670214u},
        {3,3,11,3517971239u,144701479u},
        {3,3,12,1434592918u,1375881419u},
        {3,3,13,3237530290u,281399821u},
        {3,3,14,1903259212u,2742470373u},
        {3,3,15,4150428292u,1386711205u},
        {3,3,16,2527242500u,2095985762u},
        {3,3,17,2741467744u,2749826396u},
        {3,3,18,4281248323u,2043170610u},
        {3,3,19,2046182324u,3270713282u},
        {3,3,20,571651758u,3680964856u},
        {7,9,1,2014237534u,3870463657u},
        {7,9,2,2632344413u,3249890073u},
        {7,9,3,2901842961u,719187039u},
        {7,9,4,1321254543u,2438532588u},
        {7,9,5,591886501u,3549806683u},
        {7,9,6,1284162198u,1972234550u},
        {7,9,7,2912068351u,3644523195u},
        {7,9,8,612037717u,480959124u},
        {7,9,9,474596920u,3395679132u},
        {7,9,10,1862906887u,2204293452u},
        {7,9,11,4067277931u,4289635199u},
        {7,9,12,4069747455u,683218039u},
        {7,9,13,1388525853u,2932395366u},
        {7,9,14,120994838u,3322129844u},
        {7,9,15,256511590u,2348081793u},
        {7,9,16,4000103270u,1018887720u},
        {7,9,17,2836338195u,2849492148u},
        {7,9,18,4070331306u,1747094376u},
        {7,9,19,686190750u,402621902u},
        {7,9,20,1136100173u,3131512782u},
        {9,7,1,1683494051u,3371216447u},
        {9,7,2,782031446u,2554706616u},
        {9,7,3,2632227067u,1338550422u},
        {9,7,4,437670979u,2824281037u},
        {9,7,5,2538587637u,2315477704u},
        {9,7,6,2817826866u,167093666u},
        {9,7,7,247814866u,1594895013u},
        {9,7,8,2841436643u,270214422u},
        {9,7,9,3619003169u,3237286974u},
        {9,7,10,3438409703u,701673505u},
        {9,7,11,1708664479u,2533876836u},
        {9,7,12,1632902494u,1461615758u},
        {9,7,13,2329615093u,1894532053u},
        {9,7,14,296721952u,2559070226u},
        {9,7,15,3994672762u,3507341571u},
        {9,7,16,1342143781u,3201082886u},
        {9,7,17,5129061u,4154699708u},
        {9,7,18,1131721892u,3549678794u},
        {9,7,19,4073711491u,2011650070u},
        {9,7,20,3669358865u,1986613119u},
        {16,16,1,3185113683u,2317234805u},
        {16,16,2,2727989017u,1427530092u},
        {16,16,3,996901746u,3810230455u},
        {16,16,4,3802289239u,212133231u},
        {16,16,5,4260547760u,749538938u},
        {16,16,6,3607130738u,4204107305u},
        {16,16,7,1856026895u,4143833125u},
        {16,16,8,1303204689u,879060652u},
        {16,16,9,356781623u,2994612375u},
        {16,16,10,4102701947u,267678834u},
        {16,16,11,1555087179u,2522983042u},
        {16,16,12,2872866561u,115689357u},
        {16,16,13,2101670583u,3196989541u},
        {16,16,14,3925995455u,1911878314u},
        {16,16,15,400832356u,2765106994u},
        {16,16,16,247300454u,719532260u},
        {16,16,17,1790651224u,2741903049u},
        {16,16,18,3735389089u,494783869u},
        {16,16,19,2499657173u,2792557662u},
        {16,16,20,1359693444u,2665562140u}
    };
    for(const auto &fixture:smoothing) {
        uint32_t state=fixture[2];
        heights.clear();
        for(uint32_t i=0;i<fixture[0]*fixture[1];++i) {
            state=state*214013u+2531011u;heights.push_back((state>>16)%9);
        }
        check(swgb::smoothRmsHeights(fixture[0],fixture[1],heights,&error));
        check(swgb::generateRmsSlopes(fixture[0],fixture[1],heights,slopes,&error));
        uint32_t heightHash=2166136261u,slopeHash=2166136261u;
        for(uint8_t v:heights) heightHash=(heightHash^v)*16777619u;
        for(uint8_t v:slopes) slopeHash=(slopeHash^v)*16777619u;
        check(heightHash==fixture[3] && slopeHash==fixture[4]);
    }
    const uint32_t sparse[][4]={
        {32,0,410655402u,1253853877u},
        {32,1,2630615873u,2824267061u},
        {32,2,3073650368u,2069937073u},
        {64,0,3685433194u,2010352309u},
        {64,1,2412129281u,1712082421u},
        {64,2,3554867026u,368600415u}
    };
    for(const auto &fixture:sparse) {
        const int width=fixture[0],kind=fixture[1];
        heights.assign(width*width,0);
        for(int y=0;y<width;++y) for(int x=0;x<width;++x) {
            if(kind==0) heights[y*width+x]=x==width/2 && y==width/2 ? 7:0;
            else if(kind==1) heights[y*width+x]=std::abs(x-width/2)+std::abs(y-width/2)<width/4 ? 4:0;
            else heights[y*width+x]=(x*17+y*31)%97==0 ? 1:0;
        }
        check(swgb::smoothRmsHeights(width,width,heights,&error));
        check(swgb::generateRmsSlopes(width,width,heights,slopes,&error));
        uint32_t heightHash=2166136261u,slopeHash=2166136261u;
        for(uint8_t v:heights) heightHash=(heightHash^v)*16777619u;
        for(uint8_t v:slopes) slopeHash=(slopeHash^v)*16777619u;
        check(heightHash==fixture[2] && slopeHash==fixture[3]);
    }
    heights={9};
    check(!swgb::smoothRmsHeights(1,1,heights,&error) && heights==std::vector<uint8_t>{9});
    heights={8};
    check(!swgb::smoothRmsHeights(2,1,heights,&error) && heights==std::vector<uint8_t>{8});
    check(swgb::smoothRmsHeights(1,1,heights,&error) && heights==std::vector<uint8_t>{8} && error.empty());
    slopes={42};
    for(uint32_t width:{0u,1025u,UINT32_MAX,2u})
        check(!swgb::generateRmsSlopes(width,3,inputs[0],slopes,&error) && slopes==std::vector<uint8_t>{42} && !error.empty());
    check(swgb::generateRmsSlopes(1,1,{255},slopes,&error) && slopes==std::vector<uint8_t>{0} && error.empty());
    std::cout<<"19683 original slope neighborhoods, 106 smoothing maps, boundaries and validation passed\n";
}
