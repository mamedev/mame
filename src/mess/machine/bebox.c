/***************************************************************************

    machine/bebox.c

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
#include "video/cirrus.h"
#include "cpu/powerpc/ppc.h"
#include "machine/ins8250.h"
#include "machine/upd765.h"
#include "machine/mc146818.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/8237dma.h"
#include "machine/idectrl.h"
#include "machine/pci.h"
#include "machine/intelfsh.h"
#include "machine/8042kbdc.h"
#include "machine/53c810.h"
#include "machine/ram.h"

#define LOG_CPUIMASK	1
#define LOG_UART		1
#define LOG_INTERRUPTS	1

INLINE UINT16 read16be_with_read8_handler(read8_space_func handler, address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 result = 0;
	if (ACCESSING_BITS_8_15)
		result |= ((UINT16)(*handler)(space, offset * 2 + 0, mem_mask >> 8)) << 8;
	if (ACCESSING_BITS_0_7)
		result |= ((UINT16)(*handler)(space, offset * 2 + 1, mem_mask >> 0)) << 0;
	return result;
}

INLINE void write16be_with_write8_handler(write8_space_func handler, address_space &space, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	if (ACCESSING_BITS_8_15)
		(*handler)(space, offset * 2 + 0, data >> 8, mem_mask >> 8);
	if (ACCESSING_BITS_0_7)
		(*handler)(space, offset * 2 + 1, data >> 0, mem_mask >> 0);
}

INLINE UINT32 read32be_with_read8_handler(read8_space_func handler, address_space &space, offs_t offset, UINT32 mem_mask)
{
	UINT32 result = 0;
	if (ACCESSING_BITS_16_31)
		result |= read16be_with_read8_handler(handler, space, offset * 2 + 0, mem_mask >> 16) << 16;
	if (ACCESSING_BITS_0_15)
		result |= read16be_with_read8_handler(handler, space, offset * 2 + 1, mem_mask) << 0;
	return result;
}


INLINE void write32be_with_write8_handler(write8_space_func handler, address_space &space, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_16_31)
		write16be_with_write8_handler(handler, space, offset * 2 + 0, data >> 16, mem_mask >> 16);
	if (ACCESSING_BITS_0_15)
		write16be_with_write8_handler(handler, space, offset * 2 + 1, data, mem_mask);
}

INLINE UINT64 read64be_with_read8_handler(read8_space_func handler, address_space &space, offs_t offset, UINT64 mem_mask)
{
	UINT64 result = 0;
	if (ACCESSING_BITS_32_63)
		result |= (UINT64)read32be_with_read8_handler(handler, space, offset * 2 + 0, mem_mask >> 32) << 32;
	if (ACCESSING_BITS_0_31)
		result |= (UINT64)read32be_with_read8_handler(handler, space, offset * 2 + 1, mem_mask) << 0;
	return result;
}


INLINE void write64be_with_write8_handler(write8_space_func handler, address_space &space, offs_t offset, UINT64 data, UINT64 mem_mask)
{
	if (ACCESSING_BITS_32_63)
		write32be_with_write8_handler(handler, space, offset * 2 + 0, data >> 32, mem_mask >> 32);
	if (ACCESSING_BITS_0_31)
		write32be_with_write8_handler(handler, space, offset * 2 + 1, data, mem_mask);
}


/*************************************
 *
 *  Interrupts and Motherboard Registers
 *
 *************************************/

static void bebox_update_interrupts(running_machine &machine);

static void bebox_mbreg32_w(UINT32 *target, UINT64 data, UINT64 mem_mask)
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


READ64_MEMBER(bebox_state::bebox_cpu0_imask_r )
{
	return ((UINT64) m_cpu_imask[0]) << 32;
}

READ64_MEMBER(bebox_state::bebox_cpu1_imask_r )
{
	return ((UINT64) m_cpu_imask[1]) << 32;
}

READ64_MEMBER(bebox_state::bebox_interrupt_sources_r )
{
	return ((UINT64) m_interrupts) << 32;
}

WRITE64_MEMBER(bebox_state::bebox_cpu0_imask_w )
{
	UINT32 old_imask = m_cpu_imask[0];

	bebox_mbreg32_w(&m_cpu_imask[0], data, mem_mask);

	if (old_imask != m_cpu_imask[0])
	{
		if (LOG_CPUIMASK)
		{
			logerror("BeBox CPU #0 pc=0x%08X imask=0x%08x\n",
				(unsigned) space.device().safe_pc( ), m_cpu_imask[0]);
		}
		bebox_update_interrupts(space.machine());
	}
}

