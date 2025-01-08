// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Gaelco CG-1V/GAE1 based games

    Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
    I/O ports)

***************************************************************************/

#include "emu.h"
#include "gaelco2.h"

#include "machine/eepromser.h"
#include "chd.h"


/***************************************************************************

    Split even/odd bytes from ROMs in 16 bit mode to different memory areas

***************************************************************************/

void gaelco2_state::ROM16_split_gfx(const char *src_reg, const char *dst_reg, int start, int length, int dest1, int dest2)
{
	/* get a pointer to the source data */
	u8 *src = (u8 *)memregion(src_reg)->base();

	/* get a pointer to the destination data */
	u8 *dst = (u8 *)memregion(dst_reg)->base();

	/* fill destination areas with the proper data */
	for (int i = 0; i < length/2; i++){
		dst[dest1 + i] = src[start + i*2 + 0];
		dst[dest2 + i] = src[start + i*2 + 1];
	}
}


/***************************************************************************

    Driver init routines

***************************************************************************/

void gaelco2_state::init_alighunt()
{
	/*
	For "gfx_temp" we have this memory map:
	    0x0000000-0x03fffff ROM u48
	    0x0400000-0x07fffff ROM u47
	    0x0800000-0x0bfffff ROM u50
	    0x0c00000-0x0ffffff ROM u49

	and we are going to construct this one for "gfx":
	    0x0000000-0x01fffff ROM u48 even bytes
	    0x0200000-0x03fffff ROM u47 even bytes
	    0x0400000-0x05fffff ROM u48 odd bytes
	    0x0600000-0x07fffff ROM u47 odd bytes
	    0x0800000-0x09fffff ROM u50 even bytes
	    0x0a00000-0x0bfffff ROM u49 even bytes
	    0x0c00000-0x0dfffff ROM u50 odd bytes
	    0x0e00000-0x0ffffff ROM u49 odd bytes
	*/

	/* split ROM u48 */
	ROM16_split_gfx("gfx_temp", "gfx", 0x0000000, 0x0400000, 0x0000000, 0x0400000);

	/* split ROM u47 */
	ROM16_split_gfx("gfx_temp", "gfx", 0x0400000, 0x0400000, 0x0200000, 0x0600000);

	/* split ROM u50 */
	ROM16_split_gfx("gfx_temp", "gfx", 0x0800000, 0x0400000, 0x0800000, 0x0c00000);

	/* split ROM u49 */
	ROM16_split_gfx("gfx_temp", "gfx", 0x0c00000, 0x0400000, 0x0a00000, 0x0e00000);
}


void gaelco2_state::init_touchgo()
{
	/*
	For "gfx_temp" we have this memory map:
	    0x0000000-0x03fffff ROM ic65
	    0x0400000-0x05fffff ROM ic66
	    0x0800000-0x0bfffff ROM ic67

	and we are going to construct this one for "gfx":
	    0x0000000-0x01fffff ROM ic65 even bytes
	    0x0200000-0x02fffff ROM ic66 even bytes
	    0x0400000-0x05fffff ROM ic65 odd bytes
	    0x0600000-0x06fffff ROM ic66 odd bytes
	    0x0800000-0x09fffff ROM ic67 even bytes
	    0x0c00000-0x0dfffff ROM ic67 odd bytes
	*/

	/* split ROM ic65 */
	ROM16_split_gfx("gfx_temp", "gfx", 0x0000000, 0x0400000, 0x0000000, 0x0400000);

	/* split ROM ic66 */
	ROM16_split_gfx("gfx_temp", "gfx", 0x0400000, 0x0200000, 0x0200000, 0x0600000);

	/* split ROM ic67 */
	ROM16_split_gfx("gfx_temp", "gfx", 0x0800000, 0x0400000, 0x0800000, 0x0c00000);
}


void snowboar_state::init_snowboara()
{
	/*
	For "gfx_temp" we have this memory map:
	    0x0000000-0x03fffff ROM sb44
	    0x0400000-0x07fffff ROM sb45
	    0x0800000-0x0bfffff ROM sb46

	and we are going to construct this one for "gfx":
	    0x0000000-0x01fffff ROM sb44 even bytes
	    0x0200000-0x03fffff ROM sb45 even bytes
	    0x0400000-0x05fffff ROM sb44 odd bytes
	    0x0600000-0x07fffff ROM sb45 odd bytes
	    0x0800000-0x09fffff ROM sb46 even bytes
	    0x0c00000-0x0dfffff ROM sb46 odd bytes
	*/

	/* split ROM sb44 */
	ROM16_split_gfx("gfx_temp", "gfx", 0x0000000, 0x0400000, 0x0000000, 0x0400000);

	/* split ROM sb45 */
	ROM16_split_gfx("gfx_temp", "gfx", 0x0400000, 0x0400000, 0x0200000, 0x0600000);

	/* split ROM sb46 */
	ROM16_split_gfx("gfx_temp", "gfx", 0x0800000, 0x0400000, 0x0800000, 0x0c00000);
}


