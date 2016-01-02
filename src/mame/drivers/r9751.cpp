// license:GPL-2.0+
// copyright-holders:Brandon Munger
/******************************************************************************
*
* Rolm CBX 9751 Driver
*
* This driver attempts to emulate the following models:
* * Model 10
* * Model 20
* * Model 40
* * Model 50
* * Model 70
*
* The following software releases are known:
* * 9004
* * 9005
*
* The basis of this driver was influenced by the zexall.c driver by
* Jonathan Gevaryahu and Robbbert.
*
*
* Special Thanks to:
* * Stephen Stair (sgstair)   - for help with reverse engineering,
*               programming, and emulation
* * Felipe Sanches (FSanches) - for help with MESS and emulation
* * Michael Zapf (mizapf)     - for building the HDC9234/HDC9224
*               driver that makes this driver possible
*
* Memory map:
* * 0x00000000 - 0x00ffffff : RAM 12MB to 16MB known, up to 128MB?
* * 0x08000000 - 0x0800ffff : PROM Region
*
******************************************************************************/

/* Core includes */
#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/terminal.h"

#include "bus/scsi/scsi.h"
#include "machine/wd33c93.h"

#include "machine/pdc.h"
#include "softlist.h"

#define TERMINAL_TAG "terminal"

/* Log defines */
#define TRACE_FDC 0
#define TRACE_HDC 0
#define TRACE_SMIOC 0
#define TRACE_CPU_REG 0
#define TRACE_LED 0

class r9751_state : public driver_device
{
public:
	r9751_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pdc(*this, "pdc"),
		m_wd33c93(*this, "wd33c93"),
		m_terminal(*this, TERMINAL_TAG),
		m_main_ram(*this, "main_ram")
	{
	}

	DECLARE_READ32_MEMBER(r9751_mmio_5ff_r);
	DECLARE_WRITE32_MEMBER(r9751_mmio_5ff_w);
		DECLARE_READ32_MEMBER(r9751_mmio_ff05_r);
		DECLARE_WRITE32_MEMBER(r9751_mmio_ff05_w);
	DECLARE_READ32_MEMBER(r9751_mmio_fff8_r);
	DECLARE_WRITE32_MEMBER(r9751_mmio_fff8_w);

	DECLARE_READ8_MEMBER(pdc_dma_r);
	DECLARE_WRITE8_MEMBER(pdc_dma_w);

	DECLARE_DRIVER_INIT(r9751);

	//DECLARE_FLOPPY_FORMATS( floppy_formats );
private:
	required_device<cpu_device> m_maincpu;
	required_device<pdc_device> m_pdc;
	required_device<wd33c93_device> m_wd33c93;
	required_device<generic_terminal_device> m_terminal;
	required_shared_ptr<UINT32> m_main_ram;

	// Begin registers
	UINT32 reg_ff050004;
	UINT32 reg_ff050320; // Counter?
	UINT32 reg_fff80040;
	UINT32 fdd_dest_address; // 5FF080B0
	UINT32 fdd_cmd_complete;
	UINT32 smioc_out_addr;
	// End registers

//  UINT32 fdd_scsi_command;
	address_space *m_mem;

	// functions
	UINT32 swap_uint32( UINT32 val );

	virtual void machine_reset() override;
};


UINT32 r9751_state::swap_uint32( UINT32 val )
{
	val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF );
	return (val << 16) | (val >> 16);
}

READ8_MEMBER(r9751_state::pdc_dma_r)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

WRITE8_MEMBER(r9751_state::pdc_dma_w)
{
	/* NOTE: This needs to be changed to a function that accepts an address and data */
	m_maincpu->space(AS_PROGRAM).write_byte(m_pdc->fdd_68k_dma_address,data);
}

DRIVER_INIT_MEMBER(r9751_state,r9751)
{
	reg_ff050004 = 0;
	reg_ff050320 = 1;
	reg_fff80040 = 0;
	fdd_dest_address = 0;
//  fdd_scsi_command = 0;
	fdd_cmd_complete = 0;
	smioc_out_addr = 0;
	m_mem = &m_maincpu->space(AS_PROGRAM);

}

