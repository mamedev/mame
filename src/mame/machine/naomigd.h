// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_NAOMIGD_H
#define MAME_MACHINE_NAOMIGD_H

#pragma once

#include "machine/naomibd.h"
#include "cpu/pic16c62x/pic16c62x.h"
#include "machine/i2cmem.h"
#include "machine/eepromser.h"
#include "machine/315-6154.h"
#include "machine/idectrl.h"
#include "machine/gdrom.h"

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

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void submap(address_map& map) override;
	void sh4_map(address_map &map);
	void sh4_io_map(address_map &map);
	void pic_map(address_map &map);
	void pci_map(address_map &map);
	void pci_config_map(address_map &map);

	void set_image_tag(const char *_image_tag)
	{
		image_tag = _image_tag;
	}

	uint8_t *memory(uint32_t &size) { size = dimm_data_size; return dimm_data; }

	virtual const tiny_rom_entry *device_rom_region() const override;

	DECLARE_WRITE16_MEMBER(dimm_command_w);		// 5f703c
	DECLARE_READ16_MEMBER(dimm_command_r);
	DECLARE_WRITE16_MEMBER(dimm_offsetl_w);		// 5f7040
	DECLARE_READ16_MEMBER(dimm_offsetl_r);
	DECLARE_WRITE16_MEMBER(dimm_parameterl_w);	// 5f7044
	DECLARE_READ16_MEMBER(dimm_parameterl_r);
	DECLARE_WRITE16_MEMBER(dimm_parameterh_w);	// 5f7048
	DECLARE_READ16_MEMBER(dimm_parameterh_r);
	DECLARE_WRITE16_MEMBER(dimm_status_w);		// 5f704c
	DECLARE_READ16_MEMBER(dimm_status_r);

	DECLARE_WRITE32_MEMBER(sh4_unknown_w);		// 14000000
	DECLARE_READ32_MEMBER(sh4_unknown_r);
	DECLARE_WRITE32_MEMBER(sh4_command_w);		// 14000014
	DECLARE_READ32_MEMBER(sh4_command_r);
	DECLARE_WRITE32_MEMBER(sh4_offsetl_w);		// 14000018
	DECLARE_READ32_MEMBER(sh4_offsetl_r);
	DECLARE_WRITE32_MEMBER(sh4_parameterl_w);	// 1400001c
	DECLARE_READ32_MEMBER(sh4_parameterl_r);
	DECLARE_WRITE32_MEMBER(sh4_parameterh_w);	// 14000020
	DECLARE_READ32_MEMBER(sh4_parameterh_r);
	DECLARE_WRITE32_MEMBER(sh4_status_w);		// 14000024
	DECLARE_READ32_MEMBER(sh4_status_r);
	DECLARE_WRITE32_MEMBER(sh4_control_w);		// 14000028
	DECLARE_READ32_MEMBER(sh4_control_r);
	DECLARE_WRITE32_MEMBER(sh4_des_keyl_w);		// 14000030
	DECLARE_READ32_MEMBER(sh4_des_keyl_r);
	DECLARE_WRITE32_MEMBER(sh4_des_keyh_w);		// 14000034
	DECLARE_READ32_MEMBER(sh4_des_keyh_r);

	DECLARE_READ64_MEMBER(i2cmem_dimm_r);
	DECLARE_WRITE64_MEMBER(i2cmem_dimm_w);
	DECLARE_READ8_MEMBER(pic_dimm_r);
	DECLARE_WRITE8_MEMBER(pic_dimm_w);

	DECLARE_READ32_MEMBER(ide_cs0_r);
	DECLARE_READ32_MEMBER(ide_cs1_r);
	DECLARE_WRITE32_MEMBER(ide_cs0_w);
	DECLARE_WRITE32_MEMBER(ide_cs1_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void board_setup_address(uint32_t address, bool is_dma) override;
	virtual void board_get_buffer(uint8_t *&base, uint32_t &limit) override;
	virtual void board_advance(uint32_t size) override;

private:
	enum { FILENAME_LENGTH=24 };
	const int work_mode = 0; // set to 1 and rebuild to enable the cpus

	required_device<sh4_device> m_maincpu;
	required_device<pic16c622_device> m_securitycpu;
	required_device<i2cmem_device> m_i2c0;
	required_device<i2cmem_device> m_i2c1;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<sega_315_6154_device> m_315_6154;
	required_device<bus_master_ide_controller_device> m_ide;

	const char *image_tag;
	optional_region_ptr<uint8_t> picdata;

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
	uint32_t sh4_unknown;
	uint64_t dimm_des_key;

	// Note: voluntarily not saved into the state
	uint8_t *dimm_des_data;
	uint8_t *dimm_data;
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
	uint64_t rev64(uint64_t src);
	uint64_t read_to_qword(const uint8_t *region);
	void write_from_qword(uint8_t *region, uint64_t qword);
};

DECLARE_DEVICE_TYPE(NAOMI_GDROM_BOARD, naomi_gdrom_board)

#endif // MAME_MACHINE_NAOMIGD_H
