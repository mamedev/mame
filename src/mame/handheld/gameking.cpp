// license:GPL-2.0+
// copyright-holders:Peter Trauner, AJR
/*
  TimeTop - GameKing

  PeT mess@utanet.at 2015

  Thanks to Deathadder, Judge, Porchy, Klaus Sommer, James Brolly & Brian Provinciano

  hopefully my work (reverse engineering, cartridge+bios backup, emulation) will be honored in future
  and my name will not be removed entirely, especially by simple code rewrites of working emulation

  flashcard, handheld, programmer, assembler ready to do some test on real hardware

  todo:
  !back up gameking3 bios so emulation of gameking3 gets possible; my gameking bios backup solution should work
  (improve emulation)
  (add audio)

  use gameking3 cartridge to get illegal cartridge scroller
*/

#include "emu.h"

#include "cpu/m6502/st2204.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

namespace {

class gameking_state : public driver_device
{
public:
	gameking_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_io_joy(*this, "JOY"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void gameking(machine_config &config);
	void gameking3(machine_config &config);
	void gameking1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<st2204_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_ioport m_io_joy;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;

	void gameking_palette(palette_device &palette) const;
	void timer_w(uint8_t data);
	uint8_t input_r();
	uint8_t input2_r();
	TIMER_CALLBACK_MEMBER(gameking_timer1);
	TIMER_CALLBACK_MEMBER(gameking_timer2);

	uint32_t screen_update_gameking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gameking3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void gameking_mem(address_map &map) ATTR_COLD;
	void gameking3_mem(address_map &map) ATTR_COLD;

	emu_timer *m_timer1;
	emu_timer *m_timer2;
	uint8_t m_timer1_val = 0;
};


void gameking_state::timer_w(uint8_t data)
{
	m_timer1_val = data;
	//m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	m_timer1->enable(true);
	m_timer1->reset(m_maincpu->cycles_to_attotime(data * 300/*?*/));
}

uint8_t gameking_state::input_r()
{
	return m_io_joy->read() | ~3;
}

uint8_t gameking_state::input2_r()
{
	return m_io_joy->read() | 3;
}

void gameking_state::gameking_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
}

void gameking_state::gameking3_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x800000, 0x807fff).ram();
}


static INPUT_PORTS_START( gameking )
	PORT_START("JOY")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SELECT) // ?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2) // A
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1) // B
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // ?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
INPUT_PORTS_END

static INPUT_PORTS_START( gameking3 )
	PORT_INCLUDE( gameking )
	PORT_MODIFY("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SELECT) // ?
INPUT_PORTS_END

static constexpr rgb_t gameking_pens[] =
{
	{ 255, 255, 255 },
	{ 127, 127, 127 },
	{  63,  63,  63 },
	{   0,   0,   0 }
};

void gameking_state::gameking_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, gameking_pens);
}


uint32_t gameking_state::screen_update_gameking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space *maincpu_ram = &m_maincpu->space(AS_PROGRAM);
	offs_t lssa = m_maincpu->state_int(st2xxx_device::ST_LSSA);
	if (lssa < 0x0080)
		return 0;

	for (int y=31, i=0;i<32;i++,y--)
	{
		for (int x=0, j=0;j<48/4;x+=4, j++)
		{
			uint8_t data=maincpu_ram->read_byte(lssa+j+i*12);
			bitmap.pix(y, x+3)=data&3;
			bitmap.pix(y, x+2)=(data>>2)&3;
			bitmap.pix(y, x+1)=(data>>4)&3;
			bitmap.pix(y, x)=(data>>6)&3;
		}
	}
	return 0;
}


static constexpr uint8_t gameking3_intensities[] =
{
	0,
	127,
	191,
	255
};

uint32_t gameking_state::screen_update_gameking3(screen_device& screen, bitmap_rgb32 &bitmap, const rectangle& cliprect)
{
	address_space* maincpu_ram = &m_maincpu->space(AS_PROGRAM);
	offs_t lssa = m_maincpu->state_int(st2xxx_device::ST_LSSA);
	if (lssa < 0x0080)
		return 0;

	for (int y = 0, i = 0; i < 80; y += 2, i++)
	{
		for (int x = 0, j = 0; j < 40; x += 4, j++)
		{
			uint8_t data=maincpu_ram->read_byte(lssa+j+i*40);

			// apply SPRD-C color filter
			switch (i % 3)
			{
			case 0:
				bitmap.pix(y, x + 3) = rgb_t(0, gameking3_intensities[data&3], 0);
				bitmap.pix(y + 1, x + 2) = rgb_t(gameking3_intensities[(data>>2)&3], 0, 0);
				bitmap.pix(y, x + 1) = rgb_t(0, gameking3_intensities[(data>>4)&3], 0);
				bitmap.pix(y + 1, x) = rgb_t(gameking3_intensities[(data>>6)&3], 0, 0);
				break;

			case 1:
				bitmap.pix(y, x + 3) = rgb_t(0, 0, gameking3_intensities[data&3]);
				bitmap.pix(y + 1, x+2) = rgb_t(0, gameking3_intensities[(data>>2)&3], 0);
				bitmap.pix(y, x + 1) = rgb_t(0, 0, gameking3_intensities[(data>>4)&3]);
				bitmap.pix(y + 1, x) = rgb_t(0, gameking3_intensities[(data>>6)&3], 0);
				break;

			case 2:
				bitmap.pix(y, x + 3) = rgb_t(gameking3_intensities[data&3], 0, 0);
				bitmap.pix(y + 1, x+2) = rgb_t(0, 0, gameking3_intensities[(data>>2)&3]);
				bitmap.pix(y, x + 1) = rgb_t(gameking3_intensities[(data>>4)&3], 0, 0);
				bitmap.pix(y + 1, x) = rgb_t(0, 0, gameking3_intensities[(data>>6)&3]);
				break;
			}
		}
	}

	// interpolate values for dots in between
	for (int y = 0; y < 160; y++)
	{
		for (int x = y & 1; x < 160; x += 2)
		{
			rgb_t l = rgb_t(x == 0 ? 0 : bitmap.pix(y, x - 1));
			rgb_t r = rgb_t(x == 159 ? 0 : bitmap.pix(y, x + 1));
			rgb_t u = rgb_t(y == 0 ? 0 : bitmap.pix(y - 1, x));
			rgb_t d = rgb_t(y == 159 ? 0 : bitmap.pix(y + 1, x));

			bitmap.pix(y, x) = rgb_t(
				((u.r() + d.r()) * 2 + l.r() + r.r()) / 3,
				((u.g() + d.g()) * 2 + l.g() + r.g()) / 3,
				((u.b() + d.b()) * 2 + l.b() + r.b()) / 3
			);
		}
	}

	return 0;
}


