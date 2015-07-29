// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "includes/cidelsa.h"

/* CDP1802 Interface */

READ_LINE_MEMBER( cidelsa_state::clear_r )
{
	return m_reset;
}

WRITE_LINE_MEMBER( cidelsa_state::q_w )
{
	m_cdp1802_q = state;
}

/* Sound Interface */

WRITE8_MEMBER( draco_state::sound_bankswitch_w )
{
	/*

	    pin     description

	    D0      not connected
	    D1      not connected
	    D2      not connected
	    D3      2716 A10

	*/

	int bank = BIT(data, 3);

	membank("bank1")->set_entry(bank);
}

WRITE8_MEMBER( draco_state::sound_g_w )
{
	/*

	 G1 G0  description

	  0  0  IAB     inactive
	  0  1  DWS     write to PSG
	  1  0  DTB     read from PSG
	  1  1  INTAK   latch address

	*/

	switch (data)
	{
	case 0x01:
		m_psg->data_w(space, 0, m_psg_latch);
		break;

	case 0x02:
		m_psg_latch = m_psg->data_r(space, 0);
		break;

	case 0x03:
		m_psg->address_w(space, 0, m_psg_latch);
		break;
	}
}

READ8_MEMBER( draco_state::sound_in_r )
{
	return ~(m_sound) & 0x07;
}

READ8_MEMBER( draco_state::psg_r )
{
	return m_psg_latch;
}

WRITE8_MEMBER( draco_state::psg_w )
{
	m_psg_latch = data;
}

/* Read/Write Handlers */

WRITE8_MEMBER( cidelsa_state::destryer_out1_w )
{
	/*
	  bit   description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7
	*/
}

/* CDP1852 Interfaces */

WRITE8_MEMBER( cidelsa_state::altair_out1_w )
{
	/*
	  bit   description

	    0   S1 (CARTUCHO)
	    1   S2 (CARTUCHO)
	    2   S3 (CARTUCHO)
	    3   LG1
	    4   LG2
	    5   LGF
	    6   CONT. M2
	    7   CONT. M1
	*/

	set_led_status(machine(), 0, data & 0x08); // 1P
	set_led_status(machine(), 1, data & 0x10); // 2P
	set_led_status(machine(), 2, data & 0x20); // FIRE
}

WRITE8_MEMBER( draco_state::out1_w )
{
	/*
	  bit   description

	    0   3K9 -> Green signal
	    1   820R -> Blue signal
	    2   510R -> Red signal
	    3   1K -> not connected
	    4   not connected
	    5   SONIDO A -> COP402 IN0
	    6   SONIDO B -> COP402 IN1
	    7   SONIDO C -> COP402 IN2
	*/

	m_sound = (data & 0xe0) >> 5;
}

/* Memory Maps */

// Destroyer

static ADDRESS_MAP_START( destryer_map, AS_PROGRAM, 8, cidelsa_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x20ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf400, 0xf7ff) AM_DEVICE(CDP1869_TAG, cdp1869_device, char_map)
	AM_RANGE(0xf800, 0xffff) AM_DEVICE(CDP1869_TAG, cdp1869_device, page_map)
ADDRESS_MAP_END

static ADDRESS_MAP_START( destryera_map, AS_PROGRAM, 8, cidelsa_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x3000, 0x30ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf400, 0xf7ff) AM_DEVICE(CDP1869_TAG, cdp1869_device, char_map)
	AM_RANGE(0xf800, 0xffff) AM_DEVICE(CDP1869_TAG, cdp1869_device, page_map)
ADDRESS_MAP_END

static ADDRESS_MAP_START( destryer_io_map, AS_IO, 8, cidelsa_state )
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN0") AM_WRITE(destryer_out1_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN1")
	AM_RANGE(0x03, 0x07) AM_WRITE(cdp1869_w)
ADDRESS_MAP_END

// Altair

static ADDRESS_MAP_START( altair_map, AS_PROGRAM, 8, cidelsa_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x30ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf400, 0xf7ff) AM_DEVICE(CDP1869_TAG, cdp1869_device, char_map)
	AM_RANGE(0xf800, 0xffff) AM_DEVICE(CDP1869_TAG, cdp1869_device, page_map)
ADDRESS_MAP_END

