// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner
/* HNG64 Communication / Network CPU */

// this is driven by a KL5C80A12CFP which is basically a super-charged Z80
// most of this MMU handling etc. should be moved to the core.

// I believe this CPU is used for network only (the racing games) and not related to the I/O MCU

/*
0x6010: tests RAM at [3]8000

*/

#include "emu.h"
#include "includes/hng64.h"
#include "cpu/z80/kl5c80a12.h"

uint8_t hng64_state::read_comm_data(uint32_t offset)
{
	if((offset & 0x10000) == 0)
		return m_comm_rom[offset & 0xffff];

	if(offset & 0x10000)
		return m_comm_ram[(offset & 0xffff)];

	printf("%08x\n",offset);
	return 0xff;
}

void hng64_state::write_comm_data(uint32_t offset,uint8_t data)
{
	if((offset & 0x10000) == 0)
	{
		//m_comm_rom[offset];
		return;
	}
	if(offset & 0x10000)
	{
		m_comm_ram[offset & 0xffff] = data;
		return;
	}


	printf("%08x %02x\n",offset,data);

}

READ8_MEMBER(hng64_state::hng64_comm_space_r)
{
	if((offset & 0xfc00) == 0) // B0 is fixed at 0-0x3ff
		return m_comm_rom[offset];

	for(int i=0;i<5;i++)
	{
		if(offset >= m_mmub[i] && offset <= m_mmub[i+1]-1)
			return read_comm_data(m_mmua[i]|offset);
	}

	return 0xff;
}

WRITE8_MEMBER(hng64_state::hng64_comm_space_w)
{
	if((offset & 0xfc00) == 0) // B0 is fixed at 0-0x3ff
		return;// m_comm_rom[offset];

	for(int i=0;i<5;i++)
	{
		if(offset >= m_mmub[i] && offset <= m_mmub[i+1]-1)
		{
			write_comm_data(m_mmua[i]|offset,data);
			return;
		}
	}
}

READ8_MEMBER(hng64_state::hng64_comm_mmu_r)
{
	return m_mmu_regs[offset];
}

#define MMUA (m_mmu_regs[(offset&~1)+0]>>6)|(m_mmu_regs[(offset&~1)+1]<<2)
#define MMUB (m_mmu_regs[(offset&~1)+0]&0x3f)

WRITE8_MEMBER(hng64_state::hng64_comm_mmu_w)
{
	m_mmu_regs[offset] = data;

	/* cheap: avoid to overwrite read only params*/
	if((offset & 6) == 6)
	{
		m_mmu_regs[6] = m_mmu_regs[6] & 0x3f;
		m_mmu_regs[7] = 0xf0;

	}

	{
		m_mmua[offset/2+1] = (m_mmu_regs[(offset&~1)+0]>>6)|(m_mmu_regs[(offset&~1)+1]<<2);
		m_mmua[offset/2+1]*= 0x400;
		m_mmub[offset/2+1] = (m_mmu_regs[(offset&~1)+0]&0x3f);
		m_mmub[offset/2+1]++;
		m_mmub[offset/2+1]*= 0x400;
		//printf("%d A %08x B %04x\n",offset/2,m_mmua[offset/2],m_mmub[offset/2]);
		//printf("A %04x B %02x\n",MMUA,MMUB);
	}
}

void hng64_state::hng_comm_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(hng64_state::hng64_comm_space_r), FUNC(hng64_state::hng64_comm_space_w));
}

void hng64_state::hng_comm_io_map(address_map &map)
{
	map.global_mask(0xff);
	/* Reserved for the KL5C80 internal hardware */
	map(0x00, 0x07).rw(FUNC(hng64_state::hng64_comm_mmu_r), FUNC(hng64_state::hng64_comm_mmu_w));
//  map(0x08, 0x1f).noprw();              /* Reserved */
//  map(0x20, 0x25).rw(hng64_state::));   /* Timer/Counter B */           /* hng64 writes here */
//  map(0x27, 0x27).noprw();              /* Reserved */
//  map(0x28, 0x2b).rw(hng64_state::)); /* Timer/Counter A */           /* hng64 writes here */
//  map(0x2c, 0x2f).rw(hng64_state::)); /* Parallel port A */
//  map(0x30, 0x33).rw(hng64_state::)); /* Parallel port B */
//  map(0x34, 0x37).rw(hng64_state::)); /* Interrupt controller */      /* hng64 writes here */
//  map(0x38, 0x39).rw(hng64_state::)); /* Serial port */               /* hng64 writes here */
//  map(0x3a, 0x3b).rw(hng64_state::)); /* System control register */   /* hng64 writes here */
//  map(0x3c, 0x3f).noprw();              /* Reserved */

	/* General IO */
	map(0x50, 0x57).rw(FUNC(hng64_state::hng64_com_share_r), FUNC(hng64_state::hng64_com_share_w));
//  map(0x72, 0x72).w(hng64_state::));            /* dunno yet */
}


void hng64_state::reset_net()
{
//  m_comm->pulse_input_line(INPUT_LINE_NMI, attotime::zero); // reset the CPU and let 'er rip
//  m_comm->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);     // hold on there pardner...

	m_mmub[0] = 0;
	m_mmub[5] = 0; // rolls back to 0xffff
}

void hng64_state::hng64_network(machine_config &config)
{
	KL5C80A12(config, m_comm, HNG64_MASTER_CLOCK / 4);        /* KL5C80A12CFP - binary compatible with Z80. */
	m_comm->set_addrmap(AS_PROGRAM, &hng64_state::hng_comm_map);
	m_comm->set_addrmap(AS_IO, &hng64_state::hng_comm_io_map);
}
