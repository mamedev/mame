// license:BSD-3-Clause
// copyright-holders: Aaron Giles, Ted Green
#include "gt64xxx.h"

/*************************************
 *
 *  Debugging constants
 *
 *************************************/
#define LOG_GALILEO         (0)
#define LOG_REG             (0)
#define LOG_TIMERS          (0)
#define LOG_DMA             (0)

const device_type GT64XXX      = &device_creator<gt64xxx_device>;

DEVICE_ADDRESS_MAP_START(config_map, 32, gt64xxx_device)
	AM_INHERIT_FROM(pci_device::config_map)
ADDRESS_MAP_END

// cpu i/f map
DEVICE_ADDRESS_MAP_START(cpu_map, 32, gt64xxx_device)
	AM_RANGE(0x00000000, 0x00000cff) AM_READWRITE(    cpu_if_r,          cpu_if_w)
ADDRESS_MAP_END

gt64xxx_device::gt64xxx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_host_device(mconfig, GT64XXX, "Galileo GT-64XXX System Controller", tag, owner, clock, "gt64xxx", __FILE__),
		m_be(0), m_autoconfig(0), m_irq_num(-1),
		m_mem_config("memory_space", ENDIANNESS_LITTLE, 32, 32),
		m_io_config("io_space", ENDIANNESS_LITTLE, 32, 32),
		m_region(*this, DEVICE_SELF)
{
}

const address_space_config *gt64xxx_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_PROGRAM) ? pci_bridge_device::memory_space_config(spacenum) : (spacenum == AS_DATA) ? &m_mem_config : (spacenum == AS_IO) ? &m_io_config : NULL;
}

void gt64xxx_device::device_start()
{
	pci_host_device::device_start();
	m_cpu = machine().device<mips3_device>(cpu_tag);
	m_cpu_space = &m_cpu->space(AS_PROGRAM);
	memory_space = &space(AS_DATA);
	io_space = &space(AS_IO);

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffffffff;
	io_offset       = 0x00000000;
	status = 0x0;

	// ROM size = 4 MB
	m_cpu_space->install_rom   (0x1fc00000, 0x1fffffff, m_region->base());

	// MIPS drc
	m_cpu->add_fastram(0x1fc00000, 0x1fffffff, TRUE, m_region->base());
}

void gt64xxx_device::device_reset()
{
	pci_device::device_reset();

	// Configuration register defaults
	m_reg[GREG_CPU_CONFIG] = m_be ? 0 : (1<<12);
	m_reg[GREG_RAS_1_0_LO] = 0x0;
	m_reg[GREG_RAS_1_0_HI] = 0x7;
	m_reg[GREG_RAS_3_2_LO] = 0x8;
	m_reg[GREG_RAS_3_2_HI] = 0xf;
	m_reg[GREG_CS_2_0_LO] = 0xe0;
	m_reg[GREG_CS_2_0_HI] = 0x70;
	m_reg[GREG_CS_3_BOOT_LO] = 0xf8;
	m_reg[GREG_CS_3_BOOT_HI] = 0x7f;
	m_reg[GREG_PCI_IO_LO] = 0x80;
	m_reg[GREG_PCI_IO_HI] = 0xf;
	m_reg[GREG_PCI_MEM0_LO] = 0x90;
	m_reg[GREG_PCI_MEM0_HI] = 0x1f;
	m_reg[GREG_INTERNAL_SPACE] = 0xa0;
	m_reg[GREG_PCI_MEM1_LO] = 0x790;
	m_reg[GREG_PCI_MEM1_HI] = 0x1f;

	m_reg[GREG_RAS0_LO] = 0x0;
	m_reg[GREG_RAS0_HI] = 0x7;
	m_reg[GREG_RAS1_LO] = 0x8;
	m_reg[GREG_RAS1_HI] = 0xf;
	m_reg[GREG_RAS2_LO] = 0x10;
	m_reg[GREG_RAS2_HI] = 0x17;
	m_reg[GREG_RAS3_LO] = 0x18;
	m_reg[GREG_RAS3_HI] = 0x1f;
	m_reg[GREG_CS0_LO] = 0xc0;
	m_reg[GREG_CS0_HI] = 0xc7;
	m_reg[GREG_CS1_LO] = 0xc8;
	m_reg[GREG_CS1_HI] = 0xcf;
	m_reg[GREG_CS2_LO] = 0xd0;
	m_reg[GREG_CS2_HI] = 0xdf;
	m_reg[GREG_CS3_LO] = 0xf0;
	m_reg[GREG_CS3_HI] = 0xfb;
	m_reg[GREG_CSBOOT_LO] = 0xfc;
	m_reg[GREG_CSBOOT_HI] = 0xff;

	m_reg[GREG_PCI_COMMAND] = m_be ? 0 : 1;

	map_cpu_space();
	regenerate_config_mapping();
}

