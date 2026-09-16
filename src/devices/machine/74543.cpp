// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*****************************************************************************

    74543: Octal Registered Transceiver
    (typically 74F543)

*****************************************************************************/

#include "emu.h"
#include "74543.h"

DEFINE_DEVICE_TYPE(TTL74543, ttl74543_device, "ttl74543", "74F543 Octal Registered Transceiver")

ttl74543_device::ttl74543_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TTL74543, tag, owner, clock),
	m_output_a(*this),
	m_output_b(*this),
	m_ceabpre(false),
	m_leabpre(false),
	m_oeabpre(false),
	m_cebapre(false),
	m_lebapre(false),
	m_oebapre(false)
{
}

void ttl74543_device::device_start()
{
	save_item(NAME(m_ceab));
	save_item(NAME(m_leab));
	save_item(NAME(m_oeab));
	save_item(NAME(m_ceba));
	save_item(NAME(m_leba));
	save_item(NAME(m_oeba));

	save_item(NAME(m_latch));
}

void ttl74543_device::device_reset()
{
	m_ceab = m_ceabpre;
	m_leab = m_leabpre;
	m_oeab = m_oeabpre;
	m_ceba = m_cebapre;
	m_leba = m_lebapre;
	m_oeba = m_oebapre;
}

/*
    Data I/O control table:

       Inputs            Latch      Output
  CEAB* LEAB* OEAB*      Status     Buffers
  -----------------------------------------
   H     X     X         Latched    High Z     A->B flow shown
   X     H     X         Latched      -        B->A flow control is the same
   L     L     X       Transparent    -        except using CEBA*, LEBA*, OEBA*
   X     X     H            -       High Z
   L     X     L            -       Driving

   X = immaterial
*/

void ttl74543_device::a_w(uint8_t a)
{
	if (m_ceab && m_leab) m_latch = a;
}

void ttl74543_device::b_w(uint8_t b)
{
	if (m_ceba && m_leba) m_latch = b;
}

void ttl74543_device::outputb_rz(uint8_t& value)
{
	if (m_ceab && m_oeab) value = m_latch;
}

void ttl74543_device::outputa_rz(uint8_t& value)
{
	if (m_ceba && m_oeba) value = m_latch;
}

void ttl74543_device::ceab_w(int state)
{
	m_ceab = (state == 0);
	if (m_ceab && m_oeab) m_output_b(m_latch);
}

void ttl74543_device::leab_w(int state)
{
	m_leab = (state == 0);
}

void ttl74543_device::oeab_w(int state)
{
	m_oeab = (state == 0);
	if (m_ceab && m_oeab) m_output_b(m_latch);
}

void ttl74543_device::ceba_w(int state)
{
	m_ceba = (state == 0);
	if (m_ceba && m_oeba) m_output_a(m_latch);
}

void ttl74543_device::leba_w(int state)
{
	m_leba = (state == 0);
}

void ttl74543_device::oeba_w(int state)
{
	m_oeba = (state == 0);
	if (m_ceba && m_oeba) m_output_a(m_latch);
}
