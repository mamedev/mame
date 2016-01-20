// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *  Fujitsu MB89352A SCSI Protocol Controller
 *
 *  Should be compatible with the MB87030/31, and MB89351
 *
 *  Used on the Sharp X68000 Super, X68000 XVI and X68030 (internal), and on SCSI expansion cards for any X680x0 (external)
 *
 *  Registers (based on datasheet):
 *
 *  0: BDID (Bus Device ID)
 *     bit = device ID (ie: bit 2 = device 2) (read)
 *     bit 0 = ID1, bit 1 = ID2, bit 2 = ID4 (write)
 *
 *  1: SCTL (SPC Control)
 *     (all read/write)
 *     bit 0 = INT Enable
 *     bit 1 = Reselect Enable
 *     bit 2 = Select Enable
 *     bit 3 = Parity Enable
 *     bit 4 = Arbitration Phase Enable
 *     bit 5 = Diag Mode
 *     bit 6 = Control Reset
 *     bit 7 = Reset and Disable
 *
 *  2: SCMD (Command)
 *     bits 0-2 = Transfer Modifier (read)
 *     bit 0 = Term Mode (write)
 *     bit 2 = PRG Xfer (write)
 *     bit 3 = Intercept Xfer (read/write)
 *     bit 4 = RST Out (read/write)
 *     bits 5-7 = Command code (read/write) (would have been nice if these codes were mentioned in the datasheet...)
 *                000 = Bus release
 *                001 = Selection
 *                010 = Reset ATN
 *                011 = Set ATN
 *                100 = Transfer
 *                101 = Transfer Pause
 *                110 = Reset ACK/REQ
 *                111 = Set ACK/REQ
 *
 *  3: Unused
 *
 *  4: INTS (Interrupt Sense)
 *     on write, clear interrupt
 *     on read:
 *     bit 0 = Reset Condition
 *     bit 1 = SPC Hard Error
 *     bit 2 = Timeout
 *     bit 3 = Service Required
 *     bit 4 = Command Complete
 *     bit 5 = Disconnect
 *     bit 6 = Reselected
 *     bit 7 = Selected
 *
 *  5: PSNS (Phase Sense) (read-only)
 *      b7                                          b0
 *     | REQ | ACK | ATN | SEL | BSY | MSG | C/D | I/O |
 *     SDGC (SPC Diag. Control) (write-only)
 *     bit 5 = Xfer Enable
 *     bit 4 = Unused
 *     all other bits are Diag. of the matching inputs above.
 *
 *  6: SSTS (SPC Status) (read-only)
 *     bit 0 = DREG Empty
 *     bit 1 = DREG Full
 *     bit 2 = TC=0
 *     bit 3 = SCSI RST
 *     bit 4 = Xfer in progress
 *     bit 5 = SPC BSY
 *     bit 6 = Connected to Target
 *     bit 7 = Connected to Initiator
 *
 *  7: SERR (SCSI Error Status) (read-only)
 *     bit 1 = Short Transfer Period
 *     bit 3 = TC Parity Error
 *     bit 5 = Xfer Out (related to SDGC bit 5)
 *     bits 6,7 = Data error (10 = undefined, 00 = no error, 01 and 11 = parity error)
 *
 *  8: PCTL (Phase Control)
 *     (read/write)
 *     bits 0-2 = transfer phase ( | MSG | C/D | I/O | )
 *     bits 7 = Bus Free interrupt enable
 *     other bits read 0
 *
 *  9: MBC (Modified Byte Counter)
 *     (read-only)
 *     bits 0-3 = MBC bits 0-3
 *     other bits read 0
 *
 *  A: DREG (Data Register)
 *     8-bit FIFO
 *
 *  B: TEMP (Temporary Register)
 *     on read, from SCSI
 *     on write, to SCSI
 *
 *  C, D, E: TCH, TCM, TCL (Transfer Counter High, Mid, Low)
 *     24-bit transfer counter
 *
 */

#include "emu.h"
#include "mb89352.h"

/*
 *  Device config
 */

const device_type MB89352A = &device_creator<mb89352_device>;


/*
 * Device
 */

mb89352_device::mb89352_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	legacy_scsi_host_adapter(mconfig, MB89352A, "MB89352A", tag, owner, clock, "mb89352", __FILE__),
	m_irq_cb(*this),
	m_drq_cb(*this)
{
}

