// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    Driver for Casio Picky Talk and Casio Plet's

    TODO:

    - Communication port;
    - Panel active buttons display;
    - Review PORT/OPT callbacks copied from CFX9850G;
    - Fix unk_F8 loop when viewing calendar in JD-363/JD-364 models;
    - Fix busy loop @ 20:2981 in Plet's models;
    - Keyboard input for Plet's models;

    Hardware
    --------

    Super Picky Talk - Forest of Gurutan (JD-370):

    - PCB revision: A140947-1 Z835-1
    - LSI1 (CPU): Unknown (instruction set compatible with Hitachi HCD62121)
    - LSI3 (Static RAM): NEC D441000LGZ (1M-bit, 128K-word by 8-bit)
    - LSI5 (Mask ROM): NEC D23C8000XGX-C64 (8M-bit, 1M-word by 8-bit, pin compatible with AMD AM29F800B)

    Plet's (MK-300):

    - PCB revision: A141252-1 Z836-1
    - LSI1 (CPU): Unknown (instruction set compatible with Hitachi HCD62121)
    - LSI3 (Static RAM): Toshiba TC551001BFL-10V (1M-bit, 128K-word by 8-bit)
    - LSI5 (Mask ROM): NEC D23C8000XGX-C77 (8M-bit, 1M-word by 8-bit, pin compatible with AMD AM29F800B)

    Plet's (MK-350):

    - PCB revision: A141252-1 Z836-1
    - LSI1 (CPU): Unknown (instruction set compatible with Hitachi HCD62121)
    - LSI3 (Static RAM): NEC D441000LGW-B85X
    - LSI5 (Mask ROM): Oki M538032E-48

    Hidden Features
    ---------------

    [JD-370] On the debugger, run "bpset 20adb2,,{ ip=20ad27; g; }" to access an unused test program.

***************************************************************************/

#include "emu.h"

#include "cpu/hcd62121/hcd62121.h"

#include "crsshair.h"
#include "emupal.h"
#include "screen.h"

#include "pickytlk.lh"

#define LOG_IO     (1U << 1)
#define LOG_TABLET (1U << 2)

//#define VERBOSE (LOG_IO | LOG_TABLET)
#include "logmacro.h"

namespace {

class pickytlk_base_state : public driver_device
{
public:
	DECLARE_CROSSHAIR_MAPPER_MEMBER(pen_y_mapper);
	ioport_value pen_y_rescale_r();
	ioport_value pen_target_r();

protected:
	enum pen_target : u8
	{
		PEN_TARGET_LCD = 0,
		PEN_TARGET_PANEL = 1,
	};

	enum pen_state : u8
	{
		PEN_RELEASE = 0,
		PEN_PRESS = 1,
		PEN_HOLD = 2,
	};

	pickytlk_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_display_ram(*this, "display_ram")
		, m_maincpu(*this, "maincpu")
		, m_io_buttons(*this, "BUTTONS")
		, m_io_pen_x(*this, "PEN_X")
		, m_io_pen_y(*this, "PEN_Y")
		, m_io_pen_y_rescale(*this, "PEN_Y_RESCALE")
		, m_ko(0)
		, m_port(0)
		, m_opt(0)
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void pickytlk(machine_config &config) ATTR_COLD;

	void kol_w(u8 data);
	void koh_w(u8 data);
	void port_w(u8 data);
	void opt_w(u8 data);
	u8 ki_r();
	u8 in0_r();
	u8 input_flag_read();

	TIMER_CALLBACK_MEMBER(io_timer_tick);
	u8 io_pen_x_read();
	u8 io_pen_y_read();
	virtual u8 tablet_read(offs_t offset) = 0;
	void tablet_write(offs_t offset, u8 data);

	void update_crosshair(screen_device &screen);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pickytlk_mem(address_map &map) ATTR_COLD;

	static constexpr float rescale(float x, float min_x, float max_x, float a, float b)
	{
		// Rescaling (min-max normalization) from [min_x..max_x] to [a..b].
		return a + (((x - min_x) * (b - a)) / (max_x - min_x));
	}

