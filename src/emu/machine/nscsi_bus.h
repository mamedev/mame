#ifndef __NSCSI_BUS_H__
#define __NSCSI_BUS_H__

#include "emu.h"

#define MCFG_NSCSI_BUS_ADD(_tag)		\
	MCFG_DEVICE_ADD(_tag, NSCSI_BUS, 0)

#define MCFG_NSCSI_DEVICE_ADD(_tag, _subtag, _type, _clock)			\
	MCFG_DEVICE_ADD(_tag, NSCSI_CONNECTOR, 0)						\
	downcast<nscsi_connector *>(device)->set_fixed_device(_subtag);	\
	MCFG_DEVICE_ADD(_tag ":" _subtag, _type, _clock)

#define MCFG_NSCSI_FULL_DEVICE_ADD(_tag, _subtag, _type, _clock)	\
	MCFG_NSCSI_DEVICE_ADD(_tag, _subtag, _type, _clock)

#define MCFG_NSCSI_ADD(_tag, _slot_intf, _def_slot, _def_inp)	\
	MCFG_DEVICE_ADD(_tag, NSCSI_CONNECTOR, 0)					\
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)

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
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

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
	void set_fixed_device(const char *subtag);

protected:
	virtual void device_start();

private:
	const char *fixed_subtag;
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
		S_PHASE_MASK     = S_MSG|S_CTL|S_INP,
	};

	nscsi_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	void connect_to_bus(nscsi_bus_device *bus, int refid, int default_scsi_id);
	virtual void scsi_ctrl_changed();
protected:
	int scsi_id;
	int scsi_refid;
	nscsi_bus_device *scsi_bus;
};

class nscsi_full_device : public nscsi_device
{
public:
	nscsi_full_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	virtual void scsi_ctrl_changed();
protected:
	enum { SCSI_TIMER = 100 };

	// SCSI status returns
	enum {
		SS_GOOD              = 0x00,
		SS_CHECK_CONDITION   = 0x02,
		SS_CONDITION_MET     = 0x04,
		SS_BUSY              = 0x08,
		SS_INT_GOOD          = 0x10,
		SS_INT_CONDITION_MET = 0x14,
		SS_RESV_CONFLICT     = 0x18,
		SS_TERMINATED        = 0x22,
		SS_QUEUE_FULL        = 0x28,
	};

	// SCSI commands
	enum {
		SC_TEST_UNIT_READY   = 0x00,
		SC_REZERO            = 0x01,
		SC_REQUEST_SENSE     = 0x03,
		SC_FORMAT_UNIT       = 0x04,
		SC_REASSIGN_BLOCKS   = 0x07,
		SC_READ              = 0x08,
		SC_WRITE             = 0x0a,
		SC_SEEK              = 0x0b,
		SC_INQUIRY           = 0x12,
		SC_MODE_SELECT_6     = 0x15,
		SC_RESERVE_6         = 0x16,
		SC_RELEASE_6         = 0x17,
		SC_MODE_SENSE_6      = 0x1a,
		SC_START_STOP_UNIT   = 0x1b,
		SC_RECIEVE_DIAG_RES  = 0x1c,
		SC_SEND_DIAGNOSTICS  = 0x1d,
		SC_READ_CAPACITY     = 0x25,
		SC_READ_EXTENDED     = 0x28,
		SC_WRITE_EXTENDED    = 0x2a,
		SC_SEEK_EXTENDED     = 0x2b,
		SC_WRITE_VERIFY      = 0x2e,
		SC_VERIFY            = 0x2f,
		SC_SYNC_CACHE        = 0x35,
		SC_READ_DEFECT_DATA  = 0x37,
		SC_READ_DATA_BUFFER  = 0x3c,
		SC_READ_LONG         = 0x3e,
		SC_WRITE_LONG        = 0x3f,
		SC_CHANGE_DEFINITION = 0x40,
		SC_LOG_SELECT        = 0x4c,
		SC_LOG_SENSE         = 0x4d,
		SC_MODE_SELECT_10    = 0x55,
		SC_RESERVE_10        = 0x56,
		SC_RELEASE_10        = 0x57,
		SC_MODE_SENSE_10     = 0x5a,
	};

	// SCSI Messages
	enum {
		SM_COMMAND_COMPLETE  = 0x00,
		SM_EXTENDED_MSG      = 0x01,
		SM_SAVE_DATA_PTR     = 0x02,
		SM_RESTORE_PTR       = 0x03,
		SM_DISCONNECT        = 0x04,
		SM_INITIATOR_ERROR   = 0x05,
		SM_ABORT             = 0x06,
		SM_MSG_REJECT        = 0x07,
		SM_NOP               = 0x08,
		SM_MSG_PARITY        = 0x09,
		SM_LCMD_COMPLETE     = 0x0a,
		SM_LCMD_COMPLETE_F   = 0x0b,
		SM_BUS_DEVICE_RESET  = 0x0c,
		SM_ABORT_TAG         = 0x0d,
		SM_CLEAR_QUEUE       = 0x0e,
		SM_INIT_RECOVERY     = 0x0f,
		SM_RELEASE_RECOVERY  = 0x10,
		SM_TERMINATE_IO      = 0x11,
		SM_SIMPLE_QUEUE      = 0x20,
		SM_HEAD_QUEUE        = 0x21,
		SM_ORDERED_QUEUE     = 0x22,
		SM_IGNORE_WIDE_RES   = 0x23,
	};

	enum {
		SBUF_MAIN,
		SBUF_SENSE,
	};

	UINT8 scsi_cmdbuf[4096], scsi_sense_buffer[8];
	int scsi_cmdsize;
	UINT8 scsi_identify;

	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

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
		IDLE,
	};

	enum {
		TARGET_SELECT_WAIT_BUS_SETTLE = 1,
		TARGET_SELECT_WAIT_SEL_0,

		TARGET_NEXT_CONTROL,
		TARGET_WAIT_MSG_BYTE,
		TARGET_WAIT_CMD_BYTE,
		TARGET_WAIT_DATA_IN_BYTE,
		TARGET_WAIT_DATA_OUT_BYTE,
	};

	enum {
		RECV_BYTE_T_WAIT_ACK_0 = 1,
		RECV_BYTE_T_WAIT_ACK_1,
		SEND_BYTE_T_WAIT_ACK_0,
		SEND_BYTE_T_WAIT_ACK_1,
	};

	enum {
		STATE_MASK = 0x00ff,
		SUB_SHIFT  = 8,
		SUB_MASK   = 0xff00,
	};

	enum {
		BC_MSG_OR_COMMAND,
		BC_STATUS,
		BC_MESSAGE_1,
		BC_MESSAGE_2,
		BC_DATA_IN,
		BC_DATA_OUT,
		BC_BUS_FREE,
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