static ADDRESS_MAP_START( altair_io_map, AS_IO, 8, cidelsa_state )
	AM_RANGE(0x01, 0x01) AM_DEVREAD("ic23", cdp1852_device, read) AM_DEVWRITE("ic26", cdp1852_device, write)
	AM_RANGE(0x02, 0x02) AM_DEVREAD("ic24", cdp1852_device, read)
	AM_RANGE(0x04, 0x04) AM_DEVREAD("ic25", cdp1852_device, read)
	AM_RANGE(0x03, 0x07) AM_WRITE(cdp1869_w)
ADDRESS_MAP_END

// Draco

static ADDRESS_MAP_START( draco_map, AS_PROGRAM, 8, draco_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf400, 0xf7ff) AM_DEVICE(CDP1869_TAG, cdp1869_device, char_map)
	AM_RANGE(0xf800, 0xffff) AM_DEVICE(CDP1869_TAG, cdp1869_device, page_map)
ADDRESS_MAP_END

static ADDRESS_MAP_START( draco_io_map, AS_IO, 8, draco_state )
	AM_RANGE(0x01, 0x01) AM_DEVREAD("ic29", cdp1852_device, read) AM_DEVWRITE("ic32", cdp1852_device, write)
	AM_RANGE(0x02, 0x02) AM_DEVREAD("ic30", cdp1852_device, read)
	AM_RANGE(0x04, 0x04) AM_DEVREAD("ic31", cdp1852_device, read)
	AM_RANGE(0x03, 0x07) AM_WRITE(cdp1869_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( draco_sound_map, AS_PROGRAM, 8, draco_state )
	AM_RANGE(0x000, 0x3ff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

/* Input Ports */

READ_LINE_MEMBER( cidelsa_state::cdp1869_pcb_r )
{
	return m_cdp1869_pcb;
}

static INPUT_PORTS_START( destryer )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // CARTUCHO
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) // 1P
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) // 2P
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // RG
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) // LF
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // FR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, cidelsa_state, cdp1869_pcb_r)

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "Very Conserv" )
	PORT_DIPSETTING(    0x01, "Conserv" )
	PORT_DIPSETTING(    0x02, "Liberal" )
	PORT_DIPSETTING(    0x03, "Very Liberal" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "5000" )
	PORT_DIPSETTING(    0x08, "7000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x00, "14000" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR(Coinage) )
	PORT_DIPSETTING(    0xc0, "Slot A: 1  Slot B: 2" )
	PORT_DIPSETTING(    0x80, "Slot A: 1.5  Slot B: 3" )
	PORT_DIPSETTING(    0x40, "Slot A: 2  Slot B: 4" )
	PORT_DIPSETTING(    0x00, "Slot A: 2.5  Slot B: 5" )

	PORT_START("EF")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) // inverted CDP1869 PRD, pushed
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_WRITE_LINE_DEVICE_MEMBER(CDP1802_TAG, cosmac_device, ef2_w)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_WRITE_LINE_DEVICE_MEMBER(CDP1802_TAG, cosmac_device, ef3_w)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER(CDP1802_TAG, cosmac_device, ef4_w)
INPUT_PORTS_END

static INPUT_PORTS_START( altair )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // CARTUCHO
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) // 1P
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) // 2P
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // RG
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) // LF
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // FR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, cidelsa_state, cdp1869_pcb_r)

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "Very Conserv" )
	PORT_DIPSETTING(    0x01, "Conserv" )
	PORT_DIPSETTING(    0x02, "Liberal" )
	PORT_DIPSETTING(    0x03, "Very Liberal" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "5000" )
	PORT_DIPSETTING(    0x08, "7000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x00, "14000" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR(Coinage) )
	PORT_DIPSETTING(    0xc0, "Slot A: 1  Slot B: 2" )
	PORT_DIPSETTING(    0x80, "Slot A: 1.5  Slot B: 3" )
	PORT_DIPSETTING(    0x40, "Slot A: 2  Slot B: 4" )
	PORT_DIPSETTING(    0x00, "Slot A: 2.5  Slot B: 5" )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) // UP
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) // DN
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) // IN
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EF")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) // inverted CDP1869 PRD, pushed
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_WRITE_LINE_DEVICE_MEMBER(CDP1802_TAG, cosmac_device, ef2_w)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_WRITE_LINE_DEVICE_MEMBER(CDP1802_TAG, cosmac_device, ef3_w)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER(CDP1802_TAG, cosmac_device, ef4_w)
INPUT_PORTS_END

