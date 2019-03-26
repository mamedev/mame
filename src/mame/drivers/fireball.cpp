// license:BSD-3-Clause
// copyright-holders:ANY
/***********************************************************************************

    fireball.cpp

    Mechanical game where you have a gun shooting rubber balls.

    some pics here
    http://www.schausteller.de/anzeigenmarkt/euro-ball-66634.html

    TODO
    -NEVER sends store command to Eeprom so all change are lost

************************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "machine/eepromser.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "fireball.lh"


/****************************
*    LOG defines            *
****************************/

#define LOG_DISPLAY 0
#define LOG_DISPLAY2 0
#define LOG_INPUT 0
#define LOG_AY8912 0
#define LOG_P1 0
#define LOG_P3 0
#define LOG_OUTPUT 0

/****************************
*    Clock defines          *
****************************/
#define CPU_CLK  XTAL(11'059'200)
#define AY_CLK  XTAL(11'059'200)/8


class fireball_state : public driver_device
{
public:
	fireball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ay(*this, "aysnd")
		, m_eeprom(*this, "eeprom")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void fireball(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(io_00_w);
	DECLARE_READ8_MEMBER(io_00_r);
	DECLARE_WRITE8_MEMBER(io_02_w);
	DECLARE_READ8_MEMBER(io_02_r);
	DECLARE_WRITE8_MEMBER(io_04_w);
	DECLARE_READ8_MEMBER(io_04_r);
	DECLARE_WRITE8_MEMBER(io_06_w);
	DECLARE_READ8_MEMBER(io_06_r);
	DECLARE_READ8_MEMBER(p1_r);
	DECLARE_WRITE8_MEMBER(p1_w);
	DECLARE_READ8_MEMBER(p3_r);
	DECLARE_WRITE8_MEMBER(p3_w);
	TIMER_DEVICE_CALLBACK_MEMBER(int_0);

	void fireball_io_map(address_map &map);
	void fireball_map(address_map &map);

	uint8_t m_p1_data;
	uint8_t m_p3_data;
	uint8_t int_timing;
	uint8_t int_data;
	uint8_t ay_data;
	uint8_t to_ay_data;
	uint8_t m_display_data;
	virtual void machine_reset() override;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<i8031_device> m_maincpu;
	required_device<ay8912_device> m_ay;
	required_device<eeprom_serial_x24c44_device> m_eeprom;
	output_finder<8> m_digits;
};

/****************************
*    Read/Write Handlers    *
****************************/


READ8_MEMBER(fireball_state::io_00_r)
{
	uint8_t tmp=0;

	tmp=ioport("X2-4")->read();

	if (LOG_INPUT)
		logerror("return %02X from 0x00\n",tmp);
	return tmp;
}

WRITE8_MEMBER(fireball_state::io_00_w)
{
	m_display_data= m_display_data&0x7f;
	if (LOG_DISPLAY)
		logerror("write to 0x00 IO %02X, m_display_data= %01X\n",data,m_display_data);

	switch (data&0x0f)
	{
		case 1: m_digits[2] = m_display_data;
				break;
		case 2: m_digits[1] = m_display_data;
				break;
		case 4: m_digits[4] = m_display_data;
				break;
		case 8: m_digits[3] = m_display_data;
				break;
	}


	if (LOG_OUTPUT)
		logerror("write to 0x00 IO (X11-X11A) %02X\n",data&0xf0);

	output().set_value("Hopper1", BIT(data, 4));
	output().set_value("Hopper2", BIT(data, 5));
	output().set_value("Hopper3", BIT(data, 6));
}

READ8_MEMBER(fireball_state::io_02_r)
{
	uint8_t tmp=0;

	tmp=ioport("X6-8")->read();

	if (LOG_INPUT)
		logerror("return %02X from 0x02\n",tmp);
	return tmp;
}

WRITE8_MEMBER(fireball_state::io_02_w)
{
	if (LOG_OUTPUT)
		logerror("write to 0x00 IO (X7-X9) %02X\n",data);

	output().set_value("GameOver", BIT(data, 0));
	output().set_value("Title", BIT(data, 1));
	output().set_value("Credit", BIT(data, 2));
	output().set_value("SS", BIT(data, 3));
	output().set_value("C_LOCK", BIT(~data, 4));
	output().set_value("SV", BIT(data, 5));
	output().set_value("FBV", BIT(data, 6));
	output().set_value("RV", BIT(data, 7));
}

READ8_MEMBER(fireball_state::io_04_r)
{   //contraves per mod prog
	uint8_t tmp=0;

	tmp=ioport("X10-12")->read();

	if (LOG_INPUT)
		logerror("return %02X from 0x04\n",tmp);
	return tmp;
}

WRITE8_MEMBER(fireball_state::io_04_w)
{//display data
	if (LOG_DISPLAY)
		logerror("display datat write %02X\n",data);
	m_display_data=data;
}



READ8_MEMBER(fireball_state::io_06_r)
{
	if (LOG_AY8912)
		logerror("read from 0x06 IO\n");

	return 0xbe;
				//bit 0x01 is NC
				//bit 0x40 is used to detect is the unit is powerd up!!! related to eeprom store?
}

WRITE8_MEMBER(fireball_state::io_06_w)
{
	if (LOG_AY8912)
		logerror("write to 0x06 data =%02X\n",data);

	to_ay_data= data;

	if (LOG_DISPLAY2)
		logerror("On board display write %02X\n",uint8_t(~(data&0xff)));

	m_digits[7] = uint8_t(~(data&0xff));
}


