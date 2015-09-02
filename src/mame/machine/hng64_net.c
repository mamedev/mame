// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner
/* HNG64 Communication / Network CPU */

// this is driven by a KL5C80A12CFP which is basically a super-charged Z80
// most of this MMU handling etc. should be moved to the core.

// I believe this CPU is used for network only (the racing games) and not related to the I/O MCU

/*
0x6010: tests RAM at [3]8000

*/

#include "includes/hng64.h"
#include "cpu/z80/kl5c80a12.h"

UINT8 hng64_state::read_comm_data(UINT32 offset)
{
	if((offset & 0x10000) == 0)
		return m_comm_rom[offset & 0xffff];

	if(offset & 0x10000)
		return m_comm_ram[(offset & 0xffff)];

	printf("%08x\n",offset);
	return 0xff;
}

void hng64_state::write_comm_data(UINT32 offset,UINT8 data)
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

static ADDRESS_MAP_START( hng_comm_map, AS_PROGRAM, 8, hng64_state )
	AM_RANGE(0x0000,0xffff) AM_READWRITE(hng64_comm_space_r, hng64_comm_space_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( hng_comm_io_map, AS_IO, 8, hng64_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	/* Reserved for the KL5C80 internal hardware */
	AM_RANGE(0x00, 0x07) AM_READWRITE(hng64_comm_mmu_r,hng64_comm_mmu_w )
//  AM_RANGE(0x08,0x1f) AM_NOP              /* Reserved */
//  AM_RANGE(0x20,0x25) AM_READWRITE        /* Timer/Counter B */           /* hng64 writes here */
//  AM_RANGE(0x27,0x27) AM_NOP              /* Reserved */
//  AM_RANGE(0x28,0x2b) AM_READWRITE        /* Timer/Counter A */           /* hng64 writes here */
//  AM_RANGE(0x2c,0x2f) AM_READWRITE        /* Parallel port A */
//  AM_RANGE(0x30,0x33) AM_READWRITE        /* Parallel port B */
//  AM_RANGE(0x34,0x37) AM_READWRITE        /* Interrupt controller */      /* hng64 writes here */
//  AM_RANGE(0x38,0x39) AM_READWRITE        /* Serial port */               /* hng64 writes here */
//  AM_RANGE(0x3a,0x3b) AM_READWRITE        /* System control register */   /* hng64 writes here */
//  AM_RANGE(0x3c,0x3f) AM_NOP              /* Reserved */

	/* General IO */
	AM_RANGE(0x50,0x57) AM_READWRITE(hng64_com_share_r, hng64_com_share_w)
//  AM_RANGE(0x72,0x72) AM_WRITE            /* dunno yet */
ADDRESS_MAP_END


void hng64_state::reset_net()
{
//  m_comm->set_input_line(INPUT_LINE_RESET, PULSE_LINE);     // reset the CPU and let 'er rip
//  m_comm->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);     // hold on there pardner...

	m_mmub[0] = 0;
	m_mmub[5] = 0; // rolls back to 0xffff
}

MACHINE_CONFIG_FRAGMENT( hng64_network )
	MCFG_CPU_ADD("network", KL5C80A12, HNG64_MASTER_CLOCK / 4)        /* KL5C80A12CFP - binary compatible with Z80. */
	MCFG_CPU_PROGRAM_MAP(hng_comm_map)
	MCFG_CPU_IO_MAP(hng_comm_io_map)
MACHINE_CONFIG_END
