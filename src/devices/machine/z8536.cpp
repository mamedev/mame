// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Zilog Z8536 Counter/Timer and Parallel I/O emulation

**********************************************************************/

/*

    TODO:

    - interrupts
        - vector
        - status affects vector
        - IE/IP/IUS
        - acknowledge
        - daisy chain
    - port I/O
    - counters/timers

*/

#include "emu.h"
#include "z8536.h"


// device type definition
const device_type Z8536 = &device_creator<z8536_device>;


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


static const char *PMS_PTS[] = { "Bit", "Input", "Output", "Bidirectional" };
static const char *PMS_PMS[] = { "Disabled", "AND", "OR", "OR-PEV" };
static const char *CTMS_DCS[] = { "Pulse", "One-shot", "Square Wave", "Do not use" };


// master interrupt control register
#define MICR_RESET      0x01    // reset
#define MICR_RJA        0x02    // right justified address
#define MICR_CT_VIS     0x04    // counter/timer vector includes status
#define MICR_PB_VIS     0x08    // port B vector includes status
#define MICR_PA_VIS     0x10    // port A vector includes status
#define MICR_NV         0x20    // no vector
#define MICR_DLC        0x40    // disable lower chain
#define MICR_MIE        0x80    // master interrupt enable


// master configuration control register
#define MCCR_LC_MASK    0x03    // counter/timer link controls
#define MCCR_PAE        0x04    // port A enable
#define MCCR_PLC        0x08    // port link control
#define MCCR_PCE_CT3E   0x10    // port C and counter/timer 3 enable
#define MCCR_CT2E       0x20    // counter/timer 2 enable
#define MCCR_CT1E       0x40    // counter/timer 1 enable
#define MCCR_PBE        0x80    // port B enable


// port mode specification registers
#define PMS_LPM         0x01    // latch on pattern match
#define PMS_DTE         0x01    // deskew timer enable
#define PMS_PMS_MASK    0x06    // pattern mode specification
#define PMS_IMO         0x08    // interrupt on match only
#define PMS_SB          0x10    // single buffer
#define PMS_ITB         0x20    // interrupt on two bytes
#define PMS_PTS_MASK    0xc0    // port type select


// port handshake specification registers
#define PHS_DTS_MASK    0x07    // deskew time specification
#define PHS_RWS_MASK    0x38    // request/wait specification
#define PHS_HTS_MASK    0xc0    // handshake type specification


// port command and status registers
#define PCS_IOE         0x01    // interrupt on error
#define PCS_PMF         0x02    // pattern match flag (read only)
#define PCS_IRF         0x04    // input register full (read only)
#define PCS_ORE         0x08    // output register empty (read only)
#define PCS_ERR         0x10    // interrupt error (read only)
#define PCS_IP          0x20    // interrupt pending
#define PCS_IE          0x40    // interrupt enable
#define PCS_IUS         0x80    // interrupt under service


// counter/timer mode specification registers
#define CTMS_DCS_MASK   0x03    // output duty cycle
#define CTMS_REB        0x04    // retrigger enable bit
#define CTMS_EDE        0x08    // external gate enable
#define CTMS_ETE        0x10    // external trigger enable
#define CTMS_ECE        0x20    // external count enable
#define CTMS_EOE        0x40    // external output enable
#define CTMS_CSC        0x80    // continuous/single cycle


// counter/timer command and status registers
#define CTCS_CIP        0x01    // count in progress (read only)
#define CTCS_TCB        0x02    // trigger command bit (write only - read returns 0)
#define CTCS_GCB        0x04    // gate command bit
#define CTCS_RCC        0x08    // read counter control (read/set only - cleared by reading CCR LSB)
#define CTCS_ERR        0x10    // interrupt error (read only)
#define CTCS_IP         0x20    // interrupt pending
#define CTCS_IE         0x40    // interrupt enable
#define CTCS_IUS        0x80    // interrupt under service



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  get_interrupt_vector -
//-------------------------------------------------

