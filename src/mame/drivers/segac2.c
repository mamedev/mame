// license:BSD-3-Clause
// copyright-holders:David Haywood, Aaron Giles
/***********************************************************************************************

    Sega System C (System 14)/C2 Driver
    driver by David Haywood and Aaron Giles
    ---------------------------------------
    Last Update 15 Nov 2005


    Sega's C2 was used between 1989 and 1994, the hardware being very similar to that
    used by the Sega MegaDrive/Genesis Home Console Sega produced around the same time.

    The Columns and Bloxeed (USA) manual call this system 'System 14' instead of system C/C2.


    todo: fill in protection chip data

    Year  Game                       Developer         Protection Chip  Board  Number
    ====  ====================       ================  ===============  =====  =========
    1989  Bloxeed                    Sega / Elorg      n/a              C
    1989  Bloxeed (USA)              Sega / Elorg      317-0140         C      171-5880B
    1990  Columns                    Sega              317-0149         C      171-5880B
    1990  Columns II                 Sega              317-0160         C
    1990  Thunder Force AC           Sega / Technosoft 317-0172         C2
    1990  Borench                    Sega              317-0173         C2
    1991  Twin Squash                Sega              317-0193         C2
    1992  Ribbit!                    Sega              317-0178         C2
    1992  Puyo Puyo                  Sega / Compile    317-0203         C2     171-5992A
    1992  Tant-R (Japan)             Sega              317-0211         C2
    1992  Tant-R (Korea)             Sega              ?                C2
    1994  PotoPoto (Japan)           Sega              317-0218         C2
    1994  Stack Columns (Japan)      Sega              317-0219         C2
    1994  Stack Columns (World)      Sega              317-0223         C2
    1994  Ichidant-R (Japan)         Sega              317-0224         C2
    1994  Ichidant-R (World)         Sega              ?                C2
    1994  Ichidant-R (Korea)         Sega              ?                C2
    1994  Puyo Puyo 2                Compile           317-0228         C2
    1994  Zunzunkyou No Yabou        Sega              ?                C2

    1995  Print Club (Vol.1)         Atlus             ?                C2
    1995  Print Club (Vol.2)         Atlus             ?                C2
    1996  Print Club (Vol.4)         Atlus             ?                C2
    1996  Print Club (Vol.5)         Atlus             ?                C2


     Notes:
            Bloxeed doesn't Read from the Protection Chip at all; all of the other games do.
            Currently the protection chip is mostly understood, and needs a table of 256
            4-bit values for each game. In all cases except for Poto Poto and Puyo Puyo 2,
            the table is embedded in the code. Workarounds for the other 2 cases are
            provided.

            I'm assuming System-C was the Board without the uPD7759 chip and System-C2 was the
            version of the board with it, this could be completely wrong but it doesn't really
            matter anyway.

    Bugs:   Puyo Puyo ends up with a black screen after doing memory tests
            Battery-backed RAM needs to be figured out

    Thanks: (in no particular order) to any MameDev that helped me out .. (OG, Mish etc.)
            Charles MacDonald for his C2Emu .. without it working out what were bugs in my code
                and issues due to protection would have probably killed the driver long ago :p
            Razoola & Antiriad .. for helping teach me some 68k ASM needed to work out just why
                the games were crashing :)
            Sega for producing some Fantastic Games...
            and anyone else who knows they've contributed :)

***********************************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/315_5296.h"
#include "sound/okim6295.h"
#include "sound/sn76496.h"
#include "sound/2612intf.h"
#include "sound/upd7759.h"
#include "includes/segaipt.h"

#include "includes/megadriv.h"


#define XL1_CLOCK           XTAL_640kHz
#define XL2_CLOCK           XTAL_53_693175MHz


#define LOG_PROTECTION      1
#define LOG_PALETTE         0
#define LOG_IOCHIP          0

typedef device_delegate<int (int in)> segac2_prot_delegate;

class segac2_state : public md_base_state
{
public:
	segac2_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag),
	m_paletteram(*this, "paletteram"),
	m_upd7759(*this, "upd"),
	m_screen(*this, "screen"),
	m_palette(*this, "palette") { }

	// for Print Club only
	int m_cam_data;

	int m_segac2_enable_display;

	required_shared_ptr<UINT16> m_paletteram;

	/* protection-related tracking */
	segac2_prot_delegate m_prot_func;     /* emulation of protection chip */
	UINT8       m_prot_write_buf;       /* remembers what was written */
	UINT8       m_prot_read_buf;        /* remembers what was returned */

	/* palette-related variables */
	UINT8       m_segac2_alt_palette_mode;
	UINT8       m_palbank;
	UINT8       m_bg_palbase;
	UINT8       m_sp_palbase;

	/* sound-related variables */
	UINT8       m_sound_banks;      /* number of sound banks */

	DECLARE_DRIVER_INIT(c2boot);
	DECLARE_DRIVER_INIT(bloxeedc);
	DECLARE_DRIVER_INIT(columns);
	DECLARE_DRIVER_INIT(columns2);
	DECLARE_DRIVER_INIT(tfrceac);
	DECLARE_DRIVER_INIT(tfrceacb);
	DECLARE_DRIVER_INIT(borench);
	DECLARE_DRIVER_INIT(twinsqua);
	DECLARE_DRIVER_INIT(ribbit);
	DECLARE_DRIVER_INIT(puyo);
	DECLARE_DRIVER_INIT(tantr);
	DECLARE_DRIVER_INIT(tantrkor);
	DECLARE_DRIVER_INIT(potopoto);
	DECLARE_DRIVER_INIT(stkclmns);
	DECLARE_DRIVER_INIT(stkclmnj);
	DECLARE_DRIVER_INIT(ichir);
	DECLARE_DRIVER_INIT(ichirk);
	DECLARE_DRIVER_INIT(ichirj);
	DECLARE_DRIVER_INIT(ichirjbl);
	DECLARE_DRIVER_INIT(puyopuy2);
	DECLARE_DRIVER_INIT(zunkyou);
	DECLARE_DRIVER_INIT(pclub);
	DECLARE_DRIVER_INIT(pclubjv2);
	DECLARE_DRIVER_INIT(pclubjv4);
	DECLARE_DRIVER_INIT(pclubjv5);
	void segac2_common_init(segac2_prot_delegate prot_func);
	DECLARE_VIDEO_START(segac2_new);
	DECLARE_MACHINE_START(segac2);
	DECLARE_MACHINE_RESET(segac2);

	UINT32 screen_update_segac2_new(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int m_segac2_bg_pal_lookup[4];
	int m_segac2_sp_pal_lookup[4];
	void recompute_palette_tables();

	DECLARE_WRITE_LINE_MEMBER(vdp_sndirqline_callback_c2);
	DECLARE_WRITE_LINE_MEMBER(vdp_lv6irqline_callback_c2);
	DECLARE_WRITE_LINE_MEMBER(vdp_lv4irqline_callback_c2);

	DECLARE_READ8_MEMBER(io_portc_r);
	DECLARE_WRITE8_MEMBER(io_portd_w);
	DECLARE_WRITE8_MEMBER(io_porth_w);

	DECLARE_WRITE16_MEMBER( segac2_upd7759_w );
	DECLARE_READ16_MEMBER( palette_r );
	DECLARE_WRITE16_MEMBER( palette_w );
	DECLARE_WRITE8_MEMBER( control_w );
	DECLARE_READ8_MEMBER( prot_r );
	DECLARE_WRITE8_MEMBER( prot_w );
	DECLARE_WRITE8_MEMBER( counter_timer_w );
	DECLARE_READ16_MEMBER( printer_r );
	DECLARE_WRITE16_MEMBER( print_club_camera_w );
	DECLARE_READ16_MEMBER(ichirjbl_prot_r);
	DECLARE_WRITE_LINE_MEMBER(segac2_irq2_interrupt);
	optional_device<upd7759_device> m_upd7759;
	optional_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	int prot_func_dummy(int in);
	int prot_func_columns(int in);
	int prot_func_columns2(int in);
	int prot_func_tfrceac(int in);
	int prot_func_borench(int in);
	int prot_func_ribbit(int in);
	int prot_func_twinsqua(int in);
	int prot_func_puyo(int in);
	int prot_func_tantr(int in);
	int prot_func_tantrkor(int in);
	int prot_func_potopoto(int in);
	int prot_func_stkclmnj(int in);
	int prot_func_stkclmns(int in);
	int prot_func_ichirj(int in);
	int prot_func_ichir(int in);
	int prot_func_ichirk(int in);
	int prot_func_puyopuy2(int in);
	int prot_func_zunkyou(int in);
	int prot_func_pclub(int in);
	int prot_func_pclubjv2(int in);
	int prot_func_pclubjv4(int in);
	int prot_func_pclubjv5(int in);
};


/******************************************************************************
    Machine init
*******************************************************************************

    This is called at init time, when it's safe to create a timer. We use
    it to prime the scanline interrupt timer.

******************************************************************************/

MACHINE_START_MEMBER(segac2_state,segac2)
{
	save_item(NAME(m_prot_write_buf));
	save_item(NAME(m_prot_read_buf));

	m_vdp->stop_timers();
}


MACHINE_RESET_MEMBER(segac2_state,segac2)
{
//  megadriv_scanline_timer = machine().device<timer_device>("md_scan_timer");
//  megadriv_scanline_timer->adjust(attotime::zero);
	m_segac2_bg_pal_lookup[0] = 0x00;
	m_segac2_bg_pal_lookup[1] = 0x10;
	m_segac2_bg_pal_lookup[2] = 0x20;
	m_segac2_bg_pal_lookup[3] = 0x30;

	m_segac2_sp_pal_lookup[0] = 0x00;
	m_segac2_sp_pal_lookup[1] = 0x10;
	m_segac2_sp_pal_lookup[2] = 0x20;
	m_segac2_sp_pal_lookup[3] = 0x30;

	m_vdp->device_reset_old();

	/* determine how many sound banks */
	m_sound_banks = 0;
	if (memregion("upd")->base())
		m_sound_banks = memregion("upd")->bytes() / 0x20000;

	/* reset the protection */
	m_prot_write_buf = 0;
	m_prot_read_buf = 0;
	m_segac2_alt_palette_mode = 0;

	m_palbank = 0;
	m_bg_palbase = 0;
	m_sp_palbase = 0;

	recompute_palette_tables();
}


/******************************************************************************
    Sound handlers
*******************************************************************************

    These handlers are responsible for communicating with the (genenerally)
    8-bit sound chips. All accesses are via the low byte.

    The Sega C/C2 system uses a YM3438 (compatible with the YM2612) for FM-
    based music generation, and an SN76489 for PSG and noise effects. The
    C2 board also appears to have a UPD7759 for sample playback.

******************************************************************************/

/* handle writes to the UPD7759 */
WRITE16_MEMBER(segac2_state::segac2_upd7759_w)
{
	/* make sure we have a UPD chip */
	if (m_upd7759 == NULL)
		return;

	/* only works if we're accessing the low byte */
	if (ACCESSING_BITS_0_7)
	{
		m_upd7759->port_w(space, 0, data & 0xff);
		m_upd7759->start_w(0);
		m_upd7759->start_w(1);
	}
}


/******************************************************************************
    Palette RAM Read / Write Handlers
*******************************************************************************

    The following Read / Write Handlers are used when accessing Palette RAM.
    The C2 Hardware appears to use 4 Banks of Colours 1 of which can be Mapped
    to 0x8C0000 - 0x8C03FF at any given time by writes to 0x84000E (This same
    address also looks to be used for things like Sample Banking)

    Each Colour uses 15-bits (from a 16-bit word) in the Format
        xBGRBBBB GGGGRRRR  (x = unused, B = Blue, G = Green, R = Red)

    As this works out the Palette RAM Stores 2048 from a Possible 4096 Colours
    at any given time.

******************************************************************************/

/* handle reads from the paletteram */
READ16_MEMBER(segac2_state::palette_r)
{
	offset &= 0x1ff;
	if (m_segac2_alt_palette_mode)
		offset = ((offset << 1) & 0x100) | ((offset << 2) & 0x80) | ((~offset >> 2) & 0x40) | ((offset >> 1) & 0x20) | (offset & 0x1f);

	return m_paletteram[offset + m_palbank * 0x200];
}

