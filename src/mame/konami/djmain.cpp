// license:BSD-3-Clause
// copyright-holders:smf
/*
 *  Beatmania DJ Main Board (GX753)
 *
 *  Product numbers:
 *  GQ753 beatmania (first release in 1997.12)
 *  Gx853 beatmania 2nd MIX (1998.03)
 *  Gx825 beatmania 3rd MIX (1998.09)
 *  Gx858 beatmania complete MIX (1999.01)
 *  Gx847 beatmania 4th MIX (1999.04)
 *  Gx981 beatmania 5th MIX (1999.09)
 *  Gx988 beatmania complete MIX 2 (2000.01)
 *  Gx993 beatmania Club MIX (2000.03)
 *  Gx995 beatmania featuring Dreams Come True (2000.05)
 *  GxA05 beatmania CORE REMIX (2000.11)
 *  GxA21 beatmania 6th MIX (2001.07)
 *  GxB07 beatmania 7th MIX (2002.01)
 *  GxC01 beatmania THE FINAL (2002.07))
 *
 *  Gx803 Pop'n Music 1 (1998.09)
 *  Gx831 Pop'n Music 2 (1999.04)
 *  Gx980 Pop'n Music 3 (1999.09)
 *
 *  GQ970 Pop'n Stage (1999.11)
 *  Gx970 Pop'n Stage EX (2000.03)
 *
 *  Chips:
 *  15a:    MC68EC020FG25
 *  25b:    001642
 *  18d:    055555 (priority encoder)
 *   5f:    056766 (sprites)
 *  18f:    056832 (tiles)
 *  22f:    058143 = 054156 (tiles)
 *  12j:    058141 = 054539 (x2) (2 sound chips in one)
 *
 *  TODO:
 *  - correct FPS
 *
 */

/*

Dumping a HD image.

2.5 inch    2.5 to                                  2.5 to
hard drive  3.5 adapter     long 3.5 IDE cable      3.5 adapter   PCB
               /---|-    |----------------------|    -|---\
|------|-   |-/    |-    |----------------------|    -|    \-|    -|
|      |-   |      |-    |----------------------|    -|      |    -|
|------|-   |-\    |-    |----------------------|    -|    /-|    -|
               \---|-    |----------------------|    -|---/
                  ||                                  ||
                  ||                            /\    ||<-- Power connector
                  ||                            ||          not used
                  ||                            ||
                  ||
               ---------                   unplug here
               |  PC   |                   when game PCB is booted
               |Power  |                   and working. Boot Windows and stop at menu (F8)
               |Supply |                   Then plug HD into PC IDE controller, and continue boot process
               |+5V and|                   then dump the hard drive with Winhex
               |GND    |                   once PC is booted up again.
               ---------

*/

#include "emu.h"

#include "k054156_k054157_k056832.h"
#include "k055555.h"
#include "konami_helper.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/hdd.h"
#include "cpu/m68000/m68020.h"
#include "sound/k054539.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define DISABLE_VB_INT  (!(m_v_ctrl & 0x8000))
#define NUM_SPRITES (0x800 / 16)
#define NUM_LAYERS  2

class djmain_state : public driver_device
{
public:
	djmain_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_obj_ram(*this, "obj_ram")
		, m_maincpu(*this, "maincpu")
		, m_k056832(*this, "k056832")
		, m_k055555(*this, "k055555")
		, m_ata(*this, "ata")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_turntable(*this, "TT%u", 1U)
		, m_sndram(*this, "sndram")
		, m_leds(*this, "led%u", 0U)
		, m_right_red_hlt(*this, "right-red-hlt")
		, m_left_red_hlt(*this, "left-red-hlt")
		, m_right_blue_hlt(*this, "right-blue-hlt")
		, m_left_blue_hlt(*this, "left-blue-hlt")
		, m_right_ssr(*this, "right-ssr")
		, m_left_ssr(*this, "left-ssr")
	{
	}

	void djmainj(machine_config &config);
	void djmainu(machine_config &config);
	void djmaina(machine_config &config);

	void init_bm7thmix();
	void init_bm6thmix();
	void init_hmcompmx();
	void init_bscompmx();
	void init_bmfinal();
	void init_hmcompm2();
	void init_bm5thmix();
	void init_bm4thmix();
	void init_bs4thmix();
	void init_beatmania();
	void init_bmdct();
	void init_bmcompm2();
	void init_bmcorerm();
	void init_bmclubmx();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void sndram_bank_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sndram_r(offs_t offset, uint32_t mem_mask = ~0);
	void sndram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t obj_ctrl_r(offs_t offset);
	void obj_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t obj_rom_r(offs_t offset, uint32_t mem_mask = ~0);
	void v_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t v_rom_r(offs_t offset, uint32_t mem_mask = ~0);
	uint8_t inp1_r(offs_t offset);
	uint8_t inp2_r(offs_t offset);
	uint32_t turntable_r(offs_t offset, uint32_t mem_mask = ~0);
	void turntable_select_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void light_ctrl_1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void light_ctrl_2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void unknown590000_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void unknown802000_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void unknownc02000_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t screen_update_djmain(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vb_interrupt);
	void ide_interrupt(int state);
	void draw_sprites( bitmap_rgb32 &bitmap, const rectangle &cliprect);
	K056832_CB_MEMBER(tile_callback);
	void k054539_map(address_map &map) ATTR_COLD;
	void maincpu_djmain(address_map &map) ATTR_COLD;
	void maincpu_djmaina(address_map &map) ATTR_COLD;
	void maincpu_djmainj(address_map &map) ATTR_COLD;
	void maincpu_djmainu(address_map &map) ATTR_COLD;

	required_shared_ptr<uint32_t> m_obj_ram;
	required_device<cpu_device> m_maincpu;
	required_device<k056832_device> m_k056832;
	required_device<k055555_device> m_k055555;
	required_device<ata_interface_device> m_ata;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_ioport_array<2> m_turntable;
	required_shared_ptr<uint8_t> m_sndram;
	output_finder<3> m_leds;
	output_finder<> m_right_red_hlt;
	output_finder<> m_left_red_hlt;
	output_finder<> m_right_blue_hlt;
	output_finder<> m_left_blue_hlt;
	output_finder<> m_right_ssr;
	output_finder<> m_left_ssr;

	int m_sndram_bank = 0;
	int m_turntable_select = 0;
	uint8_t m_turntable_last_pos[2]{};
	uint16_t m_turntable_pos[2]{};
	uint8_t m_pending_vb_int = 0U;
	uint16_t m_v_ctrl = 0U;
	uint32_t m_obj_regs[0xa0/4]{};
	const uint8_t *m_ata_user_password = nullptr;
	const uint8_t *m_ata_master_password = nullptr;
};


void djmain_state::draw_sprites( bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int offs, pri_code;
	int sortedlist[NUM_SPRITES];

	m_gfxdecode->gfx(0)->set_colorbase(m_k055555->K055555_read_register(K55_PALBASE_SUB2) * 0x400);

	for (offs = 0; offs < NUM_SPRITES; offs++)
		sortedlist[offs] = -1;

	/* prebuild a sorted table */
	for (offs = 0; offs < NUM_SPRITES * 4; offs += 4)
	{
		if (m_obj_ram[offs] & 0x00008000)
		{
			if (m_obj_ram[offs] & 0x80000000)
				continue;

			pri_code = m_obj_ram[offs] & (NUM_SPRITES - 1);
			sortedlist[pri_code] = offs;
		}
	}

	for (pri_code = NUM_SPRITES - 1; pri_code >= 0; pri_code--)
	{
		static const int xoffset[8] = { 0, 1, 4, 5, 16, 17, 20, 21 };
		static const int yoffset[8] = { 0, 2, 8, 10, 32, 34, 40, 42 };
		static const int sizetab[4] =  { 1, 2, 4, 8 };
		int x, y;
		int ox, oy;
		int flipx, flipy;
		int xscale, yscale;
		int code;
		int color;
		int size;

		offs = sortedlist[pri_code];
		if (offs == -1) continue;

		code = m_obj_ram[offs] >> 16;
		flipx = (m_obj_ram[offs] >> 10) & 1;
		flipy = (m_obj_ram[offs] >> 11) & 1;
		size = sizetab[(m_obj_ram[offs] >> 8) & 3];

		ox = (int16_t)(m_obj_ram[offs + 1] & 0xffff);
		oy = (int16_t)(m_obj_ram[offs + 1] >> 16);

		xscale = m_obj_ram[offs + 2] >> 16;
		yscale = m_obj_ram[offs + 2] & 0xffff;

		if (!xscale || !yscale)
			continue;

		xscale = (0x40 << 16) / xscale;
		yscale = (0x40 << 16) / yscale;
		ox -= (size * xscale) >> 13;
		oy -= (size * yscale) >> 13;

		color = (m_obj_ram[offs + 3] >> 16) & 15;

		for (x = 0; x < size; x++)
			for (y = 0; y < size; y++)
			{
				int c = code;

				if (flipx)
					c += xoffset[size - x - 1];
				else
					c += xoffset[x];

				if (flipy)
					c += yoffset[size - y - 1];
				else
					c += yoffset[y];

				if (xscale != 0x10000 || yscale != 0x10000)
				{
					int sx = ox + ((x * xscale + (1 << 11)) >> 12);
					int sy = oy + ((y * yscale + (1 << 11)) >> 12);
					int zw = ox + (((x + 1) * xscale + (1 << 11)) >> 12) - sx;
					int zh = oy + (((y + 1) * yscale + (1 << 11)) >> 12) - sy;


								m_gfxdecode->gfx(0)->zoom_transpen(bitmap,
								cliprect,
								c,
								color,
								flipx,
								flipy,
								sx,
								sy,
								(zw << 16) / 16,
								(zh << 16) / 16,
								0);
				}
				else
				{
					int sx = ox + (x << 4);
					int sy = oy + (y << 4);


							m_gfxdecode->gfx(0)->transpen(bitmap,
							cliprect,
							c,
							color,
							flipx,
							flipy,
							sx,
							sy,
							0);
				}
			}
	}
}


K056832_CB_MEMBER(djmain_state::tile_callback)
{
}

void djmain_state::video_start()
{
	m_k056832->set_layer_offs(0, -92, -27);
	// m_k056832->set_layer_offs(1, -87, -27);
	m_k056832->set_layer_offs(1, -88, -27);
}

uint32_t djmain_state::screen_update_djmain(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int enables = m_k055555->K055555_read_register(K55_INPUT_ENABLES);
	int pri[NUM_LAYERS + 1];
	int order[NUM_LAYERS + 1];
	int i, j;

	for (i = 0; i < NUM_LAYERS; i++)
		pri[i] = m_k055555->K055555_read_register(K55_PRIINP_0 + i * 3);
	pri[i] = m_k055555->K055555_read_register(K55_PRIINP_10);

	for (i = 0; i < NUM_LAYERS + 1; i++)
		order[i] = i;

	for (i = 0; i < NUM_LAYERS; i++)
		for (j = i + 1; j < NUM_LAYERS + 1; j++)
			if (pri[order[i]] > pri[order[j]])
			{
				int temp = order[i];

				order[i] = order[j];
				order[j] = temp;
			}

	bitmap.fill(m_palette->pen(0), cliprect);

	for (i = 0; i < NUM_LAYERS + 1; i++)
	{
		int layer = order[i];

		if (layer == NUM_LAYERS)
		{
			if (enables & K55_INP_SUB2)
				draw_sprites(bitmap, cliprect);
		}
		else
		{
			if (enables & (K55_INP_VRAM_A << layer))
				m_k056832->tilemap_draw_dj(screen, bitmap, cliprect, layer, 0, 1 << i);
		}
	}
	return 0;
}


/*************************************
 *
 *  68k CPU memory handlers
 *
 *************************************/

void djmain_state::sndram_bank_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
		m_sndram_bank = (data >> 16) & 0x1f;
	}
}

uint32_t djmain_state::sndram_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;

	offset |= 0x20000 * m_sndram_bank;
	if (ACCESSING_BITS_24_31)
		data |= m_sndram[offset * 4] << 24;

	if (ACCESSING_BITS_16_23)
		data |= m_sndram[offset * 4 + 1] << 16;

	if (ACCESSING_BITS_8_15)
		data |= m_sndram[offset * 4 + 2] << 8;

	if (ACCESSING_BITS_0_7)
		data |= m_sndram[offset * 4 + 3];

	return data;
}

void djmain_state::sndram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	offset |= 0x20000 * m_sndram_bank;
	if (ACCESSING_BITS_24_31)
		m_sndram[offset * 4] = data >> 24;

	if (ACCESSING_BITS_16_23)
		m_sndram[offset * 4 + 1] = data >> 16;

	if (ACCESSING_BITS_8_15)
		m_sndram[offset * 4 + 2] = data >> 8;

	if (ACCESSING_BITS_0_7)
		m_sndram[offset * 4 + 3] = data;
}


//---------

uint32_t djmain_state::obj_ctrl_r(offs_t offset)
{
	// read m_obj_regs[0x0c/4]: unknown
	// read m_obj_regs[0x24/4]: unknown

	return m_obj_regs[offset];
}

void djmain_state::obj_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// write m_obj_regs[0x28/4]: bank for rom readthrough

	COMBINE_DATA(&m_obj_regs[offset]);
}

uint32_t djmain_state::obj_rom_r(offs_t offset, uint32_t mem_mask)
{
	uint8_t *mem8 = memregion("gfx1")->base();
	int bank = m_obj_regs[0x28/4] >> 16;

	offset += bank * 0x200;
	offset *= 4;

	if (ACCESSING_BITS_0_15)
		offset += 2;

	if (mem_mask & 0xff00ff00)
		offset++;

	return mem8[offset] * 0x01010101;
}


//---------

void djmain_state::v_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
		data >>= 16;
		mem_mask >>= 16;
		COMBINE_DATA(&m_v_ctrl);

		if (m_pending_vb_int && !(!(m_v_ctrl & 0x8000))) // #define DISABLE_VB_INT  (!(m_v_ctrl & 0x8000))
		{
			m_pending_vb_int = 0;
			m_maincpu->set_input_line(M68K_IRQ_4, HOLD_LINE);
		}
	}
}

uint32_t djmain_state::v_rom_r(offs_t offset, uint32_t mem_mask)
{
	uint8_t *mem8 = memregion("k056832")->base();
	int bank = m_k056832->word_r(0x34/2);

	offset *= 2;

	if (!ACCESSING_BITS_24_31)
		offset += 1;

	offset += bank * 0x800 * 4;

	if (m_v_ctrl & 0x020)
		offset += 0x800 * 2;

	return mem8[offset] * 0x01010000;
}


//---------

uint8_t djmain_state::inp1_r(offs_t offset)
{
	static const char *const portnames[] = { "DSW3", "BTN3", "BTN2", "BTN1" };
	return ioport(portnames[ offset & 0x03 ])->read();
}

uint8_t djmain_state::inp2_r(offs_t offset)
{
	static const char *const portnames[] = { "DSW1", "DSW2", "UNK2", "UNK1" };
	return ioport(portnames[ offset & 0x03 ])->read();
}

uint32_t djmain_state::turntable_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;

	if (ACCESSING_BITS_8_15)
	{
		uint8_t pos;
		int delta;

		pos = m_turntable[m_turntable_select].read_safe(0);
		delta = pos - m_turntable_last_pos[m_turntable_select];
		if (delta < -128)
			delta += 256;
		if (delta > 128)
			delta -= 256;

		m_turntable_pos[m_turntable_select] += delta * 70;
		m_turntable_last_pos[m_turntable_select] = pos;

		result |= m_turntable_pos[m_turntable_select] & 0xff00;
	}

	return result;
}

void djmain_state::turntable_select_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_23)
		m_turntable_select = (data >> 19) & 1;
}


//---------

// light/coin blocker control

/*
 beatmania/hiphopmania
    0x5d0000 (MSW16):
    bit 0: 1P button 1 LED
        1: 1P button 2 LED
        2: 1P button 3 LED
        3: 1P button 4 LED
        4: 1P button 5 LED
        5: Right blue HIGHLIGHT (active low)
        6: 2P button 1 LED
        7: 2P button 2 LED
        8: 2P button 3 LED
        9: Left blue HIGHLIGHT  (active low)
       10: Left red HIGHLIGHT   (active low)
       11: Right red HIGHLIGHT  (active low)
    12-15: not used?        (always low)

    0x5d2000 (MSW16):
        0: 1P START button LED
        1: 2P START button LED
        2: EFFECT button LED
     3-10: not used?        (always low)
       11: SSR
       12: 2P button 4 LED
       13: 2P button 5 LED
       14: COIN BLOCKER     (active low)
       15: not used?        (always low)
*/

void djmain_state::light_ctrl_1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
		m_right_red_hlt = !BIT(data, 27);   // Right red HIGHLIGHT
		m_left_red_hlt = !BIT(data, 26);   // Left red HIGHLIGHT
		m_left_blue_hlt = !BIT(data, 25);   // Left blue HIGHLIGHT
		m_right_blue_hlt = !BIT(data, 21);   // Right blue HIGHLIGHT
	}
}

void djmain_state::light_ctrl_2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
		m_left_ssr = !!BIT(data, 27);  // SSR
		m_right_ssr = !!BIT(data, 27);    // SSR
		m_leds[0] = BIT(data, 16);            // 1P START
		m_leds[1] = BIT(data, 17);            // 2P START
		m_leds[2] = BIT(data, 18);            // EFFECT
	}
}


//---------

// unknown ports :-(

void djmain_state::unknown590000_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//logerror("%08X: unknown 590000 write %08X: %08X & %08X\n", m_maincpu->pcbase(), offset, data, mem_mask);
}

void djmain_state::unknown802000_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//logerror("%08X: unknown 802000 write %08X: %08X & %08X\n", m_maincpu->pcbase(), offset, data, mem_mask);
}

void djmain_state::unknownc02000_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//logerror("%08X: unknown c02000 write %08X: %08X & %08X\n", m_maincpu->pcbase(), offset, data, mem_mask);
}



/*************************************
 *
 *  Interrupt handlers
 *
 *************************************/

INTERRUPT_GEN_MEMBER(djmain_state::vb_interrupt)
{
	m_pending_vb_int = 0;

	if (DISABLE_VB_INT)
	{
		m_pending_vb_int = 1;
		return;
	}

	//logerror("V-Blank interrupt\n");
	device.execute().set_input_line(M68K_IRQ_4, HOLD_LINE);
}


void djmain_state::ide_interrupt(int state)
{
	if (state != CLEAR_LINE)
	{
		//logerror("IDE interrupt asserted\n");
		m_maincpu->set_input_line(M68K_IRQ_1, HOLD_LINE);
	}
	else
	{
		//logerror("IDE interrupt cleared\n");
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
	}
}




