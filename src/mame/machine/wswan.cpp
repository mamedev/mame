// license:BSD-3-Clause
// copyright-holders:Anthony Kruize,Wilbert Pol
/***************************************************************************

  wswan.c

  Machine file to handle emulation of the Bandai WonderSwan.

  Anthony Kruize
  Wilbert Pol

TODO:
  SRAM sizes should be in kbit instead of kbytes(?). This raises a few
  interesting issues:
  - mirror of smaller <64KBYTE/512kbit sram sizes
  - banking when using 1M or 2M sram sizes

***************************************************************************/

#include "includes/wswan.h"
#include "render.h"

#define INTERNAL_EEPROM_SIZE    1024

enum enum_system { TYPE_WSWAN=0, TYPE_WSC };


static const UINT8 ws_portram_init[256] =
{
	0x00, 0x00, 0x00/*?*/, 0xbb, 0x00, 0x00, 0x00, 0x26, 0xfe, 0xde, 0xf9, 0xfb, 0xdb, 0xd7, 0x7f, 0xf5,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x9e, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x99, 0xfd, 0xb7, 0xdf,
	0x30, 0x57, 0x75, 0x76, 0x15, 0x73, 0x70/*77?*/, 0x77, 0x20, 0x75, 0x50, 0x36, 0x70, 0x67, 0x50, 0x77,
	0x57, 0x54, 0x75, 0x77, 0x75, 0x17, 0x37, 0x73, 0x50, 0x57, 0x60, 0x77, 0x70, 0x77, 0x10, 0x73,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
	0x87, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x4f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xdb, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x42, 0x00, 0x83, 0x00,
	0x2f, 0x3f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1,
	0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1,
	0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1,
	0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1
};

/*
    Some fake bios code to initialize some registers and set up some things on the wonderswan.
    The code from f:ffe0 which gets copied to 0:0400 is taken from a wonderswan crystal's initial
    memory settings. Lacking real bios dumps we will use this....

    The setting of SP to 2000h is what's needed to get Wonderswan Colloseum to boot.

    f000:ffc0
    FC             cld
    BC 00 20       mov sp,2000h
    68 00 00       push 0000h
    07             pop es
    68 00 F0       push F000h
    1F             pop ds
    BF 00 04       mov di,0400h
    BE E0 FF       mov si,FFE0h
    B9 10 00       mov cx,0010h
    F3 A4          rep movsb
    B0 2F          mov al,2Fh
    E6 C0          out al,C0h
    EA 00 04 00 00 jmp 0000:0400

    f000:ffe0
    E4 A0          in al, A0h
    0C 01          or al,01h
    E6 A0          out al,A0h
    EA 00 00 FF FF jmp FFFFh:0000h

*/
static const UINT8 ws_fake_bios_code[] = {
	0xfc, 0xbc, 0x00, 0x20, 0x68, 0x00, 0x00, 0x07, 0x68, 0x00, 0xf0, 0x1f, 0xbf, 0x00, 0x04, 0xbe,
	0xe0, 0xff, 0xb9, 0x10, 0x00, 0xf3, 0xa4, 0xb0, 0x2f, 0xe6, 0xc0, 0xea, 0x00, 0x04, 0x00, 0x00,
	0xe4, 0xa0, 0x0c, 0x01, 0xe6, 0xa0, 0xea, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xea, 0xc0, 0xff, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void wswan_state::handle_irqs()
{
	if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_HBLTMR)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_HBLTMR);
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_VBL)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_VBL);
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_VBLTMR)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_VBLTMR);
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_LCMP)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_LCMP);
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_SRX)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_SRX);
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_RTC)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_RTC);
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_KEY)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_KEY);
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_STX)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_STX);
	}
	else
	{
		m_maincpu->set_input_line(0, CLEAR_LINE);
	}
}

void wswan_state::set_irq_line(int irq)
{
	if (m_ws_portram[0xb2] & irq)
	{
		m_ws_portram[0xb6] |= irq;
		handle_irqs();
	}
}

