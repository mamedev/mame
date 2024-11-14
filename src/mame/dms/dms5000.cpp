// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Digital Microsystems DMS-5000

2010-01-11 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/74259.h"
//#include "machine/z80sio.h"
#include "emupal.h"
#include "screen.h"


namespace {

class dms5000_state : public driver_device
{
public:
	dms5000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{
	}

	void dms5000(machine_config &config);
private:
	uint8_t status_r(offs_t offset);
	void brightness_w(uint8_t data);
	uint32_t screen_update_dms5000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};


uint8_t dms5000_state::status_r(offs_t offset)
{
	switch (offset)
	{
	case 0:
		return m_screen->vblank();

	case 2:
		return m_screen->frame_number() & 1;

	default:
		return 0;
	}
}

void dms5000_state::brightness_w(uint8_t data)
{
}

void dms5000_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x1ffff).ram();
	map(0x40000, 0x4ffff).ram();
	map(0xfc000, 0xfffff).rom().region("maincpu", 0);
}

void dms5000_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x20, 0x2f).r(FUNC(dms5000_state::status_r)).umask16(0xff00);
	map(0x40, 0x4f).w("cntlatch", FUNC(ls259_device::write_d0)).umask16(0x00ff);
	map(0x50, 0x57).w(FUNC(dms5000_state::brightness_w)).umask16(0x00ff);
}

/* Input ports */
static INPUT_PORTS_START( dms5000 )
INPUT_PORTS_END


uint32_t dms5000_state::screen_update_dms5000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void dms5000_state::dms5000(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, XTAL(9'830'400));
	m_maincpu->set_addrmap(AS_PROGRAM, &dms5000_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &dms5000_state::io_map);

	LS259(config, "cntlatch", 0); // V34

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(640, 480);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(dms5000_state::screen_update_dms5000));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

/* ROM definition */
ROM_START( dms5000 )
	ROM_REGION16_LE( 0x4000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dms-5000_54-8673o.bin", 0x0001, 0x2000, CRC(dce9823e) SHA1(d36ab87d2e6f5e9f02d59a6a7724ad3ce2428a2f))
	ROM_LOAD16_BYTE( "dms-5000_54-8672e.bin", 0x0000, 0x2000, CRC(94d64c06) SHA1(be5a53da7bb29a5fa9ac31efe550d5d6ff8b77cd))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                 FULLNAME    FLAGS */
COMP( 1982, dms5000, 0,      0,      dms5000, dms5000, dms5000_state, empty_init, "Digital Microsystems", "DMS-5000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