static INPUT_PORTS_START( draco )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, cidelsa_state, cdp1869_pcb_r)

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "Very Conserv" )
	PORT_DIPSETTING(    0x01, "Conserv" )
	PORT_DIPSETTING(    0x02, "Liberal" )
	PORT_DIPSETTING(    0x03, "Very Liberal" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "5000" )
	PORT_DIPSETTING(    0x08, "7000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x00, "14000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xe0, 0xc0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "Slot A: 0.5  Slot B: 0.5" )
	PORT_DIPSETTING(    0xe0, "Slot A: 0.6  Slot B: 3" )
	PORT_DIPSETTING(    0xc0, "Slot A: 1  Slot B: 2" )
	PORT_DIPSETTING(    0xa0, "Slot A: 1.2  Slot B: 6" )
	PORT_DIPSETTING(    0x80, "Slot A: 1.5  Slot B: 3" )
	PORT_DIPSETTING(    0x60, "Slot A: 1.5  Slot B: 7.5" )
	PORT_DIPSETTING(    0x20, "Slot A: 2.5  Slot B: 3" )
	PORT_DIPSETTING(    0x00, "Slot A: 3  Slot B: 6" )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )

	PORT_START("EF")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) // CDP1869 PRD, pushed
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_WRITE_LINE_DEVICE_MEMBER(CDP1802_TAG, cosmac_device, ef2_w)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_WRITE_LINE_DEVICE_MEMBER(CDP1802_TAG, cosmac_device, ef3_w)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER(CDP1802_TAG, cosmac_device, ef4_w)
INPUT_PORTS_END

/* Machine Start */

void cidelsa_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_SET_CPU_MODE:
		m_reset = 1;
		break;
	default:
		assert_always(FALSE, "Unknown id in cidelsa_state::device_timer");
	}
}

void cidelsa_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_reset));
}

void draco_state::machine_start()
{
	/* setup COP402 memory banking */
	membank("bank1")->configure_entries(0, 2, memregion(COP402N_TAG)->base(), 0x400);
	membank("bank1")->set_entry(0);

	/* register for state saving */
	save_item(NAME(m_reset));
	save_item(NAME(m_sound));
	save_item(NAME(m_psg_latch));
}

/* Machine Reset */

void cidelsa_state::machine_reset()
{
	/* reset the CPU */
	m_reset = 0;
	timer_set(attotime::from_msec(200), TIMER_SET_CPU_MODE);
}

/* Machine Drivers */

