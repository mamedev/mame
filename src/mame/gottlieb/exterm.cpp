// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari,Aaron Giles
/****************************************************************************

    Premier Technology (Gottlieb) Exterminator hardware

    driver by Zsolt Vasvari and Alex Pasadyn

    Premier Technology was the successor to Mylstar Electronics in the mid-80s.
    It still says Gottlieb on the title screen and the cabinet, but that's
    just a marketing brand name.

*****************************************************************************

    Master CPU (TMS34010, all addresses are in bits)

    ------00 0---xxxx xxxxxxxx xxxxxxxx = Background VRAM
    ------00 1-xxxxxx xxxxxxxx xxxxxxxx = Master GSP DRAM
    ------01 000000-- -------- ----xxxx = Slave HSTADRL
    ------01 000100-- -------- ----xxxx = Slave HSTADRH
    ------01 001000-- -------- ----xxxx = Slave HSTDATA
    ------01 001100-- -------- ----xxxx = Slave HSTCTL
    ------01 010000-- -------- ----xxxx = IP0S
    ------01 010001-- -------- ----xxxx = IP1S
    ------01 010010-- -------- ----xxxx = IP2S
    ------01 010100-- -------- ----xxxx = OP0S
    ------01 010110-- -------- ----xxxx = SOUND
    ------01 010111-- -------- ----xxxx = WDOG
    ------01 1------- -xxxxxxx xxxxxxxx = CLUT
    ------10 1------- -xxxxxxx xxxxxxxx = EEPROM
    ------11 xxxxxxxx xxxxxxxx xxxxxxxx = EPROM

    --------------------------------------------------------------------

    Slave CPU (TMS34010, all addresses are in bits)
    -----0-- ----xxxx xxxxxxxx xxxxxxxx = Foreground VRAM
    -----1-- -0xxxxxx xxxxxxxx xxxxxxxx = Slave DRAM bank 1
    -----1-- -1xxxxxx xxxxxxxx xxxxxxxx = Slave DRAM bank 0

    --------------------------------------------------------------------

    Master sound CPU (6502)

    000--xxx xxxxxxxx = RAM
    010----- -------- = YM2151 data write
    01100--- -------- = set NMI down counter
    01101--- -------- = read input latch and clear IRQ
    01110--- -------- = send NMI to slave sound CPU
    01111--- -------- = connected to S4-13 (unknown)
    101----- -------- = sound control register
                            D7 = to S4-15
                            D6 = to S4-12
                            D5 = to S4-11
                            D1 = to LED
                            D0 = enable NMI timer
    1xxxxxxx xxxxxxxx = ROM

    --------------------------------------------------------------------

    Slave sound CPU (6502)

    00---xxx xxxxxxxx = RAM
    01------ -------- = read input latch and clear IRQ
    10------ -------x = DAC write
    1xxxxxxx xxxxxxxx = ROM

****************************************************************************/

#include "emu.h"
#include "gottlieb_a.h"

#include "cpu/m6502/m6502.h"
#include "cpu/tms34010/tms34010.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class exterm_state : public driver_device
{
public:
	exterm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slave(*this, "slave"),
		m_master_videoram(*this, "master_videoram"),
		m_slave_videoram(*this, "slave_videoram"),
		m_dial(*this, "DIAL%u", 0U),
		m_input(*this, "P%u", 1U)
	{ }

	void exterm(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<tms34010_device> m_maincpu;
	required_device<tms34010_device> m_slave;
	required_shared_ptr<uint16_t> m_master_videoram;
	required_shared_ptr<uint16_t> m_slave_videoram;
	required_ioport_array<2> m_dial;
	required_ioport_array<2> m_input;

	uint8_t m_aimpos[2]{};
	uint8_t m_trackball_old[2]{};
	uint16_t m_last = 0U;

	void host_data_w(offs_t offset, uint16_t data);
	uint16_t host_data_r(offs_t offset);
	template<uint8_t Which> uint16_t trackball_port_r();
	void output_port_0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void exterm_palette(palette_device &palette) const;
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg_master);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg_master);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg_slave);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg_slave);

	void master_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Initialization
 *
 *************************************/

void exterm_state::machine_start()
{
	save_item(NAME(m_aimpos));
	save_item(NAME(m_trackball_old));
	save_item(NAME(m_last));
}



/*************************************
 *
 *  Palette setup
 *
 *************************************/

void exterm_state::exterm_palette(palette_device &palette) const
{
	// initialize 555 RGB lookup
	for (int i = 0; i < 32768; i++)
		palette.set_pen_color(i + 0x800, pal5bit(i >> 10), pal5bit(i >> 5), pal5bit(i >> 0));
}



