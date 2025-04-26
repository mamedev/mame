// license:BSD-3-Clause
// copyright-holders:Tyson Smith
/*
 * wd33c9x.c
 *
 * This is an NSCSI device implementation
 * of the Western Digital/AMD 33C9x SCSI
 * controllers used in many early
 * Silicon Graphics workstations.
 *
 * There are *many* things left to do, but
 * this starting point boots many low-level
 * programs from an NSCSI CDROM, on an
 * SGI driver.
 *
 */

#include "emu.h"
#include "wd33c9x.h"

#define LOG_READS       (1U << 1)
#define LOG_WRITES      (1U << 2)
#define LOG_COMMANDS    (1U << 3)
#define LOG_ERRORS      (1U << 4)
#define LOG_MISC        (1U << 5)
#define LOG_LINES       (1U << 6)
#define LOG_STATE       (1U << 7)
#define LOG_STEP        (1U << 8)
#define LOG_REGS        (LOG_READS | LOG_WRITES)
#define LOG_ALL         (LOG_REGS | LOG_COMMANDS | LOG_ERRORS | LOG_MISC | LOG_LINES | LOG_STATE | LOG_STEP)

#define VERBOSE         (0)
#include "logmacro.h"

enum register_addresses_e : uint8_t {
	OWN_ID               = 0x00, // Own ID Register                /CDB Size
	CONTROL              = 0x01, // Control Register
	TIMEOUT_PERIOD       = 0x02, // Timeout Period Register
	CDB_1                = 0x03, // Total Sectors Register         /CDB 1st
	CDB_2                = 0x04, // Total Headers Register         /CDB 2nd
	CDB_3                = 0x05, // Total Cylinders Register (MSB) /CDB 3rd
	CDB_4                = 0x06, // Total Cylinders Register (LSB) /CDB 4th
	CDB_5                = 0x07, // Logical Address (MSB)          /CDB 5th
	CDB_6                = 0x08, // Logical Address (2nd)          /CDB 6th
	CDB_7                = 0x09, // Logical Address (3rd)          /CDB 7th
	CDB_8                = 0x0a, // Logical Address (LSB)          /CDB 8th
	CDB_9                = 0x0b, // Sector Number Register         /CDB 9th
	CDB_10               = 0x0c, // Head Number Register           /CDB 10th
	CDB_11               = 0x0d, // Cylinder Number Register (MSB) /CDB 11th
	CDB_12               = 0x0e, // Cylinder Number Register (LSB) /CDB 12th
	TARGET_LUN           = 0x0f, // Target LUN Register
	COMMAND_PHASE        = 0x10, // Command Phase Register
	SYNCHRONOUS_TRANSFER = 0x11, // Synchronous Transfer Register
	TRANSFER_COUNT_MSB   = 0x12, // Transfer Count Register (MSB)
	TRANSFER_COUNT       = 0x13, // Transfer Count Register (2nd Byte)
	TRANSFER_COUNT_LSB   = 0x14, // Transfer Count Register (LSB)
	DESTINATION_ID       = 0x15, // Destination ID Register
	SOURCE_ID            = 0x16, // Source ID Register
	SCSI_STATUS          = 0x17, // SCSI Status Register
	COMMAND              = 0x18, // Command Register
	DATA                 = 0x19, // Data Register
	QUEUE_TAG            = 0x1a, // Queue Tag Register (33C93B only)
	INVALID_1B           = 0x1b,
	INVALID_1C           = 0x1c,
	INVALID_1D           = 0x1d,
	INVALID_1E           = 0x1e,
	AUXILIARY_STATUS     = 0x1f  // Auxiliary Status Register
};

// Own ID Register (0x00) fields and values
enum own_id_e : uint8_t {
	// Reset Command
	OWN_ID_SCSI_ID  = 0x07, // SCSI ID Bits
	OWN_ID_EAF      = 0x08, // Enable Advanced Features (33C93A and 33C93B only)
	OWN_ID_EHP      = 0x10, // Enable Host Parity (33C93A and 33C93B only)
	OWN_ID_RAF      = 0x20, // Really Advanced Features (33C93B only)
	OWN_ID_FS       = 0xc0, // Frequency Select (33C93A and 33C93B only)
	OWN_ID_FS_2     = 0x00, //   8-10MHz  -> clock divisor = 2
	OWN_ID_FS_3     = 0x40, //   12-15MHz -> clock divisor = 3
	OWN_ID_FS_4     = 0x80, //   16-20MHz -> clock divisor = 4

	// For other commands (when Advanced Features are enabled)
	OWN_ID_CDB_SIZE = 0x0f  // SCSI CDB Size
};

// Control Register (0x01) fields and values
enum control_e : uint8_t {
	CONTROL_HSP       = 0x01, // Halt on SCSI Parity Error
	CONTROL_HA        = 0x02, // Halt on Attention
	CONTROL_IDI       = 0x04, // Intermediate Disconnect (33C93A and 33C93B only)
	CONTROL_EDI       = 0x08, // Ending Disconnect Interrupt (33C93A and 33C93B only)
	CONTROL_HHP       = 0x10, // Halt on Host Parity Error (33C93A and 33C93B only)
	CONTROL_DM        = 0xe0, // DMA Mode Select
	CONTROL_DM_POLLED = 0x00, //   Polled I/O Mode or no DMA enabled
	CONTROL_DM_BURST  = 0x20, //   Burst Mode or demand-mode DMA (33C93A and 33C93B only)
	CONTROL_DM_BUS    = 0x40, //   WD-Bus Mode or Direct Buffer Access (DBA) mode
	CONTROL_DM_DMA    = 0x80, //   DMA MOOE or Single-byte DMA
};

// Target LUN Register (0x0f) fields and values
enum target_lun_e : uint8_t {
	TARGET_LUN_TL  = 0x07, // Target LUN
	TARGET_LUN_TRN = 0x20, // Target Routine Number
	TARGET_LUN_DOK = 0x40, // Disconnects OK
	TARGET_LUN_TLV = 0x80, // Target LUN Valid
};

// Command Phase Register (0x10) fields and values
enum command_phase_e : uint8_t {
	COMMAND_PHASE_ZERO                     = 0x00,
	COMMAND_PHASE_SELECTED                 = 0x10,
	COMMAND_PHASE_IDENTIFY_MESSAGE         = 0x20,
	COMMAND_PHASE_TAG_MESSAGE              = 0x21,
	COMMAND_PHASE_QUEUE_TAG                = 0x22,
	COMMAND_PHASE_CP_BYTES_0               = 0x30,
	COMMAND_PHASE_CP_BYTES_1               = 0x31,
	COMMAND_PHASE_CP_BYTES_2               = 0x32,
	COMMAND_PHASE_CP_BYTES_3               = 0x33,
	COMMAND_PHASE_CP_BYTES_4               = 0x34,
	COMMAND_PHASE_CP_BYTES_5               = 0x35,
	COMMAND_PHASE_CP_BYTES_6               = 0x36,
	COMMAND_PHASE_CP_BYTES_7               = 0x37,
	COMMAND_PHASE_CP_BYTES_8               = 0x38,
	COMMAND_PHASE_CP_BYTES_9               = 0x39,
	COMMAND_PHASE_CP_BYTES_A               = 0x3a,
	COMMAND_PHASE_CP_BYTES_B               = 0x3b,
	COMMAND_PHASE_CP_BYTES_C               = 0x3c,
	COMMAND_PHASE_SAVE_DATA_POINTER        = 0x41,
	COMMAND_PHASE_DISCONNECT_MESSAGE       = 0x42,
	COMMAND_PHASE_DISCONNECTED             = 0x43,
	COMMAND_PHASE_RESELECTED               = 0x44,
	COMMAND_PHASE_IDENTIFY_MATCH           = 0x45,
	COMMAND_PHASE_TRANSFER_COUNT           = 0x46,
	COMMAND_PHASE_RECEIVE_STATUS           = 0x47,
	COMMAND_PHASE_STATUS_RECEIVED          = 0x50,
	COMMAND_PHASE_COMMAND_COMPLETE         = 0x60,
	COMMAND_PHASE_LINKED_COMMAND_COMPLETE  = 0x61,
	COMMAND_PHASE_TARGET_LUN               = 0x70,
	COMMAND_PHASE_SIMPLE_QUEUE_TAG_MESSAGE = 0x71,
};

