// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "vt1682_alu.h"

#include "multibyte.h"

#define LOG_ALU     (1U << 1)

#define LOG_ALL     (LOG_ALU)

#define VERBOSE     (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(VT_VT1682_ALU, vrt_vt1682_alu_device, "vt1682alu", "VRT VT1682 ALU")

vrt_vt1682_alu_device::vrt_vt1682_alu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VT_VT1682_ALU, tag, owner, clock)
{
}

void vrt_vt1682_alu_device::device_start()
{
	save_item(NAME(m_alu_oprand));
}

void vrt_vt1682_alu_device::device_reset()
{
	for (int i=0;i<8;i++)
		m_alu_oprand[i] = 0;
}



/*
    Address 0x2130 WRITE (MAIN CPU & SOUND CPU)

    0x80 - ALU Oprand 1
    0x40 - ALU Oprand 1
    0x20 - ALU Oprand 1
    0x10 - ALU Oprand 1
    0x08 - ALU Oprand 1
    0x04 - ALU Oprand 1
    0x02 - ALU Oprand 1
    0x01 - ALU Oprand 1

    Address 0x2130 READ (MAIN CPU & SOUND CPU)

    0x80 - ALU Output 1
    0x40 - ALU Output 1
    0x20 - ALU Output 1
    0x10 - ALU Output 1
    0x08 - ALU Output 1
    0x04 - ALU Output 1
    0x02 - ALU Output 1
    0x01 - ALU Output 1
*/

uint8_t vrt_vt1682_alu_device::alu_out_1_r()
{
	uint8_t ret = m_alu_oprand[0];
	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "%s: alu_out_1_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vrt_vt1682_alu_device::alu_oprand_1_w(uint8_t data)
{
	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "%s: alu_oprand_1_w writing: %02x\n", machine().describe_context(), data);
	m_alu_oprand[0] = data;
}

/*
    Address 0x2131 WRITE (MAIN CPU & SOUND CPU)

    0x80 - ALU Oprand 2
    0x40 - ALU Oprand 2
    0x20 - ALU Oprand 2
    0x10 - ALU Oprand 2
    0x08 - ALU Oprand 2
    0x04 - ALU Oprand 2
    0x02 - ALU Oprand 2
    0x01 - ALU Oprand 2

    Address 0x2131 READ (MAIN CPU & SOUND CPU)

    0x80 - ALU Output 2
    0x40 - ALU Output 2
    0x20 - ALU Output 2
    0x10 - ALU Output 2
    0x08 - ALU Output 2
    0x04 - ALU Output 2
    0x02 - ALU Output 2
    0x01 - ALU Output 2
*/

uint8_t vrt_vt1682_alu_device::alu_out_2_r()
{
	uint8_t ret = m_alu_oprand[1];
	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "%s: alu_out_2_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vrt_vt1682_alu_device::alu_oprand_2_w(uint8_t data)
{
	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "%s: alu_oprand_2_w writing: %02x\n", machine().describe_context(), data);
	m_alu_oprand[1] = data;
}

/*
    Address 0x2132 WRITE (MAIN CPU & SOUND CPU)

    0x80 - ALU Oprand 3
    0x40 - ALU Oprand 3
    0x20 - ALU Oprand 3
    0x10 - ALU Oprand 3
    0x08 - ALU Oprand 3
    0x04 - ALU Oprand 3
    0x02 - ALU Oprand 3
    0x01 - ALU Oprand 3

    Address 0x2132 READ (MAIN CPU & SOUND CPU)

    0x80 - ALU Output 3
    0x40 - ALU Output 3
    0x20 - ALU Output 3
    0x10 - ALU Output 3
    0x08 - ALU Output 3
    0x04 - ALU Output 3
    0x02 - ALU Output 3
    0x01 - ALU Output 3
*/

uint8_t vrt_vt1682_alu_device::alu_out_3_r()
{
	uint8_t ret = m_alu_oprand[2];
	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "%s: alu_out_3_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vrt_vt1682_alu_device::alu_oprand_3_w(uint8_t data)
{
	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "%s: alu_oprand_3_w writing: %02x\n", machine().describe_context(), data);
	m_alu_oprand[2] = data;
}


/*
    Address 0x2133 WRITE (MAIN CPU & SOUND CPU)

    0x80 - ALU Oprand 4
    0x40 - ALU Oprand 4
    0x20 - ALU Oprand 4
    0x10 - ALU Oprand 4
    0x08 - ALU Oprand 4
    0x04 - ALU Oprand 4
    0x02 - ALU Oprand 4
    0x01 - ALU Oprand 4

    Address 0x2133 READ (MAIN CPU & SOUND CPU)

    0x80 - ALU Output 4
    0x40 - ALU Output 4
    0x20 - ALU Output 4
    0x10 - ALU Output 4
    0x08 - ALU Output 4
    0x04 - ALU Output 4
    0x02 - ALU Output 4
    0x01 - ALU Output 4
*/

uint8_t vrt_vt1682_alu_device::alu_out_4_r()
{
	uint8_t ret = m_alu_oprand[3];
	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "%s: alu_out_4_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}


void vrt_vt1682_alu_device::alu_oprand_4_w(uint8_t data)
{
	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "%s: alu_oprand_4_w writing: %02x\n", machine().describe_context(), data);
	m_alu_oprand[3] = data;
}

