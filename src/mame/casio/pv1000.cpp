// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Driver for Casio PV-1000

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


// PV-1000 Sound device

class pv1000_sound_device : public device_t, public device_sound_interface
{
public:
	pv1000_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void voice_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	// internal state
	struct
	{
		uint32_t  count  = 0;
		uint16_t  period = 0;
		uint8_t   val    = 1; // boot state of all channels
	} m_voice[3];

	uint8_t m_ctrl = 0;
	sound_stream *m_sh_channel = nullptr;
};

DEFINE_DEVICE_TYPE(PV1000, pv1000_sound_device, "pv1000_sound", "NEC D65010G031")

pv1000_sound_device::pv1000_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PV1000, tag, owner, clock),
	device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pv1000_sound_device::device_start()
{
	m_sh_channel = stream_alloc(0, 1, clock() / 1024);

	save_item(STRUCT_MEMBER(m_voice, count));
	save_item(STRUCT_MEMBER(m_voice, period));
	save_item(STRUCT_MEMBER(m_voice, val));

	save_item(NAME(m_ctrl));
}

void pv1000_sound_device::voice_w(offs_t offset, uint8_t data)
{
	offset &= 0x03;
	switch (offset)
	{
	case 0x03:
		m_ctrl = data;
		break;

	default:
	{
		const uint8_t per = ~data & 0x3f;

		if ((per == 0) && (m_voice[offset].period != 0))
		{
			// flip output once and stall there!
			m_voice[offset].val = !m_voice[offset].val;
		}

		m_voice[offset].period = per;
	}
	break;
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

/*
  plgDavid's audio implementation/analysis notes:

  Sound appears to be 3 50/50 pulse voices made by cutting the main clock by 1024,
  then by the value of the 6bit period registers.
  This creates a surprisingly accurate pitch range.
  Note: the register periods are inverted.

  plgDavid 2023 update: lidnariq (NESDEV) took a fondness to the system and gave me a bunch of test roms
  to strenghten the emulation.

  Quite a few things were uncovered overall, but for audio specifically:
  1)Audio mix/mux control ($FB, case 0x03) was ignored
  2)lidnariq's tracing of my PCB scans showed that all three sound outputs are mixed using different volumes:
  square1 via i/o$F8 is -6dB, square2 via i/o$F9 is -3dB, defining square3 via i/o$FA as 0dB
*/

void pv1000_sound_device::sound_stream_update(sound_stream &stream)
{
	// Each channel has a different volume via resistor mixing which correspond to -6dB, -3dB, 0dB drops
	static const int volumes[3] = { 0x1000, 0x1800, 0x2000 };

	for (int index = 0; index < stream.samples(); index++)
	{
		s32 sum = 0;

		// First calculate all vals
		for (int i = 0; i < 3; i++)
		{
			m_voice[i].count++;

			if ((m_voice[i].period > 0) && (m_voice[i].count >= m_voice[i].period))
			{
				m_voice[i].count = 0;
				m_voice[i].val = !m_voice[i].val;
			}
		}

		// Then mix channels according to m_ctrl
		if (BIT(m_ctrl, 1))
		{
			// ch0 and ch1
			if (BIT(m_ctrl, 0))
			{
				const int xor01 = BIT(m_voice[0].val ^ m_voice[1].val, 0);
				const int xor12 = BIT(m_voice[1].val ^ m_voice[2].val, 0);
				sum += xor01 * volumes[0];
				sum += xor12 * volumes[1];
			}
			else
			{
				sum += m_voice[0].val * volumes[0];
				sum += m_voice[1].val * volumes[1];
			}

			// ch3 is unaffected by m_ctrl bit 1
			sum += m_voice[2].val * volumes[2];
		}

		stream.put_int(0, index, sum, 32768);
	}
}


// PV-1000 System

class pv1000_state : public driver_device
{
public:
	pv1000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sound(*this, "pv1000_sound"),
		m_cart(*this, "cartslot"),
		m_videoram(*this, "videoram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_joysticks(*this, "IN%u", 0)
	{ }

	void pv1000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void io_w(offs_t offset, uint8_t data);
	uint8_t io_r(offs_t offset);
	void gfxram_w(offs_t offset, uint8_t data);
	uint8_t joystick_r();
	uint8_t m_io_regs[8]{};

	emu_timer *m_irq_on_timer = nullptr;
	emu_timer *m_busrq_on_timer = nullptr;
	emu_timer *m_busrq_off_timer = nullptr;
	uint8_t m_irq_enabled = 0;
	uint8_t m_irq_active = 0;
	uint8_t m_pcg_bank = 0;
	uint8_t m_force_pattern = 0;
	uint8_t m_border_col = 0;
	uint8_t m_render_disable = 0;

	uint8_t * m_gfxram = nullptr;
	void pv1000_postload();

	required_device<cpu_device> m_maincpu;
	required_device<pv1000_sound_device> m_sound;
	required_device<generic_slot_device> m_cart;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_ioport_array<4> m_joysticks;

	uint32_t screen_update_pv1000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(d65010_irq_on_cb);
	TIMER_CALLBACK_MEMBER(d65010_busrq_on_cb);
	TIMER_CALLBACK_MEMBER(d65010_busrq_off_cb);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void pv1000_mem(address_map &map) ATTR_COLD;
	void pv1000_io(address_map &map) ATTR_COLD;
};


void pv1000_state::pv1000_mem(address_map &map)
{
	//map(0x0000, 0x7fff) // mapped by the cartslot
	map(0xb800, 0xbbff).ram().share(m_videoram);
	map(0xbc00, 0xbfff).ram().w(FUNC(pv1000_state::gfxram_w)).region("gfxram", 0);
}


void pv1000_state::pv1000_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xf8, 0xff).rw(FUNC(pv1000_state::io_r), FUNC(pv1000_state::io_w));
}


void pv1000_state::gfxram_w(offs_t offset, uint8_t data)
{
	m_gfxram[offset] = data;
	m_gfxdecode->gfx(1)->mark_dirty(offset / 32);
}


void pv1000_state::io_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		//logerror("io_w offset=%02x, data=%02x (%03d)\n", offset, data , data);
		m_sound->voice_w(offset, data);
		break;

	case 0x04:
		/* Bit 1 = Matrix IRQ enabled    *
		 * Bit 0 = Prerender IRQ enabled */
		m_irq_enabled = data & 3;
		m_irq_active &= m_irq_enabled;
		if (m_irq_active == 0)
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

		break;

	case 0x05:
		// Acknowledge Prerender IRQ
		if (m_irq_active & 1)
		{
			m_irq_active &= ~1;
			if (m_irq_active == 0)
				m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
		}
		break;

	//case 0x06 VRAM + PCG location, always fixed at 0xb8xx

	case 0x07:
		/* ---- -xxx unknown, border color? */
		m_pcg_bank = (data & 0xe0) >> 5;
		m_force_pattern = ((data & 0x10) >> 4); /* Dig Dug relies on this */
		m_render_disable = ((data & 0x08) >> 3);
		if (m_render_disable == 0) // If we're enabling rendering mid-scanline...
		{
			int hpos = m_screen->hpos(); // set_raw configured so that BUSREQ is asserted from 0 to 248
			int vpos = m_screen->vpos();
			if (hpos < 248 && vpos >= 26 && vpos < 192+26)
			{
				m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, ASSERT_LINE);
				// The de-assertion is always automatically scheduled
			}
		}

		m_border_col = data & 7;
		break;
	}

	m_io_regs[offset] = data;
}


