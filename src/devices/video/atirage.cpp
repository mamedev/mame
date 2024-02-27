// license:BSD-3-Clause
// copyright-holders: R. Belmont
/*
    ATI Rage PCI/AGP SVGA

    This implementation targets the mach64 VT and 3D Rage chips.  Rage 128 has similar registers
    but they're mapped differently.

    mach64 VT = mach64 with video decoding.  Uses a Rage-compatible register layout, as opposed to earlier mach64.
    mach64 GT = Rage I (mach64 acceleration and VGA with 3D polygons and MPEG-1 decode)
    mach64 GT-B = Rage II (Rage I with faster 2D & 3D and MPEG-2 decode)
    Rage II+ = Rage II with full DVD acceleration
    Rage IIc = Rage II+ with optional AGP support
    Rage Pro = new triangle setup engine, improved perspective correction, fog + specular lighting,
               and improved video decode
    Rage Pro Turbo = Rage Pro with AGP 2X support and improved performance drivers for Win9X
    Rage LT = lower-power Rage II with DVD support
    Rage Mobility C, EC, L, and M2 = lower-power Rage Pro with DVD motion compensation
    Rage Mobility P, M, and M1 = lower-power Rage Pro with DVD motion compensation and IDCT accleration
    Rage XL = cost-reduced Rage Pro with improved 3D image quality, used in many servers until 2006

    Most PCI IDs are a 2-letter ATI product code in ASCII.  For instance, Rage I & II aka mach64 GT are 0x4754 'GT'.

    Reference: rrg-g02700_mach64_register_reference_guide_jul96.pdf, aka "mach64 Register Reference Guide: ATI VT-264 and 3D RAGE"
    http://hackipedia.org/browse.cgi/Computer/Platform/PC%2C%20IBM%20compatible/Video/VGA/SVGA/ATI%2C%20Array%20Technology%20Inc
*/

#include "emu.h"
#include "screen.h"
#include "atirage.h"

#define LOG_REGISTERS   (1U << 1)
#define LOG_CRTC        (1U << 2)
#define LOG_DAC         (1U << 3)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ATI_RAGEII, atirageii_device, "rageii", "ATI Rage II PCI")
DEFINE_DEVICE_TYPE(ATI_RAGEIIC, atirageiic_device, "rageiic", "ATI Rage IIC PCI")
DEFINE_DEVICE_TYPE(ATI_RAGEIIDVD, atirageiidvd_device, "rageiidvd", "ATI Rage II+ DVD PCI")
DEFINE_DEVICE_TYPE(ATI_RAGEPRO, atiragepro_device, "ragepro", "ATI Rage Pro PCI")

// register offsets
static constexpr u32 CRTC_H_TOTAL_DISP  = 0x00;
static constexpr u32 CRTC_V_TOTAL_DISP  = 0x08;
static constexpr u32 CRTC_OFF_PITCH     = 0x14;
static constexpr u32 CRTC_GEN_CNTL      = 0x1c;
static constexpr u32 GP_IO              = 0x78;
static constexpr u32 CLOCK_CNTL         = 0x90;
static constexpr u32 CRTC_DAC_BASE      = 0xc0;
static constexpr u32 CONFIG_CHIP_ID     = 0xe0;

// PLL register offsets
static constexpr u32 PLL_MACRO_CNTL     = 1;
static constexpr u32 PLL_REF_DIV        = 2;
static constexpr u32 PLL_GEN_CNTL       = 3;
static constexpr u32 MCLK_FB_DIV        = 4;
static constexpr u32 PLL_VCLK_CNTL      = 5;
static constexpr u32 VCLK_POST_DIV      = 6;
static constexpr u32 VCLK0_FB_DIV       = 7;
static constexpr u32 VCLK1_FB_DIV       = 8;
static constexpr u32 VCLK2_FB_DIV       = 9;
static constexpr u32 VCLK3_FB_DIV       = 10;
static constexpr u32 PLL_XCLK_CNTL      = 11;
static constexpr u32 PLL_FCP_CNTL       = 12;

// mach64 & 3D Rage post-dividers for PLL
static const int pll_post_dividers[8] =
{
	1, 2, 4, 8, 3, 5, 6, 12
};

