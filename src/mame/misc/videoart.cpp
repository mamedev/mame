// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

LJN Video Art

It's a toy for drawing/coloring pictures on the tv, not a video game console.
Picture libraries were available on separate cartridges.

On the splash screen, press CLEAR to start drawing (no need to wait half a minute).

Hardware notes:
- EF6805R2P @ 3.57Mhz (14.318MHz XTAL)
- EF9367P @ 1.507MHz
- TSGB01019ACP unknown 48-pin DIP, interfaces with EF9367P and DRAM
- 2*D41416C-15 (16Kbit*4) DRAM
- 36-pin cartridge slot, 8KB or 16KB ROM
- DB9 joystick port, no known peripherals other than the default analog joystick
- RF NTSC video, no sound

TODO:
- drawing doesn't work how it's supposed to: holding down the draw button should
  draw/erase, otherwise it should just move the cursor
- background color is wrong, this also causes it to scribble preset drawings on
  top of eachother, changing each screen_scanning(1) in video/ef9365.cpp to
  screen_scanning(0) makes it look ok
- vectors on default splash screen should be thicker, horizontal resolution is
  probably 128 instead of 512, which also makes more sense with the amount of
  RAM it has (compressing the width down to 128 crudely introduces other bugs)
- palette is approximated from photos/videos

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6805/m68705.h"
#include "machine/timer.h"
#include "video/ef9365.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"


namespace {

class videoart_state : public driver_device
{
public:
	videoart_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ef9367(*this, "ef9367"),
		m_screen(*this, "screen"),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "IN%u", 0),
		m_led(*this, "led")
	{ }

	void videoart(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<m6805r2_device> m_maincpu;
	required_device<ef9365_device> m_ef9367;
	required_device<screen_device> m_screen;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<3> m_inputs;
	output_finder<> m_led;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline) { m_ef9367->update_scanline(param); }

	void porta_w(u8 data);
	u8 porta_r();
	void portb_w(u8 data);
	void portc_w(u8 data);
	u8 portd_r();

	u8 m_porta = 0xff;
	u8 m_portb = 0xff;
	u8 m_portc = 0xff;
	u8 m_rdata = 0xff;
	u8 m_romlatch = 0;
	u8 m_ccount = 0;
	u8 m_cprev = 0;
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void videoart_state::machine_start()
{
	m_led.resolve();

	// register for savestates
	save_item(NAME(m_porta));
	save_item(NAME(m_portb));
	save_item(NAME(m_portc));
	save_item(NAME(m_rdata));
	save_item(NAME(m_romlatch));
	save_item(NAME(m_ccount));
	save_item(NAME(m_cprev));
}

void videoart_state::video_start()
{
	// initialize palette (there is no color prom)
	static const rgb_t colors[] =
	{
		{ 0x00, 0x00, 0x00 }, // 2 black
		{ 0x20, 0x18, 0x90 }, // 7 blue
		{ 0x00, 0x60, 0x00 }, // 3 dark green
		{ 0xff, 0xff, 0xff }, // 0 white
		{ 0x50, 0x20, 0x28 }, // b dark pink
		{ 0x40, 0x10, 0x50 }, // 8 purple
		{ 0x30, 0x28, 0x08 }, // a brown
		{ 0x40, 0x10, 0x10 }, // d dark red
		{ 0x80, 0x80, 0x80 }, // 1 gray
		{ 0x68, 0xa8, 0xff }, // 6 cyan
		{ 0x68, 0xb0, 0x18 }, // 4 lime green
		{ 0x48, 0xb0, 0x20 }, // 5 green
		{ 0xff, 0x78, 0xff }, // c pink
		{ 0xd8, 0x78, 0xff }, // 9 lilac
		{ 0xd0, 0x78, 0x20 }, // f orange
		{ 0xe0, 0x60, 0x58 }  // e light red
	};

	for (int i = 0; i < 16; i++)
		m_ef9367->set_color_entry(i, colors[i] >> 16 & 0xff, colors[i] >> 8 & 0xff, colors[i] >> 0 & 0xff);
}

DEVICE_IMAGE_LOAD_MEMBER(videoart_state::cart_load)
{
	u32 size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}



/*******************************************************************************
    I/O
*******************************************************************************/

void videoart_state::porta_w(u8 data)
{
	// A0-A7: EF9367 data
	// A0,A1: TSG command
	m_porta = data;
}

u8 videoart_state::porta_r()
{
	u8 data = 0xff;

	// read cartridge data
	if (~m_portb & 0x10)
	{
		u16 offset = m_romlatch << 8 | m_portc;
		data &= m_cart->read_rom(offset);
	}

	// read EF9367 data
	if (~m_portb & 1)
		data &= m_rdata;

	return data;
}

void videoart_state::portb_w(u8 data)
{
	// B0: EF9367 E
	if (~data & 1 && m_portb & 1)
	{
		if (m_portc & 0x10)
			m_rdata = m_ef9367->data_r(m_portc & 0xf);
		else
			m_ef9367->data_w(m_portc & 0xf, m_porta);
	}

	// B1: clock ROM address latch
	if (data & 2 && ~m_portb & 2)
		m_romlatch = m_portc;

	// B2: custom chip handling
	if (~data & 4 && m_portb & 4)
	{
		u8 cmd = m_porta & 3;

		// reset count
		if (~data & 2)
			m_ccount = 0;

		switch (m_ccount)
		{
			// direct command?
			case 0:
				m_ef9367->data_w(0, cmd);
				break;

			// change pen color
			case 3:
				m_ef9367->set_color_filler(cmd << 2 | m_cprev);
				break;

			default:
				break;
		}

		m_ccount++;
		m_cprev = cmd;
	}

	// B3: erase led
	m_led = BIT(~data, 3);

	// B4: ROM _OE
	// B5-B7: input mux
	m_portb = data;
}

void videoart_state::portc_w(u8 data)
{
	// C0-C7: ROM address
	// C0-C3: EF9367 address
	// C4: EF9367 R/W
	m_portc = data;
}

u8 videoart_state::portd_r()
{
	u8 data = 0;

	// D6,D7: multiplexed inputs
	for (int i = 0; i < 3; i++)
		if (!BIT(m_portb, 5 + i))
			data |= m_inputs[i]->read();

	return ~data;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( videoart )
	PORT_START("IN0")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Page")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Clear") // actually 2 buttons

