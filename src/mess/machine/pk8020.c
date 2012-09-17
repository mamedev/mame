/***************************************************************************

        PK-8020 driver by Miodrag Milanovic
            based on work of Sergey Erokhin from pk8020.narod.ru

        18/07/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/pk8020.h"
#include "cpu/i8085/i8085.h"
#include "machine/wd17xx.h"
#include "machine/ram.h"
#include "imagedev/flopdrv.h"

static void pk8020_set_bank(running_machine &machine,UINT8 data);


READ8_MEMBER(pk8020_state::keyboard_r)
{
	static const char *const keynames[] = {
		"LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7",
		"LINE8", "LINE9", "LINE10", "LINE11", "LINE12", "LINE13", "LINE14", "LINE15"
	};
	UINT8 retVal=0x00;
	UINT8 line = 0;
	if (offset & 0x100)  line=8;

	if (offset & 0x0001) retVal|=ioport(keynames[line])->read();
	line++;
	if (offset & 0x0002) retVal|=ioport(keynames[line])->read();
	line++;
	if (offset & 0x0004) retVal|=ioport(keynames[line])->read();
	line++;
	if (offset & 0x0008) retVal|=ioport(keynames[line])->read();
	line++;
	if (offset & 0x0010) retVal|=ioport(keynames[line])->read();
	line++;
	if (offset & 0x0020) retVal|=ioport(keynames[line])->read();
	line++;
	if (offset & 0x0040) retVal|=ioport(keynames[line])->read();
	line++;
	if (offset & 0x0080) retVal|=ioport(keynames[line])->read();
	line++;

	return retVal;
}

READ8_MEMBER(pk8020_state::sysreg_r)
{
	return machine().device<ram_device>(RAM_TAG)->pointer()[offset];
}
WRITE8_MEMBER(pk8020_state::sysreg_w)
{
	if (BIT(offset,7)==0) {
		pk8020_set_bank(machine(),data >> 2);
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
		palette_set_color( machine(), number, MAKE_RGB(r,g,b) );
	}
}

READ8_MEMBER(pk8020_state::text_r)
{
	if (m_attr == 3) m_text_attr=machine().device<ram_device>(RAM_TAG)->pointer()[0x40400+offset];
	return machine().device<ram_device>(RAM_TAG)->pointer()[0x40000+offset];
}

WRITE8_MEMBER(pk8020_state::text_w)
{
	UINT8 *ram = machine().device<ram_device>(RAM_TAG)->pointer();
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
	UINT8 *addr = machine().device<ram_device>(RAM_TAG)->pointer() + 0x10000 + (m_video_page_access * 0xC000);
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
	UINT8 *addr = machine().device<ram_device>(RAM_TAG)->pointer() + 0x10000 + (m_video_page_access * 0xC000);
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
	i8255_device *ppi1 = machine().device<i8255_device>("ppi8255_1");
	i8255_device *ppi2 = machine().device<i8255_device>("ppi8255_2");
	i8255_device *ppi3 = machine().device<i8255_device>("ppi8255_3");
	device_t *pit = machine().device("pit8253");
	device_t *pic = machine().device("pic8259");
	i8251_device *rs232 = machine().device<i8251_device>("rs232");
	i8251_device *lan = machine().device<i8251_device>("lan");
	device_t *fdc = machine().device("wd1793");

	switch(offset & 0x38)
	{
		case 0x00: return pit8253_r(pit,space, offset & 3);
		case 0x08: return ppi3->read(space,offset & 3);
		case 0x10: switch(offset & 1) {
						case 0 : return rs232->data_r(space,0);
						case 1 : return rs232->status_r(space,0);
				   }
				   break;
		case 0x18: switch(offset & 3) {
						case 0 : return wd17xx_status_r(fdc,space, 0);
						case 1 : return wd17xx_track_r(fdc,space, 0);
						case 2 : return wd17xx_sector_r(fdc,space, 0);
						case 3 : return wd17xx_data_r(fdc,space, 0);
					}
					break;
		case 0x20: switch(offset & 1) {
						case 0 : return lan->data_r(space,0);
						case 1 : return lan->status_r(space,0);
				   }
				   break;
		case 0x28: return pic8259_r(pic,space, offset & 1);
		case 0x30: return ppi2->read(space,offset & 3);
		case 0x38: return ppi1->read(space,offset & 3);
	}
	return 0xff;
}

WRITE8_MEMBER(pk8020_state::devices_w)
{
	i8255_device *ppi1 = machine().device<i8255_device>("ppi8255_1");
	i8255_device *ppi2 = machine().device<i8255_device>("ppi8255_2");
	i8255_device *ppi3 = machine().device<i8255_device>("ppi8255_3");
	device_t *pit = machine().device("pit8253");
	device_t *pic = machine().device("pic8259");
	i8251_device *rs232 = machine().device<i8251_device>("rs232");
	i8251_device *lan = machine().device<i8251_device>("lan");
	device_t *fdc = machine().device("wd1793");

	switch(offset & 0x38)
	{
		case 0x00: pit8253_w(pit,space, offset & 3,data); break;
		case 0x08: ppi3->write(space,offset & 3,data); break;
		case 0x10: switch(offset & 1) {
						case 0 : rs232->data_w(space,0,data); break;
						case 1 : rs232->control_w(space,0,data); break;
				   }
				   break;
		case 0x18: switch(offset & 3) {
						case 0 : wd17xx_command_w(fdc,space, 0,data);break;
						case 1 : wd17xx_track_w(fdc,space, 0,data);break;
						case 2 : wd17xx_sector_w(fdc,space, 0,data);break;
						case 3 : wd17xx_data_w(fdc,space, 0,data);break;
					}
					break;
		case 0x20: switch(offset & 1) {
						case 0 : lan->data_w(space,0,data); break;
						case 1 : lan->control_w(space,0,data); break;
				   }
				   break;
		case 0x28: pic8259_w(pic,space, offset & 1,data);break;
		case 0x30: ppi2->write(space,offset & 3,data); break;
		case 0x38: ppi1->write(space,offset & 3,data); break;
	}
}

static void pk8020_set_bank(running_machine &machine,UINT8 data)
{
	pk8020_state *state = machine.driver_data<pk8020_state>();
	address_space &space = *machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *mem = state->memregion("maincpu")->base();
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();

	switch(data & 0x1F) {
		case 0x00 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x37ff, "bank1");
						space.install_write_bank(0x0000, 0x37ff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// Keyboard
						space.install_read_handler (0x3800, 0x39ff, read8_delegate(FUNC(pk8020_state::keyboard_r),state));
						space.install_write_bank(0x3800, 0x39ff, "bank3");
						state->membank("bank3")->set_base(ram + 0x3800);
						// System reg
						space.install_read_handler (0x3a00, 0x3aff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0x3a00, 0x3aff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
						// Devices
						space.install_read_handler (0x3b00, 0x3bff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0x3b00, 0x3bff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// Text Video Memory
						space.install_read_handler (0x3c00, 0x3fff, read8_delegate(FUNC(pk8020_state::text_r),state));
						space.install_write_handler(0x3c00, 0x3fff, write8_delegate(FUNC(pk8020_state::text_w),state));
						// RAM
						space.install_read_bank (0x4000, 0xffff, "bank4");
						space.install_write_bank(0x4000, 0xffff, "bank5");
						state->membank("bank4")->set_base(ram + 0x4000);
						state->membank("bank5")->set_base(ram + 0x4000);
					}
					break;
		case 0x01 : {
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xffff, "bank3");
						space.install_write_bank(0x2000, 0xffff, "bank4");
						state->membank("bank3")->set_base(ram + 0x2000);
						state->membank("bank4")->set_base(ram + 0x2000);
					}
					break;
		case 0x02 : {
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xffff, "bank3");
						space.install_write_bank(0x4000, 0xffff, "bank4");
						state->membank("bank3")->set_base(ram + 0x4000);
						state->membank("bank4")->set_base(ram + 0x4000);
					}
					break;
		case 0x03 : {
						// RAM
						space.install_read_bank (0x0000, 0xffff, "bank1");
						space.install_write_bank(0x0000, 0xffff, "bank2");
						state->membank("bank1")->set_base(ram);
						state->membank("bank2")->set_base(ram);
					}
					break;
		case 0x04 :
		case 0x05 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xf7ff, "bank3");
						space.install_write_bank(0x2000, 0xf7ff, "bank4");
						state->membank("bank3")->set_base(ram + 0x2000);
						state->membank("bank4")->set_base(ram + 0x2000);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),state));
						space.install_write_bank(0xf800, 0xf9ff, "bank5");
						state->membank("bank5")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),state));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),state));
					}
					break;
		case 0x06 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xf7ff, "bank3");
						space.install_write_bank(0x4000, 0xf7ff, "bank4");
						state->membank("bank3")->set_base(ram + 0x4000);
						state->membank("bank4")->set_base(ram + 0x4000);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),state));
						space.install_write_bank(0xf800, 0xf9ff, "bank5");
						state->membank("bank5")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),state));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),state));
					}
					break;
		case 0x07 :
					{
						// RAM
						space.install_read_bank (0x0000, 0xf7ff, "bank1");
						space.install_write_bank(0x0000, 0xf7ff, "bank2");
						state->membank("bank1")->set_base(ram);
						state->membank("bank2")->set_base(ram);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),state));
						space.install_write_bank(0xf800, 0xf9ff, "bank3");
						state->membank("bank3")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),state));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),state));
					}
					break;
		case 0x08 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// Keyboard
						space.install_read_handler (0x3800, 0x39ff, read8_delegate(FUNC(pk8020_state::keyboard_r),state));
						space.install_write_bank(0x3800, 0x39ff, "bank3");
						state->membank("bank3")->set_base(ram + 0x3800);
						// System reg
						space.install_read_handler (0x3a00, 0x3aff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0x3a00, 0x3aff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
						// Devices
						space.install_read_handler (0x3b00, 0x3bff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0x3b00, 0x3bff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// Text Video Memory
						space.install_read_handler (0x3c00, 0x3fff, read8_delegate(FUNC(pk8020_state::text_r),state));
						space.install_write_handler(0x3c00, 0x3fff, write8_delegate(FUNC(pk8020_state::text_w),state));
						// RAM
						space.install_read_bank (0x4000, 0xbfff, "bank4");
						space.install_write_bank(0x4000, 0xbfff, "bank5");
						state->membank("bank4")->set_base(ram + 0x4000);
						state->membank("bank5")->set_base(ram + 0x4000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),state));

					}
					break;
		case 0x09 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xbfff, "bank3");
						space.install_write_bank(0x2000, 0xbfff, "bank4");
						state->membank("bank3")->set_base(ram + 0x2000);
						state->membank("bank4")->set_base(ram + 0x2000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
					}
					break;
		case 0x0A :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xbfff, "bank3");
						space.install_write_bank(0x4000, 0xbfff, "bank4");
						state->membank("bank3")->set_base(ram + 0x4000);
						state->membank("bank4")->set_base(ram + 0x4000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
					}
					break;
		case 0x0B :
					{
						// RAM
						space.install_read_bank (0x0000, 0xbfff, "bank1");
						space.install_write_bank(0x0000, 0xbfff, "bank2");
						state->membank("bank1")->set_base(ram + 0x0000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
					}
					break;
		case 0x0C :
		case 0x0D :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0x3fff, "bank3");
						space.install_write_bank(0x2000, 0x3fff, "bank4");
						state->membank("bank3")->set_base(ram + 0x2000);
						state->membank("bank4")->set_base(ram + 0x2000);
						// Video RAM
						space.install_read_handler (0x4000, 0x7fff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0x4000, 0x7fff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
						// RAM
						space.install_read_bank (0x8000, 0xfdff, "bank5");
						space.install_write_bank(0x8000, 0xfdff, "bank6");
						state->membank("bank5")->set_base(ram + 0x8000);
						state->membank("bank6")->set_base(ram + 0x8000);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
					}
					break;
		case 0x0E :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// Video RAM
						space.install_read_handler (0x4000, 0x7fff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0x4000, 0x7fff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
						// RAM
						space.install_read_bank (0x8000, 0xfdff, "bank5");
						space.install_write_bank(0x8000, 0xfdff, "bank6");
						state->membank("bank5")->set_base(ram + 0x8000);
						state->membank("bank6")->set_base(ram + 0x8000);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
					}
					break;
		case 0x0F :
					{
						// RAM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						state->membank("bank1")->set_base(ram + 0x0000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// Video RAM
						space.install_read_handler (0x4000, 0x7fff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0x4000, 0x7fff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
						// RAM
						space.install_read_bank (0x8000, 0xfdff, "bank3");
						space.install_write_bank(0x8000, 0xfdff, "bank4");
						state->membank("bank3")->set_base(ram + 0x8000);
						state->membank("bank4")->set_base(ram + 0x8000);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
					}
					break;
		case 0x10 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x5fff, "bank1");
						space.install_write_bank(0x0000, 0x5fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x6000, 0xf7ff, "bank3");
						space.install_write_bank(0x6000, 0xf7ff, "bank4");
						state->membank("bank3")->set_base(ram + 0x6000);
						state->membank("bank4")->set_base(ram + 0x6000);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),state));
						space.install_write_bank(0xf800, 0xf9ff, "bank5");
						state->membank("bank5")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),state));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),state));
					}
					break;
		case 0x11 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xf7ff, "bank3");
						space.install_write_bank(0x2000, 0xf7ff, "bank4");
						state->membank("bank3")->set_base(ram + 0x2000);
						state->membank("bank4")->set_base(ram + 0x2000);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),state));
						space.install_write_bank(0xf800, 0xf9ff, "bank5");
						state->membank("bank5")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),state));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),state));
					}
					break;
		case 0x12 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xf7ff, "bank3");
						space.install_write_bank(0x4000, 0xf7ff, "bank4");
						state->membank("bank3")->set_base(ram + 0x4000);
						state->membank("bank4")->set_base(ram + 0x4000);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),state));
						space.install_write_bank(0xf800, 0xf9ff, "bank5");
						state->membank("bank5")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),state));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),state));
					}
					break;
		case 0x13 :
					{
						// RAM
						space.install_read_bank (0x0000, 0xf7ff, "bank1");
						space.install_write_bank(0x0000, 0xf7ff, "bank2");
						state->membank("bank1")->set_base(ram + 0x0000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// Keyboard
						space.install_read_handler (0xf800, 0xf9ff, read8_delegate(FUNC(pk8020_state::keyboard_r),state));
						space.install_write_bank(0xf800, 0xf9ff, "bank3");
						state->membank("bank3")->set_base(ram + 0xf800);
						// System reg
						space.install_read_handler (0xfa00, 0xfaff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
						// Devices
						space.install_read_handler (0xfb00, 0xfbff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// Text Video Memory
						space.install_read_handler (0xfc00, 0xffff, read8_delegate(FUNC(pk8020_state::text_r),state));
						space.install_write_handler(0xfc00, 0xffff, write8_delegate(FUNC(pk8020_state::text_w),state));
					}
					break;
		case 0x14 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x5fff, "bank1");
						space.install_write_bank(0x0000, 0x5fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x6000, 0xfdff, "bank3");
						space.install_write_bank(0x6000, 0xfdff, "bank4");
						state->membank("bank3")->set_base(ram + 0x6000);
						state->membank("bank4")->set_base(ram + 0x6000);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
					}
					break;
		case 0x15 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xfdff, "bank3");
						space.install_write_bank(0x2000, 0xfdff, "bank4");
						state->membank("bank3")->set_base(ram + 0x2000);
						state->membank("bank4")->set_base(ram + 0x2000);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
					}
					break;
		case 0x16 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xfdff, "bank3");
						space.install_write_bank(0x4000, 0xfdff, "bank4");
						state->membank("bank3")->set_base(ram + 0x4000);
						state->membank("bank4")->set_base(ram + 0x4000);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
					}
					break;
		case 0x17 :
					{
						// RAM
						space.install_read_bank (0x0000, 0xfdff, "bank1");
						space.install_write_bank(0x0000, 0xfdff, "bank2");
						state->membank("bank1")->set_base(ram);
						state->membank("bank2")->set_base(ram);
						// Devices
						space.install_read_handler (0xfe00, 0xfeff, read8_delegate(FUNC(pk8020_state::devices_r),state));
						space.install_write_handler(0xfe00, 0xfeff, write8_delegate(FUNC(pk8020_state::devices_w),state));
						// System reg
						space.install_read_handler (0xff00, 0xffff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
					}
					break;
		case 0x18 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x5fff, "bank1");
						space.install_write_bank(0x0000, 0x5fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x6000, 0xbeff, "bank3");
						space.install_write_bank(0x6000, 0xbeff, "bank4");
						state->membank("bank3")->set_base(ram + 0x6000);
						state->membank("bank4")->set_base(ram + 0x6000);
						// System reg
						space.install_read_handler (0xbf00, 0xbfff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xbf00, 0xbfff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
					}
					break;
		case 0x19 :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xbeff, "bank3");
						space.install_write_bank(0x2000, 0xbeff, "bank4");
						state->membank("bank3")->set_base(ram + 0x2000);
						state->membank("bank4")->set_base(ram + 0x2000);
						// System reg
						space.install_read_handler (0xbf00, 0xbfff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xbf00, 0xbfff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
					}
					break;
		case 0x1A :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xbeff, "bank3");
						space.install_write_bank(0x4000, 0xbeff, "bank4");
						state->membank("bank3")->set_base(ram + 0x4000);
						state->membank("bank4")->set_base(ram + 0x4000);
						// System reg
						space.install_read_handler (0xbf00, 0xbfff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xbf00, 0xbfff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
					}
					break;
		case 0x1B :
					{
						// RAM
						space.install_read_bank (0x0000, 0xbeff, "bank1");
						space.install_write_bank(0x0000, 0xbeff, "bank2");
						state->membank("bank1")->set_base(ram);
						state->membank("bank2")->set_base(ram);
						// System reg
						space.install_read_handler (0xbf00, 0xbfff, read8_delegate(FUNC(pk8020_state::sysreg_r),state));
						space.install_write_handler(0xbf00, 0xbfff, write8_delegate(FUNC(pk8020_state::sysreg_w),state));
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
					}
					break;
		case 0x1C :
					{
						// ROM
						space.install_read_bank (0x0000, 0x5fff, "bank1");
						space.install_write_bank(0x0000, 0x5fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x6000, 0xbfff, "bank3");
						space.install_write_bank(0x6000, 0xbfff, "bank4");
						state->membank("bank3")->set_base(ram + 0x6000);
						state->membank("bank4")->set_base(ram + 0x6000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
					}
					break;
		case 0x1D :
					{
						// ROM
						space.install_read_bank (0x0000, 0x1fff, "bank1");
						space.install_write_bank(0x0000, 0x1fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x2000, 0xbfff, "bank3");
						space.install_write_bank(0x2000, 0xbfff, "bank4");
						state->membank("bank3")->set_base(ram + 0x2000);
						state->membank("bank4")->set_base(ram + 0x2000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
					}
					break;
		case 0x1E :
					{
						// ROM
						space.install_read_bank (0x0000, 0x3fff, "bank1");
						space.install_write_bank(0x0000, 0x3fff, "bank2");
						state->membank("bank1")->set_base(mem + 0x10000);
						state->membank("bank2")->set_base(ram + 0x0000);
						// RAM
						space.install_read_bank (0x4000, 0xbfff, "bank3");
						space.install_write_bank(0x4000, 0xbfff, "bank4");
						state->membank("bank3")->set_base(ram + 0x4000);
						state->membank("bank4")->set_base(ram + 0x4000);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
					}
					break;
		case 0x1F :
					{
						// RAM
						space.install_read_bank (0x0000, 0xbfff, "bank1");
						space.install_write_bank(0x0000, 0xbfff, "bank2");
						state->membank("bank1")->set_base(ram);
						state->membank("bank2")->set_base(ram);
						// Video RAM
						space.install_read_handler (0xc000, 0xffff, read8_delegate(FUNC(pk8020_state::gzu_r),state));
						space.install_write_handler(0xc000, 0xffff, write8_delegate(FUNC(pk8020_state::gzu_w),state));
					}
					break;

	}
}

static READ8_DEVICE_HANDLER(pk8020_porta_r)
{
	pk8020_state *state = device->machine().driver_data<pk8020_state>();
	return 0xf0 | (state->m_takt <<1) | (state->m_text_attr)<<3;
}

static WRITE8_DEVICE_HANDLER(pk8020_portc_w)
{
	pk8020_state *state = device->machine().driver_data<pk8020_state>();
	state->m_video_page_access =(data>>6) & 3;
	state->m_attr = (data >> 4) & 3;
	state->m_wide = (data >> 3) & 1;
	state->m_font = (data >> 2) & 1;
	state->m_video_page = (data & 3);


	state->m_portc_data = data;
}

static WRITE8_DEVICE_HANDLER(pk8020_portb_w)
{
	device_t *fdc = device->machine().device("wd1793");
	// Turn all motors off
	floppy_mon_w(floppy_get_device(device->machine(), 0), 1);
	floppy_mon_w(floppy_get_device(device->machine(), 1), 1);
	floppy_mon_w(floppy_get_device(device->machine(), 2), 1);
	floppy_mon_w(floppy_get_device(device->machine(), 3), 1);
	wd17xx_set_side(fdc,BIT(data,4));
	if (BIT(data,0)) {
		wd17xx_set_drive(fdc,0);
		floppy_mon_w(floppy_get_device(device->machine(), 0), 0);
		floppy_drive_set_ready_state(floppy_get_device(device->machine(), 0), 1, 1);
	} else if (BIT(data,1)) {
		wd17xx_set_drive(fdc,1);
		floppy_mon_w(floppy_get_device(device->machine(), 1), 0);
		floppy_drive_set_ready_state(floppy_get_device(device->machine(), 1), 1, 1);
	} else if (BIT(data,2)) {
		wd17xx_set_drive(fdc,2);
		floppy_mon_w(floppy_get_device(device->machine(), 2), 0);
		floppy_drive_set_ready_state(floppy_get_device(device->machine(), 2), 1, 1);
	} else if (BIT(data,3)) {
		wd17xx_set_drive(fdc,3);
		floppy_mon_w(floppy_get_device(device->machine(), 3), 0);
		floppy_drive_set_ready_state(floppy_get_device(device->machine(), 3), 1, 1);
	}
}

static READ8_DEVICE_HANDLER(pk8020_portc_r)
{
	pk8020_state *state = device->machine().driver_data<pk8020_state>();
	return state->m_portc_data;
}


I8255A_INTERFACE( pk8020_ppi8255_interface_1 )
{
	DEVCB_HANDLER(pk8020_porta_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(pk8020_portb_w),
	DEVCB_HANDLER(pk8020_portc_r),
	DEVCB_HANDLER(pk8020_portc_w)
};

static WRITE8_DEVICE_HANDLER(pk8020_2_portc_w)
{
	pk8020_state *state = device->machine().driver_data<pk8020_state>();
	device_t *speaker = device->machine().device(SPEAKER_TAG);

	state->m_sound_gate = BIT(data,3);

	speaker_level_w(speaker, state->m_sound_gate ? state->m_sound_level : 0);
}

I8255A_INTERFACE( pk8020_ppi8255_interface_2 )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(pk8020_2_portc_w)
};

I8255A_INTERFACE( pk8020_ppi8255_interface_3 )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static WRITE_LINE_DEVICE_HANDLER( pk8020_pit_out0 )
{
	pk8020_state *drvstate = device->machine().driver_data<pk8020_state>();
	device_t *speaker = device->machine().device(SPEAKER_TAG);

	drvstate->m_sound_level = state;

	speaker_level_w(speaker, drvstate->m_sound_gate ? drvstate->m_sound_level : 0);
}


static WRITE_LINE_DEVICE_HANDLER(pk8020_pit_out1)
{
}


const struct pit8253_config pk8020_pit8253_intf =
{
	{
		{
			XTAL_20MHz / 10,
			DEVCB_NULL,
			DEVCB_LINE(pk8020_pit_out0)
		},
		{
			XTAL_20MHz / 10,
			DEVCB_NULL,
			DEVCB_LINE(pk8020_pit_out1)
		},
		{
			(XTAL_20MHz / 8) / 164,
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pic8259", pic8259_ir5_w)
		}
	}
};

static WRITE_LINE_DEVICE_HANDLER( pk8020_pic_set_int_line )
{
	device->machine().device("maincpu")->execute().set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}

const struct pic8259_interface pk8020_pic8259_config =
{
	DEVCB_LINE(pk8020_pic_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};

static IRQ_CALLBACK( pk8020_irq_callback )
{
	return pic8259_acknowledge(device->machine().device("pic8259"));
}

void pk8020_state::machine_reset()
{
	pk8020_set_bank(machine(),0);
	machine().device("maincpu")->execute().set_irq_acknowledge_callback(pk8020_irq_callback);

	m_sound_gate = 0;
	m_sound_level = 0;
}

INTERRUPT_GEN( pk8020_interrupt )
{
	pk8020_state *state = device->machine().driver_data<pk8020_state>();
	state->m_takt ^= 1;
	pic8259_ir4_w(device->machine().device("pic8259"), 1);
}
