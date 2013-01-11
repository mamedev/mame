#include "emu.h"

#include "machine/pit8253.h"
#include "machine/pc_lpt.h"
#include "machine/pcshare.h"
#include "includes/amstr_pc.h"
#include "includes/pc.h"
#include "video/pc_vga.h"


/* pc20 (v2)
   fc078
   fc102 color/mono selection
   fc166
   fc1b4
   fd841 (output something)
   ff17c (output something, read monitor type inputs)
   fc212
   fc26c
   fc2df
   fc3fe
   fc0f4
   fc432
   fc49f
   fc514
   fc566
   fc5db
   fc622 in 3de

port 0379 read
port 03de write/read
 */

/* pc1512 (v1)
   fc1b5
   fc1f1
   fc264
   fc310
   fc319
   fc385
   fc436
   fc459
   fc4cb
   fc557
   fc591
   fc624
   fc768
   fc818
   fc87d display amstrad ..
    fca17 keyboard check
     fca69
     fd680
      fd7f9
     fca7b !keyboard interrupt routine for this check
 */


/* pc1640 (v3)
   bios power up self test
   important addresses

   fc0c9
   fc0f2
   fc12e
   fc193
   fc20e
   fc2c1
   fc2d4


   fc375
   fc3ba
   fc3e1
   fc412
   fc43c
   fc47d
   fc48f
   fc51f
   fc5a2
   fc5dd mouse

   fc1c0
   fc5fa
    the following when language selection not 0 (test for presence of 0x80000..0x9ffff ram)
    fc60e
    fc667
   fc678
   fc6e5
   fc72e
   fc78f
   fc7cb ; coprocessor related
   fc834
    feda6 no problem with disk inserted

   fca2a

   cmos ram 28 0 amstrad pc1512 integrated cga
   cmos ram 23 dipswitches?
*/

static struct {
	struct {
		UINT8 x,y; //byte clipping needed
	} mouse;

	// 64 system status register?
	UINT8 port60;
	UINT8 port61;
	UINT8 port62;
	UINT8 port65;

	int dipstate;
} pc1640={{0}, 0};

/* test sequence in bios
 write 00 to 65
 write 30 to 61
 read 62 and (0x10)
 write 34 to 61
 read 62 and (0x0f)
 return or of the 2 62 reads

 allows set of the original ibm pc "dipswitches"!!!!

 66 write gives reset?
*/

/* mouse x counter at 0x78 (read- writable)
   mouse y counter at 0x7a (read- writable)

   mouse button 1,2 keys
   joystick (4 directions, 2 buttons) keys
   these get value from cmos ram
   74 15 00 enter
   70 17 00 forward del
   77 1b 00 joystick button 1
   78 19 00 joystick button 2


   79 00 4d right
   7a 00 4b left
   7b 00 50 down
   7c 00 48 up

   7e 00 01 mouse button left
   7d 01 01 mouse button right
*/

WRITE8_HANDLER( pc1640_port60_w )
{
	switch (offset) {
	case 1:
		pc1640.port61=data;
		if (data==0x30) pc1640.port62=(pc1640.port65&0x10)>>4;
		else if (data==0x34) pc1640.port62=pc1640.port65&0xf;
		pit8253_gate2_w(space.machine().device("pit8253"), BIT(data, 0));
		pc_speaker_set_spkrdata( space.machine(), data & 0x02 );
		pc_keyb_set_clock(data&0x40);
		break;
	case 4:
		if (data&0x80) {
			pc1640.port60=data^0x8d;
		} else {
			pc1640.port60=data;
		}
		break;
	case 5:
		// stores the configuration data for port 62 configuration dipswitch emulation
		pc1640.port65=data;
		break;
	}

	logerror("pc1640 write %.2x %.2x\n",offset,data);
}


READ8_HANDLER( pc1640_port60_r )
{
	int data=0;
	switch (offset) {
	case 0:
		if (pc1640.port61&0x80)
			data=pc1640.port60;
		else
			data = pc_keyb_read();
		break;

	case 1:
		data = pc1640.port61;
		break;

	case 2:
		data = pc1640.port62;
		if (pit8253_get_output(space.machine().device("pit8253"), 2))
			data |= 0x20;
		break;
	}
	return data;
}

READ8_HANDLER( pc200_port378_r )
{
	device_t *lpt = space.machine().device("lpt_1");
	UINT8 data = pc_lpt_r(lpt, space, offset);

	if (offset == 1)
		data = (data & ~7) | (space.machine().root_device().ioport("DSW0")->read() & 7);
	if (offset == 2)
		data = (data & ~0xe0) | (space.machine().root_device().ioport("DSW0")->read() & 0xc0);

	return data;
}

READ8_HANDLER( pc200_port278_r )
{
	device_t *lpt = space.machine().device("lpt_2");
	UINT8 data = pc_lpt_r(lpt, space, offset);

	if (offset == 1)
		data = (data & ~7) | (space.machine().root_device().ioport("DSW0")->read() & 7);
	if (offset == 2)
		data = (data & ~0xe0) | (space.machine().root_device().ioport("DSW0")->read() & 0xc0);

	return data;
}


READ8_HANDLER( pc1640_port378_r )
{
		device_t *lpt = space.machine().device("lpt_1");
		UINT8 data = pc_lpt_r(lpt, space, offset);

	if (offset == 1)
		data=(data & ~7) | (space.machine().root_device().ioport("DSW0")->read() & 7);
	if (offset == 2)
	{
		switch (pc1640.dipstate)
		{
		case 0:
			data = (data&~0xe0) | (space.machine().root_device().ioport("DSW0")->read() & 0xe0);
			break;
		case 1:
			data = (data&~0xe0) | ((space.machine().root_device().ioport("DSW0")->read() & 0xe000)>>8);
			break;
		case 2:
			data = (data&~0xe0) | ((space.machine().root_device().ioport("DSW0")->read() & 0xe00)>>4);
			break;

		}
	}
	return data;
}

READ8_HANDLER( pc1640_port3d0_r )
{
	if (offset==0xa) pc1640.dipstate=0;
	return space.read_byte(0x3d0+offset);
}

READ8_HANDLER( pc1640_port4278_r )
{
	if (offset==2) pc1640.dipstate=1;
	// read parallelport
	return 0;
}

READ8_HANDLER( pc1640_port278_r )
{
	if ((offset==2)||(offset==0)) pc1640.dipstate=2;
	// read parallelport
	return 0;
}

READ8_HANDLER( pc1640_mouse_x_r )
{
	return pc1640.mouse.x - space.machine().root_device().ioport("pc_mouse_x")->read();
}

READ8_HANDLER( pc1640_mouse_y_r )
{
	return pc1640.mouse.y - space.machine().root_device().ioport("pc_mouse_y")->read();
}

WRITE8_HANDLER( pc1640_mouse_x_w )
{
	pc1640.mouse.x = data + space.machine().root_device().ioport("pc_mouse_x")->read();
}

WRITE8_HANDLER( pc1640_mouse_y_w )
{
	pc1640.mouse.y = data + space.machine().root_device().ioport("pc_mouse_y")->read();
}

INPUT_PORTS_START( amstrad_keyboard )

	PORT_START("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED )     /* unused scancode 0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 ^") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 &") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("<--") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)

	PORT_START("pc_keyboard_1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)

	PORT_START("pc_keyboard_2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("` ~") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L-Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)

	PORT_START("pc_keyboard_3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R-Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP * (PrtScr)") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)

	PORT_START("pc_keyboard_4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NumLock") PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ScrLock") PORT_CODE(KEYCODE_SCRLOCK)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 7 (Home)") PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 8 (Up)") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 9 (PgUp)") PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP -") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 4 (Left)") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 6 (Right)") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 1 (End)") PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_END)

	PORT_START("pc_keyboard_5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 2 (Down)") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 3 (PgDn)") PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 0 (Ins)") PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP . (Del)") PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(KEYCODE_DEL)
	PORT_BIT ( 0x0030, 0x0000, IPT_UNUSED )
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?(84/102)\\") PORT_CODE(KEYCODE_BACKSLASH2)
	PORT_BIT ( 0xff80, 0x0000, IPT_UNUSED )

	PORT_START("pc_keyboard_6")
	PORT_BIT ( 0xffff, 0x0000, IPT_UNUSED )

	PORT_START("pc_keyboard_7")
	PORT_BIT ( 0x806e, 0x0000, IPT_UNUSED )
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-->")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?Enter")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Amstrad Joystick Button 1") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Amstrad Joystick Button 2") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Amstrad Joystick Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Amstrad Joystick Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Amstrad Joystick Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Amstrad Joystick Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Amstrad Mouse Button left") PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Amstrad Mouse Button right") PORT_CODE(KEYCODE_F12)

	PORT_START("pc_mouse_misc")
	PORT_BIT ( 0xffff, 0x0000, IPT_UNUSED )

	PORT_START( "pc_mouse_x" ) /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_REVERSE

	PORT_START( "pc_mouse_y" ) /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

INPUT_PORTS_END
