// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    GAYLE

    Gate array used in the Amiga 600 and Amiga 1200 computers for the
    following functions:
    - Address decoding and timing for ROM, RAM, CIA, RTC
      (and unused: Flash ROM, ArcNet)
    - Data buffers for chip RAM
    - IDE interface
    - Credit card (PCMCIA) interface
    - System reset
    - Floppy helper
    - E clock generation

    TODO:
    - Generate bus errors (REG_CHANGE, bit 0)
    - Disabling the PCMCIA interface should remove access to PCMCIA memory
    - Move more functionality here

***************************************************************************/

#include "emu.h"
#include "gayle.h"

#define LOG_REG     (1U << 1)
#define LOG_IDE     (1U << 2)
#define LOG_CC      (1U << 3)
#define LOG_ID      (1U << 4)

//#define VERBOSE (LOG_GENERAL | LOG_REG | LOG_CC)
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GAYLE, gayle_device, "gayle", "Amiga GAYLE")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  gayle_device - constructor
//-------------------------------------------------

gayle_device::gayle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GAYLE, tag, owner, clock),
	m_int2_w(*this),
	m_int6_w(*this),
	m_rst_w(*this),
	m_ide_cs_r_cb(*this, 0xffff),
	m_ide_cs_w_cb(*this),
	m_gayle_id(0xff),
	m_gayle_id_count(0),
	m_line_state(0),
	m_cd{1,1}
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gayle_device::device_start()
{
	save_item(NAME(m_gayle_id_count));
	save_item(NAME(m_gayle_reg));
	save_item(NAME(m_line_state));
	save_item(NAME(m_cd));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void gayle_device::device_reset()
{
	// internal registers are cleared to 0 on reset
	m_gayle_reg[0] = 0;
	m_gayle_reg[1] = 0;
	m_gayle_reg[2] = 0;
	m_gayle_reg[3] = 0;

	// lower interrupts
	m_int2_w(0);
	m_int6_w(0);
}


//**************************************************************************
//  ADDRESS MAP
//**************************************************************************

void gayle_device::register_map(address_map &map)
{
	// base address 0xdaXXXX
	map(0x0000, 0x001f).mirror(0x0fe0).rw(FUNC(gayle_device::ide_cs_r<1>), FUNC(gayle_device::ide_cs_w<1>));
	map(0x2000, 0x201f).mirror(0x0fe0).rw(FUNC(gayle_device::ide_cs_r<0>), FUNC(gayle_device::ide_cs_w<0>));
	map(0x8000, 0x8001).rw(FUNC(gayle_device::status_r), FUNC(gayle_device::status_w));
	map(0x9000, 0x9001).rw(FUNC(gayle_device::change_r), FUNC(gayle_device::change_w));
	map(0xa000, 0xa001).rw(FUNC(gayle_device::int_r), FUNC(gayle_device::int_w));
	map(0xb000, 0xb001).rw(FUNC(gayle_device::control_r), FUNC(gayle_device::control_w));
}


//**************************************************************************
//  INTERNAL
//**************************************************************************

void gayle_device::dump_register()
{
	LOGMASKED(LOG_REG, "status = %02x, change = %02x, int = %02x, control = %02x\n",
		m_gayle_reg[REG_STATUS], m_gayle_reg[REG_CHANGE], m_gayle_reg[REG_INT], m_gayle_reg[REG_CONTROL]);
}

void gayle_device::line_change(int line, int state, int level)
{
	LOGMASKED(line == LINE_IDE ? LOG_IDE : LOG_CC, "line_change: %d %d %d\n", line, state, level);

	m_line_state &= ~(1 << line);
	m_line_state |= (state << line);

	// ignore pcmcia line changes if the interface is disabled
	if (line != LINE_IDE && BIT(m_gayle_reg[REG_STATUS], 0))
		return;

	// did we change state?
	if (BIT(m_gayle_reg[REG_STATUS], line) != state)
	{
		// indicate a change to the line
		m_gayle_reg[REG_CHANGE] |= 1 << line;

		// special handling for line 6 (credit card detect)
		if (line == LINE_CC_DET && BIT(m_gayle_reg[REG_CHANGE], 1))
		{
			LOG("resetting due to credit card detection change\n");

			m_rst_w(0);
			m_rst_w(1);
			reset();

			return;
		}

		if (state)
			m_gayle_reg[REG_STATUS] |= 1 << line;
		else
			m_gayle_reg[REG_STATUS] &= ~(1 << line);

		// generate interrupt (if enabled)
		if (BIT(m_gayle_reg[REG_INT], line))
		{
			LOGMASKED(line == LINE_IDE ? LOG_IDE : LOG_CC, "line %d int assert\n", line);

			if (level == 2)
				m_int2_w(1);
			else
				m_int6_w(1);
		}
	}
}


//**************************************************************************
//  REGISTER
//**************************************************************************

uint16_t gayle_device::status_r()
{
	// 7-------  ide interrupt status
	// -6------  credit card detect
	// --5-----  battery voltage 1/status change
	// ---4----  battery voltage 2/digital audio
	// ----3---  credit card write enable
	// -----2--  credit card busy/interrupt request
	// ------1-  enable pcmcia digital audio
	// -------0  disable pcmcia

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG, "status_r: %02x\n", m_gayle_reg[REG_STATUS]);

	return (m_gayle_reg[REG_STATUS] << 8) | m_gayle_reg[REG_STATUS];
}

