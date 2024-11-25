// license:BSD-3-Clause
// copyright-holders: Nicola Salmoria,Paul Hampson, Quench

/*
China Gate.
By Paul Hampson from First Principles
(IE: ROMs + a description of their contents and a list of CPUs on board.)

Based on ddragon.c:
"Double Dragon, Double Dragon (bootleg) & Double Dragon II"
"By Carlos A. Lozano & Rob Rosenbrock et. al."

NOTES:
A couple of things unaccounted for:

No backgrounds ROMs from the original board...
- TOSHIBA TRJ-100 installed at the second board should contain the image
  as U.S. Championship V'Ball has a TRJ-101 that contains it. It also contains
  related logic to generate 16-bits address and to decode pixels at 6MHz pixel
  clock, based on given attributes in multicycles, screen flip flag, and clocks.
  It seems almost equivalent to Double Dragon's  IC38, 39, 40, 53, 54, and all
  logic in the page 9 of the schematics for the second board.
- Got two bootleg sets with background gfx ROMs. Using those on the
  original games for now.

OBVIOUS SPEED PROBLEMS...
- Timers are too fast and/or too slow, and the whole thing's moving too fast

Port 0x2800 on the Sub CPU.
- All those I/O looking ports on the main CPU (0x3exx and 0x3fxx)
- One's scroll control. Probably other video control as well.
- Location 0x1a2ec in cgate51.bin (The main CPU's ROM) is 88. This is
  copied to videoram, and causes that minor visual discrepancy on
  the title screen. But the CPU tests that part of the ROM and passes
  it OK. Since it's just a simple summing of words, another word
  somewhere (or others in total) has lost 0x8000. Or the original
  game had this problem. (Not on the screenshot I got)
- The Japanese ones have a different title screen so I can't check.

ADPCM in the bootlegs is not quite right.... Misusing the data?
- They're nibble-swapped versions of the original ROMs...
- There's an Intel i8748 CPU on the bootlegs (bootleg 1 lists D8749 but
  the microcode dump's the same). This in conjunction with the different
  ADPCM chip (msm5205) are used to 'fake' a M6295.
- Bootleg 1 ADPCM is now wired up, but still not working :-(
  Definitely sync problems between the i8049 and the m5205 which need
  further looking at.


There's also a few small dumps from the boards.


MAJOR DIFFERENCES FROM DOUBLE DRAGON:
Sound system is like Double Dragon II (In fact for MAME's
purposes it's identical. I think DD3 and one or two others
also use this. Was it an addon on the original?
The dual-CPU setup looked similar to DD at first, but
the second CPU doesn't talk to the sprite RAM at all, but
just through the shared memory (which DD1 doesn't have,
except for the sprite RAM.)
Also the 2nd CPU in China Gate has just as much code as
the first CPU, and bankswitches similarly, where DD1 and DD2 have
different Sprite CPUs but only a small bank of code each.
More characters and colours of characters than DD1 or 2.
More sprites than DD1, less than DD2.
But the formats are the same (allowing for extra chars and colours)
Video hardware's like DD1 (thank god)
Input is unique but has a few similarities to DD2 (the coin inputs)

2008-07
Dip locations and factory settings verified with China Gate US manual.


PCB Layout
----------

TA-0023-P1-03 (Main Board)
|-----------------------------------------------------|
|        |      J1      |     |      J2      |        |
|        ----------------     ----------------        |
| X1                                                  |
|                                                     |
|                                                     |
|    LH0080A                                          |
|                                                     |
|                                                     |
|                HD68B09EP    HD68B09EP               |
|     23J0-0      (main)        (sub)         23J6-0  |
|                                                     |
|  YM2151                              23J5-0         |
|       23J1 23J2  23J3-0      23J4-0                 |
|        -0   -0                                      |
|                                                     |
|                                                     |
|   YM3012  M6295                                     |
|                                                     |
|                                                     |
|            X2                                       |
|                                                     |
|                                             SW2 SW1 |
|                                                     |
|                                                     |
|  VR1           |-------------------|                |
|                |   ||  JAMMA       |                |
|----------------|---||--------------|----------------|

Clock
    X1        - 3.579545MHz
    X2        - 1.056MHz

CPUs
    HD68B09EP - HITACHI 6809E for main
    HD68B09EP - HITACHI 6809E for sub
    LH0080A   - SHARP Z80 for sound

Sound
    YM2151    - YAMAHA FM sound
    YM3012    - YAMAHA DAC for YM2151
    M6295     - OKI ADPCM

PROM
    23J5-0    - user1 (82S129)

ROMs
    23J0-0    - soundcpu EPROM
    23J1-0    - oki mask ROM
    23J2-0    - oki mask ROM
    23J3-0    - maincpu EPROM
    23J4-0    - sub EPROM
    23J6-0    - gfx1 mask ROM

SRAMs
    not listed yet

Others
    VR1       - Speaker volume
    SW1       - DIPSW1
    SW2       - DIPSW2
    JAMMA     - Standard JAMMA connector


TA-0023-P2-03 (Video Board)
|-----------------------------------------------------|
|        |      J2      |     |      J1      |        |
|        ----------------     ----------------        |
|                                                     |
|                                                     |
|        23JB-0                   IC7                 |
|                                                     |
|                                                     |
| X1                                                  |
|                                                     |
|                                                     |
|                                           IC40      |
|                                                     |
|                                        |------------|
|      IC70                              |1           |
|                                        |            |
|                                        |            |
|                         IC78   IC75    |  TRJ-100   |
|                                        |            |
|                                        |            |
|                                        |32          |
|                                        |------------|
|                                                     |
|   IC106     23J7-0  23J8-0  23J9-0  23JA-0          |
|                                                     |
|                                                     |
|                                                     |
|-----------------------------------------------------|

Clock
    X1        - 12MHz

PROM
    23JB-0    - user1 (82S131)

ROMs
    23J7-0    - gfx2 mask ROM
    23J8-0    - gfx2 mask ROM
    23J9-0    - gfx2 mask ROM
    23JA-0    - gfx2 mask ROM
    TJR-100   - gfx3 custom ROM (undumped)

SRAMs (2KBx8bits) Motorola MCM2016HN55, SANYO LC3517?
    IC7       - ?
    IC40      - bgvideoram
    IC70      - ?
    IC75      - ?
    IC78      - ?
    IC106     - ?

Connectors
    J1, J2    - 50pins, almost same assignments with ones for Double Dragon.
                At this moment, 17pin is known to be used for TRJ-100.


TRJ-100 pin assigns
-------------------
Following assignments are estimated based on the circuit around the TRJ-100 in
comparison with one for Double Dragon.
/M2H2 clock is special for this PCB, and M2H is used in Double Dragon instead.
This signal is created by NAND with M2H (1.5MHz) and MH (3MHz).

Following pictures show each clock timing. '%' is the timing to latch AT[7:0]
by these clocks.
        _    ________
/M2H2 - _\__/%  _____\  duty 1:3, 1.5MHz
/M2H  -  \_____/%    \  duty 1:1, 1.5MHz, inverted
          _____
M2H   -  /%    \_____/  duty 1:1, 1.5MHz

 1 - VCC
 2 I /M2H  - inverted 1.5MHz, used to latch AT[7:0] for A[13:6]
 3 I AT0   - connected with bgvideoram d0, used as A6 and A14
 4 I AT1   - connected with bgvideoram d1, used as A7 and A15
 5 I AT2   - connected with bgvideoram d2, used as A8 and A16
 6 I AT3   - connected with bgvideoram d3, used as A9 and BPL0
 7 I AT4   - connected with bgvideoram d4, used as A10 and BPL1
 8 I AT5   - connected with bgvideoram d5, used as A11 and BPL2
 9 I AT6   - connected with bgvideoram d6, used as A12 and BINV
10 I AT7   - connected with bgvideoram d7, used as A13 and BPA
11 I /M2H2 - /(M2H & MH), used to latch AT[7:0] for A[16:14], BPL, BIN, and BPA
12 O BPAL0 - connected with J2 26pin
13 O BPAL1 - connected with J2 24pin
14 O BPAL2 - connected with J2 22pin
15 O BPRT  - connected with J2 20pin
16 I /CE?  - connected with J1 17pin, always LOW as far as it's observed
17 I BHP3  - back horizontal (y) position 0, used to select output
18 I /1P   - screen flip
19 I BHP0  - back horizontal (y) position 1, used to select output
20 I BHP1  - back horizontal (y) position 2, used as A5
21 I BHP2  - back horizontal (y) position 3, used as A6
22 I /HCLK - inverted 6MHZ clock, used as a pixel clock to shift output
23 I BVP0  - back vertical (x) position 0, used as A0
24 I BVP1  - back vertical (x) position 1, used as A1
25 I BVP2  - back vertical (x) position 2, used as A2
26 I BVP3  - back vertical (x) position 3, used as A3
27 O BCOL0 - connected with J2 34pin
28 O BCOL1 - connected with J2 32pin
29 O BCOL2 - connected with J2 30pin
30 O BCOL3 - connected with J2 28pin
31 GND
32 GND
*/

