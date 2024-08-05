// license:BSD-3-Clause
// copyright-holders:David Haywood, Aaron Giles
/***********************************************************************************************

    Sega System C (System 14)/C2 Driver
    driver by David Haywood and Aaron Giles
    ---------------------------------------

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
    1991  Waku Waku Sonic Patrol Car Sega              317-0140         C2
    1992  Ribbit!                    Sega              317-0178         C2
    1992  Ribbit! (Japan)            Sega              317-0178         C2
    1992  Puyo Puyo                  Sega / Compile    317-0203         C2     171-5992A
    1992  Tant-R (Japan)             Sega              317-0211         C2
    1992  Tant-R (Korea)             Sega              ?                C2
    1992  Waku Waku Marine           Sega              317-0140         C2
    1993  SegaSonic Popcorn Shop     Sega              317-0140         C2
    1993  SegaSonic Cosmo Fighter    Sega              317-0140         C2
    1994  PotoPoto (Japan)           Sega              317-0218         C2
    1994  Stack Columns (Japan)      Sega              317-0219         C2
    1994  Stack Columns (World)      Sega              317-0223         C2
    1994  Ichidant-R (Japan)         Sega              317-0224         C2
    1994  Ichidant-R (World)         Sega              ?                C2
    1994  Ichidant-R (Korea)         Sega              ?                C2
    1994  Puyo Puyo 2                Compile           317-0228         C2
    1994  Zunzunkyou no Yabou        Sega              317-0221         C2

    1995  Print Club (Vol.1)         Atlus             ?                C2
    1995  Print Club (Vol.2)         Atlus             ?                C2
    1996  Print Club (Vol.3)         Atlus             317-5030         C2
    1996  Print Club (Vol.4)         Atlus             ?                C2
    1996  Print Club (Vol.5)         Atlus             ?                C2


    Notes:  Bloxeed doesn't read from the protection chip at all; all of the other games do.
            Currently the protection chip is mostly understood, and needs a table of 256
            4-bit values for each game. In all cases except for Poto Poto and Puyo Puyo 2,
            the table is embedded in the code. Workarounds for the other 2 cases are
            provided.

            I'm assuming System-C was the Board without the uPD7759 chip and System-C2 was the
            version of the board with it, this could be completely wrong but it doesn't really
            matter anyway.

    TODO:   - Puyo Puyo ends up with a black screen after doing memory tests
            - Battery-backed RAM needs to be figured out
            - Correct ROM labels for: ssonicbr, ooparts, headonch
            - Does ooparts actually use upd samples? PCB has the same rom as tfrceac
            - Is the joystick hack for ooparts unofficial, or done by Sega for location test?
            - Puyo Puyo 2 VS link capabilities

    The "Print Club" boards used by Atlus are variants of the C2 board. Important differences:
            - XL2 is 52.867MHz, instead of 53.693MHz.
            - The 315-5242 DAC is on a riser board called 837-7694. It has two 74LS273 ICs
              between the original DAC location and its spot on the riser.
            - There is a daughterboard attached to CN4, which is also populated in the reverse
              orientation compared to a normal C2 with CN4 populated (in terms of the polarizing
              key orientation).
            - The daughterboard is labeled 839-0805, and contains a TC82C55AM IO control IC,
              as well as some 74 series glue logic and passives. This board talks to other
              devices used in the print club machine. This board has a 1990 stamp on it, which
              dates it much earlier than the known Atlus Print Club games.
            - There is a larger board responsible for RGB multiplexing and encoding, named
              837-10507-01. It takes RGB in from both the C2 PCB and (presumably) the camera.
              ICs of note include Sony V7040, Sony CXA1688M, SBX1744-01 (submodule).
              It has two sets of RCA jacks, likely for NTSC-compatible camera input.
            - The wiring harness has power connections and monitor connections similar to or
              directly compatible with that of a Sega New Astro City monitor and power supply.

            With XL2 replaced, the DAC brought down from the riser, CN4 rotated 180 degrees, and
            the I/O daughterboard disconnected, it becomes a normal C2 PCB.

    Notes regarding Puyo Puyo 2's link capabilities (Mike Moffitt 2024-05-24):

    Puyo Puyo 2 boards often (but not always) have CN4 populated. There exists a daughterboard
    that can be installed to allow two PCBs to be networked for a four player mode. A video has
    been shown that confirms this functionality works, but supposedly this peripheral was not
    released by Compile/Sega and thus never reached mass production. A reproduction exists but
    is not for sale, and documentation of its functionality does not yet exist.

    From what I can tell, the daughtercard at CN4 is mapped into odd bytes at $880100. I think
    it is a small amount of RAM, shared with a Z80 processor or compatible microcontroller.

    Puyo Puyo 2 has a task that runs periodically to assess the status of the versus network.
    Its initialization looks like this as far as I can tell at the time of writing:

    CommMessage := $880101
    CommStatus  := $88010F
    CommCommand := $880111
    CommControl := $880131

            - Write $01 to CommControl, then write $00 six frames later (reset message?)
            - Sleep for 120 frames (two seconds)
            - Go back to top if the current frame count is not a multiple of 4...
            - Write $FC to CommCommand (signature request)
            - Look for a signature string "PUYO2Z80" from CommMessage (on odd bytes)
              If the signature has a mismatch, the process begins from the top.
              From this point the test menu will show that the comm board status is OK.
            - Write $FD to CommCommand (CRC request)
              If the result is zero, the test menu will clear the CRC error and proceed.
            - Now the string "PUYO268K" is written out to CommMessage (on odd bytes), across
              eight frames (one char per frame). Each write looks like this:
                  * Write $FF to CommCommand
                  * Write $01 to CommStatus
                  * Write $FD to CommCommand
                  * Write the character to CommMessage + (2 * index)
                  * Increment index, yield for one frame
            - Once again check that the current frame count is a multiple of 4, and if not,
              abort and go to the top of this process.
            - Command $FD is written once more (CRC request?) and we expect to see $FF in all
              eight characters of CommMessage
            - The test routine marks the board as good, but not yet the loop test.
            - Yield for one frame.
            - Command $FE is written to CommCommand, and it's expected that CommMessage's first
              byte is $00, otherwise the process rests.
            - With the $00 response identified, the status has the loop OK bit set, and the link
              status is entirely satisfactory. The test menu then proceeds to allow TX/RX.
            - The link task accepts a new routine pointer and stores it in its TCB to handle comms.

    This task runs in the background, doing this init periodically if a link isn't established.
    The test menu only exposes the current status but doesn't do anything to affect it, aside
    from letting the user manually reset the link status.

    The string would imply that there should be a Z80 system on the board, and some shared RAM.
    The 68000 side does not write a program of any sort to this IO area, so it's unlikely the
    Z80 software is packed into the C2 code anywhere (and I cannot find the PUYO2Z80 string).

    How the TX/RX works is not yet understood.


    Thanks: (in no particular order) to any MameDev that helped me out .. (OG, Mish etc.)
            Charles MacDonald for his C2Emu .. without it working out what were bugs in my code
                and issues due to protection would have probably killed the driver long ago :p
            Razoola & Antiriad .. for helping teach me some 68k ASM needed to work out just why
                the games were crashing :)
            Sega for producing some Fantastic Games...
            The Japanese magazine that proudly previewed OOPArts as "FARTS"
            and anyone else who knows they've contributed :)

***********************************************************************************************/

#include "emu.h"
#include "segaipt.h"
#include "m50dass.h"

#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "315_5296.h"
#include "sound/sn76496.h"
#include "sound/upd7759.h"
#include "sound/ymopn.h"
#include "video/315_5313.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_PROTECTION (1U << 1)
#define LOG_PALETTE    (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

#define XL1_CLOCK           XTAL(640'000)
#define XL2_CLOCK           XTAL(53'693'175)
// The Print Club PCBs use a slightly different master clock, likely for genlock compatibility.
#define XL2_CLOCK_PCLUB     XTAL(52'867'000)


typedef device_delegate<int (int in)> segac2_prot_delegate;

class segac2_state : public driver_device
{
public:
	segac2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_paletteram(*this, "paletteram")
		, m_vdp(*this, "gen_vdp")
		, m_screen(*this, "screen")
		, m_upd_region(*this, "upd")
		, m_upd7759(*this, "upd")
		, m_ymsnd(*this, "ymsnd")
		, m_palette(*this, "palette")
		, m_io(*this, "io")
		, m_prot_func(*this)
	{ }

	void segac2(machine_config &config);
	void segac(machine_config &config);

	void tfrceacjpb(machine_config &config);
	void ribbit(machine_config &config);
	void c2m50dass(machine_config &config);

	void init_noprot();
	void init_columns();
	void init_columns2();
	void init_tfrceac();
	void init_tfrceacb();
	void init_borench();
	void init_twinsqua();
	void init_ribbit();
	void init_puyo();
	void init_tantr();
	void init_tantrkor();
	void init_potopoto();
	void init_stkclmns();
	void init_stkclmnj();
	void init_ichir();
	void init_ichirk();
	void init_ichirj();
	void init_ichirjbl();
	void init_puyopuy2();
	void init_zunkyou();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	int m_segac2_enable_display;

	required_device<m68000_base_device> m_maincpu;
	required_shared_ptr<uint16_t> m_paletteram;
	required_device<sega315_5313_device> m_vdp;
	required_device<screen_device> m_screen;
	optional_memory_region m_upd_region;
	optional_device<upd7759_device> m_upd7759;
	required_device<ym3438_device> m_ymsnd;
	required_device<palette_device> m_palette;
	required_device<sega_315_5296_device> m_io;

	/* protection-related tracking */
	segac2_prot_delegate m_prot_func;     /* emulation of protection chip */
	uint8_t       m_prot_write_buf;       /* remembers what was written */
	uint8_t       m_prot_read_buf;        /* remembers what was returned */

	/* palette-related variables */
	uint8_t       m_segac2_alt_palette_mode;
	uint8_t       m_palbank;
	uint8_t       m_bg_palbase;
	uint8_t       m_sp_palbase;

	/* sound-related variables */
	uint8_t       m_sound_banks;      /* number of sound banks */

	void set_prot_func(segac2_prot_delegate prot_func) { m_prot_func = prot_func; }

	uint32_t screen_update_segac2_new(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int m_segac2_bg_pal_lookup[4];
	int m_segac2_sp_pal_lookup[4];
	void recompute_palette_tables();

	void vdp_sndirqline_callback_c2(int state);
	void vdp_lv6irqline_callback_c2(int state);
	void vdp_lv4irqline_callback_c2(int state);
	IRQ_CALLBACK_MEMBER(int_callback);

	uint8_t io_portc_r();
	void io_portd_w(uint8_t data);
	void io_porth_w(uint8_t data);

	void segac2_upd7759_w(uint8_t data);
	uint16_t palette_r(offs_t offset);
	void palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void control_w(uint8_t data);
	uint8_t prot_r();
	void prot_w(uint8_t data);
	void counter_timer_w(uint8_t data);
	uint16_t ichirjbl_prot_r();
	void segac2_irq2_interrupt(int state);

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

	void segac_map(address_map &map);
	void segac2_map(address_map &map);
};


class wwmarine_state : public segac2_state
{
public:
	wwmarine_state(const machine_config &mconfig, device_type type, const char *tag)
		: segac2_state(mconfig, type, tag)
		, m_wheel(*this, "WHEEL")
	{ }

