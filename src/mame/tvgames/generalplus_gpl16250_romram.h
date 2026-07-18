// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_TVGAMES_GENERALPLUS_GPL16250_ROMRAM_H
#define MAME_TVGAMES_GENERALPLUS_GPL16250_ROMRAM_H

#pragma once

#include "generalplus_gpl16250.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "machine/generalplus_gpl162xx_soc.h"

#include "screen.h"
#include "speaker.h"


class wrlshunt_game_state : public gcm394_game_state
{
public:
	wrlshunt_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		gcm394_game_state(mconfig, type, tag)
	{
	}

	void gpl16250va_romram(machine_config &config) ATTR_COLD;
	void gpl16220a_romram(machine_config &config) ATTR_COLD;
	void gpl16258vb_romram(machine_config &config) ATTR_COLD;
	  
	void init_wrlshunt() ATTR_COLD;
	void init_ths() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual u16 porta_r() override;
	virtual void porta_w(u16 data) override;

	std::vector<u16> m_sdram;

private:
	virtual u16 cs0_r(offs_t offset) override;
	virtual void cs0_w(offs_t offset, u16 data) override;
	virtual u16 cs1_r(offs_t offset) override;
	virtual void cs1_w(offs_t offset, u16 data) override;

	void common_config(machine_config &config) ATTR_COLD;

	int m_romwords_mask = 0;
};

class jak_s500_game_state : public wrlshunt_game_state
{
public:
	jak_s500_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		wrlshunt_game_state(mconfig, type, tag)
	{
	}

protected:
	virtual void machine_reset() override ATTR_COLD;

	virtual u16 porta_r() override;
	virtual u16 portb_r() override;
};

class lazertag_game_state : public jak_s500_game_state
{
public:
	lazertag_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		jak_s500_game_state(mconfig, type, tag)
	{
	}

protected:
	virtual void machine_reset() override ATTR_COLD;
};


class paccon_game_state : public jak_s500_game_state
{
public:
	paccon_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		jak_s500_game_state(mconfig, type, tag)
	{
	}

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	u16 paccon_speedup_hack_r();
};

class jak_pf_game_state : public jak_s500_game_state
{
public:
	jak_pf_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		jak_s500_game_state(mconfig, type, tag)
	{
	}

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	u16 jak_pf_speedup_hack_r();
	u16 jak_pf_speedup_hack2_r();
};


class jak_prft_game_state : public jak_s500_game_state
{
public:
	jak_prft_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		jak_s500_game_state(mconfig, type, tag)
	{
	}

protected:
	virtual void machine_reset() override ATTR_COLD;
};


#endif // MAME_TVGAMES_GENERALPLUS_GPL16250_ROMRAM_H
