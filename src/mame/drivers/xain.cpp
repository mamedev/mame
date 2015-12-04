// license:???
// copyright-holders:Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino
/***************************************************************************

Solar Warrior / Xain'd Sleena
Technos, 1986

PCB Layout
----------

Top Board

TA-0019-P1-03
|------------------------------------------------------------------------|
|M51516 YM3014 YM2203 YM2203 68A09 P2-0.49                               |
|       YM3014                                                           |
|                                  6116                                 |--|
|                                                                       |  |
|        2018                      *                                    |  |
|        6148                                                           |  |
|                           PT-0.59                                     |  |
|                                  P3-0.46                              |  |
|                                                                       |  |
|J                                                                      |  |
|A    DSW2                         P4-0.45                              |--|
|M                                                     2018              |
|M                                                                       |
|A                                 P5-0.44                               |
|     DSW1                                                               |
|                                                                       |--|
|                                  P6-0.43                              |  |
|            68B09                                                      |  |
|                                              68B09                    |  |
|                   P9-02.66       P7-0.42                              |  |
|                                                                       |  |
|                                                                       |  |
| 68705             PA-03.65       P8-0.41                              |  |
| (PZ-0.113)                                P1-02.29  P0-02.15   6264   |--|
|                                                                        |
|                                  *                                     |
|------------------------------------------------------------------------|
Notes:
        6809 - Hitachi HD68A09 / HD68B09 CPU, running at 1.500MHz [12/8] (x3, DIP40)
      YM2203 - Yamaha YM2203C sound chip, running at 3.000MHz [12/4] (x2, DIP40)
       68705 - Motorola MC68705P5S Microcontroller, running at 3.000MHz [12/4] (labelled 'PZ-0', DIP28)
           * - Empty socket
        6264 - Hitachi HM6264LP-15 8K x8 SRAM (DIP28)
        2018 - Toshiba TMM2018D-45 2K x8 SRAM (DIP24)
        6148 - Hitachi HM6148HP-45 1K x4 SRAM (DIP18)
        6116 - Hitachi HM6116LP-3 2K x8 SRAM (DIP24)

       VSync - 57Hz

        ROMs - Name         Device       Use
               P9-02.66     TMM24256   \ CPU1 program
               PA-03.65     MBM27256   /

               P0-02.15     TMM24256   \ CPU2 program
               P1-02.29     TMM24256   /

               P2-0.49      TMM24256     Sound CPU program

               P3-0.46      TMM24256   / GFX
               P4-0.45      TMM24256   |
               P5-0.44      TMM24256   |
               P6-0.43      TMM24256   |
               P7-0.42      TMM24256   |
               P8-0.41      TMM24256   /

Bottom Board

TA-0019-P2-03
|------------------------------------------------------------------------|
|    PK-0.136  PC-0.114                                                  |
|    PL-0.135  PD-0.113                                                  |
|                                                                       |--|
|    PM-0.134  PE-0.112                                                 |  |
|    PN-02.133 PF-02.111                                                |  |
|                                                                       |  |
|           2018                                                        |  |
|                                                                       |  |
|    PO-0.131  PG-0.109                                                 |  |
|                                                                       |  |
|    PP-0.130  PH-0.108                                                 |--|
|                                                                  12MHz |
|    PQ-0.129  PI-0.107                                PB-0.24           |
|                                                                        |
|    PR-0.128  PJ-0.106                                6116              |
|                                                                       |--|
|         2018                                                          |  |
|                                                                       |  |
|                                                                       |  |
|                                                                       |  |
|                                                                       |  |
|                                                                       |  |
|         2018                                                   2018   |  |
|                                                                       |--|
|             2018                                                       |
|             2018                                                       |
|------------------------------------------------------------------------|
Notes:
        2018 - Toshiba TMM2018D-45 2K x8 SRAM (DIP24)

        ROMs - Name         Device       Use
               PB-0.24      TMM24256   / GFX
               PC-0.114     TMM24256   |
               PD-0.113     TMM24256   |
               PE-0.112     TMM24256   |
               PF-02.111    MBM27256   |
               PG-0.109     TMM24256   |
               PH-0.108     TMM24256   |
               PI-0.107     TMM24256   |
               PJ-0.106     TMM24256   |
               PK-0.136     TMM24256   |
               PL-0.135     TMM24256   |
               PM-0.134     TMM24256   |
               PN-02.133    MBM27256   |
               PO-0.131     TMM24256   |
               PP-0.130     TMM24256   |
               PQ-0.129     TMM24256   |
               PR-0.128     TMM24256   /


Driver by Carlos A. Lozano & Rob Rosenbrock & Phil Stroffolino
Updates by Bryan McPhail, 12/12/2004:
    Fixed NMI & FIRQ handling according to schematics.
    Fixed clock speeds.
    Implemented GFX priority register/priority PROM

    Xain has a semi-bug that shows up in MAME - at 0xa26b there is a tight
    loop that checks for the VBLANK input bit going high.  However at the
    start of a game the VBLANK interrupt routine doesn't return before
    the VBLANK input bit goes low (VBLANK is held high for 8 scanlines only).
    This would cause the emulation to hang, but it would work on the real
    board because the instruction currently being decoded would finish
    before the NMI was taken, so the VBLANK bit and NMI are not actually
    exactly synchronised in practice.  This is currently hacked in MAME
    by raising the VBLANK bit a scanline early.

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6805/m6805.h"
#include "sound/2203intf.h"
#include "includes/xain.h"

#define MASTER_CLOCK        XTAL_12MHz
#define CPU_CLOCK           MASTER_CLOCK / 8
#define MCU_CLOCK           MASTER_CLOCK / 4
#define PIXEL_CLOCK         MASTER_CLOCK / 2


/*
    Based on the Solar Warrior schematics, vertical timing counts as follows:

        08,09,0A,0B,...,FC,FD,FE,FF,E8,E9,EA,EB,...,FC,FD,FE,FF,
        08,09,....

    Thus, it counts from 08 to FF, then resets to E8 and counts to FF again.
    This gives (256 - 8) + (256 - 232) = 248 + 24 = 272 total scanlines.

    VBLK is signalled starting when the counter hits F8, and continues through
    the reset to E8 and through until the next reset to 08 again.

    Since MAME's video timing is 0-based, we need to convert this.
*/

