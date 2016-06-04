// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia
/* Serial Flash */

#pragma once

#ifndef __SERFLASH_H__
#define __SERFLASH_H__


#define FLASH_PAGE_SIZE (2048+64)






//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SERFLASH_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SERFLASH, 0)
//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class serflash_device;

typedef enum                            { STATE_IDLE = 0,   STATE_READ,     STATE_READ_ID,  STATE_READ_STATUS, STATE_BLOCK_ERASE, STATE_PAGE_PROGRAM    } flash_state_t;
//const char *m_flash_state_name[] =    { "IDLE",           "READ",         "READ_ID",      "READ_STATUS",     "BLOCK ERASE",     "PAGE PROGRAM"        };

// custom initialization for default state
typedef device_delegate<void (serflash_device &, void *, size_t)> serflash_init_delegate;


// ======================> serflash_device

class serflash_device :    public device_t,
						public device_nvram_interface
{
public:

	// construction/destruction
	serflash_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( flash_ready_r );
	DECLARE_READ8_MEMBER( flash_io_r );
	DECLARE_WRITE8_MEMBER( flash_addr_w );
	DECLARE_WRITE8_MEMBER( flash_data_w );
	DECLARE_WRITE8_MEMBER( flash_cmd_w );
	DECLARE_WRITE8_MEMBER( flash_enab_w );
	void flash_hard_reset();

	DECLARE_READ8_MEMBER(n3d_flash_r);
	DECLARE_WRITE8_MEMBER(n3d_flash_cmd_w);
	DECLARE_WRITE8_MEMBER(n3d_flash_addr_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

	void flash_change_state(flash_state_t state);

	// runtime state
	size_t                      m_length;
	UINT8* m_region;


	flash_state_t m_flash_state;

	UINT8 m_flash_enab;

	UINT8 m_flash_cmd_seq;
	UINT32 m_flash_cmd_prev;

	UINT8 m_flash_addr_seq;
	UINT8 m_flash_read_seq;

	UINT16 m_flash_row, m_flash_col;
	int m_flash_page_addr;
	UINT16 m_flash_page_index;


	dynamic_buffer m_flashwritemap;

	UINT8 m_last_flash_cmd;

	UINT32 m_flash_addr;

	UINT8 m_flash_page_data[FLASH_PAGE_SIZE];



};


// device type definition
extern const device_type SERFLASH;


#endif
