// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert
/***************************************************************************

                      -= IGS011 (Blitter) Based Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)
            code decrypted by   Olivier Galibert


CPU     :   68000
Sound   :   M6295 + Optional FM or ICS2115
Custom  :   IGS011 (blitter, protection)
            IGS012 (protection, optional)
            IGS003 (8255, protection)
NVRAM   :   Battery for main RAM

---------------------------------------------------------------------------
Year + Game                   PCB        Sound         Chips
---------------------------------------------------------------------------
95 Da Ban Cheng               NO-T0084-1 M6295         IGS011 8255
95 Long Hu Bang V033C         NO-T0093   M6295         IGS011 8255
95 Long Hu Bang V035C         ?
95 Mj Ryukobou                NO-T0094   M6295         IGS011 8255
95 Dragon World V010C         NO-0105-4  M6295 YM3812  IGS011 IGS003
95 Dragon World V011H (set 1) ?
95 Dragon World V011H (set 2) NO-T0105   M6295 YM3812  IGS011 IGS003  IGS012
95 Dragon World V020J         ?          M6295 YM3812  IGS011 IGS003  IGS012
95 Dragon World V021J         ?
95 Dragon World V021O         NO-0105-1  M6295 YM3812  IGS011 IGS003  IGS012
95 Dragon World V030O         NO-0105-1  M6295 YM3812  IGS011 IGS003
97 Dragon World V040O         NO-0105-5  M6295 YM3812  IGS011 IGS003c
97 Dragon World V040K         NO-0105-5  M6295 YM3812  IGS011 IGS003c IGS012
96 Virtua Bowling V100JCM     NO-0101-?  ICS2115       IGS011 IGS003e IGS012
96 Virtua Bowling V101XCM     NO-0101-1  ICS2115       IGS011 IGS003e IGS012
96 Virtua Bowling V101HJS     NO-0101-?  ICS2115       IGS011 IGS003e?IGS012
96 Long Hu Bang II V185H      NO-0115    M6295 YM2413  IGS011 8255
96 Te Yi Gong Neng            NO-0127-2  M6295 YM2413  IGS011 IGS003A
96 Wanli Changcheng           ?
96 Xingyun Man Guan           ?
98 Mj Nenrikishu SP V250J     NO-0115-5  M6295 YM2413  IGS011 8255
---------------------------------------------------------------------------

To do:

- Implement the I/O part of IGS003 as an 8255
- IGS003 parametric bitswap protection in nkishusp (instead of patching the roms)
- Interrupt controller at 838000 or a38000 (there's a preliminary implementation for lhb)
- A few graphical bugs

- vbowl, vbowlj: trackball support.
  Wrong colors in "Game Over" screen.

- lhb: in the copyright screen the '5' in '1995' is drawn by the cpu on layer 5,
  but with wrong colors (since the top nibble of the affected pixels is left to 0xf)
  (drgnwrld is like this too, maybe hacked, or a cheap year replacement by IGS)

- dbc: in the title screen the '5' in '1995' is drawn by the cpu with wrong colors.
  (see above comment)
  Also the background palette is wrong since the fade routine is called with wrong
  parameters, but in this case the PCB does the same.

Notes:

- In most games, keep test button pressed during boot for another test mode

***************************************************************************/

#include "emu.h"

#include "igsmahjong.h"

#include "mahjong.h"

#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/ics2115.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_BLITTER (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

class igs011_state : public driver_device
{
public:
	igs011_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_priority_ram(*this, "priority_ram")
		, m_maincpu_region(*this, "maincpu")
		, m_gfx(*this, "blitter")
		, m_gfx_hi(*this, "blitter_hi")
		, m_io_in(*this, "IN%u", 0U)
		, m_io_key(*this, "KEY%u", 0U)
		, m_io_an(*this, "AN%u", 0U)
		, m_io_dsw(*this, "DSW%u", 1U)
		, m_io_coin(*this, "COIN")
	{
	}

	int igs_hopper_r();

	void init_lhbv33c() ATTR_COLD;
	void init_drgnwrldv21j() ATTR_COLD;
	void init_wlcc() ATTR_COLD;
	void init_nkishusp() ATTR_COLD;
	void init_drgnwrldv21() ATTR_COLD;
	void init_dbc() ATTR_COLD;
	void init_lhb() ATTR_COLD;
	void init_drgnwrld() ATTR_COLD;
	void init_drgnwrldv30() ATTR_COLD;
	void init_drgnwrldv11h() ATTR_COLD;
	void init_lhb2() ATTR_COLD;
	void init_xymg() ATTR_COLD;
	void init_drgnwrldv10c() ATTR_COLD;
	void init_drgnwrldv20j() ATTR_COLD;
	void init_drgnwrldv40k() ATTR_COLD;
	void init_ryukobou() ATTR_COLD;
	void init_tygn() ATTR_COLD;

	void drgnwrld(machine_config &config) ATTR_COLD;
	void nkishusp(machine_config &config) ATTR_COLD;
	void tygn(machine_config &config) ATTR_COLD;
	void wlcc(machine_config &config) ATTR_COLD;
	void xymg(machine_config &config) ATTR_COLD;
	void lhb2(machine_config &config) ATTR_COLD;
	void lhb(machine_config &config) ATTR_COLD;
	void drgnwrld_igs012(machine_config &config) ATTR_COLD;

protected:
	struct blitter_t
	{
		u16  x, y, w, h,
			gfx_lo, gfx_hi,
			depth,
			pen,
			flags;
	};

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<u16> m_priority_ram;

	/* memory regions */
	required_memory_region m_maincpu_region;
	required_region_ptr<u8> m_gfx;
	optional_region_ptr<u8> m_gfx_hi;

	optional_ioport_array<3> m_io_in;
	optional_ioport_array<5> m_io_key;
	optional_ioport_array<2> m_io_an;
	optional_ioport_array<5> m_io_dsw;
	optional_ioport m_io_coin;

	std::unique_ptr<u8[]> m_layer[8];
	u16 m_priority;
	u8 m_blitter_pen_hi;
	u16 m_dips_sel;
	u16 m_input_sel;
	u16 m_hopper_bit;
	u8 m_prot1;
	u8 m_prot1_swap;
	u32 m_prot1_addr;
	u8 m_prot2;
	u8 m_igs012_prot;
	u8 m_igs012_prot_swap;
	u8 m_igs012_prot_mode;
	u16 m_igs003_reg;
	u16 m_lhb_irq_enable;

	blitter_t m_blitter;

	u16 m_igs003_prot_hold;
	u8 m_igs003_prot_x;
	u8 m_igs003_prot_y;
	u8 m_igs003_prot_z;
	u8 m_igs003_prot_h1;
	u8 m_igs003_prot_h2;

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void igs011_base(machine_config &config) ATTR_COLD;

	void igs011_priority_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 igs011_layers_r(offs_t offset);
	void igs011_layers_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void igs011_blit_x_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void igs011_blit_y_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void igs011_blit_gfx_lo_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void igs011_blit_gfx_hi_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void igs011_blit_w_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void igs011_blit_h_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void igs011_blit_depth_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void igs011_blit_pen_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void igs011_blit_flags_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void dips_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 key_matrix_r();
	template<unsigned Num> u16 dips_r();
	void igs011_prot1_w(offs_t offset, u8 data);
	u16 igs011_prot1_r();
	void igs011_prot_addr_w(u16 data);
	void igs011_prot2_reset_w(u8 data);
	u16 igs011_prot2_reset_r();
	void igs011_prot2_inc_w(u8 data);
	void igs011_prot2_dec_w(u8 data);
	void drgnwrld_igs011_prot2_swap_w(u8 data);
	void lhb_igs011_prot2_swap_w(u8 data);
	void wlcc_igs011_prot2_swap_w(u8 data);
	u16 drgnwrldv40k_igs011_prot2_r();
	u16 drgnwrldv21_igs011_prot2_r();
	u16 drgnwrldv20j_igs011_prot2_r();
	u16 lhb_igs011_prot2_r();
	u16 dbc_igs011_prot2_r();
	u16 ryukobou_igs011_prot2_r();
	u16 lhb2_igs011_prot2_r();
	void igs012_prot_reset_w(u16 data);
	void igs012_prot_mode_w(offs_t offset, u8 data);
	void igs012_prot_inc_w(offs_t offset, u8 data);
	void igs012_prot_dec_inc_w(offs_t offset, u8 data);
	void igs012_prot_dec_copy_w(offs_t offset, u8 data);
	void igs012_prot_copy_w(offs_t offset, u8 data);
	void igs012_prot_swap_w(offs_t offset, u8 data);
	u16 igs012_prot_r();
	void igs003_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void drgnwrld_igs003_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 drgnwrld_igs003_r();
	void lhb_inputs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 lhb_inputs_r(offs_t offset);
	void lhb2_igs003_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 lhb2_igs003_r();
	void wlcc_igs003_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 wlcc_igs003_r();
	void xymg_igs003_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 xymg_igs003_r();
	void lhb_irq_enable_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void lhb_okibank_w(u8 data);
	void sound_irq(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(lev5_timer_irq_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(lhb_timer_irq_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(lev3_timer_irq_cb);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(lhb_vblank_irq);
	void wlcc_decrypt();
	void lhb_decrypt();
	void drgnwrld_type3_decrypt();
	void drgnwrld_type2_decrypt();
	void drgnwrld_type1_decrypt();
	void drgnwrldv40k_decrypt();
	void lhb2_decrypt();
	void nkishusp_decrypt();
	void dbc_decrypt();
	void ryukobou_decrypt();
	void tygn_decrypt();
	void lhb2_gfx_decrypt();
	void drgnwrld_gfx_decrypt();
	void prot_mem_range_set();

	void drgnwrld_mem(address_map &map) ATTR_COLD;
	void drgnwrld_igs012_mem(address_map &map) ATTR_COLD;
	void lhb_mem(address_map &map) ATTR_COLD;
	void lhb2_mem(address_map &map) ATTR_COLD;
	void nkishusp_mem(address_map &map) ATTR_COLD;
	void tygn_mem(address_map &map) ATTR_COLD;
	void wlcc_mem(address_map &map) ATTR_COLD;
	void xymg_mem(address_map &map) ATTR_COLD;
};

// With trackball inputs
class vbowl_state : public igs011_state
{
public:
	vbowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: igs011_state(mconfig, type, tag)
		, m_ics(*this, "ics")
		, m_vbowl_trackball(*this, "vbowl_trackball")
		, m_io_an(*this, "AN%u", 0U)
	{
	}

	void init_vbowl() ATTR_COLD;
	void init_vbowlj() ATTR_COLD;
	void init_vbowlhk() ATTR_COLD;

	void vbowl(machine_config &config) ATTR_COLD;
	void vbowlhk(machine_config &config) ATTR_COLD;

private:
	/* devices */
	required_device<ics2115_device> m_ics;

	/* memory pointers */
	required_shared_ptr<u16> m_vbowl_trackball;

	required_ioport_array<2> m_io_an;

	void vbowl_igs011_prot2_swap_w(u8 data);
	u16 vbowl_igs011_prot2_r();
	u16 vbowlhk_igs011_prot2_r();
	void vbowl_igs003_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 vbowl_igs003_r();
	void vbowlhk_igs003_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 vbowl_unk_r();
	void vbowl_pen_hi_w(u8 data);
	void vbowl_link_0_w(u16 data);
	void vbowl_link_1_w(u16 data);
	void vbowl_link_2_w(u16 data);
	void vbowl_link_3_w(u16 data);
	void sound_irq(int state);

	void screen_vblank(int state);
	void vbowl_decrypt();
	void vbowlhk_decrypt();
	void vbowl_gfx_unpack();

	void vbowl_mem(address_map &map) ATTR_COLD;
	void vbowlhk_mem(address_map &map) ATTR_COLD;
};


/***************************************************************************

    Video

    There are 8 non scrolling layers as big as the screen (512 x 256).
    Each layer has 256 colors and its own palette.

    There are 8 priority codes with RAM associated to each (8 x 256 values).
    For each screen position, to determine which pixel to display, the video
    chip associates a bit to the opacity of that pixel for each layer
    (1 = transparent) to form an address into the selected priority RAM.
    The value at that address (0-7) is the topmost layer.

***************************************************************************/


void igs011_state::igs011_priority_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_priority);

//  logerror("%06x: priority = %02x\n", m_maincpu->pc(), m_priority);

	if (data & ~0x7)
		logerror("%06x: warning, unknown bits written to priority = %02x\n", m_maincpu->pc(), m_priority);
}


void igs011_state::video_start()
{
	for (int i = 0; i < 8; i++)
	{
		m_layer[i] = std::make_unique<u8[]>(512 * 256);
		save_pointer(NAME(m_layer[i]), 512 * 256, i);
	}

	m_blitter_pen_hi = 0;

	save_item(NAME(m_priority));
	save_item(NAME(m_blitter_pen_hi));
	save_item(NAME(m_blitter.x));
	save_item(NAME(m_blitter.y));
	save_item(NAME(m_blitter.w));
	save_item(NAME(m_blitter.h));
	save_item(NAME(m_blitter.gfx_lo));
	save_item(NAME(m_blitter.gfx_hi));
	save_item(NAME(m_blitter.depth));
	save_item(NAME(m_blitter.pen));
	save_item(NAME(m_blitter.flags));
}

u32 igs011_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#ifdef MAME_DEBUG
	int layer_enable = -1;
#endif

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int mask = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  mask |= 0x01;
		if (machine().input().code_pressed(KEYCODE_W))  mask |= 0x02;
		if (machine().input().code_pressed(KEYCODE_E))  mask |= 0x04;
		if (machine().input().code_pressed(KEYCODE_R))  mask |= 0x08;
		if (machine().input().code_pressed(KEYCODE_A))  mask |= 0x10;
		if (machine().input().code_pressed(KEYCODE_S))  mask |= 0x20;
		if (machine().input().code_pressed(KEYCODE_D))  mask |= 0x40;
		if (machine().input().code_pressed(KEYCODE_F))  mask |= 0x80;
		if (mask)   layer_enable &= mask;
	}
#endif

	u16 const *const pri_ram = &m_priority_ram[(m_priority & 7) * 512/2];

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const int scr_addr = x + y * 512;
			int pri_addr = 0xff;

			int l;
			for (l = 0; l < 8; l++)
			{
				if ((m_layer[l][scr_addr] != 0xff)
#ifdef MAME_DEBUG
						&& (layer_enable & (1 << l))
#endif
					)
					pri_addr &= ~(1 << l);
			}

			l = pri_ram[pri_addr] & 7;

#ifdef MAME_DEBUG
			if ((layer_enable != -1) && (pri_addr == 0xff))
				bitmap.pix(y, x) = m_palette->black_pen();
			else
#endif
				bitmap.pix(y, x) = m_layer[l][scr_addr] | (l << 8);
		}
	}
	return 0;
}

/***************************************************************************

    In addition to the blitter, the CPU can also read from and write to
    the framebuffers for the 8 layers, seen as 0x100000 bytes in memory.
    The first half contains layers 0-3. Layers 4-7 are in the other half.

    The layers are interleaved:

    - bytes 0x00000-0x00003 contain the 1st pixel of layer 0,1,2,3
    - bytes 0x00004-0x00007 contain the 2nd pixel of layer 0,1,2,3
    ...
    - bytes 0x80000-0x80003 contain the 1st pixel of layer 4,5,6,7
    - bytes 0x80004-0x80007 contain the 2nd pixel of layer 4,5,6,7

    and so on.

***************************************************************************/

u16 igs011_state::igs011_layers_r(offs_t offset)
{
	const int layer0 = bitswap<2>(offset ^ 1, 18, 0) << 1;

	u8 const *const l0 = m_layer[layer0].get();
	u8 const *const l1 = m_layer[layer0 + 1].get();

	offset >>= 1;
	offset &= 0x1ffff;

	return (l0[offset] << 8) | l1[offset];
}

void igs011_state::igs011_layers_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int layer0 = bitswap<2>(offset ^ 1, 18, 0) << 1;

	u8 *const l0 = m_layer[layer0].get();
	u8 *const l1 = m_layer[layer0 + 1].get();

	offset >>= 1;
	offset &= 0x1ffff;

	u16 word = (l0[offset] << 8) | l1[offset];
	COMBINE_DATA(&word);
	l0[offset] = word >> 8;
	l1[offset] = word;
}

/***************************************************************************

    Blitter

***************************************************************************/


void igs011_state::igs011_blit_x_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.x);
}

void igs011_state::igs011_blit_y_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.y);
}

void igs011_state::igs011_blit_gfx_lo_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.gfx_lo);
}

void igs011_state::igs011_blit_gfx_hi_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.gfx_hi);
}

void igs011_state::igs011_blit_w_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.w);
}

void igs011_state::igs011_blit_h_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.h);
}

void igs011_state::igs011_blit_depth_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.depth);
}

void igs011_state::igs011_blit_pen_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.pen);
}


void igs011_state::igs011_blit_flags_w(offs_t offset, u16 data, u16 mem_mask)
{
	const rectangle &clip = m_screen->visible_area();

	COMBINE_DATA(&m_blitter.flags);

	LOGMASKED(LOG_BLITTER, "%06x: blit x %03x, y %03x, w %03x, h %03x, gfx %03x%04x, depth %02x, pen %02x, flags %03x\n", m_maincpu->pc(),
		m_blitter.x,m_blitter.y,m_blitter.w,m_blitter.h,m_blitter.gfx_hi,m_blitter.gfx_lo,m_blitter.depth,m_blitter.pen,m_blitter.flags);

	u8 *const dest = m_layer[m_blitter.flags & 0x0007].get();
	const bool opaque = BIT(~m_blitter.flags, 3);
	const bool clear  = BIT( m_blitter.flags, 4);
	const bool flipx  = BIT( m_blitter.flags, 5);
	const bool flipy  = BIT( m_blitter.flags, 6);
	if                 (BIT(~m_blitter.flags, 10))
		return;

	const u8 pen_hi = (m_blitter_pen_hi & 0x07) << 5;

	// pixel address
	u32 z = m_blitter.gfx_lo + (m_blitter.gfx_hi << 16);

	// what were they smoking???
	const bool depth4 = !((m_blitter.flags & 0x7) < (4 - (m_blitter.depth & 0x7))) ||
				(z & 0x800000);     // see lhb2

	z &= 0x7fffff;

	u8 trans_pen, clear_pen;

	if (depth4)
	{
		z <<= 1;
		if (m_gfx_hi && (m_blitter.gfx_hi & 0x80)) trans_pen = 0x1f;   // lhb2
		else                                       trans_pen = 0x0f;

		clear_pen = m_blitter.pen | 0xf0;
	}
	else
	{
		if (m_gfx_hi) trans_pen = 0x1f;   // vbowl
		else          trans_pen = 0xff;

		clear_pen = m_blitter.pen;
	}

	const int xstart = (m_blitter.x & 0x1ff) - (m_blitter.x & 0x200);
	const int ystart = (m_blitter.y & 0x0ff) - (m_blitter.y & 0x100);

	int xend, xinc;
	int yend, yinc;

	if (flipx)  { xend = xstart - (m_blitter.w & 0x1ff) - 1;  xinc = -1; }
	else        { xend = xstart + (m_blitter.w & 0x1ff) + 1;  xinc =  1; }

	if (flipy)  { yend = ystart - (m_blitter.h & 0x0ff) - 1;  yinc = -1; }
	else        { yend = ystart + (m_blitter.h & 0x0ff) + 1;  yinc =  1; }

	for (int y = ystart; y != yend; y += yinc)
	{
		for (int x = xstart; x != xend; x += xinc)
		{
			u8 pen = 0;
			// fetch the pixel
			if (!clear)
			{
				if (depth4) pen = (m_gfx[(z >> 1) % m_gfx.length()] >> (BIT(z, 0) << 2)) & 0x0f;
				else        pen = m_gfx[z % m_gfx.length()];

				if (m_gfx_hi)
				{
					pen &= 0x0f;
					if (BIT(m_gfx_hi[(z >> 3) % m_gfx_hi.length()], z & 7))
						pen |= 0x10;
				}
			}

			// plot it
			if (clip.contains(x, y))
			{
				if      (clear)            dest[x + y * 512] = clear_pen;
				else if (pen != trans_pen) dest[x + y * 512] = pen | pen_hi;
				else if (opaque)           dest[x + y * 512] = 0xff;
			}

			z++;
		}
	}

	#ifdef MAME_DEBUG
#if 1
	if (machine().input().code_pressed(KEYCODE_Z))
	{   char buf[20];
		sprintf(buf, "%02X%02X",m_blitter.depth,m_blitter.flags&0xff);
//      ui_draw_text(buf, m_blitter.x, m_blitter.y);    // crashes mame!
	}
#endif
	#endif
}

/***************************************************************************

    Common functions

***************************************************************************/

void igs011_state::machine_start()
{
	m_prot1_addr = 0;
	m_lhb_irq_enable = 0;

	save_item(NAME(m_dips_sel));
	save_item(NAME(m_input_sel));
	save_item(NAME(m_hopper_bit));
	save_item(NAME(m_prot1));
	save_item(NAME(m_prot1_swap));
	save_item(NAME(m_prot1_addr));
	save_item(NAME(m_prot2));
	save_item(NAME(m_igs012_prot));
	save_item(NAME(m_igs012_prot_swap));
	save_item(NAME(m_igs012_prot_mode));
	save_item(NAME(m_igs003_reg));
	save_item(NAME(m_lhb_irq_enable));
	save_item(NAME(m_igs003_prot_hold));
	save_item(NAME(m_igs003_prot_x));
	save_item(NAME(m_igs003_prot_y));
	save_item(NAME(m_igs003_prot_z));
	save_item(NAME(m_igs003_prot_h1));
	save_item(NAME(m_igs003_prot_h2));
}

// Inputs

u16 igs011_state::key_matrix_r()
{
	u16 ret = 0xffff;

	for (int i = 0; i < 5; i++)
	{
		if (BIT(~m_input_sel, i))
			ret &= m_io_key[i]->read();
	}

	return ret;
}

int igs011_state::igs_hopper_r()
{
	return (m_hopper_bit && ((m_screen->frame_number() / 5) & 1)) ? 0x0000 : 0x0001;
}

void igs011_state::dips_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dips_sel);
}

template<unsigned Num>
u16 igs011_state::dips_r()
{
	u16 ret = 0xff;

	for (int i = 0; i < Num; i++)
		if (BIT(~m_dips_sel, i))
			ret &= m_io_dsw[i]->read();

	// 0x0100 is blitter busy
	return  (ret & 0xff) | 0x0000;
}

// Games have 3 to 5 dips

/***************************************************************************

    Code Decryption

***************************************************************************/

void igs011_state::wlcc_decrypt()
{
	u16 *src = (u16 *) (m_maincpu_region->base());

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x2000) == 0x0000 || (i & 0x0004) == 0x0000 || (i & 0x0090) == 0x0000)
			x ^= 0x0004;
		if ((i & 0x0100) == 0x0100 || (i & 0x0040) == 0x0040 || (i & 0x0012) == 0x0012)
			x ^= 0x0020;
		if ((i & 0x2400) == 0x0000 || (i & 0x4100) == 0x4100 || ((i & 0x2000) == 0x2000 && (i & 0x0c00) != 0x0000))
			x ^= 0x0200;
		if ((x & 0x0024) == 0x0004 || (x & 0x0024) == 0x0020)
			x ^= 0x0024;
		src[i] = x;
	}
}


