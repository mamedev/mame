// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Missile Command hardware

    Games supported:
        * Missile Command

    Known issues:
        * bootleg sets don't work yet

******************************************************************************************

Missile Command
Atari, 1979

PCB Layout
----------

MISSILE COMMAND
A035467-04
ATARI (C)79
|---------------------------------------------------------------------|
|                           T_5MHz   T_10MHz                          |
|    MC14584     MC14584         10MHz               RESET_SW         |
|J19                    T_HSYNC              T_R/W02         UM6502A  |
|                 T_WDOG_DIS                           T_-5V          |
|   T_BW_VID                                                       X  |
|-|                                                                X  |
  |        T_START2                        MM5290J-3               X  |
|-|J20     T_START1   T_GND     T_GND      MM5290J-3     035820-01.H1 |
|2         T_SLAM                          MM5290J-3     035821-01.JK1|
|2         T_COINR  T_COINC                MM5290J-3     035822-01.KL1|
|W  T_TEST T_COINL                                     T_GND          |
|A         T_AUD2           N82S25         MM5290J-3     035823-01.MN1|
|Y         T_AUD1             035826-01.L6 MM5290J-3               X  |
|-|           LM324   T_VSYNC              MM5290J-3   T_+12V         |
  |                   C012294B-01          MM5290J-3     035824-01.NP1|
|-|T_+5V    DSW(8)     DSW(8)                       X   035825-01.R1  |
|---------------------------------------------------------------------|
Notes:
       UM6502A     - 6502 CPU, clock input is on pin 37. This is a little strange because it
                     measures 1.17240MHz. It was assumed to be 1.25MHz [10/8]. This might be
                     caused by old components that are out of spec now, but the PCB does run
                     flawlessly, and the other clocks measure correctly so I'm not sure what's going on.
       C012294B-01 - 'Pokey' sound chip, clock 1.25MHz on pin 7 [10/8]
       035826-01   - MMI 6331 bipolar PROM
       MM5290J-3   - National Semiconductor MM5290J-3 16kx1 DRAM (=TMM416, uPD416, MK4116, TMS4116 etc)
       MC14584     - Hex Schmitt Trigger
       82S25       - Signetics 64-bit (16x4) bipolar scratch pad memory (=3101, 74S189, 7489, 9410 etc)
       LM324       - Low Power Quad Operational Amplifier
       J20         - 22-Way edge connector (for upright)
       J19         - 12-Way edge connector (additional controls for cocktail)
       T_*         - Test points
       X           - Empty location for DIP24 device (no socket)
       HSYNC       - 15.618kHz
       VSYNC       - 61.0076Hz


       Edge Connector J20 Pinout
       -------------------------
       GND                  A | 1   GND
       + 5V                 B | 2   + 5V
       + 12V                C | 3
       - 5V                 D | 4
       Audio 1 Out          E | 5   Audio 2 Out
       HSync                F | 6   VSync
       Start2 LED           H | 7   Left Fire
       Center Fire          J | 8   Right Fire
       Right Coin           K | 9   Left Coin
       Video Blue           L | 10  Video Red
       Video Green          M | 11  Left Coin Counter
       Center Coin Counter  N | 12  Right Coin Counter
       Start 1 LED          P | 13  Start Button 1
       Test Switch          R | 14  Slam Switch
       Center Coin Slot*    S | 15  Start Button 2
       Vert. Trackball Dir. T | 16  Vert. Trackball Clock
       Horiz. Trackball Clk U | 17  Horiz. Trackball Dir.
                            V | 18  Comp Sync
       NC                   W | 19  - 5V
       NC                   X | 20  + 12V
       + 5V                 Y | 21  + 5V
       GND                  Z | 22  GND


       Edge Connector J19 Pinout (Only used in cocktail version)
       -------------------------
                              A | 1
                              B | 2
                              C | 3
       Right Fire Button 2    D | 4
       Center Fire Button 2   E | 5   Left Fire Button 2
                              F | 6
       Cocktail               H | 7
                              J | 8
       Horiz. Trkball Dir. 2  K | 9   Vert. Trackball Dir. 2
       Horiz. Trkball Clock 2 L | 10  Vert. Trackball Clock 2
                              M | 11
                              N | 12

****************************************************************************

    Horizontal sync chain:

        A J/K flip flop @ D6 counts the 1H line, and cascades into a
        4-bit binary counter @ D5, which counts the 2H,4H,8H,16H lines.
        This counter cascades into a 4-bit BCD decade counter @ E5
        which counts the 32H,64H,128H,HBLANK lines. The counter system
        rolls over after counting to 320.

        Pixel clock = 5MHz
        HBLANK ends at H = 0
        HBLANK begins at H = 256
        HSYNC begins at H = 260
        HSYNC ends at H = 288
        HTOTAL = 320

    Vertical sync chain:

        The HSYNC signal clocks a 4-bit binary counter @ A4, which counts
        the 1V,2V,4V,8V lines. This counter cascades into a second 4-bit
        binary counter @ B4 which counts the 16V,32V,64V,128V lines. The
        counter system rolls over after counting to 256.

        if not flipped (V counts up):
            VBLANK ends at V = 24
            VBLANK begins at V = 0
            VSYNC begins at V = 4
            VSYNC ends at V = 8
            VTOTAL = 256

        if flipped (V counts down):
            VBLANK ends at V = 0
            VBLANK begins at V = 24
            VSYNC begins at V = 20
            VSYNC ends at V = 16
            VTOTAL = 256

    Interrupts:

        /IRQ connected to Q on flip-flop @ F7, clocked by SYNC which
        indicates an instruction opcode fetch. Input to the flip-flop
        (D) comes from a second flip-flop @ F7, which is clocked by
        /16V or 16V depending on whether or not we are flipped. Input
        to this second flip-flop is 32V.

        if not flipped (V counts up):
            clock @   0 -> 32V = 0 -> /IRQ = 0
            clock @  32 -> 32V = 1 -> /IRQ = 1
            clock @  64 -> 32V = 0 -> /IRQ = 0
            clock @  96 -> 32V = 1 -> /IRQ = 1
            clock @ 128 -> 32V = 0 -> /IRQ = 0
            clock @ 160 -> 32V = 1 -> /IRQ = 1
            clock @ 192 -> 32V = 0 -> /IRQ = 0
            clock @ 224 -> 32V = 1 -> /IRQ = 1

        if flipped (V counts down):
            clock @ 208 -> 32V = 0 -> /IRQ = 0
            clock @ 176 -> 32V = 1 -> /IRQ = 1
            clock @ 144 -> 32V = 0 -> /IRQ = 0
            clock @ 112 -> 32V = 1 -> /IRQ = 1
            clock @  80 -> 32V = 0 -> /IRQ = 0
            clock @  48 -> 32V = 1 -> /IRQ = 1
            clock @  16 -> 32V = 0 -> /IRQ = 0
            clock @ 240 -> 32V = 1 -> /IRQ = 1