void gayle_device::status_w(uint16_t data)
{
	uint8_t previous = m_gayle_reg[REG_STATUS];
	data >>= 8;
	LOGMASKED(LOG_REG, "status_w: %02x\n", data);

	// pcmcia interface disable?
	if (BIT(data, 0) == 1)
		m_gayle_reg[REG_STATUS] &= 0x03;

	// lowest two bits can be set freely
	m_gayle_reg[REG_STATUS] &= ~0x03;
	m_gayle_reg[REG_STATUS] |= data & 0x03;

	// status bits can only be set
	m_gayle_reg[REG_STATUS] |= data & 0xfc;

	// pcmcia interface re-enabled?
	if (BIT(previous, 0) == 1 && BIT(data, 0) == 0)
	{
		cc_cd1_w(m_cd[0]);
		cc_cd2_w(m_cd[1]);
		cc_bvd1_w(!BIT(m_line_state, 5));
		cc_bvd2_w(!BIT(m_line_state, 4));
		cc_wp_w(!BIT(m_line_state, 3));
	}
}

uint16_t gayle_device::change_r()
{
	// 7-------  ide interrupt changed
	// -6------  credit card detect changed
	// --5-----  battery voltage 1/status change changed
	// ---4----  battery voltage 2/digital audio changed
	// ----3---  credit card write enable changed
	// -----2--  credit card busy/interrupt request changed
	// ------1-  generate reset after cc detect change
	// -------0  generate bus error after cc detect change

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG, "change_r: %02x\n", m_gayle_reg[REG_CHANGE]);

	return (m_gayle_reg[REG_CHANGE] << 8) | m_gayle_reg[REG_CHANGE];
}

void gayle_device::change_w(uint16_t data)
{
	data >>= 8;
	LOGMASKED(LOG_REG, "change_w: %02x\n", data);

	// clear ide interrupt?
	if (BIT(data, 7) == 0)
	{
		LOGMASKED(LOG_REG, "ide int cleared\n", data);
		m_int2_w(0);
	}

	// clear cc detect interrupt?
	if (BIT(data, 6) == 0)
	{
		LOGMASKED(LOG_REG, "cc detect int cleared\n", data);
		m_int6_w(0);
	}

	// clear bvd1/sc interrupt?
	if (BIT(data, 5) == 0)
	{
		LOGMASKED(LOG_REG, "bvd1/sc int cleared\n", data);
		if (BIT(m_gayle_reg[REG_INT], 1))
			m_int6_w(0);
		else
			m_int2_w(0);
	}

	// clear bvd2/da interrupt?
	if (BIT(data, 4) == 0)
	{
		LOGMASKED(LOG_REG, "bvd2/da int cleared\n", data);
		if (BIT(m_gayle_reg[REG_INT], 1))
			m_int6_w(0);
		else
			m_int2_w(0);
	}

	// clear write protect interrupt?
	if (BIT(data, 3) == 0)
	{
		LOGMASKED(LOG_REG, "write protect int cleared\n", data);
		m_int2_w(0);
	}

	// enabling reset on cc detect change can immediately generate a reset
	if (BIT(data, 1) && BIT(m_gayle_reg[REG_CHANGE], 6))
	{
		LOGMASKED(LOG_REG, "resetting\n", data);

		m_rst_w(0);
		m_rst_w(1);
		reset();

		return;
	}

	m_gayle_reg[REG_CHANGE] = (m_gayle_reg[REG_CHANGE] & data) | (data & 0x03);
}

