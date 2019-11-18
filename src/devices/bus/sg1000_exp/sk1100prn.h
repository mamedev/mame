// license:BSD-3-Clause
// copyright-holders:Enik Land
/**********************************************************************

    Sega SK-1100 keyboard printer port emulation

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_SG1000_EXP_SK1100_PRN_H
#define MAME_BUS_SG1000_EXP_SK1100_PRN_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sk1100_printer_port_device

class device_sk1100_printer_port_interface;

class sk1100_printer_port_device : public device_t, public device_single_card_slot_interface<device_sk1100_printer_port_interface>
{
public:
	// construction/destruction
	template <typename T>
	sk1100_printer_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: sk1100_printer_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	sk1100_printer_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~sk1100_printer_port_device();

	DECLARE_READ_LINE_MEMBER(fault_r);
	DECLARE_READ_LINE_MEMBER(busy_r);

	DECLARE_WRITE_LINE_MEMBER(data_w);
	DECLARE_WRITE_LINE_MEMBER(reset_w);
	DECLARE_WRITE_LINE_MEMBER(feed_w);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	device_sk1100_printer_port_interface *m_device;
};


// ======================> device_sk1100_printer_port_interface

// class representing interface-specific live sk1100_printer_port peripheral
class device_sk1100_printer_port_interface : public device_interface
{
	friend class sk1100_printer_port_device;
public:
	// construction/destruction
	virtual ~device_sk1100_printer_port_interface();

protected:
	device_sk1100_printer_port_interface(const machine_config &mconfig, device_t &device);

	virtual DECLARE_WRITE_LINE_MEMBER( input_data )  { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_reset ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_feed ) { }

	virtual DECLARE_READ_LINE_MEMBER( output_fault ) { return 1; }
	virtual DECLARE_READ_LINE_MEMBER( output_busy ) { return 1; }
};


// device type definition
DECLARE_DEVICE_TYPE(SK1100_PRINTER_PORT, sk1100_printer_port_device)


void sk1100_printer_port_devices(device_slot_interface &device);


#endif // MAME_BUS_SG1000_EXP_SK1100_PRN_H
