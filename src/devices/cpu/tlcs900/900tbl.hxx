// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*******************************************************************

TLCS-900 instruction set

*******************************************************************/


enum e_operand
{
	p_A=1,       /* current register set register A */
	p_C8,        /* current register set byte */
	p_C16,       /* current register set word */
	p_C32,       /* current register set long word */
	p_MC16,      /* current register set mul/div register word */
	p_CC,        /* condition */
	p_CR8,
	p_CR16,
	p_CR32,
	p_D8,        /* byte displacement */
	p_D16,       /* word displacement */
	p_F,         /* F register */
	p_I3,        /* immediate 3 bit (part of last byte) */
	p_I8,        /* immediate byte */
	p_I16,       /* immediate word */
	p_I24,       /* immediate 3 byte address */
	p_I32,       /* immediate long word */
	p_M,         /* memory location (defined by extension) */
	p_M8,        /* (8) */
	p_M16,       /* (i16) */
	p_R,         /* register (defined by extension) */
	p_SR         /* status register */
};


namespace {

/* Register selector byte layout - the same encoding get_reg(),
   get_reg8(), get_reg16() and get_reg32() decode.  Bits 4-7 select the
   register bank or register group, bits 2-3 the register pair within
   it, and bits 0-1 the byte or word within the pair. */
enum : uint8_t
{
	REGSEL_BANK_MASK     = 0xF0,
	REGSEL_PAIR_MASK     = 0x0C,
	REGSEL_PART_MASK     = 0x03,
	REGSEL_PAIR_SHIFT    = 2,

	REGSEL_BANK0         = 0x00,  /* explicit register bank 0 */
	REGSEL_BANK1         = 0x10,  /* explicit register bank 1 */
	REGSEL_BANK2         = 0x20,  /* explicit register bank 2 */
	REGSEL_BANK3         = 0x30,  /* explicit register bank 3 */
	REGSEL_BANK_PREVIOUS = 0xD0,  /* current bank minus one, wrapping from bank 0 to bank 3 */
	REGSEL_BANK_CURRENT  = 0xE0,  /* current register bank */
	REGSEL_GROUP_INDEX   = 0xF0,  /* index registers and stack pointers */

	REGSEL_PART_LOW      = 0x00,  /* low byte or word of the register pair */
	REGSEL_PART_HIGH     = 0x01,  /* high byte of the register pair */

	/* register A: low byte of the XWA pair of the current register bank */
	REGSEL_A            = REGSEL_BANK_CURRENT | REGSEL_PART_LOW
};

} // anonymous namespace


bool tlcs900_device::condition_true(uint8_t cond)
{
	switch (cond & 0x0F)
	{
	/* F */
	case 0x00:
		return false;

	/* LT */
	case 0x01:
		return (((m_sr.b.l & (FLAG_SF | FLAG_VF)) == FLAG_SF) ||
			((m_sr.b.l & (FLAG_SF | FLAG_VF)) == FLAG_VF));

	/* LE */
	case 0x02:
		return (((m_sr.b.l & (FLAG_SF | FLAG_VF)) == FLAG_SF) ||
			((m_sr.b.l & (FLAG_SF | FLAG_VF)) == FLAG_VF) ||
			(m_sr.b.l & FLAG_ZF));

	/* ULE */
	case 0x03:
		return m_sr.b.l & (FLAG_ZF | FLAG_CF);

	/* OV */
	case 0x04:
		return m_sr.b.l & FLAG_VF;

	/* MI */
	case 0x05:
		return m_sr.b.l & FLAG_SF;

	/* Z */
	case 0x06:
		return m_sr.b.l & FLAG_ZF;

	/* C */
	case 0x07:
		return m_sr.b.l & FLAG_CF;

	/* T */
	case 0x08:
		return true;

	/* GE */
	case 0x09:
		return !(((m_sr.b.l & (FLAG_SF | FLAG_VF)) == FLAG_SF) ||
			((m_sr.b.l & (FLAG_SF | FLAG_VF)) == FLAG_VF));

	/* GT */
	case 0x0A:
		return !(((m_sr.b.l & (FLAG_SF | FLAG_VF)) == FLAG_SF) ||
			((m_sr.b.l & (FLAG_SF | FLAG_VF)) == FLAG_VF) ||
			(m_sr.b.l & FLAG_ZF));

	/* UGT */
	case 0x0B:
		return !(m_sr.b.l & (FLAG_ZF | FLAG_CF));

	/* NOV */
	case 0x0C:
		return !(m_sr.b.l & FLAG_VF);

	/* PL */
	case 0x0D:
		return !(m_sr.b.l & FLAG_SF);

	/* NZ */
	case 0x0E:
		return !(m_sr.b.l & FLAG_ZF);

	/* NC */
	case 0x0F:
		return !(m_sr.b.l & FLAG_CF);
	}
	return false;
}


uint32_t& tlcs900_device::get_reg32_current(uint8_t reg)
{
	switch (reg & 7)
	{
	/* XWA */
	case 0:
		return m_xwa[m_regbank].d;

	/* XBC */
	case 1:
		return m_xbc[m_regbank].d;

	/* XDE */
	case 2:
		return m_xde[m_regbank].d;

	/* XHL */
	case 3:
		return m_xhl[m_regbank].d;

	/* XIX */
	case 4:
		return m_xix.d;

	/* XIY */
	case 5:
		return m_xiy.d;

	/* XIZ */
	case 6:
		return m_xiz.d;

	/* XSP */
	case 7:
		/* TODO: Add selector for user/system stack pointer */
		return m_xssp.d;
	}
	/* keep compiler happy */
	return m_dummy.d;
}


PAIR& tlcs900_device::get_reg(uint8_t reg)
{
	uint8_t regbank;

	switch (reg & REGSEL_BANK_MASK)
	{
	case REGSEL_BANK0: case REGSEL_BANK1: /* explicit register bank */
	case REGSEL_BANK2: case REGSEL_BANK3: /* explicit register bank */
	case REGSEL_BANK_PREVIOUS:            /* current bank minus one */
	case REGSEL_BANK_CURRENT:             /* current register bank */
		regbank = (reg & REGSEL_BANK_MASK) >> 4;
		if (regbank == REGSEL_BANK_PREVIOUS >> 4)
			regbank = (m_regbank - 1) & 0x03;

		if (regbank == REGSEL_BANK_CURRENT >> 4)
			regbank = m_regbank;

		switch (reg & REGSEL_PAIR_MASK)
		{
		case 0x00:  return m_xwa[regbank];
		case 0x04:  return m_xbc[regbank];
		case 0x08:  return m_xde[regbank];
		case 0x0C:  return m_xhl[regbank];
		}
		break;
	case REGSEL_GROUP_INDEX:  /* index registers and sp */
		switch (reg & REGSEL_PAIR_MASK)
		{
		case 0x00:  return m_xix;
		case 0x04:  return m_xiy;
		case 0x08:  return m_xiz;
		/* TODO: Use correct SP */
		case 0x0C:  return m_xssp;
		}
		break;
	}

	/* illegal/unknown register reference */
	logerror("Access to unknown tlcs-900 cpu register %02x\n", reg);
	return m_dummy;
}


uint8_t& tlcs900_device::get_reg8(uint8_t reg)
{
	switch (reg & REGSEL_PART_MASK)
	{
	case 0x00: return get_reg(reg).b.l;
	case 0x01: return get_reg(reg).b.h;
	case 0x02: return get_reg(reg).b.h2;
	case 0x03: return get_reg(reg).b.h3;
	}

	/* keep compiler happy */
	return get_reg(reg).b.l;
}


uint16_t& tlcs900_device::get_reg16(uint8_t reg)
{
	return BIT(reg, 1) ? get_reg(reg).w.h : get_reg(reg).w.l;
}


uint32_t& tlcs900_device::get_reg32(uint8_t reg)
{
	return get_reg(reg).d;
}


uint8_t tlcs900_device::get_reg8_current_sel(uint8_t reg)
{
	/* W A B C D E H L */
	const uint8_t pair = ((reg & 6) >> 1) << REGSEL_PAIR_SHIFT;
	const uint8_t part = BIT(reg, 0) ? REGSEL_PART_LOW : REGSEL_PART_HIGH;

	return REGSEL_BANK_CURRENT | pair | part;
}


uint8_t tlcs900_device::get_reg16_current_sel(uint8_t reg)
{
	/* WA BC DE HL IX IY IZ SP */
	const uint8_t bank = BIT(reg, 2) ? REGSEL_GROUP_INDEX : REGSEL_BANK_CURRENT;

	return bank | ((reg & 3) << REGSEL_PAIR_SHIFT);
}


uint8_t tlcs900_device::get_reg32_current_sel(uint8_t reg)
{
	/* same register pairs as the 16-bit selector */
	return get_reg16_current_sel(reg);
}


uint8_t& tlcs900_device::reg8(uint16_t sel)
{
	if (sel < 0x100)
		return get_reg8(sel);

	switch (sel)
	{
	case regsel_sr:
		return m_sr.b.l;
	case regsel_f2:
		return m_f2.b.l;
	case regsel_dmam0: case regsel_dmam1: case regsel_dmam2: case regsel_dmam3:
		return m_dmam[sel - regsel_dmam0].b.l;
	}
	return m_dummy.b.l;
}


uint16_t& tlcs900_device::reg16(uint16_t sel)
{
	if (sel < 0x100)
		return get_reg16(sel);

	switch (sel)
	{
	case regsel_sr:
		return m_sr.w.l;
	case regsel_intnest:
		return m_intnest;
	case regsel_dmac0: case regsel_dmac1: case regsel_dmac2: case regsel_dmac3:
		return m_dmac[sel - regsel_dmac0].w.l;
	}
	return m_dummy.w.l;
}


uint32_t& tlcs900_device::reg32(uint16_t sel)
{
	if (sel < 0x100)
		return get_reg32(sel);

	switch (sel)
	{
	case regsel_dmas0: case regsel_dmas1: case regsel_dmas2: case regsel_dmas3:
		return m_dmas[sel - regsel_dmas0].d;
	case regsel_dmad0: case regsel_dmad1: case regsel_dmad2: case regsel_dmad3:
		return m_dmad[sel - regsel_dmad0].d;
	}
	return m_dummy.d;
}


template <typename T>
void tlcs900_device::parity(T a)
{
	int j = 0;
	for (int i = 0; i < 8 * sizeof(T); i++)
	{
		if (a & 1) j++;
		a >>= 1;
	}
	m_sr.b.l |= (j & 1) ? 0 : FLAG_VF;
}


template <typename T>
T tlcs900_device::adc(T a, T b)
{
	constexpr uint8_t sign_bit = 8 * sizeof(T) - 1;
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const uint8_t cy = m_sr.b.l & FLAG_CF;
	const T result = a + b + cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ((result >> sign_shift) & FLAG_SF) | (result ? 0 : FLAG_ZF);
	if constexpr (sizeof(T) <= 2)
		m_sr.b.l |= ((a ^ b) ^ result) & FLAG_HF;
	m_sr.b.l |= (BIT((result ^ a) & (result ^ b), sign_bit) ? FLAG_VF : 0) |
		(((result < a) || ((result == a) && cy)) ? FLAG_CF : 0);

	return result;
}


template <typename T>
T tlcs900_device::add(T a, T b)
{
	constexpr uint8_t sign_bit = 8 * sizeof(T) - 1;
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const T result = a + b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ((result >> sign_shift) & FLAG_SF) | (result ? 0 : FLAG_ZF);
	if constexpr (sizeof(T) <= 2)
		m_sr.b.l |= ((a ^ b) ^ result) & FLAG_HF;
	m_sr.b.l |= (BIT((result ^ a) & (result ^ b), sign_bit) ? FLAG_VF : 0) |
		((result < a) ? FLAG_CF : 0);

	return result;
}


template <typename T>
T tlcs900_device::sbc(T a, T b)
{
	constexpr uint8_t sign_bit = 8 * sizeof(T) - 1;
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const uint8_t cy = m_sr.b.l & FLAG_CF;
	const T result = a - b - cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ((result >> sign_shift) & FLAG_SF) | (result ? 0 : FLAG_ZF);
	if constexpr (sizeof(T) <= 2)
		m_sr.b.l |= ((a ^ b) ^ result) & FLAG_HF;
	m_sr.b.l |= (BIT((result ^ a) & (a ^ b), sign_bit) ? FLAG_VF : 0) |
		(((result > a) || (cy && (b == T(~0)))) ? FLAG_CF : 0) | FLAG_NF;

	return result;
}


template <typename T>
T tlcs900_device::sub(T a, T b)
{
	constexpr uint8_t sign_bit = 8 * sizeof(T) - 1;
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const T result = a - b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ((result >> sign_shift) & FLAG_SF) | (result ? 0 : FLAG_ZF);
	if constexpr (sizeof(T) <= 2)
		m_sr.b.l |= ((a ^ b) ^ result) & FLAG_HF;
	m_sr.b.l |= (BIT((result ^ a) & (a ^ b), sign_bit) ? FLAG_VF : 0) |
		((result > a) ? FLAG_CF : 0) | FLAG_NF;

	return result;
}


template <typename T>
T tlcs900_device::and_(T a, T b)
{
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const T result = a & b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ((result >> sign_shift) & FLAG_SF) | (result ? 0 : FLAG_ZF) | FLAG_HF;
	if constexpr (sizeof(T) <= 2)
		parity(result);

	return result;
}


template <typename T>
T tlcs900_device::or_(T a, T b)
{
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const T result = a | b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ((result >> sign_shift) & FLAG_SF) | (result ? 0 : FLAG_ZF);
	if constexpr (sizeof(T) <= 2)
		parity(result);

	return result;
}


template <typename T>
T tlcs900_device::xor_(T a, T b)
{
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const T result = a ^ b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ((result >> sign_shift) & FLAG_SF) | (result ? 0 : FLAG_ZF);
	if constexpr (sizeof(T) <= 2)
		parity(result);

	return result;
}


template <typename T>
void tlcs900_device::ldcf(uint8_t a, T b)
{
	if (BIT(b, a & (8 * sizeof(T) - 1)))
		m_sr.b.l |= FLAG_CF;
	else
		m_sr.b.l &= ~FLAG_CF;
}


template <typename T>
void tlcs900_device::andcf(uint8_t a, T b)
{
	if (BIT(b, a & (8 * sizeof(T) - 1)) && (m_sr.b.l & FLAG_CF))
		m_sr.b.l |= FLAG_CF;
	else
		m_sr.b.l &= ~FLAG_CF;
}


template <typename T>
void tlcs900_device::orcf(uint8_t a, T b)
{
	if (BIT(b, a & (8 * sizeof(T) - 1)))
		m_sr.b.l |= FLAG_CF;
}


template <typename T>
void tlcs900_device::xorcf(uint8_t a, T b)
{
	if (BIT(b, a & (8 * sizeof(T) - 1)))
		m_sr.b.l ^= FLAG_CF;
}


template <typename T>
T tlcs900_device::rl(T a, uint8_t s)
{
	constexpr uint8_t sign_bit = 8 * sizeof(T) - 1;
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const uint8_t count = (s & 0x0F) ? (s & 0x0F) : 16;

	for (uint8_t n = count; n > 0; n--)
	{
		if (BIT(a, sign_bit))
		{
			a = (a << 1) | (m_sr.b.l & FLAG_CF);
			m_sr.b.l |= FLAG_CF;
		}
		else
		{
			a = (a << 1) | (m_sr.b.l & FLAG_CF);
			m_sr.b.l &= ~FLAG_CF;
		}
	}
	m_cycles += tlcs900_shift_cycles(count);

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF);
	m_sr.b.l |= ((a >> sign_shift) & FLAG_SF) | (a ? 0 : FLAG_ZF);
	parity(a);

	return a;
}


template <typename T>
T tlcs900_device::rlc(T a, uint8_t s)
{
	constexpr uint8_t sign_bit = 8 * sizeof(T) - 1;
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const uint8_t count = (s & 0x0F) ? (s & 0x0F) : 16;

	for (uint8_t n = count; n > 0; n--)
	{
		a = (a << 1) | BIT(a, sign_bit);
	}
	m_cycles += tlcs900_shift_cycles(count);

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ((a >> sign_shift) & FLAG_SF) | (a ? 0 : FLAG_ZF) | (a & FLAG_CF);
	parity(a);

	return a;
}


template <typename T>
T tlcs900_device::rr(T a, uint8_t s)
{
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const uint8_t count = (s & 0x0F) ? (s & 0x0F) : 16;

	for (uint8_t n = count; n > 0; n--)
	{
		if (m_sr.b.l & FLAG_CF)
		{
			m_sr.b.l = (m_sr.b.l & ~FLAG_CF) | (a & FLAG_CF);
			a = (a >> 1) | (T(0x80) << sign_shift);
		}
		else
		{
			m_sr.b.l = (m_sr.b.l & ~FLAG_CF) | (a & FLAG_CF);
			a = (a >> 1);
		}
	}
	m_cycles += tlcs900_shift_cycles(count);

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF);
	m_sr.b.l |= ((a >> sign_shift) & FLAG_SF) | (a ? 0 : FLAG_ZF);
	parity(a);

	return a;
}


template <typename T>
T tlcs900_device::rrc(T a, uint8_t s)
{
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const uint8_t count = (s & 0x0F) ? (s & 0x0F) : 16;

	for (uint8_t n = count; n > 0; n--)
	{
		a = (a >> 1) | (BIT(a, 0) ? (T(0x80) << sign_shift) : 0);
	}
	m_cycles += tlcs900_shift_cycles(count);

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= (((a >> sign_shift) & FLAG_SF) ? FLAG_CF | FLAG_SF : 0) | (a ? 0 : FLAG_ZF);
	parity(a);

	return a;
}


template <typename T>
T tlcs900_device::sla(T a, uint8_t s)
{
	constexpr uint8_t sign_bit = 8 * sizeof(T) - 1;
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const uint8_t count = (s & 0x0F) ? (s & 0x0F) : 16;

	for (uint8_t n = count; n > 0; n--)
	{
		m_sr.b.l = (m_sr.b.l & ~FLAG_CF) | (BIT(a, sign_bit) ? FLAG_CF : 0);
		a = (a << 1);
	}
	m_cycles += tlcs900_shift_cycles(count);

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF);
	m_sr.b.l |= ((a >> sign_shift) & FLAG_SF) | (a ? 0 : FLAG_ZF);
	parity(a);

	return a;
}


template <typename T>
T tlcs900_device::sra(T a, uint8_t s)
{
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const uint8_t count = (s & 0x0F) ? (s & 0x0F) : 16;

	for (uint8_t n = count; n > 0; n--)
	{
		m_sr.b.l = (m_sr.b.l & ~FLAG_CF) | (a & FLAG_CF);
		a = (a & (T(0x80) << sign_shift)) | (a >> 1);
	}
	m_cycles += tlcs900_shift_cycles(count);

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF);
	m_sr.b.l |= ((a >> sign_shift) & FLAG_SF) | (a ? 0 : FLAG_ZF);
	parity(a);

	return a;
}


template <typename T>
T tlcs900_device::srl(T a, uint8_t s)
{
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	const uint8_t count = (s & 0x0F) ? (s & 0x0F) : 16;

	for (uint8_t n = count; n > 0; n--)
	{
		m_sr.b.l = (m_sr.b.l & ~FLAG_CF) | (a & FLAG_CF);
		a = (a >> 1);
	}
	m_cycles += tlcs900_shift_cycles(count);

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF);
	m_sr.b.l |= ((a >> sign_shift) & FLAG_SF) | (a ? 0 : FLAG_ZF);
	parity(a);

	return a;
}


/* LDI/LDD and their repeat and word variants share one implementation:
   Direction selects the direction the two pointers move, one element of
   width T per step, and Repeat re-executes the instruction until BC
   reaches zero. */

template <typename T, int Direction, bool Repeat>
void tlcs900_device::ldxx()
{
	if constexpr (sizeof(T) == 1)
		WRMEM(reg32(m_reg1), RDMEM(reg32(m_reg2)));
	else
		WRMEMW(reg32(m_reg1), RDMEMW(reg32(m_reg2)));
	reg32(m_reg1) += Direction * int(sizeof(T));
	reg32(m_reg2) += Direction * int(sizeof(T));
	m_xbc[m_regbank].w.l -= 1;
	m_sr.b.l &= ~(FLAG_HF | FLAG_VF | FLAG_NF);
	if (m_xbc[m_regbank].w.l)
	{
		m_sr.b.l |= FLAG_VF;
		if constexpr (Repeat)
		{
			m_pc.d -= 2;
			m_cycles += tlcs900_ldxx_repeat_cycles();
			m_prefetch_clear = true;
		}
	}
}


/* CPD and CPI compare A (or WA for word operations) with (HL) and step HL
   by one element of width T in the selected direction. */