// Destination ID Register (0x15) fields and values
enum destination_id_e : uint8_t {
	DESTINATION_ID_DI     = 0x07, // Destination ID
	DESTINATION_ID_TG     = 0x18, // Tag Message
	DESTINATION_ID_TG_NM  = 0x00, //   No Message
	DESTINATION_ID_TG_SQT = 0x08, //   Simple Queue Tag
	DESTINATION_ID_TG_HQT = 0x10, //   Head of Queue Tag
	DESTINATION_ID_TG_OQT = 0x18, //   Ordered Queue Tag
	DESTINATION_ID_DF     = 0x20, // Disable Feature
	DESTINATION_ID_DPD    = 0x40, // Data Phase Direction
	DESTINATION_ID_SCC    = 0x80, // Select Command Chain
};

// Source ID Register (0x16) fields and values
enum source_id_e : uint8_t {
	SOURCE_ID_SI  = 0x07, // Source ID
	SOURCE_ID_SIV = 0x08, // Source ID Valid
	SOURCE_ID_DSP = 0x20, // Disable Select Parity
	SOURCE_ID_ES  = 0x40, // Enable Selection
	SOURCE_ID_ER  = 0x80, // Enable Reselection
};

// SCSI Status Register (0x17) fields and values
enum scsi_status_e : uint8_t {
	SCSI_STATUS_RESET                                 = 0x00,
	SCSI_STATUS_RESET_EAF                             = 0x01,

	SCSI_STATUS_RESELECT_SUCCESS                      = 0x10,
	SCSI_STATUS_SELECT_SUCCESS                        = 0x11,
	SCSI_STATUS_COMMAND_SUCCESS                       = 0x13,
	SCSI_STATUS_COMMAND_ATN_SUCCESS                   = 0x14,
	SCSI_STATUS_TRANSLATE_SUCCESS                     = 0x15,
	SCSI_STATUS_SELECT_TRANSFER_SUCCESS               = 0x16,
	SCSI_STATUS_TRANSFER_SUCCESS                      = 0x18,

	SCSI_STATUS_TRANSFER_INFO_MSG_IN                  = 0x20,
	SCSI_STATUS_SAVE_DATA_POINTERS                    = 0x21,
	SCSI_STATUS_SELECTION_ABORTED                     = 0x22,
	SCSI_STATUS_RECEIVE_SEND_ABORTED                  = 0x23,
	SCSI_STATUS_RECEIVE_SEND_ABORTED_ATN              = 0x24,
	SCSI_STATUS_ABORT_DURING_SELECTION                = 0x25,
	SCSI_STATUS_RESELECTED_DURING_SELECT_AND_TRANSFER = 0x27,
	SCSI_STATUS_TRANSFER_ABORTED                      = 0x28,

	SCSI_STATUS_INVALID_COMMAND                       = 0x40,
	SCSI_STATUS_UNEXPECTED_DISCONNECT                 = 0x41,
	SCSI_STATUS_SELECTION_TIMEOUT                     = 0x42,
	SCSI_STATUS_PARITY_ERROR                          = 0x43,
	SCSI_STATUS_PARITY_ERROR_ATN                      = 0x44,
	SCSI_STATUS_LOGICAL_ADDRESS_TOO_LARGE             = 0x45,
	SCSI_STATUS_RESELECTION_MISMATCH                  = 0x46,
	SCSI_STATUS_INCORRECT_STATUS_BYTE                 = 0x47,
	SCSI_STATUS_UNEXPECTED_PHASE                      = 0x48,

	SCSI_STATUS_RESELECTED                            = 0x80,
	SCSI_STATUS_RESELECTED_EAF                        = 0x81,
	SCSI_STATUS_SELECTED                              = 0x82,
	SCSI_STATUS_SELECTED_ATN                          = 0x83,
	SCSI_STATUS_ATN                                   = 0x84,
	SCSI_STATUS_DISCONNECT                            = 0x85,
	SCSI_STATUS_NEED_COMMAND_SIZE                     = 0x87,
	SCSI_STATUS_REQ                                   = 0x88
};

// Command Register (0x18) fields and values
enum command_e : uint8_t {
	COMMAND_CC                              = 0x7f,
	COMMAND_CC_RESET                        = 0x00,
	COMMAND_CC_ABORT                        = 0x01,
	COMMAND_CC_ASSERT_ATN                   = 0x02,
	COMMAND_CC_NEGATE_ACK                   = 0x03,
	COMMAND_CC_DISCONNECT                   = 0x04,
	COMMAND_CC_RESELECT                     = 0x05,
	COMMAND_CC_SELECT_ATN                   = 0x06,
	COMMAND_CC_SELECT                       = 0x07,
	COMMAND_CC_SELECT_ATN_TRANSFER          = 0x08,
	COMMAND_CC_SELECT_TRANSFER              = 0x09,
	COMMAND_CC_RESELECT_RECEIVE_DATA        = 0x0a,
	COMMAND_CC_RESELECT_SEND_DATA           = 0x0b,
	COMMAND_CC_WAIT_SELECT_RECEIVE_DATA     = 0x0c,
	COMMAND_CC_SEND_STATUS_COMMAND_COMPLETE = 0x0d,
	COMMAND_CC_SEND_DISCONNECT_MESSAGE      = 0x0e,
	COMMAND_CC_SET_IDI                      = 0x0f,
	COMMAND_CC_RECEIVE_COMMAND              = 0x10,
	COMMAND_CC_RECEIVE_DATA                 = 0x11,
	COMMAND_CC_RECEIVE_MESSAGE_OUT          = 0x12,
	COMMAND_CC_RECEIVE_UNSPECIFIED_INFO_OUT = 0x13,
	COMMAND_CC_SEND_STATUS                  = 0x14,
	COMMAND_CC_SEND_DATA                    = 0x15,
	COMMAND_CC_SEND_MESSAGE_IN              = 0x16,
	COMMAND_CC_SEND_UNSPECIFIED_INFO_IN     = 0x17,
	COMMAND_CC_TRANSLATE_ADDRESS            = 0x18,
	COMMAND_CC_TRANSFER_PAD                 = 0x19,
	COMMAND_CC_TRANSFER_INFO                = 0x20,
	COMMAND_SBT                             = 0x80,
};

// Auxiliary Status Register (0x1f) fields and values
enum auxiliary_status_e : uint8_t {
	AUXILIARY_STATUS_DBR = 0x01, // Data Buffer Ready
	AUXILIARY_STATUS_PE  = 0x02, // Parity Error
	AUXILIARY_STATUS_FFE = 0x04, // FIFO Full/Empty (33C93B only)
	AUXILIARY_STATUS_CIP = 0x10, // Command In Progress (33C93B only)
	AUXILIARY_STATUS_BSY = 0x20, // Busy
	AUXILIARY_STATUS_LCI = 0x40, // Last Command Ignored
	AUXILIARY_STATUS_INT = 0x80  // Interrupt Pending
};

// SCSI bus connection states (modes)
enum : uint8_t {
	MODE_D, // Disconnected
	MODE_T, // Target
	MODE_I  // Initiator
};

enum : uint16_t {
	IDLE = 1,
	FINISHED,

	// Disconnected state commands
	DISC_SEL_ARBITRATION,

	// Initiator commands
	INIT_MSG_WAIT_REQ,
	INIT_XFR,
	INIT_XFR_SEND_PAD_WAIT_REQ,
	INIT_XFR_SEND_PAD,
	INIT_XFR_RECV_PAD_WAIT_REQ,
	INIT_XFR_RECV_PAD,
	INIT_XFR_RECV_BYTE_ACK,
	INIT_XFR_RECV_BYTE_NACK,
	INIT_XFR_FUNCTION_COMPLETE,
	INIT_XFR_BUS_COMPLETE,
	INIT_XFR_WAIT_REQ,
	INIT_CPT_RECV_BYTE_ACK,
	INIT_CPT_RECV_WAIT_REQ,
	INIT_CPT_RECV_BYTE_NACK
};

const char *const wd33c9x_base_device::state_names[] = {
	"-",
	"IDLE",
	"FINISHED",
	"DISC_SEL_ARBITRATION",
	"INIT_MSG_WAIT_REQ",
	"INIT_XFR",
	"INIT_XFR_SEND_PAD_WAIT_REQ",
	"INIT_XFR_SEND_PAD",
	"INIT_XFR_RECV_PAD_WAIT_REQ",
	"INIT_XFR_RECV_PAD",
	"INIT_XFR_RECV_BYTE_ACK",
	"INIT_XFR_RECV_BYTE_NACK",
	"INIT_XFR_FUNCTION_COMPLETE",
	"INIT_XFR_BUS_COMPLETE",
	"INIT_XFR_WAIT_REQ",
	"INIT_CPT_RECV_BYTE_ACK",
	"INIT_CPT_RECV_WAIT_REQ",
	"INIT_CPT_RECV_BYTE_NACK",
};