****************************************************************************

    CPU Clock:
      _   _   _   _   _   _   _   _   _   _   _   _   _   _
    _| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |  1H
        ___     ___     ___     ___     ___     ___     ___
    ___|   |___|   |___|   |___|   |___|   |___|   |___|   |  2H
            _______         _______         _______
    _______|       |_______|       |_______|       |_______|  4H
    _______     ___________     ___________     ___________
           |___|           |___|           |___|              /(4H & /2H)
    _         ___             ___             ___
     |_______|   |___________|   |___________|   |_________   /Q on FF @ A7
                ___             ___             ___
    ___________|   |___________|   |___________|   |_______   Q on FF @ B8

    When V < 224,
        ___     ___     ___     ___     ___     ___     ___
    ___|   |___|   |___|   |___|   |___|   |___|   |___|   |  Sigma-X = 2H

    When V >= 224,
                ___             ___             ___
    ___________|   |___________|   |___________|   |_______   Sigma-X = Q on FF @ B8

****************************************************************************

    Modified from original schematics...

    MISSILE COMMAND
    ---------------
    HEX      R/W   D7 D6 D5 D4 D3 D2 D2 D0  function
    ---------+-----+------------------------+------------------------
    0000-01FF  R/W   D  D  D    D  D  D  D  D   512 bytes working ram

    0200-05FF  R/W   D  D  D    D  D  D  D  D   3rd color bit region
                                                of screen ram.
                                                Each bit of every odd byte is the low color
                                                bit for the bottom scanlines
                                                The schematics say that its for the bottom
                                                32 scanlines, although the code only accesses
                                                $401-$5FF for the bottom 8 scanlines...
                                                Pretty wild, huh?

    0600-063F  R/W   D  D  D    D  D  D  D  D   More working ram.

    0640-3FFF  R/W   D  D  D    D  D  D  D  D   2-color bit region of
                                                screen ram.
                                                Writes to 4 bytes each to effectively
                                                address $1900-$ffff.

    1900-FFFF  R/W   D  D                       2-color bit region of
                                                screen ram
                                                  Only accessed with
                                                   LDA ($ZZ,X) and
                                                   STA ($ZZ,X)
                                                  Those instructions take longer
                                                  than 5 cycles.

    ---------+-----+------------------------+------------------------
    4000-400F  R/W   D  D  D    D  D  D  D  D   POKEY ports.
    -----------------------------------------------------------------
    4008         R     D  D  D  D  D  D  D  D   Game Option switches
    -----------------------------------------------------------------
    4800         R     D                        Right coin
    4800         R        D                     Center coin
    4800         R           D                  Left coin
    4800         R              D               1 player start
    4800         R                 D            2 player start
    4800         R                    D         2nd player left fire(cocktail)
    4800         R                       D      2nd player center fire  "
    4800         R                          D   2nd player right fire   "
    ---------+-----+------------------------+------------------------
    4800         R                 D  D  D  D   Horiz trackball displacement
                                                        if ctrld=high.
    4800         R     D  D  D  D               Vert trackball displacement
                                                        if ctrld=high.
    ---------+-----+------------------------+------------------------
    4800         W     D                        Unused ??
    4800         W        D                     screen flip
    4800         W           D                  left coin counter
    4800         W              D               center coin counter
    4800         W                 D            right coin counter
    4800         W                    D         2 player start LED.
    4800         W                       D      1 player start LED.
    4800         W                          D   CTRLD, 0=read switches,
                                                        1= read trackball.
    ---------+-----+------------------------+------------------------
    4900         R     D                        VBLANK read
    4900         R        D                     Self test switch input.
    4900         R           D                  SLAM switch input.
    4900         R              D               Horiz trackball direction input.
    4900         R                 D            Vert trackball direction input.
    4900         R                    D         1st player left fire.
    4900         R                       D      1st player center fire.
    4900         R                          D   1st player right fire.
    ---------+-----+------------------------+------------------------
    4A00         R     D  D  D  D  D  D  D  D   Pricing Option switches.
    4B00-4B07  W                   D  D  D  D   Color RAM.
    4C00         W                              Watchdog.
    4D00         W                              Interrupt acknowledge.
    ---------+-----+------------------------+------------------------
    5000-7FFF  R       D  D  D  D  D  D  D  D   Program.
    ---------+-----+------------------------+------------------------


    MISSILE COMMAND SWITCH SETTINGS (Atari, 1980)
    ---------------------------------------------


    GAME OPTIONS:
    (8-position switch at R8)

    1   2   3   4   5   6   7   8   Meaning
    -------------------------------------------------------------------------
    Off Off                         Game starts with 7 cities
    On  On                          Game starts with 6 cities
    On  Off                         Game starts with 5 cities
    Off On                          Game starts with 4 cities
            On                      No bonus credit
            Off                     1 bonus credit for 4 successive coins
                On                  Large trak-ball input
                Off                 Mini Trak-ball input
                    On  Off Off     Bonus city every  8000 pts
                    On  On  On      Bonus city every 10000 pts
                    Off On  On      Bonus city every 12000 pts
                    On  Off On      Bonus city every 14000 pts
                    Off Off On      Bonus city every 15000 pts
                    On  On  Off     Bonus city every 18000 pts
                    Off On  Off     Bonus city every 20000 pts
                    Off Off Off     No bonus cities
                                On  Upright
                                Off Cocktail



    PRICING OPTIONS:
    (8-position switch at R10)

    1   2   3   4   5   6   7   8   Meaning
    -------------------------------------------------------------------------
    On  On                          1 coin 1 play
    Off On                          Free play
    On Off                          2 coins 1 play
    Off Off                         1 coin 2 plays
            On  On                  Right coin mech * 1
            Off On                  Right coin mech * 4
            On  Off                 Right coin mech * 5
            Off Off                 Right coin mech * 6
                    On              Center coin mech * 1
                    Off             Center coin mech * 2
                        On  On      English
                        Off On      French
                        On  Off     German
                        Off Off     Spanish
                                On  ( Unused )
                                Off ( Unused )

    There are 2 different versions of the Super Missile Attack board.  It's not known if
    the roms are different.  The SMA manual mentions a set 3(035822-03E) that will work
    as well as set 2. Missile Command set 1 will not work with the SMA board. It would
    appear set 1 and set 2 as labeled by mame are reversed.

****************************************************************************

Super Missile Attack Board Layout

      |-------------------------------|
      |                               |
    A | 2716       74LS138       2716 |
      |                               |
    B | 2716    63S141  63S141   2716 |
      |                               |
    C | 2716                     2716 |
      |            PAL                |
    D | HDR28                    2716 |
      |            74148              |
    E | HDR28                    2716 |
      |                               |
      |-------------------------------|

          1       2   3   4       5

