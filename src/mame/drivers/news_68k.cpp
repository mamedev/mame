// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Sony NEWS M68K systems.
 *
 * Sources:
 *   - http://wiki.netbsd.org/ports/news68k/
 *
 * TODO:
 *   - mouse/keyboard
 *   - graphics/slots
 */

#include "emu.h"

#include "cpu/m68000/m68000.h"

// memory
#include "machine/ram.h"

// various hardware
#include "machine/timekpr.h"
#include "machine/z80scc.h"
#include "machine/am79c90.h"
#include "machine/upd765.h"
#include "machine/dmac_0266.h"
#include "machine/news_hid.h"
#include "machine/ncr5380n.h"

// video
#include "screen.h"

// busses and connectors
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"

#include "machine/input_merger.h"
#include "imagedev/floppy.h"
#include "formats/pc_dsk.h"

#include "debugger.h"

#define VERBOSE 0
#include "logmacro.h"

class news_68k_state : public driver_device
{
public:
	news_68k_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_dma(*this, "dma")
		, m_rtc(*this, "rtc")
		, m_scc(*this, "scc")
		, m_net(*this, "net")
		, m_fdc(*this, "fdc")
		, m_scsi(*this, "scsi:7:cxd1180")
		, m_hid(*this, "hid")
		, m_serial(*this, "serial%u", 0U)
		, m_scsibus(*this, "scsi")
		, m_scc_irq(*this, "scc_irq")
		, m_fdc_irq(*this, "fdc_irq")
		, m_eprom(*this, "eprom")
	{
	}

	void nws1580(machine_config &config);
	void init_common();

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	void cpu_map(address_map &map);
	void cpu_autovector_map(address_map &map);

	// machine config
	void common(machine_config &config);

	enum irq_number : unsigned
	{
		IPIRQ1  = 0,
		IPIRQ3  = 1,
		LANCE   = 2,
		VME4    = 3,
		VME2    = 4,
		FDC     = 5,
		PRINTER = 6,
		SCSI    = 7,
	};
	template <irq_number Number> void irq_w(int state);
	void int_check();

	void timer(void *ptr, s32 param);
	void timer_w(u8 data);

	u32 bus_error_r();

private:
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	// devices
	required_device<m68030_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<dmac_0266_device> m_dma;
	required_device<m48t02_device> m_rtc;
	required_device<z80scc_device> m_scc;
	required_device<am7990_device> m_net;
	required_device<upd72067_device> m_fdc;
	required_device<ncr5380n_device> m_scsi;
	required_device<news_hid_hle_device> m_hid;

	required_device_array<rs232_port_device, 2> m_serial;
	required_device<nscsi_bus_device> m_scsibus;

	required_device<input_merger_device> m_scc_irq;
	required_device<input_merger_device> m_fdc_irq;

	required_region_ptr<u32> m_eprom;
	std::unique_ptr<u16[]> m_net_ram;

	emu_timer *m_timer;

	u8 m_intst;
	bool m_int_state[2];
	bool m_scc_irq_state;
};

void news_68k_state::machine_start()
{
	m_net_ram = std::make_unique<u16[]>(8192);

	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(news_68k_state::timer), this));

	for (bool &int_state : m_int_state)
		int_state = false;

	m_scc_irq_state = false;
}

void news_68k_state::machine_reset()
{
	// eprom is mapped at 0 after reset
	m_cpu->space(0).install_rom(0x00000000, 0x0000ffff, m_eprom);

	m_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));
}

void news_68k_state::init_common()
{
	// HACK: hardwire the rate
	m_fdc->set_rate(500000);
}

