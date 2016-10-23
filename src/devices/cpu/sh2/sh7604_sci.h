// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

  SH7604 SCI Controller

***************************************************************************/

#pragma once

#ifndef __SH7604_SCIDEV_H__
#define __SH7604_SCIDEV_H__

enum {
	STATUS_MPBT = 1 << 0,
	STATUS_MPB =  1 << 1,
	STATUS_TEND = 1 << 2,
	STATUS_PER =  1 << 3,
	STATUS_FER =  1 << 4,
	STATUS_ORER = 1 << 5,
	STATUS_RDRF = 1 << 6,
	STATUS_TDRE = 1 << 7
};



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SH7604_SCI_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, SH7604_SCI, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sh7604_sci_device

class sh7604_sci_device : public device_t,
						  public device_memory_interface
{
public:
	// construction/destruction
	sh7604_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	uint8_t serial_mode_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void serial_mode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bitrate_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bitrate_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t serial_control_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void serial_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t transmit_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void transmit_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t serial_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void serial_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t receive_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;
protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	const address_space_config      m_space_config;
	uint8_t m_smr;
	uint8_t m_scr;
	uint8_t m_ssr;
	uint8_t m_brr;
};


// device type definition
extern const device_type SH7604_SCI;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
