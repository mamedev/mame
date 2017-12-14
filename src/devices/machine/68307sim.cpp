// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68307 SIM module */

#include "emu.h"
#include "68307sim.h"

/* ports */
#define m68307SIM_PACNT (0x10)
#define m68307SIM_PADDR (0x12)
#define m68307SIM_PADAT (0x14)
#define m68307SIM_PBCNT (0x16)
#define m68307SIM_PBDDR (0x18)
#define m68307SIM_PBDAT (0x1a)


/* interrupt logic */
#define m68307SIM_LICR1 (0x20)
#define m68307SIM_LICR2 (0x22)
#define m68307SIM_PICR  (0x24)
#define m68307SIM_PIVR  (0x26)

/* used for the CS logic */
#define m68307SIM_BR0 (0x40)
#define m68307SIM_OR0 (0x42)
#define m68307SIM_BR1 (0x44)
#define m68307SIM_OR1 (0x46)
#define m68307SIM_BR2 (0x48)
#define m68307SIM_OR2 (0x4a)
#define m68307SIM_BR3 (0x4c)
#define m68307SIM_OR3 (0x4e)

READ16_MEMBER( m68307_cpu_device::m68307_internal_sim_r )
{
	assert(m_m68307SIM);
	m68307_sim &sim = *m_m68307SIM;

	switch (offset<<1)
	{
		case m68307SIM_PADAT: return sim.read_padat(this, space, mem_mask);
		case m68307SIM_PBDAT: return sim.read_pbdat(this, space, mem_mask);

		case m68307SIM_LICR2: return sim.m_licr2;

		case m68307SIM_BR0: return sim.m_br[0];
		case m68307SIM_OR0: return sim.m_or[0];
		case m68307SIM_BR1: return sim.m_br[1];
		case m68307SIM_OR1: return sim.m_or[1];
		case m68307SIM_BR2: return sim.m_br[2];
		case m68307SIM_OR2: return sim.m_or[2];
		case m68307SIM_BR3: return sim.m_br[3];
		case m68307SIM_OR3: return sim.m_or[3];

		default:
			logerror("%08x m68307_internal_sim_r %08x, (%04x)\n", m_ppc, offset*2, mem_mask);
			return 0xff;
	}

	return 0x0000;
}


WRITE16_MEMBER( m68307_cpu_device::m68307_internal_sim_w )
{
	assert(m_m68307SIM);
	m68307_sim &sim = *m_m68307SIM;

	switch (offset<<1)
	{
		case m68307SIM_PACNT:
			logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Port A (8-bit) Control Register - PACNT)\n", m_ppc, offset*2,data,mem_mask);
			sim.write_pacnt(data,mem_mask);
			break;

		case m68307SIM_PADDR:
			logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Port A (8-bit) Direction Register - PADDR)\n", m_ppc, offset*2,data,mem_mask);
			sim.write_paddr(data,mem_mask);
			break;

		case m68307SIM_PADAT:
			sim.write_padat(this, space, data,mem_mask);
			break;

		case m68307SIM_PBCNT:
			logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Port B (16-bit) Control Register - PBCNT)\n", m_ppc, offset*2,data,mem_mask);
			sim.write_pbcnt(data,mem_mask);
			break;

		case m68307SIM_PBDDR:
			logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Port B (16-bit) Direction Register - PBDDR)\n", m_ppc, offset*2,data,mem_mask);
			sim.write_pbddr(data,mem_mask);
			break;

		case m68307SIM_PBDAT:
			sim.write_pbdat(this, space, data, mem_mask);
			break;


		case m68307SIM_LICR1:
			logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Latched Interrupt Control Register 1 - LICR1)\n", m_ppc, offset*2,data,mem_mask);
			sim.write_licr1(this,data,mem_mask);
			break;

		case m68307SIM_LICR2:
			logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Latched Interrupt Control Register 2 - LICR2)\n", m_ppc, offset*2,data,mem_mask);
			sim.write_licr2(this,data,mem_mask);
			break;

		case m68307SIM_PICR:
			logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Peripheral Interrupt Control Register - PICR)\n", m_ppc, offset*2,data,mem_mask);
			sim.write_picr(this,data,mem_mask);
			break;

		case m68307SIM_PIVR:
			logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Peripheral Interrupt Vector Register - PIVR)\n", m_ppc, offset*2,data,mem_mask);
			sim.write_pivr(this,data,mem_mask);
			break;

		case m68307SIM_BR0:
			COMBINE_DATA(&sim.m_br[0]);
			break;
		case m68307SIM_OR0:
			COMBINE_DATA(&sim.m_or[0]);
			break;
		case m68307SIM_BR1:
			COMBINE_DATA(&sim.m_br[1]);
			break;
		case m68307SIM_OR1:
			COMBINE_DATA(&sim.m_or[1]);
			break;
		case m68307SIM_BR2:
			COMBINE_DATA(&sim.m_br[2]);
			break;
		case m68307SIM_OR2:
			COMBINE_DATA(&sim.m_or[2]);
			break;
		case m68307SIM_BR3:
			COMBINE_DATA(&sim.m_br[3]);
			break;
		case m68307SIM_OR3:
			COMBINE_DATA(&sim.m_or[3]);
			break;



		default :
			logerror("%08x m68307_internal_sim_w %08x, %04x (%04x)\n", m_ppc, offset*2,data,mem_mask);
			break;
	}
}


