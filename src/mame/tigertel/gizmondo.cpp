// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
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
#include "docg3.h"
#include "machine/s3c2440.h"
#include "video/gf4500.h"
#include "screen.h"


namespace {

#define BITS(x,m,n) (((x)>>(n))&(((uint32_t)1<<((m)-(n)+1))-1))

class gizmondo_state : public driver_device
{
public:
	gizmondo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_s3c2440(*this, "s3c2440"),
		m_maincpu(*this, "maincpu"),
		m_gf4500(*this, "gf4500")
		{ }

	void gizmondo(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(port_changed);

	void init_gizmondo();

private:
	uint32_t m_port[9];
	required_device<s3c2440_device> m_s3c2440;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<gf4500_device> m_gf4500;
	uint32_t s3c2440_gpio_port_r(offs_t offset);
	void s3c2440_gpio_port_w(offs_t offset, uint32_t data);

	bitmap_rgb32 m_bitmap;
	void gizmondo_map(address_map &map) ATTR_COLD;
};


/*******************************************************************************
    ...
*******************************************************************************/

// I/O PORT

uint32_t gizmondo_state::s3c2440_gpio_port_r(offs_t offset)
{
	uint32_t data = m_port[offset];
	switch (offset)
	{
		case S3C2440_GPIO_PORT_D :
		{
			data = data & ~0x00000010;
			data |= (1 << 4); // 1 = "BOOT", 0 = "FTST"
		}
		break;
		case S3C2440_GPIO_PORT_F :
		{
			uint32_t port_c = m_port[S3C2440_GPIO_PORT_C];
			data = data & ~0x000000F2;
			// keys
			data |= 0x00F2;
			if ((port_c & 0x01) == 0) data &= ~ioport("PORTF-01")->read();
			if ((port_c & 0x02) == 0) data &= ~ioport("PORTF-02")->read();
			if ((port_c & 0x04) == 0) data &= ~ioport("PORTF-04")->read();
			if ((port_c & 0x08) == 0) data &= ~ioport("PORTF-08")->read();
			if ((port_c & 0x10) == 0) data &= ~ioport("PORTF-10")->read();
			data &= ~ioport( "PORTF")->read();
		}
		break;
		case S3C2440_GPIO_PORT_G :
		{
			data = data & ~0x00008001;
			// keys
			data = data | 0x8000;
			data &= ~ioport( "PORTG")->read();
			// no sd card inserted
			data = data | 0x0001;
		}
		break;
	}
	return data;
}

void gizmondo_state::s3c2440_gpio_port_w(offs_t offset, uint32_t data)
{
	m_port[offset] = data;
}

INPUT_CHANGED_MEMBER(gizmondo_state::port_changed)
{
	m_s3c2440->s3c2440_request_eint(4);
	//m_s3c2440->s3c2440_request_irq(S3C2440_INT_EINT1);
}

#if 0
QUICKLOAD_LOAD_MEMBER(gizmondo_state::quickload_cb)
{
	return gizmondo_quickload(image, file_type, quickload_size, 0x3000E000); // eboot
	//return gizmondo_quickload(image, file_type, quickload_size, 0x30400000); // wince
}
#endif

/*******************************************************************************
    MACHINE HARDWARE
*******************************************************************************/

void gizmondo_state::machine_start()
{
	m_port[S3C2440_GPIO_PORT_B] = 0x055E;
	m_port[S3C2440_GPIO_PORT_C] = 0x5F20;
	m_port[S3C2440_GPIO_PORT_D] = 0x4F60;
}

void gizmondo_state::machine_reset()
{
	m_maincpu->reset();
}

/*******************************************************************************
    ADDRESS MAPS
*******************************************************************************/

void gizmondo_state::gizmondo_map(address_map &map)
{
	map(0x00000000, 0x000007ff).rom();
	map(0x00000800, 0x00000fff).rw("diskonchip", FUNC(diskonchip_g3_device::sec_1_r), FUNC(diskonchip_g3_device::sec_1_w));
	map(0x00001000, 0x000017ff).rw("diskonchip", FUNC(diskonchip_g3_device::sec_2_r), FUNC(diskonchip_g3_device::sec_2_w));
	map(0x00001800, 0x00001fff).rw("diskonchip", FUNC(diskonchip_g3_device::sec_3_r), FUNC(diskonchip_g3_device::sec_3_w));
	map(0x30000000, 0x33ffffff).ram();
	map(0x34000000, 0x3413ffff).rw(m_gf4500, FUNC(gf4500_device::read), FUNC(gf4500_device::write));
}

/*******************************************************************************
    MACHINE DRIVERS
*******************************************************************************/

void gizmondo_state::init_gizmondo()
{
	// do nothing
}

void gizmondo_state::gizmondo(machine_config &config)
{
	ARM9(config, m_maincpu, 40000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &gizmondo_state::gizmondo_map);

	PALETTE(config, "palette").set_entries(32768);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(320, 240);
	screen.set_visarea_full();
	screen.set_screen_update("gf4500", FUNC(gf4500_device::screen_update));

	GF4500(config, m_gf4500, 0);

	S3C2440(config, m_s3c2440, 12000000);
	m_s3c2440->set_palette_tag("palette");
	m_s3c2440->set_screen_tag("screen");
	m_s3c2440->gpio_port_r_callback().set(FUNC(gizmondo_state::s3c2440_gpio_port_r));
	m_s3c2440->gpio_port_w_callback().set(FUNC(gizmondo_state::s3c2440_gpio_port_w));

	DISKONCHIP_G3(config, "diskonchip").set_size(64);

#if 0
	QUICKLOAD(config, "quickload", "bin", 0).set_load_callback(FUNC(gizmondo_state::quickload_cb), this);
#endif
}

static INPUT_PORTS_START( gizmondo )
	PORT_START( "PORTF-01" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_NAME("STOP") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_PLAYER(1)
	PORT_START( "PORTF-02" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_NAME("F2") PORT_PLAYER(1) PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_NAME("FORWARD") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_PLAYER(1)
	PORT_START( "PORTF-04" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_NAME("F3") PORT_PLAYER(1) PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_NAME("PLAY") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_PLAYER(1)
	PORT_START( "PORTF-08" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_NAME("F4") PORT_PLAYER(1) PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_NAME("REWIND") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_PLAYER(1)
	PORT_START( "PORTF-10" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_NAME("L") PORT_PLAYER(1) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_NAME("R") PORT_PLAYER(1) PORT_CODE(KEYCODE_R)
	PORT_START( "PORTF" )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_NAME("F5") PORT_PLAYER(1) PORT_CODE(KEYCODE_F5)
	PORT_START( "PORTG" )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_CHANGED_MEMBER(DEVICE_SELF, gizmondo_state, port_changed, 0) PORT_NAME("F1") PORT_PLAYER(1) PORT_CODE(KEYCODE_F1)
INPUT_PORTS_END

/*******************************************************************************
    GAME DRIVERS
*******************************************************************************/

ROM_START( gizmondo )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "fboot", "fboot" )
	ROMX_LOAD( "fboot.bin", 0, 0x800, CRC(28887c29) SHA1(e625caaa63b9db74cb6d7499dce12ac758c5fe76), ROM_BIOS(0) )
ROM_END

} // anonymous namespace


CONS(2005, gizmondo, 0, 0, gizmondo, gizmondo, gizmondo_state, init_gizmondo, "Tiger Telematics", "Gizmondo", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