	PORT_START("IN1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Horizontal")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Verical")

	PORT_START("IN2")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Draw")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Erase")

	PORT_START("AN0")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(2) PORT_NAME("Color")

	PORT_START("AN1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("AN2")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_REVERSE
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void videoart_state::videoart(machine_config &config)
{
	// basic machine hardware
	M6805R2(config, m_maincpu, 14.318181_MHz_XTAL / 4);
	m_maincpu->porta_w().set(FUNC(videoart_state::porta_w));
	m_maincpu->porta_r().set(FUNC(videoart_state::porta_r));
	m_maincpu->portb_w().set(FUNC(videoart_state::portb_w));
	m_maincpu->portc_w().set(FUNC(videoart_state::portc_w));
	m_maincpu->portd_r().set(FUNC(videoart_state::portd_r));
	m_maincpu->portan_r<0>().set_ioport("AN0");
	m_maincpu->portan_r<1>().set_ioport("AN1");
	m_maincpu->portan_r<2>().set_ioport("AN2");

	// video hardware
	PALETTE(config, "palette").set_entries(16);

	EF9365(config, m_ef9367, (14.318181_MHz_XTAL * 2) / 19);
	m_ef9367->set_palette_tag("palette");
	m_ef9367->set_nb_bitplanes(4);
	m_ef9367->set_display_mode(ef9365_device::DISPLAY_MODE_512x256);
	m_ef9367->irq_handler().set_inputline(m_maincpu, M6805_IRQ_LINE);

	TIMER(config, "scanline").configure_scanline(FUNC(videoart_state::scanline), "screen", 0, 1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_screen_update(m_ef9367, FUNC(ef9365_device::screen_update));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 48, 256-1);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "videoart");
	m_cart->set_device_load(FUNC(videoart_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("videoart");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( videoart )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("ljd091.u6", 0x0000, 0x1000, CRC(111ad7d4) SHA1(dec751069a6713ec2e033aed5657378ccfcddebb) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1987, videoart, 0,      0,      videoart, videoart, videoart_state, empty_init, "LJN Toys", "Video Art", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
