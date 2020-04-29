// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_SUNPLUS_GCM394_H
#define MAME_INCLUDES_SUNPLUS_GCM394_H

#pragma once

#include "machine/generalplus_gpl16250soc.h"
#include "machine/generalplus_gpl16250.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

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

	void cs_map_base(address_map &map);

	virtual DECLARE_READ16_MEMBER(cs0_r);
	virtual DECLARE_WRITE16_MEMBER(cs0_w);
	virtual DECLARE_READ16_MEMBER(cs1_r);
	virtual DECLARE_WRITE16_MEMBER(cs1_w);
	virtual DECLARE_READ16_MEMBER(cs2_r);
	virtual DECLARE_WRITE16_MEMBER(cs2_w);
	virtual DECLARE_READ16_MEMBER(cs3_r);
	virtual DECLARE_WRITE16_MEMBER(cs3_w);
	virtual DECLARE_READ16_MEMBER(cs4_r);
	virtual DECLARE_WRITE16_MEMBER(cs4_w);

	void cs_callback(uint16_t cs0, uint16_t cs1, uint16_t cs2, uint16_t cs3, uint16_t cs4);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;


	required_device<sunplus_gcm394_base_device> m_maincpu;
	required_device<screen_device> m_screen;


	required_ioport_array<3> m_io;


	optional_region_ptr<uint16_t> m_romregion;
	required_device<full_memory_device> m_memory;

	virtual DECLARE_READ16_MEMBER(porta_r);
	virtual DECLARE_READ16_MEMBER(portb_r);
	virtual DECLARE_READ16_MEMBER(portc_r);
	virtual DECLARE_WRITE16_MEMBER(porta_w);

	virtual DECLARE_READ16_MEMBER(read_external_space);
	virtual DECLARE_WRITE16_MEMBER(write_external_space);

private:
};




class tkmag220_game_state : public gcm394_game_state
{
public:
	tkmag220_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		gcm394_game_state(mconfig, type, tag)
	{
	}

	void tkmag220(machine_config &config);

protected:

	/*
	virtual DECLARE_READ16_MEMBER(porta_r) override
	{
	    return machine().rand();
	}

	virtual DECLARE_READ16_MEMBER(portb_r) override
	{
	    return machine().rand();
	}

	virtual DECLARE_WRITE16_MEMBER(porta_w) override
	{
	}
	*/

private:

	virtual DECLARE_READ16_MEMBER(cs0_r) override
	{
		return m_romregion[offset & 0x3ffffff];
	}
};


#endif
