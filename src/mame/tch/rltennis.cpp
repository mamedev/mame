// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, Marcos Hernández
/**************************************************************************************************
Reality Tennis - (c) 1993 TCH

Original driver by Tomasz Slanina.
The blitter's scaling, 8-bit spans, (1,1) origin and blit timing implemented from the game's use
of the hardware by Marcos Hernández, in collaboration, within MS-DOS Club's TCH preservation
project (github.com/MS-DOS-Club/tch-blitter).

    based on informations provided by Antonio 'Peluko' Carrillo

Game Credits:
Antonio 'Peluko' Carrillo: programmer and game designer
David Sandoval: hardware designer

PCB Layout
----------

|-----------------------------------------------------|
|  |-----|  KM424C257  tennis_6  tennis_12     -      |
|  |Bt478|  KM424C257                                 |
|  |-----|             tennis_5  tennis_11     -      |
|           |-------|                                 |
|           |Actel  | |-------|  tennis_10     -      |
|           | A1020B| |Actel  |                       |
|J          |-------| | A1020B|  tennis_9      -      |
|A  32MHz    JOAQUIN  |-------|                       |
|M                      JUANA    tennis_8      -      |
|M            28C264                                  |
|A          tennis_1  tennis_2   tennis_7  tennis_14  |
|                                 GAL22V10            |
|            MT5C256  MT5C256              tennis_13  |
|          |------------------|     74HC404           |
|          |      68000P8     |  tennis_3  tennis_4   |
|          |------------------|                       |
|                NE555              DAC        DAC    |
|-----------------------------------------------------|

Video hardware:
---------------
Blitter based. Two layers with tricky doublebuffering.
Two Actel FPGA chips (marked as JOAQUIN and JUANA).
Juana can read data from ROMs. JOAQUIN - write to VRAM.
Both can access 256x256 pixel pages.
Size and direction of data read/write, as well as active page is
selectable for each of the chips.

Sound hardware (verify):
------------------------
~15 kHz 8 bit signed (music) and unsigned (sfx) sample player.
Two custom DACs are conencted directly to data lines of sound ROMs.
A0-A10 address lines are controlled by a counter, clocked by scaline
clock ( not verified, just guessed ). Top lines are controlled by cpu,
and select 2k sample to play. There's probably no way to stop the sample
player - when there's nothing to play - first, empty 2k of ROMs are selected.

 TODO:
- the blitter's timing is estimated, not measured on a board: a time per pixel walked
  (BLIT_CLOCKS_PER_PIXEL), and a list held until the page flips at vblank
- which source pixel a scaled blit drops in each period (the phase) is a model choice, see
  do_blit; so are a destination longer than its source holding the source's last step, and a
  walk wrapping in y as it does in x; the right and bottom edges of the visible area are not
  established
- awfully similar to littlerb;


**************************************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/eeprompar.h"
#include "sound/dac.h"
#include "video/ramdac.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>
#include <bit>
#include <iterator>

#define LOG_SCALE (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

namespace {

constexpr int RLT_REFRESH_RATE   = 60;
constexpr int RLT_TIMER_FREQ     = RLT_REFRESH_RATE * 256;

constexpr XTAL RLT_MAIN_CLOCK = 32_MHz_XTAL; // the board's only crystal
constexpr unsigned BLIT_CLOCKS_PER_PIXEL = 5; // a guess, see the TODO

constexpr uint16_t BLTFLAG_DST_X_DIR   = 1 << 0;
constexpr uint16_t BLTFLAG_DST_Y_DIR   = 1 << 1;
constexpr uint16_t BLTFLAG_DST_LR      = 1 << 2;
constexpr uint16_t BLTFLAG_DST_UD      = 1 << 3;
constexpr uint16_t BLTFLAG_DISPLAY_UD  = 1 << 5;

constexpr uint16_t BLTFLAG_SRC_X_DIR   = 1 << 8;
constexpr uint16_t BLTFLAG_SRC_Y_DIR   = 1 << 9;
constexpr uint16_t BLTFLAG_SRC_LR      = 1 << 10;
constexpr uint16_t BLTFLAG_SRC_UD      = 1 << 11;

constexpr uint16_t BLTSTRT_ROM_MSB     = 1 << 8;
constexpr uint16_t BLTSTRT_TRIGGER     = 1 << 14;
constexpr uint16_t BLTSTRT_LAYER       = 1 << 15;

constexpr unsigned SRC_SHIFT           = 8;

constexpr unsigned RLT_NUM_BLITTER_REGS    = 8;
constexpr unsigned RLT_NUM_BITMAPS         = 8;

enum
{
	BLT_X_START = 0,
	BLT_X_END,
	BLT_Y_START,
	BLT_Y_END,
	BLT_FLAGS,
	BLT_SCALE,
	BLT_START,
	BLT_UNK2
};

enum
{
	BITMAP_BG=0,
	BITMAP_FG_1,
	BITMAP_FG_2,
	BITMAP_FG_DISPLAY
};

class rltennis_state : public driver_device
{
public:
	rltennis_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac%u", 1U),
		m_samples(*this, "samples%u", 1U),
		m_gfx(*this, "gfx"),
		m_data760000(0), m_data740000(0), m_dac_counter(0),
		m_offset_shift(0)
	{
		std::fill(std::begin(m_sample_rom_offset), std::end(m_sample_rom_offset), 0);
	}

	void rltennis(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<dac_byte_interface, 2> m_dac;
	required_region_ptr_array<uint8_t, 2> m_samples;
	required_region_ptr<uint8_t> m_gfx;

	uint16_t m_blitter[RLT_NUM_BLITTER_REGS];
	int32_t m_data760000;
	int32_t m_data740000;
	int32_t m_dac_counter;
	int32_t m_sample_rom_offset[2];
	int32_t m_offset_shift;
	int32_t m_unk_counter = 0;
	std::unique_ptr<bitmap_ind16> m_tmp_bitmap[RLT_NUM_BITMAPS];
	emu_timer *m_timer = nullptr;
	emu_timer *m_blit_done_timer;
	bool m_blit_busy;
	bool m_blit_deferred;
	bool m_flip_pending;
	uint16_t m_flip_display;

	uint16_t io_r();
	void snd1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void snd2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void blitter_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void decode_scale(uint16_t scale, int &dst_steps, int &src_steps) const;
	void do_blit();
	void flip_display(uint16_t display);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_CALLBACK_MEMBER(sample_player);
	TIMER_CALLBACK_MEMBER(blit_done);

	void rltennis_main(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
};

/****************************************************************************************
Blitter registers description (reg/bit names selected arbitrary ) :

700000 - BLT_X_START
     JUANA  JOAQUIN
  fedcba98 76543210
  -------- xxxxxxxx dst x start
  xxxxxxxx -------- src x start


700002 - BLT_X_END
     JUANA  JOAQUIN
  fedcba98 76543210
  -------- xxxxxxxx dst x end
  xxxxxxxx -------- src x end


700004 - BLT_Y_START
     JUANA  JOAQUIN
  fedcba98 76543210    x start
  -------- xxxxxxxx
  xxxxxxxx --------


700006 - BLT_Y_END
     JUANA  JOAQUIN
  fedcba98 76543210    x start
  -------- xxxxxxxx
  xxxxxxxx --------


700008 - BLT_FLAGS
     JUANA  JOAQUIN
  fedcba98 76543210
  -------- -------x BLTFLAG_DST_X_DIR    x dst direction (step inc or dec)
  -------- ------x- BLTFLAG_DST_Y_DIR    y dst direction
  -------- -----x-- BLTFLAG_DST_LR       LR and UD controls the quarter of framebuffer to use
  -------- ----x--- BLTFLAG_DST_UD       /
  -------- ---?----                      unknown
  -------- --x----- BLTFLAG_DISPLAY_UD   display buffer select
  -------- ??------                      unknown
  -------x -------- BLTFLAG_SRC_X_DIR    x src direction
  ------x- -------- BLTFLAG_SRC_Y_DIR    y src direction
  -----x-- -------- BLTFLAG_SRC_LR       LR and UD controls the quarter of src buffer to use
  ----x--- -------- BLTFLAG_SRC_UD       /
  xxxx---- --------                      src ROM num


70000a - BLT_SCALE
  fedcba98 76543210
  aaaa---- bbbb---- scale = (a+1)/(b+1), destination pixels per source pixel, on both axes
  ----aaaa ----bbbb the high nibbles repeated, in all but 8080 and 0004 (both 1/1)

  The game writes 0000 before loading each value. Most blits run at 0000; play uses
  2233 (3/4) and 1122 (2/3). Only reductions are seen; a > b (enlarging) is unobserved.


70000c - BLT_START
  fedcba98 76543210
  --?????- ???????? unknown
  -------x -------- BLTSTRT_ROM_MSB     src ROM MSB
  -x------ -------- BLTSTRT_TRIGGER     blit start
  x------- -------- BLTSTRT_LAYER       FG or BG layer of framebuffer

70000e - BLT_UNK2
     JUANA  JOAQUIN
  fedcba98 76543210
  ???????? ???????? unknown (set to 0 @ boot)

****************************************************************************************/

