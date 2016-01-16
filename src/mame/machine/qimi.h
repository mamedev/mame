// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    QJump/Quanta QL Internal Mouse Interface emulation

**********************************************************************/

#pragma once

#ifndef __QIMI__
#define __QIMI__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_QIMI_EXTINT_CALLBACK(_write) \
	devcb = &qimi_t::set_exting_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> qimi_t

class qimi_t :  public device_t
{
public:
	// construction/destruction
	qimi_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_exting_wr_callback(device_t &device, _Object object) { return downcast<qimi_t &>(device).m_write_extint.set_callback(object); }

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	UINT8 read(address_space &space, offs_t offset, UINT8 data);
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_INPUT_CHANGED_MEMBER( mouse_x_changed );
	DECLARE_INPUT_CHANGED_MEMBER( mouse_y_changed );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum
	{
		ST_Y_DIR = 0x01,
		ST_X_INT = 0x04,
		ST_X_DIR = 0x10,
		ST_Y_INT = 0x20
	};

	devcb_write_line m_write_extint;

	required_ioport m_buttons;

	UINT8 m_status;
	bool m_extint_en;
};


// device type definition
extern const device_type QIMI;



#endif
