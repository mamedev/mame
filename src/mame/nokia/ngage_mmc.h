// license:GPL-2.0+
// copyright-holders:Andrea Bogazzi
#ifndef MAME_NOKIA_NGAGE_MMC_H
#define MAME_NOKIA_NGAGE_MMC_H

#pragma once

class ngage_mmc_device : public device_t, public device_image_interface
{
public:
	ngage_mmc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	auto event_cb() { return m_event_cb.bind(); }
	u32 read(offs_t offset, u32 mem_mask);
	void write(offs_t offset, u32 data, u32 mem_mask);
	u32 irq_sources() const;
	bool dma_ready(bool write) const;
	bool mounted() const { return m_payload_size != 0; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual bool is_readable() const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual char const *image_type_name() const noexcept override { return "mmccard"; }
	virtual char const *image_brief_type_name() const noexcept override { return "mmc"; }
	virtual char const *file_extensions() const noexcept override { return "mmc,img,bin"; }

private:
	enum : u8 { IDLE, READY, IDENT, STANDBY, TRANSFER, DATA, RECEIVE, INACTIVE };
	enum : u8 { FIFO_IDLE, FIFO_RX, FIFO_TX };
	static constexpr u32 DATA_BUSY = 0x004, CMD_BUSY = 0x008, RESP_TIMEOUT = 0x040;
	static constexpr u32 DATA_TIMEOUT = 0x080, RESP_READY = 0x100, DATA_ERROR = 0x400;

	void start(u32 control);
	void card_command();
	void response(u32 value);
	void response(u8 const *bytes);
	void begin_data(bool write);
	void end_data();
	u32 response_r();
	u32 fifo_r(u32 mask);
	void fifo_w(u32 data, u32 mask);
	bool in_range(u64 offset, u32 length) const;
	TIMER_CALLBACK_MEMBER(command_complete);
	TIMER_CALLBACK_MEMBER(command_tail);

	devcb_write_line m_event_cb;
	emu_timer *m_command_timer = nullptr;
	emu_timer *m_tail_timer = nullptr;

	u64 m_payload_size = 0;
	u64 m_card_capacity = 0;
	u8 m_cid[16]{};
	u8 m_csd[16]{};
	u32 m_ocr = 0;
	bool m_write_protected = false;

	u32 m_reg[16]{};
	u32 m_status = 0;
	u8 m_type = 0;
	u8 m_command = 0;
	u32 m_argument = 0;
	u32 m_response[4]{};
	u8 m_response_words = 0;
	u8 m_response_pos = 0;
	u8 m_card_state = IDLE;
	u16 m_rca = 1;
	u16 m_block_length = 512;
	bool m_multiblock = false;
	u64 m_next_address = 0;

	u8 m_fifo_direction = FIFO_IDLE;
	u8 m_fifo[4096]{};
	u16 m_fifo_length = 0;
	u16 m_fifo_pos = 0;
};

DECLARE_DEVICE_TYPE(NGAGE_MMC, ngage_mmc_device)

#endif // MAME_NOKIA_NGAGE_MMC_H
