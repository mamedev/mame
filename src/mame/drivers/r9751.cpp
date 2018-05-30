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
* * 0x5ff00000 - 0x5fffffff : System boards
* * 0xff010000 - 0xfff8ffff : CPU board registers
*
* Working:
* * Floppy Disk IO (PDC device)
* * SMIOC terminal (preliminary)
*
* TODO:
* * Identify registers required for OS booting
* * Hard disk support
* * SMIOC ports (1-8)
* * Identify various LED registers for system boards
* * ROLMLink (RLI) board support
* * Analog Telephone Interface (ATI) board support
* * T1 (T1DN) board support
*
******************************************************************************/

/* Core includes */
#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/terminal.h"

#include "bus/scsi/scsi.h"
#include "machine/wd33c93.h"

#include "machine/pdc.h"
#include "machine/smioc.h"
#include "softlist.h"

#include <queue>

#define TERMINAL_TAG "terminal"

/* Log defines */
#define TRACE_FDC 0
#define TRACE_HDC 0
#define TRACE_SMIOC 0
#define TRACE_CPU_REG 0
#define TRACE_LED 0
#define TRACE_DMA 0

#define TRACE_DEVICE 0x0

#define ENABLE_TRACE_SMIOC 0

#if ENABLE_TRACE_SMIOC
#define TRACE_SMIOC_READ(address, data, reg, text) UnifiedTrace((address),(data)," Read", "SMIOC", (reg), (text))
#define TRACE_SMIOC_WRITE(address, data, reg, text) UnifiedTrace((address),(data),"Write", "SMIOC", (reg), (text))
#else
#define TRACE_SMIOC_READ(address, data, reg, text) do {} while (0)
#define TRACE_SMIOC_WRITE(address, data, reg, text) do {} while (0)
#endif

class r9751_state : public driver_device
{
public:
	r9751_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pdc(*this, "pdc"),
		m_smioc(*this, "smioc"),
		m_wd33c93(*this, "wd33c93"),
		m_terminal(*this, TERMINAL_TAG),
		m_main_ram(*this, "main_ram")
	{
	}

	void kbd_put(u8 data);

	DECLARE_READ32_MEMBER(r9751_mmio_5ff_r);
	DECLARE_WRITE32_MEMBER(r9751_mmio_5ff_w);
	DECLARE_READ32_MEMBER(r9751_mmio_ff01_r);
	DECLARE_WRITE32_MEMBER(r9751_mmio_ff01_w);
	DECLARE_READ32_MEMBER(r9751_mmio_ff05_r);
	DECLARE_WRITE32_MEMBER(r9751_mmio_ff05_w);
	DECLARE_READ32_MEMBER(r9751_mmio_fff8_r);
	DECLARE_WRITE32_MEMBER(r9751_mmio_fff8_w);

	DECLARE_READ8_MEMBER(pdc_dma_r);
	DECLARE_WRITE8_MEMBER(pdc_dma_w);

	void init_r9751();

	void r9751(machine_config &config);
	void r9751_mem(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_device<pdc_device> m_pdc;
	required_device<smioc_device> m_smioc;
	required_device<wd33c93_device> m_wd33c93;
	required_device<generic_terminal_device> m_terminal;
	required_shared_ptr<uint32_t> m_main_ram;

	m68000_base_device* ptr_m68000;

	// Begin registers
	uint32_t reg_ff050004;
	uint32_t reg_fff80040;
	uint32_t fdd_dest_address; // 5FF080B0
	uint32_t fdd_cmd_complete;
	uint32_t smioc_out_addr;
	uint32_t smioc_in_addr;
	uint32_t smioc_dma_bank;
	uint32_t smioc_dma_w_length;
	uint32_t smioc_dma_r_length;
	uint32_t fdd_dma_bank;
	attotime timer_32khz_last;
	uint16_t m_serial_status2;
	std::queue<uint8_t> kbd_queue;
	std::queue<uint16_t> serial_status_queue;
	bool serial_input;
	// End registers

	address_space *m_mem;

	// functions
	uint32_t swap_uint32( uint32_t val );
	uint32_t debug_a6();
	uint32_t debug_a5();
	uint32_t debug_a5_20();
	void UnifiedTrace(u32 address, u32 data, const char* operation="Read", const char* Device="SMIOC", const char* RegisterName=nullptr, const char* extraText=nullptr);

	virtual void machine_reset() override;
};

void r9751_state::kbd_put(u8 data)
{
	if(serial_input)
	{
		m_maincpu->space(AS_PROGRAM).write_byte(smioc_in_addr,data);
		serial_status_queue.push(0x5440);
		serial_input = false;
	}
	else
	{
		kbd_queue.push(data);
	}
}

uint32_t r9751_state::swap_uint32( uint32_t val )
{
	val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF );
	return (val << 16) | (val >> 16);
}