/* handle writes to the paletteram */
WRITE16_MEMBER(segac2_state::palette_w)
{
	int r, g, b, newword;
	int tmpr, tmpg, tmpb;

	/* adjust for the palette bank */
	offset &= 0x1ff;
	if (m_segac2_alt_palette_mode)
		offset = ((offset << 1) & 0x100) | ((offset << 2) & 0x80) | ((~offset >> 2) & 0x40) | ((offset >> 1) & 0x20) | (offset & 0x1f);
	offset += m_palbank * 0x200;

	/* combine data */
	COMBINE_DATA(&m_paletteram[offset]);
	newword = m_paletteram[offset];

	/* up to 8 bits */
	r = ((newword << 1) & 0x1e) | ((newword >> 12) & 0x01);
	g = ((newword >> 3) & 0x1e) | ((newword >> 13) & 0x01);
	b = ((newword >> 7) & 0x1e) | ((newword >> 14) & 0x01);

	/* set the color */
	m_palette->set_pen_color(offset, pal5bit(r), pal5bit(g), pal5bit(b));

	tmpr = r >> 1;
	tmpg = g >> 1;
	tmpb = b >> 1;
	m_palette->set_pen_color(offset + 0x800, pal5bit(tmpr), pal5bit(tmpg), pal5bit(tmpb));

	// how is it calculated on c2?
	tmpr = tmpr | 0x10;
	tmpg = tmpg | 0x10;
	tmpb = tmpb | 0x10;
	m_palette->set_pen_color(offset + 0x1000, pal5bit(tmpr), pal5bit(tmpg), pal5bit(tmpb));
}


/******************************************************************************
    Palette Tables
*******************************************************************************

    Both the background and sprites within the VDP have 4 possible palettes
    to select from. External hardware on the C2 boards, controlled by the
    EPM5032 chip and the I/O chip, allows for more complex palette selection.
    The actual palette entry comes from:

        Bits 10-9 = output from I/O port H
        Bits  8-5 = output from EPM5032
        Bits  4-0 = direct from the VDP

    In order to compute bits 8-5, the EPM5032 gets the sprite/background
    output line along with the two bits of palette info. For most games, the
    two bits of palette info map straight through as follows:

        Bits 10-9 = output from I/O port H
        Bits    8 = sprite/background select
        Bits  7-6 = palette selected by writing to prot_w
        Bits  5-4 = direct from the VDP palette select
        Bits  3-0 = direct from the VDP

    However, because there are 4 bits completely controlled by the EPM5032,
    it doesn't have to always map this way. An alternate palette mode can
    be selected which alters the output palette by swizzling the color
    RAM address bits.

******************************************************************************/

void segac2_state::recompute_palette_tables()
{
	int i;

	for (i = 0; i < 4; i++)
	{
		int bgpal = 0x000 + m_bg_palbase * 0x40 + i * 0x10;
		int sppal = 0x100 + m_sp_palbase * 0x40 + i * 0x10;

		if (!m_segac2_alt_palette_mode)
		{
			m_segac2_bg_pal_lookup[i] = 0x200 * m_palbank + bgpal;
			m_segac2_sp_pal_lookup[i] = 0x200 * m_palbank + sppal;
		}
		else
		{
			m_segac2_bg_pal_lookup[i] = 0x200 * m_palbank + ((bgpal << 1) & 0x180) + ((~bgpal >> 2) & 0x40) + (bgpal & 0x30);
			m_segac2_sp_pal_lookup[i] = 0x200 * m_palbank + ((~sppal << 2) & 0x100) + ((sppal << 2) & 0x80) + ((~sppal >> 2) & 0x40) + ((sppal >> 2) & 0x20) + (sppal & 0x10);
		}
	}
}


/******************************************************************************
    Sega 315-5296 I/O chip
******************************************************************************/

READ8_MEMBER(segac2_state::io_portc_r)
{
	// D7 : From MB3773P pin 1. (/RESET output)
	// D6 : From uPD7759 pin 18. (/BUSY output)
	int busy = (m_upd7759 != NULL) ? (m_upd7759->busy_r() << 6) : 0x40;
	return 0xbf | busy;
}

WRITE8_MEMBER(segac2_state::io_portd_w)
{
	/*
	 D7 : To pin 3 of JP15. (Watchdog clock control)
	 D6 : To MUTE input pin on TDA1518BQ amplifier.
	 D5 : To CN2 pin 10. (Unknown purpose)
	 D4 : To CN2 pin 11. (Unknown purpose)
	 D3 : To CN1 pin K. (Coin lockout 2)
	 D2 : To CN1 pin 9. (Coin lockout 1)
	 D1 : To CN1 pin J. (Coin meter 2)
	 D0 : To CN1 pin 8. (Coin meter 1)
	*/
	//coin_lockout_w(space.machine(), 1, data & 0x08);
	//coin_lockout_w(space.machine(), 0, data & 0x04);
	coin_counter_w(space.machine(), 1, data & 0x02);
	coin_counter_w(space.machine(), 0, data & 0x01);
}

WRITE8_MEMBER(segac2_state::io_porth_w)
{
	/*
	 D7 : To pin A19 of CN4
	 D6 : To pin B19 of CN4
	 D5 : ?
	 D4 : ?
	 D3 : To pin 31 of uPD7759 sample ROM (A18 on a 27C040)
	 D2 : To pin 30 of uPD7759 sample ROM (A17 on a 27C040)
	 D1 : To A10 of color RAM
	 D0 : To A9 of color RAM
	*/
	int newbank = data & 3;
	if (newbank != m_palbank)
	{
		//m_screen->update_partial(m_screen->vpos() + 1);
		m_palbank = newbank;
		recompute_palette_tables();
	}
	if (m_sound_banks > 1)
	{
		newbank = (data >> 2) & (m_sound_banks - 1);
		m_upd7759->set_bank_base(newbank * 0x20000);
	}
}


/******************************************************************************
    Control Write Handler
*******************************************************************************

    Seems to control some global states. The most important bit is the low
    one, which enables/disables the display. This is used while tiles are
    being modified in Bloxeed.

******************************************************************************/

WRITE8_MEMBER(segac2_state::control_w)
{
	data &= 0x0f;

	/* bit 0 controls display enable */
	//segac2_enable_display(space.machine(), ~data & 1);
	m_segac2_enable_display = ~data & 1;

	/* bit 1 resets the protection */
	if (!(data & 2))
		m_prot_write_buf = m_prot_read_buf = 0;

	/* bit 2 controls palette shuffling; only ribbit and twinsqua use this feature */
	m_segac2_alt_palette_mode = ((~data & 4) >> 2);
	recompute_palette_tables();
}


/******************************************************************************
    Protection Read / Write Handlers
*******************************************************************************

    The protection chip is fairly simple. Writes to it control the palette
    banking for the sprites and backgrounds. The low 4 bits are also
    remembered in a 2-stage FIFO buffer. A read from this chip should
    return a value from a 256x4-bit table. The index into this table is
    computed by taking the second-to-last value written in the upper 4 bits,
    and the previously-fetched table value in the lower 4 bits.

******************************************************************************/

/* protection chip reads */
READ8_MEMBER(segac2_state::prot_r)
{
	if (LOG_PROTECTION) logerror("%06X:protection r=%02X\n", space.device().safe_pcbase(), m_prot_read_buf);
	return m_prot_read_buf | 0xf0;
}


/* protection chip writes */
WRITE8_MEMBER(segac2_state::prot_w)
{
	int new_sp_palbase = (data >> 2) & 3;
	int new_bg_palbase = data & 3;
	int table_index;

	/* compute the table index */
	table_index = (m_prot_write_buf << 4) | m_prot_read_buf;

	/* keep track of the last write for the next table lookup */
	m_prot_write_buf = data & 0x0f;

	/* determine the value to return, should a read occur */
	m_prot_read_buf = m_prot_func(table_index);
	if (LOG_PROTECTION) logerror("%06X:protection w=%02X, new result=%02X\n", space.device().safe_pcbase(), data & 0x0f, m_prot_read_buf);

	/* if the palette changed, force an update */
	if (new_sp_palbase != m_sp_palbase || new_bg_palbase != m_bg_palbase)
	{
		//m_screen->update_partial(m_screen->vpos() + 1);
		m_sp_palbase = new_sp_palbase;
		m_bg_palbase = new_bg_palbase;
		recompute_palette_tables();
		if (LOG_PALETTE) logerror("Set palbank: %d/%d (scan=%d)\n", m_bg_palbase, m_sp_palbase, m_screen->vpos());
	}
}


/******************************************************************************
    Counter/timer I/O
*******************************************************************************

    There appears to be a chip that is used to count coins and track time
    played, or at the very least the current status of the game. All games
    except Puyo Puyo 2 and Poto Poto access this in a mostly consistent
    manner.

******************************************************************************/

WRITE8_MEMBER(segac2_state::counter_timer_w)
{
	/*int value = data & 1;*/
	switch (data & 0x1e)
	{
		case 0x00:  /* player 1 start/stop */
		case 0x02:  /* player 2 start/stop */
		case 0x04:  /* ??? */
		case 0x06:  /* ??? */
		case 0x08:  /* player 1 game timer? */
		case 0x0a:  /* player 2 game timer? */
		case 0x0c:  /* ??? */
		case 0x0e:  /* ??? */
			break;

		case 0x10:  /* coin counter */
//          coin_counter_w(space.machine(), 0,1);
//          coin_counter_w(space.machine(), 0,0);
			break;

		case 0x12:  /* set coinage info -- followed by two 4-bit values */
			break;

		case 0x14:  /* game timer? (see Tant-R) */
		case 0x16:  /* intro timer? (see Tant-R) */
		case 0x18:  /* ??? */
		case 0x1a:  /* ??? */
		case 0x1c:  /* ??? */
			break;

		case 0x1e:  /* reset */
			break;
	}
}


/******************************************************************************
    Print Club camera handling
*******************************************************************************

    Just some fake stuff to get us to boot.

******************************************************************************/

READ16_MEMBER(segac2_state::printer_r)
{
	return m_cam_data;
}

WRITE16_MEMBER(segac2_state::print_club_camera_w)
{
	m_cam_data = data;
}


/******************************************************************************
    Memory Maps
*******************************************************************************

    The System C/C2 68k Memory map is fairly similar to the Genesis in terms
    of RAM, ROM, VDP access locations, although the differences between the
    arcade system and the Genesis means its not same.

******************************************************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, segac2_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x800000, 0x800001) AM_MIRROR(0x13fdfe) AM_READWRITE8(prot_r, prot_w, 0x00ff)
	AM_RANGE(0x800200, 0x800201) AM_MIRROR(0x13fdfe) AM_WRITE8(control_w, 0x00ff)
	AM_RANGE(0x840000, 0x84001f) AM_MIRROR(0x13fee0) AM_DEVREADWRITE8("io", sega_315_5296_device, read, write, 0x00ff)
	AM_RANGE(0x840100, 0x840107) AM_MIRROR(0x13fef8) AM_DEVREADWRITE8("ymsnd", ym3438_device, read, write, 0x00ff)
	AM_RANGE(0x880100, 0x880101) AM_MIRROR(0x13fefe) AM_WRITE8(counter_timer_w, 0x00ff)
	AM_RANGE(0x8c0000, 0x8c0fff) AM_MIRROR(0x13f000) AM_READWRITE(palette_r, palette_w) AM_SHARE("paletteram")
	AM_RANGE(0xc00000, 0xc0001f) AM_MIRROR(0x18ff00) AM_DEVREADWRITE("gen_vdp", sega315_5313_device, vdp_r, vdp_w)
	AM_RANGE(0xe00000, 0xe0ffff) AM_MIRROR(0x1f0000) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END


/******************************************************************************
    Input Ports
*******************************************************************************

    The input ports on the C2 games always consist of 1 Coin Port, 2 Player
    Input ports and 2 Dipswitch Ports, 1 of those Dipswitch Ports being used
    for coinage, the other for Game Options.

    Most of the Games List the Dipswitchs and Inputs in the Test Menus, adding
    them is just a tedious task.  I think Columnns & Bloxeed are Exceptions
    and will need their Dipswitches working out by observation.  The Coin Part
    of the DSW's seems fairly common to all games.

******************************************************************************/

static INPUT_PORTS_START( systemc_generic )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINAGE")
	SEGA_COINAGE_LOC(SW1)

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( columns )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("DSW")
	//"SW2:1" unused
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//"SW2:3" unused
	//"SW2:4" unused
	/* The first level increase (from 0 to 1) is always after destroying
	   35 jewels. Then, the level gets 1 level more every : */
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easiest ) )  // 50 jewels
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )     // 40 jewels
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )   // 35 jewels
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )     // 25 jewels
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END

