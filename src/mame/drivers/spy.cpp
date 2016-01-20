// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    S.P.Y. (c) 1989 Konami

    Similar to Bottom of the Ninth

    driver by Nicola Salmoria


    Revisions:

    05-10-2002 Acho A. Tang
    - simulated PMCU protection(guess only)
    - changed priority scheme to fix graphics in 3D levels
    - fixed crashes caused by bank switching
    - disabled logging and debug messages

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"

#include "sound/3812intf.h"
#include "includes/konamipt.h"
#include "includes/spy.h"

INTERRUPT_GEN_MEMBER(spy_state::spy_interrupt)
{
	if (m_k052109->is_irq_enabled())
		device.execute().set_input_line(0, HOLD_LINE);
}

READ8_MEMBER(spy_state::spy_bankedram1_r)
{
	if (m_rambank & 1)
	{
		return m_paletteram[offset];
	}
	else if (m_rambank & 2)
	{
		if (m_pmcbank)
		{
			//logerror("%04x read pmcram %04x\n", space.device().safe_pc(), offset);
			return m_pmcram[offset];
		}
		else
		{
			//logerror("%04x read pmc internal ram %04x\n", space.device().safe_pc(), offset);
			return 0;
		}
	}
	else
		return m_ram[offset];
}

WRITE8_MEMBER(spy_state::spy_bankedram1_w)
{
	if (m_rambank & 1)
	{
		m_palette->write(space,offset,data);
	}
	else if (m_rambank & 2)
	{
		if (m_pmcbank)
		{
			//logerror("%04x pmcram %04x = %02x\n", space.device().safe_pc(), offset, data);
			m_pmcram[offset] = data;
		}
		//else
			//logerror("%04x pmc internal ram %04x = %02x\n", space.device().safe_pc(), offset, data);
	}
	else
		m_ram[offset] = data;
}

/*
this is the data written to internal ram on startup:
00: e7 7e 38 fc 08
01: df 36 38 dc 00
02: df 12 3a dc 00
03: df 00 38 dc 08
04: 1f 7e 00 db 00
05: 26 fe 00 ff 0c
06: 89 03 34 fc 0d
07: 81 03 34 fc 09
08: 81 03 34 fc 09
09: 81 03 34 fc 09
0a: 81 03 2f fc 09
0b: cc 36 0e d9 08
0c: 84 7e 00 ab 0c
0d: 5f 7e 03 cd 08
0e: 7f 80 fe ef 08
0f: 5f 7e 0f fd 08
10: e7 7e 38 fc 08
11: df 00 3a dc 00
12: df 12 0e d9 08
13: df ec 10 e0 0c
14: 1f fe 03 e0 0c
15: df fe 03 e0 0c
16: dc 5e 3e fc 08
17: df 12 2b d9 08
18: 67 25 38 fc 0c
19: df 12 3c dc 00
1a: df 36 00 db 00
1b: c1 14 00 fb 08
1c: c1 34 38 fc 08
1d: c5 22 37 dc 00
1e: cd 12 3c dc 04
1f: c5 46 3b dc 00
20: cd 36 00 db 04
21: 49 16 ed f9 0c
22: c9 18 ea f9 0c
23: dc 12 2a f9 08
24: cc 5a 26 f9 08
25: 5f 7e 18 fd 08
26: 5a 7e 32 f8 08
27: 84 6c 33 9c 0c
28: cc 00 0e d9 08
29: 5f 7e 14 fd 08
2a: 0a 7e 24 fd 08
2b: c5 ec 0d e0 0c
2c: 5f 7e 28 fd 08
2d: dc 16 00 fb 08
2e: dc 44 22 fd 08
2f: cd fe 02 e0 0c
30: 84 7e 00 bb 0c
31: 5a 7e 00 73 08
32: 84 7e 00 9b 0c
33: 5a 7e 00 36 08
34: 81 03 00 fb 09
35: 81 03 00 fb 09
36: 81 03 00 fe 09
37: cd fe 01 e0 0c
38: 84 7e 00 ab 0c
39: 5f 7e 00 db 00
3a: 84 7e 3f ad 0c
3b: cd ec 01 e0 0c
3c: 84 6c 00 ab 0c
3d: 5f 7e 00 db 00
3e: 84 6c 00 ab 0c
3f: 5f 7e 00 ce 08
*/

