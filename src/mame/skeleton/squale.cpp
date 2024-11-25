// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jean-Francois DEL NERO
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


LIST OF MONITOR COMMANDS (all addresses must be 4 characters)
C         Load a file from tape
D         Load DOS from floppy
E         Load a file from tape and run it
G a       Go (a = address)
M a       Memory dump (a = start address) Use the arrow keys to move around.
R         Reset then load DOS from floppy
S a b c   Save memory to tape  (a = start, b = end, c = exec)

KEYBOARD
- When started, the Lock is engaged, so numbers appear as symbols. Hitting
  Shift will unlock this and return the keyboard to normal.
- To use the natural keyboard, hit > (or some other shifted character),
  then proceed as normal.
- To paste, use the emulated keyboard and hit Shift, then do the paste.
- The monitor command-line cannot handle the arrow keys or Del correctly.
- It is thought that the BASIC cartridge has better keyboard handling.

TODO
- Cassette
- ACIA


****************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6809/m6809.h"
#include "imagedev/floppy.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "sound/ay8910.h"
#include "video/ef9365.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "utf8.h"


namespace {

#define MAIN_CLOCK           14_MHz_XTAL
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
		, m_floppy(nullptr)
		, m_cart(*this, "cartslot")
	{ }

	void squale(machine_config &config);

private:
	void ctrl_w(uint8_t data);
	uint8_t video_ram_read_reg1();
	uint8_t video_ram_read_reg2();
	void fdc_sel0_w(uint8_t data);
	uint8_t fdc_sel0_r();
	void fdc_sel1_w(uint8_t data);
	uint8_t fdc_sel1_r();
	uint8_t pia_u72_porta_r();
	uint8_t pia_u72_portb_r();
	uint8_t pia_u75_porta_r();
	uint8_t pia_u75_portb_r();
	void pia_u72_porta_w(uint8_t data);
	void pia_u72_portb_w(uint8_t data);
	void pia_u75_porta_w(uint8_t data);
	void pia_u75_portb_w(uint8_t data);

	uint8_t ay_porta_r();
	uint8_t ay_portb_r();
	void ay_porta_w(uint8_t data);
	void ay_portb_w(uint8_t data);

	void pia_u72_ca2_w(int state);
	void pia_u72_cb2_w(int state);

	void pia_u75_cb2_w(int state);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( cart_load );
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t keyboard_line = 0U;
	uint8_t fdc_sel0 = 0U;
	uint8_t fdc_sel1 = 0U;

	uint8_t  cart_addr_counter_inc_ck = 0U;
	uint8_t  cart_addr_counter_reset = 0U;
	uint16_t cart_addr_counter = 0U;

	TIMER_DEVICE_CALLBACK_MEMBER(squale_scanline);

	void squale_mem(address_map &map) ATTR_COLD;

	required_device<acia6850_device> m_acia;
	required_device<ay8910_device> m_ay8910;
	required_device<pia6821_device> m_pia_u72;
	required_device<pia6821_device> m_pia_u75;
	required_device<ef9365_device> m_ef9365;
	required_device<cpu_device> m_maincpu;
	required_device<wd1770_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;
	required_device<generic_slot_device> m_cart;

	memory_region *m_cart_rom = nullptr;
};

/*****************************************
* Machine control register I/O Handlers  *
******************************************/

void squale_state::ctrl_w(uint8_t data)
{
	#ifdef DBGMODE
	printf("write ctrl reg : 0x%X\n",data);
	#endif

	membank("rom_bank")->set_entry(data >> 7);

	m_ef9365->set_color_filler(data & 0xF);
}

uint8_t  squale_state::video_ram_read_reg1()
{
	uint8_t data;
	int p;

	//D7             D0
	//I2R2G2B2 I3R3G3B3

	data = 0x00;

	for(p = 0; p < 4 ; p++)
	{
		if( m_ef9365->get_last_readback_word(p, nullptr) & 8 )
		{
			data |= (0x01 << p);
		}
	}

	data = data << 4;

	for(p = 0; p < 4 ; p++)
	{
		if( m_ef9365->get_last_readback_word(p, nullptr) & 4 )
		{
			data |= (0x01 << p);
		}
	}

	#ifdef DBGMODE
	printf("read video_ram_read_reg1 reg : 0x%X\n",data);
	#endif

	return data;
}

