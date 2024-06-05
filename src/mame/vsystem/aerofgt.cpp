// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Various Video System Co. games using the C7-01 GGA, VS8803, VS8904, VS8905
video chips.
C7-01 GGA is used in a lot of games, some of them without sprites. So it
either controls tilemaps, or the video signal, or both.
I think 8904/8905 handle sprites, don't know about 8803.
tail2nos doesn't have the 8904/8905, and indeed it has a different sprite
system.

Driver by Nicola Salmoria


Notes:
- Sprite zoom is probably not 100% accurate.
  In pspikes, the zooming text during attract mode is horrible.
- spinlbrk: enemy sprites sometimes shows 1 pixel off on bottom if they are covered by big objects,
  such as:
  - tank boss in stage 1;
  - trenches in Greece stage;
  chip 0 draws player sprite, chip 1 all enemies. Assume btanb.

pspikes/turbofrc/aerofgtb write to two addresses which look like control
registers for a video generator. Maybe they control the display size/position.
aerofgt is different, it writes to consecutive memory addresses and the values
it writes don't seem to be related to these ones.

                  00 01 02 03 04 05  08 09 0a 0b 0c 0d
                  ------------------------------------
pspikes  352x240? 57 63 69 71 1f 00  77 79 7b 7f 1f 00
karatblz 352x240  57 63 69 71 1f 00  77 79 7b 7f 1f 00
turbofrc 352x240  57 63 69 71 1f 00  77 79 7b 7f 1f 00
spinlbrk 352x240  57 68 6f 75 ff 01  77 78 7b 7f ff 00
aerofgtb 320x224  4f 5d 63 71 1f 00  6f 70 72 7c 1f 02
tail2nos 320x240  4f 5e 64 71 1f 09  7a 7c 7e 7f 1f 02
f1gp     320x240  4f 5e 64 71 1f 09  7a 7c 7e 7f 1f 02
welltris 352x240  57 63 69 71 1f 00  7a 7b 7e 7f 1f 00

games with 8x4 tiles:

pipedrm  352x240  57 63 69 71 1f 00  7a 7b 7e 7f 1f 00 * register 0b also briefly toggled to ff
hatris   352x240  57 63 69 71 1f 00  7a 7b 7e 7f 1f 00 * register 0b also briefly toggled to ff
idolmj   352x240  57 63 69 71 1f 00  7a 7b 7e 7f 1f 00
mjnatsu  352x240  57 63 69 71 1f 00  7a 7b 7e 7f 1f 00 * register 0b also briefly toggled to ff
mfunclub 352x240  57 63 69 71 1f 00  7a 7b 7e 7f 1f 00 * register 0b also briefly toggled to ff
daiyogen 352x240  57 63 69 71 1f 00  7a 7b 7e 7f 1f 00 * register 0b also briefly toggled to ff
nmsengen 352x240  57 63 69 71 1f 00  7a 7b 7e 7f 1f 00 * register 0b also briefly toggled to ff
fromance 352x240  57 63 69 71 1f 00  7a 7b 7e 7f 1f 00 * register 0b also briefly toggled to ff

register 00 could be screen width / 4 (hblank start?)
register 08 could be screen height / 2 (vblank start?)


2007.08.25: Small note regarding DipSwitches. Locations and values have been verified for:

- svolly91 (PCB Infos from the dumper),
- aerofgt (manual),
- karatblz (US manual),
- spinlbrk (US manual),
- turbofrc (US manual)

Verification still needed for the other PCBs.

***************************************************************************/

#include "emu.h"
#include "aerofgt.h"

#include "vs9209.h"
#include "vsystem_gga.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/mb3773.h"
#include "sound/ymopm.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"

#include "screen.h"
#include "speaker.h"


void karatblzbl_state::soundlatch_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

uint8_t aerofgt_sound_cpu_state::soundlatch_pending_r()
{
	return m_soundlatch->pending_r();
}

void aerofgt_sound_cpu_state::soundlatch_pending_w(int state)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);

	// sound comms is 2-way (see soundlatch_pending_r),
	// NMI routine is very short, so briefly set perfect_quantum to make sure that the timing is right
	if (state)
		machine().scheduler().perfect_quantum(attotime::from_usec(100));
}

void aerofgt_banked_sound_state::sh_bankswitch_w(uint8_t data)
{
	m_soundbank->set_entry(data & 0x03);
}

void aerofgt_base_state::pspikesb_oki_banking_w(uint16_t data)
{
	m_okibank->set_entry(data & 3);
}

// TODO: sound banking.
void aerofgt_base_state::aerfboo2_okim6295_banking_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
//  if(ACCESSING_BITS_8_15)
//  {
//      m_oki->set_rom_bank((data & 0xf00)>>8);
//  }
}

void aerofgt_sound_cpu_state::aerfboot_okim6295_banking_w(uint8_t data)
{
	// bit 2 (0x4) set too?
	if (BIT(data, 2))
		m_okibank->set_entry(data & 0x3);
}

void karatblzbl_state::d7759_write_port_0_w(uint8_t data)
{
	m_upd7759->port_w(data);
	m_upd7759->start_w(0);
	m_upd7759->start_w(1);
}

void karatblzbl_state::d7759_reset_w(uint8_t data)
{
	m_upd7759->reset_w(BIT(data, 7));
}

template<int Layer>
void aerofgt_base_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	m_tilemap[Layer]->mark_tile_dirty(offset);
}

template<int Layer>
void aerofgt_base_state::scrollx_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scrollx[Layer]);
}

template<int Layer>
void aerofgt_base_state::scrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scrolly[Layer]);
}

void aerofgt_banked_sound_state::pspikes_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram(); // work RAM
	map(0x200000, 0x203fff).ram().share(m_sprlookupram[0]);
	map(0xff8000, 0xff8fff).ram().w(FUNC(aerofgt_banked_sound_state::vram_w<0>)).share("vram.0");
	map(0xffc000, 0xffc3ff).writeonly().share(m_spriteram);
	map(0xffd000, 0xffdfff).ram().share(m_rasterram);   // bg1 scroll registers
	map(0xffe000, 0xffefff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xfff000, 0xfff001).portr("IN0");
	map(0xfff001, 0xfff001).w(FUNC(aerofgt_banked_sound_state::pspikes_palette_bank_w));
	map(0xfff002, 0xfff003).portr("IN1");
	map(0xfff003, 0xfff003).w(FUNC(aerofgt_banked_sound_state::pspikes_gfxbank_w));
	map(0xfff004, 0xfff005).portr("DSW").w(FUNC(aerofgt_banked_sound_state::scrolly_w<0>));
	map(0xfff007, 0xfff007).r(FUNC(aerofgt_banked_sound_state::soundlatch_pending_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask16(0x00ff);
	map(0xfff400, 0xfff403).w("gga", FUNC(vsystem_gga_device::write)).umask16(0x00ff);
}

void aerofgt_base_state::pspikesb_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram(); // work RAM
	map(0x200000, 0x203fff).ram().share(m_sprlookupram[0]);
	map(0xc04000, 0xc04001).nopw();
	map(0xff8000, 0xff8fff).ram().w(FUNC(aerofgt_base_state::vram_w<0>)).share("vram.0");
	map(0xffc000, 0xffcbff).ram().share(m_spriteram);
	map(0xffd000, 0xffdfff).ram().share(m_rasterram);   // bg1 scroll registers
	map(0xffd200, 0xffd201).w(FUNC(aerofgt_base_state::pspikesb_gfxbank_w));
	map(0xffe000, 0xffefff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xfff000, 0xfff001).portr("IN0");
	map(0xfff002, 0xfff003).portr("IN1");
	map(0xfff004, 0xfff005).portr("DSW").w(FUNC(aerofgt_base_state::scrolly_w<0>));
	map(0xfff007, 0xfff007).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xfff008, 0xfff009).w(FUNC(aerofgt_base_state::pspikesb_oki_banking_w));
	map(0xfff400, 0xfff403).nopw(); // GGA access
}

void spikes91_state::spikes91_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram(); // work RAM
	map(0x200000, 0x203fff).ram().share(m_sprlookupram[0]);
	map(0xc04000, 0xc04001).nopw();
	map(0xff8000, 0xff8fff).ram().w(FUNC(spikes91_state::vram_w<0>)).share("vram.0");

	map(0xffa000, 0xffbfff).ram().share(m_tx_tilemap_ram);

	map(0xffc000, 0xffcfff).ram().share(m_spriteram);
	//map(0xffd200, 0xffd201).w(FUNC(spikes91_state::pspikesb_gfxbank_w));
	map(0xffd000, 0xffdfff).ram().share(m_rasterram);   // bg1 scroll registers
	map(0xffe000, 0xffefff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xfff000, 0xfff001).portr("IN0");
	map(0xfff002, 0xfff003).portr("IN1");
	map(0xfff003, 0xfff003).w(FUNC(spikes91_state::pspikes_gfxbank_w));
	map(0xfff004, 0xfff005).portr("DSW").w(FUNC(spikes91_state::scrolly_w<0>));
	map(0xfff007, 0xfff007).r(FUNC(spikes91_state::soundlatch_pending_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask16(0x00ff);
	map(0xfff008, 0xfff009).w(FUNC(spikes91_state::spikes91_lookup_w));
}

void aerofgt_base_state::pspikesc_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram(); // work RAM
	map(0x200000, 0x203fff).ram().share(m_sprlookupram[0]);
	map(0xff8000, 0xff8fff).ram().w(FUNC(aerofgt_base_state::vram_w<0>)).share("vram.0");
	map(0xffc000, 0xffcbff).ram().share(m_spriteram);
	map(0xffd000, 0xffdfff).ram().share(m_rasterram);   // bg1 scroll registers
	map(0xffe000, 0xffefff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xfff000, 0xfff001).portr("IN0");
	map(0xfff001, 0xfff001).w(FUNC(aerofgt_base_state::pspikes_palette_bank_w));
	map(0xfff002, 0xfff003).portr("IN1");
	map(0xfff003, 0xfff003).w(FUNC(aerofgt_base_state::pspikes_gfxbank_w));
	map(0xfff004, 0xfff005).portr("DSW");
	map(0xfff004, 0xfff005).w(FUNC(aerofgt_base_state::scrolly_w<0>));
	map(0xfff007, 0xfff007).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xfff400, 0xfff403).nopw(); // GGA access
}

void aerofgt_sound_cpu_state::kickball_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram(); // work RAM
	map(0x200000, 0x20ffff).ram().share(m_sprlookupram[0]);
	map(0xff8000, 0xff8fff).ram().w(FUNC(aerofgt_sound_cpu_state::vram_w<0>)).share("vram.0");
	map(0xffc000, 0xffc3ff).writeonly().share(m_spriteram);
	map(0xffd000, 0xffdfff).ram().share(m_rasterram);   // bg1 scroll registers
	map(0xffe000, 0xffefff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xfff000, 0xfff001).portr("IN0");
	map(0xfff001, 0xfff001).w(FUNC(aerofgt_sound_cpu_state::pspikes_palette_bank_w));
	map(0xfff002, 0xfff003).portr("IN1");
	map(0xfff003, 0xfff003).w(FUNC(aerofgt_sound_cpu_state::kickball_gfxbank_w));
	map(0xfff004, 0xfff005).portr("DSW").w(FUNC(aerofgt_sound_cpu_state::scrolly_w<0>));
	map(0xfff007, 0xfff007).r(FUNC(aerofgt_sound_cpu_state::soundlatch_pending_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask16(0x00ff);
	map(0xfff400, 0xfff403).nopw(); // GGA access
}

void aerofgt_banked_sound_state::karatblz_map(address_map &map)
{
	map.global_mask(0xfffff);
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x081fff).ram().w(FUNC(aerofgt_banked_sound_state::vram_w<0>)).share("vram.0");
	map(0x082000, 0x083fff).ram().w(FUNC(aerofgt_banked_sound_state::vram_w<1>)).share("vram.1");
	map(0x0a0000, 0x0affff).ram().share(m_sprlookupram[0]);
	map(0x0b0000, 0x0bffff).ram().share(m_sprlookupram[1]);
	map(0x0c0000, 0x0cffff).ram(); // work RAM
	map(0x0f8000, 0x0fbfff).ram(); // work RAM
	map(0x0fc000, 0x0fc7ff).ram().share(m_spriteram);
	map(0x0fe000, 0x0fe7ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0ff000, 0x0ff001).portr("IN0");
	map(0x0ff000, 0x0ff000).w(FUNC(aerofgt_banked_sound_state::spinlbrk_flip_screen_w));
	map(0x0ff002, 0x0ff003).portr("IN1");
	map(0x0ff002, 0x0ff002).w(FUNC(aerofgt_banked_sound_state::karatblz_gfxbank_w));
	map(0x0ff004, 0x0ff005).portr("IN2");
	map(0x0ff006, 0x0ff007).portr("IN3");
	map(0x0ff007, 0x0ff007).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x0ff008, 0x0ff009).portr("DSW").w(FUNC(aerofgt_banked_sound_state::scrollx_w<0>));
	map(0x0ff00b, 0x0ff00b).r(FUNC(aerofgt_banked_sound_state::soundlatch_pending_r));
	map(0x0ff00a, 0x0ff00b).w(FUNC(aerofgt_banked_sound_state::scrolly_w<0>));
	map(0x0ff00c, 0x0ff00d).w(FUNC(aerofgt_banked_sound_state::scrollx_w<1>));
	map(0x0ff00e, 0x0ff00f).w(FUNC(aerofgt_banked_sound_state::scrolly_w<1>));
	map(0x0ff400, 0x0ff403).w("gga", FUNC(vsystem_gga_device::write)).umask16(0x00ff);
}

void karatblzbl_state::main_map(address_map &map)
{
	map.global_mask(0xfffff);
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x081fff).ram().w(FUNC(karatblzbl_state::vram_w<0>)).share("vram.0");
	map(0x082000, 0x083fff).ram().w(FUNC(karatblzbl_state::vram_w<1>)).share("vram.1");
	map(0x0a0000, 0x0affff).ram().share(m_sprlookupram[0]);
	map(0x0b0000, 0x0bffff).ram().share(m_sprlookupram[1]);
	map(0x0c0000, 0x0cffff).ram(); // work RAM
	map(0x0f8000, 0x0fbfff).ram(); // work RAM
	map(0x0fc000, 0x0fc7ff).ram().share(m_spriteram);
	map(0x0fe000, 0x0fe7ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0ff000, 0x0ff001).portr("IN0");
	map(0x0ff000, 0x0ff000).w(FUNC(karatblzbl_state::spinlbrk_flip_screen_w));
	map(0x0ff002, 0x0ff003).portr("IN1");
	map(0x0ff002, 0x0ff002).w(FUNC(karatblzbl_state::karatblz_gfxbank_w));
	map(0x0ff004, 0x0ff005).portr("IN2");
	map(0x0ff006, 0x0ff007).portr("IN3");
	map(0x0ff007, 0x0ff007).w(FUNC(karatblzbl_state::soundlatch_w));
	map(0x0ff008, 0x0ff009).portr("DSW").w(FUNC(karatblzbl_state::scrollx_w<0>));
	map(0x0ff00b, 0x0ff00b).r(FUNC(karatblzbl_state::soundlatch_pending_r));
	map(0x0ff00a, 0x0ff00b).w(FUNC(karatblzbl_state::scrolly_w<0>));
	map(0x0ff00c, 0x0ff00d).w(FUNC(karatblzbl_state::scrollx_w<1>));
	map(0x0ff00e, 0x0ff00f).w(FUNC(karatblzbl_state::scrolly_w<1>));
	map(0x0ff400, 0x0ff403).nopw(); // GGA access
}

void aerofgt_banked_sound_state::spinlbrk_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x080fff).ram().w(FUNC(aerofgt_banked_sound_state::vram_w<0>)).share("vram.0");
	map(0x082000, 0x082fff).ram().w(FUNC(aerofgt_banked_sound_state::vram_w<1>)).share("vram.1");
	map(0xff8000, 0xffbfff).ram(); // work RAM
	map(0xffc000, 0xffc7ff).ram().share(m_spriteram);
	map(0xffd000, 0xffd1ff).ram().share(m_rasterram);   // bg1 scroll registers
	map(0xffe000, 0xffe7ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xfff000, 0xfff001).portr("IN0");
	map(0xfff000, 0xfff000).w(FUNC(aerofgt_banked_sound_state::spinlbrk_flip_screen_w));
	map(0xfff001, 0xfff001).w(FUNC(aerofgt_banked_sound_state::spinlbrk_gfxbank_w));
	map(0xfff002, 0xfff003).portr("IN1").w(FUNC(aerofgt_banked_sound_state::scrollx_w<1>));
	map(0xfff004, 0xfff005).portr("DSW");
	map(0xfff007, 0xfff007).w(m_soundlatch, FUNC(generic_latch_8_device::write));
//  map(0xfff008, 0xfff009); - read when analog inputs are enabled
//  map(0xfff00a, 0xfff00b); /
	map(0xfff400, 0xfff403).w("gga", FUNC(vsystem_gga_device::write)).umask16(0x00ff);
}

void aerofgt_banked_sound_state::turbofrc_map(address_map &map)
{
	map.global_mask(0xfffff);
	map(0x000000, 0x0bffff).rom();
	map(0x0c0000, 0x0cffff).ram(); // work RAM
	map(0x0d0000, 0x0d1fff).ram().w(FUNC(aerofgt_banked_sound_state::vram_w<0>)).share("vram.0");
	map(0x0d2000, 0x0d3fff).ram().w(FUNC(aerofgt_banked_sound_state::vram_w<1>)).share("vram.1");
	map(0x0e0000, 0x0e3fff).ram().share(m_sprlookupram[0]);
	map(0x0e4000, 0x0e7fff).ram().share(m_sprlookupram[1]);
	map(0x0f8000, 0x0fbfff).ram(); // work RAM
	map(0x0fc000, 0x0fc7ff).ram().share(m_spriteram);
	map(0x0fd000, 0x0fdfff).ram().share(m_rasterram);   // bg1 scroll registers
	map(0x0fe000, 0x0fe7ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0ff000, 0x0ff001).portr("IN0");
	map(0x0ff001, 0x0ff001).w(FUNC(aerofgt_banked_sound_state::turbofrc_flip_screen_w));
	map(0x0ff002, 0x0ff003).portr("IN1").w(FUNC(aerofgt_banked_sound_state::scrolly_w<0>));
	map(0x0ff004, 0x0ff005).portr("DSW").w(FUNC(aerofgt_banked_sound_state::scrollx_w<1>));
	map(0x0ff007, 0x0ff007).r(FUNC(aerofgt_banked_sound_state::soundlatch_pending_r));
	map(0x0ff006, 0x0ff007).w(FUNC(aerofgt_banked_sound_state::scrolly_w<1>));
	map(0x0ff008, 0x0ff009).portr("IN2");
	map(0x0ff008, 0x0ff00b).w(FUNC(aerofgt_banked_sound_state::turbofrc_gfxbank_w));
	map(0x0ff00c, 0x0ff00d).nopw();    // related to bg2 (written together with the scroll registers)
	map(0x0ff00e, 0x0ff00e).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x0ff400, 0x0ff403).w("gga", FUNC(vsystem_gga_device::write)).umask16(0x00ff);
}

void aerofgt_banked_sound_state::aerofgtb_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x0c0000, 0x0cffff).ram(); // work RAM
	map(0x0d0000, 0x0d1fff).ram().w(FUNC(aerofgt_banked_sound_state::vram_w<0>)).share("vram.0");
	map(0x0d2000, 0x0d3fff).ram().w(FUNC(aerofgt_banked_sound_state::vram_w<1>)).share("vram.1");
	map(0x0e0000, 0x0e3fff).ram().share(m_sprlookupram[0]);
	map(0x0e4000, 0x0e7fff).ram().share(m_sprlookupram[1]);
	map(0x0f8000, 0x0fbfff).ram(); // work RAM
	map(0x0fc000, 0x0fc7ff).ram().share(m_spriteram);
	map(0x0fd000, 0x0fd7ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0fe000, 0x0fe001).portr("IN0");
	map(0x0fe001, 0x0fe001).w(FUNC(aerofgt_banked_sound_state::turbofrc_flip_screen_w));
	map(0x0fe002, 0x0fe003).portr("IN1").w(FUNC(aerofgt_banked_sound_state::scrolly_w<0>));
	map(0x0fe004, 0x0fe005).portr("DSW1").w(FUNC(aerofgt_banked_sound_state::scrollx_w<1>));
	map(0x0fe007, 0x0fe007).r(FUNC(aerofgt_banked_sound_state::soundlatch_pending_r));
	map(0x0fe006, 0x0fe007).w(FUNC(aerofgt_banked_sound_state::scrolly_w<1>));
	map(0x0fe008, 0x0fe009).portr("DSW2");
	map(0x0fe008, 0x0fe00b).w(FUNC(aerofgt_banked_sound_state::turbofrc_gfxbank_w));
	map(0x0fe00e, 0x0fe00e).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x0fe400, 0x0fe403).w("gga", FUNC(vsystem_gga_device::write)).umask16(0x00ff);
	map(0x0ff000, 0x0fffff).ram().share(m_rasterram);   // used only for the scroll registers
}

