// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*

    M-Systems DiskOnChip G3 - Flash Disk with MLC NAND and M-Systems? x2 Technology

    (c) 2009 Tim Schuerewegen

*/

#include "emu.h"
#include "machine/docg3.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine.describe_context(), buf );
	}
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

// device type definition
const device_type DISKONCHIP_G3 = &device_creator<diskonchip_g3_device>;

//-------------------------------------------------
//  diskonchip_g3_device - constructor
//-------------------------------------------------

diskonchip_g3_device::diskonchip_g3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DISKONCHIP_G3, "DiskOnChip G3", tag, owner, clock, "diskonchip_g3", __FILE__),
		device_nvram_interface(mconfig, *this)
{
}


UINT32 diskonchip_g3_device::g3_offset_data_1()
{
//  printf( "block %d pages %d page %d planes %d plane %d user_data_size %d extra_area_size %d\n", doc->block, doc->pages, doc->page, doc->planes, doc->plane, doc->user_data_size, doc->extra_area_size);
	return ((((m_block * m_pages) + m_page) * m_planes) + m_plane) * (m_user_data_size + m_extra_area_size);
}

UINT32 diskonchip_g3_device::g3_offset_data_2()
{
	return ((((m_block * m_pages) + m_page) * m_planes) + m_plane) * 16;
}

UINT32 diskonchip_g3_device::g3_offset_data_3()
{
	return m_block * 8;
}

UINT8 diskonchip_g3_device::g3_read_data()
{
	UINT8 data = 0;
	UINT32 offset;
	if (m_test == 0)
	{
		// read page data (512 + 16)
		if (m_transfer_offset >= (m_user_data_size + m_extra_area_size))
		{
			m_transfer_offset = m_transfer_offset - (m_user_data_size + m_extra_area_size);
			m_plane++;
		}
		offset = g3_offset_data_1() + m_transfer_offset;
		data = m_data[0][offset];
	}
	else if (m_test == 1)
	{
		// read ??? (0x28 bytes)
		if (m_transfer_offset < 0x20)
		{
			offset = g3_offset_data_1() + (m_user_data_size + m_extra_area_size) + (m_user_data_size - 16) + m_transfer_offset;
			data = m_data[0][offset];
		}
		else if (m_transfer_offset < 0x28)
		{
			offset = g3_offset_data_3() + m_transfer_offset - 0x20;
			data = m_data[2][offset];
		}
		else
		{
			data = 0xFF;
		}
	}
	else if (m_test == 2)
	{
		// read erase block status
		data = 0x00;
	}
	m_transfer_offset++;
	return data;
}

READ16_MEMBER( diskonchip_g3_device::sec_1_r )
{
	UINT16 data;
	if (m_sec_2[0x1B] & 0x40)
	{
		data = g3_read_data();
		m_sec_2[0x1B] &= ~0x40;
	}
	else
	{
		data = (g3_read_data() << 0) | (g3_read_data() << 8);
	}
	verboselog( machine(), 9, "(DOC) %08X -> %04X\n", 0x0800 + (offset << 1), data);
	return data;
}

