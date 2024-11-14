// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SEGA_NAOMIGD_H
#define MAME_SEGA_NAOMIGD_H

#pragma once

#include "naomibd.h"
#include "cpu/pic16c62x/pic16c62x.h"
#include "machine/i2cmem.h"
#include "machine/eepromser.h"
#include "315-6154.h"
#include "machine/idectrl.h"

// For ide gdrom controller

class idegdrom_device : public pci_device {
public:
	idegdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const char *image_tag, const char *space_tag, int space_id);
	idegdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_callback() { return irq_cb.bind(); }

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void map_command(address_map &map) ATTR_COLD;
	void map_control(address_map &map) ATTR_COLD;
	void map_dma(address_map &map) ATTR_COLD;

	uint32_t ide_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t ide_cs1_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void ide_cs1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void ide_irq(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

private:
	required_device<bus_master_ide_controller_device> m_ide;
	devcb_write_line irq_cb;
	const char *space_owner_tag;
	int space_owner_id;
};

DECLARE_DEVICE_TYPE(IDE_GDROM, idegdrom_device)

class naomi_gdrom_board : public naomi_board
{
public:
	template <typename T, typename U>
	naomi_gdrom_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&eeprom_tag, const char *_image_tag, U &&picregion_tag)
		: naomi_gdrom_board(mconfig, tag, owner, clock)
	{
		eeprom.set_tag(std::forward<T>(eeprom_tag));
		set_image_tag(_image_tag);
		picdata.set_tag(std::forward<U>(picregion_tag));
	}

	template <typename T>
	naomi_gdrom_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const char *_image_tag, T &&picregion_tag)
		: naomi_gdrom_board(mconfig, tag, owner, clock)
	{
		picdata.set_tag(std::forward<T>(picregion_tag));
		set_image_tag(_image_tag);
	}

	naomi_gdrom_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void submap(address_map &map) override ATTR_COLD;
	void sh4_map(address_map &map) ATTR_COLD;
	void sh4_io_map(address_map &map) ATTR_COLD;
	void pci_map(address_map &map) ATTR_COLD;
	void pci_config_map(address_map &map) ATTR_COLD;

	void set_image_tag(const char *_image_tag)
	{
		image_tag = _image_tag;
	}

	uint8_t *memory(uint32_t &size) { size = dimm_data_size; return dimm_data.get(); }

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void dimm_command_w(uint16_t data);     // 5f703c
	uint16_t dimm_command_r();
	void dimm_offsetl_w(uint16_t data);     // 5f7040
	uint16_t dimm_offsetl_r();
	void dimm_parameterl_w(uint16_t data);  // 5f7044
	uint16_t dimm_parameterl_r();
	void dimm_parameterh_w(uint16_t data);  // 5f7048
	uint16_t dimm_parameterh_r();
	void dimm_status_w(uint16_t data);      // 5f704c
	uint16_t dimm_status_r();

	void sh4_unknown_w(uint32_t data);      // 14000000
	uint32_t sh4_unknown_r();
	void sh4_command_w(uint32_t data);      // 14000014
	uint32_t sh4_command_r();
	void sh4_offsetl_w(uint32_t data);      // 14000018
	uint32_t sh4_offsetl_r();
	void sh4_parameterl_w(uint32_t data);   // 1400001c
	uint32_t sh4_parameterl_r();
	void sh4_parameterh_w(uint32_t data);   // 14000020
	uint32_t sh4_parameterh_r();
	void sh4_status_w(uint32_t data);       // 14000024
	uint32_t sh4_status_r();
	void sh4_control_w(uint32_t data);      // 14000028
	uint32_t sh4_control_r();
	void sh4_sdramconfig_w(uint32_t data);  // 1400002c
	uint32_t sh4_sdramconfig_r();
	void sh4_des_keyl_w(uint32_t data);     // 14000030
	uint32_t sh4_des_keyl_r();
	void sh4_des_keyh_w(uint32_t data);     // 14000034
	uint32_t sh4_des_keyh_r();
	uint64_t shared_6154_sdram_r(offs_t offset);
	void shared_6154_sdram_w(offs_t offset, uint64_t data, uint64_t mem_mask);
	uint32_t shared_sh4_sdram_r(offs_t offset);
	void shared_sh4_sdram_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint64_t i2cmem_dimm_r();
	void i2cmem_dimm_w(uint64_t data);
	uint8_t pic_dimm_r();
	void pic_dimm_w(offs_t offset, uint8_t data, uint8_t mem_mask);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void board_setup_address(uint32_t address, bool is_dma) override;
	virtual void board_get_buffer(uint8_t *&base, uint32_t &limit) override;
	virtual void board_advance(uint32_t size) override;

private:
	enum { FILENAME_LENGTH=24 };
	int work_mode; // set it different from 0 to enable the cpus and full dimm board emulation

	required_device<sh4_device> m_maincpu;
	required_device<pic16c622_device> m_securitycpu;
	required_device<i2cmem_device> m_i2c0;
	required_device<i2cmem_device> m_i2c1;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<sega_315_6154_device> m_315_6154;
	required_device<idegdrom_device> m_idegdrom;
	required_ioport m_debug_dipswitches;
	optional_region_ptr<uint8_t> picdata;

	const char *image_tag;
	address_space *space_sh4;
	address_space *space_6154;

	uint32_t dimm_cur_address;
	uint8_t picbus;
	uint8_t picbus_pullup;
	uint8_t picbus_io[2]; // 0 for sh4, 1 for pic
	bool picbus_used;
	uint32_t dimm_command;
	uint32_t dimm_offsetl;
	uint32_t dimm_parameterl;
	uint32_t dimm_parameterh;
	uint32_t dimm_status;
	uint32_t dimm_control;
	uint32_t dimm_sdramconfig;
	uint32_t sh4_unknown;
	uint64_t dimm_des_key;

	// Note: voluntarily not saved into the state
	std::unique_ptr<uint8_t[]> dimm_des_data;
	std::unique_ptr<uint8_t[]> dimm_data;
	uint32_t dimm_data_size;

	static const uint32_t DES_LEFTSWAP[];
	static const uint32_t DES_RIGHTSWAP[];
	static const uint32_t DES_SBOX1[];
	static const uint32_t DES_SBOX2[];
	static const uint32_t DES_SBOX3[];
	static const uint32_t DES_SBOX4[];
	static const uint32_t DES_SBOX5[];
	static const uint32_t DES_SBOX6[];
	static const uint32_t DES_SBOX7[];
	static const uint32_t DES_SBOX8[];
	static const uint32_t DES_MASK_TABLE[];
	static const uint8_t DES_ROTATE_TABLE[16];

	void find_file(const char *name, const uint8_t *dir_sector, uint32_t &file_start, uint32_t &file_size);

	inline void permutate(uint32_t &a, uint32_t &b, uint32_t m, int shift);
	void des_generate_subkeys(const uint64_t key, uint32_t *subkeys);
	uint64_t des_encrypt_decrypt(bool decrypt, uint64_t src, const uint32_t *des_subkeys);
};

DECLARE_DEVICE_TYPE(NAOMI_GDROM_BOARD, naomi_gdrom_board)


#endif // MAME_SEGA_NAOMIGD_H
