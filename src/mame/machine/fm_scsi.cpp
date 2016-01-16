// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *
 *  Implementation of Fujitsu FMR-50, FMR-60 and FM-Towns SCSI hardware
 *
 *

 info from Toshiya Takeda's e-FMR50 source (the Towns uses the same controller?)
 0xc30 = Data register
 0xc32 = Status register (read)
         bit 7 = REQ
         bit 6 = I/O
         bit 5 = MSG
         bit 4 = C/D
         bit 3 = BUSY
         bit 1 = INT
         bit 0 = PERR
 0xc32 = Control register (write)
         bit 7 = WEN
         bit 6 = IMSK
         bit 4 = ATN
         bit 2 = SEL
         bit 1 = DMAE
         bit 0 = RST
*/
#include "emu.h"
#include "fm_scsi.h"
#include "debugger.h"

/*
 *  Device config
 */

const device_type FMSCSI = &device_creator<fmscsi_device>;

/*
 * Device
 */

fmscsi_device::fmscsi_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: legacy_scsi_host_adapter(mconfig, FMSCSI, "FM-SCSI", tag, owner, clock, "fmscsi", __FILE__),
	m_irq_handler(*this),
	m_drq_handler(*this)
{
}

void fmscsi_device::device_start()
{
	m_input_lines = 0;
	m_output_lines = 0;
	m_data = 0;
	m_command_index = 0;
	m_last_id = 0;
	m_target = 0;
	m_phase = SCSI_PHASE_BUS_FREE;

	m_irq_handler.resolve_safe();
	m_drq_handler.resolve_safe();

	// allocate read timer
	m_transfer_timer = timer_alloc(TIMER_TRANSFER);
	m_phase_timer = timer_alloc(TIMER_PHASE);
}

void fmscsi_device::device_reset()
{
	m_input_lines = 0;
	m_output_lines = 0;
	m_data = 0;
	m_command_index = 0;
	m_last_id = 0;
	m_target = 0;
	m_result_length = 0;
	m_result_index = 0;
	stop_transfer();
	set_input_line(FMSCSI_LINE_INT,0);
	m_irq_handler(0);

	m_phase = SCSI_PHASE_BUS_FREE;
}

// get the length of a SCSI command based on it's command byte type
int fmscsi_device::get_scsi_cmd_len(UINT8 cbyte)
{
	int group;

	group = (cbyte>>5) & 7;

	if (group == 0) return 6;
	if (group == 1 || group == 2) return 10;
	if (group == 5) return 12;

	fatalerror("fmscsi: Unknown SCSI command group %d\n", group);

	// never executed
	//return 6;
}

void fmscsi_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_TRANSFER:
		set_input_line(FMSCSI_LINE_REQ,1);
		//logerror("FMSCSI: timer triggered: %i/%i\n",m_result_index,m_result_length);
		if(m_output_lines & FMSCSI_LINE_DMAE)
		{
			m_drq_handler(1);
		}
		break;
	case TIMER_PHASE:
		set_phase(param);
		break;
	}
}

void fmscsi_device::stop_transfer()
{
	m_transfer_timer->adjust(attotime::never);  // stop timer
	m_phase_timer->adjust(attotime::from_usec(800),SCSI_PHASE_STATUS);
	if(m_output_lines & FMSCSI_LINE_DMAE)
	{
		m_drq_handler(0);
	}
	logerror("FMSCSI: Stopping transfer : (%i/%i)\n",m_result_index,m_result_length);
}

UINT8 fmscsi_device::fmscsi_data_r(void)
{
	// read from data bus
	// ACK is automatic on accessing the data bus, so REQ will go low.
	set_input_line(FMSCSI_LINE_REQ,0);

	if(m_phase == SCSI_PHASE_DATAIN)
	{
		m_data = m_buffer[m_result_index % 512];
		//logerror("FMSCSI: DATAIN phase read data %02x\n",m_data);
		m_result_index++;
		if(m_result_index % 512 == 0)
			read_data(m_buffer,512);  // read next sector

		if(m_result_index >= m_result_length)
		{
			// end of data transfer
			stop_transfer();
		}
		return m_data;
	}

	if(m_phase == SCSI_PHASE_MESSAGE_IN)
	{
		m_data = 0;  // command complete message
		m_phase_timer->adjust(attotime::from_usec(800),SCSI_PHASE_BUS_FREE);
		m_command_index = 0;
		return m_data;
	}

	if(m_phase == SCSI_PHASE_STATUS)
	{
		m_data = get_status();
		// no command complete message?
		m_phase_timer->adjust(attotime::from_usec(800),SCSI_PHASE_MESSAGE_IN);
		m_command_index = 0;
		//set_input_line(FMSCSI_LINE_REQ,1);  // raise REQ yet again
		return m_data;
	}

	return m_data;
}