uint16_t gayle_device::int_r()
{
	// 7-------  ide interrupt enable
	// -6------  credit card detect interrupt enable
	// --5-----  battery voltage 1/status change interrupt enable
	// ---4----  battery voltage 2/digital audio interrupt enable
	// ----3---  write enable interrupt enable
	// -----2--  busy/interrupt request interrupt enable
	// ------1-  set interrupt level for battery voltage 1/2
	// -------0  set interrupt level for busy/interrupt request

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG, "int_r: %02x\n", m_gayle_reg[REG_INT]);

	return (m_gayle_reg[REG_INT] << 8) | m_gayle_reg[REG_INT];
}

void gayle_device::int_w(uint16_t data)
{
	data >>= 8;
	LOGMASKED(LOG_REG, "int_w: %02x\n", data);

	m_gayle_reg[REG_INT] = data;
}

uint16_t gayle_device::control_r()
{
	// 7654----  not implemented in hardware
	// ----32--- slow mem (250 ns, 150 ns, 100 ns, 720 ns)
	// ------1-  program 12v
	// -------0  program 5v

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG, "control_r: %02x\n", m_gayle_reg[REG_CONTROL]);

	return (m_gayle_reg[REG_CONTROL] << 8) | m_gayle_reg[REG_CONTROL];
}

void gayle_device::control_w(uint16_t data)
{
	data >>= 8;
	LOGMASKED(LOG_REG, "control_w: %02x\n", data);

	m_gayle_reg[REG_CONTROL] = data;
}


//**************************************************************************
//  IDE
//**************************************************************************

template<int N>
uint16_t gayle_device::ide_cs_r(offs_t offset, uint16_t mem_mask)
{
	return m_ide_cs_r_cb[N]((offset >> 1) & 0x07, mem_mask);
}

template<int N>
void gayle_device::ide_cs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ide_cs_w_cb[N]((offset >> 1) & 0x07, data, mem_mask);
}

void gayle_device::ide_interrupt_w(int state)
{
	LOGMASKED(LOG_IDE, "ide_interrupt_w: %d\n", state);
	line_change(7, state, 2);
}


//**************************************************************************
//  CREDIT CARD
//**************************************************************************

void gayle_device::cc_cd1_w(int state)
{
	LOGMASKED(LOG_CC, "cc1_cd1_w: %d\n", state);
	m_cd[0] = state;
	line_change(LINE_CC_DET, !(m_cd[0] || m_cd[1]), 6);
}

void gayle_device::cc_cd2_w(int state)
{
	LOGMASKED(LOG_CC, "cc1_cd2_w: %d\n", state);
	m_cd[1] = state;
	line_change(LINE_CC_DET, !(m_cd[0] || m_cd[1]), 6);
}

void gayle_device::cc_bvd1_w(int state)
{
	LOGMASKED(LOG_CC, "cc_bvd1_w: %d\n", state);
	line_change(LINE_CC_BVD1_SC, !state, BIT(m_gayle_reg[REG_INT], 1) ? 6 : 2);
}

void gayle_device::cc_bvd2_w(int state)
{
	LOGMASKED(LOG_CC, "cc_bvd2_w: %d\n", state);
	line_change(LINE_CC_BVD2_DA, !state, BIT(m_gayle_reg[REG_INT], 1) ? 6 : 2);
}

void gayle_device::cc_wp_w(int state)
{
	LOGMASKED(LOG_CC, "cc_wp_w: %d\n", state);
	line_change(LINE_CC_WP, !state, 2);
}


//**************************************************************************
//  ID
//**************************************************************************

uint16_t gayle_device::gayle_id_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data;

	if (ACCESSING_BITS_8_15)
		data = ((m_gayle_id << m_gayle_id_count++) & 0x80) << 8;
	else
		data = 0xffff;

	LOGMASKED(LOG_ID, "gayle_id_r(%06x): %04x & %04x (id=%02x)\n", offset, data, mem_mask, m_gayle_id);

	return data;
}

void gayle_device::gayle_id_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_ID, "gayle_id_w(%06x): %04x & %04x (id=%02x)\n", offset, data, mem_mask, m_gayle_id);

	m_gayle_id_count = 0;
}
