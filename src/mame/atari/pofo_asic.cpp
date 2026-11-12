// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Atari Portfolio system ASIC

*/

#include "emu.h"
#include "pofo_asic.h"

#define LOG 0

DEFINE_DEVICE_TYPE(PORTFOLIO_ASIC, portfolio_asic_device, "pofo_asic", "Atari Portfolio ASIC");

portfolio_asic_device::portfolio_asic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PORTFOLIO_ASIC, tag, owner, clock),
	m_write_pint(*this),
	m_write_nmio(*this),
	m_read_nmd1(*this, 0),
	m_read_pdet(*this, 0),
	m_write_ncc1(*this),
	m_write_ncc2(*this),
	m_write_dtmf(*this),
	m_write_contrast(*this),
	m_read_lcdc(*this, 0),
	m_write_lcdc(*this),
	m_read_kop(*this, 0),
	m_read_battery(*this, 0),
	m_nmi_timer(*this, "nmi_tick"),
	m_rom_bank_view(*this, "rom_bank_view")
{
}

void portfolio_asic_device::mem_map(address_map &map)
{
	map(0xc0000, 0xdffff).view(m_rom_bank_view);
	m_rom_bank_view[0](0xc0000, 0xdffff).rom().region(m_rom_tag, 0).nopw();
	m_rom_bank_view[1](0xc0000, 0xdffff).noprw(); // EXT ROM
	map(0xe0000, 0xfffff).rom().region(m_rom_tag, 0x20000);

	// internal RAM (0x00000-vram_offset) and the mirrored VRAM window (0xb0000-0xbffff)
	// are installed at runtime from portfolio_base_state::machine_start(), once m_ram's
	// size/pointer are valid
}

void portfolio_asic_device::io_map(address_map &map)
{
	map(0x0061, 0x0061).lr8(NAME([] () { return 0x61; }));
	map(0x8000, 0x8000).mirror(0x0f).r(FUNC(portfolio_asic_device::keyboard_r));
	map(0x8010, 0x8011).mirror(0x0e).lrw8(
			NAME([this] (offs_t offset) { return m_read_lcdc(offset); }),
			NAME([this] (offs_t offset, u8 data) { m_write_lcdc(offset, data); }));
	map(0x8020, 0x8020).mirror(0x0f).lw8(NAME([this] (u8 data) { m_write_dtmf(0, data); }));
	map(0x8030, 0x8030).mirror(0x0f).w(FUNC(portfolio_asic_device::power_w));
	map(0x8040, 0x8041).mirror(0x0e).rw(FUNC(portfolio_asic_device::counter_r), FUNC(portfolio_asic_device::counter_w));
	map(0x8050, 0x8050).mirror(0x0e).rw(FUNC(portfolio_asic_device::irq_status_r), FUNC(portfolio_asic_device::irq_mask_w));
	map(0x8051, 0x8051).mirror(0x0e).rw(FUNC(portfolio_asic_device::battery_r), FUNC(portfolio_asic_device::select_w));
	map(0x8060, 0x8060).mirror(0x0f).rw(FUNC(portfolio_asic_device::contrast_r), FUNC(portfolio_asic_device::contrast_w));
}

uint8_t portfolio_asic_device::mack_r()
{
	uint8_t vector = 0;

	for (int i = 0; i < 3; i++)
	{
		if (BIT(m_ip, i) && BIT(m_ie, i))
		{
			m_ip &= ~(1 << i);

			if (LOG) logerror("%s %s IP %01x\n", machine().time().as_string(), machine().describe_context(), m_ip);

			vector = INTERRUPT_VECTOR[i];
			break;
		}
	}

	check_interrupt();

	return vector;
}

void portfolio_asic_device::device_start()
{
	// state saving
	save_item(NAME(m_sleep));
	save_item(NAME(m_ip));
	save_item(NAME(m_ie));
	save_item(NAME(m_counter));
	save_item(NAME(m_contrast));
	save_item(NAME(m_rom_b));
	save_item(NAME(m_kbd_data));
	save_item(NAME(m_kop_state));
	save_item(NAME(m_kop_row));
}

void portfolio_asic_device::device_reset()
{
	attotime const period = attotime::from_seconds(128);
	m_nmi_timer->adjust(period, 0, period);

	m_kbd_data = 0xff;

	std::fill(std::begin(m_kop_state), std::end(m_kop_state), 0);
	m_kop_row = 0;

	m_rom_bank_view.select(m_rom_b == ROM_EXT ? 1 : 0);
	update_ccm_select();
}

