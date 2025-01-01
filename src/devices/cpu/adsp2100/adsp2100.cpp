// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    adsp2100.cpp

    ADSP-21xx series emulator.

****************************************************************************

    For ADSP-2101, ADSP-2111
    ------------------------

        MMAP = 0                                        MMAP = 1

        Automatic boot loading                          No auto boot loading

        Program Space:                                  Program Space:
            0000-07ff = 2k Internal RAM (booted)            0000-37ff = 14k External access
            0800-3fff = 14k External access                 3800-3fff = 2k Internal RAM

        Data Space:                                     Data Space:
            0000-03ff = 1k External DWAIT0                  0000-03ff = 1k External DWAIT0
            0400-07ff = 1k External DWAIT1                  0400-07ff = 1k External DWAIT1
            0800-2fff = 10k External DWAIT2                 0800-2fff = 10k External DWAIT2
            3000-33ff = 1k External DWAIT3                  3000-33ff = 1k External DWAIT3
            3400-37ff = 1k External DWAIT4                  3400-37ff = 1k External DWAIT4
            3800-3bff = 1k Internal RAM                     3800-3bff = 1k Internal RAM
            3c00-3fff = 1k Internal Control regs            3c00-3fff = 1k Internal Control regs


    For ADSP-2105, ADSP-2115
    ------------------------

        MMAP = 0                                        MMAP = 1

        Automatic boot loading                          No auto boot loading

        Program Space:                                  Program Space:
            0000-03ff = 1k Internal RAM (booted)            0000-37ff = 14k External access
            0400-07ff = 1k Reserved                         3800-3bff = 1k Internal RAM
            0800-3fff = 14k External access                 3c00-3fff = 1k Reserved

        Data Space:                                     Data Space:
            0000-03ff = 1k External DWAIT0                  0000-03ff = 1k External DWAIT0
            0400-07ff = 1k External DWAIT1                  0400-07ff = 1k External DWAIT1
            0800-2fff = 10k External DWAIT2                 0800-2fff = 10k External DWAIT2
            3000-33ff = 1k External DWAIT3                  3000-33ff = 1k External DWAIT3
            3400-37ff = 1k External DWAIT4                  3400-37ff = 1k External DWAIT4
            3800-39ff = 512 Internal RAM                    3800-39ff = 512 Internal RAM
            3a00-3bff = 512 Reserved                        3a00-3bff = 512 Reserved
            3c00-3fff = 1k Internal Control regs            3c00-3fff = 1k Internal Control regs


    For ADSP-2104
    -------------

        MMAP = 0                                        MMAP = 1

        Automatic boot loading                          No auto boot loading

        Program Space:                                  Program Space:
            0000-01ff = 512 Internal RAM (booted)           0000-37ff = 14k External access
            0200-07ff = 1.5k Reserved                       3800-39ff = 512 Internal RAM
            0800-3fff = 14k External access                 3a00-3fff = 1.5k Reserved

        Data Space:                                     Data Space:
            0000-03ff = 1k External DWAIT0                  0000-03ff = 1k External DWAIT0
            0400-07ff = 1k External DWAIT1                  0400-07ff = 1k External DWAIT1
            0800-2fff = 10k External DWAIT2                 0800-2fff = 10k External DWAIT2
            3000-33ff = 1k External DWAIT3                  3000-33ff = 1k External DWAIT3
            3400-37ff = 1k External DWAIT4                  3400-37ff = 1k External DWAIT4
            3800-38ff = 256 Internal RAM                    3800-38ff = 256 Internal RAM
            3900-3bff = 768 Reserved                        3900-3bff = 768 Reserved
            3c00-3fff = 1k Internal Control regs            3c00-3fff = 1k Internal Control regs


    For ADSP-2181
    -------------

        MMAP = 0                                        MMAP = 1

        Auto boot loading via BDMA or IDMA              No auto boot loading

        Program Space:                                  Program Space:
            0000-1fff = 8k Internal RAM                     0000-1fff = 8k External access
            2000-3fff = 8k Internal RAM (PMOVLAY = 0)       2000-3fff = 8k Internal (PMOVLAY = 0)
            2000-3fff = 8k External (PMOVLAY = 1,2)

        Data Space:                                     Data Space:
            0000-1fff = 8k Internal RAM (DMOVLAY = 0)       0000-1fff = 8k Internal RAM (DMOVLAY = 0)
            0000-1fff = 8k External (DMOVLAY = 1,2)         0000-1fff = 8k External (DMOVLAY = 1,2)
            2000-3fdf = 8k-32 Internal RAM                  2000-3fdf = 8k-32 Internal RAM
            3fe0-3fff = 32 Internal Control regs            3fe0-3fff = 32 Internal Control regs

        I/O Space:                                      I/O Space:
            0000-01ff = 512 External IOWAIT0                0000-01ff = 512 External IOWAIT0
            0200-03ff = 512 External IOWAIT1                0200-03ff = 512 External IOWAIT1
            0400-05ff = 512 External IOWAIT2                0400-05ff = 512 External IOWAIT2
            0600-07ff = 512 External IOWAIT3                0600-07ff = 512 External IOWAIT3

    TODO:
    - Move internal stuffs into CPU core file (on-chip RAM, control registers, etc)
    - Support variable internal memory mappings

***************************************************************************/

#include "emu.h"
#include "adsp2100.h"
#include "2100dasm.h"


// device type definitions
DEFINE_DEVICE_TYPE(ADSP2100, adsp2100_device, "adsp2100", "Analog Devices ADSP-2100")
DEFINE_DEVICE_TYPE(ADSP2101, adsp2101_device, "adsp2101", "Analog Devices ADSP-2101")
DEFINE_DEVICE_TYPE(ADSP2104, adsp2104_device, "adsp2104", "Analog Devices ADSP-2104")
DEFINE_DEVICE_TYPE(ADSP2105, adsp2105_device, "adsp2105", "Analog Devices ADSP-2105")
DEFINE_DEVICE_TYPE(ADSP2115, adsp2115_device, "adsp2115", "Analog Devices ADSP-2115")
DEFINE_DEVICE_TYPE(ADSP2181, adsp2181_device, "adsp2181", "Analog Devices ADSP-2181")


//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  adsp21xx_device - constructor
//-------------------------------------------------

