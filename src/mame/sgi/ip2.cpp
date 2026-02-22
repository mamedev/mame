// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * SGI IRIS IP2 processor board (used in IRIS 2[345]00T and 3xxx series systems).
 *
 * Sources:
 *  - PCB Schematic, IP2 (Drawing Number 5000-558, rev C, 14 Nov 1985), Silicon Graphics Inc.
 *
 * TODO:
 *  - segment protection and limits
 *  - mouse
 *  - Multibus protection
 *  - slave mode and external interrupt
 *  - parity checking
 */

#include "emu.h"
#include "ip2.h"
#include "iris_kbd.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68020.h"
#include "machine/input_merger.h"
#include "machine/mc146818.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"

#define LOG_MULTIBUS (1U << 1)
//#define VERBOSE (LOG_GENERAL|LOG_MULTIBUS)

#include "logmacro.h"

namespace {

enum
{
	MOUSE_BUTTON_RIGHT      = 0x01,
	MOUSE_BUTTON_MIDDLE     = 0x02,
	MOUSE_BUTTON_LEFT       = 0x04,
	BOARD_REVB              = 0x00, // higher revisions (N << 5)
	BOARD_REVA              = 0x10,

	MOUSE_XFIRE     = 0x01, // X Quadrature Fired, active low
	MOUSE_XCHANGE   = 0x02, // MOUSE_XCHANGE ? x-- : x++
	MOUSE_YFIRE     = 0x04, // Y Quadrature Fired, active low
	MOUSE_YCHANGE   = 0x08, // MOUSE_YCHANGE ? y-- : y++

	PAR_UR      = 0x01, // Check parity on user-mode reads
	PAR_UW      = 0x02, // Check parity on user-mode writes
	PAR_KR      = 0x04, // Check parity on kernel-mode reads
	PAR_KW      = 0x08, // Check parity on kernel-mode writes
	PAR_DIS0    = 0x10, // Disable access to DUART0 and LEDs
	PAR_DIS1    = 0x20, // Disable access to DUART1
	PAR_MBR     = 0x40, // Check parity on multibus reads
	PAR_MBW     = 0x80, // Check parity on multibus writes

	MBP_DCACC   = 0x01, // Display controller access (I/O page 4)
	MBP_UCACC   = 0x02, // Update controller access (I/O page 3)
	MBP_GFACC   = 0x04, // Allow GF access (I/O page 1)
	MBP_DMACC   = 0x08, // Allow GL2 DMA access (0x8nnnnn - x0bnnnnn)
	MBP_LIOACC  = 0x10, // Allow lower I/O access (0x0nnnnn - 0x7nnnnn)
	MBP_HIOACC  = 0x20, // Allow upper I/O access (0x8nnnnn - 0xfnnnnn)
	MBP_LMACC   = 0x40, // Allow lower memory access (0x0nnnnn - 0x7nnnnn)
	MBP_HMACC   = 0x80, // Allow upper memory access (0x8nnnnn - 0xfnnnnn)

};

enum switch_mask : u16
{
	SW_BTTYPE  = 0x000F, // boot type
	SW_AUTOBT  = 0x0010, // autoboot
	SW_QUIET   = 0x0020, // quiet boot
	SW_2DIS    = 0x0040, // secondary display
	SW_DISPLAY = 0x0700, // display type
	SW_CONSSPD = 0x1800, // console speed
	SW_SLAVE   = 0x8000, // master/slave
};

enum status_mask : u16
{
	ST_LEDS      = 0x000f,
	ST_ENABEXT   = 0x0010, // enable external interrupt
	ST_ENABINT   = 0x0020, // enable interrupts
	ST_BINIT     = 0x0040, // Multibus init
	ST_BOOT_     = 0x0080, // system segment access only
	ST_USERFPA   = 0x0100, // user access to FPA
	ST_USERGE    = 0x0200, // user access to GE
	ST_SLAVE     = 0x0400, // Multibus access to IP2
	ST_ENABCBRQ  = 0x0800, // hold bus until CBRQ
	ST_GEMASTER_ = 0x1000, // master of pipe
	ST_GENBAD    = 0x2000, // generate incorrect parity
	ST_ENABWDOG  = 0x4000, // enable watchdog timeout
	ST_QUICKTOUT = 0x8000, // enable quick timeout

};
enum page_mask : u32
{
	PAGE_PFNUM = 0x0000'1fff,
	PAGE_P     = 0x3000'0000, // protection
	PAGE_PN    = 0x0000'0000, //  no access
	PAGE_PR    = 0x1000'0000, //  read only
	PAGE_PS    = 0x2000'0000, //  supervisor only
	PAGE_PRW   = 0x3000'0000, //  read/write
	PAGE_R     = 0x4000'0000, // referenced
	PAGE_M     = 0x8000'0000, // modified