void aerofgt_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x1a0000, 0x1a07ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x1b0000, 0x1b07ff).ram().share(m_rasterram);   // used only for the scroll registers
	map(0x1b0800, 0x1b0801).ram(); // tracks watchdog state
	map(0x1b0ff0, 0x1b0fff).ram(); // stack area during boot
	map(0x1b2000, 0x1b3fff).ram().w(FUNC(aerofgt_state::vram_w<0>)).share("vram.0");
	map(0x1b4000, 0x1b5fff).ram().w(FUNC(aerofgt_state::vram_w<1>)).share("vram.1");
	map(0x1c0000, 0x1c7fff).ram().share(m_sprlookupram[0]);
	map(0x1d0000, 0x1d1fff).ram().share(m_spriteram);
	map(0xfef000, 0xffefff).ram(); // work RAM
	map(0xffff80, 0xffff87).w(FUNC(aerofgt_state::gfxbank_w));
	map(0xffff88, 0xffff89).w(FUNC(aerofgt_state::scrolly_w<0>)); // + something else in the top byte
	map(0xffff90, 0xffff91).w(FUNC(aerofgt_state::scrolly_w<1>)); // + something else in the top byte
	map(0xffffa0, 0xffffbf).rw("io", FUNC(vs9209_device::read), FUNC(vs9209_device::write)).umask16(0x00ff);
	map(0xffffc1, 0xffffc1).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}

void aerofgt_sound_cpu_state::aerfboot_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x0c0000, 0x0cffff).ram(); // work RAM
	map(0x0d0000, 0x0d1fff).ram().w(FUNC(aerofgt_sound_cpu_state::vram_w<0>)).share("vram.0");
	map(0x0d2000, 0x0d3fff).ram().w(FUNC(aerofgt_sound_cpu_state::vram_w<1>)).share("vram.1");
	map(0x0e0000, 0x0e3fff).ram().share(m_sprlookupram[0]);
	map(0x0e4000, 0x0e7fff).ram().share(m_sprlookupram[1]);
	map(0x0f8000, 0x0fbfff).ram(); // work RAM
	map(0x0fc000, 0x0fc7ff).ram(); //.share(m_spriteram);
	map(0x0fd000, 0x0fd7ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0fe000, 0x0fe001).portr("IN0");
	map(0x0fe002, 0x0fe003).portr("IN1");
	map(0x0fe004, 0x0fe005).portr("DSW1");
	map(0x0fe008, 0x0fe009).portr("DSW2");
	map(0x0fe002, 0x0fe003).w(FUNC(aerofgt_sound_cpu_state::scrolly_w<0>));
	map(0x0fe004, 0x0fe005).w(FUNC(aerofgt_sound_cpu_state::scrollx_w<1>));
	map(0x0fe006, 0x0fe007).w(FUNC(aerofgt_sound_cpu_state::scrolly_w<1>));
	map(0x0fe008, 0x0fe00b).w(FUNC(aerofgt_sound_cpu_state::turbofrc_gfxbank_w));
	map(0x0fe00e, 0x0fe00e).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x0fe010, 0x0fe011).nopw();
	map(0x0fe012, 0x0fe013).nopw(); // MSB = watchdog?
	map(0x0fe400, 0x0fe403).nopw(); // GGA access
	map(0x0ff000, 0x0fffff).ram().share(m_rasterram);   // used only for the scroll registers
	map(0x100000, 0x107fff).nopw();
	map(0x108000, 0x10bfff).ram().share(m_spriteram);
	map(0x10c000, 0x117fff).nopw();
}

void aerofgt_base_state::aerfboo2_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x0c0000, 0x0cffff).ram(); // work RAM
	map(0x0d0000, 0x0d1fff).ram().w(FUNC(aerofgt_base_state::vram_w<0>)).share("vram.0");
	map(0x0d2000, 0x0d3fff).ram().w(FUNC(aerofgt_base_state::vram_w<1>)).share("vram.1");
	map(0x0e0000, 0x0e3fff).ram().share(m_sprlookupram[0]);
	map(0x0e4000, 0x0e7fff).ram().share(m_sprlookupram[1]);
	map(0x0f8000, 0x0fbfff).ram(); // work RAM
	map(0x0fc000, 0x0fc7ff).ram().share(m_spriteram);
	map(0x0fd000, 0x0fd7ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0fe000, 0x0fe001).portr("IN0");
	map(0x0fe002, 0x0fe003).portr("IN1");
	map(0x0fe004, 0x0fe005).portr("DSW1");
	map(0x0fe008, 0x0fe009).portr("DSW2");
	map(0x0fe002, 0x0fe003).w(FUNC(aerofgt_base_state::scrolly_w<0>));
	map(0x0fe004, 0x0fe005).w(FUNC(aerofgt_base_state::scrollx_w<1>));
	map(0x0fe006, 0x0fe007).w(FUNC(aerofgt_base_state::scrolly_w<1>));
	map(0x0fe008, 0x0fe00b).w(FUNC(aerofgt_base_state::turbofrc_gfxbank_w));
	map(0x0fe006, 0x0fe006).r(m_oki, FUNC(okim6295_device::read));
	map(0x0fe00e, 0x0fe00e).w(m_oki, FUNC(okim6295_device::write));
	map(0x0fe01e, 0x0fe01f).w(FUNC(aerofgt_base_state::aerfboo2_okim6295_banking_w));
//  map(0x0fe010, 0x0fe011).nopw();
//  map(0x0fe012, 0x0fe013).w(FUNC(aerofgt_base_state::aerfboot_soundlatch_w));
	map(0x0fe400, 0x0fe403).nopw(); // GGA access
	map(0x0ff000, 0x0fffff).ram().share(m_rasterram);   // used only for the scroll registers
}

void wbbc97_state::main_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x500000, 0x50ffff).ram(); // work RAM
	map(0x600000, 0x605fff).ram().share(m_sprlookupram[0]);
	map(0xa00000, 0xa3ffff).ram().share(m_bitmapram);
	map(0xff8000, 0xff8fff).ram().w(FUNC(wbbc97_state::vram_w<0>)).share("vram.0");
	map(0xffc000, 0xffc3ff).writeonly().share(m_spriteram);
	map(0xffd000, 0xffdfff).ram().share(m_rasterram);   // bg1 scroll registers
	map(0xffe000, 0xffefff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xfff000, 0xfff001).portr("IN0");
	map(0xfff001, 0xfff001).w(FUNC(wbbc97_state::pspikes_palette_bank_w));
	map(0xfff002, 0xfff003).portr("IN1");
	map(0xfff003, 0xfff003).w(FUNC(wbbc97_state::pspikes_gfxbank_w));
	map(0xfff004, 0xfff005).portr("DSW").w(FUNC(wbbc97_state::scrolly_w<0>));
	map(0xfff007, 0xfff007).r(FUNC(wbbc97_state::soundlatch_pending_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask16(0x00ff);
	map(0xfff00e, 0xfff00f).w(FUNC(wbbc97_state::bitmap_enable_w));
	map(0xfff400, 0xfff403).nopw(); // GGA access
}

void aerofgt_banked_sound_state::sound_map(address_map &map)
{
	map(0x0000, 0x77ff).rom().region("audiocpu", 0);
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_soundbank);
}

void aerofgt_banked_sound_state::spinlbrk_sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(aerofgt_banked_sound_state::sh_bankswitch_w));
	map(0x14, 0x14).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
	map(0x18, 0x1b).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
}

void aerofgt_banked_sound_state::aerofgt_sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0x04, 0x04).w(FUNC(aerofgt_banked_sound_state::sh_bankswitch_w));
	map(0x08, 0x08).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
	map(0x0c, 0x0c).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void aerofgt_sound_cpu_state::aerfboot_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).w(FUNC(aerofgt_sound_cpu_state::aerfboot_okim6295_banking_w));
	map(0x9800, 0x9800).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void wbbc97_state::sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xf810, 0xf811).w("ymsnd", FUNC(ym3812_device::write));
	map(0xfc00, 0xfc00).noprw();
	map(0xfc20, 0xfc20).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void karatblzbl_state::sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r(m_soundlatch, FUNC(generic_latch_8_device::read)); //.w("soundlatch2", FUNC(generic_latch_8_device::write));
}

void karatblzbl_state::sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("ymsnd", FUNC(ym3812_device::status_r), FUNC(ym3812_device::address_w));
	map(0x20, 0x20).w("ymsnd", FUNC(ym3812_device::data_w));
	map(0x40, 0x40).w(FUNC(karatblzbl_state::d7759_write_port_0_w));
	map(0x80, 0x80).w(FUNC(karatblzbl_state::d7759_reset_w));
}

void aerofgt_sound_cpu_state::kickball_sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void aerofgt_sound_cpu_state::kickball_sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("ymsnd", FUNC(ym3812_device::status_r), FUNC(ym3812_device::address_w));
	map(0x20, 0x20).w("ymsnd", FUNC(ym3812_device::data_w));
	map(0x40, 0x40).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc0, 0xc0).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
}

void spikes91_state::spikes91_sound_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe800, 0xe800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	//map(0xf000, 0xf000) // OKI M5205?
	//map(0xf400, 0xf400) // OKI M5205?
	map(0xf800, 0xffff).ram();
}

void aerofgt_base_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_okibank);
}

static INPUT_PORTS_START( pspikes )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" )                // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" )                // Listed as "Unused"
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	// According to Super Volley '91 PCB Infos, here DSW2 starts
	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPNAME( 0x0600, 0x0600, "1 Player Starting Score" )       PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0600, "12-12" )
	PORT_DIPSETTING(      0x0400, "11-11" )
	PORT_DIPSETTING(      0x0200, "11-12" )
	PORT_DIPSETTING(      0x0000, "10-12" )
	PORT_DIPNAME( 0x1800, 0x1800, "2 Players Starting Score" )      PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x1800, "9-9" )
	PORT_DIPSETTING(      0x1000, "7-7" )
	PORT_DIPSETTING(      0x0800, "5-5" )
	PORT_DIPSETTING(      0x0000, "0-0" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	// When the players continue, only half of the time in SW2:7 is added
	PORT_DIPNAME( 0x4000, 0x4000, "2 Players Time Per Credit" )     PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "3 min" )
	PORT_DIPSETTING(      0x0000, "2 min" )
	// The next one is reported as 'Must be off' in Super Volley '91 PCB Infos
	PORT_DIPNAME( 0x8000, 0x8000, "Debug" )                         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pspikesb )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	// Dips bank 1
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )
	// Dips bank 2
	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW2:1" )
	PORT_DIPNAME( 0x0600, 0x0600, "1 Player Starting Score" )   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0600, "12-12" )
	PORT_DIPSETTING(      0x0400, "11-11" )
	PORT_DIPSETTING(      0x0200, "11-12" )
	PORT_DIPSETTING(      0x0000, "10-12" )
	PORT_DIPNAME( 0x1800, 0x1800, "2 Players Starting Score" )  PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x1800, "9-9" )
	PORT_DIPSETTING(      0x1000, "7-7" )
	PORT_DIPSETTING(      0x0800, "5-5" )
	PORT_DIPSETTING(      0x0000, "0-0" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, "2 Players Time Per Credit" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "3 min" )
	PORT_DIPSETTING(      0x0000, "2 min" )
	PORT_DIPNAME( 0x8000, 0x8000, "Debug" )                     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pspikesc )
	PORT_INCLUDE( pspikes )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Region ) )                   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0030, DEF_STR( China ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Taiwan ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( China ) )
INPUT_PORTS_END


static INPUT_PORTS_START( karatblz )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:1,2,3")  // It affects Coin 1, 2, 3 and 4
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Continue Coin" )             PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, "Start 1 Coin/Continue 1 Coin" )
	PORT_DIPSETTING(      0x0000, "Start 2 Coin/Continue 1 Coin" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW1:6,7") // Game Type
	PORT_DIPSETTING(      0x0060, "2 Players" )     // 1 Unit / 2 Players
	PORT_DIPSETTING(      0x0040, "3 Players" )     // 1 Unit / 3 Players
	PORT_DIPSETTING(      0x0020, "4 Players" )     // 1 Unit / 4 Players
	PORT_DIPSETTING(      0x0000, "4 Players (Team)" )  // 2 Units / 4 Players
	/*  With 4 player (Team) selected and Same Coin Slot:
	    Coin A & B credit together for use by _only_ player 1 or player 2
	    Coin C & D credit together for use by _only_ player 3 or player 4
	    Otherwise with Individual selected, everyone is separate  */
	PORT_DIPNAME( 0x0080, 0x0080, "Coin Slot" )                 PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, "Same" )
	PORT_DIPSETTING(      0x0000, "Individual" )
	// According to Turbo Force manual, here DSW2 starts
	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPNAME( 0x0600, 0x0600, "Number of Enemies" )         PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1800, 0x1800, "Strength of Enemies" )       PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	// Listed in manual as N.C. (aka No Connection)
	PORT_DIPNAME( 0x2000, 0x2000, "Freeze" )                    PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( karatblzu )
	PORT_INCLUDE( karatblz )

	PORT_MODIFY("DSW")
	// Suggested default in US manual is DEF_STR( Hard ) for these two
	PORT_DIPNAME( 0x0600, 0x0200, "Number of Enemies" )         PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1800, 0x0800, "Strength of Enemies" )       PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( spinlbrk )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x000f, "1 Credit 1 Health Pack" )    // I chose "Health Packs" as the actual value can change
	PORT_DIPSETTING(      0x000e, "1 Credit 2 Health Packs" )   //  via dipswitch 2-7 (0x4000) see below
	PORT_DIPSETTING(      0x000d, "1 Credit 3 Health Packs" )
	PORT_DIPSETTING(      0x000c, "1 Credit 4 Health Packs" )
	PORT_DIPSETTING(      0x000b, "1 Credit 5 Health Packs" )
	PORT_DIPSETTING(      0x000a, "1 Credit 6 Health Packs" )
	PORT_DIPSETTING(      0x0009, "2 Credits 1 Health Pack" )
	PORT_DIPSETTING(      0x0008, "3 Credits 1 Health Pack" )
	PORT_DIPSETTING(      0x0007, "4 Credits 1 Health Pack" )
	PORT_DIPSETTING(      0x0006, "5 Credits 1 Health Pack" )
	PORT_DIPSETTING(      0x0005, "2 Credits 2 Health Packs" )
	PORT_DIPSETTING(      0x0004, "2-1-1C  1-1-1 HPs" )
	PORT_DIPSETTING(      0x0003, "2-2C 1-2 HPs" )
	PORT_DIPSETTING(      0x0002, "1-1-1-1-1C 1-1-1-1-2 HPs" )
	PORT_DIPSETTING(      0x0001, "1-1-1-1C 1-1-1-2 HPs" )
	PORT_DIPSETTING(      0x0000, "1-1C 1-2 HPs" )
	/* The last 5 Coin/Credit selections are cycles:
	    Example: 0x0004 = 2-1-1C 1-1-1 HPs:
	    2 Credits for the 1st Health Pack, 1 Credit for the 2nd Health Pack, 1 Credit
	    for the 3rd Health Pack... Then back to 2 Credits again for 1 HP, then 1 credit
	    and 1 credit.... on and on.  With all Coin/Credit dips set to on, it's 1 Health
	    Pack for odd credits, 2 Health Packs for even credits :p
	    */
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0x00f0, "1 Credit 1 Health Pack" )
	PORT_DIPSETTING(      0x00e0, "1 Credit 2 Health Packs" )
	PORT_DIPSETTING(      0x00d0, "1 Credit 3 Health Packs" )
	PORT_DIPSETTING(      0x00c0, "1 Credit 4 Health Packs" )
	PORT_DIPSETTING(      0x00b0, "1 Credit 5 Health Packs" )
	PORT_DIPSETTING(      0x00a0, "1 Credit 6 Health Packs" )
	PORT_DIPSETTING(      0x0090, "2 Credits 1 Health Pack" )
	PORT_DIPSETTING(      0x0080, "3 Credits 1 Health Pack" )
	PORT_DIPSETTING(      0x0070, "4 Credits 1 Health Pack" )
	PORT_DIPSETTING(      0x0060, "5 Credits 1 Health Pack" )
	PORT_DIPSETTING(      0x0050, "2 Credits 2 Health Packs" )
	PORT_DIPSETTING(      0x0040, "2-1-1C  1-1-1 HPs" )
	PORT_DIPSETTING(      0x0030, "2-2C 1-2 HPs" )
	PORT_DIPSETTING(      0x0020, "1-1-1-1-1C 1-1-1-1-2 HPs" )
	PORT_DIPSETTING(      0x0010, "1-1-1-1C 1-1-1-2 HPs" )
	PORT_DIPSETTING(      0x0000, "1-1C 1-2 HPs" )
	// According to Spinal Breakers manual, here DSW2 starts
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Credits For Extra Hitpoints" )   PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Lever Type" )                    PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, "Digital" )
	PORT_DIPSETTING(      0x0000, "Analog" )                        // This setting causes lever error???
	PORT_SERVICE_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPNAME( 0x4000, 0x4000, "Health Pack" )                   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "32 Hitpoints" )
	PORT_DIPSETTING(      0x0000, "40 Hitpoints" )
	// Default in US manual is "5 points"
	PORT_DIPNAME( 0x8000, 0x8000, "Life Restoration" )              PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, "10 Points" )
	PORT_DIPSETTING(      0x0000, "5 Points" )
INPUT_PORTS_END

static INPUT_PORTS_START( spinlbrku )
	PORT_INCLUDE(spinlbrk)

	PORT_MODIFY("DSW")
	// Suggested default in US manual is DEF_STR( Hardest )
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Health Pack" )                   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "20 Hitpoints" )
	PORT_DIPSETTING(      0x0000, "32 Hitpoints" )
	PORT_DIPNAME( 0x8000, 0x0000, "Life Restoration" )              PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, "10 Points" )
	PORT_DIPSETTING(      0x0000, "5 Points" )
INPUT_PORTS_END

static INPUT_PORTS_START( turbofrc )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )  // COIN1 in service
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE )  // "TEST"
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )  // START1

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:1,2,3")  // It affects Coin 1, 2 and 3
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Continue Coin" )             PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, "Start 1 Coin/Continue 1 Coin" )
	PORT_DIPSETTING(      0x0000, "Start 2 Coin/Continue 1 Coin" )
	PORT_DIPNAME( 0x0010, 0x0000, "Coin Slot" )                 PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, "Same" )
	PORT_DIPSETTING(      0x0000, "Individual" )
	PORT_DIPNAME( 0x0020, 0x0000, "Play Mode" )                 PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, "2 Players" )
	PORT_DIPSETTING(      0x0000, "3 Players" )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	// According to Turbo Force manual, here DSW2 starts
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e00, 0x0800, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:2,3,4")
	PORT_DIPSETTING(      0x0e00, "1 (Easiest)")
	PORT_DIPSETTING(      0x0c00, "2" )
	PORT_DIPSETTING(      0x0a00, "3" )
	PORT_DIPSETTING(      0x0800, "4 (Normal)" )
	PORT_DIPSETTING(      0x0600, "5" )
	PORT_DIPSETTING(      0x0400, "6" )
	PORT_DIPSETTING(      0x0200, "7" )
	PORT_DIPSETTING(      0x0000, "8 (Hardest)" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x1000, "3" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, "200000" )
	PORT_DIPSETTING(      0x0000, "300000" )
	// The following 2 are listed in Turbo Force manual as N.C. (aka No Connection) and "Should be kept on OFF"
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )            // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )            // Listed as "Unused"
INPUT_PORTS_END

static INPUT_PORTS_START( aerofgtb )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	// Dips bank 1
	// "Free Play mode: Have SW1:1-8 ON."
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Slot" )             PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, "Same" )
	PORT_DIPSETTING(      0x0000, "Individual" )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x000a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x0050, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Continue Coin" )         PORT_DIPLOCATION("SW1:8") // "When ON SW1:2-7 are disabled."
	PORT_DIPSETTING(      0x0080, "Start 1 Coin/Continue 1 Coin" )
	PORT_DIPSETTING(      0x0000, "Start 2 Coin/Continue 1 Coin" )

	// Dips bank 2
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, "1" )
	PORT_DIPSETTING(      0x1000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "200000" )
	PORT_DIPSETTING(      0x0000, "300000" )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )

	// Dips bank 3 (not documented)
	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Region ) )               PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Taiwan ) )
	// TODO: there are others in the table at 11910
	// this port is checked at 1b080
INPUT_PORTS_END