#include "emu.h"

#include "ddragon.h"

#include "cpu/m6809/hd6309.h"
#include "cpu/m6809/m6809.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "sound/ymopn.h"

#include "speaker.h"


namespace {

class chinagat_state : public ddragon_state
{
public:
	chinagat_state(const machine_config &mconfig, device_type type, const char *tag)
		: ddragon_state(mconfig, type, tag)
		, m_adpcm(*this, "adpcm")
		, m_adpcm_rom(*this, "adpcm")
		, m_subbank(*this, "subbank")
	{ }

	void chinagat(machine_config &config);
	void saiyugoub1(machine_config &config);
	void saiyugoub2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// for Sai Yu Gou Ma Roku
	uint32_t m_adpcm_addr = 0;
	uint8_t m_i8748_p1 = 0;
	uint8_t m_i8748_p2 = 0;
	uint8_t m_pcm_shift = 0;
	uint8_t m_pcm_nibble = 0;
	uint8_t m_mcu_command = 0;
#if 0
	uint8_t m_m5205_clk = 0;
#endif

	optional_device<msm5205_device> m_adpcm;
	optional_region_ptr<uint8_t> m_adpcm_rom;
	required_memory_bank m_subbank;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void interrupt_w(offs_t offset, uint8_t data);
	void video_ctrl_w(uint8_t data);
	void bankswitch_w(uint8_t data);
	void sub_bankswitch_w(uint8_t data);
	void sub_irq_ack_w(uint8_t data);
	uint8_t saiyugoub1_mcu_command_r();
	void saiyugoub1_mcu_command_w(uint8_t data);
	void saiyugoub1_adpcm_rom_addr_w(uint8_t data);
	void saiyugoub1_adpcm_control_w(uint8_t data);
	void saiyugoub1_m5205_clk_w(uint8_t data);
	int saiyugoub1_m5205_irq_r();
	void saiyugoub1_m5205_irq_w(int state);

	void i8748_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void saiyugoub1_sound_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
	void ym2203c_sound_map(address_map &map) ATTR_COLD;
};


void chinagat_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(chinagat_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(chinagat_state::background_scan)), 16, 16, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(chinagat_state::get_fg_16color_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scrolldy(-8, -8);
	m_bg_tilemap->set_scrolldy(-8, -8);
}
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

TIMER_DEVICE_CALLBACK_MEMBER(chinagat_state::scanline)
{
	int scanline = param;
	int screen_height = m_screen->height();
	int vcount_old = scanline_to_vcount((scanline == 0) ? screen_height - 1 : scanline - 1);
	int vcount = scanline_to_vcount(scanline);

	// update to the current point
	if (scanline > 0)
		m_screen->update_partial(scanline - 1);

	// on the rising edge of VBLK (vcount == F8), signal an NMI
	if (vcount == 0xf8)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	// set 1ms signal on rising edge of vcount & 8
	if (!(vcount_old & 8) && (vcount & 8))
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);

	// adjust for next scanline
	if (++scanline >= screen_height)
		scanline = 0;
}

void chinagat_state::interrupt_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: // 3e00 - SND irq
			m_soundlatch->write(data);
			break;

		case 1: // 3e01 - NMI ack
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			break;

		case 2: // 3e02 - FIRQ ack
			m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
			break;

		case 3: // 3e03 - IRQ ack
			m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
			break;

		case 4: // 3e04 - sub CPU IRQ ack
			m_subcpu->set_input_line(m_sprite_irq, ASSERT_LINE);
			break;
	}
}

void chinagat_state::video_ctrl_w(uint8_t data)
{
	/***************************
	---- ---x   X Scroll MSB
	---- --x-   Y Scroll MSB
	---- -x--   Flip screen
	--x- ----   Enable video ???
	****************************/
	m_scrolly_hi = ((data & 0x02) >> 1);
	m_scrollx_hi = data & 0x01;

	flip_screen_set(~data & 0x04);
}

void chinagat_state::bankswitch_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x07); // shall we check (data & 7) < 6 (# of banks)?
}

