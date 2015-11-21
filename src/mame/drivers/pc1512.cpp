// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Amstrad PC1512

    http://www.seasip.info/AmstradXT
    http://stason.org/TULARC/pc/hard-drives-hdd/tandon/TM262-21MB-5-25-HH-MFM-ST506.html

*/

/*

    TODO:

    - adjust mouse speed
    - pc1512 V3 VDU check fails
    - Amstrad SM2400 internal modem
    - Amstrad RP4 diagnostic ISA card (PC1512)
    - Amstrad RP5-2 diagnostic ISA card (PC1640)
    - 40291/8908 B ROM on PC1640 HD30 controller card

*/

/*
HARD DISC INSTALLATION INSTRUCTIONS

Applies to both ten and twenty megabyte versions.

1. Turn on machine
2. Insert disc 1 (the red disc) and press a key.
3. Type fdisk (RETURN)
4. At each prompt press the RETURN key (another three times).
5. The A> will now appear so now type format c:/s (RETURN).
6. Now push the Y key (RETURN).
7. The hard disc will now begin to format, if you have a 10Mb machine then it will count up to 305
   cylinders. If you have a 20 Mb machine then 610 cylinders will be counted.
8. Once this is done, take out disc 1 and insert disc 5 (the maroon disc).
9. Now type config(RETURN). This procedure will copy all five discs (order 5,1,2,3,4) onto the hard disc.
   Once finished the screen will show some information about the size of the disc and the number of files and
   directories present.
10. To now use the hard disc remove the floppy disc from the drive and store in a safe place with the other
    four discs then restart the computer by pushing Alt Ctrl and Del.
11. After a short while the AMSTRAD PC info will come up and tell you when the machine was last used
    and then after a little longer the screen will clear and will display this message,

F1=DOSPLUS.SYS
F2=DOS.SYS

Select operating system:

If you choose F1 then DOS Plus and GEM will be booted, or if you press F2 then MS-DOS will be booted.
*/

/*
HARD DISC INSTALLATION ON PC1640

PC1640 hard disc comes ready installed with the necessary software. That is discs 1 to 4 that are
supplied ready with the machine.

Howevere in the case of a disc failure it may be necessary to reinstall the supplied software.

This is done in the following way:
1. Put disc one into drive A: and boot up the system
2. Put disc four into drive A: and type CD\SUPPLEME and press return.
3. Type HDFORMAT and press return.
4. Answer YES to the screen prompt.
5. When HDFORMAT is completed remove disc four and replace with disc one.
6. Type CD\ and press return.
7. Type FDISC and press return.
8. Press return key every time you are asked a question.
9. With disc one still in drive A: type FORMAT C:/S and press return.
10. When formatting is finished replace disc one with disc four.
11. Type CD\SUPPLEME and press return.
12. Type CONFIG and press return.

After typing CONFIG the machine will proceed to copy the four system discs to the hard disc.
After copying each disc you will be prompted to insert the next disc.
You do not noeed to know in which order to insert the discs because the machine will tell you which disc is
needed next.

The system is now installed and should be tested by rebooting the machine.

It should be noted that if the hard disc is ok but the software has been corrupted or deleted you can
reinstall the software without reformatting.
This is done by following steps 11 and 12.
*/

#include "includes/pc1512.h"
#include "bus/rs232/rs232.h"
#include "bus/isa/ega.h"
#include "softlist.h"


//**************************************************************************
//  SYSTEM STATUS REGISTER
//**************************************************************************

//-------------------------------------------------
//  system_r -
//-------------------------------------------------

READ8_MEMBER( pc1512_state::system_r )
{
	UINT8 data = 0;

	switch (offset)
	{
	case 0:
		if (BIT(m_port61, 7))
		{
			/*

			    bit     description

			    0       1
			    1       8087 NDP installed
			    2       1
			    3       1
			    4       DDM0
			    5       DDM1
			    6       second floppy disk drive installed
			    7       0

			*/

			data = m_status1;
		}
		else
		{
			data = m_kbd;
			m_kb_bits = 0;
			m_kb->data_w(1);
			m_pic->ir1_w(CLEAR_LINE);
		}
		break;

	case 1:
		data = m_port61;
		break;

	case 2:
		/*

		    bit     description

		    0       RAM0 / RAM4
		    1       RAM1
		    2       RAM2
		    3       RAM3
		    4       undefined
		    5       8253 PIT OUT2 output
		    6       external parity error (I/OCHCK from expansion bus)
		    7       on-board system RAM parity error

		*/

		if (BIT(m_port61, 2))
			data = m_status2 & 0x0f;
		else
			data = m_status2 >> 4;

		data |= m_pit2 << 5;
		break;
	}

	return data;
}


//-------------------------------------------------
//  system_w -
//-------------------------------------------------

WRITE8_MEMBER( pc1512_state::system_w )
{
	switch (offset)
	{
	case 1:
		/*

		    bit     description

		    0       8253 GATE 2 (speaker modulate)
		    1       speaker drive
		    2       enable port C LSB / disable MSB
		    3       undefined
		    4       disable parity checking of on-board RAM
		    5       prevent external parity errors from causing NMI
		    6       enable incoming Keyboard Clock
		    7       enable Status-1/Disable Keyboard Code on Port A

		*/

		m_port61 = data;

		m_pit->write_gate2(BIT(data, 0));

		m_speaker_drive = BIT(data, 1);
		update_speaker();

		m_kb->clock_w(BIT(data, 6));
		break;

	case 4:
		/*

		    bit     description

		    0
		    1       PA1 - 8087 NDP installed
		    2
		    3
		    4       PA4 - DDM0
		    5       PA5 - DDM1
		    6       PA6 - Second Floppy disk drive installed
		    7

		*/

		if (BIT(data, 7))
			m_status1 = data ^ 0x8d;
		else
			m_status1 = data;
		break;

	case 5:
		/*

		    bit     description

		    0       PC0 (LSB) - RAM0
		    1       PC1 (LSB) - RAM1
		    2       PC2 (LSB) - RAM2
		    3       PC3 (LSB) - RAM3
		    4       PC0 (MSB) - RAM4
		    5       PC1 (MSB) - Undefined
		    6       PC2 (MSB) - Undefined
		    7       PC3 (MSB) - Undefined

		*/

		m_status2 = data;
		break;

	case 6:
		machine_reset();
		break;
	}
}