/*************************************
 *
 *  Memory definitions
 *
 *************************************/

void djmain_state::maincpu_djmain(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                         // PRG ROM
	map(0x400000, 0x40ffff).ram();                         // WORK RAM
	map(0x480000, 0x48443f).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");       // COLOR RAM
	map(0x500000, 0x57ffff).rw(FUNC(djmain_state::sndram_r), FUNC(djmain_state::sndram_w));               // SOUND RAM
	map(0x580000, 0x58003f).rw(m_k056832, FUNC(k056832_device::word_r), FUNC(k056832_device::word_w));      // VIDEO REG (tilemap)
	map(0x590000, 0x590007).w(FUNC(djmain_state::unknown590000_w));                  // ??
	map(0x5a0000, 0x5a005f).w(m_k055555, FUNC(k055555_device::K055555_long_w));                  // 055555: priority encoder
	map(0x5b0000, 0x5b04ff).rw("k054539_1", FUNC(k054539_device::read), FUNC(k054539_device::write)).umask32(0xff00ff00);
	map(0x5b0000, 0x5b04ff).rw("k054539_2", FUNC(k054539_device::read), FUNC(k054539_device::write)).umask32(0x00ff00ff);
	map(0x5c0000, 0x5c0003).r(FUNC(djmain_state::inp1_r));  //  DSW3,BTN3,BTN2,BTN1  // input port control (buttons and DIP switches)
	map(0x5c8000, 0x5c8003).r(FUNC(djmain_state::inp2_r));  //  DSW1,DSW2,UNK2,UNK1  // input port control (DIP switches)
	map(0x5d0000, 0x5d0003).w(FUNC(djmain_state::light_ctrl_1_w));                   // light/coin blocker control
	map(0x5d2000, 0x5d2003).w(FUNC(djmain_state::light_ctrl_2_w));                   // light/coin blocker control
	map(0x5d4000, 0x5d4003).w(FUNC(djmain_state::v_ctrl_w));                     // VIDEO control
	map(0x5d6000, 0x5d6003).w(FUNC(djmain_state::sndram_bank_w));                    // SOUND RAM bank
	map(0x5e0000, 0x5e0003).rw(FUNC(djmain_state::turntable_r), FUNC(djmain_state::turntable_select_w));      // input port control (turn tables)
	map(0x600000, 0x601fff).r(FUNC(djmain_state::v_rom_r));                       // VIDEO ROM readthrough (for POST)
	map(0x801000, 0x8017ff).ram().share("obj_ram");             // OBJECT RAM
	map(0x802000, 0x802fff).w(FUNC(djmain_state::unknown802000_w));                  // ??
	map(0x803000, 0x80309f).rw(FUNC(djmain_state::obj_ctrl_r), FUNC(djmain_state::obj_ctrl_w));           // OBJECT REGS
	map(0x803800, 0x803fff).r(FUNC(djmain_state::obj_rom_r));                     // OBJECT ROM readthrough (for POST)
}

void djmain_state::maincpu_djmainj(address_map &map)
{
	maincpu_djmain(map);

	map(0xc00000, 0xc01fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));  // VIDEO RAM (tilemap) (beatmania)
	map(0xc02000, 0xc02047).w(FUNC(djmain_state::unknownc02000_w));                  // ??
	map(0xf00000, 0xf0000f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w)); // IDE control regs (beatmania)
	map(0xf40000, 0xf4000f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w)); // IDE status control reg (beatmania)
}

void djmain_state::maincpu_djmainu(address_map &map)
{
	maincpu_djmain(map);

	map(0xd00000, 0xd0000f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w)); // IDE control regs (hiphopmania)
	map(0xd40000, 0xd4000f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w)); // IDE status control reg (hiphopmania)
	map(0xe00000, 0xe01fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));  // VIDEO RAM (tilemap) (hiphopmania)
}

void djmain_state::maincpu_djmaina(address_map &map)
{
	maincpu_djmain(map);

	map(0xc00000, 0xc0000f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w)); // IDE control regs
	map(0xc40000, 0xc4000f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w)); // IDE status control reg
	map(0xf00000, 0xf01fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));  // VIDEO RAM (tilemap)
}

void djmain_state::k054539_map(address_map &map)
{
	map(0x000000, 0xffffff).ram().share("sndram");
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

// #define PRIORITY_EASINESS_TO_PLAY

//--------- beatmania

static INPUT_PORTS_START( beatmania_btn ) // and turntables
	PORT_START("BTN1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_START("BTN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("Effect") /* EFFECT */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_START("BTN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F2)   /* TEST SW */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service")  /* SERVICE */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset")        /* RESET SW */
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("UNK1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("UNK2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("TT1")       /* turn table 1P */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_PLAYER(1)
	PORT_START("TT2")       /* turn table 2P */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

#ifdef PRIORITY_EASINESS_TO_PLAY
	#define BEATMANIA_DSW1_COINAGE_OLD \
		PORT_DIPNAME( 0x1f, 0x1f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:8,7,6,5,4") \
		PORT_DIPSETTING(    0x1e, "1P 3C / 2P 6C / Continue 3C" ) \
		PORT_DIPSETTING(    0x01, "1P 3C / 2P 6C / Continue 2C" ) \
		PORT_DIPSETTING(    0x11, "1P 3C / 2P 6C / Continue 1C" ) \
		PORT_DIPSETTING(    0x15, "1P 3C / 2P 3C / Continue 3C" ) \
		PORT_DIPSETTING(    0x0d, "1P 3C / 2P 3C / Continue 2C" ) \
		PORT_DIPSETTING(    0x1d, "1P 3C / 2P 3C / Continue 1C" ) \
		PORT_DIPSETTING(    0x09, "1P 3C / 2P 4C / Continue 3C" ) \
		PORT_DIPSETTING(    0x19, "1P 3C / 2P 4C / Continue 2C" ) \
		PORT_DIPSETTING(    0x05, "1P 3C / 2P 4C / Continue 1C" ) \
		PORT_DIPSETTING(    0x03, "1P 2C / 2P 4C / Continue 2C" ) \
		PORT_DIPSETTING(    0x1f, "1P 2C / 2P 4C / Continue 1C" ) \
		PORT_DIPSETTING(    0x0b, "1P 2C / 2P 3C / Continue 2C" ) \
		PORT_DIPSETTING(    0x1b, "1P 2C / 2P 3C / Continue 1C" ) \
		PORT_DIPSETTING(    0x07, "1P 2C / 2P 2C / Continue 2C" ) \
		PORT_DIPSETTING(    0x17, "1P 2C / 2P 2C / Continue 1C" ) \
		PORT_DIPSETTING(    0x0f, "1P 1C / 2P 2C / Continue 1C" ) \
		PORT_DIPSETTING(    0x13, "1P 1C / 2P 1C / Continue 1C" ) \
		PORT_DIPSETTING(    0x00, "Free_Play" )
#else
	#define BEATMANIA_DSW1_COINAGE_OLD \
		PORT_DIPNAME( 0x1f, 0x1f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:8,7,6,5,4") \
		PORT_DIPSETTING(    0x1e, "1P 3C / 2P 6C / Continue 3C" ) \
		PORT_DIPSETTING(    0x01, "1P 3C / 2P 6C / Continue 2C" ) \
		PORT_DIPSETTING(    0x11, "1P 3C / 2P 6C / Continue 1C" ) \
		PORT_DIPSETTING(    0x15, "1P 3C / 2P 3C / Continue 3C" ) \
		PORT_DIPSETTING(    0x0d, "1P 3C / 2P 3C / Continue 2C" ) \
		PORT_DIPSETTING(    0x1d, "1P 3C / 2P 3C / Continue 1C" ) \
		PORT_DIPSETTING(    0x09, "1P 3C / 2P 4C / Continue 3C" ) \
		PORT_DIPSETTING(    0x19, "1P 3C / 2P 4C / Continue 2C" ) \
		PORT_DIPSETTING(    0x05, "1P 3C / 2P 4C / Continue 1C" ) \
		PORT_DIPSETTING(    0x03, "1P 2C / 2P 4C / Continue 2C" ) \
		PORT_DIPSETTING(    0x1f, "1P 2C / 2P 4C / Continue 1C" ) \
		PORT_DIPSETTING(    0x0b, "1P 2C / 2P 3C / Continue 2C" ) \
		PORT_DIPSETTING(    0x1b, "1P 2C / 2P 3C / Continue 1C" ) \
		PORT_DIPSETTING(    0x07, "1P 2C / 2P 2C / Continue 2C" ) \
		PORT_DIPSETTING(    0x17, "1P 2C / 2P 2C / Continue 1C" ) \
		PORT_DIPSETTING(    0x0f, "1P 1C / 2P 2C / Continue 1C" ) \
		PORT_DIPSETTING(    0x13, "1P 1C / 2P 1C / Continue 1C" ) \
			PORT_DIPSETTING(  0x0e, "Free_Play" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x16, "Free_Play" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x06, "Free_Play" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x1a, "Free_Play" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x0a, "Free_Play" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x12, "Free_Play" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x02, "Free_Play" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x1c, "Free_Play" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x0c, "Free_Play" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x14, "Free_Play" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x04, "Free_Play" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x18, "Free_Play" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x08, "Free_Play" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x10, "Free_Play" ) /* duplicated setting */ \
		PORT_DIPSETTING(    0x00, "Free_Play" )
#endif

#define BEATMANIA_DSW1_COINAGE_NEW \
	PORT_DIPNAME( 0x20, 0x20, "Free Play (Ignore Coinage)" ) PORT_DIPLOCATION("SW1:3") \
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x1f, 0x1f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:8,7,6,5,4") \
	PORT_DIPSETTING(    0x00, "1P 8C / 2P 16C / Continue 8C" ) \
	PORT_DIPSETTING(    0x01, "1P 8C / 2P 16C / Continue 7C" ) \
	PORT_DIPSETTING(    0x02, "1P 8C / 2P 16C / Continue 6C" ) \
	PORT_DIPSETTING(    0x03, "1P 7C / 2P 14C / Continue 7C" ) \
	PORT_DIPSETTING(    0x04, "1P 7C / 2P 14C / Continue 6C" ) \
	PORT_DIPSETTING(    0x05, "1P 7C / 2P 14C / Continue 5C" ) \
	PORT_DIPSETTING(    0x06, "1P 6C / 2P 12C / Continue 6C" ) \
	PORT_DIPSETTING(    0x07, "1P 6C / 2P 12C / Continue 5C" ) \
	PORT_DIPSETTING(    0x08, "1P 6C / 2P 12C / Continue 4C" ) \
	PORT_DIPSETTING(    0x09, "1P 5C / 2P 10C / Continue 5C" ) \
	PORT_DIPSETTING(    0x0b, "1P 5C / 2P 10C / Continue 3C" ) \
	PORT_DIPSETTING(    0x0a, "1P 5C / 2P 10C / Continue 4C" ) \
	PORT_DIPSETTING(    0x0c, "1P 4C / 2P 8C / Continue 4C" ) \
	PORT_DIPSETTING(    0x0d, "1P 4C / 2P 8C / Continue 3C" ) \
	PORT_DIPSETTING(    0x0e, "1P 4C / 2P 8C / Continue 2C" ) \
	PORT_DIPSETTING(    0x0f, "1P 3C / 2P 6C / Continue 3C" ) \
	PORT_DIPSETTING(    0x10, "1P 3C / 2P 6C / Continue 2C" ) \
	PORT_DIPSETTING(    0x11, "1P 3C / 2P 6C / Continue 1C" ) \
	PORT_DIPSETTING(    0x12, "1P 3C / 2P 4C / Continue 3C" ) \
	PORT_DIPSETTING(    0x13, "1P 3C / 2P 4C / Continue 2C" ) \
	PORT_DIPSETTING(    0x14, "1P 3C / 2P 4C / Continue 1C" ) \
	PORT_DIPSETTING(    0x15, "1P 3C / 2P 3C / Continue 3C" ) \
	PORT_DIPSETTING(    0x16, "1P 3C / 2P 3C / Continue 2C" ) \
	PORT_DIPSETTING(    0x17, "1P 3C / 2P 3C / Continue 1C" ) \
	PORT_DIPSETTING(    0x1a, "1P 2C / 2P 3C / Continue 2C" ) \
	PORT_DIPSETTING(    0x1b, "1P 2C / 2P 3C / Continue 1C" ) \
	PORT_DIPSETTING(    0x18, "1P 2C / 2P 4C / Continue 2C" ) \
	PORT_DIPSETTING(    0x1f, "1P 2C / 2P 4C / Continue 1C" ) \
	PORT_DIPSETTING(    0x1c, "1P 2C / 2P 2C / Continue 2C" ) \
	PORT_DIPSETTING(    0x1d, "1P 2C / 2P 2C / Continue 1C" ) \
	PORT_DIPSETTING(    0x1e, "1P 1C / 2P 2C / Continue 1C" ) \
	PORT_DIPSETTING(    0x19, "1P 1C / 2P 1C / Continue 1C" )

#define BEATMANIA_DSW2_SCOREDISPLAY \
	PORT_DIPNAME( 0x80, 0x80, "Score Display" ) PORT_DIPLOCATION("SW2:1") \
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) \
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

#define BEATMANIA_DSW2_DEMOSOUNDS \
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3,2") \
	PORT_DIPSETTING(    0x60, "Loud" ) \
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) ) \
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) ) \
	PORT_DIPSETTING(    0x00, "Silent" )

#define BEATMANIA_DSW2_LEVELDISPLAY \
	PORT_DIPNAME( 0x10, 0x10, "Level Display" ) PORT_DIPLOCATION("SW2:4") \
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) \
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )

#define BEATMANIA_DSW2_DIFFICULITY_OLD \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:8,7,6,5") \
	PORT_DIPSETTING(    0x0a, "Level 0" ) \
	PORT_DIPSETTING(    0x0e, "Level 1" ) \
	PORT_DIPSETTING(    0x0d, "Level 2" ) \
	PORT_DIPSETTING(    0x0c, "Level 3" ) \
	PORT_DIPSETTING(    0x0b, "Level 4" ) \
	PORT_DIPSETTING(    0x0f, "Level 5" ) \
	PORT_DIPSETTING(    0x09, "Level 6" ) \
	PORT_DIPSETTING(    0x08, "Level 7" ) \
	PORT_DIPSETTING(    0x07, "Level 8" ) \
	PORT_DIPSETTING(    0x06, "Level 9" ) \
	PORT_DIPSETTING(    0x05, "Level 10" ) \
	PORT_DIPSETTING(    0x04, "Level 11" ) \
	PORT_DIPSETTING(    0x03, "Level 12" ) \
	PORT_DIPSETTING(    0x02, "Level 13" ) \
	PORT_DIPSETTING(    0x01, "Level 14" ) \
	PORT_DIPSETTING(    0x00, "Level 15" )

#define BEATMANIA_DSW2_DIFFICULITY_NEW( str, str2 ) \
	PORT_DIPNAME( 0x0c, 0x0c, str ) PORT_DIPLOCATION("SW2:6,5") \
	PORT_DIPSETTING(    0x08, "Level 0" ) \
	PORT_DIPSETTING(    0x0c, "Level 1" ) \
	PORT_DIPSETTING(    0x04, "Level 2" ) \
	PORT_DIPSETTING(    0x00, "Level 3" ) \
	PORT_DIPNAME( 0x03, 0x03, str2 ) PORT_DIPLOCATION("SW2:8,7") \
	PORT_DIPSETTING(    0x02, "Level 0" ) \
	PORT_DIPSETTING(    0x03, "Level 1" ) \
	PORT_DIPSETTING(    0x01, "Level 2" ) \
	PORT_DIPSETTING(    0x00, "Level 3" )

#define BEATMANIA_DSW3_EVENTMODE \
	PORT_DIPNAME( 0x20, 0x20, "Event Mode (Ignore Coinage)" ) PORT_DIPLOCATION("SW3:1") \
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

#ifdef PRIORITY_EASINESS_TO_PLAY
	#define BEATMANIA_DSW3_STAGES_OLD \
		PORT_DIPNAME( 0x1c, 0x1c, "Normal Mode Stages" ) PORT_CONDITION("DSW3", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x10, "3 Stages" ) \
		PORT_DIPSETTING(    0x00, "4 Stages" ) \
		PORT_DIPSETTING(    0x08, "5 Stages" ) \
		PORT_DIPNAME( 0x1c, 0x1c, "Event Mode Stages" ) PORT_CONDITION("DSW3", 0x20, NOTEQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x0c, "1 Stage" ) \
		PORT_DIPSETTING(    0x14, "2 Stages" ) \
		PORT_DIPSETTING(    0x10, "3 Stages" ) \
		PORT_DIPSETTING(    0x00, "4 Stages" ) \
		PORT_DIPSETTING(    0x08, "5 Stages" )
#else
	#define BEATMANIA_DSW3_STAGES_OLD \
		PORT_DIPNAME( 0x1c, 0x1c, "Normal Mode Stages" ) PORT_CONDITION("DSW3", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x10, "3 Stages" ) \
			PORT_DIPSETTING(  0x1c, "4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x0c, "4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x14, "4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x04, "4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x18, "4 Stages" ) /* duplicated setting */ \
		PORT_DIPSETTING(    0x00, "4 Stages" ) \
		PORT_DIPSETTING(    0x08, "5 Stages" ) \
		PORT_DIPNAME( 0x1c, 0x1c, "Event Mode Stages" ) PORT_CONDITION("DSW3", 0x20, NOTEQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x0c, "1 Stage" ) \
		PORT_DIPSETTING(    0x14, "2 Stages" ) \
		PORT_DIPSETTING(    0x10, "3 Stages" ) \
			PORT_DIPSETTING(  0x1c, "4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x04, "4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x18, "4 Stages" ) /* duplicated setting */ \
		PORT_DIPSETTING(    0x00, "4 Stages" ) \
		PORT_DIPSETTING(    0x08, "5 Stages" )
#endif

#ifdef PRIORITY_EASINESS_TO_PLAY
	#define BEATMANIA_DSW3_STAGES_MIDDLE( str ) \
		PORT_DIPNAME( 0x1c, 0x1c, str ) PORT_CONDITION("DSW3", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x10, "3 / 3 Stages" ) \
		PORT_DIPSETTING(    0x00, "4 / 4 Stages" ) \
		PORT_DIPSETTING(    0x08, "5 / 5 Stages" ) \
		PORT_DIPNAME( 0x1c, 0x1c, "Event Mode Stages" ) PORT_CONDITION("DSW3", 0x20, NOTEQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x0c, "1 Stage" ) \
		PORT_DIPSETTING(    0x14, "2 Stages" ) \
		PORT_DIPSETTING(    0x10, "3 Stages" ) \
		PORT_DIPSETTING(    0x00, "4 Stages" ) \
		PORT_DIPSETTING(    0x08, "5 Stages" )
#else
	#define BEATMANIA_DSW3_STAGES_MIDDLE( str ) \
		PORT_DIPNAME( 0x1c, 0x1c, str ) PORT_CONDITION("DSW3", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x10, "3 / 3 Stages" ) \
			PORT_DIPSETTING(  0x1c, "4 / 4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x0c, "4 / 4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x14, "4 / 4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x04, "4 / 4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x18, "4 / 4 Stages" ) /* duplicated setting */ \
		PORT_DIPSETTING(    0x00, "4 / 4 Stages" ) \
		PORT_DIPSETTING(    0x08, "5 / 5 Stages" ) \
		PORT_DIPNAME( 0x1c, 0x1c, "Event Mode Stages" ) PORT_CONDITION("DSW3", 0x20, NOTEQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x0c, "1 Stage" ) \
		PORT_DIPSETTING(    0x14, "2 Stages" ) \
		PORT_DIPSETTING(    0x10, "3 Stages" ) \
			PORT_DIPSETTING(  0x1c, "4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x04, "4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x18, "4 Stages" ) /* duplicated setting */ \
		PORT_DIPSETTING(    0x00, "4 Stages" ) \
		PORT_DIPSETTING(    0x08, "5 Stages" )
#endif

#ifdef PRIORITY_EASINESS_TO_PLAY
	#define BEATMANIA_DSW3_STAGES_NEW( str ) \
		PORT_DIPNAME( 0x1c, 0x1c, str ) PORT_CONDITION("DSW3", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x10, "3 / 2 Stages" ) \
		PORT_DIPSETTING(    0x00, "4 / 3 Stages" ) \
		PORT_DIPSETTING(    0x08, "5 / 3 Stages" ) \
		PORT_DIPNAME( 0x1c, 0x1c, "Event Mode Stages" ) PORT_CONDITION("DSW3", 0x20, NOTEQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x0c, "1 Stage" ) \
		PORT_DIPSETTING(    0x14, "2 Stages" ) \
		PORT_DIPSETTING(    0x10, "3 Stages" ) \
		PORT_DIPSETTING(    0x00, "4 Stages" ) \
		PORT_DIPSETTING(    0x08, "5 Stages" )
#else
	#define BEATMANIA_DSW3_STAGES_NEW( str ) \
		PORT_DIPNAME( 0x1c, 0x1c, str ) PORT_CONDITION("DSW3", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x10, "3 / 2 Stages" ) \
			PORT_DIPSETTING(  0x1c, "4 / 3 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x0c, "4 / 3 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x14, "4 / 3 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x04, "4 / 3 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x18, "4 / 3 Stages" ) /* duplicated setting */ \
		PORT_DIPSETTING(    0x00, "4 / 3 Stages" ) \
		PORT_DIPSETTING(    0x08, "5 / 3 Stages" ) \
		PORT_DIPNAME( 0x1c, 0x1c, "Event Mode Stages" ) PORT_CONDITION("DSW3", 0x20, NOTEQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x0c, "1 Stage" ) \
		PORT_DIPSETTING(    0x14, "2 Stages" ) \
		PORT_DIPSETTING(    0x10, "3 Stages" ) \
			PORT_DIPSETTING(  0x1c, "4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x04, "4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x18, "4 Stages" ) /* duplicated setting */ \
		PORT_DIPSETTING(    0x00, "4 Stages" ) \
		PORT_DIPSETTING(    0x08, "5 Stages" )
#endif

#define BM1STMIX_DSW1 \
	PORT_START("DSW1") \
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:1" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:2" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:3" ) \
	BEATMANIA_DSW1_COINAGE_OLD /* SW1:8,7,6,5,4 */

#define BM1STMIX_DSW2 \
	PORT_START("DSW2") \
	PORT_DIPNAME( 0x80, 0x80, "Enable Expert Mode" ) PORT_DIPLOCATION("SW2:1") \
	PORT_DIPSETTING(    0x00, DEF_STR( No ) ) \
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) ) \
	BEATMANIA_DSW2_DEMOSOUNDS /* SW2:3,2 */ \
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:4" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:5" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:6" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:7" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:8" )

