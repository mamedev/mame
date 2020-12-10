// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Slapstic decoding helper

**************************************************************************

    For more information on the slapstic, see slapstic.html, or go to
    http://www.aarongiles.com/slapstic.html

*************************************************************************/

#ifndef MAME_INCLUDES_SLAPSTIC_H
#define MAME_INCLUDES_SLAPSTIC_H

#pragma once


DECLARE_DEVICE_TYPE(SLAPSTIC, atari_slapstic_device)

/*************************************
 *
 *  Structure of slapstic params
 *
 *************************************/

struct mask_value
{
	int mask, value;
};


struct slapstic_data
{
	int bankstart;
	int bank[4];

	struct mask_value alt1;
	struct mask_value alt2;
	struct mask_value alt3;
	struct mask_value alt4;
	int altshift;

	struct mask_value bit1;
	struct mask_value bit2c0;
	struct mask_value bit2s0;
	struct mask_value bit2c1;
	struct mask_value bit2s1;
	struct mask_value bit3;

	struct mask_value add1;
	struct mask_value add2;
	struct mask_value addplus1;
	struct mask_value addplus2;
	struct mask_value add3;
};



/*************************************
 *
 *  Shorthand
 *
 *************************************/

#define UNKNOWN 0xffff
#define NO_BITWISE          \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN }
#define NO_ADDITIVE         \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN }

#define MATCHES_MASK_VALUE(val, maskval)    (((val) & (maskval).mask) == (maskval).value)



/*************************************
 *
 *  Constants
 *
 *************************************/

enum
{
	DISABLED,
	ENABLED,
	ALTERNATE1,
	ALTERNATE2,
	ALTERNATE3,
	BITWISE1,
	BITWISE2,
	BITWISE3,
	ADDITIVE1,
	ADDITIVE2,
	ADDITIVE3
};


class atari_slapstic_device :  public device_t
{
public:
	// construction/destruction
	atari_slapstic_device(const machine_config &mconfig, const char *tag, device_t *owner, int chipnum)
		: atari_slapstic_device(mconfig, tag, owner, u32(0))
	{
		m_chipnum = chipnum;
	}

	atari_slapstic_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_bank(T &&tag) { m_bank.set_tag(std::forward<T>(tag)); }

	int bank();
	int tweak(offs_t offset);

	void set_chipnum(int chipnum) { m_chipnum = chipnum; }

	int m_chipnum;

	u8 state;
	u8 current_bank;
	int access_68k;

	u8 alt_bank;
	u8 bit_bank;
	u8 add_bank;
	u8 bit_xor;

	struct slapstic_data slapstic;

	void slapstic_log(offs_t offset);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_validity_check(validity_checker &valid) const override;

private:
	optional_memory_bank m_bank;
};

#endif // MAME_INCLUDES_SLAPSTIC_H