	PAGE_ALL   = 0xf000'1fff,
};
enum rtc_ctrl_mask : u8
{
	RTC_AS = 0x01, // address strobe
	RTC_DS = 0x02, // data strobe
	RTC_RE = 0x04, // read enable
	RTC_CE = 0x08, // clock enable
};

class sgi_ip2_device
	: public device_t
	, public device_multibus_interface
{
public:
	sgi_ip2_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SGI_IP2, tag, owner, clock)
		, device_multibus_interface(mconfig, *this)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram", 0x40'0000, ENDIANNESS_BIG)
		, m_vector(*this, "vector")
		, m_duart(*this, "duart%u", 0U)
		, m_port(*this, "port%u", 1U)
		, m_rtc(*this, "rtc")
		, m_nvram(*this, "nvram")
		, m_boot(*this, "boot")
		, m_slave(*this, "slave")
		, m_switch(*this, "SWITCH")
		, m_mem(nullptr)
		, m_page(nullptr)
		, m_map(nullptr)
		, m_base{}
		, m_limit{}
		, m_installed(false)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;
	void sys_map(address_map &map) ATTR_COLD;
	void cpu_map(address_map &map) ATTR_COLD;

	// address translation and memory access
	template <unsigned Segment> u16 mapa(offs_t offset) const;
	template <unsigned Segment> u32 ram_r(offs_t offset, u32 mem_mask);
	template <unsigned Segment> void ram_w(offs_t offset, u32 data, u32 mem_mask);

	// various registers
	u8 mbtn_r();
	u16 mloc_r();
	u8 rtc_ctrl_r();
	void rtc_ctrl_w(u8 data);
	u8 rtc_data_r();
	void rtc_data_w(u8 data);
	u8 kbase_r();
	void kbase_w(u8 data);
	u16 status_r();
	void status_w(u16 data);
	u8 pctrl_r();
	void pctrl_w(u8 data);
	u8 mbprot_r();
	void mbprot_w(u8 data);

	// page table
	u32 page_r(offs_t offset);
	void page_w(offs_t offset, u32 data, u32 mem_mask);

	// segment base/limit registers
	template <unsigned Segment> u16 base_r();
	template <unsigned Segment> void base_w(u16 data);
	template <unsigned Segment> u16 limit_r();
	template <unsigned Segment> void limit_w(u16 data);

	// Multibus access
	u16 mem_r(offs_t offset, u16 mem_mask);
	void mem_w(offs_t offset, u16 data, u16 mem_mask);
	u16 map_r(offs_t offset, u16 mem_mask);
	void map_w(offs_t offset, u16 data, u16 mem_mask);

	// other helpers
	void bus_error(u32 address, bool read, bool retry);
	template <unsigned I> void int_w(int state);

private:
	required_device<m68020_device> m_cpu;
	memory_share_creator<u32> m_ram;
	required_region_ptr<u8> m_vector;
	required_device_array<mc68681_device, 2> m_duart;
	required_device_array<rs232_port_device, 4> m_port;
	required_device<mc146818_device> m_rtc;
	required_device<nvram_device> m_nvram;

	memory_view m_boot;
	memory_view m_slave;

	required_ioport m_switch;

	memory_access<24, 1, 0, ENDIANNESS_LITTLE>::specific m_bus_mem;
	memory_access<16, 1, 0, ENDIANNESS_LITTLE>::specific m_bus_pio;
	util::endian_cast<u32, u16, util::endianness::big> m_mem;

	std::unique_ptr<u32[]> m_page;
	std::unique_ptr<u16[]> m_map;

