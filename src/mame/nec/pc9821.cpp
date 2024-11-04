// license:BSD-3-Clause
// copyright-holders:Angelo Salese,Carl
/**************************************************************************************************

    NEC PC-9821

    follow-up to PC-9801 for the consumer market

    TODO (PC-9821):
    - non-fatal "cache error" at POST for all machines listed here;
    - undumped IDE ROM, kludged to work;
    - further state machine breakdowns;

    TODO (PC-9821As):
    - unimplemented SDIP specific access;
    - "SYSTEM SHUTDOWN" while accessing above;
    - Update: it never goes into above after I changed default m_dma_access_ctrl to 0xfe?

    TODO (PC-9821Cx3):
    - "MICON ERROR" at POST, we currently return a ready state in remote control register
      to bypass it, is it expected behaviour?
    - Hangs normally with "Set the SDIP" message, on soft reset tries to r/w I/Os
      $b00-$b03, kanji RAM $a9 and $f0 (mostly bit 5, built-in 27 inches HDD check?) then keeps
      looping;
    - 0xfa2c8 contains ITF test routines, to access it's supposedly CTRL+CAPS+KANA,
      which currently doesn't work. It also never returns a valid processor or CPU clock,
      is it a debug side-effect or supposed to be read somehow?
    - Expects 0xc0000-0xdffff to be r/w at PC=0x104e8, currently failing for inner C-Bus mappings.
      Is PCI supposed to overlay the C-Bus section?
    - Eventually jump off the weeds by taking an invalid irq in timer test;
    - Reportedly should display a CanBe logo at POST (always blue with white fg?),
      at least pc9821cx3 ROM has some VRAM data in first half of BIOS ROM.
      Where this is mapped is currently unknown;

    TODO (PC-9821Xa16/PC-9821Ra20/PC-9821Ra266/PC-9821Ra333):
    - "MICON ERROR" at POST (processor microcode detection fails, basically down to a more
      involved bankswitch with Pentium based machines);

    TODO: (PC-9821Nr15/PC-9821Nr166)
    - Tests conventional RAM then keeps polling $03c4 (should be base VGA regs read);
    - Skipping that will eventually die with a "MEMORY ERROR" (never reads extended memory);

**************************************************************************************************/

#include "emu.h"
#include "pc9821.h"

uint32_t pc9821_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	if(m_video_ff[DISPLAY_REG] != 0)
	{
		// TODO: following is wrong: definitely requires GDC latches
		// flashb (pitch=512) and skinpan (two areas on title screen)
		// aitd sets pitch=40, which means something else inside 7220 should do the bump.

		// PEGC 256 mode is linear VRAM picked from a specific VRAM buffer.
		// It doesn't latch values from GDC, it runs on its own renderer instead.
		// Is the DAC really merging two pixels not unlike VGA correlated Mode 13h?
		// https://github.com/joncampbell123/dosbox-x/issues/1289#issuecomment-543025016
		// TODO: getter cliprect from bitmap GDC
		if (m_ex_video_ff[ANALOG_256_MODE])
		{
			rgb_t const *const palette = m_palette->palette()->entry_list_raw();
			u8 *ext_gvram = (u8 *)m_ext_gvram.target();
			int base_y = cliprect.min_y;

			for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
			{
				for (int x = cliprect.min_x; x <= cliprect.max_x; x += 8)
				{
					u32 address = (y - base_y) * (640 / 8) + (x >> 3);
					for(int xi = 0; xi < 8; xi ++)
					{
						int res_x = x + xi;
						int res_y = y;

						u16 pen = ext_gvram[(address << 3) + xi + (m_vram_disp * 0x40000)];

						bitmap.pix(res_y, res_x) = palette[(pen & 0xff) + 0x20];
					}
				}
			}
		}
		else
			m_hgdc[1]->screen_update(screen, bitmap, cliprect);
		m_hgdc[0]->screen_update(screen, bitmap, cliprect);
	}

	return 0;
}

// old code, for consultation
#if 0
UPD7220_DISPLAY_PIXELS_MEMBER( pc9821_state::pegc_display_pixels )
{
	if(m_ex_video_ff[ANALOG_256_MODE])
	{
		rgb_t const *const palette = m_palette->palette()->entry_list_raw();
		u16 *ext_gvram = (u16 *)m_ext_gvram.target();

		for(int xi=0;xi<8;xi+=2)
		{
			int res_x = (x >> 1) + xi;
			int res_y = y;

			u16 pen = ext_gvram[(address << 2) + (xi >> 1) + (m_vram_disp*0x20000)];

			bitmap.pix(res_y, res_x) = palette[(pen & 0xff) + 0x20];
			bitmap.pix(res_y, res_x+1) = palette[(pen >> 8) + 0x20];
		}
	}
	else
		pc9801_state::hgdc_display_pixels(bitmap, y, x, address);
}
#endif

void pc9821_state::pc9821_egc_w(offs_t offset, u16 data, u16 mem_mask)
{
	if(!m_ex_video_ff[2])
		return;

	egc_w(offset, data, mem_mask);
}

void pc9821_state::pc9821_video_ff_w(offs_t offset, uint8_t data)
{
	if(offset == 1)
	{
		if(((data & 0xfe) == 4) && !m_ex_video_ff[3]) // TODO: many other settings are protected
			return;
		m_ex_video_ff[(data & 0xfe) >> 1] = data & 1;

		if((data & 0xfe) == 0x20)
		{
			if (data & 1)
				m_pegc_mmio_view.select(0);
			else
				m_pegc_mmio_view.disable();
		}
	}

	/* Intentional fall-through */
	pc9801rs_video_ff_w(offset,data);
}

uint8_t pc9821_state::pc9821_a0_r(offs_t offset)
{
	if((offset & 1) == 0 && offset & 8)
	{
		if(m_ex_video_ff[ANALOG_256_MODE])
		{
			logerror("256 color mode [%02x] R\n",offset);
			return 0;
		}
		else if(m_ex_video_ff[ANALOG_16_MODE]) //16 color mode, readback possible there
		{
			uint8_t res = 0;

			switch(offset)
			{
				case 0x08: res = m_analog16.pal_entry & 0xf; break;
				case 0x0a: res = m_analog16.g[m_analog16.pal_entry] & 0xf; break;
				case 0x0c: res = m_analog16.r[m_analog16.pal_entry] & 0xf; break;
				case 0x0e: res = m_analog16.b[m_analog16.pal_entry] & 0xf; break;
			}

			return res;
		}
	}

	return pc9801_a0_r(offset);
}

void pc9821_state::pc9821_a0_w(offs_t offset, uint8_t data)
{
	if((offset & 1) == 0 && offset & 8 && m_ex_video_ff[ANALOG_256_MODE])
	{
		switch(offset)
		{
			case 0x08: m_pegc.pal_entry = data & 0xff; break;
			case 0x0a: m_pegc.g[m_pegc.pal_entry] = data & 0xff; break;
			case 0x0c: m_pegc.r[m_pegc.pal_entry] = data & 0xff; break;
			case 0x0e: m_pegc.b[m_pegc.pal_entry] = data & 0xff; break;
		}

		m_palette->set_pen_color(
			m_pegc.pal_entry + 0x20,
			m_pegc.r[m_pegc.pal_entry],
			m_pegc.g[m_pegc.pal_entry],
			m_pegc.b[m_pegc.pal_entry]
		);
		return;
	}

	pc9801rs_a0_w(offset,data);
}

uint8_t pc9821_state::window_bank_r(offs_t offset)
{
	if(offset == 1)
		return m_pc9821_window_bank & 0xfe;

	return 0xff;
}

void pc9821_state::window_bank_w(offs_t offset, uint8_t data)
{
	if(offset == 1)
		m_pc9821_window_bank = data & 0xfe;
	else
		logerror("PC-9821 $f0000 window bank %02x\n",data);
}