void chinagat_state::sub_bankswitch_w(uint8_t data)
{
	m_subbank->set_entry(data & 0x07); // shall we check (data & 7) < 6 (# of banks)?
}

void chinagat_state::sub_irq_ack_w(uint8_t data)
{
	m_subcpu->set_input_line(m_sprite_irq, CLEAR_LINE);
}

uint8_t chinagat_state::saiyugoub1_mcu_command_r()
{
#if 0
	if (m_mcu_command == 0x78)
	{
		m_mcu->suspend(SUSPEND_REASON_HALT, 1); // Suspend (speed up)
	}
#endif
	return m_mcu_command;
}

void chinagat_state::saiyugoub1_mcu_command_w(uint8_t data)
{
	m_mcu_command = data;
#if 0
	if (data != 0x78)
	{
		m_mcu->resume(SUSPEND_REASON_HALT); // Wake up
	}
#endif
}

void chinagat_state::saiyugoub1_adpcm_rom_addr_w(uint8_t data)
{
	// i8748 Port 1 write
	m_i8748_p1 = data;
}

void chinagat_state::saiyugoub1_adpcm_control_w(uint8_t data)
{
	// i8748 Port 2 write
	if (data & 0x80)    // Reset m5205 and disable ADPCM ROM outputs
	{
		logerror("ADPCM output disabled\n");
		m_pcm_nibble = 0x0f;
		m_adpcm->reset_w(1);
	}
	else
	{
		if ((m_i8748_p2 & 0xc) != (data & 0xc))
		{
			if ((m_i8748_p2 & 0xc) == 0) // Latch MSB Address
			{
//             logerror("Latching MSB\n");
				m_adpcm_addr = (m_adpcm_addr & 0x3807f) | (m_i8748_p1 << 7);
			}
			if ((m_i8748_p2 & 0xc) == 4) // Latch LSB Address
			{
//             logerror("Latching LSB\n");
				m_adpcm_addr = (m_adpcm_addr & 0x3ff80) | (m_i8748_p1 >> 1);
				m_pcm_shift = (m_i8748_p1 & 1) * 4;
			}
		}

		m_adpcm_addr = ((m_adpcm_addr & 0x07fff) | ((data & 0x70) << 11));

		m_pcm_nibble = m_adpcm_rom[m_adpcm_addr & 0x3ffff];

		m_pcm_nibble = (m_pcm_nibble >> m_pcm_shift) & 0x0f;

//      logerror("Writing %02x to m5205. $ROM=%08x  P1=%02x  P2=%02x  Prev_P2=%02x  Nibble=%08x\n", m_pcm_nibble, m_adpcm_addr, m_i8748_p1, data, m_i8748_p2, m_pcm_shift);

		if (((m_i8748_p2 & 0xc) >= 8) && ((data & 0xc) == 4))
		{
			m_adpcm->data_w(m_pcm_nibble);
			logerror("Writing %02x to m5205\n", m_pcm_nibble);
		}
		logerror("$ROM=%08x  P1=%02x  P2=%02x  Prev_P2=%02x  Nibble=%1x  PCM_data=%02x\n", m_adpcm_addr, m_i8748_p1, data, m_i8748_p2, m_pcm_shift, m_pcm_nibble);
	}
	m_i8748_p2 = data;
}

[[maybe_unused]] void chinagat_state::saiyugoub1_m5205_clk_w(uint8_t data)
{
	/* i8748 T0 output clk mode
	   This signal goes through a divide by 8 counter
	   to the xtal pins of the MSM5205 */

#if 0
	m_m5205_clk++;
	if (m_m5205_clk == 8)
	{
		m_adpcm->vclk_w(1);      // ???
		m_m5205_clk = 0;
	}
	else
		m_adpcm->vclk_w(0);      // ???
#endif
}

int chinagat_state::saiyugoub1_m5205_irq_r()
{
	if (m_adpcm_sound_irq)
	{
		m_adpcm_sound_irq = 0;
		return 1;
	}
	return 0;
}

void chinagat_state::saiyugoub1_m5205_irq_w(int state)
{
	m_adpcm_sound_irq = 1;
}

void chinagat_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("share1");
	map(0x2000, 0x27ff).ram().w(FUNC(chinagat_state::fgvideoram_w)).share(m_fgvideoram);
	map(0x2800, 0x2fff).ram().w(FUNC(chinagat_state::bgvideoram_w)).share(m_bgvideoram);
	map(0x3000, 0x317f).w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x3400, 0x357f).w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x3800, 0x397f).ram().share(m_spriteram);
	map(0x3e00, 0x3e04).w(FUNC(chinagat_state::interrupt_w));
	map(0x3e06, 0x3e06).writeonly().share(m_scrolly_lo);
	map(0x3e07, 0x3e07).writeonly().share(m_scrollx_lo);
	map(0x3f00, 0x3f00).w(FUNC(chinagat_state::video_ctrl_w));
	map(0x3f01, 0x3f01).w(FUNC(chinagat_state::bankswitch_w));
	map(0x3f00, 0x3f00).portr("SYSTEM");
	map(0x3f01, 0x3f01).portr("DSW1");
	map(0x3f02, 0x3f02).portr("DSW2");
	map(0x3f03, 0x3f03).portr("P1");
	map(0x3f04, 0x3f04).portr("P2");
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}

void chinagat_state::sub_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("share1");
	map(0x2000, 0x2000).w(FUNC(chinagat_state::sub_bankswitch_w));
	map(0x2800, 0x2800).w(FUNC(chinagat_state::sub_irq_ack_w)); // Called on CPU start and after return from jump table
//  map(0x2a2b, 0x2a2b).nopr(); // What lives here?
//  map(0x2a30, 0x2a30).nopr(); // What lives here?
	map(0x4000, 0x7fff).bankr(m_subbank);
	map(0x8000, 0xffff).rom();
}

void chinagat_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x9800, 0x9800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xA000, 0xA000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void chinagat_state::ym2203c_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
// 8804 and/or 8805 make a gong sound when the coin goes in
// but only on the title screen....

	map(0x8800, 0x8801).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
//  map(0x8802, 0x8802).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
//  map(0x8803, 0x8803).w("oki", FUNC(okim6295_device::write));
	map(0x8804, 0x8805).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
//  map(0x8804, 0x8804).writeonly();
//  map(0x8805, 0x8805).writeonly();

