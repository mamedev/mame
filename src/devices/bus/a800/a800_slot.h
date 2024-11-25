// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Angelo Salese
#ifndef MAME_BUS_A800_A800_SLOT_H
#define MAME_BUS_A800_A800_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

/* PCB */
// TODO: make this enum to actually follow up the .car specs, not the other way around
enum
{
	A800_8K = 0,
	A800_8K_RIGHT,
	A800_16K,
	A800_OSS034M,
	A800_OSS043M,
	A800_OSSM091,
	A800_OSS8K,
	A800_PHOENIX,
	A800_BLIZZARD,
	A800_XEGS,
	A800_BBSB,
	A800_DIAMOND,
	A800_WILLIAMS,
	A800_EXPRESS,
	A800_SPARTADOS,
	A800_SPARTADOS_128KB,
	A800_TURBO,
	A800_TELELINK2,
	A800_ULTRACART,
	A800_ADAWLIAH,
	A800_ATRAX,
	A800_CORINA,
	A800_CORINA_SRAM,
	ATARIMAX_MAXFLASH_128KB,
	ATARIMAX_MAXFLASH_1MB,
	SIC_128KB,
	SIC_256KB,
	SIC_512KB,
	A5200_4K,
	A5200_8K,
	A5200_16K,
	A5200_32K,
	A5200_16K_2CHIPS,
	A5200_BBSB
};

class a800_cart_slot_device;

class device_a800_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_a800_cart_interface();

	virtual void cart_map(address_map &map) ATTR_COLD;
	virtual void cctl_map(address_map &map) ATTR_COLD;

	// TODO: remove all of this
	void rom_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint32_t get_rom_size() { return m_rom_size; }

	///
	/// Carts can either init RD4/RD5 at startup or not, depending on if they have
	/// reset circuitry or not. Future expansion of this getter will return a third value,
	/// giving back the timing value where the initialization should happen.
	///
	/// \return RD4, RD5 initial state
	virtual std::tuple<int, int> get_initial_rd_state() { return std::make_tuple(0, 0); }

protected:
	device_a800_cart_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	a800_cart_slot_device *m_slot;

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	// helpers
	int m_bank_mask;

	void rd4_w( int state );
	void rd5_w( int state );
	void rd_both_w ( int state );
};


// ======================> a800_cart_slot_device

class a800_cart_slot_device : public device_t,
								public device_memory_interface,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_a800_cart_interface>
{
	friend class device_a800_cart_interface;
public:
	// construction/destruction
	template <typename T>
	a800_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: a800_cart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	a800_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~a800_cart_slot_device();

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "a8bit_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,rom,car"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;
	int identify_cart_type(const uint8_t *header) const;
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	bool has_cart() { return m_cart != nullptr; }
	auto rd4_callback() { return m_rd4_cb.bind(); }
	auto rd5_callback() { return m_rd5_cb.bind(); }
	void set_is_xegs(bool is_xegs) { m_is_xegs = is_xegs; }

	template <unsigned Bank> uint8_t read_cart(offs_t offset);
	template <unsigned Bank> void write_cart(offs_t offset, uint8_t data);
	uint8_t read_cctl(offs_t offset);
	void write_cctl(offs_t offset, uint8_t data);
	auto get_initial_rd_state() { return m_cart->get_initial_rd_state(); }

protected:
	//a800_cart_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	device_a800_cart_interface*       m_cart;

	devcb_write_line m_rd4_cb;
	devcb_write_line m_rd5_cb;

	address_space_config m_space_mem_config;
	address_space_config m_space_io_config;

	address_space *m_space_mem;
	address_space *m_space_io;

	int m_type = 0;
	bool m_is_xegs = false;
};

class a5200_cart_slot_device;

class device_a5200_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_a5200_cart_interface();

	virtual void cart_map(address_map &map) ATTR_COLD;

	// TODO: remove all of this
	void rom_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint32_t get_rom_size() { return m_rom_size; }

protected:
	device_a5200_cart_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	a5200_cart_slot_device *m_slot;

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	// helpers
	int m_bank_mask;
};


class a5200_cart_slot_device : public device_t,
								public device_memory_interface,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_a5200_cart_interface>
{
	friend class device_a5200_cart_interface;
public:
	// construction/destruction
	template <typename T>
	a5200_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: a5200_cart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	a5200_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~a5200_cart_slot_device();

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "a8bit_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,rom,car,a52"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;
	int identify_cart_type(const uint8_t *header) const;
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	u8 read_cart(offs_t offset);
	void write_cart(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	device_a5200_cart_interface*       m_cart;
	address_space_config m_space_mem_config;

	address_space *m_space_mem;
	int m_type;
};

// device type definition
DECLARE_DEVICE_TYPE(A800_CART_SLOT,  a800_cart_slot_device)
DECLARE_DEVICE_TYPE(A5200_CART_SLOT, a5200_cart_slot_device)

#endif // MAME_BUS_A800_A800_SLOT_H
