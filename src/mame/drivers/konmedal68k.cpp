// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

 konmedal68k.cpp: Konami 68000 based medal games

 Pittanko Zaurus (ピッタンコ　ザウルス)
 GS562
 (c) 1995 Konami

 Konami ICs:
 K058143 + K056832 = tilemaps
 K055555 = priority blender
 K056766 = color DAC
 K056879 = input/EEPROM interface

 800000 = control
 bit 3 = write 1 to enable or ack IRQ 3
 bit 4 = write 1 to enable or ack IRQ 4

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/ymz280b.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k055555.h"
#include "video/konami_helper.h"
#include "screen.h"
#include "speaker.h"

class konmedal68k_state : public driver_device
{
public:
	konmedal68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_k056832(*this, "k056832"),
		m_k055555(*this, "k055555"),
		m_palette(*this, "palette"),
		m_ymz(*this, "ymz")
	{ }

	uint32_t screen_update_konmedal68k(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void kzaurus(machine_config &config);

	K056832_CB_MEMBER(tile_callback);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<k056832_device> m_k056832;
	required_device<k055555_device> m_k055555;
	required_device<palette_device> m_palette;
	required_device<ymz280b_device> m_ymz;
};

void konmedal68k_state::video_start()
{
}

K056832_CB_MEMBER(konmedal68k_state::tile_callback)
{
	int codebits = *code;
//  int mode, data; //, bank;

//  m_k056832->read_avac(&mode, &data);

	*color = 1;
	*code = codebits; // | (bank << 10);
}

uint32_t konmedal68k_state::screen_update_konmedal68k(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);

	// game only draws on this layer, apparently
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 0, 0, 1);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 1, 0, 1);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 2, 0, 1);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 3, 0, 1);

//  for (int i = 0; i < 5; i++)
//      printf("idx %d %x  ", i, m_k055555->K055555_get_palette_index(0));
//  printf("\n");
	return 0;
}

static ADDRESS_MAP_START( kzaurus_main, AS_PROGRAM, 16, konmedal68k_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x400000, 0x403fff) AM_RAM
	AM_RANGE(0x800000, 0x800001) AM_WRITENOP    // watchdog?  control?
	AM_RANGE(0x810000, 0x810001) AM_WRITENOP
	AM_RANGE(0x830000, 0x83003f) AM_DEVWRITE("k056832", k056832_device, word_w)
	AM_RANGE(0x840000, 0x84000f) AM_DEVWRITE("k056832", k056832_device, b_word_w)
	AM_RANGE(0x870000, 0x87005f) AM_DEVWRITE("k055555", k055555_device, K055555_word_w)
	AM_RANGE(0xa00000, 0xa01fff) AM_DEVREADWRITE("k056832", k056832_device, ram_word_r, ram_word_w)
	AM_RANGE(0xb00000, 0xb01fff) AM_RAM_DEVWRITE("palette", palette_device, write16) AM_SHARE("palette")
	AM_RANGE(0xc00000, 0xc01fff) AM_DEVREAD("k056832", k056832_device, piratesh_rom_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( kzaurus )
INPUT_PORTS_END

void konmedal68k_state::machine_start()
{
}

void konmedal68k_state::machine_reset()
{
}

MACHINE_CONFIG_START(konmedal68k_state::kzaurus)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL(33'868'800)/4 )    // 33.8688 MHz crystal verified on PCB
	MCFG_CPU_PROGRAM_MAP(kzaurus_main)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.62)  /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(80, 400-1, 16, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(konmedal68k_state, screen_update_konmedal68k)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 8192)
	MCFG_PALETTE_ENABLE_SHADOWS()
	MCFG_PALETTE_FORMAT(XBGR)
	//MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("k056832", K056832, 0)
	MCFG_K056832_CB(konmedal68k_state, tile_callback)
	MCFG_K056832_CONFIG("gfx1", K056832_BPP_4dj, 1, 0, "none")
	MCFG_K056832_PALETTE("palette")

	MCFG_K055555_ADD("k055555")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_DEVICE_ADD("ymz", YMZ280B, XTAL(33'868'800)/2) // 33.8688 MHz xtal verified on PCB
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

ROM_START( kzaurus )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* main program */
	ROM_LOAD16_WORD_SWAP( "540-b05-2n.bin", 0x000000, 0x080000, CRC(110d4ecb) SHA1(8903783f62ad5a983242a0fe8d835857964abc43) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* tilemaps */
	ROM_LOAD( "540-a06-14n.bin", 0x000000, 0x080000, CRC(260ad79e) SHA1(fb56bf6e59e78b2bd1f8df17c9c8fd0d1700dced) )
	ROM_LOAD( "540-a07-17n.bin", 0x080000, 0x080000, CRC(442bcec2) SHA1(3100de8c146a28284ae3ab8763e5b1c6fb1755c2) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "540-a01-2f.bin", 0x000000, 0x080000, CRC(391c6ee6) SHA1(a345934687a8abf818350d0597843a1159395fc0) )
ROM_END

GAME( 1995, kzaurus,    0, kzaurus, kzaurus,  konmedal68k_state, 0, ROT0, "Konami", "Pittanko Zaurus", MACHINE_NOT_WORKING)
