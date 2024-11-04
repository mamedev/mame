// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont
#ifndef MAME_SEGACONS_DCCONS_H
#define MAME_SEGACONS_DCCONS_H

#pragma once

#include "dc.h"

#include "bus/ata/ataintf.h"
#include "imagedev/cdromimg.h"
#include "machine/intelfsh.h"

class dc_cons_state : public dc_state
{
public:
	dc_cons_state(const machine_config &mconfig, device_type type, const char *tag)
		: dc_state(mconfig, type, tag)
		, m_ata(*this, "ata")
		, m_dcflash(*this, "dcflash")
		, atapi_xfercomplete(0)
	{ }

	required_device<ata_interface_device> m_ata;
	required_device<fujitsu_29lv002tc_device> m_dcflash;

	void init_dc();
	void init_tream();

	uint64_t dc_pdtra_r();
	void dc_pdtra_w(uint64_t data);
	void aica_irq(int state);
	void sh4_aica_irq(int state);
	void ata_interrupt(int state);

	TIMER_CALLBACK_MEMBER( atapi_xfer_end );

	void dreamcast_atapi_init();
	uint32_t dc_mess_g1_ctrl_r(offs_t offset);
	void dc_mess_g1_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t dc_flash_r(offs_t offset);
	void dc_flash_w(offs_t offset, uint8_t data);

	static void gdrom_config(device_t *device);
	void dc_base(machine_config &config);
	void dc(machine_config &config);
	void dc_fish(machine_config &config);
	void aica_map(address_map &map) ATTR_COLD;
	void dc_audio_map(address_map &map) ATTR_COLD;
	void dc_map(address_map &map) ATTR_COLD;
	void dc_port(address_map &map) ATTR_COLD;
private:
	uint32_t g1bus_regs[0x100/4]{}; // DC-only

	uint64_t PDTRA = 0U, PCTRA = 0U;
	emu_timer *atapi_timer = nullptr;
	int atapi_xferlen = 0, atapi_xferbase = 0, atapi_xfercomplete;
};

#endif // MAME_SEGACONS_DCCONS_H
