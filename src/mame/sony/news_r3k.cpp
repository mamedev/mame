// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Sony NEWS R3000 systems.
 *
 * Sources:
 *   - https://github.com/robohack/ucb-csrg-bsd/blob/master/sys/news3400/
 *   - https://www.mmcc.it/resources/docs/NWS-3410_3460_ServiceManual_MMCC.PDF
 *   - https://www.mmcc.it/resources/docs/Sony_NEWS_NWS-3260_ROM_Monitor_User_Guide_r2.pdf
 *
 * TODO:
 *   - lcd controller
 *   - screen params
 *   - floppy density/eject
 *   - centronics port
 *   - sound
 *   - other models, including slots/cards
 */

#include "emu.h"

#include "cpu/mips/mips1.h"

// memory
#include "machine/ram.h"

// various hardware
#include "machine/timekpr.h"
#include "machine/z80scc.h"
#include "machine/am79c90.h"
#include "machine/upd765.h"
#include "dmac_0448.h"
#include "news_hid.h"
#include "machine/cxd1185.h"

// video
#include "screen.h"

// audio
#include "sound/spkrdev.h"
#include "speaker.h"

// busses and connectors
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"

#include "imagedev/floppy.h"

#define VERBOSE 0
#include "logmacro.h"


namespace {

class news_r3k_base_state : public driver_device
{
public:
	news_r3k_base_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_dma(*this, "dma")
		, m_rtc(*this, "rtc")
		, m_scc(*this, "scc")
		, m_net(*this, "net")
		, m_fdc(*this, "fdc")
		, m_hid(*this, "hid")
		, m_scsi(*this, "scsi:7:cxd1185")
		, m_serial(*this, "serial%u", 0U)
		, m_scsibus(*this, "scsi")
		, m_led(*this, "led%u", 0U)
	{
	}

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	void cpu_map(address_map &map);

	// machine config
	void common(machine_config &config);

public:
	void init_common();

protected:
	void inten_w(offs_t offset, u16 data, u16 mem_mask);
	u16 inten_r() { return m_inten; }
	u16 intst_r() { return m_intst; }
	void intclr_w(offs_t offset, u16 data, u16 mem_mask);

	enum irq_number : unsigned
	{
		EXT3  = 0,
		EXT1  = 1,
		SLOT3 = 2,
		SLOT1 = 3,
		DMA   = 4,
		LANCE = 5,
		SCC   = 6,
		BEEP  = 7,
		CBSY  = 8,
		CFLT  = 9,
		MOUSE = 10,
		KBD   = 11,
		TIMER = 12,
		BERR  = 13,
		ABORT = 14,
		PERR  = 15,
	};
	template <irq_number Number> void irq_w(int state);
	void int_check();

	u32 bus_error();
	void itimer_w(u8 data);
	void itimer(s32 param);
	u8 debug_r() { return m_debug; }
	void debug_w(u8 data);

	// devices
	required_device<r3000a_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<dmac_0448_device> m_dma;
	required_device<m48t02_device> m_rtc;
	required_device<z80scc_device> m_scc;
	required_device<am7990_device> m_net;
	required_device<upd72067_device> m_fdc;

	required_device<news_hid_hle_device> m_hid;
	required_device<cxd1185_device> m_scsi;

	required_device_array<rs232_port_device, 2> m_serial;
	required_device<nscsi_bus_device> m_scsibus;

	output_finder<4> m_led;

	std::unique_ptr<u16[]> m_net_ram;

	emu_timer *m_itimer = nullptr;

	u16 m_inten = 0;
	u16 m_intst = 0;
	u8 m_debug = 0;

	bool m_int_state[4]{};
};

class nws3260_state : public news_r3k_base_state
{
public:
	nws3260_state(machine_config const &mconfig, device_type type, char const *tag)
		: news_r3k_base_state(mconfig, type, tag)
		, m_lcd(*this, "lcd")
		, m_vram(*this, "vram")
	{
	}

