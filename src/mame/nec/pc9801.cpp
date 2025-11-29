// license:BSD-3-Clause
// copyright-holders:Angelo Salese,Carl
/**************************************************************************************************

PC-9801 (c) 1981 NEC

References:
- https://www.pc-9800.net/index.htm
- https://www.webtech.co.jp/company/doc/undocumented_mem/index.html
- https://web.archive.org/web/20190331142002/http://www.geocities.jp/retro_zzz/machines/nec/9801/mdl98cpu.html

TODO:
- C-Bus SCSI support, remove IDE ROM loads where doesn't belong by default;
\- load actual IDE BIOSes from IPL romsets where applicable (pc9801bx onward, all pc9821)
- Port over pc88va SASI version in common C-Bus option;
- Remove kludge for POR bit in a20_ctrl_w fn;
\- Causes "SYSTEM SHUTDOWN"s on OS installs/reboots (soft reset the machine manually);
- CMT support (-03/-13/-36 i/f or cbus only, supported by i86/V30 fully compatible machines
  only);
- DAC1BIT has a bit of clicking with start/end of samples, is it fixable or just a btanb?
- Incomplete FDC inner semantics with the dual ports;
\- floppy sounds never silences when drive is idle (disabled for the time being);
\- epdiag: throws ID invalid when run with auto-detect 2HD/2DD mode (DIP-SW 3-1 -> 0);
- Export mouse support to an actual PC9871 device;
- GP-IB emulation, μPD7210;
- Per-system dip-switches/configurations;
- Disable EGC use where it's not mounted normally (test thru dbuster and hypbingo)

TODO (pc9801/pc9801f):
- Move vanilla FDC 2HD/2DD to a separate (legacy?) bus, and split pc9801f (default: 2DD)
  from pc9801m (2HD) and vanilla pc9801 (none);
- it currently hooks up half size kanji ROMs, causing missing text in many games;

TODO (pc9801uv2):
- Not enough software for the specific 3.5" 2DD format;
- Verify dipswitches (needs 2DD fixed mode for sure)

TODO (pc9801rs):
- Remove IDE hack to not make 512 to 256 sector byte translations
\- probably need a working C-Bus IDE in place, or a SCSI option;

TODO (pc9801us / pc9801fs):
- "Invalid Command Byte 13" for bitmap upd7220 at POST (?)
- RAM check detects more RAM than what's really installed (and saves previous detection in MEMSW);
- pc9801fs: Crashes with Japanese error for "HDD failure" when mounted with IDE BIOS
\- wants specifically (the internal) SCSI?

TODO (pc9801bx2):
- "SYSTEM SHUTDOWN" at POST, SDIP related, soft reset to bypass;
- Accesses $8f0-$8f2 PMC area, shared with 98NOTE machines;
- A non-fatal "MEMORY ERROR" is always thrown no matter the RAM size afterwards, related?
- unemulated conventional or EMS RAM bank, definitely should have one given the odd minimum RAM
  size;

TODO (pc9801ux):
- "I/O Error" on any 3.5" floppy, specific to this romset (i.e. those works on pc9821).
  It never access $4be, not a 3-mode floppy or needs some specific setup?

**************************************************************************************************/

#include "emu.h"
#include "pc9801.h"

#include "bus/nec_fdd/pc80s31k.h"
#include "bus/pc98_54simm/options.h"
#include "bus/pc98_54simm/slot.h"
#include "bus/pc98_61simm/options.h"
#include "bus/pc98_61simm/slot.h"

#include "bus/pc98_cbus/options.h"

#include "machine/input_merger.h"

void pc98_base_state::rtc_w(uint8_t data)
{
	m_rtc->c0_w(BIT(data, 0));
	m_rtc->c1_w(BIT(data, 1));
	m_rtc->c2_w(BIT(data, 2));
	m_rtc->stb_w(BIT(data, 3));
	m_rtc->clk_w(BIT(data, 4));
	m_rtc->data_in_w(BIT(data, 5));
	if(data & 0xc0)
		logerror("RTC write to undefined bits %02x\n",data & 0xc0);
}

void pc9801_state::dmapg4_w(offs_t offset, uint8_t data)
{
	if(offset < 4)
		m_dma_offset[(offset+1) & 3] = data & 0x0f;
}

void pc9801vm_state::dmapg8_w(offs_t offset, uint8_t data)
{
	if(offset == 4)
		m_dma_autoinc[data & 3] = (data >> 2) & 3;
	else if(offset < 4)
		m_dma_offset[(offset+1) & 3] = data;
}

void pc9801_state::nmi_ctrl_w(offs_t offset, uint8_t data)
{
	m_nmi_ff = offset;
}

void pc9801_state::vrtc_clear_w(uint8_t data)
{
	m_pic1->ir2_w(0);
}

u8 pc9801_state::fdc_2hd_ctrl_r()
{
	return 0x44;
}

void pc9801_state::fdc_2hd_ctrl_w(u8 data)
{
	//logerror("%02x ctrl\n",data);
	m_fdc_2hd->reset_w(BIT(data, 7));

	m_fdc_2hd_ctrl = data;
	if(data & 0x40)
	{
		m_fdc_2hd->set_ready_line_connected(0);
		m_fdc_2hd->ready_w(0);
	}
	else
		m_fdc_2hd->set_ready_line_connected(1);

	if(!m_sys_type) // required for 9801f 2hd adapter bios
	{
		m_fdc_2hd->subdevice<floppy_connector>("0")->get_device()->mon_w(data & 8 ? ASSERT_LINE : CLEAR_LINE);
		m_fdc_2hd->subdevice<floppy_connector>("1")->get_device()->mon_w(data & 8 ? ASSERT_LINE : CLEAR_LINE);
	}
}

bool pc9801_state::fdc_drive_ready_r(upd765a_device *fdc)
{
	floppy_image_device *floppy0 = fdc->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = fdc->subdevice<floppy_connector>("1")->get_device();

	return (!floppy0->ready_r() || !floppy1->ready_r());
}

u8 pc9801vm_state::ide_ctrl_hack_r()
{
	if (!machine().side_effects_disabled())
	{
		// HACK: RS IDE driver will try to do 512 to 256 byte sector translations
		// Never initializes buffer at $457, and MEMSW has no setting for this.

		// SDIP based machines don't need this (they will default to 512 bps, shadowed from
		// gaiji $ac403 bit 6).
		address_space &ram = m_maincpu->space(AS_PROGRAM);
		ram.write_byte(0x457, ram.read_byte(0x457) | 0xc0);
	}
	return m_ide_sel;
}

u8 pc9801vm_state::ide_ctrl_r()
{
	return m_ide_sel;
}

void pc9801vm_state::ide_ctrl_w(u8 data)
{
	if(!(data & 0x80))
		m_ide_sel = data & 1;
}

uint16_t pc9801vm_state::ide_cs0_r(offs_t offset, uint16_t mem_mask)
{
	return m_ide[m_ide_sel]->cs0_r(offset, mem_mask);
}

void pc9801vm_state::ide_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ide[m_ide_sel]->cs0_w(offset, data, mem_mask);
}

uint16_t pc9801vm_state::ide_cs1_r(offs_t offset, uint16_t mem_mask)
{
	return m_ide[m_ide_sel]->cs1_r(offset, mem_mask);
}

void pc9801vm_state::ide_cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ide[m_ide_sel]->cs1_w(offset, data, mem_mask);
}

void pc9801_state::pc9801_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x9ffff).rw("cbus_root", FUNC(pc98_cbus_root_device::mem_r), FUNC(pc98_cbus_root_device::mem_w));
	map(0xa0000, 0xa3fff).rw(FUNC(pc9801_state::tvram_r), FUNC(pc9801_state::tvram_w)); //TVRAM
	map(0xa8000, 0xbffff).rw(FUNC(pc9801_state::gvram_r), FUNC(pc9801_state::gvram_w)); //bitmap VRAM
	map(0xc0000, 0xdffff).rw("cbus_root", FUNC(pc98_cbus_root_device::mem_slot_r), FUNC(pc98_cbus_root_device::mem_slot_w));
//  map(0xcc000, 0xcffff).rom().region("sound_bios", 0); //sound BIOS
//  map(0xd6000, 0xd6fff).rom().region("fdc_bios_2dd", 0); //floppy BIOS 2dd
//  map(0xd7000, 0xd7fff).rom().region("fdc_bios_2hd", 0); //floppy BIOS 2hd
	map(0xe8000, 0xfffff).rom().region("ipl", 0);
}

/* first device is even offsets, second one is odd offsets */
void pc9801_state::pc9801_common_io(address_map &map)
{
//  map.unmap_value_high();
	map(0x0000, 0x001f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0xff00);
	map(0x0000, 0x001f).rw(FUNC(pc9801_state::pic_r), FUNC(pc9801_state::pic_w)).umask16(0x00ff); // i8259 PIC (bit 3 ON slave / master) / i8237 DMA
	map(0x0020, 0x0020).w(FUNC(pc9801_state::rtc_w));
	map(0x0030, 0x0037).rw(m_ppi_sys, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff00);
	map(0x0030, 0x0033).rw(m_sio_rs, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff); //i8251 RS232c / i8255 system port
	map(0x0040, 0x0047).rw(m_ppi_prn, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0040, 0x0043).rw(m_sio_kbd, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0xff00); //i8255 printer port / i8251 keyboard
	map(0x0050, 0x0053).w(FUNC(pc9801_state::nmi_ctrl_w)).umask16(0x00ff); // NMI FF / host FDD 2d (PC-80S31K)
	map(0x0060, 0x0063).rw(m_hgdc[0], FUNC(upd7220_device::read), FUNC(upd7220_device::write)).umask16(0x00ff); //upd7220 character ports / <undefined>
	map(0x0064, 0x0064).w(FUNC(pc9801_state::vrtc_clear_w));
//  map(0x006c, 0x006f) border color / <undefined>
	map(0x006c, 0x006f).w(FUNC(pc9801_state::border_color_w)).umask16(0x00ff);
	// TODO: PC-98Bible suggests that $73 timer #1 is unavailable on non-vanilla models (verify on HW)
	// (can be accessed only thru the $3fdb alias)
	map(0x0070, 0x0077).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0xff00);
	map(0x0070, 0x007f).rw(FUNC(pc9801_state::txt_scrl_r), FUNC(pc9801_state::txt_scrl_w)).umask16(0x00ff); //display registers / i8253 pit
//  map(0x0090, 0x0093).rw(m_sio_cmt, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0xff00); // CMT SIO (optional, C-Bus)
	map(0x7fd8, 0x7fdf).rw(m_ppi_mouse, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff00);
}

void pc9801_state::pc9801_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw("cbus_root", FUNC(pc98_cbus_root_device::io_r), FUNC(pc98_cbus_root_device::io_w));
	pc9801_common_io(map);
	map(0x0020, 0x002f).w(FUNC(pc9801_state::dmapg4_w)).umask16(0xff00);
	map(0x0050, 0x0057).m("fdd_2d", FUNC(pc80s31k_device::host_map)).umask16(0xff00);
	map(0x0068, 0x0068).w(FUNC(pc9801_state::pc9801_video_ff_w)); //mode FF / <undefined>
//  map(0x0080, 0x0080).rw(FUNC(pc9801_state::sasi_data_r), FUNC(pc9801_state::sasi_data_w));
//  map(0x0082, 0x0082).rw(FUNC(pc9801_state::sasi_status_r), FUNC(pc9801_state::sasi_ctrl_w));
	map(0x0090, 0x0090).r(m_fdc_2hd, FUNC(upd765a_device::msr_r));
	map(0x0092, 0x0092).rw(m_fdc_2hd, FUNC(upd765a_device::fifo_r), FUNC(upd765a_device::fifo_w));
	map(0x0094, 0x0094).rw(FUNC(pc9801_state::fdc_2hd_ctrl_r), FUNC(pc9801_state::fdc_2hd_ctrl_w));
	map(0x00a0, 0x00af).rw(FUNC(pc9801_state::pc9801_a0_r), FUNC(pc9801_state::pc9801_a0_w)); //upd7220 bitmap ports / display registers
//  map(0x00c8, 0x00cb).m(m_fdc_2dd, FUNC(upd765a_device::map)).umask16(0x00ff);
//  map(0x00cc, 0x00cc).rw(FUNC(pc9801_state::fdc_2dd_ctrl_r), FUNC(pc9801_state::fdc_2dd_ctrl_w)); //upd765a 2dd / <undefined>
}

/*************************************
 *
 * PC-9801RS specific handlers (IA-32)
 *
 ************************************/

// TODO: convert to device
// TODO: having this non-linear makes the system to boot in BASIC for PC-9821. Perhaps it stores settings? How to change these?
uint8_t pc9801vm_state::pc9801rs_knjram_r(offs_t offset)
{
	uint32_t pcg_offset;

	if(!(m_font_addr & 0xff))
	{
		int char_size = m_video_ff[FONTSEL_REG];
		return m_char_rom[(m_font_addr >> 8) * (8 << char_size) + (char_size * 0x800) + ((offset >> 1) & 0xf)];
	}

	pcg_offset = (m_font_addr & 0x7f7f) << 5;
	pcg_offset|= offset & 0x1e;

	// 8x16 charset selector
	// telenetm defintely mirrors offset & 1 for 8x16 romaji title songs, would otherwise blank them out
	if((m_font_addr & 0x7c00) == 0x0800)
		return m_kanji_rom[pcg_offset | 0];

	// rxtrain wants the LR setting for PCG area
	if((m_font_addr & 0xff00) == 0x5600 || (m_font_addr & 0xff00) == 0x5700)
	{
		pcg_offset |= (!m_video_ff[KAC_REG] << 12);
		return m_kanji_rom[pcg_offset | m_font_lr];
	}

	// ... but mezaset2 don't, implying it just read this linearly
	return m_kanji_rom[pcg_offset | (offset & 1)];
}

void pc9801vm_state::pc9801rs_knjram_w(offs_t offset, uint8_t data)
{
	uint32_t pcg_offset;

	pcg_offset = (m_font_addr & 0x7f7f) << 5;
	pcg_offset|= offset & 0x1e;
	pcg_offset|= m_font_lr;

	if((m_font_addr & 0xff00) == 0x5600 || (m_font_addr & 0xff00) == 0x5700)
	{
		// HACK: don't know yet how KAC works
		// 0=code access, 1=dot access
		// os2warp3 expects a kanjiram flag to RAM $596 -> PCG offset 000ac429,
		// will otherwise moan for CPU not set High.
		// This traces back by POST routines setting that location with 0x80, then it successively
		// wipes out a good chunk of the area with KAC mode enabled ...
		pcg_offset |= (!m_video_ff[KAC_REG] << 12);
		m_kanji_rom[pcg_offset] = data;
		m_gfxdecode->gfx(2)->mark_dirty(pcg_offset >> 5);
	}
}

void pc9801vm_state::itf_43d_bank_w(offs_t offset, uint8_t data)
{
	if((data & 0xf0) == 0x00 || (data & 0xf0) == 0x10)
	{
		if((data & 0xed) == 0x00)
		{
			m_ipl->set_bank((data & 2) >> 1);
			return;
		}
	}

	logerror("Unknown ITF $43d ROM bank setting %02x\n",data);
}

void pc9801vm_state::cbus_43f_bank_w(offs_t offset, uint8_t data)
{
	if((data & 0xf0) == 0x20)
		m_vram_bank = (data & 2) >> 1;
	else
	{
		logerror("Unknown C-Bus $43f bank setting %02x\n",data);
	}
}

// TODO: port 0xf1 (IDE select on later machines)
uint8_t pc9801vm_state::a20_ctrl_r(offs_t offset)
{
	if(offset == 0)
	{
		// for amd98 ID port
		return m_cbus_root->io_r(0xf0 >> 1, 0x00ff);
	}
	if(offset == 0x01)
		return (m_gate_a20 ^ 1) | 0xfe;
	else if(offset == 0x03)
		return (m_gate_a20 ^ 1) | (m_nmi_ff << 1);

	return 0xff;
}

void pc9801vm_state::a20_ctrl_w(offs_t offset, uint8_t data)
{
	if(offset == 0x00)
	{
		uint8_t por;
		/* reset POR bit */
		// TODO: is there any other way that doesn't involve direct r/w of ppi address?
		por = m_ppi_sys->read(2) & ~0x20;
		m_ppi_sys->write(2, por);
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		m_gate_a20 = 0;
	}

	if(offset == 0x01)
		m_gate_a20 = 1;

	if(offset == 0x03)
	{
		if(data == 0x02)
			m_gate_a20 = 1;
		else if(data == 0x03)
			m_gate_a20 = 0;
		else
			logerror("CPU port $00f6: unmapped data write %02x\n", data);
	}
	m_maincpu->set_input_line(INPUT_LINE_A20, m_gate_a20);
}

uint8_t pc9801_state::grcg_r(offs_t offset)
{
	if(offset == 6)
	{
		logerror("GRCG mode R\n");
		return 0xff;
	}
	else if(offset == 7)
	{
		logerror("GRCG tile R\n");
		return 0xff;
	}
	return txt_scrl_r(offset);
}

void pc9801_state::grcg_w(offs_t offset, uint8_t data)
{
	if(offset == 6)
	{
//      logerror("%02x GRCG MODE\n",data);
		m_grcg.mode = data;
		m_grcg.tile_index = 0;
		return;
	}
	else if(offset == 7)
	{
//      logerror("%02x GRCG TILE %02x\n",data,m_grcg.tile_index);
		m_grcg.tile[m_grcg.tile_index] = bitswap<8>(data,0,1,2,3,4,5,6,7);
		m_grcg.tile_index ++;
		m_grcg.tile_index &= 3;
		return;
	}

	txt_scrl_w(offset,data);
}

