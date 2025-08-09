// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Sony NEWS NWS-38xx.
 *
 * Sources:
 *
 * TODO:
 *   - r3000 cpu
 *   - misc control registers and leds
 *   - slots/graphics
 *   - sound
 *   - centronics
 */

#include "emu.h"

#include "cpu/mips/mips1.h"
#include "cpu/m68000/m68030.h"

// memory
#include "machine/ram.h"

// various hardware
#include "machine/msm6242.h"
#include "machine/z80scc.h"
#include "machine/am79c90.h"
#include "machine/upd765.h"
#include "dmac_0266.h"
#include "news_hid.h"
#include "machine/ncr5380.h"

// video
#include "screen.h"

// busses and connectors
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"

#include "machine/input_merger.h"
#include "imagedev/floppy.h"

#define VERBOSE 1
#include "logmacro.h"


namespace {

class news_38xx_state : public driver_device
{
public:
	news_38xx_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
//      , m_cpu(*this, "cpu")
		, m_iop(*this, "iop")
		, m_ram(*this, "ram")
		, m_dma(*this, "dma%u", 0U)
		, m_rtc(*this, "rtc")
		, m_scc(*this, "scc")
		, m_net(*this, "net")
		, m_fdc(*this, "fdc")
		, m_hid(*this, "hid")
		, m_scsi(*this, "scsi%u:7:cxd1180", 0U)
		, m_serial(*this, "serial%u", 0U)
		, m_scsibus(*this, "scsi%u", 0U)
		, m_serial_irq(*this, "serial_irq")
		, m_eprom(*this, "eprom")
//      , m_vram(*this, "vram")
//      , m_led(*this, "led%u", 0U)
	{
	}

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	[[maybe_unused]] void cpu_map(address_map &map) ATTR_COLD;
	void iop_map(address_map &map) ATTR_COLD;
	void iop_vector_map(address_map &map) ATTR_COLD;

	// machine config
	void common(machine_config &config);

public:
	void nws3860(machine_config &config);

	void init_common();

protected:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	// INTST only has some of the IRQ status info
	u8 intst_r() { return m_intst & 0x7f; } // TODO: distinguish between FDCDRQ and PERR since this reg only has PERR

	enum class iop_irq_number : unsigned
	{
		SLOT   = 0,   // Expansion I/O bus interrupts
		PARALLEL = 1, // Centronics interface
		FDC    = 2,   // FDCIRQ
		LANCE  = 3,   // Ethernet controller interrupts
		IPC    = 4,   // CPU and UBUS interrupts
		SCSI1  = 5,   // SCSI bus 0 interrupts
		SCSI0  = 6,   // SCSI bus 1 interrupts
		PERR = 7  // Main memory parity error
	};
	template <iop_irq_number Number> void irq_w(int state);
	void int_check_iop();

	enum class cpu_irq_number : unsigned
	{
		UBUS = 0,   // Interprocessor communication interrupt from UPU
		IOP = 1,    // Interprocessor communication interrupt from IOP
		TIMER = 2,  // 100Hz timer
		FPA = 3,    // FPU interrupt
		WRBERR = 4, // async bus write error
		PERR = 5    // Main memory parity error
	};

	u32 bus_error_r();
	void timer_w(u8 data);
	void timer(s32 param);
	void poweron_w(u8 data);

	// devices
//  required_device<r3000a_device> m_cpu;
	required_device<m68030_device> m_iop;
	required_device<ram_device> m_ram;
	required_device_array<dmac_0266_device, 2> m_dma;
	required_device<rtc62421_device> m_rtc;
	required_device<z80scc_device> m_scc;
	required_device<am7990_device> m_net;
	required_device<n82077aa_device> m_fdc;

	required_device<news_hid_hle_device> m_hid;
	required_device_array<cxd1180_device, 2> m_scsi;

	required_device_array<rs232_port_device, 2> m_serial;
	required_device_array<nscsi_bus_device, 2> m_scsibus;

