// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef __NSCSI_BUS_H__
#define __NSCSI_BUS_H__

#include "emu.h"

#define MCFG_NSCSI_BUS_ADD(_tag)        \
	MCFG_DEVICE_ADD(_tag, NSCSI_BUS, 0)

#define MCFG_NSCSI_ADD(_tag, _slot_intf, _def_slot, _fixed) \
	MCFG_DEVICE_ADD(_tag, NSCSI_CONNECTOR, 0)                   \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _fixed)

class nscsi_device;

class nscsi_bus_device : public device_t
{
public:
	nscsi_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void ctrl_w(int refid, UINT32 lines, UINT32 mask);
	void data_w(int refid, UINT32 lines);
	void ctrl_wait(int refid, UINT32 lines, UINT32 mask);

	UINT32 ctrl_r() const;
	UINT32 data_r() const;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_config_complete() override;

private:
	struct dev_t {
		nscsi_device *dev;
		UINT32 ctrl, wait_ctrl;
		UINT32 data;
	};

	dev_t dev[16];
	int devcnt;

	UINT32 data, ctrl;

	void regen_data();
	void regen_ctrl(int refid);
};

class nscsi_connector: public device_t,
						public device_slot_interface
{
public:
	nscsi_connector(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~nscsi_connector();

	nscsi_device *get_device();

protected:
	virtual void device_start() override;
};

class nscsi_device : public device_t,
						public device_slot_card_interface
{
public:
	// Here because the biggest users are the devices, not the bus
	enum {
		S_INP = 0x0001,
		S_CTL = 0x0002,
		S_MSG = 0x0004,
		S_BSY = 0x0008,
		S_SEL = 0x0010,
		S_REQ = 0x0020,
		S_ACK = 0x0040,
		S_ATN = 0x0080,
		S_RST = 0x0100,
		S_ALL = 0x01ff,

		S_PHASE_DATA_OUT = 0,
		S_PHASE_DATA_IN  = S_INP,
		S_PHASE_COMMAND  = S_CTL,
		S_PHASE_STATUS   = S_CTL|S_INP,
		S_PHASE_MSG_OUT  = S_MSG|S_CTL,
		S_PHASE_MSG_IN   = S_MSG|S_CTL|S_INP,
		S_PHASE_MASK     = S_MSG|S_CTL|S_INP
	};

	nscsi_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	void connect_to_bus(nscsi_bus_device *bus, int refid, int default_scsi_id);
	virtual void scsi_ctrl_changed();
protected:
	int scsi_id;
	int scsi_refid;
	nscsi_bus_device *scsi_bus;

	virtual void device_start() override;
};

class nscsi_full_device : public nscsi_device
{
public:
	nscsi_full_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual void scsi_ctrl_changed() override;
protected:
	enum { SCSI_TIMER = 100 };

	// SCSI status returns
	enum {
		SS_GOOD                          = 0x00,
		SS_CHECK_CONDITION               = 0x02,
		SS_CONDITION_MET                 = 0x04,
		SS_BUSY                          = 0x08,
		SS_INT_GOOD                      = 0x10,
		SS_INT_CONDITION_MET             = 0x14,
		SS_RESV_CONFLICT                 = 0x18,
		SS_TERMINATED                    = 0x22,
		SS_QUEUE_FULL                    = 0x28
	};

	// SCSI commands
	static const char *const command_names[256];
	enum {
		SC_TEST_UNIT_READY               = 0x00,
		SC_REZERO                        = 0x01,
		SC_REQUEST_SENSE                 = 0x03,
		SC_FORMAT_UNIT                   = 0x04,
		SC_REASSIGN_BLOCKS               = 0x07,
		SC_READ_6                        = 0x08,
		SC_RECIEVE                       = 0x08,
		SC_WRITE_6                       = 0x0a,
		SC_SEND                          = 0x0a,
		SC_SEEK                          = 0x0b,
		SC_INQUIRY                       = 0x12,
		SC_MODE_SELECT_6                 = 0x15,
		SC_RESERVE_6                     = 0x16,
		SC_RELEASE_6                     = 0x17,
		SC_MODE_SENSE_6                  = 0x1a,
		SC_START_STOP_UNIT               = 0x1b,
		SC_RECIEVE_DIAG_RES              = 0x1c,
		SC_SEND_DIAGNOSTICS              = 0x1d,
		SC_PREVENT_ALLOW_MEDIUM_REMOVAL  = 0x1e,
		SC_READ_FORMAT_CAPACITIES        = 0x23,
		SC_READ_CAPACITY                 = 0x25,
		SC_READ_10                       = 0x28,
		SC_READ_GENERATION               = 0x29,
		SC_WRITE_10                      = 0x2a,
		SC_SEEK_10                       = 0x2b,
		SC_ERASE_10                      = 0x2c,
		SC_READ_UPDATED_BLOCK_10         = 0x2d,
		SC_WRITE_VERIFY                  = 0x2e,
		SC_VERIFY                        = 0x2f,
		SC_SEARCH_DATA_HIGH_10           = 0x30,
		SC_SEARCH_DATA_EQUAL_10          = 0x31,
		SC_SEARCH_DATA_LOW_10            = 0x32,
		SC_SET_LIMITS_10                 = 0x33,
		SC_PREFETCH                      = 0x34,
		SC_SYNC_CACHE                    = 0x35,
		SC_LOCK_UNLOCK_CACHE             = 0x36,
		SC_READ_DEFECT_DATA              = 0x37,
		SC_MEDIUM_SCAN                   = 0x38,
		SC_COMPARE                       = 0x39,
		SC_COPY_AND_VERIFY               = 0x3a,
		SC_WRITE_BUFFER                  = 0x3b,
		SC_READ_DATA_BUFFER              = 0x3c,
		SC_UPDATE_BLOCK                  = 0x3d,
		SC_READ_LONG                     = 0x3e,
		SC_WRITE_LONG                    = 0x3f,
		SC_CHANGE_DEFINITION             = 0x40,
		SC_WRITE_SAME                    = 0x41,
		SC_READ_SUB_CHANNEL              = 0x42,
		SC_READ_TOC_PMA_ATIP             = 0x43,
		SC_READ_HEADER                   = 0x44,
		SC_PLAY_AUDIO_10                 = 0x45,
		SC_GET_CONFIGURATION             = 0x46,
		SC_PLAY_AUDIO_MSF                = 0x47,
		SC_PLAY_AUDIO_TRACK_INDEX        = 0x48,
		SC_PLAY_RELATIVE_10              = 0x49,
		SC_GET_EVENT_STATUS_NOTIFICATION = 0x4a,
		SC_PAUSE_RESUME                  = 0x4b,
		SC_LOG_SELECT                    = 0x4c,
		SC_LOG_SENSE                     = 0x4d,
		SC_STOP_PLAY_SCAN                = 0x4e,
		SC_XDWRITE                       = 0x50,
		SC_XPWRITE                       = 0x51,
		SC_READ_DISC_INFORMATION         = 0x51,
		SC_READ_TRACK_INFORMATION        = 0x52,
		SC_XDREAD                        = 0x52,
		SC_RESERVE_TRACK                 = 0x53,
		SC_SEND_OPC_INFORMATION          = 0x54,
		SC_MODE_SELECT_10                = 0x55,
		SC_RESERVE_10                    = 0x56,
		SC_RELEASE_10                    = 0x57,
		SC_REPAIR_TRACK                  = 0x58,
		SC_READ_MASTER_CUE               = 0x59,
		SC_MODE_SENSE_10                 = 0x5a,
		SC_CLOSE_TRACK_SESSION           = 0x5b,
		SC_READ_BUFFER_CAPACITY          = 0x5c,
		SC_SEND_CUE_SHEET                = 0x5d,
		SC_PERSISTENT_RESERVE_IN         = 0x5e,
		SC_PERSISTENT_RESERVE_OUT        = 0x5f,
		SC_XDWRITE_EXTENDED              = 0x80,
		SC_REBUILD                       = 0x81,
		SC_REGENERATE                    = 0x82,
		SC_EXTENDED_COPY                 = 0x83,
		SC_RECEIVE_COPY_RESULTS          = 0x84,
		SC_REPORT_LUNS                   = 0xa0,
		SC_BLANK                         = 0xa1,
		SC_SEND_EVENT                    = 0xa2,
		SC_REPORT_DEVICE_IDENTIFIER      = 0xa3,
		SC_SEND_KEY                      = 0xa3,
		SC_REPORT_KEY                    = 0xa4,
		SC_SET_DEVICE_IDENTIFIER         = 0xa4,
		SC_PLAY_AUDIO_12                 = 0xa5,
		SC_LOAD_UNLOAD_MEDIUM            = 0xa6,
		SC_MOVE_MEDIUM_ATTACHED          = 0xa7,
		SC_SET_READ_AHEAD                = 0xa7,
		SC_READ_12                       = 0xa8,
		SC_PLAY_RELATIVE_12              = 0xa9,
		SC_WRITE_12                      = 0xaa,
		SC_ERASE_12                      = 0xac,
		SC_GET_PERFORMANCE               = 0xac,
		SC_READ_DVD_STRUCTURE            = 0xad,
		SC_WRITE_AND_VERIFY_12           = 0xae,
		SC_VERIFY_12                     = 0xaf,
		SC_SEARCH_DATA_HIGH_12           = 0xb0,
		SC_SEARCH_DATA_EQUAL_12          = 0xb1,
		SC_SEARCH_DATA_LOW_12            = 0xb2,
		SC_SET_LIMITS_12                 = 0xb3,
		SC_READ_ELEMENT_STATUS_ATTACHED  = 0xb4,
		SC_SET_STREAMING                 = 0xb6,
		SC_READ_DEFECT_DATA_12           = 0xb7,
		SC_READ_CD_MSF                   = 0xb9,
		SC_SCAN_MMC                      = 0xba,
		SC_SET_CD_SPEED                  = 0xbb,
		SC_PLAY_CD                       = 0xbc,
		SC_MECHANISM_STATUS              = 0xbd,
		SC_READ_CD                       = 0xbe,
		SC_SEND_DVD_STRUCTURE            = 0xbf
	};

	// SCSI Messages
	enum {
		SM_COMMAND_COMPLETE              = 0x00,
		SM_EXTENDED_MSG                  = 0x01,
		SM_SAVE_DATA_PTR                 = 0x02,
		SM_RESTORE_PTR                   = 0x03,
		SM_DISCONNECT                    = 0x04,
		SM_INITIATOR_ERROR               = 0x05,
		SM_ABORT                         = 0x06,
		SM_MSG_REJECT                    = 0x07,
		SM_NOP                           = 0x08,
		SM_MSG_PARITY                    = 0x09,
		SM_LCMD_COMPLETE                 = 0x0a,
		SM_LCMD_COMPLETE_F               = 0x0b,
		SM_BUS_DEVICE_RESET              = 0x0c,
		SM_ABORT_TAG                     = 0x0d,
		SM_CLEAR_QUEUE                   = 0x0e,
		SM_INIT_RECOVERY                 = 0x0f,
		SM_RELEASE_RECOVERY              = 0x10,
		SM_TERMINATE_IO                  = 0x11,
		SM_SIMPLE_QUEUE                  = 0x20,
		SM_HEAD_QUEUE                    = 0x21,
		SM_ORDERED_QUEUE                 = 0x22,
		SM_IGNORE_WIDE_RES               = 0x23
	};

	enum {
		SBUF_MAIN,
		SBUF_SENSE
	};

	UINT8 scsi_cmdbuf[4096], scsi_sense_buffer[8];
	int scsi_cmdsize;
	UINT8 scsi_identify;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void scsi_message();
	virtual void scsi_command();

	void scsi_unknown_command();
	void scsi_status_complete(UINT8 st);
	void scsi_data_in(int buf, int size);
	void scsi_data_out(int buf, int size);

	void sense(bool deferred, UINT8 key);
	int get_lun(int def = 0);
	void bad_lun();

	virtual UINT8 scsi_get_data(int buf, int offset);
	virtual void scsi_put_data(int buf, int offset, UINT8 data);

	// Default delays:

	// Arbitration delay (2.4us)
	virtual attotime scsi_arbitation_delay();

	// Assertion period (90ns)
	virtual attotime scsi_assertion_period();

	// Bus clear delay (800ns)
	virtual attotime scsi_bus_clear_delay();

	// Bus free delay (800ns)
	virtual attotime scsi_bus_free_delay();

	// Bus set delay (1.8us)
	virtual attotime scsi_bus_set_delay();

	// Bus settle delay (400ns)
	virtual attotime scsi_bus_settle_delay();

	// Cable skew delay (10ns)
	virtual attotime scsi_cable_skew_delay();

	// Data release delay (400ns)
	virtual attotime scsi_data_release_delay();

	// Deskew delay (45ns)
	virtual attotime scsi_deskew_delay();

	// Disconnection delay (200us)
	virtual attotime scsi_disconnection_delay();

	// Hold time (45ns)
	virtual attotime scsi_hold_time();

	// Negation period (90ns)
	virtual attotime scsi_negation_period();

	// Reset hold time (25us)
	virtual attotime scsi_reset_hold_time();

	// Selection abort time (200us)
	virtual attotime scsi_selection_abort_time();

	// Selection timeout delay (250ms)
	virtual attotime scsi_selection_timeout_delay();

	// Fast assertion period (30ns)
	virtual attotime scsi_fast_assertion_period();

	// Fast cable skew delay (5ns)
	virtual attotime scsi_fast_cable_skew_delay();

	// Fast deskew delay (20ns)
	virtual attotime scsi_fast_deskew_delay();

	// Fast hold time (10ns)
	virtual attotime scsi_fast_hold_time();

	// Fast negation period (30ns)
	virtual attotime scsi_fast_negation_period();

private:
	enum {
		IDLE
	};

	enum {
		TARGET_SELECT_WAIT_BUS_SETTLE = 1,
		TARGET_SELECT_WAIT_SEL_0,

		TARGET_NEXT_CONTROL,
		TARGET_WAIT_MSG_BYTE,
		TARGET_WAIT_CMD_BYTE,
		TARGET_WAIT_DATA_IN_BYTE,
		TARGET_WAIT_DATA_OUT_BYTE
	};

	enum {
		RECV_BYTE_T_WAIT_ACK_0 = 1,
		RECV_BYTE_T_WAIT_ACK_1,
		SEND_BYTE_T_WAIT_ACK_0,
		SEND_BYTE_T_WAIT_ACK_1
	};

	enum {
		STATE_MASK = 0x00ff,
		SUB_SHIFT  = 8,
		SUB_MASK   = 0xff00
	};

	enum {
		BC_MSG_OR_COMMAND,
		BC_STATUS,
		BC_MESSAGE_1,
		BC_MESSAGE_2,
		BC_DATA_IN,
		BC_DATA_OUT,
		BC_BUS_FREE
	};

	struct control {
		int action;
		int param1, param2;
	};

	emu_timer *scsi_timer;

	int scsi_state, scsi_substate;
	int scsi_initiator_id;
	int data_buffer_id, data_buffer_size, data_buffer_pos;

	control buf_control[32];
	int buf_control_rpos;
	int buf_control_wpos;

	control *buf_control_push();
	control *buf_control_pop();

	void step(bool timeout);
	void target_recv_byte();
	void target_send_byte(UINT8 val);
	void target_send_buffer_byte();
	bool command_done();
};


extern const device_type NSCSI_BUS;
extern const device_type NSCSI_CONNECTOR;

#endif
