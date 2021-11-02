// license:BSD-3-Clause
// copyright-holders:Brice Onken

/*
 * Sony CXD8442Q WSC-FIFOQ APbus FIFO and DMA controller
 *
 * The FIFOQ is an APbus DMA controller designed for interfacing some of the simpler and lower speed peripherals
 * to the APbus while providing DMA capabilities. Each FIFO chip can support up to 4 devices.
 */

#ifndef MAME_MACHINE_CXD8442Q_H
#define MAME_MACHINE_CXD8442Q_H

#pragma once

#include "emu.h"
#include "device.h"
#include "devfind.h"
#include "mconfig.h"

class cxd8442q_device;

// Class that represents a single FIFO channel. Each instance of the FIFOQ chip has 4 of these.
class apfifo_channel
{
public:
	apfifo_channel(cxd8442q_device &fifo_device) : fifo_device(fifo_device) {}

	static const int DMA_EN = 0x1;
	static const int DMA_DIRECTION = 0x2; // 1 = out, 0 = in

	// Mask = total bytes avaliable for use by this channel
	uint32_t mask = 0;

	// Start address in FIFO RAM (meaning that the channel uses [address, address + mask])
	uint32_t address = 0;

	// Enables/disables DMA execution and sets the direction
	uint32_t dma_mode = 0;

	// Controls interrupt masking
	uint32_t intctrl = 0;

	// Provides interrupt status
	uint32_t intstat = 0;

	// Count of data to transfer or data recieved
	uint32_t count = 0;

	void reset()
	{
		mask = 0;
		address = 0;  // start address in FIFO RAM
		dma_mode = 0; // fifo dma mode
		intctrl = 0;  // interrupt control
		intstat = 0;  // interrupt status
		count = 0;
		drq = false;
	}

	bool dma_cycle();

	void set_dma_r_callback(std::function<uint32_t(void)> dma_r)
	{
		dma_r_callback = dma_r;
	}

	void set_dma_w_callback(std::function<void(uint32_t)> dma_w)
	{
		dma_w_callback = dma_w;
	}

	// Emulates the FIFO data port
	uint32_t read_data_from_fifo();
	void write_data_to_fifo(uint32_t data);

	void drq_w(int state);

	bool drq_r()
	{
		return drq;
	}

private:
	// reference to parent device
	cxd8442q_device &fifo_device;

	// state tracking
	bool drq = false;

	// data pointers (I think within [address, address + mask])
	uint32_t fifo_w_position = 0;
	uint32_t fifo_r_position = 0;

	// Callback pointers
	std::function<uint32_t(void)> dma_r_callback;
	std::function<void(uint32_t)> dma_w_callback;
};

class cxd8442q_device : public device_t
{
public:
	cxd8442q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map);
	void map_fifo_ram(address_map &map);

	auto out_int_callback() { return out_irq.bind(); }

	// FIFO channels
	enum fifo_channel_number
	{
		CH0 = 0,
		CH1 = 1,
		CH2 = 2,
		CH3 = 3
	};

	// Each FIFO chip has 4 channels
	static const int FIFO_CH_TOTAL = 4;

	// 128KiB used as the FIFO RAM (can be divided up to 4 regions, 1 per channel)
	static const int FIFO_MAX_RAM_SIZE = 0x20000;

	// offset from one channel to the next
	static const int FIFO_REGION_OFFSET = 0x10000;

	// offset from the channel 0 control register to the RAM
	static const int FIFO_RAM_OFFSET = 0x80000;

	// DMA emulation
	template <fifo_channel_number channel_number>
	void drq_w(int state) { fifo_channels[channel_number].drq_w(state); };
	template <fifo_channel_number channel_number>
	void bind_dma_r(std::function<uint32_t(void)> dma_r) { fifo_channels[channel_number].set_dma_r_callback(dma_r); };
	template <fifo_channel_number channel_number>
	void bind_dma_w(std::function<void(uint32_t)> dma_w) { fifo_channels[channel_number].set_dma_w_callback(dma_w); };

protected:
	// FIFO RAM
	std::unique_ptr<uint32_t[]> fifo_ram;

	// TODO: figure out the real clock rate for this
	static const int DMA_TIMER = 1;
	emu_timer *fifo_timer;
	TIMER_CALLBACK_MEMBER(fifo_dma_execute);

	// Interrupts
	devcb_write_line out_irq;
	void device_resolve_objects() override { out_irq.resolve_safe(); }
	void irq_check();

	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	apfifo_channel fifo_channels[FIFO_CH_TOTAL];
	friend class apfifo_channel;
};

DECLARE_DEVICE_TYPE(CXD8442Q, cxd8442q_device)

#endif // MAME_MACHINE_CXD8442Q_H
