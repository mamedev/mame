// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Pierpaolo Prazzoli,Grazvydas Ignotas
/****************************************** SVP related *****************************************/

/*
 * Emulator of memory controller in SVP chip
 *
 * Copyright 2008, Grazvydas Ignotas
 * based on RE work by Tasco Deluxe
 *
 * SSP1601 EXT registers are mapped as I/O ports due to their function
 * (they are interfaced through external bus), and are named as follows
 * (these are unofficial names, official ones are unknown):
 *   EXT0: PM0 - programmable register 0
 *   EXT1: PM1 - ... 1
 *   EXT2: PM2 - ... 2
 *   EXT3: XST - external status. Can also act as PM.
 *   EXT4: PM4 - ... 4
 *   EXT5: (unused)
 *   EXT6: PMC - programmable memory register control (PMAC).
 *   EXT7: AL  - although internal to SSP1601, it still causes bus access
 *
 * Depending on GPO bits in status register, PM0, PM1, PM2 and XST can act as
 * external status registers, os as programmable memory registers. PM4 always
 * acts as PM register (independent on GPO bits).
 */


#include "emu.h"
#include "svp.h"


//-------------------------------------------------
//  md_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(MD_ROM_SVP, md_rom_svp_device, "md_rom_svp", "MD Virtua Racing")

md_rom_svp_device::md_rom_svp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_md_cart_interface(mconfig, *this)
	, m_svp(*this, "svp")
	, m_test_ipt(*this, "MEMORY_TEST")
	, m_emu_status(0), m_xst(0), m_xst2(0)
{
}

md_rom_svp_device::md_rom_svp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_rom_svp_device(mconfig, MD_ROM_SVP, tag, owner, clock)
{
}


#define SSP_PMC_HAVE_ADDR  1  // address written to PMAC, waiting for mode
#define SSP_PMC_SET        2  // PMAC is set, PMx can be programmed

#define MASTER_CLOCK_NTSC 53693175

// HELPERS

static inline int get_inc(int mode)
{
	int inc = (mode >> 11) & 7;
	if (inc != 0) {
		if (inc != 7) inc--;
		inc = 1 << inc; // 0 1 2 4 8 16 32 128
		if (mode & 0x8000) inc = -inc; // decrement mode
	}
	return inc;
}

static inline void overwrite_write(uint16_t *dst, uint16_t d)
{
	if (d & 0xf000) { *dst &= ~0xf000; *dst |= d & 0xf000; }
	if (d & 0x0f00) { *dst &= ~0x0f00; *dst |= d & 0x0f00; }
	if (d & 0x00f0) { *dst &= ~0x00f0; *dst |= d & 0x00f0; }
	if (d & 0x000f) { *dst &= ~0x000f; *dst |= d & 0x000f; }
}