WRITE8_MEMBER(spy_state::bankswitch_w)
{
	int bank;

	/* bit 0 = RAM bank */
	if ((data & 1) == 0)
		popmessage("bankswitch RAM bank 0");

	/* bit 1-4 = ROM bank */
	if (data & 0x10)
		bank = 8 + ((data & 0x06) >> 1);
	else
		bank = ((data & 0x0e) >> 1);

	membank("bank1")->set_entry(bank);
}

void spy_state::spy_collision(  )
{
#define MAX_SPRITES 64
#define DEF_NEAR_PLANE 0x6400
#define NEAR_PLANE_ZOOM 0x0100
#define FAR_PLANE_ZOOM 0x0000

	int op1, x1, w1, z1, d1, y1, h1;
	int op2, x2, w2, z2, d2, y2, h2;
	int mode, i, loopend, nearplane;

	mode = m_pmcram[0x1];
	op1 = m_pmcram[0x2];
	if (op1 == 1)
	{
		x1 = (m_pmcram[0x3] << 8) + m_pmcram[0x4];
		w1 = (m_pmcram[0x5] << 8) + m_pmcram[0x6];
		z1 = (m_pmcram[0x7] << 8) + m_pmcram[0x8];
		d1 = (m_pmcram[0x9] << 8) + m_pmcram[0xa];
		y1 = (m_pmcram[0xb] << 8) + m_pmcram[0xc];
		h1 = (m_pmcram[0xd] << 8) + m_pmcram[0xe];

		for (i = 16; i < 14 * MAX_SPRITES + 2; i += 14)
		{
			op2 = m_pmcram[i];
			if (op2 || mode == 0x0c)
			{
				x2 = (m_pmcram[i + 0x1] << 8) + m_pmcram[i + 0x2];
				w2 = (m_pmcram[i + 0x3] << 8) + m_pmcram[i + 0x4];
				z2 = (m_pmcram[i + 0x5] << 8) + m_pmcram[i + 0x6];
				d2 = (m_pmcram[i + 0x7] << 8) + m_pmcram[i + 0x8];
				y2 = (m_pmcram[i + 0x9] << 8) + m_pmcram[i + 0xa];
				h2 = (m_pmcram[i + 0xb] << 8) + m_pmcram[i + 0xc];
/*
    The mad scientist's laser truck has both a high sprite center and a small height value.
    It has to be measured from the ground to detect correctly.
*/
				if (w2 == 0x58 && d2 == 0x04 && h2 == 0x10 && y2 == 0x30)
					h2 = y2;

				// what other sprites fall into:
				if ((abs(x1 - x2) < w1 + w2) && (abs(z1 - z2) < d1 + d2) && (abs(y1 - y2) < h1 + h2))
				{
					m_pmcram[0xf] = 0;
					m_pmcram[i + 0xd] = 0;
				}
				else
					m_pmcram[i + 0xd] = 1;
			}
		}
	}
	else if (op1 > 1)
	{
/*
    The PMCU also projects geometries to screen coordinates. Unfortunately I'm unable to figure
    the scale factors from the PMCU code. Plugging 0 and 0x100 to the far and near planes seems
    to do the trick though.
*/
		loopend = (m_pmcram[0] << 8) + m_pmcram[1];
		nearplane = (m_pmcram[2] << 8) + m_pmcram[3];

		// fail safe
		if (loopend > MAX_SPRITES)
			loopend = MAX_SPRITES;
		if (!nearplane)
			nearplane = DEF_NEAR_PLANE;

		loopend = (loopend << 1) + 4;

		for (i = 4; i < loopend; i += 2)
		{
			op2 = (m_pmcram[i] << 8) + m_pmcram[i + 1];
			op2 = (op2 * (NEAR_PLANE_ZOOM - FAR_PLANE_ZOOM)) / nearplane + FAR_PLANE_ZOOM;
			m_pmcram[i] = op2 >> 8;
			m_pmcram[i + 1] = op2 & 0xff;
		}

		memset(m_pmcram + loopend, 0, 0x800 - loopend); // clean up for next frame
	}
}