void z8536_device::get_interrupt_vector()
{
	UINT8 vector = 0xff;

	if (m_register[MASTER_INTERRUPT_CONTROL] & MICR_MIE)
	{
		if ((m_register[COUNTER_TIMER_3_COMMAND_AND_STATUS] & (CTCS_IP | CTCS_IE | CTCS_IUS)) == (CTCS_IP | CTCS_IE))
		{
			vector = m_register[COUNTER_TIMER_INTERRUPT_VECTOR];

			if (m_register[MASTER_INTERRUPT_CONTROL] & MICR_CT_VIS)
			{
				vector = (vector & 0xf9) | 0;
			}
		}
		else if ((m_register[PORT_A_COMMAND_AND_STATUS] & (PCS_IP | PCS_IE | PCS_IUS)) == (PCS_IP | PCS_IE))
		{
			vector = m_register[PORT_A_INTERRUPT_VECTOR];

			if (m_register[MASTER_INTERRUPT_CONTROL] & MICR_PA_VIS)
			{
				vector &= 0xf1;

				if (((m_register[PORT_A_MODE_SPECIFICATION] & PMS_PMS_MASK) >> 1) == PMS_OR_PEV)
				{
					if      (m_match[PORT_A] & 0x80) vector |= 7 << 1;
					else if (m_match[PORT_A] & 0x40) vector |= 6 << 1;
					else if (m_match[PORT_A] & 0x20) vector |= 5 << 1;
					else if (m_match[PORT_A] & 0x10) vector |= 4 << 1;
					else if (m_match[PORT_A] & 0x08) vector |= 3 << 1;
					else if (m_match[PORT_A] & 0x04) vector |= 2 << 1;
					else if (m_match[PORT_A] & 0x02) vector |= 1 << 1;
					else if (m_match[PORT_A] & 0x01) vector |= 0 << 1;
				}
				else
				{
					vector |= (m_register[PORT_A_COMMAND_AND_STATUS] & 0x0e);
				}
			}
		}
		else if ((m_register[COUNTER_TIMER_2_COMMAND_AND_STATUS] & (CTCS_IP | CTCS_IE | CTCS_IUS)) == (CTCS_IP | CTCS_IE))
		{
			vector = m_register[COUNTER_TIMER_INTERRUPT_VECTOR];

			if (m_register[MASTER_INTERRUPT_CONTROL] & MICR_CT_VIS)
			{
				vector = (vector & 0xf9) | 2;
			}
		}
		else if ((m_register[PORT_B_COMMAND_AND_STATUS] & (PCS_IP | PCS_IE | PCS_IUS)) == (PCS_IP | PCS_IE))
		{
			vector = m_register[PORT_B_INTERRUPT_VECTOR];

			if (m_register[MASTER_INTERRUPT_CONTROL] & MICR_PB_VIS)
			{
				vector &= 0xf1;

				if (((m_register[PORT_B_MODE_SPECIFICATION] & PMS_PMS_MASK) >> 1) == PMS_OR_PEV)
				{
					if      (m_match[PORT_B] & 0x80) vector |= 7 << 1;
					else if (m_match[PORT_B] & 0x40) vector |= 6 << 1;
					else if (m_match[PORT_B] & 0x20) vector |= 5 << 1;
					else if (m_match[PORT_B] & 0x10) vector |= 4 << 1;
					else if (m_match[PORT_B] & 0x08) vector |= 3 << 1;
					else if (m_match[PORT_B] & 0x04) vector |= 2 << 1;
					else if (m_match[PORT_B] & 0x02) vector |= 1 << 1;
					else if (m_match[PORT_B] & 0x01) vector |= 0 << 1;
				}
				else
				{
					vector |= (m_register[PORT_B_COMMAND_AND_STATUS] & 0x0e);
				}
			}
		}
		else if ((m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS] & (CTCS_IP | CTCS_IE | CTCS_IUS)) == (CTCS_IP | CTCS_IE))
		{
			vector = m_register[COUNTER_TIMER_INTERRUPT_VECTOR];

			if (m_register[MASTER_INTERRUPT_CONTROL] & MICR_CT_VIS)
			{
				vector = (vector & 0xf9) | 4;
			}
		}
	}

	m_register[CURRENT_VECTOR] = vector;
}


//-------------------------------------------------
//  check_interrupt - check interrupt status
//-------------------------------------------------

void z8536_device::check_interrupt()
{
	int state = ASSERT_LINE;

	if (m_register[MASTER_INTERRUPT_CONTROL] & MICR_MIE)
	{
		if (((m_register[COUNTER_TIMER_3_COMMAND_AND_STATUS] & (CTCS_IP | CTCS_IE | CTCS_IUS)) == (CTCS_IP | CTCS_IE)) ||
			((m_register[PORT_A_COMMAND_AND_STATUS] & (PCS_IP | PCS_IE | PCS_IUS)) == (PCS_IP | PCS_IE)) ||
			((m_register[COUNTER_TIMER_2_COMMAND_AND_STATUS] & (CTCS_IP | CTCS_IE | CTCS_IUS)) == (CTCS_IP | CTCS_IE)) ||
			((m_register[PORT_B_COMMAND_AND_STATUS] & (PCS_IP | PCS_IE | PCS_IUS)) == (PCS_IP | PCS_IE)) ||
			((m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS] & (CTCS_IP | CTCS_IE | CTCS_IUS)) == (CTCS_IP | CTCS_IE)))
		{
			state = ASSERT_LINE;
		}
		else
		{
			state = CLEAR_LINE;
		}
	}
	else
	{
		state = CLEAR_LINE;
	}

	if (m_irq != state)
	{
		if (LOG) logerror("%s Z8536 '%s' Interrupt: %u\n", machine().describe_context(), tag(), state);
		m_irq = state;
		m_write_irq(state);
	}
}


//-------------------------------------------------
//  read_register - read from register
//-------------------------------------------------

