// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_CONITEC_PROF80_H
#define MAME_CONITEC_PROF80_H

#pragma once

#include "bus/ecbbus/ecbbus.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/z80daisy.h"
#include "prof80mmu.h"
#include "machine/74259.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "machine/upd1990a.h"
#include "machine/upd765.h"

#define Z80_TAG         "z1"
#define UPD765_TAG      "z38"
#define UPD1990A_TAG    "z43"

// ------------------------------------------------------------------------

#define UNIO_Z80STI_TAG         "z5"
#define UNIO_Z80SIO_TAG         "z15"
#define UNIO_Z80PIO_TAG         "z13"
#define UNIO_CENTRONICS1_TAG    "n3"
#define UNIO_CENTRONICS2_TAG    "n4"

class prof80_state : public driver_device
{
public:
	prof80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_mmu(*this, "mmu")
		, m_rtc(*this, UPD1990A_TAG)
		, m_fdc(*this, UPD765_TAG)
		, m_ram(*this, RAM_TAG)
		, m_floppy(*this, UPD765_TAG":%u", 0U)
		, m_ecb(*this, "ecbbus")
		, m_rs232a(*this, "rs232a")
		, m_rs232b(*this, "rs232b")
		, m_flra(*this, "z44")
		, m_flrb(*this, "z45")
		, m_rom(*this, Z80_TAG)
		, m_j4(*this, "J4")
		, m_j5(*this, "J5")
	{ }

	void prof80(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(motor_off);

	required_device<cpu_device> m_maincpu;
	required_device<prof80_mmu_device> m_mmu;
	required_device<upd1990a_device> m_rtc;
	required_device<upd765a_device> m_fdc;
	required_device<ram_device> m_ram;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<ecbbus_device> m_ecb;
	required_device<rs232_port_device> m_rs232a;
	required_device<rs232_port_device> m_rs232b;
	required_device<ls259_device> m_flra;
	required_device<ls259_device> m_flrb;
	required_memory_region m_rom;
	required_ioport m_j4;
	required_ioport m_j5;

	void flr_w(uint8_t data);
	uint8_t status_r();
	uint8_t status2_r();

	void motor(int mon);

	void ready_w(int state);
	void inuse_w(int state);
	void motor_w(int state);
	void select_w(int state);
	void mini_w(int state);
	void mstop_w(int state);

	// floppy state
	int m_motor = 0;
	int m_ready = 0;
	int m_select = 0;

	// timers
	emu_timer   *m_floppy_motor_off_timer = nullptr;

	void prof80_io(address_map &map) ATTR_COLD;
	void prof80_mem(address_map &map) ATTR_COLD;
	void prof80_mmu(address_map &map) ATTR_COLD;
};

#endif // MAME_CONITEC_PROF80_H