void mb89352_device::device_start()
{
	legacy_scsi_host_adapter::device_start();

	m_phase = SCSI_PHASE_BUS_FREE;
	m_target = 0;
	m_command_index = 0;
	m_line_status = 0x00;
	m_spc_status = 0x01;  // presumably the data reg is empty to start with
	m_error_status = 0x00;
	m_transfer_count = 0;
	if(m_transfer_count == 0)
		m_spc_status |= SSTS_TC_ZERO;
	m_ints = 0x00;

	m_irq_cb.resolve_safe();
	m_drq_cb.resolve_safe();

	// allocate read timer
	m_transfer_timer = timer_alloc(TIMER_TRANSFER);
}

void mb89352_device::device_reset()
{
	m_phase = SCSI_PHASE_BUS_FREE;
	m_target = 0;
	m_command_index = 0;
	m_line_status = 0x00;
	m_error_status = 0x00;
	m_transfer_count = 0;
	m_spc_status = 0x05;  // presumably the data reg is empty to start with
	m_busfree_int_enable = 0;
}

void mb89352_device::device_stop()
{
}

// get the length of a SCSI command based on it's command byte type
int mb89352_device::get_scsi_cmd_len(UINT8 cbyte)
{
	int group;

	group = (cbyte>>5) & 7;

	if (group == 0) return 6;
	if (group == 1 || group == 2) return 10;
	if (group == 5) return 12;

	fatalerror("MB89352: Unknown SCSI command group %d\n", group);

	// never executed
	//return 6;
}

void mb89352_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_TRANSFER:
		// TODO: check interrupts are actually enabled
		{
			m_drq_cb(1);
		}
		break;
	}
}

void mb89352_device::set_phase(int phase)
{
	m_phase = phase;
	switch(phase)
	{
	case SCSI_PHASE_BUS_FREE:
		m_line_status = 0;
		m_spc_status &= ~SSTS_XFER_IN_PROGRESS;
		break;
	case SCSI_PHASE_COMMAND:
		m_line_status |= MB89352_LINE_REQ;
		m_line_status &= ~MB89352_LINE_ACK;
		m_line_status &= ~MB89352_LINE_MSG;
		m_line_status |= MB89352_LINE_CD;
		m_line_status &= ~MB89352_LINE_IO;
		break;
	case SCSI_PHASE_STATUS:
//      m_line_status |= MB89352_LINE_REQ;
//      m_line_status &= ~MB89352_LINE_ACK;
		m_line_status &= ~MB89352_LINE_MSG;
		m_line_status |= MB89352_LINE_CD;
		m_line_status |= MB89352_LINE_IO;
		break;
	case SCSI_PHASE_DATAIN:
//      m_line_status |= MB89352_LINE_REQ;
//      m_line_status &= ~MB89352_LINE_ACK;
		m_line_status &= ~MB89352_LINE_MSG;
		m_line_status &= ~MB89352_LINE_CD;
		m_line_status |= MB89352_LINE_IO;
		break;
	case SCSI_PHASE_DATAOUT:
//      m_line_status |= MB89352_LINE_REQ;
//      m_line_status &= ~MB89352_LINE_ACK;
		m_line_status &= ~MB89352_LINE_MSG;
		m_line_status &= ~MB89352_LINE_CD;
		m_line_status &= ~MB89352_LINE_IO;
		break;
	case SCSI_PHASE_MESSAGE_IN:
//      m_line_status |= MB89352_LINE_REQ;
//      m_line_status &= ~MB89352_LINE_ACK;
		m_line_status |= MB89352_LINE_MSG;
		m_line_status |= MB89352_LINE_CD;
		m_line_status |= MB89352_LINE_IO;
		break;
	case SCSI_PHASE_MESSAGE_OUT:
//      m_line_status |= MB89352_LINE_REQ;
//      m_line_status &= ~MB89352_LINE_ACK;
		m_line_status |= MB89352_LINE_MSG;
		m_line_status |= MB89352_LINE_CD;
		m_line_status &= ~MB89352_LINE_IO;
		break;
	}
	logerror("MB89352: phase set to %i\n",m_phase);
}

