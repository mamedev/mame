// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Tektronix Intelligent Graphics Workstatsions (62xx?, 6110?, 6120, 6130, 4132)
 *
 * Sources:
 *   - http://bitsavers.trailing-edge.com/pdf/tektronix/6130/6100_Series_Hardware_Descriptions.pdf
 *
 * TODO:
 *   - ns32082 mmu
 *   - am9516 dma controller
 *   - slots and cards
 *   - gpib devices
 *   - graphics and keyboard
 */
/*
 * WIP
 *  - novram chip select not asserted when trying to read ethernet mac address?
 *  - aborts to firmware monitor on second serial port after failing to boot
 */

#include "emu.h"

// cpus and memory
#include "cpu/ns32000/ns32000.h"
#include "machine/ram.h"

// various hardware
//#include "machine/am9516a.h"
#include "machine/eepromser.h"
#include "machine/i82586.h"
#include "machine/mm58167.h"
#include "machine/ns32081.h"
//#include "machine/ns32082.h"
#include "machine/ns32202.h"
#include "machine/tms9914.h"
#include "machine/wd_fdc.h"
#include "machine/wd1010.h"
#include "machine/z80scc.h"

// busses and connectors
#include "bus/ieee488/ieee488.h"
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class tek6100_state : public driver_device
{
public:
	tek6100_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_fpu(*this, "fpu")
		//, m_mmu(*this, "mmu")
		, m_icu(*this, "icu")
		, m_ram(*this, "ram")
		, m_scc(*this, "scc")
		, m_serial(*this, "port%u", 0U)
		, m_gpib(*this, "gpib")
		, m_rtc(*this, "rtc")
		, m_nov(*this, "nov%u", 0U)
		, m_fdc(*this, "fdc")
		, m_fdd(*this, "fdc:%u:525dd", 0U)
		//, m_dma(*this, "dma")
		, m_hdc(*this, "hdc")
		, m_hdd(*this, "hdc:0")
		, m_lan(*this, "lan")
		, m_led(*this, "led")
	{
	}

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	template <unsigned ST> void cpu_map(address_map &map);
	void lan_map(address_map &map);

public:
	// machine config
	void tek6130(machine_config &config);

	void init_common();

protected:
	// computer board control registers
	u8 nov_r() { return m_nmr; }
	u8 scr_r() { return m_scr; }
	u8 ssr_r() { return m_ssr; }
	void fcr_w(u8 data);
	void gcr_w(u8 data);
	void hcr_w(u8 data);
	void led_w(u8 data) { m_led = data; }
	void nov_w(u8 data);
	void scr_w(u8 data);

	// novram data out handler
	void nov_do(int state);

	// hard disk buffer handlers
	template <typename T> T buf_r();
	template <typename T> void buf_w(T data);

	required_device<ns32016_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	//required_device<ns32082_device> m_mmu;
	required_device<ns32202_device> m_icu;
	required_device<ram_device> m_ram;

	required_device<scc8530_device> m_scc;
	required_device_array<rs232_port_device, 2> m_serial;
	required_device<tms9914_device> m_gpib;
	required_device<mm58167_device> m_rtc;
	optional_device_array<eeprom_serial_x24c44_16bit_device, 2> m_nov;
	required_device<wd1770_device> m_fdc;
	optional_device_array<floppy_image_device, 2> m_fdd;
	//required_device<am9516a_device> m_dma;
	required_device<wd1010_device> m_hdc;
	required_device<harddisk_image_device> m_hdd;
	required_device<i82586_device> m_lan;

	output_finder<> m_led;

private:
	u8 m_hcr; // hard disk control register
	u8 m_nmr; // nonvolatile memory register
	u16 m_per; // parity error register
	u8 m_scr; // system control register
	u8 m_ssr; // system status register

	// ram parity state
	memory_passthrough_handler *m_parity_mph;
	std::unique_ptr<u32[]> m_parity_flag;
	unsigned m_parity_bad;

	// hard disk controller buffer
	u16 m_hdc_ptr;
	std::unique_ptr<u8[]> m_hdc_buf;
};

void tek6100_state::machine_start()
{
	m_hdc_buf = std::make_unique<u8[]>(4096);

	save_item(NAME(m_hcr));
	save_item(NAME(m_nmr));
	save_item(NAME(m_per));
	save_item(NAME(m_scr));
	save_item(NAME(m_ssr));

	save_item(NAME(m_hdc_ptr));
	save_pointer(NAME(m_hdc_buf), 4096);

	m_per = 0;
	m_ssr = 0;
}

