// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
 watara supervision handheld

 PeT mess@utanet.at in december 2000
******************************************************************************/

#include "emu.h"
#include "includes/svision.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "svision.lh"


#define MAKE8_RGB32(red3, green3, blue2) ( ( (red3)<<(16+5)) | ( (green3)<<(8+5)) | ( (blue2)<<(0+6)) )
#define MAKE9_RGB32(red3, green3, blue3) ( ( (red3)<<(16+5)) | ( (green3)<<(8+5)) | ( (blue3)<<(0+5)) )
#define MAKE12_RGB32(red4, green4, blue4) ( ( (red4)<<(16+4)) | ((green4)<<(8+4)) | ((blue4)<<(0+4)) )
#define MAKE24_RGB32(red8, green8, blue8) ( (((red8)&0xf8)<<16) | (((green8)&0xf8)<<8) | (((blue8)&0xf8)) )


TIMER_CALLBACK_MEMBER(svision_state::svision_pet_timer)
{
	switch (m_pet.state)
	{
		case 0x00:
			if (m_joy2.found())
			{
				m_pet.input = m_joy2->read();
			}
			[[fallthrough]];

		case 0x02: case 0x04: case 0x06: case 0x08:
		case 0x0a: case 0x0c: case 0x0e:
			m_pet.clock = m_pet.state & 2;
			m_pet.data = m_pet.input & 1;
			m_pet.input >>= 1;
			m_pet.state++;
			break;

		case 0x1f:
			m_pet.state = 0;
			break;

		default:
			m_pet.state++;
			break;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(svision_state::svision_pet_timer_dev)
{
	svision_pet_timer(ptr,param);
}

WRITE_LINE_MEMBER(svision_state::sound_irq_w)
{
	m_dma_finished = true;
	check_irq();
}

void svision_state::check_irq()
{
	bool irq = m_svision.timer_shot && BIT(m_reg[BANK], 1);
	irq = irq || (m_dma_finished && BIT(m_reg[BANK], 2));

	m_maincpu->set_input_line(M65C02_IRQ_LINE, irq ? ASSERT_LINE : CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(svision_state::svision_timer)
{
	m_svision.timer_shot = true;
	m_svision.timer1->enable(false);
	check_irq();
}

uint8_t svision_state::svision_r(offs_t offset)
{
	int data = m_reg[offset];
	switch (offset)
	{
		case 0x20:
			return m_joy->read();

		case 0x21:
			data &= ~0xf;
			data |= m_reg[0x22] & 0xf;
			if (m_pet.on)
			{
				if (!m_pet.clock)
				{
					data &= ~4;
				}
				if (!m_pet.data)
				{
					data &= ~8;
				}
			}
			break;

		case 0x27:
			data &= ~3;
			if (m_svision.timer_shot)
			{
				data |= 1;
			}
			if (m_dma_finished)
			{
				data |= 2;
			}
			break;

		case 0x24:
			m_svision.timer_shot = false;
			check_irq();
			break;

		case 0x25:
			m_dma_finished = false;
			check_irq();
			break;

		default:
			logerror("%.6f svision read %04x %02x\n", machine().time().as_double(),offset,data);
			break;
	}

	return data;
}

void svision_state::svision_w(offs_t offset, uint8_t data)
{
	m_reg[offset] = data;

	switch (offset)
	{
		case 0x02:
		case 0x03:
			break;

		case 0x26: /* bits 5,6 memory management for a000? */
		{
			logerror("%.6f svision write %04x %02x\n", machine().time().as_double(), offset, data);
			int bank = ((m_reg[0x26] & 0xe0) >> 5) % (m_cart_rom->bytes() / 0x4000);
			m_bank1->set_base(m_cart_rom->base() + (bank * 0x4000));
			check_irq();
			break;
		}

		case 0x23: /* delta hero irq routine write */
		{
			int delay = (data == 0) ? 0x100 : data;
			delay *= (BIT(m_reg[BANK], 4)) ? 0x4000 : 0x100;
			m_svision.timer1->enable(true);
			m_svision.timer1->reset(m_maincpu->cycles_to_attotime(delay));
			break;
		}

		case 0x10: case 0x11: case 0x12: case 0x13:
			m_sound->soundport_w(0, offset & 3, data);
			break;

		case 0x14: case 0x15: case 0x16: case 0x17:
			m_sound->soundport_w(1, offset & 3, data);
			break;

		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c:
			m_sound->sounddma_w(offset - 0x18, data);
			break;

		case 0x28: case 0x29: case 0x2a:
			m_sound->noise_w(offset - 0x28, data);
			break;

		default:
			logerror("%.6f svision write %04x %02x\n", machine().time().as_double(), offset, data);
			break;
	}
}

uint8_t svision_state::tvlink_r(offs_t offset)
{
	switch(offset)
	{
		default:
			if (offset >= 0x800 && offset < 0x840)
			{
				/* strange effects when modifying palette */
				return svision_r(offset);
			}
			else
			{
				return svision_r(offset);
			}
	}
}

void svision_state::tvlink_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x0e:
			m_reg[offset] = data;
			m_tvlink.palette_on = data & 1;
			if (m_tvlink.palette_on)
			{
				// hack, normally initialising with palette from ram
				m_tvlink.palette[0] = MAKE12_RGB32(163/16,172/16,115/16); // these are the tron colors messured from screenshot
				m_tvlink.palette[1] = MAKE12_RGB32(163/16,155/16,153/16);
				m_tvlink.palette[2] = MAKE12_RGB32(77/16,125/16,73/16);
				m_tvlink.palette[3] = MAKE12_RGB32(59/16,24/16,20/16);
			}
			else
			{
				// cleaner to use colors from compile time palette, or compose from "fixed" palette values
				m_tvlink.palette[0]=MAKE12_RGB32(0,0,0);
				m_tvlink.palette[1]=MAKE12_RGB32(5*16/256,18*16/256,9*16/256);
				m_tvlink.palette[2]=MAKE12_RGB32(48*16/256,76*16/256,100*16/256);
				m_tvlink.palette[3]=MAKE12_RGB32(190*16/256,190*16/256,190*16/256);
			}
			break;
		default:
			svision_w(offset,data);
			if (offset >= 0x800 && offset < 0x840)
			{
				if (offset == 0x803 && data == 0x07)
				{
					/* tron hack */
					m_reg[0x0804] = 0x00;
					m_reg[0x0805] = 0x01;
					m_reg[0x0806] = 0x00;
					m_reg[0x0807] = 0x00;
				}
				uint16_t c = m_reg[0x800] | (m_reg[0x804] << 8);
				m_tvlink.palette[0] = MAKE9_RGB32( (c>>0)&7, (c>>3)&7, (c>>6)&7);
				c = m_reg[0x801] | (m_reg[0x805] << 8);
				m_tvlink.palette[1] = MAKE9_RGB32( (c>>0)&7, (c>>3)&7, (c>>6)&7);
				c = m_reg[0x802] | (m_reg[0x806]<<8);
				m_tvlink.palette[2]=MAKE9_RGB32( (c>>0)&7, (c>>3)&7, (c>>6)&7);
				c = m_reg[0x803] | (m_reg[0x807]<<8);
				m_tvlink.palette[3]=MAKE9_RGB32( (c>>0)&7, (c>>3)&7, (c>>6)&7);
				/* writes to palette effect video color immediately */
				/* some writes modify other registers, */
				/* encoding therefor not known (rgb8 or rgb9) */
			}
	}
}

void svision_state::svision_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x3fff).rw(FUNC(svision_state::svision_r), FUNC(svision_state::svision_w)).share(m_reg);
	map(0x4000, 0x5fff).ram().share(m_videoram);
	map(0x6000, 0x7fff).noprw();
	map(0x8000, 0xbfff).bankr(m_bank1);
	map(0xc000, 0xffff).bankr(m_bank2);
}