void portfolio_asic_device::device_add_mconfig(machine_config &config)
{
	TIMER(config, "counter").configure_periodic(FUNC(portfolio_asic_device::counter_tick), attotime::from_hz(XTAL(32'768)/16384));
	TIMER(config, "nmi_tick").configure_generic(FUNC(portfolio_asic_device::nmi_tick));
	TIMER(config, "kbd_tick").configure_periodic(FUNC(portfolio_asic_device::keyboard_tick), attotime::from_hz(600));
}

void portfolio_asic_device::check_interrupt()
{
	int level = (m_ip & m_ie) ? ASSERT_LINE : CLEAR_LINE;

	m_write_pint(level);
}

void portfolio_asic_device::trigger_interrupt(int level)
{
	m_ip |= 1 << level;

	check_interrupt();
}

uint8_t portfolio_asic_device::irq_status_r()
{
	uint8_t data = m_ip;
	/*
	    The BIOS interrupt 11h (Equipment list) reports that the second floppy drive (B:) is
	    installed if the 3rd bit is set (which is also the external interrupt line).
	    It is not clear if the ~NMD1 line is OR or XORed or muxed with the interrupt line,
	    but this way seems to work.
	*/
	data |= !m_read_nmd1() << 3;

	return data;
}

void portfolio_asic_device::irq_mask_w(uint8_t data)
{
	m_ie = data;

	if (LOG) logerror("%s %s IE %01x\n", machine().time().as_string(), machine().describe_context(), data);

	check_interrupt();
}

void portfolio_asic_device::key_make(u8 row, u8 column)
{
	m_kbd_data = row * 8 + column;

	if (m_sleep)
	{
		m_sleep = false;

		m_write_nmio(ASSERT_LINE);
		m_write_nmio(CLEAR_LINE);
	}

	trigger_interrupt(INT_KEYBOARD);
}

void portfolio_asic_device::key_break(u8 row, u8 column)
{
	m_kbd_data = 0x80 | (row * 8 + column);

	trigger_interrupt(INT_KEYBOARD);
}

TIMER_DEVICE_CALLBACK_MEMBER(portfolio_asic_device::keyboard_tick)
{
	u8 &state = m_kop_state[m_kop_row];
	u8 const keys = m_read_kop[m_kop_row]();
	u8 const change = state ^ keys;

	for (u8 column = 0, mask = 1; state != keys; column++, mask <<= 1)
	{
		if (change & mask)
		{
			state ^= mask;
			if (keys & mask)
				key_make(m_kop_row, column);
			else
				key_break(m_kop_row, column);
		}
	}

	m_kop_row = (m_kop_row + 1) % 8;
}

void portfolio_asic_device::update_ccm_select()
{
	if (LOG) logerror("%s %s update_ccm_select rom_b=%x ccm_a=%d ccm_b=%d\n", machine().time().as_string(), machine().describe_context(), m_rom_b, m_rom_b == CCM_A, m_rom_b == CCM_B);

	m_write_ncc2(m_rom_b == CCM_A);
	m_write_ncc1(m_rom_b == CCM_B);
}

void portfolio_asic_device::select_w(uint8_t data)
{
	/*

	    bit     description

	    0       ?
	    1       ?
	    2       ?
	    3       ?
	    4
	    5
	    6       ?
	    7       ?

	*/

	if (LOG) logerror("%s %s SELECT %02x\n", machine().time().as_string(), machine().describe_context(), data);

	m_rom_b = data & 0x0f;

	m_rom_bank_view.select(m_rom_b == ROM_EXT ? 1 : 0);
	update_ccm_select();
}

void portfolio_asic_device::power_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1       1=stop CCLK (OFF command / auto standby)
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	if (LOG) logerror("%s %s POWER %02x\n", machine().time().as_string(), machine().describe_context(), data);

	if (BIT(data, 1))
	{
		m_sleep = true;
	}
}

uint8_t portfolio_asic_device::battery_r()
{
	/*

	    bit     signal      description

	    0       ?           bit 0 from bus select (m_rom_b)
	    1       ?
	    2       ?           bit 2 from bus select (m_rom_b)
	    3       ?
	    4       ?
	    5       PDET        1=peripheral connected
	    6       LOWB        0=battery low
	    7       BDET?       1=cold boot

	*/

	uint8_t data = 0;

	/*
	    Partially stores what has been written into this port.
	    Used by interrupt 61h service 24h (Get ROM/CCM state).
	    Setting bit 1 here causes the BIOS to permanently wedge the external ROM
	    select on, so mask it out as a workaround.
	*/
	data |= (m_rom_b & 0b101);

	// peripheral detect
	data |= m_read_pdet() << 5;

	// battery status
	data |= (m_read_battery() & 0x03) << 6;

	return data;
}

uint8_t portfolio_asic_device::counter_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_counter & 0xff;
		break;

	case 1:
		data = m_counter >> 8;
		break;
	}

	return data;
}

void portfolio_asic_device::counter_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		{
			m_counter = (m_counter & 0xff00) | data;

			attotime const period = BIT(data, 0) ? attotime::from_hz(1) : attotime::from_seconds(128);
			m_nmi_timer->adjust(period, 0, period);
		}
		break;

	case 1:
		m_counter = (data << 8) | (m_counter & 0xff);
		break;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(portfolio_asic_device::counter_tick)
{
	m_counter++;
}

TIMER_DEVICE_CALLBACK_MEMBER(portfolio_asic_device::nmi_tick)
{
	m_write_nmio(ASSERT_LINE);
	m_write_nmio(CLEAR_LINE);

	trigger_interrupt(INT_TICK);
}
