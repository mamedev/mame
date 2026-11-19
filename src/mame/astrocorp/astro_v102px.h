// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert

/***************************************************************************

    Astro Corp. V102PX custom CPU (68000 with encrypted program ROMs)

***************************************************************************/

#ifndef MAME_ASTROCORP_ASTRO_V102PX_H
#define MAME_ASTROCORP_ASTRO_V102PX_H

#pragma once

#include "cpu/m68000/m68000.h"

#include <utility>


namespace astro_v102px {

struct decryption_info
{
	struct
	{
		// Address bits used for bitswap/xor selection
		u8 bits[3];
		struct
		{
			// 8-8 Bitswap
			u8 bits[8];
			// Xor
			u8 xor_mask;
		} entries[8];
	} rom[2];
	// Global address bitswap (src -> dest, some sets use bits 12-8 only, while others 12-2)
	u8 bits[11];
};

extern const decryption_info px001;
extern const decryption_info px002;
extern const decryption_info px003;
extern const decryption_info &px004;
extern const decryption_info px005;
extern const decryption_info px006;
extern const decryption_info px007;
extern const decryption_info px008;
extern const decryption_info px010;
extern const decryption_info px012;
extern const decryption_info px013;
extern const decryption_info px014;
extern const decryption_info px016;

// V102PX model unknown
extern const decryption_info gostopac;
extern const decryption_info dslayrr;

} // namespace astro_v102px


class astro_v102px_device : public m68000_device
{
public:
	astro_v102px_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_decryption(const astro_v102px::decryption_info &table) { m_decryption = &table; }
	template <typename T> void set_encrypted_region(T &&tag) { m_encrypted.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	required_memory_region m_rom;
	required_region_ptr<u16> m_encrypted;

	const astro_v102px::decryption_info *m_decryption;
};

DECLARE_DEVICE_TYPE(ASTRO_V102PX, astro_v102px_device)

#endif // MAME_ASTROCORP_ASTRO_V102PX_H
