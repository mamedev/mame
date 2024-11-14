// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SEGA_MIE_H
#define MAME_SEGA_MIE_H

#pragma once

#include "mapledev.h"

#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/jvshost.h"


class mie_jvs_device;

// ======================> mie_device

class mie_device : public maple_device
{
public:
	template <typename T, typename U>
	mie_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&host_tag, int host_port, U &&jvs_tag)
		: mie_device(mconfig, tag, owner, clock)
	{
		host.set_tag(std::forward<T>(host_tag));
		set_host_port(host_port);
		jvs.set_tag(std::forward<U>(jvs_tag));
	}

	mie_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <uint8_t Which, typename T>
	void set_gpio_name(T &&gpio_port_tag) { gpio_port[Which].set_tag(std::forward<T>(gpio_port_tag)); }
	template <uint8_t First = 0U, typename T, typename... U>
	void set_gpio_names(T &&first_tag, U &&... other_tags)
	{
		set_gpio_name<First>(std::forward<T>(first_tag));
		set_gpio_names<First + 1>(std::forward<U>(other_tags)...);
	}

	uint8_t control_r(offs_t offset);
	void control_w(offs_t offset, uint8_t data);
	uint8_t lreg_r();
	void lreg_w(uint8_t data);
	uint8_t tbuf_r(offs_t offset);
	void tbuf_w(offs_t offset, uint8_t data);
	uint8_t gpio_r(offs_t offset);
	void gpio_w(offs_t offset, uint8_t data);
	uint8_t gpiodir_r();
	void gpiodir_w(uint8_t data);
	uint8_t adc_r();
	void adc_w(uint8_t data);

	uint8_t irq_enable_r();
	void irq_enable_w(uint8_t data);
	uint8_t maple_irqlevel_r();
	void maple_irqlevel_w(uint8_t data);
	uint8_t irq_pending_r();
	void irq_pending_w(uint8_t data);

	uint8_t jvs_r();
	void jvs_w(uint8_t data);
	void jvs_dest_w(uint8_t data);
	uint8_t jvs_status_r();
	void jvs_control_w(uint8_t data);
	uint8_t jvs_sense_r();
	void jvs_lcr_w(uint8_t data);

	uint8_t read_ff();
	uint8_t read_00();
	uint8_t read_78xx(offs_t offset);

	void maple_w(const uint32_t *data, uint32_t in_size) override;
	virtual void maple_reset() override;

	void mie_map(address_map &map) ATTR_COLD;
	void mie_port(address_map &map) ATTR_COLD;

protected:
	template <uint8_t First> void set_gpio_names() { }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_irq_reply);

private:
	enum { TBUF_SIZE = 8 };

	enum {
		CTRL_TXB  = 0x000001, // Start transmit
		CTRL_CTXB = 0x000002, // Continue transmit flag (when doing multipart)
		CTRL_ENDP = 0x000004, // Add an end pattern
		CTRL_EMP  = 0x000100, // Empty flag
		CTRL_PERR = 0x000200, // Parity error
		CTRL_BFOV = 0x000800, // Set when overflow, cleared to 0 when starting send/receive
		CTRL_RXB  = 0x001000, // Receiving
		CTRL_RFB  = 0x002000, // Receive done
		CTRL_TFB  = 0x004000, // Transmit done
		CTRL_HRES = 0x008000  // Reset pattern received
	};

	// internal state
	required_device<cpu_device> cpu;
	emu_timer *timer = nullptr;
	required_device<mie_jvs_device> jvs;
	optional_ioport_array<8> gpio_port;

	uint32_t tbuf[TBUF_SIZE]{};
	uint32_t control = 0, lreg = 0, jvs_rpos = 0;
	uint8_t gpiodir = 0, gpio_val[8]{};
	uint8_t irq_enable = 0, irq_pending = 0, maple_irqlevel = 0;
	uint8_t jvs_control = 0, jvs_dest = 0;
	uint8_t jvs_lcr = 0;

	void raise_irq(int level);
	void recalc_irq();
	IRQ_CALLBACK_MEMBER(irq_callback);
};

// Trampoline class, required for device discovery
class mie_jvs_device : public jvs_host
{
public:
	friend class mie_device;

	// construction/destruction
	mie_jvs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(MIE,     mie_device)
DECLARE_DEVICE_TYPE(MIE_JVS, mie_jvs_device)

#endif // MAME_SEGA_MIE_H
