// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**************************************************************************
    Pirate Ship

 PWB(A)354460B

 MC68HC00FN16

 054539  - 8-Channel ADPCM sound generator. Clock input 18.432MHz. Clock outputs 18.432/4 & 18.432/8
 053250  - LVC road generator
 053246A - Sprite generator
 055673  - Sprite generator
 055555  - Mixer/Priority encoder
 056832  - Tilemap generator
 054156  - Tilemap generator
 053252  - CRTC


 053250 config:

 SELC (69)  GND
 SEL1 (83)  GND
 SEL0 (82)  GND
 MODE (68)  GND

 TODO: Music stops if a coin is inserted. MAME or BTNAB?

**************************************************************************/

#include "emu.h"

#include "konamigx.h" // TODO: WHY?

#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "machine/k053252.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "sound/k054539.h"
#include "k053246_k053247_k055673.h"
#include "k053250_ps.h"
#include "k054000.h"
#include "k055555.h"
#include "konami_helper.h"

#include "speaker.h"


namespace {

class piratesh_state : public driver_device
{
public:
	piratesh_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_k053250(*this, "k053250"),
		m_k053252(*this, "k053252"),
		m_k056832(*this, "k056832"),
		m_k055673(*this, "k055673"),
		m_k055555(*this, "k055555"),
		//m_k053246(*this, "k053246"),
		m_k054539(*this, "k054539"),
		m_tickets(*this, "ticket"),
		m_hopper(*this, "hopper"),
		m_spriteram(*this,"spriteram")
	{ }

	void piratesh(machine_config &config);

	ioport_value helm_r();
	ioport_value battery_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	required_device<k053250ps_device> m_k053250;
	required_device<k053252_device> m_k053252;
	required_device<k056832_device> m_k056832;
	required_device<k055673_device> m_k055673;
	required_device<k055555_device> m_k055555;
	required_device<k054539_device> m_k054539;
//  required_device<k053247_device> m_k053246;

	required_device<ticket_dispenser_device> m_tickets;
	required_device<ticket_dispenser_device> m_hopper;

	optional_shared_ptr<uint16_t> m_spriteram;

	int m_layer_colorbase[6]{};
	int m_sprite_colorbase = 0;
	int m_lvc_colorbase = 0;

	uint8_t m_int_enable = 0;
	uint8_t m_int_status = 0;
	uint8_t m_sound_ctrl = 0;
	uint8_t m_sound_nmi_clk = 0;
	uint16_t m_control = 0;

	void update_interrupts();

