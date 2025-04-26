// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Zilog Z8036/Z8536 Counter/Timer and Parallel I/O Unit

    Much like the SCC, two alternate versions of this peripheral
    were developed by Zilog. The Z8036 (Z-CIO) was compatible with
    the Z8000's address-latched Z-Bus and could write to any
    register directly, while the Z8536 interfaced to a generic bus
    using two address bits, requiring sequential writes to its
    control register in order to access any of the internal registers
    besides the three for port data.

    Both versions of the CIO can be reset through hardware as well as
    software, despite that neither provides a dedicated RESET pin.
    Pulling AS and DS active low at the same time resets the Z8036,
    and Z8536 is reset by activating RD and WR simultaneously.

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

//#define VERBOSE 1
#include "logmacro.h"


// device type definitions
DEFINE_DEVICE_TYPE(Z8036, z8036_device, "z8036", "Zilog Z8036 Z-CIO")
DEFINE_DEVICE_TYPE(Z8536, z8536_device, "z8536", "Zilog Z8536 CIO")


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************


static char const *const PMS_PTS[] = { "Bit", "Input", "Output", "Bidirectional" };
static char const *const PMS_PMS[] = { "Disabled", "AND", "OR", "OR-PEV" };
static char const *const CTMS_DCS[] = { "Pulse", "One-shot", "Square Wave", "Do not use" };


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  get_interrupt_vector -
//-------------------------------------------------

void cio_base_device::get_interrupt_vector()
{
	u8 vector = 0xff;

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

void cio_base_device::check_interrupt()
{
	int state;

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
		LOG("%s CIO Interrupt: %u\n", machine().describe_context(), state);
		m_irq = state;
		m_write_irq(state);
	}
}


//-------------------------------------------------
//  read_register - read from register
//-------------------------------------------------

