// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood
/******************************************************************************

    Leapfrog Clickstart Emulation

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
#include "softlist.h"
#include "speaker.h"

class clickstart_state : public driver_device
{
public:
	clickstart_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_spg(*this, "spg")
		, m_cart(*this, "cartslot")
		, m_system_region(*this, "maincpu")
		, m_io_mouse_x(*this, "MOUSEX")
		, m_io_mouse_y(*this, "MOUSEY")
		, m_cart_region(nullptr)
		, m_mouse_x(0)
		, m_mouse_y(0)
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

private:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static const device_timer_id TIMER_UART_TX = 0;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void mem_map(address_map &map);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart);

	DECLARE_READ16_MEMBER(rom_r);

	DECLARE_WRITE16_MEMBER(porta_w);
	DECLARE_WRITE16_MEMBER(portb_w);
	DECLARE_WRITE16_MEMBER(portc_w);
	DECLARE_READ16_MEMBER(porta_r);
	DECLARE_READ16_MEMBER(portb_r);
	DECLARE_READ16_MEMBER(portc_r);

	DECLARE_WRITE8_MEMBER(chip_sel_w);

	void handle_uart_tx();
	void uart_tx_fifo_push(uint8_t value);

	void update_mouse_buffer();

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<spg2xx_device> m_spg;
	required_device<generic_slot_device> m_cart;
	required_memory_region m_system_region;
	required_ioport m_io_mouse_x;
	required_ioport m_io_mouse_y;
	memory_region *m_cart_region;

	uint16_t m_mouse_x;
	uint16_t m_mouse_y;
	int16_t m_mouse_dx;
	int16_t m_mouse_dy;
	uint8_t m_mouse_buffer[16];

	uint8_t m_uart_tx_fifo[32]; // arbitrary size
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
	save_item(NAME(m_mouse_dx));
	save_item(NAME(m_mouse_dy));
	save_item(NAME(m_mouse_buffer));

	save_item(NAME(m_uart_tx_fifo));
	save_item(NAME(m_uart_tx_fifo_start));
	save_item(NAME(m_uart_tx_fifo_end));
	save_item(NAME(m_uart_tx_fifo_count));

	save_item(NAME(m_unk_portc_toggle));

	m_uart_tx_timer = timer_alloc(TIMER_UART_TX);
	m_uart_tx_timer->adjust(attotime::never);
}

void clickstart_state::machine_reset()
{
	m_mouse_x = 0xffff;
	m_mouse_y = 0xffff;
	m_mouse_dx = 0;
	m_mouse_dy = 0;
	memset(m_mouse_buffer, 0, ARRAY_LENGTH(m_mouse_buffer));

	memset(m_uart_tx_fifo, 0, ARRAY_LENGTH(m_uart_tx_fifo));
	m_uart_tx_fifo_start = 0;
	m_uart_tx_fifo_end = 0;
	m_uart_tx_fifo_count = 0;
	m_uart_tx_timer->adjust(attotime::from_hz(3200/10), 0, attotime::from_hz(3200/10));

	m_unk_portc_toggle = 0;
}

DEVICE_IMAGE_LOAD_MEMBER(clickstart_state, cart)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

void clickstart_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_UART_TX)
	{
		handle_uart_tx();
	}
}

void clickstart_state::handle_uart_tx()
{
	if (m_uart_tx_fifo_count == 0)
		return;

	m_spg->uart_rx(m_uart_tx_fifo[m_uart_tx_fifo_start]);
	m_uart_tx_fifo_start = (m_uart_tx_fifo_start + 1) % ARRAY_LENGTH(m_uart_tx_fifo);
	m_uart_tx_fifo_count--;
}

void clickstart_state::uart_tx_fifo_push(uint8_t value)
{
	if (m_uart_tx_fifo_count >= ARRAY_LENGTH(m_uart_tx_fifo))
	{
		logerror("Warning: Trying to push too much data onto the mouse Tx FIFO, data will be lost.\n");
	}

	m_uart_tx_fifo[m_uart_tx_fifo_end] = value;
	m_uart_tx_fifo_end = (m_uart_tx_fifo_end + 1) % ARRAY_LENGTH(m_uart_tx_fifo);
	m_uart_tx_fifo_count++;
}

INPUT_CHANGED_MEMBER(clickstart_state::mouse_update)
{
	uint16_t x = m_io_mouse_x->read();
	uint16_t y = m_io_mouse_y->read();
	uint16_t old_mouse_x = m_mouse_x;
	uint16_t old_mouse_y = m_mouse_y;

	if (m_mouse_x == 0xffff)
	{
		old_mouse_x = x;
		old_mouse_y = y;
	}

	m_mouse_x = x;
	m_mouse_y = y;

	m_mouse_dx += (m_mouse_x - old_mouse_x);
	m_mouse_dy += (m_mouse_y - old_mouse_y);

	if (m_mouse_dx < -63)
		m_mouse_dx = -63;
	else if (m_mouse_dx > 62)
		m_mouse_dx = 62;

	if (m_mouse_dy < -63)
		m_mouse_dy = -63;
	else if (m_mouse_dy > 62)
		m_mouse_dy = 62;

	update_mouse_buffer();

	m_mouse_dx = 0;
	m_mouse_dy = 0;
}

void clickstart_state::update_mouse_buffer()
{
	if (m_mouse_dx == 0 && m_mouse_dy == 0)
		return;

	m_mouse_buffer[0] = 0x03;
	m_mouse_buffer[1] = (m_mouse_x + 1) & 0x3f;
	m_mouse_buffer[2] = (m_mouse_y + 1) & 0x3f;
	m_mouse_buffer[3] = (m_mouse_dx + 1) & 0x3f;
	m_mouse_buffer[4] = (m_mouse_dy + 1) & 0x3f;

	//printf("Queueing: ");
	uint16_t sum = 0;
	for (int i = 0; i < 5; i++)
	{
		uart_tx_fifo_push(m_mouse_buffer[i] ^ 0xff);
		sum += m_mouse_buffer[i];
		//printf("%02x ", m_mouse_buffer[i] ^ 0xff);
	}
	sum = (sum & 0xff) ^ 0xff;
	uart_tx_fifo_push((uint8_t)sum);
	//printf("%02x\n", (uint8_t)sum);
}

READ16_MEMBER(clickstart_state::rom_r)
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

WRITE16_MEMBER(clickstart_state::porta_w)
{
	//logerror("%s: porta_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
}

WRITE16_MEMBER(clickstart_state::portb_w)
{
	logerror("%s: portb_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
}

WRITE16_MEMBER(clickstart_state::portc_w)
{
	//logerror("%s: portc_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
}

READ16_MEMBER(clickstart_state::porta_r)
{
	uint16_t data = 0x4000;
	//logerror("%s: porta_r: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	return data;
}

READ16_MEMBER(clickstart_state::portb_r)
{
	logerror("%s: portb_r: %04x\n", machine().describe_context(), mem_mask);
	return 0;
}

READ16_MEMBER(clickstart_state::portc_r)
{
	uint16_t data = m_unk_portc_toggle;
	m_unk_portc_toggle ^= 0x0400;
	//logerror("%s: portc_r: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	return data;
}

WRITE8_MEMBER(clickstart_state::chip_sel_w)
{
	// Seems unused, currently
}

void clickstart_state::mem_map(address_map &map)
{
	map(0x000000, 0x3fffff).r(FUNC(clickstart_state::rom_r));
	map(0x000000, 0x003fff).m(m_spg, FUNC(spg2xx_device::map));
}

static INPUT_PORTS_START( clickstart )
	PORT_START("MOUSEX")
	PORT_BIT(0x3e, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CHANGED_MEMBER(DEVICE_SELF, clickstart_state, mouse_update, 0)

	PORT_START("MOUSEY")
	PORT_BIT(0x3e, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CHANGED_MEMBER(DEVICE_SELF, clickstart_state, mouse_update, 0)
INPUT_PORTS_END

// There is a SEEPROM on the motherboard (type?)

void clickstart_state::clickstart(machine_config &config)
{
	UNSP(config, m_maincpu, XTAL(27'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &clickstart_state::mem_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("spg", FUNC(spg2xx_device::screen_update));
	m_screen->screen_vblank().set(m_spg, FUNC(spg2xx_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SPG28X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);
	m_spg->porta_out().set(FUNC(clickstart_state::porta_w));
	m_spg->portb_out().set(FUNC(clickstart_state::portb_w));
	m_spg->portc_out().set(FUNC(clickstart_state::portc_w));
	m_spg->porta_in().set(FUNC(clickstart_state::porta_r));
	m_spg->portb_in().set(FUNC(clickstart_state::portb_r));
	m_spg->portc_in().set(FUNC(clickstart_state::portc_r));
	m_spg->adc_in().set_constant(0x0fff);
	m_spg->chip_select().set(FUNC(clickstart_state::chip_sel_w));
	m_spg->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_spg->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "clickstart_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(device_image_load_delegate(&clickstart_state::device_image_load_cart, this));

	SOFTWARE_LIST(config, "cart_list").set_original("clickstart_cart");
}

ROM_START( clikstrt )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "clickstartbios.bin", 0x000000, 0x800000, CRC(7c833bd0) SHA1(2e9ef38e1a7582705920339e6b9944f6404fcf9b) )
ROM_END

// year, name, parent, compat, machine, input, class, init, company, fullname, flags
CONS( 2007, clikstrt,  0,      0, clickstart,  clickstart, clickstart_state, empty_init, "LeapFrog Enterprises", "ClickStart",      MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // 'My First Computer' tagline