void atirage_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(FUNC(atirage_device::screen_update));

	ATIMACH64(config, m_mach64, 0);
	m_mach64->set_screen("screen");
	m_mach64->set_vram_size(0x600000);
}

atirage_device::atirage_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, type, tag, owner, clock),
	m_mach64(*this, "vga"),
	m_screen(*this, "screen"),
	read_gpio(*this, 0),
	write_gpio(*this)
{
	m_hres = m_vres = m_htotal = m_vtotal = m_format = 0;
	m_dac_windex = m_dac_rindex = m_dac_state = 0;
	m_dac_mask = 0xff;

}

atirageii_device::atirageii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atirage_device(mconfig, ATI_RAGEII, tag, owner, clock)
{
}

atirageiic_device::atirageiic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atirage_device(mconfig, ATI_RAGEIIC, tag, owner, clock)
{
}

atirageiidvd_device::atirageiidvd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atirage_device(mconfig, ATI_RAGEIIDVD, tag, owner, clock)
	, m_vga_rom(*this, "vga_rom")
{
}

atiragepro_device::atiragepro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atirage_device(mconfig, ATI_RAGEPRO, tag, owner, clock)
{
}

void atirage_device::io_map(address_map& map)
{
	map(0x00000000, 0x000003ff).rw(FUNC(atirage_device::regs_0_read), FUNC(atirage_device::regs_0_write));
}

void atirage_device::mem_map(address_map& map)
{
	map(0x00000000, 0x005fffff).rw(m_mach64, FUNC(mach64_device::framebuffer_r), FUNC(mach64_device::framebuffer_w));
	map(0x007ff800, 0x007ffbff).rw(FUNC(atirage_device::regs_1_read), FUNC(atirage_device::regs_1_write));
	map(0x007ffc00, 0x007fffff).rw(FUNC(atirage_device::regs_0_read), FUNC(atirage_device::regs_0_write));
	map(0x00800000, 0x00dfffff).rw(m_mach64, FUNC(mach64_device::framebuffer_be_r), FUNC(mach64_device::framebuffer_be_w));
}

void atirage_device::reg_map(address_map& map)
{
}

void atirage_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x0040, 0x0043).rw(FUNC(atirage_device::user_cfg_r), FUNC(atirage_device::user_cfg_w));
}

void atirage_device::device_start()
{
	pci_device::device_start();

	add_map(0x1000000, M_MEM | M_PREF, FUNC(atirage_device::mem_map));   // 16 MB memory map
	add_map(0x100, M_IO, FUNC(atirage_device::io_map));     // 256 byte I/O map
	add_map(0x01000, M_MEM, FUNC(atirage_device::reg_map)); // 4K register map

	command = 3;
	intr_pin = 1;
	intr_line = 0;

	// clear the registers
	std::fill(std::begin(m_regs0), std::end(m_regs0), 0);
	std::fill(std::begin(m_regs1), std::end(m_regs1), 0);
	std::fill(std::begin(m_pll_regs), std::end(m_pll_regs), 0);
	std::fill(std::begin(m_dac_colors), std::end(m_dac_colors), 0);

	// set PLL defaults from the manual
	m_pll_regs[PLL_MACRO_CNTL] = 0xd4;
	m_pll_regs[PLL_REF_DIV] = 0x36;
	m_pll_regs[PLL_GEN_CNTL] = 0x4f;
	m_pll_regs[MCLK_FB_DIV] = 0x97;
	m_pll_regs[PLL_VCLK_CNTL] = 0x04;
	m_pll_regs[VCLK_POST_DIV] = 0x6a;
	m_pll_regs[VCLK0_FB_DIV] = 0xbe;
	m_pll_regs[VCLK1_FB_DIV] = 0xd6;
	m_pll_regs[VCLK2_FB_DIV] = 0xee;
	m_pll_regs[VCLK3_FB_DIV] = 0x88;
	m_pll_regs[PLL_XCLK_CNTL] = 0x00;
	m_pll_regs[PLL_FCP_CNTL] = 0x41;

	m_user_cfg = 8;
	save_item(NAME(m_user_cfg));
	save_item(NAME(m_regs0));
	save_item(NAME(m_regs1));
	save_item(NAME(m_pll_regs));
	save_item(NAME(m_hres));
	save_item(NAME(m_vres));
	save_item(NAME(m_htotal));
	save_item(NAME(m_vtotal));
	save_item(NAME(m_format));
	save_item(NAME(m_pixel_clock));
	save_item(NAME(m_dac_windex));
	save_item(NAME(m_dac_rindex));
	save_item(NAME(m_dac_state));
	save_item(NAME(m_dac_mask));
	save_item(NAME(m_dac_colors));
}