static INPUT_PORTS_START( aerofgt )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	// "Free Play mode: Have SW1:1-8 ON."
	PORT_DIPNAME( 0x01, 0x01, "Coin Slot" )                 PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Same" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x80, 0x80, "Continue Coin" )             PORT_DIPLOCATION("SW1:8") // "When ON, SW1:2-7 are disabled."
	PORT_DIPSETTING(    0x80, "Start 1 Coin/Continue 1 Coin" )
	PORT_DIPSETTING(    0x00, "Start 2 Coin/Continue 1 Coin" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "200000" )
	PORT_DIPSETTING(    0x00, "300000" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )

	// Jumpers not documented in the Aero Fighters manual
	PORT_START("JP1")
	PORT_DIPNAME( 0xf, 0x0, DEF_STR( Region ) )
	PORT_DIPSETTING(   0x0, "Any" )
	PORT_DIPSETTING(   0xf, "USA/Canada" )
	PORT_DIPSETTING(   0xe, DEF_STR( Korea ) )
	PORT_DIPSETTING(   0xd, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(   0xb, DEF_STR( Taiwan ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wbbc97 )
	PORT_INCLUDE(pspikes)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END

static const gfx_layout pspikesb_charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout aerfboot_charlayout =
{
	8,8,
	RGN_FRAC(1,8),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout aerfboo2_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, RGN_FRAC(1,2)+1*4, RGN_FRAC(1,2)+0*4, 3*4, 2*4, RGN_FRAC(1,2)+3*4, RGN_FRAC(1,2)+2*4 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	32*4
};

static const gfx_layout pspikesb_spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};


static const gfx_layout aerfboot_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, RGN_FRAC(1,2)+2*4, RGN_FRAC(1,2)+3*4, 0*4, 1*4, RGN_FRAC(1,2)+0*4, RGN_FRAC(1,2)+1*4,
			6*4, 7*4, RGN_FRAC(1,2)+6*4, RGN_FRAC(1,2)+7*4, 4*4, 5*4, RGN_FRAC(1,2)+4*4, RGN_FRAC(1,2)+5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	64*8
};

static const gfx_layout aerfboo2_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0,1,2,3 },
	{ 28,24,20,16,12,8,4,0,60,56,52,48,44,40,36,32 },
	{
		0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64
	},
	16*64
};

static const gfx_layout kickball_spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 6,7, 4,5, 2,3, 0, 1, 14, 15, 12, 13, 10, 11, 8, 9 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8, 16*8, 18*8, 20*8, 22*8, 24*8, 26*8 ,28*8, 30*8 },
	32*8
};

static GFXDECODE_START( gfx_pspikes )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_lsb,      0, 64 )    // colors    0-1023 in 8 banks
	GFXDECODE_ENTRY( "gfx2", 0, gfx_16x16x4_packed_lsb, 1024, 64 )    // colors 1024-2047 in 4 banks
GFXDECODE_END

static GFXDECODE_START( gfx_pspikesb )
	GFXDECODE_ENTRY( "gfx1", 0, pspikesb_charlayout,      0, 64 )   // colors    0-1023 in 8 banks
	GFXDECODE_ENTRY( "gfx2", 0, pspikesb_spritelayout, 1024, 64 )   // colors 1024-2047 in 4 banks
GFXDECODE_END

static GFXDECODE_START( gfx_spikes91 )
	GFXDECODE_ENTRY( "gfx1", 0, pspikesb_charlayout,      0, 64 )   // colors    0-1023 in 8 banks
	GFXDECODE_ENTRY( "gfx2", 0, gfx_16x16x4_planar,    1024, 64 )   // colors 1024-2047 in 4 banks
GFXDECODE_END

static GFXDECODE_START( gfx_kickball )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_lsb,     0, 64 )    // colors    0-1023 in 8 banks
	GFXDECODE_ENTRY( "gfx2", 0, kickball_spritelayout, 1024, 64 )    // colors 1024-2047 in 4 banks
GFXDECODE_END

static GFXDECODE_START( gfx_turbofrc )
	GFXDECODE_ENTRY( "gfx1",      0, gfx_8x8x4_packed_lsb,     0, 16 )
	GFXDECODE_ENTRY( "gfx2",      0, gfx_8x8x4_packed_lsb,   256, 16 )
	GFXDECODE_ENTRY( "spritegfx", 0, gfx_16x16x4_packed_lsb, 512, 16 )
	GFXDECODE_ENTRY( "gfx4",      0, gfx_16x16x4_packed_lsb, 768, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_aerofgt )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_msb,     0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_msb,   256, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_16x16x4_packed_msb, 512, 32 )
GFXDECODE_END

static GFXDECODE_START( gfx_aerfboot )
	GFXDECODE_ENTRY( "gfx1",      0,       aerfboot_charlayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx1",      0x20000, aerfboot_charlayout,   256, 16 )
	GFXDECODE_ENTRY( "gfx2",      0,       aerfboot_spritelayout, 512, 16 )
	GFXDECODE_ENTRY( "spritegfx", 0,       aerfboot_spritelayout, 768, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_aerfboo2 )
	GFXDECODE_ENTRY( "gfx1",      0,       aerfboo2_charlayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2",      0,       aerfboo2_charlayout,   256, 16 )
	GFXDECODE_ENTRY( "spritegfx", 0,       aerfboo2_spritelayout, 512, 16 )
	GFXDECODE_ENTRY( "spritegfx", 0x100000,aerfboo2_spritelayout, 768, 16 )
GFXDECODE_END


void aerofgt_banked_sound_state::machine_start()
{
	aerofgt_sound_cpu_state::machine_start();

	m_soundbank->configure_entries(0, 4, memregion("soundbank")->base(), 0x8000);
}

void aerofgt_banked_sound_state::machine_reset()
{
	aerofgt_sound_cpu_state::machine_reset();

	m_soundbank->set_entry(0); // needed by spinlbrk
}

void aerofgt_banked_sound_state::pspikes(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(20'000'000)/2);    // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &aerofgt_banked_sound_state::pspikes_map);
	m_maincpu->set_vblank_int("screen", FUNC(aerofgt_banked_sound_state::irq1_line_hold)); // all irq vectors are the same

	Z80(config, m_audiocpu, XTAL(20'000'000)/4); // verified on pcb
	m_audiocpu->set_addrmap(AS_PROGRAM, &aerofgt_banked_sound_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &aerofgt_banked_sound_state::spinlbrk_sound_portmap);
	// IRQs are triggered by the YM2610

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(61.31);  // verified on pcb
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8+4, 44*8+4-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(aerofgt_banked_sound_state::screen_update_pspikes));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pspikes);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 2048);

	VSYSTEM_SPR2(config, m_spr_old[0], 0);
	m_spr_old[0]->set_tile_indirect_cb(FUNC(aerofgt_banked_sound_state::aerofgt_old_tile_callback));
	m_spr_old[0]->set_gfx_region(1);
	m_spr_old[0]->set_gfxdecode_tag(m_gfxdecode);

	VSYSTEM_GGA(config, "gga", XTAL(14'318'181) / 2); // divider not verified

	MCFG_VIDEO_START_OVERRIDE(aerofgt_banked_sound_state,pspikes)

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(FUNC(aerofgt_banked_sound_state::soundlatch_pending_w));
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 8000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}

void spikes91_state::spikes91(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20000000/2);   // 10 MHz (?)
	m_maincpu->set_addrmap(AS_PROGRAM, &spikes91_state::spikes91_map);
	m_maincpu->set_vblank_int("screen", FUNC(spikes91_state::irq1_line_hold)); // all irq vectors are the same

	Z80(config, m_audiocpu, 24000000/8); // ?
	m_audiocpu->set_addrmap(AS_PROGRAM, &spikes91_state::spikes91_sound_map);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 320-1, 0*8+4, 224+4-1);
	screen.set_screen_update(FUNC(spikes91_state::screen_update_spikes91));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_spikes91);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 2048);

	//VSYSTEM_GGA(config, "gga", 0);

	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 24000000/8));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.50);

	// TODO: OKI M5205
}

void aerofgt_base_state::pspikesb(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20000000/2);   // 10 MHz (?)
	m_maincpu->set_addrmap(AS_PROGRAM, &aerofgt_base_state::pspikesb_map);
	m_maincpu->set_vblank_int("screen", FUNC(aerofgt_base_state::irq1_line_hold)); // all irq vectors are the same

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8+4, 44*8+4-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(aerofgt_base_state::screen_update_pspikesb));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pspikesb);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 2048);

	//VSYSTEM_GGA(config, "gga", 0);

	MCFG_VIDEO_START_OVERRIDE(aerofgt_base_state,pspikes)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1056000, okim6295_device::PIN7_LOW); // clock frequency & pin 7 not verified, pin high causes sound pitch to be too high
	m_oki->set_addrmap(0, &aerofgt_base_state::oki_map);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

/*
    Kick Ball

    cloned bootleg-style Korean hardware, no original VSYSTEM parts
    tile banking and sound system are different like many of the bootlegs
*/

void aerofgt_sound_cpu_state::kickball(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(10'000'000)); // 10Mhz XTAL near 10Mhz rated CPU
	m_maincpu->set_addrmap(AS_PROGRAM, &aerofgt_sound_cpu_state::kickball_map);
	m_maincpu->set_vblank_int("screen", FUNC(aerofgt_sound_cpu_state::irq1_line_hold)); // only IRQ1 is valid

	Z80(config, m_audiocpu, XTAL(4'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &aerofgt_sound_cpu_state::kickball_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &aerofgt_sound_cpu_state::kickball_sound_portmap);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8+4, 44*8+4-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(aerofgt_sound_cpu_state::screen_update_pspikes));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_kickball);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 2048);

	VSYSTEM_SPR2(config, m_spr_old[0], 0);
	m_spr_old[0]->set_tile_indirect_cb(FUNC(aerofgt_sound_cpu_state::aerofgt_old_tile_callback));
	m_spr_old[0]->set_gfx_region(1);
	m_spr_old[0]->set_gfxdecode_tag(m_gfxdecode);

	//VSYSTEM_GGA(config, "gga", 0); // still accessed as if it exists, in clone hardware?

	MCFG_VIDEO_START_OVERRIDE(aerofgt_sound_cpu_state,pspikes)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(FUNC(aerofgt_sound_cpu_state::soundlatch_pending_w));
	m_soundlatch->set_separate_acknowledge(true);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(4'000'000))); // K-666 (YM3812)
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, m_oki, XTAL(4'000'000)/4, okim6295_device::PIN7_LOW); // AD-65 (M6295) clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void aerofgt_base_state::pspikesc(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20000000/2);   // 10 MHz (?)
	m_maincpu->set_addrmap(AS_PROGRAM, &aerofgt_base_state::pspikesc_map);
	m_maincpu->set_vblank_int("screen", FUNC(aerofgt_base_state::irq1_line_hold)); // all irq vectors are the same

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8+4, 44*8+4-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(aerofgt_base_state::screen_update_pspikes));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pspikes);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 2048);

	//VSYSTEM_GGA(config, "gga", 0);

	VSYSTEM_SPR2(config, m_spr_old[0], 0);
	m_spr_old[0]->set_tile_indirect_cb(FUNC(aerofgt_base_state::aerofgt_old_tile_callback));
	m_spr_old[0]->set_gfx_region(1);
	m_spr_old[0]->set_gfxdecode_tag(m_gfxdecode);

	MCFG_VIDEO_START_OVERRIDE(aerofgt_base_state,pspikes)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1056000, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->set_addrmap(0, &aerofgt_base_state::oki_map);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void aerofgt_banked_sound_state::karatblz(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20000000/2);   // 10 MHz (?)
	m_maincpu->set_addrmap(AS_PROGRAM, &aerofgt_banked_sound_state::karatblz_map);
	m_maincpu->set_vblank_int("screen", FUNC(aerofgt_banked_sound_state::irq1_line_hold));

	Z80(config, m_audiocpu, 8000000/2); // 4 MHz ???
	m_audiocpu->set_addrmap(AS_PROGRAM, &aerofgt_banked_sound_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &aerofgt_banked_sound_state::spinlbrk_sound_portmap); // IRQs are triggered by the YM2610

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(1*8, 45*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(aerofgt_banked_sound_state::screen_update_karatblz));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_turbofrc);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 1024);

	VSYSTEM_GGA(config, "gga", XTAL(14'318'181) / 2); // divider not verified

	VSYSTEM_SPR2(config, m_spr_old[0], 0);
	m_spr_old[0]->set_tile_indirect_cb(FUNC(aerofgt_banked_sound_state::aerofgt_old_tile_callback));
	m_spr_old[0]->set_gfx_region(2);
	m_spr_old[0]->set_gfxdecode_tag(m_gfxdecode);

	VSYSTEM_SPR2(config, m_spr_old[1], 0);
	m_spr_old[1]->set_tile_indirect_cb(FUNC(aerofgt_banked_sound_state::aerofgt_ol2_tile_callback));
	m_spr_old[1]->set_gfx_region(3);
	m_spr_old[1]->set_gfxdecode_tag(m_gfxdecode);

	MCFG_VIDEO_START_OVERRIDE(aerofgt_banked_sound_state,karatblz)

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(FUNC(aerofgt_banked_sound_state::soundlatch_pending_w));
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(8'000'000))); // verified on pcb
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}

void karatblzbl_state::karatblzbl(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20000000/2);   // 10 MHz (?)
	m_maincpu->set_addrmap(AS_PROGRAM, &karatblzbl_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(karatblzbl_state::irq1_line_hold));

	Z80(config, m_audiocpu, 8000000/2); // 4 MHz ???
	m_audiocpu->set_addrmap(AS_PROGRAM, &karatblzbl_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &karatblzbl_state::sound_portmap);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(1*8, 45*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(karatblzbl_state::screen_update_karatblz));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_turbofrc);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 1024);

	VSYSTEM_SPR2(config, m_spr_old[0], 0);
	m_spr_old[0]->set_tile_indirect_cb(FUNC(karatblzbl_state::aerofgt_old_tile_callback));
	m_spr_old[0]->set_gfx_region(2);
	m_spr_old[0]->set_gfxdecode_tag(m_gfxdecode);

	VSYSTEM_SPR2(config, m_spr_old[1], 0);
	m_spr_old[1]->set_tile_indirect_cb(FUNC(karatblzbl_state::aerofgt_ol2_tile_callback));
	m_spr_old[1]->set_gfx_region(3);
	m_spr_old[1]->set_gfxdecode_tag(m_gfxdecode);

	//VSYSTEM_GGA(config, "gga", 0);

	MCFG_VIDEO_START_OVERRIDE(karatblzbl_state,karatblz)

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, "soundlatch2");

	// sound hardware

	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(8'000'000)/2));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	UPD7759(config, m_upd7759);
	m_upd7759->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void aerofgt_banked_sound_state::spinlbrk(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(20'000'000)/2); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &aerofgt_banked_sound_state::spinlbrk_map);
	m_maincpu->set_vblank_int("screen", FUNC(aerofgt_banked_sound_state::irq1_line_hold)); // there are vectors for 3 and 4 too, analog related?

	Z80(config, m_audiocpu, XTAL(20'000'000)/4);   // 5mhz verified on pcb
	m_audiocpu->set_addrmap(AS_PROGRAM, &aerofgt_banked_sound_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &aerofgt_banked_sound_state::spinlbrk_sound_portmap); // IRQs are triggered by the YM2610

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(1*8, 45*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(aerofgt_banked_sound_state::screen_update_spinlbrk));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_turbofrc);
	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xRGB_555, 1024); // doesn't fully initialize palette at start-up ...

	VSYSTEM_GGA(config, "gga", XTAL(14'318'181) / 2); // divider not verified

	VSYSTEM_SPR2(config, m_spr_old[0], 0);
	m_spr_old[0]->set_pritype(1);
	m_spr_old[0]->set_gfx_region(2);
	m_spr_old[0]->set_gfxdecode_tag(m_gfxdecode);

	VSYSTEM_SPR2(config, m_spr_old[1], 0);
	m_spr_old[1]->set_tile_indirect_cb(FUNC(aerofgt_banked_sound_state::spinbrk_tile_callback)); // rom lookup
	m_spr_old[1]->set_pritype(1);
	m_spr_old[1]->set_gfx_region(3);
	m_spr_old[1]->set_gfxdecode_tag(m_gfxdecode);

	MCFG_VIDEO_START_OVERRIDE(aerofgt_banked_sound_state,spinlbrk)

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(FUNC(aerofgt_banked_sound_state::soundlatch_pending_w));
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(8'000'000)));  // verified on pcb
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}

void aerofgt_banked_sound_state::turbofrc(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(20'000'000)/2); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &aerofgt_banked_sound_state::turbofrc_map);
	m_maincpu->set_vblank_int("screen", FUNC(aerofgt_banked_sound_state::irq1_line_hold)); // all irq vectors are the same

	Z80(config, m_audiocpu, XTAL(5'000'000));  // verified on pcb
	m_audiocpu->set_addrmap(AS_PROGRAM, &aerofgt_banked_sound_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &aerofgt_banked_sound_state::spinlbrk_sound_portmap); // IRQs are triggered by the YM2610

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(61.31);   // verified on pcb
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 44*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(aerofgt_banked_sound_state::screen_update_turbofrc));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_turbofrc);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 1024);

	VSYSTEM_GGA(config, "gga", XTAL(14'318'181) / 2); // divider not verified

	VSYSTEM_SPR2(config, m_spr_old[0], 0);
	m_spr_old[0]->set_tile_indirect_cb(FUNC(aerofgt_banked_sound_state::aerofgt_old_tile_callback));
	m_spr_old[0]->set_gfx_region(2);
	m_spr_old[0]->set_gfxdecode_tag(m_gfxdecode);

	VSYSTEM_SPR2(config, m_spr_old[1], 0);
	m_spr_old[1]->set_tile_indirect_cb(FUNC(aerofgt_banked_sound_state::aerofgt_ol2_tile_callback));
	m_spr_old[1]->set_gfx_region(3);
	m_spr_old[1]->set_gfxdecode_tag(m_gfxdecode);

	MCFG_VIDEO_START_OVERRIDE(aerofgt_banked_sound_state,turbofrc)

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(FUNC(aerofgt_banked_sound_state::soundlatch_pending_w));
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(8'000'000)));  // verified on pcb
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}

void aerofgt_banked_sound_state::aerofgtb(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20000000/2);   // 10 MHz (?)
	m_maincpu->set_addrmap(AS_PROGRAM, &aerofgt_banked_sound_state::aerofgtb_map);
	m_maincpu->set_vblank_int("screen", FUNC(aerofgt_banked_sound_state::irq1_line_hold)); // all irq vectors are the same

	Z80(config, m_audiocpu, 8000000/2); // 4 MHz ???
	m_audiocpu->set_addrmap(AS_PROGRAM, &aerofgt_banked_sound_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &aerofgt_banked_sound_state::aerofgt_sound_portmap); // IRQs are triggered by the YM2610

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(500)); // wrong but improves sprite-background synchronization
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8+12, 40*8-1+12, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(aerofgt_banked_sound_state::screen_update_turbofrc));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_turbofrc);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 1024);

	VSYSTEM_GGA(config, "gga", XTAL(14'318'181) / 2); // divider not verified

	VSYSTEM_SPR2(config, m_spr_old[0], 0);
	m_spr_old[0]->set_tile_indirect_cb(FUNC(aerofgt_banked_sound_state::aerofgt_old_tile_callback));
	m_spr_old[0]->set_gfx_region(2);
	m_spr_old[0]->set_gfxdecode_tag(m_gfxdecode);
	m_spr_old[0]->set_offsets(3, -1);

	VSYSTEM_SPR2(config, m_spr_old[1], 0);
	m_spr_old[1]->set_tile_indirect_cb(FUNC(aerofgt_banked_sound_state::aerofgt_ol2_tile_callback));
	m_spr_old[1]->set_gfx_region(3);
	m_spr_old[1]->set_gfxdecode_tag(m_gfxdecode);
	m_spr_old[1]->set_offsets(3, -1);

	MCFG_VIDEO_START_OVERRIDE(aerofgt_banked_sound_state,aerofgtb)

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(FUNC(aerofgt_banked_sound_state::soundlatch_pending_w));
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 8000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}

