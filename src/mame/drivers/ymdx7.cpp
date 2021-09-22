// license:BSD-3-Clause
// copyright-holders:David Viens
/***************************************************************************

  Driver for Yamaha DX7
  Copyright 2011-2021 David Viens

	Main CPU is a HD63B03RP
	Sub  CPU is a HD6805S1P-A33 (ROM on chip) 

	The FM sound system is done by two discreet chips:
	OPS(YM2128) and EGS(YM2129)

	///////////////// ////////////////////////////////////////////////////////
	Main CPU PCB fixed pins:

	PIN02:  XTAL : NC
	PIN03:  EXTAL: 4.713250MHz (internally divided by 4 to get 1.178312MHz)
	PIN04: !NMI  : 1 (4.7K pull-up)
	PIN05: !IRQ  : 1 (4.7K pull-up) but... linked to Sub CPU

	PIN07: !STBY : 1 (4.7K pull-up)
	/////////////////////////////////////////////////////////////////////////
	RAM access is done through logic on the PCB when
	bit 6 of port 5 (RAME) is low.

	PORT1:direction 0 (all bits read only!)
	P10 to P17 : 8bits that are mapped to Sub CPU's shared Address/Data line

	PORT2 direction 0x11 (XXX10001)
	P20 : write to   TC4053's X (pin14) (SUB CPU BUSY)
	P21 : read  from TC4053's Y (pin15) (SUB CPU handshake)
	P22 : read  from TC4053's Z (pin 4) (250KHz Clock)
	P23 : read  from MIDI IN
	P24 : write to   MIDI OUT
****************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/m6800/m6801.h"
#include "cpu/m6805/m68705.h"
#include "machine/adc0808.h"
#include "machine/nvram.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
//#include "speaker.h"

constexpr auto DX7CLOCK    = 9'426'500;
constexpr auto NUM_ANALOG  = 7;
constexpr auto NUM_INITIAL = (2*NUM_ANALOG);

#define TRACE printf

//initial analogue values of the previously HLE'ed sub CPU 
constexpr uint8_t g_init_vals[NUM_INITIAL]={
	//todo all notes off?
	0x90,0x40,//Data Entry should be centered?
	0x91,0x40,
	0x92,0x40,
	0x93,0,
	0x94,0,
	0x95,0,
	0x96,0x60 //getting rid of "Change Battery"
};


class yamaha_dx7_state : public driver_device
{
public:
	yamaha_dx7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_lcdc(*this, "hd44780")		
		, m_adc(*this, "adc")
	{
	}

	void dx7(machine_config &config);

protected:
	virtual void machine_start() override;
	
private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	void palette_init(palette_device &palette);

	uint8_t dx7_8255_r(offs_t offset);
	
	void dx7_8255_w(offs_t offset, uint8_t data);	
	
	void dx7_led_w (offs_t offset, uint8_t data){
		//output_set_digit_value(offset&1,(~data) &0xFF);		
	}
	
	//since HD6303R is not fully emulated we have to bind some unused ports FIXME
	uint8_t dx7_unmapped_r(offs_t offset){return 0xFF;}
	void    dx7_unmapped_w(offs_t address, uint8_t data){};

	uint8_t dx7_p1_r(offs_t offset);
	uint8_t dx7_p2_r(offs_t offset);	
	void    dx7_p2_w(offs_t offset, uint8_t data);	

	void dx7_acept_w(offs_t offset, uint8_t data);	
	
	void dx7_ops_w  (offs_t offset, uint8_t data){
	    //TRACE("OPS W: %02X=%02X\n",offset,data);
	};
	
	void dx7_egs_w  (offs_t offset, uint8_t data){
	    //TRACE("EGS W: %02X=%02X\n",offset,data);
	};
	
	void dx7_dac_w  (offs_t offset, uint8_t data){};//TODO

	void main_map(address_map &map);
	void sub_map (address_map &map);
		
	required_device<hd6303r_cpu_device> m_maincpu;
	required_device<hd6805s1_device>    m_subcpu;
	required_device<hd44780_device>     m_lcdc;	
	required_device<adc0808_device>     m_adc;
	
	int m_clk=0;	

	uint8_t m_PORT1=0;
	uint8_t m_PORT2=0;
	uint8_t m_irq0=0;//init in start
	//past values of that port
	std::array<uint8_t,4> m_8255 = {};
	//YM_EGS_OPS_EMU _ym;

	//sub cpu message queue
	uint8_t m_q[256];//init in start
	uint8_t m_qrpos=0;
	uint8_t m_qwpos = 0;

	uint32_t m_oNUMBERS=0;
	uint8_t  m_oNOYES=0;	
	uint8_t  m_oFUNCTION=0;
	uint8_t  m_oKEYBOARD=0;	
	int m_acept=0;

	void sub_cpu_hle();	
};

void yamaha_dx7_state::machine_start()
{	
	m_irq0=1;
	
	memcpy(m_q,	g_init_vals,NUM_INITIAL);
	m_qwpos = NUM_INITIAL;
}

void yamaha_dx7_state::dx7_acept_w(offs_t address, uint8_t data)
{
	m_acept++;
	//TRACE("ACEPT reg:0x%x = 0x%x \n",offset, data);
	m_irq0 = 1; 
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}

uint8_t yamaha_dx7_state::dx7_p1_r(offs_t)
{
	//TRACE("reading PORT1\n");
	return m_PORT1;			
}

uint8_t yamaha_dx7_state::dx7_p2_r(offs_t)
{
	//TRACE("reading PORT2\n");
	uint8_t temp = 0; //P0 is WRITE! does the CPU re-reads its OWN data? can it?

	if(m_irq0) temp = temp | 0x02; //P21 is connected to !IRQ line
	if(m_clk)  temp = temp | 0x04; //P22 is connected to 250Khz clock
	
	//todo MIDI INPUT

	return temp;	
}

void yamaha_dx7_state::dx7_p2_w(offs_t offset, uint8_t data)
{
	//TRACE("W PORT2 0x%x\n", data);
	m_PORT2=data;
}

void yamaha_dx7_state::dx7_8255_w(offs_t offset, uint8_t data)
{
 	switch(offset){
		case 0:
			//PORTA write
		break;

		case 1:{
			//TRACE("PORTB  0x%x\n",data);

      //PORTB write (LCD handshake)
			// LCD Should Initially Display:
			//   "*  YAMAHA DX7  *"
			//   "*  SYNTHESIZER *"
			
			//the only way to make this work is to invert LCD_E && LCD_RW 
			//with regards to the schems, however, the schems MATCH with my machine... don't get it
			//is portB pin-out different on some 8255's?
			#define LCD_E  0x02 
			#define LCD_RW 0x04
			#define LCD_RS 0x01
			
 			if(m_8255[3] & 0x10)
				return; //firmware logic error! PortA is in READ MODE, old A port value would be random!
			
			if( (data & LCD_E) &&  !(data & LCD_RW) ){ 
				if(data & LCD_RS)				
					m_lcdc->write(1,m_8255[0]);
				else				
					m_lcdc->write(0,m_8255[0]);						
			}
		}break;

		case 2:
			//PORTC write
			TRACE("PORTC WRITE ERROR .. READ ONLY 0x%x\n",data);
		break;

		case 3:{
			//8255A MODE from ROM we get two values:
			//(0b1001100X) ACTIVE, GRP1MODE=0, PortA=INPUT, PortC(up)=INPUT,GRP2MODE=0,PORTB=OUTPUT
			//uint8_t mode=data;
			//but difference:
			//0x99 (10011001) PortC (low) = input
			//0x89 (10001001) PortC (low) = output
			//TRACE("8255W3:0x%x = 0x%x\n",offset, data);
		}break;

		default:
			TRACE("UNKW:0x%x = 0x%x \n",offset, data);
		break;
	}

	m_8255[offset] = data;
}


uint8_t yamaha_dx7_state::dx7_8255_r(offs_t offset)
{
	switch(offset){
		case 0x0:
		case 0x1:
			TRACE("8255R:0x%x\n",offset);
			//LCD BUSY FLAG ADRS counter
			return 0;//m_lcdc->read(space, offset &1);
				//return LCDReadData(&lcd);
		break;

		case 0x2:
		 	//SUSTAIN and Portamento (active LOW)
			return 3; //NO signal TODO
		break;

		default:{
			TRACE("8255R ERROR:0x%x\n",offset);
		}break;
	}

	return 0;
}


HD44780_PIXEL_UPDATE(yamaha_dx7_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

void yamaha_dx7_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t( 92,  83,  88));
}

/*
void yamaha_dx7_state::cartridge_bank_w(u8 data)
{
}
*/