/*************************************
 *
 *  Master shift register
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(exterm_state::to_shiftreg_master)
{
	memcpy(shiftreg, &m_master_videoram[address >> 4], 256 * sizeof(uint16_t));
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(exterm_state::from_shiftreg_master)
{
	memcpy(&m_master_videoram[address >> 4], shiftreg, 256 * sizeof(uint16_t));
}


TMS340X0_TO_SHIFTREG_CB_MEMBER(exterm_state::to_shiftreg_slave)
{
	memcpy(shiftreg, &m_slave_videoram[address >> 4], 256 * 2 * sizeof(uint8_t));
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(exterm_state::from_shiftreg_slave)
{
	memcpy(&m_slave_videoram[address >> 4], shiftreg, 256 * 2 * sizeof(uint8_t));
}



/*************************************
 *
 *  Main video refresh
 *
 *************************************/

TMS340X0_SCANLINE_IND16_CB_MEMBER(exterm_state::scanline_update)
{
	uint16_t *const bgsrc = &m_master_videoram[(params->rowaddr << 8) & 0xff00];
	uint16_t *const dest = &bitmap.pix(scanline);
	tms340x0_device::display_params fgparams;
	int coladdr = params->coladdr;
	int fgcoladdr = 0;

	/* get parameters for the slave CPU */
	m_slave->get_display_params(&fgparams);

	/* compute info about the slave vram */
	uint16_t *fgsrc = nullptr;
	if (fgparams.enabled && scanline >= fgparams.veblnk && scanline < fgparams.vsblnk && fgparams.heblnk < fgparams.hsblnk)
	{
		fgsrc = &m_slave_videoram[((fgparams.rowaddr << 8) + (fgparams.yoffset << 7)) & 0xff80];
		fgcoladdr = (fgparams.coladdr >> 1);
	}

	/* copy the non-blanked portions of this scanline */
	for (int x = params->heblnk; x < params->hsblnk; x += 2)
	{
		uint16_t bgdata, fgdata = 0;

		if (fgsrc != nullptr)
			fgdata = fgsrc[fgcoladdr++ & 0x7f];

		bgdata = bgsrc[coladdr++ & 0xff];
		if ((bgdata & 0xe000) == 0xe000)
			dest[x + 0] = bgdata & 0x7ff;
		else if ((fgdata & 0x00ff) != 0)
			dest[x + 0] = fgdata & 0x00ff;
		else
			dest[x + 0] = (bgdata & 0x8000) ? (bgdata & 0x7ff) : (bgdata + 0x800);

		bgdata = bgsrc[coladdr++ & 0xff];
		if ((bgdata & 0xe000) == 0xe000)
			dest[x + 1] = bgdata & 0x7ff;
		else if ((fgdata & 0xff00) != 0)
			dest[x + 1] = fgdata >> 8;
		else
			dest[x + 1] = (bgdata & 0x8000) ? (bgdata & 0x7ff) : (bgdata + 0x800);
	}
}



/*************************************
 *
 *  Master/slave communications
 *
 *************************************/

void exterm_state::host_data_w(offs_t offset, uint16_t data)
{
	m_slave->host_w(offset / 0x0010000, data);
}


uint16_t exterm_state::host_data_r(offs_t offset)
{
	return m_slave->host_r(offset / 0x0010000);
}



/*************************************
 *
 *  Input port handlers
 *
 *************************************/

template<uint8_t Which>
uint16_t exterm_state::trackball_port_r()
{
	uint16_t port;

	/* Read the fake input port */
	uint8_t trackball_pos = m_dial[Which]->read();

	/* Calculate the change from the last position. */
	uint8_t trackball_diff = m_trackball_old[Which] - trackball_pos;

	/* Store the new position for the next comparison. */
	m_trackball_old[Which] = trackball_pos;

	/* Move the sign bit to the high bit of the 6-bit trackball count. */
	if (trackball_diff & 0x80)
		trackball_diff |= 0x20;

	/* Keep adding the changes.  The counters will be reset later by a hardware write. */
	m_aimpos[Which] = (m_aimpos[Which] + trackball_diff) & 0x3f;

	/* Combine it with the standard input bits */
	port = m_input[Which]->read();

	return (port & 0xc0ff) | (m_aimpos[Which] << 8);
}



/*************************************
 *
 *  Output port handlers
 *
 *************************************/

void exterm_state::output_port_0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* All the outputs are activated on the rising edge */

	if (ACCESSING_BITS_0_7)
	{
		/* Bit 0-1= Resets analog controls */
		if ((data & 0x0001) && !(m_last & 0x0001))
			m_aimpos[0] = 0;
		if ((data & 0x0002) && !(m_last & 0x0002))
			m_aimpos[1] = 0;
	}

	if (ACCESSING_BITS_8_15)
	{
		/* Bit 13 = Resets the slave CPU */
		if ((data & 0x2000) && !(m_last & 0x2000))
			m_slave->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

		/* Bits 14-15 = Coin counters */
		machine().bookkeeping().coin_counter_w(0, data & 0x8000);
		machine().bookkeeping().coin_counter_w(1, data & 0x4000);
	}

	COMBINE_DATA(&m_last);
}



