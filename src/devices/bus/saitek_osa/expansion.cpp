// license:BSD-3-Clause
// copyright-holders:Dirk Best, hap
/***************************************************************************

Saitek OSA Expansion Slot

Used by Saitek(SciSys) chess computers Leonardo, Galileo, Renaissance.

***************************************************************************/

#include "emu.h"
#include "expansion.h"

#include "maestro.h"
#include "maestroa.h"
#include "sparc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAITEKOSA_EXPANSION, saitekosa_expansion_device, "saitekosa_expansion", "Saitek OSA Expansion Bus")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  saitekosa_expansion_device - constructor
//-------------------------------------------------

saitekosa_expansion_device::saitekosa_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SAITEKOSA_EXPANSION, tag, owner, clock),
	device_single_card_slot_interface<device_saitekosa_expansion_interface>(mconfig, *this),
	m_stb_handler(*this),
	m_rts_handler(*this),
	m_module(nullptr)
{ }

//-------------------------------------------------
//  saitekosa_expansion_device - destructor
//-------------------------------------------------

saitekosa_expansion_device::~saitekosa_expansion_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saitekosa_expansion_device::device_start()
{
	// get inserted module
	m_module = get_card_device();

	// resolve callbacks
	m_stb_handler.resolve_safe();
	m_rts_handler.resolve_safe();

	// register for savestates
	save_item(NAME(m_data));
	save_item(NAME(m_nmi));
	save_item(NAME(m_ack));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void saitekosa_expansion_device::device_add_mconfig(machine_config &config)
{
	// optional embedded screen
	auto &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(99, 16);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(saitekosa_expansion_device::screen_update));
}

//-------------------------------------------------
//  host to module interface
//-------------------------------------------------

u8 saitekosa_expansion_device::data_r()
{
	if (m_module)
		return m_module->data_r();

	return 0xff;
}

void saitekosa_expansion_device::data_w(u8 data)
{
	if (m_module)
		m_module->data_w(data);

	m_data = data;
}

void saitekosa_expansion_device::nmi_w(int state)
{
	state = (state) ? 1 : 0;

	if (m_module)
		m_module->nmi_w(state);

	m_nmi = state;
}

void saitekosa_expansion_device::ack_w(int state)
{
	state = (state) ? 1 : 0;

	if (m_module)
		m_module->ack_w(state);

	m_ack = state;
}

void saitekosa_expansion_device::pw_w(int state)
{
	state = (state) ? 1 : 0;

	if (m_module)
		m_module->pw_w(state);

	m_pw = state;
}

u32 saitekosa_expansion_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return (m_module) ? m_module->screen_update(screen, bitmap, cliprect) : 0;
}


//**************************************************************************
//  MODULE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_saitekosa_expansion_interface - constructor
//-------------------------------------------------

device_saitekosa_expansion_interface::device_saitekosa_expansion_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "saitekosaexp")
{
	m_expansion = dynamic_cast<saitekosa_expansion_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_saitekosa_expansion_interface - destructor
//-------------------------------------------------

device_saitekosa_expansion_interface::~device_saitekosa_expansion_interface()
{
}

//-------------------------------------------------
//  module list
//-------------------------------------------------

void saitekosa_expansion_modules(device_slot_interface &device)
{
	device.option_add("analyst", OSA_ANALYST);
	device.option_add("maestro", OSA_MAESTRO);
	device.option_add("maestroa", OSA_MAESTROA);
	device.option_add("sparc", OSA_SPARC);
}