	uint16_t K056832_rom_r(offs_t offset);
	void control1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void control2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void control3_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint32_t screen_update_piratesh(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void k054539_nmi_gen(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(piratesh_interrupt);
	K056832_CB_MEMBER(piratesh_tile_callback);
	K055673_CB_MEMBER(piratesh_sprite_callback);
	void piratesh_map(address_map &map) ATTR_COLD;
};


void piratesh_state::update_interrupts()
{
	m_maincpu->set_input_line(M68K_IRQ_2, m_int_status & 2 ? ASSERT_LINE : CLEAR_LINE); // INT 1
	m_maincpu->set_input_line(M68K_IRQ_4, m_int_status & 1 ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(M68K_IRQ_5, m_int_status & 4 ? ASSERT_LINE : CLEAR_LINE);
}

/*
 Priority issues:

 1. On title screen, stars should be behind the helm
 2. The Konami logo is a square transition
 3.

*/

K056832_CB_MEMBER(piratesh_state::piratesh_tile_callback)
{
	// Layer
	// Code
	// Color
	// Flags
//  if (*color != 0)
//      logerror("%x %x %x\n", layer, *code, *color >> 2);

	*color = (m_layer_colorbase[layer] << 4) + ((*color >> 2));// & 0x0f);
}

K055673_CB_MEMBER(piratesh_state::piratesh_sprite_callback)
{
	int c = *color;

	*color = (c & 0x001f);
	//int pri = (c >> 5) & 7;
	// .... .... ...x xxxx - Color
	// .... .... xxx. .... - Priority?
	// .... ..x. .... .... - ?
	// ..x. .... .... .... - ?

#if 0
	int layerpri[4];
	static const int pris[4] = { K55_PRIINP_0, K55_PRIINP_3, K55_PRIINP_6, K55_PRIINP_7 };

	for (uint32_t i = 0; i < 4; i++)
	{
		layerpri[i] = m_k055555->K055555_read_register(pris[i]);
	}

	// TODO: THIS IS ALL WRONG
	if (pri <= layerpri[0])
		*priority_mask = 0;
	else if (pri <= layerpri[1])
		*priority_mask = 0xf0;
	else if (pri <= layerpri[2])
		*priority_mask = 0xf0|0xcc;
	else
		*priority_mask = 0xf0|0xcc|0xaa;
#endif

	*priority_mask = 0;

	// 0 - Sprites over everything
	// f0 -
	// f0 cc -
	// f0 cc aa -

	// 1111 0000
	// 1100 1100
	// 1010 1010
}



void piratesh_state::video_start()
{
	// TODO: These come from the 055555
	m_layer_colorbase[0] = 0;
	m_layer_colorbase[1] = 2;
	m_layer_colorbase[2] = 4;
	m_layer_colorbase[3] = 6;
	m_sprite_colorbase = 1;
	m_lvc_colorbase = 3;
#if 0
	konamigx_mixer_init(*m_screen, 0);

	m_k056832->set_layer_offs(0, -2+2-1, 0-1);
	m_k056832->set_layer_offs(1,  0+2, 0);
	m_k056832->set_layer_offs(2,  2+2, 0);
	m_k056832->set_layer_offs(3,  3+2, 0);
#endif
}


uint32_t piratesh_state::screen_update_piratesh(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
#if 1

	int layers[4], layerpri[4];
	static const int pris[4] = { K55_PRIINP_0, K55_PRIINP_3, K55_PRIINP_6, K55_PRIINP_7 };
	static const int enables[4] = { K55_INP_VRAM_A, K55_INP_VRAM_B, K55_INP_VRAM_C, K55_INP_VRAM_D };

	for (uint32_t i = 0; i < 4; i++)
	{
		layers[i] = i;
		layerpri[i] = m_k055555->K055555_read_register(pris[i]);
	}

	konami_sortlayers4(layers, layerpri);

	screen.priority().fill(0, cliprect);

	const uint32_t input_enables = m_k055555->K055555_read_register(K55_INPUT_ENABLES);

	// TODO: FIX COLORBASES
	if (input_enables & K55_INP_SUB3)
		m_k053250->draw(bitmap, cliprect, 0x20, 0, screen.priority(), 2);
	for (uint32_t i = 0; i < 4; i++)
	{
		if (input_enables & enables[layers[i]])
		{
			m_k056832->tilemap_draw(screen, bitmap, cliprect, layers[i], 0, 1 << i);
		}
	}

	if (input_enables & K55_INP_SUB2)
		m_k055673->k053247_sprites_draw(bitmap, cliprect);

#if 0
#define K55_INP_VRAM_A      0x01
#define K55_INP_VRAM_B      0x02
#define K55_INP_VRAM_C      0x04
#define K55_INP_VRAM_D      0x08
#define K55_INP_OBJ         0x10
#define K55_INP_SUB1        0x20
#define K55_INP_SUB2        0x40
#define K55_INP_SUB3        0x80
#endif

	//055555: 4 to reg 7 (A PRI 0)
	//055555: 0 to reg 8 (A PRI 1)
	//055555: 0 to reg 9 (A COLPRI)
	//055555: 6 to reg a (B PRI 0)
	//055555: 0 to reg b (B PRI 1)
	//055555: 0 to reg c (B COLPRI)
	//055555: 16 to reg d (C PRI)
	//055555: 18 to reg e (D PRI)
	//055555: 0 to reg 11 (SUB2 PRI)
	//055555: 0 to reg 12 (SUB3 PRI)

	//055555: 0 to reg 17 (A PAL)
	//055555: 2 to reg 18 (B PAL)
	//055555: 4 to reg 19 (C PAL)
	//055555: 6 to reg 1a (D PAL)
	//055555: 3 to reg 1d (SUB2 PAL)
	//055555: 1 to reg 1e (SUB3 PAL)


#else
	// LAYER, FLAGS, PRIORITY
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 3, K056832_DRAW_FLAG_MIRROR, 1);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 2, K056832_DRAW_FLAG_MIRROR, 2);

	// TODO: Fix priority
	m_k053250->draw(bitmap, cliprect, 0x20, 0, screen.priority(), 8);

	m_k055673->k053247_sprites_draw(bitmap, cliprect);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 1, K056832_DRAW_FLAG_MIRROR, 4);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 0, K056832_DRAW_FLAG_MIRROR, 0);
#endif
	return 0;
}




