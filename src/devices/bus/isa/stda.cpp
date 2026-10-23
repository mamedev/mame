// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM RT PC Streaming Tape Drive Adapter
 *
 * Sources:
 *  - IBM RT PC Hardware Technical Reference, Volume II (75X0235), Second Edition (September 1986), International Business Machines Corporation.
 *
 * TODO:
 *  - parity
 *  - shared interrupt
 */

#include "emu.h"
#include "stda.h"

#include "bus/qic02/qic02.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/input_merger.h"
#include "machine/pls105.h"

#define LOG_REGR (1U << 1)
#define LOG_REGW (1U << 2)
#define LOG_DATA (1U << 3)

//#define VERBOSE (LOG_GENERAL|LOG_REGR|LOG_REGW|LOG_DATA)
#include "logmacro.h"

namespace {

static constexpr unsigned BUF_SIZE = 0x200U;
static constexpr unsigned BUF_MASK = BUF_SIZE - 1;

enum status_mask : u8
{
	STATUS_IBF = 0x01, // input buffer full (A)
	STATUS_OBF = 0x02, // output buffer full (A)
	STATUS_INT = 0x08, // interrupt request (A)
	STATUS_EXC = 0x10, // exception (D)
	STATUS_ONL = 0x20, // online (D)
	STATUS_RDY = 0x40, // ready (D)
	STATUS_BSY = 0x80, // busy (A)
};
enum control_mask : u8
{
	CONTROL_ENI = 0x01, // enable interrupt
	CONTROL_RST = 0x02, // reset attachment (active low)
	CONTROL_ENP = 0x04, // enable parity
};
enum p1_input_mask : u8
{
	P1_PARCK  = 0x10,
	P1_FAULT  = 0x20,
	P1_COMP   = 0x40,
	P1_INPUTS = 0x70,
};
enum p2_input_mask : u8
{
	P2_CNT512 = 0x80,
	P2_INPUTS = 0x80,
};

class isa16_stda_device
	: public device_t
	, public device_isa16_card_interface
{
public:
	isa16_stda_device(machine_config const &mconfig, char const *const tag, device_t *owner, u32 clock)
		: device_t(mconfig, ISA16_STDA, tag, owner, clock)
		, device_isa16_card_interface(mconfig, *this)
		, m_mcu(*this, "mcu")
		, m_pls(*this, "pls")
		, m_qic(*this, "qic")
		, m_irq(*this, "irq")
		, m_buf(nullptr)
		, m_status(0)
		, m_cnt(0)
		, m_p1(0)
		, m_p2(0)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void map_isa(address_map &map);

	// read handlers
	u16 buffer_r();
	u8 status_r();
	u8 data_r();

	// write handlers
	void buffer_w(u16 data);
	void control_w(u8 data);
	void data_w(u8 data);
	void command_w(u8 data);

	// board logic
	void p1_w(u8 data);
	void p2_w(u8 data);
	void count(int count);
	template <u8 Status> void status(int state);

private:
	required_device<i8742_device> m_mcu;
	required_device<pls105_device> m_pls;
	required_device<qic02_connector_device> m_qic;
	required_device<input_merger_device> m_irq;

	std::unique_ptr<u8[]> m_buf;

	u8 m_status;
	u8 m_control;

	u16 m_cnt;
	u8 m_p1;
	u8 m_p2;
};

void isa16_stda_device::device_add_mconfig(machine_config &config)
{
	I8742(config, m_mcu, 12_MHz_XTAL);
	m_mcu->t0_in_cb().set([this]() { return (m_status & STATUS_RDY) ? 0 : 1; });
	m_mcu->t1_in_cb().set([this]() { return (m_status & STATUS_EXC) ? 0 : 1; });
	m_mcu->p1_in_cb().set([this]() { return m_p1; });
	m_mcu->p2_in_cb().set([this]() { return m_p2; });
	m_mcu->p1_out_cb().set(FUNC(isa16_stda_device::p1_w));
	m_mcu->p2_out_cb().set(FUNC(isa16_stda_device::p2_w));

	PLS105(config, m_pls, 12_MHz_XTAL / 2);
	m_pls->f<0>().set([this](int state) { if (state) m_p1 |= P1_COMP; else m_p1 &= ~P1_COMP; });     // F0 -COMPLETE
	m_pls->f<1>().set([this](int state) { if (state) m_p1 |= P1_FAULT; else m_p1 &= ~P1_FAULT; });   // F1 -FAULT
	m_pls->f<2>().set([this](int state) { if (!state) count(1); });                                  // F2 -INC CTR
	m_pls->f<3>().set([this](int state) { if (!state) count(-m_cnt); });                             // F3 -CLR CNT
	m_pls->f<4>().set([this](int state) { if (!state) m_qic->data_w(m_buf[m_cnt & BUF_MASK]); });    // F4 -RAM RD SP
	m_pls->f<5>().set([this](int state) { if (!state) m_buf[m_cnt & BUF_MASK] = m_qic->data_r(); }); // F5 -RAM WR SP
	m_pls->f<6>().set([this](int state) { if (state) m_p1 |= P1_PARCK; else m_p1 &= ~P1_PARCK; });   // F6 -PAR CK
	m_pls->f<7>().set([this](int state) { m_qic->xfr_w(state); });                                   // F7 -XFR

	QIC02_CONNECTOR(config, m_qic);
	m_qic->ack().set(m_pls, FUNC(pls105_device::i_w<8>)); // I8 -ACK
	m_qic->rdy().set(FUNC(isa16_stda_device::status<STATUS_RDY>)).invert();
	m_qic->exc().set(FUNC(isa16_stda_device::status<STATUS_EXC>)).invert();
	m_qic->dir().set(m_pls, FUNC(pls105_device::i_w<1>)); // I1 -DIR

	INPUT_MERGER_ALL_HIGH(config, m_irq);
	m_irq->output_handler().set([this](int state) { m_isa->irq12_w(state); });
}

void isa16_stda_device::device_start()
{
	// AM9128-10PC x2
	m_buf = std::make_unique<u8[]>(BUF_SIZE);
	save_pointer(NAME(m_buf), BUF_SIZE);

	save_item(NAME(m_status));
	save_item(NAME(m_control));

	save_item(NAME(m_p1));
	save_item(NAME(m_p2));

	set_isa_device();

	m_isa->install_device(0x01e8, 0x01ef, *this, &isa16_stda_device::map_isa);
	// TODO: shared interrupt reset 0x6f4 (write only)
}

void isa16_stda_device::device_reset()
{
	m_control = 0;

	m_irq->in_w<0>(0);
	m_pls->pr_w(0);
}

void isa16_stda_device::map_isa(address_map &map)
{
	map(0, 1).rw(FUNC(isa16_stda_device::buffer_r), FUNC(isa16_stda_device::buffer_w)).flags(0x0001);
	map(2, 2).rw(FUNC(isa16_stda_device::status_r), FUNC(isa16_stda_device::control_w));
	map(4, 4).rw(FUNC(isa16_stda_device::data_r), FUNC(isa16_stda_device::data_w));
	map(6, 6).w(FUNC(isa16_stda_device::command_w));
}

u16 isa16_stda_device::buffer_r()
{
	u16 const data = u16(m_buf[(m_cnt + 0) & BUF_MASK]) << 8 | m_buf[(m_cnt + 1) & BUF_MASK];

	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_DATA, "%s: buffer_r 0x%04x\n", machine().describe_context(), data);
		count(2);
	}

	return data;
}