#define BM1STMIX_DSW3 \
	PORT_START("DSW3") \
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) /* bit 7,6 don't exist */ \
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:1" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:2" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:3" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:4" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:5" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:6" )

#define BM2NDMIX_DSW1 \
	PORT_START("DSW1") \
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:1" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:2" ) \
	BEATMANIA_DSW1_COINAGE_NEW /* SW1:8,7,6,5,4,3 */

#define BM2NDMIX_DSW2 \
	PORT_START("DSW2") \
	BEATMANIA_DSW2_SCOREDISPLAY /* SW2:1 */ \
	BEATMANIA_DSW2_DEMOSOUNDS /* SW2:3,2 */ \
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:4" ) \
	BEATMANIA_DSW2_DIFFICULITY_OLD /* SW2:3,8,7,6,5 */

#define BM2NDMIX_DSW3 \
	PORT_START("DSW3") \
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) /* bit 7,6 don't exist */ \
	BEATMANIA_DSW3_EVENTMODE /* SW3:1 */ \
	PORT_DIPNAME( 0x18, 0x18, "Event Mode Stages" ) PORT_DIPLOCATION("SW3:3,2") \
	PORT_DIPSETTING(    0x18, "1 Stage" ) \
	PORT_DIPSETTING(    0x08, "2 Stages" ) \
	PORT_DIPSETTING(    0x10, "3 Stages" ) \
	PORT_DIPSETTING(    0x00, "4 Stages" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:4" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:5" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:6" )

#define BM3RDMIX_DSW3 \
	PORT_START("DSW3") \
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) /* bit 7,6 don't exist */ \
	BEATMANIA_DSW3_EVENTMODE /* SW3:1 */ \
	BEATMANIA_DSW3_STAGES_OLD /* SW3:4,3,2 */ \
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:5" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:6" )

#define BM4THMIX_DSW2( str, str2 ) \
	PORT_START("DSW2") \
	BEATMANIA_DSW2_SCOREDISPLAY /* SW2:1 */ \
	BEATMANIA_DSW2_DEMOSOUNDS /* SW2:3,2 */ \
	BEATMANIA_DSW2_LEVELDISPLAY /* SW2:4 */ \
	BEATMANIA_DSW2_DIFFICULITY_NEW( str, str2 ) /* SW2:8,7,6,5 */

#define BM4THMIX_DSW3( str ) \
	PORT_START("DSW3") \
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) /* bit 7,6 don't exist */ \
	BEATMANIA_DSW3_EVENTMODE /* SW3:1 */ \
	BEATMANIA_DSW3_STAGES_MIDDLE( str ) /* SW3:4,3,2 */ \
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:5" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:6" )

#define HMCOMPM2_DSW3( str ) \
	PORT_START("DSW3") \
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) /* bit 7,6 don't exist */ \
	BEATMANIA_DSW3_EVENTMODE /* SW3:1 */ \
	BEATMANIA_DSW3_STAGES_MIDDLE( str ) /* SW3:4,3,2 */ \
	PORT_DIPNAME( 0x02, 0x02, "Game Over Mode" ) PORT_DIPLOCATION("SW3:5") \
	PORT_DIPSETTING(    0x02, "On Stage Middle" ) \
	PORT_DIPSETTING(    0x00, "On Stage Last" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:6" )

#ifdef PRIORITY_EASINESS_TO_PLAY
	#define BMDCT_DSW3( str ) \
		PORT_START("DSW3") \
		PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) /* bit 7,6 don't exist */ \
		BEATMANIA_DSW3_EVENTMODE /* SW3:1 */ \
		PORT_DIPNAME( 0x1c, 0x1c, str ) PORT_CONDITION("DSW3", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x1c, "3 / 3 Stages" ) \
		PORT_DIPSETTING(    0x10, "4 / 4 Stages" ) \
		PORT_DIPNAME( 0x1c, 0x1c, "Event Mode Stages" ) PORT_CONDITION("DSW3", 0x20, NOTEQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x0c, "1 Stage" ) \
		PORT_DIPSETTING(    0x14, "2 Stages" ) \
		PORT_DIPSETTING(    0x10, "3 Stages" ) \
		PORT_DIPSETTING(    0x1c, "4 Stages" ) \
		PORT_DIPSETTING(    0x08, "5 Stages" ) \
		PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:5" ) \
		PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:6" )
#else
	#define BMDCT_DSW3( str ) \
		PORT_START("DSW3") \
		PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) /* bit 7,6 don't exist */ \
		BEATMANIA_DSW3_EVENTMODE /* SW3:1 */ \
		PORT_DIPNAME( 0x1c, 0x1c, str ) PORT_CONDITION("DSW3", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x1c, "3 / 3 Stages" ) \
			PORT_DIPSETTING(  0x0c, "3 / 3 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x14, "3 / 3 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x04, "3 / 3 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x18, "3 / 3 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x08, "3 / 3 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x00, "3 / 3 Stages" ) /* duplicated setting */ \
		PORT_DIPSETTING(    0x10, "4 / 4 Stages" ) \
		PORT_DIPNAME( 0x1c, 0x1c, "Event Mode Stages" ) PORT_CONDITION("DSW3", 0x20, NOTEQUALS, 0x20) PORT_DIPLOCATION("SW3:4,3,2") \
		PORT_DIPSETTING(    0x0c, "1 Stage" ) \
		PORT_DIPSETTING(    0x14, "2 Stages" ) \
		PORT_DIPSETTING(    0x10, "3 Stages" ) \
		PORT_DIPSETTING(    0x1c, "4 Stages" ) \
			PORT_DIPSETTING(  0x04, "4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x18, "4 Stages" ) /* duplicated setting */ \
			PORT_DIPSETTING(  0x00, "4 Stages" ) /* duplicated setting */ \
		PORT_DIPSETTING(    0x08, "5 Stages" ) \
		PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:5" ) \
		PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:6" )
#endif

#define BM6THMIX_DSW3( str ) \
	PORT_START("DSW3") \
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) /* bit 7,6 don't exist */ \
	BEATMANIA_DSW3_EVENTMODE /* SW3:1 */ \
	BEATMANIA_DSW3_STAGES_NEW( str ) /* SW3:4,3,2 */ \
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:5" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:6" )

static INPUT_PORTS_START( bm1stmix )
	PORT_INCLUDE( beatmania_btn )
	BM1STMIX_DSW1
	BM1STMIX_DSW2
	BM1STMIX_DSW3
INPUT_PORTS_END

static INPUT_PORTS_START( bm2ndmix )
	PORT_INCLUDE( beatmania_btn )
	BM2NDMIX_DSW1
	BM2NDMIX_DSW2
	BM2NDMIX_DSW3
	/* "Free Hidden Songs" 3-3=On 3-6=On */
	PORT_MODIFY("DSW3")
	BEATMANIA_DSW3_EVENTMODE /* SW3:1 */
	PORT_DIPNAME( 0x10, 0x10, "Unused (Used if Event Mode)" ) PORT_CONDITION("DSW3", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Free Hidden Songs (step1of2)" ) PORT_CONDITION("DSW3", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x08, "No (Off)" )
	PORT_DIPSETTING(    0x00, "Yes (On)" )
	PORT_DIPNAME( 0x18, 0x18, "Event Mode Stages" ) PORT_CONDITION("DSW3", 0x20, NOTEQUALS, 0x20) PORT_DIPLOCATION("SW3:3,2")
	PORT_DIPSETTING(    0x18, "1 Stage" )
	PORT_DIPSETTING(    0x08, "2 Stages" )
	PORT_DIPSETTING(    0x10, "3 Stages" )
	PORT_DIPSETTING(    0x00, "4 Stages" )
	PORT_DIPNAME( 0x01, 0x01, "Free Hidden Songs (step2of2)" ) PORT_CONDITION("DSW3", 0x20, EQUALS, 0x20) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x01, "No (Off)" )
	PORT_DIPSETTING(    0x00, "Yes (On)" )
	PORT_DIPNAME( 0x01, 0x01, "Unused (Used if not Event Mode)" ) PORT_CONDITION("DSW3", 0x20, NOTEQUALS, 0x20) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* "Free Hidden Songs" 3-3=On 3-6=On */
INPUT_PORTS_END

static INPUT_PORTS_START( bm3rdmix )
	PORT_INCLUDE( beatmania_btn )
	BM2NDMIX_DSW1
	BM2NDMIX_DSW2
	BM3RDMIX_DSW3
INPUT_PORTS_END

static INPUT_PORTS_START( bmcompmx )
	PORT_INCLUDE( beatmania_btn )
	BM2NDMIX_DSW1
	BM2NDMIX_DSW2
	BM3RDMIX_DSW3
	/* "Free Secret Expert Course" 1-1=Off 1-2=On 2-4=On 3-5=Off 3-6=On */
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x80, 0x80, "Free Secret Expert Course (step1of5)" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, "Yes (Off)" )
	PORT_DIPSETTING(    0x00, "No (On)" )
	PORT_DIPNAME( 0x40, 0x40, "Free Secret Expert Course (step2of5)" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "No (Off)" )
	PORT_DIPSETTING(    0x00, "Yes (On)" )
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x10, 0x10, "Free Secret Expert Course (step3of5)" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, "No (Off)" )
	PORT_DIPSETTING(    0x00, "Yes (On)" )
	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x02, 0x02, "Free Secret Expert Course (step4of5)" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x02, "Yes (Off)" )
	PORT_DIPSETTING(    0x00, "No (On)" )
	PORT_DIPNAME( 0x01, 0x01, "Free Secret Expert Course (step5of5)" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x01, "No (Off)" )
	PORT_DIPSETTING(    0x00, "Yes (On)" )
	/* "Free Secret Expert Course" 1-1=Off 1-2=On 2-4=On 3-5=Off 3-6=On */
INPUT_PORTS_END

static INPUT_PORTS_START( bm4thmix )
	PORT_INCLUDE( beatmania_btn )
	BM2NDMIX_DSW1
	BM4THMIX_DSW2( "Basic / Hard Mode Difficulty", "Expert Mode Difficulty" )
	BM4THMIX_DSW3( "Basic / Hard Mode Stages" )
	/* "Free Secret Expert Course" 1-1=On 1-2=Off 2-1=On 3-5=On */
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x80, 0x80, "Free Secret Expert Course (step1of4)" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, "No (Off)" )
	PORT_DIPSETTING(    0x00, "Yes (On)" )
	PORT_DIPNAME( 0x40, 0x40, "Free Secret Expert Course (step2of4)" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "Yes (Off)" )
	PORT_DIPSETTING(    0x00, "No (On)" )
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x80, 0x80, "Score Display / Free Secret Expert Course (step3of4)" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x80, "On / No (Off)" )
	PORT_DIPSETTING(    0x00, "Off / Yes (On)" )
	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x02, 0x02, "Free Secret Expert Course (step4of4)" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x02, "No (Off)" )
	PORT_DIPSETTING(    0x00, "Yes (On)" )
	PORT_DIPNAME( 0x01, 0x01, "Score Display (if Free Secret Expert Course)" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	/* "Free Secret Expert Course" 1-1=On 1-2=Off 2-1=On 3-5=On */
INPUT_PORTS_END

static INPUT_PORTS_START( bm5thmix ) /* and bmcompm2 */
	PORT_INCLUDE( beatmania_btn )
	BM2NDMIX_DSW1
	BM4THMIX_DSW2( "Basic / Hard Mode Difficulty", "Expert Mode Difficulty" )
	BM4THMIX_DSW3( "Basic / Hard Mode Stages" )
INPUT_PORTS_END

static INPUT_PORTS_START( hmcompm2 )
	PORT_INCLUDE( beatmania_btn )
	BM2NDMIX_DSW1
	BM4THMIX_DSW2( "Basic / Hard Mode Difficulty", "Expert Mode Difficulty" )
	BM4THMIX_DSW3( "Basic / Hard Mode Stages" )
	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x02, 0x02, "Game Over Mode" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x02, "On Stage Middle" )
	PORT_DIPSETTING(    0x00, "On Stage Last" )
INPUT_PORTS_END

static INPUT_PORTS_START( bmclubmx )
	PORT_INCLUDE( beatmania_btn )
	BM2NDMIX_DSW1
	BM4THMIX_DSW2( "Normal / Maniac Mode Difficulty", "Expert Mode Difficulty" )
	BM4THMIX_DSW3( "Normal / Maniac Mode Stages" )
INPUT_PORTS_END

static INPUT_PORTS_START( bmdct )
	PORT_INCLUDE( beatmania_btn )
	BM2NDMIX_DSW1
	BM4THMIX_DSW2( "Basic / Hard Mode Difficulty", "Monkey Live Mode Difficulty" )
	BMDCT_DSW3( "Basic / Hard Mode Stages" )
INPUT_PORTS_END

static INPUT_PORTS_START( bmcorerm )
	PORT_INCLUDE( beatmania_btn )
	BM2NDMIX_DSW1
	BM4THMIX_DSW2( "Normal / Hard Mode Difficulty", "Expert Mode Difficulty" )
	BM4THMIX_DSW3( "Normal / Hard Mode Stages" )
INPUT_PORTS_END

static INPUT_PORTS_START( bm6thmix ) /* bm7thmix, and bmfinal */
	PORT_INCLUDE( beatmania_btn )
	BM2NDMIX_DSW1
	BM4THMIX_DSW2( "Normal / Free Mode Difficulty", "Expert Mode Difficulty" )
	BM6THMIX_DSW3( "Normal / Free Mode Stages" )
INPUT_PORTS_END


//--------- Pop'n Music