//  map(0x8800, 0x8801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
//  map(0x9800, 0x9800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xA000, 0xA000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void chinagat_state::saiyugoub1_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x9800, 0x9800).w(FUNC(chinagat_state::saiyugoub1_mcu_command_w));
	map(0xA000, 0xA000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void chinagat_state::i8748_map(address_map &map)
{
	map(0x0000, 0x03ff).rom();
	map(0x0400, 0x07ff).rom();     // i8749 version
}



static INPUT_PORTS_START( chinagat )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	/*PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )*/
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // "SW2:4" - Left empty in the manual
	PORT_DIPNAME( 0x30, 0x20, "Timer" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x20, "55" )
	PORT_DIPSETTING(    0x30, "60" )
	PORT_DIPSETTING(    0x10, "70" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x40, "4" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,            // 8*8 chars
	RGN_FRAC(1,1),  // num of characters
	4,              // 4 bits per pixel
	{ 0, 2, 4, 6 },     // plane offset
	{ 1, 0, 65, 64, 129, 128, 193, 192 },
	{ STEP8(0,8) },
	32*8 //* every char takes 32 consecutive bytes
};

static const gfx_layout tilelayout =
{
	16,16,          // 16x16 chars
	RGN_FRAC(1,2),  // num of Tiles/Sprites
	4,              // 4 bits per pixel
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0,4 }, // plane offset
	{ 3, 2, 1, 0, 16*8+3, 16*8+2, 16*8+1, 16*8+0,
		32*8+3,32*8+2 ,32*8+1 ,32*8+0 ,48*8+3 ,48*8+2 ,48*8+1 ,48*8+0 },
	{ STEP16(0,8) },
	64*8 // every char takes 64 consecutive bytes
};

static GFXDECODE_START( gfx_chinagat )
	GFXDECODE_ENTRY( "chars",   0, charlayout,   0,16 )    // 8x8
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 128, 8 )    // 16x16
	GFXDECODE_ENTRY( "tiles",   0, tilelayout, 256, 8 )    // 16x16
GFXDECODE_END


void chinagat_state::machine_start()
{
	m_technos_video_hw = 1;
	m_sprite_irq = M6809_IRQ_LINE;

	// configure banks
	m_mainbank->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x4000);
	m_subbank->configure_entries(0, 8, memregion("sub")->base() + 0x10000, 0x4000);

	// register for save states
	save_item(NAME(m_scrollx_hi));
	save_item(NAME(m_scrolly_hi));
	save_item(NAME(m_adpcm_sound_irq));
	save_item(NAME(m_adpcm_addr));
	save_item(NAME(m_pcm_shift));
	save_item(NAME(m_pcm_nibble));
	save_item(NAME(m_i8748_p1));
	save_item(NAME(m_i8748_p2));
	save_item(NAME(m_mcu_command));
#if 0
	save_item(NAME(m_m5205_clk));
#endif
}


void chinagat_state::machine_reset()
{
	m_scrollx_hi = 0;
	m_scrolly_hi = 0;
	m_adpcm_sound_irq = 0;
	m_adpcm_addr = 0;
	m_pcm_shift = 0;
	m_pcm_nibble = 0;
	m_i8748_p1 = 0;
	m_i8748_p2 = 0;
	m_mcu_command = 0;
#if 0
	m_m5205_clk = 0;
#endif
}


constexpr XTAL MAIN_CLOCK = 12_MHz_XTAL;
constexpr XTAL PIXEL_CLOCK = MAIN_CLOCK / 2;

void chinagat_state::chinagat(machine_config &config)
{
	// basic machine hardware
	HD6309(config, m_maincpu, MAIN_CLOCK / 2);      // 1.5 MHz (12MHz oscillator / 4 internally)
	m_maincpu->set_addrmap(AS_PROGRAM, &chinagat_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(chinagat_state::scanline), "screen", 0, 1);

	HD6309(config, m_subcpu, MAIN_CLOCK / 2);       // 1.5 MHz (12MHz oscillator / 4 internally)
	m_subcpu->set_addrmap(AS_PROGRAM, &chinagat_state::sub_map);

	Z80(config, m_soundcpu, 3.579545_MHz_XTAL);
	m_soundcpu->set_addrmap(AS_PROGRAM, &chinagat_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(6000)); // heavy interleaving to sync up sprite<->main CPUs

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, 384, 0, 256, 272, 0, 240);   // based on ddragon driver
	m_screen->set_screen_update(FUNC(chinagat_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_chinagat);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 384);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3.579545_MHz_XTAL));
	ymsnd.irq_handler().set_inputline(m_soundcpu, 0);
	ymsnd.add_route(0, "mono", 0.80);
	ymsnd.add_route(1, "mono", 0.80);

	okim6295_device &oki(OKIM6295(config, "oki", 1'065'000, okim6295_device::PIN7_HIGH)); // pin 7 not verified, clock frequency estimated with recording
	oki.add_route(ALL_OUTPUTS, "mono", 0.80);
}

void chinagat_state::saiyugoub1(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, MAIN_CLOCK / 8);     // 68B09EP 1.5 MHz (12MHz oscillator)
	m_maincpu->set_addrmap(AS_PROGRAM, &chinagat_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(chinagat_state::scanline), "screen", 0, 1);

	MC6809E(config, m_subcpu, MAIN_CLOCK / 8);      // 68B09EP 1.5 MHz (12MHz oscillator)
	m_subcpu->set_addrmap(AS_PROGRAM, &chinagat_state::sub_map);

	Z80(config, m_soundcpu, 3.579545_MHz_XTAL);
	m_soundcpu->set_addrmap(AS_PROGRAM, &chinagat_state::saiyugoub1_sound_map);

	i8748_device &mcu(I8748(config, "mcu", 9.263750_MHz_XTAL));     // divided by 3*5 internally
	mcu.set_addrmap(AS_PROGRAM, &chinagat_state::i8748_map);
	mcu.bus_in_cb().set(FUNC(chinagat_state::saiyugoub1_mcu_command_r));
	//mcu.set_t0_clk_cb(FUNC(chinagat_state::saiyugoub1_m5205_clk_w));      // Drives the clock on the m5205 at 1/8 of this frequency
	mcu.t1_in_cb().set(FUNC(chinagat_state::saiyugoub1_m5205_irq_r));
	mcu.p1_out_cb().set(FUNC(chinagat_state::saiyugoub1_adpcm_rom_addr_w));
	mcu.p2_out_cb().set(FUNC(chinagat_state::saiyugoub1_adpcm_control_w));

	config.set_maximum_quantum(attotime::from_hz(6000));  // heavy interleaving to sync up sprite<->main CPU's

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, 384, 0, 256, 272, 0, 240);   // based on ddragon driver
	m_screen->set_screen_update(FUNC(chinagat_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_chinagat);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 384);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3.579545_MHz_XTAL));
	ymsnd.irq_handler().set_inputline(m_soundcpu, 0);
	ymsnd.add_route(0, "mono", 0.80);
	ymsnd.add_route(1, "mono", 0.80);

	MSM5205(config, m_adpcm, 9.263750_MHz_XTAL / 24);
	m_adpcm->vck_legacy_callback().set(FUNC(chinagat_state::saiyugoub1_m5205_irq_w)); // Interrupt function
	m_adpcm->set_prescaler_selector(msm5205_device::S64_4B);    // vclk input mode (6030Hz, 4-bit)
	m_adpcm->add_route(ALL_OUTPUTS, "mono", 0.60);
}