	u8 m_mbtn;     // mouse buttons
	u16 m_mloc;    // mouse location
	u8 m_rtc_ctrl;
	u8 m_rtc_addr;
	u16 m_status;
	u8 m_pctrl;    // parity control
	u8 m_mbprot;   // Multibus protection
	u16 m_base[3];
	u16 m_limit[3];

	u8 m_int;
	bool m_installed;
};

void keyboard_devices(device_slot_interface &device)
{
	device.option_add("kbd", IRIS_KBD);
}

void sgi_ip2_device::device_add_mconfig(machine_config &config)
{
	M68020(config, m_cpu, 32_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &sgi_ip2_device::mem_map);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &sgi_ip2_device::cpu_map);

	// irq1: multibus 0, multibus 1
	input_merger_any_low_device &irq1(INPUT_MERGER_ANY_LOW(config, "irq1"));
	irq1.output_handler().set_inputline(m_cpu, INPUT_LINE_IRQ1);

	// irq6: multibus 6, uart0, uart1, ext, rtc
	input_merger_any_low_device &irq6(INPUT_MERGER_ANY_LOW(config, "irq6"));
	irq6.output_handler().set_inputline(m_cpu, INPUT_LINE_IRQ6);

	// irq7: multibus 7, parity, mouse, mouse unplug
	input_merger_any_low_device &irq7(INPUT_MERGER_ANY_LOW(config, "irq7"));
	irq7.output_handler().set_inputline(m_cpu, INPUT_LINE_IRQ7);

	// Multibus interrupts
	int_callback<0>().set(irq1, FUNC(input_merger_any_low_device::in_w<0>));
	int_callback<1>().set(irq1, FUNC(input_merger_any_low_device::in_w<1>));
	int_callback<2>().set_inputline(m_cpu, INPUT_LINE_IRQ2).invert();
	int_callback<3>().set_inputline(m_cpu, INPUT_LINE_IRQ3).invert();
	int_callback<4>().set_inputline(m_cpu, INPUT_LINE_IRQ4).invert();
	int_callback<5>().set_inputline(m_cpu, INPUT_LINE_IRQ5).invert();
	int_callback<6>().set(irq6, FUNC(input_merger_any_low_device::in_w<4>));
	int_callback<7>().set(irq7, FUNC(input_merger_any_low_device::in_w<0>));

	MC68681(config, m_duart[0], 3.6864_MHz_XTAL);
	m_duart[0]->irq_cb().set(FUNC(sgi_ip2_device::int_w<0>)).invert(); // FIXME: active low
	m_duart[0]->irq_cb().append(irq6, FUNC(input_merger_any_low_device::in_w<0>)).invert();

	MC68681(config, m_duart[1], 3.6864_MHz_XTAL);
	m_duart[1]->irq_cb().set(FUNC(sgi_ip2_device::int_w<1>)).invert(); // FIXME: active low
	m_duart[1]->irq_cb().append(irq6, FUNC(input_merger_any_low_device::in_w<1>)).invert();

	RS232_PORT(config, m_port[0], keyboard_devices, nullptr);
	RS232_PORT(config, m_port[1], default_rs232_devices, "terminal");
	RS232_PORT(config, m_port[2], default_rs232_devices, nullptr);
	RS232_PORT(config, m_port[3], default_rs232_devices, nullptr);

	m_duart[0]->a_tx_cb().set(m_port[0], FUNC(rs232_port_device::write_txd));
	m_duart[0]->b_tx_cb().set(m_port[1], FUNC(rs232_port_device::write_txd));
	m_duart[1]->a_tx_cb().set(m_port[2], FUNC(rs232_port_device::write_txd));
	m_duart[1]->b_tx_cb().set(m_port[3], FUNC(rs232_port_device::write_txd));

	m_port[0]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_a_w));
	m_port[1]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_b_w));
	m_port[2]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_a_w));
	m_port[3]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_b_w));

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(FUNC(sgi_ip2_device::int_w<3>)).invert(); // FIXME: active low
	m_rtc->irq().append(irq6, FUNC(input_merger_any_low_device::in_w<3>)).invert();

	NVRAM(config, m_nvram); // AM2148-55DC x 4 (1024x4 SRAM)
}

