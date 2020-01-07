// license:BSD-3-Clause
// copyright-holders:Vas Crabb,Aaron Giles

#include "emu.h"
#include "iptseqpoll.h"

#include <cassert>


input_sequence_poller::input_sequence_poller(input_manager &manager) noexcept :
	m_manager(manager),
	m_sequence(),
	m_class(ITEM_CLASS_INVALID),
	m_last_ticks(0),
	m_modified(false)
{
}


void input_sequence_poller::start(input_item_class itemclass)
{
	m_sequence.reset();
	do_start(itemclass);
}


void input_sequence_poller::start(input_item_class itemclass, input_seq const &startseq)
{
	// grab the starting sequence to append to, and append an OR if it isn't empty
	m_sequence = startseq;
	if (input_seq::end_code != m_sequence[0])
		m_sequence += input_seq::or_code;

	do_start(itemclass);
}


bool input_sequence_poller::poll()
{
	assert((ITEM_CLASS_SWITCH == m_class) || (ITEM_CLASS_ABSOLUTE == m_class) || (ITEM_CLASS_RELATIVE == m_class));
	int const curlen = m_sequence.length();
	input_code lastcode = m_sequence[curlen - 1];

	input_code newcode;
	if (ITEM_CLASS_SWITCH == m_class)
	{
		// switch case: see if we have a new code to process
		newcode = m_manager.poll_switches();
		if (INPUT_CODE_INVALID != newcode)
		{
			// if code is duplicate, toggle the NOT state on the code
			if (curlen && (newcode == lastcode))
			{
				// back up over the existing code
				m_sequence.backspace();

				// if there was a NOT preceding it, delete it as well, otherwise append a fresh one
				if (m_sequence[curlen - 2] == input_seq::not_code)
					m_sequence.backspace();
				else
					m_sequence += input_seq::not_code;
			}
		}
	}
	else
	{
		// absolute/relative case: see if we have an analog change of sufficient amount
		bool const has_or = input_seq::or_code == lastcode;
		if (has_or)
			lastcode = m_sequence[curlen - 2];
		newcode = m_manager.poll_axes();

		// if the last code doesn't match absolute/relative of this code, ignore the new one
		input_item_class const lastclass = lastcode.item_class();
		input_item_class const newclass = newcode.item_class();
		if (((ITEM_CLASS_ABSOLUTE == lastclass) && (ITEM_CLASS_ABSOLUTE != newclass)) ||
			((ITEM_CLASS_RELATIVE == lastclass) && (ITEM_CLASS_RELATIVE != newclass)))
			newcode = INPUT_CODE_INVALID;

		// if the new code is valid, check for half-axis toggles on absolute controls
		if ((INPUT_CODE_INVALID != newcode) && curlen && (ITEM_CLASS_ABSOLUTE == newclass))
		{
			input_code last_nomodifier = lastcode;
			last_nomodifier.set_item_modifier(ITEM_MODIFIER_NONE);
			if (newcode == last_nomodifier)
			{
				// increment the modifier, wrapping back to none
				switch (lastcode.item_modifier())
				{
				case ITEM_MODIFIER_NONE:
					newcode.set_item_modifier(ITEM_MODIFIER_POS);
					break;
				case ITEM_MODIFIER_POS:
					newcode.set_item_modifier(ITEM_MODIFIER_NEG);
					break;
				default:
				case ITEM_MODIFIER_NEG:
					newcode.set_item_modifier(ITEM_MODIFIER_NONE);
					break;
				}

				// back up over the previous code so we can re-append
				if (has_or)
					m_sequence.backspace();
				m_sequence.backspace();
			}
		}
	}

	// if we got a new code to append it, append it and reset the timer
	osd_ticks_t const newticks = osd_ticks();
	if (INPUT_CODE_INVALID != newcode)
	{
		m_sequence += newcode;
		m_last_ticks = newticks;
		m_modified = true;
	}

	// if we're recorded at least one item and 2/3 of a second has passed, we're done
	if (m_last_ticks && ((m_last_ticks + (osd_ticks_per_second() * 2 / 3)) < newticks))
	{
		m_class = ITEM_CLASS_INVALID;
		return true;
	}

	// return false to indicate we are still polling
	return false;
}


void input_sequence_poller::do_start(input_item_class itemclass)
{
	assert((ITEM_CLASS_SWITCH == itemclass) || (ITEM_CLASS_ABSOLUTE == itemclass) || (ITEM_CLASS_RELATIVE == itemclass));

	// reset the recording count and the clock
	m_class = itemclass;
	m_last_ticks = 0;
	m_modified = false;

	// wait for any inputs that are already active to be released
	m_manager.reset_polling();
	for (input_code dummycode = KEYCODE_ENTER; INPUT_CODE_INVALID != dummycode; )
		dummycode = (ITEM_CLASS_SWITCH == itemclass) ? m_manager.poll_switches() : m_manager.poll_axes();
}
