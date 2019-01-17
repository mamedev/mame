// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    V-Tech V.Smile console emulation
    V-Tech V.Smile Baby console emulation

    Similar Systems:

        V.Smile Pocket
        V.Smile Cyber Pocket
        V.Smile PC Pal
        V-Motion Active Learning System
        V.Flash
        V.Baby
        Leapfrog Leapster

*******************************************************************************/

#include "emu.h"

#include "bus/vsmile/vsmile_slot.h"
#include "bus/vsmile/rom.h"

#include "cpu/unsp/unsp.h"

#include "machine/bankdev.h"
#include "machine/spg2xx.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class vsmile_base_state : public driver_device
{
public:
	vsmile_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_spg(*this, "spg")
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_cart(*this, "cartslot")
		, m_bankdev(*this, "bank")
		, m_system_region(*this, "maincpu")
	{ }

	void vsmile_base(machine_config &config);

protected:
	void mem_map(address_map &map);

	required_device<spg2xx_device> m_spg;
	required_device<unsp_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<vsmile_cart_slot_device> m_cart;
	required_device<address_map_bank_device> m_bankdev;
	required_memory_region m_system_region;
};

class vsmile_state : public vsmile_base_state
{
public:
	vsmile_state(const machine_config &mconfig, device_type type, const char *tag)
		: vsmile_base_state(mconfig, type, tag)
		, m_io_joy(*this, "JOY")
		, m_io_colors(*this, "COLORS")
		, m_io_buttons(*this, "BUTTONS")
		, m_dsw_region(*this, "REGION")
		, m_uart_tx_timer(nullptr)
		, m_pad_timer(nullptr)
	{ }

	void vsmile(machine_config &config);
	void vsmilep(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(pad_joy_changed);
	DECLARE_INPUT_CHANGED_MEMBER(pad_color_changed);
	DECLARE_INPUT_CHANGED_MEMBER(pad_button_changed);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static const device_timer_id TIMER_UART_TX = 0;
	static const device_timer_id TIMER_PAD = 1;

	void banked_map(address_map &map);

	DECLARE_READ16_MEMBER(portb_r);
	DECLARE_READ16_MEMBER(portc_r);
	DECLARE_WRITE16_MEMBER(portc_w);

	DECLARE_WRITE8_MEMBER(chip_sel_w);

	DECLARE_WRITE8_MEMBER(uart_rx);
	void uart_tx_fifo_push(uint8_t data);
	void handle_uart_tx();

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
		VSMILE_PORTC_SYSRESET = 0x80,

		XMIT_STATE_IDLE       = 0,
		XMIT_STATE_RTS        = 1,
		XMIT_STATE_CTS        = 2
	};

	required_ioport m_io_joy;
	required_ioport m_io_colors;
	required_ioport m_io_buttons;
	required_ioport m_dsw_region;

	bool m_ctrl_cts[2];
	bool m_ctrl_rts[2];
	uint8_t m_ctrl_probe_history[2];
	uint8_t m_ctrl_probe_count;
	uint8_t m_uart_tx_fifo[32]; // arbitrary size
	uint8_t m_uart_tx_fifo_start;
	uint8_t m_uart_tx_fifo_end;
	uint8_t m_uart_tx_fifo_count;
	emu_timer *m_uart_tx_timer;
	int m_uart_tx_state;

	emu_timer *m_pad_timer;
};

class vsmileb_state : public vsmile_base_state
{
public:
	vsmileb_state(const machine_config &mconfig, device_type type, const char *tag)
		: vsmile_base_state(mconfig, type, tag)
	{ }

	void vsmileb(machine_config &config);

private:
	virtual void machine_start() override;

	void banked_map(address_map &map);

	DECLARE_READ16_MEMBER(porta_r);
	DECLARE_READ16_MEMBER(portb_r);
	DECLARE_READ16_MEMBER(portc_r);
	DECLARE_WRITE16_MEMBER(porta_w);
	DECLARE_WRITE16_MEMBER(portb_w);
	DECLARE_WRITE16_MEMBER(portc_w);
	DECLARE_WRITE8_MEMBER(chip_sel_w);
};

/************************************
 *
 *  Common
 *
 ************************************/

void vsmile_base_state::mem_map(address_map &map)
{
	map(0x000000, 0x3fffff).rw(m_bankdev, FUNC(address_map_bank_device::read16), FUNC(address_map_bank_device::write16));
	map(0x000000, 0x003fff).m(m_spg, FUNC(spg2xx_device::map));
}

/************************************
 *
 *  V.Smile
 *
 ************************************/

