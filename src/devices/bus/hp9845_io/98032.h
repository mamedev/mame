// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    98032.h

    98032 module (GPIO interface)

*********************************************************************/

#ifndef MAME_BUS_HP9845_IO_98032_H
#define MAME_BUS_HP9845_IO_98032_H

#pragma once

#include "hp9845_io.h"

class device_hp98032_gpio_interface;
class hp98032_gpio_slot_device;

class hp98032_io_card_device : public device_t, public device_hp9845_io_interface
{
public:
	// construction/destruction
	hp98032_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp98032_io_card_device();

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual uint16_t reg_r(address_space &space, offs_t offset) override;
	virtual void reg_w(address_space &space, offs_t offset, uint16_t data) override;

private:
	required_device<hp98032_gpio_slot_device> m_gpio;

	uint16_t m_output;
	uint16_t m_input;
	bool m_int_en;
	bool m_dma_en;
	bool m_busy;    // U5B
	bool m_pready;
	bool m_flag;
	bool m_auto_ah;
	bool m_eir;

	void pflg_w(int state);
	void psts_w(int state);
	void eir_w(int state);

	void start_hs();
	void set_busy(bool state);
	void update_flag();
	void update_irq();
	void update_dmar();
	void latch_input_MSB();
	void latch_input_LSB();
};

// The GPIO interface of HP98032 cards
class hp98032_gpio_slot_device : public device_t,
								 public device_single_card_slot_interface<device_hp98032_gpio_interface>
{
public:
	// construction/destruction
	hp98032_gpio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp98032_gpio_slot_device();

	// Bits in jumper configuration
	enum : uint16_t {
		JUMPER_1 = (1U << 0),   // Invert input data
		JUMPER_2 = (1U << 1),   // Invert output data
		JUMPER_3 = (1U << 2),   // Invert PCTL
		JUMPER_4 = (1U << 3),   // Invert PFLG
		JUMPER_5 = (1U << 4),   // Invert PSTS
		JUMPER_6 = (1U << 5),   // Pulse-mode HS
		JUMPER_7 = (1U << 6),   // DMA enable
		JUMPER_8 = (1U << 7),   // Latch input MSB when BUSY falls
		JUMPER_9 = (1U << 8),   // Latch input MSB when PREADY rises
		JUMPER_A = (1U << 9),   // Latch input MSB when reading R6
		JUMPER_B = (1U << 10),  // Word input mode
		JUMPER_C = (1U << 11),  // Latch input LSB when reading R4
		JUMPER_D = (1U << 12),  // Latch input LSB when PREADY rises
		JUMPER_E = (1U << 13),  // Latch input LSB when BUSY falls
		JUMPER_F = (1U << 14)   // Word output mode
	};

	// Get jumper configuration
	uint16_t get_jumpers() const;
	bool is_jumper_present(uint16_t mask) const { return (get_jumpers() & mask) != 0; }

	// All I/O signals on 98032 GPIO port use negative logic. Data bits are inverted.
	// Here we use positive logic & non-inverted data bits.

	// Read input port
	uint16_t input_r() const;

	// Read extended status (2 bits)
	uint8_t ext_status_r() const;

	// Write to output port
	void output_w(uint16_t data);

	// Write to extended controls (2 bits)
	void ext_control_w(uint8_t data);

	// Input signal callbacks
	auto pflg_cb() { return m_pflg_handler.bind(); }
	auto psts_cb() { return m_psts_handler.bind(); }
	auto eir_cb() { return m_eir_handler.bind(); }

	// Write to input signals (for card devices)
	void pflg_w(int state);
	void psts_w(int state);
	void eir_w(int state);

	// Write to output signals
	void pctl_w(int state);
	void io_w(int state);
	void preset_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_write_line m_pflg_handler;
	devcb_write_line m_psts_handler;
	devcb_write_line m_eir_handler;

};

// A device connected to GPIO port of HP98032
class device_hp98032_gpio_interface : public device_interface
{
public:
	virtual ~device_hp98032_gpio_interface();

	virtual uint16_t get_jumpers() const = 0;
	virtual uint16_t input_r() const = 0;
	virtual uint8_t ext_status_r() const = 0;
	virtual void output_w(uint16_t data) = 0;
	virtual void ext_control_w(uint8_t data) = 0;
	virtual void pctl_w(int state) = 0;
	virtual void io_w(int state) = 0;
	virtual void preset_w(int state) = 0;

protected:
	device_hp98032_gpio_interface(const machine_config &mconfig, device_t &device);

	void pflg_w(int state);
	void psts_w(int state);
	void eir_w(int state);
};

// GPIO loopback connector for HP98032
class hp98032_gpio_loopback_device : public device_t, public device_hp98032_gpio_interface
{
public:
	// construction/destruction
	hp98032_gpio_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp98032_gpio_loopback_device();

	virtual uint16_t get_jumpers() const override;
	virtual uint16_t input_r() const override;
	virtual uint8_t ext_status_r() const override;
	virtual void output_w(uint16_t data) override;
	virtual void ext_control_w(uint8_t data) override;
	virtual void pctl_w(int state) override;
	virtual void io_w(int state) override;
	virtual void preset_w(int state) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_output;
	uint8_t m_ext_control;
	bool m_io;
};

// device type definitions
DECLARE_DEVICE_TYPE(HP98032_IO_CARD, hp98032_io_card_device)
DECLARE_DEVICE_TYPE(HP98032_GPIO_SLOT , hp98032_gpio_slot_device)
DECLARE_DEVICE_TYPE(HP98032_GPIO_LOOPBACK , hp98032_gpio_loopback_device)

#endif // MAME_BUS_HP9845_IO_98032_H
