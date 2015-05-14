// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Special case 'Really Simple' blitters, no blending, no tinting etc.*/

#include "emu.h"
#include "epic12.h"


#define REALLY_SIMPLE 1
#define BLENDED 0

#define TRANSPARENT 1
#define FLIPX 0
#define FUNCNAME draw_sprite_f0_ti0_tr1_simple
#include "epic12in.inc"
#undef FUNCNAME
#undef FLIPX

#define FLIPX 1
#define FUNCNAME draw_sprite_f1_ti0_tr1_simple
#include "epic12in.inc"
#undef FUNCNAME
#undef FLIPX
#undef TRANSPARENT


#define TRANSPARENT 0
#define FLIPX 0
#define FUNCNAME draw_sprite_f0_ti0_tr0_simple
#include "epic12in.inc"
#undef FUNCNAME
#undef FLIPX

#define FLIPX 1
#define FUNCNAME draw_sprite_f1_ti0_tr0_simple
#include "epic12in.inc"
#undef FUNCNAME
#undef FLIPX
#undef TRANSPARENT

#undef BLENDED
#undef REALLY_SIMPLE
