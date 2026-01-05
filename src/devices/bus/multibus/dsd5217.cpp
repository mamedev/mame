// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Data Systems Design DSD 5217 Multibus Disk Controller
 *
 * Sources:
 *  - DSD 5215 Multibus Disk Controller User Guide (040040-01, Revision B), April 1984, Data Systems Design, Inc.
 *
 * TODO:
 *  - read/write controller, sequencer
 *  - ST506, SA460, QIC-02 interfaces
 *  - 8-bit and segmented modes
 */

#include "emu.h"
#include "dsd5217.h"

#include "cpu/i8085/i8085.h"
#include "machine/am2910.h"
#include "machine/am9517a.h"
#include "machine/i8155.h"
#include "imagedev/floppy.h"

#include "multibyte.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

enum port20_bits : unsigned
{
	P20_FDIDX = 4, // floppy index
	P20_FDWP  = 6, // floppy write protect
};
enum port78_bits : unsigned
{
	P78_RWDATA = 0,
	P78_MBBYTE = 1, // Multibus transfer alignment?
	P78_MBINT  = 2, // Multibus interrupt
	P78_MBA15  = 3, // Multibus address bit 15
	P78_CR1    = 4, // CR1 (ERR) LED
	P78_CR2    = 5, // CR2 (RDY) LED
};

class multibus_dsd5217_device
	: public device_t
	, public device_multibus_interface
{
public:
	multibus_dsd5217_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MULTIBUS_DSD5217, tag, owner, clock)
		, device_multibus_interface(mconfig, *this)
		, m_cpu(*this, "mpu")
		, m_rwc(*this, "rwc")
		, m_rio(*this, "rio")
		, m_dma(*this, "dma")
		, m_w6(*this, "W6")
		, m_w7(*this, "W7")
		, m_w9(*this, "W9")
		, m_w10(*this, "W10")
		, m_led(*this, "CR%u", 1U)
		, m_buf(nullptr)
		, m_crc(nullptr)
		, m_interrupt(false)
		, m_installed(false)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void cpu_mem(address_map &map);
	void cpu_pio(address_map &map);

	// host interface
	void cmd_w(u8 data);

	// handlers
	u8 bus_r(offs_t offset);
	void bus_w(offs_t offset, u8 data);
	template <bool High> void ctr_w(offs_t offset, u8 data);
	template <unsigned Port> u8 buf_r();
	template <unsigned Port> void buf_w(u8 data);
	void mua_w(u8 data);
	void port78_w(u8 data);

	// helpers
	void append_crc();
	void interrupt(bool assert);

private:
	required_device<i8085a_cpu_device> m_cpu;
	required_device<am2910_device> m_rwc;
	required_device<i8155_device> m_rio;
	required_device<am9517a_device> m_dma;

	required_ioport m_w6;
	required_ioport m_w7;
	required_ioport m_w9;
	required_ioport m_w10;
	output_finder<2> m_led;

	u16 m_wua; // wake-up address latch
	u8 m_mua;  // Multibus upper address
	u8 m_port78;
	u8 m_rwc_reg[4];

	u8 m_pa; // i8155 port A

	std::unique_ptr<u8[]> m_buf;
	u16 m_ctr[2]; // buffer counters (AM25LS2569PC x6)

	std::unique_ptr<u32[]> m_crc;

	bool m_interrupt;
	bool m_installed;
};

DEFINE_DEVICE_TYPE_PRIVATE(MULTIBUS_DSD5217, device_multibus_interface, multibus_dsd5217_device, "dsd5217", "Data Systems Design DSD 5217 Multibus Disk Controller")

