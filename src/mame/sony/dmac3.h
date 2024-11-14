// license:BSD-3-Clause
// copyright-holders:Brice Onken,Tsubai Masanari
// thanks-to:Patrick Mackinlay

/*
 * Sony CXD8403Q DMAC3 DMA controller
 *
 * Register definitions were derived from the NetBSD source code, copyright (c) 2000 Tsubai Masanari.
 *
 * References:
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/dmac3reg.h
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/dmac3var.h
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/dmac3.c
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifi.c
 */

#ifndef MAME_SONY_DMAC3_H
#define MAME_SONY_DMAC3_H

#pragma once

class dmac3_device : public device_t
{
public:
	dmac3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// DMAC3 has two controllers on-chip
	enum dmac3_controller
	{
		CTRL0 = 0,
		CTRL1 = 1,
	};

	// Address map setup
	template <typename... T>
	void set_apbus_address_translator(T &&...args) { m_apbus_virt_to_phys_callback.set(std::forward<T>(args)...); }
	template <dmac3_controller Controller>
	void map(address_map &map)
	{
		map(0x0, 0x3).rw(FUNC(dmac3_device::csr_r<Controller>), FUNC(dmac3_device::csr_w<Controller>));
		map(0x4, 0x7).rw(FUNC(dmac3_device::intr_r<Controller>), FUNC(dmac3_device::intr_w<Controller>));
		map(0x8, 0xb).rw(FUNC(dmac3_device::length_r<Controller>), FUNC(dmac3_device::length_w<Controller>));
		map(0xc, 0xf).rw(FUNC(dmac3_device::address_r<Controller>), FUNC(dmac3_device::address_w<Controller>));
		map(0x10, 0x13).rw(FUNC(dmac3_device::conf_r<Controller>), FUNC(dmac3_device::conf_w<Controller>));
	}

	// Signal routing
	template <typename T>
	void set_bus(T &&tag, int spacenum) { m_bus.set_tag(std::forward<T>(tag), spacenum); }
	template <dmac3_controller Controller>
	auto dma_r_cb() { return m_dma_r[Controller].bind(); }
	template <dmac3_controller Controller>
	auto dma_w_cb() { return m_dma_w[Controller].bind(); }
	auto irq_out() { return m_irq_handler.bind(); }

	template <dmac3_controller Controller>
	void irq_w(int state)
	{
		if (state)
		{
			m_controllers[Controller].intr |= INTR_INT;
		}
		else
		{
			m_controllers[Controller].intr &= ~INTR_INT;
		}
		m_irq_check->adjust(attotime::zero);
	}

	template <dmac3_controller Controller>
	void drq_w(int state)
	{
		m_controllers[Controller].drq = (state != 0);
		m_dma_check->adjust(attotime::zero);
	}

protected:
	// Bitmasks for DMAC3 registers
	enum DMAC3_CSR_MASKS : uint32_t
	{
		CSR_SEND = 0x0000,
		CSR_ENABLE = 0x0001,
		CSR_RECV = 0x0002,
		CSR_RESET = 0x0004,
		CSR_APAD = 0x0008,
		CSR_MBURST = 0x0010,
		CSR_DBURST = 0x0020,
	};

	enum DMAC3_INTR_MASKS : uint32_t
	{
		INTR_INT = 0x0001,
		INTR_INTEN = 0x0002,
		INTR_TCIE = 0x0020,
		INTR_TCI = 0x0040,
		INTR_EOP = 0x0100,
		INTR_EOPIE = 0x0200, // End of operation interrupt enable
		INTR_EOPI = 0x0400,
		INTR_DREQ = 0x1000,  // Is this just DRQ?
		INTR_DRQIE = 0x2000, // Interrupt on DRQ enable?
		INTR_DRQI = 0x4000,
		INTR_PERR = 0x8000,
	};

	// It is not fully clear what IPER, DERR, MPER are signalling.
	// NetBSD ignores IPER and MPER, but resets the DMAC if DERR is asserted during the interrupt routine
	// DCEN and PCEN are set by NetBSD during attach
	enum DMAC3_CONF_MASKS : uint32_t
	{
		CONF_IPER = 0x8000,
		CONF_MPER = 0x4000,
		CONF_PCEN = 0x2000,
		CONF_DERR = 0x1000,
		CONF_DCEN = 0x0800,
		CONF_ODDP = 0x0200,
		CONF_WIDTH = 0x00ff,
		CONF_SLOWACCESS = 0x0020, // SPIFI access mode (see NetBSD source code)
		CONF_FASTACCESS = 0x0001, // DMAC3 access mode (see NetBSD source code)
	};

	struct dmac3_register_file
	{
		uint32_t csr = 0;     // Status register
		uint32_t intr = 0;    // Interrupt status register
		uint32_t length = 0;  // Transfer count register
		uint32_t address = 0; // Starting byte address (APbus virtual or physical)
		uint32_t conf = 0;    // Transaction configuration register
		bool drq = false;     // TODO: Does the DMAC3 use INTR_DREQ as the DRQ?
	} m_controllers[2];

	// Connections to other devices
	// TODO: DMAC3 probably transfers more than one byte at a time
	required_address_space m_bus;
	devcb_write_line m_irq_handler;
	devcb_read8::array<2> m_dma_r;
	devcb_write8::array<2> m_dma_w;
	device_delegate<uint32_t(uint32_t)> m_apbus_virt_to_phys_callback;

	// Timers and interrupts
	emu_timer *m_irq_check;
	emu_timer *m_dma_check;
	bool m_irq = false;

	// Overrides from device_t
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// Other methods
	void reset_controller(dmac3_controller controller);

	// Register file accessors
	uint32_t csr_r(dmac3_controller controller);
	uint32_t intr_r(dmac3_controller controller);
	uint32_t length_r(dmac3_controller controller);
	uint32_t address_r(dmac3_controller controller);
	uint32_t conf_r(dmac3_controller controller);

	void csr_w(dmac3_controller controller, uint32_t data);
	void intr_w(dmac3_controller controller, uint32_t data);
	void length_w(dmac3_controller controller, uint32_t data);
	void address_w(dmac3_controller controller, uint32_t data);
	void conf_w(dmac3_controller controller, uint32_t data);

	template <dmac3_controller Controller>
	uint32_t csr_r() { return csr_r(Controller); }
	template <dmac3_controller Controller>
	uint32_t intr_r() { return intr_r(Controller); }
	template <dmac3_controller Controller>
	uint32_t length_r() { return length_r(Controller); }
	template <dmac3_controller Controller>
	uint32_t address_r() { return address_r(Controller); }
	template <dmac3_controller Controller>
	uint32_t conf_r() { return conf_r(Controller); }

	template <dmac3_controller Controller>
	void csr_w(uint32_t data) { csr_w(Controller, data); }
	template <dmac3_controller Controller>
	void intr_w(uint32_t data) { intr_w(Controller, data); }
	template <dmac3_controller Controller>
	void length_w(uint32_t data) { length_w(Controller, data); }
	template <dmac3_controller Controller>
	void address_w(uint32_t data) { address_w(Controller, data); }
	template <dmac3_controller Controller>
	void conf_w(uint32_t data) { conf_w(Controller, data); }

	// Timer callback methods
	TIMER_CALLBACK_MEMBER(irq_check);
	TIMER_CALLBACK_MEMBER(dma_check);
};

DECLARE_DEVICE_TYPE(DMAC3, dmac3_device)

#endif // MAME_SONY_DMAC3