	void nws3260(machine_config &config);

protected:
	virtual void machine_start() override;

	void nws3260_map(address_map &map);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	required_device<screen_device> m_lcd;
	required_shared_ptr<u32> m_vram;

	bool m_lcd_enable = false;
	bool m_lcd_dim = false;
};

class nws3410_state : public news_r3k_base_state
{
public:
	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

	nws3410_state(machine_config const &mconfig, device_type type, char const *tag)
		: news_r3k_base_state(mconfig, type, tag)
	{
	}

	void nws3410(machine_config &config);

protected:
	void nws3410_map(address_map &map);
};

void nws3260_state::machine_start()
{
	news_r3k_base_state::machine_start();

	save_item(NAME(m_lcd_enable));
	save_item(NAME(m_lcd_dim));
	m_lcd_enable = false;
	m_lcd_dim = false;
}

void news_r3k_base_state::machine_start()
{
	m_led.resolve();

	m_net_ram = std::make_unique<u16[]>(8192);
	save_pointer(NAME(m_net_ram), 8192);

	save_item(NAME(m_inten));
	save_item(NAME(m_intst));
	save_item(NAME(m_debug));
	save_item(NAME(m_int_state));

	m_itimer = timer_alloc(FUNC(news_r3k_base_state::itimer), this);

	for (bool &int_state : m_int_state)
		int_state = false;

	m_inten = 0;
	m_intst = 0;
}

void news_r3k_base_state::machine_reset()
{
}

void news_r3k_base_state::init_common()
{
	// map the configured ram
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());

	// HACK: hardwire the rate until fdc is better understood
	m_fdc->set_rate(500000);

	// HACK: signal floppy density?
	m_scsi->port_w(0x02);
}

void nws3260_state::nws3260_map(address_map &map)
{
	cpu_map(map);
	map(0x10000000, 0x101fffff).rom().region("krom", 0);
	map(0x10000000, 0x10000003).lw32([this] (u32 data) { m_lcd_enable = bool(data); }, "lcd_enable_w");
	map(0x10100000, 0x10100003).lw32([this] (u32 data) { m_lcd_dim = BIT(data, 0); }, "lcd_dim_w");
	map(0x10200000, 0x1021ffff).ram().share("vram").mirror(0xa0000000);
	map(0x1ff60000, 0x1ff6001b).lw8([this] (offs_t offset, u8 data) { LOG("crtc offset %x 0x%02x\n", offset, data); }, "lfbm_crtc_w"); // TODO: HD64646FS
}

void nws3410_state::nws3410_map(address_map &map)
{
	cpu_map(map);
}