enum : uint16_t {
	// Arbitration
	ARB_WAIT_BUS_FREE = 1,
	ARB_CHECK_FREE,
	ARB_EXAMINE_BUS,
	ARB_ASSERT_SEL,
	ARB_SET_DEST,
	ARB_RELEASE_BUSY,
	ARB_TIMEOUT_BUSY,
	ARB_TIMEOUT_ABORT,
	ARB_DESKEW_WAIT,

	// Send/receive byte
	SEND_WAIT_SETTLE,
	SEND_WAIT_REQ_0,
	RECV_WAIT_REQ_1,
	RECV_WAIT_SETTLE,
	RECV_WAIT_REQ_0
};

const char *const wd33c9x_base_device::substate_names[] = {
	"-",
	"ARB_WAIT_BUS_FREE",
	"ARB_CHECK_FREE",
	"ARB_EXAMINE_BUS",
	"ARB_ASSERT_SEL",
	"ARB_SET_DEST",
	"ARB_RELEASE_BUSY",
	"ARB_TIMEOUT_BUSY",
	"ARB_TIMEOUT_ABORT",
	"ARB_DESKEW_WAIT",
	"SEND_WAIT_SETTLE",
	"SEND_WAIT_REQ_0",
	"RECV_WAIT_REQ_1",
	"RECV_WAIT_SETTLE",
	"RECV_WAIT_REQ_0",
};