TIMER_CALLBACK_MEMBER(gameking_state::gameking_timer1)
{
	m_maincpu->set_state_int(st2xxx_device::ST_IREQ,
		m_maincpu->state_int(st2xxx_device::ST_IREQ) | (0x010 & m_maincpu->state_int(st2xxx_device::ST_IENA)));
	m_timer1->enable(false);
	m_timer2->enable(true);
	m_timer2->reset(m_maincpu->cycles_to_attotime(10/*?*/));
}

TIMER_CALLBACK_MEMBER(gameking_state::gameking_timer2)
{
	//m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE); // in reality int for vector at fff4
	m_timer2->enable(false);
	m_timer1->enable(true);
	m_timer1->reset(m_maincpu->cycles_to_attotime(m_timer1_val * 300/*?*/));
}

DEVICE_IMAGE_LOAD_MEMBER(gameking_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	if (size > 0x10'0000)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be no larger than 1M)");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void gameking_state::machine_start()
{
	std::string region_tag;
	memory_region *cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	if (cart_rom)
		m_maincpu->space(AS_DATA).install_rom(0x400000, 0x400000 + cart_rom->bytes() - 1, cart_rom->base()); // FIXME: gamekin3 wants Flash cartridges, not plain ROM

	m_timer1 = timer_alloc(FUNC(gameking_state::gameking_timer1), this);
	m_timer2 = timer_alloc(FUNC(gameking_state::gameking_timer2), this);

	save_item(NAME(m_timer1_val));
}


void gameking_state::gameking(machine_config &config)
{
	// basic machine hardware
	ST2204(config, m_maincpu, 6000000);
	m_maincpu->set_addrmap(AS_DATA, &gameking_state::gameking_mem);
	m_maincpu->in_pa_callback().set(FUNC(gameking_state::input_r));
	m_maincpu->in_pb_callback().set(FUNC(gameking_state::input2_r));
	m_maincpu->out_pc_callback().set(FUNC(gameking_state::timer_w)); // wrong
	m_maincpu->in_pl_callback().set_constant(6); // bios protection endless loop
	m_maincpu->dac_callback().set("dac", FUNC(dac_byte_interface::write));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(48, 32);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(gameking_state::screen_update_gameking));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(gameking_state::gameking_palette), std::size(gameking_pens));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, "dac", 0).add_route(0, "speaker", 1.0);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "gameking_cart", "bin").set_device_load(FUNC(gameking_state::cart_load));
}

void gameking_state::gameking1(machine_config &config)
{
	gameking(config);
	SOFTWARE_LIST(config, "cart_list").set_original("gameking");
}

void gameking_state::gameking3(machine_config &config)
{
	// basic machine hardware
	gameking(config);
	m_maincpu->set_clock(8000000);
	m_maincpu->set_addrmap(AS_DATA, &gameking_state::gameking3_mem);

	// video hardware
	m_screen->set_size(160, 160);
	m_screen->set_visarea_full();
	m_screen->set_physical_aspect(3, 2);
	m_screen->set_refresh_hz(39.308176); // ?
	m_screen->set_screen_update(FUNC(gameking_state::screen_update_gameking3));
	m_screen->set_no_palette();

	config.device_remove("palette");

	// cartridge
	SOFTWARE_LIST(config, "cart_list").set_original("gameking");
	SOFTWARE_LIST(config, "cart_list_3").set_original("gameking3");
}


ROM_START(gameking)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("gm218.bin", 0x00000, 0x80000, CRC(5a1ade3d) SHA1(e0d056f8ebfdf52ef6796d0375eba7fcc4a6a9d3) )
ROM_END

ROM_START(gamekin3)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("gm220.bin", 0x00000, 0x80000, CRC(1dc43bd5) SHA1(f9dcd3cb76bb7cb10565a1acb070ab375c082b4c) )
ROM_END

} // anonymous namespace


CONS( 2003, gameking, 0, 0, gameking1, gameking,  gameking_state, empty_init, "TimeTop", "GameKing GM-218", MACHINE_IMPERFECT_SOUND )
// the GameKing 2 (GM-219) is probably identical HW

CONS( 2003, gamekin3, 0, 0, gameking3, gameking3, gameking_state, empty_init, "TimeTop", "GameKing 3",      MACHINE_IMPERFECT_SOUND )
// gameking 3: similiar cartridges, accepts gameking cartridges, gameking3 cartridges not working on gameking (illegal cartridge scroller)
// my gameking bios backup solution might work on it
