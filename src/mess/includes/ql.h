// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __QL__
#define __QL__

#include "bus/rs232/rs232.h"
#include "machine/ram.h"
#include "machine/wd17xx.h"
#include "bus/centronics/ctronics.h"


#define SCREEN_TAG  "screen"

#define M68008_TAG  "ic18"
#define I8749_TAG   "ic24"
#define I8051_TAG   "i8051"
#define ZX8301_TAG  "ic22"
#define ZX8302_TAG  "ic23"
#define WD1772_TAG  "wd1772"
#define RS232_A_TAG "ser1"
#define RS232_B_TAG "ser2"

#define ROMBANK_TAG "rombank"
#define RAMBANK_TAG "rambank"

#define PRINTER_TAG "printer"

#define X1 XTAL_15MHz
#define X2 XTAL_32_768kHz
#define X3 XTAL_4_436MHz
#define X4 XTAL_11MHz

#define QL_CONFIG_PORT          "config"
#define QIMI_PORT_MASK          0x01
#define QIMI_NONE               0x00
#define QIMI_MOUSE              0x01
#define DISK_TYPE_MASK          0x06
#define DISK_TYPE_NONE          0x00
#define DISK_TYPE_TRUMP         0x02
#define DISK_TYPE_SANDY_SD      0x04
#define DISK_TYPE_SANDY_SQB     0x06
#define IS_SANDY_DISK(__dtype__)    ((__dtype__ == DISK_TYPE_SANDY_SD) || (__dtype__ == DISK_TYPE_SANDY_SQB))

#define TRUMP_DRIVE1_MASK       0x01
#define TRUMP_DRIVE0_MASK       0x02
#define TRUMP_MOTOR_MASK        0x04
#define TRUMP_SIDE_SHIFT        3
#define TRUMP_SIDE_MASK         (1 << TRUMP_SIDE_SHIFT)

#define CART_ROM_BASE           0x0c000
#define CART_ROM_END            0x0ffff

#define TRUMP_ROM_MBASE         0x10000
#define TRUMP_ROM_BASE          0x14000
#define TRUMP_ROM_LEN           0x08000
#define TRUMP_ROM_END           (TRUMP_ROM_MBASE+(TRUMP_ROM_LEN-1))

#define TRUMP_IO_BASE           0x1c000
#define TRUMP_IO_LEN            0x04000
#define TRUMP_IO_END            (TRUMP_IO_BASE+(TRUMP_IO_LEN-1))

#define SANDY_ROM_BASE_SD       0x1c000
#define SANDY_ROM_BASE_SQB      0x20000
#define SANDY_IO_BASE           0xc3fc0
#define SANDY_IO_LEN            0x00040
#define SANDY_IO_END            (SANDY_IO_BASE+(SANDY_IO_LEN-1))

#define SANDY_DRIVE0_MASK       0x02
#define SANDY_DRIVE1_MASK       0x04
#define SANDY_MOTOR_MASK        0x08
#define SANDY_SIDE_SHIFT        0
#define SANDY_SIDE_MASK         (1 << SANDY_SIDE_SHIFT)
#define SANDY_DDEN_SHIFT        4
#define SANDY_DDEN_MASK         (1 << SANDY_DDEN_SHIFT)
#define SANDY_PRINTER_STROBE    0x20
#define SANDY_PRINTER_INTMASK   0x40
#define SANDY_MOUSE_INTMASK     0x80

#define MOUSEX_TAG              "MOUSEX"
#define MOUSEY_TAG              "MOUSEY"
#define MOUSEB_TAG              "MOUSEB"

// Mouse bits in Sandy port order
#define MOUSE_MIDDLE            0x02
#define MOUSE_RIGHT             0x04
#define MOUSE_LEFT              0x08
#define MOUSE_DIRY              0x10
#define MOUSE_DIRX              0x20
#define MOUSE_INTY              0x40
#define MOUSE_INTX              0x80
#define MOUSE_INT_MASK          (MOUSE_INTX | MOUSE_INTY)

#define QIMI_IO_BASE            0x1bf9c
#define QIMI_IO_LEN             0x22
#define QIMI_IO_END             (QIMI_IO_BASE + QIMI_IO_LEN )

#define QIMI_INTX               0x04
#define QIMI_INTY               0x20
#define QIMI_DIRX               0x10
#define QIMI_DIRY               0x01
#define QIMI_LEFT               0x20
#define QIMI_RIGHT              0x10
#define QIMI_INT_MASK           (QIMI_INTX | QIMI_INTY)


class ql_state : public driver_device
{
public:
	enum
	{
		TIMER_MOUSE_TICK,
	};

	ql_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
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
			m_fdc(*this, WD1772_TAG),
			m_printer(*this, PRINTER_TAG),
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
			m_config(*this, QL_CONFIG_PORT),
			m_mousex(*this, MOUSEX_TAG),
			m_mousey(*this, MOUSEY_TAG),
			m_mouseb(*this, MOUSEB_TAG)
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
	required_device<wd1772_device> m_fdc;
	required_device<printer_image_device> m_printer;
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
	optional_ioport m_config;
	optional_ioport m_mousex;
	optional_ioport m_mousey;
	optional_ioport m_mouseb;

	virtual void machine_start();
	virtual void machine_reset();

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

	/* IPC state */
	UINT8 m_keylatch;
	int m_ipl;
	int m_comdata_to_ipc;
	int m_baudx4;

	// Trump card & Sandy superdisk
	DECLARE_READ8_MEMBER( disk_io_r );
	DECLARE_WRITE8_MEMBER( disk_io_w );
	DECLARE_READ8_MEMBER( trump_card_rom_r );
	DECLARE_READ8_MEMBER( cart_rom_r );

	void trump_card_set_control(UINT8 data);
	void sandy_set_control(UINT8 data);
	void sandy_print_char(UINT8 data);

	UINT8   m_disk_type;
	int     m_disk_io_base;
	UINT8   m_disk_io_byte;
	UINT8   m_printer_char;
	DECLARE_READ_LINE_MEMBER(disk_io_dden_r);
	DECLARE_WRITE_LINE_MEMBER(disk_io_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(disk_io_drq_w);

	DECLARE_WRITE_LINE_MEMBER( sandy_printer_busy );

	// QIMI or Sandy mouse
	void mouse_tick();

	DECLARE_READ8_MEMBER( qimi_io_r );
	DECLARE_WRITE8_MEMBER( qimi_io_w );

	UINT8   m_mouse_int;

	emu_timer *m_mouse_timer;

	UINT8 m_ql_mouse_x;
	UINT8 m_ql_mouse_y;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

};

#endif
