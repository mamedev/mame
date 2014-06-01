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
	virtual DECLARE_READ8_MEMBER(read_cart) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_cart) {}

	// ROM/RAM/SRAM management
	// Mainly used by the cartridge slot when loading images
	void rom_alloc(UINT32 size);
	void ram_alloc(UINT32 size);
	void sram_alloc(UINT32 size);

	UINT8* get_rom_base() { return m_rom; }
	UINT8* get_ram_base() { return m_ram; }
	UINT8* get_sram_base() { return m_sram; }
	UINT32 get_rom_size() { return m_rom.count(); }
	UINT32 get_ram_size() { return m_ram.count(); }
	UINT32 get_sram_size() { return m_sram.count(); }

protected:
	dynamic_buffer m_rom;
	dynamic_buffer m_ram;
	dynamic_buffer m_sram;
	devcb_write_line m_out_irq_cb;
};


#endif