void sgi_ip2_device::device_start()
{
	m_bus->space(AS_PROGRAM).specific(m_bus_mem);
	m_bus->space(AS_IO).specific(m_bus_pio);

	m_mem = util::big_endian_cast<u16>(m_ram.target());

	m_page = std::make_unique<u32[]>(16384); // AM2167-35PC x 17 (16384x1 SRAM)
	m_map = std::make_unique<u16[]>(1024); // AM2148-55DC 1Kx4 SRAM (x4)

	save_pointer(NAME(m_page), 16384);
	save_pointer(NAME(m_map), 1024);

	save_item(NAME(m_mbtn));
	save_item(NAME(m_mloc));
	save_item(NAME(m_rtc_ctrl));
	save_item(NAME(m_rtc_addr));
	save_item(NAME(m_status));
	save_item(NAME(m_pctrl));
	save_item(NAME(m_mbprot));
	save_item(NAME(m_base));
	save_item(NAME(m_limit));
	save_item(NAME(m_int));

	m_mbtn = 0;
}

void sgi_ip2_device::device_reset()
{
	if (!m_installed)
	{
		offs_t const base = (m_switch->read() & SW_SLAVE) ? 0x20'0000U : 0x00'0000U;

		m_bus->space(AS_PROGRAM).install_view(base + 0x00'0000, base + 0x1f'ffff, m_slave);

		m_slave[0].install_readwrite_handler(base + 0x00'0000, base + 0x0f'ffff,
			emu::rw_delegate(*this, FUNC(sgi_ip2_device::mem_r)),
			emu::rw_delegate(*this, FUNC(sgi_ip2_device::mem_w)));
		m_slave[0].install_readwrite_handler(base + 0x10'0000, base + 0x1f'ffff,
			emu::rw_delegate(*this, FUNC(sgi_ip2_device::map_r)),
			emu::rw_delegate(*this, FUNC(sgi_ip2_device::map_w)));

		m_installed = true;
	}

	m_rtc_ctrl = 0;
	m_status = 0;
	m_int = 0x1f;

	m_boot.select(0);
	m_slave.disable();
}

void sgi_ip2_device::device_reset_after_children()
{
	m_cpu->set_emmu_enable(true);
}

