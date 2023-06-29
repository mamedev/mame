// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom

#include "emu.h"
#include "vme_mzr8105.h"

#define VERBOSE (0) // (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VME_MZR8105, vme_mzr8105_card_device, "mzr8105", "Mizar 8105 68K CPU board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vme_mzr8105_card_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(10'000'000))
	m_maincpu->set_addrmap(AS_PROGRAM, &vme_mzr8105_card_device::mzr8105_mem);

	VME(config, "vme", 0).use_owner_spaces();
	VME_SLOT(config, "slot1", mzr8105_vme_cards, "mzr8300", 1, "vme");
}

vme_mzr8105_card_device::vme_mzr8105_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vme_mzr8105_card_device(mconfig, VME_MZR8105, tag, owner, clock)
{
	m_slot = 1;
	LOG("%s %s\n", tag, FUNCNAME);
}

vme_mzr8105_card_device::vme_mzr8105_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_vme_card_interface(mconfig, *this)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

void vme_mzr8105_card_device::device_start()
{
	LOG("%s %s\n", tag(), FUNCNAME);
}

void vme_mzr8105_card_device::device_reset()
{
	LOG("%s %s\n", tag(), FUNCNAME);
}
