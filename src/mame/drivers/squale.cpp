// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jean-François DEL NERO
/***************************************************************************

    Apollo 7 Squale

    The following hardware description is an extract of the Squale hardware analysis
    presented on this page : http://hxc2001.free.fr/Squale.
    This is a work in progress and is subject to changes

    PCB Ref Qty Manufacturer Ref    Description / Datasheet
    ============================================================================================
    U1              1   EF6809P             8-BIT MICROPROCESSOR UNIT (MPU)
    U2              1   27C32 / 27C64       EPROM
    U72,U75         1   EF6821P             PERIPHERAL INTERFACE ADAPTER (PIA)
    U69             1   EF6850              ASYNCHRONOUS COMMUNICATIONS INTERFACE ADAPTER (ACIA)
    U59             1   EF9365P             GRAPHIC DISPLAY PROCESSOR (GDP)
    U65             1   AY-3-8910A          PROGRAMMABLE SOUND GENERATOR
    U16,U17,U18,U19,
    U20,U21,U22,U23 8   MK4564              65,536 x 1-BIT DYNAMIC RAM
    U38,U39,U40,U41,
    U42,U43,U44,U45,
    U46,U47,U48,U49,
    U50,U51,U52,U53 16  TMS4116             16,384-BIT DYNAMIC RAM
    U68             1   EFB7510             SINGLE CHIP ASYNCHRONOUS FSK MODEM


    Memory map
    ==========

    Devices                                     Adresses
    =========================================================
    EPROM                                       0xF100-0xFFFF
    Extension Port                              0xF080-0xF0FF
    FREE                                        0xF070-0xF07F
    AY-3-8910A                                  0xF060-0xF06F
    ACIA EF6850 (Modem + K7)                    0xF050-0xF05F
    Restricted area                             0xF04C-0xF04F
    PIO EF6821 (Printer + Cartridge)            0xF048-0xF04B
    PIO EF6821 (Keyboard)                       0xF044-0xF047
    FREE                                        0xF040-0xF043
    VID_RD2                                     0xF030-0xF03F
    VID_RD1                                     0xF020-0xF02F
    REG1                                        0xF010-0xF01F
    Video Controller EF9365                     0xF000-0xF00F
    System RAM                                  0x0000-0xEFFF


    Notes:
    1) For 8KB versions of the monitor, the bank switching is done with the bit 7 of REG1.
    2) VID_RD1 : [7..0] = I0,R0,G0,B0,I1,R1,G1,B1 (I=Intensity,R=Red,G=Green,B=Blue)
    3) VID_RD2 : [7..0] = I2,R2,G2,B2,I3,R3,G3,B3 (I=Intensity,R=Red,G=Green,B=Blue)
    3) REG1 : [7..0] = EPROM Bank,-,Modem,K7,I,R,G,B (I=Intensity,R=Red,V=Green,B=Blue)

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/ef9365.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "sound/ay8910.h"
#include "machine/wd_fdc.h"
#include "bus/generic/carts.h"

#include "softlist.h"

#define MAIN_CLOCK           XTAL_14MHz
#define AY_CLOCK             MAIN_CLOCK / 8     /* 1.75 Mhz */
#define VIDEO_CLOCK          MAIN_CLOCK / 8     /* 1.75 Mhz */
#define CPU_CLOCK            MAIN_CLOCK / 4     /* 3.50 Mhz */

