// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    X68000 Custom SASI HD controller

*/

#include "emu.h"

enum
{
	SASI_PHASE_BUSFREE = 0,
	SASI_PHASE_ARBITRATION,
	SASI_PHASE_SELECTION,
	SASI_PHASE_RESELECTION,
	SASI_PHASE_COMMAND,
	SASI_PHASE_DATA,
	SASI_PHASE_STATUS,
	SASI_PHASE_MESSAGE,
	SASI_PHASE_READ,
	SASI_PHASE_WRITE
};

// SASI commands, based on the SASI standard
enum
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

class x68k_hdc_image_device :   public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	x68k_hdc_image_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// image-level overrides
	virtual iodevice_t image_type() const override { return IO_HARDDISK; }

	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 1; }
	virtual bool is_creatable() const override { return 1; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 0; }
	virtual const char *image_interface() const override { return nullptr; }
	virtual const char *file_extensions() const override { return "hdf"; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }
	virtual bool call_create(int format_type, option_resolution *format_options) override;

	DECLARE_WRITE16_MEMBER( hdc_w );
	DECLARE_READ16_MEMBER( hdc_r );
protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
private:
	int m_phase;
	unsigned char m_status_port;  // read at 0xe96003
	unsigned char m_status;       // status phase output
	//unsigned char m_message;
	unsigned char m_command[10];
	unsigned char m_sense[4];
	int m_command_byte_count;
	int m_command_byte_total;
	int m_current_command;
	int m_transfer_byte_count;
	int m_transfer_byte_total;
	int m_msg;  // MSG
	int m_cd;   // C/D (Command/Data)
	int m_bsy;  // BSY
	int m_io;   // I/O
	int m_req;  // REQ
};

// device type definition
extern const device_type X68KHDC;

#define MCFG_X68KHDC_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, X68KHDC, 0)