UINT8 z8536_device::read_register(offs_t offset)
{
	UINT8 data = 0;

	switch (offset)
	{
	case PORT_A_DATA:
		data = m_read_pa(0);
		break;

	case PORT_B_DATA:
		data = m_read_pb(0);
		break;

	case PORT_C_DATA:
		data = 0xf0 | (m_read_pc(0) & 0x0f);
		break;

	case COUNTER_TIMER_1_CURRENT_COUNT_MS_BYTE:
	case COUNTER_TIMER_2_CURRENT_COUNT_MS_BYTE:
	case COUNTER_TIMER_3_CURRENT_COUNT_MS_BYTE:
		{
		int timer = (offset - COUNTER_TIMER_1_CURRENT_COUNT_MS_BYTE) >> 1;

		if (m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + timer] & CTCS_RCC)
		{
			// read frozen value
			data = m_register[offset];
		}
		else
		{
			// read current count
			data = m_counter[timer] >> 8;
		}
		}
		break;

	case COUNTER_TIMER_1_CURRENT_COUNT_LS_BYTE:
	case COUNTER_TIMER_2_CURRENT_COUNT_LS_BYTE:
	case COUNTER_TIMER_3_CURRENT_COUNT_LS_BYTE:
		{
		int timer = (offset - COUNTER_TIMER_1_CURRENT_COUNT_MS_BYTE) >> 1;

		if (m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + timer] & CTCS_RCC)
		{
			// read frozen value
			data = m_register[offset];
		}
		else
		{
			// read current count
			data = m_counter[timer] & 0xff;
		}

		// clear RCC bit
		m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + timer] &= ~CTCS_RCC;
		}
		break;

	case CURRENT_VECTOR:
		get_interrupt_vector();
		data = m_register[offset];
		break;

	default:
		data = m_register[offset];
		break;
	}

	return data;
}


//-------------------------------------------------
//  read_register - masked read from register
//-------------------------------------------------

UINT8 z8536_device::read_register(offs_t offset, UINT8 mask)
{
	return read_register(offset) & mask;
}


//-------------------------------------------------
//  write_register - write to register
//-------------------------------------------------