uint32_t r9751_state::debug_a6()
{
	return m_maincpu->space(AS_PROGRAM).read_dword(ptr_m68000->state_int(M68K_A6) + 4);
}

uint32_t r9751_state::debug_a5()
{
	return m_maincpu->space(AS_PROGRAM).read_dword(ptr_m68000->state_int(M68K_A5));
}

uint32_t r9751_state::debug_a5_20()
{
	return m_maincpu->space(AS_PROGRAM).read_dword(ptr_m68000->state_int(M68K_A5) + 0x20);
}

void r9751_state::UnifiedTrace(u32 address, u32 data, const char* operation, const char* Device, const char* RegisterName, const char* extraText)
{
	u32 stacktrace[4];
	u32 basepointer[2];
	u32 reg_a6 = ptr_m68000->state_int(M68K_A6);

	for(int i=0; i<4; i++)
		stacktrace[i] = 0;
	for(int i=0; i<2; i++)
		basepointer[i] = 0;

	stacktrace[0] = m_maincpu->pc();
	if(reg_a6 + 4 < 0xFFFFFF) stacktrace[1] = m_maincpu->space(AS_PROGRAM).read_dword(reg_a6 + 4);
	if(reg_a6 < 0xFFFFFF && reg_a6 != 0) basepointer[0] = m_maincpu->space(AS_PROGRAM).read_dword(reg_a6);
	if(basepointer[0] + 4 < 0xFFFFFF && basepointer[0] != 0) stacktrace[2] = m_maincpu->space(AS_PROGRAM).read_dword(basepointer[0] + 4);
	if(basepointer[0] < 0xFFFFFF && basepointer[0] != 0) basepointer[1] = m_maincpu->space(AS_PROGRAM).read_dword(basepointer[0]);
	if(basepointer[1] + 4 < 0xFFFFFF && basepointer[1] != 0) stacktrace[3] = m_maincpu->space(AS_PROGRAM).read_dword(basepointer[1] + 4);

	logerror("%s[%08X] => %08X (%s:%s) %s (%08X, %08X, %08X, %08X)\n", operation, address, data, Device, RegisterName==nullptr?"":RegisterName, extraText==nullptr?"":extraText, stacktrace[0], stacktrace[1], stacktrace[2], stacktrace[3]);
}

READ8_MEMBER(r9751_state::pdc_dma_r)
{
	/* This callback function takes the value written to 0xFF01000C as the bank offset */
	uint32_t address = (fdd_dma_bank & 0x7FFFF800) + (offset&0x3FFFF);
	if(TRACE_DMA) logerror("DMA READ: %08X DATA: %08X\n", address, m_maincpu->space(AS_PROGRAM).read_byte(address));
	return m_maincpu->space(AS_PROGRAM).read_byte(address);
}

WRITE8_MEMBER(r9751_state::pdc_dma_w)
{
	/* This callback function takes the value written to 0xFF01000C as the bank offset */
	uint32_t address = (fdd_dma_bank & 0x7FFFF800) + (m_pdc->fdd_68k_dma_address&0x3FFFF);
	m_maincpu->space(AS_PROGRAM).write_byte(address,data);
	if(TRACE_DMA) logerror("DMA WRITE: %08X DATA: %08X\n", address,data);
}

void r9751_state::init_r9751()
{
	reg_ff050004 = 0;
	reg_fff80040 = 0;
	fdd_dest_address = 0;
	fdd_cmd_complete = 0;
	fdd_dma_bank = 0;
	smioc_out_addr = 0;
	smioc_dma_bank = 0;
	smioc_dma_w_length = 0;

	m_mem = &m_maincpu->space(AS_PROGRAM);

	m_maincpu->interface<m68000_base_device>(ptr_m68000);

	/* Save states */
	save_item(NAME(reg_ff050004));
	save_item(NAME(reg_fff80040));
	save_item(NAME(fdd_dest_address));
	save_item(NAME(fdd_cmd_complete));
	save_item(NAME(smioc_out_addr));
	save_item(NAME(smioc_dma_bank));
	save_item(NAME(fdd_dma_bank));
	save_item(NAME(timer_32khz_last));

}

