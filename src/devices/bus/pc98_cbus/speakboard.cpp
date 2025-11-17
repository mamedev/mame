// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Idol Japan SpeakBoard sound card

Predates -86/-73, first actual card to support YM2608

https://j02.nobody.jp/jto98/n_desk_sound/msb.htm

TODO:
- check ADPCM (hplus doesn't seem to support it);
- PnP;
- stereo line-in;
- Surround VR;
- EMS;
- SparkBoard derives from this (extra OPNA at $588)

===================================================================================================

- Known SW with SpeakBoard support
  hplus
  Lord Monarch demo (undumped? Support dropped in retail version)
  dedicated sound editor (undumped?)

**************************************************************************************************/

#include "emu.h"
#include "speakboard.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(SPEAKBOARD, speakboard_device, "speakboard", "Idol Japan SpeakBoard sound card")

speakboard_device::speakboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPEAKBOARD, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_opna(*this, "opna")
	, m_bios(*this, "bios")
	, m_joy(*this, "joy_port")
{
}

ROM_START( speakboard )
	ROM_REGION( 0x4000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "spb lh5764 ic21_pink.bin",    0x0001, 0x2000, CRC(5bcefa1f) SHA1(ae88e45d411bf5de1cb42689b12b6fca0146c586) )
	ROM_LOAD16_BYTE( "spb lh5764 ic22_green.bin",   0x0000, 0x2000, CRC(a7925ced) SHA1(3def9ee386ab6c31436888261bded042cd64a0eb) )
ROM_END

const tiny_rom_entry *speakboard_device::device_rom_region() const
{
	return ROM_NAME( speakboard );
}

void speakboard_device::opna_map(address_map &map)
{
	// 256KB
	map(0x000000, 0x03ffff).ram();
}


void speakboard_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	YM2608(config, m_opna, 7.987_MHz_XTAL); // actually YM2608B
	m_opna->set_addrmap(0, &speakboard_device::opna_map);
	m_opna->irq_handler().set([this] (int state) {
		//const int int_levels[4] = { 0, 4, 6, 5 };
		m_bus->int_w(5, state);
	});
	m_opna->port_a_read_callback().set([this] () {
		if((m_joy_sel & 0xc0) == 0x80)
			return m_joy->read();

		return (u8)0xff;
	});
	m_opna->port_b_write_callback().set([this] (u8 data) {
		m_joy_sel = data;
	});
	// TODO: confirm mixing
	m_opna->add_route(0, "speaker", 0.25, 0);
	m_opna->add_route(0, "speaker", 0.25, 1);
	m_opna->add_route(1, "speaker", 0.50, 0);
	m_opna->add_route(2, "speaker", 0.50, 1);

	// NOTE: 1x DE-9 port only
	MSX_GENERAL_PURPOSE_PORT(config, m_joy, msx_general_purpose_port_devices, "joystick");
}

void speakboard_device::device_start()
{
}

void speakboard_device::device_reset()
{
}

void speakboard_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		logerror("map ROM at 0x000cc000-0x000cffff\n");
		m_bus->space(AS_PROGRAM).install_rom(
			0xcc000,
			0xcffff,
			m_bios->base()
		);
	}
	else if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0xffff, *this, &speakboard_device::io_map);
	}
}


void speakboard_device::io_map(address_map &map)
{
	// TODO: PnP (0x0088~0x0188 only?)
	const u16 io_base = 0x100; //read_io_base();

	map(0x0088 + io_base, 0x008f + io_base).rw(m_opna, FUNC(ym2608_device::read), FUNC(ym2608_device::write)).umask16(0x00ff);
}