	ioport_value read_wheel() { return m_wheel->read() | ((m_screen->frame_number() & 1) * 3); }

private:
	required_ioport m_wheel;
};


class pclub_state : public segac2_state
{
public:
	pclub_state(const machine_config &mconfig, device_type type, const char *tag)
		: segac2_state(mconfig, type, tag)
	{ }

	void pclub(machine_config &config);

	static constexpr feature_type unemulated_features() { return feature::CAMERA | feature::PRINTER; }

	void init_pclub();
	void init_pclubj();
	void init_pclubjv2();
	void init_pclubjv3();
	void init_pclubjv4();
	void init_pclubjv5();

private:
	int m_cam_data = 0;

	uint16_t printer_r();
	void print_club_camera_w(uint16_t data);

	int prot_func_pclub(int in);
	int prot_func_pclubjv2(int in);
	int prot_func_pclubjv3(int in);
	int prot_func_pclubjv4(int in);
	int prot_func_pclubjv5(int in);
};


/******************************************************************************
    Machine start/reset
******************************************************************************/

void segac2_state::machine_start()
{
	save_item(NAME(m_prot_write_buf));
	save_item(NAME(m_prot_read_buf));

	// determine how many sound banks
	if (m_upd_region != nullptr)
		m_sound_banks = m_upd_region->bytes() / 0x20000;
	else
		m_sound_banks = 0;

	// init the VDP
	m_vdp->set_use_cram(0); // C2 uses its own palette ram
	m_vdp->set_vdp_pal(false);
	m_vdp->set_framerate(60);
	m_vdp->set_total_scanlines(262);
	m_vdp->stop_timers();
}


void segac2_state::machine_reset()
{
	m_segac2_bg_pal_lookup[0] = 0x00;
	m_segac2_bg_pal_lookup[1] = 0x10;
	m_segac2_bg_pal_lookup[2] = 0x20;
	m_segac2_bg_pal_lookup[3] = 0x30;

	m_segac2_sp_pal_lookup[0] = 0x00;
	m_segac2_sp_pal_lookup[1] = 0x10;
	m_segac2_sp_pal_lookup[2] = 0x20;
	m_segac2_sp_pal_lookup[3] = 0x30;

	m_vdp->device_reset_old();

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

    These handlers are responsible for communicating with the (generally)
    8-bit sound chips. All accesses are via the low byte.

    The Sega C/C2 system uses a YM3438 (compatible with the YM2612) for FM-
    based music generation, and an SN76489 for PSG and noise effects. The
    C2 board also appears to have a UPD7759 for sample playback.

******************************************************************************/

/* handle writes to the UPD7759 */
void segac2_state::segac2_upd7759_w(uint8_t data)
{
	m_upd7759->port_w(data);
	m_upd7759->start_w(0);
	m_upd7759->start_w(1);
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
uint16_t segac2_state::palette_r(offs_t offset)
{
	offset &= 0x1ff;
	if (m_segac2_alt_palette_mode)
		offset = ((offset << 1) & 0x100) | ((offset << 2) & 0x80) | ((~offset >> 2) & 0x40) | ((offset >> 1) & 0x20) | (offset & 0x1f);

	return m_paletteram[offset + m_palbank * 0x200];
}

/* handle writes to the paletteram */
void segac2_state::palette_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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

uint8_t segac2_state::io_portc_r()
{
	// D7 : From MB3773P pin 1. (/RESET output)
	// D6 : From uPD7759 pin 18. (/BUSY output)
	int busy = (m_upd7759 != nullptr) ? (m_upd7759->busy_r() << 6) : 0x40;
	return 0xbf | busy;
}

void segac2_state::io_portd_w(uint8_t data)
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
	//machine().bookkeeping().coin_lockout_w(1, data & 0x08);
	//machine().bookkeeping().coin_lockout_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
}

void segac2_state::io_porth_w(uint8_t data)
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
		m_upd7759->set_rom_bank(newbank);
	}
}


/******************************************************************************
    Control Write Handler
*******************************************************************************

    Seems to control some global states. The most important bit is the low
    one, which enables/disables the display. This is used while tiles are
    being modified in Bloxeed.

******************************************************************************/

void segac2_state::control_w(uint8_t data)
{
	data &= 0x0f;

	/* bit 0 controls display enable */
	//segac2_enable_display(machine(), ~data & 1);
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
uint8_t segac2_state::prot_r()
{
	LOGMASKED(LOG_PROTECTION, "%06X:protection r=%02X\n", m_maincpu->pcbase(), m_prot_read_buf);
	return m_prot_read_buf | 0xf0;
}


/* protection chip writes */
void segac2_state::prot_w(uint8_t data)
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
	LOGMASKED(LOG_PROTECTION, "%06X:protection w=%02X, new result=%02X\n", m_maincpu->pcbase(), data & 0x0f, m_prot_read_buf);

	/* if the palette changed, force an update */
	if (new_sp_palbase != m_sp_palbase || new_bg_palbase != m_bg_palbase)
	{
		//m_screen->update_partial(m_screen->vpos() + 1);
		m_sp_palbase = new_sp_palbase;
		m_bg_palbase = new_bg_palbase;
		recompute_palette_tables();
		if (m_screen) LOGMASKED(LOG_PALETTE, "Set palbank: %d/%d (scan=%d)\n", m_bg_palbase, m_sp_palbase, m_screen->vpos());
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

void segac2_state::counter_timer_w(uint8_t data)
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
//          machine().bookkeeping().coin_counter_w(0,1);
//          machine().bookkeeping().coin_counter_w(0,0);
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

uint16_t pclub_state::printer_r()
{
	return m_cam_data;
}

void pclub_state::print_club_camera_w(uint16_t data)
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

void segac2_state::segac_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x800000, 0x800001).mirror(0x13fdfe).rw(FUNC(segac2_state::prot_r), FUNC(segac2_state::prot_w)).umask16(0x00ff);
	map(0x800200, 0x800201).mirror(0x13fdfe).w(FUNC(segac2_state::control_w)).umask16(0x00ff);
	map(0x840000, 0x84001f).mirror(0x13fee0).rw("io", FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write)).umask16(0x00ff);
	map(0x840100, 0x840107).mirror(0x13fef8).rw(m_ymsnd, FUNC(ym3438_device::read), FUNC(ym3438_device::write)).umask16(0x00ff);
	map(0x880100, 0x880101).mirror(0x13fefe).w(FUNC(segac2_state::counter_timer_w)).umask16(0x00ff);
	map(0x8c0000, 0x8c0fff).mirror(0x13f000).rw(FUNC(segac2_state::palette_r), FUNC(segac2_state::palette_w)).share("paletteram");
	map(0xc00000, 0xc0001f).mirror(0x18ff00).rw(m_vdp, FUNC(sega315_5313_device::vdp_r), FUNC(sega315_5313_device::vdp_w));
	map(0xe00000, 0xe0ffff).mirror(0x1f0000).ram().share("nvram");
}

void segac2_state::segac2_map(address_map &map)
{
	segac_map(map);
	map(0x880000, 0x880001).mirror(0x13fefe).w(FUNC(segac2_state::segac2_upd7759_w)).umask16(0x00ff);
}


/******************************************************************************
    Input Ports
*******************************************************************************

    The input ports on the C2 games always consist of 1 Coin Port, 2 Player
    Input ports and 2 Dipswitch Ports, 1 of those Dipswitch Ports being used
    for coinage, the other for Game Options.

    Most of the Games List the Dipswitches and Inputs in the Test Menus, adding
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


static INPUT_PORTS_START( soniccar )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // siren
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // jump
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) // turbo
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) // 'winker' (a left/right joystick)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) // steering (it's a wheel on the control panel, but just seems to send left/right signals)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) // accel
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x03, "Demo Sound Interval" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, "Every 4 Minutes" )
	PORT_DIPSETTING(    0x02, "Every 2 Minutes" )
	PORT_DIPSETTING(    0x03, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Lighting Time" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Advertise & Playtime" )
	PORT_DIPSETTING(    0x00, "Playtime Only" )
	PORT_DIPNAME( 0x08, 0x08, "Light" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Screen Display" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Insert 100-400 Yen" )
	PORT_DIPSETTING(    0x00, "Insert Money" )
	//"SW2:6" unused
	//"SW2:7" unused
	//"SW2:7" unused
INPUT_PORTS_END


static INPUT_PORTS_START( wwmarine )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // Button 1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Button 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) // Lever
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, 0x00, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(wwmarine_state, read_wheel)

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x03, "Demo Sound Interval" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, "Every 3 Demo Cycles" )
	PORT_DIPSETTING(    0x02, "Every 2 Demo Cycles" )
	PORT_DIPSETTING(    0x03, DEF_STR( On ) ) // Listed as NO INTERVAL
	PORT_DIPNAME( 0x04, 0x04, "Capsule Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	//"SW2:4" unused
	PORT_DIPNAME( 0x10, 0x10, "Credit Mode" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//"SW2:6" unused
	//"SW2:7" unused
	//"SW2:7" unused

	PORT_START("WHEEL") // free-spinning wheel, some mechanism causing the input to trigger repeatedly
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END


static INPUT_PORTS_START( sonicfgt )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // shot
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // transform
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) // lever
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )

	PORT_MODIFY("P2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM ) // limit sw
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Credit Mode" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//"SW2:2" unused
	//"SW2:3" unused
	//"SW2:4" unused
	//"SW2:5" unused
	//"SW2:6" unused
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END


static INPUT_PORTS_START( sonicpop )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Relay")  // relay (must be ON by default or machine will instantly give an 'assistance' error) - pressing this advances the stages of operation (from type select, and during the turning of the wheel)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Coinblock") // coinblock (inverted)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Sensor") // sensor - causes an extra animation to play if you press it during attract and 'Sensor Advertise' dip is on
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Cup Select 2")  // cup select 2 - pressing Cup Select 1 and 2 registers as 'Cup Select 3', I presume these are lines from the mechanical part.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Cup Select 1") // cup select 1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Handle B")  // handle B
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Handle A")  // handle A

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 )  PORT_NAME("Sold Out LED1") // sold out LED 1 - are these actually output lines?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Sold Out LED2") // sold out LED 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Sold Out LED3") // sold out LED 3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Sold Out SW2") // sold out SW 2 - holding these while coining up will show popcorn as unavailable, pressing all 3 will fault the machine
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Sold Out SW3") // sold out SW 3
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Sold Out SW1") // sold out SW 1

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) // 'reset'
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x02, "Trouble BGM" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0c, 0x00, "Region" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( USA ) )
	PORT_DIPSETTING(    0x04, "Export" )
	PORT_DIPSETTING(    0x08, "USA (duplicate)" )
	PORT_DIPSETTING(    0x0c, DEF_STR( Japan ) )
	PORT_DIPNAME( 0x10, 0x10, "Sensor Advertise" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( anpanman )
	PORT_INCLUDE( sonicpop )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x02, "Demo Voice" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
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


static INPUT_PORTS_START( ribbitj )
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
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4") /* Lives are different */
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
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


