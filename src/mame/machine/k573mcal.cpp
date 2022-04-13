// license:BSD-3-Clause
// copyright-holders:windyfairy
/*
 * Konami 573 Master Calendar
 *
 * This device was made for development/factory use only.
 * It will override the game when connected and always boot into the master calendar-specific code.
 * Every game has a master calendar-specific boot sequence but a few that don't have code to initialize security cartridges.
 *
 * The only games that this does not work on that have a security cart are ddr2mc2, ddr2ml, and ddr2mla (all variants of 885jaa02).
 * Those games will boot into a screen that shows the game code, clock, and date with nothing else.
 * For these games it's possible to set Sys573 DIPSW 1 with the master calendar connected and it will do a checksum of the game's data
 * and attempt to write it to "c:/tmp/chksum.dat" on the host debugger PC but will crash in MAME.
 *
 * DIPSW 2 and 3 on the System 573 directly are also used to specify the "spec" of the game.
 * For example, setting DIPSW allows you to switch between GN and GE specs in earlier games.
 *
 * Some games require you to hold service/F2 (and set Sys573 DIPSW 3?) during boot to initialize the installation cartridge.
 *
 */

#include "emu.h"
#include "k573mcal.h"

#include "machine/timehelp.h"

k573mcal_device::k573mcal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jvs_device(mconfig, KONAMI_573_MASTER_CALENDAR, tag, owner, clock),
	m_in1(*this, "IN1"),
	seconds(0),
	mainId(0),
	subId(0)
{
}

void k573mcal_device::device_start()
{
	jvs_device::device_start();
}

void k573mcal_device::device_reset()
{
	seconds = 0;

	// Randomly picked values
	// Is rendered in-game as x41-6379 where the x is generated based on the value of the main ID
	mainId = 41; // valid range 0 - 99
	subId = 6379; // valid range 0 - 9999

	jvs_device::device_reset();
}

const char *k573mcal_device::device_id()
{
	return "KONAMI CO.,LTD.;Master Calendar;Ver1.0;";
}

uint8_t k573mcal_device::command_format_version()
{
	return 0x11;
}

uint8_t k573mcal_device::jvs_standard_version()
{
	return 0x20;
}

uint8_t k573mcal_device::comm_method_version()
{
	return 0x10;
}

