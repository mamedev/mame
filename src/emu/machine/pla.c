/**********************************************************************

    PLS100 16x48x8 Programmable Logic Array emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "pla.h"



//**************************************************************************
//  DEVICE TYPE DEFINITION
//**************************************************************************

const device_type PLS100 = &device_creator<pls100_device>;
const device_type MOS8721 = &device_creator<mos8721_device>;



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  parse_fusemap -
//-------------------------------------------------

inline void pla_device::parse_fusemap()
{
	jed_data jed;
	jedbin_parse(machine().root_device().memregion(tag())->base(), machine().root_device().memregion(tag())->bytes(), &jed);
	UINT32 fusenum = 0;
	m_xor = 0;

	for (int term = 0; term < m_terms; term++)
	{
		m_and_comp[term] = 0;
		m_and_true[term] = 0;
		m_or[term] = 0;

		for (int i = 0; i < m_inputs; i++)
		{
			m_and_comp[term] |= jed_get_fuse(&jed, fusenum++) << i;
			m_and_true[term] |= jed_get_fuse(&jed, fusenum++) << i;
		}

		for (int f = 0; f < m_outputs; f++)
		{
			m_or[term] |= !jed_get_fuse(&jed, fusenum++) << f;
		}
	}

	for (int f = 0; f < m_outputs; f++)
	{
		m_xor |= jed_get_fuse(&jed, fusenum++) << f;
	}
}


//-------------------------------------------------
//  get_product -
//-------------------------------------------------

inline bool pla_device::get_product(int term)
{
	UINT32 input_true = m_and_true[term] | m_i;
	UINT32 input_comp = m_and_comp[term] | ~m_i;

	return ((input_true & input_comp) & m_output_mask) == m_output_mask;
}


//-------------------------------------------------
//  update_outputs -
//-------------------------------------------------

inline void pla_device::update_outputs()
{
	m_s = 0;

	for (int term = 0; term < m_terms; term++)
	{
		if (get_product(term))
		{
			m_s |= m_or[term];
		}
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pla_device - constructor
//-------------------------------------------------

pla_device::pla_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int inputs, int outputs, int terms, UINT32 output_mask)
	: device_t(mconfig, type, name, tag, owner, clock),
	  m_inputs(inputs),
	  m_outputs(outputs),
	  m_terms(terms),
	  m_output_mask(output_mask)
{
}

pls100_device::pls100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : pla_device(mconfig, PLS100, "PLS100", tag, owner, clock, 16, 8, 48, 0xffff)
{
}

mos8721_device::mos8721_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : pla_device(mconfig, MOS8721, "MOS8721", tag, owner, clock, 27, 18, 48, 0x7ffffff)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pla_device::device_start()
{
	// parse fusemap
	assert(machine().root_device().memregion(tag()) != NULL);
	parse_fusemap();

	// register for state saving
	save_item(NAME(m_i));
	save_item(NAME(m_s));
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT32 pla_device::read(UINT32 input)
{
	m_i = input;

	update_outputs();

	return m_s ^ m_xor;
}