/**********************************************************************************/
/* IRQ controllers */

TIMER_DEVICE_CALLBACK_MEMBER(piratesh_state::piratesh_interrupt)
{
	int scanline = param;

	// IRQ2 - CCUINT1 (VBL START)
	// IRQ4 - Sound
	// IRQ5 - CCUINT2 (VBL END)

	if (scanline == 240)
	{
		m_k053250->vblank_w(1);

		if (m_int_enable & 2)
		{
			m_int_status |= 2;
			update_interrupts();
		}
	}

	if (scanline == 0)
	{
		m_k053250->vblank_w(0);

		if (m_int_enable & 4)
		{
			m_int_status |= 4;
			update_interrupts();
		}
	}
}



uint16_t piratesh_state::K056832_rom_r(offs_t offset)
{
	uint16_t offs;

	offs = (m_control & 2 ? 0x1000 : 0) + offset;
	return m_k056832->piratesh_rom_r(offs);
}



void piratesh_state::control1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// .... ..xx .... ....      - Unknown
	// .... .x.. .... ....      - Unknown - Active during attract, clear during game
	// .... x... .... ....      - Lamp? (active when waiting to start game)

	if (data & ~0x0f00)
		logerror("CTRL3: %x %x %x\n", offset, data, mem_mask);
}

void piratesh_state::control2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// .... .... ...x ....      - Unknown (always 1?)
	// .... .... ..x. ....      - Unknown
	// .... .... .x.. ....      - Counter out
	// .... .... x... ....      - Counter in
	// .... ...x .... ....      - 053246A OBJCRBK (Pin 9)
	// .... ..x. .... ....      - LV related
	// .... x... .... ....      - INT4/SND control (0=clear 1=enable)
	// ...x .... .... ....      - INT2/CCUINT1 control (0=clear 1=enable)
	// ..x. .... .... ....      - INT5/CCUINT2 control (0=clear 1=enable)
	// .x.. .... .... ....      - Unknown
	// x... .... .... ....      - Unknown

	m_int_enable = (data >> 11) & 7;
	m_int_status &= m_int_enable;
	update_interrupts();

	if (data & ~0xfbf0)
		logerror("CTRL2: %x %x %x\n", offset, data, mem_mask);
}

void piratesh_state::control3_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// .... .... .... ...x      - Watchdog? (051550?)
	// .... .... .... ..x.      - 056832 ROM bank control
	// .... .... ...x ....      - Ticket dispenser enable (active high)
	// .... .... ..x. ....      - Hopper enable (active high)
	// .... ...x .... ....      - Unknown (always 1?)

	if ((data & ~0x0133) || (~data & 0x100))
		logerror("CTRL1 W: %x %x %x\n", offset, data, mem_mask);

//  logerror("CTRL 1: %x\n", data & 0x0010);
	m_tickets->motor_w(data & 0x0010 ? 1 : 0);
	m_hopper->motor_w(data & 0x0020 ? 1 : 0);

	m_control = data;
}


void piratesh_state::piratesh_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x083fff).ram().share("nvram");
	map(0x084000, 0x087fff).ram();
	map(0x100000, 0x10001f).rw(m_k053252, FUNC(k053252_device::read), FUNC(k053252_device::write)).umask16(0x00ff); // CRTC
	map(0x180000, 0x18003f).w(m_k056832, FUNC(k056832_device::word_w)); // TILEMAP
	map(0x280000, 0x280007).w(m_k055673, FUNC(k055673_device::k053246_w)); // SPRITES
	map(0x290000, 0x29000f).r(m_k055673, FUNC(k055673_device::k055673_ps_rom_word_r)); // SPRITES
	map(0x290010, 0x29001f).w(m_k055673, FUNC(k055673_device::k055673_reg_word_w)); // SPRITES
	map(0x2a0000, 0x2a0fff).rw(m_k055673, FUNC(k055673_device::k053247_word_r), FUNC(k055673_device::k053247_word_w)); // SPRITES
	map(0x2a1000, 0x2a3fff).nopw();
	map(0x2b0000, 0x2b000f).rw(m_k053250, FUNC(k053250ps_device::reg_r), FUNC(k053250ps_device::reg_w)); // LVC
	map(0x300000, 0x3000ff).w(m_k055555, FUNC(k055555_device::K055555_word_w));
	map(0x380000, 0x381fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x400001).portr("IN0");
	map(0x400002, 0x400003).portr("IN1");
	map(0x400004, 0x400005).portr("DSW1");
	map(0x400006, 0x400007).portr("DSW2");
	map(0x400008, 0x400009).portr("SPECIAL");
	map(0x40000c, 0x40000d).w(FUNC(piratesh_state::control1_w));
	map(0x400010, 0x400011).w(FUNC(piratesh_state::control2_w));
	map(0x400014, 0x400015).w(FUNC(piratesh_state::control3_w));
	map(0x500000, 0x50ffff).r(FUNC(piratesh_state::K056832_rom_r)); // VRAM ROM
	map(0x580000, 0x581fff).r(m_k053250, FUNC(k053250ps_device::rom_r)); // LVC ROM access
	map(0x600000, 0x6004ff).rw("k054539", FUNC(k054539_device::read), FUNC(k054539_device::write)).umask16(0xff00); // SOUND
	map(0x680000, 0x681fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w)); // TILEMAP
	map(0x700000, 0x703fff).rw(m_k053250, FUNC(k053250ps_device::ram_r), FUNC(k053250ps_device::ram_w)); // LVC
}


