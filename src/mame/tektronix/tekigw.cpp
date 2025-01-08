// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Tektronix Intelligent Graphics Workstations (6110, 6120, 6130, 6205, 6210, 6212, 4132)
 *
 * Sources:
 *   - http://bitsavers.trailing-edge.com/pdf/tektronix/6130/6100_Series_Hardware_Descriptions.pdf
 *   - https://archive.org/details/tektronix_Tektronix_1985
 *
 * TODO:
 *   - slots and cards
 *   - gpib devices
 *   - graphics and keyboard
 */
/*
 * tek4132
 *  - novram chip select not asserted when trying to read ethernet mac address?
 *  - aborts to firmware monitor on second serial port after failing to boot
 *  - hangs during UTek boot
 *
booting /vmunix
<UTek>  RSA - RS-232 ports
<UTek>  RSADMA - RS-232 DMA
<UTek>  SCSI - SCSI interface
<UTek>  lna0: Ethernet= 8:0:11:0:0:f7 Ip=[42.128.0.1]
<UTek>  LNA - Local Area Network
<UTek>  CEPWR - Soft Power switch
<UTek> end configure
<UTek> Tektronix - UTek STRATOSIIW2.3 #1.25 Thu Oct 23 16:56:50 PDT 1986
<UTek>
<UTek> real mem = 1024k
<UTek> firstaddr = 0x3fda8
<UTek> firstaddr = 0x78600
<UTek> avail mem = 531456
<UTek> using 51 buffers containing 104448 bytes of memory
<UTek> inittodr: clock has gone backward, file system time used
<UTek>    file system time = 2eab0645
<UTek>    clock = 5bb2d3
 */
/*
 * tek6130
 * "Invalid menu file.  Dropping into shell.\n" - string at d789
 * accessed by printf at 7b82
 * message set at a6b0
 *
Booting from disk
Invalid menu file.  Dropping into shell.
diags>
 */

#include "emu.h"

// cpus and memory
#include "cpu/ns32000/ns32000.h"
#include "machine/ram.h"

// various hardware
#include "machine/am9516.h"
#include "machine/am9517a.h"
#include "machine/eepromser.h"
#include "machine/i82586.h"
#include "machine/mm58167.h"
#include "machine/ncr5385.h"
#include "machine/ns32081.h"
#include "machine/ns32082.h"
#include "machine/ns32202.h"
#include "machine/tms9914.h"
#include "machine/wd_fdc.h"
#include "machine/wd1010.h"
#include "machine/z80scc.h"

#include "machine/input_merger.h"

// busses and connectors
#include "bus/ieee488/ieee488.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"

