// license:BSD-3-Clause
// copyright-holders:Nathan Woods, R. Belmont
/***************************************************************************

    machine/bebox.cpp

    BeBox

    Memory map:

    00000000 - 3FFFFFFF   Physical RAM
    40000000 - 7FFFFFFF   Motherboard glue registers
    80000000 - 807FFFFF   ISA I/O
    81000000 - BF7FFFFF   PCI I/O
    BFFFFFF0 - BFFFFFFF   PCI/ISA interrupt acknowledge
    FF000000 - FFFFFFFF   ROM/flash

    In ISA space, peripherals are generally in similar places as they are on
    standard PC hardware (e.g. - the keyboard is 80000060 and 80000064).  The
    following table shows more:

    Keyboard/Mouse      (Intel 8242)   80000060, 80000064
    Real Time Clock     (BQ3285)       80000070, 80000074
    IDE ATA Interface                  800001F0-F7, 800003F6-7
    COM2                               800002F8-F
    GeekPort A/D Control               80000360-1
    GeekPort A/D Data                  80000362-3
    GeekPort A/D Rate                  80000364
    GeekPort OE                        80000366
    Infrared Interface                 80000368-A
    COM3                               80000380-7
    COM4                               80000388-F
    GeekPort D/A                       80000390-3
    GeekPort GPA/GPB                   80000394
    Joystick Buttons                   80000397
    SuperIO Config (PReP standard)     80000398-9
    MIDI Port 1                        800003A0-7
    MIDI Port 2                        800003A8-F
    Parallel                           800003BC-E
    Floppy                             800003F0-7
    COM1                               800003F8-F
    AD1848                             80000830-4


  Interrupt bit masks:

    bit 31  - N/A (used to set/clear masks)
    bit 30  - SMI interrupt to CPU 0 (CPU 0 only)
    bit 29  - SMI interrupt to CPU 1 (CPU 1 only)
    bit 28  - Unused
    bit 27  - COM1 (PC IRQ #4)
    bit 26  - COM2 (PC IRQ #3)
    bit 25  - COM3
    bit 24  - COM4
    bit 23  - MIDI1
    bit 22  - MIDI2
    bit 21  - SCSI
    bit 20  - PCI Slot #1
    bit 19  - PCI Slot #2
    bit 18  - PCI Slot #3
    bit 17  - Sound
    bit 16  - Keyboard (PC IRQ #1)
    bit 15  - Real Time Clock (PC IRQ #8)
    bit 14  - PC IRQ #5
    bit 13  - Floppy Disk (PC IRQ #6)
    bit 12  - Parallel Port (PC IRQ #7)
    bit 11  - PC IRQ #9
    bit 10  - PC IRQ #10
    bit  9  - PC IRQ #11
    bit  8  - Mouse (PC IRQ #12)
    bit  7  - IDE (PC IRQ #14)
    bit  6  - PC IRQ #15
    bit  5  - PIC8259
    bit  4  - Infrared Controller
    bit  3  - Analog To Digital
    bit  2  - GeekPort
    bit  1  - Unused
    bit  0  - Unused

    Be documentation uses PowerPC bit numbering conventions (i.e. - bit #0 is
    the most significant bit)

    PCI Devices:
        #0      Motorola MPC105
        #11     Intel 82378 PCI/ISA bridge
        #12     NCR 53C810 SCSI

    More hardware information at http://www.netbsd.org/Ports/bebox/hardware.html

***************************************************************************/

/* Core includes */
#include "emu.h"
#include "includes/bebox.h"

/* Components */
#include "video/pc_vga.h"
#include "bus/lpci/cirrus.h"
#include "cpu/powerpc/ppc.h"
#include "machine/mc146818.h"
#include "bus/ata/ataintf.h"
#include "bus/lpci/pci.h"

#define LOG_CPUIMASK    1
#define LOG_UART        1
#define LOG_INTERRUPTS  1

/*************************************
 *
 *  Interrupts and Motherboard Registers
 *
 *************************************/

static void bebox_mbreg32_w(uint32_t *target, uint64_t data, uint64_t mem_mask)
{
	int i;

	for (i = 1; i < 32; i++)
	{
		if ((data >> (63 - i)) & 1)
		{
			if ((data >> 63) & 1)
				*target |= 0x80000000 >> i;
			else
				*target &= ~(0x80000000 >> i);
		}
	}
}


