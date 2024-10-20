// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Tomasz Slanina
/**************************************************************************************

Space Guerrilla PCB Layout
--------------------------

All PCBs slot into a metal frame. Each PCB is joined by the 14way/28way edge connectors
on one side, and the 50 pin connectors via flat cables on the other side.

                       14-way
OEC-5            |----------------|
|----------------|----------------|-----------------|
|      1       2       3       4       5      6     |
|A   74273   7417             555                   |
|                                                   |
|B   74273   7417     555     7493    76477  LM380  |
|                                                   |
|C   DIP16   74121    4066    7400           4030   |
|                                                   |
|D   LM324   LM324    LM3900  LM3900  458    4006   |
|                                                   |
|                                                   |
|                                                   |
|                                                   |
| VR1   VR2   VR3   VR4   VR5   VR6   VR7    VOL    |
|---------------------------------------------------|

Notes:
      DIP16 - Joins to DIP16 socket at 3F on IOC2-A PCB via a 16-wire flat cable.
      VR1-7 - Potentiometers for volume of sounds
      VOL   - Master Volume
      Note PCB also contains a lot of resistors, transistors, capacitors etc



                            28-way
IOC2-A           |-----------------------------|
|----------------|-----------------------------|----------------|
|      1         2       3       4       5      6      7        |
|A     DIP16   DIP16   DIP16   DIP16                            |
|                                                           VR1 |
|B     D1-8    7404    7404    7404    LM324          4066      |
|                                                               |
|C     D9-16   74240   7406    DIP8           LM324         VR2 |
|                                                               |
|D     74240   74240   74175   75472   555           555        |
|                                                               |
|E     DSW1    74240   74273   7417                         VR3 |
|                                                               |
|F     DIP16   DIP16   DIP16   555     458    LM3900   LM3900   |
|                                                               |
|G     74244   74244   7432    555                          VR4 |
|                                                               |
|H                                                              |
|                                                               |
|                                                               |
|  |-------------|                                              |
|--|-------------|----------------------------------------------|
      50 pin

Notes:
      DIP8 - Unpopulated position for DIP8 IC
      DIP16- Unpopulated position for DIP16 IC
      D1-16- Diodes
      VR1-4- Potentiometers for volume of sounds



                            28-way
CRTC1-A          |-----------------------------|
|----------------|-----------------------------|----------------|
|   1      2      3      4      5      6      7      8      9   |
|A 20MHz  7455   Diodes 1-4   DIP16  DIP16  DIP16  4027   4027  |
|                      &                                        |
|B 74163  7404   Transtrs 1-7 DIP16  DIP16  DIP16  4027   4027  |
|                                                               |
|C 7486   74161  7404         DIP16  DIP16  DIP16  4027   4027  |
|                                                               |
|D 7486   74161  7408      82S09     74374  74157  4027   4027  |
|                                                               |
|E 7486   74161  7402  7411   74157  74377  7408   4027   4027  |
|                                                               |
|F 7486   74161  7411  74157  74244  74377  7400   74153  74157 |
|                                                               |
|G 74107  7404   7474  74175  74107  74157  7404   74377  74157 |
|                                                               |
|H 7404   7404   74244 74139  7404   7427   7474   74164  7404  |
|                                                               |
|                                                               |
|  |-------------|       |-------------|       |-------------|  |
|--|-------------|-------|-------------|-------|-------------|--|
      50 pin                 50 pin                50 pin

Notes:
      4027 - MOSTEK MK4027-4 RAM (DIP16)
      DIP16- Unpopulated position for DIP16 IC



                            28-way
CRTC2-A          |-----------------------------|
|----------------|-----------------------------|----------------|
|   1      2      3      4      5      6      7      8      9   |
|A 7402  74377  74157  7400   74194  74S288        4027   4027  |
|                                                               |
|B 7486  74153  7486   7400   74194  74S288        4027   4027  |
|                                                               |
|C 7408  74374  74158         74273  7427          4027   4027  |
|                                                               |
|D 7400  74244  25S10  74181  7430   7408          4027   4027  |
|                                                               |
|E 7400  74153  25S10  74181         7402          4027   4027  |
|                                                               |
|F 7408  74153         7411   7404   7404          4027   4027  |
|                                                               |
|G 7404  74153  DIP16  74153  74153  74283         4027   4027  |
|                                                               |
|H 74138 74244  7474   74153  74153  74157         4027   4027  |
|                                                               |
|                                                               |
|  |-------------|       |-------------|       |-------------|  |
|--|-------------|-------|-------------|-------|-------------|--|
      50 pin                 50 pin                50 pin

Notes:
      74S288 - 32bytes x8 Bipolar PROM (DIP16, both PROMs contain identical contents)
      4027   - MOSTEK MK4027-4 RAM (DIP16)



                            28-way
SBC-A            |-----------------------------|
|----------------|-----------------------------|----------------|
|   1      2      3      4      5      6      7      8      9   |
|A 74161        DIP18  4045   4045                 ROM2   ROM1  |
|                                                               |
|B 7404  7402   74139  DIP18  DIP18                ROM4   ROM3  |
|                                                               |
|C 7404  7404   7420   4045   4045                 ROM6   ROM5  |
|                                                               |
|D              7420   DIP18  DIP18                ROM8   ROM7  |
|                                                               |
|E 74241 74241         8216   8216                 DIP24  DIP24 |
|                                                               |
|F    Z80              74241                       DIP24  DIP24 |
|                                                               |
|G       7407          74241                       ROM14  ROM13 |
|                                                               |
|H                     74241  7442   7442   74244  ROM16  ROM15 |
|                                                               |
|                                                               |
|  |-------------|       |-------------|                        |
|--|-------------|-------|-------------|------------------------|
      50 pin                 50 pin

Notes:
      4045 - TMS4045-45NL RAM (DIP18)
      8216 - Mitsubishi M5L8216 RAM (DIP16)
      DIP18- Unpopulated position for DIP18 IC
      DIP24- Unpopulated position for DIP24 IC
      All ROMs type 2708 (DIP24)

**************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/samples.h"
#include "sound/sn76477.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

/*************************************
 *
 *  Driver data
 *
 *************************************/