class squale_state : public driver_device
{
public:
	squale_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_acia(*this,  "ef6850")
		, m_ay8910(*this,  "ay8910")
		, m_pia_u72(*this, "pia_u72")
		, m_pia_u75(*this, "pia_u75")
		, m_ef9365(*this, "ef9365")
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "wd1770")
		, m_floppy0(*this, "wd1770:0")
		, m_floppy1(*this, "wd1770:1")
		, m_floppy(NULL)
		, m_cart(*this, "cartslot")
	{ }

	DECLARE_WRITE8_MEMBER(ctrl_w);
	DECLARE_READ8_MEMBER(video_ram_read_reg1);
	DECLARE_READ8_MEMBER(video_ram_read_reg2);
	DECLARE_WRITE8_MEMBER(fdc_sel0_w);
	DECLARE_READ8_MEMBER(fdc_sel0_r);
	DECLARE_WRITE8_MEMBER(fdc_sel1_w);
	DECLARE_READ8_MEMBER(fdc_sel1_r);
	DECLARE_READ8_MEMBER(pia_u72_porta_r);
	DECLARE_READ8_MEMBER(pia_u72_portb_r);
	DECLARE_READ8_MEMBER(pia_u75_porta_r);
	DECLARE_READ8_MEMBER(pia_u75_portb_r);
	DECLARE_WRITE8_MEMBER(pia_u72_porta_w);
	DECLARE_WRITE8_MEMBER(pia_u72_portb_w);
	DECLARE_WRITE8_MEMBER(pia_u75_porta_w);
	DECLARE_WRITE8_MEMBER(pia_u75_portb_w);

	DECLARE_READ8_MEMBER(ay_porta_r);
	DECLARE_READ8_MEMBER(ay_portb_r);
	DECLARE_WRITE8_MEMBER(ay_porta_w);
	DECLARE_WRITE8_MEMBER(ay_portb_w);

	DECLARE_WRITE_LINE_MEMBER(pia_u72_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia_u72_cb2_w);

	DECLARE_WRITE_LINE_MEMBER(pia_u75_cb2_w);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( squale_cart );
	virtual void machine_start() override;
	virtual void machine_reset() override;

	UINT8 keyboard_line;
	UINT8 fdc_sel0;
	UINT8 fdc_sel1;

	UINT8  cart_addr_counter_inc_ck;
	UINT8  cart_addr_counter_reset;
	UINT16 cart_addr_counter;

	TIMER_DEVICE_CALLBACK_MEMBER(squale_scanline);

private:
	required_device<acia6850_device> m_acia;
	required_device<ay8910_device> m_ay8910;
	required_device<pia6821_device> m_pia_u72;
	required_device<pia6821_device> m_pia_u75;
	required_device<ef9365_device> m_ef9365;
	required_device<cpu_device> m_maincpu;
	required_device<wd1770_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;
	required_device<generic_slot_device> m_cart;

	memory_region *m_cart_rom;
};

/*****************************************
* Machine control register I/O Handlers  *
******************************************/

WRITE8_MEMBER( squale_state::ctrl_w )
{
	#ifdef DBGMODE
	printf("write ctrl reg : 0x%X\n",data);
	#endif

	membank("rom_bank")->set_entry(data >> 7);

	m_ef9365->set_color_filler(data & 0xF);
}

READ8_MEMBER( squale_state::video_ram_read_reg1 )
{
	UINT8 data;
	int p;

	//D7             D0
	//I2R2G2B2 I3R3G3B3

	data = 0x00;

	for(p = 0; p < 4 ; p++)
	{
		if( m_ef9365->get_last_readback_word(p, 0) & 8 )
		{
			data |= (0x01 << p);
		}
	}

	data = data << 4;

	for(p = 0; p < 4 ; p++)
	{
		if( m_ef9365->get_last_readback_word(p, 0) & 4 )
		{
			data |= (0x01 << p);
		}
	}

	#ifdef DBGMODE
	printf("read video_ram_read_reg1 reg : 0x%X\n",data);
	#endif

	return data;
}

READ8_MEMBER( squale_state::video_ram_read_reg2 )
{
	UINT8 data;
	int p;

	//D7             D0
	//I0R0G0B0 I1R1G1B1

	data = 0x00;

	for(p = 0; p < 4 ; p++)
	{
		if( m_ef9365->get_last_readback_word(p, 0) & 2 )
		{
			data |= (0x01 << p);
		}
	}

	data = data << 4;

	for(p = 0; p < 4 ; p++)
	{
		if( m_ef9365->get_last_readback_word(p, 0) & 1 )
		{
			data |= (0x01 << p);
		}
	}

	#ifdef DBGMODE
	printf("read video_ram_read_reg2 reg : 0x%X\n",data);
	#endif

	return data;
}

/**********************************
* Floppy controller I/O Handlers  *
***********************************/