uint32_t md_rom_svp_device::pm_io(int reg, int write, uint32_t d)
{
	if (m_emu_status & SSP_PMC_SET)
	{
		if (write)
			m_pmac_write[reg] = m_pmc.d;
		else
			m_pmac_read[reg] = m_pmc.d;

		m_emu_status &= ~SSP_PMC_SET;
		return 0;
	}

	// just in case
	if (m_emu_status & SSP_PMC_HAVE_ADDR)
		m_emu_status &= ~SSP_PMC_HAVE_ADDR;

	if (reg == 4 || (m_svp->state_int(SSP_ST) & 0x60))
	{
#define CADDR ((((mode<<16)&0x7f0000)|addr)<<1)
		uint16_t *dram = (uint16_t *)m_dram;
		if (write)
		{
			int mode = m_pmac_write[reg] >> 16;
			int addr = m_pmac_write[reg] & 0xffff;

			if ((mode & 0x43ff) == 0x0018) // DRAM
			{
				int inc = get_inc(mode);
				if (mode & 0x0400)
					overwrite_write(&dram[addr], d);
				else
					dram[addr] = d;
				m_pmac_write[reg] += inc;
			}
			else if ((mode & 0xfbff) == 0x4018) // DRAM, cell inc
			{
				if (mode & 0x0400)
					overwrite_write(&dram[addr], d);
				else
					dram[addr] = d;
				m_pmac_write[reg] += (addr & 1) ? 31 : 1;
			}
			else if ((mode & 0x47ff) == 0x001c) // IRAM
			{
				int inc = get_inc(mode);
				((uint16_t *)m_iram)[addr & 0x3ff] = d;
				m_pmac_write[reg] += inc;
			}
			else
			{
				logerror("ssp FIXME: PM%i unhandled write mode %04x, [%06x] %04x\n",
							reg, mode, CADDR, d);
			}
		}
		else
		{
			int mode = m_pmac_read[reg] >> 16;
			int addr = m_pmac_read[reg] & 0xffff;
			if ((mode & 0xfff0) == 0x0800) // ROM, inc 1, verified to be correct
			{
				uint16_t *ROM = (uint16_t *)get_rom_base();
				m_pmac_read[reg] += 1;
				d = ROM[addr | ((mode & 0xf) << 16)];
			}
			else if ((mode & 0x47ff) == 0x0018) // DRAM
			{
				int inc = get_inc(mode);
				d = dram[addr];
				m_pmac_read[reg] += inc;
			}
			else
			{
				logerror("ssp FIXME: PM%i unhandled read  mode %04x, [%06x]\n",
							reg, mode, CADDR);
				d = 0;
			}
		}

		// PMC value corresponds to last PMR accessed (not sure).
		if (write)
			m_pmc.d = m_pmac_write[reg];
		else
			m_pmc.d = m_pmac_read[reg];

		return d;
	}

	return (uint32_t)-1;
}

READ16_MEMBER( md_rom_svp_device::read_pm0 )
{
	uint32_t d = pm_io(0, 0, 0);
	if (d != (uint32_t)-1)
		return d;
	d = m_xst2;
	m_xst2 &= ~2; // ?
	return d;
}

WRITE16_MEMBER( md_rom_svp_device::write_pm0 )
{
	uint32_t r = pm_io(0, 1, data);
	if (r != (uint32_t)-1)
		return;
	m_xst2 = data; // ?
}

READ16_MEMBER( md_rom_svp_device::read_pm1 )
{
	uint32_t r = pm_io(1, 0, 0);
	if (r != (uint32_t)-1)
		return r;
	logerror("svp: PM1 acces in non PM mode?\n");
	return 0;
}

WRITE16_MEMBER( md_rom_svp_device::write_pm1 )
{
	uint32_t r = pm_io(1, 1, data);
	if (r != (uint32_t)-1)
		return;
	logerror("svp: PM1 acces in non PM mode?\n");
}

READ16_MEMBER( md_rom_svp_device::read_pm2 )
{
	uint32_t r = pm_io(2, 0, 0);
	if (r != (uint32_t)-1)
		return r;
	logerror("svp: PM2 acces in non PM mode?\n");
	return 0;
}

WRITE16_MEMBER( md_rom_svp_device::write_pm2 )
{
	uint32_t r = pm_io(2, 1, data);
	if (r != (uint32_t)-1)
		return;
	logerror("svp: PM2 acces in non PM mode?\n");
}

READ16_MEMBER( md_rom_svp_device::read_xst )
{
	uint32_t d = pm_io(3, 0, 0);
	if (d != (uint32_t)-1)
		return d;
	return m_xst;
}

WRITE16_MEMBER( md_rom_svp_device::write_xst )
{
	uint32_t r = pm_io(3, 1, data);
	if (r != (uint32_t)-1)
		return;
	m_xst2 |= 1;
	m_xst = data;
}

READ16_MEMBER( md_rom_svp_device::read_pm4 )
{
	return pm_io(4, 0, 0);
}