uint8_t squale_state::video_ram_read_reg2()
{
	uint8_t data;
	int p;

	//D7             D0
	//I0R0G0B0 I1R1G1B1

	data = 0x00;

	for(p = 0; p < 4 ; p++)
	{
		if( m_ef9365->get_last_readback_word(p, nullptr) & 2 )
		{
			data |= (0x01 << p);
		}
	}

	data = data << 4;

	for(p = 0; p < 4 ; p++)
	{
		if( m_ef9365->get_last_readback_word(p, nullptr) & 1 )
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

void squale_state::fdc_sel0_w(uint8_t data)
{
	floppy_image_device *floppy = 0;

	#ifdef DBGMODE
	printf("%s: write fdc_sel0_w reg : 0x%X\n",machine().describe_context().c_str(),data);
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

void squale_state::fdc_sel1_w(uint8_t data)
{
	#ifdef DBGMODE
	printf("%s: write fdc_sel1_w reg : 0x%X\n",machine().describe_context().c_str(),data);
	#endif

	fdc_sel1 = data;
}

uint8_t squale_state::fdc_sel0_r()
{
	uint8_t data;

	data = fdc_sel0;

	#ifdef DBGMODE
	printf("%s: read fdc_sel0_r 0x%.2X\n",machine().describe_context().c_str(),data);
	#endif

	return data;
}

uint8_t squale_state::fdc_sel1_r()
{
	uint8_t data;

	data = fdc_sel1;

	#ifdef DBGMODE
	printf("%s: read fdc_sel1_r 0x%.2X\n",machine().describe_context().c_str(),data);
	#endif

	return data;
}

/**********************************
*      Keyboard I/O Handlers      *
***********************************/

void squale_state::pia_u75_porta_w(uint8_t data)
{
	// U75 PIA Port A : Keyboard rows output
	#ifdef DBGMODE
	printf("%s: write pia_u75_porta_w : 0x%.2X\n",machine().describe_context().c_str(),data);
	#endif
	keyboard_line = data;
	return;
}

uint8_t squale_state::pia_u75_porta_r()
{
	// U75 PIA Port A : Keyboard rows output
	uint8_t data;

	#ifdef DBGMODE
	printf("%s: read pia_u75_porta_r\n",machine().describe_context().c_str());
	#endif

	data = keyboard_line;
	return data;
}

uint8_t squale_state::pia_u75_portb_r()
{
	// U75 PIA Port B : Keyboard column input
	char kbdrow[3];
	unsigned char kbdrow_state;
	uint8_t data = 0xFF;

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
	printf("%s: read pia_u75_portb_r : 0x%.2X\n",machine().describe_context().c_str(),data);
	#endif

	return data;
}

void squale_state::pia_u75_portb_w(uint8_t data)
{
	// U75 PIA Port B : Keyboard column input
	#ifdef DBGMODE
	printf("%s: write pia_u75_portb_w : 0x%.2X\n",machine().describe_context().c_str(),data);
	#endif
	return;
}

/***********************************
*      AY-8910 I/O Handlers        *
* (Joysticks, Ctrl/Shift keys,...) *
************************************/

uint8_t squale_state::ay_portb_r()
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

	uint8_t data;

	data =  ( ioport("ay_keys")->read() ) & 0x70;
	data |= ( ioport("ay_joy_2")->read() ) & 0x8F;

	#ifdef DBGMODE
	printf("%s: read ay_portb_r : 0x%.2X\n",machine().describe_context().c_str(),data);
	#endif

	return data;
}

uint8_t squale_state::ay_porta_r()
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

	uint8_t data;

	#ifdef DBGMODE
	printf("%s: read ay_porta_r\n",machine().describe_context().c_str());
	#endif

	data =  ( ioport("ay_joy_2")->read() ) & 0x8F;

	return data;
}

void squale_state::ay_porta_w(uint8_t data)
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
	printf("%s: write ay_porta_w : 0x%.2X\n",machine().describe_context().c_str(),data);
	#endif
	return;
}