	required_shared_ptr<u8> m_display_ram;
	required_device<hcd62121_cpu_device> m_maincpu;

	required_ioport m_io_buttons;
	required_ioport m_io_pen_x;
	required_ioport m_io_pen_y;
	required_ioport m_io_pen_y_rescale;

	u16 m_ko;   // KO lines
	u8 m_port;  // PORT lines (serial I/O)
	u8 m_opt;   // OPT lines (contrast)
	std::unique_ptr<u8[]> m_io_tablet_regs;
	emu_timer *m_io_timer;
	u8 m_pen_state;
	u8 m_pen_target;
};

void pickytlk_base_state::machine_start()
{
	save_item(NAME(m_ko));
	save_item(NAME(m_port));
	save_item(NAME(m_opt));
	m_io_tablet_regs = make_unique_clear<u8[]>(0x100);
	save_pointer(NAME(m_io_tablet_regs), 0x100);
	m_io_timer = timer_alloc(FUNC(pickytlk_base_state::io_timer_tick), this);
	save_item(NAME(m_pen_state));
	save_item(NAME(m_pen_target));
}

void pickytlk_base_state::machine_reset()
{
	memset(m_io_tablet_regs.get(), 0, 0x100);
	m_io_timer->reset(attotime::never);
	m_pen_state = PEN_RELEASE;
	m_pen_target = PEN_TARGET_LCD;
}

CROSSHAIR_MAPPER_MEMBER(pickytlk_base_state::pen_y_mapper)
{
	// Parameter `linear_value` is ignored, since we will read the input port directly
	// for adjustments, just need to return that value in the expected range [0.0f..1.0f].
	return (float) pen_y_rescale_r() / 0xff;
}

ioport_value pickytlk_base_state::pen_y_rescale_r()
{
	/*
	    There are two distinct areas that can be interacted with the pen:
	    - LCD screen visible area: pen coordinates in [0x20..0xa0];
	    - Bottom panel: pen coordinates in [0xa0..0xf0];
	    In order to transparently map coordinates between each area, we split
	    the value across these areas, but rescaled to the input port's full range.
	*/
	const s16 io_pen_y_min = m_io_pen_y->field(0xff)->minval();
	const s16 io_pen_y_max = m_io_pen_y->field(0xff)->maxval();
	const s16 screen_y_max = io_pen_y_max * 0.6f;
	s16 adjusted_value = m_io_pen_y->read();
	if (adjusted_value > screen_y_max)
	{
		adjusted_value = rescale(adjusted_value, screen_y_max, io_pen_y_max, io_pen_y_min, io_pen_y_max);
		m_pen_target = PEN_TARGET_PANEL;
	}
	else
	{
		adjusted_value = rescale(adjusted_value, io_pen_y_min, screen_y_max, io_pen_y_min, io_pen_y_max);
		m_pen_target = PEN_TARGET_LCD;
	}

	return adjusted_value;
}

ioport_value pickytlk_base_state::pen_target_r()
{
	return m_pen_target;
}

TIMER_CALLBACK_MEMBER(pickytlk_base_state::io_timer_tick)
{
	if (m_pen_state == PEN_PRESS)
	{
		m_pen_state = PEN_HOLD;
	}
}

u8 pickytlk_base_state::io_pen_x_read()
{
	// Pen callibration tests seem to check coordinates relative to the center of the LCD screen,
	// and those offsets also align with the LCD position relative to the full tablet surface.
	s16 io_pen_x_min = m_io_pen_x->field(0xff)->minval();
	s16 io_pen_x_max = m_io_pen_x->field(0xff)->maxval();
	s16 io_pen_x_pos = m_io_pen_x->read();
	return rescale(io_pen_x_pos, io_pen_x_min, io_pen_x_max, 0x20, 0xdf);
}

u8 pickytlk_base_state::io_pen_y_read()
{
	s16 io_pen_y_min = m_io_pen_y->field(0xff)->minval();
	s16 io_pen_y_max = m_io_pen_y->field(0xff)->maxval();
	s16 io_pen_y_pos = pen_y_rescale_r();
	return (m_pen_target == PEN_TARGET_LCD)
		? rescale(io_pen_y_pos, io_pen_y_min, io_pen_y_max, 0x20, 0xa0)
		: rescale(io_pen_y_pos, io_pen_y_min, io_pen_y_max, 0xa0, 0xf0);
}

void pickytlk_base_state::tablet_write(offs_t offset, u8 data)
{
	LOGMASKED(LOG_TABLET, "%s: tablet_write [%02x] = %02x\n", machine().describe_context(), offset, data);
	m_io_tablet_regs[offset] = data;
}

void pickytlk_base_state::kol_w(u8 data)
{
	m_ko = (m_ko & 0xff00) | data;
	LOGMASKED(LOG_IO, "%s: KO = %04x\n", machine().describe_context(), m_ko);
}

void pickytlk_base_state::koh_w(u8 data)
{
	m_ko = (m_ko & 0x00ff) | (u16(data) << 8);
	LOGMASKED(LOG_IO, "%s: KO = %04x\n", machine().describe_context(), m_ko);
}

void pickytlk_base_state::port_w(u8 data)
{
	m_port = data;
	LOGMASKED(LOG_IO, "%s: PORT = %02x\n", machine().describe_context(), m_port);
}

void pickytlk_base_state::opt_w(u8 data)
{
	m_opt = data;
	LOGMASKED(LOG_IO, "%s: OPT = %02x\n", machine().describe_context(), m_opt);
}

u8 pickytlk_base_state::ki_r()
{
	if (BIT(m_io_buttons->read(), 6))
	{
		if (m_pen_state == PEN_RELEASE)
		{
			m_pen_state = PEN_PRESS;
			// FIXME: Adjust delay when more accurate instruction timings are implemented.
			// Program code waits for input flag to be stable by executing `mov DSIZE,0xff`
			// then `movq R00,R00` 15 times (see pickytlk ROM @ 2015f6).
			m_io_timer->adjust(attotime::from_msec(20), 0, attotime::never);
		}
	}
	else
	{
		m_pen_state = PEN_RELEASE;
		m_io_timer->reset(attotime::never);
	}

	return (m_pen_state == PEN_PRESS) ? 0x80 : 0;
}

u8 pickytlk_base_state::in0_r()
{
	// battery level?
	// bit4 -> if reset CPU keeps restarting (several unknown instructions before jumping to 0)
	//         perhaps a battery present check?
	// bit 5 -> 0 = low battery

	// --XX ---- VDET
	// ---- -X-- data-in
	return 0x30 & ~0x00;
}

u8 pickytlk_base_state::input_flag_read()
{
	return (m_pen_state == PEN_PRESS || m_pen_state == PEN_HOLD) ? 0 : 1;
}

void pickytlk_base_state::update_crosshair(screen_device &screen)
{
	// Either screen crosshair or layout view's cursor should be visible at a time.
	machine().crosshair().get_crosshair(0).set_screen(m_pen_target ? CROSSHAIR_SCREEN_NONE : &screen);
}

u32 pickytlk_base_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	update_crosshair(screen);

