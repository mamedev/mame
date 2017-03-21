// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Play Mechanix / Right Hand Tech "VP100" and "VP101" platforms
    (PCBs are also marked "Raw Thrills" but all RT games appear to be on PC hardware)

    Preliminary driver by R. Belmont

    MIPS VR5500 at 300 to 400 MHz
    Xilinx Virtex-II FPGA with custom 3D hardware and 1 or 2 PowerPC 405 CPU cores
    AC97 audio with custom DMA frontend which streams 8 stereo channels
    PIC18c442 protection chip (not readable) on VP101 only (VP100 is unprotected?)

	1 MB of VRAM at main RAM offset 0x07400000

****************************************************************************/


#include "emu.h"
#include "cpu/mips/mips3.h"
#include "machine/ataintf.h"
#include "machine/nvram.h"
#include "imagedev/harddriv.h"
#include "screen.h"

class vp10x_state : public driver_device
{
public:
	vp10x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_mainram(*this, "mainram"),
			m_ata(*this, "ata")
	{ }

	virtual void machine_reset() override;
	virtual void machine_start() override;
	
	DECLARE_READ32_MEMBER(tty_ready_r);
	DECLARE_WRITE32_MEMBER(tty_w);
	DECLARE_READ32_MEMBER(test_r) { return 0xffffffff; }
	
	DECLARE_READ32_MEMBER(pic_r);
	DECLARE_WRITE32_MEMBER(pic_w);
	
	DECLARE_WRITE32_MEMBER(dmaaddr_w);
	
	DECLARE_WRITE_LINE_MEMBER(dmarq_w);
	
	
		
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:

	// devices
	required_device<mips3_device> m_maincpu;
	required_shared_ptr<uint32_t> m_mainram;
	required_device<ata_interface_device> m_ata;

	// driver_device overrides
	virtual void video_start() override;
	int pic_cmd;
	int pic_state;
	int m_dmarq_state;
	uint32_t m_dma_ptr;
};

void vp10x_state::machine_reset()
{
	m_dmarq_state = 0;
	pic_cmd = pic_state = 0;
}

void vp10x_state::machine_start()
{
	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS);
	m_maincpu->add_fastram(0x00000000, 0x07ffffff, false, m_mainram);
}
	
WRITE32_MEMBER(vp10x_state::dmaaddr_w)
{
	m_dma_ptr = (data & 0x07ffffff);
}

WRITE_LINE_MEMBER(vp10x_state::dmarq_w)
{
	if (state != m_dmarq_state)
	{
		m_dmarq_state = state;
	
		if (state)
		{
			uint16_t *RAMbase = (uint16_t *)&m_mainram[0];
			uint16_t *RAM = &RAMbase[m_dma_ptr>>1];				

			m_ata->write_dmack(ASSERT_LINE);			

			while (m_dmarq_state)
			{
				*RAM++ = m_ata->read_dma();
				m_dma_ptr += 2;	// pointer must advance
			}

			m_ata->write_dmack(CLEAR_LINE);
		}
	}
}

READ32_MEMBER(vp10x_state::pic_r) 
{
	static const uint8_t vers[5] = { 0x00, 0x01, 0x00, 0x00, 0x00 };
	static const uint8_t serial[10] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a };
	static const uint8_t magic[10] = { 0xaa, 0x55, 0x18, 0x18, 0xc0, 0x03, 0xf0, 0x0f, 0x09, 0x0a };

	switch (pic_cmd)
	{
		case 0x20:
			return vers[pic_state++];
			
		case 0x21:
		case 0x22:
			return serial[pic_state++];
		
		case 0x23:	// this is the same for jnero and specfrce.  great security!
			return magic[pic_state++];
	}
	
	return 0;
}

WRITE32_MEMBER(vp10x_state::pic_w)
{
	//printf("%02x to pic_cmd\n", data&0xff);
	if ((data & 0xff) == 0)
	{
		return;
	}
	pic_cmd = data & 0xff;
	pic_state = 0;
}


void vp10x_state::video_start()
{
}

uint32_t vp10x_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const uint32_t *video_ram;
	uint32_t word;
	uint32_t *line;
	int y, x;

	for (y = 0; y < 240; y++)
	{
		line = &bitmap.pix32(y);
		video_ram = (const uint32_t *) &m_mainram[(0x7400000/4) + (y * (0x1000/4)) + 4];

		for (x = 0; x < 320; x++)
		{
			word = *(video_ram++);
			video_ram++;
			*line++ = word;
		}
	}
	
	return 0;
}

READ32_MEMBER(vp10x_state::tty_ready_r)
{
	return 0x60;    // must return &0x20 for output at tty_w to continue
}

