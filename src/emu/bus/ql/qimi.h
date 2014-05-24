// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    QIMI (QL Internal Mouse Interface) emulation
 
    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __QIMI__
#define __QIMI__

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> qimi_device

class qimi_t : public device_t,
			   public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	qimi_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_ql_expansion_card_interface overrides

private:
	required_ioport m_mousex;
	required_ioport m_mousey;
	required_ioport m_mouseb;
};


// device type definition
extern const device_type QIMI;


#endif
// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    QIMI (QL Internal Mouse Interface) emulation
 
    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __QIMI__
#define __QIMI__

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> qimi_device

class qimi_t : public device_t,
			   public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	qimi_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_ql_expansion_card_interface overrides

private:
	required_ioport m_mousex;
	required_ioport m_mousey;
	required_ioport m_mouseb;
};


// device type definition
extern const device_type QIMI;


#endif