void igs011_state::lhb_decrypt()
{
	u16 *src = (u16 *) (m_maincpu_region->base());

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x1100) != 0x0100)
			x ^= 0x0200;

		if ((i & 0x0150) != 0x0000 && (i & 0x0152) != 0x0010)
			x ^= 0x0004;

		if ((i & 0x2084) != 0x2084 && (i & 0x2094) != 0x2014)
			x ^= 0x0020;

		src[i] = x;
	}
}


void igs011_state::drgnwrld_type3_decrypt()
{
	u16 *src = (u16 *) (m_maincpu_region->base());

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x2000) == 0x0000 || (i & 0x0004) == 0x0000 || (i & 0x0090) == 0x0000)
			x ^= 0x0004;

		if ((i & 0x0100) == 0x0100 || (i & 0x0040) == 0x0040 || (i & 0x0012) == 0x0012)
			x ^= 0x0020;

		if ((((i & 0x1000) == 0x1000) ^ ((i & 0x0100) == 0x0100))
			|| (i & 0x0880) == 0x0800 || (i & 0x0240) == 0x0240)
				x ^= 0x0200;

		if ((x & 0x0024) == 0x0004 || (x & 0x0024) == 0x0020)
			x ^= 0x0024;

		src[i] = x;
	}
}

void igs011_state::drgnwrld_type2_decrypt()
{
	u16 *src = (u16 *) (m_maincpu_region->base());

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if (((i & 0x000090) == 0x000000) || ((i & 0x002004) != 0x002004))
			x ^= 0x0004;

		if ((((i & 0x000050) == 0x000000) || ((i & 0x000142) != 0x000000)) && ((i & 0x000150) != 0x000000))
			x ^= 0x0020;

		if (((i & 0x004280) == 0x004000) || ((i & 0x004080) == 0x000000))
			x ^= 0x0200;

		if ((i & 0x0011a0) != 0x001000)
			x ^= 0x0200;

		if ((i & 0x000180) == 0x000100)
			x ^= 0x0200;

		if ((x & 0x0024) == 0x0020 || (x & 0x0024) == 0x0004)
			x ^= 0x0024;

		src[i] = x;
	}
}

void igs011_state::drgnwrld_type1_decrypt()
{
	u16 *src = (u16 *) (m_maincpu_region->base());

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x2000) == 0x0000 || (i & 0x0004) == 0x0000 || (i & 0x0090) == 0x0000)
			x ^= 0x0004;

		if ((i & 0x0100) == 0x0100 || (i & 0x0040) == 0x0040 || (i & 0x0012) == 0x0012)
			x ^= 0x0020;
/*
        if ((((i & 0x1000) == 0x1000) ^ ((i & 0x0100) == 0x0100))
            || (i & 0x0880) == 0x0800 || (i & 0x0240) == 0x0240)
                x ^= 0x0200;
*/
		if ((x & 0x0024) == 0x0004 || (x & 0x0024) == 0x0020)
			x ^= 0x0024;

		src[i] = x;
	}
}

void igs011_state::drgnwrldv40k_decrypt()
{
	drgnwrld_type1_decrypt();

	u16 *src = (u16 *) (m_maincpu_region->base());

	const int rom_size = 0x80000;

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = src[i];

		if ((i & 0x0800) != 0x0800)
			x ^= 0x0200;

		if (((i & 0x3a00) == 0x0a00) ^ ((i & 0x3a00) == 0x2a00))
			x ^= 0x0200;

		if (((i & 0x3ae0) == 0x0860) ^ ((i & 0x3ae0) == 0x2860))
			x ^= 0x0200;

		if (((i & 0x1c00) == 0x1800) ^ ((i & 0x1e00) == 0x1e00))
			x ^= 0x0200;

		if ((i & 0x1ee0) == 0x1c60)
			x ^= 0x0200;

		src[i] = x;
	}
}

void igs011_state::lhb2_decrypt()
{
	const int rom_size = 0x80000;
	u16 *src = (u16 *) (m_maincpu_region->base());
	std::vector<u16> result_data(rom_size/2);

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x0054) != 0x0000 && (i & 0x0056) != 0x0010)
			x ^= 0x0004;

		if ((i & 0x0204) == 0x0000)
			x ^= 0x0008;

		if ((i & 0x3080) != 0x3080 && (i & 0x3090) != 0x3010)
			x ^= 0x0020;

		const int j = bitswap<24>(i, 23,22,21,20,19,18,17,16,15,14,13, 8, 11,10, 9, 2, 7,6,5,4,3, 12, 1,0);

		result_data[j] = x;
	}

	memcpy(src,&result_data[0],rom_size);
}


// xor similar to ryukobou (both sets are Japan), address scrambling from lhb2
void igs011_state::nkishusp_decrypt()
{
	const int rom_size = 0x80000;
	u16 *src = (u16 *) (m_maincpu_region->base());
	std::vector<u16> result_data(rom_size/2);

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		// lhb2 address scrambling
		const int j = bitswap<24>(i, 23,22,21,20,19,18,17,16,15,14,13, 8, 11,10, 9, 2, 7,6,5,4,3, 12, 1,0);

		// ryukobou xor:

//      if ((j & 0x00100) && (j & 0x00400))
//          x ^= 0x0200;

		if (!(j & 0x00004) || !(j & 0x02000) || (!(j & 0x00080) && !(j & 0x00010)))
			x ^= 0x0020;

		if ((j & 0x00100) || (j & 0x00040) || ((j & 0x00010)&&(j & 0x00002)))
			x ^= 0x00004;

		// additional xor:

		if (!(j & 0x4000) && (j & 0x1000) && (j & 0x00200))
			x ^= 0x0008;

		result_data[j] = x;
	}

	memcpy(src,&result_data[0],rom_size);
}


void igs011_state::tygn_decrypt()
{
	const int rom_size = 0x80000;
	u16 *src = (u16 *) (m_maincpu_region->base());
	std::vector<u16> result_data(rom_size / 2);

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = src[i];

		const int j = bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 0, 2, 9, 5, 12, 14, 11, 8, 6, 3, 4, 16, 15, 13, 10, 7, 1);

		if ((j & 0x0280 / 2) || ((j & 0x0024 / 2) == 0x24 / 2))
			x ^= 0x0004;

		if (j & 0x4000 / 2)
		{
			if ((((j & 0x100 / 2) == 0x000 / 2) ^ ((j & 0x128 / 2) == 0x028 / 2)) || ((j & 0x108 / 2) == 0x100 / 2))
				x ^= 0x0020;
		}
		else
			x ^= 0x0020;

		if (!(j & 0x8000 / 2) && (j & 0x2000 / 2))
			x ^= 0x0008;

		result_data[j] = x;
	}

	memcpy(src, &result_data[0], rom_size);
}


void vbowl_state::vbowl_decrypt()
{
	u16 *src = (u16 *) (m_maincpu_region->base());

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x4100) == 0x0100)
			x ^= 0x0200;

		if ((i & 0x4000) == 0x4000 && (i & 0x0300) != 0x0100)
			x ^= 0x0200;

		if ((i & 0x5700) == 0x5100)
			x ^= 0x0200;

		if ((i & 0x5500) == 0x1000)
			x ^= 0x0200;

		if ((i & 0x0140) != 0x0000 || (i & 0x0012) == 0x0012)
			x ^= 0x0004;

		if ((i & 0x2004) != 0x2004 || (i & 0x0090) == 0x0000)
			x ^= 0x0020;

		src[i] = x;
	}
}


void vbowl_state::vbowlhk_decrypt()
{
	vbowl_decrypt();

	u16 *src = (u16 *) (m_maincpu_region->base());

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		// 00000-07fff, 2000 off, 800 off, 400 off, 200 on (2xx, 3xx)
		if ((i & 0xd700) == 0x0100)
			x ^= 0x0200;

		// 00000-07fff, 800 on, 400 off, 200 on (axx, bxx)
		if ((i & 0xc700) == 0x0500)
			x ^= 0x0200;

		// 08000-0ffff, 2000 off, 800 off, 200 off (0xx, 1xx, 4xx, 5xx)
		if ((i & 0xd500) == 0x4000)
			x ^= 0x0200;

		// 08000-0ffff, 800 on, 200 off (8xx, 9xx, cxx, dxx)
		if ((i & 0xc500) == 0x4400)
			x ^= 0x0200;

		// 10000-17fff, 2000 off, 200 off (0xx, 1xx, 4xx, 5xx, 8xx, 9xx, cxx, dxx)
		if ((i & 0xd100) == 0x8000)
			x ^= 0x0200;

		// 10000-17fff, 2000 on, 800 on, 400 off, 200 on (axx, bxx)
		if ((i & 0xd700) == 0x9500)
			x ^= 0x0200;

		// 18000-1ffff, 2000 off, 400 off, 200 on (2xx, 3xx, axx, bxx)
		if ((i & 0xd300) == 0xc100)
			x ^= 0x0200;

		// 18000-1ffff, 2000 on, 800 on, 200 off (8xx, 9xx, cxx, dxx)
		if ((i & 0xd500) == 0xd400)
			x ^= 0x0200;

		src[i] = x;
	}
}


void igs011_state::dbc_decrypt()
{
	u16 *src = (u16 *) (m_maincpu_region->base());

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if (i & 0x1000/2)
		{
			if (~i & 0x400/2)
				x ^= 0x0200;
		}

		if (i & 0x4000/2)
		{
			if (i & 0x100/2)
			{
				if (~i & 0x08/2)
					x ^= 0x0020;
			}
			else
			{
				if (~i & 0x28/2)
					x ^= 0x0020;
			}
		}
		else
		{
			x ^= 0x0020;
		}

		if (i & 0x200/2)
		{
			x ^= 0x0004;
		}
		else
		{
			if ((i & 0x80/2) == 0x80/2 || (i & 0x24/2) == 0x24/2)
				x ^= 0x0004;
		}

		src[i] = x;
	}
}


void igs011_state::ryukobou_decrypt()
{
	u16 *src = (u16 *) m_maincpu_region->base();
	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x00100) && (i & 0x00400))
			x ^= 0x0200;

		if (!(i & 0x00004) || !(i & 0x02000) || (!(i & 0x00080) && !(i & 0x00010)))
			x ^= 0x0020;

		if ((i & 0x00100) || (i & 0x00040) || ((i & 0x00010)&&(i & 0x00002)))
			x ^= 0x00004;

		src[i] = x;
	}
}


/***************************************************************************

    Gfx Decryption

***************************************************************************/


void igs011_state::lhb2_gfx_decrypt()
{
	const unsigned rom_size = 0x200000;
	u8 *src = (u8 *) (memregion("blitter")->base());
	std::vector<u8> result_data(rom_size);

	for (int i=0; i<rom_size; i++)
		result_data[i] = src[bitswap<24>(i, 23,22,21,20, 19, 17,16,15, 13,12, 10,9,8,7,6,5,4, 2,1, 3, 11, 14, 18, 0)];

	memcpy(src,&result_data[0],rom_size);
}

void igs011_state::drgnwrld_gfx_decrypt()
{
	const unsigned rom_size = 0x400000;
	u8 *src = (u8 *) (memregion("blitter")->base());
	std::vector<u8> result_data(rom_size);

	for (int i=0; i<rom_size; i++)
		result_data[i] = src[bitswap<24>(i, 23,22,21,20,19,18,17,16,15, 12, 13, 14, 11,10,9,8,7,6,5,4,3,2,1,0)];

	memcpy(src,&result_data[0],rom_size);
}

void vbowl_state::vbowl_gfx_unpack()
{
	u8 *gfx = (u8 *)memregion("blitter")->base();
	for (int i = 0x400000-1; i >= 0; i--)
	{
		gfx[i * 2 + 1] = (gfx[i] & 0xf0) >> 4;
		gfx[i * 2 + 0] = (gfx[i] & 0x0f) >> 0;
	}
}


/***************************************************************************

    IGS011 Protection

    Protection 1 ("ASIC11 CHECK PORT ERROR")

    The chip holds an internal value, a buffered value and an address base register.
    The address base register determines where the protection device is mapped in memory
    (0x00000-0xffff0), has itself a fixed address, and writes to it reset the state.
    The internal and buffered value are manipulated by issuing commands, where
    each command is assigned a specific offset, and is triggered by writing a specific
    byte value to that offset:

    Offs.   R/W     Result
    0         W     COPY: copy buffer to value
    2         W     INC:  increment value
    4         W     DEC:  decrement value
    6         W     SWAP: write bitswap1(value) to buffer
    8       R       READ: read bitswap2(value). Only 2 bits are checked (bitmask 0x24).

    Protection 2 ("CHECK PORT ERROR")

    This is probably not part of the IGS011 nor the IGS012, but a game specific protection
    similar to the above.

    The chip holds an internal value. It is manipulated by issuing commands,
    where each command is assigned a specific address range (fixed per game), and is
    triggered by writing a specific byte value to that range. Possible commands:

    - INC:   increment value
    - DEC:   decrement value
    - SWAP:  value = bitswap1(value). Bitswap1 is game specific.
    - RESET: value = 0

    The protection value is read from an additional address range:
    - READ:  read bitswap2(value). Only 1 bit is checked. Bitswap2 is game specific.

***************************************************************************/


void igs011_state::igs011_prot1_w(offs_t offset, u8 data)
{
	offset *= 2;

	switch (offset)
	{
		case 0: // COPY
			if ((data & 0xff) == 0x33)
			{
				m_prot1 = m_prot1_swap;
				return;
			}
			break;

		case 2: // INC
			if ((data & 0xff) == 0xff)
			{
				m_prot1++;
				return;
			}
			break;

		case 4: // DEC
			if ((data & 0xff) == 0xaa)
			{
				m_prot1--;
				return;
			}
			break;

		case 6: // SWAP
			if ((data & 0xff) == 0x55)
			{
				// b1 . (b2|b3) . b2 . (b0&b3)
				u8 x = m_prot1;
				m_prot1_swap = (BIT(x,1)<<3) | ((BIT(x,2)|BIT(x,3))<<2) | (BIT(x,2)<<1) | (BIT(x,0)&BIT(x,3));
				return;
			}
			break;
	}

	logerror("%s: warning, unknown igs011_prot1_w( %04x, %02x )\n", machine().describe_context(), offset, data);
}
u16 igs011_state::igs011_prot1_r()
{
	// !(b1&b2) . 0 . 0 . (b0^b3) . 0 . 0
	u8 x = m_prot1;
	return (((BIT(x,1)&BIT(x,2))^1)<<5) | ((BIT(x,0)^BIT(x,3))<<2);
}


void igs011_state::igs011_prot_addr_w(u16 data)
{
	m_prot1 = 0x00;
	m_prot1_swap = 0x00;

//  m_prot2 = 0x00;

	address_space &sp = m_maincpu->space(AS_PROGRAM);
	u8 *rom = m_maincpu_region->base();

	// Plug previous address range with ROM access
	sp.install_rom(m_prot1_addr + 0, m_prot1_addr + 9, rom + m_prot1_addr);

	m_prot1_addr = (data << 4) ^ 0x8340;

	prot_mem_range_set();
}

void igs011_state::prot_mem_range_set()
{
	address_space &sp = m_maincpu->space(AS_PROGRAM);

	// Add protection memory range
	sp.install_write_handler(m_prot1_addr + 0, m_prot1_addr + 7, write8sm_delegate(*this, FUNC(igs011_state::igs011_prot1_w)), 0xff00);
	sp.install_read_handler (m_prot1_addr + 8, m_prot1_addr + 9, read16smo_delegate(*this, FUNC(igs011_state::igs011_prot1_r)));
}
/*
u16 igs011_state::igs011_prot_fake_r(offs_t offset)
{
    switch (offset)
    {
        case 0: return m_prot1;
        case 1: return m_prot1_swap;
        case 2: return m_prot2;
    }
    return 0;
}
*/


// Prot2

// drgnwrld (33)
void igs011_state::igs011_prot2_reset_w(u8 data)
{
	m_prot2 = 0x00;
}

// wlcc
u16 igs011_state::igs011_prot2_reset_r()
{
	if (!machine().side_effects_disabled())
		m_prot2 = 0x00;

	return 0;
}


