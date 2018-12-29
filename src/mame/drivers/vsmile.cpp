// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    V-Tech V.Smile console emulat

    Status:

        Most games boot but lack controls

    To-Do:

        Proper UART support (SPG2xx) for controller

    Similar Systems: ( from http://en.wkikpedia.org/wiki/V.Smile )

        V.Smile by VTech, a system designed for children under the age of 10
        V.Smile Pocket (2 versions)
        V.Smile Cyber Pocket
        V.Smile PC Pal
        V-Motion Active Learning System
        Leapster
        V.Smile Baby Infant Development System
        V.Flash

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "cpu/unsp/unsp.h"

#include "machine/bankdev.h"
#include "machine/spg2xx.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class vsmile_state : public driver_device
{
public:
	vsmile_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_spg(*this, "spg")
		, m_cart(*this, "cartslot")
		, m_bankdev(*this, "bank")
		, m_system_region(*this, "maincpu")
		, m_io_p1(*this, "P1")
		, m_io_p2(*this, "P2")
		, m_cart_region(nullptr)
		, m_pad_timer(nullptr)
		, m_pad_counter(0)
		, m_portb_data(0)
		, m_portc_data(0)
	{ }

	void vsmile(machine_config &config);
	void vsmilep(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static const device_timer_id TIMER_UART_TX = 0;
	static const device_timer_id TIMER_PAD = 1;

	void mem_map(address_map &map);
	void banked_map(address_map &map);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart);

	DECLARE_READ16_MEMBER(portb_r);
	DECLARE_READ16_MEMBER(portc_r);
	DECLARE_WRITE16_MEMBER(portb_w);
	DECLARE_WRITE16_MEMBER(portc_w);

	DECLARE_WRITE8_MEMBER(chip_sel_w);

	DECLARE_WRITE8_MEMBER(uart_tx);

	DECLARE_READ16_MEMBER(bank0_r);
	DECLARE_READ16_MEMBER(bank1_r);
	DECLARE_READ16_MEMBER(bank2_r);
	DECLARE_READ16_MEMBER(bank3_r);

	enum
	{
		VSMILE_PORTB_CS1 =      0x01,
		VSMILE_PORTB_CS2 =      0x02,
		VSMILE_PORTB_CART =     0x04,
		VSMILE_PORTB_RESET =    0x08,
		VSMILE_PORTB_FRONT24 =  0x10,
		VSMILE_PORTB_OFF =      0x20,
		VSMILE_PORTB_OFF_SW =   0x40,
		VSMILE_PORTB_ON_SW =    0x80,

		VSMILE_PORTC_VER =      0x0f,
		VSMILE_PORTC_LOGO =     0x10,
		VSMILE_PORTC_TEST =     0x20,
		VSMILE_PORTC_AMP =      0x40,
		VSMILE_PORTC_SYSRESET = 0x80
	};

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<spg2xx_device> m_spg;
	required_device<generic_slot_device> m_cart;
	required_device<address_map_bank_device> m_bankdev;
	required_memory_region m_system_region;
	required_ioport m_io_p1;
	required_ioport m_io_p2;
	memory_region *m_cart_region;

	emu_timer *m_pad_timer;
	uint8_t m_pad_counter;

	uint16_t m_portb_data;
	uint16_t m_portc_data;
};

void vsmile_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_UART_TX:
		break;
	case TIMER_PAD:
		m_pad_counter++;
		if (m_pad_counter >= 100)
		{
			m_pad_counter = 0;
			//m_spg->uart_rx(0x55);
		}
		break;
	default:
		logerror("Unknown timer ID: %d\n", id);
		break;
	}
}

READ16_MEMBER(vsmile_state::bank0_r)
{
	return ((uint16_t*)m_cart_region->base())[offset];
}

READ16_MEMBER(vsmile_state::bank1_r)
{
	return ((uint16_t*)m_cart_region->base())[offset + 0x100000];
}

READ16_MEMBER(vsmile_state::bank2_r)
{
	return ((uint16_t*)m_cart_region->base())[offset + 0x200000];
}

