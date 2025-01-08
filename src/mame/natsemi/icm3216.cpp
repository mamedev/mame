// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * National Semiconductor ICM-3216 Integrated Computer Module
 *
 * Sources:
 *  - ICM-3216 Integrated Computer Module, January 1986, National Semiconductor (DS6786S-10M16)
 *  - ICM-3216 CPU Board Specification, revision PB, 10/17/85, National Semiconductor Corporation (426610289-000)
 *
 * TODO:
 *  - configurable ram size (1M, 2M, 4M, 8M)
 *  - iop and scsi
 *  - cpu timing
 *  - minibus
 *
 * WIP:
 *  - passes memory/parity diagnostics
 *  - boots to monitor (press Ctrl+C after beep)
 *  - enter $ to show version/memory information
 *  - other commands include: a,b,c,d,e,i,l,m,p,r,t,w,v and C,P,T,V
 */

#include "emu.h"

// cpus and memory
#include "cpu/ns32000/ns32000.h"
#include "cpu/z80/z80.h"

// various hardware
#include "machine/mc68681.h"
#include "machine/mm58274c.h"
#include "machine/ncr5385.h"
#include "machine/ns32081.h"
#include "machine/ns32082.h"
#include "machine/ns32202.h"

// busses and connectors
#include "bus/rs232/rs232.h"
#include "bus/nscsi/hd.h"
#include "machine/nscsi_bus.h"

#define LOG_PARITY (1U << 1)

//#define VERBOSE (LOG_GENERAL|LOG_PARITY)
#include "logmacro.h"

namespace {

class icm3216_state : public driver_device
{
public:
	icm3216_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_fpu(*this, "fpu")
		, m_mmu(*this, "mmu")
		, m_icu(*this, "icu")
		, m_rtc(*this, "rtc")
		, m_duart(*this, "duart%u", 0U)
		, m_serial(*this, "serial%u", 0U)
		, m_iop(*this, "iop")
		, m_scsi(*this, "scsi:7:ncr5385")
		, m_led(*this, "led%u", 1U)
		, m_boot(*this, "boot")
	{
	}

	// machine config
	void icm3216(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;
	void iop_mem_map(address_map &map) ATTR_COLD;
	void iop_pio_map(address_map &map) ATTR_COLD;

	// register read handlers
	u8 iop_r();
	u16 nmi_status_r();
	u8 nmi_enable_r();

	// register write handlers
	void iop_w(u8 data);
	void parity_select_w(u8 data);
	void parity_enable_w(u8 data);

	// parity handlers
	void parity_r(offs_t offset, u16 &data, u16 mem_mask);
	void parity_w(offs_t offset, u16 &data, u16 mem_mask);

	IRQ_CALLBACK_MEMBER(iop_ack);
	template <unsigned Source> void iop_int(int state);

private:
	required_device<ns32016_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<ns32202_device> m_icu;

	required_device<mm58274c_device> m_rtc;
	required_device_array<scn2681_device, 2> m_duart;
	required_device_array<rs232_port_device, 4> m_serial;

	required_device<z80_device> m_iop;
	required_device<ncr5385_device> m_scsi;

	output_finder<5> m_led;

	memory_view m_boot;
	memory_passthrough_handler m_boot_mph;

	enum nmi_status_mask : u16
	{
		NMI_CHIP   = 0x000f, // parity error chip(s)
		NMI_EVEN   = 0x0010, // parity error on even byte
		NMI_ODD    = 0x0020, // parity error on odd byte
		NMI_MMU    = 0x0040, // mmu interrupt
		NMI_MBIC   = 0x0080, // mbic interrupt
		NMI_PARITY = 0x0100, // parity error
	};

	enum iop_status_mask : u8
	{
		IOP_IID = 0x07, // subchannel interrupt ID
		IOP_IRS = 0x10, // interrupt request
		IOP_RST = 0x20, // scsi reset
		IOP_ABT = 0x40, // scsi abort interrupt?
		IOP_BSY = 0x80, // busy
	};

	// machine registers
	u8 m_iop_cmd;
	u8 m_iop_sts;
	u8 m_iop_vec;

	u16 m_nmi_state;      // non-maskable interrupt status register

	bool m_nmi_enable;    // non-maskable interrupt enable
	bool m_parity_enable; // parity checking enable
	bool m_parity_select; // parity mode even/odd

	// other internal state
	s32 m_parity_delta; // parity count increment/decrement
	u32 m_parity_count; // count mismatched parity writes

	std::unique_ptr<u8[]> m_parity;
	memory_passthrough_handler m_parity_mph;

	static constexpr unsigned RAM_SIZE = 0x400000;
};

void icm3216_state::machine_start()
{
	m_led.resolve();

	save_item(NAME(m_iop_cmd));
	save_item(NAME(m_iop_sts));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_parity_enable));
	save_item(NAME(m_parity_select));
	save_item(NAME(m_parity_delta));
	save_item(NAME(m_parity_count));

