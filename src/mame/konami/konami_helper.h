// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Konami video helper functions */
#ifndef MAME_KONAMI_KONAMI_HELPER_H
#define MAME_KONAMI_KONAMI_HELPER_H

#pragma once

#include <algorithm>
#include <functional>


void konami_decode_gfx(device_gfx_interface &gfxdecode, int gfx_index, uint8_t *data, uint32_t total, const gfx_layout *layout, int bpp);


// unrolled bubble sort helper
template <unsigned N, unsigned A = N - 2, unsigned B = N - 1, typename C, typename T, typename U>
inline void konami_sortlayers(C cmp, T *layer, U *pri)
{
	if constexpr (B > (A + 1))
		konami_sortlayers<N, A, B - 1>(cmp, layer, pri);
	else if constexpr (A > 0)
		konami_sortlayers<N, A - 1, N - 1>(cmp, layer, pri);
	if (cmp(pri[A], pri[B]))
	{
		using std::swap;
		swap(pri[A], pri[B]);
		swap(layer[A], layer[B]);
	}
}

// helper function to sort three tile layers by priority order
template <typename T, typename U>
inline void konami_sortlayers3(T *layer, U *pri)
{
	konami_sortlayers<3>(std::less<U>(), layer, pri);
}

// helper function to sort four tile layers by priority order
template <typename T, typename U>
inline void konami_sortlayers4(T *layer, U *pri)
{
	konami_sortlayers<4>(std::less_equal<U>(), layer, pri);
}

// helper function to sort five tile layers by priority order
template <typename T, typename U>
inline void konami_sortlayers5(T *layer, U *pri)
{
	konami_sortlayers<5>(std::less_equal<U>(), layer, pri);
}

#endif // MAME_KONAMI_KONAMI_HELPER_H