void pc9801vm_state::pc9801rs_a0_w(offs_t offset, uint8_t data)
{
	if((offset & 1) == 0 && offset & 8 && m_ex_video_ff[ANALOG_16_MODE])
	{
		switch(offset)
		{
			case 0x08: m_analog16.pal_entry = data & 0xf; break;
			case 0x0a: m_analog16.g[m_analog16.pal_entry] = data & 0xf; break;
			case 0x0c: m_analog16.r[m_analog16.pal_entry] = data & 0xf; break;
			case 0x0e: m_analog16.b[m_analog16.pal_entry] = data & 0xf; break;
		}

		m_palette->set_pen_color(
			m_analog16.pal_entry + 0x10,
			pal4bit(m_analog16.r[m_analog16.pal_entry]),
			pal4bit(m_analog16.g[m_analog16.pal_entry]),
			pal4bit(m_analog16.b[m_analog16.pal_entry])
		);
		// lemmings raster effects
		m_screen->update_partial(m_screen->vpos());
		return;
	}

	pc9801_a0_w(offset,data);
}

void pc9801vm_state::egc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if((m_egc.regs[1] & 0x6000) && (offset == 4)) // why?
		return;
	COMBINE_DATA(&m_egc.regs[offset]);
	switch(offset)
	{
		case 4:
			m_egc.mask = bitswap<16>(m_egc.regs[4],8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
			break;
		case 2:
		case 6:
		case 7:
			m_egc.leftover[0] = m_egc.leftover[1] = m_egc.leftover[2] = m_egc.leftover[3] = 0;
			m_egc.count = (m_egc.regs[7] & 0xfff) + 1;
			m_egc.first = true;
			m_egc.start = false;
			m_egc.loaded = false;
			break;
	}
}

uint16_t pc9801vm_state::grcg_gvram_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t ret = upd7220_grcg_r(offset + 0x4000, mem_mask);
	return bitswap<16>(ret,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
}

void pc9801vm_state::grcg_gvram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data = bitswap<16>(data,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
	upd7220_grcg_w(offset + 0x4000, data, mem_mask);
}

uint16_t pc9801vm_state::grcg_gvram0_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t ret = upd7220_grcg_r(offset, mem_mask);
	return bitswap<16>(ret,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
}

void pc9801vm_state::grcg_gvram0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data = bitswap<16>(data,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
	upd7220_grcg_w(offset, data, mem_mask);
}

/*
 * FDC MODE control
 *
 * ???? ---- <undefined>
 * ---- x--- (r/o) DIP-SW 3-2 built-in FDC spec (1) OFF 2HD (0) ON 2DD
 * ---- -x-- (r) DIP-SW 3-1 FDC FIXed mode (1) ON
 *           (w) Enable motor ON (1) defined by 0x94 bit 3 (0) always on
 * ---- --x- (r/w) FDD EXC access mode status (1) 2HD (0) 2DD
 * ---- ---x (r/w) PORT EXC I/F mode select (1) 2HD (0) 2DD
 *           (Disables I/O port access)
 *
 * NB: high-reso class diverges here:
 * - XA/XL themselves have no way to access any of this;
 * - post-XA/XL just has Enable motor ON writes;
 */
uint8_t pc9801vm_state::fdc_mode_r()
{
	return (m_fdc_mode & 3) | 0xf0 | (m_dsw3->read() & 3) << 2;
}

void pc9801vm_state::fdc_set_density_mode(bool is_2hd)
{
	floppy_image_device *floppy0 = m_fdc_2hd->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = m_fdc_2hd->subdevice<floppy_connector>("1")->get_device();

	floppy0->set_rpm(is_2hd ? 360 : 300);
	floppy1->set_rpm(is_2hd ? 360 : 300);

	m_fdc_2hd->set_rate(is_2hd ? 500000 : 250000);
//  printf("FDC set new mode %s\n", is_2hd ? "2HD" : "2DD");
	logerror("%s: FDC set new mode %s\n", machine().describe_context(), is_2hd ? "2HD" : "2DD");
}

void pc9801vm_state::fdc_mode_w(uint8_t data)
{
	const bool old_mode = bool(BIT(m_fdc_mode, 1));
	const bool new_mode = bool(BIT(data, 1));

	if (old_mode != new_mode)
		fdc_set_density_mode(new_mode);

	m_fdc_mode = data;

	if(BIT(m_fdc_mode, 2))
	{
		m_fdc_2hd->subdevice<floppy_connector>("0")->get_device()->mon_w(CLEAR_LINE);
		m_fdc_2hd->subdevice<floppy_connector>("1")->get_device()->mon_w(CLEAR_LINE);
	}

	//if(data & 0xfc)
	//  logerror("FDC ctrl called with %02x\n",data);
}

TIMER_CALLBACK_MEMBER(pc9801vm_state::fdc_trigger)
{
	// TODO: sorcer/hydlide definitely expects the XTMASK irq to be taken
	// NOTE: should probably trigger the FDC irq depending on mode, i.e. use fdc_irq_w fn
	if (BIT(m_fdc_2hd_ctrl, 2))
	{
		m_pic2->ir2_w(0);
		m_pic2->ir2_w(1);
	}
}

// TODO: undefined/disallow read/writes if I/F mode doesn't match
// (and that applies to FDC mapping too!)
// id port 0 -> 2DD
// id port 1 -> 2HD
template <unsigned port> u8 pc9801vm_state::fdc_2hd_2dd_ctrl_r()
{
	u8 res = fdc_2hd_ctrl_r();
	if (port == 0)
	{
		res |= 0x20;
		res |= fdc_drive_ready_r(m_fdc_2hd) << 4;
	}
	return res;
}

template <unsigned port> void pc9801vm_state::fdc_2hd_2dd_ctrl_w(u8 data)
{
	bool prev_trig = false;
	bool cur_trig = false;

	if (port == 0 && bool(BIT(m_fdc_mode, 0)) == false)
	{
		prev_trig = bool(BIT(m_fdc_2hd_ctrl, 0));
		cur_trig = bool(BIT(data, 0));
	}

	fdc_2hd_ctrl_w(data);

	// TODO: Enable motor ON is reversed compared to the docs
	if(!(m_fdc_mode & 4)) // required for 9821
	{
		m_fdc_2hd->subdevice<floppy_connector>("0")->get_device()->mon_w(data & 8 ? CLEAR_LINE : ASSERT_LINE);
		m_fdc_2hd->subdevice<floppy_connector>("1")->get_device()->mon_w(data & 8 ? CLEAR_LINE : ASSERT_LINE);
	}

	// TODO: this looks awfully similar to pc88va DMA mode, including same bits for trigger and irq mask.
	// NOTE: 100 msec too slow
	if (port == 0 && !prev_trig && cur_trig)
	{
		m_fdc_timer->reset();
		m_fdc_timer->adjust(attotime::from_msec(100));
	}
}

// TODO: some machines (which?) mirror 0x00be to 0x04be
u8 pc9801vm_state::fdc_3mode_r(offs_t offset)
{
	// freebsd21 expects 0-fill rather than the more logical 1-fill
	// on 5.25" floppies/ext. drive id checks, open bus?
	// TODO: external FDD
	if (m_fdc_3mode.dev_sel & 2)
		return 0;

	const bool is_35hd = m_fdc_2hd->subdevice<floppy_connector>(m_fdc_3mode.dev_sel ? "1" : "0")->get_device()->get_form_factor() == floppy_image::FF_35;

	if (!is_35hd)
		return 0;

	u8 res = 0xee;

	// Check if drive is in 2HD/1MB mode
	if (BIT(m_fdc_mode, 1))
		res |= 1 << 4;

	if (m_fdc_3mode.access_144mb)
		res |= 1 << 0;

	return res;
}

/*
 * -xx- ---- Drive specification
 * -00- ---- First internal drive
 * -01- ---- Second internal drive
 * -10- ---- External drive
 * ---x ---- Operation mode specification
 * ---0 ---- no-op
 * ---1 ---- Access Mode valid
 * ---- ---x Access Mode specification
 * ---- ---0 1MB/640KB
 * ---- ---1 1.44MB
 */
void pc9801vm_state::fdc_3mode_w(offs_t offset, uint8_t data)
{
	//logerror("$4be: W %02x\n", data);
	m_fdc_3mode.dev_sel = (data & 0x60) >> 5;

	// TODO: external FDD
	if (m_fdc_3mode.dev_sel & 2)
		return;

	if (BIT(data, 4))
	{
		const bool is_35hd = m_fdc_2hd->subdevice<floppy_connector>(m_fdc_3mode.dev_sel ? "1" : "0")->get_device()->get_form_factor() == floppy_image::FF_35;

		if (!is_35hd)
			return;

		floppy_image_device *floppy = m_fdc_2hd->subdevice<floppy_connector>(m_fdc_3mode.dev_sel ? "1" : "0")->get_device();
		m_fdc_3mode.access_144mb = !!(BIT(data, 0));
		if (m_fdc_3mode.access_144mb)
		{
			floppy->set_rpm(300);
			m_fdc_2hd->set_rate(500000);
		}
		else
		{
			fdc_set_density_mode(!!BIT(m_fdc_mode, 1));
		}
	}
}

void pc9801vm_state::pc9801rs_video_ff_w(offs_t offset, uint8_t data)
{
	if(offset == 1)
	{
		if((data & 0xf0) == 0) /* disable any PC-9821 specific HW regs */
			m_ex_video_ff[(data & 0xfe) >> 1] = data & 1;

		if(0)
		{
			static const char *const ex_video_ff_regnames[] =
			{
				"16 colors mode",   // 0
				"<unknown>",        // 1
				"EGC related",      // 2
				"<unknown>"         // 3
			};

			logerror("Write to extended video FF register %s -> %02x\n",ex_video_ff_regnames[(data & 0x06) >> 1],data & 1);
		}
		//else
		//  logerror("Write to extended video FF register %02x\n",data);

		return;
	}

	pc9801_video_ff_w(data);
}

/*
 * DMA access control (I/O $439)
 *
 * x--- ---- Mate A: Printer select (1) built-in (0) PC-9821-E02
 *           ^ on 98Fellow with 0 it disconnects the printer interface,
 *           ^ may still select anything but -E02?
 * --?? ---- <unknown>
 * ---- -x-- DMA address mask (1) inhibit DMA access for anything higher than 1MB
 *           ^ defaults to 0 on high-reso equipped machines (?), 1 otherwise
 * ---- --x- Graph LIO BIOS select (?), on 9801VX21
 * ---- ---x Mate A: selects high-reso mode
 *           ^ unknown otherwise
 *
 * TODO: why BIOS has to read it twice during cold boot?
 * Later 9821 machines will add a mov bh,al on second read, is there some kind of side
 * effect for reading?
 */
u8 pc9801vm_state::dma_access_ctrl_r(offs_t offset)
{
	logerror("%s: DMA access control read\n", machine().describe_context());
	return m_dma_access_ctrl;
}

void pc9801vm_state::dma_access_ctrl_w(offs_t offset, u8 data)
{
	logerror("%s: DMA access control write %02x\n", machine().describe_context(), data);
	m_dma_access_ctrl = data;
}

// ARTIC device

/*
 * [0] read bits 15-0 of the counter device
 * [1] read bits 23-8 of the counter device
 *
 * FreeDOS(98) Kernel will test [1] a whole lot during HMA allocation
 */
uint16_t pc9801vm_state::timestamp_r(offs_t offset)
{
	return (m_maincpu->total_cycles() >> (8 * offset));
}

void pc9801vm_state::artic_wait_w(u8 data)
{
	// 0.6 μsec
	m_maincpu->spin_until_time(attotime::from_nsec(600));
}

uint8_t pc9801_state::pic_r(offs_t offset)
{
	return ((offset >= 4) ? m_pic2 : m_pic1)->read(offset & 3);
}

void pc9801_state::pic_w(offs_t offset, uint8_t data)
{
	((offset >= 4) ? m_pic2 : m_pic1)->write(offset & 3, data);
}

void pc9801_state::ipl_bank(address_map &map)
{
	map(0x00000, 0x2ffff).rom().region("ipl", 0);
}

void pc9801vm_state::pc9801vm_map(address_map &map)
{
	map(0x000000, 0x09ffff).rw("cbus_root", FUNC(pc98_cbus_root_device::mem_r), FUNC(pc98_cbus_root_device::mem_w));

	map(0x0a0000, 0x0a3fff).rw(FUNC(pc9801vm_state::tvram_r), FUNC(pc9801vm_state::tvram_w));
	map(0x0a4000, 0x0a4fff).rw(FUNC(pc9801vm_state::pc9801rs_knjram_r), FUNC(pc9801vm_state::pc9801rs_knjram_w));
	map(0x0a8000, 0x0bffff).rw(FUNC(pc9801vm_state::grcg_gvram_r), FUNC(pc9801vm_state::grcg_gvram_w));
	map(0x0c0000, 0x0dffff).rw("cbus_root", FUNC(pc98_cbus_root_device::mem_slot_r), FUNC(pc98_cbus_root_device::mem_slot_w));

	map(0x0e0000, 0x0e7fff).rw(FUNC(pc9801vm_state::grcg_gvram0_r), FUNC(pc9801vm_state::grcg_gvram0_w));
	map(0x0e8000, 0x0fffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
}

void pc9801vm_state::pc9801vm_io(address_map &map)
{
	map(0x0000, 0xffff).rw(m_cbus_root, FUNC(pc98_cbus_root_device::io_r), FUNC(pc98_cbus_root_device::io_w));
//  map.unmap_value_high();
	pc9801_common_io(map);
	map(0x0020, 0x002f).w(FUNC(pc9801vm_state::dmapg8_w)).umask16(0xff00);
	map(0x005c, 0x005f).r(FUNC(pc9801vm_state::timestamp_r)); // artic
	map(0x005f, 0x005f).w(FUNC(pc9801vm_state::artic_wait_w));
	map(0x0068, 0x006b).w(FUNC(pc9801vm_state::pc9801rs_video_ff_w)).umask16(0x00ff); //mode FF / <undefined>
	map(0x0070, 0x007f).rw(FUNC(pc9801vm_state::grcg_r), FUNC(pc9801vm_state::grcg_w)).umask16(0x00ff); //display registers "GRCG" / i8253 pit
	map(0x0090, 0x0090).r(m_fdc_2hd, FUNC(upd765a_device::msr_r));
	map(0x0092, 0x0092).rw(m_fdc_2hd, FUNC(upd765a_device::fifo_r), FUNC(upd765a_device::fifo_w));
	map(0x0094, 0x0094).rw(FUNC(pc9801vm_state::fdc_2hd_2dd_ctrl_r<1>), FUNC(pc9801vm_state::fdc_2hd_2dd_ctrl_w<1>));
	map(0x00a0, 0x00af).rw(FUNC(pc9801vm_state::pc9801_a0_r), FUNC(pc9801vm_state::pc9801rs_a0_w)); //upd7220 bitmap ports / display registers
	map(0x00be, 0x00be).rw(FUNC(pc9801vm_state::fdc_mode_r), FUNC(pc9801vm_state::fdc_mode_w));
	map(0x00c8, 0x00cb).m(m_fdc_2hd, FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0x00cc, 0x00cc).rw(FUNC(pc9801vm_state::fdc_2hd_2dd_ctrl_r<0>), FUNC(pc9801vm_state::fdc_2hd_2dd_ctrl_w<0>));
	map(0x00f0, 0x00ff).rw(FUNC(pc9801vm_state::a20_ctrl_r), FUNC(pc9801vm_state::a20_ctrl_w)).umask16(0x00ff);
	map(0x0439, 0x0439).rw(FUNC(pc9801vm_state::dma_access_ctrl_r), FUNC(pc9801vm_state::dma_access_ctrl_w));
	map(0x043d, 0x043d).w(FUNC(pc9801vm_state::itf_43d_bank_w));
	map(0x043f, 0x043f).w(FUNC(pc9801vm_state::cbus_43f_bank_w));
	map(0x04a0, 0x04af).w(FUNC(pc9801vm_state::egc_w));
	map(0x04be, 0x04be).rw(FUNC(pc9801vm_state::fdc_3mode_r), FUNC(pc9801vm_state::fdc_3mode_w));
	map(0x3fd8, 0x3fdf).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0xff00);
}

void pc9801vm_state::pc9801ux_map(address_map &map)
{
	pc9801vm_map(map);
	map(0x000000, 0x09ffff).rw("simm", FUNC(pc9801_54_simm_device::read), FUNC(pc9801_54_simm_device::write));
	map(0x100000, 0x7fffff).rw("simm", FUNC(pc9801_54_simm_device::read_ext), FUNC(pc9801_54_simm_device::write_ext));
//  map(0xee8000, 0xefffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
//  map(0xfe8000, 0xffffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
}

void pc9801vm_state::pc9801vx_map(address_map &map)
{
	pc9801vm_map(map);
	map(0x000000, 0x09ffff).rw("simm", FUNC(pc9801_54_simm_device::read), FUNC(pc9801_54_simm_device::write));
	map(0x100000, 0x8fffff).rw("simm", FUNC(pc9801_54_simm_device::read_ext), FUNC(pc9801_54_simm_device::write_ext));
}

void pc9801vm_state::pc9801dx_map(address_map &map)
{
	pc9801vm_map(map);
	map(0x000000, 0x09ffff).rw("simm", FUNC(pc9801_54_simm_device::read), FUNC(pc9801_54_simm_device::write));
	map(0x100000, 0xefffff).rw("simm", FUNC(pc9801_54_simm_device::read_ext), FUNC(pc9801_54_simm_device::write_ext));
}

