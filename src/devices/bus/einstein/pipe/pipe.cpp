// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Einstein "Tatung Pipe"

***************************************************************************/

#include "emu.h"
#include "pipe.h"

// supported devices
#include "silicon_disc.h"
#include "speculator.h"
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
	m_program(*this, finder_base::DUMMY_TAG, -1),
	m_io(*this, finder_base::DUMMY_TAG, -1),
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
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tatung_pipe_device::device_config_complete()
{
	// for passthrough connectors, use the parent slot's spaces
	if (dynamic_cast<device_tatung_pipe_interface *>(owner()) != nullptr)
	{
		auto parent = dynamic_cast<tatung_pipe_device *>(owner()->owner());
		if (parent != nullptr)
		{
			if (m_program.finder_tag() == finder_base::DUMMY_TAG)
				m_program.set_tag(parent->m_program, parent->m_program.spacenum());
			if (m_io.finder_tag() == finder_base::DUMMY_TAG)
				m_io.set_tag(parent->m_io, parent->m_io.spacenum());
		}
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tatung_pipe_device::device_start()
{
	// get inserted module
	m_card = dynamic_cast<device_tatung_pipe_interface *>(get_card_device());

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
//  host to module interface
//-------------------------------------------------

WRITE_LINE_MEMBER( tatung_pipe_device::host_int_w )
{
	if (m_card)
		m_card->int_w(state);
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

void tatung_pipe_cards(device_slot_interface &device)
{
	device.option_add("silicon_disc", EINSTEIN_SILICON_DISC);
	device.option_add("speculator", EINSTEIN_SPECULATOR);
	device.option_add("tk02", TK02_80COL);
}
