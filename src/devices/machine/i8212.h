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




///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_I8212_INT_CALLBACK(_write) \
	devcb = &downcast<i8212_device &>(*device).set_int_wr_callback(DEVCB_##_write);

#define MCFG_I8212_DI_CALLBACK(_read) \
	devcb = &downcast<i8212_device &>(*device).set_di_rd_callback(DEVCB_##_read);

#define MCFG_I8212_DO_CALLBACK(_write) \
	devcb = &downcast<i8212_device &>(*device).set_do_wr_callback(DEVCB_##_write);

#define MCFG_I8212_MD_CALLBACK(_read) \
	devcb = &downcast<i8212_device &>(*device).set_md_rd_callback(DEVCB_##_read);



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> i8212_device

class i8212_device : public device_t
{
	enum class mode : u8
	{
		INPUT,
		OUTPUT
	};

public:
	// construction/destruction
	i8212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_int_wr_callback(Object &&cb) { return m_write_int.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_di_rd_callback(Object &&cb) { return m_read_di.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_do_wr_callback(Object &&cb) { return m_write_do.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_md_rd_callback(Object &&cb) { return m_read_md.set_callback(std::forward<Object>(cb)); }

	// data read handlers
	DECLARE_READ8_MEMBER(read);
	IRQ_CALLBACK_MEMBER(inta_cb);

	// data write handlers
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_WRITE8_MEMBER(strobe);

	// line write handlers
	DECLARE_WRITE_LINE_MEMBER(md_w);
	DECLARE_WRITE_LINE_MEMBER(stb_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

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