void z8536_device::write_register(offs_t offset, UINT8 data)
{
	switch (offset)
	{
	case MASTER_INTERRUPT_CONTROL:
		if (data & MICR_RESET)
		{
			if (LOG) logerror("%s Z8536 '%s' Reset\n", machine().describe_context(), tag());
			device_reset();
		}
		else
		{
			if (m_state == STATE_RESET)
			{
				m_state = STATE_0;
			}

			if (LOG)
			{
				if (LOG) logerror("%s Z8536 '%s' Master Interrupt Enable: %u\n", machine().describe_context(), tag(), (data & MICR_MIE) ? 1 : 0);
				if (LOG) logerror("%s Z8536 '%s' Disable Lower Chain: %u\n", machine().describe_context(), tag(), (data & MICR_DLC) ? 1 : 0);
				if (LOG) logerror("%s Z8536 '%s' No Vector: %u\n", machine().describe_context(), tag(), (data & MICR_NV) ? 1 : 0);
				if (LOG) logerror("%s Z8536 '%s' Port A Vector Includes Status: %u\n", machine().describe_context(), tag(), (data & MICR_PA_VIS) ? 1 : 0);
				if (LOG) logerror("%s Z8536 '%s' Port B Vector Includes Status: %u\n", machine().describe_context(), tag(), (data & MICR_PB_VIS) ? 1 : 0);
				if (LOG) logerror("%s Z8536 '%s' Counter/Timer Vector Includes Status: %u\n", machine().describe_context(), tag(), (data & MICR_CT_VIS) ? 1 : 0);
				if (LOG) logerror("%s Z8536 '%s' Right Justified Address: %u\n", machine().describe_context(), tag(), (data & MICR_RJA) ? 1 : 0);
			}

			m_register[offset] = data;
		}
		break;

	case MASTER_CONFIGURATION_CONTROL:
		if (LOG)
		{
			if (LOG) logerror("%s Z8536 '%s' Port B Enable: %u\n", machine().describe_context(), tag(), (data & MCCR_PBE) ? 1 : 0);
			if (LOG) logerror("%s Z8536 '%s' Counter/Timer 1 Enable: %u\n", machine().describe_context(), tag(), (data & MCCR_CT1E) ? 1 : 0);
			if (LOG) logerror("%s Z8536 '%s' Counter/Timer 2 Enable: %u\n", machine().describe_context(), tag(), (data & MCCR_CT2E) ? 1 : 0);
			if (LOG) logerror("%s Z8536 '%s' Port C and Counter/Timer 3 Enable: %u\n", machine().describe_context(), tag(), (data & MCCR_PCE_CT3E) ? 1 : 0);
			if (LOG) logerror("%s Z8536 '%s' Port A Enable: %u\n", machine().describe_context(), tag(), (data & MCCR_PAE) ? 1 : 0);
			if (LOG) logerror("%s Z8536 '%s' Port Link Control: %u\n", machine().describe_context(), tag(), (data & MCCR_PLC) ? 1 : 0);
			if (LOG) logerror("%s Z8536 '%s' Counter/Timer Link Controls: %u\n", machine().describe_context(), tag(), data & MCCR_LC_MASK);
		}

		m_register[offset] = data;

		for (int counter = 0; counter < 3; counter++)
		{
			// clear RCC bit if counter disabled
			if (!counter_enabled(counter)) m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + counter] &= ~CTCS_RCC;
		}
		break;

	case PORT_A_INTERRUPT_VECTOR:
		if (LOG) logerror("%s Z8536 '%s' Port A Interrupt Vector: %02x\n", machine().describe_context(), tag(), data);
		m_register[offset] = data;
		break;

	case PORT_B_INTERRUPT_VECTOR:
		if (LOG) logerror("%s Z8536 '%s' Port B Interrupt Vector: %02x\n", machine().describe_context(), tag(), data);
		m_register[offset] = data;
		break;

	case COUNTER_TIMER_INTERRUPT_VECTOR:
		if (LOG) logerror("%s Z8536 '%s' Counter/Timer Interrupt Vector: %02x\n", machine().describe_context(), tag(), data);
		m_register[offset] = data;
		break;

	case PORT_C_DATA_PATH_POLARITY:
		if (LOG) logerror("%s Z8536 '%s' Port C Data Path Polarity: %02x\n", machine().describe_context(), tag(), data);
		m_register[offset] = data;
		break;

	case PORT_C_DATA_DIRECTION:
		if (LOG) logerror("%s Z8536 '%s' Port C Data Direction: %02x\n", machine().describe_context(), tag(), data);
		m_register[offset] = data;
		break;

	case PORT_C_SPECIAL_IO_CONTROL:
		if (LOG) logerror("%s Z8536 '%s' Port C Special I/O Control: %02x\n", machine().describe_context(), tag(), data);
		m_register[offset] = data;
		break;

	case PORT_A_COMMAND_AND_STATUS:
	case PORT_B_COMMAND_AND_STATUS:
		{
		char port = 'A' + offset - PORT_A_COMMAND_AND_STATUS;

		if (LOG) logerror("%s Z8536 '%s' Port %c Interrupt on Error: %u\n", machine().describe_context(), tag(), port, (data & PCS_IOE) ? 1 : 0);

		switch (data >> 5)
		{
		case IC_CLEAR_IP_IUS:   m_register[offset] &= ~(PCS_IP | PCS_IUS);  if (LOG) logerror("%s Z8536 '%s' Port %c Clear IP/IUS\n", machine().describe_context(), tag(), port);   break;
		case IC_SET_IUS:        m_register[offset] |= PCS_IUS;              if (LOG) logerror("%s Z8536 '%s' Port %c Set IUS\n", machine().describe_context(), tag(), port);        break;
		case IC_CLEAR_IUS:      m_register[offset] &= ~PCS_IUS;             if (LOG) logerror("%s Z8536 '%s' Port %c Clear IUS\n", machine().describe_context(), tag(), port);      break;
		case IC_SET_IP:         m_register[offset] |= PCS_IP;               if (LOG) logerror("%s Z8536 '%s' Port %c Set IP\n", machine().describe_context(), tag(), port);         break;
		case IC_CLEAR_IP:       m_register[offset] &= ~PCS_IP;              if (LOG) logerror("%s Z8536 '%s' Port %c Clear IP\n", machine().describe_context(), tag(), port);       break;
		case IC_SET_IE:         m_register[offset] |= PCS_IE;               if (LOG) logerror("%s Z8536 '%s' Port %c Set IE\n", machine().describe_context(), tag(), port);         break;
		case IC_CLEAR_IE:       m_register[offset] &= ~PCS_IE;              if (LOG) logerror("%s Z8536 '%s' Port %c Clear IE\n", machine().describe_context(), tag(), port);       break;
		}

		m_register[offset] = (m_register[offset] & ~PCS_IOE) | (data & PCS_IOE);

		match_pattern(offset - PORT_A_COMMAND_AND_STATUS);
		check_interrupt();
		}
		break;

	case COUNTER_TIMER_1_COMMAND_AND_STATUS:
	case COUNTER_TIMER_2_COMMAND_AND_STATUS:
	case COUNTER_TIMER_3_COMMAND_AND_STATUS:
		{
		int counter = offset - COUNTER_TIMER_1_COMMAND_AND_STATUS;

		if (LOG)
		{
			if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Trigger Command Bit: %u\n", machine().describe_context(), tag(), counter + 1, (data & CTCS_TCB) ? 1 : 0);
			if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Gate Command Bit: %u\n", machine().describe_context(), tag(), counter + 1, (data & CTCS_GCB) ? 1 : 0);
			if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Read Counter Control: %u\n", machine().describe_context(), tag(), counter + 1, (data & CTCS_RCC) ? 1 : 0);
		}

		switch (data >> 5)
		{
		case IC_CLEAR_IP_IUS:   m_register[offset] &= ~(CTCS_IP | CTCS_IUS);if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Clear IP/IUS\n", machine().describe_context(), tag(), counter + 1);   break;
		case IC_SET_IUS:        m_register[offset] |= CTCS_IUS;             if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Set IUS\n", machine().describe_context(), tag(), counter + 1);        break;
		case IC_CLEAR_IUS:      m_register[offset] &= ~CTCS_IUS;            if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Clear IUS\n", machine().describe_context(), tag(), counter + 1);      break;
		case IC_SET_IP:         m_register[offset] |= CTCS_IP;              if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Set IP\n", machine().describe_context(), tag(), counter + 1);         break;
		case IC_CLEAR_IP:       m_register[offset] &= ~CTCS_IP;             if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Clear IP\n", machine().describe_context(), tag(), counter + 1);       break;
		case IC_SET_IE:         m_register[offset] |= CTCS_IE;              if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Set IE\n", machine().describe_context(), tag(), counter + 1);         break;
		case IC_CLEAR_IE:       m_register[offset] &= ~CTCS_IE;             if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Clear IE\n", machine().describe_context(), tag(), counter + 1);       break;
		}

		// gate command bit
		m_register[offset] = (m_register[offset] & ~CTCS_GCB) | (data & CTCS_GCB);

		// trigger command bit
		if (data & CTCS_TCB)
		{
			trigger(counter);
		}

		// read counter control
		if (counter_enabled(counter) && (data & CTCS_RCC))
		{
			// freeze current count register
			m_register[offset] |= CTCS_RCC;
			m_register[COUNTER_TIMER_1_CURRENT_COUNT_MS_BYTE + (counter << 1)] = m_counter[counter] >> 8;
			m_register[COUNTER_TIMER_1_CURRENT_COUNT_LS_BYTE + (counter << 1)] = m_counter[counter] & 0xff;
		}

		check_interrupt();
		}
		break;

	case PORT_A_DATA:
		m_write_pa((offs_t)0, data);
		break;

	case PORT_B_DATA:
		m_write_pb((offs_t)0, data);
		break;

	case PORT_C_DATA:
		{
		UINT8 mask = (data & 0xf0) | (data >> 4);

		m_output[PORT_C] = (m_output[PORT_C] & mask) | ((data & 0x0f) & (mask ^ 0xff));

		m_write_pc((offs_t)0, m_output[PORT_C]);
		}
		break;

	case COUNTER_TIMER_1_TIME_CONSTANT_MS_BYTE:
	case COUNTER_TIMER_2_TIME_CONSTANT_MS_BYTE:
	case COUNTER_TIMER_3_TIME_CONSTANT_MS_BYTE:
		if (LOG)
		{
			int counter = (offset - COUNTER_TIMER_1_TIME_CONSTANT_MS_BYTE) >> 1;
			if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Time Constant MSB: %02x\n", machine().describe_context(), tag(), counter + 1, data);
		}

		m_register[offset] = data;
		break;

	case COUNTER_TIMER_1_TIME_CONSTANT_LS_BYTE:
	case COUNTER_TIMER_2_TIME_CONSTANT_LS_BYTE:
	case COUNTER_TIMER_3_TIME_CONSTANT_LS_BYTE:
		if (LOG)
		{
			int counter = (offset - COUNTER_TIMER_1_TIME_CONSTANT_LS_BYTE) >> 1;
			if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Time Constant LSB: %02x\n", machine().describe_context(), tag(), counter + 1, data);
		}

		m_register[offset] = data;
		break;

	case COUNTER_TIMER_1_MODE_SPECIFICATION:
	case COUNTER_TIMER_2_MODE_SPECIFICATION:
	case COUNTER_TIMER_3_MODE_SPECIFICATION:
		if (LOG)
		{
			int counter = offset - COUNTER_TIMER_1_MODE_SPECIFICATION;
			int dcs = data & CTMS_DCS_MASK;

			logerror("%s Z8536 '%s' Counter/Timer %u Mode: %s\n", machine().describe_context(), tag(), counter + 1, (data & CTMS_CSC) ? "Continuous" : "Single Cycle");
			logerror("%s Z8536 '%s' Counter/Timer %u External Output Enable: %u\n", machine().describe_context(), tag(), counter + 1, (data & CTMS_EOE) ? 1 : 0);
			logerror("%s Z8536 '%s' Counter/Timer %u External Count Enable: %u\n", machine().describe_context(), tag(), counter + 1, (data & CTMS_ECE) ? 1 : 0);
			logerror("%s Z8536 '%s' Counter/Timer %u External Trigger Enable: %u\n", machine().describe_context(), tag(), counter + 1, (data & CTMS_ETE) ? 1 : 0);
			logerror("%s Z8536 '%s' Counter/Timer %u External Gate Enable: %u\n", machine().describe_context(), tag(), counter + 1, (data & CTMS_EDE) ? 1 : 0);
			logerror("%s Z8536 '%s' Counter/Timer %u Retrigger Enable: %u\n", machine().describe_context(), tag(), counter + 1, (data & CTMS_REB) ? 1 : 0);
			logerror("%s Z8536 '%s' Counter/Timer %u Output Duty Cycle: %s\n", machine().describe_context(), tag(), counter + 1, CTMS_DCS[dcs]);
		}

		m_register[offset] = data;
		break;

	case PORT_A_MODE_SPECIFICATION:
	case PORT_B_MODE_SPECIFICATION:
		if (LOG)
		{
			char port = BIT(offset, 3) ? 'B' : 'A';
			int pts = (data & PMS_PTS_MASK) >> 6;
			int pms = (data & PMS_PMS_MASK) >> 1;

			logerror("%s Z8536 '%s' Port %c Port Type: %s\n", machine().describe_context(), tag(), port, PMS_PTS[pts]);
			logerror("%s Z8536 '%s' Port %c Interrupt on 2 Bytes: %u\n", machine().describe_context(), tag(), port, (data & PMS_ITB) ? 1 : 0);
			logerror("%s Z8536 '%s' Port %c Single Buffer: %u\n", machine().describe_context(), tag(), port, (data & PMS_SB) ? 1 : 0);
			logerror("%s Z8536 '%s' Port %c Interrupt on Match Only: %u\n", machine().describe_context(), tag(), port, (data & PMS_IMO) ? 1 : 0);
			logerror("%s Z8536 '%s' Port %c Pattern Mode: %s\n", machine().describe_context(), tag(), port, PMS_PMS[pms]);

			if (pts == PTS_BIT)
				logerror("%s Z8536 '%s' Port %c Latch on Pattern Match: %u\n", machine().describe_context(), tag(), port, (data & PMS_LPM) ? 1 : 0);
			else
				logerror("%s Z8536 '%s' Port %c Deskew Timer Enable: %u\n", machine().describe_context(), tag(), port, (data & PMS_DTE) ? 1 : 0);
		}

		m_register[offset] = data;
		break;

	case PORT_A_HANDSHAKE_SPECIFICATION:
	case PORT_B_HANDSHAKE_SPECIFICATION:
		// TODO
		break;

	case PORT_A_DATA_PATH_POLARITY:
	case PORT_B_DATA_PATH_POLARITY:
		if (LOG) logerror("%s Z8536 '%s' Port %c Data Path Polarity: %02x\n", machine().describe_context(), tag(), BIT(offset, 3) ? 'B' : 'A', data);
		m_register[offset] = data;
		break;

	case PORT_A_DATA_DIRECTION:
	case PORT_B_DATA_DIRECTION:
		if (LOG) logerror("%s Z8536 '%s' Port %c Data Direction: %02x\n", machine().describe_context(), tag(), BIT(offset, 3) ? 'B' : 'A', data);
		m_register[offset] = data;
		break;

	case PORT_A_SPECIAL_IO_CONTROL:
	case PORT_B_SPECIAL_IO_CONTROL:
		if (LOG) logerror("%s Z8536 '%s' Port %c Special I/O Control: %02x\n", machine().describe_context(), tag(), BIT(offset, 3) ? 'B' : 'A', data);
		m_register[offset] = data;
		break;

	case PORT_A_PATTERN_POLARITY:
	case PORT_B_PATTERN_POLARITY:
		if (LOG) logerror("%s Z8536 '%s' Port %c Pattern Polarity: %02x\n", machine().describe_context(), tag(), BIT(offset, 3) ? 'B' : 'A', data);
		m_register[offset] = data;
		break;

	case PORT_A_PATTERN_TRANSITION:
	case PORT_B_PATTERN_TRANSITION:
		if (LOG) logerror("%s Z8536 '%s' Port %c Pattern Transition: %02x\n", machine().describe_context(), tag(), BIT(offset, 3) ? 'B' : 'A', data);
		m_register[offset] = data;
		break;

	case PORT_A_PATTERN_MASK:
	case PORT_B_PATTERN_MASK:
		if (LOG) logerror("%s Z8536 '%s' Port %c Pattern Mask: %02x\n", machine().describe_context(), tag(), BIT(offset, 3) ? 'B' : 'A', data);
		m_register[offset] = data;
		match_pattern(BIT(offset, 3));
		check_interrupt();
		break;

	default:
		logerror("%s: Z8536 '%s' Unimplemented write %02x to register %u\n", machine().describe_context(), tag(), data, offset);
		m_register[offset] = data;
	}
}