	save_pointer(NAME(m_parity), RAM_SIZE);

	m_iop_cmd = 0;
	m_iop_sts = 0;
	m_iop_vec = 0;
}

void icm3216_state::machine_reset()
{
	m_boot.select(0);

	m_boot_mph = m_cpu->space(0).install_read_tap(0x800000, 0xffffff, "boot",
		[this](offs_t offset, uint16_t &data, uint16_t mem_mask)
		{
			if (!machine().side_effects_disabled())
			{
				m_boot.disable();
				m_boot_mph.remove();
			}
		});

	m_duart[0]->ip6_w(1); // TODO: what's this?

	m_nmi_state = 0;

	m_nmi_enable = true;
	m_parity_enable = false;
	m_parity_select = false;

	m_parity_delta = 0;
	m_parity_count = 0;

	m_parity.reset();
	m_parity_mph.remove();
}

template <unsigned ST> void icm3216_state::cpu_map(address_map &map)
{
	map(0x00'0000, RAM_SIZE - 1).ram();
	map(RAM_SIZE, 0x7f'ffff).noprw();

	if (ST == 0)
	{
		map(0x00'0000, 0x01'ffff).view(m_boot);
		m_boot[0](0x00'0000, 0x01'ffff).rom().region("eprom", 0);
	}

	map(0x80'0000, 0x81'ffff).rom().region("eprom", 0).mirror(0x100000);

	map(0xa0'0000, 0xa0'001f).umask16(0x00ff).rw(m_rtc, FUNC(mm58274c_device::read), FUNC(mm58274c_device::write));
	map(0xa0'0020, 0xa0'003f).umask16(0x00ff).rw(m_duart[0], FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0xa0'0040, 0xa0'005f).umask16(0x00ff).rw(m_duart[1], FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	//map(0xa0'0080, 0xa0'0083); // parallel

	map(0xa0'00a0, 0xa0'00a1).umask16(0x00ff).rw(FUNC(icm3216_state::iop_r), FUNC(icm3216_state::iop_w));
	map(0xa0'00c0, 0xa0'00c1).r(FUNC(icm3216_state::nmi_status_r)); // TODO: minibus hold (w)
	map(0xa0'00c2, 0xa0'00cb).umask16(0x00ff).lw8(NAME([this](offs_t offset, u8 data) { m_led[offset] = data; })); // TODO: minibus reset
	map(0xa0'00cc, 0xa0'00cc).w(FUNC(icm3216_state::parity_select_w));
	map(0xa0'00ce, 0xa0'00ce).w(FUNC(icm3216_state::parity_enable_w));

	map(0xa0'00e0, 0xa0'00e0).r(FUNC(icm3216_state::nmi_enable_r)); // TODO: minibus mhl/mcl (w)

	//map(0xc0'0000, 0xfd'ffff); // minibus memory 0x000000-0x3dffff
	//map(0xfe'0000, 0xfe'ffff); // minibus 8-bit i/o
	//map(0xff'0000, 0xff'7fff); // minibus 16-bit i/o

	map(0xff'fe00, 0xff'feff).m(m_icu, FUNC(ns32202_device::map<BIT(ST, 1)>)).umask16(0x00ff);

	if (ST == 4)
		map(0xff'ff00, 0xff'ff01).nopr(); // silence cpu nmi vector
}

void icm3216_state::iop_mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("iop", 0);
	map(0x4000, 0x7fff).ram();

	//map(0xc010, 0xc010).lr8(NAME([this]() { logerror("0xc010 0x%02x (%s)\n", m_iop_cmd, machine().describe_context()); return m_iop_cmd; }));
	//map(0xc011, 0xc011).lr8(NAME([this]() { logerror("0xc011 0x%02x (%s)\n", 0x40, machine().describe_context()); return 0x40; }));
	map(0xc011, 0xc011).nopr(); // FIXME: silence

	// c010 read (after interrupt)
	// c011 read (test 0x40, test 0x80 before read16), write16 (on nmi)
	// c012 read16?
	// c013 write 0x80
	//map(0xc013, 0xc013).lw8(NAME([this](u8 data) { logerror("0xc013 0x%02x (%s)\n", data, machine().describe_context()); m_iop_sts &= ~data; }));
	// c014 write (on nmi)
	// c015 write 0x08
	// c017 write 0x00 (after read16?, clear BSY?)

	map(0xc020, 0xc02f).m(m_scsi, FUNC(ncr5385_device::map));
}

void icm3216_state::iop_pio_map(address_map &map)
{
}

static INPUT_PORTS_START(icm3216)
INPUT_PORTS_END

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
}


void icm3216_state::icm3216(machine_config &config)
{
	NS32016(config, m_cpu, 20_MHz_XTAL / 2);
	m_cpu->set_addrmap(0, &icm3216_state::cpu_map<0>);
	m_cpu->set_addrmap(4, &icm3216_state::cpu_map<4>);

	NS32081(config, m_fpu, 20_MHz_XTAL / 2);
	m_cpu->set_fpu(m_fpu);

	NS32082(config, m_mmu, 20_MHz_XTAL / 2);
	m_cpu->set_mmu(m_mmu);

	// 8 bit mode, irq: 4 serial, 1 minibus, 1 scsi, 1 printer, 1 rtc, 8 free
	NS32202(config, m_icu, 18.432_MHz_XTAL / 10);
	m_icu->out_int().set_inputline(m_cpu, INPUT_LINE_IRQ0).invert();

	MM58274C(config, m_rtc, 0);

	// we are dte, therefore: tx,rx,rts,cts,dsr,dtr,dcd
	// rts o
	// cts i
	// dsr i
	// rlsd i
	// dtr o

	SCN2681(config, m_duart[0], 3'686'400); // SCN2681A
	m_duart[0]->irq_cb().set([this](int state) { logerror("irq %d\n", state); });
	m_duart[0]->a_tx_cb().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_duart[0]->b_tx_cb().set(m_serial[1], FUNC(rs232_port_device::write_txd));

	SCN2681(config, m_duart[1], 3'686'400); // SCN2681A
	m_duart[1]->irq_cb().set([this](int state) { logerror("irq %d\n", state); });
	m_duart[1]->a_tx_cb().set(m_serial[2], FUNC(rs232_port_device::write_txd));
	m_duart[1]->b_tx_cb().set(m_serial[3], FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_serial[0], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_serial[0]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_a_w));
	m_serial[1]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_b_w));

	RS232_PORT(config, m_serial[2], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[3], default_rs232_devices, "terminal");
	m_serial[2]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_a_w));
	m_serial[3]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_b_w));

	Z80(config, m_iop, 4'000'000);
	m_iop->set_addrmap(AS_PROGRAM, &icm3216_state::iop_mem_map);
	m_iop->set_addrmap(AS_IO, &icm3216_state::iop_pio_map);
	m_iop->set_irq_acknowledge_callback(FUNC(icm3216_state::iop_ack));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5385", NCR5385).machine_config(
		[this](device_t *device)
		{
			ncr5385_device &ncr5385(downcast<ncr5385_device &>(*device));

			ncr5385.set_clock(10'000'000);

			ncr5385.irq().set(*this, FUNC(icm3216_state::iop_int<2>));
			ncr5385.dreq().set_inputline(m_iop, INPUT_LINE_NMI);
		});
}

