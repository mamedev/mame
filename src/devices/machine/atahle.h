// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    atahle.h

    ATA Device HLE

***************************************************************************/

#pragma once

#ifndef __ATAHLE_H__
#define __ATAHLE_H__

#include "atadev.h"

class ata_hle_device : public device_t,
	public ata_device_interface,
	public device_slot_card_interface
{
public:
	ata_hle_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock,const char *shortname, const char *source);

	virtual UINT16 read_dma() override;
	virtual DECLARE_READ16_MEMBER(read_cs0) override;
	virtual DECLARE_READ16_MEMBER(read_cs1) override;

	virtual void write_dma(UINT16 data) override;
	virtual DECLARE_WRITE16_MEMBER(write_cs0) override;
	virtual DECLARE_WRITE16_MEMBER(write_cs1) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_csel) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_dasp) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_dmack) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_pdiag) override;

	TIMER_CALLBACK_MEMBER(buffer_empty_timer_work);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void set_irq(int state);
	void set_dmarq(int state);
	void set_dasp(int state);
	void set_pdiag(int state);

	void start_busy(const attotime &time, int param);
	void stop_busy();

	int dev() { return (m_device_head & IDE_DEVICE_HEAD_DRV) >> 4; }
	bool device_selected() { return m_csel == dev(); }

	virtual UINT8 calculate_status() { return m_status; }
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
	virtual UINT16 read_data();
	virtual void write_data(UINT16 data);

	int bit_to_mode(UINT16 word);
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
		IDE_COMMAND_IDLE_IMMEDIATE = 0xe1,
		IDE_COMMAND_IDLE = 0xe3,
		IDE_COMMAND_CHECK_POWER_MODE = 0xe5,
		IDE_COMMAND_CACHE_FLUSH = 0xe7,
		IDE_COMMAND_IDENTIFY_DEVICE = 0xec,
		IDE_COMMAND_SET_FEATURES = 0xef,
		IDE_COMMAND_SECURITY_UNLOCK = 0xf2,
		IDE_COMMAND_READ_NATIVE_MAX_ADDRESS = 0xf8,
		IDE_COMMAND_SET_MAX = 0xf9
	};

	enum
	{
		IDE_SET_FEATURES_ENABLE_8BIT_DATA_TRANSFERS = 0x01,
		IDE_SET_FEATURES_TRANSFER_MODE = 0x03,
		IDE_SET_FEATURES_DISABLE_REVERTING_TO_POWER_ON_DEFAULTS = 0x66,
		IDE_SET_FEATURES_DISABLE_8BIT_DATA_TRANSFERS = 0x81,
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

	dynamic_buffer m_buffer;
	UINT16 m_buffer_offset;
	UINT16 m_buffer_size;
	UINT8 m_error;
	UINT8 m_feature;
	UINT16 m_sector_count;
	UINT8 m_sector_number;
	UINT8 m_cylinder_low;
	UINT8 m_cylinder_high;
	UINT8 m_device_head;
	UINT8 m_status;
	UINT8 m_command;
	UINT8 m_device_control;

	UINT16 m_identify_buffer[256];
	bool m_revert_to_defaults;
	bool m_8bit_data_transfers;

private:
	void update_irq();
	void write_buffer_full();
	void start_diagnostic();
	void finished_diagnostic();
	void finished_busy(int param);
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

#endif
