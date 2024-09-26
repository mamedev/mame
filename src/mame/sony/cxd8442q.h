// license:BSD-3-Clause
// copyright-holders:Brice Onken

/*
 * Sony CXD8442Q WSC-FIFOQ APbus FIFO and DMA controller
 *
 * The FIFOQ is an APbus DMA controller designed for interfacing some of the simpler and lower speed peripherals
 * to the APbus while providing DMA capabilities. Each FIFO chip can support up to 4 devices.
 */

#ifndef MAME_SONY_CXD8442Q_H
#define MAME_SONY_CXD8442Q_H

#pragma once

class cxd8442q_device : public device_t
{
protected:
	// Class that represents a single FIFO channel. Each instance of the FIFOQ chip has 4 of these.
	class apfifo_channel
	{
	public:
		apfifo_channel(cxd8442q_device &fifo_device) : fifo_device(fifo_device), dma_r_callback(fifo_device, 0), dma_w_callback(fifo_device)
		{
		}

		static constexpr uint32_t DMA_EN = 0x1;
		static constexpr uint32_t DMA_DIRECTION = 0x2; // 1 = out, 0 = in

		// total bytes avaliable for use by this channel
		uint32_t fifo_size = 0;

		// Start address in FIFO RAM (meaning that the channel uses [address, address + fifo_size])
		uint32_t address = 0;

		// Enables/disables DMA execution and sets the direction
		uint32_t dma_mode = 0;

		// Controls interrupt masking
		uint32_t intctrl = 0;

		// Provides interrupt status
		uint32_t intstat = 0;

		// Count of data to transfer or data received
		uint32_t count = 0;

		// DRQ status
		bool drq = false;

		// data pointers (within [address, address + fifo_size])
		uint32_t fifo_w_position = 0;
		uint32_t fifo_r_position = 0;

		void reset();

		bool dma_check();

		auto dma_r_cb() { return dma_r_callback.bind(); }

		auto dma_w_cb() { return dma_w_callback.bind(); }

		// Emulates the FIFO data port
		uint32_t read_data_from_fifo();
		void write_data_to_fifo(uint32_t data);

		void drq_w(int state);

	private:
		// reference to parent device
		cxd8442q_device &fifo_device;

		// DMA callback pointers
		devcb_read8 dma_r_callback;
		devcb_write8 dma_w_callback;

		bool dma_cycle();
	};

public:
	cxd8442q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map) ATTR_COLD;
	void map_fifo_ram(address_map &map) ATTR_COLD;

	auto out_int_callback() { return out_irq.bind(); }

	// FIFO channels
	enum fifo_channel_number
	{
		CH0 = 0,
		CH1 = 1,
		CH2 = 2,
		CH3 = 3
	};

	// DMA emulation
	template <fifo_channel_number ChannelNumber>
	void drq_w(int state) { fifo_channels[ChannelNumber].drq_w(state); };
	template <fifo_channel_number ChannelNumber>
	auto dma_r_cb() { return fifo_channels[ChannelNumber].dma_r_cb(); };
	template <fifo_channel_number ChannelNumber>
	auto dma_w_cb() { return fifo_channels[ChannelNumber].dma_w_cb(); };

protected:
	static constexpr int FIFO_CH_TOTAL = 4;

	std::unique_ptr<uint32_t[]> fifo_ram;
	emu_timer *fifo_timer;
	devcb_write_line out_irq;
	apfifo_channel fifo_channels[FIFO_CH_TOTAL];

	TIMER_CALLBACK_MEMBER(fifo_dma_execute);

	void irq_check();

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// FIFO operations
	uint32_t read_fifo_size(offs_t offset);
	void write_fifo_size(offs_t offset, uint32_t data);
	uint32_t read_address(offs_t offset);
	void write_address(offs_t offset, uint32_t data);
	uint32_t read_dma_mode(offs_t offset);
	void write_dma_mode(offs_t offset, uint32_t data);
	uint32_t read_intctrl(offs_t offset);
	void write_intctrl(offs_t offset, uint32_t data);
	uint32_t read_intstat(offs_t offset);
	uint32_t read_count(offs_t offset);
	uint8_t read_fifo_data(offs_t offset);
	void write_fifo_data(offs_t offset, uint8_t data);
	uint32_t read_fifo_ram(offs_t offset);
	void write_fifo_ram(offs_t offset, uint32_t data, uint32_t mem_mask);
};

DECLARE_DEVICE_TYPE(CXD8442Q, cxd8442q_device)

#endif // MAME_SONY_CXD8442Q_H
