/*
 * mb89352.h
 *
 *  Created on: 16/01/2011
 */

#ifndef MB89352_H_
#define MB89352_H_

#include "machine/scsihle.h"

// SCSI lines readable via PSNS register (reg 5)
#define MB89352_LINE_REQ 0x80
#define MB89352_LINE_ACK 0x40
#define MB89352_LINE_ATN 0x20
#define MB89352_LINE_SEL 0x10
#define MB89352_LINE_BSY 0x08
#define MB89352_LINE_MSG 0x04
#define MB89352_LINE_CD  0x02
#define MB89352_LINE_IO  0x01

// INTS bits
#define INTS_RESET            0x01
#define INTS_HARD_ERROR       0x02
#define INTS_TIMEOUT          0x04
#define INTS_SERVICE_REQUIRED 0x08
#define INTS_COMMAND_COMPLETE 0x10
#define INTS_DISCONNECTED     0x20
#define INTS_RESELECTION      0x40
#define INTS_SELECTION        0x80

// SSTS status bits
#define SSTS_DREG_EMPTY       0x01
#define SSTS_DREG_FULL        0x02
#define SSTS_TC_ZERO          0x04
#define SSTS_SCSI_RST         0x08
#define SSTS_XFER_IN_PROGRESS 0x10
#define SSTS_SPC_BSY          0x20
#define SSTS_TARG_CONNECTED   0x40
#define SSTS_INIT_CONNECTED   0x80

// SERR error status bits
#define SERR_OFFSET     0x01
#define SERR_SHORT_XFR  0x02
#define SERR_PHASE_ERR  0x04
#define SERR_TC_PAR     0x08
#define SERR_SPC_PAR    0x40
#define SERR_SCSI_PAR   0x80


struct mb89352_interface
{
	devcb_write_line irq_callback;  /* irq callback */
	devcb_write_line drq_callback;  /* drq callback */
};

#define MCFG_MB89352A_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MB89352A, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

class mb89352_device : public device_t,
						public mb89352_interface
{
public:
	// construction/destruction
	mb89352_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// any publically accessible interfaces needed for runtime
	DECLARE_READ8_MEMBER( mb89352_r );
	DECLARE_WRITE8_MEMBER( mb89352_w );

	void set_phase(int phase);
	int get_phase(void);

protected:
	// device-level overrides (none are required, but these are common)
	virtual void device_start();
	virtual void device_reset();
	virtual void device_stop();
	virtual void device_config_complete();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// internal device state goes here
	static const device_timer_id TIMER_TRANSFER = 0;

	int get_scsi_cmd_len(UINT8 cbyte);
	//void set_ints(UINT8 flag);

	devcb_resolved_write_line m_irq_func;
	devcb_resolved_write_line m_drq_func;

	scsihle_device* m_SCSIdevices[8];

	UINT8 m_phase;  // current SCSI phase
	UINT8 m_target; // current SCSI target
	UINT8 m_bdid;  // Bus device ID (SCSI ID of the bus?)
	UINT8 m_ints;  // Interrupt Sense
	UINT8 m_temp;  // Temporary register (To/From SCSI bus)
	UINT8 m_data;  // Data register
	UINT8 m_scmd;  // SPC Command register
	UINT32 m_transfer_count;  // byte transfer counter, also used as a timeout counter for selection.
	UINT8 m_int_enable;
	UINT8 m_sel_enable;
	UINT8 m_resel_enable;
	UINT8 m_parity_enable;
	UINT8 m_arbit_enable;
	UINT8 m_busfree_int_enable;
	UINT8 m_line_status;
	UINT8 m_spc_status;
	UINT8 m_error_status;
	UINT8 m_command_index;
	UINT8 m_command[16];
	UINT32 m_transfer_index;
	int m_result_length;
	UINT8 m_buffer[512];

	emu_timer* m_transfer_timer;
};

extern const device_type MB89352A;

#endif /* MB89352_H_ */
