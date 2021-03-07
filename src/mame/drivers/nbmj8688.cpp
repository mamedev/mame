// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    nbmj8688 - Nichibutsu Mahjong games for years 1986-1988

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/01/28 -

******************************************************************************/
/******************************************************************************

TODO:

- Animation in bijokkoy and bijokkog (while DAC playback) is not correct.
  Interrupt problem?

- Sampling rate of some DAC playback in bijokkoy and bijokkog is too high.
  Interrupt problem?

- Some games display "GFXROM BANK OVER!!" or "GFXROM ADDRESS OVER!!"
  in Debug build.

- Increasing the game out rate in kanatuen causes it to fail to boot (hangs at
  black screen).

- Screen flip is not perfect.

- Barline has wrong NMI enable trigger,causing wrong sample pitch (& sometimes
  crashes when you soft reset)

******************************************************************************/

#include "emu.h"
#include "includes/nbmj8688.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/3812intf.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "screen.h"
#include "speaker.h"

#include "nbmj8688.lh"


void nbmj8688_state::init_mjcamera()
{
	uint8_t *rom = memregion("voice")->base() + 0x20000;
	uint8_t *prot = memregion("user1")->base();

	/* this is one possible way to rearrange the protection ROM data to get the
	   expected 0x5894 checksum. It's probably completely wrong! But since the
	   game doesn't do anything else with that ROM, this is more than enough. I
	   could just fill this are with fake data, the only thing that matters is
	   the checksum. */
	for (int i = 0; i < 0x10000; i++)
	{
		rom[i] = bitswap<8>(prot[i],1,6,0,4,2,3,5,7);
	}
}

void nbmj8688_state::init_kanatuen()
{
	/* uses the same protection data as mjcamer, but a different check */
	uint8_t *rom = memregion("voice")->base() + 0x30000;

	rom[0x0004] = 0x09;
	rom[0x0103] = 0x0e;
	rom[0x0202] = 0x08;
	rom[0x0301] = 0xdc;
}

void nbmj8688_state::init_kyuhito()
{
#if 1
	/* uses the same protection data as ????, but a different check */
	uint8_t *rom = memregion("maincpu")->base();

	rom[0x0149] = 0x00;
	rom[0x014a] = 0x00;
	rom[0x014b] = 0x00;
#endif
}

void nbmj8688_state::init_idhimitu()
{
	uint8_t *rom = memregion("voice")->base() + 0x20000;
	uint8_t *prot = memregion("user1")->base();

	/* this is one possible way to rearrange the protection ROM data to get the
	   expected 0x9944 checksum. It's probably completely wrong! But since the
	   game doesn't do anything else with that ROM, this is more than enough. I
	   could just fill this are with fake data, the only thing that matters is
	   the checksum. */
	for (int i = 0; i < 0x10000; i++)
	{
		rom[i] = bitswap<8>(prot[i + 0x10000],4,6,2,1,7,0,3,5);
	}
}

void nbmj8688_state::init_kaguya2()
{
	uint8_t *rom = memregion("voice")->base() + 0x20000;
	uint8_t *prot = memregion("user1")->base();

	/* this is one possible way to rearrange the protection ROM data to get the
	   expected 0x5894 checksum. It's probably completely wrong! But since the
	   game doesn't do anything else with that ROM, this is more than enough. I
	   could just fill this are with fake data, the only thing that matters is
	   the checksum. */
	for (int i = 0; i < 0x10000; i++)
	{
		rom[i] = bitswap<8>(prot[i],1,6,0,4,2,3,5,7);
	}
}


void nbmj8688_state::mjsikaku_map(address_map &map)
{
	map(0x0000, 0xf7ff).rom();
	map(0xf800, 0xffff).ram().share("nvram");
}

void nbmj8688_state::secolove_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram().share("nvram");
}

void nbmj8688_state::ojousan_map(address_map &map)
{
	map(0x0000, 0x6fff).rom();
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).rom();
}

void nbmj8688_state::bikkuri_map(address_map &map)
{
	secolove_map(map);

	map(0xf800, 0xffff).rom();
}

uint8_t nbmj8688_state::ff_r()
{
	/* possibly because of a bug, reads from port 0xd0 must return 0xff
	   otherwise apparel doesn't clear the background when you insert a coin */
	return 0xff;
}

void nbmj8688_state::secolove_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r));
	map(0x00, 0x00).w(m_nb1413m3, FUNC(nb1413m3_device::nmi_clock_w));
	map(0x81, 0x81).r("psg", FUNC(ay8910_device::data_r));
	map(0x82, 0x83).w("psg", FUNC(ay8910_device::data_address_w));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0x90, 0x97).w(FUNC(nbmj8688_state::blitter_w));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_w));
	map(0xc0, 0xcf).w(FUNC(nbmj8688_state::clut_w));
	map(0xd0, 0xd0).r(FUNC(nbmj8688_state::ff_r));  // irq ack? watchdog?
	map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe0, 0xe0).w(FUNC(nbmj8688_state::secolove_romsel_w));
//  map(0xf0, 0xf0).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r));
//  map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
	map(0xf0, 0xf0).w(FUNC(nbmj8688_state::scrolly_w));
}

void nbmj8688_state::bikkuri_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(m_nb1413m3, FUNC(nb1413m3_device::nmi_clock_w));
	map(0x81, 0x81).r("psg", FUNC(ay8910_device::data_r));
	map(0x82, 0x83).w("psg", FUNC(ay8910_device::data_address_w));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0x90, 0x97).w(FUNC(nbmj8688_state::blitter_w));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_alt_w));
	map(0xc0, 0xcf).w(FUNC(nbmj8688_state::clut_w));
	map(0xd0, 0xd0).r(FUNC(nbmj8688_state::ff_r));  // irq ack? watchdog?
	map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe0, 0xe0).w(FUNC(nbmj8688_state::secolove_romsel_w));
}

void nbmj8688_state::barline_output_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x80);
	machine().bookkeeping().coin_counter_w(0, data & 0x02);
	machine().bookkeeping().coin_counter_w(1, data & 0x01);
}

void nbmj8688_state::barline_io_map(address_map &map)
{
	map.global_mask(0xff);
//  map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r));
	map(0x00, 0x00).w(m_nb1413m3, FUNC(nb1413m3_device::sndrombank1_w));
	map(0x70, 0x70).w(m_nb1413m3, FUNC(nb1413m3_device::nmi_clock_w));
	map(0x80, 0x81).rw("psg", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0x90, 0x97).w(FUNC(nbmj8688_state::blitter_w));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).r(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r)).w(FUNC(nbmj8688_state::barline_output_w));
	map(0xc0, 0xcf).w(FUNC(nbmj8688_state::clut_w));
	map(0xd0, 0xd0).r(FUNC(nbmj8688_state::ff_r));  // irq ack? watchdog?
//  map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w)); //not used
	map(0xe0, 0xe0).w(FUNC(nbmj8688_state::secolove_romsel_w));
	map(0xf0, 0xf0).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r)).w(FUNC(nbmj8688_state::scrolly_w));
	map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
}

void nbmj8688_state::crystalg_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r));
	map(0x00, 0x00).w(m_nb1413m3, FUNC(nb1413m3_device::nmi_clock_w));
	map(0x81, 0x81).r("psg", FUNC(ay8910_device::data_r));
	map(0x82, 0x83).w("psg", FUNC(ay8910_device::data_address_w));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0x90, 0x97).w(FUNC(nbmj8688_state::blitter_w));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_w));
	map(0xc0, 0xcf).w(FUNC(nbmj8688_state::clut_w));
	map(0xd0, 0xd0).r(FUNC(nbmj8688_state::ff_r));  // irq ack? watchdog?
	map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe0, 0xe0).w(FUNC(nbmj8688_state::crystalg_romsel_w));
//  map(0xf0, 0xf0).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r));
//  map(0xf0, 0xf0).nopw();
//  map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
}


void nbmj8688_state::otonano_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r));
	map(0x00, 0x00).w(m_nb1413m3, FUNC(nb1413m3_device::nmi_clock_w));
	map(0x20, 0x3f).w(FUNC(nbmj8688_state::clut_w));
	map(0x50, 0x50).w(FUNC(nbmj8688_state::mjsikaku_romsel_w));
	map(0x70, 0x77).w(FUNC(nbmj8688_state::blitter_w));
	map(0x80, 0x81).rw("psg", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_w));
	map(0xd0, 0xd0).r(FUNC(nbmj8688_state::ff_r));  // irq ack? watchdog?
	map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe0, 0xe0).w(FUNC(nbmj8688_state::mjsikaku_gfxflag2_w));
	map(0xf0, 0xf0).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r)).w(FUNC(nbmj8688_state::scrolly_w));
	map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
}


void nbmj8688_state::kaguya_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r));
	map(0x00, 0x00).w(m_nb1413m3, FUNC(nb1413m3_device::nmi_clock_w));
	map(0x20, 0x3f).w(FUNC(nbmj8688_state::clut_w));
	map(0x50, 0x50).w(FUNC(nbmj8688_state::mjsikaku_romsel_w));
	map(0x70, 0x77).w(FUNC(nbmj8688_state::blitter_w));
	map(0x81, 0x81).r("psg", FUNC(ay8910_device::data_r));
	map(0x82, 0x83).w("psg", FUNC(ay8910_device::data_address_w));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_w));
	map(0xd0, 0xd0).r(FUNC(nbmj8688_state::ff_r));  // irq ack? watchdog?
	map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe0, 0xe0).w(FUNC(nbmj8688_state::mjsikaku_gfxflag2_w));
	map(0xf0, 0xf0).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r)).w(FUNC(nbmj8688_state::scrolly_w));
	map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
}


void nbmj8688_state::iemoto_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r));
	map(0x00, 0x00).w(m_nb1413m3, FUNC(nb1413m3_device::nmi_clock_w));
	map(0x10, 0x10).w(m_nb1413m3, FUNC(nb1413m3_device::sndrombank2_w));
	map(0x20, 0x3f).w(FUNC(nbmj8688_state::clut_w));
	map(0x40, 0x47).w(FUNC(nbmj8688_state::blitter_w));
	map(0x50, 0x50).w(FUNC(nbmj8688_state::seiha_romsel_w));
	map(0x81, 0x81).r("psg", FUNC(ay8910_device::data_r));
	map(0x82, 0x83).w("psg", FUNC(ay8910_device::data_address_w));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_w));
	map(0xd0, 0xd0).r(FUNC(nbmj8688_state::ff_r));  // irq ack? watchdog?
	map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe0, 0xe0).w(FUNC(nbmj8688_state::mjsikaku_gfxflag2_w));
//  map(0xf0, 0xf0).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r));
//  map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
	map(0xf0, 0xf0).w(FUNC(nbmj8688_state::scrolly_w));
}


void nbmj8688_state::seiha_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r));
	map(0x00, 0x00).w(m_nb1413m3, FUNC(nb1413m3_device::nmi_clock_w));
	map(0x10, 0x10).w(m_nb1413m3, FUNC(nb1413m3_device::sndrombank2_w));
	map(0x20, 0x3f).w(FUNC(nbmj8688_state::clut_w));
	map(0x50, 0x50).w(FUNC(nbmj8688_state::seiha_romsel_w));
	map(0x81, 0x81).r("psg", FUNC(ay8910_device::data_r));
	map(0x82, 0x83).w("psg", FUNC(ay8910_device::data_address_w));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0x90, 0x97).w(FUNC(nbmj8688_state::blitter_w));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_w));
	map(0xd0, 0xd0).r(FUNC(nbmj8688_state::ff_r));  // irq ack? watchdog?
	map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe0, 0xe0).w(FUNC(nbmj8688_state::mjsikaku_gfxflag2_w));
//  map(0xf0, 0xf0).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r));
//  map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
	map(0xf0, 0xf0).w(FUNC(nbmj8688_state::scrolly_w));
}

void nbmj8688_state::mjgaiden_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r));
	map(0x00, 0x00).w(m_nb1413m3, FUNC(nb1413m3_device::nmi_clock_w));
	map(0x20, 0x3f).w(FUNC(nbmj8688_state::clut_w));
	map(0x50, 0x50).w(FUNC(nbmj8688_state::mjsikaku_romsel_w));
	map(0x81, 0x81).r("psg", FUNC(ay8910_device::data_r));
	map(0x82, 0x83).w("psg", FUNC(ay8910_device::data_address_w));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0x90, 0x97).w(FUNC(nbmj8688_state::blitter_w));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_w));
	map(0xd0, 0xd0).r(FUNC(nbmj8688_state::ff_r));  // irq ack? watchdog?
	map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe0, 0xe0).w(FUNC(nbmj8688_state::mjsikaku_gfxflag2_w));
//  map(0xf0, 0xf0).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r));
//  map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
	map(0xf0, 0xf0).w(FUNC(nbmj8688_state::scrolly_w));
}

void nbmj8688_state::p16bit_LCD_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r));
	map(0x00, 0x00).w(m_nb1413m3, FUNC(nb1413m3_device::nmi_clock_w));
	map(0x42, 0x42).w(m_lcdc0, FUNC(hd61830_device::data_w));
	map(0x43, 0x43).w(m_lcdc0, FUNC(hd61830_device::control_w));
	map(0x44, 0x44).w(m_lcdc1, FUNC(hd61830_device::data_w));
	map(0x45, 0x45).w(m_lcdc1, FUNC(hd61830_device::control_w));
	map(0x46, 0x46).w(FUNC(nbmj8688_state::HD61830B_both_data_w));
	map(0x47, 0x47).w(FUNC(nbmj8688_state::HD61830B_both_instr_w));
	map(0x81, 0x81).r("psg", FUNC(ay8910_device::data_r));
	map(0x82, 0x83).w("psg", FUNC(ay8910_device::data_address_w));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0x90, 0x97).w(FUNC(nbmj8688_state::blitter_w));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_w));
	map(0xc0, 0xcf).w(FUNC(nbmj8688_state::clut_w));
	map(0xd0, 0xd0).r(FUNC(nbmj8688_state::ff_r));  // irq ack? watchdog?
	map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe0, 0xe0).w(FUNC(nbmj8688_state::secolove_romsel_w));
//  map(0xf0, 0xf0).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r));
//  map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
	map(0xf0, 0xf0).w(FUNC(nbmj8688_state::scrolly_w));
}


void nbmj8688_state::mjsikaku_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r));
	map(0x00, 0x00).w(m_nb1413m3, FUNC(nb1413m3_device::nmi_clock_w));
	map(0x10, 0x10).w(m_nb1413m3, FUNC(nb1413m3_device::sndrombank2_w));
	map(0x20, 0x3f).w(FUNC(nbmj8688_state::clut_w));
	map(0x50, 0x50).w(FUNC(nbmj8688_state::mjsikaku_romsel_w));
	map(0x60, 0x67).w(FUNC(nbmj8688_state::blitter_w));
	map(0x80, 0x81).rw("psg", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_w));
	map(0xd0, 0xd0).r(FUNC(nbmj8688_state::ff_r));  // irq ack? watchdog?
	map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe0, 0xe0).w(FUNC(nbmj8688_state::mjsikaku_gfxflag2_w));
	map(0xf0, 0xf0).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r));
	map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
	map(0xf0, 0xf0).w(FUNC(nbmj8688_state::scrolly_w));
}


void nbmj8688_state::mmsikaku_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r));
	map(0x00, 0x00).w(m_nb1413m3, FUNC(nb1413m3_device::nmi_clock_w));
	map(0x10, 0x10).w(m_nb1413m3, FUNC(nb1413m3_device::sndrombank2_w));
	map(0x20, 0x3f).w(FUNC(nbmj8688_state::clut_w));
	map(0x40, 0x47).w(FUNC(nbmj8688_state::blitter_w));
	map(0x50, 0x50).w(FUNC(nbmj8688_state::mjsikaku_romsel_w));
	map(0x81, 0x81).r("psg", FUNC(ay8910_device::data_r));
	map(0x82, 0x83).w("psg", FUNC(ay8910_device::data_address_w));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_w));
	map(0xd0, 0xd0).r(FUNC(nbmj8688_state::ff_r));  // irq ack? watchdog?
	map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe0, 0xe0).w(FUNC(nbmj8688_state::mjsikaku_gfxflag2_w));
//  map(0xf0, 0xf0).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r));
//  map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
	map(0xf0, 0xf0).w(FUNC(nbmj8688_state::scrolly_w));
}

