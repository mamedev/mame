/* This is currently unused, video/gticlub.c has it's own implementation, why? */


#include "emu.h"
#include "k001006.h"

/*****************************************************************************/
/* Konami K001006 Custom 3D Texel Renderer chip (KS10081) */

/***************************************************************************/
/*                                                                         */
/*                                  001006                                 */
/*                                                                         */
/***************************************************************************/

const device_type K001006 = &device_creator<k001006_device>;

k001006_device::k001006_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K001006, "Konami 001006", tag, owner, clock, "k001006", __FILE__),
	m_pal_ram(NULL),
	m_unknown_ram(NULL),
	m_addr(0),
	m_device_sel(0),
	m_palette(NULL)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k001006_device::device_config_complete()
{
	// inherit a copy of the static data
	const k001006_interface *intf = reinterpret_cast<const k001006_interface *>(static_config());
	if (intf != NULL)
	*static_cast<k001006_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		m_gfx_region = "";
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k001006_device::device_start()
{
	m_pal_ram = auto_alloc_array_clear(machine(), UINT16, 0x800);
	m_unknown_ram = auto_alloc_array_clear(machine(), UINT16, 0x1000);
	m_palette = auto_alloc_array_clear(machine(), UINT32, 0x800);

	save_pointer(NAME(m_pal_ram), 0x800*sizeof(UINT16));
	save_pointer(NAME(m_unknown_ram), 0x1000*sizeof(UINT16));
	save_pointer(NAME(m_palette), 0x800*sizeof(UINT32));
	save_item(NAME(m_device_sel));
	save_item(NAME(m_addr));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k001006_device::device_reset()
{
	m_addr = 0;
	m_device_sel = 0;
	memset(m_palette, 0, 0x800*sizeof(UINT32));
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ32_MEMBER( k001006_device::read )
{
	if (offset == 1)
	{
		switch (m_device_sel)
		{
			case 0x0b:      // CG Board ROM read
			{
				UINT16 *rom = (UINT16*)space.machine().root_device().memregion(m_gfx_region)->base();
				return rom[m_addr / 2] << 16;
			}
			case 0x0d:      // Palette RAM read
			{
				UINT32 addr = m_addr;

				m_addr += 2;
				return m_pal_ram[addr >> 1];
			}
			case 0x0f:      // Unknown RAM read
			{
				return m_unknown_ram[m_addr++];
			}
			default:
			{
				fatalerror("k001006_r, unknown device %02X\n", m_device_sel);
			}
		}
	}
	return 0;
}

WRITE32_MEMBER( k001006_device::write )
{
	if (offset == 0)
	{
		COMBINE_DATA(&m_addr);
	}
	else if (offset == 1)
	{
		switch (m_device_sel)
		{
			case 0xd:   // Palette RAM write
			{
				int r, g, b, a;
				UINT32 index = m_addr;

				m_pal_ram[index >> 1] = data & 0xffff;

				a = (data & 0x8000) ? 0x00 : 0xff;
				b = ((data >> 10) & 0x1f) << 3;
				g = ((data >>  5) & 0x1f) << 3;
				r = ((data >>  0) & 0x1f) << 3;
				b |= (b >> 5);
				g |= (g >> 5);
				r |= (r >> 5);
				m_palette[index >> 1] = rgb_t(a, r, g, b);

				m_addr += 2;
				break;
			}
			case 0xf:   // Unknown RAM write
			{
			//  mame_printf_debug("Unknown RAM %08X = %04X\n", m_addr, data & 0xffff);
				m_unknown_ram[m_addr++] = data & 0xffff;
				break;
			}
			default:
			{
				mame_printf_debug("k001006_w: device %02X, write %04X to %08X\n", m_device_sel, data & 0xffff, m_addr++);
			}
		}
	}
	else if (offset == 2)
	{
		if (ACCESSING_BITS_16_31)
		{
			m_device_sel = (data >> 16) & 0xf;
		}
	}
}

UINT32 k001006_device::get_palette( int index )
{
	return m_palette[index];
}
