// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood, Tomasz Slanina
/******************************************************************************************

Korean Tetris Arcade game
(blantant rip-off of the Mirrorsoft/Andromeda Software Tetris PC-XT)

original tetriunk.c by David Haywood & Tomasz Slanina


TODO:
- 02851: tetriskr: Corrupt game graphics after some time of gameplay, caused by a wrong
  reading of the i/o $3c8 bit 1. (seems fixed?)
- tetriskr can store inputs read during the timer irq. If ds is 0x40 when the irq is taken
  it will corrupt the BIOS data area which can lead to corrupt graphics
- DC offsetted buzzer sound;

******************************************************************************************/

#include "emu.h"
#include "bus/isa/cga.h"
#include "cpu/i86/i86.h"
#include "machine/genpc.h"

class isa8_cga_tetriskr_device : public isa8_cga_superimpose_device
{
public:
	// construction/destruction
	isa8_cga_tetriskr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	uint8_t bg_bank_r();
	void bg_bank_w(uint8_t data);
private:
	required_region_ptr<uint8_t> m_bg;
	uint8_t m_bg_bank = 0;
};


/* for superimposing CGA over a different source video (i.e. tetriskr) */
DEFINE_DEVICE_TYPE(ISA8_CGA_TETRISKR, isa8_cga_tetriskr_device, "tetriskr_cga", "ISA8_CGA_TETRISKR")

//-------------------------------------------------
//  isa8_cga_tetriskr_device - constructor
//-------------------------------------------------

isa8_cga_tetriskr_device::isa8_cga_tetriskr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_cga_superimpose_device(mconfig, ISA8_CGA_TETRISKR, tag, owner, clock),
	m_bg(*this, "gfx2")
{
}


void isa8_cga_tetriskr_device::device_start()
{
	m_bg_bank = 0;
	isa8_cga_superimpose_device::device_start();
	m_isa->install_device(0x3c0, 0x3c0, read8smo_delegate(*this, FUNC(isa8_cga_tetriskr_device::bg_bank_r)), write8smo_delegate(*this, FUNC(isa8_cga_tetriskr_device::bg_bank_w)));
}

void isa8_cga_tetriskr_device::bg_bank_w(uint8_t data)
{
	m_bg_bank = (data & 0x0f) ^ 8;
}

uint8_t isa8_cga_tetriskr_device::bg_bank_r()
{
	return 0xff;
}


uint32_t isa8_cga_tetriskr_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	//popmessage("%04x",m_start_offs);

	bitmap.fill(rgb_t::black(), cliprect);

	for(int y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		int yi = y % 8;
		int yj = y / 8;
		for(int x=cliprect.min_x;x<=cliprect.max_x;x++)
		{
			int xi = x % 8;
			int xj = x / 8;
			uint8_t color = 0;
			/* TODO: first byte seems bogus? */
			for(int pen_i = 0;pen_i<4;pen_i++)
				color |= ((m_bg[yj*320/8+xj+(pen_i*0x20000)+yi*0x400+m_bg_bank*0x2000+1] >> (7-xi)) & 1) << pen_i;

			bitmap.pix(y, x) = m_palette->pen(color);
		}
	}

	isa8_cga_device::screen_update(screen, bitmap, cliprect);
	return 0;
}

ROM_START( tetriskr_cga )
	ROM_REGION( 0x2000, "gfx1",ROMREGION_ERASE00 ) /* gfx - 1bpp font*/
	ROM_LOAD( "b-3.u36", 0x1800, 0x0800, CRC(1a636f9a) SHA1(a356cc57914d0c9b9127670b55d1f340e64b1ac9) )
	ROM_IGNORE( 0x1800 )

	ROM_REGION( 0x80000+1, "gfx2",ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "b-1.u59", 0x00000, 0x10000, CRC(4719d986) SHA1(6e0499944b968d96fbbfa3ead6237d69c769d634))
	ROM_LOAD( "b-2.u58", 0x10000, 0x10000, CRC(599e1154) SHA1(14d99f90b4fedeab0ac24ffa9b1fd9ad0f0ba699))
	ROM_LOAD( "b-4.u54", 0x20000, 0x10000, CRC(e112c450) SHA1(dfdecfc6bd617ec520b7563b7caf44b79d498bd3))
	ROM_LOAD( "b-5.u53", 0x30000, 0x10000, CRC(050b7650) SHA1(5981dda4ed43b6e81fbe48bfba90a8775d5ecddf))
	ROM_LOAD( "b-6.u49", 0x40000, 0x10000, CRC(d596ceb0) SHA1(8c82fb638688971ef11159a6b240253e63f0949d))
	ROM_LOAD( "b-7.u48", 0x50000, 0x10000, CRC(79336b6c) SHA1(7a95875f3071bdc3ee25c0e6a5a3c00ef02dc977))
	ROM_LOAD( "b-8.u44", 0x60000, 0x10000, CRC(1f82121a) SHA1(106da0f39f1260d0761217ed0a24c1611bfd7f05))
	ROM_LOAD( "b-9.u43", 0x70000, 0x10000, CRC(4ea22349) SHA1(14dfd3dbd51f8bd6f3290293b8ea1c165e8cf7fd))