void vsmile_state::machine_start()
{
	vsmile_base_state::machine_start();

	m_bankdev->set_bank(m_cart && m_cart->exists() ? 4 : 0);

	m_pad_timer = timer_alloc(TIMER_PAD);
	m_pad_timer->adjust(attotime::never);

	m_uart_tx_timer = timer_alloc(TIMER_UART_TX);
	m_uart_tx_timer->adjust(attotime::never);

	save_item(NAME(m_ctrl_cts));
	save_item(NAME(m_ctrl_rts));
	save_item(NAME(m_ctrl_probe_history));
	save_item(NAME(m_ctrl_probe_count));
	save_item(NAME(m_uart_tx_fifo));
	save_item(NAME(m_uart_tx_fifo_start));
	save_item(NAME(m_uart_tx_fifo_end));
	save_item(NAME(m_uart_tx_fifo_count));
}

void vsmile_state::machine_reset()
{
	m_pad_timer->adjust(attotime::from_hz(1), 0, attotime::from_hz(1));
	m_uart_tx_timer->adjust(attotime::from_hz(9600/10), 0, attotime::from_hz(9600/10));

	memset(m_ctrl_cts, 0, sizeof(bool) * 2);
	memset(m_ctrl_rts, 0, sizeof(bool) * 2);
	memset(m_ctrl_probe_history, 0, 2);
	m_ctrl_probe_count = 0;
	memset(m_uart_tx_fifo, 0, 32);
	m_uart_tx_fifo_start = 0;
	m_uart_tx_fifo_end = 0;
	m_uart_tx_fifo_count = 0;
	m_uart_tx_state = XMIT_STATE_IDLE;
}

void vsmile_state::uart_tx_fifo_push(uint8_t data)
{
	if (m_uart_tx_fifo_count == ARRAY_LENGTH(m_uart_tx_fifo))
	{
		logerror("Warning: Trying to push more than %d bytes onto the controller Tx FIFO, data will be lost\n", ARRAY_LENGTH(m_uart_tx_fifo));
	}

	m_uart_tx_fifo[m_uart_tx_fifo_end] = data;
	m_uart_tx_fifo_count++;
	m_uart_tx_fifo_end = (m_uart_tx_fifo_end + 1) % ARRAY_LENGTH(m_uart_tx_fifo);
}

void vsmile_state::handle_uart_tx()
{
	if (m_uart_tx_fifo_count == 0)
		return;

	if (m_uart_tx_state == XMIT_STATE_IDLE)
	{
		m_uart_tx_state = XMIT_STATE_RTS;
		m_ctrl_rts[0] = true;
		m_spg->extint_w(0, true);
		return;
	}

	m_spg->uart_rx(m_uart_tx_fifo[m_uart_tx_fifo_start]);
	m_uart_tx_fifo_start = (m_uart_tx_fifo_start + 1) % ARRAY_LENGTH(m_uart_tx_fifo);
	m_uart_tx_fifo_count--;
	if (m_uart_tx_fifo_count == 0)
	{
		m_uart_tx_state = XMIT_STATE_IDLE;
		m_ctrl_rts[0] = false;
		m_spg->extint_w(0, false);
		//m_uart_tx_timer->adjust(attotime::never);
	}
}

WRITE8_MEMBER(vsmile_state::uart_rx)
{
	if ((data >> 4) == 7 || (data >> 4) == 11)
	{
		m_ctrl_probe_history[0] = m_ctrl_probe_history[1];
		m_ctrl_probe_history[1] = data;
		const uint8_t response = ((m_ctrl_probe_history[0] + m_ctrl_probe_history[1] + 0x0f) & 0x0f) ^ 0x05;
		uart_tx_fifo_push(0xb0 | response);
	}
}

void vsmile_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_UART_TX:
		handle_uart_tx();
		break;
	case TIMER_PAD:
		uart_tx_fifo_push(0x55);
		break;
	default:
		logerror("Unknown timer ID: %d\n", id);
		break;
	}
}

READ16_MEMBER(vsmile_state::bank3_r)
{
	return ((uint16_t*)m_system_region->base())[offset];
}

READ16_MEMBER(vsmile_state::portb_r)
{
	return VSMILE_PORTB_OFF_SW | VSMILE_PORTB_ON_SW | VSMILE_PORTB_RESET;
}

READ16_MEMBER(vsmile_state::portc_r)
{
	uint16_t data = m_dsw_region->read();
	data |= m_ctrl_rts[0] ? 0 : 0x0400;
	data |= m_ctrl_rts[1] ? 0 : 0x1000;
	data |= 0x2000;
	//data = machine().rand() & 0xffff;
	return data;
}

WRITE16_MEMBER(vsmile_state::portc_w)
{
	if (BIT(mem_mask, 8))
	{
		m_ctrl_cts[0] = BIT(data, 8);
		if (m_uart_tx_state == XMIT_STATE_RTS)
			m_uart_tx_state = XMIT_STATE_CTS;
	}
	if (BIT(mem_mask, 9))
	{
		m_ctrl_cts[1] = BIT(data, 9);
	}
}