void aerofgt_state::aerofgt(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(20'000'000)/2); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &aerofgt_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(aerofgt_state::irq1_line_hold)); // all irq vectors are the same

	Z80(config, m_audiocpu, XTAL(20'000'000)/4); // 5 MHz verified on pcb
	m_audiocpu->set_addrmap(AS_PROGRAM, &aerofgt_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &aerofgt_state::aerofgt_sound_portmap); // IRQs are triggered by the YM2610

	vs9209_device &io(VS9209(config, "io", 0));
	io.porta_input_cb().set_ioport("P1");
	io.portb_input_cb().set_ioport("P2");
	io.portc_input_cb().set_ioport("SYSTEM");
	io.portd_input_cb().set_ioport("DSW1");
	io.porte_input_cb().set_ioport("DSW2");
	io.portg_input_cb().set(m_soundlatch, FUNC(generic_latch_8_device::pending_r)).lshift(0);
	io.portg_output_cb().set("watchdog", FUNC(mb3773_device::write_line_ck)).bit(7);
	io.porth_input_cb().set_ioport("JP1");

	MB3773(config, "watchdog", 0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(61.31);  // verified on pcb
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(400)); // wrong but improves sprite-background synchronization
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(aerofgt_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_aerofgt);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 1024);

	VSYSTEM_SPR(config, m_spr, 0);
	m_spr->set_tile_indirect_cb(FUNC(aerofgt_state::tile_callback));
	m_spr->set_gfx_region(2);
	m_spr->set_gfxdecode_tag(m_gfxdecode);

	MCFG_VIDEO_START_OVERRIDE(aerofgt_state,turbofrc)

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(FUNC(aerofgt_state::soundlatch_pending_w));
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(8'000'000)));  // verified on pcb
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}

void aerofgt_sound_cpu_state::aerfboot(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20000000/2);   // 10 MHz (?)
	m_maincpu->set_addrmap(AS_PROGRAM, &aerofgt_sound_cpu_state::aerfboot_map);
	m_maincpu->set_vblank_int("screen", FUNC(aerofgt_sound_cpu_state::irq1_line_hold));

	Z80(config, m_audiocpu, 8000000/2); // 4 MHz ???
	m_audiocpu->set_addrmap(AS_PROGRAM, &aerofgt_sound_cpu_state::aerfboot_sound_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(500)); // wrong but improves sprite-background synchronization
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8+12, 40*8-1+12, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(aerofgt_sound_cpu_state::screen_update_aerfboot));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_aerfboot);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 1024);

	//VSYSTEM_GGA(config, "gga", 0);

	MCFG_VIDEO_START_OVERRIDE(aerofgt_sound_cpu_state,turbofrc)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	OKIM6295(config, m_oki, 1056000, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->set_addrmap(0, &aerofgt_sound_cpu_state::oki_map);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void aerofgt_base_state::aerfboo2(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20000000/2);   // 10 MHz (?)
	m_maincpu->set_addrmap(AS_PROGRAM, &aerofgt_base_state::aerfboo2_map);
	m_maincpu->set_vblank_int("screen", FUNC(aerofgt_base_state::irq2_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(500)); // wrong but improves sprite-background synchronization
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8+12, 40*8-1+12, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(aerofgt_base_state::screen_update_aerfboo2));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_aerfboo2);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 1024);

	//VSYSTEM_GGA(config, "gga", 0);

	MCFG_VIDEO_START_OVERRIDE(aerofgt_base_state,turbofrc)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1056000, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void wbbc97_state::wbbc97(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20000000/2);   // 10 MHz (?)
	m_maincpu->set_addrmap(AS_PROGRAM, &wbbc97_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(wbbc97_state::irq1_line_hold)); // all irq vectors are the same

	Z80(config, m_audiocpu, 8000000/2); // 4 MHz ???
	m_audiocpu->set_addrmap(AS_PROGRAM, &wbbc97_state::sound_map); // IRQs are triggered by the YM3812

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8+14, 44*8-1+4, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(wbbc97_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pspikes);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 2048);

	//VSYSTEM_GGA(config, "gga", 0);

	VSYSTEM_SPR2(config, m_spr_old[0], 0);
	m_spr_old[0]->set_tile_indirect_cb(FUNC(wbbc97_state::aerofgt_old_tile_callback));
	m_spr_old[0]->set_gfx_region(1);
	m_spr_old[0]->set_gfxdecode_tag(m_gfxdecode);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 3579545));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, m_oki, 1056000, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.50);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pspikes )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "pspikes2.bin", 0x00000, 0x40000, CRC(ec0c070e) SHA1(4ddcc184e835a2f9d15f01aaa03734fd75fe797e) )

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "19",           0x00000, 0x20000, CRC(7e8ed6e5) SHA1(eeb1a1e1989fad8fc1e741928422efaec0598868) )

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0, 0x00000, 0x08000 )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "g7h",          0x000000, 0x80000, CRC(74c23c3d) SHA1(c0ac57d1f05c42556f97154ce1a08f465948546b) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD32_WORD( "g7j",          0x000000, 0x80000, CRC(0b9e4739) SHA1(64b440a5026735aafe1a7cc2806fe0d78f4a6fba) )
	ROM_LOAD32_WORD( "g7l",          0x000002, 0x80000, CRC(943139ff) SHA1(59065f9c3b3a47159c5968df199bdcb1b4f51f29) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "a47",          0x00000, 0x40000, CRC(c6779dfa) SHA1(ea7adefdb0da02755428aac9a6f86c908fc11253) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "o5b",          0x000000, 0x100000, CRC(07d6cbac) SHA1(d3d5778dbaca7b6cdceae959d0847d56df7b5cc1) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "peel18cv8.bin", 0x0000, 0x0155, CRC(af5a83c9) SHA1(e8fd64ff71d1c2dff5a0d307ca3543352e903bbe) )
ROM_END

ROM_START( pspikesk )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "20",           0x00000, 0x40000, CRC(75cdcee2) SHA1(272a08c46c1d0989f9fbb156e28e6a7ffa9c0a53) )

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "19",           0x00000, 0x20000, CRC(7e8ed6e5) SHA1(eeb1a1e1989fad8fc1e741928422efaec0598868) )

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0, 0x00000, 0x08000 )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "g7h",          0x000000, 0x80000, CRC(74c23c3d) SHA1(c0ac57d1f05c42556f97154ce1a08f465948546b) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD32_WORD( "g7j",          0x000000, 0x80000, CRC(0b9e4739) SHA1(64b440a5026735aafe1a7cc2806fe0d78f4a6fba) )
	ROM_LOAD32_WORD( "g7l",          0x000002, 0x80000, CRC(943139ff) SHA1(59065f9c3b3a47159c5968df199bdcb1b4f51f29) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "a47",          0x00000, 0x40000, CRC(c6779dfa) SHA1(ea7adefdb0da02755428aac9a6f86c908fc11253) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "o5b",          0x000000, 0x100000, CRC(07d6cbac) SHA1(d3d5778dbaca7b6cdceae959d0847d56df7b5cc1) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "peel18cv8-1101a-u15.53", 0x0000, 0x0155, CRC(c05e3bea) SHA1(b42e16f1f41ad9796ff3044c850d5331e7a0f91a) )
	ROM_LOAD( "peel18cv8-1103-u112.76", 0x0200, 0x0155, CRC(786da44c) SHA1(02fd63083631abeced42714fb58a11b7d463285b) )
ROM_END

ROM_START( pspikesu )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "svolly91.73", 0x00000, 0x40000, CRC(bfbffcdb) SHA1(2bba99cb6d0cb2fbb3cd1242551dd7e2c6ebef50) )

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "19",           0x00000, 0x20000, CRC(7e8ed6e5) SHA1(eeb1a1e1989fad8fc1e741928422efaec0598868) )

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0, 0x00000, 0x08000 )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "g7h",          0x000000, 0x80000, CRC(74c23c3d) SHA1(c0ac57d1f05c42556f97154ce1a08f465948546b) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD32_WORD( "g7j",          0x000000, 0x80000, CRC(0b9e4739) SHA1(64b440a5026735aafe1a7cc2806fe0d78f4a6fba) )
	ROM_LOAD32_WORD( "g7l",          0x000002, 0x80000, CRC(943139ff) SHA1(59065f9c3b3a47159c5968df199bdcb1b4f51f29) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "a47",          0x00000, 0x40000, CRC(c6779dfa) SHA1(ea7adefdb0da02755428aac9a6f86c908fc11253) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "o5b",          0x000000, 0x100000, CRC(07d6cbac) SHA1(d3d5778dbaca7b6cdceae959d0847d56df7b5cc1) )
ROM_END

ROM_START( svolly91 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "u11.jpn",      0x00000, 0x40000, CRC(ea2e4c82) SHA1(f9cf9122499d9b1e54221fb8b6ef9c12004ca85e) )

	ROM_REGION( 0x20000, "soundbank", 0 )    // 64k for the audio CPU + banks
	ROM_LOAD( "19",           0x00000, 0x20000, CRC(7e8ed6e5) SHA1(eeb1a1e1989fad8fc1e741928422efaec0598868) )

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0, 0x00000, 0x08000 )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "g7h",          0x000000, 0x80000, CRC(74c23c3d) SHA1(c0ac57d1f05c42556f97154ce1a08f465948546b) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD32_WORD( "g7j",          0x000000, 0x80000, CRC(0b9e4739) SHA1(64b440a5026735aafe1a7cc2806fe0d78f4a6fba) )
	ROM_LOAD32_WORD( "g7l",          0x000002, 0x80000, CRC(943139ff) SHA1(59065f9c3b3a47159c5968df199bdcb1b4f51f29) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "a47",          0x00000, 0x40000, CRC(c6779dfa) SHA1(ea7adefdb0da02755428aac9a6f86c908fc11253) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "o5b",          0x000000, 0x100000, CRC(07d6cbac) SHA1(d3d5778dbaca7b6cdceae959d0847d56df7b5cc1) )
ROM_END

ROM_START( pspikesb )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.ic63",    0x00000, 0x20000, CRC(d25e184c) SHA1(89ad275b03d909a7d16d2927df3ddf12301e4c60) )
	ROM_LOAD16_BYTE( "3.ic62",    0x00001, 0x20000, CRC(5add1a34) SHA1(e166d5c76f2f087254f2af442f49251a9885f5bc) )

	ROM_REGION( 0x080000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "4.ic122",   0x00000, 0x20000, CRC(ea1c05a7) SHA1(adfdfeac80df287ffa6f469dc38ea94698817cf4) )
	ROM_LOAD( "5.ic120",   0x20000, 0x20000, CRC(bfdc60f4) SHA1(2b1893fac2651ac82f5a05b8f891b20c928ced7e) )
	ROM_LOAD( "6.ic118",   0x40000, 0x20000, CRC(96a5c235) SHA1(dad4ef9069d3130f719a402737909bb48225b73c) )
	ROM_LOAD( "7.ic116",   0x60000, 0x20000, CRC(a7e00b36) SHA1(2b5e85ec02e8893d7d730aad4d690883b1d236cc) )

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "8.ic121",   0x00000, 0x40000, CRC(fc096cfc) SHA1(75af810c97361b6f08767949b90c394a7a03f60b) )
	ROM_LOAD( "9.ic119",   0x40000, 0x40000, CRC(a45ec985) SHA1(16357f5df7841e11889ac6fced1e2a9288585a29) )
	ROM_LOAD( "10.ic117",  0x80000, 0x40000, CRC(3976b372) SHA1(72feec5a6fe7995f39d4b431dbbf25435359b04d) )
	ROM_LOAD( "11.ic115",  0xc0000, 0x40000, CRC(f9249937) SHA1(5993e5ab7295ca2fa5c8f4c05ce23731741f4e97) )

	// $00000-$20000 stays the same in all sound banks,
	// the second half of the bank is what gets switched
	ROM_REGION( 0x080000, "oki", 0 ) // Samples
	ROM_LOAD( "1.ic21",    0x000000, 0x80000, CRC(1b78ed0b) SHA1(886bfd78709c295839dd51c7f5a13f5c452c0ab3) )
ROM_END

ROM_START( pspikesba )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.ic63",    0x00000, 0x20000, CRC(dd87d28a) SHA1(09ab75bcd62db1a49af123648812852780ac9d60) ) // sldh
	ROM_LOAD16_BYTE( "3.ic62",    0x00001, 0x20000, CRC(ec505317) SHA1(1e2b9e52654b08169827dbd877de2e724140e50c) ) // sldh

	ROM_REGION( 0x080000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "4.ic122",   0x00000, 0x20000, CRC(ea1c05a7) SHA1(adfdfeac80df287ffa6f469dc38ea94698817cf4) )
	ROM_LOAD( "5.ic120",   0x20000, 0x20000, CRC(bfdc60f4) SHA1(2b1893fac2651ac82f5a05b8f891b20c928ced7e) )
	ROM_LOAD( "6.ic118",   0x40000, 0x20000, CRC(96a5c235) SHA1(dad4ef9069d3130f719a402737909bb48225b73c) )
	ROM_LOAD( "7.ic116",   0x60000, 0x20000, CRC(a7e00b36) SHA1(2b5e85ec02e8893d7d730aad4d690883b1d236cc) )

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "8.ic121",   0x00000, 0x40000, CRC(fc096cfc) SHA1(75af810c97361b6f08767949b90c394a7a03f60b) )
	ROM_LOAD( "9.ic119",   0x40000, 0x40000, CRC(a45ec985) SHA1(16357f5df7841e11889ac6fced1e2a9288585a29) )
	ROM_LOAD( "10.ic117",  0x80000, 0x40000, CRC(3976b372) SHA1(72feec5a6fe7995f39d4b431dbbf25435359b04d) )
	ROM_LOAD( "11.ic115",  0xc0000, 0x40000, CRC(f9249937) SHA1(5993e5ab7295ca2fa5c8f4c05ce23731741f4e97) )

	// $00000-$20000 stays the same in all sound banks,
	// the second half of the bank is what gets switched
	ROM_REGION( 0x080000, "oki", 0 ) // Samples
	ROM_LOAD( "1.ic21",    0x000000, 0x80000, CRC(1b78ed0b) SHA1(886bfd78709c295839dd51c7f5a13f5c452c0ab3) )
ROM_END


/*

1991 Spikes (Italian bootleg)

Anno    1991
Produttore
N.revisione PX012-30

CPU

1x MC68000P10 (main)(ic1)
1x Z8400BB1 (sound)(ic139)
1x YM2151 (sound)(ic150)
1x OKI M5205 (sound)(ic145)
2x LM324N (sound)(ic152, ic153)
1x TDA2003 (sound)(ic154)
1x oscillator 20.000 (xtal1)
1x oscillator 24.000 (xtal2)
1x blu crystal POE400B (xtal3)(sound)

ROMs
2x AM27C512 (1,2)(sound)
4x M27C1001 (3,4,5,6)
2x D27C010 (7,8) (main prg)
2x D27C512 (9,10) (gfx)
4x AM27C020 (11,12,13,14) (gfx)
1x EP910PC (ic7)
2x GAL16V8 (ic147, ic94)(not dumped)

Note
1x 28x2 JAMMA edge connector
1x trimmer (volume)
2x 8 switches dip
--------------------------------

This is a clone of "Power Spikes" with Italian language.
It was rather famous in Italy

--------------------------------

This bootleg is very ugly, for example it has 'bad' looking tiles
instead of the video system background on the intro screens.
This appears to be correct as the same behavior can be seen on the
real PCB and in MAME.

Sprite, and sound hardware are also modified when compared to the
original game

*/

ROM_START( spikes91 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "7.ic2",    0x00001, 0x20000, CRC(41e38d7e) SHA1(d0c226a8b61a2311c781ed5747d78b8dbddbc7ef) )
	ROM_LOAD16_BYTE( "8.ic3",    0x00000, 0x20000, CRC(9c488daa) SHA1(8336fec855786c6cc6a836d86b74e130d60013b7) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "3.ic76",   0x00000, 0x20000, CRC(ab451eee) SHA1(439c5f46b4d8e66610417369bd0b2bf5568936cb) )
	ROM_LOAD( "4.ic75",   0x20000, 0x20000, CRC(fe857bbd) SHA1(669151cf28f87cc494883dc537881d86887d08b9) )
	ROM_LOAD( "5.ic74",   0x40000, 0x20000, CRC(d7fcd97c) SHA1(eb7c8ac111f5916350aae0ee3edc019207fef654) )
	ROM_LOAD( "6.ic73",   0x60000, 0x20000, CRC(e6b9107f) SHA1(aaab2f2dfb85ee764091253c9a4ab89bc51d7518) )

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "11.ic118",   0x00000, 0x40000, CRC(6e65b4b2) SHA1(5296e8095ec60f79a5cd3f9db829c7d491670282) )
	ROM_LOAD( "12.ic119",   0x40000, 0x40000, CRC(60e0d3e0) SHA1(93efc58b03610e7f18ff076ac985428a446454f9) )
	ROM_LOAD( "13.ic120",   0x80000, 0x40000, CRC(89213a8c) SHA1(8524d5c14669d9b03f1fe050c4318d4111bc8ef7) )
	ROM_LOAD( "14.ic121",   0xc0000, 0x40000, CRC(468cbf5b) SHA1(60fbc2771e40f8de51a51891b8ddcc14e2b1e52c) )

	ROM_REGION16_BE( 0x020000, "sprlookuprom", 0 ) // lookup tables for the sprites
	ROM_LOAD16_BYTE( "10.ic104",   0x00000, 0x10000, CRC(769ade77) SHA1(9cb581d02592c69f37d4b5a902d3515f40915ec4) )
	ROM_LOAD16_BYTE( "9.ic103",    0x00001, 0x10000, CRC(201cb748) SHA1(f78d384e4e9c5996a278f76fb4d5f28812a27de5) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 Sound CPU + M5205 Samples
	ROM_LOAD( "1.ic140",   0x00000, 0x10000, CRC(e3065b1d) SHA1(c4a3a95ba7f43cdf1b0c574f41de06d007ad2bd8) ) // matches svolleybl in rpunch.cpp
	ROM_LOAD( "2.ic141",   0x10000, 0x10000, CRC(5dd8bf22) SHA1(d1a12894fe8ca47e47b4a1e911cabf20dd41eda4) ) // 1ST AND 2ND HALF IDENTICAL, matches svolleybl in rpunch.cpp when halved

	ROM_REGION( 0x1000, "user2", 0 ) // ?
	ROM_LOAD( "ep910pc.ic7",   0x00000, 0x884, CRC(e7a3913a) SHA1(6f18f55ecdc94a416baecd16fe7c6698b1ec9d87) )
ROM_END

ROM_START( spikes91b ) // todo, check how this differs, only 1 of the 68k pair and the sprite lookup roms were different
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "7.ic2",    0x00001, 0x20000, CRC(46433a36) SHA1(a4df1bf710693779a9444f28c99591331765aada) ) // sldh
	ROM_LOAD16_BYTE( "8.ic3",    0x00000, 0x20000, CRC(9c488daa) SHA1(8336fec855786c6cc6a836d86b74e130d60013b7) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "3.ic76",   0x00000, 0x20000, CRC(ab451eee) SHA1(439c5f46b4d8e66610417369bd0b2bf5568936cb) )
	ROM_LOAD( "4.ic75",   0x20000, 0x20000, CRC(fe857bbd) SHA1(669151cf28f87cc494883dc537881d86887d08b9) )
	ROM_LOAD( "5.ic74",   0x40000, 0x20000, CRC(d7fcd97c) SHA1(eb7c8ac111f5916350aae0ee3edc019207fef654) )
	ROM_LOAD( "6.ic73",   0x60000, 0x20000, CRC(e6b9107f) SHA1(aaab2f2dfb85ee764091253c9a4ab89bc51d7518) )

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "11.ic118",   0x00000, 0x40000, CRC(6e65b4b2) SHA1(5296e8095ec60f79a5cd3f9db829c7d491670282) )
	ROM_LOAD( "12.ic119",   0x40000, 0x40000, CRC(60e0d3e0) SHA1(93efc58b03610e7f18ff076ac985428a446454f9) )
	ROM_LOAD( "13.ic120",   0x80000, 0x40000, CRC(89213a8c) SHA1(8524d5c14669d9b03f1fe050c4318d4111bc8ef7) )
	ROM_LOAD( "14.ic121",   0xc0000, 0x40000, CRC(468cbf5b) SHA1(60fbc2771e40f8de51a51891b8ddcc14e2b1e52c) )

	ROM_REGION16_BE( 0x020000, "sprlookuprom", 0 ) // lookup tables for the sprites
	ROM_LOAD16_BYTE( "10.ic104",    0x00000, 0x8000, CRC(b6fe4e57) SHA1(6b62936ff9d0f39fd02c3db488d53bc035c2272d) ) // sldh
	ROM_LOAD16_BYTE( "9.ic103",     0x00001, 0x8000, CRC(5479ed35) SHA1(ca26289318352901841fcdf26d9b43e797ac39b6) ) // sldh

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 Sound CPU + M5205 Samples
	ROM_LOAD( "1.ic140",   0x00000, 0x10000, CRC(e3065b1d) SHA1(c4a3a95ba7f43cdf1b0c574f41de06d007ad2bd8) ) // matches svolleybl in rpunch.cpp
	ROM_LOAD( "2.ic141",   0x10000, 0x10000, CRC(5dd8bf22) SHA1(d1a12894fe8ca47e47b4a1e911cabf20dd41eda4) ) // 1ST AND 2ND HALF IDENTICAL, matches svolleybl in rpunch.cpp when halved

	ROM_REGION( 0x1000, "user2", 0 ) // ?
	ROM_LOAD( "ep910pc.ic7",   0x00000, 0x884, CRC(e7a3913a) SHA1(6f18f55ecdc94a416baecd16fe7c6698b1ec9d87) )
