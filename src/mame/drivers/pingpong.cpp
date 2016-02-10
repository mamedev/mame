// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
/***************************************************************************

Ping Pong (c) 1985 Konami

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "machine/nvram.h"
#include "includes/pingpong.h"



WRITE8_MEMBER(pingpong_state::cashquiz_question_bank_high_w)
{
	if( data != 0xff )
	{
		int i;

		for(i=0;i<8;i++)
		{
			if((~data & 0xff) == 1 << i)
			{
				m_question_addr_high = i*0x8000;
				return;
			}
		}
	}
}

WRITE8_MEMBER(pingpong_state::cashquiz_question_bank_low_w)
{
	if(data >= 0x60 && data <= 0xdf)
	{
		static const char * const bankname[] = { "bank1", "bank2", "bank3", "bank4", "bank5", "bank6", "bank7", "bank8" };
		const char *bank = bankname[data & 7];
		int bankaddr = m_question_addr_high | ((data - 0x60) * 0x100);
		UINT8 *questions = memregion("user1")->base() + bankaddr;
		membank(bank)->set_base(questions);

	}
}


WRITE8_MEMBER(pingpong_state::coin_w)
{
	/* bit 2 = irq enable, bit 3 = nmi enable */
	m_intenable = data & 0x0c;

	/* bit 0/1 = coin counters */
	machine().bookkeeping().coin_counter_w(0,data & 1);
	machine().bookkeeping().coin_counter_w(1,data & 2);

	/* other bits unknown */
}

