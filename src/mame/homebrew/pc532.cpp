// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * George Scolaro and Dave Rand's 532 Baby AT, aka pc532.
 *
 * Sources:
 *  - http://www.cpu-ns32k.net/PC532.html
 *  - https://www.netbsd.org/ports/pc532/faq.html
 *  - https://www.nic.funet.fi/pub/misc/pc532/
 *
 * TODO:
 *  - et532
 *
 * WIP:
 *  - NetBSD 1.5.3 boots from install floppy
 */

#include "emu.h"

// cpu cluster
#include "cpu/ns32000/ns32000.h"
#include "machine/ns32081.h"
#include "machine/ns32202.h"

// other devices
#include "machine/aic6250.h"
#include "machine/ds1315.h"
#include "machine/input_merger.h"
#include "machine/mc68681.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"

// busses and connectors
#include "bus/rs232/rs232.h"
#include "bus/nscsi/hd.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class pc532_state : public driver_device
{
public:
	pc532_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_fpu(*this, "fpu")
		, m_icu(*this, "icu")
		, m_rtc(*this, "rtc")
		, m_ncr5380(*this, "slot:7:ncr5380")
		, m_aic6250(*this, "scsi:0:aic6250")
		, m_duart(*this, "duart%u", 0U)
		, m_serial(*this, "serial%u", 0U)
		, m_duar(*this, "duar%u", 0U)
		, m_eprom(*this, "eprom")
		, m_swap(*this, "swap")
		, m_select(*this, "select")
	{
	}

	void pc532(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;

	required_device<ns32532_device> m_cpu;
	required_device<ns32381_device> m_fpu;
	required_device<ns32202_device> m_icu;

	required_device<ds1315_device> m_rtc;

	required_device<ncr5380_device> m_ncr5380;
	required_device<aic6250_device> m_aic6250;

	required_device_array<scn2681_device, 4> m_duart;
	required_device_array<rs232_port_device, 8> m_serial;
	required_device_array<input_merger_any_low_device, 4> m_duar;

	required_region_ptr<u32> m_eprom;

private:
	void drq_w(int state);
	void irq_w(int state);
	u32 dma_r(offs_t offset, u32 mem_mask);
	void dma_w(offs_t offset, u32 data, u32 mem_mask);

	memory_view m_swap;
	memory_view m_select;

	bool m_drq;
	bool m_irq;
	u32 m_dma;
	enum dma_state : unsigned
	{
		IDLE, // pseudo-DMA buffer is empty

		WR1,  // 1 byte remains in output buffer
		WR2,  // 2 bytes remain in output buffer
		WR3,  // 3 bytes remain in output buffer

		RD1,  // 1 byte available in input buffer
		RD2,  // 2 bytes available in input buffer
		RD3,  // 3 bytes available in input buffer
		RD4,  // input buffer is full
	}
	m_state;
};

void pc532_state::machine_start()
{
	// install phantom rtc using memory taps
	// TODO: not tested
	m_cpu->space(AS_PROGRAM).install_read_tap(0x1000'0000, 0x1000'0003, "rtc_w",
		[this](offs_t offset, u32 &data, u32 mem_mask)
		{
			m_rtc->write_data(offset & 1);
		});
	m_cpu->space(AS_PROGRAM).install_read_tap(0x1000'0004, 0x1000'0007, "rtc_r",
		[this](offs_t offset, u32 &data, u32 mem_mask)
		{
			if (m_rtc->chip_enable())
				data = m_rtc->read_data();
		});
}

void pc532_state::machine_reset()
{
	m_drq = false;
	m_irq = false;
	m_dma = 0;
	m_state = IDLE;

	// /int15 is tied high
	m_icu->ir_w<15>(1);
}

/*
 * The pc532 relies on the dynamic bus sizing and cycle extension features of
 * the NS32532 to implement CPU-driven pseudo-DMA for the SCSI controller(s).
 * In software, the CPU checks the DRQ and IRQ state of the SCSI controller
 * before performing doubleword accesses to the pseudo-DMA memory region, which
 * activates the necessary glue hardware. The hardware responds by signalling
 * an 8-bit dynamic bus size, which causes the CPU to execute a total of four
 * byte-wide accesses corresponding to each byte in the double word, while also
 * asserting the /RDY line as needed to synchronize the data transfers with the
 * DRQ line from the SCSI chip. /RDY is deasserted on SCSI interrupt, allowing
 * transfers of less than 4 bytes to be completed (e.g., at phase change).
 */
void pc532_state::drq_w(int state)
{
	if (state)
	{
		switch (m_state)
		{
		case RD1: m_dma |= u32(m_ncr5380->dma_r()) << 8; m_state = RD2; break;
		case RD2: m_dma |= u32(m_ncr5380->dma_r()) << 16; m_state = RD3; break;
		case RD3: m_dma |= u32(m_ncr5380->dma_r()) << 24; m_state = RD4; break;

		case WR3: m_ncr5380->dma_w(m_dma >> 8); m_state = WR2; break;
		case WR2: m_ncr5380->dma_w(m_dma >> 16); m_state = WR1; break;
		case WR1: m_ncr5380->dma_w(m_dma >> 24); m_state = IDLE; break;

		default:
			break;
		}
	}

	m_drq = state;
}

void pc532_state::irq_w(int state)
{
	if (state)
	{
		switch (m_state)
		{
		case RD1:
		case RD2:
		case RD3:
			// mark buffer full to enable reads of less than 4 bytes to complete
			m_state = RD4;
			break;

		case WR3:
		case WR2:
		case WR1:
			// discard untransferred buffer data for writes of less than 4 bytes
			m_state = IDLE;
			break;

		default:
			break;
		}
	}

	m_irq = state;
}

// TODO: byte and word accesses
// TODO: A22 -> EOP
u32 pc532_state::dma_r(offs_t offset, u32 mem_mask)
{
	u32 data = 0;

	if (!machine().side_effects_disabled())
	{
		switch (m_state)
		{
		case IDLE:
			if (m_drq && !m_irq)
			{
				// buffer empty and SCSI ready to transfer; read SCSI data, enter the read state, and signal the CPU to wait
				m_dma = m_ncr5380->dma_r();
				m_state = RD1;

				m_cpu->rdy_w(1);
			}
			break;

		case RD1:
		case RD2:
		case RD3:
			// buffer partly full; signal the CPU to wait
			m_cpu->rdy_w(1);
			break;

		case RD4:
			// buffer full; return the data to the CPU and enter idle state
			data = m_dma;
			m_state = IDLE;
			break;

		default:
			break;
		}
	}

	return data;
}

// TODO: byte and word accesses
// TODO: A22 -> EOP
void pc532_state::dma_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_state == IDLE)
	{
		if (m_drq)
		{
			m_dma = data;
			m_ncr5380->dma_w(m_dma >> 0);
			m_state = WR3;
		}
	}
	else
		m_cpu->rdy_w(1);
}