void svision_state::tvlink_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x3fff).rw(FUNC(svision_state::tvlink_r), FUNC(svision_state::tvlink_w)).share(m_reg);
	map(0x4000, 0x5fff).ram().share(m_videoram);
	map(0x6000, 0x7fff).noprw();
	map(0x8000, 0xbfff).bankr(m_bank1);
	map(0xc000, 0xffff).bankr(m_bank2);
}

static INPUT_PORTS_START( svision )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("B")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("A")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("Select")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START) PORT_NAME("Start/Pause")
INPUT_PORTS_END

static INPUT_PORTS_START( svisions )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("B") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("A") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("Select") PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START) PORT_NAME("Start/Pause") PORT_PLAYER(1)
	PORT_START("JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("2nd B") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("2nd A") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("2nd Select") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START) PORT_NAME("2nd Start/Pause") PORT_PLAYER(2)
INPUT_PORTS_END

// most games contain their graphics in roms, and have hardware to draw complete rectangular objects

// palette in red, green, blue triples
static constexpr rgb_t svision_pens[] =
{
#if 0
	// greens grabbed from a scan of a handheld in its best adjustment for contrast
	{ 86, 121,  86 },
	{ 81, 115,  90 },
	{ 74, 107, 101 },
	{ 54,  78,  85 }
#else
	// grabbed from chris covell's black white pics
	{ 0xe0, 0xe0, 0xe0 },
	{ 0xb9, 0xb9, 0xb9 },
	{ 0x54, 0x54, 0x54 },
	{ 0x12, 0x12, 0x12 }
#endif
};

// palette in RGB triplets
static constexpr rgb_t svisionp_pens[] =
{
	// pal
	{   1,   1,   3 },
	{   5,  18,   9 },
	{  48,  76, 100 },
	{ 190, 190, 190 }
};

// palette in RGB triplets
static constexpr rgb_t svisionn_pens[] =
{
	{   0,   0,   0 },
	{ 188, 242, 244 }, // darker
	{ 129, 204, 255 },
	{ 245, 249, 248 }
};

void svision_state::svision_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, svision_pens);
}
void svision_state::svisionn_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, svisionn_pens);
}
void svision_state::svisionp_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, svisionp_pens);
}

