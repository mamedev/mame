// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese
/**************************************************************************************************

    Konami K054000 hitbox/math custom chip

    Sort of a protection device, used for collision detection.
    It is passed a few parameters, and returns a boolean telling if collision
    happened. It has no access to gfx data, it only does arithmetical operations
    on the parameters.

    Thunder Cross II POST checks of this chip.
    It literally tests the chip in a unit test fashion:
      1. zeroing all ports;
      2. test that status returns 0;
      3. ping ACX reg 0 with 0xff;
      4. test status = 1;
      5. ping BCX reg 0 with 0xff;
      6. test status = 0;
      7. ping ACX reg 1 with 0xff;
      8. test status = 1;
      9. rinse and repeat until all registers are exhausted.

    The fun part is that the game doesn't even access the chip at all during gameplay
    (or at least not until stage 6, where game disallows continues) while the specific
    "delta" registers are instead challenged by Vendetta OTG attacks (cfr. MT06393, MT07839).

**************************************************************************************************/

#include "emu.h"
#include "k054000.h"

#include "multibyte.h"

#define LOG_HITBOX (1 << 1)

#define VERBOSE 0
#include "logmacro.h"

#include <cstring>


DEFINE_DEVICE_TYPE(K054000, k054000_device, "k054000", "Konami 054000 Protection")

k054000_device::k054000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K054000, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k054000_device::device_start()
{
	save_item(NAME(m_raw_acx));
	save_item(NAME(m_raw_acy));
	save_item(NAME(m_raw_bcx));
	save_item(NAME(m_raw_bcy));
	save_item(NAME(m_acx));
	save_item(NAME(m_acy));
	save_item(NAME(m_aax));
	save_item(NAME(m_aay));
	save_item(NAME(m_bcx));
	save_item(NAME(m_bcy));
	save_item(NAME(m_bax));
	save_item(NAME(m_bay));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k054000_device::device_reset()
{
	// TODO: verify initial state (very unlikely to be all zeroes)
	std::fill(std::begin(m_raw_acx), std::end(m_raw_acx), 0);
	std::fill(std::begin(m_raw_acy), std::end(m_raw_acy), 0);
	std::fill(std::begin(m_raw_bcx), std::end(m_raw_bcx), 0);
	std::fill(std::begin(m_raw_bcy), std::end(m_raw_bcy), 0);
	m_aax = 1;
	m_aay = 1;
	m_bax = 1;
	m_bay = 1;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

/*
Memory map:
00      unused
01-03 W A center X
04    W A delta correction X
05      unused
06    W A semiaxis X
07    W A semiaxis Y
08      unused
09-0b W A center Y
0c    W A delta correction Y
0d      unused
0e    W B semiaxis X
0f    W B semiaxis Y
10      unused
11-13 W B center Y
14      unused
15-17 W B center X
18    R 0 = collision, 1 = no collision
*/
void k054000_device::map(address_map &map)
{
	map.unmap_value_low();
	map(0x01, 0x04).w(FUNC(k054000_device::acx_w));
	map(0x06, 0x06).lw8(NAME([this] (u8 data) { m_aax = data; }));
	map(0x07, 0x07).lw8(NAME([this] (u8 data) { m_aay = data; }));
	map(0x09, 0x0c).w(FUNC(k054000_device::acy_w));

	map(0x0e, 0x0e).lw8(NAME([this] (u8 data) { m_bax = data; }));
	map(0x0f, 0x0f).lw8(NAME([this] (u8 data) { m_bay = data; }));
	map(0x11, 0x13).w(FUNC(k054000_device::bcy_w));
	map(0x15, 0x17).w(FUNC(k054000_device::bcx_w));

	map(0x18, 0x18).r(FUNC(k054000_device::status_r));
}

inline s32 k054000_device::convert_raw_to_result_delta(u8 *buf)
{
	s32 res = get_u24be(buf);

	// Last value in the buffer is used as OTG correction in Vendetta
	if (buf[3] & 0x80)
		res -= (0x100 - buf[3]);
	else
		res += buf[3];

	return res;
}

void k054000_device::acx_w(offs_t offset, u8 data)
{
	m_raw_acx[offset] = data;
	m_acx = convert_raw_to_result_delta(m_raw_acx);
}

void k054000_device::acy_w(offs_t offset, u8 data)
{
	m_raw_acy[offset] = data;
	m_acy = convert_raw_to_result_delta(m_raw_acy);
}

void k054000_device::bcx_w(offs_t offset, u8 data)
{
	m_raw_bcx[offset] = data;
	m_bcx = get_u24be(m_raw_bcx);
}

void k054000_device::bcy_w(offs_t offset, u8 data)
{
	m_raw_bcy[offset] = data;
	m_bcy = get_u24be(m_raw_bcy);
}

u8 k054000_device::axis_check(u32 ac, u32 bc, u32 aa, u32 ba)
{
	u8 res = 0;
	const s32 sub = ac - bc;

	// MSB check
	if ((sub > 511) || (sub <= -1024))
		res |= 1;

	// LSB check
	if ((abs(sub) & 0x1ff) > ((aa + ba) & 0x1ff))
		res |= 1;

	return res;
}

u8 k054000_device::status_r()
{
	u8 res;

	res = axis_check(m_acx, m_bcx, m_aax, m_bax);
	res |= axis_check(m_acy, m_bcy, m_aay, m_bay);

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_HITBOX, print_hitbox_state(res));

	return res;
}

// debugging
std::string k054000_device::print_hitbox_state(bool result)
{
	std::ostringstream outbuffer;

	util::stream_format(outbuffer, "%s collision check:\n", machine().describe_context());
	util::stream_format(outbuffer, "ACX %02x%02x%02x%02x|", m_raw_acx[0], m_raw_acx[1], m_raw_acx[2], m_raw_acx[3]);
	util::stream_format(outbuffer, "ACY %02x%02x%02x%02x|", m_raw_acy[0], m_raw_acy[1], m_raw_acy[2], m_raw_acy[3]);
	util::stream_format(outbuffer, "AAX %02x AAY %02x\n", m_aax, m_aay);
	util::stream_format(outbuffer, "BCX %02x%02x%02x%02x|", m_raw_bcx[0], m_raw_bcx[1], m_raw_bcx[2], m_raw_bcx[3]);
	util::stream_format(outbuffer, "BCY %02x%02x%02x%02x|", m_raw_bcy[0], m_raw_bcy[1], m_raw_bcy[2], m_raw_bcy[3]);
	util::stream_format(outbuffer, "BAX %02x BAY %02x\n", m_bax, m_bay);
	util::stream_format(outbuffer, "Result: %d (%s)\n", result, result ? "no" : "yes");
	util::stream_format(outbuffer, "===\n");

	return outbuffer.str();
}

// old code, left as documentation reasons
#if 0
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
//  if (m_regs[0x04] == 0xff)
//      Acx+=3;
//  if (m_regs[0x0c] == 0xff)
//      Acy+=3;
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
	//printf("%d %d %d %d (%d|%d)|%d %d %d %d\n", Acx, Acy, Aax, Aay, m_regs[0x04], m_regs[0x0c], Bcx, Bcy, Bax, Bay);

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
