/*
    Copyright (C) 2026 Glacc

    This file is part of sw-render.

    sw-render is free software: you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation, either
    version 3 of the License, or (at your option) any later version.

    sw-render is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with sw-render. If not,
    see <https://www.gnu.org/licenses/>. 
*/

#pragma once

#include <hb.h>
#include <hb-ft.h>

#include "FTEssentials.h"

typedef struct FTExtHarfBuzz_StateStruct
{
    hb_font_t *font;
    hb_face_t *face;

    hb_buffer_t *buffer;
}
FTExtHarfBuzz_State;

#ifdef __cplusplus
extern "C"
{
#endif

    extern bool FTExtHarfBuzz_InitFromFTStates(FTExtHarfBuzz_State *hb_state, FTEssentials_State *ft_state);
    extern void FTExtHarfBuzz_Cleanup(FTExtHarfBuzz_State *state);
    

#ifdef __cplusplus
}
#endif

