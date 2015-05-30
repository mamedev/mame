// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari,Aaron Giles
/****************************************************************************

    Gottlieb Exterminator hardware

    driver by Zsolt Vasvari and Alex Pasadyn

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
#include "cpu/tms34010/tms34010.h"
#include "cpu/m6502/m6502.h"
#include "sound/dac.h"
#include "sound/2151intf.h"
#include "machine/nvram.h"
#include "includes/exterm.h"



/*************************************
 *
 *  Master/slave communications
 *
 *************************************/

WRITE16_MEMBER(exterm_state::exterm_host_data_w)
{
	m_slave->host_w(space,offset / TOWORD(0x00100000), data, 0xffff);
}


READ16_MEMBER(exterm_state::exterm_host_data_r)
{
	return m_slave->host_r(space,offset / TOWORD(0x00100000), 0xffff);
}



/*************************************
 *
 *  Input port handlers
 *
 *************************************/

UINT16 exterm_state::exterm_trackball_port_r(int which, UINT16 mem_mask)
{
	UINT16 port;

	/* Read the fake input port */
	UINT8 trackball_pos = ioport(which ? "DIAL1" : "DIAL0")->read();

	/* Calculate the change from the last position. */
	UINT8 trackball_diff = m_trackball_old[which] - trackball_pos;

	/* Store the new position for the next comparision. */
	m_trackball_old[which] = trackball_pos;

	/* Move the sign bit to the high bit of the 6-bit trackball count. */
	if (trackball_diff & 0x80)
		trackball_diff |= 0x20;

	/* Keep adding the changes.  The counters will be reset later by a hardware write. */
	m_aimpos[which] = (m_aimpos[which] + trackball_diff) & 0x3f;

	/* Combine it with the standard input bits */
	port = ioport(which ? "P2" : "P1")->read();

	return (port & 0xc0ff) | (m_aimpos[which] << 8);
}


READ16_MEMBER(exterm_state::exterm_input_port_0_r)
{
	return exterm_trackball_port_r(0, mem_mask);
}


READ16_MEMBER(exterm_state::exterm_input_port_1_r)
{
	return exterm_trackball_port_r(1, mem_mask);
}



/*************************************
 *
 *  Output port handlers
 *
 *************************************/

WRITE16_MEMBER(exterm_state::exterm_output_port_0_w)
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
			m_slave->set_input_line(INPUT_LINE_RESET, PULSE_LINE);

		/* Bits 14-15 = Coin counters */
		coin_counter_w(machine(), 0, data & 0x8000);
		coin_counter_w(machine(), 1, data & 0x4000);
	}

	COMBINE_DATA(&m_last);
}


TIMER_CALLBACK_MEMBER(exterm_state::sound_delayed_w)
{
	/* data is latched independently for both sound CPUs */
	m_master_sound_latch = m_slave_sound_latch = param;
	m_audiocpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
	m_audioslave->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
}


WRITE16_MEMBER(exterm_state::sound_latch_w)
{
	if (ACCESSING_BITS_0_7)
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(exterm_state::sound_delayed_w),this), data & 0xff);
}



/*************************************
 *
 *  Sound handlers
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(exterm_state::master_sound_nmi_callback)
{
	/* bit 0 of the sound control determines if the NMI is actually delivered */
	if (m_sound_control & 0x01)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


WRITE8_MEMBER(exterm_state::ym2151_data_latch_w)
{
	ym2151_device *device = machine().device<ym2151_device>("ymsnd");
	/* bit 7 of the sound control selects which port */
	device->write(space, m_sound_control >> 7, data);
}


WRITE8_MEMBER(exterm_state::sound_nmi_rate_w)
{
	/* rate is controlled by the value written here */
	/* this value is latched into up-counters, which are clocked at the */
	/* input clock / 256 */
	attotime nmi_rate = attotime::from_hz(4000000) * (4096 * (256 - data));
	timer_device *nmi_timer = machine().device<timer_device>("snd_nmi_timer");
	nmi_timer->adjust(nmi_rate, 0, nmi_rate);
}


