// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes
/*****************************************************************************
 *
 * machine/upd7002.h
 *
 * uPD7002 Analogue to Digital Converter
 *
 * Driver by Gordon Jefferyes <mess_bbc@gjeffery.dircon.co.uk>
 *
 ****************************************************************************/

#ifndef MAME_MACHINE_UPD7002_H
#define MAME_MACHINE_UPD7002_H

#pragma once

/***************************************************************************
    MACROS
***************************************************************************/

class upd7002_device : public device_t
{
public:
	typedef device_delegate<int (int channel_number)> get_analogue_delegate;
	typedef device_delegate<void (int data)> eoc_delegate;

	upd7002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename... T> void set_get_analogue_callback(T &&... args) { m_get_analogue_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_eoc_callback(T &&... args) { m_eoc_cb.set(std::forward<T>(args)...); }

	int eoc_r();
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(conversion_complete);

private:
	// internal state

	/* Status Register
	    D0 and D1 define the currently selected input channel
	    D2 flag output
	    D3 0 = 8 bit mode   1 = 12 bit mode
	    D4 2nd MSB of conversion
	    D5     MSB of conversion
	    D6 0 = busy, 1 = not busy    (~busy)
	    D7 0 = conversion completed, 1 = conversion not completed  (~EOC)
	*/
	int m_status;

	/* High data byte
	    This byte contains the 8 most significant bits of the analogue to digital conversion. */
	int m_data1;

	/* Low data byte
	    In 12 bit mode: Bits 7 to 4 define the four low order bits of the conversion.
	    In  8 bit mode. All bits 7 to 4 are inaccurate.
	    Bits 3 to 0 are always set to low. */
	int m_data0;


	/* temporary store of the next A to D conversion */
	int m_digitalvalue;

	/* this counter is used to check a full end of conversion has been reached
	if the uPD7002 is half way through one conversion and a new conversion is requested
	the counter at the end of the first conversion will not match and not be processed
	only then at the end of the second conversion will the conversion complete function run */
	int m_conversion_counter;

	get_analogue_delegate m_get_analogue_cb;
	eoc_delegate          m_eoc_cb;

	emu_timer *m_conversion_timer;
};

DECLARE_DEVICE_TYPE(UPD7002, upd7002_device)

#endif // MAME_MACHINE_UPD7002_H
