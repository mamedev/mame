// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Fabio Priuli
/******************************************************************************

Magnavox The Voice emulation

Hardware notes:
- SP0256B-019 with 2KB internal ROM
- 16KB speech ROM, or 8*2KB on a daughterboard(same contents)
- passthrough cartridge slot, with support for extra speech ROM

Cartridge pins A,B,E,1,10,11 are repurposed for the extra speech ROM. This means
that (European) cartridges using extra I/O won't work on it.

******************************************************************************/

#include "emu.h"
#include "voice.h"
#include "speaker.h"

DEFINE_DEVICE_TYPE(O2_ROM_VOICE, o2_voice_device, "o2_voice", "Odyssey 2 The Voice Passthrough Cart")

//-------------------------------------------------
//  o2_voice_device - constructor
//-------------------------------------------------

o2_voice_device::o2_voice_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, O2_ROM_VOICE, tag, owner, clock),
	device_o2_cart_interface(mconfig, *this),
	m_speech(*this, "speech"),
	m_subslot(*this, "subslot")
{ }

void o2_voice_device::device_start()
{
	save_item(NAME(m_lrq_state));
	save_item(NAME(m_control));
}

void o2_voice_device::cart_init()
{
	u32 size = get_voice_size();
	if (size == 0)
		fatalerror("o2_voice_device: No voice region found\n");

	size = (size > 0x8000) ? 0x8000 : size;
	memcpy(memregion("speech")->base(), get_voice_base(), size);
}

void o2_voice_device::device_reset()
{
	if (m_subslot->exists() && m_subslot->cart() && m_subslot->cart()->get_voice_size())
	{
		// load additional speech data
		u32 size = m_subslot->cart()->get_voice_size();
		size = (size > 0x8000) ? 0x8000 : size;
		memcpy(memregion("speech")->base() + 0x8000, m_subslot->cart()->get_voice_base(), size);
	}
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void o2_voice_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	SP0256(config, m_speech, 3.12_MHz_XTAL);
	m_speech->data_request_callback().set(FUNC(o2_voice_device::lrq_callback));
	// The Voice uses a speaker with its own volume control so the relative volumes to use are subjective, these sound good
	m_speech->add_route(ALL_OUTPUTS, "mono", 1.00);

	O2_CART_SLOT(config, m_subslot, o2_cart, nullptr);
}

ROM_START( o2voice )
	ROM_REGION( 0x10000, "speech", ROMREGION_ERASE00 )
ROM_END

const tiny_rom_entry *o2_voice_device::device_rom_region() const
{
	return ROM_NAME( o2voice );
}


//-------------------------------------------------
//  mapper specific handlers
//-------------------------------------------------

WRITE_LINE_MEMBER(o2_voice_device::lrq_callback)
{
	m_lrq_state = state;
}

READ_LINE_MEMBER(o2_voice_device::t0_read)
{
	return m_speech->lrq_r() ? 0 : 1;
}

void o2_voice_device::io_write(offs_t offset, u8 data)
{
	if (offset & 0x80 && ~m_control & 0x10)
	{
		// A0-A6: SP0256B A1-A7 (A8 to GND)
		// D5: 7474 to SP0256B reset
		if (data & 0x20)
			m_speech->ald_w(offset & 0x7f);
		else
			m_speech->reset();
	}
}
