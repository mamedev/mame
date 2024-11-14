// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    atahle.h

    ATA Device HLE

***************************************************************************/
#ifndef MAME_MACHINE_ATAHLE_H
#define MAME_MACHINE_ATAHLE_H

#pragma once

class ata_hle_device_base : public device_t
{
protected:
	ata_hle_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(busy_tick);
	TIMER_CALLBACK_MEMBER(empty_tick);

	uint16_t dma_r();
	uint16_t command_r(offs_t offset);
	uint16_t control_r(offs_t offset);

	void dma_w(uint16_t data);
	void command_w(offs_t offset, uint16_t data);
	void control_w(offs_t offset, uint16_t data);

	void set_csel_in(int state) { m_csel = state; }
	void set_dasp_in(int state) { m_daspin = state; }
	void set_dmack_in(int state);
	void set_pdiag_in(int state);

	void set_irq(int state)
	{
		if (m_irq != state)
		{
			m_irq = state;
			update_irq();
		}
	}

	void set_dmarq(int state)
	{
		if (m_dmarq != state)
		{
			m_dmarq = state;
			set_dmarq_out(state);
		}
	}

	void set_dasp(int state)
	{
		if (m_daspout != state)
		{
			m_daspout = state;
			set_dasp_out(state);
		}
	}

	void set_pdiag(int state)
	{
		if (m_pdiagout != state)
		{
			m_pdiagout = state;
			set_pdiag_out(state);
		}
	}

	void start_busy(const attotime &time, int32_t param);
	void stop_busy();

	int dev() { return (m_device_head & IDE_DEVICE_HEAD_DRV) >> 4; }
	bool device_selected() { return m_csel == dev(); }

	virtual uint8_t calculate_status() { return m_status; }
	virtual void soft_reset();
	virtual void process_command();
	virtual void finished_command();
	virtual bool set_features();
	virtual int sector_length() = 0;
	virtual void process_buffer() = 0;
	virtual void fill_buffer() = 0;
	virtual bool is_ready() = 0;
	virtual void perform_diagnostic() = 0;
	virtual void signature() = 0;
	virtual uint16_t read_data();
	virtual void write_data(uint16_t data);

	int bit_to_mode(uint16_t word);
	int single_word_dma_mode();
	int multi_word_dma_mode();
	int ultra_dma_mode();

	/// TODO: not sure this should be protected.
	void read_buffer_empty();

	enum
	{
		IDE_STATUS_ERR = 0x01, // Error
		IDE_STATUS_IDX = 0x02, // Index
		IDE_STATUS_CORR = 0x04, // Corrected Data
		IDE_STATUS_DRQ = 0x08, // Data Request
		IDE_STATUS_DSC = 0x10, // ATA Drive Seek Complete
		IDE_STATUS_SERV = 0x10, // ATAPI Service
		IDE_STATUS_DWF = 0x20, // ATA Drive Write Fault
		IDE_STATUS_DMRD = 0x20, // ATAPI DMA Ready
		IDE_STATUS_DRDY = 0x40, // Drive Ready
		IDE_STATUS_BSY = 0x80 // Busy
	};

	enum
	{
		IDE_ERROR_NONE = 0x00,
		IDE_ERROR_DIAGNOSTIC_OK = 0x01,
		IDE_ERROR_TRACK0_NOT_FOUND = 0x02,
		IDE_ERROR_ABRT = 0x04,
		IDE_ERROR_BAD_LOCATION = 0x10,
		IDE_ERROR_BAD_SECTOR = 0x80,
		IDE_ERROR_DIAGNOSTIC_FAILED = 0x00,
		IDE_ERROR_DIAGNOSTIC_PASSED = 0x01,
		IDE_ERROR_DIAGNOSTIC_DEVICE1_FAILED = 0x80
	};