INPUT_CHANGED_MEMBER(vsmile_state::pad_joy_changed)
{
	const uint8_t value = m_io_joy->read();

	if (BIT(value, 2))
		uart_tx_fifo_push(0xcf);
	else if (BIT(value, 3))
		uart_tx_fifo_push(0xc7);
	else
		uart_tx_fifo_push(0xc0);

	if (BIT(value, 0))
		uart_tx_fifo_push(0x87);
	else if (BIT(value, 1))
		uart_tx_fifo_push(0x8f);
	else
		uart_tx_fifo_push(0x80);
}

INPUT_CHANGED_MEMBER(vsmile_state::pad_color_changed)
{
	uart_tx_fifo_push(0x90 | m_io_colors->read());
}

INPUT_CHANGED_MEMBER(vsmile_state::pad_button_changed)
{
	const uint8_t value = m_io_buttons->read();
	const size_t bit = reinterpret_cast<size_t>(param);
	if (BIT(value, bit))
	{
		uart_tx_fifo_push(0xa1 + (uint8_t)bit);
	}
	else
	{
		uart_tx_fifo_push(0xa0);
	}
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
	m_maincpu->invalidate_cache();
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

	map(0x1000000, 0x13fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank0_r), FUNC(vsmile_cart_slot_device::bank0_w));

	map(0x1400000, 0x15fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank0_r), FUNC(vsmile_cart_slot_device::bank0_w));
	map(0x1600000, 0x17fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank2_r), FUNC(vsmile_cart_slot_device::bank2_w));

	map(0x1800000, 0x18fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank0_r), FUNC(vsmile_cart_slot_device::bank0_w));
	map(0x1900000, 0x19fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank1_r), FUNC(vsmile_cart_slot_device::bank1_w));
	map(0x1a00000, 0x1afffff).r(m_cart, FUNC(vsmile_cart_slot_device::bank2_r));
	map(0x1b00000, 0x1bfffff).r(FUNC(vsmile_state::bank3_r));
}

/************************************
 *
 *  V.Smile Baby
 *
 ************************************/

void vsmileb_state::machine_start()
{
	vsmile_base_state::machine_start();

	m_bankdev->set_bank(0);
}

READ16_MEMBER(vsmileb_state::porta_r)
{
	uint16_t data = 0x0380;
	logerror("%s: GPIO Port A Read: %04x\n", machine().describe_context(), data);
	return data;
}

READ16_MEMBER(vsmileb_state::portb_r)
{
	uint16_t data = 0x0080;
	logerror("%s: GPIO Port B Read: %04x\n", machine().describe_context(), data);
	return data;
}

READ16_MEMBER(vsmileb_state::portc_r)
{
	uint16_t data = 0;
	logerror("%s: GPIO Port C Read: %04x\n", machine().describe_context(), data);
	return data;
}