//**************************************************************************
//  MOUSE
//**************************************************************************

//-------------------------------------------------
//  mouse_r -
//-------------------------------------------------

READ8_MEMBER( pc1512_state::mouse_r )
{
	UINT8 data = 0;

	switch (offset)
	{
	case 0:
		data = m_mouse_x;
		break;

	case 2:
		data = m_mouse_y;
		break;
	}

	return data;
}


//-------------------------------------------------
//  mouse_w -
//-------------------------------------------------

WRITE8_MEMBER( pc1512_state::mouse_w )
{
	switch (offset)
	{
	case 0:
		m_mouse_x = 0;
		break;

	case 2:
		m_mouse_y = 0;
		break;
	}
}



//**************************************************************************
//  DIRECT MEMORY ACCESS
//**************************************************************************

//-------------------------------------------------
//  dma_page_w -
//-------------------------------------------------

WRITE8_MEMBER( pc1512_state::dma_page_w )
{
	/*

	    bit     description

	    0       Address bit A16
	    1       Address bit A17
	    2       Address bit A18
	    3       Address bit A19
	    4
	    5
	    6
	    7

	*/

	switch (offset)
	{
	case 1:
		m_dma_page[2] = data & 0x0f;
		break;

	case 2:
		m_dma_page[3] = data & 0x0f;
		break;

	case 3:
		m_dma_page[0] = m_dma_page[1] = data & 0x0f;
		break;
	}
}



//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  nmi_mask_w -
//-------------------------------------------------

WRITE8_MEMBER( pc1512_state::nmi_mask_w )
{
	m_nmi_enable = BIT(data, 7);
}



//**************************************************************************
//  PRINTER
//**************************************************************************

//-------------------------------------------------
//  printer_r -
//-------------------------------------------------

READ8_MEMBER( pc1512_state::printer_r )
{
	UINT8 data = 0;

	switch (offset)
	{
	case 0:
		data = m_printer_data;
		break;

	case 1:
		/*

		    bit     description

		    0       LK1 fitted
		    1       LK2 fitted
		    2       LK3 fitted
		    3       printer error
		    4       printer selected
		    5       paper out
		    6       printer acknowledge
		    7       printer busy

		*/

		data |= m_lk->read() & 0x07;

		data |= m_centronics_fault << 3;
		data |= m_centronics_select << 4;
		data |= m_centronics_perror << 5;
		data |= m_centronics_ack << 6;
		data |= m_centronics_busy << 7;
		break;

	case 2:
		/*

		    bit     description

		    0       Data Strobe
		    1       Select Auto Feed
		    2       Reset Printer
		    3       Select Printer
		    4       Enable Int on ACK
		    5       1
		    6
		    7

		*/

		data = m_printer_control | 0x20;
		break;
	}

	return data;
}


//-------------------------------------------------
//  printer_r -
//-------------------------------------------------

READ8_MEMBER( pc1640_state::printer_r )
{
	UINT8 data = 0;

	switch (offset)
	{
	case 2:
		/*

		    bit     description

		    0       Data Strobe
		    1       Select Auto Feed
		    2       Reset Printer
		    3       Select Printer
		    4       Enable Int on ACK
		    5       OPT
		    6       SW6
		    7       SW7

		*/

		/*
		Bit D5 is the option (OPT) bit and can return one of three different pieces of
		information. Although not documented as such on the PC1512, Bit D5 was always a
		"1", however on the PC1640 it will always be a zero if immediately prior to the
		read of channel 037Ah the software performs an I/O read of an I/O channel
		implemented on the PC1512 main board, having address line A7 high (for example,
		the CGA channels). This is a simple test for software to detect whether it is
		running on a PC1512 or a PC1640. A PC1512 will give a 1, whereas a PC1640 will
		give a 0.

		In addition to being a test of machine type the OPT bit, D5, can also reflect
		the state of either SW9 or SW10. The OPT bit will reflect the state of switch
		SW9 by an I/O read operation to an I/O channel not implemented on the main
		board and having address lines A14 and A7 both low (for example channel 0278h)
		immediately prior to the reading of channel 037Ah. The OPT bit is set to the
		state of switch SW10 by an I/O read operation to an I/O channel not implemented
		on the main board having address lines A14 high and A7 low (for example channel
		4278h). Software testing OPT bit should disable interrupts before the initial
		(dummy) channel read and the I/O read of channel 037A in order to avoid
		additional (interrupt based) I/O operations between the setting and the testing
		of the information read back in the OPT bit. For switches SW9 and SW10, a logic
		"1" is returned when the switch is on the "on" position and a logic "0" if the
		switch is in the "off" position.
		*/
		data = m_printer_control;
		data |= m_opt << 5;
		data |= (m_sw->read() & 0x60) << 1;
		break;

	default:
		data = pc1512_state::printer_r(space, offset);
		break;
	}

	return data;
}


//-------------------------------------------------
//  printer_w -
//-------------------------------------------------

WRITE8_MEMBER( pc1512_state::printer_w )
{
	switch (offset)
	{
	case 0:
		m_printer_data = data;
		m_cent_data_out->write(space, 0, data);
		break;

	case 2:
		/*

		    bit     description

		    0       Data Strobe
		    1       Select Auto Feed
		    2       Reset Printer
		    3       Select Printer
		    4       Enable Int on ACK
		    5
		    6
		    7

		*/

		m_printer_control = data & 0x1f;

		m_centronics->write_strobe(BIT(data, 0));
		m_centronics->write_autofd(BIT(data, 1));
		m_centronics->write_init(BIT(data, 2));

		m_ack_int_enable = BIT(data, 4);
		update_ack();
		break;
	}
}


//**************************************************************************
//  PC1640 I/O ACCESS
//**************************************************************************

//-------------------------------------------------
//  io_r -
//-------------------------------------------------

