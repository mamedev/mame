// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*  Taito Wolf System

    Driver by Ville Linde

AMD M4-128N/64 stamped 'E58-01'
AMD MACH231 stamped 'E58-02'
AMD MACH211 stamped 'E58-03'
Zoom ZFX2
Zoom ZSG-2
Taito TC0510NIO
Panasonic MN1020019
1x RAM NEC 42S4260
1x RAM GM71C4400
12x RAM Alliance AS4C256K16E0-35 (256k x 16)
Mitsubishi M66220
Fujitsu MB87078
Atmel 93C66 EEPROM (4kb probably for high scores, test mode settings etc)
ICS GENDAC ICS5342-3
3DFX 500-0003-03 F805281.1 FBI
3DFX 500-0004-02 F804701.1 TMU
some logic
clocks 50MHz (near 3DFX) and 14.31818MHz (near RAMDAC)

*/

#define ENABLE_VGA 0

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pcshare.h"
#include "machine/pckeybrd.h"
#if ENABLE_VGA
#include "video/pc_vga.h"
#endif

class taitowlf_state : public pcat_base_state
{
public:
	taitowlf_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag),
		m_bootscreen_rom(*this, "bootscreen"),
		m_bank1(*this, "bank1"),
		m_palette(*this, "palette") { }

	std::unique_ptr<UINT32[]> m_bios_ram;
	UINT8 m_mtxc_config_reg[256];
	UINT8 m_piix4_config_reg[4][256];

	required_region_ptr<UINT8> m_bootscreen_rom;
	required_memory_bank m_bank1;
	required_device<palette_device> m_palette;
	DECLARE_WRITE32_MEMBER(pnp_config_w);
	DECLARE_WRITE32_MEMBER(pnp_data_w);
	DECLARE_WRITE32_MEMBER(bios_ram_w);
	DECLARE_DRIVER_INIT(taitowlf);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	#if !ENABLE_VGA
	DECLARE_PALETTE_INIT(taitowlf);
	#endif
	UINT32 screen_update_taitowlf(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void intel82439tx_init();
};

#if !ENABLE_VGA
UINT32 taitowlf_state::screen_update_taitowlf(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y,count;

	bitmap.fill(m_palette->black_pen(), cliprect);

	count = (0);

	for(y=0;y<256;y++)
	{
		for(x=0;x<512;x++)
		{
			UINT32 color;

			color = (m_bootscreen_rom[count] & 0xff);

			if(cliprect.contains(x+0, y))
				bitmap.pix32(y, x+0) = m_palette->pen(color);

			count++;
		}
	}

	return 0;
}
#endif


// Intel 82439TX System Controller (MTXC)

static UINT8 mtxc_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	taitowlf_state *state = busdevice->machine().driver_data<taitowlf_state>();
//  osd_printf_debug("MTXC: read %d, %02X\n", function, reg);

	return state->m_mtxc_config_reg[reg];
}

static void mtxc_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	taitowlf_state *state = busdevice->machine().driver_data<taitowlf_state>();
//  osd_printf_debug("%s:MTXC: write %d, %02X, %02X\n", machine.describe_context(), function, reg, data);

	switch(reg)
	{
		case 0x59:      // PAM0
		{
			if (data & 0x10)        // enable RAM access to region 0xf0000 - 0xfffff
			{
				state->m_bank1->set_entry(1);
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				state->m_bank1->set_entry(0);
			}
			break;
		}
	}

	state->m_mtxc_config_reg[reg] = data;
}

void taitowlf_state::intel82439tx_init()
{
	m_mtxc_config_reg[0x60] = 0x02;
	m_mtxc_config_reg[0x61] = 0x02;
	m_mtxc_config_reg[0x62] = 0x02;
	m_mtxc_config_reg[0x63] = 0x02;
	m_mtxc_config_reg[0x64] = 0x02;
	m_mtxc_config_reg[0x65] = 0x02;
}

static UINT32 intel82439tx_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
	UINT32 r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= mtxc_config_r(busdevice, device, function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= mtxc_config_r(busdevice, device, function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= mtxc_config_r(busdevice, device, function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= mtxc_config_r(busdevice, device, function, reg + 0) << 0;
	}
	return r;
}

static void intel82439tx_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		mtxc_config_w(busdevice, device, function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		mtxc_config_w(busdevice, device, function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		mtxc_config_w(busdevice, device, function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		mtxc_config_w(busdevice, device, function, reg + 0, (data >> 0) & 0xff);
	}
}

// Intel 82371AB PCI-to-ISA / IDE bridge (PIIX4)

static UINT8 piix4_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	taitowlf_state *state = busdevice->machine().driver_data<taitowlf_state>();
//  osd_printf_debug("PIIX4: read %d, %02X\n", function, reg);
	return state->m_piix4_config_reg[function][reg];
}

static void piix4_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	taitowlf_state *state = busdevice->machine().driver_data<taitowlf_state>();
//  osd_printf_debug("%s:PIIX4: write %d, %02X, %02X\n", machine.describe_context(), function, reg, data);
	state->m_piix4_config_reg[function][reg] = data;
}

