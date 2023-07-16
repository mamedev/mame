// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple DAFB and DAFB II video (343S0128-01 for DAFB, 343S0128-A for DAFB II)
    Emulation by R. Belmont
    Some inspiration from mv_sonora by Olivier Galibert and nubus_48gc by Vas Crabb

    DAFB (officially Direct Access Frame Buffer, internally Dave's Awesome Frame Buffer) was the on-board video
    for the Quadra 700, 900, and 950.  Standalone DAFB and DAFB-II include what Apple calls "Turbo SCSI", which
    is an interface for up to 2 5394/5396 chips that adds a configurable wait state to each access and can hold
    off /DTACK on pseudo-DMA reads and writes.

    Shipping configurations:
    DAFB    - original standalone chip, Quadra 700 and 900 (returns versions 0 and 1)
    DAFB II - revised standalone chip with 15 bpp support added (uses AC843 CODEC instead of AC842) (returns version 2)
              Used in Quadra 950.
    MEMC    - DAFB II without the Turbo SCSI logic, in the djMEMC and MEMCjr memory controllers. (returns version 3)
              Used in LC 475, LC 575, Quadra 605, Quadra 610, Quadra 650, and Quadra 800.
              This version uses a DP8533 timing generator instead of the DP8531.

    The Turbo SCSI block moved into the IOSB and PrimeTime I/O ASICs for the machines where DAFB moved into the
    memory controller.  It was enhanced slightly to allow longword pseudo-DMA transfers.

    ----------------------------------------------------------------------------------------------------------------

    Apple assigns 3 pins for monitor IDs.  These allow 8 possible codes:

    000 - color 2-Page Display (21")
    001 - monochrome Full Page display (15")
    010 - color 512x384 (12")
    011 - monochrome 2 Page display (21")
    100 - NTSC
    101 - color Full Page display (15")
    110 - High-Resolution Color (13" 640x480) or "type 6" extended codes
    111 - No monitor connected or "type 7" extended codes

    For extended codes, you drive one of the 3 pins at a time and read the 2
    undriven pins.  See http://support.apple.com/kb/TA21618?viewlocale=en_US
    for details.
*/

#include "emu.h"
#include "dafb.h"

#define LOG_SWATCH      (1U << 1)
#define LOG_CLOCKGEN    (1U << 2)
#define LOG_MONSENSE    (1U << 3)
#define LOG_AC842       (1U << 4)
#define LOG_TURBOSCSI   (1U << 5)

#define VERBOSE (0)
#include "logmacro.h"

static constexpr int VRAM_SIZE = 0x200000 / sizeof(u32);

DEFINE_DEVICE_TYPE(DAFB, dafb_device, "macdafb", "Apple DAFB video")

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void dafb_base::map(address_map &map)
{
	map(0x00000000, 0x000000ff).rw(FUNC(dafb_base::dafb_r), FUNC(dafb_base::dafb_w));
	map(0x00000100, 0x000001ff).rw(FUNC(dafb_base::swatch_r), FUNC(dafb_base::swatch_w));
	map(0x00000200, 0x000002ff).rw(FUNC(dafb_base::ac842_r), FUNC(dafb_base::ac842_w));
	map(0x00000300, 0x000003ff).rw(FUNC(dafb_base::clockgen_r), FUNC(dafb_base::clockgen_w));
}

dafb_base::dafb_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_dafb_version(1),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_monitor_config(*this, "monitor"),
	m_irq(*this),
	m_vram_offset(0), m_timing_control(0), m_mode(0), m_depth(0), m_monitor_id(0), m_pal_address(0),
	m_pal_idx(0), m_ac842_pbctrl(0), m_base(0), m_stride(1024), m_test(0), m_swatch_mode(1),
	m_cursor_line(0), m_anim_line(0), m_int_status(0), m_hres(0), m_vres(0), m_htotal(0), m_vtotal(0),
	m_config(0), m_block_control(0), m_swatch_test(0)
{
	std::fill(std::begin(m_horizontal_params), std::end(m_horizontal_params), 0);
	std::fill(std::begin(m_vertical_params), std::end(m_vertical_params), 0);
	m_scsi_read_cycles[0] = m_scsi_read_cycles[1] = 3;
	m_scsi_write_cycles[0] = m_scsi_write_cycles[1] = 3,
	m_scsi_dma_read_cycles[0] = m_scsi_dma_read_cycles[1] = 3;
	m_scsi_dma_write_cycles[0] = m_scsi_dma_write_cycles[1] = 3;
}

dafb_device::dafb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	dafb_base(mconfig, DAFB, tag, owner, clock),
	m_maincpu(*this, finder_base::DUMMY_TAG)
{
	m_drq[0] = m_drq[1] = 0;
	m_ncr[0] = m_ncr[1] = nullptr;
}

void dafb_base::device_start()
{
	m_vram = std::make_unique<u32[]>(VRAM_SIZE);

	m_vbl_timer = timer_alloc(FUNC(dafb_base::vbl_tick), this);
	m_cursor_timer = timer_alloc(FUNC(dafb_base::cursor_tick), this);

	m_vbl_timer->adjust(attotime::never);
	m_cursor_timer->adjust(attotime::never);

	save_item(NAME(m_timing_control));
	save_item(NAME(m_vram_offset));
	save_item(NAME(m_mode));
	save_item(NAME(m_depth));
	save_item(NAME(m_monitor_id));
	save_item(NAME(m_base));
	save_item(NAME(m_stride));
	save_item(NAME(m_swatch_mode));
	save_item(NAME(m_pal_address));
	save_item(NAME(m_pal_idx));
	save_item(NAME(m_ac842_pbctrl));
	save_item(NAME(m_cursor_line));
	save_item(NAME(m_hres));
	save_item(NAME(m_vres));
	save_item(NAME(m_htotal));
	save_item(NAME(m_vtotal));
	save_item(NAME(m_pixel_clock));
	save_item(NAME(m_horizontal_params));
	save_item(NAME(m_vertical_params));
	save_item(NAME(m_dp8531_regs));
	save_item(NAME(m_test));
	save_item(NAME(m_config));
	save_item(NAME(m_block_control));
	save_item(NAME(m_swatch_test));
	save_item(NAME(m_int_status));
	save_pointer(NAME(m_vram), VRAM_SIZE);

	machine().save().register_postload(save_prepost_delegate(FUNC(dafb_base::recalc_mode), this));
}

void dafb_base::device_reset()
{
}

void dafb_base::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// dot clock, htotal, hstart, hend, vtotal, vstart, vend
	m_screen->set_raw(31334400, 896, 0, 640, 525, 0, 480);
	m_screen->set_screen_update(FUNC(dafb_base::screen_update));

	PALETTE(config, m_palette).set_entries(256);
}

static constexpr u8 ext(u8 bc, u8 ac, u8 ab)
{
	return 0x40 | (bc << 4) | (ac << 2) | ab;
}

static INPUT_PORTS_START(monitor_config)
	PORT_START("monitor")
	PORT_CONFNAME(0x7f, 6, "Monitor type")
	PORT_CONFSETTING(0x00, u8"Mac 21\" Color Display (1152\u00d7870)")          // "RGB 2 Page" or "Kong"
	PORT_CONFSETTING(0x01, u8"Mac Portrait Display (B&W 15\" 640\u00d7870)")    // "Full Page" or "Portrait"
	PORT_CONFSETTING(0x02, u8"Mac RGB Display (12\" 512\u00d7384)")             // "Rubik" (modified IIgs AppleColor RGB)
	PORT_CONFSETTING(0x03, u8"Mac Two-Page Display (B&W 21\" 1152\u00d7870)")   // "2 Page"
	PORT_CONFSETTING(0x06, u8"Mac Hi-Res Display (12-14\" 640\u00d7480)")       // "High Res"
	PORT_CONFSETTING(ext(0, 0, 0), "PAL Encoder (640\u00d7480, 768\u00d7576)")
	PORT_CONFSETTING(ext(1, 1, 0), "NTSC Encoder (512\u00d7384, 640\u00d7480)")
	PORT_CONFSETTING(ext(1, 1, 3), "640x480 VGA")
	PORT_CONFSETTING(ext(2, 3, 1), "832x624 16\" RGB")                          // "Goldfish" or "16 inch RGB"
	PORT_CONFSETTING(ext(3, 0, 0), "PAL (640\u00d7480, 768\u00d7576)")
INPUT_PORTS_END

ioport_constructor dafb_base::device_input_ports() const
{
	return INPUT_PORTS_NAME(monitor_config);
}

u32 dafb_base::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]) + m_base;
	const pen_t *pens = m_palette->pens();

	// check display disable
	if (BIT(m_swatch_mode, 0))
	{
		return 0;
	}

	// if convolution is enabled, the stride is fixed at 1024
	const u32 stride = BIT(m_config, 3) ? 1024 : m_stride;

	switch (m_mode)
	{
		case 0: // 1bpp
		{
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/8; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];

					*scanline++ = pens[(pixels>>7)&1];
					*scanline++ = pens[(pixels>>6)&1];
					*scanline++ = pens[(pixels>>5)&1];
					*scanline++ = pens[(pixels>>4)&1];
					*scanline++ = pens[(pixels>>3)&1];
					*scanline++ = pens[(pixels>>2)&1];
					*scanline++ = pens[(pixels>>1)&1];
					*scanline++ = pens[(pixels&1)];
				}
			}
		}
		break;

		case 1: // 2bpp
		{
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/4; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];

					*scanline++ = pens[((pixels>>6)&3)];
					*scanline++ = pens[((pixels>>4)&3)];
					*scanline++ = pens[((pixels>>2)&3)];
					*scanline++ = pens[(pixels&3)];
				}
			}
		}
		break;

		case 2: // 4bpp
		{
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/2; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];

					*scanline++ = pens[(pixels>>4)];
					*scanline++ = pens[(pixels&0xf)];
				}
			}
		}
		break;

		case 3: // 8bpp
		{
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];
					*scanline++ = pens[pixels];
				}
			}
		}
		break;

		case 4: // 24 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				u32 const *base = &m_vram[(y * (stride/4)) + (m_base/4)];
				for (int x = 0; x < m_hres; x++)
				{
					*scanline++ = *base++;
				}
			}
			break;
	}

	return 0;
}

