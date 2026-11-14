// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

namespace swgb {
// Original 60add0/60bcc0/60dbe0: padded tile dispatch also runs for clipped pixels.
inline bool terrainDrawScanContains(int cameraX,int cameraY,int columns,int rows,int x,int y) {
    const int a=columns/2+2,b=rows/2+1;
    const int sx=cameraX-a+b,sy=cameraY-a-b-8;
    const int row=(y-x)-(sy-sx);
    if (row<0 || row>=2*rows+24) return false;
    const int column=x-sx+(row+1)/2;
    return column>=0 && column<columns+13;
}
} // namespace swgb
