// license:BSD-3-Clause
// copyright-holders:Melissa Goad

// ARM PrimeCell PL192 VIC emulation

#include "emu.h"
#include "machine/vic_pl192.h"

#define LOG_GENERAL (1U << 0)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

void vic_pl192_device::set_irq_line(int irq, int state)
{
	u32 mask = (1 << irq);

    if(state)
    {
        raw_intr |= mask;
    }
    else
    {
        raw_intr &= ~mask;
    }

    if(u32 intrs = (raw_intr | soft_intr) & intr_en)
    {
        if(intrs & intr_select) m_out_fiq_func(1);
        else m_out_fiq_func(0);

        if(intrs & ~intr_select) m_out_irq_func(1);
        else m_out_irq_func(0);
    }
}

uint32_t vic_pl192_device::read(offs_t offset)
{
    offset &= 0xffc;
    switch(offset)
    {
        case 0x000:
        {
            return raw_intr & ~intr_select; // IRQ_STATUS
        }
        case 0x004:
        {
            return raw_intr & intr_select; // FIQ_STATUS
        }
        case 0x008:
        {
            return raw_intr;
        }
        case 0x00c:
        {
            return intr_select;
        }
        case 0x010:
        {
            return intr_en;
        }
        case 0x018:
        {
            return soft_intr;
        }
        case 0x020:
        {
            return protection;
        }
        case 0x024:
        {
            return sw_priority_mask;
        }
        case 0x028:
        {
            return daisy_priority;
        }
        case 0xf00:
        {
            return vicaddress;
        }
        case 0xfe0:
        {
            return periph_id[0];
        }
        case 0xfe4:
        {
            return periph_id[1];
        }
        case 0xfe8:
        {
            return periph_id[2];
        }
        case 0xfec:
        {
            return periph_id[3];
        }
        case 0xff0:
        {
            return pcell_id[0];
        }
        case 0xff4:
        {
            return pcell_id[1];
        }
        case 0xff8:
        {
            return pcell_id[2];
        }
        case 0xffc:
        {
            return pcell_id[3];
        }
    }
    if((offset >= 0x100) && (offset < 0x180))
    {
        return vectaddr[(offset & 0x7c) >> 2];
    }
    if((offset >= 0x200) && (offset < 0x280))
    {
        return vectprio[(offset & 0x7c) >> 2];
    }
    return 0;
}

void vic_pl192_device::write(offs_t offset, u32 data)
{
    offset &= 0xffc;
    switch(offset)
    {
        case 0x00c:
        {
            intr_select = data;
            break;
        }
        case 0x010:
        {
            intr_en = data;
            break;
        }
        case 0x014:
        {
            intr_en &= ~data;
            break;
        }
        case 0x018:
        {
            soft_intr = data;
            break;
        }
        case 0x01c:
        {
            soft_intr &= ~data;
            break;
        }
        case 0x020:
        {
            protection = data & 1;
            break;
        }
        case 0x024:
        {
            sw_priority_mask = data;
            break;
        }
        case 0x028:
        {
            daisy_priority = data & 0xf;
            break;
        }
    }
    if((offset >= 0x100) && (offset < 0x180))
    {
        vectaddr[(offset & 0x7c) >> 2] = data;
    }
    if((offset >= 0x200) && (offset < 0x280))
    {
        vectprio[(offset & 0x7c) >> 2] = data & 0xf;
    }
}

void vic_pl192_device::device_start()
{
	save_item(NAME(raw_intr));
	save_item(NAME(intr_select));
	save_item(NAME(intr_en));
	save_item(NAME(soft_intr));
	save_item(NAME(vectaddr));
	save_item(NAME(vicaddress));
	save_item(NAME(protection));
	save_item(NAME(sw_priority_mask));
	save_item(NAME(daisy_priority));
	save_item(NAME(vectprio));
}

void vic_pl192_device::device_reset()
{
	raw_intr = intr_select = intr_en = soft_intr = vicaddress = protection = 0;
	sw_priority_mask = 0xffff;
	daisy_priority = 0xf;

	for(int i = 0; i < 32; i++)
	{
		vectaddr[i] = 0;
	}

	for(int i = 0; i < 32; i++)
	{
		vectprio[i] = 0xf;
	}
}

DEFINE_DEVICE_TYPE(PL192_VIC, vic_pl192_device, "vic_pl192", "ARM PL192 VIC")

vic_pl192_device::vic_pl192_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_out_irq_func(*this)
	, m_out_fiq_func(*this)
	, periph_id{0x92, 0x11, 0x04, 0x00}
	, pcell_id{0x0d, 0xf0, 0x05, 0xb1}
{
}

vic_pl192_device::vic_pl192_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vic_pl192_device(mconfig, PL192_VIC, tag, owner, clock)
{
}