// The two counters of 70000a as destination and source steps, a+1 and b+1 from the high
// nibbles; 0x0000 is 1/1, and a value whose nibbles differ is decoded the same way.
void rltennis_state::decode_scale(uint16_t scale, int &dst_steps, int &src_steps) const
{
	const auto counter = [scale] (int shift) { return BIT(scale, shift, 4) + 1; };

	dst_steps = counter(12);
	src_steps = counter(4);

	if (counter(8) != dst_steps || counter(0) != src_steps)
		LOGMASKED(LOG_SCALE, "%s: scale register %04x does not repeat its nibbles; using the high ones\n", machine().describe_context(), scale);
	if (dst_steps > src_steps)
		LOGMASKED(LOG_SCALE, "%s: scale register %04x enlarges, which the game was never seen to ask for\n", machine().describe_context(), scale);
}

void rltennis_state::blitter_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int old_data=m_blitter[offset];
	COMBINE_DATA(&m_blitter[offset]);
	int new_data=m_blitter[offset];

	// a list's first blit carries the other display page; the page flips at the next vblank, and
	// that blit waits for it, so nothing is drawn into a plane before it has been shown and cleared
	if(offset==BLT_FLAGS && ((new_data^old_data) & BLTFLAG_DISPLAY_UD) )
	{
		m_flip_pending = true;
		m_flip_display = new_data & BLTFLAG_DISPLAY_UD;
	}

	// IRQ1 is a level held while the blitter is idle; the game clears the strobe before each blit
	if(offset == BLT_START && !(new_data & BLTSTRT_TRIGGER))
		m_maincpu->set_input_line(1, CLEAR_LINE);

	if(offset == BLT_START && (((new_data ^ old_data ) & new_data) & BLTSTRT_TRIGGER))  /* blit strobe 0->1 */
	{
		if (m_flip_pending)
			m_blit_deferred = true;
		else
			do_blit();
	}
}

