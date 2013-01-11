/*
   Taito Power-JC System

   Skeleton driver. Requires TLCS-900 CPU core to make progress.

   Hardware appears sufficiently different to JC system to warrant
   a separate driver.

   PCB Information (incomplete!)
   ===============

   POWER JC MOTHER-G PCB
   K11X0870A
   OPERATION TIGER

   PowerPC 603E
   CXD1176Q

   TMS320C53PQ80
   40MHz osc
   43256 x 2
   E63-03_H.29 (AT27C512 PLCC)
   E63-04_L.28 (AT27C512 PLCC)

   E63-01 PALCE16V8H
   E63-02 PALCE22V10H

   IC41 E63-06 PALCE16V8H
   IC43 E63-07 PALCE16V8H

   uPD4218160 x 2
   uPD4218160 x 2

   uPD482445 x 4

   CY78991
   IS61LV256AH x 3
   Taito TC0780FPA x 2
   Taito TCG010PJC

   MN1020819
   ZOOM ZSG-2
   ZOOM ZFX-2
   MSM514256


   Second PCB
   ----------

   19 ROMs

   TMP95C063F
   25.0000MHz osc
   1.84320MHz osc
*/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/tlcs900/tlcs900.h"

static UINT32 jc_char_ram[0x2000];
static UINT32 jc_tile_ram[0x4000];
//static UINT32 jc_pal_ram[0x8000];
static UINT32 jc_screen_ram[0x10000];

static UINT8 common_ram[0x2000];

class taitopjc_state : public driver_device
{
public:
	taitopjc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }
	DECLARE_READ64_MEMBER(video_r);
	DECLARE_WRITE64_MEMBER(video_w);
	DECLARE_READ64_MEMBER(ppc_common_r);
	DECLARE_READ64_MEMBER(dsp_r);
	DECLARE_WRITE64_MEMBER(dsp_w);
	DECLARE_READ8_MEMBER(tlcs_common_r);
	DECLARE_WRITE8_MEMBER(tlcs_common_w);
	DECLARE_READ8_MEMBER(tlcs_sound_r);
	DECLARE_WRITE8_MEMBER(tlcs_sound_w);
	DECLARE_WRITE8_MEMBER(taitopjc_tlcs900_to1);
	DECLARE_WRITE8_MEMBER(taitopjc_tlcs900_to3);
	virtual void video_start();
	UINT32 screen_update_taitopjc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

void taitopjc_state::video_start()
{

}

UINT32 taitopjc_state::screen_update_taitopjc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	{
		UINT8 *s = (UINT8*)jc_char_ram;
		int x,y,t,u;
		for (u=0; u < 48; u++)
		{
			for (t=0; t < 32; t++)
			{
				UINT32 tile = jc_tile_ram[((u*16)+(t/2)) ^ 0];

				if (t & 1)
					tile &= 0xffff;
				else
					tile >>= 16;

				tile &= 0xff;
				tile -= 0x40;

				if (tile > 127) tile = 127;

				for (y=0; y < 16; y++)
				{
					UINT32 *fb = &bitmap.pix32(y+(u*16));
					for (x=0; x < 16; x++)
					{
						UINT8 p = s[((tile*256) + ((y*16)+x)) ^3];
						fb[x+(t*16)] = p ? 0xffffffff : 0;
					}
				}
			}
		}
	}

	return 0;
}

static UINT32 video_address;

static UINT32 videochip_r(address_space &space, offs_t address)
{
	UINT32 r = 0;

	if (address >= 0x10000000 && address < 0x10010000)
	{
		r = jc_screen_ram[address - 0x10000000];
	}

	return r;
}

static void videochip_w(address_space &space, offs_t address, UINT32 data)
{
	if (address >= 0x20000000 && address < 0x20008000)
	{
		//UINT32 r = (data >> 16) & 0xff;
		//UINT32 g = (data >> 8) & 0xff;
		//UINT32 b = (data >> 0) & 0xff;
		//palette_set_color_rgb(space.machine, address & 0x7fff, r, g, b);
	}
	else if (address >= 0x1003d000 && address < 0x1003f000)
	{
		jc_char_ram[address - 0x1003d000] = data;
	}
	else if (address >= 0x1003f000 && address < 0x10040000)
	{
		jc_tile_ram[address - 0x1003f000] = data;
	}
	else if (address >= 0x10000000 && address < 0x10010000)
	{
		jc_screen_ram[address - 0x10000000] = data;
	}
	else
	{
		printf("Address %08X = %08X\n", address, data);
	}
}

