// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * transtape.c  --  Hard Micro SA Transtape
 *
 * Spanish hacking device
 *
 */

#ifndef MAME_BUS_CPC_TRANSTAPE_H
#define MAME_BUS_CPC_TRANSTAPE_H

#pragma once

#include "cpcexp.h"

class cpc_transtape_device  : public device_t,
						public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_transtape_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void set_mapping(uint8_t type) override;
	virtual void romen_w(int state) override { m_romen = state; }

	uint8_t input_r();
	void output_w(uint8_t data);
	DECLARE_INPUT_CHANGED_MEMBER(button_red_w);
	DECLARE_INPUT_CHANGED_MEMBER(button_black_w);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	cpc_expansion_slot_device *m_slot;
	address_space* m_space;
	std::unique_ptr<uint8_t[]> m_ram;  // 8kB internal RAM
	bool m_rom_active;
	bool m_romen;
	uint8_t m_output;

	void map_enable();
};

// device type definition
DECLARE_DEVICE_TYPE(CPC_TRANSTAPE, cpc_transtape_device)

#endif // MAME_BUS_CPC_TRANSTAPE_H