inline int xain_state::scanline_to_vcount(int scanline)
{
	int vcount = scanline + 8;
	if (vcount < 0x100)
		return vcount;
	else
		return (vcount - 0x18) | 0x100;
}

TIMER_DEVICE_CALLBACK_MEMBER(xain_state::scanline)
{
	int scanline = param;
	int screen_height = m_screen->height();
	int vcount_old = scanline_to_vcount((scanline == 0) ? screen_height - 1 : scanline - 1);
	int vcount = scanline_to_vcount(scanline);

	/* update to the current point */
	if (scanline > 0)
	{
		m_screen->update_partial(scanline - 1);
	}

	/* FIRQ (IMS) fires every on every 8th scanline (except 0) */
	if (!(vcount_old & 8) && (vcount & 8))
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	}

	/* NMI fires on scanline 248 (VBL) and is latched */
	if (vcount == 0xf8)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}

	/* VBLANK input bit is held high from scanlines 248-255 */
	if (vcount >= 248-1)    // -1 is a hack - see notes above
	{
		m_vblank = 1;
	}
	else
	{
		m_vblank = 0;
	}
}

WRITE8_MEMBER(xain_state::cpuA_bankswitch_w)
{
	m_pri = data & 0x7;
	membank("bank1")->set_entry((data >> 3) & 1);
}

WRITE8_MEMBER(xain_state::cpuB_bankswitch_w)
{
	membank("bank2")->set_entry(data & 1);
}