void isa16_stda_device::buffer_w(u16 data)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_DATA, "%s: buffer_w 0x%04x\n", machine().describe_context(), data);

	m_buf[(m_cnt + 0) & BUF_MASK] = data >> 8;
	m_buf[(m_cnt + 1) & BUF_MASK] = u8(data);

	count(2);
}

u8 isa16_stda_device::status_r()
{
	u8 const data = m_status;

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REGR, "%s: status_r 0x%02x\n", machine().describe_context(), data);

	return data;
}

void isa16_stda_device::control_w(u8 data)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REGW, "%s: control_w 0x%02x\n", machine().describe_context(), data);

	if ((data ^ m_control) & CONTROL_ENI)
		m_irq->in_w<0>((data & CONTROL_ENI) ? 1 : 0);

	if ((data ^ m_control) & CONTROL_RST)
	{
		m_mcu->set_input_line(INPUT_LINE_RESET, (data & CONTROL_RST) ? 0 : 1);
		m_pls->i_w<3>((data & CONTROL_RST) ? 1 : 0); // I3 -RESET
	}

	if ((data ^ m_control) & CONTROL_ENP)
		m_pls->i_w<4>((data & CONTROL_ENP) ? 1 : 0); // I4 +ENABLE PARITY

	m_control = data;
}

u8 isa16_stda_device::data_r()
{
	if (!(m_status & STATUS_BSY) && !machine().side_effects_disabled())
	{
		u8 const data = m_mcu->upi41_master_r(0);
		LOGMASKED(LOG_REGR, "%s: data_r 0x%02x\n", machine().describe_context(), m_mcu->upi41_master_r(0));

		return data;
	}
	else
		return 0xff;
}

void isa16_stda_device::data_w(u8 data)
{
	if (!(m_status & STATUS_BSY))
	{
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_REGW, "%s: data_w 0x%02x\n", machine().describe_context(), data);

		m_mcu->upi41_master_w(0, data);
	}
}