WRITE8_MEMBER( squale_state::fdc_sel0_w )
{
	floppy_image_device *floppy = 0;

	#ifdef DBGMODE
	printf("%s: write fdc_sel0_w reg : 0x%X\n",machine().describe_context(),data);
	#endif

	fdc_sel0 = data;

	if( BIT(data, 3) ) // Drive 0
	{
		floppy = m_floppy0->get_device();
		if(!floppy)
			fdc_sel0 = fdc_sel0 & ~(0x08);
		else
		{
			if(!floppy->dskchg_r())
			{
				fdc_sel0 = fdc_sel0 & ~(0x08);
			}
		}
	}

	if( BIT(data, 2) ) // Drive 1
	{
		floppy = m_floppy1->get_device();
		if(!floppy)
			fdc_sel0 = fdc_sel0 & ~(0x04);
		else
		{
			if(!floppy->dskchg_r())
			{
				fdc_sel0 = fdc_sel0 & ~(0x04);
			}
		}
	}

	if(floppy)
	{
		floppy->ss_w(BIT(data, 4)); // Side selector bit
	}

	//m_fdc->mon_w(BIT(data, 3));
	m_fdc->dden_w(BIT(data, 5));    // Double / Single density selector bit
	m_fdc->set_floppy(floppy);
}

WRITE8_MEMBER( squale_state::fdc_sel1_w )
{
	#ifdef DBGMODE
	printf("%s: write fdc_sel1_w reg : 0x%X\n",machine().describe_context(),data);
	#endif

	fdc_sel1 = data;
}

READ8_MEMBER( squale_state::fdc_sel0_r )
{
	UINT8 data;

	data = fdc_sel0;

	#ifdef DBGMODE
	printf("%s: read fdc_sel0_r 0x%.2X\n",machine().describe_context(),data);
	#endif

	return data;
}

READ8_MEMBER( squale_state::fdc_sel1_r )
{
	UINT8 data;

	data = fdc_sel1;

	#ifdef DBGMODE
	printf("%s: read fdc_sel1_r 0x%.2X\n",machine().describe_context(),data);
	#endif

	return data;
}

/**********************************
*      Keyboard I/O Handlers      *
***********************************/

WRITE8_MEMBER( squale_state::pia_u75_porta_w )
{
	// U75 PIA Port A : Keyboard rows output
	#ifdef DBGMODE
	printf("%s: write pia_u75_porta_w : 0x%.2X\n",machine().describe_context(),data);
	#endif
	keyboard_line = data;
	return;
}

READ8_MEMBER( squale_state::pia_u75_porta_r )
{
	// U75 PIA Port A : Keyboard rows output
	UINT8 data;

	#ifdef DBGMODE
	printf("%s: read pia_u75_porta_r\n",machine().describe_context());
	#endif

	data = keyboard_line;
	return data;
}

READ8_MEMBER( squale_state::pia_u75_portb_r )
{
	// U75 PIA Port B : Keyboard column input
	char kbdrow[3];
	unsigned char kbdrow_state;
	UINT8 data = 0xFF;

	kbdrow[0] = 'X';
	kbdrow[1] = '0';
	kbdrow[2] = 0;

	for (int i = 0; i < 8; i++)
	{
		kbdrow[1] = '0' + i;
		kbdrow_state = ioport(kbdrow)->read();

		for( int j = 0; j < 8 ; j++)
		{
			if( !(keyboard_line & (0x01<<j)) )
			{
				if ( kbdrow_state & (0x01<<j) )
				{
					data &= ~( 0x01 << i);
				}
			}
		}
	}

	#ifdef DBGMODE
	printf("%s: read pia_u75_portb_r : 0x%.2X\n",machine().describe_context(),data);
	#endif

	return data;
}

WRITE8_MEMBER( squale_state::pia_u75_portb_w )
{
	// U75 PIA Port B : Keyboard column input
	#ifdef DBGMODE
	printf("%s: write pia_u75_portb_w : 0x%.2X\n",machine().describe_context(),data);
	#endif
	return;
}

/***********************************
*      AY-8910 I/O Handlers        *
* (Joysticks, Ctrl/Shift keys,...) *
************************************/

READ8_MEMBER( squale_state::ay_portb_r )
{
	// AY-8910 Port B : Joystick 2, Shift, Shift Lock, Ctrl Keys
	// B7 : Joystick 2 - Fire
	// B6 : Keyboard   - Control
	// B5 : Keyboard   - Shift
	// B4 : Keyboard   - Shift Lock
	// B3 : Joystick 2 - Up
	// B2 : Joystick 2 - Down
	// B1 : Joystick 2 - Left
	// B0 : Joystick 2 - Right

	UINT8 data;

	data =  ( ioport("ay_keys")->read() ) & 0x70;
	data |= ( ioport("ay_joy_2")->read() ) & 0x8F;

	#ifdef DBGMODE
	printf("%s: read ay_portb_r : 0x%.2X\n",machine().describe_context(),data);
	#endif

	return data;
}