	u16 offset = 0;

	for (int i = 0; i < 16; i++)
	{
		int const x = i * 8;

		for (int j = 0; j < 64; j++)
		{
			u16 *const row = &bitmap.pix(63 - j);

			u8 const data1 = m_display_ram[offset];
			u8 const data2 = m_display_ram[offset + 0x400];

			for (int b = 0; b < 8; b++)
			{
				if (x + b < 127)
				{
					row[x + b] = (BIT(data1, b) << 1) | BIT(data2, b);
				}
			}

			offset++;
		}
	}

	return 0;
}


static INPUT_PORTS_START(pickytlk)
	// TODO: On/Off/Reset
	PORT_START("BUTTONS")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Pen Down")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(pickytlk_base_state::pen_target_r))

	PORT_START("PEN_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_NAME("Pen X")

	PORT_START("PEN_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_NAME("Pen Y") PORT_CROSSHAIR_MAPPER_MEMBER(FUNC(pickytlk_base_state::pen_y_mapper))

	PORT_START("PEN_Y_RESCALE")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(pickytlk_base_state::pen_y_rescale_r))
INPUT_PORTS_END


void pickytlk_base_state::pickytlk(machine_config &config)
{
	HCD62121(config, m_maincpu, 4300000); /* X1 - 4.3 MHz */
	m_maincpu->kol_cb().set(FUNC(pickytlk_base_state::kol_w));
	m_maincpu->koh_cb().set(FUNC(pickytlk_base_state::koh_w));
	m_maincpu->port_cb().set(FUNC(pickytlk_base_state::port_w));
	m_maincpu->opt_cb().set(FUNC(pickytlk_base_state::opt_w));
	m_maincpu->ki_cb().set(FUNC(pickytlk_base_state::ki_r));
	m_maincpu->in0_cb().set(FUNC(pickytlk_base_state::in0_r));
	m_maincpu->input_flag_cb().set(FUNC(pickytlk_base_state::input_flag_read));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(127, 64);
	screen.set_visarea(0, 126, 0, 63);
	screen.set_screen_update(FUNC(pickytlk_base_state::screen_update));
	screen.set_palette("palette");
}


class pickytlk_monocolor_state : public pickytlk_base_state
{
public:
	pickytlk_monocolor_state(const machine_config &mconfig, device_type type, const char *tag)
		: pickytlk_base_state(mconfig, type, tag)
	{ }

	void pickytlk_monocolor(machine_config &config) ATTR_COLD;

private:
	virtual u8 tablet_read(offs_t offset) override;
	void pickytlk_mem(address_map &map) ATTR_COLD;
	void pickytlk_palette(palette_device &palette) const ATTR_COLD;
};

u8 pickytlk_monocolor_state::tablet_read(offs_t offset)
{
	LOGMASKED(LOG_TABLET, "%s: tablet_read [%02x] = %02x\n", machine().describe_context(), offset, m_io_tablet_regs[offset]);

	if (BIT(offset, 0))
	{
		u8 y = BIT(m_ko, 3) ? io_pen_y_read() : 0;
		LOGMASKED(LOG_TABLET, "%s: pen y = %02x\n", machine().describe_context(), y);
		return BIT(m_ko, 0) ? y : (0xff - y);
	}
	else
	{
		u8 x = BIT(m_ko, 2) ? io_pen_x_read() : 0;
		LOGMASKED(LOG_TABLET, "%s: pen x = %02x\n", machine().describe_context(), x);
		return BIT(m_ko, 0) ? x : (0xff - x);
	}
}

void pickytlk_monocolor_state::pickytlk_mem(address_map &map)
{
	map(0x000000, 0x00bfff).rom();
//  map(0x040000, 0x04ffff) // Unknown
	map(0x080000, 0x0807ff).ram();
//  map(0x100000, 0x10ffff) // Unknown
	map(0x200000, 0x2fffff).rom().region("mask_rom", 0);
	map(0x400000, 0x4007ff).ram().share("display_ram");
	map(0x400800, 0x41ffff).ram();
	// Read after pen callibration
	map(0x800030, 0x80003f).rw(FUNC(pickytlk_monocolor_state::tablet_read), FUNC(pickytlk_monocolor_state::tablet_write));
	// Read before pen callibration
	map(0x800040, 0x80004f).rw(FUNC(pickytlk_monocolor_state::tablet_read), FUNC(pickytlk_monocolor_state::tablet_write));
//  map(0xe00000, 0xe0ffff) // LCD I/O
}

void pickytlk_monocolor_state::pickytlk_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0xee, 0xee, 0xcc);
	palette.set_pen_color(1, 0x11, 0x11, 0x11);
	palette.set_pen_color(2, 0x11, 0x11, 0x11);
	palette.set_pen_color(3, 0x11, 0x11, 0x11);
}