// lhb2 (55), lhb/dbc/ryukobou (33)
void igs011_state::igs011_prot2_inc_w(u8 data)
{
//  if ((data & 0xff) == 0x55)
	{
		m_prot2++;
	}
//  else
//      logerror("%s: warning, unknown igs011_prot2_inc_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}

// vbowl (33)
void igs011_state::igs011_prot2_dec_w(u8 data)
{
//  if ((data & 0xff) == 0x33)
	{
		m_prot2--;
	}
//  else
//      logerror("%s: warning, unknown igs011_prot2_dec_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}


void igs011_state::drgnwrld_igs011_prot2_swap_w(u8 data)
{
//  if ((data & 0xff) == 0x33)
	{
		// (b3&b0) . b2 . (b0|b1) . (b2^!b4) . (!b1^b3)
		u8 x = m_prot2;
		m_prot2 = ((BIT(x,3)&BIT(x,0))<<4) | (BIT(x,2)<<3) | ((BIT(x,0)|BIT(x,1))<<2) | ((BIT(x,2)^BIT(x,4)^1)<<1) | (BIT(x,1)^1^BIT(x,3));
	}
//  else
//      logerror("%s: warning, unknown igs011_prot2_swap_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}

// lhb, xymg, lhb2
void igs011_state::lhb_igs011_prot2_swap_w(u8 data)
{
//  if ((data & 0xff) == 0x33)
	{
		// (!b0|b1) . b2 . (b0&b1)
		u8 x = m_prot2;
		m_prot2 = (((BIT(x,0)^1)|BIT(x,1))<<2) | (BIT(x,2)<<1) | (BIT(x,0)&BIT(x,1));
	}
//  else
//      logerror("%s: warning, unknown igs011_prot2_swap_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}

// wlcc
void igs011_state::wlcc_igs011_prot2_swap_w(u8 data)
{
//  if ((data & 0xff) == 0x33)
	{
		// (b3 ^ b2) . (b2 ^ b1) . (b1 ^ b0) . !(b4 ^ b0) . !(b4 ^ b3)
		u8 x = m_prot2;
		m_prot2 = ((BIT(x,3)^BIT(x,2))<<4) | ((BIT(x,2)^BIT(x,1))<<3) | ((BIT(x,1)^BIT(x,0))<<2) | ((BIT(x,4)^BIT(x,0)^1)<<1) | (BIT(x,4)^BIT(x,3)^1);
	}
//  else
//      logerror("%s: warning, unknown igs011_prot2_swap_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}

// vbowl
void vbowl_state::vbowl_igs011_prot2_swap_w(u8 data)
{
//  if ((data & 0xff) == 0x33)
	{
		// (b3 ^ b2) . (b2 ^ b1) . (b1 ^ b0) . (b4 ^ b0) . (b4 ^ b3)
		u8 x = m_prot2;
		m_prot2 = ((BIT(x,3)^BIT(x,2))<<4) | ((BIT(x,2)^BIT(x,1))<<3) | ((BIT(x,1)^BIT(x,0))<<2) | ((BIT(x,4)^BIT(x,0))<<1) | (BIT(x,4)^BIT(x,3));
	}
//  else
//      logerror("%s: warning, unknown igs011_prot2_swap_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}


// drgnwrld
u16 igs011_state::drgnwrldv40k_igs011_prot2_r()
{
	// b9 = (!b4 & !b0) | ((b3 & !b2) & !(b1 ^ b0)
	u8 x = m_prot2;
	u8 b9 = ((BIT(x, 4) ^ 1) & (BIT(x, 0) ^ 1)) | (((BIT(x, 3)) & ((BIT(x, 2)) ^ 1)) & (((BIT(x, 1)) ^ (BIT(x, 0))) ^ 1));
	return (b9 << 9);
}

u16 igs011_state::drgnwrldv21_igs011_prot2_r()
{
	// b9 = (!b4) | (!b0 & b2) | (!(b3 ^ b1) & !(!(b4 & b0) | b2))
	u8 x = m_prot2;
	u8 b9 = (BIT(x,4)^1) | ((BIT(x,0)^1) & BIT(x,2)) | ((BIT(x,3)^BIT(x,1)^1) & ((((BIT(x,4)^1) & BIT(x,0)) | BIT(x,2))^1) );
	return (b9 << 9);
}
u16 igs011_state::drgnwrldv20j_igs011_prot2_r()
{
	// b9 = (!b4 | !b0) | !(b3 | b1) | !(b2 & b0)
	u8 x = m_prot2;
	u8 b9 = ((BIT(x,4)^1) | (BIT(x,0)^1)) | ((BIT(x,3) | BIT(x,1))^1) | ((BIT(x,2) & BIT(x,0))^1);
	return (b9 << 9);
}

// lhb, xymg
u16 igs011_state::lhb_igs011_prot2_r()
{
	// b9 = !b2 | (b1 & b0)
	u8 x = m_prot2;
	u8 b9 = (BIT(x,2)^1) | (BIT(x,1) & BIT(x,0));
	return (b9 << 9);
}

// dbc
u16 igs011_state::dbc_igs011_prot2_r()
{
	// b9 = !b1 | (!b0 & b2)
	u8 x = m_prot2;
	u8 b9 = (BIT(x,1)^1) | ((BIT(x,0)^1) & BIT(x,2));
	return (b9 << 9);
}

// ryukobou
u16 igs011_state::ryukobou_igs011_prot2_r()
{
	// b9 = (!b1 | b2) & b0
	u8 x = m_prot2;
	u8 b9 = ((BIT(x,1)^1) | BIT(x,2)) & BIT(x,0);
	return (b9 << 9);
}

// lhb2
u16 igs011_state::lhb2_igs011_prot2_r()
{
	// b3 = !b2 | !b1 | b0
	u8 x = m_prot2;
	u8 b3 = (BIT(x,2)^1) | (BIT(x,1)^1) | BIT(x,0);
	return (b3 << 3);
}

// vbowl, vbowlj
u16 vbowl_state::vbowl_igs011_prot2_r()
{
	// b9 = (!b4 & !b3) | !(b2 & b1) | !(b4 | b0)
	u8 x = m_prot2;
	u8 b9 = ((BIT(x,4)^1) & (BIT(x,3)^1)) | ((BIT(x,2) & BIT(x,1))^1) | ((BIT(x,4) | BIT(x,0))^1);
	return (b9 << 9);
}

// vbowlhk
u16 vbowl_state::vbowlhk_igs011_prot2_r()
{
	// b9 = (!b4 & !b3) | !(!b2 & !b1) | !(b4 | !b0)
	u8 x = m_prot2;
	u8 b9 = ((BIT(x,4)^1) & (BIT(x,3)^1)) | (((BIT(x,2)^1) & (BIT(x,1)^1))^1) | ((BIT(x,4) | (BIT(x,0)^1))^1);
	return (b9 << 9);
}

/***************************************************************************

    IGS012 Protection ("ASIC12 CHECK PORT ERROR")

    The chip holds an internal value, a buffered value and a mode.
    These are manipulated by issuing commands, where each command is assigned
    a specific address range, and is triggered by writing a specific byte value
    to that range. Possible commands:

    - INC:   increment value
    - DEC:   decrement value
    - SWAP:  write bitswap1(value) to buffer
    - COPY:  copy buffer to value
    - MODE:  toggle mode (toggles address ranges to write/read and byte values to write)
    - RESET: value = 0, mode = 0

    The protection value is read from an additional address range:
    - READ: read bitswap2(value). Only 2 bits are checked.

***************************************************************************/


void igs011_state::igs012_prot_reset_w(u16 data)
{
	m_igs012_prot = 0x00;
	m_igs012_prot_swap = 0x00;

	m_igs012_prot_mode = 0;
}
/*
u16 igs011_state::igs012_prot_fake_r(offs_t offset)
{
    switch (offset)
    {
        case 0: return m_igs012_prot;
        case 1: return m_igs012_prot_swap;
        case 2: return m_igs012_prot_mode;
    }
    return 0;
}
*/

// Macro that checks whether the current mode and data byte written match the arguments
#define MODE_AND_DATA(_MODE,_DATA)  (m_igs012_prot_mode == (_MODE) && ((data & 0xff) == (_DATA)) )

void igs011_state::igs012_prot_mode_w(offs_t offset, u8 data)
{
	if (MODE_AND_DATA(0, 0xcc) || MODE_AND_DATA(1, 0xcc) || MODE_AND_DATA(0, 0xdd) || MODE_AND_DATA(1, 0xdd))
	{
		m_igs012_prot_mode = m_igs012_prot_mode ^ 1;
	}
	else
		logerror("%s: warning, unknown igs012_prot_mode_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_igs012_prot_mode);
}

void igs011_state::igs012_prot_inc_w(offs_t offset, u8 data)
{
	if (MODE_AND_DATA(0, 0xff) )
	{
		m_igs012_prot = (m_igs012_prot + 1) & 0x1f;
	}
	else
		logerror("%s: warning, unknown igs012_prot_inc_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_igs012_prot_mode);
}

void igs011_state::igs012_prot_dec_inc_w(offs_t offset, u8 data)
{
	if (MODE_AND_DATA(0, 0xaa) )
	{
		m_igs012_prot = (m_igs012_prot - 1) & 0x1f;
	}
	else if (MODE_AND_DATA(1, 0xfa) )
	{
		m_igs012_prot = (m_igs012_prot + 1) & 0x1f;
	}
	else
		logerror("%s: warning, unknown igs012_prot_dec_inc_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_igs012_prot_mode);
}

void igs011_state::igs012_prot_dec_copy_w(offs_t offset, u8 data)
{
	if (MODE_AND_DATA(0, 0x33) )
	{
		m_igs012_prot = m_igs012_prot_swap;
	}
	else if (MODE_AND_DATA(1, 0x5a) )
	{
		m_igs012_prot = (m_igs012_prot - 1) & 0x1f;
	}
	else
		logerror("%s: warning, unknown igs012_prot_dec_copy_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_igs012_prot_mode);
}

void igs011_state::igs012_prot_copy_w(offs_t offset, u8 data)
{
	if (MODE_AND_DATA(1, 0x22) )
	{
		m_igs012_prot = m_igs012_prot_swap;
	}
	else
		logerror("%s: warning, unknown igs012_prot_copy_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_igs012_prot_mode);
}

void igs011_state::igs012_prot_swap_w(offs_t offset, u8 data)
{
	if (MODE_AND_DATA(0, 0x55) || MODE_AND_DATA(1, 0xa5) )
	{
		// !(3 | 1)..(2 & 1)..(3 ^ 0)..(!2)
		u8 x = m_igs012_prot;
		m_igs012_prot_swap = (((BIT(x,3)|BIT(x,1))^1)<<3) | ((BIT(x,2)&BIT(x,1))<<2) | ((BIT(x,3)^BIT(x,0))<<1) | (BIT(x,2)^1);
	}
	else
		logerror("%s: warning, unknown igs012_prot_swap_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_igs012_prot_mode);
}

u16 igs011_state::igs012_prot_r()
{
	// FIXME: mode 0 and mode 1 are mapped to different memory ranges
	u8 x = m_igs012_prot;

	u8 b1 = (BIT(x,3) | BIT(x,1))^1;
	u8 b0 = BIT(x,3) ^ BIT(x,0);

	return (b1 << 1) | (b0 << 0);
}

/***************************************************************************

    IGS003 (8255 I/O + Protection)

***************************************************************************/

void igs011_state::igs003_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_igs003_reg);
}

void igs011_state::drgnwrld_igs003_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (m_igs003_reg)
	{
		case 0x00:
			if (ACCESSING_BITS_0_7)
				machine().bookkeeping().coin_counter_w(0, BIT(data, 1));

			if (data & ~0x2)
				logerror("%06x: warning, unknown bits written in coin counter = %02x\n", m_maincpu->pc(), data);

			break;

//      case 0x01:
		// 0,1,4 written

//      case 0x03:
//      case 0x04:
//      case 0x05:

		default:
//          popmessage("igs003 %x <- %04x",m_igs003_reg,data);
			logerror("%06x: warning, writing to igs003_reg %02x = %02x\n", m_maincpu->pc(), m_igs003_reg, data);
	}
}
u16 igs011_state::drgnwrld_igs003_r()
{
	switch (m_igs003_reg)
	{
		case 0x00:  return m_io_in[0]->read();
		case 0x01:  return m_io_in[1]->read();
		case 0x02:  return m_io_in[2]->read();

		case 0x20:  return 0x49;
		case 0x21:  return 0x47;
		case 0x22:  return 0x53;

		case 0x24:  return 0x41;
		case 0x25:  return 0x41;
		case 0x26:  return 0x7f;
		case 0x27:  return 0x41;
		case 0x28:  return 0x41;

		case 0x2a:  return 0x3e;
		case 0x2b:  return 0x41;
		case 0x2c:  return 0x49;
		case 0x2d:  return 0xf9;
		case 0x2e:  return 0x0a;

		case 0x30:  return 0x26;
		case 0x31:  return 0x49;
		case 0x32:  return 0x49;
		case 0x33:  return 0x49;
		case 0x34:  return 0x32;

		default:
			logerror("%06x: warning, reading with igs003_reg = %02x\n", m_maincpu->pc(), m_igs003_reg);
	}

	return 0;
}

void igs011_state::lhb_inputs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_input_sel);

	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, BIT(data, 5));
		//  coin out        BIT(data, 6)
		m_hopper_bit = BIT(data, 7);
	}

	if (m_input_sel & (~0xff))
		logerror("%06x: warning, unknown bits written in input_sel = %02x\n", m_maincpu->pc(), m_input_sel);

//  popmessage("sel2 %02x",m_input_sel&~0x1f);
}
u16 igs011_state::lhb_inputs_r(offs_t offset)
{
	switch (offset)
	{
		case 0: return m_input_sel;
		case 1: return key_matrix_r();
	}
	return 0;
}


void igs011_state::lhb2_igs003_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (m_igs003_reg)
	{
		case 0x00:
			COMBINE_DATA(&m_input_sel);

			if (ACCESSING_BITS_0_7)
			{
				machine().bookkeeping().coin_counter_w(0, BIT(data, 5));
				//  coin out        BIT(data, 6)
				m_hopper_bit        = BIT(data, 7);
			}

			if (m_input_sel & ~0x7f)
				logerror("%06x: warning, unknown bits written in input_sel = %02x\n", m_maincpu->pc(), m_input_sel);

//          popmessage("sel2 %02x",m_input_sel&~0x1f);
			break;

		case 0x02:
			if (ACCESSING_BITS_0_7)
			{
				m_blitter_pen_hi = data & 0x07;

				m_oki->set_rom_bank(BIT(data, 3));
			}

			if (m_blitter_pen_hi & ~0xf)
				logerror("%06x: warning, unknown bits written in blitter_pen_hi = %02x\n", m_maincpu->pc(), m_blitter_pen_hi);

//          popmessage("oki %02x",m_blitter_pen_hi & 0x08);
			break;

		case 0x40:
			m_igs003_prot_h2 = m_igs003_prot_h1;
			m_igs003_prot_h1 = data;
			break;

		case 0x41: // data written here is the same as reg 40
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
			break;

		case 0x48:
			m_igs003_prot_x = 0;
			if ((m_igs003_prot_h2 & 0x0a) != 0x0a)   m_igs003_prot_x |= 0x08; // $100de6 (always 0 in lhb2)
			if ((m_igs003_prot_h2 & 0x90) != 0x90)   m_igs003_prot_x |= 0x04; // $100de7 (always 0 in lhb2)
			if ((m_igs003_prot_h1 & 0x06) != 0x06)   m_igs003_prot_x |= 0x02; // $100de8
			if ((m_igs003_prot_h1 & 0x90) != 0x90)   m_igs003_prot_x |= 0x01; // $100de9
			break;

		case 0x50: // reset?
			m_igs003_prot_hold = 0;
			break;

		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
			{
				m_igs003_prot_y = m_igs003_reg & 0x07;
				m_igs003_prot_z = data;

				const u16 old = m_igs003_prot_hold;

				// rotate (with some bits inverted)
				m_igs003_prot_hold <<= 1;
				m_igs003_prot_hold |= BIT(old, 15);
				m_igs003_prot_hold ^= 0x2bad;
				// xor bit 0
				m_igs003_prot_hold ^= BIT(old, 12);
				m_igs003_prot_hold ^= BIT(old,  8);
				m_igs003_prot_hold ^= BIT(old,  3);
				m_igs003_prot_hold ^= BIT(m_igs003_prot_z, m_igs003_prot_y);
				// xor other 4 bits
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 0) <<  4;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 1) <<  6;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 2) << 10;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 3) << 12;
			}
			break;

		default:
//          popmessage("igs003 %x <- %04x",m_igs003_reg,data);
			logerror("%06x: warning, writing to igs003_reg %02x = %02x\n", m_maincpu->pc(), m_igs003_reg, data);
	}
}
u16 igs011_state::lhb2_igs003_r()
{
	switch (m_igs003_reg)
	{
		case 0x01:
			return key_matrix_r();
		default:
			if (!machine().side_effects_disabled())
				logerror("%06x: warning, reading with igs003_reg = %02x\n", m_maincpu->pc(), m_igs003_reg);
			break;

		case 0x03:
			return bitswap<16>(m_igs003_prot_hold, 14,11,8,6,4,3,1,0, 5,2,9,7,10,13,12,15) & 0xff;

		// Protection:
		// 0544FE: 20 21 22 24 25 26 27 28 2A 2B 2C 2D 2E 30 31 32 33 34
		// 0544EC: 49 47 53 41 41 7F 41 41 3E 41 49 F9 0A 26 49 49 49 32

		case 0x20:  return 0x49;
		case 0x21:  return 0x47;
		case 0x22:  return 0x53;

		case 0x24:  return 0x41;
		case 0x25:  return 0x41;
		case 0x26:  return 0x7f;
		case 0x27:  return 0x41;
		case 0x28:  return 0x41;

		case 0x2a:  return 0x3e;
		case 0x2b:  return 0x41;
		case 0x2c:  return 0x49;
		case 0x2d:  return 0xf9;
		case 0x2e:  return 0x0a;

		case 0x30:  return 0x26;
		case 0x31:  return 0x49;
		case 0x32:  return 0x49;
		case 0x33:  return 0x49;
		case 0x34:  return 0x32;
	}

	return 0;
}


void igs011_state::wlcc_igs003_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (m_igs003_reg)
	{
		case 0x02:
			if (ACCESSING_BITS_0_7)
			{
				machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
				//  coin out BIT(data, 1)

				m_oki->set_rom_bank(BIT(data, 4));
				m_hopper_bit = BIT(data, 5);
			}

			if (data & ~0x33)
				logerror("%06x: warning, unknown bits written in coin counter = %02x\n", m_maincpu->pc(), data);

//          popmessage("coin %02x",data);
			break;

		default:
			logerror("%06x: warning, writing to igs003_reg %02x = %02x\n", m_maincpu->pc(), m_igs003_reg, data);
	}
}
u16 igs011_state::wlcc_igs003_r()
{
	switch (m_igs003_reg)
	{
		case 0x00:  return m_io_in[0]->read();

		case 0x20:  return 0x49;
		case 0x21:  return 0x47;
		case 0x22:  return 0x53;

		case 0x24:  return 0x41;
		case 0x25:  return 0x41;
		case 0x26:  return 0x7f;
		case 0x27:  return 0x41;
		case 0x28:  return 0x41;

		case 0x2a:  return 0x3e;
		case 0x2b:  return 0x41;
		case 0x2c:  return 0x49;
		case 0x2d:  return 0xf9;
		case 0x2e:  return 0x0a;

		case 0x30:  return 0x26;
		case 0x31:  return 0x49;
		case 0x32:  return 0x49;
		case 0x33:  return 0x49;
		case 0x34:  return 0x32;

		default:
			if (!machine().side_effects_disabled())
				logerror("%06x: warning, reading with igs003_reg = %02x\n", m_maincpu->pc(), m_igs003_reg);
	}

	return 0;
}


void igs011_state::xymg_igs003_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (m_igs003_reg)
	{
		case 0x01:
			COMBINE_DATA(&m_input_sel);

			if (ACCESSING_BITS_0_7)
			{
				machine().bookkeeping().coin_counter_w(0, BIT(data, 5));
				//  coin out        BIT(data, 6)
				m_hopper_bit        = BIT(data, 7);
			}

			if (m_input_sel & 0x40)
				logerror("%06x: warning, unknown bits written in input_sel = %02x\n", m_maincpu->pc(), m_input_sel);

//          popmessage("sel2 %02x",m_input_sel&~0x1f);
			break;

		default:
			logerror("%06x: warning, writing to igs003_reg %02x = %02x\n", m_maincpu->pc(), m_igs003_reg, data);
	}
}
u16 igs011_state::xymg_igs003_r()
{
	switch (m_igs003_reg)
	{
		case 0x00:  return m_io_coin->read();

		case 0x02:  return key_matrix_r();

		case 0x20:  return 0x49;
		case 0x21:  return 0x47;
		case 0x22:  return 0x53;

		case 0x24:  return 0x41;
		case 0x25:  return 0x41;
		case 0x26:  return 0x7f;
		case 0x27:  return 0x41;
		case 0x28:  return 0x41;

		case 0x2a:  return 0x3e;
		case 0x2b:  return 0x41;
		case 0x2c:  return 0x49;
		case 0x2d:  return 0xf9;
		case 0x2e:  return 0x0a;

		case 0x30:  return 0x26;
		case 0x31:  return 0x49;
		case 0x32:  return 0x49;
		case 0x33:  return 0x49;
		case 0x34:  return 0x32;

		default:
			if (!machine().side_effects_disabled())
				logerror("%06x: warning, reading with igs003_reg = %02x\n", m_maincpu->pc(), m_igs003_reg);
			break;
	}

	return 0;
}


// vbowl, vbowlj
void vbowl_state::vbowl_igs003_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (m_igs003_reg)
	{
		case 0x02:
			if (ACCESSING_BITS_0_7)
			{
				machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
				machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
			}

			if (data & ~0x3)
				logerror("%06x: warning, unknown bits written in coin counter = %02x\n", m_maincpu->pc(), data);

			break;

		case 0x40:
			m_igs003_prot_h2 = m_igs003_prot_h1;
			m_igs003_prot_h1 = data;
			break;

		case 0x41: // repeat data written at $40
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
			break;

		case 0x48:
			m_igs003_prot_x = 0;
			if ((m_igs003_prot_h2 & 0x0a) != 0x0a)   m_igs003_prot_x |= 0x08;
			if ((m_igs003_prot_h2 & 0x90) != 0x90)   m_igs003_prot_x |= 0x04;
			if ((m_igs003_prot_h1 & 0x06) != 0x06)   m_igs003_prot_x |= 0x02;
			if ((m_igs003_prot_h1 & 0x90) != 0x90)   m_igs003_prot_x |= 0x01;
			break;

		case 0x50: // reset?
			m_igs003_prot_hold = 0;
			break;

		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
			{
				m_igs003_prot_y = m_igs003_reg & 0x07;
				m_igs003_prot_z = data;

				const u16 old = m_igs003_prot_hold;

				// rotate (with some bits inverted)
				m_igs003_prot_hold <<= 1;
				m_igs003_prot_hold |= BIT(old, 15);
				m_igs003_prot_hold ^= 0x2bad;
				// xor bit 0
				m_igs003_prot_hold ^= BIT(old, 10);
				m_igs003_prot_hold ^= BIT(old,  8);
				m_igs003_prot_hold ^= BIT(old,  5);
				m_igs003_prot_hold ^= BIT(m_igs003_prot_z, m_igs003_prot_y);
				// xor other 4 bits
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 0) <<  4;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 1) <<  6;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 2) << 10;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 3) << 12;
			}
			break;

		default:
//          popmessage("igs003 %x <- %04x",m_igs003_reg,data);
			logerror("%06x: warning, writing to igs003_reg %02x = %02x\n", m_maincpu->pc(), m_igs003_reg, data);
	}
}
u16 vbowl_state::vbowl_igs003_r()
{
	switch (m_igs003_reg)
	{
		case 0x00:  return m_io_in[0]->read();
		case 0x01:  return m_io_in[1]->read();

		case 0x03:
			return bitswap<16>(m_igs003_prot_hold, 14,11,8,6,4,3,1,0, 5,2,9,7,10,13,12,15) & 0xff;

		case 0x20:  return 0x49;
		case 0x21:  return 0x47;
		case 0x22:  return 0x53;

		case 0x24:  return 0x41;
		case 0x25:  return 0x41;
		case 0x26:  return 0x7f;
		case 0x27:  return 0x41;
		case 0x28:  return 0x41;

		case 0x2a:  return 0x3e;
		case 0x2b:  return 0x41;
		case 0x2c:  return 0x49;
		case 0x2d:  return 0xf9;
		case 0x2e:  return 0x0a;

		case 0x30:  return 0x26;
		case 0x31:  return 0x49;
		case 0x32:  return 0x49;
		case 0x33:  return 0x49;
		case 0x34:  return 0x32;

		default:
			if (!machine().side_effects_disabled())
				logerror("%06x: warning, reading with igs003_reg = %02x\n", m_maincpu->pc(), m_igs003_reg);
	}

	return 0;
}


// vbowlhk (different bitswap)
void vbowl_state::vbowlhk_igs003_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (m_igs003_reg)
	{
		case 0x02:
			if (ACCESSING_BITS_0_7)
			{
				machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
				machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
			}

			if (data & ~0x3)
				logerror("%06x: warning, unknown bits written in coin counter = %02x\n", m_maincpu->pc(), data);

			break;

		case 0x40:
			m_igs003_prot_h2 = m_igs003_prot_h1;
			m_igs003_prot_h1 = data;
			break;

		case 0x41: // repeat data written at $40
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
			break;

		case 0x48:
			m_igs003_prot_x = 0;
			if ((m_igs003_prot_h2 & 0x0a) != 0x0a)   m_igs003_prot_x |= 0x08;
			if ((m_igs003_prot_h2 & 0x90) != 0x90)   m_igs003_prot_x |= 0x04;
			if ((m_igs003_prot_h1 & 0x06) != 0x06)   m_igs003_prot_x |= 0x02;
			if ((m_igs003_prot_h1 & 0x90) != 0x90)   m_igs003_prot_x |= 0x01;
			break;

		case 0x50: // reset?
			m_igs003_prot_hold = 0;
			break;

		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
			{
				m_igs003_prot_y = m_igs003_reg & 0x07;
				m_igs003_prot_z = data;

				const u16 old = m_igs003_prot_hold;

				// rotate (with some bits inverted)
				m_igs003_prot_hold <<= 1;
				m_igs003_prot_hold |= BIT(old, 15);
				m_igs003_prot_hold ^= 0x2bad;
				// xor bit 0
				m_igs003_prot_hold ^= BIT(old, 7);
				m_igs003_prot_hold ^= BIT(old, 6);
				m_igs003_prot_hold ^= BIT(old, 5);
				m_igs003_prot_hold ^= BIT(m_igs003_prot_z, m_igs003_prot_y);
				// xor other 4 bits
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 0) <<  3;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 1) <<  8;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 2) << 10;
				m_igs003_prot_hold ^= BIT(m_igs003_prot_x, 3) << 14;
			}
			break;

		default:
//          popmessage("igs003 %x <- %04x",m_igs003_reg,data);
			logerror("%06x: warning, writing to igs003_reg %02x = %02x\n", m_maincpu->pc(), m_igs003_reg, data);
	}
}


/***************************************************************************

    Driver Inits (Decryption, Protection Patches)

***************************************************************************/

// V0400O
void igs011_state::init_drgnwrld()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	drgnwrld_type1_decrypt();
	drgnwrld_gfx_decrypt();
/*
    // PROTECTION CHECKS
    rom[0x032ee/2]  =   0x606c;     // 0032EE: 676C        beq 335c     (ASIC11 CHECK PORT ERROR 3)
    rom[0x23d5e/2]  =   0x606c;     // 023D5E: 676C        beq 23dcc    (CHECK PORT ERROR 1)
    rom[0x23fd0/2]  =   0x606c;     // 023FD0: 676C        beq 2403e    (CHECK PORT ERROR 2)
    rom[0x24170/2]  =   0x606c;     // 024170: 676C        beq 241de    (CHECK PORT ERROR 3)
    rom[0x24348/2]  =   0x606c;     // 024348: 676C        beq 243b6    (ASIC11 CHECK PORT ERROR 4)
    rom[0x2454e/2]  =   0x606c;     // 02454E: 676C        beq 245bc    (ASIC11 CHECK PORT ERROR 3)
    rom[0x246cc/2]  =   0x606c;     // 0246CC: 676C        beq 2473a    (ASIC11 CHECK PORT ERROR 2)
    rom[0x24922/2]  =   0x606c;     // 024922: 676C        beq 24990    (ASIC11 CHECK PORT ERROR 1)

    rom[0x24b66/2]  =   0x606c;     // 024B66: 676C        beq 24bd4    (ASIC12 CHECK PORT ERROR 4)
    rom[0x24de2/2]  =   0x606c;     // 024DE2: 676C        beq 24e50    (ASIC12 CHECK PORT ERROR 3)
    rom[0x2502a/2]  =   0x606c;     // 02502A: 676C        beq 25098    (ASIC12 CHECK PORT ERROR 2)
    rom[0x25556/2]  =   0x6000;     // 025556: 6700 E584   beq 23adc    (ASIC12 CHECK PORT ERROR 1)

    rom[0x2a16c/2]  =   0x606c;     // 02A16C: 676C        beq 2a1da    (ASIC11 CHECK PORT ERROR 2)
*/
}

void igs011_state::init_drgnwrldv30()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	drgnwrld_type1_decrypt();
	drgnwrld_gfx_decrypt();
