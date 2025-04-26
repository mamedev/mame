// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

Juno First :  memory map same as tutankham with some address changes
Chris Hardy (chrish@kcbbs.gen.nz)

Thanks to Rob Jarret for the original Tutankham memory map on which both the
Juno First emu and the MAME driver is based on.

        Juno First memory map by Chris Hardy

Read/Write memory

$0000-$7FFF = Screen RAM (only written to)
$8000-$800f = Palette RAM. BBGGGRRR (D7->D0)
$8100-$8FFF = Work RAM

Write memory

$8030   - interrupt control register D0 = interrupts on or off
$8031   - unknown
$8032   - unknown
$8033   - unknown
$8034   - flip screen x
$8035   - flip screen y

$8040   - Sound CPU req/ack data
$8050   - Sound CPU command data
$8060   - Banked memory page select.
$8070/1 - Blitter source data word
$8072/3 - Blitter destination word. Write to $8073 triggers a blit

Read memory

$8010   - Dipswitch 2
$801c   - Watchdog
$8020   - Start/Credit IO
                D2 = Credit 1
                D3 = Start 1
                D4 = Start 2
$8024   - P1 IO
                D0 = left
                D1 = right
                D2 = up
                D3 = down
                D4 = fire 2
                D5 = fire 1

$8028   - P2 IO - same as P1 IO
$802c   - Dipswitch 1



$9000->$9FFF Banked Memory - see below
$A000->$BFFF "juno\\JFA_B9.BIN",
$C000->$DFFF "juno\\JFB_B10.BIN",
$E000->$FFFF "juno\\JFC_A10.BIN",

Banked memory - Paged into $9000->$9FFF..

NOTE - In Tutankhm this only contains graphics, in Juno First it also contains code. (which
        generally sets up the blitter)

    "juno\\JFC1_A4.BIN",    $0000->$1FFF
    "juno\\JFC2_A5.BIN",    $2000->$3FFF
    "juno\\JFC3_A6.BIN",    $4000->$5FFF
    "juno\\JFC4_A7.BIN",    $6000->$7FFF
    "juno\\JFC5_A8.BIN",    $8000->$9FFF
    "juno\\JFC6_A9.BIN",    $A000->$bFFF

Blitter source graphics

    "juno\\JFS3_C7.BIN",    $C000->$DFFF
    "juno\\JFS4_D7.BIN",    $E000->$FFFF
    "juno\\JFS5_E7.BIN",    $10000->$11FFF


***************************************************************************/


#include "emu.h"
#include "tutankhm.h"
#include "konamipt.h"

#include "cpu/m6809/m6809.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "konami1.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/flt_rc.h"
#include "screen.h"
#include "speaker.h"


namespace {

class junofrst_state : public tutankhm_state
{
public:
	junofrst_state(const machine_config &mconfig, device_type type, const char *tag)
		: tutankhm_state(mconfig, type, tag)
		, m_audiocpu(*this, "audiocpu")
		, m_i8039(*this, "mcu")
		, m_filter(*this, "filter.0.%u", 0U)
		, m_blitrom(*this, "blitrom")
	{
	}

