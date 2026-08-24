// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_TVGAMES_GENERALPLUS_GPL16250_NAND_H
#define MAME_TVGAMES_GENERALPLUS_GPL16250_NAND_H

#pragma once

#include "generalplus_gpl16250.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "machine/generalplus_gpl162xx_b_soc.h"
#include "machine/nandflash.h"

#include "screen.h"
#include "speaker.h"


class generalplus_gpac800_game_state : public gcm394_game_state
{
public:
	generalplus_gpac800_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		gcm394_game_state(mconfig, type, tag),
		m_nand(*this, "nandrom"),
		m_sdram_kwords(0x400000), // 0x400000 words (0x800000 bytes)
		m_initial_copy_words(0x2000)
	{
	}

	void generalplus_gpac800_nand64mbyte(machine_config &config) ATTR_COLD;
	void generalplus_gpac800_nand128mbyte(machine_config &config) ATTR_COLD;
	void generalplus_gpac800_nand256mbyte(machine_config &config) ATTR_COLD;

	void generalplus_gpl16258vb_nand128mbyte_2048(machine_config &config) ATTR_COLD;
	void generalplus_gpl16258vb_nand512mbyte_2048(machine_config &config) ATTR_COLD;

	void generalplus_gpl16258vb_nand64mbyte(machine_config &config) ATTR_COLD;

	void nand_init() ATTR_COLD;
	void nand_init_32mb() ATTR_COLD;
	void nand_wlsair60() ATTR_COLD;
	void nand_vbaby() ATTR_COLD;
	void nand_tsm() ATTR_COLD;
	void nand_beambox() ATTR_COLD;
	void nand_kiugames() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void common_config(machine_config &config);

	virtual u16 cs0_r(offs_t offset) override;
	virtual void cs0_w(offs_t offset, u16 data) override;
	virtual u16 cs1_r(offs_t offset) override;
	virtual void cs1_w(offs_t offset, u16 data) override;

	std::vector<u16> m_sdram;
	std::vector<u16> m_sdram2;

	required_device<nand_device> m_nand;

private:
	void nand_create_stripped_region();

	void dma_complete_hacks(int state);

	void generalplus_gpac800(machine_config &config) ATTR_COLD;
	void generalplus_gpl16258vb(machine_config &config) ATTR_COLD;

	std::vector<u8> m_strippedrom;

	int m_sdram_kwords;
	int m_initial_copy_words;
	int m_vectorbase = 0;
};


class generalplus_gpac800_vbaby_game_state : public generalplus_gpac800_game_state
{
public:
	generalplus_gpac800_vbaby_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		generalplus_gpac800_game_state(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{
	}

	void generalplus_gpl16258vb_nand128mbyte_2048_vbaby(machine_config &config) ATTR_COLD;

protected:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
};


#endif // MAME_TVGAMES_GENERALPLUS_GPL16250_NAND_H