/*
    // PROTECTION CHECKS
    rom[0x032ee/2]  =   0x606c;     // 0032EE: 676C        beq 335c     (ASIC11 CHECK PORT ERROR 3)
    rom[0x23d5e/2]  =   0x606c;     // 023D5E: 676C        beq 23dcc    (CHECK PORT ERROR 1)
    rom[0x23fd0/2]  =   0x606c;     // 023FD0: 676C        beq 2403e    (CHECK PORT ERROR 2)
    rom[0x24170/2]  =   0x606c;     // 024170: 676C        beq 241de    (CHECK PORT ERROR 3)
    rom[0x24348/2]  =   0x606c;     // 024348: 676C        beq 243b6    (ASIC11 CHECK PORT ERROR 4)
    rom[0x2454e/2]  =   0x606c;     // 02454E: 676C        beq 245bc    (ASIC11 CHECK PORT ERROR 3)
    rom[0x246cc/2]  =   0x606c;     // 0246CC: 676C        beq 2473a    (ASIC11 CHECK PORT ERROR 2)
    rom[0x24922/2]  =   0x606c;     // 024922: 676C        beq 24990    (ASIC11 CHECK PORT ERROR 1)
    rom[0x24b66/2]  =   0x606c;     // 024B66: 676C        beq 24bd4    (ASIC12 CHECK PORT ERROR 4)
    rom[0x24de2/2]  =   0x606c;     // 024DE2: 676C        beq 24e50    (ASIC12 CHECK PORT ERROR 3)
    rom[0x2502a/2]  =   0x606c;     // 02502A: 676C        beq 25098    (ASIC12 CHECK PORT ERROR 2)
    rom[0x25556/2]  =   0x6000;     // 025556: 6700 E584   beq 23adc    (ASIC12 CHECK PORT ERROR 1)
    // different from drgnwrld:
    rom[0x2a162/2]  =   0x606c;     // 02A162: 676C        beq 2a1d0    (ASIC11 CHECK PORT ERROR 2)
*/
}

void igs011_state::init_drgnwrldv21()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	drgnwrld_type2_decrypt();
	drgnwrld_gfx_decrypt();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd4c0, 0xd4ff, read16smo_delegate(*this, FUNC(igs011_state::drgnwrldv21_igs011_prot2_r)));
/*
    // PROTECTION CHECKS
    // bp 32ee; bp 11ca8; bp 23d5e; bp 23fd0; bp 24170; bp 24348; bp 2454e; bp 246cc; bp 24922; bp 24b66; bp 24de2; bp 2502a; bp 25556; bp 269de; bp 2766a; bp 2a830
    rom[0x032ee/2]  =   0x606c;     // 0032EE: 676C        beq 335c     (ASIC11 CHECK PORT ERROR 3)
    rom[0x11ca8/2]  =   0x606c;     // 011CA8: 676C        beq 11d16    (CHECK PORT ERROR 1)
    rom[0x23d5e/2]  =   0x606c;     // 023D5E: 676C        beq 23dcc    (CHECK PORT ERROR 1)
    rom[0x23fd0/2]  =   0x606c;     // 023FD0: 676C        beq 2403e    (CHECK PORT ERROR 2)
    rom[0x24170/2]  =   0x606c;     // 024170: 676C        beq 241de    (CHECK PORT ERROR 3)
    rom[0x24348/2]  =   0x606c;     // 024348: 676C        beq 243b6    (ASIC11 CHECK PORT ERROR 4)
    rom[0x2454e/2]  =   0x606c;     // 02454E: 676C        beq 245bc    (ASIC11 CHECK PORT ERROR 3)
    rom[0x246cc/2]  =   0x606c;     // 0246CC: 676C        beq 2473a    (ASIC11 CHECK PORT ERROR 2)
    rom[0x24922/2]  =   0x606c;     // 024922: 676C        beq 24990    (ASIC11 CHECK PORT ERROR 1)
    rom[0x24b66/2]  =   0x606c;     // 024B66: 676C        beq 24bd4    (ASIC12 CHECK PORT ERROR 4)
    rom[0x24de2/2]  =   0x606c;     // 024DE2: 676C        beq 24e50    (ASIC12 CHECK PORT ERROR 3)
    rom[0x2502a/2]  =   0x606c;     // 02502A: 676C        beq 25098    (ASIC12 CHECK PORT ERROR 2)
    rom[0x25556/2]  =   0x6000;     // 025556: 6700 E584   beq 23adc    (ASIC12 CHECK PORT ERROR 1)
    rom[0x269de/2]  =   0x606c;     // 0269DE: 676C        beq 26a4c    (ASIC12 CHECK PORT ERROR 1)
    rom[0x2766a/2]  =   0x606c;     // 02766A: 676C        beq 276d8    (CHECK PORT ERROR 3)
    rom[0x2a830/2]  =   0x606c;     // 02A830: 676C        beq 2a89e    (ASIC11 CHECK PORT ERROR 2)
*/
}

void igs011_state::init_drgnwrldv21j()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	drgnwrld_type3_decrypt();
	drgnwrld_gfx_decrypt();
/*
    // PROTECTION CHECKS
    rom[0x033d2/2]  =   0x606c;     // 0033D2: 676C        beq 3440     (ASIC11 CHECK PORT ERROR 3)
    rom[0x11c74/2]  =   0x606c;     // 011C74: 676C        beq 11ce2    (CHECK PORT ERROR 1)
    rom[0x23d2a/2]  =   0x606c;     // 023D2A: 676C        beq 23d98
    rom[0x23f68/2]  =   0x606c;     // 023F68: 676C        beq 23fd6
    rom[0x240d4/2]  =   0x606c;     // 0240D4: 676C        beq 24142    (CHECK PORT ERROR 3)
    rom[0x242ac/2]  =   0x606c;     // 0242AC: 676C        beq 2431a
    rom[0x244b2/2]  =   0x606c;     // 0244B2: 676C        beq 24520
    rom[0x24630/2]  =   0x606c;     // 024630: 676C        beq 2469e
    rom[0x24886/2]  =   0x606c;     // 024886: 676C        beq 248f4
    rom[0x24aca/2]  =   0x606c;     // 024ACA: 676C        beq 24b38
    rom[0x24d46/2]  =   0x606c;     // 024D46: 676C        beq 24db4
    rom[0x24f8e/2]  =   0x606c;     // 024F8E: 676C        beq 24ffc
    rom[0x254ba/2]  =   0x6000;     // 0254BA: 6700 E620   beq 23adc    (ASIC12 CHECK PORT ERROR 1)
    rom[0x26a52/2]  =   0x606c;     // 026A52: 676C        beq 26ac0    (ASIC12 CHECK PORT ERROR 1)
    rom[0x276aa/2]  =   0x606c;     // 0276AA: 676C        beq 27718    (CHECK PORT ERROR 3)
    rom[0x2a870/2]  =   0x606c;     // 02A870: 676C        beq 2a8de    (ASIC11 CHECK PORT ERROR 2)
*/
}

void igs011_state::init_drgnwrldv20j()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	drgnwrld_type3_decrypt();
	drgnwrld_gfx_decrypt();
/*
    // PROTECTION CHECKS
    // bp 33d2; bp 11c74; bp 23d2a; bp 23f68; bp 240d4; bp 242ac; bp 244b2; bp 24630; bp 24886; bp 24aca; bp 24d46; bp 24f8e; bp 254ba; bp 26a52; bp 276a0; bp 2a86e
    rom[0x033d2/2]  =   0x606c;     // 0033D2: 676C        beq 3440     (ASIC11 CHECK PORT ERROR 3)
    rom[0x11c74/2]  =   0x606c;     // 011C74: 676C        beq 11ce2    (CHECK PORT ERROR 1)
    rom[0x23d2a/2]  =   0x606c;     // 023D2A: 676C        beq 23d98
    rom[0x23f68/2]  =   0x606c;     // 023F68: 676C        beq 23fd6
    rom[0x240d4/2]  =   0x606c;     // 0240D4: 676C        beq 24142    (CHECK PORT ERROR 3)
    rom[0x242ac/2]  =   0x606c;     // 0242AC: 676C        beq 2431a
    rom[0x244b2/2]  =   0x606c;     // 0244B2: 676C        beq 24520
    rom[0x24630/2]  =   0x606c;     // 024630: 676C        beq 2469e
    rom[0x24886/2]  =   0x606c;     // 024886: 676C        beq 248f4
    rom[0x24aca/2]  =   0x606c;     // 024ACA: 676C        beq 24b38
    rom[0x24d46/2]  =   0x606c;     // 024D46: 676C        beq 24db4
    rom[0x24f8e/2]  =   0x606c;     // 024F8E: 676C        beq 24ffc
    rom[0x254ba/2]  =   0x6000;     // 0254BA: 6700 E620   beq 23adc    (ASIC12 CHECK PORT ERROR 1)
    rom[0x26a52/2]  =   0x606c;     // 026A52: 676C        beq 26ac0    (ASIC12 CHECK PORT ERROR 1)
    // different from drgnwrldv21j:
    rom[0x276a0/2]  =   0x606c;     // 0276A0: 676C        beq 2770e    (CHECK PORT ERROR 3)
    rom[0x2a86e/2]  =   0x606c;     // 02A86E: 676C        beq 2a8dc    (ASIC11 CHECK PORT ERROR 2)
*/
}

void igs011_state::init_drgnwrldv40k()
{
	drgnwrldv40k_decrypt();
	drgnwrld_gfx_decrypt();

	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd4c0, 0xd4ff, read16smo_delegate(*this, FUNC(igs011_state::drgnwrldv40k_igs011_prot2_r)));
}

void igs011_state::init_drgnwrldv11h()
{
	drgnwrld_type1_decrypt();
	drgnwrld_gfx_decrypt();

	// PROTECTION CHECKS
	// the protection checks are already patched out like we do!
}

void igs011_state::init_drgnwrldv10c()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	drgnwrld_type1_decrypt();
	drgnwrld_gfx_decrypt();
/*
    // PROTECTION CHECKS
    // bp 33d2; bp 23d0e; bp 23f58; bp 240d0; bp 242a8; bp 244ae; bp 2462c; bp 24882; bp 24ac6; bp 24d42; bp 24f8a; bp 254b6; bp 2a23a
    rom[0x033d2/2]  =   0x606c;     // 0033D2: 676C        beq 3440     (ASIC11 CHECK PORT ERROR 3)
    rom[0x23d0e/2]  =   0x606c;     // 023D0E: 676C        beq 23d7c    (CHECK PORT ERROR 1)
    rom[0x23f58/2]  =   0x606c;     // 023F58: 676C        beq 23fc6    (CHECK PORT ERROR 2)
    rom[0x240d0/2]  =   0x606c;     // 0240D0: 676C        beq 2413e    (CHECK PORT ERROR 3)
    rom[0x242a8/2]  =   0x606c;     // 0242A8: 676C        beq 24316    (ASIC11 CHECK PORT ERROR 4)
    rom[0x244ae/2]  =   0x606c;     // 0244AE: 676C        beq 2451c    (ASIC11 CHECK PORT ERROR 3)
    rom[0x2462c/2]  =   0x606c;     // 02462C: 676C        beq 2469a    (ASIC11 CHECK PORT ERROR 2)
    rom[0x24882/2]  =   0x606c;     // 024882: 676C        beq 248f0    (ASIC11 CHECK PORT ERROR 1)
    rom[0x24ac6/2]  =   0x606c;     // 024AC6: 676C        beq 24b34    (ASIC12 CHECK PORT ERROR 4)
    rom[0x24d42/2]  =   0x606c;     // 024D42: 676C        beq 24db0    (ASIC12 CHECK PORT ERROR 3)
    rom[0x24f8a/2]  =   0x606c;     // 024F8A: 676C        beq 24ff8    (ASIC12 CHECK PORT ERROR 2)
    rom[0x254b6/2]  =   0x6000;     // 0254B6: 6700 E5FC   beq 23ab4    (ASIC12 CHECK PORT ERROR 1)
    rom[0x2a23a/2]  =   0x606c;     // 02A23A: 676C        beq 2a2a8    (ASIC11 CHECK PORT ERROR 2)
*/
}


void igs011_state::init_lhb()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	lhb_decrypt();

	// PROTECTION CHECKS
//  rom[0x2eef6/2]  =   0x4e75;     // 02EEF6: 4E56 FE00    link A6, #-$200  (fills palette with pink otherwise)
}

void igs011_state::init_lhbv33c()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	lhb_decrypt();

	// PROTECTION CHECKS
//  rom[0x2e988/2]  =   0x4e75;     // 02E988: 4E56 FE00    link A6, #-$200  (fills palette with pink otherwise)
}

void igs011_state::init_dbc()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	dbc_decrypt();

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x10600, 0x107ff, read16smo_delegate(*this, FUNC(igs011_state::dbc_igs011_prot2_r)));
/*
    // PROTECTION CHECKS
    rom[0x04c42/2]  =   0x602e;     // 004C42: 6604         bne 4c48  (rom test error otherwise)

    rom[0x08694/2]  =   0x6008;     // 008694: 6408         bcc 869e  (fills screen with characters otherwise)
    rom[0x0a05e/2]  =   0x4e71;     // 00A05E: 6408         bcc a068  (fills screen with characters otherwise)
    rom[0x0bec2/2]  =   0x6008;     // 00BEC2: 6408         bcc becc  (fills screen with characters otherwise)
    rom[0x0c0d4/2]  =   0x600a;     // 00C0D4: 640A         bcc c0e0  (wrong game state otherwise)
    rom[0x0c0f0/2]  =   0x4e71;     // 00C0F0: 6408         bcc c0fa  (wrong palette otherwise)
    rom[0x0e292/2]  =   0x6008;     // 00E292: 6408         bcc e29c  (fills screen with characters otherwise)
    rom[0x11b42/2]  =   0x6008;     // 011B42: 6408         bcc 11b4c (wrong game state otherwise)
    rom[0x11b5c/2]  =   0x4e71;     // 011B5C: 6408         bcc 11b66 (wrong palette otherwise)
    rom[0x170ae/2]  =   0x4e71;     // 0170AE: 6408         bcc 170b8 (fills screen with characters otherwise)
    rom[0x1842a/2]  =   0x6024;     // 01842A: 6724         beq 18450 (ASIC11 ERROR otherwise)
    rom[0x18538/2]  =   0x6008;     // 018538: 6408         bcc 18542 (wrong game state otherwise)
    rom[0x18552/2]  =   0x4e71;     // 018552: 6408         bcc 1855c (wrong palette otherwise)
    rom[0x18c0e/2]  =   0x6006;     // 018C0E: 6406         bcc 18c16 (fills screen with characters otherwise)
    rom[0x1923e/2]  =   0x4e71;     // 01923E: 6408         bcc 19248 (fills screen with characters otherwise)
*/
	// Fix for the palette fade on title screen
//  rom[0x19E90/2]  =   0x00ff;
}

void igs011_state::init_ryukobou()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	ryukobou_decrypt();

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x10600, 0x107ff, read16smo_delegate(*this, FUNC(igs011_state::ryukobou_igs011_prot2_r)));

	// PROTECTION CHECKS
//  rom[0x2df68/2]  =   0x4e75;     // 02DF68: 4E56 FE00    link A6, #-$200  (fills palette with pink otherwise)
}


void igs011_state::init_xymg()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	lhb_decrypt();
/*
    // PROTECTION CHECKS
    rom[0x00502/2]  =   0x6006;     // 000502: 6050         bra 554
    rom[0x0fc1c/2]  =   0x6036;     // 00FC1C: 6736         beq fc54  (fills palette with red otherwise)
    rom[0x1232a/2]  =   0x6036;     // 01232A: 6736         beq 12362 (fills palette with red otherwise)
    rom[0x18244/2]  =   0x6036;     // 018244: 6736         beq 1827c (fills palette with red otherwise)
    rom[0x1e15e/2]  =   0x6036;     // 01E15E: 6736         beq 1e196 (fills palette with red otherwise)
    rom[0x22286/2]  =   0x6000;     // 022286: 6700 02D2    beq 2255a (fills palette with green otherwise)
    rom[0x298ce/2]  =   0x6036;     // 0298CE: 6736         beq 29906 (fills palette with red otherwise)
    rom[0x2e07c/2]  =   0x6036;     // 02E07C: 6736         beq 2e0b4 (fills palette with red otherwise)
    rom[0x38f1c/2]  =   0x6000;     // 038F1C: 6700 071C    beq 3963a (ASIC11 ERROR 1)
    rom[0x390e8/2]  =   0x6000;     // 0390E8: 6700 0550    beq 3963a (ASIC11 ERROR 2)
    rom[0x3933a/2]  =   0x6000;     // 03933A: 6700 02FE    beq 3963a (ASIC11 ERROR 3)
    rom[0x3955c/2]  =   0x6000;     // 03955C: 6700 00DC    beq 3963a (ASIC11 ERROR 4)
    rom[0x397f4/2]  =   0x6000;     // 0397F4: 6700 02C0    beq 39ab6 (fills palette with green otherwise)
    rom[0x39976/2]  =   0x6000;     // 039976: 6700 013E    beq 39ab6 (fills palette with green otherwise)
    rom[0x39a7e/2]  =   0x6036;     // 039A7E: 6736         beq 39ab6 (fills palette with green otherwise)
    rom[0x4342c/2]  =   0x4e75;     // 04342C: 4E56 0000    link A6, #$0
    rom[0x49966/2]  =   0x6036;     // 049966: 6736         beq 4999e (fills palette with blue otherwise)
    rom[0x58140/2]  =   0x6036;     // 058140: 6736         beq 58178 (fills palette with red otherwise)
    rom[0x5e05a/2]  =   0x6036;     // 05E05A: 6736         beq 5e092 (fills palette with red otherwise)
    rom[0x5ebf0/2]  =   0x6000;     // 05EBF0: 6700 0208    beq 5edfa (fills palette with red otherwise)
    rom[0x5edc2/2]  =   0x6036;     // 05EDC2: 6736         beq 5edfa (fills palette with green otherwise)
    rom[0x5f71c/2]  =   0x6000;     // 05F71C: 6700 01F2    beq 5f910 (fills palette with green otherwise)
    rom[0x5f8d8/2]  =   0x6036;     // 05F8D8: 6736         beq 5f910 (fills palette with red otherwise)
    rom[0x64836/2]  =   0x6036;     // 064836: 6736         beq 6486e (fills palette with red otherwise)
*/
}

void igs011_state::init_wlcc()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	wlcc_decrypt();
/*
    // PROTECTION CHECKS
    rom[0x16b96/2]  =   0x6000;     // 016B96: 6700 02FE    beq 16e96  (fills palette with red otherwise)
    rom[0x16e5e/2]  =   0x6036;     // 016E5E: 6736         beq 16e96  (fills palette with green otherwise)
    rom[0x17852/2]  =   0x6000;     // 017852: 6700 01F2    beq 17a46  (fills palette with green otherwise)
    rom[0x17a0e/2]  =   0x6036;     // 017A0E: 6736         beq 17a46  (fills palette with red otherwise)
    rom[0x23636/2]  =   0x6036;     // 023636: 6736         beq 2366e  (fills palette with red otherwise)
    rom[0x2b1e6/2]  =   0x6000;     // 02B1E6: 6700 0218    beq 2b400  (fills palette with green otherwise)
    rom[0x2f9f2/2]  =   0x6000;     // 02F9F2: 6700 04CA    beq 2febe  (fills palette with green otherwise)
    rom[0x2fb2e/2]  =   0x6000;     // 02FB2E: 6700 038E    beq 2febe  (fills palette with red otherwise)
    rom[0x2fcf2/2]  =   0x6000;     // 02FCF2: 6700 01CA    beq 2febe  (fills palette with red otherwise)
    rom[0x2fe86/2]  =   0x6036;     // 02FE86: 6736         beq 2febe  (fills palette with red otherwise)
    rom[0x3016e/2]  =   0x6000;     // 03016E: 6700 03F6    beq 30566  (fills palette with green otherwise)
    rom[0x303c8/2]  =   0x6000;     // 0303C8: 6700 019C    beq 30566  (fills palette with green otherwise)
    rom[0x3052e/2]  =   0x6036;     // 03052E: 6736         beq 30566  (fills palette with green otherwise)
*/
}


void igs011_state::init_lhb2()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	lhb2_decrypt();
	lhb2_gfx_decrypt();
/*
    // PROTECTION CHECKS
    rom[0x034f4/2]  =   0x4e71;     // 0034F4: 660E    bne 3504   (rom test, fills palette with white otherwise)
    rom[0x03502/2]  =   0x6032;     // 003502: 6732    beq 3536   (rom test, fills palette with white otherwise)
    rom[0x1afea/2]  =   0x6034;     // 01AFEA: 6734    beq 1b020  (fills palette with black otherwise)
    rom[0x24b8a/2]  =   0x6036;     // 024B8A: 6736    beq 24bc2  (fills palette with green otherwise)
    rom[0x29ef8/2]  =   0x6036;     // 029EF8: 6736    beq 29f30  (fills palette with red otherwise)
    rom[0x2e69c/2]  =   0x6036;     // 02E69C: 6736    beq 2e6d4  (fills palette with green otherwise)
    rom[0x2fe96/2]  =   0x6036;     // 02FE96: 6736    beq 2fece  (fills palette with red otherwise)
    rom[0x325da/2]  =   0x6036;     // 0325DA: 6736    beq 32612  (fills palette with green otherwise)
    rom[0x3d80a/2]  =   0x6034;     // 03D80A: 6734    beq 3d840  (fills palette with black otherwise)
    rom[0x3ed80/2]  =   0x6036;     // 03ED80: 6736    beq 3edb8  (fills palette with red otherwise)
    rom[0x41d72/2]  =   0x6034;     // 041D72: 6734    beq 41da8  (fills palette with black otherwise)
    rom[0x44834/2]  =   0x6034;     // 044834: 6734    beq 4486a  (fills palette with black otherwise)
*/
}

void igs011_state::init_tygn()
{
	tygn_decrypt();

	lhb2_gfx_decrypt();

	// PROTECTION CHECKS (similar to nkishusp)

	u16 *rom = (u16 *) m_maincpu_region->base();

	rom[0x036d8/2]  =   0x6042;     // 0036d8: 660E      bne     $36e8 (ROM test)

	//TODO: are the following patches needed? in that case the ones after the blank line should be adapted to the tygn offsets
	//rom[0x1c6a0/2]  =   0x6034;     // 01c6a0: 6734      beq     $1c6d6
	//rom[0x2412a/2]  =   0x6036;     // 02412a: 6736      beq     $24162
	//rom[0x26de6/2]  =   0x6038;     // 026de6: 6e38      bgt     $26e20 (system error)
	//rom[0x2da36/2]  =   0x6036;     // 02da36: 6736      beq     $2da6e

	//rom[0x2ff20/2]  =   0x6036;     // 02ff20: 6736      beq     $2ff58
	//rom[0x3151c/2]  =   0x6036;     // 03151c: 6736      beq     $31554
	//rom[0x33dfc/2]  =   0x6036;     // 033dfc: 6736      beq     $33e34
	//rom[0x3460e/2]  =   0x6038;     // 03460e: 6e38      bgt     $34648 (system error)
	//rom[0x3f09e/2]  =   0x6034;     // 03f09e: 6734      beq     $3f0d4
	//rom[0x406a8/2]  =   0x6036;     // 0406a8: 6736      beq     $406e0
	//rom[0x4376a/2]  =   0x6034;     // 04376a: 6734      beq     $437a0
	//rom[0x462d6/2]  =   0x6034;     // 0462d6: 6734      beq     $4630c
	//rom[0x471ec/2]  =   0x6036;     // 0471ec: 6e36      bgt     $47224 (system error)
	//rom[0x49c46/2]  =   0x6000;     // 049c46: 6700 0444 beq     $4a08c
	//rom[0x4a2b6/2]  =   0x6036;     // 04a2b6: 6736      beq     $4a2ee
	//rom[0x4c67a/2]  =   0x6038;     // 04c67a: 6e38      bgt     $4c6b4 (system error)
}