void chinagat_state::saiyugoub2(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, MAIN_CLOCK / 8);     // 1.5 MHz (12MHz oscillator)
	m_maincpu->set_addrmap(AS_PROGRAM, &chinagat_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(chinagat_state::scanline), "screen", 0, 1);

	MC6809E(config, m_subcpu, MAIN_CLOCK / 8);      // 1.5 MHz (12MHz oscillator)
	m_subcpu->set_addrmap(AS_PROGRAM, &chinagat_state::sub_map);

	Z80(config, m_soundcpu, 3.579545_MHz_XTAL);
	m_soundcpu->set_addrmap(AS_PROGRAM, &chinagat_state::ym2203c_sound_map);

	config.set_maximum_quantum(attotime::from_hz(6000)); // heavy interleaving to sync up sprite<->main CPU's

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, 384, 0, 256, 272, 0, 240);   // based on ddragon driver
	m_screen->set_screen_update(FUNC(chinagat_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_chinagat);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 384);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);

	ym2203_device &ym1(YM2203(config, "ym1", 3.579545_MHz_XTAL));
	ym1.irq_handler().set_inputline(m_soundcpu, 0);
	ym1.add_route(0, "mono", 0.50);
	ym1.add_route(1, "mono", 0.50);
	ym1.add_route(2, "mono", 0.50);
	ym1.add_route(3, "mono", 0.80);

	ym2203_device &ym2(YM2203(config, "ym2", 3.579545_MHz_XTAL));
	ym2.add_route(0, "mono", 0.50);
	ym2.add_route(1, "mono", 0.50);
	ym2.add_route(2, "mono", 0.50);
	ym2.add_route(3, "mono", 0.80);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( chinagat )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "cgate51.bin", 0x10000, 0x18000, CRC(439a3b19) SHA1(01393b4302ac7a66390270b01e2757582240f6b8) )   // Banks 0x4000 long @ 0x4000
	ROM_CONTINUE(            0x08000, 0x08000 )             // Static code

	ROM_REGION( 0x28000, "sub", 0 )
	ROM_LOAD( "23j4-0.48",   0x10000, 0x18000, CRC(2914af38) SHA1(3d690fa50b7d36a22de82c026d59a16126a7b73c) ) // Banks 0x4000 long @ 0x4000
	ROM_CONTINUE(            0x08000, 0x08000 )             // Static code

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "23j0-0.40",   0x00000, 0x08000, CRC(9ffcadb6) SHA1(606dbdd73aee3cabb2142200ac6f8c96169e4b19) )

	ROM_REGION(0x20000, "chars", 0 )
	ROM_LOAD( "cgate18.bin", 0x00000, 0x20000, CRC(8d88d64d) SHA1(57265138ebb0c6419542cce5953aee7335bfa2bd) )   // 0,1,2,3

	ROM_REGION(0x80000, "sprites", 0 )
	ROM_LOAD( "23j7-0.103",  0x00000, 0x20000, CRC(2f445030) SHA1(3fcf32097e655e963d952d01a30396dc195269ca) )   // 2,3
	ROM_LOAD( "23j8-0.102",  0x20000, 0x20000, CRC(237f725a) SHA1(47bebe5b9878ca10fe6efd4f353717e53a372416) )   // 2,3
	ROM_LOAD( "23j9-0.101",  0x40000, 0x20000, CRC(8caf6097) SHA1(50ad192f831b055586a4a9974f8c6c2f2063ede5) )   // 0,1
	ROM_LOAD( "23ja-0.100",  0x60000, 0x20000, CRC(f678594f) SHA1(4bdcf9407543925f4630a8c7f1f48b85f76343a9) )   // 0,1

	ROM_REGION(0x40000, "tiles", 0 )
	ROM_LOAD( "chinagat_a-13", 0x00000, 0x10000, BAD_DUMP CRC(b745cac4) SHA1(759767ca7c5123b03b9e1a42bb105d194cb76400) ) // not dumped yet, these were taken from the bootleg set instead
	ROM_LOAD( "chinagat_a-12", 0x10000, 0x10000, BAD_DUMP CRC(3c864299) SHA1(cb12616e4d6c53a82beb4cd51510a632894b359c) ) // TRJ-100 should contain it, but not dumped yet.
	ROM_LOAD( "chinagat_a-15", 0x20000, 0x10000, BAD_DUMP CRC(2f268f37) SHA1(f82cfe3b2001d5ed2a709ca9c51febcf624bb627) )
	ROM_LOAD( "chinagat_a-14", 0x30000, 0x10000, BAD_DUMP CRC(aef814c8) SHA1(f6b9229ca7beb9a0e47d1f6a1083c6102fdd20c8) )

	ROM_REGION(0x40000, "oki", 0 )
	ROM_LOAD( "23j1-0.53", 0x00000, 0x20000, CRC(f91f1001) SHA1(378402a3c966cabd61e9662ae5decd66672a228b) )
	ROM_LOAD( "23j2-0.52", 0x20000, 0x20000, CRC(8b6f26e9) SHA1(7da26ae846814b3957b19c38b6bf7e83617dc6cc) )

	ROM_REGION(0x300, "proms", 0 )  // Unknown function
	ROM_LOAD( "23jb-0.16", 0x000, 0x200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) ) // 82S131 on video board
	ROM_LOAD( "23j5-0.45", 0x200, 0x100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) ) // 82S129 on main board
ROM_END