static INPUT_PORTS_START( headonch )
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
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	//"SW2:6" unused
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END


static INPUT_PORTS_START( ssonicbr )
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 2 Unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
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


static INPUT_PORTS_START( ooparts ) // testmode expects controls similar to twinsqua
	PORT_INCLUDE( systemc_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("P2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     /* Button 3 Unused */

	PORT_MODIFY("SERVICE") // "shot" button in testmode (needed for sound test)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x60, 0x60, "Region" ) PORT_DIPLOCATION("SW2:6,7") // undocumented
	PORT_DIPSETTING(    0x60, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x40, DEF_STR( USA ) )
	PORT_DIPSETTING(    0x20, "Export" )
	PORT_DIPSETTING(    0x00, "Export" )
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
	PORT_DIPNAME( 0x01, 0x01, "Credits to Start (VS)" )     PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, "1" ) PORT_CONDITION("DSW", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x01, "2" ) PORT_CONDITION("DSW", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x00, "2" ) PORT_CONDITION("DSW", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, "4" ) PORT_CONDITION("DSW", 0x02, EQUALS, 0x00)
	PORT_DIPNAME( 0x02, 0x02, "Credits to Start (Normal)" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	//"SW2:3" unused
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	//"SW2:7" unused
	PORT_DIPNAME( 0x80, 0x00, "High Speed Mode" )           PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bloxeedu )
	PORT_INCLUDE( bloxeedc )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x03, "Credits to Start (Normal / VS)" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "1 / 1" )
	PORT_DIPSETTING(    0x03, "1 / 2" )
	PORT_DIPSETTING(    0x02, "1 / 2" )
	PORT_DIPSETTING(    0x00, "2 / 4" )
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
    Screen update
*******************************************************************************

    C2 doesn't use the internal VDP CRAM, instead it uses the digital
    output of the chip and applies it's own external colour circuity.

******************************************************************************/

uint32_t segac2_state::screen_update_segac2_new(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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
		uint32_t *const desty = &bitmap.pix(y, 0);
		uint16_t const *const srcy = m_vdp->m_render_line_raw.get();

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint16_t src = srcy[x];
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


/******************************************************************************
    Interrupt handling
******************************************************************************/

void segac2_state::segac2_irq2_interrupt(int state)
{
	//printf("sound irq %d\n", state);
	m_maincpu->set_input_line(2, state ? ASSERT_LINE : CLEAR_LINE);
}

// the main interrupt on C2 comes from the vdp line used to drive the z80 interrupt on a regular genesis(!)
void segac2_state::vdp_sndirqline_callback_c2(int state)
{
	if (state == ASSERT_LINE)
		m_maincpu->set_input_line(6, HOLD_LINE);
}

// the line usually used to drive irq6 is not connected
void segac2_state::vdp_lv6irqline_callback_c2(int state)
{
	//
}

// the scanline interrupt seems connected as usual
void segac2_state::vdp_lv4irqline_callback_c2(int state)
{
	if (state == ASSERT_LINE)
		m_maincpu->set_input_line(4, HOLD_LINE);
	else
		m_maincpu->set_input_line(4, CLEAR_LINE);
}

/* Callback when the 68k takes an IRQ */
IRQ_CALLBACK_MEMBER(segac2_state::int_callback)
{
	if (irqline == 4)
		m_vdp->vdp_clear_irq4_pending();

	return (0x60 + irqline * 4) / 4; // vector address
}


/******************************************************************************
    Machine Drivers
*******************************************************************************

    General Overview
        M68000 @ 10MHz (Main Processor)
        YM3438 (Fm Sound)
        SN76489 (PSG, Noise, Part of the VDP)
        UPD7759 (Sample Playback, C-2 Only)

    sound output balance (tfrceac)
    reference 1: https://youtu.be/AOmeWp9qe5E
    reference 2: https://youtu.be/Tq8VkJYmij8
    reference 3: https://youtu.be/VId_HWdNuyA

******************************************************************************/

void segac2_state::segac(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XL2_CLOCK/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &segac2_state::segac_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(segac2_state::int_callback));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1); // borencha requires 0xff fill or there is no sound (it lacks some of the init code of the borench set)

	SEGA_315_5296(config, m_io, XL2_CLOCK/6); // clock divider guessed
	m_io->in_pa_callback().set_ioport("P1");
	m_io->in_pb_callback().set_ioport("P2");
	m_io->in_pc_callback().set(FUNC(segac2_state::io_portc_r));
	m_io->out_pd_callback().set(FUNC(segac2_state::io_portd_w));
	m_io->in_pe_callback().set_ioport("SERVICE");
	m_io->in_pf_callback().set_ioport("COINAGE");
	m_io->in_pg_callback().set_ioport("DSW");
	m_io->out_ph_callback().set(FUNC(segac2_state::io_porth_w));

	/* video hardware */
	SEGA315_5313(config, m_vdp, XL2_CLOCK, m_maincpu);
	m_vdp->set_is_pal(false);
	m_vdp->snd_irq().set(FUNC(segac2_state::vdp_sndirqline_callback_c2));
	m_vdp->lv6_irq().set(FUNC(segac2_state::vdp_lv6irqline_callback_c2));
	m_vdp->lv4_irq().set(FUNC(segac2_state::vdp_lv4irqline_callback_c2));
	m_vdp->set_alt_timing(1);
	m_vdp->set_screen("screen");
	m_vdp->add_route(ALL_OUTPUTS, "mono", 0.50);
	m_vdp->set_palette(m_palette);

	TIMER(config, "scantimer").configure_scanline("gen_vdp", FUNC(sega315_5313_device::megadriv_scanline_timer_callback_alt_timing), "screen", 0, 1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(double(XL2_CLOCK.value()) / 10.0 / 262.0 / 342.0); // same as SMS?
//  m_screen->set_refresh_hz(double(XL2_CLOCK.value()) / 8.0 / 262.0 / 427.0); // or 427 Htotal?
	m_screen->set_size(512, 262);
	m_screen->set_visarea(0, 32*8-1, 0, 28*8-1);
	m_screen->set_screen_update(FUNC(segac2_state::screen_update_segac2_new));

	PALETTE(config, m_palette).set_entries(2048*3);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM3438(config, m_ymsnd, XL2_CLOCK/7);
	m_ymsnd->irq_handler().set(FUNC(segac2_state::segac2_irq2_interrupt));
	m_ymsnd->add_route(0, "mono", 0.50);
	/* right channel not connected */
}

void segac2_state::segac2(machine_config &config)
{
	segac(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &segac2_state::segac2_map);
	subdevice<sega_315_5296_device>("io")->out_cnt1_callback().set(m_upd7759, FUNC(upd7759_device::reset_w));

	/* sound hardware */
	UPD7759(config, m_upd7759, XL1_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.50);
}

void segac2_state::tfrceacjpb(machine_config& config)
{
	segac2(config);
	m_io->set_ddr_override(0xf); // game erroneously writes 0x58 to DDR
}

void segac2_state::ribbit(machine_config& config)
{
	segac2(config);

	// Ribbit does random measure of UPD7759 sample #A playback time and reset to round 1 if it's not in expected range (see routine @1D8D2)
	// current UPD code is too fast, add slight delay
	m_upd7759->set_start_delay(250);
}

// Games supporting Megalo 50 moving seats (DASS)
void segac2_state::c2m50dass(machine_config& config)
{
	segac2(config);

	MEGALO50_DASS(config, "m50dass");
}

void pclub_state::pclub(machine_config& config)
{
	segac2(config);
	// Print Club boards use a different crystal, possibly for better compatibility with the camera timings.
	m_maincpu->set_clock(XL2_CLOCK_PCLUB/6);
}


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


ROM_START( bloxeedu ) /* Bloxeed USA (C System Version)  (c)1989 Sega / Elorg - 834-7306-01 BLOXEED Rev.A */
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


ROM_START( columns2 ) /* Columns II - The Voyage Through Time  (c)1990 Sega - 834-7555-01 COLUMNS 2 */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13363.ic32", 0x000000, 0x020000, CRC(c99e4ffd) SHA1(67981aa08c8a625af35dd7689011364159cf9194) )
	ROM_LOAD16_BYTE( "epr-13362.ic31", 0x000001, 0x020000, CRC(394e2419) SHA1(d4f726b32cf301d0d52611237b83177e69bfaf71) )
ROM_END


ROM_START( column2j ) /* Columns II - The Voyage Through Time (Jpn)  (c)1990 Sega - 834-7555 COLUMNS 2 */
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


ROM_START( ichirjbl ) /* Ichidant-R (Puzzle & Action 2) (Bootleg Running on C Board?, No Samples) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "27c4000.2",0x000000, 0x080000, CRC(5a194f44) SHA1(67a4d21b91704f8c2210b5106e82e22ba3366f4c) )
	ROM_LOAD16_BYTE( "27c4000.1",0x000001, 0x080000, CRC(de209f84) SHA1(0860d0ebfab2952e82fc1e292bf9410d673d9322) )
	ROM_LOAD16_BYTE( "epr-16888", 0x100000, 0x080000, CRC(85d73722) SHA1(7ebe81b4d6c89f87f60200a3a8cddb07d581adef) )
	ROM_LOAD16_BYTE( "epr-16887", 0x100001, 0x080000, CRC(bc3bbf25) SHA1(e760ad400bc183b38e9787d88c8ac084fbe2ae21) )
ROM_END


/* ----- System C-2 Games ----- */

