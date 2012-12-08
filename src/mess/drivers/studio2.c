/*

RCA Studio II

PCB Layout
----------

1809746-S1-E

|----------------------------------------------------------------------------------------------------------|
|                                  |----------------------------|                                          |
|                                  |----------------------------|                          CA555   7805    |
|      SPKR                                     CN1                                                        |
|                                                                                                          |
|                                                                                           |-----------|  |
|                                                                                           | Output    |  |
|   ROM.4  ROM.3  ROM.2  ROM.1  CDP1822  CDP1822  CDP1822  CDP1822                          |TV Module  |  |
|                                                                                           |           |  |
|                                                                 CDP1802  TA10171V1        |           |  |
|                                                                                           |           |  |
|                                                                                           |           |  |
|                                                                                           |           |  |
|                                                                                           |-----------|  |
|                                                                                                          |
|                       CD4042  CD4001  CD4515                                                             |
|                                                                                                          |
|      CN2                                                                                       CN3       |
|----------------------------------------------------------------------------------------------------------|

Notes:
      All IC's shown.

      CDP1802 - RCA CDP1802CE Microprocessor
      TA10171V1 - RCA TA10171V1 NTSC Video Display Controller (VDC) (= RCA CDP1861)
      CDP1822 - RCA CDP1822NCE 256 x4 RAM (= Mitsubishi M58721P)
      ROM.x   - RCA CDP1831CE 512 x8 MASKROM. All ROMs are marked 'PROGRAM COPYRIGHT (C) RCA CORP. 1977'
      CD4001  - 4001 Quad 2-Input NOR Buffered B Series Gate (4000-series CMOS TTL logic IC)
      CD4042  - 4042 Quad Clocked D Latch (4000-series CMOS TTL logic IC)
      CD4515  - 4515 4-Bit Latched/4-to-16 Line Decoders (4000-series CMOS TTL logic IC)
      CA555   - CA555CG General Purpose Single Bipolar Timer (= NE555)
      7805    - Voltage regulator, input 10V-35V, output +5V
      SPKR    - Loudspeaker, 8 ohms, 0.3 W
      CN1     - ROM cartridge connector, 2x22 pins, 0.154" spacing
      CN2     - Player A keypad connector, 1x12 pins
      CN3     - Player B keypad connector, 1x12 pins
*/

/*

Toshiba Visicom Console (RCA Studio II clone)
Toshiba, 1978

PCB Layout                                            Many resistors/caps
----------                                7.5VDC    / transistors in this area
                                           \/   /---|-----/
|------------------------------------------||---|-------|   |-----|C|------|
|D235  POT       TC4011     CART_SLOT           |       |   | TV Modulator |
|HEATSINK                                       |       |   |              |
|           TC4515          TC4049 TC4011 74LS08|       |   | SW (VHF Ch1) |
|DIODES(x20)                       74LS74 74LS73|       |   |    (VHF Ch2) |
|-----------|                                 3.579545MHz   |              |
            |        CDP1802                    |  POT  |   |--------------|
            |TMM331                             |       |
            |               CDP1861   TC5012 TC5012     |
            |                     TC4021 TC4021 |       |
            |                          TC5012   \-----/ |
            |          2111      2111   2111   74LS74   |
            |TC4042    2111      2111   2111   TC4011   |
            |-------------------------------------------|
Notes: (all chips shown above)
      CDP1802 - RCA CDP1802 CPU (DIP40), clock 1.7897725MHz [3.579545/2]
      CDP1861 - RCA CDP1861 Video Controller (DIP24)
                VSync - 60.4533Hz   \ (measured on pin 6)
                HSync - 15.8387kHz  /
                Clock - 223.721562kHz [3.579545/16] (measured on pin 1)
      2111    - NEC D2111AL-4 256 bytes x4 SRAM (DIP18, x6). Total 1.5k
      C       - Composite Video Output to TV from TV Modulator
      TMM331  - Toshiba TMM331AP 2k x8 MASKROM (DIP24)
                Pinout (preliminary):
                           TMM331
                        |----\/----|
                     A7 |1       24| VCC
                     A8 |2       23| D0
                     A9 |3       22| D1
                    A10 |4       21| D2
                     A0 |5       20| D3
                     A1 |6       19| D4
                     A2 |7       18| D5
                     A3 |8       17| D6
                     A4 |9       16| D7
                     A5 |10      15| E0 (measured LOW)
                     A6 |11      14| E1 (NC?)
                    GND |12      13| E2 (measured LOW)
                        |----------|

      E0 - E2 are Programmable Chip Select Inputs
      TMM331 is compatible with AMI S6831A, AMD AM9217,
      Intel 2316A/8316A, MOSTEK MK31000, GI RO-3-8316,
      NATIONAL/NEC/SYNERTEK 2316A etc
      
*/