void sgi_ip2_device::mem_map(address_map &map)
{
	// 0 text/data
	map(0x0000'0000, 0x0fff'ffff).rw(FUNC(sgi_ip2_device::ram_r<0>), FUNC(sgi_ip2_device::ram_w<0>));

	// 1 stack
	map(0x1000'0000, 0x1fff'ffff).rw(FUNC(sgi_ip2_device::ram_r<1>), FUNC(sgi_ip2_device::ram_w<1>));

	// 2 kernel
	map(0x2000'0000, 0x2fff'ffff).rw(FUNC(sgi_ip2_device::ram_r<2>), FUNC(sgi_ip2_device::ram_w<2>));

	// 3 system
	map(0x3000'0000, 0x3fff'ffff).m(*this, FUNC(sgi_ip2_device::sys_map));

	// 4 Multibus memory
	map(0x4000'0000, 0x40ff'ffff).lrw16(
		[this](offs_t offset, u16 mem_mask)
		{
			auto [data, flags] = m_bus_mem.read_word_flags(offset << 1, mem_mask);

			if (flags)
				bus_error(0x4000'0000 + (offset << 1), true, false);

			return data;
		}, "mem_r",
		[this](offs_t offset, u16 data, u16 mem_mask)
		{
			if (m_bus_mem.write_word_flags(offset << 1, data, mem_mask))
				bus_error(0x4000'0000 + (offset << 1), false, false);
		}, "mem_w");

	// 5 Multibus i/o
	map(0x5000'0000, 0x5000'ffff).lrw16(
		[this](offs_t offset, u16 mem_mask) -> u16
		{
			auto [data, flags] = m_bus_pio.read_word_flags(offset << 1, mem_mask);

			if (flags)
				bus_error(0x5000'0000 + (offset << 1), true, false);

			return data;
		}, "pio_r",
		[this](offs_t offset, u16 data, u16 mem_mask)
			{
				if (m_bus_pio.write_word_flags(offset << 1, data, mem_mask))
					bus_error(0x5000'0000 + (offset << 1), false, false);
			}, "pio_w");

	map(0x6000'0000, 0x6fff'ffff).noprw(); // geometry pipe
	map(0xf000'0000, 0xffff'ffff).noprw(); // floating point accelerator

	map(0x0000'0000, 0xffff'ffff).view(m_boot);
	m_boot[0](0x0000'0000, 0x0fff'ffff).m(*this, FUNC(sgi_ip2_device::sys_map)).mirror(0xf000'0000);
}

void sgi_ip2_device::sys_map(address_map &map)
{
	map(0x000'0000, 0x001'7fff).rom().region("cpu", 0);
	map(0x080'0000, 0x080'0000).r(FUNC(sgi_ip2_device::mbtn_r));
	map(0x100'0000, 0x100'0001).r(FUNC(sgi_ip2_device::mloc_r));
	map(0x180'0000, 0x180'0001).lr16([this]() { return m_switch->read(); }, "switch_r");
	map(0x200'0000, 0x200'000f).rw(m_duart[0], FUNC(mc68681_device::read), FUNC(mc68681_device::write));
	map(0x280'0000, 0x280'000f).rw(m_duart[1], FUNC(mc68681_device::read), FUNC(mc68681_device::write));
	map(0x300'0000, 0x300'07ff).ram().share("nvram");

	map(0x400'0000, 0x400'0000).rw(FUNC(sgi_ip2_device::rtc_ctrl_r), FUNC(sgi_ip2_device::rtc_ctrl_w));
	map(0x500'0000, 0x500'0000).rw(FUNC(sgi_ip2_device::rtc_data_r), FUNC(sgi_ip2_device::rtc_data_w));
	map(0x600'0000, 0x600'0000).rw(FUNC(sgi_ip2_device::kbase_r), FUNC(sgi_ip2_device::kbase_w));

	map(0x800'0000, 0x800'0001).rw(FUNC(sgi_ip2_device::status_r), FUNC(sgi_ip2_device::status_w));
	map(0x900'0000, 0x900'0000).rw(FUNC(sgi_ip2_device::pctrl_r), FUNC(sgi_ip2_device::pctrl_w));
	map(0xa00'0000, 0xa00'0000).rw(FUNC(sgi_ip2_device::mbprot_r), FUNC(sgi_ip2_device::mbprot_w));
	map(0xb00'0000, 0xb00'ffff).rw(FUNC(sgi_ip2_device::page_r), FUNC(sgi_ip2_device::page_w));

	map(0xc00'0000, 0xc00'0001).rw(FUNC(sgi_ip2_device::base_r<0>), FUNC(sgi_ip2_device::base_w<0>));
	map(0xd00'0000, 0xd00'0001).rw(FUNC(sgi_ip2_device::limit_r<0>), FUNC(sgi_ip2_device::limit_w<0>));
	map(0xe00'0000, 0xe00'0001).rw(FUNC(sgi_ip2_device::base_r<1>), FUNC(sgi_ip2_device::base_w<1>));
	map(0xf00'0000, 0xf00'0001).rw(FUNC(sgi_ip2_device::limit_r<1>), FUNC(sgi_ip2_device::limit_w<1>));
}

void sgi_ip2_device::cpu_map(address_map &map)
{
	/*
	 * All interrupts have user defined vectors stored in U118, a 27S29 512x8
	 * PROM. The PROM is addressed using 3 bits from the CPU address and 6 bits
	 * sampled from interrupt sources.
	 *
	 * Resulting interrupts, vectors and sources are:
	 *
	 *  IRQ  A8..6   Range   Vector and Source
	 *        000   000-03f  40=unused
	 *   1    001   040-07f  41=Multibus 0,1
	 *   2    010   080-0bf  42=Multibus 2
	 *   3    011   0c0-0ff  43=Multibus 3
	 *   4    100   100-13f  44=Multibus 4
	 *   5    101   140-17f  45=Multibus 5
	 *   6    110   180-1bf  46=Multibus 6, 50=uart0, 51=uart1, 52=ext, 53=rtc
	 *   7    111   1c0-1ff  (47=Multibus 7 unused), 55=parity, 56=mouse, 57=mouse unplugged
	 */
	map(0xffff'fff0, 0xffff'ffff).lr8(
		[this](offs_t offset)
		{
			return m_vector[BIT(offset, 1, 3) << 6 | m_int];
		}, "vector_r");
}

template <unsigned Segment> u16 sgi_ip2_device::mapa(offs_t offset) const
{
	if constexpr (Segment == 1)
		return m_base[Segment] - (BIT(offset, 10, 14) ^ 0x3fffU);
	else
		return m_base[Segment] + BIT(offset, 10, 14);
}

template <unsigned Segment> u32 sgi_ip2_device::ram_r(offs_t offset, u32 mem_mask)
{
	u32 &page = m_page[mapa<Segment>(offset)];

	if (!machine().side_effects_disabled())
	{
		// check protection
		if (!(page & PAGE_P) || ((page & PAGE_P) == PAGE_PS && !(m_cpu->get_fc() & 4)))
		{
			bus_error((Segment << 28) + (offset << 2), true, true);

			return 0;
		}

		page |= PAGE_R;
	}

	offs_t const physical = BIT(page, 0, 13) << 10 | BIT(offset, 0, 10);

	if (physical < m_ram.length())
		return m_ram[physical];

	return 0;
}

template <unsigned Segment> void sgi_ip2_device::ram_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 &page = m_page[mapa<Segment>(offset)];

	if (!machine().side_effects_disabled())
	{
		// check protection
		if (!(page & PAGE_P) || (page & PAGE_P) == PAGE_PR || ((page & PAGE_P) == PAGE_PS && !(m_cpu->get_fc() & 4)))
		{
			bus_error((Segment << 28) + (offset << 2), false, true);

			return;
		}

		page |= PAGE_M | PAGE_R;
	}

	offs_t const physical = BIT(page, 0, 13) << 10 | BIT(offset, 0, 10);

	if (physical < m_ram.length())
		COMBINE_DATA(&m_ram[physical]);
}

u8 sgi_ip2_device::mbtn_r()
{
	return m_mbtn;
}

u16 sgi_ip2_device::mloc_r()
{
	return m_mloc;
}

u8 sgi_ip2_device::rtc_ctrl_r()
{
	return m_rtc_ctrl;
}

void sgi_ip2_device::rtc_ctrl_w(u8 data)
{
	m_rtc_ctrl = data;
}

u8 sgi_ip2_device::rtc_data_r()
{
	if (m_rtc_ctrl == (RTC_RE | RTC_DS))
		return m_rtc->read_direct(m_rtc_addr);
	else
		return m_rtc_addr;
}

void sgi_ip2_device::rtc_data_w(u8 data)
{
	if (m_rtc_ctrl == RTC_CE)
		m_rtc->write_direct(m_rtc_addr, data);
	else
		m_rtc_addr = data;
}

u8 sgi_ip2_device::kbase_r()
{
	return m_base[2] >> 8;
}

void sgi_ip2_device::kbase_w(u8 data)
{
	LOG("%s: kbase_w 0x%02x\n", machine().describe_context(), data);

	// storing kernel base as u16 simplifies address translation
	m_base[2] = u16(data) << 8;
}

u16 sgi_ip2_device::status_r()
{
	return m_status;
}

void sgi_ip2_device::status_w(u16 data)
{
	LOG("%s: status_w 0x%04x\n", machine().describe_context(), data);

	if ((data ^ m_status) & ST_BOOT_)
	{
		if (data & ST_BOOT_)
			m_boot.disable();
		else
			m_boot.select(0);
	}

	if ((data ^ m_status) & ST_SLAVE)
	{
		if (data & ST_SLAVE)
			m_slave.select(0);
		else
			m_slave.disable();
	}

	m_status = data;
}

u8 sgi_ip2_device::pctrl_r()
{
	return m_pctrl;
}

void sgi_ip2_device::pctrl_w(u8 data)
{
	LOG("%s: pctrl_w: %02x\n", machine().describe_context(), data);

	m_pctrl = data;
}

uint8_t sgi_ip2_device::mbprot_r()
{
	return m_mbprot;
}

void sgi_ip2_device::mbprot_w(u8 data)
{
	LOG("%s: mbprot_w: %02x\n", machine().describe_context(), data);

	m_mbprot = data;
}

u32 sgi_ip2_device::page_r(offs_t offset)
{
	return m_page[offset];
}
void sgi_ip2_device::page_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("%s: page_w[0x%04x] data 0x%08x\n", machine().describe_context(), offset, data);

	m_page[offset] = data & PAGE_ALL;
}

template <unsigned Segment> u16 sgi_ip2_device::base_r()
{
	return m_base[Segment];
}

template <unsigned Segment> void sgi_ip2_device::base_w(u16 data)
{
	LOG("%s: base_w[%u] 0x%04x\n", machine().describe_context(), Segment, data);

	m_base[Segment] = data;
}

template <unsigned Segment> u16 sgi_ip2_device::limit_r()
{
	return m_limit[Segment];
}

template <unsigned Segment> void sgi_ip2_device::limit_w(u16 data)
{
	LOG("%s: limit_w[%u] 0x%04x\n", machine().describe_context(), Segment, data);

	m_limit[Segment] = data;
}

u16 sgi_ip2_device::mem_r(offs_t offset, u16 mem_mask)
{
	// mapped address bits A26 and A27 are discarded
	offs_t const physical = u32(m_map[offset >> 11] & 0x3fffU) << 11 | (offset & 0x7ff);
	u16 const data = m_mem[physical];

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_MULTIBUS, "%s: mem_r 0x%06x physical 0x%06x data 0x%04x\n", machine().describe_context(), offset << 1, physical << 1, data);

	return data;
}

void sgi_ip2_device::mem_w(offs_t offset, u16 data, u16 mem_mask)
{
	// mapped address bits A26 and A27 are discarded
	offs_t const physical = u32(m_map[offset >> 11] & 0x3fffU) << 11 | (offset & 0x7ff);

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_MULTIBUS, "%s: mem_w 0x%06x physical 0x%06x data 0x%04x\n", machine().describe_context(), offset << 1, physical << 1, data);

	COMBINE_DATA(&m_mem[physical]);
}

u16 sgi_ip2_device::map_r(offs_t offset, u16 mem_mask)
{
	return m_map[offset >> 11];
}

void sgi_ip2_device::map_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_MULTIBUS, "%s: map_w[0x%02x] data 0x%04x\n", machine().describe_context(), offset >> 11, data);

	m_map[offset >> 11] = data;
}