	READ8_MEMBER(fireball_state::p1_r)
	{
	uint8_t tmp=0;
	tmp=(m_p1_data&0xfe)|(m_eeprom->do_read());
	if (LOG_P1)
		logerror("readP1 port data %02X\n",tmp&0x01);
	return tmp;
	}

	WRITE8_MEMBER(fireball_state::p1_w)
	{
	//eeprom x24c44/ay8912/system stuff...
	//bit0 goes to eeprom pin 3 and 4  (0x01) Data_in and Data_out
	//bit1 goes to eeprom pin 1 (0x02)      CE Hi active
	//bit2 goes to eeprom pin 2 (0x04)      SK Clock
	//bit3 goes to dis/thr input of a ne555 that somehow reset the 8031...      TODO
	//bit4 goes to ay8912 pin bc1   (0x10)
	//bit5 goes to ay8912 pin bdir (0x20)
	//bit6 goes to                                                              TODO
	//bit7 goes to                                                              TODO

	if (LOG_AY8912){
		if(( data&0x30) !=  (m_p1_data&0x30)){
			logerror("write ay8910 control bc1= %02X bdir= %02X\n",data&0x10, data&0x20);
		}
	}

	m_eeprom->di_write(data & 0x01);
	m_eeprom->clk_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->cs_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);

	if (LOG_P1){
		if(( data&0xc8) !=  (m_p1_data&0xc8)){
			logerror("Unknown P1 data changed, old data %02X, new data %02X\n",m_p1_data,data);
		}
	}

	//AY 3-8912 data write/read

	if (data & 0x20){   //bdir
		//write to ay8912
		if (LOG_AY8912)
			logerror("write to 0x06 bdir=1\n");
		if(data & 0x10){
			//address_w
			if (LOG_AY8912)
				logerror("write to 0x06 bc1=1\n");
			m_ay->address_w(to_ay_data);
			if (LOG_AY8912)
				logerror("AY8912 address latch write=%02X\n",to_ay_data);
		}else{
			//data_w
			if (LOG_AY8912)
				logerror("write to 0x06 bc1=0\n");
			m_ay->data_w(to_ay_data);
			if (LOG_AY8912)
				logerror("AY8912 data write=%02X\n",to_ay_data);
		}
	}else{
		if (LOG_AY8912)
			logerror("write to 0x06 bdir=0\n");
		ay_data=m_ay->data_r();
	}

	m_p1_data=data;
}


	READ8_MEMBER(fireball_state::p3_r)
	{
	uint8_t ret = 0xfb | ((int_data&1)<<2);
	if (LOG_P3)
		logerror("read P3 port data = %02X\n",ret);
	return ret;
	}

WRITE8_MEMBER(fireball_state::p3_w)
	{
	if (LOG_P3)
		logerror("write to P3 port data=%02X\n",data);

	m_p3_data=data;
}

/*************************
* Memory Map Information *
*************************/

void fireball_state::fireball_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void fireball_state::fireball_io_map(address_map &map)
{

	map(0x00, 0x01).rw(FUNC(fireball_state::io_00_r), FUNC(fireball_state::io_00_w));
	map(0x02, 0x03).rw(FUNC(fireball_state::io_02_r), FUNC(fireball_state::io_02_w));
	map(0x04, 0x05).rw(FUNC(fireball_state::io_04_r), FUNC(fireball_state::io_04_w));
	map(0x06, 0x07).rw(FUNC(fireball_state::io_06_r), FUNC(fireball_state::io_06_w));

}


/*************************
*      Input Ports       *
*************************/


