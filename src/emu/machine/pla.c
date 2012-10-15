/**********************************************************************

    PLS100 16x48x8 Programmable Logic Array emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "pla.h"



//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

const device_type PLS100 = &device_creator<pls100_device>;
const device_type MOS8721 = &device_creator<mos8721_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pla_device - constructor
//-------------------------------------------------

pla_device::pla_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int inputs, int outputs, int terms, UINT32 input_mask)
	: device_t(mconfig, type, name, tag, owner, clock),
	  m_inputs(inputs),
	  m_outputs(outputs),
	  m_terms(terms),
	  m_input_mask(((UINT64)input_mask << 32) | input_mask)
{
}

pls100_device::pls100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : pla_device(mconfig, PLS100, "PLS100", tag, owner, clock, 16, 8, 48, 0xffff)
{
}

mos8721_device::mos8721_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : pla_device(mconfig, MOS8721, "MOS8721", tag, owner, clock, 27, 18, 379, 0x7ffffff) // TODO actual number of terms is unknown
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pla_device::device_start()
{
	assert(machine().root_device().memregion(tag()) != NULL);

	// parse fusemap
	parse_fusemap();

	// clear cache
	for (int i = 0; i < CACHE_SIZE; i++)
	{
		m_cache[i] = 0;
	}

	m_cache_ptr = 0;
}


//-------------------------------------------------
//  parse_fusemap -
//-------------------------------------------------

void pla_device::parse_fusemap()
{
	memory_region *region = machine().root_device().memregion(tag());
	jed_data jed;
	
	jedbin_parse(region->base(), region->bytes(), &jed);

	UINT32 fusenum = 0;

	for (int p = 0; p < m_terms; p++)
	{
		term *term = &m_term[p];

		// AND mask
		term->m_and = 0;

		for (int i = 0; i < m_inputs; i++)
		{
			// complement
			term->m_and |= (UINT64)jed_get_fuse(&jed, fusenum++) << (i + 32);

			// true
			term->m_and |= (UINT64)jed_get_fuse(&jed, fusenum++) << i;
		}

		// OR mask
		term->m_or = 0;

		for (int f = 0; f < m_outputs; f++)
		{
			term->m_or |= !jed_get_fuse(&jed, fusenum++) << f;
		}

		term->m_or <<= 32;
	}

	// XOR mask
	m_xor = 0;

	for (int f = 0; f < m_outputs; f++)
	{
		m_xor |= jed_get_fuse(&jed, fusenum++) << f;
	}

	m_xor <<= 32;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT32 pla_device::read(UINT32 input)
{
	// try the cache first
	for (int i = 0; i < CACHE_SIZE; ++i)
	{
		UINT64 cache_entry = m_cache[i];

		if ((UINT32)cache_entry == input)
		{
			// cache hit
			return cache_entry >> 32;
		}
	}

	// cache miss, process terms
	UINT64 inputs = ((~(UINT64)input << 32) | input) & m_input_mask;
	UINT64 s = 0;

	for (int i = 0; i < m_terms; ++i)
	{
		term term = m_term[i];

		if ((term.m_and | inputs) == m_input_mask)
		{
			s |= term.m_or;
		}
	}

	s ^= m_xor;

	// store output in cache
	m_cache[m_cache_ptr] = s | input;
	++m_cache_ptr &= (CACHE_SIZE - 1);

	return s >> 32;
}