uint64_t bebox_state::bebox_cpu0_imask_r()
{
	return ((uint64_t) m_cpu_imask[0]) << 32;
}

uint64_t bebox_state::bebox_cpu1_imask_r()
{
	return ((uint64_t) m_cpu_imask[1]) << 32;
}

uint64_t bebox_state::bebox_interrupt_sources_r()
{
	return ((uint64_t) m_interrupts) << 32;
}

void bebox_state::bebox_cpu0_imask_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	uint32_t old_imask = m_cpu_imask[0];

	bebox_mbreg32_w(&m_cpu_imask[0], data, mem_mask);

	if (old_imask != m_cpu_imask[0])
	{
		if (LOG_CPUIMASK)
		{
			logerror("%s BeBox CPU #0 imask=0x%08x\n",
				machine().describe_context(), m_cpu_imask[0]);
		}
		bebox_update_interrupts();
	}
}

void bebox_state::bebox_cpu1_imask_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	uint32_t old_imask = m_cpu_imask[1];

	bebox_mbreg32_w(&m_cpu_imask[1], data, mem_mask);

	if (old_imask != m_cpu_imask[1])
	{
		if (LOG_CPUIMASK)
		{
			logerror("%s BeBox CPU #1 imask=0x%08x\n",
				machine().describe_context(), m_cpu_imask[1]);
		}
		bebox_update_interrupts();
	}
}

uint64_t bebox_state::bebox_crossproc_interrupts_r(address_space &space)
{
	uint32_t result;
	result = m_crossproc_interrupts;

	/* return a different result depending on which CPU is accessing this handler */
	if (&space != &m_ppc[0]->space(AS_PROGRAM))
		result |= 0x02000000;
	else
		result &= ~0x02000000;

	return ((uint64_t) result) << 32;
}

void bebox_state::bebox_crossproc_interrupts_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	static const struct
	{
		uint32_t mask;
		int cpunum;
		int active_high;
		int inputline;
	} crossproc_map[] =
	{
		{ 0x40000000, 0, 1, 0/*PPC_INPUT_LINE_SMI*/ },
		{ 0x20000000, 1, 1, 0/*PPC_INPUT_LINE_SMI*/ },
		{ 0x08000000, 0, 0, 0/*PPC_INPUT_LINE_TLBISYNC*/ },
		{ 0x04000000, 1, 0, 0/*PPC_INPUT_LINE_TLBISYNC*/ }
	};
	uint32_t old_crossproc_interrupts = m_crossproc_interrupts;

	bebox_mbreg32_w(&m_crossproc_interrupts, data, mem_mask);

	for (int i = 0; i < std::size(crossproc_map); i++)
	{
		if ((old_crossproc_interrupts ^ m_crossproc_interrupts) & crossproc_map[i].mask)
		{
			int line;
			if (m_crossproc_interrupts & crossproc_map[i].mask)
				line = crossproc_map[i].active_high ? ASSERT_LINE : CLEAR_LINE;
			else
				line = crossproc_map[i].active_high ? CLEAR_LINE : ASSERT_LINE;

			if (LOG_INTERRUPTS)
			{
/*
                logerror("bebox_crossproc_interrupts_w(): CPU #%d %s %s\n",
                    crossproc_map[i].cpunum, line ? "Asserting" : "Clearing",
                    (crossproc_map[i].inputline == PPC_INPUT_LINE_SMI) ? "SMI" : "TLBISYNC");
                    */
			}

			m_ppc[crossproc_map[i].cpunum]->set_input_line(crossproc_map[i].inputline, line);
		}
	}
}

void bebox_state::bebox_processor_resets_w(uint64_t data)
{
	uint8_t b = (uint8_t) (data >> 56);

	if (b & 0x20)
	{
		m_ppc[1]->set_input_line(INPUT_LINE_RESET, (b & 0x80) ? CLEAR_LINE : ASSERT_LINE);
	}
}


