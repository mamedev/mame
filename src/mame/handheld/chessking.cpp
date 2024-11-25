// license:BSD-3-Clause
// copyright-holders:David Haywood, hap
/*

Chess King (棋王之王), LCD handheld console presumably from Taiwan.
Hold down U+D+L+R buttons at boot to enter factory reset mode.
Hold down START at boot to enter test mode.

TODO:
- lots of unknown writes
- dump/add more cartridges? considering how unknown the handheld is, maybe only a handful were released
- LCD chip(s) is not emulated, maybe the I/O chip does a DMA from RAM to the LCD?
- chess game is buggy, assume that's just the way it is, aka BTANB
  eg. sometimes it makes an illegal move, or castling doesn't erase the king from its original spot

Hardware notes:

Main:
- PCB label: TD-24
- NEC D70108HG-10 V20, 9.600MHz XTAL
- 64KB RAM (2*SRM20256), 256KB ROM (custom label)
- unknown 80 pin QFP, label CCH01 ET-MATE F3X0 713, assume custom I/O chip
- 1-bit sound
- cartridge slot

LCD module:
- PCB label: P102-2, DATA VISION
- 2*Hitachi HD66204F, Hitachi HD66205F
- unknown 80 pin QFP, label 16160 S2RB 94.10
- 2bpp 160*160 LCD screen

*/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/nec/nec.h"
#include "machine/nvram.h"
#include "sound/beep.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class chessking_state : public driver_device
{
public:
	chessking_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_mainrom(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_videoram(*this, "videoram"),
		m_beeper(*this, "beeper"),
		m_cart(*this, "cartslot")
	{ }

	void chesskng(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_region_ptr<uint8_t> m_mainrom;
	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<beep_device> m_beeper;
	required_device<generic_slot_device> m_cart;

	uint8_t m_3f_data = 0;
	uint8_t m_cart_bank = 0;
	uint16_t m_beeper_freq = 0;

	void chesskng_map(address_map &map) ATTR_COLD;
	void chesskng_io(address_map &map) ATTR_COLD;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	uint8_t cartridge_r(offs_t offset);
	void cart_bank_w(uint8_t data);

	void update_beeper();
	void beeper_freq_low_w(uint8_t data);
	void beeper_freq_high_w(uint8_t data);
	void beeper_enable_w(uint8_t data);

	INTERRUPT_GEN_MEMBER(interrupt);
	void irq_clear_w(uint8_t data);
	void unk_1f_w(uint8_t data);
	void unk_2f_w(uint8_t data);
	uint8_t unk_3f_r();
	void unk_3f_w(uint8_t data);
	void unk_5f_w(uint8_t data);
	void unk_6f_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

void chessking_state::machine_start()
{
	save_item(NAME(m_3f_data));
	save_item(NAME(m_cart_bank));
	save_item(NAME(m_beeper_freq));
}



/*******************************************************************************
    Video
*******************************************************************************/

uint32_t chessking_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// quickly draw from memory (should be handled by LCDC?)
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int offset = y * 256 / 8;

		uint32_t *dst = &bitmap.pix(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			// 2 256x256 images (160x160 area used) one at c000, one at e000 to form 2bpp graphics
			uint8_t data = m_videoram[0x4000 + offset + x/8];
			uint8_t data2 = m_videoram[0x6000 + offset + x/8];
			uint8_t pix = BIT(data, ~x & 7) | BIT(data2, ~x & 7) << 1;

			static const rgb_t pens[4] = { rgb_t::white(), rgb_t(0x55,0x55,0x55), rgb_t(0xaa,0xaa,0xaa), rgb_t::black() };
			dst[x] = pens[pix];
		}
	}

	return 0;
}



/*******************************************************************************
    Sound
*******************************************************************************/

void chessking_state::beeper_freq_low_w(uint8_t data)
{
	//logerror("%s: 8f write %02x\n", machine().describe_context(), data);
	m_beeper_freq = (m_beeper_freq & 0xff00) | data;
	update_beeper();
}

void chessking_state::beeper_freq_high_w(uint8_t data)
{
	//logerror("%s: 9f write %02x\n", machine().describe_context(), data);
	m_beeper_freq = (m_beeper_freq & 0x00ff) | data << 8;
	update_beeper();
}

void chessking_state::beeper_enable_w(uint8_t data)
{
	// writes same value here after writing to 0x3f, assume sound related since
	// the register is in proximity of sound freq registers
	m_beeper->set_state(BIT(data, 3));
}

void chessking_state::update_beeper()
{
	uint16_t freq = (~m_beeper_freq & 0x1ff) + 1;
	double base = (9.6_MHz_XTAL).dvalue() / 0x80;
	m_beeper->set_clock(base / freq);
}



/*******************************************************************************
    Cartridge
*******************************************************************************/

DEVICE_IMAGE_LOAD_MEMBER(chessking_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

uint8_t chessking_state::cartridge_r(offs_t offset)
{
	// bank 1 selects main rom
	if (m_cart_bank == 1)
		return m_mainrom[offset & 0x3ffff];

	// banks 4-7 go to cartridge
	else if (m_cart_bank >= 4)
		return m_cart->read_rom(offset | (m_cart_bank & 3) << 19);

	// other banks: maybe cartridge too?
	return 0;
}

void chessking_state::cart_bank_w(uint8_t data)
{
	m_cart_bank = data & 0x7;
}



/*******************************************************************************
    Misc I/O
*******************************************************************************/

INTERRUPT_GEN_MEMBER(chessking_state::interrupt)
{
	device.execute().set_input_line_and_vector(0, ASSERT_LINE, 0x20/4);
}

void chessking_state::irq_clear_w(uint8_t data)
{
	// writes here at the start of irq routine
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

void chessking_state::unk_1f_w(uint8_t data)
{
	// writes at start-up only
	logerror("%s: 1f write %02x\n", machine().describe_context(), data);
}

void chessking_state::unk_2f_w(uint8_t data)
{
	// writes at start-up only
	logerror("%s: 2f write %02x\n", machine().describe_context(), data);
}

uint8_t chessking_state::unk_3f_r()
{
	return m_3f_data;
}

void chessking_state::unk_3f_w(uint8_t data)
{
	// writes frequently, at the same times as 7f/8f/9f/af writes, note address is also read
	//logerror("%s: 3f write %02x\n", machine().describe_context(), data);
	m_3f_data = data;
}

void chessking_state::unk_5f_w(uint8_t data)
{
	// writes at start-up only
	logerror("%s: 5f write %02x\n", machine().describe_context(), data);
}

void chessking_state::unk_6f_w(uint8_t data)
{
	logerror("%s: 6f write %02x\n", machine().describe_context(), data);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void chessking_state::chesskng_map(address_map &map)
{
	map(0x00000, 0x7ffff).mirror(0x80000).r(FUNC(chessking_state::cartridge_r));
	map(0x00000, 0x1ffff).unmapr();

	map(0x00000, 0x07fff).ram().share(m_mainram); // SRM20256, battery-backed
	map(0x08000, 0x0ffff).ram().share(m_videoram); // SRM20256

	map(0xe0000, 0xfffff).rom().region("maincpu", 0x20000);
}

void chessking_state::chesskng_io(address_map &map)
{
	map(0x0f, 0x0f).portr("BUTTONS");
	map(0x1f, 0x1f).w(FUNC(chessking_state::unk_1f_w));
	map(0x2f, 0x2f).w(FUNC(chessking_state::unk_2f_w));
	map(0x3f, 0x3f).rw(FUNC(chessking_state::unk_3f_r), FUNC(chessking_state::unk_3f_w));
	map(0x4f, 0x4f).w(FUNC(chessking_state::irq_clear_w));
	map(0x5f, 0x5f).w(FUNC(chessking_state::unk_5f_w));
	map(0x6f, 0x6f).w(FUNC(chessking_state::unk_6f_w));
	map(0x7f, 0x7f).w(FUNC(chessking_state::cart_bank_w));
	map(0x8f, 0x8f).w(FUNC(chessking_state::beeper_freq_low_w));
	map(0x9f, 0x9f).w(FUNC(chessking_state::beeper_freq_high_w));
	map(0xaf, 0xaf).w(FUNC(chessking_state::beeper_enable_w));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( chesskng )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SELECT ) // SELECT / 選擇
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START ) // START / 啓動
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) // A / 確認 (confirm)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) // B / 回位 (return)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void chessking_state::chesskng(machine_config &config)
{
	// Basic machine hardware
	V20(config, m_maincpu, 9.6_MHz_XTAL); // D70108HG-10 V20
	m_maincpu->set_addrmap(AS_PROGRAM, &chessking_state::chesskng_map);
	m_maincpu->set_addrmap(AS_IO, &chessking_state::chesskng_io);

	attotime irq_freq = attotime::from_hz(9.6_MHz_XTAL / 0x10000); // gives approximate seconds on timer
	m_maincpu->set_periodic_int(FUNC(chessking_state::interrupt), irq_freq);

	NVRAM(config, "mainram", nvram_device::DEFAULT_ALL_0);

	// Video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(160, 160);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(chessking_state::screen_update));

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	BEEP(config, m_beeper, 0);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);

	// Cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "chessking_cart");
	m_cart->set_device_load(FUNC(chessking_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("chessking_cart");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( chesskng )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "etmate-cch.u6", 0x00000, 0x40000, CRC(a4d1764b) SHA1(ccfae1e985f6ad316ff192206fbc0f8bcd4e44d5) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS            INIT        COMPANY             FULLNAME                   FLAGS
SYST( 1994, chesskng, 0,      0,      chesskng, chesskng, chessking_state, empty_init, "I-Star Co., Ltd.", "Chess King (model ET-6)", MACHINE_SUPPORTS_SAVE )
