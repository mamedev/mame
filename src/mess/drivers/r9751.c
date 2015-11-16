// license:GPL-2.0+
// copyright-holders:Brandon Munger
/******************************************************************************
*
* Rolm CBX 9751 Release 9005 Driver
* 
* This driver attempts to emulate the following models:
* * Model 10
* * Model 20
* * Model 40
* * Model 50
* * Model 70
*
* The basis of this driver was influenced by the zexall.c driver by
* Jonathan Gevaryahu and Robbbert.
*
*
* Special Thanks to:
* * Stephen Stair (sgstair)   - for help with reverse engineering,
*				programming, and emulation
* * Felipe Sanches (FSanches) - for help with MESS and emulation
* * Michael Zapf (mizapf)     - for building the HDC9234/HDC9224
				driver that makes this driver possible
*
* Memory map:
* * 0x00000000 - 0x00ffffff : RAM? 16MB, up to 128MB?
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

#define TERMINAL_TAG "terminal"

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

	DECLARE_DRIVER_INIT(r9751);

	DECLARE_FLOPPY_FORMATS( floppy_formats );
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

//	UINT32 fdd_scsi_command;
	address_space *m_mem;

	// functions
	UINT32 swap_uint32( UINT32 val );
	bool fd_read(int LBA, unsigned char* buffer, int bufferLength);

	virtual void machine_reset();
};

bool r9751_state::fd_read(int LBA, unsigned char* buffer, int bufferLength)
{
	if (bufferLength < 512)
		return false;
//	std::vector<UINT32> *ptr_fdd_vector = &m_floppy->get_buffer();
//	std::vector<UINT32> *ptr_vector = &m_floppy->image->get_buffer(0,0);
	//UINT32 f_start = 0x09000000;
	UINT8 *f_hack = memregion("floppy_hack")->base();
	memcpy(buffer,f_hack+(LBA*512),bufferLength);

//	for(int i=0;i<bufferLength/4; i++)
//	{
//		buffer[i*4] = (*ptr_vector)[i] & 255;
//		buffer[i*4+1] = ((*ptr_vector)[i]>>8)&255;
//		buffer[i*4+2] = ((*ptr_vector)[i]>>16)&255;
//		buffer[i*4+3] = ((*ptr_vector)[i]>>24)&255;
//	}
	return true;
}

UINT32 r9751_state::swap_uint32( UINT32 val )
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | (val >> 16);
}

DRIVER_INIT_MEMBER(r9751_state,r9751)
{
	reg_ff050004 = 0;
	reg_ff050320 = 1;
	reg_fff80040 = 0;
	fdd_dest_address = 0;
//	fdd_scsi_command = 0;
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

READ32_MEMBER( r9751_state::r9751_mmio_5ff_r )
{
	UINT32 data;
	UINT32 address = offset * 4 + 0x5FF00000;
        if(address == 0x5FF00824)
                data = 0x10; // FDD Command result code
	else if(address == 0x5FF00898) // Serial status or DMA status
		data = 0x40;
        else if(address == 0x5FF008B0)
                data = 0x10; // HDD Command result code
	else if(address == 0x5FF03024) // HDD Command status
	{
		data = 0x1; // HDD SCSI command completed successfully
		logerror("SCSI HDD command completion status - Read: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
		return data;
	}
	else if(address == 0x5FF030B0) // FDD Command status
	{
		//data = 0x1; // FDD SCSI command completed successfully
		logerror("--- SCSI FDD command completion status - Read: %08X, From: %08X, Register: %08X\n", fdd_cmd_complete, space.machine().firstcpu->pc(), address);
		data = fdd_cmd_complete;
		fdd_cmd_complete = 0;

		return data;
	}
	else
		data = 0x00000000;
	logerror("Instruction: %08x READ MMIO(%08x): %08x & %08x\n", space.machine().firstcpu->pc(), address, data, mem_mask);
	return data;
}

WRITE32_MEMBER( r9751_state::r9751_mmio_5ff_w )
{
	UINT32 address = offset * 4 + 0x5FF00000;
	if(address == 0x5FF00224) // HDD SCSI read command
		logerror("@@@ HDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
	else if(address == 0x5FF04098) // Serial DMA Command
	{
		if(data == 0x4100) // Send byte to serial
		{
			logerror("Serial byte: %02X\n", m_mem->read_dword(smioc_out_addr));
			m_terminal->write(space,0,m_mem->read_dword(smioc_out_addr));
		}
	}
	else if(address == 0x5FF041B0) // Unknown - Probably old style commands
	{
		logerror("--- FDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
		//if(data == 0x7000 || data == 0x6800) // Unknown
		//	fdd_cmd_complete = 1;
		UINT8 data_b0;
		UINT8 data_b1;
		data_b0 = data & 0xFF;
		data_b1 = (data & 0xFF00) >> 8;
		logerror("Test: %02X and %02X\n", data_b0, data_b1);
	}
	else if(address == 0x5FF08024) // HDD SCSI read command
		logerror("@@@ HDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
	else if(address == 0x5FF0C024) // HDD SCSI read command
		logerror("@@@ HDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);

	else if(address == 0x5FF001B0) // FDD SCSI read command
	{
		logerror("--- FDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
		//fdd_cmd_complete = 1;
	}
	else if(address == 0x5FF002B0) // FDD SCSI read command
	{
		logerror("--- FDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
		//fdd_cmd_complete = 1;
	}
	else if(address == 0x5FF008B0) // FDD SCSI read command
	{
		logerror("--- FDD Command: %08X, From: %08X, Register: %08X\n", data, space.machine().firstcpu->pc(), address);
		//fdd_cmd_complete = 1;
	}
	else if(address == 0x5FF080B0) // fdd_dest_address register
	{
		fdd_dest_address = data << 1;
		logerror("FDD destination address: %08X\n", fdd_dest_address);
		//fdd_cmd_complete = 1;
	}
        else if(address == 0x5FF0C098) // Serial DMA output address
                smioc_out_addr = data * 2;
	else if((address == 0x5FF0C0B0) || (address == 0x5FF0C1B0)) // FDD Command address written to this memory location
	{
		UINT32 fdd_scsi_command;
		UINT32 fdd_scsi_command2;
		//UINT8 pdc_p38;

		unsigned char c_fdd_scsi_command[8]; // Array for SCSI command
		//unsigned char fdd_buffer[512];
		int scsi_lba; // FDD LBA location here, extracted from command
		
	//	fdd_scsi_command = swap_uint32(m_mem->read_dword(fdd_dest_address));
	//	fdd_scsi_command2 = swap_uint32(m_mem->read_dword(fdd_dest_address+4));
                fdd_scsi_command = swap_uint32(m_mem->read_dword(data << 1));
                fdd_scsi_command2 = swap_uint32(m_mem->read_dword((data << 1)+4));
		
		memcpy(c_fdd_scsi_command,&fdd_scsi_command,4);
		memcpy(c_fdd_scsi_command+4,&fdd_scsi_command2,4);

		logerror("FDD SCSI Command: ");
		for(int i = 0; i < 8; i++)
			logerror("%02X ", c_fdd_scsi_command[i]);
		logerror("\n");
		
		scsi_lba = c_fdd_scsi_command[3] | (c_fdd_scsi_command[2]<<8) | ((c_fdd_scsi_command[1]&0x1F)<<16);
		logerror("FDD SCSI LBA: %i\n", scsi_lba);

		//pdc_p38 = m_pdc->p38_r(space,0);
		m_pdc->reg_p38 |= 0x2; // Set bit 1 on port 38 register, PDC polls this port looking for a command
		//pdc_p38 = m_pdc->reg_p38;
		logerror("pdc_p38: %X\n",m_pdc->reg_p38);
		//m_pdc->p38_w(space,0,0x2); // Set bit 1 on port 38 register.  PDC polls register at this location looking for command
/*
		switch(c_fdd_scsi_command[0])
		{
			case 8:
			{
				scsi_lba = c_fdd_scsi_command[3] | (c_fdd_scsi_command[2]<<8) | ((c_fdd_scsi_command[1]&0x1F)<<16);
				unsigned char fdd_buffer[c_fdd_scsi_command[4]*512];
				logerror("FDD: SCSI READ LBA: %i\n", scsi_lba);
				fd_read(scsi_lba,fdd_buffer,sizeof(fdd_buffer));
				logerror("Bytes: \n");
				for (int i = 0; i < sizeof(fdd_buffer); i++)
				{
					logerror(" %02X",fdd_buffer[i]);
					if (i % 16 == 15)
						logerror("\n");
				}
			
				for (int i = 0; i < sizeof(fdd_buffer); i++)
				{
					m_mem->write_byte(fdd_dest_address+i, fdd_buffer[i]);
				}
				break;
			}
			default:
				logerror("FDD: Unknown SCSI command\n");
				break;
		}
*/
	}	
	else
		logerror("Instruction: %08x WRITE MMIO(%08x): %08x & %08x\n", space.machine().firstcpu->pc(), address, data, mem_mask);
	return;
}