void wswan_state::dma_sound_cb()
{
	if ((m_sound_dma.enable & 0x88) == 0x80)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		/* TODO: Output sound DMA byte */
		port_w(space, 0x89, space.read_byte(m_sound_dma.source));
		m_sound_dma.size--;
		m_sound_dma.source = (m_sound_dma.source + 1) & 0x0fffff;
		if (m_sound_dma.size == 0)
		{
			m_sound_dma.enable &= 0x7F;
		}
	}
}

void wswan_state::clear_irq_line(int irq)
{
	m_ws_portram[0xb6] &= ~irq;
	handle_irqs();
}

void wswan_state::register_save()
{
	save_item(NAME(m_ws_portram));
	save_item(NAME(m_internal_eeprom));
	save_item(NAME(m_bios_disabled));
	save_item(NAME(m_rotate));

	save_item(NAME(m_sound_dma.source));
	save_item(NAME(m_sound_dma.size));
	save_item(NAME(m_sound_dma.enable));

	if (m_cart->exists())
		m_cart->save_nvram();
}

void wswan_state::common_start()
{
	m_ws_bios_bank = std::make_unique<UINT8[]>(0x10000);
	memcpy(m_ws_bios_bank.get() + 0xffc0, ws_fake_bios_code, 0x40);

	register_save();

	machine().device<nvram_device>("nvram")->set_base(m_internal_eeprom, INTERNAL_EEPROM_SIZE);

	if (m_cart->exists())
	{
		// ROM
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x20000, 0x2ffff, read8_delegate(FUNC(ws_cart_slot_device::read_rom20),(ws_cart_slot_device*)m_cart));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x30000, 0x3ffff, read8_delegate(FUNC(ws_cart_slot_device::read_rom30),(ws_cart_slot_device*)m_cart));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x40000, 0xeffff, read8_delegate(FUNC(ws_cart_slot_device::read_rom40),(ws_cart_slot_device*)m_cart));

		// SRAM
		if (m_cart->get_type() == WS_SRAM)
		{
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x10000, 0x1ffff, read8_delegate(FUNC(ws_cart_slot_device::read_ram),(ws_cart_slot_device*)m_cart));
			m_maincpu->space(AS_PROGRAM).install_write_handler(0x10000, 0x1ffff, write8_delegate(FUNC(ws_cart_slot_device::write_ram),(ws_cart_slot_device*)m_cart));
		}
	}
}

void wswan_state::machine_start()
{
	common_start();
	m_system_type = TYPE_WSWAN;
}

MACHINE_START_MEMBER(wswan_state, wscolor)
{
	common_start();
	m_system_type = TYPE_WSC;
}

void wswan_state::machine_reset()
{
	m_bios_disabled = 0;

	if (m_cart->exists())
		m_rotate = m_cart->get_is_rotated();
	else
		m_rotate = 0;

	/* Intialize ports */
	memcpy(m_ws_portram, ws_portram_init, 256);

	render_target *target = machine().render().first_target();
	target->set_view(m_rotate);

	/* Initialize sound DMA */
	memset(&m_sound_dma, 0, sizeof(m_sound_dma));
}

READ8_MEMBER( wswan_state::bios_r )
{
	if (!m_bios_disabled)
		return m_ws_bios_bank[offset];
	else
		return m_cart->read_rom40(space, offset + 0xb0000);
}