TIMER_DEVICE_CALLBACK_MEMBER(pingpong_state::pingpong_interrupt)
{
	int scanline = param;

	if (scanline == 240)
	{
		if (m_intenable & 0x04) m_maincpu->set_input_line(0, HOLD_LINE);
	}
	else if ((scanline % 32) == 0)
	{
		if (m_intenable & 0x08) m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(pingpong_state::merlinmm_interrupt)
{
	int scanline = param;

	if (scanline == 240)
	{
		if (m_intenable & 0x04) m_maincpu->set_input_line(0, HOLD_LINE);
	}
	else if (scanline == 0)
	{
		if (m_intenable & 0x08) m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

static ADDRESS_MAP_START( pingpong_map, AS_PROGRAM, 8, pingpong_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(pingpong_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x8400, 0x87ff) AM_RAM_WRITE(pingpong_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9000, 0x9002) AM_RAM
	AM_RANGE(0x9003, 0x9052) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9053, 0x97ff) AM_RAM
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xa880, 0xa880) AM_READ_PORT("INPUTS")
	AM_RANGE(0xa900, 0xa900) AM_READ_PORT("DSW1")
	AM_RANGE(0xa980, 0xa980) AM_READ_PORT("DSW2")
	AM_RANGE(0xa000, 0xa000) AM_WRITE(coin_w)   /* coin counters + irq enables */
	AM_RANGE(0xa200, 0xa200) AM_WRITENOP        /* SN76496 data latch */
	AM_RANGE(0xa400, 0xa400) AM_DEVWRITE("snsnd", sn76496_device, write)    /* trigger read */
	AM_RANGE(0xa600, 0xa600) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( merlinmm_map, AS_PROGRAM, 8, pingpong_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x5000, 0x53ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x5400, 0x57ff) AM_RAM
	AM_RANGE(0x6000, 0x6007) AM_WRITENOP /* solenoid writes */
	AM_RANGE(0x7000, 0x7000) AM_READ_PORT("IN4")
	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(pingpong_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x8400, 0x87ff) AM_RAM_WRITE(pingpong_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9000, 0x9002) AM_RAM
	AM_RANGE(0x9003, 0x9052) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9053, 0x97ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_WRITE(coin_w)   /* irq enables */
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa080, 0xa080) AM_READ_PORT("IN1")
	AM_RANGE(0xa100, 0xa100) AM_READ_PORT("IN2")
	AM_RANGE(0xa180, 0xa180) AM_READ_PORT("IN3")
	AM_RANGE(0xa200, 0xa200) AM_WRITENOP        /* SN76496 data latch */
	AM_RANGE(0xa400, 0xa400) AM_DEVWRITE("snsnd", sn76496_device, write)    /* trigger read */
	AM_RANGE(0xa600, 0xa600) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( pingpong )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0F, 0x0F, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0A, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0F, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0C, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x0E, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0B, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0D, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xF0, 0xF0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xA0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xF0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xC0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0xE0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xB0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xD0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:7,6")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( merlinmm )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "Bank 3-3")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Bank 3-2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Bank 3-1")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Door Close")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Door Open")      // Seems strange, one input to register an open door
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )  // And a different one for closing it!
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY

	PORT_START("IN2")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "Stake" )
	PORT_DIPSETTING(    0x02, "10p" )
	PORT_DIPSETTING(    0x00, "20p" )
	PORT_DIPNAME( 0x04, 0x04, "Bank 1-6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Bank 1-5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "10p Enabled" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "20p Enabled" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "50p Enabled" )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "100p Enabled" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "Bank 2-8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Bank 2-7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Bank 2-6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Bank 2-5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Bank 2-4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Bank 2-3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Bank 2-2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Bank 2-1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "10P Level" )     // Most likely to be optos, rather than DIPs.
	PORT_DIPSETTING(    0x01, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x02, 0x02, "20P Level" )
	PORT_DIPSETTING(    0x02, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x04, 0x04, "50P Level" )
	PORT_DIPSETTING(    0x04, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x08, 0x08, "100P Level" )
	PORT_DIPSETTING(    0x08, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("10P")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("20P")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("50P")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("100P")
INPUT_PORTS_END

static INPUT_PORTS_START( cashquiz )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  // 0x20 and 0x40 if both ON enable a test menu
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 - A")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Select")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 - B")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Set")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 - C")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 - A")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 - B")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 - C")

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("10P")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("20P")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("50P")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("100P")
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,        /* 8*8 characters */
	512,        /* 512 characters */
	2,      /* 2 bits per pixel */
	{ 4, 0 },   /* the bitplanes are packed in one nibble */
	{ 3, 2, 1, 0, 8*8+3, 8*8+2, 8*8+1, 8*8+0 }, /* x bit */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8   },     /* y bit */
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,      /* 16*16 sprites */
	128,        /* 128 sprites */
	2,      /* 2 bits per pixel */
	{ 4, 0 },   /* the bitplanes are packed in one nibble */
	{ 12*16+3,12*16+2,12*16+1,12*16+0,
		8*16+3, 8*16+2, 8*16+1, 8*16+0,
		4*16+3, 4*16+2, 4*16+1, 4*16+0,
			3,      2,      1,      0 },            /* x bit */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8  },    /* y bit */
	64*8    /* every char takes 64 consecutive bytes */
};

static GFXDECODE_START( pingpong )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,         0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,    64*4, 64 )
GFXDECODE_END


