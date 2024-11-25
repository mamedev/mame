// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SATURN_SAT_SLOT_H
#define MAME_BUS_SATURN_SAT_SLOT_H

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


// ======================> device_sat_cart_interface

class device_sat_cart_interface : public device_interface
{
public:
	virtual ~device_sat_cart_interface();

	// reading from ROM
	virtual uint32_t read_rom(offs_t offset) { return 0xffffffff; }
	// reading and writing to Extended DRAM chips
	virtual uint32_t read_ext_dram0(offs_t offset) { return 0xffffffff; }
	virtual void write_ext_dram0(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { }
	virtual uint32_t read_ext_dram1(offs_t offset) { return 0xffffffff; }
	virtual void write_ext_dram1(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { }
	// reading and writing to Extended BRAM chip
	virtual uint32_t read_ext_bram(offs_t offset) { return 0xffffffff; }
	virtual void write_ext_bram(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { }

	int get_cart_type() const { return m_cart_type; }

	void rom_alloc(uint32_t size);
	void bram_alloc(uint32_t size);
	void dram0_alloc(uint32_t size);
	void dram1_alloc(uint32_t size);
	uint32_t* get_rom_base() { return m_rom; }
	uint32_t* get_ext_dram0_base() { return &m_ext_dram0[0]; }
	uint32_t* get_ext_dram1_base() { return &m_ext_dram1[0]; }
	uint8_t*  get_ext_bram_base() { return &m_ext_bram[0]; }
	uint32_t  get_rom_size() { return m_rom_size; }
	uint32_t  get_ext_dram0_size() { return m_ext_dram0.size()*sizeof(uint32_t); }
	uint32_t  get_ext_dram1_size() { return m_ext_dram1.size()*sizeof(uint32_t); }
	uint32_t  get_ext_bram_size() { return m_ext_bram.size(); }

protected:
	// construction/destruction
	device_sat_cart_interface(const machine_config &mconfig, device_t &device, int cart_type);

	const int m_cart_type;

	// internal state
	uint32_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint32_t> m_ext_dram0;
	std::vector<uint32_t> m_ext_dram1;
	std::vector<uint8_t> m_ext_bram;
};


// ======================> sat_cart_slot_device

class sat_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_sat_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	sat_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: sat_cart_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sat_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~sat_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "sat_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_cart_type();

	// reading and writing
	virtual uint32_t read_rom(offs_t offset);
	virtual uint32_t read_ext_dram0(offs_t offset);
	virtual void write_ext_dram0(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	virtual uint32_t read_ext_dram1(offs_t offset);
	virtual void write_ext_dram1(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	virtual uint32_t read_ext_bram(offs_t offset);
	virtual void write_ext_bram(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	device_sat_cart_interface*       m_cart;
};


// device type definition
DECLARE_DEVICE_TYPE(SATURN_CART_SLOT, sat_cart_slot_device)

#endif // MAME_BUS_SATURN_SAT_SLOT_H