static INPUT_PORTS_START( mjsikaku )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, "Character Display Test" )    PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Game Sounds" )               PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// does not exist on PCB
	PORT_START("DSWB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( mmsikaku )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x00, "Game Out Rate" )             PORT_DIPLOCATION("DSWA:1,2,3") // unconfirmed but extremely likely correct
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "65%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x18, 0x18, "Rate Min" )                  PORT_DIPLOCATION("DSWA:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x60, 0x00, "Rate Max" )                  PORT_DIPLOCATION("DSWA:6,7")
	PORT_DIPSETTING(    0x60, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:8") // score pool?
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:1") // rate up?
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:2") // last chance?
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Character Display Test" )    PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSWB:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSWB:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSWB:8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( otonano )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x00, "TSUMIPAI ENCHOU" )       PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Last chance needs 1,000points" ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x40, "Graphic ROM Test" )      PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Play fee display" )      PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x80, "100 Yen" )
	PORT_DIPSETTING(    0x00, "50 Yen" )

	/* note this is actually unpopulated on the PCB and marked as unused
	       in the manual */
	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "Character Display Test" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( mjcamera )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Character Display Test" )    PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB") // Does not exist on PCB
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( kaguya )
	// manual has DSWA and DSWB mixed up compared to PCB
	// PCB is traditional Nichibutsu legacy, A/B match other PCBs
	// so assumption is manual is incorrect

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, "Nudity graphic on bet" )     PORT_DIPLOCATION("DSWA:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Bet Min" )                   PORT_DIPLOCATION("DSWA:2,3")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x18, 0x00, "Number of extend TSUMO" )    PORT_DIPLOCATION("DSWA:4,5")
	PORT_DIPSETTING(    0x18, "0" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x20, 0x20, "Extend TSUMO needs credit" ) PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSWB:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	// NOTE:Coins counted by pressing service switch
	PORT_DIPNAME( 0x04, 0x04, "NOTE" )                      PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, "Coin x5" )
	PORT_DIPSETTING(    0x00, "Coin x10" )
	PORT_DIPNAME( 0x18, 0x18, "Game Out Rate" )             PORT_DIPLOCATION("DSWB:4,5")
	PORT_DIPSETTING(    0x00, "60%" )
	PORT_DIPSETTING(    0x08, "70%" )
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x18, "90%" )
	PORT_DIPNAME( 0x20, 0x20, "Bonus awarded on" )          PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, "[over BAIMAN]" )
	PORT_DIPSETTING(    0x00, "[BAIMAN]" )
	PORT_DIPNAME( 0x40, 0x40, "Variability of payout rate" ) PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, "[big]" )
	PORT_DIPSETTING(    0x00, "[small]" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END


static INPUT_PORTS_START( kanatuen )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out Rate" )             PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "67%" )
	PORT_DIPSETTING(    0x06, "70%" )
	PORT_DIPSETTING(    0x05, "73%" )
	PORT_DIPSETTING(    0x04, "76%" )
	PORT_DIPSETTING(    0x03, "79%" )
	PORT_DIPSETTING(    0x02, "82%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "88%" )
	PORT_DIPNAME( 0x08, 0x08, "Character Display Test" )    PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Last Chance" )               PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Double Bet" )                PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x06, 0x06, "Bet Min" )                   PORT_DIPLOCATION("DSWB:2,3")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x18, 0x18, "Bet Max" )                   PORT_DIPLOCATION("DSWB:4,5")
	PORT_DIPSETTING(    0x18, "8" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )                  // COIN2

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( kyuhito )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out Rate" )             PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "67%" )
	PORT_DIPSETTING(    0x06, "70%" )
	PORT_DIPSETTING(    0x05, "73%" )
	PORT_DIPSETTING(    0x04, "76%" )
	PORT_DIPSETTING(    0x03, "79%" )
	PORT_DIPSETTING(    0x02, "82%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "88%" )
	PORT_DIPNAME( 0x08, 0x08, "Character Display Test" )    PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Last Chance" )               PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Double Bet" )                PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x06, 0x06, "Bet Min" )                   PORT_DIPLOCATION("DSWB:2,3")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x18, 0x18, "Bet Max" )                   PORT_DIPLOCATION("DSWB:4,5")
	PORT_DIPSETTING(    0x18, "8" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )                  // COIN2

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( idhimitu )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out Rate" )             PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSWA:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin / 10 Credits" )
	PORT_DIPNAME( 0x20, 0x20, "Bet Max" )                   PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Last Chance" )               PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Character Display Test" )    PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Double Bet" )                PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )                  // COIN2

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( secolove )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Nudity" )                    PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )          PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x00, "Number of last chance" )     PORT_DIPLOCATION("DSWB:1,2")
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x04, 0x00, "Hanahai" )                   PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Open Reach of CPU" )         PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Cancel Hand" )               PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Wareme" )                    PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( barline )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Rate" )                 PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x00, "58%" )
	PORT_DIPSETTING(    0x01, "64%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x03, "74%" )
	PORT_DIPSETTING(    0x04, "78%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x06, "88%" )
	PORT_DIPSETTING(    0x07, "95%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:4") // auto stop reels?
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, "Music" )                     PORT_DIPLOCATION("DSWB:5,6")
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x10, "Type 2" )
	PORT_DIPSETTING(    0x20, "Type 3" )
	PORT_DIPSETTING(    0x30, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Key") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("1P Start") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Flip Flop") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / Stop")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Push Bet")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY9")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( citylove )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSWA:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "1 (Easy)" )
	PORT_DIPSETTING(    0x0e, "2" )
	PORT_DIPSETTING(    0x0d, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x0b, "5" )
	PORT_DIPSETTING(    0x0a, "6" )
	PORT_DIPSETTING(    0x09, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x07, "9" )
	PORT_DIPSETTING(    0x06, "10" )
	PORT_DIPSETTING(    0x05, "11" )
	PORT_DIPSETTING(    0x04, "12" )
	PORT_DIPSETTING(    0x03, "13" )
	PORT_DIPSETTING(    0x02, "14" )
	PORT_DIPSETTING(    0x01, "15" )
	PORT_DIPSETTING(    0x00, "16 (Hard)" )
	PORT_DIPNAME( 0x30, 0x30, "YAKUMAN cut" )               PORT_DIPLOCATION("DSWA:5,6")
	PORT_DIPSETTING(    0x30, "10%" )
	PORT_DIPSETTING(    0x20, "30%" )
	PORT_DIPSETTING(    0x10, "50%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x40, 0x00, "Nudity" )                    PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x00, "Number of last chance" )     PORT_DIPLOCATION("DSWB:1,2")
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x04, 0x04, "Hanahai" )                   PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Chonbo" )                    PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Open Reach of CPU" )         PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Open Mode" )                 PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Cansel Type" )               PORT_DIPLOCATION("DSWB:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, "TSUMO 3" )
	PORT_DIPSETTING(    0x40, "TSUMO 7" )
	PORT_DIPSETTING(    0x00, "HAIPAI" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( mcitylov )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x00, "Game Out Rate" )             PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "65%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x18, 0x18, "Rate Min" )                  PORT_DIPLOCATION("DSWA:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x60, 0x60, "Rate Max" )                  PORT_DIPLOCATION("DSWA:6,7")
	PORT_DIPSETTING(    0x60, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x80, "Open Reach Mode" )           PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x01, "Number of last chance" )     PORT_DIPLOCATION("DSWB:1,2")
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x04, 0x04, "Last Chance Charge" )        PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Cancel Type" )               PORT_DIPLOCATION("DSWB:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "All" )
	PORT_DIPNAME( 0x20, 0x20, "Cancel Charge" )             PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Ryuukyoku Renchan" )         PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Score Pool" )                PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR doesn't seem to work?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )                  // COIN1 doesn't seem to work?

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( seiha )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSWA:1,2,3")  // is this correct?
	PORT_DIPSETTING(    0x07, "1 (Hard)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Easy)" )
	PORT_DIPNAME( 0x08, 0x00, "RENCHAN after TENPAIed RYUKYOKU" ) PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Change Pai and Mat Color" )  PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Character Display Test" )    PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSWB:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSWB:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSWB:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSWB:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSWB:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSWB:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSWB:8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( seiham )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x00, "Game Out Rate" )               PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "65%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x18, 0x18, "Rate Min" )                    PORT_DIPLOCATION("DSWA:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x60, 0x00, "Rate Max" )                    PORT_DIPLOCATION("DSWA:6,7")
	PORT_DIPSETTING(    0x60, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Score Pool" )                  PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x00, "Rate Up" )                     PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Last Chance" )                 PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Character Display Test" )      PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )            PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )            PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )            PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )            PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )            PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( iemoto )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Hard)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Easy)" )
	PORT_DIPNAME( 0x08, 0x00, "RENCHAN after TENPAIed RYUKYOKU" ) PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Character Display Test" )    PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSWB:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSWB:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSWB:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSWB:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSWB:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSWB:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSWB:8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( iemotom )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out Rate" )             PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "63%" )
	PORT_DIPSETTING(    0x05, "66%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x18, 0x18, "Rate Min" )                  PORT_DIPLOCATION("DSWA:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x60, 0x60, "Rate Max" )                  PORT_DIPLOCATION("DSWA:6,7")
	PORT_DIPSETTING(    0x60, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Score Pool" )                PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x00, "Rate Up" )                   PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Last Chance" )               PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Character Display Test" )    PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( ryuuha )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out Rate" )             PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "63%" )
	PORT_DIPSETTING(    0x05, "66%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x08, 0x08, "Rate Max" )                  PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Number" )               PORT_DIPLOCATION("DSWA:5,6")
	PORT_DIPSETTING(    0x30, "A" )
	PORT_DIPSETTING(    0x20, "B" )
	PORT_DIPSETTING(    0x10, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPNAME( 0x40, 0x00, "Rate Up" )                   PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Last Chance" )               PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "Character Display Test" )    PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Play" )                      PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( bijokkoy )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )               PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, "2P Simultaneous Play (LCD req'd)" )  PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "See non-Reacher's hand" )            PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Kan-Ura" )                           PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Character Display Test" )            PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x00, "Number of last chance" )             PORT_DIPLOCATION("DSWB:1,2")
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x00, "Double Tsumo" )                      PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Chonbo" )                            PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )                  PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )                  PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )                  PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )                  PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1

	PORT_INCLUDE( nbmjcontrols )

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("Side 1 2P Start")

	PORT_MODIFY("KEY4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Side 1 1P Start")

	PORT_MODIFY("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2) PORT_NAME("Side 2 2P Start")

	PORT_MODIFY("KEY9")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Side 2 1P Start")
INPUT_PORTS_END

static INPUT_PORTS_START( bijokkog )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )               PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, "2P Simultaneous Play (LCD req'd)" )  PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "See non-Reacher's hand" )            PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Kan-Ura" )                           PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Bonus Score for Extra Credit" )      PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x00, "Number of last chance" )             PORT_DIPLOCATION("DSWB:1,2")
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x00, "Double Tsumo" )                      PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Chonbo" )                            PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Cancel Hand" )                       PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coinage ) )                  PORT_DIPLOCATION("DSWB:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1

	PORT_INCLUDE( nbmjcontrols )

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("Side 1 2P Start")

	PORT_MODIFY("KEY4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Side 1 1P Start")

	PORT_MODIFY("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2) PORT_NAME("Side 2 2P Start")

	PORT_MODIFY("KEY9")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Side 2 1P Start")
INPUT_PORTS_END

static INPUT_PORTS_START( housemnq )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )               PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x00, "Kan-Ura" )                           PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Chonbo" )                            PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Character Display Test" )            PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "RENCHAN after TENPAIed RYUKYOKU" )   PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "See non-Reacher's hand" )            PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, "Time" )                              PORT_DIPLOCATION("DSWB:1,2")
	PORT_DIPSETTING(    0x03, "120" )
	PORT_DIPSETTING(    0x02, "100" )
	PORT_DIPSETTING(    0x01, "80" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x0c, 0x0c, "Timer Speed" )                       PORT_DIPLOCATION("DSWB:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Coin Selector" )                     PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, "common" )
	PORT_DIPSETTING(    0x00, "separate" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )                  PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )                  PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1

	PORT_INCLUDE( nbmjcontrols )

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("Side 1 2P Start")

	PORT_MODIFY("KEY4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Side 1 1P Start")

	PORT_MODIFY("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2) PORT_NAME("Side 2 2P Start")

	PORT_MODIFY("KEY9")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Side 2 1P Start")
INPUT_PORTS_END

static INPUT_PORTS_START( housemn2 )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )               PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x00, "Kan-Ura" )                           PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Chonbo" )                            PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Character Display Test" )            PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "RENCHAN after TENPAIed RYUKYOKU" )   PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "See non-Reacher's hand" )            PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, "Time" )                              PORT_DIPLOCATION("DSWB:1,2")
	PORT_DIPSETTING(    0x03, "120" )
	PORT_DIPSETTING(    0x02, "100" )
	PORT_DIPSETTING(    0x01, "80" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x0c, 0x0c, "Timer Speed" )                       PORT_DIPLOCATION("DSWB:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Coin Selector" )                     PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, "common" )
	PORT_DIPSETTING(    0x00, "separate" )
	PORT_DIPNAME( 0xc0, 0xc0, "Character Display Test (manual)" )   PORT_DIPLOCATION("DSWB:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1

	PORT_INCLUDE( nbmjcontrols )

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("Side 1 2P Start")

	PORT_MODIFY("KEY4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Side 1 1P Start")

	PORT_MODIFY("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2) PORT_NAME("Side 2 2P Start")

	PORT_MODIFY("KEY9")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Side 2 1P Start")
INPUT_PORTS_END

static INPUT_PORTS_START( orangec )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x00, "Select Girl" )               PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "Extend TSUMO" )              PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )    PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSWB:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSWB:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSWB:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSWB:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSWB:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSWB:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSWB:8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( orangeci )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out Rate" )             PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "64%" )
	PORT_DIPSETTING(    0x05, "67%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x08, 0x00, "Select Girl" )               PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Play" )                      PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )    PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Rate Max" )                  PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Last Chance" )               PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Odds Number" )               PORT_DIPLOCATION("DSWB:5,6")
	PORT_DIPSETTING(    0x30, "A" )
	PORT_DIPSETTING(    0x20, "B" )
	PORT_DIPSETTING(    0x10, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( vipclub )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out Rate" )             PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "64%" )
	PORT_DIPSETTING(    0x05, "67%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x08, 0x00, "Rate Up" )                   PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Play" )                      PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )    PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Rate Max" )                  PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Last Chance" )               PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Odds Number" )               PORT_DIPLOCATION("DSWB:5,6")
	PORT_DIPSETTING(    0x30, "A" )
	PORT_DIPSETTING(    0x20, "B" )
	PORT_DIPSETTING(    0x10, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( livegal )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out Rate" )             PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "65%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x08, 0x08, "Rate Max" )                  PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "9" )
	PORT_DIPNAME( 0x10, 0x00, "Rate Up" )                   PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Odds Number" )               PORT_DIPLOCATION("DSWA:6,7")
	PORT_DIPSETTING(    0x60, "A" )
	PORT_DIPSETTING(    0x40, "B" )
	PORT_DIPSETTING(    0x20, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPNAME( 0x80, 0x80, "Last Chance Charge" )        PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown )  )         PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Time Speed" )                PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSWB:3,4")
	PORT_DIPSETTING(    0x0c, "1 (Easy)" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4 (Hard)" )
	PORT_DIPNAME( 0x10, 0x10, "Character Display Test" )    PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSWB:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )                  // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1

	PORT_INCLUDE( nbmjcontrols )

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("Side 1 2P Start / Rate")

	PORT_MODIFY("KEY4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Side 1 1P Start")

	PORT_MODIFY("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2) PORT_NAME("Side 2 2P Start / Rate")

	PORT_MODIFY("KEY9")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Side 2 1P Start")
INPUT_PORTS_END

static INPUT_PORTS_START( ojousan )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x00, "RENCHAN after TENPAIed RYUKYOKU" )   PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Character Display Test" )        PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SWA:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWA:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWA:8" )

	PORT_START("DSWB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( ojousanm )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out Rate" )             PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "63%" )
	PORT_DIPSETTING(    0x05, "66%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x18, 0x18, "Rate Min" )                  PORT_DIPLOCATION("DSWA:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x60, 0x60, "Rate Max" )                  PORT_DIPLOCATION("DSWA:6,7")
	PORT_DIPSETTING(    0x60, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Score Pool" )                PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x00, "Rate Up" )                   PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Last Chance" )               PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Character Display Test" )    PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )               // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( korinai )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Character Display Test" )    PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Select" )                    PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x00, "Girl" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSWB:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSWB:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSWB:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSWB:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSWB:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSWB:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSWB:8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )               // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( korinaim )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out Rate" )               PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "63%" )
	PORT_DIPSETTING(    0x05, "66%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x18, 0x18, "Rate Min" )                    PORT_DIPLOCATION("DSWA:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x60, 0x60, "Rate Max" )                    PORT_DIPLOCATION("DSWA:6,7")
	PORT_DIPSETTING(    0x60, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )            PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x00, "Double Bet" )                  PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Last Chance" )                 PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Character Display Test" )      PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )            PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )            PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )            PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )            PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Score Pool" )                  PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )               // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( crystalg )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x0f, 0x0d, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSWA:1,2,3,4")
	PORT_DIPSETTING(    0x0d, "1 (Easy)" )
	PORT_DIPSETTING(    0x0a, "2" )
	PORT_DIPSETTING(    0x09, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x07, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x05, "7" )
	PORT_DIPSETTING(    0x04, "8" )
	PORT_DIPSETTING(    0x00, "9 (Hard)" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSWB:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSWB:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSWB:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSWB:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSWB:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSWB:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSWB:8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )               // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( crystal2 )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x0f, 0x0d, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSWA:1,2,3,4")
	PORT_DIPSETTING(    0x0d, "1 (Easy)" )
	PORT_DIPSETTING(    0x0a, "2" )
	PORT_DIPSETTING(    0x09, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x07, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x05, "7" )
	PORT_DIPSETTING(    0x04, "8" )
	PORT_DIPSETTING(    0x00, "9 (Hard)" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x00, "Number of last chance" )     PORT_DIPLOCATION("DSWB:1,2")
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x0c, 0x00, "SANGEN Rush" )               PORT_DIPLOCATION("DSWB:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )         // OPTION (?)

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( apparel )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out Rate" )               PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "65%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x18, 0x10, "Rate Max" )                    PORT_DIPLOCATION("DSWA:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x60, 0x60, "Odds Number" )                 PORT_DIPLOCATION("DSWA:6,7")
	PORT_DIPSETTING(    0x60, "A" )
	PORT_DIPSETTING(    0x40, "B" )
	PORT_DIPSETTING(    0x20, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPNAME( 0x80, 0x00, "Rate Up" )                     PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x02, "Number of last chance" )       PORT_DIPLOCATION("DSWB:1,2")
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x0c, 0x04, "Cancel Type" )                 PORT_DIPLOCATION("DSWB:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, "Tsumo 3" )
	PORT_DIPSETTING(    0x04, "Tsumo 7" )
	PORT_DIPSETTING(    0x00, "Haipai" )
	PORT_DIPNAME( 0x10, 0x10, "Ryuukoku Renchan" )            PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Cancel Charge" )               PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Last Chance Charge" )          PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Open Reach Mode" )             PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )               // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( nightlov )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x0f, 0x0f, "Game Out Rate" )         PORT_DIPLOCATION("DSWA:1,2,3,4")
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPNAME( 0x30, 0x10, "Rate Max" )              PORT_DIPLOCATION("DSWA:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSWB:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSWB:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSWB:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSWB:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSWB:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSWB:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSWB:8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )           // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE   // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                         // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                  // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )               // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( bikkuri )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out Rate" ) PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "95%" )
	PORT_DIPSETTING(    0x06, "90%" )
	PORT_DIPSETTING(    0x05, "85%" )
	PORT_DIPSETTING(    0x04, "80%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x00, "60%" )
	// coinage
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSWA:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSWA:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSWA:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSWA:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSWA:8")

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSWB:1")
	PORT_SERVICE_DIPLOC( 0x02, IP_ACTIVE_LOW, "DSWB:2" )
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSWB:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSWB:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSWB:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSWB:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSWB:7")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Medal In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Mode SW")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("100 Yen In")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Solenoid SW") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("10 Yen In")

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Drop Kick")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Neck Hang")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Lariat")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Medal Out") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("P1 Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x3e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void nbmj8688_state::NBMJDRV_4096(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 5000000);   /* 5.00 MHz */
	m_maincpu->set_vblank_int("screen", FUNC(nbmj8688_state::irq0_line_hold));

	NB1413M3(config, m_nb1413m3, 0);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 16, 240-1);
	screen.set_screen_update(FUNC(nbmj8688_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(nbmj8688_state::mbmj8688_12bit), 4096);

	MCFG_VIDEO_START_OVERRIDE(nbmj8688_state,mbmj8688_pure_12bit)

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	ay8910_device &psg(AY8910(config, "psg", 1250000));
	psg.port_a_read_callback().set(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r));     // DIPSW-A read
	psg.port_b_read_callback().set(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));     // DIPSW-B read
	psg.add_route(ALL_OUTPUTS, "speaker", 0.35);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
}

void nbmj8688_state::NBMJDRV_256(machine_config &config)
{
	NBMJDRV_4096(config);

	/* basic machine hardware */

	/* video hardware */
	subdevice<palette_device>("palette")->set_entries(256).set_init(FUNC(nbmj8688_state::mbmj8688_8bit));

	MCFG_VIDEO_START_OVERRIDE(nbmj8688_state,mbmj8688_8bit)
}

void nbmj8688_state::NBMJDRV_65536(machine_config &config)
{
	NBMJDRV_4096(config);

	/* basic machine hardware */

	/* video hardware */
	subdevice<palette_device>("palette")->set_entries(65536).set_init(FUNC(nbmj8688_state::mbmj8688_16bit));

	MCFG_VIDEO_START_OVERRIDE(nbmj8688_state,mbmj8688_hybrid_16bit)
}

// --------------------------------------------------------------------------------

void nbmj8688_state::crystalg(machine_config &config)
{
	NBMJDRV_256(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::secolove_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::crystalg_io_map);

	m_nb1413m3->set_type(NB1413M3_CRYSTALG);
}

void nbmj8688_state::crystal2(machine_config &config)
{
	crystalg(config);

	m_nb1413m3->set_type(NB1413M3_CRYSTAL2);
}

void nbmj8688_state::nightlov(machine_config &config)
{
	crystalg(config);

	m_nb1413m3->set_type(NB1413M3_NIGHTLOV);
}

void nbmj8688_state::apparel(machine_config &config)
{
	NBMJDRV_256(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::secolove_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::secolove_io_map);

	m_nb1413m3->set_type(NB1413M3_APPAREL);
}

