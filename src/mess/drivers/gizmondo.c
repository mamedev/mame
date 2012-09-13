/*******************************************************************************

    Tiger Telematics Gizmondo

    (c) 2010 Tim Schuerewegen

*******************************************************************************/

/*

== DiskOnChip G3 - Layout ==

00000020 00000020 00000000 00000000 00000000 00000006 00000025 00000006 12A00000 000000FF 00000000 00000000 00000000 00000000 (BOOT & FTST)
00000140 00000140 00000000 00000000 00000000 00000026 00000165 00000026 12100000 000000FF 00000000 00000000 00000000 00000000 (KRNL)
00000001 00000001 00000000 00000000 00000000 00000166 00000166 00000166 12100000 000000FF 00000000 00000000 00000000 00000000
00000299 00000264 0000002F 00000003 00000003 0000016A 000003FF 00000167 1C100000 000000FF 00000000

== Windows CE - Interrupts ==

SYSINTR_KEYBOARD = INT_EINT1, INT_EINT4_7 (EINT4/EINT5/EINT6/EINT7, INT_EINT8_23 (EINT23)
SYSINTR_SERIAL_1 = INT_UART0 (SUBINT_ERR0, SUBINT_TXD0, SUBINT_RXD0), INT_EINT4_7 (EINT13)
SYSINTR_SERIAL_2 = INT_UART2 (SUBINT_ERR2, SUBINT_TXD2, SUBINT_RXD2)
SYSINTR_SERIAL_3 = INT_EINT8_23 (EINT17), ?
SYSINTR_POWER    = INT_EINT8_23 (EINT12)
SYSINTR_SYNTH    = INT_EINT8_23 (EINT16)
SYSINTR_GPS      = INT_EINT3, INT_EINT8_23 (EINT18)

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/docg3.h"
#include "machine/s3c2440.h"
#include "video/gf4500.h"
#include "rendlay.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		logerror( "%s: %s", machine.describe_context( ), buf);
	}
}

#define BIT(x,n) (((x)>>(n))&1)
#define BITS(x,m,n) (((x)>>(n))&(((UINT32)1<<((m)-(n)+1))-1))

class gizmondo_state : public driver_device
{
public:
	gizmondo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 m_port[9];
	device_t *m_s3c2440;
	DECLARE_DRIVER_INIT(gizmondo);
	virtual void machine_start();
	virtual void machine_reset();
};

/*******************************************************************************
    ...
*******************************************************************************/

// I/O PORT

static UINT32 s3c2440_gpio_port_r( device_t *device, int port, UINT32 mask)
{
	gizmondo_state *gizmondo = device->machine().driver_data<gizmondo_state>();
	UINT32 data = gizmondo->m_port[port];
	switch (port)
	{
		case S3C2440_GPIO_PORT_D :
		{
			data = data & ~0x00000010;
			data |= (1 << 4); // 1 = "BOOT", 0 = "FTST"
		}
		break;
		case S3C2440_GPIO_PORT_F :
		{
			UINT32 port_c = gizmondo->m_port[S3C2440_GPIO_PORT_C];
			data = data & ~0x000000F2;
			// keys
			data |= 0x00F2;
			if ((port_c & 0x01) == 0) data &= ~device->machine().root_device().ioport( "PORTF-01")->read();
			if ((port_c & 0x02) == 0) data &= ~device->machine().root_device().ioport( "PORTF-02")->read();
			if ((port_c & 0x04) == 0) data &= ~device->machine().root_device().ioport( "PORTF-04")->read();
			if ((port_c & 0x08) == 0) data &= ~device->machine().root_device().ioport( "PORTF-08")->read();
			if ((port_c & 0x10) == 0) data &= ~device->machine().root_device().ioport( "PORTF-10")->read();
			data &= ~device->machine().root_device().ioport( "PORTF")->read();
		}
		break;
		case S3C2440_GPIO_PORT_G :
		{
			data = data & ~0x00008001;
			// keys
			data = data | 0x8000;
			data &= ~device->machine().root_device().ioport( "PORTG")->read();
			// no sd card inserted
			data = data | 0x0001;
		}
		break;
	}
	return data;
}

static void s3c2440_gpio_port_w( device_t *device, int port, UINT32 mask, UINT32 data)
{
	gizmondo_state *gizmondo = device->machine().driver_data<gizmondo_state>();
	gizmondo->m_port[port] = data;
}

static INPUT_CHANGED( port_changed )
{
	gizmondo_state *gizmondo = field.machine().driver_data<gizmondo_state>();
	s3c2440_request_eint( gizmondo->m_s3c2440, 4);
	//s3c2440_request_irq( device, S3C2440_INT_EINT1);
}

#if 0
static QUICKLOAD_LOAD( gizmondo )
{
	return gizmondo_quickload( image, file_type, quickload_size, 0x3000E000); // eboot
	//return gizmondo_quickload( image, file_type, quickload_size, 0x30400000); // wince
}
#endif

/*******************************************************************************
    MACHINE HARDWARE
*******************************************************************************/