void r9751_state::machine_reset()
{
	uint8_t *rom = memregion("prom")->base();
	uint32_t *ram = m_main_ram;

	memcpy(ram, rom, 8);

	m_maincpu->reset();
	m_pdc->reset();
}

/******************************************************************************
 External board communication registers [0x5FF00000 - 0x5FFFFFFF]
******************************************************************************/
READ32_MEMBER( r9751_state::r9751_mmio_5ff_r )
{
	uint32_t data;

	if(TRACE_DEVICE)
		if(((offset << 2) & 0xFF) == TRACE_DEVICE * 4)
			logerror("(!) Device Read: 0x%02X - PC: %08X Register: %08X\n", TRACE_DEVICE, m_maincpu->pc(), offset << 2 | 0x5FF00000);

	switch(offset << 2)
	{
		/* PDC HDD region (0x24, device 9) */
		case 0x0824: /* HDD Command result code */
			return 0x10;
		case 0x3024: /* HDD SCSI command completed successfully */
			data = 0x1;
			if(TRACE_HDC) logerror("SCSI HDD command completion status - Read: %08X, From: %08X, Register: %08X\n", data, m_maincpu->pc(), offset << 2 | 0x5FF00000);
			return data;
		/* SMIOC region (0x98, device 26) */
		case 0x0898: /* Serial status or DMA status */
			if(!serial_status_queue.empty())
			{
				data = serial_status_queue.front();
				serial_status_queue.pop();
			}
			else
			{
				data = 0;
			}
			if(TRACE_SMIOC) logerror("serial_status_queue = %04X \n", data | 0x8);
			TRACE_SMIOC_READ(offset << 2 | 0x5FF00000, data | 0x8, "Serial Status 1",  nullptr);
			return data | 0x8;
		case 0x0870: /* Serial status or DMA status 2 */
			if(TRACE_SMIOC) logerror("m_serial_status2 = %04X \n", m_serial_status2);
			TRACE_SMIOC_READ(offset << 2 | 0x5FF00000, m_serial_status2, "Serial Status 2", nullptr);
			return m_serial_status2;
		/* SMIOC region (0x9C, device 27) */

		/* PDC FDD region (0xB0, device 44 */
		case 0x08B0: /* FDD Command result code */
			return 0x10;
		case 0x10B0: /* Clear 5FF030B0 ?? */
			if(TRACE_FDC) logerror("--- FDD 0x5FF010B0 READ (0)\n");
			return 0;
		case 0x30B0: /* FDD command completion status */
			data = (m_pdc->reg_p5 << 8) + m_pdc->reg_p4;
			if(TRACE_FDC && data != 0) logerror("--- SCSI FDD command completion status - Read: %08X, From: %08X, Register: %08X\n", data, m_maincpu->pc(), offset << 2 | 0x5FF00000);
			return data;
		default:
			if(TRACE_FDC || TRACE_HDC || TRACE_SMIOC) logerror("Unknown read address: %08X PC: %08X\n", offset << 2 | 0x5FF00000, m_maincpu->pc());
			if(((offset << 2) & 0xFF) == 0x26 * 4 || ((offset << 2) & 0xFF) == 0x27 * 4) TRACE_SMIOC_READ(offset << 2 | 0x5FF00000, 0, "Unknown", nullptr);
			return 0;
	}
}