void nbmj8688_state::mbmj_h12bit(machine_config &config)
{
	NBMJDRV_4096(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::secolove_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::secolove_io_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(nbmj8688_state,mbmj8688_hybrid_12bit)
}

void nbmj8688_state::citylove(machine_config &config)
{
	mbmj_h12bit(config);

	m_nb1413m3->set_type(NB1413M3_CITYLOVE);
}

void nbmj8688_state::mcitylov(machine_config &config)
{
	mbmj_h12bit(config);

	m_nb1413m3->set_type(NB1413M3_MCITYLOV);
}

void nbmj8688_state::secolove(machine_config &config)
{
	mbmj_h12bit(config);

	m_nb1413m3->set_type(NB1413M3_SECOLOVE);
}

/*Same as h12bit HW with different sound HW + NMI enable bit*/
void nbmj8688_state::barline(machine_config &config)
{
	mbmj_h12bit(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::barline_io_map);

	m_nb1413m3->set_type(NB1413M3_BARLINE);

	YM3812(config.replace(), "psg", 20000000/8).add_route(ALL_OUTPUTS, "speaker", 0.35);

	config.device_remove("dac");
}

void nbmj8688_state::mbmj_p16bit(machine_config &config)
{
	NBMJDRV_65536(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::secolove_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::secolove_io_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(nbmj8688_state,mbmj8688_pure_16bit)
}

void nbmj8688_state::mbmj_p16bit_LCD(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 5000000);   /* 5.00 MHz */
	m_maincpu->set_vblank_int("screen", FUNC(nbmj8688_state::irq0_line_hold));
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::secolove_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::p16bit_LCD_io_map);

	NB1413M3(config, m_nb1413m3, 0);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	PALETTE(config, "palette", FUNC(nbmj8688_state::mbmj8688_16bit), 65536);
	config.set_default_layout(layout_nbmj8688);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 16, 240-1);
	screen.set_screen_update(FUNC(nbmj8688_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette_lcd", FUNC(nbmj8688_state::mbmj8688_lcd), 2);

	screen_device &lcd0(SCREEN(config, "lcd0", SCREEN_TYPE_LCD));
	lcd0.set_physical_aspect(15, 3);
	lcd0.set_orientation(ROT180);
	lcd0.set_refresh_hz(60);
	lcd0.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lcd0.set_size(480, 64);
	lcd0.set_visarea(0, 480-1, 0, 64-1);
	lcd0.set_screen_update("lcdc0", FUNC(hd61830_device::screen_update));
	lcd0.set_palette("palette_lcd");
	HD61830B(config, m_lcdc0,  5000000/2); // ???
	m_lcdc0->set_screen("lcd0");

	screen_device &lcd1(SCREEN(config, "lcd1", SCREEN_TYPE_LCD));
	lcd1.set_physical_aspect(15, 3);
	lcd1.set_orientation(ROT180);
	lcd1.set_refresh_hz(60);
	lcd1.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lcd1.set_size(480, 64);
	lcd1.set_visarea(0, 480-1, 0, 64-1);
	lcd1.set_screen_update("lcdc1", FUNC(hd61830_device::screen_update));
	lcd1.set_palette("palette_lcd");
	HD61830B(config, m_lcdc1,  5000000/2); // ???
	m_lcdc1->set_screen("lcd1");

	MCFG_VIDEO_START_OVERRIDE(nbmj8688_state,mbmj8688_pure_16bit_LCD)

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	ay8910_device &psg(AY8910(config, "psg", 1250000));
	psg.port_a_read_callback().set(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r));     // DIPSW-A read
	psg.port_b_read_callback().set(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));     // DIPSW-B read
	psg.add_route(ALL_OUTPUTS, "speaker", 0.35);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
}

void nbmj8688_state::bijokkoy(machine_config &config)
{
	mbmj_p16bit_LCD(config);

	m_nb1413m3->set_type(NB1413M3_BIJOKKOY);
}

void nbmj8688_state::bijokkog(machine_config &config)
{
	mbmj_p16bit_LCD(config);

	m_nb1413m3->set_type(NB1413M3_BIJOKKOG);
}

void nbmj8688_state::housemnq(machine_config &config)
{
	mbmj_p16bit_LCD(config);

	m_nb1413m3->set_type(NB1413M3_HOUSEMNQ);
}

void nbmj8688_state::housemn2(machine_config &config)
{
	mbmj_p16bit_LCD(config);

	m_nb1413m3->set_type(NB1413M3_HOUSEMN2);
}

void nbmj8688_state::livegal(machine_config &config)
{
	mbmj_p16bit_LCD(config);

	m_nb1413m3->set_type(NB1413M3_LIVEGAL);
}

void nbmj8688_state::orangec(machine_config &config)
{
	mbmj_p16bit(config);

	m_nb1413m3->set_type(NB1413M3_ORANGEC);
}

void nbmj8688_state::orangeci(machine_config &config)
{
	mbmj_p16bit(config);

	m_nb1413m3->set_type(NB1413M3_ORANGECI);
}

void nbmj8688_state::vipclub(machine_config &config)
{
	mbmj_p16bit(config);

	m_nb1413m3->set_type(NB1413M3_VIPCLUB);
}

void nbmj8688_state::seiha(machine_config &config)
{
	NBMJDRV_65536(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::secolove_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::seiha_io_map);

	m_nb1413m3->set_type(NB1413M3_SEIHA);
}

void nbmj8688_state::seiham(machine_config &config)
{
	seiha(config);

	m_nb1413m3->set_type(NB1413M3_SEIHAM);
}

void nbmj8688_state::mjgaiden(machine_config &config)
{
	NBMJDRV_4096(config);

	/* basic machine hardware */

	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::ojousan_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::mjgaiden_io_map);

	m_nb1413m3->set_type(NB1413M3_OJOUSAN);
}

void nbmj8688_state::iemoto(machine_config &config)
{
	NBMJDRV_65536(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::secolove_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::iemoto_io_map);

	m_nb1413m3->set_type(NB1413M3_IEMOTO);
}

void nbmj8688_state::ojousan(machine_config &config)
{
	NBMJDRV_65536(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::ojousan_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::iemoto_io_map);

	m_nb1413m3->set_type(NB1413M3_OJOUSAN);
}

void nbmj8688_state::ojousanm(machine_config &config)
{
	ojousan(config);

	m_nb1413m3->set_type(NB1413M3_OJOUSANM);
}

void nbmj8688_state::bikkuri(machine_config &config)
{
	NBMJDRV_256(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::bikkuri_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::bikkuri_io_map);

	// m_nb1413m3->set_type(NB1413M3_TAIWANMB); // what's the correct type here?
}

void nbmj8688_state::swinggal(machine_config &config)
{
	ojousan(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::secolove_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::iemoto_io_map);
}

void nbmj8688_state::iemotom(machine_config &config)
{
	ojousan(config);

	m_nb1413m3->set_type(NB1413M3_IEMOTOM);
}

void nbmj8688_state::ryuuha(machine_config &config)
{
	ojousan(config);

	m_nb1413m3->set_type(NB1413M3_RYUUHA);
}

void nbmj8688_state::korinai(machine_config &config)
{
	ojousan(config);

	m_nb1413m3->set_type(NB1413M3_KORINAI);
}

void nbmj8688_state::korinaim(machine_config &config)
{
	ojousan(config);

	m_nb1413m3->set_type(NB1413M3_KORINAIM);
}

void nbmj8688_state::mbmj_p12bit(machine_config &config)
{
	NBMJDRV_4096(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::mjsikaku_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::kaguya_io_map);
}

void nbmj8688_state::kaguya(machine_config &config)
{
	mbmj_p12bit(config);

	m_nb1413m3->set_type(NB1413M3_KAGUYA);
}

void nbmj8688_state::kaguya2(machine_config &config)
{
	mbmj_p12bit(config);

	m_nb1413m3->set_type(NB1413M3_KAGUYA2);
}

void nbmj8688_state::kanatuen(machine_config &config)
{
	mbmj_p12bit(config);

	m_nb1413m3->set_type(NB1413M3_KANATUEN);
}

void nbmj8688_state::kyuhito(machine_config &config)
{
	mbmj_p12bit(config);

	m_nb1413m3->set_type(NB1413M3_KYUHITO);
}

void nbmj8688_state::idhimitu(machine_config &config)
{
	mbmj_p12bit(config);

	m_nb1413m3->set_type(NB1413M3_IDHIMITU);
}

void nbmj8688_state::mjsikaku(machine_config &config)
{
	NBMJDRV_4096(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::mjsikaku_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::mjsikaku_io_map);

	m_nb1413m3->set_type(NB1413M3_MJSIKAKU);

	/* sound hardware */
	YM3812(config.replace(), "psg", 20000000/8).add_route(ALL_OUTPUTS, "speaker", 0.7);
}

void nbmj8688_state::mmsikaku(machine_config &config)
{
	NBMJDRV_4096(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8688_state::secolove_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::mmsikaku_io_map);

	m_nb1413m3->set_type(NB1413M3_MMSIKAKU);
}

void nbmj8688_state::otonano(machine_config &config)
{
	mjsikaku(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &nbmj8688_state::otonano_io_map);

	m_nb1413m3->set_type(NB1413M3_OTONANO);
}

void nbmj8688_state::mjcamera(machine_config &config)
{
	otonano(config);

	m_nb1413m3->set_type(NB1413M3_MJCAMERA);
}

ROM_START( crystalg )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "mbs1.3c",  0x00000, 0x04000, CRC(1cacdbbd) SHA1(672e67b761ef1723ec2445f13881a435dbbdf53f) )
	ROM_LOAD( "mbs2.4c",  0x04000, 0x04000, CRC(bf833674) SHA1(45bc63313acc7b3c4bbbe3070dd25cac549e475c) )
	ROM_LOAD( "mbs3.5c",  0x08000, 0x04000, CRC(faacafd0) SHA1(facab33c668a15dc85ada690dd02ffee2c332485) )
	ROM_LOAD( "mbs4.6c",  0x0c000, 0x04000, CRC(b3bedcf1) SHA1(12d1b9ab94d77fdb5d6d4e42da447d27e50815d8) )

	ROM_REGION( 0x10000, "voice", ROMREGION_ERASE00 ) /* voice */
	// not used

	ROM_REGION( 0x080000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "ft1.1h",   0x000000, 0x08000, CRC(99b982ea) SHA1(0c9f76dd30d722bd359b24320fabb62944f6e841) )
	ROM_LOAD( "ft2.2h",   0x008000, 0x08000, CRC(026301da) SHA1(efeb98385f04a22e759178ce41188c0f02a2aea7) )
	ROM_LOAD( "ft3.3h",   0x010000, 0x08000, CRC(bff22ef7) SHA1(96c743ad81e8dda81ca3ff24114f5dc5b0168cab) )
	ROM_LOAD( "ft4.4h",   0x018000, 0x08000, CRC(4601e3a7) SHA1(41918bfef9239c9788de7b9b01ce9e05839fc768) )
	ROM_LOAD( "ft5.5h",   0x020000, 0x08000, CRC(e1388239) SHA1(e858edfd7caf93f54ce104027cb2d1a493b80308) )
	ROM_LOAD( "ft6.6h",   0x028000, 0x08000, CRC(da635046) SHA1(297390dbe02fbbdd9c6bf25a09cc342a4b53856b) )
	ROM_LOAD( "ft7.7h",   0x030000, 0x08000, CRC(b4d2121b) SHA1(4f692a560bf4d7740d47e08b1f039889e664a4a6) )
	ROM_LOAD( "ft8.8h",   0x038000, 0x08000, CRC(b3fab376) SHA1(e3422c62fad1488bd1c3f06cbb6ab60f142eab89) )
	ROM_LOAD( "ft9.9h",   0x040000, 0x08000, CRC(3d4102ca) SHA1(ca88adf84f50b88e3b44e2d30ef05eec1fa9de0c) )
	ROM_LOAD( "ft10.10h", 0x048000, 0x08000, CRC(264b6f7d) SHA1(ba5cd9c426afae23c83759a61591a31cfdaf8e29) )
ROM_END

ROM_START( crystal2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "cgl2_01.bin",  0x00000, 0x04000, CRC(67673350) SHA1(c7b9e5f0a9e073793db74304272c94657328cd92) )
	ROM_LOAD( "cgl2_02.bin",  0x04000, 0x04000, CRC(79c599d8) SHA1(e70cbb2b2b7867be81bc133bb4dbc19753578b7e) )
	ROM_LOAD( "cgl2_03.bin",  0x08000, 0x04000, CRC(c11987ed) SHA1(69a7c252b28843437a06812213ad1d6de683aac3) )
	ROM_LOAD( "cgl2_04.bin",  0x0c000, 0x04000, CRC(ae0b7df8) SHA1(9ca74fd087d299195b06aa8ea811393e3f87b76f) )

	ROM_REGION( 0x10000, "voice", ROMREGION_ERASE00 ) /* voice */
	// not used

	ROM_REGION( 0x080000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "cgl2_01s.bin", 0x000000, 0x08000, CRC(99b982ea) SHA1(0c9f76dd30d722bd359b24320fabb62944f6e841) ) // crystalg/ft1.1h
	ROM_LOAD( "cgl2_01m.bin", 0x008000, 0x08000, CRC(7c7a0416) SHA1(2dade3d7c2045ba84ea4bddd21264a2d1fd328f3) )
	ROM_LOAD( "cgl2_02m.bin", 0x010000, 0x08000, CRC(8511ddcd) SHA1(001f9cea0320eedb42736ed29c746642eec2b460) )
	ROM_LOAD( "cgl2_03m.bin", 0x018000, 0x08000, CRC(f594e3bc) SHA1(96bb0b5397934038a7a57fce77f5ee0ca09a8992) )
	ROM_LOAD( "cgl2_04m.bin", 0x020000, 0x08000, CRC(01a6bf99) SHA1(610b61259037a19ab01617dcc14089e45d3b3ee0) )
	ROM_LOAD( "cgl2_05m.bin", 0x028000, 0x08000, CRC(ee941bf6) SHA1(97dc870100e33f9cf3a803ebf957540de63963b1) )
	ROM_LOAD( "cgl2_06m.bin", 0x030000, 0x08000, CRC(93a8bf3b) SHA1(29648e1349909cb5a3416e688864d51503823872) )
	ROM_LOAD( "cgl2_07m.bin", 0x038000, 0x08000, CRC(b9626199) SHA1(44c52a1362b6c6609e7c3442d3c651cfec7795fc) )
	ROM_LOAD( "cgl2_08m.bin", 0x040000, 0x08000, CRC(8a4d02c9) SHA1(69c047cf480eb9edeee6d8fefc82e0273de15495) )
	ROM_LOAD( "cgl2_09m.bin", 0x048000, 0x08000, CRC(e0d58e86) SHA1(474b577f24f82036c16304994f9fbec8fb07aa04) )
	ROM_LOAD( "cgl2_02s.bin", 0x050000, 0x08000, CRC(7e0ca2a5) SHA1(90ea16a4557b2f3c87492d63e4fc7ce6cac561ba) )
	ROM_LOAD( "cgl2_03s.bin", 0x058000, 0x08000, CRC(78fc9502) SHA1(bb3bae6d4a57818f349c915b8bf15424b3bdd241) )
	ROM_LOAD( "cgl2_04s.bin", 0x060000, 0x08000, CRC(c2140826) SHA1(f8d8692cd3968f586f1bd37a292747ef00a493e0) )
	ROM_LOAD( "cgl2_05s.bin", 0x068000, 0x08000, CRC(257df5f3) SHA1(df0f5f9859a90f6fc8171bb2cdc1bee9cdbf2c27) )
	ROM_LOAD( "cgl2_06s.bin", 0x070000, 0x08000, CRC(27da3e4d) SHA1(fe0446af7eada1d937496e488422b5988e43bc2a) )
	ROM_LOAD( "cgl2_07s.bin", 0x078000, 0x08000, CRC(bd202788) SHA1(85c8b80b019476942696ce2295167c6cf8e3afc3) )
ROM_END

ROM_START( apparel )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "11.bin", 0x00000, 0x04000, CRC(31bd49d5) SHA1(104f468254e255a9a1537ec67a670f60a64e15c6) )
	ROM_LOAD( "12.bin", 0x04000, 0x04000, CRC(56acd87d) SHA1(d228db94b66c1f7da80c799251c28a0bc3e48ea9) )
	ROM_LOAD( "13.bin", 0x08000, 0x04000, CRC(3e2a9c66) SHA1(f6a4859463524f46002fc5684d573d1fae8a5e22) )

	ROM_REGION( 0x10000, "voice", ROMREGION_ERASE00 ) /* voice */
	// not used

	ROM_REGION( 0x0a0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "1.bin",  0x000000, 0x10000, CRC(6c7713ea) SHA1(a15851ca16ea0e30a9d3e6f8f9897eaafe1ff05a) )
	ROM_LOAD( "2.bin",  0x010000, 0x10000, CRC(206f4d2c) SHA1(53a05a92ded54e90eb59c55376434fa0203f96e9) )
	ROM_LOAD( "3.bin",  0x020000, 0x10000, CRC(5d8a732b) SHA1(a478af0dc3b9042fcc9ce8338226d5a378b05491) )
	ROM_LOAD( "4.bin",  0x030000, 0x10000, CRC(c40e4435) SHA1(b66c654a75be6759fa030e51b6484af7c37fca12) )
	ROM_LOAD( "5.bin",  0x040000, 0x10000, CRC(e5bde704) SHA1(6fd0d5defe0d2072a1f9efd21e2a003b9212847b) )
	ROM_LOAD( "6.bin",  0x050000, 0x10000, CRC(263673bc) SHA1(fa713101f2bf6874080c3c8db7cb55c9c084d502) )
	ROM_LOAD( "7.bin",  0x060000, 0x10000, CRC(c502dc5a) SHA1(99ae8db3f06395ab5ca0828aad6e679090008aab) )
	ROM_LOAD( "8.bin",  0x070000, 0x10000, CRC(c0af5f0f) SHA1(3e2c7c6a28540cd04366ff02d12ad566fd9d277d) )
	ROM_LOAD( "9.bin",  0x080000, 0x10000, CRC(477b6cdd) SHA1(31ecd6e2cf307d604b2c6852c45ec782f1f6d0f0) )
	ROM_LOAD( "10.bin", 0x090000, 0x10000, CRC(d06d8972) SHA1(691657f9db1b2edcfb128f9faefcceb490d2bb08) )
ROM_END

ROM_START( nightlov )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "7.ic3",  0x00000, 0x08000, CRC(f9be0b15) SHA1(11b60bb48c78fefae3e8dc63f79a3bebbf701575) )
	ROM_LOAD( "8.ic4",  0x08000, 0x08000, CRC(034c2b8c) SHA1(0aa10b28647b3511f17c62cd439f289c59ba4c01) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "9.ic5",  0x00000, 0x08000, CRC(634c2831) SHA1(02009b0bdf4e9502bcb4cf95614b7ca2e2b2f232) )
	ROM_LOAD( "10.ic6", 0x08000, 0x08000, CRC(7705ca10) SHA1(569e403dc9c7055e4ac5c4a3aa067eae37de2b12) )

	ROM_REGION( 0x060000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "5.6h",   0x00000, 0x10000, CRC(c4d5ce04) SHA1(78aef48edfd7c8f4fcdc93a5522781c4c70da2df) )
	ROM_LOAD( "2.3h",   0x10000, 0x10000, CRC(da371364) SHA1(453aba636bfcb9825acd862a5df1a0f73e7d7232) )
	ROM_LOAD( "4.5h",   0x20000, 0x10000, CRC(d65dba39) SHA1(cccf87da7a2bea9875f120180003128a983625a9) )
	ROM_LOAD( "6.7h",   0x30000, 0x10000, CRC(cacf36c1) SHA1(08c79d92333b6cef29471cbd35de4235efb8c1b1) )
	ROM_LOAD( "1.2h",   0x40000, 0x10000, CRC(80c12f4b) SHA1(15595c156b7d48e7993e8479b7957a9714c55346) )
	ROM_LOAD( "3.4h",   0x50000, 0x10000, CRC(70cffdac) SHA1(2edc1216ba462f6f56daf28dd39896927d6b06c8) )
ROM_END