/* basically a read-back of various registers */
// bit 1: GDC clock select (port 0x6a, selects with 0x84 & bit 0)
// bit 0: current setting
uint8_t pc9821_state::ext2_video_ff_r()
{
	uint8_t res;

	res = 0;

	switch(m_ext2_ff)
	{
//      case 0x00: ?
//      case 0x01: 200 line color / b&w mode (i/o 0x68 -> 0x02)
//      case 0x02: Odd-numbered raster mask  (i/o 0x68 -> 0x08)
		case 0x03: res = m_video_ff[DISPLAY_REG]; break; // display reg
//      case 0x04: palette mode (i/o 0x6a -> 0x00)
//      case 0x05: GDC sync mode (i/o 0x6a -> 0x40)
//      case 0x06: unknown (i/o 0x6a -> 0x44)
//      case 0x07: EGC compatibility mode (i/o 0x6a -> 0x04)
//      case 0x08: Protected mode f/f (i/o 0x6a -> 0x06)
//      case 0x09: GDC clock #0 (i/o 0x6a -> 0x82)
		case 0x0a: res = m_ex_video_ff[ANALOG_256_MODE]; break; // 256 color mode
//      case 0x0b: VRAM access mode (i/o 0x6a -> 0x62)
//      case 0x0c: unknown
//      case 0x0d: VRAM boundary mode (i/o 0x6a -> 0x68)
//      case 0x0e: 65,536 color GFX mode (i/o 0x6a -> 0x22)
//      case 0x0f: 65,536 color palette mode (i/o 0x6a -> 0x24)
//      case 0x10: unknown (i/o 0x6a -> 0x6a)
//      case 0x11: Reverse mode related (i/o 0x6a -> 0x26)
//      case 0x12: 256 color overscan color (i/o 0x6a -> 0x2c)
//      case 0x13: Reverse mode related (i/o 0x6a -> 0x28)
//      case 0x14: AGDC Drawing processor selection (i/o 0x6a -> 0x66)
//      case 0x15: unknown (i/o 0x6a -> 0x60)
//      case 0x16: unknown (i/o 0x6a -> 0xc2)
//      case 0x17: bitmap config direction (i/o 0x6a -> 0x6c)
//      case 0x18: High speed palette write (i/o 0x6a -> 0x2a)
//      case 0x19: unknown (i/o 0x6a -> 0x48)
//      case 0x1a: unknown (i/o 0x6a -> 0xc8)
//      case 0x1b: unknown (i/o 0x6a -> 0x2e)
//      case 0x1c: unknown (i/o 0x6a -> 0x6e)
//      case 0x1d: unknown (i/o 0x6a -> 0xc0)
//      case 0x1e: unknown (i/o 0x6a -> 0x80 or 0x46?)
//      case 0x1f: unknown (i/o 0x6a -> 0x08)
		default:
			if(m_ext2_ff < 0x20)
				popmessage("PC-9821: read ext2 f/f with value %02x",m_ext2_ff);
			break;
	}

	res |= (m_ex_video_ff[GDC_IS_5MHz] << 1);

	return res;
}

void pc9821_state::ext2_video_ff_w(uint8_t data)
{
	m_ext2_ff = data;
}

// TODO: rename ANALOG_256 refs to pegc
// (it's seemingly the official NEC naming)
uint16_t pc9821_state::pc9821_grcg_gvram_r(offs_t offset, uint16_t mem_mask)
{
	if(m_ex_video_ff[ANALOG_256_MODE])
	{
		u16 *ext_gvram = (u16 *)m_ext_gvram.target();
		int bank = offset >> 14;
		if(bank <= 1)
			return ext_gvram[((m_pegc.bank[bank])*0x4000) + (offset & 0x3fff)];
		return 0xffff;
	}

	return grcg_gvram_r(offset, mem_mask);
}

void pc9821_state::pc9821_grcg_gvram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(m_ex_video_ff[ANALOG_256_MODE])
	{
		u16 *ext_gvram = (u16 *)m_ext_gvram.target();
		int bank = offset >> 14;
		if(bank <= 1)
			COMBINE_DATA(&ext_gvram[((m_pegc.bank[bank])*0x4000) + (offset & 0x3fff)]);
		return;
	}

	grcg_gvram_w(offset,data,mem_mask);
}


void pc9821_state::pc9821_mode_ff_w(u8 data)
{
	const u8 mode_ff = data & 0xfe;
	const u8 setting = BIT(data, 0);
	// Monitor setting
	// BA / BX / PC-H98 / PC-9821 / 98NOTE uses this f/f in place of 15/24 kHz switch
	// TODO: better compose
	if (mode_ff == 0x20)
	{
		if (!setting)
		{
			const XTAL screen_clock = XTAL(21'052'600) / 8;

			m_hgdc[0]->set_unscaled_clock(screen_clock);
			m_hgdc[1]->set_unscaled_clock(screen_clock);
		}
		else
			popmessage("pc9821_mode_ff_w: 31 kHz mode selected");
	}
}