READ8_MEMBER( pc1640_state::io_r )
{
	UINT8 data = 0;
	offs_t addr = offset & 0x3ff;
	bool decoded = false;

	if      (                 addr <= 0x00f) { data = m_dmac->read(space, offset & 0x0f); decoded = true; }
	else if (addr >= 0x020 && addr <= 0x021) { data = m_pic->read(space, offset & 0x01); decoded = true; }
	else if (addr >= 0x040 && addr <= 0x043) { data = m_pit->read(space, offset & 0x03); decoded = true; }
	else if (addr >= 0x060 && addr <= 0x06f) { data = system_r(space, offset & 0x0f); decoded = true; }
	else if (addr >= 0x070 && addr <= 0x073) { data = m_rtc->read(space, offset & 0x01); decoded = true; }
	else if (addr >= 0x078 && addr <= 0x07f) { data = mouse_r(space, offset & 0x07); decoded = true; }
	else if (addr >= 0x378 && addr <= 0x37b) { data = printer_r(space, offset & 0x03); decoded = true; }
	else if (addr >= 0x3b0 && addr <= 0x3df) { decoded = true; }
	else if (addr >= 0x3f4 && addr <= 0x3f4) { data = m_fdc->fdc->msr_r(space, offset & 0x01); decoded = true; }
	else if (addr >= 0x3f5 && addr <= 0x3f5) { data = m_fdc->fdc->fifo_r(space, offset & 0x01); decoded = true; }
	else if (addr >= 0x3f8 && addr <= 0x3ff) { data = m_uart->ins8250_r(space, offset & 0x07); decoded = true; }

	if (decoded)
	{
		if (BIT(offset, 7))
		{
			m_opt = 0;
			//logerror("OPT 0\n");
		}
	}
	else if (!BIT(offset, 7))
	{
		UINT16 sw = m_sw->read();

		if (!BIT(offset, 14))
		{
			m_opt = BIT(sw, 8);
			logerror("OPT SW9 %u\n", m_opt);
		}
		else
		{
			m_opt = BIT(sw, 9);
			logerror("OPT SW10 %u\n", m_opt);
		}
	}

	return data;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( pc1512_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( pc1512_mem, AS_PROGRAM, 16, pc1512_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAM
	AM_RANGE(0xb8000, 0xbbfff) AM_READWRITE8(video_ram_r, video_ram_w, 0xffff)
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION(I8086_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( pc1512_io )
//-------------------------------------------------

static ADDRESS_MAP_START( pc1512_io, AS_IO, 16, pc1512_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_RANGE(0x000, 0x00f) AM_DEVREADWRITE8(I8237A5_TAG, am9517a_device, read, write, 0xffff)
	AM_RANGE(0x020, 0x021) AM_DEVREADWRITE8(I8259A2_TAG, pic8259_device, read, write, 0xffff)
	AM_RANGE(0x040, 0x043) AM_DEVREADWRITE8(I8253_TAG, pit8253_device, read, write, 0xffff)
	AM_RANGE(0x060, 0x06f) AM_READWRITE8(system_r, system_w, 0xffff)
	AM_RANGE(0x070, 0x071) AM_MIRROR(0x02) AM_DEVREADWRITE8(MC146818_TAG, mc146818_device, read, write, 0xffff)
	AM_RANGE(0x078, 0x07f) AM_READWRITE8(mouse_r, mouse_w, 0xffff)
	AM_RANGE(0x080, 0x083) AM_WRITE8(dma_page_w, 0xffff)
	AM_RANGE(0x0a0, 0x0a1) AM_WRITE8(nmi_mask_w, 0xff00)
	AM_RANGE(0x378, 0x37b) AM_READWRITE8(printer_r, printer_w, 0xffff)
	AM_RANGE(0x3d0, 0x3df) AM_READWRITE8(vdu_r, vdu_w, 0xffff)
	AM_RANGE(0x3f0, 0x3f7) AM_DEVICE8(PC_FDC_XT_TAG, pc_fdc_xt_device, map, 0xffff)
	AM_RANGE(0x3f8, 0x3ff) AM_DEVREADWRITE8(INS8250_TAG, ins8250_device, ins8250_r, ins8250_w, 0xffff)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( pc1640_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( pc1640_mem, AS_PROGRAM, 16, pc1640_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAM
	AM_RANGE(0xf0000, 0xf3fff) AM_MIRROR(0xc000) AM_ROM AM_REGION(I8086_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( pc1640_io )
//-------------------------------------------------

static ADDRESS_MAP_START( pc1640_io, AS_IO, 16, pc1640_state )
	AM_RANGE(0x0000, 0xffff) AM_READ8(io_r, 0xffff)

	AM_RANGE(0x000, 0x00f) AM_DEVWRITE8(I8237A5_TAG, am9517a_device, write, 0xffff)
	AM_RANGE(0x020, 0x021) AM_DEVWRITE8(I8259A2_TAG, pic8259_device, write, 0xffff)
	AM_RANGE(0x040, 0x043) AM_DEVWRITE8(I8253_TAG, pit8253_device, write, 0xffff)
	AM_RANGE(0x060, 0x06f) AM_WRITE8(system_w, 0xffff)
	AM_RANGE(0x070, 0x071) AM_MIRROR(0x02) AM_DEVWRITE8(MC146818_TAG, mc146818_device, write, 0xffff)
	AM_RANGE(0x078, 0x07f) AM_WRITE8(mouse_w, 0xffff)
	AM_RANGE(0x080, 0x083) AM_WRITE8(dma_page_w, 0xffff)
	AM_RANGE(0x0a0, 0x0a1) AM_WRITE8(nmi_mask_w, 0xff00)
	AM_RANGE(0x378, 0x37b) AM_WRITE8(printer_w, 0xffff)
	AM_RANGE(0x3f2, 0x3f3) AM_DEVWRITE8(PC_FDC_XT_TAG, pc_fdc_xt_device, dor_w, 0x00ff)
	AM_RANGE(0x3f4, 0x3f5) AM_DEVWRITE8(PC_FDC_XT_TAG ":upd765", upd765_family_device, fifo_w, 0xff00)
	AM_RANGE(0x3f8, 0x3ff) AM_DEVWRITE8(INS8250_TAG, ins8250_device, ins8250_w, 0xffff)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( mouse_button_1_changed )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( pc1512_state::mouse_button_1_changed )
{
	m_kb->m1_w(newval);
}


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( mouse_button_2_changed )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( pc1512_state::mouse_button_2_changed )
{
	m_kb->m2_w(newval);
}


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( mouse_x_changed )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( pc1512_state::mouse_x_changed )
{
	if (newval > oldval)
		m_mouse_x++;
	else
		m_mouse_x--;
}


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( mouse_y_changed )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( pc1512_state::mouse_y_changed )
{
	if (newval > oldval)
		m_mouse_y--;
	else
		m_mouse_y++;
}


//-------------------------------------------------
//  INPUT_PORTS( mouse )
//-------------------------------------------------

static INPUT_PORTS_START( mouse )
	PORT_START("MOUSEB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Left Mouse Button") PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, pc1512_state, mouse_button_1_changed, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Right Mouse Button") PORT_CODE(MOUSECODE_BUTTON2) PORT_CHANGED_MEMBER(DEVICE_SELF, pc1512_state, mouse_button_2_changed, 0)

	PORT_START("MOUSEX")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, pc1512_state, mouse_x_changed, 0)

	PORT_START("MOUSEY")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, pc1512_state, mouse_y_changed, 0)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( pc1512 )
//-------------------------------------------------

static INPUT_PORTS_START( pc1512 )
	PORT_INCLUDE( mouse )

	PORT_START("LK")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x07, DEF_STR( English ) )
	PORT_DIPSETTING(    0x06, DEF_STR( German ) )
	PORT_DIPSETTING(    0x05, DEF_STR( French ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x03, "Danish" )
	PORT_DIPSETTING(    0x02, "Swedish" )
	PORT_DIPSETTING(    0x01, DEF_STR( Italian ) )
	PORT_DIPSETTING(    0x00, "Diagnostic Mode" )
	PORT_DIPNAME( 0x08, 0x08, "Memory Size")
	PORT_DIPSETTING( 0x08, "512 KB" )
	PORT_DIPSETTING( 0x00, "640 KB" )
	PORT_DIPNAME( 0x10, 0x10, "ROM Size")
	PORT_DIPSETTING( 0x10, "16 KB" )
	PORT_DIPSETTING( 0x00, "32 KB" )
	PORT_DIPNAME( 0x60, 0x60, "Character Set")
	PORT_DIPSETTING( 0x60, "Default (Codepage 437)" )
	PORT_DIPSETTING( 0x40, "Portugese (Codepage 865)" )
	PORT_DIPSETTING( 0x20, "Norwegian (Codepage 860)" )
	PORT_DIPSETTING( 0x00, "Greek")
	PORT_DIPNAME( 0x80, 0x80, "Floppy Ready Line")
	PORT_DIPSETTING( 0x80, "Connected" )
	PORT_DIPSETTING( 0x00, "Not connected" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( pc1640 )
//-------------------------------------------------

static INPUT_PORTS_START( pc1640 )
	PORT_INCLUDE( mouse )

	PORT_START("LK")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x07, DEF_STR( English ) )
	PORT_DIPSETTING(    0x06, DEF_STR( German ) )
	PORT_DIPSETTING(    0x05, DEF_STR( French ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x03, "Danish" )
	PORT_DIPSETTING(    0x02, "Swedish" )
	PORT_DIPSETTING(    0x01, DEF_STR( Italian ) )
	PORT_DIPSETTING(    0x00, "Diagnostic Mode" )

	PORT_START("SW")
	PORT_DIPNAME( 0x0f, 0x09, "Initial Display Mode" ) PORT_DIPLOCATION("SW:1,2,3,4") PORT_CONDITION("SW", 0x200, EQUALS, 0x200)
	PORT_DIPSETTING(    0x0b, "Internal MD, External CGA80" )
	PORT_DIPSETTING(    0x0a, "Internal MD, External CGA40" )
	PORT_DIPSETTING(    0x09, "Internal ECD350, External MDA/HERC" )
	PORT_DIPSETTING(    0x08, "Internal ECD200, External MDA/HERC" )
	PORT_DIPSETTING(    0x07, "Internal CD80, External MDA/HERC" )
	PORT_DIPSETTING(    0x06, "Internal CD40, External MDA/HERC" )
	PORT_DIPSETTING(    0x05, "External CGA80, Internal MD" )
	PORT_DIPSETTING(    0x04, "External CGA40, Internal MD" )
	PORT_DIPSETTING(    0x03, "External MDA/HERC, Internal ECD350" )
	PORT_DIPSETTING(    0x02, "External MDA/HERC, Internal ECD200" )
	PORT_DIPSETTING(    0x01, "External MDA/HERC, Internal CD80" )
	PORT_DIPSETTING(    0x00, "External MDA/HERC, Internal CD40" )
	PORT_DIPNAME( 0x10, 0x10, "MC6845 Mode" ) PORT_DIPLOCATION("SW:5") PORT_CONDITION("SW", 0x200, EQUALS, 0x200)
	PORT_DIPSETTING(    0x10, "EGA" )
	PORT_DIPSETTING(    0x00, "CGA/MDA/HERC" )
	PORT_DIPNAME( 0x60, 0x00, "Font" ) PORT_DIPLOCATION("SW:6,7") PORT_CONDITION("SW", 0x300, EQUALS, 0x300)
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x60, "Danish" )
	PORT_DIPSETTING(    0x40, "Portuguese" )
	PORT_DIPSETTING(    0x20, "Greek" )
	PORT_DIPNAME( 0x60, 0x60, "Default Display Mode" ) PORT_DIPLOCATION("SW:6,7") PORT_CONDITION("SW", 0x200, EQUALS, 0x000)
	PORT_DIPSETTING(    0x60, "External EGA" )
	PORT_DIPSETTING(    0x40, "External CGA in 40 Column Mode" )
	PORT_DIPSETTING(    0x20, "External CGA in 80 Column Mode" )
	PORT_DIPSETTING(    0x00, "External Monochrome Adapter" )
	PORT_DIPNAME( 0x80, 0x00, "Monitor" ) PORT_DIPLOCATION("SW:8") PORT_CONDITION("SW", 0x200, EQUALS, 0x200)
	PORT_DIPSETTING(    0x80, "CD (Standard RGB)" )
	PORT_DIPSETTING(    0x00, "ECD (Enhanced RGB)" )
	PORT_DIPNAME( 0x100, 0x100, "Foreign Fonts" ) PORT_DIPLOCATION("SW:9") PORT_CONDITION("SW", 0x200, EQUALS, 0x200)
	PORT_DIPSETTING(     0x100, "Enabled" )
	PORT_DIPSETTING(     0x000, "Disabled" )
	PORT_DIPNAME( 0x200, 0x200, "Internal Graphics Adapter" ) PORT_DIPLOCATION("SW:10")
	PORT_DIPSETTING(     0x200, "Enabled" )
	PORT_DIPSETTING(     0x000, "Disabled" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  PC1512_KEYBOARD_INTERFACE( kb_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( pc1512_state::kbdata_w )
{
	m_kbdata = state;
}

WRITE_LINE_MEMBER( pc1512_state::kbclk_w )
{
	if (!BIT(m_port61, 7) && m_kbclk && !state)
	{
		m_kbd <<= 1;
		m_kbd |= m_kbdata;
		m_kb_bits++;

		if (m_kb_bits == 8)
		{
			m_kb->data_w(0);
			m_pic->ir1_w(ASSERT_LINE);
		}
	}

	m_kbclk = state;
}


//-------------------------------------------------
//  I8237_INTERFACE( dmac_intf )
//-------------------------------------------------

void pc1512_state::update_fdc_tc()
{
	if (m_nden)
		m_fdc->tc_w(m_neop);
	else
		m_fdc->tc_w(false);
}

WRITE_LINE_MEMBER( pc1512_state::hrq_w )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	m_dmac->hack_w(state);
}

WRITE_LINE_MEMBER( pc1512_state::eop_w )
{
	if (m_dma_channel == 2)
	{
		m_neop = !state;
		update_fdc_tc();
	}
}

READ8_MEMBER( pc1512_state::memr_r )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = m_dma_page[m_dma_channel] << 16;

	return program.read_byte(page_offset + offset);
}

WRITE8_MEMBER( pc1512_state::memw_w )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = m_dma_page[m_dma_channel] << 16;

	program.write_byte(page_offset + offset, data);
}

READ8_MEMBER( pc1512_state::ior1_r )
{
	return m_bus->dack_r(1);
}

READ8_MEMBER( pc1512_state::ior2_r )
{
	if (m_nden)
		return m_fdc->dma_r();
	else
		return m_bus->dack_r(2);
}

READ8_MEMBER( pc1512_state::ior3_r )
{
	return m_bus->dack_r(3);
}

WRITE8_MEMBER( pc1512_state::iow0_w )
{
	m_dreq0 = 0;
	m_dmac->dreq0_w(m_dreq0);
}

WRITE8_MEMBER( pc1512_state::iow1_w )
{
	m_bus->dack_w(1, data);
}

WRITE8_MEMBER( pc1512_state::iow2_w )
{
	if (m_nden)
		m_fdc->dma_w(data);
	else
		m_bus->dack_w(2, data);
}

WRITE8_MEMBER( pc1512_state::iow3_w )
{
	m_bus->dack_w(3, data);
}

WRITE_LINE_MEMBER( pc1512_state::dack0_w )
{
	if (!state) m_dma_channel = 0;
}

WRITE_LINE_MEMBER( pc1512_state::dack1_w )
{
	if (!state) m_dma_channel = 1;
}

WRITE_LINE_MEMBER( pc1512_state::dack2_w )
{
	if (!state) m_dma_channel = 2;
}

WRITE_LINE_MEMBER( pc1512_state::dack3_w )
{
	if (!state) m_dma_channel = 3;
}

//-------------------------------------------------
//  pit8253_config pit_intf
//-------------------------------------------------

void pc1512_state::update_speaker()
{
	m_speaker->level_w(m_speaker_drive & m_pit2);
}

WRITE_LINE_MEMBER( pc1512_state::pit1_w )
{
	if (!m_pit1 && state && !m_dreq0)
	{
		m_dreq0 = 1;
		m_dmac->dreq0_w(m_dreq0);
	}

	m_pit1 = state;
}

WRITE_LINE_MEMBER( pc1512_state::pit2_w )
{
	m_pit2 = state;
	update_speaker();
}

//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

void pc1512_state::update_fdc_int()
{
	if (m_nden)
		m_pic->ir6_w(m_dint);
	else
		m_pic->ir6_w(CLEAR_LINE);
}

void pc1512_state::update_fdc_drq()
{
	if (m_nden)
		m_dmac->dreq2_w(m_ddrq);
	else
		m_dmac->dreq2_w(0);
}

WRITE_LINE_MEMBER( pc1512_state::fdc_int_w )
{
	m_dint = state;
	update_fdc_int();
}

WRITE_LINE_MEMBER( pc1512_state::fdc_drq_w )
{
	m_ddrq = state;
	update_fdc_drq();
}

//-------------------------------------------------
//  centronics_interface centronics_intf
//-------------------------------------------------

void pc1512_state::update_ack()
{
	if (m_ack_int_enable)
		m_pic->ir7_w(m_centronics_ack);
	else
		m_pic->ir7_w(CLEAR_LINE);
}

WRITE_LINE_MEMBER( pc1512_state::write_centronics_ack )
{
	m_centronics_ack = state;
	update_ack();
}

WRITE_LINE_MEMBER( pc1512_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER( pc1512_state::write_centronics_perror )
{
	m_centronics_perror = state;
}

WRITE_LINE_MEMBER( pc1512_state::write_centronics_select )
{
	m_centronics_select = state;
}

WRITE_LINE_MEMBER( pc1512_state::write_centronics_fault )
{
	m_centronics_fault = state;
}


//-------------------------------------------------
//  isa8bus_interface isabus_intf
//-------------------------------------------------

SLOT_INTERFACE_START( pc1640_isa8_cards )
	SLOT_INTERFACE_INTERNAL("iga", ISA8_PC1640_IGA)
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( pc1512_state::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( pc1512_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD ) // Tandon TM65-2L
SLOT_INTERFACE_END



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( pc1512 )
//-------------------------------------------------

void pc1512_state::machine_start()
{
	// set RAM size
	size_t ram_size = m_ram->size();

	if (ram_size < 640 * 1024)
	{
		address_space &program = m_maincpu->space(AS_PROGRAM);
		program.unmap_readwrite(ram_size, 0x9ffff);
	}

	// state saving
	save_item(NAME(m_pit1));
	save_item(NAME(m_pit2));
	save_item(NAME(m_status1));
	save_item(NAME(m_status2));
	save_item(NAME(m_port61));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_kbd));
	save_item(NAME(m_kb_bits));
	save_item(NAME(m_kbclk));
	save_item(NAME(m_kbdata));
	save_item(NAME(m_mouse_x));
	save_item(NAME(m_mouse_y));
	save_item(NAME(m_dma_page));
	save_item(NAME(m_dma_channel));
	save_item(NAME(m_dreq0));
	save_item(NAME(m_nden));
	save_item(NAME(m_dint));
	save_item(NAME(m_ddrq));
	save_item(NAME(m_neop));
	save_item(NAME(m_ack_int_enable));
	save_item(NAME(m_centronics_ack));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_perror));
	save_item(NAME(m_centronics_select));
	save_item(NAME(m_centronics_fault));
	save_item(NAME(m_printer_data));
	save_item(NAME(m_printer_control));
	save_item(NAME(m_toggle));
	save_item(NAME(m_lpen));
	save_item(NAME(m_blink));
	save_item(NAME(m_cursor));
	save_item(NAME(m_blink_ctr));
	save_item(NAME(m_vdu_mode));
	save_item(NAME(m_vdu_color));
	save_item(NAME(m_vdu_plane));
	save_item(NAME(m_vdu_rdsel));
	save_item(NAME(m_vdu_border));
	save_item(NAME(m_speaker_drive));
}


void pc1512_state::machine_reset()
{
	m_nmi_enable = 0;
	m_toggle = 0;
	m_kb_bits = 0;
	m_pit2 = 1;

	m_lpen = 0;
	m_blink = 0;
	m_cursor = 0;
	m_blink_ctr = 0;
	m_vdu_mode = 0;
	m_vdu_color = 0;
	m_vdu_rdsel = 0;
	m_vdu_plane = 0x0f;
	m_vdu_border = 0;

	m_kb->clock_w(0);
}


//-------------------------------------------------
//  MACHINE_START( pc1640 )
//-------------------------------------------------

void pc1640_state::machine_start()
{
	// state saving
	save_item(NAME(m_pit1));
	save_item(NAME(m_pit2));
	save_item(NAME(m_status1));
	save_item(NAME(m_status2));
	save_item(NAME(m_port61));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_kbd));
	save_item(NAME(m_kb_bits));
	save_item(NAME(m_kbclk));
	save_item(NAME(m_kbdata));
	save_item(NAME(m_mouse_x));
	save_item(NAME(m_mouse_y));
	save_item(NAME(m_dma_page));
	save_item(NAME(m_dma_channel));
	save_item(NAME(m_dreq0));
	save_item(NAME(m_nden));
	save_item(NAME(m_dint));
	save_item(NAME(m_ddrq));
	save_item(NAME(m_neop));
	save_item(NAME(m_ack_int_enable));
	save_item(NAME(m_centronics_ack));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_perror));
	save_item(NAME(m_centronics_select));
	save_item(NAME(m_centronics_fault));
	save_item(NAME(m_printer_data));
	save_item(NAME(m_printer_control));
	save_item(NAME(m_speaker_drive));
	save_item(NAME(m_opt));
}


