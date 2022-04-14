// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
 * ncr539x.c
 *
 * NCR 53(CF)94/53(CF)96 SCSI controller
 * Includes enhanced features of the AMD 53CF94/96 and compatibles
 *
 * All new emulation in 2011 by R. Belmont.
 *
 */

#include "emu.h"
#include "ncr539x.h"

#define LOG_GENERAL (1U << 0)
#define LOG_READS   (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_READS)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGREADS(...) LOGMASKED(LOG_READS, __VA_ARGS__)

enum
{
	TIMER_539X_COMMAND,

	TIMER_539X_END
};

#define MAIN_STATUS_INTERRUPT       0x80
#define MAIN_STATUS_ILLEGAL_OPER    0x40
#define MAIN_STATUS_PARITY_ERROR    0x20
#define MAIN_STATUS_COUNT_TO_ZERO   0x10
#define MAIN_STATUS_GROUP_VALID     0x08
#define MAIN_STATUS_MESSAGE         0x04
#define MAIN_STATUS_CMD_DATA        0x02
#define MAIN_STATUS_IO              0x01

#define IRQ_STATUS_RESET            0x80
#define IRQ_STATUS_INVALID_COMMAND  0x40
#define IRQ_STATUS_DISCONNECTED     0x20
#define IRQ_STATUS_SERVICE_REQUEST  0x10
#define IRQ_STATUS_SUCCESS          0x08
#define IRQ_STATUS_RESELECTED       0x04            // we were reselected as a target
#define IRQ_STATUS_SELECTED_WITH_ATN    0x02        // we were selected as a target with ATN steps
#define IRQ_STATUS_SELECTED         0x01            // we were selected as a target

#define CR2_ALIGN_ENABLE            0x80
#define CR2_FEATURES_ENABLE         0x40
#define CR2_BYTE_ORDER              0x20
#define CR2_TRISTATE_DMA            0x10
#define CR2_SCSI2_ENABLE            0x08
#define CR2_ABORT_ON_PARITY_ERROR   0x04
#define CR2_GENERATE_REGISTER_PARITY    0x02
#define CR2_GENERATE_DATA_PARITY    0x01

static char const *const rdregs[16] = {
	"Transfer count LSB",   // 0
	"Transfer count MSB",   // 1
	"FIFO",                 // 2
	"Command",              // 3
	"Status",               // 4
	"Interrupt Status",     // 5
	"Internal State",       // 6
	"Current FIFO/Internal State",  // 7
	"Control Register 1",   // 8
	"0x9",
	"0xA",
	"Control Register 2",
	"Control Register 3",
	"Control Register 4",
	"Transfer count HSB/Chip ID",
	"0xF"
};

static char const *const wrregs[16] = {
	"Start Transfer count LSB",
	"Start Transfer count MSB",
	"FIFO",
	"Command",
	"SCSI Destination ID",
	"SCSI Timeout",
	"Synchronous Transfer Period",
	"Synchronous Offset",
	"Control Register 1",
	"Clock Factor",
	"Forced Test Mode",
	"Control Register 2",
	"Control Register 3",
	"Control Register 4",
	"Start Transfer count HSB",
	"Data Alignment"
};

