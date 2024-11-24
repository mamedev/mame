// license:BSD-3-Clause
// copyright-holders:smf
/*
  Namco Cyber Lead cabinet PCB

  CL1 I/OB  I/O LED (I/O) PCB  8699014200
  CL1 LEDA  I/O LED (LED) PCB  8699014500
  Both boards are required and use the same PCB, populated differently
  |------------------------------------------------------------------------------------------------|
  |        J3                J4           J6                                                       |
  | J2                                        J5                                                   |
  |                                           PROG            I/OLEDM1                             |
  |                        AT29C020               SED1351F                    B      A             |
  |                                  C77 14.764Mhz                    PST592F                   J7 |
  |      HC257 HC257 HC257 N341256                                                                 |
  | J1                                                                                             |
  |                                        SW1 SW2 SW3 SW4 SW5 9264LF10L 12MHZ                     |
  |   1P                           2P                                                              |
  |   J13      J12                 J11      J10                                                    |
  |------------------------------------------------------------------------------------------------|

Notes:
    J1         - 6 pin connector (I/O Only)
    J2         - 2 pin power connector
    J3         - 10 pin connector (I/O Only)
    J4         - 12 pin connector (I/O Only)
    J5 PROG    - ?
    J6         - 6 pin Mini DIN connector
    J7         - 20 pin connector (LED Only)
    J10        - 2 pin connector (I/O Only)
    J11        - 12 pin connector (I/O Only)
    J12        - 12 pin connector (I/O Only)
    J13        - 2 pin connector (I/O Only)
    A          - USB A connector (I/O Only)
    B          - USB B connector (I/O Only)
    HC257      - Quad 2-input multiplexer; 3-state (I/O Only) @IC1,IC2,IC3
    N341256    - NEC N341256 32k x8 SRAM @IC4
    AT29C020   - Atmel AT29C020 256k x8 FlashROM @IC5
    C77        - Namco C77 H8 MCU @IC6
    14.76      - X1 14.7SC6M
    SW1        - A push button
    SW2        - B push button
    SW3        - DSW4
    SW4        - Service push button
    SW5        - Test push button
    SED1351F   - EPSON SED1351F LCD controller (LED Only) @IC7
    9264LF10L  - 8192x8 Bit Static RAM (LED Only) @IC8
    I/OLEDM1   - Altera EPM7064 @IC9
    PST592F    - System reset @IC10
    12MHz      - OSC1 (LED Only)
    ADM485JR   - Analog Devices ADM485 low power EIA RS-485 transceiver (I/O Only) @IC11

Known games with LED data

  Aqua Rush
  Attack Pla Rail (*)
  Ehrgeiz (*)
  Fighting Layer (Japan) (*)
  Libero Grande
  Mr. Driller
  Soul Calibur
  Super World Stadium '98
  Super World Stadium '99
  Tekken 3 (* up to rev D)
  Tekken Tag Tournament

  (*) The only games that work with the unpatched CL1 I/OB Ver 1.03 rom
*/

#include "emu.h"
#include "cyberlead.h"
#include "cyberlead.lh"

namco_cyberlead_device::namco_cyberlead_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	namco_cyberlead_device(mconfig, NAMCO_CYBERLEAD, tag, owner, clock)
{
}

namco_cyberlead_device::namco_cyberlead_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_jvs_interface(mconfig, *this),
	m_iocpu(*this, "iocpu"),
	m_dsw(*this, "DSW"),
	m_port7(*this, "PORT7"),
	m_rs232(*this, "rs232")
{
}

void namco_cyberlead_device::device_add_mconfig(machine_config &config)
{
	NAMCO_C77(config, m_iocpu, 14.7456_MHz_XTAL);
	m_iocpu->set_addrmap(AS_PROGRAM, &namco_cyberlead_device::iocpu_program_map);
	m_iocpu->read_port4().set(FUNC(namco_cyberlead_device::iocpu_port_4_r));
	m_iocpu->write_port4().set(FUNC(namco_cyberlead_device::iocpu_port_4_w));
	m_iocpu->read_port5().set(FUNC(namco_cyberlead_device::iocpu_port_5_r));
	m_iocpu->write_port5().set(FUNC(namco_cyberlead_device::iocpu_port_5_w));
	m_iocpu->read_port6().set(FUNC(namco_cyberlead_device::iocpu_port_6_r));
	m_iocpu->write_port6().set(FUNC(namco_cyberlead_device::iocpu_port_6_w));
	m_iocpu->read_port7().set(FUNC(namco_cyberlead_device::iocpu_port_7_r));

	m_iocpu->write_sci_tx<0>().set(FUNC(namco_cyberlead_device::txd));

	config.set_maximum_quantum(attotime::from_hz(2 * 115200));

	RS232_PORT(config, m_rs232);
	m_rs232->option_add("namco_cyberlead_led", NAMCO_CYBERLEAD_LED);
	m_rs232->set_option_machine_config("namco_cyberlead_led", [&config](device_t *device)
	{
		if (!(config.gamedrv().flags & machine_flags::SWAP_XY))
			config.set_default_layout(layout_cyberlead);
	});

	m_rs232->set_default_option("namco_cyberlead_led");
	m_rs232->rxd_handler().set(m_iocpu, FUNC(h8_device::sci_rx_w<1>));
	m_iocpu->write_sci_tx<1>().set(m_rs232, FUNC(rs232_port_device::write_txd));

	add_jvs_port(config);
}

