/*  This file is part of YUView - The YUV player with advanced analytics toolset
 *   <https://github.com/IENT/YUView>
 *   Copyright (C) 2015  Institut für Nachrichtentechnik, RWTH Aachen University, GERMANY
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the
 *   OpenSSL library under certain conditions as described in each
 *   individual source file, and distribute linked combinations including
 *   the two.
 *
 *   You must obey the GNU General Public License in all respects for all
 *   of the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the
 *   file(s), but you are not obligated to do so. If you do not wish to do
 *   so, delete this exception statement from your version. If you delete
 *   this exception statement from all source files in the program, then
 *   also delete it here.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <array>

namespace video
{

/* 8bit limited range to full range mapping for luma & rgb data */
constexpr std::array<int, 256> LimitedRangeToFullRange = {
#if 0 /* without rounding */
  {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   2,
   3,   4,   5,   6,   8,   9,   10,  11,  12,  13,  15,  16,  17,  18,  19,  20,  22,  23,  24,
   25,  26,  27,  29,  30,  31,  32,  33,  34,  36,  37,  38,  39,  40,  41,  43,  44,  45,  46,
   47,  48,  50,  51,  52,  53,  54,  55,  57,  58,  59,  60,  61,  62,  64,  65,  66,  67,  68,
   69,  71,  72,  73,  74,  75,  76,  78,  79,  80,  81,  82,  83,  85,  86,  87,  88,  89,  90,
   91,  93,  94,  95,  96,  97,  98,  100, 101, 102, 103, 104, 105, 107, 108, 109, 110, 111, 112,
   114, 115, 116, 117, 118, 119, 121, 122, 123, 124, 125, 126, 128, 129, 130, 131, 132, 133, 135,
   136, 137, 138, 139, 140, 142, 143, 144, 145, 146, 147, 149, 150, 151, 152, 153, 154, 156, 157,
   158, 159, 160, 161, 163, 164, 165, 166, 167, 168, 170, 171, 172, 173, 174, 175, 176, 178, 179,
   180, 181, 182, 183, 185, 186, 187, 188, 189, 190, 192, 193, 194, 195, 196, 197, 199, 200, 201,
   202, 203, 204, 206, 207, 208, 209, 210, 211, 213, 214, 215, 216, 217, 218, 220, 221, 222, 223,
   224, 225, 227, 228, 229, 230, 231, 232, 234, 235, 236, 237, 238, 239, 241, 242, 243, 244, 245,
   246, 248, 249, 250, 251, 252, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255}};
#else /* with rounding */
  {
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   1,   2,   3,   5,   6,   7,   8,   9,  10,  12,  13,  14,  15,  16,  17,
     19,  20,  21,  22,  23,  24,  26,  27,  28,  29,  30,  31,  33,  34,  35,  36,
     37,  38,  40,  41,  42,  43,  44,  45,  47,  48,  49,  50,  51,  52,  54,  55,
     56,  57,  58,  59,  61,  62,  63,  64,  65,  66,  68,  69,  70,  71,  72,  73,
     75,  76,  77,  78,  79,  80,  82,  83,  84,  85,  86,  87,  88,  90,  91,  92,
     93,  94,  95,  97,  98,  99, 100, 101, 102, 104, 105, 106, 107, 108, 109, 111,
    112, 113, 114, 115, 116, 118, 119, 120, 121, 122, 123, 125, 126, 127, 128, 129,
    130, 132, 133, 134, 135, 136, 137, 139, 140, 141, 142, 143, 144, 146, 147, 148,
    149, 150, 151, 153, 154, 155, 156, 157, 158, 160, 161, 162, 163, 164, 165, 167,
    168, 169, 170, 171, 172, 173, 175, 176, 177, 178, 179, 180, 182, 183, 184, 185,
    186, 187, 189, 190, 191, 192, 193, 194, 196, 197, 198, 199, 200, 201, 203, 204,
    205, 206, 207, 208, 210, 211, 212, 213, 214, 215, 217, 218, 219, 220, 221, 222,
    224, 225, 226, 227, 228, 229, 231, 232, 233, 234, 235, 236, 238, 239, 240, 241,
    242, 243, 245, 246, 247, 248, 249, 250, 252, 253, 254, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  }
};
#endif

/* 8bit limited range to full range mapping for chroma data (with rounding) */
constexpr std::array<int, 256> LimitedRangeToFullRangeChroma = {
  {
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      1,   2,   3,   4,   5,   6,   7,   8,  10,  11,  12,  13,  14,  15,  16,  18,
     19,  20,  21,  22,  23,  24,  26,  27,  28,  29,  30,  31,  32,  34,  35,  36,
     37,  38,  39,  40,  41,  43,  44,  45,  46,  47,  48,  49,  51,  52,  53,  54,
     55,  56,  57,  59,  60,  61,  62,  63,  64,  65,  67,  68,  69,  70,  71,  72,
     73,  74,  76,  77,  78,  79,  80,  81,  82,  84,  85,  86,  87,  88,  89,  90,
     92,  93,  94,  95,  96,  97,  98, 100, 101, 102, 103, 104, 105, 106, 108, 109,
    110, 111, 112, 113, 114, 115, 117, 118, 119, 120, 121, 122, 123, 125, 126, 127,
    128, 129, 130, 131, 133, 134, 135, 136, 137, 138, 139, 141, 142, 143, 144, 145,
    146, 147, 148, 150, 151, 152, 153, 154, 155, 156, 158, 159, 160, 161, 162, 163,
    164, 166, 167, 168, 169, 170, 171, 172, 174, 175, 176, 177, 178, 179, 180, 182,
    183, 184, 185, 186, 187, 188, 189, 191, 192, 193, 194, 195, 196, 197, 199, 200,
    201, 202, 203, 204, 205, 207, 208, 209, 210, 211, 212, 213, 215, 216, 217, 218,
    219, 220, 221, 222, 224, 225, 226, 227, 228, 229, 230, 232, 233, 234, 235, 236,
    237, 238, 240, 241, 242, 243, 244, 245, 246, 248, 249, 250, 251, 252, 253, 254,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  }
};

} // namespace video