static INPUT_PORTS_START( columnsu )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//"SW2:3" unused
	PORT_DIPNAME( 0x04, 0x00, "Background Music" ) PORT_DIPLOCATION("SW2:3") /* listed in the manual, ON by default */
	PORT_DIPSETTING(    0x04, "BGM #1" )
	PORT_DIPSETTING(    0x00, "BGM #2" )
	/* The first level increase (from 0 to 1) is always after destroying
	   35 jewels. Then, the level gets 1 level more every : */
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easiest ) )  // 50 jewels
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )     // 40 jewels
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )   // 35 jewels
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )     // 25 jewels
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END

static INPUT_PORTS_START( columns2 )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "VS. Mode Credits/Match" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x30, 0x30, "Flash Mode Difficulty" ) PORT_DIPLOCATION("SW2:5,6") // rising up height per a skull
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )    // 1
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )  // 2
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )    // 3
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) ) // 4
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END


static INPUT_PORTS_START( borench )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Credits to Start" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Lives 1P Mode" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPNAME( 0x30, 0x30, "Lives 2P Mode" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( tfrceac )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Credits to Start" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30,  DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x10, "10k, 70k, 150k" )
	PORT_DIPSETTING(    0x30, "20k, 100k, 200k" )
	PORT_DIPSETTING(    0x20, "40k, 150k, 300k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( twinsqua )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Credits to Start" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Buy In" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x20, "Seat Type" ) PORT_DIPLOCATION("SW2:6") // Sega cabinet "MEGALO 50" has "MOVING SEAT"
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Moving" )
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END


static INPUT_PORTS_START( ribbit )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 1 Unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 1 Unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Credits to Start" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END


static INPUT_PORTS_START( puyo )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("DSW")
	//"SW2:1" unused
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "VS. Mode Credits/Match" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x18, 0x18, "1P Mode Difficulty" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	//"SW2:6" unused
	//"SW2:7" unused
	PORT_DIPNAME( 0x80, 0x80, "Moving Seat" ) PORT_DIPLOCATION("SW2:8") // Sega cabinet "MEGALO 50" has "MOVING SEAT"
	PORT_DIPSETTING(    0x80, "No Use" )
	PORT_DIPSETTING(    0x00, "In Use" )
INPUT_PORTS_END


static INPUT_PORTS_START( stkclmns )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Match Mode Price" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	//"SW2:5" unused
	//"SW2:6" unused
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END


static INPUT_PORTS_START( potopoto )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Credits to Start" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Chute Type" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x08, 0x08, "Credits to Continue" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x10, 0x10, "Buy-In" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, "Moving Seat" ) PORT_DIPLOCATION("SW2:8") // Sega cabinet "MEGALO 50" has "MOVING SEAT"
	PORT_DIPSETTING(    0x80, "No Use" )
	PORT_DIPSETTING(    0x00, "In Use" )
INPUT_PORTS_END


static INPUT_PORTS_START( zunkyou )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Game Difficulty 1" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x02, "Game Difficulty 2" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//"SW2:6" unused
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END


static INPUT_PORTS_START( ichir )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	//"SW2:4" unused
	//"SW2:5" unused
	//"SW2:6" unused
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END


static INPUT_PORTS_START( bloxeedc )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2,3") // needs to be double checked
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7") // ?
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "High Speed Mode" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( puyopuy2 )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Rannyu Off Button" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "Use" )
	PORT_DIPSETTING(    0x00, "No Use" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Turn Direction" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "1:Right  2:Left" )
	PORT_DIPSETTING(    0x00, "1:Left  2:Right")
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x60, 0x60, "VS Mode Match/1 Play" ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x80, 0x80, "Battle Start credit" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )
INPUT_PORTS_END


static INPUT_PORTS_START( pclub )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Probably Unused */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Ok")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Cancel")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Probably Unused */

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Probably Unused */

	PORT_MODIFY("COINAGE")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW4:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW4:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW4:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW4:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW4:8" )

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW5:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW5:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW5:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW5:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW5:5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds )) PORT_DIPLOCATION("SW5:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW5:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW5:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( pclubjv2 )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Probably Unused */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Ok")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Cancel")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Probably Unused */

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Probably Unused */

	PORT_MODIFY("COINAGE")
	PORT_DIPNAME( 0x07, 0x07, "Coins per Credit (Normal / Alternate)" ) PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x00, "25 / 30" ) //7C1C
	PORT_DIPSETTING(    0x01, "6 / 7" ) //6C1C
	PORT_DIPSETTING(    0x02, "10 / 12" ) //5C1C
	PORT_DIPSETTING(    0x03, "1 / 2" ) //4C1C
	PORT_DIPSETTING(    0x07, "3 / Free Play" ) //3C1C
	PORT_DIPSETTING(    0x04, "15 / 20" ) //2C1C
	PORT_DIPSETTING(    0x05, "4 / 5" ) //1C1C
	PORT_DIPSETTING(    0x06, "8 / 9" ) //FP
	PORT_DIPNAME( 0x08, 0x08, "Alternate Coinage" ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW5:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW5:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW5:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW5:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW5:5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds )) PORT_DIPLOCATION("SW5:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW5:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW5:8" )
INPUT_PORTS_END

/******************************************************************************
    Sound interfaces
******************************************************************************/

WRITE_LINE_MEMBER(segac2_state::segac2_irq2_interrupt)
{
	//printf("sound irq %d\n", state);
	m_maincpu->set_input_line(2, state ? ASSERT_LINE : CLEAR_LINE);
}


/******************************************************************************
    Machine Drivers
*******************************************************************************

    General Overview
        M68000 @ 10MHz (Main Processor)
        YM3438 (Fm Sound)
        SN76489 (PSG, Noise, Part of the VDP)
        UPD7759 (Sample Playback, C-2 Only)

******************************************************************************/

VIDEO_START_MEMBER(segac2_state,segac2_new)
{
	VIDEO_START_CALL_MEMBER(megadriv);
}

// C2 doesn't use the internal VDP CRAM, instead it uses the digital output of the chip
//  and applies it's own external colour circuity
UINT32 segac2_state::screen_update_segac2_new(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *paldata = m_palette->pens();
	if (!m_segac2_enable_display)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	/* Copy our screen buffer here */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32* desty = &bitmap.pix32(y, 0);
		UINT16* srcy;

		srcy = m_vdp->m_render_line_raw;

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			UINT16 src = srcy[x];
			switch (src & 0x1c0)
			{
				case 0x000:
					desty[x] = paldata[(src&0x0f) | m_segac2_bg_pal_lookup[(src & 0x30)>>4] | 0x800];
					break;
				case 0x040:
					desty[x] = paldata[(src&0x0f) | m_segac2_bg_pal_lookup[(src & 0x30)>>4]];
					break;
				case 0x080:
					desty[x] = paldata[(src&0x0f) | m_segac2_sp_pal_lookup[(src & 0x30)>>4]];
					break;
				case 0x0c0:
					// bg pen
					desty[x] = paldata[(src&0x0f) | m_segac2_bg_pal_lookup[(src & 0x30)>>4] | 0x1000];
					break;
				case 0x100:
					// shadow
					desty[x] = paldata[(src&0x0f) | m_segac2_bg_pal_lookup[(src & 0x30)>>4] | 0x800];
					break;
				case 0x140:
					// normal
					desty[x] = paldata[(src&0x0f) | m_segac2_bg_pal_lookup[(src & 0x30)>>4]];
					break;
				case 0x180:
					// sprite
					desty[x] = paldata[(src&0x0f) | m_segac2_sp_pal_lookup[(src & 0x30)>>4]];
					break;
				case 0x1c0:
					// highlight
					desty[x] = paldata[(src&0x0f) | m_segac2_bg_pal_lookup[(src & 0x30)>>4] | 0x1000];
					break;
			}
		}
	}

	return 0;
}




// the main interrupt on C2 comes from the vdp line used to drive the z80 interrupt on a regular genesis(!)
WRITE_LINE_MEMBER(segac2_state::vdp_sndirqline_callback_c2)
{
	if (state == ASSERT_LINE)
		m_maincpu->set_input_line(6, HOLD_LINE);
}

// the line usually used to drive irq6 is not connected
WRITE_LINE_MEMBER(segac2_state::vdp_lv6irqline_callback_c2)
{
	//
}

// the scanline interrupt seems connected as usual
WRITE_LINE_MEMBER(segac2_state::vdp_lv4irqline_callback_c2)
{
	if (state == ASSERT_LINE)
		m_maincpu->set_input_line(4, HOLD_LINE);
	else
		m_maincpu->set_input_line(4, CLEAR_LINE);
}


