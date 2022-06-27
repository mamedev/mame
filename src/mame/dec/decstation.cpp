// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    decstation.cpp: MIPS-based DECstation family

    WANTED: boot ROM dumps for KN02CA/KN04CA (MAXine) systems.

    NOTE: after all the spew of failing tests, press 'q' at the MORE prompt and
    wait a few seconds for the PROM monitor to appear.
    Type 'ls' for a list of commands (this is a very UNIX-flavored PROM monitor).

    Machine types:
        DECstation 3100 (PMAX/KN01):
            16.67 MHz R2000 with FPU and MMU
            24 MiB max RAM
            Serial: DEC "DZ" quad-UART (DC7085 gate array)
            SCSI: DEC "SII" SCSI interface (DC7061 gate array)
            Ethernet: AMD7990 "LANCE" controller
            Monochrome or color video on-board
        PMIN/KN01:
            Cheaper PMAX, 12.5 MHz R2000, othersame as PMAX

        Personal DECstation 5000/xx (MAXine/KN02CA for R3000, KN04CA? for R4000)
            20, 25, or 33 MHz R3000 or 100 MHz R4000
            40 MiB max RAM
            Serial: DEC "DZ" quad-UART for keyboard/mouse, SCC8530 for modem/printer
            SCSI: NCR53C94
            Ethernet: AMD7990 "LANCE" controller
            Audio/ISDN: AMD AM79C30
            Color 1024x768 8bpp video on-board
            2 TURBOchannel slots

        DECstation 5000/1xx: (3MIN/KN02BA, KN04BA? for R4000):
            20, 25, or 33 MHz R3000 or 100 MHz R4000
            128 MiB max RAM
            Serial: 2x SCC8530
            SCSI: NCR53C94
            Ethernet: AMD7990 "LANCE" controller
            No on-board video
            3 TURBOchannel slots

        DECstation 5000/200: (3MAX/KN02):
            25 MHz R3000
            480 MiB max RAM
            Serial: DEC "DZ" quad-UART
            SCSI: NCR53C94
            Ethernet: AMD7990 "LANCE" controllor

        DECstation 5000/240 (3MAX+/KN03AA), 5000/260 (3MAX+/KN05)
            40 MHz R3400, or 120 MHz R4400.
            480 MiB max RAM
            Serial: 2x SCC8530
            SCSI: NCR53C94
            Ethernet: AMD7990 "LANCE" controller

****************************************************************************/

#include "emu.h"
#include "cpu/mips/mips1.h"
#include "cpu/mips/mips3.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "decioga.h"
#include "machine/mc146818.h"
#include "machine/z80scc.h"
#include "machine/ncr5390.h"
//#include "machine/dc7061.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "dec_lk201.h"
#include "machine/am79c90.h"
#include "machine/dc7085.h"
#include "bus/rs232/rs232.h"
#include "screen.h"
#include "video/bt47x.h"
#include "video/bt459.h"
#include "video/decsfb.h"

#include "kn01.lh"

namespace {

class kn01_state : public driver_device
{
public:
	kn01_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_mram(*this, "mram")
		, m_esar(*this, "esar")
		, m_rtc(*this, "rtc")
		, m_dz(*this, "dc7085")
		//, m_sii(*this, "scsi:6:sii")
		, m_lance(*this, "am79c90")
		, m_lk201(*this, "lk201")
		, m_screen(*this, "screen")
		, m_vdac(*this, "bt478")
		, m_scantimer(*this, "scantimer")
		, m_vram(*this, "vram")
		, m_config(*this, "config")
		, m_leds(*this, "led%u", 0U)
	{
	}

	void kn01(machine_config &config, u32 clock);
	void pmax(machine_config &config) { kn01(config, (33.3_MHz_XTAL / 2).value()); }
	void pmin(machine_config &config) { kn01(config, 12'000'000); }

	void init();

protected:
	u16 status_r();
	void control_w(u16 data);

	u16 pcc_r(offs_t offset);
	void pcc_w(offs_t offset, u16 data);

	void vram_w(offs_t offset, u32 data, u32 mem_mask);
	void plane_mask_w(u8 data);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);

private:
	virtual void machine_start() override {}
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	required_device<mips1_device_base> m_cpu;
	required_device<ram_device> m_mram;
	required_region_ptr<u8> m_esar;

