// license:GPL-2.0+
// copyright-holders:Peter Trauner
/* TimeTop - GameKing */
/*
  PeT mess@utanet.at 2015

  Thanks to Deathadder, Judge, Porchy, Klaus Sommer, James Brolly & Brian Provinciano

  hopefully my work (reverse engineerung, cartridge+bios backup, emulation) will be honored in future
  and my name will not be removed entirely, especially by simple code rewrites of working emulation

  flashcard, handheld, programmer, assembler ready to do some test on real hardware

  todo:
  !back up gameking3 bios so emulation of gameking3 gets possible; my gameking bios backup solution should work
  (improove emulation)
  (add audio)

  use gameking3 cartridge to get illegal cartridge scroller
*/

#include "emu.h"
#include "cpu/m6502/st2204.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "emupal.h"
#include "screen.h"
#include "softlist.h"


class gameking_state : public driver_device
{
public:
	gameking_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_cart(*this, "cartslot"),
		m_io_joy(*this, "JOY")
	{ }

	void gameking(machine_config &config);
	void gameking1(machine_config &config);

	void init_gameking();

	void gameking_mem(address_map &map);

	uint8_t input_r();
	uint8_t input2_r();

	void timer_w(uint8_t data);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

protected:
	virtual void machine_start() override;

	uint32_t screen_update_gameking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<st2xxx_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	
	void gameking_palette(palette_device& palette) const;

	TIMER_CALLBACK_MEMBER(gameking_timer);
	TIMER_CALLBACK_MEMBER(gameking_timer2);

	required_device<generic_slot_device> m_cart;
	required_ioport m_io_joy;

	emu_timer *timer1;
	emu_timer *timer2;

	uint8_t m_timer;
};


class gameking3_state : public gameking_state
{
public:
	gameking3_state(const machine_config& mconfig, device_type type, const char* tag) :
		gameking_state(mconfig, type, tag)
	{ }

	void gameking3(machine_config &config);
	void gameking3_mem(address_map &map);

	uint32_t screen_update_gameking3(screen_device& screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};



void gameking_state::timer_w(uint8_t data)
{
	m_timer = data;
	//m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	timer1->enable(true);
	timer1->reset(m_maincpu->cycles_to_attotime(data * 300/*?*/));
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

void gameking3_state::gameking3_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x800000, 0x802fff).ram();
	map(0x804800, 0x8048ff).ram();
}


static INPUT_PORTS_START( gameking )
	PORT_START("JOY")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START) PORT_NAME("Start")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("Select") //?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("A")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) //?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
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
			bitmap.pix16(y, x+3)=data&3;
			bitmap.pix16(y, x+2)=(data>>2)&3;
			bitmap.pix16(y, x+1)=(data>>4)&3;
			bitmap.pix16(y, x)=(data>>6)&3;
		}
	}
	return 0;
}

uint32_t gameking3_state::screen_update_gameking3(screen_device& screen, bitmap_rgb32 &bitmap, const rectangle& cliprect)
{
	address_space* maincpu_ram = &m_maincpu->space(AS_PROGRAM);
	offs_t lssa = m_maincpu->state_int(st2xxx_device::ST_LSSA);
	if (lssa < 0x0080)
		return 0;

	int count = 0;
	for (int y = 0; y < 27; y++)
	{
		for (int x = 0; x < 40; x++)
		{
			// this is incorrect, but it seems to be at least 12 bits per pixel when compared to the 2 bits for gameking
			uint8_t data = maincpu_ram->read_byte(lssa + count);
			uint8_t data2 = maincpu_ram->read_byte(lssa + count+40);
			uint8_t data3 = maincpu_ram->read_byte(lssa + count+80);

			int r = (data & 0x0f)<<4;
			int g = (data2 & 0x0f)<<4;
			int b = (data3 & 0x0f)<<4;

			bitmap.pix32(y, (x * 2) + 1) = (r << 16) | (g << 8) | (b << 0);
			
			r = (data & 0xf0);
			g = (data2 & 0xf0);
			b = (data3 & 0xf0);
	
			bitmap.pix32(y, (x * 2) + 0) = (r << 16) | (g << 8) | (b << 0);

			count += 1;
		}

		count += 40;
		count += 40;
	}
	return 0;
}


