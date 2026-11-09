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

#include "FTExtHarfBuzz.h"

#include <hb.h>
#include <hb-ft.h>

#include "FTEssentials.h"

bool FTExtHarfBuzz_InitFromFTStates(FTExtHarfBuzz_State *hb_state, FTEssentials_State *ft_state)
{
    if (!hb_state)
        return false;

    if (!ft_state)
        return false;

    hb_state->font = hb_ft_font_create_referenced(ft_state->ft_face);
    if (!hb_state->font)
        return false;

    hb_ft_font_set_funcs(hb_state->font);

    hb_state->face = hb_ft_face_create_referenced(ft_state->ft_face);
    if (!hb_state->face)
    {
        hb_font_destroy(hb_state->font);
        hb_state->font = NULL;

        return false;
    }

    hb_state->buffer = hb_buffer_create();
    if (!hb_state->buffer)
    {
        hb_face_destroy(hb_state->face);
        hb_state->face = NULL;
        hb_font_destroy(hb_state->font);
        hb_state->font = NULL;

        return false;
    }

    return true;
}

void FTExtHarfBuzz_Cleanup(FTExtHarfBuzz_State *state)
{
    if (!state)
        return;

    hb_buffer_destroy(state->buffer);
    state->buffer = NULL;

    hb_face_destroy(state->face);
    state->face = NULL;

    hb_font_destroy(state->font);
    state->font = NULL;
}