void gt64xxx_device::map_cpu_space()
{
	UINT32 winStart, winEnd;

	// ROM region starts at 0x1fc00000
	m_cpu_space->unmap_readwrite(0x00000000, 0x1fbfffff);
	m_cpu_space->unmap_readwrite(0x20000000, 0xffffffff);

	// Clear fastram regions in cpu after rom
	m_cpu->clear_fastram(1);

	// CPU Regs
	winStart = m_reg[GREG_INTERNAL_SPACE]<<21;
	winEnd = winStart + sizeof(m_reg) - 1;
	m_cpu_space->install_device(winStart, winEnd, *static_cast<gt64xxx_device *>(this), &gt64xxx_device::cpu_map);
	if (LOG_GALILEO)
		logerror("%s: map_cpu_space cpu_reg start: %08X end: %08X\n", tag(), winStart, winEnd);

	// Ras0
	winStart = (m_reg[GREG_RAS_1_0_LO]<<21) | (m_reg[GREG_RAS0_LO]<<20);
	winEnd =   (m_reg[GREG_RAS_1_0_LO]<<21) | (m_reg[GREG_RAS0_HI]<<20) | 0xfffff;
	m_ram[0].resize((winEnd+1-winStart)/4);
	m_cpu_space->install_ram(winStart, winEnd, &m_ram[0][0]);
	m_cpu->add_fastram(winStart, m_ram[0].size()*sizeof(m_ram[0][0]), FALSE, &m_ram[0][0]);
	if (LOG_GALILEO)
		logerror("%s: map_cpu_space ras0 start: %08X end: %08X\n", tag(), winStart, winEnd);

	// PCI IO Window
	winStart = m_reg[GREG_PCI_IO_LO]<<21;
	winEnd =   (m_reg[GREG_PCI_IO_LO]<<21) | (m_reg[GREG_PCI_IO_HI]<<21) | 0x1fffff;
	m_cpu_space->install_read_handler(winStart, winEnd, 0, 0, read32_delegate(FUNC(gt64xxx_device::master_io_r), this));
	m_cpu_space->install_write_handler(winStart, winEnd, 0, 0, write32_delegate(FUNC(gt64xxx_device::master_io_w), this));
	if (LOG_GALILEO)
		logerror("%s: map_cpu_space pci_io start: %08X end: %08X\n", tag(), winStart, winEnd);

	// PCI MEM0 Window
	winStart = m_reg[GREG_PCI_MEM0_LO]<<21;
	winEnd =   (m_reg[GREG_PCI_MEM0_LO]<<21) | (m_reg[GREG_PCI_MEM0_HI]<<21) | 0x1fffff;
	m_cpu_space->install_read_handler(winStart, winEnd, 0, 0, read32_delegate(FUNC(gt64xxx_device::master_mem0_r), this));
	m_cpu_space->install_write_handler(winStart, winEnd, 0, 0, write32_delegate(FUNC(gt64xxx_device::master_mem0_w), this));
	if (LOG_GALILEO)
		logerror("%s: map_cpu_space pci_mem0 start: %08X end: %08X\n", tag(), winStart, winEnd);

	// PCI MEM1 Window
	winStart = m_reg[GREG_PCI_MEM1_LO]<<21;
	winEnd =   (m_reg[GREG_PCI_MEM1_LO]<<21) | (m_reg[GREG_PCI_MEM1_HI]<<21) | 0x1fffff;
	m_cpu_space->install_read_handler(winStart, winEnd, 0, 0, read32_delegate(FUNC(gt64xxx_device::master_mem1_r), this));
	m_cpu_space->install_write_handler(winStart, winEnd, 0, 0, write32_delegate(FUNC(gt64xxx_device::master_mem1_w), this));
	if (LOG_GALILEO)
		logerror("%s: map_cpu_space pci_mem1 start: %08X end: %08X\n", tag(), winStart, winEnd);

}