void pc9801vm_state::pc9801rs_map(address_map &map)
{
	pc9801vm_map(map);
	map(0x000000, 0x09ffff).rw("simm", FUNC(pc9801_54_simm_device::read), FUNC(pc9801_54_simm_device::write));
//  map(0x0d8000, 0x0d9fff).rom().region("ide",0);
	map(0x0da000, 0x0dbfff).ram(); // ide ram
	map(0x100000, 0xefffff).rw("simm", FUNC(pc9801_54_simm_device::read_ext), FUNC(pc9801_54_simm_device::write_ext));
	map(0xee8000, 0xefffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
	map(0xfe8000, 0xffffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
}

void pc9801vm_state::pc9801rs_io(address_map &map)
{
	map.unmap_value_high();
	pc9801vm_io(map);
	map(0x00f0, 0x00f0).lr8(NAME([] () { return 0; }));
	map(0x0430, 0x0433).rw(FUNC(pc9801vm_state::ide_ctrl_hack_r), FUNC(pc9801vm_state::ide_ctrl_w)).umask16(0x00ff);
	map(0x0640, 0x064f).rw(FUNC(pc9801vm_state::ide_cs0_r), FUNC(pc9801vm_state::ide_cs0_w));
	map(0x0740, 0x074f).rw(FUNC(pc9801vm_state::ide_cs1_r), FUNC(pc9801vm_state::ide_cs1_w));
	map(0x1e8c, 0x1e8f).noprw(); // temp
	map(0xbfdb, 0xbfdb).w(FUNC(pc9801vm_state::mouse_freq_w));
}

void pc9801us_state::pc9801us_io(address_map &map)
{
	pc9801rs_io(map);
	map(0x0430, 0x0433).rw(FUNC(pc9801us_state::ide_ctrl_r), FUNC(pc9801us_state::ide_ctrl_w)).umask16(0x00ff);
	map(0x00f6, 0x00f6).lw8(NAME([this] (offs_t offset, u8 data) {
		// despite what undocumented mem claims US and FS actually access this for SDIP banking
		if (data == 0xa0 || data == 0xe0)
			m_sdip->bank_w(BIT(data, 6));
		else
			a20_ctrl_w(3, data);
	}));

	// 0x841e ~ 0x8f1e SDIP I/O mapping
	// NOTE: split in half for pleasing emumem
	map(0x841e, 0x841e).select(0x300).lrw8(
		NAME([this] (offs_t offset) { return m_sdip->read(offset >> 8); }),
		NAME([this] (offs_t offset, u8 data) { m_sdip->write(offset >> 8, data); })
	);
	map(0x881e, 0x881e).select(0x700).lrw8(
		NAME([this] (offs_t offset) { return m_sdip->read((offset >> 8) + 4); }),
		NAME([this] (offs_t offset, u8 data) { m_sdip->write((offset >> 8) + 4, data); })
	);
//  map(0x8f1f, 0x8f1f).w(m_sdip, FUNC(pc98_sdip_device::bank_w));
}

void pc9801bx_state::pc9801bx2_map(address_map &map)
{
	pc9801vm_map(map);
	map(0x000000, 0x09ffff).rw("simm", FUNC(pc9801_61_simm_device::read), FUNC(pc9801_61_simm_device::write));

	map(0x0da000, 0x0dbfff).ram(); // ide ram

//  map(0x000a0000, 0x000a3fff).rw(FUNC(pc9801_state::tvram_r), FUNC(pc9801_state::tvram_w));
//  map(0x000a4000, 0x000a4fff).rw(FUNC(pc9801_state::pc9801rs_knjram_r), FUNC(pc9801_state::pc9801rs_knjram_w));
//  map(0x000a8000, 0x000bffff).rw(FUNC(pc9821_state::pc9821_grcg_gvram_r), FUNC(pc9821_state::pc9821_grcg_gvram_w));
//  map(0x000cc000, 0x000cffff).rom().region("sound_bios", 0); //sound BIOS
//  map(0x000d8000, 0x000d9fff).rom().region("ide",0)
//  map(0x000da000, 0x000dbfff).ram(); // ide ram (declared in RS)
//  map(0x000e0000, 0x000e7fff).rw(FUNC(pc9821_state::pc9821_grcg_gvram0_r), FUNC(pc9821_state::pc9821_grcg_gvram0_w));
	map(0x000e8000, 0x000fffff).m(m_ipl, FUNC(address_map_bank_device::amap16));

	map(0x00100000, 0x00efffff).rw("simm", FUNC(pc9801_61_simm_device::read_ext), FUNC(pc9801_61_simm_device::write_ext));
	map(0x00f00000, 0x00ffffff).view(m_hole_15M_view);
	m_hole_15M_view[0](0x00f00000, 0x00ffffff).rw("simm", FUNC(pc9801_61_simm_device::read_15m_ext), FUNC(pc9801_61_simm_device::write_15m_ext));

	map(0x01000000, 0x013fffff).rw("simm", FUNC(pc9801_61_simm_device::read_16m_ext), FUNC(pc9801_61_simm_device::write_16m_ext));
	map(0xffee8000, 0xffefffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
	map(0xfffe8000, 0xffffffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
}

u8 pc9801bx_state::i486_cpu_mode_r(offs_t offset)
{
	/*
	 * x--- ---- 33 MHz/25 MHz switch (9821 Bp, Bs, Be, Cs2, Ce2 only)
	 * ---- ---x (1) MIDDLE or LOW mode (?), (0) High mode
	 */
	// 9821 MULTi checks this and refuses to boot if finds bit 0 high
	return 0;
}

/*
 * GDC 31 kHz register
 * R/W on PC-98GS, H98, all PC-9821 except PC-9821Ts,
 * 9801BA2, 9801BS2, 9801BX2
 *
 * x--- ---- unknown, if set high then DAC1BIT goes berserk at POST.
 * ---- --xx (R/W) horizontal frequency
 * ---- --1x ^ "setting prohibited" (?)
 * ---- --01 ^ 31.47kHz
 * ---- --00 ^ 24.83kHz
 *
 * PC-9801NS/A can also r/w this but with different meaning
 */
u8 pc9801bx_state::gdc_31kHz_r(offs_t offset)
{
	return 0;
}

void pc9801bx_state::gdc_31kHz_w(offs_t offset, u8 data)
{
	// Repeatedly switches 0 to 3 during POST, sync monitor check to guess support?
//  if (data)
//      popmessage("31kHz register set %02x, contact MAMEdev", data);
}

u8 pc9801bx_state::hole_15m_control_r(offs_t offset)
{
	return m_hole_15m;
}

void pc9801bx_state::hole_15m_control_w(offs_t offset, u8 data)
{
	m_hole_15m = data;
	if (BIT(data, 2))
		m_hole_15M_view.select(0);
	else
		m_hole_15M_view.disable();

	if (data & 0xfb)
		popmessage("hole_15m_control_w: undocumented trigger %02x", data);
}

void pc9801bx_state::pc9801bx2_io(address_map &map)
{
	pc9801us_io(map);
	// NOP legacy SDIP bank access
	map(0x00f6, 0x00f6).lw8(NAME([this] (offs_t offset, u8 data) { a20_ctrl_w(3, data); }));
	map(0x043b, 0x043b).rw(FUNC(pc9801bx_state::hole_15m_control_r), FUNC(pc9801bx_state::hole_15m_control_w));
	map(0x0530, 0x0533).lr8(NAME([] () { return 0; })); // SIMM control unit
	map(0x0534, 0x0534).r(FUNC(pc9801bx_state::i486_cpu_mode_r));
	map(0x09a8, 0x09a8).rw(FUNC(pc9801bx_state::gdc_31kHz_r), FUNC(pc9801bx_state::gdc_31kHz_w));
	map(0x8f1f, 0x8f1f).lw8(NAME([this] (offs_t offset, u8 data) {
		// BA2 onward and every PC-9821 uses this method for SDIP bank
		if (data == 0x80 || data == 0xc0)
			m_sdip->bank_w(BIT(data, 6));
		else
			logerror("SDIP: I/O $8f1f unrecognized write %02x\n", data);
	}));
}

/*
 * uPD7220 maps
 */

void pc9801_state::upd7220_1_map(address_map &map)
{
	map(0x00000, 0x03fff).ram().share("video_ram_1");
}

// TODO: this may need the bank reg or the pre-vm models may have had less gvram
void pc9801_state::upd7220_2_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram().share("video_ram_2");
}

void pc9801vm_state::upd7220_grcg_2_map(address_map &map)
{
	map(0x00000, 0x3ffff).rw(FUNC(pc9801vm_state::upd7220_grcg_r), FUNC(pc9801vm_state::upd7220_grcg_w)).share("video_ram_2");
}

ioport_value pc98_base_state::system_type_r()
{
//  System Type (0x00 stock PC-9801, 0xc0 PC-9801U / PC-98LT, PC-98HA, 0x80 others)
	return m_sys_type;
}

static INPUT_PORTS_START( pc9801 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, "Display Type" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, "Normal Display (15KHz)" )
	PORT_DIPSETTING(      0x0001, "Hi-Res Display (24KHz)" )
	// TODO: "GFX" screen selections (routing?) for vanilla class
	PORT_DIPUNKNOWN_DIPLOC( 0x002, 0x002, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x004, 0x004, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x008, 0x008, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x010, 0x010, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x020, 0x000, "SW1:6" )
	// TODO: built-in RXC / TXC clocks for RS-232C (routing?) for vanilla class
	PORT_DIPUNKNOWN_DIPLOC( 0x040, 0x000, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x080, 0x080, "SW1:8" )
	PORT_DIPUNKNOWN_DIPLOC( 0x100, 0x000, "SW1:9" )
	PORT_DIPUNKNOWN_DIPLOC( 0x200, 0x200, "SW1:10" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "System Specification" ) PORT_DIPLOCATION("SW2:1") //jumps to daa00 if off, presumably some card booting
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Terminal Mode" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Text width" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "40 chars/line" )
	PORT_DIPSETTING(    0x00, "80 chars/line" )
	PORT_DIPNAME( 0x08, 0x00, "Text height" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "20 lines/screen" )
	PORT_DIPSETTING(    0x00, "25 lines/screen" )
	PORT_DIPNAME( 0x10, 0x00, "Memory Switch Init" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) ) //Fix memory switch condition
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) ) //Initialize Memory Switch with the system default
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

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
INPUT_PORTS_END

static INPUT_PORTS_START( pc9801rs )
	PORT_INCLUDE( pc9801 )

	PORT_MODIFY("DSW1")
	// LCD display, 98DO Demo explicitly wants it to be non-Plasma
	PORT_DIPNAME( 0x04, 0x04, "Monitor Type" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "RGB" )
	PORT_DIPSETTING(    0x00, "Plasma" )
	PORT_DIPNAME( 0x80, 0x00, "Graphic Function" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Basic (8 Colors)" )
	PORT_DIPSETTING(    0x00, "Expanded (16/4096 Colors)" )
	PORT_BIT(0x300, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x80, 0x80, "GDC clock" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "2.5 MHz" )
	PORT_DIPSETTING(    0x00, "5 MHz" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "FDD Fix Mode" ) PORT_DIPLOCATION("SW3:1")
	// with this OFF enables PORT EXC (fdc mode bit 0)
	PORT_DIPSETTING(    0x00, "Auto-Detection" )
	PORT_DIPSETTING(    0x01, "Fixed" )
	PORT_DIPNAME( 0x02, 0x02, "FDD Density Select" ) PORT_DIPLOCATION("SW3:!2")
	PORT_DIPSETTING(    0x00, "2DD" )
	PORT_DIPSETTING(    0x02, "2HD" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPNAME( 0x20, 0x20, "Conventional RAM size" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, "640 KB" )
	PORT_DIPSETTING(    0x00, "512 KB" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPNAME( 0x80, 0x00, "CPU Type" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, "V30" )
	PORT_DIPSETTING(    0x00, "I386" )

	PORT_START("BIOS_LOAD")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x04, 0x00, "Load IDE BIOS" )
	PORT_CONFSETTING(    0x00, DEF_STR( Yes ) )
	PORT_CONFSETTING(    0x04, DEF_STR( No ) )
INPUT_PORTS_END

//static INPUT_PORTS_START( pc9801vm11 )
//  PORT_INCLUDE( pc9801rs )
//
//  PORT_MODIFY("DSW3")
//  // TODO: "CPU Add Waitstate Penalty"?
//  // specific for PC-98DO, CV21, UV11 and VM11
//  PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:!5")
//  PORT_DIPSETTING(    0x00, DEF_STR( No ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
//INPUT_PORTS_END


static const gfx_layout charset_8x8 =
{
	8,8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout charset_8x16 =
{
	8,16,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

static const gfx_layout charset_16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

static GFXDECODE_START( gfx_pc9801 )
	GFXDECODE_ENTRY( "chargen",     0x00000, charset_8x8,     0x000, 0x01 )
	GFXDECODE_ENTRY( "chargen",     0x00800, charset_8x16,    0x000, 0x01 )
	GFXDECODE_ENTRY( "kanji",       0x00000, charset_16x16,   0x000, 0x01 )
	GFXDECODE_ENTRY( "raw_kanji",   0x00000, charset_16x16,   0x000, 0x01 )
	GFXDECODE_ENTRY( "new_chargen", 0x00000, charset_16x16,   0x000, 0x01 )
GFXDECODE_END

/****************************************
*
* I8259 PIC interface
*
****************************************/

/*
irq assignment (PC-9801F):

8259 master:
ir0 PIT
ir1 keyboard
ir2 vblank
ir3 expansion bus INT0
ir4 rs-232c
ir5 expansion bus INT1
ir6 expansion bus INT2
ir7 PIC slave

8259 slave:
ir0 printer
ir1 expansion bus INT3 (HDD)
ir2 expansion bus INT41 (2dd floppy irq)
ir3 expansion bus INT42 (2hd floppy irq)
ir4 expansion bus INT5 (usually FM sound board)
ir5 expansion bus INT6 (mouse)
ir6 NDP coprocessor (up to V30 CPU)
ir7 <gnd>
*/


uint8_t pc9801_state::get_slave_ack(offs_t offset)
{
	if (offset==7) { // IRQ = 7
		return m_pic2->acknowledge();
	}
	return 0x00;
}

/****************************************
*
* I8253 PIT interface
*
****************************************/

/* These rates do NOT appear to represent actual XTALs. They are likely obtained in
   different ways on different PC-98 models as divisions of extant XTAL frequencies
   such as 14.7456 MHz, 15.9744 MHz, 19.6608 MHz and 23.9616 MHz.
   PC-9801RS needs X1 for the pit, otherwise Uchiyama Aki no Chou Bangai has sound pitch bugs
   PC-9821 definitely needs X2, otherwise there's a timer error at POST. Unless it needs a different clock anyway ...
   */
#define BASE_CLOCK      XTAL(31'948'800)    // verified to be used by PC-98RS/98FA by wd40yasu
#define MAIN_CLOCK_X1   (BASE_CLOCK / 16)   // 1.9968 MHz
#define MAIN_CLOCK_X2   (BASE_CLOCK / 13)   // 2.4576 MHz

/****************************************
*
* I8237 DMA interface
*
****************************************/

void pc9801_state::dma_hrq_changed(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	m_dmac->hack_w(state);

//  logerror("%02x HLDA\n",state);
}

void pc9801_state::tc_w(int state)
{
	switch (m_dack)
	{
		case 0:
			m_cbus_root->eop_w(0, state);
			break;
		case 2:
		case 3:
			m_fdc_2hd->tc_w(state);
			if (m_dack == 3)
				m_cbus_root->eop_w(3, state);
			//if(m_fdc_2dd)
			//  m_fdc_2dd->tc_w(state);
			break;
	}


//  logerror("TC %02x\n",state);
}

uint8_t pc9801_state::dma_read_byte(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_offset[m_dack] << 16) | offset;

	if(offset == 0xffff)
	{
		switch(m_dma_autoinc[m_dack])
		{
			case 1:
			{
				uint8_t page = m_dma_offset[m_dack];
				m_dma_offset[m_dack] = ((page + 1) & 0xf) | (page & 0xf0);
				break;
			}
			case 3:
				m_dma_offset[m_dack]++;
				break;
		}
	}

//  logerror("%08x %02x\n",addr, m_dma_access_ctrl);
	return program.read_byte(addr);
}


void pc9801_state::dma_write_byte(offs_t offset, uint8_t data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_offset[m_dack] << 16) | offset;

	if(offset == 0xffff)
	{
		switch(m_dma_autoinc[m_dack])
		{
			case 1:
			{
				uint8_t page = m_dma_offset[m_dack];
				m_dma_offset[m_dack] = ((page + 1) & 0xf) | (page & 0xf0);
				break;
			}
			case 3:
				m_dma_offset[m_dack]++;
				break;
		}
	}
//  logerror("%08x %02x %02x\n",addr,data, m_dma_access_ctrl);

	program.write_byte(addr, data);
}

uint8_t pc9801vm_state::dma_read_byte(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_offset[m_dack] << 16) | offset;

	if (BIT(m_dma_access_ctrl, 2))
		addr &= 0xfffff;

	if(offset == 0xffff)
	{
		switch(m_dma_autoinc[m_dack])
		{
			case 1:
			{
				uint8_t page = m_dma_offset[m_dack];
				m_dma_offset[m_dack] = ((page + 1) & 0xf) | (page & 0xf0);
				break;
			}
			case 3:
				m_dma_offset[m_dack]++;
				break;
		}
	}

//  logerror("%08x %02x\n",addr, m_dma_access_ctrl);
	return program.read_byte(addr);
}


void pc9801vm_state::dma_write_byte(offs_t offset, uint8_t data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_offset[m_dack] << 16) | offset;

	if (BIT(m_dma_access_ctrl, 2))
		addr &= 0xfffff;

	if(offset == 0xffff)
	{
		switch(m_dma_autoinc[m_dack])
		{
			case 1:
			{
				uint8_t page = m_dma_offset[m_dack];
				m_dma_offset[m_dack] = ((page + 1) & 0xf) | (page & 0xf0);
				break;
			}
			case 3:
				m_dma_offset[m_dack]++;
				break;
		}
	}
//  logerror("%08x %02x %02x\n",addr,data, m_dma_access_ctrl);

	program.write_byte(addr, data);
}