WRITE16_MEMBER( md_rom_svp_device::write_pm4 )
{
	pm_io(4, 1, data);
}

READ16_MEMBER( md_rom_svp_device::read_pmc )
{
	if (m_emu_status & SSP_PMC_HAVE_ADDR)
	{
		m_emu_status |= SSP_PMC_SET;
		m_emu_status &= ~SSP_PMC_HAVE_ADDR;
		return ((m_pmc.w.l << 4) & 0xfff0) | ((m_pmc.w.l >> 4) & 0xf);
	}
	else
	{
		m_emu_status |= SSP_PMC_HAVE_ADDR;
		return m_pmc.w.l;
	}
}

WRITE16_MEMBER( md_rom_svp_device::write_pmc )
{
	if (m_emu_status & SSP_PMC_HAVE_ADDR)
	{
		m_emu_status |= SSP_PMC_SET;
		m_emu_status &= ~SSP_PMC_HAVE_ADDR;
		m_pmc.w.h = data;
	}
	else
	{
		m_emu_status |= SSP_PMC_HAVE_ADDR;
		m_pmc.w.l = data;
	}
}

READ16_MEMBER( md_rom_svp_device::read_al )
{
	m_emu_status &= ~(SSP_PMC_SET | SSP_PMC_HAVE_ADDR);
	return 0;
}

WRITE16_MEMBER( md_rom_svp_device::write_al )
{
}


READ16_MEMBER( md_rom_svp_device::rom_read1 )
{
	uint16_t *IRAM = (uint16_t *)m_iram;
	return IRAM[offset];
}

READ16_MEMBER( md_rom_svp_device::rom_read2 )
{
	return m_rom[offset + 0x800/2];
}

int md_rom_svp_device::read_test()
{
	return m_test_ipt->read();
}


static INPUT_PORTS_START( md_svp )
	PORT_START("MEMORY_TEST") /* special memtest mode */
	PORT_CONFNAME( 0x01, 0x00, "SVP Test" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, DEF_STR( On ) )
INPUT_PORTS_END

//-------------------------------------------------
//  ADDRESS_MAP( svp_ssp_map )
//-------------------------------------------------

void md_rom_svp_device::md_svp_ssp_map(address_map &map)
{
//  map(0x0000, 0x03ff).r(FUNC(md_rom_svp_device::rom_read1));
//  map(0x0400, 0xffff).r(FUNC(md_rom_svp_device::rom_read2));
	map(0x0000, 0x03ff).bankr("iram_svp");
	map(0x0400, 0xffff).bankr("cart_svp");
}

//-------------------------------------------------
//  ADDRESS_MAP( svp_ext_map )
//-------------------------------------------------

void md_rom_svp_device::md_svp_ext_map(address_map &map)
{
	map.global_mask(0xf);
	map(0*2, 0*2+1).rw(FUNC(md_rom_svp_device::read_pm0), FUNC(md_rom_svp_device::write_pm0));
	map(1*2, 1*2+1).rw(FUNC(md_rom_svp_device::read_pm1), FUNC(md_rom_svp_device::write_pm1));
	map(2*2, 2*2+1).rw(FUNC(md_rom_svp_device::read_pm2), FUNC(md_rom_svp_device::write_pm2));
	map(3*2, 3*2+1).rw(FUNC(md_rom_svp_device::read_xst), FUNC(md_rom_svp_device::write_xst));
	map(4*2, 4*2+1).rw(FUNC(md_rom_svp_device::read_pm4), FUNC(md_rom_svp_device::write_pm4));
	map(6*2, 6*2+1).rw(FUNC(md_rom_svp_device::read_pmc), FUNC(md_rom_svp_device::write_pmc));
	map(7*2, 7*2+1).rw(FUNC(md_rom_svp_device::read_al), FUNC(md_rom_svp_device::write_al));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void md_rom_svp_device::device_add_mconfig(machine_config &config)
{
	SSP1601(config, m_svp, MASTER_CLOCK_NTSC / 7 * 3); /* ~23 MHz (guessed) */
	m_svp->set_addrmap(AS_PROGRAM, &md_rom_svp_device::md_svp_ssp_map);
	m_svp->set_addrmap(AS_IO, &md_rom_svp_device::md_svp_ext_map);
}

ioport_constructor md_rom_svp_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( md_svp );
}


