// license:GPL-2.0+
// copyright-holders:Dirk Best, Vas Crabb
/*****************************************************************************
 *
 *  DL1416
 *
 * 4-Digit 16-Segment Alphanumeric Intelligent Display
 * with Memory/Decoder/Driver
 *
 * See video/dl1416.c for more info
 *
 ****************************************************************************/

#ifndef MAME_VIDEO_DL1416_H
#define MAME_VIDEO_DL1416_H

#pragma once


/***************************************************************************
    DEVICE TYPES
***************************************************************************/

DECLARE_DEVICE_TYPE(DL1414T, dl1414_device)
DECLARE_DEVICE_TYPE(DL1416B, dl1416_device)
DECLARE_DEVICE_TYPE(DL1416T, dl1416_device)


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DL1414_UPDATE_HANDLER(_devcb) \
	downcast<dl1414_device &>(*device).set_update_handler(DEVCB_##_devcb);

#define MCFG_DL1416_UPDATE_HANDLER(_devcb) \
	downcast<dl1414_device &>(*device).set_update_handler(DEVCB_##_devcb);


/***************************************************************************
    TYPE DECLARATIONS
***************************************************************************/

class dl1414_device : public device_t
{
public:
	template <typename Object> devcb_base &set_update_handler(Object &&cb) { return m_update_cb.set_callback(std::forward<Object>(cb)); }
	auto update() { return m_update_cb.bind(); }

	// signal-level interface
	DECLARE_WRITE_LINE_MEMBER(wr_w); // write strobe (rising edge)
	DECLARE_WRITE_LINE_MEMBER(ce_w); // chip enable (active low)
	void addr_w(uint8_t state);
	void data_w(uint8_t state);

	// bus interface - still requires cu_w to set cursor enable state
	virtual DECLARE_WRITE8_MEMBER(bus_w);

protected:
	dl1414_device(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void set_cursor_state(offs_t offset, bool state);
	virtual uint16_t translate(u8 digit, bool cursor) const = 0;

private:
	devcb_write16 m_update_cb;

	// internal state
	uint8_t m_digit_ram[4]; // holds the digit code for each position
	bool m_cursor_state[4]; // holds the cursor state for each position

	// input line state
	bool m_wr_in;
	bool m_ce_in, m_ce_latch;
	uint8_t m_addr_in, m_addr_latch;
	uint8_t m_data_in;
};

class dl1416_device : public dl1414_device
{
public:
	DECLARE_WRITE_LINE_MEMBER(cu_w); // cursor enable (active low)

protected:
	dl1416_device(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	bool cu_in() const { return m_cu_in; }

private:
	// input line state
	bool m_cu_in;
};

#endif // MAME_VIDEO_DL1416_H