//-------------------------------------------------
//  write_register - masked write to register
//-------------------------------------------------

void z8536_device::write_register(offs_t offset, UINT8 data, UINT8 mask)
{
	UINT8 combined_data = (data & mask) | (m_register[offset] & (mask ^ 0xff));

	write_register(offset, combined_data);
}


//-------------------------------------------------
//   counter_enabled - is counter enabled?
//-------------------------------------------------

bool z8536_device::counter_enabled(device_timer_id id)
{
	bool enabled = false;

	switch (id)
	{
	case TIMER_1:
		enabled = (m_register[MASTER_CONFIGURATION_CONTROL] & MCCR_CT1E) ? true : false;
		break;

	case TIMER_2:
		enabled = (m_register[MASTER_CONFIGURATION_CONTROL] & MCCR_CT2E) ? true : false;
		break;

	case TIMER_3:
		enabled = (m_register[MASTER_CONFIGURATION_CONTROL] & MCCR_PCE_CT3E) ? true : false;
		break;
	}

	return enabled;
}


//-------------------------------------------------
//   counter_external_output -
//-------------------------------------------------

bool z8536_device::counter_external_output(device_timer_id id)
{
	return (m_register[COUNTER_TIMER_1_MODE_SPECIFICATION + id] & CTMS_EOE) ? true : false;
}