	required_device<mc146818_device> m_rtc;
	required_device<dc7085_device> m_dz;
	//required_device<dc7061_device> m_sii;
	required_device<am79c90_device> m_lance;

	optional_device<lk201_device> m_lk201;

	optional_device<screen_device> m_screen;
	optional_device<bt478_device> m_vdac;
	optional_device<timer_device> m_scantimer;
	optional_shared_ptr<u32> m_vram;

	required_ioport m_config;
	output_finder<8> m_leds;

	void map(address_map &map);

	enum sys_csr_mask : u16
	{
		VRGTRB  = 0x0001, // video dac voltage red > blue
		VRGTRG  = 0x0002, // video dac voltage red > green
		VBGTRG  = 0x0004, // video dac voltage blue > green
		TXDIS   = 0x0100, // disable serial transmit drivers
		VINT    = 0x0200, // pcc programmable area detect 2
		MEMERR  = 0x0400, // bus timeout on write
		MONO    = 0x0800, // monochrome framebuffer installed
		CRSRTST = 0x1000, // pcc test output
		PARDIS  = 0x2000, // memory parity disable
		STATUS  = 0x4000, // self-test completed successfully
		MNFMOD  = 0x8000, // manufacturing self test jumper installed
	};
	u16 m_status;

	u32 m_plane_mask;

	enum pcc_regnum : unsigned
	{
		PCC_CMDR   =  0,
		PCC_XPOS   =  1,
		PCC_YPOS   =  2,
		PCC_XMIN1  =  3,
		PCC_XMAX1  =  4,
		PCC_YMIN1  =  5,
		PCC_YMAX1  =  6,
		PCC_XMIN2  = 11,
		PCC_XMAX2  = 12,
		PCC_YMIN2  = 13,
		PCC_YMAX2  = 14,
		PCC_MEMORY = 15,
	};
	u16 m_pcc_regs[16];

	std::unique_ptr<u16[]> m_dram; // disk ram
	std::unique_ptr<u16[]> m_nram; // network ram

	enum msr_mask : u16
	{
		MSR_DSR3 = 0x0001,
		MSR_DSR2 = 0x0200,
	};
	u16 m_msr;
};

class kn02ba_state : public driver_device
{
public:
	kn02ba_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_screen(*this, "screen")
		, m_scantimer(*this, "scantimer")
		, m_sfb(*this, "sfb")
		, m_lk201(*this, "lk201")
		, m_ioga(*this, "ioga")
		, m_rtc(*this, "rtc")
		, m_scc(*this, "scc%u", 0U)
		, m_asc(*this, "scsi:7:asc")
		, m_vrom(*this, "gfx")
		, m_bt459(*this, "bt459")
		, m_lance(*this, "am79c90")
		, m_dz(*this, "dc7085")
	{
	}

	void kn02ba(machine_config &config, u32 clock);
	void m120(machine_config &config) { kn02ba(config, 20'000'000); }
	void m125(machine_config &config) { kn02ba(config, 25'000'000); }
	void m133(machine_config &config) { kn02ba(config, 33'300'000); }

	void init() {}

protected:
	uint32_t cfb_r(offs_t offset);
	void cfb_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<mips1_device_base> m_cpu;
	required_device<screen_device> m_screen;
	optional_device<timer_device> m_scantimer;
	optional_device<decsfb_device> m_sfb;
	optional_device<lk201_device> m_lk201;
	required_device<dec_ioga_device> m_ioga;
	required_device<mc146818_device> m_rtc;
	required_device_array<z80scc_device, 2> m_scc;
	optional_device<ncr53c94_device> m_asc;
	optional_memory_region m_vrom;
	optional_device<bt459_device> m_bt459;
	required_device<am79c90_device> m_lance;
	optional_device<dc7085_device> m_dz;

	void map(address_map &map);

	u8 *m_vrom_ptr;
};

/***************************************************************************
    VIDEO HARDWARE
***************************************************************************/