void isa16_stda_device::command_w(u8 data)
{
	if ((VERBOSE & LOG_REGW) && !machine().side_effects_disabled())
	{
		static char const *const commands[] = {
			"RESET", "DRIVE SELECT", "REWIND", "ERASE", "RETENSION", "READ STATUS", "WRITE DATA", "WRITE FILE MARK",
			"READ DATA", "READ FILE MARK", nullptr, nullptr, "SKIP", "TERMINATE", "DRIVE SELF TEST 1", "DRIVE SELF TEST 2",
		};

		/*
		 * Undocumented commands:
		 *  0x10  drive self text 3 - write protect check
		 *  0x11  adapter passthrough to drive - REW timing
		 *  0x12  adapter passthrough to drive - REOF timing
		 *  0xaa  power-on reset handshake
		 *  0x89  srev?
		 */

		if (data < std::size(commands) && commands[data])
			LOGMASKED(LOG_REGW, "%s: command_w 0x%02x %s\n", machine().describe_context(), data, commands[data]);
		else
			LOGMASKED(LOG_REGW, "%s: command_w 0x%02x\n", machine().describe_context(), data);
	}

	if (!(m_status & STATUS_BSY))
		m_mcu->upi41_master_w(1, data);
}

void isa16_stda_device::p1_w(u8 data)
{
	if (u8 const delta = (data ^ m_p1) & ~P1_INPUTS)
	{
		// P10 /RD
		if (BIT(delta, 0) && !BIT(data, 0))
		{
			u8 const data = m_mcu->upi41_master_r(0);

			if (BIT(m_p2, 0))
				m_qic->data_w(data);
		}

		// P11 /WR
		if (BIT(delta, 1) && !BIT(data, 1))
		{
			u8 const data = BIT(m_p2, 0) ? 0xff : m_qic->data_r();

			m_mcu->upi41_master_w(0, data);
		}

		// P12 BUSY
		if (BIT(delta, 2))
			status<STATUS_BSY>(BIT(data, 2));

		// P13 -REQ
		if (BIT(delta, 3))
			m_qic->req_w(BIT(data, 3));

		// P17 -ONL
		if (BIT(delta, 7))
		{
			int const online = BIT(data, 7);

			status<STATUS_ONL>(!online);
			m_qic->onl_w(online);
		}

		m_p1 = (m_p1 & P1_INPUTS) | (data & ~P1_INPUTS);
	}
}

void isa16_stda_device::p2_w(u8 data)
{
	if (u8 const delta = (data ^ m_p2) & ~P2_INPUTS)
	{
		// P20 -DRV DIRC IN
		if (BIT(delta, 0))
			m_pls->i_w<2>(BIT(data, 0)); // I2 -DRV DIRC IN

		// P21 -INTERRUPT
		if (BIT(delta, 1))
		{
			int const interrupt = !BIT(data, 1);

			status<STATUS_INT>(interrupt);
			m_irq->in_w<1>(interrupt);
		}

		// P22 -RST
		if (BIT(delta, 2))
		{
			m_qic->rst_w(BIT(data, 2));
			m_pls->i_w<9>(BIT(data, 2)); // I9 -RST
		}

		// P23 -ENABLE
		if (BIT(delta, 3))
			m_pls->i_w<0>(BIT(data, 3)); // I0 -ENABLE

		// P24 +OBF
		if (BIT(delta, 4))
			status<STATUS_OBF>(BIT(data, 4));

		// P25 -IBF
		if (BIT(delta, 5))
			status<STATUS_OBF>(!BIT(data, 5));

		// P26 -CLR CNT
		if (BIT(delta, 6) && !BIT(data, 6))
			count(-m_cnt);

		m_p2 = (m_p2 & P2_INPUTS) | (data & ~P2_INPUTS);
	}
}

void isa16_stda_device::count(int count)
{
	m_cnt += count;

	// P27 +CNT 512
	if (BIT(m_cnt, 9))
		m_p2 |= P2_CNT512;
	else
		m_p2 &= ~P2_CNT512;

	m_pls->i_w<5>(BIT(m_cnt, 9)); // I5 +CNT 512
}

template <u8 Status> void isa16_stda_device::status(int state)
{
	if (state)
		m_status |= Status;
	else
		m_status &= ~Status;
}

ROM_START(stda)
	ROM_REGION(0x800, "mcu", 0)
	ROM_LOAD("p_n_22f9289__ec_a90593__c_copr_ibm_1987.u16", 0x000, 0x800, CRC(3054d259) SHA1(0b2179fc6ff83443f9e6157b268c36c1320d6120))

	ROM_REGION(0x1c1, "pls", 0)
	ROM_LOAD("pls105f.u12", 0x000, 0x1c1, CRC(c0a5ccb2) SHA1(d00dd711750b2e74663eabec551b4744a5669f8e))
ROM_END

const tiny_rom_entry *isa16_stda_device::device_rom_region() const
{
	return ROM_NAME(stda);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ISA16_STDA, device_isa16_card_interface, isa16_stda_device, "stda", "IBM RT PC Streaming Tape Drive Adapter")