ROM_START( borench ) /* Borench  (c)1990 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ic32.bin", 0x000000, 0x040000, CRC(2c54457d) SHA1(adf3ea5393d2633ec6215e64f0cd89ad4567e765) ) // Need to verify proper label - EPR-13589?
	ROM_LOAD16_BYTE( "ic31.bin", 0x000001, 0x040000, CRC(b46445fc) SHA1(24e85ef5abbc5376a854b13ed90f08f0c30d7f25) ) // Need to verify proper label - EPR-13588?

	ROM_REGION( 0x020000, "upd", 0 )
	ROM_LOAD( "epr-13587.ic4", 0x000000, 0x020000, CRC(62b85e56) SHA1(822ab733c87938bb70a9e32cc5dd36bbf6f21d11) )
ROM_END


ROM_START( borencha ) /* Borench  (c)1990 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13591.ic32", 0x000000, 0x040000, CRC(7851078b) SHA1(122934f0414a29b4b363acad01ee4db369259e72) )
	ROM_LOAD16_BYTE( "epr-13590.ic31", 0x000001, 0x040000, CRC(01bc6fe6) SHA1(b241b09852f52f712e3ddc6660ec3eb436b1302c) )

	ROM_REGION( 0x020000, "upd", 0 )
	ROM_LOAD( "epr-13587.ic4", 0x000000, 0x020000, CRC(62b85e56) SHA1(822ab733c87938bb70a9e32cc5dd36bbf6f21d11) )
ROM_END


ROM_START( borenchj ) /* Borench  (c)1990 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13586.ic32", 0x000000, 0x040000, CRC(62d7f8e8) SHA1(a2b11584c79ead70e6b8cab0b076df9bbf114803) )
	ROM_LOAD16_BYTE( "epr-13585.ic31", 0x000001, 0x040000, CRC(087b9704) SHA1(1e974d1e40ea28e8f5e721ccf78a2ffc76f4d17d) )

	ROM_REGION( 0x020000, "upd", 0 )
	ROM_LOAD( "epr-13587.ic4", 0x000000, 0x020000, CRC(62b85e56) SHA1(822ab733c87938bb70a9e32cc5dd36bbf6f21d11) )
ROM_END


ROM_START( tfrceac ) /* Thunder Force AC  (c)1990 Technosoft / Sega - 834-7745-02 THUNDER FORCE AC (EMP5032 labeled 317-0172) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13675.ic32", 0x000000, 0x040000, CRC(95ecf202) SHA1(92b0f351f2bee7d59873a4991615f14f1afe4da7) )
	ROM_LOAD16_BYTE( "epr-13674.ic31", 0x000001, 0x040000, CRC(e63d7f1a) SHA1(a40d0a5a96f379a467048dc8fddd8aaaeb94da1d) )
	/* 0x080000 - 0x100000 Empty */
	ROM_LOAD16_BYTE( "epr-13659.ic34", 0x100000, 0x040000, CRC(29f23461) SHA1(032a7125fef5a660b85654d595aafc46812cdde6) )
	ROM_LOAD16_BYTE( "epr-13658.ic33", 0x100001, 0x040000, CRC(9e23734f) SHA1(64d27dc53f0ffc3513345a26ed077751b25d15f1) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-13655.ic4", 0x000000, 0x040000, CRC(e09961f6) SHA1(e109b5f41502b765d191f22e3bbcff97d6defaa1) )
ROM_END


ROM_START( tfrceacj ) /* Thunder Force AC (Japan)  (c)1990 Technosoft / Sega - 834-7745 THUNDER FORCE AC (EMP5032 labeled 317-0172) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13657.ic32", 0x000000, 0x040000, CRC(a0f38ffd) SHA1(da548e7f61aed0e82a460553a119941da8857bc4) )
	ROM_LOAD16_BYTE( "epr-13656.ic31", 0x000001, 0x040000, CRC(b9438d1e) SHA1(598209c9fec3527fde720af09e5bebd7379f5b2b) )
	/* 0x080000 - 0x100000 Empty */
	ROM_LOAD16_BYTE( "epr-13659.ic34", 0x100000, 0x040000, CRC(29f23461) SHA1(032a7125fef5a660b85654d595aafc46812cdde6) )
	ROM_LOAD16_BYTE( "epr-13658.ic33", 0x100001, 0x040000, CRC(9e23734f) SHA1(64d27dc53f0ffc3513345a26ed077751b25d15f1) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-13655.ic4", 0x000000, 0x040000, CRC(e09961f6) SHA1(e109b5f41502b765d191f22e3bbcff97d6defaa1) )
ROM_END

/* This set has significantly different code addresses to both the Genesis Thunder Force III and the arcade Thunder Force AC, and seems to sit somewhere between them
   Some sources indicate it's a hack, but it might be a hack of an otherwise unsupported set, with the protection removed, for bootleggers to sell at a profit.
   This specific dump was sourced from a PCB sold in Canada */
ROM_START( tfrceacjpb ) // protection chip simply marked T-FORCE (not used outside of startup init?)
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ic32_t.f.ac_075f.ic32", 0x000000, 0x040000, CRC(2167dd93) SHA1(0e5b8eb87e07e6e5cecf096e6b62e15ff7406bba) )
	ROM_LOAD16_BYTE( "ic31_t.f.ac_0d26.id31", 0x000001, 0x040000, CRC(ebf02bba) SHA1(effdc60837063ba04b7b4517e57a240b29a199e1) )
	/* 0x080000 - 0x100000 Empty */
	ROM_LOAD16_BYTE( "ic34_t.f.ac_549d.ic34", 0x100000, 0x040000, CRC(902ad2ec) SHA1(58db20ca5888110e97f80e7df9dac1b8e9817562) )
	ROM_LOAD16_BYTE( "ic33_t.f.ac_d131.ic33", 0x100001, 0x040000, CRC(b162219d) SHA1(ad022307019b4a70cb532e1101cb5ed8d31f10e2) )

	ROM_REGION( 0x040000, "upd", ROMREGION_ERASE00 )
	// empty socket (not used)
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


ROM_START( soniccar ) /* Waku Waku Sonic Patrol Car  (c)1991 Sega - 834-8401 SONIC PATCAR (EMP5032 labeled 317-0140) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-14369.ic32", 0x000000, 0x040000, CRC(2ea4c9a3) SHA1(92ee27c93bd1cbb99c88af39f5f227a8cadfb13c) )
	ROM_LOAD16_BYTE( "epr-14395.ic31", 0x000001, 0x040000, CRC(39622e18) SHA1(b7093c6d86e429e9721a0e1fd781231dc4342b31) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-14394.ic4", 0x000000, 0x040000, CRC(476e30dd) SHA1(c9d381160c58b05763ea286a53c7ca6de074fda2) )
ROM_END


ROM_START( wwmarine ) /* Waku Waku Marine  (c)1992 Sega - 834-9082 WAKUWAKU MARINE (EMP5032 labeled 317-0140) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-15097.ic32", 0x000000, 0x040000, CRC(1223a77a) SHA1(3725f17745da0d1f2d50b0a99a37b768c5539ce2) )
	ROM_LOAD16_BYTE( "epr-15096.ic31", 0x000001, 0x040000, CRC(1b833932) SHA1(a37caa75cbcc2b7ce84fccfd5768e6d1580c0bc0) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-15095.ic4", 0x000000, 0x040000, CRC(df13755b) SHA1(177aac7aaadc36e14dbcdf12bd42dbe70b3edd49) )
ROM_END


ROM_START( wwanpanm ) /* Waku Waku Anpanman - 834-8191 sticker */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-14123a.ic32", 0x000000, 0x040000, CRC(0e4f38c6) SHA1(2913fbde9a7e6428bab05c6e550c3e2d79c9f211) )
	ROM_LOAD16_BYTE( "epr-14122a.ic31", 0x000001, 0x040000, CRC(01b8fe20) SHA1(8d21c346b141a298074d199ce2bc4094217e8c25) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-14121.ic4", 0x000000, 0x040000, CRC(69adf3a1) SHA1(63233e723ab9be8d5663651cb2e6e54b64a7bb8e) )
ROM_END


ROM_START( ssonicbr ) // hack: supposedly the data ROM mapping was modified
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssonicbr.ic32", 0x000000, 0x040000, CRC(cf254ecd) SHA1(4bb295ec80f8ddfeab4e360eebf12c5e2dfb9800) )
	ROM_LOAD16_BYTE( "ssonicbr.ic31", 0x000001, 0x040000, CRC(03709746) SHA1(0b457f557da77acd3f43950428117c1decdfaf26) )

	ROM_REGION( 0x020000, "upd", 0 )
	ROM_LOAD( "ssonicbr.ic4", 0x000000, 0x020000, CRC(78e56a51) SHA1(8a72c12975cd74919b4337e0f681273e6b5cbbc6) )
ROM_END


ROM_START( anpanman ) /* Sega Soreike! Anpanman Popcorn Koujou (Rev.B) (c)1993 Sega - 834-8795-01 (EMP5032 labeled 317-0140) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-14804b.ic32", 0x000000, 0x040000, CRC(7ce88c49) SHA1(959ee459a5b4a6324488a935fa6a48e38ce93464) ) // 27C020
	ROM_LOAD16_BYTE( "epr-14803b.ic31", 0x000001, 0x040000, CRC(eb3ca1b9) SHA1(e4dd9d2bd2301f47f167d6516457386ba4e57df0) ) // 27C020
	ROM_LOAD16_BYTE( "epr-14806.ic34",  0x100000, 0x040000, CRC(40f398db) SHA1(abaaf4404342232b58f02c7ea0571926b2417b45) ) // 27C020
	ROM_LOAD16_BYTE( "epr-14805.ic33",  0x100001, 0x040000, CRC(f27229ed) SHA1(307182cdcd8954bbc56d4b412df452b406466d53) ) // 27C020

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-14807.ic4", 0x000000, 0x040000, CRC(9827549f) SHA1(66d195299085ec690498fc795a3088c05e6db820) ) // 27C020
ROM_END

ROM_START( anpanmana ) /* Sega Soreike! Anpanman Popcorn Koujou (Rev.A) (c)1993 Sega - 834-8795-01 (EMP5032 labeled 317-0140) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-14804a.ic32", 0x000000, 0x040000, CRC(a80bd024) SHA1(bfd3cc45214cd89d5cbed99f88ba2d3e8cc61538) ) // 27C020
	ROM_LOAD16_BYTE( "epr-14803a.ic31", 0x000001, 0x040000, CRC(32e1f248) SHA1(151c59d0be5bb2e77c777da2b99627c887d49f55) ) // 27C020
	ROM_LOAD16_BYTE( "epr-14806.ic34",  0x100000, 0x040000, CRC(40f398db) SHA1(abaaf4404342232b58f02c7ea0571926b2417b45) ) // 27C020
	ROM_LOAD16_BYTE( "epr-14805.ic33",  0x100001, 0x040000, CRC(f27229ed) SHA1(307182cdcd8954bbc56d4b412df452b406466d53) ) // 27C020

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-14807.ic4", 0x000000, 0x040000, CRC(9827549f) SHA1(66d195299085ec690498fc795a3088c05e6db820) ) // 27C020
ROM_END


ROM_START( sonicpop ) /* SegaSonic Popcorn Shop (Rev.B) (c)1993 Sega - 834-9555-02 (EMP5032 labeled 317-0140) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-14592b.ic32", 0x000000, 0x040000, CRC(bac586a1) SHA1(0208213bfa1a5093e76edb1a7e0ba5ebc862801d) )
	ROM_LOAD16_BYTE( "epr-15491b.ic31", 0x000001, 0x040000, CRC(527106c3) SHA1(97f08006bba4b87c304c7ad3b1480b77e99dff10) )
	ROM_LOAD16_BYTE( "epr-15494.ic34",  0x100000, 0x040000, CRC(0520df5e) SHA1(5a795a1630d841406a106a566228223583deef44) )
	ROM_LOAD16_BYTE( "epr-15493.ic33",  0x100001, 0x040000, CRC(d51b3b85) SHA1(66f6b4656841ab70ffde0141a613eaf91c06f86b) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-15495.ic4", 0x000000, 0x040000, CRC(d3ee4c68) SHA1(557c57b22521339d94d9a3e6fd2af68a67a153b6) )
ROM_END


ROM_START( sonicfgt ) /* SegaSonic Cosmo Fighter (c)1993 Sega - 834-10082 940725-3598T (EMP5032 labeled 317-0140) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-17178.ic32", 0x000000, 0x040000, CRC(e05b7388) SHA1(38fd175c265986a14d1365e3a403e12fbc6c73c3) )
	ROM_LOAD16_BYTE( "epr-17177.ic31", 0x000001, 0x040000, CRC(7c2ec4eb) SHA1(f74f21f5dcfbf6ee902c2a44fed908a628d60363) )
	ROM_LOAD16_BYTE( "epr-17180.ic34", 0x100000, 0x040000, CRC(8933e91c) SHA1(5dc7451874f97e0e5d0c666800c26907b9abf5f5) )
	ROM_LOAD16_BYTE( "epr-17179.ic33", 0x100001, 0x040000, CRC(0ae979cd) SHA1(a4d4f096e976d4993123de0c2505382f878ea42a) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-17176.ic4", 0x000000, 0x040000, CRC(4211745d) SHA1(710f7dab436bf0551b95786efc5ea4303c0fd5ec) )
ROM_END

ROM_START( sonicfgtj ) /* SegaSonic Cosmo Fighter (c)1993 Sega - 834-10082 930719-1755T (EMP5032 labeled 317-0140) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-16001.ic32", 0x000000, 0x040000, CRC(8ed1dc11) SHA1(cd1cb6066c2ff159bec88802bc4b7ca7fff2ed71) )
	ROM_LOAD16_BYTE( "epr-16000.ic31", 0x000001, 0x040000, CRC(1440caec) SHA1(9e50c28544d6c42cdc7d3ae0f321670fed68fedb) )
	ROM_LOAD16_BYTE( "epr-16003.ic34", 0x100000, 0x040000, CRC(8933e91c) SHA1(5dc7451874f97e0e5d0c666800c26907b9abf5f5) )
	ROM_LOAD16_BYTE( "epr-16002.ic33", 0x100001, 0x040000, CRC(0ae979cd) SHA1(a4d4f096e976d4993123de0c2505382f878ea42a) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-16004.ic4", 0x000000, 0x040000, CRC(e87e8433) SHA1(b2b7945c7660ab2383049af5a2434768544a4ea8) )
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

ROM_START( ribbitj ) /* Ribbit  (c)1991 Sega */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-13836.ic32", 0x000000, 0x040000, CRC(21f222e2) SHA1(1e4b640833d3ec428bd50d89dc93a670a17a4451) )
	ROM_LOAD16_BYTE( "epr-13835.ic31", 0x000001, 0x040000, CRC(1c4b291a) SHA1(61b2e7721aefb97bbd38b3ca9275d7f345f3d0a6) )
	ROM_COPY( "maincpu", 0x000000, 0x080000, 0x080000 )
	ROM_LOAD16_BYTE( "epr-13838.ic34", 0x100000, 0x080000, CRC(a5d62ac3) SHA1(8d83a7bc4017e125ef4231278f766b2368d5fc1f) )
	ROM_LOAD16_BYTE( "epr-13837.ic33", 0x100001, 0x080000, CRC(434de159) SHA1(cf2973131cabf2bc0ebb2bfe9f804ad3d7d0a733) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "epr-13834.ic4", 0x000000, 0x020000, CRC(ab0c1833) SHA1(f864e12ecf6c0524da20fc66747a4fa4280e67e9) )
ROM_END


ROM_START( tantr ) /* Tant-R (Puzzle & Action)  (c)1992 Sega - 834-9664 TANT-R (EMP5032 labeled 317-0211) */
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


ROM_START( tantrbl4 ) // extremely similar to tantrbl, but in this one the card dispenser related text hasn't been removed
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a.bin", 0x000000, 0x080000, CRC(25cafec2) SHA1(872ce10c080c8bddc2eaf1a48be7b43eeae32f28) )
	ROM_LOAD16_BYTE( "b.bin", 0x000001, 0x080000, CRC(8cf5ffd5) SHA1(21bcde46d3e075a93725292a344c3d9784ae5178) )
	ROM_LOAD16_BYTE( "d.bin", 0x100000, 0x080000, CRC(17b80202) SHA1(f47bf2aa0c5972647438619b8453c7dede5c422f) )
	ROM_LOAD16_BYTE( "c.bin", 0x100001, 0x080000, CRC(36a88bd4) SHA1(cc7f6a947d1b79bb86957c43035b53d6d2bcfa28) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "2.bin", 0x000000, 0x020000, CRC(72918c58) SHA1(cb42363b163727a887a0b762519c72dcdf0a6460) )
	ROM_LOAD( "1.bin", 0x020000, 0x020000, CRC(4e85b2a3) SHA1(3f92fb931d315c5a2d6c54b3204718574928cb7b) )
ROM_END


ROM_START( ooparts ) // hack: modified to be playable with digital joystick
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ooparts.ic32", 0x000000, 0x080000, CRC(8dcf2940) SHA1(f72630e8a26e7f2089da56878a1599268c355246) )
	ROM_LOAD16_BYTE( "ooparts.ic31", 0x000001, 0x080000, CRC(35381899) SHA1(524f6e1b1292542079589275e20f45c2eb68605c) )
	ROM_LOAD16_BYTE( "ooparts.ic34", 0x100000, 0x080000, CRC(7192ac29) SHA1(d3028a9bbb7faa733285cf7e47fd840ec0d0bf69) )
	ROM_LOAD16_BYTE( "ooparts.ic33", 0x100001, 0x080000, CRC(42755dc2) SHA1(cd0aa79418b922266c5d41bf24b9136f9f105dc5) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-13655.ic4", 0x000000, 0x040000, CRC(e09961f6) SHA1(e109b5f41502b765d191f22e3bbcff97d6defaa1) )
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


ROM_START( puyoj ) /* Puyo Puyo (Rev B)  (c)1992 Sega / Compile - 834-9029 (EMP5032 labeled 317-0203) */
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


ROM_START( puyoja ) /* Puyo Puyo (Rev A)  (c)1992 Sega / Compile - 834-9029 (EMP5032 labeled 317-0203) */
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


ROM_START( ichir ) /* Ichidant-R (Puzzle & Action 2)  (c)1994 Sega (World) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pa2_32.bin",     0x000000, 0x080000, CRC(7ba0c025) SHA1(855e9bb2a20c6f51b26381233c57c26aa96ad1f6) )
	ROM_LOAD16_BYTE( "pa2_31.bin",     0x000001, 0x080000, CRC(5f86e5cc) SHA1(44e201de00dfbf7c66d0e0d40d17b162c6f0625b) )
	ROM_LOAD16_BYTE( "epr-16888.ic34", 0x100000, 0x080000, CRC(85d73722) SHA1(7ebe81b4d6c89f87f60200a3a8cddb07d581adef) )
	ROM_LOAD16_BYTE( "epr-16887.ic33", 0x100001, 0x080000, CRC(bc3bbf25) SHA1(e760ad400bc183b38e9787d88c8ac084fbe2ae21) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "pa2_02.bin", 0x000000, 0x080000, CRC(fc7b0da5) SHA1(46770aa7e19b4f8a183be3f433c48ad677b552b1) )
ROM_END


ROM_START( ichirbl ) // bootleg of the World version, only difference is the protection checks are NOPed out
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.f11", 0x000000, 0x080000, CRC(b8201c2e) SHA1(95ac035ffd5948745324d855ecffdb7d2ff9f0fb) )
	ROM_LOAD16_BYTE( "1.f10", 0x000001, 0x080000, CRC(af0dd811) SHA1(75b1f51ad93f99933037ffff4106c92f9c2393e6) )
	ROM_LOAD16_BYTE( "4.f8",  0x100000, 0x080000, CRC(85d73722) SHA1(7ebe81b4d6c89f87f60200a3a8cddb07d581adef) )
	ROM_LOAD16_BYTE( "3.f9",  0x100001, 0x080000, CRC(bc3bbf25) SHA1(e760ad400bc183b38e9787d88c8ac084fbe2ae21) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "2.e3", 0x000000, 0x080000, CRC(fc7b0da5) SHA1(46770aa7e19b4f8a183be3f433c48ad677b552b1) )
ROM_END


ROM_START( ichirk ) /* Ichidant-R (Puzzle & Action 2)  (c)1994 Sega (Korea) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	/* Again the part numbers are quite strange for the Korean verison */
	ROM_LOAD16_BYTE( "epr_ichi.32", 0x000000, 0x080000, CRC(804dea11) SHA1(40bf8cbd40969a5880df10914252b7f64d5ce8e9) )
	ROM_LOAD16_BYTE( "epr_ichi.31", 0x000001, 0x080000, CRC(92452353) SHA1(d2e1da5b139965611cd8d707d23396b5d4c07d12) )
	ROM_LOAD16_BYTE( "epr-16888.ic34", 0x100000, 0x080000, CRC(85d73722) SHA1(7ebe81b4d6c89f87f60200a3a8cddb07d581adef) )  // m17235a.34
	ROM_LOAD16_BYTE( "epr-16887.ic33", 0x100001, 0x080000, CRC(bc3bbf25) SHA1(e760ad400bc183b38e9787d88c8ac084fbe2ae21) )  // m17220a.33

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "pa2_02.bin", 0x000000, 0x080000, CRC(fc7b0da5) SHA1(46770aa7e19b4f8a183be3f433c48ad677b552b1) ) // m17220a.4
ROM_END


ROM_START( ichirj ) /* Ichidant-R (Puzzle & Action 2)  (c)1994 Sega (Japan) - 834-10935 ICHIDANT-R */
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


ROM_START( stkclmnsj ) /* Stack Columns  (c)1994 Sega - 834-10853 (EMP5032 labeled 317-0219) */
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


ROM_START( potopoto ) /* Poto Poto  (c)1994 Sega - 834-10778 (EMP5032 labeled 317-0218) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-16662a.ic32", 0x000000, 0x040000, CRC(bbd305d6) SHA1(1a4f4869fefac188c69bc67df0b625e43a0c3f1f) )
	ROM_LOAD16_BYTE( "epr-16661a.ic31", 0x000001, 0x040000, CRC(5a7d14f4) SHA1(a615b5f481256366db7b1c6302a8dcb69708102b) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "epr-16660.ic4", 0x000000, 0x040000, CRC(8251c61c) SHA1(03eef3aa0bdde2c1d93128648f54fd69278d85dd) )
ROM_END


ROM_START( zunkyou ) /* Zunzunkyou no Yabou  (c)1994 Sega - 834-10857 (EMP5032 labeled 317-0221) */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-16812.ic32", 0x000000, 0x080000, CRC(eb088fb0) SHA1(69089a3516ad50f35e81971ef3c33eb3f5d52374) )
	ROM_LOAD16_BYTE( "epr-16811.ic31", 0x000001, 0x080000, CRC(9ac7035b) SHA1(1803ffbadc1213e04646d483e27da1591e22cd06) )
	ROM_LOAD16_BYTE( "epr-16814.ic34", 0x100000, 0x080000, CRC(821b3b77) SHA1(c45c7393a792ce8306a52f83f8ed8f6b0d7c11e9) )
	ROM_LOAD16_BYTE( "epr-16813.ic33", 0x100001, 0x080000, CRC(3cba9e8f) SHA1(208819bc1a205eaab089542afc7a59f69ce5bb81) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "epr-16810.ic4", 0x000000, 0x080000, CRC(d542f0fe) SHA1(23ea50110dfe1cd9f286a535d15e0c3bcba73b00) )
ROM_END


ROM_START( headonch ) // hack: protection routine was removed
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "headonch.ic32", 0x000000, 0x080000, CRC(091cf538) SHA1(04673678f543743b395edea39ad4ee6177436dc0) )
	ROM_LOAD16_BYTE( "headonch.ic31", 0x000001, 0x080000, CRC(91f3b5f1) SHA1(15cbe7a172dde7de7b73f0c9eeddfee41e8d1f80) )
	ROM_LOAD16_BYTE( "headonch.ic34", 0x100000, 0x080000, CRC(d8dc6323) SHA1(e7e891324764641691dcb63e5222f2ed9207fb96) )
	ROM_LOAD16_BYTE( "headonch.ic33", 0x100001, 0x080000, CRC(3268e38b) SHA1(10ded2be01465014ca9e6c64ffab1190ec985359) )

	ROM_REGION( 0x040000, "upd", 0 )
	ROM_LOAD( "headonch.ic4", 0x000000, 0x040000, CRC(90af7301) SHA1(227227cb5d0df6612bac7b4c94b99e2287686ccd) )
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


ROM_START( pclubjv3 ) /* Print Club vol.3 (c)1996 Atlus */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ic32-p3jsp.ic32", 0x000000, 0x080000, CRC(cb5b6221) SHA1(43c1352bfcaa4012019ae58742a5d09c3c4e5a8f) )
	ROM_LOAD16_BYTE( "ic31-p3jsp.ic31", 0x000001, 0x080000, CRC(cdec8418) SHA1(7b5d6316182ec89230984b617ce0404ea0f1c331) )
	ROM_LOAD16_BYTE( "ic34-p3jsp.ic34", 0x100000, 0x080000, CRC(c33f71a3) SHA1(21f9de9db4504307d5290c11dc86bfed9ee23ef0) )
	ROM_LOAD16_BYTE( "ic33-p3jsp.ic33", 0x100001, 0x080000, CRC(c8da13e4) SHA1(38434f628b43549a234cf612f8a22993c76c20f0) )

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

ROM_START( pclub ) /* english version based on v2 */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-ic32.32", 0x000000, 0x080000, CRC(3fe9bda7) SHA1(2d3bf0f10c8b9a07a263365c5d81067c91974fe1) )
	ROM_LOAD16_BYTE( "epr-ic31.31", 0x000001, 0x080000, CRC(90f994d0) SHA1(687d537481bfc05a5809161a8f1c686aa157cb8f) )
	ROM_LOAD16_BYTE( "epr-ic34.34", 0x100000, 0x080000, CRC(4d1ebb55) SHA1(f403990ff41418d14bae645cffd5c042ce13312c) )
	ROM_LOAD16_BYTE( "epr-ic33.33", 0x100001, 0x080000, CRC(bdfdc797) SHA1(aec561a591ca1387b7d38ffe48a2c179a39c1b06) )

	ROM_REGION( 0x080000, "upd", 0 )
	ROM_LOAD( "epr-ic4.4", 0x000000, 0x080000, CRC(84eed1c4) SHA1(271b199250f9a7f6ba5d3bf09e187417b7c2f88e) )
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

int segac2_state::prot_func_dummy(int in)
{
	return 0x0;
}

/* 317-0149 */
int segac2_state::prot_func_columns(int in)
{
	int const b0 = BIT( in,2) ^ ((BIT(~in,0) & BIT( in,7)) | (BIT( in,4) & BIT( in,6)));
	int const b1 = BIT(~in,0) ^ (BIT( in,2) | (BIT( in,5) & BIT(~in,6) & BIT( in,7)));
	int const b2 = BIT( in,3) ^ ((BIT( in,0) & BIT( in,1)) | (BIT( in,4) & BIT( in,6)));
	int const b3 = BIT( in,1) ^ ((BIT( in,0) & BIT( in,1)) | (BIT( in,4) & BIT( in,5)) | (BIT(~in,6) & BIT( in,7)));   // 1 repeated

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0160 */
int segac2_state::prot_func_columns2(int in)
{
	int const b0 =  BIT( in,2) ^ (BIT( in,1) | (BIT( in,4) & BIT( in,5)));
	int const b1 = (BIT( in,0) & BIT( in,3) & BIT( in,4)) ^ (BIT( in,6) | (BIT( in,5) & BIT( in,7)));
	int const b2 = (BIT( in,3) & BIT(~in,2) & BIT( in,4)) ^ (BIT( in,5) | (BIT( in,0) & BIT( in,1)) | (BIT( in,4) & BIT( in,6))); // 4 repeated
	int const b3 = (BIT( in,1) & BIT( in,0) & BIT( in,2)) ^ ((BIT( in,4) & BIT(~in,6)) | (BIT( in,6) & BIT( in,7)));   // 6 repeated

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0172 */
int segac2_state::prot_func_tfrceac(int in)
{
	int const b0 = BIT(~in,2) ^ ((BIT( in,0) & BIT(~in,7)) | (BIT( in,3) & BIT( in,4)));
	int const b1 = (BIT( in,4) & BIT(~in,5) & BIT( in,7)) ^ ((BIT(~in,0) | BIT(~in,3)) & (BIT(~in,6) | BIT(~in,7)));   // not in the form x1 XOR (x2 OR x3 OR x4)
	int const b2 = BIT( in,2) ^ ((BIT( in,4) & BIT(~in,5) & BIT( in,7)) | (BIT(~in,1) & BIT( in,6)));
	int const b3 = BIT( in,0) ^ ((BIT( in,1) & BIT( in,4) & BIT( in,6)) | (BIT( in,1) & BIT( in,4) & BIT( in,7))); // 1,4 repeated

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0173 */
int segac2_state::prot_func_borench(int in)
{
	int const b0 = (BIT( in,1) & BIT( in,2) & BIT( in,3) & BIT( in,7))   ^ (BIT( in,5) | (BIT(~in,0) & BIT(~in,4)));
	int const b1 = (BIT(~in,2) & BIT( in,3) & BIT( in,5))                ^ (BIT( in,1) | (BIT( in,0) & BIT(~in,4)));
	int const b2 = (BIT( in,1) & BIT(~in,4) & BIT(~in,6))                ^ (BIT( in,2) | BIT( in,3) | (BIT( in,5) & BIT( in,7)));
	int const b3 = (BIT(~in,0) & BIT( in,5) & (BIT( in,6) | BIT( in,7))) ^ (BIT( in,1) | (BIT( in,3) & BIT( in,4)));   // not in the form x1 XOR (x2 OR x3 OR x4)

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0178 */
int segac2_state::prot_func_ribbit(int in)
{
	int const b0 = (BIT( in,0) & BIT( in,4)) ^ ((BIT( in,1) & BIT( in,2)) | BIT( in,3) | BIT(~in,5));
	int const b1 = (BIT( in,1) & BIT( in,5)) ^ ((BIT( in,2) & BIT( in,3)) | BIT( in,0) | BIT(~in,6));
	int const b2 = (BIT( in,2) & BIT( in,7)) ^ ((BIT( in,3) & BIT( in,0)) | BIT(~in,1) | BIT( in,7));
	int const b3 = (BIT( in,3) & BIT( in,6)) ^ ((BIT( in,0) & BIT( in,1)) | BIT(~in,2) | BIT( in,4));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0193 */
int segac2_state::prot_func_twinsqua(int in)
{
	int const b0 = (BIT( in,2) & BIT(~in,5)) ^ (BIT( in,3) | BIT(~in,4));
	int const b1 = (BIT( in,0) & BIT(~in,2) & BIT( in,4)) ^ (BIT(~in,0) | BIT(~in,4) | BIT(~in,6)); // 0,4 repeated
	int const b2 = (BIT( in,3) & BIT(~in,5)) ^ (BIT( in,4) & BIT( in,7));
	int const b3 =  BIT( in,1) ^ ((BIT(~in,3) & BIT(~in,6)) | (BIT( in,4) & BIT(~in,6)) | (BIT(~in,1) & BIT( in,3) & BIT(~in,4)));    // 1,3,4,6 repeated

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0203 */
int segac2_state::prot_func_puyo(int in)
{
	int const b0 = (BIT(~in,3) & BIT( in,7)) ^ ((BIT(~in,0) & BIT(~in,1)) | (BIT(~in,1) & BIT(~in,4))); // 1 repeated
	int const b1 = (BIT( in,3) & BIT( in,5)) ^ (BIT(~in,2) | BIT( in,4) | BIT( in,6));
	int const b2 = (BIT(~in,2) & BIT(~in,5)) ^ (BIT( in,1) | BIT(~in,3) | BIT(~in,6));
	int const b3 =  BIT( in,1)               ^ ((BIT( in,0) & BIT( in,3) & BIT( in,7)) | BIT( in,4));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0211 */
int segac2_state::prot_func_tantr(int in)
{
	int const b0 = (BIT( in,0) & BIT( in,4)) ^ ( BIT( in,5) | BIT(~in,6)  | (BIT(~in,3) & BIT( in,7)));
	int const b1 = (BIT( in,2) & BIT( in,6)) ^ ((BIT( in,1) & BIT( in,5)) | (BIT( in,3) & BIT( in,4)));
	int const b2 = (BIT(~in,0) & BIT( in,2)) ^ ( BIT( in,4) | BIT( in,7)  | (BIT( in,1) & BIT(~in,5)));
	int const b3 = (BIT(~in,2) & BIT( in,7)) ^ ( BIT(~in,0) | BIT( in,1)  | (BIT( in,3) & BIT( in,6)));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-???? */
int segac2_state::prot_func_tantrkor(int in)
{
	int const b0 = (BIT(~in,1) & BIT(~in,7)) ^ (BIT(~in,2) & BIT(~in,4));
	int const b1 = (BIT( in,2) & BIT( in,6)) ^ (BIT( in,0) & BIT( in,1));
	int const b2 = (BIT(~in,3) & BIT(~in,6)) ^ (BIT( in,1) | BIT(~in,4));
	int const b3 = (BIT(~in,0) & BIT(~in,2)) ^ (BIT( in,5) & BIT(~in,6));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0218 */
int segac2_state::prot_func_potopoto(int in)
{
	int const b0 = (BIT(~in,2) & BIT(~in,4)) ^ (BIT(~in,1) & BIT( in,3));
	int const b1 = (BIT( in,0) & BIT( in,5)) ^ (BIT( in,2) | BIT(~in,7));
	int const b2 = (BIT( in,0) & BIT( in,6)) ^ (BIT(~in,1) & BIT( in,7));
	int const b3 = (BIT( in,0) & BIT(~in,7)) ^ (BIT(~in,1) & BIT(~in,6));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0219 */
int segac2_state::prot_func_stkclmnj(int in)
{
	int const b0 = (BIT( in,1) & BIT( in,4)) ^ (BIT( in,5) & BIT( in,2));
	int const b1 = (BIT(~in,2) & BIT( in,6)) ^ (BIT(~in,5) & BIT( in,7));
	int const b2 = (BIT(~in,3) & BIT( in,6)) ^ (BIT(~in,5) & BIT(~in,1));
	int const b3 = (BIT(~in,3) & BIT( in,5)) ^ (BIT(~in,6) | BIT(~in,7));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0223 */
int segac2_state::prot_func_stkclmns(int in)
{
	int const b0 = (BIT( in,2) & BIT( in,4)) ^ (BIT( in,1) | BIT(~in,3));
	int const b1 = (BIT( in,0) & BIT( in,5)) ^ (BIT( in,2) & BIT( in,7));
	int const b2 = (BIT( in,0) & BIT(~in,6)) ^ (BIT( in,1) & BIT(~in,7));
	int const b3 = (BIT( in,0) & BIT(~in,7)) ^ (BIT(~in,1) & BIT( in,6));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0224 */
int segac2_state::prot_func_ichirj(int in)
{
	int const b0 = (BIT( in,2) & BIT( in,4)) ^ (BIT(~in,5) & BIT(~in,2));
	int const b1 = (BIT( in,2) & BIT(~in,6)) ^ (BIT( in,5) & BIT( in,7));
	int const b2 = (BIT(~in,3) & BIT( in,6)) ^ (BIT(~in,5) & BIT(~in,1));
	int const b3 = (BIT(~in,1) & BIT( in,5)) ^ (BIT(~in,5) & BIT( in,7));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-???? */
int segac2_state::prot_func_ichir(int in)
{
	int const b0 = (BIT(~in,2) & BIT( in,4)) ^ (BIT( in,5) & BIT(~in,2));
	int const b1 = (BIT( in,1) & BIT( in,6)) ^ (BIT( in,5) | BIT( in,7));
	int const b2 = (BIT(~in,3) & BIT( in,6)) ^ (BIT(~in,5) & BIT(~in,3));
	int const b3 = (BIT( in,0) & BIT(~in,5)) ^ (BIT( in,5) & BIT( in,7));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-???? */
int segac2_state::prot_func_ichirk(int in)
{
	int const b0 = (BIT(~in,2) & BIT( in,4)) ^ (BIT( in,5) & BIT(~in,1));
	int const b1 = (BIT( in,0) & BIT( in,6)) ^ (BIT( in,5) & BIT( in,4));
	int const b2 = (BIT(~in,1) & BIT(~in,6)) ^ (BIT(~in,5) & BIT( in,3));
	int const b3 = (BIT( in,1) & BIT( in,5)) ^ (BIT( in,6) & BIT( in,7));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

/* 317-0228 */
int segac2_state::prot_func_puyopuy2(int in)
{
	int const b0 = (BIT(~in,0) & BIT(~in,7)) ^ (BIT( in,1) | BIT(~in,4) | BIT(~in,6));
	int const b1 = (BIT( in,0) & BIT(~in,6)) ^ (BIT( in,3) & BIT( in,5));
	int const b2 = (BIT(~in,4) & BIT(~in,7)) ^ (BIT( in,0) | BIT(~in,6));
	int const b3 = (BIT(~in,1) & BIT( in,4)) ^ (BIT( in,2) & BIT(~in,3));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

int segac2_state::prot_func_zunkyou(int in)
{
	int const b0 = (BIT(~in,1) & BIT( in,6)) ^ (BIT(~in,5) & BIT( in,7));
	int const b1 = (BIT( in,0) & BIT(~in,5)) ^ (BIT(~in,3) | BIT( in,4));
	int const b2 = (BIT( in,2) & BIT(~in,3)) ^ (BIT( in,4) & BIT(~in,5));
	int const b3 = (BIT( in,0) & BIT(~in,4)) ^ (BIT(~in,2) & BIT(~in,6));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

int pclub_state::prot_func_pclub(int in)
{
	return 0xf;
}

int pclub_state::prot_func_pclubjv2(int in)
{
	int const b0 = (BIT( in,3) & BIT(~in,4)) ^ ((BIT(~in,1) & BIT(~in,7)) | BIT( in,6));
	int const b1 = (BIT( in,0) & BIT( in,5)) ^  (BIT( in,2) & BIT(~in,6));
	int const b2 = (BIT(~in,1) & BIT( in,6)) ^  (BIT( in,3) | BIT(~in,5)  | BIT(~in,1)); // 1 repeated
	int const b3 = (BIT(~in,2) & BIT(~in,7)) ^  (BIT(~in,0) | BIT(~in,4));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

int pclub_state::prot_func_pclubjv3(int in)
{
	// TODO: Determine correctly. This is just copied from V2 as a placeholder.
	int const b0 = (BIT( in,3) & BIT(~in,4)) ^ ((BIT(~in,1) & BIT(~in,7)) | BIT( in,6));
	int const b1 = (BIT( in,0) & BIT( in,5)) ^  (BIT( in,2) & BIT(~in,6));
	int const b2 = (BIT(~in,1) & BIT( in,6)) ^  (BIT( in,3) | BIT(~in,5)  | BIT(~in,1)); // 1 repeated
	int const b3 = (BIT(~in,2) & BIT(~in,7)) ^  (BIT(~in,0) | BIT(~in,4));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

int pclub_state::prot_func_pclubjv4(int in)
{
	int const b0 = (BIT(~in,2) & BIT( in,4)) ^ (BIT( in,1) & BIT(~in,6) & BIT(~in,3));
	int const b1 = (BIT(~in,3) & BIT(~in,4)) ^ (BIT( in,0) & BIT( in,5) & BIT(~in,6));
	int const b2 =  BIT(~in,0)               ^ (BIT( in,3) & BIT( in,4));
	int const b3 = (BIT(~in,1) & BIT( in,7)) ^ (BIT( in,5) & BIT(~in,7)); // 7 repeated

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

int pclub_state::prot_func_pclubjv5(int in)
{
	int const b0 = (BIT(~in,1) & BIT( in,5)) ^ (BIT(~in,2) & BIT(~in,6));
	int const b1 = (BIT(~in,0) & BIT( in,4)) ^ (BIT(~in,3) & BIT(~in,7));
	int const b2 = (BIT(~in,3) & BIT( in,7)) ^ (BIT(~in,0) | BIT(~in,4));
	int const b3 = (BIT(~in,2) & BIT( in,6)) ^ (BIT(~in,1) & BIT(~in,5));

	return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}


void segac2_state::init_noprot()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_dummy)));
}

void segac2_state::init_columns()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_columns)));
}

void segac2_state::init_columns2()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_columns2)));
}

void segac2_state::init_tfrceac()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_tfrceac)));
}

void segac2_state::init_tfrceacb()
{
	init_noprot();

	/* disable the palette bank switching from the protection chip */
	m_maincpu->space(AS_PROGRAM).nop_write(0x800000, 0x800001);
}

void segac2_state::init_borench()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_borench)));
}

void segac2_state::init_twinsqua()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_twinsqua)));
}