/*
 * bus error sources:
 *  - invalid memory segment
 *  - user access memory segment without permission
 *  - user access multibus memory without permission
 *  - memory timeout
 *  - access to protected page
 *  - FPA
 */
void sgi_ip2_device::bus_error(u32 address, bool read, bool retry)
{
	if (retry)
		LOG("%s: fault %c 0x%08x\n", machine().describe_context(), read ? 'r' : 'w', address);

	u8 const fc = m_cpu->get_fc();

	m_cpu->set_buserror_details(address, read, fc, retry);

	if (!retry)
		m_cpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
}

template <unsigned I> void sgi_ip2_device::int_w(int state)
{
	if (state)
		m_int |= 1U << I;
	else
		m_int &= ~(1U << I);
}

static INPUT_PORTS_START(sgi_ip2)
	PORT_START("SWITCH")
	PORT_DIPNAME( 0x8000, 0x0000, "Master/Slave" )
	PORT_DIPSETTING(    0x0000, "Master" )
	PORT_DIPSETTING(    0x8000, "Slave" )
	PORT_BIT( 0x6000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x1800, 0x0000, "RS232 Console Speed" )
	PORT_DIPSETTING(    0x0000, "9600 Baud" )
	PORT_DIPSETTING(    0x0800, "300 Baud" )
	PORT_DIPSETTING(    0x1000, "1200 Baud" )
	PORT_DIPSETTING(    0x1800, "19200 Baud" )
	PORT_DIPNAME( 0x0700, 0x0000, "Display Setting" )
	PORT_DIPSETTING(    0x0000, "60Hz Non-Interlaced / 60Hz Non-Interlaced" )
	PORT_DIPSETTING(    0x0100, "60Hz Non-Interlaced / 30Hz Interlaced" )
	PORT_DIPSETTING(    0x0200, "60Hz Non-Interlaced / NTSC RS 170A" )
	PORT_DIPSETTING(    0x0300, "60Hz Non-Interlaced / PAL" )
	PORT_DIPSETTING(    0x0400, "30Hz Interlaced / 60Hz Non-Interlaced" )
	PORT_DIPSETTING(    0x0500, "30Hz Interlaced / 30Hz Interlaced" )
	PORT_DIPSETTING(    0x0600, "30Hz Interlaced / NTSC RS 170A" )
	PORT_DIPSETTING(    0x0700, "30Hz Interlaced / PAL" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0000, "Enable Dual-Head Display" )
	PORT_DIPSETTING(    0x0000, "Use Primary Display" )
	PORT_DIPSETTING(    0x0040, "Use Secondary Display" )
	PORT_DIPNAME( 0x0020, 0x0000, "Verbose Boot" )
	PORT_DIPSETTING(    0x0000, "Be Verbose" )
	PORT_DIPSETTING(    0x0020, "Be Quiet" )
	PORT_DIPNAME( 0x0010, 0x0000, "Auto-Boot" )
	PORT_DIPSETTING(    0x0000, "Enter PROM Monitor" )
	PORT_DIPSETTING(    0x0010, "Auto-Boot" )
	PORT_DIPNAME( 0x000f, 0x0005, "Boot Media" )
	PORT_DIPSETTING(    0x0000, "Hard Disk (IP, SD, MD)" )
	PORT_DIPSETTING(    0x0001, "Cartridge Tape" )
	PORT_DIPSETTING(    0x0002, "Floppy Disk (SF, MF)" )
	PORT_DIPSETTING(    0x0003, "Ethernet using XNS" )
	PORT_DIPSETTING(    0x0005, "Enter PROM Monitor" )
	PORT_DIPSETTING(    0x0006, "Boot from PROM Board" )
	PORT_DIPSETTING(    0x0007, "TCP/UDP Netboot" )
	PORT_DIPSETTING(    0x0009, "Interphase SMD Disk Boot" )
	PORT_DIPSETTING(    0x000a, "Storager Tape Boot" )
	PORT_DIPSETTING(    0x000b, "Storager Floppy Boot" )
	PORT_DIPSETTING(    0x000c, "Storager Hard Disk Boot" )
	PORT_DIPSETTING(    0x000d, "DSD Tape Boot" )
	PORT_DIPSETTING(    0x000e, "DSD Floppy Boot" )
	PORT_DIPSETTING(    0x000f, "DSD Hard Disk Boot" )