READ8_MEMBER( squale_state::ay_porta_r )
{
	// AY-8910 Port A : Joystick 1, light pen
	// B7 : Joystick 1 - Fire
	// B6 : -
	// B5 : Light pen Int.
	// B4 : -
	// B3 : Joystick 1 - Up
	// B2 : Joystick 1 - Down
	// B1 : Joystick 1 - Left
	// B0 : Joystick 1 - Right

	UINT8 data;

	#ifdef DBGMODE
	printf("%s: read ay_porta_r\n",machine().describe_context());
	#endif

	data =  ( ioport("ay_joy_2")->read() ) & 0x8F;

	return data;
}

WRITE8_MEMBER( squale_state::ay_porta_w )
{
	// AY-8910 Port A : Joystick 1, light pen
	// B7 : Joystick 1 - Fire
	// B6 : -
	// B5 : Light pen Int.
	// B4 : -
	// B3 : Joystick 1 - Up
	// B2 : Joystick 1 - Down
	// B1 : Joystick 1 - Left
	// B0 : Joystick 1 - Right

	#ifdef DBGMODE
	printf("%s: write ay_porta_w : 0x%.2X\n",machine().describe_context(),data);
	#endif
	return;
}

WRITE8_MEMBER( squale_state::ay_portb_w )
{
	// AY-8910 Port B : Joystick 2, Shift, Shift Lock, Ctrl Keys
	// B7 : Joystick 2 - Fire
	// B6 : Keyboard   - Control
	// B5 : Keyboard   - Shift
	// B4 : Keyboard   - Shift Lock
	// B3 : Joystick 2 - Up
	// B2 : Joystick 2 - Down
	// B1 : Joystick 2 - Left
	// B0 : Joystick 2 - Right

	#ifdef DBGMODE
	printf("%s: write ay_portb_w : 0x%.2X\n",machine().describe_context(),data);
	#endif
	return;
}

/***********************************
*      Cartridge I/O Handlers      *
************************************/

READ8_MEMBER( squale_state::pia_u72_porta_r )
{
	// U72 PIA Port A : Cartridge data bus
	UINT8 data;

	#ifdef DBGMODE
	printf("%s: read pia_u72_porta_r\n",machine().describe_context());
	#endif

	if( m_cart_rom && m_cart_rom->bytes() )
		data = m_cart_rom->u8( cart_addr_counter % m_cart_rom->bytes() );
	else
		data = 0xFF;

	return data;
}

WRITE8_MEMBER( squale_state::pia_u72_porta_w )
{
	// U72 PIA Port A : Cartridge data bus

	#ifdef DBGMODE
	printf("%s: write pia_u72_porta_w : 0x%.2X\n",machine().describe_context(),data);
	#endif

	return;
}

WRITE_LINE_MEMBER( squale_state::pia_u72_ca2_w )
{
	// U72 PIA CA2 : Cartridge address control

	#ifdef DBGMODE
	printf("%s: U72 PIA Port CA2 Set to %2x\n", machine().describe_context(),state);
	#endif

	if( state )
	{
		cart_addr_counter_inc_ck = 1;
	}
	else
	{
		// If not in reset state, increment the address counter (U73 & U74) at the falling edge of ca2.
		if( cart_addr_counter_inc_ck && !cart_addr_counter_reset )
		{
			cart_addr_counter++;
		}

		cart_addr_counter_inc_ck = 0;
	}
}

WRITE_LINE_MEMBER( squale_state::pia_u75_cb2_w )
{
	// U75 PIA CB2 : Cartridge address reset

	#ifdef DBGMODE
	printf("%s: U75 PIA Port CB2 Set to %2x\n", machine().describe_context(),state);
	#endif

	if( state )
	{
		// Cartridge address counter (U73 & U74) reset to 0
		cart_addr_counter_reset = 1;
		cart_addr_counter = 0x0000;
	}
	else
	{
		cart_addr_counter_reset = 0;
	}
}