uint32_t kn01_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	u32 *pixel_pointer = m_vram;

	switch (m_config->read() & ~MNFMOD)
	{
	case 0x0000:
		for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
		{
			for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 4)
			{
				u32 const pixel_data = *pixel_pointer++;

				bitmap.pix(y, x + 3) = m_vdac->pen_color(u8(pixel_data >> 24));
				bitmap.pix(y, x + 2) = m_vdac->pen_color(u8(pixel_data >> 16));
				bitmap.pix(y, x + 1) = m_vdac->pen_color(u8(pixel_data >> 8));
				bitmap.pix(y, x + 0) = m_vdac->pen_color(u8(pixel_data >> 0));
			}
		}
		break;
	case 0x0800:
		for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
		{
			for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 32)
			{
				u32 const pixel_data = *pixel_pointer++;

				for (unsigned i = 0; i < 32; i++)
					bitmap.pix(y, x + i) = m_vdac->pen_color(BIT(pixel_data, i) * 128);
			}

			pixel_pointer += 1024 / 32;
		}
		break;
	}

	return 0;
}

uint32_t kn02ba_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bt459->screen_update(screen, bitmap, cliprect, (uint8_t *)m_sfb->get_vram());
	return 0;
}

uint32_t kn02ba_state::cfb_r(offs_t offset)
{
	uint32_t addr = offset << 2;

	//logerror("cfb_r: reading at %x\n", addr);

	if (addr < 0x80000)
	{
		return m_vrom_ptr[addr>>2] & 0xff;
	}

	if ((addr >= 0x100000) && (addr < 0x100200))
	{
	}

	if ((addr >= 0x200000) && (addr < 0x400000))
	{
	}

	return 0xffffffff;
}

void kn02ba_state::cfb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t addr = offset << 2;

	if ((addr >= 0x100000) && (addr < 0x100200))
	{
		return;
	}

	if ((addr >= 0x1c0000) && (addr < 0x200000))
	{
		//printf("Bt459: %08x (mask %08x) @ %x\n", data, mem_mask, offset<<2);
		return;
	}

	if ((addr >= 0x200000) && (addr < 0x400000))
	{
	}
}


u16 kn01_state::pcc_r(offs_t offset)
{
	return m_pcc_regs[offset];
}

void kn01_state::pcc_w(offs_t offset, u16 data)
{
	m_pcc_regs[offset] = data;
}

void kn01_state::plane_mask_w(u8 data)
{
	m_plane_mask = (u32(data) << 24) | (u32(data) << 16) | (u32(data) << 8) | (u32(data) << 0);
}

TIMER_DEVICE_CALLBACK_MEMBER(kn01_state::scanline_timer)
{
	int scanline = m_screen->vpos();

	if ((scanline == m_pcc_regs[PCC_YMIN2]) && (m_pcc_regs[PCC_CMDR] & 0x0400) && !(m_status & VINT))
	{
		m_status |= VINT;
		m_cpu->set_input_line(INPUT_LINE_IRQ4, ASSERT_LINE);
	}

	if ((scanline == m_pcc_regs[PCC_YMIN1]) && (m_pcc_regs[PCC_CMDR] & 0x0100))
	{
		int x, y;
		u8 *vram = (u8 *)m_vram.target();

		x = m_pcc_regs[PCC_XMIN1] - 212;
		y = m_pcc_regs[PCC_YMIN1] - 34;
		//printf("sampling for VRGTRB and friends at X=%d Y=%d\n", x, y);
		m_status &= ~(VBGTRG | VRGTRG | VRGTRB);
		if ((x >= 0) && (x <= 1023) && (y >= 0) && (y <= 863))
		{
			rgb_t const rgb = m_vdac->pen_color(vram[(y * 1024) + x]);

			//printf("R=%d, G=%d, B=%d\n", r, g, b);
			if (rgb.r() > rgb.b()) m_status |= VRGTRB;
			if (rgb.r() > rgb.g()) m_status |= VRGTRG;
			if (rgb.b() > rgb.g()) m_status |= VBGTRG;
		}
	}
}

/***************************************************************************
    MACHINE FUNCTIONS
***************************************************************************/