static INPUT_PORTS_START( popnmusic_btn )
	PORT_START("BTN1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1)
	PORT_START("BTN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Used by beatmania as P2 BUTTON 5 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Used by beatmania as P1 START */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Used by beatmania as P2 START */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Used by beatmania as EFFECT */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_START("BTN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F2)   /* TEST SW */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service")  /* SERVICE */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset")    /* RESET SW */
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("UNK1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("UNK2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	//PORT_START("TT1")     /* turn table 1P */
	//PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_PLAYER(1)
	//PORT_START("TT2")     /* turn table 2P */
	//PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

#define POPN_DSW1_COINAGE_OLD \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:8,7,6,5") \
	PORT_DIPSETTING(    0x01, "1P 5C / Continue 5C" ) \
	PORT_DIPSETTING(    0x02, "1P 5C / Continue 4C" ) \
	PORT_DIPSETTING(    0x03, "1P 5C / Continue 3C" ) \
	PORT_DIPSETTING(    0x04, "1P 5C / Continue 2C" ) \
	PORT_DIPSETTING(    0x05, "1P 5C / Continue 1C" ) \
	PORT_DIPSETTING(    0x06, "1P 4C / Continue 4C" ) \
	PORT_DIPSETTING(    0x07, "1P 4C / Continue 3C" ) \
	PORT_DIPSETTING(    0x08, "1P 4C / Continue 2C" ) \
	PORT_DIPSETTING(    0x09, "1P 4C / Continue 1C" ) \
	PORT_DIPSETTING(    0x0a, "1P 3C / Continue 3C" ) \
	PORT_DIPSETTING(    0x0b, "1P 3C / Continue 2C" ) \
	PORT_DIPSETTING(    0x0c, "1P 3C / Continue 1C" ) \
	PORT_DIPSETTING(    0x0d, "1P 2C / Continue 2C" ) \
	PORT_DIPSETTING(    0x0f, "1P 2C / Continue 1C" ) \
	PORT_DIPSETTING(    0x0e, "1P 1C / Continue 1C" ) \
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

#define POPN_DSW1_COINAGE_NEW \
	PORT_DIPNAME( 0x1f, 0x1f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:8,7,6,5,4") \
	PORT_DIPSETTING(    0x01, "1P 8C / Continue 8C" ) \
	PORT_DIPSETTING(    0x02, "1P 8C / Continue 7C" ) \
	PORT_DIPSETTING(    0x03, "1P 8C / Continue 6C" ) \
	PORT_DIPSETTING(    0x04, "1P 8C / Continue 5C" ) \
	PORT_DIPSETTING(    0x05, "1P 8C / Continue 4C" ) \
	PORT_DIPSETTING(    0x06, "1P 8C / Continue 3C" ) \
	PORT_DIPSETTING(    0x07, "1P 7C / Continue 7C" ) \
	PORT_DIPSETTING(    0x08, "1P 7C / Continue 6C" ) \
	PORT_DIPSETTING(    0x09, "1P 7C / Continue 5C" ) \
	PORT_DIPSETTING(    0x0a, "1P 7C / Continue 4C" ) \
	PORT_DIPSETTING(    0x0b, "1P 7C / Continue 3C" ) \
	PORT_DIPSETTING(    0x0c, "1P 6C / Continue 6C" ) \
	PORT_DIPSETTING(    0x0d, "1P 6C / Continue 5C" ) \
	PORT_DIPSETTING(    0x0e, "1P 6C / Continue 4C" ) \
	PORT_DIPSETTING(    0x0f, "1P 6C / Continue 3C" ) \
	PORT_DIPSETTING(    0x10, "1P 6C / Continue 2C" ) \
	PORT_DIPSETTING(    0x11, "1P 5C / Continue 5C" ) \
	PORT_DIPSETTING(    0x12, "1P 5C / Continue 4C" ) \
	PORT_DIPSETTING(    0x13, "1P 5C / Continue 3C" ) \
	PORT_DIPSETTING(    0x14, "1P 5C / Continue 2C" ) \
	PORT_DIPSETTING(    0x15, "1P 5C / Continue 1C" ) \
	PORT_DIPSETTING(    0x16, "1P 4C / Continue 4C" ) \
	PORT_DIPSETTING(    0x17, "1P 4C / Continue 3C" ) \
	PORT_DIPSETTING(    0x18, "1P 4C / Continue 2C" ) \
	PORT_DIPSETTING(    0x19, "1P 4C / Continue 1C" ) \
	PORT_DIPSETTING(    0x1a, "1P 3C / Continue 3C" ) \
	PORT_DIPSETTING(    0x1b, "1P 3C / Continue 2C" ) \
	PORT_DIPSETTING(    0x1c, "1P 3C / Continue 1C" ) \
	PORT_DIPSETTING(    0x1d, "1P 2C / Continue 2C" ) \
	PORT_DIPSETTING(    0x1f, "1P 2C / Continue 1C" ) \
	PORT_DIPSETTING(    0x1e, "1P 1C / Continue 1C" ) \
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

#define POPN_DSW1_JAMMINGGAUGE \
	PORT_DIPNAME( 0xc0, 0xc0, "Jamming Gauge Blocks" ) PORT_DIPLOCATION("SW1:2,1") \
	PORT_DIPSETTING(    0x80, "5" ) \
	PORT_DIPSETTING(    0xc0, "6" ) \
	PORT_DIPSETTING(    0x40, "7" ) \
	PORT_DIPSETTING(    0x00, "8" )

#define POPN_DSW2_GAUGEDECREMENT \
	PORT_DIPNAME( 0x0c, 0x0c, "Gauge Decrement Level" ) PORT_DIPLOCATION("SW2:6,5") \
	PORT_DIPSETTING(    0x04, "0" ) \
	PORT_DIPSETTING(    0x0c, "1" ) \
	PORT_DIPSETTING(    0x08, "2" ) \
	PORT_DIPSETTING(    0x00, "3" )

#define POPN_DSW2_GAUGEINCREMENT \
	PORT_DIPNAME( 0x03, 0x03, "Gauge Increment Level" ) PORT_DIPLOCATION("SW2:8,7") \
	PORT_DIPSETTING(    0x01, "0" ) \
	PORT_DIPSETTING(    0x03, "1" ) \
	PORT_DIPSETTING(    0x02, "2" ) \
	PORT_DIPSETTING(    0x00, "3" )

#define POPN1_DSW1 \
	PORT_START("DSW1") \
	POPN_DSW1_JAMMINGGAUGE \
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:3" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:4" ) \
	POPN_DSW1_COINAGE_OLD

#define POPN2_DSW1 \
	PORT_START("DSW1") \
	POPN_DSW1_JAMMINGGAUGE \
	PORT_DIPNAME( 0x20, 0x20, "Normal Mode Jamming" ) PORT_DIPLOCATION("SW1:3") \
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( On ) ) \
	POPN_DSW1_COINAGE_NEW

#define POPN1_DSW2 \
	PORT_START("DSW2") \
	BEATMANIA_DSW2_SCOREDISPLAY /* same as BEATMANIA */ \
	BEATMANIA_DSW2_DEMOSOUNDS /* same as BEATMANIA */ \
	PORT_DIPNAME( 0x10, 0x10, "Normal Mode Jamming" ) PORT_DIPLOCATION("SW2:4") \
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) \
	PORT_DIPSETTING(    0x10, DEF_STR( On ) ) \
	POPN_DSW2_GAUGEDECREMENT \
	POPN_DSW2_GAUGEINCREMENT

#define POPN2_DSW2 \
	PORT_START("DSW2") \
	BEATMANIA_DSW2_SCOREDISPLAY /* same as BEATMANIA */ \
	BEATMANIA_DSW2_DEMOSOUNDS /* same as BEATMANIA */ \
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:4" ) \
	POPN_DSW2_GAUGEDECREMENT \
	POPN_DSW2_GAUGEINCREMENT

#define POPN1_DSW3 \
	PORT_START("DSW3") \
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) /* bit 7,6 don't exist */ \
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:1" ) \
	PORT_DIPNAME( 0x10, 0x10, "All Song Mode [*A]" ) PORT_DIPLOCATION("SW3:2") \
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( On ) ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:3" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:4" ) \
	PORT_DIPNAME( 0x02, 0x02, "Enable \"RAVE\" (with *A=On) [*B]" ) PORT_DIPLOCATION("SW3:5") \
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x01, 0x01, "Enable \"Bonus Track\" (with *A=On and *B=On )" ) PORT_DIPLOCATION("SW3:6") \
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* "All Song Mode"      3-2=On */
	/* "Enable RAVE"        3-2=On 3-5=On */
	/* "Enable BONUS TRACK" 3-2=On 3-5=On 3-6=On */

#define POPN2_DSW3 \
	PORT_START("DSW3") \
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) /* bit 7,6 don't exist */ \
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:1" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:2" ) \
	PORT_DIPNAME( 0x08, 0x08, "All Song Mode" ) PORT_DIPLOCATION("SW3:3") \
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( On ) ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:4" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:5" ) \
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:6" )
	/* "All Song Mode" 3-3=On */

static INPUT_PORTS_START( popn1 )
	PORT_INCLUDE( popnmusic_btn )
	POPN1_DSW1
	POPN1_DSW2
	POPN1_DSW3
INPUT_PORTS_END

static INPUT_PORTS_START( popn2 )   /* popn2 and popn3 */
	PORT_INCLUDE( popnmusic_btn )
	POPN2_DSW1
	POPN2_DSW2
	POPN2_DSW3
INPUT_PORTS_END

//--------- Pop'n Stage

static INPUT_PORTS_START( popnstage )
	PORT_START("BTN1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1)
	PORT_START("BTN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) /* LEFT SELECTION */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 ) /* "OK" (MIDDLE) SELECTION */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 ) /* RIGHT SELECTION */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_START("BTN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F2)   /* TEST SW */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service")  /* SERVICE */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Used by beatmania as RESET SW */
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("UNK1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("UNK2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	//PORT_START("TT1")     /* turn table 1P */
	//PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_PLAYER(1)
	//PORT_START("TT2")     /* turn table 2P */
	//PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0xe0, 0xe0, "Coinage (6 Buttons)" ) PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(    0x20, "1P 4C / Continue 2C" )
	PORT_DIPSETTING(    0x40, "1P 3C / Continue 3C" )
	PORT_DIPSETTING(    0x60, "1P 3C / Continue 2C" )
	PORT_DIPSETTING(    0x80, "1P 3C / Continue 1C" )
	PORT_DIPSETTING(    0xa0, "1P 2C / Continue 2C" )
	PORT_DIPSETTING(    0xe0, "1P 2C / Continue 1C" )
	PORT_DIPSETTING(    0xc0, "1P 1C / Continue 1C" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1e, 0x1e, "Coinage (10 Buttons)" ) PORT_DIPLOCATION("SW1:7,6,5,4")
	PORT_DIPSETTING(    0x02, "1P 5C / Continue 5C" )
	PORT_DIPSETTING(    0x04, "1P 5C / Continue 4C" )
	PORT_DIPSETTING(    0x06, "1P 5C / Continue 3C" )
	PORT_DIPSETTING(    0x08, "1P 5C / Continue 2C" )
	PORT_DIPSETTING(    0x0c, "1P 5C / Continue 1C" )
	PORT_DIPSETTING(    0x0a, "1P 4C / Continue 4C" )
	PORT_DIPSETTING(    0x0e, "1P 4C / Continue 3C" )
	PORT_DIPSETTING(    0x10, "1P 4C / Continue 2C" )
	PORT_DIPSETTING(    0x12, "1P 4C / Continue 1C" )
	PORT_DIPSETTING(    0x14, "1P 3C / Continue 3C" )
	PORT_DIPSETTING(    0x1e, "1P 3C / Continue 2C" )
	PORT_DIPSETTING(    0x16, "1P 3C / Continue 1C" )
	PORT_DIPSETTING(    0x18, "1P 2C / Continue 2C" )
	PORT_DIPSETTING(    0x1a, "1P 2C / Continue 1C" )
	PORT_DIPSETTING(    0x1c, "1P 1C / Continue 1C" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0xc0, "Loud" )
	PORT_DIPSETTING(    0x80, DEF_STR ( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR ( Low ) )
	PORT_DIPSETTING(    0x00, "Silent" )
	PORT_DIPNAME( 0x30, 0x30, "Gauge Decrement Level" ) PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x20, "0" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x0c, 0x0c, "Gauge Increment Level" ) PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x08, "0" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x02, 0x02, "Score Display" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:8" )

	PORT_START("DSW3")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) /* bit 7,6 don't exist */
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:6" )
INPUT_PORTS_END


static INPUT_PORTS_START( popnstex )
	PORT_INCLUDE( popnstage )
	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x20, 0x20, "Enable Secret Mode (step1of3)" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Enable Secret Mode (step2of3)" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Enable Secret Mode (step3of3)" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* "Enable Secret Mode" 3-1=On 3-3=On 3-5=On */
INPUT_PORTS_END


/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16, 16, /* 16x16 characters */
	0x200000 / 128, /* 16384 characters */
	4,  /* bit planes */
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24,
		4+256, 0+256, 12+256, 8+256, 20+256, 16+256, 28+256, 24+256 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		0*32+512, 1*32+512, 2*32+512, 3*32+512, 4*32+512, 5*32+512, 6*32+512, 7*32+512 },
	16*16*4
};

static GFXDECODE_START( gfx_djmain )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0,  (0x4440/4)/16 )
GFXDECODE_END



/*************************************
 *
 *  Machine-specific init
 *
 *************************************/

void djmain_state::machine_start()
{
	ide_hdd_device *hdd = m_ata->subdevice<ata_slot_device>("0")->subdevice<ide_hdd_device>("hdd");
	if (m_ata_master_password != nullptr)
		hdd->set_master_password(m_ata_master_password);

	if (m_ata_user_password != nullptr)
		hdd->set_user_password(m_ata_user_password);

	m_leds.resolve();
	m_right_red_hlt.resolve();
	m_left_red_hlt.resolve();
	m_right_blue_hlt.resolve();
	m_left_blue_hlt.resolve();
	m_right_ssr.resolve();
	m_left_ssr.resolve();

	save_item(NAME(m_sndram_bank));
	save_item(NAME(m_pending_vb_int));
	save_item(NAME(m_v_ctrl));
	save_item(NAME(m_obj_regs));
}


void djmain_state::machine_reset()
{
	/* reset sound ram bank */
	m_sndram_bank = 0;

	/* reset LEDs */
	m_leds[0] = 1;
	m_leds[1] = 1;
	m_leds[2] = 1;
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void djmain_state::djmainj(machine_config &config)
{
	/* basic machine hardware */
	// popn3 works 9.6 MHz or slower in some songs */
	//M68EC020(config, m_maincpu, 18432000/2);    /*  9.216 MHz!? */
	M68EC020(config, m_maincpu, 32000000/4);   /*  8.000 MHz!? */
	m_maincpu->set_addrmap(AS_PROGRAM, &djmain_state::maincpu_djmainj);
	m_maincpu->set_vblank_int("screen", FUNC(djmain_state::vb_interrupt));

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, true);
	m_ata->irq_handler().set(FUNC(djmain_state::ide_interrupt));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(12, 512-12-1, 0, 384-1);
	screen.set_screen_update(FUNC(djmain_state::screen_update_djmain));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 0x4440 / 4);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_djmain);

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(djmain_state::tile_callback));
	m_k056832->set_config(K056832_BPP_4dj, 1, 1);
	m_k056832->set_palette(m_palette);

	K055555(config, m_k055555, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	k054539_device &k054539_1(K054539(config, "k054539_1", XTAL(18'432'000)));
	k054539_1.set_addrmap(0, &djmain_state::k054539_map);
	k054539_1.add_route(0, "lspeaker", 1.0);
	k054539_1.add_route(1, "rspeaker", 1.0);

	k054539_device &k054539_2(K054539(config, "k054539_2", XTAL(18'432'000)));
	k054539_2.set_addrmap(0, &djmain_state::k054539_map);
	k054539_2.add_route(0, "lspeaker", 1.0);
	k054539_2.add_route(1, "rspeaker", 1.0);
}

void djmain_state::djmainu(machine_config &config)
{
	djmainj(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &djmain_state::maincpu_djmainu);
}

void djmain_state::djmaina(machine_config &config)
{
	djmainj(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &djmain_state::maincpu_djmaina);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( bm1stmix )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "753jab01.6a", 0x000000, 0x80000, CRC(25bf8629) SHA1(2be73f9dd25cae415c6443f221cc7d38d5555ae5) )
	ROM_LOAD16_BYTE( "753jab02.8a", 0x000001, 0x80000, CRC(6ab951de) SHA1(a724ede03b74e9422c120fcc263e2ebcc3a3e110) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "753jaa03.19a", 0x000000, 0x80000, CRC(f2b2bce8) SHA1(61d31b111f35e7dde89965fa43ba627c12aff11c) )
	ROM_LOAD16_BYTE( "753jaa04.20a", 0x000001, 0x80000, CRC(85a18f9d) SHA1(ecd0ab4f53e882b00176dacad5fac35345fbea66) )
	ROM_LOAD16_BYTE( "753jaa05.22a", 0x100000, 0x80000, CRC(749b1e87) SHA1(1c771c19f152ae95171e4fd51da561ba4ec5ea87) )
	ROM_LOAD16_BYTE( "753jaa06.24a", 0x100001, 0x80000, CRC(6d86b0fd) SHA1(74a255dbb1c83131717ea1fe335f12aef81d9fcc) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "753jaa07.22d", 0x000000, 0x80000, CRC(f03ab5d8) SHA1(2ad902547908208714855aa0f2b7ed493452ee5f) )
	ROM_LOAD16_BYTE( "753jaa08.23d", 0x000001, 0x80000, CRC(6559f0c8) SHA1(0d6ec4bdc22c02cb9fb8de36b0a8f7a6c983440e) )
	ROM_LOAD16_BYTE( "753jaa09.25d", 0x100000, 0x80000, CRC(b50c3dbb) SHA1(6022ea249aad0793b2279699e68087b4bc9b4ef1) )
	ROM_LOAD16_BYTE( "753jaa10.27d", 0x100001, 0x80000, CRC(391f4bfd) SHA1(791c9889ea3ce639bbfb87934a1cad9aa3c9ccde) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "753jaa11", 0, SHA1(2e70cf31a853322f29f99b6f292c187a2cf33015) ) /* ver 1.00 JA */
	// There is an alternate image
	//DISK_IMAGE( "753jaa11", 0, MD5(260c9b72f4a03055e3abad61c6225324) SHA1(2cc3e149744516bf2353a2b47d33bc9d2072b6c4) ) /* ver 1.00 JA */
ROM_END

