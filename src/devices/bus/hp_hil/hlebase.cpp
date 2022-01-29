// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#include "emu.h"
#include "hlebase.h"

//#define VERBOSE 1
#include "logmacro.h"

namespace bus::hp_hil {

/***************************************************************************
    BASE HLE KEYBOARD DEVICE
***************************************************************************/

/*--------------------------------------------------
    hle_device_base::hle_device_base
    designated device constructor
--------------------------------------------------*/

hle_device_base::hle_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_hp_hil_interface(mconfig, *this)
	, m_powerup(true)
	, m_passthru(false)
{ }


/*--------------------------------------------------
    hle_device_base::~hle_device_base
    destructor
--------------------------------------------------*/

hle_device_base::~hle_device_base()
{ }


/*--------------------------------------------------
    hle_device_base::device_start
    perform expensive initialisations, allocate
    resources, register for save state
--------------------------------------------------*/

void hle_device_base::device_start()
{

	save_item(NAME(m_powerup));
	save_item(NAME(m_passthru));

	set_hp_hil_mlc_device();

	m_powerup = true;
	m_passthru = false;
}


/*--------------------------------------------------
    hle_device_base::device_reset
    perform startup tasks, also used for host
    requested reset
--------------------------------------------------*/

void hle_device_base::device_reset()
{
}

bool hle_device_base::hil_write(uint16_t *pdata)
{
	int frames = 0;
	uint8_t addr = (*pdata >> 8) & 7;
	uint8_t data = *pdata & 0xff;
	bool command = BIT(*pdata, 11);

	LOG("%d: rx from mlc %04X (%s addr %d, data %02X)\n", m_device_id, *pdata,
		command ? "command" : "data", addr, data);

	if (!command)
		goto out;

	if (addr != 0 && addr != m_device_id)
		goto out;

	switch (data)
	{
	case HPHIL_IFC:
		m_powerup = false;
		break;

	case HPHIL_EPT:
		m_passthru = true;
		break;

	case HPHIL_ELB:
		m_passthru = false;
		break;

	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
		m_device_id = data - 8;
		m_device_id16 = (data - 8) << 8;
		*pdata &= ~7;
		*pdata += (data - 7);
		break;

	case HPHIL_POL:
	case HPHIL_POL+1:
	case HPHIL_POL+2:
	case HPHIL_POL+3:
	case HPHIL_POL+4:
	case HPHIL_POL+5:
	case HPHIL_POL+6:
	case HPHIL_POL+7:
	case HPHIL_POL+8:
	case HPHIL_POL+9:
	case HPHIL_POL+10:
	case HPHIL_POL+11:
	case HPHIL_POL+12:
	case HPHIL_POL+13:
	case HPHIL_POL+14:
	case HPHIL_POL+15:
		frames = hil_poll();
		*pdata += frames;
		break;

	case HPHIL_DSR:
		m_device_id = m_device_id16 = 0;
		m_powerup = true;
		break;

	case HPHIL_IDD:
		hil_idd();
		break;

	case HPHIL_DHR:
		m_powerup = true;
		m_passthru = false;
		device_reset();
		return true;
		break;

	case HPHIL_DKA:
	case HPHIL_EK1:
	case HPHIL_EK2:
		hil_typematic(data);
		break;

	default:
		LOG("command %02X unknown\n", data);
		break;
	}
out:
	if (!m_passthru)
		m_hp_hil_mlc->hil_write(*pdata);
	return m_passthru;
}

} // namespace bus::hp_hil
