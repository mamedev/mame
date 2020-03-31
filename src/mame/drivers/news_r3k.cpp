// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Sony NEWS R3000 systems.
 *
 * Sources:
 *
 * TODO
 *   - sound
 *   - mouse
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
#include "machine/dmac_0448.h"
#include "machine/news_kbd.h"
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
#include "formats/pc_dsk.h"

#include "debugger.h"

#define VERBOSE 0
#include "logmacro.h"

class news_r3k_state : public driver_device
{
public:
	news_r3k_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_dma(*this, "dma")
		, m_rtc(*this, "rtc")
		, m_scc(*this, "scc")
		, m_net(*this, "net")
		, m_fdc(*this, "fdc")
		, m_lcd(*this, "lcd")
		, m_kbd(*this, "kbd")
		, m_scsi(*this, "scsi:7:cxd1185")
		, m_serial(*this, "serial%u", 0U)
		, m_scsibus(*this, "scsi")
		, m_vram(*this, "vram")
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
	void nws3260(machine_config &config);

	void init_common();

protected:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

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
	void itimer(void *ptr, s32 param);
	void kbd_irq(int state);
	u8 debug_r() { return m_debug; }
	void debug_w(u8 data);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	// devices
	required_device<r3000a_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<dmac_0448_device> m_dma;
	required_device<m48t02_device> m_rtc;
	required_device<z80scc_device> m_scc;
	required_device<am7990_device> m_net;
	required_device<upd72067_device> m_fdc;

	required_device<screen_device> m_lcd;
	required_device<news_hle_kbd_device> m_kbd;
	required_device<cxd1185_device> m_scsi;

	required_device_array<rs232_port_device, 2> m_serial;
	required_device<nscsi_bus_device> m_scsibus;

	required_shared_ptr<u32> m_vram;
	output_finder<4> m_led;

	std::unique_ptr<u16[]> m_net_ram;

	emu_timer *m_itimer;

	u16 m_inten;
	u16 m_intst;
	u8 m_kbd_status;
	u8 m_debug;

	static unsigned const NUM_INT = 4;
	bool m_int_state[NUM_INT];
};

FLOPPY_FORMATS_MEMBER(news_r3k_state::floppy_formats)
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

void news_r3k_state::machine_start()
{
	m_led.resolve();

	m_net_ram = std::make_unique<u16[]>(8192);

	m_itimer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(news_r3k_state::itimer), this));

	for (bool &int_state : m_int_state)
		int_state = false;

	m_kbd_status = 0;
}

void news_r3k_state::machine_reset()
{
}

void news_r3k_state::init_common()
{
	// map the configured ram
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());

	// HACK: hardwire the rate until fdc is better understood
	m_fdc->set_rate(500000);

	// HACK: signal floppy density?
	m_scsi->port_w(0x02);
}

