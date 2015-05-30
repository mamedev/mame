// license:BSD-3-Clause
// copyright-holders:Ville Linde
/***********************************************************************

    Konami K056800 (MIRAC)
    Sound interface and audio control

***********************************************************************/

#include "emu.h"
#include "sound/k056800.h"



const device_type K056800 = &device_creator<k056800_device>;



//-------------------------------------------------
//  k056800_device - constructor
//-------------------------------------------------

k056800_device::k056800_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, K056800, "K056800 MIRAC", tag, owner, clock, "k056800", __FILE__),
	m_int_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k056800_device::device_start()
{
	m_int_handler.resolve_safe();

	save_item(NAME(m_int_pending));
	save_item(NAME(m_host_to_snd_regs));
	save_item(NAME(m_snd_to_host_regs));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k056800_device::device_reset()
{
	m_int_pending = false;
	m_int_enabled = false;
	memset(m_host_to_snd_regs, 0, sizeof(m_host_to_snd_regs));
	memset(m_snd_to_host_regs, 0, sizeof(m_snd_to_host_regs));
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_MEMBER( k056800_device::host_r )
{
	UINT32 r = offset & 7;
	UINT8 data = 0;

	switch (r)
	{
		case 0:
		case 1:
			data = m_snd_to_host_regs[r];
			break;

		case 2:
			// .... ...x - Front volume busy
			// .... ..x. - Rear volume busy
			break;
	}

	return data;
}


WRITE8_MEMBER( k056800_device::host_w )
{
	UINT32 r = offset & 7;

	switch (r)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			m_host_to_snd_regs[r] = data;
			break;

		case 4:
			// xxxx xxxx - Front volume (CAh increments, 35h decrements)
			break;

		case 5:
			// xxxx xxxx - Rear volume (as above)
			break;

		case 6:
			// .... ...x - Mute front
			// .... ..x. - Mute rear
			break;

		case 7:
			// Sound interrupt
			m_int_pending = true;

			if (m_int_enabled)
				m_int_handler(ASSERT_LINE);

			break;
	}
}


READ8_MEMBER( k056800_device::sound_r )
{
	UINT32 r = offset & 7;
	UINT8 data = 0;

	switch (r)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			data = m_host_to_snd_regs[r];
			break;
	}

	return data;
}


WRITE8_MEMBER( k056800_device::sound_w )
{
	UINT32 r = offset & 7;

	switch (r)
	{
		case 0:
		case 1:
			m_snd_to_host_regs[r] = data;
			break;

		case 2:
		case 3:
			// TODO: Unknown
			break;

		case 4:
			// Sound CPU interrupt control
			m_int_enabled = (data & 1) != 0;

			if (m_int_enabled)
			{
				// Enable interrupt
				if (m_int_pending)
					m_int_handler(ASSERT_LINE);
			}
			else
			{
				// Disable/acknowledge interrupt
				m_int_pending = false;
				m_int_handler(CLEAR_LINE);
			}
			break;

		case 5:
			// TODO: Unknown
			break;
	}
}
