// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Modern device for the MCF5206e Peripherals
 this can be hooked properly to the CPU once the CPU is a modern device too
*/

#include "emu.h"
#include "mcf5206e.h"

static void CLIB_DECL ATTR_PRINTF(1,2) nolog(const char *format, ...) {}

//#define debuglog printf
#define debuglog logerror

#define invalidlog printf
//#define invalidlog logerror

//#define debuglogtimer printf
//#define debuglogtimer logerror
#define debuglogtimer nolog

static ADDRESS_MAP_START( coldfire_regs_map, AS_0, 32, mcf5206e_peripheral_device )


	AM_RANGE(0x014, 0x017) AM_READWRITE8(ICR1_ICR2_ICR3_ICR4_r,    ICR1_ICR2_ICR3_ICR4_w,    0xffffffff)

	AM_RANGE(0x01c, 0x01f) AM_READWRITE8(ICR9_ICR10_ICR11_ICR12_r, ICR9_ICR10_ICR11_ICR12_w, 0xffffffff)
	AM_RANGE(0x020, 0x023) AM_READWRITE8(ICR13_r,                  ICR13_w,                  0xffffffff)

	AM_RANGE(0x034, 0x037) AM_READWRITE16(IMR_r, IMR_w, 0xffffffff)

	/* Chip Select registers */
	AM_RANGE(0x064, 0x067) AM_READWRITE16(CSAR0_r, CSAR0_w, 0xffffffff)
	AM_RANGE(0x068, 0x06b) AM_READWRITE  (CSMR0_r, CSMR0_w)
	AM_RANGE(0x06c, 0x06f) AM_READWRITE16(CSCR0_r, CSCR0_w, 0xffffffff)
	AM_RANGE(0x070, 0x073) AM_READWRITE16(CSAR1_r, CSAR1_w, 0xffffffff)
	AM_RANGE(0x074, 0x077) AM_READWRITE  (CSMR1_r, CSMR1_w)
	AM_RANGE(0x078, 0x07b) AM_READWRITE16(CSCR1_r, CSCR1_w, 0xffffffff)
	AM_RANGE(0x07c, 0x07f) AM_READWRITE16(CSAR2_r, CSAR2_w, 0xffffffff)
	AM_RANGE(0x080, 0x083) AM_READWRITE  (CSMR2_r, CSMR2_w)
	AM_RANGE(0x084, 0x087) AM_READWRITE16(CSCR2_r, CSCR2_w, 0xffffffff)
	AM_RANGE(0x088, 0x08b) AM_READWRITE16(CSAR3_r, CSAR3_w, 0xffffffff)
	AM_RANGE(0x08c, 0x08f) AM_READWRITE  (CSMR3_r, CSMR3_w)
	AM_RANGE(0x090, 0x093) AM_READWRITE16(CSCR3_r, CSCR3_w, 0xffffffff)
	AM_RANGE(0x094, 0x097) AM_READWRITE16(CSAR4_r, CSAR4_w, 0xffffffff)
	AM_RANGE(0x098, 0x09b) AM_READWRITE  (CSMR4_r, CSMR4_w)
	AM_RANGE(0x09c, 0x09f) AM_READWRITE16(CSCR4_r, CSCR4_w, 0xffffffff)
	AM_RANGE(0x0a0, 0x0a3) AM_READWRITE16(CSAR5_r, CSAR5_w, 0xffffffff)
	AM_RANGE(0x0a4, 0x0a7) AM_READWRITE  (CSMR5_r, CSMR5_w)
	AM_RANGE(0x0a8, 0x0ab) AM_READWRITE16(CSCR5_r, CSCR5_w, 0xffffffff)
	AM_RANGE(0x0ac, 0x0af) AM_READWRITE16(CSAR6_r, CSAR6_w, 0xffffffff)
	AM_RANGE(0x0b0, 0x0b3) AM_READWRITE  (CSMR6_r, CSMR6_w)
	AM_RANGE(0x0b4, 0x0b7) AM_READWRITE16(CSCR6_r, CSCR6_w, 0xffffffff)
	AM_RANGE(0x0b8, 0x0bb) AM_READWRITE16(CSAR7_r, CSAR7_w, 0xffffffff)
	AM_RANGE(0x0bc, 0x0bf) AM_READWRITE  (CSMR7_r, CSMR7_w)
	AM_RANGE(0x0c0, 0x0c3) AM_READWRITE16(CSCR7_r, CSCR7_w, 0xffffffff)

	AM_RANGE(0x0c4, 0x0c7) AM_READWRITE16(DMCR_r, DMCR_w, 0xffffffff)
	AM_RANGE(0x0c8, 0x0cb) AM_READWRITE16(PAR_r, PAR_w, 0xffffffff)

	AM_RANGE(0x100, 0x103) AM_READWRITE16(TMR1_r, TMR1_w, 0xffffffff)
	AM_RANGE(0x104, 0x107) AM_READWRITE16(TRR1_r, TRR1_w, 0xffffffff)
	AM_RANGE(0x10c, 0x10f) AM_READWRITE16(TCN1_r, TCN1_w, 0xffffffff)
	AM_RANGE(0x110, 0x113) AM_READWRITE8(TER1_r, TER1_w, 0xffffffff)


	AM_RANGE(0x1c4, 0x1c7) AM_READWRITE8(PPDDR_r, PPDDR_w, 0xffffffff)
	AM_RANGE(0x1c8, 0x1cb) AM_READWRITE8(PPDAT_r, PPDAT_w, 0xffffffff)

	AM_RANGE(0x1e4, 0x1e7) AM_READWRITE8(MFDR_r, MFDR_w, 0xffffffff)
	AM_RANGE(0x1e8, 0x1eb) AM_READWRITE8(MBCR_r, MBCR_w, 0xffffffff)
	AM_RANGE(0x1ec, 0x1ef) AM_READWRITE8(MBSR_r, MBSR_w, 0xffffffff)
	AM_RANGE(0x1f0, 0x1f3) AM_READWRITE8(MBDR_r, MBDR_w, 0xffffffff)
ADDRESS_MAP_END