void rltennis_state::do_blit()
{
	int src_x0=(m_blitter[BLT_X_START]>>SRC_SHIFT)+((m_blitter[BLT_FLAGS] & BLTFLAG_SRC_LR)?256:0);
	int src_y0=(m_blitter[BLT_Y_START]>>SRC_SHIFT)+((m_blitter[BLT_FLAGS]>>3)&0xff00)+(((m_blitter[BLT_START]) & BLTSTRT_ROM_MSB)?(1<<0xd):0);

	int dst_x0=(m_blitter[BLT_X_START]&0xff);
	int dst_y0=(m_blitter[BLT_Y_START]&0xff);

	int dst_x1=(m_blitter[BLT_X_END]&0xff);
	int dst_y1=(m_blitter[BLT_Y_END]&0xff);

	int src_x1=((m_blitter[BLT_X_END]>>SRC_SHIFT)&0xff)+((m_blitter[BLT_FLAGS] & BLTFLAG_SRC_LR)?256:0);
	int src_y1=((m_blitter[BLT_Y_END]>>SRC_SHIFT)&0xff)+((m_blitter[BLT_FLAGS]>>3)&0xff00)+(((m_blitter[BLT_START]) & BLTSTRT_ROM_MSB)?(1<<0xd):0);

	int x_dst_step=(m_blitter[BLT_FLAGS] & BLTFLAG_DST_X_DIR)?1:-1;
	int y_dst_step=(m_blitter[BLT_FLAGS] & BLTFLAG_DST_Y_DIR)?1:-1;

	int x_src_step=(m_blitter[BLT_FLAGS] & BLTFLAG_SRC_X_DIR)?1:-1;
	int y_src_step=(m_blitter[BLT_FLAGS] & BLTFLAG_SRC_Y_DIR)?1:-1;

	// a walk ends where the low 8 bits of its counter reach the end coordinate; the source
	// address itself runs on past the end of its quarter
	const auto span = [] (int start, int end, int step) { return ((step > 0) ? (end - start) : (start - end)) & 0xff; };

	const int blit_w=span(src_x0, src_x1, x_src_step);
	const int blit_h=span(src_y0, src_y1, y_src_step);

	const int blit_w1=span(dst_x0, dst_x1, x_dst_step);
	const int blit_h1=span(dst_y0, dst_y1, y_dst_step);

	// a scaled blit reads source step d*(b+1)/(a+1) at destination step d; which steps
	// repeat or skip a pixel (the phase) is a model choice
	const uint16_t scale = m_blitter[BLT_SCALE];
	int scale_dst, scale_src;
	decode_scale(scale, scale_dst, scale_src);

	int layer=(m_blitter[BLT_START] & BLTSTRT_LAYER )?BITMAP_BG:BITMAP_FG_1;

	if(layer==BITMAP_FG_1)
	{
		if(m_blitter[BLT_FLAGS] & BLTFLAG_DST_UD )
		{
			layer=BITMAP_FG_2;
		}
	}

	bool force_blit=false;

	if(blit_w==1 && blit_h==1) /* seems to be bg layer color fill */
	{
		force_blit=true;
	}

	// held inside the source span, whatever encodes the ratio: a destination can be longer than
	// the ratio gives (one step on a scaled blit, a few columns on set 2's 1/1 ball animation,
	// where the ROM's next columns are not part of the picture)
	const auto src_idx = [scale_src, scale_dst] (int d, int limit) { return std::min(d * scale_src / scale_dst, limit); };

	// the source row of each destination row is the same in every column
	int src_row[256];
	for( int idx_y=0 ; idx_y<=blit_h1; idx_y++ )
		src_row[idx_y]=src_y0+(y_src_step*src_idx(idx_y, blit_h));

	for( int x=dst_x0, idx_x=0 ; idx_x<=blit_w1; x+=x_dst_step, idx_x++ )
	{
		const int col_xx=src_x0+(x_src_step*src_idx(idx_x, blit_w));

		for( int y=dst_y0, idx_y=0 ; idx_y<=blit_h1;y+=y_dst_step, idx_y++)
		{
			int xx=col_xx;
			int yy=src_row[idx_y];

			if(force_blit)
			{
				xx=src_x0;
				yy=src_y0;
			}

			int address=yy*512+xx;

			int pix = m_gfx[address & (m_gfx.length() - 1)];
			int screen_x=(x&0xff)+((m_blitter[BLT_FLAGS] & BLTFLAG_DST_LR )?256:0);

			if(pix || force_blit)
			{
				m_tmp_bitmap[layer]->pix(y & 0xff, screen_x ) = pix;
			}
		}
	}

	// the blit takes a time per pixel walked, the longer of the source and destination walks
	const unsigned walked = (std::max(blit_w, blit_w1) + 1) * (std::max(blit_h, blit_h1) + 1);
	m_blit_busy = true;
	m_blit_done_timer->adjust(attotime::from_ticks(walked * BLIT_CLOCKS_PER_PIXEL, RLT_MAIN_CLOCK));
}