ROM_START( saiyugou )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "23j3-0.51",  0x10000, 0x18000, CRC(aa8132a2) SHA1(87c3bd447767f263113c4865afc905a0e484a625) )    // Banks 0x4000 long @ 0x4000
	ROM_CONTINUE(           0x08000, 0x08000)               // Static code

	ROM_REGION( 0x28000, "sub", 0 )
	ROM_LOAD( "23j4-0.48",  0x10000, 0x18000, CRC(2914af38) SHA1(3d690fa50b7d36a22de82c026d59a16126a7b73c) )    // Banks 0x4000 long @ 0x4000
	ROM_CONTINUE(           0x08000, 0x08000)               // Static code

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "23j0-0.40",  0x00000, 0x8000, CRC(9ffcadb6) SHA1(606dbdd73aee3cabb2142200ac6f8c96169e4b19) )

	ROM_REGION(0x20000, "chars", 0 )
	ROM_LOAD( "23j6-0.18",  0x00000, 0x20000, CRC(86d33df0) SHA1(3419959c28703c5177de9c11b61e1dba9e76aca5) )    // 0,1,2,3

	ROM_REGION(0x80000, "sprites", 0 )
	ROM_LOAD( "23j7-0.103", 0x00000, 0x20000, CRC(2f445030) SHA1(3fcf32097e655e963d952d01a30396dc195269ca) )    // 2,3
	ROM_LOAD( "23j8-0.102", 0x20000, 0x20000, CRC(237f725a) SHA1(47bebe5b9878ca10fe6efd4f353717e53a372416) )    // 2,3
	ROM_LOAD( "23j9-0.101", 0x40000, 0x20000, CRC(8caf6097) SHA1(50ad192f831b055586a4a9974f8c6c2f2063ede5) )    // 0,1
	ROM_LOAD( "23ja-0.100", 0x60000, 0x20000, CRC(f678594f) SHA1(4bdcf9407543925f4630a8c7f1f48b85f76343a9) )    // 0,1

	ROM_REGION(0x40000, "tiles", 0 )
	ROM_LOAD( "saiyugou_a-13", 0x00000, 0x10000, BAD_DUMP CRC(b745cac4) SHA1(759767ca7c5123b03b9e1a42bb105d194cb76400) ) // not dumped yet, these were taken from the bootleg set instead
	ROM_LOAD( "saiyugou_a-12", 0x10000, 0x10000, BAD_DUMP CRC(3c864299) SHA1(cb12616e4d6c53a82beb4cd51510a632894b359c) ) // TRJ-100 should contain it, but not dumped yet.
	ROM_LOAD( "saiyugou_a-15", 0x20000, 0x10000, BAD_DUMP CRC(2f268f37) SHA1(f82cfe3b2001d5ed2a709ca9c51febcf624bb627) )
	ROM_LOAD( "saiyugou_a-14", 0x30000, 0x10000, BAD_DUMP CRC(aef814c8) SHA1(f6b9229ca7beb9a0e47d1f6a1083c6102fdd20c8) )

	ROM_REGION(0x40000, "oki", 0 )
	ROM_LOAD( "23j1-0.53", 0x00000, 0x20000, CRC(f91f1001) SHA1(378402a3c966cabd61e9662ae5decd66672a228b) )
	ROM_LOAD( "23j2-0.52", 0x20000, 0x20000, CRC(8b6f26e9) SHA1(7da26ae846814b3957b19c38b6bf7e83617dc6cc) )

	ROM_REGION(0x300, "proms", 0 )  // Unknown function
	ROM_LOAD( "23jb-0.16", 0x000, 0x200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) ) // 82S131 on video board
	ROM_LOAD( "23j5-0.45", 0x200, 0x100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) ) // 82S129 on main board
ROM_END

