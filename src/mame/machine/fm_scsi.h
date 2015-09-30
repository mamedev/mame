// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * fm_scsi.h
 *
 * SCSI controller used in Fujitsu FMR-50, FMR-60, and FM-Towns
 *
 */

#ifndef FM_SCSI_H_
#define FM_SCSI_H_

#include "machine/legscsi.h"

// SCSI input lines (from target)
#define FMSCSI_LINE_REQ   0x80
#define FMSCSI_LINE_IO    0x40
#define FMSCSI_LINE_MSG   0x20
#define FMSCSI_LINE_CD    0x10
#define FMSCSI_LINE_BSY   0x08
#define FMSCSI_LINE_EX    0x04
#define FMSCSI_LINE_INT   0x02
#define FMSCSI_LINE_PERR  0x01

// SCSI output lines (to target)
#define FMSCSI_LINE_WEN   0x80
#define FMSCSI_LINE_IMSK  0x40
#define FMSCSI_LINE_RMSK  0x20
#define FMSCSI_LINE_ATN   0x10
#define FMSCSI_LINE_WRD   0x08
#define FMSCSI_LINE_SEL   0x04
#define FMSCSI_LINE_DMAE  0x02
#define FMSCSI_LINE_RST   0x01

#define MCFG_FMSCSI_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, FMSCSI, 0)

#define MCFG_FMSCSI_IRQ_HANDLER(_devcb) \
	devcb = &fmscsi_device::set_irq_handler(*device, DEVCB_##_devcb);
#define MCFG_FMSCSI_DRQ_HANDLER(_devcb) \
	devcb = &fmscsi_device::set_drq_handler(*device, DEVCB_##_devcb);

class fmscsi_device : public legacy_scsi_host_adapter
{
public:
	// construction/destruction
	fmscsi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<fmscsi_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_drq_handler(device_t &device, _Object object) { return downcast<fmscsi_device &>(device).m_drq_handler.set_callback(object); }

	// any publically accessible interfaces needed for runtime
	UINT8 fmscsi_data_r(void);
	void fmscsi_data_w(UINT8 data);
	UINT8 fmscsi_status_r(void);
	void fmscsi_control_w(UINT8 data);
	DECLARE_READ8_MEMBER( fmscsi_r );
	DECLARE_WRITE8_MEMBER( fmscsi_w );

	void set_phase(int phase);
	int get_phase(void);
	void set_input_line(UINT8 line, UINT8 state);
	UINT8 get_input_line(UINT8 line);
	void set_output_line(UINT8 line, UINT8 state);
	UINT8 get_output_line(UINT8 line);

protected:
	// device-level overrides (none are required, but these are common)
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// internal device state goes here
	static const device_timer_id TIMER_TRANSFER = 0;
	static const device_timer_id TIMER_PHASE = 1;

	int get_scsi_cmd_len(UINT8 cbyte);
	void stop_transfer();

	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;

	UINT8 m_command[32];
	//UINT8 m_result[32];
	UINT8 m_command_index;
	int m_result_length;
	UINT32 m_result_index;
	UINT8 m_input_lines;
	UINT8 m_output_lines;
	UINT8 m_data;
	UINT8 m_last_id;
	UINT8 m_phase;
	UINT8 m_target;
	UINT8 m_buffer[512];
	emu_timer* m_transfer_timer;
	emu_timer* m_phase_timer;
};

extern const device_type FMSCSI;

#endif /* FM_SCSI_H_ */
