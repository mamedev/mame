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
#define LOG_ENGINE      (1U << 4)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ATI_RAGEII, atirageii_device, "rageii", "ATI Rage II PCI")
DEFINE_DEVICE_TYPE(ATI_RAGEIIC, atirageiic_device, "rageiic", "ATI Rage IIC PCI")
DEFINE_DEVICE_TYPE(ATI_RAGEIIDVD, atirageiidvd_device, "rageiidvd", "ATI Rage II+ DVD PCI")
DEFINE_DEVICE_TYPE(ATI_RAGEPRO, atiragepro_device, "ragepro", "ATI Rage Pro PCI")

static constexpr u32 CRTC_H_TOTAL_DISP  = 0x000 >> 2;
static constexpr u32 CRTC_V_TOTAL_DISP  = 0x008 >> 2;
static constexpr u32 CRTC_OFF_PITCH     = 0x014 >> 2;
static constexpr u32 CRTC_INT_CNTL      = 0x018 >> 2;
static constexpr u32 CRTC_GEN_CNTL      = 0x01c >> 2;
static constexpr u32 GP_IO              = 0x078 >> 2;
static constexpr u32 CLOCK_CNTL         = 0x090 >> 2;
static constexpr u32 CUR_CLR0           = 0x060 >> 2;
static constexpr u32 CUR_CLR1           = 0x064 >> 2;
static constexpr u32 CUR_OFFSET         = 0x068 >> 2;
static constexpr u32 CUR_HORZ_VERT_POSN = 0x06c >> 2;
static constexpr u32 CUR_HORZ_VERT_OFF  = 0x070 >> 2;
static constexpr u32 CRTC_DAC_BASE      = 0x0c0 >> 2;
static constexpr u32 GEN_TEST_CNTL      = 0x0d0 >> 2;
static constexpr u32 CONFIG_CHIP_ID     = 0x0e0 >> 2;
static constexpr u32 DST_OFF_PITCH      = 0x100 >> 2;
static constexpr u32 DST_X              = 0x104 >> 2;
static constexpr u32 DST_Y              = 0x108 >> 2;
static constexpr u32 DST_Y_X            = 0x10c >> 2;
static constexpr u32 DST_WIDTH          = 0x110 >> 2;
static constexpr u32 DST_HEIGHT         = 0x114 >> 2;
static constexpr u32 DST_HEIGHT_WIDTH   = 0x118 >> 2;
static constexpr u32 DST_X_WIDTH        = 0x11c >> 2;
static constexpr u32 DST_CNTL           = 0x130 >> 2;
static constexpr u32 DST_Y_X_ALIAS      = 0x134 >> 2;
static constexpr u32 SRC_OFF_PITCH      = 0x180 >> 2;
static constexpr u32 SRC_X              = 0x184 >> 2;
static constexpr u32 SRC_Y              = 0x188 >> 2;
static constexpr u32 SRC_Y_X            = 0x18c >> 2;
static constexpr u32 SRC_WIDTH1         = 0x190 >> 2;
static constexpr u32 SRC_HEIGHT1        = 0x194 >> 2;
static constexpr u32 SRC_HEIGHT1_WIDTH1 = 0x198 >> 2;
static constexpr u32 SRC_X_START        = 0x19c >> 2;
static constexpr u32 SRC_Y_START        = 0x1a0 >> 2;
static constexpr u32 SRC_Y_X_START      = 0x1a4 >> 2;
static constexpr u32 SRC_WIDTH2         = 0x1a8 >> 2;
static constexpr u32 SRC_HEIGHT2        = 0x1ac >> 2;
static constexpr u32 SRC_HEIGHT2_WIDTH2 = 0x1b0 >> 2;
static constexpr u32 SRC_CNTL           = 0x1b4 >> 2;
static constexpr u32 HOST_DATA0         = 0x200 >> 2;
static constexpr u32 HOST_DATA15        = 0x23c >> 2;
static constexpr u32 HOST_CNTL          = 0x240 >> 2;
static constexpr u32 PAT_REG0           = 0x280 >> 2;
static constexpr u32 PAT_REG1           = 0x284 >> 2;
static constexpr u32 PAT_CNTL           = 0x288 >> 2;
static constexpr u32 SC_LEFT            = 0x2a0 >> 2;
static constexpr u32 SC_RIGHT           = 0x2a4 >> 2;
static constexpr u32 SC_LEFT_RIGHT      = 0x2a8 >> 2;
static constexpr u32 SC_TOP             = 0x2ac >> 2;
static constexpr u32 SC_BOTTOM          = 0x2b0 >> 2;
static constexpr u32 SC_TOP_BOTTOM      = 0x2b4 >> 2;
static constexpr u32 DP_BKGD_CLR        = 0x2c0 >> 2;
static constexpr u32 DP_FRGD_CLR        = 0x2c4 >> 2;
static constexpr u32 DP_WRITE_MSK       = 0x2c8 >> 2;
static constexpr u32 DP_PIX_WIDTH       = 0x2d0 >> 2;
static constexpr u32 DP_MIX             = 0x2d4 >> 2;
static constexpr u32 DP_SRC             = 0x2d8 >> 2;
static constexpr u32 CLR_CMP_CNTL       = 0x308 >> 2;
static constexpr u32 FIFO_STAT          = 0x310 >> 2;
static constexpr u32 GUI_TRAJ_CNTL      = 0x330 >> 2;
static constexpr u32 GUI_STAT           = 0x338 >> 2;