uint32_t svision_state::screen_update_svision(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_reg[BANK], 3))
	{
		int j = m_reg[XPOS] / 4 + m_reg[YPOS] * 0x30;
		for (int y = 0; y < 160; y++)
		{
			const int start_x = 3 - (m_reg[XPOS] & 3);
			const int end_x = std::min(163, m_reg[XSIZE] | 3);
			uint16_t *line = &bitmap.pix(y, start_x);
			for (int x = start_x, i = 0; x < end_x; x+=4, i++)
			{
				uint8_t b = m_videoram[j+i];
				for (int pix = 0; pix < 4; pix++)
				{
					*line = b & 3;
					b >>= 2;
					line++;
				}
			}
			j += 0x30;
			if (j >= 0x1fe0)
				j = 0; //sssnake
		}
	}
	else
	{
		bitmap.plot_box(3, 0, 162, 159, 0);
	}
	return 0;
}

uint32_t svision_state::screen_update_tvlink(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_reg[BANK], 3))
	{
		int j = m_reg[XPOS] / 4 + m_reg[YPOS] * 0x30;
		for (int y = 0; y < 160; y++)
		{
			const int start_x = 3 - (m_reg[XPOS] & 3);
			const int end_x = std::min(163, m_reg[XSIZE] | 3);
			uint32_t *line = &bitmap.pix(y, start_x);
			for (int x = start_x, i = 0; x < end_x; x += 4, i++)
			{
				uint8_t b = m_videoram[j + i];
				for (int pix = 0; pix < 4; pix++)
				{
					*line = m_tvlink.palette[b & 3];
					b >>= 2;
					line++;
				}
			}
			j += 0x30;
			if (j >= 0x1fe0)
				j = 0; //sssnake
		}
	}
	else
	{
		bitmap.plot_box(3, 0, 162, 159, m_palette->pen(0));
	}
	return 0;
}

WRITE_LINE_MEMBER(svision_state::frame_int_w)
{
	if (!state)
		return;

	if (BIT(m_reg[BANK], 0))
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);

	m_sound->sound_decrement();
}

void svision_state::init_svision()
{
	m_svision.timer1 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(svision_state::svision_timer),this));
	m_dma_finished = false;
	m_pet.on = false;
}

void svision_state::init_svisions()
{
	m_svision.timer1 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(svision_state::svision_timer),this));
	m_dma_finished = false;
	m_pet.on = true;
	m_pet.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(svision_state::svision_pet_timer),this));
}

