// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    cococart.h

    CoCo/Dragon cartridge management

*********************************************************************/

#ifndef MAME_BUS_COCO_COCOCART_H
#define MAME_BUS_COCO_COCOCART_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> cococart_base_update_delegate

// direct region update handler
typedef delegate<void (u8 *)> cococart_base_update_delegate;


// ======================> cococart_slot_device
class device_cococart_interface;

class cococart_slot_device final : public device_t,
								public device_single_card_slot_interface<device_cococart_interface>,
								public device_cartrom_image_interface
{
public:
	// output lines on the CoCo cartridge slot
	enum class line
	{
		CART,             // connects to PIA1 CB1
		NMI,              // connects to NMI line on CPU
		HALT,             // connects to HALT line on CPU
		SOUND_ENABLE      // sound enable
	};

	// since we have a special value "Q" - we have to use a special enum here
	enum class line_value
	{
		CLEAR,
		ASSERT,
		Q
	};

	// construction/destruction
	template <typename T>
	cococart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&opts, const char *dflt)
		: cococart_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	cococart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto cart_callback() { return m_cart_callback.bind(); }
	auto nmi_callback() { return m_nmi_callback.bind(); }
	auto halt_callback() { return m_halt_callback.bind(); }

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "coco_cart"; }
	virtual const char *file_extensions() const noexcept override { return "ccc,rom"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing to $C000-$FFEF
	u8 cts_read(offs_t offset);
	void cts_write(offs_t offset, u8 data);

	// reading and writing to $FF40-$FF5F
	u8 scs_read(offs_t offset);
	void scs_write(offs_t offset, u8 data);

	// manipulation of cartridge lines
	void set_line_value(line line, line_value value);
	void set_line_delay(line line, int cycles);
	line_value get_line_value(line line) const;

	// hack to support twiddling the Q line
	void twiddle_q_lines();

	// cart base
	u8 *get_cart_base();
	u32 get_cart_size();
	void set_cart_base_update(cococart_base_update_delegate update);

private:
	// TIMER_POOL: Must be power of two
	static constexpr int TIMER_POOL = 2;

	struct coco_cartridge_line
	{
		emu_timer                   *timer[TIMER_POOL];
		int                         timer_index;
		int                         delay;
		line_value                  value;
		int                         line;
		int                         q_count;
		devcb_write_line *          callback;
	};

	// configuration
	coco_cartridge_line         m_cart_line;
	coco_cartridge_line         m_nmi_line;
	coco_cartridge_line         m_halt_line;

public:
	devcb_write_line            m_cart_callback;
	devcb_write_line            m_nmi_callback;
	devcb_write_line            m_halt_callback;

private:
	// cartridge
	device_cococart_interface   *m_cart;

	// methods
	void set_line(line ln, coco_cartridge_line &line, line_value value);
	void set_line_timer(coco_cartridge_line &line, line_value value);
	void twiddle_line_if_q(coco_cartridge_line &line);

	TIMER_CALLBACK_MEMBER(cart_line_timer_tick);
	TIMER_CALLBACK_MEMBER(nmi_line_timer_tick);
	TIMER_CALLBACK_MEMBER(halt_line_timer_tick);

public:
	static const char *line_value_string(line_value value);
};

// device type definition
DECLARE_DEVICE_TYPE(COCOCART_SLOT, cococart_slot_device)


// ======================> device_cococart_host_interface

// this is implemented by the CoCo root device itself and the Multi-Pak interface
class device_cococart_host_interface
{
public:
	virtual address_space &cartridge_space() = 0;
};


// ======================> device_cococart_interface

class device_cococart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_cococart_interface();

	virtual u8 cts_read(offs_t offset);
	virtual void cts_write(offs_t offset, u8 data);
	virtual u8 scs_read(offs_t offset);
	virtual void scs_write(offs_t offset, u8 data);
	virtual void set_sound_enable(bool sound_enable);

	virtual u8 *get_cart_base();
	virtual u32 get_cart_size();
	void set_cart_base_update(cococart_base_update_delegate update);
	virtual memory_region *get_cart_memregion();

protected:
	virtual void interface_config_complete() override;
	virtual void interface_pre_start() override;

	device_cococart_interface(const machine_config &mconfig, device_t &device);

	void cart_base_changed(void);

	// accessors for containers
	cococart_slot_device &owning_slot()     { assert(m_owning_slot); return *m_owning_slot; }
	device_cococart_host_interface &host()  { assert(m_host); return *m_host; }

	// CoCo cartridges can read directly from the address bus.  This is used by a number of
	// cartridges (e.g. - Orch-90, Multi-Pak interface) for their control registers, independently
	// of the SCS or CTS lines
	address_space &cartridge_space();
	template <typename R>
	void install_read_handler(u16 addrstart, u16 addrend, R &&rhandler)
	{
		address_space &space(cartridge_space());
		space.install_read_handler(addrstart, addrend, std::forward<R>(rhandler));
	}
	template <typename W>
	void install_write_handler(u16 addrstart, u16 addrend, W &&whandler)
	{
		address_space &space(cartridge_space());
		space.install_write_handler(addrstart, addrend, std::forward<W>(whandler));
	}
	template <typename R, typename W>
	void install_readwrite_handler(u16 addrstart, u16 addrend, R &&rhandler, W &&whandler)
	{
		address_space &space(cartridge_space());
		space.install_read_handler(addrstart, addrend, std::forward<R>(rhandler));
		space.install_write_handler(addrstart, addrend, std::forward<W>(whandler));
	}

	// setting line values
	void set_line_value(cococart_slot_device::line line, cococart_slot_device::line_value value);
	void set_line_value(cococart_slot_device::line line, bool value) { set_line_value(line, value ? cococart_slot_device::line_value::ASSERT : cococart_slot_device::line_value::CLEAR); }

	typedef cococart_slot_device::line line;
	typedef cococart_slot_device::line_value line_value;

private:
	cococart_base_update_delegate    m_update;
	cococart_slot_device * const     m_owning_slot;
	device_cococart_host_interface * m_host;
};

// methods for configuring CoCo slot devices (the expansion cart
// itself, as well as slots on the Multi-Pak)
void coco_cart_add_basic_devices(device_slot_interface &device);
void coco_cart_add_fdcs(device_slot_interface &device);
void coco_cart_add_multi_pak(device_slot_interface &device);
void dragon_cart_add_basic_devices(device_slot_interface &device);
void dragon_cart_add_fdcs(device_slot_interface &device);
void dragon_cart_add_multi_pak(device_slot_interface &device);

#endif // MAME_BUS_COCO_COCOCART_H