void vbowl_state::init_vbowl()
{
	u16 *rom = (u16 *) m_maincpu_region->base();

	vbowl_decrypt();
	vbowl_gfx_unpack();

	// Patch the bad dump so that it doesn't reboot at the end of a game (the patched value is from vbowlj)
	rom[0x080e0/2] = 0xe549;    // 0080E0: 0449 dc.w $0449; ILLEGAL
/*
    // PROTECTION CHECKS
    rom[0x03764/2] = 0x4e75;    // 003764: 4E56 0000 link    A6, #$0
    rom[0x173ee/2] = 0x600c;    // 0173EE: 670C      beq     $173fc
    rom[0x1e6e6/2] = 0x600c;    // 01E6E6: 670C      beq     $1e6f4
    rom[0x1f7ce/2] = 0x600c;    // 01F7CE: 670C      beq     $1f7dc
*/
	machine().save().register_postload(save_prepost_delegate(FUNC(vbowl_state::prot_mem_range_set), this));
}


void vbowl_state::init_vbowlj()
{
//  u16 *rom = (u16 *) m_maincpu_region->base();

	vbowl_decrypt();
	vbowl_gfx_unpack();

/*
    // PROTECTION CHECKS
    rom[0x37b4/2] = 0x4e75;     // 0037B4: 4E56 0000 link    A6, #$0
    rom[0x17720/2] = 0x600c;    // 017720: 670C      beq     1772e
    rom[0x1e6e6/2] = 0x600c;    // 01E6E6: 670C      beq     $1e6f4
    rom[0x1f7c8/2] = 0x600c;    // 01F7C8: 670C      beq     1f7d6
*/
	machine().save().register_postload(save_prepost_delegate(FUNC(vbowl_state::prot_mem_range_set), this));
}


void vbowl_state::init_vbowlhk()
{
	vbowlhk_decrypt();
	vbowl_gfx_unpack();

	machine().save().register_postload(save_prepost_delegate(FUNC(vbowl_state::prot_mem_range_set), this));
}


void igs011_state::init_nkishusp()
{
	u16 *rom = (u16 *) m_maincpu_region->base();

	nkishusp_decrypt();
	lhb2_gfx_decrypt();

	// PROTECTION CHECKS (similar to lhb2)

	rom[0x03624/2]  =   0x6042;     // 003624: 660E      bne     $3634 (ROM test)

	rom[0x1a9d2/2]  =   0x6034;     // 01A9D2: 6734      beq     $1aa08
	rom[0x26306/2]  =   0x6036;     // 026306: 6736      beq     $2633e
	rom[0x29190/2]  =   0x6038;     // 029190: 6E38      bgt     $291ca (system error)
	rom[0x2b82a/2]  =   0x6036;     // 02B82A: 6736      beq     $2b862
	rom[0x2ff20/2]  =   0x6036;     // 02FF20: 6736      beq     $2ff58
	rom[0x3151c/2]  =   0x6036;     // 03151C: 6736      beq     $31554
	rom[0x33dfc/2]  =   0x6036;     // 033DFC: 6736      beq     $33e34
	rom[0x3460e/2]  =   0x6038;     // 03460E: 6E38      bgt     $34648 (system error)
	rom[0x3f09e/2]  =   0x6034;     // 03F09E: 6734      beq     $3f0d4
	rom[0x406a8/2]  =   0x6036;     // 0406A8: 6736      beq     $406e0
	rom[0x4376a/2]  =   0x6034;     // 04376A: 6734      beq     $437a0
	rom[0x462d6/2]  =   0x6034;     // 0462D6: 6734      beq     $4630c
	rom[0x471ec/2]  =   0x6036;     // 0471EC: 6E36      bgt     $47224 (system error)
	rom[0x49c46/2]  =   0x6000;     // 049C46: 6700 0444 beq     $4a08c
	rom[0x4a2b6/2]  =   0x6036;     // 04A2B6: 6736      beq     $4a2ee
	rom[0x4c67a/2]  =   0x6038;     // 04C67A: 6E38      bgt     $4c6b4 (system error)
}


/***************************************************************************

    Memory Maps

***************************************************************************/

void igs011_state::drgnwrld_mem(address_map &map)
{
//  drgnwrld: IGS011 protection dynamically mapped at 1dd7x
//  map(0x01dd70, 0x01dd77).w(FUNC(igs011_state::igs011_prot1_w)).umask16(0xff00);
//  map(0x01dd78, 0x01dd79).r(FUNC(igs011_state::igs011_prot1_r));

	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram().share("nvram");
	map(0x200000, 0x200fff).ram().share(m_priority_ram);
	map(0x400000, 0x400fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x401000, 0x401fff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).umask16(0x00ff).share("palette_ext");
	map(0x500000, 0x500001).portr("COIN");
	map(0x600001, 0x600001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x700000, 0x700003).w("ymsnd", FUNC(ym3812_device::write)).umask16(0x00ff);

	map(0x800000, 0x800001).w(FUNC(igs011_state::igs003_w));
	map(0x800002, 0x800003).rw(FUNC(igs011_state::drgnwrld_igs003_r), FUNC(igs011_state::drgnwrld_igs003_w));

	map(0xa20000, 0xa20001).w(FUNC(igs011_state::igs011_priority_w));
	map(0xa40000, 0xa40001).w(FUNC(igs011_state::dips_w));

	map(0xa50000, 0xa50001).w(FUNC(igs011_state::igs011_prot_addr_w));
//  map(0xa50000, 0xa50005).r(FUNC(igs011_state::igs011_prot_fake_r));

	map(0xa58000, 0xa58001).w(FUNC(igs011_state::igs011_blit_x_w));
	map(0xa58800, 0xa58801).w(FUNC(igs011_state::igs011_blit_y_w));
	map(0xa59000, 0xa59001).w(FUNC(igs011_state::igs011_blit_w_w));
	map(0xa59800, 0xa59801).w(FUNC(igs011_state::igs011_blit_h_w));
	map(0xa5a000, 0xa5a001).w(FUNC(igs011_state::igs011_blit_gfx_lo_w));
	map(0xa5a800, 0xa5a801).w(FUNC(igs011_state::igs011_blit_gfx_hi_w));
	map(0xa5b000, 0xa5b001).w(FUNC(igs011_state::igs011_blit_flags_w));
	map(0xa5b800, 0xa5b801).w(FUNC(igs011_state::igs011_blit_pen_w));
	map(0xa5c000, 0xa5c001).w(FUNC(igs011_state::igs011_blit_depth_w));
	map(0xa88000, 0xa88001).r(FUNC(igs011_state::dips_r<3>));
}

void igs011_state::drgnwrld_igs012_mem(address_map &map)
{
	drgnwrld_mem(map);

	// IGS012
	map(0x001600, 0x00160f).w(FUNC(igs011_state::igs012_prot_swap_w)).mirror(0x01c000); // swap (a5 / 55)
	map(0x001610, 0x00161f).r(FUNC(igs011_state::igs012_prot_r)).mirror(0x01c000); // read (mode 0)
	map(0x001620, 0x00162f).w(FUNC(igs011_state::igs012_prot_dec_inc_w)).mirror(0x01c000); // dec  (aa), inc  (fa)
	map(0x001630, 0x00163f).w(FUNC(igs011_state::igs012_prot_inc_w)).mirror(0x01c000); // inc  (ff)
	map(0x001640, 0x00164f).w(FUNC(igs011_state::igs012_prot_copy_w)).mirror(0x01c000); // copy (22)
	map(0x001650, 0x00165f).w(FUNC(igs011_state::igs012_prot_dec_copy_w)).mirror(0x01c000); // dec  (5a), copy (33)
	map(0x001660, 0x00166f).r(FUNC(igs011_state::igs012_prot_r)).mirror(0x01c000); // read (mode 1)
	map(0x001670, 0x00167f).w(FUNC(igs011_state::igs012_prot_mode_w)).mirror(0x01c000); // mode (cc / dd)

	map(0x00d400, 0x00d43f).w(FUNC(igs011_state::igs011_prot2_dec_w));   // dec   (33)
	map(0x00d440, 0x00d47f).w(FUNC(igs011_state::drgnwrld_igs011_prot2_swap_w));   // swap  (33)
	map(0x00d480, 0x00d4bf).w(FUNC(igs011_state::igs011_prot2_reset_w));   // reset (33)
	map(0x00d4c0, 0x00d4ff).r(FUNC(igs011_state::drgnwrldv20j_igs011_prot2_r));   // read

	map(0x902000, 0x902fff).w(FUNC(igs011_state::igs012_prot_reset_w));   // reset?
//  map(0x902000, 0x902005).w(FUNC(igs011_state::igs012_prot_fake_r));
}


// Only values 0 and 7 are written (1 bit per irq source?)
void igs011_state::lhb_irq_enable_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_lhb_irq_enable);
}

void igs011_state::lhb_okibank_w(u8 data)
{
	m_oki->set_rom_bank(BIT(data, 1));

	if (data & (~0x2))
		logerror("%s: warning, unknown bits written in oki bank = %02x\n", machine().describe_context(), data);

//  popmessage("oki %04x",data);
}

void igs011_state::lhb_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

//  lhb: IGS011 protection dynamically mapped at 834x
//  map(0x008340, 0x008347).w(FUNC(igs011_state::igs011_prot1_w)).umask16(0xff00);
//  map(0x008348, 0x008349).r(FUNC(igs011_state::igs011_prot1_r));

	map(0x010000, 0x010000).w(FUNC(igs011_state::lhb_okibank_w));

	map(0x010200, 0x0103ff).w(FUNC(igs011_state::igs011_prot2_inc_w));
	map(0x010400, 0x0105ff).w(FUNC(igs011_state::lhb_igs011_prot2_swap_w));
	map(0x010600, 0x0107ff).r(FUNC(igs011_state::lhb_igs011_prot2_r));
	// no reset

	map(0x100000, 0x103fff).ram().share("nvram");
	map(0x200000, 0x200fff).ram().share(m_priority_ram);
	map(0x300000, 0x3fffff).rw(FUNC(igs011_state::igs011_layers_r), FUNC(igs011_state::igs011_layers_w));
	map(0x400000, 0x400fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x401000, 0x401fff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).umask16(0x00ff).share("palette_ext");
	map(0x600001, 0x600001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x700000, 0x700001).portr("COIN");
	map(0x700002, 0x700005).r(FUNC(igs011_state::lhb_inputs_r));
	map(0x700002, 0x700003).w(FUNC(igs011_state::lhb_inputs_w));
	map(0x820000, 0x820001).w(FUNC(igs011_state::igs011_priority_w));
	map(0x838000, 0x838001).w(FUNC(igs011_state::lhb_irq_enable_w));
	map(0x840000, 0x840001).w(FUNC(igs011_state::dips_w));

	map(0x850000, 0x850001).w(FUNC(igs011_state::igs011_prot_addr_w));
//  map(0x850000, 0x850005).w(FUNC(igs011_state::igs011_prot_fake_r));

	map(0x858000, 0x858001).w(FUNC(igs011_state::igs011_blit_x_w));
	map(0x858800, 0x858801).w(FUNC(igs011_state::igs011_blit_y_w));
	map(0x859000, 0x859001).w(FUNC(igs011_state::igs011_blit_w_w));
	map(0x859800, 0x859801).w(FUNC(igs011_state::igs011_blit_h_w));
	map(0x85a000, 0x85a001).w(FUNC(igs011_state::igs011_blit_gfx_lo_w));
	map(0x85a800, 0x85a801).w(FUNC(igs011_state::igs011_blit_gfx_hi_w));
	map(0x85b000, 0x85b001).w(FUNC(igs011_state::igs011_blit_flags_w));
	map(0x85b800, 0x85b801).w(FUNC(igs011_state::igs011_blit_pen_w));
	map(0x85c000, 0x85c001).w(FUNC(igs011_state::igs011_blit_depth_w));
	map(0x888000, 0x888001).r(FUNC(igs011_state::dips_r<5>));
}

void igs011_state::xymg_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

//  xymg: IGS011 protection dynamically mapped at 834x
//  map(0x008340, 0x008347).w(FUNC(igs011_state::igs011_prot1_w)).umask16(0xff00);
//  map(0x008348, 0x008349).r(FUNC(igs011_state::igs011_prot1_r));

	map(0x010000, 0x010000).w(FUNC(igs011_state::lhb_okibank_w));

	map(0x010200, 0x0103ff).w(FUNC(igs011_state::igs011_prot2_inc_w));   // inc  (33)
	map(0x010400, 0x0105ff).w(FUNC(igs011_state::lhb_igs011_prot2_swap_w));   // swap (33)
	map(0x010600, 0x0107ff).r(FUNC(igs011_state::lhb_igs011_prot2_r));   // read
	// no reset

	map(0x100000, 0x103fff).ram();
	map(0x1f0000, 0x1f3fff).ram().share("nvram"); // extra ram
	map(0x200000, 0x200fff).ram().share(m_priority_ram);
	map(0x300000, 0x3fffff).rw(FUNC(igs011_state::igs011_layers_r), FUNC(igs011_state::igs011_layers_w));
	map(0x400000, 0x400fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x401000, 0x401fff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).umask16(0x00ff).share("palette_ext");
	map(0x600001, 0x600001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x700000, 0x700001).w(FUNC(igs011_state::igs003_w));
	map(0x700002, 0x700003).rw(FUNC(igs011_state::xymg_igs003_r), FUNC(igs011_state::xymg_igs003_w));
	map(0x820000, 0x820001).w(FUNC(igs011_state::igs011_priority_w));
	map(0x840000, 0x840001).w(FUNC(igs011_state::dips_w));

	map(0x850000, 0x850001).w(FUNC(igs011_state::igs011_prot_addr_w));
//  map(0x850000, 0x850005).w(FUNC(igs011_state::igs011_prot_fake_r));

	map(0x858000, 0x858001).w(FUNC(igs011_state::igs011_blit_x_w));
	map(0x858800, 0x858801).w(FUNC(igs011_state::igs011_blit_y_w));
	map(0x859000, 0x859001).w(FUNC(igs011_state::igs011_blit_w_w));
	map(0x859800, 0x859801).w(FUNC(igs011_state::igs011_blit_h_w));
	map(0x85a000, 0x85a001).w(FUNC(igs011_state::igs011_blit_gfx_lo_w));
	map(0x85a800, 0x85a801).w(FUNC(igs011_state::igs011_blit_gfx_hi_w));
	map(0x85b000, 0x85b001).w(FUNC(igs011_state::igs011_blit_flags_w));
	map(0x85b800, 0x85b801).w(FUNC(igs011_state::igs011_blit_pen_w));
	map(0x85c000, 0x85c001).w(FUNC(igs011_state::igs011_blit_depth_w));
	map(0x888000, 0x888001).r(FUNC(igs011_state::dips_r<3>));
}

void igs011_state::wlcc_mem(address_map &map)
{
//  wlcc: IGS011 protection dynamically mapped at 834x
//  map(0x008340, 0x008347).w(FUNC(igs011_state::igs011_prot1_w)).umask16(0xff00);
//  map(0x008348, 0x008349).r(FUNC(igs011_state::igs011_prot1_r));

	map(0x518000, 0x5181ff).w(FUNC(igs011_state::igs011_prot2_inc_w));   // inc   (33)
	map(0x518200, 0x5183ff).w(FUNC(igs011_state::wlcc_igs011_prot2_swap_w));   // swap  (33)
	map(0x518800, 0x5189ff).r(FUNC(igs011_state::igs011_prot2_reset_r));   // reset
	map(0x519000, 0x5195ff).r(FUNC(igs011_state::lhb_igs011_prot2_r));   // read

	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram().share("nvram");
	map(0x200000, 0x200fff).ram().share(m_priority_ram);
	map(0x300000, 0x3fffff).rw(FUNC(igs011_state::igs011_layers_r), FUNC(igs011_state::igs011_layers_w));
	map(0x400000, 0x400fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x401000, 0x401fff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).umask16(0x00ff).share("palette_ext");
	map(0x520000, 0x520001).portr("COIN");
	map(0x600001, 0x600001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x800000, 0x800001).w(FUNC(igs011_state::igs003_w));
	map(0x800002, 0x800003).rw(FUNC(igs011_state::wlcc_igs003_r), FUNC(igs011_state::wlcc_igs003_w));
	map(0xa20000, 0xa20001).w(FUNC(igs011_state::igs011_priority_w));
	map(0xa40000, 0xa40001).w(FUNC(igs011_state::dips_w));

	map(0xa50000, 0xa50001).w(FUNC(igs011_state::igs011_prot_addr_w));
//  map(0xa50000, 0xa50005).r(FUNC(igs011_state::igs011_prot_fake_r));

	map(0xa58000, 0xa58001).w(FUNC(igs011_state::igs011_blit_x_w));
	map(0xa58800, 0xa58801).w(FUNC(igs011_state::igs011_blit_y_w));
	map(0xa59000, 0xa59001).w(FUNC(igs011_state::igs011_blit_w_w));
	map(0xa59800, 0xa59801).w(FUNC(igs011_state::igs011_blit_h_w));
	map(0xa5a000, 0xa5a001).w(FUNC(igs011_state::igs011_blit_gfx_lo_w));
	map(0xa5a800, 0xa5a801).w(FUNC(igs011_state::igs011_blit_gfx_hi_w));
	map(0xa5b000, 0xa5b001).w(FUNC(igs011_state::igs011_blit_flags_w));
	map(0xa5b800, 0xa5b801).w(FUNC(igs011_state::igs011_blit_pen_w));
	map(0xa5c000, 0xa5c001).w(FUNC(igs011_state::igs011_blit_depth_w));
	map(0xa88000, 0xa88001).r(FUNC(igs011_state::dips_r<4>));
}


void igs011_state::lhb2_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

//  lhb2: IGS011 protection dynamically mapped at 1ff8x
//  map(0x01ff80, 0x01ff87).w(FUNC(igs011_state::igs011_prot1_w)).umask16(0xff00);
//  map(0x01ff88, 0x01ff89).r(FUNC(igs011_state::igs011_prot1_r));

	map(0x020000, 0x0201ff).w(FUNC(igs011_state::igs011_prot2_inc_w));   // inc   (55)
	map(0x020200, 0x0203ff).w(FUNC(igs011_state::lhb_igs011_prot2_swap_w));   // swap  (33)
	map(0x020400, 0x0205ff).r(FUNC(igs011_state::lhb2_igs011_prot2_r));   // read
	map(0x020600, 0x0207ff).w(FUNC(igs011_state::igs011_prot2_reset_w));   // reset (55)

	map(0x100000, 0x103fff).ram().share("nvram");
	map(0x200001, 0x200001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x204000, 0x204003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x208000, 0x208001).w(FUNC(igs011_state::igs003_w));
	map(0x208002, 0x208003).rw(FUNC(igs011_state::lhb2_igs003_r), FUNC(igs011_state::lhb2_igs003_w));
	map(0x20c000, 0x20cfff).ram().share(m_priority_ram);
	map(0x210000, 0x210fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x211000, 0x211fff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).umask16(0x00ff).share("palette_ext");
	map(0x214000, 0x214001).portr("COIN");
	map(0x300000, 0x3fffff).rw(FUNC(igs011_state::igs011_layers_r), FUNC(igs011_state::igs011_layers_w));
	map(0xa20000, 0xa20001).w(FUNC(igs011_state::igs011_priority_w));
	map(0xa40000, 0xa40001).w(FUNC(igs011_state::dips_w));

	map(0xa50000, 0xa50001).w(FUNC(igs011_state::igs011_prot_addr_w));
//  map(0xa50000, 0xa50005).r(FUNC(igs011_state::igs011_prot_fake_r));

	map(0xa58000, 0xa58001).w(FUNC(igs011_state::igs011_blit_x_w));
	map(0xa58800, 0xa58801).w(FUNC(igs011_state::igs011_blit_y_w));
	map(0xa59000, 0xa59001).w(FUNC(igs011_state::igs011_blit_w_w));
	map(0xa59800, 0xa59801).w(FUNC(igs011_state::igs011_blit_h_w));
	map(0xa5a000, 0xa5a001).w(FUNC(igs011_state::igs011_blit_gfx_lo_w));
	map(0xa5a800, 0xa5a801).w(FUNC(igs011_state::igs011_blit_gfx_hi_w));
	map(0xa5b000, 0xa5b001).w(FUNC(igs011_state::igs011_blit_flags_w));
	map(0xa5b800, 0xa5b801).w(FUNC(igs011_state::igs011_blit_pen_w));
	map(0xa5c000, 0xa5c001).w(FUNC(igs011_state::igs011_blit_depth_w));
	map(0xa88000, 0xa88001).r(FUNC(igs011_state::dips_r<3>));
}


void igs011_state::nkishusp_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

//  nkishusp: IGS011 protection dynamically mapped at 1ff8x
//  map(0x01ff80, 0x01ff87).w(FUNC(igs011_state::igs011_prot1_w)).umask16(0xff00);
//  map(0x01ff88, 0x01ff89).r(FUNC(igs011_state::igs011_prot1_r));

	// to be done:
	map(0x023000, 0x0231ff).w(FUNC(igs011_state::igs011_prot2_inc_w));   // inc   (55)
	map(0x023200, 0x0233ff).w(FUNC(igs011_state::lhb_igs011_prot2_swap_w));   // swap  (33)
	map(0x023400, 0x0235ff).r(FUNC(igs011_state::lhb2_igs011_prot2_r));   // read
	map(0x023600, 0x0237ff).w(FUNC(igs011_state::igs011_prot2_reset_w));   // reset (55)

	map(0x100000, 0x103fff).ram().share("nvram");
	map(0x200001, 0x200001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x204000, 0x204003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x208000, 0x208001).w(FUNC(igs011_state::igs003_w));
	map(0x208002, 0x208003).rw(FUNC(igs011_state::lhb2_igs003_r), FUNC(igs011_state::lhb2_igs003_w));
	map(0x20c000, 0x20cfff).ram().share(m_priority_ram);
	map(0x210000, 0x210fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x211000, 0x211fff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).umask16(0x00ff).share("palette_ext");
	map(0x214000, 0x214001).portr("COIN");
	map(0x300000, 0x3fffff).rw(FUNC(igs011_state::igs011_layers_r), FUNC(igs011_state::igs011_layers_w));
	map(0xa20000, 0xa20001).w(FUNC(igs011_state::igs011_priority_w));
	map(0xa38000, 0xa38001).w(FUNC(igs011_state::lhb_irq_enable_w));
	map(0xa40000, 0xa40001).w(FUNC(igs011_state::dips_w));

	map(0xa50000, 0xa50001).w(FUNC(igs011_state::igs011_prot_addr_w));
