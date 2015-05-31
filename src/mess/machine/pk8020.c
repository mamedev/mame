// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        PK-8020 driver by Miodrag Milanovic
            based on work of Sergey Erokhin from pk8020.narod.ru

        18/07/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/pk8020.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/flopdrv.h"


READ8_MEMBER(pk8020_state::keyboard_r)
{
	UINT8 retVal=0x00;
	UINT8 line = 0;
	if (offset & 0x100)  line=8;

	if (offset & 0x0001) retVal|=m_io_port[line]->read();
	line++;
	if (offset & 0x0002) retVal|=m_io_port[line]->read();
	line++;
	if (offset & 0x0004) retVal|=m_io_port[line]->read();
	line++;
	if (offset & 0x0008) retVal|=m_io_port[line]->read();
	line++;
	if (offset & 0x0010) retVal|=m_io_port[line]->read();
	line++;
	if (offset & 0x0020) retVal|=m_io_port[line]->read();
	line++;
	if (offset & 0x0040) retVal|=m_io_port[line]->read();
	line++;
	if (offset & 0x0080) retVal|=m_io_port[line]->read();
	line++;

	return retVal;
}

READ8_MEMBER(pk8020_state::sysreg_r)
{
	return m_ram->pointer()[offset];
}
WRITE8_MEMBER(pk8020_state::sysreg_w)
{
	if (BIT(offset,7)==0) {
		pk8020_set_bank(data >> 2);
	} else if (BIT(offset,6)==0) {
		// Color
		m_color = data;
	} else if (BIT(offset,2)==0) {
		// Palette set
		UINT8 number = data & 0x0f;
		UINT8 color = data >> 4;
		UINT8 i = (color & 0x08) ?  0x3F : 0;
		UINT8 r = ((color & 0x04) ? 0xC0 : 0) + i;
		UINT8 g = ((color & 0x02) ? 0xC0 : 0) + i;
		UINT8 b = ((color & 0x01) ? 0xC0 : 0) + i;
		m_palette->set_pen_color( number, rgb_t(r,g,b) );
	}
}

READ8_MEMBER(pk8020_state::text_r)
{
	if (m_attr == 3) m_text_attr=m_ram->pointer()[0x40400+offset];
	return m_ram->pointer()[0x40000+offset];
}

WRITE8_MEMBER(pk8020_state::text_w)
{
	UINT8 *ram = m_ram->pointer();
	ram[0x40000+offset] = data;
	switch (m_attr) {
		case 0: break;
		case 1: ram[0x40400+offset]=0x01;break;
		case 2: ram[0x40400+offset]=0x00;break;
		case 3: ram[0x40400+offset]=m_text_attr;break;
	}
}

READ8_MEMBER(pk8020_state::gzu_r)
{
	UINT8 *addr = m_ram->pointer() + 0x10000 + (m_video_page_access * 0xC000);
	UINT8 p0 = addr[offset];
	UINT8 p1 = addr[offset + 0x4000];
	UINT8 p2 = addr[offset + 0x8000];
	UINT8 retVal = 0;
	if(m_color & 0x80) {
		// Color mode
		if (!(m_color & 0x10)) {
			p0 ^= 0xff;
		}
		if (!(m_color & 0x20)) {
			p1 ^= 0xff;
		}
		if (!(m_color & 0x40)) {
			p2 ^= 0xff;
		}
		retVal = (p0 & p1 & p2) ^ 0xff;
	} else {
		// Plane mode
		if (m_color & 0x10) {
			retVal |= p0;
		}
		if (m_color & 0x20) {
			retVal |= p1;
		}
		if (m_color & 0x40) {
			retVal |= p2;
		}
	}
	return retVal;
}

WRITE8_MEMBER(pk8020_state::gzu_w)
{
	UINT8 *addr = m_ram->pointer() + 0x10000 + (m_video_page_access * 0xC000);
	UINT8 *plane_0 = addr;
	UINT8 *plane_1 = addr + 0x4000;
	UINT8 *plane_2 = addr + 0x8000;

	if(m_color & 0x80)
	{
		// Color mode
		plane_0[offset] = (plane_0[offset] & ~data) | ((m_color & 2) ? data : 0);
		plane_1[offset] = (plane_1[offset] & ~data) | ((m_color & 4) ? data : 0);
		plane_2[offset] = (plane_2[offset] & ~data) | ((m_color & 8) ? data : 0);
	} else {
		// Plane mode
		UINT8 mask = (m_color & 1) ? data : 0;
		if (!(m_color & 0x02)) {
			plane_0[offset] = (plane_0[offset] & ~data) | mask;
		}
		if (!(m_color & 0x04)) {
			plane_1[offset] = (plane_1[offset] & ~data) | mask;
		}
		if (!(m_color & 0x08)) {
			plane_2[offset] = (plane_2[offset] & ~data) | mask;
		}
	}
}

