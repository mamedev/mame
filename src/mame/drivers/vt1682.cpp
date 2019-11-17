// license:BSD-3-Clause
// copyright-holders:David Haywood

/*  VT1682 - NOT compatible with NES, different video system, sound CPU (4x
             main CPU clock), optional internal ROM etc.

    Internal ROM can be mapped to Main CPU, or Sound CPU at 0x3000-0x3fff if used
    can also be configured as boot device
*/

#include "emu.h"
#include "machine/m6502_vt1682.h"
#include "screen.h"

class vt_vt1682_state : public driver_device
{
public:
	vt_vt1682_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void vt_vt1682(machine_config& config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);
	void vt_vt1682_map(address_map& map);


	/* Video */
	uint8_t m_2000;

	DECLARE_READ8_MEMBER(vt1682_2000_r);
	DECLARE_WRITE8_MEMBER(vt1682_2000_w);


	/* System */
	uint8_t m_prgbank1_r0;
	uint8_t m_prgbank1_r1;
	uint8_t m_210c_prgbank1_r2;
	uint8_t m_2100_prgbank1_r3;
	uint8_t m_2118_prgbank1_r4_r5;

	uint8_t m_2107_prgbank0_r0;
	uint8_t m_2108_prgbank0_r1;
	uint8_t m_2109_prgbank0_r2;
	uint8_t m_210a_prgbank0_r3;
	uint8_t m_prgbank0_r4;
	uint8_t m_prgbank0_r5;

	uint8_t m_210b_misc_cs_prg0_bank_sel;

	uint8_t m_2105_vt1682_2105_comr6_tvmodes;
	uint8_t m_211c_regs_ext2421;

	DECLARE_READ8_MEMBER(vt1682_2100_prgbank1_r3_r);
	DECLARE_WRITE8_MEMBER(vt1682_2100_prgbank1_r3_w);
	DECLARE_READ8_MEMBER(vt1682_210c_prgbank1_r2_r);
	DECLARE_WRITE8_MEMBER(vt1682_210c_prgbank1_r2_w);

	DECLARE_READ8_MEMBER(vt1682_2107_prgbank0_r0_r);
	DECLARE_WRITE8_MEMBER(vt1682_2107_prgbank0_r0_w);
	DECLARE_READ8_MEMBER(vt1682_2108_prgbank0_r1_r);
	DECLARE_WRITE8_MEMBER(vt1682_2108_prgbank0_r1_w);
	DECLARE_READ8_MEMBER(vt1682_2109_prgbank0_r2_r);
	DECLARE_WRITE8_MEMBER(vt1682_2109_prgbank0_r2_w);
	DECLARE_READ8_MEMBER(vt1682_210a_prgbank0_r3_r);
	DECLARE_WRITE8_MEMBER(vt1682_210a_prgbank0_r3_w);

	DECLARE_READ8_MEMBER(vt1682_prgbank0_r4_r);
	DECLARE_READ8_MEMBER(vt1682_prgbank0_r5_r);
	DECLARE_READ8_MEMBER(vt1682_prgbank1_r0_r);
	DECLARE_READ8_MEMBER(vt1682_prgbank1_r1_r);

	DECLARE_WRITE8_MEMBER(vt1682_prgbank1_r0_w);
	DECLARE_WRITE8_MEMBER(vt1682_prgbank1_r1_w);
	DECLARE_WRITE8_MEMBER(vt1682_prgbank0_r4_w);
	DECLARE_WRITE8_MEMBER(vt1682_prgbank0_r5_w);

	DECLARE_READ8_MEMBER(vt1682_2118_prgbank1_r4_r5_r);
	DECLARE_WRITE8_MEMBER(vt1682_2118_prgbank1_r4_r5_w);

	DECLARE_READ8_MEMBER(vt1682_210b_misc_cs_prg0_bank_sel_r);
	DECLARE_WRITE8_MEMBER(vt1682_210b_misc_cs_prg0_bank_sel_w);

	DECLARE_WRITE8_MEMBER(vt1682_2105_comr6_tvmodes_w);

	DECLARE_WRITE8_MEMBER(vt1682_211c_regs_ext2421_w);

	/* Support */

	void update_banks();
	uint8_t translate_prg0select(uint8_t tp20_tp13);
	uint32_t translate_address_4000_to_7fff(uint16_t address);
	uint32_t translate_address_8000_to_ffff(uint16_t address);

};

void vt_vt1682_state::machine_start()
{
	/* Video */
	save_item(NAME(m_2000));

	/* System */

	save_item(NAME(m_prgbank1_r0));
	save_item(NAME(m_prgbank1_r1));
	save_item(NAME(m_210c_prgbank1_r2));
	save_item(NAME(m_2100_prgbank1_r3));
	save_item(NAME(m_2118_prgbank1_r4_r5));

	save_item(NAME(m_2107_prgbank0_r0));
	save_item(NAME(m_2108_prgbank0_r1));
	save_item(NAME(m_2109_prgbank0_r2));
	save_item(NAME(m_210a_prgbank0_r3));
	save_item(NAME(m_prgbank0_r4));
	save_item(NAME(m_prgbank0_r5));

	save_item(NAME(m_210b_misc_cs_prg0_bank_sel));
	save_item(NAME(m_2105_vt1682_2105_comr6_tvmodes));
	save_item(NAME(m_211c_regs_ext2421));
}

void vt_vt1682_state::machine_reset()
{
	/* Video */
	m_2000 = 0;

	/* System */
	m_prgbank1_r0 = 0;
	m_prgbank1_r1 = 0;
	m_210c_prgbank1_r2 = 0;
	m_2100_prgbank1_r3 = 0;
	m_2118_prgbank1_r4_r5 = 0;

	m_2107_prgbank0_r0 = 0;
	m_2108_prgbank0_r1 = 0;
	m_2109_prgbank0_r2 = 0;
	m_210a_prgbank0_r3 = 0;
	m_prgbank0_r4 = 0;
	m_prgbank0_r5 = 0;

	m_210b_misc_cs_prg0_bank_sel = 0;
	m_2105_vt1682_2105_comr6_tvmodes = 0;
	m_211c_regs_ext2421 = 0;

	update_banks();
}

/*

Address tranlsation

----------------------------------------------------------------

First table uses bits from PB0r0, PB0r1, PB0r2 (0x8000 and above) or PB0r4, PB0r5 (below 0x8000)

PB0r0 = Program Bank 0 Register 0
PB0r1 = Program Bank 0 Register 1
PB0r2 = Program Bank 0 Register 2

PB0r4 = Program Bank 0 Register 4
PB0r5 = Program Bank 0 Register 5

PQ2EN   COMR6   A:15    A:14    A:13    |   TP:20   TP:19   TP:18   TP:17   TP:16   TP:15   TP:14   TP:13
-----------------------------------------------------------------------------------------------------------
0       0       1       0       0       |   PB0r0:7 PB0r0:6 PB0r0:5 PB0r0:4 PB0r0:3 PB0r0:2 PB0r0:1 PB0r0:0   (all PB0r0)
0       0       1       0       1       |   PB0r1:7 PB0r1:6 PB0r1:5 PB0r1:4 PB0r1:3 PB0r1:2 PB0r1:1 PB0r1:0   (all PB0r1)
0       0       1       1       0       |   1       1       1       1       1       1       1       0
0       0       1       1       1       |   1       1       1       1       1       1       1       1
0       1       1       0       0       |   1       1       1       1       1       1       1       0
0       1       1       0       1       |   PB0r1:7 PB0r1:6 PB0r1:5 PB0r1:4 PB0r1:3 PB0r1:2 PB0r1:1 PB0r1:0   (all PB0r1)
0       1       1       1       0       |   PB0r0:7 PB0r0:6 PB0r0:5 PB0r0:4 PB0r0:3 PB0r0:2 PB0r0:1 PB0r0:0   (all PB0r0)
0       1       1       1       1       |   1       1       1       1       1       1       1       1
1       0       1       0       0       |   PB0r0:7 PB0r0:6 PB0r0:5 PB0r0:4 PB0r0:3 PB0r0:2 PB0r0:1 PB0r0:0   (all PB0r0)
1       0       1       0       1       |   PB0r1:7 PB0r1:6 PB0r1:5 PB0r1:4 PB0r1:3 PB0r1:2 PB0r1:1 PB0r1:0   (all PB0r1)
1       0       1       1       0       |   PB0r2:7 PB0r2:6 PB0r2:5 PB0r2:4 PB0r2:3 PB0r2:2 PB0r2:1 PB0r2:0   (all PB0r2)
1       0       1       1       1       |   1       1       1       1       1       1       1       1
1       1       1       0       0       |   PB0r2:7 PB0r2:6 PB0r2:5 PB0r2:4 PB0r2:3 PB0r2:2 PB0r2:1 PB0r2:0   (all PB0r2)
1       1       1       0       1       |   PB0r1:7 PB0r1:6 PB0r1:5 PB0r1:4 PB0r1:3 PB0r1:2 PB0r1:1 PB0r1:0   (all PB0r1)
1       1       1       1       0       |   PB0r0:7 PB0r0:6 PB0r0:5 PB0r0:4 PB0r0:3 PB0r0:2 PB0r0:1 PB0r0:0   (all PB0r0)
1       1       1       1       1       |   1       1       1       1       1       1       1       1
-----------------------------------------------------------------------------------------------------------
-       -       0       1       1       |   PB0r5:7 PB0r5:6 PB0r5:5 PB0r5:4 PB0r5:3 PB0r5:2 PB0r5:1 PB0r5:0   (all PB0r5)
-       -       0       1       0       |   PB0r4:7 PB0r4:6 PB0r4:5 PB0r4:4 PB0r4:3 PB0r4:2 PB0r4:1 PB0r4:0   (all PB0r4)

----------------------------------------------------------------

second table uses bits from above, and PB0r3

Program Bank 0 Select   |   PA:20   PA:19   PA:18   PA:17   PA:16   PA:15   PA:14   PA:13
-------------------------------------------------------------------------------------------
0       0       0       |   PB0r3:7 PB0r3:6 TP:18   TP:17   TP:16   TP:15   TP:14   TP:13
0       0       1       |   PB0r3:7 PB0r3:6 PB0r3:5 TP:17   TP:16   TP:15   TP:14   TP:13
0       1       0       |   PB0r3:7 PB0r3:6 PB0r3:5 PB0r3:4 TP:16   TP:15   TP:14   TP:13
0       1       1       |   PB0r3:7 PB0r3:6 PB0r3:5 PB0r3:4 PB0r3:3 TP:15   TP:14   TP:13
1       0       0       |   PB0r3:7 PB0r3:6 PB0r3:5 PB0r3:4 PB0r3:3 PB0r3:2 TP:14   TP:13
1       0       1       |   PB0r3:7 PB0r3:6 PB0r3:5 PB0r3:4 PB0r3:3 PB0r3:2 PB0r3:1 TP:13
1       1       0       |   PB0r3:7 PB0r3:6 PB0r3:5 PB0r3:4 PB0r3:3 PB0r3:2 PB0r3:1 PB0r3:0
1       1       1       |   TP:20   TP:19   TP:18   TP:17   TP:16   TP:15   TP:14   TP:13

PB0r3 = Program Bank 0 Register 3
TP = Address translated by 1st table

----------------------------------------------------------------

third table uses bits from PB1r0, PB1r1, PB1r2, PB1r3 (0x8000 and above) or PB1r4, PB1r5 (below 0x8000)

PB1r0 = Program Bank 1 Register 0
PB1r1 = Program Bank 1 Register 1
PB1r2 = Program Bank 1 Register 2
PB1r3 = Program Bank 1 Register 3

PB1r4 = Program Bank 1 Register 4
PB1r5 = Program Bank 1 Register 5

EXT2421 PQ2EN   COMR6   A:15    A:14    A:13    |   PA:24   PA:23   PA:22   PA:21
------------------------------------------------------------------------------------
1       -       -       1       -       -       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
------------------------------------------------------------------------------------
0       0       0       1       0       0       |   PB1r0:3 PB1r0:2 PB1r0:1 PB1r0:0    (all PB1r0)
0       0       0       1       0       1       |   PB1r1:3 PB1r1:2 PB1r1:1 PB1r1:0    (all PB1r1)
0       0       0       1       1       0       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
0       0       0       1       1       1       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
0       0       1       1       0       0       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
0       0       1       1       0       1       |   PB1r1:3 PB1r1:2 PB1r1:1 PB1r1:0    (all PB1r1)
0       0       1       1       1       0       |   PB1r0:3 PB1r0:2 PB1r0:1 PB1r0:0    (all PB1r0)
0       0       1       1       1       1       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
0       1       0       1       0       0       |   PB1r0:3 PB1r0:2 PB1r0:1 PB1r0:0    (all PB1r0)
0       1       0       1       0       1       |   PB1r1:3 PB1r1:2 PB1r1:1 PB1r1:0    (all PB1r1)
0       1       0       1       1       0       |   PB1r2:3 PB1r2:2 PB1r2:1 PB1r2:0    (all PB1r2)
0       1       0       1       1       1       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
0       1       1       1       0       0       |   PB1r2:3 PB1r2:2 PB1r2:1 PB1r2:0    (all PB1r2)
0       1       1       1       0       1       |   PB1r1:3 PB1r1:2 PB1r1:1 PB1r1:0    (all PB1r1)
0       1       1       1       1       0       |   PB1r0:3 PB1r0:2 PB1r0:1 PB1r0:0    (all PB1r0)
0       1       1       1       1       1       |   PB1r3:3 PB1r3:2 PB1r3:1 PB1r3:0    (all PB1r3)
------------------------------------------------------------------------------------
-       -       -       0       1       1       |   PB1r5:3 PB1r5:2 PB1r5:1 PB1r5:0    (all PB1r5)
-       -       -       0       1       0       |   PB1r4:3 PB1r4:2 PB1r4:1 PB1r4:0    (all PB1r4)


*/
void vt_vt1682_state::update_banks()
{
	/* must use

	m_prgbank1_r0
	m_prgbank1_r1
	m_210c_prgbank1_r2
	m_2100_prgbank1_r3
	m_2118_prgbank1_r4_r5

	m_2107_prgbank0_r0
	m_2108_prgbank0_r1
	m_2109_prgbank0_r2
	m_210a_prgbank0_r3
	m_prgbank0_r4
	m_prgbank0_r5

	m_2105_vt1682_2105_comr6_tvmodes
	m_211c_regs_ext2421
	m_210b_misc_cs_prg0_bank_sel

	everything that changes these calls here, so if we wanted to do this with actual
	banks then here would be the place

	*/
}