ROM_START( bm2ndmix )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "853jab01.6a", 0x000000, 0x80000, CRC(c8df72c0) SHA1(6793b587ba0611bc3da8c4955d6a87e47a19a223) )
	ROM_LOAD16_BYTE( "853jab02.8a", 0x000001, 0x80000, CRC(bf6ace08) SHA1(29d3fdf1c73a73a0a66fa5a4c4ac3f293cb82e37) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "853jaa03.19a", 0x000000, 0x80000, CRC(1462ed23) SHA1(fdfda3060c8d367ac2e8e43dedaba8ab9012cc77) )
	ROM_LOAD16_BYTE( "853jaa04.20a", 0x000001, 0x80000, CRC(98c9b331) SHA1(51f24b3c3773c53ff492ed9bad17c9867fd94e28) )
	ROM_LOAD16_BYTE( "853jaa05.22a", 0x100000, 0x80000, CRC(0da3fef9) SHA1(f9ef24144c00c054ecc4650bb79e74c57c6d6b3c) )
	ROM_LOAD16_BYTE( "853jaa06.24a", 0x100001, 0x80000, CRC(6a66978c) SHA1(460178a6f35e554a157742d77ed5ea6989fbcee1) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "853jaa07.22d", 0x000000, 0x80000, CRC(728c0010) SHA1(18888b402e0b7ccf63c7b3cb644673df1746dba7) )
	ROM_LOAD16_BYTE( "853jaa08.23d", 0x000001, 0x80000, CRC(926fc37c) SHA1(f251cba56ca201f0e748112462116cff218b66da) )
	ROM_LOAD16_BYTE( "853jaa09.25d", 0x100000, 0x80000, CRC(8584e21e) SHA1(3d1ca6de00f9ac07bbe7cd1e67093cca7bf484bb) )
	ROM_LOAD16_BYTE( "853jaa10.27d", 0x100001, 0x80000, CRC(9cb92d98) SHA1(6ace4492ba0b5a8f94a9e7b4f7126b31c6254637) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "853jaa11", 0, SHA1(9683ff8462491252b6eb2e5b3aa6496884c01506) ) /* ver 1.10 JA */
ROM_END

ROM_START( bm2ndmxa )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "853jaa01.6a", 0x000000, 0x80000, CRC(4f0bf5d0) SHA1(4793bb411e85f2191eb703a170c16cf163ea79e7) )
	ROM_LOAD16_BYTE( "853jaa02.8a", 0x000001, 0x80000, CRC(e323925b) SHA1(1f9f52a7ab6359b617e87f8b3d7ac4269885c621) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "853jaa03.19a", 0x000000, 0x80000, CRC(1462ed23) SHA1(fdfda3060c8d367ac2e8e43dedaba8ab9012cc77) )
	ROM_LOAD16_BYTE( "853jaa04.20a", 0x000001, 0x80000, CRC(98c9b331) SHA1(51f24b3c3773c53ff492ed9bad17c9867fd94e28) )
	ROM_LOAD16_BYTE( "853jaa05.22a", 0x100000, 0x80000, CRC(0da3fef9) SHA1(f9ef24144c00c054ecc4650bb79e74c57c6d6b3c) )
	ROM_LOAD16_BYTE( "853jaa06.24a", 0x100001, 0x80000, CRC(6a66978c) SHA1(460178a6f35e554a157742d77ed5ea6989fbcee1) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "853jaa07.22d", 0x000000, 0x80000, CRC(728c0010) SHA1(18888b402e0b7ccf63c7b3cb644673df1746dba7) )
	ROM_LOAD16_BYTE( "853jaa08.23d", 0x000001, 0x80000, CRC(926fc37c) SHA1(f251cba56ca201f0e748112462116cff218b66da) )
	ROM_LOAD16_BYTE( "853jaa09.25d", 0x100000, 0x80000, CRC(8584e21e) SHA1(3d1ca6de00f9ac07bbe7cd1e67093cca7bf484bb) )
	ROM_LOAD16_BYTE( "853jaa10.27d", 0x100001, 0x80000, CRC(9cb92d98) SHA1(6ace4492ba0b5a8f94a9e7b4f7126b31c6254637) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "853jaa11", 0, SHA1(9683ff8462491252b6eb2e5b3aa6496884c01506) ) /* ver 1.10 JA */
ROM_END

ROM_START( bm3rdmix )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "825jaa01.6a", 0x000000, 0x80000, CRC(cf7494a5) SHA1(994df0644817f44d135a16f04d8dae9ec73e3728) )
	ROM_LOAD16_BYTE( "825jaa02.8a", 0x000001, 0x80000, CRC(5f787fe2) SHA1(5944da21141802d96594cf77880682e97d014ca1) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "825jaa03.19a", 0x000000, 0x80000, CRC(ecd62652) SHA1(bceab4052dce2c843358f0a98aacc6e1124e3068) )
	ROM_LOAD16_BYTE( "825jaa04.20a", 0x000001, 0x80000, CRC(437a576f) SHA1(f30fd15d4f0d776e9b29ccfcd6e26861fb42e51a) )
	ROM_LOAD16_BYTE( "825jaa05.22a", 0x100000, 0x80000, CRC(9f9a3369) SHA1(d8b20127336af89b9e886289fb4f5a2e0db65f9b) )
	ROM_LOAD16_BYTE( "825jaa06.24a", 0x100001, 0x80000, CRC(e7a3991a) SHA1(6c8cb481e721428e1365f784e97bb6f6d421ed5a) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "825jaa07.22d", 0x000000, 0x80000, CRC(a96cf46c) SHA1(c8540b452dcb15f5873ca629fa62657a5a3bb02c) )
	ROM_LOAD16_BYTE( "825jaa08.23d", 0x000001, 0x80000, CRC(06d56c3b) SHA1(19cd15ab0869773e6a16b1cad48c53bec2f60b0b) )
	ROM_LOAD16_BYTE( "825jaa09.25d", 0x100000, 0x80000, CRC(d3e65669) SHA1(51abf452da60794fa47c05d11c08b203dde563ff) )
	ROM_LOAD16_BYTE( "825jaa10.27d", 0x100001, 0x80000, CRC(44d184f3) SHA1(28f3ec33a29164a6531f53db071272ccf015f66d) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "825jaa11", 0, SHA1(048919977232bbce046406a7212586cf39b77cf2) ) /* ver 1.00 JA */
ROM_END

ROM_START( bm3rdeaa )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "825eaa01.6a", 0x000000, 0x80000, CRC(a7303584) SHA1(222d1c469a72f17ea9316cfaf22ba965b24260d6) )
	ROM_LOAD16_BYTE( "825eaa02.8a", 0x000001, 0x80000, CRC(45e65086) SHA1(0daf53379d7c64b2819a0bdc192ee4ea72160643) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "825eaa03.19a", 0x000000, 0x80000, CRC(ecd62652) SHA1(bceab4052dce2c843358f0a98aacc6e1124e3068) )
	ROM_LOAD16_BYTE( "825eaa04.20a", 0x000001, 0x80000, CRC(437a576f) SHA1(f30fd15d4f0d776e9b29ccfcd6e26861fb42e51a) )
	ROM_LOAD16_BYTE( "825eaa05.22a", 0x100000, 0x80000, CRC(9f9a3369) SHA1(d8b20127336af89b9e886289fb4f5a2e0db65f9b) )
	ROM_LOAD16_BYTE( "825eaa06.24a", 0x100001, 0x80000, CRC(e7a3991a) SHA1(6c8cb481e721428e1365f784e97bb6f6d421ed5a) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "825eaa07.22d", 0x000000, 0x80000, CRC(3a2fe19e) SHA1(65c07a8471579cb20885a45af6a9f12c9f088625) )
	ROM_LOAD16_BYTE( "825eaa08.23d", 0x000001, 0x80000, CRC(98b16689) SHA1(e2bebf40d098365e6757b83ffee924f9d3995d12) )
	ROM_LOAD16_BYTE( "825eaa09.25d", 0x100000, 0x80000, CRC(829000ca) SHA1(f52494869cca78e7bc6125d329292eaa15fffed5) )
	ROM_LOAD16_BYTE( "825eaa10.27d", 0x100001, 0x80000, CRC(345ee6e5) SHA1(ce5d05b26900ed09fb3996a60edaf332e3acd720) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "825jab11", 0, SHA1(f506a83d43aeed87a7a32c3f7312d2a2b7d60d91) )  /* ver 1.01 JA */
ROM_END

ROM_START( bmcompmx )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "858jac01.6a", 0x000000, 0x80000, CRC(b32693ca) SHA1(6518a8acbd070bb6f9039c4f9997dda2720e8e16) )
	ROM_LOAD16_BYTE( "858jac02.8a", 0x000001, 0x80000, CRC(0d11d0f4) SHA1(b9fbf2ce062e1ee4a785abe021ac039857ced0d2) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "858jaa03.19a", 0x000000, 0x80000, CRC(8559f457) SHA1(133092994087864a6c29e9d51dcdbef2e2c2a123) )
	ROM_LOAD16_BYTE( "858jaa04.20a", 0x000001, 0x80000, CRC(770824d3) SHA1(5c21bc39f8128957d76be85bc178c96976987f5f) )
	ROM_LOAD16_BYTE( "858jaa05.22a", 0x100000, 0x80000, CRC(9ce769da) SHA1(1fe2999f786effdd5e3e74475e8431393eb9403d) )
	ROM_LOAD16_BYTE( "858jaa06.24a", 0x100001, 0x80000, CRC(0cde6584) SHA1(fb58d2b4f58144b71703431740c0381bb583f581) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "858jaa07.22d", 0x000000, 0x80000, CRC(7d183f46) SHA1(7a1b0ccb0407b787af709bdf038d886727199e4e) )
	ROM_LOAD16_BYTE( "858jaa08.23d", 0x000001, 0x80000, CRC(c731dc8f) SHA1(1a937d76c02711b7f73743c9999456d4408ad284) )
	ROM_LOAD16_BYTE( "858jaa09.25d", 0x100000, 0x80000, CRC(0b4ad843) SHA1(c01e15053dd1975dc68db9f4e6da47062d8f9b54) )
	ROM_LOAD16_BYTE( "858jaa10.27d", 0x100001, 0x80000, CRC(00b124ee) SHA1(435d28a327c2707833a8ddfe841104df65ffa3f8) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "858jaa11", 0, SHA1(bc590472046336a1000f29901fe3fd7b29747e47) ) /* ver 1.00 JA */
ROM_END

ROM_START( bmcompmxb )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "858jab01.6a", 0x000000, 0x80000, CRC(92841eb5) SHA1(3a9d90a9c4b16cb7118aed2cadd3ab32919efa96) )
	ROM_LOAD16_BYTE( "858jab02.8a", 0x000001, 0x80000, CRC(7b19969c) SHA1(3545acabbf53bacc5afa72a3c5af3cd648bc2ae1) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "858jaa03.19a", 0x000000, 0x80000, CRC(8559f457) SHA1(133092994087864a6c29e9d51dcdbef2e2c2a123) )
	ROM_LOAD16_BYTE( "858jaa04.20a", 0x000001, 0x80000, CRC(770824d3) SHA1(5c21bc39f8128957d76be85bc178c96976987f5f) )
	ROM_LOAD16_BYTE( "858jaa05.22a", 0x100000, 0x80000, CRC(9ce769da) SHA1(1fe2999f786effdd5e3e74475e8431393eb9403d) )
	ROM_LOAD16_BYTE( "858jaa06.24a", 0x100001, 0x80000, CRC(0cde6584) SHA1(fb58d2b4f58144b71703431740c0381bb583f581) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "858jaa07.22d", 0x000000, 0x80000, CRC(7d183f46) SHA1(7a1b0ccb0407b787af709bdf038d886727199e4e) )
	ROM_LOAD16_BYTE( "858jaa08.23d", 0x000001, 0x80000, CRC(c731dc8f) SHA1(1a937d76c02711b7f73743c9999456d4408ad284) )
	ROM_LOAD16_BYTE( "858jaa09.25d", 0x100000, 0x80000, CRC(0b4ad843) SHA1(c01e15053dd1975dc68db9f4e6da47062d8f9b54) )
	ROM_LOAD16_BYTE( "858jaa10.27d", 0x100001, 0x80000, CRC(00b124ee) SHA1(435d28a327c2707833a8ddfe841104df65ffa3f8) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "858jaa11", 0, SHA1(bc590472046336a1000f29901fe3fd7b29747e47) ) /* ver 1.00 JA */
ROM_END

ROM_START( hmcompmx )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "858uab01.6a", 0x000000, 0x80000, CRC(f9c16675) SHA1(f2b50a3544f43af6fd987256a8bd4125b95749ef) )
	ROM_LOAD16_BYTE( "858uab02.8a", 0x000001, 0x80000, CRC(4e8f1e78) SHA1(88d654de4377b584ff8a5e1f8bc81ffb293ec8a5) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "858uaa03.19a", 0x000000, 0x80000, CRC(52b51a5e) SHA1(9f01e2fcbe5a9d7f80b377c5e10f18da2c9dcc8e) )
	ROM_LOAD16_BYTE( "858uaa04.20a", 0x000001, 0x80000, CRC(a336cee9) SHA1(0e62c0c38d86868c909b4c1790fbb7ecb2de137d) )
	ROM_LOAD16_BYTE( "858uaa05.22a", 0x100000, 0x80000, CRC(2e14cf83) SHA1(799b2162f7b11678d1d260f7e1eb841abda55a60) )
	ROM_LOAD16_BYTE( "858uaa06.24a", 0x100001, 0x80000, CRC(2be07788) SHA1(5cc2408f907ca6156efdcbb2c10a30e9b81797f8) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "858uaa07.22d", 0x000000, 0x80000, CRC(9d7c8ea0) SHA1(5ef773ade7ab12a5dc10484e8b7711c9d76fe2a1) )
	ROM_LOAD16_BYTE( "858uaa08.23d", 0x000001, 0x80000, CRC(f21c3f45) SHA1(1d7ff2c4161605b382d07900142093192aa93a48) )
	ROM_LOAD16_BYTE( "858uaa09.25d", 0x100000, 0x80000, CRC(99519886) SHA1(664f6bd953201a6e2fc123cb8b3facf72766107d) )
	ROM_LOAD16_BYTE( "858uaa10.27d", 0x100001, 0x80000, CRC(20aa7145) SHA1(eeff87eb9a9864985d751f45e843ee6e73db8cfd) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "858jaa11", 0, SHA1(bc590472046336a1000f29901fe3fd7b29747e47) ) /* ver 1.00 JA */
ROM_END

ROM_START( bmcmxaac )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "858aac01.6a", 0x000000, 0x80000, CRC(1563b021) SHA1(5b63c3f0db4704ef92ee388777611ba8d5f6a1b7) )
	ROM_LOAD16_BYTE( "858aac02.8a", 0x000001, 0x80000, CRC(53ae5d66) SHA1(cfbf870cf9c2fa8270d62423a21f3a5c19382a66) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "858aac03.19a", 0x000000, 0x80000, CRC(8559f457) SHA1(133092994087864a6c29e9d51dcdbef2e2c2a123) )
	ROM_LOAD16_BYTE( "858aac04.20a", 0x000001, 0x80000, CRC(770824d3) SHA1(5c21bc39f8128957d76be85bc178c96976987f5f) )
	ROM_LOAD16_BYTE( "858aac05.22a", 0x100000, 0x80000, CRC(2e14cf83) SHA1(799b2162f7b11678d1d260f7e1eb841abda55a60) )
	ROM_LOAD16_BYTE( "858aac06.24a", 0x100001, 0x80000, CRC(2be07788) SHA1(5cc2408f907ca6156efdcbb2c10a30e9b81797f8) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "858aac07.22d", 0x000000, 0x80000, CRC(2a925ec8) SHA1(9ffb657e0723dd4b0b6c50e67c8f89007d59fa04) )
	ROM_LOAD16_BYTE( "858aac08.23d", 0x000001, 0x80000, CRC(1f11304b) SHA1(ae48e93f694cec075b785b79f692a37d66d5a78b) )
	ROM_LOAD16_BYTE( "858aac09.25d", 0x100000, 0x80000, CRC(46b7c6e2) SHA1(913de33f10bd86f261567f634eece2f157d0c9b0) )
	ROM_LOAD16_BYTE( "858aac10.27d", 0x100001, 0x80000, CRC(15d1230f) SHA1(848df167e9e20c1d1e9f3bf79bcc4fc1d2121944) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "858aac11", 0, SHA1(31cb039c20783f399885f3d9a7582593dae0ed40) ) /* ver 1.00 AA */
ROM_END

ROM_START( bscompmx )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "858kab01.6a", 0x000000, 0x80000, CRC(47c19dcc) SHA1(3698c9d5ccaa24626d5a04b20750dc3faf423f68) )
	ROM_LOAD16_BYTE( "858kab02.8a", 0x000001, 0x80000, CRC(cc848aaa) SHA1(1ebe4f8d3936dbcd0a83dadc3547951fcab39786) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "858kab03.19a", 0x000000, 0x80000, CRC(679022e9) SHA1(70026a5c5bcb8adb58e7e7246f3cc30471a844a6) )
	ROM_LOAD16_BYTE( "858kab04.20a", 0x000001, 0x80000, CRC(d916d8ec) SHA1(827fcd2c3c5e0d01e4cf49820d23eef44b63cfaa) )
	ROM_LOAD16_BYTE( "858kab05.22a", 0x100000, 0x80000, CRC(f59c8e83) SHA1(fe4fee34ce12afc02e709190ec3a756a0cf77e08) )
	ROM_LOAD16_BYTE( "858kab06.24a", 0x100001, 0x80000, CRC(8de99a50) SHA1(bde3fae982bd0a3492135b20542d87a87dd772ce) )

	ROM_REGION( 0x200000, "k056832", 0 )    /* TILEMAP */
	ROM_LOAD16_BYTE( "858kab07.22d", 0x000000, 0x80000, CRC(3bd6d429) SHA1(bc63eb4931d528aad6199fa4374f8a7d3cc50094) )
	ROM_LOAD16_BYTE( "858kab08.23d", 0x000001, 0x80000, CRC(3a16c0bb) SHA1(77c417cab5b71dcfce4225471eb73e26e1adb1bb) )
	ROM_LOAD16_BYTE( "858kab09.25d", 0x100000, 0x80000, CRC(f4a2fab7) SHA1(b7814a2bb04cb11969295dba97b2755daddbaab6) )
	ROM_LOAD16_BYTE( "858kab10.27d", 0x100001, 0x80000, CRC(177777fb) SHA1(9d556d7d65a588bad300dbd282c6b5a31629bd6d) )

	DISK_REGION( "ata:0:hdd" )              /* IDE HARD DRIVE */
	DISK_IMAGE( "858kab11", 0, SHA1(951376b895782ac57844b25d13a4984d37cc9fc8) ) /* ver 1.00 KA */
ROM_END

