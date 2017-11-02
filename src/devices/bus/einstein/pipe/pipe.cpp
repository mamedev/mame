// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Einstein "Tatung Pipe"

***************************************************************************/

#include "emu.h"
#include "pipe.h"

// supported devices
#include "tk02.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TATUNG_PIPE, tatung_pipe_device, "tatung_pipe", "Tatung Pipe Slot")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  tatung_pipe_device - constructor
//-------------------------------------------------

tatung_pipe_device::tatung_pipe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TATUNG_PIPE, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_program(nullptr),
	m_io(nullptr),
	m_card(nullptr),
	m_int_handler(*this),
	m_nmi_handler(*this),
	m_reset_handler(*this)
{
}

//-------------------------------------------------
//  tatung_pipe_device - destructor
//-------------------------------------------------

tatung_pipe_device::~tatung_pipe_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tatung_pipe_device::device_start()
{
	// resolve callbacks
	m_int_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
	m_reset_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tatung_pipe_device::device_reset()
{
}

//-------------------------------------------------
//  set_program_space - set address space we are attached to
//-------------------------------------------------

void tatung_pipe_device::set_program_space(address_space *program)
{
	m_program = program;
}

//-------------------------------------------------
//  set_io_space - set address space we are attached to
//-------------------------------------------------

void tatung_pipe_device::set_io_space(address_space *io)
{
	m_io = io;
}


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_tatung_pipe_interface - constructor
//-------------------------------------------------

device_tatung_pipe_interface::device_tatung_pipe_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<tatung_pipe_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_tatung_pipe_interface - destructor
//-------------------------------------------------

device_tatung_pipe_interface::~device_tatung_pipe_interface()
{
}


//**************************************************************************
//  SLOT INTERFACE
//**************************************************************************

SLOT_INTERFACE_START( tatung_pipe_cards )
	SLOT_INTERFACE("tk02", TK02_80COL)
SLOT_INTERFACE_END
