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
#include "sound/dac.h"
#include "sound/volt_reg.h"
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
		, m_board(*this, "board")
		, m_inputs(*this, "P%u", 0)
		, m_digits(*this, "digit%u", 0U)
		, m_syms(*this, "sym%u", 0U)
		, m_leds(*this, "led%u", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(on_button);

	void risc2500(machine_config &config);

protected:
	DECLARE_READ32_MEMBER(p1000_r);
	DECLARE_WRITE32_MEMBER(p1000_w);
	DECLARE_READ32_MEMBER(disable_boot_rom_r);
	TIMER_CALLBACK_MEMBER(disable_boot_rom);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void install_boot_rom();
	void remove_boot_rom();
	void lcd_palette(palette_device &palette) const;

	void risc2500_mem(address_map &map);

private:
	required_device<arm_cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_device<dac_byte_interface> m_dac;
	required_device<sensorboard_device> m_board;
	required_ioport_array<8> m_inputs;
	output_finder<12> m_digits;
	output_finder<14> m_syms;
	output_finder<16> m_leds;

	uint32_t  m_p1000;
	uint16_t  m_vram_addr;
	uint8_t   m_vram[0x100];
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
	palette.set_pen_color(1, rgb_t(92, 83, 88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(138, 146, 148)); // background
}

uint32_t risc2500_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(2, cliprect);

	for(int c=0; c<12; c++)
	{
		// 12 characters 5 x 7
		for(int x=0; x<5; x++)
		{
			uint8_t gfx = bitswap<8>(m_vram[c*5 + x], 6,5,0,1,2,3,4,7);

			for(int y=0; y<7; y++)
				bitmap.pix16(y, 71 - (c*6 + x)) = (gfx >> (y + 1)) & 1;
		}

		// LCD digits and symbols
		int data_addr = 0x40 + c * 5;
		uint16_t data = ((m_vram[data_addr + 1] & 0x3) << 5) | ((m_vram[data_addr + 2] & 0x7) << 2) | (m_vram[data_addr + 4] & 0x3);
		data = bitswap<8>(data, 7,3,0,1,4,6,5,2) | ((m_vram[data_addr - 1] & 0x04) ? 0x80 : 0);

		m_digits[c] = data;
		m_syms[c] = BIT(m_vram[data_addr + 1], 2);
	}

	m_syms[12] = BIT(m_vram[0x63], 0);
	m_syms[13] = BIT(m_vram[0x4a], 0);

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


READ32_MEMBER(risc2500_state::p1000_r)
{
	uint32_t data = 0;

	for(int i=0; i<8; i++)
	{
		if (m_p1000 & (1 << i))
		{
			data |= m_inputs[i]->read();
			data |= m_board->read_rank(i, true);
		}
	}

	return data;
}

WRITE32_MEMBER(risc2500_state::p1000_w)
{
	if ((data & 0xff000000) == 0x01000000)          // VRAM address
	{
		if (data & 0x80)
			m_vram_addr = (m_vram_addr & ~0x40) | (data & 0x01 ? 0x40 : 0);
		else
			m_vram_addr = (m_vram_addr & 0x40) | (data & 0xff);
	}
	else if (data & 0x04000000)                     // VRAM write
	{
		if (!(data & 0x08000000))
			m_vram[m_vram_addr++ & 0x7f] = data & 0xff;
	}
	else if (data & 0x80000000)                     // Vertical LED
	{
		for(int i=0; i<8; i++)
			m_leds[i] = BIT(data, i);
	}
	else if (data & 0x40000000)                     // Horizontal LED
	{
		for(int i=0; i<8; i++)
			m_leds[8 + i] = BIT(data, i);
	}
	else if ((data & 0xff000000) == 0x08000000)     // Power OFF
	{
		memset(m_vram, 0, sizeof(m_vram));
	}

	m_dac->write(data >> 28 & 3);                   // Speaker

	m_p1000 = data;
}

READ32_MEMBER(risc2500_state::disable_boot_rom_r)
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
	save_item(NAME(m_vram_addr));
	save_item(NAME(m_vram));

	machine().save().register_postload(save_prepost_delegate(FUNC(risc2500_state::remove_boot_rom), this));
}

void risc2500_state::machine_reset()
{
	m_p1000 = 0;
	m_vram_addr = 0;

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
	screen.set_size(12*6+1, 7);
	screen.set_visarea(0, 12*6, 0, 7-1);
	screen.set_screen_update(FUNC(risc2500_state::screen_update));
	screen.set_palette("palette");

	config.set_default_layout(layout_saitek_risc2500);

	PALETTE(config, "palette", FUNC(risc2500_state::lcd_palette), 3);

	SENSORBOARD(config, m_board);
	m_board->set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(100));

	RAM(config, m_ram).set_default_size("2M").set_extra_options("128K, 256K, 512K, 1M, 2M");

	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_BINARY_WEIGHTED_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}


/* ROM definitions */

ROM_START( risc2500 )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE )
	ROM_SYSTEM_BIOS( 0, "v104", "v1.04" )
	ROMX_LOAD("s2500_v104.bin", 0x000000, 0x020000, CRC(84a06178) SHA1(66f4d9f53de6da865a3ebb4af1d6a3e245c59a3c), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v103", "v1.03" )
	ROMX_LOAD("s2500_v103.bin", 0x000000, 0x020000, CRC(7a707e82) SHA1(87187fa58117a442f3abd30092cfcc2a4d7c7efc), ROM_BIOS(1))
ROM_END

ROM_START( montreux )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE )
	ROM_SYSTEM_BIOS( 0, "v100", "v1.00" )
	ROMX_LOAD("montreux.bin", 0x000000, 0x040000, CRC(db374cf3) SHA1(44dd60d56779084326c3dfb41d2137ebf0b4e0ac), ROM_BIOS(0))
ROM_END


/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY   FULLNAME              FLAGS */
CONS( 1992, risc2500, 0,      0,      risc2500, risc2500, risc2500_state, empty_init, "Saitek / Tasc", "Kasparov RISC 2500", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_SOUND )
CONS( 1995, montreux, 0,      0,      risc2500, risc2500, risc2500_state, empty_init, "Saitek / Tasc", "Mephisto Montreux", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_SOUND ) // after Saitek bought Hegener + Glaser