READ8_MEMBER( wswan_state::port_r )
{
	UINT8 value = m_ws_portram[offset];

	if (offset != 2)
		logerror("PC=%X: port read %02X\n", m_maincpu->pc(), offset);

	if (offset < 0x40 || (offset >= 0xa1 && offset < 0xb0))
		return m_vdp->reg_r(space, offset);

	switch (offset)
	{
		case 0x4a:      // Sound DMA source address (low)
			value = m_sound_dma.source & 0xff;
			break;
		case 0x4b:      // Sound DMA source address (high)
			value = (m_sound_dma.source >> 8) & 0xff;
			break;
		case 0x4c:      // Sound DMA source memory segment
			value = (m_sound_dma.source >> 16) & 0xff;
			break;
		case 0x4e:      // Sound DMA transfer size (low)
			value = m_sound_dma.size & 0xff;
			break;
		case 0x4f:      // Sound DMA transfer size (high)
			value = (m_sound_dma.size >> 8) & 0xff;
			break;
		case 0x52:      // Sound DMA start/stop
			value = m_sound_dma.enable;
			break;
		case 0x60:
			value = m_vdp->reg_r(space, offset);
			break;
		case 0xa0:      // Hardware type
			// Bit 0 - Disable/enable Bios
			// Bit 1 - Determine mono/color
			// Bit 2 - Determine color/crystal
			value = value & ~ 0x02;
			if (m_system_type == TYPE_WSC)
				value |= 2;
			break;
		case 0xc0:
		case 0xc1:
		case 0xc2:
		case 0xc3:
		case 0xc4:  // EEPROM data
		case 0xc5:  // EEPROM data
		case 0xc6:
		case 0xc7:
		case 0xc8:
		case 0xc9:
		case 0xca:
		case 0xcb:  // RTC data
		case 0xcc:
		case 0xcd:
		case 0xce:
		case 0xcf:
			value = m_cart->read_io(space, offset & 0x0f);
			break;
	}

	return value;
}