void segac2_state::init_ribbit()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_ribbit)));
}

void segac2_state::init_puyo()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_puyo)));
}

void segac2_state::init_tantr()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_tantr)));
}

void segac2_state::init_tantrkor()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_tantrkor)));
}

void segac2_state::init_potopoto()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_potopoto)));
}

void segac2_state::init_stkclmns()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_stkclmns)));
}

void segac2_state::init_stkclmnj()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_stkclmnj)));
}

void segac2_state::init_ichir()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_ichir)));
}

void segac2_state::init_ichirk()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_ichirk)));
}

void segac2_state::init_ichirj()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_ichirj)));
}

uint16_t segac2_state::ichirjbl_prot_r()
{
	return 0x00f5;
}

void segac2_state::init_ichirjbl()
{
	init_noprot();

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x840108, 0x840109, read16smo_delegate(*this, FUNC(segac2_state::ichirjbl_prot_r)));
}

void segac2_state::init_puyopuy2()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_puyopuy2)));
}

void segac2_state::init_zunkyou()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(segac2_state::prot_func_zunkyou)));
}

void pclub_state::init_pclub()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x880120, 0x880121, read16smo_delegate(*this, FUNC(pclub_state::printer_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x880124, 0x880125, read16smo_delegate(*this, FUNC(pclub_state::printer_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x880124, 0x880125, write16smo_delegate(*this, FUNC(pclub_state::print_club_camera_w)));
}

void pclub_state::init_pclubj()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(pclub_state::prot_func_pclub)));
	init_pclub();
}