void gameking_state::init_gameking()
{
	timer1 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gameking_state::gameking_timer), this));
	timer2 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gameking_state::gameking_timer2), this));
}

TIMER_CALLBACK_MEMBER(gameking_state::gameking_timer)
{
	m_maincpu->set_state_int(st2xxx_device::ST_IREQ,
		m_maincpu->state_int(st2xxx_device::ST_IREQ) | (0x016 & m_maincpu->state_int(st2xxx_device::ST_IENA)));
	timer1->enable(false);
	timer2->enable(true);
	timer2->reset(m_maincpu->cycles_to_attotime(10/*?*/));
}

TIMER_CALLBACK_MEMBER(gameking_state::gameking_timer2)
{
	//m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE); // in reality int for vector at fff4
	timer2->enable(false);
	timer1->enable(true);
	timer1->reset(m_maincpu->cycles_to_attotime(m_timer * 300/*?*/));
}

DEVICE_IMAGE_LOAD_MEMBER(gameking_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	if (size > 0x100000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

void gameking_state::machine_start()
{
	std::string region_tag;
	memory_region *cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	if (cart_rom)
		m_maincpu->space(AS_DATA).install_rom(0x400000, 0x400000 + cart_rom->bytes() - 1, cart_rom->base()); // FIXME: gamekin3 wants Flash cartridges, not plain ROM
}


void gameking_state::gameking(machine_config &config)
{
	/* basic machine hardware */
	ST2204(config, m_maincpu, 6000000);
	m_maincpu->set_addrmap(AS_DATA, &gameking_state::gameking_mem);
	m_maincpu->in_pa_callback().set(FUNC(gameking_state::input_r));
	m_maincpu->in_pb_callback().set(FUNC(gameking_state::input2_r));
	m_maincpu->out_pc_callback().set(FUNC(gameking_state::timer_w)); // wrong
	m_maincpu->in_pl_callback().set_constant(6); // bios protection endless loop

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(48, 32);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(gameking_state::screen_update_gameking));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(gameking_state::gameking_palette), ARRAY_LENGTH(gameking_pens));

	/* cartridge */
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "gameking_cart", "bin").set_device_load(FUNC(gameking_state::cart_load));
}


void gameking_state::gameking1(machine_config &config)
{
	gameking(config);
	SOFTWARE_LIST(config, "cart_list").set_original("gameking");
}

void gameking3_state::gameking3(machine_config &config)
{
	/* basic machine hardware */
	ST2204(config, m_maincpu, 6000000);
	m_maincpu->set_addrmap(AS_DATA, &gameking3_state::gameking3_mem);
	m_maincpu->in_pa_callback().set(FUNC(gameking_state::input_r));
	m_maincpu->in_pb_callback().set(FUNC(gameking_state::input2_r));
	m_maincpu->out_pc_callback().set(FUNC(gameking_state::timer_w)); // wrong
	m_maincpu->in_pl_callback().set_constant(6); // bios protection endless loop

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(80, 27);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(gameking3_state::screen_update_gameking3));

	/* cartridge */
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "gameking_cart", "bin").set_device_load(FUNC(gameking_state::cart_load));

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

CONS( 2003, gameking, 0, 0, gameking1, gameking, gameking_state, init_gameking, "TimeTop", "GameKing GM-218", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
// the GameKing 2 (GM-219) is probably identical HW

CONS( 2003, gamekin3, 0, 0, gameking3, gameking, gameking3_state, init_gameking, "TimeTop", "GameKing 3",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
// gameking 3: similiar cartridges, accepts gameking cartridges, gameking3 cartridges not working on gameking (illegal cartridge scroller)
// my gameking bios backup solution might work on it
