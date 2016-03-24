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
	rx01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void command_write(UINT16 data);
	UINT16 status_read();

	void data_write(UINT16 data);
	UINT16 data_read();

	TIMER_CALLBACK_MEMBER(service_command);

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
	UINT8 m_buffer[128];
	int m_buf_pos;

	UINT16 m_rxcs; // Command and Status Register
	UINT16 m_rxdb; // Data Buffer Register
	UINT16 m_rxta; // RX Track Address
	UINT16 m_rxsa; // RX Sector Address
	UINT16 m_rxes; // RX Error and Status
	int m_unit;
	int m_interrupt;
	rx01_state m_state;
};

// device type definition
extern const device_type RX01;

#endif
