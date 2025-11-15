// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_TVGAMES_GENERALPLUS_GPL16250_H
#define MAME_TVGAMES_GENERALPLUS_GPL16250_H

#pragma once

#include "generalplus_gpl16250_m.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "machine/generalplus_gpl951xx_soc.h"
#include "machine/generalplus_gpl1625x_soc.h"

#include "screen.h"
#include "speaker.h"



class gcm394_game_state : public driver_device
{
public:
	gcm394_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_io(*this, "IN%u", 0U),
		m_romregion(*this, "maincpu"),
		m_memory(*this, "memory")
	{
	}

	void base(machine_config &config);
	void base_alt_irq(machine_config &config);

	void cs_map_base(address_map &map) ATTR_COLD;

	virtual uint16_t cs0_r(offs_t offset);
	virtual void cs0_w(offs_t offset, uint16_t data);
	virtual uint16_t cs1_r(offs_t offset);
	virtual void cs1_w(offs_t offset, uint16_t data);
	virtual uint16_t cs2_r(offs_t offset);
	virtual void cs2_w(offs_t offset, uint16_t data);
	virtual uint16_t cs3_r(offs_t offset);
	virtual void cs3_w(offs_t offset, uint16_t data);
	virtual uint16_t cs4_r(offs_t offset);
	virtual void cs4_w(offs_t offset, uint16_t data);

	void cs_callback(uint16_t cs0, uint16_t cs1, uint16_t cs2, uint16_t cs3, uint16_t cs4);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;


	required_device<sunplus_gcm394_base_device> m_maincpu;
	required_device<screen_device> m_screen;


	required_ioport_array<3> m_io;


	optional_region_ptr<uint16_t> m_romregion;
	required_device<full_memory_device> m_memory;

	virtual uint16_t porta_r();
	virtual uint16_t portb_r();
	virtual uint16_t portc_r();
	virtual void porta_w(uint16_t data);

	virtual uint16_t read_external_space(offs_t offset);
	virtual void write_external_space(offs_t offset, uint16_t data);

private:
};


#endif // MAME_TVGAMES_GENERALPLUS_GPL16250_H