//  map(0xa50000, 0xa50005).r(FUNC(igs011_state::igs011_prot_fake_r));

	map(0xa58000, 0xa58001).w(FUNC(igs011_state::igs011_blit_x_w));
	map(0xa58800, 0xa58801).w(FUNC(igs011_state::igs011_blit_y_w));
	map(0xa59000, 0xa59001).w(FUNC(igs011_state::igs011_blit_w_w));
	map(0xa59800, 0xa59801).w(FUNC(igs011_state::igs011_blit_h_w));
	map(0xa5a000, 0xa5a001).w(FUNC(igs011_state::igs011_blit_gfx_lo_w));
	map(0xa5a800, 0xa5a801).w(FUNC(igs011_state::igs011_blit_gfx_hi_w));
	map(0xa5b000, 0xa5b001).w(FUNC(igs011_state::igs011_blit_flags_w));
	map(0xa5b800, 0xa5b801).w(FUNC(igs011_state::igs011_blit_pen_w));
	map(0xa5c000, 0xa5c001).w(FUNC(igs011_state::igs011_blit_depth_w));
	map(0xa88000, 0xa88001).r(FUNC(igs011_state::dips_r<3>));
}

void igs011_state::tygn_mem(address_map &map)
{
	nkishusp_mem(map);

	map(0x208000, 0x208001).nopr(); // TODO
	map(0x208002, 0x208003).rw(FUNC(igs011_state::drgnwrld_igs003_r), FUNC(igs011_state::lhb2_igs003_w));
}


u16 vbowl_state::vbowl_unk_r()
{
	return 0xffff;
}

void vbowl_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_vbowl_trackball[0] = m_vbowl_trackball[1];
		m_vbowl_trackball[1] = (m_io_an[1]->read() << 8) | m_io_an[0]->read();
	}
}

void vbowl_state::vbowl_pen_hi_w(u8 data)
{
	m_blitter_pen_hi = data & 0x07;

	if (data & ~0x7)
		logerror("%06x: warning, unknown bits written to pen_hi = %02x\n", m_maincpu->pc(), m_priority);
}

void vbowl_state::vbowl_link_0_w(u16 data){ }
void vbowl_state::vbowl_link_1_w(u16 data){ }
void vbowl_state::vbowl_link_2_w(u16 data){ }
void vbowl_state::vbowl_link_3_w(u16 data){ }

void vbowl_state::vbowl_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

//  vbowl: IGS011 protection dynamically mapped at 834x
//  map(0x008340, 0x008347).w(FUNC(vbowl_state::igs011_prot1_w)).umask16(0xff00);
//  map(0x008348, 0x008349).r(FUNC(vbowl_state::igs011_prot1_r));

	// IGS012
	map(0x001600, 0x00160f).w(FUNC(vbowl_state::igs012_prot_swap_w)).mirror(0x01c000); // swap (a5 / 55)
	map(0x001610, 0x00161f).r(FUNC(vbowl_state::igs012_prot_r)).mirror(0x01c000); // read (mode 0)
	map(0x001620, 0x00162f).w(FUNC(vbowl_state::igs012_prot_dec_inc_w)).mirror(0x01c000); // dec  (aa), inc  (fa)
	map(0x001630, 0x00163f).w(FUNC(vbowl_state::igs012_prot_inc_w)).mirror(0x01c000); // inc  (ff)
	map(0x001640, 0x00164f).w(FUNC(vbowl_state::igs012_prot_copy_w)).mirror(0x01c000); // copy (22)
	map(0x001650, 0x00165f).w(FUNC(vbowl_state::igs012_prot_dec_copy_w)).mirror(0x01c000); // dec  (5a), copy (33)
	map(0x001660, 0x00166f).r(FUNC(vbowl_state::igs012_prot_r)).mirror(0x01c000); // read (mode 1)
	map(0x001670, 0x00167f).w(FUNC(vbowl_state::igs012_prot_mode_w)).mirror(0x01c000); // mode (cc / dd)

	map(0x00d400, 0x00d43f).w(FUNC(vbowl_state::igs011_prot2_dec_w));   // dec   (33)
	map(0x00d440, 0x00d47f).w(FUNC(vbowl_state::drgnwrld_igs011_prot2_swap_w));   // swap  (33)
	map(0x00d480, 0x00d4bf).w(FUNC(vbowl_state::igs011_prot2_reset_w));   // reset (33)
	map(0x00d4c0, 0x00d4ff).r(FUNC(vbowl_state::drgnwrldv20j_igs011_prot2_r));   // read

	map(0x50f000, 0x50f1ff).w(FUNC(vbowl_state::igs011_prot2_dec_w));   // dec   (33)
	map(0x50f200, 0x50f3ff).w(FUNC(vbowl_state::vbowl_igs011_prot2_swap_w));   // swap  (33)
	map(0x50f400, 0x50f5ff).w(FUNC(vbowl_state::igs011_prot2_reset_w));   // reset (33)
	map(0x50f600, 0x50f7ff).r(FUNC(vbowl_state::vbowl_igs011_prot2_r));   // read

	map(0x902000, 0x902fff).w(FUNC(vbowl_state::igs012_prot_reset_w));   // reset?
//  map(0x902000, 0x902005).w(FUNC(vbowl_state::igs012_prot_fake_r));

	map(0x100000, 0x103fff).ram().share("nvram");
	map(0x200000, 0x200fff).ram().share(m_priority_ram);
	map(0x300000, 0x3fffff).rw(FUNC(vbowl_state::igs011_layers_r), FUNC(vbowl_state::igs011_layers_w));
	map(0x400000, 0x400fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x401000, 0x401fff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).umask16(0x00ff).share("palette_ext");
	map(0x520000, 0x520001).portr("COIN");
	map(0x600000, 0x600007).rw(m_ics, FUNC(ics2115_device::word_r), FUNC(ics2115_device::word_w));
	map(0x700000, 0x700003).ram().share(m_vbowl_trackball);
	map(0x700005, 0x700005).w(FUNC(vbowl_state::vbowl_pen_hi_w));
	map(0x800000, 0x800001).w(FUNC(vbowl_state::igs003_w));
	map(0x800002, 0x800003).rw(FUNC(vbowl_state::vbowl_igs003_r), FUNC(vbowl_state::vbowl_igs003_w));

	map(0xa00000, 0xa00001).w(FUNC(vbowl_state::vbowl_link_0_w));
	map(0xa08000, 0xa08001).w(FUNC(vbowl_state::vbowl_link_1_w));
	map(0xa10000, 0xa10001).w(FUNC(vbowl_state::vbowl_link_2_w));
	map(0xa18000, 0xa18001).w(FUNC(vbowl_state::vbowl_link_3_w));

	map(0xa20000, 0xa20001).w(FUNC(vbowl_state::igs011_priority_w));
//  map(0xa38000, 0xa38001).w(FUNC(vbowl_state::lhb_irq_enable_w));
	map(0xa40000, 0xa40001).w(FUNC(vbowl_state::dips_w));

	map(0xa48000, 0xa48001).w(FUNC(vbowl_state::igs011_prot_addr_w));
//  map(0xa48000, 0xa48005).w(FUNC(vbowl_state::igs011_prot_fake_r));

	map(0xa58000, 0xa58001).w(FUNC(vbowl_state::igs011_blit_x_w));
	map(0xa58800, 0xa58801).w(FUNC(vbowl_state::igs011_blit_y_w));
	map(0xa59000, 0xa59001).w(FUNC(vbowl_state::igs011_blit_w_w));
	map(0xa59800, 0xa59801).w(FUNC(vbowl_state::igs011_blit_h_w));
	map(0xa5a000, 0xa5a001).w(FUNC(vbowl_state::igs011_blit_gfx_lo_w));
	map(0xa5a800, 0xa5a801).w(FUNC(vbowl_state::igs011_blit_gfx_hi_w));
	map(0xa5b000, 0xa5b001).w(FUNC(vbowl_state::igs011_blit_flags_w));
	map(0xa5b800, 0xa5b801).w(FUNC(vbowl_state::igs011_blit_pen_w));
	map(0xa5c000, 0xa5c001).w(FUNC(vbowl_state::igs011_blit_depth_w));

	map(0xa80000, 0xa80001).r(FUNC(vbowl_state::vbowl_unk_r)); // comm
	map(0xa88000, 0xa88001).r(FUNC(vbowl_state::dips_r<4>));
	map(0xa90000, 0xa90001).r(FUNC(vbowl_state::vbowl_unk_r)); // comm
	map(0xa98000, 0xa98001).r(FUNC(vbowl_state::vbowl_unk_r)); // comm
}


void vbowl_state::vbowlhk_mem(address_map &map)
{
	vbowl_mem(map);
	map(0x800002, 0x800003).w(FUNC(vbowl_state::vbowlhk_igs003_w));
	map(0x50f600, 0x50f7ff).r(FUNC(vbowl_state::vbowlhk_igs011_prot2_r));   // read
}


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( drgnwrld )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy    ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal  ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard    ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW1:6")  // DIPS 6, 7 & 8 are unlisted in DIPSW sheet
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW1:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW1:8")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Strip Girl" )            PORT_DIPLOCATION("SW2:1")  // "Open Girl" in test mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Background" )            PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, "Girl" )
	PORT_DIPSETTING(    0x00, "Scene" )     // broken backgrounds with "Strip Girl" on (PCB does the same)
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Fever Game" )            PORT_DIPLOCATION("SW2:4")  // Bonus game after round such as hitting turtles etc
	PORT_DIPSETTING(    0x08, "One Kind Only" )
	PORT_DIPSETTING(    0x00, "Several Kinds" )
	PORT_DIPUNKNOWN( 0x10, 0x10 )                       PORT_DIPLOCATION("SW2:5")  // DIPS 5, 6, 7 & 8 are unlisted in DIPSW sheet
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW2:8")

	PORT_START("DSW3")
	// On some PCBs all of DSW3 is removed
	PORT_DIPUNKNOWN( 0x01, 0x01 )                       PORT_DIPLOCATION("SW3:1")
	PORT_DIPUNKNOWN( 0x02, 0x02 )                       PORT_DIPLOCATION("SW3:2")
	PORT_DIPUNKNOWN( 0x04, 0x04 )                       PORT_DIPLOCATION("SW3:3")
	PORT_DIPUNKNOWN( 0x08, 0x08 )                       PORT_DIPLOCATION("SW3:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 )                       PORT_DIPLOCATION("SW3:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW3:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW3:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW3:8")

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )   // used?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    // press in girl test to pause, button 3 advances
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( drgnwrldc )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy    ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal  ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard    ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW1:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW1:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW1:8")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Strip Girl" )            PORT_DIPLOCATION("SW2:1")  // "Open Girl" in test mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Sex Question" )          PORT_DIPLOCATION("SW2:2")  // "background" in test mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Background" )            PORT_DIPLOCATION("SW2:3")  // "sex question" in test mode
	PORT_DIPSETTING(    0x04, "Girl" )
	PORT_DIPSETTING(    0x00, "Scene" )     // broken backgrounds with "Strip Girl" on (PCB does the same)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Tiles" )                 PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Mahjong" )
	PORT_DIPSETTING(    0x00, "Symbols" )
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPNAME( 0x40, 0x40, "Fever Game" )            PORT_DIPLOCATION("SW2:7")  // Bonus game after round such as hitting turtles etc
	PORT_DIPSETTING(    0x40, "One Kind Only" )
	PORT_DIPSETTING(    0x00, "Several Kinds" )
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )                       PORT_DIPLOCATION("SW3:1")
	PORT_DIPUNKNOWN( 0x02, 0x02 )                       PORT_DIPLOCATION("SW3:2")
	PORT_DIPUNKNOWN( 0x04, 0x04 )                       PORT_DIPLOCATION("SW3:3")
	PORT_DIPUNKNOWN( 0x08, 0x08 )                       PORT_DIPLOCATION("SW3:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 )                       PORT_DIPLOCATION("SW3:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW3:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW3:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW3:8")

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )   // used?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    // press in girl test to pause, button 3 advances
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( drgnwrldj )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy    ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal  ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard    ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW1:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW1:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW1:8")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Background" )            PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "Girl" )
	PORT_DIPSETTING(    0x00, "Scene" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Tiles" )                 PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Mahjong" )
	PORT_DIPSETTING(    0x00, "Symbols" )
	PORT_DIPUNKNOWN( 0x08, 0x08 )                       PORT_DIPLOCATION("SW2:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 )                       PORT_DIPLOCATION("SW2:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )                       PORT_DIPLOCATION("SW3:1")
	PORT_DIPUNKNOWN( 0x02, 0x02 )                       PORT_DIPLOCATION("SW3:2")
	PORT_DIPUNKNOWN( 0x04, 0x04 )                       PORT_DIPLOCATION("SW3:3")
	PORT_DIPUNKNOWN( 0x08, 0x08 )                       PORT_DIPLOCATION("SW3:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 )                       PORT_DIPLOCATION("SW3:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW3:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW3:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW3:8")

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )   // used?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    // press in girl test to pause, button 3 advances
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( lhb2 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x02, "Payout Rate" )           PORT_DIPLOCATION("SW1:1,2,3") // 機率調整
	PORT_DIPSETTING(    0x07, "50%" )
	PORT_DIPSETTING(    0x06, "54%" )
	PORT_DIPSETTING(    0x05, "58%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "66%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "74%" )
	PORT_DIPSETTING(    0x00, "78%" )
	PORT_DIPNAME( 0x08, 0x00, "Odds Rate" )             PORT_DIPLOCATION("SW1:4")     // 倍數?
	PORT_DIPSETTING(    0x00, "1,2,3,4,5,6,7,8" )
	PORT_DIPSETTING(    0x08, "1,2,3,5,8,15,30,50" )
	PORT_DIPNAME( 0x10, 0x00, "Maximum Bet" )           PORT_DIPLOCATION("SW1:5")     // 最大押注
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPNAME( 0x60, 0x60, "Minimum Bet" )           PORT_DIPLOCATION("SW1:6,7")   // 最小押注
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Credit Timer" )          PORT_DIPLOCATION("SW1:8")     // ??清除    (clears credits after timeout if you don't start a game)
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )                                        // ?
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )                                         // ?

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2")   // 投幣比率
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, "Key-in Rate" )           PORT_DIPLOCATION("SW2:3")     // 開分比率
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x08, 0x08, "Credit Limit" )          PORT_DIPLOCATION("SW2:4")     // 進分上限
	PORT_DIPSETTING(    0x08, "100" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x10, 0x10, "Credit Mode" )           PORT_DIPLOCATION("SW2:5")     // 進分方式
	PORT_DIPSETTING(    0x10, "Coin Acceptor" )                                       // 投幣
	PORT_DIPSETTING(    0x00, "Key-In" )                                              // 開分
	PORT_DIPNAME( 0x20, 0x20, "Payout Mode" )           PORT_DIPLOCATION("SW2:6")     // 退分方式
	PORT_DIPSETTING(    0x20, "Key-Out" )                                             // 洗分
	PORT_DIPSETTING(    0x00, "Return Coins" )                                        // 退幣      (doesn't seem to work properly)
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )                                     // ????
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")     // ??音?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )                                        // ?
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )                                         // ?

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:1,2")   // ??限?
	PORT_DIPSETTING(    0x03, "500" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x00, "?" )                                                   // ?限?
	PORT_DIPNAME( 0x0c, 0x0c, "Gals" )                  PORT_DIPLOCATION("SW3:3,4")   // 美女
	PORT_DIPSETTING(    0x0c, "0?" )                                                  // ?美女
	PORT_DIPSETTING(    0x08, "1?" )                                                  // ?美女
	PORT_DIPSETTING(    0x04, "2?" )                                                  // ?開女
	PORT_DIPSETTING(    0x00, "3?" )                                                  // ?開女
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )                                     // (not shown in settings display)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_CONDITION("DSW2", 0x10, EQUALS, 0x10) // 投幣
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_CONDITION("DSW2", 0x10, EQUALS, 0x00) // 投幣
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                                              // 清除
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )                                                  // 測試      (hold on start for input test)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(igs011_state::igs_hopper_r)) // 哈巴
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                                               // 查帳
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_CONDITION("DSW2", 0x20, EQUALS, 0x20) // 洗分
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_CONDITION("DSW2", 0x20, EQUALS, 0x00) // 洗分
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_INCLUDE(mahjong_matrix_1p)

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? set to 0 both
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? and you can't start a game

	PORT_MODIFY("KEY1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) // doesn't use any other gambling keys
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY2")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY3")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY4")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( nkishusp )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x02, "Payout Rate" )           PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, "74%" )
	PORT_DIPSETTING(    0x06, "77%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x04, "83%" )
	PORT_DIPSETTING(    0x03, "86%" )
	PORT_DIPSETTING(    0x02, "89%" )
	PORT_DIPSETTING(    0x01, "92%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x08, 0x00, "Odds Rate" )             PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "1,2,3,4,5,6,7,8" )
	PORT_DIPSETTING(    0x08, "1,2,3,5,8,15,30,50" )
	PORT_DIPNAME( 0x10, 0x00, "Maximum Bet" )           PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPNAME( 0x60, 0x60, "Minimum Bet" )           PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Credit Timer" )          PORT_DIPLOCATION("SW1:8")     // (clears credits after timeout if you don't start a game)
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, "Key-in Rate" )           PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x08, 0x08, "Credit Mode" )           PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Coin Acceptor" )
	PORT_DIPSETTING(    0x00, "Key-In" )
	PORT_DIPNAME( 0x10, 0x10, "Auto Play" )             PORT_DIPLOCATION("SW2:5")     // (automatically draws and discards tiles after reach)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Nudity" )                PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )                                     // (not shown in settings display)

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Credit Limit" )          PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x03, "500" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )                                           // (seems to be limited to 9,999 trying to exceed this gives "RECORD ERROR 10")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:3" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )                                     // (not shown in settings display)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08) // 投幣
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00) // 投幣
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                                              // 清除
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )                                                  // 測試      (hold on start for input test)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(igs011_state::igs_hopper_r)) // 哈巴
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                                               // 查帳
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )                                             // 洗分
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_INCLUDE(mahjong_matrix_1p)


	PORT_MODIFY("KEY0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? set to 0 both
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? and you can't start a game

	PORT_MODIFY("KEY1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) // doesn't use any other gambling keys
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY2")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY3")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY4")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


// basically same game as lhb2 and nkishusp but with joystick controls
static INPUT_PORTS_START( tygn )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Pay Out (%)" ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x07, "50" )
	PORT_DIPSETTING(    0x06, "54" )
	PORT_DIPSETTING(    0x05, "58" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x03, "66" )
	PORT_DIPSETTING(    0x02, "70" )
	PORT_DIPSETTING(    0x01, "74" )
	PORT_DIPSETTING(    0x00, "78" )
	PORT_DIPNAME( 0x08, 0x08, "Minimum Bet" ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x80, 0x80 ) PORT_DIPLOCATION("DSW1:8")

	PORT_START("DSW2") // not shown in test mode
	PORT_DIPUNKNOWN( 0x01, 0x01 ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPUNKNOWN( 0x02, 0x02 ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPUNKNOWN( 0x04, 0x04 ) PORT_DIPLOCATION("DSW2:3")
	PORT_DIPUNKNOWN( 0x08, 0x08 ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 ) PORT_DIPLOCATION("DSW2:8")

	PORT_START("DSW3") // only 2 out of 4 DIP banks phisically present
	PORT_DIPNAME( 0xff, 0xff, DEF_STR( Unused ) )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW ) // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) // stats
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) // ok
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) // ok
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) // ok
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) // ok
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // ok
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // selects input test in test mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) // selects image test in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) // selects audio test in test mode

	PORT_START("IN1") // probably unused
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2") // probably unused
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( wlcc )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x03, "1000" )
	PORT_DIPSETTING(    0x02, "1500" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Bet" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNKNOWN( 0x10, 0x10 )       // shown in test mode
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x40, "Hide Title" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x00, "9999" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" )
	PORT_DIPSETTING(    0x20, "Coins" ) // use bits 0-1
	PORT_DIPSETTING(    0x00, "Notes" ) // use bits 2-3
	PORT_DIPNAME( 0x40, 0x00, "Pay Out Type" )
	PORT_DIPSETTING(    0x00, "Coins" )
	PORT_DIPSETTING(    0x40, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1     )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2     )
	PORT_SERVICE_NO_TOGGLE( 0x04,   IP_ACTIVE_LOW ) // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE2  ) // shown in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_OTHER     ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)   // clear coin
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM   ) PORT_READ_LINE_MEMBER(FUNC(igs011_state::igs_hopper_r))   // hopper switch

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    // bet
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )
INPUT_PORTS_END


static INPUT_PORTS_START( lhb )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out (%)" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x30, 0x30, "YAKUMAN Point" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0x80, "Max Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Bet" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x70, 0x70, "DAI MANGUAN Cycle" )
	PORT_DIPSETTING(    0x70, "300" )
//  PORT_DIPSETTING(    0x60, "300" )
//  PORT_DIPSETTING(    0x50, "300" )
//  PORT_DIPSETTING(    0x40, "300" )
//  PORT_DIPSETTING(    0x30, "300" )
//  PORT_DIPSETTING(    0x20, "300" )
//  PORT_DIPSETTING(    0x10, "300" )
//  PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPNAME( 0x80, 0x80, "DAI MANGUAN Times" )
	PORT_DIPSETTING(    0x80, "2" )
//  PORT_DIPSETTING(    0x00, "2" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Max Credit" )
	PORT_DIPSETTING(    0x03, "1000" )
	PORT_DIPSETTING(    0x02, "2000" )
	PORT_DIPSETTING(    0x01, "5000" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Note" )
	PORT_DIPSETTING(    0x0c, "1000" )
	PORT_DIPSETTING(    0x08, "2000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x10, 0x10, "CPU Strength" )
	PORT_DIPSETTING(    0x10, "Strong" )
	PORT_DIPSETTING(    0x00, "Weak" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" )
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0xc0, 0xc0, "DONDEN Times" )
	PORT_DIPSETTING(    0xc0, "0" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "8" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, "Girls" )
	PORT_DIPSETTING(    0x0c, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, "Dressed" )
	PORT_DIPSETTING(    0x04, "Underwear" )
	PORT_DIPSETTING(    0x00, "Nude" )
	PORT_DIPNAME( 0x10, 0x10, "Note Rate" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x20, 0x20, "Pay Out" )
	PORT_DIPSETTING(    0x20, "Score" )
	PORT_DIPSETTING(    0x00, "Coin" )
	PORT_DIPNAME( 0x40, 0x40, "Coin In" )
	PORT_DIPSETTING(    0x40, "Credit" )
	PORT_DIPSETTING(    0x00, "Score" )
	PORT_DIPNAME( 0x80, 0x80, "Last Chance" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "In-Game Bet" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(igs011_state::igs_hopper_r)) // hopper switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )       // system reset
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )       // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )    // stats
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O) // clear coins
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )        // shows garbage in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_INCLUDE(igs_mahjong_matrix)

	//PORT_MODIFY("KEY4")
	//PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2) // shows garbage in test mode