ROM_START( citylove )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "14.12c", 0x00000, 0x08000, CRC(2db5186c) SHA1(4cd282aebaf5f9f31008c2f9ccb65d9c9b4f8f56) )
	ROM_LOAD( "13.11c", 0x08000, 0x08000, CRC(52c7632b) SHA1(05b24612c52ed41195f2d98edbb5c12b0ef97b74) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "11.8c",  0x00000, 0x08000, CRC(eabb3f32) SHA1(e06426001e17c878ae35deb70a8155095bfeaa36) )
	ROM_LOAD( "12.10c", 0x08000, 0x08000, CRC(c280f573) SHA1(2a534335f08ec6886510ca92be6247e11804e706) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "1.1h",   0x00000, 0x10000, CRC(55b911a3) SHA1(844a567a36bc2ff772ef3949b42b38b6e160807a) )
	ROM_LOAD( "2.2h",   0x10000, 0x10000, CRC(35298484) SHA1(bb1fea61cb67440ccef08d15b23e1a64b0a611f6) )
	ROM_LOAD( "3.4h",   0x20000, 0x10000, CRC(6860c6d3) SHA1(516ef0ba02e1d122bfcfc04482f0af4179d6ed61) )
	ROM_LOAD( "4.5h",   0x30000, 0x10000, CRC(21085a9a) SHA1(1651acd7bb15a0c5c9dbb89f58f6695ff3bdf90a) )
	ROM_LOAD( "5.7h",   0x40000, 0x10000, CRC(fcf53e1a) SHA1(1a9e76e3b7f24c48dd1efbc37737991594e14214) )
	ROM_LOAD( "6.1f",   0x50000, 0x10000, CRC(db11300c) SHA1(e38eed8a54609f81d8eb3841a4cfaa684affd10a) )
	ROM_LOAD( "7.2f",   0x60000, 0x10000, CRC(57a90aac) SHA1(f0b42152cfe42e7d8e051382d1c7c92ef9b94b48) )
	ROM_LOAD( "8.4f",   0x70000, 0x10000, CRC(58e1ad6f) SHA1(a615e5303339cacc04153c73a85acf857e2823a8) )
	ROM_LOAD( "9.5f",   0x80000, 0x10000, CRC(242f07e9) SHA1(c6c65cd2c36502f1df6078e245f5bfae95eeaf9a) )
	ROM_LOAD( "10.7f",  0x90000, 0x10000, CRC(c032d8c3) SHA1(6a7f61e4c5b9cda9894410ea2df508a4ffd088e8) )
ROM_END

ROM_START( mcitylov )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "m14.12c", 0x00000, 0x08000, CRC(b0815b50) SHA1(499239ee63801fc14267ad253f4da1d9a961c802) )
	ROM_LOAD( "13.11c",  0x08000, 0x08000, CRC(52c7632b) SHA1(05b24612c52ed41195f2d98edbb5c12b0ef97b74) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "11.8c",   0x00000, 0x08000, CRC(eabb3f32) SHA1(e06426001e17c878ae35deb70a8155095bfeaa36) )
	ROM_LOAD( "12.10c",  0x08000, 0x08000, CRC(c280f573) SHA1(2a534335f08ec6886510ca92be6247e11804e706) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "m1.1h",   0x00000, 0x10000, CRC(94dd69ac) SHA1(6f1043752e24e3b874276e79ecc2a9cdbe751856) )
	ROM_LOAD( "m2.3h",   0x10000, 0x10000, CRC(206f4d2c) SHA1(53a05a92ded54e90eb59c55376434fa0203f96e9) )   // apparel/2.bin
	ROM_LOAD( "m3.4h",   0x20000, 0x10000, CRC(5d8a732b) SHA1(a478af0dc3b9042fcc9ce8338226d5a378b05491) )   // apparel/3.bin
	ROM_LOAD( "m4.5h",   0x30000, 0x10000, CRC(c40e4435) SHA1(b66c654a75be6759fa030e51b6484af7c37fca12) )   // apparel/4.bin
	ROM_LOAD( "m5.7h",   0x40000, 0x10000, CRC(e5bde704) SHA1(6fd0d5defe0d2072a1f9efd21e2a003b9212847b) )   // apparel/5.bin
	ROM_LOAD( "m6.1f",   0x50000, 0x10000, CRC(263673bc) SHA1(fa713101f2bf6874080c3c8db7cb55c9c084d502) )   // apparel/6.bin
	ROM_LOAD( "m7.3f",   0x60000, 0x10000, CRC(c502dc5a) SHA1(99ae8db3f06395ab5ca0828aad6e679090008aab) )   // apparel/7.bin
	ROM_LOAD( "m8.4f",   0x70000, 0x10000, CRC(c0af5f0f) SHA1(3e2c7c6a28540cd04366ff02d12ad566fd9d277d) )   // apparel/8.bin
	ROM_LOAD( "m9.5f",   0x80000, 0x10000, CRC(e2c6f70a) SHA1(6723ac5c954f9080cfd9e2d7e5d972d2e464f9b1) )
	ROM_LOAD( "m10.7f",  0x90000, 0x10000, CRC(3b2280c1) SHA1(5d0588d33b42e72cf7baf92f97a44333a3b7fb44) )
ROM_END

ROM_START( secolove )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "slov_08.bin", 0x00000, 0x08000, CRC(5aad556e) SHA1(f02e3014dab69598fd3a30deb66fe24932fd9665) )
	ROM_LOAD( "slov_07.bin", 0x08000, 0x08000, CRC(94175129) SHA1(1cb2dfdbfa42296a120e5160ee57da013a6f1b9c) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "slov_05.bin", 0x00000, 0x08000, CRC(fa1debd9) SHA1(173bdda2c21a9afdb21ef71db0a4b76bdd8289da) )
	ROM_LOAD( "slov_06.bin", 0x08000, 0x08000, CRC(a83be399) SHA1(06d6b98b4cbb6a751e03ddf993b0b392fb575793) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "slov_01.bin", 0x000000, 0x10000, CRC(9d792c34) SHA1(d027873129280706883ca3223c5709b063a7754f) )
	ROM_LOAD( "slov_02.bin", 0x010000, 0x10000, CRC(b9671c88) SHA1(df11ef26cbd7fcc68b39cf02ae9833e5b0111167) )
	ROM_LOAD( "slov_03.bin", 0x020000, 0x10000, CRC(5f57e4f2) SHA1(72674b3fce41fbec0f2e01601cbe934574c1cc37) )
	ROM_LOAD( "slov_04.bin", 0x030000, 0x10000, CRC(4b0c700c) SHA1(b671101ae45be6b043758b2c1c753c2931e57ae8) )
	ROM_LOAD( "slov_c1.bin", 0x100000, 0x80000, CRC(200170ba) SHA1(02033353cfd40613a5edc0d976a99b7f2ee44aec) )
	ROM_LOAD( "slov_c2.bin", 0x180000, 0x80000, CRC(dd5c23a1) SHA1(0b4faa33ea7b7855357f7805e80e81c368f79c3d) )
ROM_END

ROM_START( livegal )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4c",     0x00000, 0x08000, CRC(25f28dfb) SHA1(beb8c82e0d460f1d2a6583e4625628933bb46a01) )
	ROM_LOAD( "2.3c",     0x08000, 0x08000, CRC(4177cccf) SHA1(eeb8ed9b75893fe02a5556e9fc59a4c3f7a4d841) )

	ROM_REGION( 0x40000, "voice", 0 ) /* voice */
	ROM_LOAD( "3.5a",     0x00000, 0x10000, CRC(4fb5c4c4) SHA1(9a9199eeffdf212f68efbd5d11679ff5ad91c8ee) )

	ROM_REGION( 0x280000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "ic1i.bin", 0x000000, 0x40000, CRC(0263ff75) SHA1(16a18dfaf732ab94dec70fd8e955d6179525115c) )
	ROM_LOAD( "ic2i.bin", 0x040000, 0x40000, CRC(788cd3ca) SHA1(955a520e122aaee30e080d0a784556b69ba3de36) )
	ROM_LOAD( "ic3i.bin", 0x080000, 0x40000, CRC(a3175a8f) SHA1(8214fdefa1186dd96bc55a30b64a24a486750f05) )
	ROM_LOAD( "ic4i.bin", 0x0c0000, 0x40000, CRC(da46163e) SHA1(c6e5f59fe813915f94d81ff28526614c943b7082) )
	ROM_LOAD( "ic5i.bin", 0x100000, 0x40000, CRC(ea2b78b3) SHA1(38ec10a29f32cbb6b270fa10ade815cf3e0a54c2) )
	ROM_LOAD( "14.6i",    0x180000, 0x10000, CRC(671eac88) SHA1(64fc74e6eeeb0effd659f5ce20524e11c4271929) )
	ROM_LOAD( "15.7i",    0x190000, 0x10000, CRC(e8970858) SHA1(4f6f36d61547cd34e21ab75d0f5ed8e999d295b9) )
ROM_END

ROM_START( housemnq )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4c",   0x00000, 0x08000, CRC(465f61bb) SHA1(32e0931dd672ffaf5acf09d552f36e06162f8ef5) )
	ROM_LOAD( "2.3c",   0x08000, 0x08000, CRC(e4499d02) SHA1(737c4d9f5b93d351d80bd7c2d5a337d7d5b8bec9) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "3.5a",   0x00000, 0x10000, CRC(141ce8b9) SHA1(911243be0ab57fce113886c41d7413dd53fe12ec) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "1i.bin", 0x000000, 0x40000, CRC(2199e3e9) SHA1(965af4a29db4ff909dbeeebab1b828eb4f23f57e) )
	ROM_LOAD( "2i.bin", 0x040000, 0x40000, CRC(f730ea47) SHA1(f969fa85a91a337ba3fc89e9c458ef116088075e) )
	ROM_LOAD( "3i.bin", 0x080000, 0x40000, CRC(f85c5b07) SHA1(0fc55e9b60ccc630a0d77862eb5e64a3ba366947) )
	ROM_LOAD( "4i.bin", 0x0c0000, 0x40000, CRC(88f33049) SHA1(8b2d019b09ed854f40a8b0c7782645f50b1f2900) )
	ROM_LOAD( "5i.bin", 0x100000, 0x40000, CRC(77ba1eaf) SHA1(bde55b4d2938f44fd07ff7d5b5a845f2ea64b4fc) )
ROM_END

ROM_START( housemn2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "hmq2_01.bin", 0x00000, 0x08000, CRC(a5aaf6c8) SHA1(caf70d311cb5617eddcd6274cedefb05ae2a24a6) )
	ROM_LOAD( "hmq2_02.bin", 0x08000, 0x08000, CRC(6bdcc867) SHA1(bd9fe709c3bc42cf9142ab171a62460dbc6f5de0) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "hmq2_03.bin", 0x00000, 0x10000, CRC(c08081d8) SHA1(335e9fe25c076d159daed07c01d6d559691d5db3) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "hmq2_c5.bin", 0x000000, 0x40000, CRC(0263ff75) SHA1(16a18dfaf732ab94dec70fd8e955d6179525115c) )  // livegal/ic1i.bin
	ROM_LOAD( "hmq2_c1.bin", 0x040000, 0x40000, CRC(788cd3ca) SHA1(955a520e122aaee30e080d0a784556b69ba3de36) )  // livegal/ic2i.bin
	ROM_LOAD( "hmq2_c2.bin", 0x080000, 0x40000, CRC(a3175a8f) SHA1(8214fdefa1186dd96bc55a30b64a24a486750f05) )  // livegal/ic3i.bin
	ROM_LOAD( "hmq2_c3.bin", 0x0c0000, 0x40000, CRC(da46163e) SHA1(c6e5f59fe813915f94d81ff28526614c943b7082) )  // livegal/ic4i.bin
	ROM_LOAD( "hmq2_c4.bin", 0x100000, 0x40000, CRC(ea2b78b3) SHA1(38ec10a29f32cbb6b270fa10ade815cf3e0a54c2) )  // livegal/ic5i.bin
ROM_END

ROM_START( seiha )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "seiha1.4g",  0x00000, 0x08000, CRC(ad5ba5b5) SHA1(976fa8651000b1103302d38d323a3ffbd81d89e5) )
	ROM_LOAD( "seiha2.3g",  0x08000, 0x08000, CRC(0fe7a4b8) SHA1(5e702db804ba5a0bf38e6511f10a6704854d9aca) )

	ROM_REGION( 0x30000, "voice", 0 ) /* voice */
	ROM_LOAD( "seiha03.3i", 0x00000, 0x10000, CRC(2bcf3d87) SHA1(e768d112d7c314d1252c41793352bdca7a86f92e) )
	ROM_LOAD( "seiha04.2i", 0x10000, 0x10000, CRC(2fc905d0) SHA1(add824681979c2eba42b199280a99f7ea063b18e) )
	ROM_LOAD( "seiha05.1i", 0x20000, 0x10000, CRC(8eace19c) SHA1(bc715b17aa13e986dd7c6a8255bff3efdc4a8a01) )

	ROM_REGION( 0x280000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "seiha19.1a", 0x000000, 0x40000, CRC(788cd3ca) SHA1(955a520e122aaee30e080d0a784556b69ba3de36) )
	ROM_LOAD( "seiha20.2a", 0x040000, 0x40000, CRC(a3175a8f) SHA1(8214fdefa1186dd96bc55a30b64a24a486750f05) )
	ROM_LOAD( "seiha21.3a", 0x080000, 0x40000, CRC(da46163e) SHA1(c6e5f59fe813915f94d81ff28526614c943b7082) )
	ROM_LOAD( "seiha22.4a", 0x0c0000, 0x40000, CRC(ea2b78b3) SHA1(38ec10a29f32cbb6b270fa10ade815cf3e0a54c2) )
	ROM_LOAD( "seiha23.5a", 0x100000, 0x40000, CRC(0263ff75) SHA1(16a18dfaf732ab94dec70fd8e955d6179525115c) )
	ROM_LOAD( "seiha06.8a", 0x180000, 0x10000, CRC(9fefe2ca) SHA1(7b638a739640e9d311ee15c0e7b4f3f2dfdd3589) )
	ROM_LOAD( "seiha07.9a", 0x190000, 0x10000, CRC(a7d438ec) SHA1(5d145bab0ffc76fd77582ea5495ca4496210d41a) )
	ROM_LOAD( "se1507.6a",  0x200000, 0x80000, CRC(f1e9555e) SHA1(a34ffcff2b2d6ba40a8a453b89970d636515a8ad) )
ROM_END

ROM_START( seiham )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "seih_01m.bin", 0x00000, 0x08000, CRC(0c9a081b) SHA1(3cbc6aecca7e48099ea8e5a8092965150b8d3da0) )
	ROM_LOAD( "seih_02m.bin", 0x08000, 0x08000, CRC(a32cdb9a) SHA1(249efb16bd40a63d201a210b449b3121310ca600) )

	ROM_REGION( 0x30000, "voice", 0 ) /* voice */
	ROM_LOAD( "seiha03.3i",   0x00000, 0x10000, CRC(2bcf3d87) SHA1(e768d112d7c314d1252c41793352bdca7a86f92e) )  // seiah/seiha03.3i
	ROM_LOAD( "seiha04.2i",   0x10000, 0x10000, CRC(2fc905d0) SHA1(add824681979c2eba42b199280a99f7ea063b18e) )  // seiha/seiha04.2i
	ROM_LOAD( "seiha05.1i",   0x20000, 0x10000, CRC(8eace19c) SHA1(bc715b17aa13e986dd7c6a8255bff3efdc4a8a01) )  // seiha/seiha05.1i

	ROM_REGION( 0x280000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "seiha19.1a",   0x000000, 0x40000, CRC(788cd3ca) SHA1(955a520e122aaee30e080d0a784556b69ba3de36) ) // seiha/seiha19.1a
	ROM_LOAD( "seiha20.2a",   0x040000, 0x40000, CRC(a3175a8f) SHA1(8214fdefa1186dd96bc55a30b64a24a486750f05) ) // seiha/seiha20.2a
	ROM_LOAD( "seiha21.3a",   0x080000, 0x40000, CRC(da46163e) SHA1(c6e5f59fe813915f94d81ff28526614c943b7082) ) // seiha/seiha21.3a
	ROM_LOAD( "seiha22.4a",   0x0c0000, 0x40000, CRC(ea2b78b3) SHA1(38ec10a29f32cbb6b270fa10ade815cf3e0a54c2) ) // seiha/seiha22.4a
	ROM_LOAD( "seiha23.5a",   0x100000, 0x40000, CRC(0263ff75) SHA1(16a18dfaf732ab94dec70fd8e955d6179525115c) ) // seiha/seiha23.5a
	ROM_LOAD( "seiha06.8a",   0x180000, 0x10000, CRC(9fefe2ca) SHA1(7b638a739640e9d311ee15c0e7b4f3f2dfdd3589) ) // seiha/seiha06.8a
	ROM_LOAD( "seiha07.9a",   0x190000, 0x10000, CRC(a7d438ec) SHA1(5d145bab0ffc76fd77582ea5495ca4496210d41a) ) // seiha/seiha07.9a
	ROM_LOAD( "seih_08m.bin", 0x1a0000, 0x10000, CRC(e8e61e48) SHA1(e1d0e64b39bad3e294b061fb6f02ece2f2ee4bca) )
	ROM_LOAD( "se1507.6a",    0x200000, 0x80000, CRC(f1e9555e) SHA1(a34ffcff2b2d6ba40a8a453b89970d636515a8ad) ) // seiha/se1507.6a
ROM_END

/*
Mahjong Gaiden
(c)1987 Central Denshi

CPU: Z80B
Sound: AY-3-8910
OSC: 5.000MHz
Custom: 1413M3

ROMs:
1.4G
2.3G
3.3I
4.2I
5.1I
W19.1A
W20.2A
W21.3A
W22.4A
W23.5A

Subboard
6.2A
7.3A
8.4A
9.2B
10.3B


dumped by sayu
--- Team Japump!!! ---

*/

/*
Is this a hack of Seiha or an officially licensed game? There are Seiha references in various places plus
it shares some gfx roms...
*/
ROM_START( mjgaiden )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4g",      0x00000, 0x08000, CRC(6f54ab3d) SHA1(08fe565616de2e06141407c56b6de23014cfc56c) )
	ROM_LOAD( "2.3g",      0x08000, 0x08000, CRC(b4fed864) SHA1(a48300e586cb160fff903fb4203ee66418a81b3d) )

	ROM_REGION( 0x40000, "voice", 0 ) /* voice */
	ROM_LOAD( "3.3i",   0x00000, 0x10000, CRC(2bcf3d87) SHA1(e768d112d7c314d1252c41793352bdca7a86f92e) )
	ROM_LOAD( "4.2i",   0x10000, 0x10000, CRC(2fc905d0) SHA1(add824681979c2eba42b199280a99f7ea063b18e) )

	/*TODO: check if the w labeled roms are correctly mapped.*/
	ROM_REGION( 0x400000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "se1507.6a",0x000000, 0x80000, CRC(f1e9555e) SHA1(a34ffcff2b2d6ba40a8a453b89970d636515a8ad) ) // seiha/se1507.6a
	ROM_LOAD( "w19.1a",   0x080000, 0x40000, CRC(788cd3ca) SHA1(955a520e122aaee30e080d0a784556b69ba3de36) )
	ROM_LOAD( "w20.2a",   0x0c0000, 0x40000, CRC(a3175a8f) SHA1(8214fdefa1186dd96bc55a30b64a24a486750f05) )
	ROM_LOAD( "w21.3a",   0x100000, 0x40000, CRC(da46163e) SHA1(c6e5f59fe813915f94d81ff28526614c943b7082) )
	ROM_LOAD( "6.2a",     0x180000, 0x10000, CRC(9fefe2ca) SHA1(7b638a739640e9d311ee15c0e7b4f3f2dfdd3589) ) // seiha/seiha06.8a
	ROM_LOAD( "7.3a",     0x190000, 0x10000, CRC(a7d438ec) SHA1(5d145bab0ffc76fd77582ea5495ca4496210d41a) ) // seiha/seiha07.9a
	ROM_LOAD( "8.4a",     0x1a0000, 0x10000, CRC(e8e61e48) SHA1(e1d0e64b39bad3e294b061fb6f02ece2f2ee4bca) )
	ROM_LOAD( "9.2b",     0x1b0000, 0x10000, CRC(541f6e9f) SHA1(946a9c9cc8e6985098af4dd035f80ecc50e800ec) ) // seiha/seiha05.1i
	ROM_LOAD( "10.3b",    0x1c0000, 0x10000, CRC(a4144f78) SHA1(316ebe91aa604f1d4a0f1942df9d87de487c977a) )
	ROM_LOAD( "w22.4a",   0x200000, 0x40000, CRC(ea2b78b3) SHA1(38ec10a29f32cbb6b270fa10ade815cf3e0a54c2) )
	ROM_LOAD( "w23.5a",   0x240000, 0x40000, CRC(0263ff75) SHA1(16a18dfaf732ab94dec70fd8e955d6179525115c) )
