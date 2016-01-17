// license:BSD-3-Clause
// copyright-holders:Curt Coder, hap
/**********************************************************************

    PLA (Programmable Logic Array) emulation

**********************************************************************/

#include "pla.h"
#include "jedparse.h"
#include "plaparse.h"


const device_type PLA = &device_creator<pla_device>;

//-------------------------------------------------
//  pla_device - constructor
//-------------------------------------------------

pla_device::pla_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PLA, "PLA", tag, owner, clock, "pla", __FILE__),
		m_format(PLA_FMT_JEDBIN),
		m_inputs(0),
		m_outputs(0),
		m_terms(0),
		m_input_mask(0),
		m_xor(0), m_cache_size(0), m_cache2_ptr(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pla_device::device_start()
{
	assert(region() != nullptr);
	assert(m_terms < MAX_TERMS);
	assert(m_inputs < 32 && m_outputs <= 32);

	if (m_input_mask == 0)
		m_input_mask = ((UINT64)1 << m_inputs) - 1;
	m_input_mask = ((UINT64)m_input_mask << 32) | m_input_mask;

	// parse fusemap
	parse_fusemap();

	// initialize cache
	m_cache2_ptr = 0;
	for (auto & elem : m_cache2)
		elem = 0x80000000;

	m_cache_size = 0;
	int csize = 1 << ((m_inputs > MAX_CACHE_BITS) ? MAX_CACHE_BITS : m_inputs);
	m_cache.resize(csize);
	for (int i = 0; i < csize; i++)
		m_cache[i] = read(i);

	m_cache_size = csize;
}


//-------------------------------------------------
//  parse_fusemap -
//-------------------------------------------------

void pla_device::parse_fusemap()
{
	jed_data jed;
	int result = JEDERR_NONE;

	// read pla file
	switch (m_format)
	{
		case PLA_FMT_JEDBIN:
			result = jedbin_parse(region()->base(), region()->bytes(), &jed);
			break;

		case PLA_FMT_BERKELEY:
			result = pla_parse(region()->base(), region()->bytes(), &jed);
			break;
	}

	if (result != JEDERR_NONE)
	{
		for (int p = 0; p < m_terms; p++)
		{
			m_term[p].and_mask = 0;
			m_term[p].or_mask = 0;
		}

		logerror("%s PLA parse error %d!\n", tag().c_str(), result);
		return;
	}

	// parse it
	UINT32 fusenum = 0;

	for (int p = 0; p < m_terms; p++)
	{
		term *term = &m_term[p];

		// AND mask
		term->and_mask = 0;

		for (int i = 0; i < m_inputs; i++)
		{
			// complement
			term->and_mask |= (UINT64)jed_get_fuse(&jed, fusenum++) << (i + 32);

			// true
			term->and_mask |= (UINT64)jed_get_fuse(&jed, fusenum++) << i;
		}

		// OR mask
		term->or_mask = 0;

		for (int f = 0; f < m_outputs; f++)
		{
			term->or_mask |= !jed_get_fuse(&jed, fusenum++) << f;
		}

		term->or_mask <<= 32;
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
	if (input < m_cache_size)
		return m_cache[input];

	for (auto cache2_entry : m_cache2)
	{
		if ((UINT32)cache2_entry == input)
		{
			// cache2 hit
			return cache2_entry >> 32;
		}
	}

	// cache miss, process terms
	UINT64 inputs = ((~(UINT64)input << 32) | input) & m_input_mask;
	UINT64 s = 0;

	for (int i = 0; i < m_terms; ++i)
	{
		term* term = &m_term[i];

		if ((term->and_mask | inputs) == m_input_mask)
		{
			s |= term->or_mask;
		}
	}

	s ^= m_xor;

	// store output in cache2
	m_cache2[m_cache2_ptr] = s | input;
	++m_cache2_ptr &= (CACHE2_SIZE - 1);

	return s >> 32;
}
