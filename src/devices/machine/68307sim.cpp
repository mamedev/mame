// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68307 SIM module */

#include "emu.h"
#include "68307.h"


READ16_MEMBER( m68307cpu_device::m68307_internal_sim_r )
{
	m68307cpu_device *m68k = this;
	m68307_sim* sim = m68k->m68307SIM;
	assert(sim != NULL);

	int pc = space.device().safe_pc();

	if (sim)
	{
		switch (offset<<1)
		{
			case m68307SIM_PADAT: return sim->read_padat(this, space, mem_mask);
			case m68307SIM_PBDAT: return sim->read_pbdat(this, space, mem_mask);

			case m68307SIM_LICR2: return  (sim->m_licr2);

			case m68307SIM_BR0: return (sim->m_br[0]);
			case m68307SIM_OR0: return (sim->m_or[0]);
			case m68307SIM_BR1: return (sim->m_br[1]);
			case m68307SIM_OR1: return (sim->m_or[1]);
			case m68307SIM_BR2: return (sim->m_br[2]);
			case m68307SIM_OR2: return (sim->m_or[2]);
			case m68307SIM_BR3: return (sim->m_br[3]);
			case m68307SIM_OR3: return (sim->m_or[3]);

			default:
				logerror("%08x m68307_internal_sim_r %08x, (%04x)\n", pc, offset*2,mem_mask);
				return 0xff;

		}
	}

	return 0x0000;
}


WRITE16_MEMBER( m68307cpu_device::m68307_internal_sim_w )
{
	m68307cpu_device *m68k = this;
	m68307_sim* sim = m68k->m68307SIM;
	assert(sim != NULL);

	int pc = space.device().safe_pc();

	if (sim)
	{
		switch (offset<<1)
		{
			case m68307SIM_PACNT:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Port A (8-bit) Control Register - PACNT)\n", pc, offset*2,data,mem_mask);
				sim->write_pacnt(data,mem_mask);
				break;

			case m68307SIM_PADDR:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Port A (8-bit) Direction Register - PADDR)\n", pc, offset*2,data,mem_mask);
				sim->write_paddr(data,mem_mask);
				break;

			case m68307SIM_PADAT:
				sim->write_padat(this, space, data,mem_mask);
				break;

			case m68307SIM_PBCNT:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Port B (16-bit) Control Register - PBCNT)\n", pc, offset*2,data,mem_mask);
				sim->write_pbcnt(data,mem_mask);
				break;

			case m68307SIM_PBDDR:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Port B (16-bit) Direction Register - PBDDR)\n", pc, offset*2,data,mem_mask);
				sim->write_pbddr(data,mem_mask);
				break;

			case m68307SIM_PBDAT:
				sim->write_pbdat(this, space, data, mem_mask);
				break;


			case m68307SIM_LICR1:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Latched Interrupt Control Register 1 - LICR1)\n", pc, offset*2,data,mem_mask);
				sim->write_licr1(this,data,mem_mask);
				break;

			case m68307SIM_LICR2:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Latched Interrupt Control Register 2 - LICR2)\n", pc, offset*2,data,mem_mask);
				sim->write_licr2(this,data,mem_mask);
				break;

			case m68307SIM_PICR:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Peripheral Interrupt Control Register - PICR)\n", pc, offset*2,data,mem_mask);
				sim->write_picr(this,data,mem_mask);
				break;

			case m68307SIM_PIVR:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Peripheral Interrupt Vector Register - PIVR)\n", pc, offset*2,data,mem_mask);
				sim->write_pivr(this,data,mem_mask);
				break;

			case m68307SIM_BR0:
				COMBINE_DATA(&sim->m_br[0]);
				break;
			case m68307SIM_OR0:
				COMBINE_DATA(&sim->m_or[0]);
				break;
			case m68307SIM_BR1:
				COMBINE_DATA(&sim->m_br[1]);
				break;
			case m68307SIM_OR1:
				COMBINE_DATA(&sim->m_or[1]);
				break;
			case m68307SIM_BR2:
				COMBINE_DATA(&sim->m_br[2]);
				break;
			case m68307SIM_OR2:
				COMBINE_DATA(&sim->m_or[2]);
				break;
			case m68307SIM_BR3:
				COMBINE_DATA(&sim->m_br[3]);
				break;
			case m68307SIM_OR3:
				COMBINE_DATA(&sim->m_or[3]);
				break;



			default :
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x)\n", pc, offset*2,data,mem_mask);
				break;

		}
	}
}


void m68307_sim::write_pacnt(UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_pacnt);
}

void m68307_sim::write_paddr(UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_paddr);
}