void bebox_state::bebox_update_interrupts()
{
	uint32_t interrupt;

	for (int cpunum = 0; cpunum < 2; cpunum++)
	{
		interrupt = m_interrupts & m_cpu_imask[cpunum];

		if (LOG_INTERRUPTS)
		{
			logerror("\tbebox_update_interrupts(): CPU #%d [%08X|%08X] IRQ %s\n", cpunum,
				m_interrupts, m_cpu_imask[cpunum], interrupt ? "on" : "off");
		}

		m_ppc[cpunum]->set_input_line(INPUT_LINE_IRQ0, interrupt ? ASSERT_LINE : CLEAR_LINE);
	}
}


void bebox_state::bebox_set_irq_bit(unsigned int interrupt_bit, int val)
{
	static const char *const interrupt_names[32] =
	{
		nullptr,
		nullptr,
		"GEEKPORT",
		"ADC",
		"IR",
		"PIC8259",
		"PCIRQ 15",
		"IDE",
		"MOUSE",
		"PCIRQ 11",
		"PCIRQ 10",
		"PCIRQ 9",
		"PARALLEL",
		"FLOPPY",
		"PCIRQ 5",
		"RTC",
		"KEYBOARD",
		"SOUND",
		"PCI3",
		"PCI2",
		"PCI1",
		"SCSI",
		"MIDI2",
		"MIDI1",
		"COM4",
		"COM3",
		"COM2",
		"COM1",
		nullptr,
		"SMI1",
		"SMI0",
		nullptr
	};
	uint32_t old_interrupts;

	if (LOG_INTERRUPTS)
	{
		/* make sure that we don't shoot ourself in the foot */
		if ((interrupt_bit >= std::size(interrupt_names)) || !interrupt_names[interrupt_bit])
			throw emu_fatalerror("bebox_state::bebox_set_irq_bit: Raising invalid interrupt");

		logerror("bebox_set_irq_bit(): pc[0]=0x%08x pc[1]=0x%08x %s interrupt #%u (%s)\n",
			unsigned(m_ppc[0]->pc()),
			unsigned(m_ppc[1]->pc()),
			val ? "Asserting" : "Clearing",
			interrupt_bit, interrupt_names[interrupt_bit]);
	}

	old_interrupts = m_interrupts;
	if (val)
		m_interrupts |= 1 << interrupt_bit;
	else
		m_interrupts &= ~(1 << interrupt_bit);

	/* if interrupt values have changed, update the lines */
	if (m_interrupts != old_interrupts)
		bebox_update_interrupts();
}

/*************************************
 *
 *  Floppy Disk Controller
 *
 *************************************/

WRITE_LINE_MEMBER( bebox_state::fdc_interrupt )
{
	bebox_set_irq_bit(13, state);
	m_pic8259[0]->ir6_w(state);
}

/*************************************
 *
 *  8259 PIC
 *
 *************************************/

uint64_t bebox_state::bebox_interrupt_ack_r()
{
	uint32_t result;
	result = m_pic8259[0]->acknowledge();
	bebox_set_irq_bit(5, 0);   /* HACK */
	return ((uint64_t) result) << 56;
}


/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

WRITE_LINE_MEMBER(bebox_state::bebox_pic8259_master_set_int_line)
{
	bebox_set_irq_bit(5, state);
}

WRITE_LINE_MEMBER(bebox_state::bebox_pic8259_slave_set_int_line)
{
	m_pic8259[0]->ir2_w(state);
}

/*************************************
 *
 *  Floppy/IDE/ATA
 *
 *************************************/

WRITE_LINE_MEMBER(bebox_state::bebox_ide_interrupt)
{
	bebox_set_irq_bit(7, state);
	m_pic8259[0]->ir6_w(state);
}


/*************************************
 *
 *  Video card (Cirrus Logic CL-GD5430)
 *
 *************************************/