/*

Mustang 9016 Telespiel Computer

PCB Layout
----------

|----------------------------------------------------------------------------------------------------------|
|7805                              |----------------------------|                          CD4069  MC14001 |
|                                  |----------------------------|                                          |
|                                               CN1                                                        |
|                                                                                                          |
|       ROM.IC13  ROM.IC14      CDP1822  CDP1822 CDP1822 CDP1822                            |-----------|  |
|                                                                                           | Output    |  |
|                                                                                           |TV Module? |  |
| ROM.IC12                                                        CDP1802  CDP1864          |           |  |
|                 CDP1822                                                                   |           |  |
|                          CD4019 CDP1858 CD4081 CD4069                                     |           |  |
|                                                                                           |           |  |
|                                                        CD4515                             |           |  |
|                                                                          1.750MHz         |-----------|  |
|                                                                                              4.3236MHz   |
|                                                                                                          |
|                                                                                                          |
|                                                                                                          |
|----------------------------------------------------------------------------------------------------------|

Notes:
      All IC's shown.

      CDP1802 - RCA CDP1802CE Microprocessor
      CDP1864 - RCA CDP1864CE PAL Video Display Controller (VDC)
      CDP1822 - RCA CDP1822NCE 256 x4 RAM (= Mitsubishi M58721P)
      ROM.ICx - RCA CDP1833 1k x8 MASKROM. All ROMs are marked 'PROGRAM COPYRIGHT (C) RCA CORP. 1978'
      CD4019  - 4019 Quad AND-OR Select Gate (4000-series CMOS TTL logic IC)
      CDP1858 - RCA CDP1858E Latch/Decoder - 4-bit
      CD4081  - 4081 Quad 2-Input AND Buffered B Series Gate (4000-series CMOS TTL logic IC)
      CD4069  - 4069 Hex Buffer, Inverter (4000-series CMOS TTL logic IC)
      CD4515  - 4515 4-Bit Latched/4-to-16 Line Decoders (4000-series CMOS TTL logic IC)
      7805    - Voltage regulator, input 10V-35V, output +5V
      CN1     - ROM cartridge connector, 2x22 pins, 0.154" spacing
*/

/*

    RCA Studio II games list

    Title                           Series                  Dumped
    ----------------------------------------------------------------------------
    Bowling                         built-in                yes
    Doodles                         built-in                yes
    Freeway                         built-in                yes
    Math                            built-in                yes
    Patterns                        built-in                yes
    Gunfighter/Moonship Battle      TV Arcade               yes
    Space War                       TV Arcade I             yes
    Fun with Numbers                TV Arcade II            no, but Guru has one
    Tennis/Squash                   TV Arcade III           yes
    Baseball                        TV Arcade IV            yes
    Speedway/Tag                    TV Arcade               yes
    Blackjack                       TV Casino I             yes
    Bingo                           TV Casino               no
    Math and Social Studies         TV School House I       no, but Guru has one
    Math Fun                        TV School House II      yes
    Biorhythm                       TV Mystic               yes


    MPT-02 games list

    ID      Title                   Series                  Dumped
    ----------------------------------------------------------------------------
    MG-201  Bingo                                           no
    MG-202  Concentration Match                             no, but Guru has one
    MG-203  Star Wars                                       no, but Guru has one
    MG-204  Math Fun                School House II         no, but Guru has one
    MG-205  Pinball                                         no, but Guru has one
    MG-206  Biorythm                                        no
    MG-207  Tennis/Squash                                   no
    MG-208  Fun with Numbers                                no
    MG-209  Computer Quiz           School House I          no
    MG-210  Baseball                                        no
    MG-211  Speedway/Tag                                    no
    MG-212  Spacewar Intercept                              no
    MG-213  Gun Fight/Moon Ship                             no

*/

/*

    TODO:

    - disable ic13/14 when cartridge plugged in
    - mpt02 clones' colors
    - visicom colors
    - NE555 discrete sound

*/

#include "includes/studio2.h"

/* Read/Write Handlers */

WRITE8_MEMBER( studio2_state::keylatch_w )
{
	m_keylatch = data & 0x0f;
}