uint8_t pv1000_state::io_r(offs_t offset)
{
	/* Real hardware always reads MSbit set from the two readable registers;
	   lidnariq suspects D7 is input-only and the 128s bit is high due to the
	   weak pull-ups on the data bus */
	switch (offset)
	{
		case 0x04: // port $FC returns player 2 joystick and interrupt status
			return 0x80 | (joystick_r() & 0x0c) | (m_irq_active & 3); // Bit 1 = Matrix IRQ, Bit 0 = Prerender IRQ

		case 0x05: // port $FD returns both joysticks and acknowledges matrix scan IRQ
			if (!machine().side_effects_disabled() && (m_irq_active & 2))
			{
				m_irq_active &= ~2;
				if (m_irq_active == 0)
					m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
			}
			return 0x80 | joystick_r();

		default:
			/* Ports $F8-$FB, $FE, and $FF are undriven, and pulled high by the
			   resistors next to the Z80 */
			return 0xff;
	}
}

uint8_t pv1000_state::joystick_r()
{
	uint8_t data = 0;

	for (int i = 0; i < 4; i++)
		if (BIT(m_io_regs[5], i))
			data |= m_joysticks[i]->read();

	return data & 0x0f;
}


static INPUT_PORTS_START( pv1000 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


DEVICE_IMAGE_LOAD_MEMBER(pv1000_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	if (size != 0x2000 && size != 0x4000 && size != 0x8000)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be 8K, 16K or 32K)");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}


uint32_t pv1000_state::screen_update_pv1000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_border_col); // border is on top and bottom

	if (m_render_disable)
		return 0;

	for (int y = 0; y < 24; y++)
	{
		for (int x = 2; x < 30; x++) // left-right most columns never even drawn, black instead
		{
			uint16_t tile = m_videoram[y * 32 + x];

			if (tile < 0xe0 || m_force_pattern)
			{
				tile += (m_pcg_bank << 8);
				//When we adjusted timing in set_raw so that BUSREQ is asserted during a clean rectangle, we need to compensate for that here
				m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, tile, 0, 0, 0, x*8+16, y*8+26);
			}
			else
			{
				tile -= 0xe0;
				m_gfxdecode->gfx(1)->opaque(bitmap,cliprect, tile, 0, 0, 0, x*8+16, y*8+26);
			}
		}
	}

	return 0;
}