namespace {

static INPUT_PORTS_START(cyberleadjvs)
	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "SW3:3")
	PORT_DIPUNUSED_DIPLOC(0x01, IP_ACTIVE_LOW, "SW3:4")

	PORT_START("PORT7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("LED Mode B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("LED Mode A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x8c, IP_ACTIVE_LOW, IPT_UNKNOWN) // inputs?
	PORT_BIT(0x70, IP_ACTIVE_LOW, IPT_UNKNOWN) // outputs?
INPUT_PORTS_END

static INPUT_PORTS_START(cyberleadjvs_default)
	PORT_INCLUDE(cyberleadjvs)

	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH)

	PORT_START("PLAYER1")
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_START)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_SERVICE1)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON2)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_BUTTON3)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_BUTTON4)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_BUTTON5)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_BUTTON6)

	PORT_START("PLAYER2")
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_START) PORT_PLAYER(2)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(2)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_PLAYER(2)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_PLAYER(2)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_PLAYER(2)

	PORT_START("COIN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1)

	PORT_START("COIN2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN2)
INPUT_PORTS_END

} // anonymous namespace

ioport_constructor namco_cyberlead_device::device_input_ports() const
{
	return m_default_inputs ? INPUT_PORTS_NAME(cyberleadjvs_default) : INPUT_PORTS_NAME(cyberleadjvs);
}

namespace {

ROM_START( cyberleadjvs )
	ROM_REGION( 0x040000, "iocpu", 0 )
	ROM_LOAD( "cl1-iob.ic5",  0x000000, 0x040000, CRC(abb90360) SHA1(d938b1e1ae596d0ab1007352f61b0b800363c762) )
	ROM_FILL( 0xa6b, 0x1, 0x12 ) // HACK to improve compatibility, until newer revision is dumped
ROM_END

} // anonymous namespace

const tiny_rom_entry *namco_cyberlead_device::device_rom_region() const
{
	return ROM_NAME(cyberleadjvs);
}

void namco_cyberlead_device::device_start()
{
}

void namco_cyberlead_device::rxd(int state)
{
	m_iocpu->sci_rx_w<0>(state);
}

void namco_cyberlead_device::iocpu_program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("iocpu", 0);
	map(0xc000, 0xfb7f).ram();
	map(0xff80, 0xff83).r(FUNC(namco_cyberlead_device::iocpu_in_r));
}

uint8_t namco_cyberlead_device::iocpu_in_r(offs_t offset)
{
	if (offset == 0)
	{
		const auto p1 = player_r(0, 0x80bf);
		return ~bitswap<8>(p1, 15, 0, 1, 7, 2, 3, 4, 5); // BUTTON3, BUTTON2, BUTTON1, START, RIGHT, LEFT, DOWN, UP
	}
	else if (offset == 1)
	{
		const auto p2 = player_r(1, 0x80bf);
		return ~bitswap<8>(p2, 15, 0, 1, 7, 2, 3, 4, 5); // BUTTON3, BUTTON2, BUTTON1, START, RIGHT, LEFT, DOWN, UP
	}
	else if (offset == 2)
	{
		const auto p1 = player_r(0, 0x7000);
		const auto c1 = coin_r(0, 0x01);
		return ~((BIT(c1, 0) << 3) | bitswap<3>(p1, 12, 13, 14)); // COIN, BUTTON6, BUTTON5, BUTTON4
	}
	else if (offset == 3)
	{
		const auto p2 = player_r(1, 0x7000);
		const auto c2 = coin_r(1, 0x01);
		return ~((BIT(c2, 0) << 3) | bitswap<3>(p2, 12, 13, 14)); // COIN, BUTTON6, BUTTON5, BUTTON4
	}

	return 0xff;
}

