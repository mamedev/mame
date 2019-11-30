// license:BSD-3-Clause
// copyright-holders:Curt Coder, hap
/**********************************************************************

    PLA (Programmable Logic Array) emulation

**********************************************************************/

#include "emu.h"
#include "pla.h"
#include "jedparse.h"
#include "plaparse.h"

#define LOG_TERMS (1 << 0U)
//#define VERBOSE (LOG_TERMS)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(PLA, pla_device, "pla", "PLA")
DEFINE_DEVICE_TYPE(PLS100, pls100_device, "pls100", "82S100-series PLA")
DEFINE_DEVICE_TYPE(MOS8721, mos8721_device, "mos8721", "MOS 8721 PLA")

//-------------------------------------------------
//  pla_device - constructor
//-------------------------------------------------

pla_device::pla_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_region(*this, DEVICE_SELF)
	, m_format(FMT::JEDBIN)
	, m_inputs(0)
	, m_outputs(0)
	, m_terms(0)
	, m_input_mask(0)
	, m_xor(0)
	, m_cache_size(0), m_cache2_ptr(0)
{
}

pla_device::pla_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pla_device(mconfig, PLA, tag, owner, clock)
{
}

pls100_device::pls100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pla_device(mconfig, PLS100, tag, owner, clock)
{
	set_num_inputs(16);
	set_num_outputs(8);
	set_num_terms(48);
}

mos8721_device::mos8721_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pla_device(mconfig, MOS8721, tag, owner, clock)
{
	// TODO: actual number of terms is unknown
	set_num_inputs(27);
	set_num_outputs(18);
	set_num_terms(379);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pla_device::device_start()
{
	assert(m_terms < MAX_TERMS);
	assert(m_inputs < 32 && m_outputs <= 32);

	if (m_input_mask == 0)
		m_input_mask = ((uint64_t)1 << m_inputs) - 1;
	m_input_mask = ((uint64_t)m_input_mask << 32) | m_input_mask;

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
		case FMT::JEDBIN:
			result = jedbin_parse(m_region->base(), m_region->bytes(), &jed);
			break;

		case FMT::BERKELEY:
			result = pla_parse(m_region->base(), m_region->bytes(), &jed);
			break;
	}

	if (result != JEDERR_NONE)
	{
		for (int p = 0; p < m_terms; p++)
		{
			m_term[p].and_mask = 0;
			m_term[p].or_mask = 0;
		}

		logerror("%s PLA parse error %d!\n", tag(), result);
		return;
	}

	// parse it
	uint32_t fusenum = 0;

	for (int p = 0; p < m_terms; p++)
	{
		term *term = &m_term[p];

		// AND mask
		term->and_mask = 0;

		for (int i = 0; i < m_inputs; i++)
		{
			// complement
			term->and_mask |= (uint64_t)jed_get_fuse(&jed, fusenum++) << (i + 32);

			// true
			term->and_mask |= (uint64_t)jed_get_fuse(&jed, fusenum++) << i;
		}

		// OR mask
		term->or_mask = 0;

		for (int f = 0; f < m_outputs; f++)
		{
			term->or_mask |= !jed_get_fuse(&jed, fusenum++) << f;
		}

		term->or_mask <<= 32;

		LOGMASKED(LOG_TERMS, "F |= %0*X if (I & %0*X) == zeroes and (I & %0*X) == ones [term %d%s]\n",
			(m_outputs + 3) / 4,
			term->or_mask >> 32,
			(m_inputs + 3) / 4,
			(term->and_mask ^ m_input_mask) >> 32,
			(m_inputs + 3) / 4,
			uint32_t(term->and_mask ^ m_input_mask),
			p,
			(~term->and_mask & (~term->and_mask >> 32) & m_input_mask) == 0 ? "" : ", ignored");
	}

	// XOR mask
	m_xor = 0;

	for (int f = 0; f < m_outputs; f++)
	{
		m_xor |= jed_get_fuse(&jed, fusenum++) << f;
	}

	m_xor <<= 32;

	LOGMASKED(LOG_TERMS, "F ^= %0*X\n", (m_outputs + 3) / 4, m_xor >> 32);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint32_t pla_device::read(uint32_t input)
{
	// try the cache first
	if (input < m_cache_size)
		return m_cache[input];

	for (auto cache2_entry : m_cache2)
	{
		if ((uint32_t)cache2_entry == input)
		{
			// cache2 hit
			return cache2_entry >> 32;
		}
	}

	// cache miss, process terms
	uint64_t inputs = ((~(uint64_t)input << 32) | input) & m_input_mask;
	uint64_t s = 0;

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