u8 cio_base_device::read_register(offs_t offset)
{
	u8 data;

	switch (offset)
	{
	case PORT_A_DATA:
		// TODO: take data path polarity into account
		data = m_output[PORT_A];
		if (m_register[PORT_A_DATA_DIRECTION] != 0)
			data = (data & ~m_register[PORT_A_DATA_DIRECTION]) | (m_read_pa() & m_register[PORT_A_DATA_DIRECTION]);
		break;

	case PORT_B_DATA:
		// TODO: take data path polarity into account
		data = m_output[PORT_B];
		if (m_register[PORT_B_DATA_DIRECTION] != 0)
			data = (data & ~m_register[PORT_B_DATA_DIRECTION]) | (m_read_pb() & m_register[PORT_B_DATA_DIRECTION]);
		break;

	case PORT_C_DATA:
		// TODO: take data path polarity into account
		data = 0xf0 | (m_read_pc() & 0x0f);
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

u8 cio_base_device::read_register(offs_t offset, u8 mask)
{
	return read_register(offset) & mask;
}


//-------------------------------------------------
//  write_register - write to register
//-------------------------------------------------

void cio_base_device::write_register(offs_t offset, u8 data)
{
	switch (offset)
	{
	case MASTER_INTERRUPT_CONTROL:
		if (data & MICR_RESET)
		{
			LOG("%s CIO Reset\n", machine().describe_context());
			device_reset();
		}
		else
		{
			LOG("%s CIO Master Interrupt Enable: %u\n", machine().describe_context(), (data & MICR_MIE) ? 1 : 0);
			LOG("%s CIO Disable Lower Chain: %u\n", machine().describe_context(), (data & MICR_DLC) ? 1 : 0);
			LOG("%s CIO No Vector: %u\n", machine().describe_context(), (data & MICR_NV) ? 1 : 0);
			LOG("%s CIO Port A Vector Includes Status: %u\n", machine().describe_context(), (data & MICR_PA_VIS) ? 1 : 0);
			LOG("%s CIO Port B Vector Includes Status: %u\n", machine().describe_context(), (data & MICR_PB_VIS) ? 1 : 0);
			LOG("%s CIO Counter/Timer Vector Includes Status: %u\n", machine().describe_context(), (data & MICR_CT_VIS) ? 1 : 0);
			LOG("%s CIO Right Justified Address: %u\n", machine().describe_context(), (data & MICR_RJA) ? 1 : 0);

			m_register[offset] = data;
		}
		break;

	case MASTER_CONFIGURATION_CONTROL:
		LOG("%s CIO Port B Enable: %u\n", machine().describe_context(), (data & MCCR_PBE) ? 1 : 0);
		LOG("%s CIO Counter/Timer 1 Enable: %u\n", machine().describe_context(), (data & MCCR_CT1E) ? 1 : 0);
		LOG("%s CIO Counter/Timer 2 Enable: %u\n", machine().describe_context(), (data & MCCR_CT2E) ? 1 : 0);
		LOG("%s CIO Port C and Counter/Timer 3 Enable: %u\n", machine().describe_context(), (data & MCCR_PCE_CT3E) ? 1 : 0);
		LOG("%s CIO Port A Enable: %u\n", machine().describe_context(), (data & MCCR_PAE) ? 1 : 0);
		LOG("%s CIO Port Link Control: %u\n", machine().describe_context(), (data & MCCR_PLC) ? 1 : 0);
		LOG("%s CIO Counter/Timer Link Controls: %u\n", machine().describe_context(), data & MCCR_LC_MASK);

		m_register[offset] = data;

		for (int counter = 0; counter < 3; counter++)
		{
			// clear RCC bit if counter disabled
			if (!counter_enabled(counter)) m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + counter] &= ~CTCS_RCC;
		}
		break;

	case PORT_A_INTERRUPT_VECTOR:
		LOG("%s CIO Port A Interrupt Vector: %02x\n", machine().describe_context(), data);
		m_register[offset] = data;
		break;

	case PORT_B_INTERRUPT_VECTOR:
		LOG("%s CIO Port B Interrupt Vector: %02x\n", machine().describe_context(), data);
		m_register[offset] = data;
		break;

	case COUNTER_TIMER_INTERRUPT_VECTOR:
		LOG("%s CIO Counter/Timer Interrupt Vector: %02x\n", machine().describe_context(), data);
		m_register[offset] = data;
		break;

	case PORT_C_DATA_PATH_POLARITY:
		LOG("%s CIO Port C Data Path Polarity: %02x\n", machine().describe_context(), data);
		m_register[offset] = data & 0x0f;
		break;

	case PORT_C_DATA_DIRECTION:
		LOG("%s CIO Port C Data Direction: %02x\n", machine().describe_context(), data);
		m_register[offset] = data & 0x0f;
		break;

	case PORT_C_SPECIAL_IO_CONTROL:
		LOG("%s CIO Port C Special I/O Control: %02x\n", machine().describe_context(), data);
		m_register[offset] = data & 0x0f;
		break;

	case PORT_A_COMMAND_AND_STATUS:
	case PORT_B_COMMAND_AND_STATUS:
		{
		char port = 'A' + offset - PORT_A_COMMAND_AND_STATUS;

		LOG("%s CIO Port %c Interrupt on Error: %u\n", machine().describe_context(), port, (data & PCS_IOE) ? 1 : 0);

		switch (data >> 5)
		{
		case IC_CLEAR_IP_IUS:   m_register[offset] &= ~(PCS_IP | PCS_IUS);  LOG("%s CIO Port %c Clear IP/IUS\n", machine().describe_context(), port);   break;
		case IC_SET_IUS:        m_register[offset] |= PCS_IUS;              LOG("%s CIO Port %c Set IUS\n", machine().describe_context(), port);        break;
		case IC_CLEAR_IUS:      m_register[offset] &= ~PCS_IUS;             LOG("%s CIO Port %c Clear IUS\n", machine().describe_context(), port);      break;
		case IC_SET_IP:         m_register[offset] |= PCS_IP;               LOG("%s CIO Port %c Set IP\n", machine().describe_context(), port);         break;
		case IC_CLEAR_IP:       m_register[offset] &= ~PCS_IP;              LOG("%s CIO Port %c Clear IP\n", machine().describe_context(), port);       break;
		case IC_SET_IE:         m_register[offset] |= PCS_IE;               LOG("%s CIO Port %c Set IE\n", machine().describe_context(), port);         break;
		case IC_CLEAR_IE:       m_register[offset] &= ~PCS_IE;              LOG("%s CIO Port %c Clear IE\n", machine().describe_context(), port);       break;
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

		LOG("%s CIO Counter/Timer %u Trigger Command Bit: %u\n", machine().describe_context(), counter + 1, (data & CTCS_TCB) ? 1 : 0);
		LOG("%s CIO Counter/Timer %u Gate Command Bit: %u\n", machine().describe_context(), counter + 1, (data & CTCS_GCB) ? 1 : 0);
		LOG("%s CIO Counter/Timer %u Read Counter Control: %u\n", machine().describe_context(), counter + 1, (data & CTCS_RCC) ? 1 : 0);

		switch (data >> 5)
		{
		case IC_CLEAR_IP_IUS:   m_register[offset] &= ~(CTCS_IP | CTCS_IUS);LOG("%s CIO Counter/Timer %u Clear IP/IUS\n", machine().describe_context(), counter + 1);   break;
		case IC_SET_IUS:        m_register[offset] |= CTCS_IUS;             LOG("%s CIO Counter/Timer %u Set IUS\n", machine().describe_context(), counter + 1);        break;
		case IC_CLEAR_IUS:      m_register[offset] &= ~CTCS_IUS;            LOG("%s CIO Counter/Timer %u Clear IUS\n", machine().describe_context(), counter + 1);      break;
		case IC_SET_IP:         m_register[offset] |= CTCS_IP;              LOG("%s CIO Counter/Timer %u Set IP\n", machine().describe_context(), counter + 1);         break;
		case IC_CLEAR_IP:       m_register[offset] &= ~CTCS_IP;             LOG("%s CIO Counter/Timer %u Clear IP\n", machine().describe_context(), counter + 1);       break;
		case IC_SET_IE:         m_register[offset] |= CTCS_IE;              LOG("%s CIO Counter/Timer %u Set IE\n", machine().describe_context(), counter + 1);         break;
		case IC_CLEAR_IE:       m_register[offset] &= ~CTCS_IE;             LOG("%s CIO Counter/Timer %u Clear IE\n", machine().describe_context(), counter + 1);       break;
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
		// TODO: take data path polarity into account
		m_output[PORT_A] = data;
		m_write_pa(data);
		break;

	case PORT_B_DATA:
		// TODO: take data path polarity into account
		m_output[PORT_B] = data;
		m_write_pb(data);
		break;

	case PORT_C_DATA:
		{
		u8 mask = (data & 0xf0) | (data >> 4);

		m_output[PORT_C] = (m_output[PORT_C] & mask) | ((data & 0x0f) & (mask ^ 0xff));

		// TODO: take data path polarity into account
		m_write_pc(m_output[PORT_C]);
		}
		break;

	case COUNTER_TIMER_1_TIME_CONSTANT_MS_BYTE:
	case COUNTER_TIMER_2_TIME_CONSTANT_MS_BYTE:
	case COUNTER_TIMER_3_TIME_CONSTANT_MS_BYTE:
		LOG("%s CIO Counter/Timer %u Time Constant MSB: %02x\n", machine().describe_context(), ((offset - COUNTER_TIMER_1_TIME_CONSTANT_MS_BYTE) >> 1) + 1, data);

		m_register[offset] = data;
		break;

	case COUNTER_TIMER_1_TIME_CONSTANT_LS_BYTE:
	case COUNTER_TIMER_2_TIME_CONSTANT_LS_BYTE:
	case COUNTER_TIMER_3_TIME_CONSTANT_LS_BYTE:
		LOG("%s CIO Counter/Timer %u Time Constant LSB: %02x\n", machine().describe_context(), ((offset - COUNTER_TIMER_1_TIME_CONSTANT_LS_BYTE) >> 1) + 1, data);

		m_register[offset] = data;
		break;

	case COUNTER_TIMER_1_MODE_SPECIFICATION:
	case COUNTER_TIMER_2_MODE_SPECIFICATION:
	case COUNTER_TIMER_3_MODE_SPECIFICATION:
		if (VERBOSE)
		{
			int const counter = offset - COUNTER_TIMER_1_MODE_SPECIFICATION;
			int const dcs = data & CTMS_DCS_MASK;

			LOG("%s CIO Counter/Timer %u Mode: %s\n", machine().describe_context(), counter + 1, (data & CTMS_CSC) ? "Continuous" : "Single Cycle");
			LOG("%s CIO Counter/Timer %u External Output Enable: %u\n", machine().describe_context(), counter + 1, (data & CTMS_EOE) ? 1 : 0);
			LOG("%s CIO Counter/Timer %u External Count Enable: %u\n", machine().describe_context(), counter + 1, (data & CTMS_ECE) ? 1 : 0);
			LOG("%s CIO Counter/Timer %u External Trigger Enable: %u\n", machine().describe_context(), counter + 1, (data & CTMS_ETE) ? 1 : 0);
			LOG("%s CIO Counter/Timer %u External Gate Enable: %u\n", machine().describe_context(), counter + 1, (data & CTMS_EDE) ? 1 : 0);
			LOG("%s CIO Counter/Timer %u Retrigger Enable: %u\n", machine().describe_context(), counter + 1, (data & CTMS_REB) ? 1 : 0);
			LOG("%s CIO Counter/Timer %u Output Duty Cycle: %s\n", machine().describe_context(), counter + 1, CTMS_DCS[dcs]);
		}

		m_register[offset] = data;
		break;

	case PORT_A_MODE_SPECIFICATION:
	case PORT_B_MODE_SPECIFICATION:
		if (VERBOSE)
		{
			char const port = BIT(offset, 3) ? 'B' : 'A';
			int const pts = (data & PMS_PTS_MASK) >> 6;
			int const pms = (data & PMS_PMS_MASK) >> 1;

			LOG("%s CIO Port %c Port Type: %s\n", machine().describe_context(), port, PMS_PTS[pts]);
			LOG("%s CIO Port %c Interrupt on 2 Bytes: %u\n", machine().describe_context(), port, (data & PMS_ITB) ? 1 : 0);
			LOG("%s CIO Port %c Single Buffer: %u\n", machine().describe_context(), port, (data & PMS_SB) ? 1 : 0);
			LOG("%s CIO Port %c Interrupt on Match Only: %u\n", machine().describe_context(), port, (data & PMS_IMO) ? 1 : 0);
			LOG("%s CIO Port %c Pattern Mode: %s\n", machine().describe_context(), port, PMS_PMS[pms]);

			if (pts == PTS_BIT)
				LOG("%s CIO Port %c Latch on Pattern Match: %u\n", machine().describe_context(), port, (data & PMS_LPM) ? 1 : 0);
			else
				LOG("%s CIO Port %c Deskew Timer Enable: %u\n", machine().describe_context(), port, (data & PMS_DTE) ? 1 : 0);
		}

		m_register[offset] = data;
		break;

	case PORT_A_HANDSHAKE_SPECIFICATION:
	case PORT_B_HANDSHAKE_SPECIFICATION:
		// TODO
		break;

	case PORT_A_DATA_PATH_POLARITY:
	case PORT_B_DATA_PATH_POLARITY:
		LOG("%s CIO Port %c Data Path Polarity: %02x\n", machine().describe_context(), BIT(offset, 3) ? 'B' : 'A', data);
		m_register[offset] = data;
		break;

	case PORT_A_DATA_DIRECTION:
	case PORT_B_DATA_DIRECTION:
		LOG("%s CIO Port %c Data Direction: %02x\n", machine().describe_context(), BIT(offset, 3) ? 'B' : 'A', data);
		m_register[offset] = data;
		break;

	case PORT_A_SPECIAL_IO_CONTROL:
	case PORT_B_SPECIAL_IO_CONTROL:
		LOG("%s CIO Port %c Special I/O Control: %02x\n", machine().describe_context(), BIT(offset, 3) ? 'B' : 'A', data);
		m_register[offset] = data;
		break;

	case PORT_A_PATTERN_POLARITY:
	case PORT_B_PATTERN_POLARITY:
		LOG("%s CIO Port %c Pattern Polarity: %02x\n", machine().describe_context(), BIT(offset, 3) ? 'B' : 'A', data);
		m_register[offset] = data;
		break;

	case PORT_A_PATTERN_TRANSITION:
	case PORT_B_PATTERN_TRANSITION:
		LOG("%s CIO Port %c Pattern Transition: %02x\n", machine().describe_context(), BIT(offset, 3) ? 'B' : 'A', data);
		m_register[offset] = data;
		break;

	case PORT_A_PATTERN_MASK:
	case PORT_B_PATTERN_MASK:
		LOG("%s CIO Port %c Pattern Mask: %02x\n", machine().describe_context(), BIT(offset, 3) ? 'B' : 'A', data);
		m_register[offset] = data;
		match_pattern(BIT(offset, 3));
		check_interrupt();
		break;

	default:
		logerror("%s: Z8536 Unimplemented write %02x to register %u\n", machine().describe_context(), data, offset);
		m_register[offset] = data;
	}
}


//-------------------------------------------------
//  write_register - masked write to register
//-------------------------------------------------

void cio_base_device::write_register(offs_t offset, u8 data, u8 mask)
{
	u8 combined_data = (data & mask) | (m_register[offset] & (mask ^ 0xff));

	write_register(offset, combined_data);
}


//-------------------------------------------------
//   counter_enabled - is counter enabled?
//-------------------------------------------------

bool cio_base_device::counter_enabled(int id)
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

bool cio_base_device::counter_external_output(int id)
{
	return (m_register[COUNTER_TIMER_1_MODE_SPECIFICATION + id] & CTMS_EOE) ? true : false;
}


//-------------------------------------------------
//   counter_external_count -
//-------------------------------------------------

bool cio_base_device::counter_external_count(int id)
{
	return (m_register[COUNTER_TIMER_1_MODE_SPECIFICATION + id] & CTMS_ECE) ? true : false;
}


//-------------------------------------------------
//   counter_external_trigger -
//-------------------------------------------------

bool cio_base_device::counter_external_trigger(int id)
{
	return (m_register[COUNTER_TIMER_1_MODE_SPECIFICATION + id] & CTMS_ETE) ? true : false;
}


//-------------------------------------------------
//   counter_external_gate -
//-------------------------------------------------

bool cio_base_device::counter_external_gate(int id)
{
	return (m_register[COUNTER_TIMER_1_MODE_SPECIFICATION + id] & CTMS_EDE) ? true : false;
}


//-------------------------------------------------
//   counter_gated -
//-------------------------------------------------

bool cio_base_device::counter_gated(int id)
{
	return (m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + id] & CTCS_GCB) ? true : false;
}


//-------------------------------------------------
//   count - count down
//-------------------------------------------------

void cio_base_device::count(int id)
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
			LOG("%s CIO Counter/Timer %u Interrupt Pending\n", machine().describe_context(), id + 1);

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
			LOG("%s CIO Counter/Timer %u Terminal Count\n", machine().describe_context(), id + 1);

			// clear count in progress bit
			m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + id] &= ~CTCS_CIP;
		}

		check_interrupt();
	}
}