READ8_MEMBER(exterm_state::sound_master_latch_r)
{
	/* read latch and clear interrupt */
	m_audiocpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	return m_master_sound_latch;
}


READ8_MEMBER(exterm_state::sound_slave_latch_r)
{
	/* read latch and clear interrupt */
	m_audioslave->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	return m_slave_sound_latch;
}


WRITE8_MEMBER(exterm_state::sound_slave_dac_w)
{
	/* DAC A is used to modulate DAC B */
	m_dac_value[offset & 1] = data;
	m_dac->write_unsigned16((m_dac_value[0] ^ 0xff) * m_dac_value[1]);
}


READ8_MEMBER(exterm_state::sound_nmi_to_slave_r)
{
	/* a read from here triggers an NMI pulse to the slave */
	m_audioslave->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	return 0xff;
}


WRITE8_MEMBER(exterm_state::sound_control_w)
{
/*
    D7 = to S4-15
    D6 = to S4-12
    D5 = to S4-11
    D1 = to LED
    D0 = enable NMI timer
*/
	m_sound_control = data;
}



/*************************************
 *
 *  Master/slave memory maps
 *
 *************************************/

static ADDRESS_MAP_START( master_map, AS_PROGRAM, 16, exterm_state )
	AM_RANGE(0xc0000000, 0xc00001ff) AM_DEVREADWRITE("maincpu", tms34010_device, io_register_r, io_register_w)
	AM_RANGE(0x00000000, 0x000fffff) AM_MIRROR(0xfc700000) AM_RAM AM_SHARE("master_videoram")
	AM_RANGE(0x00800000, 0x00bfffff) AM_MIRROR(0xfc400000) AM_RAM
	AM_RANGE(0x01000000, 0x013fffff) AM_MIRROR(0xfc000000) AM_READWRITE(exterm_host_data_r, exterm_host_data_w)
	AM_RANGE(0x01400000, 0x0143ffff) AM_MIRROR(0xfc000000) AM_READ(exterm_input_port_0_r)
	AM_RANGE(0x01440000, 0x0147ffff) AM_MIRROR(0xfc000000) AM_READ(exterm_input_port_1_r)
	AM_RANGE(0x01480000, 0x014bffff) AM_MIRROR(0xfc000000) AM_READ_PORT("DSW")
	AM_RANGE(0x01500000, 0x0153ffff) AM_MIRROR(0xfc000000) AM_WRITE(exterm_output_port_0_w)
	AM_RANGE(0x01580000, 0x015bffff) AM_MIRROR(0xfc000000) AM_WRITE(sound_latch_w)
	AM_RANGE(0x015c0000, 0x015fffff) AM_MIRROR(0xfc000000) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x01800000, 0x01807fff) AM_MIRROR(0xfc7f8000) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x02800000, 0x02807fff) AM_MIRROR(0xfc7f8000) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x03000000, 0x03ffffff) AM_MIRROR(0xfc000000) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END


static ADDRESS_MAP_START( slave_map, AS_PROGRAM, 16, exterm_state )
	AM_RANGE(0xc0000000, 0xc00001ff) AM_DEVREADWRITE("slave", tms34010_device, io_register_r, io_register_w)
	AM_RANGE(0x00000000, 0x000fffff) AM_MIRROR(0xfbf00000) AM_RAM AM_SHARE("slave_videoram")
	AM_RANGE(0x04000000, 0x047fffff) AM_MIRROR(0xfb800000) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Audio memory maps
 *
 *************************************/

static ADDRESS_MAP_START( sound_master_map, AS_PROGRAM, 8, exterm_state )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0x4000, 0x5fff) AM_WRITE(ym2151_data_latch_w)
	AM_RANGE(0x6000, 0x67ff) AM_WRITE(sound_nmi_rate_w)
	AM_RANGE(0x6800, 0x6fff) AM_READ(sound_master_latch_r)
	AM_RANGE(0x7000, 0x77ff) AM_READ(sound_nmi_to_slave_r)