ROM_START( bm4thmix )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "847jaa01.6a", 0x000000, 0x80000, CRC(81138a1b) SHA1(ebe211126f871e541881e1670f56d50b058dead3) )
	ROM_LOAD16_BYTE( "847jaa02.8a", 0x000001, 0x80000, CRC(4eeb0010) SHA1(942303dfb19a4a78dd74ad24576031760553a661) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "847jaa03.19a", 0x000000, 0x80000, CRC(f447d140) SHA1(cc15b80419940d127a77765508f877421ed86ee2) )
	ROM_LOAD16_BYTE( "847jaa04.20a", 0x000001, 0x80000, CRC(edc3e286) SHA1(341b1dc6ee1562b1ddf235a66ac96b94c482b67c) )
	ROM_LOAD16_BYTE( "847jaa05.22a", 0x100000, 0x80000, CRC(da165b5e) SHA1(e46110590e6ab89b55f6abfbf6c53c99d28a75a9) )
	ROM_LOAD16_BYTE( "847jaa06.24a", 0x100001, 0x80000, CRC(8bfc2f28) SHA1(f8869867945d63d9f34b6228d95c5a61b193eed2) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "847jab07.22d", 0x000000, 0x80000, CRC(c159e7c4) SHA1(96af0c29b2f1fef494b2223179862d16f26bb33f) )
	ROM_LOAD16_BYTE( "847jab08.23d", 0x000001, 0x80000, CRC(8ff084d6) SHA1(50cff8c701e33f2630925c1a9ae4351076912acd) )
	ROM_LOAD16_BYTE( "847jab09.25d", 0x100000, 0x80000, CRC(2e4ac9fe) SHA1(bbd4c6e0c82fc0be88f851e901e5853b6bcf775f) )
	ROM_LOAD16_BYTE( "847jab10.27d", 0x100001, 0x80000, CRC(c78516f5) SHA1(1adf5805c808dc55de14a9a9b20c3d2cf7bf414d) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "847jaa11", 0, SHA1(8cad631531b5616d6a4b0a99d988f4b525932dc7) ) /* ver 1.00 JA */
ROM_END

ROM_START( bs4thmix )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "847kaa01.6a", 0x000000, 0x80000, CRC(17c994e5) SHA1(2249d9e788029d194454dc0552246262d4131e8c) )
	ROM_LOAD16_BYTE( "847kaa02.8a", 0x000001, 0x80000, CRC(25b2a690) SHA1(90216cc7fbbaa8709eec348a7dcc5e25c7638b34) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "847kaa03.19a", 0x000000, 0x80000, CRC(f447d140) SHA1(cc15b80419940d127a77765508f877421ed86ee2) )
	ROM_LOAD16_BYTE( "847kaa04.20a", 0x000001, 0x80000, CRC(edc3e286) SHA1(341b1dc6ee1562b1ddf235a66ac96b94c482b67c) )
	ROM_LOAD16_BYTE( "847kaa05.22a", 0x100000, 0x80000, CRC(da165b5e) SHA1(e46110590e6ab89b55f6abfbf6c53c99d28a75a9) )
	ROM_LOAD16_BYTE( "847kaa06.24a", 0x100001, 0x80000, CRC(8bfc2f28) SHA1(f8869867945d63d9f34b6228d95c5a61b193eed2) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "847kaa07.22d", 0x000000, 0x80000, CRC(0528276a) SHA1(ab4f2cdd2938a04f7da3e85f3cec9ca66c85b78a) )
	ROM_LOAD16_BYTE( "847kaa08.23d", 0x000001, 0x80000, CRC(3c659505) SHA1(ffa81d2f3823076a16422b49ac0ecfb0db376d54) )
	ROM_LOAD16_BYTE( "847kaa09.25d", 0x100000, 0x80000, CRC(c078f7d3) SHA1(2c268f1b7f1fa71c659d899a49e839128b789245) )
	ROM_LOAD16_BYTE( "847kaa10.27d", 0x100001, 0x80000, CRC(2f676be7) SHA1(43d1844280117e76c95bb9b32ea3ca511fffc131) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "847kaa01", 0, SHA1(be35c25d11892b57817ca9da90734a439d259824) )
ROM_END

ROM_START( bm5thmix )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "981jaa01.6a", 0x000000, 0x80000, CRC(03bbe7e3) SHA1(7d4ec3bc7719a3f1b81df309b5c74afaffde42ba) )
	ROM_LOAD16_BYTE( "981jaa02.8a", 0x000001, 0x80000, CRC(f4e59923) SHA1(a4983435e3f2243ea9ccc2fd5439d86c30b6f604) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "981jaa03.19a", 0x000000, 0x80000, CRC(8b7e6d72) SHA1(d470377e20e4d4935af5e57d081ce24dd9ea5793) )
	ROM_LOAD16_BYTE( "981jaa04.20a", 0x000001, 0x80000, CRC(5139988a) SHA1(2b1eb97dcbfbe6bba1352a02cf0036e9a721ab39) )
	ROM_LOAD16_BYTE( "981jaa05.22a", 0x100000, 0x80000, CRC(f370fdb9) SHA1(3a2bbdda984f2630e8ae505a8db259d9162e07a3) )
	ROM_LOAD16_BYTE( "981jaa06.24a", 0x100001, 0x80000, CRC(da6e3813) SHA1(9163bd2cfb0a32798e797c7b4eea21e28772a206) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "981jaa07.22d", 0x000000, 0x80000, CRC(f6c72998) SHA1(e78af5b515b224c534f47abd6477dd97dc521b0d) )
	ROM_LOAD16_BYTE( "981jaa08.23d", 0x000001, 0x80000, CRC(aa4ff682) SHA1(3750e1e81b7c1a4fb419076171f20e4c36b1c544) )
	ROM_LOAD16_BYTE( "981jaa09.25d", 0x100000, 0x80000, CRC(d96d4e1c) SHA1(379aa4e82cd06490645f54dab1724c827108735d) )
	ROM_LOAD16_BYTE( "981jaa10.27d", 0x100001, 0x80000, CRC(06bee0e4) SHA1(6eea8614cb01e7079393b9976b6fd6a52c14e3c0) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "981jaa11", 0, SHA1(dc7353fa436d96ae174a58d3a38ca9928a63727f) ) /* ver 1.00 JA */
ROM_END

ROM_START( bmclubmx )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "993jaa01.6a", 0x000000, 0x80000, CRC(b314af94) SHA1(6448554e1d565ee1558d13f484b5fa0018ac3667) )
	ROM_LOAD16_BYTE( "993jaa02.8a", 0x000001, 0x80000, CRC(0aa9f16a) SHA1(508d41e141997ba07443c4ab98454cec515d731c) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "993jaa03.19a", 0x000000, 0x80000, CRC(00394778) SHA1(3631a42ed0c8ee572e7faafdaacce9fc2b372d25) )
	ROM_LOAD16_BYTE( "993jaa04.20a", 0x000001, 0x80000, CRC(2522f3b0) SHA1(1ab8618b732f1402fc7bfb141630873d4c706d34) )
	ROM_LOAD16_BYTE( "993jaa05.22a", 0x100000, 0x80000, CRC(4e340947) SHA1(a0a7f3b222a292b07bc5c7acd61547ea2bdbad43) )
	ROM_LOAD16_BYTE( "993jaa06.24a", 0x100001, 0x80000, CRC(c0a711d6) SHA1(ab581c5215c4db6dbf58b47f54834fe81e8a569b) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "993jaa07.22d", 0x000000, 0x80000, CRC(4fc588cf) SHA1(00fb73002b6b5ae414eef320169e379b94ee33a1) )
	ROM_LOAD16_BYTE( "993jaa08.23d", 0x000001, 0x80000, CRC(b6c88e9e) SHA1(e3b76e782b9507dad2bdb9de1a34d125f6100cc8) )
	ROM_LOAD16_BYTE( "993jaa09.25d", 0x100000, 0x80000, CRC(e1a172dd) SHA1(42e850c055dc5bfccf6b6989f9f3a945fce13006) )
	ROM_LOAD16_BYTE( "993jaa10.27d", 0x100001, 0x80000, CRC(9d113a2d) SHA1(eee94a5f7015c49aa630b8df0c8e9d137d238811) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "993hdda01", 0, SHA1(f5d4df1dd27ce6ee2d0897852342691d55b63bfb) )
	// this image has not been verified
	//  DISK_IMAGE( "993jaa11", 0, MD5(e26eb62d7cf3357585f5066da6063143) )  /* ver 1.00 JA */
ROM_END

ROM_START( bmcompm2 )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "988jaa01.6a", 0x000000, 0x80000, CRC(31be1d4c) SHA1(ab8c2b4a2b48e3b2b549022f65afb206ab125680) )
	ROM_LOAD16_BYTE( "988jaa02.8a", 0x000001, 0x80000, CRC(0413de32) SHA1(f819e8756e2000de5df61ad42ac01de14b7330f9) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "988jaa03.19a", 0x000000, 0x80000, CRC(c0ad86d4) SHA1(6aca5bf3fbc0bd69116e442053840660eeff0239) )
	ROM_LOAD16_BYTE( "988jaa04.20a", 0x000001, 0x80000, CRC(84801a50) SHA1(8700e4fb56941b87f8333e72e2a1c7ac9e322312) )
	ROM_LOAD16_BYTE( "988jaa05.22a", 0x100000, 0x80000, CRC(0ddf7d6d) SHA1(aa110ab64c2fbf427796dff3a817b57cf6a9440d) )
	ROM_LOAD16_BYTE( "988jaa06.24a", 0x100001, 0x80000, CRC(2a87f69e) SHA1(fe84bb50864467a83d06d34a18123ab11fb55781) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "988jaa07.22d", 0x000000, 0x80000, CRC(9e57fe24) SHA1(40bd0428227e46ebe365f2f6821b08182a0ce698) )
	ROM_LOAD16_BYTE( "988jaa08.23d", 0x000001, 0x80000, CRC(bf604ca4) SHA1(6abc81d5d9084fcf59f70a6bd57e1b36041a1072) )
	ROM_LOAD16_BYTE( "988jaa09.25d", 0x100000, 0x80000, CRC(8f3bae7f) SHA1(c4dac14f6c7f75a2b19153e05bfe969e9eb4aca0) )
	ROM_LOAD16_BYTE( "988jaa10.27d", 0x100001, 0x80000, CRC(248bf0ee) SHA1(d89205ed57e771401bfc2c24043d200ecbd0b7fc) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "988jaa11", 0, SHA1(12a0988c631dd3331e54b8417a9659402afe168b) ) /* ver 1.00 JA */
ROM_END

ROM_START( hmcompm2 )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "988uaa01.6a", 0x000000, 0x80000, CRC(5e5cc6c0) SHA1(0e7cd601d4543715cbc9f65e6fd48837179c962a) )
	ROM_LOAD16_BYTE( "988uaa02.8a", 0x000001, 0x80000, CRC(e262984a) SHA1(f47662e40f91f2addb1a4b649923c1d0ee017341) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "988uaa03.19a", 0x000000, 0x80000, CRC(d0f204c8) SHA1(866baac5a6d301d5b9cf0c14e9937ee5f435db77) )
	ROM_LOAD16_BYTE( "988uaa04.20a", 0x000001, 0x80000, CRC(74c6b3ed) SHA1(7d9b064bab3f29fc6435f6430c71208abbf9d861) )
	ROM_LOAD16_BYTE( "988uaa05.22a", 0x100000, 0x80000, CRC(6b9321cb) SHA1(449e5f85288a8c6724658050fa9521c7454a1e46) )
	ROM_LOAD16_BYTE( "988uaa06.24a", 0x100001, 0x80000, CRC(da6e0c1e) SHA1(4ef37db6c872bccff8c27fc53cccc0b269c7aee4) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "988uaa07.22d", 0x000000, 0x80000, CRC(9217870d) SHA1(d0536a8a929c41b49cdd053205165bfb8150e0c5) )
	ROM_LOAD16_BYTE( "988uaa08.23d", 0x000001, 0x80000, CRC(77777e59) SHA1(33b5508b961a04b82c9967a3326af6bbd838b85e) )
	ROM_LOAD16_BYTE( "988uaa09.25d", 0x100000, 0x80000, CRC(c2ad6810) SHA1(706388c5acf6718297fd90e10f8a673463a0893b) )
	ROM_LOAD16_BYTE( "988uaa10.27d", 0x100001, 0x80000, CRC(dab0f3c9) SHA1(6fd899e753e32f60262c54ab8553c686c7ef28de) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "988jaa11", 0, SHA1(12a0988c631dd3331e54b8417a9659402afe168b) ) /* ver 1.00 JA */
ROM_END

ROM_START( bmdct )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "995jaa01.6a", 0x000000, 0x80000, CRC(2c224169) SHA1(0608469fa0a15026f461be5141ed29bf740144ca) )
	ROM_LOAD16_BYTE( "995jaa02.8a", 0x000001, 0x80000, CRC(a2edb472) SHA1(795e44e56dfee6c5eceb28172bc20ba5b31c366b) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "995jaa03.19a", 0x000000, 0x80000, CRC(77a7030c) SHA1(8f7988ca5c248d0846ec22c0975ae008d85e8d72) )
	ROM_LOAD16_BYTE( "995jaa04.20a", 0x000001, 0x80000, CRC(a12ea45d) SHA1(9bd48bc25c17f885d74e859de153ec49012a4e39) )
	ROM_LOAD16_BYTE( "995jaa05.22a", 0x100000, 0x80000, CRC(1493fd98) SHA1(4cae2ebccc79b21d7e21b984dc6fe10ab3013a2d) )
	ROM_LOAD16_BYTE( "995jaa06.24a", 0x100001, 0x80000, CRC(86bff0bb) SHA1(658280f78987eaee31b60a7826db6df105601f0a) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "995jaa07.22d", 0x000000, 0x80000, CRC(ce030edf) SHA1(1e2594a6a04559d70b09750bb665d8cd3d0288ea) )
	ROM_LOAD16_BYTE( "995jaa08.23d", 0x000001, 0x80000, CRC(375d3d17) SHA1(180cb5ad4497b3745aa9317764f237b30a678b31) )
	ROM_LOAD16_BYTE( "995jaa09.25d", 0x100000, 0x80000, CRC(1510a9c2) SHA1(daf1ab26b7b6b0fe0123b3fbee68684157c2ce51) )
	ROM_LOAD16_BYTE( "995jaa10.27d", 0x100001, 0x80000, CRC(f9e4e9f2) SHA1(fe91badf6b0baeea690d75399d8c66fabcf6d352) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "995jaa11", 0, SHA1(8fec3c4d97f64f48b9867230a97cda4347496075) ) /* ver 1.00 JA */
ROM_END

ROM_START( bmcorerm )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "a05jaa01.6a", 0x000000, 0x80000, CRC(cd6f1fc5) SHA1(237cbc17a693efb6bffffd6afb24f0944c29330c) )
	ROM_LOAD16_BYTE( "a05jaa02.8a", 0x000001, 0x80000, CRC(fe07785e) SHA1(14c652008cb509b5206fb515aad7dfe36a6fe6f4) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "a05jaa03.19a", 0x000000, 0x80000, CRC(8b88932a) SHA1(df20f8323adb02d07b835da98f4a29b3142175c9) )
	ROM_LOAD16_BYTE( "a05jaa04.20a", 0x000001, 0x80000, CRC(cc72629f) SHA1(f95d06f409c7d6422d66a55c0452eb3feafc6ef0) )
	ROM_LOAD16_BYTE( "a05jaa05.22a", 0x100000, 0x80000, CRC(e241b22b) SHA1(941a76f6ac821e0984057ec7df7862b12fa657b8) )
	ROM_LOAD16_BYTE( "a05jaa06.24a", 0x100001, 0x80000, CRC(77eb08a3) SHA1(fd339aaec06916abfc928e850e33480707b5450d) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "a05jaa07.22d", 0x000000, 0x80000, CRC(4d79646d) SHA1(5f1237bbd3cb09b27babf1c5359ef6c0d80ae3a9) )
	ROM_LOAD16_BYTE( "a05jaa08.23d", 0x000001, 0x80000, CRC(f067494f) SHA1(ef031b5501556c1aa047a51604a44551b35a8b99) )
	ROM_LOAD16_BYTE( "a05jaa09.25d", 0x100000, 0x80000, CRC(1504d62c) SHA1(3c31c6625bc089235a96fe21021239f2d0c0f6e1) )
	ROM_LOAD16_BYTE( "a05jaa10.27d", 0x100001, 0x80000, CRC(99d75c36) SHA1(9599420863aa0a9492d3caeb03f8ac5fd4c3cdb2) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "a05jaa11", 0, SHA1(7ebc41cc3e9a0a922b49201b34e29201522eb726) ) /* ver 1.00 JA */
ROM_END

ROM_START( bm6thmix )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "a21jaa01.6a", 0x000000, 0x80000, CRC(6d7ccbe3) SHA1(633c69c14dfd70866664b94095fa5f21087428d8) )
	ROM_LOAD16_BYTE( "a21jaa02.8a", 0x000001, 0x80000, CRC(f10076fa) SHA1(ab9f3e75a36fdaccec411afd77f588f040db139d) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "a21jaa03.19a", 0x000000, 0x80000, CRC(ca806266) SHA1(6b5f9d5089a992347745ab6af4dadaac4e3b0742) )
	ROM_LOAD16_BYTE( "a21jaa04.20a", 0x000001, 0x80000, CRC(71124e79) SHA1(d9fd8f662ac9c29daf25acd310fd0f27051dea0b) )
	ROM_LOAD16_BYTE( "a21jaa05.22a", 0x100000, 0x80000, CRC(818e34e6) SHA1(8a9093b92392a065d0cf94d56195a6f3ca611044) )
	ROM_LOAD16_BYTE( "a21jaa06.24a", 0x100001, 0x80000, CRC(36f2043b) SHA1(d2846cc10173662029da7c5d686cf89299be2be5) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "a21jaa07.22d", 0x000000, 0x80000, CRC(841d83e1) SHA1(c85962abcc955e8f11138e03002b16afd3791f0a) )
	ROM_LOAD16_BYTE( "a21jaa08.23d", 0x000001, 0x80000, CRC(4e561919) SHA1(4b91560d9ba367c848d784db760f042d5d76e003) )
	ROM_LOAD16_BYTE( "a21jaa09.25d", 0x100000, 0x80000, CRC(181e6f70) SHA1(82c7ca3068ace9a66b614ead4b90ea6fe4017d51) )
	ROM_LOAD16_BYTE( "a21jaa10.27d", 0x100001, 0x80000, CRC(1ac33595) SHA1(3173bb8dc420487c4d427e779444a98aad37d51e) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "a21jaa11", 0, SHA1(ed0a07212a360e75934fc22c56265842cf0829b6) ) /* ver 1.00 JA */
ROM_END