TIMER_CALLBACK_MEMBER(rltennis_state::blit_done)
{
	m_blit_busy = false;
	m_maincpu->set_input_line(1, ASSERT_LINE);
}

void rltennis_state::flip_display(uint16_t display)
{
	// the plane that was drawn becomes the visible one, and is cleared for the next list
	bitmap_ind16 &drawn = *m_tmp_bitmap[display ? BITMAP_FG_1 : BITMAP_FG_2];
	copybitmap(*m_tmp_bitmap[BITMAP_FG_DISPLAY], drawn, 0, 0, 0, 0, m_tmp_bitmap[BITMAP_FG_DISPLAY]->cliprect());
	drawn.fill(0);
}

void rltennis_state::screen_vblank(int state)
{
	if (state && m_flip_pending && !m_blit_busy)
	{
		flip_display(m_flip_display);
		m_flip_pending = false;
		if (m_blit_deferred)
		{
			m_blit_deferred = false;
			do_blit();
		}
	}
}

void rltennis_state::video_start()
{
	// assumes it can make an address mask with m_gfx.length() - 1
	assert(std::has_single_bit(m_gfx.length()));

	m_tmp_bitmap[BITMAP_BG] = std::make_unique<bitmap_ind16>(512, 256);
	m_tmp_bitmap[BITMAP_FG_1] = std::make_unique<bitmap_ind16>(512, 256);
	m_tmp_bitmap[BITMAP_FG_2] = std::make_unique<bitmap_ind16>(512, 256);
	m_tmp_bitmap[BITMAP_FG_DISPLAY] = std::make_unique<bitmap_ind16>(512, 256);

	save_item(NAME(m_blitter));
	// the planes persist between frames and are painted blit by blit, so they are state
	for (int i = BITMAP_BG; i <= BITMAP_FG_DISPLAY; i++)
		save_item(NAME(*m_tmp_bitmap[i]), i);
}