void pc1640_state::machine_reset()
{
	m_nmi_enable = 0;
	m_kb_bits = 0;
	m_kb->clock_w(0);
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( pc1512 )
//-------------------------------------------------

static MACHINE_CONFIG_START( pc1512, pc1512_state )
	MCFG_CPU_ADD(I8086_TAG, I8086, XTAL_24MHz/3)
	MCFG_CPU_PROGRAM_MAP(pc1512_mem)
	MCFG_CPU_IO_MAP(pc1512_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(I8259A2_TAG, pic8259_device, inta_cb)

	// video
	MCFG_FRAGMENT_ADD(pc1512_video)

	// sound
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	// devices
	MCFG_DEVICE_ADD(PC1512_KEYBOARD_TAG, PC1512_KEYBOARD, 0)
	MCFG_PC1512_KEYBOARD_CLOCK_CALLBACK(WRITELINE(pc1512_state, kbclk_w))
	MCFG_PC1512_KEYBOARD_DATA_CALLBACK(WRITELINE(pc1512_state, kbdata_w))
	MCFG_DEVICE_ADD(I8237A5_TAG, AM9517A, XTAL_24MHz/6)
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(pc1512_state, hrq_w))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(pc1512_state, eop_w))
	MCFG_I8237_IN_MEMR_CB(READ8(pc1512_state, memr_r))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(pc1512_state, memw_w))
	MCFG_I8237_IN_IOR_1_CB(READ8(pc1512_state, ior1_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(pc1512_state, ior2_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(pc1512_state, ior3_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(pc1512_state, iow0_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(pc1512_state, iow1_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(pc1512_state, iow2_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(pc1512_state, iow3_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(pc1512_state, dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(pc1512_state, dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(pc1512_state, dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(pc1512_state, dack3_w))
	MCFG_PIC8259_ADD(I8259A2_TAG, INPUTLINE(I8086_TAG, INPUT_LINE_IRQ0), VCC, NULL)

	MCFG_DEVICE_ADD(I8253_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_28_63636MHz/24)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(XTAL_28_63636MHz/24)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(pc1512_state, pit1_w))
	MCFG_PIT8253_CLK2(XTAL_28_63636MHz/24)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(pc1512_state, pit2_w))

	MCFG_MC146818_ADD(MC146818_TAG, XTAL_32_768kHz)
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir2_w))
	MCFG_PC_FDC_XT_ADD(PC_FDC_XT_TAG)
	MCFG_PC_FDC_INTRQ_CALLBACK(WRITELINE(pc1512_state, fdc_int_w))
	MCFG_PC_FDC_DRQ_CALLBACK(WRITELINE(pc1512_state, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD(PC_FDC_XT_TAG ":0", pc1512_floppies, "525dd", pc1512_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(PC_FDC_XT_TAG ":1", pc1512_floppies, NULL,    pc1512_state::floppy_formats)
	MCFG_DEVICE_ADD(INS8250_TAG, INS8250, XTAL_1_8432MHz)
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir4_w))

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(pc1512_state, write_centronics_ack))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(pc1512_state, write_centronics_busy))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(pc1512_state, write_centronics_perror))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(pc1512_state, write_centronics_select))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(pc1512_state, write_centronics_fault))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, cts_w))

	// ISA8 bus
	MCFG_DEVICE_ADD(ISA_BUS_TAG, ISA8, 0)
	MCFG_ISA8_CPU(":" I8086_TAG)
	MCFG_ISA_OUT_IRQ2_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir2_w))
	MCFG_ISA_OUT_IRQ3_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ4_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ5_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ6_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ7_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir7_w))
	MCFG_ISA_OUT_DRQ1_CB(DEVWRITELINE(I8237A5_TAG, am9517a_device, dreq1_w))
	MCFG_ISA_OUT_DRQ2_CB(DEVWRITELINE(I8237A5_TAG, am9517a_device, dreq2_w))
	MCFG_ISA_OUT_DRQ3_CB(DEVWRITELINE(I8237A5_TAG, am9517a_device, dreq3_w))
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa1", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa2", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa3", pc_isa8_cards, NULL, false)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
	MCFG_RAM_EXTRA_OPTIONS("544K,576K,608K,640K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "pc1512")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pc1512dd )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pc1512dd, pc1512 )
	MCFG_DEVICE_MODIFY(PC_FDC_XT_TAG ":1")
	MCFG_SLOT_DEFAULT_OPTION("525dd")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pc1512hd )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pc1512hd, pc1512 )
	MCFG_DEVICE_MODIFY("isa1")
	MCFG_SLOT_DEFAULT_OPTION("wdxt_gen")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pc1640 )