void pc9801_state::set_dma_channel(int channel, int state)
{
	if (!state) m_dack = channel;
}

/*
 * ch0 expansion slot or internal SASI/SCSI
 * ch1 memory refresh on vanilla/VM classes, expansion slot or internal HDD setting otherwise
 * ch2 1MB FDC
 * ch3 640KB FDC
 *
 * NOTE: PC-H98, PC-98RL & PC-98XL very different and changes configuration
 * depending on boot mode. cfr. io_dma.txt
 */

void pc9801_state::dack0_w(int state) { /*logerror("%02x 0\n",state);*/ set_dma_channel(0, state); }
void pc9801_state::dack1_w(int state) { /*logerror("%02x 1\n",state);*/ set_dma_channel(1, state); }
void pc9801_state::dack2_w(int state) { /*logerror("%02x 2\n",state);*/ set_dma_channel(2, state); }
void pc9801_state::dack3_w(int state) { /*logerror("%02x 3\n",state);*/ set_dma_channel(3, state); }

/*
 * PPI "system" I/F
 *
 * Port A:
 * xxxx xxxx DIP SW 2
 *           ^ treated as RAM on pc98lt/ha and 9821Ap, As, Ae
 *
 * Port B:
 * x--- ---- RS-232C CI signal (active low)
 * -x-- ---- RS-232C CS signal (active low)
 * --x- ---- RS-232C CD signal (active low)
 * (normal)
 * ---x ---- INT3 status for expansion bus
 * ---- x--- DIP SW 1-1 (0) 15 kHz (1) 24 kHz
 * ---- -x-- IMCK internal RAM parity error
 * ---- --x- EMCK expansion RAM parity error
 * ---- ---x CDAT RTC data read
 * H98 overrides bits 4, 3 with SHUTx status
 *
 * Port C:
 * x--- ---- (286+ machines) SHUT0 status
 * -x-- ---- PSTBM printer PSTB mask
 * --x- ---- (286+ machines) SHUT1 status
 *           ^ pc98lt/ha: ROM drive write protection (active low)
 * ---x ---- MCHKEN RAM parity check (0) invalid
 * ---- x--- buzzer/DAC1BIT mute
 * ---- -x-- TXRE interrupt enable for TXRDY in RS-232C
 * ---- --x- TXEE interrupt enable for TXEMPTY in RS-232C
 * ---- ---x RXRE interrupt enable for RXRDY in RS-232C
 *
 */

u8 pc9801_state::ppi_sys_portb_r()
{
	// TODO: should be active low for rs232
	u8 res = 0;

	res |= BIT(m_dsw1->read(), 0) << 3;
	res |= m_rtc->data_out_r();

	return res;
}

void pc9801_state::uart_irq_check()
{
	m_pic1->ir4_w(m_uart_irq_pending & m_uart_irq_mask ? 1 : 0);
}

template <unsigned N> void pc98_base_state::update_uart_irq(int state)
{
	if (state)
		m_uart_irq_pending |= 1 << N;
	else
		m_uart_irq_pending &= ~(1 << N);
	uart_irq_check();
}

void pc98_base_state::ppi_sys_beep_portc_w(uint8_t data)
{
	m_beeper->set_state(!(data & 0x08));
	m_uart_irq_mask = data & 7;
	uart_irq_check();
}

void pc9801vm_state::ppi_sys_dac_portc_w(uint8_t data)
{
	m_dac1bit_disable = BIT(data, 3);
	// TODO: some models have a finer grained volume control at I/O port 0xae8e
	// (98NOTE only?)
	m_dac1bit->set_output_gain(0, m_dac1bit_disable ? 0.0 : 1.0);
	m_uart_irq_mask = data & 7;
	uart_irq_check();
}

/*
 * PPI "printer" I/F
 *
 * Port B:
 * xx-- ---- TYP1, 0 system type
 * 11-- ---- PC-9801U, PC98LT/HA
 * 10-- ---- <everything else>
 * 01-- ---- <undefined>
 * 00-- ---- vanilla PC-9801
 * --x- ---- MOD system clock
 * --1- ---- CPU 8 MHz Timer 2 MHz
 * --0- ---- CPU 5/10 MHz Timer 2.5 MHz
 * ---x ---- DIP SW 1-3 plasma display (notebooks & pc98lt/ha only)
 * ---- x--- DIP SW 1-8 analog 16 enable
 * ---- -x-- Printer BUSY signal (active low)
 * ---- --x- (normal) CPUT V30 mode
 *           ^ (pc98lt/ha) CPUT system type
 * ---- ---x VF flag (PC-9801VF, PC-9801U)
 *           ^ ? only for models with built-in 2DD
 *
 */

u8 pc9801_state::ppi_prn_portb_r()
{
	u8 res = 0;

//  res |= BIT(m_dsw1->read(), 7) << 3;
//  res |= BIT(m_dsw1->read(), 2) << 4;
	res |= m_sys_type << 6;

	return res;
}

u8 pc9801vm_state::ppi_prn_portb_r()
{
	u8 res = pc9801_state::ppi_prn_portb_r();

	res |= BIT(m_dsw1->read(), 7) << 3;
	res |= BIT(m_dsw1->read(), 2) << 4;

	return res;
}

/*
 * Mouse 8255 I/F
 *
 * Port A:
 * x--- ---- LEFT mouse button
 * -x-- ---- MIDDLE mouse button
 *           \- Undocumented, most PC98 mice don't have it
 * --x- ---- RIGHT mouse button
 * ---? ---- <unused>
 * ---- xxxx MD3-0 mouse direction latch
 *
 * Port B:
 * x--- ---- H98 only: DIP SW 1-4 (FDD external drive select)
 * -x-- ---- DIP SW 3-6: conventional RAM size (RAMKL)
 * ---x ---- PC98RL only: DIP SW 3-3 SASI DMA channel 0
 * ---- --x- SPDSW readout: selects CPU clock speed for 286/386 equipped machines
 * ---- ---x SPDSW readout for H98
 *
 * Port C:
 *
 * x--- ---- HC Latch Mode (1=read latch, 0=read delta)
 *           \- on 0->1 transition reset delta
 * -x-- ---- SXY Axis select (1=Y 0=X)
 * --x- ---- SHL Read nibble select (1) upper (0) lower
 * ---x ---- INT # (1) disable (0) enable
 * ---- x--- (r/o) H98 only: MODSW (0) High-reso mode (1) Normal mode
 * ---- -x-- (r/o) CPUSW: DIP SW 3-8 (1) V30 compatible mode
 * ---- --xx (r/o) RS-232C sync mode settings, DIP SWs 1-6 & 1-5
 *
 * Reading Port B and Port C low nibble are misc DIPSW selectors,
 * their meaning diverges on XA/XL/RL classes vs. the rest.
 *
 */

u8 pc9801_state::ppi_mouse_porta_r()
{
	u8 res = ioport("MOUSE_B")->read() & 0xf0;
	const u8 isporthi = ((m_mouse.control & 0x20) >> 5)*4;

	if ((m_mouse.control & 0x80) == 0)
	{
		if (m_mouse.control & 0x40)
			res |= (m_mouse.dy >> isporthi) & 0xf;
		else
			res |= (m_mouse.dx >> isporthi) & 0xf;
	}
	else
	{
		if (m_mouse.control & 0x40)
			res |= (m_mouse.ly >> isporthi) & 0xf;
		else
			res |= (m_mouse.lx >> isporthi) & 0xf;
	}

//  logerror("A\n");
	return res;
}

void pc9801_state::ppi_mouse_porta_w(uint8_t data)
{
//  logerror("A %02x\n",data);
}

u8 pc9801vm_state::ppi_mouse_portb_r()
{
	return (BIT(m_dsw3->read(), 5) << 6) | 2;
}

void pc9801_state::ppi_mouse_portb_w(uint8_t data)
{
	logerror("%s: PPI mouse port B %02x\n", machine().describe_context() ,data);
}

u8 pc9801vm_state::ppi_mouse_portc_r()
{
	return (BIT(m_dsw3->read(), 7) << 2);
}

void pc9801_state::ppi_mouse_portc_w(uint8_t data)
{
	// fsmoon:   0x00 -> 0x80 -> 0xa0 -> 0xc0 -> 0xf0
	//           (read latch as relative)
	// prinmak2: 0x00 -> 0x20 -> 0x40 -> 0x60 -> 0x60
	//           (keeps reading "delta" but never reset it, absolute mode)
	// biblems2: 0x0f -> 0x2f -> 0x4f -> 0x6f -> 0xef
	//           (latches a delta reset then reads delta diff, relative mode)

	const u8 mouse_x = ioport("MOUSE_X")->read();
	const u8 mouse_y = ioport("MOUSE_Y")->read();
	m_mouse.dx = (mouse_x - m_mouse.prev_dx) & 0xff;
	m_mouse.dy = (mouse_y - m_mouse.prev_dy) & 0xff;

	if ((m_mouse.control & 0x80) == 0 && data & 0x80)
	{
		m_mouse.lx = m_mouse.dx & 0xff;
		m_mouse.ly = m_mouse.dy & 0xff;
		m_mouse.prev_dx = mouse_x;
		m_mouse.prev_dy = mouse_y;
	}

	m_mouse.control = data;
}

// extended port $bfdb
u8 pc9801vm_state::mouse_freq_r(offs_t offset)
{
	// TODO: not all models support read-back
	// PC-9801DA: 0xff
	// H-98S: reads frequency only, '1' everything else
	// PC-9801US: frequency & irq cycle, '0' everything else
	// PC-9801Xa: frequency, '0' everything else
	popmessage("Read mouse $bfdb, contact MAMEdev");
	return m_mouse.freq_reg;
}

void pc9801vm_state::mouse_freq_w(offs_t offset, u8 data)
{
	// Port unavailable on PC-9871 1st gen & PC-9871/K, PC-9801F3, PC-9801M2, PC-9801M3
	// Some if not all of those have hardcoded HW switch for changing the timing instead
	/*
	 * xxxx xx-- mouse irq cycle setting (edge selection?)
	 * 0000 10-- ^ set on POST
	 * 0000 00-- ^ set by MS-DOS 6.20
	 * 0010 0111 ^ jastrike (main menu)
	 * 1010 0100 ^ jastrike (gameplay)
	 * ---- --xx mouse irq frequency setting (120 >> x) Hz
	 */
	logerror("%s: mouse $bfdb register write %02x\n",machine().describe_context(), data);

	// jastrike definitely don't intend to write to frequency too when
	// feeding 0x27/0xa* values at PC=156ec
	// (without this options menu barely detects any mouse input)
	if (data & 0xfc)
		return;

	m_mouse.freq_reg = data & 3;
	m_mouse.freq_index = 0;
}

/****************************************
*
* UPD765 interface
*
****************************************/

static void pc9801_floppies(device_slot_interface &device)
{
	device.option_add("525dd", TEAC_FD_55F);
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35hd", FLOPPY_35_HD);
}

void pc9801vm_state::fdc_irq_w(int state)
{
	if(m_fdc_mode & 1)
		m_pic2->ir3_w(state);
	else
		m_pic2->ir2_w(state);
}

void pc9801vm_state::fdc_drq_w(int state)
{
	if(m_fdc_mode & 1)
		m_dmac->dreq2_w(state ^ 1);
	else
		m_dmac->dreq3_w(state ^ 1);
}

uint32_t pc9801vm_state::a20_286(bool state)
{
	return (state ? 0xffffff : 0x0fffff);
}

/****************************************
*
* Init emulation status
*
****************************************/

void pc9801_state::pc9801_palette(palette_device &palette) const
{
	for(int i = 0; i < 8; i++)
		palette.set_pen_color(i, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));

	for(int i = 8; i < palette.entries(); i++)
		palette.set_pen_color(i, rgb_t::black());
}

MACHINE_START_MEMBER(pc9801_state,pc9801_common)
{
	m_rtc->cs_w(1);
	m_rtc->oe_w(1);
}

MACHINE_START_MEMBER(pc9801_state,pc9801f)
{
	MACHINE_START_CALL_MEMBER(pc9801_common);

	m_fdc_2hd->set_rate(500000);

	m_sys_type = 0x00 >> 6;
}

MACHINE_START_MEMBER(pc9801vm_state,pc9801rs)
{
	MACHINE_START_CALL_MEMBER(pc9801_common);

	m_sys_type = 0x80 >> 6;

	m_fdc_timer = timer_alloc(FUNC(pc9801vm_state::fdc_trigger), this);

	save_item(NAME(m_dac1bit_disable));

	save_item(NAME(m_dma_access_ctrl));

	save_item(NAME(m_egc.regs));
	save_item(NAME(m_egc.pat));
	save_item(NAME(m_egc.src));
	save_item(NAME(m_egc.count));
	save_item(NAME(m_egc.leftover));
	save_item(NAME(m_egc.first));
	save_item(NAME(m_egc.start));
	save_item(NAME(m_egc.mask));

	save_item(STRUCT_MEMBER(m_grcg, mode));
	//  save_pointer(STRUCT_MEMBER(m_grcg, tile), 4);
	save_item(STRUCT_MEMBER(m_grcg, tile_index));

	save_item(NAME(m_vram_bank));
}

MACHINE_START_MEMBER(pc9801us_state,pc9801us)
{
	MACHINE_START_CALL_MEMBER(pc9801rs);
}

MACHINE_START_MEMBER(pc9801bx_state,pc9801bx2)
{
	MACHINE_START_CALL_MEMBER(pc9801us);

	// ...
}

MACHINE_RESET_MEMBER(pc9801_state,pc9801_common)
{
	memset(m_tvram.get(), 0, sizeof(uint16_t) * 0x2000);

	m_nmi_ff = 0;
	m_mouse.control = 0xff;
	m_mouse.freq_reg = 0;
	m_mouse.freq_index = 0;
	m_mouse.lx = m_mouse.ly = m_mouse.prev_dx = m_mouse.prev_dy = m_mouse.dx = m_mouse.dy = 0;
	m_dma_autoinc[0] = m_dma_autoinc[1] = m_dma_autoinc[2] = m_dma_autoinc[3] = 0;
}

MACHINE_RESET_MEMBER(pc9801_state,pc9801f)
{
	MACHINE_RESET_CALL_MEMBER(pc9801_common);

	m_beeper->set_state(0);
}

MACHINE_RESET_MEMBER(pc9801vm_state,pc9801rs)
{
	MACHINE_RESET_CALL_MEMBER(pc9801_common);

	m_gate_a20 = 0;
	m_fdc_mode = 3;
	m_fdc_3mode.dev_sel = 2;
	m_fdc_3mode.access_144mb = false;
	fdc_set_density_mode(true); // 2HD
	// 0xfb on PC98XL
	// TODO: breaks UART setup for pc9801rs
	// m_dma_access_ctrl = 0xfe;
	m_dma_access_ctrl = 0;
	m_ide_sel = 0;
	m_maincpu->set_input_line(INPUT_LINE_A20, m_gate_a20);

	if(memregion("ide"))
	{
		if(!(ioport("BIOS_LOAD")->read() & 4))
			m_maincpu->space(AS_PROGRAM).install_rom(0xd8000, 0xd9fff, memregion("ide")->base());
		else
			m_maincpu->space(AS_PROGRAM).install_rom(0xd8000, 0xd9fff, memregion("ide")->base() + 0x2000);
	}

	m_dac1bit_disable = true;

	// flashb in particular don't initialize the mask in 16 color mode
	m_egc.regs[0] = 0xfff0;
	m_egc.regs[1] = 0x00ff;
	m_egc.mask = 0xffff;
	m_egc.regs[7] = 0x000f;
}

MACHINE_RESET_MEMBER(pc9801bx_state,pc9801bx2)
{
	MACHINE_RESET_CALL_MEMBER(pc9801rs);

	// TODO: if returning default 0xfe / 0xfb then it never ever surpass the "SYSTEM SHUTDOWN" even with a soft reset
	m_dma_access_ctrl = 0x00;
}

void pc9801_state::vrtc_irq(int state)
{
	if(state)
		m_pic1->ir2_w(1);
}


void pc98_base_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_PC98_FORMAT);
	fr.add(FLOPPY_PC98FDI_FORMAT);
	fr.add(FLOPPY_FDD_FORMAT);
	fr.add(FLOPPY_DCP_FORMAT);
	fr.add(FLOPPY_DIP_FORMAT);
	fr.add(FLOPPY_NFD_FORMAT);
	// *nix/FreeBSD may distribute with this
	fr.add(FLOPPY_IMG_FORMAT);
}

TIMER_DEVICE_CALLBACK_MEMBER( pc9801_state::mouse_irq_cb )
{
	// irq mask
	if((m_mouse.control & 0x10) == 0)
	{
		m_mouse.freq_index ++;

//      logerror("%02x\n",m_mouse.freq_index);
		if(m_mouse.freq_index > m_mouse.freq_reg)
		{
//          logerror("irq %02x\n",m_mouse.freq_reg);
			m_mouse.freq_index = 0;
			m_pic2->ir5_w(0);
			m_pic2->ir5_w(1);
		}
	}
}

void pc9801_atapi_devices(device_slot_interface &device)
{
	device.option_add("pc98_cd", PC98_CD);
}

void pc9801_state::config_video(machine_config &config)
{
	m_hgdc[0]->set_addrmap(0, &pc9801_state::upd7220_1_map);
	m_hgdc[0]->set_draw_text(FUNC(pc9801_state::hgdc_draw_text));
	m_hgdc[0]->vsync_wr_callback().set(m_hgdc[1], FUNC(upd7220_device::ext_sync_w));
	m_hgdc[0]->vsync_wr_callback().append(FUNC(pc9801_state::vrtc_irq));

	m_hgdc[1]->set_addrmap(0, &pc9801_state::upd7220_2_map);
	m_hgdc[1]->set_display_pixels(FUNC(pc9801_state::hgdc_display_pixels));
}