uint32_t rltennis_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, *m_tmp_bitmap[BITMAP_BG], 0, 0, 0, 0, cliprect);
	copybitmap_trans(bitmap, *m_tmp_bitmap[BITMAP_FG_DISPLAY], 0, 0, 0, 0, cliprect, 0);
	return 0;
}

uint16_t rltennis_state::io_r()
{
	return (ioport("P1" )->read()&0x1fff) | (m_unk_counter<<13); // Top 3 bits control sample address update
}

void rltennis_state::snd1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_data760000);
}

void rltennis_state::snd2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_data740000);
}

void rltennis_state::rltennis_main(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x103fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x200000, 0x20ffff).ram();
	map(0x700000, 0x70000f).w(FUNC(rltennis_state::blitter_w));
	map(0x720001, 0x720001).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x720003, 0x720003).rw("ramdac", FUNC(ramdac_device::pal_r), FUNC(ramdac_device::pal_w));
	map(0x720007, 0x720007).w("ramdac", FUNC(ramdac_device::index_r_w));
	map(0x740000, 0x740001).w(FUNC(rltennis_state::snd1_w));
	map(0x760000, 0x760001).w(FUNC(rltennis_state::snd2_w));
	map(0x780000, 0x780001).nopw();    // Sound control, unknown, usually = 0x0044
	map(0x7a0000, 0x7a0003).nopr();    // Unknown, read only at boot time
	map(0x7e0000, 0x7e0001).r(FUNC(rltennis_state::io_r));
	map(0x7e0002, 0x7e0003).portr("P2");
}

static INPUT_PORTS_START( rltennis )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE )

	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

TIMER_CALLBACK_MEMBER(rltennis_state::sample_player)
{
	if((m_dac_counter&0x7ff) == 0x7ff) // Reload top address bits
	{
		m_sample_rom_offset[0]=(( m_data740000 >> m_offset_shift ) & 0xff )<<11;
		m_sample_rom_offset[1]=(( m_data760000 >> m_offset_shift ) & 0xff )<<11;
		m_offset_shift^=8; // Switch between MSB and LSB
	}
	++m_dac_counter; // Update low address bits

	m_dac[0]->write(m_samples[0][m_sample_rom_offset[0] + (m_dac_counter & 0x7ff)]);
	m_dac[1]->write(m_samples[1][m_sample_rom_offset[1] + (m_dac_counter & 0x7ff)]);
	m_timer->adjust(attotime::from_hz( RLT_TIMER_FREQ ));
}

INTERRUPT_GEN_MEMBER(rltennis_state::interrupt)
{
	++m_unk_counter; // Frame counter? verify
	device.execute().set_input_line(4, HOLD_LINE);
}

void rltennis_state::machine_start()
{
	m_timer = timer_alloc(FUNC(rltennis_state::sample_player), this);
	m_blit_done_timer = timer_alloc(FUNC(rltennis_state::blit_done), this);

	m_flip_display = 0;

	save_item(NAME(m_data760000));
	save_item(NAME(m_data740000));
	save_item(NAME(m_dac_counter));
	save_item(NAME(m_sample_rom_offset));
	save_item(NAME(m_offset_shift));
	save_item(NAME(m_unk_counter));
	save_item(NAME(m_blit_busy));
	save_item(NAME(m_blit_deferred));
	save_item(NAME(m_flip_pending));
	save_item(NAME(m_flip_display));
}

void rltennis_state::machine_reset()
{
	m_timer->adjust(attotime::from_hz(RLT_TIMER_FREQ));

	// the blitter comes out of reset idle; the driver also clears its registers, against which
	// each write finds the flip and strobe edges
	std::fill(std::begin(m_blitter), std::end(m_blitter), 0);
	m_blit_done_timer->adjust(attotime::never);
	m_blit_busy = false;
	m_blit_deferred = false;
	m_flip_pending = false;
	m_maincpu->set_input_line(1, ASSERT_LINE);
}