/*
    Address 0x2134 WRITE (MAIN CPU & SOUND CPU)

    0x80 - ALU Mul Oprand 5
    0x40 - ALU Mul Oprand 5
    0x20 - ALU Mul Oprand 5
    0x10 - ALU Mul Oprand 5
    0x08 - ALU Mul Oprand 5
    0x04 - ALU Mul Oprand 5
    0x02 - ALU Mul Oprand 5
    0x01 - ALU Mul Oprand 5

    Address 0x2134 READ (MAIN CPU & SOUND CPU)

    0x80 - ALU Output 5
    0x40 - ALU Output 5
    0x20 - ALU Output 5
    0x10 - ALU Output 5
    0x08 - ALU Output 5
    0x04 - ALU Output 5
    0x02 - ALU Output 5
    0x01 - ALU Output 5
*/

uint8_t vrt_vt1682_alu_device::alu_out_5_r()
{
	uint8_t ret = m_alu_oprand[4];
	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "%s: alu_out_5_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}


void vrt_vt1682_alu_device::alu_oprand_5_mult_w(uint8_t data)
{
	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "%s: alu_oprand_5_mult_w writing: %02x\n", machine().describe_context(), data);
	m_alu_oprand[4] = data;
}


/*
    Address 0x2135 WRITE (MAIN CPU & SOUND CPU)

    0x80 - ALU Mul Oprand 6
    0x40 - ALU Mul Oprand 6
    0x20 - ALU Mul Oprand 6
    0x10 - ALU Mul Oprand 6
    0x08 - ALU Mul Oprand 6
    0x04 - ALU Mul Oprand 6
    0x02 - ALU Mul Oprand 6
    0x01 - ALU Mul Oprand 6

    Address 0x2135 READ (MAIN CPU & SOUND CPU)

    0x80 - ALU Output 6
    0x40 - ALU Output 6
    0x20 - ALU Output 6
    0x10 - ALU Output 6
    0x08 - ALU Output 6
    0x04 - ALU Output 6
    0x02 - ALU Output 6
    0x01 - ALU Output 6
*/

uint8_t vrt_vt1682_alu_device::alu_out_6_r()
{
	uint8_t ret = m_alu_oprand[5];
	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "%s: alu_out_6_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vrt_vt1682_alu_device::alu_oprand_6_mult_w(uint8_t data)
{
	// used one of the 32in1 menus

	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "%s: alu_oprand_6_mult_w writing: %02x\n", machine().describe_context(), data);
	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "------------------------------------------ MULTIPLICATION REQUESTED ------------------------------------\n");
	m_alu_oprand[5] = data;

	int param1 = get_u16le(&m_alu_oprand[4]);
	int param2 = get_u16le(&m_alu_oprand[0]);

	uint32_t result = param1 * param2;

	put_u32le(&m_alu_oprand[0], result);

	// lxts3 (Sunnyside Race) reads result from 4/5 instead of 0/1
	put_u16le(&m_alu_oprand[4], result & 0xffff);
}


/*
    Address 0x2136 WRITE ONLY (MAIN CPU & SOUND CPU)

    0x80 - ALU Div Oprand 5
    0x40 - ALU Div Oprand 5
    0x20 - ALU Div Oprand 5
    0x10 - ALU Div Oprand 5
    0x08 - ALU Div Oprand 5
    0x04 - ALU Div Oprand 5
    0x02 - ALU Div Oprand 5
    0x01 - ALU Div Oprand 5

*/

void vrt_vt1682_alu_device::alu_oprand_5_div_w(uint8_t data)
{
	if (!m_is_sound_alu) LOGMASKED(LOG_ALU, "%s: alu_oprand_5_div_w writing: %02x\n", machine().describe_context(), data);
	m_alu_oprand[6] = data;
}

/*
    Address 0x2137 WRITE ONLY (MAIN CPU & SOUND CPU)

    0x80 - ALU Div Oprand 6
    0x40 - ALU Div Oprand 6
    0x20 - ALU Div Oprand 6
    0x10 - ALU Div Oprand 6
    0x08 - ALU Div Oprand 6
    0x04 - ALU Div Oprand 6
    0x02 - ALU Div Oprand 6
    0x01 - ALU Div Oprand 6
*/

void vrt_vt1682_alu_device::alu_oprand_6_div_w(uint8_t data)
{
	//LOGMASKED(LOG_ALU, "%s: alu_oprand_6_div_w writing: %02x\n", machine().describe_context(), data);
	m_alu_oprand[7] = data;

	uint32_t param1 = get_u32le(&m_alu_oprand[0]);
	uint16_t param2 = get_u16le(&m_alu_oprand[6]);

	if (param2 != 0)
	{
		//if (!m_is_sound_alu) popmessage("------------------------------------------ DIVISION REQUESTED ------------------------------------\n");

		uint32_t quotient = param1 / param2;
		uint16_t remainder = param1 - param2 * (quotient & ~uint32_t(1));

		put_u32le(&m_alu_oprand[0], quotient);
		put_u16le(&m_alu_oprand[4], (remainder >> 1) | (remainder << 15));
	}
}
