// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/* CALC 3 */
#ifndef MAME_KANEKO_KANEKO_CALC3_H
#define MAME_KANEKO_KANEKO_CALC3_H

#pragma once

#include "machine/eepromser.h"

class kaneko_calc3_device : public device_t
{
public:
	template <typename T, typename U, typename V>
	kaneko_calc3_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag, U &&eeprom_tag, V &&region_tag)
		: kaneko_calc3_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
		m_eeprom.set_tag(std::forward<U>(eeprom_tag));
		m_calc3_region.set_tag(std::forward<V>(region_tag));
	}

	kaneko_calc3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mcu_com0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void mcu_com1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void mcu_com2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void mcu_com3_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void reset_run_timer();
	void mcu_run();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(mcu_run_trigger);

private:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_memory_region m_calc3_region;
	required_shared_ptr<uint16_t> m_mcuram;
	int m_mcu_status;
	int m_mcu_command_offset;
	uint16_t m_mcu_crc;
	uint8_t m_decryption_key_byte;
	uint8_t m_alternateswaps;
	uint8_t m_shift;
	uint8_t m_subtracttype;
	uint8_t m_mode;
	uint8_t m_blocksize_offset;
	uint16_t m_dataend;
	uint16_t m_database;
	int m_data_header[2];
	uint32_t m_writeaddress;
	uint32_t m_writeaddress_current;
	uint16_t m_dsw_addr;
	uint16_t m_eeprom_addr;
	uint16_t m_poll_addr;
	uint16_t m_checksumaddress;
	emu_timer* m_runtimer;

	void mcu_init();
	void initial_scan_tables();
	void mcu_com_w(offs_t offset, uint16_t data, uint16_t mem_mask, int _n_);
	uint8_t shift_bits(uint8_t dat, int bits);
	int decompress_table(int tabnum, uint8_t* dstram, int dstoffset);

	static const int16_t s_keydata[0x40*0x100];
};


DECLARE_DEVICE_TYPE(KANEKO_CALC3, kaneko_calc3_device)

#endif // MAME_KANEKO_KANEKO_CALC3_H