ROM_START( bm7thmix )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "b07jab01.6a", 0x000000, 0x80000, CRC(433d0074) SHA1(5a9709ce200cbff340063469956d1c55a46810d9) )
	ROM_LOAD16_BYTE( "b07jab02.8a", 0x000001, 0x80000, CRC(794773af) SHA1(c823deb077f6515d7701de84d324c3d367719819) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "b07jaa03.19a", 0x000000, 0x80000, CRC(3e30af3f) SHA1(f092c4156bc7d0a0309171fd1e00a6d4c33cb08f) )
	ROM_LOAD16_BYTE( "b07jaa04.20a", 0x000001, 0x80000, CRC(190a4a83) SHA1(f7ae2d3ccd98f99fdae61c1a2145f993c4064ebd) )
	ROM_LOAD16_BYTE( "b07jaa05.22a", 0x100000, 0x80000, CRC(415a6363) SHA1(b3edbcd293006c3738a10680ecfa66e105028786) )
	ROM_LOAD16_BYTE( "b07jaa06.24a", 0x100001, 0x80000, CRC(46c59a43) SHA1(ba58432bf7df394b5c633e63bcf2321bc320f023) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "b07jaa07.22d", 0x000000, 0x80000, CRC(b2908dc7) SHA1(22e36afef9a03681928d37a8ffe50078d04525ce) )
	ROM_LOAD16_BYTE( "b07jaa08.23d", 0x000001, 0x80000, CRC(cbbefecf) SHA1(ed1347d1a8fd59677e4290b8cd568ddf505a7265) )
	ROM_LOAD16_BYTE( "b07jaa09.25d", 0x100000, 0x80000, CRC(2530cedb) SHA1(94b38b4fe198b26a2ff4d99d2cb28a0f935fe940) )
	ROM_LOAD16_BYTE( "b07jaa10.27d", 0x100001, 0x80000, CRC(6b75ba9c) SHA1(aee922adc3bc0296ae6e08e461b20a9e5e72a2df) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "b07jaa11", 0, SHA1(e4925494f0a801abb4d3aa6524c379eb445d8dff) ) /* ver 1.00 JA */
	// this image has not been verified
	//DISK_IMAGE( "b07jab11", 0, MD5(0e9440787ca69567792095085e2a3619) )    /* ver 1.00 JA */
ROM_END

ROM_START( bmfinal )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "c01jaa01.6a", 0x000000, 0x80000, CRC(a64eeff7) SHA1(377eee1f41e3072f9154a7c17ec4c4f3fb63ea4a) )
	ROM_LOAD16_BYTE( "c01jaa02.8a", 0x000001, 0x80000, CRC(599bdac5) SHA1(f85aff020c92fcd3c2a42036615226b54e5bee98) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "c01jaa03.19a", 0x000000, 0x80000, CRC(1c9c6eb7) SHA1(bd1a9d8ed78095328817f599f52d9d34e09e9275) )
	ROM_LOAD16_BYTE( "c01jaa04.20a", 0x000001, 0x80000, CRC(4e5aa665) SHA1(22f3888a29497ff0a801cce620ca0373268e5cd9) )
	ROM_LOAD16_BYTE( "c01jaa05.22a", 0x100000, 0x80000, CRC(37dab217) SHA1(66b07c36e7749a4c9d9dfaca633958a4922c4562) )
	ROM_LOAD16_BYTE( "c01jaa06.24a", 0x100001, 0x80000, CRC(d35c6818) SHA1(ce608603ea3662f8cda5cf958a676d64a0f74645) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "c01jaa07.22d", 0x000000, 0x80000, CRC(3e70f506) SHA1(d3cd0b48383bf2514b7f47fade8549ea8e3c5555) )
	ROM_LOAD16_BYTE( "c01jaa08.23d", 0x000001, 0x80000, CRC(535e6065) SHA1(131f7eec4179145781bbd23474202f4eaf9cefd0) )
	ROM_LOAD16_BYTE( "c01jaa09.25d", 0x100000, 0x80000, CRC(45cf93b1) SHA1(7c5082bcd1fe15761a0a965e25dda121904ff1bd) )
	ROM_LOAD16_BYTE( "c01jaa10.27d", 0x100001, 0x80000, CRC(c9927749) SHA1(c2644877bda483e241381265e723ea8ab8357761) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "c01jaa11", 0, SHA1(0a53c4412a72a886f5fb98c12c529d056d625244) ) /* ver 1.00 JA */
	// this image has not been verified
	//DISK_IMAGE( "c01jaa11", 0, MD5(8bb7e6b6bc63cac8a4f2997307c25748) )    /* ver 1.00 JA */
ROM_END

ROM_START( popn1 )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "803t_a01.6a", 0x000000, 0x80000, CRC(e85032bc) SHA1(625ac755a77a861ecede766d77fbecb29683844b) )
	ROM_LOAD16_BYTE( "803j_a02.8a", 0x000001, 0x80000, CRC(17e90be0) SHA1(8857d46d63b7cd97aed5c7a5cf804fe5311255ac) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "803t_a03.19a", 0x000000, 0x80000, CRC(a0970b32) SHA1(4bf06f977f67f13d0994e80021faaefed015ca1c) )
	ROM_LOAD16_BYTE( "803t_a04.20a", 0x000001, 0x80000, CRC(a158b950) SHA1(535d994a68fe56c8a93d460e5272b1a0b081383d) )
	ROM_LOAD16_BYTE( "803t_a05.22a", 0x100000, 0x80000, CRC(8a23c18d) SHA1(714b68a3112dff52feba6a50ae15c02d041b1def) )
	ROM_LOAD16_BYTE( "803t_a06.24a", 0x100001, 0x80000, CRC(bd6450ff) SHA1(bb89c65d1b0e38c0a26ec4cccd35103583efff45) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "803t_a07.22d", 0x000000, 0x80000, CRC(4c97dbf0) SHA1(c33f84e2c1a59294413163a5546cb2fe092e42d1) )
	ROM_LOAD16_BYTE( "803t_a08.23d", 0x000001, 0x80000, CRC(cc6b21d3) SHA1(eac3b92bc392265df08b0311320c4e23f257c58b) )
	ROM_LOAD16_BYTE( "803t_a09.25d", 0x100000, 0x80000, CRC(45ab3978) SHA1(c34b800dfb14affcb705dffa566d046a6d1e7c7c) )
	ROM_LOAD16_BYTE( "803t_a10.27d", 0x100001, 0x80000, CRC(a487ad02) SHA1(ca9edb611ddf2f1e7d42b049ce867b173fce3634) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "803_ta_hdd", 0, SHA1(be833a3c0f1e5953271fa7d8d6e81211fbe9710f) ) /* GQ803AA  pop'n music ver1.01  1998/10/14 (C) KONAMI */
ROM_END

ROM_START( popn1a )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "803aaa01.6a", 0x000000, 0x80000, CRC(70466eff) SHA1(d59f97a6882ad65afa3b5de491d44bb4d839d286) )
	ROM_LOAD16_BYTE( "803aaa02.8a", 0x000001, 0x80000, CRC(c102c909) SHA1(630a74ccec78d36adaba38a85ed6a0e45dca96a4) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "803aaa03.19a", 0x000000, 0x80000, CRC(a0970b32) SHA1(4bf06f977f67f13d0994e80021faaefed015ca1c) )
	ROM_LOAD16_BYTE( "803aaa04.20a", 0x000001, 0x80000, CRC(a158b950) SHA1(535d994a68fe56c8a93d460e5272b1a0b081383d) )
	ROM_LOAD16_BYTE( "803aaa05.22a", 0x100000, 0x80000, CRC(8a23c18d) SHA1(714b68a3112dff52feba6a50ae15c02d041b1def) )
	ROM_LOAD16_BYTE( "803aaa06.24a", 0x100001, 0x80000, CRC(bd6450ff) SHA1(bb89c65d1b0e38c0a26ec4cccd35103583efff45) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "803aaa07.22d", 0x000000, 0x80000, CRC(4c97dbf0) SHA1(c33f84e2c1a59294413163a5546cb2fe092e42d1) )
	ROM_LOAD16_BYTE( "803aaa08.23d", 0x000001, 0x80000, CRC(cc6b21d3) SHA1(eac3b92bc392265df08b0311320c4e23f257c58b) )
	ROM_LOAD16_BYTE( "803aaa09.25d", 0x100000, 0x80000, CRC(45ab3978) SHA1(c34b800dfb14affcb705dffa566d046a6d1e7c7c) )
	ROM_LOAD16_BYTE( "803aaa10.27d", 0x100001, 0x80000, CRC(a487ad02) SHA1(ca9edb611ddf2f1e7d42b049ce867b173fce3634) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "803jaa11", 0, SHA1(8684c1a1f0de9496bac0333c0302e35359173d8a) ) /* GQ803 pop'n music ver1.00  1998/09/04 (C) KONAMI */
ROM_END

ROM_START( popn1k )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "803k a01.bin", 0x000000, 0x80000, CRC(5037e700) SHA1(b1e4d0a69187b22706d7a744da63bf639aee9738) )
	ROM_LOAD16_BYTE( "803k a02.bin", 0x000001, 0x80000, CRC(14aff1c4) SHA1(0832c1eea7fb593245fba23bf7dc4b4495ff0cdc) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "803aaa03.19a", 0x000000, 0x80000, CRC(a0970b32) SHA1(4bf06f977f67f13d0994e80021faaefed015ca1c) )
	ROM_LOAD16_BYTE( "803aaa04.20a", 0x000001, 0x80000, CRC(a158b950) SHA1(535d994a68fe56c8a93d460e5272b1a0b081383d) )
	ROM_LOAD16_BYTE( "803aaa05.22a", 0x100000, 0x80000, CRC(8a23c18d) SHA1(714b68a3112dff52feba6a50ae15c02d041b1def) )
	ROM_LOAD16_BYTE( "803aaa06.24a", 0x100001, 0x80000, CRC(bd6450ff) SHA1(bb89c65d1b0e38c0a26ec4cccd35103583efff45) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "803aaa07.22d", 0x000000, 0x80000, CRC(4c97dbf0) SHA1(c33f84e2c1a59294413163a5546cb2fe092e42d1) )
	ROM_LOAD16_BYTE( "803aaa08.23d", 0x000001, 0x80000, CRC(cc6b21d3) SHA1(eac3b92bc392265df08b0311320c4e23f257c58b) )
	ROM_LOAD16_BYTE( "803aaa09.25d", 0x100000, 0x80000, CRC(45ab3978) SHA1(c34b800dfb14affcb705dffa566d046a6d1e7c7c) )
	ROM_LOAD16_BYTE( "803aaa10.27d", 0x100001, 0x80000, CRC(a487ad02) SHA1(ca9edb611ddf2f1e7d42b049ce867b173fce3634) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "803kaa11", 0, SHA1(be833a3c0f1e5953271fa7d8d6e81211fbe9710f) ) /* GQ803AA  pop'n music ver1.01  1998/10/14 (C) KONAMI */
ROM_END

ROM_START( popn1j )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "803jaa01.6a", 0x000000, 0x80000, CRC(469cee89) SHA1(d7c3e25e48492bceb17825db357830b08a20f09a) )
	ROM_LOAD16_BYTE( "803jaa02.8a", 0x000001, 0x80000, CRC(112ff5a3) SHA1(74d7155a1b63d411a8c3f99e511fc4c331b4c62f) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "803jaa03.19a", 0x000000, 0x80000, CRC(d80315f6) SHA1(070ea8d00aeecce1e357be5a9c434ef46f57a7e9) )
	ROM_LOAD16_BYTE( "803jaa04.20a", 0x000001, 0x80000, CRC(f7b9ac82) SHA1(898fbe229a3fdea5988d46359d030c3ec35eaafd) )
	ROM_LOAD16_BYTE( "803jaa05.22a", 0x100000, 0x80000, CRC(2902f6df) SHA1(658ccae9a67196a310bd69870c350058d2911feb) )
	ROM_LOAD16_BYTE( "803jaa06.24a", 0x100001, 0x80000, CRC(508f326a) SHA1(a55c17f88b5856a754f00a6e32b6f60685a88bec) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "803jaa07.22d", 0x000000, 0x80000, CRC(b9c12071) SHA1(8f67965d5c8e7c9bfac528a77a9e7c8e0d8b17c8) )
	ROM_LOAD16_BYTE( "803jaa08.23d", 0x000001, 0x80000, CRC(a263f819) SHA1(b479a215282212e9253e4085640c0638a4036e31) )
	ROM_LOAD16_BYTE( "803jaa09.25d", 0x100000, 0x80000, CRC(204d53eb) SHA1(349de147246b0ed08fb7e473d63e073b71fa30c9) )
	ROM_LOAD16_BYTE( "803jaa10.27d", 0x100001, 0x80000, CRC(535a61a3) SHA1(b24c57601a7e3a349473af69114703133a46806d) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "803jaa11", 0, SHA1(8684c1a1f0de9496bac0333c0302e35359173d8a) ) /* GQ803 pop'n music ver1.00  1998/09/04 (C) KONAMI */
ROM_END

ROM_START( popn2 )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "831jaa01.8a", 0x000000, 0x80000, CRC(d6214cac) SHA1(18e74c81710228c91ab9eb554b63d9bd69b93ec8) )
	ROM_LOAD16_BYTE( "831jaa02.6a", 0x000001, 0x80000, CRC(aabe8689) SHA1(d51d277e9b5d0233d1c6bdfec40c32587f84b31a) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "831jaa03.19a", 0x000000, 0x80000, CRC(a07aeb72) SHA1(4d957c15d1b989e955249c34b0aa5679fb3e4fbf) )
	ROM_LOAD16_BYTE( "831jaa04.20a", 0x000001, 0x80000, CRC(9277d1d2) SHA1(6946845973f0ce15db383032343f6852873698eb) )
	ROM_LOAD16_BYTE( "831jaa05.22a", 0x100000, 0x80000, CRC(f3b63033) SHA1(c3c6de0d8c749ddf4926040637f03b11c2a21b99) )
	ROM_LOAD16_BYTE( "831jaa06.24a", 0x100001, 0x80000, CRC(43564e9c) SHA1(54b792b8aaf22876f9eb806e31b86af4b354bcf6) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "831jaa07.22d", 0x000000, 0x80000, CRC(25af75f5) SHA1(c150514a3bc6f3f88a5b98ef0db5440e2c5fec2d) )
	ROM_LOAD16_BYTE( "831jaa08.23d", 0x000001, 0x80000, CRC(3b1b5629) SHA1(95b6bed5c5218a3bfb10996cd9af31bd7e08c1c4) )
	ROM_LOAD16_BYTE( "831jaa09.25d", 0x100000, 0x80000, CRC(ae7838d2) SHA1(4f8a6793065c6c1eb08161f65b1d6246987bf47e) )
	ROM_LOAD16_BYTE( "831jaa10.27d", 0x100001, 0x80000, CRC(85173cb6) SHA1(bc4d86bf4654a9a0a58e624f77090854950f3993) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "831jhdda01", 0, SHA1(ef62d5fcc1a36235fc932e6ecef71dc845d1d72d) ) /* GQ831JA  pop'n music ver1.00  1999/2/28 (C) KONAMI */
ROM_END

ROM_START( popn3 )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "980a01.6a",    0x000000, 0x080000, CRC(ffd37d2c) SHA1(2a62ccfdb77a10356dbf08d6daa84faa3ff5d93a) )
	ROM_LOAD16_BYTE( "980a02.8a",    0x000001, 0x080000, CRC(00b15e1b) SHA1(7725b244b2964952e52a266aff697a8632830c97) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "980a03.19a",   0x000000, 0x080000, CRC(3674ba5b) SHA1(8741a43b099936c5f8add33d487b511c1ee8d21b) )
	ROM_LOAD16_BYTE( "980a04.20a",   0x000001, 0x080000, CRC(32e8ca33) SHA1(5aab1cb334e57667e146516125574f4f14676104) )
	ROM_LOAD16_BYTE( "980a05.22a",   0x100000, 0x080000, CRC(d31072e4) SHA1(c23c0e21fb22fe82b9a76d28bf2896dfec6bdc9b) )
	ROM_LOAD16_BYTE( "980a06.24a",   0x100001, 0x080000, CRC(d2bbcf36) SHA1(4f44c5d8df5dabf2956bdf33739a97b0645b5a5d) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "980a07.22d",   0x000000, 0x080000, CRC(770732d0) SHA1(f4330952d1e54658077e315ebd3cfd35e267219c) )
	ROM_LOAD16_BYTE( "980a08.23d",   0x000001, 0x080000, CRC(64ba3895) SHA1(3e4654c970d6fffe46b4e1097c1a6cda196ec92a) )
	ROM_LOAD16_BYTE( "980a09.25d",   0x100000, 0x080000, CRC(1cb4d84e) SHA1(9669585c6a2825aeae6e47dd03458624b4c44721) )
	ROM_LOAD16_BYTE( "980a10.27d",   0x100001, 0x080000, CRC(7776b87e) SHA1(662b7cd7cb4fb8f8bab240ef543bf9a593e23a03) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "980hdda01", 0, SHA1(57ece2b63c32d194d4e41b4e4a45aaab30150fd4) ) /* GQ980JA  pop'n music 3 ver. 1.00  1999/09/02 (C) KONAMI */
ROM_END

ROM_START( popnstex )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "970jba01.6a", 0x000000, 0x80000, CRC(8fa0c957) SHA1(12d1d6f15e19955c663ebdfcb16d5f6d209c0f76) )
	ROM_LOAD16_BYTE( "970jba02.8a", 0x000001, 0x80000, CRC(7adb00a0) SHA1(70a86897ab6cbc3f34be51f7f078644de697e331) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "970jba03.19a", 0x000000, 0x80000, CRC(e5d15d3c) SHA1(bdbd3c59e3377e071b199eea6cfb2ad84d37e971) )
	ROM_LOAD16_BYTE( "970jba04.20a", 0x000001, 0x80000, CRC(687f9beb) SHA1(6baac0aa2db3af9e34469b1719ccff3643fd85f7) )
	ROM_LOAD16_BYTE( "970jba05.22a", 0x100000, 0x80000, CRC(3bedc09c) SHA1(d0806bb54a3e620a987d61c6a5f04a2e1fc613a8) )
	ROM_LOAD16_BYTE( "970jba06.24a", 0x100001, 0x80000, CRC(1673a771) SHA1(2768434f1c94543f69d40165e68d325ae5d553cd) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "970jba07.22d", 0x000000, 0x80000, CRC(6fd06bdb) SHA1(1dc621923e0871d2d5171753f5ddb97786ab12bd) )
	ROM_LOAD16_BYTE( "970jba08.23d", 0x000001, 0x80000, CRC(28256891) SHA1(2069f52d596acbf355f205bb8d69cefc4cce3542) )
	ROM_LOAD16_BYTE( "970jba09.25d", 0x100000, 0x80000, CRC(5d2bda52) SHA1(d03c135ac04437b54e4d267ae168fe7ebb9e5b65) )
	ROM_LOAD16_BYTE( "970jba10.27d", 0x100001, 0x80000, CRC(edc4a245) SHA1(30bbd7bf0299a064119c535abb9be69d725aa130) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "970jba11", 0, SHA1(72e92b73b22a8f35e9faca93211e5acf781e66bb) )  /* GQ970  pop'n stage 1.5 ver1.00  1999/10/15 (C) KONAMI */