	void junofrst(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void blitter_w(offs_t offset, uint8_t data);
	void sh_irqtrigger_w(uint8_t data);
	void i8039_irq_w(uint8_t data);
	void i8039_irqen_and_status_w(uint8_t data);
	uint8_t portA_r();
	void portB_w(uint8_t data);

	void _30hz_irq(int state);
	void audio_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void mcu_io_map(address_map &map) ATTR_COLD;
	void mcu_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_audiocpu;
	required_device<i8039_device> m_i8039;
	required_device_array<filter_rc_device, 3> m_filter;
	required_region_ptr<uint8_t> m_blitrom;

	uint8_t  m_blitterdata[4]{};
	uint8_t  m_i8039_status = 0;
	uint8_t  m_last_irq = 0;
};


/* Juno First Blitter Hardware emulation

    Juno First can blit a 16x16 graphics which comes from un-memory mapped graphics roms

    $8070->$8071 specifies the destination NIBBLE address
    $8072->$8073 specifies the source NIBBLE address

    Depending on bit 0 of the source address either the source pixels will be copied to
    the destination address, or a zero will be written.

    Only source pixels which aren't 0 are copied or cleared.

    This allows the game to quickly clear the sprites from the screen

    TODO: Does bit 1 of the source address mean something?
          We have to mask it off otherwise the "Juno First" logo on the title screen is wrong.
*/

void junofrst_state::blitter_w(offs_t offset, uint8_t data)
{
	m_blitterdata[offset] = data;

	/* blitter is triggered by $8073 */
	if (offset == 3)
	{
		offs_t src = ((m_blitterdata[2] << 8) | m_blitterdata[3]) & 0xfffc;
		offs_t dest = (m_blitterdata[0] << 8) | m_blitterdata[1];

		bool const copy = BIT(m_blitterdata[3], 0);

		/* 16x16 graphics */
		for (int i = 0; i < 16; i++)
		{
			for (int j = 0; j < 16; j++)
			{
				uint8_t data;

				if (BIT(src, 0))
					data = m_blitrom[src >> 1] & 0x0f;
				else
					data = m_blitrom[src >> 1] >> 4;

				src++;

				/* if there is a source pixel either copy the pixel or clear the pixel depending on the copy flag */

				if (data)
				{
					if (!copy)
						data = 0;

					if (BIT(dest, 0))
						m_videoram[dest >> 1] = (m_videoram[dest >> 1] & 0x0f) | (data << 4);
					else
						m_videoram[dest >> 1] = (m_videoram[dest >> 1] & 0xf0) | data;
				}
				dest++;
			}
			dest += 240;
		}
	}
}


uint8_t junofrst_state::portA_r()
{
	/* main xtal 14.318MHz, divided by 8 to get the CPU clock, further */
	/* divided by 1024 to get this timer */
	/* (divide by (1024/2), and not 1024, because the CPU cycle counter is */
	/* incremented every other state change of the clock) */
	int const timer = (m_audiocpu->total_cycles() / (1024 / 2)) & 0x0f;

	/* low three bits come from the 8039 */

	return (timer << 4) | m_i8039_status;
}


void junofrst_state::portB_w(uint8_t data)
{
	for (int i = 0; i < 3; i++)
	{
		int C = 0;

		if (BIT(data, 0))
			C += 47000; /* 47000pF = 0.047uF */
		if (BIT(data, 1))
			C += 220000;    /* 220000pF = 0.22uF */

		data >>= 2;
		m_filter[i]->filter_rc_set_RC(filter_rc_device::LOWPASS_3R, 1000, 2200, 200, CAP_P(C));
	}
}


void junofrst_state::sh_irqtrigger_w(uint8_t data)
{
	if (m_last_irq == 0 && data == 1)
	{
		/* setting bit 0 low then high triggers IRQ on the sound CPU */
		m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
	}
	m_last_irq = data;
}


void junofrst_state::i8039_irq_w(uint8_t data)
{
	m_i8039->set_input_line(0, ASSERT_LINE);
}


void junofrst_state::i8039_irqen_and_status_w(uint8_t data)
{
	if (BIT(~data, 7))
		m_i8039->set_input_line(0, CLEAR_LINE);
	m_i8039_status = (data & 0x70) >> 4;
}


void junofrst_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share(m_videoram);
	map(0x8000, 0x800f).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x8010, 0x8010).portr("DSW2");
	map(0x801c, 0x801c).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0x8020, 0x8020).portr("SYSTEM");
	map(0x8024, 0x8024).portr("P1");
	map(0x8028, 0x8028).portr("P2");
	map(0x802c, 0x802c).portr("DSW1");
	map(0x8030, 0x8037).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x8040, 0x8040).w(FUNC(junofrst_state::sh_irqtrigger_w));
	map(0x8050, 0x8050).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x8060, 0x8060).w(FUNC(junofrst_state::bankselect_w));
	map(0x8070, 0x8073).w(FUNC(junofrst_state::blitter_w));
	map(0x8100, 0x8fff).ram();
	map(0x9000, 0x9fff).bankr(m_mainbank);
	map(0xa000, 0xffff).rom();
}