/*************************************
 *
 *  Master/slave memory maps
 *
 *************************************/

void exterm_state::master_map(address_map &map)
{
	map(0x00000000, 0x000fffff).mirror(0xfc700000).ram().share("master_videoram");
	map(0x00800000, 0x00bfffff).mirror(0xfc400000).ram();
	map(0x01000000, 0x013fffff).mirror(0xfc000000).rw(FUNC(exterm_state::host_data_r), FUNC(exterm_state::host_data_w));
	map(0x01400000, 0x0143ffff).mirror(0xfc000000).r(FUNC(exterm_state::trackball_port_r<0>));
	map(0x01440000, 0x0147ffff).mirror(0xfc000000).r(FUNC(exterm_state::trackball_port_r<1>));
	map(0x01480000, 0x014bffff).mirror(0xfc000000).portr("DSW");
	map(0x01500000, 0x0153ffff).mirror(0xfc000000).w(FUNC(exterm_state::output_port_0_w));
	map(0x01580000, 0x015bffff).mirror(0xfc000000).w("p5sound", FUNC(gottlieb_sound_p5_device::write)).umask16(0x00ff);
	map(0x015c0000, 0x015fffff).mirror(0xfc000000).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x01800000, 0x01807fff).mirror(0xfc7f8000).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x02800000, 0x02807fff).mirror(0xfc7f8000).ram().share("nvram");
	map(0x03000000, 0x03ffffff).mirror(0xfc000000).rom().region("maincpu", 0);
}


