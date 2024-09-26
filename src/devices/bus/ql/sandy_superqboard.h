// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    Sandy SuperQBoard/SuperQMouse (with HD upgrade) emulation

**********************************************************************/

#ifndef MAME_BUS_QL_SANDY_SUPERQBOARD_H
#define MAME_BUS_QL_SANDY_SUPERQBOARD_H

#pragma once

#include "exp.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sandy_superqboard_device

class sandy_superqboard_device : public device_t, public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	sandy_superqboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER( mouse_x_changed );
	DECLARE_INPUT_CHANGED_MEMBER( mouse_y_changed );

protected:
	sandy_superqboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int ram_size);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_ql_expansion_card_interface overrides
	virtual uint8_t read(offs_t offset, uint8_t data) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	void busy_w(int state);

	static void floppy_formats(format_registration &fr);

	enum
	{
		ST_BUSY = 0x01,
		ST_MIDDLE = 0x02,
		ST_RIGHT = 0x04,
		ST_LEFT = 0x08,
		ST_Y_DIR = 0x10,
		ST_X_DIR = 0x20,
		ST_Y_INT = 0x40,
		ST_X_INT = 0x80
	};

	void check_interrupt();

	required_device<wd1772_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_latch;
	required_memory_region m_rom;
	memory_share_creator<uint8_t> m_ram;
	optional_ioport m_buttons;

	int m_ram_size;
	int m_fd6;
	int m_fd7;

	uint8_t m_status;
};


// ======================> sandy_superqboard_512k_device

class sandy_superqboard_512k_device :  public sandy_superqboard_device
{
public:
	// construction/destruction
	sandy_superqboard_512k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> sandy_superqmouse_device

class sandy_superqmouse_device :  public sandy_superqboard_device
{
public:
	// construction/destruction
	sandy_superqmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> sandy_superqmouse_512k_device

class sandy_superqmouse_512k_device :  public sandy_superqboard_device
{
public:
	// construction/destruction
	sandy_superqmouse_512k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(SANDY_SUPERQBOARD,      sandy_superqboard_device)
DECLARE_DEVICE_TYPE(SANDY_SUPERQBOARD_512K, sandy_superqboard_512k_device)
DECLARE_DEVICE_TYPE(SANDY_SUPERQMOUSE,      sandy_superqmouse_device)
DECLARE_DEVICE_TYPE(SANDY_SUPERQMOUSE_512K, sandy_superqmouse_512k_device)

#endif // MAME_BUS_QL_SANDY_SUPERQBOARD_H