*****************************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/bankdev.h"
#include "machine/rescap.h"
#include "machine/watchdog.h"
#include "sound/pokey.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class missile_state : public driver_device
{
public:
	missile_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainmap(*this, "mainmap")
		, m_videoram(*this, "videoram", 0x10000, ENDIANNESS_LITTLE)
		, m_watchdog(*this, "watchdog")
		, m_pokey(*this, "pokey")
		, m_inputs(*this, { "IN0", "IN1", "R10", "R8" })
		, m_track(*this, { "TRACK0_X", "TRACK0_Y", "TRACK1_X", "TRACK1_Y" })
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_leds(*this, "led%u", 0U)
		, m_writeprom(*this, "proms")
	{ }

	void missileb(machine_config &config);
	void missile(machine_config &config);
	void missilea(machine_config &config);

	void init_missilem();
	void init_suprmatk();

	int vblank_r();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	inline int scanline_to_v(int scanline);
	inline int v_to_scanline(int v);
	inline void schedule_next_irq(int curv);
	TIMER_CALLBACK_MEMBER(clock_irq);
	void sync_w(int state);
	void irqack_w(uint8_t data);

	TIMER_CALLBACK_MEMBER(adjust_cpu_speed);

	inline void load_madsel(uint8_t data);
	inline bool get_madsel();
	inline offs_t get_bit3_addr(offs_t pixaddr);
	inline void vram_mad_w(offs_t offset, uint8_t data);
	inline uint8_t vram_mad_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data) { m_videoram[offset] = data; }
	uint8_t vram_r(offs_t offset) { return m_videoram[offset]; }

	uint32_t screen_update_missile(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t trackball_r();
	void output_w(uint8_t data);
	void palette_w(offs_t offset, uint8_t data);

	void trampoline_map(address_map &map);
	u8 trampoline_r(offs_t offset);
	void trampoline_w(offs_t offset, uint8_t data);

	void base_map(address_map &map);
	void missile_map(address_map &map);
	void missilea_map(address_map &map);
	void mcombat_map(address_map &map);

	required_device<m6502_device> m_maincpu;
	required_device<address_map_bank_device> m_mainmap;
	memory_share_creator<uint8_t> m_videoram;
	required_device<watchdog_timer_device> m_watchdog;
	optional_device<pokey_device> m_pokey;
	required_ioport_array<4> m_inputs;
	required_ioport_array<4> m_track;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	output_finder<2> m_leds;
	required_region_ptr<uint8_t> m_writeprom;

	emu_timer *m_irq_timer = nullptr;
	emu_timer *m_cpu_timer = nullptr;
	uint8_t m_irq_state = 0;
	uint8_t m_irq_pin = 0;
	uint8_t m_ctrld = 0;
	uint8_t m_flipscreen = 0;
	uint64_t m_madsel_lastcycles = 0;
};


#define MASTER_CLOCK    XTAL(10'000'000)

#define PIXEL_CLOCK     (MASTER_CLOCK/2)
#define HTOTAL          (320)
#define HBSTART         (256)
#define HBEND           (0)
#define VTOTAL          (256)
#define VBSTART         (256)
#define VBEND           (25)    /* 24 causes a garbage line at the top of the screen */



/*************************************
 *
 *  VBLANK and IRQ generation
 *
 *************************************/

int missile_state::scanline_to_v(int scanline)
{
	// since the vertical sync counter counts backwards when flipped, this function returns
	// the current effective V value, given that vpos() only counts forward
	return m_flipscreen ? (256 - scanline) : scanline;
}

int missile_state::v_to_scanline(int v)
{
	// same as a above, but the opposite transformation
	return m_flipscreen ? (256 - v) : v;
}

void missile_state::schedule_next_irq(int curv)
{
	// IRQ = /32V, clocked by /16V ^ flip
	// When not flipped, clocks on 0, 64, 128, 192
	// When flipped, clocks on 16, 80, 144, 208
	if (m_flipscreen)
		curv = ((curv - 32) & 0xff) | 0x10;
	else
		curv = ((curv + 32) & 0xff) & ~0x10;

	// next one at the start of this scanline
	m_irq_timer->adjust(m_screen->time_until_pos(v_to_scanline(curv)), curv);
}

TIMER_CALLBACK_MEMBER(missile_state::clock_irq)
{
	int curv = param;

	// set pending IRQ change
	m_irq_state = BIT(~curv, 5);

	// force an update while we're here
	m_screen->update_partial(v_to_scanline(curv));

	// find the next edge
	schedule_next_irq(curv);
}

void missile_state::sync_w(int state)
{
	// SYNC latches IRQ pin
	if (state && m_irq_state != m_irq_pin)
	{
		m_irq_pin = m_irq_state;
		m_maincpu->set_input_line(0, m_irq_pin ? ASSERT_LINE : CLEAR_LINE);
	}
}

void missile_state::irqack_w(uint8_t data)
{
	m_irq_state = 0;
}

int missile_state::vblank_r()
{
	int v = scanline_to_v(m_screen->vpos());
	return v < 24;
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

TIMER_CALLBACK_MEMBER(missile_state::adjust_cpu_speed)
{
	int curv = param;

	// starting at scanline 224, the CPU runs at half speed
	if (curv == 224)
		m_maincpu->set_unscaled_clock(MASTER_CLOCK/16);
	else
		m_maincpu->set_unscaled_clock(MASTER_CLOCK/8);

	// scanline for the next run
	curv ^= 224;
	m_cpu_timer->adjust(m_screen->time_until_pos(v_to_scanline(curv)), curv);
}

void missile_state::machine_start()
{
	m_leds.resolve();

	// create a timer to speed/slow the CPU
	m_cpu_timer = timer_alloc(FUNC(missile_state::adjust_cpu_speed), this);
	m_cpu_timer->adjust(m_screen->time_until_pos(v_to_scanline(0), 0));

	// create a timer for IRQs and set up the first callback
	m_irq_timer = timer_alloc(FUNC(missile_state::clock_irq), this);
	schedule_next_irq(-32);

	// setup for save states
	save_item(NAME(m_irq_state));
	save_item(NAME(m_irq_pin));
	save_item(NAME(m_ctrld));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_madsel_lastcycles));
}

void missile_state::machine_reset()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_irq_pin = 0;
	m_irq_state = 0;
	m_madsel_lastcycles = 0;
}



/*************************************
 *
 *  VRAM access
 *
 *************************************/

void missile_state::load_madsel(uint8_t data)
{
	// MADSEL counter is loaded at SYNC when the low 5 bytes of the data bus are 0x01
	// and the IRQ signal is clear
	if (!m_irq_pin && ((data & 0x1f) == 0x01) && m_maincpu->get_sync() && !machine().side_effects_disabled())
		m_madsel_lastcycles = m_maincpu->total_cycles();
}

