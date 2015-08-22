// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

Sound board for an unknown game.

"Copyright (c) 1983  SoftLogic JAPAN" in the program rom.

The same PCB, with serial 025707, is allegedly for "Vampire" (prototype).

PCB: 

Entertainment Enterprises Ltd. 1983 (sticker)
Serial No. 025402 (sticker)
MADE IN JAPAN (etched)

CPU:    R6502P
RAM:    M58725P (2KB)
I/O:    R6522P (VIA)
Speech: Digitalker
Sound:  AY-3-8910
Misc:   XTAL 4MHz, DSW4, 42-pin connector

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "sound/digitalk.h"
#include "machine/6522via.h"

#define MAIN_CLOCK XTAL_4MHz

class vampire_state : public driver_device
{
public:
	vampire_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_audiocpu(*this, "audiocpu"),
			m_digitalker(*this, "digitalker"),
			m_aysnd(*this, "aysnd")
	{
	}

	// devices
	required_device<cpu_device> m_audiocpu;
	required_device<digitalker_device> m_digitalker;
	required_device<ay8910_device> m_aysnd;

	// vars
	UINT8  m_ay_cmd, m_ay_data;
	UINT8  m_port_a, m_port_b;
	UINT8  m_coin;
	UINT8  m_bank;
	UINT32 m_shift;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// handlers
	DECLARE_READ8_MEMBER(via_a_in);
	DECLARE_READ8_MEMBER(via_b_in);

	DECLARE_WRITE8_MEMBER(via_a_out);
	DECLARE_WRITE8_MEMBER(via_b_out);

	DECLARE_WRITE_LINE_MEMBER(via_ca2_out);
	DECLARE_WRITE_LINE_MEMBER(via_cb1_out);
	DECLARE_WRITE_LINE_MEMBER(via_cb2_out);
	DECLARE_WRITE_LINE_MEMBER(via_irq_out);

	DECLARE_WRITE8_MEMBER(vampire_ay_w);
	DECLARE_READ8_MEMBER(vampire_coin_r);
	DECLARE_WRITE8_MEMBER(vampire_coin_w);

	// digitalker
	void digitalker_set_bank(UINT8 bank);

	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
};


// VIA