//-------------------------------------------------
//   counter_external_count -
//-------------------------------------------------

bool z8536_device::counter_external_count(device_timer_id id)
{
	return (m_register[COUNTER_TIMER_1_MODE_SPECIFICATION + id] & CTMS_ECE) ? true : false;
}


//-------------------------------------------------
//   counter_external_trigger -
//-------------------------------------------------

bool z8536_device::counter_external_trigger(device_timer_id id)
{
	return (m_register[COUNTER_TIMER_1_MODE_SPECIFICATION + id] & CTMS_ETE) ? true : false;
}


//-------------------------------------------------
//   counter_external_gate -
//-------------------------------------------------

bool z8536_device::counter_external_gate(device_timer_id id)
{
	return (m_register[COUNTER_TIMER_1_MODE_SPECIFICATION + id] & CTMS_EDE) ? true : false;
}


//-------------------------------------------------
//   counter_gated -
//-------------------------------------------------

bool z8536_device::counter_gated(device_timer_id id)
{
	return (m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + id] & CTCS_GCB) ? true : false;
}


//-------------------------------------------------
//   count - count down
//-------------------------------------------------

void z8536_device::count(device_timer_id id)
{
	if (!counter_gated(id)) return;
	if (!(m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + id] & CTCS_CIP)) return;

	// count down
	m_counter[id]--;

	if (m_counter[id] == 0)
	{
		if (m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + id] & CTCS_IP)
		{
			// set interrupt error bit
			m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + id] |= CTCS_ERR;
		}
		else
		{
			if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Interrupt Pending\n", machine().describe_context(), tag(), id + 1);

			// set interrupt pending bit
			m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + id] |= CTCS_IP;
		}

		if (m_register[COUNTER_TIMER_1_MODE_SPECIFICATION + id] & CTMS_CSC)
		{
			// reload counter with time constant
			m_counter[id] = (m_register[COUNTER_TIMER_1_TIME_CONSTANT_MS_BYTE + (id << 1)] << 8) | m_register[COUNTER_TIMER_1_TIME_CONSTANT_LS_BYTE + (id << 1)];
		}
		else
		{
			if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Terminal Count\n", machine().describe_context(), tag(), id + 1);

			// clear count in progress bit
			m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + id] &= ~CTCS_CIP;
		}

		check_interrupt();
	}
}


