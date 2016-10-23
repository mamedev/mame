// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SNS_SGB_H
#define __SNS_SGB_H

#include "snes_slot.h"
#include "rom.h"

#include "cpu/lr35902/lr35902.h"
#include "bus/gameboy/gb_slot.h"
#include "bus/gameboy/rom.h"
#include "bus/gameboy/mbc.h"
#include "video/gb_lcd.h"
#include "sound/gb.h"


// ======================> sns_rom_sgb_device

class sns_rom_sgb_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_sgb_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t chip_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void chip_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual uint8_t gb_cart_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void gb_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual uint8_t gb_ram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void gb_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual uint8_t gb_echo_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void gb_echo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual uint8_t gb_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void gb_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual uint8_t gb_ie_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void gb_ie_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void gb_timer_callback(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	required_device<lr35902_cpu_device> m_sgb_cpu;
	required_device<gameboy_sound_device> m_sgb_apu;
	required_device<sgb_ppu_device> m_sgb_ppu;
	required_device<gb_cart_slot_device> m_cartslot;
	required_memory_region m_region_bios;

	void lcd_render(uint32_t *source);

	// ICD2 regs
	uint8_t m_sgb_ly;
	uint8_t m_sgb_row;
	uint8_t m_vram;
	uint8_t m_port;
	uint8_t m_joy1, m_joy2, m_joy3, m_joy4;
	uint8_t m_joy_pckt[16];
	uint16_t m_vram_offs;
	uint8_t m_mlt_req;

	uint32_t m_lcd_buffer[4 * 160 * 8];
	uint16_t m_lcd_output[320];
	uint16_t m_lcd_row;

	// input bits
	int m_packetsize;
	uint8_t m_packet_data[64][16];

	bool m_bios_disabled;
};


class sns_rom_sgb1_device : public sns_rom_sgb_device
{
public:
	// construction/destruction
	sns_rom_sgb1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};


class sns_rom_sgb2_device : public sns_rom_sgb_device
{
public:
	// construction/destruction
	sns_rom_sgb2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

// device type definition
extern const device_type SNS_LOROM_SUPERGB;
extern const device_type SNS_LOROM_SUPERGB2;

#endif