void multibus_dsd5217_device::device_start()
{
	m_buf = std::make_unique<u8[]>(4096); // LC3517A-12 x2 (2048x8 static RAM)
	m_crc = std::make_unique<u32[]>(256);

	save_item(NAME(m_wua));
	save_item(NAME(m_mua));
	save_item(NAME(m_port78));
	save_item(NAME(m_rwc_reg));
	save_item(NAME(m_pa));
	save_item(NAME(m_ctr));

	save_pointer(NAME(m_buf), 4096);

	m_led.resolve();

	// populate 32-bit crc lookup table
	u32 const polynomial = 0x0010'5187U;
	for (unsigned i = 0; i < 256; i++)
	{
		u32 crc = i << 24;
		for (unsigned j = 0; j < 8; j++)
		{
			if (crc & 0x8000'0000)
				crc = (crc << 1) ^ polynomial;
			else
				crc <<= 1;
		}
		m_crc[i] = crc;
	}
}

void multibus_dsd5217_device::device_reset()
{
	if (!m_installed)
	{
		// TODO: 8-bit and segmented modes
		offs_t const wua = (m_w7->read() << 8) | m_w9->read();

		m_bus->space(AS_IO).install_write_handler(wua, wua, emu::rw_delegate(*this, FUNC(multibus_dsd5217_device::cmd_w)));

		m_installed = true;
	}

	m_wua = 0;
	m_mua = 0;
	m_port78 = 0;
	m_ctr[0] = 0;
	m_ctr[1] = 0;

	interrupt(false);
}

void multibus_dsd5217_device::device_add_mconfig(machine_config &config)
{
	I8085A(config, m_cpu, 10_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &multibus_dsd5217_device::cpu_mem);
	m_cpu->set_addrmap(AS_IO, &multibus_dsd5217_device::cpu_pio);
	m_cpu->in_sid_func().set([this]() -> int { return !BIT(m_wua, 15); });

	AM2910(config, m_rwc, 0); // TODO: clocked at disk data rate
	// TODO: 1K words of prom

	I8155(config, m_rio, 10_MHz_XTAL / 2);
	m_rio->out_to_callback().set_inputline(m_cpu, I8085_TRAP_LINE);
	m_rio->out_pa_callback().set([this](u8 data) { logerror("pa 0x%02x\n", data); m_pa = data; });
	m_rio->out_pc_callback().set(
		[this](int state)
		{
			/*
			 * 3 precomp
			 * 2 reduced write current/hs3
			 * 1 direction?
			 * 0 step?
			 *
			 */
			if (state)
			{
				if (BIT(m_port78, P78_CR1))
					m_wua <<= 1;
				else
					m_wua = (m_w7->read() << 8) | m_w9->read();
			}
		}).bit(5);

	AM9517A(config, m_dma, 10_MHz_XTAL / 2);
	// ch0: tape, ch1: buffer
	m_dma->out_hreq_callback().set(m_dma, FUNC(am9517a_device::hack_w));
	m_dma->in_memr_callback().set([this](offs_t offset) { return m_cpu->space(AS_PROGRAM).read_byte(offset); });
	m_dma->out_memw_callback().set([this](offs_t offset, u8 data) { m_cpu->space(AS_PROGRAM).write_byte(offset, data); });
	m_dma->in_ior_callback<1>().set([this](offs_t offset) { return m_buf[offset & 0xfff]; });
	m_dma->out_iow_callback<1>().set([this](offs_t offset, u8 data) { m_buf[offset & 0xfff] = data; });
}

void multibus_dsd5217_device::append_crc()
{
	// port A bit 5 controls 32-bit or 16-bit crc
	if (BIT(m_pa, 5))
	{
		u32 crc = 0xffff'ffffU;

		for (u16 offset = 0; offset < m_ctr[0]; offset++)
			crc = m_crc[u8((crc >> 24) ^ m_buf[offset])] ^ (crc << 8);

		logerror("crc 0x%08x\n", crc);
		put_u32be(&m_buf[m_ctr[0]], crc);
		m_ctr[0] += 4;
	}
	else
	{
		util::crc16_t const crc = util::crc16_creator::simple(m_buf.get(), m_ctr[0]);

		put_u16be(&m_buf[m_ctr[0]], crc);
		m_ctr[0] += 2;
	}
}

void multibus_dsd5217_device::cpu_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("cpu", 0);
	map(0x4000, 0x40ff).rw(m_rio, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));

	// read/write controller crc/ecc
	map(0x6000, 0x6000).lr8([]() { return 4; }, "rwc_r");
	map(0x6000, 0x6003).lw8(
		[this](offs_t offset, u8 data)
		{
			// TODO: fd: sector,head,cyl,flags
			m_rwc_reg[offset] = data;

			// HACK: this code allows diagnostic to pass, but is probably wrong
			if (offset == 0)
			{
				append_crc();

				// append data written to 0x6000..6003
				for (size_t i = 0; i < std::size(m_rwc_reg); i++)
					m_buf[m_ctr[0]++] = m_rwc_reg[i];

				// append one more byte (valid values 0x52..0x5f)?
				m_buf[m_ctr[0]++] = 0x53;
			}
		}, "crc_w");

	map(0x8000, 0xffff).rw(FUNC(multibus_dsd5217_device::bus_r), FUNC(multibus_dsd5217_device::bus_w));
}