void news_r3k_state::cpu_map(address_map &map)
{
	map(0x08000000, 0x080fffff).ram().share("vram");

	// FIXME: silence a lot of unhandled graphics addresses
	map(0x187702b0, 0x187702b3).nopw().umask32(0xffff);
	map(0x187c0000, 0x187c0003).nopw(); // palette?
	map(0x187e0000, 0x187e000f).nopw(); // lcdc?
	map(0x18f702b0, 0x18f702b3).nopw().umask32(0xffff);
	map(0x18fc0000, 0x18fc0003).nopw(); // palette?
	map(0x18fe0000, 0x18fe0003).nopw(); // lcdc?

	// respond to absent hardware probes
	// 0x18500000 lcdc?
	map(0x18600000, 0x186fffff).r(FUNC(news_r3k_state::bus_error)); // ??
	map(0x18780000, 0x18780003).r(FUNC(news_r3k_state::bus_error)); // nwb-225
	map(0x18c30000, 0x18c30003).r(FUNC(news_r3k_state::bus_error)); // second lance
	map(0x18c40000, 0x18c40003).r(FUNC(news_r3k_state::bus_error)); // second scc
	map(0x18c40004, 0x18c40007).r(FUNC(news_r3k_state::bus_error)); // third scc
	map(0x18c70000, 0x18c70003).r(FUNC(news_r3k_state::bus_error)); // third lance
	map(0x18e00000, 0x18e00003).r(FUNC(news_r3k_state::bus_error)); // nwb-252/nwb-253 crt
	map(0x18ff0000, 0x18ff0003).r(FUNC(news_r3k_state::bus_error)); // nwb-252/nwb-253 ctrl

	map(0x1fc00000, 0x1fc1ffff).rom().region("eprom", 0);
	//map(0x1fc40004, 0x1fc40004).w().umask32(0xff); ??
	map(0x1fc80000, 0x1fc80001).rw(FUNC(news_r3k_state::inten_r), FUNC(news_r3k_state::inten_w));
	map(0x1fc80002, 0x1fc80003).r(FUNC(news_r3k_state::intst_r));
	map(0x1fc80004, 0x1fc80005).w(FUNC(news_r3k_state::intclr_w));
	map(0x1fc80006, 0x1fc80006).w(FUNC(news_r3k_state::itimer_w));
	map(0x1fcc0003, 0x1fcc0003).rw(FUNC(news_r3k_state::debug_r), FUNC(news_r3k_state::debug_w));

	map(0x1fd00000, 0x1fd00000).r(m_kbd, FUNC(news_hle_kbd_device::data_r));
	map(0x1fd00001, 0x1fd00001).lr8([this]() { return m_kbd_status; }, "kbd_status_r");
	map(0x1fd00002, 0x1fd00002).lw8([this](u8 data) { m_kbd->reset(); }, "kbd_reset_w");
	map(0x1fd40000, 0x1fd40003).noprw().umask32(0xffff); // FIXME: ignore buzzer for now

	map(0x1fe00000, 0x1fe0000f).m(m_dma, FUNC(dmac_0448_device::map));
	map(0x1fe00100, 0x1fe0010f).m(m_scsi, FUNC(cxd1185_device::map));
	map(0x1fe00200, 0x1fe00203).m(m_fdc, FUNC(upd72067_device::map));
	map(0x1fe00300, 0x1fe00300).lr8([this]() { return 0xff; }, "sound_r"); // HACK: disable sound
	//map(0x1fe00300, 0x1fe00307); // sound
	map(0x1fe40000, 0x1fe40003).portr("SW2");
	map(0x1fe80000, 0x1fe800ff).rom().region("idrom", 0).mirror(0x0003ff00);
	map(0x1fec0000, 0x1fec0003).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));

	map(0x1ff40000, 0x1ff407ff).rw(m_rtc, FUNC(m48t02_device::read), FUNC(m48t02_device::write));
	map(0x1ff50000, 0x1ff50003).r(FUNC(news_r3k_state::bus_error)); // lfbm?
	map(0x1ff80000, 0x1ff80003).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));
	map(0x1ffc0000, 0x1ffc3fff).lrw16(
		[this](offs_t offset) { return m_net_ram[offset]; }, "net_ram_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset]); }, "net_ram_w");
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

u32 news_r3k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	u32 *pixel_pointer = m_vram;

	for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
	{
		for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 4)
		{
			u32 const pixel_data = *pixel_pointer++;

			bitmap.pix(y, x + 0) = u8(pixel_data >> 24) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x + 1) = u8(pixel_data >> 16) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x + 2) = u8(pixel_data >> 8) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x + 3) = u8(pixel_data >> 0) ? rgb_t::white() : rgb_t::black();
		}
	}

	return 0;
}

void news_r3k_state::inten_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_inten);

	int_check();
}

template <news_r3k_state::irq_number Number> void news_r3k_state::irq_w(int state)
{
	LOG("irq number %d state %d\n",  Number, state);

	if (state)
		m_intst |= 1U << Number;
	else
		m_intst &= ~(1U << Number);

	int_check();
}

void news_r3k_state::intclr_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_intst &= ~(data & mem_mask);

	int_check();
}

void news_r3k_state::int_check()
{
	// TODO: assume 44422222 11100000
	static int const int_line[] = { INPUT_LINE_IRQ0, INPUT_LINE_IRQ1, INPUT_LINE_IRQ2, INPUT_LINE_IRQ4 };
	static u16 const int_mask[] = { 0x001f, 0x00e0, 0x1f00, 0xe000 };

	for (unsigned i = 0; i < NUM_INT; i++)
	{
		bool const int_state = m_intst & m_inten & int_mask[i];

		if (m_int_state[i] != int_state)
		{
			m_int_state[i] = int_state;
			m_cpu->set_input_line(int_line[i], int_state);
		}
	}
}

u32 news_r3k_state::bus_error()
{
	irq_w<BERR>(ASSERT_LINE);

	return 0;
}

void news_r3k_state::itimer_w(u8 data)
{
	LOG("itimer_w 0x%02x (%s)\n", data, machine().describe_context());

	// TODO: assume 0xff stops the timer
	u8 const ticks = data + 1;

	m_itimer->adjust(attotime::from_ticks(ticks, 800), 0, attotime::from_ticks(ticks, 800));
}

