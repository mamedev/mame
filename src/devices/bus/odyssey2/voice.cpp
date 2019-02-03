// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Magnavox The Voice emulation

 TODO:
   - load speech ROM from softlist
   - move external speech rom for S.I.D. the Spellbinder into the softlist entry

 ***********************************************************************************************************/


#include "emu.h"
#include "voice.h"
#include "speaker.h"


//-------------------------------------------------
//  o2_voice_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(O2_ROM_VOICE, o2_voice_device, "o2_voice", "Odyssey 2 The Voice Passthrough Cart")


o2_voice_device::o2_voice_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: o2_rom_device(mconfig, O2_ROM_VOICE, tag, owner, clock)
	, m_speech(*this, "sp0256_speech")
	, m_subslot(*this, "subslot")
	, m_lrq_state(0)
{
}


void o2_voice_device::device_start()
{
	save_item(NAME(m_lrq_state));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void o2_voice_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	SP0256(config, m_speech, 3120000);
	m_speech->data_request_callback().set(FUNC(o2_voice_device::lrq_callback));
	// The Voice uses a speaker with its own volume control so the relative volumes to use are subjective, these sound good
	m_speech->add_route(ALL_OUTPUTS, "mono", 1.00);

	O2_CART_SLOT(config, m_subslot, o2_cart, nullptr);
}


ROM_START( o2voice )
	ROM_REGION( 0x10000, "sp0256_speech", 0 )
	// SP0256B-019 Speech chip w/2KiB mask rom
	ROM_LOAD( "sp0256b-019.bin",  0x1000, 0x0800, CRC(4bb43724) SHA1(49f5326ad45392dc96c89d1d4e089a20bd21e609) )

	/* A note about "The Voice": Two versions of "The Voice" exist:
	   * An earlier version with eight 2KiB speech roms, spr016-??? through spr016-??? on a small daughterboard
	     <note to self: fill in numbers later>
	   * A later version with one 16KiB speech rom, spr128-003, mounted directly on the mainboard
	   The rom contents of these two versions are EXACTLY the same.
	   Both versions have an sp0256b-019 speech chip, which has 2KiB of its own internal speech data
	   Thanks to kevtris for this info. - LN
	*/

	// External 16KiB speech ROM (spr128-003) from "The Voice"
	ROM_LOAD( "spr128-003.bin",   0x4000, 0x4000, CRC(509367b5) SHA1(0f31f46bc02e9272885779a6dd7102c78b18895b) )
	// Additional External 16KiB speech ROM (spr128-004) from S.I.D. the Spellbinder
	ROM_LOAD( "spr128-004.bin",   0x8000, 0x4000, CRC(e79dfb75) SHA1(37f33d79ffd1739d7c2f226b010a1eac28d74ca0) )
ROM_END

const tiny_rom_entry *o2_voice_device::device_rom_region() const
{
	return ROM_NAME( o2voice );
}

WRITE_LINE_MEMBER(o2_voice_device::lrq_callback)
{
	m_lrq_state = state;
}

WRITE8_MEMBER(o2_voice_device::io_write)
{
	if (data & 0x20)
		m_speech->ald_w(offset & 0x7f);
	else
		m_speech->reset();
}
