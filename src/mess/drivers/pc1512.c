/*

    Amstrad PC1512

    http://www.seasip.info/AmstradXT
    http://stason.org/TULARC/pc/hard-drives-hdd/tandon/TM262-21MB-5-25-HH-MFM-ST506.html

*/

/*

    TODO:

    - adjust mouse speed
    - RTC should not be y2k compliant
    - V3 VDU check fails

*/

#include "includes/pc1512.h"



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
			pic8259_ir1_w(m_pic, CLEAR_LINE);
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

		pit8253_gate2_w(m_pit, BIT(data, 0));

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

		data |= m_centronics->fault_r() << 3;
		data |= m_centronics->vcc_r() << 4;
		data |= m_centronics->pe_r() << 5;
		data |= m_centronics->ack_r() << 6;
		data |= m_centronics->busy_r() << 7;
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
		m_centronics->write(space, 0, data);
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

		m_centronics->strobe_w(BIT(data, 0));
		m_centronics->autofeed_w(BIT(data, 1));
		m_centronics->init_prime_w(BIT(data, 2));

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

READ8_MEMBER( pc1640_state::io_unmapped_r )
{
	test_unmapped = true;
	return 0xff;
}


READ8_MEMBER( pc1640_state::io_r )
{
	test_unmapped = false;

	UINT8 data = space.read_byte(offset + 0x10000);

	if (!test_unmapped)
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
	AM_RANGE(0x020, 0x021) AM_DEVREADWRITE8_LEGACY(I8259A2_TAG, pic8259_r, pic8259_w, 0xffff)
	AM_RANGE(0x040, 0x043) AM_DEVREADWRITE8_LEGACY(I8253_TAG, pit8253_r, pit8253_w, 0xffff)
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
	AM_RANGE(0xa0000, 0xbffff) AM_READWRITE8(video_ram_r, video_ram_w, 0xffff)
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM AM_REGION("iga", 0)
//  AM_RANGE(0xc8000, 0xc9fff) AM_ROM AM_REGION("hdc", 0)
	AM_RANGE(0xf0000, 0xf3fff) AM_MIRROR(0xc000) AM_ROM AM_REGION(I8086_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( pc1640_io )
//-------------------------------------------------

static ADDRESS_MAP_START( pc1640_io, AS_IO, 16, pc1640_state )
	AM_RANGE(0x0000, 0xffff) AM_READ8(io_r, 0xffff)

	// Mirrored over to 10000 for indirect reads through io_r

	AM_RANGE(0x000, 0x00f) AM_MIRROR(0x10000) AM_DEVWRITE8(I8237A5_TAG, am9517a_device, write, 0xffff)
	AM_RANGE(0x020, 0x021) AM_MIRROR(0x10000) AM_DEVWRITE8_LEGACY(I8259A2_TAG, pic8259_w, 0xffff)
	AM_RANGE(0x040, 0x043) AM_MIRROR(0x10000) AM_DEVWRITE8_LEGACY(I8253_TAG, pit8253_w, 0xffff)
	AM_RANGE(0x060, 0x06f) AM_MIRROR(0x10000) AM_WRITE8(system_w, 0xffff)
	AM_RANGE(0x070, 0x071) AM_MIRROR(0x10000) AM_MIRROR(0x02) AM_DEVWRITE8(MC146818_TAG, mc146818_device, write, 0xffff)
	AM_RANGE(0x078, 0x07f) AM_MIRROR(0x10000) AM_WRITE8(mouse_w, 0xffff)
	AM_RANGE(0x080, 0x083) AM_MIRROR(0x10000) AM_WRITE8(dma_page_w, 0xffff)
	AM_RANGE(0x0a0, 0x0a1) AM_MIRROR(0x10000) AM_WRITE8(nmi_mask_w, 0xff00)
	AM_RANGE(0x378, 0x37b) AM_MIRROR(0x10000) AM_WRITE8(printer_w, 0xffff)
	AM_RANGE(0x3b0, 0x3df) AM_MIRROR(0x10000) AM_WRITE8(iga_w, 0xffff)
	AM_RANGE(0x3f0, 0x3f7) AM_MIRROR(0x10000) AM_DEVICE8(PC_FDC_XT_TAG, pc_fdc_xt_device, map, 0xffff)
	AM_RANGE(0x3f8, 0x3ff) AM_MIRROR(0x10000) AM_DEVWRITE8(INS8250_TAG, ins8250_device, ins8250_w, 0xffff)
	AM_RANGE(0x10000, 0x1ffff) AM_READ8(io_unmapped_r, 0xffff)
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
			pic8259_ir1_w(m_pic, ASSERT_LINE);
		}
	}

	m_kbclk = state;
}

