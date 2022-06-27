// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    inputseq.cpp

    A combination of hosts inputs that can be assigned to a control.

***************************************************************************/

#include "inputseq.h"


namespace osd {

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// additional expanded input codes for sequences
constexpr input_code input_seq::end_code;
constexpr input_code input_seq::default_code;
constexpr input_code input_seq::not_code;
constexpr input_code input_seq::or_code;

// constant sequences
const input_seq input_seq::empty_seq;



//**************************************************************************
//  INPUT SEQ
//**************************************************************************

//-------------------------------------------------
//  operator+= - append a code to the end of an
//  input sequence
//-------------------------------------------------

input_seq &input_seq::operator+=(input_code code) noexcept
{
	// if not enough room, return false
	const int curlength = length();
	if (curlength < m_code.size())
	{
		m_code[curlength] = code;
		if ((curlength + 1) < m_code.size())
			m_code[curlength + 1] = end_code;
	}
	return *this;
}


//-------------------------------------------------
//  operator|= - append a code to a sequence; if
//  the sequence is non-empty, insert an OR
//  before the new code
//-------------------------------------------------

input_seq &input_seq::operator|=(input_code code) noexcept
{
	// overwrite end/default with the new code
	if (m_code[0] == default_code)
	{
		m_code[0] = code;
		m_code[1] = end_code;
	}
	else
	{
		// otherwise, append an OR token and then the new code
		const int curlength = length();
		if ((curlength + 1) < m_code.size())
		{
			m_code[curlength] = or_code;
			m_code[curlength + 1] = code;
			if ((curlength + 2) < m_code.size())
				m_code[curlength + 2] = end_code;
		}
	}
	return *this;
}


//-------------------------------------------------
//  length - return the length of the sequence
//-------------------------------------------------

int input_seq::length() const noexcept
{
	// find the end token; error if none found
	for (int seqnum = 0; seqnum < m_code.size(); seqnum++)
		if (m_code[seqnum] == end_code)
			return seqnum;
	return m_code.size();
}


//-------------------------------------------------
//  is_valid - return true if a given sequence is
//  valid
//-------------------------------------------------

bool input_seq::is_valid() const noexcept
{
	// "default" can only be of length 1
	if (m_code[0] == default_code)
		return m_code[1] == end_code;

	// scan the sequence for valid codes
	input_item_class lastclass = ITEM_CLASS_INVALID;
	input_code lastcode = INPUT_CODE_INVALID;
	decltype(m_code) positive_codes;
	decltype(m_code) negative_codes;
	auto positive_codes_end = positive_codes.begin();
	auto negative_codes_end = negative_codes.begin();
	for (input_code code : m_code)
	{
		// invalid codes are never permitted
		if (code == INPUT_CODE_INVALID)
			return false;

		// if we hit an OR or the end, validate the previous chunk
		if (code == or_code || code == end_code)
		{
			// must be at least one positive code
			if (positive_codes.begin() == positive_codes_end)
				return false;

			// last code must not have been an internal code
			if (lastcode.internal())
				return false;

			// if this is the end, we're ok
			if (code == end_code)
				return true;

			// reset the state for the next chunk
			positive_codes_end = positive_codes.begin();
			negative_codes_end = negative_codes.begin();
			lastclass = ITEM_CLASS_INVALID;
		}
		else if (code == not_code)
		{
			// if we hit a NOT, make sure we don't have a double
			if (lastcode == not_code)
				return false;
		}
		else
		{
			// track positive codes, and don't allow positive and negative for the same code
			if (lastcode != not_code)
			{
				*positive_codes_end++ = code;
				if (std::find(negative_codes.begin(), negative_codes_end, code) != negative_codes_end)
					return false;
			}
			else
			{
				*negative_codes_end++ = code;
				if (std::find(positive_codes.begin(), positive_codes_end, code) != positive_codes_end)
					return false;
			}

			// non-switch items can't have a NOT
			input_item_class itemclass = code.item_class();
			if (itemclass != ITEM_CLASS_SWITCH && lastcode == not_code)
				return false;

			// absolute/relative items must all be the same class
			if ((lastclass == ITEM_CLASS_ABSOLUTE && itemclass != ITEM_CLASS_ABSOLUTE) ||
				(lastclass == ITEM_CLASS_RELATIVE && itemclass != ITEM_CLASS_RELATIVE))
				return false;
		}

		// remember the last code
		lastcode = code;
	}

	// if we got here, we were missing an END token; fail
	return false;
}


//-------------------------------------------------
//  backspace - "backspace" over the last entry in
//  a sequence
//-------------------------------------------------

void input_seq::backspace() noexcept
{
	// if we have at least one entry, remove it
	const int curlength = length();
	if (curlength > 0)
		m_code[curlength - 1] = end_code;
}


//-------------------------------------------------
//  replace - replace all instances of oldcode
//  with newcode in a sequence
//-------------------------------------------------

void input_seq::replace(input_code oldcode, input_code newcode) noexcept
{
	for (input_code &elem : m_code)
		if (elem == oldcode)
			elem = newcode;
}

} // namespace osd
