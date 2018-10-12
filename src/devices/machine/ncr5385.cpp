// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***********************************************************************

    NCR 5385E SCSI Controller

    TOOD:
    - Everything.

***********************************************************************/

#include "emu.h"
#include "ncr5385.h"

DEFINE_DEVICE_TYPE(NCR5385, ncr5385_device, "ncr5385", "NCR 5385E SCSI Controller")

ncr5385_device::ncr5385_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NCR5385, tag, owner, clock)
	, m_int(*this)
{
}

void ncr5385_device::device_start()
{
	m_int.resolve_safe();
}

void ncr5385_device::device_reset()
{
	m_state = STATE_IDLE;
	m_int_reg = 0;
	m_ctrl_reg = 0;
	m_aux_status_reg = AUX_STATUS_TC_ZERO;
	m_diag_status_reg = DIAG_COMPLETE;
}

WRITE8_MEMBER(ncr5385_device::write)
{
	switch (offset)
	{
		case 0x0: // Data Register
			switch (m_state)
			{
				case STATE_DIAGNOSTIC_GOOD_PARITY:
					m_aux_status_reg &= ~AUX_STATUS_PARITY_ERR;
					m_aux_status_reg |= AUX_STATUS_DATA_FULL;
					m_int_reg = INT_FUNC_COMPLETE;
					m_diag_status_reg = DIAG_COMPLETE | DIAG_TURN_GOOD_PARITY;
					m_state = STATE_IDLE;
					m_int(1);
					logerror("%s: ncr5385_w: data=%02x (diagnostic w/ good parity)\n", machine().describe_context(), data);
					break;
				case STATE_DIAGNOSTIC_BAD_PARITY:
					m_aux_status_reg |= AUX_STATUS_PARITY_ERR | AUX_STATUS_DATA_FULL;
					m_int_reg = INT_FUNC_COMPLETE;
					m_diag_status_reg = DIAG_COMPLETE | DIAG_TURN_BAD_PARITY;
					m_state = STATE_IDLE;
					m_int(1);
					logerror("%s: ncr5385_w: data=%02x (diagnostic w/ bad parity)\n", machine().describe_context(), data);
					break;
				default:
					logerror("%s: ncr5385_w: data=%02x\n", machine().describe_context(), data);
					break;
			}
			break;
		case 0x1: // Command Register
			switch (data & 0x3f)
			{
				case 0x00: // Chip Reset
					logerror("%s: ncr5385_w: command: reset\n", machine().describe_context());
					m_state = STATE_IDLE;
					m_int_reg = 0;
					m_aux_status_reg = AUX_STATUS_TC_ZERO;
					m_diag_status_reg = DIAG_COMPLETE;
					m_int(0);
					break;
				case 0x0b: // Diagnostic
					logerror("%s: ncr5385_w: command: diagnostic (%s parity)\n", machine().describe_context(), BIT(data, 6) ? "bad" : "good");
					if (BIT(data, 6))
						m_state = STATE_DIAGNOSTIC_BAD_PARITY;
					else
						m_state = STATE_DIAGNOSTIC_GOOD_PARITY;
					break;
				default:
					logerror("%s: ncr5385_w: command: %02x\n", machine().describe_context(), data);
					break;
			}
			break;
		case 0x2: // Control Register
			m_ctrl_reg = data & 0x07;
			logerror("%s: ncr5385_w: control: parity_en=%d, reselect_en=%d, select_en=%d\n", machine().describe_context(), BIT(data, CTRL_PARITY_BIT), BIT(data, CTRL_RESELECT_BIT), BIT(data, CTRL_SELECT_BIT));
			break;
		default:
			logerror("%s: ncr5385_w: %x=%02x\n", machine().describe_context(), offset, data);
			break;
	}
}

READ8_MEMBER(ncr5385_device::read)
{
	switch (offset)
	{
		case 0x2:
			logerror("%s: ncr5385_r: control (%02x)\n", machine().describe_context(), m_ctrl_reg);
			return m_ctrl_reg;
		case 0x4:
			logerror("%s: ncr5385_r: aux status (%02x)\n", machine().describe_context(), m_aux_status_reg);
			return m_aux_status_reg;
		case 0x6:
			logerror("%s: ncr5385_r: interrupt (%02x)\n", machine().describe_context(), m_int_reg);
			m_int(1);
			return m_int_reg;
		case 0x9:
			logerror("%s: ncr5385_r: diagnostic status (%02x)\n", machine().describe_context(), m_diag_status_reg);
			return m_diag_status_reg;
		default:
			logerror("%s: ncr5385_r: %x (%02x)\n", machine().describe_context(), offset, 0);
			return 0;
	}
}