WRITE16_MEMBER(vsmileb_state::porta_w)
{
	logerror("%s: GPIO Port A Write: %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(vsmileb_state::portb_w)
{
	logerror("%s: GPIO Port B Write: %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(vsmileb_state::portc_w)
{
	logerror("%s: GPIO Port C Write: %04x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(vsmileb_state::chip_sel_w)
{
	logerror("%s: Chip Select Write: %d\n", machine().describe_context(), data);
	m_bankdev->set_bank(data);
}

void vsmileb_state::banked_map(address_map &map)
{
	map(0x0000000, 0x03fffff).rom().region("maincpu", 0);
	//map(0x0400000, 0x07fffff).rom().region("maincpu", 0);
	//map(0x0800000, 0x0bfffff).rom().region("maincpu", 0);
	//map(0x0000000, 0x03fffff).rom().region("maincpu", 0);
	//map(0x0100000, 0x01fffff).rom().region("maincpu", 0);
	//map(0x0200000, 0x02fffff).rom().region("maincpu", 0);
	//map(0x0300000, 0x03fffff).rom().region("maincpu", 0);

	map(0x0400000, 0x05fffff).rom().region("maincpu", 0);
	//map(0x0600000, 0x07fffff).nopr();
	//map(0x0200000, 0x02fffff).rom().region("maincpu", 0);
	//map(0x0300000, 0x03fffff).rom().region("maincpu", 0);
}

/************************************
 *
 *  Inputs
 *
 ************************************/

static INPUT_PORTS_START( vsmile )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_state, pad_joy_changed, 0) PORT_NAME("Joypad Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_state, pad_joy_changed, 0) PORT_NAME("Joypad Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_state, pad_joy_changed, 0) PORT_NAME("Joypad Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_state, pad_joy_changed, 0) PORT_NAME("Joypad Right")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COLORS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_state, pad_color_changed, 0) PORT_NAME("Green")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_state, pad_color_changed, 0) PORT_NAME("Blue")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_state, pad_color_changed, 0) PORT_NAME("Yellow")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_state, pad_color_changed, 0) PORT_NAME("Red")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_state, pad_button_changed, 0) PORT_NAME("OK")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_state, pad_button_changed, 1) PORT_NAME("Quit")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_state, pad_button_changed, 2) PORT_NAME("Help")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_state, pad_button_changed, 3) PORT_NAME("ABC")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("REGION")
	PORT_DIPNAME( 0x0f, 0x04, "BIOS Region" )
	PORT_DIPSETTING(    0x04, "UK/US" )
	PORT_DIPSETTING(    0x07, "China" )
	PORT_DIPSETTING(    0x08, "Mexico" )
	PORT_DIPSETTING(    0x0a, "Italy" )
	PORT_DIPSETTING(    0x0b, "Germany" )
	PORT_DIPSETTING(    0x0c, "Spain" )
	PORT_DIPSETTING(    0x0d, "France" )
	PORT_DIPNAME( 0x10, 0x10, "VTech Intro" )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x10, "Yes" )
	PORT_BIT( 0xe0, 0x00, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( vsmileb )
INPUT_PORTS_END

static void vsmile_cart(device_slot_interface &device)
{
	device.option_add_internal("vsmile_rom", VSMILE_ROM_STD);
	device.option_add_internal("vsmile_ram", VSMILE_ROM_RAM);
}

void vsmile_base_state::vsmile_base(machine_config &config)
{
	UNSP(config, m_maincpu, XTAL(27'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &vsmile_base_state::mem_map);
	m_maincpu->set_force_no_drc(true);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("spg", FUNC(spg2xx_device::screen_update));
	m_screen->screen_vblank().set(m_spg, FUNC(spg2xx_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SPG24X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);
	m_spg->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_spg->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

	ADDRESS_MAP_BANK(config, m_bankdev);
	m_bankdev->set_endianness(ENDIANNESS_LITTLE);
	m_bankdev->set_data_width(16);
	m_bankdev->set_shift(-1);
	m_bankdev->set_stride(0x400000);

	VSMILE_CART_SLOT(config, m_cart, vsmile_cart, nullptr);
}

void vsmile_state::vsmile(machine_config &config)
{
	vsmile_base(config);

	m_spg->portb_in().set(FUNC(vsmile_state::portb_r));
	m_spg->portc_in().set(FUNC(vsmile_state::portc_r));
	m_spg->portc_out().set(FUNC(vsmile_state::portc_w));
	m_spg->chip_select().set(FUNC(vsmile_state::chip_sel_w));
	m_spg->uart_tx().set(FUNC(vsmile_state::uart_rx));

	m_bankdev->set_addrmap(AS_PROGRAM, &vsmile_state::banked_map);

	SOFTWARE_LIST(config, "cart_list").set_original("vsmile_cart");
	SOFTWARE_LIST(config, "cart_list2").set_original("vsmilem_cart");
}

void vsmile_state::vsmilep(machine_config &config)
{
	vsmile(config);
	m_spg->set_pal(true);
}

void vsmileb_state::vsmileb(machine_config &config)
{
	vsmile_base(config);

	m_spg->porta_in().set(FUNC(vsmileb_state::porta_r));
	m_spg->portb_in().set(FUNC(vsmileb_state::portb_r));
	m_spg->portc_in().set(FUNC(vsmileb_state::portc_r));
	m_spg->porta_out().set(FUNC(vsmileb_state::porta_w));
	m_spg->portb_out().set(FUNC(vsmileb_state::portb_w));
	m_spg->portc_out().set(FUNC(vsmileb_state::portc_w));
	m_spg->chip_select().set(FUNC(vsmileb_state::chip_sel_w));

	m_bankdev->set_addrmap(AS_PROGRAM, &vsmileb_state::banked_map);

	SOFTWARE_LIST(config, "cart_list").set_original("vsmileb_cart");
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

//    year, name,    parent, compat, machine, input,   class,         init,       company, fullname,            flags
CONS( 2005, vsmile,  0,      0,      vsmile,  vsmile,  vsmile_state,  empty_init, "VTech", "V.Smile (US)",      MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2005, vsmileg, vsmile, 0,      vsmilep, vsmile,  vsmile_state,  empty_init, "VTech", "V.Smile (Germany)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2005, vsmilef, vsmile, 0,      vsmilep, vsmile,  vsmile_state,  empty_init, "VTech", "V.Smile (France)",  MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2005, vsmileb, 0,      0,      vsmileb, vsmileb, vsmileb_state, empty_init, "VTech", "V.Smile Baby (US)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