u32 dafb_base::dafb_r(offs_t offset)
{
	switch (offset<<2)
	{
		case 0x1c:  // inverse of monitor sense
			{
				u8 mon = m_monitor_config->read();
				u8 res;
				LOGMASKED(LOG_MONSENSE, "mon = %02x, m_monitor_id = %02x\n", mon, m_monitor_id);
				if (mon & 0x40)
				{
					res = 7;
					if (m_monitor_id == 0x4)
					{
						res &= 4 | (BIT(mon, 5) << 1) | BIT(mon, 4);
					}
					if (m_monitor_id == 0x2)
					{
						res &= (BIT(mon, 3) << 2) | 2 | BIT(mon, 2);
					}
					if (m_monitor_id == 0x1)
					{
						res &= (BIT(mon, 1) << 2) | (BIT(mon, 0) << 1) | 1;
					}
				}
				else
				{
					res = mon;
				}

				LOGMASKED(LOG_MONSENSE, "sense result = %x\n", res);
				return res ^ 7; // return value is the inverse of the sense bits
			}
			break;

		case 0x24: // SCSI 539x #1 status
			return m_scsi_ctrl[0] | (m_drq[0] << 9);

		case 0x28: // SCSI 539x #2 status
			return m_scsi_ctrl[1] | (m_drq[1] << 9);

		case 0x2c: // test / version (0 = original, 1 = NTSC and PAL fix, 2 = discrete DAFB II, 3 = MEMC/MEMCjr integrated DAFB cell)
			return (m_test & 0x1ff) | (2<<9);
	}

	return 0;
}

