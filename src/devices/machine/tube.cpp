// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Tube ULA emulation

    The Tube ULA acts as a parallel interface between two asynchronous
    processor systems. It consists of four byte-wide read-only registers
    and four byte-wide write-only registers. Eight bytes of memory mapped
    I/O space are used to address these registers, four for the data
    registers and four for the associated status registers.

**********************************************************************/

#include "emu.h"
#include "machine/tube.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TUBE, tube_device, "tube", "Acorn Tube ULA")



//-------------------------------------------------
//  bbc_tube_slot_device - constructor
//-------------------------------------------------

tube_device::tube_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TUBE, tag, owner, clock),
	m_hirq_handler(*this),
	m_pnmi_handler(*this),
	m_pirq_handler(*this),
	m_drq_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tube_device::device_start()
{
	// resolve callbacks
	m_hirq_handler.resolve_safe();
	m_pnmi_handler.resolve_safe();
	m_pirq_handler.resolve_safe();
	m_drq_handler.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tube_device::device_reset()
{
	m_ph1pos = m_hp3pos = 0;
	m_ph3pos = 1;
	m_r1stat = 0;
	m_hstat[0] = m_hstat[1] = m_hstat[3] = 0x40;
	m_hstat[2] = 0xc0;
	m_pstat[0] = m_pstat[1] = m_pstat[2] = m_pstat[3] = 0x40;
}


void tube_device::update_interrupts()
{
	m_hirq_handler(BIT(m_r1stat, 0) && BIT(m_hstat[3], 7) ? ASSERT_LINE : CLEAR_LINE);

	m_pirq_handler((BIT(m_r1stat, 1) && BIT(m_pstat[0], 7)) || (BIT(m_r1stat, 2) && BIT(m_pstat[3], 7)) ? ASSERT_LINE : CLEAR_LINE);

	m_pnmi_handler(BIT(m_r1stat, 3) && ((m_hp3pos > BIT(m_r1stat, 4)) || (m_ph3pos == 0)) ? ASSERT_LINE : CLEAR_LINE);

	m_drq_handler(!BIT(m_r1stat, 4) && ((m_hp3pos > BIT(m_r1stat, 4)) || (m_ph3pos == 0)) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t tube_device::host_r(offs_t offset)
{
	uint8_t data = 0xfe;

	switch (offset & 0x07)
	{
	case 0: /* Status and Register 1 flags */
		data = (m_hstat[0] & 0xc0) | m_r1stat;
		break;

	case 1: /* Register 1 */
		data = m_ph1[0];
		if (!machine().side_effects_disabled())
		{
			for (int i = 0; i < 23; i++) m_ph1[i] = m_ph1[i + 1];
			m_ph1pos--;
			m_pstat[0] |= 0x40;
			if (!m_ph1pos) m_hstat[0] &= ~0x80;
		}
		break;

	case 2: /* Register 2 flags */
		data = m_hstat[1];
		break;

	case 3: /* Register 2 */
		data = m_ph2;
		if (BIT(m_hstat[1], 7) && !machine().side_effects_disabled())
		{
			m_hstat[1] &= ~0x80;
			m_pstat[1] |= 0x40;
		}
		break;

	case 4: /* Register 3 flags */
		data = m_hstat[2];
		break;

	case 5: /* Register 3 */
		data = m_ph3[0];
		if ((m_ph3pos > 0) && !machine().side_effects_disabled())
		{
			m_ph3[0] = m_ph3[1];
			m_ph3pos--;
			m_pstat[2] |= 0xc0;
			if (!m_ph3pos) m_hstat[2] &= ~0x80;
		}
		break;

	case 6: /* Register 4 flags */
		data = m_hstat[3];
		break;

	case 7: /* Register 4 */
		data = m_ph4;
		if (BIT(m_hstat[3], 7) && !machine().side_effects_disabled())
		{
			m_hstat[3] &= ~0x80;
			m_pstat[3] |= 0x40;
		}
		break;
	}
	update_interrupts();

	return data;
}

void tube_device::host_w(offs_t offset, uint8_t data)
{
	switch (offset & 0x07)
	{
	case 0: /* Status flags */
		if (BIT(data, 7))
			m_r1stat |= (data & 0x3f);
		else
			m_r1stat &= ~(data & 0x3f);
		m_hstat[0] = (m_hstat[0] & 0xc0) | (data & 0x3f);
		break;

	case 1: /* Register 1 */
		m_hp1 = data;
		m_pstat[0] |= 0x80;
		m_hstat[0] &= ~0x40;
		break;

	case 3: /* Register 2 */
		m_hp2 = data;
		m_pstat[1] |= 0x80;
		m_hstat[1] &= ~0x40;
		break;

	case 5: /* Register 3 */
		if (BIT(m_r1stat, 4))
		{
			if (m_hp3pos < 2)
				m_hp3[m_hp3pos++] = data;
			if (m_hp3pos == 2)
			{
				m_pstat[2] |= 0x80;
				m_hstat[2] &= ~0x40;
			}
		}
		else
		{
			m_hp3[0] = data;
			m_hp3pos = 1;
			m_pstat[2] |= 0x80;
			m_hstat[2] &= ~0x40;
		}
		break;

	case 7: /* Register 4 */
		m_hp4 = data;
		m_pstat[3] |= 0x80;
		m_hstat[3] &= ~0x40;
		break;
	}

	update_interrupts();
}

uint8_t tube_device::parasite_r(offs_t offset)
{
	uint8_t data = 0x00;

	switch (offset & 0x07)
	{
	case 0: /*Register 1 flags */
		data = m_pstat[0] | m_r1stat;
		break;

	case 1: /* Register 1 */
		data = m_hp1;
		if (BIT(m_pstat[0], 7) && !machine().side_effects_disabled())
		{
			m_pstat[0] &= ~0x80;
			m_hstat[0] |= 0x40;
		}
		break;

	case 2: /* Register 2 flags */
		data = m_pstat[1];
		break;

	case 3: /* Register 2 */
		data = m_hp2;
		if (BIT(m_pstat[1], 7) && !machine().side_effects_disabled())
		{
			m_pstat[1] &= ~0x80;
			m_hstat[1] |= 0x40;
		}
		break;

	case 4: /* Register 3 flags */
		data = m_pstat[2];
		break;

	case 5: /* Register 3 */
		data = m_hp3[0];
		if ((m_hp3pos > 0) && !machine().side_effects_disabled())
		{
			m_hp3[0] = m_hp3[1];
			m_hp3pos--;
			if (!m_hp3pos)
			{
				m_hstat[2] |= 0x40;
				m_pstat[2] &= ~0x80;
			}
		}
		break;

	case 6: /* Register 4 flags */
		data = m_pstat[3];
		break;

	case 7: /* Register 4 */
		data = m_hp4;
		if (BIT(m_pstat[3], 7) && !machine().side_effects_disabled())
		{
			m_pstat[3] &= ~0x80;
			m_hstat[3] |= 0x40;
		}
		break;
	}
	update_interrupts();

	return data;
}

void tube_device::parasite_w(offs_t offset, uint8_t data)
{
	switch (offset & 0x07)
	{
	case 1: /* Register 1 */
		if (m_ph1pos < 24)
		{
			m_ph1[m_ph1pos++] = data;
			m_hstat[0] |= 0x80;
			if (m_ph1pos == 24)
				m_pstat[0] &= ~0x40;
		}
		break;

	case 3: /* Register 2 */
		m_ph2 = data;
		m_hstat[1] |= 0x80;
		m_pstat[1] &= ~0x40;
		break;

	case 5: /* Register 3 */
		if (BIT(m_r1stat, 4))
		{
			if (m_ph3pos < 2)
				m_ph3[m_ph3pos++] = data;
			if (m_ph3pos == 2)
			{
				m_hstat[2] |= 0x80;
				m_pstat[2] &= ~0x40;
			}
		}
		else
		{
			m_ph3[0] = data;
			m_ph3pos = 1;
			m_hstat[2] |= 0x80;
			m_pstat[2] &= ~0xc0;
		}
		break;

	case 7: /* Register 4 */
		m_ph4 = data;
		m_hstat[3] |= 0x80;
		m_pstat[3] &= ~0x40;
		break;
	}
	update_interrupts();
}