void atirageii_device::device_start()
{
	// mach64 GT-B / 3D Rage II (ATI documentation uses both names)
	set_ids(0x10024754, 0x00, 0x030000, 0x10026987);
	atirage_device::device_start();
	revision = 0x9a;
	m_regs0[CONFIG_CHIP_ID] = 0x54;
	m_regs0[CONFIG_CHIP_ID+1] = 0x47;
	m_regs0[CONFIG_CHIP_ID+3] = 0x9a;
}

void atirageiic_device::device_start()
{
	// Rage IIc PCI
	set_ids(0x10024756, 0x00, 0x030000, 0x10026987);
	atirage_device::device_start();
	revision = 0x3a;
	m_regs0[CONFIG_CHIP_ID] = 0x56;
	m_regs0[CONFIG_CHIP_ID+1] = 0x47;
	m_regs0[CONFIG_CHIP_ID+3] = 0x3a;
}

// TODO: this core is currently hardwired to legacy x86 interface, as a testbed for p5txla
void atirageiidvd_device::device_start()
{
	// Mach64 GT-B [3D Rage II+ DVD]
	// TODO: verify subvendor ID & revision
	set_ids(0x10024755, 0x00, 0x030000, 0x10026987);
	atirage_device::device_start();
	revision = 0x3a;
	m_regs0[CONFIG_CHIP_ID] = 0x55;
	m_regs0[CONFIG_CHIP_ID+1] = 0x47;
	m_regs0[CONFIG_CHIP_ID+3] = 0x3a;
//  m_regs0[CRTC_GEN_CNTL+3] = 0;

	// TODO: opt-in Mach64 legacy x86 memory & i/o VGA bridge control
	command = 0;

	add_rom((u8 *)m_vga_rom->base(), 0x8000);
	expansion_rom_base = 0xc0000;
}

void atirageiidvd_device::device_reset()
{
	atirage_device::device_reset();
	// TODO: verify actual x86 BAR defaults
	// p5txla starts up with (unmapped) writes at I/O $6100, eventually relocating BAR1 to $6300.
	// 2mbsgr VGA BIOS (at least, PC=c1c2d) does an inefficient CONFIG_CHIP_ID scan of the I/O
	// space thru all of $xxe0, instead of just using BLK_IO_BASE readback alias at VGA $3c3.
	// On top, we need to remap BARs 0 & 2 on x86 because otherwise it will clash with real memory
	// area, perhaps they are supposed to be disabled on startup?
	set_map_address(0, 0xf0000000);
	set_map_address(1, 0x6100);
	set_map_address(2, 0xe0000000);
	remap_cb();
}

ROM_START( atirageiidvd )
	ROM_REGION32_LE( 0x10000, "vga_rom", ROMREGION_ERASEFF )
	// Header, P/N then date
	ROM_SYSTEM_BIOS( 0, "2mbsgr", "ATI Mach64 2mb 113-40109-100 1997/10/03" )
	ROMX_LOAD( "2mbsgr.vbi", 0x0000, 0x8000, CRC(d800adfd) SHA1(17492b51b5ec158db618f2851ce8beca91d12aa8), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "4mbsgr", "ATI Mach64 4mb 113-37914-103 1997/04/15" )
	ROMX_LOAD( "4mbsgr.vbi", 0x0000, 0xc000, CRC(e974821f) SHA1(185557cec469f54e15cbe30241bd1af56ed303d2), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "4mbedo", "ATI Mach64 GTB 4mb EDO 113-38801-101 1997/02/12" )
	ROMX_LOAD( "4mbedo.vbi", 0x0000, 0x8800, CRC(0c344b72) SHA1(a068ef73d56b5fc200076283d32676b818404f1b), ROM_BIOS(2) )