void fmscsi_device::fmscsi_data_w(UINT8 data)
{
	int phase;

	// write to data bus
	m_data = data;
	//logerror("FMSCSI: data write %02x\n",data);

	// ACK is automatic on accessing the data bus, so REQ will go low.
	set_input_line(FMSCSI_LINE_REQ,0);

	if(m_phase == SCSI_PHASE_BUS_FREE)
	{
		// select target
		switch(data & 0x7f)
		{
		case 0x01: m_target = 0; break;
		case 0x02: m_target = 1; break;
		case 0x04: m_target = 2; break;
		case 0x08: m_target = 3; break;
		case 0x10: m_target = 4; break;
		}
	}
	if(m_phase == SCSI_PHASE_DATAOUT)
	{
		m_buffer[m_result_index % 512] = m_data;
		m_result_index++;
		if(m_result_index % 512 == 0)
			write_data(m_buffer,512);  // write buffer to disc
		if(m_result_index >= m_result_length)
		{
			// end of data transfer
			m_transfer_timer->adjust(attotime::never);  // stop timer
			m_phase_timer->adjust(attotime::from_usec(800),SCSI_PHASE_STATUS);
			if(m_output_lines & FMSCSI_LINE_DMAE)
			{
				m_drq_handler(0);
			}
			logerror("FMSCSI: Stopping transfer : (%i/%i)\n",m_result_index,m_result_length);
		}
	}
	if(m_phase == SCSI_PHASE_COMMAND)
	{
		m_command[m_command_index] = data;
		logerror("FMSCSI: writing command byte %02x [%i]\n",data,m_command_index);
		m_command_index++;
		if(m_command_index >= get_scsi_cmd_len(m_command[0]))
		{
			// command complete
			send_command(m_command,m_command_index);
			m_result_length = get_length();
			phase = legacy_scsi_host_adapter::get_phase();
			m_phase_timer->adjust(attotime::from_usec(800),phase);

			logerror("FMSCSI: Command %02x sent, result length = %i\n",m_command[0],m_result_length);
		}
		else
		{
			m_phase_timer->adjust(attotime::from_usec(800),SCSI_PHASE_COMMAND);
		}
	}
	if(m_phase == SCSI_PHASE_MESSAGE_OUT)
	{
		m_phase_timer->adjust(attotime::from_usec(800),SCSI_PHASE_STATUS);
	}
}

void fmscsi_device::set_phase(int phase)
{
	m_phase = phase;
	logerror("FMSCSI: phase set to %i\n",m_phase);
	// set input lines accordingly
	switch(phase)
	{
	case SCSI_PHASE_BUS_FREE:
		set_input_line(FMSCSI_LINE_BSY,0);
		set_input_line(FMSCSI_LINE_CD,0);
		set_input_line(FMSCSI_LINE_MSG,0);
		set_input_line(FMSCSI_LINE_IO,0);
		//set_input_line(FMSCSI_LINE_REQ,0);
		break;
	case SCSI_PHASE_COMMAND:
		set_input_line(FMSCSI_LINE_BSY,1);
		set_input_line(FMSCSI_LINE_CD,1);
		set_input_line(FMSCSI_LINE_MSG,0);
		set_input_line(FMSCSI_LINE_IO,0);
		set_input_line(FMSCSI_LINE_REQ,1);
		break;
	case SCSI_PHASE_STATUS:
		set_input_line(FMSCSI_LINE_CD,1);
		set_input_line(FMSCSI_LINE_MSG,0);
		set_input_line(FMSCSI_LINE_IO,1);
		set_input_line(FMSCSI_LINE_REQ,1);
		break;
	case SCSI_PHASE_DATAIN:
		set_input_line(FMSCSI_LINE_CD,0);
		set_input_line(FMSCSI_LINE_MSG,0);
		set_input_line(FMSCSI_LINE_IO,1);
		set_input_line(FMSCSI_LINE_REQ,1);
		// start transfer timer
		m_transfer_timer->adjust(attotime::zero,0,attotime::from_hz(3000000));  // arbitrary value for now
		read_data(m_buffer,512);
		m_result_index = 0;
		logerror("FMSCSI: Starting transfer in (%i)\n",m_result_length);
		break;
	case SCSI_PHASE_DATAOUT:
		set_input_line(FMSCSI_LINE_CD,0);
		set_input_line(FMSCSI_LINE_MSG,0);
		set_input_line(FMSCSI_LINE_IO,0);
		set_input_line(FMSCSI_LINE_REQ,1);
		// start transfer timer
		m_transfer_timer->adjust(attotime::zero,0,attotime::from_hz(3000000));  // arbitrary value for now
		m_result_index = 0;
		logerror("FMSCSI: Starting transfer out (%i)\n",m_result_length);
		break;
	case SCSI_PHASE_MESSAGE_IN:
		set_input_line(FMSCSI_LINE_CD,1);
		set_input_line(FMSCSI_LINE_MSG,1);
		set_input_line(FMSCSI_LINE_IO,1);
		set_input_line(FMSCSI_LINE_REQ,1);
		break;
	case SCSI_PHASE_MESSAGE_OUT:
		set_input_line(FMSCSI_LINE_CD,1);
		set_input_line(FMSCSI_LINE_MSG,1);
		set_input_line(FMSCSI_LINE_IO,0);
		set_input_line(FMSCSI_LINE_REQ,1);
		break;
	}
}