static MACHINE_CONFIG_START( segac, segac2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XL2_CLOCK/6)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(md_base_state,genesis_int_callback)

	MCFG_MACHINE_START_OVERRIDE(segac2_state,segac2)
	MCFG_MACHINE_RESET_OVERRIDE(segac2_state,segac2)
	MCFG_NVRAM_ADD_1FILL("nvram") // borencha requires 0xff fill or there is no sound (it lacks some of the init code of the borench set)

	MCFG_DEVICE_ADD("io", SEGA_315_5296, XL2_CLOCK/6) // clock divider guessed
	MCFG_315_5296_IN_PORTA_CB(IOPORT("P1"))
	MCFG_315_5296_IN_PORTB_CB(IOPORT("P2"))
	MCFG_315_5296_IN_PORTC_CB(READ8(segac2_state, io_portc_r))
	MCFG_315_5296_OUT_PORTD_CB(WRITE8(segac2_state, io_portd_w))
	MCFG_315_5296_IN_PORTE_CB(IOPORT("SERVICE"))
	MCFG_315_5296_IN_PORTF_CB(IOPORT("COINAGE"))
	MCFG_315_5296_IN_PORTG_CB(IOPORT("DSW"))
	MCFG_315_5296_OUT_PORTH_CB(WRITE8(segac2_state, io_porth_w))

	/* video hardware */
	MCFG_DEVICE_ADD("gen_vdp", SEGA315_5313, 0)
	MCFG_SEGA315_5313_IS_PAL(false)
	MCFG_SEGA315_5313_SND_IRQ_CALLBACK(WRITELINE(segac2_state, vdp_sndirqline_callback_c2));
	MCFG_SEGA315_5313_LV6_IRQ_CALLBACK(WRITELINE(segac2_state, vdp_lv6irqline_callback_c2));
	MCFG_SEGA315_5313_LV4_IRQ_CALLBACK(WRITELINE(segac2_state, vdp_lv4irqline_callback_c2));
	MCFG_SEGA315_5313_ALT_TIMING(1);
	MCFG_VIDEO_SET_SCREEN("megadriv")

	MCFG_TIMER_DEVICE_ADD_SCANLINE("scantimer", "gen_vdp", sega315_5313_device, megadriv_scanline_timer_callback_alt_timing, "megadriv", 0, 1)

	MCFG_SCREEN_ADD("megadriv", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(segac2_state, screen_update_segac2_new)
	MCFG_SCREEN_VBLANK_DRIVER(segac2_state, screen_eof_megadriv )

	MCFG_PALETTE_ADD("palette", 2048*3)

	MCFG_VIDEO_START_OVERRIDE(segac2_state,segac2_new)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3438, XL2_CLOCK/7)
	MCFG_YM2612_IRQ_HANDLER(WRITELINE(segac2_state, segac2_irq2_interrupt))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	/* right channel not connected */

	MCFG_SOUND_ADD("snsnd", SN76496, XL2_CLOCK/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( segac2, segac )

	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("io")
	MCFG_315_5296_OUT_CNT1_CB(DEVWRITELINE("upd", upd7759_device, reset_w))

	/* sound hardware */
	MCFG_SOUND_ADD("upd", UPD7759, XL1_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/******************************************************************************
    Rom Definitions
*******************************************************************************

    Games are in Order of Date (Year) with System-C titles coming first.

******************************************************************************/


/* ----- System C Games ----- */

ROM_START( bloxeedc ) /* Bloxeed (C System Version)  (c)1989 Sega / Elorg */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-12908.ic32", 0x000000, 0x020000, CRC(fc77cb91) SHA1(248a462e3858ffdc171af7d806e57deecb5dae50) )
	ROM_LOAD16_BYTE( "epr-12907.ic31", 0x000001, 0x020000, CRC(e5fcbac6) SHA1(a1adec5ef5574bff96a3d66619a24a6715097bb9) )
	ROM_LOAD16_BYTE( "epr-12993.ic34", 0x040000, 0x020000, CRC(487bc8fc) SHA1(3fb205bf56f35443e993e08b39c1a08c13ca5e3b) )
	ROM_LOAD16_BYTE( "epr-12992.ic33", 0x040001, 0x020000, CRC(19b0084c) SHA1(b3ba0f3d8d39a19aa66edb24885ea21192e22704) )
ROM_END

ROM_START( bloxeedu ) /* Bloxeed USA (C System Version)  (c)1989 Sega / Elorg */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-12997a.ic32", 0x000000, 0x020000, CRC(23655bc9) SHA1(32fc1f75a43aa49dc656d40d34ec10f3f0a2bdb3) )
	ROM_LOAD16_BYTE( "epr-12996a.ic31", 0x000001, 0x020000, CRC(83c83f0c) SHA1(ca8e2ad7cceabd8de7a91b91cb92eafb6dd3171f) )
	ROM_LOAD16_BYTE( "epr-12993.ic34",  0x040000, 0x020000, CRC(487bc8fc) SHA1(3fb205bf56f35443e993e08b39c1a08c13ca5e3b) )
	ROM_LOAD16_BYTE( "epr-12992.ic33",  0x040001, 0x020000, CRC(19b0084c) SHA1(b3ba0f3d8d39a19aa66edb24885ea21192e22704) )

	ROM_REGION( 0x0004, "pals", 0 )
	ROM_LOAD( "315-5393.ic24", 0x0000, 0x0001, NO_DUMP ) /* PALCE16V8H-25PC */
	ROM_LOAD( "315-5394.ic25", 0x0000, 0x0001, NO_DUMP ) /* PALCE16V8H-25PC */
	ROM_LOAD( "315-5395.ic26", 0x0000, 0x0001, NO_DUMP ) /* PALCE16V8H-25PC */
	ROM_LOAD( "317-0140.ic27", 0x0000, 0x0001, NO_DUMP ) /* EPM4032DC (Protection Chip) */
ROM_END


ROM_START( columns ) /* Columns (World) (c)1990 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13114.ic32", 0x000000, 0x020000, CRC(ff78f740) SHA1(0a034103a4b942f43e62f6e717f5dbf1bfb0b613) )
	ROM_LOAD16_BYTE( "epr-13113.ic31", 0x000001, 0x020000, CRC(9a426d9b) SHA1(3322e65ebf8d0a6047f7d408387c63ea401b8973) )
ROM_END

ROM_START( columnsu ) /* Columns (US) (c)1990 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13116a.ic32", 0x000000, 0x020000, CRC(a0284b16) SHA1(a72c8567ab2386ef6bc7bb83cc1612f4c6bf8461) )
	ROM_LOAD16_BYTE( "epr-13115a.ic31", 0x000001, 0x020000, CRC(e37496f3) SHA1(30ebeed76613ae8d6d3ce9fca282124685067b27) )
ROM_END

ROM_START( columnsj ) /* Columns (Jpn) (c)1990 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13112.ic32", 0x000000, 0x020000, CRC(bae6e53e) SHA1(2c2fd621eecd55591f22d076323972a7d0314615) )
	ROM_LOAD16_BYTE( "epr-13111.ic31", 0x000001, 0x020000, CRC(aa5ccd6d) SHA1(480e29e3112282d1790f1fb68075453325ba4336) )
ROM_END


ROM_START( columns2 ) /* Columns II - The Voyage Through Time  (c)1990 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13363.ic32", 0x000000, 0x020000, CRC(c99e4ffd) SHA1(67981aa08c8a625af35dd7689011364159cf9194) )
	ROM_LOAD16_BYTE( "epr-13362.ic31", 0x000001, 0x020000, CRC(394e2419) SHA1(d4f726b32cf301d0d52611237b83177e69bfaf71) )
ROM_END

ROM_START( column2j ) /* Columns II - The Voyage Through Time (Jpn)  (c)1990 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13361.ic32", 0x000000, 0x020000, CRC(b54b5f12) SHA1(4d7fbae7d9bcadd433ebc25aef255dc43df611bc) )
	ROM_LOAD16_BYTE( "epr-13360.ic31", 0x000001, 0x020000, CRC(a59b1d4f) SHA1(e9ee315677782e1c61ae8f11260101cc03176188) )
ROM_END


ROM_START( tantrbl2 ) /* Tant-R (Puzzle & Action) (Alt Bootleg Running on C Board?, No Samples) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "trb2_2.32",       0x000000, 0x080000, CRC(8fc99c48) SHA1(d90ed673fe1f6e1f878c0d8fc62f5439b56d0a47) )
	ROM_LOAD16_BYTE( "trb2_1.31",       0x000001, 0x080000, CRC(c318d00d) SHA1(703760d4ddc45bc0921ae96a27d9a8fbf12a1e96) )
	ROM_LOAD16_BYTE( "mpr-15616.ic34",  0x100000, 0x080000, CRC(17b80202) SHA1(f47bf2aa0c5972647438619b8453c7dede5c422f) )
	ROM_LOAD16_BYTE( "mpr-15615.ic33",  0x100001, 0x080000, CRC(36a88bd4) SHA1(cc7f6a947d1b79bb86957c43035b53d6d2bcfa28) )
ROM_END

ROM_START( tantrbl3 ) /* Tant-R (Puzzle & Action) (Alt Bootleg Running on C Board?, No Samples) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.u29",  0x000000, 0x080000, CRC(faecb7b1) SHA1(ddca61996dbd0d66e9ecc04647e61a6d0af25807) )
	ROM_LOAD16_BYTE( "1.u28",  0x000001, 0x080000, CRC(3304556d) SHA1(586265c846bf6404dcb33404e961bf0733e24bf5) )
	ROM_LOAD16_BYTE( "4.u31",  0x100000, 0x080000, CRC(17b80202) SHA1(f47bf2aa0c5972647438619b8453c7dede5c422f) )
	ROM_LOAD16_BYTE( "3.u30",  0x100001, 0x080000, CRC(36a88bd4) SHA1(cc7f6a947d1b79bb86957c43035b53d6d2bcfa28) )
ROM_END

ROM_START( ichirjbl ) /* Ichident-R (Puzzle & Action 2) (Bootleg Running on C Board?, No Samples) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "27c4000.2",0x000000, 0x080000, CRC(5a194f44) SHA1(67a4d21b91704f8c2210b5106e82e22ba3366f4c) )
	ROM_LOAD16_BYTE( "27c4000.1",0x000001, 0x080000, CRC(de209f84) SHA1(0860d0ebfab2952e82fc1e292bf9410d673d9322) )
	ROM_LOAD16_BYTE( "epr-16888", 0x100000, 0x080000, CRC(85d73722) SHA1(7ebe81b4d6c89f87f60200a3a8cddb07d581adef) )
	ROM_LOAD16_BYTE( "epr-16887", 0x100001, 0x080000, CRC(bc3bbf25) SHA1(e760ad400bc183b38e9787d88c8ac084fbe2ae21) )
ROM_END


/* ----- System C-2 Games ----- */

ROM_START( borench ) /* Borench  (c)1990 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ic32.bin", 0x000000, 0x040000, CRC(2c54457d) SHA1(adf3ea5393d2633ec6215e64f0cd89ad4567e765) )
	ROM_LOAD16_BYTE( "ic31.bin", 0x000001, 0x040000, CRC(b46445fc) SHA1(24e85ef5abbc5376a854b13ed90f08f0c30d7f25) )

	ROM_REGION( 0x020000, "upd", 0 )
	ROM_LOAD( "13587.ic4", 0x000000, 0x020000, CRC(62b85e56) SHA1(822ab733c87938bb70a9e32cc5dd36bbf6f21d11) )
ROM_END

ROM_START( borencha ) /* Borench  (c)1990 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "13591.ic32", 0x000000, 0x040000, CRC(7851078b) SHA1(122934f0414a29b4b363acad01ee4db369259e72) )
	ROM_LOAD16_BYTE( "13590.ic31", 0x000001, 0x040000, CRC(01bc6fe6) SHA1(b241b09852f52f712e3ddc6660ec3eb436b1302c) )

	ROM_REGION( 0x020000, "upd", 0 )
	ROM_LOAD( "13587.ic4", 0x000000, 0x020000, CRC(62b85e56) SHA1(822ab733c87938bb70a9e32cc5dd36bbf6f21d11) )
ROM_END


ROM_START( tfrceac ) /* Thunder Force AC  (c)1990 Technosoft / Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13675.ic32", 0x000000, 0x040000, CRC(95ecf202) SHA1(92b0f351f2bee7d59873a4991615f14f1afe4da7) )
	ROM_LOAD16_BYTE( "epr-13674.ic31", 0x000001, 0x040000, CRC(e63d7f1a) SHA1(a40d0a5a96f379a467048dc8fddd8aaaeb94da1d) )
	/* 0x080000 - 0x100000 Empty */
	ROM_LOAD16_BYTE( "epr-13659.ic34", 0x100000, 0x040000, CRC(29f23461) SHA1(032a7125fef5a660b85654d595aafc46812cdde6) )
	ROM_LOAD16_BYTE( "epr-13658.ic33", 0x100001, 0x040000, CRC(9e23734f) SHA1(64d27dc53f0ffc3513345a26ed077751b25d15f1) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-13655.ic4", 0x000000, 0x040000, CRC(e09961f6) SHA1(e109b5f41502b765d191f22e3bbcff97d6defaa1) )
ROM_END

ROM_START( tfrceacj ) /* Thunder Force AC (Jpn)  (c)1990 Technosoft / Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13657.ic32", 0x000000, 0x040000, CRC(a0f38ffd) SHA1(da548e7f61aed0e82a460553a119941da8857bc4) )
	ROM_LOAD16_BYTE( "epr-13656.ic31", 0x000001, 0x040000, CRC(b9438d1e) SHA1(598209c9fec3527fde720af09e5bebd7379f5b2b) )
	/* 0x080000 - 0x100000 Empty */
	ROM_LOAD16_BYTE( "epr-13659.ic34", 0x100000, 0x040000, CRC(29f23461) SHA1(032a7125fef5a660b85654d595aafc46812cdde6) )
	ROM_LOAD16_BYTE( "epr-13658.ic33", 0x100001, 0x040000, CRC(9e23734f) SHA1(64d27dc53f0ffc3513345a26ed077751b25d15f1) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-13655.ic4", 0x000000, 0x040000, CRC(e09961f6) SHA1(e109b5f41502b765d191f22e3bbcff97d6defaa1) )
ROM_END

ROM_START( tfrceacb ) /* Thunder Force AC (Bootleg)  (c)1990 Technosoft / Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "4.bin",    0x000000, 0x040000, CRC(eba059d3) SHA1(7bc04401f9a138fa151ac09a528b70acfb2021e3) )
	ROM_LOAD16_BYTE( "3.bin",    0x000001, 0x040000, CRC(3e5dc542) SHA1(4a66dc842afaa145dab82b232738eea107bdf0f8) )
	/* 0x080000 - 0x100000 Empty */
	ROM_LOAD16_BYTE( "epr-13659.ic34", 0x100000, 0x040000, CRC(29f23461) SHA1(032a7125fef5a660b85654d595aafc46812cdde6) )
	ROM_LOAD16_BYTE( "epr-13658.ic33", 0x100001, 0x040000, CRC(9e23734f) SHA1(64d27dc53f0ffc3513345a26ed077751b25d15f1) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-13655.ic4", 0x000000, 0x040000, CRC(e09961f6) SHA1(e109b5f41502b765d191f22e3bbcff97d6defaa1) )
ROM_END


ROM_START( twinsqua ) /* Twin Squash  (c)1991 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-14657.ic32", 0x000000, 0x040000, CRC(becbb1a1) SHA1(787b1a4bf420186d05b5448582f6492e40d394fa) )
	ROM_LOAD16_BYTE( "epr-14656.ic31", 0x000001, 0x040000, CRC(411906e7) SHA1(68a4e66b9e18499d77cdb584470f35f67edec6fd) )

	ROM_REGION( 0x020000, "upd", 0 )
	ROM_LOAD( "epr-14588.ic4", 0x000000, 0x020000, CRC(5a9b6881) SHA1(d86ec7f569fae5a1ce93a1cf40998cbb13726e0c) )
ROM_END


ROM_START( ribbit ) /* Ribbit  (c)1991 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13833.ic32", 0x000000, 0x040000, CRC(5347f8ce) SHA1(b95b99536157edfbf0d74a42f64235f47dca7ee1) )
	ROM_LOAD16_BYTE( "epr-13832.ic31", 0x000001, 0x040000, CRC(889c42c2) SHA1(0839a50a68b64a66d995f1bfaff42fcb60bb4d45) )
	ROM_COPY( "maincpu", 0x000000, 0x080000, 0x080000 )
	ROM_LOAD16_BYTE( "epr-13838.ic34", 0x100000, 0x080000, CRC(a5d62ac3) SHA1(8d83a7bc4017e125ef4231278f766b2368d5fc1f) )
	ROM_LOAD16_BYTE( "epr-13837.ic33", 0x100001, 0x080000, CRC(434de159) SHA1(cf2973131cabf2bc0ebb2bfe9f804ad3d7d0a733) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "epr-13834.ic4", 0x000000, 0x020000, CRC(ab0c1833) SHA1(f864e12ecf6c0524da20fc66747a4fa4280e67e9) )
ROM_END


ROM_START( tantr ) /* Tant-R (Puzzle & Action)  (c)1992 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-15614.ic32", 0x000000, 0x080000, CRC(557782bc) SHA1(1546a999ab97c380dc87f6c95d5687722206740d) )
	ROM_LOAD16_BYTE( "epr-15613.ic31", 0x000001, 0x080000, CRC(14bbb235) SHA1(8dbfec5fb1d7a695acbb2fc0e78e4bdf76eb8d9d) )
	ROM_LOAD16_BYTE( "mpr-15616.ic34", 0x100000, 0x080000, CRC(17b80202) SHA1(f47bf2aa0c5972647438619b8453c7dede5c422f) )
	ROM_LOAD16_BYTE( "mpr-15615.ic33", 0x100001, 0x080000, CRC(36a88bd4) SHA1(cc7f6a947d1b79bb86957c43035b53d6d2bcfa28) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-15617.ic4", 0x000000, 0x040000, CRC(338324a1) SHA1(79e2782d0d4764dd723886f846c852a6f6c1fb64) )
ROM_END

ROM_START( tantrkor ) /* Tant-R (Puzzle & Action) (Korea) (c)1993 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	/* strange names, but this is what was printed on the (original) chips */
	ROM_LOAD16_BYTE( "mpr-15592b.ic32", 0x000000, 0x080000, CRC(7efe26b3) SHA1(958420b9b400eafe392745af90bff729463427c7) )
	ROM_LOAD16_BYTE( "mpr-15592b.ic31", 0x000001, 0x080000, CRC(af5a860f) SHA1(cb0011f420721d035e9f0e43bb72cf286982fd32) )
	ROM_LOAD16_BYTE( "mpr-15992b.ic34", 0x100000, 0x080000, CRC(6282a5d4) SHA1(9220e119e79d969d7d70e8a25c75dd3d9bc340ae) )
	ROM_LOAD16_BYTE( "mpr-15592b.ic33", 0x100001, 0x080000, CRC(82d78413) SHA1(9ff9c2b1632e280444965110bab90c0fc98cd6da) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-15617.ic4", 0x000000, 0x040000, CRC(338324a1) SHA1(79e2782d0d4764dd723886f846c852a6f6c1fb64) )
ROM_END

ROM_START( tantrbl ) /* Tant-R (Puzzle & Action) (Bootleg)  (c)1992 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pa_e10.bin",  0x000000, 0x080000, CRC(6c3f711f) SHA1(55aa2d50422134b95d9a7c5cbdc453b207b91b4c) )
	ROM_LOAD16_BYTE( "pa_f10.bin",  0x000001, 0x080000, CRC(75526786) SHA1(8f5aa7f6918b71a79e6fca18194beec2aef15844) )
	ROM_LOAD16_BYTE( "mpr-15616.ic34", 0x100000, 0x080000, CRC(17b80202) SHA1(f47bf2aa0c5972647438619b8453c7dede5c422f) )
	ROM_LOAD16_BYTE( "mpr-15615.ic33", 0x100001, 0x080000, CRC(36a88bd4) SHA1(cc7f6a947d1b79bb86957c43035b53d6d2bcfa28) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "pa_e03.bin", 0x000000, 0x020000, CRC(72918c58) SHA1(cb42363b163727a887a0b762519c72dcdf0a6460) )
	ROM_LOAD( "pa_e02.bin", 0x020000, 0x020000, CRC(4e85b2a3) SHA1(3f92fb931d315c5a2d6c54b3204718574928cb7b) )
ROM_END


ROM_START( puyo ) /* Puyo Puyo  (c)1992 Sega / Compile */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-15198.ic32", 0x000000, 0x020000, CRC(9610d80c) SHA1(1ffad09d3369c1942d4db611c41bae47d08c7564) )
	ROM_LOAD16_BYTE( "epr-15197.ic31", 0x000001, 0x020000, CRC(7b1f3229) SHA1(13d0905291e748973d7d17eb404a286ffb94de03) )
	/* 0x040000 - 0x100000 Empty */
	ROM_LOAD16_BYTE( "epr-15200.ic34", 0x100000, 0x020000, CRC(0a0692e5) SHA1(d4ecc5b1791a91e3b33a5d4d0dd305f1623483d9) )
	ROM_LOAD16_BYTE( "epr-15199.ic33", 0x100001, 0x020000, CRC(353109b8) SHA1(92440987add3124b758e7eaa77a3a6f54ca61bb8) )

	ROM_REGION( 0x020000, "upd", 0 )
	ROM_LOAD( "epr-15196.ic4", 0x000000, 0x020000, CRC(79112b3b) SHA1(fc3a202e1e2ff39950d4af689b7fcca86c301805) )
ROM_END

ROM_START( puyoj ) /* Puyo Puyo (Rev B)  (c)1992 Sega / Compile */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-15036b.ic32", 0x000000, 0x020000, CRC(5310ca1b) SHA1(dcfe2bf7476b640dfb790e8716e75b483d535e48) )
	ROM_LOAD16_BYTE( "epr-15035b.ic31", 0x000001, 0x020000, CRC(bc62e400) SHA1(12bb6031574838a28889f6edb31dbb689265287c) )
	/* 0x040000 - 0x100000 Empty */
	ROM_LOAD16_BYTE( "epr-15038.ic34", 0x100000, 0x020000, CRC(3b9eea0c) SHA1(e3e6148c1769834cc0061932eb035daa79673144) )
	ROM_LOAD16_BYTE( "epr-15037.ic33", 0x100001, 0x020000, CRC(be2f7974) SHA1(77027ced7a62f94e9fc6e8a0a4ac0c62f7ea813b) )

	ROM_REGION( 0x020000, "upd", 0 )
	ROM_LOAD( "epr-15034.ic4", 0x000000, 0x020000, CRC(5688213b) SHA1(f3f234e482871ca903a782e51008f3bfed04ee63) )

	ROM_REGION( 0x0004, "pals", 0 )
	ROM_LOAD( "315-5452.ic24", 0x0000, 0x0001, NO_DUMP ) /* PALCE16V8H-25PC */
	ROM_LOAD( "315-5394.ic25", 0x0000, 0x0001, NO_DUMP ) /* PALCE16V8H-25PC */
	ROM_LOAD( "315-5395.ic26", 0x0000, 0x0001, NO_DUMP ) /* PALCE16V8H-25PC */
	ROM_LOAD( "317-0203.ic27", 0x0000, 0x0001, NO_DUMP ) /* EPM4032DC-25 (Protection Chip) */
ROM_END

ROM_START( puyoja ) /* Puyo Puyo (Rev A)  (c)1992 Sega / Compile */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-15036a.ic32", 0x000000, 0x020000, CRC(61b35257) SHA1(e09a7e992999befc88fc7928a478d1e2d14d7b08) )
	ROM_LOAD16_BYTE( "epr-15035a.ic31", 0x000001, 0x020000, CRC(dfebb6d9) SHA1(6f685729ef4660c2eba409c5236c6d2f313eef5b) )
	/* 0x040000 - 0x100000 Empty */
	ROM_LOAD16_BYTE( "epr-15038.ic34",  0x100000, 0x020000, CRC(3b9eea0c) SHA1(e3e6148c1769834cc0061932eb035daa79673144) )
	ROM_LOAD16_BYTE( "epr-15037.ic33",  0x100001, 0x020000, CRC(be2f7974) SHA1(77027ced7a62f94e9fc6e8a0a4ac0c62f7ea813b) )

	ROM_REGION( 0x020000, "upd", 0 )
	ROM_LOAD( "epr-15034.ic4", 0x000000, 0x020000, CRC(5688213b) SHA1(f3f234e482871ca903a782e51008f3bfed04ee63) )

	ROM_REGION( 0x0004, "pals", 0 )
	ROM_LOAD( "315-5452.ic24", 0x0000, 0x0001, NO_DUMP ) /* PALCE16V8H-25PC */
	ROM_LOAD( "315-5394.ic25", 0x0000, 0x0001, NO_DUMP ) /* PALCE16V8H-25PC */
	ROM_LOAD( "315-5395.ic26", 0x0000, 0x0001, NO_DUMP ) /* PALCE16V8H-25PC */
	ROM_LOAD( "317-0203.ic27", 0x0000, 0x0001, NO_DUMP ) /* EPM4032DC-25 (Protection Chip) */
ROM_END

ROM_START( puyobl ) /* Puyo Puyo  (c)1992 Sega / Compile  Bootleg */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "puyopuyb.4bo", 0x000000, 0x020000, CRC(89ea4d33) SHA1(bef9d011524e71c072d309f6da3c2ebc38878e0e) )
	ROM_LOAD16_BYTE( "puyopuyb.3bo", 0x000001, 0x020000, CRC(c002e545) SHA1(7a59ac764d60e9955830d9617b0bd122b44e7b2f) )
	/* 0x040000 - 0x100000 Empty */
	ROM_LOAD16_BYTE( "puyopuyb.6bo", 0x100000, 0x020000, CRC(0a0692e5) SHA1(d4ecc5b1791a91e3b33a5d4d0dd305f1623483d9) ) // same as epr-15200.34 from the world set
	ROM_LOAD16_BYTE( "puyopuyb.5bo", 0x100001, 0x020000, CRC(353109b8) SHA1(92440987add3124b758e7eaa77a3a6f54ca61bb8) ) // same as epr-15199.33 from the world set

	ROM_REGION( 0x020000, "upd", 0 )
	ROM_LOAD( "puyopuyb.abo", 0x000000, 0x020000, CRC(79112b3b) SHA1(fc3a202e1e2ff39950d4af689b7fcca86c301805) ) // same as epr-15196.4 from the world set
ROM_END

ROM_START( ichir ) /* Ichident-R (Puzzle & Action 2)  (c)1994 Sega (World) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pa2_32.bin",     0x000000, 0x080000, CRC(7ba0c025) SHA1(855e9bb2a20c6f51b26381233c57c26aa96ad1f6) )
	ROM_LOAD16_BYTE( "pa2_31.bin",     0x000001, 0x080000, CRC(5f86e5cc) SHA1(44e201de00dfbf7c66d0e0d40d17b162c6f0625b) )
	ROM_LOAD16_BYTE( "epr-16888.ic34", 0x100000, 0x080000, CRC(85d73722) SHA1(7ebe81b4d6c89f87f60200a3a8cddb07d581adef) )
	ROM_LOAD16_BYTE( "epr-16887.ic33", 0x100001, 0x080000, CRC(bc3bbf25) SHA1(e760ad400bc183b38e9787d88c8ac084fbe2ae21) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "pa2_02.bin", 0x000000, 0x080000, CRC(fc7b0da5) SHA1(46770aa7e19b4f8a183be3f433c48ad677b552b1) )
ROM_END

ROM_START( ichirk ) /* Ichident-R (Puzzle & Action 2)  (c)1994 Sega (Korea) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	/* Again the part numbers are quite strange for the Korean verison */
	ROM_LOAD16_BYTE( "epr_ichi.32", 0x000000, 0x080000, CRC(804dea11) SHA1(40bf8cbd40969a5880df10914252b7f64d5ce8e9) )
	ROM_LOAD16_BYTE( "epr_ichi.31", 0x000001, 0x080000, CRC(92452353) SHA1(d2e1da5b139965611cd8d707d23396b5d4c07d12) )
	ROM_LOAD16_BYTE( "epr-16888.ic34", 0x100000, 0x080000, CRC(85d73722) SHA1(7ebe81b4d6c89f87f60200a3a8cddb07d581adef) )  // m17235a.34
	ROM_LOAD16_BYTE( "epr-16887.ic33", 0x100001, 0x080000, CRC(bc3bbf25) SHA1(e760ad400bc183b38e9787d88c8ac084fbe2ae21) )  // m17220a.33

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "pa2_02.bin", 0x000000, 0x080000, CRC(fc7b0da5) SHA1(46770aa7e19b4f8a183be3f433c48ad677b552b1) ) // m17220a.4
ROM_END

ROM_START( ichirj ) /* Ichident-R (Puzzle & Action 2)  (c)1994 Sega (Japan) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-16886.ic32", 0x000000, 0x080000, CRC(38208e28) SHA1(07fc634bdf2d3e25274c9c374b3506dec765114c) )
	ROM_LOAD16_BYTE( "epr-16885.ic31", 0x000001, 0x080000, CRC(1ce4e837) SHA1(16600600e12e3f35e3da89524f7f51f019b5ad17) )
	ROM_LOAD16_BYTE( "epr-16888.ic34", 0x100000, 0x080000, CRC(85d73722) SHA1(7ebe81b4d6c89f87f60200a3a8cddb07d581adef) )
	ROM_LOAD16_BYTE( "epr-16887.ic33", 0x100001, 0x080000, CRC(bc3bbf25) SHA1(e760ad400bc183b38e9787d88c8ac084fbe2ae21) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "epr-16884.ic4", 0x000000, 0x080000, CRC(fd9dcdd6) SHA1(b8053a2e68072e7664ffc3c53f983f3ba72a892b) )
ROM_END

ROM_START( stkclmns ) /* Stack Columns  (c)1994 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-16874.ic32", 0x000000, 0x080000, CRC(d78a871c) SHA1(7efcd5d07b089442be5170a3cf9e09579527252f) )
	ROM_LOAD16_BYTE( "epr-16873.ic31", 0x000001, 0x080000, CRC(1def1da4) SHA1(da534a971b40277b2d58ef22c07ca468250d23ca) )
	ROM_LOAD16_BYTE( "mpr-16797.ic34", 0x100000, 0x080000, CRC(b28e9bd5) SHA1(227eb591d10c9dbc52b35954ebd322e2a4451df2) )
	ROM_LOAD16_BYTE( "mpr-16796.ic33", 0x100001, 0x080000, CRC(ec7de52d) SHA1(85bc48cef15e615ad9059500808d17916c854a87) )

	ROM_REGION( 0x020000, "upd", 0 )
	ROM_LOAD( "epr-16793.ic4", 0x000000, 0x020000, CRC(ebb2d057) SHA1(4a19ee5d71e4aabe7d9b9b968ab5ee4bc6262aad) )
ROM_END

ROM_START( stkclmnsj ) /* Stack Columns  (c)1994 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-16795.ic32", 0x000000, 0x080000, CRC(b478fd02) SHA1(aaf9d9f9f4dc900b4e8ff6f258f26e782e5c3166) )
	ROM_LOAD16_BYTE( "epr-16794.ic31", 0x000001, 0x080000, CRC(6d0e8c56) SHA1(8f98d9fd98a1faa70b173cfd72f15102d11e79ae) )
	ROM_LOAD16_BYTE( "mpr-16797.ic34", 0x100000, 0x080000, CRC(b28e9bd5) SHA1(227eb591d10c9dbc52b35954ebd322e2a4451df2) )
	ROM_LOAD16_BYTE( "mpr-16796.ic33", 0x100001, 0x080000, CRC(ec7de52d) SHA1(85bc48cef15e615ad9059500808d17916c854a87) )

	ROM_REGION( 0x020000, "upd", 0 )
	ROM_LOAD( "epr-16793.ic4", 0x000000, 0x020000, CRC(ebb2d057) SHA1(4a19ee5d71e4aabe7d9b9b968ab5ee4bc6262aad) )
ROM_END


ROM_START( puyopuy2 ) /* Puyo Puyo 2  (c)1994 Compile */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-17241.ic32", 0x000000, 0x080000, CRC(1cad1149) SHA1(77fb0482fa35e615c0bed65f4d7f4dd89b241f23) )
	ROM_LOAD16_BYTE( "epr-17240.ic31", 0x000001, 0x080000, CRC(beecf96d) SHA1(c2bdad4b6184c11f81f2a5db409cb4ea186205a7) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "epr-17239.ic4", 0x000000, 0x080000, CRC(020ff6ef) SHA1(6095b8277b47a6fd7a9721f15a70ae5bf6be9b1a) )
ROM_END


ROM_START( potopoto ) /* Poto Poto  (c)1994 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-16662.ic32", 0x000000, 0x040000, CRC(bbd305d6) SHA1(1a4f4869fefac188c69bc67df0b625e43a0c3f1f) )
	ROM_LOAD16_BYTE( "epr-16661.ic31", 0x000001, 0x040000, CRC(5a7d14f4) SHA1(a615b5f481256366db7b1c6302a8dcb69708102b) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-16660.ic4", 0x000000, 0x040000, CRC(8251c61c) SHA1(03eef3aa0bdde2c1d93128648f54fd69278d85dd) )
ROM_END


ROM_START( zunkyou ) /* Zunzunkyou No Yabou  (c)1994 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-16812.ic32", 0x000000, 0x080000, CRC(eb088fb0) SHA1(69089a3516ad50f35e81971ef3c33eb3f5d52374) )
	ROM_LOAD16_BYTE( "epr-16811.ic31", 0x000001, 0x080000, CRC(9ac7035b) SHA1(1803ffbadc1213e04646d483e27da1591e22cd06) )
	ROM_LOAD16_BYTE( "epr-16814.ic34", 0x100000, 0x080000, CRC(821b3b77) SHA1(c45c7393a792ce8306a52f83f8ed8f6b0d7c11e9) )
	ROM_LOAD16_BYTE( "epr-16813.ic33", 0x100001, 0x080000, CRC(3cba9e8f) SHA1(208819bc1a205eaab089542afc7a59f69ce5bb81) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "epr-16810.ic4", 0x000000, 0x080000, CRC(d542f0fe) SHA1(23ea50110dfe1cd9f286a535d15e0c3bcba73b00) )
ROM_END


ROM_START( pclubj ) /* Print Club (c)1995 Atlus */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr18171.32", 0x000000, 0x080000, CRC(6c8eb8e2) SHA1(bbd885a83269524215c1d8470544086e3e82c05c) )
	ROM_LOAD16_BYTE( "epr18170.31", 0x000001, 0x080000, CRC(72c631e6) SHA1(77c4ed793db6cb75346998f38a637db64fd258bd) )
	ROM_LOAD16_BYTE( "epr18173.34", 0x100000, 0x080000, CRC(9809dc72) SHA1(6dbe6b7d4e525aa9b6174f8dc5aee12a5e00a009) )
	ROM_LOAD16_BYTE( "epr18172.33", 0x100001, 0x080000, CRC(c61d819b) SHA1(4813ed3161e16099f482e0cf8df3cbe6c01c619c) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "epr18169.4", 0x000000, 0x080000, CRC(5c00ccfb) SHA1(d043ffa6528bb9b76774c96df4edf8222a1878a4) )
ROM_END


ROM_START( pclubjv2 ) /* Print Club vol.2 (c)1995 Atlus */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p2jwn.u32", 0x000000, 0x080000, CRC(dfc0f7f1) SHA1(d2399f3ff05006590903f943cd77a9c709b9b5b1) )
	ROM_LOAD16_BYTE( "p2jwn.u31", 0x000001, 0x080000, CRC(6ab4c694) SHA1(d8cfaa1a49e86842079c6e3800a95c5afaf76ab6) )
	ROM_LOAD16_BYTE( "p2jwn.u34", 0x100000, 0x080000, CRC(854fd456) SHA1(eff7413a7acd8ee37cb73bc8dfd4f4ae53c04836) )
	ROM_LOAD16_BYTE( "p2jwn.u33", 0x100001, 0x080000, CRC(64428a69) SHA1(e2c5ead4b35db76fda1db03adcd020bde5ca1dd2) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "epr18169.4", 0x000000, 0x080000, CRC(5c00ccfb) SHA1(d043ffa6528bb9b76774c96df4edf8222a1878a4) )
ROM_END


ROM_START( pclubjv4 ) /* Print Club vol.4 (c)1996 Atlus */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p4jsm.u32", 0x000000, 0x080000, CRC(36ff5f80) SHA1(33872aa00c8ca3f54dd7503a44562fbdad92df7d) )
	ROM_LOAD16_BYTE( "p4jsm.u31", 0x000001, 0x080000, CRC(f3c021ad) SHA1(34792d861265b609d5022955eb7d2f471c63dfb8) )
	ROM_LOAD16_BYTE( "p4jsm.u34", 0x100000, 0x080000, CRC(d0fd4b33) SHA1(c272404f09bdb6596740ab150eb158cc22cc9aa6) )
	ROM_LOAD16_BYTE( "p4jsm.u33", 0x100001, 0x080000, CRC(ec667875) SHA1(d235a1d8dfa90e1c638e1f079ce528f61450e1f0) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "epr-18169.ic4", 0x000000, 0x080000, CRC(5c00ccfb) SHA1(d043ffa6528bb9b76774c96df4edf8222a1878a4) )
ROM_END


ROM_START( pclubjv5 ) /* Print Club vol.5 (c)1996 Atlus */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p5jat.u32", 0x000000, 0x080000, CRC(72220e69) SHA1(615de759d73469841987fb028eaf5d5598c32553) )
	ROM_LOAD16_BYTE( "p5jat.u31", 0x000001, 0x080000, CRC(06d83fde) SHA1(dc68375ccb16cde7900eb05f702bc15e7e702ea5) )
	ROM_LOAD16_BYTE( "p5jat.u34", 0x100000, 0x080000, CRC(b172ab58) SHA1(47a70bd678f6c4dafe70b83bd3db678cf44de48b) )
	ROM_LOAD16_BYTE( "p5jat.u33", 0x100001, 0x080000, CRC(ba38ec50) SHA1(666fdba56d8a4dab041015c5e8102305b491d293) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "epr-18169.ic4", 0x000000, 0x080000, CRC(5c00ccfb) SHA1(d043ffa6528bb9b76774c96df4edf8222a1878a4) )
ROM_END



/******************************************************************************
    Machine Init Functions
*******************************************************************************

All of the Sega C/C2 games apart from Bloxeed used a protection chip.
The games contain various checks which make sure this protection chip is
present and returning the expected values. It appears that different
tables are used for Japanese vs. English variants of some games
(Puzzle & Action 2) but not others (Columns).


The basic operation in the EPM5032 macrocell (that is, without using the expander
products) is:

out = x1 XOR (x2 OR x3 OR x4)

where x1, x2, x3, and x4 are the AND of any number of input bits. Each input can
be inverted. The inputs come either from the external pins or from feedback from
the output pins.

Expander products (64 in total) are the NOT of the AND of any number of input bits
(optionally inverted). It's not clear whether the I/O feedback is available to
the expander or not. Looking at bit 1 of prot_func_tfrceac(), it would seem that
it should be, otherwise I don't see how the formula could be computed.

******************************************************************************/

void segac2_state::segac2_common_init(segac2_prot_delegate prot_func)
{
	DRIVER_INIT_CALL(megadriv_c2);
	m_prot_func = prot_func;

	if (m_upd7759 != NULL)
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x880000, 0x880001, 0, 0x13fefe, write16_delegate(FUNC(segac2_state::segac2_upd7759_w),this));
}

int segac2_state::prot_func_dummy(int in)
{
	return 0x0;
}

/* 317-0149 */
int segac2_state::prot_func_columns(int in)
{
	int const b0 = BIT( in,2) ^ ((BIT(~in,0) && BIT( in,7)) || (BIT( in,4) && BIT( in,6)));
	int const b1 = BIT(~in,0) ^ (BIT( in,2) || (BIT( in,5) && BIT(~in,6) && BIT( in,7)));
	int const b2 = BIT( in,3) ^ ((BIT( in,0) && BIT( in,1)) || (BIT( in,4) && BIT( in,6)));
	int const b3 = BIT( in,1) ^ ((BIT( in,0) && BIT( in,1)) || (BIT( in,4) && BIT( in,5)) || (BIT(~in,6) && BIT( in,7)));   // 1 repeated

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0160 */
int segac2_state::prot_func_columns2(int in)
{
	int const b0 =  BIT( in,2) ^ (BIT( in,1) || (BIT( in,4) && BIT( in,5)));
	int const b1 = (BIT( in,0) && BIT( in,3) && BIT( in,4)) ^ (BIT( in,6) || (BIT( in,5) && BIT( in,7)));
	int const b2 = (BIT( in,3) && BIT(~in,2) && BIT( in,4)) ^ (BIT( in,5) || (BIT( in,0) && BIT( in,1)) || (BIT( in,4) && BIT( in,6))); // 4 repeated
	int const b3 = (BIT( in,1) && BIT( in,0) && BIT( in,2)) ^ ((BIT( in,4) && BIT(~in,6)) || (BIT( in,6) && BIT( in,7)));   // 6 repeated

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0172 */
int segac2_state::prot_func_tfrceac(int in)
{
	int const b0 = BIT(~in,2) ^ ((BIT( in,0) && BIT(~in,7)) || (BIT( in,3) && BIT( in,4)));
	int const b1 = (BIT( in,4) && BIT(~in,5) && BIT( in,7)) ^ ((BIT(~in,0) || BIT(~in,3)) && (BIT(~in,6) || BIT(~in,7)));   // not in the form x1 XOR (x2 OR x3 OR x4)
	int const b2 = BIT( in,2) ^ ((BIT( in,4) && BIT(~in,5) && BIT( in,7)) || (BIT(~in,1) && BIT( in,6)));
	int const b3 = BIT( in,0) ^ ((BIT( in,1) && BIT( in,4) && BIT( in,6)) || (BIT( in,1) && BIT( in,4) && BIT( in,7))); // 1,4 repeated

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0173 */
int segac2_state::prot_func_borench(int in)
{
	int const b0 = (BIT( in,1) && BIT( in,2) && BIT( in,3) && BIT( in,7))   ^ (BIT( in,5) || (BIT(~in,0) && BIT(~in,4)));
	int const b1 = (BIT(~in,2) && BIT( in,3) && BIT( in,5))                 ^ (BIT( in,1) || (BIT( in,0) && BIT(~in,4)));
	int const b2 = (BIT( in,1) && BIT(~in,4) && BIT(~in,6))                 ^ (BIT( in,2) || BIT( in,3) || (BIT( in,5) && BIT( in,7)));
	int const b3 = (BIT(~in,0) && BIT( in,5) && (BIT( in,6) || BIT( in,7))) ^ (BIT( in,1) || (BIT( in,3) && BIT( in,4)));   // not in the form x1 XOR (x2 OR x3 OR x4)

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0178 */
int segac2_state::prot_func_ribbit(int in)
{
	int const b0 = (BIT( in,0) && BIT( in,4)) ^ ((BIT( in,1) && BIT( in,2)) || BIT( in,3) || BIT(~in,5));
	int const b1 = (BIT( in,1) && BIT( in,5)) ^ ((BIT( in,2) && BIT( in,3)) || BIT( in,0) || BIT(~in,6));
	int const b2 = (BIT( in,2) && BIT( in,7)) ^ ((BIT( in,3) && BIT( in,0)) || BIT(~in,1) || BIT( in,7));
	int const b3 = (BIT( in,3) && BIT( in,6)) ^ ((BIT( in,0) && BIT( in,1)) || BIT(~in,2) || BIT( in,4));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0193 */
int segac2_state::prot_func_twinsqua(int in)
{
	int const b0 = (BIT( in,2) && BIT(~in,5)) ^ (BIT( in,3) || BIT(~in,4));
	int const b1 = (BIT( in,0) && BIT(~in,2) && BIT( in,4)) ^ (BIT(~in,0) || BIT(~in,4) || BIT(~in,6)); // 0,4 repeated
	int const b2 = (BIT( in,3) && BIT(~in,5)) ^ (BIT( in,4) && BIT( in,7));
	int const b3 =  BIT( in,1) ^ ((BIT(~in,3) && BIT(~in,6)) || (BIT( in,4) && BIT(~in,6)) || (BIT(~in,1) && BIT( in,3) && BIT(~in,4)));    // 1,3,4,6 repeated

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0203 */
int segac2_state::prot_func_puyo(int in)
{
	int const b0 = (BIT(~in,3) && BIT( in,7)) ^ ((BIT(~in,0) && BIT(~in,1)) || (BIT(~in,1) && BIT(~in,4))); // 1 repeated
	int const b1 = (BIT( in,3) && BIT( in,5)) ^ (BIT(~in,2) || BIT( in,4) || BIT( in,6));
	int const b2 = (BIT(~in,2) && BIT(~in,5)) ^ (BIT( in,1) || BIT(~in,3) || BIT(~in,6));
	int const b3 =  BIT( in,1)                ^ ((BIT( in,0) && BIT( in,3) && BIT( in,7)) || BIT( in,4));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0211 */
int segac2_state::prot_func_tantr(int in)
{
	int const b0 = (BIT( in,0) && BIT( in,4)) ^ ( BIT( in,5) || BIT(~in,6)  || (BIT(~in,3) && BIT( in,7)));
	int const b1 = (BIT( in,2) && BIT( in,6)) ^ ((BIT( in,1) && BIT( in,5)) || (BIT( in,3) && BIT( in,4)));
	int const b2 = (BIT(~in,0) && BIT( in,2)) ^ ( BIT( in,4) || BIT( in,7)  || (BIT( in,1) && BIT(~in,5)));
	int const b3 = (BIT(~in,2) && BIT( in,7)) ^ ( BIT(~in,0) || BIT( in,1)  || (BIT( in,3) && BIT( in,6)));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-???? */
int segac2_state::prot_func_tantrkor(int in)
{
	int const b0 = (BIT(~in,1) && BIT(~in,7)) ^ (BIT(~in,2) && BIT(~in,4));
	int const b1 = (BIT( in,2) && BIT( in,6)) ^ (BIT( in,0) && BIT( in,1));
	int const b2 = (BIT(~in,3) && BIT(~in,6)) ^ (BIT( in,1) || BIT(~in,4));
	int const b3 = (BIT(~in,0) && BIT(~in,2)) ^ (BIT( in,5) && BIT(~in,6));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0218 */
int segac2_state::prot_func_potopoto(int in)
{
	int const b0 = (BIT(~in,2) && BIT(~in,4)) ^ (BIT(~in,1) && BIT( in,3));
	int const b1 = (BIT( in,0) && BIT( in,5)) ^ (BIT( in,2) || BIT(~in,7));
	int const b2 = (BIT( in,0) && BIT( in,6)) ^ (BIT(~in,1) && BIT( in,7));
	int const b3 = (BIT( in,0) && BIT(~in,7)) ^ (BIT(~in,1) && BIT(~in,6));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0219 */
int segac2_state::prot_func_stkclmnj(int in)
{
	int const b0 = (BIT( in,1) && BIT( in,4)) ^ (BIT( in,5) && BIT( in,2));
	int const b1 = (BIT(~in,2) && BIT( in,6)) ^ (BIT(~in,5) && BIT( in,7));
	int const b2 = (BIT(~in,3) && BIT( in,6)) ^ (BIT(~in,5) && BIT(~in,1));
	int const b3 = (BIT(~in,3) && BIT( in,5)) ^ (BIT(~in,6) || BIT(~in,7));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0223 */
int segac2_state::prot_func_stkclmns(int in)
{
	int const b0 = (BIT( in,2) && BIT( in,4)) ^ (BIT( in,1) || BIT(~in,3));
	int const b1 = (BIT( in,0) && BIT( in,5)) ^ (BIT( in,2) && BIT( in,7));
	int const b2 = (BIT( in,0) && BIT(~in,6)) ^ (BIT( in,1) && BIT(~in,7));
	int const b3 = (BIT( in,0) && BIT(~in,7)) ^ (BIT(~in,1) && BIT( in,6));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0224 */
int segac2_state::prot_func_ichirj(int in)
{
	int const b0 = (BIT( in,2) && BIT( in,4)) ^ (BIT(~in,5) && BIT(~in,2));
	int const b1 = (BIT( in,2) && BIT(~in,6)) ^ (BIT( in,5) && BIT( in,7));
	int const b2 = (BIT(~in,3) && BIT( in,6)) ^ (BIT(~in,5) && BIT(~in,1));
	int const b3 = (BIT(~in,1) && BIT( in,5)) ^ (BIT(~in,5) && BIT( in,7));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-???? */
int segac2_state::prot_func_ichir(int in)
{
	int const b0 = (BIT(~in,2) && BIT( in,4)) ^ (BIT( in,5) && BIT(~in,2));
	int const b1 = (BIT( in,1) && BIT( in,6)) ^ (BIT( in,5) || BIT( in,7));
	int const b2 = (BIT(~in,3) && BIT( in,6)) ^ (BIT(~in,5) && BIT(~in,3));
	int const b3 = (BIT( in,0) && BIT(~in,5)) ^ (BIT( in,5) && BIT( in,7));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-???? */
int segac2_state::prot_func_ichirk(int in)
{
	int const b0 = (BIT(~in,2) && BIT( in,4)) ^ (BIT( in,5) && BIT(~in,1));
	int const b1 = (BIT( in,0) && BIT( in,6)) ^ (BIT( in,5) && BIT( in,4));
	int const b2 = (BIT(~in,1) && BIT(~in,6)) ^ (BIT(~in,5) && BIT( in,3));
	int const b3 = (BIT( in,1) && BIT( in,5)) ^ (BIT( in,6) && BIT( in,7));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0228 */
int segac2_state::prot_func_puyopuy2(int in)
{
	int const b0 = (BIT(~in,0) && BIT(~in,7)) ^ (BIT( in,1) || BIT(~in,4) || BIT(~in,6));
	int const b1 = (BIT( in,0) && BIT(~in,6)) ^ (BIT( in,3) && BIT( in,5));
	int const b2 = (BIT(~in,4) && BIT(~in,7)) ^ (BIT( in,0) || BIT(~in,6));
	int const b3 = (BIT(~in,1) && BIT( in,4)) ^ (BIT( in,2) && BIT(~in,3));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

int segac2_state::prot_func_zunkyou(int in)
{
	int const b0 = (BIT(~in,1) && BIT( in,6)) ^ (BIT(~in,5) && BIT( in,7));
	int const b1 = (BIT( in,0) && BIT(~in,5)) ^ (BIT(~in,3) || BIT( in,4));
	int const b2 = (BIT( in,2) && BIT(~in,3)) ^ (BIT( in,4) && BIT(~in,5));
	int const b3 = (BIT( in,0) && BIT(~in,4)) ^ (BIT(~in,2) && BIT(~in,6));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

int segac2_state::prot_func_pclub(int in)
{
	return 0xf;
}

int segac2_state::prot_func_pclubjv2(int in)
{
	int const b0 = (BIT( in,3) && BIT(~in,4)) ^ ((BIT(~in,1) && BIT(~in,7)) || BIT( in,6));
	int const b1 = (BIT( in,0) && BIT( in,5)) ^  (BIT( in,2) && BIT(~in,6));
	int const b2 = (BIT(~in,1) && BIT( in,6)) ^  (BIT( in,3) || BIT(~in,5)  || BIT(~in,1)); // 1 repeated
	int const b3 = (BIT(~in,2) && BIT(~in,7)) ^  (BIT(~in,0) || BIT(~in,4));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

int segac2_state::prot_func_pclubjv4(int in)
{
	int const b0 = (BIT(~in,2) && BIT( in,4)) ^ (BIT( in,1) && BIT(~in,6) && BIT(~in,3));
	int const b1 = (BIT(~in,3) && BIT(~in,4)) ^ (BIT( in,0) && BIT( in,5) && BIT(~in,6));
	int const b2 =  BIT(~in,0)                ^ (BIT( in,3) && BIT( in,4));
	int const b3 = (BIT(~in,1) && BIT( in,7)) ^ (BIT( in,5) && BIT(~in,7)); // 7 repeated

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

int segac2_state::prot_func_pclubjv5(int in)
{
	int const b0 = (BIT(~in,1) && BIT( in,5)) ^ (BIT(~in,2) && BIT(~in,6));
	int const b1 = (BIT(~in,0) && BIT( in,4)) ^ (BIT(~in,3) && BIT(~in,7));
	int const b2 = (BIT(~in,3) && BIT( in,7)) ^ (BIT(~in,0) || BIT(~in,4));
	int const b3 = (BIT(~in,2) && BIT( in,6)) ^ (BIT(~in,1) && BIT(~in,5));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}



DRIVER_INIT_MEMBER(segac2_state,c2boot)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_dummy),this));
}

DRIVER_INIT_MEMBER(segac2_state,bloxeedc)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_dummy),this));
}

DRIVER_INIT_MEMBER(segac2_state,columns)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_columns),this));
}

DRIVER_INIT_MEMBER(segac2_state,columns2)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_columns2),this));
}

DRIVER_INIT_MEMBER(segac2_state,tfrceac)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_tfrceac),this));
}

DRIVER_INIT_MEMBER(segac2_state,tfrceacb)
{
	/* disable the palette bank switching from the protection chip */
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_dummy),this));
	m_maincpu->space(AS_PROGRAM).nop_write(0x800000, 0x800001);
}