void md_rom_svp_device::set_bank_to_rom(const char *banktag, uint32_t offset)
{
	if (membank(banktag))
		membank(banktag)->set_base(m_rom + offset);
}


void md_rom_svp_device::device_start()
{
	memset(m_pmac_read, 0, ARRAY_LENGTH(m_pmac_read));
	memset(m_pmac_write, 0, ARRAY_LENGTH(m_pmac_write));
	m_pmc.d = 0;
	m_pmc.w.l = 0;
	m_pmc.w.h = 0;
	m_emu_status = 0;
	m_xst = 0;
	m_xst2 = 0;

	// SVP stuff
	this->membank("iram_svp")->set_base(m_iram);
	// the other bank, "cart_svp", is setup at call_load

	save_item(NAME(m_pmac_read));
	save_item(NAME(m_pmac_write));
	save_item(NAME(m_emu_status));
	save_item(NAME(m_xst));
	save_item(NAME(m_xst2));
	save_item(NAME(m_pmc.d));
	save_item(NAME(m_pmc.w.l));
	save_item(NAME(m_pmc.w.h));
	save_item(NAME(m_dram));
	save_item(NAME(m_iram));
}

READ16_MEMBER(md_rom_svp_device::read)
{
	uint16_t *DRAM = (uint16_t *)m_dram;

	if (offset >= 0x300000/2 && offset < 0x320000/2)
	{
		return DRAM[offset - 0x300000/2];
	}
	else if (offset >= 0x390000/2 && offset < 0x3a0000/2)
	{
		// this is rewritten 68k test code
		uint32_t a1 = offset - 0x390000/2;
		a1 = (a1 & 0x7001) | ((a1 & 0x3e) << 6) | ((a1 & 0xfc0) >> 5);
		return DRAM[a1];
	}
	else if (offset >= 0x3a0000/2 && offset < 0x3b0000/2)
	{
		// this is rewritten 68k test code
		uint32_t a1 = offset - 0x3a0000/2;
		a1 = (a1 & 0x7801) | ((a1 & 0x1e) << 6) | ((a1 & 0x7e0) >> 4);
		return DRAM[a1];
	}
	if (offset < 0x200000/2)
		return m_rom[offset];
	else
	{
		printf("read out of bound\n");
		return 0xffff;
	}
}

WRITE16_MEMBER(md_rom_svp_device::write)
{
	if (offset >= 0x300000/2 && offset < 0x320000/2)
	{
		uint32_t a1 = offset - 0x300000/2;
		uint16_t *DRAM = (uint16_t *)m_dram;
		DRAM[a1] = data;
	}
}

READ16_MEMBER(md_rom_svp_device::read_a15)
{
	uint32_t d;
	switch (offset)
	{
		// 0xa15000, 0xa15002
		case 0:
		case 1:  return m_xst;
		// 0xa15004
		case 2:  d = m_xst2; m_xst2 &= ~1; return d;
		default: logerror("unhandled SVP reg read @ %x\n", offset << 1);
	}
	return 0;
}

WRITE16_MEMBER(md_rom_svp_device::write_a15)
{
	switch (offset)
	{
		// 0xa15000, 0xa15002
		case 0:
		case 1:  m_xst = data; m_xst2 |= 2; break;
		// 0xa15006
		case 3:  break; // possibly halts SSP1601
		default: logerror("unhandled SVP reg write %04x @ %x\n", data, offset << 1);
	}
}
