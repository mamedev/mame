// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    DEC RX01 floppy drive controller

**********************************************************************/

#pragma once

#ifndef __RX01__
#define __RX01__

#include "imagedev/flopdrv.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RX01_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, RX01, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rx01_device

class rx01_device :  public device_t
{
public:
	// construction/destruction
	rx01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void command_write(uint16_t data);
	uint16_t status_read();

	void data_write(uint16_t data);
	uint16_t data_read();

	void service_command(void *ptr, int32_t param);

	void position_head();
	void read_sector();
	void write_sector(int ddam);
private:
	enum rx01_state {
		RX01_FILL,
		RX01_EMPTY,
		RX01_SET_SECTOR,
		RX01_SET_TRACK,
		RX01_TRANSFER,
		RX01_COMPLETE,
		RX01_INIT
	};

	legacy_floppy_image_device *m_image[2];
	uint8_t m_buffer[128];
	int m_buf_pos;

	uint16_t m_rxcs; // Command and Status Register
	uint16_t m_rxdb; // Data Buffer Register
	uint16_t m_rxta; // RX Track Address
	uint16_t m_rxsa; // RX Sector Address
	uint16_t m_rxes; // RX Error and Status
	int m_unit;
	int m_interrupt;
	rx01_state m_state;
};

// device type definition
extern const device_type RX01;

#endif
