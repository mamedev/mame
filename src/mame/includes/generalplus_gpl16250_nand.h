// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_GENERALPLUS_GPL16250_NAND_H
#define MAME_INCLUDES_GENERALPLUS_GPL16250_NAND_H

#pragma once

#include "includes/generalplus_gpl16250.h"
#include "machine/generalplus_gpl16250soc.h"
#include "machine/generalplus_gpl16250.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "speaker.h"


class generalplus_gpac800_game_state : public gcm394_game_state
{
public:
	generalplus_gpac800_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		gcm394_game_state(mconfig, type, tag),
		m_nandregion(*this, "nandrom"),
		m_sdram_kwords(0x400000), // 0x400000 words (0x800000 bytes)
		m_initial_copy_words(0x2000)
	{
	}

	void generalplus_gpac800(machine_config &config);

	void nand_init210();
	void nand_init210_32mb();
	void nand_init840();
	void nand_wlsair60();
	void nand_vbaby();
	void nand_tsm();
	void nand_beambox();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint8_t read_nand(offs_t offset);
	std::vector<uint16_t> m_sdram;
	std::vector<uint16_t> m_sdram2;

	virtual DECLARE_READ16_MEMBER(cs0_r) override;
	virtual DECLARE_WRITE16_MEMBER(cs0_w) override;
	virtual DECLARE_READ16_MEMBER(cs1_r) override;
	virtual DECLARE_WRITE16_MEMBER(cs1_w) override;

private:
	optional_region_ptr<uint8_t> m_nandregion;

	void nand_create_stripped_region();

	std::vector<uint8_t> m_strippedrom;
	int m_strippedsize;
	int m_size;
	int m_nandblocksize;
	int m_nandblocksize_stripped;

	int m_sdram_kwords;
	int m_initial_copy_words;
	int m_vectorbase;
};


class generalplus_gpac800_vbaby_game_state : public generalplus_gpac800_game_state
{
public:
	generalplus_gpac800_vbaby_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		generalplus_gpac800_game_state(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{
	}

	void generalplus_gpac800_vbaby(machine_config &config);

protected:
	required_device<generic_slot_device> m_cart;
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

private:
};


#endif // MAME_INCLUDES_GENERALPLUS_GPL16250_NAND_H

