// license:BSD-3-Clause
// copyright-holders:R. Belmont

/*
 * Digital Equipment Corporation DECstation 2100/3100.
 *
 * Model  Board  Codename  CPU               RAM
 * 2100   KN01   PMIN      R2000 @ 12.5MHz   24M
 * 3100   KN01   PMAX      R2000 @ 16.67MHz  24M
 *
 * Sources:
 *  - DECstation 3100 Desktop Workstation Functional Specification, Revision 1.3, August 28, 1990, Workstation Systems Engineering, Digital Equipment Corporation
 *
 * TODO:
 *  - scsi
 *  - devicify pcc
 *  - mouse
 */

/*
 * ds3100 wip
 * - http://www.vanade.com/~blc/DS3100/bootrom.html
 * - http://www.vanade.com/~blc/DS3100/3100test.html
 *
 * - press Ctrl+C at flashing >> prompt to enter monitor
 * - boot -f rz(0,4,0)vmunix to boot ULTRIX from CD-ROM at SCSI ID=4
 *     ?0bd-36 rst err csr a exp 00000000 00000010
 *     ?446 scsi rst
 *     ?470 bt err: rz(0,4,0)vmunix
 *
 * diagnostic test failures
 *  D  passes with rs232 loopback on com_port and prt_port
 *  k  keyboard
 *  P  mouse not emulated
 *  r  passes if seconds increment within 300,000 iterations of loop
 *  s  sii not emulated
 */

#include "emu.h"

#include "dc7061.h"
#include "dc7085.h"
#include "lk201.h"

#include "cpu/mips/mips1.h"

#include "machine/am79c90.h"
#include "machine/mc146818.h"
#include "machine/nscsi_bus.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "video/bt47x.h"

#include "bus/rs232/rs232.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"

#include "screen.h"

#include "endianness.h"

#include "kn01.lh"

#define VERBOSE (0)

#include "logmacro.h"

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
		, m_sii(*this, "scsi:6:sii")
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

	void pmax(machine_config &config) { kn01(config, 33.33_MHz_XTAL / 2); }
	void pmin(machine_config &config) { kn01(config, 25_MHz_XTAL / 2); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void kn01(machine_config &config, XTAL clock);

	void map(address_map &map) ATTR_COLD;

	u16 status_r();
	void control_w(u16 data);

	u16 pcc_r(offs_t offset);
	void pcc_w(offs_t offset, u16 data);

	void vram_w(offs_t offset, u32 data, u32 mem_mask);
	void plane_mask_w(u8 data);

	void memerr_w(offs_t offset, u32 data, u32 mem_mask);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

private:
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

	enum msr_mask : u16
	{
		MSR_DSR3 = 0x0001,
		MSR_DSR2 = 0x0200,
	};

	required_device<mips1_device_base> m_cpu;
	required_device<ram_device> m_mram;
	required_region_ptr<u8> m_esar;

	required_device<mc146818_device> m_rtc;
	required_device<dc7085_device> m_dz;
	required_device<dc7061_device> m_sii;
	required_device<am79c90_device> m_lance;

	optional_device<lk201_device> m_lk201;

	optional_device<screen_device> m_screen;
	optional_device<bt478_device> m_vdac;
	optional_device<timer_device> m_scantimer;
	optional_shared_ptr<u32> m_vram;

	required_ioport m_config;
	output_finder<8> m_leds;

	u16 m_status;

	u32 m_plane_mask;

	u16 m_pcc_regs[16];

	std::unique_ptr<u16[]> m_dram; // disk ram
	std::unique_ptr<u16[]> m_nram; // network ram

	u16 m_msr;
	u32 m_weaddr; // write error address
};

uint32_t kn01_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	u32 const *pixel_pointer = m_vram;

	if (m_status & MONO)
	{
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
	}
	else
	{
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
	}

	return 0;
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
		auto const vram = util::little_endian_cast<u8 const>(m_vram.target());

		int const x = m_pcc_regs[PCC_XMIN1] - 212;
		int const y = m_pcc_regs[PCC_YMIN1] - 34;
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