void gt64xxx_device::map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
									UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space)
{
	/*
	UINT32 winStart, winEnd, winSize;

	// PCI Target Window 1
	if (m_cpu_regs[NREG_PCITW1]&0x1000) {
	    winStart = m_cpu_regs[NREG_PCITW1]&0xffe00000;
	    winEnd = winStart | (~(0xf0000000 | (((m_cpu_regs[NREG_PCITW1]>>13)&0x7f)<<21)));
	    winSize = winEnd - winStart + 1;
	    memory_space->install_read_handler(winStart, winEnd, 0, 0, read32_delegate(FUNC(gt64xxx_device::target1_r), this));
	    memory_space->install_write_handler(winStart, winEnd, 0, 0, write32_delegate(FUNC(gt64xxx_device::target1_w), this));
	    if (LOG_GALILEO)
	        logerror("%s: map_extra Target Window 1 start=%08X end=%08X size=%08X laddr=%08X\n", tag(), winStart, winEnd, winSize,  m_target1_laddr);
	}
	*/
}

void gt64xxx_device::reset_all_mappings()
{
	pci_device::reset_all_mappings();
}

// PCI bus control
READ32_MEMBER (gt64xxx_device::pci_config_r)
{
	UINT32 result = 0;
	if (LOG_GALILEO)
		logerror("%06X:galileo pci_config_r from offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, result, mem_mask);
	return result;
}
WRITE32_MEMBER (gt64xxx_device::pci_config_w)
{
	if (LOG_GALILEO)
		logerror("%06X:galileo pci_config_w to offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
}
// PCI Master Window 0
READ32_MEMBER (gt64xxx_device::master_mem0_r)
{
	UINT32 result = this->space(AS_DATA).read_dword((m_reg[GREG_PCI_MEM0_LO]<<21) | (offset*4), mem_mask);
	if (LOG_GALILEO)
		logerror("%06X:galileo pci mem0 read from offset %08X = %08X & %08X\n", space.device().safe_pc(), (m_reg[GREG_PCI_MEM0_LO]<<21) | (offset*4), result, mem_mask);
	return result;
}
WRITE32_MEMBER (gt64xxx_device::master_mem0_w)
{
	this->space(AS_DATA).write_dword((m_reg[GREG_PCI_MEM0_LO]<<21) | (offset*4), data, mem_mask);
	if (LOG_GALILEO)
		logerror("%06X:galileo pci mem0 write to offset %08X = %08X & %08X\n", space.device().safe_pc(), (m_reg[GREG_PCI_MEM0_LO]<<21) | (offset*4), data, mem_mask);
}

// PCI Master Window 1
READ32_MEMBER (gt64xxx_device::master_mem1_r)
{
	UINT32 result = this->space(AS_DATA).read_dword((m_reg[GREG_PCI_MEM1_LO]<<21) | (offset*4), mem_mask);
	if (LOG_GALILEO)
		logerror("%06X:galileo pci mem1 read from offset %08X = %08X & %08X\n", space.device().safe_pc(), (m_reg[GREG_PCI_MEM1_LO]<<21) | (offset*4), result, mem_mask);
	return result;
}
WRITE32_MEMBER (gt64xxx_device::master_mem1_w)
{
	this->space(AS_DATA).write_dword((m_reg[GREG_PCI_MEM1_LO]<<21) | (offset*4), data, mem_mask);
	if (LOG_GALILEO)
		logerror("%06X:galileo pci mem1 write to offset %08X = %08X & %08X\n", space.device().safe_pc(), (m_reg[GREG_PCI_MEM1_LO]<<21) | (offset*4), data, mem_mask);
}

// PCI Master IO
READ32_MEMBER (gt64xxx_device::master_io_r)
{
	UINT32 result = this->space(AS_IO).read_dword((m_reg[GREG_PCI_IO_LO]<<21) | (offset*4), mem_mask);
	if (LOG_GALILEO)
		logerror("%06X:galileo pci io read from offset %08X = %08X & %08X\n", space.device().safe_pc(), (m_reg[GREG_PCI_IO_LO]<<21) | (offset*4), result, mem_mask);
	return result;
}
WRITE32_MEMBER (gt64xxx_device::master_io_w)
{
	this->space(AS_IO).write_dword((m_reg[GREG_PCI_IO_LO]<<21) | (offset*4), data, mem_mask);
	if (LOG_GALILEO)
		logerror("%06X:galileo pciio write to offset %08X = %08X & %08X\n", space.device().safe_pc(), (m_reg[GREG_PCI_IO_LO]<<21) | (offset*4), data, mem_mask);
}


// CPU I/F
READ32_MEMBER (gt64xxx_device::cpu_if_r)
{
	UINT32 result = m_reg[offset];

	/* switch off the offset for special cases */
	switch (offset)
	{
		case GREG_TIMER0_COUNT:
		case GREG_TIMER1_COUNT:
		case GREG_TIMER2_COUNT:
		case GREG_TIMER3_COUNT:
		{
			int which = offset % 4;
			galileo_timer *timer = &m_timer[which];

			result = timer->count;
			if (timer->active)
			{
				UINT32 elapsed = (timer->timer->elapsed() * m_clock).as_double();
				result = (result > elapsed) ? (result - elapsed) : 0;
			}

			/* eat some time for those which poll this register */
			space.device().execute().eat_cycles(100);

			if (LOG_TIMERS)
				logerror("%08X:hires_timer_r = %08X\n", space.device().safe_pc(), result);
			break;
		}

		case GREG_PCI_COMMAND:
			// code at 40188 loops until this returns non-zero in bit 0
			//result = 0x0001;
			break;

		case GREG_CONFIG_DATA:
			result = config_data_r(space, offset);
			break;

		case GREG_CONFIG_ADDRESS:
			result = config_address_r(space, offset);
			break;

		case GREG_INT_STATE:
		case GREG_INT_MASK:
		case GREG_TIMER_CONTROL:
//          if (LOG_GALILEO)
//              logerror("%08X:Galileo read from offset %03X = %08X\n", space.device().safe_pc(), offset*4, result);
			break;

		default:
			logerror("%08X:Galileo read from offset %03X = %08X\n", space.device().safe_pc(), offset*4, result);
			break;
	}

	if (m_be) result =  FLIPENDIAN_INT32(result);

	return result;
}

WRITE32_MEMBER(gt64xxx_device::cpu_if_w)
{
	if (m_be) {
		data = FLIPENDIAN_INT32(data);
		mem_mask = FLIPENDIAN_INT32(mem_mask);
	}

	UINT32 oldata = m_reg[offset];
	COMBINE_DATA(&m_reg[offset]);

	/* switch off the offset for special cases */
	switch (offset)
	{
		case GREG_RAS_1_0_LO:
		case GREG_RAS_1_0_HI:
		case GREG_RAS_3_2_LO:
		case GREG_RAS_3_2_HI:
		case GREG_CS_2_0_LO:
		case GREG_CS_2_0_HI:
		case GREG_CS_3_BOOT_LO:
		case GREG_CS_3_BOOT_HI:
		case GREG_PCI_IO_LO:
		case GREG_PCI_IO_HI:
		case GREG_PCI_MEM0_LO:
		case GREG_PCI_MEM0_HI:
		case GREG_INTERNAL_SPACE:
		case GREG_PCI_MEM1_LO:
		case GREG_PCI_MEM1_HI:
			map_cpu_space();
			if (LOG_GALILEO)
				logerror("%08X:Galileo Memory Map data write to offset %03X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
			break;

		case GREG_DMA0_CONTROL:
		case GREG_DMA1_CONTROL:
		case GREG_DMA2_CONTROL:
		case GREG_DMA3_CONTROL:
		{
			int which = offset % 4;

			if (LOG_DMA)
				logerror("%08X:Galileo write to offset %03X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);

			/* keep the read only activity bit */
			m_reg[offset] &= ~0x4000;
			m_reg[offset] |= (oldata & 0x4000);

			/* fetch next record */
			if (data & 0x2000)
				dma_fetch_next(space, which);
			m_reg[offset] &= ~0x2000;

			/* if enabling, start the DMA */
			if (!(oldata & 0x1000) && (data & 0x1000))
				perform_dma(space, which);
			break;
		}

		case GREG_TIMER0_COUNT:
		case GREG_TIMER1_COUNT:
		case GREG_TIMER2_COUNT:
		case GREG_TIMER3_COUNT:
		{
			int which = offset % 4;
			galileo_timer *timer = &m_timer[which];

			if (which != 0)
				data &= 0xffffff;
			if (!timer->active)
				timer->count = data;
			if (LOG_TIMERS)
				logerror("%08X:timer/counter %d count = %08X [start=%08X]\n", space.device().safe_pc(), offset % 4, data, timer->count);
			break;
		}

		case GREG_TIMER_CONTROL:
		{
			int which, mask;

			if (LOG_TIMERS)
				logerror("%08X:timer/counter control = %08X\n", space.device().safe_pc(), data);
			for (which = 0, mask = 0x01; which < 4; which++, mask <<= 2)
			{
				galileo_timer *timer = &m_timer[which];
				if (!timer->active && (data & mask))
				{
					timer->active = 1;
					if (timer->count == 0)
					{
						timer->count = m_reg[GREG_TIMER0_COUNT + which];
						if (which != 0)
							timer->count &= 0xffffff;
					}
					timer->timer->adjust(TIMER_PERIOD * timer->count, which);
					if (LOG_TIMERS)
						logerror("Adjusted timer to fire in %f secs\n", (TIMER_PERIOD * timer->count).as_double());
				}
				else if (timer->active && !(data & mask))
				{
					UINT32 elapsed = (timer->timer->elapsed() * m_clock).as_double();
					timer->active = 0;
					timer->count = (timer->count > elapsed) ? (timer->count - elapsed) : 0;
					timer->timer->adjust(attotime::never, which);
					if (LOG_TIMERS)
						logerror("Disabled timer\n");
				}
			}
			break;
		}

		case GREG_INT_STATE:
			if (LOG_GALILEO)
				logerror("%08X:Galileo write to IRQ clear = %08X & %08X\n", offset*4, data, mem_mask);
			m_reg[offset] = oldata & data;
			update_irqs();
			break;

		case GREG_CONFIG_DATA:
			pci_host_device::config_data_w(space, offset, data);
			if (LOG_GALILEO)
				logerror("%08X:Galileo PCI config data write to offset %03X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
			break;

		case GREG_CONFIG_ADDRESS:
			pci_host_device::config_address_w(space, offset, data);
			if (LOG_GALILEO)
				logerror("%08X:Galileo PCI config address write to offset %03X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
			break;

		case GREG_DMA0_COUNT:   case GREG_DMA1_COUNT:   case GREG_DMA2_COUNT:   case GREG_DMA3_COUNT:
		case GREG_DMA0_SOURCE:  case GREG_DMA1_SOURCE:  case GREG_DMA2_SOURCE:  case GREG_DMA3_SOURCE:
		case GREG_DMA0_DEST:    case GREG_DMA1_DEST:    case GREG_DMA2_DEST:    case GREG_DMA3_DEST:
		case GREG_DMA0_NEXT:    case GREG_DMA1_NEXT:    case GREG_DMA2_NEXT:    case GREG_DMA3_NEXT:
		case GREG_INT_MASK:
			if (LOG_GALILEO)
				logerror("%08X:Galileo write to offset %03X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
			break;

		default:
			logerror("%08X:Galileo write to offset %03X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
			break;
	}
}

/*************************************
 *
 *  Galileo timers & interrupts
 *
 *************************************/

void gt64xxx_device::update_irqs()
{
	int state = CLEAR_LINE;

	/* if any unmasked interrupts are live, we generate */
	if (m_reg[GREG_INT_STATE] & m_reg[GREG_INT_MASK])
		state = ASSERT_LINE;
	if (m_irq_num != -1)
		m_cpu->set_input_line(m_irq_num, state);

	if (LOG_GALILEO)
		logerror("Galileo IRQ %s\n", (state == ASSERT_LINE) ? "asserted" : "cleared");
}


TIMER_CALLBACK_MEMBER(gt64xxx_device::timer_callback)
{
	int which = param;
	galileo_timer *timer = &m_timer[which];

	if (LOG_TIMERS)
		logerror("timer %d fired\n", which);

	/* copy the start value from the registers */
	timer->count = m_reg[GREG_TIMER0_COUNT + which];
	if (which != 0)
		timer->count &= 0xffffff;

	/* if we're a timer, adjust the timer to fire again */
	if (m_reg[GREG_TIMER_CONTROL] & (2 << (2 * which)))
		timer->timer->adjust(TIMER_PERIOD * timer->count, which);
	else
		timer->active = timer->count = 0;

	/* trigger the interrupt */
	m_reg[GREG_INT_STATE] |= 1 << (GINT_T0EXP_SHIFT + which);
	update_irqs();
}

/*************************************
 *
 *  Galileo DMA handler
 *
 *************************************/

int gt64xxx_device::dma_fetch_next(address_space &space, int which)
{
	offs_t address = 0;
	UINT32 data;

	/* no-op for unchained mode */
	if (!(m_reg[GREG_DMA0_CONTROL + which] & 0x200))
		address = m_reg[GREG_DMA0_NEXT + which];

	/* if we hit the end address, signal an interrupt */
	if (address == 0)
	{
		if (m_reg[GREG_DMA0_CONTROL + which] & 0x400)
		{
			m_reg[GREG_INT_STATE] |= 1 << (GINT_DMA0COMP_SHIFT + which);
			update_irqs();
		}
		m_reg[GREG_DMA0_CONTROL + which] &= ~0x5000;
		return 0;
	}

	/* fetch the byte count */
	data = space.read_dword(address); address += 4;
	m_reg[GREG_DMA0_COUNT + which] = data;

	/* fetch the source address */
	data = space.read_dword(address); address += 4;
	m_reg[GREG_DMA0_SOURCE + which] = data;

	/* fetch the dest address */
	data = space.read_dword(address); address += 4;
	m_reg[GREG_DMA0_DEST + which] = data;

	/* fetch the next record address */
	data = space.read_dword(address); address += 4;
	m_reg[GREG_DMA0_NEXT + which] = data;
	return 1;
}


void gt64xxx_device::perform_dma(address_space &space, int which)
{
	do
	{
		offs_t srcaddr = m_reg[GREG_DMA0_SOURCE + which];
		offs_t dstaddr = m_reg[GREG_DMA0_DEST + which];
		UINT32 bytesleft = m_reg[GREG_DMA0_COUNT + which] & 0xffff;
		int srcinc, dstinc;

		m_dma_active = which;
		m_reg[GREG_DMA0_CONTROL + which] |= 0x5000;

		/* determine src/dst inc */
		switch ((m_reg[GREG_DMA0_CONTROL + which] >> 2) & 3)
		{
			default:
			case 0:     srcinc = 1;     break;
			case 1:     srcinc = -1;    break;
			case 2:     srcinc = 0;     break;
		}
		switch ((m_reg[GREG_DMA0_CONTROL + which] >> 4) & 3)
		{
			default:
			case 0:     dstinc = 1;     break;
			case 1:     dstinc = -1;    break;
			case 2:     dstinc = 0;     break;
		}

		if (LOG_DMA)
			logerror("Performing DMA%d: src=%08X dst=%08X bytes=%04X sinc=%d dinc=%d\n", which, srcaddr, dstaddr, bytesleft, srcinc, dstinc);

		/* standard transfer */
			while (bytesleft > 0)
			{
				space.write_byte(dstaddr, space.read_byte(srcaddr));
				srcaddr += srcinc;
				dstaddr += dstinc;
				bytesleft--;
			}

		/* not verified, but seems logical these should be updated byte the end */
		m_reg[GREG_DMA0_SOURCE + which] = srcaddr;
		m_reg[GREG_DMA0_DEST + which] = dstaddr;
		m_reg[GREG_DMA0_COUNT + which] = (m_reg[GREG_DMA0_COUNT + which] & ~0xffff) | bytesleft;
		m_dma_active = -1;

		/* if we did not hit zero, punt and return later */
		if (bytesleft != 0)
			return;

		/* interrupt? */
		if (!(m_reg[GREG_DMA0_CONTROL + which] & 0x400))
		{
			m_reg[GREG_INT_STATE] |= 1 << (GINT_DMA0COMP_SHIFT + which);
			update_irqs();
		}
	} while (dma_fetch_next(space, which));

	m_reg[GREG_DMA0_CONTROL + which] &= ~0x5000;
}
