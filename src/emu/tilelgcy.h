/***************************************************************************

    tilelgcy.h

    Legacy tilemap helpers.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __TILELGCY_H__
#define __TILELGCY_H__


//**************************************************************************
//  MACROS
//**************************************************************************

#define TILE_GET_INFO(_name)            void _name(driver_device &device, tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
#define TILEMAP_MAPPER(_name)           tilemap_memory_index _name(driver_device &device, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows)

#define SET_TILE_INFO(GFX,CODE,COLOR,FLAGS)         tileinfo.set(device.machine(), GFX, CODE, COLOR, FLAGS)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// legacy callbacks
typedef void (*tile_get_info_func)(driver_device &device, tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
typedef tilemap_memory_index (*tilemap_mapper_func)(driver_device &device, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************


// ----- tilemap creation and configuration -----

// create a new tilemap; note that tilemaps are tracked by the core so there is no dispose
inline tilemap_t *tilemap_create(running_machine &machine, tile_get_info_func tile_get_info, tilemap_mapper_func mapper, int tilewidth, int tileheight, int cols, int rows)
{ return &machine.tilemap().create(tilemap_get_info_delegate(tile_get_info, "", machine.driver_data()), tilemap_mapper_delegate(mapper, "", machine.driver_data()), tilewidth, tileheight, cols, rows); }

inline tilemap_t *tilemap_create(running_machine &machine, tile_get_info_func tile_get_info, tilemap_standard_mapper mapper, int tilewidth, int tileheight, int cols, int rows)
{ return &machine.tilemap().create(tilemap_get_info_delegate(tile_get_info, "", machine.driver_data()), mapper, tilewidth, tileheight, cols, rows); }

#endif  /* __TILELGCY_H__ */