READ8_MEMBER(pk8020_state::devices_r)
{
	switch(offset & 0x38)
	{
		case 0x00: return m_pit8253->read(space, offset & 3);
		case 0x08: return m_ppi8255_3->read(space,offset & 3);
		case 0x10: switch(offset & 1) {
						case 0 : return m_rs232->data_r(space,0);
						case 1 : return m_rs232->status_r(space,0);
					}
					break;
		case 0x18: return m_wd1793->read(space, offset & 0x03);
		case 0x20: switch(offset & 1) {
						case 0 : return m_lan->data_r(space,0);
						case 1 : return m_lan->status_r(space,0);
					}
					break;
		case 0x28: return m_pic8259->read(space, offset & 1);
		case 0x30: return m_ppi8255_2->read(space,offset & 3);
		case 0x38: return m_ppi8255_1->read(space,offset & 3);
	}
	return 0xff;
}

WRITE8_MEMBER(pk8020_state::devices_w)
{
	switch(offset & 0x38)
	{
		case 0x00: m_pit8253->write(space, offset & 3, data); break;
		case 0x08: m_ppi8255_3->write(space,offset & 3,data); break;
		case 0x10: switch(offset & 1) {
						case 0 : m_rs232->data_w(space,0,data); break;
						case 1 : m_rs232->control_w(space,0,data); break;
					}
					break;
		case 0x18:
			m_wd1793->write(space, offset & 0x03, data);
			break;
		case 0x20: switch(offset & 1) {
						case 0 : m_lan->data_w(space,0,data); break;
						case 1 : m_lan->control_w(space,0,data); break;
					}
					break;
		case 0x28: m_pic8259->write(space, offset & 1,data);break;
		case 0x30: m_ppi8255_2->write(space,offset & 3,data); break;
		case 0x38: m_ppi8255_1->write(space,offset & 3,data); break;
	}
}