WRITE8_MEMBER(xain_state::sound_command_w)
{
	soundlatch_byte_w(space,offset,data);
	m_audiocpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
}

WRITE8_MEMBER(xain_state::main_irq_w)
{
	switch (offset)
	{
	case 0: /* 0x3a09 - NMI clear */
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		break;
	case 1: /* 0x3a0a - FIRQ clear */
		m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
		break;
	case 2: /* 0x3a0b - IRQ clear */
		m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
		break;
	case 3: /* 0x3a0c - IRQB assert */
		m_subcpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
		break;
	}
}

WRITE8_MEMBER(xain_state::irqA_assert_w)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

WRITE8_MEMBER(xain_state::irqB_clear_w)
{
	m_subcpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

READ8_MEMBER(xain_state::m68705_r)
{
	m_mcu_ready = 1;
	return m_from_mcu;
}

WRITE8_MEMBER(xain_state::m68705_w)
{
	m_from_main = data;
	m_mcu_accept = 0;

	if (m_mcu != nullptr)
		m_mcu->set_input_line(0, ASSERT_LINE);
}

CUSTOM_INPUT_MEMBER(xain_state::vblank_r)
{
	return m_vblank;
}


/***************************************************************************

    MC68705P5 I/O

***************************************************************************/

READ8_MEMBER(xain_state::m68705_port_a_r)
{
	return (m_port_a_out & m_ddr_a) | (m_port_a_in & ~m_ddr_a);
}

WRITE8_MEMBER(xain_state::m68705_port_a_w)
{
	m_port_a_out = data;
}

WRITE8_MEMBER(xain_state::m68705_ddr_a_w)
{
	m_ddr_a = data;
}

READ8_MEMBER(xain_state::m68705_port_b_r)
{
	return (m_port_b_out & m_ddr_b) | (m_port_b_in & ~m_ddr_b);
}

WRITE8_MEMBER(xain_state::m68705_port_b_w)
{
	if ((m_ddr_b & 0x02) && (~data & 0x02))
	{
		m_port_a_in = m_from_main;
	}
	/* Rising edge of PB1 */
	else if ((m_ddr_b & 0x02) && (~m_port_b_out & 0x02) && (data & 0x02))
	{
		m_mcu_accept = 1;
		m_mcu->set_input_line(0, CLEAR_LINE);
	}

	/* Rising edge of PB2 */
	if ((m_ddr_b & 0x04) && (~m_port_b_out & 0x04) && (data & 0x04))
	{
		m_mcu_ready = 0;
		m_from_mcu = m_port_a_out;
	}

	m_port_b_out = data;
}

WRITE8_MEMBER(xain_state::m68705_ddr_b_w)
{
	m_ddr_b = data;
}

READ8_MEMBER(xain_state::m68705_port_c_r)
{
	m_port_c_in = 0;

	if (!m_mcu_accept)
		m_port_c_in |= 0x01;
	if (m_mcu_ready)
		m_port_c_in |= 0x02;

	return (m_port_c_out & m_ddr_c) | (m_port_c_in & ~m_ddr_c);
}

WRITE8_MEMBER(xain_state::m68705_port_c_w)
{
	m_port_c_out = data;
}

WRITE8_MEMBER(xain_state::m68705_ddr_c_w)
{
	m_ddr_c = data;
}

CUSTOM_INPUT_MEMBER(xain_state::mcu_status_r)
{
	UINT8 res = 0;

	if (m_mcu != nullptr)
	{
		if (m_mcu_ready == 1)
			res |= 0x01;
		if (m_mcu_accept == 1)
			res |= 0x02;
	}
	else
	{
		return 3;
	}

	return res;
}

READ8_MEMBER(xain_state::mcu_comm_reset_r)
{
	m_mcu_ready = 1;
	m_mcu_accept = 1;

	if (m_mcu != nullptr)
		m_mcu->set_input_line(0, CLEAR_LINE);

	return 0xff;
}


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, xain_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE(charram_w) AM_SHARE("charram")
	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE(bgram1_w) AM_SHARE("bgram1")
	AM_RANGE(0x3000, 0x37ff) AM_RAM_WRITE(bgram0_w) AM_SHARE("bgram0")
	AM_RANGE(0x3800, 0x397f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x3a00, 0x3a00) AM_READ_PORT("P1")
	AM_RANGE(0x3a00, 0x3a01) AM_WRITE(scrollxP1_w)
	AM_RANGE(0x3a01, 0x3a01) AM_READ_PORT("P2")
	AM_RANGE(0x3a02, 0x3a02) AM_READ_PORT("DSW0")
	AM_RANGE(0x3a02, 0x3a03) AM_WRITE(scrollyP1_w)
	AM_RANGE(0x3a03, 0x3a03) AM_READ_PORT("DSW1")
	AM_RANGE(0x3a04, 0x3a04) AM_READ(m68705_r)
	AM_RANGE(0x3a04, 0x3a05) AM_WRITE(scrollxP0_w)
	AM_RANGE(0x3a05, 0x3a05) AM_READ_PORT("VBLANK")
	AM_RANGE(0x3a06, 0x3a06) AM_READ(mcu_comm_reset_r)
	AM_RANGE(0x3a06, 0x3a07) AM_WRITE(scrollyP0_w)
	AM_RANGE(0x3a08, 0x3a08) AM_WRITE(sound_command_w)
	AM_RANGE(0x3a09, 0x3a0c) AM_WRITE(main_irq_w)
	AM_RANGE(0x3a0d, 0x3a0d) AM_WRITE(flipscreen_w)
	AM_RANGE(0x3a0e, 0x3a0e) AM_WRITE(m68705_w)
	AM_RANGE(0x3a0f, 0x3a0f) AM_WRITE(cpuA_bankswitch_w)
	AM_RANGE(0x3c00, 0x3dff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x3e00, 0x3fff) AM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu_map_B, AS_PROGRAM, 8, xain_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x2000, 0x2000) AM_WRITE(irqA_assert_w)
	AM_RANGE(0x2800, 0x2800) AM_WRITE(irqB_clear_w)
	AM_RANGE(0x3000, 0x3000) AM_WRITE(cpuB_bankswitch_w)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank2")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_map, AS_PROGRAM, 8, xain_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(m68705_port_a_r, m68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(m68705_port_b_r, m68705_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(m68705_port_c_r, m68705_port_c_w)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(m68705_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(m68705_ddr_b_w)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(m68705_ddr_c_w)
