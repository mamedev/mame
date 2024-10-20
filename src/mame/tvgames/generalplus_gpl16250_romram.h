// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_TVGAMES_GENERALPLUS_GPL16250_ROMRAM_H
#define MAME_TVGAMES_GENERALPLUS_GPL16250_ROMRAM_H

#pragma once

#include "generalplus_gpl16250.h"
#include "machine/generalplus_gpl16250soc.h"
#include "generalplus_gpl16250_m.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "speaker.h"


class wrlshunt_game_state : public gcm394_game_state
{
public:
	wrlshunt_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		gcm394_game_state(mconfig, type, tag)
	{
	}

	void gpl16250_romram(machine_config &config);

	void init_wrlshunt();
	void init_ths();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	std::vector<uint16_t> m_sdram;

	virtual uint16_t porta_r() override;
	virtual void porta_w(uint16_t data) override;

private:


	//required_shared_ptr<u16> m_mainram;

	virtual uint16_t cs0_r(offs_t offset) override;
	virtual void cs0_w(offs_t offset, uint16_t data) override;
	virtual uint16_t cs1_r(offs_t offset) override;
	virtual void cs1_w(offs_t offset, uint16_t data) override;

	int m_romwords_mask = 0;
};

class jak_s500_game_state : public wrlshunt_game_state
{
public:
	jak_s500_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		wrlshunt_game_state(mconfig, type, tag)
	{
	}

protected:
	//virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual uint16_t porta_r() override;
	virtual uint16_t portb_r() override;

private:
};

class lazertag_game_state : public jak_s500_game_state
{
public:
	lazertag_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		jak_s500_game_state(mconfig, type, tag)
	{
	}

protected:
	//virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
};


class paccon_game_state : public jak_s500_game_state
{
public:
	paccon_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		jak_s500_game_state(mconfig, type, tag)
	{
	}

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	uint16_t paccon_speedup_hack_r();
};

class jak_pf_game_state : public jak_s500_game_state
{
public:
	jak_pf_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		jak_s500_game_state(mconfig, type, tag)
	{
	}

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	uint16_t jak_pf_speedup_hack_r();
	uint16_t jak_pf_speedup_hack2_r();
};


class jak_prft_game_state : public jak_s500_game_state
{
public:
	jak_prft_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		jak_s500_game_state(mconfig, type, tag)
	{
	}

protected:

	virtual void machine_reset() override ATTR_COLD;

private:
};



#endif // MAME_TVGAMES_GENERALPLUS_GPL16250_ROMRAM_H