class spaceg_state : public driver_device
{
public:
	spaceg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_io9400(*this, "io9400"),
		m_io9401(*this, "io9401"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_samples(*this, "samples"),
		m_sn(*this, "snsnd"),
		m_sound1(0),
		m_sound2(0),
		m_sound3(0)
	{ }

	void spaceg(machine_config &config);

protected:
	virtual void driver_start() override;

private:
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_io9400;
	required_shared_ptr<uint8_t> m_io9401;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<samples_device> m_samples;
	required_device<sn76477_device> m_sn;

	uint8_t m_sound1;
	uint8_t m_sound2;
	uint8_t m_sound3;

	void zvideoram_w(offs_t offset, uint8_t data);
	uint8_t colorram_r(offs_t offset);
	void sound1_w(uint8_t data);
	void sound2_w(uint8_t data);
	void sound3_w(uint8_t data);

	void spaceg_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void spaceg_map(address_map &map) ATTR_COLD;
};

void spaceg_state::driver_start()
{
	save_item(NAME(m_sound1));
	save_item(NAME(m_sound2));
	save_item(NAME(m_sound3));
}

/*************************************
 *
 *  Video emulation
 *
 *************************************/

void spaceg_state::spaceg_palette(palette_device &palette) const
{
	for (int i = 0; i < 128; i++)
		palette.set_pen_color(i, rgb_t(0x00, 0x00, 0x00));

	// proms are currently undumped...
	palette.set_pen_color( 0, rgb_t(0x00, 0x00, 0x00)); //ok czarny
	palette.set_pen_color( 1, rgb_t(0x7f, 0x00, 0x00));//???
	palette.set_pen_color( 2, rgb_t(0xff, 0xff, 0xff)); //ok+ bialy
	palette.set_pen_color( 3, rgb_t(0xff, 0x00, 0x00)); //ok j.czerw.
	palette.set_pen_color( 4, rgb_t(0x3f, 0x3f, 0xff)); //ok j.niebieski
	palette.set_pen_color( 5, rgb_t(0x3f, 0xff, 0x3f)); //ok j.zielony
	palette.set_pen_color( 6, rgb_t(0xff, 0xbf, 0xbf)); //ok+ 'majtki'
	palette.set_pen_color( 7, rgb_t(0xff, 0xff, 0x00)); //ok+ zolty

	palette.set_pen_color( 8, rgb_t(0xff, 0x7f, 0x00)); //ok+ pomaranczowy
	palette.set_pen_color( 9, rgb_t(0x3f, 0xbf, 0xff)); //ok j.niebieski (ciemniejszy od 13)
	palette.set_pen_color(10, rgb_t(0x3f, 0xbf, 0x3f));    //ok+ c.zielony
	palette.set_pen_color(11, rgb_t(0x00, 0xff, 0x00));    //ok j.zielony
	palette.set_pen_color(12, rgb_t(0x7f, 0x00, 0x00));    //ok brazowy (c.czerw)
	palette.set_pen_color(13, rgb_t(0x7f, 0xbf, 0xff));    //ok j.niebieski (jasniejszy od 9)
	palette.set_pen_color(14, rgb_t(0x00, 0xff, 0xff));//???
	palette.set_pen_color(15, rgb_t(0x7f, 0x7f, 0x7f));//???
}

void spaceg_state::zvideoram_w(offs_t offset, uint8_t data)
{
	int col = m_colorram[0x400];
	int xoff = *m_io9400 >> 5 & 7;
	uint16_t offset2 = (offset + 0x100) & 0x1fff;
	uint16_t sdata = data << (8 - xoff);
	uint16_t vram_data = m_videoram[offset] << 8 | (m_videoram[offset2]);

	if (col > 0x0f) popmessage("color > 0x0f = %2d", col);
	col &= 0x0f;

	switch (*m_io9401)
	{
		// draw
		case 0:
			vram_data &= ~(0xff00 >> xoff);
			[[fallthrough]];
		case 1:
			vram_data |= sdata;

			// update colorram
			if (sdata&0xff00) m_colorram[offset] = col;
			if (sdata&0x00ff) m_colorram[offset2] = col;
			break;

		// erase
		case 0xd:
			vram_data &= ~sdata;
			break;

		default:
			logerror("mode = %02x pc = %04x\n", *m_io9401, m_maincpu->pc());
			popmessage("mode = %02x pc = %04x\n", *m_io9401, m_maincpu->pc());
			return;
	}

	m_videoram[offset]=vram_data>>8;
	m_videoram[offset2]=vram_data&0xff;
}


uint8_t spaceg_state::colorram_r(offs_t offset)
{
	int rgbcolor;

	if (offset < 0x400)
	{
		rgbcolor = (m_colorram[offset] << 1) | ((offset &0x100) >> 8);

		if ((offset >= 0x200) && (offset < 0x220)) /* 0xa200- 0xa21f */
		{
			/* palette 1 */
			int col_ind = offset & 0x1f;
			m_palette->set_pen_color(0x10 + 0x00 + col_ind, pal3bit(rgbcolor >> 0), pal3bit(rgbcolor >> 6), pal3bit(rgbcolor >> 3));
		}
		else if ((offset >= 0x300) && (offset < 0x320)) /* 0xa300- 0xa31f */
		{
			/* palette 2 */
			int col_ind = offset & 0x1f;
			m_palette->set_pen_color(0x10 + 0x00 + col_ind, pal3bit(rgbcolor >> 0), pal3bit(rgbcolor >> 6), pal3bit(rgbcolor >> 3));
		}
		else
			logerror("palette? read from colorram offset = %04x\n",offset);
	}

	if (*m_io9401 != 0x40)
		logerror("colorram read in mode: 9401 = %02x (offset = %04x)\n", *m_io9401, offset);

	return m_colorram[offset];
}


uint32_t spaceg_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < 0x2000; offs++)
	{
		uint8_t data = m_videoram[offs];
		int y = offs & 0xff;
		int x = (offs >> 8) << 3;

		for (int i = 0; i < 8; i++)
		{
			bitmap.pix(y, x) = (data & 0x80) ? m_colorram[offs] : 0;

			x++;
			data <<= 1;
		}
	}

	return 0;
}