void rltennis_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb888_w));
}

void rltennis_state::rltennis(machine_config &config)
{
	M68000(config, m_maincpu, RLT_MAIN_CLOCK / 4); // MC68000P8, divider is a guess
	m_maincpu->set_addrmap(AS_PROGRAM, &rltennis_state::rltennis_main);
	m_maincpu->set_vblank_int("screen", FUNC(rltennis_state::interrupt));

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(RLT_REFRESH_RATE);
	// the game anchors its full-screen pictures at (1,1) and lets a sprite crossing x=255 spill
	// one duplicate pixel onto column 0, so the picture starts at (1,1); the total is the planes'
	// size and the refresh 60 Hz, both nominal, since this board's field timing is not known
	screen.set_size(512, 256);
	screen.set_visarea(1, 320, 1, 240);
	screen.set_screen_update(FUNC(rltennis_state::screen_update));
	screen.screen_vblank().set(FUNC(rltennis_state::screen_vblank));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(256);

	EEPROM_2864(config, "eeprom");

	ramdac_device &ramdac(RAMDAC(config, "ramdac", "palette"));
	ramdac.set_addrmap(0, &rltennis_state::ramdac_map);
	ramdac.set_split_read(1);

	SPEAKER(config, "speaker").front_center();

	DAC_8BIT_R2R(config, "dac1", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // Unknown DAC
	DAC_8BIT_R2R(config, "dac2", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // Unknown DAC
}

ROM_START( rltennis )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tennis_1.u12", 0x00001, 0x80000, CRC(2ded10d7) SHA1(cca1e858c9c759ef5c0aca6ee50d23d5d532534c) )
	ROM_LOAD16_BYTE( "tennis_2.u19", 0x00000, 0x80000, CRC(a0dbd2ed) SHA1(8db7dbb6a36fd0fb382a4938d7eba1f7662aa672) )

	ROM_REGION( 0x1000000, "gfx", ROMREGION_ERASE00  )
	ROM_LOAD( "tennis_5.u33", 0x000000, 0x80000, CRC(067a2e4b) SHA1(ab5a227de2b0c51b17aeca68c8af1bf224904ac8) )
	ROM_LOAD( "tennis_6.u34", 0x080000, 0x80000, CRC(901df2c1) SHA1(7e57d7c7e281ddc02a3e34178d3e471bd8e1d572) )
	ROM_LOAD( "tennis_7.u35", 0x100000, 0x80000, CRC(8d70fb37) SHA1(250c4c3d32e5a7e17413ee41e1abccb0492b63fd) )
	ROM_LOAD( "tennis_8.u36", 0x180000, 0x80000, CRC(26d202ba) SHA1(0e841e35de328f23624a19780a734a18f5409d69) )
	ROM_LOAD( "tennis_9.u37", 0x200000, 0x80000, CRC(1d164ee0) SHA1(b9c80b3c0dadbff36a04141b8995a5282a8d10f7) )
	ROM_LOAD( "tennis_10.u38",0x280000, 0x80000, CRC(fd2c6647) SHA1(787f236d5b72ee24d39e783eb2453bea58f07290) )
	ROM_LOAD( "tennis_11.u39",0x300000, 0x80000, CRC(a59dc0c8) SHA1(48f258e74fbb64b7538c9777d7598774ca8396eb) )
	ROM_LOAD( "tennis_12.u40",0x380000, 0x80000, CRC(b9677887) SHA1(84b79864555d3d6e9c443913910a055e27d30d08) )
	ROM_LOAD( "tennis_13.u41",0x400000, 0x80000, CRC(3d4fbcac) SHA1(e01f479d7d516ff83cbbd82d83617146d7a242d3) )
	ROM_LOAD( "tennis_14.u42",0x480000, 0x80000, CRC(37fe0f5d) SHA1(7593f1ea07bc0a741c952e6850bed1bf0a824510) )

	ROM_REGION( 0x080000, "samples1", 0 )
	ROM_LOAD( "tennis_4.u59", 0x00000, 0x80000, CRC(f56462ea) SHA1(638777e12f2649a5b4366f034f0ba721fc4580a8) )

	ROM_REGION( 0x080000, "samples2", 0 )
	ROM_LOAD( "tennis_3.u52", 0x00000, 0x80000, CRC(517dcd0e) SHA1(b2703e185ee8cf7e115ea07151e7bee8be34948b) )

	ROM_REGION( 0x2e5, "plds", 0 )
	ROM_LOAD( "realitytennis_gal22v10.u20", 0x000, 0x2e5, BAD_DUMP CRC(13be6cec) SHA1(5a07ed3ac6a1993196e0e76b852d0ba132f5ddb9) ) // Bruteforced but verified
ROM_END

// PCB marked as "TCH-MAVIR.001
// Two TI TPC1020AF instead of two Actel A1020B, also labeled as JOAQUIN (A) and JUANA (B)
ROM_START( rltennisa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tennis_1_a.bin", 0x00001, 0x80000, CRC(30bd9f1d) SHA1(52225793fb1fee4d395f21c45756e9244cf60b9e) )
	ROM_LOAD16_BYTE( "tennis_2_a.bin", 0x00000, 0x80000, CRC(f2997957) SHA1(c2ccaecd337c46cd9e4a40d43f30f203efc10111) )

	ROM_REGION( 0x1000000, "gfx", ROMREGION_ERASE00  )
	ROM_LOAD( "tennis_5.u33", 0x000000, 0x80000, CRC(067a2e4b) SHA1(ab5a227de2b0c51b17aeca68c8af1bf224904ac8) )
	ROM_LOAD( "tennis_6.u34", 0x080000, 0x80000, CRC(901df2c1) SHA1(7e57d7c7e281ddc02a3e34178d3e471bd8e1d572) )
	ROM_LOAD( "tennis_7.u35", 0x100000, 0x80000, CRC(8d70fb37) SHA1(250c4c3d32e5a7e17413ee41e1abccb0492b63fd) )
	ROM_LOAD( "tennis_8.u36", 0x180000, 0x80000, CRC(26d202ba) SHA1(0e841e35de328f23624a19780a734a18f5409d69) )
	ROM_LOAD( "tennis_9.u37", 0x200000, 0x80000, CRC(1d164ee0) SHA1(b9c80b3c0dadbff36a04141b8995a5282a8d10f7) )
	ROM_LOAD( "tennis_10.u38",0x280000, 0x80000, CRC(fd2c6647) SHA1(787f236d5b72ee24d39e783eb2453bea58f07290) )
	ROM_LOAD( "tennis_11.u39",0x300000, 0x80000, CRC(a59dc0c8) SHA1(48f258e74fbb64b7538c9777d7598774ca8396eb) )
	ROM_LOAD( "tennis_12.u40",0x380000, 0x80000, CRC(b9677887) SHA1(84b79864555d3d6e9c443913910a055e27d30d08) )
	ROM_LOAD( "tennis_13.u41",0x400000, 0x80000, CRC(3d4fbcac) SHA1(e01f479d7d516ff83cbbd82d83617146d7a242d3) )
	ROM_LOAD( "tennis_14.u42",0x480000, 0x80000, CRC(37fe0f5d) SHA1(7593f1ea07bc0a741c952e6850bed1bf0a824510) )

	ROM_REGION( 0x080000, "samples1", 0 )
	ROM_LOAD( "tennis_4.u59", 0x00000, 0x80000, CRC(f56462ea) SHA1(638777e12f2649a5b4366f034f0ba721fc4580a8) )

	ROM_REGION( 0x080000, "samples2", 0 )
	ROM_LOAD( "tennis_3.u52", 0x00000, 0x80000, CRC(517dcd0e) SHA1(b2703e185ee8cf7e115ea07151e7bee8be34948b) )

	ROM_REGION( 0x3fc, "plds", 0 )
	ROM_LOAD( "gal16v8.ic23", 0x000, 0x117, CRC(7bc89e80) SHA1(6b34527121e4dbac37fbbd79c57a7eab81de0198) )
	ROM_LOAD( "gal22v10.ic2", 0x117, 0x2e5, CRC(b5a7cb92) SHA1(05d6e8c8208ac486e849de3322f049bcd38561e1) )
ROM_END

} // anonymous namespace

GAME( 1993, rltennis,         0, rltennis, rltennis, rltennis_state, empty_init, ROT0, "TCH", "Reality Tennis (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_SUPPORTS_SAVE )
GAME( 1993, rltennisa, rltennis, rltennis, rltennis, rltennis_state, empty_init, ROT0, "TCH", "Reality Tennis (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_SUPPORTS_SAVE )