uint8_t namco_cyberlead_device::iocpu_port_4_r()
{
	return (1 << 7) | // unknown input
		0x63 | // unknown outputs
		((m_jvs_sense != jvs_port_device::sense::None) << 4) | ((m_jvs_sense != jvs_port_device::sense::Initialized) << 3) |
		(1 << 2);// sense output
}

void namco_cyberlead_device::iocpu_port_4_w(uint8_t data)
{
	sense(BIT(data, 2) ? jvs_port_device::sense::Initialized : jvs_port_device::sense::Uninitialized);
}

uint8_t namco_cyberlead_device::iocpu_port_5_r()
{
	return 0xe9 | // unknown outputs
		(1 << 4) | // unknown input
		(1 << 2) | // rts
		(1 << 1); // unknown input
}

void namco_cyberlead_device::iocpu_port_5_w(uint8_t data)
{
	rts(BIT(data, 2));
}

uint8_t namco_cyberlead_device::iocpu_port_6_r()
{
	const auto system = system_r(0x80);
	const auto p1 = player_r(0, 0x40);

	return (1 << 7) | // unknown
		(3 << 5) | // coin counters
		(BIT(~p1, 6) << 4) | // SERVICE1
		(BIT(m_dsw->read(), 1, 3) << 1) | // DSW1-3 TODO: confirm
		(BIT(~system, 7) << 0); // TEST
}

void namco_cyberlead_device::iocpu_port_6_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 5));
}

uint8_t namco_cyberlead_device::iocpu_port_7_r()
{
	return m_port7->read();
}


namco_cyberlead_led_device::namco_cyberlead_led_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCO_CYBERLEAD_LED, tag, owner, clock),
	device_rs232_port_interface(mconfig, *this),
	m_ledcpu(*this, "ledcpu"),
	m_flash(*this, "flash"),
	m_screen(*this, "led"),
	m_scroll(*this, "scroll"),
	m_sed1351f_ram(*this, "sed1351f_ram"),
	m_sed1351f_control(*this, "sed1351f_control"),
	m_dsw(*this, "DSW")
{
}

namespace {

} // anonymous namespace

void namco_cyberlead_led_device::device_add_mconfig(machine_config &config)
{
	NAMCO_C77(config, m_ledcpu, 14.7456_MHz_XTAL);
	m_ledcpu->set_addrmap(AS_PROGRAM, &namco_cyberlead_led_device::ledcpu_program_map);
	m_ledcpu->read_port6().set(FUNC(namco_cyberlead_led_device::ledcpu_port_6_r));

	m_ledcpu->write_sci_tx<1>().set(FUNC(namco_cyberlead_led_device::output_rxd));

	ATMEL_29C020(config, m_flash);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_native_aspect();
	m_screen->set_size(96 * 8, 16 * 8);
	m_screen->set_visarea(0, (96 * 8) - 1, 0, (16 * 8) - 1);
	m_screen->set_screen_update(FUNC(namco_cyberlead_led_device::screen_update));
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_orientation(ROT180);
	m_screen->screen_vblank().set([this](int state)
	{
		m_ledcpu->set_input_line(6, state ? ASSERT_LINE : CLEAR_LINE);
	});
}

namespace {

static INPUT_PORTS_START(cyberleadled)
	PORT_START("DSW")
	PORT_DIPNAME(0x08, 0x08, "Display Test") PORT_DIPLOCATION("SW3:1") // c9fb
	PORT_DIPSETTING(0x08, "1 Time")
	PORT_DIPSETTING(0x00, "4 Times")
	PORT_DIPNAME(0x04, 0x04, "Disable Epilepsy Filter") PORT_DIPLOCATION("SW3:2") // c9fc
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "SW3:3")
	PORT_DIPUNUSED_DIPLOC(0x01, IP_ACTIVE_LOW, "SW3:4")
INPUT_PORTS_END

} // anonymous namespace

ioport_constructor namco_cyberlead_led_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cyberleadled);
}

namespace {

static const uint16_t led_gfx[] =
{
	0b0001101010010000,
	0b0110111111100100,
	0b1011111111111001,
	0b1011111111111001,
	0b1011111111111001,
	0b0110111111100100,
	0b0001101010010000,
	0b0000010101000000,
};

ROM_START( cyberleadled )
	ROM_REGION( 0x40000, "flash", 0 )
	ROM_LOAD( "cl1-leda.ic5", 0x000000, 0x040000, CRC(5717e05f) SHA1(61d61338e1bb414af32dbdc1e24d54c02fb9196e) )
ROM_END

} // anonymous namespace

