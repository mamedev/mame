// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Ash Wolf
/***************************************************************************

        Psion 5mx (EPOC R5) series

        Skeleton driver by Ryan Holtz, ported from work by Ash Wolf

        TODO:
        - everything

        More info:
            https://github.com/Treeki/WindEmu

****************************************************************************/


#include "emu.h"
#include "cpu/arm7/arm7.h"

#include "screen.h"

class psion5_state : public driver_device
{
public:
	psion5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void psion5mx(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	DECLARE_READ32_MEMBER(periphs_r);
	DECLARE_WRITE32_MEMBER(periphs_w);

	enum
	{
		REG_MEMCFG1   = 0x0000,
		REG_MEMCFG2   = 0x0004,

		REG_DRAMCFG   = 0x0100,

		REG_LCDCTL    = 0x0200,
		REG_LCDST     = 0x0204,
		REG_LCD_DBAR1 = 0x0210,
		REG_LCDT0     = 0x0220,
		REG_LCDT1     = 0x0224,
		REG_LCDT2     = 0x0228,

		REG_PWRSR     = 0x0400,
		REG_PWRCNT    = 0x0404,
		REG_HALT      = 0x0408,
		REG_STBY      = 0x040c,
		REG_BLEOI     = 0x0410,
		REG_MCEOI     = 0x0414,
		REG_TEOI      = 0x0418,
		REG_STFCLR    = 0x041c,
		REG_E2EOI     = 0x0420,

		REG_INTSR     = 0x0500,
		REG_INTRSR    = 0x0504,
		REG_INTENS    = 0x0508,
		REG_INTENC    = 0x050c,
		REG_INTTEST1  = 0x0514,
		REG_INTTEST2  = 0x0518,

		REG_PUMPCON   = 0x0900,

		REG_CODR      = 0x0a00,
		REG_CONFG     = 0x0a04,
		REG_COLFG     = 0x0a08,
		REG_COEOI     = 0x0a0c,
		REG_COTEST    = 0x0a10,

		REG_SSCR0     = 0x0b00,
		REG_SSCR1     = 0x0b04,
		REG_SSDR      = 0x0b0c,
		REG_SSSR      = 0x0b14,

		REG_TC1LOAD   = 0x0c00,
		REG_TC1VAL    = 0x0c04,
		REG_TC1CTRL   = 0x0c08,
		REG_TC1EOI    = 0x0c0c,
		REG_TC2LOAD   = 0x0c20,
		REG_TC2VAL    = 0x0c24,
		REG_TC2CTRL   = 0x0c28,
		REG_TC2EOI    = 0x0c2c,

		REG_BZCONT    = 0x0c40,

		REG_RTCDRL    = 0x0d00,
		REG_RTCDRU    = 0x0d04,
		REG_RTCMRL    = 0x0d08,
		REG_RTCMRU    = 0x0d0c,
		REG_RTCEOI    = 0x0d10,

		REG_PADR      = 0x0e00,
		REG_PBDR      = 0x0e04,
		REG_PCDR      = 0x0e08,
		REG_PDDR      = 0x0e0c,
		REG_PADDR     = 0x0e10,
		REG_PBDDR     = 0x0e14,
		REG_PCDDR     = 0x0e18,
		REG_PDDDR     = 0x0e1c,
		REG_PEDR      = 0x0e20,
		REG_PEDDR     = 0x0e24,

		REG_KSCAN     = 0x0e28,
		REG_LCDMUX    = 0x0e2c
	};

	void main_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<arm710t_cpu_device> m_maincpu;
};

void psion5_state::machine_start()
{
}

void psion5_state::machine_reset()
{
}

READ32_MEMBER(psion5_state::periphs_r)
{
	const uint32_t reg = offset << 2;
	uint32_t data = 0;
	switch (reg)
	{
		case REG_MEMCFG1:
			logerror("%s: peripheral read, MEMCFG1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_MEMCFG2:
			logerror("%s: peripheral read, MEMCFG2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_DRAMCFG:
			logerror("%s: peripheral read, DRAMCFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_LCDCTL:
			logerror("%s: peripheral read, LCDCTL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDST:
			logerror("%s: peripheral read, LCDST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCD_DBAR1:
			logerror("%s: peripheral read, LCD_DBAR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDT0:
			logerror("%s: peripheral read, LCDT0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDT1:
			logerror("%s: peripheral read, LCDT1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDT2:
			logerror("%s: peripheral read, LCDT2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_PWRSR:
			logerror("%s: peripheral read, PWRSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PWRCNT:
			logerror("%s: peripheral read, PWRCNT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_HALT:
			logerror("%s: peripheral read, HALT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_STBY:
			logerror("%s: peripheral read, STBY = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_BLEOI:
			logerror("%s: peripheral read, BLEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_MCEOI:
			logerror("%s: peripheral read, MCEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TEOI:
			logerror("%s: peripheral read, TEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_STFCLR:
			logerror("%s: peripheral read, STFCLR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_E2EOI:
			logerror("%s: peripheral read, E2EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_INTSR:
			logerror("%s: peripheral read, INTSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTRSR:
			logerror("%s: peripheral read, INTRSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTENS:
			logerror("%s: peripheral read, INTENS = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTENC:
			logerror("%s: peripheral read, INTENC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTTEST1:
			logerror("%s: peripheral read, INTTEST1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTTEST2:
			logerror("%s: peripheral read, INTTEST2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_PUMPCON:
			logerror("%s: peripheral read, PUMPCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_CODR:
			logerror("%s: peripheral read, CODR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_CONFG:
			logerror("%s: peripheral read, CONFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_COLFG:
			logerror("%s: peripheral read, COLFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_COEOI:
			logerror("%s: peripheral read, COEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_COTEST:
			logerror("%s: peripheral read, COTEST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_SSCR0:
			logerror("%s: peripheral read, SSCR0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_SSCR1:
			logerror("%s: peripheral read, SSCR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_SSDR:
			logerror("%s: peripheral read, SSDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_SSSR:
			logerror("%s: peripheral read, SSSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_TC1LOAD:
			logerror("%s: peripheral read, TC1LOAD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC1VAL:
			logerror("%s: peripheral read, TC1VAL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC1CTRL:
			logerror("%s: peripheral read, TC1CTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC1EOI:
			logerror("%s: peripheral read, TC1EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC2LOAD:
			logerror("%s: peripheral read, TC2LOAD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC2VAL:
			logerror("%s: peripheral read, TC2VAL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC2CTRL:
			logerror("%s: peripheral read, TC2CTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC2EOI:
			logerror("%s: peripheral read, TC2EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_BZCONT:
			logerror("%s: peripheral read, BZCONT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_RTCDRL:
			logerror("%s: peripheral read, RTCDRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCDRU:
			logerror("%s: peripheral read, RTCDRU = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCMRL:
			logerror("%s: peripheral read, RTCMRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCMRU:
			logerror("%s: peripheral read, RTCMRU = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCEOI:
			logerror("%s: peripheral read, RTCEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_PADR:
			logerror("%s: peripheral read, PADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PBDR:
			logerror("%s: peripheral read, PBDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PCDR:
			logerror("%s: peripheral read, PCDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PDDR:
			logerror("%s: peripheral read, PDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PADDR:
			logerror("%s: peripheral read, PADDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PBDDR:
			logerror("%s: peripheral read, PBDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PCDDR:
			logerror("%s: peripheral read, PCDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PDDDR:
			logerror("%s: peripheral read, PDDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PEDR:
			logerror("%s: peripheral read, PEDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PEDDR:
			logerror("%s: peripheral read, PEDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_KSCAN:
			logerror("%s: peripheral read, KSCAN = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDMUX:
			logerror("%s: peripheral read, LCDMUX = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		default:
			logerror("%s: peripheral read, Unknown = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
	}
	return 0;
}

WRITE32_MEMBER(psion5_state::periphs_w)
{
	const uint32_t reg = offset << 2;
	switch (reg)
	{
		case REG_MEMCFG1:
			logerror("%s: peripheral write, MEMCFG1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_MEMCFG2:
			logerror("%s: peripheral write, MEMCFG2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_DRAMCFG:
			logerror("%s: peripheral write, DRAMCFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_LCDCTL:
			logerror("%s: peripheral write, LCDCTL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDST:
			logerror("%s: peripheral write, LCDST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCD_DBAR1:
			logerror("%s: peripheral write, LCD_DBAR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDT0:
			logerror("%s: peripheral write, LCDT0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDT1:
			logerror("%s: peripheral write, LCDT1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDT2:
			logerror("%s: peripheral write, LCDT2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_PWRSR:
			logerror("%s: peripheral write, PWRSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PWRCNT:
			logerror("%s: peripheral write, PWRCNT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_HALT:
			logerror("%s: peripheral write, HALT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_STBY:
			logerror("%s: peripheral write, STBY = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_BLEOI:
			logerror("%s: peripheral write, BLEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_MCEOI:
			logerror("%s: peripheral write, MCEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TEOI:
			logerror("%s: peripheral write, TEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_STFCLR:
			logerror("%s: peripheral write, STFCLR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_E2EOI:
			logerror("%s: peripheral write, E2EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_INTSR:
			logerror("%s: peripheral write, INTSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTRSR:
			logerror("%s: peripheral write, INTRSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTENS:
			logerror("%s: peripheral write, INTENS = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTENC:
			logerror("%s: peripheral write, INTENC = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTTEST1:
			logerror("%s: peripheral write, INTTEST1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_INTTEST2:
			logerror("%s: peripheral write, INTTEST2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_PUMPCON:
			logerror("%s: peripheral write, PUMPCON = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_CODR:
			logerror("%s: peripheral write, CODR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_CONFG:
			logerror("%s: peripheral write, CONFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_COLFG:
			logerror("%s: peripheral write, COLFG = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_COEOI:
			logerror("%s: peripheral write, COEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_COTEST:
			logerror("%s: peripheral write, COTEST = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_SSCR0:
			logerror("%s: peripheral write, SSCR0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_SSCR1:
			logerror("%s: peripheral write, SSCR1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_SSDR:
			logerror("%s: peripheral write, SSDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_SSSR:
			logerror("%s: peripheral write, SSSR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_TC1LOAD:
			logerror("%s: peripheral write, TC1LOAD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC1VAL:
			logerror("%s: peripheral write, TC1VAL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC1CTRL:
			logerror("%s: peripheral write, TC1CTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC1EOI:
			logerror("%s: peripheral write, TC1EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC2LOAD:
			logerror("%s: peripheral write, TC2LOAD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC2VAL:
			logerror("%s: peripheral write, TC2VAL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC2CTRL:
			logerror("%s: peripheral write, TC2CTRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_TC2EOI:
			logerror("%s: peripheral write, TC2EOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_BZCONT:
			logerror("%s: peripheral write, BZCONT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_RTCDRL:
			logerror("%s: peripheral write, RTCDRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCDRU:
			logerror("%s: peripheral write, RTCDRU = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCMRL:
			logerror("%s: peripheral write, RTCMRL = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCMRU:
			logerror("%s: peripheral write, RTCMRU = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_RTCEOI:
			logerror("%s: peripheral write, RTCEOI = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_PADR:
			logerror("%s: peripheral write, PADR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PBDR:
			logerror("%s: peripheral write, PBDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PCDR:
			logerror("%s: peripheral write, PCDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PDDR:
			logerror("%s: peripheral write, PDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PADDR:
			logerror("%s: peripheral write, PADDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PBDDR:
			logerror("%s: peripheral write, PBDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PCDDR:
			logerror("%s: peripheral write, PCDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PDDDR:
			logerror("%s: peripheral write, PDDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PEDR:
			logerror("%s: peripheral write, PEDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_PEDDR:
			logerror("%s: peripheral write, PEDDR = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case REG_KSCAN:
			logerror("%s: peripheral write, KSCAN = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case REG_LCDMUX:
			logerror("%s: peripheral write, LCDMUX = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		default:
			logerror("%s: peripheral write, Unknown = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
	}
}

void psion5_state::main_map(address_map &map)
{
	map(0x00000000, 0x009fffff).rom().region("maincpu", 0);
	map(0x80000000, 0x80000fff).rw(FUNC(psion5_state::periphs_r), FUNC(psion5_state::periphs_w));
	map(0xc0000000, 0xc03fffff).ram().mirror(0x1fc00000);
}

uint32_t psion5_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/* Input ports */
INPUT_PORTS_START( psion5 )
INPUT_PORTS_END

/* basic configuration for 2 lines display */
void psion5_state::psion5mx(machine_config &config)
{
	/* basic machine hardware */
	ARM710T(config, m_maincpu, 18000000); // 18MHz, per wikipedia
	m_maincpu->set_addrmap(AS_PROGRAM, &psion5_state::main_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0)); /* not accurate */
	screen.set_screen_update(FUNC(psion5_state::screen_update));
	screen.set_size(640, 240);
	screen.set_visarea(0, 640-1, 0, 240-1);
}

/* ROM definition */

ROM_START( psion5mx )
	ROM_REGION( 0xa00000, "maincpu", 0 )
	ROM_LOAD( "5mx.rom", 0x000000, 0xa00000, CRC(a1e2d038) SHA1(4c082321264e1ae7fe77699e59b8960460690fa6) )
ROM_END

/* Driver */

//    YEAR  NAME        PARENT   COMPAT  MACHINE    INPUT   CLASS         INIT        COMPANY  FULLNAME  FLAGS
COMP( 1999, psion5mx,   0,       0,      psion5mx,  psion5, psion5_state, empty_init, "Psion", "5mx",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
