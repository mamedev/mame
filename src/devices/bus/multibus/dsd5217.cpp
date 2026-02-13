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

#include "bus/qic02/qic02.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"
#include "machine/am2910.h"
#include "machine/am9517a.h"
#include "machine/i8155.h"
#include "machine/input_merger.h"

#include "multibyte.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

enum port20_mask : u8
{
	P20_WRDY   = 0x01, // winchester ready
	P20_WTRK0  = 0x02, // winchester track 0
	P20_WWFLT  = 0x04, // winchester write fault
	P20_WSEEK  = 0x08, // winchester seek complete
	P20_FIDX   = 0x10, // floppy index
	P20_FTRK0  = 0x20, // floppy track 0
	P20_FWPT   = 0x40, // floppy write protect
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

enum portB_bits : unsigned
{
	PB_WDS0  = 0, // winchester drive select 1
	PB_WDS1  = 1, // winchester drive select 2
	PB_FDS0  = 2, // floppy drive select 1
	PB_FDS1  = 3, // floppy drive select 2
	PB_HS0   = 4, // winchester head select 0/floppy side select
	PB_HS1   = 5, // winchester head select 1
	PB_HS2   = 6, // winchester head select 2
	PB_MOTOR = 7, // floppy motor
};
enum portC_bits : unsigned
{
	PC_STEP = 0, // step pulse
	PC_DIR  = 1, // step direction
	PC_HS3  = 2, // winchester head select 3/reduced write current
	PC_RWC  = 3, // winchester reduced write current
	// clear after rwc wait timeout: rwc reset?
	PC_WUA  = 5, // wake-up address
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
		, m_fdc(*this, "fdc%u", 0U)
		, m_hdd(*this, "hdd%u", 0U)
		, m_qic(*this, "qic")
		, m_xfr(*this, "xfr")
		, m_w6(*this, "W6")
		, m_w7(*this, "W7")
		, m_w9(*this, "W9")
		, m_w10(*this, "W10")
		, m_led(*this, "CR%u", 1U)
		, m_port70(0)
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
	void rwc_w(offs_t offset, u8 data);
	u8 bus_r(offs_t offset);
	void bus_w(offs_t offset, u8 data);
	template <bool High> void ctr_w(offs_t offset, u8 data);
	u8 buf_r();
	void buf_w(u8 data);
	void mua_w(u8 data);
	u8 port20_r();
	void port78_w(u8 data);
	void port7c_w(u8 data);

	void pa_w(u8 data);
	void pb_w(u8 data);
	void pc_w(u8 data);

	// helpers
	floppy_image_device *selected_fdd() const;
	std::optional<unsigned> selected_hdd() const;
	void append_crc();
	void interrupt(bool assert);
	void dump_iopb();

private:
	required_device<i8085a_cpu_device> m_cpu;
	required_device<am2910_device> m_rwc;
	required_device<i8155_device> m_rio;
	required_device<am9517a_device> m_dma;

	required_device_array<floppy_connector, 2> m_fdc;
	required_device_array<harddisk_image_device, 2> m_hdd;
	required_device<qic02_connector_device> m_qic;

	required_device<input_merger_any_high_device> m_xfr;

	required_ioport m_w6;
	required_ioport m_w7;
	required_ioport m_w9;
	required_ioport m_w10;
	output_finder<2> m_led;

	u16 m_wua; // wake-up address latch
	u8 m_mua;  // Multibus upper address
	u8 m_port70;
	u8 m_port78;
	u8 m_port7c;
	u8 m_rwc_ram[4];

	u8 m_pa; // i8155 port A
	u8 m_pb; // i8155 port B
	u8 m_pc; // i8155 port C

	std::unique_ptr<u8[]> m_buf; // LC3517A-12 x2 (2048x8 static RAM)
	u16 m_ctr[2]; // buffer counters (AM25LS2569PC x6)

	u16 m_hdd_cyl[2]; // current cylinder
	std::unique_ptr<u32[]> m_crc;

