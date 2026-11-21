// license:GPL-2.0+
// copyright-holders:Andrea Bogazzi
#ifndef MAME_NOKIA_NGAGE_SIM_H
#define MAME_NOKIA_NGAGE_SIM_H

#pragma once

class ngage_sim_device : public device_t
{
public:
	ngage_sim_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	void set_present(bool present) { m_present = present; }
	u16 status() const { return m_status; }
	void acknowledge(u16 bits);
	void start(u16 command);
	u8 read();
	void write(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void reply_status(u16 status);
	void command(u8 length);

	enum : u8 { ATR, TX_COMPLETE, RX };

	bool m_present = false;
	bool m_started = false;
	u16 m_status = 0;
	u8 m_phase = ATR;
	u8 m_position = 0;
	u8 m_reply[25]{};
	u8 m_length = 0;
	u16 m_directory = 0x3f00;
	u16 m_selected = 0x3f00;
	u16 m_file_id = 0;
	u8 m_header[4]{};
	u8 m_header_count = 0;
	u8 m_select_left = 0;
	u8 m_terminal_left = 0;
};

DECLARE_DEVICE_TYPE(NGAGE_SIM, ngage_sim_device)

#endif // MAME_NOKIA_NGAGE_SIM_H
