// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
#pragma once

#ifndef __QL__
#define __QL__

#include "emu.h"
#include "bus/ql/exp.h"
#include "bus/ql/rom.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/microdrv.h"
#include "machine/qimi.h"
#include "machine/ram.h"
#include "machine/zx8302.h"
#include "sound/speaker.h"
#include "video/zx8301.h"

#define SCREEN_TAG  "screen"

#define M68008_TAG  "ic18"
#define I8749_TAG   "ic24"
#define I8051_TAG   "i8051"
#define ZX8301_TAG  "ic22"
#define ZX8302_TAG  "ic23"
#define RS232_A_TAG "ser1"
#define RS232_B_TAG "ser2"
#define QIMI_TAG    "qimi"

#define X1 XTAL_15MHz
#define X2 XTAL_32_768kHz
#define X3 XTAL_4_436MHz
#define X4 XTAL_11MHz

class ql_state : public driver_device
{
public:
	ql_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, M68008_TAG),
		m_ipc(*this, I8749_TAG),
		m_zx8301(*this, ZX8301_TAG),
		m_zx8302(*this, ZX8302_TAG),
		m_speaker(*this, "speaker"),
		m_mdv1(*this, MDV_1),
		m_mdv2(*this, MDV_2),
		m_ser1(*this, RS232_A_TAG),
		m_ser2(*this, RS232_A_TAG),
		m_ram(*this, RAM_TAG),
		m_exp(*this, "exp"),
		m_cart(*this, "rom"),
		m_qimi(*this, QIMI_TAG),
		m_rom(*this, M68008_TAG),
		m_y0(*this, "Y0"),
		m_y1(*this, "Y1"),
		m_y2(*this, "Y2"),
		m_y3(*this, "Y3"),
		m_y4(*this, "Y4"),
		m_y5(*this, "Y5"),
		m_y6(*this, "Y6"),
		m_y7(*this, "Y7"),
		m_joy0(*this, "JOY0"),
		m_joy1(*this, "JOY1"),
		m_config(*this, "config"),
		m_extintl(CLEAR_LINE),
		m_keylatch(0),
		m_ipl(0),
		m_comdata_to_ipc(0),
		m_baudx4(0),
		m_qimi_enabled(false),
		m_qimi_extint(CLEAR_LINE)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_ipc;
	required_device<zx8301_device> m_zx8301;
	required_device<zx8302_device> m_zx8302;
	required_device<speaker_sound_device> m_speaker;
	required_device<microdrive_image_device> m_mdv1;
	required_device<microdrive_image_device> m_mdv2;
	required_device<rs232_port_device> m_ser1;
	required_device<rs232_port_device> m_ser2;
	required_device<ram_device> m_ram;
	required_device<ql_expansion_slot_t> m_exp;
	required_device<ql_rom_cartridge_slot_t> m_cart;
	optional_device<qimi_t> m_qimi;
	required_memory_region m_rom;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_joy0;
	required_ioport m_joy1;
	required_ioport m_config;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE8_MEMBER( ipc_w );
	DECLARE_WRITE8_MEMBER( ipc_port1_w );
	DECLARE_WRITE8_MEMBER( ipc_port2_w );
	DECLARE_READ8_MEMBER( ipc_port2_r );
	DECLARE_READ8_MEMBER( ipc_t1_r );
	DECLARE_READ8_MEMBER( ipc_bus_r );
	DECLARE_READ8_MEMBER( ql_ram_r );
	DECLARE_WRITE8_MEMBER( ql_ram_w );
	DECLARE_WRITE_LINE_MEMBER( ql_baudx4_w );
	DECLARE_WRITE_LINE_MEMBER( ql_comdata_w );
	DECLARE_WRITE_LINE_MEMBER( zx8302_mdselck_w );
	DECLARE_WRITE_LINE_MEMBER( zx8302_mdrdw_w );
	DECLARE_WRITE_LINE_MEMBER( zx8302_erase_w );
	DECLARE_WRITE_LINE_MEMBER( zx8302_raw1_w );
	DECLARE_READ_LINE_MEMBER( zx8302_raw1_r );
	DECLARE_WRITE_LINE_MEMBER( zx8302_raw2_w );
	DECLARE_READ_LINE_MEMBER( zx8302_raw2_r );
	DECLARE_WRITE_LINE_MEMBER( exp_extintl_w );
	DECLARE_WRITE_LINE_MEMBER( qimi_extintl_w );

	void update_interrupt();

	int m_extintl;

	// IPC
	UINT8 m_keylatch;
	int m_ipl;
	int m_comdata_to_ipc;
	int m_baudx4;

	// QIMI
	bool m_qimi_enabled;
	int m_qimi_extint;
};

#endif