READ64_MEMBER(taitopjc_state::video_r)
{
	UINT64 r = 0;

	if (offset == 0)
	{
		if (ACCESSING_BITS_32_63)
		{
			r |= (UINT64)(videochip_r(space, video_address)) << 32;
		}
	}

	return r;
}

WRITE64_MEMBER(taitopjc_state::video_w)
{
	if (offset == 0)
	{
		if (ACCESSING_BITS_32_63)
		{
			//printf("Address %08X = %08X\n", video_address, (UINT32)(data >> 32));
			videochip_w(space, video_address, (UINT32)(data >> 32));
		}
	}
	if (offset == 1)
	{
		if (ACCESSING_BITS_32_63)
		{
			video_address = (UINT32)(data >> 32);
		}
	}
}

/*
static UINT16 com_ram[256] =
{
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x434F, 0x4D4E, 0x4F4B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     // COMNOK
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x574F, 0x524B, 0x4F4B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     // WORKOK
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x5355, 0x4E44, 0x4F4B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     // SUNDOK
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};
*/

READ64_MEMBER(taitopjc_state::ppc_common_r)
{
	UINT64 r = 0;
	UINT32 address;

	logerror("com_r: %08X, %08X%08X\n", offset, (UINT32)(mem_mask >> 32), (UINT32)(mem_mask));

	address = offset * 4;

	if (ACCESSING_BITS_48_63)
	{
		r |= (UINT64)(common_ram[address]) << 48;
		r |= (UINT64)(common_ram[address+1]) << 56;
	}
	if (ACCESSING_BITS_16_31)
	{
		r |= (UINT64)(common_ram[address+2]) << 16;
		r |= (UINT64)(common_ram[address+3]) << 24;
	}

	/*
	if (offset < 0x7f)
	{
	    int reg = (offset & 0x7f) * 2;

	    if (!(mem_mask & U64(0xffff000000000000)))
	    {
	        r |= (UINT64)(com_ram[reg + 0]) << 48;
	    }
	    if (!(mem_mask & U64(0x00000000ffff0000)))
	    {
	        r |= (UINT64)(com_ram[reg + 1]) << 16;
	    }
	}
	*/
	return r;
}

static UINT32 dsp_value = 0x4f4b0000;

static UINT16 dsp_ram[0x1000];

READ64_MEMBER(taitopjc_state::dsp_r)
{
	UINT64 r = 0;

	if (offset == 0x7fe)
	{
		if (!(mem_mask & U64(0xffffffff00000000)))
		{
			r |= (UINT64)(dsp_value) << 32;
		}
	}

	return r;
}

WRITE64_MEMBER(taitopjc_state::dsp_w)
{
	logerror("dsp_w: %08X, %08X%08X, %08X%08X\n", offset, (UINT32)(data >> 32), (UINT32)(data), (UINT32)(mem_mask >> 32), (UINT32)(mem_mask));

	if (offset == 0x7fe)
	{
		if (!(mem_mask & U64(0xffffffff00000000)))
		{
			dsp_value = data >> 32;
		}

		#if 0
		{
			int i;
			FILE *f = fopen("dspram.bin", "wb");
			for (i=0; i < 0x1000; i++)
			{
				fputc((dsp_ram[i] >> 0) & 0xff, f);
				fputc((dsp_ram[i] >> 8) & 0xff, f);
			}
			fclose(f);
		}
		#endif
	}

	if (!(mem_mask & U64(0xffff000000000000)))
	{
		int addr = offset * 2;
		dsp_ram[addr+0] = (data >> 48) & 0xffff;
	}
	if (!(mem_mask & U64(0x00000000ffff0000)))
	{
		int addr = offset * 2;
		dsp_ram[addr+1] = (data >> 16) & 0xffff;
	}
}

// BAT Config:
// IBAT0 U: 0x40000002   L: 0x40000022      (0x40000000...0x4001ffff)
// IBAT1 U: 0x0000007f   L: 0x00000002      (0x00000000...0x003fffff)
// IBAT2 U: 0xc0000003   L: 0xc0000022      (0xc0000000...0xc001ffff)
// IBAT3 U: 0xfe0003ff   L: 0xfe000022      (0xfe000000...0xffffffff)
// DBAT0 U: 0x40000002   L: 0x40000022      (0x40000000...0x4001ffff)
// DBAT1 U: 0x0000007f   L: 0x00000002      (0x00000000...0x003fffff)
// DBAT2 U: 0xc0000003   L: 0xc0000022      (0xc0000000...0xc001ffff)
// DBAT3 U: 0xfe0003ff   L: 0xfe000022      (0xfe000000...0xffffffff)