void dafb_base::dafb_w(offs_t offset, u32 data)
{
	switch (offset << 2)
	{
		case 0: // bits 20-9 of base
			m_base &= 0x1e0;
			m_base |= (data & 0xfff) << 9;
			LOG("baseA: wrote %08x => %08x\n", data, m_base);
			break;

		case 4: // bits 8-5 of base
			m_base &= ~0x1e0;
			m_base |= (data & 0xf) << 5;
			LOG("baseB wrote %08x => %08x\n", data, m_base);
			break;

		case 8:
			m_stride = data<<2;    // stride in DWORDs
			LOG("Stride = %d\n", m_stride);
			break;

		case 0xc:   // timing control
			m_timing_control = data;
			LOG("Timing control = %08x\n", data);
			break;

		case 0x10: // configuration
			LOG("DAFB config = %08x\n", data);
			m_config = data;
			break;


		case 0x14: // block write control
			LOG("Block write control = %08x\n", data);
			m_block_control = data;
			break;

		case 0x1c: // drive monitor sense lines. 0=drive to value in bit 0 of TEST, 1=tri-state
			m_monitor_id = (data & 0x7) ^ 7;
			LOGMASKED(LOG_MONSENSE, "%x to sense drive\n", data & 0xf);
			break;

		/*
		    SCSI bus 1 control:
		    bit 0 = SCSI register read is 6 clocks (if neither bit 0 or 1 are set, 3 clocks?)
		    bit 1 = SCSI register read is 4 clocks
		    bit 2 = SCSI register write is 3 clocks (else what?)
		    bit 3 = SCSI pseudo-DMA read is 3 clocks (else what?)
		    bit 4 = SCSI pseudo-DMA write is 5 clocks
		    bit 5 = SCSI pseudo-DMA write is 3 clocks
		    bit 6 = CS PW Check (?)
		    bit 7 = DRQ Check Read (PDMA reads wait if DRQ isn't set and bus error on timeout)
		    bit 8 = DRQ Check Write
		    bit 9 = DREQ status read
		*/
		case 0x24:
			m_scsi_ctrl[0] = data;
			if (BIT(data, 0))
			{
				m_scsi_read_cycles[0] = 6;
			}
			else if (BIT(data, 1))
			{
				m_scsi_read_cycles[0] = 4;
			}
			else
			{
				m_scsi_read_cycles[0] = 3;
			}

			if (BIT(data, 2))
			{
				m_scsi_write_cycles[0] = 3;
			}
			else
			{
				m_scsi_write_cycles[0] = 4;
			}

			if (BIT(data, 3))
			{
				m_scsi_dma_read_cycles[0] = 3;
			}
			else
			{
				m_scsi_dma_read_cycles[0] = 4;
			}

			if (BIT(data, 4))
			{
				m_scsi_dma_write_cycles[0] = 5;
			}
			else if (BIT(data, 5))
			{
				m_scsi_dma_write_cycles[0] = 3;
			}
			LOGMASKED(LOG_TURBOSCSI, "SCSI bus 1 timings: R %d W %d DMAR %d DMAW %d\n", m_scsi_read_cycles[0], m_scsi_write_cycles[0], m_scsi_dma_read_cycles[0], m_scsi_dma_write_cycles[0]);
			break;

		// SCSI bus 2 control, same definitions as above
		case 0x28:
			m_scsi_ctrl[1] = data;
			if (BIT(data, 0))
			{
				m_scsi_read_cycles[1] = 6;
			}
			else if (BIT(data, 1))
			{
				m_scsi_read_cycles[1] = 4;
			}
			else
			{
				m_scsi_read_cycles[1] = 3;
			}

			if (BIT(data, 2))
			{
				m_scsi_write_cycles[1] = 3;
			}
			else
			{
				m_scsi_write_cycles[1] = 4;
			}

			if (BIT(data, 3))
			{
				m_scsi_dma_read_cycles[1] = 3;
			}
			else
			{
				m_scsi_dma_read_cycles[1] = 4;
			}

			if (BIT(data, 4))
			{
				m_scsi_dma_write_cycles[1] = 5;
			}
			else if (BIT(data, 5))
			{
				m_scsi_dma_write_cycles[1] = 3;
			}
			LOGMASKED(LOG_TURBOSCSI, "SCSI bus 2 timings: R %d W %d DMAR %d DMAW %d\n", m_scsi_read_cycles[1], m_scsi_write_cycles[1], m_scsi_dma_read_cycles[1], m_scsi_dma_write_cycles[1]);
			break;

		// TEST register.  Bit 0 is supposedly the value to drive on the monitor sense pins, but that's not what
		// the code does on the Q700.
		case 0x2c:
			LOG("%08x to TEST\n", data);
			m_test = data;
			break;
	}
}