ROM_END


#if 0
// for reference, these sets have not been verified
ROM_START( bm3rdmxb )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* MC68EC020FG25 MPU */
	ROM_LOAD16_BYTE( "825jab01.6a", 0x000000, 0x80000, CRC(934fdcb2) SHA1(b88bada065b5464c579039c2e403c061e6eeb356) )
	ROM_LOAD16_BYTE( "825jab02.8a", 0x000001, 0x80000, CRC(6012c488) SHA1(df32db41942c2fe2b2aa7439900372e22ea54c3c) )

	ROM_REGION( 0x200000, "gfx1", 0)        /* SPRITE */
	ROM_LOAD16_BYTE( "825jaa03.19a", 0x000000, 0x80000, CRC(ecd62652) SHA1(bceab4052dce2c843358f0a98aacc6e1124e3068) )
	ROM_LOAD16_BYTE( "825jaa04.20a", 0x000001, 0x80000, CRC(437a576f) SHA1(f30fd15d4f0d776e9b29ccfcd6e26861fb42e51a) )
	ROM_LOAD16_BYTE( "825jaa05.22a", 0x100000, 0x80000, CRC(9f9a3369) SHA1(d8b20127336af89b9e886289fb4f5a2e0db65f9b) )
	ROM_LOAD16_BYTE( "825jaa06.24a", 0x100001, 0x80000, CRC(e7a3991a) SHA1(6c8cb481e721428e1365f784e97bb6f6d421ed5a) )

	ROM_REGION( 0x200000, "k056832", 0 )       /* TILEMAP */
	ROM_LOAD16_BYTE( "825jab07.22d", 0x000000, 0x80000, CRC(1a515c82) SHA1(a0c908d449aa45cb3a90a42c97429f10873e884b) )
	ROM_LOAD16_BYTE( "825jab08.23d", 0x000001, 0x80000, CRC(82731b07) SHA1(c0d391fcd94c6b2225fca338c0c5db5d35e2d8bc) )
	ROM_LOAD16_BYTE( "825jab09.25d", 0x100000, 0x80000, CRC(1407ba5d) SHA1(e7a0d190326589f4d94e83cb7c85dd4e91f4efad) )
	ROM_LOAD16_BYTE( "825jab10.27d", 0x100001, 0x80000, CRC(2afd0a10) SHA1(1b8b868ac5720bb1b376f4eb8952efb190257bda) )

	DISK_REGION( "ata:0:hdd" )            /* IDE HARD DRIVE */
	DISK_IMAGE( "825jab11", 0, MD5(f4360da10a932ba90e93469df7426d1d) SHA1(1) )  /* ver 1.01 JA */
ROM_END



#endif

/*************************************
 *
 *  Driver-specific init
 *
 *************************************/

void djmain_state::init_beatmania()
{
	m_ata_master_password = nullptr;
	m_ata_user_password = nullptr;
}

static const uint8_t beatmania_master_password[2 + 32] =
{
	0x01, 0x00,
	0x4d, 0x47, 0x43, 0x28, 0x4b, 0x29, 0x4e, 0x4f,
	0x4d, 0x41, 0x20, 0x49, 0x4c, 0x41, 0x20, 0x4c,
	0x49, 0x52, 0x48, 0x47, 0x53, 0x54, 0x52, 0x20,
	0x53, 0x45, 0x52, 0x45, 0x45, 0x56, 0x2e, 0x44
};

void djmain_state::init_hmcompmx()
{
	static const uint8_t hmcompmx_user_password[2 + 32] =
	{
		0x00, 0x00,
		0x44, 0x42, 0x56, 0x4b, 0x3a, 0x34, 0x38, 0x2a,
		0x5a, 0x4d, 0x78, 0x3e, 0x74, 0x61, 0x6c, 0x0a,
		0x7a, 0x63, 0x19, 0x77, 0x73, 0x7d, 0x0d, 0x12,
		0x6b, 0x09, 0x02, 0x0f, 0x05, 0x00, 0x7d, 0x1b
	};

	init_beatmania();

	m_ata_master_password = beatmania_master_password;
	m_ata_user_password = hmcompmx_user_password;
}

void djmain_state::init_bscompmx()
{
	static const uint8_t bscompmx_user_password[2 + 32] =
	{
		0x00, 0x00,
		0x44, 0x42, 0x56, 0x4b, 0x3a, 0x34, 0x38, 0x2a,
		0x5a, 0x53, 0x78, 0x3e, 0x74, 0x61, 0x6c, 0x0a,
		0x7a, 0x63, 0x19, 0x77, 0x73, 0x7d, 0x0d, 0x12,
		0x6b, 0x09, 0x02, 0x0f, 0x05, 0x00, 0x7d, 0x1b
	};

	init_beatmania();

	m_ata_master_password = beatmania_master_password;
	m_ata_user_password = bscompmx_user_password;
}

void djmain_state::init_bm4thmix()
{
	static const uint8_t bm4thmix_user_password[2 + 32] =
	{
		0x00, 0x00,
		0x44, 0x42, 0x29, 0x4b, 0x2f, 0x2c, 0x4c, 0x32,
		0x48, 0x5d, 0x0c, 0x3e, 0x62, 0x6f, 0x7e, 0x73,
		0x67, 0x10, 0x19, 0x79, 0x6c, 0x7d, 0x00, 0x01,
		0x18, 0x06, 0x1e, 0x07, 0x77, 0x1a, 0x7d, 0x77
	};

	init_beatmania();

	m_ata_user_password = bm4thmix_user_password;
}

void djmain_state::init_bs4thmix()
{
	static const uint8_t bs4thmix_user_password[2 + 32] =
	{
		0x00, 0x00,
		0x44, 0x42, 0x56, 0x4b, 0x4e, 0x47, 0x4c, 0x32,
		0x48, 0x5d, 0x0c, 0x3e, 0x62, 0x6f, 0x7e, 0x73,
		0x67, 0x10, 0x19, 0x79, 0x6c, 0x7d, 0x00, 0x01,
		0x18, 0x06, 0x1e, 0x07, 0x77, 0x1a, 0x7d, 0x77
	};

	init_beatmania();

	m_ata_master_password = beatmania_master_password;
	m_ata_user_password = bs4thmix_user_password;
}

void djmain_state::init_bm5thmix()
{
	static const uint8_t bm5thmix_user_password[2 + 32] =
	{
		0x00, 0x00,
		0x44, 0x42, 0x56, 0x4b, 0x37, 0x35, 0x4a, 0x23,
		0x5a, 0x52, 0x0c, 0x3e, 0x60, 0x04, 0x6c, 0x78,
		0x77, 0x7e, 0x74, 0x16, 0x6c, 0x7d, 0x00, 0x16,
		0x6b, 0x1a, 0x1e, 0x06, 0x04, 0x01, 0x7d, 0x1f
	};

	init_beatmania();

	m_ata_master_password = beatmania_master_password;
	m_ata_user_password = bm5thmix_user_password;
}

void djmain_state::init_bmclubmx()
{
	static const uint8_t bmclubmx_user_password[2 + 32] =
	{
		0x00, 0x00,
		0x44, 0x20, 0x30, 0x57, 0x3c, 0x3f, 0x38, 0x32,
		0x4f, 0x38, 0x74, 0x4c, 0x07, 0x61, 0x6c, 0x64,
		0x76, 0x7d, 0x70, 0x16, 0x1f, 0x6f, 0x0c, 0x0f,
		0x0a, 0x1a, 0x71, 0x07, 0x1e, 0x19, 0x7d, 0x02
	};

	init_beatmania();

	m_ata_master_password = beatmania_master_password;
	m_ata_user_password = bmclubmx_user_password;
}


void djmain_state::init_bmcompm2()
{
	static const uint8_t bmcompm2_user_password[2 + 32] =
	{
		0x00, 0x00,
		0x3a, 0x20, 0x31, 0x3e, 0x46, 0x2c, 0x35, 0x46,
		0x48, 0x51, 0x6f, 0x3e, 0x73, 0x6b, 0x68, 0x0a,
		0x60, 0x71, 0x19, 0x6f, 0x70, 0x68, 0x07, 0x62,
		0x6b, 0x0d, 0x71, 0x0f, 0x1d, 0x10, 0x7d, 0x7a
	};

	init_beatmania();

	m_ata_master_password = beatmania_master_password;
	m_ata_user_password = bmcompm2_user_password;
}

void djmain_state::init_hmcompm2()
{
	static const uint8_t hmcompm2_user_password[2 + 32] =
	{
		0x00, 0x00,
		0x3b, 0x39, 0x24, 0x3e, 0x4e, 0x59, 0x5c, 0x32,
		0x3b, 0x4c, 0x72, 0x57, 0x69, 0x04, 0x79, 0x65,
		0x76, 0x10, 0x6a, 0x77, 0x1f, 0x65, 0x0a, 0x16,
		0x09, 0x68, 0x71, 0x0b, 0x77, 0x15, 0x17, 0x1e
	};

	init_beatmania();

	m_ata_master_password = beatmania_master_password;
	m_ata_user_password = hmcompm2_user_password;
}

void djmain_state::init_bmdct()
{
	static const uint8_t bmdct_user_password[2 + 32] =
	{
		0x00, 0x00,
		0x52, 0x47, 0x30, 0x3f, 0x2f, 0x39, 0x54, 0x5e,
		0x4f, 0x4b, 0x65, 0x3e, 0x07, 0x6e, 0x6c, 0x67,
		0x7d, 0x79, 0x7b, 0x16, 0x6d, 0x73, 0x65, 0x06,
		0x0e, 0x0a, 0x05, 0x0f, 0x13, 0x74, 0x09, 0x19
	};

	init_beatmania();

	m_ata_master_password = beatmania_master_password;
	m_ata_user_password = bmdct_user_password;
}

void djmain_state::init_bmcorerm()
{
	static const uint8_t bmcorerm_user_password[2 + 32] =
	{
		0x00, 0x00,
		0x44, 0x42, 0x56, 0x4b, 0x3f, 0x4d, 0x4a, 0x27,
		0x5a, 0x52, 0x0c, 0x3e, 0x6a, 0x04, 0x63, 0x6f,
		0x72, 0x64, 0x72, 0x7f, 0x1f, 0x73, 0x17, 0x04,
		0x05, 0x09, 0x14, 0x0d, 0x7a, 0x74, 0x7d, 0x7a
	};

	init_beatmania();

	m_ata_master_password = beatmania_master_password;
	m_ata_user_password = bmcorerm_user_password;
}

void djmain_state::init_bm6thmix()
{
	static const uint8_t bm6thmix_user_password[2 + 32] =
	{
		0x00, 0x00,
		0x44, 0x42, 0x56, 0x4b, 0x3d, 0x4d, 0x4a, 0x23,
		0x5a, 0x52, 0x0c, 0x3e, 0x6a, 0x04, 0x63, 0x65,
		0x7e, 0x7f, 0x77, 0x77, 0x1f, 0x79, 0x04, 0x0f,
		0x02, 0x06, 0x09, 0x0f, 0x7a, 0x74, 0x7d, 0x7a
	};

	init_beatmania();

	m_ata_master_password = beatmania_master_password;
	m_ata_user_password = bm6thmix_user_password;
}

void djmain_state::init_bm7thmix()
{
	static const uint8_t bm7thmix_user_password[2 + 32] =
	{
		0x00, 0x00,
		0x44, 0x42, 0x56, 0x4b, 0x3f, 0x4e, 0x4a, 0x25,
		0x5a, 0x52, 0x0c, 0x3e, 0x15, 0x04, 0x6f, 0x0a,
		0x77, 0x71, 0x74, 0x16, 0x6d, 0x73, 0x0c, 0x0c,
		0x0c, 0x06, 0x7c, 0x6e, 0x77, 0x74, 0x7d, 0x7a
	};

	init_beatmania();

	m_ata_master_password = beatmania_master_password;
	m_ata_user_password = bm7thmix_user_password;
}

void djmain_state::init_bmfinal()
{
	static const uint8_t bmfinal_user_password[2 + 32] =
	{
		0x00, 0x00,
		0x44, 0x42, 0x56, 0x4b, 0x3f, 0x4f, 0x4a, 0x23,
		0x5a, 0x52, 0x0c, 0x3e, 0x07, 0x04, 0x63, 0x7f,
		0x76, 0x74, 0x6a, 0x64, 0x7e, 0x68, 0x0c, 0x0c,
		0x0c, 0x06, 0x71, 0x6e, 0x77, 0x79, 0x7d, 0x7a
	};

	init_beatmania();

	m_ata_master_password = beatmania_master_password;
	m_ata_user_password = bmfinal_user_password;
}

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

// commented out games should also run on this driver

GAME( 1997, bm1stmix, 0,        djmainj, bm1stmix,  djmain_state, init_beatmania, ROT0, "Konami", "beatmania (ver JA-B)", 0 )
GAME( 1998, bm2ndmix, 0,        djmainj, bm2ndmix,  djmain_state, init_beatmania, ROT0, "Konami", "beatmania 2nd MIX (ver JA-B)", 0 )
GAME( 1998, bm2ndmxa, bm2ndmix, djmainj, bm2ndmix,  djmain_state, init_beatmania, ROT0, "Konami", "beatmania 2nd MIX (ver JA-A)", 0 )
GAME( 1998, bm3rdmix, 0,        djmainj, bm3rdmix,  djmain_state, init_beatmania, ROT0, "Konami", "beatmania 3rd MIX (ver JA-A)", 0 )
GAME( 1998, bm3rdeaa, bm3rdmix, djmainu, bm3rdmix,  djmain_state, init_beatmania, ROT0, "Konami", "beatmania 3rd MIX (ver EA-A)", 0 )
GAME( 1999, bmcompmx, 0,        djmainj, bmcompmx,  djmain_state, init_beatmania, ROT0, "Konami", "beatmania complete MIX (ver JA-C)", 0 )
GAME( 1999, bmcompmxb,bmcompmx, djmainj, bmcompmx,  djmain_state, init_beatmania, ROT0, "Konami", "beatmania complete MIX (ver JA-B)", 0 )
GAME( 1999, bmcmxaac, bmcompmx, djmainu, bmcompmx,  djmain_state, init_beatmania, ROT0, "Konami", "beatmania complete MIX (ver AA-C)", 0 )
GAME( 1999, hmcompmx, bmcompmx, djmainu, bmcompmx,  djmain_state, init_hmcompmx,  ROT0, "Konami", "hiphopmania complete MIX (ver UA-B)", 0 )
GAME( 1999, bscompmx, bmcompmx, djmainu, bmcompmx,  djmain_state, init_bscompmx,  ROT0, "Konami", "beatstage complete MIX (ver KA-B)", 0 )
GAME( 1999, bm4thmix, 0,        djmainj, bm4thmix,  djmain_state, init_bm4thmix,  ROT0, "Konami", "beatmania 4th MIX (ver JA-A)", 0 )
GAME( 1999, bs4thmix, bm4thmix, djmainu, bm4thmix,  djmain_state, init_bs4thmix,  ROT0, "Konami", "beatstage 4th MIX (ver KA-A)", 0 )
GAME( 1999, bm5thmix, 0,        djmainj, bm5thmix,  djmain_state, init_bm5thmix,  ROT0, "Konami", "beatmania 5th MIX (ver JA-A)", 0 )
GAME( 2000, bmcompm2, 0,        djmainj, bm5thmix,  djmain_state, init_bmcompm2,  ROT0, "Konami", "beatmania complete MIX 2 (ver JA-A)", 0 )
GAME( 2000, hmcompm2, bmcompm2, djmainu, hmcompm2,  djmain_state, init_hmcompm2,  ROT0, "Konami", "hiphopmania complete MIX 2 (ver UA-A)", 0 )
GAME( 2000, bmclubmx, 0,        djmainj, bmclubmx,  djmain_state, init_bmclubmx,  ROT0, "Konami", "beatmania Club MIX (ver JA-A)", 0 )
GAME( 2000, bmdct,    0,        djmainj, bmdct,     djmain_state, init_bmdct,     ROT0, "Konami", "beatmania featuring Dreams Come True (ver JA-A)", 0 )
GAME( 2000, bmcorerm, 0,        djmainj, bmcorerm,  djmain_state, init_bmcorerm,  ROT0, "Konami", "beatmania CORE REMIX (ver JA-A)", 0 )
GAME( 2001, bm6thmix, 0,        djmainj, bm6thmix,  djmain_state, init_bm6thmix,  ROT0, "Konami", "beatmania 6th MIX (ver JA-A)", 0 )
GAME( 2001, bm7thmix, 0,        djmainj, bm6thmix,  djmain_state, init_bm7thmix,  ROT0, "Konami", "beatmania 7th MIX (ver JA-B)", 0 )
GAME( 2002, bmfinal,  0,        djmainj, bm6thmix,  djmain_state, init_bmfinal,   ROT0, "Konami", "beatmania THE FINAL (ver JA-A)", 0 )

GAME( 1998, popn1,    0,        djmaina, popn1,     djmain_state, init_beatmania, ROT0, "Konami", "Pop'n Music 1 (ver TA-A, HDD 1.01)", 0 )
GAME( 1998, popn1a,   popn1,    djmaina, popn1,     djmain_state, init_beatmania, ROT0, "Konami", "Pop'n Music 1 (ver AA-A, HDD 1.00)", 0 )
GAME( 1998, popn1k,   popn1,    djmaina, popn1,     djmain_state, init_beatmania, ROT0, "Konami", "Pop'n Music 1 (ver KA-A, HDD 1.01)", 0 ) // KA-A based on filenames provided, no warning message
GAME( 1998, popn1j,   popn1,    djmainj, popn1,     djmain_state, init_beatmania, ROT0, "Konami", "Pop'n Music 1 (ver JA-A, HDD 1.00)", 0 )
GAME( 1999, popn2,    0,        djmainj, popn2,     djmain_state, init_beatmania, ROT0, "Konami", "Pop'n Music 2 (ver JA-A)", 0 )
GAME( 1999, popn3,    0,        djmainj, popn2,     djmain_state, init_beatmania, ROT0, "Konami", "Pop'n Music 3 (ver JA-A)", 0 )
// Pop'n Stage
GAME( 1999, popnstex, 0,        djmainj, popnstex,  djmain_state, init_beatmania, ROT0, "Konami", "Pop'n Stage EX (ver JB-A)", 0 )

// for reference, these sets have not been verified
//GAME( 1998, bm3rdmxb, bm3rdmix, djmainj, bm3rdmix,  djmain_state, init_beatmania, ROT0, "Konami", "beatmania 3rd MIX (ver JA-B)", 0 )