void wrally2_state::init_wrally2()
{
	/*
	For "gfx_temp" we have this memory map:
	    0x0000000-0x03fffff ROM wr2_ic69.ic69
	    0x0400000-0x05fffff ROM wr2_ic70.ic70

	and we are going to construct this one for "gfx":
	    0x0000000-0x01fffff ROM wr2_ic69.ic69 even bytes
	    0x0200000-0x03fffff ROM wr2_ic69.ic69 odd bytes
	    0x0400000-0x04fffff ROM wr2_ic70.ic70 even bytes
	    0x0600000-0x06fffff ROM wr2_ic70.ic70 odd bytes
	*/

	/* split ROM wr2_ic69.ic69 */
	ROM16_split_gfx("gfx_temp", "gfx", 0x0000000, 0x0400000, 0x0000000, 0x0200000);

	/* split ROM wr2_ic70.ic70 */
	ROM16_split_gfx("gfx_temp", "gfx", 0x0400000, 0x0200000, 0x0400000, 0x0600000);
}


/***************************************************************************

    MCU communication

***************************************************************************/

void gaelco2_state::shareram_w(offs_t offset, u8 data)
{
	// why isn't there address map functionality for this?
	util::big_endian_cast<u8>(m_shareram.target())[offset] = data;
}

u8 gaelco2_state::shareram_r(offs_t offset)
{
	// why isn't there address map functionality for this?
	return util::big_endian_cast<u8 const>(m_shareram.target())[offset];
}


/***************************************************************************

    Coin counters/lockouts

***************************************************************************/

void gaelco2_state::wrally2_latch_w(offs_t offset, u16 data)
{
	m_mainlatch->write_bit(offset >> 2, BIT(data, 0));
}

void gaelco2_state::coin1_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void gaelco2_state::coin2_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

void gaelco2_state::coin3_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(2, state);
}

void gaelco2_state::coin4_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(3, state);
}

void gaelco2_state::alighunt_coin_w(u16 data)
{
	/* Coin Lockouts */
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 0));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 1));

	/* Coin Counters */
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 3));
}

/***************************************************************************

    Bang

***************************************************************************/

void bang_state::machine_start()
{
	gaelco2_state::machine_start();

	save_item(NAME(m_clr_gun_int));
}

void bang_state::bang_clr_gun_int_w(u16 data)
{
	m_clr_gun_int = true;
}

TIMER_DEVICE_CALLBACK_MEMBER(bang_state::bang_irq)
{
	int scanline = param;

	if (scanline == 256)
	{
		m_maincpu->set_input_line(2, HOLD_LINE);
		m_clr_gun_int = false;
	}

	if (((scanline % 64) == 0) && m_clr_gun_int)
		m_maincpu->set_input_line(4, HOLD_LINE);
}

/***************************************************************************

    Protection

***************************************************************************/

void snowboar_state::machine_start()
{
	gaelco2_state::machine_start();

	save_item(NAME(m_snowboard_latch));
}

static u16 get_lo(u32 x)
{
	return ((x & 0x00000010) <<  1) |
			((x & 0x00000800) <<  3) |
			((x & 0x40000000) >> 27) |
			((x & 0x00000005) <<  6) |
			((x & 0x00000008) <<  8) |
			rotl_32(x & 0x00800040, 9) |
			((x & 0x04000000) >> 16) |
			((x & 0x00008000) >> 14) |
			((x & 0x00002000) >> 11) |
			((x & 0x00020000) >> 10) |
			((x & 0x00100000) >>  8) |
			((x & 0x00044000) >>  5) |
			((x & 0x00000020) >>  1);
}

static u16 get_hi(u32 x)
{
	return ((x & 0x00001400) >>  0) |
			((x & 0x10000000) >> 26) |
			((x & 0x02000000) >> 24) |
			((x & 0x08000000) >> 21) |
			((x & 0x00000002) << 12) |
			((x & 0x01000000) >> 19) |
			((x & 0x20000000) >> 18) |
			((x & 0x80000000) >> 16) |
			((x & 0x00200000) >> 13) |
			((x & 0x00010000) >> 12) |
			((x & 0x00080000) >> 10) |
			((x & 0x00000200) >>  9) |
			((x & 0x00400000) >>  8) |
			((x & 0x00000080) >>  4) |
			((x & 0x00000100) >>  1);
}

static u16 get_out(u16 x)
{
	return ((x & 0xc840) <<  0) |
			((x & 0x0080) <<  2) |
			((x & 0x0004) <<  3) |
			((x & 0x0008) <<  5) |
			((x & 0x0010) <<  8) |
			((x & 0x0002) <<  9) |
			((x & 0x0001) << 13) |
			((x & 0x0200) >>  9) |
			((x & 0x1400) >>  8) |
			((x & 0x0100) >>  7) |
			((x & 0x2000) >>  6) |
			((x & 0x0020) >>  2);
}

u16 mangle(u32 x)
{
	u16 a = get_lo(x);
	u16 b = get_hi(x);
	return get_out(((a ^ 0x0010) - (b ^ 0x0024)) ^ 0x5496);
}

u16 snowboar_state::snowboar_protection_r()
{
	u16 ret = mangle(m_snowboard_latch);
	ret = ((ret & 0xff00) >> 8) | ((ret & 0x00ff) << 8);
	return ret;
}

void snowboar_state::snowboar_protection_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_snowboar_protection[offset]);

	m_snowboard_latch = (m_snowboard_latch << 16) | data;

	logerror("%06x: protection write %04x to %04x\n", m_maincpu->pc(), data, offset*2);
}
