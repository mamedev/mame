// license:BSD-3-Clause
// copyright-holders:
/*
 *  session time
 *
 */

#ifndef MAME_MACHINE_SESSION_TIME_H
#define MAME_MACHINE_SESSION_TIME_H

#pragma once

class session_time_device : public device_t
{
public:
	session_time_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	session_time_device(const machine_config &mconfig, const char *tag, device_t *owner, int paperwidth, int paperheight) :
		session_time_device(mconfig, tag, owner, u32(0))
	{

	}

protected:
	session_time_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

public:

protected:
	// device-level overrides

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	std::string m_printername;
	std::string m_snapshotdir;
	time_t m_session_time;

public:

	DECLARE_INPUT_CHANGED_MEMBER(reset_session_time);
public:

	u32 pagecount = 0;
	device_t* getrootdev();
	std::string fixchar(std::string in, char from, char to);
	std::string fixcolons(std::string in);
	std::string sessiontime();
	std::string tagname();
	std::string simplename();
	void setprintername(std::string name){ m_printername = name; }
	std::string getprintername(){ return m_printername; }
	void initprintername(){ setprintername(sessiontime() + std::string("_") + simplename()); }

	std::string padzeroes( std::string s, int len) { return std::string(len - s.length(), '0') + s; }
};

DECLARE_DEVICE_TYPE(SESSION_TIME, session_time_device)

#endif // MAME_MACHINE_SESSION_TIME_H