void pclub_state::init_pclubjv2()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(pclub_state::prot_func_pclubjv2)));
	init_pclub();
}

void pclub_state::init_pclubjv3()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(pclub_state::prot_func_pclubjv3)));
	init_pclub();
}

void pclub_state::init_pclubjv4()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(pclub_state::prot_func_pclubjv4)));
	init_pclub();
}

void pclub_state::init_pclubjv5()
{
	set_prot_func(segac2_prot_delegate(*this, FUNC(pclub_state::prot_func_pclubjv5)));
	init_pclub();
}


} // Anonymous namespace

/******************************************************************************
    Game Drivers
*******************************************************************************

    These cover all the above games.

    Dates are all verified correct from Ingame display, some of the Titles
    such as Ichidant-R, Tant-R might be slightly incorrect as I've seen the
    games referred to by other names such as Ichidant-R, Tanto-R, Tanto Arle
    etc.

    bloxeedc is set as as clone of bloxeed as it is the same game but running
    on a different piece of hardware.  The parent 'bloxeed' is a system18 game

******************************************************************************/

//    YEAR, NAME,       PARENT,   MACHINE,    INPUT,    CLASS,           INIT,          MONITOR,COMPANY,FULLNAME,FLAGS
/* System C Games */
GAME( 1989, bloxeedc,   bloxeed,  segac,      bloxeedc, segac2_state,    init_noprot,   ROT0,   "Sega / Elorg", "Bloxeed (World, C System)", 0 )
GAME( 1989, bloxeedu,   bloxeed,  segac,      bloxeedu, segac2_state,    init_noprot,   ROT0,   "Sega / Elorg", "Bloxeed (US, C System, Rev A)", 0 )