void kn01_state::memerr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("memerr_w 0x%08x mask 0x%08x\n", offset << 2, mem_mask);
	m_status |= MEMERR;
	switch (mem_mask)
	{
	case 0x0000'ff00:
		m_weaddr = (offset << 2) | 1;
		break;
	case 0x00ff'0000:
		m_weaddr = (offset << 2) | 2;
		break;
	case 0xff00'0000:
		m_weaddr = (offset << 2) | 3;
		break;
	case 0xffff'0000:
		m_weaddr = (offset << 2) | 2;
		break;
	default:
		m_weaddr = offset << 2;
		break;
	}

	m_cpu->set_input_line(INPUT_LINE_IRQ4, ASSERT_LINE);
}

void kn01_state::machine_start()
{
	m_dram = std::make_unique<u16[]>(65536);
	m_nram = std::make_unique<u16[]>(32768);

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

u16 kn01_state::status_r()
{
	return m_status;
}

void kn01_state::control_w(u16 data)
{
	// update LEDs
	for (unsigned i = 0; i < 8; i++)
		m_leds[i] = BIT(data, i);

	if (data & VINT)
	{
		m_cpu->set_input_line(INPUT_LINE_IRQ4, CLEAR_LINE);
		m_status &= ~VINT;
	}

	if (data & MEMERR)
	{
		m_cpu->set_input_line(INPUT_LINE_IRQ4, CLEAR_LINE);
		m_status &= ~MEMERR;
	}

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

void kn01_state::map(address_map &map)
{
	map(0x00000000, 0x1fffffff).w(FUNC(kn01_state::memerr_w));

	map(0x0fc00000, 0x0fcfffff).ram().share("vram").w(FUNC(kn01_state::vram_w));

	map(0x10000000, 0x10000000).w(FUNC(kn01_state::plane_mask_w));
	map(0x11000000, 0x1100003f).rw(FUNC(kn01_state::pcc_r), FUNC(kn01_state::pcc_w)).umask32(0x0000ffff);
	map(0x12000000, 0x1200001f).m(m_vdac, FUNC(bt478_device::map)).umask32(0x000000ff).mirror(0xe0);
	map(0x17000000, 0x17000003).lr32(NAME([this]() { return m_weaddr; }));
	map(0x18000000, 0x18000007).rw(m_lance, FUNC(am79c90_device::regs_r), FUNC(am79c90_device::regs_w)).umask32(0x0000ffff);
	map(0x19000000, 0x1901ffff).lrw16(
		[this](offs_t offset) { return m_nram[offset]; }, "nram_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_nram[offset]); }, "nram_w").umask32(0xffff);
	map(0x1a000000, 0x1a000057).m(m_sii, FUNC(dc7061_device::map)).umask32(0xffff);
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

static void dec_scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
}

void kn01_state::kn01(machine_config &config, XTAL clock)
{
	R2000(config, m_cpu, clock, 65536, 65536);
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
	NSCSI_CONNECTOR(config, "scsi:6").option_set("sii", DC7061).clock(20_MHz_XTAL).machine_config(
		[this](device_t *device)
		{
			dc7061_device &sii = downcast<dc7061_device &>(*device);

			sii.sys_int().set_inputline(m_cpu, INPUT_LINE_IRQ0);
		});
	NSCSI_CONNECTOR(config, "scsi:7", dec_scsi_devices, nullptr);

	config.set_default_layout(layout_kn01);
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

ROM_START( ds3100 )
	ROM_REGION32_LE( 0x40000, "eprom", 0 )
	ROM_LOAD( "kn01-aa.v7.01.img", 0x000000, 0x040000, CRC(e2478aa7) SHA1(e789387c52df3e0d83fde97cb48314627ea90b93) )

	// hand-crafted following the documentation and logic of the "t e" diagnostic test
	ROM_REGION(0x20, "esar", 0)
	ROM_LOAD("esar.bin", 0x00, 0x20, CRC(ff083e3b) SHA1(1714338d8747ec434e77b72e7bd81f77aacf27d2))
ROM_END

#define rom_ds2100 rom_ds3100

} // anonymous namespace

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                          FULLNAME           FLAGS
COMP( 1989, ds2100, 0,      0,      pmin,    kn01,  kn01_state, empty_init, "Digital Equipment Corporation", "DECstation 2100", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1989, ds3100, 0,      0,      pmax,    kn01,  kn01_state, empty_init, "Digital Equipment Corporation", "DECstation 3100", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