uint8_t vt_vt1682_state::translate_prg0select(uint8_t tp20_tp13)
{
	uint8_t bank = m_210b_misc_cs_prg0_bank_sel & 0x07;

	uint8_t ret = 0x00;

	switch (bank)
	{
	case 0x0: ret = (m_210a_prgbank0_r3 & 0xc0) | (tp20_tp13 & 0x3f); break;
	case 0x1: ret = (m_210a_prgbank0_r3 & 0xe0) | (tp20_tp13 & 0x1f); break;
	case 0x2: ret = (m_210a_prgbank0_r3 & 0xf0) | (tp20_tp13 & 0x0f); break;
	case 0x3: ret = (m_210a_prgbank0_r3 & 0xf8) | (tp20_tp13 & 0x07); break;
	case 0x4: ret = (m_210a_prgbank0_r3 & 0xfc) | (tp20_tp13 & 0x03); break;
	case 0x5: ret = (m_210a_prgbank0_r3 & 0xfe) | (tp20_tp13 & 0x01); break;
	case 0x6: ret = m_210a_prgbank0_r3; break;
	case 0x7: ret = tp20_tp13;  break;
	}

	return ret;
}

uint32_t vt_vt1682_state::translate_address_4000_to_7fff(uint16_t address)
{
	uint32_t realaddress = 0x00000000;

	uint8_t prgbank1_r4 = (m_2118_prgbank1_r4_r5 & 0x0f);
	uint8_t prgbank1_r5 = (m_2118_prgbank1_r4_r5 & 0xf0)>>4;

	int tp20_tp13 = 0;
	int pa24_pa21 = 0;

	switch (address & 0x6000)
	{
	case 0x4000:
		tp20_tp13 = m_prgbank0_r4;
		pa24_pa21 = prgbank1_r4;
		break;

	case 0x6000:
		tp20_tp13 = m_prgbank0_r5;
		pa24_pa21 = prgbank1_r5;
		break;

	// invalid cases
	default:
	case 0x0000:
	case 0x2000:
		break;

	}

	int pa20_pa13 = translate_prg0select(tp20_tp13);

	realaddress = address & 0x1fff;
	realaddress |= pa20_pa13 << 13;
	realaddress |= pa24_pa21 << 21;

	return realaddress;
}

uint32_t vt_vt1682_state::translate_address_8000_to_ffff(uint16_t address)
{
	uint32_t realaddress = 0x00000000;

	int pq2en = (m_210b_misc_cs_prg0_bank_sel & 0x40)>>6;
	int comr6 = (m_2105_vt1682_2105_comr6_tvmodes & 0x40)>>6;
	int a14_a13 = (address & 0x6000) >> 13;

	int tp20_tp13 = 0;
	int lookup = a14_a13 | (comr6 << 2) | (pq2en << 3);

	switch (lookup)
	{
	case 0x0: tp20_tp13 = m_2107_prgbank0_r0; break;
	case 0x1: tp20_tp13 = m_2108_prgbank0_r1; break;
	case 0x2: tp20_tp13 = 0xfe; break;
	case 0x3: tp20_tp13 = 0xff; break;
	case 0x4: tp20_tp13 = 0xfe; break;
	case 0x5: tp20_tp13 = m_2108_prgbank0_r1; break;
	case 0x6: tp20_tp13 = m_2107_prgbank0_r0; break;
	case 0x7: tp20_tp13 = 0xff; break;
	case 0x8: tp20_tp13 = m_2107_prgbank0_r0; break;
	case 0x9: tp20_tp13 = m_2108_prgbank0_r1; break;
	case 0xa: tp20_tp13 = m_2109_prgbank0_r2; break;
	case 0xb: tp20_tp13 = 0xff; break;
	case 0xc: tp20_tp13 = m_2109_prgbank0_r2; break;
	case 0xd: tp20_tp13 = m_2108_prgbank0_r1; break;
	case 0xe: tp20_tp13 = m_2107_prgbank0_r0; break;
	case 0xf: tp20_tp13 = 0xff; break;
	}

	int pa20_pa13 = translate_prg0select(tp20_tp13);

	int pa24_pa21 = 0;

	int ext2421 = (m_211c_regs_ext2421 & 0x20) >> 5;

	if (ext2421)
	{
		pa24_pa21 = m_2100_prgbank1_r3;
	}
	else
	{
		switch (lookup)
		{
		case 0x0: pa24_pa21 = m_prgbank1_r0; break;
		case 0x1: pa24_pa21 = m_prgbank1_r1; break;
		case 0x2: pa24_pa21 = m_2100_prgbank1_r3; break;
		case 0x3: pa24_pa21 = m_2100_prgbank1_r3;  break;
		case 0x4: pa24_pa21 = m_2100_prgbank1_r3; break;
		case 0x5: pa24_pa21 = m_prgbank1_r1; break;
		case 0x6: pa24_pa21 = m_prgbank1_r0; break;
		case 0x7: pa24_pa21 = m_2100_prgbank1_r3; break;
		case 0x8: pa24_pa21 = m_prgbank1_r0; break;
		case 0x9: pa24_pa21 = m_prgbank1_r1; break;
		case 0xa: pa24_pa21 = m_210c_prgbank1_r2; break;
		case 0xb: pa24_pa21 = m_2100_prgbank1_r3; break;
		case 0xc: pa24_pa21 = m_210c_prgbank1_r2; break;
		case 0xd: pa24_pa21 = m_prgbank1_r1; break;
		case 0xe: pa24_pa21 = m_prgbank1_r0; break;
		case 0xf: pa24_pa21 = m_2100_prgbank1_r3; break;
		}
	}

	realaddress = address & 0x1fff;
	realaddress |= pa20_pa13 << 13;
	realaddress |= pa24_pa21 << 21;

	return realaddress;
}



/************************************************************************************************************************************
 VT1682 PPU Registers
************************************************************************************************************************************/

/*
    Address 0x2000 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - Capture
    0x08 - SLAVE
    0x04 - (unused)
    0x02 - (unused)
    0x01 - NMI_EN
*/

READ8_MEMBER(vt_vt1682_state::vt1682_2000_r)
{
	uint8_t ret = m_2000;
	logerror("%s: vt1682_2000_r offset: %02x returning: %02x\n", machine().describe_context(), offset, ret);
	return ret;
}

WRITE8_MEMBER(vt_vt1682_state::vt1682_2000_w)
{
	logerror("%s: vt1682_2000_w offset: %02x writing: %02x\n", machine().describe_context(), offset, data);
	m_2000 = data;
}

/*
    Address 0x2001 READ (MAIN CPU)

    0x80 - VBLANK
    0x40 - SP ERR
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - (unused)
    0x01 - (unused)

    Address 0x2001 WRITE (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - EXT CLK DIV
    0x04 - EXT CLK DIV
    0x02 - SP INI
    0x01 - BK INI
*/


/*
    Address 0x2002 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - SPRAM ADDR:2
    0x02 - SPRAM ADDR:1
    0x01 - SPRAM ADDR:0
*/

/*
    Address 0x2003 r/w (MAIN CPU)

    0x80 - SPRAM ADDR:10
    0x40 - SPRAM ADDR:9
    0x20 - SPRAM ADDR:8
    0x10 - SPRAM ADDR:7
    0x08 - SPRAM ADDR:6
    0x04 - SPRAM ADDR:5
    0x02 - SPRAM ADDR:4
    0x01 - SPRAM ADDR:3
*/

/*
    Address 0x2004 r/w (MAIN CPU)

    0x80 - SPRAM DATA:7
    0x40 - SPRAM DATA:6
    0x20 - SPRAM DATA:5
    0x10 - SPRAM DATA:4
    0x08 - SPRAM DATA:3
    0x04 - SPRAM DATA:2
    0x02 - SPRAM DATA:1
    0x01 - SPRAM DATA:0
*/

/*
    Address 0x2005 r/w (MAIN CPU)

    0x80 - VRAM ADDR:7
    0x40 - VRAM ADDR:6
    0x20 - VRAM ADDR:5
    0x10 - VRAM ADDR:4
    0x08 - VRAM ADDR:3
    0x04 - VRAM ADDR:2
    0x02 - VRAM ADDR:1
    0x01 - VRAM ADDR:0
*/