void pickytlk_monocolor_state::pickytlk_monocolor(machine_config &config)
{
	pickytlk_base_state::pickytlk(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &pickytlk_monocolor_state::pickytlk_mem);

	// TODO: Verify palette. Colors can be changed by changing the contrast.
	PALETTE(config, "palette", FUNC(pickytlk_monocolor_state::pickytlk_palette), 4);

	config.set_default_layout(layout_pickytlk);
}


class pickytlk_multicolor_state : public pickytlk_base_state
{
public:
	pickytlk_multicolor_state(const machine_config &mconfig, device_type type, const char *tag)
		: pickytlk_base_state(mconfig, type, tag)
	{ }

	void plets(machine_config &config) ATTR_COLD;
	void pickytlk_multicolor(machine_config &config) ATTR_COLD;

private:
	virtual u8 tablet_read(offs_t offset) override;
	void pickytlk_mem(address_map &map) ATTR_COLD;
	void pickytlk_palette(palette_device &palette) const ATTR_COLD;
};

u8 pickytlk_multicolor_state::tablet_read(offs_t offset)
{
	/*
	    Pen coordinates can return a mirrored value when bit 4 is not set.
	    Both pairs of values <x1,x2>, <y1,y2> are approximated from these tests:
	    - General case:
	        - x1 + x2 > 0xc8
	        - y1 + y2 > 0xc8
	    - Reset screen with "OK" prompt:
	        - x1 > 0x7d
	        - y1 > 0x7d
	    - Pen callibration:
	        - x1 < 0x72
	        - x2 < 0x72
	        - y1 < 0x64
	        - y1 > 0x68
	*/
	LOGMASKED(LOG_TABLET, "%s: tablet_read [%02x] = %02x\n", machine().describe_context(), offset, m_io_tablet_regs[offset]);
	switch (offset)
	{
	case 0:
		{
			u8 y = BIT(m_ko, 7) ? io_pen_y_read() : 0;
			LOGMASKED(LOG_TABLET, "%s: pen y = %02x\n", machine().describe_context(), y);
			return BIT(m_ko, 4) ? y : (0xff - y);
		}
	case 1:
		{
			u8 x = BIT(m_ko, 6) ? io_pen_x_read() : 0;
			LOGMASKED(LOG_TABLET, "%s: pen x = %02x\n", machine().describe_context(), x);
			return BIT(m_ko, 4) ? x : (0xff - x);
		}
	case 4:
		// Can return 0 if other values are not stable/ready?
		return 0x80;
	default:
		return m_io_tablet_regs[offset];
	}
}

