// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COM8116 Dual Baud Rate Generator (Programmable Divider) emulation

    COM5016 is a mostly-compatible clone of this chip, with +12V on
    pin 9 rather than NC.

**********************************************************************
                            _____   _____
             XTAL/EXT1   1 |*    \_/     | 18  XTAL/EXT2
                   +5V   2 |             | 17  fT
                    fR   3 |             | 16  Ta
                    Ra   4 |   COM8116   | 15  Tb
                    Rb   5 |   COM8116T  | 14  Tc
                    Rc   6 |   COM8136   | 13  Td
                    Rd   7 |   COM8136T  | 12  STT
                   STR   8 |             | 11  GND
                    NC   9 |_____________| 10  fX/4

**********************************************************************/

#ifndef MAME_MACHINE_COM8116_H
#define MAME_MACHINE_COM8116_H

#pragma once


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> com8116_device

class com8116_device :  public device_t
{
public:
	// construction/destruction
	com8116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto fx4_handler() { return m_fx4_handler.bind(); }
	auto fr_handler() { return m_fr_handler.bind(); }
	auto ft_handler() { return m_ft_handler.bind(); }

	void str_w(uint8_t data);
	void stt_w(uint8_t data);
	void str_stt_w(uint8_t data);
	void stt_str_w(uint8_t data);

protected:
	com8116_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const int *divisors);

	static const int divisors_16X_5_0688MHz[16];
	static const int divisors_16X_6_01835MHz[16];
	static const int divisors_16X_4_9152MHz[16];
	static const int divisors_32X_5_0688MHz[16];
	static const int divisors_16X_2_7648MHz[16];
	static const int divisors_16X_1_8432MHz[16];
	static const int divisors_16X_5_0688MHz_030[16];
	static const int divisors_16X_4_6080MHz[16];

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(fx4_tick);
	TIMER_CALLBACK_MEMBER(fr_tick);
	TIMER_CALLBACK_MEMBER(ft_tick);

private:
	devcb_write_line   m_fx4_handler;
	devcb_write_line   m_fr_handler;
	devcb_write_line   m_ft_handler;

	int m_fx4;
	int m_fr;
	int m_ft;

	const int *const m_divisors;

	// timers
	emu_timer *m_fx4_timer;
	emu_timer *m_fr_timer;
	emu_timer *m_ft_timer;
};


// ======================> com8116_003_device

class com8116_003_device : public com8116_device
{
public:
	// construction/destruction
	com8116_003_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> com5016_5_device

class com5016_5_device : public com8116_device
{
public:
	// construction/destruction
	com5016_5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> com5016_013_device

class com5016_013_device : public com8116_device
{
public:
	// construction/destruction
	com5016_013_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> com8116_020_device

class com8116_020_device : public com8116_device
{
public:
	// construction/destruction
	com8116_020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> com8116_003_device

class k1135ab_device : public com8116_device
{
public:
	// construction/destruction
	k1135ab_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(COM8116, com8116_device)
DECLARE_DEVICE_TYPE(COM8116_003, com8116_003_device)
DECLARE_DEVICE_TYPE(COM5016_5, com5016_5_device)
DECLARE_DEVICE_TYPE(COM5016_013, com5016_013_device)
DECLARE_DEVICE_TYPE(COM8116_020, com8116_020_device)
DECLARE_DEVICE_TYPE(K1135AB, k1135ab_device)

#endif // MAME_MACHINE_COM8116_H