GAME( 1990, columns,    0,        segac,      columns,  segac2_state,    init_columns,  ROT0,   "Sega", "Columns (World)", 0 )
GAME( 1990, columnsu,   columns,  segac,      columnsu, segac2_state,    init_columns,  ROT0,   "Sega", "Columns (US, cocktail, Rev A)", 0 ) // has cocktail mode dsw
GAME( 1990, columnsj,   columns,  segac,      columns,  segac2_state,    init_columns,  ROT0,   "Sega", "Columns (Japan)", 0 )

GAME( 1990, columns2,   0,        segac,      columns2, segac2_state,    init_columns2, ROT0,   "Sega", "Columns II: The Voyage Through Time (World)", 0 )
GAME( 1990, column2j,   columns2, segac,      columns2, segac2_state,    init_columns2, ROT0,   "Sega", "Columns II: The Voyage Through Time (Japan)", 0 )

/* System C-2 Games */
GAME( 1990, tfrceac,    0,        segac2,     tfrceac,  segac2_state,    init_tfrceac,  ROT0,   "Technosoft / Sega", "Thunder Force AC", 0 )
GAME( 1990, tfrceacj,   tfrceac,  segac2,     tfrceac,  segac2_state,    init_tfrceac,  ROT0,   "Technosoft / Sega", "Thunder Force AC (Japan)", 0 )
GAME( 1990, tfrceacb,   tfrceac,  segac2,     tfrceac,  segac2_state,    init_tfrceacb, ROT0,   "bootleg",           "Thunder Force AC (bootleg)", 0 )
GAME( 1990, tfrceacjpb, tfrceac,  tfrceacjpb, tfrceac,  segac2_state,    init_tfrceac,  ROT0,   "Technosoft / Sega", "Thunder Force AC (Japan, prototype, bootleg)", 0 )

