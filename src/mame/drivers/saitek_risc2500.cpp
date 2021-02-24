// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

Saitek RISC 2500, Mephisto Montreux

The chess engine is also compatible with Tasc's The ChessMachine software.
The hardware+software appears to have been subcontracted to Tasc. It has similarities
with Tasc R30, PCB label and Montreux repair manual schematics footnotes say TASC23C.

notes:
- holding LEFT+RIGHT on boot load the QC TestMode
- holding UP+DOWN on boot load the TestMode

TODO:
- bootrom disable timer shouldn't be needed, real ARM has already fetched the next opcode
- Sound is too short and high pitched, better when you underclock the cpu.
  Is cpu cycle timing wrong? I suspect conditional branch timing due to cache miss
  (pipeline has to refill). The delay loop between writing to the speaker is simply:
  SUBS R2, R2, #$1, BNE $2000cd8

******************************************************************************/


#include "emu.h"
#include "cpu/arm/arm.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "video/sed1520.h"
#include "sound/dac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "saitek_risc2500.lh"


class risc2500_state : public driver_device
{
public:
	risc2500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_nvram(*this, "nvram")
		, m_dac(*this, "dac")
		, m_lcdc(*this, "lcdc")
		, m_board(*this, "board")
		, m_inputs(*this, "P%u", 0)
		, m_digits(*this, "digit%u", 0U)
		, m_syms(*this, "sym%u", 0U)
		, m_leds(*this, "led%u", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(on_button);

	void risc2500(machine_config &config);

protected:
	uint32_t p1000_r();
	void p1000_w(uint32_t data);
	uint32_t disable_boot_rom_r();
	TIMER_CALLBACK_MEMBER(disable_boot_rom);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	SED1520_UPDATE_CB(screen_update_cb);
	void install_boot_rom();
	void remove_boot_rom();
	void lcd_palette(palette_device &palette) const;

	void risc2500_mem(address_map &map);

private:
	required_device<arm_cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_device<dac_byte_interface> m_dac;
	required_device<sed1520_device> m_lcdc;
	required_device<sensorboard_device> m_board;
	required_ioport_array<8> m_inputs;
	output_finder<12> m_digits;
	output_finder<14> m_syms;
	output_finder<16> m_leds;

	uint32_t  m_p1000;
	emu_timer *m_boot_rom_disable_timer;
};


void risc2500_state::install_boot_rom()
{
	m_maincpu->space(AS_PROGRAM).install_rom(0x00000000, 0x001ffff, memregion("maincpu")->base());
}

void risc2500_state::remove_boot_rom()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0x00000000, m_ram->size() - 1, m_ram->pointer());
}

void risc2500_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(131, 136, 139)); // lcd pixel off
	palette.set_pen_color(1, rgb_t(51, 42, 43)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(138, 146, 148)); // background
}

SED1520_UPDATE_CB(risc2500_state::screen_update_cb)
{
	bitmap.fill(2, cliprect);

	for (int c=0; c<12; c++)
	{
		// 12 characters 5 x 7
		for (int x=0; x<5; x++)
		{
			uint8_t gfx = 0;
			if (lcd_on)
				gfx = bitswap<8>(dram[c * 5 + x], 6,5,0,1,2,3,4,7);

			for (int y=1; y<8; y++)
				bitmap.pix(y, 71 - (c * 6 + x)) = BIT(gfx, y);
		}

		// LCD digits and symbols
		if (lcd_on)
		{
			int data_addr = 80 + c * 5;
			uint16_t data = ((dram[data_addr + 1] & 0x3) << 5) | ((dram[data_addr + 2] & 0x7) << 2) | (dram[data_addr + 4] & 0x3);
			data = bitswap<8>(data, 7,3,0,1,4,6,5,2) | ((dram[data_addr - 1] & 0x04) ? 0x80 : 0);

			m_digits[c] = data;
			m_syms[c] = BIT(dram[data_addr + 1], 2);
		}
		else
		{
			m_digits[c] = 0;
			m_syms[c] = 0;
		}
	}

	m_syms[12] = lcd_on ? BIT(dram[0x73], 0) : 0;
	m_syms[13] = lcd_on ? BIT(dram[0x5a], 0) : 0;

	return 0;
}

INPUT_CHANGED_MEMBER(risc2500_state::on_button)
{
	if (newval)
	{
		install_boot_rom();
		m_maincpu->reset();
	}
}

static INPUT_PORTS_START( risc2500 )
	PORT_START("P0")
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD)     PORT_NAME("Pawn")
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("BACK")

	PORT_START("P1")
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD)     PORT_NAME("Knight")
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER)     PORT_NAME("ENTER")

	PORT_START("P2")
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD)     PORT_NAME("Bishop")
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN)      PORT_NAME("DOWN")

	PORT_START("P3")
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD)     PORT_NAME("Rook")
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP)        PORT_NAME("UP")

	PORT_START("P4")
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD)     PORT_NAME("Queen")
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M)         PORT_NAME("MENU")

	PORT_START("P5")
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD)     PORT_NAME("King")
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L)         PORT_NAME("PLAY")

	PORT_START("P6")
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT)     PORT_NAME("RIGHT")
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N)         PORT_NAME("NEW GAME")

	PORT_START("P7")
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT)      PORT_NAME("LEFT")
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O)         PORT_NAME("OFF")

	PORT_START("RESET")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I)         PORT_NAME("ON")  PORT_CHANGED_MEMBER(DEVICE_SELF, risc2500_state, on_button, 0)
