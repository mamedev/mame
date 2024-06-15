// license:BSD-3-Clause
// copyright-holders:David Haywood, XingXing

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/nvram.h"
#include "pgmcrypt.h"
#include "sound/ics2115.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class igs_fear_state : public driver_device
{
public:
	igs_fear_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_palette(*this, "palette"),
		m_gfxrom(*this, "gfx1")
	{ }

	void igs_fear(machine_config &config);

	void init_igs_fear();
	void init_igs_superkds();

protected:
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint32_t> m_videoram;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_gfxrom;

	void main_map(address_map &map);

	void sound_irq(int state);
	void vblank_irq(int state);

	void draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int xpos, int ypos, int height, int width, int palette, int romoffset);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


void igs_fear_state::video_start()
{
}

void igs_fear_state::draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int xpos, int ypos, int height, int width, int palette, int romoffset)
{
	if ((romoffset != 0) && (romoffset != 0xffffffff))
	{
		//logerror("x=%d, y=%d, w=%d pix, h=%d pix, c=0x%02x, romoffset=0x%08x\n", xpos, ypos, width, height, palette, romoffset << 2);
		uint8_t* gfxrom = &m_gfxrom[romoffset << 2];
		palette = (palette & 0x3f) << 7;

		for (int y = 0; y < height; y++)
		{
			uint16_t* dest = &bitmap.pix(ypos + y);
			for (int x = 0; x < width; x++)
			{
				uint8_t pix = *gfxrom++;
				if (pix)
				{
					if (cliprect.contains(xpos + x, ypos + y))
						dest[xpos + x] = pix | palette;
				}
			}

		}
	}
}

uint32_t igs_fear_state::screen_update(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	bitmap.fill(0x3ff, cliprect);

	for (int i = 0; i < 0x2000 / 0x10; i++)
	{
		const int xpos    = (m_videoram[(i * 4) + 0] & 0x0000ffff) >> 0;
		const int ypos    = (m_videoram[(i * 4) + 0] & 0xffff0000) >> 16;
		int       height  = (m_videoram[(i * 4) + 1] & 0x0000ffff) >> 0;
		int       width   = (m_videoram[(i * 4) + 1] & 0xffff0000) >> 16;
		const int palette = (m_videoram[(i * 4) + 2] & 0x0000ffff) >> 0;
		const int rom_msb = (m_videoram[(i * 4) + 2] & 0xffff0000) >> 16;
		const int rom_lsb = (m_videoram[(i * 4) + 3] & 0x0000ffff) >> 0;

		const int romoffset = rom_msb + (rom_lsb << 16);

		width = (width + 1) << 2;
		height = height + 1;

		draw_sprite(bitmap, cliprect, xpos, ypos, height, width, palette, romoffset);
	}

	return 0;
}

void igs_fear_state::main_map(address_map &map)
{
	map(0x00000000, 0x00003fff).rom(); /* Internal ROM */
	map(0x08000000, 0x0807ffff).rom().region("user1", 0);/* Game ROM */
	map(0x10000000, 0x100003ff).ram().share("iram");
	map(0x18000000, 0x1800ffff).ram().share("sram");
	map(0x50000000, 0x500003ff).ram().share("xortab");
	map(0x38000000, 0x38001fff).ram().share("videoram");
	map(0x38004000, 0x38007fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
}

static INPUT_PORTS_START( fear )
INPUT_PORTS_END

void igs_fear_state::sound_irq(int state)
{
}

void igs_fear_state::vblank_irq(int state)
{
	if (state)
		m_maincpu->pulse_input_line(ARM7_FIRQ_LINE, m_maincpu->minimum_quantum_time());
}

void igs_fear_state::igs_fear(machine_config &config)
{
	ARM7(config, m_maincpu, 50000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_fear_state::main_map);

	// MX10EXAQC (Philips 80C51 XA)

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(igs_fear_state::screen_update));
	screen.screen_vblank().set(FUNC(igs_fear_state::vblank_irq));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xGBR_555, 0x4000/2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ics2115_device &ics(ICS2115(config, "ics", 33.8688_MHz_XTAL)); // TODO : Correct?
	ics.irq().set(FUNC(igs_fear_state::sound_irq));
	ics.add_route(ALL_OUTPUTS, "mono", 5.0);
}


