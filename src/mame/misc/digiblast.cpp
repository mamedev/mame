// license:BSD-3-Clause
// copyright-holders:David Haywood, Ryan Holtz, Peter Wilhelmsen
/*
    Nikko / Grey Innovation digiBLAST (2005)

    S3C2410 ARM920T, 16 MiB SDRAM, NAND cartridge, CS43L43 audio DAC.
    The cartridge supplies U-Boot, Linux and the application.  The active-low
    keys use GPF/GPG external interrupts; GPH8 detects the TV connection.

    TODO: verify cartridge part numbers and spare-area dumps, EEPROM wiring,
    battery sensing, TV encoder, USB accessories and LCD timing.

*/

// FIXME: Gormiti Masters of the Gorm Island runs too fast in LCD mode.

#include "emu.h"

#include "bus/digiblast/slot.h"
#include "cpu/arm7/arm7.h"
#include "machine/s3c24xx.h"
#include "sound/cs43l43.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#define LOG_ADC     (1U << 2)
#define LOG_GPIO    (1U << 3)
#define LOG_I2C     (1U << 4)
#define VERBOSE (0)
#include "logmacro.h"

namespace {

class digiblast_state : public driver_device
{
public:
	digiblast_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_s3c2410(*this, "s3c2410"),
		m_i2sdac(*this, "i2sdac"),
		m_cart(*this, "cartslot"),
		m_buttons(*this, "GPIO%c", 'F'),
		m_display(*this, "DISPLAY"),
		m_dac_sda(1)
	{
	}

	void digiblast(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(key_changed);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void main_map(address_map &map) ATTR_COLD;

	uint32_t adc_data_r(offs_t offset);
	void i2c_scl_w(int state);
	void i2c_sda_cb(int state);
	int i2c_sda_r();
	void i2c_sda_w(int state);
	uint32_t gpio_port_r(offs_t offset);

	required_device<cpu_device> m_maincpu;
	required_device<s3c2410_device> m_s3c2410;
	required_device<cs43l43_device> m_i2sdac;

	required_device<digiblast_cart_slot_device> m_cart;
	required_ioport_array<2> m_buttons;
	required_ioport m_display;

	u8 m_dac_sda;
};

void digiblast_state::main_map(address_map &map)
{
	map(0x30000000, 0x30ffffff).mirror(0x02000000).ram();
	map(0x40000000, 0x5fffffff).m(m_s3c2410, FUNC(s3c2410_device::map));
}

static INPUT_PORTS_START( digiblast )
	PORT_START("GPIOF")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("A") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("L") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("R") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("B") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 6)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 7)

	PORT_START("GPIOG")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Previous") PORT_CODE(KEYCODE_COMMA) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 8)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_START) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 16)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Play / Pause") PORT_CODE(KEYCODE_P) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 17)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Stop") PORT_CODE(KEYCODE_S) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 18)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_SELECT) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 19)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("C") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 20)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Next") PORT_CODE(KEYCODE_STOP) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digiblast_state::key_changed), 21)
	PORT_BIT(0xc0fe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DISPLAY")
	PORT_CONFNAME(0x0100, 0x0100, "Display Connection")
	PORT_CONFSETTING(0x0100, "LCD")
	PORT_CONFSETTING(0x0000, "TV")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(digiblast_state::key_changed)
{
	m_s3c2410->gpio_set_extint(param, newval ? 1 : 0);
}

void digiblast_state::machine_start()
{
	save_item(NAME(m_dac_sda));
}

uint32_t digiblast_state::adc_data_r(offs_t offset)
{
	const uint32_t data = 0x3ff;
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_ADC, "%s: adc_data_r(%d): %08x\n", machine().describe_context(), offset, data);
	return data;
}

void digiblast_state::i2c_scl_w(int state)
{
	LOGMASKED(LOG_I2C, "%s: i2c_scl_w: %d\n", machine().describe_context(), state);
	m_cart->i2c_scl_w(state);
	m_i2sdac->scl_write(state);
}

void digiblast_state::i2c_sda_cb(int state)
{
	m_dac_sda = state;
}

int digiblast_state::i2c_sda_r()
{
	const bool nvram_state = m_cart->i2c_sda_r();
	const bool state = nvram_state && m_dac_sda;
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_I2C, "%s: i2c_sda_r: %d\n", machine().describe_context(), state);
	return state;
}

