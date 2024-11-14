// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood

#include "emu.h"
#include "ms1_gatearray.h"

//#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(MEGASYS1_GATEARRAY_D65006, megasys1_gatearray_d65006_device, "ms1_d65005", "Mega System 1 Gate Array D65005")
DEFINE_DEVICE_TYPE(MEGASYS1_GATEARRAY_GS88000, megasys1_gatearray_gs88000_device, "ms1_gs88000", "Mega System 1 Gate Array GS-88000")
DEFINE_DEVICE_TYPE(MEGASYS1_GATEARRAY_UNKARRAY, megasys1_gatearray_unkarray_device, "ms1_unkarray", "Mega System 1 Gate Array UNKNOWN")

megasys1_gatearray_device::megasys1_gatearray_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_cpuregion(*this, finder_base::DUMMY_TAG)
{
}


void megasys1_gatearray_device::device_start()
{
	save_item(NAME(m_gatearray_hs));
	save_item(NAME(m_gatearray_hs_ram));
	rom_decode();
	install_overlay();
}

void megasys1_gatearray_device::device_reset()
{
	m_gatearray_hs = 0;
}


void megasys1_gatearray_d65006_device::rom_decode()
{
	u16 *RAM = (u16 *)m_cpuregion->base();
	int size = m_cpuregion->bytes();
	if (size > 0x40000) size = 0x40000;

	for (int i = 0 ; i < size/2 ; i++)
	{
		const u16 x = RAM[i];

		auto const BITSWAP_0 = [x] () { return bitswap<16>(x,0xd,0xe,0xf,0x0,0x1,0x8,0x9,0xa,0xb,0xc,0x5,0x6,0x7,0x2,0x3,0x4); };
		auto const BITSWAP_1 = [x] () { return bitswap<16>(x,0xf,0xd,0xb,0x9,0x7,0x5,0x3,0x1,0xe,0xc,0xa,0x8,0x6,0x4,0x2,0x0); };
		auto const BITSWAP_2 = [x] () { return bitswap<16>(x,0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0xb,0xa,0x9,0x8,0xf,0xe,0xd,0xc); };

		u16 y;
		if      (i < 0x08000/2) { y = ( (i | (0x248/2)) != i ) ? BITSWAP_0() : BITSWAP_1(); }
		else if (i < 0x10000/2) { y = BITSWAP_2(); }
		else if (i < 0x18000/2) { y = ( (i | (0x248/2)) != i ) ? BITSWAP_0() : BITSWAP_1(); }
		else if (i < 0x20000/2) { y = BITSWAP_1(); }
		else                    { y = BITSWAP_2(); }

		RAM[i] = y;
	}

}

void megasys1_gatearray_gs88000_device::rom_decode()
{
	u16 *RAM = (u16 *)m_cpuregion->base();
	int size = m_cpuregion->bytes();
	if (size > 0x40000) size = 0x40000;

	for (int i = 0 ; i < size/2 ; i++)
	{
		const u16 x = RAM[i];

		auto const BITSWAP_0 = [x] () { return bitswap<16>(x,0xd,0xe,0xf,0x0,0xa,0x9,0x8,0x1,0x6,0x5,0xc,0xb,0x7,0x2,0x3,0x4); };
		auto const BITSWAP_1 = [x] () { return bitswap<16>(x,0xf,0xd,0xb,0x9,0x7,0x5,0x3,0x1,0x8,0xa,0xc,0xe,0x0,0x2,0x4,0x6); };
		auto const BITSWAP_2 = [x] () { return bitswap<16>(x,0x4,0x5,0x6,0x7,0x0,0x1,0x2,0x3,0xb,0xa,0x9,0x8,0xf,0xe,0xd,0xc); };

		u16 y;
		if      (i < 0x08000/2) { y = ( (i | (0x248/2)) != i ) ? BITSWAP_0() : BITSWAP_1(); }
		else if (i < 0x10000/2) { y = BITSWAP_2(); }
		else if (i < 0x18000/2) { y = ( (i | (0x248/2)) != i ) ? BITSWAP_0() : BITSWAP_1(); }
		else if (i < 0x20000/2) { y = BITSWAP_1(); }
		else                    { y = BITSWAP_2(); }

		RAM[i] = y;
	}
}

void megasys1_gatearray_unkarray_device::rom_decode()
{
	u16 *RAM = (u16 *)m_cpuregion->base();
	int size = m_cpuregion->bytes();
	if (size > 0x40000) size = 0x40000;

	for (int i = 0 ; i < size/2 ; i++)
	{
		const u16 x = RAM[i];

		auto const BITSWAP_0 = [x] () { return bitswap<16>(x,0xd,0x0,0xa,0x9,0x6,0xe,0xb,0xf,0x5,0xc,0x7,0x2,0x3,0x8,0x1,0x4); };
		auto const BITSWAP_1 = [x] () { return bitswap<16>(x,0x4,0x5,0x6,0x7,0x0,0x1,0x2,0x3,0xb,0xa,0x9,0x8,0xf,0xe,0xd,0xc); };
		auto const BITSWAP_2 = [x] () { return bitswap<16>(x,0xf,0xd,0xb,0x9,0xc,0xe,0x0,0x7,0x5,0x3,0x1,0x8,0xa,0x2,0x4,0x6); };
		auto const BITSWAP_3 = [x] () { return bitswap<16>(x,0x4,0x5,0x1,0x2,0xe,0xd,0x3,0xb,0xa,0x9,0x6,0x7,0x0,0x8,0xf,0xc); };

		u16 y;
		if      (i < 0x08000/2) { y = ( (i | (0x248/2)) != i ) ? BITSWAP_0() : BITSWAP_1(); }
		else if (i < 0x10000/2) { y = ( (i | (0x248/2)) != i ) ? BITSWAP_2() : BITSWAP_3(); }
		else if (i < 0x18000/2) { y = ( (i | (0x248/2)) != i ) ? BITSWAP_0() : BITSWAP_1(); }
		else if (i < 0x20000/2) { y = BITSWAP_1(); }
		else                    { y = BITSWAP_3(); }

		RAM[i] = y;
	}
}