WRITE64_MEMBER(bebox_state::bebox_cpu1_imask_w )
{
	UINT32 old_imask = m_cpu_imask[1];

	bebox_mbreg32_w(&m_cpu_imask[1], data, mem_mask);

	if (old_imask != m_cpu_imask[1])
	{
		if (LOG_CPUIMASK)
		{
			logerror("BeBox CPU #1 pc=0x%08X imask=0x%08x\n",
				(unsigned) space.device() .safe_pc( ), m_cpu_imask[1]);
		}
		bebox_update_interrupts(space.machine());
	}
}

READ64_MEMBER(bebox_state::bebox_crossproc_interrupts_r )
{
	UINT32 result;
	result = m_crossproc_interrupts;

	/* return a different result depending on which CPU is accessing this handler */
	if (&space != &space.machine().device("ppc1")->memory().space(AS_PROGRAM))
		result |= 0x02000000;
	else
		result &= ~0x02000000;

	return ((UINT64) result) << 32;
}

WRITE64_MEMBER(bebox_state::bebox_crossproc_interrupts_w )
{
	static const struct
	{
		UINT32 mask;
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
	int i, line;
	UINT32 old_crossproc_interrupts = m_crossproc_interrupts;
	static const char *const cputags[] = { "ppc1", "ppc2" };

	bebox_mbreg32_w(&m_crossproc_interrupts, data, mem_mask);

	for (i = 0; i < ARRAY_LENGTH(crossproc_map); i++)
	{
		if ((old_crossproc_interrupts ^ m_crossproc_interrupts) & crossproc_map[i].mask)
		{
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

			space.machine().device(cputags[crossproc_map[i].cpunum])->execute().set_input_line(crossproc_map[i].inputline, line);
		}
	}
}

WRITE64_MEMBER(bebox_state::bebox_processor_resets_w )
{
	UINT8 b = (UINT8) (data >> 56);

	if (b & 0x20)
	{
		space.machine().device("ppc2")->execute().set_input_line(INPUT_LINE_RESET, (b & 0x80) ? CLEAR_LINE : ASSERT_LINE);
	}
}


static void bebox_update_interrupts(running_machine &machine)
{
	bebox_state *state = machine.driver_data<bebox_state>();
	int cpunum;
	UINT32 interrupt;
	static const char *const cputags[] = { "ppc1", "ppc2" };

	for (cpunum = 0; cpunum < 2; cpunum++)
	{
		interrupt = state->m_interrupts & state->m_cpu_imask[cpunum];

		if (LOG_INTERRUPTS)
		{
			logerror("\tbebox_update_interrupts(): CPU #%d [%08X|%08X] IRQ %s\n", cpunum,
				state->m_interrupts, state->m_cpu_imask[cpunum], interrupt ? "on" : "off");
		}

		machine.device(cputags[cpunum])->execute().set_input_line(INPUT_LINE_IRQ0, interrupt ? ASSERT_LINE : CLEAR_LINE);
	}
}


void bebox_set_irq_bit(running_machine &machine, unsigned int interrupt_bit, int val)
{
	bebox_state *state = machine.driver_data<bebox_state>();
	static const char *const interrupt_names[32] =
	{
		NULL,
		NULL,
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
		NULL,
		"SMI1",
		"SMI0",
		NULL
	};
	UINT32 old_interrupts;

	if (LOG_INTERRUPTS)
	{
		/* make sure that we don't shoot ourself in the foot */
		assert_always((interrupt_bit < ARRAY_LENGTH(interrupt_names)) && (interrupt_names[interrupt_bit] != NULL), "Raising invalid interrupt");

		logerror("bebox_set_irq_bit(): pc[0]=0x%08x pc[1]=0x%08x %s interrupt #%u (%s)\n",
			(unsigned) machine.device("ppc1")->safe_pc(),
			(unsigned) machine.device("ppc2")->safe_pc(),
			val ? "Asserting" : "Clearing",
			interrupt_bit, interrupt_names[interrupt_bit]);
	}

	old_interrupts = state->m_interrupts;
	if (val)
		state->m_interrupts |= 1 << interrupt_bit;
	else
		state->m_interrupts &= ~(1 << interrupt_bit);

	/* if interrupt values have changed, update the lines */
	if (state->m_interrupts != old_interrupts)
		bebox_update_interrupts(machine);
}


/*************************************
 *
 *  COM ports
 *
 *************************************/

const ins8250_interface bebox_uart_inteface_0 =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

const ins8250_interface bebox_uart_inteface_1 =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

const ins8250_interface bebox_uart_inteface_2 =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

const ins8250_interface bebox_uart_inteface_3 =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


/*************************************
 *
 *  Floppy Disk Controller
 *
 *************************************/

void bebox_state::fdc_interrupt(bool state)
{
	bebox_set_irq_bit(machine(), 13, state);
	if ( m_devices.pic8259_master ) {
		pic8259_ir6_w(m_devices.pic8259_master, state);
	}
}


void bebox_state::fdc_dma_drq(bool state)
{
	if ( m_devices.dma8237_1 ) {
		i8237_dreq2_w(m_devices.dma8237_1, state);
	}
}

/*************************************
 *
 *  8259 PIC
 *
 *************************************/

READ64_MEMBER(bebox_state::bebox_interrupt_ack_r )
{
	int result;
	result = pic8259_acknowledge( m_devices.pic8259_master );
	bebox_set_irq_bit(space.machine(), 5, 0);	/* HACK */
	return ((UINT64) result) << 56;
}


/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

WRITE_LINE_MEMBER(bebox_state::bebox_pic8259_master_set_int_line)
{
	bebox_set_irq_bit(machine(), 5, state);
}

WRITE_LINE_MEMBER(bebox_state::bebox_pic8259_slave_set_int_line)
{
	if (m_devices.pic8259_master)
		pic8259_ir2_w(m_devices.pic8259_master, state);
}

READ8_MEMBER(bebox_state::get_slave_ack)
{
	if (offset==2) { // IRQ = 2
		return pic8259_acknowledge(m_devices.pic8259_slave);
	}
	return 0x00;
}

const struct pic8259_interface bebox_pic8259_master_config =
{
	DEVCB_DRIVER_LINE_MEMBER(bebox_state,bebox_pic8259_master_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_DRIVER_MEMBER(bebox_state,get_slave_ack)
};

const struct pic8259_interface bebox_pic8259_slave_config =
{
	DEVCB_DRIVER_LINE_MEMBER(bebox_state,bebox_pic8259_slave_set_int_line),
	DEVCB_LINE_GND,
	DEVCB_NULL
};


/*************************************
 *
 *  Floppy/IDE/ATA
 *
 *************************************/

static device_t *ide_device(running_machine &machine)
{
	return machine.device("ide");
}

READ8_MEMBER(bebox_state::bebox_800001F0_r ) { return ide_controller_r(ide_device(space.machine()), offset + 0x1F0, 1); }
WRITE8_MEMBER(bebox_state::bebox_800001F0_w ) { ide_controller_w(ide_device(space.machine()), offset + 0x1F0, 1, data); }

READ64_MEMBER(bebox_state::bebox_800003F0_r )
{
	UINT64 result = 0;

	if (((mem_mask >> 8) & 0xFF) == 0)
	{
		result |= ide_controller_r(space.machine().device("ide"), 0x3F6, 1) << 8;
	}
	return result;
}


WRITE64_MEMBER(bebox_state::bebox_800003F0_w )
{
	if (((mem_mask >> 8) & 0xFF) == 0)
		ide_controller_w(space.machine().device("ide"), 0x3F6, 1, (data >> 8) & 0xFF);
}


void bebox_ide_interrupt(device_t *device, int state)
{
	bebox_state *drvstate = device->machine().driver_data<bebox_state>();
	bebox_set_irq_bit(device->machine(), 7, state);
	if ( drvstate->m_devices.pic8259_master ) {
		pic8259_ir6_w(drvstate->m_devices.pic8259_master, state);
	}
}


/*************************************
 *
 *  Video card (Cirrus Logic CL-GD5430)
 *
 *************************************/
/*
static READ64_MEMBER(bebox_state::bebox_video_r )
{
    UINT64 result = 0;
    mem_mask = FLIPENDIAN_INT64(mem_mask);
    if (ACCESSING_BITS_0_7)
        result |= (UINT64)vga_mem_linear_r(space, offset * 8 + 0, mem_mask >> 0) << 0;
    if (ACCESSING_BITS_8_15)
        result |= (UINT64)vga_mem_linear_r(space, offset * 8 + 1, mem_mask >> 8) << 8;
    if (ACCESSING_BITS_16_23)
        result |= (UINT64)vga_mem_linear_r(space, offset * 8 + 2, mem_mask >> 16) << 16;
    if (ACCESSING_BITS_24_31)
        result |= (UINT64)vga_mem_linear_r(space, offset * 8 + 3, mem_mask >> 24) << 24;
    if (ACCESSING_BITS_32_39)
        result |= (UINT64)vga_mem_linear_r(space, offset * 8 + 4, mem_mask >> 32) << 32;
    if (ACCESSING_BITS_40_47)
        result |= (UINT64)vga_mem_linear_r(space, offset * 8 + 5, mem_mask >> 40) << 40;
    if (ACCESSING_BITS_48_55)
        result |= (UINT64)vga_mem_linear_r(space, offset * 8 + 6, mem_mask >> 48) << 48;
    if (ACCESSING_BITS_56_63)
        result |= (UINT64)vga_mem_linear_r(space, offset * 8 + 7, mem_mask >> 56) << 56;
    return FLIPENDIAN_INT64(result);
}


static WRITE64_MEMBER(bebox_state::bebox_video_w )
{
    data = FLIPENDIAN_INT64(data);
    mem_mask = FLIPENDIAN_INT64(mem_mask);
    if (ACCESSING_BITS_0_7)
        vga_mem_linear_w(space, offset * 8 + 0, data >> 0 , mem_mask >> 0);
    if (ACCESSING_BITS_8_15)
        vga_mem_linear_w(space, offset * 8 + 1, data >> 8 , mem_mask >> 8);
    if (ACCESSING_BITS_16_23)
        vga_mem_linear_w(space, offset * 8 + 2, data >> 16, mem_mask >> 16);
    if (ACCESSING_BITS_24_31)
        vga_mem_linear_w(space, offset * 8 + 3, data >> 24, mem_mask >> 24);
    if (ACCESSING_BITS_32_39)
        vga_mem_linear_w(space, offset * 8 + 4, data >> 32, mem_mask >> 32);
    if (ACCESSING_BITS_40_47)
        vga_mem_linear_w(space, offset * 8 + 5, data >> 40, mem_mask >> 40);
    if (ACCESSING_BITS_48_55)
        vga_mem_linear_w(space, offset * 8 + 6, data >> 48, mem_mask >> 48);
    if (ACCESSING_BITS_56_63)
        vga_mem_linear_w(space, offset * 8 + 7, data >> 56, mem_mask >> 56);
}
*/
/*************************************
 *
 *  8237 DMA
 *
 *************************************/


READ8_MEMBER(bebox_state::bebox_page_r)
{
	UINT8 data = m_at_pages[offset % 0x10];

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


WRITE8_MEMBER(bebox_state::bebox_page_w)
{
	m_at_pages[offset % 0x10] = data;

	switch(offset % 8)
	{
		case 1:
			m_dma_offset[(offset / 8) & 1][2] &= 0xFF00;
			m_dma_offset[(offset / 8) & 1][2] |= ((UINT16 ) data) << 0;
			break;
		case 2:
			m_dma_offset[(offset / 8) & 1][3] &= 0xFF00;
			m_dma_offset[(offset / 8) & 1][3] |= ((UINT16 ) data) << 0;
			break;
		case 3:
			m_dma_offset[(offset / 8) & 1][1] &= 0xFF00;
			m_dma_offset[(offset / 8) & 1][1] |= ((UINT16 ) data) << 0;
			break;
		case 7:
			m_dma_offset[(offset / 8) & 1][0] &= 0xFF00;
			m_dma_offset[(offset / 8) & 1][0] |= ((UINT16 ) data) << 0;
			break;
	}
}


WRITE8_MEMBER(bebox_state::bebox_80000480_w)
{
	switch(offset % 8)
	{
		case 1:
			m_dma_offset[(offset / 8) & 1][2] &= 0x00FF;
			m_dma_offset[(offset / 8) & 1][2] |= ((UINT16 ) data) << 8;
			break;
		case 2:
			m_dma_offset[(offset / 8) & 1][3] &= 0x00FF;
			m_dma_offset[(offset / 8) & 1][3] |= ((UINT16 ) data) << 8;
			break;
		case 3:
			m_dma_offset[(offset / 8) & 1][1] &= 0x00FF;
			m_dma_offset[(offset / 8) & 1][1] |= ((UINT16 ) data) << 8;
			break;
		case 7:
			m_dma_offset[(offset / 8) & 1][0] &= 0x00FF;
			m_dma_offset[(offset / 8) & 1][0] |= ((UINT16 ) data) << 8;
			break;
	}
}


READ8_MEMBER(bebox_state::bebox_80000480_r)
{
	fatalerror("NYI\n");
}


WRITE_LINE_MEMBER(bebox_state::bebox_dma_hrq_changed)
{
	machine().device("ppc1")->execute().set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( machine().device("dma8237_1"), state );
}


READ8_MEMBER(bebox_state::bebox_dma_read_byte )
{
	address_space& prog_space = machine().device<cpu_device>("ppc1")->space(AS_PROGRAM); // get the right address space
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0x7FFF0000;
	return prog_space.read_byte(page_offset + offset);
}


WRITE8_MEMBER(bebox_state::bebox_dma_write_byte )
{
	address_space& prog_space = machine().device<cpu_device>("ppc1")->space(AS_PROGRAM); // get the right address space
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0x7FFF0000;
	prog_space.write_byte(page_offset + offset, data);
}


READ8_MEMBER(bebox_state::bebox_dma8237_fdc_dack_r){
	return machine().device<smc37c78_device>("smc37c78")->dma_r();
}


WRITE8_MEMBER(bebox_state::bebox_dma8237_fdc_dack_w){
	machine().device<smc37c78_device>("smc37c78")->dma_w(data);
}


WRITE_LINE_MEMBER(bebox_state::bebox_dma8237_out_eop){
	machine().device<smc37c78_device>("smc37c78")->tc_w(state);
}

static void set_dma_channel(running_machine &machine, int channel, int state)
{
	bebox_state *drvstate = machine.driver_data<bebox_state>();
	if (!state) drvstate->m_dma_channel = channel;
}

WRITE_LINE_MEMBER(bebox_state::pc_dack0_w){ set_dma_channel(machine(), 0, state); }
WRITE_LINE_MEMBER(bebox_state::pc_dack1_w){ set_dma_channel(machine(), 1, state); }
WRITE_LINE_MEMBER(bebox_state::pc_dack2_w){ set_dma_channel(machine(), 2, state); }
WRITE_LINE_MEMBER(bebox_state::pc_dack3_w){ set_dma_channel(machine(), 3, state); }


I8237_INTERFACE( bebox_dma8237_1_config )
{
	DEVCB_DRIVER_LINE_MEMBER(bebox_state,bebox_dma_hrq_changed),
	DEVCB_DRIVER_LINE_MEMBER(bebox_state,bebox_dma8237_out_eop),
	DEVCB_DRIVER_MEMBER(bebox_state, bebox_dma_read_byte),
	DEVCB_DRIVER_MEMBER(bebox_state, bebox_dma_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_DRIVER_MEMBER(bebox_state,bebox_dma8237_fdc_dack_r), DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_DRIVER_MEMBER(bebox_state,bebox_dma8237_fdc_dack_w), DEVCB_NULL },
	{ DEVCB_DRIVER_LINE_MEMBER(bebox_state,pc_dack0_w), DEVCB_DRIVER_LINE_MEMBER(bebox_state,pc_dack1_w), DEVCB_DRIVER_LINE_MEMBER(bebox_state,pc_dack2_w), DEVCB_DRIVER_LINE_MEMBER(bebox_state,pc_dack3_w) }
};


I8237_INTERFACE( bebox_dma8237_2_config )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL }
};


/*************************************
 *
 *  8254 PIT
 *
 *************************************/

WRITE_LINE_MEMBER(bebox_state::bebox_timer0_w)
{
	if (m_devices.pic8259_master)
		pic8259_ir0_w(m_devices.pic8259_master, state);
}


const struct pit8253_config bebox_pit8254_config =
{
	{
		{
			4772720/4,				/* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_DRIVER_LINE_MEMBER(bebox_state,bebox_timer0_w)
		},
		{
			4772720/4,				/* dram refresh */
			DEVCB_NULL,
			DEVCB_NULL
		},
		{
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};


/*************************************
 *
 *  Flash ROM
 *
 *************************************/

READ8_MEMBER(bebox_state::bebox_flash_r )
{
	fujitsu_29f016a_device *flash = space.machine().device<fujitsu_29f016a_device>("flash");
	offset = (offset & ~7) | (7 - (offset & 7));
	return flash->read(offset);
}


WRITE8_MEMBER(bebox_state::bebox_flash_w )
{
	fujitsu_29f016a_device *flash = space.machine().device<fujitsu_29f016a_device>("flash");
	offset = (offset & ~7) | (7 - (offset & 7));
	flash->write(offset, data);
}

/*************************************
 *
 *  Keyboard
 *
 *************************************/

static void bebox_keyboard_interrupt(running_machine &machine,int state)
{
	bebox_state *drvstate = machine.driver_data<bebox_state>();
	bebox_set_irq_bit(machine, 16, state);
	if ( drvstate->m_devices.pic8259_master ) {
		pic8259_ir1_w(drvstate->m_devices.pic8259_master, state);
	}
}

static int bebox_get_out2(running_machine &machine) {
	return pit8253_get_output(machine.device("pit8254"), 2 );
}

static const struct kbdc8042_interface bebox_8042_interface =
{
	KBDC8042_STANDARD,
	NULL,
	bebox_keyboard_interrupt,
	NULL,
	bebox_get_out2
};


/*************************************
 *
 *  SCSI
 *
 *************************************/


READ64_MEMBER(bebox_state::scsi53c810_r )
{
	int reg = offset*8;
	UINT64 r = 0;
	if (!(mem_mask & U64(0xff00000000000000))) {
		r |= (UINT64)m_lsi53c810->lsi53c810_reg_r(reg+0) << 56;
	}
	if (!(mem_mask & U64(0x00ff000000000000))) {
		r |= (UINT64)m_lsi53c810->lsi53c810_reg_r(reg+1) << 48;
	}
	if (!(mem_mask & U64(0x0000ff0000000000))) {
		r |= (UINT64)m_lsi53c810->lsi53c810_reg_r(reg+2) << 40;
	}
	if (!(mem_mask & U64(0x000000ff00000000))) {
		r |= (UINT64)m_lsi53c810->lsi53c810_reg_r(reg+3) << 32;
	}
	if (!(mem_mask & U64(0x00000000ff000000))) {
		r |= (UINT64)m_lsi53c810->lsi53c810_reg_r(reg+4) << 24;
	}
	if (!(mem_mask & U64(0x0000000000ff0000))) {
		r |= (UINT64)m_lsi53c810->lsi53c810_reg_r(reg+5) << 16;
	}
	if (!(mem_mask & U64(0x000000000000ff00))) {
		r |= (UINT64)m_lsi53c810->lsi53c810_reg_r(reg+6) << 8;
	}
	if (!(mem_mask & U64(0x00000000000000ff))) {
		r |= (UINT64)m_lsi53c810->lsi53c810_reg_r(reg+7) << 0;
	}

	return r;
}


WRITE64_MEMBER(bebox_state::scsi53c810_w )
{
	int reg = offset*8;
	if (!(mem_mask & U64(0xff00000000000000))) {
		m_lsi53c810->lsi53c810_reg_w(reg+0, data >> 56);
	}
	if (!(mem_mask & U64(0x00ff000000000000))) {
		m_lsi53c810->lsi53c810_reg_w(reg+1, data >> 48);
	}
	if (!(mem_mask & U64(0x0000ff0000000000))) {
		m_lsi53c810->lsi53c810_reg_w(reg+2, data >> 40);
	}
	if (!(mem_mask & U64(0x000000ff00000000))) {
		m_lsi53c810->lsi53c810_reg_w(reg+3, data >> 32);
	}
	if (!(mem_mask & U64(0x00000000ff000000))) {
		m_lsi53c810->lsi53c810_reg_w(reg+4, data >> 24);
	}
	if (!(mem_mask & U64(0x0000000000ff0000))) {
		m_lsi53c810->lsi53c810_reg_w(reg+5, data >> 16);
	}
	if (!(mem_mask & U64(0x000000000000ff00))) {
		m_lsi53c810->lsi53c810_reg_w(reg+6, data >> 8);
	}
	if (!(mem_mask & U64(0x00000000000000ff))) {
		m_lsi53c810->lsi53c810_reg_w(reg+7, data >> 0);
	}
}


UINT32 scsi53c810_pci_read(device_t *busdevice, device_t *device, int function, int offset, UINT32 mem_mask)
{
	bebox_state *state = device->machine().driver_data<bebox_state>();
	UINT32 result = 0;

	if (function == 0)
	{
		switch(offset)
		{
			case 0x00:	/* vendor/device ID */
				result = 0x00011000;
				break;

			case 0x08:
				result = 0x01000000;
				break;

			default:
				result = state->m_scsi53c810_data[offset / 4];
				break;
		}
	}
	return result;
}


void scsi53c810_pci_write(device_t *busdevice, device_t *device, int function, int offset, UINT32 data, UINT32 mem_mask)
{
	bebox_state *state = device->machine().driver_data<bebox_state>();
	offs_t addr;

	if (function == 0)
	{
		state->m_scsi53c810_data[offset / 4] = data;

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
					if (state->m_scsi53c810_data[5] != 0xFFFFFFF0)
					{
						address_space &space = device->machine().device("ppc1")->memory().space(AS_PROGRAM);

						addr = (state->m_scsi53c810_data[5] | 0xC0000000) & ~0xFF;
						space.install_readwrite_handler(addr, addr + 0xFF, read64_delegate(FUNC(bebox_state::scsi53c810_r),state), write64_delegate(FUNC(bebox_state::scsi53c810_w),state));
					}
				}
				break;
		}
	}
}


TIMER_CALLBACK_MEMBER(bebox_state::bebox_get_devices){
	m_devices.pic8259_master = machine().device("pic8259_master");
	m_devices.pic8259_slave = machine().device("pic8259_slave");
	m_devices.dma8237_1 = machine().device("dma8237_1");
	m_devices.dma8237_2 = machine().device("dma8237_2");
}


/*************************************
 *
 *  Driver main
 *
 *************************************/

void bebox_state::machine_reset()
{
	m_devices.pic8259_master = NULL;
	m_devices.pic8259_slave = NULL;
	m_devices.dma8237_1 = NULL;
	m_devices.dma8237_2 = NULL;

	machine().scheduler().timer_set(attotime::zero, timer_expired_delegate(FUNC(bebox_state::bebox_get_devices),this));

	machine().device("ppc1")->execute().set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	machine().device("ppc2")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	memcpy(machine().device<fujitsu_29f016a_device>("flash")->space().get_read_ptr(0),memregion("user1")->base(),0x200000);
}

void bebox_state::machine_start()
{
	smc37c78_device *fdc = machine().device<smc37c78_device>("smc37c78");
	fdc->setup_intrq_cb(smc37c78_device::line_cb(FUNC(bebox_state::fdc_interrupt), this));
	fdc->setup_drq_cb(smc37c78_device::line_cb(FUNC(bebox_state::fdc_dma_drq), this));
}

DRIVER_INIT_MEMBER(bebox_state,bebox)
{
	address_space &space_0 = machine().device("ppc1")->memory().space(AS_PROGRAM);
	address_space &space_1 = machine().device("ppc2")->memory().space(AS_PROGRAM);

	/* set up boot and flash ROM */
	membank("bank2")->set_base(machine().root_device().memregion("user2")->base());

	/* install MESS managed RAM */
	space_0.install_readwrite_bank(0, machine().device<ram_device>(RAM_TAG)->size() - 1, 0, 0x02000000, "bank3");
	space_1.install_readwrite_bank(0, machine().device<ram_device>(RAM_TAG)->size() - 1, 0, 0x02000000, "bank3");
	membank("bank3")->set_base(machine().device<ram_device>(RAM_TAG)->pointer());

	kbdc8042_init(machine(), &bebox_8042_interface);

	/* The following is a verrrry ugly hack put in to support NetBSD for
     * NetBSD.  When NetBSD/bebox it does most of its work on CPU #0 and then
     * lets CPU #1 go.  However, it seems that CPU #1 jumps into never-never
     * land, crashes, and then goes into NetBSD's crash handler which catches
     * it.  The current PowerPC core cannot catch this trip into never-never
     * land properly, and MESS crashes.  In the interim, this "mitten" catches
     * the crash
     */
	{
		static UINT64 ops[2] =
		{
			/* li r0, 0x0700 */
			/* mtspr ctr, r0 */
			U64(0x380007007C0903A6),

			/* bcctr 0x14, 0 */
			U64(0x4E80042000000000)
		};
		space_1.install_read_bank(0x9421FFF0, 0x9421FFFF, "bank1");
		membank("bank1")->set_base(ops);
	}
}