	enum
	{
		IDE_COMMAND_NOP = 0x00,
		IDE_COMMAND_DEVICE_RESET = 0x08,
		IDE_COMMAND_RECALIBRATE = 0x10,
		IDE_COMMAND_READ_SECTORS = 0x20,
		IDE_COMMAND_READ_SECTORS_NORETRY = 0x21,
		IDE_COMMAND_WRITE_SECTORS = 0x30,
		IDE_COMMAND_WRITE_SECTORS_NORETRY = 0x31,
		IDE_COMMAND_VERIFY_SECTORS = 0x40,
		IDE_COMMAND_VERIFY_SECTORS_NORETRY = 0x41,
		IDE_COMMAND_SEEK = 0x70,
		IDE_COMMAND_DIAGNOSTIC = 0x90,
		IDE_COMMAND_SET_CONFIG = 0x91,
		IDE_COMMAND_PACKET = 0xa0,
		IDE_COMMAND_IDENTIFY_PACKET_DEVICE = 0xa1,
		IDE_COMMAND_READ_MULTIPLE = 0xc4,
		IDE_COMMAND_WRITE_MULTIPLE = 0xc5,
		IDE_COMMAND_SET_BLOCK_COUNT = 0xc6,
		IDE_COMMAND_READ_DMA = 0xc8,
		IDE_COMMAND_WRITE_DMA = 0xca,
		IDE_COMMAND_STANDBY_IMMEDIATE = 0xe0,
		IDE_COMMAND_IDLE_IMMEDIATE = 0xe1,
		IDE_COMMAND_STANDBY = 0xe2,
		IDE_COMMAND_IDLE = 0xe3,
		IDE_COMMAND_READ_BUFFER = 0xe4,
		IDE_COMMAND_CHECK_POWER_MODE = 0xe5,
		IDE_COMMAND_CACHE_FLUSH = 0xe7,
		IDE_COMMAND_WRITE_BUFFER = 0xe8,
		IDE_COMMAND_IDENTIFY_DEVICE = 0xec,
		IDE_COMMAND_SET_FEATURES = 0xef,
		IDE_COMMAND_SECURITY_UNLOCK = 0xf2,
		IDE_COMMAND_SECURITY_DISABLE_PASSWORD = 0xf6,
		IDE_COMMAND_READ_NATIVE_MAX_ADDRESS = 0xf8,
		IDE_COMMAND_SET_MAX = 0xf9
	};

	enum
	{
		IDE_SET_FEATURES_ENABLE_8BIT_DATA_TRANSFERS = 0x01,
		IDE_SET_FEATURES_TRANSFER_MODE = 0x03,
		IDE_SET_FEATURES_DISABLE_REVERTING_TO_POWER_ON_DEFAULTS = 0x66,
		IDE_SET_FEATURES_DISABLE_8BIT_DATA_TRANSFERS = 0x81,
		IDE_SET_FEATURES_ENABLE_ECC = 0x88,
		IDE_SET_FEATURES_ENABLE_RETRIES = 0x99,
		IDE_SET_FEATURES_ENABLE_READ_LOOK_AHEAD = 0xaa,
		IDE_SET_FEATURES_ENABLE_REVERTING_TO_POWER_ON_DEFAULTS = 0xcc
	};

	enum ide_transfer_type_t
	{
		IDE_TRANSFER_TYPE_PIO_DEFAULT = 0x00,
		IDE_TRANSFER_TYPE_PIO_FLOW_CONTROL = 0x08,
		IDE_TRANSFER_TYPE_SINGLE_WORD_DMA = 0x10,
		IDE_TRANSFER_TYPE_MULTI_WORD_DMA = 0x20,
		IDE_TRANSFER_TYPE_ULTRA_DMA = 0x40,
		IDE_TRANSFER_TYPE_MASK = 0xf8
	};

	enum
	{
		IDE_DEVICE_HEAD_HS = 0x0f,
		IDE_DEVICE_HEAD_DRV = 0x10,
		IDE_DEVICE_HEAD_L = 0x40,
		IDE_DEVICE_HEAD_OBSOLETE = 0x80 | 0x20
	};

	enum
	{
		TID_BUSY,
		TID_BUFFER_EMPTY
	};

	enum
	{
		PARAM_RESET,
		PARAM_DETECT_DEVICE1,
		PARAM_DIAGNOSTIC,
		PARAM_WAIT_FOR_PDIAG,
		PARAM_COMMAND
	};

	attotime MINIMUM_COMMAND_TIME;

	std::vector<uint8_t> m_buffer;
	uint16_t m_buffer_offset;
	uint16_t m_buffer_size;
	uint8_t m_error;
	uint8_t m_feature;
	uint16_t m_sector_count;
	uint8_t m_sector_number;
	uint8_t m_cylinder_low;
	uint8_t m_cylinder_high;
	uint8_t m_device_head;
	uint8_t m_status;
	uint8_t m_command;
	uint8_t m_device_control;

	uint16_t m_identify_buffer[256];
	bool m_revert_to_defaults;
	bool m_8bit_data_transfers;

private:
	virtual void set_irq_out(int state) = 0;
	virtual void set_dmarq_out(int state) = 0;
	virtual void set_dasp_out(int state) = 0;
	virtual void set_pdiag_out(int state) = 0;

	void update_irq();
	void write_buffer_full();
	void start_diagnostic();
	void finished_diagnostic();
	void finished_busy(int32_t param);
	void clear_dma_modes();
	bool set_dma_mode(int word);

	int m_csel;
	int m_daspin;
	int m_daspout;
	int m_dmack;
	int m_dmarq;
	int m_irq;
	int m_pdiagin;
	int m_pdiagout;

	bool m_single_device;
	bool m_resetting;

	emu_timer *m_busy_timer;
	emu_timer *m_buffer_empty_timer;
};

#endif // MAME_MACHINE_ATAHLE_H