READ8_MEMBER( mcf5206e_peripheral_device::ICR1_ICR2_ICR3_ICR4_r )
{
	switch (offset)
	{
	case 0: // 0x014
		debuglog("%s: (External IRQ1/IPL1 Interrupt Vector) ICR1_r\n", this->machine().describe_context());
		return m_ICR[ICR1];
	case 1: // 0x015
		debuglog("%s: (External IPL2 Interrupt Vector) ICR2_r\n", this->machine().describe_context());
		return m_ICR[ICR2];
	case 2: // 0x016
		debuglog("%s: (External IPL3 Interrupt Vector) ICR3_r\n", this->machine().describe_context());
		return m_ICR[ICR3];
	case 3: // 0x017
		debuglog("%s: (External IRQ4/IPL4 Interrupt Vector) ICR4_r\n", this->machine().describe_context());
		return m_ICR[ICR4];
	}

	return 0;
}

WRITE8_MEMBER( mcf5206e_peripheral_device::ICR1_ICR2_ICR3_ICR4_w )
{
	switch (offset)
	{
	case 0: // 0x014
		m_ICR[ICR1] = data;
		debuglog("%s: (External IRQ1/IPL1 Interrupt Vector) ICR1_w %02x\n", this->machine().describe_context(), data);
		ICR_info(m_ICR[ICR1]);
		break;
	case 1: // 0x015
		m_ICR[ICR2] = data;
		debuglog("%s: (External IPL2 Interrupt Vector) ICR2_w %02x\n", this->machine().describe_context(), data);
		ICR_info(m_ICR[ICR2]);
		break;
	case 2: // 0x016
		m_ICR[ICR3] = data;
		debuglog("%s: (External IPL3 Interrupt Vector) ICR3_w %02x\n", this->machine().describe_context(), data);
		ICR_info(m_ICR[ICR3]);
		break;
	case 3: // 0x017
		m_ICR[ICR4] = data;
		debuglog("%s: (External IRQ4/IPL4 Interrupt Vector) ICR4_w %02x\n", this->machine().describe_context(), data);
		ICR_info(m_ICR[ICR4]);
		break;
	}
}

READ8_MEMBER( mcf5206e_peripheral_device::ICR9_ICR10_ICR11_ICR12_r )
{
	switch (offset)
	{
	case 0: // 0x01c
		debuglog("%s: (Timer 1 Interrupt Vector) ICR9_r\n", this->machine().describe_context());
		return m_ICR[ICR9];
	case 1: // 0x01d
		debuglog("%s: (Timer 2 Interrupt Vector) ICR10_r\n", this->machine().describe_context());
		return m_ICR[ICR10];
	case 2: // 0x01e
		debuglog("%s: (MBUS Interrupt Vector) ICR11_r\n", this->machine().describe_context());
		return m_ICR[ICR11];
	case 3: // 0x01f
		debuglog("%s: (UART1 Interrupt Vector) ICR12_r\n", this->machine().describe_context());
		return m_ICR[ICR12];
	}

	return 0;
}

WRITE8_MEMBER( mcf5206e_peripheral_device::ICR9_ICR10_ICR11_ICR12_w )
{
	switch (offset)
	{
	case 0: // 0x01c
		m_ICR[ICR9] = data;
		debuglog("%s: (Timer 1 Interrupt Vector) ICR9_w %02x\n", this->machine().describe_context(), data);
		ICR_info(m_ICR[ICR9]);
		break;
	case 1: // 0x01d
		m_ICR[ICR10] = data;
		debuglog("%s: (Timer 2 Interrupt Vector) ICR10_w %02x\n", this->machine().describe_context(), data);
		ICR_info(m_ICR[ICR10]);
		break;
	case 2: // 0x01e
		m_ICR[ICR11] = data;
		debuglog("%s: (MBUS Interrupt Vector) ICR11_w %02x\n", this->machine().describe_context(), data);
		ICR_info(m_ICR[ICR11]);
		break;
	case 3: // 0x01f
		m_ICR[ICR12] = data;
		debuglog("%s: (UART1 Interrupt Vector) ICR12_w %02x\n", this->machine().describe_context(), data);
		ICR_info(m_ICR[ICR12]);
		break;
	}
}

READ8_MEMBER( mcf5206e_peripheral_device::ICR13_r )
{
	switch (offset)
	{
	case 0: // 0x020
		debuglog("%s: (UART2 Interrupt Vector) ICR13_r\n", this->machine().describe_context());
		return m_ICR[ICR13];
	case 1:
	case 2:
	case 3:
		invalidlog("%s: invalid ICR13_r %d\n", this->machine().describe_context(), offset);
		return 0;
	}

	return 0;
}

WRITE8_MEMBER( mcf5206e_peripheral_device::ICR13_w )
{
	switch (offset)
	{
	case 0: // 0x020
		m_ICR[ICR13] = data;
		debuglog("%s: (UART2 Interrupt Vector) ICR13_w %02x\n", this->machine().describe_context(), data);
		ICR_info(m_ICR[ICR13]);
		break;
	case 1:
	case 2:
	case 3:
		invalidlog("%s: invalid ICR13_w %d, %02x\n", this->machine().describe_context(), offset, data);
		break;
	}
}


inline UINT16 mcf5206e_peripheral_device::CSAR_r(int which, int offset, UINT16 mem_mask)
{
	if (offset==0)
	{
		debuglog("%s: CSAR%d_r\n", this->machine().describe_context(), which);
		return m_CSAR[which];
	}
	else
	{
		invalidlog("%s: invalid CSAR%d_r with offset %d\n", this->machine().describe_context(), which, offset);
		return 0;
	}
}

inline void mcf5206e_peripheral_device::CSAR_w(int which, int offset, UINT16 data, UINT16 mem_mask)
{
	if (offset==0)
	{
		COMBINE_DATA( &m_CSAR[which] );
		debuglog("%s: CSAR%d_w %04x\n", this->machine().describe_context(), which, data);
	}
	else
	{
		invalidlog("%s: invalid CSAR%d_w with offset %d %04x\n", this->machine().describe_context(), which, offset, data);
	}
}

inline UINT32 mcf5206e_peripheral_device::CSMR_r(int which, UINT32 mem_mask)
{
	debuglog("%s: CSMR%d_r\n", this->machine().describe_context(), which);
	return m_CSMR[0];
}