void kn01_state::init()
{
	m_dram = make_unique_clear<u16[]>(65536);
	m_nram = make_unique_clear<u16[]>(32768);

	m_leds.resolve();

	m_cpu->space(AS_PROGRAM).install_ram(0, m_mram->mask(), m_mram->pointer());

	m_cpu->space(AS_PROGRAM).install_readwrite_tap(0x1c000000, 0x1c00001b, "dz_delay",
		[this](offs_t offset, u32 &data, u32 mem_mask) { m_cpu->eat_cycles(13); },
		[this](offs_t offset, u32 &data, u32 mem_mask) { m_cpu->eat_cycles(18); });

#if 0
	m_cpu->space(AS_PROGRAM).install_readwrite_tap(0x1d000000, 0x1d0000ff, "rtc_delay",
		[this](offs_t offset, u32 &data, u32 mem_mask) { m_cpu->eat_cycles(13); },
		[this](offs_t offset, u32 &data, u32 mem_mask) { m_cpu->eat_cycles(16); });

	switch (m_config->read() & ~MNFMOD)
	{
	case 0x0800: // monochrome
		m_cpu->space(AS_PROGRAM).unmap_readwrite(0x0fc20000, 0x0fcfffff);
		break;

	case 0x0801: // none
		m_cpu->space(AS_PROGRAM).unmap_readwrite(0x0fc00000, 0x0fcfffff);
		break;
	}
#endif
}

void kn01_state::machine_reset()
{
	m_status = m_config->read() & (MNFMOD | MONO);

	for (auto &l : m_leds)
		l = 0;

	m_plane_mask = 0;
}

void kn01_state::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	mem_mask &= m_plane_mask;

	COMBINE_DATA(&m_vram[offset]);
}

void kn02ba_state::machine_start()
{
	if (m_vrom)
		m_vrom_ptr = m_vrom->base();
}

void kn02ba_state::machine_reset()
{
	m_ioga->set_dma_space(&m_cpu->space(AS_PROGRAM));
}

u16 kn01_state::status_r()
{
	return m_status;
}