ROM_END


// this is a bootleg / chinese hack of power spikes

ROM_START( pspikesc )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "27c010.1",     0x00000, 0x20000, CRC(06a6ed73) SHA1(05bffe8766131a8729115244ed499ecdd872962a) )
	ROM_LOAD16_BYTE( "27c010.2",     0x00001, 0x20000, CRC(ff31474e) SHA1(f21d44c15aeffd19e8c7fac49d6b9b239bd41c1b) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "vlh30.bin",    0x000000, 0x80000, CRC(74c23c3d) SHA1(c0ac57d1f05c42556f97154ce1a08f465948546b) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD32_WORD( "vlh10-vh118.bin", 0x000000, 0x80000, CRC(0b9e4739) SHA1(64b440a5026735aafe1a7cc2806fe0d78f4a6fba) )
	ROM_LOAD32_WORD( "vlh20-vh102.bin", 0x000002, 0x80000, CRC(943139ff) SHA1(59065f9c3b3a47159c5968df199bdcb1b4f51f29) )

	// $00000-$20000 stays the same in all sound banks,
	// the second half of the bank is what gets switched
	ROM_REGION( 0x080000, "oki", 0 ) // Samples
	ROM_LOAD( "vlh40.bin",    0x00000, 0x80000, CRC(27166dd4) SHA1(f32ef1735d1a1aeda5df0337e46d65282dd798ad) )
ROM_END

ROM_START( spinlbrk )
	ROM_REGION( 0x60000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "ic98",    0x00000, 0x10000, CRC(36c2bf70) SHA1(f627d0e7dad1760bcc95af4a6346050a1a277048) )
	ROM_LOAD16_BYTE( "ic104",   0x00001, 0x10000, CRC(34a7e158) SHA1(5884570c1be38bfedffca3fd38089d0ae3391d4f) )
	ROM_LOAD16_BYTE( "ic93",    0x20000, 0x10000, CRC(726f4683) SHA1(65aff0548333571d47a96d4bf5a7857f12399cc7) )
	ROM_LOAD16_BYTE( "ic94",    0x20001, 0x10000, CRC(c4385e03) SHA1(6683eed812fa8a5430125b14e8647f8e9024bbdd) )

	ROM_REGION( 0x20000, "soundbank", 0 )
	ROM_LOAD( "ic118",        0x00000, 0x10000, CRC(1025f024) SHA1(3e497c74c950d2cd2a0931cf2ae9b0124d11ca6a) )
	ROM_RELOAD(               0x10000, 0x10000 )

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_LOAD( "ic117",        0x00000, 0x08000, CRC(625ada41) SHA1(2dd0674c68ea382431115c155afbf880f5b9deb2) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "ic15",         0x000000, 0x80000, CRC(e318cf3a) SHA1(d634001a0029566ce7b8fa30075970919eb5f44e) )
	ROM_LOAD( "ic9",          0x080000, 0x80000, CRC(e071f674) SHA1(b6d98d7fcc28516d937d8c655d07305515be8a20) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ic17",         0x000000, 0x80000, CRC(a63d5a55) SHA1(a942651a206a2abe4f60f0717e1d5d8e89b993d4) )
	ROM_LOAD( "ic11",         0x080000, 0x80000, CRC(7dcc913d) SHA1(527bae5020581d1ac322ea25c8e0994d54bbc051) )
	ROM_LOAD( "ic16",         0x100000, 0x80000, CRC(0d84af7f) SHA1(07356ee61c84c4c4ccb49c8dfe8c468990580041) ) //FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x100000, "spritegfx", 0 )
	ROM_LOAD32_WORD( "ic12",         0x000000, 0x80000, CRC(d63fac4e) SHA1(bb96d2e41334d136b9208dbe7e88a45e3bbc6542) )
	ROM_LOAD32_WORD( "ic18",         0x000002, 0x80000, CRC(5a60444b) SHA1(62c418aedd1087dac82dcb44830cce00278103dd) )

	ROM_REGION( 0x200000, "gfx4", 0 )
	ROM_LOAD32_WORD( "ic14",         0x000000, 0x80000, CRC(1befd0f3) SHA1(7ab6fb5bf814ef3ae9a306a0d32d1078ee594461) )
	ROM_LOAD32_WORD( "ic20",         0x100000, 0x80000, CRC(c2f84a61) SHA1(1dce538ced54a61c43ed25e1d71b5ac1c8935cc5) )
	ROM_LOAD32_WORD( "ic35",         0x000002, 0x80000, CRC(eba8e1a3) SHA1(976ef30437df9aba6fa6d5cd11728476f34eb05b) )
	ROM_LOAD32_WORD( "ic40",         0x100002, 0x80000, CRC(5ef5aa7e) SHA1(8d4b0f2348c536c6781c8ba25722301673aca289) )

	ROM_REGION16_BE( 0x24000, "sprlookuprom", 0 )   // hardcoded sprite maps
	ROM_LOAD16_BYTE( "ic19",    0x00000, 0x10000, CRC(db24eeaa) SHA1(300dd1ce81dd258b265bc3a64b8542ed152ed2cf) )
	ROM_LOAD16_BYTE( "ic13",    0x00001, 0x10000, CRC(97025bf4) SHA1(0519f0c94f3d417bf8ff0124a3a137035a4013dc) )
	// 20000-23fff empty space, filled in vh_startup

	// no "ymsnd:adpcmb"

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "ic166",        0x000000, 0x80000, CRC(6e0d063a) SHA1(313983e69f9625814de033fef7f6e9564694117a) )
	ROM_LOAD( "ic163",        0x080000, 0x80000, CRC(e6621dfb) SHA1(85ee77c4720b7eb20ecf293c16b3105c8dcb1114) ) //FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "epl16p8bp.ic133", 0x0000, 0x0107, NO_DUMP )  // read protected
	ROM_LOAD( "epl16p8bp.ic127", 0x0200, 0x0107, NO_DUMP )  // read protected
	ROM_LOAD( "epl16p8bp.ic99",  0x0400, 0x0107, NO_DUMP )  // read protected
	ROM_LOAD( "epl16p8bp.ic100", 0x0600, 0x0107, NO_DUMP )  // read protected
	ROM_LOAD( "gal16v8a.ic95",   0x0800, 0x0117, NO_DUMP )  // read protected
	ROM_LOAD( "gal16v8a.ic114",  0x0a00, 0x0117, NO_DUMP )  // read protected
ROM_END

ROM_START( spinlbrku )
	ROM_REGION( 0x60000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "ic98.u5", 0x00000, 0x10000, CRC(3a0f7667) SHA1(55d5fa1a325c17532ed83d231032bdbe9fb84d85) )
	ROM_LOAD16_BYTE( "ic104.u6",0x00001, 0x10000, CRC(a0e0af31) SHA1(21f6c3246bb7be2fd926324fd6d041e319a4e214) )
	ROM_LOAD16_BYTE( "ic93.u4", 0x20000, 0x10000, CRC(0cf73029) SHA1(e1346b759a41f9eec9536dc90671778582e595b4) )
	ROM_LOAD16_BYTE( "ic94.u3", 0x20001, 0x10000, CRC(5cf7c426) SHA1(b201da40c4511d2845004dff72d36adbb8a4fab9) )

	ROM_REGION( 0x20000, "soundbank", 0 )
	ROM_LOAD( "ic118",        0x00000, 0x10000, CRC(1025f024) SHA1(3e497c74c950d2cd2a0931cf2ae9b0124d11ca6a) )
	ROM_RELOAD(               0x10000, 0x10000 )

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_LOAD( "ic117",        0x00000, 0x08000, CRC(625ada41) SHA1(2dd0674c68ea382431115c155afbf880f5b9deb2) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "ic15",         0x000000, 0x80000, CRC(e318cf3a) SHA1(d634001a0029566ce7b8fa30075970919eb5f44e) )
	ROM_LOAD( "ic9",          0x080000, 0x80000, CRC(e071f674) SHA1(b6d98d7fcc28516d937d8c655d07305515be8a20) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ic17",         0x000000, 0x80000, CRC(a63d5a55) SHA1(a942651a206a2abe4f60f0717e1d5d8e89b993d4) )
	ROM_LOAD( "ic11",         0x080000, 0x80000, CRC(7dcc913d) SHA1(527bae5020581d1ac322ea25c8e0994d54bbc051) )
	ROM_LOAD( "ic16",         0x100000, 0x80000, CRC(0d84af7f) SHA1(07356ee61c84c4c4ccb49c8dfe8c468990580041) ) //FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x100000, "spritegfx", 0 )
	ROM_LOAD32_WORD( "ic12",         0x000000, 0x80000, CRC(d63fac4e) SHA1(bb96d2e41334d136b9208dbe7e88a45e3bbc6542) )
	ROM_LOAD32_WORD( "ic18",         0x000002, 0x80000, CRC(5a60444b) SHA1(62c418aedd1087dac82dcb44830cce00278103dd) )

	ROM_REGION( 0x200000, "gfx4", 0 )
	ROM_LOAD32_WORD( "ic14",         0x000000, 0x80000, CRC(1befd0f3) SHA1(7ab6fb5bf814ef3ae9a306a0d32d1078ee594461) )
	ROM_LOAD32_WORD( "ic20",         0x100000, 0x80000, CRC(c2f84a61) SHA1(1dce538ced54a61c43ed25e1d71b5ac1c8935cc5) )
	ROM_LOAD32_WORD( "ic35",         0x000002, 0x80000, CRC(eba8e1a3) SHA1(976ef30437df9aba6fa6d5cd11728476f34eb05b) )
	ROM_LOAD32_WORD( "ic40",         0x100002, 0x80000, CRC(5ef5aa7e) SHA1(8d4b0f2348c536c6781c8ba25722301673aca289) )

	ROM_REGION16_BE( 0x24000, "sprlookuprom", 0 )   // hardcoded sprite maps
	ROM_LOAD16_BYTE( "ic19",    0x00000, 0x10000, CRC(db24eeaa) SHA1(300dd1ce81dd258b265bc3a64b8542ed152ed2cf) )
	ROM_LOAD16_BYTE( "ic13",    0x00001, 0x10000, CRC(97025bf4) SHA1(0519f0c94f3d417bf8ff0124a3a137035a4013dc) )
	// 20000-23fff empty space, filled in vh_startup

	// no "ymsnd:adpcmb"

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "ic166",        0x000000, 0x80000, CRC(6e0d063a) SHA1(313983e69f9625814de033fef7f6e9564694117a) )
	ROM_LOAD( "ic163",        0x080000, 0x80000, CRC(e6621dfb) SHA1(85ee77c4720b7eb20ecf293c16b3105c8dcb1114) ) //FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "epl16p8bp.ic133", 0x0000, 0x0107, NO_DUMP )  // read protected
	ROM_LOAD( "epl16p8bp.ic127", 0x0200, 0x0107, NO_DUMP )  // read protected
	ROM_LOAD( "epl16p8bp.ic99",  0x0400, 0x0107, NO_DUMP )  // read protected
	ROM_LOAD( "epl16p8bp.ic100", 0x0600, 0x0107, NO_DUMP )  // read protected
	ROM_LOAD( "gal16v8a.ic95",   0x0800, 0x0117, NO_DUMP )  // read protected
	ROM_LOAD( "gal16v8a.ic114",  0x0a00, 0x0117, NO_DUMP )  // read protected
ROM_END

ROM_START( spinlbrkj )
	ROM_REGION( 0x60000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "j5",      0x00000, 0x10000, CRC(6a3d690e) SHA1(4ac1985ea0a73b8fc12105ff75121718595dd171) )
	ROM_LOAD16_BYTE( "j6",      0x00001, 0x10000, CRC(869593fa) SHA1(5821b011d42113f247bd100cecf140bbfc1e969c) )
	ROM_LOAD16_BYTE( "j4",      0x20000, 0x10000, CRC(33e33912) SHA1(d6d052cd8dbedfd254bdf5e82ad770e4bf241777) )
	ROM_LOAD16_BYTE( "j3",      0x20001, 0x10000, CRC(16ca61d0) SHA1(5d99a1261251412c3c758af751997fe31026c0d6) )

	ROM_REGION( 0x20000, "soundbank", 0 )
	ROM_LOAD( "ic118",        0x00000, 0x10000, CRC(1025f024) SHA1(3e497c74c950d2cd2a0931cf2ae9b0124d11ca6a) )
	ROM_RELOAD(               0x10000, 0x10000 )

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_LOAD( "ic117",        0x00000, 0x08000, CRC(625ada41) SHA1(2dd0674c68ea382431115c155afbf880f5b9deb2) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "ic15",         0x000000, 0x80000, CRC(e318cf3a) SHA1(d634001a0029566ce7b8fa30075970919eb5f44e) )
	ROM_LOAD( "ic9",          0x080000, 0x80000, CRC(e071f674) SHA1(b6d98d7fcc28516d937d8c655d07305515be8a20) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ic17",         0x000000, 0x80000, CRC(a63d5a55) SHA1(a942651a206a2abe4f60f0717e1d5d8e89b993d4) )
	ROM_LOAD( "ic11",         0x080000, 0x80000, CRC(7dcc913d) SHA1(527bae5020581d1ac322ea25c8e0994d54bbc051) )
	ROM_LOAD( "ic16",         0x100000, 0x80000, CRC(0d84af7f) SHA1(07356ee61c84c4c4ccb49c8dfe8c468990580041) ) //FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x100000, "spritegfx", 0 )
	ROM_LOAD32_WORD( "ic12",         0x000000, 0x80000, CRC(d63fac4e) SHA1(bb96d2e41334d136b9208dbe7e88a45e3bbc6542) )
	ROM_LOAD32_WORD( "ic18",         0x000002, 0x80000, CRC(5a60444b) SHA1(62c418aedd1087dac82dcb44830cce00278103dd) )

	ROM_REGION( 0x200000, "gfx4", 0 )
	ROM_LOAD32_WORD( "ic14",         0x000000, 0x80000, CRC(1befd0f3) SHA1(7ab6fb5bf814ef3ae9a306a0d32d1078ee594461) )
	ROM_LOAD32_WORD( "ic20",         0x100000, 0x80000, CRC(c2f84a61) SHA1(1dce538ced54a61c43ed25e1d71b5ac1c8935cc5) )
	ROM_LOAD32_WORD( "ic35",         0x000002, 0x80000, CRC(eba8e1a3) SHA1(976ef30437df9aba6fa6d5cd11728476f34eb05b) )
	ROM_LOAD32_WORD( "ic40",         0x100002, 0x80000, CRC(5ef5aa7e) SHA1(8d4b0f2348c536c6781c8ba25722301673aca289) )

	ROM_REGION16_BE( 0x24000, "sprlookuprom", 0 )   // hardcoded sprite maps
	ROM_LOAD16_BYTE( "ic19",    0x00000, 0x10000, CRC(db24eeaa) SHA1(300dd1ce81dd258b265bc3a64b8542ed152ed2cf) )
	ROM_LOAD16_BYTE( "ic13",    0x00001, 0x10000, CRC(97025bf4) SHA1(0519f0c94f3d417bf8ff0124a3a137035a4013dc) )
	// 20000-23fff empty space, filled in vh_startup

	// no "ymsnd:adpcmb"

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "ic166",        0x000000, 0x80000, CRC(6e0d063a) SHA1(313983e69f9625814de033fef7f6e9564694117a) )
	ROM_LOAD( "ic163",        0x080000, 0x80000, CRC(e6621dfb) SHA1(85ee77c4720b7eb20ecf293c16b3105c8dcb1114) ) //FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "epl16p8bp.ic133", 0x0000, 0x0107, NO_DUMP )  // read protected
	ROM_LOAD( "epl16p8bp.ic127", 0x0200, 0x0107, NO_DUMP )  // read protected
	ROM_LOAD( "epl16p8bp.ic99",  0x0400, 0x0107, NO_DUMP )  // read protected
	ROM_LOAD( "epl16p8bp.ic100", 0x0600, 0x0107, NO_DUMP )  // read protected
	ROM_LOAD( "gal16v8a.ic95",   0x0800, 0x0117, NO_DUMP )  // read protected
	ROM_LOAD( "gal16v8a.ic114",  0x0a00, 0x0117, NO_DUMP )  // read protected
ROM_END

/*
Dumper's notes:
-board has sockets and jumpers for ROMS not fitted on the final board.
-early version has game crashing bugs and other glitches.
-the mask ROMs match the regular version.
-RAM & ROM check is not shown.
-region warning seems much slower to pass.
-attract mode starts in the jungle, old dumps all start with a building with fences
*/

ROM_START( spinlbrkup ) // the labels are official Video System without numbering and handwritten on top
	ROM_REGION( 0x60000, "maincpu", 0 ) // these differ quite a lot
	ROM_LOAD16_BYTE( "spb0-e.ic98",  0x00000, 0x10000, CRC(421eaff2) SHA1(e0ffd37fa8361a544cd7a66da0b802367adccd7d) )
	ROM_LOAD16_BYTE( "spb0-o.ic104", 0x00001, 0x10000, CRC(9576d508) SHA1(426a630a7a5bdaa0316391582b25274a6d923c79) )
	ROM_LOAD16_BYTE( "sbp1-e.ic93",  0x20000, 0x10000, CRC(d6444d1e) SHA1(2aa80b8cf3c44e4a583b57f4a6cc673d984f3a05) )
	ROM_LOAD16_BYTE( "sbp1-o.ic94",  0x20001, 0x10000, CRC(a3f7bd8e) SHA1(dc393b4911c6acadb97b0380db15ae5c972a3505) )

	ROM_REGION( 0x20000, "soundbank", 0 )
	ROM_LOAD( "11-14.m.bank.ic118",      0x00000, 0x10000, CRC(a1ed270b) SHA1(f513baed1f7efda3d843982e1fa4857f058e8595) ) // relatively minor differences
	ROM_RELOAD(                          0x10000, 0x10000 )

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "11-14-15.00.music.ic117", 0x00000, 0x08000, CRC(6b8c8f09) SHA1(d57699ea82f5e48ed8c9909a040c2fa6f988f8b3) ) // this differs quite a lot

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "ic15",         0x000000, 0x80000, CRC(e318cf3a) SHA1(d634001a0029566ce7b8fa30075970919eb5f44e) )
	ROM_LOAD( "ic9",          0x080000, 0x80000, CRC(e071f674) SHA1(b6d98d7fcc28516d937d8c655d07305515be8a20) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ic17",         0x000000, 0x80000, CRC(a63d5a55) SHA1(a942651a206a2abe4f60f0717e1d5d8e89b993d4) )
	ROM_LOAD( "ic11",         0x080000, 0x80000, CRC(7dcc913d) SHA1(527bae5020581d1ac322ea25c8e0994d54bbc051) )
	ROM_LOAD( "ic16",         0x100000, 0x80000, CRC(0d84af7f) SHA1(07356ee61c84c4c4ccb49c8dfe8c468990580041) ) //FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x100000, "spritegfx", 0 )
	ROM_LOAD32_WORD( "ic12",         0x000000, 0x80000, CRC(d63fac4e) SHA1(bb96d2e41334d136b9208dbe7e88a45e3bbc6542) )
	ROM_LOAD32_WORD( "ic18",         0x000002, 0x80000, CRC(5a60444b) SHA1(62c418aedd1087dac82dcb44830cce00278103dd) )

	ROM_REGION( 0x200000, "gfx4", 0 )
	ROM_LOAD32_WORD( "ic14",         0x000000, 0x80000, CRC(1befd0f3) SHA1(7ab6fb5bf814ef3ae9a306a0d32d1078ee594461) )
	ROM_LOAD32_WORD( "ic20",         0x100000, 0x80000, CRC(c2f84a61) SHA1(1dce538ced54a61c43ed25e1d71b5ac1c8935cc5) )
	ROM_LOAD32_WORD( "ic35",         0x000002, 0x80000, CRC(eba8e1a3) SHA1(976ef30437df9aba6fa6d5cd11728476f34eb05b) )
	ROM_LOAD32_WORD( "ic40",         0x100002, 0x80000, CRC(5ef5aa7e) SHA1(8d4b0f2348c536c6781c8ba25722301673aca289) )

	ROM_REGION16_BE( 0x24000, "sprlookuprom", 0 ) // very minor differences
	ROM_LOAD16_BYTE( "sbm-1-18.ic19",    0x00000, 0x10000, CRC(e155357f) SHA1(05864c5d281df52adce4e959c65260ee3f585988) )
	ROM_LOAD16_BYTE( "sbm-0-18.ic13",    0x00001, 0x10000, CRC(16b79e45) SHA1(9f89381bc3949adf1317836fc35608c0982a4b46) )

	// no "ymsnd:adpcmb"

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "ic166",        0x000000, 0x80000, CRC(6e0d063a) SHA1(313983e69f9625814de033fef7f6e9564694117a) )
	ROM_LOAD( "ic163",        0x080000, 0x80000, CRC(e6621dfb) SHA1(85ee77c4720b7eb20ecf293c16b3105c8dcb1114) ) //FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "epl16p8bp.ic133", 0x0000, 0x0107, NO_DUMP ) // read protected
	ROM_LOAD( "epl16p8bp.ic127", 0x0200, 0x0107, NO_DUMP ) // read protected
	ROM_LOAD( "epl16p8bp.ic99",  0x0400, 0x0107, NO_DUMP ) // read protected
	ROM_LOAD( "epl16p8bp.ic100", 0x0600, 0x0107, NO_DUMP ) // read protected
	ROM_LOAD( "gal16v8a.ic95",   0x0800, 0x0117, NO_DUMP ) // read protected
	ROM_LOAD( "gal16v8a.ic114",  0x0a00, 0x0117, NO_DUMP ) // read protected