/*
uint64_t bebox_state::bebox_video_r(offs_t offset, uint64_t mem_mask)
{
    uint64_t result = 0;
    mem_mask = FLIPENDIAN_INT64(mem_mask);
    if (ACCESSING_BITS_0_7)
        result |= (uint64_t)vga_mem_linear_r(offset * 8 + 0, mem_mask >> 0) << 0;
    if (ACCESSING_BITS_8_15)
        result |= (uint64_t)vga_mem_linear_r(offset * 8 + 1, mem_mask >> 8) << 8;
    if (ACCESSING_BITS_16_23)
        result |= (uint64_t)vga_mem_linear_r(offset * 8 + 2, mem_mask >> 16) << 16;
    if (ACCESSING_BITS_24_31)
        result |= (uint64_t)vga_mem_linear_r(offset * 8 + 3, mem_mask >> 24) << 24;
    if (ACCESSING_BITS_32_39)
        result |= (uint64_t)vga_mem_linear_r(offset * 8 + 4, mem_mask >> 32) << 32;
    if (ACCESSING_BITS_40_47)
        result |= (uint64_t)vga_mem_linear_r(offset * 8 + 5, mem_mask >> 40) << 40;
    if (ACCESSING_BITS_48_55)
        result |= (uint64_t)vga_mem_linear_r(offset * 8 + 6, mem_mask >> 48) << 48;
    if (ACCESSING_BITS_56_63)
        result |= (uint64_t)vga_mem_linear_r(offset * 8 + 7, mem_mask >> 56) << 56;
    return FLIPENDIAN_INT64(result);
}


void bebox_state::bebox_video_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
    data = FLIPENDIAN_INT64(data);
    mem_mask = FLIPENDIAN_INT64(mem_mask);
    if (ACCESSING_BITS_0_7)
        vga_mem_linear_w(offset * 8 + 0, data >> 0 , mem_mask >> 0);
    if (ACCESSING_BITS_8_15)
        vga_mem_linear_w(offset * 8 + 1, data >> 8 , mem_mask >> 8);
    if (ACCESSING_BITS_16_23)
        vga_mem_linear_w(offset * 8 + 2, data >> 16, mem_mask >> 16);
    if (ACCESSING_BITS_24_31)
        vga_mem_linear_w(offset * 8 + 3, data >> 24, mem_mask >> 24);
    if (ACCESSING_BITS_32_39)
        vga_mem_linear_w(offset * 8 + 4, data >> 32, mem_mask >> 32);
    if (ACCESSING_BITS_40_47)
        vga_mem_linear_w(offset * 8 + 5, data >> 40, mem_mask >> 40);
    if (ACCESSING_BITS_48_55)
        vga_mem_linear_w(offset * 8 + 6, data >> 48, mem_mask >> 48);
    if (ACCESSING_BITS_56_63)
        vga_mem_linear_w(offset * 8 + 7, data >> 56, mem_mask >> 56);
}
*/
/*************************************
 *
 *  8237 DMA
 *
 *************************************/


uint8_t bebox_state::bebox_page_r(offs_t offset)
{
	uint8_t data = m_at_pages[offset % 0x10];

	switch(offset % 8)
	{
		case 1:
			data = m_dma_offset[(offset / 8) & 1][2];
			break;
		case 2:
			data = m_dma_offset[(offset / 8) & 1][3];
			break;
		case 3:
			data = m_dma_offset[(offset / 8) & 1][1];
			break;
		case 7:
			data = m_dma_offset[(offset / 8) & 1][0];
			break;
	}
	return data;
}


void bebox_state::bebox_page_w(offs_t offset, uint8_t data)
{
	m_at_pages[offset % 0x10] = data;

	switch(offset % 8)
	{
		case 1:
			m_dma_offset[(offset / 8) & 1][2] &= 0xFF00;
			m_dma_offset[(offset / 8) & 1][2] |= ((uint16_t ) data) << 0;
			break;
		case 2:
			m_dma_offset[(offset / 8) & 1][3] &= 0xFF00;
			m_dma_offset[(offset / 8) & 1][3] |= ((uint16_t ) data) << 0;
			break;
		case 3:
			m_dma_offset[(offset / 8) & 1][1] &= 0xFF00;
			m_dma_offset[(offset / 8) & 1][1] |= ((uint16_t ) data) << 0;
			break;
		case 7:
			m_dma_offset[(offset / 8) & 1][0] &= 0xFF00;
			m_dma_offset[(offset / 8) & 1][0] |= ((uint16_t ) data) << 0;
			break;
	}
}


