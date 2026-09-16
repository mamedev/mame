// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

Nichibutsu 1412M2 device emulation

***************************************************************************/

#ifndef MAME_NICHIBUTSU_NB1412M2_H
#define MAME_NICHIBUTSU_NB1412M2_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nb1412m2_device

class nb1412m2_device : public device_t, public device_memory_interface
{
public:
	// construction/destruction
	nb1412m2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void command_w(uint8_t data);
	void data_w(uint8_t data);
	uint8_t data_r();

	auto dac_callback() { return m_dac_cb.bind(); }

	void nb1412m2_map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const override;
//  virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	TIMER_CALLBACK_MEMBER(main_timer_tick);
	TIMER_CALLBACK_MEMBER(update_dac);

private:
	uint8_t m_command;
	uint16_t m_rom_address;
	uint16_t m_adj_address;
	uint16_t m_dac_start_address, m_dac_current_address;
	double m_dac_frequency;
	uint8_t m_timer_rate;
	uint8_t m_rom_op;
	uint8_t m_const90;
	bool m_timer_reg;
	bool m_dac_playback;
	const address_space_config m_space_config;
	emu_timer *m_timer;
	emu_timer *m_dac_timer;

	required_region_ptr<uint8_t> m_data;
	devcb_write8 m_dac_cb;

	void rom_address_w(offs_t offset, uint8_t data);
	uint8_t rom_decrypt_r();
	void rom_op_w(uint8_t data);
	void rom_adjust_w(offs_t offset, uint8_t data);
	uint8_t timer_r();
	void timer_w(uint8_t data);
	void timer_ack_w(uint8_t data);
	uint8_t const90_r();
	void const90_w(uint8_t data);
	void dac_address_w(offs_t offset, uint8_t data);
	void dac_control_w(uint8_t data);
	void dac_timer_w(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(NB1412M2, nb1412m2_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_NICHIBUTSU_NB1412M2_H