WRITE8_MEMBER(spy_state::spy_3f90_w)
{
	/*********************************************************************
	*
	* Signals, from schematic:
	*   Bit 0 - CTR1 0x01
	*   Bit 1 - CTR2 0x02
	*   Bit 2 - CHA-RD 0x04
	*   Bit 3 - TV-KILL 0x08  +TV-KILL & COLORBLK to pin 7 of
	*                                    052535 video chips
	*
	*   Bit 4 - COLORBK/RVBK 0x10
	*   Bit 5 - PMCBK 0x20  GX857 053180 PAL20P Pin 7 (MCE1)
	*   Bit 6 - PMC-START 0x40  PMC START
	*   Bit 7 - PMC-BK 0x80  PMC BK
	*
	*   PMC takes AB0-AB12, D0-D7 from 6809E, outputs EA0-EA10, ED0-ED7,
	*   tied to A and D bus of 2128SL
	*
	*   See "MCPU" page of S.P.Y schematics for more...
	*
	*    PMC ERWE -> ~WR of 2128SL
	*    PMC ERCS -> ~CE of 2128SL
	*    PMC EROE -> ~OE of 2128SL
	*
	*    PMCOUTO -> PMCFIRQ -> 6809E ~FIRQ and PORT4, bit 0x08
	*
	*   PMC selected by PMC/RVRAMCS signal: pin 16 of PAL20P 05318
	*
	*    AB0xC -> 0x1000, so if address & 0x1000, appears PMC is selected.
	*
	*   Other apparent selects:
	*
	*    0x0800 -> COLORCS (color enable?)
	*    0x2000 -> ~CS1 on 6264W
	*    0x4000 -> ~OE on S63 27512
	*    0x8000 -> ~OE on S22 27512
	*
	********************************************************************/

	/* bits 0/1 = coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	/* bit 2 = enable char ROM reading through the video RAM */
	m_k052109->set_rmrd_line((data & 0x04) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 3 = disable video */
	m_video_enable = ~(data & 0x08);

	/* bit 4 = read RAM at 0000 (if set) else read color palette RAM */
	/* bit 5 = PMCBK */
	m_rambank = (data & 0x30) >> 4;
	/* bit 7 = PMC-BK */
	m_pmcbank = (data & 0x80) >> 7;

//logerror("%04x: 3f90_w %02x\n", space.device().safe_pc(), data);
	/* bit 6 = PMC-START */
	if ((data & 0x40) && !(m_old_3f90 & 0x40))
	{
		/* we should handle collision here */
//AT
		if (0)
		{
			int i;

			logerror("collision test:\n");
			for (i = 0; i < 0xfe; i++)
			{
				logerror("%02x ", m_pmcram[i]);
				if (i == 0x0f || (i > 0x10 && (i - 0x10) % 14 == 13))
				logerror("\n");
			}
		}
		spy_collision();
//ZT
		m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
	}

	m_old_3f90 = data;
}


WRITE8_MEMBER(spy_state::spy_sh_irqtrigger_w)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}

WRITE8_MEMBER(spy_state::sound_bank_w)
{
	int bank_A, bank_B;

	bank_A = (data >> 0) & 0x03;
	bank_B = (data >> 2) & 0x03;
	m_k007232_1->set_bank(bank_A, bank_B);

	bank_A = (data >> 4) & 0x03;
	bank_B = (data >> 6) & 0x03;
	m_k007232_2->set_bank(bank_A, bank_B);
}


READ8_MEMBER(spy_state::k052109_051960_r)
{
	if (m_k052109->get_rmrd_line() == CLEAR_LINE)
	{
		if (offset >= 0x3800 && offset < 0x3808)
			return m_k051960->k051937_r(space, offset - 0x3800);
		else if (offset < 0x3c00)
			return m_k052109->read(space, offset);
		else
			return m_k051960->k051960_r(space, offset - 0x3c00);
	}
	else
		return m_k052109->read(space, offset);
}

WRITE8_MEMBER(spy_state::k052109_051960_w)
{
	if (offset >= 0x3800 && offset < 0x3808)
		m_k051960->k051937_w(space, offset - 0x3800, data);
	else if (offset < 0x3c00)
		m_k052109->write(space, offset, data);
	else
		m_k051960->k051960_w(space, offset - 0x3c00, data);
}