void yamaha_dx7_state::main_map(address_map &map)
{
	//decoded from IC23 74LS138 using A11 to A15 (2kb blocks)
	//Y0 (0x0000) NC
	//Y1 (0x0800) NC
	//Y2 (0x1000) RAM1
	//Y3 (0x1800) RAM2
	//Y4 (0x2000) RAM3
	//Y5 (0x2800) (another 74LS138), see below
	//Y6 (0x3000) EGS
	//Y7 (0x3800) NC
	
	map(0x0000, 0x001f).m(m_maincpu, FUNC(m6801_cpu_device::m6801_io));
	map(0x0080, 0x00ff).ram();/* HR6303R built in RAM */

	map(0x1000, 0x17ff).ram().share("ram1");/* 2kb RAM1 IC19 M5M118P (Voice Memory part1) */
	map(0x1800, 0x1fff).ram().share("ram2");/* 2kb RAM2 IC20 M5M118P (Voice Memory part2) */
	map(0x2000, 0x27ff).ram().share("ram3");/* 2kb RAM3 IC21 M5M118P (Working Area)       */
	
	//the 0x2800 to 0x2FFF range is then resplit by IC24(another 74LS138)
	// A3 A2 A1 A0
	//  0  0  0  X:  Y0 : IC12 8255 (LCD,porta/sustain)
	//  0  0  1  X:  Y1 : IC12 8255 (LCD,porta/sustain)
	//  0  1  0  X:  Y2 : OPS
	//  0  1  1  X:  Y3 : NC
	//  1  0  0  X:  Y4 : NC
	//  1  0  1  X:  Y5 : DAC
	//  1  1  0  X:  Y6 : ACEPT (IC5)
	//  1  1  1  X:  Y7 : LED DRIVERs

	map(0x2800, 0x2803).rw(FUNC(yamaha_dx7_state::dx7_8255_r), FUNC(yamaha_dx7_state::dx7_8255_w));	
	map(0x2804, 0x2805).w(FUNC(yamaha_dx7_state::dx7_ops_w));
	map(0x280A, 0x280B).w(FUNC(yamaha_dx7_state::dx7_dac_w));
	map(0x280C, 0x280D).w(FUNC(yamaha_dx7_state::dx7_acept_w));
	map(0x280E, 0x280F).w(FUNC(yamaha_dx7_state::dx7_led_w));

	//EGS (YM2129)
	map(0x3000, 0x37ff).w(FUNC(yamaha_dx7_state::dx7_egs_w));

  //map(0x4000, 0x4fff).rom().region("cartridge", 0); //or RAM!
	map(0xC000, 0xffff).rom().region("program", 0);
}