WRITE32_MEMBER( r9751_state::r9751_mmio_5ff_w )
{
	uint8_t data_b0, data_b1;

	//logerror("(!!) 0x5ff Register Write - PC: %08X Register: %08X Data: %08X\n", m_maincpu->pc(), offset << 2 | 0x5FF00000, data);

	if(TRACE_DEVICE)
		if(((offset << 2) & 0xFF) == TRACE_DEVICE * 4)
			logerror("(!) Device Write: 0x%02X - PC: %08X Register: %08X Data: %08X\n", TRACE_DEVICE, m_maincpu->pc(), offset << 2 | 0x5FF00000, data);

	/* Unknown mask */
	if (mem_mask != 0xFFFFFFFF)
		logerror("Mask found: %08X Register: %08X PC: %08X\n", mem_mask, offset << 2 | 0x5FF00000, m_maincpu->pc());

	switch(offset << 2)
	{
		/* PDC HDD region (0x24, device 9 */
		case 0x0224: /* HDD SCSI read command */
			if(TRACE_HDC) logerror("@@@ HDD Command: %08X, From: %08X, Register: %08X\n", data, m_maincpu->pc(), offset << 2 | 0x5FF00000);
			break;
		case 0x8024: /* HDD SCSI read command */
			if(TRACE_HDC) logerror("@@@ HDD Command: %08X, From: %08X, Register: %08X\n", data, m_maincpu->pc(), offset << 2 | 0x5FF00000);
			break;
		case 0xC024: /* HDD SCSI read command */
			if(TRACE_HDC) logerror("@@@ HDD Command: %08X, From: %08X, Register: %08X\n", data, m_maincpu->pc(), offset << 2 | 0x5FF00000);
			break;
		/* SMIOC region (0x98, device 26) - Output */
		case 0x0298:
			if(!serial_status_queue.empty())
				serial_status_queue.pop();
			if(TRACE_SMIOC) logerror("Serial status: %08X PC: %08X\n", data, m_maincpu->pc());
			TRACE_SMIOC_WRITE(offset << 2 | 0x5FF00000, data, "Serial Status 1", nullptr);
			break;
		case 0x0270:
			m_serial_status2 = data;
			if(TRACE_SMIOC) logerror("Serial status2: %08X PC: %08X\n", data, m_maincpu->pc());
			TRACE_SMIOC_WRITE(offset << 2 | 0x5FF00000, data, "Serial Status 2", nullptr);
			break;
		case 0x4090:
		case 0x4098: /* Serial DMA Command */
			switch(data)
			{
				case 0x1000:
					serial_status_queue.push(0x0140);
					m_serial_status2 = 0x0140;
					if(TRACE_SMIOC) logerror("Serial DMA command 0x1000 PC: %08X\n", m_maincpu->pc());
					break;
				case 0x4100: /* Send byte to serial */
					for(int i = 0; i < smioc_dma_w_length; i++)
					{
						if(TRACE_SMIOC) logerror("Serial byte: %02X PC: %08X\n", m_mem->read_word(smioc_out_addr+i*2), m_maincpu->pc());
						m_terminal->write(space,0,m_mem->read_word(smioc_out_addr+i*2));
					}
					serial_status_queue.push(0x5140);
					break;
				case 0x4200: /* Serial input */
					if(kbd_queue.empty())
					{
						serial_input = true;
						break;
					}

					for(int i = 0; i < smioc_dma_r_length && !kbd_queue.empty(); i++)
					{
						m_maincpu->space(AS_PROGRAM).write_byte(smioc_in_addr+i,kbd_queue.front());
						kbd_queue.pop();
					}

					serial_status_queue.push(0xE440);
					break;
				default:
					if(TRACE_SMIOC) logerror("Unknown serial DMA command: %X\n", data);
					serial_status_queue.push(0x40);
			}
			TRACE_SMIOC_WRITE(offset << 2 | 0x5FF00000, data, "Serial Command", nullptr);
			break;
		case 0xC098: /* Serial DMA output address */
			smioc_out_addr = (smioc_dma_bank & 0x7FFFF800) | ((data&0x3FF)<<1);
			if(TRACE_SMIOC) logerror("Serial output address: %08X PC: %08X\n", smioc_out_addr, m_maincpu->pc());
			TRACE_SMIOC_WRITE(offset << 2 | 0x5FF00000, data, "Serial Out Address", nullptr);
			break;
		/* SMIOC region (0x9C, device 27) - Input */
		case 0x409C: /* Serial DMA write length */
			smioc_dma_w_length = (~data+1) & 0xFFFF;
			if(TRACE_SMIOC) logerror("Serial DMA write length: %08X PC: %08X\n", smioc_dma_w_length, m_maincpu->pc());
			if(smioc_dma_w_length > 0x400) smioc_dma_w_length = 0x400;
			TRACE_SMIOC_WRITE(offset << 2 | 0x5FF00000, data, "Serial Write Length", nullptr);
			break;
		case 0x809C: /* Serial DMA input address */
			smioc_in_addr = (smioc_dma_bank & 0x7FFFF800) | ((data&0x3FF)<<1);
			if(TRACE_SMIOC) logerror("Serial input address: %08X PC: %08X\n", smioc_out_addr, m_maincpu->pc());
			TRACE_SMIOC_WRITE(offset << 2 | 0x5FF00000, data, "Serial In Address", nullptr);
			break;
		case 0xC09C: /* Serial DMA read length */
			smioc_dma_r_length = (~data+1) & 0xFFFF;
			if(TRACE_SMIOC) logerror("Serial DMA read length: %08X PC: %08X\n", smioc_dma_r_length, m_maincpu->pc());
			if(smioc_dma_r_length > 0x400) smioc_dma_r_length = 0x400;
			TRACE_SMIOC_WRITE(offset << 2 | 0x5FF00000, data, "Serial Read Length", nullptr);
			break;
		/* PDC FDD region (0xB0, device 44 */
		case 0x01B0: /* FDD SCSI read command */
			if(TRACE_FDC) logerror("--- FDD Command: %08X, From: %08X, Register: %08X\n", data, m_maincpu->pc(), offset << 2 | 0x5FF00000);
			break;
		case 0x02B0: /* FDD SCSI read command */
			if(TRACE_FDC) logerror("--- FDD Command: %08X, From: %08X, Register: %08X\n", data, m_maincpu->pc(), offset << 2 | 0x5FF00000);
			break;
		case 0x04B0: /* FDD RESET PDC */
			if(TRACE_FDC) logerror("PDC RESET, PC: %08X DATA: %08X\n", m_maincpu->pc(), data);
			m_pdc->reset();
			break;
		case 0x08B0: /* FDD SCSI read command */
			if(TRACE_FDC) logerror("--- FDD Command: %08X, From: %08X, Register: %08X\n", data, m_maincpu->pc(), offset << 2 | 0x5FF00000);
			break;
		case 0x41B0: /* Unknown - Probably old style commands */
			if(TRACE_FDC) logerror("--- FDD Command: %08X, From: %08X, Register: %08X\n", data, m_maincpu->pc(), offset << 2 | 0x5FF00000);

			/* Clear FDD Command completion status 0x5FF030B0 (PDC 0x4, 0x5) */
			m_pdc->reg_p4 = 0;
			m_pdc->reg_p5 = 0;

			data_b0 = data & 0xFF;
			data_b1 = (data & 0xFF00) >> 8;
			m_pdc->reg_p0 = data_b0;
			m_pdc->reg_p1 = data_b1;
			m_pdc->reg_p38 |= 0x2; /* Set bit 1 on port 38 register, PDC polls this port looking for a command */
			if(TRACE_FDC) logerror("--- FDD Old Command: %02X and %02X\n", data_b0, data_b1);
			break;
		case 0xC0B0:
		case 0xC1B0: /* fdd_dest_address register */
			fdd_dest_address = data << 1;
			if(TRACE_FDC) logerror("--- FDD destination address: %08X PC: %08X Register: %08X (A6+4): %08X\n", (fdd_dma_bank & 0x7FFFF800) + (fdd_dest_address&0x3FFFF), m_maincpu->pc(), offset << 2 | 0x5FF00000, debug_a6());
			data_b0 = data & 0xFF;
			data_b1 = (data & 0xFF00) >> 8;
			m_pdc->reg_p6 = data_b0;
			m_pdc->reg_p7 = data_b1;
			m_pdc->reg_p38 |= 0x2; // Set bit 1 on port 38 register, PDC polls this port looking for a command
			if(TRACE_FDC)logerror("--- FDD SET PDC Port 38: %X\n",m_pdc->reg_p38);
			break;
		case 0x80B0: /* FDD command address register */
			uint32_t fdd_scsi_command;
			uint32_t fdd_scsi_command2;
			unsigned char c_fdd_scsi_command[8]; // Array for SCSI command
			int scsi_lba; // FDD LBA location here, extracted from command

			/* Clear FDD Command completion status 0x5FF030B0 (PDC 0x4, 0x5) */
			m_pdc->reg_p4 = 0;
			m_pdc->reg_p5 = 0;

			/* Send FDD SCSI command location address to PDC 0x2, 0x3 */
			if(TRACE_FDC) logerror("--- FDD command address: %08X PC: %08X Register: %08X (A6+4): %08X A4: %08X (A5): %08X (A5+20): %08X\n", (fdd_dma_bank & 0x7FFFF800) + ((data << 1)&0x3FFFF), m_maincpu->pc(), offset << 2 | 0x5FF00000, debug_a6(), ptr_m68000->state_int(M68K_A4), debug_a5(), debug_a5_20());
			data_b0 = data & 0xFF;
			data_b1 = (data & 0xFF00) >> 8;
			m_pdc->reg_p2 = data_b0;
			m_pdc->reg_p3 = data_b1;

			fdd_scsi_command = swap_uint32(m_mem->read_dword((fdd_dma_bank & 0x7FFFF800) + ((data << 1)&0x3FFFF)));
			fdd_scsi_command2 = swap_uint32(m_mem->read_dword(((fdd_dma_bank & 0x7FFFF800) + ((data << 1)&0x3FFFF))+4));

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

			break;

		default:
			if(TRACE_FDC || TRACE_HDC || TRACE_SMIOC) logerror("Unknown write address: %08X Data: %08X PC: %08X\n", offset << 2 | 0x5FF00000, data, m_maincpu->pc());
			if(((offset << 2) & 0xFF) == 0x26 * 4 || ((offset << 2) & 0xFF) == 0x27 * 4) TRACE_SMIOC_WRITE(offset << 2 | 0x5FF00000, data, "Unknown", nullptr);
	}
}