ROM_END

/*

Karate Blazers regions known to exist but not dumped or not verified:

Toushin Blazers title:
  4V2 with 1V1   Tecmo license??
   V2 with 1V1   "original" non Tecmo verions??

Karate Blazers title
  3V2 with 1V1  Euro, current parent??

Note: It's unknown what if any difference there is between V1 and 1V1 ROMs

*/
ROM_START( karatblz )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "rom2v3.u14", 0x00000, 0x40000, CRC(01f772e1) SHA1(f87f19a82d75839b5671f23ce14218d7b910eabc) ) // need to verify correct "region" stamped number, should this be "3v2.u14" ??
	ROM_LOAD16_WORD_SWAP( "v1.u15",     0x40000, 0x40000, CRC(d16ee21b) SHA1(d454cdf22b72a537b9d7ae73deb8136a4f09da47) ) // chip with VideoSystem logo V and 1

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "v5.u92", 0x00000, 0x20000, CRC(97d67510) SHA1(1ffd419e3dec7de1099cd5819b0309f7dd0df80e) ) // chip with VideoSystem logo V and 5

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "gha.u55", 0x00000, 0x80000, CRC(3e0cea91) SHA1(bab41715f106d364013b64649441d280bc6893cf) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "gh9.u61", 0x00000, 0x80000, CRC(5d1676bd) SHA1(6227d489c9c6259a0ac2bef62821fbf94efca8c6) )

	ROM_REGION( 0x400000, "spritegfx", 0 )
	ROM_LOAD32_WORD( "u42",    0x000000, 0x100000, CRC(65f0da84) SHA1(0bfbc6f4b87583703246704eb9fa13b1b3e6f90e) )
	ROM_LOAD32_WORD( "v3.u44", 0x200000, 0x020000, CRC(34bdead2) SHA1(99f9a8cac807fcd599db55d2dc624ed92a3862ef) ) // chip with VideoSystem logo V and 3
	ROM_LOAD32_WORD( "u43",    0x000002, 0x100000, CRC(7b349e5d) SHA1(8590a328f403e2c697a8d698c08d4adaf01fff62) )
	ROM_LOAD32_WORD( "v4.u45", 0x200002, 0x020000, CRC(be4d487d) SHA1(6d19c91d0498c43017219f0c10f4845a51ccfa7f) ) // chip with VideoSystem logo V and 4

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD32_WORD( "u59.ghb", 0x000000, 0x80000, CRC(158c9cde) SHA1(a2c1b404d40e6c2627691f5c7a3f63484bd5d2de) )
	ROM_LOAD32_WORD( "ghd.u60", 0x000002, 0x80000, CRC(73180ae3) SHA1(e4eaf6693826d9e72032d0a0e25938a23ab7d792) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "u105.gh8", 0x000000, 0x080000, CRC(7a68cb1b) SHA1(1bdd0000c2d68019b9e5bf8f7ad84a6ae1af8443) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "u104", 0x000000, 0x100000, CRC(5795e884) SHA1(a4178497ad0a1e60ceb87612b218d77b36d2a11b) )
ROM_END

ROM_START( karatblzt ) // Karate Blazers, Tecmo license
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "2v2.u14",  0x00000, 0x40000, CRC(7ae17b7f) SHA1(d7916c3159e22dde56a0de750032da9ee46b3d6c) ) // 2 stamped on chip with VideoSystem logo V and 2
	ROM_LOAD16_WORD_SWAP( "v1.u15",   0x40000, 0x40000, CRC(d16ee21b) SHA1(d454cdf22b72a537b9d7ae73deb8136a4f09da47) ) // chip with VideoSystem logo V and 1

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "v5.u92", 0x00000, 0x20000, CRC(97d67510) SHA1(1ffd419e3dec7de1099cd5819b0309f7dd0df80e) ) // chip with VideoSystem logo V and 5

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "gha.u55", 0x00000, 0x80000, CRC(3e0cea91) SHA1(bab41715f106d364013b64649441d280bc6893cf) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "gh9.u61", 0x00000, 0x80000, CRC(5d1676bd) SHA1(6227d489c9c6259a0ac2bef62821fbf94efca8c6) )

	ROM_REGION( 0x400000, "spritegfx", 0 )
	ROM_LOAD32_WORD( "u42",    0x000000, 0x100000, CRC(65f0da84) SHA1(0bfbc6f4b87583703246704eb9fa13b1b3e6f90e) )
	ROM_LOAD32_WORD( "v3.u44", 0x200000, 0x020000, CRC(34bdead2) SHA1(99f9a8cac807fcd599db55d2dc624ed92a3862ef) ) // chip with VideoSystem logo V and 3
	ROM_LOAD32_WORD( "u43",    0x000002, 0x100000, CRC(7b349e5d) SHA1(8590a328f403e2c697a8d698c08d4adaf01fff62) )
	ROM_LOAD32_WORD( "v4.u45", 0x200002, 0x020000, CRC(be4d487d) SHA1(6d19c91d0498c43017219f0c10f4845a51ccfa7f) ) // chip with VideoSystem logo V and 4

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD32_WORD( "u59.ghb", 0x000000, 0x80000, CRC(158c9cde) SHA1(a2c1b404d40e6c2627691f5c7a3f63484bd5d2de) )
	ROM_LOAD32_WORD( "ghd.u60", 0x000002, 0x80000, CRC(73180ae3) SHA1(e4eaf6693826d9e72032d0a0e25938a23ab7d792) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "u105.gh8", 0x000000, 0x080000, CRC(7a68cb1b) SHA1(1bdd0000c2d68019b9e5bf8f7ad84a6ae1af8443) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "u104", 0x000000, 0x100000, CRC(5795e884) SHA1(a4178497ad0a1e60ceb87612b218d77b36d2a11b) )
ROM_END

ROM_START( karatblza )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "_v2.u14",  0x00000, 0x40000, CRC(7a78976e) SHA1(3b74b80765622b8488bdd0729ec98a2c7584cad5) ) // need to verify correct "region" stamped number
	ROM_LOAD16_WORD_SWAP( "_v1.u15",  0x40000, 0x40000, CRC(47e410fe) SHA1(d26fc93f91ccf00856db2b7dfd0d905d87e99bd8) ) // need to verify correct "region" stamped number, is this one "1v1" ??

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "v5.u92", 0x00000, 0x20000, CRC(97d67510) SHA1(1ffd419e3dec7de1099cd5819b0309f7dd0df80e) ) // chip with VideoSystem logo V and 5

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "gha.u55", 0x00000, 0x80000, CRC(3e0cea91) SHA1(bab41715f106d364013b64649441d280bc6893cf) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "gh9.u61", 0x00000, 0x80000, CRC(5d1676bd) SHA1(6227d489c9c6259a0ac2bef62821fbf94efca8c6) )

	ROM_REGION( 0x400000, "spritegfx", 0 )
	ROM_LOAD32_WORD( "u42",    0x000000, 0x100000, CRC(65f0da84) SHA1(0bfbc6f4b87583703246704eb9fa13b1b3e6f90e) )
	ROM_LOAD32_WORD( "v3.u44", 0x200000, 0x020000, CRC(34bdead2) SHA1(99f9a8cac807fcd599db55d2dc624ed92a3862ef) ) // chip with VideoSystem logo V and 3
	ROM_LOAD32_WORD( "u43",    0x000002, 0x100000, CRC(7b349e5d) SHA1(8590a328f403e2c697a8d698c08d4adaf01fff62) )
	ROM_LOAD32_WORD( "v4.u45", 0x200002, 0x020000, CRC(be4d487d) SHA1(6d19c91d0498c43017219f0c10f4845a51ccfa7f) ) // chip with VideoSystem logo V and 4

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD32_WORD( "u59.ghb", 0x000000, 0x80000, CRC(158c9cde) SHA1(a2c1b404d40e6c2627691f5c7a3f63484bd5d2de) )
	ROM_LOAD32_WORD( "ghd.u60", 0x000002, 0x80000, CRC(73180ae3) SHA1(e4eaf6693826d9e72032d0a0e25938a23ab7d792) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "u105.gh8", 0x000000, 0x080000, CRC(7a68cb1b) SHA1(1bdd0000c2d68019b9e5bf8f7ad84a6ae1af8443) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "u104", 0x000000, 0x100000, CRC(5795e884) SHA1(a4178497ad0a1e60ceb87612b218d77b36d2a11b) )
ROM_END

ROM_START( karatblzu )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "1v2.u14", 0x00000, 0x40000, CRC(202e6220) SHA1(2605511a0574cbc39fdf3d8ae27a0aa9b43345fb) ) // 1 stamped on chip with VideoSystem logo V and 2
	ROM_LOAD16_WORD_SWAP( "v1.u15",  0x40000, 0x40000, CRC(d16ee21b) SHA1(d454cdf22b72a537b9d7ae73deb8136a4f09da47) ) // chip with VideoSystem logo V and 1

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "v5.u92", 0x00000, 0x20000, CRC(97d67510) SHA1(1ffd419e3dec7de1099cd5819b0309f7dd0df80e) ) // chip with VideoSystem logo V and 5

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "gha.u55", 0x00000, 0x80000, CRC(3e0cea91) SHA1(bab41715f106d364013b64649441d280bc6893cf) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "gh9.u61", 0x00000, 0x80000, CRC(5d1676bd) SHA1(6227d489c9c6259a0ac2bef62821fbf94efca8c6) )

	ROM_REGION( 0x400000, "spritegfx", 0 )
	ROM_LOAD32_WORD( "u42",    0x000000, 0x100000, CRC(65f0da84) SHA1(0bfbc6f4b87583703246704eb9fa13b1b3e6f90e) )
	ROM_LOAD32_WORD( "v3.u44", 0x200000, 0x020000, CRC(34bdead2) SHA1(99f9a8cac807fcd599db55d2dc624ed92a3862ef) ) // chip with VideoSystem logo V and 3
	ROM_LOAD32_WORD( "u43",    0x000002, 0x100000, CRC(7b349e5d) SHA1(8590a328f403e2c697a8d698c08d4adaf01fff62) )
	ROM_LOAD32_WORD( "v4.u45", 0x200002, 0x020000, CRC(be4d487d) SHA1(6d19c91d0498c43017219f0c10f4845a51ccfa7f) ) // chip with VideoSystem logo V and 4

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD32_WORD( "u59.ghb", 0x000000, 0x80000, CRC(158c9cde) SHA1(a2c1b404d40e6c2627691f5c7a3f63484bd5d2de) )
	ROM_LOAD32_WORD( "ghd.u60", 0x000002, 0x80000, CRC(73180ae3) SHA1(e4eaf6693826d9e72032d0a0e25938a23ab7d792) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "u105.gh8", 0x000000, 0x080000, CRC(7a68cb1b) SHA1(1bdd0000c2d68019b9e5bf8f7ad84a6ae1af8443) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "u104", 0x000000, 0x100000, CRC(5795e884) SHA1(a4178497ad0a1e60ceb87612b218d77b36d2a11b) )
ROM_END

ROM_START( karatblzj ) // Toushin Blazers, Tecmo license
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "2tecmo.u14",  0x00000, 0x40000, CRC(57e52654) SHA1(15939d8f7c693b9248f3dd2b2ad5fbae2c19621f) ) // need to verify correct "region" stamped number, is this one "4v2" ??
	ROM_LOAD16_WORD_SWAP( "v1.u15",      0x40000, 0x40000, CRC(d16ee21b) SHA1(d454cdf22b72a537b9d7ae73deb8136a4f09da47) ) // chip with VideoSystem logo V and 1

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "v5.u92", 0x00000, 0x20000, CRC(97d67510) SHA1(1ffd419e3dec7de1099cd5819b0309f7dd0df80e) ) // chip with VideoSystem logo V and 5

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "gha.u55", 0x00000, 0x80000, CRC(3e0cea91) SHA1(bab41715f106d364013b64649441d280bc6893cf) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "gh9.u61", 0x00000, 0x80000, CRC(5d1676bd) SHA1(6227d489c9c6259a0ac2bef62821fbf94efca8c6) )

	ROM_REGION( 0x400000, "spritegfx", 0 )
	ROM_LOAD32_WORD( "u42",    0x000000, 0x100000, CRC(65f0da84) SHA1(0bfbc6f4b87583703246704eb9fa13b1b3e6f90e) )
	ROM_LOAD32_WORD( "v3.u44", 0x200000, 0x020000, CRC(34bdead2) SHA1(99f9a8cac807fcd599db55d2dc624ed92a3862ef) ) // chip with VideoSystem logo V and 3
	ROM_LOAD32_WORD( "u43",    0x000002, 0x100000, CRC(7b349e5d) SHA1(8590a328f403e2c697a8d698c08d4adaf01fff62) )
	ROM_LOAD32_WORD( "v4.u45", 0x200002, 0x020000, CRC(be4d487d) SHA1(6d19c91d0498c43017219f0c10f4845a51ccfa7f) ) // chip with VideoSystem logo V and 4

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD32_WORD( "u59.ghb", 0x000000, 0x80000, CRC(158c9cde) SHA1(a2c1b404d40e6c2627691f5c7a3f63484bd5d2de) )
	ROM_LOAD32_WORD( "ghd.u60", 0x000002, 0x80000, CRC(73180ae3) SHA1(e4eaf6693826d9e72032d0a0e25938a23ab7d792) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "u105.gh8", 0x000000, 0x080000, CRC(7a68cb1b) SHA1(1bdd0000c2d68019b9e5bf8f7ad84a6ae1af8443) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "u104", 0x000000, 0x100000, CRC(5795e884) SHA1(a4178497ad0a1e60ceb87612b218d77b36d2a11b) )
ROM_END

/*

Karate Blazers (bootleg)

CPU : MC68000
SND : Z80 + NEC D7759c + YM3812 (no number on chip, but ROM is the same as streetsm, which uses a YM3812)

*/

ROM_START( karatblzbl )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "9.u5",        0x000000, 0x040000, CRC(33c3d3cd) SHA1(a8490091fe8d96b984ebbbe6b2f107fe23411814) )
	ROM_LOAD16_BYTE( "10.u2",       0x000001, 0x040000, CRC(dbed7323) SHA1(3ec2fffea41e606c176e2392b37f757ad6febf99) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "2.u80",        0x00000, 0x10000, CRC(ca4b171e) SHA1(a05fd81f68759a09be3ec09f38d7c9364dfb6c14) )

	ROM_REGION( 0x20000, "upd", 0 ) // D7759c data
	ROM_LOAD( "1.u88",        0x00000, 0x20000, CRC(47db1605) SHA1(ae00e633eb98567f04ff97e3d63e04e049d955ec) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "3.u81",      0x00000, 0x80000, CRC(3e0cea91) SHA1(bab41715f106d364013b64649441d280bc6893cf) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "4.u82",      0x00000, 0x80000, CRC(5d1676bd) SHA1(6227d489c9c6259a0ac2bef62821fbf94efca8c6) )

	ROM_REGION( 0x400000, "spritegfx", 0 )
	ROM_LOAD32_BYTE( "gfx14.u58",        0x000000, 0x020000, CRC(41e18169) SHA1(fc1723cb5d5ddf7dc3d418070605ad14770620e5) )
	ROM_LOAD32_BYTE( "gfx19.u63",        0x000001, 0x020000, CRC(09833f2a) SHA1(13d7d530c5f7baa4442c950c8a976934a3df2304) )
	ROM_LOAD32_BYTE( "gfx24.u68",        0x000002, 0x020000, CRC(a4417bf8) SHA1(791a3d70376915848fe1f26aa3152c96533c2783) )
	ROM_LOAD32_BYTE( "gfx29.u73",        0x000003, 0x020000, CRC(5affb5d0) SHA1(c89517dfb139d18f313f15c70481999f136ecb82) )
	ROM_LOAD32_BYTE( "gfx13.u59",        0x080000, 0x020000, CRC(666a8c19) SHA1(0392eb45b135db139d0149cbc140bdcaec627bf2) )
	ROM_LOAD32_BYTE( "gfx18.u64",        0x080001, 0x020000, CRC(4be67468) SHA1(67413b9fc33483133a8ef39c39a1f202be1efcbf) )
	ROM_LOAD32_BYTE( "gfx23.u69",        0x080002, 0x020000, CRC(3ff332e2) SHA1(2c3e3fc9af2735d21f0d14c08c731f148c5cc301) )
	ROM_LOAD32_BYTE( "gfx28.u74",        0x080003, 0x020000, CRC(ccfdd9ad) SHA1(53b4d68bcae4c84fb73f4b6b5d1d1503f125c3c4) )
	ROM_LOAD32_BYTE( "gfx12.u60",        0x100000, 0x020000, CRC(f425fe4c) SHA1(446b414e7f6bd699592dfdfc41e48d8f2f3a91b6) )
	ROM_LOAD32_BYTE( "gfx17.u65",        0x100001, 0x020000, CRC(96e77e04) SHA1(2c4f07c0bd7748470fb320c2e87bcf8c359d10ab) )
	ROM_LOAD32_BYTE( "gfx22.u70",        0x100002, 0x020000, CRC(5d673b74) SHA1(d54d22969c0a2d155775c48bea93f6946457d405) )
	ROM_LOAD32_BYTE( "gfx27.u75",        0x100003, 0x020000, CRC(a38802d4) SHA1(f29d69dbac451be3b855733b77b83f86b8f2af86) )
	ROM_LOAD32_BYTE( "gfx11.u61",        0x180000, 0x020000, CRC(09ae152b) SHA1(5d558e938f5d914f8bed2a8d980d08ad9e85d505) )
	ROM_LOAD32_BYTE( "gfx16.u66",        0x180001, 0x020000, CRC(cc3a2c8f) SHA1(d69a2127f6142c4d5b4363720511d564ceecf2b9) )
	ROM_LOAD32_BYTE( "gfx21.u71",        0x180002, 0x020000, CRC(ffd66ea0) SHA1(fc1b2fa27d28a61b381e3d4f15809c740082d07f) )
	ROM_LOAD32_BYTE( "gfx26.u76",        0x180003, 0x020000, CRC(7ae76103) SHA1(5c42fbe133cbf600d2150295a70a1541b79706b5) )
	ROM_LOAD32_BYTE( "gfx15.u57",        0x200000, 0x020000, CRC(876cf42e) SHA1(d9d5025f421f3dd297b519418a3cac528fcc269a) )
	ROM_LOAD32_BYTE( "gfx20.u62",        0x200001, 0x020000, CRC(8b759fde) SHA1(90ba7f71736a2bd35af045372db34d69d3015ff1) )
	ROM_LOAD32_BYTE( "gfx25.u67",        0x200002, 0x020000, CRC(1195b559) SHA1(2fd00b3360df5f0a762569ab49c445b68568cf2e) )
	ROM_LOAD32_BYTE( "gfx30.u72",        0x200003, 0x020000, CRC(7593679f) SHA1(3a6199d1dc60d2c05084fe41c639228613831d99) )

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD32_BYTE( "5.u62",        0x000000, 0x040000, CRC(1ed12174) SHA1(1e4fc511ad644aaf90505f7930957b4adf9f6c2a) )
	ROM_LOAD32_BYTE( "6.u63",        0x000001, 0x040000, CRC(874c5251) SHA1(487c2ff66e03e0c1d415039a11404c503b48f9c6) )
	ROM_LOAD32_BYTE( "7.u64",        0x000002, 0x040000, CRC(c2ed2666) SHA1(5c7b9cf46d13309facc8da82ef3eeb7ef3707ca0) )
	ROM_LOAD32_BYTE( "8.u65",        0x000003, 0x040000, CRC(b491201a) SHA1(c18cb1cc8a8bf031e00d8d89de62f7ed5548e767) )
ROM_END