void bebox_state::bebox_80000480_w(offs_t offset, uint8_t data)
{
	switch(offset % 8)
	{
		case 1:
			m_dma_offset[(offset / 8) & 1][2] &= 0x00FF;
			m_dma_offset[(offset / 8) & 1][2] |= ((uint16_t ) data) << 8;
			break;
		case 2:
			m_dma_offset[(offset / 8) & 1][3] &= 0x00FF;
			m_dma_offset[(offset / 8) & 1][3] |= ((uint16_t ) data) << 8;
			break;
		case 3:
			m_dma_offset[(offset / 8) & 1][1] &= 0x00FF;
			m_dma_offset[(offset / 8) & 1][1] |= ((uint16_t ) data) << 8;
			break;
		case 7:
			m_dma_offset[(offset / 8) & 1][0] &= 0x00FF;
			m_dma_offset[(offset / 8) & 1][0] |= ((uint16_t ) data) << 8;
			break;
	}
}


uint8_t bebox_state::bebox_80000480_r()
{
	fatalerror("NYI\n");
}


WRITE_LINE_MEMBER(bebox_state::bebox_dma_hrq_changed)
{
	m_ppc[0]->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237[0]->hack_w( state );
}


uint8_t bebox_state::bebox_dma_read_byte(offs_t offset)
{
	address_space& prog_space = m_ppc[0]->space(AS_PROGRAM); // get the right address space
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0x7FFF0000;
	return prog_space.read_byte(page_offset + offset);
}


void bebox_state::bebox_dma_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_ppc[0]->space(AS_PROGRAM); // get the right address space
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0x7FFF0000;
	prog_space.write_byte(page_offset + offset, data);
}

WRITE_LINE_MEMBER(bebox_state::bebox_dma8237_out_eop){
	m_smc37c78->tc_w(state);
}

inline void bebox_state::set_dma_channel(int channel, int state)
{
	if (!state) m_dma_channel = channel;
}

WRITE_LINE_MEMBER(bebox_state::pc_dack0_w){ set_dma_channel(0, state); }
WRITE_LINE_MEMBER(bebox_state::pc_dack1_w){ set_dma_channel(1, state); }
WRITE_LINE_MEMBER(bebox_state::pc_dack2_w){ set_dma_channel(2, state); }
WRITE_LINE_MEMBER(bebox_state::pc_dack3_w){ set_dma_channel(3, state); }

/*************************************
 *
 *  8254 PIT
 *
 *************************************/

WRITE_LINE_MEMBER(bebox_state::bebox_timer0_w)
{
	m_pic8259[0]->ir0_w(state);
}


/*************************************
 *
 *  Flash ROM
 *
 *************************************/

uint8_t bebox_state::bebox_flash_r(offs_t offset)
{
	offset = (offset & ~7) | (7 - (offset & 7));
	return m_flash->read(offset);
}


void bebox_state::bebox_flash_w(offs_t offset, uint8_t data)
{
	offset = (offset & ~7) | (7 - (offset & 7));
	m_flash->write(offset, data);
}

/*************************************
 *
 *  SCSI
 *
 *************************************/


uint64_t bebox_state::scsi53c810_r(offs_t offset, uint64_t mem_mask)
{
	int reg = offset*8;
	uint64_t r = 0;
	if (!ACCESSING_BITS_56_63) {
		r |= (uint64_t)m_lsi53c810->reg_r(reg+0) << 56;
	}
	if (!ACCESSING_BITS_48_55) {
		r |= (uint64_t)m_lsi53c810->reg_r(reg+1) << 48;
	}
	if (!ACCESSING_BITS_40_47) {
		r |= (uint64_t)m_lsi53c810->reg_r(reg+2) << 40;
	}
	if (!ACCESSING_BITS_32_39) {
		r |= (uint64_t)m_lsi53c810->reg_r(reg+3) << 32;
	}
	if (!ACCESSING_BITS_24_31) {
		r |= (uint64_t)m_lsi53c810->reg_r(reg+4) << 24;
	}
	if (!ACCESSING_BITS_16_23) {
		r |= (uint64_t)m_lsi53c810->reg_r(reg+5) << 16;
	}
	if (!ACCESSING_BITS_8_15) {
		r |= (uint64_t)m_lsi53c810->reg_r(reg+6) << 8;
	}
	if (!ACCESSING_BITS_0_7) {
		r |= (uint64_t)m_lsi53c810->reg_r(reg+7) << 0;
	}

	return r;
}