static PC1512_KEYBOARD_INTERFACE( kb_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(pc1512_state, kbdata_w),
	DEVCB_DRIVER_LINE_MEMBER(pc1512_state, kbclk_w)
};


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

static I8237_INTERFACE( dmac_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(pc1512_state, hrq_w),
	DEVCB_DRIVER_LINE_MEMBER(pc1512_state, eop_w),
	DEVCB_DRIVER_MEMBER(pc1512_state, memr_r),
	DEVCB_DRIVER_MEMBER(pc1512_state, memw_w),
	{ DEVCB_NULL,
		DEVCB_DRIVER_MEMBER(pc1512_state, ior1_r),
		DEVCB_DRIVER_MEMBER(pc1512_state, ior2_r),
		DEVCB_DRIVER_MEMBER(pc1512_state, ior3_r) },
	{ DEVCB_DRIVER_MEMBER(pc1512_state, iow0_w),
		DEVCB_DRIVER_MEMBER(pc1512_state, iow1_w),
		DEVCB_DRIVER_MEMBER(pc1512_state, iow2_w),
		DEVCB_DRIVER_MEMBER(pc1512_state, iow3_w) },
	{ DEVCB_DRIVER_LINE_MEMBER(pc1512_state, dack0_w),
		DEVCB_DRIVER_LINE_MEMBER(pc1512_state, dack1_w),
		DEVCB_DRIVER_LINE_MEMBER(pc1512_state, dack2_w),
		DEVCB_DRIVER_LINE_MEMBER(pc1512_state, dack3_w) }
};


//-------------------------------------------------
//  pic8259_interface pic_intf
//-------------------------------------------------

static IRQ_CALLBACK( pc1512_irq_callback )
{
	pc1512_state *state = device->machine().driver_data<pc1512_state>();

	return pic8259_acknowledge(state->m_pic);
}

static const struct pic8259_interface pic_intf =
{
	DEVCB_CPU_INPUT_LINE(I8086_TAG, INPUT_LINE_IRQ0),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};


//-------------------------------------------------
//  pit8253_config pit_intf
//-------------------------------------------------