void squale_state::ay_portb_w(uint8_t data)
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
	printf("%s: write ay_portb_w : 0x%.2X\n",machine().describe_context().c_str(),data);
	#endif
	return;
}

/***********************************
*      Cartridge I/O Handlers      *
************************************/

uint8_t squale_state::pia_u72_porta_r()
{
	// U72 PIA Port A : Cartridge data bus
	uint8_t data;

	#ifdef DBGMODE
	printf("%s: read pia_u72_porta_r\n",machine().describe_context().c_str());
	#endif

	if( m_cart_rom && m_cart_rom->bytes() )
		data = m_cart_rom->as_u8( cart_addr_counter % m_cart_rom->bytes() );
	else
		data = 0xFF;

	return data;
}

void squale_state::pia_u72_porta_w(uint8_t data)
{
	// U72 PIA Port A : Cartridge data bus

	#ifdef DBGMODE
	printf("%s: write pia_u72_porta_w : 0x%.2X\n",machine().describe_context().c_str(),data);
	#endif

	return;
}

void squale_state::pia_u72_ca2_w(int state)
{
	// U72 PIA CA2 : Cartridge address control

	#ifdef DBGMODE
	printf("%s: U72 PIA Port CA2 Set to %2x\n", machine().describe_context().c_str(),state);
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

void squale_state::pia_u75_cb2_w(int state)
{
	// U75 PIA CB2 : Cartridge address reset

	#ifdef DBGMODE
	printf("%s: U75 PIA Port CB2 Set to %2x\n", machine().describe_context().c_str(),state);
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

uint8_t squale_state::pia_u72_portb_r()
{
	// U72 PIA Port B : Printer data bus

	uint8_t data = 0xFF;

	#ifdef DBGMODE
	printf("%s: read pia_u72_portb_r\n",machine().describe_context().c_str());
	#endif

	return data;
}

void squale_state::pia_u72_portb_w(uint8_t data)
{
	// U72 PIA Port B : Printer data bus

	#ifdef DBGMODE
	printf("%s: write pia_u72_portb_w : 0x%.2X\n",machine().describe_context().c_str(),data);
	#endif

	return;
}

void squale_state::pia_u72_cb2_w(int state)
{
	// U72 PIA CB2 : Printer Data Strobe line

	#ifdef DBGMODE
	printf("%s: U72 PIA Port CB2 Set to %2x\n", machine().describe_context().c_str(),state);
	#endif
}

DEVICE_IMAGE_LOAD_MEMBER( squale_state::cart_load )
{
	uint32_t const size = m_cart->common_get_size("rom");

	if (!size || size > 0x1'0000)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be more than 64K)");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

TIMER_DEVICE_CALLBACK_MEMBER( squale_state::squale_scanline )
{
	m_ef9365->update_scanline((uint16_t)param);
}

void squale_state::squale_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xefff).ram();
	map(0xf000, 0xf00f).rw(m_ef9365, FUNC(ef9365_device::data_r), FUNC(ef9365_device::data_w));
	map(0xf010, 0xf01f).w(FUNC(squale_state::ctrl_w));
	map(0xf020, 0xf02f).r(FUNC(squale_state::video_ram_read_reg1));
	map(0xf030, 0xf03f).r(FUNC(squale_state::video_ram_read_reg2));
	map(0xf044, 0xf047).rw(m_pia_u75, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xf048, 0xf04b).rw(m_pia_u72, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xf050, 0xf05f).rw(m_acia, FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w));
	map(0xf060, 0xf06f).rw(m_ay8910, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0xf080, 0xf083).rw(m_fdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write));
	map(0xf08a, 0xf08a).rw(FUNC(squale_state::fdc_sel0_r), FUNC(squale_state::fdc_sel0_w));
	map(0xf08b, 0xf08b).rw(FUNC(squale_state::fdc_sel1_r), FUNC(squale_state::fdc_sel1_w));
	map(0xf100, 0xffff).bankr("rom_bank");

}

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
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))  // 0x0b

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(DEL)) // 0x7f
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ret") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(39)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) // 0x08

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) // 0x09

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) // 0x0a

	PORT_START("ay_keys")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Lock") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
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

