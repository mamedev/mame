// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "k054000.h"


#define VERBOSE 0
#include "logmacro.h"


/***************************************************************************/
/*                                                                         */
/*                                 054000                                  */
/*                                                                         */
/***************************************************************************/

/*


054000
------
Sort of a protection device, used for collision detection.
It is passed a few parameters, and returns a boolean telling if collision
happened. It has no access to gfx data, it only does arithmetical operations
on the parameters.

Memory map:
00      unused
01-03 W A center X
04    W unknown, needed by thndrx2 to pass the startup check, we use a hack
05      unused
06    W A semiaxis X
07    W A semiaxis Y
08      unused
09-0b W A center Y
0c    W unknown, needed by thndrx2 to pass the startup check, we use a hack
0d      unused
0e    W B semiaxis X
0f    W B semiaxis Y
10      unused
11-13 W B center Y
14      unused
15-17 W B center X
18    R 0 = collision, 1 = no collision

*/


/***************************************************************************/
/*                                                                         */
/*                                 054000                                  */
/*                                                                         */
/***************************************************************************/

DEFINE_DEVICE_TYPE(K054000, k054000_device, "k054000", "K054000 Protection")

k054000_device::k054000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K054000, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k054000_device::device_start()
{
	save_item(NAME(m_Acx));
	save_item(NAME(m_Acy));
	save_item(NAME(m_Aax));
	save_item(NAME(m_Aay));
	save_item(NAME(m_Bcx));
	save_item(NAME(m_Bcy));
	save_item(NAME(m_Bax));
	save_item(NAME(m_Bay));
	save_pointer(NAME(m_raw_Acx), 4);
	save_pointer(NAME(m_raw_Acy), 4);
	save_pointer(NAME(m_raw_Bcx), 4);
	save_pointer(NAME(m_raw_Bcy), 4);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k054000_device::device_reset()
{
	// TODO: verify initial state (very unlikely to be all zeroes)
	std::fill(std::begin(m_raw_Acx), std::end(m_raw_Acx), 0);
	std::fill(std::begin(m_raw_Acy), std::end(m_raw_Acy), 0);
	std::fill(std::begin(m_raw_Bcx), std::end(m_raw_Bcx), 0);
	std::fill(std::begin(m_raw_Bcy), std::end(m_raw_Bcy), 0);
	m_Aax = 1;
	m_Aay = 1;
	m_Bax = 1;
	m_Bay = 1;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k054000_device::map(address_map &map)
{
	map.unmap_value_low();
	map(0x01, 0x04).w(FUNC(k054000_device::acx_w));
	map(0x06, 0x06).lw8(NAME([this] (u8 data) { m_Aax = data + 1; }));
	map(0x07, 0x07).lw8(NAME([this] (u8 data) { m_Aay = data + 1; }));
	map(0x09, 0x0c).w(FUNC(k054000_device::acy_w));

	map(0x0e, 0x0e).lw8(NAME([this] (u8 data) { m_Bax = data + 1; }));
	map(0x0f, 0x0f).lw8(NAME([this] (u8 data) { m_Bay = data + 1; }));
	map(0x11, 0x13).w(FUNC(k054000_device::bcy_w));
	map(0x15, 0x17).w(FUNC(k054000_device::bcx_w));

	map(0x18, 0x18).r(FUNC(k054000_device::status_r));
}

inline int k054000_device::convert_raw_to_result(u8 *buf)
{
	int res = (buf[0] << 16) | (buf[1] << 8) | buf[2];
	//if (buf[0] & 0x80)
	//	res = (0x1000000 - res);
	// last value in the buffer is used as OTG correction in Vendetta
	if (buf[3] & 0x80)
		res -= (0x100 - buf[3]);
	else
		res += buf[3];
	return res;
}

void k054000_device::acx_w(offs_t offset, u8 data)
{
	m_raw_Acx[offset] = data;
	m_Acx = convert_raw_to_result(m_raw_Acx);
}

void k054000_device::acy_w(offs_t offset, u8 data)
{
	m_raw_Acy[offset] = data;
	m_Acy = convert_raw_to_result(m_raw_Acy);
}

void k054000_device::bcx_w(offs_t offset, u8 data)
{
	m_raw_Bcx[offset] = data;
	m_Bcx = convert_raw_to_result(m_raw_Bcx);
}

void k054000_device::bcy_w(offs_t offset, u8 data)
{
	m_raw_Bcy[offset] = data;
	m_Bcy = convert_raw_to_result(m_raw_Bcy);
}

u8 k054000_device::status_r()
{
	u8 res = 0;
	
	if (m_Acx + m_Aax < m_Bcx - m_Bax)
		res |= 1;

	if (m_Bcx + m_Bax < m_Acx - m_Aax)
		res |= 1;

	if (m_Acy + m_Aay < m_Bcy - m_Bay)
		res |= 1;

	if (m_Bcy + m_Bay < m_Acy - m_Aay)
		res |= 1;

//	printf("%d %d %d %d (%d|%d)|%d %d %d %d == %d\n", m_Acx, m_Acy, m_Aax, m_Aay, m_raw_Acx[3], m_raw_Acy[3], m_Bcx, m_Bcy, m_Bax, m_Bay, res);
	printf("ACX %02x%02x%02x%02x|", m_raw_Acx[0], m_raw_Acx[1], m_raw_Acx[2], m_raw_Acx[3]);
	printf("ACY %02x%02x%02x%02x|", m_raw_Acy[0], m_raw_Acy[1], m_raw_Acy[2], m_raw_Acy[3]);
	printf("AAX %02x AAY %02x\n", m_Aax, m_Aay);
	printf("BCX %02x%02x%02x%02x|", m_raw_Bcx[0], m_raw_Bcx[1], m_raw_Bcx[2], m_raw_Bcx[3]);
	printf("BCY %02x%02x%02x%02x|", m_raw_Bcy[0], m_raw_Bcy[1], m_raw_Bcy[2], m_raw_Bcy[3]);
	printf("BAX %02x BAY %02x\n", m_Bax, m_Bay);
	printf("%d\n===\n", res);

	return res;
}

#ifdef UNUSED_FUNCTION
u8 k054000_device::read(offs_t offset)
{
	int Acx, Acy, Aax, Aay;
	int Bcx, Bcy, Bax, Bay;

	//logerror("%s: read 054000 address %02x\n", m_maincpu->pc(), offset);

	if (offset != 0x18)
		return 0;

	Acx = (m_regs[0x01] << 16) | (m_regs[0x02] << 8) | m_regs[0x03];
	Acy = (m_regs[0x09] << 16) | (m_regs[0x0a] << 8) | m_regs[0x0b];

	// TODO: this is a hack to make thndrx2 pass the startup check. It is certainly wrong.
//	if (m_regs[0x04] == 0xff)
//		Acx+=3;
//	if (m_regs[0x0c] == 0xff)
//		Acy+=3;
	// Used as OTG correction in Vendetta
	if (m_regs[0x04] & 0x80)
		Acx -= (0x100 - m_regs[0x04]);
	else
		Acx += m_regs[0x04];
	
	if (m_regs[0x0c] & 0x80)
		Acy -= (0x100 - m_regs[0x0c]);
	else
		Acy += m_regs[0x0c];

	Aax = m_regs[0x06] + 1;
	Aay = m_regs[0x07] + 1;

	Bcx = (m_regs[0x15] << 16) | (m_regs[0x16] << 8) | m_regs[0x17];
	Bcy = (m_regs[0x11] << 16) | (m_regs[0x12] << 8) | m_regs[0x13];
	Bax = m_regs[0x0e] + 1;
	Bay = m_regs[0x0f] + 1;

	//if (m_regs[0x04] || m_regs[0x0c])
	printf("%d %d %d %d (%d|%d)|%d %d %d %d\n", Acx, Acy, Aax, Aay, m_regs[0x04], m_regs[0x0c], Bcx, Bcy, Bax, Bay);

	if (Acx + Aax < Bcx - Bax)
		return 1;

	if (Bcx + Bax < Acx - Aax)
		return 1;

	if (Acy + Aay < Bcy - Bay)
		return 1;

	if (Bcy + Bay < Acy - Aay)
		return 1;

	return 0;
}
#endif