/*  AM_RANGE(0x7800, 0x7fff) unknown - to S4-13 */
	AM_RANGE(0xa000, 0xbfff) AM_WRITE(sound_control_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_slave_map, AS_PROGRAM, 8, exterm_state )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x3800) AM_RAM
	AM_RANGE(0x4000, 0x5fff) AM_READ(sound_slave_latch_r)
	AM_RANGE(0x8000, 0xbfff) AM_WRITE(sound_slave_dac_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



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
	PORT_BIT( 0x3f00, IP_ACTIVE_LOW, IPT_SPECIAL) /* trackball data */
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
	PORT_BIT( 0x3f00, IP_ACTIVE_LOW, IPT_SPECIAL) /* trackball data */
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

static MACHINE_CONFIG_START( exterm, exterm_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS34010, 40000000)
	MCFG_CPU_PROGRAM_MAP(master_map)
	MCFG_TMS340X0_HALT_ON_RESET(FALSE) /* halt on reset */
	MCFG_TMS340X0_PIXEL_CLOCK(40000000/8) /* pixel clock */
	MCFG_TMS340X0_PIXELS_PER_CLOCK(1) /* pixels per clock */
	MCFG_TMS340X0_SCANLINE_IND16_CB(exterm_state, scanline_update)     /* scanline updater (indexed16) */
	MCFG_TMS340X0_TO_SHIFTREG_CB(exterm_state, to_shiftreg_master)  /* write to shiftreg function */
	MCFG_TMS340X0_FROM_SHIFTREG_CB(exterm_state, from_shiftreg_master) /* read from shiftreg function */

	MCFG_CPU_ADD("slave", TMS34010, 40000000)
	MCFG_CPU_PROGRAM_MAP(slave_map)
	MCFG_TMS340X0_HALT_ON_RESET(TRUE) /* halt on reset */
	MCFG_TMS340X0_PIXEL_CLOCK(40000000/8) /* pixel clock */
	MCFG_TMS340X0_PIXELS_PER_CLOCK(1) /* pixels per clock */
	MCFG_TMS340X0_TO_SHIFTREG_CB(exterm_state, to_shiftreg_slave)   /* write to shiftreg function */
	MCFG_TMS340X0_FROM_SHIFTREG_CB(exterm_state, from_shiftreg_slave)  /* read from shiftreg function */

	MCFG_CPU_ADD("audiocpu", M6502, 2000000)
	MCFG_CPU_PROGRAM_MAP(sound_master_map)

	MCFG_CPU_ADD("audioslave", M6502, 2000000)
	MCFG_CPU_PROGRAM_MAP(sound_slave_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_TIMER_DRIVER_ADD("snd_nmi_timer", exterm_state, master_sound_nmi_callback)

	/* video hardware */
	MCFG_PALETTE_ADD("palette", 2048+32768)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)
	MCFG_PALETTE_INIT_OWNER(exterm_state, exterm)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(40000000/8, 318, 0, 256, 264, 0, 240)
	MCFG_SCREEN_UPDATE_DEVICE("maincpu", tms34010_device, tms340x0_ind16)
	MCFG_SCREEN_PALETTE("palette")


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_YM2151_ADD("ymsnd", 4000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( exterm )
	ROM_REGION( 0x10000, "audiocpu", 0 )        /* 64k for YM2151 code */
	ROM_LOAD( "v101y1", 0x8000, 0x8000, CRC(cbeaa837) SHA1(87d8a258f059512dbf9bc0e7cfff728ef9e616f1) )

	ROM_REGION( 0x10000, "audioslave", 0 )      /* 64k for DAC code */
	ROM_LOAD( "v101d1", 0x8000, 0x8000, CRC(83268b7d) SHA1(a9139e80e2382122e9919c0555937e120d4414cf) )

	ROM_REGION16_LE( 0x200000, "user1", 0 ) /* 2MB for 34010 code */
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
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1989, exterm, 0, exterm, exterm, driver_device, 0, ROT0, "Gottlieb / Premier Technology", "Exterminator", 0 )