INPUT_PORTS_END


// vbowlhk: joystick only, no cabinet linking
static INPUT_PORTS_START( vbowlhk )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Sexy Interlude" )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Open Picture" )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC(0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy   ) )   // 5
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )   // 7
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )   // 9
	PORT_DIPSETTING(    0x00, DEF_STR( Hard   ) )   // 11
	PORT_DIPNAME( 0x04, 0x04, "Spares To Win (Frames 1-5)" )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x18, 0x18, "Points To Win (Frames 6-10)" )      PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "160" )
	PORT_DIPSETTING(    0x10, "170" )
	PORT_DIPSETTING(    0x08, "180" )
	PORT_DIPSETTING(    0x00, "190" )
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC(0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC(0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, IP_ACTIVE_LOW, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC(0x10, IP_ACTIVE_LOW, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC(0x40, IP_ACTIVE_LOW, "SW3:7" )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW3:8" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x81, 0x81, "Protection & Comm Test" )      PORT_DIPLOCATION("SW4:1,8") // includes IGS011 and IGS012 protection tests
	PORT_DIPSETTING(    0x81, "No (0)" )
	PORT_DIPSETTING(    0x80, "No (1)" )
	PORT_DIPSETTING(    0x01, "No (2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "SW4:2" )
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "SW4:3" )
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "SW4:4" )
	PORT_DIPUNKNOWN_DIPLOC(0x10, IP_ACTIVE_LOW, "SW4:5" )
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "SW4:6" )
	PORT_DIPUNKNOWN_DIPLOC(0x40, IP_ACTIVE_LOW, "SW4:7" )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("AN0")
	PORT_BIT( 0xff, 0xff, IPT_UNKNOWN )

	PORT_START("AN1")
	PORT_BIT( 0xff, 0xff, IPT_UNKNOWN )
INPUT_PORTS_END


// vbowl: add trackball and linked cabinets options
static INPUT_PORTS_START( vbowl )
	PORT_INCLUDE(vbowlhk)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Controls ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Trackball ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Cabinet ID" )      PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Linked Cabinets" )      PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)
INPUT_PORTS_END


// vbowlj: remove "open picture"
static INPUT_PORTS_START( vbowlj )
	PORT_INCLUDE(vbowl)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Controls ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Trackball ) )
INPUT_PORTS_END


static INPUT_PORTS_START( xymg )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2")   // 投幣比率
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Key-in Rate" )           PORT_DIPLOCATION("SW1:3,4")   // 開分比率
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Credit Limit" )          PORT_DIPLOCATION("SW1:5")     // 進分上限
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x00, "Unlimited" )                                           // 無限制     (if you have 10,000 or more credits, further credits will be rejected)
	PORT_DIPNAME( 0x20, 0x20, "Credit Mode" )           PORT_DIPLOCATION("SW1:6")     // 進分方式
	PORT_DIPSETTING(    0x20, "Coin Acceptor" )                                       // 投幣
	PORT_DIPSETTING(    0x00, "Key-In" )                                              // 開分
	PORT_DIPNAME( 0x40, 0x40, "Payout Mode" )           PORT_DIPLOCATION("SW1:7")     // 退分方式
	PORT_DIPSETTING(    0x40, "Return Coins" )                                        // 退幣
	PORT_DIPSETTING(    0x00, "Key-Out" )                                             // 洗分
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:8")     // 示範音樂
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )                                        // 無
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )                                         // 有

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Double Up Jackpot" )     PORT_DIPLOCATION("SW2:1,2")   // 比倍爆機
	PORT_DIPSETTING(    0x03, "1000" )
	PORT_DIPSETTING(    0x02, "1500" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Minimum Bet" )           PORT_DIPLOCATION("SW2:3,4")   // 最小押注
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Double Up Game" )        PORT_DIPLOCATION("SW2:5")     // 比倍遊戲
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )                                        // 無
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )                                         // 有
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )                                     // (not shown in settings display)

	// FIXME: SW3 doesn't respond in input test - are these hooked up correctly?
	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(igs011_state::igs_hopper_r)) // hopper switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )      // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )   // stats
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_CONDITION("DSW1", 0x20, EQUALS, 0x20)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_CONDITION("DSW1", 0x20, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x40)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_INCLUDE(igs_mahjong_matrix)
INPUT_PORTS_END


/***************************************************************************

    Machine Drivers

***************************************************************************/

// for debugging

#if 0
static const gfx_layout layout_16x16x1 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(15,-1) },
	{ STEP16(0,16*1) },
	16*16*1
};

static GFXDECODE_START( gfx_igs011 )
	GFXDECODE_ENTRY( "blitter", 0, gfx_8x8x4_packed_lsb,   0, 0x80 )
	GFXDECODE_ENTRY( "blitter", 0, gfx_16x16x4_packed_lsb, 0, 0x80 )
	GFXDECODE_ENTRY( "blitter", 0, gfx_8x8x8_raw,          0, 0x08 )
	GFXDECODE_ENTRY( "blitter", 0, gfx_16x16x8_raw,        0, 0x08 )
GFXDECODE_END

static GFXDECODE_START( gfx_igs011_hi )
	GFXDECODE_ENTRY( "blitter", 0, gfx_8x8x4_packed_lsb,   0, 0x80 )
	GFXDECODE_ENTRY( "blitter", 0, gfx_16x16x4_packed_lsb, 0, 0x80 )
	GFXDECODE_ENTRY( "blitter", 0, gfx_8x8x8_raw,          0, 0x08 )
	GFXDECODE_ENTRY( "blitter", 0, gfx_16x16x8_raw,        0, 0x08 )
	GFXDECODE_ENTRY( "blitter_hi", 0, layout_16x16x1,      0, 0x80 )
GFXDECODE_END
#endif

void igs011_state::igs011_base(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(22'000'000)/3);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(igs011_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x2000/4);
	m_palette->set_membits(8);
//  GFXDECODE(config, "gfxdecode", m_palette, gfx_igs011);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	OKIM6295(config, m_oki, XTAL(22'000'000)/21, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

TIMER_DEVICE_CALLBACK_MEMBER( igs011_state::lev5_timer_irq_cb )
{
	m_maincpu->set_input_line(5, HOLD_LINE);
}

void igs011_state::drgnwrld(machine_config &config)
{
	igs011_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_state::drgnwrld_mem);
	m_maincpu->set_vblank_int("screen", FUNC(igs011_state::irq6_line_hold));
	TIMER(config, "timer_irq").configure_periodic(FUNC(igs011_state::lev5_timer_irq_cb), attotime::from_hz(240)); // lev5 frequency drives the music tempo

	YM3812(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 2.0);
}

void igs011_state::drgnwrld_igs012(machine_config &config)
{
	drgnwrld(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_state::drgnwrld_igs012_mem);
}


INTERRUPT_GEN_MEMBER(igs011_state::lhb_vblank_irq)
{
	if (!m_lhb_irq_enable)
		return;

	m_maincpu->set_input_line(6, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER( igs011_state::lhb_timer_irq_cb )
{
	if (!m_lhb_irq_enable)
		return;

	m_maincpu->set_input_line(5, HOLD_LINE);
}

void igs011_state::lhb(machine_config &config)
{
	igs011_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_state::lhb_mem);
	m_maincpu->set_vblank_int("screen", FUNC(igs011_state::lhb_vblank_irq));
	TIMER(config, "timer_irq").configure_periodic(FUNC(igs011_state::lhb_timer_irq_cb), attotime::from_hz(240)); // lev5 frequency drives the music tempo
	// irq 3 points to an apparently unneeded routine
}

TIMER_DEVICE_CALLBACK_MEMBER( igs011_state::lev3_timer_irq_cb )
{
	m_maincpu->set_input_line(3, HOLD_LINE);
}


void igs011_state::wlcc(machine_config &config)
{
	igs011_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_state::wlcc_mem);
	m_maincpu->set_vblank_int("screen", FUNC(igs011_state::irq6_line_hold));
	TIMER(config, "timer_irq").configure_periodic(FUNC(igs011_state::lev3_timer_irq_cb), attotime::from_hz(240)); // lev3 frequency drives the music tempo
}


void igs011_state::xymg(machine_config &config)
{
	igs011_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_state::xymg_mem);
	m_maincpu->set_vblank_int("screen", FUNC(igs011_state::irq6_line_hold));
	TIMER(config, "timer_irq").configure_periodic(FUNC(igs011_state::lev3_timer_irq_cb), attotime::from_hz(240)); // lev3 frequency drives the music tempo
}


void igs011_state::lhb2(machine_config &config)
{
	igs011_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_state::lhb2_mem);
	m_maincpu->set_vblank_int("screen", FUNC(igs011_state::irq6_line_hold));
	TIMER(config, "timer_irq").configure_periodic(FUNC(igs011_state::lev5_timer_irq_cb), attotime::from_hz(240)); // lev5 frequency drives the music tempo

//  GFXDECODE(config, "gfxdecode", m_palette, gfx_igs011_hi);

	YM2413(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 2.0);
}


void igs011_state::nkishusp(machine_config &config)
{
	igs011_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_state::nkishusp_mem);
	m_maincpu->set_vblank_int("screen", FUNC(igs011_state::irq6_line_hold));
	TIMER(config, "timer_irq").configure_periodic(FUNC(igs011_state::lev3_timer_irq_cb), attotime::from_hz(240)); // lev3 frequency drives the music tempo

	// VSync 60.0052Hz, HSync 15.620kHz

//  GFXDECODE(config, "gfxdecode", m_palette, gfx_igs011_hi);

	YM2413(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 2.0);
}


void igs011_state::tygn(machine_config &config)
{
	nkishusp(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_state::tygn_mem);
}


void vbowl_state::sound_irq(int state)
{
//   m_maincpu->set_input_line(3, state);
}

void vbowl_state::vbowl(machine_config &config)
{
	igs011_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vbowl_state::vbowl_mem);
	m_maincpu->set_vblank_int("screen", FUNC(vbowl_state::irq6_line_hold));
	TIMER(config, "timer_irq").configure_periodic(FUNC(vbowl_state::lev3_timer_irq_cb), attotime::from_hz(240)); // lev3 frequency drives the music tempo
	// irq 5 points to a debug function (all routines are clearly patched out)
	// irq 4 points to an apparently unneeded routine

	m_screen->screen_vblank().set(FUNC(vbowl_state::screen_vblank));
//  GFXDECODE(config, "gfxdecode", m_palette, gfx_igs011_hi);

	config.device_remove("oki");
	ICS2115(config, m_ics, 33.8688_MHz_XTAL);
	m_ics->irq().set(FUNC(vbowl_state::sound_irq));
	m_ics->add_route(ALL_OUTPUTS, "mono", 5.0);
}

void vbowl_state::vbowlhk(machine_config &config)
{
	vbowl(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vbowl_state::vbowlhk_mem);
}


/***************************************************************************

    ROMs Loading

***************************************************************************/

/***************************************************************************

Dragon World (World, V040O)
(C) 1997 IGS

Chips:
  1x 68000 (main)
  1x AC0A26 (equivalent to OKI M6295)(sound)
  1x 6564L (equivalent to YM3812)(sound)
  1x custom IGS003c (marked on PCB as 8255)
  1x oscillator 22.0000MHz (main)
  1x oscillator 3.579545MHz (sound)
  1x custom IGS011 (FPGA?)

ROMs:
  1x MX27C4096 (u3)(main) (dumped)
  1x custom IGSD0301 (mask rom) (not dumped yet)
  1x NEC D27C2001D (IGSS0302)(sound) (not dumped yet)

Notes:
  1x JAMMA edge connector
  1x trimmer (volume)
  3x 8 switches dips
  PCB serial number is: 0105-5

***************************************************************************/

ROM_START( drgnwrld )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "chinadr-v0400.u3", 0x00000, 0x80000, CRC(a6daa2b8) SHA1(0cbfd001c1fd82a6385453d1c2a808add67746af) )

	ROM_REGION( 0x400000, "blitter", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

/***************************************************************************

Dragon World (World, V030O)
(C) 1995 IGS

Chips:
  1x MC68HC000P10           U2 (main)
  1x CUSTOM IGS011          U24
  1x oscillator 22.000MHz   U25
  1x oscillator 3.579545MHz X1

ROMs:
  1x MX27C4096              U3 (main)

Notes:
  1x JAMMA edge connector
  1x trimmer (volume)
  3x 8 switches dips
  PCB serial number is: 0105-1

***************************************************************************/

ROM_START( drgnwrldv30 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "chinadr-v0300.u3", 0x00000, 0x80000, CRC(5ac243e5) SHA1(50cccff0307239187ac2b65331ad2bcc666f8033) )

	ROM_REGION( 0x400000, "blitter", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

/***************************************************************************

Dragon World (World, V021O)
(C) 1995 IGS

Chips:
    1x MC68HC000P10             u2      16/32-bit Microprocessor - main
    1x AR17961-AP0642           u41     4-Channel Mixing ADCPM Voice Synthesis LSI - sound
    1x 6564L                    u40     FM Operator Type-L II (OPL II) - sound
    1x LS138S                   u42     sound
    1x LM7805CV                 u38     sound
    1x UPC1242H                 u46     sound
    1x IGS003                   u10     Programmable Peripheral Interface
    1x oscillator   22.0000MHz  u25
    1x oscillator   3.579545MHz x1

ROMs:
    1x  M27C4096                u3      (main) dumped
    1x  custom IGSD0301         u39     not dumped yet
    x   NEC D27C2001D           u43     (sound) dumped

RAMs:
    2x  UM6264                  u4,u5
    2x  CXK5863AP               u31,u32

PLDs:
    1x  custom IGS011 (FPGA?)   u24     not dumped
    1x  custom IGS012 (FPGA?)   u1      not dumped
    2x  PAL16L8A                u17,u18 read protected
    2x  ATF22V10B               u15,u45 read protected
    1x  ATV750                  u16     read protected

***************************************************************************/

ROM_START( drgnwrldv21 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "china-dr-v-0210.u3", 0x00000, 0x80000, CRC(60c2b018) SHA1(58563e3ccb51bd9d8362aa17c23743bb5a593c3b) )

	ROM_REGION( 0x400000, "blitter", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "china-dr-sp.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

/***************************************************************************

中國龍/Chuugokuryuu (china dragon jpn ver.)
(c)IGS
Distributed by ALTA

MAIN CPU   : 68000
I/O        : IGS003 (=8255)
SOUND ?    : 6564L  (=OPL?)  , AR17961 (=M6295?)
CRTC ?     : IGS011
SOUND CPU? : IGSD0301 (DIP 42P)
OSC        : 22Mhz , 3.579545Mhz
DIPSW      : 8bitx 3 (SW3 is not used)
OTHER      : IGS012

MAIN PRG   : "CHINA DRAGON U020J" (japan)
SOUND PRG? : "CHINA DRAGON SP"
SOUND DATA?: "CHINA DRAGON U44"

***************************************************************************/

ROM_START( drgnwrldv20j )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "china_jp.v20", 0x00000, 0x80000, CRC(9e018d1a) SHA1(fe14e6344434cabf43685e50fd49c90f05f565be) )

	ROM_REGION( 0x420000, "blitter", 0 )
	// igs-d0301.u39 wasn't in this set
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )
	ROM_LOAD( "china.u44",     0x400000, 0x020000, CRC(10549746) SHA1(aebd83796679c85b43ad514b2771897f94e61294) ) // 1xxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) ) // original label: "sp"
ROM_END

ROM_START( drgnwrldv21j )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "v-021j", 0x00000, 0x80000, CRC(2f87f6e4) SHA1(d43065b078fdd9605c121988ad3092dce6cf0bf1) )

	ROM_REGION( 0x420000, "blitter", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )
	ROM_LOAD( "cg",            0x400000, 0x020000, CRC(2dda0be3) SHA1(587b7cab747d4336515c98eb3365341bb6c7e5e4) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) ) // original label: "sp"
ROM_END

/***************************************************************************

  東方之珠/Dung1 Fong1 Zi1 Zyu1
  (Hong Kong version of 中國龍/Zhōngguó Lóng, V011H)

***************************************************************************/

ROM_START( drgnwrldv11h )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "c_drgn_hk.u3", 0x00000, 0x80000, CRC(182037ce) SHA1(141b698777533e57493e588d2526523d4bd3e17d) )

	ROM_REGION( 0x400000, "blitter", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

ROM_START( drgnwrldv11ha )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "u3", 0x00000, 0x80000, CRC(b68113c4) SHA1(618fbf8ef71fcab5cd887e2d06f08169aec9e59f) ) // label not readable

	ROM_REGION( 0x400000, "blitter", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "china-dr-sp.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

/***************************************************************************

中國龍/Zhōngguó Lóng (China, V010C)
(C) 1995 IGS

Chips:
  CPU 1x MC68HC000P10 (main)
  1x AR17961-AP0642 (equivalent to OKI M6295)(sound)
  1x 6564L (equivalent to YM3812)(sound)
  1x LS138S (sound)
  1x LM7805CV (sound)
  1x UPC1242H (sound)
  1x custom IGS003 (marked on PCB as 8255)
  1x oscillator 22.0000MHz (main)
  1x oscillator 3.579545MHz (sound)
  1x custom IGS011 (FPGA?)

ROMs:
  1x maskrom 256x16 IGSD0303 (u3)(main)
  1x maskrom 2Mx16 UM23V32000 (IGSD0301)(u39)(gfx)
  1x empty socket for 27C040 (u44)
  1x maskrom NEC D27C2001D (IGSS0302)(u43)(sound)
  2x PAL16L8ACN (u17,u18)(read protected)
  2x PALATF22V10B (u15,u45)
  1x empty space for additional PALATV750 (u16)

Notes:
  1x JAMMA edge connector
  1x trimmer (volume)
  3x 8x2 switches DIP

The PCB is perfectly working, empty spaces and empty sockets are clearly intended to be empty.
25/07/2007 f205v Corrado Tomaselli Gnoppi

***************************************************************************/

ROM_START( drgnwrldv10c )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "igs-d0303.u3", 0x00000, 0x80000, CRC(3b3c29bb) SHA1(77b7e58104314303985c283cce3aec40bd7b9334) )

	ROM_REGION( 0x400000, "blitter", 0 )
	//ROM_LOAD( "igs-0301.u39", 0x000000, 0x400000, CRC(655ab941) SHA1(4bbefb27e8971446998508969661042c5111bc72) ) // bad dump
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )

	ROM_REGION( 0x40000, "plds", 0 )
	ROM_LOAD( "ccdu15.u15", 0x000, 0x2e5, CRC(a15fce69) SHA1(3e38d75c7263bfb36aebdbbd55ebbdd7ca601633) )
	//ROM_LOAD( "ccdu17.u17.bad.dump", 0x000, 0x104, CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
	//ROM_LOAD( "ccdu18.u18.bad.dump", 0x000, 0x104, CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
	ROM_LOAD( "ccdu45.u45", 0x000, 0x2e5, CRC(a15fce69) SHA1(3e38d75c7263bfb36aebdbbd55ebbdd7ca601633) )
ROM_END

/***************************************************************************

  東方之珠/dongbang jiju
  (Korea version of 中國龍/Zhōngguó Lóng)

***************************************************************************/

ROM_START( drgnwrldv40k )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "v-040k.u3", 0x00000, 0x80000, CRC(397404ef) SHA1(5228558760be7c103d4b9e2e2a31ca5619ca8055) )

	ROM_REGION( 0x400000, "blitter", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )
	// u44 unpopulated

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )

	ROM_REGION( 0x40000, "plds", 0 )
	ROM_LOAD( "ccdu15.u15", 0x000, 0x2e5, CRC(a15fce69) SHA1(3e38d75c7263bfb36aebdbbd55ebbdd7ca601633) )
	//ROM_LOAD( "ccdu17.u17.bad.dump", 0x000, 0x104, CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
	//ROM_LOAD( "ccdu18.u18.bad.dump", 0x000, 0x104, CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
	ROM_LOAD( "ccdu45.u45", 0x000, 0x2e5, CRC(a15fce69) SHA1(3e38d75c7263bfb36aebdbbd55ebbdd7ca601633) )
ROM_END

/***************************************************************************

    萬里長城/Wànlǐ Chángchéng (The Great Wall)

    Other files in the zip:

     5.942    16-6126.G16
    14.488    U3-9911.G22
    14.488    U4-82E6.G22
    14.488    U5-6C5E.G22

***************************************************************************/

ROM_START( wlcc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "wlcc4096.rom", 0x00000, 0x80000, CRC(3b16729f) SHA1(4ef4e5cbd6ccc65775e36c2c8b459bc1767d6574) )
	ROM_CONTINUE        (                 0x00000, 0x80000 ) // 1ST+2ND IDENTICAL

	ROM_REGION( 0x280000, "blitter", 0 )
	ROM_LOAD( "m0201-ig.160", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "wlcc.gfx",     0x200000, 0x080000, CRC(1f7ad299) SHA1(ab0a8fb31906519b9352ba172def48456e8d565c) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "040-c3c2.snd", 0x00000, 0x80000, CRC(220949aa) SHA1(1e0dba168a0687d32aaaed42714ae24358f4a3e7) ) // 2 banks
	ROM_CONTINUE(             0x00000, 0x80000 ) // 1ST+2ND IDENTICAL
ROM_END

/***************************************************************************

    龙虎榜/Lóng hǔ bǎng (V035C)

    Other files in the zip:

     5.938    16V8.jed
    14.464    LHB-U33.jed
    14.488    LHB-U34.jed
    14.488    LHB-U35.jed

***************************************************************************/

ROM_START( lhb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	// identical to LHB-4096
	ROM_LOAD16_WORD_SWAP( "v305j-409", 0x00000, 0x80000, CRC(701de8ef) SHA1(4a77160f642f4de02fa6fbacf595b75c0d4a505d) )

	ROM_REGION( 0x200000, "blitter", 0 )
	ROM_LOAD( "m0201-ig.160", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )

	ROM_REGION( 0x80000, "oki", 0 )
	// identical to 040-c3c2.snd
	ROM_LOAD( "m0202.snd", 0x00000, 0x80000, CRC(220949aa) SHA1(1e0dba168a0687d32aaaed42714ae24358f4a3e7) ) // 2 banks
	ROM_CONTINUE(          0x00000, 0x80000 ) // 1ST+2ND IDENTICAL
ROM_END

/***************************************************************************

龙虎榜/Lóng hǔ bǎng (V033C)

PCB Layout
----------

IGS PCB NO-T0093
|---------------------------------------|
|uPD1242H     VOL       DSW5            |
|  IGS_M0202                            |
|             AR17961   DSW4            |
|                             CY7C185   |
|                       DSW3            |
|      8255             DSW2  CY7C185   |
|                       DSW1            |
|1                                      |
|8                           DIP32      |
|W             |-------|                |
|A             |       |     IGS_M0201  |
|Y  BATTERY    |IGS011 |                |
|              |       |          PAL   |
|              |-------|                |
|                         TC524256      |
|    MAJ_V-033C                         |
|                         TC524256      |
|1      6264                            |
|0                        TC524256      |
|W      6264    22.285MHz               |
|A         PAL            TC524256      |
|Y         PAL                          |
| SPDT_SW  PAL      68000               |
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      TC524256 - Toshiba TC524256BZ-80 256k x4 Dual Port VRAM (ZIP28)
      CY7C185  - Cypress CY7C185-20PC 8k x8 SRAM (DIP28)
      6264     - UT6264PC-70LL 8k x8 SRAM (DIP28)
      IGS011   - Custom IGS IC (QFP160)
      AR17961  - == OkiM6295 (QFP44)
      DIP32    - Empty socket, maybe a ROM missing, maybe not used?

      ROMs -
            MAJ_V-033C - Main Program (27C4096)
            IGS_M0201  - Graphics (16M maskROM)
            IGS_M0202  - OKI samples (4M maskROM)

***************************************************************************/

ROM_START( lhbv33c )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "maj_v-033c.u30", 0x00000, 0x80000, CRC(02a0b716) SHA1(cd0ee32ea69f66768196b0e9b4df0fae3af84ed3) )

	ROM_REGION( 0x200000, "blitter", 0 )
	ROM_LOAD( "igs_m0201.u15", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )

	ROM_REGION( 0x80000, "oki", 0 )
	// identical to 040-c3c2.snd
	ROM_LOAD( "igs_m0202.u39", 0x00000, 0x80000, CRC(106ac5f7) SHA1(5796a880c3424e3d2251b2223a0e594957afecaf) ) // 2 banks