void yamaha_dx7_state::sub_map(address_map &map)
{
	map(0x0000, 0x07FF).rom().region("subcpu", 0); //hhhm
}

static INPUT_PORTS_START(dx7)
INPUT_PORTS_END

void yamaha_dx7_state::dx7(machine_config &config)
{
	HD6303R(config, m_maincpu, DX7CLOCK/2); // HD63B03RP
	m_maincpu->set_addrmap(AS_PROGRAM, &yamaha_dx7_state::main_map);
	
	//making somme unmapped binds because HD6303R's specific registers does not contain P3/4
	m_maincpu->in_p1_cb().set(FUNC(yamaha_dx7_state::dx7_p1_r));
	m_maincpu->in_p2_cb().set(FUNC(yamaha_dx7_state::dx7_p2_r));
	m_maincpu->in_p3_cb().set(FUNC(yamaha_dx7_state::dx7_unmapped_r));
	m_maincpu->in_p4_cb().set(FUNC(yamaha_dx7_state::dx7_unmapped_r));

	m_maincpu->out_p1_cb().set(FUNC(yamaha_dx7_state::dx7_unmapped_w));
	m_maincpu->out_p2_cb().set(FUNC(yamaha_dx7_state::dx7_p2_w));
	m_maincpu->out_p3_cb().set(FUNC(yamaha_dx7_state::dx7_unmapped_w));
	m_maincpu->out_p4_cb().set(FUNC(yamaha_dx7_state::dx7_unmapped_w));

	HD6805S1(config, m_subcpu, 4_MHz_XTAL); // HD6805S1P-A33 
	m_subcpu->set_addrmap(AS_PROGRAM, &yamaha_dx7_state::sub_map);
	
	// mcu ports TODO
	/*
	m_subcpu->porta_w().set(FUNC(ncd68k_state::mcu_porta_w));
	m_subcpu->portb_w().set(FUNC(ncd68k_state::mcu_portb_w));
	m_subcpu->portc_w().set(FUNC(ncd68k_state::mcu_portc_w));
	m_subcpu->porta_r().set(FUNC(ncd68k_state::mcu_porta_r));
	m_subcpu->portb_r().set(FUNC(ncd68k_state::mcu_portb_r));
	*/

	NVRAM(config, "ram1", nvram_device::DEFAULT_ALL_0);/* 2kb RAM1 IC19 M5M118P (Voice Memory part1) */
	NVRAM(config, "ram2", nvram_device::DEFAULT_ALL_0);/* 2kb RAM2 IC20 M5M118P (Voice Memory part2) */
	NVRAM(config, "ram3", nvram_device::DEFAULT_ALL_0);/* 2kb RAM3 IC21 M5M118P (Working Area)       */
	
	M58990(config, m_adc, 8_MHz_XTAL / 16); // M58990P-1; divider not verified (actually clocked by P26 output of sub CPU)

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_color(rgb_t::green());
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(yamaha_dx7_state::palette_init), 2);

	HD44780(config, m_lcdc, 0);
	m_lcdc->set_lcd_size(2, 16);
	m_lcdc->set_pixel_update_cb(FUNC(yamaha_dx7_state::lcd_pixel_update));
}

