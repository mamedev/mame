// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    MSM6222B

    A somewhat hd44780-compatible LCD controller.

    The -01 variant has a fixed cgrom, the other variants are mask-programmed.

***************************************************************************/

#include "emu.h"
#include "msm6222b.h"

const device_type MSM6222B = &device_creator<msm6222b_device>;
const device_type MSM6222B_01 = &device_creator<msm6222b_01_device>;

ROM_START( msm6222b_01 )
	ROM_REGION( 0x1000, "cgrom", 0 )
	ROM_LOAD( "msm6222b-01.bin", 0x0000, 0x1000, CRC(8ffa8521) SHA1(e108b520e6d20459a7bbd5958bbfa1d551a690bd) )
ROM_END

msm6222b_device::msm6222b_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source), cursor_direction(false), cursor_blinking(false), two_line(false), shift_on_write(false), double_height(false), cursor_on(false), display_on(false), adc(0), shift(0), cgrom(nullptr)
{
}

msm6222b_device::msm6222b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MSM6222B, "msm6222b-xx", tag, owner, clock, "msm6222b", __FILE__), cursor_direction(false), cursor_blinking(false), two_line(false), shift_on_write(false), double_height(false), cursor_on(false), display_on(false), adc(0), shift(0), cgrom(nullptr)
{
}

msm6222b_01_device::msm6222b_01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	msm6222b_device(mconfig, MSM6222B_01, "msm6222b-01", tag, owner, clock, "msm6222b", __FILE__)
{
}

const rom_entry *msm6222b_01_device::device_rom_region() const
{
	return ROM_NAME(msm6222b_01);
}

void msm6222b_device::device_start()
{
	if(memregion("cgrom"))
		cgrom = memregion("cgrom")->base();
	else if(m_region)
		cgrom = m_region->base();
	else
		cgrom = nullptr;

	memset(cgram, 0, sizeof(cgram));
	memset(ddram, 0x20, sizeof(ddram));

	cursor_direction = true;
	cursor_blinking = false;
	display_on = false;
	two_line = false;
	cursor_on = false;
	shift_on_write = false;
	double_height = false;
	adc = 0x00;
	shift = 0;
}

void msm6222b_device::control_w(UINT8 data)
{
	int cmd;
	for(cmd = 7; cmd >= 0 && !(data & (1<<cmd)); cmd--);
	switch(cmd) {
	case 0:
		memset(ddram, 0x20, sizeof(ddram));
		adc = 0x00;
		break;

	case 1:
		adc = 0x00;
		shift = 0x00;
		break;
	case 2:
		shift_on_write = data & 1;
		cursor_direction = data & 2;
		break;

	case 3:
		display_on = data & 4;
		cursor_on = data & 2;
		cursor_blinking = data & 1;
		break;

	case 4:
		if(data & 8)
			shift_step(data & 4);
		else
			cursor_step(data & 4);
		break;

	case 5:
		two_line = data & 8;
		double_height = (data & 0xc) == 4;
		// Bit 4 is 4bits/8bits data access
		break;

	case 6:
		adc = data & 0x3f;
		break;

	case 7:
		adc = data; // Bit 7 is set
		break;
	}
}

UINT8 msm6222b_device::control_r()
{
	return adc & 0x7f;
}

void msm6222b_device::data_w(UINT8 data)
{
	if(adc & 0x80) {
		int adr = adc & 0x7f;
		if(two_line) {
			if((adr >= 40 && adr < 64) || adr >= 64+40)
				adr = -1;
			if(adr >= 64)
				adr += 40-64;
		} else {
			if(adr >= 80)
				adr = -1;
		}
		if(adr != -1) {
			ddram[adr] = data;
			if(shift_on_write)
				shift_step(cursor_direction);
			else
				cursor_step(cursor_direction);
		}
	} else {
		if(adc < 8*8) {
			cgram[adc] = data;
			cursor_step(cursor_direction);
		}
	}
}

void msm6222b_device::cursor_step(bool direction)
{
	if(direction) {
		if(adc & 0x80) {
			if(two_line && adc == (0x80|39))
				adc = 0x80|64;
			else if(two_line && adc == (0x80|(64+39)))
				adc = 0x80;
			else if((!two_line) && adc == (0x80|79))
				adc = 0x80;
			else
				adc++;
		} else {
			if(adc == 8*8-1)
				adc = 0x00;
			else
				adc++;
		}
	} else {
		if(adc & 0x80) {
			if(adc == 0x80)
				adc = two_line ? 0x80|(64+39) : 0x80|79;
			else if(two_line && adc == (0x80|64))
				adc = 0x80|39;
			else
				adc--;
		} else {
			if(adc == 0x00)
				adc = 8*8-1;
			else
				adc--;
		}
	}
}

void msm6222b_device::shift_step(bool direction)
{
	if(direction) {
		if(shift == 79)
			shift = 0;
		else
			shift++;
	} else {
		if(shift == 0)
			shift = 79;
		else
			shift--;
	}
}

bool msm6222b_device::blink_on() const
{
	if(!cursor_blinking)
		return false;
	UINT64 clocks = machine().time().as_ticks(250000);
	if(double_height)
		return clocks % 281600 >= 140800;
	else
		return clocks % 204800 >= 102400;
}

const UINT8 *msm6222b_device::render()
{
	memset(render_buf, 0, 80*16);
	if(!display_on)
		return render_buf;

	int char_height = double_height ? 11 : 8;

	for(int i=0; i<80; i++) {
		UINT8 c = ddram[(i+shift) % 80];
		if(c < 16)
			memcpy(render_buf + 16*i, double_height ? cgram + 8*(c & 6) : cgram + 8*(c & 7), char_height);
		else if(cgrom)
			memcpy(render_buf + 16*i, cgrom + 16*c, char_height);
	}

	if(cursor_on) {
		int cpos = adc & 0x7f;
		if(two_line) {
			if((cpos >= 40 && cpos < 64) || cpos >= 64+40)
				cpos = -1;
			else if(cpos >= 64)
				cpos += 40-64;
		} else {
			if(cpos >= 80)
				cpos = -1;
		}
		if(cpos != -1) {
			cpos = (cpos + shift) % 80;
			render_buf[cpos*16 + (double_height ? 10 : 7)] |= 0x1f;
			if(blink_on())
				for(int i=0; i<char_height; i++)
					render_buf[cpos*16 + i] ^= 0x1f;
		}
	}

	return render_buf;
}