void m68307_cpu_device::m68307_sim::write_pacnt(uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pacnt);
}

void m68307_cpu_device::m68307_sim::write_paddr(uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_paddr);
}


uint16_t m68307_cpu_device::m68307_sim::read_padat(m68307_cpu_device* m68k, address_space &space, uint16_t mem_mask)
{
	if (!m68k->m_porta_r.isnull())
	{
		// for general purpose bits, if configured as 'output' then anything output gets latched
		// and anything configured as input is read from the port
		uint8_t outputbits = m_paddr;
		uint8_t inputbits = ~m_paddr;
		uint8_t general_purpose_bits = ~m_pacnt;
		uint8_t indat = m68k->m_porta_r(space, false, (inputbits & general_purpose_bits)&mem_mask) & ((inputbits & general_purpose_bits) & mem_mask); // read general purpose input lines
		indat |= m68k->m_porta_r(space, true, (inputbits & ~general_purpose_bits)&mem_mask) & ((inputbits & ~general_purpose_bits)& mem_mask); // read dedicated input lines
		uint8_t outdat = (m_padat & outputbits) & general_purpose_bits; // read general purpose output lines (reads latched data)

		return (indat | outdat);

	}
	else
	{
		m68k->logerror("%08x m68307_internal_sim_r (%04x) (Port A (8-bit) Data Register - PADAT)\n", m68k->pcbase(), mem_mask);
	}
	return 0xffff;
}


void m68307_cpu_device::m68307_sim::write_padat(m68307_cpu_device* m68k, address_space &space, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_padat);

	if (!m68k->m_porta_w.isnull())
	{
		m68k->m_porta_w(space, false, data, 0xff);
	}
	else
	{
		m68k->logerror("%08x m68307_internal_sim_w %04x (%04x) (Port A (8-bit) Data Register - PADAT)\n", m68k->pcbase(), data,mem_mask);
	}
}

void m68307_cpu_device::m68307_sim::write_pbcnt(uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pbcnt);
}

void m68307_cpu_device::m68307_sim::write_pbddr(uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pbddr);
}

uint16_t m68307_cpu_device::m68307_sim::read_pbdat(m68307_cpu_device* m68k, address_space &space, uint16_t mem_mask)
{
	if (!m68k->m_portb_r.isnull())
	{
		// for general purpose bits, if configured as 'output' then anything output gets latched
		// and anything configured as input is read from the port
		uint16_t outputbits = m_pbddr;
		uint16_t inputbits = ~m_pbddr;
		uint16_t general_purpose_bits = ~m_pbcnt;

		uint16_t indat = m68k->m_portb_r(space, false, (inputbits & general_purpose_bits)&mem_mask) & ((inputbits & general_purpose_bits) & mem_mask); // read general purpose input lines
		indat |= m68k->m_portb_r(space, true, (inputbits & ~general_purpose_bits)&mem_mask) & ((inputbits & ~general_purpose_bits)& mem_mask); // read dedicated input lines
		uint16_t outdat = (m_pbdat & outputbits) & general_purpose_bits; // read general purpose output lines (reads latched data)

		return (indat | outdat);
	}
	else
	{
		m68k->logerror("%08x m68307_internal_sim_r (%04x) (Port B (16-bit) Data Register - PBDAT)\n", m68k->pcbase(), mem_mask);
	}
	return 0xffff;
}