enum : uint16_t {
	STATE_MASK = 0x00ff,
	SUB_SHIFT  = 8,
	SUB_MASK   = 0xff00
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(WD33C92,  wd33c92_device,  "wd33c92",  "Western Digital WD33C92 SCSI Controller")
DEFINE_DEVICE_TYPE(WD33C93,  wd33c93_device,  "wd33c93",  "Western Digital WD33C93 SCSI Controller")
DEFINE_DEVICE_TYPE(WD33C93A, wd33c93a_device, "wd33c93a", "Western Digital WD33C93A SCSI Controller")
DEFINE_DEVICE_TYPE(WD33C93B, wd33c93b_device, "wd33c93b", "Western Digital WD33C93B SCSI Controller")

//-------------------------------------------------
//  wd33c9x_base_device - constructor/destructor
//-------------------------------------------------

wd33c9x_base_device::wd33c9x_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nscsi_device{ mconfig, type, tag, owner, clock }
	, nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF)
	, m_addr{ 0 }
	, m_regs{ 0 }
	, m_command_length{ 0 }
	, m_last_message{ 0 }
	, m_scsi_state{ IDLE }
	, m_mode{ MODE_D }
	, m_xfr_phase{ 0 }
	, m_transfer_count{ 0 }
	, m_data_fifo{ 0 }
	, m_data_fifo_pos{ 0 }
	, m_data_fifo_size{ 0 }
	, m_irq_fifo{ 0 }
	, m_irq_fifo_pos{ 0 }
	, m_irq_fifo_size{ 0 }
	, m_irq_cb{ *this }
	, m_drq_cb{ *this }
	, m_drq_state{ false }
	, m_timer{ nullptr }
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wd33c9x_base_device::device_start()
{
	m_timer = timer_alloc(FUNC(wd33c9x_base_device::update_step), this);
	save_item(NAME(m_addr));
	save_item(NAME(m_regs));
	save_item(NAME(m_command_length));
	save_item(NAME(m_last_message));
	save_item(NAME(m_mode));
	save_item(NAME(m_scsi_state));
	save_item(NAME(m_xfr_phase));
	save_item(NAME(m_transfer_count));
	save_item(NAME(m_data_fifo));
	save_item(NAME(m_data_fifo_pos));
	save_item(NAME(m_data_fifo_size));
	save_item(NAME(m_irq_fifo));
	save_item(NAME(m_irq_fifo_pos));
	save_item(NAME(m_irq_fifo_size));
	save_item(NAME(m_drq_state));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wd33c9x_base_device::device_reset()
{
	// This is a hardware reset.  Software reset is handled
	// under COMMAND_CC_RESET.
	scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
	scsi_bus->ctrl_wait(scsi_refid, S_SEL|S_BSY|S_RST, S_ALL);
	m_addr = 0;
	for (uint8_t reg = 0; reg < NUM_REGS; ++reg) {
		// FIXME - QUEUE_TAG is a valid register for 93B only
		m_regs[reg] = (QUEUE_TAG <= reg && reg <= INVALID_1E) ? 0xff : 0;
	}
	m_command_length = 0;
	m_last_message = 0;
	set_scsi_state(IDLE);
	m_mode = MODE_D;
	m_xfr_phase = 0;
	m_transfer_count = 0;
	data_fifo_reset();
	irq_fifo_reset();
	m_irq_cb(CLEAR_LINE);
	m_drq_cb(CLEAR_LINE);
	m_drq_state = false;

	// Hardware reset triggers a SCSI_STATUS_RESET interrupt.
	irq_fifo_push(SCSI_STATUS_RESET);
	update_irq();
}


//-------------------------------------------------
//  update_step -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(wd33c9x_base_device::update_step)
{
	step(true);
}


//-------------------------------------------------
//  scsi_ctrl_changed - smart comment
//-------------------------------------------------

void wd33c9x_base_device::scsi_ctrl_changed()
{
	const uint32_t ctrl = scsi_bus->ctrl_r();
	if (ctrl & S_RST) {
		LOG("scsi bus reset\n");
		// FIXME - Do something...
		return;
	}
	step(false);
}


//**************************************************************************
//  MEMORY HANDLERS
//**************************************************************************


//-------------------------------------------------
//  dir_r
//-------------------------------------------------

uint8_t wd33c9x_base_device::dir_r(offs_t offset)
{
	m_addr = offset & REGS_MASK;
	return indir_reg_r();
}


//-------------------------------------------------
//  dir_w
//-------------------------------------------------

void wd33c9x_base_device::dir_w(offs_t offset, uint8_t data)
{
	m_addr = offset & REGS_MASK;
	indir_reg_w(data);
}


//-------------------------------------------------
//  indir_r
//-------------------------------------------------

uint8_t wd33c9x_base_device::indir_r(offs_t offset)
{
	switch (offset) {
	case 0:
		return indir_addr_r();
	case 1:
		return indir_reg_r();
	default:
		LOGMASKED(LOG_READS | LOG_ERRORS, "Read from invalid offset %d\n", offset);
		break;
	}
	return 0;
}


//-------------------------------------------------
//  indir_w
//-------------------------------------------------

void wd33c9x_base_device::indir_w(offs_t offset, uint8_t data)
{
	switch (offset) {
	case 0:
		indir_addr_w(data);
		break;
	case 1:
		indir_reg_w(data);
		break;
	default:
		LOGMASKED(LOG_WRITES | LOG_ERRORS, "Write to invalid offset %d (data=%02x)\n", offset, data);
		break;
	}
}


//-------------------------------------------------
//  indir_addr_r
//-------------------------------------------------

uint8_t wd33c9x_base_device::indir_addr_r()
{
	// Trick to push the interrupt flag after the fifo is empty to help cps3
	return m_regs[AUXILIARY_STATUS] & 0x01 ? m_regs[AUXILIARY_STATUS] & 0x7f : m_regs[AUXILIARY_STATUS];
}


//-------------------------------------------------
//  indir_addr_w
//-------------------------------------------------

void wd33c9x_base_device::indir_addr_w(uint8_t data)
{
	m_addr = data & REGS_MASK;
}


//-------------------------------------------------
//  indir_reg_r
//-------------------------------------------------

uint8_t wd33c9x_base_device::indir_reg_r()
{
	uint8_t ret;
	switch (m_addr) {
	case DATA: {
		if (!(m_regs[AUXILIARY_STATUS] & AUXILIARY_STATUS_DBR)) {
			// The processor, except in one case, should only
			// access the Data Register when the DBR bit in the
			// Auxiliary Status Register is true. The exception
			// occurs when the 33C93B is reselected while operating
			// in advanced mode; the processor must retrieve
			// the Identify message from the target by reading the
			// Data Register.
			fatalerror("%s: The host should never access the data register without DBR set.\n", shortname());
		}
		bool was_full = data_fifo_full();
		ret = data_fifo_pop();
		if (data_fifo_empty())
			m_regs[AUXILIARY_STATUS] &= ~AUXILIARY_STATUS_DBR;
		if (was_full)
			step(false);
		break;
	}

	default:
		if (m_addr == OWN_ID) {
			ret = m_command_length;
		}
		else {
			ret = m_regs[m_addr];
		}

		// Clear IRQ when the SCSI Status Register is read
		if (m_addr == SCSI_STATUS) {
			update_irq();
		}

		// No address increment on accesses to Command, Data, and Auxiliary Status Registers
		if (m_addr != COMMAND && m_addr != AUXILIARY_STATUS) {
			m_addr = (m_addr + 1) & REGS_MASK;
		}
		break;
	}

	return ret;
}


//-------------------------------------------------
//  indir_reg_w
//-------------------------------------------------

void wd33c9x_base_device::indir_reg_w(uint8_t data)
{
	switch (m_addr) {
	case SCSI_STATUS:
	case QUEUE_TAG: // Only for 92/93 and 93A
	case INVALID_1B:
	case INVALID_1C:
	case INVALID_1D:
	case INVALID_1E:
	case AUXILIARY_STATUS:
		LOGMASKED(LOG_WRITES | LOG_ERRORS, "Write to read-only register address %d (data=%02x)\n", m_addr, data);
		break;

	case COMMAND: {
		if (m_regs[AUXILIARY_STATUS] & (AUXILIARY_STATUS_INT | AUXILIARY_STATUS_CIP)) {
			logerror("%s: The host should never write to the command register when INT or CIP are set.\n", shortname());
		}

		const uint8_t cc = (data & COMMAND_CC);
		if (cc == COMMAND_CC_SET_IDI) {
			m_regs[CONTROL] |= CONTROL_IDI;
			break;
		}

		if (cc > COMMAND_CC_DISCONNECT && (m_regs[AUXILIARY_STATUS] & AUXILIARY_STATUS_BSY)) {
			fatalerror("%s: The host should never issue a Level II command when BSY is set.\n", shortname());
		}

		m_regs[COMMAND] = data;
		start_command();
	} break;

	case DATA:
		if (!(m_regs[AUXILIARY_STATUS] & AUXILIARY_STATUS_DBR)) {
			fatalerror("%s: The host should never write the data register without DBR set.\n", shortname());
		}
		m_regs[AUXILIARY_STATUS] &= ~AUXILIARY_STATUS_DBR;
		data_fifo_push(data);
		decrement_transfer_count();
		step(false);
		break;

	default:
		if (m_addr == OWN_ID) {
			m_command_length = data;
		}
		else {
			m_regs[m_addr] = data;
		}
		m_addr = (m_addr + 1) & REGS_MASK;
		break;
	}
}


//-------------------------------------------------
//  reset - Host reset line handler
//-------------------------------------------------

void wd33c9x_base_device::reset_w(int state)
{
	if (state) {
		LOGMASKED(LOG_LINES, "Reset via MR line\n");
		device_reset();
	}
}


//-------------------------------------------------
//  dma_r - DMA read interface
//-------------------------------------------------

uint8_t wd33c9x_base_device::dma_r()
{
	const uint8_t ret = data_fifo_pop();
	decrement_transfer_count();
	clear_drq();
	return ret;
}


//-------------------------------------------------
//  dma_w - DMA write interface
//-------------------------------------------------

void wd33c9x_base_device::dma_w(const uint8_t data)
{
	data_fifo_push(data);
	decrement_transfer_count();
	clear_drq();
}


static const char * select_strings[4] = {
	"Select-w/Atn",
	"Select",
	"Select-w/Atn-and-Transfer",
	"Select-and-Transfer"
};


//-------------------------------------------------
//  start_command
//-------------------------------------------------

void wd33c9x_base_device::start_command()
{
	const uint8_t cc = m_regs[COMMAND] & COMMAND_CC;

	// Command In Progress
	//  The CIP flag being set only means that the WD33C9x is
	//  *interpreting* the contents of the Command Register.
	//  It shouldn't actually be set.
	//m_regs[AUXILIARY_STATUS] |= AUXILIARY_STATUS_CIP;
	if (cc > COMMAND_CC_DISCONNECT && cc != COMMAND_CC_SET_IDI) {
		m_regs[AUXILIARY_STATUS] |= AUXILIARY_STATUS_BSY;
	}

	switch (cc) {
	case COMMAND_CC_RESET:
		LOGMASKED(LOG_COMMANDS, "Reset Command\n");
		scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
		scsi_bus->ctrl_wait(scsi_refid, S_SEL|S_BSY|S_RST, S_ALL);
		m_regs[OWN_ID] = m_command_length;
		memset(&m_regs[CONTROL], 0, SOURCE_ID - CONTROL);
		m_regs[COMMAND] = 0;
		m_regs[AUXILIARY_STATUS] &= ~AUXILIARY_STATUS_DBR;
		m_mode = MODE_D;
		data_fifo_reset();
		irq_fifo_reset();
		update_irq();
		set_scsi_state(FINISHED);
		irq_fifo_push((m_regs[OWN_ID] & OWN_ID_EAF) ? SCSI_STATUS_RESET_EAF : SCSI_STATUS_RESET);
		scsi_id = (m_regs[OWN_ID] & OWN_ID_SCSI_ID);
		step(false);
		break;

	case COMMAND_CC_ABORT:
		LOGMASKED(LOG_COMMANDS, "Abort Command\n");
		set_scsi_state(FINISHED);
		// FIXME
		irq_fifo_push((m_regs[OWN_ID] & OWN_ID_EAF) ? SCSI_STATUS_RESET_EAF : SCSI_STATUS_RESET);
		break;

	case COMMAND_CC_ASSERT_ATN:
		LOGMASKED(LOG_COMMANDS, "Assert ATN Command\n");
		if (m_mode != MODE_I) {
			fatalerror("%s: ASSERT_ATN command only valid in the Initiator state.", shortname());
		}
		scsi_bus->ctrl_w(scsi_refid, S_ATN, S_ATN);
		return;

	case COMMAND_CC_NEGATE_ACK:
		LOGMASKED(LOG_COMMANDS, "Negate ACK Command\n");
		// FIXME - This is causing problems, so ignore for now.
		//if (m_mode != MODE_I) {
		//  fatalerror("NEGATE_ACK command only valid in the Initiator state.");
		//}
		scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		return;

	case COMMAND_CC_DISCONNECT:
		LOGMASKED(LOG_COMMANDS, "Disconnect Command\n");
		scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
		scsi_bus->ctrl_wait(scsi_refid, S_SEL|S_BSY|S_RST, S_ALL);
		m_mode = MODE_D;
		set_scsi_state(IDLE);
		m_regs[AUXILIARY_STATUS] &= ~(AUXILIARY_STATUS_CIP | AUXILIARY_STATUS_BSY);
		break;

	case COMMAND_CC_SELECT:
	case COMMAND_CC_SELECT_ATN:
		LOGMASKED(LOG_COMMANDS, "%s Command\n", select_strings[cc - COMMAND_CC_SELECT_ATN]);
		if (m_mode != MODE_D) {
			fatalerror("Select commands only valid in the Disconnected state.");
		}
		set_scsi_state((ARB_WAIT_BUS_FREE << SUB_SHIFT) | DISC_SEL_ARBITRATION);
		step(false);
		break;

	case COMMAND_CC_SELECT_TRANSFER:
	case COMMAND_CC_SELECT_ATN_TRANSFER:
		LOGMASKED(LOG_COMMANDS, "%s Command\n", select_strings[cc - COMMAND_CC_SELECT_ATN]);
		if (m_mode == MODE_D) {
			set_scsi_state((ARB_WAIT_BUS_FREE << SUB_SHIFT) | DISC_SEL_ARBITRATION);
			m_regs[COMMAND_PHASE] = COMMAND_PHASE_ZERO;
		}
		else if (m_mode == MODE_I) {
			set_scsi_state(INIT_XFR);
		}
		else {
			fatalerror("%s: Select-and-Transfer commands only valid in the Disconnected and Initiator states.", shortname());
		}
		set_command_length(cc);
		load_transfer_count();
		step(false);
		break;

	case COMMAND_CC_TRANSLATE_ADDRESS:
		LOGMASKED(LOG_COMMANDS, "Translate Address Command\n");
		{
			uint8_t total_sectors = m_regs[CDB_1];
			uint8_t total_heads = m_regs[CDB_2];
			uint16_t total_cylinders = m_regs[CDB_3] << 8 | m_regs[CDB_4];
			uint32_t lba = (m_regs[CDB_5] << 24) | (m_regs[CDB_6] << 16) | (m_regs[CDB_7] << 8) | (m_regs[CDB_8] << 0);

			LOGMASKED(LOG_COMMANDS, "total_sectors=%02x, total_heads=%02x, total_cylinders=%04x, lba=%08x\n", total_sectors, total_heads, total_cylinders, lba);

			uint16_t cylinder = lba / (total_sectors * total_heads);
			uint8_t head = (lba - (cylinder * total_sectors * total_heads)) / total_sectors;
			uint8_t sector = (lba - (cylinder * total_sectors * total_heads)) % total_sectors;

			LOGMASKED(LOG_COMMANDS, "-> cylinder=%04x, head=%02x, sector=%02x\n", cylinder, head, sector);

			m_regs[CDB_9] = sector;
			m_regs[CDB_10] = head;
			m_regs[CDB_11] = cylinder >> 8;
			m_regs[CDB_12] = cylinder >> 0;

			m_regs[AUXILIARY_STATUS] &= ~(AUXILIARY_STATUS_CIP | AUXILIARY_STATUS_BSY);

			if (cylinder >= total_cylinders)
				irq_fifo_push(SCSI_STATUS_LOGICAL_ADDRESS_TOO_LARGE);
			else
				irq_fifo_push(SCSI_STATUS_TRANSLATE_SUCCESS);

			update_irq();
		}
		break;

	case COMMAND_CC_TRANSFER_INFO:
		LOGMASKED(LOG_COMMANDS, "Transfer Info Command\n");
		if (m_mode != MODE_I) {
			fatalerror("%s: TRANSFER_INFO command only valid in the Initiator state.", shortname());
		}
		m_regs[AUXILIARY_STATUS] &= ~AUXILIARY_STATUS_DBR;
		set_scsi_state(INIT_XFR);
		set_command_length(COMMAND_CC_TRANSFER_INFO);
		load_transfer_count();
		m_xfr_phase = (scsi_bus->ctrl_r() & S_PHASE_MASK);
		step(false);
		return;

	default:
		fatalerror("%s: Unimplemented command: 0x%02x", shortname(), cc);
		break;
	}
}


static const char * phase_strings[8] = {
	"DATA_OUT",
	"DATA_IN",
	"COMMAND",
	"STATUS",
	"INVALID_4",
	"INVALID_5",
	"MSG_OUT",
	"MSG_IN",
};


//-------------------------------------------------
//  step - advance the SCSI state machine
//-------------------------------------------------

void wd33c9x_base_device::step(bool timeout)
{
	const uint8_t cc = (m_regs[COMMAND] & COMMAND_CC);
	const bool sat = (cc == COMMAND_CC_SELECT_TRANSFER || cc == COMMAND_CC_SELECT_ATN_TRANSFER);

	const uint32_t ctrl = scsi_bus->ctrl_r();
	const uint32_t data = scsi_bus->data_r();

	LOGMASKED(LOG_STEP,
			  "%s: step - PHASE:%s BSY:%x SEL:%x REQ:%x ACK:%x ATN:%x RST:%x DATA:%02x (%s.%s) %s\n",
			  shortname(),
			  phase_strings[ctrl & S_PHASE_MASK],
			  (ctrl & S_BSY) ? 1 : 0,
			  (ctrl & S_SEL) ? 1 : 0,
			  (ctrl & S_REQ) ? 1 : 0,
			  (ctrl & S_ACK) ? 1 : 0,
			  (ctrl & S_ATN) ? 1 : 0,
			  (ctrl & S_RST) ? 1 : 0,
			  data,
			  state_names[m_scsi_state & STATE_MASK], substate_names[m_scsi_state >> SUB_SHIFT],
			  (timeout) ? "timeout" : "change"
			  );

	if (m_mode == MODE_I) {
		if (ctrl & S_BSY) {
			if (ctrl & S_REQ) {
				uint8_t xfr_phase = (ctrl & S_PHASE_MASK);
				switch (m_scsi_state) {
				case DISC_SEL_ARBITRATION:
					m_xfr_phase = xfr_phase;
					break;

				case INIT_XFR_WAIT_REQ:
					break;

				default:
					if (m_xfr_phase != xfr_phase) {
						fatalerror("%s: Unexpected phase change during state.\n", shortname());
					}
					break;
				}
			}
		} else {
			LOGMASKED(LOG_STATE, "Target disconnected\n");
			if (sat) {
				switch (m_regs[COMMAND_PHASE]) {
				case COMMAND_PHASE_DISCONNECT_MESSAGE:
					set_scsi_state(FINISHED);
					m_regs[COMMAND_PHASE] = COMMAND_PHASE_DISCONNECTED;
					break;

				case COMMAND_PHASE_COMMAND_COMPLETE:
					if (m_regs[CONTROL] & CONTROL_EDI) {
						set_scsi_state(FINISHED);
						irq_fifo_push(SCSI_STATUS_SELECT_TRANSFER_SUCCESS);
					} else {
						set_scsi_state(FINISHED);
						irq_fifo_push(SCSI_STATUS_DISCONNECT);
					}
					break;

				default:
					fatalerror("%s: Unhandled command phase during Select-and-Transfer disconnect.\n", shortname());
					break;
				}
			} else {
				set_scsi_state(FINISHED);
				irq_fifo_push(SCSI_STATUS_DISCONNECT);
			}
			m_mode = MODE_D;
			scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
			scsi_bus->ctrl_wait(scsi_refid, S_SEL|S_BSY|S_RST, S_ALL);
		}
	}

	switch (m_scsi_state & SUB_MASK ? m_scsi_state & SUB_MASK : m_scsi_state & STATE_MASK) {
	case IDLE:
		break;

	case FINISHED:
		set_scsi_state(IDLE);
		m_regs[AUXILIARY_STATUS] &= ~(AUXILIARY_STATUS_CIP | AUXILIARY_STATUS_BSY);
		update_irq();
		break;

	case ARB_WAIT_BUS_FREE << SUB_SHIFT:
		if (!(ctrl & (S_BSY | S_SEL))) {
			set_scsi_state_sub(ARB_CHECK_FREE);
			delay(1);
		}
		break;

	case ARB_CHECK_FREE << SUB_SHIFT:
		if (ctrl & (S_BSY | S_SEL)) {
			set_scsi_state_sub(ARB_CHECK_FREE);
			break;
		}
		if (timeout) {
			scsi_bus->data_w(scsi_refid, 1 << scsi_id);
			scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
			set_scsi_state_sub(ARB_EXAMINE_BUS);
			delay(1);
		}
		break;

	case ARB_EXAMINE_BUS << SUB_SHIFT:
		if (timeout) {
			if (ctrl & S_SEL) {
				scsi_bus->ctrl_w(scsi_refid, 0, S_BSY);
				scsi_bus->data_w(scsi_refid, 0);
				set_scsi_state_sub(ARB_WAIT_BUS_FREE);
			} else {
				int win;
				for (win = 7; win >=0 && !(data & (1 << win)); win--);
				if (win == scsi_id) {
					scsi_bus->ctrl_w(scsi_refid, S_SEL, S_SEL);
					set_scsi_state_sub(ARB_ASSERT_SEL);
					delay(1);
				} else {
					scsi_bus->data_w(scsi_refid, 0);
					scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
					set_scsi_state_sub(ARB_CHECK_FREE);
				}
			}
		}
		break;

	case ARB_ASSERT_SEL << SUB_SHIFT:
		if (timeout) {
			scsi_bus->data_w(scsi_refid, (1 << scsi_id) | (1 << (m_regs[DESTINATION_ID] & DESTINATION_ID_DI)));
			set_scsi_state_sub(ARB_SET_DEST);
			delay(1);
		}
		break;

	case ARB_SET_DEST << SUB_SHIFT:
		if (timeout) {
			scsi_bus->ctrl_w(scsi_refid, (cc == COMMAND_CC_SELECT_ATN || cc == COMMAND_CC_SELECT_ATN_TRANSFER) ? S_ATN : 0, S_ATN | S_BSY);
			set_scsi_state_sub(ARB_RELEASE_BUSY);
			delay(1);
		}
		break;

	case ARB_RELEASE_BUSY << SUB_SHIFT:
		if (timeout) {
			if (ctrl & S_BSY) {
				set_scsi_state_sub(ARB_DESKEW_WAIT);
				if (cc == COMMAND_CC_RESELECT) {
					scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
				}
				delay(1);
			} else {
				set_scsi_state_sub(ARB_TIMEOUT_BUSY);
				delay(1); // Should be the select timeout...
			}
		}
		break;

	case ARB_DESKEW_WAIT << SUB_SHIFT:
		if (timeout) {
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_SEL);
			m_mode = (cc == COMMAND_CC_RESELECT) ? MODE_T : MODE_I;
			set_scsi_state_sub(0);
			step(true);
		}
		break;

	case ARB_TIMEOUT_BUSY << SUB_SHIFT:
		if (timeout) {
			scsi_bus->data_w(scsi_refid, 0);
			set_scsi_state_sub(ARB_TIMEOUT_ABORT);
			delay(1000);
		} else if (ctrl & S_BSY) {
			set_scsi_state_sub(ARB_DESKEW_WAIT);
			if (cc == COMMAND_CC_RESELECT) {
				scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
			}
			delay(1);
		}
		break;

	case ARB_TIMEOUT_ABORT << SUB_SHIFT:
		if (timeout) {
			if (ctrl & S_BSY) {
				set_scsi_state_sub(ARB_DESKEW_WAIT);
				if (cc == COMMAND_CC_RESELECT) {
					scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
				}
				delay(1);
			} else {
				scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
				scsi_bus->ctrl_wait(scsi_refid, S_SEL|S_BSY|S_RST, S_ALL);
				m_regs[AUXILIARY_STATUS] &= ~(AUXILIARY_STATUS_CIP | AUXILIARY_STATUS_BSY);
				m_mode = MODE_D;
				set_scsi_state(IDLE);
				irq_fifo_push(SCSI_STATUS_SELECTION_TIMEOUT);
				update_irq();
			}
		}
		break;

	case SEND_WAIT_SETTLE << SUB_SHIFT:
		if (timeout) {
			set_scsi_state_sub(SEND_WAIT_REQ_0);
			step(false);
		}
		break;

	case SEND_WAIT_REQ_0 << SUB_SHIFT:
		if (!(ctrl & S_REQ)) {
			set_scsi_state_sub(0);
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
			if (sat) {
				switch (m_xfr_phase) {
				case S_PHASE_COMMAND:
					++m_regs[COMMAND_PHASE];
					break;
				}
			}
			step(false);
		}
		break;

	case RECV_WAIT_REQ_1 << SUB_SHIFT:
		if (ctrl & S_REQ) {
			set_scsi_state_sub(RECV_WAIT_SETTLE);
			delay(1);
		}
		break;

	case RECV_WAIT_SETTLE << SUB_SHIFT:
		if (timeout) {
			if (sat) {
				switch (m_xfr_phase) {
				case S_PHASE_DATA_IN:
					data_fifo_push(data);
					if ((m_regs[CONTROL] & CONTROL_DM) != CONTROL_DM_POLLED) {
						set_drq();
					} else {
						decrement_transfer_count();
						m_regs[AUXILIARY_STATUS] |= AUXILIARY_STATUS_DBR;
					}
					break;

				case S_PHASE_STATUS:
					m_regs[TARGET_LUN] = data;
					m_regs[COMMAND_PHASE] = COMMAND_PHASE_STATUS_RECEIVED;
					break;

				case S_PHASE_MSG_IN:
					m_last_message = data;
					break;

				default:
					fatalerror("%s: Unexpected phase in RECV_WAIT_SETTLE.\n", shortname());
					break;
				}
			} else {
				data_fifo_push(data);
				if (m_xfr_phase == S_PHASE_DATA_IN && (m_regs[CONTROL] & CONTROL_DM) != CONTROL_DM_POLLED) {
					set_drq();
				} else {
					decrement_transfer_count();
					m_regs[AUXILIARY_STATUS] |= AUXILIARY_STATUS_DBR;
				}
			}
			set_scsi_state_sub(RECV_WAIT_REQ_0);
			scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
			step(false);
		}
		break;

	case RECV_WAIT_REQ_0 << SUB_SHIFT:
		if (!(ctrl & S_REQ)) {
			set_scsi_state_sub(0);
			step(false);
		}
		break;

	case DISC_SEL_ARBITRATION:
		scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ);
		if (cc == COMMAND_CC_SELECT || cc == COMMAND_CC_SELECT_ATN) {
			set_scsi_state(FINISHED);
			irq_fifo_push(SCSI_STATUS_SELECT_SUCCESS);
			if (ctrl & S_REQ) {
				irq_fifo_push(SCSI_STATUS_REQ | m_xfr_phase);
			}
		} else {
			if(cc == COMMAND_CC_SELECT_TRANSFER) {
				m_regs[COMMAND_PHASE] = COMMAND_PHASE_CP_BYTES_0;
				std::string cmd;
				for (uint8_t i = 0; i < m_command_length; ++i) {
					const uint8_t command_byte = m_regs[CDB_1 + i];
					cmd += util::string_format(" %02x", command_byte);
					data_fifo_push(command_byte);
				}
				LOGMASKED(LOG_COMMANDS, "Sending command:%s (%d)\n", cmd, m_transfer_count);
			} else
				m_regs[COMMAND_PHASE] = COMMAND_PHASE_SELECTED;
			set_scsi_state(INIT_XFR);
		}
		step(false);
		break;

	case INIT_XFR:
		if (ctrl & S_REQ) {
			switch (m_xfr_phase) {
			case S_PHASE_DATA_OUT:
				if ((m_regs[CONTROL] & CONTROL_DM) != CONTROL_DM_POLLED) {
					if(!data_fifo_full() && m_transfer_count > 0)
						set_drq();
				}
				if (!data_fifo_empty()) {
					set_scsi_state(INIT_XFR_WAIT_REQ);
					delay(send_byte());
				} else if ((m_regs[CONTROL] & CONTROL_DM) == CONTROL_DM_POLLED) {
					m_regs[AUXILIARY_STATUS] |= AUXILIARY_STATUS_DBR;
				} else {
					delay(1);
				}
				break;

			case S_PHASE_COMMAND:
				if (!data_fifo_empty()) {
					uint32_t mask;
					if (sat) {
						mask = 0;
					} else {
						m_regs[AUXILIARY_STATUS] |= AUXILIARY_STATUS_DBR;
						mask = (m_transfer_count == 0 && m_data_fifo_size == 1) ? S_ATN : 0;
					}
					set_scsi_state(INIT_XFR_WAIT_REQ);
					delay(send_byte(0, mask));
				} else if (!sat) {
					m_regs[AUXILIARY_STATUS] |= AUXILIARY_STATUS_DBR;
				}
				break;

			case S_PHASE_MSG_OUT:
				if (sat) {
					data_fifo_push(get_msg_out());
				}
				if (!data_fifo_empty()) {
					uint32_t mask;
					if (sat) {
						mask = S_ATN;
					} else {
						m_regs[AUXILIARY_STATUS] |= AUXILIARY_STATUS_DBR;
						mask = (m_transfer_count == 0 && m_data_fifo_size == 1) ? S_ATN : 0;
					}
					set_scsi_state(INIT_XFR_WAIT_REQ);
					delay(send_byte(0, mask));
				} else if (!sat) {
					m_regs[AUXILIARY_STATUS] |= AUXILIARY_STATUS_DBR;
				}
				break;

			case S_PHASE_DATA_IN:
			case S_PHASE_STATUS:
			case S_PHASE_MSG_IN:
				if (!data_fifo_full()) {
					// if it's the last message byte, ACK remains asserted, terminate with function_complete()
					//state = (m_xfr_phase == S_PHASE_MSG_IN && (!dma_command || tcounter == 1)) ? INIT_XFR_RECV_BYTE_NACK : INIT_XFR_RECV_BYTE_ACK;
					if (m_drq_state) {
						delay(1);
					} else {
						scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ);
						set_scsi_state((RECV_WAIT_REQ_1 << SUB_SHIFT) | INIT_XFR_RECV_BYTE_ACK);
						step(false);
					}
				}
				break;

			default:
				fatalerror("%s: Invalid phase during INIT_XFR.\n", shortname());
				break;
			}
		}
		break;

	case INIT_XFR_WAIT_REQ:
		if (ctrl & S_REQ) {
			uint16_t next_state = m_scsi_state;

			const uint8_t xfr_phase = (ctrl & S_PHASE_MASK);

			switch ((m_xfr_phase << 3) | xfr_phase) {
			case ((S_PHASE_MSG_OUT << 3) | S_PHASE_MSG_OUT):
			case ((S_PHASE_COMMAND << 3) | S_PHASE_COMMAND):
			case ((S_PHASE_MSG_IN  << 3) | S_PHASE_MSG_IN):
				next_state = INIT_XFR;
				break;

			case ((S_PHASE_DATA_IN  << 3) | S_PHASE_DATA_IN):
			case ((S_PHASE_DATA_OUT << 3) | S_PHASE_DATA_OUT):
				if (sat || cc == COMMAND_CC_TRANSFER_INFO) {
					if (m_transfer_count > 0 || (m_xfr_phase == S_PHASE_DATA_OUT && !data_fifo_empty())) {
						next_state = INIT_XFR;
					}
					else {
						next_state = FINISHED;
						uint8_t scsi_status;
						if (sat) {
							m_regs[COMMAND_PHASE] = COMMAND_PHASE_TRANSFER_COUNT;
							scsi_status = SCSI_STATUS_UNEXPECTED_PHASE;
						}
						else {
							scsi_status = SCSI_STATUS_TRANSFER_SUCCESS;
						}
						irq_fifo_push(scsi_status | m_xfr_phase);
					}
				}
				else {
					fatalerror("%s: Unhandled command in data phase.\n", shortname());
					next_state = FINISHED;
				}
				break;

			case ((S_PHASE_MSG_OUT  << 3) | S_PHASE_COMMAND):
			case ((S_PHASE_COMMAND  << 3) | S_PHASE_DATA_OUT):
			case ((S_PHASE_COMMAND  << 3) | S_PHASE_DATA_IN):
			case ((S_PHASE_COMMAND  << 3) | S_PHASE_STATUS):
			case ((S_PHASE_COMMAND  << 3) | S_PHASE_MSG_IN):
			case ((S_PHASE_DATA_OUT << 3) | S_PHASE_STATUS):
			case ((S_PHASE_DATA_IN  << 3) | S_PHASE_STATUS):
			case ((S_PHASE_STATUS   << 3) | S_PHASE_MSG_IN):
				if (!(m_xfr_phase & 1) && !data_fifo_empty()) {
					fatalerror("%s: Data FIFO is not empty on phase transition.\n", shortname());
				}

				if (sat) {
					switch (xfr_phase) {
					case S_PHASE_MSG_OUT:
						next_state = INIT_XFR;
						break;

					case S_PHASE_COMMAND: {
						next_state = INIT_XFR;
						m_regs[COMMAND_PHASE] = COMMAND_PHASE_CP_BYTES_0;
						std::string cmd;
						for (uint8_t i = 0; i < m_command_length; ++i) {
							const uint8_t command_byte = m_regs[CDB_1 + i];
							cmd += util::string_format(" %02x", command_byte);
							data_fifo_push(command_byte);
						}
						LOGMASKED(LOG_COMMANDS, "Sending command:%s (%d)\n", cmd, m_transfer_count);
						break;
					}

					case S_PHASE_DATA_OUT:
					case S_PHASE_DATA_IN:
						next_state = INIT_XFR;
						break;

					case S_PHASE_STATUS:
						next_state = INIT_XFR;
						m_regs[COMMAND_PHASE] = COMMAND_PHASE_RECEIVE_STATUS;
						break;

					case S_PHASE_MSG_IN:
						next_state = INIT_XFR;
						break;

					default:
						fatalerror("%s: Unhandled phase in Select-w/Atn-and-Transfer.\n", shortname());
						next_state = FINISHED;
						break;
					}
				}
				else if (cc == COMMAND_CC_TRANSFER_INFO) {
					next_state = FINISHED;
					irq_fifo_push(SCSI_STATUS_TRANSFER_SUCCESS | xfr_phase);
				}
				else {
					fatalerror("%s: Unhandled command in data phase.\n", shortname());
					next_state = FINISHED;
				}
				break;

			default:
				fatalerror("%s: Unhandled phase transition in INIT_XFR_WAIT_REQ.\n", shortname());
				next_state = FINISHED;
				break;
			}

			if (next_state != m_scsi_state) {
				set_scsi_state(next_state);
				m_xfr_phase = xfr_phase;
				step(false);
			}
		}
		break;

	case INIT_XFR_RECV_BYTE_ACK:
		if (sat && m_xfr_phase == S_PHASE_MSG_IN) {
			if (m_regs[COMMAND_PHASE] <= COMMAND_PHASE_CP_BYTES_C) {
				switch (m_last_message) {
				case SM_SAVE_DATA_POINTER:
					set_scsi_state(FINISHED);
					irq_fifo_push(SCSI_STATUS_SAVE_DATA_POINTERS);
					m_regs[COMMAND_PHASE] = COMMAND_PHASE_SAVE_DATA_POINTER;
					break;

				case SM_DISCONNECT:
					m_regs[COMMAND_PHASE] = COMMAND_PHASE_DISCONNECT_MESSAGE;
					break;

				default:
					fatalerror("%s: Unhandled MSG_IN %02x.\n", shortname(), m_last_message);
					break;
				}
			} else if (m_regs[COMMAND_PHASE] < COMMAND_PHASE_COMMAND_COMPLETE) {
				switch (m_last_message) {
				case SM_COMMAND_COMPLETE:
					set_scsi_state(FINISHED);
					irq_fifo_push(SCSI_STATUS_SELECT_TRANSFER_SUCCESS);
					m_regs[COMMAND_PHASE] = COMMAND_PHASE_COMMAND_COMPLETE;
					break;
				default:
					fatalerror("%s: Unhandled MSG_IN %02x.\n", shortname(), m_last_message);
					break;
				}
			}
		} else {
			set_scsi_state(INIT_XFR_WAIT_REQ);
		}
		scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		step(false);
		break;

	default:
		fatalerror("%s: Unhandled state in step.\n", shortname());
		break;
	}
}