u32 dafb_base::swatch_r(offs_t offset)
{
	switch (offset << 2)
	{
		case 0x8: // IRQ/VBL status
			return m_int_status;

		case 0xc: // clear cursor scanline int
			if (!machine().side_effects_disabled())
			{
				m_int_status &= ~4;
				recalc_ints();
			}
			break;

		case 0x14: // clear VBL int
			if (!machine().side_effects_disabled())
			{
				m_int_status &= ~1;
				recalc_ints();
			}
			break;

		case 0x20: // unused register, used by the driver to stash data
			return m_swatch_test;
	}
	return 0;
}

void dafb_base::swatch_w(offs_t offset, u32 data)
{
	switch (offset << 2)
	{
		case 0x0:           // Swatch mode
			m_swatch_mode = data;
			break;

		case 0x4:
			if (data & 1)   // VBL enable
			{
				m_vbl_timer->adjust(m_screen->time_until_pos(480, 0), 0);
			}
			else
			{
				m_vbl_timer->adjust(attotime::never);
				m_int_status &= ~1;
				recalc_ints();
			}

			if (data & 2)   // aux scanline interrupt enable
			{
				fatalerror("DAFB: Aux scanline interrupt enable not supported!\n");
			}

			if (data & 4)   // cursor scanline interrupt enable
			{
				m_cursor_timer->adjust(m_screen->time_until_pos(m_cursor_line, 0), 0);
			}
			else
			{
				m_cursor_timer->adjust(attotime::never);
				m_int_status &= ~4;
				recalc_ints();
			}
			break;

		case 0xc: // clear cursor scanline int
			m_int_status &= ~4;
			recalc_ints();
			break;

		case 0x14: // clear VBL int
			m_int_status &= ~1;
			recalc_ints();
			break;

		case 0x18:  // cursor IRQ line
			m_cursor_line = data;
			break;

		case 0x1c: // animation IRQ line
			m_anim_line = data;
			break;

		case 0x20:
			m_swatch_test = data;
			break;

		case 0x24: // HSERR - location of horizontal serration pulse
		case 0x28: // HLFLN - Half-line point where equalizing pulses or serrations fall
		case 0x2c: // HEQ - Horizontal equalizing pulse
		case 0x30: // HSP - Horizontal sync pulse
		case 0x34: // HBWAY - Horizontal breezeway
		case 0x38: // HBRST - Horizontal burst (where the NTSC colorburst would happen if this wasn't RGB)
		case 0x3c: // HBP - Horizontal back porch
		case 0x40: // HAL - Horizontal active line (start of active display area)
		case 0x44: // HFP - Horizontal front porch (end of active display area)
		case 0x48: // HPIX - Horizontal pixels - total # of pixel locations in a line minus 2
			LOGMASKED(LOG_SWATCH, "%d to horiz param offset %02x\n", data, offset);
			m_horizontal_params[offset - (0x24 / 4)] = data;
			break;

		case 0x4c: // VHLINE - Vertical half-lines, the total # of half-lines in a field (odd for interlaced, even for NI)
		case 0x50: // VSYNC - Vertical sync
		case 0x54: // VBPEQ - Vertical Back Porch Equalization
		case 0x58: // VBP - Vertical Back Porch (start of active display area)
		case 0x5c: // VAL - Vertical Active Lines (end of active display area)
		case 0x60: // VFP - Vertical Front Porch
		case 0x64: // VFPEQ - Vertical Front Porch Equalization
			LOGMASKED(LOG_SWATCH, "%d to vertical param offset %02x\n", data, offset);
			m_vertical_params[offset - (0x4c / 4)] = data;
			break;
	}
}