void junofrst_state::audio_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x3000, 0x3000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x4000, 0x4000).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x4001, 0x4001).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x4002, 0x4002).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x5000, 0x5000).w("soundlatch2", FUNC(generic_latch_8_device::write));
	map(0x6000, 0x6000).w(FUNC(junofrst_state::i8039_irq_w));
}


void junofrst_state::mcu_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}


void junofrst_state::mcu_io_map(address_map &map)
{
	map(0x00, 0xff).r("soundlatch2", FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( junofrst )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_MONO_B213_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_B213_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "256 (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6,7")
	PORT_DIPSETTING(    0x70, "1 (Easiest)" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x50, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void junofrst_state::machine_start()
{
	// note that base class version is not called

	m_mainbank->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x1000);

	save_item(NAME(m_i8039_status));
	save_item(NAME(m_last_irq));
	save_item(NAME(m_irq_toggle));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_flipscreen_x));
	save_item(NAME(m_flipscreen_y));
	save_item(NAME(m_blitterdata));
}

void junofrst_state::machine_reset()
{
	// note that base class version is not called
	m_i8039_status = 0;
	m_last_irq = 0;
	m_blitterdata[0] = 0;
	m_blitterdata[1] = 0;
	m_blitterdata[2] = 0;
	m_blitterdata[3] = 0;
	m_irq_toggle = 0;
}

void junofrst_state::_30hz_irq(int state)
{
	/* flip flops cause the interrupt to be signalled every other frame */
	if (state)
	{
		m_irq_toggle ^= 1;
		if (m_irq_toggle && m_irq_enable)
			m_maincpu->set_input_line(0, ASSERT_LINE);
	}
}

void junofrst_state::junofrst(machine_config &config)
{
	/* basic machine hardware */
	KONAMI1(config, m_maincpu, 1500000);         /* 1.5 MHz ??? */
	m_maincpu->set_addrmap(AS_PROGRAM, &junofrst_state::main_map);

	Z80(config, m_audiocpu, 14318000/8);    /* 1.78975 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &junofrst_state::audio_map);

	I8039(config, m_i8039, 8000000);  /* 8MHz crystal */
	m_i8039->set_addrmap(AS_PROGRAM, &junofrst_state::mcu_map);
	m_i8039->set_addrmap(AS_IO, &junofrst_state::mcu_io_map);
	m_i8039->p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));
	m_i8039->p2_out_cb().set(FUNC(junofrst_state::i8039_irqen_and_status_w));

	ls259_device &mainlatch(LS259(config, "mainlatch")); // B3
	mainlatch.q_out_cb<0>().set(FUNC(junofrst_state::irq_enable_w));
	mainlatch.q_out_cb<1>().set(FUNC(junofrst_state::coin_counter_2_w));
	mainlatch.q_out_cb<2>().set(FUNC(junofrst_state::coin_counter_1_w));
	mainlatch.q_out_cb<3>().set_nop();
	mainlatch.q_out_cb<4>().set(FUNC(junofrst_state::flip_screen_x_w)); // HFF
	mainlatch.q_out_cb<5>().set(FUNC(junofrst_state::flip_screen_y_w)); // VFLIP

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(GALAXIAN_PIXEL_CLOCK, GALAXIAN_HTOTAL, GALAXIAN_HBEND, GALAXIAN_HBSTART, GALAXIAN_VTOTAL, GALAXIAN_VBEND, GALAXIAN_VBSTART);
	PALETTE(config, m_palette).set_format(1, tutankhm_state::raw_to_rgb_func, 16);

	m_screen->set_screen_update(FUNC(junofrst_state::screen_update_scramble));
	m_screen->screen_vblank().set(FUNC(junofrst_state::_30hz_irq));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, "soundlatch");
	GENERIC_LATCH_8(config, "soundlatch2");

	ay8910_device &aysnd(AY8910(config, "aysnd", 14318000/8));
	aysnd.port_a_read_callback().set(FUNC(junofrst_state::portA_r));
	aysnd.port_b_write_callback().set(FUNC(junofrst_state::portB_w));
	aysnd.add_route(0, "filter.0.0", 0.30);
	aysnd.add_route(1, "filter.0.1", 0.30);
	aysnd.add_route(2, "filter.0.2", 0.30);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // 100K (R56-63)/200K (R64-71) ladder network

	FILTER_RC(config, m_filter[0]).add_route(ALL_OUTPUTS, "speaker", 1.0);
	FILTER_RC(config, m_filter[1]).add_route(ALL_OUTPUTS, "speaker", 1.0);
	FILTER_RC(config, m_filter[2]).add_route(ALL_OUTPUTS, "speaker", 1.0);
}