int fmscsi_device::get_phase(void)
{
	return m_phase;
}

void fmscsi_device::set_input_line(UINT8 line, UINT8 state)
{
	if(line == FMSCSI_LINE_REQ)
	{
		if(state != 0 && !(m_input_lines & FMSCSI_LINE_REQ))  // low to high
		{
			if(m_output_lines & FMSCSI_LINE_IMSK && m_phase != SCSI_PHASE_DATAIN && m_phase != SCSI_PHASE_DATAOUT)
			{
				set_input_line(FMSCSI_LINE_INT,1);
				m_irq_handler(1);
				logerror("FMSCSI: IRQ high\n");
			}
		}
		if(state == 0 && (m_input_lines & FMSCSI_LINE_REQ))  // high to low
		{
			if(m_output_lines & FMSCSI_LINE_IMSK && m_phase != SCSI_PHASE_DATAIN && m_phase != SCSI_PHASE_DATAOUT)
			{
				set_input_line(FMSCSI_LINE_INT,0);
				m_irq_handler(0);
				logerror("FMSCSI: IRQ low\n");
			}
		}
	}
	if(state != 0)
		m_input_lines |= line;
	else
		m_input_lines &= ~line;
//  logerror("FMSCSI: input line %02x set to %i\n",line,state);
}

UINT8 fmscsi_device::get_input_line(UINT8 line)
{
	return m_input_lines & line;
}

void fmscsi_device::set_output_line(UINT8 line, UINT8 state)
{
	if(line == FMSCSI_LINE_RST && state != 0)
	{
		device_reset();
		logerror("FMSCSI: reset\n");
	}

	if(line == FMSCSI_LINE_SEL)
	{
		if(state != 0 && !(m_output_lines & FMSCSI_LINE_SEL)) // low to high transition
		{
			if (select(m_target))
			{
				m_phase_timer->adjust(attotime::from_usec(800),SCSI_PHASE_COMMAND);
				m_data = 0x08;
			}
		}
	}

	if(line == FMSCSI_LINE_ATN)
	{
		if(state != 0)
			m_phase_timer->adjust(attotime::from_usec(800),SCSI_PHASE_MESSAGE_OUT);
	}

	if(state != 0)
		m_output_lines |= line;
	else
		m_output_lines &= ~line;
//  logerror("FMSCSI: output line %02x set to %i\n",line,state);
}

UINT8 fmscsi_device::get_output_line(UINT8 line)
{
	return m_output_lines & line;
}

UINT8 fmscsi_device::fmscsi_status_r(void)
{
	// status inputs
	return m_input_lines;
}

void fmscsi_device::fmscsi_control_w(UINT8 data)
{
	// control outputs
	set_output_line(FMSCSI_LINE_RST,data & FMSCSI_LINE_RST);
	set_output_line(FMSCSI_LINE_DMAE,data & FMSCSI_LINE_DMAE);
	set_output_line(FMSCSI_LINE_IMSK,data & FMSCSI_LINE_IMSK);
	set_output_line(FMSCSI_LINE_ATN,data & FMSCSI_LINE_ATN);
	set_output_line(FMSCSI_LINE_WEN,data & FMSCSI_LINE_WEN);
	set_output_line(FMSCSI_LINE_SEL,data & FMSCSI_LINE_SEL);
	logerror("FMSCSI: control write %02x\n",data);
}

READ8_MEMBER( fmscsi_device::fmscsi_r )
{
	switch(offset & 0x03)
	{
	case 0x00:
		return fmscsi_data_r();
	case 0x01:
		return fmscsi_status_r();
	case 0x02:
		return 0x80;  // Linux uses this port to detect the ability to do word transfers.  We'll tell it that it doesn't for now.
	default:
		logerror("FMSCSI: Unknown read at offset %i\n",offset);
	}
	return 0;
}

WRITE8_MEMBER( fmscsi_device::fmscsi_w )
{
	switch(offset & 0x03)
	{
	case 0x00:
		fmscsi_data_w(data);
		break;
	case 0x01:
		fmscsi_control_w(data);
		break;
	default:
		logerror("FMSCSI: Unknown write 0x%02x at offset %i\n",data,offset);
	}
}