const tiny_rom_entry *namco_cyberlead_led_device::device_rom_region() const
{
	return ROM_NAME(cyberleadled);
}

void namco_cyberlead_led_device::device_start()
{
	for (int c = 0; c < 4; c++)
		for (int i = 0; i < 4; i++)
		{
			int r = ((((0xff * c) / 3) + 0x2f) * i) / 3;
			m_palette[c][i] = rgb_t(std::min(r, 0xff), r / 4, r / 3);
		}

	m_flash_bank = 0;

	save_item(NAME(m_flash_bank));
}

void namco_cyberlead_led_device::input_txd(int state)
{
	m_ledcpu->sci_rx_w<1>(state);
}

void namco_cyberlead_led_device::ledcpu_program_map(address_map &map)
{
	map(0x0000, 0x3fff).rw(m_flash, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x4000, 0x5fff).ram().share(m_sed1351f_ram);
	map(0x6000, 0x600f).ram().share(m_sed1351f_control);
	map(0x7000, 0x7001).ram().share(m_scroll);
	map(0x8000, 0xbfff).rw(FUNC(namco_cyberlead_led_device::ledcpu_banked_flash_r), FUNC(namco_cyberlead_led_device::ledcpu_banked_flash_w));
	map(0xc000, 0xfb7f).ram();
	map(0xff84, 0xff87).w(FUNC(namco_cyberlead_led_device::ledcpu_flash_bank_w));
}

uint8_t namco_cyberlead_led_device::ledcpu_banked_flash_r(offs_t offset)
{
	return m_flash->read(offset + m_flash_bank);
}

void namco_cyberlead_led_device::ledcpu_banked_flash_w(offs_t offset, uint8_t data)
{
	m_flash->write(offset + m_flash_bank, data);
}

void namco_cyberlead_led_device::ledcpu_flash_bank_w(offs_t offset, uint8_t data)
{
	m_flash_bank = 0x4000 * (offset | (data << 2));
}

uint8_t namco_cyberlead_led_device::ledcpu_port_6_r()
{
	const auto system = 0;
	const auto p1 = 0;

	return (1 << 7) | // unknown
		(3 << 5) | // coin counters
		(BIT(~p1, 6) << 4) | // SERVICE1
		(BIT(m_dsw->read(), 1, 3) << 1) | // DSW1-3 TODO: confirm
		(BIT(~system, 7) << 0); // TEST
}

uint32_t namco_cyberlead_led_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int start = swapendian_int16(m_sed1351f_control[6 / 2]) & 0x1fff;
	int scroll = ~m_ledcpu->space(AS_PROGRAM).read_byte(0xd0bf) & 3;
	int scrollx = ((start & 0x1f) * 4) + scroll;
	int scrolly = start >> 5;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *dst = &bitmap.pix(y, cliprect.min_x);
		uint16_t *src = &m_sed1351f_ram[((scrolly + (y >> 3)) & 511) * 16];
		uint16_t gfx = led_gfx[y & 7];
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int p = (scrollx + (x >> 3)) & 127;
			*dst++ = m_palette[BIT(src[p >> 3], (~p & 7) * 2, 2)][BIT(gfx, (x & 7) << 1, 2)];
		}
	}
	return 0;
}

namespace {

ROM_START( cyberleadajvs )
	ROM_REGION( 0x040000, "iocpu", 0 )
	ROM_LOAD( "cl1-iob.ic5",  0x000000, 0x040000, CRC(abb90360) SHA1(d938b1e1ae596d0ab1007352f61b0b800363c762) )
ROM_END

class namco_cyberleada_device :
	public namco_cyberlead_device
{
public:
	namco_cyberleada_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0)
		: namco_cyberlead_device(mconfig, NAMCO_CYBERLEADA, tag, owner, clock)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD
	{
		return ROM_NAME(cyberleadajvs);
	}
};

} // anonymous namespace


DEFINE_DEVICE_TYPE(NAMCO_CYBERLEAD, namco_cyberlead_device, "namco_cyberlead", "Namco Cyber Lead I/O LED(I/O) PCB 8699014200 (compatibility patch)")
DEFINE_DEVICE_TYPE_PRIVATE(NAMCO_CYBERLEADA, device_jvs_interface, namco_cyberleada_device, "namco_cyberleada", "Namco Cyber Lead I/O LED(I/O) PCB 8699014200")
DEFINE_DEVICE_TYPE(NAMCO_CYBERLEAD_LED, namco_cyberlead_led_device, "namco_cyberlead_led", "Namco Cyber Lead I/O LED(LED) PCB 8699014500")