//-------------------------------------------------
//  trigger -
//-------------------------------------------------

void cio_base_device::trigger(int id)
{
	// ignore triggers during countdown if retrigger is disabled
	if (!(m_register[COUNTER_TIMER_1_MODE_SPECIFICATION + id] & CTMS_REB) && (m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + id] & CTCS_CIP)) return;

	LOG("%s CIO Counter/Timer %u Trigger\n", machine().describe_context(), id + 1);

	// load counter with time constant
	m_counter[id] = (m_register[COUNTER_TIMER_1_TIME_CONSTANT_MS_BYTE + (id << 1)] << 8) | m_register[COUNTER_TIMER_1_TIME_CONSTANT_LS_BYTE + (id << 1)];

	// set count in progress bit
	m_register[COUNTER_TIMER_1_COMMAND_AND_STATUS + id] |= CTCS_CIP;
}


//-------------------------------------------------
//  gate -
//-------------------------------------------------

void cio_base_device::gate(int id, int state)
{
	// TODO
}


//-------------------------------------------------
//  match_pattern -
//-------------------------------------------------

void cio_base_device::match_pattern(int port)
{
	u8 pms = m_register[PORT_A_MODE_SPECIFICATION + (port << 3)];
	u8 pm = m_register[PORT_A_PATTERN_MASK + (port << 3)];
	u8 ddr = m_register[PORT_A_DATA_DIRECTION + (port << 3)];

	switch ((pms & PMS_PMS_MASK) >> 1)
	{
	case PMS_OR_PEV:
		m_match[port] = m_input[port] & ddr & pm;

		if (m_match[port])
		{
			LOG("%s CIO Port %c Interrupt Pending\n", machine().describe_context(), 'A' + port);
			m_register[PORT_A_COMMAND_AND_STATUS + port] |= PCS_IP;
			check_interrupt();
		}
		break;
	}
}