READ16_MEMBER(vsmile_state::bank3_r)
{
	return ((uint16_t*)m_system_region->base())[offset];
}

READ16_MEMBER(vsmile_state::portb_r)
{
	//const uint8_t inputs = m_io_p2->read();
	//const uint16_t input_bits = BIT(inputs, 0) ? VSMILE_PORTB_ON_SW : 0;
	//const uint16_t data = VSMILE_PORTB_ON_SW | VSMILE_PORTB_OFF_SW | (m_cart && m_cart->exists() ? VSMILE_PORTB_CART : 0);
	//logerror("V.Smile Port B read  %04x, mask %04x\n", data, mem_mask);
	//printf("V.Smile Port B read  %04x, mask %04x\n", data, mem_mask);
	return m_portb_data;// | data;
}

READ16_MEMBER(vsmile_state::portc_r)
{
	uint16_t data = 0x0004;
	if (m_portc_data & 0x0100)
		data |= 0x0400;
	if (m_portc_data & 0x0200)
		data |= 0x1000;
	//logerror("V.Smile Port C read  %04x, mask %04x\n", data, mem_mask);
	return (m_portc_data & ~0x000f) | data;
}

WRITE16_MEMBER(vsmile_state::portb_w)
{
	m_portb_data = data;//(m_portb_data &~ mem_mask) | (data & mem_mask);
	//logerror("V.Smile Port B write %04x, mask %04x\n", m_portb_data, mem_mask);
	//printf("V.Smile Port B write %04x, mask %04x\n", m_portb_data, mem_mask);
}

WRITE16_MEMBER(vsmile_state::portc_w)
{
	m_portc_data = data;//(m_portc_data &~ mem_mask) | (data & mem_mask);
	//logerror("V.Smile Port C write %04x, mask %04x\n", m_portc_data, mem_mask);
	//printf("V.Smile Port C write %04x, mask %04x\n", m_portc_data, mem_mask);
	//printf("%02x ", data >> 8);
}

WRITE8_MEMBER(vsmile_state::uart_tx)
{
	logerror("UART Tx: %02x\n", data);
}

WRITE8_MEMBER(vsmile_state::chip_sel_w)
{
	const uint16_t cart_offset = m_cart && m_cart->exists() ? 4 : 0;
	switch (data)
	{
		case 0:
			m_bankdev->set_bank(cart_offset);
			break;
		case 1:
			m_bankdev->set_bank(1 + cart_offset);
			break;
		case 2:
		case 3:
			m_bankdev->set_bank(2 + cart_offset);
			break;
	}
}

void vsmile_state::machine_start()
{
	// if there's a cart, override the standard banking
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	}

	m_bankdev->set_bank(m_cart && m_cart->exists() ? 4 : 0);

	m_pad_timer = timer_alloc(TIMER_PAD);
	m_pad_timer->adjust(attotime::never);
}

void vsmile_state::machine_reset()
{
	m_portb_data = 0;
	m_portc_data = 0;

	m_pad_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));
	m_pad_counter = 0;
}

DEVICE_IMAGE_LOAD_MEMBER(vsmile_state, cart)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

void vsmile_state::banked_map(address_map &map)
{
	map(0x0000000, 0x00fffff).rom().region("maincpu", 0);
	map(0x0100000, 0x01fffff).rom().region("maincpu", 0);
	map(0x0200000, 0x02fffff).rom().region("maincpu", 0);
	map(0x0300000, 0x03fffff).rom().region("maincpu", 0);

	map(0x0400000, 0x04fffff).rom().region("maincpu", 0);
	map(0x0500000, 0x05fffff).rom().region("maincpu", 0);
	map(0x0600000, 0x06fffff).rom().region("maincpu", 0);
	map(0x0700000, 0x07fffff).rom().region("maincpu", 0);

	map(0x0800000, 0x08fffff).rom().region("maincpu", 0);
	map(0x0900000, 0x09fffff).rom().region("maincpu", 0);
	map(0x0a00000, 0x0afffff).rom().region("maincpu", 0);
	map(0x0b00000, 0x0bfffff).rom().region("maincpu", 0);

	map(0x1000000, 0x13fffff).r(FUNC(vsmile_state::bank0_r));

	map(0x1400000, 0x15fffff).r(FUNC(vsmile_state::bank0_r));
	map(0x1600000, 0x17fffff).r(FUNC(vsmile_state::bank1_r));

	map(0x1800000, 0x18fffff).r(FUNC(vsmile_state::bank0_r));
	map(0x1900000, 0x19fffff).r(FUNC(vsmile_state::bank1_r));
	map(0x1a00000, 0x1afffff).nopr();
	map(0x1b00000, 0x1bfffff).r(FUNC(vsmile_state::bank3_r));
}