GAME( 1990, borench,    0,        segac2,     borench,  segac2_state,    init_borench,  ROT0,   "Sega", "Borench (set 1)", 0 )
GAME( 1990, borencha,   borench,  segac2,     borench,  segac2_state,    init_borench,  ROT0,   "Sega", "Borench (set 2)", 0 )
GAME( 1990, borenchj,   borench,  segac2,     borench,  segac2_state,    init_borench,  ROT0,   "Sega", "Borench (Japan)", 0 )

GAME( 1991, ribbit,     0,        ribbit,     ribbit,   segac2_state,    init_ribbit,   ROT0,   "Sega", "Ribbit!", 0 )
GAME( 1991, ribbitj,    ribbit,   ribbit,     ribbitj,  segac2_state,    init_ribbit,   ROT0,   "Sega", "Ribbit! (Japan)", 0 )

GAME( 1991, twinsqua,   0,        c2m50dass,  twinsqua, segac2_state,    init_twinsqua, ROT0,   "Sega", "Twin Squash", 0 )

GAME( 1991, soniccar,   0,        segac2,     soniccar, segac2_state,    init_noprot,   ROT0,   "Sega", "Waku Waku Sonic Patrol Car", 0 )

GAME( 1992, ssonicbr,   0,        segac2,     ssonicbr, segac2_state,    init_noprot,   ROT0,   "hack", "SegaSonic Bros. (prototype, hack)", 0 )

GAME( 1992, ooparts,    0,        segac2,     ooparts,  segac2_state,    init_noprot,   ROT270, "hack", "OOPArts (prototype, joystick hack)", 0 )

GAME( 1992, puyo,       0,        c2m50dass,  puyo,     segac2_state,    init_puyo,     ROT0,   "Compile / Sega", "Puyo Puyo (World)", 0 )
GAME( 1992, puyobl,     puyo,     c2m50dass,  puyo,     segac2_state,    init_puyo,     ROT0,   "bootleg",        "Puyo Puyo (World, bootleg)", 0 )
GAME( 1992, puyoj,      puyo,     c2m50dass,  puyo,     segac2_state,    init_puyo,     ROT0,   "Compile / Sega", "Puyo Puyo (Japan, Rev B)", 0 )
GAME( 1992, puyoja,     puyo,     c2m50dass,  puyo,     segac2_state,    init_puyo,     ROT0,   "Compile / Sega", "Puyo Puyo (Japan, Rev A)", 0 )

GAME( 1992, tantr,      0,        segac2,     ichir,    segac2_state,    init_tantr,    ROT0,   "Sega",    "Puzzle & Action: Tant-R (Japan)", 0 )
GAME( 1993, tantrkor,   tantr,    segac2,     ichir,    segac2_state,    init_tantrkor, ROT0,   "Sega",    "Puzzle & Action: Tant-R (Korea)", 0 )
GAME( 1992, tantrbl,    tantr,    segac2,     ichir,    segac2_state,    init_noprot,   ROT0,   "bootleg", "Puzzle & Action: Tant-R (Japan) (bootleg set 1)", 0 )
GAME( 1992, tantrbl4,   tantr,    segac2,     ichir,    segac2_state,    init_noprot,   ROT0,   "bootleg", "Puzzle & Action: Tant-R (Japan) (bootleg set 4)", 0 )
GAME( 1994, tantrbl2,   tantr,    segac,      ichir,    segac2_state,    init_tantr,    ROT0,   "bootleg", "Puzzle & Action: Tant-R (Japan) (bootleg set 2)", 0 ) // Common bootleg in Europe, C board, no samples
GAME( 1994, tantrbl3,   tantr,    segac,      ichir,    segac2_state,    init_tantr,    ROT0,   "bootleg", "Puzzle & Action: Tant-R (Japan) (bootleg set 3)", 0 ) // Common bootleg in Europe, C board, no samples

GAME( 1992, wwanpanm,   0,        segac2,     wwmarine, wwmarine_state,  init_noprot,   ROT0,   "Sega", "Waku Waku Anpanman (Rev A)", 0 )
GAME( 1992, wwmarine,   0,        segac2,     wwmarine, wwmarine_state,  init_noprot,   ROT0,   "Sega", "Waku Waku Marine", 0 )

// not really sure how this should hook up, things like the 'sold out' flags could be mechanical sensors, or from another MCU / CPU board in the actual popcorn part of the machine?
GAME( 1992, anpanman,   0,        segac2,     anpanman, segac2_state,    init_noprot,   ROT0,   "Sega", "Soreike! Anpanman Popcorn Koujou (Rev B)", MACHINE_MECHANICAL ) // 'Mechanical' part isn't emulated
GAME( 1992, anpanmana,  anpanman, segac2,     anpanman, segac2_state,    init_noprot,   ROT0,   "Sega", "Soreike! Anpanman Popcorn Koujou (Rev A)", MACHINE_MECHANICAL ) // 'Mechanical' part isn't emulated
GAME( 1993, sonicpop,   0,        segac2,     sonicpop, segac2_state,    init_noprot,   ROT0,   "Sega", "SegaSonic Popcorn Shop (Rev B)", MACHINE_MECHANICAL ) // region DSW for USA / Export / Japan, still speaks Japanese tho. 'Mechanical' part isn't emulated

GAME( 1993, sonicfgt,   0,        segac2,     sonicfgt, segac2_state,    init_noprot,   ROT0,   "Sega", "SegaSonic Cosmo Fighter (World)", 0 )
GAME( 1993, sonicfgtj,  sonicfgt, segac2,     sonicfgt, segac2_state,    init_noprot,   ROT0,   "Sega", "SegaSonic Cosmo Fighter (Japan)", 0 )

GAME( 1994, potopoto,   0,        c2m50dass,  potopoto, segac2_state,    init_potopoto, ROT0,   "Sega", "Poto Poto (Japan, Rev A)", 0 )

GAME( 1994, stkclmns,   0,        segac2,     stkclmns, segac2_state,    init_stkclmns, ROT0,   "Sega", "Stack Columns (World)", 0 )
GAME( 1994, stkclmnsj,  stkclmns, segac2,     stkclmns, segac2_state,    init_stkclmnj, ROT0,   "Sega", "Stack Columns (Japan)", 0 )

GAME( 1994, ichir,      0,        segac2,     ichir,    segac2_state,    init_ichir,    ROT0,   "Sega",    "Puzzle & Action: Ichidant-R (World)", 0 )
GAME( 1994, ichirbl,    ichir,    segac2,     ichir,    segac2_state,    init_noprot,   ROT0,   "bootleg", "Puzzle & Action: Ichidant-R (World) (bootleg)", 0 )
GAME( 1994, ichirk,     ichir,    segac2,     ichir,    segac2_state,    init_ichirk,   ROT0,   "Sega",    "Puzzle & Action: Ichidant-R (Korea)", 0 )
GAME( 1994, ichirj,     ichir,    segac2,     ichir,    segac2_state,    init_ichirj,   ROT0,   "Sega",    "Puzzle & Action: Ichidant-R (Japan)", 0 )
GAME( 1994, ichirjbl,   ichir,    segac,      ichir,    segac2_state,    init_ichirjbl, ROT0,   "bootleg", "Puzzle & Action: Ichidant-R (Japan) (bootleg)", 0 ) // C board, no samples

GAME( 1994, puyopuy2,   0,        segac2,     puyopuy2, segac2_state,    init_puyopuy2, ROT0,   "Compile (Sega license)", "Puyo Puyo 2 (Japan)", 0 )

GAME( 1994, zunkyou,    0,        segac2,     zunkyou,  segac2_state,    init_zunkyou,  ROT0,   "Sega", "Zunzunkyou no Yabou (Japan)", 0 )

GAME( 1994, headonch,   0,        segac2,     headonch, segac2_state,    init_noprot,   ROT0,   "hack", "Monita to Rimoko no Head On Channel (prototype, hack)", 0 )

/* Atlus Print Club 'Games' (C-2 Hardware) requires printer and camera emulation */
GAME( 1995, pclubj,     0,        pclub,      pclub,    pclub_state,     init_pclubj,   ROT0,   "Atlus", "Print Club (Japan Vol.1)", MACHINE_NOT_WORKING )

GAME( 1995, pclubjv2,   0,        pclub,      pclubjv2, pclub_state,     init_pclubjv2, ROT0,   "Atlus", "Print Club (Japan Vol.2)", MACHINE_NOT_WORKING )
GAME( 1995, pclub,      pclubjv2, pclub,      pclubjv2, pclub_state,     init_pclubj,   ROT0,   "Atlus", "Print Club (World)", MACHINE_NOT_WORKING ) // based on Japan Vol.2 but no Vol.2 subtitle

GAME( 1995, pclubjv3,   0,        pclub,      pclubjv2, pclub_state,     init_pclubjv3, ROT0,   "Atlus", "Print Club (Japan Vol.3)", MACHINE_NOT_WORKING )

GAME( 1996, pclubjv4,   0,        pclub,      pclubjv2, pclub_state,     init_pclubjv4, ROT0,   "Atlus", "Print Club (Japan Vol.4)", MACHINE_NOT_WORKING )

GAME( 1996, pclubjv5,   0,        pclub,      pclubjv2, pclub_state,     init_pclubjv5, ROT0,   "Atlus", "Print Club (Japan Vol.5)", MACHINE_NOT_WORKING )
