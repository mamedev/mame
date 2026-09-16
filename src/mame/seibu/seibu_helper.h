// license:BSD-3-Clause
// copyright-holders:Ville Linde, Nicola Salmoria
#ifndef MAME_SEIBU_SEIBU_HELPER_H
#define MAME_SEIBU_SEIBU_HELPER_H

#pragma once

u32 seibu_partial_carry_sum(u32 add1, u32 add2, u32 carry_mask, int bits);

u32 seibu_partial_carry_sum32(u32 add1, u32 add2, u32 carry_mask);
u32 seibu_partial_carry_sum24(u32 add1, u32 add2, u32 carry_mask);

#endif // MAME_SEIBU_SEIBU_HELPER_H