// CRTC_INT_CNTL
static constexpr u32 CRTC_VBLANK        = 1U << 0;
static constexpr u32 CRTC_VBLANK_INT_EN = 1U << 1;
static constexpr u32 CRTC_VBLANK_INT    = 1U << 2;
static constexpr u32 CRTC_VLINE_INT_EN  = 1U << 3;
static constexpr u32 CRTC_VLINE_INT     = 1U << 4;

// DP_SRC colour source select, for DP_FRGD_SRC and DP_BKGD_SRC
enum : u32
{
	SRC_BKGD_CLR = 0,
	SRC_FRGD_CLR = 1,
	SRC_HOST = 2,
	SRC_BLIT = 3,
	SRC_PATTERN = 4
};

// DP_SRC monochrome source select
enum : u32
{
	MONO_ALWAYS_1 = 0,
	MONO_PATTERN = 1,
	MONO_HOST = 2,
	MONO_BLIT = 3
};

// The 3D RAGE parameter FIFO holds 48 entries, but the hardware never reports more than 32
// of them free.  Because our draw operations happen immediately we just always return this.
static constexpr u32 GUI_FIFO_ENTRIES   = 32;

// DAC register offsets
static constexpr offs_t DAC_WINDEX         = 0;
static constexpr offs_t DAC_DATA           = 1;
static constexpr offs_t DAC_MASK           = 2;
static constexpr offs_t DAC_RINDEX         = 3;

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
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(FUNC(atirage_device::screen_update));
	screen.screen_vblank().set(FUNC(atirage_device::vblank_w));

	ATIMACH64(config, m_mach64);
	m_mach64->set_screen("screen");
	m_mach64->set_vram_size(0x600000);
}

atirage_device::atirage_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, type, tag, owner, clock),
	m_mach64(*this, "vga"),
	m_screen(*this, "screen"),
	read_gpio(*this, 0),
	write_gpio(*this),
	write_irq(*this)
{
	m_hres = m_vres = m_htotal = m_vtotal = m_format = 0;
	m_dac_windex = m_dac_rindex = m_dac_state = 0;
	m_dac_mask = 0xff;
	m_gpio_pullups = 0;
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
	map(0x00800000, 0x00dfffff).rw(m_mach64, FUNC(mach64_device::framebuffer_r), FUNC(mach64_device::framebuffer_w));
}

void atirage_device::reg_map(address_map& map)
{
	map(0x00000000, 0x000003ff).mirror(0x800).rw(FUNC(atirage_device::regs_1_read), FUNC(atirage_device::regs_1_write));
	map(0x00000400, 0x000007ff).mirror(0x800).rw(FUNC(atirage_device::regs_0_read), FUNC(atirage_device::regs_0_write));
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
	m_irq_active = false;
	m_host_active = false;
	m_host_mono = false;
	m_host_x = m_host_y = m_host_start_x = m_host_col = 0;
	m_host_width = m_host_lines = 0;
	m_host_x_step = m_host_y_step = 1;
	save_item(NAME(m_irq_active));
	save_item(NAME(m_host_active));
	save_item(NAME(m_host_mono));
	save_item(NAME(m_host_x));
	save_item(NAME(m_host_y));
	save_item(NAME(m_host_start_x));
	save_item(NAME(m_host_col));
	save_item(NAME(m_host_width));
	save_item(NAME(m_host_lines));
	save_item(NAME(m_host_x_step));
	save_item(NAME(m_host_y_step));
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
	// CONFIG_CHIP_ID: 15:0 = chip type, 23:16 = chip class, 31:24 = chip revision
	m_regs0[CONFIG_CHIP_ID] = 0x4754 | (u32(revision) << 24);
}

void atirageiic_device::device_start()
{
	// Rage IIc PCI
	set_ids(0x10024756, 0x00, 0x030000, 0x10026987);
	atirage_device::device_start();
	revision = 0x3a;
	m_regs0[CONFIG_CHIP_ID] = 0x4756 | (u32(revision) << 24);
}

// TODO: this core is currently hardwired to legacy x86 interface, as a testbed for p5txla
void atirageiidvd_device::device_start()
{
	// Mach64 GT-B [3D Rage II+ DVD]
	// TODO: verify subvendor ID & revision
	set_ids(0x10024755, 0x00, 0x030000, 0x10026987);
	atirage_device::device_start();
	revision = 0x3a;
	m_regs0[CONFIG_CHIP_ID] = 0x4755 | (u32(revision) << 24);

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
	m_regs0[CONFIG_CHIP_ID] = 0x4750 | (u32(revision) << 24);
}

void atirage_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
}

// PLL_ADDR selects one of the indirect clock synthesizer registers.  mach64 VT, 3D Rage,
// and Rage Pro all have 16 of them and a 4 bit address, but Rage LT Pro has 64 of them.
u32 atirage_device::pll_addr() const
{
	return BIT(m_regs0[CLOCK_CNTL], 10, 4);
}

u8 atirage_device::dac_read(int index)
{
	switch (index)
	{
		case DAC_WINDEX:
			return m_dac_windex;

		case DAC_DATA:
			{
				u8 result = 0;
				switch (m_dac_state)
				{
					case 0: // red
						result = ((m_dac_colors[m_dac_rindex] >> 16) & 0xff);
						break;

					case 1: // green
						result = ((m_dac_colors[m_dac_rindex] >> 8) & 0xff);
						break;

					case 2: // blue
						result = (m_dac_colors[m_dac_rindex] & 0xff);
						break;
				}

				if (!machine().side_effects_disabled())
				{
					m_dac_state++;
					if (m_dac_state >= 3)
					{
						m_dac_state = 0;
						m_dac_rindex++;
					}
				}
				return result;
			}

		case DAC_MASK:
			return m_dac_mask;

		case DAC_RINDEX:
			return m_dac_rindex;
	}

	return 0;
}