ROM_END

ROM_START( bijokkoy )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4c",   0x00000, 0x08000, CRC(7dec7ae1) SHA1(fd16a33342014b1c7400e278755ae68248134129) )
	ROM_LOAD( "2.3c",   0x08000, 0x08000, CRC(3ae9650f) SHA1(c2a5b0114b7d147ecd8e3760ee45fdf25fc32c14) )

	ROM_REGION( 0x40000, "voice", 0 ) /* voice */
	ROM_LOAD( "3.ic1",  0x00000, 0x10000, CRC(221743b1) SHA1(66a1d3e9c6019b88dd073a04b6c3ac480584bd72) )
	ROM_LOAD( "4.ic2",  0x10000, 0x10000, CRC(9f1f4461) SHA1(eb64b9d78c6a4d933d22031d6c32c683ddeacd8f) )
	ROM_LOAD( "5.ic3",  0x20000, 0x10000, CRC(6e7b3024) SHA1(366de01cad2dacc7d1a611948cc82545a621b645) )
	ROM_LOAD( "6.ic4",  0x30000, 0x10000, CRC(5e912211) SHA1(a5a26424c3720277bcbd4169d2bda9fa82c76bd2) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "1h.bin", 0x000000, 0x40000, CRC(da56ccac) SHA1(d317c5fafed6a28c032f2ce08c20a7b7ac1922ce) )
	ROM_LOAD( "2h.bin", 0x040000, 0x40000, CRC(21c0227a) SHA1(5ad0adb2f5fd975ed4ed1c7969e32b260a87201c) )
	ROM_LOAD( "3h.bin", 0x080000, 0x40000, CRC(aa66d9f3) SHA1(d2d0870da1d523d3b8c64d2f24077ec90a845468) )
	ROM_LOAD( "4h.bin", 0x0c0000, 0x40000, CRC(5d10fb0a) SHA1(858cc4db96ecf55d9a8a436bd9a20f7eeec9130b) )
	ROM_LOAD( "5h.bin", 0x100000, 0x40000, CRC(e22d6ca8) SHA1(653339063c0fb3a2ad49a4381c40ccece1534467) )
ROM_END

ROM_START( iemoto )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "iemoto1.4g",  0x00000, 0x08000, CRC(ab51f5c3) SHA1(5b2a6be6ac3d1409523ce495605f906b47128f65) )
	ROM_LOAD( "iemoto2.3g",  0x08000, 0x08000, CRC(873cd265) SHA1(283fa86d916a7ad08dfdcd4b9592d36d48b62ede) )

	ROM_REGION( 0x30000, "voice", 0 ) /* voice */
	ROM_LOAD( "iemoto3.3i",  0x00000, 0x10000, CRC(32d71ff9) SHA1(eefe53c6ad95d5e1f116162bbb30f8ec7e7ad005) )
	ROM_LOAD( "iemoto4.2i",  0x10000, 0x10000, CRC(06f8e505) SHA1(dfca4999df2c9cb98204e3d2c3bd37ea561f3604) )
	ROM_LOAD( "iemoto5.1i",  0x20000, 0x10000, CRC(261eb61a) SHA1(8d03a190b9a5fbda318e475f64d0c94a3d4ed362) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "iemoto31.1a", 0x000000, 0x40000, CRC(ba005a3a) SHA1(305041f764b5ba9ffa882c1a69555a38a53b1556) )
	ROM_LOAD( "iemoto32.2a", 0x040000, 0x40000, CRC(fa9a74ae) SHA1(08dd0cd07aeb8d77152e93c76db44e9034aa3954) )
	ROM_LOAD( "iemoto33.3a", 0x080000, 0x40000, CRC(efb13b61) SHA1(61d100b52d01e447dd599cc9ff06b97dd7a4ae0b) )
	ROM_LOAD( "iemoto44.4a", 0x0c0000, 0x40000, CRC(9acc54fa) SHA1(7975370e1dd32ecd98d7f2e32f14feb88e0cdb43) )
ROM_END

ROM_START( iemotom )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4g",        0x00000, 0x08000, CRC(c4107170) SHA1(92ab01e91a7cbd4ac5d74352c9630aa918cbe6b2) )
	ROM_LOAD( "2.3g",        0x08000, 0x08000, CRC(6778cf82) SHA1(f3eec7dcda00ebf5097df0111908029337a15032) )

	ROM_REGION( 0x30000, "voice", 0 ) /* voice */
	ROM_LOAD( "3.3i",        0x00000, 0x10000, CRC(32d71ff9) SHA1(eefe53c6ad95d5e1f116162bbb30f8ec7e7ad005) )   // iemoto/iemoto3.3i
	ROM_LOAD( "4.2i",        0x10000, 0x10000, CRC(06f8e505) SHA1(dfca4999df2c9cb98204e3d2c3bd37ea561f3604) )   // iemoto/iemoto4.2i
	ROM_LOAD( "5.1i",        0x20000, 0x10000, CRC(261eb61a) SHA1(8d03a190b9a5fbda318e475f64d0c94a3d4ed362) )   // iemoto/iemoto5.1i

	ROM_REGION( 0x120000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "iemoto31.1a", 0x000000, 0x40000, CRC(ba005a3a) SHA1(305041f764b5ba9ffa882c1a69555a38a53b1556) )  // iemoto/iemoto31.1a
	ROM_LOAD( "iemoto32.2a", 0x040000, 0x40000, CRC(fa9a74ae) SHA1(08dd0cd07aeb8d77152e93c76db44e9034aa3954) )  // iemoto/iemoto32.2a
	ROM_LOAD( "iemoto33.3a", 0x080000, 0x40000, CRC(efb13b61) SHA1(61d100b52d01e447dd599cc9ff06b97dd7a4ae0b) )  // iemoto/iemoto33.3a
	ROM_LOAD( "iemoto44.4a", 0x0c0000, 0x40000, CRC(9acc54fa) SHA1(7975370e1dd32ecd98d7f2e32f14feb88e0cdb43) )  // iemoto/iemoto44.4a
	ROM_LOAD( "6.6a",        0x110000, 0x10000, CRC(9eae7c9e) SHA1(dbc6c8b31f6e484078d880914b96133a41cd3f14) )
ROM_END

ROM_START( ryuuha )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4ga",       0x00000, 0x08000, CRC(2f995640) SHA1(f95d7cd6b7598c263011a61c58451f6615c81966) )
	ROM_LOAD( "2.3ga",        0x08000, 0x08000, CRC(0787d707) SHA1(4d0df545cc3892690593216afbee4a5529afddfe) )

	ROM_REGION( 0x30000, "voice", 0 ) /* voice */
	ROM_LOAD( "3.3i",        0x00000, 0x10000, CRC(32d71ff9) SHA1(eefe53c6ad95d5e1f116162bbb30f8ec7e7ad005) )   // iemoto/iemoto3.3i
	ROM_LOAD( "4.2i",        0x10000, 0x10000, CRC(06f8e505) SHA1(dfca4999df2c9cb98204e3d2c3bd37ea561f3604) )   // iemoto/iemoto4.2i
	ROM_LOAD( "5.1i",        0x20000, 0x10000, CRC(261eb61a) SHA1(8d03a190b9a5fbda318e475f64d0c94a3d4ed362) )   // iemoto/iemoto5.1i

	ROM_REGION( 0x120000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "iemoto31.1a", 0x000000, 0x40000, CRC(ba005a3a) SHA1(305041f764b5ba9ffa882c1a69555a38a53b1556) )  // iemoto/iemoto31.1a
	ROM_LOAD( "iemoto32.2a", 0x040000, 0x40000, CRC(fa9a74ae) SHA1(08dd0cd07aeb8d77152e93c76db44e9034aa3954) )  // iemoto/iemoto32.2a
	ROM_LOAD( "iemoto33.3a", 0x080000, 0x40000, CRC(efb13b61) SHA1(61d100b52d01e447dd599cc9ff06b97dd7a4ae0b) )  // iemoto/iemoto33.3a
	ROM_LOAD( "iemoto44.4a", 0x0c0000, 0x40000, CRC(9acc54fa) SHA1(7975370e1dd32ecd98d7f2e32f14feb88e0cdb43) )  // iemoto/iemoto44.4a
	ROM_LOAD( "6.5a",        0x100000, 0x10000, CRC(48710ae3) SHA1(d32fa1de44390d84f4c3141b70034119ca79a19b) )
	ROM_LOAD( "7.6a" ,       0x110000, 0x10000, CRC(9eae7c9e) SHA1(dbc6c8b31f6e484078d880914b96133a41cd3f14) )  // iemotom/6.6a
ROM_END

ROM_START( ojousan )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4g",    0x00000, 0x08000, CRC(c0166351) SHA1(656f5ac0846d9e7ee5059947597c5863bb213165) )
	ROM_LOAD( "2.3g",    0x08000, 0x08000, CRC(2c264eb2) SHA1(eb94a1ef88c499d2f57881d886a206b599441698) )

	ROM_REGION( 0x30000, "voice", 0 ) /* voice */
	ROM_LOAD( "mask.3i", 0x00000, 0x10000, CRC(59f355eb) SHA1(24826f5a89d8dfc64bc327982b4e9b5afd43368e) )
	ROM_LOAD( "mask.2i", 0x10000, 0x10000, CRC(6f750500) SHA1(0f958cfb1f3c1846f2f7ae94465b54207c7312ac) )
	ROM_LOAD( "mask.1i", 0x20000, 0x10000, CRC(4babcb40) SHA1(4940a9ef210ea2128d562564f251078fc6e28bed) )

	ROM_REGION( 0x1c0000, "gfx1", 0 ) /* gfx */
	/* 000000-0fffff empty */
	ROM_LOAD( "3.5a",    0x100000, 0x20000, CRC(3bdb9d2a) SHA1(01dbe293c455256d82207bd4eed389c118df510b) )
	ROM_LOAD( "4.6a",    0x120000, 0x20000, CRC(72b689b9) SHA1(cb3cd6d17ea367dd10cf178a061f709c12bef5cd) )
	ROM_LOAD( "5.7a",    0x140000, 0x20000, CRC(e32e5e8a) SHA1(7e1bd1b4c2c30ec30b784b824e05123909360287) )
	ROM_LOAD( "6.8a",    0x160000, 0x20000, CRC(f313337a) SHA1(374ed1a6da4ab3143f2f25dbb35930e6595dc32a) )
	ROM_LOAD( "7.9a",    0x180000, 0x20000, CRC(c2428e95) SHA1(4bd35e3f0c6c7dece18e168d2c6261e64c051569) )
	ROM_LOAD( "8.10a",   0x1a0000, 0x20000, CRC(f04c6003) SHA1(2fd81cc1e1f91630ef5767ba20be3fac9e131370) )
ROM_END

ROM_START( ojousanm )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4ga", 0x00000, 0x08000, CRC(ba1d08dd) SHA1(a1c46a0027b4294e50afd48a2389222ef06cd903) )
	ROM_LOAD( "2.3ga", 0x08000, 0x08000, CRC(26a093fa) SHA1(b5bdc9b5f21655e8fe47c09c0bb3bb211d555f52) )

	ROM_REGION( 0x30000, "voice", 0 ) /* voice */
	ROM_LOAD( "3.3i", 0x00000, 0x10000, CRC(59f355eb) SHA1(24826f5a89d8dfc64bc327982b4e9b5afd43368e) )  // ojousan/mask.3i
	ROM_LOAD( "4.2i", 0x10000, 0x10000, CRC(6f750500) SHA1(0f958cfb1f3c1846f2f7ae94465b54207c7312ac) )  // ojousan/mask.2i
	ROM_LOAD( "5.1i", 0x20000, 0x10000, CRC(4babcb40) SHA1(4940a9ef210ea2128d562564f251078fc6e28bed) )  // ojousan/mask.1i

	ROM_REGION( 0x0c0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "3.2a", 0x000000, 0x20000, CRC(5da11cc3) SHA1(95814d722d95a7ad7da53ac7de046470a176cda3) )
	ROM_LOAD( "4.3a", 0x020000, 0x20000, CRC(d1cf2096) SHA1(b241eb0369a4d4101ccad894c48f8024ba288fe5) )
	ROM_LOAD( "5.4a", 0x040000, 0x20000, CRC(935c765f) SHA1(fc3148e3ba354bf733a0962795122fda74e2ebfe) )
	ROM_LOAD( "6.5a", 0x060000, 0x20000, CRC(57b6906c) SHA1(e808bef5a47361723a155b60dedb2f613df5e455) )
	ROM_LOAD( "7.6a", 0x080000, 0x20000, CRC(c2428e95) SHA1(4bd35e3f0c6c7dece18e168d2c6261e64c051569) ) // ojousan/7.9a
	ROM_LOAD( "8.7a", 0x0a0000, 0x20000, CRC(f04c6003) SHA1(2fd81cc1e1f91630ef5767ba20be3fac9e131370) ) // ojousan/8.10a
ROM_END

ROM_START( swinggal )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4g",    0x00000, 0x08000, CRC(e0e94bb9) SHA1(f514993614376c2175e906eab2776567e0c32f67) )
	ROM_LOAD( "2.3g",    0x08000, 0x08000, CRC(577d786e) SHA1(e47e78d8607be0e41c2b220eb367f67e5e19c481) )

	ROM_REGION( 0x30000, "voice", 0 ) /* voice */
	ROM_LOAD( "3i", 0x00000, 0x10000, CRC(b15ac0c4) SHA1(88c7a0be533465fc9c6833e52bcb1729c2307d06) )
	ROM_LOAD( "2i", 0x10000, 0x10000, CRC(ff44d460) SHA1(cd070b6b48cfafa67e9fe778a2a31e623caa3ddd) )
	ROM_LOAD( "1i", 0x20000, 0x10000, CRC(05294295) SHA1(bb2114dccc94545664c10a9e32ebd87eed59865c) )

	ROM_REGION( 0x1c0000, "gfx1", 0 ) /* gfx */
	/* 000000-0fffff empty */
	ROM_LOAD( "3.5a",    0x100000, 0x20000, CRC(c1f49332) SHA1(35e4db593ab54b0ab6734a377cc2ed1c8670314c) )
	ROM_LOAD( "4.6a",    0x120000, 0x20000, CRC(f9bac001) SHA1(3aa707c75c27b968303bb8bc007e5da872a807f5) )
	ROM_LOAD( "5.7a",    0x140000, 0x20000, CRC(d9ec3418) SHA1(0d29d800a821b6806dc6b9d5a7aaa0939346ddc4) )
	ROM_LOAD( "6.8a",    0x160000, 0x20000, CRC(5a966741) SHA1(c122ada315f09b0851947e9efd67e0a11663cf66) )
	ROM_LOAD( "7.9a",    0x180000, 0x20000, CRC(93abe5c5) SHA1(d71da2ded70726af0aa1ee751fbab758326447ab) )
	ROM_LOAD( "8.10a",   0x1a0000, 0x20000, CRC(010d353d) SHA1(5ea7fd6f26f6ad484bb6e07e66e8c25d178ac037) )
ROM_END

ROM_START( bijokkog )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4c",    0x00000, 0x08000, CRC(3c28b45c) SHA1(289582f607322b878b4737325498e749f5460586) )
	ROM_LOAD( "2.3c",    0x08000, 0x08000, CRC(396f6a05) SHA1(f983b34b2b782631f9913a85f933ec0c504f4047) )

	ROM_REGION( 0x40000, "voice", 0 ) /* voice */
	ROM_LOAD( "3.ic1",   0x00000, 0x10000, CRC(a92b1445) SHA1(97315ee16c7f1becb939c93ed671e0a0cf8dfa03) )
	ROM_LOAD( "4.ic2",   0x10000, 0x10000, CRC(5127e958) SHA1(962aee0a997e24bbfd3ec732f8ef6a49638a1a32) )
	ROM_LOAD( "5.ic3",   0x20000, 0x10000, CRC(6c717330) SHA1(17c08e7a0e42af002a73110907b3677273cab276) )
	ROM_LOAD( "6.ic4",   0x30000, 0x10000, CRC(a3cf8d12) SHA1(dbfa5898cec3168c49f361219ca6b9090d455e8f) )

	ROM_REGION( 0x0c0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "1s.bin",  0x000000, 0x10000, CRC(9eadc3ea) SHA1(b40d354e7e85e4716d90ede68bbf01d0b3fdac32) )
	ROM_LOAD( "2s.bin",  0x010000, 0x10000, CRC(1161484c) SHA1(b6835814117b7077ab05507f6fde6203f392947a) )
	ROM_LOAD( "3s.bin",  0x020000, 0x10000, CRC(41f5dc43) SHA1(eb878f5becd5333c9accaf1a29cefa1dae0e02da) )
	ROM_LOAD( "4s.bin",  0x030000, 0x10000, CRC(3d9b79db) SHA1(42637e2bc8c36547f33ce3eceb4e2880f8f394fe) )
	ROM_LOAD( "5s.bin",  0x040000, 0x10000, CRC(eb54c3e3) SHA1(436ae251cd398ca8a3bf16ece4d195dfb7c4e44c) )
	ROM_LOAD( "6s.bin",  0x050000, 0x10000, CRC(d8deeea2) SHA1(27d429ec19beae8a287d7e7bd05326a74be68232) )
	ROM_LOAD( "7s.bin",  0x060000, 0x10000, CRC(e42c67f1) SHA1(ac336aa3cfd5fc122089241d0ecd860a8271590c) )
	ROM_LOAD( "8s.bin",  0x070000, 0x10000, CRC(cd11c78a) SHA1(a97f8b7bc3f8e55896299a873cd5e8fee6d233e4) )
	ROM_LOAD( "9s.bin",  0x080000, 0x10000, CRC(2f3453a1) SHA1(741bae271a497165ab968625fc1a6dcc98a832c4) )
	ROM_LOAD( "10s.bin", 0x090000, 0x10000, CRC(d80dd0b4) SHA1(fd55d760aed97891c614cef33430f9bc78109f1a) )
	ROM_LOAD( "11s.bin", 0x0a0000, 0x10000, CRC(ade64867) SHA1(c016ce653181bb0403c13ca0dfd1ebf7dfa59a7d) )
	ROM_LOAD( "12s.bin", 0x0b0000, 0x10000, CRC(918a8f36) SHA1(277fd79938c3c8a3114c2ec9c46aa04f4abb34c9) )
ROM_END