static UINT32 intel82371ab_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
	UINT32 r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 0) << 0;
	}
	return r;
}

static void intel82371ab_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		piix4_config_w(busdevice, device, function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		piix4_config_w(busdevice, device, function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		piix4_config_w(busdevice, device, function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		piix4_config_w(busdevice, device, function, reg + 0, (data >> 0) & 0xff);
	}
}

// ISA Plug-n-Play
WRITE32_MEMBER(taitowlf_state::pnp_config_w)
{
	if (ACCESSING_BITS_8_15)
	{
//      osd_printf_debug("PNP Config: %02X\n", (data >> 8) & 0xff);
	}
}

WRITE32_MEMBER(taitowlf_state::pnp_data_w)
{
	if (ACCESSING_BITS_8_15)
	{
//      osd_printf_debug("PNP Data: %02X\n", (data >> 8) & 0xff);
	}
}



WRITE32_MEMBER(taitowlf_state::bios_ram_w)
{
	if (m_mtxc_config_reg[0x59] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ram.get() + offset);
	}
}


static ADDRESS_MAP_START( taitowlf_map, AS_PROGRAM, 32, taitowlf_state )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	#if ENABLE_VGA
	AM_RANGE(0x000a0000, 0x000bffff) AM_DEVREADWRITE8("vga", vga_device, mem_r, mem_w, 0xffffffff)
	#else
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM
	#endif
	#if ENABLE_VGA
	AM_RANGE(0x000c0000, 0x000c7fff) AM_RAM AM_REGION("video_bios", 0)
	#else
	AM_RANGE(0x000c0000, 0x000c7fff) AM_NOP
	#endif
	AM_RANGE(0x000e0000, 0x000effff) AM_RAM
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROMBANK("bank1")
	AM_RANGE(0x000f0000, 0x000fffff) AM_WRITE(bios_ram_w)
	AM_RANGE(0x00100000, 0x01ffffff) AM_RAM
//  AM_RANGE(0xf8000000, 0xf83fffff) AM_ROM AM_REGION("user3", 0)
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)   /* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(taitowlf_io, AS_IO, 32, taitowlf_state )
	AM_IMPORT_FROM(pcat32_io_common)

	AM_RANGE(0x00e8, 0x00eb) AM_NOP
	AM_RANGE(0x0300, 0x03af) AM_NOP
	AM_RANGE(0x03b0, 0x03df) AM_NOP
	AM_RANGE(0x0278, 0x027b) AM_WRITE(pnp_config_w)
	#if ENABLE_VGA
	AM_RANGE(0x03b0, 0x03bf) AM_DEVREADWRITE8("vga", vga_device, port_03b0_r, port_03b0_w, 0xffffffff)
	AM_RANGE(0x03c0, 0x03cf) AM_DEVREADWRITE8("vga", vga_device, port_03c0_r, port_03c0_w, 0xffffffff)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE8("vga", vga_device, port_03d0_r, port_03d0_w, 0xffffffff)
	#endif
	AM_RANGE(0x0a78, 0x0a7b) AM_WRITE(pnp_data_w)
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)
ADDRESS_MAP_END

/*****************************************************************************/

#if 0
#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(text) PORT_CODE(key1)

static INPUT_PORTS_START(taitowlf)
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
#endif

void taitowlf_state::machine_start()
{
}

void taitowlf_state::machine_reset()
{
	// disable RAM access (reads go to BIOS ROM)
	m_bank1->set_entry(0);
}


#if !ENABLE_VGA
/* debug purpose*/
PALETTE_INIT_MEMBER(taitowlf_state, taitowlf)
{
	palette.set_pen_color(0x70,rgb_t(0xff,0xff,0xff));
	palette.set_pen_color(0x71,rgb_t(0xff,0xff,0xff));
	palette.set_pen_color(0x01,rgb_t(0x55,0x00,0x00));
	palette.set_pen_color(0x10,rgb_t(0xaa,0x00,0x00));
	palette.set_pen_color(0x00,rgb_t(0x00,0x00,0x00));
}
#endif

static MACHINE_CONFIG_START( taitowlf, taitowlf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM, 200000000)
	MCFG_CPU_PROGRAM_MAP(taitowlf_map)
	MCFG_CPU_IO_MAP(taitowlf_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)


	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, nullptr, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(7, nullptr, intel82371ab_pci_r, intel82371ab_pci_w)

	MCFG_FRAGMENT_ADD( pcat_common )

	/* video hardware */
	#if ENABLE_VGA
	MCFG_FRAGMENT_ADD( pcvideo_vga )
	#else
	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(taitowlf_state, screen_update_taitowlf)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(taitowlf_state, taitowlf)
	#endif
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(taitowlf_state,taitowlf)
{
	m_bios_ram = std::make_unique<UINT32[]>(0x10000/4);

	m_bank1->configure_entry(1, m_bios_ram.get());
	m_bank1->configure_entry(0, memregion("bios")->base() + 0x30000);
	intel82439tx_init();
}