void digiblast_state::i2c_sda_w(int state)
{
	LOGMASKED(LOG_I2C, "%s: i2c_sda_w: %d\n", machine().describe_context(), state);
	m_cart->i2c_sda_w(state);
	m_i2sdac->sda_write(state);
}

uint32_t digiblast_state::gpio_port_r(offs_t offset)
{
	uint32_t data = 0;
	switch (offset)
	{
		case s3c24xx_device::GPIO_PORT_A:
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_GPIO, "%s: gpio_port_r port A: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_B:
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_GPIO, "%s: gpio_port_r port B: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_C:
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_GPIO, "%s: gpio_port_r port C: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_D:
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_GPIO, "%s: gpio_port_r port D: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_E:
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_GPIO, "%s: gpio_port_r port E: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_F:
			data = m_buttons[0]->read();
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_GPIO, "%s: gpio_port_r port F: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_G:
			data = m_buttons[1]->read();
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_GPIO, "%s: gpio_port_r port G: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_H:
			data = m_display->read();
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_GPIO, "%s: gpio_port_r port H: %08x\n", machine().describe_context(), data);
			break;
	}
	return data;
}

void digiblast_state::digiblast(machine_config &config)
{
	/* basic machine hardware */
	ARM920T(config, m_maincpu, 200'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &digiblast_state::main_map);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(455, 262);
	screen.set_visarea(0, 240-1, 0, 160-1);
	screen.set_screen_update(m_s3c2410, FUNC(s3c2410_device::screen_update));

	PALETTE(config, "palette").set_entries(256);

	S3C2410(config, m_s3c2410, 12_MHz_XTAL);
	m_s3c2410->set_palette_tag("palette");
	m_s3c2410->set_screen_tag("screen");
	m_s3c2410->om0_r_callback().set_constant(0); // OM[1:0] = 00: Enable NAND flash memory boot
	m_s3c2410->om1_r_callback().set_constant(0);
	m_s3c2410->ncon_r_callback().set_constant(1); // 512-byte pages, four address cycles during NAND boot
	m_s3c2410->adc_data_r_callback().set(FUNC(digiblast_state::adc_data_r));
	m_s3c2410->i2c_scl_w_callback().set(FUNC(digiblast_state::i2c_scl_w));
	m_s3c2410->i2c_sda_r_callback().set(FUNC(digiblast_state::i2c_sda_r));
	m_s3c2410->i2c_sda_w_callback().set(FUNC(digiblast_state::i2c_sda_w));
	m_s3c2410->i2s_data_w_callback().set(m_i2sdac, FUNC(cs43l43_device::write_i2s));
	m_s3c2410->i2s_clockchange_w_callback().set(m_i2sdac, FUNC(cs43l43_device::set_frequency));
	m_s3c2410->gpio_port_r_callback().set(FUNC(digiblast_state::gpio_port_r));
	m_s3c2410->nand_command_w_callback().set(m_cart, FUNC(digiblast_cart_slot_device::command_w));
	m_s3c2410->nand_address_w_callback().set(m_cart, FUNC(digiblast_cart_slot_device::address_w));
	m_s3c2410->nand_update_ecc_r_callback().set_constant(1);
	m_s3c2410->nand_data_r_callback().set(m_cart, FUNC(digiblast_cart_slot_device::data_r));
	m_s3c2410->nand_data_w_callback().set(m_cart, FUNC(digiblast_cart_slot_device::data_w));

	DIGIBLAST_CART_SLOT(config, m_cart, digiblast_cart);
	m_cart->set_must_be_loaded(true);
	m_cart->ready_callback().set(m_s3c2410, FUNC(s3c2410_device::frnb_w));

	SOFTWARE_LIST(config, "cart_list").set_original("digiblast_cart");

	/* sound hardware */
	SPEAKER(config, "speakerl").front_left();
	SPEAKER(config, "speakerr").front_right();

	CS43L43(config, m_i2sdac, 0);
	m_i2sdac->sda_callback().set(FUNC(digiblast_state::i2c_sda_cb));
	m_i2sdac->add_route(0, "speakerl", 0.7);
	m_i2sdac->add_route(1, "speakerr", 0.7);
}

ROM_START( digiblst )
ROM_END

} // Anonymous namespace

CONS( 2005, digiblst,  0, 0, digiblast,  digiblast,  digiblast_state, empty_init, "Nikko Entertainment B.V. / Grey Innovation", "digiBLAST", MACHINE_IMPERFECT_TIMING | MACHINE_SUPPORTS_SAVE )
