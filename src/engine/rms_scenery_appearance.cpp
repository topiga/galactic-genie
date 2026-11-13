// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_scenery_appearance.h"
#include "../core/genie_random.h"

namespace swgb {
RmsCarbonAppearance drawRmsCarbonAppearance(uint32_t &randomState) {
    genieRandomDraw(randomState); //54c44f: temporary base facet.
    genieRandomDraw(randomState); //423742: frame draw, even with exactly one frame.
    const uint32_t index=genieRandomDraw(randomState)*12u/32767u;
    // Original4a099a x87 heading and4a13c0 facet mapping, verified for all32768 draws.
    // Preserve the max-draw endpoint and float-rounding boundary at index2.
    static constexpr RmsCarbonAppearance appearances[]={
        {11,0.f}, {0,0.5235987901687622f}, {0,1.0471975803375244f},
        {1,1.5707963705062866f}, {2,2.094395160675049f}, {3,2.6179938316345215f},
        {4,3.1415927410125732f}, {5,3.665191411972046f}, {6,4.188790321350098f},
        {7,4.71238899230957f}, {8,5.235987663269043f}, {9,5.759586334228516f},
        {10,6.2831854820251465f}};
    return appearances[index];
}
}
