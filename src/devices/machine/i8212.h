// license:BSD-3-Clause
// copyright-holders:Curt Coder,AJR
/**********************************************************************

    Intel 8212/3212 8-Bit Input/Output Port (Multi-Mode Latch Buffer)

**********************************************************************
                            _____   _____
                  _DS1   1 |*    \_/     | 24  Vcc
                    MD   2 |             | 23  _INT
                   DI1   3 |             | 22  DI8
                   DO1   4 |             | 21  DO8
                   DI2   5 |             | 20  DI7
                   DO2   6 |    8212     | 19  DO7
                   DI3   7 |    3212     | 18  DI6
                   DO3   8 |             | 17  DO6
                   DI4   9 |             | 16  DI5
                   DO4  10 |             | 15  DO5
                   STB  11 |             | 14  _CLR
                   GND  12 |_____________| 13  DS2

**********************************************************************/

#ifndef MAME_MACHINE_I8212_H
#define MAME_MACHINE_I8212_H

#pragma once

class i8212_device : public device_t
{
	enum class mode : u8
	{
		INPUT,
		OUTPUT
	};

public:
	// construction/destruction
	i8212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto int_wr_callback() { return m_write_int.bind(); }
	auto di_rd_callback() { return m_read_di.bind(); }
	auto do_wr_callback() { return m_write_do.bind(); }
	auto md_rd_callback() { return m_read_md.bind(); }

	// data read handlers
	uint8_t read();
	IRQ_CALLBACK_MEMBER(inta_cb);

	// data write handlers
	void write(uint8_t data);
	void strobe(uint8_t data);

	// line write handlers
	void stb_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// helpers
	mode get_mode();

	devcb_write_line   m_write_int;
	devcb_read8        m_read_di;
	devcb_write8       m_write_do;
	devcb_read_line    m_read_md;

	int m_stb;                  // strobe
	uint8_t m_data;             // data latch
};


// device type definition
DECLARE_DEVICE_TYPE(I8212, i8212_device)

#endif // MAME_MACHINE_I8212_H
