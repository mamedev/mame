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
class hp98032_gpio_slot_device;

class hp98032_io_card_device : public hp9845_io_card_device
{
public:
	// construction/destruction
	hp98032_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp98032_io_card_device();

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ16_MEMBER(reg_r) override;
	virtual DECLARE_WRITE16_MEMBER(reg_w) override;

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

	DECLARE_WRITE_LINE_MEMBER(pflg_w);
	DECLARE_WRITE_LINE_MEMBER(psts_w);
	DECLARE_WRITE_LINE_MEMBER(eir_w);

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
								 public device_slot_interface
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
	DECLARE_WRITE_LINE_MEMBER(pflg_w);
	DECLARE_WRITE_LINE_MEMBER(psts_w);
	DECLARE_WRITE_LINE_MEMBER(eir_w);

	// Write to output signals
	DECLARE_WRITE_LINE_MEMBER(pctl_w);
	DECLARE_WRITE_LINE_MEMBER(io_w);
	DECLARE_WRITE_LINE_MEMBER(preset_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line m_pflg_handler;
	devcb_write_line m_psts_handler;
	devcb_write_line m_eir_handler;

};

// A device connected to GPIO port of HP98032
class hp98032_gpio_card_device : public device_t,
								 public device_slot_card_interface
{
public:
	virtual uint16_t get_jumpers() const = 0;
	virtual uint16_t input_r() const = 0;
	virtual uint8_t ext_status_r() const = 0;
	virtual void output_w(uint16_t data) = 0;
	virtual void ext_control_w(uint8_t data) = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(pctl_w) = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(io_w) = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(preset_w) = 0;

protected:
	// construction/destruction
	hp98032_gpio_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp98032_gpio_card_device();

	DECLARE_WRITE_LINE_MEMBER(pflg_w);
	DECLARE_WRITE_LINE_MEMBER(psts_w);
	DECLARE_WRITE_LINE_MEMBER(eir_w);
};

// GPIO loopback connector for HP98032
class hp98032_gpio_loopback_device : public hp98032_gpio_card_device
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
	virtual DECLARE_WRITE_LINE_MEMBER(pctl_w) override;
	virtual DECLARE_WRITE_LINE_MEMBER(io_w) override;
	virtual DECLARE_WRITE_LINE_MEMBER(preset_w) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
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