void news_r3k_base_state::cpu_map(address_map &map)
{
	map.unmap_value_high();

	map(0x18000000, 0x18ffffff).r(FUNC(news_r3k_base_state::bus_error));

	map(0x1fc00000, 0x1fc1ffff).rom().region("eprom", 0);
	//map(0x1fc40004, 0x1fc40007).w().umask32(0xff); ??
	// 1fc40007 // power/reboot/PARK?
	map(0x1fc80000, 0x1fc80001).rw(FUNC(news_r3k_base_state::inten_r), FUNC(news_r3k_base_state::inten_w));
	map(0x1fc80002, 0x1fc80003).r(FUNC(news_r3k_base_state::intst_r));
	map(0x1fc80004, 0x1fc80005).w(FUNC(news_r3k_base_state::intclr_w));
	map(0x1fc80006, 0x1fc80006).w(FUNC(news_r3k_base_state::itimer_w));
	// 1fcc0000 // cstrobe?
	// 1fcc0002 // sccstatus0?
	map(0x1fcc0003, 0x1fcc0003).rw(FUNC(news_r3k_base_state::debug_r), FUNC(news_r3k_base_state::debug_w));
	// 1fcc0007 // sccvect?

	map(0x1fd00000, 0x1fd00007).m(m_hid, FUNC(news_hid_hle_device::map));
	map(0x1fd40000, 0x1fd40003).noprw(); // FIXME: ignore buzzer for now

	map(0x1fe00000, 0x1fe0000f).m(m_dma, FUNC(dmac_0448_device::map));
	map(0x1fe00100, 0x1fe0010f).m(m_scsi, FUNC(cxd1185_device::map));
	map(0x1fe00200, 0x1fe00203).m(m_fdc, FUNC(upd72067_device::map));
	map(0x1fe00300, 0x1fe00300).lr8([] () { return 0xff; }, "sound_r"); // HACK: disable sound
	//map(0x1fe00300, 0x1fe00307); // sound
	map(0x1fe40000, 0x1fe40003).portr("SW2");
	//map(0x1fe70000, 0x1fe9ffff).ram(); // ??
	map(0x1fe80000, 0x1fe800ff).rom().region("idrom", 0).mirror(0x0003ff00);
	map(0x1fec0000, 0x1fec0003).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));

	map(0x1ff40000, 0x1ff407ff).rw(m_rtc, FUNC(m48t02_device::read), FUNC(m48t02_device::write));

	map(0x1ff80000, 0x1ff80003).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));
	map(0x1ffc0000, 0x1ffc3fff).lrw16(
			[this] (offs_t offset) { return m_net_ram[offset]; }, "net_ram_r",
			[this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset]); }, "net_ram_w");
}

static INPUT_PORTS_START(nws3260)
	PORT_START("SW2")
	// TODO: other combinations of switches 1-3 may be valid
	PORT_DIPNAME(0x07000000, 0x02000000, "Display") PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(0x00000000, "Console")
	PORT_DIPSETTING(0x02000000, "LCD")
	PORT_DIPNAME(0x08000000, 0x00000000, "Boot Device") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x00000000, "Disk")
	PORT_DIPSETTING(0x08000000, "Network")
	PORT_DIPNAME(0x10000000, 0x00000000, "Automatic Boot") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x00000000, DEF_STR(Off))
	PORT_DIPSETTING(0x10000000, DEF_STR(On))
	PORT_DIPNAME(0x20000000, 0x00000000, "Diagnostic Mode") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(0x00000000, DEF_STR(Off))
	PORT_DIPSETTING(0x20000000, DEF_STR(On))
	// TODO: not completely clear what this switch does
	PORT_DIPNAME(0x40000000, 0x00000000, "RAM") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(0x00000000, "Enabled")
	PORT_DIPSETTING(0x40000000, "Disabled")
	PORT_DIPNAME(0x80000000, 0x00000000, "Console Baud") PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(0x00000000, "9600")
	PORT_DIPSETTING(0x80000000, "1200")

	PORT_START("SW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

static INPUT_PORTS_START(nws3410)
	PORT_START("SW2")
	PORT_DIPNAME(0x07000000, 0x02000000, "Console") PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(0x00000000, "Serial")
	PORT_DIPSETTING(0x02000000, "NWB-252/NWB-253") // NWB-252 (color) or NWB-253 (monochrome) NWS-3400 series bitmap board
	PORT_DIPSETTING(0x07000000, "NWB-514/NWB-251") // NWB-514 (monochrome) or NWB-251 (color) expansion framebuffer
	PORT_DIPNAME(0x08000000, 0x00000000, "Boot Device") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x00000000, "Disk")
	PORT_DIPSETTING(0x08000000, "Network")
	PORT_DIPNAME(0x10000000, 0x00000000, "Automatic Boot") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x00000000, DEF_STR(Off))
	PORT_DIPSETTING(0x10000000, DEF_STR(On))
	PORT_DIPNAME(0x20000000, 0x00000000, "Diagnostic Mode") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(0x00000000, DEF_STR(Off))
	PORT_DIPSETTING(0x20000000, DEF_STR(On))
	PORT_DIPNAME(0x40000000, 0x00000000, "RAM") PORT_DIPLOCATION("SW2:7") // Controls "No Memory Mode" for testing, forces monitor to use RTC RAM instead
	PORT_DIPSETTING(0x00000000, "Enabled")
	PORT_DIPSETTING(0x40000000, "Disabled")
	PORT_DIPNAME(0x80000000, 0x00000000, "Console Baud") PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(0x00000000, "9600")
	PORT_DIPSETTING(0x80000000, "1200")
