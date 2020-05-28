// license:BSD-3-Clause
// copyright-holders:Vas Crabb,Aaron Giles
/***************************************************************************

    iptseqpoll.h

    Helper for letting the user select input sequences.

***************************************************************************/
#ifndef MAME_FRONTEND_IPTSEQPOLL_H
#define MAME_FRONTEND_IPTSEQPOLL_H

#pragma once


class input_sequence_poller
{
public:
	input_sequence_poller(input_manager &manager) noexcept;

	void start(input_item_class itemclass);
	void start(input_item_class itemclass, input_seq const &startseq);
	bool poll();

	input_seq const &sequence() const noexcept { return m_sequence; }
	bool valid() const noexcept { return m_sequence.is_valid(); }
	bool modified() const noexcept { return m_modified; }

private:
	void do_start(input_item_class itemclass);

	input_manager &m_manager;
	input_seq m_sequence;
	input_item_class m_class;
	osd_ticks_t m_last_ticks;
	bool m_modified;
};

#endif // MAME_FRONTEND_IPTSEQPOLL_H