/*
    Gate array handshake sequence:
    the UPD65006 gate array can overlay 0x40 bytes of data inside the ROM space.
    The offset where this happens is given by m68k to UPD65006 write [0x8/2] << 6.
    For example stdragon writes 0x33e -> maps at 0xcf80-0xcfbf while stdragona writes 0x33f -> maps at 0xcfc0-0xcfff.
    Note: stdragona forgets to turn off the overlay before the ROM check in service mode (hence it reports an error).
*/



inline bool megasys1_gatearray_device::hs_seq() const
{

	return
			m_gatearray_hs_ram[0/2] == m_gatearray_seq[0] &&
			m_gatearray_hs_ram[2/2] == m_gatearray_seq[1] &&
			m_gatearray_hs_ram[4/2] == m_gatearray_seq[2] &&
			m_gatearray_hs_ram[6/2] == m_gatearray_seq[3];
}


u16 megasys1_gatearray_device::gatearray_r(offs_t offset, u16 mem_mask)
{
	u16 *rom_maincpu = (u16 *)m_cpuregion->base();
	if(m_gatearray_hs && ((m_gatearray_hs_ram[8/2] << 6) & 0x3ffc0) == ((offset*2) & 0x3ffc0))
	{
		LOG("GATEARRAY HS R (%04x) <- [%02x]\n",mem_mask,offset*2);

		if (m_gatearray_seq)
			return m_gatearray_seq[4];
		else
			return 0;
	}
	return rom_maincpu[offset];
}

void megasys1_gatearray_device::gatearray_w(offs_t offset, u16 data, u16 mem_mask)
{
	// astyanax writes to 0x2000x
	// iganinju writes to 0x2f00x
	// stdragon writes to 0x23ffx
	// tshingen writes to 0x20c0x
	// assume writes mirror across the space

	if (!m_gatearray_seq)
	{
		logerror("Write to ROM area with no gatearray sequence");
		return;
	}

	offset &= 0x7;

	COMBINE_DATA(&m_gatearray_hs_ram[offset]);

	if (hs_seq() && offset == 0x8/2)
		m_gatearray_hs = 1;
	else
		m_gatearray_hs = 0;

	LOG("GATEARRAY HS W %04x (%04x) -> [%02x]\n",data,mem_mask,offset*2);
}

void megasys1_gatearray_device::install_overlay()
{
	m_cpu->space(AS_PROGRAM).install_read_handler(0x00000, 0x3ffff, read16s_delegate(*this, FUNC(megasys1_gatearray_device::gatearray_r)));
	m_cpu->space(AS_PROGRAM).install_write_handler(0x20000, 0x2ffff, write16s_delegate(*this, FUNC(megasys1_gatearray_device::gatearray_w)));
	m_gatearray_hs = 0;
	memset(m_gatearray_hs_ram, 0, sizeof(m_gatearray_hs_ram));
}


megasys1_gatearray_d65006_device::megasys1_gatearray_d65006_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: megasys1_gatearray_d65006_device(mconfig, MEGASYS1_GATEARRAY_D65006, tag, owner, clock)
{
	m_gatearray_seq = jaleco_d65006_unlock_sequence;
}

megasys1_gatearray_d65006_device::megasys1_gatearray_d65006_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: megasys1_gatearray_device(mconfig, type, tag, owner, clock)
{
}

megasys1_gatearray_gs88000_device::megasys1_gatearray_gs88000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: megasys1_gatearray_gs88000_device(mconfig, MEGASYS1_GATEARRAY_GS88000, tag, owner, clock)
{
	m_gatearray_seq = jaleco_gs88000_unlock_sequence;
}

megasys1_gatearray_gs88000_device::megasys1_gatearray_gs88000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: megasys1_gatearray_device(mconfig, type, tag, owner, clock)
{
}


megasys1_gatearray_unkarray_device::megasys1_gatearray_unkarray_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: megasys1_gatearray_unkarray_device(mconfig, MEGASYS1_GATEARRAY_UNKARRAY, tag, owner, clock)
{
	m_gatearray_seq = nullptr; // only used by rodland, which doesn't enable an overlay
}

megasys1_gatearray_unkarray_device::megasys1_gatearray_unkarray_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: megasys1_gatearray_device(mconfig, type, tag, owner, clock)
{
}