READ32_MEMBER( r9751_state::r9751_mmio_ff05_r ) //0xFF050000 - 0xFF06FFFF
{
	UINT32 data;
        UINT32 address = offset * 4 + 0xFF050000;
	if(address == 0xFF050610)
		data = 0xabacabac;
	else if(address == 0xFF050004)
		data = reg_ff050004;
	else if(address == 0xFF050300)
		data = 0x1B | (1<<0x14);
	else if(address == 0xFF050320) // Some type of counter
	{	
		//data = 0x1;
		reg_ff050320++;
		data = reg_ff050320;
	}
	else if(address == 0xFF050584)
		return 0x0;
	else if(address == 0xFF060014)
		data = 0x80;
	else
		data = 0x00000000;
        logerror("Instruction: %08x READ MMIO(%08x): %08x & %08x\n", space.machine().firstcpu->pc(), address, data, mem_mask);
        return data;
}

WRITE32_MEMBER( r9751_state::r9751_mmio_ff05_w )
{
	UINT32 address = offset * 4 + 0xFF050000;
	if(address == 0xFF050004)
		reg_ff050004 = data;
	else if(address == 0xFF05000C) // LED hex display indicator
	{
		logerror("\n*** LED: %02x, Instruction: %08x ***\n\n", data, space.machine().firstcpu->pc());
/*		char buf[10];
		char *buf2;
		sprintf(buf,"%02X",data);
		buf2 = buf;
		while(*buf2)
		{
			m_terminal->write(space,0,*buf2);
			buf2++;
		}
		m_terminal->write(space,0,0x0a);
*/
		return;
	}
	logerror("Instruction: %08x WRITE MMIO(%08x): %08x & %08x\n", space.machine().firstcpu->pc(), address, data, mem_mask);
        return;
}

