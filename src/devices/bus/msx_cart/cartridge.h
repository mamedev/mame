// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_CARTRIDGE_H
#define MAME_BUS_MSX_CART_CARTRIDGE_H

#pragma once


void msx_cart(device_slot_interface &device);
void msx_yamaha_60pin(device_slot_interface &device);   // 60 pin expansion slots as found in yamaha machines


class msx_slot_cartridge_device;

class msx_cart_interface : public device_interface
{
	friend class msx_slot_cartridge_device;

public:
	// This is called after loading cartridge contents and allows the cartridge
	// implementation to perform some additional initialization based on the
	// cartridge contents.
	virtual void initialize_cartridge() { }
	virtual void interface_pre_start() override { assert(m_exp != nullptr); }

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) { return 0xff; }
	virtual void write_cart(offs_t offset, uint8_t data) { }

	DECLARE_WRITE_LINE_MEMBER(irq_out);
	address_space &memory_space() const;
	address_space &io_space() const;

	// ROM/RAM/SRAM management
	// Mainly used by the cartridge slot when loading images
	void rom_alloc(uint32_t size);
	void ram_alloc(uint32_t size);
	void rom_vlm5030_alloc(uint32_t size);
	void sram_alloc(uint32_t size);

	uint8_t* get_rom_base() { return &m_rom[0]; }
	uint8_t* get_rom_vlm5030_base() { return &m_rom_vlm5030[0]; }
	uint8_t* get_ram_base() { return &m_ram[0]; }
	uint8_t* get_sram_base() { return &m_sram[0]; }
	uint32_t get_rom_size() { return m_rom.size(); }
	uint32_t get_rom_vlm5030_size() { return m_rom_vlm5030.size(); }
	uint32_t get_ram_size() { return m_ram.size(); }
	uint32_t get_sram_size() { return m_sram.size(); }

protected:
	msx_cart_interface(const machine_config &mconfig, device_t &device);

	std::vector<uint8_t> m_rom;
	std::vector<uint8_t> m_ram;
	std::vector<uint8_t> m_rom_vlm5030;
	std::vector<uint8_t> m_sram;

private:
	msx_slot_cartridge_device *m_exp;
};


#endif // MAME_BUS_MSX_CART_CARTRIDGE_H