//  AM_RANGE(0x0008, 0x0008) AM_READWRITE(m68705_tdr_r, m68705_tdr_w)
//  AM_RANGE(0x0009, 0x0009) AM_READWRITE(m68705_tcr_r, m68705_tcr_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, xain_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x1000, 0x1000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x2800, 0x2801) AM_DEVWRITE("ym1", ym2203_device, write)
	AM_RANGE(0x3000, 0x3001) AM_DEVWRITE("ym2", ym2203_device, write)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( xsleena )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "Slow" )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "20k 70k and every 70k" )
	PORT_DIPSETTING(    0x20, "30k 80k and every 80k" )
	PORT_DIPSETTING(    0x10, "20k and 80k" )
	PORT_DIPSETTING(    0x00, "30k and 80k" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "3")
	PORT_DIPSETTING(    0x80, "4")
	PORT_DIPSETTING(    0x40, "6")
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")

	PORT_START("VBLANK")
	PORT_BIT( 0x03, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN3 )
	PORT_BIT( 0x18, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, xain_state, mcu_status_r, NULL)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, xain_state, vblank_r, NULL)   /* VBLANK */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 2, 4, 6 },
	{ STEP2(0*8+1,-1), STEP2(8*8+1,-1), STEP2(16*8+1,-1), STEP2(24*8+1,-1) },
	{ STEP8(0,8) },
	32*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ STEP4(0*8+3,-1), STEP4(16*8+3,-1), STEP4(32*8+3,-1), STEP4(48*8+3,-1) },
	{ STEP16(0,8) },
	64*8
};

