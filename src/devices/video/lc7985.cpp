// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

        Sanyo LC7985NA/LC7985ND LCD controller

***************************************************************************/

#include "emu.h"
#include "lc7985.h"

DEFINE_DEVICE_TYPE(LC7985, lc7985_device, "lc7985", "Sanyo LC7985NA/LC7985ND LCD controller")


ROM_START( lc7985 )
  ROM_REGION( 0x1000, "cgrom", 0 )
  ROM_LOAD( "lc7985.bin", 0x0000, 0x1000,  BAD_DUMP CRC(fdc64160) SHA1(8e6b54f8fb7c4c15aab2e65dd1a44729b97423b1)) // from page 12 of the LC7985D datasheet
ROM_END

lc7985_device::lc7985_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LC7985, tag, owner, clock),
	  m_cgrom_region(*this, DEVICE_SELF)
{
}

const tiny_rom_entry *lc7985_device::device_rom_region() const
{
	return ROM_NAME(lc7985);
}

void lc7985_device::device_start()
{
	m_cgrom = m_cgrom_region.found() ? m_cgrom_region : memregion("cgrom")->base();
	m_busy_timer = timer_alloc(FUNC(lc7985_device::clear_busy_flag), this);

	save_item(NAME(m_ddram));
	save_item(NAME(m_cgram));
	save_item(NAME(m_busy_flag));
	save_item(NAME(m_ddac));
	save_item(NAME(m_cgac));
	save_item(NAME(m_shift));
	save_item(NAME(m_access_ddram));
	save_item(NAME(m_function));
	save_item(NAME(m_cds));
}

void lc7985_device::device_reset()
{
	memset(m_ddram, 0x20, sizeof(m_ddram)); // filled with SPACE char
	memset(m_cgram, 0, sizeof(m_cgram));
	m_ddac = 0x00;
	m_cgac = 0x00;
	m_shift = 0x00;
	m_access_ddram = false;
	m_function = 0x00;
	m_cds = 0x00;
	m_display = 0x00;
	m_entry = 0x02;

	busy(attotime::from_msec(10));
}

void lc7985_device::busy(attotime time)
{
	m_busy_flag = true;
	m_busy_timer->adjust(time);
}

TIMER_CALLBACK_MEMBER(lc7985_device::clear_busy_flag)
{
	m_busy_flag = false;
}

void lc7985_device::inc_ddac()
{
	if(m_function & 0x08) { // 2 lines
		if(m_ddac == 39)
			m_ddac = 64;
		else if(m_ddac == 64+39)
			m_ddac = 0;
		else
			m_ddac++;
	} else {
		if(m_ddac == 79)
			m_ddac = 0;
		else
			m_ddac++;
	}
}

void lc7985_device::dec_ddac()
{
	if(m_function & 0x08) { // 2 lines
		if(m_ddac == 64)
			m_ddac = 39;
		else if(m_ddac == 0)
			m_ddac = 64+39;
		else
			m_ddac--;
	} else {
		if(m_ddac == 0)
			m_ddac = 79;
		else
			m_ddac--;
	}
}

void lc7985_device::shift_left()
{
	if(m_shift == 79)
		m_shift = 0;
	else
		m_shift++;
}

void lc7985_device::shift_right()
{
	if(m_shift == 0)
		m_shift = 79;
	else
		m_shift--;
}

void lc7985_device::ir_w(u8 data)
{
	if(m_busy_flag)
		return;

	if(data & 0x80) {
		// Set DDRAM address
		m_ddac = data & 0x7f;
		m_access_ddram = true;
		busy(attotime::from_usec(40));

	} else if(data & 0x40) {
		// Set CGRAM address
		m_cgac = data & 0x3f;
		m_access_ddram = false;
		busy(attotime::from_usec(40));

	} else if(data & 0x20) {
		// Set Function
		m_function = data;
		busy(attotime::from_usec(40));

	} else if(data & 0x10) {
		// Cursor/Display Shift
		m_access_ddram = true;
		switch((data >> 2) & 3) {
		case 0: dec_ddac(); break;
		case 1: inc_ddac(); break;
		case 2: shift_left(); break;
		case 3: shift_right(); break;
		}

		busy(attotime::from_usec(40));

	} else if(data & 0x08) {
		// Display On/Off
		m_display = data;
		busy(attotime::from_usec(40));

	} else if(data & 0x04) {
		// Set Entry Mode
		m_entry = data;
		busy(attotime::from_usec(40));

	} else if(data & 0x02) {
		// Cursor home
		m_ddac = 0;
		m_shift = 0;
		m_access_ddram = true;
		busy(attotime::from_usec(16400));

	} else if(data & 0x01) {
		// Display clear
		memset(m_ddram, 0x20, sizeof(m_ddram));
		m_ddac = 0x00;
		m_entry |= 0x02;
		busy(attotime::from_usec(16400));
	}
}