template <typename T, int Direction>
void tlcs900_device::cpx()
{
	constexpr uint8_t sign_shift = 8 * (sizeof(T) - 1);
	T result;
	if constexpr (sizeof(T) == 1)
		result = m_xwa[m_regbank].b.l - RDMEM(reg32(m_reg2));
	else
		result = m_xwa[m_regbank].w.l - RDMEMW(reg32(m_reg2));

	reg32(m_reg2) += Direction * int(sizeof(T));
	m_xbc[m_regbank].w.l -= 1;
	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF);
	m_sr.b.l |= ((result >> sign_shift) & FLAG_SF) | (result ? FLAG_NF : FLAG_NF | FLAG_ZF) |
		(m_xbc[m_regbank].w.l ? FLAG_VF : 0);
}


uint16_t tlcs900_device::div8(uint16_t a, uint8_t b)
{
	if (!b)
	{
		m_sr.b.l |= FLAG_VF;
		return (a << 8) | ((a >> 8) ^ 0xFF);
	}

	ldiv_t result;

	if (a >= (0x0200 * b)) {
		const uint16_t diff = a - (0x0200 * b);
		const uint16_t range = 0x100 - b;

		result = ldiv(diff, range);
		result.quot = 0x1FF - result.quot;
		result.rem = result.rem + b;
	}
	else
	{
		result = ldiv(a, b);
	}

	if (result.quot > 0xFF)
		m_sr.b.l |= FLAG_VF;
	else
		m_sr.b.l &= ~FLAG_VF;

	return (result.quot & 0xFF) | ((result.rem & 0xFF) << 8);
}


uint32_t tlcs900_device::div16(uint32_t a, uint16_t b)
{
	if (!b)
	{
		m_sr.b.l |= FLAG_VF;
		return (a << 16) | ((a >> 16) ^ 0xFFFF);
	}

	const ldiv_t result = ldiv(a, b);

	if (result.quot > 0xFFFF)
		m_sr.b.l |= FLAG_VF;
	else
		m_sr.b.l &= ~FLAG_VF;

	return (result.quot & 0xFFFF) | ((result.rem & 0xFFFF) << 16);
}


uint16_t tlcs900_device::divs8(int16_t a, int8_t b)
{
	if (!b)
	{
		m_sr.b.l |= FLAG_VF;
		return (a << 8) | ((a >> 8) ^ 0xFF);
	}

	const ldiv_t result = ldiv(a, b);

	if (result.quot > 0xFF)
		m_sr.b.l |= FLAG_VF;
	else
		m_sr.b.l &= ~FLAG_VF;

	return (result.quot & 0xFF) | ((result.rem & 0xFF) << 8);
}


uint32_t tlcs900_device::divs16(int32_t a, int16_t b)
{
	if (!b)
	{
		m_sr.b.l |= FLAG_VF;
		return (a << 16) | ((a >> 16) ^ 0xFFFF);
	}

	const ldiv_t result = ldiv(a, b);

	if (result.quot > 0xFFFF)
		m_sr.b.l |= FLAG_VF;
	else
		m_sr.b.l &= ~FLAG_VF;

	return (result.quot & 0xFFFF) | ((result.rem & 0xFFFF) << 16);
}


void tlcs900_device::op_ADCBMI()
{
	WRMEM(m_ea1.d, adc<uint8_t>(RDMEM(m_ea1.d), m_imm2.b.l));
}


void tlcs900_device::op_ADCBMR()
{
	WRMEM(m_ea1.d, adc<uint8_t>(RDMEM(m_ea1.d), reg8(m_reg2)));
}


void tlcs900_device::op_ADCBRI()
{
	reg8(m_reg1) = adc<uint8_t>(reg8(m_reg1), m_imm2.b.l);
}


