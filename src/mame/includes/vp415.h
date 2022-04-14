// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Skeleton driver for Philips VP415 LV ROM Player

    List of Modules:
        A - Audio Processor
        B - RGB
        C - Video Processor
        D - Ref Source
        E - Slide Drive
        F - Motor+Sequence
        G - Gen Lock
        H - ETBC B
        I - ETBC C
        J - Focus
        K - HF Processor
        L - Video Dropout Correction
        M - Radial
        N - Display Keyboard
        P - Front Loader
        Q - RC5 Mirror
        R - Drive Processor
        S - Control
        T - Supply
        U - Analog I/O
        V - Module Carrier
        W - CPU Datagrabber
        X - LV ROM
        Y - Vid Mix
        Z - Deck Electronics

    TODO:
    - Driver currently fails the initial self-test with code 073. Per
      the service manual, code 73 means "a/d converted mirror pos. min.
      (out of field of view)".

***************************************************************************/

#ifndef MAME_INCLUDES_VP415_H
#define MAME_INCLUDES_VP415_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"

#include "machine/i8155.h"
#include "machine/i8255.h"
#include "machine/ncr5385.h"
#include "machine/saa1043.h"

#include "video/mb88303.h"

#include "screen.h"


class vp415_state : public driver_device
{
public:
	vp415_state(const machine_config &mconfig, device_type type, const char *tag);

	void vp415(machine_config &config);

	static const char *const DATACPU_TAG;
	static const char *const DATAMCU_TAG;

	static const char *const DESCRAMBLE_ROM_TAG;
	static const char *const SYNC_ROM_TAG;
	static const char *const DRIVE_ROM_TAG;

	static const char *const CTRLMCU_TAG;
	static const char *const CONTROL_ROM_TAG;

	static const char *const SWITCHES_TAG;

private:
	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void sel34_w(uint8_t data);
	uint8_t sel37_r();

	DECLARE_WRITE_LINE_MEMBER(cpu_int1_w);

	void data_mcu_port1_w(uint8_t data);
	uint8_t data_mcu_port1_r();
	void data_mcu_port2_w(uint8_t data);
	uint8_t data_mcu_port2_r();

	void ctrl_regs_w(offs_t offset, uint8_t data);
	uint8_t ctrl_regs_r(offs_t offset);
	void ctrl_cpu_port1_w(uint8_t data);
	uint8_t ctrl_cpu_port1_r();
	void ctrl_cpu_port3_w(uint8_t data);
	uint8_t ctrl_cpu_port3_r();

	void ctrl_mcu_port1_w(uint8_t data);
	uint8_t ctrl_mcu_port1_r();
	void ctrl_mcu_port2_w(uint8_t data);
	uint8_t ctrl_mcu_port2_r();

	uint8_t drive_i8155_pb_r();
	uint8_t drive_i8155_pc_r();

	void drive_i8255_pa_w(uint8_t data);
	void drive_i8255_pb_w(uint8_t data);
	uint8_t drive_i8255_pc_r();
	void drive_cpu_port1_w(uint8_t data);
	void drive_cpu_port3_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(refv_w);

	// CPU Board enums
	enum
	{
		SEL34_INTR_N = 0x01,
		SEL34_RES    = 0x20,
		SEL34_ERD    = 0x40,
		SEL34_ENW    = 0x80,
		SEL34_INTR_N_BIT = 0,
		SEL34_RES_BIT    = 5,
		SEL34_ERD_BIT    = 6,
		SEL34_ENW_BIT    = 7,
	};

	enum
	{
		SEL37_ID0   = 0x01,
		SEL37_ID1   = 0x02,
		SEL37_BRD   = 0x10,
		SEL37_MON_N = 0x20,
		SEL37_SK1c  = 0x40,
		SEL37_SK1d  = 0x40,

		SEL37_ID0_BIT   = 0,
		SEL37_ID1_BIT   = 1,
		SEL37_BRD_BIT   = 4,
		SEL37_MON_N_BIT = 5,
		SEL37_SK1c_BIT  = 6,
		SEL37_SK1d_BIT  = 7,
	};

	// Control Board enums
	enum
	{
		CTRL_P3_INT1 = 0x08,

		CTRL_P3_INT1_BIT = 3
	};

	// Drive Board enums
	enum
	{
		I8255PC_NOT_FOCUSED     = 0x02,
		I8255PC_0RPM_N          = 0x08,
		I8255PC_DISC_REFLECTION = 0x10,
	};