adsp21xx_device::adsp21xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t chiptype)
	: cpu_device(mconfig, type, tag, owner, clock),
		m_program_config("program", ENDIANNESS_LITTLE, 32, 14, -2),
		m_data_config("data", ENDIANNESS_LITTLE, 16, 14, -1),
		m_chip_type(chiptype),
		m_pc(0),
		m_ppc(0),
		m_loop(0),
		m_loop_condition(0),
		m_cntr(0),
		m_astat(0),
		m_sstat(0),
		m_mstat(0),
		m_mstat_prev(0),
		m_astat_clear(0),
		m_idle(0),
		m_px(0),
		m_pmovlay(0),
		m_dmovlay(0),
		m_pc_sp(0),
		m_cntr_sp(0),
		m_stat_sp(0),
		m_loop_sp(0),
		m_flagout(0),
		m_flagin(0),
		m_fl0(0),
		m_fl1(0),
		m_fl2(0),
		m_idma_addr(0),
		m_idma_cache(0),
		m_idma_offs(0),
		m_imask(0),
		m_icntl(0),
		m_ifc(0),
		m_icount(0),
		m_mstat_mask((m_chip_type >= CHIP_TYPE_ADSP2101) ? 0x7f : 0x0f),
		m_imask_mask((m_chip_type >= CHIP_TYPE_ADSP2181) ? 0x3ff :
					(m_chip_type >= CHIP_TYPE_ADSP2101) ? 0x3f : 0x0f),
		m_sport_rx_cb(*this, 0),
		m_sport_tx_cb(*this),
		m_timer_fired_cb(*this),
		m_dmovlay_cb(*this)
{
	// initialize remaining state
	memset(&m_core, 0, sizeof(m_core));
	memset(&m_alt, 0, sizeof(m_alt));
	memset(&m_i, 0, sizeof(m_i));
	memset(&m_m, 0, sizeof(m_m));
	memset(&m_l, 0, sizeof(m_l));
	memset(&m_lmask, 0, sizeof(m_lmask));
	memset(&m_base, 0, sizeof(m_base));
	memset(&m_loop_stack, 0, sizeof(m_loop_stack));
	memset(&m_cntr_stack, 0, sizeof(m_cntr_stack));
	memset(&m_pc_stack, 0, sizeof(m_pc_stack));
	memset(&m_stat_stack, 0, sizeof(m_stat_stack));
	memset(&m_irq_state, 0, sizeof(m_irq_state));
	memset(&m_irq_latch, 0, sizeof(m_irq_latch));

	// create the tables
	create_tables();

	// set up read register group 0 pointers
	m_read0_ptr[0x00] = &m_core.ax0.s;
	m_read0_ptr[0x01] = &m_core.ax1.s;
	m_read0_ptr[0x02] = &m_core.mx0.s;
	m_read0_ptr[0x03] = &m_core.mx1.s;
	m_read0_ptr[0x04] = &m_core.ay0.s;
	m_read0_ptr[0x05] = &m_core.ay1.s;
	m_read0_ptr[0x06] = &m_core.my0.s;
	m_read0_ptr[0x07] = &m_core.my1.s;
	m_read0_ptr[0x08] = &m_core.si.s;
	m_read0_ptr[0x09] = &m_core.se.s;
	m_read0_ptr[0x0a] = &m_core.ar.s;
	m_read0_ptr[0x0b] = &m_core.mr.mrx.mr0.s;
	m_read0_ptr[0x0c] = &m_core.mr.mrx.mr1.s;
	m_read0_ptr[0x0d] = &m_core.mr.mrx.mr2.s;
	m_read0_ptr[0x0e] = &m_core.sr.srx.sr0.s;
	m_read0_ptr[0x0f] = &m_core.sr.srx.sr1.s;

	// set up read register group 1 + 2 pointers
	for (int index = 0; index < 4; index++)
	{
		m_read1_ptr[0x00 + index] = &m_i[0 + index];
		m_read1_ptr[0x04 + index] = (uint32_t *)&m_m[0 + index];
		m_read1_ptr[0x08 + index] = &m_l[0 + index];
		m_read1_ptr[0x0c + index] = &m_l[0 + index];
		m_read2_ptr[0x00 + index] = &m_i[4 + index];
		m_read2_ptr[0x04 + index] = (uint32_t *)&m_m[4 + index];
		m_read2_ptr[0x08 + index] = &m_l[4 + index];
		m_read2_ptr[0x0c + index] = &m_l[4 + index];
	}

	// set up ALU register pointers
	m_alu_xregs[0] = &m_core.ax0;
	m_alu_xregs[1] = &m_core.ax1;
	m_alu_xregs[2] = &m_core.ar;
	m_alu_xregs[3] = &m_core.mr.mrx.mr0;
	m_alu_xregs[4] = &m_core.mr.mrx.mr1;
	m_alu_xregs[5] = &m_core.mr.mrx.mr2;
	m_alu_xregs[6] = &m_core.sr.srx.sr0;
	m_alu_xregs[7] = &m_core.sr.srx.sr1;
	m_alu_yregs[0] = &m_core.ay0;
	m_alu_yregs[1] = &m_core.ay1;
	m_alu_yregs[2] = &m_core.af;
	m_alu_yregs[3] = &m_core.zero;

	// set up MAC register pointers
	m_mac_xregs[0] = &m_core.mx0;
	m_mac_xregs[1] = &m_core.mx1;
	m_mac_xregs[2] = &m_core.ar;
	m_mac_xregs[3] = &m_core.mr.mrx.mr0;
	m_mac_xregs[4] = &m_core.mr.mrx.mr1;
	m_mac_xregs[5] = &m_core.mr.mrx.mr2;
	m_mac_xregs[6] = &m_core.sr.srx.sr0;
	m_mac_xregs[7] = &m_core.sr.srx.sr1;
	m_mac_yregs[0] = &m_core.my0;
	m_mac_yregs[1] = &m_core.my1;
	m_mac_yregs[2] = &m_core.mf;
	m_mac_yregs[3] = &m_core.zero;

	// set up shift register pointers
	m_shift_xregs[0] = &m_core.si;
	m_shift_xregs[1] = &m_core.si;
	m_shift_xregs[2] = &m_core.ar;
	m_shift_xregs[3] = &m_core.mr.mrx.mr0;
	m_shift_xregs[4] = &m_core.mr.mrx.mr1;
	m_shift_xregs[5] = &m_core.mr.mrx.mr2;
	m_shift_xregs[6] = &m_core.sr.srx.sr0;
	m_shift_xregs[7] = &m_core.sr.srx.sr1;
}

adsp2100_device::adsp2100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: adsp21xx_device(mconfig, ADSP2100, tag, owner, clock, CHIP_TYPE_ADSP2100)
{ }

adsp2101_device::adsp2101_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: adsp2101_device(mconfig, ADSP2101, tag, owner, clock, CHIP_TYPE_ADSP2101)
{ }

adsp2101_device::adsp2101_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t chiptype)
	: adsp21xx_device(mconfig, type, tag, owner, clock, chiptype)
{ }

adsp2104_device::adsp2104_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: adsp2101_device(mconfig, ADSP2104, tag, owner, clock, CHIP_TYPE_ADSP2104)
{ }

adsp2105_device::adsp2105_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: adsp2101_device(mconfig, ADSP2105, tag, owner, clock, CHIP_TYPE_ADSP2105)
{ }

adsp2115_device::adsp2115_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: adsp2101_device(mconfig, ADSP2115, tag, owner, clock, CHIP_TYPE_ADSP2115)
{ }

adsp2181_device::adsp2181_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: adsp21xx_device(mconfig, ADSP2181, tag, owner, clock, CHIP_TYPE_ADSP2181)
	, m_io_config("io", ENDIANNESS_LITTLE, 16, 11, -1)
{ }


//-------------------------------------------------
//  ~adsp21xx_device - destructor
//-------------------------------------------------

adsp21xx_device::~adsp21xx_device()
{
#if ADSP_TRACK_HOTSPOTS
	FILE *log = fopen("adsp.hot", "w");
	while (1)
	{
		int maxindex = 0, i;
		for (i = 1; i < 0x4000; i++)
			if (m_pcbucket[i] > m_pcbucket[maxindex])
				maxindex = i;
		if (m_pcbucket[maxindex] == 0)
			break;
		fprintf(log, "PC=%04X  (%10d hits)\n", maxindex, m_pcbucket[maxindex]);
		m_pcbucket[maxindex] = 0;
	}
	fclose(log);
#endif
}


//-------------------------------------------------
//  load_boot_data - load the boot data from an
//  8-bit ROM
//-------------------------------------------------

void adsp21xx_device::load_boot_data(uint8_t *srcdata, uint32_t *dstdata)
{
	// see how many words we need to copy
	int pagelen = (srcdata[3] + 1) * 8;
	for (int i = 0; i < pagelen; i++)
	{
		uint32_t opcode = (srcdata[i*4+0] << 16) | (srcdata[i*4+1] << 8) | srcdata[i*4+2];
		dstdata[i] = opcode;
	}
}


//-------------------------------------------------
//  idma_addr_w - write the IDMA address register
//-------------------------------------------------

void adsp2181_device::idma_addr_w(uint16_t data)
{
	m_idma_addr = data;
	m_idma_offs = 0;
}


//-------------------------------------------------
//  idma_addr_r - read the IDMA address register
//-------------------------------------------------

uint16_t adsp2181_device::idma_addr_r()
{
	return m_idma_addr;
}


//-------------------------------------------------
//  idma_data_w - write the IDMA data register
//-------------------------------------------------

void adsp2181_device::idma_data_w(uint16_t data)
{
	// program memory?
	if (!(m_idma_addr & 0x4000))
	{
		// upper 16 bits
		if (m_idma_offs == 0)
		{
			m_idma_cache = data;
			m_idma_offs = 1;
		}

		// lower 8 bits
		else
		{
			program_write(m_idma_addr++ & 0x3fff, (m_idma_cache << 8) | (data & 0xff));
			m_idma_offs = 0;
		}
	}

	// data memory
	else
		data_write(m_idma_addr++ & 0x3fff, data);
}


//-------------------------------------------------
//  idma_data_r - read the IDMA data register
//-------------------------------------------------

uint16_t adsp2181_device::idma_data_r()
{
	uint16_t result;

	// program memory?
	if (!(m_idma_addr & 0x4000))
	{
		// upper 16 bits
		if (m_idma_offs == 0)
		{
			result = program_read(m_idma_addr & 0x3fff) >> 8;
			m_idma_offs = 1;
		}

		// lower 8 bits
		else
		{
			result = program_read(m_idma_addr++ & 0x3fff) & 0xff;
			m_idma_offs = 0;
		}
	}

	// data memory
	else
		result = data_read(m_idma_addr++ & 0x3fff);

	return result;
}


