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
	DECLARE_WRITE_LINE_MEMBER(eop_w);
	template <unsigned Channel> DECLARE_WRITE_LINE_MEMBER(dreq_w);

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
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	void command(u8 data);
	void chain(unsigned const c);
	void operate(s32 param);
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
	u8 m_master_mode;
	u16 m_chain_control;
	u8 m_pointer;
	u16 m_temporary;

	// channels
	struct channel
	{
		channel(am9516_device &parent)
			: udc(parent)
			, flyby_byte_r(parent)
			, flyby_byte_w(parent)
			, flyby_word_r(parent)
			, flyby_word_w(parent)
		{}

		u32 address(u16 &aru, u16 &arl, int delta = 0);

		u8 read_byte(bool flip = false);
		void write_byte(u8 data, bool flip = false);
		u16 read_word(bool flip = false);
		void write_word(u16 data, bool flip = false);

		void interrupt(bool assert);

		am9516_device &udc;

		devcb_read8 flyby_byte_r;
		devcb_write8 flyby_byte_w;
		devcb_read16 flyby_word_r;
		devcb_write16 flyby_word_w;

		emu_timer *run = nullptr;

		u16 cabl = 0;    // current address b lower
		u16 babl = 0;    // base address b lower
		u16 caal = 0;    // current address a lower
		u16 baal = 0;    // base address a lower
		u16 cabu = 0;    // current address b upper
		u16 babu = 0;    // base address b upper
		u16 caau = 0;    // current address a upper
		u16 baau = 0;    // base address a upper
		u16 cal = 0;     // chain address lower
		u16 cau = 0;     // chain address upper
		u16 is = 0;      // interrupt save
		u16 status = 0;  // status
		u16 coc = 0;     // current operation count
		u16 boc = 0;     // base operation count
		u16 pattern = 0; // pattern
		u16 mask = 0;    // mask
		u16 cml = 0;     // channel mode low
		u16 cmh = 0;     // channel mode high
		u8 iv = 0;       // interrupt vector
	}
	m_channel[2];
};

DECLARE_DEVICE_TYPE(AM9516, am9516_device)

#endif // MAME_MACHINE_AM9516_H