	required_device<input_merger_device> m_serial_irq;

	required_region_ptr<u32> m_eprom;
	//required_shared_ptr<u32> m_vram;
	//output_finder<4> m_led;

	std::unique_ptr<u16[]> m_net_ram;

	emu_timer *m_timer = nullptr;

	u8 m_intst = 0;
	u8 m_inten = 0;

	bool m_int_state[3]{};
	bool m_scc_irq_state = false;
};

void news_38xx_state::machine_start()
{
	//m_led.resolve();

	m_net_ram = std::make_unique<u16[]>(8192);

	save_pointer(NAME(m_net_ram), 8192);

	save_item(NAME(m_intst));
	save_item(NAME(m_inten));
	save_item(NAME(m_int_state));

	m_timer = timer_alloc(FUNC(news_38xx_state::timer), this);

	m_intst = 0;
	m_inten = 0x6d;
	for (bool &int_state : m_int_state)
		int_state = false;
	m_scc_irq_state = false;
}

void news_38xx_state::machine_reset()
{
	// eprom is mapped at 0 after reset
	m_iop->space(0).install_rom(0x00000000, 0x0000ffff, m_eprom);

	m_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));
}

void news_38xx_state::init_common()
{
	// HACK: hardwire the rate
	m_fdc->set_rate(500000);
}

void news_38xx_state::cpu_map(address_map &map)
{
}

void news_38xx_state::iop_map(address_map &map)
{
	map.global_mask(0x3fffffff); // A29-A0 are connected
	map(0x18000000, 0x1803ffff).ram(); // IOP ram

	map(0x20000000, 0x2000ffff).rom().region("eprom", 0).mirror(0x1fff0000);

	map(0x22000000, 0x22000000).w(FUNC(news_38xx_state::poweron_w));
	map(0x22000001, 0x22000001).lw8([this](u8 data) { m_iop->space(0).install_ram(0x00000000, m_ram->mask(), 0x00000000, m_ram->pointer()); }, "ram_enable");
	map(0x22000002, 0x22000002).lw8([this](u8 data) { m_inten |= 0x80; }, "pie_w");
	map(0x22000003, 0x22000003).w(FUNC(news_38xx_state::timer_w));
	// 0x22000004 // soft interrupt
	// 0x22000005 // ast interrupt
	// 0x22000006 // iopled 0
	// 0x22000007 // iopled 1

	// 0x22800000-0x2280001f Inter-processor Control Registers
	// 0x22800000 CPUSTART -> Run the main CPU
	// 0x22800000 HPUSTART -> Run the UPU (UBUS bus master, expansion slot A??)
	// 0x22800008 IPINTCP  -> Send level 2 IRQ to CPU
	// 0x22800009 IPINTHP  -> Send level 2 IRQ to UPU
	// 0x22800010 IPENICP  -> Enable level 4 IRQ from CPU (writing 0 will not clear IRQ)
	// 0x22800011 IPENIHP  -> Enable level 4 IRQ from UPU (writing 0 will not clear IRQ)
	// 0x22800018 IPCLICP  -> Clear level 4 IRQ from CPU
	// 0x22800019 IPCLIHP  -> Clear level 4 IRQ from UPU

	map(0x23000000, 0x23000000).r(FUNC(news_38xx_state::intst_r));
	// 0x23800000 // inter-processor interrupt status -> bit 0 = CPU, bit 1 = UPU

	map(0x24000000, 0x24000007).m(m_fdc, FUNC(n82077aa_device::map));
	map(0x24000105, 0x24000105).rw(m_fdc, FUNC(n82077aa_device::dma_r), FUNC(n82077aa_device::dma_w));

	// 0x26040000 // centronics data
	// 0x26040001 centronics strobe
	// 0x26040002 centronics IRQ clear
	map(0x26040003, 0x26040003).lw8([this](u8 data) { m_inten |= 0x02; }, "cie_w");

	map(0x26080000, 0x26080007).m(m_scsi[0], FUNC(cxd1180_device::map));
	map(0x260c0000, 0x260c0007).m(m_scsi[1], FUNC(cxd1180_device::map));

	map(0x26100000, 0x26100007).m(m_hid, FUNC(news_hid_hle_device::map_68k));
	map(0x26140000, 0x26140003).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));
	map(0x26180000, 0x2618000f).rw(m_rtc, FUNC(rtc62421_device::read), FUNC(rtc62421_device::write));
	// 0x261c0000 // kb beep frequency LOWTONE 0x1 = low, 0x0 = high

	map(0x26200000, 0x26203fff).lrw16(
		[this](offs_t offset) { return m_net_ram[offset]; }, "net_ram_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset]); }, "net_ram_w");
	// 0x262c0000 // audio

	map(0x26300000, 0x26300003).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));
	// 0x26340000 // park status (SCC and parallel info, TODO: double-check that park has this)
	// 0x26380000 // divide fdd index pulse for ECMA format (1 = 1/2x 0 = 1x)

	map(0x28000000, 0x28000017).m(m_dma[0], FUNC(dmac_0266_device::map));
	map(0x2a000000, 0x2a000017).m(m_dma[1], FUNC(dmac_0266_device::map));

	map(0x2c000000, 0x2c0000ff).rom().region("idrom", 0);
	map(0x2c000100, 0x2c000103).portr("SW1");

	map(0x2e000000, 0x2effffff).r(FUNC(news_38xx_state::bus_error_r)).mirror(0x10000000); // Expansion I/O and mirror
}

