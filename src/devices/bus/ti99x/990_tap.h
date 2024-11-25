// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    990_tap.h: include file for 990_tap.c
*/
#ifndef MAME_BUS_TI99X_990_TAP_H
#define MAME_BUS_TI99X_990_TAP_H

#pragma once

DECLARE_DEVICE_TYPE(TI990_TAPE_CTRL, tap_990_device)

class tap_990_device : public device_t
{
public:
	tap_990_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void set_tape(int id, device_image_interface *img, bool bot, bool eot, bool wp)
	{
		m_tape[id].img = img;
		m_tape[id].bot = bot;
		m_tape[id].eot = eot;
		m_tape[id].wp = wp;
	}

	template <typename T> void set_memory_space(T &&tag, int spacenum) { m_memory_space.set_tag(std::forward<T>(tag), spacenum); }
	auto int_cb() { return m_int_line.bind(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr unsigned MAX_TAPE_UNIT = 4;

	struct tape_unit_t
	{
		device_image_interface *img;        // image descriptor
		bool bot;   // true if we are at the beginning of tape
		bool eot;   // true if we are at the end of tape
		bool wp;    // true if tape is write-protected
	};

	int     cur_tape_unit();
	void    update_interrupt();
	void    cmd_read_binary_forward();
	void    cmd_record_skip_forward();
	void    cmd_record_skip_reverse();
	void    cmd_rewind();
	void    cmd_rewind_and_offline();
	void    read_transport_status();
	void    execute_command();

	required_address_space m_memory_space;

	devcb_write_line m_int_line;

	uint16_t m_w[8];

	tape_unit_t m_tape[MAX_TAPE_UNIT];
};

#endif // MAME_BUS_TI99X_990_TAP_H