void pk8020_state::pk8020_set_bank(UINT8 data)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 *mem = m_region_maincpu->base();
	UINT8 *ram = m_ram->pointer();

	switch(data & 0x1F) {
		case 0x00 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x37ff, "bank1");
						space.install_write_bank(0x0000, 0x37ff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// Keyboard
						space.install_read_handler (0x3800, 0x39ff, read8_delegate(FUNC(pk8020_state::keyboard_r),this));
						space.install_write_bank(0x3800, 0x39ff, "bank3");
						membank("bank3")->set_base(ram + 0x3800);
						// System reg
						space.install_read_handler (0x3a00, 0x3aff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0x3a00, 0x3aff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
						// Devices
						space.install_read_handler (0x3b00, 0x3bff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0x3b00, 0x3bff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// Text Video Memory
						space.install_read_handler (0x3c00, 0x3fff, read8_delegate(FUNC(pk8020_state::text_r),this));
						space.install_write_handler(0x3c00, 0x3fff, write8_delegate(FUNC(pk8020_state::text_w),this));
						// RAM
						space.install_read_bank (0x4000, 0xffff, "bank4");
						space.install_write_bank(0x4000, 0xffff, "bank5");
						membank("bank4")->set_base(ram + 0x4000);
						membank("bank5")->set_base(ram + 0x4000);
					}
					break;
		case 0x01 : {
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xffff, "bank3");
						space.install_write_bank(0x2000, 0xffff, "bank4");
						membank("bank3")->set_base(ram + 0x2000);
						membank("bank4")->set_base(ram + 0x2000);
					}
					break;
		case 0x02 : {
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xffff, "bank3");
						space.install_write_bank(0x4000, 0xffff, "bank4");
						membank("bank3")->set_base(ram + 0x4000);
						membank("bank4")->set_base(ram + 0x4000);
					}
					break;
		case 0x03 : {
						// RAM
						space.install_read_bank (0x0000, 0xffff, "bank1");
						space.install_write_bank(0x0000, 0xffff, "bank2");
						membank("bank1")->set_base(ram);
						membank("bank2")->set_base(ram);
					}
					break;
		case 0x04 :
		case 0x05 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xf7ff, "bank3");
						space.install_write_bank(0x2000, 0xf7ff, "bank4");
						membank("bank3")->set_base(ram + 0x2000);
						membank("bank4")->set_base(ram + 0x2000);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),this));
						space.install_write_bank(0xf800, 0xf9ff, "bank5");
						membank("bank5")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),this));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),this));
					}
					break;
		case 0x06 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xf7ff, "bank3");
						space.install_write_bank(0x4000, 0xf7ff, "bank4");
						membank("bank3")->set_base(ram + 0x4000);
						membank("bank4")->set_base(ram + 0x4000);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),this));
						space.install_write_bank(0xf800, 0xf9ff, "bank5");
						membank("bank5")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),this));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),this));
					}
					break;
		case 0x07 :
					{
						// RAM
						space.install_read_bank (0x0000, 0xf7ff, "bank1");
						space.install_write_bank(0x0000, 0xf7ff, "bank2");
						membank("bank1")->set_base(ram);
						membank("bank2")->set_base(ram);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),this));
						space.install_write_bank(0xf800, 0xf9ff, "bank3");
						membank("bank3")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),this));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),this));
					}
					break;
		case 0x08 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// Keyboard
						space.install_read_handler (0x3800, 0x39ff, read8_delegate(FUNC(pk8020_state::keyboard_r),this));
						space.install_write_bank(0x3800, 0x39ff, "bank3");
						membank("bank3")->set_base(ram + 0x3800);
						// System reg
						space.install_read_handler (0x3a00, 0x3aff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0x3a00, 0x3aff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
						// Devices
						space.install_read_handler (0x3b00, 0x3bff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0x3b00, 0x3bff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// Text Video Memory
						space.install_read_handler (0x3c00, 0x3fff, read8_delegate(FUNC(pk8020_state::text_r),this));
						space.install_write_handler(0x3c00, 0x3fff, write8_delegate(FUNC(pk8020_state::text_w),this));
						// RAM
						space.install_read_bank (0x4000, 0xbfff, "bank4");
						space.install_write_bank(0x4000, 0xbfff, "bank5");
						membank("bank4")->set_base(ram + 0x4000);
						membank("bank5")->set_base(ram + 0x4000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),this));

					}
					break;
		case 0x09 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xbfff, "bank3");
						space.install_write_bank(0x2000, 0xbfff, "bank4");
						membank("bank3")->set_base(ram + 0x2000);
						membank("bank4")->set_base(ram + 0x2000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
					}
					break;
		case 0x0A :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xbfff, "bank3");
						space.install_write_bank(0x4000, 0xbfff, "bank4");
						membank("bank3")->set_base(ram + 0x4000);
						membank("bank4")->set_base(ram + 0x4000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
					}
					break;
		case 0x0B :
					{
						// RAM
						space.install_read_bank (0x0000, 0xbfff, "bank1");
						space.install_write_bank(0x0000, 0xbfff, "bank2");
						membank("bank1")->set_base(ram + 0x0000);
						membank("bank2")->set_base(ram + 0x0000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
					}
					break;
		case 0x0C :
		case 0x0D :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0x3fff, "bank3");
						space.install_write_bank(0x2000, 0x3fff, "bank4");
						membank("bank3")->set_base(ram + 0x2000);
						membank("bank4")->set_base(ram + 0x2000);
						// Video RAM
						space.install_read_handler (0x4000, 0x7fff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0x4000, 0x7fff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
						// RAM
						space.install_read_bank (0x8000, 0xfdff, "bank5");
						space.install_write_bank(0x8000, 0xfdff, "bank6");
						membank("bank5")->set_base(ram + 0x8000);
						membank("bank6")->set_base(ram + 0x8000);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
					}
					break;
		case 0x0E :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// Video RAM
						space.install_read_handler (0x4000, 0x7fff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0x4000, 0x7fff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
						// RAM
						space.install_read_bank (0x8000, 0xfdff, "bank5");
						space.install_write_bank(0x8000, 0xfdff, "bank6");
						membank("bank5")->set_base(ram + 0x8000);
						membank("bank6")->set_base(ram + 0x8000);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
					}
					break;
		case 0x0F :
					{
						// RAM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						membank("bank1")->set_base(ram + 0x0000);
						membank("bank2")->set_base(ram + 0x0000);
						// Video RAM
						space.install_read_handler (0x4000, 0x7fff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0x4000, 0x7fff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
						// RAM
						space.install_read_bank (0x8000, 0xfdff, "bank3");
						space.install_write_bank(0x8000, 0xfdff, "bank4");
						membank("bank3")->set_base(ram + 0x8000);
						membank("bank4")->set_base(ram + 0x8000);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
					}
					break;
		case 0x10 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x5fff, "bank1");
						space.install_write_bank(0x0000, 0x5fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x6000, 0xf7ff, "bank3");
						space.install_write_bank(0x6000, 0xf7ff, "bank4");
						membank("bank3")->set_base(ram + 0x6000);
						membank("bank4")->set_base(ram + 0x6000);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),this));
						space.install_write_bank(0xf800, 0xf9ff, "bank5");
						membank("bank5")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),this));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),this));
					}
					break;
		case 0x11 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xf7ff, "bank3");
						space.install_write_bank(0x2000, 0xf7ff, "bank4");
						membank("bank3")->set_base(ram + 0x2000);
						membank("bank4")->set_base(ram + 0x2000);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),this));
						space.install_write_bank(0xf800, 0xf9ff, "bank5");
						membank("bank5")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),this));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),this));
					}
					break;
		case 0x12 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xf7ff, "bank3");
						space.install_write_bank(0x4000, 0xf7ff, "bank4");
						membank("bank3")->set_base(ram + 0x4000);
						membank("bank4")->set_base(ram + 0x4000);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),this));
						space.install_write_bank(0xf800, 0xf9ff, "bank5");
						membank("bank5")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),this));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),this));
					}
					break;
		case 0x13 :
					{
						// RAM
						space.install_read_bank (0x0000, 0xf7ff, "bank1");
						space.install_write_bank(0x0000, 0xf7ff, "bank2");
						membank("bank1")->set_base(ram + 0x0000);
						membank("bank2")->set_base(ram + 0x0000);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),this));
						space.install_write_bank(0xf800, 0xf9ff, "bank3");
						membank("bank3")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),this));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),this));
					}
					break;
		case 0x14 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x5fff, "bank1");
						space.install_write_bank(0x0000, 0x5fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x6000, 0xfdff, "bank3");
						space.install_write_bank(0x6000, 0xfdff, "bank4");
						membank("bank3")->set_base(ram + 0x6000);
						membank("bank4")->set_base(ram + 0x6000);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
					}
					break;
		case 0x15 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xfdff, "bank3");
						space.install_write_bank(0x2000, 0xfdff, "bank4");
						membank("bank3")->set_base(ram + 0x2000);
						membank("bank4")->set_base(ram + 0x2000);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
					}
					break;
		case 0x16 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xfdff, "bank3");
						space.install_write_bank(0x4000, 0xfdff, "bank4");
						membank("bank3")->set_base(ram + 0x4000);
						membank("bank4")->set_base(ram + 0x4000);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
					}
					break;
		case 0x17 :
					{
						// RAM
						space.install_read_bank (0x0000, 0xfdff, "bank1");
						space.install_write_bank(0x0000, 0xfdff, "bank2");
						membank("bank1")->set_base(ram);
						membank("bank2")->set_base(ram);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),this));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),this));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
					}
					break;
		case 0x18 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x5fff, "bank1");
						space.install_write_bank(0x0000, 0x5fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x6000, 0xbeff, "bank3");
						space.install_write_bank(0x6000, 0xbeff, "bank4");
						membank("bank3")->set_base(ram + 0x6000);
						membank("bank4")->set_base(ram + 0x6000);
						// System reg
						space.install_read_handler (0xbf00, 0xbfff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xbf00, 0xbfff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
					}
					break;
		case 0x19 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xbeff, "bank3");
						space.install_write_bank(0x2000, 0xbeff, "bank4");
						membank("bank3")->set_base(ram + 0x2000);
						membank("bank4")->set_base(ram + 0x2000);
						// System reg
						space.install_read_handler (0xbf00, 0xbfff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xbf00, 0xbfff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
					}
					break;
		case 0x1A :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xbeff, "bank3");
						space.install_write_bank(0x4000, 0xbeff, "bank4");
						membank("bank3")->set_base(ram + 0x4000);
						membank("bank4")->set_base(ram + 0x4000);
						// System reg
						space.install_read_handler (0xbf00, 0xbfff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xbf00, 0xbfff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
					}
					break;
		case 0x1B :
					{
						// RAM
						space.install_read_bank (0x0000, 0xbeff, "bank1");
						space.install_write_bank(0x0000, 0xbeff, "bank2");
						membank("bank1")->set_base(ram);
						membank("bank2")->set_base(ram);
						// System reg
						space.install_read_handler (0xbf00, 0xbfff, read8_delegate(FUNC(pk8020_state::sysreg_r),this));
						space.install_write_handler(0xbf00, 0xbfff, write8_delegate(FUNC(pk8020_state::sysreg_w),this));
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
					}
					break;
		case 0x1C :
					{
						// ROM
						space.install_read_bank (0x0000, 0x5fff, "bank1");
						space.install_write_bank(0x0000, 0x5fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x6000, 0xbfff, "bank3");
						space.install_write_bank(0x6000, 0xbfff, "bank4");
						membank("bank3")->set_base(ram + 0x6000);
						membank("bank4")->set_base(ram + 0x6000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
					}
					break;
		case 0x1D :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xbfff, "bank3");
						space.install_write_bank(0x2000, 0xbfff, "bank4");
						membank("bank3")->set_base(ram + 0x2000);
						membank("bank4")->set_base(ram + 0x2000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
					}
					break;
		case 0x1E :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						membank("bank1")->set_base(mem + 0x10000);
						membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xbfff, "bank3");
						space.install_write_bank(0x4000, 0xbfff, "bank4");
						membank("bank3")->set_base(ram + 0x4000);
						membank("bank4")->set_base(ram + 0x4000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
					}
					break;
		case 0x1F :
					{
						// RAM
						space.install_read_bank (0x0000, 0xbfff, "bank1");
						space.install_write_bank(0x0000, 0xbfff, "bank2");
						membank("bank1")->set_base(ram);
						membank("bank2")->set_base(ram);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),this));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),this));
					}
					break;

	}
}