/******************************************************************************
 CPU board registers [0xFF010000 - 0xFF06FFFF]
******************************************************************************/
READ32_MEMBER( r9751_state::r9751_mmio_ff01_r )
{
	switch(offset << 2)
	{
		default:
			return 0;
	}
}

WRITE32_MEMBER( r9751_state::r9751_mmio_ff01_w )
{
	/* Unknown mask */
	if (mem_mask != 0xFFFFFFFF)
		logerror("Mask found: %08X Register: %08X PC: %08X\n", mem_mask, offset << 2 | 0xFF010000, m_maincpu->pc());

	switch(offset << 2)
	{
		case 0x000C: /* FDD DMA Offset */
			fdd_dma_bank = data;
			if(TRACE_DMA) logerror("Banking register(FDD): %08X PC: %08X Data: %08X\n", offset << 2 | 0xFF010000, m_maincpu->pc(), data);
			return;
		case 0x0010: /* SMIOC DMA Offset */
			smioc_dma_bank = data;
			if(TRACE_DMA) logerror("Banking register(SMIOC): %08X PC: %08X Data: %08X\n", offset << 2 | 0xFF010000, m_maincpu->pc(), data);
			return;
		default:
			if(TRACE_DMA) logerror("Banking register(Unknown): %08X PC: %08X Data: %08X\n", offset << 2 | 0xFF010000, m_maincpu->pc(), data);
			return;
	}
}

