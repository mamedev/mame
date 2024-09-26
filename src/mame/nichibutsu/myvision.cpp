// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Nichibutsu My Vision
      driver by Wilbert Pol

        2013/12/01 Skeleton driver.
        2013/12/02 Working driver.

    Known issues:
    - The inputs sometimes feel a bit unresponsive. Was the real unit like
      that? Or is it just because we have incorrect clocks?

    TODO:
    - Review software list
    - Add clickable artwork
    - Verify sound chip model
    - Verify exact TMS9918 model
    - Verify clock crystal(s)
    - Verify size of vram

****************************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/tms9928a.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist_dev.h"
#include "speaker.h"


namespace {

class myvision_state : public driver_device
{
public:
	myvision_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_io_row0(*this, "ROW0")
		, m_io_row1(*this, "ROW1")
		, m_io_row2(*this, "ROW2")
		, m_io_row3(*this, "ROW3")
	{ }

	void myvision(machine_config &config);

private:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( cart_load );
	uint8_t ay_port_a_r();
	uint8_t ay_port_b_r();
	void ay_port_a_w(uint8_t data);
	void ay_port_b_w(uint8_t data);

	void myvision_io(address_map &map) ATTR_COLD;
	void myvision_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	uint8_t m_column;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_row2;
	required_ioport m_io_row3;
};


void myvision_state::myvision_mem(address_map &map)
{
	map.unmap_value_high();
	//map(0x0000, 0x5fff)      // mapped by the cartslot
	map(0xa000, 0xa7ff).ram();
	map(0xe000, 0xe000).rw("tms9918", FUNC(tms9918a_device::vram_read), FUNC(tms9918a_device::vram_write));
	map(0xe002, 0xe002).rw("tms9918", FUNC(tms9918a_device::register_read), FUNC(tms9918a_device::register_write));
}


void myvision_state::myvision_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w("ay8910", FUNC(ay8910_device::address_w));
	map(0x01, 0x01).w("ay8910", FUNC(ay8910_device::data_w));
	map(0x02, 0x02).r("ay8910", FUNC(ay8910_device::data_r));
}


/* Input ports */
/*
  Keyboard layout is something like:
                       B
                  A          D    E
                       C
  1 2 3 4 5 6 7 8 9 10 11 12 13   14
 */
static INPUT_PORTS_START( myvision )
	PORT_START("ROW0")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M) PORT_NAME("13")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_NAME("C/Down")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_I) PORT_NAME("9")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_MAHJONG_E) PORT_NAME("5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_MAHJONG_A) PORT_NAME("1")

	PORT_START("ROW1")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_NAME("B/Up")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_L) PORT_NAME("12")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_MAHJONG_H) PORT_NAME("8")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_MAHJONG_D) PORT_NAME("4")

	PORT_START("ROW2")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N) PORT_NAME("14/Start")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("D/Right")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_J) PORT_NAME("10")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_MAHJONG_F) PORT_NAME("6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_MAHJONG_B) PORT_NAME("2")

	PORT_START("ROW3")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_NAME("A/Left")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("E")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_K) PORT_NAME("11")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_MAHJONG_G) PORT_NAME("7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_MAHJONG_C) PORT_NAME("3")

INPUT_PORTS_END


void myvision_state::machine_start()
{
	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x5fff, read8sm_delegate(*m_cart, FUNC(generic_slot_device::read_rom)));

	save_item(NAME(m_column));
}


void myvision_state::machine_reset()
{
	m_column = 0xff;
}


DEVICE_IMAGE_LOAD_MEMBER( myvision_state::cart_load )
{
	uint32_t const size = m_cart->common_get_size("rom");

	if (size != 0x4000 && size != 0x6000)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be 16K or 24K)");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}


uint8_t myvision_state::ay_port_a_r()
{
	uint8_t data = 0xff;

	if (!(m_column & 0x80))
		data &= m_io_row0->read();

	if (!(m_column & 0x40))
		data &= m_io_row1->read();

	if (!(m_column & 0x20))
		data &= m_io_row2->read();

	if (!(m_column & 0x10))
		data &= m_io_row3->read();

	return data;
}


uint8_t myvision_state::ay_port_b_r()
{
	return 0xff;
}


void myvision_state::ay_port_a_w(uint8_t data)
{
}


// Upper 4 bits select column
void myvision_state::ay_port_b_w(uint8_t data)
{
	m_column = data;
}

void myvision_state::myvision(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(10'738'635)/3);  /* Not verified */
	m_maincpu->set_addrmap(AS_PROGRAM, &myvision_state::myvision_mem);
	m_maincpu->set_addrmap(AS_IO, &myvision_state::myvision_io);

	/* video hardware */
	tms9918a_device &vdp(TMS9918A(config, "tms9918", XTAL(10'738'635)));  /* Exact model not verified */
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);  /* Not verified */
	vdp.int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay8910(AY8910(config, "ay8910", XTAL(10'738'635)/3/2));  /* Exact model and clock not verified */
	ay8910.port_a_read_callback().set(FUNC(myvision_state::ay_port_a_r));
	ay8910.port_b_read_callback().set(FUNC(myvision_state::ay_port_b_r));
	ay8910.port_a_write_callback().set(FUNC(myvision_state::ay_port_a_w));
	ay8910.port_b_write_callback().set(FUNC(myvision_state::ay_port_b_w));
	ay8910.add_route(ALL_OUTPUTS, "mono", 0.50);

	/* cartridge */
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "myvision_cart"));
	cartslot.set_device_load(FUNC(myvision_state::cart_load));
	//cartslot.set_must_be_loaded(true);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("myvision");
}

/* ROM definition */
ROM_START( myvision )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     STATE           INIT        COMPANY       FULLN AME              FLAGS
CONS( 1983, myvision, 0,      0,      myvision, myvision, myvision_state, empty_init, "Nichibutsu", "My Vision (KH-1000)", 0 )