INPUT_PORTS_END

u32 nws3260_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	if (!m_lcd_enable)
		return 0;

	rgb_t const black = rgb_t::black();
	rgb_t const white = m_lcd_dim ? rgb_t(191, 191, 191) : rgb_t::white();

	u32 const *pixel_pointer = m_vram;

	for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
	{
		for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 32)
		{
			u32 const pixel_data = *pixel_pointer++;

			for (unsigned i = 0; i < 32; i++)
				bitmap.pix(y, x + i) = BIT(pixel_data, 31 - i) ? black : white;
		}
	}

	return 0;
}

void news_r3k_base_state::inten_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_inten);

	int_check();
}

template <news_r3k_base_state::irq_number Number> void news_r3k_base_state::irq_w(int state)
{
	LOG("irq number %d state %d\n",  Number, state);

	if (state)
		m_intst |= 1U << Number;
	else
		m_intst &= ~(1U << Number);

	int_check();
}

void news_r3k_base_state::intclr_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_intst &= ~(data & mem_mask);

	int_check();
}

void news_r3k_base_state::int_check()
{
	// TODO: assume 44422222 11100000
	static int const int_line[] = { INPUT_LINE_IRQ0, INPUT_LINE_IRQ1, INPUT_LINE_IRQ2, INPUT_LINE_IRQ4 };
	static u16 const int_mask[] = { 0x001f, 0x00e0, 0x1f00, 0xe000 };

	for (unsigned i = 0; i < std::size(m_int_state); i++)
	{
		bool const int_state = m_intst & m_inten & int_mask[i];

		if (m_int_state[i] != int_state)
		{
			m_int_state[i] = int_state;
			m_cpu->set_input_line(int_line[i], int_state);
		}
	}
}

u32 news_r3k_base_state::bus_error()
{
	if (!machine().side_effects_disabled())
		irq_w<BERR>(ASSERT_LINE);

	return 0;
}

void news_r3k_base_state::itimer_w(u8 data)
{
	LOG("itimer_w 0x%02x (%s)\n", data, machine().describe_context());

	// TODO: assume 0xff stops the timer
	u8 const ticks = data + 1;

	m_itimer->adjust(attotime::from_ticks(ticks, 800), 0, attotime::from_ticks(ticks, 800));
}

void news_r3k_base_state::itimer(s32 param)
{
	irq_w<TIMER>(ASSERT_LINE);
}

void news_r3k_base_state::debug_w(u8 data)
{
	/*
	 * The low four bits of this register control the diagnostic LEDs labelled 1-4
	 * with bit 0 correspondig to LED #1, and a 0 value enabling the LED. A non-
	 * exhaustive list of diagnostic codes produced by the PROM follows:
	 *
	 *  4321  Stage
	 *  ...x  EPROM checksum
	 *  ..x.  NVRAM test (byte)
	 *  ..xx  NVRAM test (word)
	 *  .x..  NVRAM test (dword)
	 *  .x.x  read dip-switch SW2
	 *  .xx.  write test 0x1fe70000-1fe9ffff?
	 *  .xxx  address decode
	 *  x...  NVRAM test (dword)
	 *  x..x  RAM sizing
	 *  x.x.  inventory/boot
	 *
	 */
	LOG("debug_w 0x%02x (%s)\n", data, machine().describe_context());

	for (unsigned i = 0; i < 4; i++)
		if (BIT(data, i + 4))
			m_led[i] = BIT(data, i);

	m_debug = data;
}