DRIVER_INIT_MEMBER(segac2_state,borench)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_borench),this));
}

DRIVER_INIT_MEMBER(segac2_state,twinsqua)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_twinsqua),this));
}

DRIVER_INIT_MEMBER(segac2_state,ribbit)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_ribbit),this));
}

DRIVER_INIT_MEMBER(segac2_state,puyo)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_puyo),this));
}

DRIVER_INIT_MEMBER(segac2_state,tantr)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_tantr),this));
}

DRIVER_INIT_MEMBER(segac2_state,tantrkor)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_tantrkor),this));
}

DRIVER_INIT_MEMBER(segac2_state,potopoto)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_potopoto),this));
}

DRIVER_INIT_MEMBER(segac2_state,stkclmns)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_stkclmns),this));
}

DRIVER_INIT_MEMBER(segac2_state,stkclmnj)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_stkclmnj),this));
}

DRIVER_INIT_MEMBER(segac2_state,ichir)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_ichir),this));
}

DRIVER_INIT_MEMBER(segac2_state,ichirk)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_ichirk),this));
}

DRIVER_INIT_MEMBER(segac2_state,ichirj)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_ichirj),this));
}

READ16_MEMBER(segac2_state::ichirjbl_prot_r )
{
	return 0x00f5;
}

DRIVER_INIT_MEMBER(segac2_state,ichirjbl)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_dummy),this));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x840108, 0x840109, read16_delegate(FUNC(segac2_state::ichirjbl_prot_r),this) );
}

