// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    DEC RX01 floppy drive controller

**********************************************************************/

#ifndef MAME_MACHINE_RX01_H
#define MAME_MACHINE_RX01_H

#pragma once

#include "imagedev/flopdrv.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rx01_device

class rx01_device :  public device_t
{
public:
	// construction/destruction
	rx01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	void command_write(uint16_t data);
	uint16_t status_read();

	void data_write(uint16_t data);
	uint16_t data_read();

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

	required_device_array<legacy_floppy_image_device, 2> m_image;
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
DECLARE_DEVICE_TYPE(RX01, rx01_device)

#endif // MAME_MACHINE_RX01_H
