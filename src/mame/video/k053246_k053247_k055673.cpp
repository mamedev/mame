// license:BSD-3-Clause
// copyright-holders:David Haywood, Olivier Galibert
/***************************************************************************/
/*                                                                         */
/*    053246 with 053247 or 055673                                         */
/*    058142 with 055673                                                   */
/*                                                                         */
/*    Konami Sprite Chips                                                  */
/*                                                                         */
/***************************************************************************/
/* later Konami GX board replaces the 053246 with a 058142 */

/*

053246/053247
053246/055673
058142/055673
-------------


This chip combination generates a sprite layer in konami systems.  The
053246 manages the spriteram, computes the sprite tiles positions,
generates rom addresses and sends render commands to the
053247/055673.  That chip, with the read tile data, renders the data
to a double-buffered linebuffer, the other buffer being scanned out to
the mixer chip.


Per-sprite information (spriteram):

     f   e   d   c   b   a   9   8   7   6   5   4   3   2   1   0
+0   b  cz  vf  hf   -vs--   -hs--   ------------zcode------------
+2   -------------------------charcode----------------------------
+4   .   .   .   .   .   .   ----------------vpos-----------------
+6   .   .   .   .   .   .   ----------------hpos-----------------
+8   .   .   .   .   .   .   ----------------vzoom----------------
+a   .   .   .   .   .   .   ----------------hzoom----------------
+c  vb  hb   .   .   -sd--   ----------------attr-----------------
+e   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .



Registers 054236/058142 (objset1):

     f   e   d   c   b   a   9   8   7   6   5   4   3   2   1   0
+0   .   .   .   .   .   .   ----------------hscr-----------------
+2   .   .   .   .   .   .   ----------------vscr-----------------
+4   ---------ocha (low)----------   .   . sd0 dma res w16 gvf ghf
+6   ----------------------ocha (high)----------------------------


Registers 055673 (objset2):

     f   e   d   c   b   a   9   8   7   6   5   4   3   2   1   0
+0-f -----------------character rom data--------------------------
+10  .   .   .   . wr0   ---c0---- mo0 mh0   -mv0- bo0 bh0   -bv0-
+12  .   .   .   . wr1   ---c1---- mo1 mh1   -mv1- bo1 bh1   -bv1-
+14  .   .   .   . wr2   ---c2---- mo2 mh2   -mv2- bo2 bh2   -bv2-
+16  .   .   .   . wr3   ---c3---- mo3 mh3   -mv3- bo3 bh3   -bv3-
+18  .   .   .   .   ----bank1----   .   .   .   .   ----bank0----
+1a  .   .   .   .   ----bank3----   .   .   .   .   ----bank2----
+1c  .   .   .   .   ----coreg---- rom w1k sds pri owr   ---cm----
+1e  .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .






Sprite generators. Nothing is known about their external interface.
The sprite RAM format is very similar to the 053245.

053246 memory map (but the 053247 sees and processes them too):
000-001 W  global X offset
002-003 W  global Y offset
004     W  low 8 bits of the ROM address to read
005     W  bit 0 = flip screen X
           bit 1 = flip screen Y
           bit 2 = unknown
           bit 4 = interrupt enable
           bit 5 = unknown
006-007 W  high 16 bits of the ROM address to read

???-??? R  reads data from the gfx ROMs (16 bits in total). The address of the
           data is determined by the registers above


*/

#include "k053246_k053247_k055673.h"

const device_type K053246_053247 = device_creator<k053246_053247_device>;
const device_type K053246_055673 = device_creator<k053246_055673_device>;

