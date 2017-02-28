// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom

#include "emu.h"
#include "vme_mzr8105.h"

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04

#define VERBOSE 0 // (LOG_PRINTF | LOG_SETUP  | LOG_GENERAL)

#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)

#define LOG(...)      LOGMASK(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASK(LOG_SETUP,   __VA_ARGS__)

#if VERBOSE & LOG_PRINTF
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type VME_MZR8105 = device_creator<vme_mzr8105_card_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

MACHINE_CONFIG_EXTERN( mzr8105 );

machine_config_constructor vme_mzr8105_card_device::device_mconfig_additions() const
{
	LOG("%s %s\n", tag(), FUNCNAME);
	return MACHINE_CONFIG_NAME( mzr8105 );
}

vme_mzr8105_card_device::vme_mzr8105_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VME_MZR8105, "Mizar 8105 68K CPU board", tag, owner, clock, "mzr8105", __FILE__),
	device_vme_card_interface(mconfig, *this)
{
	m_slot = 1;
	LOG("%s %s\n", tag, FUNCNAME);
}

vme_mzr8105_card_device::vme_mzr8105_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
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