void news_68k_state::cpu_map(address_map &map)
{
	map(0xe0000000, 0xe000ffff).rom().region("eprom", 0);

	map(0xe0c80000, 0xe0c80003).m(m_fdc, FUNC(upd72067_device::map));
	//map(0xe0c80100, 0xe0c80100); // fdc psuedo - dma ?
	//map(0xe0cc0000, 0xe0cc0007).m(m_scsi, FUNC(ncr5380n_device::map));
	map(0xe0cc0000, 0xe0cc0007).rw(m_scsi, FUNC(ncr5380n_device::read), FUNC(ncr5380n_device::write));

	map(0xe0d00000, 0xe0d00007).m(m_hid, FUNC(news_hid_hle_device::map_68k));
	map(0xe0d40000, 0xe0d40003).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));
	map(0xe0d80000, 0xe0d807ff).rw(m_rtc, FUNC(m48t02_device::read), FUNC(m48t02_device::write));
	map(0xe0dc0000, 0xe0dc0000).lw8([this](u8 data) { logerror("debug_w 0x%02x\n", data); }, "debug_w");

	map(0xe0e00000, 0xe0e03fff).lrw16(
		[this](offs_t offset) { return m_net_ram[offset]; }, "net_ram_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset]); }, "net_ram_w");
	map(0xe0e80000, 0xe0e80017).m(m_dma, FUNC(dmac_0266_device::map));

	map(0xe0f00000, 0xe0f00003).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));

	map(0xe1000000, 0xe1000000).w(FUNC(news_68k_state::timer_w));
	map(0xe1080000, 0xe1080000).lw8([this](u8 data) { logerror("parity enable 0x%02x\n", data); }, "parity_enable_w");
	map(0xe1180000, 0xe1180000).lw8([this](u8 data) { logerror("intl2 enable 0x%02x\n", data); }, "intl2_enable_w");
	map(0xe1200000, 0xe1200000).lw8([this](u8 data) { m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), 0xc0000000, m_ram->pointer()); }, "ram_enable");
	map(0xe1280000, 0xe1280000).lw8([this](u8 data) { logerror("ast enable 0x%02x\n", data); }, "ast_enable_w");
	map(0xe1300000, 0xe1300000).lw8([this](u8 data) { logerror("cache enable 0x%02x (%s)\n", data, machine().describe_context()); }, "cache_enable_w");
	map(0xe1900000, 0xe1900000).lw8([this](u8 data) { logerror("cache clear 0x%02x\n", data); }, "cache_clear_w");
	map(0xe1a00000, 0xe1a00000).lw8([this](u8 data) { logerror("parity clear 0x%02x\n", data); }, "parity_clear_w");

	map(0xe1c00000, 0xe1c000ff).rom().region("idrom", 0);
	map(0xe1c00100, 0xe1c00103).portr("SW1");
	map(0xe1c00200, 0xe1c00203).lr8([this]() { return m_intst; }, "intst_r");

	map(0xf0000000, 0xffffffff).r(FUNC(news_68k_state::bus_error_r));

	// 0xf0c30000 expansion lance #1
	// 0xf0c20000   lance #1 memory
	// 0xf0c38000   lance #1 etherid
	// 0xf0c70000 expansion lance #2
	// 0xf0c60000   lance #2 memory
	// 0xf0c78000   lance #2 etherid

	// 0xf0f00000 nwb512_base
	// 0xf0fc0000 nwb512krom_base
	// 0xf0700000 nwb225_base
	// 0xf0600000 nwb225krom_base
}

void news_68k_state::cpu_autovector_map(address_map &map)
{
	map.global_mask(0xffffffff);

	map(0xfffffff3, 0xfffffff3).lr8(NAME([]() { return m68000_base_device::autovector(1); }));
	map(0xfffffff5, 0xfffffff5).lr8(NAME([]() { return m68000_base_device::autovector(2); }));
	map(0xfffffff7, 0xfffffff7).lr8(NAME([]() { return m68000_base_device::autovector(3); }));
	map(0xfffffff9, 0xfffffff9).lr8(NAME([]() { return m68000_base_device::autovector(4); }));
	map(0xfffffffb, 0xfffffffb).lr8(NAME([this]() { return m_scc_irq_state ? m_scc->m1_r() : m68000_base_device::autovector(5); }));
	map(0xfffffffd, 0xfffffffd).lr8(NAME([]() { return m68000_base_device::autovector(6); }));
	map(0xffffffff, 0xffffffff).lr8(NAME([]() { return m68000_base_device::autovector(7); }));
}

template <news_68k_state::irq_number Number> void news_68k_state::irq_w(int state)
{
	LOG("irq number %d state %d\n", Number, state);

	if (state)
		m_intst |= 1U << Number;
	else
		m_intst &= ~(1U << Number);

	int_check();
}

void news_68k_state::int_check()
{
	// TODO: assume 43334443, masking?
	static int const int_line[] = { INPUT_LINE_IRQ3, INPUT_LINE_IRQ4 };
	static u8 const int_mask[] = { 0x71, 0x8e };

	for (unsigned i = 0; i < ARRAY_LENGTH(m_int_state); i++)
	{
		bool const int_state = m_intst & int_mask[i];

		if (m_int_state[i] != int_state)
		{
			m_int_state[i] = int_state;
			m_cpu->set_input_line(int_line[i], int_state);
		}
	}
}

void news_68k_state::timer_w(u8 data)
{
	LOG("timer_w 0x%02x\n", data);

	m_timer->set_param(data);

	if (!data)
		m_cpu->set_input_line(INPUT_LINE_IRQ6, CLEAR_LINE);
}

void news_68k_state::timer(void *ptr, s32 param)
{
	if (param)
		m_cpu->set_input_line(INPUT_LINE_IRQ6, ASSERT_LINE);
}

