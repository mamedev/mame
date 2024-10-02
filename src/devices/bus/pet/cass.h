// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore PET/VIC-20/C64/Plus-4 Datassette Port emulation

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       +5V
                  MOTOR       3      C       MOTOR
                   READ       4      D       READ
                  WRITE       5      E       WRITE
                  SENSE       6      F       SENSE

**********************************************************************/

#ifndef MAME_BUS_PET_CASS_H
#define MAME_BUS_PET_CASS_H

#pragma once




//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define PET_DATASSETTE_PORT_TAG     "tape"
#define PET_DATASSETTE_PORT2_TAG     "tape2"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pet_datassette_port_device

class device_pet_datassette_port_interface;

class pet_datassette_port_device : public device_t, public device_single_card_slot_interface<device_pet_datassette_port_interface>
{
public:
	template <typename T>
	pet_datassette_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: pet_datassette_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	pet_datassette_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~pet_datassette_port_device();

	// static configuration helpers
	auto read_handler() { return m_read_handler.bind(); }

	// computer interface
	int read();
	void write(int state);
	int sense_r();
	void motor_w(int state);

	// device interface
	void read_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	devcb_write_line m_read_handler;

	device_pet_datassette_port_interface *m_cart;
};


// ======================> device_pet_datassette_port_interface

// class representing interface-specific live c64_expansion card
class device_pet_datassette_port_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_pet_datassette_port_interface();

	virtual int datassette_read() { return 1; }
	virtual void datassette_write(int state) { }
	virtual int datassette_sense() { return 1; }
	virtual void datassette_motor(int state) { }

protected:
	device_pet_datassette_port_interface(const machine_config &mconfig, device_t &device);

	pet_datassette_port_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(PET_DATASSETTE_PORT, pet_datassette_port_device)


void cbm_datassette_devices(device_slot_interface &device);

#endif // MAME_BUS_PET_CASS_H