void icm3216_state::parity_select_w(u8 data)
{
	LOGMASKED(LOG_PARITY, "parity %s (%s)\n", BIT(data, 0) ? "odd" : "even", machine().describe_context());

	if (m_parity_select != BIT(data, 0))
	{
		if (!m_parity)
		{
			LOGMASKED(LOG_PARITY, "parity handlers installed\n");

			// allocate and initialise parity store
			m_parity = std::make_unique<u8[]>(RAM_SIZE >> 3);
			if (m_parity_select)
				std::fill_n(m_parity.get(), RAM_SIZE >> 3, u8(0xff));

			// install parity handlers
			m_parity_mph = m_cpu->space(0).install_readwrite_tap(0, RAM_SIZE - 1, "parity",
				std::bind(&icm3216_state::parity_r, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
				std::bind(&icm3216_state::parity_w, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

			// count mismatched parity writes
			m_parity_delta = 1;
		}
		else if (!m_parity_count)
		{
			LOGMASKED(LOG_PARITY, "parity handlers removed\n");

			m_parity_mph.remove();
			m_parity.reset();

			m_parity_delta = 0;
		}
		else
			// invert mismatched parity count direction
			m_parity_delta *= -1;

		m_parity_select = BIT(data, 0);
	}
}

void icm3216_state::parity_enable_w(u8 data)
{
	LOGMASKED(LOG_PARITY, "parity %s (%s)\n", BIT(data, 0) ? "enabled" : "disabled", machine().describe_context());

	m_parity_enable = BIT(data, 0);
}

void icm3216_state::parity_r(offs_t offset, u16 &data, u16 mem_mask)
{
	if (!machine().side_effects_disabled() && m_parity_enable && !m_nmi_state)
	{
		LOGMASKED(LOG_PARITY, "parity r 0x%06x mask 0x%04x %s (%s)\n",
			offset, mem_mask, m_parity_select ? "odd" : "even", machine().describe_context());

		for (unsigned byte = 0; byte < 2; byte++)
		{
			if (BIT(mem_mask, byte * 8, 8))
			{
				if (BIT(m_parity[offset >> 3], BIT(offset, 1, 2) * 2 + byte) != m_parity_select)
					m_nmi_state |= NMI_PARITY | (NMI_EVEN << byte);
			}
		}

		if (m_nmi_state && m_nmi_enable)
		{
			LOGMASKED(LOG_PARITY, "parity error 0x%06x (%s)\n", offset, machine().describe_context());

			m_cpu->set_input_line(INPUT_LINE_NMI, 1);
		}
	}
}

void icm3216_state::parity_w(offs_t offset, u16 &data, u16 mem_mask)
{
	LOGMASKED(LOG_PARITY, "parity w 0x%06x mask 0x%04x %s (%s)\n",
		offset, mem_mask, m_parity_select ? "odd" : "even", machine().describe_context());

	for (unsigned byte = 0; byte < 2; byte++)
	{
		unsigned const parity_bit = BIT(offset, 1, 2) * 2 + byte;

		if (BIT(mem_mask, byte * 8, 8) && BIT(m_parity[offset >> 3], parity_bit) != m_parity_select)
		{
			if (m_parity_select)
				m_parity[offset >> 3] |= 1U << parity_bit;
			else
				m_parity[offset >> 3] &= ~(1U << parity_bit);

			m_parity_count += m_parity_delta;
		}
	}

	if (!m_parity_count)
	{
		LOGMASKED(LOG_PARITY, "parity handlers removed\n");

		m_parity_mph.remove();
		m_parity.reset();

		m_parity_delta = 0;
	}
}

u16 icm3216_state::nmi_status_r()
{
	u16 const data = m_nmi_state;

	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_PARITY, "nmi status 0x%04x (%s)\n", data, machine().describe_context());

		m_nmi_state = 0;
	}

	return data;
}

u8 icm3216_state::nmi_enable_r()
{
	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_PARITY, "nmi enable (%s)\n", machine().describe_context());

		m_nmi_enable = true;
	}

	return 0;
}

