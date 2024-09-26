// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia, Sebastien Volpe
/* Kaneko Toybox */
#ifndef MAME_KANEKO_KANEKO_TOYBOX_H
#define MAME_KANEKO_KANEKO_TOYBOX_H

#pragma once

#include "machine/eepromser.h"

class kaneko_toybox_device : public device_t
{
public:
	static constexpr int GAME_NORMAL = 0;
	static constexpr int GAME_BONK = 1;

	static constexpr int TABLE_NORMAL = 0;
	static constexpr int TABLE_ALT = 1;

	template <typename T, typename U, typename V, typename W>
	kaneko_toybox_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&eeprom_tag, U &&dsw_tag, V &&mcuram_tag, W &&mcudata_tag)
		: kaneko_toybox_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_eeprom.set_tag(std::forward<T>(eeprom_tag));
		m_dsw1.set_tag(std::forward<U>(dsw_tag));
		m_mcuram.set_tag(std::forward<V>(mcuram_tag));
		m_mcudata.set_tag(std::forward<W>(mcudata_tag));
	}

	kaneko_toybox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_table(int tabletype) { m_tabletype = tabletype; }
	void set_game_type(int gametype) { m_gametype = gametype; }

	void mcu_com0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void mcu_com1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void mcu_com2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void mcu_com3_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t mcu_status_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_ioport m_dsw1;
	required_shared_ptr<uint16_t> m_mcuram;
	required_region_ptr<uint8_t> m_mcudata;
	uint16_t m_mcu_com[4];
	int m_gametype;
	int m_tabletype;

	void mcu_com_w(offs_t offset, uint16_t data, uint16_t mem_mask, int _n_);
	void decrypt_rom();
	void handle_04_subcommand(uint8_t mcu_subcmd, uint16_t *mcu_ram);
	void mcu_init();
	void mcu_run();
};


DECLARE_DEVICE_TYPE(KANEKO_TOYBOX, kaneko_toybox_device)

#endif // MAME_KANEKO_KANEKO_TOYBOX_H