	enum
	{
		I8255PB_COMM1    = 0x01,
		I8255PB_COMM2    = 0x02,
		I8255PB_COMM3    = 0x04,
		I8255PB_COMM4    = 0x08,
		I8255PB_RLS_N    = 0x10,
		I8255PB_SL_PWR   = 0x20,
		I8255PB_RAD_FS_N = 0x40,
		I8255PB_STR1     = 0x80,

		I8255PB_COMM1_BIT    = 0,
		I8255PB_COMM2_BIT    = 1,
		I8255PB_COMM3_BIT    = 2,
		I8255PB_COMM4_BIT    = 3,
		I8255PB_RLS_N_BIT    = 4,
		I8255PB_SL_PWR_BIT   = 5,
		I8255PB_RAD_FS_N_BIT = 6,
		I8255PB_STR1_BIT     = 7,
	};

	enum
	{
		I8155PB_2PPR    = 0x01,
		I8155PB_RAD_MIR = 0x04,
		I8155PB_FRLOCK  = 0x08,

		I8155PB_2PPR_BIT    = 0,
		I8155PB_RAD_MIR_BIT = 2,
		I8155PB_FRLOCK_BIT  = 3,
	};

	enum
	{
		DRIVE_P1_CP1    = 0x01,
		DRIVE_P1_CP2    = 0x02,
		DRIVE_P1_LDI    = 0x04,
		DRIVE_P1_ATN_N  = 0x08,
		DRIVE_P1_TX     = 0x10,
		DRIVE_P1_STB_N  = 0x20,
		DRIVE_P1_STR0_N = 0x40,
		DRIVE_P1_TP2    = 0x80,

		DRIVE_P1_CP1_BIT    = 0,
		DRIVE_P1_CP2_BIT    = 1,
		DRIVE_P1_LDI_BIT    = 2,
		DRIVE_P1_ATN_N_BIT  = 3,
		DRIVE_P1_TX_BIT     = 4,
		DRIVE_P1_STB_N_BIT  = 5,
		DRIVE_P1_STR0_N_BIT = 6,
		DRIVE_P1_TP2_BIT    = 7
	};

	virtual void video_start() override;

	void z80_program_map(address_map &map);
	void z80_io_map(address_map &map);
	void datamcu_program_map(address_map &map);
	void set_int_line(uint8_t line, uint8_t value);
	void update_cpu_int();

	void ctrl_program_map(address_map &map);
	void ctrl_io_map(address_map &map);
	void ctrlmcu_program_map(address_map &map);
	void sd_w(uint8_t data);
	uint8_t sd_r();

	void drive_program_map(address_map &map);
	void drive_io_map(address_map &map);

	required_device<z80_device> m_datacpu;
	required_device<i8041a_device> m_datamcu;
	required_device<ncr5385_device> m_scsi;
	required_device<i8031_device> m_drivecpu;
	required_device<i8031_device> m_ctrlcpu;
	required_device<i8041a_device> m_ctrlmcu;
	required_device<mb88303_device> m_chargen;
	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_ctrlram;
	required_ioport m_switches;

	uint8_t m_sel34 = 0;
	uint8_t m_sel37 = 0;

	uint8_t m_int_lines[2]{};

	uint8_t m_refv = 0;

	uint8_t m_ctrl_cpu_p1 = 0;
	uint8_t m_ctrl_cpu_p3 = 0;
	uint8_t m_ctrl_mcu_p1 = 0;
	uint8_t m_ctrl_mcu_p2 = 0;

	uint8_t m_drive_p1 = 0;
	uint8_t m_drive_pc_bits = 0;

	uint8_t m_drive_rad_mir_dac = 0;
	uint8_t m_drive_i8255_pb = 0;
	emu_timer *m_drive_2ppr_timer = nullptr;
	uint8_t m_drive_2ppr = 0;

	static const char *const DATARAM_TAG;
	static const char *const SCSI_TAG;

	static const char *const CTRLCPU_TAG;
	static const char *const CTRLRAM_TAG;

	static const char *const DRIVECPU_TAG;
	static const char *const I8155_TAG;
	static const char *const I8255_TAG;
	static const char *const CHARGEN_TAG;
	static const char *const SYNCGEN_TAG;

	static const device_timer_id DRIVE_2PPR_ID;
};

#endif // MAME_INCLUDES_VP415_H
