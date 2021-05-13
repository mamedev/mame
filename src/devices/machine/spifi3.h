// license:BSD-3-Clause
// copyright-holders:Brice Onken,Tsubai Masanari
// thanks-to:Patrick Mackinlay

/*
 * HP 1TV3-0302 SPIFI3-SE SCSI controller
 *
 * This is just a skeleton that logs accesses to inspect SCSI transactions.
 * Datasheets for this seem to be impossible to find - the only avaliable implementation to reference that I have
 * found is the Sony NEWS APBus NetBSD driver. Hopefully a datasheet will turn up eventually.
 * Based on internet research, it seems some HP PA-RISC systems also used the SPIFI3, including the E55.
 *
 * Register definitions were derived from the NetBSD source code, copyright (c) 2000 Tsubai Masanari.
 *
 * References:
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifireg.h
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifi.c
 */

#ifndef MAME_MACHINE_SPIFI3_H
#define MAME_MACHINE_SPIFI3_H

#pragma once

#include "machine/nscsi_bus.h"

class spifi3_device
	: public nscsi_device
	, public nscsi_slot_card_interface
{
public:
	spifi3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
	void map(address_map &map);

private:
	// AUXCTRL constants and functions
	const uint32_t AUXCTRL_DMAEDGE = 0x04;
	const uint32_t AUXCTRL_SETRST = 0x20;
	const uint32_t AUXCTRL_CRST = 0x40;
	const uint32_t AUXCTRL_SRST = 0x80;
	uint32_t auxctrl_r();
	void auxctrl_w(uint32_t data);

	// FIFOCTRL constants and functions
	const uint32_t FIFOC_FSLOT = 0x0f; // Free slots in FIFO - max 8. Free slots = 8 - (FIFOCTRL & FIFOC_FSLOT) */
	const uint32_t FIFOC_SSTKACT = 0x10;
	const uint32_t FIFOC_RQOVRN = 0x20;
	const uint32_t FIFOC_CLREVEN = 0x00;
	const uint32_t FIFOC_CLRODD = 0x40;
	const uint32_t FIFOC_FLUSH = 0x80;
	const uint32_t FIFOC_LOAD = 0xc0;
	uint32_t fifoctrl_r();
	void fifoctrl_w(uint32_t data);

	// Command buffer constants and functions
	uint8_t cmd_buf_r(offs_t offset);
	void cmd_buf_w(offs_t offset, uint8_t data);

	struct spifi_cmd_entry
	{
		// NetBSD has these mapped as uint32_t to align the accesses and such
		// in reality, these are all 8-bit values that are mapped, in typical NWS-5000 series
		// fashion, to be 32-bit word aligned.
		// the same probably applies to the register file.
		uint8_t cdb[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		uint8_t quecode = 0;
		uint8_t quetag = 0;
		uint8_t idmsg = 0;
		uint8_t status = 0;
	};

	struct register_file
	{
/*00*/	uint32_t spstat = 0;
		uint32_t cmlen = 0;
		uint32_t cmdpage = 0;
		uint32_t count_hi = 0;

/*10*/	uint32_t count_mid = 0;
		uint32_t count_low = 0;
		uint32_t svptr_hi = 0;
		uint32_t svptr_mid = 0;

/*20*/	uint32_t svptr_low = 0;
		uint32_t intr = 0;
		uint32_t imask = 0;
		uint32_t prctrl = 0;

/*30*/	uint32_t prstat = 0;
		uint32_t init_status = 0;
		uint32_t fifoctrl = 0;
		uint32_t fifodata = 0;

/*40*/	uint32_t config = 0;
		uint32_t data_xfer = 0;
		uint32_t autocmd = 0;
		uint32_t autostat = 0;

/*50*/	uint32_t resel = 0;
		uint32_t select = 0;
		uint32_t prcmd = 0;
		uint32_t auxctrl = 0;

/*60*/  uint32_t autodata = 0;
		uint32_t loopctrl = 0;
		uint32_t loopdata = 0;
		uint32_t identify = 0;

/*70*/  uint32_t complete = 0;
		uint32_t scsi_status = 0x1; // MROM reads this to check if the SPIFI is alive at system boot, so the WO description from NetBSD might be wrong.
		uint32_t data = 0;
		uint32_t icond = 0;

/*80*/	uint32_t fastwide = 0;
		uint32_t exctrl = 0;
		uint32_t exstat = 0;
		uint32_t test = 0;

/*90*/	uint32_t quematch = 0;
		uint32_t quecode = 0;
		uint32_t quetag = 0;
		uint32_t quepage = 0;

		// uint32_t image[88]; // mirror of the previous values
		spifi_cmd_entry cmbuf[8];
	} spifi_reg;
};

DECLARE_DEVICE_TYPE(SPIFI3, spifi3_device)

#endif // MAME_MACHINE_SPIFI3_H