ROM_END

const tiny_rom_entry *atirageiidvd_device::device_rom_region() const
{
	return ROM_NAME(atirageiidvd);
}

uint8_t atirageiidvd_device::vram_r(offs_t offset)
{
	return downcast<mach64_device *>(m_mach64.target())->mem_r(offset);
}

void atirageiidvd_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<mach64_device *>(m_mach64.target())->mem_w(offset, data);
}

void atirageiidvd_device::legacy_io_map(address_map &map)
{
	map(0x0000, 0x02f).m(m_mach64, FUNC(mach64_device::io_map));
}

void atirageiidvd_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(atirageiidvd_device::vram_r)), write8sm_delegate(*this, FUNC(atirageiidvd_device::vram_w)));
	}

	if (BIT(command, 0))
	{
		io_space->install_device(0x03b0, 0x03df, *this, &atirageiidvd_device::legacy_io_map);
		io_space->install_readwrite_handler(0x01ce, 0x01cf, read8sm_delegate(*m_mach64, FUNC(mach64_device::ati_port_ext_r)), write8sm_delegate(*m_mach64, FUNC(mach64_device::ati_port_ext_w)));
	}
}

void atiragepro_device::device_start()
{
	// Rage Pro PCI
	set_ids(0x10024750, 0x00, 0x030000, 0x10026987);
	atirage_device::device_start();
	revision = 0x5c;
	m_regs0[CONFIG_CHIP_ID] = 0x50;
	m_regs0[CONFIG_CHIP_ID+1] = 0x47;
	m_regs0[CONFIG_CHIP_ID+3] = 0x5c;
}

void atirage_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
}

u8 atirage_device::regs_0_read(offs_t offset)
{
	switch (offset)
	{
		case CRTC_DAC_BASE: // DAC write index
			return m_dac_windex;

		case CRTC_DAC_BASE + 1:
			{
				u8 result = 0;
				switch (m_dac_state)
				{
					case 0: // red
						result = ((m_dac_colors[m_dac_rindex] >> 16) & 0xff);
						break;

					case 1: // blue
						result = ((m_dac_colors[m_dac_rindex] >> 8) & 0xff);
						break;

					case 2: // green
						result = (m_dac_colors[m_dac_rindex] & 0xff);
						break;
				}

				m_dac_state++;
				if (m_dac_state >= 3)
				{
					m_dac_state = 0;
					m_dac_rindex++;
				}
				return result;
			}
			break;

		case CRTC_DAC_BASE + 2:
			return m_dac_mask;

		case CRTC_DAC_BASE + 3:
			return m_dac_rindex;

		case CLOCK_CNTL + 2:
			return m_pll_regs[(m_regs0[CLOCK_CNTL+1] >> 2) & 0xf] << 16;
	}

	return m_regs0[offset];
}