void kn01_state::control_w(u16 data)
{
	// update leds
	for (unsigned i = 0; i < 8; i++)
		m_leds[i] = BIT(data, i);

	if (data & VINT)
	{
		m_cpu->set_input_line(INPUT_LINE_IRQ4, CLEAR_LINE);
		m_status &= ~VINT;
	}

	if (data & MEMERR)
		m_status &= ~MEMERR;

	if (data & PARDIS)
	{
		// TODO: disable parity checking
	}

	if (data & STATUS)
	{
		// TODO: assert status output
	}

	m_status = (m_status & ~(TXDIS | PARDIS | STATUS)) | (data & (TXDIS | PARDIS | STATUS));
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void kn01_state::map(address_map &map)
{
	map(0x0fc00000, 0x0fcfffff).ram().share("vram").w(FUNC(kn01_state::vram_w));

	map(0x10000000, 0x10000000).w(FUNC(kn01_state::plane_mask_w));
	map(0x11000000, 0x1100003f).rw(FUNC(kn01_state::pcc_r), FUNC(kn01_state::pcc_w)).umask32(0x0000ffff);
	map(0x12000000, 0x1200001f).m(m_vdac, FUNC(bt478_device::map)).umask32(0x000000ff).mirror(0xe0);
	map(0x18000000, 0x18000007).rw(m_lance, FUNC(am79c90_device::regs_r), FUNC(am79c90_device::regs_w)).umask32(0x0000ffff);
	map(0x19000000, 0x1901ffff).lrw16(
		[this](offs_t offset) { return m_nram[offset]; }, "nram_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_nram[offset]); }, "nram_w").umask32(0xffff);
	//map(0x1a000000, 0x1a000057).m(m_sii, FUNC(dc7061_device::map)).umask32(0xffff);
	map(0x1b000000, 0x1b03ffff).lrw16(
		[this](offs_t offset) { return m_dram[offset]; }, "dram_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_dram[offset]); }, "dram_w").umask32(0xffff);
	map(0x1c000000, 0x1c00001b).m(m_dz, FUNC(dc7085_device::map)).umask32(0xffff);
	map(0x1c000018, 0x1c000019).lr16([this]() { return m_msr; }, "msr_r");
	map(0x1d000000, 0x1d0000ff).rw(m_rtc, FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct)).umask32(0x000000ff);
	map(0x1d000000, 0x1d00007f).lr8([this](offs_t offset) { return m_esar[offset]; }, "esar_r").umask32(0xff00);
	map(0x1e000000, 0x1e000001).rw(FUNC(kn01_state::status_r), FUNC(kn01_state::control_w));
	map(0x1fc00000, 0x1fc3ffff).rom().region("eprom", 0);
}

void kn02ba_state::map(address_map &map)
{
	map(0x00000000, 0x07ffffff).ram();  // full 128 MB
	map(0x10000000, 0x1007ffff).rw(FUNC(kn02ba_state::cfb_r), FUNC(kn02ba_state::cfb_w));
	map(0x10100000, 0x101001ff).rw(m_sfb, FUNC(decsfb_device::read), FUNC(decsfb_device::write));
	map(0x101c0000, 0x101c000f).m("bt459", FUNC(bt459_device::map)).umask32(0x000000ff);
	map(0x10200000, 0x103fffff).rw(m_sfb, FUNC(decsfb_device::vram_r), FUNC(decsfb_device::vram_w));
	map(0x1c000000, 0x1c07ffff).m(m_ioga, FUNC(dec_ioga_device::map));
	map(0x1c0c0000, 0x1c0c0007).rw(m_lance, FUNC(am79c90_device::regs_r), FUNC(am79c90_device::regs_w)).umask32(0x0000ffff);
	map(0x1c100000, 0x1c100003).rw(m_scc[0], FUNC(z80scc_device::ca_r), FUNC(z80scc_device::ca_w)).umask32(0x0000ff00);
	map(0x1c100004, 0x1c100007).rw(m_scc[0], FUNC(z80scc_device::da_r), FUNC(z80scc_device::da_w)).umask32(0x0000ff00);
	map(0x1c100008, 0x1c10000b).rw(m_scc[0], FUNC(z80scc_device::cb_r), FUNC(z80scc_device::cb_w)).umask32(0x0000ff00);
	map(0x1c10000c, 0x1c10000f).rw(m_scc[0], FUNC(z80scc_device::db_r), FUNC(z80scc_device::db_w)).umask32(0x0000ff00);
	map(0x1c180000, 0x1c180003).rw(m_scc[1], FUNC(z80scc_device::ca_r), FUNC(z80scc_device::ca_w)).umask32(0x0000ff00);
	map(0x1c180004, 0x1c180007).rw(m_scc[1], FUNC(z80scc_device::da_r), FUNC(z80scc_device::da_w)).umask32(0x0000ff00);
	map(0x1c180008, 0x1c18000b).rw(m_scc[1], FUNC(z80scc_device::cb_r), FUNC(z80scc_device::cb_w)).umask32(0x0000ff00);
	map(0x1c18000c, 0x1c18000f).rw(m_scc[1], FUNC(z80scc_device::db_r), FUNC(z80scc_device::db_w)).umask32(0x0000ff00);
	map(0x1c200000, 0x1c2000ff).rw(m_rtc, FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct)).umask32(0x000000ff);
	map(0x1c300000, 0x1c30003f).m(m_asc, FUNC(ncr53c94_device::map)).umask32(0x000000ff);
	map(0x1fc00000, 0x1fc3ffff).rom().region("user1", 0);
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static void dec_scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
}

void kn01_state::kn01(machine_config &config, u32 clock)
{
	R2000(config, m_cpu, clock, 65536, 131072);
	m_cpu->set_endianness(ENDIANNESS_LITTLE);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010Av4, INPUT_LINE_IRQ5);
	m_cpu->in_brcond<0>().set_constant(1);
	m_cpu->set_addrmap(AS_PROGRAM, &kn01_state::map);

	RAM(config, m_mram);
	m_mram->set_default_size("24MiB");
	m_mram->set_extra_options("4MiB,8MiB,12MiB,16MiB,20MiB");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(69169800, 1280, 212, 1024+212, 901, 34, 864+34);
	m_screen->set_screen_update(FUNC(kn01_state::screen_update));

	TIMER(config, m_scantimer, 0);
	m_scantimer->configure_scanline(FUNC(kn01_state::scanline_timer), "screen", 0, 1);

	BT478(config, m_vdac, 69169800);

	AM79C90(config, m_lance, XTAL(12'500'000));
	m_lance->intr_out().set_inputline(m_cpu, INPUT_LINE_IRQ1).invert();
	m_lance->dma_in().set([this](offs_t offset) { return m_nram[(offset & 0xffff) >> 1]; });
	m_lance->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_nram[(offset & 0xffff) >> 1]); });

	DS1287(config, m_rtc, XTAL(32'768));
	m_rtc->set_binary(true);
	m_rtc->irq().set_inputline(m_cpu, INPUT_LINE_IRQ3);

	/*
	 * 0: keyboard (rx/tx)
	 * 1: mouse (rx/tx)
	 * 2: modem (rx/tx, dtr/dsr)
	 * 3: printer/console (rx/tx)
	 */
	DC7085(config, m_dz, 15.2064_MHz_XTAL);
	m_dz->int_cb().set_inputline(m_cpu, INPUT_LINE_IRQ2);

	LK201(config, m_lk201, 0);
	m_dz->tx_cb<0>().set([this](int state) { if (!(m_status & TXDIS)) m_lk201->rx_w(state); });
	m_lk201->tx_handler().set(m_dz, FUNC(dc7085_device::rx_w<0>));

	// TODO: kn01 prom requires a "mouse terminator" (Tx to Rx loopback) when no mouse is connected
	m_dz->tx_cb<1>().set([this](int state) { if (!(m_status & TXDIS)) m_dz->rx_w<1>(state); });

	rs232_port_device &com(RS232_PORT(config, "com_port", default_rs232_devices, nullptr));
	m_dz->tx_cb<2>().set([this, &com](int state) { if (!(m_status & TXDIS)) com.write_txd(state); });
	m_dz->dtr_cb<2>().set(com, FUNC(rs232_port_device::write_dtr));
	com.rxd_handler().set(m_dz, FUNC(dc7085_device::rx_w<2>));
	com.dsr_handler().set(
		[this](int state)
		{
			if (state)
				m_msr |= MSR_DSR2;
			else
				m_msr &= ~MSR_DSR2;
		});

	rs232_port_device &prt(RS232_PORT(config, "prt_port", default_rs232_devices, nullptr));
	m_dz->tx_cb<3>().set([this, &prt](int state) { if (!(m_status & TXDIS)) prt.write_txd(state); });
	m_dz->dtr_cb<3>().set(prt, FUNC(rs232_port_device::write_dtr));
	prt.rxd_handler().set(m_dz, FUNC(dc7085_device::rx_w<3>));
	prt.dsr_handler().set(
		[this](int state)
		{
			if (state)
				m_msr |= MSR_DSR3;
			else
				m_msr &= ~MSR_DSR3;
		});

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", dec_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", dec_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", dec_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", dec_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", dec_scsi_devices, "cdrom");
	NSCSI_CONNECTOR(config, "scsi:5", dec_scsi_devices, nullptr);
#if 0
	NSCSI_CONNECTOR(config, "scsi:6").option_set("sii", DC7061).clock(20_MHz_XTAL).machine_config(
		[this](device_t *device)
		{
			dc7061_device &sii = downcast<dc7061_device &>(*device);

			sii.irq_handler().set_inputline(m_cpu, INPUT_LINE_IRQ0);
		});
#endif
	NSCSI_CONNECTOR(config, "scsi:7", dec_scsi_devices, nullptr);

	config.set_default_layout(layout_kn01);
}

void kn02ba_state::kn02ba(machine_config &config, u32 clock)
{
	R3000A(config, m_cpu, clock, 65536, 131072);
	m_cpu->set_endianness(ENDIANNESS_LITTLE);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010Av4);
	m_cpu->in_brcond<0>().set_constant(1);
	m_cpu->set_addrmap(AS_PROGRAM, &kn02ba_state::map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(130000000, 1704, 32, (1280+32), 1064, 3, (1024+3));
	m_screen->set_screen_update(FUNC(kn02ba_state::screen_update));

	DECSFB(config, m_sfb, 25'000'000);  // clock based on white paper which quotes "40ns" gate array cycle times
//  m_sfb->int_cb().set(FUNC(dec_ioga_device::slot0_irq_w));

	BT459(config, m_bt459, 83'020'800);

	AM79C90(config, m_lance, XTAL(12'500'000));
	m_lance->intr_out().set("ioga", FUNC(dec_ioga_device::lance_irq_w));
	m_lance->dma_in().set("ioga", FUNC(dec_ioga_device::lance_dma_r));
	m_lance->dma_out().set("ioga", FUNC(dec_ioga_device::lance_dma_w));

	DECSTATION_IOGA(config, m_ioga, XTAL(12'500'000));
	m_ioga->irq_out().set_inputline(m_cpu, INPUT_LINE_IRQ3);

	MC146818(config, m_rtc, XTAL(32'768));
	m_rtc->irq().set("ioga", FUNC(dec_ioga_device::rtc_irq_w));
	m_rtc->set_binary(true);

	SCC85C30(config, m_scc[0], XTAL(14'745'600)/2);
	m_scc[0]->out_int_callback().set("ioga", FUNC(dec_ioga_device::scc0_irq_w));
	m_scc[0]->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_scc[0]->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));

	SCC85C30(config, m_scc[1], XTAL(14'745'600)/2);
	m_scc[1]->out_int_callback().set("ioga", FUNC(dec_ioga_device::scc1_irq_w));
	m_scc[1]->out_txdb_callback().set(m_lk201, FUNC(lk201_device::rx_w));

	LK201(config, m_lk201, 0);
	m_lk201->tx_handler().set(m_scc[1], FUNC(z80scc_device::rxb_w));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc[0], FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc[0], FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc[0], FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc[0], FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc[0], FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc[0], FUNC(z80scc_device::ctsb_w));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", dec_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", dec_scsi_devices, "cdrom");
	NSCSI_CONNECTOR(config, "scsi:2", dec_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", dec_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", dec_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", dec_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", dec_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("asc", NCR53C94).clock(10_MHz_XTAL).machine_config(
		[this](device_t *device)
		{
			ncr53c94_device &asc = downcast<ncr53c94_device &>(*device);

			asc.irq_handler_cb().set_inputline(m_cpu, INPUT_LINE_IRQ0);
		});
}