//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void adsp21xx_device::device_start()
{
	// get our address spaces
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_DATA).specific(m_data);
	if(has_space(AS_IO))
		space(AS_IO).specific(m_io);

	// "core"
	save_item(NAME(m_core.ax0.u));
	save_item(NAME(m_core.ax1.u));
	save_item(NAME(m_core.ay0.u));
	save_item(NAME(m_core.ay1.u));
	save_item(NAME(m_core.ar.u));
	save_item(NAME(m_core.af.u));
	save_item(NAME(m_core.mx0.u));
	save_item(NAME(m_core.mx1.u));
	save_item(NAME(m_core.my0.u));
	save_item(NAME(m_core.my1.u));
	save_item(NAME(m_core.mr.mr));
	save_item(NAME(m_core.mf.u));
	save_item(NAME(m_core.si.u));
	save_item(NAME(m_core.se.u));
	save_item(NAME(m_core.sb.u));
	save_item(NAME(m_core.sr.sr));
	save_item(NAME(m_core.zero.u));

	// "alt"
	save_item(NAME(m_alt.ax0.u));
	save_item(NAME(m_alt.ax1.u));
	save_item(NAME(m_alt.ay0.u));
	save_item(NAME(m_alt.ay1.u));
	save_item(NAME(m_alt.ar.u));
	save_item(NAME(m_alt.af.u));
	save_item(NAME(m_alt.mx0.u));
	save_item(NAME(m_alt.mx1.u));
	save_item(NAME(m_alt.my0.u));
	save_item(NAME(m_alt.my1.u));
	save_item(NAME(m_alt.mr.mr));
	save_item(NAME(m_alt.mf.u));
	save_item(NAME(m_alt.si.u));
	save_item(NAME(m_alt.se.u));
	save_item(NAME(m_alt.sb.u));
	save_item(NAME(m_alt.sr.sr));
	save_item(NAME(m_alt.zero.u));

	save_item(NAME(m_i));
	save_item(NAME(m_m));
	save_item(NAME(m_l));
	save_item(NAME(m_lmask));
	save_item(NAME(m_base));
	save_item(NAME(m_px));
	save_item(NAME(m_pmovlay));
	save_item(NAME(m_dmovlay));

	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_loop));
	save_item(NAME(m_loop_condition));
	save_item(NAME(m_cntr));
	save_item(NAME(m_astat));
	save_item(NAME(m_sstat));
	save_item(NAME(m_mstat));
	save_item(NAME(m_mstat_prev));
	save_item(NAME(m_astat_clear));
	save_item(NAME(m_idle));

	save_item(NAME(m_loop_stack));
	save_item(NAME(m_cntr_stack));
	save_item(NAME(m_pc_stack));
	save_item(NAME(m_stat_stack));

	save_item(NAME(m_pc_sp));
	save_item(NAME(m_cntr_sp));
	save_item(NAME(m_stat_sp));
	save_item(NAME(m_loop_sp));

	save_item(NAME(m_flagout));
	save_item(NAME(m_flagin));
	save_item(NAME(m_fl0));
	save_item(NAME(m_fl1));
	save_item(NAME(m_fl2));
	save_item(NAME(m_idma_addr));
	save_item(NAME(m_idma_cache));
	save_item(NAME(m_idma_offs));

	save_item(NAME(m_imask));
	save_item(NAME(m_icntl));
	save_item(NAME(m_ifc));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_irq_latch));

	// register state with the debugger
	state_add(ADSP2100_PC,      "PC",        m_pc).callimport();
	state_add(STATE_GENPCBASE,  "CURPC",     m_ppc).callimport().noshow();
	state_add(STATE_GENFLAGS,   "CURFLAGS",  m_astat).mask(0xff).formatstr("%8s").noshow();

	state_add(ADSP2100_AX0,     "AX0",       m_core.ax0.u);
	state_add(ADSP2100_AX1,     "AX1",       m_core.ax1.u);
	state_add(ADSP2100_AY0,     "AY0",       m_core.ay0.u);
	state_add(ADSP2100_AY1,     "AY1",       m_core.ay1.u);
	state_add(ADSP2100_AR,      "AR",        m_core.ar.u);
	state_add(ADSP2100_AF,      "AF",        m_core.af.u);

	state_add(ADSP2100_MX0,     "MX0",       m_core.mx0.u);
	state_add(ADSP2100_MX1,     "MX1",       m_core.mx1.u);
	state_add(ADSP2100_MY0,     "MY0",       m_core.my0.u);
	state_add(ADSP2100_MY1,     "MY1",       m_core.my1.u);
	state_add(ADSP2100_MR0,     "MR0",       m_core.mr.mrx.mr0.u);
	state_add(ADSP2100_MR1,     "MR1",       m_core.mr.mrx.mr1.u);
	state_add(ADSP2100_MR2,     "MR2",       m_core.mr.mrx.mr2.u).signed_mask(0xff);
	state_add(ADSP2100_MF,      "MF",        m_core.mf.u);

	state_add(ADSP2100_SI,      "SI",        m_core.si.u);
	state_add(ADSP2100_SE,      "SE",        m_core.se.u).signed_mask(0xff);
	state_add(ADSP2100_SB,      "SB",        m_core.sb.u).signed_mask(0x1f);
	state_add(ADSP2100_SR0,     "SR0",       m_core.sr.srx.sr0.u);
	state_add(ADSP2100_SR1,     "SR1",       m_core.sr.srx.sr1.u);

	state_add(ADSP2100_AX0_SEC, "AX0_SEC",   m_alt.ax0.u);
	state_add(ADSP2100_AX1_SEC, "AX1_SEC",   m_alt.ax1.u);
	state_add(ADSP2100_AY0_SEC, "AY0_SEC",   m_alt.ay0.u);
	state_add(ADSP2100_AY1_SEC, "AY1_SEC",   m_alt.ay1.u);
	state_add(ADSP2100_AR_SEC,  "AR_SEC",    m_alt.ar.u);
	state_add(ADSP2100_AF_SEC,  "AF_SEC",    m_alt.af.u);

	state_add(ADSP2100_MX0_SEC, "MX0_SEC",   m_alt.mx0.u);
	state_add(ADSP2100_MX1_SEC, "MX1_SEC",   m_alt.mx1.u);
	state_add(ADSP2100_MY0_SEC, "MY0_SEC",   m_alt.my0.u);
	state_add(ADSP2100_MY1_SEC, "MY1_SEC",   m_alt.my1.u);
	state_add(ADSP2100_MR0_SEC, "MR0_SEC",   m_alt.mr.mrx.mr0.u);
	state_add(ADSP2100_MR1_SEC, "MR1_SEC",   m_alt.mr.mrx.mr1.u);
	state_add(ADSP2100_MR2_SEC, "MR2_SEC",   m_alt.mr.mrx.mr2.u).signed_mask(0xff);
	state_add(ADSP2100_MF_SEC,  "MF_SEC",    m_alt.mf.u);

	state_add(ADSP2100_SI_SEC,  "SI_SEC",    m_alt.si.u);
	state_add(ADSP2100_SE_SEC,  "SE_SEC",    m_alt.se.u).signed_mask(0xff);
	state_add(ADSP2100_SB_SEC,  "SB_SEC",    m_alt.sb.u).signed_mask(0x1f);
	state_add(ADSP2100_SR0_SEC, "SR0_SEC",   m_alt.sr.srx.sr0.u);
	state_add(ADSP2100_SR1_SEC, "SR1_SEC",   m_alt.sr.srx.sr1.u);

	for (int ireg = 0; ireg < 8; ireg++)
		state_add(ADSP2100_I0 + ireg, string_format("I%d", ireg).c_str(), m_i[ireg]).mask(0x3fff).callimport();

	for (int lreg = 0; lreg < 8; lreg++)
		state_add(ADSP2100_L0 + lreg, string_format("L%d", lreg).c_str(), m_l[lreg]).mask(0x3fff).callimport();

	for (int mreg = 0; mreg < 8; mreg++)
		state_add(ADSP2100_M0 + mreg, string_format("M%d", mreg).c_str(), m_m[mreg]).signed_mask(0x3fff);

	state_add(ADSP2100_PX,      "PX",        m_px);
	state_add(ADSP2100_CNTR,    "CNTR",      m_cntr).mask(0x3fff);
	state_add(ADSP2100_ASTAT,   "ASTAT",     m_astat).mask(0xff);
	state_add(ADSP2100_SSTAT,   "SSTAT",     m_sstat).mask(0xff);
	state_add(ADSP2100_MSTAT,   "MSTAT",     m_mstat).mask((m_chip_type == CHIP_TYPE_ADSP2100) ? 0x0f : 0x7f).callimport();

	state_add(ADSP2100_PCSP,    "PCSP",      m_pc_sp).mask(0xff);
	state_add(ADSP2100_CNTRSP,  "CNTRSP",    m_cntr_sp).mask(0xf);
	state_add(ADSP2100_STATSP,  "STATSP",    m_stat_sp).mask(0xf);
	state_add(ADSP2100_LOOPSP,  "LOOPSP",    m_loop_sp).mask(0xf);

	state_add(ADSP2100_IMASK,   "IMASK",     m_imask).mask((m_chip_type == CHIP_TYPE_ADSP2100) ? 0x00f : (m_chip_type == CHIP_TYPE_ADSP2181) ? 0x3ff : 0x07f).callimport();
	state_add(ADSP2100_ICNTL,   "ICNTL",     m_icntl).mask(0x1f).callimport();

	for (int irqnum = 0; irqnum < 4; irqnum++)
		if (irqnum < 4 || m_chip_type == CHIP_TYPE_ADSP2100)
			state_add(ADSP2100_IRQSTATE0 + irqnum, string_format("IRQ%d", irqnum).c_str(), m_irq_state[irqnum]).mask(1).callimport();

	state_add(ADSP2100_FLAGIN,  "FLAGIN",    m_flagin).mask(1);
	state_add(ADSP2100_FLAGOUT, "FLAGOUT",   m_flagout).mask(1);
	state_add(ADSP2100_FL0,     "FL0",       m_fl0).mask(1);
	state_add(ADSP2100_FL1,     "FL1",       m_fl1).mask(1);
	state_add(ADSP2100_FL2,     "FL2",       m_fl2).mask(1);

	if (m_chip_type == CHIP_TYPE_ADSP2181)
	{
		state_add(ADSP2100_PMOVLAY, "PMOVLAY",   m_pmovlay);
		state_add(ADSP2100_DMOVLAY, "DMOVLAY",   m_dmovlay);
	}

	// set our instruction counter
	set_icountptr(m_icount);
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void adsp21xx_device::device_reset()
{
	// ensure that zero is zero
	m_core.zero.u = m_alt.zero.u = 0;

	// recompute the memory registers with their current values
	write_reg1(0x08, m_l[0]);   write_reg1(0x00, m_i[0]);
	write_reg1(0x09, m_l[1]);   write_reg1(0x01, m_i[1]);
	write_reg1(0x0a, m_l[2]);   write_reg1(0x02, m_i[2]);
	write_reg1(0x0b, m_l[3]);   write_reg1(0x03, m_i[3]);
	write_reg2(0x08, m_l[4]);   write_reg2(0x00, m_i[4]);
	write_reg2(0x09, m_l[5]);   write_reg2(0x01, m_i[5]);
	write_reg2(0x0a, m_l[6]);   write_reg2(0x02, m_i[6]);
	write_reg2(0x0b, m_l[7]);   write_reg2(0x03, m_i[7]);

	// reset overlays
	if (m_chip_type == CHIP_TYPE_ADSP2181)
	{
		m_pmovlay = m_dmovlay = 0;
		// PMOVLAY
		update_dmovlay();
	}

	// reset PC and loops
	m_pc = (m_chip_type >= CHIP_TYPE_ADSP2101) ? 0 : 4;
	m_ppc = m_pc;
	m_loop = 0xffff;
	m_loop_condition = 0;

	// reset status registers
	m_astat_clear = ~(CFLAG | VFLAG | NFLAG | ZFLAG);
	m_mstat = 0;
	m_sstat = 0x55;
	m_idle = 0;
	update_mstat();

	// reset stacks
	m_pc_sp = 0;
	m_cntr_sp = 0;
	m_stat_sp = 0;
	m_loop_sp = 0;

	// reset external I/O
	m_flagout = 0;
	m_flagin = 0;
	m_fl0 = 0;
	m_fl1 = 0;
	m_fl2 = 0;

	// reset interrupts
	m_imask = 0;
	for (int irq = 0; irq < 10; irq++)
		m_irq_state[irq] = m_irq_latch[irq] = CLEAR_LINE;
}