u32 dafb_base::ac842_r(offs_t offset)
{
	switch (offset << 2)
	{
		case 0:
			m_pal_idx = 0;
			return m_pal_address;

		case 0x10:
			{
				pen_t entry = m_palette->pen(m_pal_address);
				if (!machine().side_effects_disabled())
				{
					m_pal_idx++;
				}
				switch (m_pal_idx - 1)
				{
					case 0:
						return (entry >> 16) & 0xff;
					case 1:
						return (entry >> 8) & 0xff;
					case 2:
						return entry & 0xff;
				}
			}
			break;

		case 0x20:
			return m_ac842_pbctrl;
	}
	return 0;
}

void dafb_base::ac842_w(offs_t offset, u32 data)
{
	switch (offset << 2)
	{
	case 0:
		m_pal_address = data & 0xff;
		m_pal_idx = 0;
		break;

	case 0x10:
		if ((m_monitor_config->read() == 1) || (m_monitor_config->read() == 3))
		{
			// monochrome monitors put info only on the blue channel
			if (m_pal_idx == 2)
			{
				m_palette->set_pen_red_level(m_pal_address, data & 0xff);
				m_palette->set_pen_green_level(m_pal_address, data & 0xff);
				m_palette->set_pen_blue_level(m_pal_address, data & 0xff);
			}
		}
		else
		{
			switch (m_pal_idx)
			{
				case 0:
					m_palette->set_pen_red_level(m_pal_address, data & 0xff);
					break;
				case 1:
					m_palette->set_pen_green_level(m_pal_address, data & 0xff);
					break;
				case 2:
					m_palette->set_pen_blue_level(m_pal_address, data & 0xff);
					break;
			}
		}
		m_pal_idx++;
		if (m_pal_idx == 3)
		{
			m_pal_idx = 0;
			m_pal_address++;
		}
		break;

	case 0x20:
		m_ac842_pbctrl = data;
		LOGMASKED(LOG_AC842, "%02x to AC842 pixel bus control, & 0x1c = %02x\n", data, data & 0x1c);
		switch (data & 0x1c)
		{
			case 0x00:
				m_mode = 0; // 1bpp
				break;

			case 0x08:
				m_mode = 1; // 2bpp
				break;

			case 0x10:
				m_mode = 2; // 4bpp
				break;

			case 0x18:
				m_mode = 3; // 8bpp
				break;

			case 0x1c:
				m_mode = 4; // 24bpp
				break;
		}
		recalc_mode();
		break;
	}
}