DEVICE_ADDRESS_MAP_START(objset1, 16, k053246_055673_device)
	AM_RANGE(0x00, 0x01) AM_WRITE(hscr_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(vscr_w)
	AM_RANGE(0x04, 0x05) AM_WRITE(oms_w)
	AM_RANGE(0x06, 0x07) AM_WRITE(ocha_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(objset2, 16, k053246_055673_device)
	AM_RANGE(0x00, 0x07) AM_WRITE(atrbk_w)
	AM_RANGE(0x08, 0x0b) AM_WRITE(vrcbk_w)
	AM_RANGE(0x0c, 0x0d) AM_WRITE(opset_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(objset1_8, 8, k053246_055673_device)
ADDRESS_MAP_END

k053246_055673_device::k053246_055673_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K053246_055673, "053246/055673 Sprite Generator Combo", tag, owner, clock, "k055673", __FILE__),
	  device_gfx_interface(mconfig, *this),
	  m_dmairq_cb(*this),
	  m_dmaact_cb(*this),
	  m_region(*this, DEVICE_SELF)
{
	m_is_053247 = false;
}

k053246_055673_device::k053246_055673_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	  device_gfx_interface(mconfig, *this),
	  m_dmairq_cb(*this),
	  m_dmaact_cb(*this),
	  m_region(*this, DEVICE_SELF)
{
	m_is_053247 = false;
}

void k053246_055673_device::set_info(const char *palette_tag, const char *spriteram_tag)
{
	static_set_palette(*this, palette_tag);
	m_spriteram_tag = spriteram_tag;
}

void k053246_055673_device::set_wiring_cb(wiring_delegate cb)
{
	m_wiring_cb = cb;
}

void k053246_055673_device::device_start()
{
	m_dmaact_cb.resolve_safe();
	m_dmairq_cb.resolve_safe();

	// Can't use a required_* because the pointer type varies
	m_spriteram = owner()->memshare(m_spriteram_tag);
	if(!m_spriteram)
		fatalerror("Spriteram shared memory '%s' does not exist\n", m_spriteram_tag);
	if(m_spriteram->bytewidth() > 4)
		fatalerror("Spriteram shared memory '%s' byte width is %d, which is too large\n", m_spriteram_tag, m_spriteram->bytewidth());

	m_timer_objdma = timer_alloc(0);

	save_item(NAME(m_timer_objdma_state));
	save_item(NAME(m_sram));
	save_item(NAME(m_ocha));
	save_item(NAME(m_hscr));
	save_item(NAME(m_vscr));
	save_item(NAME(m_atrbk));
	save_item(NAME(m_vrcbk));
	save_item(NAME(m_oms));
	save_item(NAME(m_coreg));
	save_item(NAME(m_opset));
	save_item(NAME(m_dmairq_on));
}

void k053246_055673_device::device_reset()
{
	m_ocha = 0;
	m_hscr = 0;
	m_vscr = 0;
	memset(m_atrbk, 0, sizeof(m_atrbk));
	memset(m_vrcbk, 0, sizeof(m_vrcbk));
	m_oms = 0;
	m_coreg = 0;
	m_opset = 0;
	m_dmairq_on = false;

	m_timer_objdma_state = OD_IDLE;
	m_timer_objdma->adjust(attotime::never);

	decode_sprite_roms();
}

void k053246_055673_device::device_post_load()
{
	decode_sprite_roms();
}

void k053246_055673_device::set_objcha(bool active)
{
}

WRITE16_MEMBER(k053246_055673_device::hscr_w)
{
	uint16_t old = m_hscr;
	COMBINE_DATA(&m_hscr);
	if(m_hscr != old) {
		logerror("hscr_w %04x\n", m_hscr);
		logerror("TIMINGS OH %4d %03x\n", m_hscr & 0x3ff, m_hscr & 0x3ff);
	}
}

WRITE16_MEMBER(k053246_055673_device::vscr_w)
{
	uint16_t old = m_vscr;
	COMBINE_DATA(&m_vscr);
	if(m_vscr != old) {
		logerror("vscr_w %04x\n", m_vscr);
		logerror("TIMINGS OV %4d %03x\n", m_vscr & 0x3ff, m_vscr & 0x3ff);
	}
}

WRITE16_MEMBER(k053246_055673_device::oms_w)
{
	if(ACCESSING_BITS_8_15 && (data >> 8) != (m_ocha & 0xff))
		m_ocha = (m_ocha & 0xffff00) | (data >> 8);

	if(ACCESSING_BITS_0_7 && (data & 0x3f) != (m_oms & 0x3f)) {
		uint8_t diff = (data ^ m_oms) & 0x2f;
		m_oms = data & 0x3f;
		if(!(m_oms & 0x10) && m_dmairq_on) {
			m_dmairq_on = false;
			m_dmairq_cb(CLEAR_LINE);
		}

		if(diff)
			logerror("oms_w sd0en=%s dma=%s res=%s mode=%d flip=%c%c\n",
					 m_oms & 0x20 ? "normal" : "shadow",
					 m_oms & 0x10 ? "on" : "off",
					 m_oms & 0x08 ? "high" : "normal",
					 m_oms & 0x04 ? 16 : 8,
					 m_oms & 0x02 ? 'y' : '-',
					 m_oms & 0x01 ? 'x' : '-');
	}
}

WRITE16_MEMBER(k053246_055673_device::ocha_w)
{
	m_ocha = (m_ocha & 0x0000ff) | ((data << 8) & 0xffff00);
}

WRITE16_MEMBER(k053246_055673_device::atrbk_w)
{
	if(m_atrbk[offset] != (data & 0xfff)) {
		m_atrbk[offset] = data & 0xfff;
		logerror("atrbk_w %d wr=%s trans=0-%x mix=%c%c bri=%c%c\n",
				 offset,
				 m_atrbk[offset] & 0x800 ? "on" : "off",
				 0x7f >> (((~data) >> 8) & 7),
				 m_atrbk[offset] & 0x040 ? '0' : m_atrbk[offset] & 0x080 ? 'c' : m_atrbk[offset] & 0x020 ? '1' : '0',
				 m_atrbk[offset] & 0x080 ? 'c' : m_atrbk[offset] & 0x010 ? '1' : '0',
				 m_atrbk[offset] & 0x004 ? '0' : m_atrbk[offset] & 0x008 ? 'c' : m_atrbk[offset] & 0x002 ? '1' : '0',
				 m_atrbk[offset] & 0x008 ? 'c' : m_atrbk[offset] & 0x001 ? '1' : '0');

	}
}

WRITE16_MEMBER(k053246_055673_device::vrcbk_w)
{
	if(ACCESSING_BITS_8_15 && m_vrcbk[offset*2+1] != ((data >> 8) & 0xf)) {
		m_vrcbk[offset*2+1] = (data >> 8) & 0x0f;
		logerror("vrcbk[%d] = %x\n", offset*2+1, m_vrcbk[offset*2+1]);
	}

	if(ACCESSING_BITS_0_7 && m_vrcbk[offset*2] != (data & 0xf)) {
		m_vrcbk[offset*2] = data & 0x0f;
		logerror("vrcbk[%d] = %x\n", offset*2, m_vrcbk[offset*2]);
	}
}

WRITE16_MEMBER(k053246_055673_device::opset_w)
{
	if(ACCESSING_BITS_8_15 && m_coreg != ((data >> 8) & 0xf)) {
		m_coreg = (data >> 8) & 0xf;
		logerror("coreg_w %x\n", m_coreg);
	}
	if(ACCESSING_BITS_0_7 && m_opset != (data & 0xff)) {
		static int bpp[8] = { 4, 5, 6, 7, 8, 8, 8, 8 };
		m_opset = data;
		decode_sprite_roms();
		logerror("opset_w rom=%s wrap=%d sdsel=%s pri=%s objwr=%s bpp=%d\n",
				 m_opset & 0x80 ? "readback" : "normal",
				 m_opset & 0x40 ? 512 : 1024,
				 m_opset & 0x20 ? "pri" : "nopri",
				 m_opset & 0x10 ? "higher" : "lower",
				 m_opset & 0x08 ? "invert" : "normal",
				 bpp[m_opset & 7]);
	}
}

READ8_MEMBER(k053246_055673_device::rom8_r)
{
	uint32_t off = ((m_vrcbk[(m_ocha >> 20) & 3] << 22) | ((m_ocha & 0xffffc) << 2) | offset) & (m_region->bytes() - 1);
	const uint8_t *rom = m_region->base() + (off^1);
	return rom[0];
}

READ16_MEMBER(k053246_055673_device::rom16_r)
{
	uint32_t off = ((m_vrcbk[(m_ocha >> 20) & 3] << 22) | ((m_ocha & 0xffffc) << 2) | (offset << 1)) & (m_region->bytes() - 1);
	const uint8_t *rom = m_region->base() + off;
	return (rom[1] << 8) | rom[0];
}

READ32_MEMBER(k053246_055673_device::rom32_r)
{
	uint32_t off = ((m_vrcbk[(m_ocha >> 20) & 3] << 22) | ((m_ocha & 0xffffc) << 2) | (offset << 2)) & (m_region->bytes() - 1);
	const uint8_t *rom = m_region->base() + off;
	return (rom[1] << 24) | (rom[0] << 16) | (rom[3] << 8) | rom[2];
}


WRITE_LINE_MEMBER(k053246_055673_device::vblank_w)
{
	if(state) {
		if(m_oms & 0x10) {
			if(m_dmairq_on) {
				m_dmairq_on = false;
				m_dmairq_cb(CLEAR_LINE);
			}
			m_timer_objdma_state = OD_WAIT_START;
			m_timer_objdma->adjust(attotime::from_ticks(256, clock()));
		} else
			memset(m_sram, 0, sizeof(m_sram));
	}
}

void k053246_055673_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(m_timer_objdma_state) {
	case OD_WAIT_START:
		m_timer_objdma_state = OD_WAIT_END;
		m_timer_objdma->adjust(attotime::from_ticks(2048, clock()));
		m_dmaact_cb(ASSERT_LINE);
		switch(m_spriteram->bytewidth()) {
		case 1: {
			const uint8_t *base = (const uint8_t *)m_spriteram->ptr();
			for(uint32_t i=0; i<0x800; i++)
				m_sram[i] = (base[2*i] << 8) | base[2*i+1];
			break;
		}
		case 2: {
			memcpy(m_sram, m_spriteram->ptr(), 0x1000);
			break;
		}
		case 4: {
			const uint32_t *base = (const uint32_t *)m_spriteram->ptr();
			for(uint32_t i=0; i<0x800; i+=2) {
				m_sram[i] = base[i>>1] >> 16;
				m_sram[i+1] = base[i>>1];
			}
			break;
		}
		}
		break;

	case OD_WAIT_END:
		m_timer_objdma_state = OD_IDLE;
		m_timer_objdma->adjust(attotime::never);
		m_dmaact_cb(CLEAR_LINE);
		if((m_oms & 0x10) && !m_dmairq_on) {
			m_dmairq_on = true;
			m_dmairq_cb(ASSERT_LINE);
		}
		break;
	}
}

void k053246_055673_device::generate_tile_layout(tile_layout *tl, int &tl_count, uint32_t minp, uint32_t maxp, int screen_center, int offset, int width_order, int zoom, const int *tile_id_steps, bool flip, bool mirror, bool gflip)
{
	tl_count = 0;
	if(!zoom)
		return;

	if(gflip) {
		screen_center = ~screen_center;
		flip = !flip;
	}

	screen_center += offset;

	uint32_t pxor[2], tbase[2];
	int32_t step[2];
	uint32_t pxor_test = width_order ? 1 << (width_order-1) : 0;
	uint32_t xor_mask = (1 << width_order) - 1;

	if(mirror) {
		pxor[0]  = 0;
		pxor[1]  = xor_mask;
		tbase[0] = 0;
		tbase[1] = 0x3ff;
		step[0]  = zoom;
		step[1]  = -zoom;
	} else if(flip) {
		pxor[0]  = xor_mask;
		pxor[1]  = xor_mask;
		tbase[0] = 0x3ff;
		tbase[1] = 0x3ff;
		step[0]  = -zoom;
		step[1]  = -zoom;
	} else {
		pxor[0]  = 0;
		pxor[1]  = 0;
		tbase[0] = 0;
		tbase[1] = 0;
		step[0]  = zoom;
		step[1]  = zoom;
	}

	if(false)
		fprintf(stderr, "generate span=(%d %d) center=%d width=%d zoom=%d%s%s\n", minp, maxp, screen_center, 1<<(4+width_order), zoom, flip ? " (flipped)" : "", mirror ? " (mirrored)" : "");

	// Assume the inverse table produces 10 bits, we'll see how it goes
	uint32_t inverse = (1 << (10+10)) / zoom;

	// All computations are fixed-point, 6 bits as pixel fraction.
	// Integer part size tends to be 10 bits.

	// Wrap at 512 or 1024 according to the internal
	// configuration.  We don't really know where zero is, but
	// it's around there, where "there" is the end of hsync.
	uint32_t wrap_mask = m_opset & 0x40 ? 0x7fff : 0xffff;

	// Start at the left/top position (max width_order is 3), which is
	// at half the sprite size.
	uint32_t pos = ((screen_center << 6) - (inverse >> (5 - width_order))) & wrap_mask;

	for(uint32_t tnum = 0; tnum != (1 << width_order); tnum++) {
		// Advance by 16 sprite pixels
		uint32_t npos = (pos + (inverse >> 4)) & wrap_mask;

		// Compute integer screen positions
		uint32_t pos1 = pos >> 6;
		uint32_t npos1 = npos >> 6;

		if(false)
			fprintf(stderr, "* %d  %x %x -> %d %d\n", tnum, pos, npos, pos1, npos1);

		// Only bother if not fully clipped and not reduced to zero pixels
		if(((npos1 > pos1) && (npos1 > minp && pos1 <= maxp)) ||
		   ((pos1 > npos1) && (npos1 > minp || pos1 <= maxp))) {
			// Basics of the tile
			int slot = (tnum & pxor_test) ? 1 : 0;
			tl[tl_count].step = step[slot];
			tl[tl_count].tileid_delta = tile_id_steps[tnum ^ pxor[slot]];

			// Left clipping, if needed.  Probably done with hard
			// clipping in the chip.
			if(pos1 < minp || pos1 > maxp) {
				tl[tl_count].sc_min = minp;
				tl[tl_count].tile_min = tbase[slot] + step[slot] * ((minp - pos1) & 0x1ff);
			} else {
				tl[tl_count].sc_min = pos1;
				tl[tl_count].tile_min = tbase[slot];
			}

			// Right clipping
			if(npos1 > minp && npos1 <= maxp)
				tl[tl_count].sc_max = npos1 - 1;
			else
				tl[tl_count].sc_max = maxp;

			tl_count++;
		}
		pos = npos;
	}
	if(false) {
		for(int i=0; i<tl_count; i++)
			fprintf(stderr, " [(%d, %d) %x.%02x %d %d]", tl[i].sc_min, tl[i].sc_max, (tl[i].tile_min >> 6) & 7, tl[i].tile_min & 0x3f, tl[i].step, tl[i].tileid_delta);
		fprintf(stderr, "\n");
	}
}

template<uint16_t mask> void k053246_055673_device::draw_tile_keep_shadow(bitmap_ind16 *bcolor, bitmap_ind16 *battr,
																		  gfx_element *g, uint32_t tile_base_id,
																		  const tile_layout &lx, const tile_layout &ly,
																		  uint16_t attr, uint16_t palette, uint8_t trans_mask)
{
	uint32_t tid = tile_base_id ^ lx.tileid_delta ^ ly.tileid_delta;
	const uint8_t *data = g->get_data(tid);
	uint32_t typ = ly.tile_min;
	for(int y = ly.sc_min; y <= ly.sc_max; y++) {
		const uint8_t *data1 = data + 16*(typ >> 6);		
		uint16_t *bc = &bcolor->pix16(y, lx.sc_min);
		uint16_t *ba = &battr ->pix16(y, lx.sc_min);
		uint32_t txp = lx.tile_min;
		for(int x = lx.sc_min; x <= lx.sc_max; x++) {
			uint8_t col = data1[txp >> 6];
			if(col & trans_mask) {
				*bc++ = palette | col;
				*ba = (*ba & mask) | attr;
				ba++;
			} else {
				bc++;
				ba++;
			}
			txp += lx.step;
		}
		typ += ly.step;
	}
}

void k053246_055673_device::draw_tile_force_shadow(bitmap_ind16 *bcolor, bitmap_ind16 *battr,
												   gfx_element *g, uint32_t tile_base_id,
												   const tile_layout &lx, const tile_layout &ly,
												   uint16_t attr, uint16_t palette, uint8_t trans_mask)
{
	uint32_t tid = tile_base_id + lx.tileid_delta + ly.tileid_delta;
	const uint8_t *data = g->get_data(tid);
	uint32_t typ = ly.tile_min;
	for(int y = ly.sc_min; y <= ly.sc_max; y++) {
		const uint8_t *data1 = data + 16*(typ >> 6);		
		uint16_t *bc = &bcolor->pix16(y, lx.sc_min);
		uint16_t *ba = &battr ->pix16(y, lx.sc_min);
		uint32_t txp = lx.tile_min;
		for(int x = lx.sc_min; x <= lx.sc_max; x++) {
			uint8_t col = data1[txp >> 6];
			if(col & trans_mask) {
				*bc++ = palette | col;
				*ba++ = attr;

			} else {
				bc++;
				ba++;
			}
			txp += lx.step;
		}
		typ += ly.step;
	}
}

template<uint16_t mask> void k053246_055673_device::draw_tile_keep_detect_shadow(bitmap_ind16 *bcolor, bitmap_ind16 *battr,
																				 gfx_element *g, uint32_t tile_base_id,
																				 const tile_layout &lx, const tile_layout &ly,
																				 uint16_t noshadow_attr, uint16_t shadow_attr,
																				 uint16_t palette, uint8_t trans_mask, uint8_t shadow_color)
{
	uint32_t tid = tile_base_id | lx.tileid_delta | ly.tileid_delta;
	const uint8_t *data = g->get_data(tid);
	uint32_t typ = ly.tile_min;
	for(int y = ly.sc_min; y <= ly.sc_max; y++) {
		const uint8_t *data1 = data + 16*(typ >> 6);		
		uint16_t *bc = &bcolor->pix16(y, lx.sc_min);
		uint16_t *ba = &battr ->pix16(y, lx.sc_min);
		uint32_t txp = lx.tile_min;
		for(int x = lx.sc_min; x <= lx.sc_max; x++) {
			uint8_t col = data1[txp >> 6];
			if((col & shadow_color) == shadow_color) {
				*bc++ = palette; // Hypothesis: a sprite cannot shadow another sprite
				*ba++ = shadow_attr;
				
			} else if(col & trans_mask) {
				*bc++ = palette | col;
				*ba = (*ba & mask) | noshadow_attr;
				ba++;

			} else {
				bc++;
				ba++;
			}
			txp += lx.step;
		}
		typ += ly.step;
	}
}

void k053246_055673_device::draw_tile_force_detect_shadow(bitmap_ind16 *bcolor, bitmap_ind16 *battr,
														  gfx_element *g, uint32_t tile_base_id,
														  const tile_layout &lx, const tile_layout &ly,
														  uint16_t noshadow_attr, uint16_t shadow_attr,
														  uint16_t palette, uint8_t trans_mask, uint8_t shadow_color)
{
	uint32_t tid = tile_base_id | lx.tileid_delta | ly.tileid_delta;
	const uint8_t *data = g->get_data(tid);
	uint32_t typ = ly.tile_min;
	for(int y = ly.sc_min; y <= ly.sc_max; y++) {
		const uint8_t *data1 = data + 16*(typ >> 6);		
		uint16_t *bc = &bcolor->pix16(y, lx.sc_min);
		uint16_t *ba = &battr ->pix16(y, lx.sc_min);
		uint32_t txp = lx.tile_min;
		for(int x = lx.sc_min; x <= lx.sc_max; x++) {
			uint8_t col = data1[txp >> 6];
			if((col & shadow_color) == shadow_color) {
				*bc++ = palette;
				*ba++ = shadow_attr;
				
			} else if(col & trans_mask) {
				*bc++ = palette | col;
				*ba++ = noshadow_attr;

			} else {
				bc++;
				ba++;
			}
			txp += lx.step;
		}
		typ += ly.step;
	}
}

void k053246_055673_device::bitmap_update(bitmap_ind16 *bcolor, bitmap_ind16 *battr, const rectangle &cliprect)
{
	static const int xoff[16] = { 0, 1, 4,  5, 16, 17, 20, 21, 0, 1, 4,  5, 16, 17, 20, 21 };
	static const int yoff[16] = { 0, 2, 8, 10, 32, 34, 40, 42, 0, 2, 8, 10, 32, 34, 40, 42 };
	static const int unwrap[32] = { 0, 1, 0, 1, 2, 3, 2, 3, 0, 1, 0, 1, 2, 3, 2, 3, 4, 5, 4, 5, 6, 7, 6, 7, 4, 5, 4, 5, 6, 7, 6, 7 };
	uint16_t wrmask = m_opset & 0x08 ? 0x0800 : 0x000;
	int bpp;
	if(m_opset & 0x04)
		bpp = 8;
	else
		bpp = 4 | (m_opset & 0x03);

	uint8_t shadow_color = (1 << bpp) - 1;
	uint32_t coreg = (m_coreg << 12) & (0xf00 << bpp);
	
	bcolor->fill(0, cliprect);
	battr->fill(0, cliprect);

	int offx = 0, offy = 0;

	uint32_t off = m_hscr & 0x3ff;
	bool flip = m_oms & 0x01;

	// goku [:video_timings] TIMINGS '252  6 384 48 288 16 32 264 15 224 17  8
	// sexy [:video_timings] TIMINGS '252  6 384 48 288 16 32 264 15 224 17  8

	static uint32_t curx = 0x02c;

	// gokuparo
	if(     !flip && off == 0x3be && 1)
		offx = 0x02c;
	else if( flip && off == 0x296 && 1)
		offx = 0x156;

	else {
		if(machine().input().code_pressed(KEYCODE_D))
			curx++;
		if(machine().input().code_pressed(KEYCODE_A))
			curx--;
		curx &= 0x3ff;
		logerror("Unknown X offset, hscr=%03x, flip=%s, cur=%03x\n", off, flip ? "on" : "off", curx);
		offx = curx;
	}

	static uint32_t cury = 0x000;

	off = m_vscr & 0x3ff;
	flip = m_oms & 0x02;

	// gokuparo, 
	if(      flip && off == 0x3e9 && 1)
		offy = 0x000;
	else if(!flip && off == 0x2e9 && 1)
		offy = 0x0ff;

	else {
		if(machine().input().code_pressed(KEYCODE_S))
			cury++;
		if(machine().input().code_pressed(KEYCODE_W))
			cury--;
		cury &= 0x3ff;
		logerror("Unknown Y offset, vscr=%03x, flip=%s, cur=%03x\n", off, flip ? "on" : "off", cury);
		offy = cury;
	}

	// The sprites are drawn in priority order.  The draw order within
	// a priority level is unknown.
	int bucket[256];
	int next[256];
	memset(bucket, 0xff, sizeof(bucket));
	memset(next, 0xff, sizeof(next));

	uint8_t prixor = m_opset & 0x10 ? 0x00 : 0xff;
	for(int i=0; i<0x100; i++) {
		const uint16_t *spr = m_sram + (i << 3);
		if(!(spr[0] & 0x8000))
			continue;

		int slot = (spr[0] ^ prixor) & 0xff; 
		next[i] = bucket[slot];
		bucket[slot] = i;
	}

	gfx_element *g = gfx(0);
	for(int i=0; i<256; i++) {
		int sid = bucket[i];
		while(sid != -1) {
			const uint16_t *spr = m_sram + (sid << 3);
			
			uint16_t atrbk = m_is_053247 ? 0x0000 : m_atrbk[(spr[6] >> 8) & 3];
			if((atrbk ^ wrmask) & 0x0800)
				continue;
			
			int osx = (spr[0] >>  8) & 3;
			int osy = (spr[0] >> 10) & 3;

			uint32_t tile_base_id = spr[1];
			if(!m_is_053247)
				tile_base_id = ((tile_base_id & 0x3fff) | (m_vrcbk[(tile_base_id >> 14) & 3] << 14));

			int dx = unwrap[tile_base_id & 0x1f];
			int dy = unwrap[(tile_base_id >> 1) & 0x1f];
			tile_base_id &= ~0x3f;

			int zoom = spr[4] & 0x3ff;
			tile_layout ly[16];
			int tly;
			generate_tile_layout(ly, tly, cliprect.min_y, cliprect.max_y, -spr[2], offy, osy, zoom, yoff+dy, spr[0] & 0x2000, spr[6] & 0x8000, m_oms & 2);
			if(!tly)
				goto skip;
			
			if(!(spr[0] & 0x4000))
				zoom = spr[5] & 0x3ff;
			tile_layout lx[16];
			int tlx;
			generate_tile_layout(lx, tlx, cliprect.min_x, cliprect.max_x, spr[3], offx, osx, zoom, xoff+dx, spr[0] & 0x1000, spr[6] & 0x4000, m_oms & 1);
		
			if(!tlx)
				goto skip;

			uint32_t info_ns, info_s;
			uint16_t trans_mask, mixbri;
			if(!m_is_053247) {
				info_s = ((spr[6] & 0xf00) << 8) | ((spr[6] & 0x0ff) << bpp) | coreg;
				info_ns = info_s & (m_oms & 0x20 ? 0x3ffff : 0x7ffff);
				trans_mask = (0xff << ((atrbk >> 8) & 7)) & shadow_color;
				mixbri = 0;
				switch(atrbk & 0x0c0) {
				case 0x000: mixbri |= (atrbk & 0x030) << 8; break;
				case 0x040: mixbri |= (atrbk & 0x010) << 8; break;
				default: logerror("unhandled mix atrbk %x\n", atrbk);
				}
				switch(atrbk & 0x00c) {
				case 0x000: mixbri |= (atrbk & 0x003) << 10; break;
				case 0x004: mixbri |= (atrbk & 0x001) << 10; break;
				default: logerror("unhandled bri atrbk %x\n", atrbk);
				}
			} else {
				info_s  = (spr[6] & 0xfff) << 4;
				info_ns = info_s & (m_oms & 0x20 ? 0x3fff : 0x7fff);
				trans_mask = shadow_color;
				mixbri = 0;
			}

#if 0
			color = (spr[6] << bpp) & 0xff;
			attr = ((spr[6] & 0xff) >> (8-bpp)) | coreg;
#endif
			uint16_t color, shadow_attr, noshadow_attr;

			m_wiring_cb(info_ns, color, noshadow_attr);
			m_wiring_cb(info_s,  color, shadow_attr);

			noshadow_attr |= mixbri;
			shadow_attr   |= mixbri;

			//			logerror("tbid %x color %x ns %x s %x atrbk %x [%x %x %x %x]\n", tile_base_id, color, noshadow_attr, shadow_attr, atrbk, m_atrbk[0], m_atrbk[1], m_atrbk[2], m_atrbk[3]);

			//		uint16_t color          = info_ns & 0x1ff;
			//		uint16_t noshadow_attr  = ((info_ns & 0xc000) >> 6) | ((info_ns >> 8) & 0x3e);
			//		uint16_t shadow_attr    = ((info_s  & 0xc000) >> 6) | ((info_s  >> 8) & 0x3e);

			if((m_opset & 0x20) || !(m_oms & 0x20)) {
				if(!(spr[6] & 0x0c00))
					for(int y=0; y<tly; y++)
						for(int x=0; x<tlx; x++)
							draw_tile_force_shadow(bcolor, battr, g, tile_base_id, lx[x], ly[y], noshadow_attr, color, trans_mask);
				else
					for(int y=0; y<tly; y++)
						for(int x=0; x<tlx; x++)
							draw_tile_force_detect_shadow(bcolor, battr, g, tile_base_id, lx[x], ly[y], noshadow_attr, shadow_attr, color, trans_mask, shadow_color);

			} else if(m_oms & 0x20)  {
				if(!(spr[6] & 0x0c00))
					for(int y=0; y<tly; y++)
						for(int x=0; x<tlx; x++)
							draw_tile_keep_shadow<0x0300>(bcolor, battr, g, tile_base_id, lx[x], ly[y], noshadow_attr, color, trans_mask);
				else
					for(int y=0; y<tly; y++)
						for(int x=0; x<tlx; x++)
							draw_tile_keep_detect_shadow<0x0300>(bcolor, battr, g, tile_base_id, lx[x], ly[y], noshadow_attr, shadow_attr, color, trans_mask, shadow_color);

			} else {
				if(!(spr[6] & 0x0800))
					for(int y=0; y<tly; y++)
						for(int x=0; x<tlx; x++)
							draw_tile_keep_shadow<0x0100>(bcolor, battr, g, tile_base_id, lx[x], ly[y], noshadow_attr, color, trans_mask);
				else
					for(int y=0; y<tly; y++)
						for(int x=0; x<tlx; x++)
							draw_tile_keep_detect_shadow<0x0100>(bcolor, battr, g, tile_base_id, lx[x], ly[y], noshadow_attr, shadow_attr, color, trans_mask, shadow_color);
			}

		skip:
			sid = next[sid];
		}
	}
}

void k053246_055673_device::decode_sprite_roms()
{
	gfx_layout gfx_layouts[1];
	gfx_decode_entry gfx_entries[2];

	int bpp;

	if(m_is_053247)
		bpp = 4;
	else if(m_opset & 0x04)
		bpp = 8;
	else
		bpp = 4 | (m_opset & 0x03);

	logerror("Decoding sprite roms as %d bpp %s\n", bpp, m_is_053247 ? "chunky" : "planar");

	int bits_per_block = m_is_053247 ? 32 : 64;

	gfx_layouts[0].width = 16;
	gfx_layouts[0].height = 16;
	gfx_layouts[0].total = m_region->bytes() / (bits_per_block*16*2/8);
	gfx_layouts[0].planes = bpp;

	if(m_is_053247) {
		// Chunky format, 32 bits per line, 4bpp
		for(int j=0; j<bpp; j++)
			gfx_layouts[0].planeoffset[j] = j;
		for(int j=0; j<16; j++) {
			gfx_layouts[0].xoffset[j] = 4*(j & 7) + (j & 8 ? bits_per_block : 0);
			gfx_layouts[0].yoffset[j] = j*2*bits_per_block;
		}

	} else {
		// Planar format, 64 bits per line (32 to 64 actually used)
		for(int j=0; j<bpp; j++)
			gfx_layouts[0].planeoffset[bpp-1-j] = 8*j;
		for(int j=0; j<16; j++) {
			gfx_layouts[0].xoffset[j] = (j & 7) + (j & 8 ? bits_per_block : 0);
			gfx_layouts[0].yoffset[j] = j*2*bits_per_block;
		}
	}
	gfx_layouts[0].extxoffs = nullptr;
	gfx_layouts[0].extyoffs = nullptr;
	gfx_layouts[0].charincrement = bits_per_block * 2 * 16;

	gfx_entries[0].memory_region = tag();
	gfx_entries[0].start = 0;
	gfx_entries[0].gfxlayout = gfx_layouts;
	gfx_entries[0].color_codes_start = 0;
	gfx_entries[0].total_color_codes = palette().entries() >> bpp;
	gfx_entries[0].flags = 0;

	gfx_entries[1].gfxlayout = nullptr;

	decode_gfx(gfx_entries);
}


k053246_053247_device::k053246_053247_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: k053246_055673_device(mconfig, K053246_053247, "K053246/053247 Sprite Generator Combo", tag, owner, clock, "k053247", __FILE__)
{
	m_is_053247 = true;
}

READ8_MEMBER(k053246_053247_device::rom8_r)
{
	uint32_t off = ((m_ocha << 1) | offset) & (m_region->bytes() - 1);
	const uint8_t *rom = m_region->base() + (off^1);
	return rom[0];
}

READ16_MEMBER(k053246_053247_device::rom16_r)
{
	uint32_t off = (m_ocha << 1) & (m_region->bytes() - 1);
	const uint8_t *rom = m_region->base() + off;
	return (rom[1] << 8) | rom[0];
}

READ32_MEMBER(k053246_053247_device::rom32_r)
{
	abort();
}