ROM_START( orangec )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "ft2.3c",   0x00000, 0x08000, CRC(4ed413aa) SHA1(7cfa3a2efa41b60e261e5cc2e58736b97957e819) )
	ROM_LOAD( "ft1.2c",   0x08000, 0x08000, CRC(f26bfd1b) SHA1(a34352d5dc3f41ee6c4ca480a0e501e8c0b82766) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "ft3.5c",   0x00000, 0x10000, CRC(2390a28b) SHA1(7bced9e7680d0cc98e30ab82da1c4ab0c4ef37b4) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "ic5.bin",  0x000000, 0x10000, CRC(e6fe4540) SHA1(00625ea017305b2622ac31ad2e6e4c928ee0cfcd) )
	ROM_LOAD( "ic6.bin",  0x010000, 0x10000, CRC(343664f4) SHA1(fb817f5c2174c823b3aedc806c63338bd97b8346) )
	ROM_LOAD( "ic7.bin",  0x020000, 0x10000, CRC(5d5bcba8) SHA1(f1e19d64185deb12560b54d27eca2aa52a6dc1f2) )
	ROM_LOAD( "ic8.bin",  0x030000, 0x10000, CRC(80ec6473) SHA1(b2c2b146470ee9ec6914c9b0c7d36539bb6e6536) )
	ROM_LOAD( "ic9.bin",  0x040000, 0x10000, CRC(30648437) SHA1(521c20c648720cffe334b4168aebaca8f8863242) )
	ROM_LOAD( "ic10.bin", 0x050000, 0x10000, CRC(30e74967) SHA1(824ddc3a8c91517f3ec8a6386226eb548a1d4b39) )
	ROM_LOAD( "ic1.bin",  0x100000, 0x40000, CRC(a3175a8f) SHA1(8214fdefa1186dd96bc55a30b64a24a486750f05) )
	ROM_LOAD( "ic2.bin",  0x140000, 0x40000, CRC(da46163e) SHA1(c6e5f59fe813915f94d81ff28526614c943b7082) )
	ROM_LOAD( "ic3.bin",  0x180000, 0x40000, CRC(efb13b61) SHA1(61d100b52d01e447dd599cc9ff06b97dd7a4ae0b) )
	ROM_LOAD( "ic4.bin",  0x1c0000, 0x40000, CRC(9acc54fa) SHA1(7975370e1dd32ecd98d7f2e32f14feb88e0cdb43) )
	ROM_LOAD( "ic6i.bin", 0x0f0000, 0x10000, CRC(94bf4847) SHA1(a1ff0a5b1918b9f1a0f608ad341d091512988c1a) ) // orangec/ic6i.bin
	ROM_LOAD( "ic7i.bin", 0x100000, 0x10000, CRC(284f5648) SHA1(f0a734744901313f5052ea1727815e11a93e1811) ) // orangec/ic7i.bin overlaps ic1!
	ROM_LOAD( "ic7i.bin", 0x110000, 0x10000, CRC(284f5648) SHA1(f0a734744901313f5052ea1727815e11a93e1811) ) // orangec/ic7i.bin overlaps ic1!
ROM_END

ROM_START( orangeci )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "2.bin",    0x00000, 0x08000, CRC(8599bf78) SHA1(1c2c14205dcd2fc0160d31c0839404168b59ee3f) )
	ROM_LOAD( "1.bin",    0x08000, 0x08000, CRC(adc9b0ab) SHA1(0fbb7b419f645b4715407e45c8e564b7bf334a9d) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "ft3.5c",   0x00000, 0x10000, CRC(2390a28b) SHA1(7bced9e7680d0cc98e30ab82da1c4ab0c4ef37b4) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "ic5.bin",  0x000000, 0x10000, CRC(e6fe4540) SHA1(00625ea017305b2622ac31ad2e6e4c928ee0cfcd) )
	ROM_LOAD( "ic6.bin",  0x010000, 0x10000, CRC(343664f4) SHA1(fb817f5c2174c823b3aedc806c63338bd97b8346) )
	ROM_LOAD( "ic7.bin",  0x020000, 0x10000, CRC(5d5bcba8) SHA1(f1e19d64185deb12560b54d27eca2aa52a6dc1f2) )
	ROM_LOAD( "ic8.bin",  0x030000, 0x10000, CRC(80ec6473) SHA1(b2c2b146470ee9ec6914c9b0c7d36539bb6e6536) )
	ROM_LOAD( "ic9.bin",  0x040000, 0x10000, CRC(30648437) SHA1(521c20c648720cffe334b4168aebaca8f8863242) )
	ROM_LOAD( "ic10.bin", 0x050000, 0x10000, CRC(30e74967) SHA1(824ddc3a8c91517f3ec8a6386226eb548a1d4b39) )
	ROM_LOAD( "ic1.bin",  0x100000, 0x40000, CRC(a3175a8f) SHA1(8214fdefa1186dd96bc55a30b64a24a486750f05) )
	ROM_LOAD( "ic2.bin",  0x140000, 0x40000, CRC(da46163e) SHA1(c6e5f59fe813915f94d81ff28526614c943b7082) )
	ROM_LOAD( "ic3.bin",  0x180000, 0x40000, CRC(efb13b61) SHA1(61d100b52d01e447dd599cc9ff06b97dd7a4ae0b) )
	ROM_LOAD( "ic4.bin",  0x1c0000, 0x40000, CRC(9acc54fa) SHA1(7975370e1dd32ecd98d7f2e32f14feb88e0cdb43) )
	ROM_LOAD( "ic6i.bin", 0x0f0000, 0x10000, CRC(94bf4847) SHA1(a1ff0a5b1918b9f1a0f608ad341d091512988c1a) ) // orangec/ic6i.bin
	ROM_LOAD( "ic7i.bin", 0x100000, 0x10000, CRC(284f5648) SHA1(f0a734744901313f5052ea1727815e11a93e1811) ) // orangec/ic7i.bin overlaps ic1!
	ROM_LOAD( "ic7i.bin", 0x110000, 0x10000, CRC(284f5648) SHA1(f0a734744901313f5052ea1727815e11a93e1811) ) // orangec/ic7i.bin overlaps ic1!
ROM_END

ROM_START( vipclub )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "2.3c",     0x00000, 0x08000, CRC(49acc59a) SHA1(a1e65e2804fde817c4e6f1e9b0949a4cbe537a6d) )
	ROM_LOAD( "1.2c",     0x08000, 0x08000, CRC(42101925) SHA1(b3e1b4a3c905e0c5ad85fd1276b221440937719e) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "ft3.5c",   0x00000, 0x10000, CRC(2390a28b) SHA1(7bced9e7680d0cc98e30ab82da1c4ab0c4ef37b4) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "ic5.bin",  0x000000, 0x10000, CRC(e6fe4540) SHA1(00625ea017305b2622ac31ad2e6e4c928ee0cfcd) ) // orangec/ic5.bin
	ROM_LOAD( "ic6.bin",  0x010000, 0x10000, CRC(343664f4) SHA1(fb817f5c2174c823b3aedc806c63338bd97b8346) ) // orangec/ic6.bin
	ROM_LOAD( "ic7_bin",  0x020000, 0x10000, CRC(4811e122) SHA1(1f40c5ef94732554e458ccdbae847fff5aa3b316) )
	ROM_LOAD( "ic8.bin",  0x030000, 0x10000, CRC(80ec6473) SHA1(b2c2b146470ee9ec6914c9b0c7d36539bb6e6536) ) // orangec/ic8.bin
	ROM_LOAD( "ic9.bin",  0x040000, 0x10000, CRC(30648437) SHA1(521c20c648720cffe334b4168aebaca8f8863242) ) // orangec/ic9.bin
	ROM_LOAD( "ic10.bin", 0x050000, 0x10000, CRC(30e74967) SHA1(824ddc3a8c91517f3ec8a6386226eb548a1d4b39) ) // orangec/10.bin
	ROM_LOAD( "ic1.bin",  0x100000, 0x40000, CRC(a3175a8f) SHA1(8214fdefa1186dd96bc55a30b64a24a486750f05) ) // orangec/ic1.bin
	ROM_LOAD( "ic2.bin",  0x140000, 0x40000, CRC(da46163e) SHA1(c6e5f59fe813915f94d81ff28526614c943b7082) ) // orangec/ic2.bin
	ROM_LOAD( "ic3.bin",  0x180000, 0x40000, CRC(efb13b61) SHA1(61d100b52d01e447dd599cc9ff06b97dd7a4ae0b) ) // orangec/ic3.bin
	ROM_LOAD( "ic4.bin",  0x1c0000, 0x40000, CRC(9acc54fa) SHA1(7975370e1dd32ecd98d7f2e32f14feb88e0cdb43) ) // orangec/ic4.bin
	ROM_LOAD( "ic6i.bin", 0x0f0000, 0x10000, CRC(94bf4847) SHA1(a1ff0a5b1918b9f1a0f608ad341d091512988c1a) ) // orangec/ic6i.bin
	ROM_LOAD( "ic7i.bin", 0x100000, 0x10000, CRC(284f5648) SHA1(f0a734744901313f5052ea1727815e11a93e1811) ) // orangec/ic7i.bin overlaps ic1!
	ROM_LOAD( "ic7i.bin", 0x110000, 0x10000, CRC(284f5648) SHA1(f0a734744901313f5052ea1727815e11a93e1811) ) // orangec/ic7i.bin overlaps ic1!
ROM_END

ROM_START( korinai )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4g",       0x00000, 0x08000, CRC(ddcf787c) SHA1(d73d274a0ae87515e5943b112d85e8d02117c4ea) )
	ROM_LOAD( "2.3g",       0x08000, 0x08000, CRC(9bb992f5) SHA1(912daad2ae19e639b5064544bc7b31f8d36862a3) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "3.3i",       0x00000, 0x10000, CRC(d6fb023f) SHA1(cca290cdbcedee5222788fb33568238bc66c29af) )
	ROM_LOAD( "4.2i",       0x10000, 0x10000, CRC(460917cf) SHA1(c845a012f7a8758cf7bbfc01a95780d1dd7d48b4) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "5.2a",       0x000000, 0x20000, CRC(f0732f3e) SHA1(b7a1fb00452d076ae5f91e1d3c2986ef156c84e1) )
	ROM_LOAD( "6.3a",       0x020000, 0x20000, CRC(2b1da51e) SHA1(7083e9ee8eb82d1f562aba2183c8d532d73c3686) )
	ROM_LOAD( "7.4a",       0x040000, 0x20000, CRC(85c260b9) SHA1(d1813329c66419bb4f19d9bba948d6662fc1142b) )
	ROM_LOAD( "8.5a",       0x060000, 0x20000, CRC(6a2763e1) SHA1(0718238a7b1cd3b1409824355f04f3c4ba73a8d5) )
	ROM_LOAD( "9.6a",       0x080000, 0x20000, CRC(81287588) SHA1(57784e8b62df68963592cffe0028dcc0118d44fd) )
	ROM_LOAD( "10.7a",      0x0a0000, 0x20000, CRC(9506d9cc) SHA1(8312e595b176ec43f0e77c26be165416ba43da4d) )
	ROM_LOAD( "11.8a",      0x0c0000, 0x20000, CRC(680d882e) SHA1(3505c9b530fd388e35467fdc0e31d125332fbc00) )
	ROM_LOAD( "12.9a",      0x0e0000, 0x20000, CRC(41a25dfe) SHA1(a71db0d896665f1943b92b7fa3c7ad9cd7ad8653) )
	ROM_LOAD( "13.10a",     0x100000, 0x10000, CRC(7dc27aa9) SHA1(06e741c0949398b57ad85d0460391e5d43f68182) )
	ROM_LOAD( "se-1507.1a", 0x180000, 0x80000, CRC(f1e9555e) SHA1(a34ffcff2b2d6ba40a8a453b89970d636515a8ad) )
ROM_END

ROM_START( korinaim )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "11.4g",      0x00000, 0x08000, CRC(23664cdc) SHA1(66992910df215578bb48d4678595251db4db3191) )
	ROM_LOAD( "12.3g",      0x08000, 0x08000, CRC(10cf7144) SHA1(467fb8f11266cc0add5beb6faf2f7b7bc8fadc17) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "3.3i",       0x00000, 0x10000, CRC(d6fb023f) SHA1(cca290cdbcedee5222788fb33568238bc66c29af) )    // korinai/3.3i
	ROM_LOAD( "4.2i",       0x10000, 0x10000, CRC(460917cf) SHA1(c845a012f7a8758cf7bbfc01a95780d1dd7d48b4) )    // korinai/4.2i

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "15.2a",      0x000000, 0x20000, CRC(6a9dae99) SHA1(639a56a2e84803f5e07cf799c51b15f813f993d6) )
	ROM_LOAD( "6.3a",       0x020000, 0x20000, CRC(2b1da51e) SHA1(7083e9ee8eb82d1f562aba2183c8d532d73c3686) )   // korinai/6.3a
	ROM_LOAD( "7.4a",       0x040000, 0x20000, CRC(85c260b9) SHA1(d1813329c66419bb4f19d9bba948d6662fc1142b) )   // korinai/7.4a
	ROM_LOAD( "8.5a",       0x060000, 0x20000, CRC(6a2763e1) SHA1(0718238a7b1cd3b1409824355f04f3c4ba73a8d5) )   // korinai/8.5a
	ROM_LOAD( "9.6a",       0x080000, 0x20000, CRC(81287588) SHA1(57784e8b62df68963592cffe0028dcc0118d44fd) )   // korinai/9.6a
	ROM_LOAD( "10.7a",      0x0a0000, 0x20000, CRC(9506d9cc) SHA1(8312e595b176ec43f0e77c26be165416ba43da4d) )   // korinai/10.7a
	ROM_LOAD( "11.8a",      0x0c0000, 0x20000, CRC(680d882e) SHA1(3505c9b530fd388e35467fdc0e31d125332fbc00) )   // korinai/11.8a
	ROM_LOAD( "12.9a",      0x0e0000, 0x20000, CRC(41a25dfe) SHA1(a71db0d896665f1943b92b7fa3c7ad9cd7ad8653) )   // korinai/12.9a
	ROM_LOAD( "13.10aa",    0x100000, 0x20000, CRC(1b578345) SHA1(c5685074599eb3e6f124f92029418e686f1d0bba) )
	ROM_LOAD( "14.11a",     0x120000, 0x20000, CRC(228c7a61) SHA1(0b5626cde26935f066cba33a125ed7fa00f2c295) )
	ROM_LOAD( "se-1507.1a", 0x180000, 0x80000, CRC(f1e9555e) SHA1(a34ffcff2b2d6ba40a8a453b89970d636515a8ad) )   // korinai/se-1507.1a
ROM_END

ROM_START( kaguya )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "kaguya01.bin", 0x00000, 0x10000, CRC(6ac18c32) SHA1(3605d12c42850bb679c0375b1c03fde7a15d0782) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "kaguya02.bin", 0x00000, 0x10000, CRC(561dc656) SHA1(0c3ca794ec71202aabcb337bb7d972a6d69dbbc7) )
	ROM_LOAD( "kaguya03.bin", 0x10000, 0x10000, CRC(a09e9387) SHA1(c5f5e0f5d841671bc38cd240193f60ccf7ab0455) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "kaguya04.bin", 0x000000, 0x20000, CRC(ccd08d8d) SHA1(6495946efdc99a945e30e7a46d4e0e2045ce62d5) )
	ROM_LOAD( "kaguya05.bin", 0x020000, 0x20000, CRC(a3abc686) SHA1(822c9e2a25343501f4168b45bb93942952640feb) )
	ROM_LOAD( "kaguya06.bin", 0x040000, 0x20000, CRC(6accd6d3) SHA1(ebcb911580329453ea91f2fbcba9f46bd5f6110f) )
	ROM_LOAD( "kaguya07.bin", 0x060000, 0x20000, CRC(93c64846) SHA1(3fff47cc3617285fc6feabd67f84ec2478519a8f) )
	ROM_LOAD( "kaguya08.bin", 0x080000, 0x20000, CRC(f0ad7c6c) SHA1(fb7587852b29b68bbeea3c123b13d50c23ba7584) )
	ROM_LOAD( "kaguya09.bin", 0x0a0000, 0x20000, CRC(f33fefdf) SHA1(ac063ea842cf2bdff9fb180b2b0bb4740df54fc1) )
	ROM_LOAD( "kaguya10.bin", 0x0c0000, 0x20000, CRC(741d13f6) SHA1(5d8143ec158b2e6c58b44f61dd063fb1615ff59e) )
	ROM_LOAD( "kaguya11.bin", 0x0e0000, 0x20000, CRC(fcbede4f) SHA1(543912e9fbb1c2b208701e5eb1347f734f5ce3cb) )
ROM_END

