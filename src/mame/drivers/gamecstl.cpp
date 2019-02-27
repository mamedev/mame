// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Cristaltec "Game Cristal" (MAME bootleg)

    Skeleton driver by R. Belmont, based on taitowlf.c by Ville Linde

    Note:
    - bp 000F16B5 do bx=0x16bb (can't skip this check?)

    Specs: P3-866, SiS 630 graphics card, SiS 7018 sound, Windows 98, DirectX 8.1.

    Input is via a custom COM1 port JAMMA adaptor.

    The custom emulator is a heavily modified version of MAME32.  If you extract the
    disk image, it's in C:\GH4\GH4.EXE.  It's UPX compressed, so unpack it before doing
    any forensics.  The emulator does run on Windows as new as XP Pro SP2 but you can't
    control it due to the lack of the custom input.

    Updates 27/11/2007 (Diego Nappino):
    The COM1 port is opened at 19200 bps, No parity, 8 bit data,1 stop bit.
    The protocol is based on a 6 bytes frame with a leading byte valued 0x05 and a trailing one at 0x02
    The four middle bytes are used, in negative logic (0xFF = No button pressed), to implement the inputs.
    Each bit meaning as follows :

               Byte 1         Byte 2          Byte 3        Byte 4
       Bit 0    P1-Credit      P1-Button C     P2-Left        UNUSED
    Bit 1    P1-Start       P1-Button D     P2-Right       UNUSED
    Bit 2    P1-Down        P1-Button E     P2-Button A    SERVICE
    Bit 3    P1-Up          TEST            P2-Button B    UNUSED
    Bit 4    P1-Left        P2-Credit       P2-Button C    UNUSED
    Bit 5    P1-Right       P2-Start        P2-Button D    UNUSED
    Bit 6    P1-Button A    P2-Down         P2-Button E    UNUSED
    Bit 7    P1-Button B    P2-Up           VIDEO-MODE     UNUSED

    The JAMMA adaptor sends a byte frame each time an input changes. So, in example, if the P1-Button A and P1-Button B are both pressed, it will send :

    0x05 0xFC 0xFF 0xFF 0xFF 0x02

    And when the buttons are both released

    0x05 0xFF 0xFF 0xFF 0xFF 0x02

    CPUID info:
    Original set:

    CPUID Level:       EAX:           EBX:           ECX:           EDX:
    00000000       00000003       756E6547       6C65746E       49656E69
    00000001       0000068A       00000002       00000000       0387F9FF
    00000002       03020101       00000000       00000000       0C040882
    00000003       00000000       00000000       CA976D2E       000082F6
    80000000       00000000       00000000       CA976D2E       000082F6
    C0000000       00000000       00000000       CA976D2E       000082F6


    Version 2:
    CPUID Level:       EAX:           EBX:           ECX:           EDX:
    00000000       00000003       756E6547       6C65746E       49656E69
    00000001       0000068A       00000002       00000000       0387F9FF
    00000002       03020101       00000000       00000000       0C040882
    00000003       00000000       00000000       B8BA1941       00038881
    80000000       00000000       00000000       B8BA1941       00038881
    C0000000       00000000       00000000       B8BA1941       00038881

*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/idectrl.h"
#include "machine/lpci.h"
#include "machine/pckeybrd.h"
#include "machine/pcshare.h"
#include "emupal.h"
#include "screen.h"


class gamecstl_state : public pcat_base_state
{
public:
	gamecstl_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag),
		m_cga_ram(*this, "cga_ram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	void gamecstl(machine_config &config);

	void init_gamecstl();

private:
	required_shared_ptr<uint32_t> m_cga_ram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	std::unique_ptr<uint32_t[]> m_bios_ram;
	uint8_t m_mtxc_config_reg[256];
	uint8_t m_piix4_config_reg[4][256];

	DECLARE_WRITE32_MEMBER(pnp_config_w);
	DECLARE_WRITE32_MEMBER(pnp_data_w);
	DECLARE_WRITE32_MEMBER(bios_ram_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_gamecstl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_char(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int ch, int att, int x, int y);
	void intel82439tx_init();
	void gamecstl_io(address_map &map);
	void gamecstl_map(address_map &map);

	uint8_t mtxc_config_r(int function, int reg);
	void mtxc_config_w(int function, int reg, uint8_t data);
	uint32_t intel82439tx_pci_r(int function, int reg, uint32_t mem_mask);
	void intel82439tx_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
	uint8_t piix4_config_r(int function, int reg);
	void piix4_config_w(int function, int reg, uint8_t data);
	uint32_t intel82371ab_pci_r(int function, int reg, uint32_t mem_mask);
	void intel82371ab_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
	uint32_t pci_3dfx_r(int function, int reg, uint32_t mem_mask);
	void pci_3dfx_w(int function, int reg, uint32_t data, uint32_t mem_mask);
};


static const rgb_t cga_palette[16] =
{
	rgb_t( 0x00, 0x00, 0x00 ), rgb_t( 0x00, 0x00, 0xaa ), rgb_t( 0x00, 0xaa, 0x00 ), rgb_t( 0x00, 0xaa, 0xaa ),
	rgb_t( 0xaa, 0x00, 0x00 ), rgb_t( 0xaa, 0x00, 0xaa ), rgb_t( 0xaa, 0x55, 0x00 ), rgb_t( 0xaa, 0xaa, 0xaa ),
	rgb_t( 0x55, 0x55, 0x55 ), rgb_t( 0x55, 0x55, 0xff ), rgb_t( 0x55, 0xff, 0x55 ), rgb_t( 0x55, 0xff, 0xff ),
	rgb_t( 0xff, 0x55, 0x55 ), rgb_t( 0xff, 0x55, 0xff ), rgb_t( 0xff, 0xff, 0x55 ), rgb_t( 0xff, 0xff, 0xff ),
};

void gamecstl_state::video_start()
{
	int i;
	for (i=0; i < 16; i++)
		m_palette->set_pen_color(i, cga_palette[i]);
}

void gamecstl_state::draw_char(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int ch, int att, int x, int y)
{
	int i,j;
	const uint8_t *dp;
	int index = 0;
	dp = gfx->get_data(ch);

	for (j=y; j < y+8; j++)
	{
		uint16_t *p = &bitmap.pix16(j);

		for (i=x; i < x+8; i++)
		{
			uint8_t pen = dp[index++];
			if (pen)
				p[i] = gfx->colorbase() + (att & 0xf);
			else
				p[i] = gfx->colorbase()  + ((att >> 4) & 0x7);
		}
	}
}

uint32_t gamecstl_state::screen_update_gamecstl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, j;
	gfx_element *gfx = m_gfxdecode->gfx(0);
	uint32_t *cga = m_cga_ram;
	int index = 0;

	bitmap.fill(0, cliprect);

	for (j=0; j < 25; j++)
	{
		for (i=0; i < 80; i+=2)
		{
			int att0 = (cga[index] >> 8) & 0xff;
			int ch0 = (cga[index] >> 0) & 0xff;
			int att1 = (cga[index] >> 24) & 0xff;
			int ch1 = (cga[index] >> 16) & 0xff;

			draw_char(bitmap, cliprect, gfx, ch0, att0, i*8, j*8);
			draw_char(bitmap, cliprect, gfx, ch1, att1, (i*8)+8, j*8);
			index++;
		}
	}
	return 0;
}

// Intel 82439TX System Controller (MTXC)

uint8_t gamecstl_state::mtxc_config_r(int function, int reg)
{
	logerror("MTXC: read %d, %02X\n", function, reg);
	return m_mtxc_config_reg[reg];
}

void gamecstl_state::mtxc_config_w(int function, int reg, uint8_t data)
{
	logerror("%s:MTXC: write %d, %02X, %02X\n", machine().describe_context(), function, reg, data);

	switch(reg)
	{
		case 0x59:      // PAM0
		{
			if (data & 0x10)        // enable RAM access to region 0xf0000 - 0xfffff
			{
				membank("bank1")->set_base(m_bios_ram.get());
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				membank("bank1")->set_base(memregion("bios")->base() + 0x30000);
			}
			break;
		}
	}

	m_mtxc_config_reg[reg] = data;
}

void gamecstl_state::intel82439tx_init()
{
	m_mtxc_config_reg[0x60] = 0x02;
	m_mtxc_config_reg[0x61] = 0x02;
	m_mtxc_config_reg[0x62] = 0x02;
	m_mtxc_config_reg[0x63] = 0x02;
	m_mtxc_config_reg[0x64] = 0x02;
	m_mtxc_config_reg[0x65] = 0x02;
}

uint32_t gamecstl_state::intel82439tx_pci_r(int function, int reg, uint32_t mem_mask)
{
	uint32_t r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= mtxc_config_r(function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= mtxc_config_r(function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= mtxc_config_r(function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= mtxc_config_r(function, reg + 0) << 0;
	}
	return r;
}

void gamecstl_state::intel82439tx_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		mtxc_config_w(function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		mtxc_config_w(function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		mtxc_config_w(function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		mtxc_config_w(function, reg + 0, (data >> 0) & 0xff);
	}
}

// Intel 82371AB PCI-to-ISA / IDE bridge (PIIX4)

uint8_t gamecstl_state::piix4_config_r(int function, int reg)
{
	logerror("PIIX4: read %d, %02X\n", function, reg);
	return m_piix4_config_reg[function][reg];
}

void gamecstl_state::piix4_config_w(int function, int reg, uint8_t data)
{
	logerror("%s:PIIX4: write %d, %02X, %02X\n", machine().describe_context(), function, reg, data);
	m_piix4_config_reg[function][reg] = data;
}

uint32_t gamecstl_state::intel82371ab_pci_r(int function, int reg, uint32_t mem_mask)
{
	uint32_t r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= piix4_config_r(function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= piix4_config_r(function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= piix4_config_r(function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= piix4_config_r(function, reg + 0) << 0;
	}
	return r;
}

void gamecstl_state::intel82371ab_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		piix4_config_w(function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		piix4_config_w(function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		piix4_config_w(function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		piix4_config_w(function, reg + 0, (data >> 0) & 0xff);
	}
}

// ISA Plug-n-Play
WRITE32_MEMBER(gamecstl_state::pnp_config_w)
{
	if (ACCESSING_BITS_8_15)
	{
//      osd_printf_debug("PNP Config: %02X\n", (data >> 8) & 0xff);
	}
}

WRITE32_MEMBER(gamecstl_state::pnp_data_w)
{
	if (ACCESSING_BITS_8_15)
	{
//      osd_printf_debug("PNP Data: %02X\n", (data >> 8) & 0xff);
	}
}



WRITE32_MEMBER(gamecstl_state::bios_ram_w)
{
	if (m_mtxc_config_reg[0x59] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ram.get() + offset);
	}
}

/*****************************************************************************/

void gamecstl_state::gamecstl_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000a0000, 0x000affff).ram();
	map(0x000b0000, 0x000b7fff).ram().share("cga_ram");
	map(0x000e0000, 0x000effff).ram();
	map(0x000f0000, 0x000fffff).bankr("bank1");
	map(0x000f0000, 0x000fffff).w(FUNC(gamecstl_state::bios_ram_w));
	map(0x00100000, 0x01ffffff).ram();
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);    /* System BIOS */
}

void gamecstl_state::gamecstl_io(address_map &map)
{
	pcat32_io_common(map);
	map(0x00e8, 0x00eb).noprw();
	map(0x00ec, 0x00ef).noprw();
	map(0x01f0, 0x01f7).rw("ide", FUNC(ide_controller_device::cs0_r), FUNC(ide_controller_device::cs0_w));
	map(0x0300, 0x03af).noprw();
	map(0x03b0, 0x03df).noprw();
	map(0x0278, 0x027b).w(FUNC(gamecstl_state::pnp_config_w));
	map(0x03f0, 0x03f7).rw("ide", FUNC(ide_controller_device::cs1_r), FUNC(ide_controller_device::cs1_w));
	map(0x0a78, 0x0a7b).w(FUNC(gamecstl_state::pnp_data_w));
	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_legacy_device::read), FUNC(pci_bus_legacy_device::write));
}

/*****************************************************************************/

static const gfx_layout CGA_charlayout =
{
	8,8,                    /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 0,1,2,3,4,5,6,7 },
	/* y offsets */
	{ 0*8,1*8,2*8,3*8,
		4*8,5*8,6*8,7*8 },
	8*8                     /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_cga )