bool missile_state::get_madsel()
{
	// the MADSEL signal disables standard address decoding and routes writes to video RAM;
	// it goes high 5 cycles after loading the counter, and goes low again after 1 cycle
	bool madsel = false;

	if (m_madsel_lastcycles)
	{
		madsel = (m_maincpu->total_cycles() - m_madsel_lastcycles) == 5;

		// reset the count until next time
		if (madsel && !machine().side_effects_disabled())
			m_madsel_lastcycles = 0;
	}

	return madsel;
}

offs_t missile_state::get_bit3_addr(offs_t pixaddr)
{
	// the 3rd bit of video RAM is scattered about various areas, we take a 16-bit pixel address here
	// and convert it into a video RAM address based on logic in the schematics
	return  (( pixaddr & 0x0800) >> 1) |
			((~pixaddr & 0x0800) >> 2) |
			(( pixaddr & 0x07f8) >> 2) |
			(( pixaddr & 0x1000) >> 12);
}

void missile_state::vram_mad_w(offs_t offset, uint8_t data)
{
	static const uint8_t data_lookup[4] = { 0x00, 0x0f, 0xf0, 0xff };
	offs_t vramaddr;
	uint8_t vramdata;
	uint8_t vrammask;

	// basic 2 bit VRAM writes go to addr >> 2
	// data comes from bits 6 and 7
	// this should only be called if MADSEL == 1
	vramaddr = offset >> 2;
	vramdata = data_lookup[data >> 6];
	vrammask = m_writeprom[(offset & 7) | 0x10];
	m_videoram[vramaddr] = (m_videoram[vramaddr] & vrammask) | (vramdata & ~vrammask);

	// 3-bit VRAM writes use an extra clock to write the 3rd bit elsewhere
	// on the schematics, this is the MUSHROOM == 1 case
	if ((offset & 0xe000) == 0xe000)
	{
		vramaddr = get_bit3_addr(offset);
		vramdata = -((data >> 5) & 1);
		vrammask = m_writeprom[(offset & 7) | 0x18];
		m_videoram[vramaddr] = (m_videoram[vramaddr] & vrammask) | (vramdata & ~vrammask);

		// account for the extra clock cycle
		if (!machine().side_effects_disabled())
			m_maincpu->adjust_icount(-1);
	}
}

uint8_t missile_state::vram_mad_r(offs_t offset)
{
	offs_t vramaddr;
	uint8_t vramdata;
	uint8_t vrammask;
	uint8_t result = 0xff;

	// basic 2 bit VRAM reads go to addr >> 2
	// data goes to bits 6 and 7
	// this should only be called if MADSEL == 1
	vramaddr = offset >> 2;
	vrammask = 0x11 << (offset & 3);
	vramdata = m_videoram[vramaddr] & vrammask;
	if ((vramdata & 0xf0) == 0)
		result &= ~0x80;
	if ((vramdata & 0x0f) == 0)
		result &= ~0x40;

	// 3-bit VRAM reads use an extra clock to read the 3rd bit elsewhere
	// on the schematics, this is the MUSHROOM == 1 case
	if ((offset & 0xe000) == 0xe000)
	{
		vramaddr = get_bit3_addr(offset);
		vrammask = 1 << (offset & 7);
		vramdata = m_videoram[vramaddr] & vrammask;
		if (vramdata == 0)
			result &= ~0x20;

		// account for the extra clock cycle
		if (!machine().side_effects_disabled())
			m_maincpu->adjust_icount(-1);
	}
	return result;
}



/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t missile_state::screen_update_missile(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw the bitmap to the screen, looping over Y
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint16_t *const dst = &bitmap.pix(y);

		int const effy = m_flipscreen ? ((256+24 - y) & 0xff) : y;
		uint8_t const *const src = &m_videoram[effy * 64];
		uint8_t const *src3 = nullptr;

		// compute the base of the 3rd pixel row
		if (effy >= 224)
			src3 = &m_videoram[get_bit3_addr(effy << 8)];

		// loop over X
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			uint8_t pix = src[x / 4] >> (x & 3);
			pix = ((pix >> 2) & 4) | ((pix << 1) & 2);

			// if we're in the lower region, get the 3rd bit
			if (src3)
				pix |= (src3[(x / 8) * 2] >> (x & 7)) & 1;

			dst[x] = pix;
		}
	}
	return 0;
}



/*************************************
 *
 *  Misc. I/O
 *
 *************************************/

uint8_t missile_state::trackball_r()
{
	// read trackball
	if (m_ctrld)
	{
		if (!m_flipscreen)
			return ((m_track[1]->read() << 4) & 0xf0) | (m_track[0]->read() & 0x0f);
		else
			return ((m_track[3]->read() << 4) & 0xf0) | (m_track[2]->read() & 0x0f);
	}

	// normal buttons
	return m_inputs[0]->read();
}

void missile_state::output_w(uint8_t data)
{
	// bit 0 selects trackball
	m_ctrld = data & 1;

	// leds
	m_leds[0] = BIT(~data, 1);
	m_leds[1] = BIT(~data, 2);

	// coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x20);
	machine().bookkeeping().coin_counter_w(1, data & 0x10);
	machine().bookkeeping().coin_counter_w(2, data & 0x08);

	// flip screen
	m_flipscreen = ~data & 0x40;
}

void missile_state::palette_w(offs_t offset, uint8_t data)
{
	// color RAM
	m_palette->set_pen_color(offset & 7, pal1bit(~data >> 3), pal1bit(~data >> 2), pal1bit(~data >> 1));
}



/*************************************
 *
 *  Address maps
 *
 *************************************/

void missile_state::trampoline_w(offs_t offset, uint8_t data)
{
	// if this is a MADSEL cycle, write to video RAM
	if (get_madsel())
		vram_mad_w(offset, data);
	else
		m_mainmap->write8(offset, data);
}

u8 missile_state::trampoline_r(offs_t offset)
{
	// if this is a MADSEL cycle, read from video RAM
	if (get_madsel())
		return vram_mad_r(offset);

	uint8_t data = m_mainmap->read8(offset);
	load_madsel(data);

	return data;
}

void missile_state::trampoline_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(missile_state::trampoline_r), FUNC(missile_state::trampoline_w));
}

void missile_state::base_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7fff);
	map(0x0000, 0x3fff).rw(FUNC(missile_state::vram_r), FUNC(missile_state::vram_w));
	map(0x4800, 0x4800).mirror(0x00ff).rw(FUNC(missile_state::trackball_r), FUNC(missile_state::output_w));
	map(0x4900, 0x4900).mirror(0x00ff).portr("IN1");
	map(0x4a00, 0x4a00).mirror(0x00ff).portr("R10");
	map(0x4b00, 0x4b07).mirror(0x00f8).w(FUNC(missile_state::palette_w)).nopr();
	map(0x4c00, 0x4c00).mirror(0x00ff).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x4d00, 0x4d00).mirror(0x00ff).w(FUNC(missile_state::irqack_w));
	map(0x5000, 0x7fff).rom().region("maincpu", 0x5000);
}