inline void mcf5206e_peripheral_device::CSMR_w(int which, UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA( &m_CSMR[0] );
	debuglog("%s: CSMR%d_w %08x\n", this->machine().describe_context(), which, data);
}

inline UINT16 mcf5206e_peripheral_device::CSCR_r(int which, int offset, UINT16 mem_mask)
{
	if (offset==1)
	{
		debuglog("%s: CSCR%d_r\n", this->machine().describe_context(), which);
		return m_CSCR[which];
	}
	else
	{
		invalidlog("%s: invalid CSCR%d_r with offset %d\n", this->machine().describe_context(), which, offset);
		return 0;
	}
}

inline void mcf5206e_peripheral_device::CSCR_w(int which, int offset, UINT16 data, UINT16 mem_mask)
{
	if (offset==1)
	{
		COMBINE_DATA( &m_CSCR[which] );
		debuglog("%s: CSCR%d_w %04x\n", this->machine().describe_context(), which, data);
	}
	else
	{
		invalidlog("%s: invalid CSCR%d_r with offset %d %04x\n", this->machine().describe_context(), which, offset, data);
	}
}




READ16_MEMBER(  mcf5206e_peripheral_device::CSAR0_r) { return CSAR_r(0, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSAR0_w) {        CSAR_w(0, offset, data, mem_mask); }
READ32_MEMBER(  mcf5206e_peripheral_device::CSMR0_r) { return CSMR_r(0, mem_mask); }
WRITE32_MEMBER( mcf5206e_peripheral_device::CSMR0_w) {        CSMR_w(0, data, mem_mask); }
READ16_MEMBER(  mcf5206e_peripheral_device::CSCR0_r) { return CSCR_r(0, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSCR0_w) {        CSCR_w(0, offset, data, mem_mask); }

READ16_MEMBER(  mcf5206e_peripheral_device::CSAR1_r) { return CSAR_r(1, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSAR1_w) {        CSAR_w(1, offset, data, mem_mask); }
READ32_MEMBER(  mcf5206e_peripheral_device::CSMR1_r) { return CSMR_r(1, mem_mask); }
WRITE32_MEMBER( mcf5206e_peripheral_device::CSMR1_w) {        CSMR_w(1, data, mem_mask); }
READ16_MEMBER(  mcf5206e_peripheral_device::CSCR1_r) { return CSCR_r(1, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSCR1_w) {        CSCR_w(1, offset, data, mem_mask); }

READ16_MEMBER(  mcf5206e_peripheral_device::CSAR2_r) { return CSAR_r(2, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSAR2_w) {        CSAR_w(2, offset, data, mem_mask); }
READ32_MEMBER(  mcf5206e_peripheral_device::CSMR2_r) { return CSMR_r(2, mem_mask); }
WRITE32_MEMBER( mcf5206e_peripheral_device::CSMR2_w) {        CSMR_w(2, data, mem_mask); }
READ16_MEMBER(  mcf5206e_peripheral_device::CSCR2_r) { return CSCR_r(2, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSCR2_w) {        CSCR_w(2, offset, data, mem_mask); }

READ16_MEMBER(  mcf5206e_peripheral_device::CSAR3_r) { return CSAR_r(3, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSAR3_w) {        CSAR_w(3, offset, data, mem_mask); }
READ32_MEMBER(  mcf5206e_peripheral_device::CSMR3_r) { return CSMR_r(3, mem_mask); }
WRITE32_MEMBER( mcf5206e_peripheral_device::CSMR3_w) {        CSMR_w(3, data, mem_mask); }
READ16_MEMBER(  mcf5206e_peripheral_device::CSCR3_r) { return CSCR_r(3, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSCR3_w) {        CSCR_w(3, offset, data, mem_mask); }

READ16_MEMBER(  mcf5206e_peripheral_device::CSAR4_r) { return CSAR_r(4, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSAR4_w) {        CSAR_w(4, offset, data, mem_mask); }
READ32_MEMBER(  mcf5206e_peripheral_device::CSMR4_r) { return CSMR_r(4, mem_mask); }
WRITE32_MEMBER( mcf5206e_peripheral_device::CSMR4_w) {        CSMR_w(4, data, mem_mask); }
READ16_MEMBER(  mcf5206e_peripheral_device::CSCR4_r) { return CSCR_r(4, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSCR4_w) {        CSCR_w(4, offset, data, mem_mask); }

READ16_MEMBER(  mcf5206e_peripheral_device::CSAR5_r) { return CSAR_r(5, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSAR5_w) {        CSAR_w(5, offset, data, mem_mask); }
READ32_MEMBER(  mcf5206e_peripheral_device::CSMR5_r) { return CSMR_r(5, mem_mask); }
WRITE32_MEMBER( mcf5206e_peripheral_device::CSMR5_w) {        CSMR_w(5, data, mem_mask); }
READ16_MEMBER(  mcf5206e_peripheral_device::CSCR5_r) { return CSCR_r(5, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSCR5_w) {        CSCR_w(5, offset, data, mem_mask); }

READ16_MEMBER(  mcf5206e_peripheral_device::CSAR6_r) { return CSAR_r(6, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSAR6_w) {        CSAR_w(6, offset, data, mem_mask); }
READ32_MEMBER(  mcf5206e_peripheral_device::CSMR6_r) { return CSMR_r(6, mem_mask); }
WRITE32_MEMBER( mcf5206e_peripheral_device::CSMR6_w) {        CSMR_w(6, data, mem_mask); }
READ16_MEMBER(  mcf5206e_peripheral_device::CSCR6_r) { return CSCR_r(6, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSCR6_w) {        CSCR_w(6, offset, data, mem_mask); }

READ16_MEMBER(  mcf5206e_peripheral_device::CSAR7_r) { return CSAR_r(7, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSAR7_w) {        CSAR_w(7, offset, data, mem_mask); }
READ32_MEMBER(  mcf5206e_peripheral_device::CSMR7_r) { return CSMR_r(7, mem_mask); }
WRITE32_MEMBER( mcf5206e_peripheral_device::CSMR7_w) {        CSMR_w(7, data, mem_mask); }
READ16_MEMBER(  mcf5206e_peripheral_device::CSCR7_r) { return CSCR_r(7, offset, mem_mask); }
WRITE16_MEMBER( mcf5206e_peripheral_device::CSCR7_w) {        CSCR_w(7, offset, data, mem_mask); }


READ16_MEMBER( mcf5206e_peripheral_device::DMCR_r)
{
	switch (offset)
	{
	case 1:
		debuglog("%s: DMCR_r %04x\n", this->machine().describe_context(), mem_mask);
		return m_DMCR;
	case 0:
		invalidlog("%s: invalid DMCR_r %d %04x\n", this->machine().describe_context(), offset, mem_mask);
		return 0;
	}

	return 0;
}

WRITE16_MEMBER( mcf5206e_peripheral_device::DMCR_w)
{
	switch (offset)
	{
	case 1:
		COMBINE_DATA(&m_DMCR);
		debuglog("%s: DMCR_w %04x %04x\n", this->machine().describe_context(), data, mem_mask);
		break;
	case 0:
		invalidlog("%s: invalid DMCR_w %d, %04x %04x\n", this->machine().describe_context(), offset, data, mem_mask);
		break;

	}
}


READ16_MEMBER( mcf5206e_peripheral_device::PAR_r)
{
	switch (offset)
	{
	case 1:
		debuglog("%s: PAR_r %04x\n", this->machine().describe_context(), mem_mask);
		return m_PAR;
	case 0:
		invalidlog("%s: invalid PAR_r %d %04x\n", this->machine().describe_context(), offset, mem_mask);
		return 0;
	}

	return 0;
}

WRITE16_MEMBER( mcf5206e_peripheral_device::PAR_w)
{
	switch (offset)
	{
	case 1:
		COMBINE_DATA(&m_PAR);
		debuglog("%s: PAR_w %04x %04x\n", this->machine().describe_context(), data, mem_mask);
		break;
	case 0:
		invalidlog("%s: invalid PAR_w %d, %04x %04x\n", this->machine().describe_context(), offset, data, mem_mask);
		break;

	}
}



READ8_MEMBER( mcf5206e_peripheral_device::PPDDR_r)
{
	switch (offset)
	{
	case 0:
	case 2:
	case 3:
		invalidlog("%s: invalid PPDDR_r %d\n", this->machine().describe_context(), offset);
		return 0;
	case 1: // '$1C5'
		debuglog("%s: (Port A Data Direction Register) PPDDR_r\n", this->machine().describe_context());
		return m_PPDDR;
	}

	return 0;
}

WRITE8_MEMBER( mcf5206e_peripheral_device::PPDDR_w)
{
	switch (offset)
	{
	case 0:
	case 2:
	case 3:
		invalidlog("%s: invalid PPDDR_w %d %02x\n", this->machine().describe_context(), offset, data);
		break;
	case 1: // '$1C5'
		m_PPDDR = data;
		debuglog("%s: (Port A Data Direction Register) PPDDR_w %02x\n", this->machine().describe_context(), data);
		break;
	}
}

READ8_MEMBER( mcf5206e_peripheral_device::PPDAT_r)
{
	switch (offset)
	{
	case 0:
	case 2:
	case 3:
		invalidlog("%s: invalid PPDAT_r %d\n", this->machine().describe_context(), offset);
		return 0;
	case 1: // '$1C9'
		debuglog("%s: (Port A Data Register) PPDAT_r\n", this->machine().describe_context());
		return m_PPDAT; // should use a callback.
	}

	return 0;
}

WRITE8_MEMBER( mcf5206e_peripheral_device::PPDAT_w)
{
	switch (offset)
	{
	case 0:
	case 2:
	case 3:
		invalidlog("%s: invalid PPDAT_w %d, %02x\n", this->machine().describe_context(), offset, data);
		break;
	case 1: // '$1C9'
		m_PPDAT = data; // should use a callback.
		debuglog("%s: (Port A Data Register) PPDAT_w %02x\n", this->machine().describe_context(), data);
		break;
	}

}


READ8_MEMBER( mcf5206e_peripheral_device::MBCR_r)
{
	switch (offset)
	{
	case 0:
		debuglog("%s: (M-Bus Control Register) MBCR_r\n", this->machine().describe_context());
		return m_MBCR;
	case 1:
	case 2:
	case 3:
		invalidlog("%s: invalid MBCR_r %d\n", this->machine().describe_context(), offset);
		return 0;
	}

	return 0;
}

WRITE8_MEMBER( mcf5206e_peripheral_device::MBCR_w)
{
	switch (offset)
	{
	case 0:
		m_MBCR = data;
		debuglog("%s: (M-Bus Control Register) MBCR_w %02x\n", this->machine().describe_context(), data);
		break;
	case 1:
	case 2:
	case 3:
		invalidlog("%s: invalid MBCR_w %d, %02x\n", this->machine().describe_context(), offset, data);
		break;

	}
}

READ8_MEMBER( mcf5206e_peripheral_device::MFDR_r)
{
	switch (offset)
	{
	case 0:
		debuglog("%s: (M-Bus Frequency Divider Register) MFDR_r\n", this->machine().describe_context());
		return m_MFDR;
	case 1:
	case 2:
	case 3:
		invalidlog("%s: invalid MFDR_r %d\n", this->machine().describe_context(), offset);
		return 0;
	}

	return 0;
}

WRITE8_MEMBER( mcf5206e_peripheral_device::MFDR_w)
{
	switch (offset)
	{
	case 0:
		m_MFDR = data;
		debuglog("%s: (M-Bus Frequency Divider Register) MFDR_w %02x\n", this->machine().describe_context(), data);
		break;
	case 1:
	case 2:
	case 3:
		invalidlog("%s: invalid MFDR_w %d, %02x\n", this->machine().describe_context(), offset, data);
		break;

	}
}






READ8_MEMBER( mcf5206e_peripheral_device::MBSR_r)
{
	int hack = 0x00;

	switch (offset)
	{
	case 0:
	{
		hack ^= (machine().rand()&0xff);
		debuglog("%s: (M-Bus Status Register) MBSR_r\n", this->machine().describe_context());
		return m_MBSR ^ hack; // will loop on this after a while
	}
	case 1:
	case 2:
	case 3:
		invalidlog("%s: invalid MBSR_r %d\n", this->machine().describe_context(), offset);
		return 0;
	}

	return 0;
}

WRITE8_MEMBER( mcf5206e_peripheral_device::MBSR_w)
{
	switch (offset)
	{
	case 0:
		m_MBSR = data;
		debuglog("%s: (M-Bus Status Register) MBSR_w %02x\n", this->machine().describe_context(), data);
		break;
	case 1:
	case 2:
	case 3:
		invalidlog("%s: invalid MBSR_w %d, %02x\n", this->machine().describe_context(), offset, data);
		break;

	}
}




READ8_MEMBER( mcf5206e_peripheral_device::MBDR_r)
{
	int hack = 0x00;

	switch (offset)
	{
	case 0:
	{
		hack ^= (machine().rand()&0xff);
		debuglog("%s: (M-Bus Data I/O Register) MBDR_r\n", this->machine().describe_context());
		return m_MBDR ^ hack;
	}
	case 1:
	case 2:
	case 3:
		invalidlog("%s: invalid MBDR_r %d\n", this->machine().describe_context(), offset);
		return 0;
	}

	return 0;
}

WRITE8_MEMBER( mcf5206e_peripheral_device::MBDR_w)
{
	switch (offset)
	{
	case 0:
		m_MBDR = data;
		debuglog("%s: (M-Bus Data I/O Register) MBDR_w %02x\n", this->machine().describe_context(), data);
		break;
	case 1:
	case 2:
	case 3:
		invalidlog("%s: invalid MBDR_w %d, %02x\n", this->machine().describe_context(), offset, data);
		break;

	}
}



READ16_MEMBER( mcf5206e_peripheral_device::IMR_r)
{
	switch (offset)
	{
	case 1:
		debuglog("%s: (Interrupt Mask Register) IMR_r %04x\n", this->machine().describe_context(), mem_mask);
		return m_IMR;
	case 0:
		invalidlog("%s: invalid IMR_r %d %04x\n", this->machine().describe_context(), offset, mem_mask);
		return 0;
	}

	return 0;
}

WRITE16_MEMBER( mcf5206e_peripheral_device::IMR_w)
{
	switch (offset)
	{
	case 1:
		COMBINE_DATA(&m_IMR);
		debuglog("%s: (Interrupt Mask Register) IMR_w %04x %04x\n", this->machine().describe_context(), data, mem_mask);
		break;
	case 0:
		invalidlog("%s: invalid IMR_w %d, %04x %04x\n", this->machine().describe_context(), offset, data, mem_mask);
		break;

	}
}

void mcf5206e_peripheral_device::ICR_info(UINT8 ICR)
{
	debuglog("  (AutoVector) AVEC : %01x | ", (ICR&0x80)>>7);
	debuglog("(Interrupt Level) IL : %01x | ", (ICR&0x1c)>>2); // if autovector (AVEC) is used then the vectors referenced are at +24 (+0x18) + IL, ie the standard 68k autovectors, otherwise vector must be provided by device
	debuglog("(Interrupt Priority) IP : %01x |", (ICR&0x03)>>0);
	debuglog("(Unused bits) : %01x\n", (ICR&0x60)>>5);
}



TIMER_CALLBACK_MEMBER(mcf5206e_peripheral_device::timer1_callback)
{
	UINT8 ICR = m_ICR[ICR9];

	// technically we should do the vector check in the IRQ callback as well as various checks based on the IRQ masks before asserting the interrupt
	if (ICR & 0x80) // AVEC
	{
		if (!(m_IMR & 0x0200)) m_cpu->set_input_line((ICR&0x1c)>>2, HOLD_LINE);
	}

	debuglogtimer("timer1_callback\n");
	m_TER1 |= 0x02;

	m_timer1->adjust(attotime::from_msec(10)); // completely made up value just to fire our timers for now
}


READ16_MEMBER( mcf5206e_peripheral_device::TMR1_r)
{
	switch (offset)
	{
	case 0:
		debuglogtimer("%s: (Timer 1 Mode Register) TMR1_r %04x\n", this->machine().describe_context(), mem_mask);
		return m_TMR1;
	case 1:
		invalidlog("%s: invalid TMR1_r %d %04x\n", this->machine().describe_context(), offset, mem_mask);
		return 0;
	}

	return 0;
}

WRITE16_MEMBER( mcf5206e_peripheral_device::TMR1_w)
{
	switch (offset)
	{
	case 0:
		COMBINE_DATA(&m_TMR1);
		debuglogtimer("%s: (Timer 1 Mode Register) TMR1_w %04x %04x\n", this->machine().describe_context(), data, mem_mask);

		debuglogtimer(" (Prescale) PS : %02x  (Capture Edge/Interrupt) CE : %01x (Output Mode) OM : %01x  (Output Reference Interrupt En) ORI : %01x   Free Run (FRR) : %01x  Input Clock Source (ICLK) : %01x  (Reset Timer) RST : %01x  \n", (m_TMR1 & 0xff00)>>8, (m_TMR1 & 0x00c0)>>6,  (m_TMR1 & 0x0020)>>5, (m_TMR1 & 0x0010)>>4, (m_TMR1 & 0x0008)>>3, (m_TMR1 & 0x0006)>>1, (m_TMR1 & 0x0001)>>0);

		if (m_TMR1 & 0x0001)
		{
			m_timer1->adjust(attotime::from_seconds(1)); // completely made up value just to fire our timers for now
		}
		else
		{
			m_timer1->adjust(attotime::never);
		}


		break;
	case 1:
		invalidlog("%s: invalid TMR1_w %d, %04x %04x\n", this->machine().describe_context(), offset, data, mem_mask);
		break;

	}
}

READ16_MEMBER( mcf5206e_peripheral_device::TRR1_r)
{
	switch (offset)
	{
	case 0:
		debuglogtimer("%s: (Timer 1 Reference Register) TRR1_r %04x\n", this->machine().describe_context(), mem_mask);
		return m_TRR1;
	case 1:
		invalidlog("%s: invalid TRR1_r %d %04x\n", this->machine().describe_context(), offset, mem_mask);
		return 0;
	}

	return 0;
}

WRITE16_MEMBER( mcf5206e_peripheral_device::TRR1_w)
{
	switch (offset)
	{
	case 0:
		COMBINE_DATA(&m_TRR1);
		debuglogtimer("%s: (Timer 1 Reference Register) TRR1_w %04x %04x\n", this->machine().describe_context(), data, mem_mask);
		break;
	case 1:
		debuglog("%s: invalid TRR1_w %d, %04x %04x\n", this->machine().describe_context(), offset, data, mem_mask);
		break;

	}
}



READ8_MEMBER( mcf5206e_peripheral_device::TER1_r)
{
	switch (offset)
	{
	case 1:
		debuglogtimer("%s: TER1_r\n", this->machine().describe_context());
		return m_TER1; // set on timer events, cleared by writing below
	case 0:
	case 2:
	case 3:
		invalidlog("%s: invalid TER1_r %d\n", this->machine().describe_context(), offset);
		return 0;
	}

	return 0;
}

WRITE8_MEMBER( mcf5206e_peripheral_device::TER1_w)
{
	switch (offset)
	{
	case 1:
		m_TER1 &= ~data; // writes should clear the bits..
		debuglogtimer("%s: TER1_w %02x\n", this->machine().describe_context(), data);
		break;
	case 0:
	case 2:
	case 3:
		invalidlog("%s: invalid TER1_w %d, %02x\n", this->machine().describe_context(), offset, data);
		break;

	}
}

READ16_MEMBER( mcf5206e_peripheral_device::TCN1_r)
{
	switch (offset)
	{
	case 0:
		debuglogtimer("%s: (Timer 1 Counter) TCN1_r %04x\n", this->machine().describe_context(), mem_mask);
		// return 0x9c40;
		return 0x8ca0 -1;// m_TCN1; // this should be the counter, code has a hardcoded >= check against 8ca0.
	case 1:
		invalidlog("%s: invalid TCN1_r %d %04x\n", this->machine().describe_context(), offset, mem_mask);
		return 0;
	}

	return 0;
}

WRITE16_MEMBER( mcf5206e_peripheral_device::TCN1_w)
{
	switch (offset)
	{
	case 0:
		COMBINE_DATA(&m_TCN1);
		debuglogtimer("%s: (Timer 1 Counter) TCN1_w %04x %04x\n", this->machine().describe_context(), data, mem_mask);
		break;
	case 1:
		invalidlog("%s: invalid TCN1_w %d, %04x %04x\n", this->machine().describe_context(), offset, data, mem_mask);
		break;

	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type MCF5206E_PERIPHERAL = &device_creator<mcf5206e_peripheral_device>;

//-------------------------------------------------
//  mcf5206e_peripheral_device - constructor
//-------------------------------------------------

mcf5206e_peripheral_device::mcf5206e_peripheral_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MCF5206E_PERIPHERAL, "MCF5206E Peripheral", tag, owner, clock, "mcf5206e_peripheral", __FILE__),
		device_memory_interface(mconfig, *this),
		m_space_config("coldfire_regs", ENDIANNESS_BIG, 32,10, 0, nullptr, *ADDRESS_MAP_NAME(coldfire_regs_map))

{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mcf5206e_peripheral_device::device_config_complete()
{
}

const address_space_config *mcf5206e_peripheral_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : nullptr;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mcf5206e_peripheral_device::device_start()
{
	init_regs(true);

	m_timer1 = machine().scheduler().timer_alloc( timer_expired_delegate( FUNC( mcf5206e_peripheral_device::timer1_callback ), this) );

	save_item(NAME(m_ICR));
	save_item(NAME(m_CSAR));
	save_item(NAME(m_CSMR));
	save_item(NAME(m_CSCR));
	save_item(NAME(m_DMCR));
	save_item(NAME(m_PAR));
	save_item(NAME(m_TMR1));
	save_item(NAME(m_TRR1));
	save_item(NAME(m_TER1));
	save_item(NAME(m_TCN1));
	save_item(NAME(m_PPDDR));
	save_item(NAME(m_PPDAT));
	save_item(NAME(m_IMR));
	save_item(NAME(m_MBCR));
	save_item(NAME(m_MBSR));
	save_item(NAME(m_MFDR));
	save_item(NAME(m_MBDR));
	save_item(NAME(m_coldfire_regs));
}

void mcf5206e_peripheral_device::device_reset()
{
	m_cpu = (cpu_device*)machine().device(":maincpu"); // hack. this device should really be attached to a modern CPU core

	init_regs(false);
	m_timer1->adjust(attotime::never);
}

READ32_MEMBER(mcf5206e_peripheral_device::dev_r)
{
	address_space &reg_space = this->space();
	return reg_space.read_dword(offset*4, mem_mask);
}

WRITE32_MEMBER(mcf5206e_peripheral_device::dev_w)
{
	address_space &reg_space = this->space();
	reg_space.write_dword(offset*4, data, mem_mask);
}


// ColdFire peripherals

enum {
	CF_PPDAT    =   0x1c8/4,
	CF_MBSR     =   0x1ec/4
};

WRITE32_MEMBER(mcf5206e_peripheral_device::seta2_coldfire_regs_w)
{
	COMBINE_DATA( &m_coldfire_regs[offset] );
}

READ32_MEMBER(mcf5206e_peripheral_device::seta2_coldfire_regs_r)
{
	switch( offset )
	{
		case CF_MBSR:
			return machine().rand();

		case CF_PPDAT:
			return ioport(":BATTERY")->read() << 16;
	}

	return m_coldfire_regs[offset];
}

#define UNINIT 0
#define UNINIT_NOTE 0

void mcf5206e_peripheral_device::init_regs(bool first_init)
{
	m_ICR[ICR1] =   0x04;
	m_ICR[ICR2] =   0x08;
	m_ICR[ICR3] =   0x0C;
	m_ICR[ICR4] =   0x10;
	m_ICR[ICR5] =   0x14;
	m_ICR[ICR6] =   0x18;
	m_ICR[ICR7] =   0x1C;
	m_ICR[ICR8] =   0x1C;
	m_ICR[ICR9] =   0x80;
	m_ICR[ICR10] =  0x80;
	m_ICR[ICR11] =  0x80;
	m_ICR[ICR12] =      0x00;
	m_ICR[ICR13] =  0x00;

	m_CSAR[0] = 0x0000;
	m_CSMR[0] = 0x00000000;
	m_CSCR[0] =   0x3C1F; /* 3C1F, 3C5F, 3C9F, 3CDF, 3D1F, 3D5F, 3D9F, 3DDF |  AA set by IRQ 7 at reset, PS1 set by IRQ 4 at reset, PS0 set by IRQ 1 at reset*/

	if (first_init)
	{
		for (int x=1;x<8;x++)
		{
			m_CSAR[1] = UNINIT;
			m_CSMR[1] = UNINIT;
			m_CSCR[1] = UNINIT_NOTE; // except BRST=ASET=WRAH=RDAH=WR=RD=0
		}
	}

	m_DMCR = 0x0000;
	m_PAR = 0x0000;

	m_TMR1 = 0x0000;
	m_TRR1 = 0xffff;
	m_TER1 = 0x00;
	m_TCN1 = 0x0000;

	m_PPDDR = 0x00;
	m_PPDAT = 0x00;

	m_IMR = 0x3FFE;

	m_MFDR = 0x00;
	m_MBCR = 0x00;
	m_MBSR = 0x00;
	m_MBDR = 0x00;
}

/*

ADDRESS (LE)            REG         WIDTH   NAME/DESCRIPTION                                    INIT VALUE (MR=Master Reset, NR=Normal Reset)       Read or Write access
* = inited
- = skeleton handler

op MOVEC with $C0F      MBAR        32      Module Base Address Register                        uninit (except V=0)                                 W
$003                    SIMR        8       SIM Configuration Register                          C0                                                  R/W
$014*-                  ICR1        8       Interrupt Control Register 1 - External IRQ1/IPL1   04                                                  R/W
$015*-                  ICR2        8       Interrupt Control Register 2 - External IPL2        08                                                  R/W
$016*-                  ICR3        8       Interrupt Control Register 3 - External IPL3        0C                                                  R/W
$017*-                  ICR4        8       Interrupt Control Register 4 - External IRQ4/IPL4   10                                                  R/W
$018*                   ICR5        8       Interrupt Control Register 5 - External IPL5        14                                                  R/W
$019*                   ICR6        8       Interrupt Control Register 6 - External IPL6        18                                                  R/W
$01A*                   ICR7        8       Interrupt Control Register 7 - External IRQ7/IPL7   1C                                                  R/W
$01B*                   ICR8        8       Interrupt Control Register 8 - SWT                  1C                                                  R/W
$01C*-                  ICR9        8       Interrupt Control Register 9 - Timer 1 Interrupt    80                                                  R/W
$01D*-                  ICR10       8       Interrupt Control Register 10 - Timer 2 Interrupt   80                                                  R/W
$01E*-                  ICR11       8       Interrupt Control Register 11 - MBUS Interrupt      80                                                  R/W
$01F*-                  ICR12       8       Interrupt Control Register 12 - UART 1 Interrupt    00                                                  R/W
$020*-                  ICR13       8       Interrupt Control Register 13 - UART 2 Interrupt    00                                                  R/W
$036*-                  IMR         16      Interrupt Mask Register                             3FFE                                                R/W
$03A                    IPR         16      Interrupt Pending Register                          0000                                                R
$040                    RSR         8       Reset Status Register                               80 / 20                                             R/W
$041                    SYPCR       8       System Protection Control Register                  00                                                  R/W
$042                    SWIVR       8       Software Watchdog Interrupt Vector Register         0F                                                  R/W
$043                    SWSR        8       Software Watchdog Service Register                  uninit                                              W
$046                    DCRR        16      DRAM Controller Refresh                             MR 0000   - NR uninit                               R/W
$04A                    DCTR        16      DRAM Controller Timing Register                     MR 0000   - NR uninit                               R/W
$04C                    DCAR0       16      DRAM Controller 0 Address Register                  MR uninit - NR uninit                               R/W
$050                    DCMR0       32      DRAM Controller 0 Mask Register                     MR uninit - NR uninit                               R/W
$057                    DCCR0       8       DRAM Controller 0 Control Register                  MR 00     - NR 00                                   R/W
$058                    DCAR1       16      DRAM Controller 1 Address Register                  MR uninit - NR uninit                               R/W
$05C                    DCMR1       32      DRAM Controller 1 Mask Register                     MR uninit - NR uninit                               R/W
$063                    DCCR1       8       DRAM Controller 1 Control Register                  MR 00     - NR 00                                   R/W
--------- CHIP SELECTS -----------
$064*-                  CSAR0       16      Chip-Select 0 Address Register                      0000                                                R/W
$068*-                  CSMR0       32      Chip-Select 0 Mask Register                         00000000                                            R/W
$06E*-                  CSCR0       16      Chip-Select 0 Control Register                      3C1F, 3C5F, 3C9F, 3CDF, 3D1F, 3D5F, 3D9F, 3DDF      R/W
                                                                                                AA set by IRQ 7 at reset
                                                                                                PS1 set by IRQ 4 at reset
                                                                                                PS0 set by IRQ 1 at reset
$070*-                  CSAR1       16      Chip-Select 1 Address Register                      uninit                                              R/W
$074*-                  CSMR1       32      Chip-Select 1 Mask Register                         uninit                                              R/W
$07A*-                  CSCR1       16      Chip-Select 1 Control Register                      uninit *1                                           R/W
$07C*-                  CSAR2       16      Chip-Select 2 Address Register                      uninit                                              R/W
$080*-                  CSMR2       32      Chip-Select 2 Mask Register                         uninit                                              R/W
$086*-                  CSCR2       16      Chip-Select 2 Control Register                      uninit *1                                           R/W
$088*-                  CSAR3       16      Chip-Select 3 Address Register                      uninit                                              R/W
$08C*-                  CSMR3       32      Chip-Select 3 Mask Register                         uninit                                              R/W
$092*-                  CSCR3       16      Chip-Select 3 Control Register                      uninit *1                                           R/W
$094*-                  CSAR4       16      Chip-Select 4 Address Register                      uninit                                              R/W
$098*-                  CSMR4       32      Chip-Select 4 Mask Register                         uninit                                              R/W
$09E*-                  CSCR4       16      Chip-Select 4 Control Register                      uninit *1                                           R/W
$0A0*-                  CSAR5       16      Chip-Select 5 Address Register                      uninit                                              R/W
$0A4*-                  CSMR5       32      Chip-Select 5 Mask Register                         uninit                                              R/W
$0AA*-                  CSCR5       16      Chip-Select 5 Control Register                      uninit *1                                           R/W
$0AC*-                  CSAR6       16      Chip-Select 6 Address Register                      uninit                                              R/W
$0B0*-                  CSMR6       32      Chip-Select 6 Mask Register                         uninit                                              R/W
$0B6*-                  CSCR6       16      Chip-Select 6 Control Register                      uninit *1                                           R/W
$0B8*-                  CSAR7       16      Chip-Select 7 Address Register                      uninit                                              R/W
$0BC*-                  CSMR7       32      Chip-Select 7 Mask Register                         uninit                                              R/W
$0C2*-                  CSCR7       16      Chip-Select 7 Control Register                      uninit *1                                           R/W
$0C6*-                  DMCR        16      Default Memory Control Register                     0000                                                R/W
$0CA*-                  PAR         16      Pin Assignment Register                             00                                                  R/W
--------- TIMER MODULE -----------
$100*-                  TMR1        16      Timer 1 Mode Register                               0000                                                R/W
$104*-                  TRR1        16      Timer 1 Reference Register                          FFFF                                                R/W
$108                    TCR1        16      Timer 1 Capture Register                            0000                                                R
$10C*-                  TCN1        16      Timer 1 Counter                                     0000                                                R/W
$111*-                  TER1        8       Timer 1 Event Register                              00                                                  R/W
$120                    TMR2        16      Timer 2 Mode Register                               0000                                                R/W
$124                    TRR2        16      Timer 2 Reference Register                          FFFF                                                R/W
$128                    TCR2        16      Timer 2 Capture Register                            0000                                                R
$12C                    TCN2        16      Timer 2 Counter                                     0000                                                R/W
$131                    TER2        8       Timer 2 Event Register                              00                                                  R/W
------------ UART SERIAL PORTS  -----------
$140                    UMR1,2      8       UART 1 Mode Registers                               00                                                  R/W
$144                    USR         8       UART 1 Status Register                              00                                                  R
                        UCSR        8       UART 1 Clock-Select Register                        DD                                                  W
$148                    UCR         8       UART 1 Command Register                             00                                                  W
$14C                    URB         8       UART 1 Receive Buffer                               FF                                                  R
                        UTB         8       UART 1 Transmit Buffer                              00                                                  W
$150                    UIPCR       8       UART Input Port Change Register                     0F                                                  R
                        UACR        8       UART 1 Auxilary Control Register                    00                                                  W
$154                    UISR        8       UART 1 Interrupt Status Register                    00                                                  R
                        UIMR        8       UART 1 Interrupt Mask Register                      00                                                  W
$158                    UBG1        8       UART 1 Baud Rate Generator Prescale MSB             uninit                                              W
$15C                    UBG2        8       UART 1 Baud Rate Generator Prescale LSB             uninit                                              W
$170                    UIVR        8       UART 1 Interrupt Vector Register                    0F                                                  R/W
$174                    UIP         8       UART 1 Input Port Register                          FF                                                  R
$178                    UOP1        8       UART 1 Output Port Bit Set CMD                      UOP1[7-1]=undef; UOP1=0                             W
$17C                    UOP0        8       UART 1 Output Port Bit Reset CMD                    uninit                                              W

$180                    UMR1,2      8       UART 2 Mode Registers                               00                                                  R/W
$184                    USR         8       UART 2 Status Register                              00                                                  R
                        UCSR        8       UART 2 Clock-Select Register                        DD                                                  W
$188                    UCR         8       UART 2 Command Register                             00                                                  W
$18C                    URB         8       UART 2 Receive Buffer                               FF                                                  R
                        UTB         8       UART 2 Transmit Buffer                              00                                                  W
$190                    UIPCR       8       UART 2 Input Port Change Register                   0F                                                  R
                        UACR        8       UART 2 Auxilary Control Register                    00                                                  W
$194                    UISR        8       UART 2 Interrupt Status Register                    00                                                  R
                        UIMR        8       UART 2 Interrupt Mask Register                      00                                                  W
$198                    UBG1        8       UART 2 Baud Rate Generator Prescale MSB             uninit                                              R/W
$19C                    UBG2        8       UART 2 Barud Rate Generator Prescale LSB            uninit                                              R/W
$1B0                    UIVR        8       UART 2 Interrupt Vector Register                    0F                                                  R/W
$1B4                    UIP         8       UART 2 Input Port Register                          FF                                                  R
$1B8                    UOP1        8       UART 2 Output Port Bit Set CMD                      UOP1[7-1]=undef; UOP1=0                             W
$1BC                    UOP0        8       UART 2 Output Port Bit Reset CMD                    uninit                                              W

$1C5*-                  PPDDR       8       Port A Data Direction Register                      00                                                  R/W
$1C9*-                  PPDAT       8       Port A Data Register                                00                                                  R/W
------------ MBUS  -----------
$1E0                    MADR        8       M-Bus Address Register                              00                                                  R/W
$1E4*-                  MFDR        8       M-Bus Frequency Divider Register                    00                                                  R/W
$1E8*-                  MBCR        8       M-Bus Control Register                              00                                                  R/W
$1EC*-                  MBSR        8       M-Bus Status Register                               00                                                  R/W
$1F0*-                  MBDR        8       M-Bus Data I/O Register                             00                                                  R/W
------------ DMA Controller -----------
$200                    DMASAR0     32      Source Address Register 0                           00                                                  R/W
$204                    DMADAR0     32      Destination Address Register 0                      00                                                  R/W
$208                    DCR0        16      DMA Control Register 0                              00                                                  R/W
$20C                    BCR0        16      Byte Count Register 0                               00                                                  R/W
$210                    DSR0        8       Status Register 0                                   00                                                  R/W
$214                    DIVR0       8       Interrupt Vector Register 0                         0F                                                  R/W
$240                    DMASAR1     32      Source Address Register 1                           00                                                  R/W
$244                    DMADAR1     32      Destination Address Register 1                      00                                                  R/W
$248                    DCR1        16      DMA Control Register 1                              00                                                  R/W
$24C                    BCR1        16      Byte Count Register 1                               00                                                  R/W
$250                    DSR1        8       Status Register 1                                   00                                                  R/W
$254                    DIVR1       8       Interrupt Vector Register 1                         0F                                                  R/W

*1 - uninit except BRST=ASET=WRAH=RDAH=WR=RD=0

*/