READ8_MEMBER( mb89352_device::mb89352_r )
{
	UINT8 ret;
	switch(offset & 0x0f)
	{
	case 0x00:  // BDID - Bus Device ID
		return (1 << m_bdid);
	case 0x01:  // SCTL - SPC Control
		ret = 0x00;
		if(m_arbit_enable)
			ret |= 0x10;
		if(m_parity_enable)
			ret |= 0x08;
		if(m_sel_enable)
			ret |= 0x04;
		if(m_resel_enable)
			ret |= 0x02;
		if(m_int_enable)
			ret |= 0x01;
		return ret;
	case 0x02:  // SCMD - Command
		return m_scmd;
	case 0x03:  // Unused
		return 0xff;
	case 0x04:  // INTS - Interrupt Sense
		return m_ints;
	case 0x05:  // PSNS - Phase Sense
		return m_line_status;  // active low -- but Human68k expects it to be zero?
	case 0x06:  // SSTS - SPC Status
		return m_spc_status;
	case 0x07:  // SERR - SPC Error Status
		/*  #define SERR_SCSI_PAR   0x80
		    #define SERR_SPC_PAR    0x40
		    #define SERR_TC_PAR     0x08
		    #define SERR_PHASE_ERR  0x04
		    #define SERR_SHORT_XFR  0x02
		    #define SERR_OFFSET     0x01*/
		return 0;
	case 0x08:  // PCTL - Phase Control
		return ((m_busfree_int_enable) ? (m_line_status & 0x07) | 0x80 : (m_line_status & 0x07));
	case 0x0a:  // DREG - Data register (for data transfers)
		if(m_spc_status & SSTS_XFER_IN_PROGRESS)
		{
			m_data = m_buffer[m_transfer_index % 512];
			m_transfer_index++;
			m_transfer_count--;
			if(m_transfer_index % 512 == 0)
				read_data(m_buffer, 512);
			if(m_transfer_count == 0)
			{
				// End of transfer
				m_spc_status &= ~SSTS_XFER_IN_PROGRESS;
				m_spc_status |= SSTS_DREG_EMPTY;
				m_ints |= INTS_COMMAND_COMPLETE;
				if(m_int_enable != 0)
					m_irq_cb(1);
				if(m_phase == SCSI_PHASE_MESSAGE_IN)
					set_phase(SCSI_PHASE_BUS_FREE);
				else if(m_phase == SCSI_PHASE_DATAIN)
					set_phase(SCSI_PHASE_STATUS);
			}
		}
		return m_data;
	case 0x0b:  // TEMP - Temporary
		logerror("mb89352: read temporary register.\n");
		return m_temp;
	case 0x0c:  // TCH - Transfer Counter High
		return (m_transfer_count & 0x00ff0000) >> 16;
	case 0x0d:  // TCM - Transfer Counter Mid
		return (m_transfer_count & 0x0000ff00) >> 8;
	case 0x0e:  // TCL - Transfer Counter Low
		return (m_transfer_count & 0x000000ff);
	default:
		logerror("mb89352: read from register %02x\n",offset & 0x0f);
	}
	return 0xff;
}

