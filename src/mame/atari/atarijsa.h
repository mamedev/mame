// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarijsa.h

    Functions to emulate the Atari "JSA" audio boards

***************************************************************************/

#ifndef MAME_ATARI_ATARIJSA_H
#define MAME_ATARI_ATARIJSA_H

#pragma once

#include "cpu/m6502/m6502.h"
#include "sound/okim6295.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"
#include "sound/ymopm.h"
#include "atariscom.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(ATARI_JSA_I,    atari_jsa_i_device)
DECLARE_DEVICE_TYPE(ATARI_JSA_II,   atari_jsa_ii_device)
DECLARE_DEVICE_TYPE(ATARI_JSA_III,  atari_jsa_iii_device)
DECLARE_DEVICE_TYPE(ATARI_JSA_IIIS, atari_jsa_iiis_device)


//**************************************************************************
//  I/O PORT BIT HELPERS
//**************************************************************************

#define PORT_ATARI_JSA_SOUND_TO_MAIN_READY(_tag) \
	PORT_READ_LINE_DEVICE_MEMBER(_tag, FUNC(atari_jsa_base_device::sound_to_main_ready))

#define PORT_ATARI_JSA_MAIN_TO_SOUND_READY(_tag) \
	PORT_READ_LINE_DEVICE_MEMBER(_tag, FUNC(atari_jsa_base_device::main_to_sound_ready))



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> atari_jsa_base_device

class atari_jsa_base_device :   public device_t,
								public device_mixer_interface
{
protected:
	// construction/destruction
	atari_jsa_base_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner, uint32_t clock);

public:
	// configuration
	auto test_read_cb() { return m_test_read_cb.bind(); }
	auto main_int_cb() { return m_main_int_cb.bind(); }
	void set_swapped_coins(bool swap) { m_swapped_coins = swap; }

	// getters
	m6502_device &soundcpu() const { return *m_jsacpu; }
	int main_to_sound_ready() { return m_soundcomm->main_to_sound_ready(); }
	int sound_to_main_ready() { return m_soundcomm->sound_to_main_ready(); }

	// main cpu accessors
	void main_command_w(uint8_t data);
	uint8_t main_response_r();
	void sound_reset_w(uint16_t data);

	// read/write handlers
	void ym2151_port_w(uint8_t data);
	int main_test_read_line();

	// I/O lines
	void main_int_write_line(int state);

	// 6502 interrupt handlers
	INTERRUPT_GEN_MEMBER(sound_irq_gen);
	void sound_irq_ack_w(u8 data = 0);
	u8 sound_irq_ack_r();
	void ym2151_irq_gen(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal helpers
	virtual void update_all_volumes() = 0;
	void update_sound_irq();

	// devices
	required_device<atari_sound_comm_device> m_soundcomm;
	required_device<m6502_device> m_jsacpu;
	required_device<ym2151_device> m_ym2151;

	// memory regions
	required_memory_region m_cpu_region;

	// memory banks
	required_memory_bank m_cpu_bank;

	// configuration state
	devcb_read_line    m_test_read_cb;
	devcb_write_line   m_main_int_cb;

	// internal state
	bool                m_timed_int;
	bool                m_ym2151_int;
	double              m_ym2151_volume;
	uint8_t             m_ym2151_ct1;
	uint8_t             m_ym2151_ct2;
	bool                m_swapped_coins;
};


// ======================> atari_jsa_oki_base_device

class atari_jsa_oki_base_device : public atari_jsa_base_device
{
protected:
	// derived construction/destruction
	atari_jsa_oki_base_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner, uint32_t clock);

public:
	// read/write handlers
	uint8_t oki_r(offs_t offset);
	void oki_w(offs_t offset, uint8_t data);
	void wrio_w(uint8_t data);
	void mix_w(uint8_t data);
	void overall_volume_w(uint8_t data);

protected:
	// device level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal helpers
	virtual void update_all_volumes() override;

	// devices
	optional_device<okim6295_device> m_oki1;
	optional_device<okim6295_device> m_oki2;    // JSA IIIs only

	// memory regions
	optional_memory_region m_oki1_region;
	optional_memory_region m_oki2_region;

	// memory banks
	optional_memory_bank m_oki1_banklo;         // JSA III(s) only
	optional_memory_bank m_oki1_bankhi;         // JSA III(s)
	optional_memory_bank m_oki2_banklo;         // JSA IIIs only
	optional_memory_bank m_oki2_bankhi;         // JSA IIIs only

	// internal state
	double              m_oki6295_volume;
	double              m_overall_volume;       // JSA III(s) only
};


// ======================> atari_jsa_i_device

class atari_jsa_i_device : public atari_jsa_base_device
{
public:
	// construction/destruction
	atari_jsa_i_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write handlers
	uint8_t rdio_r();
	void wrio_w(uint8_t data);
	void mix_w(uint8_t data);
	void tms5220_voice(uint8_t data);
	uint8_t pokey_r(offs_t offset);
	void pokey_w(offs_t offset, uint8_t data);

	void atarijsa1_map(address_map &map) ATTR_COLD;
protected:
	// device level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal helpers
	virtual void update_all_volumes() override;

	// devices
	optional_device<pokey_device> m_pokey;
	optional_device<tms5220_device> m_tms5220;
	required_ioport m_jsai;

	// internal state
	double              m_pokey_volume;
	double              m_tms5220_volume;
};


// ======================> atari_jsa_ii_device

class atari_jsa_ii_device : public atari_jsa_oki_base_device
{
public:
	// construction/destruction
	atari_jsa_ii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write handlers
	uint8_t rdio_r();

	void atarijsa2_map(address_map &map) ATTR_COLD;
protected:
	// device level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	required_ioport m_jsaii;
};


// ======================> atari_jsa_iii_device

class atari_jsa_iii_device : public atari_jsa_oki_base_device
{
public:
	// construction/destruction
	atari_jsa_iii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void atarijsa3_map(address_map &map) ATTR_COLD;
	void jsa3_oki1_map(address_map &map) ATTR_COLD;
protected:
	// derived construction/destruction
	atari_jsa_iii_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner, uint32_t clock);

public:
	// read/write handlers
	uint8_t rdio_r();

protected:
	// device level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	required_ioport m_jsaiii;
};


// ======================> atari_jsa_iiis_device

class atari_jsa_iiis_device : public atari_jsa_iii_device
{
public:
	// construction/destruction
	atari_jsa_iiis_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void jsa3_oki2_map(address_map &map) ATTR_COLD;
protected:
	// device level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


#endif // MAME_ATARI_ATARIJSA_H