void exterm_state::slave_map(address_map &map)
{
	map(0x00000000, 0x000fffff).mirror(0xfbf00000).ram().share("slave_videoram");
	map(0x04000000, 0x047fffff).mirror(0xfb800000).ram();
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( exterm )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x3f00, IP_ACTIVE_LOW, IPT_CUSTOM) /* trackball data */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x3f00, IP_ACTIVE_LOW, IPT_CUSTOM) /* trackball data */
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unused ) ) /* According to the test screen */
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	/* Note that the coin settings don't match the setting shown on the test screen,
	   but instead what the game appears to used. This is either a bug in the game,
	   or I don't know what else. */
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Memory Test" )
	PORT_DIPSETTING(      0x0040, "Once" )
	PORT_DIPSETTING(      0x0000, "Continuous" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIAL0") /* Fake trackball input port */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X)

	PORT_START("DIAL1") /* Fake trackball input port. */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M)
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void exterm_state::exterm(machine_config &config)
{
	/* basic machine hardware */
	TMS34010(config, m_maincpu, 40000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &exterm_state::master_map);
	m_maincpu->set_halt_on_reset(false);
	m_maincpu->set_pixel_clock(40000000/8);
	m_maincpu->set_pixels_per_clock(1);
	m_maincpu->set_scanline_ind16_callback(FUNC(exterm_state::scanline_update));
	m_maincpu->set_shiftreg_in_callback(FUNC(exterm_state::to_shiftreg_master));
	m_maincpu->set_shiftreg_out_callback(FUNC(exterm_state::from_shiftreg_master));

	TMS34010(config, m_slave, 40000000);
	m_slave->set_addrmap(AS_PROGRAM, &exterm_state::slave_map);
	m_slave->set_halt_on_reset(true);
	m_slave->set_pixel_clock(40000000/8);
	m_slave->set_pixels_per_clock(1);
	m_slave->set_shiftreg_in_callback(FUNC(exterm_state::to_shiftreg_slave));
	m_slave->set_shiftreg_out_callback(FUNC(exterm_state::from_shiftreg_slave));

	config.set_maximum_quantum(attotime::from_hz(6000));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	PALETTE(config, "palette", FUNC(exterm_state::exterm_palette)).set_format(palette_device::xRGB_555, 2048+32768);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(40000000/8, 318, 0, 256, 264, 0, 240);
	screen.set_screen_update("maincpu", FUNC(tms34010_device::tms340x0_ind16));
	screen.set_palette("palette");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	GOTTLIEB_SOUND_PIN5(config, "p5sound").add_route(ALL_OUTPUTS, "mono", 1.00);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( exterm )
	ROM_REGION16_LE( 0x200000, "maincpu", 0 ) // 2MB for 34010 code
	ROM_LOAD16_BYTE( "v101bg0",  0x000000, 0x10000, CRC(8c8e72cf) SHA1(5e0fa805334f54f7e0293ea400bacb0e3e79ed56) )
	ROM_LOAD16_BYTE( "v101bg1",  0x000001, 0x10000, CRC(cc2da0d8) SHA1(4ac23048d3ca771e315388603ad3b1b25030d6ff) )
	ROM_LOAD16_BYTE( "v101bg2",  0x020000, 0x10000, CRC(2dcb3653) SHA1(2d74b58b02ae0587e3789d69feece268f582f226) )
	ROM_LOAD16_BYTE( "v101bg3",  0x020001, 0x10000, CRC(4aedbba0) SHA1(73b7e4864b1e71103229edd3cae268ab91144ef2) )
	ROM_LOAD16_BYTE( "v101bg4",  0x040000, 0x10000, CRC(576922d4) SHA1(c8cdfb0727c9f1f6e2d2008611372f386fd35fc4) )
	ROM_LOAD16_BYTE( "v101bg5",  0x040001, 0x10000, CRC(a54a4bc2) SHA1(e0f3648454cafeee1f3f58af03489d3256f66965) )
	ROM_LOAD16_BYTE( "v101bg6",  0x060000, 0x10000, CRC(7584a676) SHA1(c9bc651f90ab752f73e735cb80e5bb109e2cac5f) )
	ROM_LOAD16_BYTE( "v101bg7",  0x060001, 0x10000, CRC(a4f24ff6) SHA1(adabbe1c93beb4fcc6fa2f13e687a866fb54fbdb) )
	ROM_LOAD16_BYTE( "v101bg8",  0x080000, 0x10000, CRC(fda165d6) SHA1(901bdede00a936c0160d9fea8a2975ff893e52d0) )
	ROM_LOAD16_BYTE( "v101bg9",  0x080001, 0x10000, CRC(e112a4c4) SHA1(8938d6857b3c5cd3f5560496e087e3b3ff3dab81) )
	ROM_LOAD16_BYTE( "v101bg10", 0x0a0000, 0x10000, CRC(f1a5cf54) SHA1(749531036a1100e092b7edfba14097d5aaab26aa) )
	ROM_LOAD16_BYTE( "v101bg11", 0x0a0001, 0x10000, CRC(8677e754) SHA1(dd8135de8819096150914798ab37a17ae396af32) )
	ROM_LOAD16_BYTE( "v101fg0",  0x180000, 0x10000, CRC(38230d7d) SHA1(edd575192c0376183c415c61a3c3f19555522549) )
	ROM_LOAD16_BYTE( "v101fg1",  0x180001, 0x10000, CRC(22a2bd61) SHA1(59ed479b8ae8328014be4e2a5575d00105fd83f3) )
	ROM_LOAD16_BYTE( "v101fg2",  0x1a0000, 0x10000, CRC(9420e718) SHA1(1fd9784d40e496ebc4772baff472eb25b5106725) )
	ROM_LOAD16_BYTE( "v101fg3",  0x1a0001, 0x10000, CRC(84992aa2) SHA1(7dce2bef695c2a9b5a03d217bbff8fbece459a92) )
	ROM_LOAD16_BYTE( "v101fg4",  0x1c0000, 0x10000, CRC(38da606b) SHA1(59479ff99b1748ddc36de32b368dd38cb2965868) )
	ROM_LOAD16_BYTE( "v101fg5",  0x1c0001, 0x10000, CRC(842de63a) SHA1(0b292a8b7f4b86a2d3bd6b5b7ec0287e2bf88263) )
	ROM_LOAD16_BYTE( "v101p0",   0x1e0000, 0x10000, CRC(6c8ee79a) SHA1(aa051e33e3ed6eed475a37e5dae1be0ac6471b12) )
	ROM_LOAD16_BYTE( "v101p1",   0x1e0001, 0x10000, CRC(557bfc84) SHA1(8d0f1b40adbf851a85f626663956f3726ca8026d) )

	ROM_REGION( 0x10000, "p5sound:audiocpu", 0 ) // 64k for DAC code
	ROM_LOAD( "v101d1", 0x8000, 0x8000, CRC(83268b7d) SHA1(a9139e80e2382122e9919c0555937e120d4414cf) )

	ROM_REGION( 0x10000, "p5sound:speechcpu", 0 ) // 64k for YM2151 code
	ROM_LOAD( "v101y1", 0x8000, 0x8000, CRC(cbeaa837) SHA1(87d8a258f059512dbf9bc0e7cfff728ef9e616f1) )
ROM_END

} // anonymous namespace



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1989, exterm, 0, exterm, exterm, exterm_state, empty_init, ROT0, "Premier Technology", "Exterminator", MACHINE_SUPPORTS_SAVE )