//-------------------------------------------------
//  load_transfer_count
//-------------------------------------------------

void wd33c9x_base_device::load_transfer_count()
{
	if (m_regs[COMMAND] & COMMAND_SBT) {
		m_transfer_count = 1;
	}
	else {
		m_transfer_count = (
			(uint32_t(m_regs[TRANSFER_COUNT_MSB]) << 16) |
			(uint32_t(m_regs[TRANSFER_COUNT])     <<  8) |
			(uint32_t(m_regs[TRANSFER_COUNT_LSB]) <<  0)
		);
		if (m_transfer_count == 0) {
			m_transfer_count = 1;
		}
	}
	LOGMASKED(LOG_COMMANDS, "Transfer Count %d bytes\n", m_transfer_count);
}


//-------------------------------------------------
//  decrement_transfer_count
//-------------------------------------------------

bool wd33c9x_base_device::decrement_transfer_count()
{
	if (m_transfer_count == 0) {
		return true;
	}
	--m_transfer_count;
	if (m_transfer_count == 0) {
		// After the completion of any successful transfer,
		// including commands issued in Single Byte Transfer
		// mode, the Transfer Count Register will be zero.
		m_regs[TRANSFER_COUNT_MSB] = 0;
		m_regs[TRANSFER_COUNT] = 0;
		m_regs[TRANSFER_COUNT_LSB] = 0;
		return true;
	}
	return false;
}


