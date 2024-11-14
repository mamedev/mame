// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Terak 8510A

2009-02-23 Skeleton driver.

Known chips: i8257 DMA, i8272a FDC
Floppies were 8 inch IBM format.

****************************************************************************/

#include "emu.h"
//#include "bus/qbus/qbus.h"
#include "cpu/t11/t11.h"
#include "emupal.h"
#include "screen.h"


namespace {

class terak_state : public driver_device
{
public:
	terak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void terak(machine_config &config);

private:
	uint16_t terak_fdc_status_r();
	void terak_fdc_command_w(uint16_t data);
	uint16_t terak_fdc_data_r();
	void terak_fdc_data_w(uint16_t data);
	uint32_t screen_update_terak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;

	uint8_t m_unit = 0;
	uint8_t m_cmd = 0;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	required_device<t11_device> m_maincpu;
};

uint16_t terak_state::terak_fdc_status_r()
{
	logerror("terak_fdc_status_r\n");
	if (m_cmd==3)
	{
		logerror("cmd is 3\n");
		return 0xffff;
	}
	return 0;
}

void terak_state::terak_fdc_command_w(uint16_t data)
{
	m_unit = (data >> 8) & 0x03;
	m_cmd  = (data >> 1) & 0x07;
	logerror("terak_fdc_command_w %04x [%d %d]\n",data,m_unit,m_cmd);
}

uint16_t terak_state::terak_fdc_data_r()
{
	logerror("terak_fdc_data_r\n");
	return 0;
}

void terak_state::terak_fdc_data_w(uint16_t data)
{
	logerror("terak_fdc_data_w %04x\n",data);
}

void terak_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xf5ff).ram(); // RAM

	// octal
	map(0173000, 0173177).rom(); // ROM
	map(0177000, 0177001).rw(FUNC(terak_state::terak_fdc_status_r), FUNC(terak_state::terak_fdc_command_w));
	map(0177002, 0177003).rw(FUNC(terak_state::terak_fdc_data_r), FUNC(terak_state::terak_fdc_data_w));
}

/* Input ports */
static INPUT_PORTS_START( terak )
INPUT_PORTS_END


void terak_state::machine_reset()
{
}

void terak_state::video_start()
{
}

uint32_t terak_state::screen_update_terak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void terak_state::terak(machine_config &config)
{
	/* basic machine hardware */
	T11(config, m_maincpu, 4'000'000); // FIXME: actually LSI-11
	m_maincpu->set_initial_mode(6 << 13);
	m_maincpu->set_addrmap(AS_PROGRAM, &terak_state::mem_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(terak_state::screen_update_terak));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

/* ROM definition */
ROM_START( terak )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "terak.rom", 0173000, 0x0080, CRC(fd654b8e) SHA1(273a9933b68a290c5aedcd6d69faa7b1d22c0344))

	ROM_REGION( 0x2000, "kbd", 0)
	// keytronic keyboard, roms are unlabelled, type 6301-1J. CPU is 30293E-003. No crystal.
	ROM_LOAD( "82s129.z2", 0x0000, 0x0100, CRC(a5dce419) SHA1(819197a03eb9b6ea3318f5afc37c0b436dd747a7) )
	ROM_LOAD( "82s129.z1", 0x0100, 0x0100, CRC(f34e061f) SHA1(3cb354b2680056d4b3234c680958d4591279ac8a) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME       FLAGS
COMP( 1977, terak, 0,      0,      terak,   terak, terak_state, empty_init, "Terak", "Terak 8510A", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
