// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <optional>
namespace swgb {
// Original5819c0: class overrides apply to observed actors; fact lookups pass -1.
inline int aiKnownCategory(int prototype,int unitClass=-1) {
    const auto cls=int16_t(unitClass);
    if(cls==58)return 34;
    if(cls==8)return 35;
    if(cls==50 || cls==51)return 33;
    switch(int16_t(prototype)) {
    case 323:
        return 0;
    case 584:
        return 1;
    case 68:
        return 2;
    case 319:
        return 3;
    case 562:
        return 4;
    case 12:
        return 5;
    case 335:
        return 6;
    case 1001:
        return 7;
    case 109:
        return 8;
    case 87:
        return 9;
    case 101:
        return 10;
    case 276:
        return 11;
    case 45:
        return 12;
    case 598:
        return 13;
    case 104:
        return 14;
    case 82:
        return 15;
    case 70:
        return 16;
    case 49:
        return 17;
    case 103:
        return 18;
    case 209:
        return 19;
    case 84:case 530:
        return 20;
    case 317:
        return 21;
    case 72:
        return 22;
    case 117:
        return 23;
    case 155:
        return 24;
    case 195:
        return 25;
    case 79:
        return 26;
    case 234:
        return 27;
    case 235:
        return 28;
    case 236:
        return 29;
    case 196:
        return 30;
    case 50:
        return 31;
    case 199:
        return 32;
    case 931:
        return 36;
    case 13:
        return 37;
    case 434:
        return 38;
    case 176:case 888:case 890:case 892:case 894:case 896:case 1477:case 1479:
        return 39;
    case 177:case 803:case 805:case 807:case 809:case 811:case 1481:case 1483:
        return 40;
    case 182:case 908:case 910:case 912:case 914:case 916:case 1485:case 1487:
        return 41;
    case 184:case 829:case 831:case 835:case 928:case 930:case 1489:case 1491:
        return 42;
    case 186:case 898:case 900:case 902:case 904:case 906:case 1493:case 1495:
        return 43;
    case 187:case 868:case 870:case 872:case 874:case 876:case 1497:case 1499:
        return 44;
    case 188:case 918:case 920:case 922:case 924:case 926:case 1501:case 1503:
        return 45;
    case 189:case 878:case 880:case 882:case 884:case 886:case 1505:case 1507:
        return 46;
    case 190:case 815:case 818:case 820:case 823:case 825:case 1509:case 1511:
        return 47;
    case 191:case 838:case 839:case 840:case 841:case 842:case 1513:case 1515:
        return 48;
    case 108:case 733:case 735:case 745:case 748:case 750:case 1517:case 1518:
        return 49;
    case 113:case 752:case 754:case 756:case 758:case 760:case 1519:case 1520:
        return 50;
    case 1036:case 1038:case 1040:case 1042:case 1044:case 1046:case 1521:case 1522:
        return 51;
    case 127:case 762:case 764:case 766:case 769:case 771:case 1523:case 1524:
        return 52;
    case 158:case 773:case 775:case 777:case 779:case 781:case 1525:case 1526:
        return 53;
    case 160:case 783:case 785:case 787:case 789:case 791:case 1527:case 1528:
        return 54;
    case 163:case 793:case 795:case 797:case 799:case 801:case 1529:case 1530:
        return 55;
    case 169:case 951:case 952:case 953:case 954:case 955:case 1531:case 1532:
        return 56;
    case 170:case 961:case 963:case 965:case 967:case 969:case 1533:case 1535:
        return 57;
    case 172:case 638:case 640:case 971:case 975:case 977:case 979:case 981:
        return 58;
    case 468:case 983:case 984:case 985:case 986:case 987:case 1537:case 1538:
        return 59;
    case 2:case 4:case 6:case 8:case 174:case 641:case 642:case 993:
        return 60;
    case 192:case 631:case 633:case 635:case 637:case 639:case 1539:case 1541:
        return 61;
    case 197:case 651:case 653:case 657:case 675:case 679:case 1543:case 1545:
        return 62;
    case 200:case 681:case 683:case 685:case 687:case 689:case 1547:case 1549:
        return 63;
    case 203:case 691:case 693:case 695:case 698:case 700:case 1551:case 1553:
        return 64;
    case 207:case 702:case 704:case 706:case 708:case 711:case 1555:case 1557:
        return 65;
    case 231:case 713:case 715:case 717:case 719:case 721:case 1559:case 1561:
        return 66;
    case 244:case 469:case 471:case 473:case 475:case 479:case 903:case 907:
        return 67;
    case 246:case 485:case 489:case 493:case 495:case 497:case 911:case 915:
        return 68;
    case 249:case 500:case 502:case 508:case 514:case 517:case 919:case 923:
        return 69;
    case 258:case 550:case 552:case 554:case 556:case 558:case 927:case 932:
        return 70;
    case 261:case 560:case 566:case 568:case 570:case 572:case 957:case 959:
        return 71;
    case 263:case 574:case 576:case 578:case 588:case 596:case 973:case 980:
        return 72;
    case 277:case 603:case 622:case 624:case 626:case 629:case 989:case 991:
        return 73;
    case 167:case 939:case 941:case 943:case 945:case 947:case 996:case 1005:
        return 74;
    case 287:case 329:case 331:case 344:case 346:case 352:case 1015:case 1037:
        return 75;
    case 289:case 359:case 361:case 370:case 376:case 378:case 1043:case 1047:
        return 76;
    case 292:case 381:case 383:case 385:case 387:case 389:case 1478:case 1482:
        return 77;
    case 295:case 391:case 393:case 395:case 397:case 399:case 1486:case 1490:
        return 78;
    case 301:case 401:case 403:case 405:case 407:case 409:case 1494:case 1498:
        return 79;
    case 297:case 412:case 418:case 420:case 422:case 424:case 1502:case 1506:
        return 80;
    case 303:case 426:case 428:case 430:case 432:case 436:case 1510:case 1514:
        return 81;
    case 305:case 438:case 440:case 442:case 444:case 446:case 1540:case 1544:
        return 82;
    case 238:case 280:case 282:case 286:case 307:case 309:case 1548:case 1552:
        return 83;
    case 315:case 448:case 452:case 454:case 460:case 462:case 1556:case 1560:
        return 84;
    case 1007:case 1008:case 1009:case 1010:case 1011:case 1012:case 1013:case 1014:
        return 85;
    case 619:case 620:case 625:case 628:case 630:case 632:case 634:case 636:
        return 86;
    default:return -1;
    }
}
struct AiKnownCategorySnapshot {int prototype=-1,unitClass=-1,owner=-1;uint32_t lastObservedTime=0;};
class AiKnownCategoryMemory {
public:
    std::optional<uint32_t> lastRefreshTime;
    int count(int owner,int prototype) const {
        const int category=aiKnownCategory(prototype);
        return owner>=1 && owner<=8 && category>=0?counts_[size_t(owner)][size_t(category)]:-1;
    }
    // Caller supplies a qualifying observed record. New foreign admission591db0
    // increments once; own records exist but do not credit the foreign cache.
    void observe(int observer,int id,int owner,int prototype,int unitClass,uint32_t now=0) {
        if(id<0)return;
        auto [it,added]=records_.emplace(id,AiKnownCategorySnapshot{prototype,unitClass,owner,now});
        const int next=aiKnownCategory(prototype,unitClass);
        if(added) {
            if(owner!=observer)adjust(owner,next,1);
        } else {
            const int prior=aiKnownCategory(it->second.prototype,it->second.unitClass);
            // Actual587e60 uses incoming owner for BOTH adjustments. An ownership
            // change with an unchanged category leaves all counters unchanged.
            if(prior!=next) {adjust(owner,prior,-1);adjust(owner,next,1);}
            it->second={prototype,unitClass,owner,now};
        }
    }
    // Actual58a8a0 changes remembered ownership only if its object lookup resolves.
    // It neither erases the record nor decrements category counts. This is NOT an
    // invented generic death hook: callers must identify the original event.
    void ownedRemoval(int id,int resolvedOwner) {
        auto i=records_.find(id);if(i!=records_.end() && resolvedOwner>=0)i->second.owner=resolvedOwner;
    }
    // Original585b18/591eb0: strict unsigned five-minute gate, twenty-minute
    // observation horizon. Lookup is restricted to retained IDs, but deliberately
    // uses their current live prototype/class/owner even outside current sight.
    template<class Lookup> bool refreshIfDue(int observer,uint32_t now,Lookup lookup) {
        if(!lastRefreshTime || uint32_t(now-*lastRefreshTime)<=300000u)return false;
        rebuild(observer,now>=1200000u?now-1200000u:0u,lookup);
        lastRefreshTime=now;return true;
    }
    template<class Lookup> void rebuild(int observer,uint32_t cutoff,Lookup lookup) {
        for(auto &owner:counts_)owner.fill(0);
        for(const auto &[id,record]:records_) {
            if(record.owner==observer || (cutoff!=UINT32_MAX && record.lastObservedTime<cutoff))continue;
            const auto current=lookup(id);
            if(current)adjust(current->owner,aiKnownCategory(current->prototype,current->unitClass),1);
        }
    }
    const AiKnownCategorySnapshot *snapshot(int id) const {
        const auto i=records_.find(id);return i==records_.end()?nullptr:&i->second;
    }
private:
    void adjust(int owner,int category,int delta) {
        if(owner>=1 && owner<=8 && category>=0)
            counts_[size_t(owner)][size_t(category)]+=delta;
    }
    std::vector<std::array<int32_t,87>> counts_{9};
    std::unordered_map<int,AiKnownCategorySnapshot> records_;
};
} // namespace swgb