void gizmondo_state::machine_start()
{
	m_s3c2440 = machine().device( "s3c2440");
	m_port[S3C2440_GPIO_PORT_B] = 0x055E;
	m_port[S3C2440_GPIO_PORT_C] = 0x5F20;
	m_port[S3C2440_GPIO_PORT_D] = 0x4F60;
}

void gizmondo_state::machine_reset()
{
	machine().device("maincpu")->reset();
}

/*******************************************************************************
    ADDRESS MAPS
*******************************************************************************/

static ADDRESS_MAP_START( gizmondo_map, AS_PROGRAM, 32, gizmondo_state )
	AM_RANGE(0x00000000, 0x000007ff) AM_ROM
	AM_RANGE(0x00000800, 0x00000fff) AM_DEVREADWRITE16( "diskonchip", diskonchip_g3_device, sec_1_r, sec_1_w, 0xffffffff)
	AM_RANGE(0x00001000, 0x000017ff) AM_DEVREADWRITE16( "diskonchip", diskonchip_g3_device, sec_2_r, sec_2_w, 0xffffffff)
	AM_RANGE(0x00001800, 0x00001fff) AM_DEVREADWRITE16( "diskonchip", diskonchip_g3_device, sec_3_r, sec_3_w, 0xffffffff)
	AM_RANGE(0x30000000, 0x33ffffff) AM_RAM
	AM_RANGE(0x34000000, 0x3413ffff) AM_READWRITE_LEGACY(gf4500_r, gf4500_w)
ADDRESS_MAP_END

/*******************************************************************************
    MACHINE DRIVERS
*******************************************************************************/

DRIVER_INIT_MEMBER(gizmondo_state,gizmondo)
{
	// do nothing
}

static S3C2440_INTERFACE( gizmondo_s3c2440_intf )
{
	// CORE (pin read / pin write)
	{ NULL, NULL },
	// GPIO (port read / port write)
	{ s3c2440_gpio_port_r, s3c2440_gpio_port_w },
	// I2C (scl write / sda read / sda write)
	{ NULL, NULL, NULL },
	// ADC (data read)
	{ NULL },
	// I2S (data write)
	{ NULL },
	// NAND (command write / address write / data read / data write)
	{ NULL, NULL, NULL, NULL },
	// LCD (flags)
	{ 0 }
};

static MACHINE_CONFIG_START( gizmondo, gizmondo_state )
	MCFG_CPU_ADD("maincpu", ARM9, 40000000)
	MCFG_CPU_PROGRAM_MAP(gizmondo_map)

	MCFG_PALETTE_LENGTH(32768)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 320 - 1, 0, 240 - 1)
	MCFG_SCREEN_UPDATE_STATIC(gf4500)

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_VIDEO_START(gf4500)


	MCFG_S3C2440_ADD("s3c2440", 12000000, gizmondo_s3c2440_intf)

	MCFG_DISKONCHIP_G3_ADD("diskonchip", 64)

#if 0
	MCFG_QUICKLOAD_ADD("quickload", wince, "bin", 0)
#endif
MACHINE_CONFIG_END

static INPUT_PORTS_START( gizmondo )
	PORT_START( "PORTF-01" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED(port_changed, NULL) PORT_NAME("STOP") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED(port_changed, NULL) PORT_PLAYER(1)
	PORT_START( "PORTF-02" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_CHANGED(port_changed, NULL) PORT_NAME("F2") PORT_PLAYER(1) PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED(port_changed, NULL) PORT_NAME("FORWARD") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED(port_changed, NULL) PORT_PLAYER(1)
	PORT_START( "PORTF-04" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_CHANGED(port_changed, NULL) PORT_NAME("F3") PORT_PLAYER(1) PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED(port_changed, NULL) PORT_NAME("PLAY") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED(port_changed, NULL) PORT_PLAYER(1)
	PORT_START( "PORTF-08" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_CHANGED(port_changed, NULL) PORT_NAME("F4") PORT_PLAYER(1) PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED(port_changed, NULL) PORT_NAME("REWIND") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED(port_changed, NULL) PORT_PLAYER(1)
	PORT_START( "PORTF-10" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CHANGED(port_changed, NULL) PORT_NAME("L") PORT_PLAYER(1) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_CHANGED(port_changed, NULL) PORT_NAME("R") PORT_PLAYER(1) PORT_CODE(KEYCODE_R)
	PORT_START( "PORTF" )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_CHANGED(port_changed, NULL) PORT_NAME("F5") PORT_PLAYER(1) PORT_CODE(KEYCODE_F5)
	PORT_START( "PORTG" )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_CHANGED(port_changed, NULL) PORT_NAME("F1") PORT_PLAYER(1) PORT_CODE(KEYCODE_F1)
INPUT_PORTS_END

/*******************************************************************************
    GAME DRIVERS
*******************************************************************************/

ROM_START( gizmondo )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "fboot", "fboot" )
	ROMX_LOAD( "fboot.bin", 0, 0x800, CRC(28887c29) SHA1(e625caaa63b9db74cb6d7499dce12ac758c5fe76), ROM_BIOS(1) )
ROM_END

CONS(2005, gizmondo, 0, 0, gizmondo, gizmondo, gizmondo_state, gizmondo, "Tiger Telematics", "Gizmondo", GAME_NOT_WORKING | GAME_NO_SOUND)