void vsmile_state::mem_map(address_map &map)
{
	map(0x000000, 0x3fffff).r(m_bankdev, FUNC(address_map_bank_device::read16));
	map(0x000000, 0x003fff).m(m_spg, FUNC(spg2xx_device::map));
}

static INPUT_PORTS_START( vsmile )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Menu")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("X Button")

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "POWER ON" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "POWER OFF" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

void vsmile_state::vsmile(machine_config &config)
{
	UNSP(config, m_maincpu, XTAL(27'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &vsmile_state::mem_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("spg", FUNC(spg2xx_device::screen_update));
	m_screen->screen_vblank().set(m_spg, FUNC(spg2xx_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SPG24X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);
	m_spg->portb_in().set(FUNC(vsmile_state::portb_r));
	m_spg->portc_in().set(FUNC(vsmile_state::portc_r));
	m_spg->portb_out().set(FUNC(vsmile_state::portb_w));
	m_spg->portc_out().set(FUNC(vsmile_state::portc_w));
	m_spg->chip_select().set(FUNC(vsmile_state::chip_sel_w));
	m_spg->uart_tx().set(FUNC(vsmile_state::uart_tx));
	m_spg->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_spg->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

	ADDRESS_MAP_BANK(config, m_bankdev);
	m_bankdev->set_addrmap(AS_PROGRAM, &vsmile_state::banked_map);
	m_bankdev->set_endianness(ENDIANNESS_LITTLE);
	m_bankdev->set_data_width(16);
	m_bankdev->set_shift(-1);
	m_bankdev->set_stride(0x400000);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "vsmile_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(device_image_load_delegate(&vsmile_state::device_image_load_cart, this));

	SOFTWARE_LIST(config, "cart_list").set_original("vsmile_cart");
}

void vsmile_state::vsmilep(machine_config &config)
{
	vsmile(config);
	m_spg->set_pal(true);
}

ROM_START( vsmile )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vsmilebios.bin", 0x000000, 0x200000, CRC(11f1b416) SHA1(11f77c4973d29c962567390e41879c86a759c93b) )
ROM_END

ROM_START( vsmileg )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "bios german.bin", 0x000000, 0x200000, CRC(205c5296) SHA1(7fbcf761b5885c8b1524607aabaf364b4559c8cc) )
ROM_END

ROM_START( vsmilef )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "sysrom_france", 0x000000, 0x200000, CRC(0cd0bdf5) SHA1(5c8d1eada1b6b545555b8d2b09325d7127681af8) )
ROM_END

ROM_START( vsmileb )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vbabybios.bin", 0x000000, 0x800000, CRC(ddc7f845) SHA1(2c17d0f54200070176d03d44a40c7923636e596a) )
ROM_END

// year, name, parent, compat, machine, input, class, init, company, fullname, flags
CONS( 2005, vsmile,  0,      0, vsmile,  vsmile, vsmile_state, empty_init, "VTech", "V.Smile (US)",      MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
CONS( 2005, vsmileg, vsmile, 0, vsmilep, vsmile, vsmile_state, empty_init, "VTech", "V.Smile (Germany)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
CONS( 2005, vsmilef, vsmile, 0, vsmilep, vsmile, vsmile_state, empty_init, "VTech", "V.Smile (France)",  MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
CONS( 2005, vsmileb, 0,      0, vsmile,  vsmile, vsmile_state, empty_init, "VTech", "V.Smile Baby (US)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
