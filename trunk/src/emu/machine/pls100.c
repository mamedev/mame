/**********************************************************************

    PLS100 16x48x8 Programmable Logic Array emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "pls100.h"



//**************************************************************************
//  DEVICE TYPE DEFINITION
//**************************************************************************

const device_type PLS100 = &device_creator<pls100_device>;



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  parse_fusemap -
//-------------------------------------------------

inline void pls100_device::parse_fusemap()
{
	jed_data jed;
	jedbin_parse(machine().root_device().memregion(tag())->base(), machine().root_device().memregion(tag())->bytes(), &jed);
	UINT32 fusenum = 0;
	m_xor = 0;

	for (int term = 0; term < PAL_TERMS; term++)
	{
		m_and_comp[term] = 0;
		m_and_true[term] = 0;
		m_or[term] = 0;

		for (int i = 0; i < PAL_INPUTS; i++)
		{
			m_and_comp[term] |= jed_get_fuse(&jed, fusenum++) << i;
			m_and_true[term] |= jed_get_fuse(&jed, fusenum++) << i;
		}

		for (int f = 0; f < PAL_OUTPUTS; f++)
		{
			m_or[term] |= !jed_get_fuse(&jed, fusenum++) << f;
		}
	}

	for (int f = 0; f < PAL_OUTPUTS; f++)
	{
		m_xor |= jed_get_fuse(&jed, fusenum++) << f;
	}
}


//-------------------------------------------------
//  get_product -
//-------------------------------------------------

inline int pls100_device::get_product(int term)
{
	UINT16 input_true = m_and_true[term] | m_i;
	UINT16 input_comp = m_and_comp[term] | (m_i ^ 0xffff);

	return (input_true & input_comp) == 0xffff;
}


//-------------------------------------------------
//  update_outputs -
//-------------------------------------------------

inline void pls100_device::update_outputs()
{
	m_s = 0;

	for (int term = 0; term < PAL_TERMS; term++)
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
//  pls100_device - constructor
//-------------------------------------------------

pls100_device::pls100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, PLS100, "PLS100", tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pls100_device::device_start()
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

UINT8 pls100_device::read(UINT16 input)
{
	m_i = input;

	update_outputs();

	return m_s ^ m_xor;
}