ROM_START( kaguya2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.3f",    0x00000, 0x10000, CRC(1a6ad8fd) SHA1(ebb1e3f08643e0602a0ec2e7401c3ee2fccff9f5) )

	ROM_REGION( 0x40000, "voice", 0 ) /* voice */
	ROM_LOAD( "2.3k",    0x00000, 0x10000, CRC(561dc656) SHA1(0c3ca794ec71202aabcb337bb7d972a6d69dbbc7) )   // kaguya/kaguya02.bin
	ROM_LOAD( "3.4p",    0x10000, 0x10000, CRC(a09e9387) SHA1(c5f5e0f5d841671bc38cd240193f60ccf7ab0455) )   // kaguya/kaguya03.bin
	/* protection data is mapped at 20000-2ffff */

	ROM_REGION( 0x0a0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4.6a",    0x000000, 0x10000, CRC(333c3cb9) SHA1(980826812b3b880c2c69d7dccf46e35858e0f219) )
	ROM_LOAD( "5.6c",    0x010000, 0x10000, CRC(e628601a) SHA1(eb0208c175c0633ab760d2e6ec34d565dafde792) )
	ROM_LOAD( "6.6d",    0x020000, 0x10000, CRC(db0d1f1a) SHA1(e47ff3a19863ca9461199043ec69cd212652d758) )
	ROM_LOAD( "7.6e",    0x030000, 0x10000, CRC(e1d76d58) SHA1(04cf85a651610905ff3a91663e28d1c4d894553f) )
	ROM_LOAD( "8.6f",    0x040000, 0x10000, CRC(935e59be) SHA1(15fca31491261efbbe90ace4b9b68b498e2e5769) )
	ROM_LOAD( "9.6h",    0x050000, 0x10000, CRC(bd184e9c) SHA1(1f0d2d4334e120986d62cd50f64dc33253702207) )
	ROM_LOAD( "10.6k",   0x060000, 0x10000, CRC(efdcfa40) SHA1(fe6e2db588d90386da687e77c65dffb4a1db0aea) )
	ROM_LOAD( "11.6l",   0x070000, 0x10000, CRC(ad980f55) SHA1(7d8c4fc0c5b98c6eb28ed4ad749b7025a95f19ef) )
	ROM_LOAD( "12.6m",   0x080000, 0x10000, CRC(420402f7) SHA1(7013b1e5d0f1a135d5f8422058c8ecd93e5b1890) )
	ROM_LOAD( "13.6p",   0x090000, 0x10000, CRC(ecb9f670) SHA1(c58ef1b17841d292e11e8906f11e61124eef672d) )

	ROM_REGION( 0x40000, "user1", 0 ) /* protection data */
	ROM_LOAD( "ic4m.bin", 0x00000, 0x40000, CRC(f85c5b07) SHA1(0fc55e9b60ccc630a0d77862eb5e64a3ba366947) )  // same as housemnq/3i.bin gfx data
ROM_END

ROM_START( kaguya2f )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.3fa",   0x00000, 0x10000, CRC(8b5481a0) SHA1(268b0ec8a8871c7172533ff8fb9731f4603e57a9) )

	ROM_REGION( 0x40000, "voice", 0 ) /* voice */
	ROM_LOAD( "2.3k",    0x00000, 0x10000, CRC(561dc656) SHA1(0c3ca794ec71202aabcb337bb7d972a6d69dbbc7) )   // kaguya/kaguya02.bin
	ROM_LOAD( "3.4p",    0x10000, 0x10000, CRC(a09e9387) SHA1(c5f5e0f5d841671bc38cd240193f60ccf7ab0455) )   // kaguya/kaguya03.bin
	/* protection data is mapped at 20000-2ffff */

	ROM_REGION( 0x0a0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4.6a",    0x000000, 0x10000, CRC(333c3cb9) SHA1(980826812b3b880c2c69d7dccf46e35858e0f219) )
	ROM_LOAD( "5.6c",    0x010000, 0x10000, CRC(e628601a) SHA1(eb0208c175c0633ab760d2e6ec34d565dafde792) )
	ROM_LOAD( "6.6d",    0x020000, 0x10000, CRC(db0d1f1a) SHA1(e47ff3a19863ca9461199043ec69cd212652d758) )
	ROM_LOAD( "7.6e",    0x030000, 0x10000, CRC(e1d76d58) SHA1(04cf85a651610905ff3a91663e28d1c4d894553f) )
	ROM_LOAD( "8.6f",    0x040000, 0x10000, CRC(935e59be) SHA1(15fca31491261efbbe90ace4b9b68b498e2e5769) )
	ROM_LOAD( "9.6h",    0x050000, 0x10000, CRC(bd184e9c) SHA1(1f0d2d4334e120986d62cd50f64dc33253702207) )
	ROM_LOAD( "10.6k",   0x060000, 0x10000, CRC(efdcfa40) SHA1(fe6e2db588d90386da687e77c65dffb4a1db0aea) )
	ROM_LOAD( "11.6l",   0x070000, 0x10000, CRC(ad980f55) SHA1(7d8c4fc0c5b98c6eb28ed4ad749b7025a95f19ef) )
	ROM_LOAD( "12.6m",   0x080000, 0x10000, CRC(420402f7) SHA1(7013b1e5d0f1a135d5f8422058c8ecd93e5b1890) )
	ROM_LOAD( "13.6p",   0x090000, 0x10000, CRC(ecb9f670) SHA1(c58ef1b17841d292e11e8906f11e61124eef672d) )

	ROM_REGION( 0x40000, "user1", ROMREGION_ERASE00 ) /* protection data */
//  ROM_LOAD( "ic4m.bin", 0x00000, 0x40000, CRC(f85c5b07) SHA1(0fc55e9b60ccc630a0d77862eb5e64a3ba366947) )  // same as housemnq/3i.bin gfx data
ROM_END

ROM_START( kanatuen )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "11.3f", 0x00000, 0x10000, CRC(3345d977) SHA1(9fd53d44b8f929a57b2900974f645898e3f92668) )

	ROM_REGION( 0x40000, "voice", 0 ) /* voice */
	ROM_LOAD( "12.3k", 0x00000, 0x10000, CRC(a4424adc) SHA1(caa4d607cb50ec2709c69f2f443e7cd7d0302aae) )
	/* protection data is mapped at 30000-3ffff */

	ROM_REGION( 0x0c0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "1.6a",  0x000000, 0x20000, CRC(a62b5982) SHA1(2faf908ff2cb37e7274215e1664836c991ad168c) )
	ROM_LOAD( "2.6bc", 0x020000, 0x20000, CRC(fd36dcae) SHA1(3e26503870c3554de02af26a85efc441b3724d0c) )
	ROM_LOAD( "3.6d",  0x040000, 0x10000, CRC(7636cbde) SHA1(d458dafcc56430a0f3628cc59307fae1f8b3e82a) )
	ROM_LOAD( "4.6e",  0x050000, 0x10000, CRC(ed9c7744) SHA1(c5d47f0364c150235fdcb88ff7ff0c6a880b8e20) )
	ROM_LOAD( "5.6f",  0x060000, 0x10000, CRC(d54cd45d) SHA1(82bc8f284db60553271dd0d1984636f2a087771f) )
	ROM_LOAD( "6.6h",  0x070000, 0x10000, CRC(1a0fbf52) SHA1(d433e2e6266631b8ee96cc960e502d60162b64d9) )
	ROM_LOAD( "7.6k",  0x080000, 0x10000, CRC(ea0c45f5) SHA1(ccdab9837fc70d1fab490500063712d5e6ade568) )
	ROM_LOAD( "8.6l",  0x090000, 0x10000, CRC(8754fc38) SHA1(c53da91f6dd34a7612b757278184360345cf4d84) )
	ROM_LOAD( "9.6m",  0x0a0000, 0x10000, CRC(51437563) SHA1(eb9133c19b4abad82f74d4091559c6d55337af11) )
	ROM_LOAD( "10.6p", 0x0b0000, 0x10000, CRC(1447ed65) SHA1(6b0f4ef3aef4dffe235a63000103c53ccad1c94f) )

	ROM_REGION( 0x40000, "user1", 0 ) /* protection data */
	ROM_LOAD( "mask.bin", 0x00000, 0x40000, CRC(f85c5b07) SHA1(0fc55e9b60ccc630a0d77862eb5e64a3ba366947) )  // same as housemnq/3i.bin gfx data
ROM_END

ROM_START( kyuhito )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "11.3fa", 0x00000, 0x10000, CRC(f3929245) SHA1(3654c6f167d643f0e24b44a1cfa44663b5b5ffbb) )

	ROM_REGION( 0x40000, "voice", 0 ) /* voice */
	ROM_LOAD( "12.3k", 0x00000, 0x10000, CRC(a4424adc) SHA1(caa4d607cb50ec2709c69f2f443e7cd7d0302aae) )     // kanatuen/12.3k
	/* protection data is mapped at 30000-3ffff */

	ROM_REGION( 0x0c0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "1.6a",  0x000000, 0x20000, CRC(a62b5982) SHA1(2faf908ff2cb37e7274215e1664836c991ad168c) )    // kanatuen/1.6a
	ROM_LOAD( "2.6bc", 0x020000, 0x20000, CRC(fd36dcae) SHA1(3e26503870c3554de02af26a85efc441b3724d0c) )    // kanatuen/2.6bc
	ROM_LOAD( "3.6d",  0x040000, 0x10000, CRC(7636cbde) SHA1(d458dafcc56430a0f3628cc59307fae1f8b3e82a) )    // kanatuen/3.6d
	ROM_LOAD( "4.6e",  0x050000, 0x10000, CRC(ed9c7744) SHA1(c5d47f0364c150235fdcb88ff7ff0c6a880b8e20) )    // kanatuen/4.6e
	ROM_LOAD( "5.6f",  0x060000, 0x10000, CRC(d54cd45d) SHA1(82bc8f284db60553271dd0d1984636f2a087771f) )    // kanatuen/5.6f
	ROM_LOAD( "6.6h",  0x070000, 0x10000, CRC(1a0fbf52) SHA1(d433e2e6266631b8ee96cc960e502d60162b64d9) )    // kanatuen/6.6h
	ROM_LOAD( "7.6k",  0x080000, 0x10000, CRC(ea0c45f5) SHA1(ccdab9837fc70d1fab490500063712d5e6ade568) )    // kanatuen/7.6k
	ROM_LOAD( "8.6la", 0x090000, 0x10000, CRC(a8ce450a) SHA1(e009429e0c4ca5332b8f411fcce1189e53d98834) )
	ROM_LOAD( "9.6ma", 0x0a0000, 0x10000, CRC(401c23fe) SHA1(ef3d484d4b2c640fbdb5b023d271ee98e1a85d1a) )
	ROM_LOAD( "10.6p", 0x0b0000, 0x10000, CRC(1447ed65) SHA1(6b0f4ef3aef4dffe235a63000103c53ccad1c94f) )    // kanatuen/10.6p

	ROM_REGION( 0x40000, "user1", ROMREGION_ERASE00 ) /* protection data */
//  ROM_LOAD( "mask.bin", 0x00000, 0x40000, CRC(f85c5b07) SHA1(0fc55e9b60ccc630a0d77862eb5e64a3ba366947) )  // same as housemnq/3i.bin gfx data
ROM_END

ROM_START( mjsikaku )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "mjsk_01.bin", 0x00000, 0x10000, CRC(6b64c96a) SHA1(2f267b66773dc8c0ad260f081738b30fd555c818) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "mjsk_02.bin", 0x00000, 0x10000, CRC(cc0262bb) SHA1(ce980bd83f8aec775a92f4ea21ff0cc2a9ed7886) )
	ROM_LOAD( "mjsk_03.bin", 0x10000, 0x10000, CRC(7dedcd75) SHA1(60add3b00fb0c35e111d883e3bceee8c85840455) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "mjsk_04.bin", 0x000000, 0x20000, CRC(34d13d1e) SHA1(8d13b3d2dc89c092e2770440d3768ee887f49563) )
	ROM_LOAD( "mjsk_05.bin", 0x020000, 0x20000, CRC(8c70aed5) SHA1(b97a1850f0ce421226818e59ca209f30f6a57b6f) )
	ROM_LOAD( "mjsk_06.bin", 0x040000, 0x20000, CRC(1dad8355) SHA1(dd38793cc86b07b25fd110313b29c6cce8069cf6) )
	ROM_LOAD( "mjsk_07.bin", 0x060000, 0x20000, CRC(8174a28a) SHA1(46e89a84675b2a61c4a0771e57edd8f31d6ba3c6) )
	ROM_LOAD( "mjsk_08.bin", 0x080000, 0x20000, CRC(3e182aaa) SHA1(6fa4ee29a2e402872a4d3f54b5f99e4dfe02636a) )
	ROM_LOAD( "mjsk_09.bin", 0x0a0000, 0x20000, CRC(a17a3328) SHA1(4bd8a4aba042fa2bd10d99e52360f6fe8cc0d3d3) )
	ROM_LOAD( "mjsk_10.bin", 0x0c0000, 0x10000, CRC(cab4909f) SHA1(c6a3dd53bddb5322df5fbc771b4981acc1cc4040) )
	ROM_LOAD( "mjsk_11.bin", 0x0d0000, 0x10000, CRC(dd7a95c8) SHA1(33d12ecd4b963f8ff7f03d8ce2832242b8d087f5) )
	ROM_LOAD( "mjsk_12.bin", 0x0e0000, 0x10000, CRC(20c25377) SHA1(e1f6aae7db249d6cceef4421abf5a9ecfd60b5c5) )
	ROM_LOAD( "mjsk_13.bin", 0x0f0000, 0x10000, CRC(967e9a91) SHA1(41c81021ebecc57abae884ec115de2a31024e1a8) )
ROM_END

ROM_START( mjsikakb )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "sikaku.1",    0x00000, 0x10000, CRC(66349663) SHA1(48cdf25a30e11c06b79f218f4744719199961429) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "mjsk_02.bin", 0x00000, 0x10000, CRC(cc0262bb) SHA1(ce980bd83f8aec775a92f4ea21ff0cc2a9ed7886) )   // mjsikaku/mjsk_02.bin
	ROM_LOAD( "mjsk_03.bin", 0x10000, 0x10000, CRC(7dedcd75) SHA1(60add3b00fb0c35e111d883e3bceee8c85840455) )   // mjsikaku/mjsk_03.bin

	ROM_REGION( 0x100000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "mjsk_04.bin", 0x000000, 0x20000, CRC(34d13d1e) SHA1(8d13b3d2dc89c092e2770440d3768ee887f49563) )  // mjsikaku/mjsk_04.bin
	ROM_LOAD( "mjsk_05.bin", 0x020000, 0x20000, CRC(8c70aed5) SHA1(b97a1850f0ce421226818e59ca209f30f6a57b6f) )  // mjsikaku/mjsk_05.bin
	ROM_LOAD( "mjsk_06.bin", 0x040000, 0x20000, CRC(1dad8355) SHA1(dd38793cc86b07b25fd110313b29c6cce8069cf6) )  // mjsikaku/mjsk_06.bin
	ROM_LOAD( "mjsk_07.bin", 0x060000, 0x20000, CRC(8174a28a) SHA1(46e89a84675b2a61c4a0771e57edd8f31d6ba3c6) )  // mjsikaku/mjsk_07.bin
	ROM_LOAD( "mjsk_08.bin", 0x080000, 0x20000, CRC(3e182aaa) SHA1(6fa4ee29a2e402872a4d3f54b5f99e4dfe02636a) )  // mjsikaku/mjsk_08.bin
	ROM_LOAD( "mjsk_09.bin", 0x0a0000, 0x20000, CRC(a17a3328) SHA1(4bd8a4aba042fa2bd10d99e52360f6fe8cc0d3d3) )  // mjsikaku/mjsk_09.bin
	ROM_LOAD( "sikaku.10",   0x0c0000, 0x20000, CRC(f91757bc) SHA1(58fc1e9b291cbca0e169945bed375fb4438e96d4) )
	ROM_LOAD( "sikaku.11",   0x0e0000, 0x20000, CRC(abd280b6) SHA1(46e1cb56a768467a8a802d58e2150a25cd0fb8bd) )
ROM_END

ROM_START( mjsikakc )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.bin",       0x00000, 0x10000, CRC(74e6e403) SHA1(975ea8792511d9962ccd41a2cc70bce9e97a187d) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "mjsk_02.bin", 0x00000, 0x10000, CRC(cc0262bb) SHA1(ce980bd83f8aec775a92f4ea21ff0cc2a9ed7886) )   // mjsikaku/mjsk_02.bin
	ROM_LOAD( "mjsk_03.bin", 0x10000, 0x10000, CRC(7dedcd75) SHA1(60add3b00fb0c35e111d883e3bceee8c85840455) )   // mjsikaku/mjsk_03.bin

	ROM_REGION( 0x100000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "mjsk_04.bin", 0x000000, 0x20000, CRC(34d13d1e) SHA1(8d13b3d2dc89c092e2770440d3768ee887f49563) )  // mjsikaku/mjsk_04.bin
	ROM_LOAD( "mjsk_05.bin", 0x020000, 0x20000, CRC(8c70aed5) SHA1(b97a1850f0ce421226818e59ca209f30f6a57b6f) )  // mjsikaku/mjsk_05.bin
	ROM_LOAD( "mjsk_06.bin", 0x040000, 0x20000, CRC(1dad8355) SHA1(dd38793cc86b07b25fd110313b29c6cce8069cf6) )  // mjsikaku/mjsk_06.bin
	ROM_LOAD( "mjsk_07.bin", 0x060000, 0x20000, CRC(8174a28a) SHA1(46e89a84675b2a61c4a0771e57edd8f31d6ba3c6) )  // mjsikaku/mjsk_07.bin
	ROM_LOAD( "mjsk_08.bin", 0x080000, 0x20000, CRC(3e182aaa) SHA1(6fa4ee29a2e402872a4d3f54b5f99e4dfe02636a) )  // mjsikaku/mjsk_08.bin
	ROM_LOAD( "mjsk_09.bin", 0x0a0000, 0x20000, CRC(a17a3328) SHA1(4bd8a4aba042fa2bd10d99e52360f6fe8cc0d3d3) )  // mjsikaku/mjsk_09.bin
	ROM_LOAD( "sikaku.10",   0x0c0000, 0x20000, CRC(f91757bc) SHA1(58fc1e9b291cbca0e169945bed375fb4438e96d4) )  // mjsikakb/sikaku.10
	ROM_LOAD( "sikaku.11",   0x0e0000, 0x20000, CRC(abd280b6) SHA1(46e1cb56a768467a8a802d58e2150a25cd0fb8bd) )  // mjsikakb/sikaku.11
ROM_END

ROM_START( mjsikakd )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "11.bin",      0x00000, 0x10000, CRC(372474bd) SHA1(12ee6f3a49926d8120b46e36df4d7df628e86ac1) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "mjsk_02.bin", 0x00000, 0x10000, CRC(cc0262bb) SHA1(ce980bd83f8aec775a92f4ea21ff0cc2a9ed7886) )   // mjsikaku/mjsk_02.bin
	ROM_LOAD( "mjsk_03.bin", 0x10000, 0x10000, CRC(7dedcd75) SHA1(60add3b00fb0c35e111d883e3bceee8c85840455) )   // mjsikaku/mjsk_03.bin

	ROM_REGION( 0x100000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "mjsk_04.bin", 0x000000, 0x20000, CRC(34d13d1e) SHA1(8d13b3d2dc89c092e2770440d3768ee887f49563) )  // mjsikaku/mjsk_04.bin
	ROM_LOAD( "mjsk_05.bin", 0x020000, 0x20000, CRC(8c70aed5) SHA1(b97a1850f0ce421226818e59ca209f30f6a57b6f) )  // mjsikaku/mjsk_05.bin
	ROM_LOAD( "mjsk_06.bin", 0x040000, 0x20000, CRC(1dad8355) SHA1(dd38793cc86b07b25fd110313b29c6cce8069cf6) )  // mjsikaku/mjsk_06.bin
	ROM_LOAD( "mjsk_07.bin", 0x060000, 0x20000, CRC(8174a28a) SHA1(46e89a84675b2a61c4a0771e57edd8f31d6ba3c6) )  // mjsikaku/mjsk_07.bin
	ROM_LOAD( "mjsk_08.bin", 0x080000, 0x20000, CRC(3e182aaa) SHA1(6fa4ee29a2e402872a4d3f54b5f99e4dfe02636a) )  // mjsikaku/mjsk_08.bin
	ROM_LOAD( "mjsk_09.bin", 0x0a0000, 0x20000, CRC(a17a3328) SHA1(4bd8a4aba042fa2bd10d99e52360f6fe8cc0d3d3) )  // mjsikaku/mjsk_09.bin
	ROM_LOAD( "sikaku.10",   0x0c0000, 0x20000, CRC(f91757bc) SHA1(58fc1e9b291cbca0e169945bed375fb4438e96d4) )  // mjsikakb/sikaku.10
	ROM_LOAD( "sikaku.11",   0x0e0000, 0x20000, CRC(abd280b6) SHA1(46e1cb56a768467a8a802d58e2150a25cd0fb8bd) )  // mjsikakb/sikaku.11
ROM_END

ROM_START( mmsikaku )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4g",        0x00000, 0x08000, CRC(6a8483af) SHA1(0ba9177991f14ea26cec1d1e01c3c38921d5b3cf) )
	ROM_LOAD( "2.3g",        0x08000, 0x08000, CRC(eb352bea) SHA1(6e0233de09d74ff0bc92cbc9715b8c7c2fefdade) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "3.3i",        0x00000, 0x10000, CRC(ce53f2bc) SHA1(685205f03ab2e295f1ed8a4c19494fda79083472) )

	ROM_REGION( 0x1c0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "1.2a",        0x000000, 0x20000, CRC(0024bce0) SHA1(669be183eecce8d4655884c8e3d744e9ccc63667) )
	ROM_LOAD( "mjsk_05.bin", 0x020000, 0x20000, CRC(8c70aed5) SHA1(b97a1850f0ce421226818e59ca209f30f6a57b6f) )  // mjsikaku/mjsk_05.bin
	ROM_LOAD( "mjsk_06.bin", 0x040000, 0x20000, CRC(1dad8355) SHA1(dd38793cc86b07b25fd110313b29c6cce8069cf6) )  // mjsikaku/mjsk_06.bin
	ROM_LOAD( "mjsk_07.bin", 0x060000, 0x20000, CRC(8174a28a) SHA1(46e89a84675b2a61c4a0771e57edd8f31d6ba3c6) )  // mjsikaku/mjsk_07.bin
	ROM_LOAD( "mjsk_08.bin", 0x080000, 0x20000, CRC(3e182aaa) SHA1(6fa4ee29a2e402872a4d3f54b5f99e4dfe02636a) )  // mjsikaku/mjsk_08.bin
	ROM_LOAD( "mjsk_09.bin", 0x0a0000, 0x20000, CRC(a17a3328) SHA1(4bd8a4aba042fa2bd10d99e52360f6fe8cc0d3d3) )  // mjsikaku/mjsk_09.bin
	ROM_LOAD( "7.8a",        0x0c0000, 0x20000, CRC(f91757bc) SHA1(58fc1e9b291cbca0e169945bed375fb4438e96d4) )  // mjsikakb/sikaku.10
	ROM_LOAD( "8.9a",        0x0e0000, 0x20000, CRC(d453a221) SHA1(7cdc96f7634d3c7b1c51588e4951719c0c016af1) )
	ROM_LOAD( "ic1a.bin",    0x180000, 0x40000, CRC(2199e3e9) SHA1(965af4a29db4ff909dbeeebab1b828eb4f23f57e) )  // housemnq/1i.bin
ROM_END

