// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_CART_CARTRIDGE_H
#define __MSX_CART_CARTRIDGE_H


SLOT_INTERFACE_EXTERN(msx_cart);
SLOT_INTERFACE_EXTERN(msx_yamaha_60pin);   // 60 pin expansion slots as found in yamaha machines


class msx_cart_interface : public device_slot_card_interface
{
public:
	msx_cart_interface(const machine_config &mconfig, device_t &device);

	template<class _Object> void set_out_irq_cb(_Object object) { m_out_irq_cb.set_callback(object); m_out_irq_cb.resolve_safe(); }

	// This is called after loading cartridge contents and allows the cartridge
	// implementation to perform some additional initialization based on the
	// cartridge contents.
	virtual void initialize_cartridge() {}

	// reading and writing
	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return 0xff; }
	virtual void write_cart(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) {}

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
	std::vector<uint8_t> m_rom;
	std::vector<uint8_t> m_ram;
	std::vector<uint8_t> m_rom_vlm5030;
	std::vector<uint8_t> m_sram;
	devcb_write_line m_out_irq_cb;
};


#endif