void atirage_device::dac_write(int index, u8 data)
{
	switch (index)
	{
		case DAC_WINDEX:
			m_dac_state = 0;
			m_dac_windex = data;
			break;

		case DAC_DATA:
			switch (m_dac_state)
			{
				case 0: // red
					m_dac_colors[m_dac_windex] &= 0x00ffff;
					m_dac_colors[m_dac_windex] |= (u32(data) << 16);
					break;

				case 1: // green
					m_dac_colors[m_dac_windex] &= 0xff00ff;
					m_dac_colors[m_dac_windex] |= (u32(data) << 8);
					break;

				case 2: // blue
					m_dac_colors[m_dac_windex] &= 0xffff00;
					m_dac_colors[m_dac_windex] |= data;
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

		case DAC_MASK:
			m_dac_mask = data;
			break;

		case DAC_RINDEX:
			m_dac_state = 0;
			m_dac_rindex = data;
			break;
	}
}

void atirage_device::vblank_w(int state)
{
	if (state)
	{
		m_regs0[CRTC_INT_CNTL] |= CRTC_VBLANK | CRTC_VBLANK_INT;
	}
	else
	{
		m_regs0[CRTC_INT_CNTL] &= ~CRTC_VBLANK;
	}

	update_irq();
}

void atirage_device::update_irq()
{
	const u32 int_cntl = m_regs0[CRTC_INT_CNTL];
	const bool active =
			((int_cntl & (CRTC_VBLANK_INT | CRTC_VBLANK_INT_EN)) == (CRTC_VBLANK_INT | CRTC_VBLANK_INT_EN))
			|| ((int_cntl & (CRTC_VLINE_INT | CRTC_VLINE_INT_EN)) == (CRTC_VLINE_INT | CRTC_VLINE_INT_EN));

	if (active != m_irq_active)
	{
		m_irq_active = active;
		write_irq(active ? ASSERT_LINE : CLEAR_LINE);
	}
}

u32 atirage_device::regs_0_read(offs_t offset, u32 mem_mask)
{
	switch (offset)
	{
		case CRTC_DAC_BASE:
			{
				u32 result = 0;
				for (int lane = 0; lane < 4; lane++)
				{
					if (BIT(mem_mask, lane * 8, 8) == 0xff)
					{
						result |= u32(dac_read(lane)) << (lane * 8);
					}
				}
				return result;
			}

		case CLOCK_CNTL:
			return (m_regs0[CLOCK_CNTL] & 0xff00ffff) | (u32(m_pll_regs[pll_addr()]) << 16);

		case FIFO_STAT:
			return 0;

		case GUI_STAT:
			return GUI_FIFO_ENTRIES << 16;
	}

	return m_regs0[offset];
}

void atirage_device::regs_0_write(offs_t offset, u32 data, u32 mem_mask)
{
	// 80s/90s Apple loved to write to read-only things, often as a way to trigger a
	// logic analyzer / in-circuit emulator.
	if (offset == CONFIG_CHIP_ID)
	{
		return;
	}

	LOGMASKED(LOG_REGISTERS, "regs_0_write: %08x & %08x to %03x\n", data, mem_mask, offset * 4);

	if (offset == CRTC_INT_CNTL)
	{
		static constexpr u32 STATUS = CRTC_VBLANK_INT | CRTC_VLINE_INT;

		const u32 old = m_regs0[CRTC_INT_CNTL];
		u32 value = old;
		COMBINE_DATA(&value);

		value = (value & ~CRTC_VBLANK) | (old & CRTC_VBLANK);
		value = (value & ~STATUS) | (old & STATUS & ~(data & mem_mask));

		m_regs0[CRTC_INT_CNTL] = value;
		update_irq();
		return;
	}

	if ((offset >= HOST_DATA0) && (offset <= HOST_DATA15))
	{
		host_data_w(data);
		return;
	}

	if (offset == CRTC_DAC_BASE)
	{
		for (int lane = 0; lane < 4; lane++)
		{
			if (BIT(mem_mask, lane * 8, 8) == 0xff)
				dac_write(lane, BIT(data, lane * 8, 8));
		}
		return;
	}

	COMBINE_DATA(&m_regs0[offset]);

	switch (offset)
	{
		case CRTC_OFF_PITCH:
		case CRTC_GEN_CNTL:
			update_mode();
			break;

		case DST_Y_X:
		case DST_Y_X_ALIAS:
			m_regs0[DST_Y] = BIT(data, 0, 15);
			m_regs0[DST_X] = BIT(data, 16, 14);
			break;

		case SRC_Y_X:
			m_regs0[SRC_Y] = BIT(data, 0, 15);
			m_regs0[SRC_X] = BIT(data, 16, 14);
			break;

		case SRC_Y_X_START:
			m_regs0[SRC_Y_START] = BIT(data, 0, 15);
			m_regs0[SRC_X_START] = BIT(data, 16, 14);
			break;

		case SRC_HEIGHT1_WIDTH1:
			m_regs0[SRC_HEIGHT1] = BIT(data, 0, 15);
			m_regs0[SRC_WIDTH1] = BIT(data, 16, 14);
			break;

		case SRC_HEIGHT2_WIDTH2:
			m_regs0[SRC_HEIGHT2] = BIT(data, 0, 15);
			m_regs0[SRC_WIDTH2] = BIT(data, 16, 14);
			break;

		case SC_LEFT_RIGHT:
			m_regs0[SC_LEFT] = BIT(data, 0, 13);
			m_regs0[SC_RIGHT] = BIT(data, 16, 13);
			break;

		case SC_TOP_BOTTOM:
			m_regs0[SC_TOP] = BIT(data, 0, 15);
			m_regs0[SC_BOTTOM] = BIT(data, 16, 15);
			break;

		case GUI_TRAJ_CNTL:
			m_regs0[DST_CNTL] = BIT(data, 0, 16);
			m_regs0[SRC_CNTL] = BIT(data, 16, 3)                // PATT_EN, PATT_ROT_EN, LINEAR_EN
					| (BIT(data, 29) << 3)                      // SRC_BYTE_ALIGN
					| (BIT(data, 19) << 4)                      // SRC_LINE_X_DIR
					| (BIT(data, 20) << 5);                     // SRC_TRACK_DST
			m_regs0[PAT_CNTL] = BIT(data, 23, 3);            	// MONO_EN, CLR_4x2_EN, CLR_8x1_EN
			m_regs0[HOST_CNTL] = BIT(data, 26)               	// HOST_BYTE_ALIGN
					| (BIT(data, 28) << 1);                  	// HOST_BIG_ENDIAN_EN
			break;

		// Initiator registers: writing one of these starts a rectangle blit
		case DST_HEIGHT_WIDTH:
			m_regs0[DST_HEIGHT] = BIT(data, 0, 15);
			m_regs0[DST_WIDTH] = BIT(data, 16, 14);
			draw_rectangle();
			break;

		case DST_X_WIDTH:
			m_regs0[DST_X] = BIT(data, 0, 14);
			m_regs0[DST_WIDTH] = BIT(data, 16, 14);
			draw_rectangle();
			break;

		case DST_WIDTH:
			draw_rectangle();
			break;

		case CLOCK_CNTL:
			if (ACCESSING_BITS_16_23 && BIT(m_regs0[CLOCK_CNTL], 9))
			{
				m_pll_regs[pll_addr()] = BIT(data, 16, 8);
			}
			break;

		case GP_IO:
			{
				const u16 ddr = BIT(m_regs0[GP_IO], 16, 16);
				const u16 out_data = u16(m_regs0[GP_IO]) & ddr;

				// send the data to an external handler
				// AND the pullups by the inverse of DDR, so bits set to input get the pullup
				write_gpio(out_data | (m_gpio_pullups & (ddr ^ 0xffff)));

				// get the updated data from the port
				u16 new_data = read_gpio();
				new_data &= (ddr ^ 0xffff);     // AND against inverted DDR mask so 0 bits are output
				new_data |= out_data;
				m_regs0[GP_IO] = (m_regs0[GP_IO] & 0xffff0000) | new_data;
			}
			break;
	}
}

static u32 pixel_bytes(u32 code)
{
	switch (code)
	{
		case 2: return 1;           // 8 bpp
		case 3: case 4: return 2;   // 15 bpp (5,5,5) and 16 bpp (5,6,5)
		case 6: return 4;           // 32 bpp
	}

	return 0;
}

static u32 pixel_mask(u32 bytes)
{
	return (bytes >= 4) ? 0xffffffffU : ((1U << (bytes * 8)) - 1);
}

static bool pixel_is_be(u32 code)
{
	return (code == 3) || (code == 6);
}

static u32 read_pixel(const u8 *p, u32 code)
{
	switch (pixel_bytes(code))
	{
		case 1: return p[0];
		case 2: return pixel_is_be(code) ? ((p[0] << 8) | p[1]) : ((p[1] << 8) | p[0]);
		case 4: return pixel_is_be(code)
				? ((u32(p[0]) << 24) | (p[1] << 16) | (p[2] << 8) | p[3])
				: ((u32(p[3]) << 24) | (p[2] << 16) | (p[1] << 8) | p[0]);
	}
	return 0;
}

static void write_pixel(u8 *p, u32 code, u32 value)
{
	switch (pixel_bytes(code))
	{
		case 1:
			p[0] = value;
			break;

		case 2:
			p[pixel_is_be(code) ? 0 : 1] = value >> 8;
			p[pixel_is_be(code) ? 1 : 0] = value;
			break;

		case 4:
			p[pixel_is_be(code) ? 0 : 3] = value >> 24;
			p[pixel_is_be(code) ? 1 : 2] = value >> 16;
			p[pixel_is_be(code) ? 2 : 1] = value >> 8;
			p[pixel_is_be(code) ? 3 : 0] = value;
			break;
	}
}

// and here are the ROPs
static u32 apply_mix(u32 mix, u32 s, u32 d)
{
	switch (mix & 0x1f)
	{
		case 0x0: return ~d;
		case 0x1: return 0;
		case 0x2: return ~u32(0);
		case 0x3: return d;
		case 0x4: return ~s;
		case 0x5: return d ^ s;
		case 0x6: return ~d ^ s;
		case 0x7: return s;
		case 0x8: return ~d | ~s;
		case 0x9: return d | ~s;
		case 0xa: return ~d | s;
		case 0xb: return d | s;
		case 0xc: return d & s;
		case 0xd: return ~d & s;
		case 0xe: return d & ~s;
		case 0xf: return ~d & ~s;
	}
	return s;
}

// Write one 8 bpp destination pixel through the tail of the pixel data path: scissors,
// the ALU mix, then the plane-wise write mask.  Returns false if the pixel was dropped.
bool atirage_device::dst_pixel(int x, int y, u32 src, u32 mix)
{
	if ((x < int(BIT(m_regs0[SC_LEFT], 0, 13))) || (x > int(BIT(m_regs0[SC_RIGHT], 0, 13))))
	{
		return false;
	}

	if ((y < int(BIT(m_regs0[SC_TOP], 0, 15))) || (y > int(BIT(m_regs0[SC_BOTTOM], 0, 15))))
	{
		return false;
	}

	const u32 code = BIT(m_regs0[DP_PIX_WIDTH], 0, 4);
	const u32 bytes = pixel_bytes(code);

	// DST_OFFSET is in 64 bit words regardless of the depth, DST_PITCH is in 8-pixel units
	const u32 pitch = BIT(m_regs0[DST_OFF_PITCH], 22, 10) * 8;
	const u32 addr = (BIT(m_regs0[DST_OFF_PITCH], 0, 20) * 8)
			+ (((u32(y) * pitch) + u32(x)) * bytes);
	if ((addr + bytes) > m_mach64->get_framebuffer_size())
		return false;

	u8 *const vram = m_mach64->get_framebuffer_addr();
	const u32 mask = m_regs0[DP_WRITE_MSK] & pixel_mask(bytes);
	const u32 dst = read_pixel(&vram[addr], code);
	const u32 res = apply_mix(mix, src, dst);
	write_pixel(&vram[addr], code, (dst & ~mask) | (res & mask));
	return true;
}

// fetch one bit from a monochrome pattern
bool atirage_device::pattern_bit(int x, int y) const
{
	const u32 line = BIT(y, 2) ? m_regs0[PAT_REG1] : m_regs0[PAT_REG0];
	const u32 bits = BIT(line, BIT(y, 0, 2) * 8, 8);
	const int col = x & 7;
	return BIT(bits, BIT(m_regs0[DP_PIX_WIDTH], 24) ? col : (7 - col));
}

// Read one source pixel for a blit
u32 atirage_device::src_pixel(int x, int y)
{
	const u32 code = BIT(m_regs0[DP_PIX_WIDTH], 8, 4);
	const u32 bytes = pixel_bytes(code);

	// SRC_OFFSET is in 64 bit words regardless of the depth, SRC_PITCH is in 8-pixel units
	const u32 pitch = BIT(m_regs0[SRC_OFF_PITCH], 22, 10) * 8;
	const u32 addr = (BIT(m_regs0[SRC_OFF_PITCH], 0, 20) * 8)
			+ (((u32(y) * pitch) + u32(x)) * bytes);
	if ((addr + bytes) > m_mach64->get_framebuffer_size())
	{
		return 0;
	}

	return read_pixel(&m_mach64->get_framebuffer_addr()[addr], code);
}

// Host data is a single register aliased over sixteen consecutive addresses for burst
// write operations.
void atirage_device::host_data_w(u32 data)
{
	if (!m_host_active)
	{
		return;
	}

	const u32 host_code = BIT(m_regs0[DP_PIX_WIDTH], 16, 4);
	const u32 host_bytes = pixel_bytes(host_code);
	const int per_dword = m_host_mono ? 32 : int(4 / host_bytes);

	const u32 clr_mask = pixel_mask(pixel_bytes(BIT(m_regs0[DP_PIX_WIDTH], 0, 4)));
	const u32 frgd = (BIT(m_regs0[DP_SRC], 8, 3) == SRC_FRGD_CLR)
			? (m_regs0[DP_FRGD_CLR] & clr_mask) : (m_regs0[DP_BKGD_CLR] & clr_mask);
	const u32 bkgd = (BIT(m_regs0[DP_SRC], 0, 3) == SRC_FRGD_CLR)
			? (m_regs0[DP_FRGD_CLR] & clr_mask) : (m_regs0[DP_BKGD_CLR] & clr_mask);
	const u32 frgd_mix = BIT(m_regs0[DP_MIX], 16, 5);
	const u32 bkgd_mix = BIT(m_regs0[DP_MIX], 0, 5);

	const bool lsb_first = BIT(m_regs0[DP_PIX_WIDTH], 24);
	const bool byte_align = BIT(m_regs0[HOST_CNTL], 0);

	for (int i = 0; i < per_dword; )
	{
		if (m_host_mono)
		{
			const int lane = (m_host_x_step > 0) ? (i >> 3) : (3 - (i >> 3));
			const int nbit = i & 7;
			const bool set = BIT(BIT(data, lane * 8, 8), lsb_first ? nbit : (7 - nbit));

			dst_pixel(m_host_x, m_host_y, set ? frgd : bkgd, set ? frgd_mix : bkgd_mix);
		}
		else
		{
			// pixels come out of the low end of the dword for a left to right destination
			const int lane = (m_host_x_step > 0) ? i : (per_dword - 1 - i);
			u8 bytes[4];
			for (int b = 0; b < 4; b++)
			{
				bytes[b] = BIT(data, b * 8, 8);
			}
			dst_pixel(m_host_x, m_host_y, read_pixel(&bytes[lane * host_bytes], host_code), frgd_mix);
		}
		i++;

		m_host_x += m_host_x_step;
		if (++m_host_col >= m_host_width)
		{
			m_host_col = 0;
			m_host_x = m_host_start_x;
			m_host_y += m_host_y_step;

			if (--m_host_lines <= 0)
			{
				m_host_active = false;
				return;
			}

			// Host data is packed, so by default the next line resumes mid-dword.  With
			// HOST_BYTE_ALIGN the rest of the current byte is padding instead, which is
			// how a driver feeds bitmaps whose rows are padded out - it only means
			// anything when a pixel is narrower than a byte.
			if (byte_align && m_host_mono)
			{
				i = (i + 7) & ~7;
			}
		}
	}
}

void atirage_device::draw_rectangle()
{
	const u32 dst_bytes = pixel_bytes(BIT(m_regs0[DP_PIX_WIDTH], 0, 4));
	if (dst_bytes == 0)
	{
		LOGMASKED(LOG_ENGINE, "draw_rectangle: unhandled DP_PIX_WIDTH %08x\n", m_regs0[DP_PIX_WIDTH]);
		return;
	}
	const u32 clr_mask = pixel_mask(dst_bytes);

	// starting any operation abandons a transfer that never got all its data
	m_host_active = false;

	const u32 mono_src = BIT(m_regs0[DP_SRC], 16, 3);
	const u32 frgd_src = BIT(m_regs0[DP_SRC], 8, 3);
	const u32 bkgd_src = BIT(m_regs0[DP_SRC], 0, 3);

	// A monochrome source picks the foreground or background half of the data path per pixel.
	if ((mono_src != MONO_ALWAYS_1) && (mono_src != MONO_HOST) && (mono_src != MONO_PATTERN))
	{
		LOGMASKED(LOG_ENGINE, "draw_rectangle: unhandled DP_MONO_SRC %d\n", mono_src);
		return;
	}

	// Color expansion only makes sense when both halves come from a color register.
	if (((mono_src == MONO_HOST) || (mono_src == MONO_PATTERN))
			&& (((frgd_src != SRC_FRGD_CLR) && (frgd_src != SRC_BKGD_CLR))
				|| ((bkgd_src != SRC_FRGD_CLR) && (bkgd_src != SRC_BKGD_CLR))))
	{
		LOGMASKED(LOG_ENGINE, "draw_rectangle: unhandled mono expansion sources %d/%d\n",
				frgd_src, bkgd_src);
		return;
	}

	u32 src = 0;
	switch (mono_src == MONO_HOST ? u32(SRC_HOST) : frgd_src)
	{
		case SRC_BKGD_CLR: src = m_regs0[DP_BKGD_CLR] & clr_mask; break;
		case SRC_FRGD_CLR: src = m_regs0[DP_FRGD_CLR] & clr_mask; break;

		case SRC_HOST:
			if ((mono_src != MONO_HOST) && (pixel_bytes(BIT(m_regs0[DP_PIX_WIDTH], 16, 4)) != dst_bytes))
			{
				LOGMASKED(LOG_ENGINE, "draw_rectangle: unhandled DP_HOST_PIX_WIDTH %08x\n", m_regs0[DP_PIX_WIDTH]);
				return;
			}
			break;

		case SRC_BLIT:
			if (pixel_bytes(BIT(m_regs0[DP_PIX_WIDTH], 8, 4)) != dst_bytes)
			{
				LOGMASKED(LOG_ENGINE, "draw_rectangle: unhandled DP_SRC_PIX_WIDTH %08x\n", m_regs0[DP_PIX_WIDTH]);
				return;
			}
			break;

		default:
			LOGMASKED(LOG_ENGINE, "draw_rectangle: unhandled DP_FRGD_SRC %d\n", frgd_src);
			return;
	}

	if (BIT(m_regs0[CLR_CMP_CNTL], 0, 3) != 0)
	{
		LOGMASKED(LOG_ENGINE, "draw_rectangle: colour compare %08x ignored\n", m_regs0[CLR_CMP_CNTL]);
	}

	const int width = BIT(m_regs0[DST_WIDTH], 0, 14);
	const int height = BIT(m_regs0[DST_HEIGHT], 0, 15);
	const int start_x = BIT(m_regs0[DST_X], 0, 14);
	const int start_y = BIT(m_regs0[DST_Y], 0, 15);
	const int x_step = BIT(m_regs0[DST_CNTL], 0) ? 1 : -1;  // DST_X_DIR, 1 = left to right
	const int y_step = BIT(m_regs0[DST_CNTL], 1) ? 1 : -1;  // DST_Y_DIR, 1 = top to bottom

	LOGMASKED(LOG_ENGINE, "fill %dx%d at %d,%d dir %d/%d colour %02x mix %x\n",
			width, height, start_x, start_y, x_step, y_step, src, BIT(m_regs0[DP_MIX], 16, 5));

	// HACK? Ahead of every line of text the MacOS ATI driver executes a single-line fill at the
	// left edge of the screen that's exactly one 64 bit word wide.  Possibly a workaround for a
	// hardware bug?  DingusPPC has a similar hack for the same problem, so it's not specific to
	// how we implement the Rage.
	if ((height == 1) && (start_x == 0) && ((u32(width) * dst_bytes) == 8)
			&& (mono_src != MONO_PATTERN))
	{
		LOGMASKED(LOG_ENGINE, "draw_rectangle: dropping the %dx%d left edge mark at 0,%d\n",
				width, height, start_y);
		return;
	}

	if ((frgd_src == SRC_HOST) || (mono_src == MONO_HOST))
	{
		m_host_mono = (mono_src == MONO_HOST);
		m_host_active = (width > 0) && (height > 0);
		m_host_x = start_x;
		m_host_y = start_y;
		m_host_start_x = start_x;
		m_host_col = 0;
		m_host_width = width;
		m_host_lines = height;
		m_host_x_step = x_step;
		m_host_y_step = y_step;
		return;
	}

	// Source trajectory, per the SRC_CNTL table.
	const bool patt_en = BIT(m_regs0[SRC_CNTL], 0);
	const bool patt_rot = BIT(m_regs0[SRC_CNTL], 1);
	const bool linear_en = BIT(m_regs0[SRC_CNTL], 2);

	const int src_x0 = BIT(m_regs0[SRC_X], 0, 14);
	const int src_y0 = BIT(m_regs0[SRC_Y], 0, 15);
	const int x_reset = patt_rot ? int(BIT(m_regs0[SRC_X_START], 0, 14)) : src_x0;
	const int y_reset = patt_rot ? int(BIT(m_regs0[SRC_Y_START], 0, 15)) : src_y0;
	const int width1 = BIT(m_regs0[SRC_WIDTH1], 0, 14);
	const int height1 = BIT(m_regs0[SRC_HEIGHT1], 0, 15);
	const int width2 = patt_rot ? int(BIT(m_regs0[SRC_WIDTH2], 0, 14)) : width1;
	const int height2 = patt_rot ? int(BIT(m_regs0[SRC_HEIGHT2], 0, 15)) : height1;

	int sx = src_x0, sy = src_y0;
	int x_span = width1, x_left = width1;
	int y_span = height1, y_left = height1;
	u32 linear = 0;

	const u32 frgd_mix = BIT(m_regs0[DP_MIX], 16, 5);
	const u32 bkgd_mix = BIT(m_regs0[DP_MIX], 0, 5);
	const int sc_top = BIT(m_regs0[SC_TOP], 0, 15);
	const int sc_bottom = BIT(m_regs0[SC_BOTTOM], 0, 15);

	// The two halves of the data path for a monochrome pattern to choose between.
	const u32 frgd_clr = (frgd_src == SRC_FRGD_CLR)
			? (m_regs0[DP_FRGD_CLR] & clr_mask) : (m_regs0[DP_BKGD_CLR] & clr_mask);
	const u32 bkgd_clr = (bkgd_src == SRC_FRGD_CLR)
			? (m_regs0[DP_FRGD_CLR] & clr_mask) : (m_regs0[DP_BKGD_CLR] & clr_mask);
	const bool mono_pat = (mono_src == MONO_PATTERN);

	for (int iy = 0; iy < height; iy++)
	{
		const int y = start_y + (iy * y_step);
		const bool line_visible = (y >= sc_top) && (y <= sc_bottom);

		for (int ix = 0; ix < width; ix++)
		{
			if (frgd_src == SRC_BLIT)
			{
				if (line_visible)
				{
					src = linear_en ? src_pixel(int(linear), 0) : src_pixel(sx, sy);
				}

				// the source is still consumed even if the destination is clipped
				linear++;
				sx += x_step;

				// A zero span means the source never wraps.
				if (!linear_en && (x_span > 0) && (--x_left <= 0))
				{
					sx = x_reset;
					x_span = patt_en ? width2 : width1;
					x_left = x_span;
				}
			}

			if (line_visible)
			{
				const int x = start_x + (ix * x_step);

				if (mono_pat)
				{
					const bool set = pattern_bit(x, y);
					dst_pixel(x, y, set ? frgd_clr : bkgd_clr, set ? frgd_mix : bkgd_mix);
				}
				else
				{
					dst_pixel(x, y, src, frgd_mix);
				}
			}
		}

		// The destination advancing in Y restarts the X traversal and steps the source Y.
		if ((frgd_src == SRC_BLIT) && !linear_en)
		{
			sx = src_x0;
			x_span = width1;
			x_left = x_span;
			sy += y_step;

			// only the pattern trajectories bound the source in Y
			if (patt_en && (--y_left <= 0))
			{
				sy = y_reset;
				y_span = height2;
				y_left = y_span;
			}
		}
	}

	if (BIT(m_regs0[DST_CNTL], 3))
	{
		m_regs0[DST_X] = (start_x + (width * x_step)) & 0x3fff;
	}

	if (BIT(m_regs0[DST_CNTL], 4))
	{
		m_regs0[DST_Y] = (start_y + (height * y_step)) & 0x7fff;
	}
}

// The cursor image is a 64x64 array of 2 bit pixels, 16 bytes per line, little-endian.
void atirage_device::draw_cursor(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const u32 base = BIT(m_regs0[CUR_OFFSET], 0, 24) * 8;
	if ((base + (64 * 16)) > m_mach64->get_framebuffer_size())
	{
		LOGMASKED(LOG_ENGINE, "cursor definition at %x is outside VRAM\n", base);
		return;
	}
	const u8 *const cursor = m_mach64->get_framebuffer_addr() + base;

	const int xpos = BIT(m_regs0[CUR_HORZ_VERT_POSN], 0, 11);
	const int ypos = BIT(m_regs0[CUR_HORZ_VERT_POSN], 16, 11);
	const int xoff = BIT(m_regs0[CUR_HORZ_VERT_OFF], 0, 6);
	const int yoff = BIT(m_regs0[CUR_HORZ_VERT_OFF], 16, 6);
	const u32 color0 = BIT(m_regs0[CUR_CLR0], 8, 24);
	const u32 color1 = BIT(m_regs0[CUR_CLR1], 8, 24);

	for (int line = 0; line < (64 - yoff); line++)
	{
		const int y = ypos + line;
		if ((y < cliprect.top()) || (y > cliprect.bottom()))
		{
			continue;
		}

		const u8 *const src = &cursor[line * 16];
		u32 *const dst = &bitmap.pix(y, 0);

		for (int col = xoff; col < 64; col++)
		{
			const int x = xpos + col - xoff;
			if ((x < cliprect.left()) || (x > cliprect.right()))
			{
				continue;
			}

			// Cursor pixels are a 2-bit ROP: color0, color1, transparent, or invert the destination.
			switch (BIT(src[col >> 2], (col & 3) * 2, 2))
			{
				case 0: dst[x] = color0; break;
				case 1: dst[x] = color1; break;
				case 2: break;
				case 3: dst[x] = ~dst[x] & 0xffffff; break;
			}
		}
	}
}

u32 atirage_device::regs_1_read(offs_t offset, u32 mem_mask)
{
	LOGMASKED(LOG_REGISTERS, "regs_1_read: & %08x @ %03x\n", mem_mask, offset * 4);
	return m_regs1[offset];
}

void atirage_device::regs_1_write(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_REGISTERS, "regs_1_write: %08x & %08x to %03x\n", data, mem_mask, offset * 4);
	COMBINE_DATA(&m_regs1[offset]);
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
	if (BIT(m_regs0[CRTC_GEN_CNTL], 24, 2) != 3)
	{
		LOGMASKED(LOG_CRTC, "VGA mode must be OFF and CRTC must be ON\n");
		return;
	}

	m_htotal = BIT(m_regs0[CRTC_H_TOTAL_DISP], 0, 9) + 1;
	m_htotal <<= 3; // in units of 8 pixels
	m_hres = BIT(m_regs0[CRTC_H_TOTAL_DISP], 16, 8) + 1;
	m_hres <<= 3;
	m_vres = BIT(m_regs0[CRTC_V_TOTAL_DISP], 16, 11) + 1;
	m_vtotal = BIT(m_regs0[CRTC_V_TOTAL_DISP], 0, 11) + 1;
	m_format = BIT(m_regs0[CRTC_GEN_CNTL], 8, 3);   // CRTC_PIX_WIDTH
	LOGMASKED(LOG_CRTC, "Setting mode (%d x %d), total (%d x %d) format %d\n", m_hres, m_vres, m_htotal, m_vtotal, m_format);

	if ((m_hres >= m_htotal) || (m_vres >= m_vtotal))
	{
		LOGMASKED(LOG_CRTC, "atirage: CRTC timings don't make sense, ignoring\n");
		return;
	}

	double vpll_frequency;
	int clk_source = BIT(m_regs0[CLOCK_CNTL], 0, 2);

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
	m_screen->configure(m_htotal, m_vtotal, visarea, attotime::from_ticks(m_htotal * m_vtotal, m_pixel_clock));
}