ROM_START( otonano )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "otona_01.bin", 0x00000, 0x10000, CRC(ee629b72) SHA1(f4661981e6e41cf17d2ca453a6c53f9b9bbd93f1) )

	ROM_REGION( 0x40000, "voice", 0 ) /* voice */
	ROM_LOAD( "otona_02.bin", 0x00000, 0x10000, CRC(2864b8ef) SHA1(1388b4a897a840563195f53b53ea2afcde56872e) )
	ROM_LOAD( "otona_03.bin", 0x10000, 0x10000, CRC(ece880e0) SHA1(b695fb7b861e19d1c18a3740cb055b7df55d0245) )
	ROM_LOAD( "otona_04.bin", 0x20000, 0x10000, CRC(5a25b251) SHA1(3da145c8e4f0785ecf3289fe73832f56893d0ba8) )
	ROM_LOAD( "otona_05.bin", 0x30000, 0x10000, CRC(469d580d) SHA1(2e2be602862560c31b9f13d969493e77ab235733) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "otona_06.bin", 0x000000, 0x20000, CRC(2d41f854) SHA1(133fe6d73361ec37d04792fc42f3c5e69a6a7f12) )
	ROM_LOAD( "otona_07.bin", 0x020000, 0x20000, CRC(58d6717d) SHA1(41fc584c9f261aae12ba7598917f641e043015e5) )
	ROM_LOAD( "otona_08.bin", 0x040000, 0x20000, CRC(40f8d432) SHA1(f5f5d525cc77a6decfddd42ce2ec4f66d9cf5782) )
	ROM_LOAD( "otona_09.bin", 0x060000, 0x20000, CRC(fd80fdc2) SHA1(df2773f852ac05cccc30e375ed390e3937a74a95) )
	ROM_LOAD( "otona_10.bin", 0x080000, 0x20000, CRC(50ff867a) SHA1(dabaa6850fee4f394ae2780caecf29a1b8e4143a) )
	ROM_LOAD( "otona_11.bin", 0x0a0000, 0x20000, CRC(c467e822) SHA1(c0af856acbea4ac15c50732e9a488df7586e2e2b) )
	ROM_LOAD( "otona_12.bin", 0x0c0000, 0x20000, CRC(1a0f9250) SHA1(18cb43887a27eaf143a1c5bbf0a9d57eb4fcebf2) )
	ROM_LOAD( "otona_13.bin", 0x0e0000, 0x20000, CRC(208dee43) SHA1(f154ac4dab929c6f610038dbbebcf5283258e553) )
ROM_END

ROM_START( mjcamera )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "mcam_01.bin", 0x00000, 0x10000, CRC(73d4b9ff) SHA1(219bc9617c14490d70bb3e28ab497dfd2ef01cf8) )

	ROM_REGION( 0x30000, "voice", 0 ) /* voice */
	ROM_LOAD( "1.2k",        0x00000, 0x10000, CRC(fe8e975e) SHA1(7287f5654aebc1f27c957d4af997480fa380b15a) )
	/* protection data is mapped at 20000-2ffff */

	ROM_REGION( 0x100000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "3.8c",        0x000000, 0x10000, CRC(273fb8bc) SHA1(d5aa20570a1ad7a97d2b4eb12039c51d85288a96) )
	ROM_LOAD( "4.8d",        0x010000, 0x10000, CRC(82995399) SHA1(0aa4dcbed01f6a4893c31487469989035fd791c3) )
	ROM_LOAD( "5.8e",        0x020000, 0x10000, CRC(a7c51d54) SHA1(b553136a1e2bf3e6cd61a0b0ebd0341a389ac65f) )
	ROM_LOAD( "6.8f",        0x030000, 0x10000, CRC(f221700c) SHA1(e8f640bd8a155be96e70bc8bc4e29b614695b0db) )
	ROM_LOAD( "7.8h",        0x040000, 0x10000, CRC(6baa4d45) SHA1(61a5470f85260ad957069cfa422c918905659f02) )
	ROM_LOAD( "8.8k",        0x050000, 0x10000, CRC(91d9c868) SHA1(a584f6fa46defbee49c5c6441b50233749a45118) )
	ROM_LOAD( "9.8l",        0x060000, 0x10000, CRC(56a35d4b) SHA1(1c769798661531f760da5d54af7f86d45e8e6c0f) )
	ROM_LOAD( "10.8m",       0x070000, 0x10000, CRC(480e23c4) SHA1(e357134a3bc68437b5f36a563c69ef7583861aab) )
	ROM_LOAD( "11.8n",       0x080000, 0x10000, CRC(2c29accc) SHA1(e3ff6db06e4001262093d28cb44c0912de16989a) )
	ROM_LOAD( "12.8p",       0x090000, 0x10000, CRC(902d73f8) SHA1(cead5c1a072fb95847f50af2e65f6108ef5f4928) )
	ROM_LOAD( "13.10c",      0x0a0000, 0x10000, CRC(fcba0179) SHA1(34b1e9a4908dbed3dcbbeafe5b05dccee6aef13a) )
	ROM_LOAD( "14.10d",      0x0b0000, 0x10000, CRC(ee2c37a9) SHA1(fff260eade85ee3c01b32d3eea6133c85a22d645) )
	ROM_LOAD( "15.10e",      0x0c0000, 0x10000, CRC(90fd36f8) SHA1(ec8e9e6a52a5a8a9e3f688a400e946dae643f747) )
	ROM_LOAD( "16.10f",      0x0d0000, 0x10000, CRC(41265f7f) SHA1(98d02ed1af3adeaf9aa261d98e48d2745a0eec28) )
	ROM_LOAD( "17.10h",      0x0e0000, 0x10000, CRC(78cef468) SHA1(aedd94d3fcf097587e77f52d03a50a63606bdab6) )
	ROM_LOAD( "mcam_18.bin", 0x0f0000, 0x10000, CRC(3a3da341) SHA1(198ea75aedff187b02a740d5a1cc49c76340831f) )

	ROM_REGION( 0x40000, "user1", 0 ) /* protection data */
	ROM_LOAD( "mcam_m1.bin", 0x00000, 0x40000, CRC(f85c5b07) SHA1(0fc55e9b60ccc630a0d77862eb5e64a3ba366947) )   // same as housemnq/3i.bin gfx data
ROM_END

ROM_START( idhimitu )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.3f",     0x00000, 0x10000, CRC(619f9465) SHA1(9e4d3cab6370bda06ddedea5ca4b6d4cbd158174) )

	ROM_REGION( 0x30000, "voice", 0 ) /* voice */
	ROM_LOAD( "2.3k",     0x00000, 0x10000, CRC(9a5f7907) SHA1(939e2dd2765a922aaf3c6a104caf459f1478863f) )
	/* protection data is mapped at 20000-2ffff */

	ROM_REGION( 0x0e0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "3.6a",     0x000000, 0x10000, CRC(4677f0d0) SHA1(e2fc7dfdb1e4d85964937a1a0deaa4e7e2ef40db) )
	ROM_LOAD( "4.6b",     0x010000, 0x10000, CRC(f935a681) SHA1(764d69ca149cfcc42676c0a3c2f347f723b22f3f) )
	ROM_LOAD( "8.6h",     0x050000, 0x10000, CRC(f03768b0) SHA1(001f3532b9a2f909184a9a18ca0352d660e0f82d) )
	ROM_LOAD( "9.6k",     0x060000, 0x10000, CRC(04918709) SHA1(606d87bdebeeaa14aaa1ce643f0919c67bda3c1a) )
	ROM_LOAD( "10.6l",    0x070000, 0x10000, CRC(ae95e5e2) SHA1(bc5621665cb8e6a3b151a986f2fe469046cec4ef) )
	ROM_LOAD( "11.6m",    0x080000, 0x10000, CRC(f9865cf3) SHA1(d93c6364eec2a5539ebb6d98491c4a6d8d374431) )
	ROM_LOAD( "12.6p",    0x090000, 0x10000, CRC(99545a6b) SHA1(b44927dc9d299dc20647a342c3bdfcdd4cbb7a77) )
	ROM_LOAD( "13.7a",    0x0a0000, 0x10000, CRC(60472080) SHA1(4d3f8bc02bc4c9abbe0ce08c3061aa68407ebb03) )
	ROM_LOAD( "14.7b",    0x0b0000, 0x10000, CRC(3e26e374) SHA1(aa06bdb022a25f1580597f0af3ae8413e140562d) )
	ROM_LOAD( "15.7d",    0x0c0000, 0x10000, CRC(9e303eda) SHA1(14a988c8df572aa16bc0464bcb9fd627c8b57537) )
	ROM_LOAD( "16.7e",    0x0d0000, 0x10000, CRC(0429ae8f) SHA1(e380e159b2dcafcbfd3e9991ee9e76b842189e37) )

	ROM_REGION( 0x40000, "user1", 0 ) /* protection data */
	ROM_LOAD( "ic3m.bin", 0x00000, 0x40000, CRC(ba005a3a) SHA1(305041f764b5ba9ffa882c1a69555a38a53b1556) )  // same as iemoto/iemoto31.1a gfx data
ROM_END

ROM_START( barline )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "16061_1h1r.c2", 0x00000, 0x10000, CRC(0a1d3e61) SHA1(652005181779e69c03f2b29e6aac2481321b8d06) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "16061_2a.e9", 0x00000, 0x10000, CRC(53c1a339) SHA1(439bb1dc072be47233567ec9215384cb1959e2d4) )
	ROM_LOAD( "16061_2b.e10",0x10000, 0x10000, CRC(8126dac6) SHA1(45b7f2f0dd373847bdfe13c7f51b198ea409d70e) )
	ROM_LOAD( "16061_3a.f9", 0x20000, 0x10000, CRC(ceb17b22) SHA1(a7b72ec3e93bab6dcb5c480d812c0223b7e0acc1) )
	ROM_LOAD( "16061_3b.f10",0x30000, 0x10000, CRC(b269b85b) SHA1(a8f21f37b7dde7425fa1fd0264b3c67620ffcdaa) )
	ROM_LOAD( "16061_4a.g9", 0x40000, 0x10000, CRC(729dbf45) SHA1(8b5fbeacf45365d16546654525fe1e65ec781ece) )
	ROM_LOAD( "16061_4b.g10",0x50000, 0x10000, CRC(7c6946be) SHA1(5f236658073b3b3a54c82f0a973fb7a8c91a1e13) )
	ROM_LOAD( "16061_5a.h9", 0x60000, 0x10000, CRC(707ca3b9) SHA1(6a5d931bfbfeb7b6be038d3bdb982055c201335e) )
	ROM_LOAD( "16061_5b.h10",0x70000, 0x10000, CRC(6d83713e) SHA1(5548b75d07793a609e1b92bd385c77efff41e46d) )
	ROM_LOAD( "16061_6a.i9", 0x80000, 0x10000, CRC(79d93064) SHA1(ad07e22519064f6e952787e1ff072f769536cb2b) )
	ROM_LOAD( "16061_6b.i10",0x90000, 0x10000, CRC(e724d22f) SHA1(6810a4b0e4665d6773c55355607e85b4e3efe380) )
	ROM_LOAD( "16061_7a.j9", 0xa0000, 0x10000, CRC(49a5322c) SHA1(0037107833ece237b22a00793ac8a8562d57a3c5) )
	ROM_LOAD( "16061_7b.j10",0xb0000, 0x10000, CRC(cfbbe06b) SHA1(f251df0f102d2812a23ff99dbdbd832ba122d787) )

	ROM_REGION( 0x104, "pals", 0 )
	ROM_LOAD( "16061.c10",0x000, 0x104, CRC(f8a85391) SHA1(ac47909fd4ab8fa198b14b82c2619e82a79ae3ef) )
	ROM_LOAD( "16061.db", 0x000, 0x104, CRC(9c28c9c9) SHA1(7622f985fece66951a5d9d2e3a2b2c26d0980d26) )
	ROM_LOAD( "16061.g3", 0x000, 0x104, CRC(22579310) SHA1(33fe9ea70895e0233fa1c5a999d3c9e50031209c) )
	ROM_LOAD( "16061.k7", 0x000, 0x104, CRC(d25ccac8) SHA1(cfad5a4cd9609ac2461314d77a5e0cecd326c63b) )
ROM_END

ROM_START( bikkuri )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "12.2c.27512", 0x00000, 0x10000, CRC(b5caab77) SHA1(e42657a17e0e4644524d116950b152c7dd23528f) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "13.9e.27512", 0x00000, 0x10000, CRC(d264f352) SHA1(dd75ed368f6afa6b42a6ae720077f18b839e2674) )
	ROM_LOAD( "14.9f.27512", 0x10000, 0x10000, CRC(40e15d4b) SHA1(5a055cdff4841d1dec3578fc886f53cc8eb1328c) )
	ROM_LOAD( "15.9g.27512", 0x20000, 0x10000, CRC(cf8a3bb6) SHA1(0e25d0440d1479a30010850329c519249f5a65ed) )
	ROM_LOAD( "19.9h.27512", 0x30000, 0x10000, CRC(a0735d2d) SHA1(30bb62dab6057f6fb7c60ffc514ca81b027ef8b8) )
	ROM_LOAD( "20.9i.27512", 0x40000, 0x10000, CRC(abe6c935) SHA1(846b93e5028319d2ec8e2bcfde52af7110d6d3be) )
	ROM_LOAD( "21.9j.27512", 0x50000, 0x10000, CRC(b4c708da) SHA1(f474c5a24c9b72c2c38272915c45ea7dcebcde18) )
	ROM_LOAD( "22.9k.27512", 0x60000, 0x10000, CRC(dc79f73b) SHA1(eb47e7a218610d53ba78a2cb344d8fc491ab7fbd) )
	ROM_LOAD( "23.9l.27512", 0x70000, 0x10000, CRC(f18bbe10) SHA1(ce7862b9a812523ddadfc7137a47da197d9a2df0) )

	ROM_REGION( 0x500, "pals", ROMREGION_ERASE00 ) // no dump attempt made, there might be more
	ROM_LOAD( "pal10l8nc.8a",  0x000, 0x02c, NO_DUMP )
	ROM_LOAD( "pal16l8acn.3g", 0x100, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8acn.8k", 0x300, 0x104, NO_DUMP )
ROM_END


/* 8-bit palette */
GAME( 1986, crystalg, 0,        crystalg,        crystalg, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Crystal Gal (Japan 860512)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, crystal2, 0,        crystal2,        crystal2, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Crystal Gal 2 (Japan 860620)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, nightlov, 0,        nightlov,        nightlov, nbmj8688_state, empty_init,    ROT0, "Central Denshi", "Night Love (Japan 860705)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1986, apparel,  0,        apparel,         apparel,  nbmj8688_state, empty_init,    ROT0, "Central Denshi", "Apparel Night (Japan 860929)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, bikkuri,  0,        bikkuri,         bikkuri,  nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Bikkuri Pro Wrestling (Japan 881221, Ver 1.05)", MACHINE_SUPPORTS_SAVE )

/* hybrid 12-bit palette */
GAME( 1986, citylove, 0,        citylove,        citylove, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "City Love (Japan 860908)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, mcitylov, citylove, mcitylov,        mcitylov, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "City Love [BET] (Japan 860904)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, secolove, 0,        secolove,        secolove, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Second Love (Japan 861201)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, barline,  0,        barline,         barline,  nbmj8688_state, empty_init,    ROT180, "Nichibutsu",   "Barline (Japan?)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

/* hybrid 16-bit palette */
GAME( 1987, seiha,    0,        seiha,           seiha,    nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Seiha (Japan 870725)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, seiham,   seiha,    seiham,          seiham,   nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Seiha [BET] (Japan 870723)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, mjgaiden, 0,        mjgaiden,        ojousan,  nbmj8688_state, empty_init,    ROT0, "Central Denshi", "Mahjong Gaiden [BET] (Japan 870803)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, iemoto,   0,        iemoto,          iemoto,   nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Iemoto (Japan 871020)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, iemotom,  iemoto,   iemotom,         iemotom,  nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Iemoto [BET] (Japan 871118)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ryuuha,   iemoto,   ryuuha,          ryuuha,   nbmj8688_state, empty_init,    ROT0, "Central Denshi", "Ryuuha [BET] (Japan 871027)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ojousan,  0,        ojousan,         ojousan,  nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Ojousan (Japan 871204)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ojousanm, ojousan,  ojousanm,        ojousanm, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Ojousan [BET] (Japan 870108)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, swinggal, 0,        swinggal,        ryuuha,   nbmj8688_state, empty_init,    ROT0, "Digital Denshi", "Swing Gal [BET] (Japan 871221)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, korinai,  0,        korinai,         korinai,  nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Mahjong-zukino Korinai Menmen (Japan 880425)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, korinaim, korinai,  korinaim,        korinaim, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Mahjong-zukino Korinai Menmen [BET] (Japan 880920)", MACHINE_SUPPORTS_SAVE )

/* pure 16-bit palette (+ LCD in some) */
GAME( 1987, housemnq, 0,        housemnq,        housemnq, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "House Mannequin (Japan 870217)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, housemn2, 0,        housemn2,        housemn2, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "House Mannequin Roppongi Live hen (Japan 870418)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, livegal,  0,        livegal,         livegal,  nbmj8688_state, empty_init,    ROT0, "Central Denshi", "Live Gal [BET] (Japan 870530)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, bijokkoy, 0,        bijokkoy,        bijokkoy, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Bijokko Yume Monogatari (Japan 870925)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, bijokkog, 0,        bijokkog,        bijokkog, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Bijokko Gakuen (Japan 880116)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, orangec,  0,        orangec,         orangec,  nbmj8688_state, empty_init,    ROT0, "Daiichi Denshi", "Orange Club - Maruhi Kagai Jugyou (Japan 880213)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, orangeci, orangec,  orangeci,        orangeci, nbmj8688_state, empty_init,    ROT0, "Daiichi Denshi", "Orange Club - Maru-hi Ippatsu Kaihou [BET] (Japan 880221)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, vipclub,  orangec,  vipclub,         vipclub,  nbmj8688_state, empty_init,    ROT0, "Daiichi Denshi", "Vip Club - Maru-hi Ippatsu Kaihou [BET] (Japan 880310)", MACHINE_SUPPORTS_SAVE )

/* pure 12-bit palette */
GAME( 1988, kaguya,   0,        kaguya,          kaguya,   nbmj8688_state, empty_init,    ROT0, "Miki Syouji",    "Mahjong Kaguyahime [BET] (Japan 880521)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, kaguya2,  0,        kaguya2,         kaguya,   nbmj8688_state, init_kaguya2,  ROT0, "Miki Syouji",    "Mahjong Kaguyahime Sono2 [BET] (Japan 890829)", MACHINE_SUPPORTS_SAVE )
GAME( 2001, kaguya2f, kaguya2,  kaguya2,         kaguya,   nbmj8688_state, init_kaguya2,  ROT0, "Miki Syouji",    "Mahjong Kaguyahime Sono2 Fukkokuban [BET] (Japan 010808)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, kanatuen, 0,        kanatuen,        kanatuen, nbmj8688_state, init_kanatuen, ROT0, "Panac",          "Kanatsuen no Onna [BET] (Japan 880905)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, kyuhito,  kanatuen, kyuhito,         kyuhito,  nbmj8688_state, init_kyuhito,  ROT0, "Roller Tron",    "Kyukyoku no Hito [BET] (Japan 880824)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, idhimitu, 0,        idhimitu,        idhimitu, nbmj8688_state, init_idhimitu, ROT0, "Digital Soft",   "Idol no Himitsu [BET] (Japan 890304)", MACHINE_SUPPORTS_SAVE )

/* pure 12-bit palette + YM3812 instead of AY-3-8910 */
GAME( 1988, mjsikaku, 0,        mjsikaku,        mjsikaku, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Mahjong Shikaku (Japan 880908)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, mjsikakb, mjsikaku, mjsikaku,        mjsikaku, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Mahjong Shikaku (Japan 880722)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, mjsikakc, mjsikaku, mjsikaku,        mjsikaku, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Mahjong Shikaku (Japan 880806)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, mjsikakd, mjsikaku, mjsikaku,        mjsikaku, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Mahjong Shikaku (Japan 880802)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, mmsikaku, mjsikaku, mmsikaku,        mmsikaku, nbmj8688_state, empty_init,    ROT0, "Nichibutsu",     "Mahjong Shikaku [BET] (Japan 880929)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, otonano,  0,        otonano,         otonano,  nbmj8688_state, empty_init,    ROT0, "Apple",          "Otona no Mahjong (Japan 880628)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, mjcamera, 0,        mjcamera,        mjcamera, nbmj8688_state, init_mjcamera, ROT0, "Miki Syouji",    "Mahjong Camera Kozou (set 1) (Japan 881109)", MACHINE_SUPPORTS_SAVE )