// $e0000 base
void pc9821_state::pegc_mmio_map(address_map &map)
{
	map(0x0004, 0x0004).select(2).lrw8(
		NAME([this] (offs_t offset) {
			return m_pegc.bank[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pegc.bank[offset] = data & 0xf;
		})
	);
	map(0x100, 0x100).lw8(
		NAME([this] (u8 data) {
			m_pegc.packed_mode = bool(BIT(data, 0));
            logerror("$e0100 packed mode %02x\n", data);
		})
	);
//  map(0x102, 0x102) enable pegc linear VRAM at upper addresses
	// $4a0 alias
	map(0x104, 0x104).lw8(
		NAME([this] (u8 data) {
			pc9821_egc_w(0x0 / 2, data, 0x00ff);
		})
	);
	// $4a4 alias
	map(0x108, 0x109).lw16(
		NAME([this] (u16 data, u16 mem_mask) {
			pc9821_egc_w(0x4 / 2, data, mem_mask);
		})
	);
//  map(0x10a, 0x10a) enable color comparator when reading VRAM
	// $4a8 alias (mask)
	// TODO: verify what happens on 32-bit accesses
	map(0x10c, 0x10d).lw16(
		NAME([this] (u16 data, u16 mem_mask) {
			pc9821_egc_w(0x8 / 2, data, mem_mask);
		})
	);
	// $4ae alias (block transfer)
	map(0x110, 0x111).lw16(
		NAME([this] (u16 data, u16 mem_mask) {
			pc9821_egc_w(0xe / 2, data, mem_mask);
		})
	);
	// $4ac alias (shift reg)
	map(0x112, 0x113).lw16(
		NAME([this] (u16 data, u16 mem_mask) {
			pc9821_egc_w(0xc / 2, data, mem_mask);
		})
	);
	// $4a6 alias (foreground color)
	map(0x114, 0x114).lw8(
		NAME([this] (u8 data) {
			pc9821_egc_w(0x6 / 2, data, 0xff);
		})
	);
	// $4aa alias (background color)
	map(0x118, 0x118).lw8(
		NAME([this] (u8 data) {
			pc9821_egc_w(0xa / 2, data, 0xff);
		})
	);
//  map(0x120, 0x19f) pattern register (image_xfer? relates to bit 15 of $108)
}

void pc9821_state::pc9821_map(address_map &map)
{
	pc9801bx2_map(map);
	map(0x000a8000, 0x000bffff).rw(FUNC(pc9821_state::pc9821_grcg_gvram_r), FUNC(pc9821_state::pc9821_grcg_gvram_w));
//  map(0x000cc000, 0x000cffff).rom().region("sound_bios", 0); //sound BIOS
//  map(0x000d8000, 0x000d9fff).rom().region("ide",0)
//  map(0x000da000, 0x000dbfff).ram(); // ide ram
	map(0x000e0000, 0x000e7fff).rw(FUNC(pc9821_state::grcg_gvram0_r), FUNC(pc9821_state::grcg_gvram0_w));
	map(0x000e0000, 0x000e7fff).view(m_pegc_mmio_view);
	m_pegc_mmio_view[0](0x000e0000, 0x000e7fff).m(*this, FUNC(pc9821_state::pegc_mmio_map));
	map(0x00f00000, 0x00f9ffff).ram().share("ext_gvram");
	map(0xfff00000, 0xfff9ffff).ram().share("ext_gvram");
}

void pc9821_state::pc9821_io(address_map &map)
{
	pc9801bx2_io(map);
//  map.unmap_value_high(); // TODO: a read to somewhere makes this to fail at POST
	map(0x0000, 0x001f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask32(0xff00ff00);
	map(0x0000, 0x001f).lr8(NAME([this] (offs_t o) { return BIT(o, 1) ? 0xff : pic_r(o); })).umask32(0x00ff00ff);
	map(0x0000, 0x001f).w(FUNC(pc9821_state::pic_w)).umask32(0x00ff00ff);  // i8259 PIC (bit 3 ON slave / master) / i8237 DMA
	map(0x0020, 0x002f).w(FUNC(pc9821_state::rtc_w)).umask32(0x000000ff);
	map(0x0020, 0x002f).w(FUNC(pc9821_state::dmapg8_w)).umask32(0xff00ff00);
	map(0x0030, 0x0037).rw(m_ppi_sys, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask32(0xff00ff00); //i8251 RS232c / i8255 system port
	map(0x0040, 0x0047).rw(m_ppi_prn, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask32(0x00ff00ff);
	map(0x0040, 0x0047).rw(m_keyb, FUNC(pc9801_kbd_device::rx_r), FUNC(pc9801_kbd_device::tx_w)).umask32(0xff00ff00); //i8255 printer port / i8251 keyboard
//  map(0x0050, 0x0053).w(FUNC(pc9821_state::nmi_ctrl_w)).umask32(0x00ff00ff);
//  map(0x005c, 0x005f).r(FUNC(pc9821_state::timestamp_r)).nopw(); // artic
//  map(0x0060, 0x0063).rw(m_hgdc[0], FUNC(upd7220_device::read), FUNC(upd7220_device::write)).umask32(0x00ff00ff); //upd7220 character ports / <undefined>
//  map(0x0060, 0x0063).r(FUNC(pc9821_state::unk_r)).umask32(0xff00ff00); // mouse related (unmapped checking for AT keyb controller\PS/2 mouse?)
//  map(0x0064, 0x0064).w(FUNC(pc9821_state::vrtc_clear_w));
	map(0x0068, 0x006b).w(FUNC(pc9821_state::pc9821_video_ff_w)).umask32(0x00ff00ff); //mode FF / <undefined>
	map(0x006c, 0x006d).w(FUNC(pc9821_state::border_color_w)).umask16(0x00ff);
	map(0x006e, 0x006f).w(FUNC(pc9821_state::pc9821_mode_ff_w)).umask16(0x00ff);
//  map(0x0070, 0x007f).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask32(0xff00ff00);
//  map(0x0070, 0x007f).rw(FUNC(pc9821_state::grcg_r), FUNC(pc9821_state::grcg_w)).umask32(0x00ff00ff); //display registers "GRCG" / i8253 pit
	map(0x0090, 0x0093).m(m_fdc_2hd, FUNC(upd765a_device::map)).umask32(0x00ff00ff);
	map(0x0094, 0x0094).rw(FUNC(pc9821_state::fdc_2hd_ctrl_r), FUNC(pc9821_state::fdc_2hd_ctrl_w));
	map(0x00a0, 0x00af).rw(FUNC(pc9821_state::pc9821_a0_r), FUNC(pc9821_state::pc9821_a0_w)); //upd7220 bitmap ports / display registers
//  map(0x00b0, 0x00b3) PC9861k (serial port?)
//  map(0x00b9, 0x00b9) PC9861k
//  map(0x00bb, 0x00bb) PC9861k
//  map(0x00bc, 0x00bf).rw(FUNC(pc9821_state::fdc_mode_ctrl_r), FUNC(pc9821_state::fdc_mode_ctrl_w));
	map(0x00c8, 0x00cb).m(m_fdc_2hd, FUNC(upd765a_device::map)).umask32(0x00ff00ff);
//  map(0x00cc, 0x00cc).rw(FUNC(pc9821_state::fdc_2hd_ctrl_r), FUNC(pc9821_state::fdc_2hd_ctrl_w));
	//  map(0x00d8, 0x00df) AMD98 (sound?) board
//  map(0x00f0, 0x00ff).rw(FUNC(pc9821_state::a20_ctrl_r), FUNC(pc9821_state::a20_ctrl_w)).umask32(0x00ff00ff);
//  map(0x0188, 0x018f).rw(FUNC(pc9821_state::pc9801_opn_r), FUNC(pc9821_state::pc9801_opn_w)); //ym2203 opn / <undefined>
//  map(0x018c, 0x018f) YM2203 OPN extended ports / <undefined>
//  map(0x0430, 0x0433).rw(FUNC(pc9821_state::ide_ctrl_r), FUNC(pc9821_state::ide_ctrl_w)).umask32(0x00ff00ff);
//  map(0x0438, 0x043b).rw(FUNC(pc9821_state::access_ctrl_r), FUNC(pc9821_state::access_ctrl_w));
//  map(0x043c, 0x043f).w(FUNC(pc9821_state::pc9801rs_bank_w)); //ROM/RAM bank (NEC)
//  map(0x043c, 0x043f) ROM/RAM bank (EPSON)
	map(0x0460, 0x0463).rw(FUNC(pc9821_state::window_bank_r), FUNC(pc9821_state::window_bank_w));
	map(0x04a0, 0x04af).w(FUNC(pc9821_state::pc9821_egc_w));
//  map(0x04be, 0x04be) FDC "RPM" register
//  map(0x0640, 0x064f).rw(FUNC(pc9821_state::ide_cs0_r), FUNC(pc9821_state::ide_cs0_w));
//  map(0x0740, 0x074f).rw(FUNC(pc9821_state::ide_cs1_r), FUNC(pc9821_state::ide_cs1_w));
//  map(0x08e0, 0x08ea) <undefined> / EMM SIO registers
	map(0x09a0, 0x09a0).rw(FUNC(pc9821_state::ext2_video_ff_r), FUNC(pc9821_state::ext2_video_ff_w)); // GDC extended register r/w
//  map(0x09a8, 0x09a8) GDC 31KHz register r/w
//  map(0x0c07, 0x0c07) EPSON register w
//  map(0x0c03, 0x0c03) EPSON register 0 r
//  map(0x0c13, 0x0c14) EPSON register 1 r
//  map(0x0c24, 0x0c24) cs4231 PCM board register control
//  map(0x0c2b, 0x0c2b) cs4231 PCM board low byte control
//  map(0x0c2d, 0x0c2d) cs4231 PCM board hi byte control
	map(0x0ca0, 0x0ca0).lr8(NAME([] () { return 0xff; })); // high reso detection
//  map(0x0cc0, 0x0cc7) SCSI interface / <undefined>
//  map(0x0cfc, 0x0cff) PCI bus
	map(0x1e8c, 0x1e8f).noprw(); // IDE RAM switch
	map(0x2ed0, 0x2edf).lr8(NAME([] (address_space &s, offs_t o, u8 mm) { return 0xff; })).umask32(0xffffffff); // unknown sound related
//  map(0x3fd8, 0x3fdf).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask32(0xff00ff00); // <undefined> / pit mirror ports
//  map(0x7fd8, 0x7fdf).rw(m_ppi_mouse, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask32(0xff00ff00);
//  map(0x841c, 0x8f1f).rw(FUNC(pc9821_state::sdip_r<0x0>), FUNC(pc9821_state::sdip_w<0x0>));
//  map(0xa460, 0xa46f) cs4231 PCM extended port / <undefined>
	map(0xbfdb, 0xbfdb).rw(FUNC(pc9821_state::mouse_freq_r), FUNC(pc9821_state::mouse_freq_w));
//  map(0xc0d0, 0xc0d3) MIDI port, option 0 / <undefined>
//  map(0xc4d0, 0xc4d3) MIDI port, option 1 / <undefined>
//  map(0xc8d0, 0xc8d3) MIDI port, option 2 / <undefined>
//  map(0xccd0, 0xccd3) MIDI port, option 3 / <undefined>
//  map(0xd0d0, 0xd0d3) MIDI port, option 4 / <undefined>
//  map(0xd4d0, 0xd4d3) MIDI port, option 5 / <undefined>
//  map(0xd8d0, 0xd8d3) MIDI port, option 6 / <undefined>
//  map(0xdcd0, 0xdcd3) MIDI port, option 7 / <undefined>
//  map(0xe0d0, 0xe0d3).r(FUNC(pc9821_state::midi_r)); // MIDI port, option 8 / <undefined>
//  map(0xe4d0, 0xe4d3) MIDI port, option 9 / <undefined>
//  map(0xe8d0, 0xe8d3) MIDI port, option A / <undefined>
//  map(0xecd0, 0xecd3) MIDI port, option B / <undefined>
//  map(0xf0d0, 0xf0d3) MIDI port, option C / <undefined>
//  map(0xf4d0, 0xf4d3) MIDI port, option D / <undefined>
//  map(0xf8d0, 0xf8d3) MIDI port, option E / <undefined>
//  map(0xfcd0, 0xfcd3) MIDI port, option F / <undefined>
}

/*
 * 98MATE A overrides
 */
// TODO: SDIP extended access for 9821Ap, As, Ae
// It never r/w the conventional ports, at least on POST.
// I also suspect a few ports here not being direct RAM r/w but actual regs instead.
u8 pc9821_mate_a_state::ext_sdip_data_r(offs_t offset)
{
	logerror("%s: EXT SDIP access read %02x %02x\n", machine().describe_context(), m_ext_sdip_addr, m_ext_sdip[m_ext_sdip_addr]);
	return m_ext_sdip[m_ext_sdip_addr];
}

void pc9821_mate_a_state::ext_sdip_data_w(offs_t offset, u8 data)
{
	m_ext_sdip[m_ext_sdip_addr] = data;
}

void pc9821_mate_a_state::ext_sdip_access_w(offs_t offset, u8 data)
{
	// access enable?
}

void pc9821_mate_a_state::ext_sdip_address_w(offs_t offset, uint8_t data)
{
	m_ext_sdip_addr = data;
}

void pc9821_mate_a_state::pc9821as_io(address_map &map)
{
	pc9821_io(map);
	map(0x0468, 0x0468).rw(FUNC(pc9821_mate_a_state::ext_sdip_data_r), FUNC(pc9821_mate_a_state::ext_sdip_data_w));
	map(0x046a, 0x046a).w(FUNC(pc9821_mate_a_state::ext_sdip_access_w));
	map(0x046c, 0x046c).w(FUNC(pc9821_mate_a_state::ext_sdip_address_w));
	// TODO: specific MATE A local bus (location?)
}

/*
 * CanBe overrides
 */

/*
 * CanBe Remote control
 * I/O $f4a: remote index (write only?)
 * I/O $f4b: remote data r/w
 * [0x00] <unknown>
 * [0x01] Windows Sound System related
 * ---- xxxx irq select
 * ---- 1000 INT8
 * ---- 0011 INT0
 * ---- 0010 INT41
 * ---- 0000 INT5
 * [0x02] <unknown>
 * [0x03] Remote control reset
 * ---- -x-- (1) Mic through (loopback?)
 * ---- ---x (1) Remote control reset
 * [0x04] Mute control
 * ---- ---x (Global?) sound mute
 * [0x10] Remote control data status
 * ---- --x- (1) device ready (0) busy
 * ---- ---x (1) received data available
 * [0x11] Remote control code
 * <returns the button pressed in 2 bytes form, 2nd byte is the XOR-ed version of 1st>
 * [0x12] <unknown>
 * [0x13] Power control
 * <succession of bytes to power on/off>
 * [0x14] remote irq control
 * ---- -x-- irq enable
 * ---- --xx irq select
 * ---- --11 INT41
 * ---- --10 INT1
 * ---- --01 INT2
 * ---- --00 INT0
 * [0x30] VOL1,2 YMF288 Left sound output
 * [0x31] VOL1,2 YMF288 Right sound output
 * [0x32] VOL3,4 Line Left input
 * [0x33] VOL3,4 Line Right input
 * [0x34] <unknown>
 * [0x35] <unknown>
 */
// TODO: export remote control to an actual device, and pinpoint actual name
void pc9821_canbe_state::remote_addr_w(offs_t offset, u8 data)
{
	m_remote.index = data;
}

u8 pc9821_canbe_state::remote_data_r(offs_t offset)
{
	uint8_t res;

	res = 0;

	logerror("%s: remote control reg read %02x\n", machine().describe_context(), m_remote.index);

	switch(m_remote.index)
	{
		case 0x10:
			res |= 2; // POST will throw "MICOM ERROR" otherwise
			break;
	}
	return res;
}

void pc9821_canbe_state::remote_data_w(offs_t offset, u8 data)
{
	switch(m_remote.index)
	{
		default:
			logerror("%s: remote control reg write %02x %02x\n", machine().describe_context(), m_remote.index, data);
	}
}

void pc9821_canbe_state::pc9821cx3_map(address_map &map)
{
	pc9821_map(map);
	// TODO: overwritten by C-bus mapping, but definitely tested as RAM on POST
	map(0x000c0000, 0x000dffff).ram();
}

void pc9821_canbe_state::pc9821cx3_io(address_map &map)
{
	pc9821_io(map);
	map(0x0f4a, 0x0f4a).w(FUNC(pc9821_canbe_state::remote_addr_w));
	map(0x0f4b, 0x0f4b).rw(FUNC(pc9821_canbe_state::remote_data_r), FUNC(pc9821_canbe_state::remote_data_w));
}

static INPUT_PORTS_START( pc9821 )
	// TODO: verify how many "switches" are really present on pc9821
	// I suspect there are none: if you flip some of these then BIOS keeps throwing
	// "set the sdip" warning until it matches parity odd.
	// They may actually be hardwired defaults that should return constant values depending
	// on machine type.
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Display Type" )
	PORT_DIPSETTING(    0x00, "Normal Display (15KHz)" )
	PORT_DIPSETTING(    0x01, "Hi-Res Display (24KHz)" )
	PORT_DIPNAME( 0x02, 0x00, "DSW1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Monitor Type" )
	PORT_DIPSETTING(    0x04, "RGB" )
	PORT_DIPSETTING(    0x00, "Plasma" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Graphic Function" )
	PORT_DIPSETTING(    0x80, "Basic (8 Colors)" )
	PORT_DIPSETTING(    0x00, "Expanded (16/4096 Colors)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "GDC clock" )
	PORT_DIPSETTING(    0x80, "2.5 MHz" )
	PORT_DIPSETTING(    0x00, "5 MHz" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "FDD Fix Mode" )
	PORT_DIPSETTING(    0x00, "Auto-Detection" )
	PORT_DIPSETTING(    0x01, "Fixed" )
	PORT_DIPNAME( 0x02, 0x02, "FDD Density Select" )
	PORT_DIPSETTING(    0x00, "2DD" )
	PORT_DIPSETTING(    0x02, "2HD" )
	PORT_DIPNAME( 0x04, 0x04, "DSW3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Conventional RAM size" )
	PORT_DIPSETTING(    0x40, "640 KB" )
	PORT_DIPSETTING(    0x00, "512 KB" )
	PORT_DIPNAME( 0x80, 0x00, "CPU Type" )
	PORT_DIPSETTING(    0x80, "V30" )
	PORT_DIPSETTING(    0x00, "I386" )

	// TODO: make a mouse device, not unlike pc9801_epson.cpp
	PORT_START("MOUSE_X")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30)

	PORT_START("MOUSE_Y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30)

	PORT_START("MOUSE_B")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Right Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(MOUSECODE_BUTTON3) PORT_NAME("Mouse Middle Button")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Left Button")

	PORT_START("ROM_LOAD")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x04, 0x04, "Load IDE BIOS" )
	PORT_CONFSETTING(    0x00, DEF_STR( Yes ) )
	PORT_CONFSETTING(    0x04, DEF_STR( No ) )
INPUT_PORTS_END

MACHINE_START_MEMBER(pc9821_state,pc9821)
{
	MACHINE_START_CALL_MEMBER(pc9801bx2);

	// ...
}

MACHINE_START_MEMBER(pc9821_mate_a_state,pc9821ap2)
{
	MACHINE_START_CALL_MEMBER(pc9821);

	// ...
}

MACHINE_START_MEMBER(pc9821_canbe_state,pc9821_canbe)
{
	MACHINE_START_CALL_MEMBER(pc9821);

	// ...
}

MACHINE_RESET_MEMBER(pc9821_state,pc9821)
{
	MACHINE_RESET_CALL_MEMBER(pc9801rs);

	m_pc9821_window_bank = 0x08;
	m_pegc_mmio_view.disable();
}

// TODO: setter for DMAC clock should follow up whatever is the CPU clock

// TODO: remove me, cfr. pc9801.cpp; verify that 9801 clocks are correct for 9821 series as well
#define BASE_CLOCK      XTAL(31'948'800)    // verified for PC-9801RS/FA
#define MAIN_CLOCK_X1   (BASE_CLOCK / 16)   // 1.9968 MHz
#define MAIN_CLOCK_X2   (BASE_CLOCK / 13)   // 2.4576 MHz

void pc9821_state::pc9821(machine_config &config)
{
	// TODO: specs for a vanilla MULTi doesn't match
	// should be 386sx at 20 MHz, this may be "just" a FA/BX class instead
	pc9801rs(config);
	const auto xtal = BASE_CLOCK / 2;
	I486(config.replace(), m_maincpu, xtal); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	m_pit->set_clk<0>(MAIN_CLOCK_X2);
	m_pit->set_clk<1>(MAIN_CLOCK_X2);
	m_pit->set_clk<2>(MAIN_CLOCK_X2);

	m_cbus[0]->set_default_option("pc9801_86");

	MCFG_MACHINE_START_OVERRIDE(pc9821_state, pc9821)
	MCFG_MACHINE_RESET_OVERRIDE(pc9821_state, pc9821)

	m_dmac->set_clock(xtal); // unknown clock

	PALETTE(config.replace(), m_palette, FUNC(pc9821_state::pc9801_palette), 16 + 16 + 256);

//  m_hgdc[1]->set_display_pixels(FUNC(pc9821_state::pegc_display_pixels));
}

void pc9821_mate_a_state::pc9821as(machine_config &config)
{
	pc9821(config);
	const XTAL xtal = XTAL(33'000'000);
	I486(config.replace(), m_maincpu, xtal); // i486dx
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_mate_a_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_mate_a_state::pc9821as_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	// 9821Ap, As, Ae has no dips to port A but reads back written content
	m_ppi_sys->in_pa_callback().set(m_ppi_sys, FUNC(i8255_device::pa_r));

	MCFG_MACHINE_START_OVERRIDE(pc9821_mate_a_state, pc9821ap2)
}

void pc9821_mate_a_state::pc9821ap2(machine_config &config)
{
	pc9821(config);
	const XTAL xtal = XTAL(66'000'000);
	I486(config.replace(), m_maincpu, xtal); // i486dx2
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_mate_a_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_mate_a_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	pit_clock_config(config, xtal / 4); // unknown, fixes timer error at POST

	MCFG_MACHINE_START_OVERRIDE(pc9821_mate_a_state, pc9821ap2)
}

void pc9821_canbe_state::pc9821ce2(machine_config &config)
{
	pc9821(config);
	const XTAL xtal = XTAL(25'000'000);
	I486(config.replace(), m_maincpu, xtal); // i486sx
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_canbe_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_canbe_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	pit_clock_config(config, xtal / 4); // unknown, fixes timer error at POST

	m_cbus[0]->set_default_option("pc9801_118");

	MCFG_MACHINE_START_OVERRIDE(pc9821_canbe_state, pc9821_canbe);
}

void pc9821_canbe_state::pc9821cx3(machine_config &config)
{
	pc9821(config);
	const XTAL xtal = XTAL(100'000'000); // Pentium Pro, 512 kB second cache option RAM
	PENTIUM(config.replace(), m_maincpu, xtal);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_canbe_state::pc9821cx3_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_canbe_state::pc9821cx3_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	pit_clock_config(config, xtal / 4); // unknown, fixes timer error at POST

//  m_cbus[0]->set_default_option(nullptr);
	m_cbus[0]->set_default_option("pc9801_118");

	MCFG_MACHINE_START_OVERRIDE(pc9821_canbe_state, pc9821_canbe);

	// VLSI Supercore594 (Wildcat) PCI 2.0
	// GD5440
	// built-in 3.5 floppy x 1
	// file bay with built-in CD-Rom (4x)
	// HDD with pre-installed software (850MB, 1.2GB)
	// minimum RAM: 16MB
	// maximum RAM: 128MB
	// C-Bus x 3
	// PC-9821CB-B04, on dedicated bus (Fax/Modem 14'400 bps) and IrDA board (115'200 bps)
	// Optional PC-9821C3-B02 MIDI board, on dedicated bus
}

void pc9821_mate_x_state::pc9821xs(machine_config &config)
{
	pc9821(config);
	const XTAL xtal = XTAL(66'000'000);
	I486(config.replace(), m_maincpu, xtal); // i486dx2
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_mate_x_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_mate_x_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));
}

void pc9821_mate_x_state::pc9821xa16(machine_config &config)
{
	pc9821(config);
	PENTIUM(config.replace(), m_maincpu, 166000000); // Pentium P54C
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_mate_x_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_mate_x_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));
}

void pc9821_valuestar_state::pc9821v13(machine_config &config)
{
	pc9821(config);
	const double xtal = 133000000;
	PENTIUM(config.replace(), m_maincpu, xtal); // Pentium Pro, 256kB cache RAM
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_valuestar_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_valuestar_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	// VLSI Supercore594 (Wildcat) / Intel 430FX (Triton) PCI 2.0
	// PCI slot x 1
	// GD5440
	// built-in 3.5 floppy x 1
	// file bay with built-in CD-Rom (4x, 6x, 8x depending on sub-model type)
	// HDD with pre-installed software (850MB, 1.2GB, 1.6GB)
	// minimum RAM: 16MB
	// maximum RAM: 128MB
	// C-Bus x 2
	// PC-9801-120 pre-installed (fax/modem 28'000 bps) or PC-9801-121 (ISDN)
}

void pc9821_valuestar_state::pc9821v20(machine_config &config)
{
	pc9821(config);
	const double xtal = 200000000;
	PENTIUM(config.replace(), m_maincpu, xtal); // Pentium Pro
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_valuestar_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_valuestar_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));
}

