// license:BSD-3-Clause
// copyright-holders:Vas Crabb,Aaron Giles
/***************************************************************************

    iptseqpoll.h

    Helper for letting the user select input sequences.

***************************************************************************/
#ifndef MAME_FRONTEND_IPTSEQPOLL_H
#define MAME_FRONTEND_IPTSEQPOLL_H

#pragma once

#include <utility>
#include <vector>


class input_code_poller
{
public:
	virtual ~input_code_poller();

	virtual void reset();
	virtual input_code poll() = 0;

protected:
	input_code_poller(input_manager &manager) noexcept;

	bool code_pressed_once(input_code code, bool moved);

	input_manager &m_manager;
	std::vector<std::pair<input_device_item *, s32> > m_axis_memory;
	std::vector<input_code> m_switch_memory;
};


class axis_code_poller : public input_code_poller
{
public:
	axis_code_poller(input_manager &manager) noexcept;

	virtual void reset() override;
	virtual input_code poll() override;

private:
	std::vector<bool> m_axis_active;
};


class switch_code_poller : public input_code_poller
{
public:
	switch_code_poller(input_manager &manager) noexcept;

	virtual input_code poll() override;
};


class keyboard_code_poller : public input_code_poller
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

	void set_modified() noexcept { m_modified = true; }

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