READ8_MEMBER(pk8020_state::pk8020_porta_r)
{
	return 0xf0 | (m_takt <<1) | (m_text_attr)<<3;
}

WRITE8_MEMBER(pk8020_state::pk8020_portc_w)
{
	m_video_page_access =(data>>6) & 3;
	m_attr = (data >> 4) & 3;
	m_wide = (data >> 3) & 1;
	m_font = (data >> 2) & 1;
	m_video_page = (data & 3);


	m_portc_data = data;
}

WRITE8_MEMBER(pk8020_state::pk8020_portb_w)
{
	floppy_image_device *floppy = NULL;

	// Turn all motors off
	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(1);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(1);
	if (m_floppy2->get_device()) m_floppy2->get_device()->mon_w(1);
	if (m_floppy3->get_device()) m_floppy3->get_device()->mon_w(1);

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();
	if (BIT(data, 2)) floppy = m_floppy2->get_device();
	if (BIT(data, 3)) floppy = m_floppy3->get_device();

	m_wd1793->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 4));
	}

	// todo: at least bit 5 and bit 7 is connected to something too...
}

READ8_MEMBER(pk8020_state::pk8020_portc_r)
{
	return m_portc_data;
}


WRITE8_MEMBER(pk8020_state::pk8020_2_portc_w)
{
	m_sound_gate = BIT(data,3);

	m_speaker->level_w(m_sound_gate ? m_sound_level : 0);
}

WRITE_LINE_MEMBER(pk8020_state::pk8020_pit_out0)
{
	m_sound_level = state;

	m_speaker->level_w(m_sound_gate ? m_sound_level : 0);
}


WRITE_LINE_MEMBER(pk8020_state::pk8020_pit_out1)
{
}


void pk8020_state::machine_start()
{
	static const char *const keynames[] = {
		"LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7",
		"LINE8", "LINE9", "LINE10", "LINE11", "LINE12", "LINE13", "LINE14", "LINE15"
	};

	for ( int i = 0; i < 16; i++ )
	{
		m_io_port[i] = ioport(keynames[i]);
	}
}

void pk8020_state::machine_reset()
{
	pk8020_set_bank(0);

	m_sound_gate = 0;
	m_sound_level = 0;
}

INTERRUPT_GEN_MEMBER(pk8020_state::pk8020_interrupt)
{
	m_takt ^= 1;
	m_pic8259->ir4_w(1);
}