void pc9821_mate_r_state::pc9821ra20(machine_config &config)
{
	pc9821(config);
	PENTIUM_PRO(config.replace(), m_maincpu, XTAL(200'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_mate_r_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_mate_r_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));
}

void pc9821_mate_r_state::pc9821ra266(machine_config &config)
{
	pc9821(config);
	const double xtal = 266000000;
	PENTIUM2(config.replace(), m_maincpu, xtal);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_mate_r_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_mate_r_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	// 512KB CPU cache RAM
	// Trident TGUI9682XGi + integrated 98 gfx card
	// 3x cbus + 2x PCI slots
	// 3GB HDD
	// built-in ethernet 100BASE-TX/10BASE-T
}

void pc9821_mate_r_state::pc9821ra333(machine_config &config)
{
	pc9821(config);
	const double xtal = 333000000;
	PENTIUM2(config.replace(), m_maincpu, xtal); // actually a Celeron
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_mate_r_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_mate_r_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	// 128KB CPU cache RAM
	// Trident TGUI9682XGi + integrated 98 gfx card
	// 3x cbus + 2x PCI slots
	// 6GB HDD
	// built-in ethernet 100BASE-TX/10BASE-T
}

// 9821 NOTE machine configs

void pc9821_note_state::pc9821ne(machine_config &config)
{
	pc9821(config);
	const XTAL xtal = XTAL(33'000'000);
	I486(config.replace(), m_maincpu, xtal); // i486sx
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_note_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_note_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	pit_clock_config(config, xtal / 4); // unknown, fixes timer error at POST

	// 9.5 TFT with 640x480x256 mode
	// 1x internal 3.5 floppy
	// PCMCIA2.0/JEIDA 4.1
	// 110-pin expansion bus (?)
	// Ni-Cd battery, around 1 hour of session duration
	// minimum RAM: 3.6MB
	// maximum RAM: 14.6MB
}

void pc9821_note_lavie_state::pc9821nr15(machine_config &config)
{
	pc9821(config);
//  const XTAL xtal = XTAL(150'000'000);
	const double xtal = 150000000;
	PENTIUM_PRO(config.replace(), m_maincpu, xtal); // unsure if normal or pro, clock and cache size suggests latter
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_note_lavie_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_note_lavie_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	// 256KB CPU cache RAM
	// TFT 12.1 screen with 800x600 max resolution
	// Trident Cyber9385 Flat Panel Controller (SVGA, PCI?)
	// -86 board
	// PCI TypeII x 2 (Type III x 1)
	// C-Bus x 1 or ZV port (?)
	// 1GB or 1.4GB HDD
	// 8x/11x CD-ROM
	// Optional FAX
}

void pc9821_note_lavie_state::pc9821nr166(machine_config &config)
{
	pc9821(config);
	const double xtal = 166000000;
	PENTIUM_MMX(config.replace(), m_maincpu, xtal);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_note_lavie_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_note_lavie_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	// 256KB CPU cache RAM
	// TFT 13.3 screen with 1024x768 resolution
	// Trident Cyber9385 Flat Panel Controller (SVGA, PCI?)
	// PCI TypeII x 2 (Type III x 1)
	// C-Bus x 1 or ZV port (?)
	// 3GB HDD
	// Has FAX or ethernet 100BASE-TX/10BASE-T (depending on submodel type)
}

void pc9821_note_lavie_state::pc9821nw150(machine_config &config)
{
	pc9821(config);
	const double xtal = 150000000;
	PENTIUM_MMX(config.replace(), m_maincpu, xtal);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9821_note_lavie_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9821_note_lavie_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	// 256KB CPU cache RAM
	// TFT 12.1 screen with 800x600 resolution & true color
	// Trident Cyber9385-1 Flat Panel Controller (SVGA, PCI?)
	// built-in CD-Rom x16
	// equivalent sound board -86 built-in
	// PCI TypeII x 2 (Type III x 1)
	// C-Bus x 1 or ZV port (?)
	// 2GB HDD (max 31.51GB)
	// has composite video output
	// Has 33.6Kbps FAX
	// Ni-cd (?) battery with 1.5 ~ 2.3 hours of duration
}

/* took from "raw" memory dump */
#define LOAD_IDE_ROM \
	ROM_REGION( 0x4000, "ide", ROMREGION_ERASEVAL(0xcb) ) \
	ROM_LOAD( "d8000.rom", 0x0000, 0x2000, BAD_DUMP CRC(5dda57cc) SHA1(d0dead41c5b763008a4d777aedddce651eb6dcbb) ) \
	ROM_IGNORE( 0x2000 ) \
	ROM_IGNORE( 0x2000 ) \
	ROM_IGNORE( 0x2000 )

// all of these are half size :/
#define LOAD_KANJI_ROMS \
	ROM_REGION( 0x80000, "raw_kanji", ROMREGION_ERASEFF ) \
	ROM_LOAD16_BYTE( "24256c-x01.bin", 0x00000, 0x4000, BAD_DUMP CRC(28ec1375) SHA1(9d8e98e703ce0f483df17c79f7e841c5c5cd1692) ) \
	ROM_CONTINUE(                      0x20000, 0x4000  ) \
	ROM_LOAD16_BYTE( "24256c-x02.bin", 0x00001, 0x4000, BAD_DUMP CRC(90985158) SHA1(78fb106131a3f4eb054e87e00fe4f41193416d65) ) \
	ROM_CONTINUE(                      0x20001, 0x4000  ) \
	ROM_LOAD16_BYTE( "24256c-x03.bin", 0x40000, 0x4000, BAD_DUMP CRC(d4893543) SHA1(eb8c1bee0f694e1e0c145a24152222d4e444e86f) ) \
	ROM_CONTINUE(                      0x60000, 0x4000  ) \
	ROM_LOAD16_BYTE( "24256c-x04.bin", 0x40001, 0x4000, BAD_DUMP CRC(5dec0fc2) SHA1(41000da14d0805ed0801b31eb60623552e50e41c) ) \
	ROM_CONTINUE(                      0x60001, 0x4000  ) \
	ROM_REGION( 0x100000, "kanji", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x80000, "new_chargen", ROMREGION_ERASEFF )

/*
98MATE A - 80486SX 25

(note: might be a different model!)
*/

ROM_START( pc9821 )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf.rom",  0x10000, 0x08000, CRC(dd4c7bb8) SHA1(cf3aa193df2722899066246bccbed03f2e79a74a) )
	ROM_LOAD( "bios.rom", 0x18000, 0x18000, BAD_DUMP CRC(34a19a59) SHA1(2e92346727b0355bc1ec9a7ded1b444a4917f2b9) )
	ROM_FILL(0x24c40, 4, 0) // hide the _32_ marker until we have a 32-bit clean IDE bios otherwise windows tries to
							// make a 32-bit call into 16-bit code
	ROM_FILL(0x27ffe, 1, 0x92)
	ROM_FILL(0x27fff, 1, 0xd7)

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x00000, 0x46800, BAD_DUMP CRC(a61c0649) SHA1(554b87377d176830d21bd03964dc71f8e98676b1) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
As - 80486DX 33
*/

ROM_START( pc9821as )
	ROM_REGION( 0x80000, "biosrom", ROMREGION_ERASEFF )
//  ROM_LOAD( "itf.rom",     0x10000, 0x08000, BAD_DUMP CRC(dd4c7bb8) SHA1(cf3aa193df2722899066246bccbed03f2e79a74a) )
//  ROM_LOAD( "bios_as.rom", 0x18000, 0x18000, BAD_DUMP CRC(0a682b93) SHA1(76a7360502fa0296ea93b4c537174610a834d367) )
	ROM_LOAD( "mvs0100-1.bin", 0x00000, 0x80000, CRC(ca37b631) SHA1(8c481dd0608d6c27235bc88bd77e345628dc28a1) )

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// backported from pc9821ap2
	ROM_COPY( "biosrom", 0x20000, 0x10000, 0x08000 ) // ITF
	ROM_COPY( "biosrom", 0x28000, 0x18000, 0x18000 ) // BIOS

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_as.rom",     0x000000, 0x046800, BAD_DUMP CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
PC-9821AP2/U8W
80486DX2 66MHz
DOS 5.0, Windows 3.1
5.6MB RAM, up to 73.6MB
340MB HD
Expansion slot C-BUS4 (4)
Graphics controller S3 86C928
*/

ROM_START( pc9821ap2 )
	ROM_REGION( 0x80000, "biosrom", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("phd0104")
	ROM_SYSTEM_BIOS(0, "phd0104",  "PHD0104")
	ROMX_LOAD( "phd0104.rom",     0x000000, 0x80000, CRC(da73b372) SHA1(2c15b63a0869b81ef7f04972dbb0975f4e77d384), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "phd0102",  "PHD0102")
	ROMX_LOAD( "phd0102.rom",     0x000000, 0x80000, CRC(3036774c) SHA1(59856a348f156adf5eca06326f967aca54ff871c), ROM_BIOS(1) )

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// 0x00000-0x04ff0 <unknown>
	// 0x0c000-0x0ffff internal sound BIOS?
	// 0x10000-0x13fff ^ mirror of above?
	// 0x14000-0x14ff0 <unknown>
	// 0x16000-0x19fff contains refs to 765 and HDDs "Conner Peripherals", IDE BIOS?
	// 0x1c000-0x1ffff contains refs to SDIP setup
	ROM_COPY( "biosrom", 0x20000, 0x10000, 0x08000 ) // ITF
	ROM_COPY( "biosrom", 0x28000, 0x18000, 0x18000 ) // BIOS
	// 0x40000-0x4ffff empty
	// 0x50000-0x57fff extended BIOS check?
	// 0x58000-0x65fff empty
	// 0x66000-0x77fff mirrors of IDE or sound BIOS (left-overs?)
	// 0x78000-0x7ffff Has a (c) 1986, more left-over?

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x00000, 0x46800, BAD_DUMP CRC(a61c0649) SHA1(554b87377d176830d21bd03964dc71f8e98676b1) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END


/*
98NOTE - i486SX 33
*/

ROM_START( pc9821ne )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf.rom",     0x10000, 0x08000, BAD_DUMP CRC(dd4c7bb8) SHA1(cf3aa193df2722899066246bccbed03f2e79a74a) )
	ROM_LOAD( "bios_ne.rom", 0x18000, 0x18000, BAD_DUMP CRC(2ae070c4) SHA1(d7963942042bfd84ed5fc9b7ba8f1c327c094172) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_ne.rom", 0x00000, 0x46800, BAD_DUMP CRC(fb213757) SHA1(61525826d62fb6e99377b23812faefa291d78c2e) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
98MULTi Ce2 - 80486SX 25
*/

ROM_START( pc9821ce2 )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf_ce2.rom",  0x10000, 0x008000, CRC(273e9e88) SHA1(9bca7d5116788776ed0f297bccb4dfc485379b41) )
	ROM_LOAD( "bios_ce2.rom", 0x18000, 0x018000, BAD_DUMP CRC(76affd90) SHA1(910fae6763c0cd59b3957b6cde479c72e21f33c1) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_ce2.rom", 0x00000, 0x046800, BAD_DUMP CRC(d1c2702a) SHA1(e7781e9d35b6511d12631641d029ad2ba3f7daef) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
PC-9821CX3

Pentium @ 100 MHz
16MB, max 128 MB
3.5x1, 4xCD-ROM
CL GD5440
PCI VLSI Supercore594 (Wildcat), PCI Rev. 2.0
*/

ROM_START( pc9821cx3 )
	ROM_REGION16_LE( 0x80000, "biosrom", ROMREGION_ERASEFF )
	// ROM BIOS rev. 0.13
	ROM_LOAD( "pc-9821cx3.bin", 0x000000, 0x080000, CRC(0360ed78) SHA1(2e4fe059d001d980add1656816bda97cd11ef331) )

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// TODO: all of the 512k space seems valid
	// all GFXs are 1bpp packed with different pitches
	// 0 - 0x56xx: CanBe mascot GFX animations
	// 0x1fda8 (?) - 0x2458f: monitor GFXs
	// 0x24590 - 0x36xxx: more CanBe mascot GFX animations
	// 0x3c000: NEC & CanBe logo GFXs
	ROM_COPY( "biosrom", 0x68000, 0x00000, 0x18000 )
	ROM_COPY( "biosrom", 0x30000, 0x18000, 0x18000 )

	// "microcode" memory dump, probably identical to above but shuffled
	// left for consultation
	ROM_REGION16_LE( 0x80000, "memory", ROMREGION_ERASEFF )
	ROM_LOAD( "bank0.bin",    0x000000, 0x008000, CRC(bccc5233) SHA1(1246203bebf9f04e3bac2df7fc64719304f9f1bd) )
	ROM_LOAD( "bank1.bin",    0x008000, 0x008000, CRC(190b448b) SHA1(01d555cb1044ea280759c5fb724b24ca38ba67f7) )
	ROM_LOAD( "bank2.bin",    0x010000, 0x008000, CRC(2a73461c) SHA1(745d48a33766b7f4dab84faa87014fd6f4c8ce34) )
	ROM_LOAD( "bank3.bin",    0x018000, 0x008000, CRC(13ffa799) SHA1(dba02689de02d4c2d6bbf10efe86a37653f3aa86) )
	ROM_LOAD( "bank4.bin",    0x020000, 0x008000, CRC(b5fa9408) SHA1(fd94a0da767b5de2c10da154ae336c14a1d70e4f) )
	ROM_LOAD( "bank5.bin",    0x028000, 0x008000, CRC(4e32081e) SHA1(e23571273b7cad01aa116cb7414c5115a1093f85) )
	ROM_LOAD( "bank6.bin",    0x030000, 0x008000, CRC(1021ccca) SHA1(2154b03c8650e66700a9ea83f1eb2bb32a2bad46) )
	ROM_LOAD( "bank7.bin",    0x038000, 0x008000, CRC(d339d36f) SHA1(fdb5e2d8bfcc723a86d4706c7c00e3adb1bc421b) )
	ROM_LOAD( "bank8.bin",    0x040000, 0x008000, CRC(41263c3a) SHA1(fa415639882b266e663f36b22a8f6336258b7f93) )
	ROM_LOAD( "bank9.bin",    0x048000, 0x008000, CRC(28ccca78) SHA1(44b10f0cfae71b34221306db5d93d5d5aaec3cd3) )
	ROM_LOAD( "banka.bin",    0x050000, 0x008000, CRC(1b43eabd) SHA1(ca711c69165e1fa5be72993b9a7870ef6d485249) )
	ROM_LOAD( "bankb.bin",    0x058000, 0x008000, CRC(2ec7b657) SHA1(34b9ba199b13c71d4facc4215931574d59afd928) )
	ROM_LOAD( "bankc.bin",    0x060000, 0x008000, CRC(b3559c78) SHA1(d8f177b75e96ad22393933534b0432bc0018eae5) )
	ROM_LOAD( "bankd.bin",    0x068000, 0x008000, CRC(9bc44372) SHA1(baa917086edd578d88b730ab1dca1899beb0525d) )
	ROM_LOAD( "banke.bin",    0x070000, 0x008000, CRC(911003f4) SHA1(a35b72130b2c3c3822a1648d1f470a6973262c73) )
	ROM_LOAD( "bankf.bin",    0x078000, 0x008000, CRC(77f6a5c7) SHA1(b8fbf104dd8a8e00855be18f0c3b71da79b6a841) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_ce2.rom", 0x00000, 0x046800, BAD_DUMP CRC(d1c2702a) SHA1(e7781e9d35b6511d12631641d029ad2ba3f7daef) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
98MATE X - 486/Pentium based
*/

ROM_START( pc9821xs )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// "ROM SUM ERROR"
	ROM_LOAD( "itf.rom",         0x10000, 0x008000, BAD_DUMP CRC(dd4c7bb8) SHA1(cf3aa193df2722899066246bccbed03f2e79a74a) )
	ROM_LOAD( "bios_xs.rom",     0x18000, 0x018000, BAD_DUMP CRC(0a682b93) SHA1(76a7360502fa0296ea93b4c537174610a834d367) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_xs.rom",     0x00000, 0x046800, BAD_DUMP CRC(c9a77d8f) SHA1(deb8563712eb2a634a157289838b95098ba0c7f2) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END


/*
9821Xa16

Pentium P54C @ 166
32MB
3.5"2DD/2HDx1, 8xCD-ROM
CBus: 3 slots

*/

ROM_START( pc9821xa16 )
	ROM_REGION16_LE( 0x40000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD( "pc-9821xa16_g8yewa_a1_wsg8b01_ab28f200b5-t.bin", 0x00000, 0x040000, CRC(f99c8ce2) SHA1(2bc328d2c496046f6f4f39b0637e90b713a63155) ) // SOP44

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// TODO: all of the 256k space seems valid
	ROM_COPY( "biosrom", 0x28000, 0x00000, 0x18000 )
	ROM_COPY( "biosrom", 0x00000, 0x18000, 0x18000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x00000, 0x46800, BAD_DUMP CRC(a61c0649) SHA1(554b87377d176830d21bd03964dc71f8e98676b1) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
PC-9821Ra20 (98MATE R)

Pentium Pro @ 200
32MB
3.5"2DD/2HDx1, 8xCD-ROM
CBus: 3 slots
*/

ROM_START( pc9821ra20 )
	ROM_REGION16_LE( 0x40000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD( "g8wtp_a13_wtp8b01_ab28f200b5-t.bin", 0x00000, 0x040000, CRC(cd3acc5c) SHA1(746490d7f3d8d0e8df865315adaaae65f3fd0425) ) // SOP44

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// TODO: all of the 256k space seems valid
	ROM_COPY( "biosrom", 0x28000, 0x00000, 0x18000 )
	ROM_COPY( "biosrom", 0x00000, 0x18000, 0x18000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x00000, 0x46800, BAD_DUMP CRC(a61c0649) SHA1(554b87377d176830d21bd03964dc71f8e98676b1) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
PC-9821Ra266

Pentium II @ 266 MHz
Trident TGUI9682XGi
32MB, max 256 MB (ECC EDO RAM)
3.5x1, 16xCD-ROM
CBus: 3 slots, PCI: 2 slots
PCI Intel 440FX (Natoma), PCI Rev. 2.1

PC-9821Ra266/M30R has been re-released in 1998,
unknown differences other than having Win98 pre-installed
*/

ROM_START( pc9821ra266 )
	ROM_REGION16_LE( 0x40000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD( "g8ykkw.bin", 0x000000, 0x040000, CRC(d73a2795) SHA1(65d4e1e438e91c1646bcc06a9868aa474faf0ccf) ) // SOP44

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// TODO: all of the 256k space seems valid
	ROM_COPY( "biosrom", 0x28000, 0x00000, 0x18000 )
	ROM_COPY( "biosrom", 0x00000, 0x18000, 0x18000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x00000, 0x46800, BAD_DUMP CRC(a61c0649) SHA1(554b87377d176830d21bd03964dc71f8e98676b1) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
PC-9821Ra333 (98MATE R)

Celeron @ 333
32MB, max 256 MB (ECC EDO RAM)
3.5x1, 24xCD-ROM
CBus: 3 slots, PCI: 2 slots
*/

ROM_START( pc9821ra333 )
	ROM_REGION16_LE( 0x40000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD( "g8ykkw.bin", 0x00000, 0x040000, CRC(c605ef31) SHA1(3779aed757f21eb75093c1bfcbf18a232c198ee6) )

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// TODO: all of the 256k space seems valid
	ROM_COPY( "biosrom", 0x28000, 0x00000, 0x18000 )
	ROM_COPY( "biosrom", 0x00000, 0x18000, 0x18000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x00000, 0x46800, BAD_DUMP CRC(a61c0649) SHA1(554b87377d176830d21bd03964dc71f8e98676b1) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END


/*
98MATE VALUESTAR - Pentium based
*/

ROM_START( pc9821v13 )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// "ROM SUM ERROR"
	ROM_LOAD( "itf.rom",      0x10000, 0x08000, BAD_DUMP CRC(dd4c7bb8) SHA1(cf3aa193df2722899066246bccbed03f2e79a74a) )
//  ROM_LOAD( "itf_v20.rom",  0x10000, 0x08000, BAD_DUMP CRC(10e52302) SHA1(f95b8648e3f5a23e507a9fbda8ab2e317d8e5151) )
	ROM_LOAD( "bios_v13.rom", 0x18000, 0x18000, BAD_DUMP CRC(0a682b93) SHA1(76a7360502fa0296ea93b4c537174610a834d367) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_v13.rom",   0x00000, 0x46800, BAD_DUMP CRC(c9a77d8f) SHA1(deb8563712eb2a634a157289838b95098ba0c7f2) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM

	// TODO: factory HDDs
ROM_END

/*
98MATE VALUESTAR - Pentium based
*/

ROM_START( pc9821v20 )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// Doesn't boot, not an ITF ROM!
	ROM_LOAD( "itf.rom",      0x10000, 0x08000, BAD_DUMP CRC(dd4c7bb8) SHA1(cf3aa193df2722899066246bccbed03f2e79a74a) )
//  ROM_LOAD( "itf_v20.rom",  0x10000, 0x08000, CRC(10e52302) SHA1(f95b8648e3f5a23e507a9fbda8ab2e317d8e5151) )
	ROM_LOAD( "bios_v20.rom", 0x18000, 0x18000, BAD_DUMP CRC(d5d1f13b) SHA1(bf44b5f4e138e036f1b848d6616fbd41b5549764) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_v20.rom", 0x00000, 0x046800, BAD_DUMP CRC(6244c4c0) SHA1(9513cac321e89b4edb067b30e9ecb1adae7e7be7) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM

	// TODO: factory HDDs
ROM_END

/*
PC-9821Nr15

Pentium [Pro?] @ 150 MHz
16MB, max 128 MB
3.5x1, 8xCD-ROM
Trident Cyber9385
TFT 12.1 inches @ 800x600

Nr15/S14F has:
11xCD-ROM
FAX
*/

ROM_START( pc9821nr15 )
	ROM_REGION16_LE( 0x40000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD( "g8xwa_xwa00_ab28f200bx-t.bin", 0x000000, 0x040000, CRC(17b91850) SHA1(755eb8767c08980d0f1b6e32638e9fdd616a1b26) ) // SOP44

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// TODO: all of the 256k space seems valid
	ROM_COPY( "biosrom", 0x28000, 0x00000, 0x18000 )
	ROM_COPY( "biosrom", 0x00000, 0x18000, 0x18000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x00000, 0x46800, BAD_DUMP CRC(a61c0649) SHA1(554b87377d176830d21bd03964dc71f8e98676b1) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
PC-9821Nr166

Pentium MMX @ 166 MHz
32MB, max 128 MB
3.5x1, 11xCD-ROM
Trident Cyber9385
TFT 13.3 inches, 1024x768
Optional FAX or LAN (depending on model sub-type)
*/

ROM_START( pc9821nr166 )
	ROM_REGION16_LE( 0x40000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD( "pc-9821nr166.bin", 0x000000, 0x040000, CRC(6a137f51) SHA1(1b8264ff525cfda5b367bf85570ff53a6ad42cd4) )

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// TODO: all of the 256k space seems valid
	ROM_COPY( "biosrom", 0x28000, 0x00000, 0x18000 )
	ROM_COPY( "biosrom", 0x00000, 0x18000, 0x18000 )

	// "microcode" memory dump, probably identical to above but shuffled
	// left for consultation
	ROM_REGION16_LE( 0x40000, "memory", ROMREGION_ERASEFF )
	ROM_LOAD( "bank0.bin",    0x000000, 0x008000, CRC(c08b4f76) SHA1(bc352826e33566b0e9f5e54c391d7690b6d8fff0) )
	ROM_LOAD( "bank1.bin",    0x008000, 0x008000, CRC(62dfab8b) SHA1(e95cd8e6f385a8074fc311dcfc982b42ce1f1a7c) )
	ROM_LOAD( "bank2.bin",    0x010000, 0x008000, CRC(1df9ebcb) SHA1(91db85ac60ab7c9100aa95c945f50564f4933776) )
	ROM_LOAD( "bank3.bin",    0x018000, 0x008000, CRC(e2b44219) SHA1(f02449f37a2b3d7a22551c3c4fd018d426059829) )
	ROM_LOAD( "bank4.bin",    0x020000, 0x008000, CRC(507d66df) SHA1(f29434ce472ea49b87a17f195fef7d31b7f9ba67) )
	ROM_LOAD( "bank5.bin",    0x028000, 0x008000, CRC(4e32081e) SHA1(e23571273b7cad01aa116cb7414c5115a1093f85) )
	ROM_LOAD( "bank6.bin",    0x030000, 0x008000, CRC(c0f0495b) SHA1(5fd9db08f61faadc8d5b004e41005f113c480ee5) )
	ROM_LOAD( "bank7.bin",    0x038000, 0x008000, CRC(cf92cf6b) SHA1(880350ce71fcf5a039155062d6065566f0c8fa46) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x00000, 0x46800, BAD_DUMP CRC(a61c0649) SHA1(554b87377d176830d21bd03964dc71f8e98676b1) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

ROM_START( pc9821nw150 )
	ROM_REGION16_LE( 0x40000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD( "g8yxb_b7a_yxb00_ab28f200bx-t.bin", 0x000000, 0x040000, CRC(75f547f6) SHA1(5ff610f3796a3742b674151e4e5a750ded48c951) ) // SOP44

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// TODO: all of the 256k space seems valid
	ROM_COPY( "biosrom", 0x28000, 0x00000, 0x18000 )
	ROM_COPY( "biosrom", 0x00000, 0x18000, 0x18000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x00000, 0x46800, BAD_DUMP CRC(a61c0649) SHA1(554b87377d176830d21bd03964dc71f8e98676b1) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END


// PC9821 [desktop] class
// NB: several of these sub-models don't have the final letter in the original flyers but present in later docs as an afterthought.
// To mark this we intentionally add square brackets here as optional omission notation.

// 98MULTi (i386, desktop)
COMP( 1992, pc9821,      0,          0, pc9821,        pc9821,    pc9821_state,        init_pc9801_kanji,   "NEC",   "PC-9821 (98MULTi)",             MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// 98MATE [A] (i486, desktop, has 98 MATE local bus "ML", with optional RL-like high-reso)
COMP( 1993, pc9821as,    0,          0, pc9821as,      pc9821,    pc9821_mate_a_state, init_pc9801_kanji,   "NEC",   "PC-9821As (98MATE A)",          MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1993, pc9821ap2,   pc9821as,   0, pc9821ap2,     pc9821,    pc9821_mate_a_state, init_pc9801_kanji,   "NEC",   "PC-9821Ap2/U8W (98MATE A)",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// SC-9821A (rebranded MATE A machines with minor differences such as SW power control)
// ...

// 98MATE [B] (i486, desktop, has GD5428 and no built-in sound)
// ...

// 98MULTi CanBe (i486/Pentium, desktop & tower, Multimedia PC with optional TV Tuner & remote control function, Fax, Modem, MPEG-2, FX-98IF for PC-FX compatibility etc. etc.)
COMP( 1994, pc9821ce2,   0,           0, pc9821ce2,    pc9821,   pc9821_canbe_state, init_pc9801_kanji,   "NEC",   "PC-9821Ce2 (98MULTi CanBe)",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1995, pc9821cx3,   pc9821ce2,   0, pc9821cx3,    pc9821,   pc9821_canbe_state, init_pc9801_kanji,   "NEC",   "PC-9821Cx3 (98MULTi CanBe)",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// 98MULTi CanBe Jam (Pentium Pro equipped, laptop, Multimedia PC as above + JEIDA 4.2/PCMCIA 2.1)
// ...

// CEREB (Pentium MMX based CanBe follow-up. Multimedia PC with a Sony DDU100E DVD-ROM drive as most notable addition.
//        Form factor is way more akin of a DVD player than a PC)
// ...

// 98MATE X (i486sx/Pentium/Pentium Pro, has PCI, comes with various SVGA cards)
COMP( 1994, pc9821xs,    0,           0, pc9821xs,     pc9821,   pc9821_mate_x_state, init_pc9801_kanji,   "NEC",   "PC-9821Xs (98MATE X)",          MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1996, pc9821xa16,  pc9821xs,    0, pc9821xa16,   pc9821,   pc9821_mate_x_state, init_pc9801_kanji,   "NEC",   "PC-9821Xa16 (98MATE X)",        MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// 98MATE VALUESTAR (Pentium, comes with Windows 95 and several programs pre-installed)
COMP( 1998, pc9821v13,   0,           0, pc9821v13,    pc9821,   pc9821_valuestar_state, init_pc9801_kanji,   "NEC",   "PC-9821V13 (98MATE VALUESTAR)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1998, pc9821v20,   pc9821v13,   0, pc9821v20,    pc9821,   pc9821_valuestar_state, init_pc9801_kanji,   "NEC",   "PC-9821V20 (98MATE VALUESTAR)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// 98MATE R (Pentium Pro, otherwise same as 98MATE X?)
COMP( 1996, pc9821ra20,  0,            0, pc9821ra20,  pc9821,   pc9821_mate_r_state, init_pc9801_kanji,   "NEC",   "PC-9821Ra20 (98MATE R)",        MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1997, pc9821ra266, pc9821ra20,   0, pc9821ra266, pc9821,   pc9821_mate_r_state, init_pc9801_kanji,   "NEC",   "PC-9821Ra266 (98MATE R)",       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1998, pc9821ra333, pc9821ra20,   0, pc9821ra333, pc9821,   pc9821_mate_r_state, init_pc9801_kanji,   "NEC",   "PC-9821Ra333 (98MATE R)",       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// 98MATE SERVER, pc9821rs* (Server variant of 98MATE R. Inherits concepts from SV-H98 98SERVER)
// ...

// PC-9821 NOTE[book] class
// 98NOTE
COMP( 1994, pc9821ne,    0,            0, pc9821ne,    pc9821,   pc9821_note_state,       init_pc9801_kanji,   "NEC",   "PC-9821Ne (98NOTE)",              MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// 98NOTE Lavie
COMP( 1996, pc9821nr15,  0,            0, pc9821nr15,  pc9821,   pc9821_note_lavie_state, init_pc9801_kanji,   "NEC",   "PC-9821Nr15 (98NOTE Lavie)",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1997, pc9821nr166, pc9821nr15,   0, pc9821nr166, pc9821,   pc9821_note_lavie_state, init_pc9801_kanji,   "NEC",   "PC-9821Nr166 (98NOTE Lavie)",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1997, pc9821nw150, pc9821nr15,   0, pc9821nw150, pc9821,   pc9821_note_lavie_state, init_pc9801_kanji,   "NEC",   "PC-9821Nw150 (98NOTE Lavie)",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// 98NOTE Light
// ...

// 98NOTE Aile
// ...

// 98FiNE
// ...