/**********************************
*      Printer I/O Handlers      *
***********************************/

READ8_MEMBER( squale_state::pia_u72_portb_r )
{
	// U72 PIA Port B : Printer data bus

	UINT8 data = 0xFF;

	#ifdef DBGMODE
	printf("%s: read pia_u72_portb_r\n",machine().describe_context());
	#endif

	return data;
}

WRITE8_MEMBER( squale_state::pia_u72_portb_w )
{
	// U72 PIA Port B : Printer data bus

	#ifdef DBGMODE
	printf("%s: write pia_u72_portb_w : 0x%.2X\n",machine().describe_context(),data);
	#endif

	return;
}

WRITE_LINE_MEMBER( squale_state::pia_u72_cb2_w )
{
	// U72 PIA CB2 : Printer Data Strobe line

	#ifdef DBGMODE
	printf("%s: U72 PIA Port CB2 Set to %2x\n", machine().describe_context(),state);
	#endif
}

DEVICE_IMAGE_LOAD_MEMBER( squale_state, squale_cart )
{
	UINT32 size = m_cart->common_get_size("rom");

	if ( ! size || size > 0x10000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return IMAGE_INIT_FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}

TIMER_DEVICE_CALLBACK_MEMBER( squale_state::squale_scanline )
{
	m_ef9365->update_scanline((UINT16)param);
}

static ADDRESS_MAP_START(squale_mem, AS_PROGRAM, 8, squale_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0xefff) AM_RAM
	AM_RANGE(0xf000,0xf00f) AM_DEVREADWRITE("ef9365", ef9365_device, data_r, data_w)
	AM_RANGE(0xf010,0xf01f) AM_WRITE( ctrl_w )
	AM_RANGE(0xf020,0xf02f) AM_READ( video_ram_read_reg1 )
	AM_RANGE(0xf030,0xf03f) AM_READ( video_ram_read_reg2 )
	AM_RANGE(0xf044,0xf047) AM_DEVREADWRITE("pia_u75", pia6821_device, read, write)
	AM_RANGE(0xf048,0xf04b) AM_DEVREADWRITE("pia_u72", pia6821_device, read, write)
	AM_RANGE(0xf050,0xf05f) AM_DEVREADWRITE("ef6850", acia6850_device, data_r, data_w)
	AM_RANGE(0xf060,0xf06f) AM_DEVREADWRITE("ay8910", ay8910_device, data_r, address_data_w)
	AM_RANGE(0xf080,0xf083) AM_DEVREADWRITE("wd1770", wd1770_t, read, write)
	AM_RANGE(0xf08a,0xf08a) AM_READWRITE( fdc_sel0_r, fdc_sel0_w )
	AM_RANGE(0xf08b,0xf08b) AM_READWRITE( fdc_sel1_r, fdc_sel1_w )
	AM_RANGE(0xf100,0xffff) AM_ROMBANK("rom_bank");

ADDRESS_MAP_END

static ADDRESS_MAP_START( squale_io, AS_IO, 8, squale_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( squale )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BackS  Delete") PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('-')

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('[')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(']')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LF") PORT_CODE(KEYCODE_RALT) PORT_CHAR(10)

	PORT_START("ay_keys")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Lck") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("ay_joy_1")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_CODE(KEYCODE_8_PAD) PORT_CODE(JOYCODE_Y_UP_SWITCH)    PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_CODE(KEYCODE_2_PAD) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_CODE(KEYCODE_4_PAD) PORT_CODE(JOYCODE_X_LEFT_SWITCH)  PORT_PLAYER(1)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON1)        PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1)        PORT_PLAYER(1)

	PORT_START("ay_joy_2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_CODE(KEYCODE_8_PAD) PORT_CODE(JOYCODE_Y_UP_SWITCH)    PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_CODE(KEYCODE_2_PAD) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_CODE(KEYCODE_4_PAD) PORT_CODE(JOYCODE_X_LEFT_SWITCH)  PORT_PLAYER(2)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON1)        PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1)        PORT_PLAYER(2)

INPUT_PORTS_END

static SLOT_INTERFACE_START( squale_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

void squale_state::machine_start()
{
	int i;
	std::string region_tag;

	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	fdc_sel0 = 0x00;
	fdc_sel1 = 0x00;
	m_floppy = NULL;

	cart_addr_counter_reset = 0;
	cart_addr_counter = 0x0000;

	membank("rom_bank")->configure_entry(0, memregion("maincpu")->base() + 0x100);
	membank("rom_bank")->configure_entry(1, memregion("maincpu")->base() + 0x1100);
	membank("rom_bank")->set_entry( 0 );

	// Generate Squale hardware palette
	for(i=0;i<8;i++)
	{
		m_ef9365->set_color_entry(i,(((i&4)>>2)^1) * 255,(((i&2)>>1)^1) * 255, ((i&1)^1) * 255 );
	}

	for(i=0;i<8;i++)
	{
		m_ef9365->set_color_entry(i + 8,(((i&4)>>2)^1) * 127,(((i&2)>>1)^1) * 127, ((i&1)^1) * 127 );
	}
}

void squale_state::machine_reset()
{
}

static MACHINE_CONFIG_START( squale, squale_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6809E, CPU_CLOCK) // 12/2015 : Should be set to M6809 but it actually have the wrong clock divisor (1 instead of 4) and working 4 times too fast...
	MCFG_CPU_PROGRAM_MAP(squale_mem)
	MCFG_CPU_IO_MAP(squale_io)

	/* Cartridge pia */
	MCFG_DEVICE_ADD("pia_u72", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(squale_state, pia_u72_porta_r))
	MCFG_PIA_READPB_HANDLER(READ8(squale_state, pia_u72_portb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(squale_state, pia_u72_porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(squale_state, pia_u72_portb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(squale_state, pia_u72_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(squale_state, pia_u72_cb2_w))

	/* Keyboard pia */
	MCFG_DEVICE_ADD("pia_u75", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(squale_state, pia_u75_porta_r))
	MCFG_PIA_READPB_HANDLER(READ8(squale_state, pia_u75_portb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(squale_state, pia_u75_porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(squale_state, pia_u75_portb_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(squale_state, pia_u75_cb2_w))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, AY_CLOCK)
	// TODO : Add port I/O handler
	MCFG_AY8910_PORT_A_READ_CB(READ8(squale_state, ay_porta_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(squale_state, ay_portb_r))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(squale_state, ay_porta_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(squale_state, ay_portb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DEVICE_ADD ("ef6850", ACIA6850, 0)

	/* screen */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DEVICE("ef9365", ef9365_device, screen_update)

	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)
	MCFG_PALETTE_ADD("palette", 16)

	MCFG_DEVICE_ADD("ef9365", EF9365, VIDEO_CLOCK)
	MCFG_EF936X_PALETTE("palette")
	MCFG_EF936X_BITPLANES_CNT(4);
	MCFG_EF936X_DISPLAYMODE(EF936X_256x256_DISPLAY_MODE);
	MCFG_TIMER_DRIVER_ADD_SCANLINE("squale_sl", squale_state, squale_scanline, "screen", 0, 10)

	/* Floppy */
	MCFG_WD1770_ADD("wd1770", XTAL_8MHz )
	MCFG_FLOPPY_DRIVE_ADD("wd1770:0", squale_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd1770:1", squale_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_SOFTWARE_LIST_ADD("flop525_list", "squale")

	/* Cartridge slot */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_linear_slot, "squale_cart")
	MCFG_GENERIC_MANDATORY
	MCFG_GENERIC_LOAD(squale_state, squale_cart)

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( squale )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v201")

	ROM_SYSTEM_BIOS(0, "v201", "Version 2.1")
	ROMX_LOAD( "sqmon_2r1.bin", 0x0000, 0x2000, CRC(ed57c707) SHA1(c8bd33a6fb07fe7f881f2605ad867b7e82366bfc), ROM_BIOS(1) )

	// place ROM v1.2 signature here.

	ROM_REGION( 0x1E0, "ef9365", 0 )
	ROM_LOAD( "charset_ef9365.rom", 0x0000, 0x01E0, CRC(8d3053be) SHA1(0f9a64d217a0f7f04ee0720d49c5b680ad0ae359) )
ROM_END

/* Driver */

/*    YEAR   NAME   PARENT  COMPAT   MACHINE    INPUT  CLASS           INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1984, squale, 0,      0,       squale,    squale,driver_device,   0,     "Apollo 7", "Squale",    MACHINE_TYPE_COMPUTER )