int k573mcal_device::handle_message(const uint8_t* send_buffer, uint32_t send_size, uint8_t*& recv_buffer)
{
	switch (send_buffer[0]) {
	case 0xf0:
		// msg: f0 d9
		device_reset();
		break;

	case 0x70: {
		// msg: 70
		// Writes to RTC chip

		system_time systime;
		machine().base_datetime(systime);

		uint8_t resp[] = {
			0x01, // status, must be 1
			uint8_t(systime.local_time.year % 100),
			uint8_t(systime.local_time.month + 1),
			systime.local_time.mday,
			systime.local_time.weekday,
			systime.local_time.hour,
			systime.local_time.minute,
			seconds // Can't be the same value twice in a row
		};

		seconds = (seconds + 1) % 60;

		memcpy(recv_buffer, resp, sizeof(resp));
		recv_buffer += sizeof(resp);
		return 1;
	}

	case 0x71: {
		// msg: 71 ff ff 01

		uint8_t resp[] = {
			0x01, // status, must be 1
			uint8_t(m_in1->read() & 0x0f), // Area specification
		};

		memcpy(recv_buffer, resp, sizeof(resp));
		recv_buffer += sizeof(resp);
		return 4;
	}

	case 0x7c: {
		// msg: 7c 7f 00 04
		const uint16_t val = (send_buffer[1] << 8) | send_buffer[2];

		if (val == 0x7f00) {
			// Return main ID
			uint8_t resp[] = {
				0x01, // status, must be 1
				uint8_t((mainId >> 24) & 0xff), uint8_t((mainId >> 16) & 0xff), uint8_t((mainId >> 8) & 0xff), uint8_t(mainId & 0xff),
			};

			memcpy(recv_buffer, resp, sizeof(resp));
			recv_buffer += sizeof(resp);
		}
		else if (val == 0x8000) {
			// Return sub ID
			uint8_t resp[] = {
				0x01, // status, must be 1
				'<', 'I', 'N', 'I', 'T', ' ', 'C', 'O', 'M', 'P', 'L', 'E', 'T', 'E', '!', '>',
				uint8_t((subId >> 24) & 0xff), uint8_t((subId >> 16) & 0xff), uint8_t((subId >> 8) & 0xff), uint8_t(subId & 0xff),
				uint8_t((~subId >> 24) & 0xff), uint8_t((~subId >> 16) & 0xff), uint8_t((~subId >> 8) & 0xff), uint8_t(~subId & 0xff),
			};

			memcpy(recv_buffer, resp, sizeof(resp));
			recv_buffer += sizeof(resp);
		}

		return 4;
	}

	case 0x7d: {
		// msg: 7d 80 10 08 00 00 00 01 ff ff ff fe
		const uint16_t val = (send_buffer[1] << 8) | send_buffer[2];

		if (val == 0x8010) {
			// Set next sub ID
			subId = (send_buffer[4] << 24) | (send_buffer[5] << 16) | (send_buffer[6] << 8) | send_buffer[7];

			uint8_t resp[] = {
				0x01, // status, must be 1
			};

			memcpy(recv_buffer, resp, sizeof(resp));
			recv_buffer += sizeof(resp);
		}

		return 12;
	}

	case 0x7e: {
		// This builds some buffer that creates data like this: @2B0001:020304050607:BC9A78563412:000000000000B5
		// 2B0001 is ???
		// 020304050607 is the machine SID
		// BC9A78563412 is the machine XID
		// 000000000000B5 is ???

		// msg: 7e xx
		uint8_t resp[] = {
			// 0x01 - Breaks loop, sends next byte
			// 0x04 - Resends byte
			0x01,
		};

		memcpy(recv_buffer, resp, sizeof(resp));
		recv_buffer += sizeof(resp);

		return 2;
	}

	case 0x7f:
		// TODO: Where is this used?
		// The command existed in the command list for a few games (starting at Drummania?) but was never referenced and then disappeared around DDR 3rd mix.
		break;
	}

	// Command not recognized, pass it off to the base message handler
	return jvs_device::handle_message(send_buffer, send_size, recv_buffer);
}

INPUT_PORTS_START( k573mcal )
	PORT_START("IN1")
	// Default the area to 3 because it's unused and will force you to actively select the region to initialize.
	// For all but the earliest games it will show a message saying "this game only supports regions x, y, z".
	// This is also a good way to discover new bootable variants that exist on the disc but were previously unknown.
	PORT_DIPNAME(0x0f, 0x03, "Area")
	PORT_DIPSETTING(0x00, "JA")
	PORT_DIPSETTING(0x01, "UA")
	PORT_DIPSETTING(0x02, "EA")
	PORT_DIPSETTING(0x03, "3") // Unused
	PORT_DIPSETTING(0x04, "AA")
	PORT_DIPSETTING(0x05, "KA")
	PORT_DIPSETTING(0x06, "JY/AY")
	PORT_DIPSETTING(0x07, "JR")
	PORT_DIPSETTING(0x08, "JB")
	PORT_DIPSETTING(0x09, "UB")
	PORT_DIPSETTING(0x0a, "EB")
	PORT_DIPSETTING(0x0b, "11") // Unused
	PORT_DIPSETTING(0x0c, "AB")
	PORT_DIPSETTING(0x0d, "KB")
	PORT_DIPSETTING(0x0e, "JZ/AZ")
	PORT_DIPSETTING(0x0f, "JS")
INPUT_PORTS_END

ioport_constructor k573mcal_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(k573mcal);
}

DEFINE_DEVICE_TYPE(KONAMI_573_MASTER_CALENDAR, k573mcal_device, "k573mcal", "Konami 573 Master Calendar")