template <unsigned ST> void pc532_state::cpu_map(address_map &map)
{
	if (ST == AS_PROGRAM)
	{
		map(0x0000'0000, 0x01ff'ffff).view(m_swap);
		m_swap[0](0x0000'0000, 0x01ff'ffff).ram().share("ram");
		m_swap[1](0x0000'0000, 0x0000'7fff).rom().region("eprom", 0);
	}
	else
		map(0x0000'0000, 0x01ff'ffff).ram().share("ram");

	map(0x1000'0000, 0x1000'7fff).rom().region("eprom", 0);

	map(0x2800'0000, 0x2800'000f).rw(m_duart[0], FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x2800'0010, 0x2800'001f).rw(m_duart[1], FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x2800'0020, 0x2800'002f).rw(m_duart[2], FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x2800'0030, 0x2800'003f).rw(m_duart[3], FUNC(scn2681_device::read), FUNC(scn2681_device::write));

	map(0x2800'0050, 0x2800'0053).noprw(); // clear NMI

	if (ST == ns32000::ST_ODT)
	{
		map(0x3000'0000, 0x3fff'ffff).view(m_select);
		m_select[0](0x3000'0000, 0x3000'0007).m(m_ncr5380, FUNC(ncr5380_device::map));
		m_select[0](0x3800'0000, 0x3fff'ffff).rw(FUNC(pc532_state::dma_r), FUNC(pc532_state::dma_w));
		m_select[1](0x3000'0000, 0x3000'0001).rw(m_aic6250, FUNC(aic6250_device::read), FUNC(aic6250_device::write));
	}

	map(0xffff'fe00, 0xffff'feff).m(m_icu, FUNC(ns32202_device::map<BIT(ST, 1)>));
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
}