	bool m_interrupt;
	bool m_installed;
};

void multibus_dsd5217_device::device_start()
{
	m_buf = std::make_unique<u8[]>(4096);
	m_crc = std::make_unique<u32[]>(256);

	save_item(NAME(m_wua));
	save_item(NAME(m_mua));
	save_item(NAME(m_port70));
	save_item(NAME(m_port78));
	save_item(NAME(m_port7c));
	save_item(NAME(m_rwc_ram));

	save_item(NAME(m_pa));
	save_item(NAME(m_pb));
	save_item(NAME(m_pc));

	save_pointer(NAME(m_buf), 4096);
	save_item(NAME(m_ctr));

	save_item(NAME(m_hdd_cyl));

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
	m_port7c = 0;
	m_pa = 0;
	m_pb = 0;
	m_pc = 0;
	m_ctr[0] = 0;
	m_ctr[1] = 0;

	m_hdd_cyl[0] = 0;
	m_hdd_cyl[1] = 0;

	interrupt(false);

	m_xfr->in_w<0>(0);
	m_xfr->in_w<1>(0);
}

void multibus_dsd5217_device::device_add_mconfig(machine_config &config)
{
	I8085A(config, m_cpu, 10_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &multibus_dsd5217_device::cpu_mem);
	m_cpu->set_addrmap(AS_IO, &multibus_dsd5217_device::cpu_pio);
	m_cpu->in_sid_func().set([this]() -> int { return !BIT(m_wua, 15); });

	AM2910(config, m_rwc, 0); // TODO: clocked at disk data rate
	// TODO: 1K words of prom

	I8155(config, m_rio, 10_MHz_XTAL / 4);
	m_rio->out_to_callback().set_inputline(m_cpu, I8085_TRAP_LINE);
	m_rio->out_pa_callback().set(FUNC(multibus_dsd5217_device::pa_w));
	m_rio->out_pb_callback().set(FUNC(multibus_dsd5217_device::pb_w));
	m_rio->out_pc_callback().set(FUNC(multibus_dsd5217_device::pc_w));

	AM9517A(config, m_dma, 10_MHz_XTAL / 2);
	m_dma->out_hreq_callback().set_inputline(m_cpu, INPUT_LINE_HALT);
	m_dma->out_hreq_callback().append(m_xfr, FUNC(input_merger_any_high_device::in_w<0>));
	m_dma->out_hreq_callback().append(m_dma, FUNC(am9517a_device::hack_w));

	m_dma->in_memr_callback().set([this](offs_t offset) { return m_cpu->space(AS_PROGRAM).read_byte(offset); });
	m_dma->out_memw_callback().set([this](offs_t offset, u8 data) { m_cpu->space(AS_PROGRAM).write_byte(offset, data); });

	m_dma->in_ior_callback<0>().set(FUNC(multibus_dsd5217_device::buf_r));
	m_dma->out_iow_callback<0>().set(FUNC(multibus_dsd5217_device::buf_w));
	m_dma->in_ior_callback<1>().set(FUNC(multibus_dsd5217_device::buf_r));
	m_dma->out_iow_callback<1>().set(FUNC(multibus_dsd5217_device::buf_w));

	// floppy: 250Kbps MFM, 80 tracks, 2 sides, 8 sectors/track, 512 byte sectors,
	FLOPPY_CONNECTOR(config, m_fdc[0], "525qd", FLOPPY_525_QD, true,  floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_fdc[1], "525qd", FLOPPY_525_QD, false, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	HARDDISK(config, m_hdd[0]);
	HARDDISK(config, m_hdd[1]);

	QIC02_CONNECTOR(config, m_qic);
	m_qic->ack().set(m_dma, FUNC(am9517a_device::dreq0_w));
	m_qic->rdy().set([this](int state) { if (!state) m_port70 |= 0x80; else m_port70 &= ~0x80; });
	m_qic->exc().set_inputline(m_cpu, I8085_RST65_LINE).invert();
	//m_qic->dir();

	INPUT_MERGER_ANY_HIGH(config, m_xfr);
	m_xfr->output_handler().set(
		[this](int state)
		{
			if (BIT(m_port7c, 1))
				m_qic->xfr_w(state ? 0 : 1);
			else
				m_qic->xfr_w(1);
		}
	);
}

floppy_image_device *multibus_dsd5217_device::selected_fdd() const
{
	if (BIT(m_pb, PB_FDS0))
		return m_fdc[0]->get_device();
	else if (BIT(m_pb, PB_FDS1))
		return m_fdc[1]->get_device();

	return nullptr;
}

std::optional<unsigned> multibus_dsd5217_device::selected_hdd() const
{
	if (BIT(m_pb, PB_WDS0) && m_hdd[0]->exists())
		return 0;
	else if (BIT(m_pb, PB_WDS1) && m_hdd[1]->exists())
		return 1;

	return std::nullopt;
}

void multibus_dsd5217_device::append_crc()
{
	// port A bit 5 controls 32-bit or 16-bit crc
	if (BIT(m_pa, 5))
	{
		u32 crc = 0xffff'ffffU;

		for (u16 offset = 0; offset < m_ctr[0]; offset++)
			crc = m_crc[u8((crc >> 24) ^ m_buf[offset])] ^ (crc << 8);

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
	map(0x6000, 0x6003).w(FUNC(multibus_dsd5217_device::rwc_w));
	map(0x7000, 0x7000).rw(m_qic, FUNC(qic02_connector_device::data_r), FUNC(qic02_connector_device::data_w)).mirror(0xfff);
	map(0x8000, 0xffff).rw(FUNC(multibus_dsd5217_device::bus_r), FUNC(multibus_dsd5217_device::bus_w));
}

void multibus_dsd5217_device::cpu_pio(address_map &map)
{
	map(0x00, 0x0f).rw(m_dma, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x20, 0x20).r(FUNC(multibus_dsd5217_device::port20_r));
	map(0x40, 0x47).rw(m_rio, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));

	map(0x60, 0x60).rw(FUNC(multibus_dsd5217_device::buf_r), FUNC(multibus_dsd5217_device::buf_w));
	map(0x64, 0x64).lw8([this](u8 data) { multibus_dsd5217_device::ctr_w<true>(0, data); }, "ctr0h_w");
	map(0x68, 0x68).lw8([this](u8 data) { multibus_dsd5217_device::ctr_w<false>(0, data); }, "ctr0l_w");
	map(0x6c, 0x6c).lw8([this](u8 data) { multibus_dsd5217_device::ctr_w<true>(1, data); }, "ctr1h_w");
	map(0x70, 0x70).lw8([this](u8 data) { multibus_dsd5217_device::ctr_w<false>(1, data); }, "ctr1l_w");
	map(0x70, 0x70).lr8([this]() { return u8(m_w6->read()) | m_port70; }, "w6_r");
	map(0x74, 0x74).w(FUNC(multibus_dsd5217_device::mua_w));
	map(0x78, 0x78).w(FUNC(multibus_dsd5217_device::port78_w));
	map(0x7c, 0x7c).w(FUNC(multibus_dsd5217_device::port7c_w));
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
		if (VERBOSE & LOG_GENERAL)
			dump_iopb();
		m_cpu->set_input_line(I8085_RST75_LINE, ASSERT_LINE);
		m_cpu->set_input_line(I8085_RST75_LINE, CLEAR_LINE);
		break;
	case 0x02:
		LOG("Reset\n");
		m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		break;
	}
}

void multibus_dsd5217_device::rwc_w(offs_t offset, u8 data)
{
	LOG("%s: rwc_w[%u] data 0x%02x\n", machine().describe_context(), offset, data);

	m_rwc_ram[offset] = data;

	// writing to offset 0 triggers operation?
	if (offset == 0)
	{
		if (std::optional<unsigned> hdd = selected_hdd())
		{
			harddisk_image_device &hid = *m_hdd[hdd.value()];

			unsigned const c = m_hdd_cyl[hdd.value()];
			unsigned const h = BIT(m_pb, PB_HS0, 3) + BIT(m_pc, PC_HS3) * 8;
			unsigned const s = m_rwc_ram[0];

			hard_disk_file::info const info = hid.get_info();
			unsigned const lba = ((c * info.heads) + h) * info.sectors + s;

			static char const *const type[] = { "format", "read", "write", "unknown" };
			LOG("hd%u: %s cylinder=%u head=%u sector=%u lba=%u ctr0=0x%04x ctr1=0x%04x\n",
				hdd.value(), type[BIT(m_pa, 1, 2)], c, h, s, lba, m_ctr[0], m_ctr[1]);

			if (BIT(m_pa, 1))
			{
				u16 &ctr = m_ctr[1];
				hid.read(lba, &m_buf[ctr]);
				ctr = (ctr + 0x200) & 0xfff;
			}
			else if (BIT(m_pa, 2))
			{
				u16 &ctr = m_ctr[1];
				hid.write(lba, &m_buf[ctr]);
				ctr = (ctr + 0x200) & 0xfff;
			}
		}
		else
		{
			// HACK: this code allows diagnostic to pass, but is probably wrong
			append_crc();

			// append scratch data
			for (size_t i = 0; i < std::size(m_rwc_ram); i++)
				m_buf[m_ctr[0]++] = m_rwc_ram[i];

			// append one more byte (valid values 0x52..0x5f)?
			m_buf[m_ctr[0]++] = 0x53;
		}
	}
}

u8 multibus_dsd5217_device::bus_r(offs_t offset)
{
	offs_t const address = (offs_t(m_mua) << 16) | (BIT(m_port78, P78_MBA15) << 15) | offset;

	return m_bus->space(AS_PROGRAM).read_byte(address);
}

void multibus_dsd5217_device::bus_w(offs_t offset, u8 data)
{
	offs_t const address = (offs_t(m_mua) << 16) | (BIT(m_port78, P78_MBA15) << 15) | offset;

	m_bus->space(AS_PROGRAM).write_byte(address, data);
}

template <bool High> void multibus_dsd5217_device::ctr_w(offs_t offset, u8 data)
{
	u16 &ctr = m_ctr[offset];

	if (High)
		ctr = (ctr & 0x00ffU) | u16(data & 0x0f) << 8;
	else
		ctr = (ctr & 0x0f00U) | data;
}

u8 multibus_dsd5217_device::buf_r()
{
	u8 const data = m_buf[m_ctr[0]];

	if (!machine().side_effects_disabled())
		m_ctr[0] = (m_ctr[0] + 1) & 0x0fffU;

	return data;
}

void multibus_dsd5217_device::buf_w(u8 data)
{
	if (!machine().side_effects_disabled())
		m_buf[m_ctr[0]] = data;

	m_ctr[0] = (m_ctr[0] + 1) & 0x0fffU;
}

void multibus_dsd5217_device::mua_w(u8 data)
{
	if (m_mua ^ data)
		LOG("%s: mua_w 0x%02x\n", machine().describe_context(), data);

	m_mua = data;
}

u8 multibus_dsd5217_device::port20_r()
{
	u8 data = 0;

	if (std::optional<unsigned> hdd = selected_hdd())
	{
		data |= P20_WRDY;
		data |= P20_WSEEK;

		if (m_hdd_cyl[hdd.value()] == 0)
			data |= P20_WTRK0;
	}

	if (floppy_image_device *fdd = selected_fdd())
	{
		if (fdd->idx_r())
			data |= P20_FIDX;
		if (!fdd->trk00_r())
			data |= P20_FTRK0;
		if (fdd->wpt_r())
			data |= P20_FWPT;
	}

	return data;
}

void multibus_dsd5217_device::port78_w(u8 data)
{
	if (m_port78 ^ data)
		LOG("%s: port78_w 0x%02x\n", machine().describe_context(), data);

	m_led[0] = BIT(data, P78_CR1);
	m_led[1] = BIT(data, P78_CR2);

	interrupt(BIT(data, P78_MBINT));

	m_port78 = data;
}

void multibus_dsd5217_device::port7c_w(u8 data)
{
	u8 const delta = m_port7c ^ data;

	if (delta)
		LOG("%s: port7c_w 0x%02x\n", machine().describe_context(), data);

	m_port7c = data;

	if (BIT(delta, 7))
		m_qic->onl_w(!BIT(data, 7));
	if (BIT(delta, 6))
		m_qic->req_w(!BIT(data, 6));
	if (BIT(delta, 5))
		m_qic->rst_w(!BIT(data, 5));
	if (BIT(delta, 0, 2))
		m_xfr->in_w<1>(BIT(data, 1) && BIT(data, 0));
}

void multibus_dsd5217_device::pa_w(u8 data)
{
	if (m_pa ^ data)
		LOG("%s: port A 0x%02x\n", machine().describe_context(), data);

	m_pa = data;
}

void multibus_dsd5217_device::pb_w(u8 data)
{
	if (m_pb ^ data)
		LOG("%s: port B 0x%02x\n", machine().describe_context(), data);

	if (data & PB_MOTOR)
	{
		if (floppy_image_device *fdd = selected_fdd())
		{
			if (BIT(m_pb ^ data, PB_MOTOR))
				fdd->mon_w(!BIT(data, PB_MOTOR));
			if (BIT(m_pb ^ data, PB_HS0))
				fdd->ss_w(BIT(data, PB_HS0));
		}
	}

	m_pb = data;
}

void multibus_dsd5217_device::pc_w(u8 data)
{
	if (m_pc ^ data)
		LOG("%s: port C 0x%02x\n", machine().describe_context(), data);

	if (BIT(data, PC_WUA))
	{
		if (BIT(m_port78, P78_CR1))
			m_wua <<= 1;
		else
			m_wua = (m_w7->read() << 8) | m_w9->read();
	}

	if (floppy_image_device *fdd = selected_fdd())
	{
		if (BIT(m_pc ^ data, PC_DIR))
			fdd->dir_w(!BIT(data, PC_DIR));

		if (BIT(m_pc ^ data, PC_STEP) && BIT(m_pb, PB_MOTOR))
			fdd->stp_w(BIT(data, PC_STEP));
	}

	if (std::optional<unsigned> hdd = selected_hdd())
	{
		if (!BIT(m_pc, PC_STEP) && BIT(data, PC_STEP))
		{
			u16 &cyl = m_hdd_cyl[hdd.value()];

			if (BIT(data, PC_DIR))
				cyl++;
			else
				cyl--;
		}
	}

	m_pc = data;
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

void multibus_dsd5217_device::dump_iopb()
{
	auto const suppressor(machine().disable_side_effects());

	offs_t const wua = ((m_w7->read() << 8) | m_w9->read()) << 4;
	address_space &s = m_bus->space(AS_PROGRAM);
	//offs_t const mua = offs_t(m_mua) << 16;

	offs_t const ccb = s.read_dword(wua + 2);
	logerror("wub 0x%04x: extension=%u ccb=0x%x\n", wua, s.read_byte(wua + 0), ccb);

	offs_t const cib = s.read_dword_unaligned(ccb + 2);
	logerror("ccb 0x%06x: cib=0x%06x cp=0x%06x\n", ccb, cib, s.read_dword_unaligned(ccb + 10));

	if (s.read_byte(ccb + 1) == 00)
	{
		offs_t const iopb = s.read_dword_unaligned(cib + 4);
		logerror("cib 0x%06x: iopb=0x%06x\n", cib, iopb);

		u16 const device = s.read_word(iopb + 8);
		u8 const unit = s.read_byte(iopb + 10);
		u8 const function = s.read_byte(iopb + 11);
		offs_t const buffer = s.read_dword_unaligned(iopb + 18);
		logerror("iopb device=%u unit=%u function=0x%02x modifier=%u cylinder=%u head=%u sector=%u buffer=0x%06x count=%u\n",
			device, unit, function,
			s.read_word(iopb + 12), s.read_word(iopb + 14), s.read_byte(iopb + 16), s.read_byte(iopb + 17),
			buffer, s.read_dword_unaligned(iopb + 22));

		switch (function)
		{
		case 0x00: // initialize
			if (device == 4)
			{
				logerror("initialize: tpb=0x%02x\n", s.read_byte(buffer + 0));
				s.write_byte(buffer + 0, 0x01);
			}
			else
				logerror("initialize: cylinders=%u fixed_heads=%u removable_heads=%u sectors=%u sector_size=%u alternate_cylinders=%u\n",
					s.read_word(buffer + 0), s.read_byte(buffer + 2), s.read_byte(buffer + 3),
					s.read_byte(buffer + 4), s.read_word_unaligned(buffer + 5), s.read_byte(buffer + 7));
			break;
		case 0x02: // format
			switch (s.read_byte(buffer + 0))
			{
			case 0x00: logerror("format: data track, user pattern=0x%02x 0x%02x 0x%02x 0x%02x interleave=%u\n",
				s.read_byte(buffer + 1), s.read_byte(buffer + 2), s.read_byte(buffer + 3), s.read_byte(buffer + 4), s.read_byte(buffer + 5)); break;
			case 0x40: logerror("format: alternative track, user pattern=0x%02x 0x%02x 0x%02x 0x%02x interleave=%u\n",
				s.read_byte(buffer + 1), s.read_byte(buffer + 2), s.read_byte(buffer + 3), s.read_byte(buffer + 4), s.read_byte(buffer + 5)); break;
			case 0x80: logerror("format: defective track, cylinder=%u head=%u interleave=%u\n",
				s.read_word_unaligned(buffer + 1), s.read_byte(buffer + 3), s.read_byte(buffer + 5)); break;
			}
			break;
		}
	}
}

ROM_START(dsd5217)
	ROM_REGION(0x4000, "cpu", 0)
	ROM_DEFAULT_BIOS("5217")

	ROM_SYSTEM_BIOS(0, "5215", "080341 Rev C")
	ROMX_LOAD("080341_5.bin", 0x0000, 0x2000, CRC(3d774f14) SHA1(cb21936d2b9de9d0167e0e6f0bb122e11b182bcf), ROM_BIOS(0))
	ROMX_LOAD("080341_6.bin", 0x2000, 0x2000, CRC(40b02f18) SHA1(7303d35aba6f3e394949126bf115fdd19a33ec7a), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "5217", "080448 Rev A")
	ROMX_LOAD("080448_01.bin", 0x0000, 0x2000, CRC(371cb3d0) SHA1(a9a64c75ac622bae3fbecb116e0445df9cb53205), ROM_BIOS(1))
	ROMX_LOAD("080448_02.bin", 0x2000, 0x2000, CRC(edb2ad5d) SHA1(8b3dca21eea8d3b912572480f20fc1e53610ea04), ROM_BIOS(1))

	ROM_REGION(0x802, "seq", 0)
	ROM_LOAD("080202_1.bin", 0x000, 0x401, CRC(9287dbf5) SHA1(4293cde7ddbedf02814d144d77dba67387677004)) // AM27S35 including "initialize word"
	ROM_LOAD("080202_2.bin", 0x401, 0x401, CRC(896c5aaf) SHA1(5fe589a79806451abcbfbcff28eca39e925df264)) // AM27S35 including "initialize word"
ROM_END

const tiny_rom_entry *multibus_dsd5217_device::device_rom_region() const
{
	return ROM_NAME(dsd5217);
}

INPUT_PORTS_START(dsd5217)
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

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MULTIBUS_DSD5217, device_multibus_interface, multibus_dsd5217_device, "dsd5217", "Data Systems Design DSD 5217 Multibus Disk Controller")