static MACHINE_CONFIG_START( pingpong, pingpong_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,18432000/6)      /* 3.072 MHz (probably) */
	MCFG_CPU_PROGRAM_MAP(pingpong_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", pingpong_state, pingpong_interrupt, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(456, 262)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pingpong_state, screen_update_pingpong)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pingpong)
	MCFG_PALETTE_ADD("palette", 64*4+64*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(pingpong_state, pingpong)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76496, 18432000/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/* too fast! */
static MACHINE_CONFIG_DERIVED( merlinmm, pingpong )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(merlinmm_map)
	MCFG_TIMER_MODIFY("scantimer")
	MCFG_TIMER_DRIVER_CALLBACK(pingpong_state, merlinmm_interrupt)

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pingpong )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp_e04.rom",   0x0000, 0x4000, CRC(18552f8f) SHA1(cb03659b5e8a68003e72182a20979384d829280f) )
	ROM_LOAD( "pp_e03.rom",   0x4000, 0x4000, CRC(ae5f01e8) SHA1(f0d6a2c64822f2662fed3f601e279db18246f894) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pp_e01.rom",   0x0000, 0x2000, CRC(d1d6f090) SHA1(7b7d7cb90bed746dda871227463145263e4b0c5a) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "pp_e02.rom",   0x0000, 0x2000, CRC(33c687e0) SHA1(7c90de4d163d2ffad00c8cb6a194fa6125a4f4c1) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pingpong.3j",  0x0000, 0x0020, CRC(3e04f06e) SHA1(a642c350f148e062d56eb2a2fc53c470603000e3) ) /* palette (this might be bad) */
	ROM_LOAD( "pingpong.5h",  0x0020, 0x0100, CRC(8456046a) SHA1(8226f1325c14eb8aed5cd3c3d6bad9f9fd88c5fa) ) /* characters */
	ROM_LOAD( "pingpong.11j", 0x0120, 0x0100, CRC(09d96b08) SHA1(81405e33eacc47f91ea4c7221d122f7e6f5b1e5d) ) /* sprites */
ROM_END

ROM_START( merlinmm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "merlinmm.ic2", 0x0000, 0x4000, CRC(ea5b6590) SHA1(fdd5873c67761955e33260743cc45075dea34fb4) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "merlinmm.7h",  0x0000, 0x2000, CRC(f7d535aa) SHA1(65f100c15b07ec3aa21f5ed132e2fbf6e9120dbe) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "merl_sp.12c",  0x0000, 0x2000, CRC(517ecd57) SHA1(b0d4e2d106cddd6d19acd0e10f2d32544c84a900) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "merlinmm.3j",  0x0000, 0x0020, CRC(d56e91f4) SHA1(152d88e4d168f697030d96c02ab9aeb220cc765d) ) /* palette */
	ROM_LOAD( "pingpong.5h",  0x0020, 0x0100, CRC(8456046a) SHA1(8226f1325c14eb8aed5cd3c3d6bad9f9fd88c5fa) ) /* characters */
	ROM_LOAD( "pingpong.11j", 0x0120, 0x0100, CRC(09d96b08) SHA1(81405e33eacc47f91ea4c7221d122f7e6f5b1e5d) ) /* sprites */
ROM_END

ROM_START( cashquiz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashqcv5.ic3", 0x0000, 0x4000, CRC(8e9e2bed) SHA1(1894d40f89226a810c703ce5e49fdfd64d70287f) )
	/* 0x4000 - 0x7fff = extra hardware for question board */

	ROM_REGION( 0x40000, "user1", 0 ) /* Question roms */
	ROM_LOAD( "q30_soaps.ic1",      0x02000, 0x6000, CRC(b35a30ac) SHA1(5daf52a6d973f5a1b1ec3395962bcab690c54e43) )
	ROM_CONTINUE(                   0x00000, 0x2000 )
	ROM_LOAD( "q10.ic2",            0x0a000, 0x6000, CRC(54962e11) SHA1(3c89ac26ebc002b2bc723f1424a7ba3db7a98e5f) )
	ROM_CONTINUE(                   0x08000, 0x2000 )
	ROM_LOAD( "q29_newsoccrick.ic3",0x12000, 0x6000, CRC(03d47262) SHA1(8a849cb4d4440042042cbdc0f34feebe71d6cb37) )
	ROM_CONTINUE(                   0x10000, 0x2000 )
	ROM_LOAD( "q28_sportstime.ic4", 0x1a000, 0x6000, CRC(2bd00476) SHA1(88ed9d26909873c52273290686b4783563edfb61) )
	ROM_CONTINUE(                   0x18000, 0x2000 )
	ROM_LOAD( "q20_mot.ic5",        0x22000, 0x6000, CRC(17a38baf) SHA1(5560932e4747a242df7c8b7bbaf8679c9a8be6ac) )
	ROM_CONTINUE(                   0x20000, 0x2000 )
	ROM_LOAD( "q14_popmusic2.ic6",  0x2a000, 0x6000, CRC(e486d6ee) SHA1(421723fa7604c0509092891e53723191bd62e294) )
	ROM_CONTINUE(                   0x28000, 0x2000 )
	ROM_LOAD( "q26_screenent.ic7",  0x32000, 0x6000, CRC(9d130515) SHA1(bfc32219d4d4eaca4efa02c3c46125144c8cd286) )
	ROM_CONTINUE(                   0x30000, 0x2000 )
	ROM_LOAD( "q19.ic8",            0x3a000, 0x6000, CRC(9f3f77e6) SHA1(aa1600215e774b090f379a0aae520027cd1795c1) )
	ROM_CONTINUE(                   0x38000, 0x2000 )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cashq.7h",  0x0000, 0x2000, CRC(44b72a4f) SHA1(a993f1570cf9d8f86d4229198e9b1a0d6a92e51f) )
	ROM_CONTINUE(          0x0000, 0x2000 ) /* rom is a 27128 in a 2764 socket */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "cashq.12c",  0x0000, 0x2000, NO_DUMP ) // missing :-(

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "cashquiz.3j",  0x0000, 0x0020, CRC(dc70e23b) SHA1(90948f76d5c61eb57838e013aa93d733913a2d92) ) /* palette */
	ROM_LOAD( "pingpong.5h",  0x0020, 0x0100, CRC(8456046a) SHA1(8226f1325c14eb8aed5cd3c3d6bad9f9fd88c5fa) ) /* characters */
	ROM_LOAD( "pingpong.11j", 0x0120, 0x0100, CRC(09d96b08) SHA1(81405e33eacc47f91ea4c7221d122f7e6f5b1e5d) ) /* sprites */
