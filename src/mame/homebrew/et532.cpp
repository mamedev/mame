// license:BSD-3-Clause
// copyright-holders:Dave Rand

/*
 * George Scolaro's ET532 — an Ethernet/serial variant of the pc532 (532 Baby AT),
 * here as a STAND-ALONE system (the J4-jumpered "not in a K532 system" mode).
 *
 * The whole board is the shared et532_board_device (bus/nscsi/et532.{h,cpp}) —
 * NS32532 cluster, 8MB DRAM, EPROM, COPS MAC EEPROM, two SCC2698 octal UARTs (16
 * serial lines), DP8390 Ethernet, and a DP8490 SCSI with CPU-driven pseudo-DMA.
 * This driver wraps that core for stand-alone operation: the board's DP8490 is the
 * SCSI INITIATOR on a private bus carrying disk/tape.  The very same core also
 * drops onto a host bus as the et532_scsi slot card (the in-system 532SC role,
 * board = target); the two are peers — see docs/et532/ET532-SLAVE-LOG.txt.
 *
 * The ET532 was designed (rev 1A, 1989) but never built, so there is no original
 * firmware: the EPROM is a purpose-built monitor written from scratch with the
 * Definicon DSI-32 NS32k toolchain (banner, console echo, examine/deposit/go,
 * console + block-0 SCSI loaders, signature+checksum autoboot).
 *
 * Sources:
 *  - ET532 schematic (PROJECT:ET532, rev 1A, (C) 1988-90 George Scolaro)
 *  - PAL equations: KSER/PALS/{DEC32,DRAMC,DRAMEN,WAIT,COPETH}.TDL
 *  - pc532.cpp (Patrick Mackinlay) — the device-wiring template this is based on
 */

#include "emu.h"

#include "bus/nscsi/et532.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/tape.h"
#include "machine/nscsi_bus.h"

namespace {

class et532_state : public driver_device
{
public:
	et532_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_board(*this, "board")
		, m_scsi(*this, "board:dp8490")
	{
	}

	void et532(machine_config &config) ATTR_COLD;

private:
	required_device<et532_board_device> m_board;
	required_device<dp8490_device> m_scsi;
};

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("tape", NSCSI_TAPE);
}

void et532_state::et532(machine_config &config)
{
	ET532_BOARD(config, m_board, 40_MHz_XTAL);

	// stand-alone (J4): the board's DP8490 is the initiator on a private 532SC bus
	auto &scsi(NSCSI_BUS(config, "scsi"));
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, "tape", false);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr, false);
	scsi.set_external_device(7, m_scsi);
}

ROM_START(et532)
ROM_END

} // anonymous namespace

/*   YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY           FULLNAME                       FLAGS */
COMP(1989, et532, 0,      0,      et532,   0,     et532_state, empty_init, "George Scolaro", "ET532 (NS32532 + Ethernet)",  MACHINE_NO_SOUND_HW)