void dafb_base::recalc_mode()
{
	m_htotal = m_horizontal_params[HPIX];
	m_vtotal = m_vertical_params[VFPEQ] >> 1;

	if ((m_htotal > 0) && (m_vtotal > 0))
	{
		m_hres = m_horizontal_params[HFP] - m_horizontal_params[HAL];
		m_vres = (m_vertical_params[VFP] >> 1) - (m_vertical_params[VAL] >> 1); // these are in half-line units for interlace

		// Quadra 700 programs the wrong base for the 512x384 mode and is off-by-1 on the vertical res.
		// Maybe that monitor wasn't really intended to be supported?
		if (m_hres == 512)
		{
			m_base = 0x1000;
			m_vres = 384;
		}

		const int clockdiv = 1 << ((m_ac842_pbctrl & 0x60) >> 5);
		LOGMASKED(LOG_SWATCH, "RAW hres %d vres %d htotal %d vtotal %d (clockdiv %d conv %d)\n", m_hres, m_vres, m_htotal, m_vtotal, clockdiv, BIT(m_config, 3));

		// If convolution is active, divide the horiz. res and stride by the clock divider.
		// If it's not, multiply the horiz. res by the clockdiv.
		if (BIT(m_config, 3))
		{
			m_hres /= clockdiv;
			m_stride /= clockdiv;

			// All modes with convolution enabled on the Q700 overstate the horizontal resolution by 23 for some reason.
			// The documentation, including the spreadsheet of mode examples, doesn't show that.
			// TODO: possibly working around a bug in early chip revisions?  Check when DAFB-II machines are suported.
			m_hres -= 23;
		}
		else
		{
			m_hres *= clockdiv;
		}

		// if we're interlaced, bump the vertical back to double
		if (BIT(m_config, 2))
		{
			m_vres <<= 1;
			m_vtotal <<= 1;
		}

		LOGMASKED(LOG_SWATCH, "hres %d vres %d htotal %d vtotal %d\n", m_hres, m_vres, m_htotal, m_vtotal);
		if ((m_hres != 0) && (m_vres != 0))
		{
			rectangle visarea(0, m_hres - 1, 0, m_vres - 1);
			m_screen->configure(m_htotal, m_vtotal, visarea, attotime::from_ticks(m_htotal * m_vtotal, m_pixel_clock).as_attoseconds());
		}
	}
}