ROM_END

/***************************************************************************

大阪城/Daai6 Baan2 Sing4

PCB Layout
----------

IGS PCB NO-T0084-1
|---------------------------------------|
|uPD1242H     VOL       DSW5            |
|  IGS_M0202                            |
|             AR17961   DSW4            |
|                             CY7C185   |
|                       DSW3            |
|      8255             DSW2  CY7C185   |
|                       DSW1            |
|1                                      |
|8                           MAJ-H_CG   |
|W    PAL      |-------|                |
|A             |       |     IGS_M0201  |
|Y    PAL      |IGS011 |                |
|              |       |          PAL   |
|     PAL      |-------|                |
|                         TC524256      |
|     6264                              |
|                         TC524256      |
|1    6264                              |
|0           22.0994MHz   TC524256      |
|W    MAJ-H_V027H                       |
|A                        TC524256      |
|Y         BATTERY                      |
| SPDT_SW           68000               |
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      TC524256 - Toshiba TC524256BZ-80 256k x4 Dual Port VRAM (ZIP28)
      CY7C185  - Cypress CY7C185-20PC 8k x8 SRAM (DIP28)
      6264     - UT6264PC-70LL 8k x8 SRAM (DIP28)
      IGS011   - Custom IGS IC (QFP160)
      AR17961  - == OkiM6295 (QFP44)

      ROMs -
            MAJ-H_V027H- Main Program (27C4096)
            IGS_M0201  - Graphics (16M maskROM)
            IGS_M0202  - OKI samples (4M maskROM)
            MAJ-H_CG   - Graphics (27c4001 EPROM)

***************************************************************************/

ROM_START( dbc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "maj-h_v027h.u30", 0x00000, 0x80000, CRC(5d5ccd5b) SHA1(7a1223923f9a5825fd919ae9a36912284e705382) )

	ROM_REGION( 0x280000, "blitter", 0 )
	ROM_LOAD( "igs_m0201.u15", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "maj-h_cg.u8",   0x200000, 0x080000, CRC(ee45cc46) SHA1(ed011f758a02026222994aaea0677a4e9580fbda) )   // 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_m0202.u39", 0x00000, 0x80000, CRC(106ac5f7) SHA1(5796a880c3424e3d2251b2223a0e594957afecaf) ) // 2 banks
ROM_END

/***************************************************************************

龍虎榜/Mahjong Ryukobou
Alta, 1995

PCB Layout
----------

IGS PCB NO-T0094
|------------------------------------|
|UPC1242  VOL  LM7805  DSW5          |
|   MAJ-J_SP           DSW4          |
|  TD62003     M6295   DSW3 CY7C185  |
|        8255          DSW2 CY7C185  |
|M                     DSW1 MAJ-J_CG |
|A                                   |
|H              IGS011      IGS_M0201|
|J                               PAL |
|O                                   |
|N  MAJ_V030J     22MHz              |
|G    6264              TC524256     |
|     6264              TC524256     |
|     PAL               TC524256     |
|     PAL      68000P10 TC524256     |
|     PAL                            |
|------------------------------------|
Notes:
      68000 - clock 7.33333MHz [22/3]
      M6295 - clock 1.04762MHz [22/21]
      VSync - 60.0078Hz
      HSync - 15.620kHz

***************************************************************************/

ROM_START( ryukobou )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "maj_v030j.u30", 0x000000, 0x80000, CRC(186f2b4e) SHA1(380a3d94722d9f5fa5ec206ed0af6dbb8dd81715) )

	ROM_REGION( 0x280000, "blitter", 0 )
	ROM_LOAD( "igs_m0201.u15", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "maj-j_cg.u8",   0x200000, 0x080000, CRC(3c8de5d1) SHA1(51e43bfe7b5157112ded9a34b7032f73b7ffad11) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "maj-j_sp.u39", 0x000000, 0x80000, CRC(82f670f3) SHA1(d10dd67e40aee0e1c4f4b2c1217ce0935cb86406) )
ROM_END

/***************************************************************************

龍虎榜/Lung4 Fu2 Bong2 II
IGS, 1996

PCB Layout
----------

IGS PCB NO-0115
|---------------------------------------------|
|                  M6295  IGSS0503.U38        |
|  UM3567  3.57945MHz                         |
|                          DSW3               |
|                          DSW2     PAL       |
| IGSM0502.U5              DSW1    6264       |
| IGSM0501.U7     PAL              6264       |
|                 PAL                         |
|                 PAL            IGS011       |
|                 PAL                         |
|                 PAL                         |
|                                             |
|   MC68HC000P10          22MHz  TC524258AZ-10|
|           6264         8255    TC524258AZ-10|
|    BATT   6264   MAJ2V185H.U29 TC524258AZ-10|
|                                TC524258AZ-10|
|---------------------------------------------|

Notes:
        68k clock: 7.3333MHz (i.e. 22/3)
      M6295 clock: 1.0476MHz (i.e. 22/21) \
         M6295 SS: HIGH                   / Therefore sampling freq = 7.936363636kHz (i.e. 1047600 / 132)
           UM3567: Compatible with YM2413, clock = 3.57945MHz
            HSync: 15.78kHz
            VSync: 60Hz

***************************************************************************/

ROM_START( lhb2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "maj2v185h.u29", 0x00000, 0x80000, CRC(2572d59a) SHA1(1d5362e209dadf8b21c10d1351d4bb038bfcaaef) )

	ROM_REGION( 0x200000, "blitter", 0 )
	ROM_LOAD( "igsm0501.u7", 0x00000, 0x200000, CRC(1c952bd6) SHA1(a6b6f1cdfb29647e81c032ffe59c94f1a10ceaf8) )

	ROM_REGION( 0x80000, "blitter_hi", 0 ) // high order bit of graphics (5th bit)
	/* these are identical ..seems ok as igs number is same, only ic changed */
	ROM_LOAD( "igsm0502.u4", 0x00000, 0x80000, CRC(5d73ae99) SHA1(7283aa3d6b15ceb95db80756892be46eb997ef15) )
	ROM_LOAD( "igsm0502.u5", 0x00000, 0x80000, CRC(5d73ae99) SHA1(7283aa3d6b15ceb95db80756892be46eb997ef15) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igss0503.u38", 0x00000, 0x80000, CRC(c9609c9c) SHA1(f036e682b792033409966e84292a69275eaa05e5) )  // 2 banks
ROM_END

/***************************************************************************

Mahjong Nenrikishu SP (V250J)
IGS/Alta, 1998

PCB Layout
----------

IGS PCB NO-0115-5
|----------------------------------|
|UPC1242H VOL M6295     SP         |
|   U3567 LM7805                   |
|   CG   3.579545MHz   DSW3        |
|  M0502               DSW2   PAL  |
|M                     DSW1  6264  |
|A M0501                     6264  |
|H              PAL        IGS011  |
|J              PAL                |
|O              PAL                |
|N              PAL                |
|G                                 |
|                   22MHz M5M442256|
|     68000P16            M5M442256|
|RESET    6264     8255   M5M442256|
| BATT    6264  V250J     M5M442256|
|----------------------------------|
Notes:
      68000 - Clock 7.3333MHz [22/2]
      U3567 - YM2413, clock 3.579545MHz
      M6295 - Clock 1.0476 [22/21]. pin 7 high
      VSync - 60.0052Hz
      HSync - 15.620kHz

***************************************************************************/

ROM_START( nkishusp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "v250j.u29", 0x00000, 0x80000, CRC(500cb919) SHA1(76eed80b59c43e8cc1e258056cfe08b33a651852) )

	ROM_REGION( 0x800000, "blitter", ROMREGION_ERASEFF )
	ROM_LOAD( "m0501.u7", 0x000000, 0x200000, CRC(1c952bd6) SHA1(a6b6f1cdfb29647e81c032ffe59c94f1a10ceaf8) ) // Identical to igsm0501.u7
	// empty
	ROM_LOAD( "cg.u4",    0x400000, 0x080000, CRC(fe60f485) SHA1(d75e5f7a187161137a7f7b54d495d1cb3e1802a4) )
	// empty

	ROM_REGION( 0x200000, "blitter_hi", ROMREGION_ERASEFF ) // high order bit of graphics (5th bit)
	ROM_LOAD( "m0502.u6", 0x00000, 0x80000, CRC(5d73ae99) SHA1(7283aa3d6b15ceb95db80756892be46eb997ef15) ) // Identical to igsm0502.u4, igsm0502.u5
	// empty

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "sp.u38", 0x00000, 0x80000, CRC(d80e28e2) SHA1(c03441686e770227db6a2a41922fbb4284710571) )
ROM_END

/*********************************************************************************

Te Yi Gong Neng, IGS 1996

This is the joystick version of LHB2

PCB Layout
----------

IGS PCB NO- 0127-2
|-----------------------------------------|
|UPC1242          U3587   K668   S0503.U42|
|                     3.579545MHz         |
|                            SW1          |
|   M0502.U3                 SW2          |
|                            SW3          |
|                            SW4          |
|                                   6164  |
|J                                        |
|A  M0501.U7             PAL        6164  |
|M                       PAL              |
|M                       PAL              |
|A                       PAL      IGS011  |
|                        PAL              |
|                                         |
|     68000            22MHz   KM428C256  |
|           27C4096.U13                   |
|    T518B  IGS003A     6164   KM428C256  |
| SW5  BATTERY          6164              |
|-----------------------------------------|
Notes:
      68000 - Clock 7.33333MHz [22/3]
       K668 - Oki M6295 clone. Clock 1.047619048MHz [22/21]
      U3587 - This is a YM2412 clone. Clock input 3.579545MHz
  SW1/2/3/4 - 8-Position DIP Switch. SW3 and SW4 not populated
        SW5 - Reset / NVRAM Clear
       6164 - 8kBx8-bit SRAM
  KM428C256 - Dual-Port Video RAM with 256kBx8-bit DRAM and 512x8 SAM (Serial Access Memory).
              This is the same type of RAM used on Midway T-Unit hardware but 2X larger.

*********************************************************************************/

ROM_START( tygn ) // GFX ROMs same as lhb2 and nkishusp, sound ROM same as lhb2
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "27c4096.u13", 0x00000, 0x80000, CRC(25b1de1e) SHA1(7c3d0458b30db614e551fdf0db3f93fa44fcb758) )

	ROM_REGION( 0x800000, "blitter", ROMREGION_ERASEFF )
	ROM_LOAD( "m0501.u7", 0x000000, 0x200000, CRC(1c952bd6) SHA1(a6b6f1cdfb29647e81c032ffe59c94f1a10ceaf8) )

	ROM_REGION( 0x200000, "blitter_hi", ROMREGION_ERASEFF ) // high order bit of graphics (5th bit)
	ROM_LOAD( "m0502.u3", 0x00000, 0x80000, CRC(5d73ae99) SHA1(7283aa3d6b15ceb95db80756892be46eb997ef15) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s0502.u42", 0x00000, 0x80000, CRC(c9609c9c) SHA1(f036e682b792033409966e84292a69275eaa05e5) )
ROM_END


/***************************************************************************

Virtua Bowling by IGS

PCB # 0101

U45  (27c240) is probably program
next to 68000 processor
U68,U69 probably images   (27c800 - mask)
U67, U66 sound and ????  (27c040)

ASIC chip used

SMD - custom chip IGS 011      F5XD  174
SMD - custom --near sound section - unknown -- i.d. rubbed off
SMD - custom  -- near inputs and 68000  IGS012    9441EK001

XTL near sound 33.868mhz
XTL near 68000  22.0000mhz

there are 4 banks of 8 dip switches

***************************************************************************/

ROM_START( vbowl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "bowlingv101xcm.u45", 0x00000, 0x80000, BAD_DUMP CRC(ab8e3f1f) SHA1(69159e22559d6a26fe2afafd770aa640c192ba4b) )

	ROM_REGION( 0x400000 * 2, "blitter", 0)
	ROM_LOAD( "vrbowlng.u69", 0x000000, 0x400000, CRC(b0d339e8) SHA1(a26a5e0202a78e8cdc562b10d64e14eadfa4e115) )
	// extra space to expand every 4 bits to 8

	ROM_REGION( 0x100000, "blitter_hi", ROMREGION_INVERT )
	ROM_LOAD( "vrbowlng.u68", 0x000000, 0x100000, CRC(b0ce27e7) SHA1(6d3ef97edd606f384b1e05b152fbea12714887b7) )

	ROM_REGION( 0x400000, "ics", 0 )
	ROM_LOAD( "vrbowlng.u67", 0x00000, 0x80000, CRC(53000936) SHA1(e50c6216f559a9248c095bdfae05c3be4be79ff3) )  // 8 bit signed mono & u-law
	ROM_LOAD( "vrbowlng.u66", 0x80000, 0x80000, CRC(f62cf8ed) SHA1(c53e47e2c619ed974ad40ee4aaa4a35147ea8311) )  // 8 bit signed mono
	ROM_COPY( "ics", 0x000000, 0x100000,0x100000)
	ROM_COPY( "ics", 0x000000, 0x200000,0x100000)
	ROM_COPY( "ics", 0x000000, 0x300000,0x100000)
ROM_END

ROM_START( vbowlj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "vrbowlng.u45", 0x00000, 0x80000, CRC(091c19c1) SHA1(5a7bfbee357122e9061b38dfe988c3853b0984b0) ) // second half all 00

	ROM_REGION( 0x400000 * 2, "blitter", 0)
	ROM_LOAD( "vrbowlng.u69", 0x000000, 0x400000, CRC(b0d339e8) SHA1(a26a5e0202a78e8cdc562b10d64e14eadfa4e115) )
	// extra space to expand every 4 bits to 8

	ROM_REGION( 0x100000, "blitter_hi", ROMREGION_INVERT )
	ROM_LOAD( "vrbowlng.u68", 0x000000, 0x100000, CRC(b0ce27e7) SHA1(6d3ef97edd606f384b1e05b152fbea12714887b7) )

	ROM_REGION( 0x400000, "ics", 0 )
	ROM_LOAD( "vrbowlng.u67", 0x00000, 0x80000, CRC(53000936) SHA1(e50c6216f559a9248c095bdfae05c3be4be79ff3) )  // 8 bit signed mono & u-law
	ROM_LOAD( "vrbowlng.u66", 0x80000, 0x80000, CRC(f62cf8ed) SHA1(c53e47e2c619ed974ad40ee4aaa4a35147ea8311) )  // 8 bit signed mono
	ROM_COPY( "ics", 0x000000, 0x100000,0x100000)
	ROM_COPY( "ics", 0x000000, 0x200000,0x100000)
	ROM_COPY( "ics", 0x000000, 0x300000,0x100000)
ROM_END

ROM_START( vbowlhk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "bowlingv101hjs.u45", 0x00000, 0x80000, CRC(92fbfa72) SHA1(824b666ea2247ef5a974675925bcef94dbd1a992) ) // second half all 00

	ROM_REGION( 0x400000 * 2, "blitter", 0)
	ROM_LOAD( "vrbowlng.u69", 0x000000, 0x400000, CRC(b0d339e8) SHA1(a26a5e0202a78e8cdc562b10d64e14eadfa4e115) )
	// extra space to expand every 4 bits to 8

	ROM_REGION( 0x100000, "blitter_hi", ROMREGION_INVERT )
	ROM_LOAD( "vrbowlng.u68", 0x000000, 0x100000, CRC(b0ce27e7) SHA1(6d3ef97edd606f384b1e05b152fbea12714887b7) )

	ROM_REGION( 0x400000, "ics", 0 )
	ROM_LOAD( "vrbowlng.u67", 0x00000, 0x80000, CRC(53000936) SHA1(e50c6216f559a9248c095bdfae05c3be4be79ff3) )  // 8 bit signed mono & u-law
	ROM_LOAD( "vrbowlng.u66", 0x80000, 0x80000, CRC(f62cf8ed) SHA1(c53e47e2c619ed974ad40ee4aaa4a35147ea8311) )  // 8 bit signed mono
	ROM_COPY( "ics", 0x000000, 0x100000,0x100000)
	ROM_COPY( "ics", 0x000000, 0x200000,0x100000)
	ROM_COPY( "ics", 0x000000, 0x300000,0x100000)
ROM_END

/***************************************************************************

    幸運滿貫 (Xìngyùn Mǎn Guàn)

    Other files in the zip:

    14.484 U33-82E6.jed
    14.484 U34-1.jed
    14.484 U35-7068.jed

***************************************************************************/

ROM_START( xymg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "u30-ebac.rom", 0x00000, 0x80000, CRC(7d272b6f) SHA1(15fd1be23cabdc77b747541f5cd9fed6b08be4ad) )

	ROM_REGION( 0x280000, "blitter", 0 )
	ROM_LOAD( "m0201-ig.160", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "ygxy-u8.rom",  0x200000, 0x080000, CRC(56a2706f) SHA1(98bf4b3153eef53dd449e2538b4b7ff2cc2fe6fa) )

	ROM_REGION( 0x80000, "oki", 0 )
	// identical to 040-c3c2.snd
	ROM_LOAD( "m0202.snd", 0x00000, 0x80000, CRC(220949aa) SHA1(1e0dba168a0687d32aaaed42714ae24358f4a3e7) ) // 2 banks
	ROM_CONTINUE(          0x00000, 0x80000 ) // 1ST+2ND IDENTICAL
ROM_END

} // anonymous namespace


/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 1997, drgnwrld,      0,        drgnwrld,        drgnwrld,  igs011_state, init_drgnwrld,     ROT0, "IGS",                     "Dragon World (World, V040O)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv40k,  drgnwrld, drgnwrld_igs012, drgnwrldc, igs011_state, init_drgnwrldv40k, ROT0, "IGS",                     "Dongbang Jiju (Korea, V040K)",                MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv30,   drgnwrld, drgnwrld,        drgnwrld,  igs011_state, init_drgnwrldv30,  ROT0, "IGS",                     "Dragon World (World, V030O)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv21,   drgnwrld, drgnwrld_igs012, drgnwrld,  igs011_state, init_drgnwrldv21,  ROT0, "IGS",                     "Dragon World (World, V021O)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv21j,  drgnwrld, drgnwrld_igs012, drgnwrldj, igs011_state, init_drgnwrldv21j, ROT0, "IGS / Alta",              "Chuugokuryuu (Japan, V021J)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv20j,  drgnwrld, drgnwrld_igs012, drgnwrldj, igs011_state, init_drgnwrldv20j, ROT0, "IGS / Alta",              "Chuugokuryuu (Japan, V020J)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv11h,  drgnwrld, drgnwrld,        drgnwrldc, igs011_state, init_drgnwrldv11h, ROT0, "IGS",                     "Dung Fong Zi Zyu (Hong Kong, V011H, set 1)",  MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv11ha, drgnwrld, drgnwrld_igs012, drgnwrldc, igs011_state, init_drgnwrldv40k, ROT0, "IGS",                     "Dung Fong Zi Zyu (Hong Kong, V011H, set 2)",  MACHINE_SUPPORTS_SAVE ) // different encryption and with IGS012
GAME( 1995, drgnwrldv10c,  drgnwrld, drgnwrld,        drgnwrldc, igs011_state, init_drgnwrldv10c, ROT0, "IGS",                     "Zhongguo Long (China, V010C)",                MACHINE_SUPPORTS_SAVE )
GAME( 1995, lhb,           0,        lhb,             lhb,       igs011_state, init_lhb,          ROT0, "IGS",                     "Long Hu Bang (China, V035C)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1995, lhbv33c,       lhb,      lhb,             lhb,       igs011_state, init_lhbv33c,      ROT0, "IGS",                     "Long Hu Bang (China, V033C)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1995, dbc,           lhb,      lhb,             lhb,       igs011_state, init_dbc,          ROT0, "IGS",                     "Daai Baan Sing (Hong Kong, V027H)",           MACHINE_SUPPORTS_SAVE )
GAME( 1995, ryukobou,      lhb,      lhb,             lhb,       igs011_state, init_ryukobou,     ROT0, "IGS / Alta",              "Mahjong Ryukobou (Japan, V030J)",             MACHINE_SUPPORTS_SAVE )
GAME( 1996, lhb2,          0,        lhb2,            lhb2,      igs011_state, init_lhb2,         ROT0, "IGS",                     "Lung Fu Bong II (Hong Kong, V185H)",          MACHINE_SUPPORTS_SAVE )
GAME( 1996, tygn,          lhb2,     tygn,            tygn,      igs011_state, init_tygn,         ROT0, "IGS",                     "Te Yi Gong Neng (China, V632C)",              MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // needs correct IGS003 routines
GAME( 1996, xymg,          0,        xymg,            xymg,      igs011_state, init_xymg,         ROT0, "IGS",                     "Xingyun Man Guan (China, V651C)",             MACHINE_SUPPORTS_SAVE )
GAME( 1996, wlcc,          xymg,     wlcc,            wlcc,      igs011_state, init_wlcc,         ROT0, "IGS",                     "Wanli Changcheng (China, V638C)",             MACHINE_SUPPORTS_SAVE )
GAME( 1996, vbowl,         0,        vbowl,           vbowl,     vbowl_state,  init_vbowl,        ROT0, "IGS",                     "Virtua Bowling (World, V101XCM)",             MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1996, vbowlj,        vbowl,    vbowl,           vbowlj,    vbowl_state,  init_vbowlj,       ROT0, "IGS / Alta",              "Virtua Bowling (Japan, V100JCM)",             MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1996, vbowlhk,       vbowl,    vbowlhk,         vbowlhk,   vbowl_state,  init_vbowlhk,      ROT0, "IGS / Tai Tin Amusement", "Virtua Bowling (Hong Kong, V101HJS)",         MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1998, nkishusp,      lhb2,     nkishusp,        nkishusp,  igs011_state, init_nkishusp,     ROT0, "IGS / Alta",              "Mahjong Nenrikishu SP (Japan, V250J)",        MACHINE_SUPPORTS_SAVE )
