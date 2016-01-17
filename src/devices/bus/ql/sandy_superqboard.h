// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    Sandy SuperQBoard/SuperQMouse (with HD upgrade) emulation

**********************************************************************/

#pragma once

#ifndef __SANDY_SUPERQBOARD__
#define __SANDY_SUPERQBOARD__

#include "exp.h"
#include "bus/centronics/ctronics.h"
#include "formats/ql_dsk.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sandy_superqboard_t

class sandy_superqboard_t : public device_t,
							public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	sandy_superqboard_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	sandy_superqboard_t(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source, int ram_size);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	WRITE_LINE_MEMBER( busy_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_INPUT_CHANGED_MEMBER( mouse_x_changed );
	DECLARE_INPUT_CHANGED_MEMBER( mouse_y_changed );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_ql_expansion_card_interface overrides
	virtual UINT8 read(address_space &space, offs_t offset, UINT8 data) override;
	virtual void write(address_space &space, offs_t offset, UINT8 data) override;

private:
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

	required_device<wd1772_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_latch;
	required_memory_region m_rom;
	optional_shared_ptr<UINT8> m_ram;
	optional_ioport m_buttons;

	int m_ram_size;
	int m_fd6;
	int m_fd7;

	UINT8 m_status;
};


// ======================> sandy_superqboard_512k_t

class sandy_superqboard_512k_t :  public sandy_superqboard_t
{
public:
	// construction/destruction
	sandy_superqboard_512k_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


// ======================> sandy_superqmouse_t

class sandy_superqmouse_t :  public sandy_superqboard_t
{
public:
	// construction/destruction
	sandy_superqmouse_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
};


// ======================> sandy_superqmouse_512k_t

class sandy_superqmouse_512k_t :  public sandy_superqboard_t
{
public:
	// construction/destruction
	sandy_superqmouse_512k_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
};


// device type definition
extern const device_type SANDY_SUPERQBOARD;
extern const device_type SANDY_SUPERQBOARD_512K;
extern const device_type SANDY_SUPERQMOUSE;
extern const device_type SANDY_SUPERQMOUSE_512K;



#endif