ROM_END

DRIVER_INIT_MEMBER(pingpong_state,merlinmm)
{
	UINT8 *ROM = memregion("maincpu")->base();
	int i;

	/* decrypt program code */
	for( i = 0; i < 0x4000; i++ )
		ROM[i] = BITSWAP8(ROM[i],0,1,2,3,4,5,6,7);
}

DRIVER_INIT_MEMBER(pingpong_state,cashquiz)
{
	UINT8 *ROM;
	int i;

	/* decrypt program code */
	ROM = memregion("maincpu")->base();
	for( i = 0; i < 0x4000; i++ )
		ROM[i] = BITSWAP8(ROM[i],0,1,2,3,4,5,6,7);

	/* decrypt questions */
	ROM = memregion("user1")->base();
	for( i = 0; i < 0x40000; i++ )
		ROM[i] = BITSWAP8(ROM[i],0,1,2,3,4,5,6,7);

	/* questions banking handlers */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4000, 0x4000, write8_delegate(FUNC(pingpong_state::cashquiz_question_bank_high_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4001, 0x4001, write8_delegate(FUNC(pingpong_state::cashquiz_question_bank_low_w),this));

	// 8 independents banks for questions
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x5000, 0x50ff, "bank1");
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x5100, 0x51ff, "bank2");
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x5200, 0x52ff, "bank3");
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x5300, 0x53ff, "bank4");
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x5400, 0x54ff, "bank5");
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x5500, 0x55ff, "bank6");
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x5600, 0x56ff, "bank7");
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x5700, 0x57ff, "bank8");

	// setup default banks
	membank("bank1")->set_base(memregion("user1")->base() + 0x100*0 );
	membank("bank2")->set_base(memregion("user1")->base() + 0x100*1 );
	membank("bank3")->set_base(memregion("user1")->base() + 0x100*2 );
	membank("bank4")->set_base(memregion("user1")->base() + 0x100*3 );
	membank("bank5")->set_base(memregion("user1")->base() + 0x100*4 );
	membank("bank6")->set_base(memregion("user1")->base() + 0x100*5 );
	membank("bank7")->set_base(memregion("user1")->base() + 0x100*6 );
	membank("bank8")->set_base(memregion("user1")->base() + 0x100*7 );
}


GAME( 1985, pingpong, 0, pingpong, pingpong, driver_device, 0,         ROT0, "Konami", "Konami's Ping-Pong", 0 )
GAME( 1986, merlinmm, 0, merlinmm, merlinmm, pingpong_state, merlinmm, ROT90,"Zilec-Zenitone", "Merlins Money Maze", 0 )
GAME( 1986, cashquiz, 0, merlinmm, cashquiz, pingpong_state, cashquiz, ROT0, "Zilec-Zenitone", "Cash Quiz (Type B, Version 5)", MACHINE_IMPERFECT_GRAPHICS )