ROM_START(dx7)
	
	//From service manual there are two rom revisions mentioned:
	//one that's split into two 2764's:
	//iG 10570(0)  uPD 2764-2    ROM1 DX7~#2600
	//iG 10575(0)  uPD 2764-2    ROM2 DX7~#2600 
	//and a single 27128:
	//iG 11461(0)  HN 613128PC86 ROM DX7#2661~
	
	//V1.?
	//ROM_LOAD( "ig11461.ic14", 0xC000, 0x4000, CRC(fb50c62b) SHA1(21c4995d65d0ae6f4868ac89bf7a4ae81dc3bd31) )
	
	//V1.? 
	//ROM_LOAD( "ig11464.ic14", 0xC000, 0x4000, CRC(126c5a98) SHA1(ce4df31878dda9ec27b31c7bc172f16419264b90) )
		
	//V1.? ig11467 (references online)
	//V1.? ig11468 (references online)
	
	//V1.8 24-Oct-85
	ROM_REGION(0x4000, "program", 0)	
	ROM_LOAD( "ig11469.ic14", 0x0000, 0x4000, CRC(6cbb0865) SHA1(715dbb8e96a4df2a7f096b368334a7654860bb26) )
	

	ROM_REGION(0x0800, "subcpu", 0)
	ROM_LOAD("hd6805s1p-a33.ic13", 0x0000, 0x800, CRC(ac1d84b3) SHA1(ee0ebb118dd0d282d7c195d3b246a0094b2cb6ad))
ROM_END

SYST(1983, dx7, 0, 0, dx7, dx7, yamaha_dx7_state, empty_init, "Yamaha", "DX7 Digital Programmable Algorithm Synthesizer", MACHINE_IS_SKELETON)