u32 dafb_base::clockgen_r(offs_t offset)
{
	return 0;
}

void dafb_base::clockgen_w(offs_t offset, u32 data)
{
	m_dp8531_regs[offset>>2] = data & 0xf;
	LOGMASKED(LOG_CLOCKGEN, "%s: Write %x to DP8531 at %d\n", tag(), data, offset);

	if ((offset >> 2) == 15)
	{
		int r = m_dp8531_regs[6] << 8 | m_dp8531_regs[5] << 4 | m_dp8531_regs[4];
		int p = (1 << m_dp8531_regs[9]);

		int n_modulus = m_dp8531_regs[3]<<12 | m_dp8531_regs[2]<<8 | m_dp8531_regs[1]<<4 | m_dp8531_regs[0];
		int a = (n_modulus & 0x1f) ^ 0x1f;  // the inverse of the lowest 5 bits of n_modulus
		int b = (n_modulus & 0xffe0) >> 5;  // the top 11 bits of n_modulus

		a = std::min(a, b);
		b = std::max(b, 2);

		// N = 32(B - A) + 31(1 + A)
		int n = (32 * (b - a)) + (31 * (1 + a));
		int vco = ((20'000'000/r) * n);
		m_pixel_clock = vco / p;

		LOGMASKED(LOG_CLOCKGEN, "VCO %d, PCLK %d\n", vco, m_pixel_clock);
	}
}

u32 dafb_base::vram_r(offs_t offset)
{
	return m_vram[offset];
}

void dafb_base::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

void dafb_base::recalc_ints()
{
	if (m_int_status != 0)
	{
		m_irq(ASSERT_LINE);
	}
	else
	{
		m_irq(CLEAR_LINE);
	}
}

TIMER_CALLBACK_MEMBER(dafb_base::vbl_tick)
{
	m_int_status |= 1;
	recalc_ints();

	m_vbl_timer->adjust(m_screen->time_until_pos(480, 0), 0);
}

TIMER_CALLBACK_MEMBER(dafb_base::cursor_tick)
{
	m_int_status |= 4;
	recalc_ints();

	m_cursor_timer->adjust(m_screen->time_until_pos(m_cursor_line, 0), 0);
}