void m68307_cpu_device::m68307_sim::write_pbdat(m68307_cpu_device* m68k, address_space &space, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pbdat);

	if (!m68k->m_portb_w.isnull())
	{
		m68k->m_portb_w(space, false, data, mem_mask);
	}
	else
	{
		m68k->logerror("%08x m68307_internal_sim_w %04x (%04x) (Port B (16-bit) Data Register - PBDAT)\n", m68k->pcbase(), data,mem_mask);
	}
}

void m68307_cpu_device::m68307_sim::write_licr1(m68307_cpu_device* m68k, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_licr1);
	data = m_licr1;
	m68k->logerror("m_licr1 value %04x : Details :\n", data);
	m68k->logerror("int4ipl %01x\n", (data>>0)&7);
	m68k->logerror("pir4    %01x\n", (data>>3)&1);
	m68k->logerror("int3ipl %01x\n", (data>>4)&7);
	m68k->logerror("pir3    %01x\n", (data>>7)&1);
	m68k->logerror("int2ipl %01x\n", (data>>8)&7);
	m68k->logerror("pir2    %01x\n", (data>>11)&1);
	m68k->logerror("int1ipl %01x\n", (data>>12)&7);
	m68k->logerror("pir1    %01x\n", (data>>15)&1);
	m68k->logerror("\n");
}

void m68307_cpu_device::m68307_sim::write_licr2(m68307_cpu_device* m68k, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_licr2);
	uint16_t newdata = m_licr2;
	m68k->logerror("m_licr2 value %04x : Details :\n", newdata);
	m68k->logerror("int8ipl %01x\n", (newdata>>0)&7);
	m68k->logerror("pir8    %01x\n", (newdata>>3)&1);
	m68k->logerror("int7ipl %01x\n", (newdata>>4)&7);
	m68k->logerror("pir7    %01x\n", (newdata>>7)&1);
	m68k->logerror("int6ipl %01x\n", (newdata>>8)&7);
	m68k->logerror("pir6    %01x\n", (newdata>>11)&1);
	m68k->logerror("int5ipl %01x\n", (newdata>>12)&7);
	m68k->logerror("pir5    %01x\n", (newdata>>15)&1);
	m68k->logerror("\n");

	if (data & 0x0008) m_licr2 = m_licr2 & ~0x0008;
	if (data & 0x0080) m_licr2 = m_licr2 & ~0x0080;
	if (data & 0x0800) m_licr2 = m_licr2 & ~0x0800;
	if (data & 0x8000) m_licr2 = m_licr2 & ~0x8000;


}


void m68307_cpu_device::m68307_sim::write_picr(m68307_cpu_device* m68k, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_picr);
	data = m_picr;
	m68k->logerror("picr value %04x : Details :\n", data);
	m68k->logerror("mbipl %01x\n", (data>>0)&7);
	m68k->logerror("uaipl %01x\n", (data>>4)&7);
	m68k->logerror("t2ipl %01x\n", (data>>8)&7);
	m68k->logerror("t1ipl %01x\n", (data>>12)&7);
	m68k->logerror("\n");
}

void m68307_cpu_device::m68307_sim::write_pivr(m68307_cpu_device* m68k, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pivr);
	data = m_pivr;
	m68k->logerror("pivr value %04x : Details :\n", data);
	m68k->logerror("unused %01x\n", (data>>0)&0xf);
	m68k->logerror("high vector %01x\n", (data>>4)&0xf);
}

void m68307_cpu_device::m68307_sim::reset()
{
	for (int i=0;i<4;i++)
	{
		m_br[i] = 0xc001;
		m_or[i] = 0xdffd;
	}
}