ROM_START( fearless )
	ROM_REGION( 0x04000, "maincpu", 0 ) // Internal rom of IGS027A ARM based MCU
	// this is taken from superkds, the XOR table required (that is uploaded by this internal ROM) is the same as superkds at least, but actual internal ROM might not be identical
	ROM_LOAD( "fearless_igs027a.bin", 0x00000, 0x4000, BAD_DUMP CRC(9a8e790d) SHA1(ab020a04a4ed0c0e5ec8c979f206fe57572d2304) ) // sticker marked 'F1'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "fearlessp_v-101us.u37", 0x000000, 0x80000, CRC(2522873c) SHA1(8db709877311b6d2796353fc9a44a820937e35c2) )

	ROM_REGION( 0x10000, "plcc", 0 ) // MX10EXAQC (80C51 XA based MCU) marked 07, not read protected
	ROM_LOAD( "fearlessp_07.u33", 0x000000, 0x10000, CRC(7dae4900) SHA1(bbf7ba7c9e95ff2ffeb1dc0fc7ccedd4da274d01) )

	ROM_REGION( 0x3000000, "gfx1", 0 ) // FIXED BITS (0xxxxxxx) (graphics are 7bpp)
	ROM_LOAD32_WORD( "fearlessp_u7_cg-0l.u7",   0x0000000, 0x800000, CRC(ca254db4) SHA1(f5670c2ff0720c84c9aff3cea95b118b6044e469) )
	ROM_LOAD32_WORD( "fearlessp_u6_cg-0h.u6",   0x0000002, 0x800000, CRC(02d8bbbf) SHA1(7cf36c909a5d76096a725ffe0a697bcbafbcf985) )
	ROM_LOAD32_WORD( "fearlessp_u14_cg-1l.u14", 0x1000000, 0x800000, CRC(7fe312d2) SHA1(c0add22d9fc4c0e32a03922cb709b947bfff429d) )
	ROM_LOAD32_WORD( "fearlessp_u13_cg-1h.u13", 0x1000002, 0x800000, CRC(c9d2a56d) SHA1(91d1665623bca743f68f15a27bbf433e2ffc0808) )
	ROM_LOAD32_WORD( "fearlessp_u18_cg-2l.u18", 0x2000000, 0x800000, CRC(07623d66) SHA1(041d5e44917bc16caa720ea98bdc0a4f5fb4b8e0) )
	ROM_LOAD32_WORD( "fearlessp_u17_cg-2h.u17", 0x2000002, 0x800000, CRC(756fe1f2) SHA1(48ee81c5fa4808406b57b2521b836db3ff5a7fa9) )

	ROM_REGION( 0x800000, "ics", 0 )
	ROM_LOAD( "fearlessp_u25_music0.u25", 0x000000, 0x400000, CRC(a015b9b1) SHA1(7b129c59acd523dec82e58a75d873bbc5341fb28) )
	ROM_LOAD( "fearlessp_u26_music1.u26", 0x400000, 0x400000, CRC(9d5f18da) SHA1(42e5224c1af0898cc2e02b2e051ea8b629d5fb6d) )
ROM_END

ROM_START( superkds )
	ROM_REGION( 0x04000, "maincpu", 0 ) // Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "superkids_igs027a.bin", 0x00000, 0x4000, CRC(9a8e790d) SHA1(ab020a04a4ed0c0e5ec8c979f206fe57572d2304) ) // sticker marked 'F5'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "superkids_s019cn.u37", 0x000000, 0x80000, CRC(1a7f17dd) SHA1(ba20c0f521bff2f5ae2103ea49bd413b0e6459ba) )

	ROM_REGION( 0x10000, "plcc", 0 ) // MX10EXAQC (80C51 XA based MCU) marked 07, not read protected
	ROM_LOAD( "superkids_mx10exa.u33", 0x000000, 0x10000, CRC(bb4881e4) SHA1(7b78c5a1b9a061f6e27adbe696d42cc92d2e3df2) ) // sticker marked G6

	ROM_REGION( 0x2000000, "gfx1", 0 ) // FIXED BITS (0xxxxxxx) (graphics are 7bpp)
	ROM_LOAD32_WORD( "superkids_cg-0l.u7",  0x0000000, 0x800000, CRC(6c4eff23) SHA1(1089222bfedcd562f2a3c43c90512bbb9e22a8bf) )
	ROM_LOAD32_WORD( "superkids_cg-0h.u6",  0x0000002, 0x800000, CRC(b797b0b6) SHA1(4298564d963fdaeca94cc0e739fbcae401168949) )
	ROM_LOAD32_WORD( "superkids_cg-1l.u14", 0x1000000, 0x800000, CRC(57081c96) SHA1(886ac14ad1c9ce8c7a67bbfc6c00e7c75be634dc) )
	ROM_LOAD32_WORD( "superkids_cg-1h.u13", 0x1000002, 0x800000, CRC(cd1e41ef) SHA1(a40bcbd97fa3e742e8f9c7b7c7d8879175bf10ee) )

	ROM_REGION( 0x800000, "ics", 0 )
	ROM_LOAD( "superkids_music0.u25", 0x000000, 0x400000, CRC(d7c37216) SHA1(ffcf7f1bf3093eb34ad0ae2cc89062de45b9d420) )
	ROM_LOAD( "superkids_music1.u26", 0x400000, 0x400000, CRC(5f080dbf) SHA1(f02330db3336f6606aae9f5a9eca819701caa3bf) )
ROM_END

void igs_fear_state::init_igs_fear()
{
	/*
	bp 8002A3C,1,{r0=1;g;}
	*/
	fearless_decrypt(machine());
}
void igs_fear_state::init_igs_superkds()
{
	/*
	bp 5b4,1,{r0=0;g;}
	bp 8001EAC,1,{r0=1;g;}
	bp 8002450,1,{r3=0;g;}
	bp 964,1,{r0=0;g;}
	bp 698,1,{r0=0;g;}
	*/
	superkds_decrypt(machine());
}

} // anonymous namespace

GAME( 2005, superkds, 0, igs_fear, fear, igs_fear_state, init_igs_superkds, ROT0, "IGS", "Super Kids (V019CN)",           MACHINE_IS_SKELETON )
GAME( 2006, fearless, 0, igs_fear, fear, igs_fear_state, init_igs_fear,     ROT0, "IGS", "Fearless Pinocchio (V101US)",   MACHINE_IS_SKELETON )