/*

Turbo Force

 Program ROM comparision by region code:

              Region Code 3  Region Code 4  Region Code 7  Region Code 8
              ----------------------------------------------------------
   SUBPCB U2   0x721300ee     0x721300ee     0x721300ee    0x721300ee  <-- Same data all sets
   SUBPCB U1   0x71b6431b     0x6cd5312b     0xcc324da6    0xd1513f96  <-- Each set unique
Main PCB U14   0x63f50557     0x63f50557     0xc0a15480    0x63f50557  <-- Only turbofrcu differs by 1 byte:  0x240B:3C (vs 0x25 for other sets)

NOTE: Swapping in a 0x63f50557 ROM in turbofrcu in place of 8v3.u14 does NOT trigger a "PROGRAM ROM AREA" error.
      Is 8v3.u14 a bad dump?, need to get it redumped to verify if it should indeed be different.

*/
ROM_START( turbofrc ) // World version with no copyright notice
	ROM_REGION( 0xc0000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "4v2.subpcb.u2", 0x00000, 0x40000, CRC(721300ee) SHA1(79ab32fdfd377592a0bdbd1c4794cfd529a3eb7b) ) // 27c2048 - located on a OR-10 SUB BOARD - 4 stamped on chip with VideoSystem logo V
	ROM_LOAD16_WORD_SWAP( "4v1.subpcb.u1", 0x40000, 0x40000, CRC(6cd5312b) SHA1(57b109fe268fb963e981c91b6d288667a3c9a665) ) // 27c2048 - located on a OR-10 SUB BOARD - 4 stamped on chip with VideoSystem logo V
	ROM_LOAD16_WORD_SWAP( "4v3.u14",       0x80000, 0x40000, CRC(63f50557) SHA1(f8dba8c9ba412c9a67457ec31a804c57593ab20b) ) // 27c2048 - 4 stamped on chip with VideoSystem logo V

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "6.u166", 0x00000, 0x20000, CRC(2ca14a65) SHA1(95f6e7b4fa7ca26872ff472d7e6fb75fd4f281d5) ) // 27c1001

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "lh534ggs.u94", 0x000000, 0x80000, CRC(baa53978) SHA1(7f103122dd0bf675226ccf309fba73f645e0c79b) ) // mask rom
	ROM_LOAD( "7.u95",        0x080000, 0x20000, CRC(71a6c573) SHA1(f14ebca676d85fabcde27631145933abc376dd12) ) // 27c1001a

	ROM_REGION( 0x0a0000, "gfx2", 0 )
	ROM_LOAD( "lh534ggy.u105", 0x000000, 0x80000, CRC(4de4e59e) SHA1(571396dadb8aac043319cabe24e629210e442d57) ) // mask rom
	ROM_LOAD( "8.u106",        0x080000, 0x20000, CRC(c6479eb5) SHA1(47a58f082c73bc9dae3970e760ba46478ce6a190) ) // 27c1001a

	ROM_REGION( 0x200000, "spritegfx", 0 )
	ROM_LOAD32_WORD( "lh534gh2.u116", 0x000000, 0x80000, CRC(df210f3b) SHA1(990ac43e4a46fee6b929c5b27d317cdadf179b8b) ) // mask rom
	ROM_LOAD32_WORD( "5.u118",        0x100000, 0x40000, CRC(f61d1d79) SHA1(2b8e33912c05c26170afd2fced0ff06cb7a097fa) ) // 27c2048
	ROM_LOAD32_WORD( "lh534gh1.u117", 0x000002, 0x80000, CRC(f70812fd) SHA1(1964e1134940825211cd4825fdd3f13b8242192d) ) // mask rom
	ROM_LOAD32_WORD( "4.u119",        0x100002, 0x40000, CRC(474ea716) SHA1(67753e96fa4fc8cd689a8bddeb60dbde259cacaa) ) // 27c2048

	ROM_REGION( 0x080000, "gfx4", 0 )
	ROM_LOAD32_WORD( "lh532a52.u134", 0x000000, 0x40000, CRC(3c725a48) SHA1(120e62b2ef911bfa0f8a1468966ff70fab2d7582) ) // mask rom
	ROM_LOAD32_WORD( "lh532a51.u135", 0x000002, 0x40000, CRC(95c63559) SHA1(5f77bd22dce1ac4aa7291e5c3c3c358e2f066e8c) ) // mask rom

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "lh532h74.u180", 0x00000, 0x40000, CRC(a3d43254) SHA1(d0225d6cf9299ecc39d8e3f64f48cf80d554a67f) ) // mask rom

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "lh538o7j.u179", 0x000000, 0x100000, CRC(60ca0333) SHA1(28b94edc98d360386759780ccd1122d43ffa5279) ) // mask rom
ROM_END

ROM_START( turbofrco ) // World version with no copyright notice
	ROM_REGION( 0xc0000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "3v2.subpcb.u2", 0x00000, 0x40000, CRC(721300ee) SHA1(79ab32fdfd377592a0bdbd1c4794cfd529a3eb7b) ) // 27c2048 - located on a OR-10 SUB BOARD - 3 stamped on chip with VideoSystem logo V
	ROM_LOAD16_WORD_SWAP( "3v1.subpcb.u1", 0x40000, 0x40000, CRC(71b6431b) SHA1(471bada4730a1a323af9b493f8c5f57c2a147f8d) ) // 27c2048 - located on a OR-10 SUB BOARD - 3 stamped on chip with VideoSystem logo V
	ROM_LOAD16_WORD_SWAP( "3v3.u14",       0x80000, 0x40000, CRC(63f50557) SHA1(f8dba8c9ba412c9a67457ec31a804c57593ab20b) ) // 27c2048 - 3 stamped on chip with VideoSystem logo V

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "6.u166", 0x00000, 0x20000, CRC(2ca14a65) SHA1(95f6e7b4fa7ca26872ff472d7e6fb75fd4f281d5) ) // 27c1001

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "lh534ggs.u94", 0x000000, 0x80000, CRC(baa53978) SHA1(7f103122dd0bf675226ccf309fba73f645e0c79b) ) // mask rom
	ROM_LOAD( "7.u95",        0x080000, 0x20000, CRC(71a6c573) SHA1(f14ebca676d85fabcde27631145933abc376dd12) ) // 27c1001a

	ROM_REGION( 0x0a0000, "gfx2", 0 )
	ROM_LOAD( "lh534ggy.u105", 0x000000, 0x80000, CRC(4de4e59e) SHA1(571396dadb8aac043319cabe24e629210e442d57) ) // mask rom
	ROM_LOAD( "8.u106",        0x080000, 0x20000, CRC(c6479eb5) SHA1(47a58f082c73bc9dae3970e760ba46478ce6a190) ) // 27c1001a

	ROM_REGION( 0x200000, "spritegfx", 0 )
	ROM_LOAD32_WORD( "lh534gh2.u116", 0x000000, 0x80000, CRC(df210f3b) SHA1(990ac43e4a46fee6b929c5b27d317cdadf179b8b) ) // mask rom
	ROM_LOAD32_WORD( "5.u118",        0x100000, 0x40000, CRC(f61d1d79) SHA1(2b8e33912c05c26170afd2fced0ff06cb7a097fa) ) // 27c2048
	ROM_LOAD32_WORD( "lh534gh1.u117", 0x000002, 0x80000, CRC(f70812fd) SHA1(1964e1134940825211cd4825fdd3f13b8242192d) ) // mask rom
	ROM_LOAD32_WORD( "4.u119",        0x100002, 0x40000, CRC(474ea716) SHA1(67753e96fa4fc8cd689a8bddeb60dbde259cacaa) ) // 27c2048

	ROM_REGION( 0x080000, "gfx4", 0 )
	ROM_LOAD32_WORD( "lh532a52.u134", 0x000000, 0x40000, CRC(3c725a48) SHA1(120e62b2ef911bfa0f8a1468966ff70fab2d7582) ) // mask rom
	ROM_LOAD32_WORD( "lh532a51.u135", 0x000002, 0x40000, CRC(95c63559) SHA1(5f77bd22dce1ac4aa7291e5c3c3c358e2f066e8c) ) // mask rom

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "lh532h74.u180", 0x00000, 0x40000, CRC(a3d43254) SHA1(d0225d6cf9299ecc39d8e3f64f48cf80d554a67f) ) // mask rom

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "lh538o7j.u179", 0x000000, 0x100000, CRC(60ca0333) SHA1(28b94edc98d360386759780ccd1122d43ffa5279) ) // mask rom
ROM_END

ROM_START( turbofrcu ) // US version - need to redump 8v3.u14 to verify it's unique
	ROM_REGION( 0xc0000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "8v2.subpcb.u2", 0x00000, 0x40000, CRC(721300ee) SHA1(79ab32fdfd377592a0bdbd1c4794cfd529a3eb7b) ) // 27c2048 - located on a OR-10 SUB BOARD - 8 stamped on chip with VideoSystem logo V
	ROM_LOAD16_WORD_SWAP( "8v1.subpcb.u1", 0x40000, 0x40000, CRC(cc324da6) SHA1(ed2eaff7351914e3ebaf925ddc01be9d44d89fa6) ) // 27c2048 - located on a OR-10 SUB BOARD - 8 stamped on chip with VideoSystem logo V
	ROM_LOAD16_WORD_SWAP( "8v3.u14",       0x80000, 0x40000, CRC(c0a15480) SHA1(1ec99382e0a00a8167773b1d454a63cc5cd6199c) ) // 27c2048 - 8 stamped on chip with VideoSystem logo V - 0x240B:3C (vs 0x25 for other sets)

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "6.u166", 0x00000, 0x20000, CRC(2ca14a65) SHA1(95f6e7b4fa7ca26872ff472d7e6fb75fd4f281d5) ) // 27c1001

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "lh534ggs.u94", 0x000000, 0x80000, CRC(baa53978) SHA1(7f103122dd0bf675226ccf309fba73f645e0c79b) ) // mask rom
	ROM_LOAD( "7.u95",        0x080000, 0x20000, CRC(71a6c573) SHA1(f14ebca676d85fabcde27631145933abc376dd12) ) // 27c1001a

	ROM_REGION( 0x0a0000, "gfx2", 0 )
	ROM_LOAD( "lh534ggy.u105", 0x000000, 0x80000, CRC(4de4e59e) SHA1(571396dadb8aac043319cabe24e629210e442d57) ) // mask rom
	ROM_LOAD( "8.u106",        0x080000, 0x20000, CRC(c6479eb5) SHA1(47a58f082c73bc9dae3970e760ba46478ce6a190) ) // 27c1001a

	ROM_REGION( 0x200000, "spritegfx", 0 )
	ROM_LOAD32_WORD( "lh534gh2.u116", 0x000000, 0x80000, CRC(df210f3b) SHA1(990ac43e4a46fee6b929c5b27d317cdadf179b8b) ) // mask rom
	ROM_LOAD32_WORD( "5.u118",        0x100000, 0x40000, CRC(f61d1d79) SHA1(2b8e33912c05c26170afd2fced0ff06cb7a097fa) ) // 27c2048
	ROM_LOAD32_WORD( "lh534gh1.u117", 0x000002, 0x80000, CRC(f70812fd) SHA1(1964e1134940825211cd4825fdd3f13b8242192d) ) // mask rom
	ROM_LOAD32_WORD( "4.u119",        0x100002, 0x40000, CRC(474ea716) SHA1(67753e96fa4fc8cd689a8bddeb60dbde259cacaa) ) // 27c2048

	ROM_REGION( 0x080000, "gfx4", 0 )
	ROM_LOAD32_WORD( "lh532a52.u134", 0x000000, 0x40000, CRC(3c725a48) SHA1(120e62b2ef911bfa0f8a1468966ff70fab2d7582) ) // mask rom
	ROM_LOAD32_WORD( "lh532a51.u135", 0x000002, 0x40000, CRC(95c63559) SHA1(5f77bd22dce1ac4aa7291e5c3c3c358e2f066e8c) ) // mask rom

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "lh532h74.u180", 0x00000, 0x40000, CRC(a3d43254) SHA1(d0225d6cf9299ecc39d8e3f64f48cf80d554a67f) ) // mask rom

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "lh538o7j.u179", 0x000000, 0x100000, CRC(60ca0333) SHA1(28b94edc98d360386759780ccd1122d43ffa5279) ) // mask rom
ROM_END

ROM_START( turbofrcua ) // US version
	ROM_REGION( 0xc0000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "7v2.subpcb.u2", 0x00000, 0x40000, CRC(721300ee) SHA1(79ab32fdfd377592a0bdbd1c4794cfd529a3eb7b) ) // 27c2048 - located on a OR-10 SUB BOARD - 7 stamped on chip with VideoSystem logo V
	ROM_LOAD16_WORD_SWAP( "7v1.subpcb.u1", 0x40000, 0x40000, CRC(d1513f96) SHA1(7acb96a44f661cd9c7561dfab0d60c993942d669) ) // 27c2048 - located on a OR-10 SUB BOARD - 7 stamped on chip with VideoSystem logo V
	ROM_LOAD16_WORD_SWAP( "7v3.u14",       0x80000, 0x40000, CRC(63f50557) SHA1(f8dba8c9ba412c9a67457ec31a804c57593ab20b) ) // 27c2048 - 7 stamped on chip with VideoSystem logo V

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "6.u166", 0x00000, 0x20000, CRC(2ca14a65) SHA1(95f6e7b4fa7ca26872ff472d7e6fb75fd4f281d5) ) // 27c1001

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x0a0000, "gfx1", 0 )
	ROM_LOAD( "lh534ggs.u94", 0x000000, 0x80000, CRC(baa53978) SHA1(7f103122dd0bf675226ccf309fba73f645e0c79b) ) // mask rom
	ROM_LOAD( "7.u95",        0x080000, 0x20000, CRC(71a6c573) SHA1(f14ebca676d85fabcde27631145933abc376dd12) ) // 27c1001a

	ROM_REGION( 0x0a0000, "gfx2", 0 )
	ROM_LOAD( "lh534ggy.u105", 0x000000, 0x80000, CRC(4de4e59e) SHA1(571396dadb8aac043319cabe24e629210e442d57) ) // mask rom
	ROM_LOAD( "8.u106",        0x080000, 0x20000, CRC(c6479eb5) SHA1(47a58f082c73bc9dae3970e760ba46478ce6a190) ) // 27c1001a

	ROM_REGION( 0x200000, "spritegfx", 0 )
	ROM_LOAD32_WORD( "lh534gh2.u116", 0x000000, 0x80000, CRC(df210f3b) SHA1(990ac43e4a46fee6b929c5b27d317cdadf179b8b) ) // mask rom
	ROM_LOAD32_WORD( "5.u118",        0x100000, 0x40000, CRC(f61d1d79) SHA1(2b8e33912c05c26170afd2fced0ff06cb7a097fa) ) // 27c2048
	ROM_LOAD32_WORD( "lh534gh1.u117", 0x000002, 0x80000, CRC(f70812fd) SHA1(1964e1134940825211cd4825fdd3f13b8242192d) ) // mask rom
	ROM_LOAD32_WORD( "4.u119",        0x100002, 0x40000, CRC(474ea716) SHA1(67753e96fa4fc8cd689a8bddeb60dbde259cacaa) ) // 27c2048

	ROM_REGION( 0x080000, "gfx4", 0 )
	ROM_LOAD32_WORD( "lh532a52.u134", 0x000000, 0x40000, CRC(3c725a48) SHA1(120e62b2ef911bfa0f8a1468966ff70fab2d7582) ) // mask rom
	ROM_LOAD32_WORD( "lh532a51.u135", 0x000002, 0x40000, CRC(95c63559) SHA1(5f77bd22dce1ac4aa7291e5c3c3c358e2f066e8c) ) // mask rom

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "lh532h74.u180", 0x00000, 0x40000, CRC(a3d43254) SHA1(d0225d6cf9299ecc39d8e3f64f48cf80d554a67f) ) // mask rom

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "lh538o7j.u179", 0x000000, 0x100000, CRC(60ca0333) SHA1(28b94edc98d360386759780ccd1122d43ffa5279) ) // mask rom
ROM_END

ROM_START( aerofgt )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "1.u4",         0x00000, 0x80000, CRC(6fdff0a2) SHA1(7cc9529b426091027aa3e23586cb7d162376c0ff) )

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "2.153",        0x00000, 0x20000, CRC(a1ef64ec) SHA1(fa3e434738bf4e742ad68882c1e914100ce0f761) )

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_WORD_SWAP( "538a54.124",   0x000000, 0x80000, CRC(4d2c4df2) SHA1(f51c2b3135f0a921ac1a79e63d6878c03cb6254b) )
	ROM_LOAD16_WORD_SWAP( "1538a54.124",  0x080000, 0x80000, CRC(286d109e) SHA1(3a5f3d2d89cf58f6ef15e4bd3f570b84e8e695b2) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_WORD_SWAP( "538a53.u9",    0x000000, 0x100000, CRC(630d8e0b) SHA1(5a0c252ccd53c5199a695909d25ecb4e53dc15b9) )
	ROM_LOAD16_WORD_SWAP( "534g8f.u18",   0x200000, 0x080000, CRC(76ce0926) SHA1(5ef4cec215d4dd600d8fcd1bd9a4c09081d59e33) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "it-19-01",     0x00000, 0x40000, CRC(6d42723d) SHA1(57c59234e9925430a4c687733682efed06d7eed1) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "it-19-06",     0x000000, 0x100000, CRC(cdbbdb1d) SHA1(067c816545f246ff1fd4c821d70df1e7eb47938c) )
ROM_END

ROM_START( aerofgtb )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "v2",                0x00000, 0x40000, CRC(5c9de9f0) SHA1(93b62c59f0bc052c6fdbd5aae292a7ab2122dfd1) )
	ROM_LOAD16_BYTE( "v1",                0x00001, 0x40000, CRC(89c1dcf4) SHA1(41401d63049c140e4254dc791022d85c44271390) )

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "v3",           0x00000, 0x20000, CRC(cbb18cf4) SHA1(7119a7536cf710660ff06d1e7d2879c79ef12b3d) )

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "it-19-03",     0x000000, 0x80000, CRC(85eba1a4) SHA1(5691a95d6359fdab29be0d615066370c2b856c0a) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "it-19-02",     0x000000, 0x80000, CRC(4f57f8ba) SHA1(aaad548e9a7490dfd48a975135716225f416b6f6) )

	ROM_REGION( 0x100000, "spritegfx", 0 )
	ROM_LOAD32_WORD_SWAP( "it-19-04",     0x000000, 0x80000, CRC(3b329c1f) SHA1(279cb32d69ce1e71f42cfad93d395794a3e92bc6) )
	ROM_LOAD32_WORD_SWAP( "it-19-05",     0x000002, 0x80000, CRC(02b525af) SHA1(07f23d15938dfbdc4f0977ba1463a06090569026) )

	ROM_REGION( 0x080000, "gfx4", 0 )
	ROM_LOAD32_WORD_SWAP( "g27",          0x000000, 0x40000, CRC(4d89cbc8) SHA1(93f248f3dc1a15c32d14a147b37d5d660d0e4337) )
	ROM_LOAD32_WORD_SWAP( "g26",          0x000002, 0x40000, CRC(8072c1d2) SHA1(c14634f5f2686cf616f415d9ea4a0c6490054beb) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "it-19-01",     0x00000, 0x40000, CRC(6d42723d) SHA1(57c59234e9925430a4c687733682efed06d7eed1) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "it-19-06",     0x000000, 0x100000, CRC(cdbbdb1d) SHA1(067c816545f246ff1fd4c821d70df1e7eb47938c) )
ROM_END

ROM_START( aerofgtc )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "v2.149",            0x00000, 0x40000, CRC(f187aec6) SHA1(8905af34f114ae22fbfbd3ae115f19280bdd4fb3) )
	ROM_LOAD16_BYTE( "v1.111",            0x00001, 0x40000, CRC(9e684b19) SHA1(b5e1e5b74ed9fd223c9315ee2d548e620224c102) )

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "2.153",        0x00000, 0x20000, CRC(a1ef64ec) SHA1(fa3e434738bf4e742ad68882c1e914100ce0f761) )

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	// gfx ROMs were missing in this set, I'm using the aerofgtb ones
	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "it-19-03",     0x000000, 0x80000, CRC(85eba1a4) SHA1(5691a95d6359fdab29be0d615066370c2b856c0a) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "it-19-02",     0x000000, 0x80000, CRC(4f57f8ba) SHA1(aaad548e9a7490dfd48a975135716225f416b6f6) )

	ROM_REGION( 0x100000, "spritegfx", 0 )
	ROM_LOAD32_WORD_SWAP( "it-19-04",     0x000000, 0x80000, CRC(3b329c1f) SHA1(279cb32d69ce1e71f42cfad93d395794a3e92bc6) )
	ROM_LOAD32_WORD_SWAP( "it-19-05",     0x000002, 0x80000, CRC(02b525af) SHA1(07f23d15938dfbdc4f0977ba1463a06090569026) )

	ROM_REGION( 0x080000, "gfx4", 0 )
	ROM_LOAD32_WORD_SWAP( "g27",          0x000000, 0x40000, CRC(4d89cbc8) SHA1(93f248f3dc1a15c32d14a147b37d5d660d0e4337) )
	ROM_LOAD32_WORD_SWAP( "g26",          0x000002, 0x40000, CRC(8072c1d2) SHA1(c14634f5f2686cf616f415d9ea4a0c6490054beb) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "it-19-01",     0x00000, 0x40000, CRC(6d42723d) SHA1(57c59234e9925430a4c687733682efed06d7eed1) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "it-19-06",     0x000000, 0x100000, CRC(cdbbdb1d) SHA1(067c816545f246ff1fd4c821d70df1e7eb47938c) )