//-------------------------------------------------

static MACHINE_CONFIG_START( pc1640, pc1640_state )
	MCFG_CPU_ADD(I8086_TAG, I8086, XTAL_24MHz/3)
	MCFG_CPU_PROGRAM_MAP(pc1640_mem)
	MCFG_CPU_IO_MAP(pc1640_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(I8259A2_TAG, pic8259_device, inta_cb)

	// sound
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	// devices
	MCFG_DEVICE_ADD(PC1512_KEYBOARD_TAG, PC1512_KEYBOARD, 0)
	MCFG_PC1512_KEYBOARD_CLOCK_CALLBACK(WRITELINE(pc1512_state, kbclk_w))
	MCFG_PC1512_KEYBOARD_DATA_CALLBACK(WRITELINE(pc1512_state, kbdata_w))
	MCFG_DEVICE_ADD(I8237A5_TAG, AM9517A, XTAL_24MHz/6)
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(pc1512_state, hrq_w))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(pc1512_state, eop_w))
	MCFG_I8237_IN_MEMR_CB(READ8(pc1512_state, memr_r))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(pc1512_state, memw_w))
	MCFG_I8237_IN_IOR_1_CB(READ8(pc1512_state, ior1_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(pc1512_state, ior2_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(pc1512_state, ior3_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(pc1512_state, iow0_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(pc1512_state, iow1_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(pc1512_state, iow2_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(pc1512_state, iow3_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(pc1512_state, dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(pc1512_state, dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(pc1512_state, dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(pc1512_state, dack3_w))
	MCFG_PIC8259_ADD(I8259A2_TAG, INPUTLINE(I8086_TAG, INPUT_LINE_IRQ0), VCC, NULL)

	MCFG_DEVICE_ADD(I8253_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_28_63636MHz/24)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(XTAL_28_63636MHz/24)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(pc1512_state, pit1_w))
	MCFG_PIT8253_CLK2(XTAL_28_63636MHz/24)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(pc1512_state, pit2_w))

	MCFG_MC146818_ADD(MC146818_TAG, XTAL_32_768kHz)
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir2_w))
	MCFG_PC_FDC_XT_ADD(PC_FDC_XT_TAG)
	MCFG_PC_FDC_INTRQ_CALLBACK(WRITELINE(pc1512_state, fdc_int_w))
	MCFG_PC_FDC_DRQ_CALLBACK(WRITELINE(pc1512_state, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD(PC_FDC_XT_TAG ":0", pc1512_floppies, "525dd", pc1512_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(PC_FDC_XT_TAG ":1", pc1512_floppies, NULL,    pc1512_state::floppy_formats)
	MCFG_DEVICE_ADD(INS8250_TAG, INS8250, XTAL_1_8432MHz)
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir4_w))


	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(pc1512_state, write_centronics_ack))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(pc1512_state, write_centronics_busy))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(pc1512_state, write_centronics_perror))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(pc1512_state, write_centronics_select))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(pc1512_state, write_centronics_fault))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, cts_w))

	// ISA8 bus
	MCFG_DEVICE_ADD(ISA_BUS_TAG, ISA8, 0)
	MCFG_ISA8_CPU(":" I8086_TAG)
	MCFG_ISA_OUT_IRQ2_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir2_w))
	MCFG_ISA_OUT_IRQ3_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ4_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ5_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ6_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ7_CB(DEVWRITELINE(I8259A2_TAG, pic8259_device, ir7_w))
	MCFG_ISA_OUT_DRQ1_CB(DEVWRITELINE(I8237A5_TAG, am9517a_device, dreq1_w))
	MCFG_ISA_OUT_DRQ2_CB(DEVWRITELINE(I8237A5_TAG, am9517a_device, dreq2_w))
	MCFG_ISA_OUT_DRQ3_CB(DEVWRITELINE(I8237A5_TAG, am9517a_device, dreq3_w))
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa1", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa2", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa3", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa4", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa5", pc1640_isa8_cards, "iga", false)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "pc1640")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pc1640dd )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pc1640dd, pc1640 )
	MCFG_DEVICE_MODIFY(PC_FDC_XT_TAG ":1")
	MCFG_SLOT_DEFAULT_OPTION("525dd")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pc1640hd )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pc1640hd, pc1640 )
	MCFG_DEVICE_MODIFY("isa1")
	MCFG_SLOT_DEFAULT_OPTION("wdxt_gen")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( pc1512 )