static INPUT_PORTS_START(kn01)
	PORT_START("config")
	PORT_DIPNAME(0x0801, 0x0000, "Graphics Mode")
	PORT_DIPSETTING(     0x0000, "Color")
	PORT_DIPSETTING(     0x0800, "Monochrome")
	PORT_DIPSETTING(     0x0801, "None")

	PORT_DIPNAME(0x8000, 0x8000, "Manufacturing Mode")
	PORT_DIPSETTING(     0x8000, DEF_STR(Off))
	PORT_DIPSETTING(     0x0000, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START(kn02ba)
	PORT_START("UNUSED") // unused IN0
	PORT_BIT(0xffff, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( ds3100 )
	ROM_REGION32_LE( 0x40000, "eprom", 0 )
	ROM_LOAD( "kn01-aa.v7.01.img", 0x000000, 0x040000, CRC(e2478aa7) SHA1(e789387c52df3e0d83fde97cb48314627ea90b93) )

	// hand-crafted following the documentation and logic of the "t e" diagnostic test
	ROM_REGION(0x20, "esar", 0)
	ROM_LOAD("esar.bin", 0x00, 0x20, CRC(ff083e3b) SHA1(1714338d8747ec434e77b72e7bd81f77aacf27d2))
ROM_END

#define rom_ds2100 rom_ds3100

ROM_START( ds5k133 )
	ROM_REGION32_LE( 0x40000, "user1", 0 )
	// 5.7j                                                                                                                                                                                                                                 sx
	ROM_LOAD( "ds5000-133_005eb.bin", 0x000000, 0x040000, CRC(76a91d29) SHA1(140fcdb4fd2327daf764a35006d05fabfbee8da6) )

	ROM_REGION32_LE( 0x20000, "gfx", 0 )
	ROM_LOAD( "pmagb-ba-rom.img", 0x000000, 0x020000, CRC(91f40ab0) SHA1(a39ce6ed52697a513f0fb2300a1a6cf9e2eabe33) )
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT  COMPANY                          FULLNAME               FLAGS
COMP( 1989, ds2100,  0,      0,      pmin,    kn01,   kn01_state,   init, "Digital Equipment Corporation", "DECstation 2100",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1989, ds3100,  0,      0,      pmax,    kn01,   kn01_state,   init, "Digital Equipment Corporation", "DECstation 3100",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1992, ds5k133, 0,      0,      m133,    kn02ba, kn02ba_state, init, "Digital Equipment Corporation", "DECstation 5000/133", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