ROM_START( saiyugoub1 )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "23j3-0.51",  0x10000, 0x18000, CRC(aa8132a2) SHA1(87c3bd447767f263113c4865afc905a0e484a625) )    // Banks 0x4000 long @ 0x4000
	/* Orientation of bootleg ROMs which are split, but otherwise the same.
	   ROM_LOAD( "a-5.bin", 0x10000, 0x10000, CRC(39795aa5) SHA1(475dc547b823436b25f3bdff22434e3898c23d9f) )    Banks 0x4000 long @ 0x4000
	   ROM_LOAD( "a-9.bin", 0x20000, 0x08000, CRC(051ebe92) SHA1(f3d179e7794f18aa65f24422364c1d71735fcc29) )     Banks 0x4000 long @ 0x4000
	*/
	ROM_CONTINUE(           0x08000, 0x08000 )              // Static code

	ROM_REGION( 0x28000, "sub", 0 )
	ROM_LOAD( "23j4-0.48",  0x10000, 0x18000, CRC(2914af38) SHA1(3d690fa50b7d36a22de82c026d59a16126a7b73c) )    // Banks 0x4000 long @ 0x4000
	/* Orientation of bootleg ROMs which are split, but otherwise the same.
	   ROM_LOAD( "a-4.bin", 0x10000, 0x10000, CRC(9effddc1) SHA1(f4d336991ba73241c683a12c5949f8929fcaae14) )     Banks 0x4000 long @ 0x4000
	   ROM_LOAD( "a-8.bin", 0x20000, 0x08000, CRC(a436edb8) SHA1(f6504bcfe6dd9d756bcf5443fb702a7c82408ea9) )     Banks 0x4000 long @ 0x4000
	*/
	ROM_CONTINUE(           0x08000, 0x08000 )              // Static code

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "a-1.bin",  0x00000, 0x8000,  CRC(46e5a6d4) SHA1(965ed7bdb727ab32ce3322ca49f1a4e3786e8051) )

	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "mcu8748.bin", 0x000, 0x400, CRC(6d28d6c5) SHA1(20582c62a72545e68c2e155b063ee7e95e1228ce) )

	ROM_REGION(0x20000, "chars", 0 )
	ROM_LOAD( "23j6-0.18",  0x00000, 0x20000, CRC(86d33df0) SHA1(3419959c28703c5177de9c11b61e1dba9e76aca5) )    // 0,1,2,3
	/* Orientation of bootleg ROMs which are split, but otherwise the same.
	   ROM_LOAD( "a-2.bin", 0x00000, 0x10000, CRC(baa5a3b9) SHA1(073685f4c9dbe90480cf5debea999ae3d7d49346) )     0,1
	   ROM_LOAD( "a-3.bin", 0x10000, 0x10000, CRC(532d59be) SHA1(48d7cf73362d019a5d9a8e1669c86ef52307bad1) )     2,3
	*/

	ROM_REGION(0x80000, "sprites", 0 )
	ROM_LOAD( "23j7-0.103",  0x00000, 0x20000, CRC(2f445030) SHA1(3fcf32097e655e963d952d01a30396dc195269ca) )   // 2,3
	ROM_LOAD( "23j8-0.102",  0x20000, 0x20000, CRC(237f725a) SHA1(47bebe5b9878ca10fe6efd4f353717e53a372416) )   // 2,3
	ROM_LOAD( "23j9-0.101",  0x40000, 0x20000, CRC(8caf6097) SHA1(50ad192f831b055586a4a9974f8c6c2f2063ede5) )   // 0,1
	ROM_LOAD( "23ja-0.100",  0x60000, 0x20000, CRC(f678594f) SHA1(4bdcf9407543925f4630a8c7f1f48b85f76343a9) )   // 0,1
	/* Orientation of bootleg ROMs which are split, but otherwise the same
	   ROM_LOAD( "a-23.bin", 0x00000, 0x10000, CRC(12b56225) SHA1(cc1617b92194f40dd343d83a98165912553215d9) )    2,3
	   ROM_LOAD( "a-22.bin", 0x10000, 0x10000, CRC(b592aa9b) SHA1(c04dcda040e7598ebc90bd0e0ba7117c2fcc7f4b) )    2,3
	   ROM_LOAD( "a-21.bin", 0x20000, 0x10000, CRC(a331ba3d) SHA1(c7a8c5f10031b0ffcb4bb5bf73e5edfb0013373d) )    2,3
	   ROM_LOAD( "a-20.bin", 0x30000, 0x10000, CRC(2515d742) SHA1(eada6a8dcd19dc380a097e8a312822abdf01f9b9) )    2,3
	   ROM_LOAD( "a-19.bin", 0x40000, 0x10000, CRC(d796f2e4) SHA1(8e50b117e64160e59f7c55f7fb57cde32f4c0030) )    0,1
	   ROM_LOAD( "a-18.bin", 0x50000, 0x10000, CRC(c9e1c2f9) SHA1(5db992822fd5458a76861763ae661b7c6f22b9c7) )    0,1
	   ROM_LOAD( "a-17.bin", 0x60000, 0x10000, CRC(00b6db0a) SHA1(3219233159c1ce350bb572a43fe66836e67e72d7) )    0,1
	   ROM_LOAD( "a-16.bin", 0x70000, 0x10000, CRC(f196818b) SHA1(f4a27e90720094f6a06b6b7f1dad7be25de8e9ba) )    0,1
	*/

	ROM_REGION(0x40000, "tiles", 0 )
	ROM_LOAD( "a-13", 0x00000, 0x10000, CRC(b745cac4) SHA1(759767ca7c5123b03b9e1a42bb105d194cb76400) )
	ROM_LOAD( "a-12", 0x10000, 0x10000, CRC(3c864299) SHA1(cb12616e4d6c53a82beb4cd51510a632894b359c) )
	ROM_LOAD( "a-15", 0x20000, 0x10000, CRC(2f268f37) SHA1(f82cfe3b2001d5ed2a709ca9c51febcf624bb627) )
	ROM_LOAD( "a-14", 0x30000, 0x10000, CRC(aef814c8) SHA1(f6b9229ca7beb9a0e47d1f6a1083c6102fdd20c8) )

	/* Some bootlegs have incorrectly halved the ADPCM data !
	   These are same as the 128k sample except nibble-swapped */
	ROM_REGION(0x40000, "adpcm", 0 )       // Bootleggers wrong data
	ROM_LOAD ( "a-6.bin",   0x00000, 0x10000, CRC(4da4e935) SHA1(235a1589165a23cfad29e07cf66d7c3a777fc904) )    // 0x8000, 0x7cd47f01
	ROM_LOAD ( "a-7.bin",   0x10000, 0x10000, CRC(6284c254) SHA1(e01be1bd4768ae0ccb1cec65b3a6bc80ed7a4b00) )    // 0x8000, 0x7091959c
	ROM_LOAD ( "a-10.bin",  0x20000, 0x10000, CRC(b728ec6e) SHA1(433b5f907e4918e89b79bd927e2993ad3030017b) )    // 0x8000, 0x78349cb6
	ROM_LOAD ( "a-11.bin",  0x30000, 0x10000, CRC(a50d1895) SHA1(0c2c1f8a2e945d6c53ce43413f0e63ced45bae17) )    // 0x8000, 0xaa5b6834

	ROM_REGION(0x300, "proms", 0 )  // Unknown function
	ROM_LOAD( "23jb-0.16", 0x000, 0x200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) ) // 82S131 on video board
	ROM_LOAD( "23j5-0.45", 0x200, 0x100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) ) // 82S129 on main board
ROM_END

ROM_START( saiyugoub2 )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "23j3-0.51",   0x10000, 0x18000, CRC(aa8132a2) SHA1(87c3bd447767f263113c4865afc905a0e484a625) )   // Banks 0x4000 long @ 0x4000
	/* Orientation of bootleg ROMs which are split, but otherwise the same.
	   ROM_LOAD( "sai5.bin", 0x10000, 0x10000, CRC(39795aa5) SHA1(475dc547b823436b25f3bdff22434e3898c23d9f) )    Banks 0x4000 long @ 0x4000
	   ROM_LOAD( "sai9.bin", 0x20000, 0x08000, CRC(051ebe92) SHA1(f3d179e7794f18aa65f24422364c1d71735fcc29) )    Banks 0x4000 long @ 0x4000
	*/
	ROM_CONTINUE(            0x08000, 0x08000 )             // Static code

	ROM_REGION( 0x28000, "sub", 0 )
	ROM_LOAD( "23j4-0.48", 0x10000, 0x18000, CRC(2914af38) SHA1(3d690fa50b7d36a22de82c026d59a16126a7b73c) ) // Banks 0x4000 long @ 0x4000
	/* Orientation of bootleg ROMs which are split, but otherwise the same.
	   ROM_LOAD( "sai4.bin", 0x10000, 0x10000, CRC(9effddc1) SHA1(f4d336991ba73241c683a12c5949f8929fcaae14) )    Banks 0x4000 long @ 0x4000
	   ROM_LOAD( "sai8.bin", 0x20000, 0x08000, CRC(a436edb8) SHA1(f6504bcfe6dd9d756bcf5443fb702a7c82408ea9) )    Banks 0x4000 long @ 0x4000
	*/
	ROM_CONTINUE(         0x08000, 0x08000 )                // Static code

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sai-alt1.bin", 0x00000, 0x8000, CRC(8d397a8d) SHA1(52599521c3dbcecc1ae56bb80dc855e76d700134) )

