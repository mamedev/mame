// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_SUNPLUS_GCM394_H
#define MAME_INCLUDES_SUNPLUS_GCM394_H

#pragma once

#include "machine/sunplus_gcm394.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "speaker.h"


class full_memory_device :
	public device_t,
	public device_memory_interface
{
public:
	// construction/destruction
	full_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	template <typename... T> full_memory_device& set_map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); return *this; }

	template <typename... T> full_memory_device& map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); return *this; }

	address_space* get_program() { return m_program; }

protected:
	virtual void device_start() override;
	virtual void device_config_complete() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;


private:
	// internal state
	address_space_config m_program_config;
	address_space *m_program;
};


// device type definition
DECLARE_DEVICE_TYPE(FULL_MEMORY, full_memory_device)



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

	DECLARE_READ8_MEMBER(read_nand);
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


class generalplus_gpspispi_game_state : public gcm394_game_state
{
public:
	generalplus_gpspispi_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		gcm394_game_state(mconfig, type, tag)
	{
	}

	void generalplus_gpspispi(machine_config &config);

	void init_spi();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
};



class generalplus_gpspispi_bkrankp_game_state : public generalplus_gpspispi_game_state
{
public:
	generalplus_gpspispi_bkrankp_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		generalplus_gpspispi_game_state(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{
	}

	void generalplus_gpspispi_bkrankp(machine_config &config);

protected:
	required_device<generic_slot_device> m_cart;
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

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

class wrlshunt_game_state : public gcm394_game_state
{
public:
	wrlshunt_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		gcm394_game_state(mconfig, type, tag)
	{
	}

	void wrlshunt(machine_config &config);
	void paccon(machine_config &config);

	void init_wrlshunt();
	void init_ths();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	std::vector<uint16_t> m_sdram;

	virtual DECLARE_READ16_MEMBER(porta_r) override;
	virtual DECLARE_WRITE16_MEMBER(porta_w) override;

private:


	//required_shared_ptr<u16> m_mainram;

	virtual DECLARE_READ16_MEMBER(cs0_r) override;
	virtual DECLARE_WRITE16_MEMBER(cs0_w) override;
	virtual DECLARE_READ16_MEMBER(cs1_r) override;
	virtual DECLARE_WRITE16_MEMBER(cs1_w) override;
};

class jak_s500_game_state : public wrlshunt_game_state
{
public:
	jak_s500_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		wrlshunt_game_state(mconfig, type, tag)
	{
	}

protected:
	//virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual DECLARE_READ16_MEMBER(porta_r) override;
	virtual DECLARE_READ16_MEMBER(portb_r) override;

private:
};

#endif
