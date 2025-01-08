// license:GPL-2.0+
// copyright-holders:Jonas Jago
/*******************************************************************************

Beyond Kung-Fu

Irem M62-based unreleased Kung-Fu Master sequel
video reference: https://www.youtube.com/watch?v=Efr9EQkbCSQ

TODO:
- finish background gfx emulation, supposedly via an (undumped) MCU

*******************************************************************************/

#include "emu.h"
#include "m62.h"
#include "iremipt.h"


namespace {

class m62_bkungfu_state : public m62_state
{
public:
	m62_bkungfu_state(const machine_config &mconfig, device_type type, const char *tag)
		: m62_state(mconfig, type, tag)
		, m_bkungfu_tileram(*this, "tileram", 64*32*2, ENDIANNESS_LITTLE)
		, m_blitterdatarom(*this, "blitterdat")
		, m_blittercmdram(*this, "blittercmdram")
	{ }

	void bkungfu(machine_config& config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t bkungfu_blitter_r(offs_t offset);
	void bkungfu_blitter_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_bkungfu_bg_tile_info);
	DECLARE_VIDEO_START(bkungfu);

	memory_share_creator<uint8_t> m_bkungfu_tileram;

	required_region_ptr<uint8_t> m_blitterdatarom;
	required_shared_ptr<uint8_t> m_blittercmdram;
};



/*******************************************************************************
    Video
*******************************************************************************/

TILE_GET_INFO_MEMBER(m62_bkungfu_state::get_bkungfu_bg_tile_info)
{
	int code = m_bkungfu_tileram[(tile_index << 1)];
	int color = m_bkungfu_tileram[(tile_index << 1) | 1];

	tileinfo.set(0, code | ((color & 0xe0) << 3) | (m_kidniki_background_bank << 11), color & 0x1f, 0);

	if ((tile_index / 64) < 6 || ((color & 0x1f) >> 1) > 0x0c)
		tileinfo.category = 1;
	else
		tileinfo.category = 0;
}

VIDEO_START_MEMBER(m62_bkungfu_state,bkungfu)
{
	m62_start(tilemap_get_info_delegate(*this, FUNC(m62_bkungfu_state::get_bkungfu_bg_tile_info)), 32, 0, 8, 8, 64, 32);
}



/*******************************************************************************
    Blitter
*******************************************************************************/

uint8_t m62_bkungfu_state::bkungfu_blitter_r(offs_t offset)
{
	return 0xfe;
}

void m62_bkungfu_state::bkungfu_blitter_w(offs_t offset, uint8_t data)
{
	m_blittercmdram[offset] = data;

	if (offset == 0x00)
	{
		if (data == 0x14)
		{
			//logerror("%s: blitter: draw text from ROM\n", machine().describe_context());

			uint16_t blitterromptr = m_blittercmdram[0x001] * 2;
			uint16_t blitterromdataptr = m_blitterdatarom[blitterromptr] | (m_blitterdatarom[blitterromptr + 1] << 8);

			uint8_t blitdat = m_blitterdatarom[blitterromdataptr++];
			while (blitdat != 0x00)
			{
				if (blitdat == 0x01)
				{
					// change color value during blit
					m_blittercmdram[0x004] = m_blitterdatarom[blitterromdataptr++];
				}
				else if (blitdat == 0x02)
				{
					// change position params during blit
					m_blittercmdram[0x002] = m_blitterdatarom[blitterromdataptr++];
					m_blittercmdram[0x003] = m_blitterdatarom[blitterromdataptr++];
				}
				else
				{
					uint16_t position = (m_blittercmdram[0x003] << 8) | m_blittercmdram[0x002];

					m_bkungfu_tileram[(position) & 0xfff] = blitdat;
					m_bkungfu_tileram[(position + 1) & 0xfff] = m_blittercmdram[0x004];
					m_bg_tilemap->mark_tile_dirty((position&0xfff) >> 1);

					position += 2;
					m_blittercmdram[0x002] = position & 0xff;
					m_blittercmdram[0x003] = (position & 0xff00) >> 8;
				}

				blitdat = m_blitterdatarom[blitterromdataptr++];
			}
		}
		else if (data == 0x08) // clear layer to fixed value
		{
			for (int position = 0; position < 0x1000; position += 2)
			{
				m_bkungfu_tileram[(position) & 0xfff] = m_blittercmdram[0x002];
				m_bkungfu_tileram[(position + 1) & 0xfff] = m_blittercmdram[0x001];
			}
		}
		else if (data == 0x02)
		{
			//logerror("%s: blitter: draw level from ROM(2)\n", machine().describe_context());
		}
		else if (data == 0x01)
		{
			//logerror("%s: blitter: draw level from ROM(1)\n", machine().describe_context());
		}
		else if (data == 0x0a)
		{
			// level data is in custom format. draws 14x4 tile blocks. compression?
			//logerror("%s: blitter: draw level from ROM initialize\n", machine().describe_context());

			//uint16_t blitterromptr = m_blittercmdram[0x001] << 1;
			//uint16_t blitterromdataptr = m_blitterdatarom[0x200 + blitterromptr] | (m_blitterdatarom[0x200 + blitterromptr + 1] << 8);
			//logerror("source address is % 04x\n", blitterromdataptr);
		}
		else if (data == 0x05)
		{
			//logerror("%s: blitter: draw level ROM on level 3+\n", machine().describe_context());
		}
		else if (data == 0x0f)
		{
			//logerror("%s: blitter: draw title animation flames\n", machine().describe_context());
		}
		else if (data == 0x0d)
		{
			//logerror("%s: blitter: draw title animation flames(2)\n", machine().describe_context());
		}
		else if (data == 0x10)
		{
			//logerror("%s: blitter: coin up\n", machine().describe_context());
		}
		else if (data == 0xfe)
		{
			//logerror("%s: blitter: start up\n", machine().describe_context());
		}
		else
		{
			//logerror("%s: blitter: unknown\n", machine().describe_context());
		}
		m_bg_tilemap->mark_all_dirty();
	}
	else
	{
		// higher offsets used for score & timer display
	}
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void m62_bkungfu_state::mem_map(address_map& map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc0ff).ram().share("spriteram");
	map(0xc800, 0xcfff).rw(FUNC(m62_bkungfu_state::bkungfu_blitter_r), FUNC(m62_bkungfu_state::bkungfu_blitter_w)).share("blittercmdram");
	map(0xe000, 0xefff).ram();
}

void m62_bkungfu_state::io_map(address_map &map)
{
	kungfum_io_map(map);
	map(0x80, 0x80).w(FUNC(m62_bkungfu_state::m62_hscroll_low_w));
	map(0x81, 0x81).w(FUNC(m62_bkungfu_state::m62_hscroll_high_w));
	map(0x83, 0x83).w(FUNC(m62_bkungfu_state::kidniki_background_bank_w));
	//map(0x84, 0x84).nopw();
}



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void m62_bkungfu_state::bkungfu(machine_config& config)
{
	kungfum(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &m62_bkungfu_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &m62_bkungfu_state::io_map);

	MCFG_VIDEO_START_OVERRIDE(m62_bkungfu_state,bkungfu)
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( bkungfu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "km-a.4e", 0x00000, 0x4000, CRC(083632aa) SHA1(0a52c6162b2fb55057735a54c59f7cb88d870593) )
	ROM_LOAD( "km-a.4d", 0x04000, 0x4000, CRC(08b14684) SHA1(8d60abe5f06e1b3ce465ec740df3f4ee8e9398bc) )
	ROM_LOAD( "km-z.7j", 0x08000, 0x4000, CRC(2bd2aa83) SHA1(422ef2a64f040a0974311ff692726c6f3a8f8b13) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "mcu",     0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "blitterdat", ROMREGION_ERASEFF )
	ROM_LOAD( "km-z.4h", 0x0000, 0x8000, CRC(252bb4a9) SHA1(2a69ee113950ea58895b42102bbb5263865ace9d) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "km-a.3a", 0x4000, 0x4000, CRC(bb709dd6) SHA1(aa491ec2d64b096927546b4362fa41c4784659c9) )
	ROM_LOAD( "km-a.3d", 0x8000, 0x4000, CRC(ef8551cb) SHA1(2a26feeae8ea7ddc5d899592bc4c17b40571fe8f) )
	ROM_LOAD( "km-a.3f", 0xc000, 0x4000, CRC(bec93bdc) SHA1(e287d3e689e18b192d686f313c63b6ed4e10a83f) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "km-z.3d", 0x00000, 0x8000, CRC(a8007429) SHA1(5e315ba41bbb2248cf49b1fd0a1601c08bf891b4) )
	ROM_LOAD( "km-z.3c", 0x08000, 0x8000, CRC(a99be837) SHA1(48b720462b359ef3551d50b8a41d2e3a2e146a60) )
	ROM_LOAD( "km-z.3a", 0x10000, 0x8000, CRC(0bb7fda0) SHA1(aaab12b5e5402f3bbd1a2700ecc65dc588f0e590) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "km-b.4k", 0x00000, 0x4000, CRC(7e8bec97) SHA1(6ca5939bd64df63124de997b53c9ac9975450ab5) )
	ROM_LOAD( "km-b.4f", 0x04000, 0x4000, CRC(3a75305b) SHA1(f62e7a293647be8aabbdd0b366459f634de593c6) )
	ROM_LOAD( "km-b.4l", 0x08000, 0x4000, CRC(fe746127) SHA1(53f446a28466e0a749c27aa2eeb7d7bad4bd8d9b) )
	ROM_LOAD( "km-b.4h", 0x0c000, 0x4000, CRC(f322c5dd) SHA1(03cb9658f31853f1ba41d8bf7e8cbc87353cc432) )
	ROM_LOAD( "km-b.3n", 0x10000, 0x4000, CRC(b88a4d16) SHA1(2ab45a4bf44b5827def8166b30faa08514e7a814) )
	ROM_LOAD( "km-b.4n", 0x14000, 0x4000, CRC(f92be992) SHA1(dd3ddc1ba76ceba71435a63c43f1be24a3272011) )
	ROM_LOAD( "km-b.4m", 0x18000, 0x4000, CRC(53623913) SHA1(efb3de824df15b95e5eb91b5907ae2232501e3e3) )
	ROM_LOAD( "km-b.3m", 0x1c000, 0x4000, CRC(1d1ec6f2) SHA1(fcdaf34529166701aad87d013176f4709cb5d540) )
	ROM_LOAD( "km-b.4c", 0x20000, 0x4000, CRC(31cb5b98) SHA1(54bb88d668c59ec5248098a053a99132a121df74) )
	ROM_LOAD( "km-b.4e", 0x24000, 0x4000, CRC(942e60fc) SHA1(d14553854b8300e80d7556b3be47544c537daac3) )
	ROM_LOAD( "km-b.4d", 0x28000, 0x4000, CRC(90a03502) SHA1(bf4efc5e170f8ae479411eef98d8c38bb32d2648) )
	ROM_LOAD( "km-b.4a", 0x2c000, 0x4000, CRC(018509c2) SHA1(844f3d79a4f358ccce1023deb76c052914570aed) )

	ROM_REGION( 0x300, "spr_color_proms", 0 )
	ROM_LOAD( "km-b.1m", 0x0000, 0x0100, CRC(73638418) SHA1(ad3f9cf08d334e76294bd796b7f8849014f05b8e) )
	ROM_LOAD( "km-b.1n", 0x0100, 0x0100, CRC(7967dfdf) SHA1(95f5aac75ce902287c0d0ada6ee9e1e1bd9d87c0) )
	ROM_LOAD( "km-b.1l", 0x0200, 0x0100, CRC(4f44ef5c) SHA1(094e6ab24a5663208fc9d54203ed200c9e4a9fc3) )

	ROM_REGION( 0x300, "chr_color_proms", 0 )
	ROM_LOAD( "km-z-1j", 0x0000, 0x0100, CRC(1da0ad7f) SHA1(3b3ba444efa8f5481c64b3c9ef1c0ab6561c9f16) )
	ROM_LOAD( "km-z-1h", 0x0100, 0x0100, CRC(23a217c0) SHA1(723738cd8875c5bb449be2da75ef0b04cca8dd55) )
	ROM_LOAD( "km-z-1f", 0x0200, 0x0100, CRC(ec9df4f2) SHA1(702f7d536a5f79987342521c66d703153a888010) )

	ROM_REGION( 0x20, "spr_height_prom", 0 )
	ROM_LOAD( "km-b.5p", 0x0000, 0x0020, CRC(33409e90) SHA1(b84f7df9c27aa18255099b0473c6088c6fd7adfa) )

	ROM_REGION( 0x100, "timing", 0 )
	ROM_LOAD( "km-b.6f", 0x0000, 0x0100, CRC(82c20d12) SHA1(268903f7d9be58a70d030b02bf31a2d6b5b6e249) )
ROM_END

} // anonymous namespace


/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  MACHINE  INPUT     CLASS              INIT        ROT     COMPANY  FULLNAME                          FLAGS
GAME( 1987, bkungfu, 0,      bkungfu, kungfum,  m62_bkungfu_state, empty_init, ROT0,   "Irem",  "Beyond Kung-Fu (location test)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
