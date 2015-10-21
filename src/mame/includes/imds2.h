// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// Driver for Intel Intellec MDS series-II

#ifndef _IMDS2_H_
#define _IMDS2_H_

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8257.h"
#include "video/i8275.h"
#include "sound/beep.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/i8271.h"
#include "imagedev/flopdrv.h"
#include "bus/centronics/ctronics.h"

class imds2_state : public driver_device
{
	public:
	imds2_state(const machine_config &mconfig, device_type type, const char *tag);

	DECLARE_READ8_MEMBER(ipc_mem_read);
	DECLARE_WRITE8_MEMBER(ipc_mem_write);
	DECLARE_WRITE8_MEMBER(imds2_ipc_control_w);
	DECLARE_WRITE_LINE_MEMBER(imds2_ipc_intr);
	DECLARE_READ8_MEMBER(imds2_ipcsyspic_r);
	DECLARE_READ8_MEMBER(imds2_ipclocpic_r);
	DECLARE_WRITE8_MEMBER(imds2_ipcsyspic_w);
	DECLARE_WRITE8_MEMBER(imds2_ipclocpic_w);

	DECLARE_WRITE8_MEMBER(imds2_miscout_w);
	DECLARE_READ8_MEMBER(imds2_miscin_r);
	DECLARE_WRITE_LINE_MEMBER(imds2_beep_timer_w);
	DECLARE_WRITE8_MEMBER(imds2_start_timer_w);
	DECLARE_READ8_MEMBER(imds2_kb_read);
	DECLARE_READ8_MEMBER(imds2_kb_port_p2_r);
	DECLARE_WRITE8_MEMBER(imds2_kb_port_p1_w);
	DECLARE_READ8_MEMBER(imds2_kb_port_t0_r);
	DECLARE_READ8_MEMBER(imds2_kb_port_t1_r);
	DECLARE_WRITE8_MEMBER(imds2_ioc_dbbout_w);
	DECLARE_WRITE8_MEMBER(imds2_ioc_f0_w);
	DECLARE_WRITE8_MEMBER(imds2_ioc_set_f1_w);
	DECLARE_WRITE8_MEMBER(imds2_ioc_reset_f1_w);
	DECLARE_READ8_MEMBER(imds2_ioc_status_r);
	DECLARE_READ8_MEMBER(imds2_ioc_dbbin_r);
	DECLARE_READ8_MEMBER(imds2_ipc_dbbout_r);
	DECLARE_READ8_MEMBER(imds2_ipc_status_r);
	DECLARE_WRITE8_MEMBER(imds2_ipc_dbbin_data_w);
	DECLARE_WRITE8_MEMBER(imds2_ipc_dbbin_cmd_w);
	DECLARE_WRITE_LINE_MEMBER(imds2_hrq_w);

	DECLARE_READ8_MEMBER(imds2_ioc_mem_r);
	DECLARE_WRITE8_MEMBER(imds2_ioc_mem_w);
	DECLARE_READ8_MEMBER(imds2_pio_port_p1_r);
	DECLARE_WRITE8_MEMBER(imds2_pio_port_p1_w);
	DECLARE_READ8_MEMBER(imds2_pio_port_p2_r);
	DECLARE_WRITE8_MEMBER(imds2_pio_port_p2_w);
	DECLARE_WRITE_LINE_MEMBER(imds2_pio_lpt_ack_w);
	DECLARE_WRITE_LINE_MEMBER(imds2_pio_lpt_busy_w);
	DECLARE_WRITE_LINE_MEMBER(imds2_pio_lpt_select_w);

	I8275_DRAW_CHARACTER_MEMBER(crtc_display_pixels);

	virtual void driver_start();
	virtual void machine_start();
	virtual void video_start();
	virtual void machine_reset();

	private:
	required_device<i8085a_cpu_device> m_ipccpu;
	required_device<pic8259_device> m_ipcsyspic;
	required_device<pic8259_device> m_ipclocpic;
	required_device<i8080a_cpu_device> m_ioccpu;
	required_device<i8257_device> m_iocdma;
	required_device<i8275_device> m_ioccrtc;
	required_device<beep_device> m_iocbeep;
	required_device<pit8253_device> m_ioctimer;
	required_device<i8271_device> m_iocfdc;
	required_device<floppy_connector> m_flop0;
	required_device<i8041_device> m_iocpio;
	required_device<i8741_device> m_kbcpu;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<centronics_device> m_centronics;
	required_ioport m_io_key0;
	required_ioport m_io_key1;
	required_ioport m_io_key2;
	required_ioport m_io_key3;
	required_ioport m_io_key4;
	required_ioport m_io_key5;
	required_ioport m_io_key6;
	required_ioport m_io_key7;
	required_ioport m_ioc_options;

	std::vector<UINT8> m_ipc_ram;

	bool imds2_in_ipc_rom(offs_t offset) const;

	void imds2_update_beeper(void);
	void imds2_update_printer(void);

	// IPC control port
	UINT8 m_ipc_control;

	// IPC ROM content
	const UINT8 *m_ipc_rom;

	// Character generator
	const UINT8 *m_chargen;

	// MISCOUT state
	UINT8 m_miscout;

	// Beeper timer line
	int m_beeper_timer;

	// Keyboard state
	UINT8 m_kb_p1;

	// IPC to IOC buffer
	UINT8 m_ioc_ibf;

	// IOC to IPC buffer
	UINT8 m_ioc_obf;

	// IPC/IOC status
	UINT8 m_ipc_ioc_status;

	// PIO port 1
	UINT8 m_pio_port1;

	// PIO port 2
	UINT8 m_pio_port2;

	// PIO device status byte
	UINT8 m_device_status_byte;
};

#endif /* _IMDS2_H_ */