/* Interrupt is triggering 16 times during vblank. */
/* They are spaced every 4 scanlines, with equal padding before and after */
TIMER_CALLBACK_MEMBER(pv1000_state::d65010_irq_on_cb)
{
	int vpos = m_screen->vpos();
	int next_vpos = vpos + 4;

	m_irq_active |= 2 & m_irq_enabled;
	if (vpos == 20)
	{
		m_irq_active |= 1 & m_irq_enabled;
	}

	if (m_irq_active)
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	}

	/* Schedule next IRQ trigger

	   These numbers are an artifact of defining Y=0 as the
	   top scanline of the non-blanking display. */
	if (vpos >= 258)
	{
		next_vpos = 0; // 262=0, 4, 8, 12, 16, 20
	}
	else if (vpos >= 20 && vpos < 222)
	{
		next_vpos = 222; // 226, 230, 234, 238, 242, 246, 250, 254, 258
	}
	m_irq_on_timer->adjust(m_screen->time_until_pos(next_vpos, 224+32));
}

/* Add BUSACK-triggered scanline renderer */
TIMER_CALLBACK_MEMBER(pv1000_state::d65010_busrq_on_cb)
{
	int vpos = m_screen->vpos();
	int next_vpos = vpos + 1;

	if (m_render_disable == 0)
	{
		m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, ASSERT_LINE);
	}

	// schedule the de-assertion of Busreq that corresponds to the current assertion
	m_busrq_off_timer->adjust(m_screen->time_until_pos(vpos, 248));

	if (vpos >= 192 + 26)
	{
		next_vpos = 26;
	}
	m_busrq_on_timer->adjust(m_screen->time_until_pos(next_vpos, 0));
}

TIMER_CALLBACK_MEMBER(pv1000_state::d65010_busrq_off_cb)
{
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, CLEAR_LINE);
}

