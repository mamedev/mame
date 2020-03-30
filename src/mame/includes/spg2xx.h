// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood
#ifndef MAME_INCLUDES_SPG2XX_H
#define MAME_INCLUDES_SPG2XX_H

#pragma once

#include "cpu/unsp/unsp.h"
#include "machine/i2cmem.h"
#include "machine/spg2xx.h"


#include "screen.h"
#include "softlist.h"
#include "speaker.h"
#include "machine/eepromser.h"
#include "machine/i2cmem.h"


class spg2xx_game_state : public driver_device
{
public:
	spg2xx_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_bank(*this, "cartbank"),
		m_io_p1(*this, "P1"),
		m_io_p2(*this, "P2"),
		m_io_p3(*this, "P3"),
		m_io_guny(*this, "GUNY"),
		m_io_gunx(*this, "GUNX"),
		m_i2cmem(*this, "i2cmem")
	{ }

	void spg2xx_base(machine_config &config);
	void rad_skat(machine_config &config);
	void rad_skatp(machine_config &config);
	void rad_sktv(machine_config &config);
	void rad_crik(machine_config &config);
	void non_spg_base(machine_config &config);
	void abltenni(machine_config &config);
	void comil(machine_config &config);
	void tvsprt10(machine_config &config);
	void guitarfv(machine_config &config);
	void tmntmutm(machine_config &config);


	void init_crc();
	void init_tvsprt10();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void decrypt_ac_ff(uint16_t* ROM, int size);

	void switch_bank(uint32_t bank);

	DECLARE_WRITE8_MEMBER(i2c_w);
	DECLARE_READ8_MEMBER(i2c_r);

	virtual DECLARE_WRITE16_MEMBER(porta_w);
	virtual DECLARE_WRITE16_MEMBER(portb_w);
	virtual DECLARE_WRITE16_MEMBER(portc_w);

	DECLARE_READ16_MEMBER(base_porta_r);
	DECLARE_READ16_MEMBER(base_portb_r);
	DECLARE_READ16_MEMBER(base_portc_r);
	DECLARE_READ16_MEMBER(base_guny_r);
	DECLARE_READ16_MEMBER(base_gunx_r);

	required_device<spg2xx_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_memory_bank m_bank;


	virtual void mem_map_4m(address_map &map);
	virtual void mem_map_2m(address_map &map);
	virtual void mem_map_1m(address_map &map);

	uint32_t m_current_bank;

	required_ioport m_io_p1;
	optional_ioport m_io_p2;
	optional_ioport m_io_p3;
	optional_ioport m_io_guny;
	optional_ioport m_io_gunx;
	optional_device<i2cmem_device> m_i2cmem;
};


class spg2xx_game_pballpup_state : public spg2xx_game_state
{
public:
	spg2xx_game_pballpup_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_eeprom(*this, "eeprom")
	{ }

	void pballpup(machine_config &config);

private:
	DECLARE_READ16_MEMBER(porta_r);
	virtual DECLARE_WRITE16_MEMBER(porta_w) override;

	required_device<eeprom_serial_93cxx_device> m_eeprom;
};

class spg2xx_game_comil_state : public spg2xx_game_state
{
public:
	spg2xx_game_comil_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_porta_data(0),
		m_extra_in(*this, "EXTRA%u", 0U)
	{ }

	void comil(machine_config &config);

private:
	DECLARE_READ16_MEMBER(porta_r);
	DECLARE_WRITE16_MEMBER(porta_w) override;
	DECLARE_READ16_MEMBER(portb_r);
	uint16_t m_porta_data;
	required_ioport_array<8> m_extra_in;
};

class spg2xx_game_swclone_state : public spg2xx_game_state
{
public:
	spg2xx_game_swclone_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_porta_data(0),
		m_i2cmem(*this, "i2cmem")
	{ }

	void swclone(machine_config &config);
	void init_swclone();

private:
	DECLARE_READ16_MEMBER(porta_r);
	DECLARE_WRITE16_MEMBER(porta_w) override;
	uint16_t m_porta_data;

	required_device<i2cmem_device> m_i2cmem;
};

class spg2xx_game_tmntmutm_state : public spg2xx_game_state
{
public:
	spg2xx_game_tmntmutm_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_i2cmem(*this, "i2cmem")
	{ }

	void tmntmutm(machine_config &config);

private:
	DECLARE_READ16_MEMBER(guny_r);
	DECLARE_READ16_MEMBER(gunx_r);

	required_device<i2cmem_device> m_i2cmem;
};

class spg2xx_game_albkickb_state : public spg2xx_game_state
{
public:
	spg2xx_game_albkickb_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void ablkickb(machine_config &config);

	void init_ablkickb();

private:
	DECLARE_READ16_MEMBER(portb_r);
};

class spg2xx_game_dreamlss_state : public spg2xx_game_state
{
public:
	spg2xx_game_dreamlss_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_porta_data(0),
		m_portb_data(0),
		m_i2cmem(*this, "i2cmem")
	{ }

	void dreamlss(machine_config &config);

private:
	uint16_t m_porta_data;
	uint16_t m_portb_data;

	DECLARE_READ16_MEMBER(porta_r);
	DECLARE_READ16_MEMBER(portb_r);
	virtual DECLARE_WRITE16_MEMBER(portb_w) override;
	virtual DECLARE_WRITE16_MEMBER(porta_w) override;

	required_device<i2cmem_device> m_i2cmem;
};



#endif // MAME_INCLUDES_SPG2XX_H