void pickytlk_multicolor_state::pickytlk_mem(address_map &map)
{
	map(0x000000, 0x007fff).rom();
	map(0x080000, 0x0807ff).ram();
	map(0x080300, 0x08030f).rw(FUNC(pickytlk_multicolor_state::tablet_read), FUNC(pickytlk_multicolor_state::tablet_write));
//  map(0x100000, 0x10ffff) // Unknown
//  map(0x110000, 0x11ffff) // LCD I/O
	map(0x200000, 0x2fffff).rom().region("mask_rom", 0);
	map(0x400000, 0x4007ff).ram().share("display_ram");
	map(0x400800, 0x41ffff).ram();
//  map(0xe10000, 0xe1ffff) // LCD I/O
}

void pickytlk_multicolor_state::pickytlk_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0xee, 0xee, 0xcc);
	palette.set_pen_color(1, 0x11, 0x33, 0x99);
	palette.set_pen_color(2, 0x33, 0xcc, 0x77);
	palette.set_pen_color(3, 0xee, 0x77, 0x33);
}

void pickytlk_multicolor_state::plets(machine_config &config)
{
	pickytlk_base_state::pickytlk(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &pickytlk_multicolor_state::pickytlk_mem);

	// TODO: Verify palette. Colors can be changed by changing the contrast.
	PALETTE(config, "palette", FUNC(pickytlk_multicolor_state::pickytlk_palette), 4);
}

void pickytlk_multicolor_state::pickytlk_multicolor(machine_config &config)
{
	plets(config);

	config.set_default_layout(layout_pickytlk);
}


#define CPU_ROM_MONOCOLOR \
	ROM_REGION(0xc000, "maincpu", 0) \
	ROM_LOAD("cpu.lsi1", 0x0000, 0xc000, CRC(67c76253) SHA1(2f7d368e584608d0ed3135159c36f35f02cdd8c4))

