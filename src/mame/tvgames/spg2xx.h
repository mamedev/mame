// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood
#ifndef MAME_TVGAMES_SPG2XX_H
#define MAME_TVGAMES_SPG2XX_H

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
	void spg2xx(machine_config &config);
	void spg2xx_pal(machine_config &config);
	void spg28x(machine_config &config);

	void rad_skat(machine_config &config);
	void rad_skatp(machine_config &config);
	void rad_sktv(machine_config &config);
	void rad_crik(machine_config &config);
	void non_spg_base(machine_config &config);
	void comil(machine_config &config);
	void tvsprt10(machine_config &config);
	void guitarfv(machine_config &config);
	void tmntmutm(machine_config &config);


	void init_crc();
	void init_tvsprt10();
	void init_itvphone();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void decrypt_ac_ff(uint16_t* ROM, int size);

	void switch_bank(uint32_t bank);

	void i2c_w(offs_t offset, uint8_t data);
	uint8_t i2c_r(offs_t offset);

	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	virtual void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t base_porta_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t base_portb_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t base_portc_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t base_guny_r();
	uint16_t base_gunx_r();

	required_device<spg2xx_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_memory_bank m_bank;


	virtual void mem_map_4m(address_map &map) ATTR_COLD;
	virtual void mem_map_2m(address_map &map) ATTR_COLD;
	virtual void mem_map_1m(address_map &map) ATTR_COLD;

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
	uint16_t porta_r();
	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

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
	uint16_t porta_r(offs_t offset, uint16_t mem_mask = ~0);
	void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	uint16_t portb_r(offs_t offset, uint16_t mem_mask = ~0);
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
	uint16_t porta_r();
	void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
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
	uint16_t guny_r();
	uint16_t gunx_r();

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
	uint16_t portb_r(offs_t offset, uint16_t mem_mask = ~0);
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

	uint16_t porta_r();
	uint16_t portb_r();
	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	required_device<i2cmem_device> m_i2cmem;
};

class spg2xx_game_gssytts_state : public spg2xx_game_state
{
public:
	spg2xx_game_gssytts_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_upperbank(*this, "upperbank")
	{ }

	void gssytts(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	required_memory_bank m_upperbank;

private:

	void mem_map_upperbank(address_map &map) ATTR_COLD;
};

class spg2xx_game_senwfit_state : public spg2xx_game_gssytts_state
{
public:
	spg2xx_game_senwfit_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_gssytts_state(mconfig, type, tag)
	{ }

	void init_senwfit();

protected:
	virtual void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};


class spg2xx_game_senspeed_state : public spg2xx_game_state
{
public:
	spg2xx_game_senspeed_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_i2cmem(*this, "i2cmem")
	{ }

	void senspeed(machine_config &config);

private:
	uint16_t portb_r();
	void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	required_device<i2cmem_device> m_i2cmem;
};

class spg2xx_game_fordrace_state : public spg2xx_game_state
{
public:
	spg2xx_game_fordrace_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void fordrace(machine_config &config);

	ioport_value wheel_r();
	ioport_value wheel2_r();
};



class spg2xx_game_wfcentro_state : public spg2xx_game_state
{
public:
	spg2xx_game_wfcentro_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void wfcentro(machine_config &config);

protected:
//  virtual void machine_start() override ATTR_COLD;
//  virtual void machine_reset() override ATTR_COLD;

//  virtual void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

private:

	void mem_map_wfcentro(address_map &map) ATTR_COLD;
};


class spg2xx_game_lexiart_state : public spg2xx_game_state
{
public:
	spg2xx_game_lexiart_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void lexiart(machine_config &config);

private:
	void mem_map_lexiart(address_map &map) ATTR_COLD;
};


class spg2xx_game_ordentv_state : public spg2xx_game_state
{
public:
	spg2xx_game_ordentv_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void ordentv(machine_config &config);

	void init_ordentv();
	void init_jeuint();

protected:
	uint16_t ordentv_portc_r(offs_t offset, uint16_t mem_mask = ~0);
};

class spg2xx_game_hotwheels_state : public spg2xx_game_state
{
public:
	spg2xx_game_hotwheels_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_porta_dat_hot(0xffff),
		m_io_p1_extra(*this, "P1EXTRA")
	{ }

	void hotwheels(machine_config &config);

protected:
	uint16_t hotwheels_porta_r(offs_t offset, uint16_t mem_mask = ~0);
	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

private:
	uint16_t m_porta_dat_hot;
	required_ioport m_io_p1_extra;
};

class spg2xx_game_doraphone_state : public spg2xx_game_state
{
public:
	spg2xx_game_doraphone_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_portb_data(0),
		m_io_p1_rows(*this, "P1_ROW%u", 1U)
	{ }

	void doraphone(machine_config &config);
	void doraphonep(machine_config &config);

private:
	uint16_t porta_r(offs_t offset, uint16_t mem_mask = ~0);

	void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	uint16_t m_portb_data;

	required_ioport_array<6> m_io_p1_rows;
};

class spg2xx_game_ddr33v_state : public spg2xx_game_state
{
public:
	spg2xx_game_ddr33v_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void init_ddr33v();
};

class epo_tetr_game_state : public spg2xx_game_state
{
public:
	epo_tetr_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_ioextra(*this, "EXTRA")
	{ }

	void epo_tetr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint16_t epo_tetr_r(offs_t offset, uint16_t mem_mask);
	void epo_tetr_portb_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t m_old_portb_data;
	uint16_t m_old_portb_extra_latch;

	required_ioport m_ioextra;
};

class spg2xx_game_prail_state : public spg2xx_game_state
{
public:
	spg2xx_game_prail_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_i2cmem(*this, "i2cmem")
	{ }

	void prail(machine_config &config);

protected:
	void prail_portb_w(offs_t offset, uint16_t data, uint16_t mem_mask);

private:
	required_device<i2cmem_device> m_i2cmem;
};

#endif // MAME_TVGAMES_SPG2XX_H