static ADDRESS_MAP_START( spy_map, AS_PROGRAM, 8, spy_state )
	AM_RANGE(0x0000, 0x07ff) AM_READWRITE(spy_bankedram1_r, spy_bankedram1_w) AM_SHARE("ram")
	AM_RANGE(0x0800, 0x1aff) AM_RAM
	AM_RANGE(0x3f80, 0x3f80) AM_WRITE(bankswitch_w)
	AM_RANGE(0x3f90, 0x3f90) AM_WRITE(spy_3f90_w)
	AM_RANGE(0x3fa0, 0x3fa0) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x3fb0, 0x3fb0) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x3fc0, 0x3fc0) AM_WRITE(spy_sh_irqtrigger_w)
	AM_RANGE(0x3fd0, 0x3fd0) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x3fd1, 0x3fd1) AM_READ_PORT("P1")
	AM_RANGE(0x3fd2, 0x3fd2) AM_READ_PORT("P2")
	AM_RANGE(0x3fd3, 0x3fd3) AM_READ_PORT("DSW1")
	AM_RANGE(0x3fe0, 0x3fe0) AM_READ_PORT("DSW2")
	AM_RANGE(0x2000, 0x5fff) AM_READWRITE(k052109_051960_r, k052109_051960_w)
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( spy_sound_map, AS_PROGRAM, 8, spy_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_WRITE(sound_bank_w)
	AM_RANGE(0xa000, 0xa00d) AM_DEVREADWRITE("k007232_1", k007232_device, read, write)
	AM_RANGE(0xb000, 0xb00d) AM_DEVREADWRITE("k007232_2", k007232_device, read, write)
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)
	AM_RANGE(0xd000, 0xd000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( spy )
	PORT_START("P1")
	KONAMI8_ALT_B21(1)  /* button 3 unused */

	PORT_START("P2")
	KONAMI8_ALT_B21(2)  /* button 3 unused */

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "10k and every 20k" )
	PORT_DIPSETTING(    0x10, "20k and every 30k" )
	PORT_DIPSETTING(    0x08, "20k only" )
	PORT_DIPSETTING(    0x00, "30k only" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL )        /* PMCFIRQ signal from the PMC */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW3:2" )
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x00, "5 Times" )
	PORT_DIPSETTING(    0x80, "Unlimited" )
INPUT_PORTS_END



WRITE8_MEMBER(spy_state::volume_callback0)
{
	m_k007232_1->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232_1->set_volume(1, 0, (data & 0x0f) * 0x11);
}

WRITE8_MEMBER(spy_state::volume_callback1)
{
	m_k007232_2->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232_2->set_volume(1, 0, (data & 0x0f) * 0x11);
}


void spy_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 12, &ROM[0x10000], 0x2000);

	m_paletteram.resize(0x800);
	m_palette->basemem().set(m_paletteram, ENDIANNESS_BIG, 2);

	memset(m_pmcram, 0, sizeof(m_pmcram));

	save_item(NAME(m_paletteram));
	save_item(NAME(m_rambank));
	save_item(NAME(m_pmcbank));
	save_item(NAME(m_video_enable));
	save_item(NAME(m_old_3f90));
	save_item(NAME(m_pmcram));
}

void spy_state::machine_reset()
{
	m_rambank = 0;
	m_pmcbank = 0;
	m_video_enable = 0;
	m_old_3f90 = -1;
}