DRIVER_INIT_MEMBER(segac2_state,puyopuy2)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_puyopuy2),this));
}

DRIVER_INIT_MEMBER(segac2_state,zunkyou)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_zunkyou),this));
}


DRIVER_INIT_MEMBER(segac2_state,pclub)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_pclub),this));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x880120, 0x880121, read16_delegate(FUNC(segac2_state::printer_r),this) );/*Print Club Vol.1*/
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x880124, 0x880125, read16_delegate(FUNC(segac2_state::printer_r),this) );/*Print Club Vol.2*/
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x880124, 0x880125, write16_delegate(FUNC(segac2_state::print_club_camera_w),this));
}

DRIVER_INIT_MEMBER(segac2_state,pclubjv2)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_pclubjv2),this));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x880120, 0x880121, read16_delegate(FUNC(segac2_state::printer_r),this) );/*Print Club Vol.1*/
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x880124, 0x880125, read16_delegate(FUNC(segac2_state::printer_r),this) );/*Print Club Vol.2*/
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x880124, 0x880125, write16_delegate(FUNC(segac2_state::print_club_camera_w),this));
}

DRIVER_INIT_MEMBER(segac2_state,pclubjv4)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_pclubjv4),this));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x880120, 0x880121, read16_delegate(FUNC(segac2_state::printer_r),this) );/*Print Club Vol.1*/
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x880124, 0x880125, read16_delegate(FUNC(segac2_state::printer_r),this) );/*Print Club Vol.2*/
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x880124, 0x880125, write16_delegate(FUNC(segac2_state::print_club_camera_w),this));
}

