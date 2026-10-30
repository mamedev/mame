// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_TVGAMES_GENERALPLUS_GPL16250_SPI_H
#define MAME_TVGAMES_GENERALPLUS_GPL16250_SPI_H

#pragma once

#include "generalplus_gpl16250.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "machine/generalplus_gpl162xx_soc.h"

#include "screen.h"
#include "speaker.h"


class generalplus_gpspispi_game_state : public gcm394_game_state
{
public:
	generalplus_gpspispi_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		gcm394_game_state(mconfig, type, tag)
	{
	}

	void generalplus_gpspispi(machine_config &config) ATTR_COLD;

	void init_spi() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	virtual u16 cs0_r(offs_t offset) override;
	virtual void cs0_w(offs_t offset, u16 data) override;
	virtual u16 cs1_r(offs_t offset) override;
	virtual void cs1_w(offs_t offset, u16 data) override;
};


class generalplus_gpspispi_bkrankp_game_state : public generalplus_gpspispi_game_state
{
public:
	generalplus_gpspispi_bkrankp_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		generalplus_gpspispi_game_state(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{
	}

	void generalplus_gpspispi_bkrankp(machine_config &config) ATTR_COLD;

protected:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
};


#endif // MAME_TVGAMES_GENERALPLUS_GPL16250_SPI_H
