// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood
/******************************************************************************

    Leapfrog Clickstart Emulation

         Die markings show "SunPlus QL8041C" (known as Sunplus SPG2??).
         The keyboard has a "SunPlus PU6583" MCU under a glob, and the
         mouse optical sensor is a N2163, probably from Agilent.

    ClickStart cartridges pinout:

         1 N/C          2 GND
         3 GND          4 VCC
         5 GND          6 VCC
         7 N/C          8 N/C
         9 N/C         10 N/C
        11 N/C         12 to 53
        13 A2          14 A1
        15 A4          16 A3
        17 A6          18 A5
        19 A17         20 A7
                 Key
        21 N/C         22 A18
        23 A19         24 A20
        25 A9          26 A8
        27 A11         28 A10
        29 A13         30 A12
        31 A15         32 A14
        33 D15         34 A16
        35 D14         36 D7
        37 D13         38 D6
        39 D12         40 D5
        41 D11         42 D4
        43 D10         44 D3
        45 D9          46 D2
        47 D8          48 D1
        49 N/C         50 D0
        51 /OE         52 A0
        53 to 12       54 N/C
        55 GND         56 N/C
        57 GND         58 N/C
        59 GND         60 GND

    Status:

         Some games have Checksums listed in the header area that appear to be
         like the byte checksums on the Radica games in vii.cpp, however the
         calculation doesn't add up correctly.  There is also a checksum in
         a footer area at the end of every ROM that does add up correctly in
         all cases.

         The ROM carts are marked for 4MByte ROMs at least so the sizes
         should be correct.

        What type of SPG is this?

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "cpu/unsp/unsp.h"

#include "machine/spg2xx.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class clickstart_state : public driver_device
{
public:
	clickstart_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_cart(*this, "cartslot")
		, m_system_region(*this, "maincpu")
		, m_io_mouse_x(*this, "MOUSEX")
		, m_io_mouse_y(*this, "MOUSEY")
		, m_keys(*this, "KEYS%u", 0U)
		, m_cart_region(nullptr)
		, m_mouse_x(0)
		, m_mouse_y(0)
		, m_mouse_button(0)
		, m_mouse_dx(0)
		, m_mouse_dy(0)
		, m_uart_tx_fifo_start(0)
		, m_uart_tx_fifo_end(0)
		, m_uart_tx_fifo_count(0)
		, m_uart_tx_timer(nullptr)
		, m_unk_portc_toggle(0)
	{ }

	void clickstart(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(mouse_update);
	DECLARE_INPUT_CHANGED_MEMBER(key_update);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	uint16_t rom_r(offs_t offset);

	void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t porta_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t portb_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t portc_r(offs_t offset, uint16_t mem_mask = ~0);

	void chip_sel_w(uint8_t data);

	TIMER_CALLBACK_MEMBER(handle_uart_tx);
	void uart_tx_fifo_push(uint8_t value);

	void update_mouse_buffer();

	required_device<spg2xx_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<generic_slot_device> m_cart;
	required_memory_region m_system_region;
	required_ioport m_io_mouse_x;
	required_ioport m_io_mouse_y;
	required_ioport_array<3> m_keys;
	memory_region *m_cart_region;

	uint16_t m_mouse_x;
	uint16_t m_mouse_y;
	uint8_t m_mouse_button;
	int16_t m_mouse_dx;
	int16_t m_mouse_dy;

	uint8_t m_uart_tx_fifo[1024]; // arbitrary size
	uint8_t m_uart_tx_fifo_start;
	uint8_t m_uart_tx_fifo_end;
	uint8_t m_uart_tx_fifo_count;
	emu_timer *m_uart_tx_timer;

	uint16_t m_unk_portc_toggle;
};

void clickstart_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	}

	save_item(NAME(m_mouse_x));
	save_item(NAME(m_mouse_y));
	save_item(NAME(m_mouse_button));
	save_item(NAME(m_mouse_dx));
	save_item(NAME(m_mouse_dy));

	save_item(NAME(m_uart_tx_fifo));
	save_item(NAME(m_uart_tx_fifo_start));
	save_item(NAME(m_uart_tx_fifo_end));
	save_item(NAME(m_uart_tx_fifo_count));

	save_item(NAME(m_unk_portc_toggle));

	m_uart_tx_timer = timer_alloc(FUNC(clickstart_state::handle_uart_tx), this);
	m_uart_tx_timer->adjust(attotime::never);
}

void clickstart_state::machine_reset()
{
	m_mouse_x = m_io_mouse_x->read();
	m_mouse_y = m_io_mouse_y->read();
	m_mouse_button = 0;
	m_mouse_dx = 0;
	m_mouse_dy = 0;

	std::fill(std::begin(m_uart_tx_fifo), std::end(m_uart_tx_fifo), 0);
	m_uart_tx_fifo_start = 0;
	m_uart_tx_fifo_end = 0;
	m_uart_tx_fifo_count = 0;
	m_uart_tx_timer->adjust(attotime::from_hz(3200/10), 0, attotime::from_hz(3200/10));

	m_unk_portc_toggle = 0;
}

DEVICE_IMAGE_LOAD_MEMBER(clickstart_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

TIMER_CALLBACK_MEMBER(clickstart_state::handle_uart_tx)
{
	if (m_uart_tx_fifo_count == 0)
		return;

	m_maincpu->uart_rx(m_uart_tx_fifo[m_uart_tx_fifo_start]);
	m_uart_tx_fifo_start = (m_uart_tx_fifo_start + 1) % std::size(m_uart_tx_fifo);
	m_uart_tx_fifo_count--;
}

void clickstart_state::uart_tx_fifo_push(uint8_t value)
{
	if (m_uart_tx_fifo_count >= std::size(m_uart_tx_fifo))
	{
		logerror("Warning: Trying to push too much data onto the mouse Tx FIFO, data will be lost.\n");
	}

	m_uart_tx_fifo[m_uart_tx_fifo_end] = value;
	m_uart_tx_fifo_end = (m_uart_tx_fifo_end + 1) % std::size(m_uart_tx_fifo);
	m_uart_tx_fifo_count++;
}

INPUT_CHANGED_MEMBER(clickstart_state::key_update)
{
	const size_t keycode = static_cast<size_t>(param);

	printf("keycode:%02x, oldval:%02x, newval:%02x\n", (uint8_t)keycode, oldval, newval);

	uint8_t buffer[6] = {};
	buffer[0] = 0x00;
	buffer[1] = 0x01;
	buffer[2] = newval ? keycode : 0x3f;
	buffer[3] = 0x2f;
	buffer[4] = 0x01;
	buffer[5] = 0x01;

	printf("Keyboard queueing: ");
	uint16_t sum = 0;
	for (int i = 0; i < 6; i++)
	{
		uart_tx_fifo_push(buffer[i] ^ 0xff);
		sum += buffer[i];
		printf("%02x/%02x ", buffer[i], buffer[i] ^ 0xff);
	}
	sum = (sum & 0xff) ^ 0xff;
	uart_tx_fifo_push((uint8_t)sum);
	printf("%02x\n", (uint8_t)sum);
}

INPUT_CHANGED_MEMBER(clickstart_state::mouse_update)
{
	m_mouse_button = newval ? (uint8_t)static_cast<size_t>(param) : 0;

	const uint16_t x_val(m_io_mouse_x->read());
	const uint16_t y_val(m_io_mouse_y->read());
	int16_t x_delta(x_val - m_mouse_x);
	int16_t y_delta(y_val - m_mouse_y);

	// deal with wraparound
	if (0x0200 <= x_delta)
		x_delta -= 0x400;
	else if (-0x0200 >= x_delta)
		x_delta += 0x400;
	if (0x100 <= y_delta)
		y_delta -= 0x200;
	else if (-0x100 >= y_delta)
		x_delta += 0x200;

	m_mouse_x = x_val;
	m_mouse_y = y_val;

	m_mouse_dx = x_delta;
	m_mouse_dy = -y_delta;

	if (m_mouse_dx < -255)
		m_mouse_dx = -255;
	else if (m_mouse_dx > 255)
		m_mouse_dx = 255;

	if (m_mouse_dy < -255)
		m_mouse_dy = -255;
	else if (m_mouse_dy > 255)
		m_mouse_dy = 255;

	update_mouse_buffer();

	m_mouse_dx = 0;
	m_mouse_dy = 0;
}

void clickstart_state::update_mouse_buffer()
{
	uint8_t buffer[6] = {};
	buffer[0] = 0x00;
	buffer[1] = 0x01 | m_mouse_button;
	buffer[2] = 0x3e | ((m_mouse_dx >> 1) & 0x80);
	buffer[3] = 0x3f | ((m_mouse_dy >> 1) & 0x80);
	buffer[4] = (m_mouse_dx & 0xfe) + 1;
	buffer[5] = (m_mouse_dy & 0xfe) + 1;

	uint16_t sum = 0;
	for (int i = 0; i < 6; i++)
	{
		uart_tx_fifo_push(buffer[i] ^ 0xff);
		sum += buffer[i];
	}
	sum = (sum & 0xff) ^ 0xff;
	uart_tx_fifo_push((uint8_t)sum);
}

uint16_t clickstart_state::rom_r(offs_t offset)
{
	if (offset < 0x400000 / 2)
	{
		if (m_cart->exists())
			return ((uint16_t*)m_cart_region->base())[offset];
		else
			return ((uint16_t*)m_system_region->base())[offset];
	}
	else
	{
		return ((uint16_t*)m_system_region->base())[offset];
	}
}

void clickstart_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//logerror("%s: porta_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
}

void clickstart_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//logerror("%s: portb_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
}

void clickstart_state::portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// Bit 12: SCK from SPG SIO
	// Bit 11: SDA from SPG SIO
	//logerror("%s: portc_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
}

uint16_t clickstart_state::porta_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0x4000;
	logerror("%s: porta_r: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	return data;
}

uint16_t clickstart_state::portb_r(offs_t offset, uint16_t mem_mask)
{
	//logerror("%s: portb_r: %04x\n", machine().describe_context(), mem_mask);
	return 0;
}

uint16_t clickstart_state::portc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_unk_portc_toggle;
	m_unk_portc_toggle ^= 0x0400;
	//logerror("%s: portc_r: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void clickstart_state::chip_sel_w(uint8_t data)
{
	// Seems unused, currently
}

void clickstart_state::mem_map(address_map &map)
{
	map(0x000000, 0x3fffff).r(FUNC(clickstart_state::rom_r));
}

static INPUT_PORTS_START( clickstart )
	PORT_START("MOUSEX")
	PORT_BIT(0x3ff, 0x00, IPT_MOUSE_X) PORT_MINMAX(0x0000,0x03ff) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::mouse_update), 0)

	PORT_START("MOUSEY")
	PORT_BIT(0x1ff, 0x00, IPT_MOUSE_Y) PORT_MINMAX(0x0000,0x01ff) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::mouse_update), 0)

	PORT_START("MOUSEBTN")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::mouse_update), 0x10)
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEYS0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x01) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x02) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x03) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x04) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x05) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x06) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x07) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x08) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x09) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x0a) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x0b) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x0c) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x0d) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x0e) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x0f) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x10) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')

	PORT_START("KEYS1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x11) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('q')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x12) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('r')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x13) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('s')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x14) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('t')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x15) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('u')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x16) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('v')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x17) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('w')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x18) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('x')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x19) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('y')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x1a) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('z')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x25) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x26) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x28) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x29) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x2a) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x2b) PORT_CODE(KEYCODE_5) PORT_CHAR('5')

	PORT_START("KEYS2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x2d) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x2e) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x2f) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x24) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x27) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x2c) PORT_CODE(KEYCODE_LALT) PORT_NAME("Music")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0xa9) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("Shift") PORT_TOGGLE
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(clickstart_state::key_update), 0x2f) PORT_CODE(KEYCODE_TILDE) PORT_NAME("Pause") PORT_TOGGLE
	PORT_BIT(0xff00, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

// There is a 24C08AN SEEPROM on the motherboard

void clickstart_state::clickstart(machine_config &config)
{
	SPG28X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &clickstart_state::mem_map);
	m_maincpu->porta_out().set(FUNC(clickstart_state::porta_w));
	m_maincpu->portb_out().set(FUNC(clickstart_state::portb_w));
	m_maincpu->portc_out().set(FUNC(clickstart_state::portc_w));
	m_maincpu->porta_in().set(FUNC(clickstart_state::porta_r));
	m_maincpu->portb_in().set(FUNC(clickstart_state::portb_r));
	m_maincpu->portc_in().set(FUNC(clickstart_state::portc_r));
	m_maincpu->adc_in<0>().set_constant(0x0fff);
	m_maincpu->chip_select().set(FUNC(clickstart_state::chip_sel_w));
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("maincpu", FUNC(spg2xx_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(spg2xx_device::vblank));

	SPEAKER(config, "speaker", 2).front();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "clickstart_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(clickstart_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("clickstart_cart");
}

ROM_START( clikstrt )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "clickstartbios.bin", 0x000000, 0x800000, CRC(7c833bd0) SHA1(2e9ef38e1a7582705920339e6b9944f6404fcf9b) )
ROM_END

} // anonymous namespace


// year, name, parent, compat, machine, input, class, init, company, fullname, flags
CONS( 2007, clikstrt,  0,      0, clickstart,  clickstart, clickstart_state, empty_init, "LeapFrog Enterprises", "ClickStart",      MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // 'My First Computer' tagline