static const char *const invaders_sample_names[] =
{
	"*invaders",
	"1",        /* shot/missle */
	"2",        /* base hit/explosion */
	"3",        /* invader hit */
	"4",        /* fleet move 1 */
	"5",        /* fleet move 2 */
	"6",        /* fleet move 3 */
	"7",        /* fleet move 4 */
	"8",        /* UFO/saucer hit */
	"9",        /* bonus base */
	nullptr
};

void spaceg_state::sound1_w(uint8_t data)
{
	if (!BIT(m_sound1, 1) && BIT(data, 1))
		m_samples->start(1, 1); // Death

	if (!BIT(m_sound1, 2) && BIT(data, 2))
		m_samples->start(0, 0); // Shoot

	m_sn->enable_w(!(data & 0x08)); // Boss

	m_sound1 = data;

	if (data & ~0x0e) logerror("spaceg sound3 unmapped %02x\n", data & ~0x0e);
}

void spaceg_state::sound2_w(uint8_t data)
{
	// game writes 0x01 at bootup & 0x11 when you start a game
	m_sound2 = data;

	if (data & ~0x11) logerror("spaceg sound2 unmapped %02x\n", data & ~0x11);
}

void spaceg_state::sound3_w(uint8_t data)
{
	if (!BIT(m_sound3, 0) && BIT(data, 0))
		m_samples->start(4, 8); // Start of level

	if (!BIT(m_sound3, 1) && BIT(data, 1))
		m_samples->start(5, 8); // Rocket

	if (!BIT(m_sound3, 2) && BIT(data, 2))
		m_samples->start(2, 2); // Hit

	if (!BIT(m_sound3, 3) && BIT(data, 3))
		m_samples->start(3, 7); // Dive bomb

	m_sound3 = data;

	if (data & ~0x0f) logerror("spaceg sound3 unmapped %02x\n", data & ~0x0f);
}