void pv1000_state::pv1000_postload()
{
	// restore GFX ram
	for (int i = 0; i < 0x400; i++)
		gfxram_w(i, m_gfxram[i]);
}

void pv1000_state::machine_start()
{
	m_irq_on_timer = timer_alloc(FUNC(pv1000_state::d65010_irq_on_cb), this);
	m_busrq_on_timer = timer_alloc(FUNC(pv1000_state::d65010_busrq_on_cb), this);
	m_busrq_off_timer = timer_alloc(FUNC(pv1000_state::d65010_busrq_off_cb), this);

	m_gfxram = memregion("gfxram")->base();
	save_pointer(NAME(m_gfxram), 0x400);

	if (m_cart->exists())
	{
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x7fff, read8sm_delegate(*m_cart, FUNC(generic_slot_device::read_rom)));

		// FIXME: this is needed for gfx decoding, but there is probably a cleaner solution!
		std::string region_tag;
		memcpy(memregion("gfxrom")->base(), memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str())->base(), m_cart->get_rom_size());
	}

	save_item(NAME(m_io_regs));
	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_irq_active));
	save_item(NAME(m_pcg_bank));
	save_item(NAME(m_force_pattern));
	save_item(NAME(m_border_col));
	save_item(NAME(m_render_disable));

	machine().save().register_postload(save_prepost_delegate(FUNC(pv1000_state::pv1000_postload), this));
}


void pv1000_state::machine_reset()
{
	m_io_regs[5] = 0;
	m_irq_on_timer->adjust(m_screen->time_until_pos(222, 256));
	m_busrq_on_timer->adjust(m_screen->time_until_pos(26, 0));
	m_busrq_off_timer->adjust(m_screen->time_until_pos(26, 248));
}


static const gfx_layout pv1000_3bpp_gfx =
{
	8, 8,           /* 8x8 characters */
	RGN_FRAC(1,1),
	3,
	{ 16*8, 8*8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8*4
};


static GFXDECODE_START( gfx_pv1000 )
	GFXDECODE_ENTRY( "gfxrom", 8, pv1000_3bpp_gfx, 0, 8 )
	GFXDECODE_ENTRY( "gfxram", 8, pv1000_3bpp_gfx, 0, 8 )
GFXDECODE_END


void pv1000_state::pv1000(machine_config &config)
{
	Z80(config, m_maincpu, 17897725/5);
	m_maincpu->set_addrmap(AS_PROGRAM, &pv1000_state::pv1000_mem);
	m_maincpu->set_addrmap(AS_IO, &pv1000_state::pv1000_io);

	/* D65010G031 - Video & sound chip */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(17897725/4, 288, 32, 32+224, 262, 0, 244);
	// Pixel aspect is 48/35.
	// Display aspect is MAME's 4:3 default.

	// Note that this value is overridden by the user's pv1000.cfg, if present.
	// 206px x 48/35(PAR) / 4/3(DAR) = 212sl
	m_screen->set_default_position(
			216/206.0, 0, //216 px in storage aspect; cropped to 206 px
			244/212.0, 0); //244 sl in storage aspect; cropped to 212 sl
	m_screen->set_screen_update(FUNC(pv1000_state::screen_update_pv1000));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::RGB_3BIT);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pv1000);

	SPEAKER(config, "mono").front_center();
	PV1000(config, m_sound, 17897725).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* Cartridge slot */
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_linear_slot, "pv1000_cart"));
	cartslot.set_must_be_loaded(true);
	cartslot.set_device_load(FUNC(pv1000_state::cart_load));

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("pv1000");
}


ROM_START( pv1000 )
	ROM_REGION( 0x8000, "gfxrom", ROMREGION_ERASE00 )
	ROM_REGION( 0x400, "gfxram", ROMREGION_ERASE00 )
ROM_END


/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME    FLAGS */
CONS( 1983, pv1000, 0,      0,      pv1000,  pv1000, pv1000_state, empty_init, "Casio", "PV-1000",  MACHINE_SUPPORTS_SAVE )
