// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * mb89352.h
 *
 *  Created on: 16/01/2011
 */

#ifndef MB89352_H_
#define MB89352_H_

#include "legscsi.h"

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


#define MCFG_MB89352A_IRQ_CB(_devcb) \
	devcb = &mb89352_device::set_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_MB89352A_DRQ_CB(_devcb) \
	devcb = &mb89352_device::set_drq_callback(*device, DEVCB_##_devcb);

class mb89352_device : public legacy_scsi_host_adapter
{
public:
	// construction/destruction
	mb89352_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_callback(device_t &device, _Object object) { return downcast<mb89352_device &>(device).m_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_drq_callback(device_t &device, _Object object) { return downcast<mb89352_device &>(device).m_drq_cb.set_callback(object); }

	// any publically accessible interfaces needed for runtime
	DECLARE_READ8_MEMBER( mb89352_r );
	DECLARE_WRITE8_MEMBER( mb89352_w );

	void set_phase(int phase);

protected:
	// device-level overrides (none are required, but these are common)
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// internal device state goes here
	static const device_timer_id TIMER_TRANSFER = 0;

	int get_scsi_cmd_len(UINT8 cbyte);
	//void set_ints(UINT8 flag);

	devcb_write_line m_irq_cb;  /* irq callback */
	devcb_write_line m_drq_cb;  /* drq callback */

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
	UINT8 m_buffer[512];

	emu_timer* m_transfer_timer;
};

extern const device_type MB89352A;

#endif /* MB89352_H_ */