void diskonchip_g3_device::g3_write_data(UINT8 data)
{
	UINT32 offset;
	if (m_test == 3)
	{
		// read page data (512 + 16)
		if (m_transfer_offset >= (m_user_data_size + m_extra_area_size))
		{
			m_transfer_offset = m_transfer_offset - (m_user_data_size + m_extra_area_size);
			m_plane++;
		}
		if (m_transfer_offset == 0)
		{
			const UINT8 xxx[] = { 0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
			offset = g3_offset_data_2();
			memcpy( m_data[1] + offset, xxx, 16);
		}
		offset = g3_offset_data_1() + m_transfer_offset;
		m_data[0][offset] = data;
	}
	m_transfer_offset++;
}

WRITE16_MEMBER( diskonchip_g3_device::sec_1_w )
{
	verboselog( machine(), 9, "(DOC) %08X <- %04X\n", 0x0800 + (offset << 1), data);
	if (m_sec_2[0x1B] & 0x40)
	{
		g3_write_data(data);
		m_sec_2[0x1B] &= ~0x40;
	}
	else
	{
		g3_write_data((data >> 0) & 0xFF);
		g3_write_data((data >> 8) & 0xFF);
	}
}

// #define DoC_G3_IO 0x0800
// #define DoC_G3_ChipID 0x1000
// #define DoC_G3_DeviceIdSelect 0x100a
// #define DoC_G3_Ctrl 0x100c
// #define DoC_G3_CtrlConfirm 0x1072
// #define DoC_G3_ReadAddress 0x101a
// #define DoC_G3_FlashSelect 0x1032
// #define DoC_G3_FlashCmd 0x1034
// #define DoC_G3_FlashAddr 0x1036
// #define DoC_G3_FlashCtrl 0x1038
// #define DoC_G3_Nop 0x103e

UINT16 diskonchip_g3_device::sec_2_read_1000()
{
	if (m_device == 0)
	{
		return 0x0200;
	}
	else
	{
		return 0x008C;
	}
}

UINT16 diskonchip_g3_device::sec_2_read_1074()
{
	if (m_device == 0)
	{
		return 0xFDFF;
	}
	else
	{
		return 0x02B3;
	}
}

UINT8 diskonchip_g3_device::sec_2_read_1042()
{
	UINT8 data;
	if ((m_block == 0) && (m_page == 0) && (m_transfersize == 0x27))
	{
		data = 0x07;
	}
	else
	{
		UINT32 offset = g3_offset_data_2() + 0x02;
		data = m_data[1][offset]; //0x07; //data & ~(1 << 7);
		if ((m_sec_2[0x41] & 0x10) == 0)
		{
			data = data & ~0x20;
		}
		if ((m_sec_2[0x41] & 0x08) == 0)
		{
			data = data & ~0x80;
		}
		if (m_test != 0)
		{
			data = 0x00;
		}
	}
	return data;
}

UINT8 diskonchip_g3_device::sec_2_read_1046()
{
	if (m_test == 0)
	{
		UINT32 offset = g3_offset_data_2() + 0x06;
		return m_data[1][offset];
	}
	else
	{
		return 0x00;
	}
}

UINT8 diskonchip_g3_device::sec_2_read_1048()
{
	if (m_test == 0)
	{
		UINT32 offset = g3_offset_data_2() + 0x08;
		return m_data[1][offset];
	}
	else
	{
		return 0x00;
	}
}

UINT8 diskonchip_g3_device::sec_2_read_1049()
{
	if (m_test == 0)
	{
		UINT32 offset = g3_offset_data_2() + 0x09;
		return m_data[1][offset];
	}
	else
	{
		return 0x00;
	}
}

UINT8 diskonchip_g3_device::sec_2_read_104A()
{
	if (m_test == 0)
	{
		UINT32 offset = g3_offset_data_2() + 0x0A;
		return m_data[1][offset];
	}
	else
	{
		return 0x00;
	}
}

UINT8 diskonchip_g3_device::sec_2_read_104B()
{
	if (m_test == 0)
	{
		UINT32 offset = g3_offset_data_2() + 0x0B;
		return m_data[1][offset];
	}
	else
	{
		return 0x00;
	}
}

UINT8 diskonchip_g3_device::sec_2_read_104C()
{
	if (m_test == 0)
	{
		UINT32 offset = g3_offset_data_2() + 0x0C;
		return m_data[1][offset];
	}
	else
	{
		return 0x00;
	}
}

UINT8 diskonchip_g3_device::sec_2_read_104D()
{
	if (m_test == 0)
	{
		UINT32 offset = g3_offset_data_2() + 0x0D;
		return m_data[1][offset];
	}
	else
	{
		return 0x00;
	}
}

UINT8 diskonchip_g3_device::sec_2_read_104E()
{
	if (m_test == 0)
	{
		UINT32 offset = g3_offset_data_2() + 0x0E;
		return m_data[1][offset];
	}
	else
	{
		return 0x00;
	}
}

UINT8 diskonchip_g3_device::sec_2_read_104F()
{
	if (m_test == 0)
	{
		UINT32 offset = g3_offset_data_2() + 0x0F;
		return m_data[1][offset];
	}
	else
	{
		return 0x00;
	}
}

UINT8 diskonchip_g3_device::sec_2_read_100E()
{
	return 0x81;
}

UINT8 diskonchip_g3_device::sec_2_read_1014()
{
	return 0x01;
}

UINT8 diskonchip_g3_device::sec_2_read_1022()
{
	return 0x85;
}

UINT8 diskonchip_g3_device::sec_2_read_1038()
{
	// bit 0 = ?
	// bit 1 = ? error 0x7C
	// bit 2 = ? error 0x73
	// bit 3 = ?
	// bit 4 = ?
	// bit 5 = ?
	// bit 6 = ?
	// bit 7 = ?
	if (m_device == 0)
	{
		return 0x19;
	}
	else
	{
		return 0x38;
	}
}

UINT16 diskonchip_g3_device::sec_2_read16(UINT32 offset)
{
	UINT16 data = (m_sec_2[offset+0] << 0) + (m_sec_2[offset+1] << 8);
	switch (0x1000 + offset)
	{
		// ?
		case 0x1000 : data = sec_2_read_1000(); break;
		// ?
		case 0x100E : data = sec_2_read_100E(); break;
		// ?
		case 0x1014 : data = sec_2_read_1014(); break;
		// ?
		case 0x1022 : data = sec_2_read_1022(); break;
		// flash control
		case 0x1038 : data = sec_2_read_1038(); break;
		// flash ?
		case 0x1042 : data = sec_2_read_1042(); break;
		// ?
		case 0x1046 : data = sec_2_read_1046(); break;
		// ?
		case 0x1048 : data = (sec_2_read_1048() << 0) + (sec_2_read_1049() << 8); break;
		case 0x104A : data = (sec_2_read_104A() << 0) + (sec_2_read_104B() << 8); break;
		case 0x104C : data = (sec_2_read_104C() << 0) + (sec_2_read_104D() << 8); break;
		case 0x104E : data = (sec_2_read_104E() << 0) + (sec_2_read_104F() << 8); break;
		// ?
		case 0x1074 : data = sec_2_read_1074(); break;
	}
	verboselog( machine(), 9, "(DOC) %08X -> %04X\n", 0x1000 + offset, data);
	return data;
}

UINT8 diskonchip_g3_device::sec_2_read8(UINT32 offset)
{
	UINT8 data = m_sec_2[offset];
	switch (0x1000 + offset)
	{
		// ?
		case 0x100E : data = sec_2_read_100E(); break;
		// ?
		case 0x1014 : data = sec_2_read_1014(); break;
		// ?
		case 0x1022 : data = sec_2_read_1022(); break;
		// flash control
		case 0x1038 : data = sec_2_read_1038(); break;
		// flash ?
		case 0x1042 : data = sec_2_read_1042(); break;
		// ?
		case 0x1046 : data = sec_2_read_1046(); break;
		// ?
		case 0x1048 : data = sec_2_read_1048(); break;
		case 0x1049 : data = sec_2_read_1049(); break;
		case 0x104A : data = sec_2_read_104A(); break;
		case 0x104B : data = sec_2_read_104B(); break;
		case 0x104C : data = sec_2_read_104C(); break;
		case 0x104D : data = sec_2_read_104D(); break;
		case 0x104E : data = sec_2_read_104E(); break;
		case 0x104F : data = sec_2_read_104F(); break;
	}
	verboselog( machine(), 9, "(DOC) %08X -> %02X\n", 0x1000 + offset, data);
	return data;
}

void diskonchip_g3_device::sec_2_write_100C(UINT8 data)
{
	const char *mode_name[] = { "reset", "normal", "deep power down" };
	UINT32 mode = data & 3;
	verboselog( machine(), 5, "mode %d (%s)\n", mode, mode_name[mode]);
	if (mode == 0)
	{
		m_sec_2[0x04] = 00;
	}
}

void diskonchip_g3_device::sec_2_write_1032(UINT8 data)
{
	verboselog( machine(), 5, "flash select %02X\n", data);
	if ((data == 0x12) || (data == 0x27))
	{
		m_transfer_offset = 0;
		m_plane = 0;
		m_block = 0;
		m_page = 0;
	}
}

void diskonchip_g3_device::g3_erase_block()
{
	UINT32 offset;
	int i, j;
	const UINT8 xxx[] = { 0x00, 0x00, 0x87, 0x00, 0x00, 0x00, 0xCE, 0x00, 0xCF, 0x72, 0xFC, 0x1B, 0xA9, 0xC7, 0xB9, 0x00 };
	verboselog( machine(), 5, "erase block %04X\n", m_block);
	for (i=0;i<m_pages;i++)
	{
		m_page = i;
		for (j=0;j<2;j++)
		{
			m_plane = j;
			offset = g3_offset_data_1();
			memset( m_data[0] + offset, 0xFF, (m_user_data_size + m_extra_area_size));
			offset = g3_offset_data_2();
			memcpy( m_data[1] + offset, xxx, 16);
		}
	}
}

// 'maincpu' (028848E4): diskonchip_sec_2_write_1034: unknown value 60/27
// 'maincpu' (028848E4): diskonchip_sec_2_write_1034: unknown value D0/27
// 'maincpu' (028848E4): diskonchip_sec_2_write_1034: unknown value 71/31
// 'maincpu' (028848E4): diskonchip_sec_2_write_1034: unknown value 80/1D
// 'maincpu' (028848E4): diskonchip_sec_2_write_1034: unknown value 11/1D
// 'maincpu' (028848E4): diskonchip_sec_2_write_1034: unknown value 10/1D

void diskonchip_g3_device::sec_2_write_1034(UINT8 data)
{
	verboselog( machine(), 5, "flash command %02X\n", data);
	if ((m_sec_2[0x32] == 0x0E) && (data == 0x00))
	{
		m_test = 0;
		m_address_count = 0;
	}
	else if ((m_sec_2[0x32] == 0x10) && (data == 0x50))
	{
		m_test = 1;
		m_address_count = 0;
	}
	else if ((m_sec_2[0x32] == 0x03) && (data == 0x3C))
	{
		// do nothing
	}
	else if ((m_sec_2[0x32] == 0x00) && (data == 0xFF))
	{
		m_address_count = 0;
	}
	else if (m_sec_2[0x32] == 0x09)
	{
		if (data == 0x22)
		{
			// do nothing
		}
		else if (data == 0xA2)
		{
			// do nothing
		}
		else
		{
			verboselog( machine(), 0, "diskonchip_sec_2_write_1034: unknown value %02X/%02X\n", data, m_sec_2[0x32]);
		}
	}
	else if (m_sec_2[0x32] == 0x12)
	{
		if (data == 0x60)
		{
			m_data_1036 = 0;
			m_data_1036_count = 0;
			m_address_count++;
		}
		else if (data == 0x30)
		{
			// do nothing
		}
		else if (data == 0x05)
		{
			// do nothing
		}
		else if (data == 0xE0)
		{
			// do nothing
		}
		else
		{
			verboselog( machine(), 0, "diskonchip_sec_2_write_1034: unknown value %02X/%02X\n", data, m_sec_2[0x32]);
		}
	}
	else if (m_sec_2[0x32] == 0x27)
	{
		if (data == 0x60)
		{
			m_data_1036 = 0;
			m_data_1036_count = 0;
			m_address_count++;
		}
		else if (data == 0xD0)
		{
			g3_erase_block();
		}
		else
		{
			verboselog( machine(), 0, "diskonchip_sec_2_write_1034: unknown value %02X/%02X\n", data, m_sec_2[0x32]);
		}
	}
	else if ((m_sec_2[0x32] == 0x31) && (data == 0x71))
	{
		// erase block status? (after: read one byte from 08xx, bit 0/1/2 is checked)
		m_test = 2;
	}
	else if (m_sec_2[0x32] == 0x1D)
	{
		if (data == 0x80)
		{
			m_data_1036 = 0;
			m_data_1036_count = 0;
			m_address_count++;
		}
		else if (data == 0x11)
		{
			m_test = 3;
		}
		else
		{
			verboselog( machine(), 0, "diskonchip_sec_2_write_1034: unknown value %02X/%02X\n", data, m_sec_2[0x32]);
		}
	}
	else
	{
		verboselog( machine(), 0, "diskonchip_sec_2_write_1034: unknown value %02X/%02X\n", data, m_sec_2[0x32]);
	}
}

void diskonchip_g3_device::sec_2_write_1036(UINT8 data)
{
	if (m_sec_2[0x34] == 0x60)
	{
		m_data_1036 |= data << (8 * m_data_1036_count++);
		if (m_data_1036_count == 3)
		{
			UINT32 block, page, plane;
			block = (m_data_1036 >> 7);
			if (block >= m_blocks) fatalerror( "DOCG3: invalid block (%d)\n", block);
			plane = (m_data_1036 >> 6) & 1;
			page = (m_data_1036 >> 0) & 0x3F;
			verboselog( machine(), 5, "flash address %d - %06X (plane %d block %04X page %04X)\n", m_address_count, m_data_1036, plane, block, page);
			if (m_address_count == 1)
			{
				m_plane = 0;
				m_block = block;
				m_page = page;
			}
		}
	}
	else if (m_sec_2[0x34] == 0x80)
	{
		m_data_1036 |= data << (8 * m_data_1036_count++);
		if (m_data_1036_count == 4)
		{
			UINT32 block, page, plane, unk;
			block = (m_data_1036 >> 15);
			plane = (m_data_1036 >> 14) & 1;
			page = (m_data_1036 >> 8) & 0x3F;
			unk = (m_data_1036 >> 0) & 0xFF;
			verboselog( machine(), 5, "flash address %d - %08X (plane %d block %04X page %04X unk %02X)\n", m_address_count, m_data_1036, plane, block, page, unk);
			if (m_address_count == 1)
			{
				m_plane = 0;
				m_block = block;
				m_page = page;
				m_transfer_offset = 0;
			}
		}
	}
	else if (m_sec_2[0x34] == 0x05)
	{
		m_transfer_offset = data << 2;
		verboselog( machine(), 5, "flash transfer offset %04X\n", m_transfer_offset);
	}
}

void diskonchip_g3_device::sec_2_write_1040(UINT16 data)
{
	m_transfersize = (data & 0x3FF);
	verboselog( machine(), 5, "flash transfer size %04X\n", m_transfersize);
}

void diskonchip_g3_device::sec_2_write_100A(UINT8 data)
{
	m_device = data & 3;
	verboselog( machine(), 5, "select device %d\n", m_device);
}

void diskonchip_g3_device::sec_2_write16(UINT32 offset, UINT16 data)
{
	m_sec_2[offset+0] = (data >> 0) & 0xFF;
	m_sec_2[offset+1] = (data >> 8) & 0xFF;
	verboselog( machine(), 9, "(DOC) %08X <- %04X\n", 0x1000 + offset, data);
	switch (0x1000 + offset)
	{
		// ?
		case 0x100C : sec_2_write_100C(data); break;
		// Device ID Select Register
		case 0x100A : sec_2_write_100A(data); break;
		// flash select
		case 0x1032 : sec_2_write_1032(data); break;
		// flash command
		case 0x1034 : sec_2_write_1034(data); break;
		// flash address
		case 0x1036 : sec_2_write_1036(data); break;
		// ?
		case 0x1040 : sec_2_write_1040(data); break;
	}
}

void diskonchip_g3_device::sec_2_write8(UINT32 offset, UINT8 data)
{
	m_sec_2[offset] = data;
	verboselog( machine(), 9, "(DOC) %08X <- %02X\n", 0x1000 + offset, data);
	switch (0x1000 + offset)
	{
		// ?
		case 0x100C : sec_2_write_100C(data); break;
		// Device ID Select Register
		case 0x100A : sec_2_write_100A(data); break;
		// flash select
		case 0x1032 : sec_2_write_1032(data); break;
		// flash command
		case 0x1034 : sec_2_write_1034(data); break;
		// flash address
		case 0x1036 : sec_2_write_1036(data); break;
	}
}

READ16_MEMBER( diskonchip_g3_device::sec_2_r )
{
	if (mem_mask == 0xffff)
	{
		return sec_2_read16(offset * 2);
	}
	else if (mem_mask == 0x00ff)
	{
		return sec_2_read8(offset * 2 + 0) << 0;
	}
	else if (mem_mask == 0xff00)
	{
		return sec_2_read8( offset * 2 + 1) << 8;
	}
	else
	{
		verboselog( machine(), 0, "diskonchip_g3_sec_2_r: unknown mem_mask %08X\n", mem_mask);
		return 0;
	}
}

WRITE16_MEMBER( diskonchip_g3_device::sec_2_w )
{
	if (mem_mask == 0xffff)
	{
		sec_2_write16(offset * 2, data);
	}
	else if (mem_mask == 0x00ff)
	{
		sec_2_write8(offset * 2 + 0, (data >> 0) & 0xFF);
	}
	else if (mem_mask == 0xff00)
	{
		sec_2_write8(offset * 2 + 1, (data >> 8) & 0xFF);
	}
	else
	{
		verboselog( machine(), 0, "diskonchip_g3_sec_2_w: unknown mem_mask %08X\n", mem_mask);
	}
}

READ16_MEMBER( diskonchip_g3_device::sec_3_r )
{
	UINT16 data = 0;
	verboselog( machine(), 9, "(DOC) %08X -> %04X\n", 0x1800 + (offset << 1), data);
	return data;
}

WRITE16_MEMBER( diskonchip_g3_device::sec_3_w )
{
	verboselog( machine(), 9, "(DOC) %08X <- %02X\n", 0x1800 + (offset << 1), data);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void diskonchip_g3_device::device_start()
{
	verboselog( machine(), 9, "(DOC) device start\n");

	switch (m_size)
	{
		case 64 :
		{
			m_planes = 2;
			m_blocks = 1024;
			m_pages = 64;
			m_user_data_size = 512;
			m_extra_area_size = 16;
		}
		break;
	}

	m_data_size[0] = m_planes * m_blocks * m_pages * (m_user_data_size + m_extra_area_size);
	m_data_size[1] = m_planes * m_blocks * m_pages * 16;
	m_data_size[2] = m_blocks * 8;

	m_device = 0;
	m_block = 0;
	m_page = 0;
	m_transfersize = 0;
	m_plane = 0;
	m_test = 0;

	memset(m_sec_2, 0, sizeof(m_sec_2));

	m_data[0] = auto_alloc_array( machine(), UINT8, m_data_size[0]);
	memset(m_data[0], 0, sizeof(UINT8) * m_data_size[0]);
	m_data[1] = auto_alloc_array( machine(), UINT8, m_data_size[1]);
	memset(m_data[1], 0, sizeof(UINT8) * m_data_size[1]);
	m_data[2] = auto_alloc_array( machine(), UINT8, m_data_size[2]);
	memset(m_data[2], 0, sizeof(UINT8) * m_data_size[2]);

//  diskonchip_load( device, "diskonchip");

	save_item( NAME(m_planes));
	save_item( NAME(m_blocks));
	save_item( NAME(m_pages));
	save_item( NAME(m_user_data_size));
	save_item( NAME(m_extra_area_size));
	save_pointer( NAME(m_data[0]), m_data_size[0]);
	save_pointer( NAME(m_data[1]), m_data_size[1]);
	save_pointer( NAME(m_data[2]), m_data_size[2]);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void diskonchip_g3_device::device_reset()
{
	verboselog( machine(), 9, "(DOC) device reset\n");
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void diskonchip_g3_device::nvram_default()
{
	memset(m_data[0], 0xFF, m_data_size[0]);
	memset(m_data[1], 0x00, m_data_size[1]);
	memset(m_data[2], 0xFF, m_data_size[2]);
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void diskonchip_g3_device::nvram_read(emu_file &file)
{
	file.read(m_data[0], m_data_size[0]);
	file.read(m_data[1], m_data_size[1]);
	file.read(m_data[2], m_data_size[2]);
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void diskonchip_g3_device::nvram_write(emu_file &file)
{
	file.write(m_data[0], m_data_size[0]);
	file.write(m_data[1], m_data_size[1]);
	file.write(m_data[2], m_data_size[2]);
}