/*
    Address 0x2006 r/w (MAIN CPU)

    0x80 - VRAM ADDR:15
    0x40 - VRAM ADDR:14
    0x20 - VRAM ADDR:13
    0x10 - VRAM ADDR:12
    0x08 - VRAM ADDR:11
    0x04 - VRAM ADDR:10
    0x02 - VRAM ADDR:9
    0x01 - VRAM ADDR:8
*/

/*
    Address 0x2007 r/w (MAIN CPU)

    0x80 - VRAM DATA:7
    0x40 - VRAM DATA:6
    0x20 - VRAM DATA:5
    0x10 - VRAM DATA:4
    0x08 - VRAM DATA:3
    0x04 - VRAM DATA:2
    0x02 - VRAM DATA:1
    0x01 - VRAM DATA:0
*/

/*
    Address 0x2008 r/w (MAIN CPU)

    0x80 - LCD VS DELAY
    0x40 - LCD VS DELAY
    0x20 - LCD VS DELAY
    0x10 - LCD VS DELAY
    0x08 - LCD VS DELAY
    0x04 - LCD VS DELAY
    0x02 - LCD VS DELAY
    0x01 - LCD VS DELAY
*/

/*
    Address 0x2009 r/w (MAIN CPU)

    0x80 - LCD HS DELAY
    0x40 - LCD HS DELAY
    0x20 - LCD HS DELAY
    0x10 - LCD HS DELAY
    0x08 - LCD HS DELAY
    0x04 - LCD HS DELAY
    0x02 - LCD HS DELAY
    0x01 - LCD HS DELAY
*/

/*
    Address 0x200a r/w (MAIN CPU)

    0x80 - LCD FR DELAY:7
    0x40 - LCD FR DELAY:6
    0x20 - LCD FR DELAY:5
    0x10 - LCD FR DELAY:4
    0x08 - LCD FR DELAY:3
    0x04 - LCD FR DELAY:2
    0x02 - LCD FR DELAY:1
    0x01 - LCD FR DELAY:0
*/

/*
    Address 0x200b r/w (MAIN CPU)

    0x80 - CH2 Odd Line Colour
    0x40 - CH2 Odd Line Colour
    0x20 - CH2 Even Line Colour
    0x10 - CH2 Even Line Colour
    0x08 - CH2 SEL
    0x04 - CH2 REV
    0x02 - LCD FR:8
    0x01 - LCD HS:8
*/

/*
    Address 0x200c r/w (MAIN CPU)

    0x80 - FRate
    0x40 - DotODR
    0x20 - LCD CLOCK
    0x10 - LCD CLOCK
    0x08 - UPS 052
    0x04 - Field AC
    0x02 - LCD MODE
    0x01 - LCD MODE
*/

/*
    Address 0x200d r/w (MAIN CPU)

    0x80 - LCD ENABLE
    0x40 - Dot 240
    0x20 - Reverse
    0x10 - Vcom
    0x08 - Odd Line Color
    0x04 - Odd Line Color
    0x02 - Even Line Color
    0x01 - Even Line Color
*/

/*
    Address 0x200e r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - Blend2
    0x10 - Blend1
    0x08 - Palette 2 Out Sel
    0x04 - Palette 2 Out Sel
    0x02 - Palette 1 Out Sel
    0x01 - Palette 1 Out Sel
*/

/*
    Address 0x200f r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Bk2 Palette Select
    0x04 - Bk2 Palette Select
    0x02 - Bk1 Palette Select
    0x01 - Bk1 Palette Select
*/

/*
    Address 0x2010 r/w (MAIN CPU)

    0x80 - BK1X:7
    0x40 - BK1X:6
    0x20 - BK1X:5
    0x10 - BK1X:4
    0x08 - BK1X:3
    0x04 - BK1X:2
    0x02 - BK1X:1
    0x01 - BK1X:0
*/

/*
    Address 0x2011 r/w (MAIN CPU)

    0x80 - BK1Y:7
    0x40 - BK1Y:6
    0x20 - BK1Y:5
    0x10 - BK1Y:4
    0x08 - BK1Y:3
    0x04 - BK1Y:2
    0x02 - BK1Y:1
    0x01 - BK1Y:0
*/

/*
    Address 0x2012 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - BK1 HCLR
    0x08 - BK1 Scroll Enable
    0x04 - BK1 Scroll Enable
    0x02 - BK1Y:8
    0x01 - BK1X:8
*/

/*
    Address 0x2013 r/w (MAIN CPU)

    0x80 - BK1 Enable
    0x40 - BK1 Palette
    0x20 - BK1 Depth
    0x10 - BK1 Depth
    0x08 - BK1 Colour
    0x04 - BK1 Colour
    0x02 - BK1 Line
    0x01 - BK1 Size
*/

/*
    Address 0x2014 r/w (MAIN CPU)

    0x80 - BK2X:7
    0x40 - BK2X:6
    0x20 - BK2X:5
    0x10 - BK2X:4
    0x08 - BK2X:3
    0x04 - BK2X:2
    0x02 - BK2X:1
    0x01 - BK2X:0
*/

/*
    Address 0x2015 r/w (MAIN CPU)

    0x80 - BK2Y:7
    0x40 - BK2Y:6
    0x20 - BK2Y:5
    0x10 - BK2Y:4
    0x08 - BK2Y:3
    0x04 - BK2Y:2
    0x02 - BK2Y:1
    0x01 - BK2Y:0
*/

/*
    Address 0x2016 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - BK2 Scroll Enable
    0x04 - BK2 Scroll Enable
    0x02 - BK2Y:8
    0x01 - BK2X:8
*/

/*
    Address 0x2017 r/w (MAIN CPU)

    0x80 - BK2 Enable
    0x40 - BK2 Palette
    0x20 - BK2 Depth
    0x10 - BK2 Depth
    0x08 - BK2 Colour
    0x04 - BK2 Colour
    0x02 - (unused)
    0x01 - BK2 Size
*/

/*
    Address 0x2018 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - SPALSEL
    0x04 - SP ENABLE
    0x02 - SP SIZE
    0x01 - SP SIZE
*/

/*
    Address 0x2019 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - BK2 Gain
    0x04 - BK2 Gain
    0x02 - BK1 Gain
    0x01 - BK1 Gain
*/

/*
    Address 0x201a r/w (MAIN CPU)

    0x80 - SP SEGMENT:7
    0x40 - SP SEGMENT:6
    0x20 - SP SEGMENT:5
    0x10 - SP SEGMENT:4
    0x08 - SP SEGMENT:3
    0x04 - SP SEGMENT:2
    0x02 - SP SEGMENT:1
    0x01 - SP SEGMENT:0
*/

/*
    Address 0x201b r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - SP SEGMENT:11
    0x04 - SP SEGMENT:10
    0x02 - SP SEGMENT:9
    0x01 - SP SEGMENT:8
*/

/*
    Address 0x201c r/w (MAIN CPU)

    0x80 - BK1 SEGMENT:7
    0x40 - BK1 SEGMENT:6
    0x20 - BK1 SEGMENT:5
    0x10 - BK1 SEGMENT:4
    0x08 - BK1 SEGMENT:3
    0x04 - BK1 SEGMENT:2
    0x02 - BK1 SEGMENT:1
    0x01 - BK1 SEGMENT:0
*/

/*
    Address 0x201d r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - BK1 SEGMENT:11
    0x04 - BK1 SEGMENT:10
    0x02 - BK1 SEGMENT:9
    0x01 - BK1 SEGMENT:8
*/

/*
    Address 0x201e r/w (MAIN CPU)

    0x80 - BK2 SEGMENT:7
    0x40 - BK2 SEGMENT:6
    0x20 - BK2 SEGMENT:5
    0x10 - BK2 SEGMENT:4
    0x08 - BK2 SEGMENT:3
    0x04 - BK2 SEGMENT:2
    0x02 - BK2 SEGMENT:1
    0x01 - BK2 SEGMENT:0
*/

/*
    Address 0x201f r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - BK2 SEGMENT:11
    0x04 - BK2 SEGMENT:10
    0x02 - BK2 SEGMENT:9
    0x01 - BK2 SEGMENT:8
*/


/*
    Address 0x2020 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - BK2 L EN
    0x10 - BK1 L EN
    0x08 - Scroll Bank
    0x04 - Scroll Bank
    0x02 - Scroll Bank
    0x01 - Scroll Bank
*/

/*
    Address 0x2021 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - Luminance_offset
    0x10 - Luminance_offset
    0x08 - Luminance_offset
    0x04 - Luminance_offset
    0x02 - Luminance_offset
    0x01 - Luminance_offset
*/

/*
    Address 0x2022 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - VCOMIO
    0x10 - RGB DAC
    0x08 - CCIR Out
    0x04 - Saturation
    0x02 - Saturation
    0x01 - Saturation
*/

/*
    Address 0x2023 r/w (MAIN CPU)

    0x80 - Light Gun Reset
    0x40 - Light Gun Reset
    0x20 - Light Gun Reset
    0x10 - Light Gun Reset
    0x08 - Light Gun Reset
    0x04 - Light Gun Reset
    0x02 - Light Gun Reset
    0x01 - Light Gun Reset
*/

/*
    Address 0x2024 r/w (MAIN CPU)

    0x80 - Light Gun 1 Y
    0x40 - Light Gun 1 Y
    0x20 - Light Gun 1 Y
    0x10 - Light Gun 1 Y
    0x08 - Light Gun 1 Y
    0x04 - Light Gun 1 Y
    0x02 - Light Gun 1 Y
    0x01 - Light Gun 1 Y
*/

/*
    Address 0x2025 r/w (MAIN CPU)

    0x80 - Light Gun 1 X
    0x40 - Light Gun 1 X
    0x20 - Light Gun 1 X
    0x10 - Light Gun 1 X
    0x08 - Light Gun 1 X
    0x04 - Light Gun 1 X
    0x02 - Light Gun 1 X
    0x01 - Light Gun 1 X
*/

/*
    Address 0x2026 r/w (MAIN CPU)

    0x80 - Light Gun 2 Y
    0x40 - Light Gun 2 Y
    0x20 - Light Gun 2 Y
    0x10 - Light Gun 2 Y
    0x08 - Light Gun 2 Y
    0x04 - Light Gun 2 Y
    0x02 - Light Gun 2 Y
    0x01 - Light Gun 2 Y
*/

/*
    Address 0x2027 r/w (MAIN CPU)

    0x80 - Light Gun 2 X
    0x40 - Light Gun 2 X
    0x20 - Light Gun 2 X
    0x10 - Light Gun 2 X
    0x08 - Light Gun 2 X
    0x04 - Light Gun 2 X
    0x02 - Light Gun 2 X
    0x01 - Light Gun 2 X
*/

/*
    Address 0x2028 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - CCIR Y
    0x10 - CCIR Y
    0x08 - CCIR Y
    0x04 - CCIR Y
    0x02 - CCIR Y
    0x01 - CCIR Y
*/

/*
    Address 0x2029 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - CCIR X
    0x08 - CCIR X
    0x04 - CCIR X
    0x02 - CCIR X
    0x01 - CCIR X
*/

/*
    Address 0x202a r/w (MAIN CPU)

    0x80 - VS Phase
    0x40 - HS Phase
    0x20 - YC Swap
    0x10 - CbCr Swap
    0x08 - SyncMod
    0x04 - YUV_RGB
    0x02 - Field O En
    0x01 - Field On
*/

/*
    Address 0x202b r/w (MAIN CPU)

    0x80 - R En
    0x40 - G En
    0x20 - B En
    0x10 - Halftone
    0x08 - B/W
    0x04 - CCIR Depth
    0x02 - CCIR Depth
    0x01 - CCIR Depth
*/