static void news_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
}

void news_r3k_base_state::common(machine_config &config)
{
	DMAC_0448(config, m_dma, 0);
	m_dma->set_bus(m_cpu, 0);
	m_dma->out_int_cb().set(FUNC(news_r3k_base_state::irq_w<DMA>));
	m_dma->dma_r_cb<1>().set(m_fdc, FUNC(upd72067_device::dma_r));
	m_dma->dma_w_cb<1>().set(m_fdc, FUNC(upd72067_device::dma_w));
	// TODO: channel 2 audio
	// TODO: channel 3 video

	M48T02(config, m_rtc);

	SCC85C30(config, m_scc, 4.9152_MHz_XTAL);
	m_scc->out_int_callback().set(FUNC(news_r3k_base_state::irq_w<SCC>));

	// scc channel A
	RS232_PORT(config, m_serial[0], default_rs232_devices, nullptr);
	m_serial[0]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));
	m_serial[0]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	m_serial[0]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	m_scc->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_scc->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));

	// scc channel B
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_serial[1]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));
	m_serial[1]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	m_serial[1]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	m_scc->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));

	AM7990(config, m_net);
	m_net->intr_out().set(FUNC(news_r3k_base_state::irq_w<LANCE>)).invert();
	m_net->dma_in().set([this](offs_t offset) { return m_net_ram[offset >> 1]; });
	m_net->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset >> 1]); });

	UPD72067(config, m_fdc, 16_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(m_dma, FUNC(dmac_0448_device::irq<1>));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(dmac_0448_device::drq<1>));
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(false);

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus);
	// inquiry content for hard disk is "HITACHI DK312C          CS01"
	// HD CHDs will be treated as MO disks using the inquiry content "SONY    SMO-C501        1.00"
	// The CHS for converting a raw MO dump for 282MByte per side disks is 18678,1,31 per NEWS-OS 4's disktab file
	NSCSI_CONNECTOR(config, "scsi:0", news_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", news_scsi_devices, nullptr);

	// scsi host adapter
	NSCSI_CONNECTOR(config, "scsi:7").option_set("cxd1185", CXD1185).clock(16_MHz_XTAL).machine_config(
		[this] (device_t *device)
		{
			cxd1185_device &adapter = downcast<cxd1185_device &>(*device);

			adapter.irq_out_cb().set(m_dma, FUNC(dmac_0448_device::irq<0>));
			adapter.drq_out_cb().set(m_dma, FUNC(dmac_0448_device::drq<0>));
			adapter.port_out_cb().set(
				[this] (u8 data)
				{
					LOG("floppy %s\n", BIT(data, 0) ? "mount" : "eject");
				});

			subdevice<dmac_0448_device>(":dma")->dma_r_cb<0>().set(adapter, FUNC(cxd1185_device::dma_r));
			subdevice<dmac_0448_device>(":dma")->dma_w_cb<0>().set(adapter, FUNC(cxd1185_device::dma_w));
		});

	NEWS_HID_HLE(config, m_hid);
	m_hid->irq_out<news_hid_hle_device::KEYBOARD>().set(FUNC(news_r3k_base_state::irq_w<KBD>));
	m_hid->irq_out<news_hid_hle_device::MOUSE>().set(FUNC(news_r3k_base_state::irq_w<MOUSE>));

	SOFTWARE_LIST(config, "software_list").set_original("sony_news").set_filter("RISC,NWS3000");
}