UINT16 m68307_sim::read_padat(m68307cpu_device* m68k, address_space &space, UINT16 mem_mask)
{
	int pc = space.device().safe_pc();

	if (!m68k->m_m68307_porta_r.isnull())
	{
		// for general purpose bits, if configured as 'output' then anything output gets latched
		// and anything configured as input is read from the port
		UINT8 outputbits = m_paddr;
		UINT8 inputbits = ~m_paddr;
		UINT8 general_purpose_bits = ~m_pacnt;
		UINT8 indat = m68k->m_m68307_porta_r(space, false, (inputbits & general_purpose_bits)&mem_mask) & ((inputbits & general_purpose_bits) & mem_mask); // read general purpose input lines
		indat |= m68k->m_m68307_porta_r(space, true, (inputbits & ~general_purpose_bits)&mem_mask) & ((inputbits & ~general_purpose_bits)& mem_mask); // read dedicated input lines
		UINT8 outdat = (m_padat & outputbits) & general_purpose_bits; // read general purpose output lines (reads latched data)

		return (indat | outdat);

	}
	else
	{
		m68k->logerror("%08x m68307_internal_sim_r (%04x) (Port A (8-bit) Data Register - PADAT)\n", pc, mem_mask);
	}
	return 0xffff;
}


void m68307_sim::write_padat(m68307cpu_device* m68k, address_space &space, UINT16 data, UINT16 mem_mask)
{
	int pc = space.device().safe_pc();
	COMBINE_DATA(&m_padat);

	if (!m68k->m_m68307_porta_w.isnull())
	{
		m68k->m_m68307_porta_w(space, false, data, 0xff);
	}
	else
	{
		m68k->logerror("%08x m68307_internal_sim_w %04x (%04x) (Port A (8-bit) Data Register - PADAT)\n", pc, data,mem_mask);
	}
}

void m68307_sim::write_pbcnt(UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_pbcnt);
}

void m68307_sim::write_pbddr(UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_pbddr);
}

UINT16 m68307_sim::read_pbdat(m68307cpu_device* m68k, address_space &space, UINT16 mem_mask)
{
	int pc = space.device().safe_pc();

	if (!m68k->m_m68307_portb_r.isnull())
	{
		// for general purpose bits, if configured as 'output' then anything output gets latched
		// and anything configured as input is read from the port
		UINT16 outputbits = m_pbddr;
		UINT16 inputbits = ~m_pbddr;
		UINT16 general_purpose_bits = ~m_pbcnt;

		UINT16 indat = m68k->m_m68307_portb_r(space, false, (inputbits & general_purpose_bits)&mem_mask) & ((inputbits & general_purpose_bits) & mem_mask); // read general purpose input lines
		indat |= m68k->m_m68307_portb_r(space, true, (inputbits & ~general_purpose_bits)&mem_mask) & ((inputbits & ~general_purpose_bits)& mem_mask); // read dedicated input lines
		UINT16 outdat = (m_pbdat & outputbits) & general_purpose_bits; // read general purpose output lines (reads latched data)

		return (indat | outdat);
	}
	else
	{
		m68k->logerror("%08x m68307_internal_sim_r (%04x) (Port B (16-bit) Data Register - PBDAT)\n", pc, mem_mask);
	}
	return 0xffff;
}


void m68307_sim::write_pbdat(m68307cpu_device* m68k, address_space &space, UINT16 data, UINT16 mem_mask)
{
	int pc = space.device().safe_pc();
	COMBINE_DATA(&m_pbdat);

	if (!m68k->m_m68307_portb_w.isnull())
	{
		m68k->m_m68307_portb_w(space, false, data, mem_mask);
	}
	else
	{
		m68k->logerror("%08x m68307_internal_sim_w %04x (%04x) (Port B (16-bit) Data Register - PBDAT)\n", pc, data,mem_mask);
	}
}

void m68307_sim::write_licr1(m68307cpu_device* m68k, UINT16 data, UINT16 mem_mask)
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

void m68307_sim::write_licr2(m68307cpu_device* m68k, UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_licr2);
	UINT16 newdata = m_licr2;
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


void m68307_sim::write_picr(m68307cpu_device* m68k, UINT16 data, UINT16 mem_mask)
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

void m68307_sim::write_pivr(m68307cpu_device* m68k, UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_pivr);
	data = m_pivr;
	m68k->logerror("pivr value %04x : Details :\n", data);
	m68k->logerror("unused %01x\n", (data>>0)&0xf);
	m68k->logerror("high vector %01x\n", (data>>4)&0xf);
}

void m68307_sim::reset(void)
{
	for (int i=0;i<4;i++)
	{
		m_br[i] = 0xc001;
		m_or[i] = 0xdffd;
	}
}
