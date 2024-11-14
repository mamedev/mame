// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/

#ifndef MAME_SEGA_SEGAIC16_M_H
#define MAME_SEGA_SEGAIC16_M_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> sega_315_5248_multiplier_device

class sega_315_5248_multiplier_device : public device_t
{
public:
	// construction/destruction
	sega_315_5248_multiplier_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// public interface
	u16 read(offs_t offset);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	u16                      m_regs[4];
};


// ======================> sega_315_5249_divider_device

class sega_315_5249_divider_device : public device_t
{
public:
	// construction/destruction
	sega_315_5249_divider_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// public interface
	u16 read(offs_t offset);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal helpers
	void execute(int mode);

	// internal state
	u16                      m_regs[8];
};


// ======================> sega_315_5250_compare_timer_device

class sega_315_5250_compare_timer_device : public device_t
{
public:
	// construction/destruction
	sega_315_5250_compare_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto m68kint_callback() { return m_68kint_callback.bind(); }
	auto zint_callback() { return m_zint_callback.bind(); }

	// public interface
	void exck_w(int state);
	u16 read(offs_t offset);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 zread();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal helpers
	void execute(bool update_history = false);
	void interrupt_ack();
	TIMER_CALLBACK_MEMBER(write_to_sound);

	// configuration
	devcb_write_line         m_68kint_callback;
	devcb_write_line         m_zint_callback;

	// internal state
	u16                      m_regs[16];
	u16                      m_counter;
	u8                       m_bit;
	bool                     m_exck;
};


// device type definition
DECLARE_DEVICE_TYPE(SEGA_315_5248_MULTIPLIER,    sega_315_5248_multiplier_device)
DECLARE_DEVICE_TYPE(SEGA_315_5249_DIVIDER,       sega_315_5249_divider_device)
DECLARE_DEVICE_TYPE(SEGA_315_5250_COMPARE_TIMER, sega_315_5250_compare_timer_device)


#endif // MAME_SEGA_SEGAIC16_M_H