//-------------------------------------------------
//  data_fifo_pop
//-------------------------------------------------

uint8_t wd33c9x_base_device::data_fifo_pop()
{
	if (data_fifo_empty()) {
		fatalerror("%s: Data FIFO underflow.\n", shortname());
	}
	--m_data_fifo_size;
	uint8_t ret = m_data_fifo[m_data_fifo_pos];
	m_data_fifo_pos = (m_data_fifo_pos + 1) % DATA_FIFO_SIZE;
	return ret;
}


//-------------------------------------------------
//  data_fifo_push
//-------------------------------------------------

void wd33c9x_base_device::data_fifo_push(const uint8_t data)
{
	if (data_fifo_full()) {
		fatalerror("%s: Data FIFO overflow.\n", shortname());
	}
	m_data_fifo[(m_data_fifo_pos + m_data_fifo_size) % DATA_FIFO_SIZE] = data;
	++m_data_fifo_size;
}


//-------------------------------------------------
//  data_fifo_empty
//-------------------------------------------------

bool wd33c9x_base_device::data_fifo_empty() const
{
	return m_data_fifo_size <= 0;
}


//-------------------------------------------------
//  data_fifo_full
//-------------------------------------------------

bool wd33c9x_base_device::data_fifo_full() const
{
	return m_data_fifo_size >= DATA_FIFO_SIZE;
}