WRITE32_MEMBER(vp10x_state::tty_w)  // set breakpoint at bfc01430 to catch when it's printing things
{
// uncomment to see startup messages - it says "RAM OK" and "EPI RSS Ver 4.5.1" followed by "<RSS active>" and then lots of dots
// Special Forces also says "<inited tv_cap> = 00000032"
	printf("%c", data);
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 32, vp10x_state )
	AM_RANGE(0x00000000, 0x07ffffff) AM_RAM AM_SHARE("mainram")            // this is a sufficient amount to get "RAM OK"
	AM_RANGE(0x14000000, 0x14000003) AM_READ(test_r)
	AM_RANGE(0x1c000000, 0x1c000003) AM_WRITE(tty_w)        // RSS OS code uses this one
	AM_RANGE(0x1c000014, 0x1c000017) AM_READ(tty_ready_r)
	AM_RANGE(0x1c400000, 0x1c400003) AM_WRITE(tty_w)        // boot ROM code uses this one
	AM_RANGE(0x1c400014, 0x1c400017) AM_READ(tty_ready_r)
	AM_RANGE(0x1ca0000c, 0x1ca0000f) AM_READ_PORT("IN0")
	AM_RANGE(0x1ca00010, 0x1ca00013) AM_READ(test_r)		// bits here cause various test mode stuff
	AM_RANGE(0x1cf00000, 0x1cf00003) AM_NOP AM_READNOP
	AM_RANGE(0x1d000030, 0x1d000033) AM_WRITE(dmaaddr_w)	// ATA DMA destination address
	AM_RANGE(0x1d000040, 0x1d00005f) AM_DEVREADWRITE16("ata", ata_interface_device, read_cs0, write_cs0, 0x0000ffff)
	AM_RANGE(0x1d000060, 0x1d00007f) AM_DEVREADWRITE16("ata", ata_interface_device, read_cs1, write_cs1, 0x0000ffff)
	AM_RANGE(0x1f200000, 0x1f200003) AM_READWRITE(pic_r, pic_w)
	AM_RANGE(0x1f807000, 0x1f807fff) AM_RAM	AM_SHARE("nvram")
	AM_RANGE(0x1fc00000, 0x1fffffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( vp101 )
	PORT_START("IN0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000008,  IP_ACTIVE_LOW, IPT_START2 )
	
	PORT_BIT( 0xfffffff0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static MACHINE_CONFIG_START( vp101, vp10x_state )
	MCFG_CPU_ADD("maincpu", R5000LE, 400000000) /* actually VR5500 with added NEC VR-series custom instructions */
	MCFG_MIPS3_ICACHE_SIZE(32768)
	MCFG_MIPS3_DCACHE_SIZE(32768)
	MCFG_MIPS3_SYSTEM_CLOCK(100000000)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_UPDATE_DRIVER(vp10x_state, screen_update)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	
	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", nullptr, false)
	MCFG_ATA_INTERFACE_DMARQ_HANDLER(WRITELINE(vp10x_state, dmarq_w))
	
	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END


ROM_START(jnero)
	ROM_REGION(0x400000, "maincpu", 0)  /* Boot ROM */
	ROM_LOAD( "d710.05523.bin", 0x000000, 0x100000, CRC(6054a066) SHA1(58e68b7d86e6f24c79b99c8406e86e3c14387726) )

	ROM_REGION(0x80000, "pic", 0)       /* PIC18c422 program - read-protected, need dumped */
	ROM_LOAD( "8722a-1206.bin", 0x000000, 0x80000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY("jn010108", 0, SHA1(4f3e9c6349c9be59213df1236dba7d79e7cd704e) )
ROM_END

ROM_START(specfrce)
	ROM_REGION(0x400000, "maincpu", 0)  /* Boot ROM */
	ROM_LOAD( "special_forces_boot_v3.4.u4", 0x000000, 0x100000, CRC(db4862ac) SHA1(a1e886d424cf7d26605e29d972d48e8d44ae2d58) )
	ROM_LOAD( "special_forces_boot_v3.5.u4", 0x000000, 0x100000, CRC(ae8dfdf0) SHA1(d64130e710d0c70095ad8ebd4e2194b8c461be4a) ) /* Newer, but keep both in driver */

	ROM_REGION(0x80000, "pic", 0)       /* PIC18c422 I/P program - read-protected, need dumped */
	ROM_LOAD( "special_forces_et_u7_rev1.2.u7", 0x000000, 0x80000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY("sf010101", 0, SHA1(59b5e3d8e1d5537204233598830be2066aad0556) )
ROM_END

ROM_START(zoofari)
	ROM_REGION(0x400000, "maincpu", 0)  /* Boot ROM */
	ROM_LOAD( "zf_boot_rel.u13", 0x000000, 0x400000, CRC(e629689a) SHA1(7352d033c638040c3e51a453e2440a7f38a1b406) )

	ROM_REGION(0x80000, "pic", 0)       /* PIC18c422 program - read-protected, need dumped */
	ROM_LOAD( "8777z-568.bin", 0x000000, 0x80000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY("zoofari", 0, SHA1(8fb9cfb1ab2660f40b643fcd772243903bd69a6c) )
ROM_END

GAME( 2002,  specfrce,  0,  vp101,  vp101, driver_device,  0,  ROT0,  "ICE/Play Mechanix",    "Special Forces Elite Training",   MACHINE_IS_SKELETON )
GAME( 2004,  jnero,     0,  vp101,  vp101, driver_device,  0,  ROT0,  "ICE/Play Mechanix",    "Johnny Nero Action Hero",         MACHINE_IS_SKELETON )
GAME( 2006,  zoofari,   0,  vp101,  vp101, driver_device,  0,  ROT0,  "ICE/Play Mechanix",    "Zoofari",         MACHINE_IS_SKELETON )
