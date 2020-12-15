// license:BSD-3-Clause
// copyright-holders:Vas Crabb,Aaron Giles
/***************************************************************************

    iptseqpoll.h

    Helper for letting the user select input sequences.

***************************************************************************/
#ifndef MAME_FRONTEND_IPTSEQPOLL_H
#define MAME_FRONTEND_IPTSEQPOLL_H

#pragma once

#include <vector>


class input_code_poller
{
public:
	virtual ~input_code_poller();

	virtual void reset();
	virtual input_code poll() = 0;

protected:
	input_code_poller(input_manager &manager) noexcept;

	input_manager &m_manager;
};


class switch_code_poller_base : public input_code_poller
{
public:
	virtual void reset() override;

protected:
	switch_code_poller_base(input_manager &manager) noexcept;

	bool code_pressed_once(input_code code);

private:
	std::vector<input_code> m_switch_memory;
};


class axis_code_poller : public input_code_poller
{
public:
	axis_code_poller(input_manager &manager) noexcept;

	virtual input_code poll() override;
};


class switch_code_poller : public switch_code_poller_base
{
public:
	switch_code_poller(input_manager &manager) noexcept;

	virtual input_code poll() override;
};


class keyboard_code_poller : public switch_code_poller_base
{
public:
	keyboard_code_poller(input_manager &manager) noexcept;

	virtual input_code poll() override;
};


class input_sequence_poller
{
public:
	virtual ~input_sequence_poller();

	void start();
	void start(input_seq const &startseq);
	bool poll();

	input_seq const &sequence() const noexcept { return m_sequence; }
	bool valid() const noexcept { return m_sequence.is_valid(); }
	bool modified() const noexcept { return m_modified; }

protected:
	input_sequence_poller() noexcept;

	input_seq m_sequence;

private:
	virtual void do_start() = 0;
	virtual input_code do_poll() = 0;

	osd_ticks_t m_last_ticks;
	bool m_modified;
};


class axis_sequence_poller : public input_sequence_poller
{
public:
	axis_sequence_poller(input_manager &manager) noexcept;

private:
	virtual void do_start() override;
	virtual input_code do_poll() override;

	axis_code_poller m_code_poller;
};


class switch_sequence_poller : public input_sequence_poller
{
public:
	switch_sequence_poller(input_manager &manager) noexcept;

private:
	virtual void do_start() override;
	virtual input_code do_poll() override;

	switch_code_poller m_code_poller;
};

#endif // MAME_FRONTEND_IPTSEQPOLL_H