DRIVER_INIT_MEMBER(segac2_state,pclubjv5)
{
	segac2_common_init(segac2_prot_delegate(FUNC(segac2_state::prot_func_pclubjv5),this));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x880120, 0x880121, read16_delegate(FUNC(segac2_state::printer_r),this) );/*Print Club Vol.1*/
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x880124, 0x880125, read16_delegate(FUNC(segac2_state::printer_r),this) );/*Print Club Vol.2*/
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x880124, 0x880125, write16_delegate(FUNC(segac2_state::print_club_camera_w),this));
}



/******************************************************************************
    Game Drivers
*******************************************************************************

    These cover all the above games.

    Dates are all verified correct from Ingame display, some of the Titles
    such as Ichidant-R, Tant-R might be slightly incorrect as I've seen the
    games referred to by other names such as Ichident-R, Tanto-R, Tanto Arle
    etc.

    bloxeedc is set as as clone of bloxeed as it is the same game but running
    on a different piece of hardware.  The parent 'bloxeed' is a system18 game

******************************************************************************/

//    YEAR, NAME,      PARENT,   MACHINE,INPUT,    INIT,     MONITOR,COMPANY,FULLNAME,FLAGS
/* System C Games */
GAME( 1989, bloxeedc,  bloxeed,  segac,  bloxeedc, segac2_state, bloxeedc, ROT0,   "Sega / Elorg", "Bloxeed (World, C System)", 0 )
GAME( 1989, bloxeedu,  bloxeed,  segac,  bloxeedc, segac2_state, bloxeedc, ROT0,   "Sega / Elorg", "Bloxeed (US, C System)", 0 )