static MACHINE_CONFIG_START( spy, spy_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 3000000) /* ? */
	MCFG_CPU_PROGRAM_MAP(spy_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", spy_state,  spy_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(spy_sound_map) /* nmi by the sound chip */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(13*8, (64-13)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(spy_state, screen_update_spy)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_ENABLE_SHADOWS()
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)

	MCFG_DEVICE_ADD("k052109", K052109, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K052109_CB(spy_state, tile_callback)

	MCFG_DEVICE_ADD("k051960", K051960, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051960_SCREEN_TAG("screen")
	MCFG_K051960_CB(spy_state, sprite_callback)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, 3579545)
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", INPUT_LINE_NMI))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("k007232_1", K007232, 3579545)
	MCFG_K007232_PORT_WRITE_HANDLER(WRITE8(spy_state, volume_callback0))
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)

	MCFG_SOUND_ADD("k007232_2", K007232, 3579545)
	MCFG_K007232_PORT_WRITE_HANDLER(WRITE8(spy_state, volume_callback1))
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)
MACHINE_CONFIG_END


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( spy )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* code + banked roms + space for banked ram */
	ROM_LOAD( "857n03.bin",   0x10000, 0x10000, CRC(97993b38) SHA1(0afd561bc85fcbfe30f2d16807424ceec7188ce7) )
	ROM_LOAD( "857n02.bin",   0x20000, 0x08000, CRC(31a97efe) SHA1(6c9ec3954e4d16634bf95835b8b404d3a6ef6e24) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code */
	ROM_LOAD( "857d01.bin",   0x0000, 0x8000, CRC(aad4210f) SHA1(bb40b8673939b5ce51012606da86b4dcbfc52a57) )

	ROM_REGION( 0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "857b09.bin",   0x00000, 0x40000, CRC(b8780966) SHA1(6c255f1e4d1398fa9010a1ae0f5172dc524df109) )
	ROM_LOAD32_WORD( "857b08.bin",   0x00002, 0x40000, CRC(3e4d8d50) SHA1(70f45a725bf1e9d15285ffb6b280945f7ce7faf0) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "857b06.bin",   0x00000, 0x80000, CRC(7b515fb1) SHA1(3830649d47964940023760b76e2bf94bb9163f23) )
	ROM_LOAD32_WORD( "857b05.bin",   0x00002, 0x80000, CRC(27b0f73b) SHA1(6b6a3da11c3005e3a62e6280818c18ae2ea31800) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "857a10.bin",   0x0000, 0x0100, CRC(32758507) SHA1(c21f89ad253502968a755fb0d23da98319f9cd93) )    /* priority encoder (not used) */

	ROM_REGION( 0x40000, "k007232_1", 0 ) /* samples for 007232 #0 */
	ROM_LOAD( "857b07.bin",   0x00000, 0x40000, CRC(ce3512d4) SHA1(1e7c3feabfc3ac89056982b76de39e283cf5894d) )

	ROM_REGION( 0x40000, "k007232_2", 0 ) /* samples for 007232 #1 */
	ROM_LOAD( "857b04.bin",   0x00000, 0x40000, CRC(20b83c13) SHA1(63062f1c0a9adbbced3d3d73682a2cd1217bee7d) )
ROM_END

ROM_START( spyu )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* code + banked roms + space for banked ram */
	ROM_LOAD( "857m03.bin",   0x10000, 0x10000, CRC(3bd87fa4) SHA1(257371ef31c8adcdc04f46e989b7a2f3531c2ab1) )
	ROM_LOAD( "857m02.bin",   0x20000, 0x08000, CRC(306cc659) SHA1(91d150b8d320bf19c12bc46103ffdffacf4387c3) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code */
	ROM_LOAD( "857d01.bin",   0x0000, 0x8000, CRC(aad4210f) SHA1(bb40b8673939b5ce51012606da86b4dcbfc52a57) )

	ROM_REGION( 0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "857b09.bin",   0x00000, 0x40000, CRC(b8780966) SHA1(6c255f1e4d1398fa9010a1ae0f5172dc524df109) )
	ROM_LOAD32_WORD( "857b08.bin",   0x00002, 0x40000, CRC(3e4d8d50) SHA1(70f45a725bf1e9d15285ffb6b280945f7ce7faf0) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "857b06.bin",   0x00000, 0x80000, CRC(7b515fb1) SHA1(3830649d47964940023760b76e2bf94bb9163f23) )
	ROM_LOAD32_WORD( "857b05.bin",   0x00002, 0x80000, CRC(27b0f73b) SHA1(6b6a3da11c3005e3a62e6280818c18ae2ea31800) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "857a10.bin",   0x0000, 0x0100, CRC(32758507) SHA1(c21f89ad253502968a755fb0d23da98319f9cd93) )    /* priority encoder (not used) */

	ROM_REGION( 0x40000, "k007232_1", 0 ) /* samples for 007232 #0 */
	ROM_LOAD( "857b07.bin",   0x00000, 0x40000, CRC(ce3512d4) SHA1(1e7c3feabfc3ac89056982b76de39e283cf5894d) )

	ROM_REGION( 0x40000, "k007232_2", 0 ) /* samples for 007232 #1 */
	ROM_LOAD( "857b04.bin",   0x00000, 0x40000, CRC(20b83c13) SHA1(63062f1c0a9adbbced3d3d73682a2cd1217bee7d) )
ROM_END


GAME( 1989, spy,  0,   spy, spy, driver_device, 0, ROT0, "Konami", "S.P.Y. - Special Project Y (World ver. N)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, spyu, spy, spy, spy, driver_device, 0, ROT0, "Konami", "S.P.Y. - Special Project Y (US ver. M)", MACHINE_SUPPORTS_SAVE )