void news_38xx_state::iop_vector_map(address_map &map)
{
	map(0xfffffff3, 0xfffffff3).lr8(NAME([] { return m68000_base_device::autovector(1); }));
	map(0xfffffff5, 0xfffffff5).lr8(NAME([] { return m68000_base_device::autovector(2); }));
	map(0xfffffff7, 0xfffffff7).lr8(NAME([] { return m68000_base_device::autovector(3); }));
	map(0xfffffff9, 0xfffffff9).lr8(NAME([] { return m68000_base_device::autovector(4); }));
	map(0xfffffffb, 0xfffffffb).lr8(NAME([this] { return m_scc_irq_state ? m_scc->m1_r() : m68000_base_device::autovector(5); }));
	map(0xfffffffd, 0xfffffffd).lr8(NAME([] { return m68000_base_device::autovector(6); }));
	map(0xffffffff, 0xffffffff).lr8(NAME([] { return m68000_base_device::autovector(7); }));
}

template <news_38xx_state::iop_irq_number Number> void news_38xx_state::irq_w(int state)
{
	LOG("irq number %d state %d\n",  static_cast<unsigned>(Number), state);

	if (state)
		m_intst |= 1U << static_cast<unsigned>(Number);
	else
		m_intst &= ~(1U << static_cast<unsigned>(Number));

	int_check_iop();
}

void news_38xx_state::int_check_iop()
{
	static int constexpr intst_line[] = { INPUT_LINE_IRQ7, INPUT_LINE_IRQ4, INPUT_LINE_IRQ3 };
	static u8 constexpr intst_mask[] = { 0x80, 0x78, 0x07 };

	for (unsigned i = 0; i < std::size(m_int_state); i++)
	{
		bool const int_state = m_intst & m_inten & intst_mask[i];

		if (m_int_state[i] != int_state)
		{
			m_int_state[i] = int_state;
			m_iop->set_input_line(intst_line[i], int_state);
		}
	}
}

u32 news_38xx_state::bus_error_r()
{
	if (!machine().side_effects_disabled())
		m_iop->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);

	return 0;
}

void news_38xx_state::timer_w(u8 data)
{
	LOG("timer_w 0x%02x\n", data);

	m_timer->set_param(data);

	if (!data)
		m_iop->set_input_line(INPUT_LINE_IRQ6, CLEAR_LINE);
}