READ8_MEMBER(vampire_state::via_a_in)
{
	UINT8 ret = 0;
	logerror("%s: VIA read A: %02X\n", machine().describe_context(), ret);
	return ret;
}
READ8_MEMBER(vampire_state::via_b_in)
{
	UINT8 ret = 0;
	logerror("%s: VIA read B: %02X\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vampire_state::via_a_out)
{
	m_port_a = data;	// multiplexer
//	logerror("%s: VIA write A = %02X\n", machine().describe_context(), data);
}
WRITE8_MEMBER(vampire_state::via_b_out)
{
	m_port_b = data;
//	logerror("%s: VIA write B = %02X\n", machine().describe_context(), data);
}

WRITE_LINE_MEMBER(vampire_state::via_ca2_out)
{
//	logerror("%s: VIA write CA2 = %02X\n", machine().describe_context(), state);
}
WRITE_LINE_MEMBER(vampire_state::via_cb1_out)
{
//	logerror("%s: VIA write CB1 = %02X\n", machine().describe_context(), state);
}
WRITE_LINE_MEMBER(vampire_state::via_cb2_out)
{
	m_shift = ((m_shift << 1) & 0xffffff) | state;
//	logerror("%s: VIA write CB2 = %02X\n", machine().describe_context(), state);
}
WRITE_LINE_MEMBER(vampire_state::via_irq_out)
{
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
//	logerror("%s: VIA write IRQ = %02X\n", machine().describe_context(), state);
}

// Video

void vampire_state::video_start()
{
}

UINT32 vampire_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	// NMI on coin-in?
	UINT8 coin = (~ioport("INPUTS")->read()) & 0x03;
	m_audiocpu->set_input_line(INPUT_LINE_NMI, coin ? ASSERT_LINE : CLEAR_LINE);

	// Play the digitalker samples (it's not hooked up yet)
	static UINT8 sample = 0, bank = 0;

	if (screen.machine().input().code_pressed_once(KEYCODE_Q))
		--bank;
	if (screen.machine().input().code_pressed_once(KEYCODE_W))
		++bank;
	bank %= 3;
	digitalker_set_bank(bank);

	if (screen.machine().input().code_pressed_once(KEYCODE_A))
		--sample;
	if (screen.machine().input().code_pressed_once(KEYCODE_S))
		++sample;

	if (screen.machine().input().code_pressed_once(KEYCODE_Z))
	{
		m_digitalker->digitalker_0_cms_w(CLEAR_LINE);
		m_digitalker->digitalker_0_cs_w(CLEAR_LINE);

		address_space &space = m_audiocpu->space(AS_PROGRAM);
		m_digitalker->digitalker_data_w(space, 0, sample, 0xff);

		m_digitalker->digitalker_0_wr_w(ASSERT_LINE);
		m_digitalker->digitalker_0_wr_w(CLEAR_LINE);
		m_digitalker->digitalker_0_wr_w(ASSERT_LINE);
	}	

	popmessage("COIN: %02X VIAB: %02X\nCOUNT: %02X %02X %02X\nSAMPLE: %02X (B%X)",
				m_coin, m_port_b,
				(m_shift >> 16) & 0xff, (m_shift >> 8) & 0xff, (m_shift >> 0) & 0xff,	// 3 x 7-seg?
				sample, bank
	);
	return 0;
}

// Sound

#if 0
READ8_MEMBER(scramble_state::vampire_digitalker_intr_r)
{
	return m_digitalker->digitalker_0_intr_r();
}

WRITE8_MEMBER(scramble_state::vampire_digitalker_control_w)
{
	m_digitalker->digitalker_0_cs_w (data & 1 ? ASSERT_LINE : CLEAR_LINE);
	m_digitalker->digitalker_0_cms_w(data & 2 ? ASSERT_LINE : CLEAR_LINE);
	m_digitalker->digitalker_0_wr_w (data & 4 ? ASSERT_LINE : CLEAR_LINE);
}
#endif

WRITE8_MEMBER(vampire_state::vampire_ay_w)
{
	if (offset)
	{
		m_ay_data = data;
		return;
	}

	if (m_ay_cmd == 0x00 && data == 0x03)
	{
		m_aysnd->address_w(space, offset, m_ay_data, mem_mask);
//		logerror("%s: AY addr = %02X\n", machine().describe_context(), m_ay_data);
	}
	else if (m_ay_cmd == 0x00 && data == 0x02)
	{
		m_aysnd->data_w(space, offset, m_ay_data, mem_mask);
//		logerror("%s: AY data = %02X\n", machine().describe_context(), m_ay_data);
	}
	m_ay_cmd = data;
}

// Memory Map

READ8_MEMBER(vampire_state::vampire_coin_r)
{
	UINT8 ret = ioport("DSW")->read();			// bits 0-3
	UINT8 inp = ioport("INPUTS")->read();		// bit 7 (multiplexed)

	for (int i = 0; i < 8; ++i)
		if ( ((~m_port_a) & (1 << i)) && ((~inp) & (1 << i)) )
			ret &= 0x7f;

	return ret;
}

WRITE8_MEMBER(vampire_state::vampire_coin_w)
{
	m_coin = data;
//	coin_counter_w(machine(), 0, data & 0x01);
//	coin_counter_w(machine(), 1, data & 0x02);
	coin_lockout_w(machine(), 0, data & 0x01);
	coin_lockout_w(machine(), 1, data & 0x02);
}

//M58725P - 2KB
static ADDRESS_MAP_START( vampire_map, AS_PROGRAM, 8, vampire_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x01ff) AM_RAM

	AM_RANGE(0x4000, 0x400f) AM_DEVREADWRITE("via6522", via6522_device, read, write)

	AM_RANGE(0x8000, 0x8000) AM_READ(vampire_coin_r)

	AM_RANGE(0xa000, 0xa001) AM_WRITE(vampire_ay_w)

	AM_RANGE(0xc000, 0xc000) AM_WRITE(vampire_coin_w)

	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION("audiocpu", 0)
ADDRESS_MAP_END

// Inputs

static INPUT_PORTS_START( vampire )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(1) // coin 1 (start music)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2   ) PORT_IMPULSE(1) // coin 2 (start music)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3   ) // ?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) // (stop music)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) // (stop music)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) // ?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, "Coins Per Credit?" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPNAME( 0x0c, 0x00, "Music Delay?" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x04, "60" )
	PORT_DIPSETTING(    0x08, "80" )
	PORT_DIPSETTING(    0x0c, "100" )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) // multiplexed inputs