void tek6100_state::machine_reset()
{
	hcr_w(0);
	nov_w(0);
	scr_w(0);
	fcr_w(0);
}

void tek6100_state::init_common()
{
	m_led.resolve();

	m_lan->space(0).install_ram(0, m_ram->mask(), m_ram->pointer());
}

enum fcr_mask : u8
{
	FCR_DRIVE = 0x01, // drive select
	FCR_SIDE  = 0x02, // side select
	FCR_DDEN  = 0x04, // double density enable
	FCR_FDEN  = 0x08, // flexible disk enable
};

void tek6100_state::fcr_w(u8 data)
{
	floppy_image_device *fdd = nullptr;
	if (data & FCR_FDEN)
		fdd = m_fdd[data & FCR_DRIVE];

	m_fdc->set_floppy(fdd);
	if (fdd)
		fdd->ss_w(bool(data & FCR_SIDE));

	m_fdc->dden_w(bool(data & FCR_DDEN));
}

enum gcr_mask : u8
{
	GCR_SYSCTL = 0x02, // gpib line driver enable
};

void tek6100_state::gcr_w(u8 data)
{
	// TODO: enable/disable line drivers
}

enum scr_mask : u8
{
	SCR_RAME  = 0x01, // ram enable
	SCR_NMIE  = 0x04, // nmi enable
	SCR_LLOP  = 0x08, // lan loopback enable
	SCR_BERRE = 0x10, // bus error enable
	SCR_DMAE  = 0x20, // dma enable
	SCR_PARB  = 0x40, // parity bad
	SCR_PARE  = 0x80, // parity enable
};

enum ssr_mask : u8
{
	SSR_DMACH = 0x0f, // dma channel
	SSR_BERR  = 0x10, // bus error
	SSR_PWRSW = 0x20, // power switch
	SSR_PFAIL = 0x40, // power fail
	SSR_PERR  = 0x80, // parity error
};

enum per_mask : u16
{
	PER_PARADR = 0x3fff, // parity address
	PER_HIERR  = 0x4000,
	PER_LOERR  = 0x8000,
};

void tek6100_state::scr_w(u8 data)
{
	if (!(m_scr & SCR_RAME) && (data & SCR_RAME))
		m_cpu->space(0).install_ram(0, m_ram->mask(), m_ram->pointer());

	if ((m_scr ^ data) & SCR_LLOP)
		m_lan->set_loopback(data & SCR_LLOP);

	// install parity handlers
	if (!(m_scr & SCR_PARB) && (data & SCR_PARB) && !m_parity_mph)
	{
		m_parity_flag = std::make_unique<u32[]>(m_ram->size() / 32);
		m_parity_bad = 0;

		m_parity_mph = m_cpu->space(0).install_readwrite_tap(0, m_ram->mask(), "parity",
			[this](offs_t offset, u16 &data, u16 mem_mask)
			{
				if (m_scr & SCR_PARE)
				{
					bool error = false;

					// check bad parity (lo)
					if ((mem_mask & 0x00ff) && BIT(m_parity_flag[offset / 32], (offset & 31) + 0))
					{
						m_ssr |= SSR_PERR;
						m_per &= ~PER_PARADR;
						m_per |= PER_LOERR | ((offset >> 9) & PER_PARADR);

						error = true;
					}

					// check bad parity (hi)
					if ((mem_mask & 0xff00) && BIT(m_parity_flag[offset / 32], (offset & 31) + 1))
					{
						m_ssr |= SSR_PERR;
						m_per &= ~PER_PARADR;
						m_per |= PER_HIERR | ((offset >> 9) & PER_PARADR);

						error = true;
					}

					if (error && (m_scr & SCR_NMIE))
						m_cpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
				}
			},
			[this](offs_t offset, u16 &data, u16 mem_mask)
			{
				if (m_scr & SCR_PARB)
				{
					// mark bad parity (lo)
					if ((mem_mask & 0x00ff) && !BIT(m_parity_flag[offset / 32], (offset & 31) + 0))
					{
						m_parity_flag[offset / 32] |= 1U << ((offset & 31) + 0);
						m_parity_bad++;
					}

					// mark bad parity (hi)
					if ((mem_mask & 0xff00) && !BIT(m_parity_flag[offset / 32], (offset & 31) + 1))
					{
						m_parity_flag[offset / 32] |= 1U << ((offset & 31) + 1);
						m_parity_bad++;
					}
				}
				else
				{
					// clear bad parity (lo)
					if ((mem_mask & 0x00ff) && BIT(m_parity_flag[offset / 32], (offset & 31) + 0))
					{
						m_parity_flag[offset / 32] &= ~(1U << ((offset & 31) + 0));
						m_parity_bad--;
					}

					// clear bad parity (hi)
					if ((mem_mask & 0xff00) && BIT(m_parity_flag[offset / 32], (offset & 31) + 1))
					{
						m_parity_flag[offset / 32] &= ~(1U << ((offset & 31) + 1));
						m_parity_bad--;
					}

					// stop checking parity if all clear
					if (!m_parity_bad)
					{
						m_parity_flag.reset();
						m_parity_mph->remove();
						m_parity_mph = nullptr;
					}
				}
			});
	}

	// stop checking parity if all clear
	if ((m_scr & SCR_PARB) && !(data & SCR_PARB) && m_parity_mph && !m_parity_bad)
	{
		m_parity_flag.reset();
		m_parity_mph->remove();
		m_parity_mph = nullptr;
	}

	if (data & SCR_PARE)
		m_per = 0;
	else
		m_ssr &= ~SSR_PERR;

	m_scr = data;
}

