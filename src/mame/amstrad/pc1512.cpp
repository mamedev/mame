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

PC1512-HD10: ?
PC1512-HD20: Tandon TM-262 [-chs 615,4,17 -ss 512]
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
7. Type FDISK and press return.
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

PC1640-HD20: Amstrad 40095 (Alps DRMD20A12A), Tandon TM-262 [-chs 615,4,17 -ss 512]
PC1640-HD30: Western Digital 95038 [-chs 615,6,17 -ss 512]
*/

#include "emu.h"
#include "pc1512.h"
#include "bus/rs232/rs232.h"
#include "bus/isa/ega.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


//**************************************************************************
//  SYSTEM STATUS REGISTER
//**************************************************************************

//-------------------------------------------------
//  system_r -
//-------------------------------------------------

uint8_t pc1512_base_state::system_r(offs_t offset)
{
	uint8_t data = 0;

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

void pc1512_base_state::system_w(offs_t offset, uint8_t data)
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

		if (BIT(data, 7))
		{
			m_kb_bits = 0;
			m_kb->data_w(1);
			m_pic->ir1_w(CLEAR_LINE);
		}
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

uint8_t pc1512_base_state::mouse_r(offs_t offset)
{
	uint8_t data = 0;

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

void pc1512_base_state::mouse_w(offs_t offset, uint8_t data)
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

void pc1512_base_state::dma_page_w(offs_t offset, uint8_t data)
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

void pc1512_base_state::nmi_mask_w(uint8_t data)
{
	m_nmi_enable = BIT(data, 7);
}



//**************************************************************************
//  PRINTER
//**************************************************************************

//-------------------------------------------------
//  printer_r -
//-------------------------------------------------

uint8_t pc1512_base_state::printer_r(offs_t offset)
{
	uint8_t data = 0;

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

uint8_t pc1640_state::printer_r(offs_t offset)
{
	uint8_t data = 0;

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
		data = pc1512_base_state::printer_r(offset);
		break;
	}

	return data;
}


//-------------------------------------------------
//  printer_w -
//-------------------------------------------------

void pc1512_base_state::printer_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		m_printer_data = data;
		m_cent_data_out->write(data);
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

uint8_t pc1640_state::io_r(offs_t offset)
{
	uint8_t data = 0;
	offs_t addr = offset & 0x3ff;
	bool decoded = false;

	if      (                 addr <= 0x00f) { data = m_dmac->read(offset & 0x0f); decoded = true; }
	else if (addr >= 0x020 && addr <= 0x021) { data = m_pic->read(offset & 0x01); decoded = true; }
	else if (addr >= 0x040 && addr <= 0x043) { data = m_pit->read(offset & 0x03); decoded = true; }
	else if (addr >= 0x060 && addr <= 0x06f) { data = system_r(offset & 0x0f); decoded = true; }
	else if (addr >= 0x070 && addr <= 0x073) { data = m_rtc->read(offset & 0x01); decoded = true; }
	else if (addr >= 0x078 && addr <= 0x07f) { data = mouse_r(offset & 0x07); decoded = true; }
	else if (addr >= 0x378 && addr <= 0x37b) { data = printer_r(offset & 0x03); decoded = true; }
	else if (addr >= 0x3b0 && addr <= 0x3df) { decoded = true; }
	else if (addr >= 0x3f4 && addr <= 0x3f4) { data = m_fdc->msr_r(); decoded = true; }
	else if (addr >= 0x3f5 && addr <= 0x3f5) { data = m_fdc->fifo_r(); decoded = true; }
	else if (addr >= 0x3f8 && addr <= 0x3ff) { data = m_uart->ins8250_r(offset & 0x07); decoded = true; }

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
		uint16_t sw = m_sw->read();

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

void pc1512_state::pc1512_mem(address_map &map)
{
	map(0x00000, 0x9ffff).ram();
	map(0xb8000, 0xbbfff).rw(m_vdu, FUNC(ams40041_device::video_ram_r), FUNC(ams40041_device::video_ram_w));
	map(0xfc000, 0xfffff).rom().region(I8086_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( pc1512_io )
//-------------------------------------------------

void pc1512_state::pc1512_io(address_map &map)
{
	// [RH] 29 Aug 2016: I can find no evidence to indicate that Amstrad had only 10 I/O lines, as the
	// schematic calls for a stock 8086 and the I/O and data lines are multiplexed onto the same bus,
	// plus address lines 20-10 are towards the middle of a standard ISA slot. If it turns out that this
	// is not in fact accurate to hardware, please add this back in.
	// map.global_mask(0x3ff);
	map(0x000, 0x00f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x020, 0x021).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x040, 0x043).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x060, 0x06f).rw(FUNC(pc1512_state::system_r), FUNC(pc1512_state::system_w));
	map(0x070, 0x071).mirror(0x02).rw(m_rtc, FUNC(mc146818_device::read), FUNC(mc146818_device::write));
	map(0x078, 0x07f).rw(FUNC(pc1512_state::mouse_r), FUNC(pc1512_state::mouse_w));
	map(0x080, 0x083).w(FUNC(pc1512_state::dma_page_w));
	map(0x0a1, 0x0a1).w(FUNC(pc1512_state::nmi_mask_w));
	map(0x378, 0x37b).rw(FUNC(pc1512_state::printer_r), FUNC(pc1512_state::printer_w));
	map(0x3d0, 0x3df).rw(m_vdu, FUNC(ams40041_device::vdu_r), FUNC(ams40041_device::vdu_w));
	map(0x3f2, 0x3f2).w(FUNC(pc1512_state::drive_select_w));
	map(0x3f4, 0x3f5).m(m_fdc, FUNC(upd765a_device::map));
	map(0x3f8, 0x3ff).rw(m_uart, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( pc1640_mem )
//-------------------------------------------------

void pc1640_state::pc1640_mem(address_map &map)
{
	map(0x00000, 0x9ffff).ram();
	map(0xf0000, 0xf3fff).mirror(0xc000).rom().region(I8086_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( pc1640_io )
//-------------------------------------------------

void pc1640_state::pc1640_io(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(pc1640_state::io_r));

	map(0x000, 0x00f).w(m_dmac, FUNC(am9517a_device::write));
	map(0x020, 0x021).w(m_pic, FUNC(pic8259_device::write));
	map(0x040, 0x043).w(m_pit, FUNC(pit8253_device::write));
	map(0x060, 0x06f).w(FUNC(pc1640_state::system_w));
	map(0x070, 0x071).mirror(0x02).w(m_rtc, FUNC(mc146818_device::write));
	map(0x078, 0x07f).w(FUNC(pc1640_state::mouse_w));
	map(0x080, 0x083).w(FUNC(pc1640_state::dma_page_w));
	map(0x0a1, 0x0a1).w(FUNC(pc1640_state::nmi_mask_w));
	map(0x378, 0x37b).w(FUNC(pc1640_state::printer_w));
	map(0x3f2, 0x3f2).w(FUNC(pc1640_state::drive_select_w));
	map(0x3f5, 0x3f5).w(m_fdc, FUNC(upd765a_device::fifo_w));
	map(0x3f8, 0x3ff).w(m_uart, FUNC(ins8250_device::ins8250_w));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( pc1512 )
//-------------------------------------------------

static INPUT_PORTS_START( pc1512 )
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
	PORT_DIPSETTING( 0x40, "Portuguese (Codepage 865)" )
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

WRITE_LINE_MEMBER( pc1512_base_state::kbdata_w )
{
	m_kbdata = state;
}

WRITE_LINE_MEMBER( pc1512_base_state::kbclk_w )
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

void pc1512_base_state::mouse_x_w(uint8_t data)
{
	if (data > m_mouse_x_old)
		m_mouse_x+=3;
	else
		m_mouse_x-=3;

	m_mouse_x_old = data;
}

void pc1512_base_state::mouse_y_w(uint8_t data)
{
	if (data > m_mouse_y_old)
		m_mouse_y-=3;
	else
		m_mouse_y+=3;

	m_mouse_y_old = data;
}

//-------------------------------------------------
//  I8237_INTERFACE( dmac_intf )
//-------------------------------------------------

void pc1512_base_state::update_fdc_tc()
{
	if (m_nden)
		m_fdc->tc_w(m_neop);
	else
		m_fdc->tc_w(false);
}

WRITE_LINE_MEMBER( pc1512_base_state::hrq_w )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	m_dmac->hack_w(state);
}

WRITE_LINE_MEMBER( pc1512_base_state::eop_w )
{
	if (m_dma_channel == 2)
	{
		m_neop = !state;
		update_fdc_tc();
	}
}

uint8_t pc1512_base_state::memr_r(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = m_dma_page[m_dma_channel] << 16;

	return program.read_byte(page_offset + offset);
}

void pc1512_base_state::memw_w(offs_t offset, uint8_t data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = m_dma_page[m_dma_channel] << 16;

	program.write_byte(page_offset + offset, data);
}

uint8_t pc1512_base_state::ior1_r()
{
	return m_bus->dack_r(1);
}

uint8_t pc1512_base_state::ior2_r()
{
	if (m_nden)
		return m_fdc->dma_r();
	else
		return m_bus->dack_r(2);
}

uint8_t pc1512_base_state::ior3_r()
{
	return m_bus->dack_r(3);
}

void pc1512_base_state::iow0_w(uint8_t data)
{
	m_dreq0 = 0;
	m_dmac->dreq0_w(m_dreq0);
}

void pc1512_base_state::iow1_w(uint8_t data)
{
	m_bus->dack_w(1, data);
}

void pc1512_base_state::iow2_w(uint8_t data)
{
	if (m_nden)
		m_fdc->dma_w(data);
	else
		m_bus->dack_w(2, data);
}

void pc1512_base_state::iow3_w(uint8_t data)
{
	m_bus->dack_w(3, data);
}

WRITE_LINE_MEMBER( pc1512_base_state::dack0_w )
{
	if (!state) m_dma_channel = 0;
}

WRITE_LINE_MEMBER( pc1512_base_state::dack1_w )
{
	if (!state) m_dma_channel = 1;
}

WRITE_LINE_MEMBER( pc1512_base_state::dack2_w )
{
	if (!state) m_dma_channel = 2;
}

WRITE_LINE_MEMBER( pc1512_base_state::dack3_w )
{
	if (!state) m_dma_channel = 3;
}

//-------------------------------------------------
//  pit8253_config pit_intf
//-------------------------------------------------

void pc1512_base_state::update_speaker()
{
	m_speaker->level_w(m_speaker_drive & m_pit2);
}

WRITE_LINE_MEMBER( pc1512_base_state::pit1_w )
{
	if (!m_pit1 && state && !m_dreq0)
	{
		m_dreq0 = 1;
		m_dmac->dreq0_w(m_dreq0);
	}

	m_pit1 = state;
}

WRITE_LINE_MEMBER( pc1512_base_state::pit2_w )
{
	m_pit2 = state;
	update_speaker();
}

//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

void pc1512_base_state::update_fdc_int()
{
	if (m_nden)
		m_pic->ir6_w(m_dint);
	else
		m_pic->ir6_w(CLEAR_LINE);
}

void pc1512_base_state::update_fdc_drq()
{
	if (m_nden)
		m_dmac->dreq2_w(m_ddrq);
	else
		m_dmac->dreq2_w(0);
}

WRITE_LINE_MEMBER( pc1512_base_state::fdc_int_w )
{
	m_dint = state;
	update_fdc_int();
}

WRITE_LINE_MEMBER( pc1512_base_state::fdc_drq_w )
{
	m_ddrq = state;
	update_fdc_drq();
}

void pc1512_base_state::drive_select_w(uint8_t data)
{
	m_fdc->set_floppy((data & 0x03) < 2 ? m_floppy[data & 0x03]->get_device() : nullptr);
	for (int n = 0; n < 2; n++)
	{
		floppy_image_device *img = m_floppy[n]->get_device();
		if (img != nullptr)
			 img->mon_w((data & 0x03) == n && BIT(data, n + 4) ? 0 : 1);
	}

	m_fdc->reset_w(!BIT(data, 2));
	m_nden = BIT(data, 3);
	update_fdc_int();
	update_fdc_drq();
}

//-------------------------------------------------
//  centronics_interface centronics_intf
//-------------------------------------------------

void pc1512_base_state::update_ack()
{
	if (m_ack_int_enable)
		m_pic->ir7_w(m_centronics_ack);
	else
		m_pic->ir7_w(CLEAR_LINE);
}

WRITE_LINE_MEMBER( pc1512_base_state::write_centronics_ack )
{
	m_centronics_ack = state;
	update_ack();
}

WRITE_LINE_MEMBER( pc1512_base_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER( pc1512_base_state::write_centronics_perror )
{
	m_centronics_perror = state;
}

WRITE_LINE_MEMBER( pc1512_base_state::write_centronics_select )
{
	m_centronics_select = state;
}

WRITE_LINE_MEMBER( pc1512_base_state::write_centronics_fault )
{
	m_centronics_fault = state;
}


//-------------------------------------------------
//  isa8bus_interface isabus_intf
//-------------------------------------------------

void pc1640_isa8_cards(device_slot_interface &device)
{
	device.option_add_internal("iga", ISA8_PC1640_IGA);
}

static void pc1512_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD); // Tandon TM65-2L
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

void pc1512_base_state::machine_start()
{
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
	save_item(NAME(m_mouse_x_old));
	save_item(NAME(m_mouse_y_old));
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
}

void pc1512_state::machine_start()
{
	pc1512_base_state::machine_start();

	// set RAM size
	size_t ram_size = m_ram->size();

	if (ram_size < 640 * 1024)
	{
		address_space &program = m_maincpu->space(AS_PROGRAM);
		program.unmap_readwrite(ram_size, 0x9ffff);
	}
}

void pc1512_base_state::machine_reset()
{
	m_nmi_enable = 0;
	drive_select_w(0);

	m_kb_bits = 0;
	m_kb->data_w(1);
	m_pic->ir1_w(CLEAR_LINE);
}

void pc1512_state::machine_reset()
{
	pc1512_base_state::machine_reset();

	m_pit2 = 1;
}


//-------------------------------------------------
//  MACHINE_START( pc1640 )
//-------------------------------------------------

void pc1640_state::machine_start()
{
	pc1512_base_state::machine_start();

	// state saving
	save_item(NAME(m_opt));
}


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( pc1512 )
//-------------------------------------------------

void pc1512_state::pc1512(machine_config &config)
{
	I8086(config, m_maincpu, 24_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc1512_state::pc1512_mem);
	m_maincpu->set_addrmap(AS_IO, &pc1512_state::pc1512_io);
	m_maincpu->set_irq_acknowledge_callback(I8259A2_TAG, FUNC(pic8259_device::inta_cb));

	// video
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_raw(28.636363_MHz_XTAL, 912, 0, 910, 262, 0, 260);
	screen.set_screen_update(m_vdu, FUNC(ams40041_device::screen_update));

	AMS40041(config, m_vdu, 28.636363_MHz_XTAL);
	m_vdu->set_screen(SCREEN_TAG);
	m_vdu->set_show_border_area(true);

	// sound
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.80);

	// devices
	PC1512_KEYBOARD(config, m_kb, 0);
	m_kb->clock_wr_callback().set(FUNC(pc1512_state::kbclk_w));
	m_kb->data_wr_callback().set(FUNC(pc1512_state::kbdata_w));

	pc1512_mouse_port_device &mouse(PC1512_MOUSE_PORT(config, "mous", pc1512_mouse_port_devices, "mouse"));
	mouse.x_wr_callback().set(FUNC(pc1512_base_state::mouse_x_w));
	mouse.y_wr_callback().set(FUNC(pc1512_base_state::mouse_y_w));
	mouse.m1_wr_callback().set(m_kb, FUNC(pc1512_keyboard_device::m1_w));
	mouse.m2_wr_callback().set(m_kb, FUNC(pc1512_keyboard_device::m2_w));

	AM9517A(config, m_dmac, 24_MHz_XTAL / 6);
	m_dmac->out_hreq_callback().set(FUNC(pc1512_state::hrq_w));
	m_dmac->out_eop_callback().set(FUNC(pc1512_state::eop_w));
	m_dmac->in_memr_callback().set(FUNC(pc1512_state::memr_r));
	m_dmac->out_memw_callback().set(FUNC(pc1512_state::memw_w));
	m_dmac->in_ior_callback<1>().set(FUNC(pc1512_state::ior1_r));
	m_dmac->in_ior_callback<2>().set(FUNC(pc1512_state::ior2_r));
	m_dmac->in_ior_callback<3>().set(FUNC(pc1512_state::ior3_r));
	m_dmac->out_iow_callback<0>().set(FUNC(pc1512_state::iow0_w));
	m_dmac->out_iow_callback<1>().set(FUNC(pc1512_state::iow1_w));
	m_dmac->out_iow_callback<2>().set(FUNC(pc1512_state::iow2_w));
	m_dmac->out_iow_callback<3>().set(FUNC(pc1512_state::iow3_w));
	m_dmac->out_dack_callback<0>().set(FUNC(pc1512_state::dack0_w));
	m_dmac->out_dack_callback<1>().set(FUNC(pc1512_state::dack1_w));
	m_dmac->out_dack_callback<2>().set(FUNC(pc1512_state::dack2_w));
	m_dmac->out_dack_callback<3>().set(FUNC(pc1512_state::dack3_w));

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(28.636363_MHz_XTAL / 24);
	m_pit->out_handler<0>().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_pit->set_clk<1>(28.636363_MHz_XTAL / 24);
	m_pit->out_handler<1>().set(FUNC(pc1512_state::pit1_w));
	m_pit->set_clk<2>(28.636363_MHz_XTAL / 24);
	m_pit->out_handler<2>().set(FUNC(pc1512_state::pit2_w));

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_pic, FUNC(pic8259_device::ir2_w));

	UPD765A(config, m_fdc, 24_MHz_XTAL / 6, false, false);
	// SED9420CAC (dedicated 16 MHz XTAL) is used as read data separator only
	m_fdc->intrq_wr_callback().set(FUNC(pc1512_state::fdc_int_w));
	m_fdc->drq_wr_callback().set(FUNC(pc1512_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, m_floppy[0], pc1512_floppies, "525dd", floppy_image_device::default_pc_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], pc1512_floppies, nullptr, floppy_image_device::default_pc_floppy_formats);

	INS8250(config, m_uart, 1.8432_MHz_XTAL);
	m_uart->out_tx_callback().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	m_uart->out_dtr_callback().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	m_uart->out_rts_callback().set(RS232_TAG, FUNC(rs232_port_device::write_rts));
	m_uart->out_int_callback().set(m_pic, FUNC(pic8259_device::ir4_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(FUNC(pc1512_state::write_centronics_ack));
	m_centronics->busy_handler().set(FUNC(pc1512_state::write_centronics_busy));
	m_centronics->perror_handler().set(FUNC(pc1512_state::write_centronics_perror));
	m_centronics->select_handler().set(FUNC(pc1512_state::write_centronics_select));
	m_centronics->fault_handler().set(FUNC(pc1512_state::write_centronics_fault));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(ins8250_uart_device::rx_w));
	rs232.dcd_handler().set(m_uart, FUNC(ins8250_uart_device::dcd_w));
	rs232.dsr_handler().set(m_uart, FUNC(ins8250_uart_device::dsr_w));
	rs232.ri_handler().set(m_uart, FUNC(ins8250_uart_device::ri_w));
	rs232.cts_handler().set(m_uart, FUNC(ins8250_uart_device::cts_w));

	// ISA8 bus
	isa8_device &isa(ISA8(config, ISA_BUS_TAG, 0));
	isa.set_memspace(m_maincpu, AS_PROGRAM);
	isa.set_iospace(m_maincpu, AS_IO);
	isa.irq2_callback().set(m_pic, FUNC(pic8259_device::ir2_w));
	isa.irq3_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	isa.irq4_callback().set(m_pic, FUNC(pic8259_device::ir4_w));
	isa.irq5_callback().set(m_pic, FUNC(pic8259_device::ir5_w));
	isa.irq6_callback().set(m_pic, FUNC(pic8259_device::ir6_w));
	isa.irq7_callback().set(m_pic, FUNC(pic8259_device::ir7_w));
	isa.drq1_callback().set(I8237A5_TAG, FUNC(am9517a_device::dreq1_w));
	isa.drq2_callback().set(I8237A5_TAG, FUNC(am9517a_device::dreq2_w));
	isa.drq3_callback().set(I8237A5_TAG, FUNC(am9517a_device::dreq3_w));
	ISA8_SLOT(config, "isa1", 0, ISA_BUS_TAG, pc_isa8_cards, nullptr, false); // FIXME: determine ISA clock
	ISA8_SLOT(config, "isa2", 0, ISA_BUS_TAG, pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa3", 0, ISA_BUS_TAG, pc_isa8_cards, nullptr, false);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("512K").set_extra_options("544K,576K,608K,640K");

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("pc1512_flop");
	SOFTWARE_LIST(config, "hdd_list").set_original("pc1512_hdd");
}


//-------------------------------------------------
//  machine_config( pc1512dd )
//-------------------------------------------------

void pc1512_state::pc1512dd(machine_config &config)
{
	pc1512(config);
	m_floppy[1]->set_default_option("525dd");
}


//-------------------------------------------------
//  machine_config( pc1512hd )
//-------------------------------------------------

void pc1512_state::pc1512hd(machine_config &config)
{
	pc1512(config);
	//subdevice<isa8_slot_device>("isa1")->set_default_option("wdxt_gen");
	subdevice<isa8_slot_device>("isa1")->set_default_option("hdc");
}


//-------------------------------------------------
//  machine_config( pc1640 )
//-------------------------------------------------

void pc1640_state::pc1640(machine_config &config)
{
	I8086(config, m_maincpu, 24_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc1640_state::pc1640_mem);
	m_maincpu->set_addrmap(AS_IO, &pc1640_state::pc1640_io);
	m_maincpu->set_irq_acknowledge_callback(I8259A2_TAG, FUNC(pic8259_device::inta_cb));

	// sound
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.80);

	// devices
	PC1512_KEYBOARD(config, m_kb, 0);
	m_kb->clock_wr_callback().set(FUNC(pc1512_base_state::kbclk_w));
	m_kb->data_wr_callback().set(FUNC(pc1512_base_state::kbdata_w));

	pc1512_mouse_port_device &mouse(PC1512_MOUSE_PORT(config, "mous", pc1512_mouse_port_devices, "mouse"));
	mouse.x_wr_callback().set(FUNC(pc1512_base_state::mouse_x_w));
	mouse.y_wr_callback().set(FUNC(pc1512_base_state::mouse_y_w));
	mouse.m1_wr_callback().set(m_kb, FUNC(pc1512_keyboard_device::m1_w));
	mouse.m2_wr_callback().set(m_kb, FUNC(pc1512_keyboard_device::m2_w));

	AM9517A(config, m_dmac, 24_MHz_XTAL / 6);
	m_dmac->out_hreq_callback().set(FUNC(pc1640_state::hrq_w));
	m_dmac->out_eop_callback().set(FUNC(pc1640_state::eop_w));
	m_dmac->in_memr_callback().set(FUNC(pc1640_state::memr_r));
	m_dmac->out_memw_callback().set(FUNC(pc1640_state::memw_w));
	m_dmac->in_ior_callback<1>().set(FUNC(pc1640_state::ior1_r));
	m_dmac->in_ior_callback<2>().set(FUNC(pc1640_state::ior2_r));
	m_dmac->in_ior_callback<3>().set(FUNC(pc1640_state::ior3_r));
	m_dmac->out_iow_callback<0>().set(FUNC(pc1640_state::iow0_w));
	m_dmac->out_iow_callback<1>().set(FUNC(pc1640_state::iow1_w));
	m_dmac->out_iow_callback<2>().set(FUNC(pc1640_state::iow2_w));
	m_dmac->out_iow_callback<3>().set(FUNC(pc1640_state::iow3_w));
	m_dmac->out_dack_callback<0>().set(FUNC(pc1640_state::dack0_w));
	m_dmac->out_dack_callback<1>().set(FUNC(pc1640_state::dack1_w));
	m_dmac->out_dack_callback<2>().set(FUNC(pc1640_state::dack2_w));
	m_dmac->out_dack_callback<3>().set(FUNC(pc1640_state::dack3_w));

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(28.636363_MHz_XTAL / 24);
	m_pit->out_handler<0>().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_pit->set_clk<1>(28.636363_MHz_XTAL / 24);
	m_pit->out_handler<1>().set(FUNC(pc1512_base_state::pit1_w));
	m_pit->set_clk<2>(28.636363_MHz_XTAL / 24);
	m_pit->out_handler<2>().set(FUNC(pc1512_base_state::pit2_w));

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_pic, FUNC(pic8259_device::ir2_w));

	UPD765A(config, m_fdc, 24_MHz_XTAL / 6, false, false);
	// FDC91C36 (clocked by CK8K) is used as read data separator only
	m_fdc->intrq_wr_callback().set(FUNC(pc1512_base_state::fdc_int_w));
	m_fdc->drq_wr_callback().set(FUNC(pc1512_base_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, m_floppy[0], pc1512_floppies, "525dd", floppy_image_device::default_pc_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], pc1512_floppies, nullptr, floppy_image_device::default_pc_floppy_formats);

	INS8250(config, m_uart, 1.8432_MHz_XTAL);
	m_uart->out_tx_callback().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	m_uart->out_dtr_callback().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	m_uart->out_rts_callback().set(RS232_TAG, FUNC(rs232_port_device::write_rts));
	m_uart->out_int_callback().set(m_pic, FUNC(pic8259_device::ir4_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(FUNC(pc1512_state::write_centronics_ack));
	m_centronics->busy_handler().set(FUNC(pc1512_state::write_centronics_busy));
	m_centronics->perror_handler().set(FUNC(pc1512_state::write_centronics_perror));
	m_centronics->select_handler().set(FUNC(pc1512_state::write_centronics_select));
	m_centronics->fault_handler().set(FUNC(pc1512_state::write_centronics_fault));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(ins8250_uart_device::rx_w));
	rs232.dcd_handler().set(m_uart, FUNC(ins8250_uart_device::dcd_w));
	rs232.dsr_handler().set(m_uart, FUNC(ins8250_uart_device::dsr_w));
	rs232.ri_handler().set(m_uart, FUNC(ins8250_uart_device::ri_w));
	rs232.cts_handler().set(m_uart, FUNC(ins8250_uart_device::cts_w));

	// ISA8 bus
	isa8_device &isa(ISA8(config, ISA_BUS_TAG, 0));
	isa.set_memspace(m_maincpu, AS_PROGRAM);
	isa.set_iospace(m_maincpu, AS_IO);
	isa.irq2_callback().set(m_pic, FUNC(pic8259_device::ir2_w));
	isa.irq3_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	isa.irq4_callback().set(m_pic, FUNC(pic8259_device::ir4_w));
	isa.irq5_callback().set(m_pic, FUNC(pic8259_device::ir5_w));
	isa.irq6_callback().set(m_pic, FUNC(pic8259_device::ir6_w));
	isa.irq7_callback().set(m_pic, FUNC(pic8259_device::ir7_w));
	isa.drq1_callback().set(I8237A5_TAG, FUNC(am9517a_device::dreq1_w));
	isa.drq2_callback().set(I8237A5_TAG, FUNC(am9517a_device::dreq2_w));
	isa.drq3_callback().set(I8237A5_TAG, FUNC(am9517a_device::dreq3_w));
	ISA8_SLOT(config, "isa1", 0, ISA_BUS_TAG, pc_isa8_cards, nullptr, false); // FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa2", 0, ISA_BUS_TAG, pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa3", 0, ISA_BUS_TAG, pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa4", 0, ISA_BUS_TAG, pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa5", 0, ISA_BUS_TAG, pc1640_isa8_cards, "iga", false);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("640K");

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("pc1640_flop");
	SOFTWARE_LIST(config, "hdd_list").set_original("pc1640_hdd");
}


//-------------------------------------------------
//  machine_config( pc1640dd )
//-------------------------------------------------

void pc1640_state::pc1640dd(machine_config &config)
{
	pc1640(config);
	m_floppy[1]->set_default_option("525dd");
}


//-------------------------------------------------
//  machine_config( pc1640hd )
//-------------------------------------------------

void pc1640_state::pc1640hd(machine_config &config)
{
	pc1640(config);
	//subdevice<isa8_slot_device>("isa1")->set_default_option("wdxt_gen");
	subdevice<isa8_slot_device>("isa1")->set_default_option("hdc");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( pc1512 )
//-------------------------------------------------

ROM_START( pc1512 )
	ROM_REGION16_LE( 0x4000, I8086_TAG, 0)
	ROM_SYSTEM_BIOS( 0, "v1", "Version 1" )
	ROMX_LOAD( "40044.ic132", 0x0000, 0x2000, CRC(f72f1582) SHA1(7781d4717917262805d514b331ba113b1e05a247), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "40043.ic129", 0x0001, 0x2000, CRC(668fcc94) SHA1(74002f5cc542df442eec9e2e7a18db3598d8c482), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v2", "Version 2" )
	ROMX_LOAD( "40044v2.ic132", 0x0000, 0x2000, CRC(1aec54fa) SHA1(b12fd73cfc35a240ed6da4dcc4b6c9910be611e0), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "40043v2.ic129", 0x0001, 0x2000, CRC(d2d4d2de) SHA1(c376fd1ad23025081ae16c7949e88eea7f56e1bb), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v3", "Version 3" )
	ROMX_LOAD( "40044-2.ic132", 0x0000, 0x2000, CRC(ea527e6e) SHA1(b77fa44767a71a0b321a88bb0a394f1125b7c220), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "40043-2.ic129", 0x0001, 0x2000, CRC(532c3854) SHA1(18a17b710f9eb079d9d7216d07807030f904ceda), ROM_SKIP(1) | ROM_BIOS(2) )

	ROM_REGION( 0x2000, AMS40041_TAG, 0 )
	ROMX_LOAD( "40045.ic127", 0x0000, 0x2000, CRC(dd5e030f) SHA1(7d858bbb2e8d6143aa67ab712edf5f753c2788a7), ROM_BIOS(0) )
	ROMX_LOAD( "40078.ic127", 0x0000, 0x2000, CRC(ae9c0d04) SHA1(bc8dc4dcedeea5bc1c04986b1f105ad93cb2ebcd), ROM_BIOS(1) )
	ROMX_LOAD( "40078.ic127", 0x0000, 0x2000, CRC(ae9c0d04) SHA1(bc8dc4dcedeea5bc1c04986b1f105ad93cb2ebcd), ROM_BIOS(2) )
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
	ROMX_LOAD( "40044-1 8809.ic132", 0x0000, 0x2000, CRC(f1c074f3) SHA1(a055ea7e933d137623c22fe24004e870653c7952), ROM_SKIP(1) | ROM_BIOS(0) ) // 8809 B
	ROMX_LOAD( "40043-1 8809.ic129", 0x0001, 0x2000, CRC(e40a1513) SHA1(447eff2057e682e51b1c7593cb6fad0e53879fa8), ROM_SKIP(1) | ROM_BIOS(0) ) // 8809 B
	ROM_SYSTEM_BIOS( 1, "8738", "Week 38/1987" )
	ROMX_LOAD( "40044 8738.ic132", 0x0000, 0x2000, CRC(43832ea7) SHA1(eea4a8836f966940a88c88de6c5cc14852545f7d), ROM_SKIP(1) | ROM_BIOS(1) ) // 8738 D F
	ROMX_LOAD( "40043 8738.ic129", 0x0001, 0x2000, CRC(768498f9) SHA1(ac48cb892417d7998d604f3b79756140c554f476), ROM_SKIP(1) | ROM_BIOS(1) ) // 8738 D F
	ROM_SYSTEM_BIOS( 2, "88xx", "Week ?/1988" )
	ROMX_LOAD( "40044 88xx.ic132", 0x0000, 0x2000, CRC(6090f782) SHA1(e21ae524d5b4d00696d293dbd4fe4d7bca22e277), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "40043 88xx.ic129", 0x0001, 0x2000, CRC(9219d0aa) SHA1(dde1a46c8f83e413d7070f1356fc91b9f595a8b6), ROM_SKIP(1) | ROM_BIOS(2) )
ROM_END

#define rom_pc1640dd    rom_pc1640
#define rom_pc1640hd20  rom_pc1640
#define rom_pc1640hd30  rom_pc1640



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT  COMPAT  MACHINE   INPUT   CLASS         INIT        COMPANY        FULLNAME       FLAGS
COMP( 1986, pc1512,     0,      0,      pc1512,   pc1512, pc1512_state, empty_init, "Amstrad plc", "PC1512 SD",   MACHINE_SUPPORTS_SAVE )
COMP( 1986, pc1512dd,   pc1512, 0,      pc1512dd, pc1512, pc1512_state, empty_init, "Amstrad plc", "PC1512 DD",   MACHINE_SUPPORTS_SAVE )
COMP( 1986, pc1512hd10, pc1512, 0,      pc1512hd, pc1512, pc1512_state, empty_init, "Amstrad plc", "PC1512 HD10", MACHINE_SUPPORTS_SAVE )
COMP( 1986, pc1512hd20, pc1512, 0,      pc1512hd, pc1512, pc1512_state, empty_init, "Amstrad plc", "PC1512 HD20", MACHINE_SUPPORTS_SAVE )
COMP( 1987, pc1640,     0,      0,      pc1640,   pc1640, pc1640_state, empty_init, "Amstrad plc", "PC1640 SD",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1987, pc1640dd,   pc1640, 0,      pc1640dd, pc1640, pc1640_state, empty_init, "Amstrad plc", "PC1640 DD",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1987, pc1640hd20, pc1640, 0,      pc1640hd, pc1640, pc1640_state, empty_init, "Amstrad plc", "PC1640 HD20", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1987, pc1640hd30, pc1640, 0,      pc1640hd, pc1640, pc1640_state, empty_init, "Amstrad plc", "PC1640 HD30", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
