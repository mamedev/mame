// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    ch376.h

    "File manage and control chip CH376"
    This is a module intended to offload USB and USB mass storage
    I/O from a small microcontroller or microprocessor.

***************************************************************************/

#ifndef MAME_MACHINE_CH376_H
#define MAME_MACHINE_CH376_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> ch376_device

class ch376_device :   public device_t
{
public:
	// construction/destruction
	ch376_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	enum
	{
		STATE_IDLE = 0,
		STATE_USB_MODE_SET,
		STATE_GET_STATUS,
		STATE_SET_FILE_NAME,
		STATE_SEEK,
		STATE_SEEK1,
		STATE_SEEK2,
		STATE_SEEK3,
		STATE_READ_SIZE,
		STATE_READ_SIZE2,
		STATE_WRITE_SIZE_REPLY,
		STATE_READ_WRITE_DATA,
		STATE_GET_FILE_SIZE,
		STATE_CHECK_EXIST
	};

	bool generateNextDirEntry();
	void doNextFileRead();

	int m_state;
	u8 m_status, m_int_status, m_data, m_last_command;
	char m_file_name[16];
	int m_pointer;
	std::string m_current_directory;
	osd::directory::ptr m_directory;
	osd_file::ptr m_file;
	u32 m_file_pos;

	u8 m_dataBuffer[512];
	int m_dataPtr, m_dataLen;
	u16 m_readLen;
	u32 m_cur_file_size;
};


// device type definition
DECLARE_DEVICE_TYPE(CH376, ch376_device)

#endif // MAME_MACHINE_CH376_H