static GFXDECODE_START( xain )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 8 )    /* 8x8 text */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 256, 8 )    /* 16x16 Background */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout, 384, 8 )    /* 16x16 Background */
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout, 128, 8 )    /* Sprites */
GFXDECODE_END


void xain_state::machine_start()
{
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x4000, 0xc000);
	membank("bank2")->configure_entries(0, 2, memregion("sub")->base()  + 0x4000, 0xc000);
	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);

	save_item(NAME(m_vblank));

	if (m_mcu)
	{
		save_item(NAME(m_from_main));
		save_item(NAME(m_from_mcu));
		save_item(NAME(m_ddr_a));
		save_item(NAME(m_ddr_b));
		save_item(NAME(m_ddr_c));
		save_item(NAME(m_port_a_out));
		save_item(NAME(m_port_b_out));
		save_item(NAME(m_port_c_out));
		save_item(NAME(m_port_a_in));
		save_item(NAME(m_port_b_in));
		save_item(NAME(m_port_c_in));
		save_item(NAME(m_mcu_ready));
		save_item(NAME(m_mcu_accept));
	}
}

static MACHINE_CONFIG_START( xsleena, xain_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", xain_state, scanline, "screen", 0, 1)

	MCFG_CPU_ADD("sub", M6809, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(cpu_map_B)

	MCFG_CPU_ADD("audiocpu", M6809, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_CPU_ADD("mcu", M68705, MCU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(mcu_map)


	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 384, 0, 256, 272, 8, 248)   /* based on ddragon driver */
	MCFG_SCREEN_UPDATE_DRIVER(xain_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", xain)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, MCU_CLOCK)
	MCFG_YM2203_IRQ_HANDLER(INPUTLINE("audiocpu", M6809_FIRQ_LINE))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 0.40)

	MCFG_SOUND_ADD("ym2", YM2203, MCU_CLOCK)
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 0.40)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( xsleenab, xsleena )
	MCFG_DEVICE_REMOVE("mcu")
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( xsleena )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "p9-08.ic66",   0x08000, 0x8000, CRC(5179ae3f) SHA1(9e4e2825e56b090aa759b0da39ccb17ccd77ede2) )
	ROM_LOAD( "pa-09.ic65",   0x04000, 0x4000, CRC(10a7c800) SHA1(f19201fe1414faed649b8e49416025aae44bcb6c) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-0.ic29",    0x08000, 0x8000, CRC(a1a860e2) SHA1(fb2b152bfafc44608039774436ddf3b17eed979c) )
	ROM_LOAD( "p0-0.ic15",    0x04000, 0x4000, CRC(948b9757) SHA1(3ea840cc47ae6a66f3e5f6a2f3e88475dcfe1840) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "pz-0.113",      0x0000, 0x0800, CRC(a432a907) SHA1(4708a40e3a82dec2c5a64bc5da884a37d503cb6b) ) /* MC68705P5S MCU internal code */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-01.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-0.ic133",   0x18000, 0x8000, CRC(d37939e0) SHA1(301d9f6720857c64a4e070444a07a38138ddd4ef) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-0.ic111",   0x38000, 0x8000, CRC(e59a2f27) SHA1(4643cea85f8613c36b416f46f9d1753fa9839237) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END

ROM_START( xsleenaj )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "p9-01.ic66",   0x08000, 0x8000, CRC(370164be) SHA1(65c9951cac7dc3943fa4d5f9919ebb4c4f29b3ae) ) /* the '1' on the label was ink stamped */
	ROM_LOAD( "pa-0.ic65",    0x04000, 0x4000, CRC(d22bf859) SHA1(9edb159bef2eba2c5d93c03c15fbcb87eea52236) ) /* Need to verify proper rom label */
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-0.ic29",    0x08000, 0x8000, CRC(a1a860e2) SHA1(fb2b152bfafc44608039774436ddf3b17eed979c) )
	ROM_LOAD( "p0-0.ic15",    0x04000, 0x4000, CRC(948b9757) SHA1(3ea840cc47ae6a66f3e5f6a2f3e88475dcfe1840) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "pz-0.113",      0x0000, 0x0800, CRC(a432a907) SHA1(4708a40e3a82dec2c5a64bc5da884a37d503cb6b) ) /* MC68705P5S MCU internal code */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-01.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-0.ic133",   0x18000, 0x8000, CRC(d37939e0) SHA1(301d9f6720857c64a4e070444a07a38138ddd4ef) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-0.ic111",   0x38000, 0x8000, CRC(e59a2f27) SHA1(4643cea85f8613c36b416f46f9d1753fa9839237) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END

ROM_START( solrwarr )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "p9-02.ic66",   0x08000, 0x8000, CRC(8ff372a8) SHA1(0fc396e662419fb9cb5bea11748aa8e0e8d072e6) )
	ROM_LOAD( "pa-03.ic65",   0x04000, 0x4000, CRC(154f946f) SHA1(25b776eb9c494e5302795ae79e494cbfc7c104b1) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-02.ic29",   0x08000, 0x8000, CRC(f5f235a3) SHA1(9f57dd7c5e514afa750edc6da6d263bf1e913c14) )
	ROM_LOAD( "p0-02.ic133",  0x04000, 0x4000, CRC(51ae95ae) SHA1(e03f7ccb0b33b05547577c60a7f92dc75e24b4d6) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "pz-0.113",      0x0000, 0x0800, CRC(a432a907) SHA1(4708a40e3a82dec2c5a64bc5da884a37d503cb6b) ) /* MC68705P5S MCU internal code */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-01.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-02.ic133",  0x18000, 0x8000, CRC(d2ed6f94) SHA1(155a0d1d978f07517400d0c602fc40657f8569dc) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-02.ic111",  0x38000, 0x8000, CRC(6e627a77) SHA1(1d16031acd53c9e691ae7eac8a6f1ae3954fac8c) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END

ROM_START( xsleenab )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "1.rom",        0x08000, 0x8000, CRC(79f515a7) SHA1(e61f18e3639dd9afe16c7bcb90fa7be31905e2c6) )
	ROM_LOAD( "pa-0.ic65",    0x04000, 0x4000, CRC(d22bf859) SHA1(9edb159bef2eba2c5d93c03c15fbcb87eea52236) ) /* Need to verify proper rom label */
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-0.ic29",    0x08000, 0x8000, CRC(a1a860e2) SHA1(fb2b152bfafc44608039774436ddf3b17eed979c) )
	ROM_LOAD( "p0-0.ic15",    0x04000, 0x4000, CRC(948b9757) SHA1(3ea840cc47ae6a66f3e5f6a2f3e88475dcfe1840) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-01.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-0.ic133",   0x18000, 0x8000, CRC(d37939e0) SHA1(301d9f6720857c64a4e070444a07a38138ddd4ef) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-0.ic111",   0x38000, 0x8000, CRC(e59a2f27) SHA1(4643cea85f8613c36b416f46f9d1753fa9839237) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END


GAME( 1986, xsleena,  0,       xsleena,  xsleena, driver_device, 0, ROT0, "Technos Japan (Taito license)", "Xain'd Sleena (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, xsleenaj, xsleena, xsleena,  xsleena, driver_device, 0, ROT0, "Technos Japan", "Xain'd Sleena (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, solrwarr, xsleena, xsleena,  xsleena, driver_device, 0, ROT0, "Technos Japan (Taito / Memetron license)", "Solar-Warrior (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, xsleenab, xsleena, xsleenab, xsleena, driver_device, 0, ROT0, "bootleg", "Xain'd Sleena (bootleg)", MACHINE_SUPPORTS_SAVE )