READ32_MEMBER( r9751_state::r9751_mmio_ff05_r )
{
	uint32_t data;

	switch(offset << 2)
	{
		case 0x0004:
			return reg_ff050004;
		case 0x0300:
			return 0x1B | (1<<0x14);
		case 0x0320: /* Some type of counter */
			return (machine().time() - timer_32khz_last).as_ticks(32768) & 0xFFFF;
		case 0x0584:
			return 0;
		case 0x0610:
			return 0xabacabac;
		case 0x0014:
			return 0x80;
		default:
			data = 0;
			if(TRACE_CPU_REG) logerror("Instruction: %08x READ MMIO(%08x): %08x & %08x\n", m_maincpu->pc(), offset << 2 | 0xFF050000, data, mem_mask);
			return data;
	}
}

WRITE32_MEMBER( r9751_state::r9751_mmio_ff05_w )
{
	/* Unknown mask */
	if (mem_mask != 0xFFFFFFFF)
		logerror("Mask found: %08X Register: %08X PC: %08X\n", mem_mask, offset << 2 | 0xFF050000, m_maincpu->pc());

	switch(offset << 2)
	{
		case 0x0004:
			reg_ff050004 = data;
			return;
		case 0x000C: /* CPU LED hex display indicator */
			if(TRACE_LED) logerror("\n*** LED: %02x, Instruction: %08x ***\n\n", data, m_maincpu->pc());
			return;
		case 0x0320:
			timer_32khz_last = machine().time();
			return;
		default:
			if(TRACE_CPU_REG) logerror("Instruction: %08x WRITE MMIO(%08x): %08x & %08x\n", m_maincpu->pc(),  offset << 2 | 0xFF050000, data, mem_mask);
			return;
	}
}

