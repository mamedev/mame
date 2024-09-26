// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    MTX expansion emulation

**********************************************************************/


#ifndef MAME_BUS_MTX_EXP_H
#define MAME_BUS_MTX_EXP_H

#pragma once

#include "imagedev/cartrom.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mtx_exp_slot_device

class device_mtx_exp_interface;

class mtx_exp_slot_device : public device_t, public device_single_card_slot_interface<device_mtx_exp_interface>, public device_cartrom_image_interface
{
	friend class device_mtx_exp_interface;
public:
	// construction/destruction
	template <typename T>
	mtx_exp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: mtx_exp_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	mtx_exp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io.set_tag(std::forward<T>(tag), spacenum); }

	// callbacks
	auto busreq_handler() { return m_busreq_handler.bind(); }
	auto int_handler() { return m_int_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }

	// for the card to use
	void busreq_w(int state) { m_busreq_handler(state); }
	void int_w(int state) { m_int_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }

	virtual void bankswitch(uint8_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "mtx_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,rom"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

private:
	// address spaces we are attached to
	required_address_space m_program;
	required_address_space m_io;

	devcb_write_line m_busreq_handler;
	devcb_write_line m_int_handler;
	devcb_write_line m_nmi_handler;

	device_mtx_exp_interface *m_card;
};


// ======================> device_mtx_exp_interface

class device_mtx_exp_interface : public device_interface
{
public:
	virtual void bankswitch(uint8_t data) { }

	void rom_alloc(uint32_t size, const char *tag);
	uint8_t* get_rom_base() { return m_rom; }
	uint32_t get_rom_size() { return m_rom_size; }

protected:
	// construction/destruction
	device_mtx_exp_interface(const machine_config &mconfig, device_t &device);

	address_space &program_space() { return *m_slot->m_program; }
	address_space &io_space() { return *m_slot->m_io; }

	mtx_exp_slot_device *m_slot;

private:
	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
};


// device type definition
DECLARE_DEVICE_TYPE(MTX_EXP_SLOT, mtx_exp_slot_device)

void mtx_int_expansion_devices(device_slot_interface &device);
void mtx_ext_expansion_devices(device_slot_interface &device);


#endif // MAME_BUS_MTX_EXP_H