#define CPU_ROM_MULTICOLOR \
	ROM_REGION(0x8000, "maincpu", 0) \
	ROM_LOAD("cpu.lsi1", 0x0000, 0x8000, CRC(d58efff9) SHA1(a8d2c2a331d79c5299274e2f2d180deda60a5aed))


ROM_START(jd363)
	CPU_ROM_MONOCOLOR

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("d23c8000lwgx-c11.lsi5", 0x00000, 0x100000, CRC(d4c6e7d2) SHA1(ef199725f04da977cd47293dba8086011f156baf))
ROM_END

ROM_START(pickydis)
	CPU_ROM_MONOCOLOR

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("d23c8000lwgx-c14.lsi5", 0x00000, 0x100000, CRC(1a6273c7) SHA1(f0601873503272d9a620789b5c9798f6e1baf4e7))
ROM_END

ROM_START(jd364)
	CPU_ROM_MULTICOLOR

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("d23c8000lwgx-c12.lsi5", 0x00000, 0x100000, CRC(c023b709) SHA1(0c081a62f00d0fbee496a5c9067fb145cb79c8cd))
ROM_END

ROM_START(jd368)
	CPU_ROM_MULTICOLOR

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("d23c8000xgx-c42.lsi5", 0x00000, 0x100000, CRC(c396bafb) SHA1(0d5610d288ab2474017c05ed54fc816d2e82525f))
ROM_END

ROM_START(pickytlk)
	CPU_ROM_MULTICOLOR

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("d23c8000xgx-c64.lsi5", 0x00000, 0x100000, CRC(6ed6feae) SHA1(f9a63db3d048da0954cab052690deb01ec384b22))
ROM_END

ROM_START(plets300)
	CPU_ROM_MULTICOLOR

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("d23c8000xgx-c77.lsi5", 0x00000, 0x100000, CRC(50ecb853) SHA1(5f2564ccb6ff7e0e5a21064ca32626f35dc81506))
ROM_END

ROM_START(plets350)
	CPU_ROM_MULTICOLOR

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("m538032e-48.lsi5", 0x00000, 0x100000, CRC(42068a99) SHA1(24f8dc15a51a391d4c35cce7332d55f1fa4d8160))
ROM_END

} // anonymous namespace


// Release date 1995-09 from "Casio Game Perfect Catalogue"
COMP(1995, jd363, 0, 0, pickytlk_monocolor, pickytlk, pickytlk_monocolor_state, empty_init, "Casio", "Picky Talk - Super Denshi Techou", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// ROM date 9539K7001
COMP(1995?, pickydis, 0, 0, pickytlk_monocolor, pickytlk, pickytlk_monocolor_state, empty_init, "Tsukuda Original", "Disney Characters - Tegaki Electronic Note", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// Release date 1995-10 from "Casio Game Perfect Catalogue"
COMP(1995, jd364, 0, 0, pickytlk_multicolor, pickytlk, pickytlk_multicolor_state, empty_init, "Casio", "Color Picky Talk - Super Denshi Techou", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// ROM date 9737K7041
COMP(1997?, jd368, 0, 0, pickytlk_multicolor, pickytlk, pickytlk_multicolor_state, empty_init, "Casio", "Super Picky Talk - Access Pet", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// Release date 1998-07 from "Casio Game Perfect Catalogue"
COMP(1998, pickytlk, 0, 0, pickytlk_multicolor, pickytlk, pickytlk_multicolor_state, empty_init, "Casio", "Super Picky Talk - Forest of Gurutan", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// Release date 1999-09 from "Casio Game Perfect Catalogue"
COMP(1999, plets300, 0, 0, plets, pickytlk, pickytlk_multicolor_state, empty_init, "Casio", "Plet's (MK-300)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// ROM date 0019K7001
COMP(2000?, plets350, 0, 0, plets, pickytlk, pickytlk_multicolor_state, empty_init, "Casio", "Plet's (MK-350)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