//-------------------------------------------------
//  data_fifo_reset
//-------------------------------------------------

void wd33c9x_base_device::data_fifo_reset()
{
	memset(m_data_fifo, 0, sizeof(m_data_fifo));
	m_data_fifo_pos = 0;
	m_data_fifo_size = 0;
}


//-------------------------------------------------
//  send_byte
//-------------------------------------------------

uint32_t wd33c9x_base_device::send_byte(const uint32_t value, const uint32_t mask)
{
	set_scsi_state_sub(SEND_WAIT_SETTLE);
	scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ);
	scsi_bus->data_w(scsi_refid, data_fifo_pop());
	scsi_bus->ctrl_w(scsi_refid, S_ACK | value, S_ACK | mask);
	return 1;
}


//-------------------------------------------------
//  set_scsi_state - change SCSI state
//-------------------------------------------------

void wd33c9x_base_device::set_scsi_state(uint16_t state)
{
	LOGMASKED(LOG_STEP, "SCSI state change: %s.%s to %s.%s\n",
			  state_names[m_scsi_state & STATE_MASK], substate_names[m_scsi_state >> SUB_SHIFT],
			  state_names[state & STATE_MASK], substate_names[state >> SUB_SHIFT]);
	m_scsi_state = state;
}

//-------------------------------------------------
//  set_scsi_state_sub - change SCSI sub-state
//-------------------------------------------------