u32 news_68k_state::bus_error_r()
{
	if (!machine().side_effects_disabled())
		m_cpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);

	return 0;
}

static void news_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
}

void news_68k_state::common(machine_config &config)
{
	M68030(config, m_cpu, 50_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &news_68k_state::cpu_map);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &news_68k_state::cpu_autovector_map);

	// 16 SIMM slots for RAM arranged as two groups of 8 slots, with each bank
	// corresponding to a pair of slots in each group; first bank soldered in
	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	// TODO: assume only 1M modules are supported
	m_ram->set_extra_options("8M,12M,16M");
	m_ram->set_default_value(0);

	M48T02(config, m_rtc);

	DMAC_0266(config, m_dma, 0);
	m_dma->set_bus(m_cpu, 0);

	INPUT_MERGER_ANY_HIGH(config, m_scc_irq);
	m_scc_irq->output_handler().set_inputline(m_cpu, INPUT_LINE_IRQ5);

	SCC85C30(config, m_scc, 3993600);
	m_scc->out_int_callback().set(
		[this](int state)
		{
			m_scc_irq_state = bool(state);
			m_scc_irq->in_w<2>(state);
		});

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
	m_net->intr_out().set(FUNC(news_68k_state::irq_w<LANCE>)).invert();
	m_net->dma_in().set([this](offs_t offset) { return m_net_ram[offset >> 1]; });
	m_net->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset >> 1]); });

	INPUT_MERGER_ANY_HIGH(config, m_fdc_irq);
	m_fdc_irq->output_handler().set(FUNC(news_68k_state::irq_w<FDC>));

	UPD72067(config, m_fdc, 16_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(m_fdc_irq, FUNC(input_merger_any_high_device::in_w<0>));
	m_fdc->drq_wr_callback().set(m_fdc_irq, FUNC(input_merger_any_high_device::in_w<1>));
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_formats).enable_sound(false);

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus);

	/*
	 * CDC WREN V HH 94221-5 (5.25" half-height SCSI-1 single-ended)
	 * 1544 cylinders, 5 heads, 52 sectors/cylinder, ~170MiB formatted
	 *
	 * Vendor   Product          Rev. Vendor-specific
	 * CDC      94221-5          5457 00018715
	 */
	NSCSI_CONNECTOR(config, "scsi:0", news_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", news_scsi_devices, nullptr);

	// scsi host adapter
	NSCSI_CONNECTOR(config, "scsi:7").option_set("cxd1180", NCR5380N).machine_config(
		[this](device_t *device)
		{
			ncr5380n_device &adapter = downcast<ncr5380n_device &>(*device);

			adapter.irq_handler().set(*this, FUNC(news_68k_state::irq_w<SCSI>));
			adapter.drq_handler().set(m_dma, FUNC(dmac_0266_device::drq_w));

			//subdevice<dmac_0266_device>(":dma")->out_eop_cb().set(adapter, FUNC(ncr5380n_device::eop_w));
			subdevice<dmac_0266_device>(":dma")->dma_r_cb().set(adapter, FUNC(ncr5380n_device::dma_r));
			subdevice<dmac_0266_device>(":dma")->dma_w_cb().set(adapter, FUNC(ncr5380n_device::dma_w));
		});

	NEWS_HID_HLE(config, m_hid);
	//m_hid->irq_out<news_hid_hle_device::KEYBOARD>().set(m_scc_irq, FUNC(input_merger_device::in_w<0>));
	//m_hid->irq_out<news_hid_hle_device::MOUSE>().set(m_scc_irq, FUNC(input_merger_device::in_w<1>));
}

void news_68k_state::nws1580(machine_config &config)
{
	common(config);
}

FLOPPY_FORMATS_MEMBER(news_68k_state::floppy_formats)
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static INPUT_PORTS_START(nws15x0)
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

ROM_START(nws1580)
	ROM_REGION32_BE(0x10000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "nws1580", "NWS-1580 v1.3")
	ROMX_LOAD("pws_1500__ver_1.3__8906.bin", 0x00000, 0x10000, CRC(76395ad9) SHA1(c2ae00218c23cef6519a4d7c74ac2c552790dfd4), ROM_BIOS(0))

	// MB7114 256x4 TTL PROM
	ROM_REGION32_BE(0x100, "idrom", 0)
	ROM_LOAD("n1580_50093.ic63", 0x000, 0x100, CRC(a7f293d6) SHA1(21deffed69e07af515ffc5511bdbf73a2a4c14fb))
ROM_END

/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS           INIT         COMPANY  FULLNAME    FLAGS */
COMP(1988, nws1580, 0,      0,      nws1580, nws15x0, news_68k_state, init_common, "Sony",  "NWS-1580", MACHINE_NOT_WORKING)