static MACHINE_CONFIG_START( destryer, cidelsa_state )
	/* basic system hardware */
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, DESTRYER_CHR1)
	MCFG_CPU_PROGRAM_MAP(destryer_map)
	MCFG_CPU_IO_MAP(destryer_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(cidelsa_state, clear_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(cidelsa_state, q_w))

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* sound and video hardware */
	MCFG_FRAGMENT_ADD(destryer_video)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( destryera, cidelsa_state )
	/* basic system hardware */
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, DESTRYER_CHR1)
	MCFG_CPU_PROGRAM_MAP(destryera_map)
	MCFG_CPU_IO_MAP(destryer_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(cidelsa_state, clear_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(cidelsa_state, q_w))

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* sound and video hardware */
	MCFG_FRAGMENT_ADD(destryer_video)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( altair, cidelsa_state )
	/* basic system hardware */
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, ALTAIR_CHR1)
	MCFG_CPU_PROGRAM_MAP(altair_map)
	MCFG_CPU_IO_MAP(altair_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(cidelsa_state, clear_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(cidelsa_state, q_w))

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* input/output hardware */
	MCFG_DEVICE_ADD("ic23", CDP1852, 0) // clock is really tied to CDP1869 CMSEL (pin 37)
	MCFG_CDP1852_MODE_CALLBACK(GND)
	MCFG_CDP1852_DI_CALLBACK(IOPORT("IN0"))
	MCFG_DEVICE_ADD("ic24", CDP1852, 0)
	MCFG_CDP1852_MODE_CALLBACK(GND)
	MCFG_CDP1852_DI_CALLBACK(IOPORT("IN1"))
	MCFG_DEVICE_ADD("ic25", CDP1852, 0)
	MCFG_CDP1852_MODE_CALLBACK(GND)
	MCFG_CDP1852_DI_CALLBACK(IOPORT("IN2"))
	MCFG_DEVICE_ADD("ic26", CDP1852, ALTAIR_CHR1 / 8) // clock is CDP1802 TPB
	MCFG_CDP1852_MODE_CALLBACK(VCC)
	MCFG_CDP1852_DO_CALLBACK(WRITE8(cidelsa_state, altair_out1_w))

	/* sound and video hardware */
	MCFG_FRAGMENT_ADD(altair_video)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( draco, draco_state )
	/* basic system hardware */
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, DRACO_CHR1)
	MCFG_CPU_PROGRAM_MAP(draco_map)
	MCFG_CPU_IO_MAP(draco_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(cidelsa_state, clear_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(cidelsa_state, q_w))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_CPU_ADD(COP402N_TAG, COP402, DRACO_SND_CHR1)
	MCFG_CPU_PROGRAM_MAP(draco_sound_map)
	MCFG_COP400_CONFIG( COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, COP400_MICROBUS_DISABLED )
	MCFG_COP400_WRITE_D_CB(WRITE8(draco_state, sound_bankswitch_w))
	MCFG_COP400_WRITE_G_CB(WRITE8(draco_state, sound_g_w))
	MCFG_COP400_READ_L_CB(READ8(draco_state, psg_r))
	MCFG_COP400_WRITE_L_CB(WRITE8(draco_state, psg_w))
	MCFG_COP400_READ_IN_CB(READ8(draco_state, sound_in_r))

	/* input/output hardware */
	MCFG_DEVICE_ADD("ic29", CDP1852, 0) // clock is really tied to CDP1869 CMSEL (pin 37)
	MCFG_CDP1852_MODE_CALLBACK(GND)
	MCFG_CDP1852_DI_CALLBACK(IOPORT("IN0"))
	MCFG_DEVICE_ADD("ic30", CDP1852, 0)
	MCFG_CDP1852_MODE_CALLBACK(GND)
	MCFG_CDP1852_DI_CALLBACK(IOPORT("IN1"))
	MCFG_DEVICE_ADD("ic31", CDP1852, 0)
	MCFG_CDP1852_MODE_CALLBACK(GND)
	MCFG_CDP1852_DI_CALLBACK(IOPORT("IN2"))
	MCFG_DEVICE_ADD("ic32", CDP1852, ALTAIR_CHR1 / 8) // clock is CDP1802 TPB
	MCFG_CDP1852_MODE_CALLBACK(VCC)
	MCFG_CDP1852_DO_CALLBACK(WRITE8(draco_state, out1_w))

	/* sound and video hardware */
	MCFG_FRAGMENT_ADD(draco_video)
MACHINE_CONFIG_END

/* ROMs */

ROM_START( destryer )
	ROM_REGION( 0x2000, CDP1802_TAG, 0 )
	ROM_LOAD( "des a 2.ic4", 0x0000, 0x0800, CRC(63749870) SHA1(a8eee4509d7a52dcf33049de221d928da3632174) )
	ROM_LOAD( "des b 2.ic5", 0x0800, 0x0800, CRC(60604f40) SHA1(32ca95c5b38b0f4992e04d77123d217f143ae084) )
	ROM_LOAD( "des c 2.ic6", 0x1000, 0x0800, CRC(a7cdeb7b) SHA1(a5a7748967d4ca89fb09632e1f0130ef050dbd68) )
	ROM_LOAD( "des d 2.ic7", 0x1800, 0x0800, CRC(dbec0aea) SHA1(1d9d49009a45612ee79763781a004499313b823b) )
ROM_END

// this was destroyer2.rom in standalone emu..
ROM_START( destryera )
	ROM_REGION( 0x2000, CDP1802_TAG, 0 )
	ROM_LOAD( "destryera_1", 0x0000, 0x0800, CRC(421428e9) SHA1(0ac3a1e7f61125a1cd82145fa28cbc4b93505dc9) )
	ROM_LOAD( "destryera_2", 0x0800, 0x0800, CRC(55dc8145) SHA1(a0066d3f3ac0ae56273485b74af90eeffea5e64e) )
	ROM_LOAD( "destryera_3", 0x1000, 0x0800, CRC(5557bdf8) SHA1(37a9cbc5d25051d3bed7535c58aac937cd7c64e1) )
	ROM_LOAD( "destryera_4", 0x1800, 0x0800, CRC(608b779c) SHA1(8fd6cc376c507680777553090329cc66be42a934) )
ROM_END

ROM_START( altair )
	ROM_REGION( 0x3000, CDP1802_TAG, 0 )
	ROM_LOAD( "alt a 1.ic7",  0x0000, 0x0800, CRC(37c26c4e) SHA1(30df7efcf5bd12dafc1cb6e894fc18e7b76d3e61) )
	ROM_LOAD( "alt b 1.ic8",  0x0800, 0x0800, CRC(76b814a4) SHA1(e8ab1d1cbcef974d929ef8edd10008f60052a607) )
	ROM_LOAD( "alt c 1.ic9",  0x1000, 0x0800, CRC(2569ce44) SHA1(a09597d2f8f50fab9a09ed9a59c50a2bdcba47bb) )
	ROM_LOAD( "alt d 1.ic10", 0x1800, 0x0800, CRC(a25e6d11) SHA1(c197ff91bb9bdd04e88908259e4cde11b990e31d) )
	ROM_LOAD( "alt e 1.ic11", 0x2000, 0x0800, CRC(e497f23b) SHA1(6094e9873df7bd88c521ddc3fd63961024687243) )
	ROM_LOAD( "alt f 1.ic12", 0x2800, 0x0800, CRC(a06dd905) SHA1(c24ad9ff6d4e3b4e57fd75f946e8832fa00c2ea0) )
ROM_END

ROM_START( draco )
	ROM_REGION( 0x4000, CDP1802_TAG, 0 )
	ROM_LOAD( "dra a 1.ic10", 0x0000, 0x0800, CRC(ca127984) SHA1(46721cf42b1c891f7c88bc063a2149dd3cefea74) )
	ROM_LOAD( "dra b 1.ic11", 0x0800, 0x0800, CRC(e4936e28) SHA1(ddbbf769994d32a6bce75312306468a89033f0aa) )
	ROM_LOAD( "dra c 1.ic12", 0x1000, 0x0800, CRC(94480f5d) SHA1(8f49ce0f086259371e999d097a502482c83c6e9e) )
	ROM_LOAD( "dra d 1.ic13", 0x1800, 0x0800, CRC(32075277) SHA1(2afaa92c91f554e3bdcfec6d94ef82df63032afb) )
	ROM_LOAD( "dra e 1.ic14", 0x2000, 0x0800, CRC(cce7872e) SHA1(c956eb994452bd8a27bbc6d0e6d103e87a4a3e6e) )
	ROM_LOAD( "dra f 1.ic15", 0x2800, 0x0800, CRC(e5927ec7) SHA1(42e0aabb6187bbb189648859fd5dddda43814526) )
	ROM_LOAD( "dra g 1.ic16", 0x3000, 0x0800, CRC(f28546c0) SHA1(daedf1d64f94358b15580d697dd77d3c977aa22c) )
	ROM_LOAD( "dra h 1.ic17", 0x3800, 0x0800, CRC(dce782ea) SHA1(f558096f43fb30337bc4a527169718326c265c2c) )

	ROM_REGION( 0x800, COP402N_TAG, 0 )
	ROM_LOAD( "dra s 1.ic4",  0x0000, 0x0800, CRC(292a57f8) SHA1(b34a189394746d77c3ee669db24109ee945c3be7) )
ROM_END

/* Game Drivers */

GAME( 1980, destryer, 0,        destryer, destryer, driver_device, 0, ROT90, "Cidelsa", "Destroyer (Cidelsa) (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, destryera,destryer, destryera,destryer, driver_device, 0, ROT90, "Cidelsa", "Destroyer (Cidelsa) (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, altair,   0,        altair,   altair, driver_device,   0, ROT90, "Cidelsa", "Altair", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, draco,    0,        draco,    draco, driver_device,    0, ROT90, "Cidelsa", "Draco", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