//-------------------------------------------------

ROM_START( pc1512 )
	ROM_REGION16_LE( 0x4000, I8086_TAG, 0)
	ROM_SYSTEM_BIOS( 0, "v1", "Version 1" )
	ROMX_LOAD( "40044.ic132", 0x0000, 0x2000, CRC(f72f1582) SHA1(7781d4717917262805d514b331ba113b1e05a247), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "40043.ic129", 0x0001, 0x2000, CRC(668fcc94) SHA1(74002f5cc542df442eec9e2e7a18db3598d8c482), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v2", "Version 2" )
	ROMX_LOAD( "40044v2.ic132", 0x0000, 0x2000, CRC(1aec54fa) SHA1(b12fd73cfc35a240ed6da4dcc4b6c9910be611e0), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "40043v2.ic129", 0x0001, 0x2000, CRC(d2d4d2de) SHA1(c376fd1ad23025081ae16c7949e88eea7f56e1bb), ROM_SKIP(1) | ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "v3", "Version 3" )
	ROMX_LOAD( "40044-2.ic132", 0x0000, 0x2000, CRC(ea527e6e) SHA1(b77fa44767a71a0b321a88bb0a394f1125b7c220), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "40043-2.ic129", 0x0001, 0x2000, CRC(532c3854) SHA1(18a17b710f9eb079d9d7216d07807030f904ceda), ROM_SKIP(1) | ROM_BIOS(3) )

	ROM_REGION( 0x2000, AMS40041_TAG, 0 )
	ROMX_LOAD( "40045.ic127", 0x0000, 0x2000, CRC(dd5e030f) SHA1(7d858bbb2e8d6143aa67ab712edf5f753c2788a7), ROM_BIOS(1) )
	ROMX_LOAD( "40078.ic127", 0x0000, 0x2000, CRC(ae9c0d04) SHA1(bc8dc4dcedeea5bc1c04986b1f105ad93cb2ebcd), ROM_BIOS(2) )
	ROMX_LOAD( "40078.ic127", 0x0000, 0x2000, CRC(ae9c0d04) SHA1(bc8dc4dcedeea5bc1c04986b1f105ad93cb2ebcd), ROM_BIOS(3) )
ROM_END

#define rom_pc1512dd    rom_pc1512
#define rom_pc1512hd10  rom_pc1512
#define rom_pc1512hd20  rom_pc1512


//-------------------------------------------------
//  ROM( pc1640 )
//-------------------------------------------------

ROM_START( pc1640 )
	ROM_REGION16_LE( 0x4000, I8086_TAG, 0)
	ROM_SYSTEM_BIOS( 0, "8809", "Week 9/1988" )
	ROMX_LOAD( "40044-1 8809.ic132", 0x0000, 0x2000, CRC(f1c074f3) SHA1(a055ea7e933d137623c22fe24004e870653c7952), ROM_SKIP(1) | ROM_BIOS(1) ) // 8809 B
	ROMX_LOAD( "40043-1 8809.ic129", 0x0001, 0x2000, CRC(e40a1513) SHA1(447eff2057e682e51b1c7593cb6fad0e53879fa8), ROM_SKIP(1) | ROM_BIOS(1) ) // 8809 B
	ROM_SYSTEM_BIOS( 1, "8738", "Week 38/1987" )
	ROMX_LOAD( "40044 8738.ic132", 0x0000, 0x2000, CRC(43832ea7) SHA1(eea4a8836f966940a88c88de6c5cc14852545f7d), ROM_SKIP(1) | ROM_BIOS(2) ) // 8738 D F
	ROMX_LOAD( "40043 8738.ic129", 0x0001, 0x2000, CRC(768498f9) SHA1(ac48cb892417d7998d604f3b79756140c554f476), ROM_SKIP(1) | ROM_BIOS(2) ) // 8738 D F
	ROM_SYSTEM_BIOS( 2, "88xx", "Week ?/1988" )
	ROMX_LOAD( "40044 88xx.ic132", 0x0000, 0x2000, CRC(6090f782) SHA1(e21ae524d5b4d00696d293dbd4fe4d7bca22e277), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "40043 88xx.ic129", 0x0001, 0x2000, CRC(9219d0aa) SHA1(dde1a46c8f83e413d7070f1356fc91b9f595a8b6), ROM_SKIP(1) | ROM_BIOS(3) )
ROM_END

#define rom_pc1640dd    rom_pc1640
#define rom_pc1640hd20  rom_pc1640
#define rom_pc1640hd30  rom_pc1640



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT    COMPANY         FULLNAME        FLAGS
COMP( 1986, pc1512,     0,          0,      pc1512,     pc1512, driver_device,      0,      "Amstrad plc",  "PC1512 SD",    MACHINE_SUPPORTS_SAVE )
COMP( 1986, pc1512dd,   pc1512,     0,      pc1512dd,   pc1512, driver_device,      0,      "Amstrad plc",  "PC1512 DD",    MACHINE_SUPPORTS_SAVE )
COMP( 1986, pc1512hd10, pc1512,     0,      pc1512hd,   pc1512, driver_device,      0,      "Amstrad plc",  "PC1512 HD10",  MACHINE_SUPPORTS_SAVE )
COMP( 1986, pc1512hd20, pc1512,     0,      pc1512hd,   pc1512, driver_device,      0,      "Amstrad plc",  "PC1512 HD20",  MACHINE_SUPPORTS_SAVE )
COMP( 1987, pc1640,     0,          0,      pc1640,     pc1640, driver_device,      0,      "Amstrad plc",  "PC1640 SD",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1987, pc1640dd,   pc1640,     0,      pc1640dd,   pc1640, driver_device,      0,      "Amstrad plc",  "PC1640 DD",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1987, pc1640hd20, pc1640,     0,      pc1640hd,   pc1640, driver_device,      0,      "Amstrad plc",  "PC1640 HD20",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1987, pc1640hd30, pc1640,     0,      pc1640hd,   pc1640, driver_device,      0,      "Amstrad plc",  "PC1640 HD30",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
