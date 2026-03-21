// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek

#include "emu.h"

#include "pcecommn.h"

#include "cpu/h6280/h6280.h"

#include "screen.h"

// other values are unused at arcade bootlegs?
//static constexpr u8 TG_16_JOY_SIG = 0x00;
static constexpr u8 PCE_JOY_SIG   = 0x40;
//static constexpr u8 NO_CD_SIG     = 0x80;
//static constexpr u8 CD_SIG        = 0x00;
/* these might be used to indicate something, but they always seem to return 1 */
static constexpr u8 CONST_SIG   = 0x30;

/* joystick related data*/
static constexpr u8 JOY_CLOCK = 0x01;
static constexpr u8 JOY_RESET = 0x02;

void pce_common_state::machine_start()
{
	save_item(NAME(m_joystick_port_select));
	save_item(NAME(m_joystick_data_select));
}

/* todo: how many input ports does the PCE have? */
void pce_common_state::pce_joystick_w(u8 data)
{
	/* bump counter on a low-to-high transition of bit 1 */
	if ((!m_joystick_data_select) && (data & JOY_CLOCK))
	{
		m_joystick_port_select = (m_joystick_port_select + 1) & 0x07;
	}

	/* do we want buttons or direction? */
	m_joystick_data_select = data & JOY_CLOCK;

	/* clear counter if bit 2 is set */
	if (data & JOY_RESET)
	{
		m_joystick_port_select = 0;
	}
}

u8 pce_common_state::joy_read()
{
	return m_io_joy->read();
}

u8 pce_common_state::pce_joystick_r()
{
	u8 data = joy_read();
	if (m_joystick_data_select) data >>= 4;
	u8 ret = (data & 0x0f) | m_io_port_options;
#ifdef UNIFIED_PCE
	ret &= ~0x40;
#endif
	return ret;
}

void pce_common_state::init_pce_common()
{
	m_io_port_options = PCE_JOY_SIG | CONST_SIG;
}

void pce_common_state::common_mem_map(address_map &map)
{
	map(0x1f0000, 0x1f1fff).ram().mirror(0x6000);
	map(0x1fe000, 0x1fe3ff).rw("huc6270", FUNC(huc6270_device::read8), FUNC(huc6270_device::write8));
	map(0x1fe400, 0x1fe7ff).rw(m_huc6260, FUNC(huc6260_device::read), FUNC(huc6260_device::write));
}

void pce_common_state::common_io_map(address_map &map)
{
	map(0x00, 0x03).rw("huc6270", FUNC(huc6270_device::read8), FUNC(huc6270_device::write8));
}

void pce_common_state::common_cpu(machine_config &config)
{
	H6280(config, m_maincpu, PCE_MAIN_CLOCK/3);
	m_maincpu->set_addrmap(AS_IO, &pce_common_state::common_io_map);
	m_maincpu->port_in_cb().set(FUNC(pce_common_state::pce_joystick_r));
	m_maincpu->port_out_cb().set(FUNC(pce_common_state::pce_joystick_w));
}

void pce_common_state::common_video(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PCE_MAIN_CLOCK, huc6260_device::WPF, 64, 64 + 1024 + 64, huc6260_device::LPF, 18, 18 + 242);
	screen.set_screen_update(m_huc6260, FUNC(huc6260_device::screen_update));
	screen.set_palette(m_huc6260);

	HUC6260(config, m_huc6260, PCE_MAIN_CLOCK);
	m_huc6260->next_pixel_data().set("huc6270", FUNC(huc6270_device::next_pixel));
	m_huc6260->time_til_next_event().set("huc6270", FUNC(huc6270_device::time_until_next_event));
	m_huc6260->vsync_changed().set("huc6270", FUNC(huc6270_device::vsync_changed));
	m_huc6260->hsync_changed().set("huc6270", FUNC(huc6270_device::hsync_changed));

	huc6270_device &huc6270(HUC6270(config, "huc6270", PCE_MAIN_CLOCK));
	huc6270.set_vram_size(0x10000);
	huc6270.irq().set_inputline(m_maincpu, 0);
}
