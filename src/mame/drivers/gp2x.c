// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**************************************************************************
 *
 * gp2x.c - Game Park Holdings GP2X
 * Skeleton by R. Belmont
 *
 * CPU: MagicEyes MP2520F SoC
 * MP2520F consists of:
 *    ARM920T CPU core + MMU
 *    ARM940T slave CPU w/o MMU
 *    LCD controller
 *    DMA controller
 *    Interrupt controller
 *    NAND flash interface
 *    MMC/SD Card interface
 *    USB controller
 *    and more.
 *
 **************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"


class gp2x_state : public driver_device
{
public:
	gp2x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_maincpu_region(*this, "maincpu"),
			m_ram(*this, "ram"){ }

	required_device<cpu_device> m_maincpu;
	required_region_ptr<UINT32> m_maincpu_region;
	DECLARE_READ32_MEMBER(gp2x_lcdc_r);
	DECLARE_WRITE32_MEMBER(gp2x_lcdc_w);
	DECLARE_READ32_MEMBER(nand_r);
	DECLARE_WRITE32_MEMBER(nand_w);
	DECLARE_READ32_MEMBER(tx_status_r);
	DECLARE_WRITE32_MEMBER(tx_xmit_w);
	DECLARE_READ32_MEMBER(timer_r);
	DECLARE_READ32_MEMBER(nand_ctrl_r);
	DECLARE_WRITE32_MEMBER(nand_ctrl_w);
	DECLARE_READ32_MEMBER(sdcard_r);
	required_shared_ptr<UINT32> m_ram;
	UINT16 m_vidregs[0x200/2];
	UINT32 m_nand_ptr;
	UINT32 m_nand_cmd;
	UINT32 m_nand_subword_stage;
	UINT32 m_nand_stage;
	UINT32 m_nand_ptr_temp;
	UINT32 m_timer;
	UINT32 screen_update_gp2x(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};



#if 0
static const char *const gp2x_regnames[0x200] =
{
	"DPC Control",
	"PAD Control",
	"PAD Polarity Control 1",
	"PAD Polarity Control 2",
	"PAD Polarity Control 3",
	"PAD Enable Control 1",
	"PAD Enable Control 2",
	"PAD Enable Control 3",
	"",
	"",
	"",
	"Active width",
	"Active height",
	"HSYNC width",
	"HSYNC start",
	"HSYNC end",
	"VSYNC control",    // 20
	"VSYNC end",
	"",
	"DE Control",
	"S Control",
	"FG Control",
	"LP Control",
	"CLKV Control",
	"CLKV Control 2",   // 30
	"POL Control",
	"CIS Sync Control",
	"",
	"",
	"Luma blank level",
	"Chroma blank level",
	"Luma pos sync",
	"Luma neg sync",    // 40
	"Chroma pos sync",
	"Chroma neg sync",
	"Interrupt control",
	"Clock control",
	"",
	"",
	"",
	"",         // 50
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",         // 60
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",         // 70
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"Overlay control",  // 80
	"Image effect",
	"Image control",
	"Scale factor A top",
	"Scale factor A bottom",
	"Scale factor A top 2 H",
	"Scale factor A top 2 L",
	"Scale factor A bottom 2 H",
	"Scale factor A bottom 2 L",    // 90
	"Width region A top",
	"Width region A bottom",
	"H Offset region A",
	"H Offset region A 2",
	"V Start region A",
	"V End region A top",
	"V End region A bottom",
	"YUV Source region A L",    // a0
	"YUV Source region A H",
};
#endif
UINT32 gp2x_state::screen_update_gp2x(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// display enabled?
	if (m_vidregs[0] & 1)
	{
		// only support RGB still image layer for now
		if (m_vidregs[0x80/2] & 4)
		{
			int x, y;
			UINT16 *vram = (UINT16 *)&m_ram[0x2100000/4];

/*          printf("RGB still image 1 enabled, bpp %d, size is %d %d %d %d\n",
                (m_vidregs[(0xda/2)]>>9)&3,
                m_vidregs[(0xe2/2)],
                m_vidregs[(0xe4/2)],
                m_vidregs[(0xe6/2)],
                m_vidregs[(0xe8/2)]);*/


			for (y = 0; y < 240; y++)
			{
				UINT32 *scanline = &bitmap.pix32(y);

				for (x = 0; x < 320; x++)
				{
					UINT16 pixel = vram[(320*y)+x];

					*scanline++ = rgb_t(0xff, (pixel>>11)<<3, ((pixel>>5)&0x3f)<<2, (pixel&0x1f)<<3);
				}
			}
		}
	}

	return 0;
}

READ32_MEMBER( gp2x_state::gp2x_lcdc_r )
{
	return m_vidregs[offset*2] | m_vidregs[(offset*2)+1]<<16;
}

WRITE32_MEMBER( gp2x_state::gp2x_lcdc_w )
{
	if (mem_mask == 0xffff)
	{
		m_vidregs[offset*2] = data;
//      printf("%x to video reg %x (%s)\n", data, offset*2, gp2x_regnames[offset*2]);
	}
	else if (mem_mask == 0xffff0000)
	{
		m_vidregs[(offset*2)+1] = data>>16;
//      printf("%x to video reg %x (%s)\n", data>>16, (offset*2)+1, gp2x_regnames[(offset*2)+1]);
	}
	else
	{
		logerror("GP2X LCDC: ERROR: write %x to vid reg @ %x mask %08x\n", data, offset, mem_mask);
	}
}

READ32_MEMBER( gp2x_state::nand_r )
{
	UINT32 *ROM = m_maincpu_region;
	UINT32 ret;

	if (offset == 0)
	{
		switch (m_nand_cmd)
		{
			case 0:
				if (mem_mask == 0xffffffff)
				{
					return ROM[m_nand_ptr++];
				}
				else if (mem_mask == 0x000000ff)    // byte-wide reads?
				{
					switch (m_nand_subword_stage++)
					{
						case 0:
							return ROM[m_nand_ptr];
						case 1:
							return ROM[m_nand_ptr]>>8;
						case 2:
							return ROM[m_nand_ptr]>>16;
						case 3:
							ret = ROM[m_nand_ptr]>>24;
							m_nand_subword_stage = 0;
							m_nand_ptr++;
							return ret;
						default:
							logerror("Bad nand_subword_stage = %d\n", m_nand_subword_stage);
							break;
					}
				}
				break;

			case 0x50:  // read out-of-band data
				return 0xff;

			case 0x90:  // read ID
				switch (m_nand_stage++)
				{
					case 0:
						return 0xec;    // Samsung
					case 1:
						return 0x76;    // 64MB
				}
				break;

			default:
				logerror("NAND: read unk command %x (PC %x)\n", m_nand_cmd, m_maincpu->pc());
				break;
		}
	}

//  printf("Read unknown nand offset %x\n", offset);

	return 0;
}

WRITE32_MEMBER( gp2x_state::nand_w )
{
	switch (offset)
	{
		case 4: // command
			m_nand_cmd = data;
//          printf("NAND: command %x (PC %x0)\n", data, m_maincpu->pc());
			m_nand_stage = 0;
			m_nand_subword_stage = 0;
			break;

		case 6: // address
			if (m_nand_cmd == 0)
			{
				switch (m_nand_stage)
				{
					case 0:
						m_nand_ptr_temp &= ~0xff;
						m_nand_ptr_temp |= data;
						break;
					case 1:
						m_nand_ptr_temp &= ~0xff00;
						m_nand_ptr_temp |= data<<8;
						break;
					case 2:
						m_nand_ptr_temp &= ~0xff0000;
						m_nand_ptr_temp |= data<<16;
						break;
					case 3:
						m_nand_ptr_temp &= ~0xff000000;
						m_nand_ptr_temp |= data<<24;

//                      printf("NAND: ptr now %x, /2 %x, replacing %x\n", m_nand_ptr_temp, m_nand_ptr_temp/2, m_nand_ptr);
						m_nand_ptr = m_nand_ptr_temp/2;
						break;
				}
				m_nand_stage++;
			}
			break;

		default:
			logerror("nand_w: %x to unknown offset %x\n", data, offset);
			break;
	}
}

READ32_MEMBER( gp2x_state::tx_status_r )
{
	return 0x6; // tx ready, tx empty
}

WRITE32_MEMBER( gp2x_state::tx_xmit_w )
{
	printf("%c", data&0xff);
}

READ32_MEMBER( gp2x_state::timer_r )
{
	return m_timer++;
}

READ32_MEMBER( gp2x_state::nand_ctrl_r )
{
	return 0x8000<<16;      // timed out
}

WRITE32_MEMBER( gp2x_state::nand_ctrl_w )
{
//  printf("%08x to nand_ctrl_w\n", data);
}

READ32_MEMBER( gp2x_state::sdcard_r )
{
	return 0xffff<<16;  // at 3e146b0 - indicate timeout & CRC error
}

static ADDRESS_MAP_START( gp2x_map, AS_PROGRAM, 32, gp2x_state )
	AM_RANGE(0x00000000, 0x00007fff) AM_ROM
	AM_RANGE(0x01000000, 0x04ffffff) AM_RAM AM_SHARE("ram") // 64 MB of RAM
	AM_RANGE(0x9c000000, 0x9c00001f) AM_READWRITE(nand_r, nand_w)
	AM_RANGE(0xc0000a00, 0xc0000a03) AM_READ(timer_r)
	AM_RANGE(0xc0001208, 0xc000120b) AM_READ(tx_status_r)
	AM_RANGE(0xc0001210, 0xc0001213) AM_WRITE(tx_xmit_w)
	AM_RANGE(0xc0001508, 0xc000150b) AM_READ(sdcard_r)
	AM_RANGE(0xc0002800, 0xc00029ff) AM_READWRITE(gp2x_lcdc_r, gp2x_lcdc_w)
	AM_RANGE(0xc0003a38, 0xc0003a3b) AM_READWRITE(nand_ctrl_r, nand_ctrl_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( gp2x )
INPUT_PORTS_END

static MACHINE_CONFIG_START( gp2x, gp2x_state )
	MCFG_CPU_ADD("maincpu", ARM9, 80000000)
	MCFG_CPU_PROGRAM_MAP(gp2x_map)

	MCFG_PALETTE_ADD("palette", 32768)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(gp2x_state, screen_update_gp2x)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END

ROM_START(gp2x)
	ROM_REGION( 0x600000, "maincpu", 0 )    // contents of NAND flash
	ROM_SYSTEM_BIOS(0, "v2", "version 2.0")
	ROMX_LOAD( "gp2xboot.img",   0x000000, 0x05b264, CRC(9d82937e) SHA1(9655f04c11f78526b3b8a4613897070df3119ead), ROM_BIOS(1))
	ROMX_LOAD( "gp2xkernel.img", 0x080000, 0x09a899, CRC(e272eac9) SHA1(9d814ad6c27d22812ba3f101a7358698d1b035eb), ROM_BIOS(1))
	ROMX_LOAD( "gp2xsound.wav",  0x1a0000, 0x03a42c, CRC(b6ac0ade) SHA1(32362ebbcd09a3b15e164ec3fbd2a8f11159bcd7), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(1, "v3", "version 3.0")
	ROMX_LOAD( "gp2xboot.v3",    0x000000, 0x05b264, CRC(ab1fc556) SHA1(2ce66fec325b6e8e29f540322aefb435d8e3baf0), ROM_BIOS(2))
	ROMX_LOAD( "gp2xkernel.v3",  0x080000, 0x09f047, CRC(6f8922ae) SHA1(0f44204757f9251c7d68560ff31a1ce72f16698e), ROM_BIOS(2))
	ROMX_LOAD( "gp2xsound.wav",  0x1a0000, 0x03a42c, CRC(b6ac0ade) SHA1(32362ebbcd09a3b15e164ec3fbd2a8f11159bcd7), ROM_BIOS(2))
	ROMX_LOAD( "gp2xyaffs.v3",   0x300000, 0x2ec4d0, CRC(f81a4a57) SHA1(73bb84798ad3a4e281612a47abb2da4772cbe6cd), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(2, "v4", "version 4.0")
	ROMX_LOAD( "gp2xboot.v4",    0x000000, 0x05b264, CRC(160af379) SHA1(0940fc975960fa52f32a9258a9480cc5cb1140e2), ROM_BIOS(3))
	ROMX_LOAD( "gp2xkernel.v4",  0x080000, 0x0a43a8, CRC(77b1cf9c) SHA1(7e759e4581399bfbee982e1b6c3b54a10b0e9c3d), ROM_BIOS(3))
	ROMX_LOAD( "gp2xsound.wav",  0x1a0000, 0x03a42c, CRC(b6ac0ade) SHA1(32362ebbcd09a3b15e164ec3fbd2a8f11159bcd7), ROM_BIOS(3))
	ROMX_LOAD( "gp2xyaffs.v4",   0x300000, 0x2dfed0, CRC(e77efc53) SHA1(21477ff77aacb84005bc465a03066d71031a6098), ROM_BIOS(3))
ROM_END

CONS(2005, gp2x, 0, 0, gp2x, gp2x, driver_device, 0, "Game Park Holdings", "GP2X", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
