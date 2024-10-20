// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_INTV_SLOT_H
#define MAME_BUS_INTV_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	INTV_STD = 0,
	INTV_RAM,
	INTV_GFACT, // has RAM too but at diff offset
	INTV_WSMLB,
	INTV_VOICE,
	INTV_ECS,
	INTV_KEYCOMP
};


#define INTV_ROM16_READ(addr) \
	(uint16_t) (m_rom[(addr) << 1] | (m_rom[((addr) << 1) + 1] << 8))


// ======================> device_intv_cart_interface

class device_intv_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_intv_cart_interface();

	// reading and writing
	virtual uint16_t read_rom04(offs_t offset) { return 0xffff; }
	virtual uint16_t read_rom20(offs_t offset) { return 0xffff; }
	virtual uint16_t read_rom40(offs_t offset) { return 0xffff; }
	virtual uint16_t read_rom48(offs_t offset) { return 0xffff; }
	virtual uint16_t read_rom50(offs_t offset) { return 0xffff; }
	virtual uint16_t read_rom60(offs_t offset) { return 0xffff; }
	virtual uint16_t read_rom70(offs_t offset) { return 0xffff; }
	virtual uint16_t read_rom80(offs_t offset) { return 0xffff; }
	virtual uint16_t read_rom90(offs_t offset) { return 0xffff; }
	virtual uint16_t read_roma0(offs_t offset) { return 0xffff; }
	virtual uint16_t read_romb0(offs_t offset) { return 0xffff; }
	virtual uint16_t read_romc0(offs_t offset) { return 0xffff; }
	virtual uint16_t read_romd0(offs_t offset) { return 0xffff; }
	virtual uint16_t read_rome0(offs_t offset) { return 0xffff; }
	virtual uint16_t read_romf0(offs_t offset) { return 0xffff; }

	virtual uint16_t read_ram(offs_t offset) { return 0xffff; }
	virtual void write_ram(offs_t offset, uint16_t data) {}

	// Used by IntelliVoice & ECS
	virtual uint16_t read_ay(offs_t offset) { return 0xffff; }
	virtual void write_ay(offs_t offset, uint16_t data) {}
	virtual uint16_t read_speech(offs_t offset) { return 0xffff; }
	virtual void write_speech(offs_t offset, uint16_t data) {}
	virtual void write_d0(offs_t offset, uint16_t data) {}
	virtual void write_88(offs_t offset, uint16_t data) {}
	virtual void write_rom20(offs_t offset, uint16_t data) {}
	virtual void write_rom70(offs_t offset, uint16_t data) {}
	virtual void write_rome0(offs_t offset, uint16_t data) {}
	virtual void write_romf0(offs_t offset, uint16_t data) {}

	void rom_alloc(uint32_t size);
	void ram_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint8_t* get_ram_base() { return &m_ram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_ram_size() { return m_ram.size(); }

	void save_ram() { device().save_item(NAME(m_ram)); }
	virtual void late_subslot_setup() {}

protected:
	device_intv_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_ram;
};


// ======================> intv_cart_slot_device

class intv_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_intv_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	intv_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: intv_cart_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	intv_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~intv_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override {}

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "intv_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,int,rom,itv"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }
	std::error_condition load_fullpath();

	void save_ram() { if (m_cart && m_cart->get_ram_size()) m_cart->save_ram(); }

	// reading and writing
	uint16_t read_rom04(offs_t offset) { if (m_cart) return m_cart->read_rom04(offset); else return 0xffff; }
	uint16_t read_rom20(offs_t offset) { if (m_cart) return m_cart->read_rom20(offset); else return 0xffff; }
	uint16_t read_rom40(offs_t offset) { if (m_cart) return m_cart->read_rom40(offset); else return 0xffff; }
	uint16_t read_rom48(offs_t offset) { if (m_cart) return m_cart->read_rom48(offset); else return 0xffff; }
	uint16_t read_rom50(offs_t offset) { if (m_cart) return m_cart->read_rom50(offset); else return 0xffff; }
	uint16_t read_rom60(offs_t offset) { if (m_cart) return m_cart->read_rom60(offset); else return 0xffff; }
	uint16_t read_rom70(offs_t offset) { if (m_cart) return m_cart->read_rom70(offset); else return 0xffff; }
	uint16_t read_rom80(offs_t offset) { if (m_cart) return m_cart->read_rom80(offset); else return 0xffff; }
	uint16_t read_rom90(offs_t offset) { if (m_cart) return m_cart->read_rom90(offset); else return 0xffff; }
	uint16_t read_roma0(offs_t offset) { if (m_cart) return m_cart->read_roma0(offset); else return 0xffff; }
	uint16_t read_romb0(offs_t offset) { if (m_cart) return m_cart->read_romb0(offset); else return 0xffff; }
	uint16_t read_romc0(offs_t offset) { if (m_cart) return m_cart->read_romc0(offset); else return 0xffff; }
	uint16_t read_romd0(offs_t offset) { if (m_cart) return m_cart->read_romd0(offset); else return 0xffff; }
	uint16_t read_rome0(offs_t offset) { if (m_cart) return m_cart->read_rome0(offset); else return 0xffff; }
	uint16_t read_romf0(offs_t offset) { if (m_cart) return m_cart->read_romf0(offset); else return 0xffff; }

	uint16_t read_ay(offs_t offset);
	void write_ay(offs_t offset, uint16_t data);
	uint16_t read_speech(offs_t offset);
	void write_speech(offs_t offset, uint16_t data);
	uint16_t read_ram(offs_t offset) { if (m_cart) return m_cart->read_ram(offset); else return 0xffff; }
	void write_ram(offs_t offset, uint16_t data) { if (m_cart) m_cart->write_ram(offset, data); }

	virtual void late_subslot_setup() { if (m_cart) return m_cart->late_subslot_setup(); }

	// these RAM accessors are needed to deal with IntelliVoice and ECS mounting RAM-equipped carts
	void write_d0(offs_t offset, uint16_t data) { if (m_cart) m_cart->write_d0(offset, data); }
	void write_88(offs_t offset, uint16_t data) { if (m_cart) m_cart->write_88(offset, data); }

	// ECS paged roms need these
	void write_rom20(offs_t offset, uint16_t data) { if (m_cart) m_cart->write_rom20(offset, data); }
	void write_rom70(offs_t offset, uint16_t data) { if (m_cart) m_cart->write_rom70(offset, data); }
	void write_rome0(offs_t offset, uint16_t data) { if (m_cart) m_cart->write_rome0(offset, data); }
	void write_romf0(offs_t offset, uint16_t data) { if (m_cart) m_cart->write_romf0(offset, data); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	int m_type;
	device_intv_cart_interface*       m_cart;
};



// device type definition
DECLARE_DEVICE_TYPE(INTV_CART_SLOT, intv_cart_slot_device)


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

void intv_cart(device_slot_interface &device);

#endif // MAME_BUS_INTV_SLOT_H
