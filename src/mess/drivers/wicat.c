// license:MAME
// copyright-holders:Robbbert
/***************************************************************************

Wicat - various systems.

2013-09-01 Skeleton driver

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/terminal.h"
#include "machine/6522via.h"
#include "machine/mm58274c.h"
#include "machine/mc2661.h"
#include "wicat.lh"

class wicat_state : public driver_device
{
public:
	wicat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//		, m_p_base(*this, "rambase")
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, TERMINAL_TAG)
		, m_rtc(*this, "rtc")
		, m_uart0(*this,"uart0")
		, m_uart1(*this,"uart1")
		, m_uart2(*this,"uart2")
		, m_uart3(*this,"uart3")
		, m_uart4(*this,"uart4")
		, m_uart5(*this,"uart5")
		, m_uart6(*this,"uart6")
	{ }

	//DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ16_MEMBER(invalid_r);
	DECLARE_WRITE16_MEMBER(invalid_w);
	DECLARE_WRITE16_MEMBER(serial_w);
	DECLARE_WRITE16_MEMBER(parallel_led_w);
	DECLARE_READ8_MEMBER(via_a_r);
	DECLARE_READ8_MEMBER(via_b_r);
	DECLARE_WRITE8_MEMBER(via_a_w);
	DECLARE_WRITE8_MEMBER(via_b_w);
private:
	UINT8 m_term_data;
	virtual void machine_start();
	virtual void machine_reset();
//	required_shared_ptr<UINT16> m_p_base;
	required_device<cpu_device> m_maincpu;
	required_device<serial_terminal_device> m_terminal;
	required_device<mm58274c_device> m_rtc;
	required_device<mc2661_device> m_uart0;
	required_device<mc2661_device> m_uart1;
	required_device<mc2661_device> m_uart2;
	required_device<mc2661_device> m_uart3;
	required_device<mc2661_device> m_uart4;
	required_device<mc2661_device> m_uart5;
	required_device<mc2661_device> m_uart6;

	UINT8 m_portA;
	UINT8 m_portB;
};


static ADDRESS_MAP_START(wicat_mem, AS_PROGRAM, 16, wicat_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)
//	AM_RANGE(0x000000, 0x01efff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x000000, 0x001fff) AM_ROM AM_REGION("c2", 0x0000)
	AM_RANGE(0x020000, 0x1fffff) AM_RAM
	AM_RANGE(0x200000, 0x2fffff) AM_RAM
	AM_RANGE(0x300000, 0xdfffff) AM_READWRITE(invalid_r,invalid_w)
	AM_RANGE(0xeff800, 0xeffbff) AM_RAM  // memory mapping SRAM, used during boot sequence for the stack (TODO)
	AM_RANGE(0xf00000, 0xf00007) AM_DEVREADWRITE8("uart0",mc2661_device,read,write,0xff00)  // UARTs
	AM_RANGE(0xf00008, 0xf0000f) AM_DEVREADWRITE8("uart1",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00010, 0xf00017) AM_DEVREADWRITE8("uart2",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00018, 0xf0001f) AM_DEVREADWRITE8("uart3",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00020, 0xf00027) AM_DEVREADWRITE8("uart4",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00028, 0xf0002f) AM_DEVREADWRITE8("uart5",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00030, 0xf00037) AM_DEVREADWRITE8("uart6",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00040, 0xf0005f) AM_DEVREADWRITE8("via",via6522_device,read,write,0xff00)
	AM_RANGE(0xf00060, 0xf0007f) AM_DEVREADWRITE8("rtc",mm58274c_device,read,write,0xff00)
	AM_RANGE(0xf000d0, 0xf000d1) AM_WRITE(parallel_led_w)
	AM_RANGE(0xf00f00, 0xf00fff) AM_READWRITE(invalid_r,invalid_w)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( wicat )
INPUT_PORTS_END


void wicat_state::machine_start()
{
}

void wicat_state::machine_reset()
{
}

//WRITE8_MEMBER( wicat_state::kbd_put )
//{
//	m_term_data = data;
//}

WRITE16_MEMBER( wicat_state::serial_w )
{
	if(ACCESSING_BITS_8_15)  // even addresses
	{
		switch(offset)
		{
		//case 0x00:
		//case 0x01:
		//case 0x02:
		//case 0x03:
			//m_terminal->write(space,0,data >> 8);
		default:
			logerror("Serial: Unused serial port write %02x to offset %02x\n",data,offset);
		}
	}
}

WRITE16_MEMBER( wicat_state::parallel_led_w )
{
	// bit 0 - parallel port A direction (0 = input)
	// bit 1 - parallel port B direction (0 = input)
	output_set_value("led1",(~data) & 0x0400);
	output_set_value("led2",(~data) & 0x0800);
	output_set_value("led3",(~data) & 0x1000);
	output_set_value("led4",(~data) & 0x2000);
	output_set_value("led5",(~data) & 0x4000);
	output_set_value("led6",(~data) & 0x8000);
}

READ8_MEMBER( wicat_state::via_a_r )
{
	return m_portA;
}

READ8_MEMBER( wicat_state::via_b_r )
{
	return m_portB;
}

WRITE8_MEMBER( wicat_state::via_a_w )
{
	m_portA = data;
	logerror("VIA: write %02x to port A\n",data);
}

WRITE8_MEMBER( wicat_state::via_b_w )
{
	m_portB = data;
	logerror("VIA: write %02x to port B\n",data);
}

READ16_MEMBER( wicat_state::invalid_r )
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0xff;
}

WRITE16_MEMBER( wicat_state::invalid_w )
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
}

static serial_terminal_interface terminal_intf =
{
	DEVCB_NULL
};

static mc2661_interface wicat_uart0_intf =
{
	0,  // RXC
	0,  // TXC
	DEVCB_NULL, //DEVCB_DEVICE_LINE_MEMBER(RS232_TAG, serial_port_device, rx),  // RXD in
	DEVCB_NULL, //DEVCB_DEVICE_LINE_MEMBER(RS232_TAG, serial_port_device, tx),  // RXD out
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // RXRDY out
	DEVCB_NULL,  // TXRDY out
	DEVCB_NULL, //DEVCB_DEVICE_LINE_MEMBER(RS232_TAG, rs232_port_device, rts_w),  // RTS out
	DEVCB_NULL, //DEVCB_DEVICE_LINE_MEMBER(RS232_TAG, rs232_port_device, dtr_w),  // DTR out
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // TXEMT out
	DEVCB_NULL,  // BKDET out
	DEVCB_NULL   // XSYNC out
};

static mc2661_interface wicat_unused_intf =
{
	0,
	0,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static via6522_interface wicat_via_intf =
{
	DEVCB_DRIVER_MEMBER(wicat_state,via_a_r),  // Port A in
	DEVCB_DRIVER_MEMBER(wicat_state,via_b_r),  // Port B in
	DEVCB_NULL,  // CA1 in
	DEVCB_NULL,  // CB1 in
	DEVCB_NULL,  // CA2 in
	DEVCB_NULL,  // CB2 in
	DEVCB_DRIVER_MEMBER(wicat_state,via_a_w),  // Port A out
	DEVCB_DRIVER_MEMBER(wicat_state,via_b_w),  // Port B out
	DEVCB_NULL,  // CA1 out
	DEVCB_NULL,  // CB1 out
	DEVCB_NULL,  // CA2 out
	DEVCB_NULL,  // CB2 out
	DEVCB_CPU_INPUT_LINE("maincpu", M68K_IRQ_1)  // IRQ
};

static mm58274c_interface wicat_rtc_intf =
{
	0,  // 12 hour
	1   // first day
};

static MACHINE_CONFIG_START( wicat, wicat_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_8MHz) // unknown clock
	MCFG_CPU_PROGRAM_MAP(wicat_mem)

	/* video hardware */
	MCFG_SERIAL_TERMINAL_ADD(TERMINAL_TAG, terminal_intf, XTAL_5_0688MHz)

	MCFG_VIA6522_ADD("via",XTAL_4MHz,wicat_via_intf)

	MCFG_MM58274C_ADD("rtc",wicat_rtc_intf)  // actually an MM58174AN, but should be compatible

	MCFG_MC2661_ADD("uart0", XTAL_5_0688MHz, wicat_uart0_intf)  // connected to terminal board (TODO)
	MCFG_MC2661_ADD("uart1", XTAL_5_0688MHz, wicat_unused_intf)
	MCFG_MC2661_ADD("uart2", XTAL_5_0688MHz, wicat_unused_intf)
	MCFG_MC2661_ADD("uart3", XTAL_5_0688MHz, wicat_unused_intf)
	MCFG_MC2661_ADD("uart4", XTAL_5_0688MHz, wicat_unused_intf)
	MCFG_MC2661_ADD("uart5", XTAL_5_0688MHz, wicat_unused_intf)
	MCFG_MC2661_ADD("uart6", XTAL_5_0688MHz, wicat_unused_intf)  // connected to modem port

	MCFG_DEFAULT_LAYOUT(layout_wicat)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( wicat )
	ROM_REGION16_BE(0x4000, "c1", 0)
	ROM_LOAD16_BYTE("wiboot.e",   0x00000, 0x0800, CRC(6f0f73c6) SHA1(be635bf3ffa1301f844a3d5560e278de46740d19) )
	ROM_LOAD16_BYTE("wiboot.o",   0x00001, 0x0800, CRC(b9763bbd) SHA1(68f497be56ff69534e17b41a40737cd6f708d65e) )
	ROM_LOAD16_BYTE("tpcnif.e",   0x01000, 0x0800, CRC(fd1127ec) SHA1(7c6b436c0cea41dbb23cb6bd9b9a5c21fa61d232) )
	ROM_LOAD16_BYTE("tpcnif.o",   0x01001, 0x0800, CRC(caa16e2a) SHA1(b3e64b676f50b65b3e365fc5f17eb1759c1310df) )
	ROM_LOAD16_BYTE("tpcf.e",     0x02000, 0x0800, CRC(d34be25c) SHA1(1b167918cbc19c9364f020176f4cc3722cba8434) )
	ROM_LOAD16_BYTE("tpcf.o",     0x02001, 0x0800, CRC(7712c570) SHA1(8743b7c98190ecf3bf7e917e6143b47b3b36db8d) )
	ROM_REGION(0x0060, "c1proms", 0)
	ROM_LOAD       ("cpu.8b",     0x00000, 0x0020, CRC(99b90665) SHA1(8a4677ea814e1843001fe28b284226b7291cdf76) )
	ROM_LOAD       ("cpu.8c",     0x00020, 0x0020, CRC(190a55ad) SHA1(de8a847bff8c343d69b853a215e6ee775ef2ef96) )
	ROM_LOAD       ("cpu.15c",    0x00040, 0x0020, CRC(ba2dd77d) SHA1(eb693d6d30aa6a9dba61c6c41a75614ed4e9e69a) )

	// System 150 CPU/MU board
	ROM_REGION16_BE(0x2000, "c2", 0)
	ROM_LOAD16_BYTE("boot156.a5", 0x00000, 0x0800, CRC(58510a52) SHA1(d2135b056a04ba830b0ae1cef539e4a9a1b58f82) )
	ROM_LOAD16_BYTE("boot156.a7", 0x00001, 0x0800, CRC(e53999f1) SHA1(9c6c6a3a56b5c16a35e1fe824f37c8ae739ebcb9) )
	ROM_LOAD16_BYTE("wd3_15.b5",  0x01000, 0x0800, CRC(a765899b) SHA1(8427c564029914b7dbc29768ce451604180e390f) )
	ROM_LOAD16_BYTE("wd3_15.b7",  0x01001, 0x0800, CRC(9d986585) SHA1(1ac7579c692f827b121c56dac0a77b15400caba1) )

	// Terminal CPU board (Graphical)
	ROM_REGION16_BE(0x8000, "g1", 0)
	ROM_LOAD16_BYTE("1term0.e",   0x00000, 0x0800, CRC(a9aade37) SHA1(644e9362d5a9523be5c6f39a650b574735dbd4a2) )
	ROM_LOAD16_BYTE("1term0.o",   0x00001, 0x0800, CRC(8026b5b7) SHA1(cb93e0595b321889694cbb87f497d244e6a2d648) )
	ROM_LOAD16_BYTE("1term1.e",   0x01000, 0x0800, CRC(e6ce8016) SHA1(fae987f1ac26d027ed176f8886832e87d1feae60) )
	ROM_LOAD16_BYTE("1term1.o",   0x01001, 0x0800, CRC(d71f763e) SHA1(b0a7f4cc90ce267aec7e72ad22a227f0c8c1f650) )
	ROM_LOAD16_BYTE("1term2.e",   0x02000, 0x0800, CRC(c0e82703) SHA1(7a17da13c01e15b61eea65b06d988ab8ba7eaaf3) )
	ROM_LOAD16_BYTE("1term2.o",   0x02001, 0x0800, CRC(aa0d5b4f) SHA1(b37c2e5220f4838a805b20a0ef21689067f1a759) )
	ROM_LOAD16_BYTE("1term3.e",   0x03000, 0x0800, CRC(cd33f4c8) SHA1(6603c5f2330a9a5ec1121a367cebe6e900a00cb0) )
	ROM_LOAD16_BYTE("1term3.o",   0x03001, 0x0800, CRC(05e56714) SHA1(0c31be3c9ec90a0858fe04a208e2627e4beb12b0) )
	ROM_LOAD16_BYTE("1term4.e",   0x04000, 0x0800, CRC(a157c61f) SHA1(59b7be6cd696b2508b5c1fd7b6e6f7cb5a9f12ab) )
	ROM_LOAD16_BYTE("1term4.o",   0x04001, 0x0800, CRC(364c1a95) SHA1(bfd62a71c9d8f83dc12a7dbbf362d18819380ef3) )
	ROM_LOAD16_BYTE("1term5.e",   0x05000, 0x0800, CRC(c2b8bc9e) SHA1(cd054988a9694b3a211e1993da1b3dc2c5e6fdc2) )
	ROM_LOAD16_BYTE("1term5.o",   0x05001, 0x0800, CRC(421e0521) SHA1(29b87938f5c25c05920ca2c14893700bc45a86c5) )
	ROM_LOAD16_BYTE("1term6.e",   0x06000, 0x0800, CRC(f0d14ed6) SHA1(840acc2b90e8d16df7e5d60c399b08ec0e126a88) )
	ROM_LOAD16_BYTE("1term6.o",   0x06001, 0x0800, CRC(e245ff49) SHA1(9a34e6cf6013b1044cccf26371cc3a000f17b58c) )
	ROM_LOAD16_BYTE("1term7.e",   0x07000, 0x0800, CRC(0c918550) SHA1(2ef6ce41cc2643d45c4bae31ce151d8b6c363471) )
	ROM_LOAD16_BYTE("1term7.o",   0x07001, 0x0800, CRC(71fdc692) SHA1(d6f12ec20ff2e4948f54b0c79f11ccbdc9db865c) )

	ROM_REGION16_BE(0x8000, "g2", 0)
	ROM_LOAD16_BYTE("2term0.e",   0x00000, 0x0800, CRC(29e5dd68) SHA1(9023f53d554b9ef4f4efc731645ba42f728bcd2c) )
	ROM_LOAD16_BYTE("2term0.o",   0x00001, 0x0800, CRC(91edd05d) SHA1(378b06fc8316199b7c580a6e7f28368dacdac5a9) )
	ROM_LOAD16_BYTE("2term1.e",   0x01000, 0x0800, CRC(2b48abe4) SHA1(4c9b4db1c1408b6551d50172dda994b36a2ee4b1) )
	ROM_LOAD16_BYTE("2term1.o",   0x01001, 0x0800, CRC(4c0e4f95) SHA1(bd49bf71fea1acfd50781820f0a650411b6f996b) )
	ROM_LOAD16_BYTE("2term2.e",   0x02000, 0x0800, CRC(3251324b) SHA1(e8f52308c9cbb9bcb5adb2685609d6a69b9eec1d) )
	ROM_LOAD16_BYTE("2term2.o",   0x02001, 0x0800, CRC(3a49c9e7) SHA1(0718b029ed316bc8e7bf22b0e94b6b5628758580) )
	ROM_LOAD16_BYTE("2term3.e",   0x03000, 0x0800, CRC(0f17be85) SHA1(9c40b4d06f3fb8def88b87615a590bb03dcfc4f4) )
	ROM_LOAD16_BYTE("2term3.o",   0x03001, 0x0800, CRC(08ae31c5) SHA1(2e53f87b6a4e0b973f7918d97f57f6560c651ab6) )
	ROM_LOAD16_BYTE("2term4.e",   0x04000, 0x0800, CRC(413936e7) SHA1(ce9d8666ca4e6847514bcf4de5703f0845e72928) )
	ROM_LOAD16_BYTE("2term4.o",   0x04001, 0x0800, CRC(06deab4e) SHA1(af5be7105a24d81dcc539296631b4309f7b8cb3f) )
	ROM_LOAD16_BYTE("2term5.e",   0x05000, 0x0800, CRC(7979bf59) SHA1(1bc397c58ce026fb90a02714d42df8f179a4f50e) )
	ROM_LOAD16_BYTE("2term5.o",   0x05001, 0x0800, CRC(e1f738ca) SHA1(bd8d7f1acb243880fd364f71097b9711de496739) )
	ROM_LOAD16_BYTE("2term6.e",   0x06000, 0x0800, CRC(bb04d70c) SHA1(0b482c2f06fe5e042a5813f027f5cf034d72e0dd) )
	ROM_LOAD16_BYTE("2term6.o",   0x06001, 0x0800, CRC(0afb566c) SHA1(761455ced46b6fccd0be9c8fa920f7954a36972b) )
	ROM_LOAD16_BYTE("2term7.e",   0x07000, 0x0800, CRC(033ea830) SHA1(27c33eea2df812a1a96e2f47ba7993e2ca3675ad) )
	ROM_LOAD16_BYTE("2term7.o",   0x07001, 0x0800, CRC(e157c5d2) SHA1(3cd1ea0fb9df1358e8a358468a4df5e4eaaa86a2) )

	// Terminal Video board
	ROM_REGION(0x1000, "g2char", 0)
	ROM_LOAD       ("ascii.chr",  0x00000, 0x0800, CRC(43e26e37) SHA1(f3d5d16040c66f0e827f72a35d4694ca62950949) )
	ROM_LOAD       ("apl.chr",    0x00800, 0x0800, CRC(8c6d698e) SHA1(147dd9296fe2efc6140fa148a6edf673c33f9371) )

	ROM_REGION(0x1800, "wd3", 0)
	ROM_LOAD       ("wd3.u95",    0x00000, 0x0800, CRC(80bb0617) SHA1(ac0f3194fcbef77532571baa3fec78b3010528bf) )
	ROM_LOAD       ("wd3.u96",    0x00800, 0x0800, CRC(52736e61) SHA1(71c7c9170c733c483393969cb1cb3798b3eb980c) )
	ROM_LOAD       ("wd3.u97",    0x01000, 0x0800, CRC(a66619ec) SHA1(5d091ac7c88f2f45b4a05e78bfc7a16c206b31ff) )
ROM_END


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   CLASS         INIT    COMPANY          FULLNAME       FLAGS */
COMP( 198?, wicat, 0,       0,     wicat, wicat, driver_device, 0, "Millennium Systems", "Wicat", GAME_NOT_WORKING | GAME_NO_SOUND_HW )