DEVICE_IMAGE_LOAD_MEMBER( svision_state::cart_load )
{
	uint32_t size = m_cart->common_get_size("rom");

	if (size > 0x80000)
	{
		image.seterror(image_error::INVALIDIMAGE, "Unsupported cartridge size");
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

void svision_state::machine_start()
{
	std::string region_tag(m_cart->tag());
	region_tag.append(GENERIC_ROM_REGION_TAG);
	m_cart_rom = memregion(region_tag.c_str());

	if (m_cart_rom)
	{
		int num_banks = m_cart_rom->bytes() / 0x4000;
		m_bank1->set_base(m_cart_rom->base());
		m_bank2->set_base(m_cart_rom->base() + (num_banks - 1) * 0x4000); // bank2 is set to the last bank
	}
}

void svision_state::machine_reset()
{
	m_svision.timer_shot = false;
	m_dma_finished = false;
}


MACHINE_RESET_MEMBER(svision_state,tvlink)
{
	svision_state::machine_reset();
	m_tvlink.palette_on = false;

	memset(m_reg + 0x800, 0xff, 0x40); // normally done from m_tvlink microcontroller
	m_reg[0x82a] = 0xdf;

	m_tvlink.palette[0] = MAKE24_RGB32(svisionp_pens[0].r(), svisionp_pens[0].g(), svisionp_pens[0].b());
	m_tvlink.palette[1] = MAKE24_RGB32(svisionp_pens[1].r(), svisionp_pens[1].g(), svisionp_pens[1].b());
	m_tvlink.palette[2] = MAKE24_RGB32(svisionp_pens[2].r(), svisionp_pens[2].g(), svisionp_pens[2].b());
	m_tvlink.palette[3] = MAKE24_RGB32(svisionp_pens[3].r(), svisionp_pens[3].g(), svisionp_pens[3].b());
}

void svision_state::svision_base(machine_config &config)
{
	config.set_default_layout(layout_svision);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	SVISION_SND(config, m_sound, 4000000, m_maincpu, m_bank1);
	m_sound->add_route(0, "lspeaker", 0.50);
	m_sound->add_route(1, "rspeaker", 0.50);
	m_sound->irq_cb().set(FUNC(svision_state::sound_irq_w));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "svision_cart", "bin,ws,sv");
	m_cart->set_must_be_loaded(true);
	m_cart->set_device_load(FUNC(svision_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("svision");
}

void svision_state::svision(machine_config &config)
{
	svision_base(config);

	M65C02(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &svision_state::svision_mem);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(61);
	m_screen->set_size(3+160+3, 160);
	m_screen->set_visarea(3+0, 3+160-1, 0, 160-1);
	m_screen->set_screen_update(FUNC(svision_state::screen_update_svision));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(svision_state::frame_int_w));

	PALETTE(config, m_palette, FUNC(svision_state::svision_palette), std::size(svision_pens));
}

void svision_state::svisions(machine_config &config)
{
	svision(config);
	TIMER(config, "pet_timer").configure_periodic(FUNC(svision_state::svision_pet_timer_dev), attotime::from_seconds(8));
}

void svision_state::svisionp(machine_config &config)
{
	svision(config);

	m_maincpu->set_clock(4430000);
	m_screen->set_refresh(HZ_TO_ATTOSECONDS(50));
	m_palette->set_init(FUNC(svision_state::svisionp_palette));
}

void svision_state::svisionn(machine_config &config)
{
	svision(config);
	m_maincpu->set_clock(3560000/*?*/);
	m_screen->set_refresh(HZ_TO_ATTOSECONDS(60));
	m_palette->set_init(FUNC(svision_state::svisionn_palette));
}

void svision_state::tvlinkp(machine_config &config)
{
	svisionp(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &svision_state::tvlink_mem);

	m_screen->set_no_palette();
	m_screen->set_screen_update(FUNC(svision_state::screen_update_tvlink));

	MCFG_MACHINE_RESET_OVERRIDE(svision_state, tvlink)
}

ROM_START(svision)
	ROM_REGION(0x80000, "maincpu", ROMREGION_ERASE00)
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

#define rom_svisions rom_svision
#define rom_svisionn rom_svision
#define rom_svisionp rom_svision
#define rom_tvlinkp  rom_svision

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT     STATE          INIT           COMPANY   FULLNAME                                       FLAGS
// marketed under a ton of firms and names
CONS(1992,  svision,  0,       0,      svision,  svision,  svision_state, init_svision,  "Watara", "Super Vision",                                0 )
// svdual 2 connected via communication port
CONS( 1992, svisions, svision, 0,      svisions, svisions, svision_state, init_svisions, "Watara", "Super Vision (PeT Communication Simulation)", 0 )

CONS( 1993, svisionp, svision, 0,      svisionp, svision,  svision_state, init_svision,  "Watara", "Super Vision (PAL TV Link Colored)",          0 )
CONS( 1993, svisionn, svision, 0,      svisionn, svision,  svision_state, init_svision,  "Watara", "Super Vision (NTSC TV Link Colored)",         0 )
// svtvlink (2 supervisions)
// tvlink (pad supervision simulated)
CONS( 199?, tvlinkp,  svision, 0,      tvlinkp,  svision,  svision_state, init_svision,  "Watara", "TV Link PAL",                                 0 )