void pc532_state::pc532(machine_config &config)
{
	NS32532(config, m_cpu, 50_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &pc532_state::cpu_map<0>);
	m_cpu->set_addrmap(ns32000::ST_IAM, &pc532_state::cpu_map<ns32000::ST_IAM>);
	// ODT required because system software uses explicit MOV instead of RETI instructions
	m_cpu->set_addrmap(ns32000::ST_ODT, &pc532_state::cpu_map<ns32000::ST_ODT>);

	NS32381(config, m_fpu, 50_MHz_XTAL / 2);
	m_cpu->set_fpu(m_fpu);

	NS32202(config, m_icu, 3.6864_MHz_XTAL);
	m_icu->out_int().set_inputline(m_cpu, INPUT_LINE_IRQ0).invert();
	m_icu->out_g<0>().set([this](int state) { m_swap.select(state); });
	m_icu->out_g<7>().set([this](int state) { m_select.select(state); });

	DS1315(config, m_rtc, 32.768_kHz_XTAL);

	NSCSI_BUS(config, "slot");
	NSCSI_CONNECTOR(config, "slot:0", scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "slot:1", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "slot:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "slot:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "slot:7").option_set("ncr5380", NCR5380).machine_config( // DP8490
		[this](device_t *device)
		{
			ncr5380_device &ncr5380(downcast<ncr5380_device &>(*device));

			ncr5380.drq_handler().set(*this, FUNC(pc532_state::drq_w));
			ncr5380.irq_handler().append(m_icu, FUNC(ns32202_device::ir_w<4>));
			ncr5380.irq_handler().append(*this, FUNC(pc532_state::irq_w));
		});

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0").option_set("aic6250", AIC6250).machine_config(
		[this](device_t *device)
		{
			aic6250_device &aic6250(downcast<aic6250_device &>(*device));

			aic6250.set_clock(20_MHz_XTAL);
			aic6250.int_cb().set(m_icu, FUNC(ns32202_device::ir_w<5>));
			// TODO: drq
		});
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7", scsi_devices, nullptr, false);

	SCN2681(config, m_duart[0], 3.6864_MHz_XTAL).irq_cb().set(m_icu, FUNC(ns32202_device::ir_w<13>)).invert();
	SCN2681(config, m_duart[1], 3.6864_MHz_XTAL).irq_cb().set(m_icu, FUNC(ns32202_device::ir_w<11>)).invert();
	SCN2681(config, m_duart[2], 3.6864_MHz_XTAL).irq_cb().set(m_icu, FUNC(ns32202_device::ir_w<9>)).invert();
	SCN2681(config, m_duart[3], 3.6864_MHz_XTAL).irq_cb().set(m_icu, FUNC(ns32202_device::ir_w<7>)).invert();

	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[2], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[3], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[4], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[5], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[6], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[7], default_rs232_devices, nullptr);

	INPUT_MERGER_ANY_LOW(config, m_duar[0]).output_handler().set(m_icu, FUNC(ns32202_device::ir_w<12>)).invert();
	INPUT_MERGER_ANY_LOW(config, m_duar[1]).output_handler().set(m_icu, FUNC(ns32202_device::ir_w<10>)).invert();
	INPUT_MERGER_ANY_LOW(config, m_duar[2]).output_handler().set(m_icu, FUNC(ns32202_device::ir_w<8>)).invert();
	INPUT_MERGER_ANY_LOW(config, m_duar[3]).output_handler().set(m_icu, FUNC(ns32202_device::ir_w<6>)).invert();

	for (unsigned i = 0; i < std::size(m_duart); i++)
	{
		// channel A outputs
		m_duart[i]->a_tx_cb().set(m_serial[i * 2 + 0], FUNC(rs232_port_device::write_txd));
		m_duart[i]->outport_cb().append(m_serial[i * 2 + 0], FUNC(rs232_port_device::write_rts)).bit(0);
		m_duart[i]->outport_cb().append(m_serial[i * 2 + 0], FUNC(rs232_port_device::write_dtr)).bit(2);
		m_duart[i]->outport_cb().append(m_duar[i], FUNC(input_merger_any_low_device::in_w<0>)).bit(4);

		// channel A inputs
		m_serial[i * 2 + 0]->rxd_handler().set(m_duart[i], FUNC(scn2681_device::rx_a_w));
		m_serial[i * 2 + 0]->cts_handler().set(m_duart[i], FUNC(scn2681_device::ip0_w));
		m_serial[i * 2 + 0]->dcd_handler().set(m_duart[i], FUNC(scn2681_device::ip3_w));

		// channel B outputs
		m_duart[i]->b_tx_cb().set(m_serial[i * 2 + 1], FUNC(rs232_port_device::write_txd));
		m_duart[i]->outport_cb().append(m_serial[i * 2 + 1], FUNC(rs232_port_device::write_rts)).bit(1);
		m_duart[i]->outport_cb().append(m_serial[i * 2 + 1], FUNC(rs232_port_device::write_dtr)).bit(3);
		m_duart[i]->outport_cb().append(m_duar[i], FUNC(input_merger_any_low_device::in_w<1>)).bit(5);

		// channel B inputs
		m_serial[i * 2 + 1]->rxd_handler().set(m_duart[i], FUNC(scn2681_device::rx_b_w));
		m_serial[i * 2 + 1]->cts_handler().set(m_duart[i], FUNC(scn2681_device::ip1_w));
		m_serial[i * 2 + 1]->dcd_handler().set(m_duart[i], FUNC(scn2681_device::ip2_w));
	}
}

ROM_START(pc532)
	ROM_REGION32_LE(0x8000, "eprom", 0)

	ROM_SYSTEM_BIOS(0, "911113-9600", "Wed Nov 13 21:45:02 1991, Phil Nelson, 9600bps")
	ROMX_LOAD("911105_9600.u44", 0x0000, 0x8000, CRC(e927cac7) SHA1(90bf5d0e1e86f2a75f7abd4ced7edf8794fa89e5), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "900328-9600", "Wed Mar 28 09:31:00 PST 1990, Bruce Culbertson, 9600bps")
	ROMX_LOAD("900328_9600.u44", 0x0000, 0x8000, CRC(63caac86) SHA1(5c7011684b1bce3dd6b5fcf3c81479e40c61c4e3), ROM_BIOS(1))
ROM_END

} // anonymous namespace

/*   YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY           FULLNAME  FLAGS */
COMP(1989, pc532, 0,      0,      pc532,   0,     pc532_state, empty_init, "George Scolaro", "pc532",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