// graphics
#include "cpu/mcs51/mcs51.h"
#include "video/mc6845.h"
#include "screen.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class tekigw_state_base : public driver_device
{
protected:
	tekigw_state_base(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_fpu(*this, "fpu")
		, m_mmu(*this, "mmu")
		, m_icu(*this, "icu")
		, m_ram(*this, "ram")
		, m_scc(*this, "scc")
		, m_serial(*this, "port%u", 0U)
		, m_rtc(*this, "rtc")
		, m_nov(*this, "nov%u", 0U)
		, m_dma(*this, "dma")
		, m_lan(*this, "lan")
		, m_led(*this, "led")
	{
	}

	void common_config(machine_config &config);
	void common_init();

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;
	void dma_map(address_map &map) ATTR_COLD;

	// computer board control registers
	u8 nov_r() { return m_nmr; }
	u8 scr_r() { return m_scr; }
	u8 ssr_r() { return m_ssr; }
	void led_w(u8 data) { m_led = data; }
	void nov_w(u8 data);
	void scr_w(u8 data);

	// novram data out handler
	void nov_do(int state);

	void buserror(s32 param);

	required_device<ns32016_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<ns32202_device> m_icu;
	required_device<ram_device> m_ram;

	required_device<scc8530_device> m_scc;
	required_device_array<rs232_port_device, 2> m_serial;
	required_device<mm58167_device> m_rtc;
	optional_device_array<eeprom_serial_x24c44_16bit_device, 2> m_nov;
	required_device<am9516_device> m_dma;
	required_device<i82586_device> m_lan;

	output_finder<> m_led;

	emu_timer *m_buserror = nullptr;

private:
	u8 m_nmr = 0; // nonvolatile memory register
	u16 m_per = 0; // parity error register
	u8 m_scr = 0; // system control register
	u8 m_ssr = 0; // system status register

	// ram parity state
	memory_passthrough_handler m_parity_mph;
	std::unique_ptr<u32 []> m_parity_flag;
	unsigned m_parity_bad = 0;
	bool m_parity_check = 0;
};

class tek6100_state : public tekigw_state_base
{
public:
	tek6100_state(machine_config const &mconfig, device_type type, char const *tag)
		: tekigw_state_base(mconfig, type, tag)
		, m_gpib(*this, "gpib")
		, m_fdc(*this, "fdc")
		, m_fdd(*this, "fdc:%u:525dd", 0U)
		, m_hdc(*this, "hdc")
		, m_hdd(*this, "hdc:0")
		, m_dpu_cpu(*this, "dpu_cpu")
		, m_dpu_cram(*this, "dpu_cram")
		, m_dpu_vram(*this, "dpu_vram")
	{
	}

	void tek6130(machine_config &config);
	void init() { tekigw_state_base::common_init(); }

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;
	void lan_map(address_map &map) ATTR_COLD;
	template <unsigned ST> void dpu_cpu_map(address_map &map) ATTR_COLD;

private:
	// computer board control registers
	void fcr_w(u8 data);
	void gcr_w(u8 data);
	void hcr_w(u8 data);

	// hard disk buffer handlers
	template <typename T> T buf_r();
	template <typename T> void buf_w(T data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect) { return 0; }

	required_device<tms9914_device> m_gpib;
	required_device<wd1770_device> m_fdc;
	optional_device_array<floppy_image_device, 2> m_fdd;
	required_device<wd1010_device> m_hdc;
	required_device<harddisk_image_device> m_hdd;

	optional_device<ns32016_device> m_dpu_cpu;
	optional_device<ram_device> m_dpu_cram;
	optional_device<ram_device> m_dpu_vram;

	u8 m_hcr = 0; // hard disk control register

	// hard disk controller buffer
	u16 m_hdc_ptr = 0;
	std::unique_ptr<u8[]> m_hdc_buf;
};

class tek4132_state : public tekigw_state_base
{
public:
	tek4132_state(machine_config const &mconfig, device_type type, char const *tag)
		: tekigw_state_base(mconfig, type, tag)
		, m_scsibus(*this, "scsi")
		, m_scsi(*this, "scsi:7:ncr5385")
		, m_sdma(*this, "sdma")
		, m_sirq(*this, "sirq")
	{
	}

	void tek4132(machine_config &config);
	void init() { tekigw_state_base::common_init(); }

protected:
	// driver_device overrides
	//virtual void machine_start() override ATTR_COLD;
	//virtual void machine_reset() override ATTR_COLD;

	// address maps
	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;
	void lan_map(address_map &map) ATTR_COLD;

private:
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr5385_device> m_scsi;
	required_device<am9517a_device> m_sdma;

	required_device<input_merger_all_high_device> m_sirq;
};

void tekigw_state_base::machine_start()
{
	save_item(NAME(m_nmr));
	save_item(NAME(m_per));
	save_item(NAME(m_scr));
	save_item(NAME(m_ssr));

	m_per = 0;
	m_ssr = 0;

	m_parity_check = false;

	m_buserror = timer_alloc(FUNC(tek4132_state::buserror), this);
}

void tekigw_state_base::machine_reset()
{
	nov_w(0);
	scr_w(0);

	m_icu->g_w<0>(1); // mmu present
	m_icu->g_w<7>(1); // fpu present
	m_icu->ir_w<12>(0);
}

void tek6100_state::machine_start()
{
	tekigw_state_base::machine_start();

	m_hdc_buf = std::make_unique<u8[]>(4096);

	save_item(NAME(m_hcr));

	save_item(NAME(m_hdc_ptr));
	save_pointer(NAME(m_hdc_buf), 4096);
}

void tek6100_state::machine_reset()
{
	tekigw_state_base::machine_reset();

	hcr_w(0);
	fcr_w(0);
}

void tekigw_state_base::common_init()
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

void tekigw_state_base::scr_w(u8 data)
{
	LOG("scr_w 0x%02x (%s)\n", data, machine().describe_context());

	if (!(m_scr & SCR_RAME) && (data & SCR_RAME))
		m_cpu->space(0).install_ram(0, m_ram->mask(), m_ram->pointer());

	if ((m_scr ^ data) & SCR_LLOP)
		m_lan->set_loopback(data & SCR_LLOP);

	if (m_scr & SCR_BERRE && !(data & SCR_BERRE))
		m_ssr &= ~(SSR_BERR | SSR_DMACH);

	// install parity handlers
	if (!(m_scr & SCR_PARB) && (data & SCR_PARB) && !m_parity_check)
	{
		m_parity_flag = std::make_unique<u32[]>(m_ram->size() / 32);
		m_parity_bad = 0;
		m_parity_check = true;

		m_parity_mph = m_cpu->space(0).install_readwrite_tap(
				0, m_ram->mask(),
				"parity",
				[this] (offs_t offset, u16 &data, u16 mem_mask)
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
				[this] (offs_t offset, u16 &data, u16 mem_mask)
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
							m_parity_mph.remove();
							m_parity_check = false;
						}
					}
				},
				&m_parity_mph);
	}

	// stop checking parity if all clear
	if ((m_scr & SCR_PARB) && !(data & SCR_PARB) && m_parity_check && !m_parity_bad)
	{
		m_parity_flag.reset();
		m_parity_mph.remove();
		m_parity_check = false;
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

void tekigw_state_base::nov_w(u8 data)
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

void tekigw_state_base::nov_do(int state)
{
	if (state)
		m_nmr |= NOV_NDAT;
	else
		m_nmr &= ~NOV_NDAT;
}

void tekigw_state_base::buserror(s32 param)
{
	if (m_scr & SCR_BERRE)
	{
		m_ssr |= SSR_BERR | (param & SSR_DMACH);

		if (m_scr & SCR_NMIE)
			m_cpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
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

	m_hdc->head_w(data & 7);
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

template <unsigned ST> void tekigw_state_base::cpu_map(address_map &map)
{
	map(0xfff004, 0xfff005).lw16([this](u16 data) { m_lan->ca(1); }, "lan_ca");
	map(0xfff006, 0xfff006).rw(FUNC(tekigw_state_base::nov_r), FUNC(tekigw_state_base::nov_w));
	map(0xfff008, 0xfff009).portr("config");
	map(0xfff008, 0xfff008).w(FUNC(tekigw_state_base::led_w));
	map(0xfff00a, 0xfff00b).lr16([this]() { return m_per; }, "per_r");
	map(0xfff00c, 0xfff00c).rw(FUNC(tekigw_state_base::scr_r), FUNC(tekigw_state_base::scr_w));
	map(0xfff00e, 0xfff00e).r(FUNC(tekigw_state_base::ssr_r));
	map(0xfff00f, 0xfff00f).lr8([]() { return 7; }, "mem_size"); // code at 804703 uses this to size memory

	map(0xfff800, 0xfff83f).rw(m_rtc, FUNC(mm58167_device::read), FUNC(mm58167_device::write)).umask16(0xff);
	map(0xfffa00, 0xfffa07).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask16(0xff);
	//map(0xfffd00, 0xfffdff); // dma controller
	map(0xfffe00, 0xfffeff).m(m_icu, FUNC(ns32202_device::map<BIT(ST, 1)>)).umask16(0xff);
}

template <unsigned ST> void tek6100_state::cpu_map(address_map &map)
{
	tekigw_state_base::cpu_map<ST>(map);

	map(0x000000, 0x007fff).rom().region("kernel", 0);

	//map(0x300000, 0x7fffff); // external bus
	map(0x800000, 0x807fff).rom().region("kernel", 0);
	//map(0x820000, 0xfdffff); // external bus
	map(0xfe0000, 0xfeffff).lr8(
		[this](address_space &space)
		{
			// FIXME: Should be 20μs according to 6100 documentation; CPU is still
			// too fast, and timing may differ on 4132. Also DMACH field should be
			// 7 for CPU, but diagnostic expects F; maybe 4132 is different?
			m_buserror->adjust(attotime::from_ticks(10, m_cpu->clock()), 0x7);

			return space.unmap();
		}, "buserror"); // expansion I/O

	map(0xff0000, 0xff7fff).rw(FUNC(tek6100_state::buf_r<u16>), FUNC(tek6100_state::buf_w<u16>));

	map(0xfff000, 0xfff000).w(FUNC(tek6100_state::fcr_w));
	map(0xfff002, 0xfff002).w(FUNC(tek6100_state::gcr_w));

	map(0xfff900, 0xfff90f).rw(m_gpib, FUNC(tms9914_device::read), FUNC(tms9914_device::write)).umask16(0xff);
	map(0xfffb00, 0xfffb0f).rw(m_hdc, FUNC(wd1010_device::read), FUNC(wd1010_device::write)).umask16(0xff);
	map(0xfffb10, 0xfffb10).w(FUNC(tek6100_state::hcr_w));
	map(0xfffc00, 0xfffc07).rw(m_fdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write)).umask16(0xff);
	map(0xffff00, 0xffffff).rom().region("kernel", 0x7f00);
}

void tek6100_state::lan_map(address_map &map)
{
	map.global_mask(0x3fffff);

	map(0x3fff00, 0x3fffff).rom().region("kernel", 0x7f00); // 0xf00000-0xffffff
}

template <unsigned ST> void tek6100_state::dpu_cpu_map(address_map &map)
{
	map(0x000000, 0x007fff).rom().region("dpu", 0);

	map(0x100000, 0x107fff).rom().region("dpu", 0);


	map(0x20000e, 0x20000e).lr8([]() { return 0x20; }, "fpu_present");

	//map(0x200026, 0x200026);
	map(0x200028, 0x200028).lw8(
		[this](u8 data)
		{
			if (!BIT(data, 7) && m_dpu_cpu)
			{
				m_dpu_cpu->space(0).install_ram(0, m_dpu_cram->mask(), m_dpu_cram->pointer());
				m_dpu_cpu->space(0).install_ram(0x400000, 0x400000 | m_dpu_vram->mask(), m_dpu_vram->pointer());
			}
		}, "ram_enable");
	//map(0x20002a, 0x20002a);
	//map(0x20002c, 0x20002c);

	//map(0x300000, 0x300000); // PUbram - pattern ram
	map(0xc00000, 0xc0000f).ram(); // PUpixm1 (1/2 bit x 4 planes)
}

enum csr_mask : u8
{
	CSR_IDENT  = 0x07, // scsi bus id (inverted)
	CSR_RESET  = 0x08, // scsi bus reset
	CSR_FEOP   = 0x10, // dma force eop
	CSR_IENBL  = 0x20, // scsi interrupt enable
	CSR_DIPEND = 0x40, // dma interrupt pending
	CSR_SIPEND = 0x80, // scsi interrupt pending
};

template <unsigned ST> void tek4132_state::cpu_map(address_map &map)
{
	map(0x000000, 0xffffff).lr8(
		[this](address_space &space, offs_t offset)
		{
			if (!machine().side_effects_disabled())
			{
				// FIXME: Should be 20μs according to 6100 documentation; CPU is still
				// too fast, and timing may differ on 4132. Also DMACH field should be
				// 7 for CPU, but diagnostic expects F; maybe 4132 is different?
				m_buserror->adjust(attotime::from_ticks(20, m_cpu->clock()), 0xf);
				logerror("buserror 0x%06x (%s)\n", offset, machine().describe_context());
			}
			return space.unmap();
		}, "buserror");

	tekigw_state_base::cpu_map<ST>(map);

	map(0x000000, 0x00ffff).rom().region("kernel", 0);
	map(0x800000, 0x80ffff).rom().region("kernel", 0);

	//map(0xfff000, 0xfff000).w(FUNC(tek6100_state::fcr_w)); // rsaa_dmaaos serial dma address offset register
	//map(0xfff002, 0xfff002).w(FUNC(tek6100_state::gcr_w)); // rsaa_auxlatch serial auxiliary control register
	//map(0xfff008, 0xfff008).w(); // rsaa_irqreset
	//map(0xfff00f, 0xfff00f).lr8([]() { return 0x7; }, "memsize"); // code at 804703 uses this to size memory

	map(0xfff900, 0xfff91f).rw(m_sdma, FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0xff);
	//map(0xfffa08, 0xfffa08); // rsaa_status

	//map(0xfffc00, 0xfffc1f); // signature
	map(0xfffc20, 0xfffc3f).lw8(
		[this](offs_t offset, u8 data)
		{
			LOG("csr_w 0x%02x offset 0x%x (%s)\n", data, offset, machine().describe_context());

			m_scsi->set_own_id(~data & CSR_IDENT);
			m_sirq->in_w<0>(bool(data & CSR_IENBL));
		}, "csr_w");
	map(0xfffc40, 0xfffc5f).m(m_scsi, FUNC(ncr5385_device::map)).umask16(0xff);
	map(0xfffc60, 0xfffc61).rw(m_dma, FUNC(am9516_device::data_r), FUNC(am9516_device::data_w));
	map(0xfffc62, 0xfffc63).rw(m_dma, FUNC(am9516_device::addr_r), FUNC(am9516_device::addr_w));
	//map(0xfffd00, 0xfffdff).ram(); // stub out 9516

	map(0xffff00, 0xffffff).rom().region("kernel", 0xff00);
}

void tek4132_state::lan_map(address_map &map)
{
	map.global_mask(0x3fffff);

	map(0x3fff00, 0x3fffff).rom().region("kernel", 0xff00); // 0xf00000-0xffffff
}

void tekigw_state_base::dma_map(address_map &map)
{
	map(0x000000, 0xffffff).lrw16(
		[this](offs_t offset, u16 mem_mask)
		{
			return (mem_mask == 0xffff)
				? m_cpu->space(0).read_word(offset << 1)
				: swapendian_int16(m_cpu->space(0).read_word(offset << 1, swapendian_int16(mem_mask)));
		}, "dma_r",
		[this](offs_t offset, u16 data, u16 mem_mask)
		{
			if (mem_mask == 0xffff)
				m_cpu->space(0).write_word(offset << 1, data, mem_mask);
			else
				m_cpu->space(0).write_word(offset << 1, swapendian_int16(data), swapendian_int16(mem_mask));
		}, "dma_w");
}

void tekigw_state_base::common_config(machine_config &config)
{
	NS32016(config, m_cpu, 20_MHz_XTAL / 2);

	NS32081(config, m_fpu, 20_MHz_XTAL / 2);
	m_cpu->set_fpu(m_fpu);

	NS32082(config, m_mmu, 20_MHz_XTAL / 2);
	m_cpu->set_mmu(m_mmu);

	NS32202(config, m_icu, 20_MHz_XTAL / 1000);
	m_icu->out_int().set_inputline(m_cpu, INPUT_LINE_IRQ0).invert();

	RAM(config, m_ram);
	m_ram->set_default_size("1M");

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

	/*
	 * non-volatile memory: xx yy mm mm mm mm mm mm ii ii ii ii
	 *
	 * where (bit order reversed):
	 *   xx yy - checksum?
	 *   mm    - mac address
	 *   ii    - IP address
	 */
	MM58167(config, m_rtc, 32.768_kHz_XTAL);

	EEPROM_X24C44_16BIT(config, m_nov[0]); // X2443P
	m_nov[0]->do_callback().set(FUNC(tekigw_state_base::nov_do));

	AM9516(config, m_dma, 16_MHz_XTAL / 4); // TODO: 4132 has Am9516A-8
	m_dma->set_addrmap(am9516_device::SYSTEM_MEM, &tekigw_state_base::dma_map);
	m_dma->set_addrmap(am9516_device::NORMAL_MEM, &tekigw_state_base::dma_map);

	// TODO: channel B fdc

	I82586(config, m_lan, 16_MHz_XTAL / 2);
	m_lan->out_irq_cb().set(m_icu, FUNC(ns32202_device::ir_w<11>));
}

void tek6100_state::tek6130(machine_config &config)
{
	tekigw_state_base::common_config(config);

	m_cpu->set_addrmap(0, &tek6100_state::cpu_map<0>);
	m_cpu->set_addrmap(ns32000::ST_EIM, &tek6100_state::cpu_map<ns32000::ST_EIM>);

	m_lan->set_addrmap(0, &tek6100_state::lan_map);

	/*
	 * Interrupt sources
	 *  0 interval clock
	 *  1 serial
	 *  2 hard disk
	 *  3 display
	 *  4 backplane slot 1
	 *  5 backplane slot 2
	 *  6 backplane slot 3
	 *  7 backplane slot 4
	 *  8 backplane slot 5
	 *  9 backplane slot 6
	 * 10 gpib
	 * 11 local area network
	 * 12 power switch
	 * 13 flexible disk
	 * 14 network level
	 * 15 software clock
	 */

	WD1770(config, m_fdc, 8_MHz_XTAL); // clock?
	m_fdc->intrq_wr_callback().set(m_icu, FUNC(ns32202_device::ir_w<13>));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(am9516_device::dreq_w<1>));
	FLOPPY_CONNECTOR(config, "fdc:0", "525dd", FLOPPY_525_DD, true, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	WD1010(config, m_hdc, 20_MHz_XTAL / 4);
	m_hdc->out_intrq_callback().set(m_icu, FUNC(ns32202_device::ir_w<2>));
	m_hdc->in_data_callback().set(FUNC(tek6100_state::buf_r<u8>));
	m_hdc->out_data_callback().set(FUNC(tek6100_state::buf_w<u8>));
	m_hdc->out_bcr_callback().set([this](int state) { if (state) m_hdc_ptr = 0; });

	/*
	 * Supported hard drives (c,h,s,bps):
	 *  - Micropolis 1302 (830,3,16,512)
	 *  - Micropolis 1304 (830,6,16,512)
	 *  - Maxtor XT-1105 (918,11,16,512)
	 */
	HARDDISK(config, m_hdd, 0);

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

	// TODO: graphics board disabled for now
	if (false)
	{
		NS32016(config, m_dpu_cpu, 0); // 8'000'000);
		m_dpu_cpu->set_addrmap(0, &tek6100_state::dpu_cpu_map<0>);

		ns32081_device &dpu_fpu(NS32081(config, "dpu_fpu", 8'000'000));
		m_dpu_cpu->set_fpu(dpu_fpu);

		ns32202_device &dpu_icu(NS32202(config, "dpu_icu", 20'000));
		dpu_icu.out_int().set_inputline(m_dpu_cpu, INPUT_LINE_IRQ0).invert();

		i8744_device &dpu_mcu(I8744(config, "dpu_mcu", 0)); // 10'000'000)); // 8744H-10
		(void)dpu_mcu;

		mc6845_device &dpu_crtc(SY6845E(config, "dpu_crtc", 25.2_MHz_XTAL)); // SYP6845EA
		dpu_crtc.set_screen("dpu_screen");

		// 32 x HM50256-15 (256K x 1 DRAM) total 1MiB
		RAM(config, m_dpu_cram);
		m_dpu_cram->set_default_size("1MiB");

		// 32 x IMS2620P-12 (16K x 4 DRAM) total 256KiB
		RAM(config, m_dpu_vram);
		m_dpu_vram->set_default_size("256KiB");

		// 13-inch 640x480 60Hz 4-bit color
		// 15-inch 640x480 60Hz monochrome
		screen_device &dpu_screen(SCREEN(config, "dpu_screen", SCREEN_TYPE_RASTER));
		dpu_screen.set_raw(25200000, 800, 0, 640, 525, 0, 480);
		dpu_screen.set_screen_update(FUNC(tek6100_state::screen_update));
	}
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
}

void tek4132_state::tek4132(machine_config &config)
{
	tekigw_state_base::common_config(config);

	m_cpu->set_addrmap(0, &tek4132_state::cpu_map<0>);
	m_cpu->set_addrmap(ns32000::ST_EIM, &tek4132_state::cpu_map<ns32000::ST_EIM>);
	//m_cpu->set_addrmap(ns32000::ST_ODT, &tek4132_state::cpu_map<ns32000::ST_ODT>);

	m_lan->set_addrmap(0, &tek4132_state::lan_map);

	m_scc->configure_channels(3'686'400, 0, 3'686'400, 0);

	/*
	 * Interrupt sources
	 *  0 interval clock
	 *  1 serial
	 *  2 display
	 *  3 serial dma
	 *  4 scsi controller
	 *  5 backplane slot 1
	 *  6 backplane slot 2
	 *  7 backplane slot 3
	 *  8 backplane slot 4
	 *  9 backplane slot 5
	 * 10 backplane slot 6
	 * 11 local area network
	 * 12 power switch interrupt
	 * 13 time of day clock
	 * 14 network level
	 * 15 software clock
	 */

	m_rtc->irq().set(m_icu, FUNC(ns32202_device::ir_w<13>)).invert(); // device emulation has inverted polarity

	EEPROM_X24C44_16BIT(config, m_nov[1]); // X2443P
	m_nov[1]->do_callback().set(FUNC(tek4132_state::nov_do));

	AM9517A(config, m_sdma, 4'000'000); // D8237A-5 (clock?)
	m_sdma->dreq_active_low();

	INPUT_MERGER_ALL_HIGH(config, m_sirq);
	//m_sirq->output_handler().set(m_icu, FUNC(ns32202_device::ir_w<4>));

	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5385", NCR5385).clock(10'000'000).machine_config(
		[this](device_t *device)
		{
			ncr5385_device &adapter = downcast<ncr5385_device &>(*device);

			//adapter.irq().set(m_sirq, FUNC(input_merger_all_high_device::in_w<1>));
			adapter.irq().set(m_icu, FUNC(ns32202_device::ir_w<4>));
			adapter.dreq().set(m_dma, FUNC(am9516_device::dreq_w<0>)).invert();

		});

	m_dma->flyby_byte_r<0>().set(":scsi:7:ncr5385", FUNC(ncr5385_device::dma_r));
	m_dma->flyby_byte_w<0>().set(":scsi:7:ncr5385", FUNC(ncr5385_device::dma_w));
}

static INPUT_PORTS_START(tekigw)
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
	ROM_SYSTEM_BIOS(0, "5.107", "5.107 85/02/20 13:51:51")
	ROMX_LOAD("160-2921-02.bin", 0x00000, 0x4000, CRC(44688258) SHA1(645ceaa6ea9ef76ba8b557dec0387fce73f669c0), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("160-2922-02.bin", 0x00001, 0x4000, CRC(5d84dff3) SHA1(08e74f0157a5c002397b3c8644613dc0ae8355a0), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(1, "5.105", "5.105 85/02/16 11:03:32")
	ROMX_LOAD("pwr_up__5.105__adb_lo.bin", 0x00000, 0x4000, CRC(429a6a4a) SHA1(5ec5a045ec14a1b546d75e9406f0ad21163298f6), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("pwr_up__5.105__adb_hi.bin", 0x00001, 0x4000, CRC(1ad7a762) SHA1(98472c30f1148c43f43d2131ebc9644d8ebf376a), ROM_BIOS(1) | ROM_SKIP(1))

	ROM_REGION16_LE(0x8000, "dpu", 0)
	ROMX_LOAD("l__sdpu__1.73.bin", 0x00000, 0x4000, CRC(cd1be127) SHA1(9c16ab7e8b195e9ae660288b4b26566581a029e5), ROM_SKIP(1))
	ROMX_LOAD("h__sdpu__1.73.bin", 0x00001, 0x4000, CRC(1adf3130) SHA1(966f839de0b10aff3ffc6af64d94e4b19ece52c7), ROM_SKIP(1))

	ROM_REGION(0x1000, "dpu_mcu", 0)
	ROM_LOAD("2960-01.bin", 0x0, 0x1000, CRC(5974eaec) SHA1(362cdfb7be162636429d614807b9066ca8bbab28))
ROM_END

ROM_START(tek4132)
	ROM_REGION16_LE(0x10000, "kernel", 0)
	ROM_SYSTEM_BIOS(0, "1.7", "5.107 85/02/20 13:51:51")
	ROMX_LOAD("160-4018-00__v1.7.u3130", 0x00000, 0x8000, CRC(cdf154ab) SHA1(381d012b0ec85db03d8cb164ef22f92e33d40b57), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("160-4019-00__v1.7.u3140", 0x00001, 0x8000, CRC(5fe870b1) SHA1(369507763e37e947bbbfde080ba3d83bb6f3a7bf), ROM_BIOS(0) | ROM_SKIP(1))
ROM_END
} // anonymous namespace

/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS          INIT  COMPANY      FULLNAME  FLAGS */
COMP(1984, tek6130, 0,      0,      tek6130, tekigw, tek6100_state, init, "Tektronix", "6130",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP(1984, tek4132, 0,      0,      tek4132, tekigw, tek4132_state, init, "Tektronix", "4132",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