INPUT_PORTS_END

ioport_constructor sgi_ip2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sgi_ip2);
}

ROM_START(sgi_ip2)
	ROM_DEFAULT_BIOS("v3010")
	ROM_SYSTEM_BIOS(0, "v3010", "IRIS Monitor Version 3.0.10 July 1, 1987")

	ROM_REGION32_BE(0x18000, "cpu", 0)
	ROMX_LOAD( "ip2_0__5000_455_08.u91", 0x00000, 0x8000, CRC(32e1f6b5) SHA1(2bd928c3fe2e364b9a38189158e9bad0e5271a59), ROM_BIOS(0))
	ROMX_LOAD( "ip2_1__5000_456_08.u92", 0x08000, 0x8000, CRC(13dbfdb3) SHA1(3361fb62f7a8c429653700bccfc3e937f7508182), ROM_BIOS(0))
	ROMX_LOAD( "ip2_2__5000_484_08.u93", 0x10000, 0x8000, CRC(bf967590) SHA1(1aac48e4f5531a25c5482f64de5cd3c7a9931f11), ROM_BIOS(0))

	ROM_REGION(0x200, "vector", 0)
	ROM_LOAD("ip2_u118__5000_457_02.u118", 0x000, 0x200, CRC(215e7b45) SHA1(32907201621ef128c4150721570085c554df918e))
ROM_END

const tiny_rom_entry *sgi_ip2_device::device_rom_region() const
{
	return ROM_NAME(sgi_ip2);
}

} // Anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(SGI_IP2, device_multibus_interface, sgi_ip2_device, "sgi_ip2", "Silicon Graphics IP2")