void atirage_device::regs_0_write(offs_t offset, u8 data)
{
	// the FCode drivers try to write to the chip ID, no idea why
	if ((offset >= CONFIG_CHIP_ID) && (offset <= (CONFIG_CHIP_ID + 3)))
	{
		return;
	}

	LOGMASKED(LOG_REGISTERS, "regs_0_write: %02x to %x\n", data, offset);

	m_regs0[offset] = data;
	switch (offset)
	{
		case CRTC_DAC_BASE: // DAC write index
				m_dac_state = 0;
				m_dac_windex = data;
				break;

		case CRTC_DAC_BASE + 1:
			switch (m_dac_state)
			{
				case 0: // red
					m_dac_colors[m_dac_windex] &= 0x00ffff;
					m_dac_colors[m_dac_windex] |= ((data & 0xff) << 16);
					break;

				case 1: // green
					m_dac_colors[m_dac_windex] &= 0xff00ff;
					m_dac_colors[m_dac_windex] |= ((data & 0xff) << 8);
					break;

				case 2: // blue
					m_dac_colors[m_dac_windex] &= 0xffff00;
					m_dac_colors[m_dac_windex] |= (data & 0xff);
					break;
			}

			m_dac_state++;
			if (m_dac_state == 3)
			{
				m_dac_state = 0;
				m_mach64->set_color(m_dac_windex, m_dac_colors[m_dac_windex]);
				m_dac_windex++;
			}
			break;

		case CRTC_DAC_BASE + 2:
			m_dac_mask = data;
			break;

		case CRTC_DAC_BASE + 3:
			m_dac_state = 0;
			m_dac_rindex = data;
			break;

		case CRTC_OFF_PITCH:
		case CRTC_GEN_CNTL:
			update_mode();
			break;

		case CLOCK_CNTL + 2:
			if (BIT(m_regs0[CLOCK_CNTL+1], 1))
			{
				u8 regnum = (m_regs0[CLOCK_CNTL+1] >> 2) & 0xf;
				m_pll_regs[regnum] = data & 0xff;
			}
			break;

		case GP_IO:
		case GP_IO + 1:
		case GP_IO + 2:
		case GP_IO + 3:
			{
				u16 old_data = *(u16 *)&m_regs0[GP_IO];
				const u16 ddr = *(u16 *)&m_regs0[GP_IO+2];

				old_data &= ddr;                // 0 bits are input

				// send the data to an external handler
				// AND the pullups by the inverse of DDR, so bits set to input get the pullup
				write_gpio(old_data | (m_gpio_pullups & (ddr ^ 0xffff)));

				// get the updated data from the port
				u16 new_data = read_gpio();
				new_data &= (ddr ^ 0xffff);     // AND against inverted DDR mask so 0 bits are output
				new_data |= old_data;
				m_regs0[GP_IO] = (new_data & 0xff);
				m_regs0[GP_IO + 1] = (new_data >> 8) & 0xff;
			}
			break;
	}
}

u8 atirage_device::regs_1_read(offs_t offset)
{
	LOGMASKED(LOG_REGISTERS, "regs 1 read @ %x\n", offset);
	return m_regs1[offset];
}

void atirage_device::regs_1_write(offs_t offset, u8 data)
{
	m_regs1[offset] = data;
}

u32 atirage_device::user_cfg_r()
{
	return m_user_cfg;
}

void atirage_device::user_cfg_w(u32 data)
{
	m_user_cfg = data;
}

void atirage_device::update_mode()
{
	// first prereq: must be in native mode and the CRTC must be enabled
	if (!(m_regs0[CRTC_GEN_CNTL+3] & 3))
	{
		LOGMASKED(LOG_CRTC, "VGA mode must be OFF and CRTC must be ON\n");
		return;
	}

	m_htotal = (m_regs0[CRTC_H_TOTAL_DISP] | (m_regs0[CRTC_H_TOTAL_DISP+1] & 1) << 8) + 1;
	m_htotal <<= 3; // in units of 8 pixels
	m_hres = m_regs0[CRTC_H_TOTAL_DISP+2] + 1;
	m_hres <<= 3;
	m_vres = (m_regs0[CRTC_V_TOTAL_DISP+2] | (m_regs0[CRTC_V_TOTAL_DISP+3] & 7) << 8) + 1;
	m_vtotal = (m_regs0[CRTC_V_TOTAL_DISP] | (m_regs0[CRTC_V_TOTAL_DISP+1] & 7) << 8) + 1;
	m_format = m_regs0[CRTC_GEN_CNTL+1] & 7;
	LOGMASKED(LOG_CRTC, "Setting mode (%d x %d), total (%d x %d) format %d\n", m_hres, m_vres, m_htotal, m_vtotal, m_format);

	double vpll_frequency;
	int clk_source = m_regs0[CLOCK_CNTL] & 3;

	switch (m_pll_regs[PLL_VCLK_CNTL] & 3)
	{
		case 0: // CPUCLK (the PCI bus clock, not to exceed 33 MHz)
			vpll_frequency = (33000000.0 * m_pll_regs[VCLK0_FB_DIV + clk_source]) / m_pll_regs[PLL_REF_DIV];
			break;

		case 3: // PLLVCLK
			vpll_frequency = ((clock() * 2.0) * m_pll_regs[VCLK0_FB_DIV + clk_source]) / m_pll_regs[PLL_REF_DIV];
			break;

		default:
			LOGMASKED(LOG_CRTC, "VCLK source (%d) is not VPLL, can't calculate dot clock\n", m_pll_regs[PLL_VCLK_CNTL] & 3);
			return;
	}
	LOGMASKED(LOG_CRTC, "VPLL freq %f\n", vpll_frequency);

	int vpll_post_divider = (m_pll_regs[VCLK_POST_DIV] >> (clk_source << 1)) & 3;
	// Rage Pro adds one more bit to the divider from bits 4/5/6/7 of XCLK_CNTL depending on the clock source.
	// This should always be zero on mach64/Rage/Rage II.
	vpll_post_divider |= ((m_pll_regs[PLL_XCLK_CNTL] >> (clk_source + 2)) & 4);

	m_pixel_clock = u32(vpll_frequency / pll_post_dividers[vpll_post_divider]);
	LOGMASKED(LOG_CRTC, "Pixel clock = %d, refresh = %f\n", m_pixel_clock, (double)m_pixel_clock / (double)m_htotal / (double)m_vtotal);

	rectangle visarea(0, m_hres - 1, 0, m_vres - 1);
	m_screen->configure(m_htotal, m_vtotal, visarea, attotime::from_ticks(m_htotal * m_vtotal, m_pixel_clock).as_attoseconds());
}