void wd33c9x_base_device::set_scsi_state_sub(uint8_t sub)
{
	set_scsi_state((m_scsi_state & STATE_MASK) | (uint16_t(sub) << SUB_SHIFT));
}


//-------------------------------------------------
//  irq_fifo_pop
//-------------------------------------------------

uint8_t wd33c9x_base_device::irq_fifo_pop()
{
	if (irq_fifo_empty()) {
		fatalerror("%s: IRQ FIFO underflow.\n", shortname());
	}
	--m_irq_fifo_size;
	uint8_t ret = m_irq_fifo[m_irq_fifo_pos];
	m_irq_fifo_pos = (m_irq_fifo_pos + 1) % IRQ_FIFO_SIZE;
	return ret;
}


//-------------------------------------------------
//  irq_fifo_push
//-------------------------------------------------

void wd33c9x_base_device::irq_fifo_push(const uint8_t status)
{
	if (irq_fifo_full()) {
		fatalerror("%s: IRQ FIFO overflow.\n", shortname());
	}
	// Kind of hacky, but don't push duplicate interrupt statuses.
	if (m_irq_fifo_size &&
		m_irq_fifo[(m_irq_fifo_pos + m_irq_fifo_size - 1) % IRQ_FIFO_SIZE] == status) {
		return;
	}
	m_irq_fifo[(m_irq_fifo_pos + m_irq_fifo_size) % IRQ_FIFO_SIZE] = status;
	++m_irq_fifo_size;
}


//-------------------------------------------------
//  irq_fifo_empty
//-------------------------------------------------

bool wd33c9x_base_device::irq_fifo_empty() const
{
	return m_irq_fifo_size <= 0;
}


//-------------------------------------------------
//  irq_fifo_full
//-------------------------------------------------

bool wd33c9x_base_device::irq_fifo_full() const
{
	return m_irq_fifo_size >= IRQ_FIFO_SIZE;
}


//-------------------------------------------------
//  irq_fifo_reset
//-------------------------------------------------

void wd33c9x_base_device::irq_fifo_reset()
{
	memset(m_irq_fifo, 0, sizeof(m_irq_fifo));
	m_irq_fifo_pos = 0;
	m_irq_fifo_size = 0;
}


//-------------------------------------------------
//  update_irq
//-------------------------------------------------

void wd33c9x_base_device::update_irq()
{
	if (m_regs[AUXILIARY_STATUS] & AUXILIARY_STATUS_INT) {
		m_regs[AUXILIARY_STATUS] &= ~AUXILIARY_STATUS_INT;
		LOGMASKED(LOG_LINES, "Clearing IRQ\n");
		m_irq_cb(CLEAR_LINE);
	}
	if (!irq_fifo_empty()) {
		m_regs[SCSI_STATUS] = irq_fifo_pop();
		m_regs[AUXILIARY_STATUS] |= AUXILIARY_STATUS_INT;

		const uint8_t cc = (m_regs[COMMAND] & COMMAND_CC);
		if (cc == COMMAND_CC_SELECT_TRANSFER || cc == COMMAND_CC_SELECT_ATN_TRANSFER) {
			switch (m_regs[SCSI_STATUS]) {
			case SCSI_STATUS_DISCONNECT:
				if (!(m_regs[CONTROL] & CONTROL_IDI)) {
					return;
				}
				break;

			case SCSI_STATUS_SELECT_TRANSFER_SUCCESS:
				if ((m_regs[CONTROL] & CONTROL_EDI) && m_mode != MODE_D) {
					return;
				}
				break;
			}
		}

		LOGMASKED(LOG_LINES, "Asserting IRQ - SCSI Status (%02x)\n", m_regs[SCSI_STATUS]);
		m_irq_cb(ASSERT_LINE);
	}
}


//-------------------------------------------------
//  set_drq
//-------------------------------------------------

void wd33c9x_base_device::set_drq()
{
	if (!m_drq_state) {
		LOGMASKED(LOG_LINES, "Asserting DRQ\n");
		m_drq_state = true;
		m_drq_cb(ASSERT_LINE);
	}
}


//-------------------------------------------------
//  clear_drq
//-------------------------------------------------

void wd33c9x_base_device::clear_drq()
{
	if (m_drq_state) {
		LOGMASKED(LOG_LINES, "Clearing DRQ\n");
		m_drq_state = false;
		m_drq_cb(CLEAR_LINE);
	}
}


//-------------------------------------------------
//  delay
//-------------------------------------------------

void wd33c9x_base_device::delay(uint32_t cycles)
{
	// FIXME - This should take Own ID Frequency Scale into account.
	delay_cycles(cycles);
}


//-------------------------------------------------
//  delay_cycles
//-------------------------------------------------

void wd33c9x_base_device::delay_cycles(uint32_t cycles)
{
	m_timer->adjust(clocks_to_attotime(cycles));
}


//-------------------------------------------------
//  set_command_length
//-------------------------------------------------

bool wd33c9x_base_device::set_command_length(const uint8_t cc)
{
	const bool eaf = ((m_regs[OWN_ID] & OWN_ID_EAF) != 0);
	bool ret;
	if (eaf && (cc == COMMAND_CC_SELECT_TRANSFER || cc == COMMAND_CC_SELECT_ATN_TRANSFER)) {
		m_command_length &= OWN_ID_CDB_SIZE;
		ret = true;
	} else if (eaf && cc == COMMAND_CC_WAIT_SELECT_RECEIVE_DATA) {
		m_command_length = 6;
		m_regs[COMMAND_PHASE] = COMMAND_PHASE_CP_BYTES_1;
		irq_fifo_push(SCSI_STATUS_NEED_COMMAND_SIZE);
		update_irq();
		ret = false;
	} else {
		switch (m_regs[CDB_1] >> 5) {
		default:
		case 0: m_command_length = 6;  break;
		case 1: m_command_length = 10; break;
		case 5: m_command_length = 12; break;
		}
		ret = true;
	}
	LOGMASKED(LOG_COMMANDS, "SCSI Command Length %d bytes\n", m_command_length);
	return ret;
}

//-------------------------------------------------
//  get_msg_out
//-------------------------------------------------

uint8_t wd33c9x_base_device::get_msg_out() const
{
	return 0x80 | ((m_regs[SOURCE_ID] & SOURCE_ID_ER) ? 0x40 : 0x00) | (m_regs[TARGET_LUN] & TARGET_LUN_TL);
}



wd33c92_device::wd33c92_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd33c9x_base_device(mconfig, WD33C92, tag, owner, clock)
{
}

wd33c93_device::wd33c93_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd33c9x_base_device(mconfig, WD33C93, tag, owner, clock)
{
}

wd33c93a_device::wd33c93a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd33c9x_base_device(mconfig, WD33C93A, tag, owner, clock)
{
}

wd33c93b_device::wd33c93b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd33c9x_base_device(mconfig, WD33C93B, tag, owner, clock)
{
}