void tlcs900_device::op_ADCBRM()
{
	reg8(m_reg1) = adc<uint8_t>(reg8(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_ADCBRR()
{
	reg8(m_reg1) = adc<uint8_t>(reg8(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_ADCWMI()
{
	WRMEMW(m_ea1.d, adc<uint16_t>(RDMEMW(m_ea1.d), m_imm2.w.l));
}


void tlcs900_device::op_ADCWMR()
{
	WRMEMW(m_ea1.d, adc<uint16_t>(RDMEMW(m_ea1.d), reg16(m_reg2)));
}


void tlcs900_device::op_ADCWRI()
{
	reg16(m_reg1) = adc<uint16_t>(reg16(m_reg1), m_imm2.w.l);
}


void tlcs900_device::op_ADCWRM()
{
	reg16(m_reg1) = adc<uint16_t>(reg16(m_reg1), RDMEMW(m_ea2.d));
}


void tlcs900_device::op_ADCWRR()
{
	reg16(m_reg1) = adc<uint16_t>(reg16(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_ADCLMR()
{
	WRMEML(m_ea1.d, adc<uint32_t>(RDMEML(m_ea1.d), reg32(m_reg2)));
}


void tlcs900_device::op_ADCLRI()
{
	reg32(m_reg1) = adc<uint32_t>(reg32(m_reg1), m_imm2.d);
}


void tlcs900_device::op_ADCLRM()
{
	reg32(m_reg1) = adc<uint32_t>(reg32(m_reg1), RDMEML(m_ea2.d));
}


void tlcs900_device::op_ADCLRR()
{
	reg32(m_reg1) = adc<uint32_t>(reg32(m_reg1), reg32(m_reg2));
}


void tlcs900_device::op_ADDBMI()
{
	WRMEM(m_ea1.d, add<uint8_t>(RDMEM(m_ea1.d), m_imm2.b.l));
}


void tlcs900_device::op_ADDBMR()
{
	WRMEM(m_ea1.d, add<uint8_t>(RDMEM(m_ea1.d), reg8(m_reg2)));
}


void tlcs900_device::op_ADDBRI()
{
	reg8(m_reg1) = add<uint8_t>(reg8(m_reg1), m_imm2.b.l);
}


void tlcs900_device::op_ADDBRM()
{
	reg8(m_reg1) = add<uint8_t>(reg8(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_ADDBRR()
{
	reg8(m_reg1) = add<uint8_t>(reg8(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_ADDWMI()
{
	WRMEMW(m_ea1.d, add<uint16_t>(RDMEMW(m_ea1.d), m_imm2.w.l));
}


void tlcs900_device::op_ADDWMR()
{
	WRMEMW(m_ea1.d, add<uint16_t>(RDMEMW(m_ea1.d), reg16(m_reg2)));
}


void tlcs900_device::op_ADDWRI()
{
	reg16(m_reg1) = add<uint16_t>(reg16(m_reg1), m_imm2.w.l);
}


void tlcs900_device::op_ADDWRM()
{
	reg16(m_reg1) = add<uint16_t>(reg16(m_reg1), RDMEMW(m_ea2.d));
}


void tlcs900_device::op_ADDWRR()
{
	reg16(m_reg1) = add<uint16_t>(reg16(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_ADDLMR()
{
	WRMEML(m_ea1.d, add<uint32_t>(RDMEML(m_ea1.d), reg32(m_reg2)));
}


void tlcs900_device::op_ADDLRI()
{
	reg32(m_reg1) = add<uint32_t>(reg32(m_reg1), m_imm2.d);
}


void tlcs900_device::op_ADDLRM()
{
	reg32(m_reg1) = add<uint32_t>(reg32(m_reg1), RDMEML(m_ea2.d));
}


void tlcs900_device::op_ADDLRR()
{
	reg32(m_reg1) = add<uint32_t>(reg32(m_reg1), reg32(m_reg2));
}


void tlcs900_device::op_ANDBMI()
{
	WRMEM(m_ea1.d, and_<uint8_t>(RDMEM(m_ea1.d), m_imm2.b.l));
}


void tlcs900_device::op_ANDBMR()
{
	WRMEM(m_ea1.d, and_<uint8_t>(RDMEM(m_ea1.d), reg8(m_reg2)));
}


void tlcs900_device::op_ANDBRI()
{
	reg8(m_reg1) = and_<uint8_t>(reg8(m_reg1), m_imm2.b.l);
}


void tlcs900_device::op_ANDBRM()
{
	reg8(m_reg1) = and_<uint8_t>(reg8(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_ANDBRR()
{
	reg8(m_reg1) = and_<uint8_t>(reg8(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_ANDWMI()
{
	WRMEMW(m_ea1.d, and_<uint16_t>(RDMEMW(m_ea1.d), m_imm2.w.l));
}


void tlcs900_device::op_ANDWMR()
{
	WRMEMW(m_ea1.d, and_<uint16_t>(RDMEMW(m_ea1.d), reg16(m_reg2)));
}


void tlcs900_device::op_ANDWRI()
{
	reg16(m_reg1) = and_<uint16_t>(reg16(m_reg1), m_imm2.w.l);
}


void tlcs900_device::op_ANDWRM()
{
	reg16(m_reg1) = and_<uint16_t>(reg16(m_reg1), RDMEMW(m_ea2.d));
}


void tlcs900_device::op_ANDWRR()
{
	reg16(m_reg1) = and_<uint16_t>(reg16(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_ANDLMR()
{
	WRMEML(m_ea1.d, and_<uint32_t>(RDMEML(m_ea1.d), reg32(m_reg2)));
}


void tlcs900_device::op_ANDLRI()
{
	reg32(m_reg1) = and_<uint32_t>(reg32(m_reg1), m_imm2.d);
}


void tlcs900_device::op_ANDLRM()
{
	reg32(m_reg1) = and_<uint32_t>(reg32(m_reg1), RDMEML(m_ea2.d));
}


void tlcs900_device::op_ANDLRR()
{
	reg32(m_reg1) = and_<uint32_t>(reg32(m_reg1), reg32(m_reg2));
}


void tlcs900_device::op_ANDCFBIM()
{
	andcf<uint8_t>(m_imm1.b.l, RDMEM(m_ea2.d));
}


void tlcs900_device::op_ANDCFBIR()
{
	andcf<uint8_t>(m_imm1.b.l, reg8(m_reg2));
}


void tlcs900_device::op_ANDCFBRM()
{
	andcf<uint8_t>(reg8(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_ANDCFBRR()
{
	andcf<uint8_t>(reg8(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_ANDCFWIR()
{
	andcf<uint16_t>(m_imm1.b.l, reg16(m_reg2));
}


void tlcs900_device::op_ANDCFWRR()
{
	andcf<uint16_t>(reg8(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_BITBIM()
{
	m_sr.b.l &= ~(FLAG_ZF | FLAG_NF);
	if (RDMEM(m_ea2.d) & (1 << (m_imm1.b.l & 0x07)))
		m_sr.b.l |= FLAG_HF;
	else
		m_sr.b.l |= FLAG_HF | FLAG_ZF;
}


void tlcs900_device::op_BITBIR()
{
	m_sr.b.l &= ~(FLAG_ZF | FLAG_NF);
	if (reg8(m_reg2) & (1 << (m_imm1.b.l & 0x0F)))
		m_sr.b.l |= FLAG_HF;
	else
		m_sr.b.l |= FLAG_HF | FLAG_ZF;
}


void tlcs900_device::op_BITWIR()
{
	m_sr.b.l &= ~(FLAG_ZF | FLAG_NF);
	if (reg16(m_reg2) & (1 << (m_imm1.b.l & 0x0F)))
		m_sr.b.l |= FLAG_HF;
	else
		m_sr.b.l |= FLAG_HF | FLAG_ZF;
}


void tlcs900_device::op_BS1BRR()
{
	uint16_t r = reg16(m_reg2);

	if (r)
	{
		m_sr.b.l &= ~FLAG_VF;
		reg8(m_reg1) = 15;
		while (!BIT(r, 15))
		{
			r <<= 1;
			reg8(m_reg1) -= 1;
		}
	}
	else
		m_sr.b.l |= FLAG_VF;
}


void tlcs900_device::op_BS1FRR()
{
	uint16_t  r = reg16(m_reg2);

	if (r)
	{
		m_sr.b.l &= ~FLAG_VF;
		reg8(m_reg1) = 0;
		while (!BIT(r, 0))
		{
			r >>= 1;
			reg8(m_reg1) += 1;
		}
	}
	else
		m_sr.b.l |= FLAG_VF;
}


void tlcs900_device::op_CALLI()
{
	m_xssp.d -= 4;
	WRMEML(m_xssp.d, m_pc.d);
	m_pc.d = m_imm1.d;
	m_prefetch_clear = true;
}


void tlcs900_device::op_CALLM()
{
	if (condition_true(m_op))
	{
		m_xssp.d -= 4;
		WRMEML(m_xssp.d, m_pc.d);
		m_pc.d = m_ea2.d;
		m_cycles += tlcs900_call_true_cycles();
		m_prefetch_clear = true;
	}
}


void tlcs900_device::op_CALR()
{
	m_xssp.d -= 4;
	WRMEML(m_xssp.d, m_pc.d);
	m_pc.d = m_ea1.d;
	m_prefetch_clear = true;
}


void tlcs900_device::op_CCF()
{
	m_sr.b.l &= ~FLAG_NF;
	m_sr.b.l ^= FLAG_CF;
}


void tlcs900_device::op_CHGBIM()
{
	WRMEM(m_ea2.d, RDMEM(m_ea2.d) ^ (1 << (m_imm1.b.l & 0x07)));
}


void tlcs900_device::op_CHGBIR()
{
	reg8(m_reg2) ^= (1 << (m_imm1.b.l & 0x07));
}


void tlcs900_device::op_CHGWIR()
{
	reg16(m_reg2) ^= (1 << (m_imm1.b.l & 0x0F));
}


void tlcs900_device::op_CPBMI()
{
	sub<uint8_t>(RDMEM(m_ea1.d), m_imm2.b.l);
}


void tlcs900_device::op_CPBMR()
{
	sub<uint8_t>(RDMEM(m_ea1.d), reg8(m_reg2));
}


void tlcs900_device::op_CPBRI()
{
	sub<uint8_t>(reg8(m_reg1), m_imm2.b.l);
}


void tlcs900_device::op_CPBRM()
{
	sub<uint8_t>(reg8(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_CPBRR()
{
	sub<uint8_t>(reg8(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_CPWMI()
{
	sub<uint16_t>(RDMEMW(m_ea1.d), m_imm2.w.l);
}


void tlcs900_device::op_CPWMR()
{
	sub<uint16_t>(RDMEMW(m_ea1.d), reg16(m_reg2));
}


void tlcs900_device::op_CPWRI()
{
	sub<uint16_t>(reg16(m_reg1), m_imm2.w.l);
}


void tlcs900_device::op_CPWRM()
{
	sub<uint16_t>(reg16(m_reg1), RDMEMW(m_ea2.d));
}


void tlcs900_device::op_CPWRR()
{
	sub<uint16_t>(reg16(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_CPLMR()
{
	sub<uint32_t>(RDMEML(m_ea1.d), reg32(m_reg2));
}


void tlcs900_device::op_CPLRI()
{
	sub<uint32_t>(reg32(m_reg1), m_imm2.d);
}


void tlcs900_device::op_CPLRM()
{
	sub<uint32_t>(reg32(m_reg1), RDMEML(m_ea2.d));
}


void tlcs900_device::op_CPLRR()
{
	sub<uint32_t>(reg32(m_reg1), reg32(m_reg2));
}


void tlcs900_device::op_CPD()
{
	cpx<uint8_t, -1>();
}


void tlcs900_device::op_CPDR()
{
	op_CPD();

	if ((m_sr.b.l & (FLAG_ZF | FLAG_VF)) == FLAG_VF)
	{
		m_pc.d -= 2;
		m_cycles += tlcs900_ldxx_repeat_cycles();
		m_prefetch_clear = true;
	}
}


void tlcs900_device::op_CPDW()
{
	cpx<uint16_t, -1>();
}


void tlcs900_device::op_CPDRW()
{
	op_CPDW();

	if ((m_sr.b.l & (FLAG_ZF | FLAG_VF)) == FLAG_VF)
	{
		m_pc.d -= 2;
		m_cycles += tlcs900_ldxx_repeat_cycles();
		m_prefetch_clear = true;
	}
}


void tlcs900_device::op_CPI()
{
	cpx<uint8_t, 1>();
}


void tlcs900_device::op_CPIR()
{
	op_CPI();

	if ((m_sr.b.l & (FLAG_ZF | FLAG_VF)) == FLAG_VF)
	{
		m_pc.d -= 2;
		m_cycles += tlcs900_ldxx_repeat_cycles();
		m_prefetch_clear = true;
	}
}


void tlcs900_device::op_CPIW()
{
	cpx<uint16_t, 1>();
}


void tlcs900_device::op_CPIRW()
{
	op_CPIW();

	if ((m_sr.b.l & (FLAG_ZF | FLAG_VF)) == FLAG_VF)
	{
		m_pc.d -= 2;
		m_cycles += tlcs900_ldxx_repeat_cycles();
		m_prefetch_clear = true;
	}
}


void tlcs900_device::op_CPLBR()
{
	reg8(m_reg1) = ~reg8(m_reg1);
	m_sr.b.l |= FLAG_HF | FLAG_NF;
}


void tlcs900_device::op_CPLWR()
{
	reg16(m_reg1) = ~reg16(m_reg1);
	m_sr.b.l |= FLAG_HF | FLAG_NF;
}


void tlcs900_device::op_DAABR()
{
	const uint8_t oldval = reg8(m_reg1);
	const uint8_t high = reg8(m_reg1) & 0xF0;
	const uint8_t low = reg8(m_reg1) & 0x0F;
	uint8_t fixval = 0;
	uint8_t carry = 0;

	if (m_sr.b.l & FLAG_CF)
	{
		if (m_sr.b.l & FLAG_HF)
		{
			fixval = 0x66;
		}
		else
		{
			if (low < 0x0A)
				fixval = 0x60;
			else
				fixval = 0x66;
		}
		carry = 1;
	}
	else
	{
		if (m_sr.b.l & FLAG_HF)
		{
			if (reg8(m_reg1) < 0x9A)
				fixval = 0x06;
			else
				fixval = 0x66;
		}
		else
		{
			if (high < 0x90 && low > 0x09)
				fixval = 0x06;
			else if (high > 0x80 && low > 0x09)
				fixval = 0x66;
			else if (high > 0x90 && low < 0x0A)
				fixval = 0x60;
		}
	}
	m_sr.b.l &= ~(FLAG_VF | FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_CF);
	if (m_sr.b.l & FLAG_NF)
	{
		/* after SUB, SBC, or NEG operation */
		reg8(m_reg1) -= fixval;
		m_sr.b.l |= ((reg8(m_reg1) > oldval || carry) ? FLAG_CF : 0);
	}
	else
	{
		/* after ADD or ADC operation */
		reg8(m_reg1) += fixval;
		m_sr.b.l |= ((reg8(m_reg1) < oldval || carry) ? FLAG_CF : 0);
	}
	m_sr.b.l |= (reg8(m_reg1) & FLAG_SF) | (reg8(m_reg1) ? 0 : FLAG_ZF) |
		(((oldval ^ fixval) ^ reg8(m_reg1)) & FLAG_HF);

	parity<uint8_t>(reg8(m_reg1));
}


void tlcs900_device::op_DB()
{
	logerror("%08x: invalid or illegal instruction\n", m_pc.d);
}


void tlcs900_device::op_DECBIM()
{
	const uint8_t cy = m_sr.b.l & FLAG_CF;

	WRMEM(m_ea2.d, sub<uint8_t>(RDMEM(m_ea2.d), m_imm1.b.l ? m_imm1.b.l : 8));
	m_sr.b.l = (m_sr.b.l & ~FLAG_CF) | cy;
}


void tlcs900_device::op_DECBIR()
{
	const uint8_t cy = m_sr.b.l & FLAG_CF;

	reg8(m_reg2) = sub<uint8_t>(reg8(m_reg2), m_imm1.b.l ? m_imm1.b.l : 8);
	m_sr.b.l = (m_sr.b.l & ~FLAG_CF) | cy;
}


void tlcs900_device::op_DECWIM()
{
	const uint8_t cy = m_sr.b.l & FLAG_CF;

	WRMEMW(m_ea2.d, sub<uint16_t>(RDMEMW(m_ea2.d), m_imm1.b.l ? m_imm1.b.l : 8));
	m_sr.b.l = (m_sr.b.l & ~FLAG_CF) | cy;
}


void tlcs900_device::op_DECWIR()
{
	reg16(m_reg2) -= m_imm1.b.l ? m_imm1.b.l : 8;
}


void tlcs900_device::op_DECLIR()
{
	reg32(m_reg2) -= m_imm1.b.l ? m_imm1.b.l : 8;
}


void tlcs900_device::op_DECF()
{
	/* 0x03 for MAX mode, 0x07 for MIN mode */
	m_sr.b.h = (m_sr.b.h & 0xF8) | ((m_sr.b.h - 1) & 0x07);
	m_regbank = m_sr.b.h & 0x03;
}


void tlcs900_device::op_DIVBRI()
{
	reg16(m_reg1) = div8(reg16(m_reg1), m_imm2.b.l);
}


void tlcs900_device::op_DIVBRM()
{
	reg16(m_reg1) = div8(reg16(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_DIVBRR()
{
	reg16(m_reg1) = div8(reg16(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_DIVWRI()
{
	reg32(m_reg1) = div16(reg32(m_reg1), m_imm2.w.l);
}


void tlcs900_device::op_DIVWRM()
{
	reg32(m_reg1) = div16(reg32(m_reg1), RDMEMW(m_ea2.d));
}


void tlcs900_device::op_DIVWRR()
{
	reg32(m_reg1) = div16(reg32(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_DIVSBRI()
{
	reg16(m_reg1) = divs8(reg16(m_reg1), m_imm2.b.l);
}


void tlcs900_device::op_DIVSBRM()
{
	reg16(m_reg1) = divs8(reg16(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_DIVSBRR()
{
	reg16(m_reg1) = divs8(reg16(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_DIVSWRI()
{
	reg32(m_reg1) = divs16(reg32(m_reg1), m_imm2.w.l);
}


void tlcs900_device::op_DIVSWRM()
{
	reg32(m_reg1) = divs16(reg32(m_reg1), RDMEMW(m_ea2.d));
}


void tlcs900_device::op_DIVSWRR()
{
	reg32(m_reg1) = divs16(reg32(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_DJNZB()
{
	reg8(m_reg1) -= 1;
	if (reg8(m_reg1))
	{
		m_pc.d = m_ea2.d;
		m_cycles += tlcs900_djnz_true_cycles();
		m_prefetch_clear = true;
	}
}


void tlcs900_device::op_DJNZW()
{
	reg16(m_reg1) -= 1;
	if (reg16(m_reg1))
	{
		m_pc.d = m_ea2.d;
		m_cycles += tlcs900_djnz_true_cycles();
		m_prefetch_clear = true;
	}
}


void tlcs900_device::op_EI()
{
	m_sr.b.h = (m_sr.b.h & 0x8F) | ((m_imm1.b.l & 0x07) << 4);
	m_check_irqs = 1;
	m_irq_inhibit = true;  /* defer interrupt acceptance by 1 instruction */
}


void tlcs900_device::op_EXBMR()
{
	const uint8_t i = RDMEM(m_ea1.d);

	WRMEM(m_ea1.d, reg8(m_reg2));
	reg8(m_reg2) = i;
}


void tlcs900_device::op_EXBRR()
{
	const uint8_t i = reg8(m_reg2);

	reg8(m_reg2) = reg8(m_reg1);
	reg8(m_reg1) = i;
}


void tlcs900_device::op_EXWMR()
{
	const uint16_t i = RDMEMW(m_ea1.d);

	WRMEMW(m_ea1.d, reg16(m_reg2));
	reg16(m_reg2) = i;
}


void tlcs900_device::op_EXWRR()
{
	const uint16_t i = reg16(m_reg2);

	reg16(m_reg2) = reg16(m_reg1);
	reg16(m_reg1) = i;
}


void tlcs900_device::op_EXTSWR()
{
	if (BIT(reg16(m_reg1), 7))
		reg16(m_reg1) |= 0xFF00;
	else
		reg16(m_reg1) &= 0x00FF;
}


void tlcs900_device::op_EXTSLR()
{
	if (BIT(reg32(m_reg1), 15))
		reg32(m_reg1) |= 0xFFFF'0000;
	else
		reg32(m_reg1) &= 0x0000'FFFF;
}


void tlcs900_device::op_EXTZWR()
{
	reg16(m_reg1) &= 0x00FF;
}


void tlcs900_device::op_EXTZLR()
{
	reg32(m_reg1) &= 0x0000'FFFF;
}


void tlcs900_device::op_HALT()
{
	m_halted = 1;
}


void tlcs900_device::op_INCBIM()
{
	const uint8_t cy = m_sr.b.l & FLAG_CF;

	WRMEM(m_ea2.d, add<uint8_t>(RDMEM(m_ea2.d), m_imm1.b.l ? m_imm1.b.l : 8));
	m_sr.b.l = (m_sr.b.l & ~FLAG_CF) | cy;
}


void tlcs900_device::op_INCBIR()
{
	const uint8_t cy = m_sr.b.l & FLAG_CF;

	reg8(m_reg2) = add<uint8_t>(reg8(m_reg2), m_imm1.b.l ? m_imm1.b.l : 8);
	m_sr.b.l = (m_sr.b.l & ~FLAG_CF) | cy;
}


void tlcs900_device::op_INCWIM()
{
	const uint8_t cy = m_sr.b.l & FLAG_CF;

	WRMEMW(m_ea2.d, add<uint16_t>(RDMEMW(m_ea2.d), m_imm1.b.l ? m_imm1.b.l : 8));
	m_sr.b.l = (m_sr.b.l & ~FLAG_CF) | cy;
}


void tlcs900_device::op_INCWIR()
{
	reg16(m_reg2) += m_imm1.b.l ? m_imm1.b.l : 8;
}


void tlcs900_device::op_INCLIR()
{
	reg32(m_reg2) += m_imm1.b.l ? m_imm1.b.l : 8;
}


void tlcs900_device::op_INCF()
{
	/* 0x03 for MAX mode, 0x07 for MIN mode */
	m_sr.b.h = (m_sr.b.h & 0xF8) | ((m_sr.b.h + 1) & 0x07);
	m_regbank = m_sr.b.h & 0x03;
}


void tlcs900_device::op_JPI()
{
	m_pc.d = m_imm1.d;
	m_prefetch_clear = true;
}


void tlcs900_device::op_JPM()
{
	if (condition_true(m_op))
	{
		m_pc.d = m_ea2.d;
		m_cycles += tlcs900_jp_true_cycles();
		m_prefetch_clear = true;
	}
}


void tlcs900_device::op_JR()
{
	if (condition_true(m_op))
	{
		m_pc.d = m_ea2.d;
		m_cycles += tlcs900_jp_true_cycles();
		m_prefetch_clear = true;
	}
}


void tlcs900_device::op_JRL()
{
	if (condition_true(m_op))
	{
		m_pc.d = m_ea2.d;
		m_cycles += tlcs900_jp_true_cycles();
		m_prefetch_clear = true;
	}
}


void tlcs900_device::op_LDBMI()
{
	WRMEM(m_ea1.d, m_imm2.b.l);
}


void tlcs900_device::op_LDBMM()
{
	WRMEM(m_ea1.d, RDMEM(m_ea2.d));
}


void tlcs900_device::op_LDBMR()
{
	WRMEM(m_ea1.d, reg8(m_reg2));
}


void tlcs900_device::op_LDBRI()
{
	reg8(m_reg1) = m_imm2.b.l;
}


void tlcs900_device::op_LDBRM()
{
	reg8(m_reg1) = RDMEM(m_ea2.d);
}


void tlcs900_device::op_LDBRR()
{
	reg8(m_reg1) = reg8(m_reg2);
}


void tlcs900_device::op_LDWMI()
{
	WRMEMW(m_ea1.d, m_imm2.w.l);
}


void tlcs900_device::op_LDWMM()
{
	WRMEMW(m_ea1.d, RDMEMW(m_ea2.d));
}


void tlcs900_device::op_LDWMR()
{
	WRMEMW(m_ea1.d, reg16(m_reg2));
}


void tlcs900_device::op_LDWRI()
{
	reg16(m_reg1) = m_imm2.w.l;
}


void tlcs900_device::op_LDWRM()
{
	reg16(m_reg1) = RDMEMW(m_ea2.d);
}


void tlcs900_device::op_LDWRR()
{
	reg16(m_reg1) = reg16(m_reg2);
}


void tlcs900_device::op_LDLRI()
{
	reg32(m_reg1) = m_imm2.d;
}


void tlcs900_device::op_LDLRM()
{
	reg32(m_reg1) = RDMEML(m_ea2.d);
}


void tlcs900_device::op_LDLRR()
{
	reg32(m_reg1) = reg32(m_reg2);
}


void tlcs900_device::op_LDLMR()
{
	WRMEML(m_ea1.d, reg32(m_reg2));
}


void tlcs900_device::op_LDAW()
{
	reg16(m_reg1) = m_ea2.w.l;
}


void tlcs900_device::op_LDAL()
{
	reg32(m_reg1) = m_ea2.d;
}


void tlcs900_device::op_LDCBRR()
{
	reg8(m_reg1) = reg8(m_reg2);
}


void tlcs900_device::op_LDCWRR()
{
	reg16(m_reg1) = reg16(m_reg2);
}


void tlcs900_device::op_LDCLRR()
{
	reg32(m_reg1) = reg32(m_reg2);
}


void tlcs900_device::op_LDCFBIM()
{
	ldcf<uint8_t>(m_imm1.b.l, RDMEM(m_ea2.d));
}


void tlcs900_device::op_LDCFBIR()
{
	ldcf<uint8_t>(m_imm1.b.l, reg8(m_reg2));
}


void tlcs900_device::op_LDCFBRM()
{
	ldcf<uint8_t>(reg8(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_LDCFBRR()
{
	ldcf<uint8_t>(reg8(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_LDCFWIR()
{
	ldcf<uint16_t>(m_imm1.b.l, reg16(m_reg2));
}


void tlcs900_device::op_LDCFWRR()
{
	ldcf<uint16_t>(reg8(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_LDD()
{
	ldxx<uint8_t, -1, false>();
}


void tlcs900_device::op_LDDR()
{
	ldxx<uint8_t, -1, true>();
}


void tlcs900_device::op_LDDRW()
{
	ldxx<uint16_t, -1, true>();
}


void tlcs900_device::op_LDDW()
{
	ldxx<uint16_t, -1, false>();
}


void tlcs900_device::op_LDF()
{
	m_sr.b.h = (m_sr.b.h & 0xF8) | (m_imm1.b.l & 0x07);
	m_regbank = m_imm1.b.l & 0x03;
}


void tlcs900_device::op_LDI()
{
	ldxx<uint8_t, 1, false>();
}


void tlcs900_device::op_LDIR()
{
	ldxx<uint8_t, 1, true>();
}


void tlcs900_device::op_LDIRW()
{
	ldxx<uint16_t, 1, true>();
}


void tlcs900_device::op_LDIW()
{
	ldxx<uint16_t, 1, false>();
}


void tlcs900_device::op_LDX()
{
	RDOP();
	const uint8_t a = RDOP();
	RDOP();
	const uint8_t b = RDOP();
	RDOP();
	WRMEM(a, b);
}


void tlcs900_device::op_LINK()
{
	m_xssp.d -= 4;
	WRMEML(m_xssp.d, reg32(m_reg1));
	reg32(m_reg1) = m_xssp.d;
	m_xssp.d += m_imm2.sw.l;
}


void tlcs900_device::op_MAX()
{
	m_sr.b.h |= 0x08;
}


void tlcs900_device::op_MDEC1()
{
	if ((reg16(m_reg2) & m_imm1.w.l) == m_imm1.w.l)
		reg16(m_reg2) += m_imm1.w.l;
	else
		reg16(m_reg2) -= 1;
}


void tlcs900_device::op_MDEC2()
{
	if ((reg16(m_reg2) & m_imm1.w.l) == m_imm1.w.l)
		reg16(m_reg2) += m_imm1.w.l;
	else
		reg16(m_reg2) -= 2;
}


void tlcs900_device::op_MDEC4()
{
	if ((reg16(m_reg2) & m_imm1.w.l) == m_imm1.w.l)
		reg16(m_reg2) += m_imm1.w.l;
	else
		reg16(m_reg2) -= 4;
}


void tlcs900_device::op_MINC1()
{
	if ((reg16(m_reg2) & m_imm1.w.l) == m_imm1.w.l)
		reg16(m_reg2) -= m_imm1.w.l;
	else
		reg16(m_reg2) += 1;
}


void tlcs900_device::op_MINC2()
{
	if ((reg16(m_reg2) & m_imm1.w.l) == m_imm1.w.l)
		reg16(m_reg2) -= m_imm1.w.l;
	else
		reg16(m_reg2) += 2;
}


void tlcs900_device::op_MINC4()
{
	if ((reg16(m_reg2) & m_imm1.w.l) == m_imm1.w.l)
		reg16(m_reg2) -= m_imm1.w.l;
	else
		reg16(m_reg2) += 4;
}


void tlcs900_device::op_MIRRW()
{
	uint16_t r = reg16(m_reg1);
	uint16_t s = BIT(r, 0);

	for (int i = 0; i < 15; i++)
	{
		r >>= 1;
		s <<= 1;
		s |= BIT(r, 0);
	}

	reg16(m_reg1) = s;
}


void tlcs900_device::op_MULBRI()
{
	reg16(m_reg1) = (reg16(m_reg1) & 0xFF) * m_imm2.b.l;
}


void tlcs900_device::op_MULBRM()
{
	reg16(m_reg1) = (reg16(m_reg1) & 0xFF) * RDMEM(m_ea2.d);
}


void tlcs900_device::op_MULBRR()
{
	reg16(m_reg1) = (reg16(m_reg1) & 0xFF) * reg8(m_reg2);
}


void tlcs900_device::op_MULWRI()
{
	reg32(m_reg1) = (reg32(m_reg1) & 0xFFFF) * m_imm2.w.l;
}


void tlcs900_device::op_MULWRM()
{
	reg32(m_reg1) = (reg32(m_reg1) & 0xFFFF) * RDMEMW(m_ea2.d);
}


void tlcs900_device::op_MULWRR()
{
	reg32(m_reg1) = (reg32(m_reg1) & 0xFFFF) * reg16(m_reg2);
}


void tlcs900_device::op_MULAR()
{
	reg32(m_reg1) = reg32(m_reg1) +
		(int16_t(RDMEMW(m_xde[m_regbank].d)) * int16_t(RDMEMW(m_xhl[m_regbank].d)));
	m_xhl[m_regbank].d -= 2;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_VF);
	m_sr.b.l |= ((reg32(m_reg1) >> 24) & FLAG_SF) | (reg32(m_reg1) ? 0 : FLAG_ZF);
}


void tlcs900_device::op_MULSBRI()
{
	reg16(m_reg1) = int8_t(reg16(m_reg1) & 0xFF) * m_imm2.sb.l;
}


void tlcs900_device::op_MULSBRM()
{
	reg16(m_reg1) = int8_t(reg16(m_reg1) & 0xFF) * int8_t(RDMEM(m_ea2.d));
}


void tlcs900_device::op_MULSBRR()
{
	reg16(m_reg1) = int8_t(reg16(m_reg1) & 0xFF) * int8_t(reg8(m_reg2));
}


void tlcs900_device::op_MULSWRI()
{
	reg32(m_reg1) = int16_t(reg32(m_reg1) & 0xFFFF) * m_imm2.sw.l;
}


void tlcs900_device::op_MULSWRM()
{
	reg32(m_reg1) = int16_t(reg32(m_reg1) & 0xFFFF) * int16_t(RDMEMW(m_ea2.d));
}


void tlcs900_device::op_MULSWRR()
{
	reg32(m_reg1) = int16_t(reg32(m_reg1) & 0xFFFF) * int16_t(reg16(m_reg2));
}


void tlcs900_device::op_NEGBR()
{
	reg8(m_reg1) = sub<uint8_t>(0, reg8(m_reg1));
}


void tlcs900_device::op_NEGWR()
{
	reg16(m_reg1) = sub<uint16_t>(0, reg16(m_reg1));
}


void tlcs900_device::op_NOP()
{
	/* Do nothing */
}


void tlcs900_device::op_NORMAL()
{
	m_sr.b.h &= 0x7F;
}


void tlcs900_device::op_ORBMI()
{
	WRMEM(m_ea1.d, or_<uint8_t>(RDMEM(m_ea1.d), m_imm2.b.l));
}


void tlcs900_device::op_ORBMR()
{
	WRMEM(m_ea1.d, or_<uint8_t>(RDMEM(m_ea1.d), reg8(m_reg2)));
}


void tlcs900_device::op_ORBRI()
{
	reg8(m_reg1) = or_<uint8_t>(reg8(m_reg1), m_imm2.b.l);
}


void tlcs900_device::op_ORBRM()
{
	reg8(m_reg1) = or_<uint8_t>(reg8(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_ORBRR()
{
	reg8(m_reg1) = or_<uint8_t>(reg8(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_ORWMI()
{
	WRMEMW(m_ea1.d, or_<uint16_t>(RDMEMW(m_ea1.d), m_imm2.w.l));
}


void tlcs900_device::op_ORWMR()
{
	WRMEMW(m_ea1.d, or_<uint16_t>(RDMEMW(m_ea1.d), reg16(m_reg2)));
}


void tlcs900_device::op_ORWRI()
{
	reg16(m_reg1) = or_<uint16_t>(reg16(m_reg1), m_imm2.w.l);
}


void tlcs900_device::op_ORWRM()
{
	reg16(m_reg1) = or_<uint16_t>(reg16(m_reg1), RDMEMW(m_ea2.d));
}


void tlcs900_device::op_ORWRR()
{
	reg16(m_reg1) = or_<uint16_t>(reg16(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_ORLMR()
{
	WRMEML(m_ea1.d, or_<uint32_t>(RDMEML(m_ea1.d), reg32(m_reg2)));
}


void tlcs900_device::op_ORLRI()
{
	reg32(m_reg1) = or_<uint32_t>(reg32(m_reg1), m_imm2.d);
}


void tlcs900_device::op_ORLRM()
{
	reg32(m_reg1) = or_<uint32_t>(reg32(m_reg1), RDMEML(m_ea2.d));
}


void tlcs900_device::op_ORLRR()
{
	reg32(m_reg1) = or_<uint32_t>(reg32(m_reg1), reg32(m_reg2));
}


void tlcs900_device::op_ORCFBIM()
{
	orcf<uint8_t>(m_imm1.b.l, RDMEM(m_ea2.d));
}


void tlcs900_device::op_ORCFBIR()
{
	orcf<uint8_t>(m_imm1.b.l, reg8(m_reg2));
}


void tlcs900_device::op_ORCFBRM()
{
	orcf<uint8_t>(reg8(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_ORCFBRR()
{
	orcf<uint8_t>(reg8(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_ORCFWIR()
{
	orcf<uint16_t>(m_imm1.b.l, reg16(m_reg2));
}


void tlcs900_device::op_ORCFWRR()
{
	orcf<uint16_t>(reg8(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_PAAWR()
{
	if (BIT(reg16(m_reg1), 0))
		reg16(m_reg1) += 1;
}


void tlcs900_device::op_PAALR()
{
	if (BIT(reg32(m_reg1), 0))
		reg32(m_reg1) += 1;
}


void tlcs900_device::op_POPBM()
{
	WRMEM(m_ea1.d, RDMEM(m_xssp.d));
	m_xssp.d += 1;
}


void tlcs900_device::op_POPBR()
{
	reg8(m_reg1) = RDMEM(m_xssp.d);
	m_xssp.d += 1;
}


void tlcs900_device::op_POPWM()
{
	WRMEMW(m_ea1.d, RDMEMW(m_xssp.d));
	m_xssp.d += 2;
}


void tlcs900_device::op_POPWR()
{
	reg16(m_reg1) = RDMEMW(m_xssp.d);
	m_xssp.d += 2;
}


void tlcs900_device::op_POPWSR()
{
	op_POPWR();
	m_regbank = m_sr.b.h & 0x03;
	m_check_irqs = 1;
}


void tlcs900_device::op_POPLR()
{
	reg32(m_reg1) = RDMEML(m_xssp.d);
	m_xssp.d += 4;
}


void tlcs900_device::op_PUSHBI()
{
	m_xssp.d -= 1;
	WRMEM(m_xssp.d, m_imm1.b.l);
}


void tlcs900_device::op_PUSHBM()
{
	m_xssp.d -= 1;
	WRMEM(m_xssp.d, RDMEM(m_ea1.d));
}


void tlcs900_device::op_PUSHBR()
{
	m_xssp.d -= 1;
	WRMEM(m_xssp.d, reg8(m_reg1));
}


void tlcs900_device::op_PUSHWI()
{
	m_xssp.d -= 2;
	WRMEMW(m_xssp.d, m_imm1.w.l);
}


void tlcs900_device::op_PUSHWM()
{
	m_xssp.d -= 2;
	WRMEMW(m_xssp.d, RDMEMW(m_ea1.d));
}


void tlcs900_device::op_PUSHWR()
{
	m_xssp.d -= 2;
	WRMEMW(m_xssp.d, reg16(m_reg1));
}


void tlcs900_device::op_PUSHLR()
{
	m_xssp.d -= 4;
	WRMEML(m_xssp.d, reg32(m_reg1));
}


void tlcs900_device::op_RCF()
{
	m_sr.b.l &= ~(FLAG_HF | FLAG_NF | FLAG_CF);
}


void tlcs900_device::op_RESBIM()
{
	WRMEM(m_ea2.d, RDMEM(m_ea2.d) & ~(1 << (m_imm1.d & 0x07)));
}


void tlcs900_device::op_RESBIR()
{
	reg8(m_reg2) = reg8(m_reg2) & ~(1 << (m_imm1.d & 0x07));
}


void tlcs900_device::op_RESWIR()
{
	reg16(m_reg2) = reg16(m_reg2) & ~(1 << (m_imm1.d & 0x0F));
}


void tlcs900_device::op_RET()
{
	m_pc.d = RDMEML(m_xssp.d);
	m_xssp.d += 4;
	m_prefetch_clear = true;
}


void tlcs900_device::op_RETCC()
{
	if (condition_true(m_op))
	{
		m_pc.d = RDMEML(m_xssp.d);
		m_xssp.d += 4;
		m_cycles += tlcs900_call_true_cycles();
		m_prefetch_clear = true;
	}
}


void tlcs900_device::op_RETD()
{
	m_pc.d = RDMEML(m_xssp.d);
	m_xssp.d += 4 + m_imm1.sw.l;
	m_prefetch_clear = true;
}


void tlcs900_device::op_RETI()
{
	/* INTNEST: pair the decrement with the increment at interrupt acceptance.
	   Clamped, so an unmatched RETI reads as "not nested" rather than wrapping. */
	if (m_intnest)
		m_intnest--;

	m_sr.w.l = RDMEMW(m_xssp.d);
	m_xssp.d += 2;
	m_pc.d = RDMEML(m_xssp.d);
	m_xssp.d += 4;
	m_regbank = m_sr.b.h & 0x03;
	m_check_irqs = 1;
	m_irq_inhibit = true;  /* defer interrupt acceptance by 1 instruction */
	m_prefetch_clear = true;
}


void tlcs900_device::op_RLBM()
{
	WRMEM(m_ea2.d, rl<uint8_t>(RDMEM(m_ea2.d), 1));
}


void tlcs900_device::op_RLWM()
{
	WRMEMW(m_ea2.d, rl<uint16_t>(RDMEMW(m_ea2.d), 1));
}


void tlcs900_device::op_RLBIR()
{
	reg8(m_reg2) = rl<uint8_t>(reg8(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_RLBRR()
{
	reg8(m_reg2) = rl<uint8_t>(reg8(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_RLWIR()
{
	reg16(m_reg2) = rl<uint16_t>(reg16(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_RLWRR()
{
	reg16(m_reg2) = rl<uint16_t>(reg16(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_RLLIR()
{
	reg32(m_reg2) = rl<uint32_t>(reg32(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_RLLRR()
{
	reg32(m_reg2) = rl<uint32_t>(reg32(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_RLCBM()
{
	WRMEM(m_ea2.d, rlc<uint8_t>(RDMEM(m_ea2.d), 1));
}


void tlcs900_device::op_RLCWM()
{
	WRMEMW(m_ea2.d, rlc<uint16_t>(RDMEMW(m_ea2.d), 1));
}


void tlcs900_device::op_RLCBIR()
{
	reg8(m_reg2) = rlc<uint8_t>(reg8(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_RLCBRR()
{
	reg8(m_reg2) = rlc<uint8_t>(reg8(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_RLCWIR()
{
	reg16(m_reg2) = rlc<uint16_t>(reg16(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_RLCWRR()
{
	reg16(m_reg2) = rlc<uint16_t>(reg16(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_RLCLIR()
{
	reg32(m_reg2) = rlc<uint32_t>(reg32(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_RLCLRR()
{
	reg32(m_reg2) = rlc<uint32_t>(reg32(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_RLDRM()
{
	const uint8_t a = reg8(m_reg1) & 0x0F;
	const uint8_t b = RDMEM(m_ea2.d);

	reg8(m_reg1) = (reg8(m_reg1) & 0xF0) | ((b & 0xF0) >> 4);
	WRMEM(m_ea2.d, ((b & 0x0F) << 4) | a);
	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= (reg8(m_reg1) & FLAG_SF) | (reg8(m_reg1) ? 0 : FLAG_ZF);
	parity<uint8_t>(reg8(m_reg1));
}


void tlcs900_device::op_RRBM()
{
	WRMEM(m_ea2.d, rr<uint8_t>(RDMEM(m_ea2.d), 1));
}


void tlcs900_device::op_RRWM()
{
	WRMEMW(m_ea2.d, rr<uint16_t>(RDMEMW(m_ea2.d), 1));
}


void tlcs900_device::op_RRBIR()
{
	reg8(m_reg2) = rr<uint8_t>(reg8(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_RRBRR()
{
	reg8(m_reg2) = rr<uint8_t>(reg8(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_RRWIR()
{
	reg16(m_reg2) = rr<uint16_t>(reg16(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_RRWRR()
{
	reg16(m_reg2) = rr<uint16_t>(reg16(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_RRLIR()
{
	reg32(m_reg2) = rr<uint32_t>(reg32(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_RRLRR()
{
	reg32(m_reg2) = rr<uint32_t>(reg32(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_RRCBM()
{
	WRMEM(m_ea2.d, rrc<uint8_t>(RDMEM(m_ea2.d), 1));
}


void tlcs900_device::op_RRCWM()
{
	WRMEMW(m_ea2.d, rrc<uint16_t>(RDMEMW(m_ea2.d), 1));
}


void tlcs900_device::op_RRCBIR()
{
	reg8(m_reg2) = rrc<uint8_t>(reg8(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_RRCBRR()
{
	reg8(m_reg2) = rrc<uint8_t>(reg8(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_RRCWIR()
{
	reg16(m_reg2) = rrc<uint16_t>(reg16(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_RRCWRR()
{
	reg16(m_reg2) = rrc<uint16_t>(reg16(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_RRCLIR()
{
	reg32(m_reg2) = rrc<uint32_t>(reg32(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_RRCLRR()
{
	reg32(m_reg2) = rrc<uint32_t>(reg32(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_RRDRM()
{
	const uint8_t a = reg8(m_reg1) & 0x0F;
	const uint8_t b = RDMEM(m_ea2.d);

	reg8(m_reg1) = (reg8(m_reg1) & 0xF0) | (b & 0x0F);
	WRMEM(m_ea2.d, ((b & 0xF0) >> 4) | (a << 4));
	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= (reg8(m_reg1) & FLAG_SF) | (reg8(m_reg1) ? 0 : FLAG_ZF);
	parity<uint8_t>(reg8(m_reg1));
}


void tlcs900_device::op_SBCBMI()
{
	WRMEM(m_ea1.d, sbc<uint8_t>(RDMEM(m_ea1.d), m_imm2.b.l));
}


void tlcs900_device::op_SBCBMR()
{
	WRMEM(m_ea1.d, sbc<uint8_t>(RDMEM(m_ea1.d), reg8(m_reg2)));
}


void tlcs900_device::op_SBCBRI()
{
	reg8(m_reg1) = sbc<uint8_t>(reg8(m_reg1), m_imm2.b.l);
}


void tlcs900_device::op_SBCBRM()
{
	reg8(m_reg1) = sbc<uint8_t>(reg8(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_SBCBRR()
{
	reg8(m_reg1) = sbc<uint8_t>(reg8(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_SBCWMI()
{
	WRMEMW(m_ea1.d, sbc<uint16_t>(RDMEMW(m_ea1.d), m_imm2.w.l));
}


void tlcs900_device::op_SBCWMR()
{
	WRMEMW(m_ea1.d, sbc<uint16_t>(RDMEMW(m_ea1.d), reg16(m_reg2)));
}


void tlcs900_device::op_SBCWRI()
{
	reg16(m_reg1) = sbc<uint16_t>(reg16(m_reg1), m_imm2.w.l);
}


void tlcs900_device::op_SBCWRM()
{
	reg16(m_reg1) = sbc<uint16_t>(reg16(m_reg1), RDMEMW(m_ea2.d));
}


void tlcs900_device::op_SBCWRR()
{
	reg16(m_reg1) = sbc<uint16_t>(reg16(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_SBCLMR()
{
	WRMEML(m_ea1.d, sbc<uint32_t>(RDMEML(m_ea1.d), reg32(m_reg2)));
}


void tlcs900_device::op_SBCLRI()
{
	reg32(m_reg1) = sbc<uint32_t>(reg32(m_reg1), m_imm2.d);
}


void tlcs900_device::op_SBCLRM()
{
	reg32(m_reg1) = sbc<uint32_t>(reg32(m_reg1), RDMEML(m_ea2.d));
}


void tlcs900_device::op_SBCLRR()
{
	reg32(m_reg1) = sbc<uint32_t>(reg32(m_reg1), reg32(m_reg2));
}


void tlcs900_device::op_SCCBR()
{
	reg8(m_reg2) = condition_true(m_op) ? 1 : 0;
}


void tlcs900_device::op_SCCWR()
{
	reg16(m_reg2) = condition_true(m_op) ? 1 : 0;
}


void tlcs900_device::op_SCF()
{
	m_sr.b.l &= ~(FLAG_HF | FLAG_NF);
	m_sr.b.l |= FLAG_CF;
}


void tlcs900_device::op_SETBIM()
{
	WRMEM(m_ea2.d, RDMEM(m_ea2.d) | (1 << (m_imm1.d & 0x07)));
}


void tlcs900_device::op_SETBIR()
{
	reg8(m_reg2) = reg8(m_reg2) | (1 << (m_imm1.d & 0x07));
}


void tlcs900_device::op_SETWIR()
{
	reg16(m_reg2) = reg16(m_reg2) | (1 << (m_imm1.d & 0x0F));
}


void tlcs900_device::op_SLABM()
{
	WRMEM(m_ea2.d, sla<uint8_t>(RDMEM(m_ea2.d), 1));
}


void tlcs900_device::op_SLAWM()
{
	WRMEMW(m_ea2.d, sla<uint16_t>(RDMEMW(m_ea2.d), 1));
}


void tlcs900_device::op_SLABIR()
{
	reg8(m_reg2) = sla<uint8_t>(reg8(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_SLABRR()
{
	reg8(m_reg2) = sla<uint8_t>(reg8(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_SLAWIR()
{
	reg16(m_reg2) = sla<uint16_t>(reg16(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_SLAWRR()
{
	reg16(m_reg2) = sla<uint16_t>(reg16(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_SLALIR()
{
	reg32(m_reg2) = sla<uint32_t>(reg32(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_SLALRR()
{
	reg32(m_reg2) = sla<uint32_t>(reg32(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_SLLBM()
{
	WRMEM(m_ea2.d, sla<uint8_t>(RDMEM(m_ea2.d), 1));
}


void tlcs900_device::op_SLLWM()
{
	WRMEMW(m_ea2.d, sla<uint16_t>(RDMEMW(m_ea2.d), 1));
}


void tlcs900_device::op_SLLBIR()
{
	reg8(m_reg2) = sla<uint8_t>(reg8(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_SLLBRR()
{
	reg8(m_reg2) = sla<uint8_t>(reg8(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_SLLWIR()
{
	reg16(m_reg2) = sla<uint16_t>(reg16(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_SLLWRR()
{
	reg16(m_reg2) = sla<uint16_t>(reg16(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_SLLLIR()
{
	reg32(m_reg2) = sla<uint32_t>(reg32(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_SLLLRR()
{
	reg32(m_reg2) = sla<uint32_t>(reg32(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_SRABM()
{
	WRMEM(m_ea2.d, sra<uint8_t>(RDMEM(m_ea2.d), 1));
}


void tlcs900_device::op_SRAWM()
{
	WRMEMW(m_ea2.d, sra<uint16_t>(RDMEMW(m_ea2.d), 1));
}


void tlcs900_device::op_SRABIR()
{
	reg8(m_reg2) = sra<uint8_t>(reg8(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_SRABRR()
{
	reg8(m_reg2) = sra<uint8_t>(reg8(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_SRAWIR()
{
	reg16(m_reg2) = sra<uint16_t>(reg16(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_SRAWRR()
{
	reg16(m_reg2) = sra<uint16_t>(reg16(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_SRALIR()
{
	reg32(m_reg2) = sra<uint32_t>(reg32(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_SRALRR()
{
	reg32(m_reg2) = sra<uint32_t>(reg32(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_SRLBM()
{
	WRMEM(m_ea2.d, srl<uint8_t>(RDMEM(m_ea2.d), 1));
}


void tlcs900_device::op_SRLWM()
{
	WRMEMW(m_ea2.d, srl<uint16_t>(RDMEMW(m_ea2.d), 1));
}


void tlcs900_device::op_SRLBIR()
{
	reg8(m_reg2) = srl<uint8_t>(reg8(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_SRLBRR()
{
	reg8(m_reg2) = srl<uint8_t>(reg8(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_SRLWIR()
{
	reg16(m_reg2) = srl<uint16_t>(reg16(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_SRLWRR()
{
	reg16(m_reg2) = srl<uint16_t>(reg16(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_SRLLIR()
{
	reg32(m_reg2) = srl<uint32_t>(reg32(m_reg2), m_imm1.b.l);
}


void tlcs900_device::op_SRLLRR()
{
	reg32(m_reg2) = srl<uint32_t>(reg32(m_reg2), reg8(m_reg1));
}


void tlcs900_device::op_STCFBIM()
{
	if (m_sr.b.l & FLAG_CF)
		WRMEM(m_ea2.d, RDMEM(m_ea2.d) | (1 << (m_imm1.b.l & 0x07)));
	else
		WRMEM(m_ea2.d, RDMEM(m_ea2.d) & ~(1 << (m_imm1.b.l & 0x07)));
}


void tlcs900_device::op_STCFBIR()
{
	if (m_sr.b.l & FLAG_CF)
		reg8(m_reg2) |= (1 << (m_imm1.b.l & 0x07));
	else
		reg8(m_reg2) &= ~(1 << (m_imm1.b.l & 0x07));
}


void tlcs900_device::op_STCFBRM()
{
	if (m_sr.b.l & FLAG_CF)
		WRMEM(m_ea2.d, RDMEM(m_ea2.d) | (1 << (reg8(m_reg1) & 0x07)));
	else
		WRMEM(m_ea2.d, RDMEM(m_ea2.d) & ~(1 << (reg8(m_reg1) & 0x07)));
}


void tlcs900_device::op_STCFBRR()
{
	if (m_sr.b.l & FLAG_CF)
		reg8(m_reg2) |= (1 << (reg8(m_reg1) & 0x07));
	else
		reg8(m_reg2) &= ~(1 << (reg8(m_reg1) & 0x07));
}


void tlcs900_device::op_STCFWIR()
{
	if (m_sr.b.l & FLAG_CF)
		reg16(m_reg2) |= (1 << (m_imm1.b.l & 0x0F));
	else
		reg16(m_reg2) &= ~(1 << (m_imm1.b.l & 0x0F));
}


void tlcs900_device::op_STCFWRR()
{
	if (m_sr.b.l & FLAG_CF)
		reg16(m_reg2) |= (1 << (reg8(m_reg1) & 0x0F));
	else
		reg16(m_reg2) &= ~(1 << (reg8(m_reg1) & 0x0F));
}


void tlcs900_device::op_SUBBMI()
{
	WRMEM(m_ea1.d, sub<uint8_t>(RDMEM(m_ea1.d), m_imm2.b.l));
}


void tlcs900_device::op_SUBBMR()
{
	WRMEM(m_ea1.d, sub<uint8_t>(RDMEM(m_ea1.d), reg8(m_reg2)));
}


void tlcs900_device::op_SUBBRI()
{
	reg8(m_reg1) = sub<uint8_t>(reg8(m_reg1), m_imm2.b.l);
}


void tlcs900_device::op_SUBBRM()
{
	reg8(m_reg1) = sub<uint8_t>(reg8(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_SUBBRR()
{
	reg8(m_reg1) = sub<uint8_t>(reg8(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_SUBWMI()
{
	WRMEMW(m_ea1.d, sub<uint16_t>(RDMEMW(m_ea1.d), m_imm2.w.l));
}


void tlcs900_device::op_SUBWMR()
{
	WRMEMW(m_ea1.d, sub<uint16_t>(RDMEMW(m_ea1.d), reg16(m_reg2)));
}


void tlcs900_device::op_SUBWRI()
{
	reg16(m_reg1) = sub<uint16_t>(reg16(m_reg1), m_imm2.w.l);
}


void tlcs900_device::op_SUBWRM()
{
	reg16(m_reg1) = sub<uint16_t>(reg16(m_reg1), RDMEMW(m_ea2.d));
}


void tlcs900_device::op_SUBWRR()
{
	reg16(m_reg1) = sub<uint16_t>(reg16(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_SUBLMR()
{
	WRMEML(m_ea1.d, sub<uint32_t>(RDMEML(m_ea1.d), reg32(m_reg2)));
}


void tlcs900_device::op_SUBLRI()
{
	reg32(m_reg1) = sub<uint32_t>(reg32(m_reg1), m_imm2.d);
}


void tlcs900_device::op_SUBLRM()
{
	reg32(m_reg1) = sub<uint32_t>(reg32(m_reg1), RDMEML(m_ea2.d));
}


void tlcs900_device::op_SUBLRR()
{
	reg32(m_reg1) = sub<uint32_t>(reg32(m_reg1), reg32(m_reg2));
}


void tlcs900_device::op_SWI()
{
	m_xssp.d -= 4;
	WRMEML(m_xssp.d, m_pc.d);
	m_xssp.d -= 2;
	WRMEMW(m_xssp.d, m_sr.w.l);
	m_pc.d = RDMEML(0x00FFFF00 + 4 * m_imm1.b.l);
	m_prefetch_clear = true;
}


void tlcs900_device::op_SWI900()
{
	m_xssp.d -= 4;
	WRMEML(m_xssp.d, m_pc.d);
	m_xssp.d -= 2;
	WRMEMW(m_xssp.d, m_sr.w.l);
	m_pc.d = 0x0000'8000 + 0x10 * m_imm1.b.l;
	m_prefetch_clear = true;
}


void tlcs900_device::op_TSETBIM()
{
	const uint8_t b = 1 << (m_imm1.b.l & 0x07);
	const uint8_t a = RDMEM(m_ea2.d);

	m_sr.b.l &= ~(FLAG_ZF | FLAG_NF);
	m_sr.b.l |= ((a & b) ? 0 : FLAG_ZF) | FLAG_HF;
	WRMEM(m_ea2.d, a | b);
}


void tlcs900_device::op_TSETBIR()
{
	uint8_t b = 1 << (m_imm1.b.l & 0x07);

	m_sr.b.l &= ~(FLAG_ZF | FLAG_NF);
	m_sr.b.l |= ((reg8(m_reg2) & b) ? 0 : FLAG_ZF) | FLAG_HF;
	reg8(m_reg2) |= b;
}


void tlcs900_device::op_TSETWIR()
{
	uint16_t b = 1 << (m_imm1.b.l & 0x0F);

	m_sr.b.l &= ~(FLAG_ZF | FLAG_NF);
	m_sr.b.l |= ((reg16(m_reg2) & b) ? 0 : FLAG_ZF) | FLAG_HF;
	reg16(m_reg2) |= b;
}


void tlcs900_device::op_UNLK()
{
	m_xssp.d = reg32(m_reg1);
	reg32(m_reg1) = RDMEML(m_xssp.d);
	m_xssp.d += 4;
}


void tlcs900_device::op_XORBMI()
{
	WRMEM(m_ea1.d, xor_<uint8_t>(RDMEM(m_ea1.d), m_imm2.b.l));
}


void tlcs900_device::op_XORBMR()
{
	WRMEM(m_ea1.d, xor_<uint8_t>(RDMEM(m_ea1.d), reg8(m_reg2)));
}


void tlcs900_device::op_XORBRI()
{
	reg8(m_reg1) = xor_<uint8_t>(reg8(m_reg1), m_imm2.b.l);
}


void tlcs900_device::op_XORBRM()
{
	reg8(m_reg1) = xor_<uint8_t>(reg8(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_XORBRR()
{
	reg8(m_reg1) = xor_<uint8_t>(reg8(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_XORWMI()
{
	WRMEMW(m_ea1.d, xor_<uint16_t>(RDMEMW(m_ea1.d), m_imm2.w.l));
}


void tlcs900_device::op_XORWMR()
{
	WRMEMW(m_ea1.d, xor_<uint16_t>(RDMEMW(m_ea1.d), reg16(m_reg2)));
}


void tlcs900_device::op_XORWRI()
{
	reg16(m_reg1) = xor_<uint16_t>(reg16(m_reg1), m_imm2.w.l);
}


void tlcs900_device::op_XORWRM()
{
	reg16(m_reg1) = xor_<uint16_t>(reg16(m_reg1), RDMEMW(m_ea2.d));
}


void tlcs900_device::op_XORWRR()
{
	reg16(m_reg1) = xor_<uint16_t>(reg16(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_XORLMR()
{
	WRMEML(m_ea1.d, xor_<uint32_t>(RDMEML(m_ea1.d), reg32(m_reg2)));
}


void tlcs900_device::op_XORLRI()
{
	reg32(m_reg1) = xor_<uint32_t>(reg32(m_reg1), m_imm2.d);
}


void tlcs900_device::op_XORLRM()
{
	reg32(m_reg1) = xor_<uint32_t>(reg32(m_reg1), RDMEML(m_ea2.d));
}


void tlcs900_device::op_XORLRR()
{
	reg32(m_reg1) = xor_<uint32_t>(reg32(m_reg1), reg32(m_reg2));
}


void tlcs900_device::op_XORCFBIM()
{
	xorcf<uint8_t>(m_imm1.b.l, RDMEM(m_ea2.d));
}


void tlcs900_device::op_XORCFBIR()
{
	xorcf<uint8_t>(m_imm1.b.l, reg8(m_reg2));
}


void tlcs900_device::op_XORCFBRM()
{
	xorcf<uint8_t>(reg8(m_reg1), RDMEM(m_ea2.d));
}


void tlcs900_device::op_XORCFBRR()
{
	xorcf<uint8_t>(reg8(m_reg1), reg8(m_reg2));
}


void tlcs900_device::op_XORCFWIR()
{
	xorcf<uint16_t>(m_imm1.b.l, reg16(m_reg2));
}


void tlcs900_device::op_XORCFWRR()
{
	xorcf<uint16_t>(reg8(m_reg1), reg16(m_reg2));
}


void tlcs900_device::op_ZCF()
{
	m_sr.b.l &= ~(FLAG_NF | FLAG_CF);
	m_sr.b.l |= ((m_sr.b.l & FLAG_ZF) ? 0 : FLAG_CF);
}


void tlcs900_device::prepare_operands(const tlcs900inst &inst)
{
	switch (inst.operand1)
	{
	case p_A:
		m_reg1 = REGSEL_A;
		break;
	case p_F:
		m_reg1 = regsel_sr;
		break;
	case p_SR:
		m_reg1 = regsel_sr;
		break;
	case p_C8:
		m_reg1 = get_reg8_current_sel(m_op);
		break;
	case p_C16:
		m_reg1 = get_reg16_current_sel(m_op);
		break;
	case p_MC16: /* For MUL and DIV operations */
		m_reg1 = get_reg16_current_sel((m_op >> 1) & 0x03);
		break;
	case p_C32:
		m_reg1 = get_reg32_current_sel(m_op);
		break;
	case p_CR8:
		m_imm1.d = RDOP();
		switch (m_imm1.d)
		{
		case 0x22:  // TMP96C141/TMP95C061/TMP95C063
			m_reg1 = regsel_dmam0;
			break;
		case 0x26:  // TMP96C141/TMP95C061/TMP95C063
			m_reg1 = regsel_dmam1;
			break;
		case 0x2A:  // TMP96C141/TMP95C061/TMP95C063
			m_reg1 = regsel_dmam2;
			break;
		case 0x2E:  // TMP96C141/TMP95C061/TMP95C063
			m_reg1 = regsel_dmam3;
			break;
		case 0x42:  // TMP94C241
			m_reg1 = regsel_dmam0;
			break;
		case 0x46:  // TMP94C241
			m_reg1 = regsel_dmam1;
			break;
		case 0x4A:  // TMP94C241
			m_reg1 = regsel_dmam2;
			break;
		case 0x4E:  // TMP94C241
			m_reg1 = regsel_dmam3;
			break;
		default:
			m_reg1 = regsel_dummy;
			break;
		}
		break;
	case p_CR16:
		m_imm1.d = RDOP();
		switch (m_imm1.d)
		{
		case 0x20:  // TMP96C141/TMP95C061/TMP95C063
			m_reg1 = regsel_dmac0;
			break;
		case 0x24:  // TMP96C141/TMP95C061/TMP95C063
			m_reg1 = regsel_dmac1;
			break;
		case 0x28:  // TMP96C141/TMP95C061/TMP95C063
			m_reg1 = regsel_dmac2;
			break;
		case 0x2C:  // TMP96C141/TMP95C061/TMP95C063
			m_reg1 = regsel_dmac3;
			break;
		case 0x40:  // TMP94C241
			m_reg1 = regsel_dmac0;
			break;
		case 0x44:  // TMP94C241
			m_reg1 = regsel_dmac1;
			break;
		case 0x48:  // TMP94C241
			m_reg1 = regsel_dmac2;
			break;
		case 0x4C:  // TMP94C241
			m_reg1 = regsel_dmac3;
			break;
		case 0x3C:  // TMP96C141/TMP95C061/TMP95C063 -- INTNEST
		case 0x7C:  // TMP94C241 -- INTNEST
			m_reg1 = regsel_intnest;
			break;
		default:
			m_reg1 = regsel_dummy;
			break;
		}
		break;
	case p_CR32:
		m_imm1.d = RDOP();
		switch (m_imm1.d)
		{
		case 0x00:  // all variants
			m_reg1 = regsel_dmas0;
			break;
		case 0x04:  // all variants
			m_reg1 = regsel_dmas1;
			break;
		case 0x08:  // all variants
			m_reg1 = regsel_dmas2;
			break;
		case 0x0C:  // all variants
			m_reg1 = regsel_dmas3;
			break;
		case 0x10:  // TMP96C141/TMP95C061/TMP95C063
			m_reg1 = regsel_dmad0;
			break;
		case 0x14:  // TMP96C141/TMP95C061/TMP95C063
			m_reg1 = regsel_dmad1;
			break;
		case 0x18:  // TMP96C141/TMP95C061/TMP95C063
			m_reg1 = regsel_dmad2;
			break;
		case 0x1C:  // TMP96C141/TMP95C061/TMP95C063
			m_reg1 = regsel_dmad3;
			break;
		case 0x20:  // TMP94C241
			m_reg1 = regsel_dmad0;
			break;
		case 0x24:  // TMP94C241
			m_reg1 = regsel_dmad1;
			break;
		case 0x28:  // TMP94C241
			m_reg1 = regsel_dmad2;
			break;
		case 0x2C:  // TMP94C241
			m_reg1 = regsel_dmad3;
			break;
		default:
			m_reg1 = regsel_dummy;
			break;
		}
		break;
	case p_D8:
		m_ea1.d = RDOP();
		m_ea1.d = m_pc.d + m_ea1.sb.l;
		break;
	case p_D16:
		m_ea1.d = RDOP();
		m_ea1.b.h = RDOP();
		m_ea1.d = m_pc.d + m_ea1.sw.l;
		break;
	case p_I3:
		m_imm1.d = m_op & 0x07;
		break;
	case p_I8:
		m_imm1.d = RDOP();
		break;
	case p_I16:
		m_imm1.d = RDOP();
		m_imm1.b.h = RDOP();
		break;
	case p_I24:
		m_imm1.d = RDOP();
		m_imm1.b.h = RDOP();
		m_imm1.b.h2 = RDOP();
		break;
	case p_I32:
		m_imm1.d = RDOP();
		m_imm1.b.h = RDOP();
		m_imm1.b.h2 = RDOP();
		m_imm1.b.h3 = RDOP();
		break;
	case p_M:
		m_ea1.d = m_ea2.d;
		break;
	case p_M8:
		m_ea1.d = RDOP();
		break;
	case p_M16:
		m_ea1.d = RDOP();
		m_ea1.b.h = RDOP();
		break;
	case p_R:
		m_reg1 = m_reg2;
		break;
	}

	switch (inst.operand2)
	{
	case p_A:
		m_reg2 = REGSEL_A;
		break;
	case p_F:        /* F' */
		m_reg2 = regsel_f2;
		break;
	case p_SR:
		m_reg2 = regsel_sr;
		break;
	case p_C8:
		m_reg2 = get_reg8_current_sel(m_op);
		break;
	case p_C16:
		m_reg2 = get_reg16_current_sel(m_op);
		break;
	case p_C32:
		m_reg2 = get_reg32_current_sel(m_op);
		break;
	case p_CR8:
		m_imm1.d = RDOP();
		switch (m_imm1.d)
		{
		case 0x22:  // TMP96C141/TMP95C061/TMP95C063
			m_reg2 = regsel_dmam0;
			break;
		case 0x26:  // TMP96C141/TMP95C061/TMP95C063
			m_reg2 = regsel_dmam1;
			break;
		case 0x2A:  // TMP96C141/TMP95C061/TMP95C063
			m_reg2 = regsel_dmam2;
			break;
		case 0x2E:  // TMP96C141/TMP95C061/TMP95C063
			m_reg2 = regsel_dmam3;
			break;
		case 0x42:  // TMP94C241
			m_reg2 = regsel_dmam0;
			break;
		case 0x46:  // TMP94C241
			m_reg2 = regsel_dmam1;
			break;
		case 0x4A:  // TMP94C241
			m_reg2 = regsel_dmam2;
			break;
		case 0x4E:  // TMP94C241
			m_reg2 = regsel_dmam3;
			break;
		default:
			m_reg2 = regsel_dummy;
			break;
		}
		break;
	case p_CR16:
		m_imm1.d = RDOP();
		switch (m_imm1.d)
		{
		case 0x20:  // TMP96C141/TMP95C061/TMP95C063
			m_reg2 = regsel_dmac0;
			break;
		case 0x24:  // TMP96C141/TMP95C061/TMP95C063
			m_reg2 = regsel_dmac1;
			break;
		case 0x28:  // TMP96C141/TMP95C061/TMP95C063
			m_reg2 = regsel_dmac2;
			break;
		case 0x2C:  // TMP96C141/TMP95C061/TMP95C063
			m_reg2 = regsel_dmac3;
			break;
		case 0x40:  // TMP94C241
			m_reg2 = regsel_dmac0;
			break;
		case 0x44:  // TMP94C241
			m_reg2 = regsel_dmac1;
			break;
		case 0x48:  // TMP94C241
			m_reg2 = regsel_dmac2;
			break;
		case 0x4C:  // TMP94C241
			m_reg2 = regsel_dmac3;
			break;
		case 0x3C:  // TMP96C141/TMP95C061/TMP95C063 -- INTNEST
		case 0x7C:  // TMP94C241 -- INTNEST
			m_reg2 = regsel_intnest;
			break;
		default:
			m_reg2 = regsel_dummy;
			break;
		}
		break;
	case p_CR32:
		m_imm1.d = RDOP();
		switch (m_imm1.d)
		{
		case 0x00:  // all variants
			m_reg2 = regsel_dmas0;
			break;
		case 0x04:  // all variants
			m_reg2 = regsel_dmas1;
			break;
		case 0x08:  // all variants
			m_reg2 = regsel_dmas2;
			break;
		case 0x0C:  // all variants
			m_reg2 = regsel_dmas3;
			break;
		case 0x10:  // TMP96C141/TMP95C061/TMP95C063
			m_reg2 = regsel_dmad0;
			break;
		case 0x14:  // TMP96C141/TMP95C061/TMP95C063
			m_reg2 = regsel_dmad1;
			break;
		case 0x18:  // TMP96C141/TMP95C061/TMP95C063
			m_reg2 = regsel_dmad2;
			break;
		case 0x1C:  // TMP96C141/TMP95C061/TMP95C063
			m_reg2 = regsel_dmad3;
			break;
		case 0x20:  // TMP94C241
			m_reg2 = regsel_dmad0;
			break;
		case 0x24:  // TMP94C241
			m_reg2 = regsel_dmad1;
			break;
		case 0x28:  // TMP94C241
			m_reg2 = regsel_dmad2;
			break;
		case 0x2C:  // TMP94C241
			m_reg2 = regsel_dmad3;
			break;
		default:
			m_reg2 = regsel_dummy;
			break;
		}
		break;
	case p_D8:
		m_ea2.d = RDOP();
		m_ea2.d = m_pc.d + m_ea2.sb.l;
		break;
	case p_D16:
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_ea2.d = m_pc.d + m_ea2.sw.l;
		break;
	case p_I3:
		m_imm2.d = m_op & 0x07;
		break;
	case p_I8:
		m_imm2.d = RDOP();
		break;
	case p_I16:
		m_imm2.d = RDOP();
		m_imm2.b.h = RDOP();
		break;
	case p_I32:
		m_imm2.d = RDOP();
		m_imm2.b.h = RDOP();
		m_imm2.b.h2 = RDOP();
		m_imm2.b.h3 = RDOP();
		break;
	case p_M8:
		m_ea2.d = RDOP();
		break;
	case p_M16:
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		break;
	}
}


void tlcs900_device::execute_op(const tlcs900inst (&mnemonic)[256])
{
	m_op = RDOP();
	const tlcs900inst &inst = mnemonic[m_op];
	prepare_operands(inst);

	/* Execute the instruction */
	(this->*inst.opfunc)();
	m_cycles += inst.cycles;
}


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_80[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_PUSHBM, p_M, 0, 7 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_RLDRM, p_A, p_M, 12 }, { &tlcs900_device::op_RRDRM, p_A, p_M, 12 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_LDI, 0, 0, 10 }, { &tlcs900_device::op_LDIR, 0, 0, 10 }, { &tlcs900_device::op_LDD, 0, 0, 10 }, { &tlcs900_device::op_LDDR, 0, 0, 10 },
	{ &tlcs900_device::op_CPI, 0, 0, 8 }, { &tlcs900_device::op_CPIR, 0, 0, 10 }, { &tlcs900_device::op_CPD, 0, 0, 8 }, { &tlcs900_device::op_CPDR, 0, 0, 10 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDBMM, p_M16, p_M, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ADDBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_ADCBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_SUBBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_SBCBMI, p_M, p_I8, 7 },
	{ &tlcs900_device::op_ANDBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_XORBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_ORBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_CPBMI, p_M, p_I8, 6 },

	/* 40 - 5F */
	{ &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 },
	{ &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 },
	{ &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 },
	{ &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 },
	{ &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 },
	{ &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 },
	{ &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 },
	{ &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 },

	/* 60 - 7F */
	{ &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_RLCBM, p_M, 0, 6 }, { &tlcs900_device::op_RRCBM, p_M, 0, 6 }, { &tlcs900_device::op_RLBM, p_M, 0, 6 }, { &tlcs900_device::op_RRBM, p_M, 0, 6 },
	{ &tlcs900_device::op_SLABM, p_M, 0, 6 }, { &tlcs900_device::op_SRABM, p_M, 0, 6 }, { &tlcs900_device::op_SLLBM, p_M, 0, 6 }, { &tlcs900_device::op_SRLBM, p_M, 0, 6 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 },

	/* A0 - BF */
	{ &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 },

	/* C0 - DF */
	{ &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 },

	/* E0 - FF */
	{ &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 },
};


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_88[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_PUSHBM, p_M, 0, 7 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_RLDRM, p_A, p_M, 12 }, { &tlcs900_device::op_RRDRM, p_A, p_M, 12 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDBMM, p_M16, p_M, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ADDBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_ADCBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_SUBBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_SBCBMI, p_M, p_I8, 7 },
	{ &tlcs900_device::op_ANDBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_XORBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_ORBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_CPBMI, p_M, p_I8, 6 },

	/* 40 - 5F */
	{ &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 },
	{ &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 },
	{ &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 },
	{ &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 },
	{ &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 },
	{ &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 },
	{ &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 },
	{ &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 },

	/* 60 - 7F */
	{ &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_RLCBM, p_M, 0, 6 }, { &tlcs900_device::op_RRCBM, p_M, 0, 6 }, { &tlcs900_device::op_RLBM, p_M, 0, 6 }, { &tlcs900_device::op_RRBM, p_M, 0, 6 },
	{ &tlcs900_device::op_SLABM, p_M, 0, 6 }, { &tlcs900_device::op_SRABM, p_M, 0, 6 }, { &tlcs900_device::op_SLLBM, p_M, 0, 6 }, { &tlcs900_device::op_SRLBM, p_M, 0, 6 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 },

	/* A0 - BF */
	{ &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 },

	/* C0 - DF */
	{ &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 },

	/* E0 - FF */
	{ &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 },
};


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_90[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_PUSHWM, p_M, 0, 7 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_LDIW, 0, 0, 10 }, { &tlcs900_device::op_LDIRW, 0, 0, 10 }, { &tlcs900_device::op_LDDW, 0, 0, 10 }, { &tlcs900_device::op_LDDRW, 0, 0, 10 },
	{ &tlcs900_device::op_CPIW, 0, 0, 8 }, { &tlcs900_device::op_CPIRW, 0, 0, 10 }, { &tlcs900_device::op_CPDW, 0, 0, 8 }, { &tlcs900_device::op_CPDRW, 0, 0, 10 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDWMM, p_M16, p_M, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ADDWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_ADCWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_SUBWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_SBCWMI, p_M, p_I16, 8 },
	{ &tlcs900_device::op_ANDWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_XORWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_ORWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_CPWMI, p_M, p_I16, 6 },

	/* 40 - 5F */
	{ &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 },
	{ &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 },
	{ &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 },
	{ &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 },
	{ &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 },
	{ &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 },
	{ &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 },
	{ &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 },

	/* 60 - 7F */
	{ &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_RLCWM, p_M, 0, 6 }, { &tlcs900_device::op_RRCWM, p_M, 0, 6 }, { &tlcs900_device::op_RLWM, p_M, 0, 6 }, { &tlcs900_device::op_RRWM, p_M, 0, 6 },
	{ &tlcs900_device::op_SLAWM, p_M, 0, 6 }, { &tlcs900_device::op_SRAWM, p_M, 0, 6 }, { &tlcs900_device::op_SLLWM, p_M, 0, 6 }, { &tlcs900_device::op_SRLWM, p_M, 0, 6 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 },

	/* A0 - BF */
	{ &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 },

	/* C0 - DF */
	{ &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 },

	/* E0 - FF */
	{ &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 },
};


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_98[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_PUSHWM, p_M, 0, 7 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDWMM, p_M16, p_M, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ADDWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_ADCWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_SUBWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_SBCWMI, p_M, p_I16, 8 },
	{ &tlcs900_device::op_ANDWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_XORWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_ORWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_CPWMI, p_M, p_I16, 6 },

	/* 40 - 5F */
	{ &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 },
	{ &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 },
	{ &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 },
	{ &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 },
	{ &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 },
	{ &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 },
	{ &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 },
	{ &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 },

	/* 60 - 7F */
	{ &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_RLCWM, p_M, 0, 6 }, { &tlcs900_device::op_RRCWM, p_M, 0, 6 }, { &tlcs900_device::op_RLWM, p_M, 0, 6 }, { &tlcs900_device::op_RRWM, p_M, 0, 6 },
	{ &tlcs900_device::op_SLAWM, p_M, 0, 6 }, { &tlcs900_device::op_SRAWM, p_M, 0, 6 }, { &tlcs900_device::op_SLLWM, p_M, 0, 6 }, { &tlcs900_device::op_SRLWM, p_M, 0, 6 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 },

	/* A0 - BF */
	{ &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 },

	/* C0 - DF */
	{ &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 },

	/* E0 - FF */
	{ &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 },
};


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_a0[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_LDLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_LDLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 },

	/* A0 - BF */
	{ &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 },

	/* C0 - DF */
	{ &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_XORLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_XORLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_XORLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_XORLMR, p_M, p_C32, 10 },

	/* E0 - FF */
	{ &tlcs900_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ORLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ORLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ORLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ORLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_CPLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_CPLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_CPLMR, p_M, p_C32, 6 },
	{ &tlcs900_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_CPLMR, p_M, p_C32, 6 },
};


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_b0[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_LDBMI, p_M, p_I8, 5 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDWMI, p_M, p_I16, 6 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_POPBM, p_M, 0, 6 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_POPWM, p_M, 0, 6 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_LDBMM, p_M, p_M16, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDWMM, p_M, p_M16, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ANDCFBRM, p_A, p_M, 8 }, { &tlcs900_device::op_ORCFBRM, p_A, p_M, 8 }, { &tlcs900_device::op_XORCFBRM, p_A, p_M, 8 }, { &tlcs900_device::op_LDCFBRM, p_A, p_M, 8 },
	{ &tlcs900_device::op_STCFBRM, p_A, p_M, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 },

	/* A0 - BF */
	{ &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 },
	{ &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 },
	{ &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 },

	/* C0 - DF */
	{ &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 },

	/* E0 - FF */
	{ &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900_device::op_RETCC, p_CC, 0, 6 },
	{ &tlcs900_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900_device::op_RETCC, p_CC, 0, 6 },
	{ &tlcs900_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900_device::op_RETCC, p_CC, 0, 6 },
	{ &tlcs900_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900_device::op_RETCC, p_CC, 0, 6 }
};


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_b8[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_LDBMI, p_M, p_I8, 5 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDWMI, p_M, p_I16, 6 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_POPBM, p_M, 0, 6 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_POPWM, p_M, 0, 6 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_LDBMM, p_M, p_M16, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDWMM, p_M, p_M16, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ANDCFBRM, p_A, p_M, 8 }, { &tlcs900_device::op_ORCFBRM, p_A, p_M, 8 }, { &tlcs900_device::op_XORCFBRM, p_A, p_M, 8 }, { &tlcs900_device::op_LDCFBRM, p_A, p_M, 8 },
	{ &tlcs900_device::op_STCFBRM, p_A, p_M, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 },

	/* A0 - BF */
	{ &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 },
	{ &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 },
	{ &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 },

	/* C0 - DF */
	{ &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 },

	/* E0 - FF */
	{ &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }
};


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_c0[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_PUSHBM, p_M, 0, 7 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_RLDRM, p_A, p_M, 12 }, { &tlcs900_device::op_RRDRM, p_A, p_M, 12 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDBMM, p_M16, p_M, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ADDBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_ADCBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_SUBBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_SBCBMI, p_M, p_I8, 7 },
	{ &tlcs900_device::op_ANDBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_XORBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_ORBMI, p_M, p_I8, 7 }, { &tlcs900_device::op_CPBMI, p_M, p_I8, 6 },

	/* 40 - 5F */
	{ &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 },
	{ &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULBRM, p_MC16, p_M, 18 },
	{ &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 },
	{ &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900_device::op_MULSBRM, p_MC16, p_M, 18 },
	{ &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 },
	{ &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900_device::op_DIVBRM, p_MC16, p_M, 22 },
	{ &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 },
	{ &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900_device::op_DIVSBRM, p_MC16, p_M, 24 },

	/* 60 - 7F */
	{ &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_RLCBM, p_M, 0, 6 }, { &tlcs900_device::op_RRCBM, p_M, 0, 6 }, { &tlcs900_device::op_RLBM, p_M, 0, 6 }, { &tlcs900_device::op_RRBM, p_M, 0, 6 },
	{ &tlcs900_device::op_SLABM, p_M, 0, 6 }, { &tlcs900_device::op_SRABM, p_M, 0, 6 }, { &tlcs900_device::op_SLLBM, p_M, 0, 6 }, { &tlcs900_device::op_SRLBM, p_M, 0, 6 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ADCBMR, p_M, p_C8, 6 },

	/* A0 - BF */
	{ &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_SBCBMR, p_M, p_C8, 6 },

	/* C0 - DF */
	{ &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_XORBMR, p_M, p_C8, 6 },

	/* E0 - FF */
	{ &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 },
	{ &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900_device::op_CPBMR, p_M, p_C8, 6 },
};


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_c8[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDBRI, p_R, p_I8, 4 },
	{ &tlcs900_device::op_PUSHBR, p_R, 0, 6 }, { &tlcs900_device::op_POPBR, p_R, 0, 6 }, { &tlcs900_device::op_CPLBR, p_R, 0, 4 }, { &tlcs900_device::op_NEGBR, p_R, 0, 5 },
	{ &tlcs900_device::op_MULBRI, p_R, p_I8, 18 }, { &tlcs900_device::op_MULSBRI, p_R, p_I8, 18 }, { &tlcs900_device::op_DIVBRI, p_R, p_I8, 22 }, { &tlcs900_device::op_DIVSBRI, p_R, p_I8, 24 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DAABR, p_R, 0, 6 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DJNZB, p_R, p_D8, 7 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_ANDCFBIR, p_I8, p_R, 4 }, { &tlcs900_device::op_ORCFBIR, p_I8, p_R, 4 }, { &tlcs900_device::op_XORCFBIR, p_I8, p_R, 4 }, { &tlcs900_device::op_LDCFBIR, p_I8, p_R, 4 },
	{ &tlcs900_device::op_STCFBIR, p_I8, p_R, 4 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_ANDCFBRR, p_A, p_R, 4 }, { &tlcs900_device::op_ORCFBRR, p_A, p_R, 4 }, { &tlcs900_device::op_XORCFBRR, p_A, p_R, 4 }, { &tlcs900_device::op_LDCFBRR, p_A, p_R, 4 },
	{ &tlcs900_device::op_STCFBRR, p_A, p_R, 4 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDCBRR, p_CR8, p_R, 8 }, { &tlcs900_device::op_LDCBRR, p_R, p_CR8, 8 },
	{ &tlcs900_device::op_RESBIR, p_I8, p_R, 4 }, { &tlcs900_device::op_SETBIR, p_I8, p_R, 4 }, { &tlcs900_device::op_CHGBIR, p_I8, p_R, 4 }, { &tlcs900_device::op_BITBIR, p_I8, p_R, 4 },
	{ &tlcs900_device::op_TSETBIR, p_I8, p_R, 6 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900_device::op_MULBRR, p_MC16, p_R, 18 }, { &tlcs900_device::op_MULBRR, p_MC16, p_R, 18 }, { &tlcs900_device::op_MULBRR, p_MC16, p_R, 18 }, { &tlcs900_device::op_MULBRR, p_MC16, p_R, 18 },
	{ &tlcs900_device::op_MULBRR, p_MC16, p_R, 18 }, { &tlcs900_device::op_MULBRR, p_MC16, p_R, 18 }, { &tlcs900_device::op_MULBRR, p_MC16, p_R, 18 }, { &tlcs900_device::op_MULBRR, p_MC16, p_R, 18 },
	{ &tlcs900_device::op_MULSBRR, p_MC16, p_R, 18 }, { &tlcs900_device::op_MULSBRR, p_MC16, p_R, 18 }, { &tlcs900_device::op_MULSBRR, p_MC16, p_R, 18 }, { &tlcs900_device::op_MULSBRR, p_MC16, p_R, 18 },
	{ &tlcs900_device::op_MULSBRR, p_MC16, p_R, 18 }, { &tlcs900_device::op_MULSBRR, p_MC16, p_R, 18 }, { &tlcs900_device::op_MULSBRR, p_MC16, p_R, 18 }, { &tlcs900_device::op_MULSBRR, p_MC16, p_R, 18 },
	{ &tlcs900_device::op_DIVBRR, p_MC16, p_R, 22 }, { &tlcs900_device::op_DIVBRR, p_MC16, p_R, 22 }, { &tlcs900_device::op_DIVBRR, p_MC16, p_R, 22 }, { &tlcs900_device::op_DIVBRR, p_MC16, p_R, 22 },
	{ &tlcs900_device::op_DIVBRR, p_MC16, p_R, 22 }, { &tlcs900_device::op_DIVBRR, p_MC16, p_R, 22 }, { &tlcs900_device::op_DIVBRR, p_MC16, p_R, 22 }, { &tlcs900_device::op_DIVBRR, p_MC16, p_R, 22 },
	{ &tlcs900_device::op_DIVSBRR, p_MC16, p_R, 24 }, { &tlcs900_device::op_DIVSBRR, p_MC16, p_R, 24 }, { &tlcs900_device::op_DIVSBRR, p_MC16, p_R, 24 }, { &tlcs900_device::op_DIVSBRR, p_MC16, p_R, 24 },
	{ &tlcs900_device::op_DIVSBRR, p_MC16, p_R, 24 }, { &tlcs900_device::op_DIVSBRR, p_MC16, p_R, 24 }, { &tlcs900_device::op_DIVSBRR, p_MC16, p_R, 24 }, { &tlcs900_device::op_DIVSBRR, p_MC16, p_R, 24 },

	/* 60 - 7F */
	{ &tlcs900_device::op_INCBIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCBIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCBIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCBIR, p_I3, p_R, 4 },
	{ &tlcs900_device::op_INCBIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCBIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCBIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCBIR, p_I3, p_R, 4 },
	{ &tlcs900_device::op_DECBIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECBIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECBIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECBIR, p_I3, p_R, 4 },
	{ &tlcs900_device::op_DECBIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECBIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECBIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECBIR, p_I3, p_R, 4 },
	{ &tlcs900_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCBR, p_CC, p_R, 6 },
	{ &tlcs900_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCBR, p_CC, p_R, 6 },
	{ &tlcs900_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCBR, p_CC, p_R, 6 },
	{ &tlcs900_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCBR, p_CC, p_R, 6 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ADDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ADDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ADDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ADDBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_ADDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ADDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ADDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ADDBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_LDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_LDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_LDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_LDBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_LDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_LDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_LDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_LDBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_ADCBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ADCBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ADCBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ADCBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_ADCBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ADCBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ADCBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ADCBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_LDBRR, p_R, p_C8, 4 }, { &tlcs900_device::op_LDBRR, p_R, p_C8, 4 }, { &tlcs900_device::op_LDBRR, p_R, p_C8, 4 }, { &tlcs900_device::op_LDBRR, p_R, p_C8, 4 },
	{ &tlcs900_device::op_LDBRR, p_R, p_C8, 4 }, { &tlcs900_device::op_LDBRR, p_R, p_C8, 4 }, { &tlcs900_device::op_LDBRR, p_R, p_C8, 4 }, { &tlcs900_device::op_LDBRR, p_R, p_C8, 4 },

	/* A0 - BF */
	{ &tlcs900_device::op_SUBBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_SUBBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_SUBBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_SUBBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_SUBBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_SUBBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_SUBBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_SUBBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_LDBRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDBRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDBRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDBRI, p_R, p_I3, 4 },
	{ &tlcs900_device::op_LDBRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDBRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDBRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDBRI, p_R, p_I3, 4 },
	{ &tlcs900_device::op_SBCBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_SBCBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_SBCBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_SBCBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_SBCBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_SBCBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_SBCBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_SBCBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_EXBRR, p_C8, p_R, 5 }, { &tlcs900_device::op_EXBRR, p_C8, p_R, 5 }, { &tlcs900_device::op_EXBRR, p_C8, p_R, 5 }, { &tlcs900_device::op_EXBRR, p_C8, p_R, 5 },
	{ &tlcs900_device::op_EXBRR, p_C8, p_R, 5 }, { &tlcs900_device::op_EXBRR, p_C8, p_R, 5 }, { &tlcs900_device::op_EXBRR, p_C8, p_R, 5 }, { &tlcs900_device::op_EXBRR, p_C8, p_R, 5 },

	/* C0 - DF */
	{ &tlcs900_device::op_ANDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ANDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ANDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ANDBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_ANDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ANDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ANDBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ANDBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_ADDBRI, p_R, p_I8, 4 }, { &tlcs900_device::op_ADCBRI, p_R, p_I8, 4 }, { &tlcs900_device::op_SUBBRI, p_R, p_I8, 4 }, { &tlcs900_device::op_SBCBRI, p_R, p_I8, 4 },
	{ &tlcs900_device::op_ANDBRI, p_R, p_I8, 4 }, { &tlcs900_device::op_XORBRI, p_R, p_I8, 4 }, { &tlcs900_device::op_ORBRI, p_R, p_I8, 4 }, { &tlcs900_device::op_CPBRI, p_R, p_I8, 4 },
	{ &tlcs900_device::op_XORBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_XORBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_XORBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_XORBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_XORBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_XORBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_XORBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_XORBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_CPBRI, p_R, p_I3, 4 }, { &tlcs900_device::op_CPBRI, p_R, p_I3, 4 }, { &tlcs900_device::op_CPBRI, p_R, p_I3, 4 }, { &tlcs900_device::op_CPBRI, p_R, p_I3, 4 },
	{ &tlcs900_device::op_CPBRI, p_R, p_I3, 4 }, { &tlcs900_device::op_CPBRI, p_R, p_I3, 4 }, { &tlcs900_device::op_CPBRI, p_R, p_I3, 4 }, { &tlcs900_device::op_CPBRI, p_R, p_I3, 4 },

	/* E0 - FF */
	{ &tlcs900_device::op_ORBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ORBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ORBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ORBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_ORBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ORBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ORBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_ORBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_RLCBIR, p_I8, p_R, 6 }, { &tlcs900_device::op_RRCBIR, p_I8, p_R, 6 }, { &tlcs900_device::op_RLBIR, p_I8, p_R, 6 }, { &tlcs900_device::op_RRBIR, p_I8, p_R, 6 },
	{ &tlcs900_device::op_SLABIR, p_I8, p_R, 6 }, { &tlcs900_device::op_SRABIR, p_I8, p_R, 6 }, { &tlcs900_device::op_SLLBIR, p_I8, p_R, 6 }, { &tlcs900_device::op_SRLBIR, p_I8, p_R, 6 },
	{ &tlcs900_device::op_CPBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_CPBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_CPBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_CPBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_CPBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_CPBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_CPBRR, p_C8, p_R, 4 }, { &tlcs900_device::op_CPBRR, p_C8, p_R, 4 },
	{ &tlcs900_device::op_RLCBRR, p_A, p_R, 6 }, { &tlcs900_device::op_RRCBRR, p_A, p_R, 6 }, { &tlcs900_device::op_RLBRR, p_A, p_R, 6 }, { &tlcs900_device::op_RRBRR, p_A, p_R, 6 },
	{ &tlcs900_device::op_SLABRR, p_A, p_R, 6 }, { &tlcs900_device::op_SRABRR, p_A, p_R, 6 }, { &tlcs900_device::op_SLLBRR, p_A, p_R, 6 }, { &tlcs900_device::op_SRLBRR, p_A, p_R, 6 }
};


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_d0[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_PUSHWM, p_M, 0, 7 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDWMM, p_M16, p_M, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ADDWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_ADCWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_SUBWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_SBCWMI, p_M, p_I16, 8 },
	{ &tlcs900_device::op_ANDWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_XORWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_ORWMI, p_M, p_I16, 8 }, { &tlcs900_device::op_CPWMI, p_M, p_I16, 6 },

	/* 40 - 5F */
	{ &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 },
	{ &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULWRM, p_C32, p_M, 26 },
	{ &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 },
	{ &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900_device::op_MULSWRM, p_C32, p_M, 26 },
	{ &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 },
	{ &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900_device::op_DIVWRM, p_C32, p_M, 30 },
	{ &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 },
	{ &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900_device::op_DIVSWRM, p_C32, p_M, 32 },

	/* 60 - 7F */
	{ &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_RLCWM, p_M, 0, 6 }, { &tlcs900_device::op_RRCWM, p_M, 0, 6 }, { &tlcs900_device::op_RLWM, p_M, 0, 6 }, { &tlcs900_device::op_RRWM, p_M, 0, 6 },
	{ &tlcs900_device::op_SLAWM, p_M, 0, 6 }, { &tlcs900_device::op_SRAWM, p_M, 0, 6 }, { &tlcs900_device::op_SLLWM, p_M, 0, 6 }, { &tlcs900_device::op_SRLWM, p_M, 0, 6 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ADCWMR, p_M, p_C16, 6 },

	/* A0 - BF */
	{ &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_SBCWMR, p_M, p_C16, 6 },

	/* C0 - DF */
	{ &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_XORWMR, p_M, p_C16, 6 },

	/* E0 - FF */
	{ &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 },
	{ &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900_device::op_CPWMR, p_M, p_C16, 6 },
};


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_d8[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDWRI, p_R, p_I16, 4 },
	{ &tlcs900_device::op_PUSHWR, p_R, 0, 5 }, { &tlcs900_device::op_POPWR, p_R, 0, 6 }, { &tlcs900_device::op_CPLWR, p_R, 0, 4 }, { &tlcs900_device::op_NEGWR, p_R, 0, 5 },
	{ &tlcs900_device::op_MULWRI, p_R, p_I16, 26 }, { &tlcs900_device::op_MULSWRI, p_R, p_I16, 26 }, { &tlcs900_device::op_DIVWRI, p_R, p_I16, 30 }, { &tlcs900_device::op_DIVSWRI, p_R, p_I16, 32 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_BS1FRR, p_A, p_R, 4 }, { &tlcs900_device::op_BS1BRR, p_A, p_R, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_EXTZWR, p_R, 0, 4 }, { &tlcs900_device::op_EXTSWR, p_R, 0, 5 },
	{ &tlcs900_device::op_PAAWR, p_R, 0, 4 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_MIRRW, p_R, 0, 4 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_MULAR, p_R, 0, 31 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DJNZW, p_R, p_D8, 7 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_ANDCFWIR, p_I8, p_R, 4 }, { &tlcs900_device::op_ORCFWIR, p_I8, p_R, 4 }, { &tlcs900_device::op_XORCFWIR, p_I8, p_R, 4 }, { &tlcs900_device::op_LDCFWIR, p_I8, p_R, 4 },
	{ &tlcs900_device::op_STCFWIR, p_I8, p_R, 4 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_ANDCFWRR, p_A, p_R, 4 }, { &tlcs900_device::op_ORCFWRR, p_A, p_R, 4 }, { &tlcs900_device::op_XORCFWRR, p_A, p_R, 4 }, { &tlcs900_device::op_LDCFWRR, p_A, p_R, 4 },
	{ &tlcs900_device::op_STCFWRR, p_A, p_R, 4 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDCWRR, p_CR16, p_R, 8 }, { &tlcs900_device::op_LDCWRR, p_R, p_CR16, 8 },
	{ &tlcs900_device::op_RESWIR, p_I8, p_R, 4 }, { &tlcs900_device::op_SETWIR, p_I8, p_R, 4 }, { &tlcs900_device::op_CHGWIR, p_I8, p_R, 4 }, { &tlcs900_device::op_BITWIR, p_I8, p_R, 4 },
	{ &tlcs900_device::op_TSETWIR, p_I8, p_R, 6 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_MINC1, p_I16, p_R, 8 }, { &tlcs900_device::op_MINC2, p_I16, p_R, 8 }, { &tlcs900_device::op_MINC4, p_I16, p_R, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_MDEC1, p_I16, p_R, 7 }, { &tlcs900_device::op_MDEC2, p_I16, p_R, 7 }, { &tlcs900_device::op_MDEC4, p_I16, p_R, 7 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900_device::op_MULWRR, p_C32, p_R, 26 }, { &tlcs900_device::op_MULWRR, p_C32, p_R, 26 }, { &tlcs900_device::op_MULWRR, p_C32, p_R, 26 }, { &tlcs900_device::op_MULWRR, p_C32, p_R, 26 },
	{ &tlcs900_device::op_MULWRR, p_C32, p_R, 26 }, { &tlcs900_device::op_MULWRR, p_C32, p_R, 26 }, { &tlcs900_device::op_MULWRR, p_C32, p_R, 26 }, { &tlcs900_device::op_MULWRR, p_C32, p_R, 26 },
	{ &tlcs900_device::op_MULSWRR, p_C32, p_R, 26 }, { &tlcs900_device::op_MULSWRR, p_C32, p_R, 26 }, { &tlcs900_device::op_MULSWRR, p_C32, p_R, 26 }, { &tlcs900_device::op_MULSWRR, p_C32, p_R, 26 },
	{ &tlcs900_device::op_MULSWRR, p_C32, p_R, 26 }, { &tlcs900_device::op_MULSWRR, p_C32, p_R, 26 }, { &tlcs900_device::op_MULSWRR, p_C32, p_R, 26 }, { &tlcs900_device::op_MULSWRR, p_C32, p_R, 26 },
	{ &tlcs900_device::op_DIVWRR, p_C32, p_R, 30 }, { &tlcs900_device::op_DIVWRR, p_C32, p_R, 30 }, { &tlcs900_device::op_DIVWRR, p_C32, p_R, 30 }, { &tlcs900_device::op_DIVWRR, p_C32, p_R, 30 },
	{ &tlcs900_device::op_DIVWRR, p_C32, p_R, 30 }, { &tlcs900_device::op_DIVWRR, p_C32, p_R, 30 }, { &tlcs900_device::op_DIVWRR, p_C32, p_R, 30 }, { &tlcs900_device::op_DIVWRR, p_C32, p_R, 30 },
	{ &tlcs900_device::op_DIVSWRR, p_C32, p_R, 32 }, { &tlcs900_device::op_DIVSWRR, p_C32, p_R, 32 }, { &tlcs900_device::op_DIVSWRR, p_C32, p_R, 32 }, { &tlcs900_device::op_DIVSWRR, p_C32, p_R, 32 },
	{ &tlcs900_device::op_DIVSWRR, p_C32, p_R, 32 }, { &tlcs900_device::op_DIVSWRR, p_C32, p_R, 32 }, { &tlcs900_device::op_DIVSWRR, p_C32, p_R, 32 }, { &tlcs900_device::op_DIVSWRR, p_C32, p_R, 32 },

	/* 60 - 7F */
	{ &tlcs900_device::op_INCWIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCWIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCWIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCWIR, p_I3, p_R, 4 },
	{ &tlcs900_device::op_INCWIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCWIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCWIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCWIR, p_I3, p_R, 4 },
	{ &tlcs900_device::op_DECWIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECWIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECWIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECWIR, p_I3, p_R, 4 },
	{ &tlcs900_device::op_DECWIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECWIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECWIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECWIR, p_I3, p_R, 4 },
	{ &tlcs900_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCWR, p_CC, p_R, 6 },
	{ &tlcs900_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCWR, p_CC, p_R, 6 },
	{ &tlcs900_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCWR, p_CC, p_R, 6 },
	{ &tlcs900_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900_device::op_SCCWR, p_CC, p_R, 6 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ADDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ADDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ADDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ADDWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_ADDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ADDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ADDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ADDWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_LDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_LDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_LDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_LDWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_LDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_LDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_LDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_LDWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_ADCWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ADCWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ADCWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ADCWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_ADCWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ADCWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ADCWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ADCWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_LDWRR, p_R, p_C16, 4 }, { &tlcs900_device::op_LDWRR, p_R, p_C16, 4 }, { &tlcs900_device::op_LDWRR, p_R, p_C16, 4 }, { &tlcs900_device::op_LDWRR, p_R, p_C16, 4 },
	{ &tlcs900_device::op_LDWRR, p_R, p_C16, 4 }, { &tlcs900_device::op_LDWRR, p_R, p_C16, 4 }, { &tlcs900_device::op_LDWRR, p_R, p_C16, 4 }, { &tlcs900_device::op_LDWRR, p_R, p_C16, 4 },

	/* A0 - BF */
	{ &tlcs900_device::op_SUBWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_SUBWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_SUBWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_SUBWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_SUBWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_SUBWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_SUBWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_SUBWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_LDWRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDWRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDWRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDWRI, p_R, p_I3, 4 },
	{ &tlcs900_device::op_LDWRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDWRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDWRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDWRI, p_R, p_I3, 4 },
	{ &tlcs900_device::op_SBCWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_SBCWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_SBCWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_SBCWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_SBCWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_SBCWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_SBCWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_SBCWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_EXWRR, p_C16, p_R, 5 }, { &tlcs900_device::op_EXWRR, p_C16, p_R, 5 }, { &tlcs900_device::op_EXWRR, p_C16, p_R, 5 }, { &tlcs900_device::op_EXWRR, p_C16, p_R, 5 },
	{ &tlcs900_device::op_EXWRR, p_C16, p_R, 5 }, { &tlcs900_device::op_EXWRR, p_C16, p_R, 5 }, { &tlcs900_device::op_EXWRR, p_C16, p_R, 5 }, { &tlcs900_device::op_EXWRR, p_C16, p_R, 5 },

	/* C0 - DF */
	{ &tlcs900_device::op_ANDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ANDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ANDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ANDWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_ANDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ANDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ANDWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ANDWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_ADDWRI, p_R, p_I16, 4 }, { &tlcs900_device::op_ADCWRI, p_R, p_I16, 4 }, { &tlcs900_device::op_SUBWRI, p_R, p_I16, 4 }, { &tlcs900_device::op_SBCWRI, p_R, p_I16, 4 },
	{ &tlcs900_device::op_ANDWRI, p_R, p_I16, 4 }, { &tlcs900_device::op_XORWRI, p_R, p_I16, 4 }, { &tlcs900_device::op_ORWRI, p_R, p_I16, 4 }, { &tlcs900_device::op_CPWRI, p_R, p_I16, 4 },
	{ &tlcs900_device::op_XORWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_XORWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_XORWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_XORWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_XORWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_XORWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_XORWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_XORWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_CPWRI, p_R, p_I3, 4 }, { &tlcs900_device::op_CPWRI, p_R, p_I3, 4 }, { &tlcs900_device::op_CPWRI, p_R, p_I3, 4 }, { &tlcs900_device::op_CPWRI, p_R, p_I3, 4 },
	{ &tlcs900_device::op_CPWRI, p_R, p_I3, 4 }, { &tlcs900_device::op_CPWRI, p_R, p_I3, 4 }, { &tlcs900_device::op_CPWRI, p_R, p_I3, 4 }, { &tlcs900_device::op_CPWRI, p_R, p_I3, 4 },

	/* E0 - FF */
	{ &tlcs900_device::op_ORWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ORWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ORWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ORWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_ORWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ORWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ORWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_ORWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_RLCWIR, p_I8, p_R, 6 }, { &tlcs900_device::op_RRCWIR, p_I8, p_R, 6 }, { &tlcs900_device::op_RLWIR, p_I8, p_R, 6 }, { &tlcs900_device::op_RRWIR, p_I8, p_R, 6 },
	{ &tlcs900_device::op_SLAWIR, p_I8, p_R, 6 }, { &tlcs900_device::op_SRAWIR, p_I8, p_R, 6 }, { &tlcs900_device::op_SLLWIR, p_I8, p_R, 6 }, { &tlcs900_device::op_SRLWIR, p_I8, p_R, 6 },
	{ &tlcs900_device::op_CPWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_CPWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_CPWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_CPWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_CPWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_CPWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_CPWRR, p_C16, p_R, 4 }, { &tlcs900_device::op_CPWRR, p_C16, p_R, 4 },
	{ &tlcs900_device::op_RLCWRR, p_A, p_R, 6 }, { &tlcs900_device::op_RRCWRR, p_A, p_R, 6 }, { &tlcs900_device::op_RLWRR, p_A, p_R, 6 }, { &tlcs900_device::op_RRWRR, p_A, p_R, 6 },
	{ &tlcs900_device::op_SLAWRR, p_A, p_R, 6 }, { &tlcs900_device::op_SRAWRR, p_A, p_R, 6 }, { &tlcs900_device::op_SLLWRR, p_A, p_R, 6 }, { &tlcs900_device::op_SRLWRR, p_A, p_R, 6 }
};


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_e0[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_LDLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_LDLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADDLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADDLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ADCLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ADCLMR, p_M, p_C32, 10 },

	/* A0 - BF */
	{ &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SUBLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SUBLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_SBCLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_SBCLMR, p_M, p_C32, 10 },

	/* C0 - DF */
	{ &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ANDLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ANDLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_XORLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_XORLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_XORLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_XORLMR, p_M, p_C32, 10 },

	/* E0 - FF */
	{ &tlcs900_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ORLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_ORLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ORLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900_device::op_ORLMR, p_M, p_C32, 10 },
	{ &tlcs900_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_CPLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900_device::op_CPLRM, p_C32, p_M, 6 },
	{ &tlcs900_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_CPLMR, p_M, p_C32, 6 },
	{ &tlcs900_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_CPLMR, p_M, p_C32, 6 },
};


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_e8[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDLRI, p_R, p_I32, 6 },
	{ &tlcs900_device::op_PUSHLR, p_R, 0, 7 }, { &tlcs900_device::op_POPLR, p_R, 0, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_LINK, p_R, p_I16, 10 }, { &tlcs900_device::op_UNLK, p_R, 0, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_EXTZLR, p_R, 0, 4 }, { &tlcs900_device::op_EXTSLR, p_R, 0, 5 },
	{ &tlcs900_device::op_PAALR, p_R, 0, 4 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDCLRR, p_CR32, p_R, 8 }, { &tlcs900_device::op_LDCLRR, p_R, p_CR32, 8 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900_device::op_INCLIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCLIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCLIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCLIR, p_I3, p_R, 4 },
	{ &tlcs900_device::op_INCLIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCLIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCLIR, p_I3, p_R, 4 }, { &tlcs900_device::op_INCLIR, p_I3, p_R, 4 },
	{ &tlcs900_device::op_DECLIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECLIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECLIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECLIR, p_I3, p_R, 4 },
	{ &tlcs900_device::op_DECLIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECLIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECLIR, p_I3, p_R, 4 }, { &tlcs900_device::op_DECLIR, p_I3, p_R, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ADDLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ADDLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ADDLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ADDLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_ADDLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ADDLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ADDLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ADDLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_LDLRR, p_C32, p_R, 4 }, { &tlcs900_device::op_LDLRR, p_C32, p_R, 4 }, { &tlcs900_device::op_LDLRR, p_C32, p_R, 4 }, { &tlcs900_device::op_LDLRR, p_C32, p_R, 4 },
	{ &tlcs900_device::op_LDLRR, p_C32, p_R, 4 }, { &tlcs900_device::op_LDLRR, p_C32, p_R, 4 }, { &tlcs900_device::op_LDLRR, p_C32, p_R, 4 }, { &tlcs900_device::op_LDLRR, p_C32, p_R, 4 },
	{ &tlcs900_device::op_ADCLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ADCLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ADCLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ADCLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_ADCLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ADCLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ADCLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ADCLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_LDLRR, p_R, p_C32, 4 }, { &tlcs900_device::op_LDLRR, p_R, p_C32, 4 }, { &tlcs900_device::op_LDLRR, p_R, p_C32, 4 }, { &tlcs900_device::op_LDLRR, p_R, p_C32, 4 },
	{ &tlcs900_device::op_LDLRR, p_R, p_C32, 4 }, { &tlcs900_device::op_LDLRR, p_R, p_C32, 4 }, { &tlcs900_device::op_LDLRR, p_R, p_C32, 4 }, { &tlcs900_device::op_LDLRR, p_R, p_C32, 4 },

	/* A0 - BF */
	{ &tlcs900_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_SUBLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_SUBLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDLRI, p_R, p_I3, 4 },
	{ &tlcs900_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900_device::op_LDLRI, p_R, p_I3, 4 },
	{ &tlcs900_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_SBCLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_SBCLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* C0 - DF */
	{ &tlcs900_device::op_ANDLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ANDLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ANDLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ANDLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_ANDLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ANDLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ANDLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ANDLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_ADDLRI, p_R, p_I32, 7 }, { &tlcs900_device::op_ADCLRI, p_R, p_I32, 7 }, { &tlcs900_device::op_SUBLRI, p_R, p_I32, 7 }, { &tlcs900_device::op_SBCLRI, p_R, p_I32, 7 },
	{ &tlcs900_device::op_ANDLRI, p_R, p_I32, 7 }, { &tlcs900_device::op_XORLRI, p_R, p_I32, 7 }, { &tlcs900_device::op_ORLRI, p_R, p_I32, 7 }, { &tlcs900_device::op_CPLRI, p_R, p_I32, 7 },
	{ &tlcs900_device::op_XORLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_XORLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_XORLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_XORLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_XORLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_XORLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_XORLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_XORLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_CPLRI, p_R, p_I3, 6 }, { &tlcs900_device::op_CPLRI, p_R, p_I3, 6 }, { &tlcs900_device::op_CPLRI, p_R, p_I3, 6 }, { &tlcs900_device::op_CPLRI, p_R, p_I3, 6 },
	{ &tlcs900_device::op_CPLRI, p_R, p_I3, 6 }, { &tlcs900_device::op_CPLRI, p_R, p_I3, 6 }, { &tlcs900_device::op_CPLRI, p_R, p_I3, 6 }, { &tlcs900_device::op_CPLRI, p_R, p_I3, 6 },

	/* E0 - FF */
	{ &tlcs900_device::op_ORLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ORLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ORLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ORLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_ORLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ORLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ORLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_ORLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_RLCLIR, p_I8, p_R, 8 }, { &tlcs900_device::op_RRCLIR, p_I8, p_R, 8 }, { &tlcs900_device::op_RLLIR, p_I8, p_R, 8 }, { &tlcs900_device::op_RRLIR, p_I8, p_R, 8 },
	{ &tlcs900_device::op_SLALIR, p_I8, p_R, 8 }, { &tlcs900_device::op_SRALIR, p_I8, p_R, 8 }, { &tlcs900_device::op_SLLLIR, p_I8, p_R, 8 }, { &tlcs900_device::op_SRLLIR, p_I8, p_R, 8 },
	{ &tlcs900_device::op_CPLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_CPLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_CPLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_CPLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_CPLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_CPLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_CPLRR, p_C32, p_R, 7 }, { &tlcs900_device::op_CPLRR, p_C32, p_R, 7 },
	{ &tlcs900_device::op_RLCLRR, p_A, p_R, 8 }, { &tlcs900_device::op_RRCLRR, p_A, p_R, 8 }, { &tlcs900_device::op_RLLRR, p_A, p_R, 8 }, { &tlcs900_device::op_RRLRR, p_A, p_R, 8 },
	{ &tlcs900_device::op_SLALRR, p_A, p_R, 8 }, { &tlcs900_device::op_SRALRR, p_A, p_R, 8 }, { &tlcs900_device::op_SLLLRR, p_A, p_R, 8 }, { &tlcs900_device::op_SRLLRR, p_A, p_R, 8 }
};


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic_f0[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_LDBMI, p_M, p_I8, 5 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDWMI, p_M, p_I16, 6 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_POPBM, p_M, 0, 6 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_POPWM, p_M, 0, 6 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_LDBMM, p_M, p_M16, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_LDWMM, p_M, p_M16, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900_device::op_ANDCFBRM, p_A, p_M, 8 }, { &tlcs900_device::op_ORCFBRM, p_A, p_M, 8 }, { &tlcs900_device::op_XORCFBRM, p_A, p_M, 8 }, { &tlcs900_device::op_LDCFBRM, p_A, p_M, 8 },
	{ &tlcs900_device::op_STCFBRM, p_A, p_M, 8 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ANDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_ORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_XORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_LDCFBIM, p_I3, p_M, 8 },

	/* A0 - BF */
	{ &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_STCFBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 },
	{ &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900_device::op_TSETBIM, p_I3, p_M, 10 },
	{ &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_RESBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_SETBIM, p_I3, p_M, 8 },

	/* C0 - DF */
	{ &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_CHGBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900_device::op_BITBIM, p_I3, p_M, 8 },
	{ &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900_device::op_JPM, p_CC, p_M, 4 },

	/* E0 - FF */
	{ &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }
};


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP) used as source in byte operations */
void tlcs900_device::op_80()
{
	/* For CPI/CPIR/CPD/CPDR/LDI/LDD/LDIR/LDDR operations */
	m_reg1 = get_reg32_current_sel(m_op - 1);
	m_reg2 = get_reg32_current_sel(m_op);

	m_ea2.d = get_reg32_current(m_op);
	execute_op(*m_mnemonic_80);
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP + d8) used as source in byte operations */
void tlcs900_device::op_88()
{
	/* For CPI/CPIR/CPD/CPDR/LDI/LDD/LDIR/LDDR operations */
	m_reg1 = get_reg32_current_sel(m_op - 1);
	m_reg2 = get_reg32_current_sel(m_op);

	m_ea2.d = get_reg32_current(m_op);
	m_op = RDOP();
	m_ea2.d += int8_t(m_op);
	m_cycles += tlcs900_mem_index_cycles();
	execute_op(*m_mnemonic_80);
}


/* (XWA/XBC/XDE/XHL/XIXI/XIY/XIZ/XSP) used as source in word operations */
void tlcs900_device::op_90()
{
	/* For CPI/CPIR/CPD/CPDR/LDI/LDD/LDIR/LDDR operations */
	m_reg1 = get_reg32_current_sel(m_op - 1);
	m_reg2 = get_reg32_current_sel(m_op);

	m_ea2.d = get_reg32_current(m_op);
	execute_op(*m_mnemonic_90);
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP + d8) used as source in word operations */
void tlcs900_device::op_98()
{
	m_ea2.d = get_reg32_current(m_op);
	m_op = RDOP();
	m_ea2.d += int8_t(m_op);
	m_cycles += tlcs900_mem_index_cycles();
	execute_op(*m_mnemonic_98);
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP) used as source in long word operations */
void tlcs900_device::op_A0()
{
	m_ea2.d = get_reg32_current(m_op);
	execute_op(*m_mnemonic_a0);
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP + d8) used as source in long word operations */
void tlcs900_device::op_A8()
{
	m_ea2.d = get_reg32_current(m_op);
	m_op = RDOP();
	m_ea2.d += int8_t(m_op);
	m_cycles += tlcs900_mem_index_cycles();
	execute_op(*m_mnemonic_a0);
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP) used as destination in operations */
void tlcs900_device::op_B0()
{
	m_ea2.d = get_reg32_current(m_op);
	execute_op(*m_mnemonic_b0);
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP + d8) used as destination in operations */
void tlcs900_device::op_B8()
{
	m_ea2.d = get_reg32_current(m_op);
	m_op = RDOP();
	m_ea2.d += int8_t(m_op);
	m_cycles += tlcs900_mem_index_cycles();
	execute_op(*m_mnemonic_b8);
}


/* memory used as source in byte operations */
void tlcs900_device::op_C0()
{
	switch (m_op & 0x07)
	{
	case 0x00:  /* (n) */
		m_ea2.d = RDOP();
		m_cycles += tlcs900_mem_absolute_8_cycles();
		break;

	case 0x01:  /* (nn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_cycles += tlcs900_mem_absolute_16_cycles();
		break;

	case 0x02:  /* (nnn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_ea2.b.h2 = RDOP();
		m_cycles += tlcs900_mem_absolute_24_cycles();
		break;

	case 0x03:
		m_op = RDOP();
		switch (m_op & 0x03)
		{
		/* (xrr) */
		case 0x00:
			m_ea2.d = get_reg32(m_op);
			m_cycles += tlcs900_mem_gpr_indirect_cycles();
			break;

		/* (xrr+d16) */
		case 0x01:
			m_ea2.b.l = RDOP();
			m_ea2.b.h = RDOP();
			m_ea2.d = get_reg32(m_op) + m_ea2.sw.l;
			m_cycles += tlcs900_mem_gpr_index_cycles();
			break;

		/* unknown/illegal */
		case 0x02:
			break;

		case 0x03:
			switch (m_op)
			{
			/* (xrr+r8) */
			case 0x03:
				m_op = RDOP();
				m_ea2.d = get_reg32(m_op);
				m_op = RDOP();
				m_ea2.d += int8_t(get_reg8(m_op));
				m_cycles += tlcs900_mem_gpr_reg_index_cycles();
				break;

			/* (xrr+r16) */
			case 0x07:
				m_op = RDOP();
				m_ea2.d = get_reg32(m_op);
				m_op = RDOP();
				m_ea2.d += int16_t(get_reg16(m_op));
				m_cycles += tlcs900_mem_gpr_reg_index_cycles();
				break;

			/* (pc+d16) */
			case 0x13:
				m_ea2.b.l = RDOP();
				m_ea2.b.h = RDOP();
				m_ea2.d = m_pc.d + m_ea2.sw.l;
				m_cycles += tlcs900_mem_gpr_index_cycles();
				break;
			}
		}
		break;

	case 0x04:  /* (-xrr) */
	{
		m_op = RDOP();
		uint32_t &reg = get_reg32(m_op);
		reg -= (1 << (m_op & 0x03));
		m_ea2.d = reg;
		m_cycles += tlcs900_mem_indirect_prepost_cycles();
		break;
	}

	case 0x05:  /* (xrr+) */
	{
		m_op = RDOP();
		uint32_t &reg = get_reg32(m_op);
		m_ea2.d = reg;
		reg += (1 << (m_op & 0x03));
		m_cycles += tlcs900_mem_indirect_prepost_cycles();
		break;
	}
	}
	execute_op(*m_mnemonic_c0);
}


void tlcs900_device::oC8()
{
	if (BIT(m_op, 3))
	{
		m_reg2 = get_reg8_current_sel(m_op);
		/* for MUL and DIV the word view of the same register pair derives
		   from the selector */
	}
	else
	{
		m_op = RDOP();
		m_reg2 = m_op;
	}
	execute_op(*m_mnemonic_c8);
}


/* memory used as source in word operations */
void tlcs900_device::op_D0()
{
	switch (m_op & 0x07)
	{
	case 0x00:  /* (n) */
		m_ea2.d = RDOP();
		m_cycles += tlcs900_mem_absolute_8_cycles();
		break;

	case 0x01:  /* (nn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_cycles += tlcs900_mem_absolute_16_cycles();
		break;

	case 0x02:  /* (nnn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_ea2.b.h2 = RDOP();
		m_cycles += tlcs900_mem_absolute_24_cycles();
		break;

	case 0x03:
		m_op = RDOP();
		switch (m_op & 0x03)
		{
		/* (xrr) */
		case 0x00:
			m_ea2.d = get_reg32(m_op);
			m_cycles += tlcs900_mem_gpr_indirect_cycles();
			break;

		/* (xrr+d16) */
		case 0x01:
			m_ea2.b.l = RDOP();
			m_ea2.b.h = RDOP();
			m_ea2.d = get_reg32(m_op) + m_ea2.sw.l;
			m_cycles += tlcs900_mem_gpr_index_cycles();
			break;

		/* unknown/illegal */
		case 0x02:
			break;

		case 0x03:
			switch (m_op)
			{
			/* (xrr+r8) */
			case 0x03:
				m_op = RDOP();
				m_ea2.d = get_reg32(m_op);
				m_op = RDOP();
				m_ea2.d += int8_t(get_reg8(m_op));
				m_cycles += tlcs900_mem_gpr_reg_index_cycles();
				break;

			/* (xrr+r16) */
			case 0x07:
				m_op = RDOP();
				m_ea2.d = get_reg32(m_op);
				m_op = RDOP();
				m_ea2.d += int16_t(get_reg16(m_op));
				m_cycles += tlcs900_mem_gpr_reg_index_cycles();
				break;

			/* (pc+d16) */
			case 0x13:
				m_ea2.b.l = RDOP();
				m_ea2.b.h = RDOP();
				m_ea2.d = m_pc.d + m_ea2.sw.l;
				m_cycles += tlcs900_mem_gpr_reg_index_cycles();
				break;
			}
		}
		break;

	case 0x04:  /* (-xrr) */
	{
		m_op = RDOP();
		uint32_t &reg = get_reg32(m_op);
		reg -= (1 << (m_op & 0x03));
		m_ea2.d = reg;
		m_cycles += tlcs900_mem_indirect_prepost_cycles();
		break;
	}

	case 0x05:  /* (xrr+) */
	{
		m_op = RDOP();
		uint32_t &reg = get_reg32(m_op);
		m_ea2.d = reg;
		reg += (1 << (m_op & 0x03));
		m_cycles += tlcs900_mem_indirect_prepost_cycles();
		break;
	}
	}
	execute_op(*m_mnemonic_d0);
}


void tlcs900_device::oD8()
{
	if (BIT(m_op, 3))
	{
		m_reg2 = get_reg16_current_sel(m_op);
		/* for MUL and DIV the long word view of the same register pair
		   derives from the selector */
	}
	else
	{
		m_op = RDOP();
		m_reg2 = m_op;
	}
	execute_op(*m_mnemonic_d8);
}


/* memory used as source in long word operations */
void tlcs900_device::op_E0()
{
	switch (m_op & 0x07)
	{
	case 0x00:  /* (n) */
		m_ea2.d = RDOP();
		m_cycles += tlcs900_mem_absolute_8_cycles();
		break;

	case 0x01:  /* (nn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_cycles += tlcs900_mem_absolute_16_cycles();
		break;

	case 0x02:  /* (nnn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_ea2.b.h2 = RDOP();
		m_cycles += tlcs900_mem_absolute_24_cycles();
		break;

	case 0x03:
		m_op = RDOP();
		switch (m_op & 0x03)
		{
		/* (xrr) */
		case 0x00:
			m_ea2.d = get_reg32(m_op);
			m_cycles += tlcs900_mem_gpr_indirect_cycles();
			break;

		/* (xrr+d16) */
		case 0x01:
			m_ea2.b.l = RDOP();
			m_ea2.b.h = RDOP();
			m_ea2.d = get_reg32(m_op) + m_ea2.sw.l;
			m_cycles += tlcs900_mem_gpr_index_cycles();
			break;

		/* unknown/illegal */
		case 0x02:
			break;

		case 0x03:
			switch (m_op)
			{
			/* (xrr+r8) */
			case 0x03:
				m_op = RDOP();
				m_ea2.d = get_reg32(m_op);
				m_op = RDOP();
				m_ea2.d += int8_t(get_reg8(m_op));
				m_cycles += tlcs900_mem_gpr_reg_index_cycles();
				break;

			/* (xrr+r16) */
			case 0x07:
				m_op = RDOP();
				m_ea2.d = get_reg32(m_op);
				m_op = RDOP();
				m_ea2.d += int16_t(get_reg16(m_op));
				m_cycles += tlcs900_mem_gpr_reg_index_cycles();
				break;

			/* (pc+d16) */
			case 0x13:
				m_ea2.b.l = RDOP();
				m_ea2.b.h = RDOP();
				m_ea2.d = m_pc.d + m_ea2.sw.l;
				m_cycles += tlcs900_mem_gpr_index_cycles();
				break;
			}
		}
		break;

	case 0x04:  /* (-xrr) */
	{
		m_op = RDOP();
		uint32_t &reg = get_reg32(m_op);
		reg -= (1 << (m_op & 0x03));
		m_ea2.d = reg;
		m_cycles += tlcs900_mem_indirect_prepost_cycles();
		break;
	}

	case 0x05:  /* (xrr+) */
	{
		m_op = RDOP();
		uint32_t &reg = get_reg32(m_op);
		m_ea2.d = reg;
		reg += (1 << (m_op & 0x03));
		m_cycles += tlcs900_mem_indirect_prepost_cycles();
		break;
	}
	}
	execute_op(*m_mnemonic_e0);
}


void tlcs900_device::op_E8()
{
	if (BIT(m_op, 3))
	{
		m_reg2 = get_reg32_current_sel(m_op);
	}
	else
	{
		m_op = RDOP();
		m_reg2 = m_op;
	}
	execute_op(*m_mnemonic_e8);
}


/* memory used as destination operations */
void tlcs900_device::op_F0()
{
	switch (m_op & 0x07)
	{
	case 0x00:  /* (n) */
		m_ea2.d = RDOP();
		m_cycles += tlcs900_mem_absolute_8_cycles();
		break;

	case 0x01:  /* (nn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_cycles += tlcs900_mem_absolute_16_cycles();
		break;

	case 0x02:  /* (nnn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_ea2.b.h2 = RDOP();
		m_cycles += tlcs900_mem_absolute_24_cycles();
		break;

	case 0x03:
		m_op = RDOP();
		switch (m_op & 0x03)
		{
		/* (xrr) */
		case 0x00:
			m_ea2.d = get_reg32(m_op);
			m_cycles += tlcs900_mem_gpr_indirect_cycles();
			break;

		/* (xrr+d16) */
		case 0x01:
			m_ea2.b.l = RDOP();
			m_ea2.b.h = RDOP();
			m_ea2.d = get_reg32(m_op) + m_ea2.sw.l;
			m_cycles += tlcs900_mem_gpr_index_cycles();
			break;

		/* unknown/illegal */
		case 0x02:
			break;

		case 0x03:
			switch (m_op)
			{
			/* (xrr+r8) */
			case 0x03:
				m_op = RDOP();
				m_ea2.d = get_reg32(m_op);
				m_op = RDOP();
				m_ea2.d += int8_t(get_reg8(m_op));
				m_cycles += tlcs900_mem_gpr_reg_index_cycles();
				break;

			/* (xrr+r16) */
			case 0x07:
				m_op = RDOP();
				m_ea2.d = get_reg32(m_op);
				m_op = RDOP();
				m_ea2.d += int16_t(get_reg16(m_op));
				m_cycles += tlcs900_mem_gpr_reg_index_cycles();
				break;

			/* (pc+d16) */
			case 0x13:
				m_ea2.b.l = RDOP();
				m_ea2.b.h = RDOP();
				m_ea2.d = m_pc.d + m_ea2.sw.l;
				m_cycles += tlcs900_mem_gpr_index_cycles();
				break;
			}
		}
		break;

	case 0x04:  /* (-xrr) */
	{
		m_op = RDOP();
		uint32_t &reg = get_reg32(m_op);
		reg -= (1 << (m_op & 0x03));
		m_ea2.d = reg;
		m_cycles += tlcs900_mem_indirect_prepost_cycles();
		break;
	}

	case 0x05:  /* (xrr+) */
	{
		m_op = RDOP();
		uint32_t &reg = get_reg32(m_op);
		m_ea2.d = reg;
		reg += (1 << (m_op & 0x03));
		m_cycles += tlcs900_mem_indirect_prepost_cycles();
		break;
	}
	}

	execute_op(*m_mnemonic_f0);
}


const tlcs900_device::tlcs900inst tlcs900_device::s_mnemonic[256] =
{
	/* 00 - 1F */
	{ &tlcs900_device::op_NOP, 0, 0, 1 }, { &tlcs900_device::op_NORMAL, 0, 0, 4 }, { &tlcs900_device::op_PUSHWR, p_SR, 0, 4 }, { &tlcs900_device::op_POPWSR, p_SR, 0, 6 },
	{ &tlcs900_device::op_MAX, 0, 0, 4 }, { &tlcs900_device::op_HALT, 0, 0, 8 }, { &tlcs900_device::op_EI, p_I8, 0, 5 }, { &tlcs900_device::op_RETI, 0, 0, 12 },
	{ &tlcs900_device::op_LDBMI, p_M8, p_I8, 5 }, { &tlcs900_device::op_PUSHBI, p_I8, 0, 4 }, { &tlcs900_device::op_LDWMI, p_M8, p_I16, 6 }, { &tlcs900_device::op_PUSHWI, p_I16, 0, 5 },
	{ &tlcs900_device::op_INCF, 0, 0, 2 }, { &tlcs900_device::op_DECF, 0, 0, 2 }, { &tlcs900_device::op_RET, 0, 0, 9 }, { &tlcs900_device::op_RETD, p_I16, 0, 9 },
	{ &tlcs900_device::op_RCF, 0, 0, 2 }, { &tlcs900_device::op_SCF, 0, 0, 2 }, { &tlcs900_device::op_CCF, 0, 0, 2 }, { &tlcs900_device::op_ZCF, 0, 0, 2 },
	{ &tlcs900_device::op_PUSHBR, p_A, 0, 3 }, { &tlcs900_device::op_POPBR, p_A, 0, 4 }, { &tlcs900_device::op_EXBRR, p_F, p_F, 2 }, { &tlcs900_device::op_LDF, p_I8, 0, 2 },
	{ &tlcs900_device::op_PUSHBR, p_F, 0, 3 }, { &tlcs900_device::op_POPBR, p_F, 0, 4 }, { &tlcs900_device::op_JPI, p_I16, 0, 7 }, { &tlcs900_device::op_JPI, p_I24, 0, 7 },
	{ &tlcs900_device::op_CALLI, p_I16, 0, 12 }, { &tlcs900_device::op_CALLI, p_I24, 0, 12 }, { &tlcs900_device::op_CALR, p_D16, 0, 12 }, { &tlcs900_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900_device::op_LDBRI, p_C8, p_I8, 2 },
	{ &tlcs900_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900_device::op_LDBRI, p_C8, p_I8, 2 },
	{ &tlcs900_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900_device::op_PUSHWR, p_C16, 0, 3 },
	{ &tlcs900_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900_device::op_PUSHWR, p_C16, 0, 3 },
	{ &tlcs900_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900_device::op_LDWRI, p_C16, p_I16, 3 },
	{ &tlcs900_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900_device::op_LDWRI, p_C16, p_I16, 3 },
	{ &tlcs900_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900_device::op_PUSHLR, p_C32, 0, 5 },
	{ &tlcs900_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900_device::op_PUSHLR, p_C32, 0, 5 },

	/* 40 - 5F */
	{ &tlcs900_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900_device::op_LDLRI, p_C32, p_I32, 5 },
	{ &tlcs900_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900_device::op_LDLRI, p_C32, p_I32, 5 },
	{ &tlcs900_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900_device::op_POPWR, p_C16, 0, 4 },
	{ &tlcs900_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900_device::op_POPWR, p_C16, 0, 4 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 }, { &tlcs900_device::op_DB, 0, 0, 1 },
	{ &tlcs900_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900_device::op_POPLR, p_C32, 0, 6 },
	{ &tlcs900_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900_device::op_POPLR, p_C32, 0, 6 },

	/* 60 - 7F */
	{ &tlcs900_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900_device::op_JR, p_CC, p_D8, 4 },
	{ &tlcs900_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900_device::op_JR, p_CC, p_D8, 4 },
	{ &tlcs900_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900_device::op_JR, p_CC, p_D8, 4 },
	{ &tlcs900_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900_device::op_JR, p_CC, p_D8, 4 },
	{ &tlcs900_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900_device::op_JRL, p_CC, p_D16, 4 },
	{ &tlcs900_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900_device::op_JRL, p_CC, p_D16, 4 },
	{ &tlcs900_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900_device::op_JRL, p_CC, p_D16, 4 },
	{ &tlcs900_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900_device::op_JRL, p_CC, p_D16, 4 },

	/* 80 - 9F */
	{ &tlcs900_device::op_80, 0, 0, 0 }, { &tlcs900_device::op_80, 0, 0, 0 }, { &tlcs900_device::op_80, 0, 0, 0 }, { &tlcs900_device::op_80, 0, 0, 0 },
	{ &tlcs900_device::op_80, 0, 0, 0 }, { &tlcs900_device::op_80, 0, 0, 0 }, { &tlcs900_device::op_80, 0, 0, 0 }, { &tlcs900_device::op_80, 0, 0, 0 },
	{ &tlcs900_device::op_88, 0, 0, 0 }, { &tlcs900_device::op_88, 0, 0, 0 }, { &tlcs900_device::op_88, 0, 0, 0 }, { &tlcs900_device::op_88, 0, 0, 0 },
	{ &tlcs900_device::op_88, 0, 0, 0 }, { &tlcs900_device::op_88, 0, 0, 0 }, { &tlcs900_device::op_88, 0, 0, 0 }, { &tlcs900_device::op_88, 0, 0, 0 },
	{ &tlcs900_device::op_90, 0, 0, 0 }, { &tlcs900_device::op_90, 0, 0, 0 }, { &tlcs900_device::op_90, 0, 0, 0 }, { &tlcs900_device::op_90, 0, 0, 0 },
	{ &tlcs900_device::op_90, 0, 0, 0 }, { &tlcs900_device::op_90, 0, 0, 0 }, { &tlcs900_device::op_90, 0, 0, 0 }, { &tlcs900_device::op_90, 0, 0, 0 },
	{ &tlcs900_device::op_98, 0, 0, 0 }, { &tlcs900_device::op_98, 0, 0, 0 }, { &tlcs900_device::op_98, 0, 0, 0 }, { &tlcs900_device::op_98, 0, 0, 0 },
	{ &tlcs900_device::op_98, 0, 0, 0 }, { &tlcs900_device::op_98, 0, 0, 0 }, { &tlcs900_device::op_98, 0, 0, 0 }, { &tlcs900_device::op_98, 0, 0, 0 },

	/* A0 - BF */
	{ &tlcs900_device::op_A0, 0, 0, 0 }, { &tlcs900_device::op_A0, 0, 0, 0 }, { &tlcs900_device::op_A0, 0, 0, 0 }, { &tlcs900_device::op_A0, 0, 0, 0 },
	{ &tlcs900_device::op_A0, 0, 0, 0 }, { &tlcs900_device::op_A0, 0, 0, 0 }, { &tlcs900_device::op_A0, 0, 0, 0 }, { &tlcs900_device::op_A0, 0, 0, 0 },
	{ &tlcs900_device::op_A8, 0, 0, 0 }, { &tlcs900_device::op_A8, 0, 0, 0 }, { &tlcs900_device::op_A8, 0, 0, 0 }, { &tlcs900_device::op_A8, 0, 0, 0 },
	{ &tlcs900_device::op_A8, 0, 0, 0 }, { &tlcs900_device::op_A8, 0, 0, 0 }, { &tlcs900_device::op_A8, 0, 0, 0 }, { &tlcs900_device::op_A8, 0, 0, 0 },
	{ &tlcs900_device::op_B0, 0, 0, 0 }, { &tlcs900_device::op_B0, 0, 0, 0 }, { &tlcs900_device::op_B0, 0, 0, 0 }, { &tlcs900_device::op_B0, 0, 0, 0 },
	{ &tlcs900_device::op_B0, 0, 0, 0 }, { &tlcs900_device::op_B0, 0, 0, 0 }, { &tlcs900_device::op_B0, 0, 0, 0 }, { &tlcs900_device::op_B0, 0, 0, 0 },
	{ &tlcs900_device::op_B8, 0, 0, 0 }, { &tlcs900_device::op_B8, 0, 0, 0 }, { &tlcs900_device::op_B8, 0, 0, 0 }, { &tlcs900_device::op_B8, 0, 0, 0 },
	{ &tlcs900_device::op_B8, 0, 0, 0 }, { &tlcs900_device::op_B8, 0, 0, 0 }, { &tlcs900_device::op_B8, 0, 0, 0 }, { &tlcs900_device::op_B8, 0, 0, 0 },

	/* C0 - DF */
	{ &tlcs900_device::op_C0, 0, 0, 0 }, { &tlcs900_device::op_C0, 0, 0, 0 }, { &tlcs900_device::op_C0, 0, 0, 0 }, { &tlcs900_device::op_C0, 0, 0, 0 },
	{ &tlcs900_device::op_C0, 0, 0, 0 }, { &tlcs900_device::op_C0, 0, 0, 0 }, { &tlcs900_device::op_DB, 0, 0, 0 }, { &tlcs900_device::oC8, 0, 0, 0 },
	{ &tlcs900_device::oC8, 0, 0, 0 }, { &tlcs900_device::oC8, 0, 0, 0 }, { &tlcs900_device::oC8, 0, 0, 0 }, { &tlcs900_device::oC8, 0, 0, 0 },
	{ &tlcs900_device::oC8, 0, 0, 0 }, { &tlcs900_device::oC8, 0, 0, 0 }, { &tlcs900_device::oC8, 0, 0, 0 }, { &tlcs900_device::oC8, 0, 0, 0 },
	{ &tlcs900_device::op_D0, 0, 0, 0 }, { &tlcs900_device::op_D0, 0, 0, 0 }, { &tlcs900_device::op_D0, 0, 0, 0 }, { &tlcs900_device::op_D0, 0, 0, 0 },
	{ &tlcs900_device::op_D0, 0, 0, 0 }, { &tlcs900_device::op_D0, 0, 0, 0 }, { &tlcs900_device::op_DB, 0, 0, 0 }, { &tlcs900_device::oD8, 0, 0, 0 },
	{ &tlcs900_device::oD8, 0, 0, 0 }, { &tlcs900_device::oD8, 0, 0, 0 }, { &tlcs900_device::oD8, 0, 0, 0 }, { &tlcs900_device::oD8, 0, 0, 0 },
	{ &tlcs900_device::oD8, 0, 0, 0 }, { &tlcs900_device::oD8, 0, 0, 0 }, { &tlcs900_device::oD8, 0, 0, 0 }, { &tlcs900_device::oD8, 0, 0, 0 },

	/* E0 - FF */
	{ &tlcs900_device::op_E0, 0, 0, 0 }, { &tlcs900_device::op_E0, 0, 0, 0 }, { &tlcs900_device::op_E0, 0, 0, 0 }, { &tlcs900_device::op_E0, 0, 0, 0 },
	{ &tlcs900_device::op_E0, 0, 0, 0 }, { &tlcs900_device::op_E0, 0, 0, 0 }, { &tlcs900_device::op_DB, 0, 0, 0 }, { &tlcs900_device::op_E8, 0, 0, 0 },
	{ &tlcs900_device::op_E8, 0, 0, 0 }, { &tlcs900_device::op_E8, 0, 0, 0 }, { &tlcs900_device::op_E8, 0, 0, 0 }, { &tlcs900_device::op_E8, 0, 0, 0 },
	{ &tlcs900_device::op_E8, 0, 0, 0 }, { &tlcs900_device::op_E8, 0, 0, 0 }, { &tlcs900_device::op_E8, 0, 0, 0 }, { &tlcs900_device::op_E8, 0, 0, 0 },
	{ &tlcs900_device::op_F0, 0, 0, 0 }, { &tlcs900_device::op_F0, 0, 0, 0 }, { &tlcs900_device::op_F0, 0, 0, 0 }, { &tlcs900_device::op_F0, 0, 0, 0 },
	{ &tlcs900_device::op_F0, 0, 0, 0 }, { &tlcs900_device::op_F0, 0, 0, 0 }, { &tlcs900_device::op_DB, 0, 0, 0 }, { &tlcs900_device::op_LDX, 0, 0, 9 },
	{ &tlcs900_device::op_SWI900, p_I3, 0, 16 }, { &tlcs900_device::op_SWI900, p_I3, 0, 16 }, { &tlcs900_device::op_SWI900, p_I3, 0, 16 }, { &tlcs900_device::op_SWI900, p_I3, 0, 16 },
	{ &tlcs900_device::op_SWI900, p_I3, 0, 16 }, { &tlcs900_device::op_SWI900, p_I3, 0, 16 }, { &tlcs900_device::op_SWI900, p_I3, 0, 16 }, { &tlcs900_device::op_SWI900, p_I3, 0, 16 }
};