void multibus_dsd5217_device::cpu_pio(address_map &map)
{
	map(0x00, 0x0f).rw(m_dma, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x20, 0x20).lr8([]() { return 0x09; }, "unit_ready");
	map(0x40, 0x47).rw(m_rio, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));

	map(0x60, 0x60).rw(FUNC(multibus_dsd5217_device::buf_r<0>), FUNC(multibus_dsd5217_device::buf_w<0>));
	map(0x64, 0x64).lw8([this](u8 data) { multibus_dsd5217_device::ctr_w<true>(0, data); }, "ctr0h_w");
	map(0x68, 0x68).lw8([this](u8 data) { multibus_dsd5217_device::ctr_w<false>(0, data); }, "ctr0l_w");
	map(0x6c, 0x6c).lw8([this](u8 data) { multibus_dsd5217_device::ctr_w<true>(1, data); }, "ctr1h_w");
	map(0x70, 0x70).lw8([this](u8 data) { multibus_dsd5217_device::ctr_w<false>(1, data); }, "ctr1l_w");
	map(0x70, 0x70).lr8([this]() { return u8(m_w6->read()) | 0x80; }, "w6_r");
	map(0x74, 0x74).w(FUNC(multibus_dsd5217_device::mua_w));
	map(0x78, 0x78).w(FUNC(multibus_dsd5217_device::port78_w));
}

void multibus_dsd5217_device::cmd_w(u8 data)
{
	LOG("%s: cmd_w 0x%02x\n", machine().describe_context(), data);

	switch (data & 3)
	{
	case 0x00:
		LOG("Clear\n");
		m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		break;
	case 0x01:
		LOG("Start\n");
		m_cpu->set_input_line(I8085_RST75_LINE, ASSERT_LINE);
		m_cpu->set_input_line(I8085_RST75_LINE, CLEAR_LINE);
		break;
	case 0x02:
		LOG("Reset\n");
		m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		break;
	}
}

u8 multibus_dsd5217_device::bus_r(offs_t offset)
{
	offs_t const address = (u32(m_mua) << 16) | (BIT(m_port78, P78_MBA15) << 15) | offset;

	return m_bus->space(AS_PROGRAM).read_byte(address);
}

void multibus_dsd5217_device::bus_w(offs_t offset, u8 data)
{
	offs_t const address = (u32(m_mua) << 16) | (BIT(m_port78, P78_MBA15) << 15) | offset;

	m_bus->space(AS_PROGRAM).write_byte(address, data);
}

template <bool High> void multibus_dsd5217_device::ctr_w(offs_t offset, u8 data)
{
	if (High)
		m_ctr[offset] = (m_ctr[offset] & 0x00ffU) | u16(data & 0x0f) << 8;
	else
		m_ctr[offset] = (m_ctr[offset] & 0x0f00U) | data;
}

template <unsigned Port> u8 multibus_dsd5217_device::buf_r()
{
	u8 const data = m_buf[m_ctr[Port]];

	m_ctr[Port] = (m_ctr[Port] + 1) & 0x0fffU;

	return data;
}

template <unsigned Port> void multibus_dsd5217_device::buf_w(u8 data)
{
	m_buf[m_ctr[Port]] = data;

	m_ctr[Port] = (m_ctr[Port] + 1) & 0x0fffU;
}

void multibus_dsd5217_device::mua_w(u8 data)
{
	LOG("%s: mua_w 0x%02x\n", machine().describe_context(), data);
	m_mua = data;
}
void multibus_dsd5217_device::port78_w(u8 data)
{
	LOG("%s: port78_w 0x%02x\n", machine().describe_context(), data);

	m_led[0] = BIT(data, P78_CR1);
	m_led[1] = BIT(data, P78_CR2);

	interrupt(BIT(data, P78_MBINT));

	m_port78 = data;
}

void multibus_dsd5217_device::interrupt(bool state)
{
	if (m_interrupt != state)
	{
		LOG("interrupt %u\n", state);

		switch (m_w10->read())
		{
		case 0xfe: m_bus->int_w<0>(state ? 0 : 1); break;
		case 0xfd: m_bus->int_w<1>(state ? 0 : 1); break;
		case 0xfb: m_bus->int_w<2>(state ? 0 : 1); break;
		case 0xf7: m_bus->int_w<3>(state ? 0 : 1); break;
		case 0xef: m_bus->int_w<4>(state ? 0 : 1); break;
		case 0xdf: m_bus->int_w<5>(state ? 0 : 1); break;
		case 0xbf: m_bus->int_w<6>(state ? 0 : 1); break;
		case 0x7f: m_bus->int_w<7>(state ? 0 : 1); break;
		}

		m_interrupt = state;
	}
}

ROM_START(dsd5217)
	ROM_SYSTEM_BIOS(0, "a", "080448-02 Rev A")

	ROM_REGION(0x4000, "cpu", 0)
	ROMX_LOAD("80448_01.bin", 0x0000, 0x2000, CRC(371cb3d0) SHA1(a9a64c75ac622bae3fbecb116e0445df9cb53205), ROM_BIOS(0))
	ROMX_LOAD("80448_02.bin", 0x2000, 0x2000, CRC(edb2ad5d) SHA1(8b3dca21eea8d3b912572480f20fc1e53610ea04), ROM_BIOS(0))
ROM_END