ROM_END

ROM_START( sonicwi )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.149",        0x00000, 0x40000, CRC(3d1b96ba) SHA1(941be323c0cb15e05c92b897984617b05c5cf676) )
	ROM_LOAD16_BYTE( "1.111",        0x00001, 0x40000, CRC(a3d09f94) SHA1(a1064d659488878f5303edc2b8636312ab632a83) )

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "2.153",        0x00000, 0x20000, CRC(a1ef64ec) SHA1(fa3e434738bf4e742ad68882c1e914100ce0f761) )  // 3.156

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	// gfx ROMs were missing in this set, I'm using the aerofgtb ones
	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "it-19-03",     0x000000, 0x80000, CRC(85eba1a4) SHA1(5691a95d6359fdab29be0d615066370c2b856c0a) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "it-19-02",     0x000000, 0x80000, CRC(4f57f8ba) SHA1(aaad548e9a7490dfd48a975135716225f416b6f6) )

	ROM_REGION( 0x100000, "spritegfx", 0 )
	ROM_LOAD32_WORD_SWAP( "it-19-04",     0x000000, 0x80000, CRC(3b329c1f) SHA1(279cb32d69ce1e71f42cfad93d395794a3e92bc6) )
	ROM_LOAD32_WORD_SWAP( "it-19-05",     0x000002, 0x80000, CRC(02b525af) SHA1(07f23d15938dfbdc4f0977ba1463a06090569026) )

	ROM_REGION( 0x080000, "gfx4", 0 )
	ROM_LOAD32_WORD_SWAP( "g27",          0x000000, 0x40000, CRC(4d89cbc8) SHA1(93f248f3dc1a15c32d14a147b37d5d660d0e4337) )
	ROM_LOAD32_WORD_SWAP( "g26",          0x000002, 0x40000, CRC(8072c1d2) SHA1(c14634f5f2686cf616f415d9ea4a0c6490054beb) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "it-19-01",     0x00000, 0x40000, CRC(6d42723d) SHA1(57c59234e9925430a4c687733682efed06d7eed1) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "it-19-06",     0x000000, 0x100000, CRC(cdbbdb1d) SHA1(067c816545f246ff1fd4c821d70df1e7eb47938c) )
ROM_END

ROM_START( aerfboot )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "afb_ep2.u3",  0x00000, 0x40000, CRC(2bb9edf7) SHA1(cf0a62070fc0803dd8c473c375f6a2d1884ba2bf) )
	ROM_LOAD16_BYTE( "afb_ep3.u2",  0x00001, 0x40000, CRC(475d3df3) SHA1(58bde24e9dea2fb0d7ae4f2a574b06bc1a33a13d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "afb_ep1.u17",  0x0000, 0x8000, CRC(d41b5ab2) SHA1(17d9b999c9af1f332d67e7ce1a2f71fd08178303) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "afb_ep9.hh",   0x000000, 0x40000, CRC(41233923) SHA1(20f2849407ac7bd851d2617ad72fd413775da410) )
	ROM_LOAD( "afb_ep8.hi",   0x040000, 0x40000, CRC(97607ad3) SHA1(fb72e7ef0c6f7a736e12a9ff71017460f866195e) )
	ROM_LOAD( "afb_ep7.hj",   0x080000, 0x40000, CRC(01dc793e) SHA1(dbd9d22d75f5bcef9102667722cebb75574badd3) )
	ROM_LOAD( "afb_ep6.hk",   0x0c0000, 0x40000, CRC(cad7862a) SHA1(bfd729b19ff740ad3dc3b645c4f07f71126c0f3e) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "afb_ep12.tc",  0x000000, 0x80000, CRC(1e692065) SHA1(a67da59cd65ec492d6e6ab14b1800fd35480a52d) )
	ROM_LOAD( "afb_ep10.ta",  0x080000, 0x80000, CRC(e50db1a7) SHA1(952676879fb6a260c56a120b849abfae75f4cf2b) )

	ROM_REGION( 0x080000, "spritegfx", 0 )
	ROM_LOAD( "afb_ep13.td",  0x000000, 0x40000, CRC(1830f70c) SHA1(1759de9b56e4999defc08b2423eff38ec98c4f17) )
	ROM_LOAD( "afb_ep11.tb",  0x040000, 0x40000, CRC(6298c0eb) SHA1(ede63849973742c67637eac0ec9cda95ea2ecebc) )

	ROM_REGION( 0xa0000, "oki", ROMREGION_ERASEFF ) // sound samples
	ROM_LOAD( "afb_ep5.u29",  0x000000, 0x20000, CRC(3559609a) SHA1(6f0b633bf74f41487fc98dcdc43a83eb67f3d14c) )
	ROM_LOAD( "afb_ep4.u30",  0x020000, 0x80000, CRC(f9652163) SHA1(d8c1fcf44b350cc65378869e4eb188ea232b4948) )
ROM_END

ROM_START( aerfboo2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "p2",  0x00000, 0x40000, CRC(6c4ec09b) SHA1(cdfb8c59ddd6360487fee017d5093636aa52c5c2) )
	ROM_LOAD16_BYTE( "p1",  0x00001, 0x40000, CRC(841c513a) SHA1(819e634f0aec29b1863c9cf0118cc33154d10037) )

	// No z80 on this bootleg

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "g5"        ,   0x000000, 0x80000, CRC(1c2bd86c) SHA1(f16d7eba967d76faaaeae5101db43141ef9e2eed) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "g6"        ,   0x000000, 0x80000, CRC(b9b1b9b0) SHA1(c25e1ef8b5ecb4b630fb850fe483d7efb0544a6c) )

	ROM_REGION( 0x200000, "spritegfx", 0 )
	ROM_LOAD32_BYTE( "g2"        ,   0x000000, 0x80000, CRC(84774dbd) SHA1(731b08a62446ff9cf36a43d42d217f73b4e2437c) )
	ROM_LOAD32_BYTE( "g1"        ,   0x000001, 0x80000, CRC(4ab31e69) SHA1(1c6bf5bf4a887cf21da76c6a874f8ff5d3540e3a) )
	ROM_LOAD32_BYTE( "g4"        ,   0x000002, 0x80000, CRC(97725694) SHA1(59316e4be043e0b7111c6777b36bcfd39c899e72) )
	ROM_LOAD32_BYTE( "g3"        ,   0x000003, 0x80000, CRC(7be8cef0) SHA1(b227252fd288e8eb06507397f3ad625465dc1b0a) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASEFF ) // sound samples
	ROM_LOAD( "s2"        ,     0x00000, 0x80000, CRC(2e316ee8) SHA1(a163dddee6d8cfd1286059ee561e3a01df49381b) )
	ROM_LOAD( "s1"        ,     0x80000, 0x80000, CRC(9e09813d) SHA1(582a36b5a46f4d8eaedca22e583b6949535d24a5) )
ROM_END

ROM_START( wbbc97 )
	ROM_REGION( 0x400000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "03.27c040.ur4.rom",  0x000001, 0x80000, CRC(fb4e48fc) SHA1(cffc75766a9b867ab73597156142aa7c70bf6f20) )
	ROM_LOAD16_BYTE( "07.27c040.uo4.rom",  0x000000, 0x80000, CRC(87605dcc) SHA1(c5d05e7c581e02f88fd42c65768f5c8632e571a1) )
	ROM_LOAD16_BYTE( "04.27c4000.ur4a.rom",0x100001, 0x80000, CRC(2dd6ff07) SHA1(54724f49d4ca1db16a799704a9e023f6ee407fee) )
	ROM_LOAD16_BYTE( "08.27c4000.uo4a.rom",0x100000, 0x80000, CRC(1b96ef5b) SHA1(10bfecfc18c65735ddecf830dd72dd855ecf5ee7) )
	ROM_LOAD16_BYTE( "05.27c4000.ur4b.rom",0x200001, 0x80000, CRC(84104886) SHA1(807d4441bde6535b780c0c680773804b1268a024) )
	ROM_LOAD16_BYTE( "09.27c4000.uo4b.rom",0x200000, 0x80000, CRC(0367043c) SHA1(a5b77730e17b6223a8b465fe36d9447b60eb51ab) )
	ROM_LOAD16_BYTE( "06.27c4000.ur4c.rom",0x300001, 0x80000, CRC(b22d11c4) SHA1(15d2ba97704bbcf9d851b650a9c56a6a668cfe63) )
	ROM_LOAD16_BYTE( "10.27c040.uo4c.rom", 0x300000, 0x80000, CRC(fe403e8b) SHA1(5f8202792d9ec3e0404637614277c0375c747f7e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // z80 code
	ROM_LOAD( "02.27c512.su11.rom",  0x000000, 0x10000, CRC(f03178e9) SHA1(5b0abee03059109a3cdb08a9341091255d5cb6ae) )

	ROM_REGION( 0x40000, "gfx1", 0 )    // GFX
	ROM_LOAD( "15.27c020.uu10.rom",  0x000000, 0x40000, CRC(965bc99e) SHA1(db72121cfbcd6916f46ac5bd3592681eafa4e5da) )

	ROM_REGION( 0x100000, "gfx2", 0 )   // GFX
	ROM_LOAD32_BYTE( "11.27c020.ue12.rom", 0x000003, 0x40000, CRC(a0b23c8a) SHA1(85ccc5dcc891a352b90f0f3d89f6115bc52face6) )
	ROM_LOAD32_BYTE( "12.27c020.ue11.rom", 0x000002, 0x40000, CRC(4e529623) SHA1(b3e1e1ba5e05f7e095c0409f199c89b81297cf40) )
	ROM_LOAD32_BYTE( "13.27c020.ue10.rom", 0x000001, 0x40000, CRC(3745f892) SHA1(085986dff9639dedaee3bcecca17a6ea7e4a45f4) )
	ROM_LOAD32_BYTE( "14.27c020.ue9.rom",  0x000000, 0x40000, CRC(2814f4d2) SHA1(bf459b9ff160d0f18d74224d5e0729b8120261e6) )

	ROM_REGION( 0x40000, "oki", 0 ) // OKIM6295 samples
	ROM_LOAD( "01.27c020.su10.rom",  0x000000, 0x40000, CRC(c024e48c) SHA1(d3caedd22044c1645d96301a93f794db3ff77047) )

	ROM_REGION( 0x200, "user1", 0 ) // ???
	ROM_LOAD( "82s147a.rom",  0x000000, 0x200, CRC(72cec9d2) SHA1(1c6fe6b47fe24bdbebb51d6bef56bf71c9029e72) )
ROM_END

ROM_START( kickball )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "kickball.1", 0x000000, 0x040000, CRC(f0fd971d) SHA1(b887196d9ff62403fd27e41ebda0a1d13215a456) )
	ROM_LOAD16_BYTE( "kickball.2", 0x000001, 0x040000, CRC(7dab432d) SHA1(e077ea552e9aba4ecc82abd07ea5e94f0f09303f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "kickball.4", 0x00000, 0x10000, CRC(ef10c2bf) SHA1(44fd03a28221dffe9c4281ef920fe975bbfb67f0) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD( "kickball.9",  0x000000, 0x080000, CRC(19be87f3) SHA1(661966683f74b4fbfd77eab4477fb0d75e87230e) )
	ROM_LOAD( "kickball.10", 0x080000, 0x080000, CRC(e3b4f894) SHA1(44b107b87cf9e94f67cfac98b67abed874d534c0) )

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )
	ROM_LOAD( "kickball.5", 0x000000, 0x080000, CRC(050b6387) SHA1(59aa685014a6f138e14dbfe858c6ecc6514e44f6) )
	ROM_LOAD( "kickball.6", 0x080000, 0x080000, CRC(1e55252f) SHA1(ce1604921af26e8da2fa4cf4a49c67f3b7d4222d) )
	ROM_LOAD( "kickball.7", 0x100000, 0x080000, CRC(b2ee5218) SHA1(65e240b3ddb673b593404525aa2775c342228130) )
	ROM_LOAD( "kickball.8", 0x180000, 0x080000, CRC(5f1b07f8) SHA1(add1f66fe09684ce65a54752cc90d7f0a05efc4f) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "kickball.3", 0x000000, 0x040000, CRC(2f3ed4c1) SHA1(4688df5d420343a935d066f3b46580b77ee77b0e) )
ROM_END

void aerofgt_base_state::init_pspikesb()
{
	m_okibank->configure_entries(0, 4, memregion("oki")->base(), 0x20000);
	m_okibank->set_entry(0);
}


void aerofgt_sound_cpu_state::init_banked_oki()
{
	m_okibank->configure_entries(0, 4, memregion("oki")->base() + 0x20000, 0x20000);
}


void aerofgt_sound_cpu_state::init_kickball()
{
	// 2 lines on 1 gfx rom are swapped, why?
	uint8_t *src = memregion("gfx2")->base();
	for (int i = 0; i < 0x80000; i++)
	{
		src[i] = bitswap<8>(src[i], 7, 5, 6, 4, 3, 2, 1, 0);
	}
}

GAME( 1990, spinlbrk,   0,        spinlbrk,   spinlbrk,  aerofgt_banked_sound_state, empty_init,     ROT0,   "V-System Co.",     "Spinal Breakers (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1990, spinlbrku,  spinlbrk, spinlbrk,   spinlbrku, aerofgt_banked_sound_state, empty_init,     ROT0,   "V-System Co.",     "Spinal Breakers (US)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1990, spinlbrkj,  spinlbrk, spinlbrk,   spinlbrk,  aerofgt_banked_sound_state, empty_init,     ROT0,   "V-System Co.",     "Spinal Breakers (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1990, spinlbrkup, spinlbrk, spinlbrk,   spinlbrk,  aerofgt_banked_sound_state, empty_init,     ROT0,   "V-System Co.",     "Spinal Breakers (US, prototype)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL ) // build date 11/15 15:00

GAME( 1991, pspikes,    0,        pspikes,    pspikes,   aerofgt_banked_sound_state, empty_init,      ROT0,   "Video System Co.",    "Power Spikes (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, pspikesk,   pspikes,  pspikes,    pspikes,   aerofgt_banked_sound_state, empty_init,      ROT0,   "Video System Co.",    "Power Spikes (Korea)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, pspikesu,   pspikes,  pspikes,    pspikes,   aerofgt_banked_sound_state, empty_init,      ROT0,   "Video System Co.",    "Power Spikes (US)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, svolly91,   pspikes,  pspikes,    pspikes,   aerofgt_banked_sound_state, empty_init,      ROT0,   "Video System Co.",    "Super Volley '91 (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, pspikesb,   pspikes,  pspikesb,   pspikesb,  aerofgt_base_state,         init_pspikesb,   ROT0,   "bootleg",             "Power Spikes (bootleg)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, pspikesba,  pspikes,  pspikesb,   pspikesb,  aerofgt_base_state,         init_pspikesb,   ROT0,   "bootleg (Playmark?)", "Power Spikes (Italian bootleg)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, spikes91,   pspikes,  spikes91,   pspikes,   spikes91_state,             empty_init,      ROT0,   "bootleg",             "1991 Spikes (Italian bootleg, set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NO_COCKTAIL ) // OKI M5205 not hooked up yet
GAME( 1991, spikes91b,  pspikes,  spikes91,   pspikes,   spikes91_state,             empty_init,      ROT0,   "bootleg",             "1991 Spikes (Italian bootleg, set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NO_COCKTAIL ) // OKI M5205 not hooked up yet
GAME( 1991, pspikesc,   pspikes,  pspikesc,   pspikesc,  aerofgt_base_state,         init_pspikesb,   ROT0,   "bootleg",             "Power Spikes (China, bootleg)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND )
GAME( 1997, wbbc97,     0,        wbbc97,     wbbc97,    wbbc97_state,               empty_init,      ROT0,   "Comad",               "Beach Festival World Championship 1997", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL ) // based on power spikes codebase
GAME( 1998, kickball,   0,        kickball,   pspikes,   aerofgt_sound_cpu_state,    init_kickball,   ROT0,   "Seoung Youn",    "Kick Ball", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS ) // based on power spikes codebase, wrong priorities

GAME( 1991, karatblz,   0,        karatblz,   karatblz,  aerofgt_banked_sound_state, empty_init,      ROT0,   "Video System Co.",                 "Karate Blazers (World, set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, karatblza,  karatblz, karatblz,   karatblz,  aerofgt_banked_sound_state, empty_init,      ROT0,   "Video System Co.",                 "Karate Blazers (World, set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, karatblzt,  karatblz, karatblz,   karatblz,  aerofgt_banked_sound_state, empty_init,      ROT0,   "Video System Co. (Tecmo license)", "Karate Blazers (World, Tecmo license)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, karatblzu,  karatblz, karatblz,   karatblzu, aerofgt_banked_sound_state, empty_init,      ROT0,   "Video System Co.",                 "Karate Blazers (US)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, karatblzj,  karatblz, karatblz,   karatblz,  aerofgt_banked_sound_state, empty_init,      ROT0,   "Video System Co. (Tecmo license)", "Toushin Blazers (Japan, Tecmo license)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, karatblzbl, karatblz, karatblzbl, karatblz,  karatblzbl_state,           empty_init,      ROT0,   "bootleg",                          "Karate Blazers (bootleg with Street Smart sound hardware)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND )

// according to Gamest magazine in new revision they changed the points value of the rocks in level 6 (5'000 versus 500)
// turbofrcua gives 5'000, the others 500.
// NOTE: turbofrcua also denotes HP rank on these rocks, getting there without any life lost (almost impossible without cheating) makes those literally indestructible.
GAME( 1991, turbofrc,   0,        turbofrc,   turbofrc,  aerofgt_banked_sound_state, empty_init,      ROT270, "Video System Co.",    "Turbo Force (World, set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL ) // region code 4
GAME( 1991, turbofrco,  turbofrc, turbofrc,   turbofrc,  aerofgt_banked_sound_state, empty_init,      ROT270, "Video System Co.",    "Turbo Force (World, set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL ) // region code 3
GAME( 1991, turbofrcu,  turbofrc, turbofrc,   turbofrc,  aerofgt_banked_sound_state, empty_init,      ROT270, "Video System Co.",    "Turbo Force (US, set 1)",    MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL ) // region code 8
GAME( 1991, turbofrcua, turbofrc, turbofrc,   turbofrc,  aerofgt_banked_sound_state, empty_init,      ROT270, "Video System Co.",    "Turbo Force (US, set 2)",    MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL ) // region code 7

// the tiles on these also contain an alt title 'The Final War' for both the title screen and attract logo was it ever used?
// sonicwi looks oldest set, aerofgt is slightly easier, aerofgtb/aerofgtc are noticeably harder
// aerofgtb sports an extra srl->rr in Z80 code at PC=1a8a compared to all other sets, bugfix rev?
GAME( 1992, aerofgt,    0,        aerofgt,    aerofgt,   aerofgt_state,              empty_init,      ROT270, "Video System Co.",    "Aero Fighters (World / USA + Canada / Korea / Hong Kong / Taiwan) (newer hardware)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL ) // this has the newer sprite chip etc. unlike all other games in this driver..
GAME( 1992, aerofgtb,   aerofgt,  aerofgtb,   aerofgtb,  aerofgt_banked_sound_state, empty_init,      ROT270, "Video System Co.",    "Aero Fighters (Taiwan / Japan, set 1)", MACHINE_SUPPORTS_SAVE ) // probably intended for Taiwan because the Japanese name is Sonic Wings (below)
GAME( 1992, aerofgtc,   aerofgt,  aerofgtb,   aerofgtb,  aerofgt_banked_sound_state, empty_init,      ROT270, "Video System Co.",    "Aero Fighters (Taiwan / Japan, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sonicwi,    aerofgt,  aerofgtb,   aerofgtb,  aerofgt_banked_sound_state, empty_init,      ROT270, "Video System Co.",    "Sonic Wings (Japan)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1992, aerfboot,   aerofgt,  aerfboot,   aerofgtb,  aerofgt_sound_cpu_state,    init_banked_oki, ROT270, "bootleg",             "Aero Fighters (bootleg, set 1)",         MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND )
GAME( 1992, aerfboo2,   aerofgt,  aerfboo2,   aerofgtb,  aerofgt_base_state,         empty_init,      ROT270, "bootleg",             "Aero Fighters (bootleg, set 2)",         MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND )
