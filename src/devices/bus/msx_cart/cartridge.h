// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_CARTRIDGE_H
#define MAME_BUS_MSX_CART_CARTRIDGE_H

#pragma once


void msx_cart(device_slot_interface &device);
void msx_yamaha_60pin(device_slot_interface &device);   // 60 pin expansion slots as found in yamaha machines


class msx_cart_interface : public device_slot_card_interface
{
public:
	template <class Object> void set_out_irq_cb(Object &&cb) { m_out_irq_cb.set_callback(std::forward<Object>(cb)); }

	// This is called after loading cartridge contents and allows the cartridge
	// implementation to perform some additional initialization based on the
	// cartridge contents.
	virtual void initialize_cartridge() { }

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_cart) { }

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

	virtual void interface_pre_start() override { m_out_irq_cb.resolve_safe(); }

	std::vector<uint8_t> m_rom;
	std::vector<uint8_t> m_ram;
	std::vector<uint8_t> m_rom_vlm5030;
	std::vector<uint8_t> m_sram;
	devcb_write_line m_out_irq_cb;
};


#endif // MAME_BUS_MSX_CART_CARTRIDGE_H