enum nov_mask : u8
{
	NOV_NDAT = 0x01, // nonvolatile memory data
	NOV_SK   = 0x02, // serial clock
	NOV_CS0  = 0x04, // chip select 0
	NOV_CS1  = 0x08, // chip select 1
	NOV_LED  = 0x10, // power-on led
	NOV_PON  = 0x20, // system power enable
	NOV_MSK  = 0x3e,
};

void tek6100_state::nov_w(u8 data)
{
	bool const cs0 = data & NOV_CS0;
	bool const cs1 = data & NOV_CS1;

	if (m_nov[0] && ((m_nmr ^ data) & NOV_CS0))
		m_nov[0]->cs_write(cs0);
	if (m_nov[1] && ((m_nmr ^ data) & NOV_CS1))
		m_nov[1]->cs_write(cs1);

	if (m_nov[0] && cs0)
	{
		m_nov[0]->di_write(bool(data & NOV_NDAT));
		m_nov[0]->clk_write(bool(data & NOV_SK));
	}

	if (m_nov[1] && cs1)
	{
		m_nov[1]->di_write(bool(data & NOV_NDAT));
		m_nov[1]->clk_write(bool(data & NOV_SK));
	}

	m_nmr = (m_nmr & ~NOV_MSK) | (data & NOV_MSK);
}

void tek6100_state::nov_do(int state)
{
	if (state)
		m_nmr |= NOV_NDAT;
	else
		m_nmr &= ~NOV_NDAT;
}

enum hcr_mask : u8
{
	HCR_HDSEL = 0x0f, // head select
	HCR_DRSEL = 0x10, // drive select
	HCR_MULT  = 0x20, // multiple sector
	HCR_BFRDY = 0x40, // buffer ready
	HCR_BCCLR = 0x80, // buffer counter clear
};

void tek6100_state::hcr_w(u8 data)
{
	m_hdc->drdy_w(m_hdd->exists() && bool(data & HCR_DRSEL));
	m_hdc->brdy_w(bool(data & HCR_BFRDY));

	if (!(data & HCR_BCCLR))
		m_hdc_ptr = 0;

	m_hdc->head_w(data & HCR_HDSEL);
	m_hdc->sc_w(bool(data & HCR_DRSEL));

	m_hcr = data;
}

template <typename T> T tek6100_state::buf_r()
{
	T data = 0;

	for (unsigned i = 0; i < sizeof(T); i++)
		data |= T(m_hdc_buf[m_hdc_ptr++ & 0xfff]) << (i * 8);

	return data;
}

template <typename T> void tek6100_state::buf_w(T data)
{
	for (unsigned i = 0; i < sizeof(T); i++)
		m_hdc_buf[m_hdc_ptr++ & 0xfff] = u8(data >> (i * 8));
}