GAME( 1990, columns,   0,        segac,  columns, segac2_state,  columns,  ROT0,   "Sega", "Columns (World)", 0 )
GAME( 1990, columnsu,  columns,  segac,  columnsu, segac2_state, columns,  ROT0,   "Sega", "Columns (US, cocktail)", 0 ) // has cocktail mode dsw
GAME( 1990, columnsj,  columns,  segac,  columns, segac2_state,  columns,  ROT0,   "Sega", "Columns (Japan)", 0 )

GAME( 1990, columns2,  0,        segac,  columns2, segac2_state, columns2, ROT0,   "Sega", "Columns II: The Voyage Through Time (World)", 0 )
GAME( 1990, column2j,  columns2, segac,  columns2, segac2_state, columns2, ROT0,   "Sega", "Columns II: The Voyage Through Time (Japan)", 0 )

/* System C-2 Games */
GAME( 1990, tfrceac,   0,        segac2, tfrceac, segac2_state,  tfrceac,  ROT0,   "Technosoft / Sega", "Thunder Force AC", 0 )
GAME( 1990, tfrceacj,  tfrceac,  segac2, tfrceac, segac2_state,  tfrceac,  ROT0,   "Technosoft / Sega", "Thunder Force AC (Japan)", 0 )
GAME( 1990, tfrceacb,  tfrceac,  segac2, tfrceac, segac2_state,  tfrceacb, ROT0,   "bootleg", "Thunder Force AC (bootleg)", 0 )

GAME( 1990, borench,   0,        segac2, borench, segac2_state,  borench,  ROT0,   "Sega", "Borench (set 1)", 0 )
GAME( 1990, borencha,  borench,  segac2, borench, segac2_state,  borench,  ROT0,   "Sega", "Borench (set 2)", 0 )

GAME( 1991, twinsqua,  0,        segac2, twinsqua, segac2_state, twinsqua, ROT0,   "Sega", "Twin Squash", 0 )

GAME( 1991, ribbit,    0,        segac2, ribbit, segac2_state,   ribbit,   ROT0,   "Sega", "Ribbit!", 0 )

GAME( 1992, puyo,      0,        segac2, puyo, segac2_state,     puyo,     ROT0,   "Compile / Sega", "Puyo Puyo (World)", 0 )
GAME( 1992, puyobl,    puyo,     segac2, puyo, segac2_state,     puyo,     ROT0,   "bootleg", "Puyo Puyo (World, bootleg)", 0 )
GAME( 1992, puyoj,     puyo,     segac2, puyo, segac2_state,     puyo,     ROT0,   "Compile / Sega", "Puyo Puyo (Japan, Rev B)", 0 )
GAME( 1992, puyoja,    puyo,     segac2, puyo, segac2_state,     puyo,     ROT0,   "Compile / Sega", "Puyo Puyo (Japan, Rev A)", 0 )

GAME( 1992, tantr,     0,        segac2, ichir, segac2_state,    tantr,    ROT0,   "Sega", "Puzzle & Action: Tant-R (Japan)", 0 )
GAME( 1993, tantrkor,  tantr,    segac2, ichir, segac2_state,    tantrkor, ROT0,   "Sega", "Puzzle & Action: Tant-R (Korea)", 0 )
GAME( 1992, tantrbl,   tantr,    segac2, ichir, segac2_state,    c2boot,   ROT0,   "bootleg", "Puzzle & Action: Tant-R (Japan) (bootleg set 1)", 0 )
GAME( 1994, tantrbl2,  tantr,    segac,  ichir, segac2_state,    tantr,    ROT0,   "bootleg", "Puzzle & Action: Tant-R (Japan) (bootleg set 2)", 0 ) // Common bootleg in Europe, C board, no samples
GAME( 1994, tantrbl3,  tantr,    segac,  ichir, segac2_state,    tantr,    ROT0,   "bootleg", "Puzzle & Action: Tant-R (Japan) (bootleg set 3)", 0 ) // Common bootleg in Europe, C board, no samples

GAME( 1994, potopoto,  0,        segac2, potopoto, segac2_state, potopoto, ROT0,   "Sega", "Poto Poto (Japan)", 0 )

GAME( 1994, stkclmns,  0,        segac2, stkclmns, segac2_state, stkclmns, ROT0,   "Sega", "Stack Columns (World)", 0 )
GAME( 1994, stkclmnsj, stkclmns, segac2, stkclmns, segac2_state, stkclmnj, ROT0,   "Sega", "Stack Columns (Japan)", 0 )

GAME( 1994, ichir,     0,        segac2, ichir, segac2_state,    ichir,    ROT0,   "Sega", "Puzzle & Action: Ichidant-R (World)", 0 )
GAME( 1994, ichirk,    ichir,    segac2, ichir, segac2_state,    ichirk,   ROT0,   "Sega", "Puzzle & Action: Ichidant-R (Korea)", 0 )
GAME( 1994, ichirj,    ichir,    segac2, ichir, segac2_state,    ichirj,   ROT0,   "Sega", "Puzzle & Action: Ichidant-R (Japan)", 0 )
GAME( 1994, ichirjbl,  ichir,    segac,  ichir, segac2_state,    ichirjbl, ROT0,   "bootleg", "Puzzle & Action: Ichidant-R (Japan) (bootleg)", 0 ) // C board, no samples

GAME( 1994, puyopuy2,  0,        segac2, puyopuy2, segac2_state, puyopuy2, ROT0,   "Compile (Sega license)", "Puyo Puyo 2 (Japan)", 0 )

GAME( 1994, zunkyou,   0,        segac2, zunkyou, segac2_state,  zunkyou,  ROT0,   "Sega", "Zunzunkyou No Yabou (Japan)", 0 )

/* Atlus Print Club 'Games' (C-2 Hardware, might not be possible to support them because they use camera + printer, really just put here for reference) */
GAME( 1995, pclubj,    0,        segac2, pclub, segac2_state,    pclub,    ROT0,   "Atlus", "Print Club (Japan Vol.1)", MACHINE_NOT_WORKING )
GAME( 1995, pclubjv2,  pclubj,   segac2, pclubjv2, segac2_state, pclubjv2, ROT0,   "Atlus", "Print Club (Japan Vol.2)", MACHINE_NOT_WORKING )
GAME( 1996, pclubjv4,  pclubj,   segac2, pclubjv2, segac2_state, pclubjv4, ROT0,   "Atlus", "Print Club (Japan Vol.4)", MACHINE_NOT_WORKING )
GAME( 1996, pclubjv5,  pclubj,   segac2, pclubjv2, segac2_state, pclubjv5, ROT0,   "Atlus", "Print Club (Japan Vol.5)", MACHINE_NOT_WORKING )