READ32_MEMBER( r9751_state::r9751_mmio_fff8_r )
{
        UINT32 data;
        UINT32 address = offset * 4 + 0xFFF80000;
	if(address == 0xFFF80040)
		data = reg_fff80040;
	else
		data = 0x00000000;

        logerror("Instruction: %08x READ MMIO(%08x): %08x & %08x\n", space.machine().firstcpu->pc(), address, data, mem_mask);
        return data;
}

WRITE32_MEMBER( r9751_state::r9751_mmio_fff8_w )
{
        UINT32 address = offset * 4 + 0xFFF80000;
	if(address == 0xFFF80040)
		reg_fff80040 = data;

        logerror("Instruction: %08x WRITE MMIO(%08x): %08x & %08x\n", space.machine().firstcpu->pc(), address, data, mem_mask);
        return;
}

/******************************************************************************
 Address Maps
******************************************************************************/

static ADDRESS_MAP_START(r9751_mem, AS_PROGRAM, 32, r9751_state)
	//ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000,0x00ffffff) AM_RAM AM_SHARE("main_ram") // 16MB
	//AM_RANGE(0x01000000,0x07ffffff) AM_NOP
	AM_RANGE(0x08000000,0x0800ffff) AM_ROM AM_REGION("prom", 0)
	AM_RANGE(0x09000000,0x09167fff) AM_ROM AM_REGION("floppy_hack",0)
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
	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	//MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "cdrom", SCSICD, SCSI_ID_1)
	MCFG_DEVICE_ADD("wd33c93", WD33C93, 0)
	MCFG_LEGACY_SCSI_PORT("scsi")
	//MCFG_WD33C93_IRQ_CB(WRITELINE(r9751_state,scsi_irq))
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(r9751)
	ROM_REGION32_BE(0x00010000, "prom", 0)
	ROM_SYSTEM_BIOS(0, "prom34",  "PROM Version 3.4")
	ROMX_LOAD( "p-n_98d4643__abaco_v3.4__(49fe7a)__j221.27512.bin", 0x0000, 0x10000, CRC(9fb19a85) SHA1(c861e15a2fc9a4ef689c2034c53fbb36f17f7da6), ROM_GROUPWORD | ROM_BIOS(1) ) // Label: "P/N 98D4643 // ABACO V3.4 // (49FE7A) // J221" 27128 @Unknown
	
	ROM_REGION(0x00168000, "floppy_hack",0)
	ROMX_LOAD( "floppy1.img", 0x0000, 0x00168000, CRC(b5d349b5) SHA1(9e7e4f13723f54eed7bcbabd6c4fe5670ee1aa96), ROM_GROUPWORD)
ROM_END



/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT      COMPANY                     FULLNAME                                                    FLAGS */
COMP( 1988, r9751,   0,          0,      r9751,   r9751, r9751_state, r9751,      "ROLM Systems, Inc.",   "ROLM 9751 Release 9005", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
