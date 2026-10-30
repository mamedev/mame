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
        #11     Intel 82378ZB PCI/ISA bridge
        #12     NCR 53C810 SCSI

    Super I/O is an Intel 82091AA AIP

    More hardware information at http://www.netbsd.org/Ports/bebox/hardware.html

***************************************************************************/

/* Core includes */
#include "emu.h"
#include "bebox.h"

/* Components */
#include "cpu/powerpc/ppc.h"
#include "machine/mc146818.h"
#include "bus/ata/ataintf.h"

#define LOG_CPUIMASK   (1U << 1)
#define LOG_INTERRUPTS (1U << 2)

#define VERBOSE (LOG_CPUIMASK | LOG_INTERRUPTS)
#include "logmacro.h"


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
		LOGMASKED(LOG_CPUIMASK, "%s BeBox CPU #0 imask=0x%08x\n",
			machine().describe_context(), m_cpu_imask[0]);
		bebox_update_interrupts();
	}
}

void bebox_state::bebox_cpu1_imask_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	uint32_t old_imask = m_cpu_imask[1];

	bebox_mbreg32_w(&m_cpu_imask[1], data, mem_mask);

	if (old_imask != m_cpu_imask[1])
	{
		LOGMASKED(LOG_CPUIMASK, "%s BeBox CPU #1 imask=0x%08x\n",
			machine().describe_context(), m_cpu_imask[1]);
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
	// SMI is separate from the normal interrupt input. TLBISYNC (bits
	// 27/26, active low) is retained for readback. The CPU core has no
	// TLBISYNC input; these bits must not drive the ordinary IRQ line.
	bebox_mbreg32_w(&m_crossproc_interrupts, data, mem_mask);
	bebox_update_interrupts();
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

		LOGMASKED(LOG_INTERRUPTS, "\tbebox_update_interrupts(): CPU #%d [%08X|%08X] IRQ %s\n", cpunum,
			m_interrupts, m_cpu_imask[cpunum], interrupt ? "on" : "off");

		m_ppc[cpunum]->set_input_line(INPUT_LINE_IRQ0, interrupt ? ASSERT_LINE : CLEAR_LINE);
		u32 const smi_mask = 0x40000000U >> cpunum;
		m_ppc[cpunum]->set_input_line(PPC_SMI,
			(m_crossproc_interrupts & m_cpu_imask[cpunum] & smi_mask) ? ASSERT_LINE : CLEAR_LINE);
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

	/* make sure that we don't shoot ourself in the foot */
	if ((interrupt_bit >= std::size(interrupt_names)) || !interrupt_names[interrupt_bit])
		throw emu_fatalerror("bebox_state::bebox_set_irq_bit: Raising invalid interrupt");

	LOGMASKED(LOG_INTERRUPTS, "bebox_set_irq_bit(): pc[0]=0x%08x pc[1]=0x%08x %s interrupt #%u (%s)\n",
		unsigned(m_ppc[0]->pc()),
		unsigned(m_ppc[1]->pc()),
		val ? "Asserting" : "Clearing",
		interrupt_bit, interrupt_names[interrupt_bit]);

	old_interrupts = m_interrupts;
	if (val)
		m_interrupts |= 1 << interrupt_bit;
	else
		m_interrupts &= ~(1 << interrupt_bit);

	/* if interrupt values have changed, update the lines */
	if (m_interrupts != old_interrupts)
		bebox_update_interrupts();
}

uint64_t bebox_state::bebox_interrupt_ack_r()
{
	return u64(m_sio->acknowledge()) << 56;
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

// The I/O card presents a parallel interface to the serial MAX186.
// Firmware supplies SEL/UNI/SGL in bits 6:2; the glue supplies START
// and the serial clock. Bit 11 reports that the result has been latched.
// TODO: Verify the glue clock and implement continuous/rate/IRQ modes
// (0x364 and the remaining control bits) from the I/O card schematics.
u16 bebox_state::adc_control_r()
{
	return (m_adc_control & ~0x0800) | (m_adc_done ? 0x0800 : 0);
}

void bebox_state::adc_control_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_adc_control);
	if (ACCESSING_BITS_0_7)
	{
		m_adc_done = false;
		m_adc_shift = 0;
		m_adc_cycle = 0;
		m_adc->cs_w(1);
		m_adc->sclk_w(0);
		m_adc->cs_w(0);
		// Approximate the board serializer with a 2 MHz external clock.
		m_adc_timer->adjust(attotime::from_nsec(500), 0, attotime::from_nsec(500));
	}
}

TIMER_CALLBACK_MEMBER(bebox_state::adc_tick)
{
	++m_adc_cycle;
	u8 const command = (m_adc_control & 0x7c) | 0x83;
	m_adc->din_w(m_adc_cycle <= 8 ? BIT(command, 8 - m_adc_cycle) : 0);
	m_adc->sclk_w(1);
	m_adc->sclk_w(0);
	if (m_adc_cycle >= 9 && m_adc_cycle <= 20)
		m_adc_shift = (m_adc_shift << 1) | m_adc->dout_r();
	if (m_adc_cycle == 24)
	{
		m_adc_result = m_adc_shift;
		m_adc_done = true;
		m_adc->cs_w(1);
		m_adc_timer->adjust(attotime::never);
	}
}

double bebox_state::adc_input(u8 channel)
{
	if (channel == 4 || channel == 5)
		return double(m_joy_axes[(channel - 4) * 4 + BIT(m_joy_mux, 8, 2)]->read()) / 4096.0;
	// R3 distinguishes I/O card revisions 4 and 5 using channel 6.
	// Model revision 5 with its identification input pulled high.
	// The four GeekPort inputs and unused channel 7 are tied to ground
	// until external analog peripherals are supported.
	return channel == 6 ? 1.0 : 0.0;
}

/*************************************
 *
 *  Driver main
 *
 *************************************/

void bebox_state::machine_reset()
{
	m_adc_timer->adjust(attotime::never);
	m_adc_control = m_adc_result = m_adc_shift = m_joy_mux = 0;
	m_adc_cycle = 0;
	m_adc_done = false;
	m_cpu_imask[0] = m_cpu_imask[1] = 0;
	m_crossproc_interrupts = 0;
	bebox_update_interrupts();
	m_adc->cs_w(1);
	m_ppc[0]->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	m_ppc[1]->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	// Endianness? Bah!
	memcpy(m_flash->base(),memregion("user1")->base(),0x200000);
}

void bebox_state::machine_start()
{
	m_adc_timer = timer_alloc(FUNC(bebox_state::adc_tick), this);
	save_item(NAME(m_adc_control));
	save_item(NAME(m_adc_result));
	save_item(NAME(m_adc_shift));
	save_item(NAME(m_joy_mux));
	save_item(NAME(m_adc_cycle));
	save_item(NAME(m_adc_done));
	save_item(NAME(m_cpu_imask));
	save_item(NAME(m_interrupts));
	save_item(NAME(m_crossproc_interrupts));
	m_mpc105->set_ram_info(m_ram->pointer(), m_ram->size());

	// board devices outside the 82378/82091 pair sit on the ISA side
	m_isabus->space(isa16_device::AS_ISA_IO).install_device(0x0000, 0xffff, *this, &bebox_state::io_map);
}

void bebox_state::init_bebox()
{
	address_space &space_1 = m_ppc[1]->space(AS_PROGRAM);

	/* set up boot and flash ROM */
	membank("bank2")->set_base(memregion("user2")->base());

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