void news_38xx_state::timer(s32 param)
{
	if (param)
		m_iop->set_input_line(INPUT_LINE_IRQ6, ASSERT_LINE);
}

void news_38xx_state::poweron_w(uint8_t data)
{
	LOG("Write POWERON = 0x%x (%s)\n", data, machine().describe_context());

	if (!data && !machine().side_effects_disabled())
	{
		machine().schedule_exit();
	}
}

void news_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
}

void news_38xx_state::common(machine_config &config)
{
	//R3000A(config, m_cpu, 25_MHz_XTAL, 32768, 32768);
	//m_cpu->set_addrmap(AS_PROGRAM, &news_38xx_state::cpu_map);
	//m_cpu->set_fpu(mips1_device_base::MIPS_R3010Av4);

	M68030(config, m_iop, 50_MHz_XTAL / 2);
	m_iop->set_addrmap(AS_PROGRAM, &news_38xx_state::iop_map);
	m_iop->set_addrmap(m68000_base_device::AS_CPU_SPACE, &news_38xx_state::iop_vector_map);

	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	// TODO: confirm each bank supports 4x1M or 4x4M
	//m_ram->set_extra_options("4M,8M,12M,20M,24M,32M,36M,48M");

	RTC62421(config, m_rtc, 32.768_kHz_XTAL);

	DMAC_0266(config, m_dma[0], 0);
	m_dma[0]->set_bus(m_iop, 0);

	DMAC_0266(config, m_dma[1], 0);
	m_dma[1]->set_bus(m_iop, 0);

	INPUT_MERGER_ANY_HIGH(config, m_serial_irq);
	m_serial_irq->output_handler().set_inputline(m_iop, INPUT_LINE_IRQ5);

	SCC85C30(config, m_scc, 4.9152_MHz_XTAL);
	m_scc->out_int_callback().set(
		[this](int state)
		{
			m_scc_irq_state = bool(state);
			m_serial_irq->in_w<2>(state);
		});

	// scc channel A
	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
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
	m_net->intr_out().set(FUNC(news_38xx_state::irq_w<iop_irq_number::LANCE>)).invert();
	m_net->dma_in().set([this](offs_t offset) { return m_net_ram[offset >> 1]; });
	m_net->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset >> 1]); });

	N82077AA(config, m_fdc, 16_MHz_XTAL, n82077aa_device::mode_t::PS2);
	m_fdc->intrq_wr_callback().set(FUNC(news_38xx_state::irq_w<iop_irq_number::FDC>));
	m_fdc->drq_wr_callback().set(FUNC(news_38xx_state::irq_w<iop_irq_number::PERR>)); // TODO: don't just use PERR here
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(false);

	// scsi bus 0 and devices
	NSCSI_BUS(config, m_scsibus[0]);
	NSCSI_CONNECTOR(config, "scsi0:0", news_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi0:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:6", news_scsi_devices, nullptr);

	// scsi bus 0 host adapter
	NSCSI_CONNECTOR(config, "scsi0:7").option_set("cxd1180", CXD1180).machine_config(
		[this](device_t *device)
		{
			auto &adapter = downcast<cxd1180_device &>(*device);

			adapter.irq_handler().set(*this, FUNC(news_38xx_state::irq_w<iop_irq_number::SCSI0>));
			adapter.irq_handler().append(m_dma[0], FUNC(dmac_0266_device::eop_w));
			adapter.drq_handler().set(m_dma[0], FUNC(dmac_0266_device::req_w));

			//subdevice<dmac_0266_device>(":dma0")->out_eop_cb().set(adapter, FUNC(cxd1180_device::eop_w));
			subdevice<dmac_0266_device>(":dma0")->dma_r_cb().set(adapter, FUNC(cxd1180_device::dma_r));
			subdevice<dmac_0266_device>(":dma0")->dma_w_cb().set(adapter, FUNC(cxd1180_device::dma_w));
		});

	// scsi bus 1 and devices
	NSCSI_BUS(config, m_scsibus[1]);
	NSCSI_CONNECTOR(config, "scsi1:0", news_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi1:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:6", news_scsi_devices, nullptr);

	// scsi bus 1 host adapter
	NSCSI_CONNECTOR(config, "scsi1:7").option_set("cxd1180", CXD1180).machine_config(
		[this](device_t *device)
		{
			auto &adapter = downcast<cxd1180_device &>(*device);

			adapter.irq_handler().set(*this, FUNC(news_38xx_state::irq_w<iop_irq_number::SCSI1>));
			adapter.irq_handler().append(m_dma[1], FUNC(dmac_0266_device::eop_w));
			adapter.drq_handler().set(m_dma[1], FUNC(dmac_0266_device::req_w));

			//subdevice<dmac_0266_device>(":dma1")->out_eop_cb().set(adapter, FUNC(cxd1180_device::eop_w));
			subdevice<dmac_0266_device>(":dma1")->dma_r_cb().set(adapter, FUNC(cxd1180_device::dma_r));
			subdevice<dmac_0266_device>(":dma1")->dma_w_cb().set(adapter, FUNC(cxd1180_device::dma_w));
		});

	NEWS_HID_HLE(config, m_hid);
	m_hid->irq_out<news_hid_hle_device::KEYBOARD>().set(m_serial_irq, FUNC(input_merger_device::in_w<0>));
	m_hid->irq_out<news_hid_hle_device::MOUSE>().set(m_serial_irq, FUNC(input_merger_device::in_w<1>));

	SOFTWARE_LIST(config, "software_list").set_original("sony_news").set_filter("RISC");
}

void news_38xx_state::nws3860(machine_config &config)
{
	common(config);
}

ROM_START(nws3860)
	ROM_REGION32_BE(0x10000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "nws3860", "NWS-3860 v1.6")
	ROMX_LOAD("mpu_10__ver.1.6__1990_sony.ic159", 0x00000, 0x10000, CRC(542e21b8) SHA1(631dca1f3446761973073f5c32c1a0aeba538c2c), ROM_BIOS(0))

	ROM_REGION32_BE(0x100, "idrom", 0)
	ROM_LOAD("idrom.bin", 0x000, 0x100, CRC(a2b9e968) SHA1(09c7d253c7ed0b368c49f4b60bfe5ca76acd7cc3) BAD_DUMP)
ROM_END

static INPUT_PORTS_START(nws3860)
	PORT_START("SW1")
	PORT_DIPNAME(0x07000000, 0x07000000, "Display") PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(0x07000000, "Console")
	PORT_DIPSETTING(0x06000000, "NWB-512")
	PORT_DIPSETTING(0x03000000, "NWB-225A")
	PORT_DIPSETTING(0x00000000, "Autoselect")

	PORT_DIPNAME(0x08000000, 0x08000000, "Boot Device") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x08000000, "SCSI")
	PORT_DIPSETTING(0x00000000, "Floppy")

	PORT_DIPNAME(0x10000000, 0x10000000, "Automatic Boot") PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(0x10000000, DEF_STR(Off))
	PORT_DIPSETTING(0x00000000, DEF_STR(On))

	PORT_DIPNAME(0x20000000, 0x20000000, "Diagnostic Mode") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(0x20000000, DEF_STR(Off))
	PORT_DIPSETTING(0x00000000, DEF_STR(On))

	PORT_DIPNAME(0xc0000000, 0xc0000000, "Unused") PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(0xc0000000, DEF_STR(Off))
INPUT_PORTS_END

} // anonymous namespace


/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS            INIT         COMPANY  FULLNAME    FLAGS */
COMP(1989, nws3860, 0,      0,      nws3860, nws3860, news_38xx_state, init_common, "Sony",  "NWS-3860", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