u32 atirage_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// are we in VGA mode rather than native?  if so, let the legacy VGA stuff draw.
	if (!(m_regs0[CRTC_GEN_CNTL+3] & 1))
	{
		return m_mach64->screen_update(screen, bitmap, cliprect);
	}

	// is the CRTC not enabled or the display disable bit set?
	if ((!(m_regs0[CRTC_GEN_CNTL+3] & 2)) || (m_regs0[CRTC_GEN_CNTL] & 0x40))
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	const int offset = ((m_regs0[CRTC_OFF_PITCH+2] & 0xf) << 16) | (m_regs0[CRTC_OFF_PITCH+1] << 8) | (m_regs0[CRTC_OFF_PITCH]);
	u8 *vram = m_mach64->get_framebuffer_addr() + (offset * 8);
	int stride = (m_regs0[CRTC_OFF_PITCH+2] >> 6) | (m_regs0[CRTC_OFF_PITCH+3] << 2);
	stride *= 4;

	switch (m_format)
	{
		case 2: // 8 bpp (also can be a weird 2/2/3 direct color mode)
			for (u32 y = 0; y < m_vres; y++)
			{
				const u8 *src = &vram[stride*y];
				u32 *dst = &bitmap.pix(y, 0);
				for (u32 x = 0; x < m_hres; x++)
				{
					*dst++ = m_dac_colors[src[x]];
				}
				vram += stride;
			}
			break;

		default:
			LOG("Unknown pixel format %d\n", m_format);
			break;
	}

	return 0;
}

/*
02 to CLOCK_CTNL + 1
PLL: cd to 0
06 to CLOCK_CTNL + 1
PLL: d5 to 1
2a to CLOCK_CTNL + 1
PLL: 17 to 10
1a to CLOCK_CTNL + 1
PLL: c0 to 6
0a to CLOCK_CTNL + 1
PLL: 21 to 2
16 to CLOCK_CTNL + 1
PLL: 03 to 5
12 to CLOCK_CTNL + 1
PLL: 91 to 4
0e to CLOCK_CTNL + 1
PLL: 14 to 3
2e to CLOCK_CTNL + 1
PLL: 01 to 11
32 to CLOCK_CTNL + 1
PLL: a6 to 12
32 to CLOCK_CTNL + 1
PLL: e6 to 12
32 to CLOCK_CTNL + 1
PLL: a6 to 12
2a to CLOCK_CTNL + 1
PLL: 98 to 10
2e to CLOCK_CTNL + 1
PLL: 01 to 11
18 to CLOCK_CTNL + 1
Read PLL 6
1a to CLOCK_CTNL + 1
PLL: 80 to 6
0a to CLOCK_CTNL + 1
PLL: 24 to 2
12 to CLOCK_CTNL + 1
PLL: 9e to 4
32 to CLOCK_CTNL + 1
PLL: a6 to 12
32 to CLOCK_CTNL + 1
PLL: e6 to 12
32 to CLOCK_CTNL + 1
PLL: a6 to 12

*/
