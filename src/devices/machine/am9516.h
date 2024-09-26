// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_AM9516_H
#define MAME_MACHINE_AM9516_H

#pragma once

class am9516_device
	: public device_t
	, public device_memory_interface
{
public:
	enum address_reference : unsigned
	{
		SYSTEM_IO  = 0,
		SYSTEM_MEM = 1,
		NORMAL_IO  = 2,
		NORMAL_MEM = 3,
	};

	am9516_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// output lines
	auto out_int() { return m_int.bind(); }
	auto out_eop() { return m_eop.bind(); }

	// input lines
	void eop_w(int state);
	template <unsigned Channel> void dreq_w(int state);

	template <unsigned Channel> auto flyby_byte_r() { return m_channel[Channel].flyby_byte_r.bind(); }
	template <unsigned Channel> auto flyby_byte_w() { return m_channel[Channel].flyby_byte_w.bind(); }
	template <unsigned Channel> auto flyby_word_r() { return m_channel[Channel].flyby_word_r.bind(); }
	template <unsigned Channel> auto flyby_word_w() { return m_channel[Channel].flyby_word_w.bind(); }

	// register access
	u16 addr_r() { return m_pointer; }
	void addr_w(u16 data) { m_pointer = data; }
	u16 data_r();
	void data_w(u16 data);

	u16 acknowledge();

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	void command(u8 data);
	template <unsigned Channel> void operate(s32 param);
	void complete(unsigned const c, u16 status);
	void interrupt();

	address_space_config m_space_config[4];

	devcb_write_line m_int;
	devcb_write_line m_eop;

	// i/o line state
	bool m_int_state;
	bool m_eop_out_state;
	bool m_eop_in_state;

	// chip-level registers
	u8 m_mode;
	u8 m_pointer;
	u16 m_temporary;

	// channels
	struct channel
	{
		channel(am9516_device &parent)
			: udc(parent)
			, flyby_byte_r(parent, 0)
			, flyby_byte_w(parent)
			, flyby_word_r(parent, 0)
			, flyby_word_w(parent)
		{}

		u32 address(u16 &aru, u16 &arl, int delta = 0);

		u8 read_byte(unsigned &cycles, bool flip = false);
		void write_byte(u8 data, unsigned &cycles, bool flip = false);
		u16 read_word(unsigned &cycles, bool flip = false);
		void write_word(u16 data, unsigned &cycles, bool flip = false);

		void interrupt(bool assert);
		void chain();
		void reload();

		void log_mode(unsigned mask, bool high = false) const;
		void log_addr(unsigned mask, const char *const name, u16 aru, u16 arl) const;

		am9516_device &udc;

		devcb_read8 flyby_byte_r;
		devcb_write8 flyby_byte_w;
		devcb_read16 flyby_word_r;
		devcb_write16 flyby_word_w;

		emu_timer *run;

		u16 cabl;    // current address b lower
		u16 babl;    // base address b lower
		u16 caal;    // current address a lower
		u16 baal;    // base address a lower
		u16 cabu;    // current address b upper
		u16 babu;    // base address b upper
		u16 caau;    // current address a upper
		u16 baau;    // base address a upper
		u16 cal;     // chain address lower
		u16 cau;     // chain address upper
		u16 is;      // interrupt save
		u16 status;  // status
		u16 coc;     // current operation count
		u16 boc;     // base operation count
		u16 pattern; // pattern
		u16 mask;    // mask
		u16 cml;     // channel mode low
		u16 cmh;     // channel mode high
		u8 iv;       // interrupt vector

		unsigned const wait_states[4] = { 0, 1, 2, 4 };
	}
	m_channel[2];
};

DECLARE_DEVICE_TYPE(AM9516, am9516_device)

#endif // MAME_MACHINE_AM9516_H