void pc1512_state::update_speaker()
{
	speaker_level_w(m_speaker, m_speaker_drive & m_pit2);
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

static const struct pit8253_config pit_intf =
{
	{
		{
			XTAL_28_63636MHz/24,
			DEVCB_LINE_VCC,
			DEVCB_DEVICE_LINE(I8259A2_TAG, pic8259_ir0_w)
		}, {
			XTAL_28_63636MHz/24,
			DEVCB_LINE_VCC,
			DEVCB_DRIVER_LINE_MEMBER(pc1512_state, pit1_w)
		}, {
			XTAL_28_63636MHz/24,
			DEVCB_NULL,
			DEVCB_DRIVER_LINE_MEMBER(pc1512_state, pit2_w)
		}
	}
};


//-------------------------------------------------
//  mc146818_interface rtc_intf
//-------------------------------------------------

static const struct mc146818_interface rtc_intf =
{
	DEVCB_DEVICE_LINE(I8259A2_TAG, pic8259_ir2_w)
};


//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

void pc1512_state::update_fdc_int()
{
	if (m_nden)
		pic8259_ir6_w(m_pic, m_dint);
	else
		pic8259_ir6_w(m_pic, CLEAR_LINE);
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
//  ins8250_interface uart_intf
//-------------------------------------------------

static const ins8250_interface uart_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE(I8259A2_TAG, pic8259_ir4_w),
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  centronics_interface centronics_intf
//-------------------------------------------------

void pc1512_state::update_ack()
{
	if (m_ack_int_enable)
		pic8259_ir7_w(m_pic, m_ack);
	else
		pic8259_ir7_w(m_pic, CLEAR_LINE);
}

WRITE_LINE_MEMBER( pc1512_state::ack_w )
{
	m_ack = state;
	update_ack();
}

static const centronics_interface centronics_intf =
{
	DEVCB_DRIVER_LINE_MEMBER(pc1512_state, ack_w),
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  isa8bus_interface isabus_intf
//-------------------------------------------------

static SLOT_INTERFACE_START( pc1512_isa8_cards )
	SLOT_INTERFACE("wdxt_gen", WDXT_GEN)
SLOT_INTERFACE_END

static const isa8bus_interface isabus_intf =
{
	// interrupts
	DEVCB_DEVICE_LINE(I8259A2_TAG, pic8259_ir2_w),
	DEVCB_DEVICE_LINE(I8259A2_TAG, pic8259_ir3_w),
	DEVCB_DEVICE_LINE(I8259A2_TAG, pic8259_ir4_w),
	DEVCB_DEVICE_LINE(I8259A2_TAG, pic8259_ir5_w),
	DEVCB_DEVICE_LINE(I8259A2_TAG, pic8259_ir6_w),
	DEVCB_DEVICE_LINE(I8259A2_TAG, pic8259_ir7_w),

	// dma request
	DEVCB_DEVICE_LINE_MEMBER(I8237A5_TAG, am9517a_device, dreq1_w),
	DEVCB_DEVICE_LINE_MEMBER(I8237A5_TAG, am9517a_device, dreq2_w),
	DEVCB_DEVICE_LINE_MEMBER(I8237A5_TAG, am9517a_device, dreq3_w)
};

FLOPPY_FORMATS_MEMBER( pc1512_state::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( ibmpc_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END


//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( pc1512 )
//-------------------------------------------------

void pc1512_state::machine_start()
{
	// register CPU IRQ callback
	m_maincpu->set_irq_acknowledge_callback(pc1512_irq_callback);

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
	save_item(NAME(m_ack));
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


//-------------------------------------------------
//  MACHINE_RESET( pc1512 )
//-------------------------------------------------

void pc1512_state::machine_reset()
{
	m_nmi_enable = 0;
	m_toggle = 0;
	m_kb_bits = 0;

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
	// register CPU IRQ callback
	m_maincpu->set_irq_acknowledge_callback(pc1512_irq_callback);

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
	save_item(NAME(m_ack));
	save_item(NAME(m_printer_data));
	save_item(NAME(m_printer_control));
	save_item(NAME(m_speaker_drive));
}


//-------------------------------------------------
//  MACHINE_RESET( pc1640 )
//-------------------------------------------------

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

	// video
	MCFG_FRAGMENT_ADD(pc1512_video)

	// sound
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	// devices
	MCFG_PC1512_KEYBOARD_ADD(kb_intf)
	MCFG_I8237_ADD(I8237A5_TAG, XTAL_24MHz/6, dmac_intf)
	MCFG_PIC8259_ADD(I8259A2_TAG, pic_intf)
	MCFG_PIT8253_ADD(I8253_TAG, pit_intf)
	MCFG_MC146818_IRQ_ADD(MC146818_TAG, MC146818_STANDARD, rtc_intf)
	MCFG_PC_FDC_XT_ADD(PC_FDC_XT_TAG)
	MCFG_INS8250_ADD(INS8250_TAG, uart_intf, XTAL_1_8432MHz)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, centronics_intf)
	MCFG_FLOPPY_DRIVE_ADD(PC_FDC_XT_TAG ":0", ibmpc_floppies, "525dd", 0, pc1512_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(PC_FDC_XT_TAG ":1", ibmpc_floppies, "525dd", 0, pc1512_state::floppy_formats)

	// ISA8 bus
	MCFG_ISA8_BUS_ADD(ISA_BUS_TAG, ":" I8086_TAG, isabus_intf)
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa1", pc1512_isa8_cards, NULL, NULL, false)
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa2", pc1512_isa8_cards, NULL, NULL, false)
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa3", pc1512_isa8_cards, NULL, NULL, false)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
	MCFG_RAM_EXTRA_OPTIONS("544K,576K,608K,640K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "pc1512")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pc1640 )
//-------------------------------------------------

static MACHINE_CONFIG_START( pc1640, pc1640_state )
	MCFG_CPU_ADD(I8086_TAG, I8086, XTAL_24MHz/3)
	MCFG_CPU_PROGRAM_MAP(pc1640_mem)
	MCFG_CPU_IO_MAP(pc1640_io)

	// video
	MCFG_FRAGMENT_ADD(pc1640_video)

	// sound
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	// devices
	MCFG_PC1512_KEYBOARD_ADD(kb_intf)
	MCFG_I8237_ADD(I8237A5_TAG, XTAL_24MHz/6, dmac_intf)
	MCFG_PIC8259_ADD(I8259A2_TAG, pic_intf)
	MCFG_PIT8253_ADD(I8253_TAG, pit_intf)
	MCFG_MC146818_IRQ_ADD(MC146818_TAG, MC146818_STANDARD, rtc_intf)
	MCFG_PC_FDC_XT_ADD(PC_FDC_XT_TAG)
	MCFG_INS8250_ADD(INS8250_TAG, uart_intf, XTAL_1_8432MHz)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, centronics_intf)
	MCFG_FLOPPY_DRIVE_ADD(PC_FDC_XT_TAG ":0", ibmpc_floppies, "525dd", 0, pc1512_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(PC_FDC_XT_TAG ":1", ibmpc_floppies, "525dd", 0, pc1512_state::floppy_formats)

	// ISA8 bus
	MCFG_ISA8_BUS_ADD(ISA_BUS_TAG, ":" I8086_TAG, isabus_intf)
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa1", pc1512_isa8_cards, "wdxt_gen", NULL, false)
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa2", pc1512_isa8_cards, NULL, NULL, false)
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa3", pc1512_isa8_cards, NULL, NULL, false)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "pc1640")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( pc1512 )
//-------------------------------------------------

ROM_START( pc1512 )
	ROM_REGION16_LE( 0x4000, I8086_TAG, 0)
	ROM_LOAD16_BYTE( "40044.ic132", 0x0000, 0x2000, CRC(f72f1582) SHA1(7781d4717917262805d514b331ba113b1e05a247) )
	ROM_LOAD16_BYTE( "40043.ic129", 0x0001, 0x2000, CRC(668fcc94) SHA1(74002f5cc542df442eec9e2e7a18db3598d8c482) )

	ROM_REGION( 0x2000, AMS40041_TAG, 0 )
	ROM_LOAD( "40045.ic127", 0x0000, 0x2000, CRC(dd5e030f) SHA1(7d858bbb2e8d6143aa67ab712edf5f753c2788a7) )
ROM_END


//-------------------------------------------------
//  ROM( pc1512v2 )
//-------------------------------------------------

ROM_START( pc1512v2 )
	ROM_REGION16_LE( 0x4000, I8086_TAG, 0)
	ROM_LOAD16_BYTE( "40044v2.ic132", 0x0000, 0x2000, CRC(1aec54fa) SHA1(b12fd73cfc35a240ed6da4dcc4b6c9910be611e0) )
	ROM_LOAD16_BYTE( "40043v2.ic129", 0x0001, 0x2000, CRC(d2d4d2de) SHA1(c376fd1ad23025081ae16c7949e88eea7f56e1bb) )

	ROM_REGION( 0x2000, AMS40041_TAG, 0 )
	ROM_LOAD( "40078.ic127", 0x0000, 0x2000, CRC(ae9c0d04) SHA1(bc8dc4dcedeea5bc1c04986b1f105ad93cb2ebcd) )
ROM_END


//-------------------------------------------------
//  ROM( pc1512v3 )
//-------------------------------------------------

ROM_START( pc1512v3 )
	ROM_REGION16_LE( 0x4000, I8086_TAG, 0)
	ROM_LOAD16_BYTE( "40044-2.ic132", 0x0000, 0x2000, CRC(ea527e6e) SHA1(b77fa44767a71a0b321a88bb0a394f1125b7c220) )
	ROM_LOAD16_BYTE( "40043-2.ic129", 0x0001, 0x2000, CRC(532c3854) SHA1(18a17b710f9eb079d9d7216d07807030f904ceda) )

	ROM_REGION( 0x2000, AMS40041_TAG, 0 )
	ROM_LOAD( "40078.ic127", 0x0000, 0x2000, CRC(ae9c0d04) SHA1(bc8dc4dcedeea5bc1c04986b1f105ad93cb2ebcd) )
ROM_END


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

	ROM_REGION16_LE( 0x8000, "iga", 0)
	ROM_LOAD( "40100.ic913", 0x0000, 0x8000, CRC(d2d1f1ae) SHA1(98302006ee38a17c09bd75504cc18c0649174e33) ) // 8736 E
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT    COMPANY         FULLNAME        FLAGS
COMP( 1986, pc1512,     0,          0,      pc1512,     pc1512, driver_device,      0,      "Amstrad plc",  "PC1512 (V1)",  GAME_SUPPORTS_SAVE )
COMP( 1987, pc1512v2,   pc1512,     0,      pc1512,     pc1512, driver_device,      0,      "Amstrad plc",  "PC1512 (V2)",  GAME_SUPPORTS_SAVE )
COMP( 1989, pc1512v3,   pc1512,     0,      pc1512,     pc1512, driver_device,      0,      "Amstrad plc",  "PC1512 (V3)",  GAME_NOT_WORKING )
COMP( 1987, pc1640,     0,          0,      pc1640,     pc1640, driver_device,      0,      "Amstrad plc",  "PC1640",       GAME_NOT_WORKING )