static ADDRESS_MAP_START( ppc603e_mem, AS_PROGRAM, 64, taitopjc_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM // Work RAM
	AM_RANGE(0x40000000, 0x4000000f) AM_READWRITE(video_r, video_w)
	AM_RANGE(0x80000000, 0x80003fff) AM_READWRITE(dsp_r, dsp_w)
	AM_RANGE(0xc0000000, 0xc000ffff) AM_READ(ppc_common_r)
	AM_RANGE(0xff000000, 0xff01ffff) AM_ROM AM_REGION("user2", 0)
	AM_RANGE(0xffe00000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END




READ8_MEMBER(taitopjc_state::tlcs_common_r)
{
	return common_ram[offset];
}

WRITE8_MEMBER(taitopjc_state::tlcs_common_w)
{
//  printf("tlcs_common_w: %08X, %02X\n", offset, data);

	common_ram[offset] = data;
}

READ8_MEMBER(taitopjc_state::tlcs_sound_r)
{
	if (offset == 0x17)
	{
		return 0x55;
	}

	return 0;
}

WRITE8_MEMBER(taitopjc_state::tlcs_sound_w)
{
//  printf("tlcs_sound_w: %08X, %02X\n", offset, data);
}

// TLCS900 interrupt vectors
// 0xfc0100: reset
// 0xfc00ea: INT0 (dummy)
// 0xfc00eb: INT1
// 0xfc00f0: INT2
// 0xfc00f5: INT3 (dummy)
// 0xfc00f6: INT4 (dummy)
// 0xfc00f7: INT5 (dummy)
// 0xfc00f8: INT6
// 0xfc00fd: INT7 (dummy)
// 0xfc00fe: INT8 (dummy)
// 0xfc0663: INTT1
// 0xfc0f7d: INTRX0
// 0xfc0f05: INTTX0
// 0xfc0fb5: INTRX1
// 0xfc0f41: INTTX1

static ADDRESS_MAP_START( tlcs900h_mem, AS_PROGRAM, 8, taitopjc_state )
	AM_RANGE(0x010000, 0x02ffff) AM_RAM     // Work RAM
	AM_RANGE(0x040000, 0x0400ff) AM_READWRITE(tlcs_sound_r, tlcs_sound_w)
	AM_RANGE(0x060000, 0x061fff) AM_READWRITE(tlcs_common_r, tlcs_common_w)
	AM_RANGE(0xfc0000, 0xffffff) AM_ROM AM_REGION("io_cpu", 0)
ADDRESS_MAP_END



WRITE8_MEMBER(taitopjc_state::taitopjc_tlcs900_to1)
{
}

WRITE8_MEMBER(taitopjc_state::taitopjc_tlcs900_to3)
{
}


static INPUT_PORTS_START( taitopjc )
INPUT_PORTS_END


static const powerpc_config ppc603e_config =
{
	XTAL_66_6667MHz,        /* Multiplier 1.5, Bus = 66MHz, Core = 100MHz */
	NULL,
	NULL
};

static const tlcs900_interface taitopjc_tlcs900_interface =
{
	DEVCB_DRIVER_MEMBER(taitopjc_state,taitopjc_tlcs900_to1 ),
	DEVCB_DRIVER_MEMBER(taitopjc_state,taitopjc_tlcs900_to3 )
};

static MACHINE_CONFIG_START( taitopjc, taitopjc_state )
	MCFG_CPU_ADD("maincpu", PPC603E, 100000000)
	MCFG_CPU_CONFIG(ppc603e_config)
	MCFG_CPU_PROGRAM_MAP(ppc603e_mem)

	/* TMP95C063F I/O CPU */
	MCFG_CPU_ADD("iocpu", TLCS900H, 25000000)
	MCFG_CPU_CONFIG(taitopjc_tlcs900_interface)
	MCFG_CPU_PROGRAM_MAP(tlcs900h_mem)

	/* TMS320C53 DSP */
	/* MN1020819DA sound CPU - NOTE: May have 64kB internal ROM */

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(640, 768)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 767)
	MCFG_SCREEN_UPDATE_DRIVER(taitopjc_state, screen_update_taitopjc)

MACHINE_CONFIG_END


ROM_START( optiger )
	ROM_REGION64_BE( 0x200000, "user1", 0 )
	ROM_LOAD32_BYTE( "e63-33-1_p-hh.23", 0x000000, 0x080000, CRC(5ab176e2) SHA1(a0a5b7c0e91928d0a49987f88f6ae647f5cb3e34) )
	ROM_LOAD32_BYTE( "e63-32-1_p-hl.22", 0x000001, 0x080000, CRC(cca8bacc) SHA1(e5a081f5c12a52601745f5b67fe3412033581b00) )
	ROM_LOAD32_BYTE( "e63-31-1_p-lh.8",  0x000002, 0x080000, CRC(ad69e649) SHA1(9fc853d2cb6e7cac87dc06bad91048f191b799c5) )
	ROM_LOAD32_BYTE( "e63-30-1_p-ll.7",  0x000003, 0x080000, CRC(a6183479) SHA1(e556c3edf100342079e680ec666f018fca7a82b0) )

	ROM_REGION64_BE( 0x20000, "user2", 0 )
	/* More PowerPC code? */
	ROM_LOAD16_BYTE( "e63-04_l.29",  0x000001, 0x010000, CRC(eccae391) SHA1(e5293c16342cace54dc4b6dfb827558e18ac25a4) )
	ROM_LOAD16_BYTE( "e63-03_h.28",  0x000000, 0x010000, CRC(58fce52f) SHA1(1e3d9ee034b25e658ca45a8b900de2aa54b00135) )

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e63-28-1_0.59", 0x000000, 0x020000, CRC(ef41ffaf) SHA1(419621f354f548180d37961b861304c469e43a65) )
	ROM_LOAD16_BYTE( "e63-27-1_1.58", 0x000001, 0x020000, CRC(facc17a7) SHA1(40d69840cfcfe5a509d69824c2994de56a3c6ece) )

	ROM_REGION( 0x80000, "unk1", 0 )
	ROM_LOAD16_BYTE( "e63-17-1_s-l.18", 0x000000, 0x040000, CRC(2a063d5b) SHA1(a2b2fe4d8bad1aef7d9dcc0be607cc4e5bc4f0eb) )
	ROM_LOAD16_BYTE( "e63-18-1_s-h.19", 0x000001, 0x040000, CRC(2f590881) SHA1(7fb827a676f45b24380558b0068b76cb858314f6) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD32_WORD_SWAP( "e63-21_c-h.24", 0x000000, 0x400000, CRC(c818b211) SHA1(dce07bfe71a9ba11c3f028a640226c6e59c6aece) )
	ROM_LOAD32_WORD_SWAP( "e63-15_c-l.9",  0x000002, 0x400000, CRC(4ec6a2d7) SHA1(2ee6270cff7ea2459121961a29d42e000cee2921) )
	ROM_LOAD32_WORD_SWAP( "e63-22_m-h.25", 0x800000, 0x400000, CRC(6d895eb6) SHA1(473795da42fd29841a926f18a93e5992f4feb27c) )
	ROM_LOAD32_WORD_SWAP( "e63-16_m-l.10", 0x800002, 0x400000, CRC(d39c1e34) SHA1(6db0ce2251841db3518a9bd9c4520c3c666d19a0) )

	ROM_REGION( 0xc00000, "poly", 0 )
	ROM_LOAD( "e63-09_poly0.3", 0x000000, 0x400000, CRC(c3e2b1e0) SHA1(ee71f3f59b46e26dbe2ff724da2c509267c8bf2f) )
	ROM_LOAD( "e63-10_poly1.4", 0x400000, 0x400000, CRC(f4a56390) SHA1(fc3c51a7f4639479e66ad50dcc94255d94803c97) )
	ROM_LOAD( "e63-11_poly2.5", 0x800000, 0x400000, CRC(2293d9f8) SHA1(16adaa0523168ee63a7a34b29622c623558fdd82) )
// Poly 3 is not populated

	ROM_REGION( 0x800000, "sound_data", 0 )
	ROM_LOAD( "e63-23_wd0.36", 0x000000, 0x200000, CRC(d69e196e) SHA1(f738bb9e1330f6dabb5e0f0378a1a8eb48a4fa40) )
	ROM_LOAD( "e63-24_wd1.37", 0x200000, 0x200000, CRC(cd55f17b) SHA1(08f847ef2fd592dbaf63ef9e370cdf1f42012f74) )
	ROM_LOAD( "e63-25_wd2.38", 0x400000, 0x200000, CRC(bd35bdac) SHA1(5cde6c1a6b74659507b31fcb88257e65f230bfe2) )
	ROM_LOAD( "e63-26_wd3.39", 0x600000, 0x200000, CRC(346bd413) SHA1(0f6081d22db88eef08180278e7ae97283b5e8452) )

	ROM_REGION( 0x500, "plds", ROMREGION_ERASEFF )
	// TODO: There are 6 PALs in total on the main PCB.
ROM_END

GAME( 1998, optiger, 0, taitopjc, taitopjc, driver_device, 0, ROT0, "Taito", "Operation Tiger", GAME_NOT_WORKING | GAME_NO_SOUND )