WRITE8_MEMBER( wswan_state::port_w )
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	UINT8 input;
	logerror("PC=%X: port write %02X <- %02X\n", m_maincpu->pc(), offset, data);

	if (offset < 0x40 || (offset >= 0xa1 && offset < 0xb0))
	{
		m_vdp->reg_w(space, offset, data);
		return;
	}

	switch (offset)
	{
		case 0x40:  /* DMA source address (low)
                     Bit 0-7 - DMA source address bit 0-7
                     */
		case 0x41:  /* DMA source address (high)
                     Bit 0-7 - DMA source address bit 8-15
                     */
		case 0x42:  /* DMA source bank
                     Bit 0-7 - DMA source bank number
                     */
		case 0x43:  /* DMA destination bank
                     Bit 0-7 - DMA destination bank number
                     */
		case 0x44:  /* DMA destination address (low)
                     Bit 0-7 - DMA destination address bit 0-7
                     */
		case 0x45:  /* DMA destination address (high)
                     Bit 0-7 - DMA destination address bit 8-15
                     */
		case 0x46:  /* Size of copied data (low)
                     Bit 0-7 - DMA size bit 0-7
                     */
		case 0x47:  /* Size of copied data (high)
                     Bit 0-7 - DMA size bit 8-15
                     */
			break;
		case 0x48:  /* DMA control
                     Bit 0-6 - Unknown
                     Bit 7   - DMA stop/start
                     */
			if (data & 0x80)
			{
				UINT32 src, dst;
				UINT16 length;

				src = m_ws_portram[0x40] + (m_ws_portram[0x41] << 8) + (m_ws_portram[0x42] << 16);
				dst = m_ws_portram[0x44] + (m_ws_portram[0x45] << 8) + (m_ws_portram[0x43] << 16);
				length = m_ws_portram[0x46] + (m_ws_portram[0x47] << 8);
				for ( ; length > 0; length--)
				{
					mem.write_byte(dst, mem.read_byte(src));
					src++;
					dst++;
				}
#ifdef MAME_DEBUG
				logerror("DMA  src:%X dst:%X length:%d\n", src, dst, length);
#endif
				m_ws_portram[0x40] = src & 0xff;
				m_ws_portram[0x41] = (src >> 8) & 0xff;
				m_ws_portram[0x44] = dst & 0xff;
				m_ws_portram[0x45] = (dst >> 8) & 0xff;
				m_ws_portram[0x46] = length & 0xff;
				m_ws_portram[0x47] = (length >> 8) & 0xff;
				data &= 0x7f;
			}
			break;
		case 0x4a:  /* Sound DMA source address (low)
                     Bit 0-7 - Sound DMA source address bit 0-7
                     */
			m_sound_dma.source = (m_sound_dma.source & 0x0fff00) | data;
			break;
		case 0x4b:  /* Sound DMA source address (high)
                     Bit 0-7 - Sound DMA source address bit 8-15
                     */
			m_sound_dma.source = (m_sound_dma.source & 0x0f00ff) | (data << 8);
			break;
		case 0x4c:  /* Sound DMA source memory segment
                     Bit 0-3 - Sound DMA source address segment
                     Bit 4-7 - Unknown
                     */
			m_sound_dma.source = (m_sound_dma.source & 0xffff) | ((data & 0x0f) << 16);
			break;
		case 0x4d:  /* Unknown */
			break;
		case 0x4e:  /* Sound DMA transfer size (low)
                     Bit 0-7 - Sound DMA transfer size bit 0-7
                     */
			m_sound_dma.size = (m_sound_dma.size & 0xff00) | data;
			break;
		case 0x4f:  /* Sound DMA transfer size (high)
                     Bit 0-7 - Sound DMA transfer size bit 8-15
                     */
			m_sound_dma.size = (m_sound_dma.size & 0xff) | (data << 8);
			break;
		case 0x50:  /* Unknown */
		case 0x51:  /* Unknown */
			break;
		case 0x52:  /* Sound DMA start/stop
                     Bit 0-6 - Unknown
                     Bit 7   - Sound DMA stop/start
                     */
			m_sound_dma.enable = data;
			break;
		case 0x60:
			m_vdp->reg_w(space, offset, data);
			break;
		case 0x80:  /* Audio 1 freq (lo)
                     Bit 0-7 - Audio channel 1 frequency bit 0-7
                     */
		case 0x81:  /* Audio 1 freq (hi)
                     Bit 0-7 - Audio channel 1 frequency bit 8-15
                     */
		case 0x82:  /* Audio 2 freq (lo)
                     Bit 0-7 - Audio channel 2 frequency bit 0-7
                     */
		case 0x83:  /* Audio 2 freq (hi)
                     Bit 0-7 - Audio channel 2 frequency bit 8-15
                     */
		case 0x84:  /* Audio 3 freq (lo)
                     Bit 0-7 - Audio channel 3 frequency bit 0-7
                     */
		case 0x85:  /* Audio 3 freq (hi)
                     Bit 0-7 - Audio channel 3 frequency bit 8-15
                     */
		case 0x86:  /* Audio 4 freq (lo)
                     Bit 0-7 - Audio channel 4 frequency bit 0-7
                     */
		case 0x87:  /* Audio 4 freq (hi)
                     Bit 0-7 - Audio channel 4 frequency bit 8-15
                     */
		case 0x88:  /* Audio 1 volume
                     Bit 0-3 - Right volume audio channel 1
                     Bit 4-7 - Left volume audio channel 1
                     */
		case 0x89:  /* Audio 2 volume
                     Bit 0-3 - Right volume audio channel 2
                     Bit 4-7 - Left volume audio channel 2
                     */
		case 0x8a:  /* Audio 3 volume
                     Bit 0-3 - Right volume audio channel 3
                     Bit 4-7 - Left volume audio channel 3
                     */
		case 0x8b:  /* Audio 4 volume
                     Bit 0-3 - Right volume audio channel 4
                     Bit 4-7 - Left volume audio channel 4
                     */
		case 0x8c:  /* Sweep step
                     Bit 0-7 - Sweep step
                     */
		case 0x8d:  /* Sweep time
                     Bit 0-7 - Sweep time
                     */
		case 0x8e:  /* Noise control
                     Bit 0-2 - Noise generator type
                     Bit 3   - Reset
                     Bit 4   - Enable
                     Bit 5-7 - Unknown
                     */
		case 0x8f:  /* Sample location
                     Bit 0-7 - Sample address location 0 00xxxxxx xx000000
                     */
		case 0x90:  /* Audio control
                     Bit 0   - Audio 1 enable
                     Bit 1   - Audio 2 enable
                     Bit 2   - Audio 3 enable
                     Bit 3   - Audio 4 enable
                     Bit 4   - Unknown
                     Bit 5   - Audio 2 voice mode enable
                     Bit 6   - Audio 3 sweep mode enable
                     Bit 7   - Audio 4 noise mode enable
                     */
		case 0x91:  /* Audio output
                     Bit 0   - Mono select
                     Bit 1-2 - Output volume
                     Bit 3   - External stereo
                     Bit 4-6 - Unknown
                     Bit 7   - External speaker (Read-only, set by hardware)
                     */
		case 0x92:  /* Noise counter shift register (lo)
                     Bit 0-7 - Noise counter shift register bit 0-7
                     */
		case 0x93:  /* Noise counter shift register (hi)
                     Bit 0-6 - Noise counter shift register bit 8-14
                     bit 7   - Unknown
                     */
		case 0x94:  /* Master volume
                     Bit 0-3 - Master volume
                     Bit 4-7 - Unknown
                     */
			m_sound->port_w(space, offset, data);
			break;
		case 0xa0:  /* Hardware type - this is probably read only
                     Bit 0   - Enable cartridge slot and/or disable bios
                     Bit 1   - Hardware type: 0 = WS, 1 = WSC
                     Bit 2-7 - Unknown
                     */
			if ((data & 0x01) && !m_bios_disabled)
				m_bios_disabled = 1;
			break;

		case 0xb0:  /* Interrupt base vector
                     Bit 0-7 - Interrupt base vector
                     */
			break;
		case 0xb1:  /* Communication byte
                     Bit 0-7 - Communication byte
                     */
			break;
		case 0xb2:  /* Interrupt enable
                     Bit 0   - Serial transmit interrupt enable
                     Bit 1   - Key press interrupt enable
                     Bit 2   - RTC alarm interrupt enable
                     Bit 3   - Serial receive interrupt enable
                     Bit 4   - Drawing line detection interrupt enable
                     Bit 5   - VBlank timer interrupt enable
                     Bit 6   - VBlank interrupt enable
                     Bit 7   - HBlank timer interrupt enable
                     */
			break;
		case 0xb3:  /* serial communication control
                     Bit 0   - Receive complete
                     Bit 1   - Error
                     Bit 2   - Send complete
                     Bit 3-4 - Unknown
                     Bit 5   - Send data interrupt generation
                     Bit 6   - Connection speed: 0 = 9600 bps, 1 = 38400 bps
                     bit 7   - Receive data interrupt generation
                     */
			//          data |= 0x02;
			m_ws_portram[0xb1] = 0xff;
			if (data & 0x80)
			{
				//              m_ws_portram[0xb1] = 0x00;
				data |= 0x04;
			}
			if (data & 0x20)
			{
				//              data |= 0x01;
			}
			break;
		case 0xb5:  /* Read controls
                     Bit 0-3 - Current state of input lines (read-only)
                     Bit 4-6 - Select line of inputs to read
                     001 - Read Y cursors
                     010 - Read X cursors
                     100 - Read START,A,B buttons
                     Bit 7   - Unknown
                     */
			data = data & 0xf0;
			switch (data)
		{
			case 0x10:  /* Read Y cursors: Y1 - Y2 - Y3 - Y4 */
				input = m_cursy->read();
				if (m_rotate) // reorient controls if the console is rotated
				{
					if (input & 0x01) data |= 0x02;
					if (input & 0x02) data |= 0x04;
					if (input & 0x04) data |= 0x08;
					if (input & 0x08) data |= 0x01;
				}
				else
					data = data | input;
				break;
			case 0x20:  /* Read X cursors: X1 - X2 - X3 - X4 */
				input = m_cursx->read();
				if (m_rotate) // reorient controls if the console is rotated
				{
					if (input & 0x01) data |= 0x02;
					if (input & 0x02) data |= 0x04;
					if (input & 0x04) data |= 0x08;
					if (input & 0x08) data |= 0x01;
				}
				else
					data = data | input;
				break;
			case 0x40:  /* Read buttons: START - A - B */
				data = data | m_buttons->read();
				break;
		}
			break;
		case 0xb6:  /* Interrupt acknowledge
                     Bit 0   - Serial transmit interrupt acknowledge
                     Bit 1   - Key press interrupt acknowledge
                     Bit 2   - RTC alarm interrupt acknowledge
                     Bit 3   - Serial receive interrupt acknowledge
                     Bit 4   - Drawing line detection interrupt acknowledge
                     Bit 5   - VBlank timer interrupt acknowledge
                     Bit 6   - VBlank interrupt acknowledge
                     Bit 7   - HBlank timer interrupt acknowledge
                     */
			clear_irq_line(data);
			data = m_ws_portram[0xb6];
			break;
		case 0xba:  /* Internal EEPROM data (low)
                     Bit 0-7 - Internal EEPROM data transfer bit 0-7
                     */
		case 0xbb:  /* Internal EEPROM data (high)
                     Bit 0-7 - Internal EEPROM data transfer bit 8-15
                     */
			break;
		case 0xbc:  /* Internal EEPROM address (low)
                     Bit 0-7 - Internal EEPROM address bit 1-8
                     */
		case 0xbd:  /* Internal EEPROM address (high)
                     Bit 0   - Internal EEPROM address bit 9(?)
                     Bit 1-7 - Unknown
                     Only 1KByte internal EEPROM??
                     */
			break;
		case 0xbe:  /* Internal EEPROM command
                     Bit 0   - Read complete (read only)
                     Bit 1   - Write complete (read only)
                     Bit 2-3 - Unknown
                     Bit 4   - Read
                     Bit 5   - Write
                     Bit 6   - Protect
                     Bit 7   - Initialize
                     */
			if (data & 0x20)
			{
				UINT16 addr = ( ( ( m_ws_portram[0xbd] << 8 ) | m_ws_portram[0xbc] ) << 1 ) & 0x1FF;
				m_internal_eeprom[ addr ] = m_ws_portram[0xba];
				m_internal_eeprom[ addr + 1 ] = m_ws_portram[0xbb];
				data |= 0x02;
			}
			else if ( data & 0x10 )
			{
				UINT16 addr = ( ( ( m_ws_portram[0xbd] << 8 ) | m_ws_portram[0xbc] ) << 1 ) & 0x1FF;
				m_ws_portram[0xba] = m_internal_eeprom[ addr ];
				m_ws_portram[0xbb] = m_internal_eeprom[ addr + 1];
				data |= 0x01;
			}
			else
			{
				logerror( "Unsupported internal EEPROM command: %X\n", data );
			}
			break;
		case 0xc0:  // ROM bank $40000-$fffff
		case 0xc1:  // SRAM bank
		case 0xc2:  // ROM bank $20000-$2ffff
		case 0xc3:  // ROM bank $30000-$3ffff
		case 0xc4:
		case 0xc5:
		case 0xc6:  // EEPROM address / command
		case 0xc7:  // EEPROM address / command
		case 0xc8:  // EEPROM command
		case 0xc9:
		case 0xca:  // RTC command
		case 0xcb:  // RTC data
		case 0xcc:
		case 0xcd:
		case 0xce:
		case 0xcf:
			m_cart->write_io(space, offset & 0x0f, data);
			break;
		default:
			logerror( "Write to unsupported port: %X - %X\n", offset, data );
			break;
	}

	/* Update the port value */
	m_ws_portram[offset] = data;
}