/*****************************************************************************/

ROM_START(pf2012)
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD("p5tx-la.bin", 0x00000, 0x40000, CRC(072e6d51) SHA1(70414349b37e478fc28ecbaba47ad1033ae583b7))

	#if ENABLE_VGA
	ROM_REGION( 0x8000, "video_bios", 0 ) // debug
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )
	#endif

	ROM_REGION(0x400000, "user3", 0)       // Program ROM disk
	ROM_LOAD("u1.bin", 0x000000, 0x200000, CRC(8f4c09cb) SHA1(0969a92fec819868881683c580f9e01cbedf4ad2))
	ROM_LOAD("u2.bin", 0x200000, 0x200000, CRC(59881781) SHA1(85ff074ab2a922eac37cf96f0bf153a2dac55aa4))

	ROM_REGION(0x4000000, "user4", 0)      // Data ROM disk
	ROM_LOAD("e59-01.u20", 0x0000000, 0x800000, CRC(701d3a9a) SHA1(34c9f34f4da34bb8eed85a4efd1d9eea47a21d77) )
	ROM_LOAD("e59-02.u23", 0x0800000, 0x800000, CRC(626df682) SHA1(35bb4f91201734ce7ccdc640a75030aaca3d1151) )
	ROM_LOAD("e59-03.u26", 0x1000000, 0x800000, CRC(74e4efde) SHA1(630235c2e4a11f615b5f3b8c93e1e645da09eefe) )
	ROM_LOAD("e59-04.u21", 0x1800000, 0x800000, CRC(c900e8df) SHA1(93c06b8f5082e33f0dcc41f1be6a79283de16c40) )
	ROM_LOAD("e59-05.u24", 0x2000000, 0x800000, CRC(85b0954c) SHA1(1b533d5888d56d1510c79f790e4fa708f77e836f) )
	ROM_LOAD("e59-06.u27", 0x2800000, 0x800000, CRC(0573a113) SHA1(ee76a71dfd31289a9a5428653a36d01d914fc5d9) )
	ROM_LOAD("e59-07.u22", 0x3000000, 0x800000, CRC(1f0ddcdc) SHA1(72ffe08f5effab093bdfe9863f8a11f80e914272) )
	ROM_LOAD("e59-08.u25", 0x3800000, 0x800000, CRC(8db38ffd) SHA1(4b71ea86fb774ba6a8ac45abf4191af64af007e7) )

	ROM_REGION(0x1400000, "samples", 0)         // ZOOM sample data
	ROM_LOAD("e59-09.u29", 0x0000000, 0x800000, CRC(d0da5c50) SHA1(56fb3c38f35244720d32a44fed28e6b58c7851f7) )
	ROM_LOAD("e59-10.u32", 0x0800000, 0x800000, CRC(4c0e0a5c) SHA1(6454befa3a1dd532eb2a760129dcd7e611508730) )
	ROM_LOAD("e59-11.u33", 0x1000000, 0x400000, CRC(c90a896d) SHA1(2b62992f20e4ca9634e7953fe2c553906de44f04) )

	ROM_REGION(0x180000, "cpu1", 0)         // MN10200 program
	ROM_LOAD("e59-12.u13", 0x000000, 0x80000, CRC(9a473a7e) SHA1(b0ec7b0ae2b33a32da98899aa79d44e8e318ceb7) )
	ROM_LOAD("e59-13.u15", 0x080000, 0x80000, CRC(77719880) SHA1(8382dd2dfb0dae60a3831ed6d3ff08539e2d94eb) )
	ROM_LOAD("e59-14.u14", 0x100000, 0x40000, CRC(d440887c) SHA1(d965871860d757bc9111e9adb2303a633c662d6b) )
	ROM_LOAD("e59-15.u16", 0x140000, 0x40000, CRC(eae8e523) SHA1(8a054d3ded7248a7906c4f0bec755ddce53e2023) )

	ROM_REGION(0x20000, "bootscreen", 0)         // bootscreen
	ROM_LOAD("e58-04.u71", 0x000000, 0x20000, CRC(500e6113) SHA1(93226706517c02e336f96bdf9443785158e7becf) )
ROM_END

/*****************************************************************************/

GAME(1997, pf2012, 0,   taitowlf, pc_keyboard, taitowlf_state, taitowlf,    ROT0,   "Taito",  "Psychic Force 2012", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