//-------------------------------------------------
//  device_post_load - called after loading a saved state
//-------------------------------------------------

void adsp21xx_device::device_post_load()
{
	// update overlays
	if (m_chip_type == CHIP_TYPE_ADSP2181)
	{
		// PMOVLAY
		update_dmovlay();
	}
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

device_memory_interface::space_config_vector adsp2100_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}

device_memory_interface::space_config_vector adsp2101_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}

device_memory_interface::space_config_vector adsp2181_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void adsp21xx_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case ADSP2100_PC:
			m_ppc = m_pc;
			break;

		case STATE_GENPCBASE:
			m_pc = m_ppc;
			break;

		case ADSP2100_MSTAT:
			update_mstat();
			break;

		case ADSP2100_IMASK:
		case ADSP2100_ICNTL:
		case ADSP2100_IRQSTATE0:
		case ADSP2100_IRQSTATE1:
		case ADSP2100_IRQSTATE2:
		case ADSP2100_IRQSTATE3:
			check_irqs();
			break;

		case ADSP2100_I0:
		case ADSP2100_I1:
		case ADSP2100_I2:
		case ADSP2100_I3:
		case ADSP2100_I4:
		case ADSP2100_I5:
		case ADSP2100_I6:
		case ADSP2100_I7:
			update_i(entry.index() - ADSP2100_I0);
			break;

		case ADSP2100_L0:
		case ADSP2100_L1:
		case ADSP2100_L2:
		case ADSP2100_L3:
		case ADSP2100_L4:
		case ADSP2100_L5:
		case ADSP2100_L6:
		case ADSP2100_L7:
			update_l(entry.index() - ADSP2100_L0);
			break;

		// PMOVLAY
		case ADSP2100_DMOVLAY:
			update_dmovlay();
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(adsp21xx) called for unexpected value\n");
	}
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void adsp21xx_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				m_astat & 0x80 ? 'X':'.',
				m_astat & 0x40 ? 'M':'.',
				m_astat & 0x20 ? 'Q':'.',
				m_astat & 0x10 ? 'S':'.',
				m_astat & 0x08 ? 'C':'.',
				m_astat & 0x04 ? 'V':'.',
				m_astat & 0x02 ? 'N':'.',
				m_astat & 0x01 ? 'Z':'.');
			break;
	}
}


//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> adsp21xx_device::create_disassembler()
{
	return std::make_unique<adsp21xx_disassembler>();
}



/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

inline uint16_t adsp21xx_device::data_read(uint32_t addr)
{
	return m_data.read_word(addr);
}

inline void adsp21xx_device::data_write(uint32_t addr, uint16_t data)
{
	m_data.write_word(addr, data);
}

inline uint16_t adsp21xx_device::io_read(uint32_t addr)
{
	return m_io.read_word(addr);
}

inline void adsp21xx_device::io_write(uint32_t addr, uint16_t data)
{
	m_io.write_word(addr, data);
}

inline uint32_t adsp21xx_device::program_read(uint32_t addr)
{
	return m_program.read_dword(addr);
}

inline void adsp21xx_device::program_write(uint32_t addr, uint32_t data)
{
	m_program.write_dword(addr, data & 0xffffff);
}

inline uint32_t adsp21xx_device::opcode_read()
{
	return m_cache.read_dword(m_pc);
}


/***************************************************************************
    IMPORT CORE UTILITIES
***************************************************************************/

#include "2100ops.hxx"



/***************************************************************************
    IRQ HANDLING
***************************************************************************/

bool adsp2100_device::generate_irq(int which, int indx)
{
	// skip if masked
	if (!(m_imask & (1 << which)))
		return false;

	// clear the latch
	m_irq_latch[which] = 0;

	// push the PC and the status
	pc_stack_push();
	stat_stack_push();

	// vector to location & stop idling
	m_pc = indx;
	m_idle = 0;

	// mask other interrupts based on the nesting bit
	if (m_icntl & 0x10) m_imask &= ~((2 << which) - 1);
	else m_imask &= ~0xf;

	return true;
}


bool adsp2101_device::generate_irq(int which, int indx)
{
	// skip if masked
	if (!(m_imask & (0x20 >> indx)))
		return false;

	// clear the latch
	m_irq_latch[which] = 0;

	// push the PC and the status
	pc_stack_push();
	stat_stack_push();

	// vector to location & stop idling
	m_pc = 0x04 + indx * 4;
	m_idle = 0;

	// mask other interrupts based on the nesting bit
	if (m_icntl & 0x10) m_imask &= ~(0x3f >> indx);
	else m_imask &= ~0x3f;

	return true;
}


bool adsp2181_device::generate_irq(int which, int indx)
{
	// skip if masked
	if (!(m_imask & (0x200 >> indx)))
		return false;

	// clear the latch
	m_irq_latch[which] = 0;

	// push the PC and the status
	pc_stack_push();
	stat_stack_push();

	// vector to location & stop idling
	m_pc = 0x04 + indx * 4;
	m_idle = 0;

	// mask other interrupts based on the nesting bit
	if (m_icntl & 0x10) m_imask &= ~(0x3ff >> indx);
	else m_imask &= ~0x3ff;

	return true;
}


void adsp2100_device::check_irqs()
{
	uint8_t check;

	// check IRQ3
	check = (m_icntl & 8) ? m_irq_latch[ADSP2100_IRQ3] : m_irq_state[ADSP2100_IRQ3];
	if (check && generate_irq(ADSP2100_IRQ3, 3))
		return;

	// check IRQ2
	check = (m_icntl & 4) ? m_irq_latch[ADSP2100_IRQ2] : m_irq_state[ADSP2100_IRQ2];
	if (check && generate_irq(ADSP2100_IRQ2, 2))
		return;

	// check IRQ1
	check = (m_icntl & 2) ? m_irq_latch[ADSP2100_IRQ1] : m_irq_state[ADSP2100_IRQ1];
	if (check && generate_irq(ADSP2100_IRQ1, 1))
		return;

	// check IRQ0
	check = (m_icntl & 1) ? m_irq_latch[ADSP2100_IRQ0] : m_irq_state[ADSP2100_IRQ0];
	if (check && generate_irq(ADSP2100_IRQ0, 0))
		return;
}


void adsp2101_device::check_irqs()
{
	uint8_t check;

	// check IRQ2
	check = (m_icntl & 4) ? m_irq_latch[ADSP2101_IRQ2] : m_irq_state[ADSP2101_IRQ2];
	if (check && generate_irq(ADSP2101_IRQ2, 0))
		return;

	// check SPORT0 transmit
	check = m_irq_latch[ADSP2101_SPORT0_TX];
	if (check && generate_irq(ADSP2101_SPORT0_TX, 1))
		return;

	// check SPORT0 receive
	check = m_irq_latch[ADSP2101_SPORT0_RX];
	if (check && generate_irq(ADSP2101_SPORT0_RX, 2))
		return;

	// check IRQ1/SPORT1 transmit
	check = (m_icntl & 2) ? m_irq_latch[ADSP2101_IRQ1] : m_irq_state[ADSP2101_IRQ1];
	if (check && generate_irq(ADSP2101_IRQ1, 3))
		return;

	// check IRQ0/SPORT1 receive
	check = (m_icntl & 1) ? m_irq_latch[ADSP2101_IRQ0] : m_irq_state[ADSP2101_IRQ0];
	if (check && generate_irq(ADSP2101_IRQ0, 4))
		return;

	// check timer
	check = m_irq_latch[ADSP2101_TIMER];
	if (check && generate_irq(ADSP2101_TIMER, 5))
		return;
}