u32 atirage_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// are we in VGA mode rather than native?  if so, let the legacy VGA stuff draw.
	if (!BIT(m_regs0[CRTC_GEN_CNTL], 24))
	{
		return m_mach64->screen_update(screen, bitmap, cliprect);
	}

	// is the CRTC not enabled or the display disable bit set?
	if (!BIT(m_regs0[CRTC_GEN_CNTL], 25) || BIT(m_regs0[CRTC_GEN_CNTL], 6))
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	// CRTC_OFFSET is in units of 64 bit words, CRTC_PITCH is in 8-pixel units
	const u32 offset = BIT(m_regs0[CRTC_OFF_PITCH], 0, 20);
	const u32 pitch = BIT(m_regs0[CRTC_OFF_PITCH], 22, 10) * 8;    // pixels per line
	const u8 *const vram = m_mach64->get_framebuffer_addr() + (offset * 8);

	switch (m_format)
	{
		case 2: // 8 bpp (also can be a weird 2/2/3 direct color mode)
			for (u32 y = 0; y < m_vres; y++)
			{
				const u8 *src = &vram[pitch * y];
				u32 *dst = &bitmap.pix(y, 0);
				for (u32 x = 0; x < m_hres; x++)
				{
					*dst++ = m_dac_colors[src[x]];
				}
			}
			break;

		case 3: // 15 bpp (5,5,5) aka MacOS "thousands of colors"
			for (u32 y = 0; y < m_vres; y++)
			{
				const u8 *src = &vram[pitch * 2 * y];
				u32 *dst = &bitmap.pix(y, 0);
				for (u32 x = 0; x < m_hres; x++, src += 2)
				{
					const u16 pixel = (src[0] << 8) | src[1];
					*dst++ = rgb_t(pal5bit(pixel >> 10), pal5bit(pixel >> 5), pal5bit(pixel));
				}
			}
			break;

		case 4: // 16 bpp (5,6,5)
			for (u32 y = 0; y < m_vres; y++)
			{
				const u8 *src = &vram[pitch * 2 * y];
				u32 *dst = &bitmap.pix(y, 0);
				for (u32 x = 0; x < m_hres; x++, src += 2)
				{
					const u16 pixel = (src[1] << 8) | src[0];
					*dst++ = rgb_t(pal5bit(pixel >> 11), pal6bit(pixel >> 5), pal5bit(pixel));
				}
			}
			break;

		case 6: // 32 bpp aka MacOS "millions of colors"
			for (u32 y = 0; y < m_vres; y++)
			{
				const u8 *src = &vram[pitch * 4 * y];
				u32 *dst = &bitmap.pix(y, 0);
				for (u32 x = 0; x < m_hres; x++, src += 4)
				{
					*dst++ = rgb_t(src[1], src[2], src[3]);  // memory order is A, R, G, B
				}
			}
			break;

		default:
			LOG("Unknown pixel format %d\n", m_format);
			break;
	}

	if (BIT(m_regs0[GEN_TEST_CNTL], 7))
	{
		draw_cursor(bitmap, cliprect);
	}

	return 0;
}