/*************************************
 *
 *  Memory maps
 *
 *************************************/

void spaceg_state::spaceg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x3000, 0x3fff).rom();
	map(0x7000, 0x77ff).ram();

	map(0xa000, 0xbfff).ram().r(FUNC(spaceg_state::colorram_r)).share("colorram");
	map(0xc000, 0xdfff).ram().w(FUNC(spaceg_state::zvideoram_w)).share("videoram");

	map(0x9400, 0x9400).writeonly().share("io9400"); /* gfx ctrl */
	map(0x9401, 0x9401).writeonly().share("io9401"); /* gfx ctrl */
	/* 9402 -
	    bits 0 and 1 probably control the lamps under the player 1 and player 2 start buttons
	    bit 2 - unknown -
	    bit 3 is probably a flip screen
	    bit 7 - unknown - set to 1 during the gameplay (coinlock ?)
	*/
	map(0x9402, 0x9402).nopw();
	map(0x9405, 0x9405).w(FUNC(spaceg_state::sound1_w));
	map(0x9406, 0x9406).w(FUNC(spaceg_state::sound2_w));
	map(0x9407, 0x9407).w(FUNC(spaceg_state::sound3_w));

	map(0x9800, 0x9800).portr("9800");
	map(0x9801, 0x9801).portr("9801");
	map(0x9802, 0x9802).portr("9802");
	map(0x9805, 0x9805).portr("9805");
	map(0x9806, 0x9806).portr("9806");
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( spaceg )
	PORT_START("9800")
	PORT_DIPUNUSED(  0x01, IP_ACTIVE_HIGH )                 /* was related to coinage */
	PORT_DIPUNKNOWN( 0x02, IP_ACTIVE_HIGH )                 /* check code at 0x127d - when bases are supposed to disappear */
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x20, "30000" )
	PORT_DIPSETTING(    0x10, "40000" )
	PORT_DIPSETTING(    0x30, "50000" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Unknown ) )          /* set bit 2 of 0x9402 depending on score - previously 2nd bonus life ? */
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x80, "30000" )
	PORT_DIPSETTING(    0x40, "40000" )
	PORT_DIPSETTING(    0xc0, "50000" )

	PORT_START("9801")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("9802")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("9805")    /* player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)

	PORT_START("9806")    /* player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
INPUT_PORTS_END


/*************************************
 *
 *  Machine config
 *
 *************************************/