u8 icm3216_state::iop_r()
{
	return m_iop_sts;
}

void icm3216_state::iop_w(u8 data)
{
	/*
	 * cmd  function
	 *  0x00   write command pointer table (followed by address, lsb first)
	 *  0x01   acknowledge interrupt
	 *  0x02   reset i/o controller?
	 *  0x03   scsi reset
	 *  0x05   reset i/o controller?
	 *  0x1n   start i/o subchannel n
	 *  0x2n   abort i/o subchannel n
	 */
	LOG("iop_w 0x%02x (%s)\n", data, machine().describe_context());
	m_iop_cmd = data;

	m_iop_sts |= IOP_BSY;

	iop_int<1>(1);
}

// iop interrupt vector bits
// bit  source
//  1   host?
//  2   scsi
//  3   reset?

IRQ_CALLBACK_MEMBER(icm3216_state::iop_ack)
{
	u8 const data = m_iop_vec;

	m_iop->set_input_line(INPUT_LINE_IRQ0, 0);

	return data;
}

template <unsigned Source> void icm3216_state::iop_int(int state)
{
	if (state)
		m_iop_vec |= 1U << Source;
	else
		m_iop_vec &= ~(1U << Source);

	m_iop->set_input_line(INPUT_LINE_IRQ0, state);
}

ROM_START(icm3216)
	ROM_REGION16_LE(0x20000, "eprom", 0)

	ROM_SYSTEM_BIOS(0, "v244", "ICM-3216 Rom Monitor V2.44")
	ROMX_LOAD("610289_002__rev_b_u34.u34", 0x0000, 0x8000, CRC(fb542bd1) SHA1(39b7f1f90ffdae07ddc547eff564096bd2092d58), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("610289_002__rev_b_u35.u35", 0x0001, 0x8000, CRC(78cfc8dd) SHA1(bb87f068a6ab7ef82d8865154856064b9e8a99a6), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_REGION(0x4000, "iop", 0)
	ROM_LOAD("600045_003__rev_a.u29", 0x0000, 0x4000, CRC(865e3e8b) SHA1(c6e47d304fd67a9b384a139ef917913fc60c0b32))
ROM_END

}

/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                   FULLNAME    FLAGS */
COMP(1985, icm3216, 0,      0,      icm3216, icm3216, icm3216_state, empty_init, "National Semiconductor", "ICM-3216", MACHINE_NOT_WORKING)