template <unsigned ST> void tek6100_state::cpu_map(address_map &map)
{
	map(0x000000, 0x007fff).rom().region("kernel", 0);
	map(0x800000, 0x807fff).rom().region("kernel", 0);

	map(0xff0000, 0xff7fff).rw(FUNC(tek6100_state::buf_r<u16>), FUNC(tek6100_state::buf_w<u16>));

	map(0xfff000, 0xfff000).w(FUNC(tek6100_state::fcr_w));
	map(0xfff002, 0xfff002).w(FUNC(tek6100_state::gcr_w));
	map(0xfff004, 0xfff005).lw16([this](u16 data) { m_lan->ca(1); }, "lan_ca");
	map(0xfff006, 0xfff006).rw(FUNC(tek6100_state::nov_r), FUNC(tek6100_state::nov_w));
	map(0xfff008, 0xfff009).portr("config");
	map(0xfff008, 0xfff008).w(FUNC(tek6100_state::led_w));
	map(0xfff00a, 0xfff00b).lr16([this]() { return m_per; }, "per_r");
	map(0xfff00c, 0xfff00c).rw(FUNC(tek6100_state::scr_r), FUNC(tek6100_state::scr_w));
	map(0xfff00e, 0xfff00e).r(FUNC(tek6100_state::ssr_r));

	map(0xfff800, 0xfff83f).rw(m_rtc, FUNC(mm58167_device::read), FUNC(mm58167_device::write)).umask16(0xff);
	map(0xfff900, 0xfff90f).rw(m_gpib, FUNC(tms9914_device::read), FUNC(tms9914_device::write)).umask16(0xff);
	map(0xfffa00, 0xfffa07).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask16(0xff);
	map(0xfffb00, 0xfffb0f).rw(m_hdc, FUNC(wd1010_device::read), FUNC(wd1010_device::write)).umask16(0xff);
	map(0xfffb10, 0xfffb10).w(FUNC(tek6100_state::hcr_w));
	map(0xfffc00, 0xfffc07).rw(m_fdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write)).umask16(0xff);
	//map(0xfffd00, 0xfffdff); // dma controller
	map(0xfffe00, 0xfffeff).m(m_icu, FUNC(ns32202_device::map<BIT(ST, 1)>)).umask16(0xff);
	map(0xffff00, 0xffffff).rom().region("kernel", 0x7f00);
}

void tek6100_state::lan_map(address_map &map)
{
	map.global_mask(0x3fffff);

	map(0x3fff00, 0x3fffff).rom().region("kernel", 0x7f00); // 0xf00000-0xffffff
}

void tek6100_state::tek6130(machine_config &config)
{
	NS32016(config, m_cpu, 20_MHz_XTAL / 2);
	m_cpu->set_addrmap(0, &tek6100_state::cpu_map<0>);
	m_cpu->set_addrmap(6, &tek6100_state::cpu_map<6>);

	NS32081(config, m_fpu, 20_MHz_XTAL / 2);
	m_cpu->set_fpu(m_fpu);

	NS32202(config, m_icu, 20_MHz_XTAL / 1000);
	m_icu->out_int().set_inputline(m_cpu, INPUT_LINE_IRQ0).invert();
	/*
	 *  0   mmu configuration link
	 *  1 l rs-232-c
	 *  2 l hard disk interface
	 *  3 l display system
	 *  4 l backplane slot 1
	 *  5 l backplane slot 2
	 *  6 l backplane slot 3
	 *  7 l backplane slot 4
	 *  8 l backplane slot 5
	 *  9 l backplane slot 6
	 * 10 l gpib
	 * 11 h local area network
	 * 12 h power switch interrupt
	 * 13 h flexible disk interrupt
	 * 14   fpu configuration link
	 * 15   software interrupt 0
	 */

	RAM(config, m_ram);
	m_ram->set_default_size("1M");
	m_ram->set_default_value(0);

	SCC8530N(config, m_scc, 16_MHz_XTAL / 4);
	m_scc->out_int_callback().set(m_icu, FUNC(ns32202_device::ir_w<1>)).invert();
	m_scc->configure_channels(2'457'600, 0, 2'457'600, 0);

	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	m_serial[0]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));
	m_serial[0]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	m_serial[0]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	m_scc->out_dtra_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_scc->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_serial[1]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));
	m_serial[1]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	m_serial[1]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	m_scc->out_dtrb_callback().set(m_serial[1], FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));

	MM58167(config, m_rtc, 32.768_kHz_XTAL);

	EEPROM_X24C44_16BIT(config, m_nov[0]);
	m_nov[0]->do_callback().set(FUNC(tek6100_state::nov_do));
	EEPROM_X24C44_16BIT(config, m_nov[1]);
	m_nov[1]->do_callback().set(FUNC(tek6100_state::nov_do));

	WD1770(config, m_fdc, 8_MHz_XTAL); // clock?
	m_fdc->intrq_wr_callback().set(m_icu, FUNC(ns32202_device::ir_w<13>));
	//m_fdc->drq_wr_callback().set(m_dma, FUNC(::));
	FLOPPY_CONNECTOR(config, "fdc:0", "525dd", FLOPPY_525_DD, true, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	WD1010(config, m_hdc, 20_MHz_XTAL / 4);
	m_hdc->out_intrq_callback().set(m_icu, FUNC(ns32202_device::ir_w<2>));
	m_hdc->in_data_callback().set(FUNC(tek6100_state::buf_r<u8>));
	m_hdc->out_data_callback().set(FUNC(tek6100_state::buf_w<u8>));
	m_hdc->out_bcr_callback().set([this](int state) { if (state) m_hdc_ptr = 0; });

	HARDDISK(config, m_hdd, 0);

	I82586(config, m_lan, 16_MHz_XTAL / 2);
	m_lan->set_addrmap(0, &tek6100_state::lan_map);
	m_lan->out_irq_cb().set(m_icu, FUNC(ns32202_device::ir_w<11>)).invert();

	TMS9914(config, m_gpib, 20_MHz_XTAL / 4);
	m_gpib->int_write_cb().set(m_icu, FUNC(ns32202_device::ir_w<10>));
	m_gpib->dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_gpib->dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	m_gpib->eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	m_gpib->dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_gpib->nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	m_gpib->ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_gpib->ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	m_gpib->srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	m_gpib->atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	m_gpib->ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));

	ieee488_device &ieee(IEEE488(config, IEEE488_TAG));
	ieee.eoi_callback().set(m_gpib, FUNC(tms9914_device::eoi_w));
	ieee.dav_callback().set(m_gpib, FUNC(tms9914_device::dav_w));
	ieee.nrfd_callback().set(m_gpib, FUNC(tms9914_device::nrfd_w));
	ieee.ndac_callback().set(m_gpib, FUNC(tms9914_device::ndac_w));
	ieee.ifc_callback().set(m_gpib, FUNC(tms9914_device::ifc_w));
	ieee.srq_callback().set(m_gpib, FUNC(tms9914_device::srq_w));
	ieee.atn_callback().set(m_gpib, FUNC(tms9914_device::atn_w));
	ieee.ren_callback().set(m_gpib, FUNC(tms9914_device::ren_w));

	IEEE488_SLOT(config, "ieee_rem", 0, remote488_devices, nullptr);
}

