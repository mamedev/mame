// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Konami video helper functions */
#ifndef MAME_KONAMI_KONAMI_HELPER_H
#define MAME_KONAMI_KONAMI_HELPER_H

#pragma once

void konami_decode_gfx(device_gfx_interface &gfxdecode, int gfx_index, uint8_t *data, uint32_t total, const gfx_layout *layout, int bpp);

template <typename T, typename U>
static inline void swap_less(int a, int b, T *layer, U *pri)
{
	if (pri[a] < pri[b])
	{
		int t = pri[a]; pri[a] = pri[b]; pri[b] = t;
		t = layer[a]; layer[a] = layer[b]; layer[b] = t;
	}
}

template <typename T, typename U>
static inline void swap_less_or_equal(int a, int b, T *layer, U *pri)
{
	if (pri[a] <= pri[b])
	{
		int t = pri[a]; pri[a] = pri[b]; pri[b] = t;
		t = layer[a]; layer[a] = layer[b]; layer[b] = t;
	}
}

// helper function to sort three tile layers by priority order
template <typename T, typename U>
static inline void konami_sortlayers3(T *layer, U *pri)
{
	swap_less(0, 1, layer, pri);
	swap_less(0, 2, layer, pri);
	swap_less(1, 2, layer, pri);
}
// helper function to sort four tile layers by priority order
template <typename T, typename U>
static inline void konami_sortlayers4(T *layer, U *pri)
{
	swap_less_or_equal(0, 1, layer, pri);
	swap_less_or_equal(0, 2, layer, pri);
	swap_less_or_equal(0, 3, layer, pri);
	swap_less_or_equal(1, 2, layer, pri);
	swap_less_or_equal(1, 3, layer, pri);
	swap_less_or_equal(2, 3, layer, pri);
}
// helper function to sort five tile layers by priority order
template <typename T, typename U>
static inline void konami_sortlayers5(T *layer, U *pri)
{
	swap_less_or_equal(0, 1, layer, pri);
	swap_less_or_equal(0, 2, layer, pri);
	swap_less_or_equal(0, 3, layer, pri);
	swap_less_or_equal(0, 4, layer, pri);
	swap_less_or_equal(1, 2, layer, pri);
	swap_less_or_equal(1, 3, layer, pri);
	swap_less_or_equal(1, 4, layer, pri);
	swap_less_or_equal(2, 3, layer, pri);
	swap_less_or_equal(2, 4, layer, pri);
	swap_less_or_equal(3, 4, layer, pri);
}


#endif // MAME_KONAMI_KONAMI_HELPER_H
