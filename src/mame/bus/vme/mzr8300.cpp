// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom

#include "emu.h"
#include "mzr8300.h"
#include "machine/z80sio.h"

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
//	GLOBAL VARIABLES
//**************************************************************************

const device_type VME_MZR8300 = &device_creator<vme_mzr8300_card_device>;

vme_mzr8300_card_device::vme_mzr8300_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VME_MZR8300, "Mizar 8300 SIO serial board", tag, owner, clock, "mzr8300", __FILE__),
	device_vme_p1_card_interface(mconfig, *this)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

vme_mzr8300_card_device::vme_mzr8300_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_vme_p1_card_interface(mconfig, *this)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

void vme_mzr8300_card_device::device_start()
{
	LOG("%s %s\n", tag(), FUNCNAME);
	set_vme_p1_device();

	/* Setup r/w handlers for first SIO in A16 */
	uint32_t base = 0xFF0000;
	m_vme_p1->install_device(base + 0, base + 1, // Channel B - Data
							 read8_delegate(FUNC(z80sio_device::db_r),  subdevice<z80sio_device>("sio0")), write8_delegate(FUNC(z80sio_device::db_w), subdevice<z80sio_device>("sio0")), 0x00ff);
	m_vme_p1->install_device(base + 2, base + 3, // Channel B - Control
							 read8_delegate(FUNC(z80sio_device::cb_r),  subdevice<z80sio_device>("sio0")), write8_delegate(FUNC(z80sio_device::cb_w), subdevice<z80sio_device>("sio0")), 0x00ff);
	m_vme_p1->install_device(base + 4, base + 5, // Channel A - Data
							 read8_delegate(FUNC(z80sio_device::da_r),  subdevice<z80sio_device>("sio0")), write8_delegate(FUNC(z80sio_device::da_w), subdevice<z80sio_device>("sio0")), 0x00ff);
	m_vme_p1->install_device(base + 6, base + 7, // Channel A - Control
							 read8_delegate(FUNC(z80sio_device::ca_r),  subdevice<z80sio_device>("sio0")), write8_delegate(FUNC(z80sio_device::ca_w), subdevice<z80sio_device>("sio0")), 0x00ff);
}

void vme_mzr8300_card_device::device_reset()
{
	LOG("%s %s\n", tag(), FUNCNAME);
}

//-------------------------------------------------
//	machine_config_additions - device-specific
//	machine configurations
//-------------------------------------------------

MACHINE_CONFIG_EXTERN( mzr8300 );

machine_config_constructor vme_mzr8300_card_device::device_mconfig_additions() const
{
	LOG("%s %s\n", tag(), FUNCNAME);
	return MACHINE_CONFIG_NAME( mzr8300 );
}

READ8_MEMBER (vme_mzr8300_card_device::read8){
	LOG("%s()\n", FUNCNAME);
	return (uint8_t) 0;
}

WRITE8_MEMBER (vme_mzr8300_card_device::write8){
	LOG("%s()\n", FUNCNAME);
}