//-------------------------------------------------
//  external_port_w - external port write
//-------------------------------------------------

void cio_base_device::external_port_w(int port, int bit, int state)
{
	switch (port)
	{
	case PORT_A:
	case PORT_B:
		{
		assert((PORT_A_DATA_DIRECTION + (port << 3)) >= 0 && (PORT_A_DATA_DIRECTION + (port << 3)) < std::size(m_register));
		u8 ddr = m_register[PORT_A_DATA_DIRECTION + (port << 3)];

		if (!BIT(ddr, bit)) return;

		LOG("%s CIO Port %c Bit %u: %u\n", machine().describe_context(), 'A' + port, bit, state);

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
//  cio_base_device - constructor
//-------------------------------------------------

cio_base_device::cio_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_write_irq(*this),
	m_read_pa(*this, 0),
	m_write_pa(*this),
	m_read_pb(*this, 0),
	m_write_pb(*this),
	m_read_pc(*this, 0),
	m_write_pc(*this),
	m_irq(CLEAR_LINE)
{
}


//-------------------------------------------------
//  z8036_device - constructor
//-------------------------------------------------

z8036_device::z8036_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	cio_base_device(mconfig, Z8036, tag, owner, clock)
{
}


//-------------------------------------------------
//  z8536_device - constructor
//-------------------------------------------------

z8536_device::z8536_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	cio_base_device(mconfig, Z8536, tag, owner, clock),
	device_z80daisy_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cio_base_device::device_start()
{
	for (int i = 0; i < 3; i++)
	{
		m_input[i] = 0;
		m_output[i] = 0;
		m_buffer[i] = 0;
		m_match[i] = 0;
	}

	// allocate timer
	m_timer = timer_alloc(FUNC(cio_base_device::advance_counters), this);
	m_timer->adjust(attotime::from_hz(clock() / 2), 0, attotime::from_hz(clock() / 2));

	save_item(NAME(m_irq));
	save_item(NAME(m_register));
	save_item(NAME(m_input));
	save_item(NAME(m_output));
	save_item(NAME(m_buffer));
	save_item(NAME(m_match));
	save_item(NAME(m_counter));
}

void z8536_device::device_start()
{
	cio_base_device::device_start();

	save_item(NAME(m_state0));
	save_item(NAME(m_pointer));
}


//-------------------------------------------------
//  device_start - device-specific reset
//-------------------------------------------------

void cio_base_device::device_reset()
{
	for (auto & elem : m_register)
	{
		elem = 0;
	}

	m_register[MASTER_INTERRUPT_CONTROL] = MICR_RESET;
	m_register[PORT_A_COMMAND_AND_STATUS] = PCS_ORE;
	m_register[PORT_B_COMMAND_AND_STATUS] = PCS_ORE;
	m_register[CURRENT_VECTOR] = 0xff;
	m_register[PORT_A_DATA_DIRECTION] = 0xff;
	m_register[PORT_B_DATA_DIRECTION] = 0xff;
	m_register[PORT_C_DATA_DIRECTION] = 0x0f;

	check_interrupt();
}

void z8536_device::device_reset()
{
	m_pointer = 0;
	m_state0 = false;

	cio_base_device::device_reset();
}


//-------------------------------------------------
//  advance_counters -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(cio_base_device::advance_counters)
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
	static const int prio[] =
	{
		COUNTER_TIMER_3_COMMAND_AND_STATUS,
		PORT_A_COMMAND_AND_STATUS,
		COUNTER_TIMER_2_COMMAND_AND_STATUS,
		PORT_B_COMMAND_AND_STATUS,
		COUNTER_TIMER_1_COMMAND_AND_STATUS
	};

	if (m_register[MASTER_INTERRUPT_CONTROL] & MICR_MIE)
	{
		for (int i = 0; i < 5; i++)
		{
			if (m_register[prio[i]] & PCS_IUS)
			{
				// we are currently servicing an interrupt request
				return Z80_DAISY_IEO;
			}
			else if ((m_register[prio[i]] & PCS_IE) && (m_register[prio[i]] & PCS_IP))
			{
				// indicate that we have an interrupt request waiting
				return Z80_DAISY_INT;
			}
		}
	}

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


//-------------------------------------------------
//  intack_r - interrupt acknowledge
//-------------------------------------------------

int cio_base_device::intack_r()
{
	get_interrupt_vector();
	int data = m_register[CURRENT_VECTOR];

	LOG("%s CIO Interrupt Acknowledge: %02x\n", machine().describe_context(), data);

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


//**************************************************************************
//  INTERNAL STATE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  read - register read (Z-Bus)
//-------------------------------------------------

u8 z8036_device::read(offs_t offset)
{
	return is_reset() ? 0 : read_register(offset);
}


//-------------------------------------------------
//  read - register read (universal bus)
//-------------------------------------------------

u8 z8536_device::read(offs_t offset)
{
	u8 data = 0;

	if (!is_reset())
	{
		switch (offset & 0x03)
		{
		case EXT_PORT_C:
			data = read_register(PORT_C_DATA);
			break;

		case EXT_PORT_B:
			data = read_register(PORT_B_DATA);
			break;

		case EXT_PORT_A:
			data = read_register(PORT_A_DATA);
			break;

		case EXT_CONTROL:
			// state 0 or state 1: read data
			data = read_register(m_pointer);

			// return to state 0 after read
			if (!machine().side_effects_disabled())
				m_state0 = true;

			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  write - register write (Z-Bus)
//-------------------------------------------------

void z8036_device::write(offs_t offset, u8 data)
{
	if (is_reset())
	{
		// writes to other registers are ignored during reset
		if (offset != MASTER_INTERRUPT_CONTROL)
			return;

		// RESET bit must be written before all others
		data &= 0x01;
	}

	write_register(offset, data);
}


//-------------------------------------------------
//  write - register write (universal bus)
//-------------------------------------------------

void z8536_device::write(offs_t offset, u8 data)
{
	if (is_reset())
	{
		// write RESET bit
		write_register(m_pointer, data, 0x01);

		// proceed to state 0 if no longer reset
		m_state0 = !is_reset();
	}
	else
	{
		switch (offset & 0x03)
		{
		case EXT_PORT_C:
			write_register(PORT_C_DATA, data);
			break;

		case EXT_PORT_B:
			write_register(PORT_B_DATA, data);
			break;

		case EXT_PORT_A:
			write_register(PORT_A_DATA, data);
			break;

		case EXT_CONTROL:
			if (m_state0)
			{
				// state 0: write pointer
				m_pointer = data;

				// proceed to state 1
				m_state0 = false;
			}
			else
			{
				// state 1: write data
				write_register(m_pointer, data);

				// return to state 0 unless reset
				m_state0 = !is_reset();
			}
			break;
		}
	}
}