// get the length of a SCSI command based on its command byte type
static int get_cmd_len(int cbyte)
{
	int group;

	group = (cbyte>>5) & 7;

	if (group == 0) return 6;
	if (group == 1 || group == 2) return 10;
	if (group == 5) return 12;

	return 6;
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(NCR539X, ncr539x_device, "ncr539x", "NCR/AMD 5394/5396 SCSI")

//-------------------------------------------------
//  ncr539x_device - constructor/destructor
//-------------------------------------------------

ncr539x_device::ncr539x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	legacy_scsi_host_adapter(mconfig, NCR539X, tag, owner, clock),
	m_out_irq_cb(*this),
	m_out_drq_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ncr539x_device::device_start()
{
	legacy_scsi_host_adapter::device_start();

	// resolve line callbacks
	m_out_irq_cb.resolve_safe();
	m_out_drq_cb.resolve_safe();

	m_operation_timer = timer_alloc(0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ncr539x_device::device_reset()
{
	m_fifo_ptr = 0;
	m_fifo_read_ptr = 0;
	m_irq_status = 0;
	m_status = SCSI_PHASE_STATUS;
	m_internal_state = 0;
	m_buffer_offset = 512;
	m_buffer_remaining = 0;
	m_dma_size = 0;
	m_xfer_count = 0;
	m_total_data = 0;
	m_selected = false;
	m_control1 = m_control2 = m_control3 = m_control4 = 0;
	m_chipid_available = false;
	m_chipid_lock = false;
	m_fifo_internal_state = 0;

	m_out_irq_cb(CLEAR_LINE);
	m_out_drq_cb(CLEAR_LINE);
}

void ncr539x_device::dma_read_data(int bytes, uint8_t *pData)
{
	read_data(pData, bytes);
}


void ncr539x_device::dma_write_data(int bytes, uint8_t *pData)
{
	write_data(pData, bytes);
}

void ncr539x_device::device_timer(emu_timer &timer, device_timer_id tid, int param)
{
	//printf("539X: device_timer expired, param = %d, m_command = %02x\n", param, m_command);

	switch (param)
	{
		case TIMER_539X_COMMAND:
			// if this is a DMA command, raise DRQ now
			if (m_command & 0x80)
			{
				m_out_drq_cb(ASSERT_LINE);
			}

			switch (m_command & 0x7f)
			{
				case 0x41:  // select without ATN steps
					if (select(m_last_id))
					{
						m_irq_status |= IRQ_STATUS_SERVICE_REQUEST | IRQ_STATUS_SUCCESS;
						// we should now be in the command phase
						m_status &= ~7; // clear bus phases
						m_status |= MAIN_STATUS_INTERRUPT | SCSI_PHASE_COMMAND;
						m_fifo_ptr = 0;
						m_selected = true;

						LOG("Selecting w/o ATN, irq_status = %02x, status = %02x!\n", m_irq_status, m_status);

						// if DMA is not enabled, there should already be a command loaded into the FIFO
						if (!(m_command & 0x80))
						{
							exec_fifo();
						}
						update_fifo_internal_state(0);
					}
					else
					{
						LOG("Select failed, no device @ ID %d!\n", m_last_id);
						m_status |= MAIN_STATUS_INTERRUPT;
						m_irq_status |= IRQ_STATUS_DISCONNECTED;
					}
					m_out_irq_cb(ASSERT_LINE);
					break;

				case 0x42:  // Select with ATN steps
					if (select(m_last_id))
					{
						m_irq_status |= IRQ_STATUS_SERVICE_REQUEST | IRQ_STATUS_SUCCESS;
						// we should now be in the command phase
						m_status &= ~7; // clear bus phases
						m_status |= MAIN_STATUS_INTERRUPT | SCSI_PHASE_COMMAND;
						m_fifo_ptr = 0;
						m_selected = true;
						LOG("Selecting with ATN, irq_status = %02x, status = %02x!\n", m_irq_status, m_status);

						// if DMA is not enabled, there should already be a command loaded into the FIFO
						if (!(m_command & 0x80))
						{
							exec_fifo();
						}
						update_fifo_internal_state(0);
					}
					else
					{
						LOG("Select failed, no device @ ID %d!\n", m_last_id);
						m_status |= MAIN_STATUS_INTERRUPT;
						m_irq_status |= IRQ_STATUS_DISCONNECTED;
					}
					m_out_irq_cb(ASSERT_LINE);
					break;

				case 0x11:  // initiator command complete
					LOG("Initiator command complete\n");
					m_irq_status = IRQ_STATUS_SERVICE_REQUEST;
					m_status &= ~7; // clear phase bits
					m_status |= MAIN_STATUS_INTERRUPT | SCSI_PHASE_DATAIN;  // go to data in phase (?)
					m_out_irq_cb(ASSERT_LINE);

					// this puts status and message bytes into the FIFO (todo: what are these?)
					m_fifo_ptr = 0;
					m_xfer_count = 2;
					m_buffer_remaining = m_total_data = 0;
					m_fifo[0] = 0;  // status byte
					m_fifo[1] = 0;  // message byte
					m_selected = false;
					update_fifo_internal_state(2);
					break;

				case 0x12:  // message accepted
					LOG("Message accepted\n");
					m_irq_status = IRQ_STATUS_SERVICE_REQUEST;
					m_status |= MAIN_STATUS_INTERRUPT;
					m_out_irq_cb(ASSERT_LINE);
					break;

				default:
					fatalerror("539x: Unhandled command %02x\n", m_command);
			}
			break;

		default:
			break;
	}
}

uint8_t ncr539x_device::read(offs_t offset)
{
	uint8_t rv = 0;

	LOGREADS("539x: Read @ %s (%02x) (%s) (status %02x irq_status %02x)\n", rdregs[offset], offset, machine().describe_context(), m_status, m_irq_status);

	switch (offset)
	{
		case 0:
			rv = m_xfer_count & 0xff;
			break;

		case 1:
			rv = (m_xfer_count>>8) & 0xff;
			break;

		case 2: // FIFO
			{
				uint8_t fifo_bytes = m_fifo_internal_state & 0x1f;

				if (!fifo_bytes)
				{
					rv = 0;
				}
				else
				{
					rv = m_fifo[m_fifo_read_ptr++];
					m_fifo_read_ptr &= (m_fifo_size-1);

					fifo_bytes--;
					m_xfer_count--;
					update_fifo_internal_state(fifo_bytes);

					LOG("Read %02x from FIFO[%d], FIFO now contains %d bytes (%s) (m_buffer_remaining %x)\n", rv, m_fifo_read_ptr-1, fifo_bytes, machine().describe_context(), m_buffer_remaining);

					if (fifo_bytes == 0)
					{
						// the last transfer command has more data for us
						if (m_xfer_count > 0)
						{
							int fifo_fill_size = m_fifo_size;
							if (m_xfer_count < fifo_fill_size)
							{
								fifo_fill_size = m_xfer_count;
							}
							assert(m_buffer_offset < m_buffer_size);
							assert((m_buffer_offset + fifo_fill_size) <= m_buffer_size);
							memcpy(m_fifo, &m_buffer[m_buffer_offset], fifo_fill_size);
							m_buffer_offset += fifo_fill_size;
							m_buffer_remaining -= fifo_fill_size;
							m_fifo_ptr = 0;
							update_fifo_internal_state(fifo_fill_size);
							LOG("Refreshing FIFO (%x remaining from transfer, %x in buffer, %x in total)\n", m_xfer_count, m_buffer_remaining, m_total_data);
						}
						else
						{
							LOG("FIFO empty, asserting service request (buffer_remaining %x)\n", m_buffer_remaining);
							m_irq_status = IRQ_STATUS_SERVICE_REQUEST;
							m_status &= 0x7;    // clear everything but the phase bits
							m_status |= MAIN_STATUS_INTERRUPT | MAIN_STATUS_COUNT_TO_ZERO;
							m_out_irq_cb(ASSERT_LINE);

							// if no data at all, drop the phase
							if ((m_buffer_remaining + m_total_data) == 0)
							{
								LOG("Out of data, setting phase STATUS\n");
								m_status &= ~0x7;
								m_status |= SCSI_PHASE_STATUS;
							}
						}
					}
				}
			}
			break;

		case 3:
			rv = m_command;
			break;

		case 4:
			rv = m_status;
			break;

		case 5:
			rv = m_irq_status;
			// clear the interrupt state
			m_status &= ~MAIN_STATUS_INTERRUPT;
			m_out_irq_cb(CLEAR_LINE);
			break;

		case 6:
			rv = m_internal_state;
			break;

		case 7:
			rv = m_fifo_internal_state;
			break;

		case 8:
			rv = m_control1;
			break;

		case 0xb:
			rv = m_control2;
			break;

		case 0xc:
			rv = m_control3;
			break;

		case 0xd:
			rv = m_control4;
			break;

		case 0xe:
			if (m_control2 & CR2_FEATURES_ENABLE)
			{
				if (m_chipid_available)
				{
					rv = 0xa2;  // 0x12 for CF94, 0xa2 for CF96
				}
				else
				{
					rv = (m_xfer_count>>16) & 0xff;
				}
			}
			break;

	}
	return rv;
}

void ncr539x_device::write(offs_t offset, uint8_t data)
{
	//if (offset != 2)
		LOG("539x: Write %02x @ %s (%02x) (%s)\n", data, wrregs[offset], offset, machine().describe_context());

	switch (offset)
	{
		case 0:
			m_dma_size &= 0xff00;
			m_dma_size |= data;
			break;

		case 1:
			m_dma_size &= 0x00ff;
			m_dma_size |= (data<<8);
			break;

		case 2: // FIFO
			fifo_write(data);
			break;

		case 3:
			m_command = data;

			// clear status bits (OK to do here?)
			m_status &= ~MAIN_STATUS_INTERRUPT;
			m_irq_status = 0;

			switch (data & 0x7f)
			{
				case 0x00:  // NOP
					m_irq_status = IRQ_STATUS_SUCCESS;
					m_status |= MAIN_STATUS_INTERRUPT;
					m_out_irq_cb(ASSERT_LINE);

					// DMA NOP?  allow chip ID
					if ((m_command == 0x80) && (!m_chipid_lock))
					{
						m_chipid_available = true;
					}
					break;

				case 0x01:  // Clear FIFO (must not change buffer state)
					m_fifo_ptr = 0;
					update_fifo_internal_state(0);
					m_irq_status = IRQ_STATUS_SUCCESS;
					m_status |= MAIN_STATUS_INTERRUPT;
					m_out_irq_cb(ASSERT_LINE);
					break;

				case 0x02:  // Reset device
					device_reset();

					m_irq_status = IRQ_STATUS_SUCCESS;
					m_status |= MAIN_STATUS_INTERRUPT;
					m_out_irq_cb(ASSERT_LINE);
					break;

				case 0x03:  // Reset SCSI bus
					m_status = 0;
					m_irq_status = IRQ_STATUS_SUCCESS;
					m_status |= MAIN_STATUS_INTERRUPT;
					m_out_irq_cb(ASSERT_LINE);
					break;

				case 0x10:  // information transfer (must happen immediately)
					m_status &= 0x7;    // clear everything but the phase bits
					m_status |= MAIN_STATUS_INTERRUPT;
					m_irq_status = IRQ_STATUS_SUCCESS;

					int phase;
					phase = get_phase();

					LOG("Information transfer: phase %d buffer remaining %x\n", phase, m_buffer_remaining);

					if (phase == SCSI_PHASE_DATAIN) // target -> initiator transfer
					{
						int amtToGet = m_buffer_size;

						// fill the internal sector buffer
						if (m_buffer_remaining <= 0)
						{
							if (m_total_data < m_buffer_size)
							{
								amtToGet = m_total_data;
							}

							LOG("amtToGet = %x\n", amtToGet);

							if (amtToGet > 0)
							{
								read_data(m_buffer, amtToGet);

								m_total_data -= amtToGet;
								m_buffer_offset = 0;
								m_buffer_remaining = amtToGet;
							}
						}

						// copy the requested amount into the FIFO
						if (amtToGet > 0)
						{
							if (m_buffer_remaining < m_dma_size)
							{
								m_dma_size = m_buffer_remaining;
							}

							int fifo_fill_size = m_fifo_size;

							if (m_dma_size < fifo_fill_size)
							{
								fifo_fill_size = m_dma_size;
							}

							LOG("filling FIFO from buffer[%x] for %x bytes\n", m_buffer_offset, fifo_fill_size);

							memcpy(m_fifo, &m_buffer[m_buffer_offset], fifo_fill_size);
							m_buffer_offset += fifo_fill_size;
							m_buffer_remaining -= fifo_fill_size;

							m_xfer_count = m_dma_size;
							m_fifo_ptr = 0;
							update_fifo_internal_state(fifo_fill_size);
							m_out_drq_cb(ASSERT_LINE);
						}

						m_status |= MAIN_STATUS_COUNT_TO_ZERO;

						LOG("Information transfer: put %02x bytes into FIFO (dma size %x) (buffer remaining %x)\n", m_fifo_internal_state & 0x1f, m_dma_size, m_buffer_remaining);
					}
					else if (phase == SCSI_PHASE_DATAOUT)
					{
						m_xfer_count = m_dma_size;
						if (m_xfer_count == 0)
						{
							m_xfer_count = 0x10000;
						}
						LOG("dma_size %x, xfer_count %x\n", m_dma_size, m_xfer_count);
						m_status &= ~MAIN_STATUS_COUNT_TO_ZERO;
						m_fifo_ptr = 0;
						m_buffer_offset = 0;
						m_buffer_remaining = 0;
					}
					m_out_irq_cb(ASSERT_LINE);
					break;

				case 0x24:  // Terminate steps
					LOG("Terminate steps\n");
					m_irq_status = IRQ_STATUS_SUCCESS | IRQ_STATUS_DISCONNECTED;
					m_status |= MAIN_STATUS_INTERRUPT;
					m_out_irq_cb(ASSERT_LINE);
					m_fifo_ptr = 0;
					update_fifo_internal_state(0);
					break;

				case 0x27:  // Disconnect
					LOG("Disconnect\n");
					m_irq_status = IRQ_STATUS_SUCCESS;
					m_status |= MAIN_STATUS_INTERRUPT;
					m_out_irq_cb(ASSERT_LINE);
					break;

				case 0x44:  // Enable selection/reselection
					LOG("Enable selection/reselection\n");
					m_irq_status = IRQ_STATUS_SUCCESS;
					m_status |= MAIN_STATUS_INTERRUPT;
					m_out_irq_cb(ASSERT_LINE);
					break;

				case 0x47:  // Reselect with ATN3 steps
					if (select(m_last_id))
					{
						m_irq_status |= IRQ_STATUS_SERVICE_REQUEST | IRQ_STATUS_SUCCESS;
						// we should now be in the command phase
						m_status &= ~7; // clear bus phases
						m_status |= MAIN_STATUS_INTERRUPT | SCSI_PHASE_COMMAND;
						m_fifo_ptr = 0;
						m_selected = true;
						LOG("Reselecting with ATN3, irq_status = %02x, status = %02x!\n", m_irq_status, m_status);

						// if DMA is not enabled, there should already be a command loaded into the FIFO
						if (!(m_command & 0x80))
						{
							exec_fifo();
						}
						update_fifo_internal_state(0);
					}
					else
					{
						LOG("Reselect with ATN3 failed, no device @ ID %d!\n", m_last_id);
						m_status |= MAIN_STATUS_INTERRUPT;
						m_irq_status |= IRQ_STATUS_DISCONNECTED;
					}
					m_out_irq_cb(ASSERT_LINE);
					break;

				default:    // other commands are not instantaneous
					LOG("Setting timer for command %02x\n", data);
					// 1x commands happen much faster
					if ((m_command & 0x70) == 0x10)
					{
						m_operation_timer->adjust(attotime::from_hz(65536), TIMER_539X_COMMAND);
					}
					else
					{
						m_operation_timer->adjust(attotime::from_hz(16384), TIMER_539X_COMMAND);
					}
					break;
			}
			break;

		case 4:
			m_last_id = data;
			break;

		case 5:
			m_timeout = data;
			break;

		case 6:
			m_sync_xfer_period = data;
			break;

		case 7:
			m_sync_offset = data;
			break;

		case 8:
			m_control1 = data;
			break;

		case 9:
			m_clock_factor = data;
			break;

		case 0xa:
			m_forced_test = data;
			break;

		case 0xb:
			m_control2 = data;
			break;

		case 0xc:
			m_control3 = data;
			break;

		case 0xd:
			m_control4 = data;
			break;

		case 0xe:
			if (m_control2 & CR2_FEATURES_ENABLE)
			{
				m_dma_size &= 0xffff;
				m_dma_size |= (data<<16);
				m_chipid_available = false;
				m_chipid_lock = true;
			}
			break;

		case 0xf:
			m_data_alignment = data;
			break;
	}
}

void ncr539x_device::exec_fifo()
{
	int length, phase;

	send_command(&m_fifo[0], 12);
	length = get_length();
	phase = get_phase();

	LOG("Command executed (id %d), new phase %d, length %x\n", m_last_id, phase, length);

	m_buffer_offset = m_buffer_size;
	m_buffer_remaining = 0;
	m_total_data = length;

	m_status &= ~7; // clear bus phases
	m_status |= (phase & 7);    // set the phase reported by the device
}

void ncr539x_device::check_fifo_executable()
{
	if (get_cmd_len(m_fifo[0]) == m_fifo_ptr)
	{
		exec_fifo();
	}
}

void ncr539x_device::fifo_write(uint8_t data)
{
	int phase = (m_status & 7);

	if (phase != SCSI_PHASE_DATAOUT)
	{
		LOG("539x: Write %02x @ FIFO[%x]\n", data, m_fifo_ptr);
		m_fifo[m_fifo_ptr++] = data;
		update_fifo_internal_state(m_fifo_ptr);

		if (m_selected)
		{
			check_fifo_executable();
		}
	}
	else    // phase is DATAOUT
	{
		m_buffer[m_buffer_offset++] = data;
		m_xfer_count--;
		m_total_data--;
		LOG("539x: Write %02x @ buffer[%x], xfer_count %x, total %x\n", data, m_buffer_offset-1, m_xfer_count, m_total_data);

		// default to flushing our entire buffer
		int flush_size = m_buffer_size;

		// if the actual size is less than the buffer size, flush that instead
		if (m_dma_size < m_buffer_size)
		{
			flush_size = m_dma_size;
		}

		if ((m_buffer_offset == flush_size) || (m_xfer_count == 0))
		{
			LOG("Flushing buffer to device, %x bytes left in buffer (%x total)\n", m_xfer_count, m_total_data);
			write_data(m_buffer, flush_size);
			m_buffer_offset = 0;

			// need a service request here too
			m_irq_status = IRQ_STATUS_SERVICE_REQUEST;
			m_status &= 7;
			m_status |= MAIN_STATUS_INTERRUPT;
			m_out_irq_cb(ASSERT_LINE);
		}

		if ((m_xfer_count == 0) && (m_total_data == 0))
		{
			LOG("End of write, asserting service request\n");

			m_buffer_offset = 0;
			m_irq_status = IRQ_STATUS_SERVICE_REQUEST;
			m_status = MAIN_STATUS_INTERRUPT | SCSI_PHASE_STATUS;
			m_out_irq_cb(ASSERT_LINE);
		}
	}
}

void ncr539x_device::update_fifo_internal_state(int bytes)
{
	if (bytes >= 0x1f)
	{
		m_fifo_internal_state |= 0x1f;
	}
	else
	{
		m_fifo_internal_state &= ~0x1f;
		m_fifo_internal_state |= (bytes & 0x1f);
	}
}