const tiny_rom_entry *multibus_dsd5217_device::device_rom_region() const
{
	return ROM_NAME(dsd5217);
}

static INPUT_PORTS_START(dsd5217)
	PORT_START("W6")
	PORT_DIPNAME(0x03, 0x00, "Floppy Drive Class") PORT_DIPLOCATION("W6:!1,W6:!2") PORT_CONDITION("W6", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x01, "1")
	PORT_DIPSETTING(0x02, "2")
	PORT_DIPSETTING(0x03, "3 Reserved")

	PORT_DIPNAME(0x1c, 0x00, "Winchester Drive Class") PORT_DIPLOCATION("W6:!3,W6:!4,W6:!5") PORT_CONDITION("W6", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x04, "1")
	PORT_DIPSETTING(0x08, "2")
	PORT_DIPSETTING(0x0c, "3")
	PORT_DIPSETTING(0x10, "4")
	PORT_DIPSETTING(0x14, "5")
	PORT_DIPSETTING(0x18, "6")
	PORT_DIPSETTING(0x1c, "7 Reserved")

	PORT_DIPNAME(0x1f, 0x00, "Diagnostic Routine") PORT_DIPLOCATION("W6:!1,W6:!2,W6:!3,W6:!4,W6:!5") PORT_CONDITION("W6", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(0x00, "8085 Environment Self-Test")
	PORT_DIPSETTING(0x01, "Winchester PLL Alignment")
	PORT_DIPSETTING(0x02, "Floppy Double-Density PLL Alignment")
	PORT_DIPSETTING(0x03, "Floppy Single-Density PLL Alignment")
	PORT_DIPSETTING(0x04, "R/WC Self-Test")
	PORT_DIPSETTING(0x05, "CR1, CR2 Blinking Wakeup Address")
	PORT_DIPSETTING(0x06, "Floppy Drive 0")
	PORT_DIPSETTING(0x07, "Floppy Drive 1")
	PORT_DIPSETTING(0x08, "Winchester Drive 0")
	PORT_DIPSETTING(0x09, "Winchester Drive 1")
	PORT_DIPSETTING(0x0a, "Tape Drive")
	PORT_DIPSETTING(0x0b, "Stand Alone System")
	PORT_DIPSETTING(0x0c, "Multibus Read/Write")

	PORT_DIPNAME(0x20, 0x00, "Diagnostics") PORT_DIPLOCATION("W6:!6")
	PORT_DIPSETTING(0x20, "Enabled")
	PORT_DIPSETTING(0x00, "Disabled")

	PORT_START("W7")
	PORT_DIPNAME(0x01, 0x01, "Wake-up Address (A8)") PORT_DIPLOCATION("W7:!1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "Wake-up Address (A9)") PORT_DIPLOCATION("W7:!2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "Wake-up Address (A10)") PORT_DIPLOCATION("W7:!3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "Wake-up Address (A11)") PORT_DIPLOCATION("W7:!4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x00, "Wake-up Address (A12)") PORT_DIPLOCATION("W7:!5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x00, "Wake-up Address (A13)") PORT_DIPLOCATION("W7:!6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x00, "Wake-up Address (A14)") PORT_DIPLOCATION("W7:!7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x00, "Wake-up Address (A15)") PORT_DIPLOCATION("W7:!8")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("W9")
	PORT_DIPNAME(0x01, 0x00, "Wake-up Address (A0)") PORT_DIPLOCATION("W9:!1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "Wake-up Address (A1)") PORT_DIPLOCATION("W9:!2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, "Wake-up Address (A2)") PORT_DIPLOCATION("W9:!3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x00, "Wake-up Address (A3)") PORT_DIPLOCATION("W9:!4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, "Wake-up Address (A4)") PORT_DIPLOCATION("W9:!5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, "Wake-up Address (A5)") PORT_DIPLOCATION("W9:!6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, "Wake-up Address (A6)") PORT_DIPLOCATION("W9:!7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x00, "Wake-up Address (A7)") PORT_DIPLOCATION("W9:!8")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("W10")
	PORT_DIPNAME(0xff, 0xdf, "Interrupt Priority Level") PORT_DIPLOCATION("W10:!1,W10:!2,W10:!3,W10:!4,W10:!5,W10:!6,W10:!7,W10:!8")
	PORT_DIPSETTING(0xfe, "0")
	PORT_DIPSETTING(0xfd, "1")
	PORT_DIPSETTING(0xfb, "2")
	PORT_DIPSETTING(0xf7, "3")
	PORT_DIPSETTING(0xef, "4")
	PORT_DIPSETTING(0xdf, "5")
	PORT_DIPSETTING(0xbf, "6")
	PORT_DIPSETTING(0x7f, "7")
INPUT_PORTS_END

ioport_constructor multibus_dsd5217_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(dsd5217);
}