static void squale_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void squale_state::machine_start()
{
	int i;
	std::string region_tag;

	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	fdc_sel0 = 0x00;
	fdc_sel1 = 0x00;
	m_floppy = nullptr;

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

void squale_state::squale(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &squale_state::squale_mem);

	/* Cartridge pia */
	PIA6821(config, m_pia_u72);
	m_pia_u72->readpa_handler().set(FUNC(squale_state::pia_u72_porta_r));
	m_pia_u72->readpb_handler().set(FUNC(squale_state::pia_u72_portb_r));
	m_pia_u72->writepa_handler().set(FUNC(squale_state::pia_u72_porta_w));
	m_pia_u72->writepb_handler().set(FUNC(squale_state::pia_u72_portb_w));
	m_pia_u72->ca2_handler().set(FUNC(squale_state::pia_u72_ca2_w));
	m_pia_u72->cb2_handler().set(FUNC(squale_state::pia_u72_cb2_w));

	/* Keyboard pia */
	PIA6821(config, m_pia_u75);
	m_pia_u75->readpa_handler().set(FUNC(squale_state::pia_u75_porta_r));
	m_pia_u75->readpb_handler().set(FUNC(squale_state::pia_u75_portb_r));
	m_pia_u75->writepa_handler().set(FUNC(squale_state::pia_u75_porta_w));
	m_pia_u75->writepb_handler().set(FUNC(squale_state::pia_u75_portb_w));
	m_pia_u75->cb2_handler().set(FUNC(squale_state::pia_u75_cb2_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay8910, AY_CLOCK);
	// TODO : Add port I/O handler
	m_ay8910->port_a_read_callback().set(FUNC(squale_state::ay_porta_r));
	m_ay8910->port_b_read_callback().set(FUNC(squale_state::ay_portb_r));
	m_ay8910->port_a_write_callback().set(FUNC(squale_state::ay_porta_w));
	m_ay8910->port_b_write_callback().set(FUNC(squale_state::ay_portb_w));
	m_ay8910->add_route(ALL_OUTPUTS, "mono", 0.50);

	ACIA6850(config, m_acia, 0);

	/* screen */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_screen_update("ef9365", FUNC(ef9365_device::screen_update));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 0, 256-1);

	PALETTE(config, "palette").set_entries(16);

	EF9365(config, m_ef9365, VIDEO_CLOCK);
	m_ef9365->set_palette_tag("palette");
	m_ef9365->set_nb_bitplanes(4);
	m_ef9365->set_display_mode(ef9365_device::DISPLAY_MODE_256x256);
	m_ef9365->irq_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	TIMER(config, "squale_sl").configure_scanline(FUNC(squale_state::squale_scanline), "screen", 0, 10);

	/* Floppy */
	WD1770(config, m_fdc, 8_MHz_XTAL);
	FLOPPY_CONNECTOR(config, "wd1770:0", squale_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "wd1770:1", squale_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	//SOFTWARE_LIST(config, "flop525_list").set_original("squale_flop");   // list does not exist

	/* Cartridge slot */
	GENERIC_CARTSLOT(config, "cartslot", generic_linear_slot, "squale_cart").set_device_load(FUNC(squale_state::cart_load));
	SOFTWARE_LIST(config, "cart_list").set_original("squale_cart");
}

/* ROM definition */
ROM_START( squale )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v201")

	ROM_SYSTEM_BIOS(0, "v201", "Version 2.1")
	ROMX_LOAD( "sqmon_2r1.bin", 0x0000, 0x2000, CRC(ed57c707) SHA1(c8bd33a6fb07fe7f881f2605ad867b7e82366bfc), ROM_BIOS(0) )

	// place ROM v1.2 signature here.
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY     FULLNAME  FLAGS
COMP( 1984, squale, 0,      0,      squale,  squale, squale_state, empty_init, "Apollo 7", "Squale", 0 )