void spaceg_state::spaceg(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2500000);         /* 2.5 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &spaceg_state::spaceg_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(256, 256);
	screen.set_visarea(0, 255, 32, 255);
	screen.set_screen_update(FUNC(spaceg_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline("maincpu", INPUT_LINE_NMI); // 60 Hz NMIs (verified)

	PALETTE(config, m_palette, FUNC(spaceg_state::spaceg_palette), 16+128-16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	// HACK: SN76477 parameters copied from space invaders
	SN76477(config, m_sn);
	m_sn->set_noise_params(0, 0, 0);
	m_sn->set_decay_res(0);
	m_sn->set_attack_params(0, RES_K(100));
	m_sn->set_amp_res(RES_K(56));
	m_sn->set_feedback_res(RES_K(10));
	m_sn->set_vco_params(0, CAP_U(0.1), RES_K(8.2));
	m_sn->set_pitch_voltage(5.0);
	m_sn->set_slf_params(CAP_U(1.0), RES_K(120));
	m_sn->set_oneshot_params(0, 0);
	m_sn->set_vco_mode(1);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(1, 0);
	m_sn->set_enable(1);
	m_sn->add_route(ALL_OUTPUTS, "mono", 0.5);

	// HACK: samples copied from space invaders
	SAMPLES(config, m_samples);
	m_samples->set_channels(6);
	m_samples->set_samples_names(invaders_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( spaceg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.9a",  0x0000, 0x0400, CRC(d6f80b0e) SHA1(503e832c065bb302ec60ed239c4d99a605cb931a) )
	ROM_LOAD( "2.8a",  0x0400, 0x0400, CRC(708b8eec) SHA1(70f9b1506a653985d9d3eacba220f0a4eb241318) )
	ROM_LOAD( "3.9b",  0x0800, 0x0400, CRC(7f0b5cb4) SHA1(97a7125d046e7191b5d3be8f203d1bbb6f988209) )
	ROM_LOAD( "4.8b",  0x0c00, 0x0400, CRC(8b40a154) SHA1(f42bdd8e94090fc5eae58e833b8443300d3ce991) )
	ROM_LOAD( "5.9c",  0x1000, 0x0400, CRC(5279241c) SHA1(7278b6b037322b2f75311ed247f2de3c4816681b) )
	ROM_LOAD( "6.8c",  0x1400, 0x0400, CRC(9b84fe3a) SHA1(7ebeca10ee11d22f4af06be9f381f46864464ec2) )
	ROM_LOAD( "7.9d",  0x1800, 0x0400, CRC(95279b25) SHA1(367d129d4dd2cfea2a2f4703f41f24cc49453715) )
	ROM_LOAD( "8.8d",  0x1c00, 0x0400, CRC(6a824383) SHA1(7c43f2c7d1f070d93f6a8b5b4f7f97f3578bd91d) )
	ROM_LOAD( "13.9g", 0x3000, 0x0400, CRC(dccc386f) SHA1(5d493da3e7b8269314dd54f0b3ba9f71829a14da) )
	ROM_LOAD( "14.8g", 0x3400, 0x0400, CRC(dc9a10c2) SHA1(8fb2316d6e8aeef558d0da5029e2932abf47a6b4) )
	ROM_LOAD( "15.9h", 0x3800, 0x0400, CRC(55e2950d) SHA1(2241c3620c9a6df8b8bd234ccee9af5d3d19a5d4) )
	ROM_LOAD( "16.8h", 0x3c00, 0x0400, CRC(567259c4) SHA1(b2c3f7aaceabea075af6a43b89fb7331732278c8) )

	ROM_REGION( 0x40, "proms", 0 ) // the 2 PROMs are identical
	ROM_LOAD( "74s288.6a", 0x0000, 0x0020, CRC(ae1f4acd) SHA1(1d502b61db73cf6a4ac3d235455a5c464f12652a) )
	ROM_LOAD( "74s288.6b", 0x0020, 0x0020, CRC(ae1f4acd) SHA1(1d502b61db73cf6a4ac3d235455a5c464f12652a) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1979, spaceg, 0, spaceg, spaceg, spaceg_state, empty_init, ROT270, "Omori Electric Co., Ltd.", "Space Guerrilla", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