//-------------------------------------------------
//  trigger -
//-------------------------------------------------

void z8536_device::trigger(device_timer_id id)
{
	// ignore triggers during countdown if retrigger is disabled
	if (!(m_register[COUNTER_TIMER_1_MODE_SPECIFICATION + id] & CTMS_REB) && (m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + id] & CTCS_CIP)) return;

	if (LOG) logerror("%s Z8536 '%s' Counter/Timer %u Trigger\n", machine().describe_context(), tag(), id + 1);

	// load counter with time constant
	m_counter[id] = (m_register[COUNTER_TIMER_1_TIME_CONSTANT_MS_BYTE + (id << 1)] << 8) | m_register[COUNTER_TIMER_1_TIME_CONSTANT_LS_BYTE + (id << 1)];

	// set count in progress bit
	m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + id] |= CTCS_CIP;
}


//-------------------------------------------------
//  gate -
//-------------------------------------------------

void z8536_device::gate(device_timer_id id, int state)
{
	// TODO
}


//-------------------------------------------------
//  match_pattern -
//-------------------------------------------------

void z8536_device::match_pattern(int port)
{
	UINT8 pms = m_register[PORT_A_MODE_SPECIFICATION + (port << 3)];
	UINT8 pm = m_register[PORT_A_PATTERN_MASK + (port << 3)];
	UINT8 ddr = m_register[PORT_A_DATA_DIRECTION + (port << 3)];

	switch ((pms & PMS_PMS_MASK) >> 1)
	{
	case PMS_OR_PEV:
		m_match[port] = m_input[port] & ddr & pm;

		if (m_match[port])
		{
			if (LOG) logerror("%s Z8536 '%s' Port %c Interrupt Pending\n", machine().describe_context(), tag(), 'A' + port);
			m_register[PORT_A_COMMAND_AND_STATUS + port] |= PCS_IP;
			check_interrupt();
		}
		break;
	}
}


//-------------------------------------------------
//  external_port_w - external port write
//-------------------------------------------------

