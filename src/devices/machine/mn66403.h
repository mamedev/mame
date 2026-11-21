// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_MN66403_H
#define MAME_MACHINE_MN66403_H

#pragma once

#include "nscsi_bus.h"

class mn664031_device : public device_t, public nscsi_device_interface
{
public:
	mn664031_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto gpio_callback() { return m_gpio_cb.bind(); }
	auto irq_callback() { return m_irq_cb.bind(); }
	auto decoder_irq_callback() { return m_decoder_irq_cb.bind(); }

	void map(address_map &map) ATTR_COLD;

	void sector_w(const u8 *data);
	void subcode_w(const u8 *q);
	int irq_r() const { return m_irq; }
	int scsi_id() const { return m_scsi_id; }

	// nscsi_device_interface implementation
	virtual void scsi_ctrl_changed() override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum : u8 { BUS_FREE, SELECTED, COMMAND, MESSAGE_OUT, DATA_IN, DATA_OUT, STATUS, MESSAGE_IN, WAIT_COMMAND };

	static constexpr u32 BUFFER_SIZE = 0x40000;

	u8 reg_r(offs_t offset);
	void reg_w(offs_t offset, u8 data);
	template <offs_t Base> u8 decoder_r(offs_t offset) { return m_decoder_regs[Base + offset]; }
	u8 decoder_ack_r();
	u8 sector_ack_r();
	u8 dma_blocks_low_r();
	u8 dma_blocks_high_r();
	u8 command_fifo_r();
	u8 data_fifo_r();
	u8 message_r();
	u8 status_r();
	u8 sequencer_r();
	u8 buffer_r();
	u8 semaphore_r();
	void capture_control_w(u8 data);
	void pio_count_w(u8 data);
	void scsi_status_w(u8 data);
	void data_fifo_w(u8 data);
	void message_w(u8 data);
	void abort_w(u8 data);
	void flush_w(u8 data);
	void sequencer_w(u8 data);
	void command_extension_w(u8 data);
	void buffer_w(u8 data);
	void pending_w(u8 data);
	void irq_mask_w(offs_t offset, u8 data);
	void gpio_w(u8 data);

	TIMER_CALLBACK_MEMBER(bus_step);
	void set_phase(u8 state, u32 phase);
	void request_byte();
	void finish_transfer(u8 result = 0);
	void abort_transfer();
	void update_irq();
	void interrupt(u8 bits);
	u32 buffer_address() const;
	void advance_buffer();
	void capture_sector();
	void finish_dma();

	memory_share_creator<u8> m_buffer;
	devcb_write8 m_gpio_cb;
	devcb_write_line m_irq_cb;
	devcb_write_line m_decoder_irq_cb;

	emu_timer *m_bus_timer;

	u8 m_regs[0x40];
	u8 m_decoder_regs[0x14];
	u8 m_sector[2352];
	u8 m_input_sectors[3][2352];
	u8 m_input_index;
	u8 m_input_count;
	u8 m_headers[2][4];
	u32 m_sector_addresses[2];
	u8 m_pipeline_valid;
	u8 m_decoder_pending;
	bool m_decoder_irq;
	u32 m_capture_address;
	u32 m_subcode_address;
	u32 m_subcode_addresses[3];
	u8 m_subcode_index;
	u8 m_subcode_count;
	u8 m_command_fifo[16];
	u8 m_data_fifo[16];
	u8 m_data_count;
	u8 m_data_read;
	u8 m_fifo_count;
	u8 m_fifo_read;
	u8 m_command_size;
	u8 m_pending;
	u8 m_state;
	u8 m_status;
	u8 m_message;
	u8 m_message_fifo[16];
	u8 m_message_count;
	u8 m_message_read;
	bool m_message_valid;
	bool m_disconnect;
	bool m_reset;
	bool m_irq;
	bool m_ack;
	u32 m_dma_address;
	u32 m_dma_remaining;
	u32 m_dma_block_size;
	u32 m_dma_block_address;
	u16 m_dma_blocks;
	u8 m_dma_command;
};

DECLARE_DEVICE_TYPE(MN664031, mn664031_device)

#endif // MAME_MACHINE_MN66403_H