void r9751_state::machine_reset()
{
	UINT8 *rom = memregion("prom")->base();
	UINT32 *ram = m_main_ram;

	memcpy(ram, rom, 8);

	m_maincpu->reset();
	m_pdc->reset();
}

/******************************************************************************
 External board communication registers [0x5FF00000 - 0x5FFFFFFF]
******************************************************************************/
READ32_MEMBER( r9751_state::r9751_mmio_5ff_r )
{
	UINT32 data;
	UINT32 address = offset * 4 + 0x5FF00000;

	switch(address)
	{
		/* PDC HDD region (0x24, device 9) */
		case 0x5FF00824: /* HDD Command result code */
			return 0x10;
			break;
		case 0x5FF03024: /* HDD SCSI command completed successfully */
			data = 0x1;
			if(TRACE_HDC) logerror("SCSI HDD command completion status - Read: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
			return data;
			break;
		/* SMIOC region (0x98, device 26) */
		case 0x5FF00898: /* Serial status or DMA status */
			return 0x40;
			break;
		/* PDC FDD region (0xB0, device 44 */
		case 0x5FF008B0: /* FDD Command result code */
			return 0x10;
			break;
		case 0x5FF010B0: /* Clear 5FF030B0 ?? */
			if(TRACE_FDC) logerror("--- FDD 0x5FF010B0 READ (0)\n");
			return 0;
		case 0x5FF030B0: /* FDD command completion status */
			data = (m_pdc->reg_p5 << 8) + m_pdc->reg_p4;
			if(TRACE_FDC) logerror("--- SCSI FDD command completion status - Read: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
			return data;
			break;
		default:
			if(TRACE_FDC || TRACE_HDC || TRACE_SMIOC) logerror("Instruction: %08x READ MMIO(%08x): %08x & %08x\n", space.machine().firstcpu->pc(), address, 0, mem_mask);
			return 0;
	}
}

WRITE32_MEMBER( r9751_state::r9751_mmio_5ff_w )
{
	UINT8 data_b0, data_b1;
	UINT32 address = offset * 4 + 0x5FF00000;

	switch(address)
	{
		/* PDC HDD region (0x24, device 9 */
		case 0x5FF00224: /* HDD SCSI read command */
			if(TRACE_HDC) logerror("@@@ HDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
			break;
		case 0x5FF08024: /* HDD SCSI read command */
			if(TRACE_HDC) logerror("@@@ HDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
			break;
		case 0x5FF0C024: /* HDD SCSI read command */
			if(TRACE_HDC) logerror("@@@ HDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
			break;
		/* SMIOC region (0x98, device 26) */
		case 0x5FF04098: /* Serial DMA Command */
			switch(data)
			{
				case 0x4100: /* Send byte to serial */
					if(TRACE_SMIOC) logerror("Serial byte: %02X\n", m_mem->read_dword(smioc_out_addr));
					m_terminal->write(space,0,m_mem->read_dword(smioc_out_addr));
					break;
				default:
					if(TRACE_SMIOC) logerror("Uknown serial DMA command: %X\n", data);
			}
			break;
		case 0x5FF0C098: /* Serial DMA output address */
			smioc_out_addr = data * 2;
			break;
		/* PDC FDD region (0xB0, device 44 */
		case 0x5FF001B0: /* FDD SCSI read command */
			if(TRACE_FDC) logerror("--- FDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
			break;
		case 0x5FF002B0: /* FDD SCSI read command */
			if(TRACE_FDC) logerror("--- FDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
			break;
		case 0x5FF008B0: /* FDD SCSI read command */
			if(TRACE_FDC) logerror("--- FDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
			break;
		case 0x5FF041B0: /* Unknown - Probably old style commands */
			if(TRACE_FDC) logerror("--- FDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);

			/* Clear FDD Command completion status 0x5FF030B0 (PDC 0x4, 0x5)*/
			m_pdc->reg_p4 = 0;
			m_pdc->reg_p5 = 0;

			data_b0 = data & 0xFF;
			data_b1 = (data & 0xFF00) >> 8;
			m_pdc->reg_p0 = data_b0;
			m_pdc->reg_p1 = data_b1;
			m_pdc->reg_p38 |= 0x2; /* Set bit 1 on port 38 register, PDC polls this port looking for a command */
			if(TRACE_FDC) logerror("--- FDD Old Command: %02X and %02X\n", data_b0, data_b1);
			break;
		case 0x5FF080B0: /* fdd_dest_address register */
			fdd_dest_address = data << 1;
			if(TRACE_FDC) logerror("--- FDD destination address: %08X\n", fdd_dest_address);
			data_b0 = data & 0xFF;
			data_b1 = (data & 0xFF00) >> 8;
			m_pdc->reg_p6 = data_b0;
			m_pdc->reg_p7 = data_b1;
			break;
		case 0x5FF0C0B0:
		case 0x5FF0C1B0: /* FDD command address register */
			UINT32 fdd_scsi_command;
			UINT32 fdd_scsi_command2;
			unsigned char c_fdd_scsi_command[8]; // Array for SCSI command
			int scsi_lba; // FDD LBA location here, extracted from command

			/* Clear FDD Command completion status 0x5FF030B0 (PDC 0x4, 0x5)*/
			m_pdc->reg_p4 = 0;
			m_pdc->reg_p5 = 0;

			/* Send FDD SCSI command location address to PDC 0x2, 0x3 */
			if(TRACE_FDC) logerror("--- FDD command address: %08X\n", data);
			data_b0 = data & 0xFF;
			data_b1 = (data & 0xFF00) >> 8;
			m_pdc->reg_p2 = data_b0;
			m_pdc->reg_p3 = data_b1;

			fdd_scsi_command = swap_uint32(m_mem->read_dword(data << 1));
			fdd_scsi_command2 = swap_uint32(m_mem->read_dword((data << 1)+4));

			memcpy(c_fdd_scsi_command,&fdd_scsi_command,4);
			memcpy(c_fdd_scsi_command+4,&fdd_scsi_command2,4);

			if(TRACE_FDC)
			{
				logerror("--- FDD SCSI Command: ");
				for(int i = 0; i < 8; i++)
					logerror("%02X ", c_fdd_scsi_command[i]);
				logerror("\n");
			}

			scsi_lba = c_fdd_scsi_command[3] | (c_fdd_scsi_command[2]<<8) | ((c_fdd_scsi_command[1]&0x1F)<<16);
			if(TRACE_FDC) logerror("--- FDD SCSI LBA: %i\n", scsi_lba);

			m_pdc->reg_p38 |= 0x2; // Set bit 1 on port 38 register, PDC polls this port looking for a command
			if(TRACE_FDC)logerror("--- FDD SET PDC Port 38: %X\n",m_pdc->reg_p38);
			break;

		default:
			if(TRACE_FDC || TRACE_HDC || TRACE_SMIOC) logerror("Instruction: %08x WRITE MMIO(%08x): %08x & %08x\n", space.machine().firstcpu->pc(), address, data, mem_mask);
	}
}

/******************************************************************************
 CPU board registers [0xFF050000 - 0xFF06FFFF]
******************************************************************************/
READ32_MEMBER( r9751_state::r9751_mmio_ff05_r )
{
	UINT32 data;
		UINT32 address = offset * 4 + 0xFF050000;

	switch(address)
	{
		case 0xFF050004:
			return reg_ff050004;
			break;
		case 0xFF050300:
			return 0x1B | (1<<0x14);
			break;
		case 0xFF050320: /* Some type of counter */
			reg_ff050320++;
			return reg_ff050320;
			break;
		case 0xFF050584:
			return 0;
			break;
		case 0xFF050610:
			return 0xabacabac;
			break;
		case 0xFF060014:
			return 0x80;
			break;
		default:
			data = 0;
			if(TRACE_CPU_REG) logerror("Instruction: %08x READ MMIO(%08x): %08x & %08x\n", space.machine().firstcpu->pc(), address, data, mem_mask);
			return data;
	}
}

WRITE32_MEMBER( r9751_state::r9751_mmio_ff05_w )
{
	UINT32 address = offset * 4 + 0xFF050000;

	switch(address)
	{
		case 0xFF050004:
			reg_ff050004 = data;
			return;
			break;
		case 0xFF05000C: /* CPU LED hex display indicator */
			if(TRACE_LED) logerror("\n*** LED: %02x, Instruction: %08x ***\n\n", data, space.machine().firstcpu->pc());
			return;
			break;
		default:
			if(TRACE_CPU_REG) logerror("Instruction: %08x WRITE MMIO(%08x): %08x & %08x\n", space.machine().firstcpu->pc(), address, data, mem_mask);
			return;
	}
}

READ32_MEMBER( r9751_state::r9751_mmio_fff8_r )
{
		UINT32 data;
		UINT32 address = offset * 4 + 0xFFF80000;

	switch(address)
	{
		case 0xFFF80040:
			return reg_fff80040;
			break;
		default:
			data = 0;
			if(TRACE_CPU_REG) logerror("Instruction: %08x READ MMIO(%08x): %08x & %08x\n", space.machine().firstcpu->pc(), address, data, mem_mask);
			return data;
	}
}

WRITE32_MEMBER( r9751_state::r9751_mmio_fff8_w )
{
		UINT32 address = offset * 4 + 0xFFF80000;

	switch(address)
	{
		case 0xFFF80040:
			reg_fff80040 = data;
			return;
			break;
		default:
			if(TRACE_CPU_REG) logerror("Instruction: %08x WRITE MMIO(%08x): %08x & %08x\n", space.machine().firstcpu->pc(), address, data, mem_mask);
	}
}

/******************************************************************************
 Address Maps
******************************************************************************/

static ADDRESS_MAP_START(r9751_mem, AS_PROGRAM, 32, r9751_state)
	//ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000,0x00ffffff) AM_RAM AM_SHARE("main_ram") // 16MB
	//AM_RANGE(0x01000000,0x07ffffff) AM_NOP
	AM_RANGE(0x08000000,0x0800ffff) AM_ROM AM_REGION("prom", 0)
		AM_RANGE(0x5FF00000,0x5FFFFFFF) AM_READWRITE(r9751_mmio_5ff_r, r9751_mmio_5ff_w)
		AM_RANGE(0xFF050000,0xFF06FFFF) AM_READWRITE(r9751_mmio_ff05_r, r9751_mmio_ff05_w)
	AM_RANGE(0xFFF80000,0xFFF8FFFF) AM_READWRITE(r9751_mmio_fff8_r, r9751_mmio_fff8_w)
	//AM_RANGE(0xffffff00,0xffffffff) AM_RAM // Unknown area
ADDRESS_MAP_END

/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( r9751 )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( r9751, r9751_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68030, 20000000)
	MCFG_CPU_PROGRAM_MAP(r9751_mem)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)

	/* disk hardware */
	MCFG_DEVICE_ADD("pdc", PDC, 0)
	MCFG_PDC_R_CB(READ8(r9751_state, pdc_dma_r))
	MCFG_PDC_W_CB(WRITE8(r9751_state, pdc_dma_w))
	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	//MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "cdrom", SCSICD, SCSI_ID_1)
	MCFG_DEVICE_ADD("wd33c93", WD33C93, 0)
	MCFG_LEGACY_SCSI_PORT("scsi")
	//MCFG_WD33C93_IRQ_CB(WRITELINE(r9751_state,scsi_irq))

	/* software list */
	MCFG_SOFTWARE_LIST_ADD("flop_list","r9751")
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(r9751)
	ROM_REGION32_BE(0x00010000, "prom", 0)
	ROM_SYSTEM_BIOS(0, "prom34",  "PROM Version 3.4")
	ROMX_LOAD( "p-n_98d4643__abaco_v3.4__(49fe7a)__j221.27512.bin", 0x0000, 0x10000, CRC(9fb19a85) SHA1(c861e15a2fc9a4ef689c2034c53fbb36f17f7da6), ROM_GROUPWORD | ROM_BIOS(1) ) // Label: "P/N 98D4643 // ABACO V3.4 // (49FE7A) // J221" 27128 @Unknown
ROM_END



/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT      COMPANY                     FULLNAME                                                    FLAGS */
COMP( 1988, r9751,   0,          0,      r9751,   r9751, r9751_state, r9751,      "ROLM Systems, Inc.",   "ROLM 9751 Model 10", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