ROM_START( junofrst )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 ) /* code + space for decrypted opcodes */
	ROM_LOAD( "jfa_b9.bin",   0x0a000, 0x2000, CRC(f5a7ab9d) SHA1(9603e797839290f8e1f93ccff9cc820604cc49ab) ) /* program ROMs */
	ROM_LOAD( "jfb_b10.bin",  0x0c000, 0x2000, CRC(f20626e0) SHA1(46f58bdc1a613124e2c148b61f774fcc6c232868) )
	ROM_LOAD( "jfc_a10.bin",  0x0e000, 0x2000, CRC(1e7744a7) SHA1(bee69833af886436016560295cddf0c8b4c5e771) )

	ROM_LOAD( "jfc1_a4.bin",  0x10000, 0x2000, CRC(03ccbf1d) SHA1(02b45fe3c51bdc940919aac68136a121ed9bee18) ) /* graphic and code ROMs (banked) */
	ROM_LOAD( "jfc2_a5.bin",  0x12000, 0x2000, CRC(cb372372) SHA1(a48e7de08647cbece7787c287217eac7e7a7510b) )
	ROM_LOAD( "jfc3_a6.bin",  0x14000, 0x2000, CRC(879d194b) SHA1(3c7af8767c9ce908fa1761180c6e585823216d8a) )
	ROM_LOAD( "jfc4_a7.bin",  0x16000, 0x2000, CRC(f28af80b) SHA1(4d0e247e729365476dd3996c7d1f2a19fc83d773) )
	ROM_LOAD( "jfc5_a8.bin",  0x18000, 0x2000, CRC(0539f328) SHA1(c532aaed7f9e6f564e3df0dc6d8fdbee6ed721a2) )
	ROM_LOAD( "jfc6_a9.bin",  0x1a000, 0x2000, CRC(1da2ad6e) SHA1(de997d1b2ff6671088b57192bc9f1279359fad5d) )

	ROM_REGION(  0x1000, "audiocpu", 0 ) /* 4k for Z80 sound CPU code */
	ROM_LOAD( "jfs1_j3.bin",  0x0000, 0x1000, CRC(235a2893) SHA1(b90251c4971f7ba12e407f86c32723d513d6b4a0) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* 8039 */
	ROM_LOAD( "jfs2_p4.bin",  0x0000, 0x1000, CRC(d0fa5d5f) SHA1(9d0730d1d037bf96b0c933a32355602bf2d735dd) )

	ROM_REGION( 0x6000, "blitrom", 0 ) /* BLTROM, used at runtime */
	ROM_LOAD( "jfs3_c7.bin",  0x00000, 0x2000, CRC(aeacf6db) SHA1(f99ef9f9153d7a83e1881d9181faac99cb8c8a57) )
	ROM_LOAD( "jfs4_d7.bin",  0x02000, 0x2000, CRC(206d954c) SHA1(65494766676f18d8b5ae9a54cee00790e7b1e67e) )
	ROM_LOAD( "jfs5_e7.bin",  0x04000, 0x2000, CRC(1eb87a6e) SHA1(f5471b9f6f1fa6d6e5d76300d89f71da3129516a) )
ROM_END

ROM_START( junofrstg )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 ) /* code + space for decrypted opcodes */
	ROM_LOAD( "jfg_a.9b",     0x0a000, 0x2000, CRC(8f77d1c5) SHA1(d47fcdbc47673c228661a3528fff0c691c76df9e) ) /* program ROMs */
	ROM_LOAD( "jfg_b.10b",    0x0c000, 0x2000, CRC(cd645673) SHA1(25994210a8a424bdf2eca3efa19e7eeffc097cec) )
	ROM_LOAD( "jfg_c.10a",    0x0e000, 0x2000, CRC(47852761) SHA1(eeef814b6ad681d4c2274f0a69d1ed9c5c1b9118) )

	ROM_LOAD( "jfgc1.4a",     0x10000, 0x2000, CRC(90a05ae6) SHA1(0aa835e1d33ab0433189b329b791c952e69103c1) ) /* graphic and code ROMs (banked) */
	ROM_LOAD( "jfc2_a5.bin",  0x12000, 0x2000, CRC(cb372372) SHA1(a48e7de08647cbece7787c287217eac7e7a7510b) )
	ROM_LOAD( "jfc3_a6.bin",  0x14000, 0x2000, CRC(879d194b) SHA1(3c7af8767c9ce908fa1761180c6e585823216d8a) )
	ROM_LOAD( "jfgc4.7a",     0x16000, 0x2000, CRC(e8864a43) SHA1(52b04e69036622abeb6ec99ac3daeda6a2572994) )
	ROM_LOAD( "jfc5_a8.bin",  0x18000, 0x2000, CRC(0539f328) SHA1(c532aaed7f9e6f564e3df0dc6d8fdbee6ed721a2) )
	ROM_LOAD( "jfc6_a9.bin",  0x1a000, 0x2000, CRC(1da2ad6e) SHA1(de997d1b2ff6671088b57192bc9f1279359fad5d) )

	ROM_REGION(  0x1000, "audiocpu", 0 ) /* 4k for Z80 sound CPU code */
	ROM_LOAD( "jfs1_j3.bin",  0x0000, 0x1000, CRC(235a2893) SHA1(b90251c4971f7ba12e407f86c32723d513d6b4a0) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* 8039 */
	ROM_LOAD( "jfs2_p4.bin",  0x0000, 0x1000, CRC(d0fa5d5f) SHA1(9d0730d1d037bf96b0c933a32355602bf2d735dd) )

	ROM_REGION( 0x6000, "blitrom", 0 ) /* BLTROM, used at runtime */
	ROM_LOAD( "jfs3_c7.bin",  0x00000, 0x2000, CRC(aeacf6db) SHA1(f99ef9f9153d7a83e1881d9181faac99cb8c8a57) )
	ROM_LOAD( "jfs4_d7.bin",  0x02000, 0x2000, CRC(206d954c) SHA1(65494766676f18d8b5ae9a54cee00790e7b1e67e) )
	ROM_LOAD( "jfs5_e7.bin",  0x04000, 0x2000, CRC(1eb87a6e) SHA1(f5471b9f6f1fa6d6e5d76300d89f71da3129516a) )
ROM_END

} // Anonymous namespace


GAME( 1983, junofrst,  0,        junofrst, junofrst, junofrst_state, empty_init, ROT90, "Konami", "Juno First", MACHINE_SUPPORTS_SAVE )
GAME( 1983, junofrstg, junofrst, junofrst, junofrst, junofrst_state, empty_init, ROT90, "Konami (Gottlieb license)", "Juno First (Gottlieb)", MACHINE_SUPPORTS_SAVE )