/* Support up to four CGA fonts */
	GFXDECODE_ENTRY( "gfx1", 0x0000, CGA_charlayout,              0, 256 )   /* Font 0 */
	GFXDECODE_ENTRY( "gfx1", 0x0800, CGA_charlayout,              0, 256 )   /* Font 1 */
	GFXDECODE_ENTRY( "gfx1", 0x1000, CGA_charlayout,              0, 256 )   /* Font 2 */
	GFXDECODE_ENTRY( "gfx1", 0x1800, CGA_charlayout,              0, 256 )   /* Font 3*/
GFXDECODE_END

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(text) PORT_CODE(key1)

static INPUT_PORTS_START(gamecstl)
	PORT_START("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED )     /* unused scancode 0 */
	AT_KEYB_HELPER( 0x0002, "Esc",          KEYCODE_Q           ) /* Esc                         01  81 */

	PORT_START("pc_keyboard_1")
	AT_KEYB_HELPER( 0x0020, "Y",            KEYCODE_Y           ) /* Y                           15  95 */
	AT_KEYB_HELPER( 0x1000, "Enter",        KEYCODE_ENTER       ) /* Enter                       1C  9C */

	PORT_START("pc_keyboard_2")

	PORT_START("pc_keyboard_3")
	AT_KEYB_HELPER( 0x0002, "N",            KEYCODE_N           ) /* N                           31  B1 */
	AT_KEYB_HELPER( 0x0800, "F1",           KEYCODE_S           ) /* F1                          3B  BB */

	PORT_START("pc_keyboard_4")

	PORT_START("pc_keyboard_5")

	PORT_START("pc_keyboard_6")
	AT_KEYB_HELPER( 0x0040, "(MF2)Cursor Up",       KEYCODE_UP          ) /* Up                          67  e7 */
	AT_KEYB_HELPER( 0x0080, "(MF2)Page Up",         KEYCODE_PGUP        ) /* Page Up                     68  e8 */
	AT_KEYB_HELPER( 0x0100, "(MF2)Cursor Left",     KEYCODE_LEFT        ) /* Left                        69  e9 */
	AT_KEYB_HELPER( 0x0200, "(MF2)Cursor Right",    KEYCODE_RIGHT       ) /* Right                       6a  ea */
	AT_KEYB_HELPER( 0x0800, "(MF2)Cursor Down",     KEYCODE_DOWN        ) /* Down                        6c  ec */
	AT_KEYB_HELPER( 0x1000, "(MF2)Page Down",       KEYCODE_PGDN        ) /* Page Down                   6d  ed */
	AT_KEYB_HELPER( 0x4000, "Del",                  KEYCODE_A           ) /* Delete                      6f  ef */

	PORT_START("pc_keyboard_7")
INPUT_PORTS_END

void gamecstl_state::machine_start()
{
}

void gamecstl_state::machine_reset()
{
	membank("bank1")->set_base(memregion("bios")->base() + 0x30000);
}

MACHINE_CONFIG_START(gamecstl_state::gamecstl)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", PENTIUM3, 200000000)
	MCFG_DEVICE_PROGRAM_MAP(gamecstl_map)
	MCFG_DEVICE_IO_MAP(gamecstl_io)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)

	pcat_common(config);

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, DEVICE_SELF, gamecstl_state, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(7, DEVICE_SELF, gamecstl_state, intel82371ab_pci_r, intel82371ab_pci_w)

	ide_controller_device &ide(IDE_CONTROLLER(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir6_w));

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 199)
	MCFG_SCREEN_UPDATE_DRIVER(gamecstl_state, screen_update_gamecstl)
	MCFG_SCREEN_PALETTE("palette")

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cga);
	MCFG_PALETTE_ADD("palette", 16)


