// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

#include "emu.h"
#include "spg2xx.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist_dev.h"


namespace {

class vii_state : public spg2xx_game_state
{
public:
	vii_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_cart(*this, "cartslot"),
		m_io_motionx(*this, "MOTIONX"),
		m_io_motiony(*this, "MOTIONY"),
		m_io_motionz(*this, "MOTIONZ"),
		m_cart_region(nullptr),
		m_ctrl_poll_timer(nullptr)
	{ }

	void vii(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void vii_portb_w(uint16_t data);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load_vii);

	TIMER_CALLBACK_MEMBER(poll_controls);

	required_device<generic_slot_device> m_cart;
	required_ioport m_io_motionx;
	required_ioport m_io_motiony;
	required_ioport m_io_motionz;
	memory_region *m_cart_region;

	emu_timer *m_ctrl_poll_timer;
	uint8_t m_controller_input[8];
};


void vii_state::vii_portb_w(uint16_t data)
{
	switch_bank(((data & 0x80) >> 7) | ((data & 0x20) >> 4));
}


static INPUT_PORTS_START( vii )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("Button A")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Button B")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("Button C")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("Button D")

	PORT_START("MOTIONX")
	PORT_BIT( 0x3ff, 0x200, IPT_PADDLE ) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(50) PORT_KEYDELTA(50) PORT_NAME("Motion Control X")

	PORT_START("MOTIONY")
	PORT_BIT( 0x3ff, 0x200, IPT_PADDLE ) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(50) PORT_KEYDELTA(50) PORT_NAME("Motion Control Y") PORT_PLAYER(2)

	PORT_START("MOTIONZ")
	PORT_BIT( 0x3ff, 0x200, IPT_PADDLE ) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(50) PORT_KEYDELTA(50) PORT_NAME("Motion Control Z") PORT_PLAYER(3)
INPUT_PORTS_END

void vii_state::machine_start()
{
	spg2xx_game_state::machine_start();

	// if there's a cart, override the standard banking
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
		m_bank->configure_entries(0, (m_cart_region->bytes() + 0x7fffff) / 0x800000, m_cart_region->base(), 0x800000);
		m_bank->set_entry(0);
	}

	m_ctrl_poll_timer = timer_alloc(FUNC(vii_state::poll_controls), this);
	m_ctrl_poll_timer->adjust(attotime::never);

	save_item(NAME(m_controller_input));
}

void vii_state::machine_reset()
{
	spg2xx_game_state::machine_reset();

	m_controller_input[0] = 0;
	m_controller_input[4] = 0;
	m_controller_input[6] = 0xff;
	m_controller_input[7] = 0;

	m_ctrl_poll_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
}


TIMER_CALLBACK_MEMBER(vii_state::poll_controls)
{
	int32_t x = m_io_motionx ? ((int32_t)m_io_motionx->read() - 0x200) : 0;
	int32_t y = m_io_motiony ? ((int32_t)m_io_motiony->read() - 0x200) : 0;
	int32_t z = m_io_motionz ? ((int32_t)m_io_motionz->read() - 0x200) : 0;

	uint8_t old_input[8];
	memcpy(old_input, m_controller_input, 8);

	m_controller_input[0] = m_io_p1->read();
	m_controller_input[1] = (uint8_t)x;
	m_controller_input[2] = (uint8_t)y;
	m_controller_input[3] = (uint8_t)z;
	m_controller_input[4] = 0;
	x = (x >> 8) & 3;
	y = (y >> 8) & 3;
	z = (z >> 8) & 3;
	m_controller_input[5] = (z << 4) | (y << 2) | x;
	m_controller_input[6] = 0xff;
	m_controller_input[7] = 0;

	if (memcmp(old_input, m_controller_input, 8))
	{
		for(int i = 0; i < 8; i++)
			m_maincpu->uart_rx(m_controller_input[i]);
	}
}

DEVICE_IMAGE_LOAD_MEMBER(vii_state::cart_load_vii)
{
	uint32_t const size = m_cart->common_get_size("rom");

	if (size < 0x80'0000)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be at least 8M)");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void vii_state::vii(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &vii_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->portb_out().set(FUNC(vii_state::vii_portb_w));
	m_maincpu->i2c_w().set(FUNC(vii_state::i2c_w));
	m_maincpu->i2c_r().set(FUNC(vii_state::i2c_r));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "vii_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(vii_state::cart_load_vii));

	SOFTWARE_LIST(config, "vii_cart").set_original("vii");
}

ROM_START( vii )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "vii.bin", 0x0000, 0x2000000, CRC(04627639) SHA1(f883a92d31b53c9a5b0cdb112d07cd793c95fc43))
ROM_END

} // anonymous namespace


// Jungle Soft TV games
CONS( 2007, vii,      0, 0, vii,        vii,      vii_state,         empty_init,      "Jungle Soft / KenSingTon / Siatronics",       "Vii",         MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // motion controls are awkward, but playable for the most part