/* Address 0x202c Unused */
/* Address 0x202d Unused */

/*
    Address 0x202e r/w (MAIN CPU)

    0x80 - TRC EN
    0x40 - CCIR EN
    0x20 - Bluescreen EN
    0x10 - Touch EN
    0x08 - CCIR TH
    0x04 - CCIR TH
    0x02 - CCIR TH
    0x01 - CCIR TH
*/

/* Address 0x202f Unused */

/*
    Address 0x2030 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - VDACSW
    0x20 - VDACOUT:5
    0x10 - VDACOUT:4
    0x08 - VDACOUT:3
    0x04 - VDACOUT:2
    0x02 - VDACOUT:1
    0x01 - VDACOUT:0
*/

/*
    Address 0x2031 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - R DAC SW
    0x10 - R DAC OUT:4
    0x08 - R DAC OUT:3
    0x04 - R DAC OUT:2
    0x02 - R DAC OUT:1
    0x01 - R DAC OUT:0
*/

/*
    Address 0x2032 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - G DAC SW
    0x10 - G DAC OUT:4
    0x08 - G DAC OUT:3
    0x04 - G DAC OUT:2
    0x02 - G DAC OUT:1
    0x01 - G DAC OUT:0
*/

/*
    Address 0x2033 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - B DAC SW
    0x10 - B DAC OUT:4
    0x08 - B DAC OUT:3
    0x04 - B DAC OUT:2
    0x02 - B DAC OUT:1
    0x01 - B DAC OUT:0
*/

/************************************************************************************************************************************
 VT1682 Sys Registers
************************************************************************************************************************************/

/*
    Address 0x2100 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 3
    0x04 - Program Bank 1 Register 3
    0x02 - Program Bank 1 Register 3
    0x01 - Program Bank 1 Register 3
*/