// ************************************************************************
//  dafb_device overrides/additions
// ************************************************************************
void dafb_device::device_start()
{
	dafb_base::device_start();
	m_maincpu->set_emmu_enable(true);
}

template <int bus>
u8 dafb_device::turboscsi_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu->adjust_icount(-m_scsi_read_cycles[bus]);
	}
	return m_ncr[bus]->read(offset>>4);
}

template u8 dafb_device::turboscsi_r<0>(offs_t offset);
template u8 dafb_device::turboscsi_r<1>(offs_t offset);

template <int bus>
void dafb_device::turboscsi_w(offs_t offset, u8 data)
{
	m_maincpu->adjust_icount(-m_scsi_write_cycles[bus]);
	m_ncr[bus]->write(offset>>4, data);
}

template void dafb_device::turboscsi_w<0>(offs_t offset, u8 data);
template void dafb_device::turboscsi_w<1>(offs_t offset, u8 data);

template <int bus>
u16 dafb_device::turboscsi_dma_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled() && BIT(offset << 1, 18))
	{
		m_maincpu->adjust_icount(-m_scsi_dma_read_cycles[bus]);
	}

	if (BIT(m_scsi_ctrl[bus], 7))
	{
		if (!m_drq[bus])
		{
			// The real DAFB simply holds off /DTACK here, we simulate that
			// by rewinding and repeating the instruction until DRQ is asserted.
			m_maincpu->restart_this_instruction();
			m_maincpu->spin_until_time(attotime::from_usec(50));
			return 0xffff;
		}
	}

	if (mem_mask == 0xffff)
	{
		return m_ncr[bus]->dma16_swap_r();
	}
	else if (ACCESSING_BITS_0_7)
	{
		return m_ncr[bus]->dma_r();
	}
	else
	{
		return m_ncr[bus]->dma_r()<<8;
	}
}

template u16 dafb_device::turboscsi_dma_r<0>(offs_t offset, u16 mem_mask);
template u16 dafb_device::turboscsi_dma_r<1>(offs_t offset, u16 mem_mask);

template <int bus>
void dafb_device::turboscsi_dma_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (!machine().side_effects_disabled() && BIT(offset << 1, 18))
	{
		m_maincpu->adjust_icount(-m_scsi_dma_write_cycles[bus]);
	}

	LOGMASKED(LOG_TURBOSCSI, "dma_w %04x (mask %04x)\n", data & mem_mask, mem_mask);

	if (BIT(m_scsi_ctrl[bus], 8))
	{
		if (!m_drq[bus])
		{
			m_maincpu->restart_this_instruction();
			m_maincpu->spin_until_time(attotime::from_usec(50));
			return;
		}

		if (mem_mask == 0xffff)
		{
			m_ncr[bus]->dma16_swap_w(data);
		}
		else if (ACCESSING_BITS_0_7)
		{
			m_ncr[bus]->dma_w(data & 0xff);
		}
		else
		{
			m_ncr[bus]->dma_w(data >> 8);
		}
	}
	else    // no DRQ safety check, just blindly push to the 53c9x
	{
		if (mem_mask == 0xffff)
		{
			m_ncr[bus]->dma16_swap_w(data);
		}
		else if (ACCESSING_BITS_0_7)
		{
			m_ncr[bus]->dma_w(data & 0xff);
		}
		else
		{
			m_ncr[bus]->dma_w(data >> 8);
		}
	}
}

template void dafb_device::turboscsi_dma_w<0>(offs_t offset, u16 data, u16 mem_mask);
template void dafb_device::turboscsi_dma_w<1>(offs_t offset, u16 data, u16 mem_mask);

template <int bus>
void dafb_device::turboscsi_drq_w(int state)
{
	LOGMASKED(LOG_TURBOSCSI, "Bus %d DRQ %d (was %d)\n", bus + 1, state, m_drq[bus]);
	m_drq[bus] = state;
}

template void dafb_device::turboscsi_drq_w<0>(int state);
template void dafb_device::turboscsi_drq_w<1>(int state);