INPUT_PORTS_END

// Machine

void vampire_state::machine_start()
{
}

void vampire_state::digitalker_set_bank(UINT8 bank)
{
	if (m_bank != bank)
	{
		UINT8 *src = memregion("samples")->base();
		UINT8 *dst = memregion("digitalker")->base();

		memcpy(dst, src + bank * 0x4000, 0x4000);

		m_bank = bank;
	}
}

void vampire_state::machine_reset()
{
	m_bank = -1;
	digitalker_set_bank(0);
}

static MACHINE_CONFIG_START( vampire, vampire_state )

	// basic machine hardware
	MCFG_CPU_ADD("audiocpu", M6502, MAIN_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(vampire_map)
//	MCFG_CPU_VBLANK_INT_DRIVER("screen", vampire_state, irq0_line_hold)
//	MCFG_CPU_VBLANK_INT_DRIVER("screen", vampire_state, nmi_line_pulse)

	// via
	MCFG_DEVICE_ADD("via6522", VIA6522, MAIN_CLOCK/4)

	MCFG_VIA6522_READPA_HANDLER(READ8(vampire_state,via_a_in))
	MCFG_VIA6522_READPB_HANDLER(READ8(vampire_state,via_b_in))

	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(vampire_state, via_a_out))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(vampire_state, via_b_out))

	MCFG_VIA6522_CA2_HANDLER(WRITELINE(vampire_state, via_ca2_out))
	MCFG_VIA6522_CB1_HANDLER(WRITELINE(vampire_state, via_cb1_out))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(vampire_state, via_cb2_out))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(vampire_state, via_irq_out)/*DEVWRITELINE("audiocpu", m6502_device, write_irq4)*/)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(vampire_state, screen_update)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_DIGITALKER_ADD("digitalker", MAIN_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.16)
MACHINE_CONFIG_END

// ROMs

ROM_START( vampire )
	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "1.5d", 0x0000, 0x1000, CRC(6ab050be) SHA1(ebecae855e22e9c3c46bdee51f84fd5352bf191a) )

	ROM_REGION( 0x4000, "digitalker", ROMREGION_ERASE00 )
	// bank switched (from samples region)

	ROM_REGION( 0xc000, "samples", 0 )
	ROM_LOAD( "9.2a", 0x0000, 0x2000, CRC(059b3725) SHA1(5837bee1ef34ce19a3101b851ca55029776e4b3e) )	// digitalker header
	ROM_LOAD( "8.2b", 0x2000, 0x2000, CRC(679da4e1) SHA1(01a5b9dd132c1b0de97c153d7de226f5bf357338) )

	ROM_LOAD( "7.2c", 0x4000, 0x2000, CRC(f8461b33) SHA1(717a8842e0ce9ba94dd59504a324bede4844e389) )	// digitalker header
	ROM_LOAD( "6.2d", 0x6000, 0x2000, CRC(156c91e0) SHA1(6017d4b5609b214a6e66dcd76493a7d1442c04d4) )

	ROM_LOAD( "5.3a", 0x8000, 0x2000, CRC(19904604) SHA1(633c211a9a822cdf597a6f3c221ae9c8d6482e82) )	// digitalker header
	ROM_LOAD( "4.3b", 0xa000, 0x2000, CRC(c3386d51) SHA1(7882e88db55ba914be81075e4b2d76e246c34d3b) )
ROM_END

GAME( 1983, vampire, 0, vampire, vampire, driver_device, 0, ROT270, "SoftLogic (Entertainment Enterprises, Ltd. license)", "Vampire?", MACHINE_NOT_WORKING )