INPUT_PORTS_END


uint32_t risc2500_state::p1000_r()
{
	uint32_t data = 0;

	for (int i=0; i<8; i++)
	{
		if (m_p1000 & (1 << i))
		{
			data |= m_inputs[i]->read();
			data |= m_board->read_rank(i, true);
		}
	}

	return data | ((uint32_t)m_lcdc->status_read() << 16);
}

void risc2500_state::p1000_w(uint32_t data)
{
	if (!BIT(data, 27))
	{
		if (BIT(data, 26))
			m_lcdc->data_write(data);
		else
			m_lcdc->control_write(data);
	}

	if (BIT(data, 31))                     // Vertical LED
	{
		for (int i=0; i<8; i++)
			m_leds[i] = BIT(data, i);
	}

	if (BIT(data, 30))                     // Horizontal LED
	{
		for (int i=0; i<8; i++)
			m_leds[8 + i] = BIT(data, i);
	}

	m_dac->write(data >> 28 & 3);                   // Speaker

	m_p1000 = data;
}

uint32_t risc2500_state::disable_boot_rom_r()
{
	m_boot_rom_disable_timer->adjust(m_maincpu->cycles_to_attotime(10));
	return 0;
}

TIMER_CALLBACK_MEMBER(risc2500_state::disable_boot_rom)
{
	remove_boot_rom();
}

void risc2500_state::machine_start()
{
	m_digits.resolve();
	m_syms.resolve();
	m_leds.resolve();

	m_nvram->set_base(m_ram->pointer(), m_ram->size());

	m_boot_rom_disable_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(risc2500_state::disable_boot_rom), this));

	save_item(NAME(m_p1000));

	machine().save().register_postload(save_prepost_delegate(FUNC(risc2500_state::remove_boot_rom), this));
}

void risc2500_state::machine_reset()
{
	m_p1000 = 0;

	install_boot_rom();
}

void risc2500_state::risc2500_mem(address_map &map)
{
	map(0x00000000, 0x0001ffff).ram();
	map(0x01800000, 0x01800003).r(FUNC(risc2500_state::disable_boot_rom_r));
	map(0x01000000, 0x01000003).rw(FUNC(risc2500_state::p1000_r), FUNC(risc2500_state::p1000_w));
	map(0x02000000, 0x0203ffff).rom().region("maincpu", 0);
}

void risc2500_state::risc2500(machine_config &config)
{
	ARM(config, m_maincpu, XTAL(28'322'000) / 2); // VY86C010
	m_maincpu->set_addrmap(AS_PROGRAM, &risc2500_state::risc2500_mem);
	m_maincpu->set_copro_type(arm_cpu_device::copro_type::VL86C020);
	m_maincpu->set_periodic_int(FUNC(risc2500_state::irq1_line_hold), attotime::from_hz(32.768_kHz_XTAL/128)); // 256Hz

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(12*6+1, 7+2);
	screen.set_visarea_full();
	screen.set_screen_update(m_lcdc, FUNC(sed1520_device::screen_update));
	screen.set_palette("palette");

	config.set_default_layout(layout_saitek_risc2500);

	PALETTE(config, "palette", FUNC(risc2500_state::lcd_palette), 3);

	SED1520(config, m_lcdc);
	m_lcdc->set_screen_update_cb(FUNC(risc2500_state::screen_update_cb));

	SENSORBOARD(config, m_board);
	m_board->set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(100));
	m_board->set_nvram_enable(true);

	RAM(config, m_ram).set_extra_options("128K, 256K, 512K, 1M, 2M");
	m_ram->set_default_size("2M");
	m_ram->set_default_value(0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_BINARY_WEIGHTED_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
}


/* ROM definitions */

ROM_START( risc2500 )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("risc2500_v.1.04.u7", 0x000000, 0x020000, CRC(84a06178) SHA1(66f4d9f53de6da865a3ebb4af1d6a3e245c59a3c) ) // M27C1001
ROM_END

ROM_START( risc2500a )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("risc2500_v.1.03.u7", 0x000000, 0x020000, CRC(7a707e82) SHA1(87187fa58117a442f3abd30092cfcc2a4d7c7efc) )
ROM_END

ROM_START( montreux ) // v1.00
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("rt17b_103_u_7.u7", 0x000000, 0x040000, CRC(db374cf3) SHA1(44dd60d56779084326c3dfb41d2137ebf0b4e0ac) ) // 27C020-15
ROM_END


/*    YEAR  NAME       PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS */
CONS( 1992, risc2500,  0,        0,      risc2500, risc2500, risc2500_state, empty_init, "Saitek / Tasc", "Kasparov RISC 2500 (v1.04)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_SOUND )
CONS( 1992, risc2500a, risc2500, 0,      risc2500, risc2500, risc2500_state, empty_init, "Saitek / Tasc", "Kasparov RISC 2500 (v1.03)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_SOUND )

CONS( 1995, montreux,  0,        0,      risc2500, risc2500, risc2500_state, empty_init, "Saitek / Tasc", "Mephisto Montreux", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_SOUND ) // after Saitek bought Hegener + Glaser