ROM_END

const tiny_rom_entry *isa8_cga_tetriskr_device::device_rom_region() const
{
	return ROM_NAME( tetriskr_cga );
}


namespace {

class tetriskr_state : public driver_device
{
public:
	tetriskr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mb(*this, "mb")
	{ }

	void tetriskr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sample_tick);

private:
	uint8_t m_port_b_data;

	uint8_t port_a_r();
	uint8_t port_b_r();
	uint8_t port_c_r();
	void port_b_w(uint8_t data);

	required_device<cpu_device> m_maincpu;
	required_device<pc_noppi_mb_device> m_mb;
	void tetriskr_io(address_map &map) ATTR_COLD;
	void tetriskr_map(address_map &map) ATTR_COLD;
};

uint8_t tetriskr_state::port_a_r()
{
	//harmless keyboard error occurs without this
	return 0xaa;
}

uint8_t tetriskr_state::port_b_r()
{
	return m_port_b_data;
}

uint8_t tetriskr_state::port_c_r()
{
	return 0x00;// DIPS?
}

void tetriskr_state::port_b_w(uint8_t data)
{
	m_mb->m_pit8253->write_gate2(BIT(data, 0));
	m_mb->pc_speaker_set_spkrdata(BIT(data, 1));
	m_port_b_data = data;
//  m_cvsd->digit_w(data);
}

void tetriskr_state::tetriskr_map(address_map &map)
{
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void tetriskr_state::tetriskr_io(address_map &map)
{
	map.global_mask(0x3ff);
	map(0x0000, 0x00ff).m(m_mb, FUNC(pc_noppi_mb_device::map));
	map(0x0060, 0x0060).r(FUNC(tetriskr_state::port_a_r));  //not a real 8255
	map(0x0061, 0x0061).rw(FUNC(tetriskr_state::port_b_r), FUNC(tetriskr_state::port_b_w));
	map(0x0062, 0x0062).r(FUNC(tetriskr_state::port_c_r));
	map(0x03c8, 0x03c8).portr("IN0");
	map(0x03c9, 0x03c9).portr("IN1");
//  map(0x03ce, 0x03ce).portr("IN1"); //read then discarded?
}

static INPUT_PORTS_START( tetriskr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) //probably unused
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_START("IN1") //dip-switches
	PORT_DIPNAME( 0x03, 0x03, "Starting Level" )
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, "Starting Bomb" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) ) duplicate
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START( "pcvideo_cga_config" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void tetriskr_state::machine_start()
{
}

void tetriskr_state::machine_reset()
{
}

static void tetriskr_isa8_cards(device_slot_interface &device)
{
	device.option_add_internal("tetriskr", ISA8_CGA_TETRISKR);
}


void tetriskr_state::tetriskr(machine_config &config)
{
	I8088(config, m_maincpu, XTAL(14'318'181)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &tetriskr_state::tetriskr_map);
	m_maincpu->set_addrmap(AS_IO, &tetriskr_state::tetriskr_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	PCNOPPI_MOTHERBOARD(config, m_mb, 0).set_cputag(m_maincpu);
	m_mb->int_callback().set_inputline(m_maincpu, 0);
	m_mb->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	ISA8_SLOT(config, "isa1", 0, "mb:isa", tetriskr_isa8_cards, "tetriskr", true); // FIXME: determine ISA bus clock

	RAM(config, RAM_TAG).set_default_size("64K");
}

ROM_START( tetriskr )
	ROM_REGION( 0x10000, "bios", 0 ) /* code */
	ROM_LOAD( "b-10.u10", 0x0000, 0x10000, CRC(efc2a0f6) SHA1(5f0f1e90237bee9b78184035a32055b059a91eb3) )
	ROM_FILL( 0x1bdb, 1, 0xba ) // patch to work around input bug mentioned above
	ROM_FILL( 0x1bdc, 1, 0x00 )
	ROM_FILL( 0x1bdd, 1, 0x01 )
	ROM_FILL( 0x1bde, 1, 0x8e )
	ROM_FILL( 0x1bdf, 1, 0xda )
ROM_END

} // anonymous namespace


GAME( 1988?,tetriskr, 0, tetriskr, tetriskr, tetriskr_state, empty_init, ROT0,  "bootleg",    "Tetris (Korean bootleg of Mirrorsoft PC-XT Tetris)", MACHINE_IMPERFECT_SOUND )