void z8536_device::external_port_w(int port, int bit, int state)
{
	switch (port)
	{
	case PORT_A:
	case PORT_B:
		{
		assert((PORT_A_DATA_DIRECTION + (port << 3)) >= 0 && (PORT_A_DATA_DIRECTION + (port << 3)) < ARRAY_LENGTH(m_register));
		UINT8 ddr = m_register[PORT_A_DATA_DIRECTION + (port << 3)];

		if (!BIT(ddr, bit)) return;

		if (LOG) logerror("%s Z8536 '%s' Port %c Bit %u: %u\n", machine().describe_context(), tag(), 'A' + port, bit, state);

		m_input[port] = (m_input[port] & ~(1 << bit)) | (state << bit);

		match_pattern(port);
		}
		break;

	case PORT_C:
		break;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z8536_device - constructor
//-------------------------------------------------

z8536_device::z8536_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, Z8536, "Zilog Z8536", tag, owner, clock, "z8536", __FILE__),
	device_z80daisy_interface(mconfig, *this),
	m_write_irq(*this),
	m_read_pa(*this),
	m_write_pa(*this),
	m_read_pb(*this),
	m_write_pb(*this),
	m_read_pc(*this),
	m_write_pc(*this),
	m_irq(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z8536_device::device_start()
{
	for (int i = 0; i < 3; i++)
	{
		m_input[i] = 0;
		m_output[i] = 0;
		m_buffer[i] = 0;
		m_match[i] = 0;
	}

	// allocate timer
	m_timer = timer_alloc();
	m_timer->adjust(attotime::from_hz(clock() / 2), 0, attotime::from_hz(clock() / 2));

	// resolve callbacks
	m_write_irq.resolve_safe();
	m_read_pa.resolve_safe(0);
	m_write_pa.resolve_safe();
	m_read_pb.resolve_safe(0);
	m_write_pb.resolve_safe();
	m_read_pc.resolve_safe(0);
	m_write_pc.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific reset
//-------------------------------------------------

void z8536_device::device_reset()
{
	m_state = STATE_RESET;

	for (auto & elem : m_register)
	{
		elem = 0;
	}

	m_register[MASTER_INTERRUPT_CONTROL] = MICR_RESET;
	m_register[PORT_A_COMMAND_AND_STATUS] = PCS_ORE;
	m_register[PORT_B_COMMAND_AND_STATUS] = PCS_ORE;
	m_register[CURRENT_VECTOR] = 0xff;

	m_pointer = 0;

	check_interrupt();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void z8536_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (counter_enabled(TIMER_1) && !counter_external_count(TIMER_1))
	{
		count(TIMER_1);
	}

	if (counter_enabled(TIMER_2) && !counter_external_count(TIMER_2))
	{
		count(TIMER_2);
	}

	if (counter_enabled(TIMER_3) && !counter_external_count(TIMER_3))
	{
		count(TIMER_3);
	}
}



//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - return the overall IRQ
//  state for this device
//-------------------------------------------------

int z8536_device::z80daisy_irq_state()
{
	return 0;
}


//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int z8536_device::z80daisy_irq_ack()
{
	return intack_r();
}


//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void z8536_device::z80daisy_irq_reti()
{
}



//**************************************************************************
//  INTERNAL STATE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  read - register read
//-------------------------------------------------

READ8_MEMBER( z8536_device::read )
{
	UINT8 data = 0;

	if (m_state == STATE_RESET)
	{
		// read RESET bit
		data = read_register(m_pointer, 0x01);
	}
	else
	{
		switch (offset & 0x03)
		{
		case PORT_C:
			data = read_register(PORT_C_DATA);
			break;

		case PORT_B:
			data = read_register(PORT_B_DATA);
			break;

		case PORT_A:
			data = read_register(PORT_A_DATA);
			break;

		case CONTROL:
			switch (m_state)
			{
			case STATE_1:
				m_state = STATE_0;
				// fallthru
			case STATE_0:
				data = read_register(m_pointer);
				break;
			}
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

WRITE8_MEMBER( z8536_device::write )
{
	if (m_state == STATE_RESET)
	{
		// write RESET bit
		write_register(m_pointer, data, 0x01);
	}
	else
	{
		switch (offset & 0x03)
		{
		case PORT_C:
			write_register(PORT_C_DATA, data);
			break;

		case PORT_B:
			write_register(PORT_B_DATA, data);
			break;

		case PORT_A:
			write_register(PORT_A_DATA, data);
			break;

		case CONTROL:
			switch (m_state)
			{
			case STATE_0:
				m_pointer = data;
				m_state = STATE_1;
				break;

			case STATE_1:
				write_register(m_pointer, data);
				m_state = STATE_0;
			}
			break;
		}
	}
}


//-------------------------------------------------
//  intack_r - interrupt acknowledge
//-------------------------------------------------

int z8536_device::intack_r()
{
	get_interrupt_vector();
	int data = m_register[CURRENT_VECTOR];

	if (LOG) logerror("%s Z8536 '%s' Interrupt Acknowledge: %02x\n", machine().describe_context(), tag(), data);

	// set interrupt under service bit
	if ((m_register[COUNTER_TIMER_3_COMMAND_AND_STATUS] & (CTCS_IP | CTCS_IE)) == (CTCS_IP | CTCS_IE))
	{
		m_register[COUNTER_TIMER_3_COMMAND_AND_STATUS] |= CTCS_IUS;
	}
	else if ((m_register[PORT_A_COMMAND_AND_STATUS] & (PCS_IP | PCS_IE)) == (PCS_IP | PCS_IE))
	{
		m_register[PORT_A_COMMAND_AND_STATUS] |= PCS_IUS;
	}
	else if ((m_register[COUNTER_TIMER_2_COMMAND_AND_STATUS] & (CTCS_IP | CTCS_IE)) == (CTCS_IP | CTCS_IE))
	{
		m_register[COUNTER_TIMER_2_COMMAND_AND_STATUS] |= CTCS_IUS;
	}
	else if ((m_register[PORT_B_COMMAND_AND_STATUS] & (PCS_IP | PCS_IE)) == (PCS_IP | PCS_IE))
	{
		m_register[PORT_B_COMMAND_AND_STATUS] |= PCS_IUS;
	}
	else if ((m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS] & (CTCS_IP | CTCS_IE)) == (CTCS_IP | CTCS_IE))
	{
		m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS] |= CTCS_IUS;
	}

	check_interrupt();

	if (m_register[MASTER_INTERRUPT_CONTROL] & MICR_NV)
	{
		// no vector
		data = -1;
	}

	return data;
}
