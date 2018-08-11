// license:BSD-3-Clause
// copyright-holders:superctr, Valley Bell
/***************************************************************************

    Capcom System QSound™

***************************************************************************/

#include "qsoundbase.h"

qsound_base_device::qsound_base_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this, 24)
{
}

ROM_START( qsound )
	ROM_REGION16_BE( 0x2000, "dsp", 0 )
	ROM_LOAD16_WORD_SWAP( "dl-1425.bin", 0x0000, 0x2000, CRC(d6cf5ef5) SHA1(555f50fe5cdf127619da7d854c03f4a244a0c501) )
	ROM_IGNORE( 0x4000 )
ROM_END

//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------

const tiny_rom_entry *qsound_base_device::device_rom_region() const
{
	return ROM_NAME( qsound );
}