//  ROM_REGION( 0x800, "cpu3", 0 )
//  ROM_LOAD( "sgr-8749.bin", 0x000, 0x800, CRC(9237e8c5) ) // same as above but padded with 00 for different MCU

	ROM_REGION(0x20000, "chars", 0 )
	ROM_LOAD( "23j6-0.18", 0x00000, 0x20000, CRC(86d33df0) SHA1(3419959c28703c5177de9c11b61e1dba9e76aca5) ) // 0,1,2,3
	/* Orientation of bootleg ROMs which are split, but otherwise the same.
	   ROM_LOAD( "sai2.bin", 0x00000, 0x10000, CRC(baa5a3b9) SHA1(073685f4c9dbe90480cf5debea999ae3d7d49346) )    0,1
	   ROM_LOAD( "sai3.bin", 0x10000, 0x10000, CRC(532d59be) SHA1(48d7cf73362d019a5d9a8e1669c86ef52307bad1) )    2,3
	*/

	ROM_REGION(0x80000, "sprites", 0 )
	ROM_LOAD( "23j7-0.103",   0x00000, 0x20000, CRC(2f445030) SHA1(3fcf32097e655e963d952d01a30396dc195269ca) )  // 2,3
	ROM_LOAD( "23j8-0.102",   0x20000, 0x20000, CRC(237f725a) SHA1(47bebe5b9878ca10fe6efd4f353717e53a372416) )  // 2,3
	ROM_LOAD( "23j9-0.101",   0x40000, 0x20000, CRC(8caf6097) SHA1(50ad192f831b055586a4a9974f8c6c2f2063ede5) )  // 0,1
	ROM_LOAD( "23ja-0.100",   0x60000, 0x20000, CRC(f678594f) SHA1(4bdcf9407543925f4630a8c7f1f48b85f76343a9) )  // 0,1
	/* Orientation of bootleg ROMs which are split, but otherwise the same
	   ROM_LOAD( "sai23.bin", 0x00000, 0x10000, CRC(12b56225) SHA1(cc1617b92194f40dd343d83a98165912553215d9) )   2,3
	   ROM_LOAD( "sai22.bin", 0x10000, 0x10000, CRC(b592aa9b) SHA1(c04dcda040e7598ebc90bd0e0ba7117c2fcc7f4b) )   2,3
	   ROM_LOAD( "sai21.bin", 0x20000, 0x10000, CRC(a331ba3d) SHA1(c7a8c5f10031b0ffcb4bb5bf73e5edfb0013373d) )   2,3
	   ROM_LOAD( "sai20.bin", 0x30000, 0x10000, CRC(2515d742) SHA1(eada6a8dcd19dc380a097e8a312822abdf01f9b9) )   2,3
	   ROM_LOAD( "sai19.bin", 0x40000, 0x10000, CRC(d796f2e4) SHA1(8e50b117e64160e59f7c55f7fb57cde32f4c0030) )   0,1
	   ROM_LOAD( "sai18.bin", 0x50000, 0x10000, CRC(c9e1c2f9) SHA1(5db992822fd5458a76861763ae661b7c6f22b9c7) )   0,1
	   ROM_LOAD( "roku17.bin",0x60000, 0x10000, CRC(00b6db0a) SHA1(3219233159c1ce350bb572a43fe66836e67e72d7) )   0,1
	   ROM_LOAD( "sai16.bin", 0x70000, 0x10000, CRC(f196818b) SHA1(f4a27e90720094f6a06b6b7f1dad7be25de8e9ba) )   0,1
	*/

	ROM_REGION(0x40000, "tiles", 0 )
	ROM_LOAD( "a-13", 0x00000, 0x10000, CRC(b745cac4) SHA1(759767ca7c5123b03b9e1a42bb105d194cb76400) )
	ROM_LOAD( "a-12", 0x10000, 0x10000, CRC(3c864299) SHA1(cb12616e4d6c53a82beb4cd51510a632894b359c) )
	ROM_LOAD( "a-15", 0x20000, 0x10000, CRC(2f268f37) SHA1(f82cfe3b2001d5ed2a709ca9c51febcf624bb627) )
	ROM_LOAD( "a-14", 0x30000, 0x10000, CRC(aef814c8) SHA1(f6b9229ca7beb9a0e47d1f6a1083c6102fdd20c8) )

	ROM_REGION(0x40000, "adpcm", 0 )
	/* These are same as the 128k sample except nibble-swapped
	   Some bootlegs have incorrectly halved the ADPCM data !  Bootleggers wrong data */
	ROM_LOAD ( "a-6.bin",   0x00000, 0x10000, CRC(4da4e935) SHA1(235a1589165a23cfad29e07cf66d7c3a777fc904) )    // 0x8000, 0x7cd47f01
	ROM_LOAD ( "a-7.bin",   0x10000, 0x10000, CRC(6284c254) SHA1(e01be1bd4768ae0ccb1cec65b3a6bc80ed7a4b00) )    // 0x8000, 0x7091959c
	ROM_LOAD ( "a-10.bin",  0x20000, 0x10000, CRC(b728ec6e) SHA1(433b5f907e4918e89b79bd927e2993ad3030017b) )    // 0x8000, 0x78349cb6
	ROM_LOAD ( "a-11.bin",  0x30000, 0x10000, CRC(a50d1895) SHA1(0c2c1f8a2e945d6c53ce43413f0e63ced45bae17) )    // 0x8000, 0xaa5b6834

	ROM_REGION(0x300, "proms", 0 )  // Unknown function
	ROM_LOAD( "23jb-0.16", 0x000, 0x200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) ) // 82S131 on video board
	ROM_LOAD( "23j5-0.45", 0x200, 0x100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) ) // 82S129 on main board
ROM_END

} // anonymous namespace


//  ( YEAR  NAME        PARENT    MACHINE     INPUT     STATE           INIT        MONITOR COMPANY                                    FULLNAME                                FLAGS )
GAME( 1988, chinagat,   0,        chinagat,   chinagat, chinagat_state, empty_init, ROT0,   "Technos Japan (Taito / Romstar license)", "China Gate (US)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1988, saiyugou,   chinagat, chinagat,   chinagat, chinagat_state, empty_init, ROT0,   "Technos Japan",                           "Sai Yu Gou Ma Roku (Japan)",           MACHINE_SUPPORTS_SAVE )
GAME( 1988, saiyugoub1, chinagat, saiyugoub1, chinagat, chinagat_state, empty_init, ROT0,   "bootleg",                                 "Sai Yu Gou Ma Roku (Japan bootleg 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1988, saiyugoub2, chinagat, saiyugoub2, chinagat, chinagat_state, empty_init, ROT0,   "bootleg",                                 "Sai Yu Gou Ma Roku (Japan bootleg 2)", MACHINE_SUPPORTS_SAVE )