static INPUT_PORTS_START( fireball )

	PORT_START("X2-4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("100 Points")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("100 Points second input")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("200 Points")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("300 Points")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("400 Points")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("500 Points")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("600 Points")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("800 Points")

	PORT_START("X6-8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("Empty Hopper A") //activeLow to fool the game code...
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("Empty Hopper B")//at least one hopper must be full
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Empty Hopper C")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("Programming Button")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("Confirm Value")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("All Options Default value")

	PORT_START("X10-12")
	PORT_DIPNAME( 0xff, 0x00, "Programming Value" )
	PORT_DIPSETTING(    0x00, "00" )    //0
	PORT_DIPSETTING(    0x01, "01" )
	PORT_DIPSETTING(    0x02, "02" )
	PORT_DIPSETTING(    0x03, "03" )
	PORT_DIPSETTING(    0x04, "04" )
	PORT_DIPSETTING(    0x05, "05" )
	PORT_DIPSETTING(    0x06, "06" )
	PORT_DIPSETTING(    0x07, "07" )
	PORT_DIPSETTING(    0x08, "08" )
	PORT_DIPSETTING(    0x09, "09" )
	PORT_DIPSETTING(    0x10, "10" )    //10
	PORT_DIPSETTING(    0x11, "11" )
	PORT_DIPSETTING(    0x12, "12" )
	PORT_DIPSETTING(    0x13, "13" )
	PORT_DIPSETTING(    0x14, "14" )
	PORT_DIPSETTING(    0x15, "15" )
	PORT_DIPSETTING(    0x16, "16" )
	PORT_DIPSETTING(    0x17, "17" )
	PORT_DIPSETTING(    0x18, "18" )
	PORT_DIPSETTING(    0x19, "19" )
	PORT_DIPSETTING(    0x20, "20" )    //20
	PORT_DIPSETTING(    0x21, "21" )
	PORT_DIPSETTING(    0x22, "22" )
	PORT_DIPSETTING(    0x23, "23" )
	PORT_DIPSETTING(    0x24, "24" )
	PORT_DIPSETTING(    0x25, "25" )
	PORT_DIPSETTING(    0x26, "26" )
	PORT_DIPSETTING(    0x27, "27" )
	PORT_DIPSETTING(    0x28, "28" )
	PORT_DIPSETTING(    0x29, "29" )
	PORT_DIPSETTING(    0x30, "30" )    //30
	PORT_DIPSETTING(    0x31, "31" )
	PORT_DIPSETTING(    0x32, "32" )
	PORT_DIPSETTING(    0x33, "33" )
	PORT_DIPSETTING(    0x34, "34" )
	PORT_DIPSETTING(    0x35, "35" )
	PORT_DIPSETTING(    0x36, "36" )
	PORT_DIPSETTING(    0x37, "37" )
	PORT_DIPSETTING(    0x38, "38" )
	PORT_DIPSETTING(    0x39, "39" )
	PORT_DIPSETTING(    0x40, "40" )    //40
	PORT_DIPSETTING(    0x41, "41" )
	PORT_DIPSETTING(    0x42, "42" )
	PORT_DIPSETTING(    0x43, "43" )
	PORT_DIPSETTING(    0x44, "44" )
	PORT_DIPSETTING(    0x45, "45" )
	PORT_DIPSETTING(    0x46, "46" )
	PORT_DIPSETTING(    0x47, "47" )
	PORT_DIPSETTING(    0x48, "48" )
	PORT_DIPSETTING(    0x49, "49" )
	PORT_DIPSETTING(    0x50, "50" )    //50
	PORT_DIPSETTING(    0x51, "51" )
	PORT_DIPSETTING(    0x52, "52" )
	PORT_DIPSETTING(    0x53, "53" )
	PORT_DIPSETTING(    0x54, "54" )
	PORT_DIPSETTING(    0x55, "55" )
	PORT_DIPSETTING(    0x56, "56" )
	PORT_DIPSETTING(    0x57, "57" )
	PORT_DIPSETTING(    0x58, "58" )
	PORT_DIPSETTING(    0x59, "59" )
	PORT_DIPSETTING(    0x60, "60" )    //60
	PORT_DIPSETTING(    0x61, "61" )
	PORT_DIPSETTING(    0x62, "62" )
	PORT_DIPSETTING(    0x63, "63" )
	PORT_DIPSETTING(    0x64, "64" )
	PORT_DIPSETTING(    0x65, "65" )
	PORT_DIPSETTING(    0x66, "66" )
	PORT_DIPSETTING(    0x67, "67" )
	PORT_DIPSETTING(    0x68, "68" )
	PORT_DIPSETTING(    0x69, "69" )
	PORT_DIPSETTING(    0x70, "70" )    //70
	PORT_DIPSETTING(    0x71, "71" )
	PORT_DIPSETTING(    0x72, "72" )
	PORT_DIPSETTING(    0x73, "73" )
	PORT_DIPSETTING(    0x74, "74" )
	PORT_DIPSETTING(    0x75, "75" )
	PORT_DIPSETTING(    0x76, "76" )
	PORT_DIPSETTING(    0x77, "77" )
	PORT_DIPSETTING(    0x78, "78" )
	PORT_DIPSETTING(    0x79, "79" )
	PORT_DIPSETTING(    0x80, "80" )    //80
	PORT_DIPSETTING(    0x81, "81" )
	PORT_DIPSETTING(    0x82, "82" )
	PORT_DIPSETTING(    0x83, "83" )
	PORT_DIPSETTING(    0x84, "84" )
	PORT_DIPSETTING(    0x85, "85" )
	PORT_DIPSETTING(    0x86, "86" )
	PORT_DIPSETTING(    0x87, "87" )
	PORT_DIPSETTING(    0x88, "88" )
	PORT_DIPSETTING(    0x89, "89" )
	PORT_DIPSETTING(    0x90, "90" )    //90
	PORT_DIPSETTING(    0x91, "91" )
	PORT_DIPSETTING(    0x92, "92" )
	PORT_DIPSETTING(    0x93, "93" )
	PORT_DIPSETTING(    0x94, "94" )
	PORT_DIPSETTING(    0x95, "95" )
	PORT_DIPSETTING(    0x96, "96" )
	PORT_DIPSETTING(    0x97, "97" )
	PORT_DIPSETTING(    0x98, "98" )
	PORT_DIPSETTING(    0x99, "99" )

INPUT_PORTS_END

/******************************
*   machine reset             *
******************************/

void fireball_state::machine_reset()
{
	int_timing=1;
	m_digits[5] = 0x3f;
	m_digits[6] = 0x3f;

	output().set_value("Hopper1", 0);
	output().set_value("Hopper2", 0);
	output().set_value("Hopper3", 0);

	output().set_value("GameOver", 0);
	output().set_value("Title", 0);
	output().set_value("Credit", 0);
	output().set_value("SS", 0);
	output().set_value("C_LOCK", 0);
	output().set_value("SV", 0);
	output().set_value("FBV", 0);
	output().set_value("RV", 0);
}

/*************************
*   INT callback         *
*************************/

TIMER_DEVICE_CALLBACK_MEMBER( fireball_state::int_0 )
{
	/* toggle the INT0 line on the CPU */
	if (int_timing==1){
		//logerror("INT set\n");
		m_maincpu->set_input_line(MCS51_INT0_LINE, ASSERT_LINE);
			int_data=1;
	}
	if (int_timing==2){
		//logerror("INT clear\n");
		m_maincpu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);
		int_data=0;
	}
	if (int_timing==5){
		int_timing=0;
	}
	int_timing++;
}


/*************************
*    Machine Drivers     *
*************************/


void fireball_state::fireball(machine_config &config)
{
	/* basic machine hardware */
	I8031(config, m_maincpu, CPU_CLK); //
	m_maincpu->set_addrmap(AS_PROGRAM, &fireball_state::fireball_map);
	m_maincpu->set_addrmap(AS_IO, &fireball_state::fireball_io_map);
	m_maincpu->port_in_cb<1>().set(FUNC(fireball_state::p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(fireball_state::p1_w));
	m_maincpu->port_in_cb<3>().set(FUNC(fireball_state::p3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(fireball_state::p3_w));

	//9ms from scope reading 111Hz take care of this in the handler
	TIMER(config, "int_0", 0).configure_periodic(timer_device::expired_delegate(FUNC(fireball_state::int_0), this), attotime::from_hz(555));

	EEPROM_X24C44_16BIT(config, "eeprom");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8912(config, m_ay, AY_CLK).add_route(ALL_OUTPUTS, "mono", 1.0);

	/* Video */
	config.set_default_layout(layout_fireball);
}


/*************************
*        Rom Load        *
*************************/

ROM_START(fireball)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("euroball-89-07-13-c026.bin", 0x0000, 0x2000, CRC(cab3fc1c) SHA1(bcf0d17e26f2d9f5e20bda258728c989ea138702))

	ROM_REGION16_BE( 0x20, "eeprom", 0 ) // default eeprom must have some specific value at 0x03 at least
	ROM_LOAD( "fireball.nv", 0x0000, 0x020, CRC(1d0f5f0f) SHA1(8e68fcd8782f39ed3b1df6162db9be83cb3335e4) )  //default setting
ROM_END

/*************************
*      Game Drivers      *
*************************/
//    YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT        ROT   COMPANY  FULLNAME    FLAGS
GAME( 1989, fireball, 0,      fireball, fireball, fireball_state, empty_init, ROT0, "Valco", "Fireball", MACHINE_MECHANICAL ) //1989 by rom name