void pc9801vm_state::config_video(machine_config &config)
{
	pc9801_state::config_video(config);
	m_hgdc[1]->set_addrmap(0, &pc9801vm_state::upd7220_grcg_2_map);
}

void pc9801_state::config_keyboard(machine_config &config)
{
	I8251(config, m_sio_kbd, 0);
	m_sio_kbd->txd_handler().set("keyb", FUNC(pc98_kbd_device::input_txd));
	m_sio_kbd->dtr_handler().set("keyb", FUNC(pc98_kbd_device::input_rty));
	m_sio_kbd->rts_handler().set("keyb", FUNC(pc98_kbd_device::input_kbde));
	m_sio_kbd->write_cts(0);
	m_sio_kbd->write_dsr(0);
	m_sio_kbd->rxrdy_handler().set(m_pic1, FUNC(pic8259_device::ir1_w));

	clock_device &kbd_clock(CLOCK(config, "kbd_clock", 19'200));
	kbd_clock.signal_handler().set(m_sio_kbd, FUNC(i8251_device::write_rxc));
	kbd_clock.signal_handler().append(m_sio_kbd, FUNC(i8251_device::write_txc));

	PC98_KBD(config, m_keyb, 0);
	m_keyb->rxd_callback().set("sio_kbd", FUNC(i8251_device::write_rxd));
}

void pc9801_state::pit_clock_config(machine_config &config, const XTAL clock)
{
	m_pit->set_clk<0>(clock);
	m_pit->set_clk<1>(clock);
	m_pit->set_clk<2>(clock);
}

void pc9801_state::pc9801_mouse(machine_config &config)
{
	I8255(config, m_ppi_mouse);
	m_ppi_mouse->in_pa_callback().set(FUNC(pc9801_state::ppi_mouse_porta_r));
	m_ppi_mouse->out_pa_callback().set(FUNC(pc9801_state::ppi_mouse_porta_w));
	// Regular vanilla doesn't have readouts of these
	// (since mouse isn't built-in but comes from an external board at best)
//  m_ppi_mouse->in_pb_callback().set_ioport("DSW3");
	m_ppi_mouse->in_pb_callback().set_constant(0xff);
	m_ppi_mouse->out_pb_callback().set(FUNC(pc9801_state::ppi_mouse_portb_w));
//  m_ppi_mouse->in_pc_callback().set_ioport("DSW4");
	m_ppi_mouse->in_pc_callback().set_constant(0xff);
	m_ppi_mouse->out_pc_callback().set(FUNC(pc9801_state::ppi_mouse_portc_w));

	// TODO: timing is configurable
	TIMER(config, "mouse_timer").configure_periodic(FUNC(pc9801_state::mouse_irq_cb), attotime::from_hz(120));
}

void pc9801_state::pc9801_cbus(machine_config &config)
{
	PC98_CBUS_ROOT(config, m_cbus_root, 0);
	m_cbus_root->int_cb<0>().set("pic8259_master", FUNC(pic8259_device::ir3_w));
	m_cbus_root->int_cb<1>().set("pic8259_master", FUNC(pic8259_device::ir5_w));
	m_cbus_root->int_cb<2>().set("pic8259_master", FUNC(pic8259_device::ir6_w));
	m_cbus_root->int_cb<3>().set("pic8259_slave", FUNC(pic8259_device::ir1_w));
	m_cbus_root->int_cb<4>().set("pic8259_slave", FUNC(pic8259_device::ir3_w)); // INT42
	m_cbus_root->int_cb<5>().set("pic8259_slave", FUNC(pic8259_device::ir4_w));
	m_cbus_root->int_cb<6>().set("pic8259_slave", FUNC(pic8259_device::ir5_w));
	m_cbus_root->int_cb<7>().set("pic8259_slave", FUNC(pic8259_device::ir2_w)); // INT41
	m_cbus_root->drq_cb<0>().set(m_dmac, FUNC(am9517a_device::dreq0_w)).invert();
	m_cbus_root->drq_cb<3>().set(m_dmac, FUNC(am9517a_device::dreq3_w)).invert();
}

void pc9801vm_state::cdrom_headphones(device_t *device)
{
	cdda_device *cdda = device->subdevice<cdda_device>("cdda");
	cdda->add_route(0, "^^headphone", 1.0, 0);
	cdda->add_route(1, "^^headphone", 1.0, 1);
}

void pc9801vm_state::pc9801_ide(machine_config &config)
{
	SPEAKER(config, "headphone", 2).front();
	ATA_INTERFACE(config, m_ide[0]).options(ata_devices, "hdd", nullptr, false);
	m_ide[0]->irq_handler().set("ideirq", FUNC(input_merger_device::in_w<0>));
	ATA_INTERFACE(config, m_ide[1]).options(pc9801_atapi_devices, "pc98_cd", nullptr, false);
	m_ide[1]->irq_handler().set("ideirq", FUNC(input_merger_device::in_w<1>));
	m_ide[1]->slot(0).set_option_machine_config("pc98_cd", cdrom_headphones);

	INPUT_MERGER_ANY_HIGH(config, "ideirq").output_handler().set("pic8259_slave", FUNC(pic8259_device::ir1_w));

	SOFTWARE_LIST(config, "hdd_list").set_original("pc98_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("pc98_cd");
}

void pc98_base_state::pc9801_serial(machine_config &config)
{
	// clocked by PIT channel 2
	I8251(config, m_sio_rs, 0);
	m_sio_rs->txd_handler().set("serial", FUNC(rs232_port_device::write_txd));
	m_sio_rs->rts_handler().set("serial", FUNC(rs232_port_device::write_rts));
	m_sio_rs->dtr_handler().set("serial", FUNC(rs232_port_device::write_dtr));
	m_sio_rs->rxrdy_handler().set([this] (int state) { update_uart_irq<0>(state); });
	m_sio_rs->txempty_handler().set([this] (int state) { update_uart_irq<1>(state); });
	m_sio_rs->txrdy_handler().set([this] (int state) { update_uart_irq<2>(state); });

	rs232_port_device &rs232(RS232_PORT(config, "serial", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_sio_rs, FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set(m_sio_rs, FUNC(i8251_device::write_cts));
	rs232.dsr_handler().set(m_sio_rs, FUNC(i8251_device::write_dsr));
}

void pc9801_state::pc9801_common(machine_config &config)
{
	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(MAIN_CLOCK_X1); // heartbeat IRQ
	m_pit->out_handler<0>().set(m_pic1, FUNC(pic8259_device::ir0_w));
	m_pit->set_clk<1>(MAIN_CLOCK_X1); // Memory Refresh
	m_pit->set_clk<2>(MAIN_CLOCK_X1); // RS-232C
	m_pit->out_handler<2>().set(m_sio_rs, FUNC(i8251_device::write_txc));
	m_pit->out_handler<2>().append(m_sio_rs, FUNC(i8251_device::write_rxc));

	AM9517A(config, m_dmac, 5000000); // unknown clock, TODO: check channels 0 - 1
	m_dmac->dreq_active_low();
	m_dmac->out_hreq_callback().set(FUNC(pc9801_state::dma_hrq_changed));
	m_dmac->out_eop_callback().set(FUNC(pc9801_state::tc_w));
	m_dmac->in_memr_callback().set(FUNC(pc9801_state::dma_read_byte));
	m_dmac->out_memw_callback().set(FUNC(pc9801_state::dma_write_byte));
	m_dmac->in_ior_callback<0>().set([this] () { return m_cbus_root->dack_r(0); });
	m_dmac->out_iow_callback<0>().set([this] (u8 data) { m_cbus_root->dack_w(0, data); });

	m_dmac->in_ior_callback<2>().set(m_fdc_2hd, FUNC(upd765a_device::dma_r));
	m_dmac->out_iow_callback<2>().set(m_fdc_2hd, FUNC(upd765a_device::dma_w));
	m_dmac->in_ior_callback<3>().set([this] () { return m_cbus_root->dack_r(3); });
	m_dmac->out_iow_callback<3>().set([this] (u8 data) { m_cbus_root->dack_w(3, data); });

	m_dmac->out_dack_callback<0>().set(FUNC(pc9801_state::dack0_w));
	m_dmac->out_dack_callback<1>().set(FUNC(pc9801_state::dack1_w));
	m_dmac->out_dack_callback<2>().set(FUNC(pc9801_state::dack2_w));
	m_dmac->out_dack_callback<3>().set(FUNC(pc9801_state::dack3_w));

	PIC8259(config, m_pic1, 0);
	m_pic1->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic1->in_sp_callback().set_constant(1);
	m_pic1->read_slave_ack_callback().set(FUNC(pc9801_state::get_slave_ack));

	PIC8259(config, m_pic2, 0);
	m_pic2->out_int_callback().set(m_pic1, FUNC(pic8259_device::ir7_w)); // TODO: Check ir7_w
	m_pic2->in_sp_callback().set_constant(0);

	I8255(config, m_ppi_sys, 0);
	m_ppi_sys->in_pa_callback().set_ioport("DSW2");
	m_ppi_sys->in_pb_callback().set(FUNC(pc9801_state::ppi_sys_portb_r));
	m_ppi_sys->in_pc_callback().set_constant(0xa0); // 0x80 cpu triple fault reset flag?
//  m_ppi_sys->out_pc_callback().set(FUNC(pc9801_state::ppi_sys_portc_w));

	I8255(config, m_ppi_prn, 0);
	// TODO: other ports
	m_ppi_prn->in_pb_callback().set(FUNC(pc9801_state::ppi_prn_portb_r));

	config_keyboard(config);
	pc9801_mouse(config);

	pc9801_serial(config);

	PC98_MEMSW(config, m_memsw, 0);

	UPD765A(config, m_fdc_2hd, 8'000'000, true, true);
	m_fdc_2hd->intrq_wr_callback().set(m_pic2, FUNC(pic8259_device::ir3_w));
	m_fdc_2hd->drq_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq2_w)).invert();
	FLOPPY_CONNECTOR(config, "fdc_2hd:0", pc9801_floppies, "525hd", pc9801_state::floppy_formats);//.enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc_2hd:1", pc9801_floppies, "525hd", pc9801_state::floppy_formats);//.enable_sound(true);

	SOFTWARE_LIST(config, "disk_list").set_original("pc98");
	SOFTWARE_LIST(config, "disk_orig_list").set_original("pc98_flop_orig");
	SOFTWARE_LIST(config, "flop_generic_list").set_compatible("generic_flop_525").set_filter("pc98");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(21.0526_MHz_XTAL, 848, 0, 640, 440, 0, 400);
	m_screen->set_screen_update(FUNC(pc9801_state::screen_update));

	UPD7220(config, m_hgdc[0], 21.0526_MHz_XTAL / 8, "screen");
	UPD7220(config, m_hgdc[1], 21.0526_MHz_XTAL / 8, "screen");
	config_video(config);

	SPEAKER(config, "mono").front_center();

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pc9801);
}

void pc9801_state::config_floppy_525hd(machine_config &config)
{
	FLOPPY_CONNECTOR(config.replace(), "fdc_2hd:0", pc9801_floppies, "525hd", pc9801_state::floppy_formats);
	FLOPPY_CONNECTOR(config.replace(), "fdc_2hd:1", pc9801_floppies, "525hd", pc9801_state::floppy_formats);
}

void pc9801_state::config_floppy_35hd(machine_config &config)
{
	FLOPPY_CONNECTOR(config.replace(), "fdc_2hd:0", pc9801_floppies, "35hd", pc9801_state::floppy_formats);
	FLOPPY_CONNECTOR(config.replace(), "fdc_2hd:1", pc9801_floppies, "35hd", pc9801_state::floppy_formats);
}

void pc9801_state::pc9801(machine_config &config)
{
	I8086(config, m_maincpu, 5'000'000); // 5 MHz for vanilla, 8 MHz for direct children
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801_state::pc9801_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801_state::pc9801_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	pc9801_cbus(config);
	PC98_CBUS_SLOT(config, "cbus0", 0, "cbus_root", pc98_cbus_devices, "pc9801_26");
	PC98_CBUS_SLOT(config, "cbus1", 0, "cbus_root", pc98_cbus_devices, nullptr);
	PC98_CBUS_SLOT(config, "cbus2", 0, "cbus_root", pc98_cbus_devices, nullptr);
	PC98_CBUS_SLOT(config, "cbus3", 0, "cbus_root", pc98_cbus_devices, nullptr);
	PC98_CBUS_SLOT(config, "cbus4", 0, "cbus_root", pc98_cbus_devices, nullptr);
	PC98_CBUS_SLOT(config, "cbus5", 0, "cbus_root", pc98_cbus_devices, nullptr);

	// RAM 128KB (vanilla/F1/F2) ~ 256KB (F3/M2/M3) ~ 640KB (max)
	// TODO: really dedicates this space in N slots above
	PC98_CBUS_SLOT(config, "cbus_ram", 0, "cbus_root", pc98_cbus_ram_devices, "640kb");
//  RAM(config, m_ram).set_default_size("640K").set_extra_options("128K,256K,384K,512K");

	pc9801_common(config);
	m_ppi_sys->out_pc_callback().set(FUNC(pc9801_state::ppi_sys_beep_portc_w));

	MCFG_MACHINE_START_OVERRIDE(pc9801_state, pc9801f)
	MCFG_MACHINE_RESET_OVERRIDE(pc9801_state, pc9801f)

	UPD1990A(config, m_rtc);

	BEEP(config, m_beeper, 2400).add_route(ALL_OUTPUTS, "mono", 0.15);
	PALETTE(config, m_palette, FUNC(pc9801_state::pc9801_palette), 16);

	// TODO: should be PC80S31, using 'K variant for the better BIOS instead
	// (and no patch downstream).
	PC80S31K(config, "fdd_2d", XTAL(31'948'800) / 8);
}

void pc9801_state::pc9801f(machine_config &config)
{
	pc9801(config);
	PC98_CBUS_SLOT(config.replace(), "cbus1", 0, "cbus_root", pc98_cbus_devices, "fdd_2dd");
}

void pc9801_state::pc9801m(machine_config &config)
{
	pc9801(config);
	PC98_CBUS_SLOT(config.replace(), "cbus1", 0, "cbus_root", pc98_cbus_devices, "fdd_2hd");
}

void pc9801vm_state::pc9801vm(machine_config &config)
{
	V30(config, m_maincpu, 10'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801vm_state::pc9801vm_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801vm_state::pc9801vm_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	pc9801_common(config);
	pc9801_cbus(config);
	PC98_CBUS_SLOT(config, "cbus0", 0, "cbus_root", pc98_cbus_devices, "pc9801_26");
	PC98_CBUS_SLOT(config, "cbus1", 0, "cbus_root", pc98_cbus_devices, nullptr);
	PC98_CBUS_SLOT(config, "cbus2", 0, "cbus_root", pc98_cbus_devices, nullptr);
	PC98_CBUS_SLOT(config, "cbus3", 0, "cbus_root", pc98_cbus_devices, nullptr);

	m_ppi_sys->out_pc_callback().set(FUNC(pc9801vm_state::ppi_sys_dac_portc_w));
	// TODO: verify if it needs invert();
	m_pit->out_handler<1>().set( m_dac1bit, FUNC(speaker_sound_device::level_w));

	m_ppi_mouse->in_pb_callback().set(FUNC(pc9801vm_state::ppi_mouse_portb_r));
	m_ppi_mouse->in_pc_callback().set(FUNC(pc9801vm_state::ppi_mouse_portc_r));

	ADDRESS_MAP_BANK(config, m_ipl).set_map(&pc9801vm_state::ipl_bank).set_options(ENDIANNESS_LITTLE, 16, 18, 0x18000);

	// RAM 384KB (VM0/VM2/VM4) ~ 640KB (VM21/VM11)
	PC98_CBUS_SLOT(config, "cbus_ram", 0, "cbus_root", pc98_cbus_ram_devices, "640kb");
//  RAM(config, m_ram).set_default_size("640K").set_extra_options("384K");

	MCFG_MACHINE_START_OVERRIDE(pc9801vm_state, pc9801rs)
	MCFG_MACHINE_RESET_OVERRIDE(pc9801vm_state, pc9801_common)

	m_dmac->set_clock(MAIN_CLOCK_X1*8); // unknown clock

	UPD4990A(config, m_rtc);

	m_fdc_2hd->intrq_wr_callback().set(FUNC(pc9801vm_state::fdc_irq_w));
	m_fdc_2hd->drq_wr_callback().set(FUNC(pc9801vm_state::fdc_drq_w));
	// ch. 3 used when in 2DD mode (mightyhd, rogue)
	// TODO: should lock as everything else depending on mode bit 0
	m_dmac->in_ior_callback<3>().set(m_fdc_2hd, FUNC(upd765a_device::dma_r));
	m_dmac->out_iow_callback<3>().set(m_fdc_2hd, FUNC(upd765a_device::dma_w));

//  DAC_1BIT(config, m_dac1bit, 0).set_output_range(-1, 1).add_route(ALL_OUTPUTS, "mono", 0.15);
	SPEAKER_SOUND(config, m_dac1bit).add_route(ALL_OUTPUTS, "mono", 0.40);

	// analog mode optional on earlier VM, with PC-9801-24 gfx board
	PALETTE(config, m_palette, FUNC(pc9801vm_state::pc9801_palette), 16 + 16);
}

// UV is essentially a VM with 3.5" 2DD drives
// Released as UV2, UV21 then UV11 (UV21 but smaller?)
void pc9801vm_state::pc9801uv(machine_config &config)
{
	pc9801vm(config);

	FLOPPY_CONNECTOR(config.replace(), "fdc_2hd:0", pc9801_floppies, "35dd", pc9801_state::floppy_formats);
	FLOPPY_CONNECTOR(config.replace(), "fdc_2hd:1", pc9801_floppies, "35dd", pc9801_state::floppy_formats);

	// RAM 384KB (UV2) ~ 640KB (UV21/ UV11)
//  m_ram->set_default_size("640K").set_extra_options("384K");
}

void pc9801vm_state::pc9801ux(machine_config &config)
{
	pc9801vm(config);
	i80286_cpu_device &maincpu(I80286(config.replace(), m_maincpu, 10000000));
	maincpu.set_addrmap(AS_PROGRAM, &pc9801vm_state::pc9801ux_map);
	maincpu.set_addrmap(AS_IO, &pc9801vm_state::pc9801vm_io);
	maincpu.set_a20_callback(FUNC(pc9801vm_state::a20_286));
	maincpu.set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	config_floppy_35hd(config);
//  AM9157A(config, "i8237", 10000000); // unknown clock

	MCFG_MACHINE_START_OVERRIDE(pc9801vm_state, pc9801rs)
	MCFG_MACHINE_RESET_OVERRIDE(pc9801vm_state, pc9801rs)

	// RAM 640 KB ~ 6.6MB
	config.device_remove("cbus_ram");
	PC9801_54_SIMM(config, "simm", pc9801ux_simm_options, "2mb");
//  m_ram->set_default_size("2M");
//  m_ram->set_extra_options("640K,4M,7M");

	// 20MB SASI HDD (UV41 only)
}

void pc9801vm_state::pc9801vx(machine_config &config)
{
	pc9801vm(config);
	i80286_cpu_device &maincpu(I80286(config.replace(), m_maincpu, 10000000));
	maincpu.set_addrmap(AS_PROGRAM, &pc9801vm_state::pc9801vx_map);
	maincpu.set_addrmap(AS_IO, &pc9801vm_state::pc9801vm_io);
	maincpu.set_a20_callback(FUNC(pc9801vm_state::a20_286));
	maincpu.set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	config_floppy_525hd(config);

	// TODO: EGC initial buggy revision
	// Reportedly has a bug with a RMW op, details TBD
	// ...
	MCFG_MACHINE_START_OVERRIDE(pc9801vm_state, pc9801rs)
	MCFG_MACHINE_RESET_OVERRIDE(pc9801vm_state, pc9801rs)

	// RAM 640 KB ~ 8.6MB
	config.device_remove("cbus_ram");
	PC9801_54_SIMM(config, "simm", pc9801vx_simm_options, "2mb");
//  m_ram->set_default_size("2M");
//  m_ram->set_extra_options("640K,4M,7M,8M,9M");

	// GDC & EGC, DAC1BIT built-in
	// Either 2x 5.25 or 2x 3.5 internal floppy drives
	// 4x C-Bus slots (3x plus 1x dedicated RAM?)
}

void pc9801vm_state::pc9801dx(machine_config &config)
{
	pc9801vm(config);
	i80286_cpu_device &maincpu(I80286(config.replace(), m_maincpu, 12000000));
	maincpu.set_addrmap(AS_PROGRAM, &pc9801vm_state::pc9801dx_map);
	maincpu.set_addrmap(AS_IO, &pc9801vm_state::pc9801vm_io);
	maincpu.set_a20_callback(FUNC(pc9801vm_state::a20_286));
	maincpu.set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	config_floppy_525hd(config);
//  AM9157A(config, "i8237", 10000000); // unknown clock

	MCFG_MACHINE_START_OVERRIDE(pc9801vm_state, pc9801rs)
	MCFG_MACHINE_RESET_OVERRIDE(pc9801vm_state, pc9801rs)

	// RAM 640KB ~ 14.6MB
	config.device_remove("cbus_ram");
	PC9801_54_SIMM(config, "simm", pc9801dx_simm_options, "2mb");
}

void pc9801vm_state::pc9801rs(machine_config &config)
{
	pc9801vm(config);
	I386SX(config.replace(), m_maincpu, MAIN_CLOCK_X1*8); // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801vm_state::pc9801rs_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801vm_state::pc9801rs_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	MCFG_MACHINE_START_OVERRIDE(pc9801vm_state, pc9801rs)
	MCFG_MACHINE_RESET_OVERRIDE(pc9801vm_state, pc9801rs)

	pc9801_ide(config);

	// RAM 640KB ~ 14.6MB (with dedicated memory slot)
	config.device_remove("cbus_ram");
	PC9801_54_SIMM(config, "simm", pc9801dx_simm_options, "2mb");
//  RAM(config, m_ram).set_default_size("2M").set_extra_options("640K,4M,8M,14M,15M");
}

void pc9801us_state::pc9801us(machine_config &config)
{
	pc9801rs(config);
	const XTAL xtal = BASE_CLOCK / 2; // ~16 MHz
	I386SX(config.replace(), m_maincpu, xtal);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801us_state::pc9801rs_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801us_state::pc9801us_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	config_floppy_35hd(config);

	pit_clock_config(config, xtal / 4);

	PC98_119_KBD(config.replace(), m_keyb, 0);
	m_keyb->rxd_callback().set("sio_kbd", FUNC(i8251_device::write_rxd));

	PC98_SDIP(config, "sdip", 0);

	// RAM 640KB ~ 14.6MB
	// m_ram->set_default_size("2M");
	// m_ram->set_extra_options("640K,4M,8M,14M,15M");
}

void pc9801us_state::pc9801fs(machine_config &config)
{
	pc9801rs(config);
	const XTAL xtal = XTAL(20'000'000); // ~20 MHz
	I386SX(config.replace(), m_maincpu, xtal);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801us_state::pc9801rs_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801us_state::pc9801us_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	// optional 3'5 floppies x2
	config_floppy_525hd(config);

	// optional SCSI HDD

	pit_clock_config(config, xtal / 4);

//  PC98_119_KBD(config.replace(), m_keyb, 0);
//  m_keyb->rxd_callback().set("sio_kbd", FUNC(i8251_device::write_rxd));

	PC98_SDIP(config, "sdip", 0);

	// RAM 640KB ~ 14.6MB
	// m_ram->set_default_size("2M");
	// m_ram->set_extra_options("640K,4M,8M,14M,15M");
}

void pc9801bx_state::pc9801bx2(machine_config &config)
{
	pc9801rs(config);
	const XTAL xtal = XTAL(25'000'000);
	I486(config.replace(), m_maincpu, xtal); // i486sx, ODP upgradeable
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801bx_state::pc9801bx2_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801bx_state::pc9801bx2_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	MCFG_MACHINE_START_OVERRIDE(pc9801bx_state, pc9801bx2)
	MCFG_MACHINE_RESET_OVERRIDE(pc9801bx_state, pc9801bx2)

	pit_clock_config(config, xtal / 4); // unknown, fixes timer error at POST, /4 ~ /7

	PC98_SDIP(config, "sdip", 0);

	// RAM 1.8 MB (U2/M2) / 3.6 MB (U7) ~ 19.6 MB (from EMS?)
	config.device_remove("simm");
	PC9801_61_SIMM(config, "simm", pc9801bx2_simm_options, "2mb");
//  m_ram->set_default_size("2M");
//  m_ram->set_extra_options("640K,4M,7M,8M,14M,20M");

	// GDC & EGC, DAC1BIT built-in
	// 2x 3.5/5.25 internal floppy drives or 1x 3.5 and 120MB IDE HDD
	// 1x mountable File Bay
	// 3x C-Bus slots
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
"vanilla" - μPD8086, 5 MHz?
*/

ROM_START( pc9801 )
	ROM_REGION16_LE( 0x18000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ruq2g 06.bin", 0x14000, 0x2000, CRC(ee7b336b) SHA1(0691ebfcb9a8ce56ca303c552f634e953bb2ea7c) )
	ROM_LOAD16_BYTE( "ruq4f 06.bin", 0x14001, 0x2000, CRC(1e5c38c4) SHA1(5a8681ae0b1c3248d81a5b6707595d85cabe6bc8) )
	ROM_LOAD16_BYTE( "ruq2f 06.bin", 0x10000, 0x2000, CRC(be95afa5) SHA1(137fc4dd10ecc9f058b819df89a67df819a6509c) )
	ROM_LOAD16_BYTE( "ruq4e 06.bin", 0x10001, 0x2000, CRC(bc425f21) SHA1(f688ef89ebe3993dcbf70608d996067e92176be1) )
	ROM_LOAD16_BYTE( "ruq2e 06.bin", 0x0c000, 0x2000, CRC(16a3eaca) SHA1(345c1764e1b8aa5f3baa876658cf4cd224351fae) )
	ROM_LOAD16_BYTE( "ruq4d 06.bin", 0x0c001, 0x2000, CRC(0ca07388) SHA1(bdd564d19fcfa3dff8a695e2386c94defadcb164) )
	ROM_LOAD16_BYTE( "ruq2d 06.bin", 0x08000, 0x2000, CRC(907d0263) SHA1(30ba910424c99ae4c54eb2be5472258a3d5e4f29) )
	ROM_LOAD16_BYTE( "ruq4b 06.bin", 0x08001, 0x2000, CRC(b41d15e7) SHA1(321ec22e50fd2ee69f73c1e3f11c9fd07afa46fc) )
	ROM_LOAD16_BYTE( "ruq1f 06.bin", 0x00000, 0x2000, CRC(12d6ea62) SHA1(3a612428aaf3120ec00c10d709674535668f1d65) )
	ROM_LOAD16_BYTE( "ruq4h 06.bin", 0x00001, 0x2000, CRC(348992b9) SHA1(f5735f57305fd6585b2db1c81d34bb7ba2ed7510) )
	ROM_LOAD16_BYTE( "ruq1g 06.bin", 0x04000, 0x2000, CRC(d4ea8a62) SHA1(c899ea64ce8652a5b6976d62466efe2864cfb049) )
	ROM_LOAD16_BYTE( "ruq4g 06.bin", 0x04001, 0x2000, CRC(c1470ae5) SHA1(4eb31b2ad0b8f0dfad99bb67ada9e5853d5af4a1) )

	ROM_REGION( 0x80000, "chargen", 0 )
	// TODO: original dump, needs heavy bitswap mods
	ROM_LOAD( "sfz4w 00.bin",   0x00000, 0x02000, CRC(11197271) SHA1(8dbd2f25daeed545ea2c74d849f0a209ceaf4dd7) )
	// taken from i386 model
	ROM_LOAD( "d23128c-17.bin", 0x00000, 0x00800, BAD_DUMP CRC(eea57180) SHA1(4aa037c684b72ad4521212928137d3369174eb1e) )
	// bad dump, 8x16 charset? (it's on the kanji board)
	ROM_LOAD("hn613128pac8.bin",0x00800, 0x01000, BAD_DUMP CRC(b5a15b5c) SHA1(e5f071edb72a5e9a8b8b1c23cf94a74d24cb648e) )

	ROM_REGION( 0x80000, "raw_kanji", ROMREGION_ERASEFF )
	// original pc9801f dump, half size
	ROM_LOAD16_BYTE( "24256c-x01.bin", 0x00000, 0x4000, BAD_DUMP CRC(28ec1375) SHA1(9d8e98e703ce0f483df17c79f7e841c5c5cd1692) )
	ROM_CONTINUE(                      0x20000, 0x4000  )
	ROM_LOAD16_BYTE( "24256c-x02.bin", 0x00001, 0x4000, BAD_DUMP CRC(90985158) SHA1(78fb106131a3f4eb054e87e00fe4f41193416d65) )
	ROM_CONTINUE(                      0x20001, 0x4000  )
	ROM_LOAD16_BYTE( "24256c-x03.bin", 0x40000, 0x4000, BAD_DUMP CRC(d4893543) SHA1(eb8c1bee0f694e1e0c145a24152222d4e444e86f) )
	ROM_CONTINUE(                      0x60000, 0x4000  )
	ROM_LOAD16_BYTE( "24256c-x04.bin", 0x40001, 0x4000, BAD_DUMP CRC(5dec0fc2) SHA1(41000da14d0805ed0801b31eb60623552e50e41c) )
	ROM_CONTINUE(                      0x60001, 0x4000  )

	ROM_REGION( 0x100000, "kanji", ROMREGION_ERASEFF )
	// raw extracted from pc9801vm (after driver_init conversion)
	ROM_LOAD( "kanji.bin", 0, 0x100000, BAD_DUMP CRC(2de4336f) SHA1(dd783d4dca5812561f853ad0307ae90420292f09) )

	ROM_REGION( 0x80000, "new_chargen", ROMREGION_ERASEFF )
ROM_END


/*
F - 8086 8
*/

ROM_START( pc9801f )
	ROM_REGION16_LE( 0x18000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "urm01-02.bin", 0x00000, 0x4000, CRC(cde04615) SHA1(8f6fb587c0522af7a8131b45d13f8ae8fc60e8cd) )
	ROM_LOAD16_BYTE( "urm02-02.bin", 0x00001, 0x4000, CRC(9e39b8d1) SHA1(df1f3467050a41537cb9d071e4034f0506f07eda) )
	ROM_LOAD16_BYTE( "urm03-02.bin", 0x08000, 0x4000, CRC(95e79064) SHA1(c27d96949fad82aeb26e316200c15a4891e1063f) )
	ROM_LOAD16_BYTE( "urm04-02.bin", 0x08001, 0x4000, CRC(e4855a53) SHA1(223f66482c77409706cfc64c214cec7237c364e9) )
	ROM_LOAD16_BYTE( "urm05-02.bin", 0x10000, 0x4000, CRC(ffefec65) SHA1(106e0d920e857e59da12225a489ca2756ca405c1) )
	ROM_LOAD16_BYTE( "urm06-02.bin", 0x10001, 0x4000, CRC(1147760b) SHA1(4e0299091dfd53ac7988d40c5a6775a10389faac) )

	ROM_REGION( 0x80000, "chargen", 0 )
	// note: ROM labels of following two may be swapped
	//original is a bad dump, this is taken from i386 model
	ROM_LOAD( "d23128c-17.bin", 0x00000, 0x00800, BAD_DUMP CRC(eea57180) SHA1(4aa037c684b72ad4521212928137d3369174eb1e) )
	//bad dump, 8x16 charset? (it's on the kanji board)
	ROM_LOAD("hn613128pac8.bin",0x00800, 0x01000, BAD_DUMP CRC(b5a15b5c) SHA1(e5f071edb72a5e9a8b8b1c23cf94a74d24cb648e) )

	ROM_REGION( 0x80000, "raw_kanji", ROMREGION_ERASEFF )
	// original pc9801f dump, half size
	ROM_LOAD16_BYTE( "24256c-x01.bin", 0x00000, 0x4000, BAD_DUMP CRC(28ec1375) SHA1(9d8e98e703ce0f483df17c79f7e841c5c5cd1692) )
	ROM_CONTINUE(                      0x20000, 0x4000  )
	ROM_LOAD16_BYTE( "24256c-x02.bin", 0x00001, 0x4000, BAD_DUMP CRC(90985158) SHA1(78fb106131a3f4eb054e87e00fe4f41193416d65) )
	ROM_CONTINUE(                      0x20001, 0x4000  )
	ROM_LOAD16_BYTE( "24256c-x03.bin", 0x40000, 0x4000, BAD_DUMP CRC(d4893543) SHA1(eb8c1bee0f694e1e0c145a24152222d4e444e86f) )
	ROM_CONTINUE(                      0x60000, 0x4000  )
	ROM_LOAD16_BYTE( "24256c-x04.bin", 0x40001, 0x4000, BAD_DUMP CRC(5dec0fc2) SHA1(41000da14d0805ed0801b31eb60623552e50e41c) )
	ROM_CONTINUE(                      0x60001, 0x4000  )

	ROM_REGION( 0x100000, "kanji", ROMREGION_ERASEFF )
	// raw extracted from pc9801vm (after driver_init conversion)
	ROM_LOAD( "kanji.bin", 0, 0x100000, BAD_DUMP CRC(2de4336f) SHA1(dd783d4dca5812561f853ad0307ae90420292f09) )

	ROM_REGION( 0x80000, "new_chargen", ROMREGION_ERASEFF )
ROM_END

/*
M2 - 8086 8
*/

ROM_START( pc9801m2 )
	ROM_REGION16_LE( 0x18000, "ipl", ROMREGION_ERASEFF )
	// from CSCP package, same as F?
	ROM_LOAD( "ipl_efm.rom", 0x00000, 0x18000, BAD_DUMP CRC(be00c88b) SHA1(1e7c0e61db5441f2a91c56c4085a7024c8d07c10) )

	ROM_REGION( 0x80000, "chargen", 0 )
	// note: ROM labels of following two may be swapped
	//original is a bad dump, this is taken from i386 model
	ROM_LOAD( "d23128c-17.bin", 0x00000, 0x00800, BAD_DUMP CRC(eea57180) SHA1(4aa037c684b72ad4521212928137d3369174eb1e) )
	//bad dump, 8x16 charset? (it's on the kanji board)
	ROM_LOAD("hn613128pac8.bin",0x00800, 0x01000, BAD_DUMP CRC(b5a15b5c) SHA1(e5f071edb72a5e9a8b8b1c23cf94a74d24cb648e) )

	ROM_REGION( 0x80000, "raw_kanji", ROMREGION_ERASEFF )
	// original pc9801f dump, half size
	ROM_LOAD16_BYTE( "24256c-x01.bin", 0x00000, 0x4000, BAD_DUMP CRC(28ec1375) SHA1(9d8e98e703ce0f483df17c79f7e841c5c5cd1692) )
	ROM_CONTINUE(                      0x20000, 0x4000  )
	ROM_LOAD16_BYTE( "24256c-x02.bin", 0x00001, 0x4000, BAD_DUMP CRC(90985158) SHA1(78fb106131a3f4eb054e87e00fe4f41193416d65) )
	ROM_CONTINUE(                      0x20001, 0x4000  )
	ROM_LOAD16_BYTE( "24256c-x03.bin", 0x40000, 0x4000, BAD_DUMP CRC(d4893543) SHA1(eb8c1bee0f694e1e0c145a24152222d4e444e86f) )
	ROM_CONTINUE(                      0x60000, 0x4000  )
	ROM_LOAD16_BYTE( "24256c-x04.bin", 0x40001, 0x4000, BAD_DUMP CRC(5dec0fc2) SHA1(41000da14d0805ed0801b31eb60623552e50e41c) )
	ROM_CONTINUE(                      0x60001, 0x4000  )

	ROM_REGION( 0x100000, "kanji", ROMREGION_ERASEFF )
	// raw extracted from pc9801vm (after driver_init conversion)
	ROM_LOAD( "kanji.bin", 0, 0x100000, BAD_DUMP CRC(2de4336f) SHA1(dd783d4dca5812561f853ad0307ae90420292f09) )

	ROM_REGION( 0x80000, "new_chargen", ROMREGION_ERASEFF )
ROM_END



/*
VM - V30 8/10

missing itf roms, if they exist
*/

ROM_START( pc9801vm )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
//  ROM_LOAD( "itf_ux.rom",  0x10000, 0x08000, BAD_DUMP CRC(c7942563) SHA1(61bb210d64c7264be939b11df1e9cd14ffeee3c9) )
//  ROM_LOAD( "bios_vm.rom", 0x18000, 0x18000, CRC(2e2d7cee) SHA1(159549f845dc70bf61955f9469d2281a0131b47f) )
	// bios
	ROM_LOAD16_BYTE( "cpu_board_1a_23128e.bin",   0x10001, 0x4000, CRC(9965c914) SHA1(1ed318b774340bd532ef02ac02f39a012354dbf8) )
	ROM_LOAD16_BYTE( "cpu_board_4a_d23128ec.bin", 0x10000, 0x4000, CRC(e7c24a70) SHA1(cc9584b8e56b391f103e9d559d397d0bc6d00b35) )
	ROM_LOAD16_BYTE( "cpu_board_2a_d23c256ec.bin", 0x08001, 0x4000, CRC(3874970d) SHA1(e50ec5ae38f00dbfd156288dd42c7f2a2bf8bc35) )
	ROM_CONTINUE( 0x00001, 0x4000 )
	ROM_LOAD16_BYTE( "cpu_board_3a_23c256e.bin",   0x08000, 0x4000, CRC(4128276e) SHA1(32acb7eee779a31838a17ce51b05a9a987af4099) )
	ROM_CONTINUE( 0x00000, 0x4000 )

	ROM_REGION( 0x80000, "chargen", 0 )
//  ROM_LOAD( "font_vm.rom",     0x000000, 0x046800, BAD_DUMP CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff) )
	// TODO: contains 8x8 "graphics" characters but we don't use them
	ROM_LOAD( "main_board_12f_d2364ec.bin", 0x000000, 0x002000, CRC(11197271) SHA1(8dbd2f25daeed545ea2c74d849f0a209ceaf4dd7) )

	ROM_REGION( 0x80000, "raw_kanji", ROMREGION_ERASEFF )
	// on main board, uPD23100 type roms
	// kanji and most other 16x16 characters
	ROM_LOAD16_BYTE( "main_board_12h_231000.bin", 0x00000, 0x20000, CRC(ecc2c062) SHA1(36c935c0f26c02a2b1ea46f5b6cd03fc11c7b003) )
	ROM_LOAD16_BYTE( "main_board_10h_231000.bin", 0x00001, 0x20000, CRC(91d78281) SHA1(85a18ad40e281e68071f91800201e43d78fb4f1c) )
	// 8x16 characters and the remaining 16x16 characters, with inverted bit order like 12f
	ROM_LOAD16_BYTE( "main_board_8h_d23256ac.bin", 0x40000, 0x04000, CRC(62a32ba6) SHA1(cdab480ae0dad9d128e52afb15e6c0b2b122cc3f) )
	ROM_CONTINUE( 0x40001, 0x04000 )

	ROM_REGION( 0x100000, "kanji", ROMREGION_ERASEFF )
	ROM_REGION( 0x80000, "new_chargen", ROMREGION_ERASEFF )

//  LOAD_KANJI_ROMS
//  LOAD_IDE_ROM
ROM_END

ROM_START( pc9801uv2 )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "d23c128ec-195.bin", 0x10000, 0x4000, CRC(082c86eb) SHA1(6f503b75906fd4932152f45c6d37c1e230934773) )
	ROM_LOAD16_BYTE( "d23c128ec-196.bin", 0x10001, 0x4000, CRC(d90b730b) SHA1(27f9b67c0454ee6107db20912f08f87ff682adcc) )
	ROM_LOAD16_BYTE( "d23c256ec-164.bin", 0x08000, 0x4000, CRC(d6cd9fef) SHA1(4ade6f891ee4c5ccb31031a520ab5ba757a6944c) )
	ROM_CONTINUE( 0x00000, 0x4000 )
	ROM_LOAD16_BYTE( "d23c256ec-165.bin", 0x08001, 0x4000, CRC(2d348381) SHA1(a1c7ebb7727380bcb879b2c609a1fe6cd5bfa0bb) )
	ROM_CONTINUE( 0x00001, 0x4000 )

	// borrowed from pc9801vm
	ROM_REGION( 0x80000, "chargen", 0 )
//  ROM_LOAD( "font_vm.rom",     0x000000, 0x046800, BAD_DUMP CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff) )
	// TODO: contains 8x8 "graphics" characters but we don't use them
	ROM_LOAD( "main_board_12f_d2364ec.bin", 0x000000, 0x002000, CRC(11197271) SHA1(8dbd2f25daeed545ea2c74d849f0a209ceaf4dd7) )

	ROM_REGION( 0x80000, "raw_kanji", ROMREGION_ERASEFF )
	// on main board, uPD23100 type roms
	// kanji and most other 16x16 characters
	ROM_LOAD16_BYTE( "main_board_12h_231000.bin", 0x00000, 0x20000, CRC(ecc2c062) SHA1(36c935c0f26c02a2b1ea46f5b6cd03fc11c7b003) )
	ROM_LOAD16_BYTE( "main_board_10h_231000.bin", 0x00001, 0x20000, CRC(91d78281) SHA1(85a18ad40e281e68071f91800201e43d78fb4f1c) )
	// 8x16 characters and the remaining 16x16 characters, with inverted bit order like 12f
	ROM_LOAD16_BYTE( "main_board_8h_d23256ac.bin", 0x40000, 0x04000, CRC(62a32ba6) SHA1(cdab480ae0dad9d128e52afb15e6c0b2b122cc3f) )
	ROM_CONTINUE( 0x40001, 0x04000 )

	ROM_REGION( 0x100000, "kanji", ROMREGION_ERASEFF )
	ROM_REGION( 0x80000, "new_chargen", ROMREGION_ERASEFF )

//  LOAD_KANJI_ROMS
//  LOAD_IDE_ROM
ROM_END

/*
VX - 80286 10 + V30 8

UVPROM label on extension board for CPU board (4 * NEC D27C256D-15):
YLL01/YLL02/YLL03/YLL04
00
(C) '86 NEC

Dump coming from a dead machine
*/

ROM_START( pc9801vx )
	ROM_REGION16_LE( 0x20000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "nec_d27c256d-15_cpu_extboard_yll01.bin", 0x00000, 0x08000, CRC(1b235313) SHA1(d2c5e2cea3ee0a643d3c5d384d134404b58db793) )
	ROM_LOAD16_BYTE( "nec_d27c256d-15_cpu_extboard_yll03.bin", 0x00001, 0x08000, CRC(33605ae3) SHA1(f644ff15c54c8568e643324f38aa3b6211912af0) )
	ROM_LOAD16_BYTE( "nec_d27c256d-15_cpu_extboard_yll02.bin", 0x10000, 0x08000, CRC(948f8658) SHA1(674378d4e90fee715ccfdd49378cd5c2fe8d7f62) )
	ROM_LOAD16_BYTE( "nec_d27c256d-15_cpu_extboard_yll04.bin", 0x10001, 0x08000, CRC(2ce2101b) SHA1(2158d022d5424daf6981bf4da0ab9613bf9646f5) )

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_COPY( "biosrom", 0x18000, 0x10000, 0x08000 )  //ITF ROM
	ROM_COPY( "biosrom", 0x08000, 0x18000, 0x08000 )  //BIOS ROM
	ROM_COPY( "biosrom", 0x00000, 0x20000, 0x08000 )
	ROM_COPY( "biosrom", 0x10000, 0x28000, 0x08000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_ux.rom",     0x000000, 0x046800, BAD_DUMP CRC(19a76eeb) SHA1(96a006e8515157a624599c2b53a581ae0dd560fd) )

	LOAD_KANJI_ROMS
//  LOAD_IDE_ROM
ROM_END

/*
UX - 80286 10 + V30 8
*/

ROM_START( pc9801ux )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf_ux.rom",  0x10000, 0x08000, CRC(c7942563) SHA1(61bb210d64c7264be939b11df1e9cd14ffeee3c9) )
	ROM_LOAD( "bios_ux.rom", 0x18000, 0x18000, BAD_DUMP CRC(97375ca2) SHA1(bfe458f671d90692104d0640730972ca8dc0a100) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_ux.rom",     0x000000, 0x046800, BAD_DUMP CRC(19a76eeb) SHA1(96a006e8515157a624599c2b53a581ae0dd560fd) )

	LOAD_KANJI_ROMS
//  LOAD_IDE_ROM
ROM_END

/*
DX - 80286 12 + V30 8
*/

ROM_START( pc9801dx )
	ROM_REGION16_LE( 0x40000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "hdk01_02.bin",  0x000000, 0x020000, CRC(bf8b25fd) SHA1(e86eb6b46d73aad1cc96945bd34bd728d882583e) )
	ROM_LOAD16_BYTE( "hdk02_02.bin",  0x000001, 0x020000, CRC(37f21929) SHA1(1bb7c2f09eed399a78c3668f4193429a1980acc9) )

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_COPY( "biosrom", 0x20000, 0x10000, 0x08000 )  //ITF ROM
	ROM_COPY( "biosrom", 0x28000, 0x18000, 0x08000 )  //BIOS ROM
	ROM_COPY( "biosrom", 0x30000, 0x20000, 0x08000 )
	ROM_COPY( "biosrom", 0x38000, 0x28000, 0x08000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_ux.rom",     0x000000, 0x046800, BAD_DUMP CRC(19a76eeb) SHA1(96a006e8515157a624599c2b53a581ae0dd560fd) )

	LOAD_KANJI_ROMS
//  LOAD_IDE_ROM
ROM_END

/*
US - i386SX @ 16 MHz

Built-in 2.5 IDE interface
*/

ROM_START( pc9801us )
	ROM_REGION16_LE( 0x80000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD( "lrh8a00.bin",  0x000000, 0x080000, CRC(a86d8cdb) SHA1(01c805274ca943c1febedda5ad85ba532aac949c) )

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// 0x0c000-0x0ffff sound ROM BIOS
	// 0x10000-0x13fff ^ mirror
	// 0x14000-0x17fff <empty>
	// 0x18000-0x191ff unknown, disk BIOS?
	// 0x20000-0x27fff ITF ROM
	// 0x40000-0x7ffff <empty>
	ROM_COPY( "biosrom", 0x20000, 0x10000, 0x08000 )  //ITF ROM
	ROM_COPY( "biosrom", 0x28000, 0x18000, 0x08000 )  //BIOS ROM
	ROM_COPY( "biosrom", 0x30000, 0x20000, 0x08000 )
	ROM_COPY( "biosrom", 0x38000, 0x28000, 0x08000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_ux.rom",     0x000000, 0x046800, BAD_DUMP CRC(19a76eeb) SHA1(96a006e8515157a624599c2b53a581ae0dd560fd) )

	LOAD_KANJI_ROMS

	ROM_REGION( 0x4000, "ide", ROMREGION_ERASEVAL(0xcb) )
	ROM_COPY( "biosrom", 0x18000, 0x00000, 0x02000 )
ROM_END

/*
FS - 80386 20
*/

ROM_START( pc9801fs )
	ROM_REGION16_LE( 0x40000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "kqx01_00.bin",  0x000000, 0x020000, CRC(4713d388) SHA1(9ae48fbe7b8ab7144e045e183ed88d2544d9a61c) )
	ROM_LOAD16_BYTE( "kqx02_00.bin",  0x000001, 0x020000, CRC(f55e42d6) SHA1(2ab0ae817e9abed984544c920182689127550ce3) )

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// 0x0c000-0x0ffff sound ROM BIOS
	// 0x10000-0x13fff ^ mirror
	// 0x14000-0x16fff <unknown>
	// 0x17000-0x17fff SCSI disk BIOS?
	// 0x18000-0x1ffff <empty>
	ROM_COPY( "biosrom", 0x20000, 0x10000, 0x08000 )  //ITF ROM
	ROM_COPY( "biosrom", 0x28000, 0x18000, 0x08000 )  //BIOS ROM
	ROM_COPY( "biosrom", 0x30000, 0x20000, 0x08000 )
	ROM_COPY( "biosrom", 0x38000, 0x28000, 0x08000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_ux.rom",     0x000000, 0x046800, BAD_DUMP CRC(19a76eeb) SHA1(96a006e8515157a624599c2b53a581ae0dd560fd) )

	LOAD_KANJI_ROMS
//  LOAD_IDE_ROM

//  ROM_REGION( 0x4000, "scsi", ROMREGION_ERASEVAL(0xcb) )
//  ROM_COPY( "biosrom", 0x16000, 0x00000, 0x02000 )
ROM_END

/*
RX - 80286 12 (no V30?)

The bios accesses PCI registers and $4be, can't be an RX
*/

ROM_START( pc9801rx )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf_rs.rom",  0x10000, 0x08000, BAD_DUMP CRC(c1815325) SHA1(a2fb11c000ed7c976520622cfb7940ed6ddc904e) )
	ROM_LOAD( "bios_rx.rom", 0x18000, 0x18000, BAD_DUMP CRC(0a682b93) SHA1(76a7360502fa0296ea93b4c537174610a834d367) )
	// fix csum
	ROM_FILL(0x2fffe, 1, 0x0d)

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_rx.rom",     0x000000, 0x046800, BAD_DUMP CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
RS - 386SX @ 16 MHz

(note: might be a different model!)
*/

ROM_START( pc9801rs )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf_rs.rom",  0x10000, 0x08000, CRC(c1815325) SHA1(a2fb11c000ed7c976520622cfb7940ed6ddc904e) )
	ROM_LOAD( "bios_rs.rom", 0x18000, 0x18000, BAD_DUMP CRC(315d2703) SHA1(4f208d1dbb68373080d23bff5636bb6b71eb7565) )

	/* following is an emulator memory dump, should be checked and eventually nuked if nothing worth is there */
	ROM_REGION( 0x100000, "memory", 0 )
	// refs for an "EPSON SCANNER"
	ROM_LOAD( "00000.rom", 0x00000, 0x8000, CRC(6e299128) SHA1(d0e7d016c005cdce53ea5ecac01c6f883b752b80) )
	// 1-filled
	ROM_LOAD( "c0000.rom", 0xc0000, 0x8000, CRC(1b43eabd) SHA1(ca711c69165e1fa5be72993b9a7870ef6d485249) )
	// sound BIOS at $cc000
	ROM_LOAD( "c8000.rom", 0xc8000, 0x8000, CRC(f2a262b0) SHA1(fe97d2068d18bbb7425d9774e2e56982df2aa1fb) )
	// 1-filled
	ROM_LOAD( "d0000.rom", 0xd0000, 0x8000, CRC(1b43eabd) SHA1(ca711c69165e1fa5be72993b9a7870ef6d485249) )
	// n-88 basic (86) v2.0
	ROM_LOAD( "e8000.rom", 0xe8000, 0x8000, CRC(4e32081e) SHA1(e23571273b7cad01aa116cb7414c5115a1093f85) )
	ROM_LOAD( "f0000.rom", 0xf0000, 0x8000, CRC(4da85a6c) SHA1(18dccfaf6329387c0c64cc4c91b32c25cde8bd5a) )
	ROM_LOAD( "f8000.rom", 0xf8000, 0x8000, CRC(2b1e45b1) SHA1(1fec35f17d96b2e2359e3c71670575ad9ff5007e) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_rs.rom", 0x00000, 0x46800, BAD_DUMP CRC(da370e7a) SHA1(584d0c7fde8c7eac1f76dc5e242102261a878c5e) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
BX2/U2 - 486SX @ 25 MHz, ODP upgradeable

RAM 1.8MB to 19.6MB
Floppy 3.5" x 2
C-Bus x 3
EGC

BX2/U7
minimum RAM bumped to 3.6MB
Floppy 3.5" x 1
120 MB IDE built-in

BX2/M2
Floppy 5.25" x 2, otherwise same as U2

*/

ROM_START( pc9801bx2 )
	ROM_REGION16_LE( 0x40000, "biosrom", ROMREGION_ERASEFF )
	// baddump: getitf98 format
	ROM_LOAD( "pc98bank0.bin",  0x00000, 0x08000, BAD_DUMP CRC(bfd100cc) SHA1(cf8e6a5679cca7761481abef0ba4b35ead39efdb) )
	ROM_LOAD( "pc98bank1.bin",  0x08000, 0x08000, BAD_DUMP CRC(d0562af8) SHA1(2c4fd27eb598f4b8a00f3e86941ba27007d58e47) )
	ROM_LOAD( "pc98bank2.bin",  0x10000, 0x08000, BAD_DUMP CRC(12818a14) SHA1(9c31e8ac85d78fa779d6bbc2095557065294ec09) )
	ROM_LOAD( "pc98bank3.bin",  0x18000, 0x08000, BAD_DUMP CRC(d0bda44e) SHA1(c1022a3b2be4d2a1e43914df9e4605254e5f99d5) )
	ROM_LOAD( "pc98bank4.bin",  0x20000, 0x08000, BAD_DUMP CRC(be8092f4) SHA1(12c8a166b8c6ebbef85568b67e1f098562883365) )
	ROM_LOAD( "pc98bank5.bin",  0x28000, 0x08000, BAD_DUMP CRC(4e32081e) SHA1(e23571273b7cad01aa116cb7414c5115a1093f85) )
	ROM_LOAD( "pc98bank6.bin",  0x30000, 0x08000, BAD_DUMP CRC(f878c160) SHA1(cad47f09075ffe4f7b51bb937c9f716c709d4596) )
	ROM_LOAD( "pc98bank7.bin",  0x38000, 0x08000, BAD_DUMP CRC(1bd6537b) SHA1(ff9ee1c976a12b87851635ce8991ac4ad607675b) )

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// 0x1a000: setup mode
	ROM_COPY( "biosrom", 0x20000, 0x10000, 0x8000 ) // ITF ROM
	ROM_COPY( "biosrom", 0x28000, 0x18000, 0x8000 ) // BIOS ROM
	ROM_COPY( "biosrom", 0x30000, 0x20000, 0x8000 )
	ROM_COPY( "biosrom", 0x38000, 0x28000, 0x8000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_rs.rom", 0x00000, 0x46800, BAD_DUMP CRC(da370e7a) SHA1(584d0c7fde8c7eac1f76dc5e242102261a878c5e) )

	LOAD_KANJI_ROMS

	ROM_REGION( 0x4000, "ide", ROMREGION_ERASEVAL(0xcb) )
	ROM_COPY( "biosrom", 0x18000, 0x00000, 0x02000 )
ROM_END

void pc9801_state::init_pc9801_kanji()
{
	#define copy_kanji_strip(_dst,_src,_fill_type) \
	for (uint32_t i = _dst, k = _src; i < _dst + 0x20; i++, k++) \
	{ \
		for (uint32_t j = 0; j < 0x20; j++) \
			kanji[j+(i << 5)] = _fill_type ? new_chargen[j+(k << 5)] : 0; \
	}
	uint32_t pcg_tile;
	uint8_t *kanji = memregion("kanji")->base();
	uint8_t *raw_kanji = memregion("raw_kanji")->base();
	uint8_t *new_chargen = memregion("new_chargen")->base();
	uint8_t *chargen = memregion("chargen")->base();

	/* Convert the ROM bitswap here from the original structure */
	/* TODO: kanji bitswap should be completely wrong, will check it out once that a dump is remade. */
	for (uint32_t i = 0; i < 0x80000 / 0x20; i++)
	{
		for (uint32_t j = 0; j < 0x20; j++)
		{
			pcg_tile = bitswap<16>(i,15,14,13,12,11,7,6,5,10,9,8,4,3,2,1,0) << 5;
			kanji[j+(i << 5)] = raw_kanji[j+pcg_tile];
		}
	}

	/* convert charset into even/odd structure */
	for (uint32_t i = 0; i < 0x80000 / 0x20; i++)
	{
		for (uint32_t j = 0; j < 0x10; j++)
		{
			new_chargen[j*2 + (i << 5)] = chargen[j + (i << 5)];
			new_chargen[j*2 + (i << 5) + 1] = chargen[j + (i << 5) + 0x10];
		}
	}

	/* now copy the data from the fake roms into our kanji struct */
	copy_kanji_strip(0x0800,   -1,0); copy_kanji_strip(0x0820,   -1,0); copy_kanji_strip(0x0840,   -1,0); copy_kanji_strip(0x0860,   -1,0);
	copy_kanji_strip(0x0900,   -1,0); copy_kanji_strip(0x0920,0x3c0,1); copy_kanji_strip(0x0940,0x3e0,1); copy_kanji_strip(0x0960,0x400,1);
	copy_kanji_strip(0x0a00,   -1,0); copy_kanji_strip(0x0a20,0x420,1); copy_kanji_strip(0x0a40,0x440,1); copy_kanji_strip(0x0a60,0x460,1);
	copy_kanji_strip(0x0b00,   -1,0); copy_kanji_strip(0x0b20,0x480,1); copy_kanji_strip(0x0b40,0x4a0,1); copy_kanji_strip(0x0b60,0x4c0,1);
	copy_kanji_strip(0x0c00,   -1,0); copy_kanji_strip(0x0c20,0x4e0,1); copy_kanji_strip(0x0c40,0x500,1); copy_kanji_strip(0x0c60,0x520,1);
	copy_kanji_strip(0x0d00,   -1,0); copy_kanji_strip(0x0d20,0x540,1); copy_kanji_strip(0x0d40,0x560,1); copy_kanji_strip(0x0d60,0x580,1);
	copy_kanji_strip(0x0e00,   -1,0); copy_kanji_strip(0x0e20,   -1,0); copy_kanji_strip(0x0e40,   -1,0); copy_kanji_strip(0x0e60,   -1,0);
	copy_kanji_strip(0x0f00,   -1,0); copy_kanji_strip(0x0f20,   -1,0); copy_kanji_strip(0x0f40,   -1,0); copy_kanji_strip(0x0f60,   -1,0);
	{
		int src_1,dst_1;

		for(src_1=0x1000,dst_1=0x660;src_1<0x8000;src_1+=0x100,dst_1+=0x60)
		{
			copy_kanji_strip(src_1,             -1,0);
			copy_kanji_strip(src_1+0x20,dst_1+0x00,1);
			copy_kanji_strip(src_1+0x40,dst_1+0x20,1);
			copy_kanji_strip(src_1+0x60,dst_1+0x40,1);
		}
	}
	#undef copy_kanji_strip
}

void pc9801vm_state::init_pc9801vm_kanji()
{
	uint32_t raw_tile;
	uint8_t *chargen = memregion("chargen")->base();
	uint8_t *raw_kanji = memregion("raw_kanji")->base();
	uint8_t *kanji = memregion("kanji")->base();

	/* swap bits for 8x8 characters, discard 8x8 "graphics" characters */
	/* TODO: should we keep and use the "graphics" characters? */
	for( uint32_t i = 0; i < 0x100; i++ )
	{
		for( uint32_t j = 0; j < 8; j++ )
		{
			chargen[i*8+j] = bitswap<8>(chargen[i*0x10+j],0,1,2,3,4,5,6,7);
		}
	}
	/* swap bits for 8x16 characters */
	for( uint32_t i = 0; i < 0x100; i++ )
	{
		for( uint32_t j = 0; j < 0x10; j++ )
		{
			chargen[0x100*8+i*0x10+j] = bitswap<8>(chargen[0x100*0x10+i*0x10+j],0,1,2,3,4,5,6,7);
		}
	}
	/* 16x16 0x0020-0x077f */
	for( uint32_t hibyte = 0x00; hibyte <= 0x07; hibyte++ )
	{
		for( uint32_t lobyte = 0x20; lobyte <= 0x7f; lobyte++ )
		{
			raw_tile = bitswap<16>(hibyte*0x100+lobyte,15,14,13,12,11,7,6,5,10,9,8,4,3,2,1,0) * 0x20;
			for( uint32_t line = 0; line < 0x20; line++ )
			{
				kanji[(hibyte*0x100+lobyte)*0x20+line] = raw_kanji[raw_tile+line];
			}
		}
	}
	/* 16x16 0x0820-0x0f7f (swapped bits) */
	for( uint32_t hibyte = 0x08; hibyte <= 0x0f; hibyte++ )
	{
		for( uint32_t lobyte = 0x20; lobyte <= 0x7f; lobyte++ )
		{
			raw_tile = bitswap<16>((hibyte-0x08)*0x100+lobyte,15,14,13,12,11,7,6,5,10,9,8,4,3,2,1,0) * 0x20 + 0x2000 * 0x20;
			for( uint32_t line = 0; line < 0x20; line++ )
			{
				kanji[(hibyte*0x100+lobyte)*0x20+line] = bitswap<8>(raw_kanji[raw_tile+line],0,1,2,3,4,5,6,7);
			}
		}
	}
	/* 16x16 0x1020-0x4f7f */
	for( uint32_t hibyte = 0x10; hibyte <= 0x4f; hibyte++ )
	{
		for( uint32_t lobyte = 0x20; lobyte <= 0x7f; lobyte++ )
		{
			raw_tile = bitswap<16>((hibyte-0x10)*0x100+lobyte,15,14,7,13,6,5,12,11,10,9,8,4,3,2,1,0) * 0x20;
			for( uint32_t line = 0; line < 0x20; line++ )
			{
				kanji[(hibyte*0x100+lobyte)*0x20+line] = raw_kanji[raw_tile+line];
			}
		}
	}
	/* 16x16 0x5020-0x537f */
	for( uint32_t hibyte = 0x50; hibyte <= 0x53; hibyte++ )
	{
		for( uint32_t lobyte = 0x20; lobyte <= 0x7f; lobyte++ )
		{
			raw_tile = bitswap<16>((hibyte-0x50)*0x100+lobyte,15,14,13,12,11,7,6,5,10,9,8,4,3,2,1,0) * 0x20 + 0x1000 * 0x20;
			for( uint32_t line = 0; line < 0x20; line++ )
			{
				kanji[(hibyte*0x100+lobyte)*0x20+line] = raw_kanji[raw_tile+line];
			}
		}
	}
}

// We keep track of anything undumped of note that belongs to the "PC-98" family tree here, and try to give a
// sub-tree code in state machine if not obvious from the naming suffix.
// This is also repeated in SW list reports, i.e. you have to use an "On RS class xxx" format to indicate a bug report
// specifically happening for PC9801RS. This will be hopefully put into stone with driver splits at some point in future.

// "vanilla" class (i86, E/F/M)
COMP( 1982, pc9801,     0,        0, pc9801,    pc9801,   pc9801_state, empty_init,   "NEC",   "PC-9801",   MACHINE_NOT_WORKING ) // genuine dump
//COMP(1983, pc9801e same with 8MHz CPU bump
COMP( 1983, pc9801f,    pc9801,   0, pc9801f,   pc9801,   pc9801_state, empty_init,   "NEC",   "PC-9801F",  MACHINE_NOT_WORKING ) // genuine dump
COMP( 1984, pc9801m2,   pc9801,   0, pc9801m,   pc9801,   pc9801_state, empty_init,   "NEC",   "PC-9801M2", MACHINE_NOT_WORKING )
//COMP(1985, pc9801m3 same with slotted SASI + 20MB

// VM class (V30)
//COMP(1985, pc9801u2
//COMP(1985, pc9801vf2
COMP( 1985, pc9801vm,   0,        0, pc9801vm,  pc9801rs, pc9801vm_state, init_pc9801vm_kanji, "NEC",   "PC-9801VM",                     MACHINE_NOT_WORKING ) // genuine dump
COMP( 1986, pc9801uv2,  pc9801vm, 0, pc9801uv,  pc9801rs, pc9801vm_state, init_pc9801vm_kanji, "NEC",   "PC-9801UV2",                     MACHINE_NOT_WORKING ) // genuine dump

// VX class (i286 + V30, first model using an EGC)
// original VX0/VX2/VX4 released in Nov 1986, minor updates with OS pre-installed etc. in 1987
// (PC-9801VX4/WN PC-9801VX41/WN)
COMP( 1986, pc9801vx,   0,        0, pc9801vx,  pc9801rs, pc9801vm_state, init_pc9801_kanji,   "NEC",   "PC-9801VX",                     MACHINE_NOT_WORKING )

// CV class (V30, compact version with monitor built-in like a Macintosh)
//COMP(1988, pc9801cv

// RX class (i286 + V30)
COMP( 1987, pc9801ux,   0,        0, pc9801ux,  pc9801rs, pc9801vm_state, init_pc9801_kanji,   "NEC",   "PC-9801UX",                     MACHINE_NOT_WORKING )
COMP( 1988, pc9801rx,   0,        0, pc9801rs,  pc9801rs, pc9801vm_state, init_pc9801_kanji,   "NEC",   "PC-9801RX",                     MACHINE_NOT_WORKING )

// RA class (i386dx + V30)
//COMP(1988, pc9801ra

// RS class (i386sx)
COMP( 1989, pc9801rs,   0,        0, pc9801rs,  pc9801rs, pc9801vm_state, init_pc9801_kanji,   "NEC",   "PC-9801RS",                     0 )
//COMP( 1991, pc9801ds
//COMP( 1991, pc9801cs
//COMP( 1992, pc9801fx

// DX class (i286)
COMP( 1990, pc9801dx,   0,        0, pc9801dx,  pc9801rs, pc9801vm_state, init_pc9801_kanji,   "NEC",   "PC-9801DX",                     MACHINE_NOT_WORKING )

// DA class (i386DX + SDIP and EMS)
//COMP( 1991, pc9801da
// ROM mapping at https://bauxite.sakura.ne.jp/wiki/mypad.cgi?p=PC-98x1%2Fmisc%2FPC-9801DA%A4%CEROM

// UF class (V30HL, Tower form factor)
//COMP( 1991, pc9801uf
//COMP( 1991, pc9801ur

// FS class (i386SX + ?)
COMP( 1992, pc9801fs,   0,        0, pc9801fs,  pc9801rs, pc9801us_state, init_pc9801_kanji,   "NEC",   "PC-9801FS",                     MACHINE_NOT_WORKING )

// US class (i386SX + SDIP, optional high-reso according to BIOS? Derivatives of UX)
COMP( 1992, pc9801us,   0,        0, pc9801us,  pc9801rs, pc9801us_state, init_pc9801_kanji,   "NEC",   "PC-9801US",                     MACHINE_NOT_WORKING )

// FA class (i486sx)
//COMP( 1992, pc9801fa

// BX class (i486sx2, official nickname "98 FELLOW", the lower end of PC-9821 line at this point)
//COMP( 1993, pc9801bx
COMP( 1993, pc9801bx2,  0,        0, pc9801bx2, pc9801rs, pc9801bx_state, init_pc9801_kanji,   "NEC",   "PC-9801BX2/U2 (98 FELLOW)",                 MACHINE_NOT_WORKING )
//COMP( 1993, pc9801bx3
//COMP( 1995, pc9801bx4

// BS class (i486sx, "98 FELLOW")
//COMP( 1993, pc9801bs2

// BA class (i486dx2, "98 FELLOW")
//COMP( 1993, pc9801ba
//COMP( 1993, pc9801ba2
//COMP( 1995, pc9801ba3

// http://www.amy.hi-ho.ne.jp/nakajima-jr/kikaku/special3/office.htm
// N5200 (started as a vanilla PC-98 business line derivative,
//        eventually diverged into its own thing and incorporated various Hyper 98 features.
//        Runs proprietary PTOS)
// APC III (US version of either vanilla PC9801 or N5200, aimed at business market. Runs MS-DOS 2.11/3.xx or PC-UX)
// ...

// 文豪 a.k.a. "Bungo" (A full family of desktop & notebook word processors)
// NWP-20N (OG 1981 model)
// HWP "Hyper 7" (business model)
// DP "DP NOTE" ("Document Processor", based off various flavours of N5820 & 9821NOTE combo)
// DP-70F (based off N5820-70FA & 9821ap2)
// DP-70S (Document Filing Server)
// Mini 5 -> cfr. bungo.cpp
// Mini 7
// JX series
// ...

// XA/XL class (1120 x 750 true color, nicknamed "High-reso")
// ...

// PC-H98 (Hyper 98, '90-'93 high end line with High-reso, proprietary NESA bus, E²GC)
// PC-H98S cfr. pc_h98.cpp
// PC-H98T (LCD Hyper 98)
// SV-H98 "98SERVER" (i486, later Hyper 98 revision)
// SV-98 (Pentium based, second gen of 98SERVER)
// OP-98 ("Office Processor", released around '91. Reports claims to be H98-like, with extra connectivity with NEC 7200 workstation)
// ...

// PC-98GS (Multimedia PC, exclusive video mode "Extended Screen Graphics", -73 sound board (a superset of later -86), superimposition)
// ...

// Epson class knockoffs -> cfr. pc9801_epson.cpp

// PC-9821 -> cfr. pc9821.cpp

// PC-98DO (PC88+PC98, V33 + μPD70008AC)
// ...

// PC-98LT / PC-98HA -> cfr. pc98ha.cpp
// PC-9801N "98NOTE" (V30 based, EMS + 3.5" floppy, 8.9" FL blue LCD).
// PC-9801N* (98NOTE upgrades)
// PC-9801T (i386SX, extremely expensive TFT or LCD laptop with C-Bus slots, de-facto a "portable" desktop machine)
// PC-9801LX (i286, belongs to pc98ha.cpp?)
// PC-9801LS (i386SX, Plasma laptop)
// RC-9801 (portable (color?) LCD, i386SX, wireless 9600bps modem)
// PC-9801P (LCD with light pen)
// DB-P1 (1993 b&w LCD touch tablet / eBook)

// DtermPC (V30/V40/V50 LCD terminal telephone aimed at digital PBX exchanges)

// FC-9801 (FC stands for "Factory Computer". Aimed at industrial automation, has similarly named models from 9801 to 9821 and H98.
//          Uses a FC-9801-06 RAS "Reliability, Availability and Serviceability" specific expansion)
// FC98-NX (Evolution of FC-9821Ka, first model FC20C released in 1998, branch is still running to date.
//          Most likely just DOS/V compatible and not going to fit here except for RAS capabilities)

// TWINPOS ("Point Of Sale" from NEC, originally based off PC-98 arch, eventually switched to DOS/V too?)

// Metrologie BFM 186 (speculated, French PAL CAD machine with dual text and gfx 7220 + other NEC parts & Basic 86, fabricated by Ye DATA Japan)