MACHINE_CONFIG_END

void gamecstl_state::init_gamecstl()
{
	m_bios_ram = std::make_unique<uint32_t[]>(0x10000/4);

	intel82439tx_init();
}

/*****************************************************************************/

// not the correct BIOS, f205v owes me a dump of it...
ROM_START(gamecstl)
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD( "bios.bin",     0x000000, 0x040000, BAD_DUMP CRC(27834ce9) SHA1(134c546dd75138c6f4bc5729b40e20e118454df9) )

	ROM_REGION(0x08100, "gfx1", 0)
	ROM_LOAD("cga.chr",     0x00000, 0x01000, BAD_DUMP CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "gamecstl", 0, SHA1(b431af3c42c48ba07972d77a3d24e60ee1e4359e) )
ROM_END

ROM_START(gamecst2)
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD( "bios.bin",     0x000000, 0x040000, BAD_DUMP CRC(27834ce9) SHA1(134c546dd75138c6f4bc5729b40e20e118454df9) )

	ROM_REGION(0x08100, "gfx1", 0)
	ROM_LOAD("cga.chr",     0x00000, 0x01000, BAD_DUMP CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "gamecst2", 0, SHA1(14e1b311cb474801c7bdda3164a0c220fb102159) )
ROM_END

/*****************************************************************************/

GAME(2002, gamecstl, 0,        gamecstl, gamecstl, gamecstl_state, init_gamecstl, ROT0, "Cristaltec", "GameCristal",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(2002, gamecst2, gamecstl, gamecstl, gamecstl, gamecstl_state, init_gamecstl, ROT0, "Cristaltec", "GameCristal (version 2.613)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