void adsp2181_device::check_irqs()
{
	uint8_t check;

	// check IRQ2
	check = (m_icntl & 4) ? m_irq_latch[ADSP2181_IRQ2] : m_irq_state[ADSP2181_IRQ2];
	if (check && generate_irq(ADSP2181_IRQ2, 0))
		return;

	// check IRQL1
	check = m_irq_state[ADSP2181_IRQL1];
	if (check && generate_irq(ADSP2181_IRQL1, 1))
		return;

	// check IRQL0
	check = m_irq_state[ADSP2181_IRQL0];
	if (check && generate_irq(ADSP2181_IRQL0, 2))
		return;

	// check SPORT0 transmit
	check = m_irq_latch[ADSP2181_SPORT0_TX];
	if (check && generate_irq(ADSP2181_SPORT0_TX, 3))
		return;

	// check SPORT0 receive
	check = m_irq_latch[ADSP2181_SPORT0_RX];
	if (check && generate_irq(ADSP2181_SPORT0_RX, 4))
		return;

	// check IRQE
	check = m_irq_latch[ADSP2181_IRQE];
	if (check && generate_irq(ADSP2181_IRQE, 5))
		return;

	// check BDMA interrupt
	check = m_irq_latch[ADSP2181_BDMA];
	if (check && generate_irq(ADSP2181_BDMA, 6))
		return;

	// check IRQ1/SPORT1 transmit
	check = (m_icntl & 2) ? m_irq_latch[ADSP2181_IRQ1] : m_irq_state[ADSP2181_IRQ1];
	if (check && generate_irq(ADSP2181_IRQ1, 7))
		return;

	// check IRQ0/SPORT1 receive
	check = (m_icntl & 1) ? m_irq_latch[ADSP2181_IRQ0] : m_irq_state[ADSP2181_IRQ0];
	if (check && generate_irq(ADSP2181_IRQ0, 8))
		return;

	// check timer
	check = m_irq_latch[ADSP2181_TIMER];
	if (check && generate_irq(ADSP2181_TIMER, 9))
		return;
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

void adsp21xx_device::create_tables()
{
	// initialize the bit reversing table
	for (int i = 0; i < 0x4000; i++)
	{
		uint16_t data = 0;

		data |= (i >> 13) & 0x0001;
		data |= (i >> 11) & 0x0002;
		data |= (i >> 9)  & 0x0004;
		data |= (i >> 7)  & 0x0008;
		data |= (i >> 5)  & 0x0010;
		data |= (i >> 3)  & 0x0020;
		data |= (i >> 1)  & 0x0040;
		data |= (i << 1)  & 0x0080;
		data |= (i << 3)  & 0x0100;
		data |= (i << 5)  & 0x0200;
		data |= (i << 7)  & 0x0400;
		data |= (i << 9)  & 0x0800;
		data |= (i << 11) & 0x1000;
		data |= (i << 13) & 0x2000;

		m_reverse_table[i] = data;
	}

	// initialize the mask table
	for (int i = 0; i < 0x4000; i++)
	{
		if (i > 0x2000)      m_mask_table[i] = 0x0000;
		else if (i > 0x1000) m_mask_table[i] = 0x2000;
		else if (i > 0x0800) m_mask_table[i] = 0x3000;
		else if (i > 0x0400) m_mask_table[i] = 0x3800;
		else if (i > 0x0200) m_mask_table[i] = 0x3c00;
		else if (i > 0x0100) m_mask_table[i] = 0x3e00;
		else if (i > 0x0080) m_mask_table[i] = 0x3f00;
		else if (i > 0x0040) m_mask_table[i] = 0x3f80;
		else if (i > 0x0020) m_mask_table[i] = 0x3fc0;
		else if (i > 0x0010) m_mask_table[i] = 0x3fe0;
		else if (i > 0x0008) m_mask_table[i] = 0x3ff0;
		else if (i > 0x0004) m_mask_table[i] = 0x3ff8;
		else if (i > 0x0002) m_mask_table[i] = 0x3ffc;
		else if (i > 0x0001) m_mask_table[i] = 0x3ffe;
		else                 m_mask_table[i] = 0x3fff;
	}

	// initialize the condition table
	for (int i = 0; i < 0x100; i++)
	{
		int az = ((i & ZFLAG) != 0);
		int an = ((i & NFLAG) != 0);
		int av = ((i & VFLAG) != 0);
		int ac = ((i & CFLAG) != 0);
		int mv = ((i & MVFLAG) != 0);
		int as = ((i & SFLAG) != 0);

		m_condition_table[i | 0x000] = az;
		m_condition_table[i | 0x100] = !az;
		m_condition_table[i | 0x200] = !((an ^ av) | az);
		m_condition_table[i | 0x300] = (an ^ av) | az;
		m_condition_table[i | 0x400] = an ^ av;
		m_condition_table[i | 0x500] = !(an ^ av);
		m_condition_table[i | 0x600] = av;
		m_condition_table[i | 0x700] = !av;
		m_condition_table[i | 0x800] = ac;
		m_condition_table[i | 0x900] = !ac;
		m_condition_table[i | 0xa00] = as;
		m_condition_table[i | 0xb00] = !as;
		m_condition_table[i | 0xc00] = mv;
		m_condition_table[i | 0xd00] = !mv;
		m_condition_table[i | 0xf00] = 1;
	}
}



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t adsp21xx_device::execute_min_cycles() const noexcept
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t adsp21xx_device::execute_max_cycles() const noexcept
{
	return 1;
}


void adsp21xx_device::execute_set_input(int inputnum, int state)
{
	// update the latched state
	if (state != CLEAR_LINE && m_irq_state[inputnum] == CLEAR_LINE)
		m_irq_latch[inputnum] = 1;

	// update the absolute state
	m_irq_state[inputnum] = state;
}


void adsp21xx_device::execute_run()
{
	// Return if CPU is halted
	if (current_input_state(INPUT_LINE_HALT)) {
		debugger_wait_hook();
		m_icount = 0;
		return;
	}

	bool check_debugger = ((device_t::machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);

	check_irqs();

	do
	{
		// debugging
		m_ppc = m_pc;   // copy PC to previous PC
		if (check_debugger)
			debugger_instruction_hook(m_pc);

#if ADSP_TRACK_HOTSPOTS
		m_pcbucket[m_pc & 0x3fff]++;
#endif

		// instruction fetch
		uint32_t op = opcode_read();

		// advance to the next instruction
		if (m_pc != m_loop)
			m_pc++;

		// handle looping
		else
		{
			// condition not met, keep looping
			if (condition(m_loop_condition))
				m_pc = pc_stack_top();

			// condition met; pop the PC and loop stacks and fall through
			else
			{
				loop_stack_pop();
				pc_stack_pop_val();
				m_pc++;
			}
		}

		// parse the instruction
		uint32_t temp;
		switch (BIT(op, 16, 8))
		{
			case 0x00:
				// 00000000 00000000 00000000  NOP
				break;
			case 0x01:
				// 00000001 0xxxxxxx xxxxxxxx  dst = IO(x)
				// 00000001 1xxxxxxx xxxxxxxx  IO(x) = dst
				// ADSP-218x only
				if (m_chip_type >= CHIP_TYPE_ADSP2181)
				{
					if (!BIT(op, 15))
						write_reg0(BIT(op, 0, 4), io_read(BIT(op, 4, 11)));
					else
						io_write(BIT(op, 4, 11), read_reg0(BIT(op, 0, 4)));
				}
				break;
			case 0x02:
				// 00000010 0000xxxx xxxxxxxx  modify flag out
				// 00000010 10000000 00000000  idle
				// 00000010 10000000 0000xxxx  idle (n)
				if (BIT(op, 15))
				{
					m_idle = 1;
					m_icount = 0;
				}
				else
				{
					if (condition(BIT(op, 0, 4)))
					{
						if (BIT(op, 5)) m_flagout = 0;
						if (BIT(op, 4)) m_flagout ^= 1;
						if (m_chip_type >= CHIP_TYPE_ADSP2101)
						{
							if (BIT(op, 7)) m_fl0 = 0;
							if (BIT(op, 6)) m_fl0 ^= 1;
							if (BIT(op, 9)) m_fl1 = 0;
							if (BIT(op, 8)) m_fl1 ^= 1;
							if (BIT(op, 11)) m_fl2 = 0;
							if (BIT(op, 10)) m_fl2 ^= 1;
						}
					}
				}
				break;
			case 0x03:
				// 00000011 xxxxxxxx xxxxxxxx  call or jump on flag in
				if (BIT(op, 1) ? m_flagin : !m_flagin)
				{
					if (BIT(op, 0))
						pc_stack_push();
					m_pc = BIT(op, 4, 12) | BIT(op, 2, 2) << 12;
				}
				break;
			case 0x04:
				// 00000100 00000000 000xxxxx  stack control
				if (BIT(op, 4)) pc_stack_pop_val();
				if (BIT(op, 3)) loop_stack_pop();
				if (BIT(op, 2)) cntr_stack_pop();
				if (BIT(op, 1))
				{
					if (BIT(op, 0)) stat_stack_pop();
					else stat_stack_push();
				}
				break;
			case 0x05:
				// 00000101 00000000 00000000  saturate MR
				if (GET_MV)
				{
					if (m_core.mr.mrx.mr2.u & 0x80)
						m_core.mr.mrx.mr2.u = 0xffff, m_core.mr.mrx.mr1.u = 0x8000, m_core.mr.mrx.mr0.u = 0x0000;
					else
						m_core.mr.mrx.mr2.u = 0x0000, m_core.mr.mrx.mr1.u = 0x7fff, m_core.mr.mrx.mr0.u = 0xffff;
				}
				break;
			case 0x06:
				// 00000110 000xxxxx 00000000  DIVS
				{
					int xop = BIT(op, 8, 3);
					int yop = BIT(op, 11, 2);

					xop = ALU_GETXREG_UNSIGNED(xop);
					yop = ALU_GETYREG_UNSIGNED(yop);

					temp = xop ^ yop;
					m_astat = (m_astat & ~QFLAG) | ((temp >> 10) & QFLAG);
					m_core.af.u = (yop << 1) | (m_core.ay0.u >> 15);
					m_core.ay0.u = (m_core.ay0.u << 1) | (temp >> 15);
				}
				break;
			case 0x07:
				// 00000111 00010xxx 00000000  DIVQ
				{
					int xop = BIT(op, 8, 3);
					int res;

					xop = ALU_GETXREG_UNSIGNED(xop);

					if (GET_Q)
						res = m_core.af.u + xop;
					else
						res = m_core.af.u - xop;

					temp = res ^ xop;
					m_astat = (m_astat & ~QFLAG) | ((temp >> 10) & QFLAG);
					m_core.af.u = (res << 1) | (m_core.ay0.u >> 15);
					m_core.ay0.u = (m_core.ay0.u << 1) | ((~temp >> 15) & 0x0001);
				}
				break;
			case 0x08:
				// 00001000 00000000 0000xxxx  reserved
				break;
			case 0x09:
				// 00001001 00000000 000xxxxx  modify address register
				temp = BIT(op, 2, 3);
				modify_address(temp, (temp & 4) | (op & 3));
				break;
			case 0x0a:
				// 00001010 00000000 000xxxxx  conditional return
				if (condition(BIT(op, 0, 4)))
				{
					pc_stack_pop();

					// RTI case
					if (BIT(op, 4))
						stat_stack_pop();
				}
				break;
			case 0x0b:
				// 00001011 00000000 xxxxxxxx  conditional jump (indirect address)
				if (condition(BIT(op, 0, 4)))
				{
					if (BIT(op, 4))
						pc_stack_push();
					m_pc = m_i[4 + BIT(op, 6, 2)] & 0x3fff;
				}
				break;
			case 0x0c:
				// 00001100 xxxxxxxx xxxxxxxx  mode control
				if (m_chip_type >= CHIP_TYPE_ADSP2101)
				{
					if (BIT(op, 3)) m_mstat = BIT(op, 2) ? (m_mstat | MSTAT_GOMODE) : (m_mstat & ~MSTAT_GOMODE);
					if (BIT(op, 13)) m_mstat = BIT(op, 12) ? (m_mstat | MSTAT_INTEGER) : (m_mstat & ~MSTAT_INTEGER);
					if (BIT(op, 15)) m_mstat = BIT(op, 14) ? (m_mstat | MSTAT_TIMER) : (m_mstat & ~MSTAT_TIMER);
				}
				if (BIT(op, 5)) m_mstat = BIT(op, 4) ? (m_mstat | MSTAT_BANK) : (m_mstat & ~MSTAT_BANK);
				if (BIT(op, 7)) m_mstat = BIT(op, 6) ? (m_mstat | MSTAT_REVERSE) : (m_mstat & ~MSTAT_REVERSE);
				if (BIT(op, 9)) m_mstat = BIT(op, 8) ? (m_mstat | MSTAT_STICKYV) : (m_mstat & ~MSTAT_STICKYV);
				if (BIT(op, 11)) m_mstat = BIT(op, 10) ? (m_mstat | MSTAT_SATURATE) : (m_mstat & ~MSTAT_SATURATE);
				update_mstat();
				break;
			case 0x0d:
				// 00001101 0000xxxx xxxxxxxx  internal data move
				switch (BIT(op, 8, 4))
				{
					case 0x00:  write_reg0(BIT(op, 4, 4), read_reg0(BIT(op, 0, 4))); break;
					case 0x01:  write_reg0(BIT(op, 4, 4), read_reg1(BIT(op, 0, 4))); break;
					case 0x02:  write_reg0(BIT(op, 4, 4), read_reg2(BIT(op, 0, 4))); break;
					case 0x03:  write_reg0(BIT(op, 4, 4), read_reg3(BIT(op, 0, 4))); break;
					case 0x04:  write_reg1(BIT(op, 4, 4), read_reg0(BIT(op, 0, 4))); break;
					case 0x05:  write_reg1(BIT(op, 4, 4), read_reg1(BIT(op, 0, 4))); break;
					case 0x06:  write_reg1(BIT(op, 4, 4), read_reg2(BIT(op, 0, 4))); break;
					case 0x07:  write_reg1(BIT(op, 4, 4), read_reg3(BIT(op, 0, 4))); break;
					case 0x08:  write_reg2(BIT(op, 4, 4), read_reg0(BIT(op, 0, 4))); break;
					case 0x09:  write_reg2(BIT(op, 4, 4), read_reg1(BIT(op, 0, 4))); break;
					case 0x0a:  write_reg2(BIT(op, 4, 4), read_reg2(BIT(op, 0, 4))); break;
					case 0x0b:  write_reg2(BIT(op, 4, 4), read_reg3(BIT(op, 0, 4))); break;
					case 0x0c:  write_reg3(BIT(op, 4, 4), read_reg0(BIT(op, 0, 4))); break;
					case 0x0d:  write_reg3(BIT(op, 4, 4), read_reg1(BIT(op, 0, 4))); break;
					case 0x0e:  write_reg3(BIT(op, 4, 4), read_reg2(BIT(op, 0, 4))); break;
					case 0x0f:  write_reg3(BIT(op, 4, 4), read_reg3(BIT(op, 0, 4))); break;
				}
				break;
			case 0x0e:
				// 00001110 0xxxxxxx xxxxxxxx  conditional shift
				if (condition(BIT(op, 0, 4))) shift_op(op);
				break;
			case 0x0f:
				// 00001111 0xxxxxxx xxxxxxxx  shift immediate
				shift_op_imm(op);
				break;
			case 0x10:
				// 00010000 0xxxxxxx xxxxxxxx  shift with internal data register move
				shift_op(op);
				temp = read_reg0(BIT(op, 0, 4));
				write_reg0(BIT(op, 4, 4), temp);
				break;
			case 0x11:
				// 00010001 xxxxxxxx xxxxxxxx  shift with pgm memory read/write
				if (BIT(op, 15))
				{
					pgm_write_dag2(op, read_reg0(BIT(op, 4, 4)));
					shift_op(op);
				}
				else
				{
					shift_op(op);
					write_reg0(BIT(op, 4, 4), pgm_read_dag2(op));
				}
				break;
			case 0x12:
				// 00010010 xxxxxxxx xxxxxxxx  shift with data memory read/write DAG1
				if (BIT(op, 15))
				{
					data_write_dag1(op, read_reg0(BIT(op, 4, 4)));
					shift_op(op);
				}
				else
				{
					shift_op(op);
					write_reg0(BIT(op, 4, 4), data_read_dag1(op));
				}
				break;
			case 0x13:
				// 00010011 xxxxxxxx xxxxxxxx  shift with data memory read/write DAG2
				if (BIT(op, 15))
				{
					data_write_dag2(op, read_reg0(BIT(op, 4, 4)));
					shift_op(op);
				}
				else
				{
					shift_op(op);
					write_reg0(BIT(op, 4, 4), data_read_dag2(op));
				}
				break;
			case 0x14: case 0x15: case 0x16: case 0x17:
				// 000101xx xxxxxxxx xxxxxxxx  do until
				loop_stack_push(op & 0x3ffff);
				pc_stack_push();
				break;
			case 0x18: case 0x19: case 0x1a: case 0x1b:
				// 000110xx xxxxxxxx xxxxxxxx  conditional jump (immediate addr)
				if (condition(BIT(op, 0, 4)))
				{
					m_pc = BIT(op, 4, 14);
					// check for a busy loop
					if (m_pc == m_ppc)
						m_icount = 0;
				}
				break;
			case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				// 000111xx xxxxxxxx xxxxxxxx  conditional call (immediate addr)
				if (condition(BIT(op, 0, 4)))
				{
					pc_stack_push();
					m_pc = BIT(op, 4, 14);
				}
				break;
			case 0x20: case 0x21:
				// 0010000x xxxxxxxx xxxxxxxx  conditional MAC to MR
				if (condition(BIT(op, 0, 4)))
				{
					if (m_chip_type >= CHIP_TYPE_ADSP2181 && (op & 0x0018f0) == 0x000010)
						mac_op_mr_xop(op);
					else
						mac_op_mr(op);
				}
				break;
			case 0x22: case 0x23:
				// 0010001x xxxxxxxx xxxxxxxx  conditional ALU to AR
				if (condition(BIT(op, 0, 4)))
				{
					if (m_chip_type >= CHIP_TYPE_ADSP2181 && BIT(op, 4))
						alu_op_ar_const(op);
					else
						alu_op_ar(op);
				}
				break;
			case 0x24: case 0x25:
				// 0010010x xxxxxxxx xxxxxxxx  conditional MAC to MF
				if (condition(BIT(op, 0, 4)))
				{
					if (m_chip_type >= CHIP_TYPE_ADSP2181 && (op & 0x0018f0) == 0x000010)
						mac_op_mf_xop(op);
					else
						mac_op_mf(op);
				}
				break;
			case 0x26: case 0x27:
				// 0010011x xxxxxxxx xxxxxxxx  conditional ALU to AF
				if (condition(BIT(op, 0, 4)))
				{
					if (m_chip_type >= CHIP_TYPE_ADSP2181 && BIT(op, 4))
						alu_op_af_const(op);
					else
						alu_op_af(op);
				}
				break;
			case 0x28: case 0x29:
				// 0010100x xxxxxxxx xxxxxxxx  MAC to MR with internal data register move
				temp = read_reg0(BIT(op, 0, 4));
				mac_op_mr(op);
				write_reg0(BIT(op, 4, 4), temp);
				break;
			case 0x2a: case 0x2b:
				// 0010101x xxxxxxxx xxxxxxxx  ALU to AR with internal data register move
				if (m_chip_type >= CHIP_TYPE_ADSP2181 && BIT(op, 0, 8) == 0xaa)
					alu_op_none(op);
				else
				{
					temp = read_reg0(BIT(op, 0, 4));
					alu_op_ar(op);
					write_reg0(BIT(op, 4, 4), temp);
				}
				break;
			case 0x2c: case 0x2d:
				// 0010110x xxxxxxxx xxxxxxxx  MAC to MF with internal data register move
				temp = read_reg0(BIT(op, 0, 4));
				mac_op_mf(op);
				write_reg0(BIT(op, 4, 4), temp);
				break;
			case 0x2e: case 0x2f:
				// 0010111x xxxxxxxx xxxxxxxx  ALU to AF with internal data register move
				temp = read_reg0(BIT(op, 0, 4));
				alu_op_af(op);
				write_reg0(BIT(op, 4, 4), temp);
				break;
			case 0x30: case 0x31: case 0x32: case 0x33:
				// 001100xx xxxxxxxx xxxxxxxx  load non-data register immediate (group 0)
				write_reg0(BIT(op, 0, 4), util::sext(op >> 4, 14));
				break;
			case 0x34: case 0x35: case 0x36: case 0x37:
				// 001101xx xxxxxxxx xxxxxxxx  load non-data register immediate (group 1)
				write_reg1(BIT(op, 0, 4), util::sext(op >> 4, 14));
				break;
			case 0x38: case 0x39: case 0x3a: case 0x3b:
				// 001110xx xxxxxxxx xxxxxxxx  load non-data register immediate (group 2)
				write_reg2(BIT(op, 0, 4), util::sext(op >> 4, 14));
				break;
			case 0x3c: case 0x3d: case 0x3e: case 0x3f:
				// 001111xx xxxxxxxx xxxxxxxx  load non-data register immediate (group 3)
				write_reg3(BIT(op, 0, 4), util::sext(op >> 4, 14));
				break;
			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
			case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
				// 0100xxxx xxxxxxxx xxxxxxxx  load data register immediate
				write_reg0(BIT(op, 0, 4), BIT(op, 4, 16));
				break;
			case 0x50: case 0x51:
				// 0101000x xxxxxxxx xxxxxxxx  MAC to MR with pgm memory read
				mac_op_mr(op);
				write_reg0(BIT(op, 4, 4), pgm_read_dag2(op));
				break;
			case 0x52: case 0x53:
				// 0101001x xxxxxxxx xxxxxxxx  ALU to AR with pgm memory read
				alu_op_ar(op);
				write_reg0(BIT(op, 4, 4), pgm_read_dag2(op));
				break;
			case 0x54: case 0x55:
				// 0101010x xxxxxxxx xxxxxxxx  MAC to MF with pgm memory read
				mac_op_mf(op);
				write_reg0(BIT(op, 4, 4), pgm_read_dag2(op));
				break;
			case 0x56: case 0x57:
				// 0101011x xxxxxxxx xxxxxxxx  ALU to AF with pgm memory read
				alu_op_af(op);
				write_reg0(BIT(op, 4, 4), pgm_read_dag2(op));
				break;
			case 0x58: case 0x59:
				// 0101100x xxxxxxxx xxxxxxxx  MAC to MR with pgm memory write
				pgm_write_dag2(op, read_reg0(BIT(op, 4, 4)));
				mac_op_mr(op);
				break;
			case 0x5a: case 0x5b:
				// 0101101x xxxxxxxx xxxxxxxx  ALU to AR with pgm memory write
				pgm_write_dag2(op, read_reg0(BIT(op, 4, 4)));
				alu_op_ar(op);
				break;
			case 0x5c: case 0x5d:
				// 0101110x xxxxxxxx xxxxxxxx  ALU to MR with pgm memory write
				pgm_write_dag2(op, read_reg0(BIT(op, 4, 4)));
				mac_op_mf(op);
				break;
			case 0x5e: case 0x5f:
				// 0101111x xxxxxxxx xxxxxxxx  ALU to MF with pgm memory write
				pgm_write_dag2(op, read_reg0(BIT(op, 4, 4)));
				alu_op_af(op);
				break;
			case 0x60: case 0x61:
				// 0110000x xxxxxxxx xxxxxxxx  MAC to MR with data memory read DAG1
				mac_op_mr(op);
				write_reg0(BIT(op, 4, 4), data_read_dag1(op));
				break;
			case 0x62: case 0x63:
				// 0110001x xxxxxxxx xxxxxxxx  ALU to AR with data memory read DAG1
				alu_op_ar(op);
				write_reg0(BIT(op, 4, 4), data_read_dag1(op));
				break;
			case 0x64: case 0x65:
				// 0110010x xxxxxxxx xxxxxxxx  MAC to MF with data memory read DAG1
				mac_op_mf(op);
				write_reg0(BIT(op, 4, 4), data_read_dag1(op));
				break;
			case 0x66: case 0x67:
				// 0110011x xxxxxxxx xxxxxxxx  ALU to AF with data memory read DAG1
				alu_op_af(op);
				write_reg0(BIT(op, 4, 4), data_read_dag1(op));
				break;
			case 0x68: case 0x69:
				// 0110100x xxxxxxxx xxxxxxxx  MAC to MR with data memory write DAG1
				data_write_dag1(op, read_reg0(BIT(op, 4, 4)));
				mac_op_mr(op);
				break;
			case 0x6a: case 0x6b:
				// 0110101x xxxxxxxx xxxxxxxx  ALU to AR with data memory write DAG1
				data_write_dag1(op, read_reg0(BIT(op, 4, 4)));
				alu_op_ar(op);
				break;
			case 0x6c: case 0x6d:
				// 0111110x xxxxxxxx xxxxxxxx  MAC to MF with data memory write DAG1
				data_write_dag1(op, read_reg0(BIT(op, 4, 4)));
				mac_op_mf(op);
				break;
			case 0x6e: case 0x6f:
				// 0111111x xxxxxxxx xxxxxxxx  ALU to AF with data memory write DAG1
				data_write_dag1(op, read_reg0(BIT(op, 4, 4)));
				alu_op_af(op);
				break;
			case 0x70: case 0x71:
				// 0111000x xxxxxxxx xxxxxxxx  MAC to MR with data memory read DAG2
				mac_op_mr(op);
				write_reg0(BIT(op, 4, 4), data_read_dag2(op));
				break;
			case 0x72: case 0x73:
				// 0111001x xxxxxxxx xxxxxxxx  ALU to AR with data memory read DAG2
				alu_op_ar(op);
				write_reg0(BIT(op, 4, 4), data_read_dag2(op));
				break;
			case 0x74: case 0x75:
				// 0111010x xxxxxxxx xxxxxxxx  MAC to MF with data memory read DAG2
				mac_op_mf(op);
				write_reg0(BIT(op, 4, 4), data_read_dag2(op));
				break;
			case 0x76: case 0x77:
				// 0111011x xxxxxxxx xxxxxxxx  ALU to AF with data memory read DAG2
				alu_op_af(op);
				write_reg0(BIT(op, 4, 4), data_read_dag2(op));
				break;
			case 0x78: case 0x79:
				// 0111100x xxxxxxxx xxxxxxxx  MAC to MR with data memory write DAG2
				data_write_dag2(op, read_reg0(BIT(op, 4, 4)));
				mac_op_mr(op);
				break;
			case 0x7a: case 0x7b:
				// 0111101x xxxxxxxx xxxxxxxx  ALU to AR with data memory write DAG2
				data_write_dag2(op, read_reg0(BIT(op, 4, 4)));
				alu_op_ar(op);
				break;
			case 0x7c: case 0x7d:
				// 0111110x xxxxxxxx xxxxxxxx  MAC to MF with data memory write DAG2
				data_write_dag2(op, read_reg0(BIT(op, 4, 4)));
				mac_op_mf(op);
				break;
			case 0x7e: case 0x7f:
				// 0111111x xxxxxxxx xxxxxxxx  ALU to AF with data memory write DAG2
				data_write_dag2(op, read_reg0(BIT(op, 4, 4)));
				alu_op_af(op);
				break;
			case 0x80: case 0x81: case 0x82: case 0x83:
				// 100000xx xxxxxxxx xxxxxxxx  read data memory (immediate addr) to reg group 0
				write_reg0(BIT(op, 0, 4), data_read(BIT(op, 4, 14)));
				break;
			case 0x84: case 0x85: case 0x86: case 0x87:
				// 100001xx xxxxxxxx xxxxxxxx  read data memory (immediate addr) to reg group 1
				write_reg1(BIT(op, 0, 4), data_read(BIT(op, 4, 14)));
				break;
			case 0x88: case 0x89: case 0x8a: case 0x8b:
				// 100010xx xxxxxxxx xxxxxxxx  read data memory (immediate addr) to reg group 2
				write_reg2(BIT(op, 0, 4), data_read(BIT(op, 4, 14)));
				break;
			case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				// 100011xx xxxxxxxx xxxxxxxx  read data memory (immediate addr) to reg group 3
				write_reg3(BIT(op, 0, 4), data_read(BIT(op, 4, 14)));
				break;
			case 0x90: case 0x91: case 0x92: case 0x93:
				// 1001xxxx xxxxxxxx xxxxxxxx  write data memory (immediate addr) from reg group 0
				data_write(BIT(op, 4, 14), read_reg0(BIT(op, 0, 4)));
				break;
			case 0x94: case 0x95: case 0x96: case 0x97:
				// 1001xxxx xxxxxxxx xxxxxxxx  write data memory (immediate addr) from reg group 1
				data_write(BIT(op, 4, 14), read_reg1(BIT(op, 0, 4)));
				break;
			case 0x98: case 0x99: case 0x9a: case 0x9b:
				// 1001xxxx xxxxxxxx xxxxxxxx  write data memory (immediate addr) from reg group 2
				data_write(BIT(op, 4, 14), read_reg2(BIT(op, 0, 4)));
				break;
			case 0x9c: case 0x9d: case 0x9e: case 0x9f:
				// 1001xxxx xxxxxxxx xxxxxxxx  write data memory (immediate addr) from reg group 3
				data_write(BIT(op, 4, 14), read_reg3(BIT(op, 0, 4)));
				break;
			case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
				// 1010xxxx xxxxxxxx xxxxxxxx  data memory write (immediate) DAG1
				data_write_dag1(op, BIT(op, 4, 16));
				break;
			case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
				// 1011xxxx xxxxxxxx xxxxxxxx  data memory write (immediate) DAG2
				data_write_dag2(op, BIT(op, 4, 16));
				break;
			case 0xc0: case 0xc1:
				// 1100000x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX0 & pgm read to AY0
				mac_op_mr(op);
				m_core.ax0.u = data_read_dag1(op);
				m_core.ay0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xc2: case 0xc3:
				// 1100001x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX0 & pgm read to AY0
				alu_op_ar(op);
				m_core.ax0.u = data_read_dag1(op);
				m_core.ay0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xc4: case 0xc5:
				// 1100010x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX1 & pgm read to AY0
				mac_op_mr(op);
				m_core.ax1.u = data_read_dag1(op);
				m_core.ay0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xc6: case 0xc7:
				// 1100011x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX1 & pgm read to AY0
				alu_op_ar(op);
				m_core.ax1.u = data_read_dag1(op);
				m_core.ay0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xc8: case 0xc9:
				// 1100100x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX0 & pgm read to AY0
				mac_op_mr(op);
				m_core.mx0.u = data_read_dag1(op);
				m_core.ay0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xca: case 0xcb:
				// 1100101x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX0 & pgm read to AY0
				alu_op_ar(op);
				m_core.mx0.u = data_read_dag1(op);
				m_core.ay0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xcc: case 0xcd:
				// 1100110x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX1 & pgm read to AY0
				mac_op_mr(op);
				m_core.mx1.u = data_read_dag1(op);
				m_core.ay0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xce: case 0xcf:
				// 1100111x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX1 & pgm read to AY0
				alu_op_ar(op);
				m_core.mx1.u = data_read_dag1(op);
				m_core.ay0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xd0: case 0xd1:
				// 1101000x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX0 & pgm read to AY1
				mac_op_mr(op);
				m_core.ax0.u = data_read_dag1(op);
				m_core.ay1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xd2: case 0xd3:
				// 1101001x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX0 & pgm read to AY1
				alu_op_ar(op);
				m_core.ax0.u = data_read_dag1(op);
				m_core.ay1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xd4: case 0xd5:
				// 1101010x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX1 & pgm read to AY1
				mac_op_mr(op);
				m_core.ax1.u = data_read_dag1(op);
				m_core.ay1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xd6: case 0xd7:
				// 1101011x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX1 & pgm read to AY1
				alu_op_ar(op);
				m_core.ax1.u = data_read_dag1(op);
				m_core.ay1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xd8: case 0xd9:
				// 1101100x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX0 & pgm read to AY1
				mac_op_mr(op);
				m_core.mx0.u = data_read_dag1(op);
				m_core.ay1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xda: case 0xdb:
				// 1101101x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX0 & pgm read to AY1
				alu_op_ar(op);
				m_core.mx0.u = data_read_dag1(op);
				m_core.ay1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xdc: case 0xdd:
				// 1101110x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX1 & pgm read to AY1
				mac_op_mr(op);
				m_core.mx1.u = data_read_dag1(op);
				m_core.ay1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xde: case 0xdf:
				// 1101111x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX1 & pgm read to AY1
				alu_op_ar(op);
				m_core.mx1.u = data_read_dag1(op);
				m_core.ay1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xe0: case 0xe1:
				// 1110000x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX0 & pgm read to MY0
				mac_op_mr(op);
				m_core.ax0.u = data_read_dag1(op);
				m_core.my0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xe2: case 0xe3:
				// 1110001x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX0 & pgm read to MY0
				alu_op_ar(op);
				m_core.ax0.u = data_read_dag1(op);
				m_core.my0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xe4: case 0xe5:
				// 1110010x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX1 & pgm read to MY0
				mac_op_mr(op);
				m_core.ax1.u = data_read_dag1(op);
				m_core.my0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xe6: case 0xe7:
				// 1110011x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX1 & pgm read to MY0
				alu_op_ar(op);
				m_core.ax1.u = data_read_dag1(op);
				m_core.my0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xe8: case 0xe9:
				// 1110100x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX0 & pgm read to MY0
				mac_op_mr(op);
				m_core.mx0.u = data_read_dag1(op);
				m_core.my0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xea: case 0xeb:
				// 1110101x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX0 & pgm read to MY0
				alu_op_ar(op);
				m_core.mx0.u = data_read_dag1(op);
				m_core.my0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xec: case 0xed:
				// 1110110x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX1 & pgm read to MY0
				mac_op_mr(op);
				m_core.mx1.u = data_read_dag1(op);
				m_core.my0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xee: case 0xef:
				// 1110111x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX1 & pgm read to MY0
				alu_op_ar(op);
				m_core.mx1.u = data_read_dag1(op);
				m_core.my0.u = pgm_read_dag2(op >> 4);
				break;
			case 0xf0: case 0xf1:
				// 1111000x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX0 & pgm read to MY1
				mac_op_mr(op);
				m_core.ax0.u = data_read_dag1(op);
				m_core.my1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xf2: case 0xf3:
				// 1111001x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX0 & pgm read to MY1
				alu_op_ar(op);
				m_core.ax0.u = data_read_dag1(op);
				m_core.my1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xf4: case 0xf5:
				// 1111010x xxxxxxxx xxxxxxxx  MAC to MR with data read to AX1 & pgm read to MY1
				mac_op_mr(op);
				m_core.ax1.u = data_read_dag1(op);
				m_core.my1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xf6: case 0xf7:
				// 1111011x xxxxxxxx xxxxxxxx  ALU to AR with data read to AX1 & pgm read to MY1
				alu_op_ar(op);
				m_core.ax1.u = data_read_dag1(op);
				m_core.my1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xf8: case 0xf9:
				// 1111100x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX0 & pgm read to MY1
				mac_op_mr(op);
				m_core.mx0.u = data_read_dag1(op);
				m_core.my1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xfa: case 0xfb:
				// 1111101x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX0 & pgm read to MY1
				alu_op_ar(op);
				m_core.mx0.u = data_read_dag1(op);
				m_core.my1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xfc: case 0xfd:
				// 1111110x xxxxxxxx xxxxxxxx  MAC to MR with data read to MX1 & pgm read to MY1
				mac_op_mr(op);
				m_core.mx1.u = data_read_dag1(op);
				m_core.my1.u = pgm_read_dag2(op >> 4);
				break;
			case 0xfe: case 0xff:
				// 1111111x xxxxxxxx xxxxxxxx  ALU to AR with data read to MX1 & pgm read to MY1
				alu_op_ar(op);
				m_core.mx1.u = data_read_dag1(op);
				m_core.my1.u = pgm_read_dag2(op >> 4);
				break;
		}

		m_icount--;
	} while (m_icount > 0);
}
