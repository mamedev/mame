// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    X68000 Custom SASI HD controller

*/
#ifndef MAME_MACHINE_X68K_HDC_H
#define MAME_MACHINE_X68K_HDC_H

#pragma once


class x68k_hdc_image_device :   public device_t,
								public device_image_interface
{
	enum class sasi_phase : u8
	{
		BUSFREE = 0,
		ARBITRATION,
		SELECTION,
		RESELECTION,
		COMMAND,
		DATA,
		STATUS,
		MESSAGE,
		READ,
		WRITE
	};

	// SASI commands, based on the SASI standard
	enum sasi_cmd : u8
	{
		// Class 0 (6-byte) commands
		SASI_CMD_TEST_UNIT_READY = 0,
		SASI_CMD_REZERO_UNIT,
		SASI_CMD_RESERVED_02,
		SASI_CMD_REQUEST_SENSE,
		SASI_CMD_FORMAT_UNIT,
		SASI_CMD_RESERVED_05,
		SASI_CMD_FORMAT_UNIT_06,  // the X68000 uses command 0x06 for Format Unit, despite the SASI specs saying 0x04
		SASI_CMD_RESERVED_07,
		SASI_CMD_READ,
		SASI_CMD_RESERVED_09,
		SASI_CMD_WRITE,
		SASI_CMD_SEEK,
		SASI_CMD_RESERVED_0C,
		SASI_CMD_RESERVED_0D,
		SASI_CMD_RESERVED_0E,
		SASI_CMD_WRITE_FILE_MARK,
		SASI_CMD_INVALID_10,
		SASI_CMD_INVALID_11,
		SASI_CMD_RESERVE_UNIT,
		SASI_CMD_RELEASE_UNIT,
		SASI_CMD_INVALID_14,
		SASI_CMD_INVALID_15,
		SASI_CMD_READ_CAPACITY,
		SASI_CMD_INVALID_17,
		SASI_CMD_INVALID_18,
		SASI_CMD_INVALID_19,
		SASI_CMD_READ_DIAGNOSTIC,
		SASI_CMD_WRITE_DIAGNOSTIC,
		SASI_CMD_INVALID_1C,
		SASI_CMD_INVALID_1D,
		SASI_CMD_INVALID_1E,
		SASI_CMD_INQUIRY,
		// Class 1 commands  (yes, just the one)
		SASI_CMD_RESERVED_20,
		SASI_CMD_RESERVED_21,
		SASI_CMD_RESERVED_22,
		SASI_CMD_SET_BLOCK_LIMITS = 0x28,
		// Class 2 commands
		SASI_CMD_EXTENDED_ADDRESS_READ = 0x48,
		SASI_CMD_INVALID_49,
		SASI_CMD_EXTENDED_ADDRESS_WRITE,
		SASI_CMD_WRITE_AND_VERIFY = 0x54,
		SASI_CMD_VERIFY,
		SASI_CMD_INVALID_56,
		SASI_CMD_SEARCH_DATA_HIGH,
		SASI_CMD_SEARCH_DATA_EQUAL,
		SASI_CMD_SEARCH_DATA_LOW,
		// controller-specific commands
		SASI_CMD_SPECIFY = 0xc2
	};

	enum sasi_status : u8
	{
		SASI_STATUS_MSG = 1 << 4,   // MSG
		SASI_STATUS_CD = 1 << 3,    // C/D (Command/Data)
		SASI_STATUS_IO = 1 << 2,    // I/O
		SASI_STATUS_BSY = 1 << 1,   // BSY
		SASI_STATUS_REQ = 1 << 0    // REQ
	};

public:
	// construction/destruction
	x68k_hdc_image_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// image-level overrides
	virtual iodevice_t image_type() const override { return IO_HARDDISK; }

	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return true; }
	virtual bool is_creatable() const override { return true; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return false; }
	virtual const char *file_extensions() const override { return "hdf"; }
	virtual const char *custom_instance_name() const override { return "sasihd"; }
	virtual const char *custom_brief_instance_name() const override { return "sasi"; }
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;

	DECLARE_WRITE16_MEMBER( hdc_w );
	DECLARE_READ16_MEMBER( hdc_r );
protected:
	// device-level overrides
	virtual void device_start() override;
private:
	TIMER_CALLBACK_MEMBER(req_timer_callback);

	sasi_phase m_phase;
	u8 m_status_port;  // read at 0xe96003
	u8 m_status;       // status phase output
	u8 m_command[10];
	u8 m_sense[4];
	u16 m_command_byte_count;
	u16 m_command_byte_total;
	u8 m_current_command;
	u16 m_transfer_byte_count;
	u16 m_transfer_byte_total;
	emu_timer *m_req_timer;
};

// device type definition
DECLARE_DEVICE_TYPE(X68KHDC, x68k_hdc_image_device)

#endif // MAME_MACHINE_X68K_HDC_H