void nws3260_state::nws3260(machine_config &config)
{
	R3000A(config, m_cpu, 20_MHz_XTAL, 32768, 32768);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010Av4);
	m_cpu->set_addrmap(AS_PROGRAM, &nws3260_state::nws3260_map);

	// 3 banks of 4x30-pin SIMMs with parity, first bank is soldered
	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	// TODO: confirm each bank supports 4x1M or 4x4M
	m_ram->set_extra_options("4M,8M,12M,20M,24M,32M,36M,48M");
	common(config);

	// Integrated LCD panel
	SCREEN(config, m_lcd, SCREEN_TYPE_LCD);
	m_lcd->set_raw(52416000, 1120, 0, 1120, 780, 0, 780);
	m_lcd->set_screen_update(FUNC(nws3260_state::screen_update));
}

void nws3410_state::nws3410(machine_config &config)
{
	R3000A(config, m_cpu, 20_MHz_XTAL, 65536, 65536);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010Av4);
	m_cpu->set_addrmap(AS_PROGRAM, &nws3410_state::nws3410_map);

	// Per the service manual, one or more NWA-029 4MB expansion kits can be used to increase from the base 8M up to 16M
	RAM(config, m_ram);
	m_ram->set_default_size("8M");
	m_ram->set_extra_options("12M,16M");
	common(config);

	m_serial[0]->set_default_option("terminal"); // No framebuffer emulation yet
}

ROM_START(nws3260)
	ROM_REGION32_BE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "nws3260", "NWS-3260 v2.0A")
	ROMX_LOAD("mpu-16__ver.2.0a__1990_sony.ic64", 0x00000, 0x20000, CRC(61222991) SHA1(076fab0ad0682cd7dacc7094e42efe8558cbaaa1), ROM_BIOS(0))

	// 2 x MB834200A-20 (4Mb mask ROM)
	ROM_REGION32_BE(0x200000, "krom", ROMREGION_ERASEFF)
	ROM_LOAD64_WORD("051_aa.ic109", 0x00000, 0x80000, CRC(1411cbcb) SHA1(793394cd3919034f85bfb015d6d3c504f83b6626))
	ROM_LOAD64_WORD("052_aa.ic110", 0x00004, 0x80000, CRC(df0f39da) SHA1(076881da022a3fe6731de0ead217285293c25dc7))

	/*
	 * This is probably a 4-bit device: only the low 4 bits in each location
	 * are used, and are merged together into bytes when copied into RAM, with
	 * the most-significant bits at the lower address. The sum of resulting
	 * big-endian 32-bit words must be zero.
	 *
	 * offset  purpose
	 *  0x00   magic number (0x0f 0x0f)
	 *  0x10   ethernet mac address (low 4 bits of 12 bytes)
	 *  0x28   machine identification (low 4 bits of 8 bytes)
	 *  0x60   model number (null-terminated string)
	 */
	ROM_REGION32_BE(0x100, "idrom", 0)
	ROM_LOAD("idrom.bin", 0x000, 0x100, CRC(17a3d9c6) SHA1(d300e6908210f540951211802c38ad7f8037aa15) BAD_DUMP)
ROM_END

ROM_START(nws3410)
	ROM_REGION32_BE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "nws3410", "NWS-3410 v2.0")
	ROMX_LOAD("sony_nws-3410_mpu-12_v2_rom.bin", 0x00000, 0x20000, CRC(48a726c4) SHA1(5c6e9e6bccaaa3d63bc136355a436c17c49c9876), ROM_BIOS(0))

	ROM_REGION32_BE(0x100, "idrom", 0)
	ROM_LOAD("idrom.bin", 0x000, 0x100, CRC(661e2516) SHA1(f0dca34174747321dad6f48c466e1c549b797d2e) BAD_DUMP)
ROM_END

} // anonymous namespace


/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT         COMPANY  FULLNAME    FLAGS */
COMP(1991, nws3260, 0,      0,      nws3260, nws3260, nws3260_state, init_common, "Sony",  "NWS-3260", MACHINE_NO_SOUND)
COMP(1991, nws3410, 0,      0,      nws3410, nws3410, nws3410_state, init_common, "Sony",  "NWS-3410", MACHINE_NO_SOUND)