void bebox_state::scsi53c810_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	int reg = offset*8;
	if (!ACCESSING_BITS_56_63) {
		m_lsi53c810->reg_w(reg+0, data >> 56);
	}
	if (!ACCESSING_BITS_48_55) {
		m_lsi53c810->reg_w(reg+1, data >> 48);
	}
	if (!ACCESSING_BITS_40_47) {
		m_lsi53c810->reg_w(reg+2, data >> 40);
	}
	if (!ACCESSING_BITS_32_39) {
		m_lsi53c810->reg_w(reg+3, data >> 32);
	}
	if (!ACCESSING_BITS_24_31) {
		m_lsi53c810->reg_w(reg+4, data >> 24);
	}
	if (!ACCESSING_BITS_16_23) {
		m_lsi53c810->reg_w(reg+5, data >> 16);
	}
	if (!ACCESSING_BITS_8_15) {
		m_lsi53c810->reg_w(reg+6, data >> 8);
	}
	if (!ACCESSING_BITS_0_7) {
		m_lsi53c810->reg_w(reg+7, data >> 0);
	}
}

// the 2 following methods are legacy code, currently unused

uint32_t bebox_state::scsi53c810_pci_read(int function, int offset, uint32_t mem_mask)
{
	uint32_t result = 0;

	if (function == 0)
	{
		switch(offset)
		{
			case 0x00:  /* vendor/device ID */
				result = 0x00011000;
				break;

			case 0x08:
				result = 0x01000000;
				break;

			default:
				result = m_scsi53c810_data[offset / 4];
				break;
		}
	}
	return result;
}


void bebox_state::scsi53c810_pci_write(int function, int offset, uint32_t data, uint32_t mem_mask)
{
	offs_t addr;

	if (function == 0)
	{
		m_scsi53c810_data[offset / 4] = data;

		switch(offset)
		{
			case 0x04:
				/* command
				 *
				 * bit 8:   SERR/ Enable
				 * bit 6:   Enable Parity Response
				 * bit 4:   Write and Invalidate Mode
				 * bit 2:   Enable Bus Mastering
				 * bit 1:   Enable Memory Space
				 * bit 0:   Enable IO Space
				 */
				if (data & 0x02)
				{
					/* brutal ugly hack; at some point the PCI code should be handling this stuff */
					if (m_scsi53c810_data[5] != 0xFFFFFFF0)
					{
						address_space &space = m_ppc[0]->space(AS_PROGRAM);

						addr = (m_scsi53c810_data[5] | 0xC0000000) & ~0xFF;
						space.install_readwrite_handler(addr, addr + 0xFF, read64s_delegate(*this, FUNC(bebox_state::scsi53c810_r)), write64s_delegate(*this, FUNC(bebox_state::scsi53c810_w)));
					}
				}
				break;
		}
	}
}


/*************************************
 *
 *  Driver main
 *
 *************************************/

void bebox_state::machine_reset()
{
	m_ppc[0]->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	m_ppc[1]->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	// Endianness? Bah!
	memcpy(m_flash->base(),memregion("user1")->base(),0x200000);
}

void bebox_state::machine_start()
{
	m_interrupts = 0;
}

void bebox_state::init_bebox()
{
	address_space &space_0 = m_ppc[0]->space(AS_PROGRAM);
	address_space &space_1 = m_ppc[1]->space(AS_PROGRAM);

	/* set up boot and flash ROM */
	membank("bank2")->set_base(memregion("user2")->base());

	/* install managed RAM */
	space_0.install_ram(0, m_ram->size() - 1, 0x02000000, m_ram->pointer());
	space_1.install_ram(0, m_ram->size() - 1, 0x02000000, m_ram->pointer());

	/* The following is a verrrry ugly hack put in to support NetBSD for
	 * NetBSD.  When NetBSD/bebox it does most of its work on CPU #0 and then
	 * lets CPU #1 go.  However, it seems that CPU #1 jumps into never-never
	 * land, crashes, and then goes into NetBSD's crash handler which catches
	 * it.  The current PowerPC core cannot catch this trip into never-never
	 * land properly, and MAME crashes.  In the interim, this "mitten" catches
	 * the crash
	 */
	{
		static uint64_t ops[2] =
		{
			/* li r0, 0x0700 */
			/* mtspr ctr, r0 */
			0x380007007C0903A6U,

			/* bcctr 0x14, 0 */
			0x4E80042000000000U
		};
		space_1.install_rom(0x9421FFF0, 0x9421FFFF, ops);
	}
}