READ8_MEMBER( studio2_state::dispon_r )
{
	m_vdc->disp_on_w(1);
	m_vdc->disp_on_w(0);

	return 0xff;
}

WRITE8_MEMBER( studio2_state::dispon_w )
{
	m_vdc->disp_on_w(1);
	m_vdc->disp_on_w(0);
}

/* Memory Maps */

static ADDRESS_MAP_START( studio2_map, AS_PROGRAM, 8, studio2_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x09ff) AM_MIRROR(0xf400) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( studio2_io_map, AS_IO, 8, studio2_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x01, 0x01) AM_READ(dispon_r)
	AM_RANGE(0x02, 0x02) AM_WRITE(keylatch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( visicom_map, AS_PROGRAM, 8, visicom_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x1000, 0x10ff) AM_RAM
	AM_RANGE(0x1100, 0x11ff) AM_RAM AM_SHARE("color_ram")
	AM_RANGE(0x1300, 0x13ff) AM_RAM AM_SHARE("color_ram1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( visicom_io_map, AS_IO, 8, visicom_state )
	AM_RANGE(0x01, 0x01) AM_WRITE(dispon_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(keylatch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpt02_map, AS_PROGRAM, 8, mpt02_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x09ff) AM_RAM
	AM_RANGE(0x0b00, 0x0b3f) AM_RAM AM_SHARE("color_ram")
	AM_RANGE(0x0c00, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpt02_io_map, AS_IO, 8, mpt02_state )
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE(CDP1864_TAG, cdp1864_device, dispon_r, step_bgcolor_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(keylatch_w)
	AM_RANGE(0x04, 0x04) AM_DEVREADWRITE(CDP1864_TAG, cdp1864_device, dispoff_r, tone_latch_w)
ADDRESS_MAP_END

/* Input Ports */

INPUT_CHANGED_MEMBER( studio2_state::reset_w )
{
	if (oldval && !newval)
	{
		machine_reset();
	}
}

static INPUT_PORTS_START( studio2 )
	PORT_START("A")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("A 0") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("A 1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("A 2") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("A 3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("A 4") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("A 5") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("A 6") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("A 7") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("A 8") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("A 9") PORT_CODE(KEYCODE_C)

	PORT_START("B")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("B 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("B 1") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("B 2") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("B 3") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("B 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("B 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("B 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("B 7") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("B 8") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("B 9") PORT_CODE(KEYCODE_3_PAD)

	PORT_START("CLEAR")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Clear") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHANGED_MEMBER(DEVICE_SELF, studio2_state, reset_w, 0)
INPUT_PORTS_END

/* Video */

static CDP1861_INTERFACE( studio2_cdp1861_intf )
{
	CDP1802_TAG,
	SCREEN_TAG,
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_INT),
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_DMAOUT),
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_EF1)
};

static const rgb_t VISICOM_PALETTE[] =
{
    MAKE_RGB(0x00, 0x80, 0x00),
    MAKE_RGB(0x00, 0x00, 0xff),
    MAKE_RGB(0x00, 0xff, 0x00),
    MAKE_RGB(0xff, 0x00, 0x00)
};

READ_LINE_MEMBER( mpt02_state::rdata_r )
{
	return BIT(m_color, 0);
}

READ_LINE_MEMBER( mpt02_state::bdata_r )
{
	return BIT(m_color, 1);
}

READ_LINE_MEMBER( mpt02_state::gdata_r )
{
	return BIT(m_color, 2);
}

static CDP1864_INTERFACE( mpt02_cdp1864_intf )
{
	CDP1802_TAG,
	SCREEN_TAG,
	CDP1864_INTERLACED,
	DEVCB_DRIVER_LINE_MEMBER(mpt02_state, rdata_r),
	DEVCB_DRIVER_LINE_MEMBER(mpt02_state, bdata_r),
	DEVCB_DRIVER_LINE_MEMBER(mpt02_state, gdata_r),
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_INT),
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_DMAOUT),
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_EF1),
	DEVCB_NULL,
	RES_K(2.2),	// unverified
	RES_K(1),	// unverified
	RES_K(5.1),	// unverified
	RES_K(4.7)	// unverified
};

/* CDP1802 Configuration */

READ_LINE_MEMBER( studio2_state::clear_r )
{
	return BIT(ioport("CLEAR")->read(), 0);
}

READ_LINE_MEMBER( studio2_state::ef3_r )
{
	return BIT(ioport("A")->read(), m_keylatch);
}

READ_LINE_MEMBER( studio2_state::ef4_r )
{
	return BIT(ioport("B")->read(), m_keylatch);
}

WRITE_LINE_MEMBER( studio2_state::q_w )
{
	beep_set_state(m_speaker, state);
}

static COSMAC_INTERFACE( studio2_cosmac_intf )
{
	DEVCB_LINE_VCC,
	DEVCB_DRIVER_LINE_MEMBER(studio2_state, clear_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(studio2_state, ef3_r),
	DEVCB_DRIVER_LINE_MEMBER(studio2_state, ef4_r),
	DEVCB_DRIVER_LINE_MEMBER(studio2_state, q_w),
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(CDP1861_TAG, cdp1861_device, dma_w),
	NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

WRITE8_MEMBER( mpt02_state::dma_w )
{
	UINT8 addr = ((offset & 0xe0) >> 2) | (offset & 0x07);

	m_color = m_color_ram[addr];

	m_cti->con_w(0); // HACK
	m_cti->dma_w(space, offset, data);
}

static COSMAC_INTERFACE( mpt02_cosmac_intf )
{
	DEVCB_LINE_VCC,
	DEVCB_DRIVER_LINE_MEMBER(studio2_state, clear_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(studio2_state, ef3_r),
	DEVCB_DRIVER_LINE_MEMBER(studio2_state, ef4_r),
	DEVCB_DRIVER_LINE_MEMBER(studio2_state, q_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(mpt02_state, dma_w),
	NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

/* Machine Initialization */

void studio2_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_keylatch));
}

void studio2_state::machine_reset()
{
	m_vdc->reset();
}

void mpt02_state::machine_reset()
{
	m_cti->reset();
}

DEVICE_IMAGE_LOAD( studio2_cart_load )
{
	if (image.software_entry() == NULL)
		return device_load_st2_cartslot_load(image);
	else
	{
		// WARNING: list code currently assume that cart mapping starts at 0x400.
		// the five dumps currently available work like this, but the .st2 format
		// allows for more freedom... how was the content of a real cart mapped?
		UINT8 *ptr = ((UINT8 *) image.device().machine().root_device().memregion(CDP1802_TAG)->base()) + 0x400;
		memcpy(ptr, image.get_software_region("rom"), image.get_software_region_length("rom"));
		return IMAGE_INIT_PASS;
	}
}

/* Machine Drivers */

static MACHINE_CONFIG_FRAGMENT( studio2_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("st2,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(studio2_cart_load)
	MCFG_CARTSLOT_INTERFACE("studio2_cart")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","studio2")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( studio2, studio2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(CDP1802_TAG, COSMAC, 1760000) /* the real clock is derived from an oscillator circuit */
	MCFG_CPU_PROGRAM_MAP(studio2_map)
	MCFG_CPU_IO_MAP(studio2_io_map)
	MCFG_CPU_CONFIG(studio2_cosmac_intf)

    /* video hardware */
	MCFG_CDP1861_SCREEN_ADD(CDP1861_TAG, SCREEN_TAG, 1760000)
	MCFG_CDP1861_ADD(CDP1861_TAG, 1760000, studio2_cdp1861_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(BEEPER_TAG, BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_FRAGMENT_ADD( studio2_cartslot )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( visicom, visicom_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(CDP1802_TAG, COSMAC, XTAL_3_579545MHz/2)
	MCFG_CPU_PROGRAM_MAP(visicom_map)
	MCFG_CPU_IO_MAP(visicom_io_map)
	MCFG_CPU_CONFIG(studio2_cosmac_intf)

    /* video hardware */
	MCFG_CDP1861_SCREEN_ADD(CDP1861_TAG, SCREEN_TAG, XTAL_3_579545MHz/2)
	MCFG_CDP1861_ADD(CDP1861_TAG, XTAL_3_579545MHz/2/8, studio2_cdp1861_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(BEEPER_TAG, BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_FRAGMENT_ADD( studio2_cartslot )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mpt02, mpt02_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(CDP1802_TAG, COSMAC, CDP1864_CLOCK)
	MCFG_CPU_PROGRAM_MAP(mpt02_map)
	MCFG_CPU_IO_MAP(mpt02_io_map)
	MCFG_CPU_CONFIG(mpt02_cosmac_intf)

    /* video hardware */
	MCFG_CDP1864_SCREEN_ADD(SCREEN_TAG, CDP1864_CLOCK)
	MCFG_SCREEN_UPDATE_DEVICE(CDP1864_TAG, cdp1864_device, screen_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(BEEPER_TAG, BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_CDP1864_ADD(CDP1864_TAG, CDP1864_CLOCK, mpt02_cdp1864_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_FRAGMENT_ADD( studio2_cartslot )
MACHINE_CONFIG_END

/* ROMs */

ROM_START( studio2 )
	ROM_REGION( 0x10000, CDP1802_TAG, 0 )
	ROM_LOAD( "84932.ic11", 0x0000, 0x0200, CRC(283b7e65) SHA1(4b6d21cde59712ecb5941ff63d8eb161420b0aac) )
	ROM_LOAD( "84933.ic12", 0x0200, 0x0200, CRC(a396b77c) SHA1(023517f67af61790e6916b6c4dbe2d9dc07ae3ff) )
	ROM_LOAD( "85456.ic13", 0x0400, 0x0200, CRC(d25cf97f) SHA1(d489f41f1125c76cc8ed9defa82a877ae014ef21) )
	ROM_LOAD( "85457.ic14", 0x0600, 0x0200, CRC(74aa724f) SHA1(085832f29e0d2a387c75463d66c54fb6c1e9e72c) )
ROM_END

ROM_START( visicom )
	ROM_REGION( 0x10000, CDP1802_TAG, 0 )
	ROM_LOAD( "visicom.q003", 0x0000, 0x0800, CRC(23d22074) SHA1(a0a8be23f70621a2bd8010b1134e8a0019075bf1) )
ROM_END

ROM_START( mpt02 )
	ROM_REGION( 0x10000, CDP1802_TAG, 0 )
	ROM_LOAD( "86676.ic13",  0x0000, 0x0400, CRC(a7d0dd3b) SHA1(e1881ab4d67a5d735dd2c8d7e924e41df6f2aeec) )
	ROM_LOAD( "86677b.ic14", 0x0400, 0x0400, CRC(82a2d29e) SHA1(37e02089d611db10bad070d89c8801de41521189) )
	ROM_LOAD( "87201.ic12",  0x0c00, 0x0400, CRC(8006a1e3) SHA1(b67612d98231485fce55d604915abd19b6d64eac) )
ROM_END

#define rom_mpt02h rom_mpt02
#define rom_mtc9016 rom_mpt02
#define rom_shmc1200 rom_mpt02
#define rom_cm1200 rom_mpt02
#define rom_apollo80 rom_mpt02

/* Driver Initialization */

TIMER_CALLBACK_MEMBER(studio2_state::setup_beep)
{
	device_t *speaker = machine().device(BEEPER_TAG);
	beep_set_state(speaker, 0);
	beep_set_frequency(speaker, 300);
}

DRIVER_INIT_MEMBER(studio2_state,studio2)
{
	machine().scheduler().timer_set(attotime::zero, timer_expired_delegate(FUNC(studio2_state::setup_beep),this));
}

/* Game Drivers */

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT        COMPANY   FULLNAME */
CONS( 1977,	studio2,	0,		0,		studio2,	studio2, studio2_state,	studio2,	"RCA",		"Studio II", GAME_SUPPORTS_SAVE )
CONS( 1978, visicom,	studio2,0,		visicom,	studio2, studio2_state,	studio2,	"Toshiba",	"Visicom COM-100 (Japan)", GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
CONS( 1978,	mpt02,		studio2,0,		mpt02,		studio2, studio2_state,	studio2,	"Soundic",	"Victory MPT-02 Home TV Programmer (Austria)", GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
CONS( 1978,	mpt02h,		studio2,0,		mpt02,		studio2, studio2_state,	studio2,	"Hanimex",	"MPT-02 Jeu TV Programmable (France)", GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE)
CONS( 1978,	mtc9016,	studio2,0,		mpt02,		studio2, studio2_state,	studio2,	"Mustang",	"9016 Telespiel Computer (Germany)", GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
CONS( 1978, shmc1200,	studio2,0,		mpt02,		studio2, studio2_state,	studio2,	"Sheen",	"1200 Micro Computer (Australia)", GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
CONS( 1978, cm1200,		studio2,0,		mpt02,		studio2, studio2_state,	studio2,	"Conic",	"M-1200 (?)", GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
CONS( 1978, apollo80,	studio2,0,		mpt02,		studio2, studio2_state,	studio2,	"Academy",	"Apollo 80 (Germany)", GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