void piratesh_state::k054539_nmi_gen(int state)
{
	static int m_sound_intck = 0; // TODO: KILL ME

	// Trigger an interrupt on the rising edge
	if (!m_sound_intck && state)
	{
		if (m_int_enable & 1)
		{
			m_int_status |= 1;
			update_interrupts();
		}
	}

	m_sound_intck = state;
}

ioport_value piratesh_state::helm_r()
{
	// Appears to be a quadrature encoder
	uint8_t xa, xb;
	uint16_t dx = ioport("HELM")->read();

	xa = ((dx + 1) & 7) <= 3;
	xb = (dx & 7) <= 3;

	return (xb << 1) | xa;
}

ioport_value piratesh_state::battery_r()
{
	// .x MB3790 /ALARM1
	// x. MB3790 /ALARM2

	return 0x3;
}

/**********************************************************************************/

static INPUT_PORTS_START( piratesh )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) // 7f60  btst $7,$40000
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) // HELM?
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )   // HELM?
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F2)

	PORT_START("SPECIAL")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("k053250", k053250ps_device, dmairq_r)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // FIXME: NCPU from 053246 (DMA)
	PORT_BIT( 0x0c00, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(piratesh_state, battery_r)

	PORT_START("HELM")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(1)

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x1800, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(piratesh_state, helm_r)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1") // TODO: DIP switches are used for settings when battery failure has occurred
	PORT_DIPNAME( 0x0100, 0x0100, "DSW1:0" ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "DSW1:1" ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "DSW1:2" ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "DSW1:3" ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x1000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x0000, "A" )
	PORT_DIPSETTING(    0x1000, "B" )
	PORT_DIPSETTING(    0x2000, "C" )
	PORT_DIPSETTING(    0x3000, "D" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("DSW2") // TODO: Finish me
	PORT_DIPNAME( 0x0100, 0x0100, "DSW2:0" ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "DSW2:1" ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "DSW2:2" ) PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "DSW2:3" ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "DSW2:4" ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "DSW2:5" ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "DSW2:6" ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Redemption Type" ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x8000, "Ticket" )
	PORT_DIPSETTING(    0x0000, "Capsule" )
INPUT_PORTS_END

/**********************************************************************************/

void piratesh_state::machine_start()
{
#if 0
	m_sound_ctrl = 2;
	m_mw_irq_control = 0;

	/* konamigx_mixer uses this, so better initialize it */
	m_gx_wrport1_0 = 0;

	save_item(NAME(m_mw_irq_control));
	save_item(NAME(m_sound_ctrl));
	save_item(NAME(m_sound_nmi_clk));
#else
	(void)m_sound_ctrl;
	(void)m_sound_nmi_clk;
#endif
}

void piratesh_state::machine_reset()
{
	m_int_status = 0;

	int i;

	// soften chorus(chip 0 channel 0-3), boost voice(chip 0 channel 4-7)
	for (i=0; i<=7; i++)
	{
	//  m_k054539->set_gain(i, 0.5);
	}

//  // soften percussions(chip 1 channel 0-7)
//  for (i=0; i<=7; i++) m_k054539_2->set_gain(i, 0.5);

}

void piratesh_state::piratesh(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &piratesh_state::piratesh_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(piratesh_state::piratesh_interrupt), "screen", 0, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	K053252(config, m_k053252, XTAL(32'000'000)/4);
	m_k053252->set_offsets(40, 16); // TODO

	TICKET_DISPENSER(config, "ticket", attotime::from_msec(200));
	HOPPER(config, "hopper", attotime::from_msec(200));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
//  screen.set_refresh_hz(60);
	screen.set_raw(6000000, 288+16+32+48, 0, 287, 224+16+8+16, 0, 223); // TODO
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(600));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(24, 24+288-1, 16, 16+224-1);
	screen.set_screen_update(FUNC(piratesh_state::screen_update_piratesh));

	PALETTE(config, "palette").set_format(palette_device::BGRx_888, 2048).enable_shadows().enable_hilights();

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(piratesh_state::piratesh_tile_callback));
	m_k056832->set_config(K056832_BPP_4PIRATESH, 1, 0);
	m_k056832->set_palette("palette");

	K055555(config, m_k055555, 0);

	K053250PS(config, m_k053250, 12000000, "palette", "screen", -16, 0);

	K055673(config, m_k055673, 0);
	m_k055673->set_sprite_callback(FUNC(piratesh_state::piratesh_sprite_callback));
	m_k055673->set_config(K055673_LAYOUT_PS, -60, 24);
	m_k055673->set_palette("palette");

	// ????
	//K053246(config, m_k053246, 0);
	//m_k053246->set_sprite_callback(FUNC(moo_state::sprite_callback));
	//m_k053246->set_config("k053246", NORMAL_PLANE_ORDER, -48+1, 23);
	//m_k053246->set_palette(m_palette);

	K054338(config, "k054338", 0, m_k055555).set_alpha_invert(1);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	K054539(config, m_k054539, XTAL(18'432'000));
	m_k054539->timer_handler().set(FUNC(piratesh_state::k054539_nmi_gen));
	m_k054539->add_route(0, "lspeaker", 0.2);
	m_k054539->add_route(1, "rspeaker", 0.2);
}