void news_r3k_state::itimer(void *ptr, s32 param)
{
	irq_w<TIMER>(ASSERT_LINE);
}

void news_r3k_state::kbd_irq(int state)
{
	if (state)
		m_kbd_status |= 2;
	else
		m_kbd_status &= ~2;

	irq_w<KBD>(state);
}

void news_r3k_state::debug_w(u8 data)
{
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

void news_r3k_state::common(machine_config &config)
{
	R3000A(config, m_cpu, 20_MHz_XTAL, 32768, 32768);
	m_cpu->set_addrmap(AS_PROGRAM, &news_r3k_state::cpu_map);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010Av4);

	// 12 SIMM slots
	// 30pin 4Mbyte SIMMs with parity?
	RAM(config, m_ram);
	m_ram->set_default_size("16M");

	DMAC_0448(config, m_dma, 0);
	m_dma->set_bus(m_cpu, 0);
	m_dma->out_int_cb().set(FUNC(news_r3k_state::irq_w<DMA>));
	// TODO: channel 0 scsi
	m_dma->dma_r_cb<1>().set(m_fdc, FUNC(upd72067_device::dma_r));
	m_dma->dma_w_cb<1>().set(m_fdc, FUNC(upd72067_device::dma_w));
	// TODO: channel 2 audio
	// TODO: channel 3 video

	M48T02(config, m_rtc);

	SCC85C30(config, m_scc, 4.9152_MHz_XTAL);
	m_scc->out_int_callback().set(FUNC(news_r3k_state::irq_w<SCC>));

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
	m_net->intr_out().set(FUNC(news_r3k_state::irq_w<LANCE>));
	m_net->dma_in().set([this](offs_t offset) { return m_net_ram[offset >> 1]; });
	m_net->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset >> 1]); });

	// μPD72067, clock?
	UPD72067(config, m_fdc, 16_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(m_dma, FUNC(dmac_0448_device::irq<1>));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(dmac_0448_device::drq<1>));
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_formats).enable_sound(false);

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", news_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", news_scsi_devices, nullptr);

	// scsi host adapter
	NSCSI_CONNECTOR(config, "scsi:7").option_set("cxd1185", CXD1185).clock(16_MHz_XTAL).machine_config(
		[this](device_t *device)
		{
			cxd1185_device &adapter = downcast<cxd1185_device &>(*device);

			adapter.irq_out_cb().set(m_dma, FUNC(dmac_0448_device::irq<0>));
			adapter.drq_out_cb().set(m_dma, FUNC(dmac_0448_device::drq<0>));
			adapter.port_out_cb().set(
				[this](u8 data)
				{
					LOG("floppy %s\n", BIT(data, 0) ? "mount" : "eject");
				});

			subdevice<dmac_0448_device>(":dma")->dma_r_cb<0>().set(adapter, FUNC(cxd1185_device::dma_r));
			subdevice<dmac_0448_device>(":dma")->dma_w_cb<0>().set(adapter, FUNC(cxd1185_device::dma_w));
		});

	/*
	 * FIXME: the screen is supposed to be an 1120x780 monochrome (black/white)
	 * LCD, with an HD64646FS LCD controller. The boot prom is happy if we just
	 * ignore the LCDC and pretend the screen is 1024 pixels wide.
	 */
	// apparently 1024x768?
	SCREEN(config, m_lcd, SCREEN_TYPE_LCD);
	m_lcd->set_raw(47185920, 1024, 0, 1024, 768, 0, 768);
	m_lcd->set_screen_update(FUNC(news_r3k_state::screen_update));

	NEWS_HLE_KBD(config, m_kbd);
	m_kbd->irq_out().set(*this, FUNC(news_r3k_state::kbd_irq));
}

void news_r3k_state::nws3260(machine_config &config)
{
	common(config);
}

ROM_START(nws3260)
	ROM_REGION32_BE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "nws3260", "NWS-3260 v2.0A")
	ROMX_LOAD("mpu-16__ver.2.0a__1990_sony.ic64", 0x00000, 0x20000, CRC(61222991) SHA1(076fab0ad0682cd7dacc7094e42efe8558cbaaa1), ROM_BIOS(0))

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

/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS           INIT         COMPANY  FULLNAME    FLAGS */
COMP(1991, nws3260, 0,      0,      nws3260, nws3260, news_r3k_state, init_common, "Sony",  "NWS-3260", MACHINE_IS_SKELETON)