READ32_MEMBER( r9751_state::r9751_mmio_fff8_r )
{
	uint32_t data;

	switch(offset << 2)
	{
		case 0x0040:
			return reg_fff80040;
		default:
			data = 0;
			if(TRACE_CPU_REG) logerror("Instruction: %08x READ MMIO(%08x): %08x & %08x\n", m_maincpu->pc(), offset << 2 | 0xFFF80000, data, mem_mask);
			return data;
	}
}

WRITE32_MEMBER( r9751_state::r9751_mmio_fff8_w )
{
	/* Unknown mask */
	if (mem_mask != 0xFFFFFFFF)
		logerror("Mask found: %08X Register: %08X PC: %08X\n", mem_mask, offset << 2 | 0xFFF80000, m_maincpu->pc());

	switch(offset << 2)
	{
		case 0x0040:
			reg_fff80040 = data;
			return;
		default:
			if(TRACE_CPU_REG) logerror("Instruction: %08x WRITE MMIO(%08x): %08x & %08x\n", m_maincpu->pc(), offset << 2 | 0xFFF80000, data, mem_mask);
	}
}

/******************************************************************************
 Address Maps
******************************************************************************/

void r9751_state::r9751_mem(address_map &map)
{
	//ADDRESS_MAP_UNMAP_HIGH
	map(0x00000000, 0x00ffffff).ram().share("main_ram"); // 16MB
	map(0x08000000, 0x0800ffff).rom().region("prom", 0);
	map(0x5FF00000, 0x5FFFFFFF).rw(this, FUNC(r9751_state::r9751_mmio_5ff_r), FUNC(r9751_state::r9751_mmio_5ff_w));
	map(0xFF010000, 0xFF01FFFF).rw(this, FUNC(r9751_state::r9751_mmio_ff01_r), FUNC(r9751_state::r9751_mmio_ff01_w));
	map(0xFF050000, 0xFF06FFFF).rw(this, FUNC(r9751_state::r9751_mmio_ff05_r), FUNC(r9751_state::r9751_mmio_ff05_w));
	map(0xFFF80000, 0xFFF8FFFF).rw(this, FUNC(r9751_state::r9751_mmio_fff8_r), FUNC(r9751_state::r9751_mmio_fff8_w));
	//AM_RANGE(0xffffff00,0xffffffff) AM_RAM // Unknown area
}

/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( r9751 )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

MACHINE_CONFIG_START(r9751_state::r9751)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M68030, 20000000)
	MCFG_DEVICE_PROGRAM_MAP(r9751_mem)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(PUT(r9751_state, kbd_put))

	/* i/o hardware */
	MCFG_DEVICE_ADD("smioc", SMIOC, 0)

	/* disk hardware */
	MCFG_DEVICE_ADD("pdc", PDC, 0)
	MCFG_PDC_R_CB(READ8(*this, r9751_state, pdc_dma_r))
	MCFG_PDC_W_CB(WRITE8(*this, r9751_state, pdc_dma_w))
	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_DEVICE_ADD("wd33c93", WD33C93, 0)
	MCFG_LEGACY_SCSI_PORT("scsi")

	/* software list */
	MCFG_SOFTWARE_LIST_ADD("flop_list","r9751")
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(r9751)
	ROM_REGION32_BE(0x00010000, "prom", 0)
	ROM_SYSTEM_BIOS(0, "prom34",  "PROM Version 3.4")
	ROMX_LOAD( "p-n_98d4643__abaco_v3.4__=49fe7a=__j221.27512.bin", 0x0000, 0x10000, CRC(9fb19a85) SHA1(c861e15a2fc9a4ef689c2034c53fbb36f17f7da6), ROM_GROUPWORD | ROM_BIOS(1) ) // Label: "P/N 98D4643 // ABACO V3.4 // (49FE7A) // J221" 27512 @Unknown

	ROM_SYSTEM_BIOS(1, "prom42", "PROM Version 4.2")
	ROMX_LOAD( "98d5731__zebra_v4.2__4cd79d.u5", 0x0000, 0x10000, CRC(e640f8df) SHA1(a9e4fa271d7f2f3a134e2120932ec088d5b8b007), ROM_GROUPWORD | ROM_BIOS(2) ) // Label: 98D5731 // ZEBRA V4.2 // 4CD79D 27512 @Unknown
ROM_END



/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY               FULLNAME              FLAGS
COMP( 1988, r9751, 0,      0,      r9751,   r9751, r9751_state, init_r9751, "ROLM Systems, Inc.", "ROLM 9751 Model 10", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
