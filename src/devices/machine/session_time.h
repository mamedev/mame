// license:BSD-3-Clause
// copyright-holders:
/*

	session time device (for use by bitmap_printer)

*/

#ifndef MAME_MACHINE_SESSION_TIME_H
#define MAME_MACHINE_SESSION_TIME_H

#pragma once

class session_time_device : public device_t
{
public:
	session_time_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	session_time_device(const machine_config &mconfig, const char *tag, device_t *owner, int skip_device_level) :
		session_time_device(mconfig, tag, owner, u32(0))
	{
		m_skip_device_level = skip_device_level;  // when building name, skip the last x devices
	}

protected:
	session_time_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	std::string m_printername;
	time_t m_session_time;
	int m_skip_device_level = 0;
public:

	DECLARE_INPUT_CHANGED_MEMBER(reset_session_time);
public:

	u32 page_num = 1;  // start numbering at 1
	std::string sessiontime();
	std::string build_name_skip_last(int skipcount);
	void setprintername(std::string name){ m_printername = name; }
	std::string getprintername(){ return m_printername; }
	void initprintername(){ setprintername(sessiontime() + std::string("_") + build_name_skip_last(m_skip_device_level)); }

	std::string padzeroes( std::string s, int len) { return std::string(len - s.length(), '0') + s; }

private:
	device_t* getrootdev();
	std::string fixchar(std::string in, char from, char to);
	std::string fixcolons(std::string in);
};

DECLARE_DEVICE_TYPE(SESSION_TIME, session_time_device)

#endif // MAME_MACHINE_SESSION_TIME_H
