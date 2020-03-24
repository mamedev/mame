// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Sony NEWS R3000 systems.
 *
 * Sources:
 *
 * TODO
 *   - interrupts
 *   - scsi
 *   - dma
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
#include "machine/news_kbd.h"

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
		, m_rtc(*this, "rtc")
		, m_scc(*this, "scc")
		, m_net(*this, "net")
		, m_fdc(*this, "fdc")
		, m_lcd(*this, "lcd")
		, m_kbd(*this, "kbd")
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

	void kbd_irq(int state);
	void debug_w(u8 data);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	// devices
	required_device<r3000a_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<m48t02_device> m_rtc;
	required_device<z80scc_device> m_scc;
	required_device<am7990_device> m_net;
	required_device<upd72065_device> m_fdc;

	required_device<screen_device> m_lcd;
	required_device<news_hle_kbd_device> m_kbd;

	required_device_array<rs232_port_device, 2> m_serial;
	required_device<nscsi_bus_device> m_scsibus;

	required_shared_ptr<u32> m_vram;
	output_finder<4> m_led;

	std::unique_ptr<u16[]> m_net_ram;
	u8 m_kbd_status;
};

FLOPPY_FORMATS_MEMBER(news_r3k_state::floppy_formats)
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

void news_r3k_state::machine_start()
{
	m_led.resolve();

	m_net_ram = std::make_unique<u16[]>(8192);

	m_kbd_status = 0;
}

void news_r3k_state::machine_reset()
{
}

void news_r3k_state::init_common()
{
	// map the configured ram
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());
}

void news_r3k_state::cpu_map(address_map &map)
{
	// FIXME: silence a lot of unhandled graphics addresses
	map(0x187702b0, 0x187702b3).nopw().umask32(0xffff);
	map(0x187c0000, 0x187c0003).nopw(); // palette?
	map(0x187e0000, 0x187e000f).nopw(); // lcdc?
	map(0x18f702b0, 0x18f702b3).nopw().umask32(0xffff);
	map(0x18fc0000, 0x18fc0003).nopw(); // palette?
	map(0x18fe0000, 0x18fe0003).nopw(); // lcdc?

	map(0x08000000, 0x080fffff).ram().share("vram");

	map(0x1fc00000, 0x1fc1ffff).rom().region("eprom", 0);

	//map(0x1fc40004, 0x1fc40004).w().umask32(0xff); ??
	//map(0x1fc80000, 0x1fc80001); // pinten
	//map(0x1fc80002, 0x1fc80003); // pintstat
	//map(0x1fc80004, 0x1fc80005); // intclr
	//map(0x1fc80006, 0x1fc80006); // itimer
	map(0x1fcc0003, 0x1fcc0003).w(FUNC(news_r3k_state::debug_w));

	map(0x1fd00000, 0x1fd00000).r(m_kbd, FUNC(news_hle_kbd_device::data_r));
	map(0x1fd00001, 0x1fd00001).lr8([this]() { return m_kbd_status; }, "kbd_status_r");
	map(0x1fd00002, 0x1fd00002).lw8([this](u8 data) { m_kbd->reset(); }, "kbd_reset_w");
	map(0x1fd40000, 0x1fd40003).noprw().umask32(0xffff); // FIXME: ignore buzzer for now

	//map(0x1fe00000, 0x1fe0000f); // dmac 0448
	//map(0x1fe00100, 0x1fe00100); // scsi
	map(0x1fe00200, 0x1fe00203).m(m_fdc, FUNC(upd72069_device::map)).umask32(0xffff);
	//map(0x1fe00300, 0x1fe00307); // sound

	map(0x1fe40000, 0x1fe40003).portr("SW2");
	map(0x1fe80000, 0x1fe800ff).rom().region("idrom", 0);

	map(0x1fec0000, 0x1fec0003).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));

	map(0x1ff40000, 0x1ff407ff).rw(m_rtc, FUNC(m48t02_device::read), FUNC(m48t02_device::write));
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

void news_r3k_state::kbd_irq(int state)
{
	// TODO: relay irq to cpu
	if (state)
		m_kbd_status |= 2;
	else
		m_kbd_status &= ~2;
}

void news_r3k_state::debug_w(u8 data)
{
	LOG("debug_w 0x%02x (%s)\n", data, machine().describe_context());

	for (unsigned i = 0; i < 4; i++)
		if (BIT(data, i + 4))
			m_led[i] = BIT(data, i);
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

	M48T02(config, m_rtc);

	SCC85C30(config, m_scc, 4.9152_MHz_XTAL);
	//m_scc->out_int_callback().set(FUNC(rx3230_state::irq_w<INT_SCC>)).invert();

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
	//m_net->intr_out().set(FUNC(news_r3k_state::irq_w<INT_NET>));
	m_net->dma_in().set([this](offs_t offset) { return m_net_ram[offset >> 1]; });
	m_net->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset >> 1]); });

	// μPD72067, clock?
	UPD72069(config, m_fdc, 16_MHz_XTAL);
	//m_fdc->intrq_wr_callback().set_inputline(m_cpu, INPUT_LINE_IRQ4);
	//m_fdc->drq_wr_callback().set();
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_formats).enable_sound(false);

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", news_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", news_scsi_devices, "cdrom");

	/*
	 * FIXME: the screen is supposed to be an 1120x780 monochrome (black/white)
	 * LCD, with an HD64646FS LCD controller. The boot prom is happy if we just
	 * ignore the LCDC and pretend the screen is 1024 pixels wide.
	 */
	SCREEN(config, m_lcd, SCREEN_TYPE_LCD);
	m_lcd->set_raw(47923200, 1024, 0, 1023, 780, 0, 779);
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
	ROMX_LOAD("nws3260.bin", 0x00000, 0x20000, CRC(61222991) SHA1(076fab0ad0682cd7dacc7094e42efe8558cbaaa1), ROM_BIOS(0))

	/*
	 * This is probably a 4-bit device: only the low 4 bits in each location
	 * are used, and are merged together into bytes when copied into RAM, with
	 * the most-significant bits at the lower address. The sum of resulting
	 * bytes must be zero.
	 *
	 * offset  purpose
	 *  0x00   magic number (0x0f 0x0f)
	 *  0x10   ethernet mac address (low 4 bits of 12 bytes)
	 *  0x28   machine identification (low 4 bits of 8 bytes)
	 *  0x60   model number (null-terminated string)
	 */
	ROM_REGION32_BE(0x100, "idrom", 0)
	ROM_LOAD("idrom.bin", 0x000, 0x100, CRC(b257474c) SHA1(25908483fe72edd0ff51f58c16f7dbb8e72822e7))
ROM_END

/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS           INIT         COMPANY  FULLNAME    FLAGS */
COMP(1991, nws3260, 0,      0,      nws3260, nws3260, news_r3k_state, init_common, "Sony",  "NWS-3260", MACHINE_IS_SKELETON)