void missile_state::missile_map(address_map &map)
{
	base_map(map);
	map(0x4000, 0x400f).mirror(0x07f0).rw(m_pokey, FUNC(pokey_device::read), FUNC(pokey_device::write));
}

void missile_state::missilea_map(address_map &map)
{
	base_map(map);
	map(0x4000, 0x4000).mirror(0x00ff).portr("R8"); // also sound
}

void missile_state::mcombat_map(address_map &map)
{
	missilea_map(map);
	map(0x4900, 0x4900).mirror(0x00ff).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x4c00, 0x4c00).mirror(0x00ff).unmapw();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( missile )
	PORT_START("IN0") // IN0
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("IN1") // IN1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x18, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_TOGGLE // switch inside the coin door
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(missile_state, vblank_r)

	PORT_START("R10") // IN2
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) ) PORT_DIPLOCATION("R10:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, "Right Coin" ) PORT_DIPLOCATION("R10:3,4")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x04, "*4" )
	PORT_DIPSETTING(    0x08, "*5" )
	PORT_DIPSETTING(    0x0c, "*6" )
	PORT_DIPNAME( 0x10, 0x00, "Center Coin" ) PORT_DIPLOCATION("R10:5")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x10, "*2" )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("R10:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x20, DEF_STR( French ) )
	PORT_DIPSETTING(    0x40, DEF_STR( German ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Spanish ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("R10:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("R8") // IN3
	PORT_DIPNAME( 0x03, 0x03, "Cities" ) PORT_DIPLOCATION("R8:!1,!2")
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, "Bonus Credit for 4 Coins" ) PORT_DIPLOCATION("R8:!3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Trackball Size" ) PORT_DIPLOCATION("R8:!4")
	PORT_DIPSETTING(    0x00, "Mini" ) // Faster Cursor Speed
	PORT_DIPSETTING(    0x08, "Large" ) // Slower Cursor Speed
	PORT_DIPNAME( 0x70, 0x70, "Bonus City" ) PORT_DIPLOCATION("R8:!5,!6,!7")
	PORT_DIPSETTING(    0x10, "8000" )
	PORT_DIPSETTING(    0x70, "10000" )
	PORT_DIPSETTING(    0x60, "12000" )
	PORT_DIPSETTING(    0x50, "14000" )
	PORT_DIPSETTING(    0x40, "15000" )
	PORT_DIPSETTING(    0x30, "18000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("R8:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("TRACK0_X") // FAKE
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10)

	PORT_START("TRACK0_Y") // FAKE
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("TRACK1_X") // FAKE
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL

	PORT_START("TRACK1_Y") // FAKE
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( missileb )
	PORT_INCLUDE(missile)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE // switch inside the coin door
INPUT_PORTS_END

static INPUT_PORTS_START( suprmatk )
	PORT_INCLUDE(missile)

	PORT_MODIFY("R10") // IN2
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("R10:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, "Game" ) PORT_DIPLOCATION("R10:7,8")
	PORT_DIPSETTING(    0x00, "Missile Command" )
	PORT_DIPSETTING(    0x40, "Easy Super Missile Attack" )
	PORT_DIPSETTING(    0x80, "Reg. Super Missile Attack" )
	PORT_DIPSETTING(    0xc0, "Hard Super Missile Attack" )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void missile_state::missile(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, MASTER_CLOCK/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &missile_state::trampoline_map);
	m_maincpu->sync_cb().set(FUNC(missile_state::sync_w));

	ADDRESS_MAP_BANK(config, m_mainmap);
	m_mainmap->set_options(ENDIANNESS_LITTLE, 8, 16);
	m_mainmap->set_map(&missile_state::missile_map);

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count(m_screen, 8);

	// video hardware
	PALETTE(config, m_palette).set_entries(8);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(missile_state::screen_update_missile));
	m_screen->set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	POKEY(config, m_pokey, MASTER_CLOCK/8);
	m_pokey->allpot_r().set_ioport("R8");
	m_pokey->set_output_rc(RES_K(10), CAP_U(0.1), 5.0);
	m_pokey->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void missile_state::missilea(machine_config &config)
{
	missile(config);

	m_mainmap->set_map(&missile_state::missilea_map);

	config.device_remove("pokey");
}

void missile_state::missileb(machine_config &config)
{
	missilea(config);

	m_mainmap->set_map(&missile_state::mcombat_map);

	AY8912(config, "ay8912", MASTER_CLOCK/8).add_route(ALL_OUTPUTS, "mono", 0.75);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( missile )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "035820-02.h1",  0x5000, 0x0800, CRC(7a62ce6a) SHA1(9a39978138dc28fdefe193bfae1b226391e471db) )
	ROM_LOAD( "035821-02.jk1", 0x5800, 0x0800, CRC(df3bd57f) SHA1(0916925d3c94d766d33f0e4badf6b0add835d748) )
	ROM_LOAD( "035822-03e.kl1",0x6000, 0x0800, CRC(1a2f599a) SHA1(2deb1219223032a9c83114e4e8b2fc11a570754c) )
	ROM_LOAD( "035823-02.ln1", 0x6800, 0x0800, CRC(82e552bb) SHA1(d0f22894f779c74ceef644c9f03d840d9545efea) )
	ROM_LOAD( "035824-02.np1", 0x7000, 0x0800, CRC(606e42e0) SHA1(9718f84a73c66b4e8ef7805a7ab638a7380624e1) )
	ROM_LOAD( "035825-02.r1",  0x7800, 0x0800, CRC(f752eaeb) SHA1(0339a6ce6744d2091cc7e07675e509b202b0f380) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "035826-01.l6",  0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END


ROM_START( missile2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "035820-02.h1",  0x5000, 0x0800, CRC(7a62ce6a) SHA1(9a39978138dc28fdefe193bfae1b226391e471db) )
	ROM_LOAD( "035821-02.jk1", 0x5800, 0x0800, CRC(df3bd57f) SHA1(0916925d3c94d766d33f0e4badf6b0add835d748) )
	ROM_LOAD( "035822-02.kl1", 0x6000, 0x0800, CRC(a1cd384a) SHA1(a1dd0953423750a0fbc6e3dccbf2ca64ef5a1f54) )
	ROM_LOAD( "035823-02.ln1", 0x6800, 0x0800, CRC(82e552bb) SHA1(d0f22894f779c74ceef644c9f03d840d9545efea) )
	ROM_LOAD( "035824-02.np1", 0x7000, 0x0800, CRC(606e42e0) SHA1(9718f84a73c66b4e8ef7805a7ab638a7380624e1) )
	ROM_LOAD( "035825-02.r1",  0x7800, 0x0800, CRC(f752eaeb) SHA1(0339a6ce6744d2091cc7e07675e509b202b0f380) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "035826-01.l6",  0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END


ROM_START( missile1 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "35820-01.h1",  0x5000, 0x0800, CRC(41cbb8f2) SHA1(5dcb58276c08d75d36baadb6cefe30d4916de9b0) )
	ROM_LOAD( "35821-01.jk1", 0x5800, 0x0800, CRC(728702c8) SHA1(6f25af7133d3ec79029117162649f94e93f36e0e) )
	ROM_LOAD( "35822-01.kl1", 0x6000, 0x0800, CRC(28f0999f) SHA1(eb52b11c6757c8dc3be88b276ea4dc7dfebf7cf7) )
	ROM_LOAD( "35823-01.ln1", 0x6800, 0x0800, CRC(bcc93c94) SHA1(f0daa5d2835a856e2038612e755dc7ded28fc923) )
	ROM_LOAD( "35824-01.np1", 0x7000, 0x0800, CRC(0ca089c8) SHA1(7f69ee990fd4fa1f2fceca7fc66fcaa02e4d2314) )
	ROM_LOAD( "35825-01.r1",  0x7800, 0x0800, CRC(428cf0d5) SHA1(03cabbef50c33852fbbf38dd3eecaf70a82df82f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "035826-01.l6", 0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END

ROM_START( suprmatk )
	ROM_REGION( 0x9000, "maincpu", 0 ) // ROM's located on the enhancement board
	ROM_LOAD( "035820-02.c1", 0x5000, 0x0800, CRC(7a62ce6a) SHA1(9a39978138dc28fdefe193bfae1b226391e471db) )
	ROM_LOAD( "035821-02.b1", 0x5800, 0x0800, CRC(df3bd57f) SHA1(0916925d3c94d766d33f0e4badf6b0add835d748) )
	ROM_LOAD( "035822-02.a1", 0x6000, 0x0800, CRC(a1cd384a) SHA1(a1dd0953423750a0fbc6e3dccbf2ca64ef5a1f54) )
	ROM_LOAD( "035823-02.a5", 0x6800, 0x0800, CRC(82e552bb) SHA1(d0f22894f779c74ceef644c9f03d840d9545efea) )
	ROM_LOAD( "035824-02.b5", 0x7000, 0x0800, CRC(606e42e0) SHA1(9718f84a73c66b4e8ef7805a7ab638a7380624e1) )
	ROM_LOAD( "035825-02.c5", 0x7800, 0x0800, CRC(f752eaeb) SHA1(0339a6ce6744d2091cc7e07675e509b202b0f380) )
	ROM_LOAD( "e0.d5",        0x8000, 0x0800, CRC(d0b20179) SHA1(e2a9855899b6ff96b8dba169e0ab83f00a95919f) )
	ROM_LOAD( "e1.e5",        0x8800, 0x0800, CRC(c6c818a3) SHA1(b9c92a85c07dd343d990e196d37b92d92a85a5e0) )

	ROM_REGION( 0x0020, "proms", 0 ) // PROM located on the Missile Command board
	ROM_LOAD( "035826-01.l6", 0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )

	ROM_REGION( 0x0200, "proms2", 0 ) // 63S141 PROMs located on the enhancement board
	ROM_LOAD( "63s141.b2",    0x0000, 0x0100, CRC(2de8ee4d) SHA1(ff28c007df9c52227dfce76af6f7b1dfac3c2296) )
	ROM_LOAD( "63s141.b4",    0x0100, 0x0100, CRC(390fc532) SHA1(f9adde3f18f3db225ac3f3771c38ff139ef0a65e) )
ROM_END


ROM_START( suprmatkd )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "035820.sma",   0x5000, 0x0800, CRC(75f01b87) SHA1(32ed71b6a869d7b361f244c384bbe6f407f6c6d7) )
	ROM_LOAD( "035821.sma",   0x5800, 0x0800, CRC(3320d67e) SHA1(5bb04b985421af6309818b94676298f4b90495cf) )
	ROM_LOAD( "035822.sma",   0x6000, 0x0800, CRC(e6be5055) SHA1(43912cc565cb43256a9193594cf36abab1c85d6f) )
	ROM_LOAD( "035823.sma",   0x6800, 0x0800, CRC(a6069185) SHA1(899cd8b378802eb6253d4bca7432797168595d53) )
	ROM_LOAD( "035824.sma",   0x7000, 0x0800, CRC(90a06be8) SHA1(f46fd6847bc9836d11ea0042df19fbf33ddab0db) )
	ROM_LOAD( "035825.sma",   0x7800, 0x0800, CRC(1298213d) SHA1(c8e4301704e3700c339557f2a833e70f6a068d5e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "035826-01.l6", 0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END


/*

Missile Command Multigame, produced by Braze Technologies
from 2005(1st version) to 2007(version 1d). This kit combines
Missile Command and Super Missile Attack on a daughterboard
plugged into the main pcb cpu slot.

- M6502 CPU (from main pcb)
- 27C512 64KB EPROM
- 93C46P E2PROM for saving highscore/settings
- two 74LS chips (labels sandpapered off)

*/

ROM_START( missilem )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 ) // banked, decrypted rom goes here

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "mcm001d.512", 0x00000, 0x10000, CRC(0a5845b5) SHA1(4828866018a984e7cd7f55a33613f43ece5d8d63) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "035826-01.l6", 0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END


/*

Missile Combat bootlegs by 'Videotron'

1x 6502A (main)
1x AY-3-8912 (sound)
1x oscillator 10000

PCB is marked: "VIDEOTRON BOLOGNA 002"

*/

ROM_START( mcombat )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "002-0-0.10a",  0x5000, 0x0800, CRC(589b81de) SHA1(06f18a837cedb0da5464dfaa04f92bd035db3752) )
	ROM_LOAD( "002-1-1.9a",   0x5800, 0x0800, CRC(08796a78) SHA1(e5aabe775889752ad1581098fcbf52ff1fa03b3b) )
	ROM_LOAD( "002-2-2.8a",   0x6000, 0x0800, CRC(59ab750c) SHA1(4555c27ddeb22ba895610a9c516fe574664a6f4b) )
	ROM_LOAD( "002-3-3.7a",   0x6800, 0x0800, CRC(3295cc3f) SHA1(2be0d492bd791df19d138d5bfe956361ee461989) )
	ROM_LOAD( "002-4-4.6a",   0x7000, 0x0800, CRC(aac71e95) SHA1(7daf115eb2cdde69b7c4de1e1a6ee68cd2fd0f2c) )
	ROM_LOAD( "002-5-5.5a",   0x7800, 0x0800, CRC(1b9a16e2) SHA1(03fb292bb6f815724b2fc4b2f561398000367373) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6f",   0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END


ROM_START( mcombata )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "002-0-0.10a",  0x5000, 0x0800, CRC(589b81de) SHA1(06f18a837cedb0da5464dfaa04f92bd035db3752) )
	ROM_LOAD( "002-1-1.9a",   0x5800, 0x0800, CRC(08796a78) SHA1(e5aabe775889752ad1581098fcbf52ff1fa03b3b) )
	ROM_LOAD( "002-2-2.8a",   0x6000, 0x0800, CRC(59ab750c) SHA1(4555c27ddeb22ba895610a9c516fe574664a6f4b) )
	ROM_LOAD( "3.bin",        0x6800, 0x0800, CRC(ddbfda20) SHA1(444daaa76751853f67a7c0e5bf620ae5623d2105) )
	ROM_LOAD( "4.bin",        0x7000, 0x0800, CRC(e3b5428d) SHA1(ac9eb459df68a117a49e92fbc5ed88faaf46a395) )
	ROM_LOAD( "002-5-5.5a",   0x7800, 0x0800, CRC(1b9a16e2) SHA1(03fb292bb6f815724b2fc4b2f561398000367373) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6f",   0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END

ROM_START( mcombats ) // bootleg (Sidam) @ $
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "002-0-0.10a",  0x5000, 0x0800, CRC(589b81de) SHA1(06f18a837cedb0da5464dfaa04f92bd035db3752) ) // 002-0.0a
	ROM_LOAD( "002-1-1.9a",   0x5800, 0x0800, CRC(08796a78) SHA1(e5aabe775889752ad1581098fcbf52ff1fa03b3b) ) // 002-1.1a
	ROM_LOAD( "002-2-2.8a",   0x6000, 0x0800, CRC(59ab750c) SHA1(4555c27ddeb22ba895610a9c516fe574664a6f4b) ) // 002-2.2a
	ROM_LOAD( "002-3.3a",     0x6800, 0x0800, CRC(3ad69b83) SHA1(847fa697fc8e9c890ebf3794fb14fedad81aee19) )
	ROM_LOAD( "002-4.4a",     0x7000, 0x0800, CRC(aac71e95) SHA1(7daf115eb2cdde69b7c4de1e1a6ee68cd2fd0f2c) )
	ROM_LOAD( "002-5-5.5a",   0x7800, 0x0800, CRC(1b9a16e2) SHA1(03fb292bb6f815724b2fc4b2f561398000367373) ) // 002-5.5a

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6f",   0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END


/*
CPUs
QTY     Type    clock   position    function
1x  6502        2B  8-bit Microprocessor - main
1x  LM380       12B     Audio Amplifier - sound
1x  oscillator  10.000  6C

ROMs
QTY     Type    position    status
2x  F2708   10C, 10E    dumped
6x  MCM2716C    1-6     dumped
1x  DM74S288N   6L  dumped

RAMs
QTY     Type    position
8x  TMS4116     4F,4H,4J,4K,4L,4M,4N,4P
1x  74S189N     7L

Others

1x 22x2 edge connector
1x trimmer (volume)(12E)
2x 8x2 switches DIP(8R,10R)
*/

ROM_START( missilea )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "1.1h", 0x5000, 0x0800, CRC(49d66ca1) SHA1(59e20a048ac76aff5843dc8253fe61cdb65f94fb) )
	ROM_LOAD( "2.1j", 0x5800, 0x0800, CRC(8008918d) SHA1(a72ce8997ba66c0f51d94e4f2cb3cf2734612ac9) )
	ROM_LOAD( "3.1k", 0x6000, 0x0800, CRC(b87c02f7) SHA1(748fa3aa2e7314407ded0d6c0fc24f9015c66c28) )
	ROM_LOAD( "4.1l", 0x6800, 0x0800, CRC(33bc6f47) SHA1(5753fbfbcd65a4863f877cd3a07400bd61ffb8ce) )
	ROM_LOAD( "5.1n", 0x7000, 0x0800, CRC(df8c58f4) SHA1(44a0cdb0e5222e14e3e91547c35e73b2cf2df174) )
	ROM_LOAD( "6.1r", 0x7800, 0x0800, CRC(96a21c1f) SHA1(6e43ce8d53aa6d38cef920e7fa2e0683ce42cdb4) )

	ROM_REGION( 0x800, "samples", 0 )
	ROM_LOAD( "2708.10c",  0x0000, 0x0400, CRC(9f6978c4) SHA1(34b356fddd86b8b73ee1415b3ad6b00dc4be60e2) )
	ROM_LOAD( "2708.10e",  0x0400, 0x0400, CRC(90eb28c8) SHA1(c82bc0a00d9e54004b0210f95343c8d7dc1f2050) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "dm74s288n.6l", 0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END


ROM_START( x80wc ) // PCB etched COMPUTER GAME 80
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "20mm.b1", 0x5000, 0x0400, CRC(20460c38) SHA1(a9a0e951ead6ca96d094bfdcbb89a802aa108340) )
	ROM_LOAD( "1mm.c1",  0x5400, 0x0400, CRC(83d0a10e) SHA1(b382ba1a893601a4a00922960c79363664bceaa4) ) // 1xxxxxxxxxx = 0xFF
	ROM_IGNORE(                  0x0400 )
	ROM_LOAD( "2mm.d1",  0x5800, 0x0400, CRC(291fe2bf) SHA1(9d2ed129e5da90b2542e001e0a8a5f67e4f2194b) )
	ROM_LOAD( "3mm.e1",  0x5c00, 0x0400, CRC(d868e424) SHA1(be51917c609a8d804ec271ba84358aa928572d1c) )
	ROM_LOAD( "4mm.fh1", 0x6000, 0x0400, CRC(72d0e053) SHA1(5f4e6c826081d19bb185f00be5a22481b17ede4c) )
	ROM_LOAD( "5mm.j1",  0x6400, 0x0400, CRC(dd8b21a3) SHA1(5f09fdcf915728762f50655c48aaed40c6e87d7c) )
	ROM_LOAD( "6mm.k1",  0x6800, 0x0400, CRC(066c48d7) SHA1(34d78d3854816e5935acfda814be0aeffc4450d1) )
	ROM_LOAD( "7mm.l1",  0x6c00, 0x0400, CRC(305781c7) SHA1(618303d596d114e91cf56573f518e6188af628bb) )
	ROM_LOAD( "8mm.m1",  0x7000, 0x0400, CRC(091a9353) SHA1(63364a4ddb71d0760a51dabb8aa697931a858169) )
	ROM_LOAD( "9mm.n1",  0x7400, 0x0400, CRC(1955b6e4) SHA1(118e813c3b965c879185efdd75c2f1b9ef9fda9d) )
	ROM_LOAD( "10mm.p1", 0x7800, 0x0400, CRC(770fe1a3) SHA1(b75220778909887daef2d6a0da2d01bcce6629df) )
	ROM_LOAD( "11mm.r1", 0x7c00, 0x0400, CRC(5c5e775b) SHA1(72be5e60f115c1836d6a5e02b55877127855cf3f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.l6",  0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void missile_state::init_suprmatk()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x40; i++)
	{
		rom[0x7cc0+i] = rom[0x8000+i];
		rom[0x5440+i] = rom[0x8040+i];
		rom[0x5b00+i] = rom[0x8080+i];
		rom[0x5740+i] = rom[0x80c0+i];
		rom[0x6000+i] = rom[0x8100+i];
		rom[0x6540+i] = rom[0x8140+i];
		rom[0x7500+i] = rom[0x8180+i];
		rom[0x7100+i] = rom[0x81c0+i];
		rom[0x7800+i] = rom[0x8200+i];
		rom[0x5580+i] = rom[0x8240+i];
		rom[0x5380+i] = rom[0x8280+i];
		rom[0x6900+i] = rom[0x82c0+i];
		rom[0x6e00+i] = rom[0x8300+i];
		rom[0x6cc0+i] = rom[0x8340+i];
		rom[0x7dc0+i] = rom[0x8380+i];
		rom[0x5b80+i] = rom[0x83c0+i];
		rom[0x5000+i] = rom[0x8400+i];
		rom[0x7240+i] = rom[0x8440+i];
		rom[0x7040+i] = rom[0x8480+i];
		rom[0x62c0+i] = rom[0x84c0+i];
		rom[0x6840+i] = rom[0x8500+i];
		rom[0x7ec0+i] = rom[0x8540+i];
		rom[0x7d40+i] = rom[0x8580+i];
		rom[0x66c0+i] = rom[0x85c0+i];
		rom[0x72c0+i] = rom[0x8600+i];
		rom[0x7080+i] = rom[0x8640+i];
		rom[0x7d00+i] = rom[0x8680+i];
		rom[0x5f00+i] = rom[0x86c0+i];
		rom[0x55c0+i] = rom[0x8700+i];
		rom[0x5a80+i] = rom[0x8740+i];
		rom[0x6080+i] = rom[0x8780+i];
		rom[0x7140+i] = rom[0x87c0+i];
		rom[0x7000+i] = rom[0x8800+i];
		rom[0x6100+i] = rom[0x8840+i];
		rom[0x5400+i] = rom[0x8880+i];
		rom[0x5bc0+i] = rom[0x88c0+i];
		rom[0x7e00+i] = rom[0x8900+i];
		rom[0x71c0+i] = rom[0x8940+i];
		rom[0x6040+i] = rom[0x8980+i];
		rom[0x6e40+i] = rom[0x89c0+i];
		rom[0x5800+i] = rom[0x8a00+i];
		rom[0x7d80+i] = rom[0x8a40+i];
		rom[0x7a80+i] = rom[0x8a80+i];
		rom[0x53c0+i] = rom[0x8ac0+i];
		rom[0x6140+i] = rom[0x8b00+i];
		rom[0x6700+i] = rom[0x8b40+i];
		rom[0x7280+i] = rom[0x8b80+i];
		rom[0x7f00+i] = rom[0x8bc0+i];
		rom[0x5480+i] = rom[0x8c00+i];
		rom[0x70c0+i] = rom[0x8c40+i];
		rom[0x7f80+i] = rom[0x8c80+i];
		rom[0x5780+i] = rom[0x8cc0+i];
		rom[0x6680+i] = rom[0x8d00+i];
		rom[0x7200+i] = rom[0x8d40+i];
		rom[0x7e40+i] = rom[0x8d80+i];
		rom[0x7ac0+i] = rom[0x8dc0+i];
		rom[0x6300+i] = rom[0x8e00+i];
		rom[0x7180+i] = rom[0x8e40+i];
		rom[0x7e80+i] = rom[0x8e80+i];
		rom[0x6280+i] = rom[0x8ec0+i];
		rom[0x7f40+i] = rom[0x8f00+i];
		rom[0x6740+i] = rom[0x8f40+i];
		rom[0x74c0+i] = rom[0x8f80+i];
		rom[0x7fc0+i] = rom[0x8fc0+i];
	}
}

void missile_state::init_missilem()
{
	uint8_t *src = memregion("user1")->base();
	uint8_t *dest = memregion("maincpu")->base();

	// decrypt rom and put in maincpu region (result looks correct, but is untested)
	for (int i = 0; i < 0x10000; i++)
	{
		int a = bitswap<16>(i, 15,2,3,0,8,9,7,5,1,4,6,14,13,12,10,11);
		int d = bitswap<8>(src[a], 3,2,4,5,6,1,7,0);

		a = i;
		a ^= (~a >> 1 & 0x400);
		a ^= (~a >> 4 & 0x100);
		a ^= ( a >> 7 & 0x100);

		dest[a] = d;
	}
}

} // anonymous namespace



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1980, missile,  0,       missile, missile,   missile_state, empty_init,    ROT0, "Atari", "Missile Command (rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, missile2, missile, missile, missile,   missile_state, empty_init,    ROT0, "Atari", "Missile Command (rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, missile1, missile, missile, missile,   missile_state, empty_init,    ROT0, "Atari", "Missile Command (rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, suprmatk, missile, missile, suprmatk,  missile_state, init_suprmatk, ROT0, "Atari / General Computer Corporation", "Super Missile Attack (for rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, suprmatkd,missile, missile, suprmatk,  missile_state, empty_init,    ROT0, "Atari / General Computer Corporation", "Super Missile Attack (not encrypted)", MACHINE_SUPPORTS_SAVE )

// the following bootleg has extremely similar program ROMs to missile1, but has different unknown sound hardware and 2 more ROMs
GAME( 1981, missilea, missile, missilea, missile,  missile_state, empty_init,    ROT0, "bootleg (U.Games)", "Missile Attack (U.Games bootleg of Missile Command)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

// the following bootleg has different unknown sound hardware (sound chip had been removed from the PCB the dump comes from)
GAME( 1981, x80wc,    missile, missilea, missile,  missile_state, empty_init,    ROT0, "bootleg (ManilaMatic)", "X80 - War Command (ManilaMatic bootleg of Missile Command)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

// the following bootlegs are on different hardware and don't work
GAME( 1980, mcombat,  missile, missileb, missileb, missile_state, empty_init,    ROT0, "bootleg (Videotron)", "Missile Combat (Videotron bootleg, set 1)", MACHINE_NOT_WORKING )
GAME( 1980, mcombata, missile, missileb, missileb, missile_state, empty_init,    ROT0, "bootleg (Videotron)", "Missile Combat (Videotron bootleg, set 2)", MACHINE_NOT_WORKING )
GAME( 1980, mcombats, missile, missileb, missileb, missile_state, empty_init,    ROT0, "bootleg (Sidam)", "Missile Combat (Sidam bootleg)", MACHINE_NOT_WORKING )
GAME( 2005, missilem, missile, missile,  missile,  missile_state, init_missilem, ROT0, "hack (Braze Technologies)", "Missile Command Multigame", MACHINE_NOT_WORKING )