static INPUT_PORTS_START(tek6100)
	PORT_START("config")
	PORT_DIPNAME(0x80, 0x00, "Mode") PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(   0x00, "Normal")
	PORT_DIPSETTING(   0x80, "Service")
	PORT_DIPNAME(0x60, 0x20, "Console") PORT_DIPLOCATION("SW:3,2")
	PORT_DIPSETTING(   0x00, "Display")
	PORT_DIPSETTING(   0x20, "9600 baud RS-232-C terminal (port 0)")
	PORT_DIPSETTING(   0x40, "1200 baud RS-232-C modem/terminal (port 1)")
	PORT_DIPSETTING(   0x60, "300 baud model/terminal (port 1)")
	PORT_DIPNAME(0x10, 0x00, "Boot") PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(   0x00, "UTek")
	PORT_DIPSETTING(   0x10, "File")
	PORT_DIPNAME(0x0c, 0x00, "Boot Device") PORT_DIPLOCATION("SW:6,5")
	PORT_DIPSETTING(   0x00, "Autoboot")
	PORT_DIPSETTING(   0x04, "Hard disk")
	PORT_DIPSETTING(   0x08, "Diskette drive")
	PORT_DIPSETTING(   0x0c, "LAN port")
	PORT_DIPNAME(0x03, 0x00, "Diagnostic") PORT_DIPLOCATION("SW:8,7")
	PORT_DIPSETTING(   0x00, "0")
	PORT_DIPSETTING(   0x01, "1")
	PORT_DIPSETTING(   0x02, "2")
	PORT_DIPSETTING(   0x03, "3")
INPUT_PORTS_END

ROM_START(tek6130)
	ROM_REGION16_LE(0x8000, "kernel", 0)
	ROM_SYSTEM_BIOS(0, "kernel", "PWR UP 5.105 ADB 20")
	ROMX_LOAD("pwr_up__5.105__adb_lo.bin", 0x00000, 0x4000, CRC(429a6a4a) SHA1(5ec5a045ec14a1b546d75e9406f0ad21163298f6), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("pwr_up__5.105__adb_hi.bin", 0x00001, 0x4000, CRC(1ad7a762) SHA1(98472c30f1148c43f43d2131ebc9644d8ebf376a), ROM_BIOS(0) | ROM_SKIP(1))
ROM_END

} // anonymous namespace

/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT         COMPANY      FULLNAME  FLAGS */
COMP(1984, tek6130, 0,      0,      tek6130, tek6100, tek6100_state, init_common, "Tektronix", "6130",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