u8 lc7985_device::status_r()
{
	return (m_access_ddram ? m_ddac : m_cgac) | (m_busy_flag ? 0x80 : 0x00);
}

void lc7985_device::dr_w(u8 data)
{
	if(m_access_ddram) {
		m_ddram[(m_function & 0x08) && m_ddac >= 64 ? m_ddac - (64-40) : m_ddac] = data;
		switch(m_entry & 0x03) {
		case 0: dec_ddac(); break;
		case 1: dec_ddac(); shift_right(); break;
		case 2: inc_ddac(); break;
		case 3: inc_ddac(); shift_left(); break;
		}

	} else {
		m_cgram[m_cgac] = data;
		if(m_entry & 0x02)
			m_cgac = (m_cgac + 1) & 0x3f;
		else
			m_cgac = (m_cgac - 1) & 0x3f;
	}
}

u8 lc7985_device::dr_r()
{
	u8 res;
	if(m_access_ddram) {
		res = m_ddram[(m_function & 0x08) && m_ddac >= 64 ? m_ddac - (64-40) : m_ddac];
		if(m_entry & 0x02)
			inc_ddac();
		else
			dec_ddac();

	} else {
		res = m_cgram[m_cgac];
		if(m_entry & 0x02)
			m_cgac = (m_cgac + 1) & 0x3f;
		else
			m_cgac = (m_cgac - 1) & 0x3f;
	}
	return res;
}

const u8 *lc7985_device::render()
{
	memset(m_render_buffer, 0, sizeof(m_render_buffer));
	if(!(m_display & 0x04))
		return m_render_buffer;

	if(m_function & 0x08) {
		for(int y = 0; y != 2; y++) {
			for(int x = 0; x != 40; x ++) {
				u8 c = m_ddram[((x + 80 - m_shift) % 40) + 40*y];
				const u8 *src = c < 32 ? m_cgram + 8*(c & 7) : m_cgrom + 16 * c;
				u8 *dest = m_render_buffer + 8 * y + 16 * x;
				for(int z = 0; z != 8; z ++)
					*dest++ = *src++ & 0x1f;
			}
		}
		if(m_display & 0x03) {
			int cx = ((m_ddac & 0x3f) + 80 - m_shift) % 80;
			u8 *dest = m_render_buffer + (m_ddac >= 0x40 ? 8 : 0) + 16*cx;
			if(m_display & 0x02)
				dest[7] = 0x1f;
			if(m_display & 0x01) {
				bool on = int(machine().time().as_double() / 0.409) & 1;
				if(on)
					for(int z = 0; z != 8; z ++)
						*dest++ = 0x1f;
			}
		}

	} else if(m_function & 0x04) {
		for(int x = 0; x != 80; x ++) {
			u8 c = m_ddram[(x + 80 - m_shift) % 80];
			const u8 *src = c < 32 ? m_cgram + 8*(c & 6) : m_cgrom + 16 * c;
			u8 *dest = m_render_buffer + 16 * x;
			for(int z = 0; z != 11; z ++)
				*dest++ = *src++ & 0x1f;
		}
		if(m_display & 0x03) {
			int cx = (m_ddac + 80 - m_shift) % 80;
			u8 *dest = m_render_buffer + 16*cx;
			if(m_display & 0x02)
				dest[10] = 0x1f;
			if(m_display & 0x01) {
				bool on = int(machine().time().as_double() / 0.409) & 1;
				if(on)
					for(int z = 0; z != 11; z ++)
						*dest++ = 0x1f;
			}
		}

	} else {
		for(int x = 0; x != 80; x ++) {
			u8 c = m_ddram[(x + 80 - m_shift) % 80];
			const u8 *src = c < 32 ? m_cgram + 8*(c & 7) : m_cgrom + 16 * c;
			u8 *dest = m_render_buffer + 16 * x;
			for(int z = 0; z != 8; z ++)
				*dest++ = *src++ & 0x1f;
		}
		if(m_display & 0x03) {
			int cx = (m_ddac + 80 - m_shift) % 80;
			u8 *dest = m_render_buffer + 16*cx;
			if(m_display & 0x02)
				dest[7] = 0x1f;
			if(m_display & 0x01) {
				bool on = int(machine().time().as_double() / 0.409) & 1;
				if(on)
					for(int z = 0; z != 8; z ++)
						*dest++ = 0x1f;
			}
		}
	}
	return m_render_buffer;
}