READ8_MEMBER(vt_vt1682_state::vt1682_2100_prgbank1_r3_r)
{
	uint8_t ret = m_2100_prgbank1_r3;
	logerror("%s: vt1682_2100_prgbank1_r3_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vt_vt1682_state::vt1682_2100_prgbank1_r3_w)
{
	logerror("%s: vt1682_2100_prgbank1_r3_w writing: %02x (4-bits)\n", machine().describe_context(), data);
	m_2100_prgbank1_r3 = data;
	update_banks();
}


/*
    Address 0x2101 r/w (MAIN CPU)

    0x80 - Timer Preload:7
    0x40 - Timer Preload:6
    0x20 - Timer Preload:5
    0x10 - Timer Preload:4
    0x08 - Timer Preload:3
    0x04 - Timer Preload:2
    0x02 - Timer Preload:1
    0x01 - Timer Preload:0
*/

/*
    Address 0x2102 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - TMR_IRQ
    0x01 - TMR_EN
*/

/*
    Address 0x2103 r/w (MAIN CPU)

    0x80 - Timer IRQ Clear
    0x40 - Timer IRQ Clear
    0x20 - Timer IRQ Clear
    0x10 - Timer IRQ Clear
    0x08 - Timer IRQ Clear
    0x04 - Timer IRQ Clear
    0x02 - Timer IRQ Clear
    0x01 - Timer IRQ Clear
*/

/*
    Address 0x2104 r/w (MAIN CPU)

    0x80 - Timer Preload:15
    0x40 - Timer Preload:14
    0x20 - Timer Preload:13
    0x10 - Timer Preload:12
    0x08 - Timer Preload:11
    0x04 - Timer Preload:10
    0x02 - Timer Preload:9
    0x01 - Timer Preload:8
*/

/*
    Address 0x2105 WRITE ONLY (MAIN CPU)

    0x80 - (unused)
    0x40 - COMR6
    0x20 - TV SYS SE:1
    0x10 - TV SYS SE:0
    0x08 - CCIR SEL
    0x04 - Double
    0x02 - ROM SEL
    0x01 - PRAM
*/

WRITE8_MEMBER(vt_vt1682_state::vt1682_2105_comr6_tvmodes_w)
{
	// COMR6 is used for banking
	logerror("%s: vt1682_2105_comr6_tvmodes_w writing: %02x\n", machine().describe_context(), data);
	m_2105_vt1682_2105_comr6_tvmodes = data;
	update_banks();
}


/*
    Address 0x2106 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - SCPURN
    0x10 - SCPU ON
    0x08 - SPI ON
    0x04 - UART ON
    0x02 - TV ON
    0x01 - LCD ON
*/

/*
    Address 0x2107 r/w (MAIN CPU)

    0x80 - Program Bank 0 Register 0
    0x40 - Program Bank 0 Register 0
    0x20 - Program Bank 0 Register 0
    0x10 - Program Bank 0 Register 0
    0x08 - Program Bank 0 Register 0
    0x04 - Program Bank 0 Register 0
    0x02 - Program Bank 0 Register 0
    0x01 - Program Bank 0 Register 0
*/

READ8_MEMBER(vt_vt1682_state::vt1682_2107_prgbank0_r0_r)
{
	uint8_t ret = m_2107_prgbank0_r0;
	logerror("%s: vt1682_2107_prgbank0_r0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vt_vt1682_state::vt1682_2107_prgbank0_r0_w)
{
	logerror("%s: vt1682_2107_prgbank0_r0_w writing: %02x\n", machine().describe_context(), data);
	m_2107_prgbank0_r0 = data;
	update_banks();
}

/*
    Address 0x2108 r/w (MAIN CPU)

    0x80 - Program Bank 0 Register 1
    0x40 - Program Bank 0 Register 1
    0x20 - Program Bank 0 Register 1
    0x10 - Program Bank 0 Register 1
    0x08 - Program Bank 0 Register 1
    0x04 - Program Bank 0 Register 1
    0x02 - Program Bank 0 Register 1
    0x01 - Program Bank 0 Register 1
*/

READ8_MEMBER(vt_vt1682_state::vt1682_2108_prgbank0_r1_r)
{
	uint8_t ret = m_2108_prgbank0_r1;
	logerror("%s: vt1682_2108_prgbank0_r1_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vt_vt1682_state::vt1682_2108_prgbank0_r1_w)
{
	logerror("%s: vt1682_2108_prgbank0_r1_w writing: %02x\n", machine().describe_context(), data);
	m_2108_prgbank0_r1 = data;
	update_banks();
}


/*
    Address 0x2109 r/w (MAIN CPU)

    0x80 - Program Bank 0 Register 2
    0x40 - Program Bank 0 Register 2
    0x20 - Program Bank 0 Register 2
    0x10 - Program Bank 0 Register 2
    0x08 - Program Bank 0 Register 2
    0x04 - Program Bank 0 Register 2
    0x02 - Program Bank 0 Register 2
    0x01 - Program Bank 0 Register 2
*/


READ8_MEMBER(vt_vt1682_state::vt1682_2109_prgbank0_r2_r)
{
	uint8_t ret = m_2109_prgbank0_r2;
	logerror("%s: vt1682_2109_prgbank0_r2_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vt_vt1682_state::vt1682_2109_prgbank0_r2_w)
{
	logerror("%s: vt1682_2109_prgbank0_r2_w writing: %02x\n", machine().describe_context(), data);
	m_2109_prgbank0_r2 = data;
	update_banks();
}

/*
    Address 0x210a r/w (MAIN CPU)

    0x80 - Program Bank 0 Register 3
    0x40 - Program Bank 0 Register 3
    0x20 - Program Bank 0 Register 3
    0x10 - Program Bank 0 Register 3
    0x08 - Program Bank 0 Register 3
    0x04 - Program Bank 0 Register 3
    0x02 - Program Bank 0 Register 3
    0x01 - Program Bank 0 Register 3
*/

READ8_MEMBER(vt_vt1682_state::vt1682_210a_prgbank0_r3_r)
{
	uint8_t ret = m_210a_prgbank0_r3;
	logerror("%s: vt1682_210a_prgbank0_r3_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vt_vt1682_state::vt1682_210a_prgbank0_r3_w)
{
	logerror("%s: vt1682_210a_prgbank0_r3_w writing: %02x\n", machine().describe_context(), data);
	m_210a_prgbank0_r3 = data;
	update_banks();
}

/*
    Address 0x210b r/w (MAIN CPU)

    0x80 - TSYSN Enable
    0x40 - PQ2 Enable
    0x20 - BUS Tristate
    0x10 - CS Control:1
    0x08 - CS Control:0
    0x04 - Program Bank 0 Select
    0x02 - Program Bank 0 Select
    0x01 - Program Bank 0 Select
*/

READ8_MEMBER(vt_vt1682_state::vt1682_210b_misc_cs_prg0_bank_sel_r)
{
	uint8_t ret = m_210b_misc_cs_prg0_bank_sel;
	logerror("%s: vt1682_210b_misc_cs_prg0_bank_sel_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vt_vt1682_state::vt1682_210b_misc_cs_prg0_bank_sel_w)
{
	// PQ2 Enable is also used for ROM banking along with Program Bank 0 select

	logerror("%s: vt1682_210b_misc_cs_prg0_bank_sel_w writing: %02x\n", machine().describe_context(), data);
	m_210b_misc_cs_prg0_bank_sel = data;
	update_banks();
}


/*
    Address 0x210c r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 2
    0x04 - Program Bank 1 Register 2
    0x02 - Program Bank 1 Register 2
    0x01 - Program Bank 1 Register 2
*/

READ8_MEMBER(vt_vt1682_state::vt1682_210c_prgbank1_r2_r)
{
	uint8_t ret = m_210c_prgbank1_r2;
	logerror("%s: vt1682_210c_prgbank1_r2_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vt_vt1682_state::vt1682_210c_prgbank1_r2_w)
{
	logerror("%s: vt1682_210c_prgbank1_r2_w writing: %02x (4-bits)\n", machine().describe_context(), data);
	m_210c_prgbank1_r2 = data;
	update_banks();
}


/*
    Address 0x210d r/w (MAIN CPU)

    0x80 - IOD ENB
    0x40 - IOD OE
    0x20 - IOC ENB
    0x10 - IOC OE
    0x08 - IOB ENB
    0x04 - IOB OE
    0x02 - IOA ENB
    0x01 - IOA OE
*/

/*
    Address 0x210e r/w (MAIN CPU)

    0x80 - IOB:3
    0x40 - IOB:2
    0x20 - IOB:1
    0x10 - IOB:0
    0x08 - IOA:3
    0x04 - IOA:2
    0x02 - IOA:1
    0x01 - IOA:0
*/

/*
    Address 0x210f r/w (MAIN CPU)

    0x80 - IOD:3
    0x40 - IOD:2
    0x20 - IOD:1
    0x10 - IOD:0
    0x08 - IOC:3
    0x04 - IOC:2
    0x02 - IOC:1
    0x01 - IOC:0
*/

/*
   Address 0x2110 READ (MAIN CPU)

    0x80 - Program Bank 0 Register 4
    0x40 - Program Bank 0 Register 4
    0x20 - Program Bank 0 Register 4
    0x10 - Program Bank 0 Register 4
    0x08 - Program Bank 0 Register 4
    0x04 - Program Bank 0 Register 4
    0x02 - Program Bank 0 Register 4
    0x01 - Program Bank 0 Register 4

    Address 0x2110 WRITE (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 0
    0x04 - Program Bank 1 Register 0
    0x02 - Program Bank 1 Register 0
    0x01 - Program Bank 1 Register 0
*/

READ8_MEMBER(vt_vt1682_state::vt1682_prgbank0_r4_r)
{
	uint8_t ret = m_prgbank0_r4;
	logerror("%s: (2110) vt1682_prgbank0_r4_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vt_vt1682_state::vt1682_prgbank1_r0_w)
{
	logerror("%s: (2110) vt1682_prgbank1_r0_w writing: %02x (4-bits)\n", machine().describe_context(), data);
	m_prgbank1_r0 = data;
	update_banks();
}

/*
   Address 0x2111 READ (MAIN CPU)

    0x80 - Program Bank 0 Register 5
    0x40 - Program Bank 0 Register 5
    0x20 - Program Bank 0 Register 5
    0x10 - Program Bank 0 Register 5
    0x08 - Program Bank 0 Register 5
    0x04 - Program Bank 0 Register 5
    0x02 - Program Bank 0 Register 5
    0x01 - Program Bank 0 Register 5

    Address 0x2111 WRITE (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 1
    0x04 - Program Bank 1 Register 1
    0x02 - Program Bank 1 Register 1
    0x01 - Program Bank 1 Register 1
*/

READ8_MEMBER(vt_vt1682_state::vt1682_prgbank0_r5_r)
{
	uint8_t ret = m_prgbank0_r5;
	logerror("%s: (2111) vt1682_prgbank0_r5_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}


WRITE8_MEMBER(vt_vt1682_state::vt1682_prgbank1_r1_w)
{
	logerror("%s: (2111) vt1682_prgbank1_r1_w writing: %02x (4-bits)\n", machine().describe_context(), data);
	m_prgbank1_r1 = data;
	update_banks();
}


/*
    Address 0x2112 READ (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 0
    0x04 - Program Bank 1 Register 0
    0x02 - Program Bank 1 Register 0
    0x01 - Program Bank 1 Register 0

    Address 0x2112 WRITE (MAIN CPU)

    0x80 - Program Bank 0 Register 4
    0x40 - Program Bank 0 Register 4
    0x20 - Program Bank 0 Register 4
    0x10 - Program Bank 0 Register 4
    0x08 - Program Bank 0 Register 4
    0x04 - Program Bank 0 Register 4
    0x02 - Program Bank 0 Register 4
    0x01 - Program Bank 0 Register 4
*/

READ8_MEMBER(vt_vt1682_state::vt1682_prgbank1_r0_r)
{
	uint8_t ret = m_prgbank1_r0;
	logerror("%s: (2112) vt1682_prgbank1_r0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}


WRITE8_MEMBER(vt_vt1682_state::vt1682_prgbank0_r4_w)
{
	logerror("%s: (2112) vt1682_prgbank0_r4_w writing: %02x (8-bits)\n", machine().describe_context(), data);
	m_prgbank0_r4 = data;
	update_banks();
}


/*
    Address 0x2113 READ (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 1
    0x04 - Program Bank 1 Register 1
    0x02 - Program Bank 1 Register 1
    0x01 - Program Bank 1 Register 1

    Address 0x2113 WRITE (MAIN CPU)

    0x80 - Program Bank 0 Register 5
    0x40 - Program Bank 0 Register 5
    0x20 - Program Bank 0 Register 5
    0x10 - Program Bank 0 Register 5
    0x08 - Program Bank 0 Register 5
    0x04 - Program Bank 0 Register 5
    0x02 - Program Bank 0 Register 5
    0x01 - Program Bank 0 Register 5
*/

READ8_MEMBER(vt_vt1682_state::vt1682_prgbank1_r1_r)
{
	uint8_t ret = m_prgbank1_r1;
	logerror("%s: (2113) vt1682_prgbank1_r1_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vt_vt1682_state::vt1682_prgbank0_r5_w)
{
	logerror("%s: (2113) vt1682_prgbank0_r5_w writing: %02x (8-bits)\n", machine().describe_context(), data);
	m_prgbank0_r5 = data;
	update_banks();
}

/*
    Address 0x2114 r/w (MAIN CPU)

    0x80 - Baud Rate:7
    0x40 - Baud Rate:6
    0x20 - Baud Rate:5
    0x10 - Baud Rate:4
    0x08 - Baud Rate:3
    0x04 - Baud Rate:2
    0x02 - Baud Rate:1
    0x01 - Baud Rate:0
*/

/*
    Address 0x2115 r/w (MAIN CPU)

    0x80 - Baud Rate:15
    0x40 - Baud Rate:14
    0x20 - Baud Rate:13
    0x10 - Baud Rate:12
    0x08 - Baud Rate:11
    0x04 - Baud Rate:10
    0x02 - Baud Rate:9
    0x01 - Baud Rate:8
*/

/*
    Address 0x2116 r/w (MAIN CPU)

    0x80 - 16bit SPI
    0x40 - SPIEN
    0x20 - SPI RST
    0x10 - M/SB
    0x08 - CLK PHASE
    0x04 - CLK POLARITY
    0x02 - CLK FREQ:1
    0x01 - CLK FREQ:0
*/

/*
    Address 0x2117 WRITE (MAIN CPU)

    0x80 - SPI TX Data
    0x40 - SPI TX Data
    0x20 - SPI TX Data
    0x10 - SPI TX Data
    0x08 - SPI TX Data
    0x04 - SPI TX Data
    0x02 - SPI TX Data
    0x01 - SPI TX Data

    Address 0x2117 READ (MAIN CPU)

    0x80 - SPI RX Data
    0x40 - SPI RX Data
    0x20 - SPI RX Data
    0x10 - SPI RX Data
    0x08 - SPI RX Data
    0x04 - SPI RX Data
    0x02 - SPI RX Data
    0x01 - SPI RX Data
*/

/*
    Address 0x2118 r/w (MAIN CPU)

    0x80 - Program Bank 1 Register 5
    0x40 - Program Bank 1 Register 5
    0x20 - Program Bank 1 Register 5
    0x10 - Program Bank 1 Register 5
    0x08 - Program Bank 1 Register 4
    0x04 - Program Bank 1 Register 4
    0x02 - Program Bank 1 Register 4
    0x01 - Program Bank 1 Register 4
*/

READ8_MEMBER(vt_vt1682_state::vt1682_2118_prgbank1_r4_r5_r)
{
	uint8_t ret = m_2118_prgbank1_r4_r5;
	logerror("%s: vt1682_2118_prgbank1_r4_r5_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vt_vt1682_state::vt1682_2118_prgbank1_r4_r5_w)
{
	logerror("%s: vt1682_2118_prgbank1_r4_r5_w writing: %02x (2x 4-bits)\n", machine().describe_context(), data);
	m_2118_prgbank1_r4_r5 = data;
	update_banks();
}


/*
    Address 0x2119 WRITE ONLY (MAIN CPU)

    0x80 - (unused)
    0x40 - Carrier En
    0x20 - UART En
    0x10 - Tx IRQ En
    0x08 - Rx IRQ En
    0x04 - Parity En
    0x02 - Odd/Even
    0x01 - 9bit Mode
*/

/*
    Address 0x211a WRITE (MAIN CPU)

    0x80 - TX Data
    0x40 - TX Data
    0x20 - TX Data
    0x10 - TX Data
    0x08 - TX Data
    0x04 - TX Data
    0x02 - TX Data
    0x01 - TX Data

    Address 0x211a READ (MAIN CPU)

    0x80 - RX Data
    0x40 - RX Data
    0x20 - RX Data
    0x10 - RX Data
    0x08 - RX Data
    0x04 - RX Data
    0x02 - RX Data
    0x01 - RX Data
*/

/*
    Address 0x211b WRITE (MAIN CPU)

    0x80 - Carrier Freq
    0x40 - Carrier Freq
    0x20 - Carrier Freq
    0x10 - Carrier Freq
    0x08 - Carrier Freq
    0x04 - Carrier Freq
    0x02 - Carrier Freq
    0x01 - Carrier Freq

    Address 0x211b READ (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - Rx Error
    0x10 - Tx Status
    0x08 - Rx Status
    0x04 - Parity Error
    0x02 - (unused)
    0x01 - (unused)
*/

/*
    Address 0x211c WRITE (MAIN CPU)

    0x80 - AutoWake
    0x40 - KeyWake
    0x20 - EXT2421EN
    0x10 - SCPUIRQ
    0x08 - SLEEPM
    0x04 - (unused)
    0x02 - SLEEP SEL
    0x01 - CLK SEL

    Address 0x211c READ (MAIN CPU)

    0x80 - Clear_SCPU_IRQ
    0x40 - Clear_SCPU_IRQ
    0x20 - Clear_SCPU_IRQ
    0x10 - Clear_SCPU_IRQ
    0x08 - Clear_SCPU_IRQ
    0x04 - Clear_SCPU_IRQ
    0x02 - Clear_SCPU_IRQ
    0x01 - Clear_SCPU_IRQ
*/

WRITE8_MEMBER(vt_vt1682_state::vt1682_211c_regs_ext2421_w)
{
	// EXT2421EN is used for ROM banking
	logerror("%s: vt1682_211c_regs_ext2421_w writing: %02x\n", machine().describe_context(), data);
	m_211c_regs_ext2421 = data;
	update_banks();
}


/*
    Address 0x211d WRITE (MAIN CPU)

    0x80 - LVDEN
    0x40 - LVDS1
    0x20 - LVDS0
    0x10 - VDAC_EN
    0x08 - ADAC_EN
    0x04 - PLL_EN
    0x02 - LCDACEN
    0x01 - (unused)

    Address 0x211d READ (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - (unused)
    0x01 - LVD
*/

/*
    Address 0x211e WRITE (MAIN CPU)

    0x80 - ADCEN
    0x40 - ADCS1
    0x20 - ADCS0
    0x10 - (unused)
    0x08 - IOFOEN3
    0x04 - IOFOEN2
    0x02 - IOFOEN1
    0x01 - IOFOEN0

    Address 0x211e READ (MAIN CPU)

    0x80 - ADC DATA:7
    0x40 - ADC DATA:6
    0x20 - ADC DATA:5
    0x10 - ADC DATA:4
    0x08 - ADC DATA:3
    0x04 - ADC DATA:2
    0x02 - ADC DATA:1
    0x01 - ADC DATA:0
*/

/*
    Address 0x211f r/w (MAIN CPU)

    0x80 - VGCEN
    0x40 - VGCA6
    0x20 - VGCA5
    0x10 - VGCA4
    0x08 - VGCA3
    0x04 - VGCA2
    0x02 - VGCA1
    0x01 - VGCA0
*/

/*
    Address 0x2120 r/w (MAIN CPU)

    0x80 - Sleep Period
    0x40 - Sleep Period
    0x20 - Sleep Period
    0x10 - Sleep Period
    0x08 - Sleep Period
    0x04 - Sleep Period
    0x02 - Sleep Period
    0x01 - Sleep Period
*/

/*
    Address 0x2121 READ (MAIN CPU) (maybe)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - SPI MSK
    0x08 - UART MSK
    0x04 - SPU MSK
    0x02 - TMR MSK
    0x01 - Ext MSK

    Address 0x2121 WRITE (MAIN CPU) (maybe)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - Clear SPU
    0x02 - (unused)
    0x01 - Clear Ext
*/

/*
    Address 0x2122 r/w (MAIN CPU)

    0x80 - DMA DT ADDR:7
    0x40 - DMA DT ADDR:6
    0x20 - DMA DT ADDR:5
    0x10 - DMA DT ADDR:4
    0x08 - DMA DT ADDR:3
    0x04 - DMA DT ADDR:2
    0x02 - DMA DT ADDR:1
    0x01 - DMA DT ADDR:0
*/

/*
    Address 0x2123 r/w (MAIN CPU)

    0x80 - DMA DT ADDR:15
    0x40 - DMA DT ADDR:14
    0x20 - DMA DT ADDR:13
    0x10 - DMA DT ADDR:12
    0x08 - DMA DT ADDR:11
    0x04 - DMA DT ADDR:10
    0x02 - DMA DT ADDR:9
    0x01 - DMA DT ADDR:8
*/

/*
    Address 0x2124 r/w (MAIN CPU)

    0x80 - DMA SR ADDR:7
    0x40 - DMA SR ADDR:6
    0x20 - DMA SR ADDR:5
    0x10 - DMA SR ADDR:4
    0x08 - DMA SR ADDR:3
    0x04 - DMA SR ADDR:2
    0x02 - DMA SR ADDR:1
    0x01 - DMA SR ADDR:0
*/

/*
    Address 0x2125 r/w (MAIN CPU)

    0x80 - DMA SR ADDR:15
    0x40 - DMA SR ADDR:14
    0x20 - DMA SR ADDR:13
    0x10 - DMA SR ADDR:12
    0x08 - DMA SR ADDR:11
    0x04 - DMA SR ADDR:10
    0x02 - DMA SR ADDR:9
    0x01 - DMA SR ADDR:8
*/

/*
    Address 0x2126 r/w (MAIN CPU)

    0x80 - DMA SR BANK:22
    0x40 - DMA SR BANK:21
    0x20 - DMA SR BANK:20
    0x10 - DMA SR BANK:19
    0x08 - DMA SR BANK:18
    0x04 - DMA SR BANK:17
    0x02 - DMA SR BANK:16
    0x01 - DMA SR BANK:15
*/

/*
    Address 0x2127 WRITE (MAIN CPU)

    0x80 - DMA Number
    0x40 - DMA Number
    0x20 - DMA Number
    0x10 - DMA Number
    0x08 - DMA Number
    0x04 - DMA Number
    0x02 - DMA Number
    0x01 - DMA Number

    Address 0x2127 READ (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - (unused)
    0x01 - DMA Status
*/

/*
    Address 0x2128 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - DMA SR BANK:24
    0x01 - DMA SR BANK:23
*/

/*
    Address 0x2129 READ (MAIN CPU)

    0x80 - UIOA DATA IN / Send Joy CLK
    0x40 - UIOA DATA IN / Send Joy CLK
    0x20 - UIOA DATA IN / Send Joy CLK
    0x10 - UIOA DATA IN / Send Joy CLK
    0x08 - UIOA DATA IN / Send Joy CLK
    0x04 - UIOA DATA IN / Send Joy CLK
    0x02 - UIOA DATA IN / Send Joy CLK
    0x01 - UIOA DATA IN / Send Joy CLK

    Address 0x2129 WRITE (MAIN CPU)

    0x80 - UIOA DATA OUT
    0x40 - UIOA DATA OUT
    0x20 - UIOA DATA OUT
    0x10 - UIOA DATA OUT
    0x08 - UIOA DATA OUT
    0x04 - UIOA DATA OUT
    0x02 - UIOA DATA OUT
    0x01 - UIOA DATA OUT

*/

/*
    Address 0x212a READ (MAIN CPU)

    0x80 - Send Joy CLK 2
    0x40 - Send Joy CLK 2
    0x20 - Send Joy CLK 2
    0x10 - Send Joy CLK 2
    0x08 - Send Joy CLK 2
    0x04 - Send Joy CLK 2
    0x02 - Send Joy CLK 2
    0x01 - Send Joy CLK 2

    Address 0x212a WRITE (MAIN CPU)

    0x80 - UIOA DIRECTION
    0x40 - UIOA DIRECTION
    0x20 - UIOA DIRECTION
    0x10 - UIOA DIRECTION
    0x08 - UIOA DIRECTION
    0x04 - UIOA DIRECTION
    0x02 - UIOA DIRECTION
    0x01 - UIOA DIRECTION
*/

/*
    Address 0x212b r/w (MAIN CPU)

    0x80 - UIOA ATTRIBUTE
    0x40 - UIOA ATTRIBUTE
    0x20 - UIOA ATTRIBUTE
    0x10 - UIOA ATTRIBUTE
    0x08 - UIOA ATTRIBUTE
    0x04 - UIOA ATTRIBUTE
    0x02 - UIOA ATTRIBUTE
    0x01 - UIOA ATTRIBUTE
*/

/*
    Address 0x212c READ (MAIN CPU)

    0x80 - Pseudo Random Number
    0x40 - Pseudo Random Number
    0x20 - Pseudo Random Number
    0x10 - Pseudo Random Number
    0x08 - Pseudo Random Number
    0x04 - Pseudo Random Number
    0x02 - Pseudo Random Number
    0x01 - Pseudo Random Number

    Address 0x212c WRITE (MAIN CPU)

    0x80 - Pseudo Random Number Seed
    0x40 - Pseudo Random Number Seed
    0x20 - Pseudo Random Number Seed
    0x10 - Pseudo Random Number Seed
    0x08 - Pseudo Random Number Seed
    0x04 - Pseudo Random Number Seed
    0x02 - Pseudo Random Number Seed
    0x01 - Pseudo Random Number Seed
*/

/*
    Address 0x212d WRITE ONLY (MAIN CPU)

    0x80 - PLL B
    0x40 - PLL B
    0x20 - PLL B
    0x10 - PLL B
    0x08 - PLL M
    0x04 - PLL A
    0x02 - PLL A
    0x01 - PLL A
*/

/* Address 0x212e Unused */
/* Address 0x212f Unused */

/*
    Address 0x2130 WRITE (MAIN CPU)

    0x80 - ALU Oprand 1
    0x40 - ALU Oprand 1
    0x20 - ALU Oprand 1
    0x10 - ALU Oprand 1
    0x08 - ALU Oprand 1
    0x04 - ALU Oprand 1
    0x02 - ALU Oprand 1
    0x01 - ALU Oprand 1

    Address 0x2130 READ (MAIN CPU)

    0x80 - ALU Output 1
    0x40 - ALU Output 1
    0x20 - ALU Output 1
    0x10 - ALU Output 1
    0x08 - ALU Output 1
    0x04 - ALU Output 1
    0x02 - ALU Output 1
    0x01 - ALU Output 1
*/

/*
    Address 0x2131 WRITE (MAIN CPU)

    0x80 - ALU Oprand 2
    0x40 - ALU Oprand 2
    0x20 - ALU Oprand 2
    0x10 - ALU Oprand 2
    0x08 - ALU Oprand 2
    0x04 - ALU Oprand 2
    0x02 - ALU Oprand 2
    0x01 - ALU Oprand 2

    Address 0x2131 READ (MAIN CPU)

    0x80 - ALU Output 2
    0x40 - ALU Output 2
    0x20 - ALU Output 2
    0x10 - ALU Output 2
    0x08 - ALU Output 2
    0x04 - ALU Output 2
    0x02 - ALU Output 2
    0x01 - ALU Output 2
*/

/*
    Address 0x2132 WRITE (MAIN CPU)

    0x80 - ALU Oprand 3
    0x40 - ALU Oprand 3
    0x20 - ALU Oprand 3
    0x10 - ALU Oprand 3
    0x08 - ALU Oprand 3
    0x04 - ALU Oprand 3
    0x02 - ALU Oprand 3
    0x01 - ALU Oprand 3

    Address 0x2132 READ (MAIN CPU)

    0x80 - ALU Output 3
    0x40 - ALU Output 3
    0x20 - ALU Output 3
    0x10 - ALU Output 3
    0x08 - ALU Output 3
    0x04 - ALU Output 3
    0x02 - ALU Output 3
    0x01 - ALU Output 3
*/

/*
    Address 0x2133 WRITE (MAIN CPU)

    0x80 - ALU Oprand 4
    0x40 - ALU Oprand 4
    0x20 - ALU Oprand 4
    0x10 - ALU Oprand 4
    0x08 - ALU Oprand 4
    0x04 - ALU Oprand 4
    0x02 - ALU Oprand 4
    0x01 - ALU Oprand 4

    Address 0x2133 READ (MAIN CPU)

    0x80 - ALU Output 4
    0x40 - ALU Output 4
    0x20 - ALU Output 4
    0x10 - ALU Output 4
    0x08 - ALU Output 4
    0x04 - ALU Output 4
    0x02 - ALU Output 4
    0x01 - ALU Output 4
*/

/*
    Address 0x2134 WRITE (MAIN CPU)

    0x80 - ALU Mul Oprand 5
    0x40 - ALU Mul Oprand 5
    0x20 - ALU Mul Oprand 5
    0x10 - ALU Mul Oprand 5
    0x08 - ALU Mul Oprand 5
    0x04 - ALU Mul Oprand 5
    0x02 - ALU Mul Oprand 5
    0x01 - ALU Mul Oprand 5

    Address 0x2134 READ (MAIN CPU)

    0x80 - ALU Output 5
    0x40 - ALU Output 5
    0x20 - ALU Output 5
    0x10 - ALU Output 5
    0x08 - ALU Output 5
    0x04 - ALU Output 5
    0x02 - ALU Output 5
    0x01 - ALU Output 5
*/

/*
    Address 0x2135 WRITE (MAIN CPU)

    0x80 - ALU Mul Oprand 6
    0x40 - ALU Mul Oprand 6
    0x20 - ALU Mul Oprand 6
    0x10 - ALU Mul Oprand 6
    0x08 - ALU Mul Oprand 6
    0x04 - ALU Mul Oprand 6
    0x02 - ALU Mul Oprand 6
    0x01 - ALU Mul Oprand 6

    Address 0x2135 READ (MAIN CPU)

    0x80 - ALU Output 6
    0x40 - ALU Output 6
    0x20 - ALU Output 6
    0x10 - ALU Output 6
    0x08 - ALU Output 6
    0x04 - ALU Output 6
    0x02 - ALU Output 6
    0x01 - ALU Output 6
*/

/*
    Address 0x2136 WRITE ONLY (MAIN CPU)

    0x80 - ALU Div Oprand 5
    0x40 - ALU Div Oprand 5
    0x20 - ALU Div Oprand 5
    0x10 - ALU Div Oprand 5
    0x08 - ALU Div Oprand 5
    0x04 - ALU Div Oprand 5
    0x02 - ALU Div Oprand 5
    0x01 - ALU Div Oprand 5

*/

/*
    Address 0x2137 WRITE ONLY (MAIN CPU)

    0x80 - ALU Div Oprand 6
    0x40 - ALU Div Oprand 6
    0x20 - ALU Div Oprand 6
    0x10 - ALU Div Oprand 6
    0x08 - ALU Div Oprand 6
    0x04 - ALU Div Oprand 6
    0x02 - ALU Div Oprand 6
    0x01 - ALU Div Oprand 6
*/

/* Address 0x2138 Unused */
/* Address 0x2139 Unused */
/* Address 0x213a Unused */
/* Address 0x213b Unused */
/* Address 0x213c Unused */
/* Address 0x213d Unused */
/* Address 0x213e Unused */
/* Address 0x213f Unused */

/*
    Address 0x2140 r/w (MAIN CPU)

    0x80 - I2C ID
    0x40 - I2C ID
    0x20 - I2C ID
    0x10 - I2C ID
    0x08 - I2C ID
    0x04 - I2C ID
    0x02 - I2C ID
    0x01 - I2C ID
*/

/*
    Address 0x2141 r/w (MAIN CPU)

    0x80 - I2C ADDR
    0x40 - I2C ADDR
    0x20 - I2C ADDR
    0x10 - I2C ADDR
    0x08 - I2C ADDR
    0x04 - I2C ADDR
    0x02 - I2C ADDR
    0x01 - I2C ADDR
*/

/*
    Address 0x2142 r/w (MAIN CPU)

    0x80 - I2C DATA
    0x40 - I2C DATA
    0x20 - I2C DATA
    0x10 - I2C DATA
    0x08 - I2C DATA
    0x04 - I2C DATA
    0x02 - I2C DATA
    0x01 - I2C DATA
*/

/*
    Address 0x2143 WRITE ONLY (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - I2C CLK SELECT
    0x01 - I2C CLK SELECT
*/

/* Address 0x2144 Unused */
/* Address 0x2145 Unused */
/* Address 0x2146 Unused */
/* Address 0x2147 Unused */

/*
    Address 0x2148 WRITE ONLY (MAIN CPU)

    0x80 - UIOB SEL:7
    0x40 - UIOB SEL:6
    0x20 - UIOB SEL:5
    0x10 - UIOB SEL:4
    0x08 - UIOB SEL:3
    0x04 - (unused)
    0x02 - UIOA MODE
    0x01 - UIOA MODE
*/

/*
    Address 0x2149 WRITE (MAIN CPU)

    0x80 - UIOB DATA OUT
    0x40 - UIOB DATA OUT
    0x20 - UIOB DATA OUT
    0x10 - UIOB DATA OUT
    0x08 - UIOB DATA OUT
    0x04 - UIOB DATA OUT
    0x02 - UIOB DATA OUT
    0x01 - UIOB DATA OUT

    Address 0x2149 READ (MAIN CPU)

    0x80 - UIOB DATA IN
    0x40 - UIOB DATA IN
    0x20 - UIOB DATA IN
    0x10 - UIOB DATA IN
    0x08 - UIOB DATA IN
    0x04 - UIOB DATA IN
    0x02 - UIOB DATA IN
    0x01 - UIOB DATA IN
*/

/*
    Address 0x214a r/w (MAIN CPU)

    0x80 - UIOB DIRECTION
    0x40 - UIOB DIRECTION
    0x20 - UIOB DIRECTION
    0x10 - UIOB DIRECTION
    0x08 - UIOB DIRECTION
    0x04 - UIOB DIRECTION
    0x02 - UIOB DIRECTION
    0x01 - UIOB DIRECTION
*/

/*
    Address 0x214b r/w (MAIN CPU)

    0x80 - UIOB ATTRIBUTE
    0x40 - UIOB ATTRIBUTE
    0x20 - UIOB ATTRIBUTE
    0x10 - UIOB ATTRIBUTE
    0x08 - UIOB ATTRIBUTE
    0x04 - UIOB ATTRIBUTE
    0x02 - UIOB ATTRIBUTE
    0x01 - UIOB ATTRIBUTE
*/

/*
    Address 0x214c r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - Keychange Enable
    0x10 - Keychange Enable
    0x08 - IOFEN
    0x04 - (unused)
    0x02 - (unused)
    0x01 - IOEOEN
*/

/*
    Address 0x214d r/w (MAIN CPU)

    0x80 - IOF:3
    0x40 - IOF:2
    0x20 - IOF:1
    0x10 - IOF:0
    0x08 - IOE:3
    0x04 - IOE:2
    0x02 - IOE:1
    0x01 - IOE:0
*/


/************************************************************************************************************************************
 VT1682 Sound CPU Registers
************************************************************************************************************************************/

/*
    Address 0x2100 r/w (SOUND CPU)

    0x80 - Timer A Preload:7
    0x40 - Timer A Preload:6
    0x20 - Timer A Preload:5
    0x10 - Timer A Preload:4
    0x08 - Timer A Preload:3
    0x04 - Timer A Preload:2
    0x02 - Timer A Preload:1
    0x01 - Timer A Preload:0
*/

/*
    Address 0x2101 r/w (SOUND CPU)

    0x80 - Timer A Preload:15
    0x40 - Timer A Preload:14
    0x20 - Timer A Preload:13
    0x10 - Timer A Preload:12
    0x08 - Timer A Preload:11
    0x04 - Timer A Preload:10
    0x02 - Timer A Preload:9
    0x01 - Timer A Preload:8
*/

/*
    Address 0x2102 r/w (SOUND CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - TMRA IRQ
    0x01 - TMRA EN
*/

/*
    Address 0x2103 r/w (SOUND CPU)

    0x80 - Timer A IRQ Clear
    0x40 - Timer A IRQ Clear
    0x20 - Timer A IRQ Clear
    0x10 - Timer A IRQ Clear
    0x08 - Timer A IRQ Clear
    0x04 - Timer A IRQ Clear
    0x02 - Timer A IRQ Clear
    0x01 - Timer A IRQ Clear
*/

/* Address 0x2104 Unused (SOUND CPU) */
/* Address 0x2105 Unused (SOUND CPU) */
/* Address 0x2106 Unused (SOUND CPU) */
/* Address 0x2107 Unused (SOUND CPU) */
/* Address 0x2108 Unused (SOUND CPU) */
/* Address 0x2109 Unused (SOUND CPU) */
/* Address 0x210a Unused (SOUND CPU) */
/* Address 0x210b Unused (SOUND CPU) */
/* Address 0x210c Unused (SOUND CPU) */
/* Address 0x210d Unused (SOUND CPU) */
/* Address 0x210e Unused (SOUND CPU) */
/* Address 0x210f Unused (SOUND CPU) */


/*
    Address 0x2110 r/w (SOUND CPU)

    0x80 - Timer B Preload:7
    0x40 - Timer B Preload:6
    0x20 - Timer B Preload:5
    0x10 - Timer B Preload:4
    0x08 - Timer B Preload:3
    0x04 - Timer B Preload:2
    0x02 - Timer B Preload:1
    0x01 - Timer B Preload:0
*/

/*
    Address 0x2111 r/w (SOUND CPU)

    0x80 - Timer B Preload:15
    0x40 - Timer B Preload:14
    0x20 - Timer B Preload:13
    0x10 - Timer B Preload:12
    0x08 - Timer B Preload:11
    0x04 - Timer B Preload:10
    0x02 - Timer B Preload:9
    0x01 - Timer B Preload:8
*/

/*
    Address 0x2112 r/w (SOUND CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - TMRB IRQ
    0x01 - TMRB EN
*/

/*
    Address 0x2113 r/w (SOUND CPU)

    0x80 - Timer B IRQ Clear
    0x40 - Timer B IRQ Clear
    0x20 - Timer B IRQ Clear
    0x10 - Timer B IRQ Clear
    0x08 - Timer B IRQ Clear
    0x04 - Timer B IRQ Clear
    0x02 - Timer B IRQ Clear
    0x01 - Timer B IRQ Clear
*/

/* Address 0x2114 Unused (SOUND CPU) */
/* Address 0x2115 Unused (SOUND CPU) */
/* Address 0x2116 Unused (SOUND CPU) */
/* Address 0x2117 Unused (SOUND CPU) */

/*
    Address 0x2118 r/w (SOUND CPU)

    0x80 - Audio DAC Left:7
    0x40 - Audio DAC Left:6
    0x20 - Audio DAC Left:5
    0x10 - Audio DAC Left:4
    0x08 - Audio DAC Left:3
    0x04 - Audio DAC Left:2
    0x02 - Audio DAC Left:1
    0x01 - Audio DAC Left:0
*/

/*
    Address 0x2119 r/w (SOUND CPU)

    0x80 - Audio DAC Left:15
    0x40 - Audio DAC Left:14
    0x20 - Audio DAC Left:13
    0x10 - Audio DAC Left:12
    0x08 - Audio DAC Left:11
    0x04 - Audio DAC Left:10
    0x02 - Audio DAC Left:9
    0x01 - Audio DAC Left:8
*/

/*
    Address 0x211a r/w (SOUND CPU)

    0x80 - Audio DAC Right:7
    0x40 - Audio DAC Right:6
    0x20 - Audio DAC Right:5
    0x10 - Audio DAC Right:4
    0x08 - Audio DAC Right:3
    0x04 - Audio DAC Right:2
    0x02 - Audio DAC Right:1
    0x01 - Audio DAC Right:0
*/

/*
    Address 0x211b r/w (SOUND CPU)

    0x80 - Audio DAC Right:15
    0x40 - Audio DAC Right:14
    0x20 - Audio DAC Right:13
    0x10 - Audio DAC Right:12
    0x08 - Audio DAC Right:11
    0x04 - Audio DAC Right:10
    0x02 - Audio DAC Right:9
    0x01 - Audio DAC Right:8
*/

/*
    Address 0x211c WRITE (SOUND CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - IRQ_OUT
    0x08 - SLEEP
    0x04 - ExtIRQSel
    0x02 - NMI_EN
    0x01 - ExtMask

    Address 0x211c READ (SOUND CPU)

    0x80 - Clear_CPU_IRQ
    0x40 - Clear_CPU_IRQ
    0x20 - Clear_CPU_IRQ
    0x10 - Clear_CPU_IRQ
    0x08 - Clear_CPU_IRQ
    0x04 - Clear_CPU_IRQ
    0x02 - Clear_CPU_IRQ
    0x01 - Clear_CPU_IRQ
*/

/*
    Address 0x211d r/w (SOUND CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - IIS Mode
    0x01 - IIS EN
*/

/* Address 0x211E Unused? (maybe) (SOUND CPU) */
/* Address 0x211F Unused (SOUND CPU) */
/* Address 0x2120 Unused (SOUND CPU) */
/* Address 0x2121 Unused (SOUND CPU) */
/* Address 0x2122 Unused (SOUND CPU) */
/* Address 0x2123 Unused (SOUND CPU) */
/* Address 0x2124 Unused (SOUND CPU) */
/* Address 0x2125 Unused (SOUND CPU) */
/* Address 0x2126 Unused (SOUND CPU) */
/* Address 0x2127 Unused (SOUND CPU) */
/* Address 0x2128 Unused (SOUND CPU) */
/* Address 0x2129 Unused (SOUND CPU) */
/* Address 0x212a Unused (SOUND CPU) */
/* Address 0x212b Unused (SOUND CPU) */
/* Address 0x212c Unused (SOUND CPU) */
/* Address 0x212d Unused (SOUND CPU) */
/* Address 0x212e Unused (SOUND CPU) */
/* Address 0x212f Unused (SOUND CPU) */

/*
    Address 0x2130 WRITE (SOUND CPU)

    0x80 - ALU Oprand 1
    0x40 - ALU Oprand 1
    0x20 - ALU Oprand 1
    0x10 - ALU Oprand 1
    0x08 - ALU Oprand 1
    0x04 - ALU Oprand 1
    0x02 - ALU Oprand 1
    0x01 - ALU Oprand 1

    Address 0x2130 READ (SOUND CPU)

    0x80 - ALU Output 1
    0x40 - ALU Output 1
    0x20 - ALU Output 1
    0x10 - ALU Output 1
    0x08 - ALU Output 1
    0x04 - ALU Output 1
    0x02 - ALU Output 1
    0x01 - ALU Output 1
*/

/*
    Address 0x2131 WRITE (SOUND CPU)

    0x80 - ALU Oprand 2
    0x40 - ALU Oprand 2
    0x20 - ALU Oprand 2
    0x10 - ALU Oprand 2
    0x08 - ALU Oprand 2
    0x04 - ALU Oprand 2
    0x02 - ALU Oprand 2
    0x01 - ALU Oprand 2

    Address 0x2131 READ (SOUND CPU)

    0x80 - ALU Output 2
    0x40 - ALU Output 2
    0x20 - ALU Output 2
    0x10 - ALU Output 2
    0x08 - ALU Output 2
    0x04 - ALU Output 2
    0x02 - ALU Output 2
    0x01 - ALU Output 2
*/

/*
    Address 0x2132 WRITE (SOUND CPU)

    0x80 - ALU Oprand 3
    0x40 - ALU Oprand 3
    0x20 - ALU Oprand 3
    0x10 - ALU Oprand 3
    0x08 - ALU Oprand 3
    0x04 - ALU Oprand 3
    0x02 - ALU Oprand 3
    0x01 - ALU Oprand 3

    Address 0x2132 READ (SOUND CPU)

    0x80 - ALU Output 3
    0x40 - ALU Output 3
    0x20 - ALU Output 3
    0x10 - ALU Output 3
    0x08 - ALU Output 3
    0x04 - ALU Output 3
    0x02 - ALU Output 3
    0x01 - ALU Output 3
*/

/*
    Address 0x2133 WRITE (SOUND CPU)

    0x80 - ALU Oprand 4
    0x40 - ALU Oprand 4
    0x20 - ALU Oprand 4
    0x10 - ALU Oprand 4
    0x08 - ALU Oprand 4
    0x04 - ALU Oprand 4
    0x02 - ALU Oprand 4
    0x01 - ALU Oprand 4

    Address 0x2133 READ (SOUND CPU)

    0x80 - ALU Output 4
    0x40 - ALU Output 4
    0x20 - ALU Output 4
    0x10 - ALU Output 4
    0x08 - ALU Output 4
    0x04 - ALU Output 4
    0x02 - ALU Output 4
    0x01 - ALU Output 4
*/

/*
    Address 0x2134 WRITE (SOUND CPU)

    0x80 - ALU Mul Oprand 5
    0x40 - ALU Mul Oprand 5
    0x20 - ALU Mul Oprand 5
    0x10 - ALU Mul Oprand 5
    0x08 - ALU Mul Oprand 5
    0x04 - ALU Mul Oprand 5
    0x02 - ALU Mul Oprand 5
    0x01 - ALU Mul Oprand 5

    Address 0x2134 READ (SOUND CPU)

    0x80 - ALU Output 5
    0x40 - ALU Output 5
    0x20 - ALU Output 5
    0x10 - ALU Output 5
    0x08 - ALU Output 5
    0x04 - ALU Output 5
    0x02 - ALU Output 5
    0x01 - ALU Output 5
*/

/*
    Address 0x2135 WRITE (SOUND CPU)

    0x80 - ALU Mul Oprand 6
    0x40 - ALU Mul Oprand 6
    0x20 - ALU Mul Oprand 6
    0x10 - ALU Mul Oprand 6
    0x08 - ALU Mul Oprand 6
    0x04 - ALU Mul Oprand 6
    0x02 - ALU Mul Oprand 6
    0x01 - ALU Mul Oprand 6

    Address 0x2135 READ (SOUND CPU)

    0x80 - ALU Output 6
    0x40 - ALU Output 6
    0x20 - ALU Output 6
    0x10 - ALU Output 6
    0x08 - ALU Output 6
    0x04 - ALU Output 6
    0x02 - ALU Output 6
    0x01 - ALU Output 6
*/

/*
    Address 0x2136 WRITE ONLY (SOUND CPU)

    0x80 - ALU Div Oprand 5
    0x40 - ALU Div Oprand 5
    0x20 - ALU Div Oprand 5
    0x10 - ALU Div Oprand 5
    0x08 - ALU Div Oprand 5
    0x04 - ALU Div Oprand 5
    0x02 - ALU Div Oprand 5
    0x01 - ALU Div Oprand 5

*/

/*
    Address 0x2137 WRITE ONLY (SOUND CPU)

    0x80 - ALU Div Oprand 6
    0x40 - ALU Div Oprand 6
    0x20 - ALU Div Oprand 6
    0x10 - ALU Div Oprand 6
    0x08 - ALU Div Oprand 6
    0x04 - ALU Div Oprand 6
    0x02 - ALU Div Oprand 6
    0x01 - ALU Div Oprand 6
*/

/* Address 0x2138 Unused (SOUND CPU) */
/* Address 0x2139 Unused (SOUND CPU) */
/* Address 0x213a Unused (SOUND CPU) */
/* Address 0x213b Unused (SOUND CPU) */
/* Address 0x213c Unused (SOUND CPU) */
/* Address 0x213d Unused (SOUND CPU) */
/* Address 0x213e Unused (SOUND CPU) */
/* Address 0x213f Unused (SOUND CPU) */

/*
    Address 0x2140 r/w (SOUND CPU)

    0x80 - IOA DATA
    0x40 - IOA DATA
    0x20 - IOA DATA
    0x10 - IOA DATA
    0x08 - IOA DATA
    0x04 - IOA DATA
    0x02 - IOA DATA
    0x01 - IOA DATA
*/

/*
    Address 0x2141 r/w (SOUND CPU)

    0x80 - IOA DIR
    0x40 - IOA DIR
    0x20 - IOA DIR
    0x10 - IOA DIR
    0x08 - IOA DIR
    0x04 - IOA DIR
    0x02 - IOA DIR
    0x01 - IOA DIR
*/

/*
    Address 0x2142 r/w (SOUND CPU)

    0x80 - IOA PLH
    0x40 - IOA PLH
    0x20 - IOA PLH
    0x10 - IOA PLH
    0x08 - IOA PLH
    0x04 - IOA PLH
    0x02 - IOA PLH
    0x01 - IOA PLH
*/

/* Address 0x2143 Unused (SOUND CPU) */

/*
    Address 0x2144 r/w (SOUND CPU)

    0x80 - IOB DATA
    0x40 - IOB DATA
    0x20 - IOB DATA
    0x10 - IOB DATA
    0x08 - IOB DATA
    0x04 - IOB DATA
    0x02 - IOB DATA
    0x01 - IOB DATA
*/

/*
    Address 0x2145 r/w (SOUND CPU)

    0x80 - IOB DIR
    0x40 - IOB DIR
    0x20 - IOB DIR
    0x10 - IOB DIR
    0x08 - IOB DIR
    0x04 - IOB DIR
    0x02 - IOB DIR
    0x01 - IOB DIR
*/

/*
    Address 0x2146 r/w (SOUND CPU)

    0x80 - IOB PLH
    0x40 - IOB PLH
    0x20 - IOB PLH
    0x10 - IOB PLH
    0x08 - IOB PLH
    0x04 - IOB PLH
    0x02 - IOB PLH
    0x01 - IOB PLH
*/


uint32_t vt_vt1682_state::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	return 0;
}

void vt_vt1682_state::vt_vt1682_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();

	/* Video */
	map(0x2000, 0x2000).rw(FUNC(vt_vt1682_state::vt1682_2000_r), FUNC(vt_vt1682_state::vt1682_2000_w));

	/* System */
	map(0x2100, 0x2100).rw(FUNC(vt_vt1682_state::vt1682_2100_prgbank1_r3_r), FUNC(vt_vt1682_state::vt1682_2100_prgbank1_r3_w));

	map(0x2105, 0x2105).w(FUNC(vt_vt1682_state::vt1682_2105_comr6_tvmodes_w));

	map(0x2107, 0x2107).rw(FUNC(vt_vt1682_state::vt1682_2107_prgbank0_r0_r), FUNC(vt_vt1682_state::vt1682_2107_prgbank0_r0_w));
	map(0x2108, 0x2108).rw(FUNC(vt_vt1682_state::vt1682_2108_prgbank0_r1_r), FUNC(vt_vt1682_state::vt1682_2108_prgbank0_r1_w));
	map(0x2109, 0x2109).rw(FUNC(vt_vt1682_state::vt1682_2109_prgbank0_r2_r), FUNC(vt_vt1682_state::vt1682_2109_prgbank0_r2_w));
	map(0x210a, 0x210a).rw(FUNC(vt_vt1682_state::vt1682_210a_prgbank0_r3_r), FUNC(vt_vt1682_state::vt1682_210a_prgbank0_r3_w));
	map(0x210b, 0x210b).rw(FUNC(vt_vt1682_state::vt1682_210b_misc_cs_prg0_bank_sel_r), FUNC(vt_vt1682_state::vt1682_210b_misc_cs_prg0_bank_sel_w));
	map(0x210c, 0x210c).rw(FUNC(vt_vt1682_state::vt1682_210c_prgbank1_r2_r), FUNC(vt_vt1682_state::vt1682_210c_prgbank1_r2_w));

	// either reads/writes are on different addresses or our source info is incorrect
	map(0x2110, 0x2110).rw(FUNC(vt_vt1682_state::vt1682_prgbank0_r4_r), FUNC(vt_vt1682_state::vt1682_prgbank1_r0_w));
	map(0x2111, 0x2111).rw(FUNC(vt_vt1682_state::vt1682_prgbank0_r5_r), FUNC(vt_vt1682_state::vt1682_prgbank1_r1_w));
	map(0x2112, 0x2112).rw(FUNC(vt_vt1682_state::vt1682_prgbank1_r0_r), FUNC(vt_vt1682_state::vt1682_prgbank0_r4_w));
	map(0x2113, 0x2113).rw(FUNC(vt_vt1682_state::vt1682_prgbank1_r1_r), FUNC(vt_vt1682_state::vt1682_prgbank0_r5_w));

	map(0x2118, 0x2118).rw(FUNC(vt_vt1682_state::vt1682_2118_prgbank1_r4_r5_r), FUNC(vt_vt1682_state::vt1682_2118_prgbank1_r4_r5_w));

	map(0x211c, 0x211c).w(FUNC(vt_vt1682_state::vt1682_211c_regs_ext2421_w));

	// 3000-3fff internal ROM if enabled
	map(0x4000, 0xffff).rom().region("mainrom", 0x74000);
}

static INPUT_PORTS_START( intec )
INPUT_PORTS_END

void vt_vt1682_state::vt_vt1682(machine_config &config)
{
	/* basic machine hardware */
	M6502_VT1682(config, m_maincpu, 5000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt_vt1682_state::vt_vt1682_map);

	// 6502 sound CPU running at 20mhz

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-1);
	m_screen->set_screen_update(FUNC(vt_vt1682_state::screen_update));
}

ROM_START( ii8in1 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "ii8in1.bin", 0x00000, 0x2000000, CRC(7aee7464) SHA1(7a9cf7f54a350f0853a17459f2dcbef34f4f7c30) ) // 2ND HALF EMPTY

	// possible undumped 0x1000 bytes of Internal ROM
ROM_END

ROM_START( ii32in1 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "ii32in1.bin", 0x00000, 0x2000000, CRC(ddee4eac) SHA1(828c0c18a66bb4872299f9a43d5e3647482c5925) )

	// possible undumped 0x1000 bytes of Internal ROM
ROM_END

CONS( 200?, ii8in1,    0,  0,  vt_vt1682,    intec, vt_vt1682_state, empty_init, "Intec", "InterAct 8-in-1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 200?, ii32in1,   0,  0,  vt_vt1682,    intec, vt_vt1682_state, empty_init, "Intec", "InterAct 32-in-1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