ROM_START( piratesh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "360ua-c04.4p", 0x000000, 0x80000, CRC(6d69dd90) SHA1(ccbdbfea406d9cbc3f242211290ba82ccbbe3795) )

	/* tiles */
	ROM_REGION( 0x80000, "k056832", ROMREGION_ERASE00 ) // 27C4096
	ROM_LOAD( "360ua-a01.17g", 0x000000, 0x80000, CRC(e39153f5) SHA1(5da9132a2c24a15b55c3f65c26e2ad0467411a88) )

	/* sprites */
	ROM_REGION( 0x80000*8, "k055673", ROMREGION_ERASE00 ) // 27C4096
	ROM_LOAD16_BYTE( "360ua-a02.21l", 0x000000, 0x80000, CRC(82207997) SHA1(fe143285a12fab5227e883113d798acad7bf4c97) )
	ROM_LOAD16_BYTE( "360ua-a03.23l", 0x000001, 0x80000, CRC(a9e36d51) SHA1(1a8de8d8d2abfee5ac0f0822e203846f7f5f1767) )

	/* road generator */
	ROM_REGION( 0x080000, "k053250", ROMREGION_ERASE00 ) // 27C040
	ROM_LOAD( "360ua-a05.26p", 0x000000, 0x80000, CRC(dab7f439) SHA1(2372612c0b04c77a85ccbadc100cb741b85f0481) )

	/* sound data */
	ROM_REGION( 0x100000, "k054539", 0 ) // 27C040
	ROM_LOAD( "360ua-a06.15t", 0x000000, 0x80000, CRC(6816a493) SHA1(4fc4cfbc164d84bbf8d75ccd78c9f40f3273d852) )
	ROM_LOAD( "360ua-a07.17t", 0x080000, 0x80000, CRC(af7127c5) SHA1(b525f3c6b831e3354eba46016d414bedcb3ae8dc) )

//  ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
//  ROM_LOAD( "piratesh.nv", 0x0000, 0x080, CRC(28df2269) SHA1(3f071c97662745a199f96964e2e79f795bd5a391) )
ROM_END

} // anonymous namespace


//    year  name        parent    machine   input     state           init
GAME( 1995, piratesh,   0,        piratesh, piratesh, piratesh_state, empty_init, ROT90,  "Konami", "Pirate Ship (ver UAA)", MACHINE_IMPERFECT_GRAPHICS )