WRITE8_MEMBER( mb89352_device::mb89352_w )
{
	switch(offset & 0x0f)
	{
	case 0x00:  // BDID - Bus Device ID
		m_bdid = data;
		m_spc_status &= ~SSTS_TARG_CONNECTED;
		m_spc_status |= SSTS_INIT_CONNECTED;
		logerror("mb89352: BDID set to %i\n",data);
		break;
	case 0x01:  // SCTL - SPC Control
		if(data & 0x80)  // reset and disable
		{
			device_reset();
			logerror("mb89352: SCTL: Reset and disable.\n");
		}
		if(data & 0x10)
		{
			m_arbit_enable = 1;
			logerror("mb89352: SCTL: Arbitration enabled.\n");
		}
		else
			m_arbit_enable = 0;
		if(data & 0x08)
		{
			m_parity_enable = 1;
			logerror("mb89352: SCTL: Parity enabled.\n");
		}
		else
			m_parity_enable = 0;
		if(data & 0x04)
		{
			m_sel_enable = 1;
			logerror("mb89352: SCTL: Selection enabled.\n");
		}
		else
			m_sel_enable = 0;
		if(data & 0x02)
		{
			m_resel_enable = 1;
			logerror("mb89352: SCTL: Reselection enabled.\n");
		}
		else
			m_resel_enable = 0;
		if(data & 0x01)
		{
			m_int_enable = 1;
			logerror("mb89352: SCTL: Interrupts enabled.\n");
		}
		else
		{
			m_int_enable = 0;
			logerror("mb89352: SCTL: Interrupts disabled.\n");
		}
		break;
	case 0x02:  // SCMD - Command
		/* From NetBSD/x68k source
		#define SCMD_BUS_REL    0x00
		#define SCMD_SELECT     0x20
		#define SCMD_RST_ATN    0x40
		#define SCMD_SET_ATN    0x60
		#define SCMD_XFR        0x80
		#define SCMD_XFR_PAUSE  0xa0
		#define SCMD_RST_ACK    0xc0
		#define SCMD_SET_ACK    0xe0
		 */
		m_scmd = data;
		switch((data & 0xe0) >> 5)
		{
		case 0x00:
			// Bus Free
			m_line_status = 0;
			m_spc_status &= ~SSTS_TARG_CONNECTED;
			m_spc_status &= ~SSTS_INIT_CONNECTED;
			m_spc_status &= ~SSTS_XFER_IN_PROGRESS;
			set_phase(SCSI_PHASE_BUS_FREE);
			if(m_busfree_int_enable)
			{
				if(m_int_enable != 0)
					m_irq_cb(1);
			}
			logerror("mb89352: SCMD: Bus free\n");
			break;
		case 0x01:
			// Selection
			m_target = m_temp;
			m_target &= ~(1 << m_bdid);  // mask off the bit relating to initiator
			switch(m_target)
			{
			case 0x01: m_target = 0; break;
			case 0x02: m_target = 1; break;
			case 0x04: m_target = 2; break;
			case 0x08: m_target = 3; break;
			case 0x10: m_target = 4; break;
			case 0x20: m_target = 5; break;
			case 0x40: m_target = 6; break;
			case 0x80: m_target = 7; break;
			}
			if(m_sel_enable != 0)
			{
				//m_ints |= INTS_SELECTION;
			}
			select(m_target);
			set_phase(SCSI_PHASE_COMMAND); // straight to command phase, may need a delay between selection and command phases
			m_line_status |= MB89352_LINE_SEL;
			m_line_status |= MB89352_LINE_BSY;
			m_spc_status &= ~SSTS_TARG_CONNECTED;
			m_spc_status |= SSTS_INIT_CONNECTED;
			m_spc_status |= SSTS_SPC_BSY;
			m_ints |= INTS_COMMAND_COMPLETE;
			if(m_int_enable != 0)
				m_irq_cb(1);
			logerror("mb89352: SCMD: Selection (SCSI ID%i)\n",m_target);
			break;
		case 0x02:  // Reset ATN
			m_line_status &= ~MB89352_LINE_ATN;
			logerror("mb89352: SCMD: Reset ATN\n");
			break;
		case 0x03:  // Set ATN
			m_line_status |= MB89352_LINE_ATN;
			logerror("mb89352: SCMD: Set ATN\n");
			break;
		case 0x04:   // Transfer
			m_transfer_index = 0;
			m_spc_status |= SSTS_XFER_IN_PROGRESS;
			if(m_phase == SCSI_PHASE_DATAIN)  // if we are reading data...
			{
				m_spc_status &= ~SSTS_DREG_EMPTY;  // DREG is no longer empty
				read_data(m_buffer, 512);
			}
			if(m_phase == SCSI_PHASE_MESSAGE_IN)
			{
				m_spc_status &= ~SSTS_DREG_EMPTY;  // DREG is no longer empty
				m_data = 0;
				m_temp = 0x00;
				set_phase(SCSI_PHASE_BUS_FREE);
				m_spc_status &= ~SSTS_XFER_IN_PROGRESS;
				m_command_index = 0;
			}
			logerror("mb89352: SCMD: Start Transfer\n");
			break;
		case 0x05:  // Transfer pause
			logerror("mb89352: SCMD: Pause Transfer\n");
			break;
		case 0x06:  // reset REQ/ACK
			m_line_status &= ~MB89352_LINE_ACK;
			if(m_phase != SCSI_PHASE_BUS_FREE)
			{
				m_line_status |= MB89352_LINE_REQ;
			}
			else
			{
				m_spc_status &= ~SSTS_INIT_CONNECTED;
				m_spc_status &= ~SSTS_TARG_CONNECTED;
				m_spc_status &= ~SSTS_SPC_BSY;
			}
			logerror("mb89352: SCMD: Reset REQ/ACK\n");
			break;
		case 0x07:  // set REQ/ACK
			m_line_status &= ~MB89352_LINE_REQ;
			m_line_status |= MB89352_LINE_ACK;
			logerror("mb89352: SCMD: Set REQ/ACK\n");
			if(m_phase == SCSI_PHASE_COMMAND)
			{
				m_command[m_command_index++] = m_temp;  // temp register puts data onto the SCSI bus
				if(m_command_index >= get_scsi_cmd_len(m_command[0]))
				{
					int x;
					int phase;
					// execute SCSI command
					send_command(m_command, m_command_index);
					phase = get_phase();
					if(m_command[0] == 1) // Rezero Unit - not implemented in SCSI code
						set_phase(SCSI_PHASE_STATUS);
					else
						set_phase(phase);
					logerror("Command executed: ");
					for(x=0;x<m_command_index;x++)
						logerror(" %02x",m_command[x]);
					logerror("\n");
				}
				return;
			}
			if(m_phase == SCSI_PHASE_STATUS)
			{
				m_temp = get_status();
				set_phase(SCSI_PHASE_MESSAGE_IN);
				return;
			}
			if(m_phase == SCSI_PHASE_MESSAGE_IN)
			{
				m_temp = 0x00;
				set_phase(SCSI_PHASE_BUS_FREE);
				m_command_index = 0;
				return;
			}
			break;
		default:
			logerror("mb89352: SCMD: Unimplemented command %02x\n",(data & 0xe0) >> 5);
			break;
		}
		break;
	case 0x04:  // INTS - Interrupt Sense
		m_ints &= ~data;  // resets relevant status bits to zero
		m_irq_cb(0);  // clear IRQ
		logerror("mb89352: Reset INTS status bits %02x\n",data);
		break;
	case 0x08:  // PCTL - Phase control
		if((data & 0x80) == 0 && (m_phase == SCSI_PHASE_SELECT))  // if writing 0 to bit 7, selection phase is reset
		{
			m_ints &= ~INTS_SELECTION;
			m_target = 0;
			logerror("mb89352: PCTL selection cancelled\n");
		}
		// writing to the low 3 bits sets the phase
		if((m_phase & 0x07) != (data & 0x07))
			set_phase(data & 0x07);
		m_busfree_int_enable = data & 0x80;
		logerror("mb89352: PCTL write %02x\n",data);
		break;
	case 0x0a:  // DREG - Data register
		if(m_phase == SCSI_PHASE_COMMAND)
		{
			m_command[m_command_index++] = data;
			if(m_command_index >= get_scsi_cmd_len(m_command[0]))
			{
				int x;
				int phase;
				// execute SCSI command
				send_command(m_command, m_command_index);
				phase = get_phase();
				if(m_command[0] == 1) // Rezero Unit - not implemented in SCSI code
					set_phase(SCSI_PHASE_STATUS);
				else
					set_phase(phase);
				logerror("Command executed: ");
				for(x=0;x<m_command_index;x++)
					logerror(" %02x",m_command[x]);
				logerror("\n");
			}
			return;
		}
		if(m_spc_status & SSTS_XFER_IN_PROGRESS)
		{
			m_buffer[m_transfer_index % 512] = data;
			m_spc_status |= SSTS_DREG_EMPTY;  // DREG is empty once sent
			m_transfer_index++;
			m_transfer_count--;
			if(m_transfer_index % 512 == 0)
				write_data(m_buffer, 512);
			if(m_transfer_count == 0)
			{
				// End of transfer
				m_spc_status &= ~SSTS_XFER_IN_PROGRESS;
				m_spc_status |= SSTS_DREG_EMPTY;
				m_ints |= INTS_COMMAND_COMPLETE;
				if(m_int_enable != 0)
					m_irq_cb(1);
				set_phase(SCSI_PHASE_STATUS);
			}
		}
		break;
	case 0x0b:  // TEMP - Temporary
		m_temp = data;
		logerror("mb89352: Write %02x to temporary register\n",data);
		break;
	case 0x0c:  // TCH - Transfer Counter High
		m_transfer_count = (m_transfer_count & 0x0000ffff) | (data << 16);
		if(m_transfer_count == 0)
			m_spc_status |= SSTS_TC_ZERO;
		else
			m_spc_status &= ~SSTS_TC_ZERO;
		logerror("mb89352: TCH: Write %02x [%06x]\n",data,m_transfer_count);
		break;
	case 0x0d:  // TCM - Transfer Counter Mid
		m_transfer_count = (m_transfer_count & 0x00ff00ff) | (data << 8);
		if(m_transfer_count == 0)
			m_spc_status |= SSTS_TC_ZERO;
		else
			m_spc_status &= ~SSTS_TC_ZERO;
		logerror("mb89352: TCM: Write %02x [%06x]\n",data,m_transfer_count);
		break;
	case 0x0e:  // TCL - Transfer Counter Low
		m_transfer_count = (m_transfer_count & 0x00ffff00) | data;
		if(m_transfer_count == 0)
			m_spc_status |= SSTS_TC_ZERO;
		else
			m_spc_status &= ~SSTS_TC_ZERO;
		logerror("mb89352: TCL: Write %02x [%06x]\n",data,m_transfer_count);
		break;
	default:
		logerror("mb89352: write %02x to register %02x\n",data,offset & 0x0f);
	}
}
