// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    multibus.h

    Intel Multibus

*********************************************************************/

#ifndef MAME_BUS_MULTIBUS_MULTIBUS_H
#define MAME_BUS_MULTIBUS_MULTIBUS_H

#pragma once

class device_multibus_interface;

class multibus_slot_device : public device_t,
							 public device_single_card_slot_interface<device_multibus_interface>
{
public:
	multibus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~multibus_slot_device();

	static constexpr unsigned BUS_CLOCK = 9830400;

	// Install r/w functions in I/O space
	void install_io_rw(address_space& space);

	// Set memory space
	void install_mem_rw(address_space& space);

protected:
	virtual void device_start() override;
};

class device_multibus_interface : public device_interface
{
public:
	// Install r/w functions in I/O space
	virtual void install_io_rw(address_space& space) = 0;

	// Set CPU memory space
	virtual void install_mem_rw(address_space& space) = 0;

protected:
	device_multibus_interface(const machine_config &mconfig , device_t &device);
	virtual ~device_multibus_interface();
};

// device type declaration
DECLARE_DEVICE_TYPE(MULTIBUS_SLOT, multibus_slot_device)

#endif /* MAME_BUS_MULTIBUS_MULTIBUS_H */
