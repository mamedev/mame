// license:BSD-3-Clause
// copyright-holders:Curt Coder
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
                Pinout:
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


Cartridges
----------

Inside is a Toshiba TMM331AP ROM, which is pin compatible with the Signetics S6831.
The cartridge to TMM331 pin connections are as follows, with cartridge pin 1 being the leftmost angled contact:

Pin 1 to ROM pins 12,13 (GND and E2)
Pin 2 to ROM pins 24,15 (VCC and E0)
Pin 3 to ROM pin 23 (D0)
Pin 4 to ROM pin 22 (D1)
Pin 5 to ROM pin 21 (D2)
Pin 6 to ROM pin 20 (D3)
Pin 7 to ROM pin 19 (D4)
Pin 8 to ROM pin 18 (D5)
Pin 9 to ROM pin 17 (D6)
Pin 10 to ROM pin 16 (D7)
Pin 11 to ROM pin 14 (E1)
Pin 12 to ROM pin 11 (A6)
Pin 13 to ROM pin 10 (A5)
Pin 14 to ROM pin 9 (A4)
Pin 15 to ROM pin 8 (A3)
Pin 16 to ROM pin 7 (A2)
Pin 17 to ROM pin 6 (A1)
Pin 18 to ROM pin 5 (A0)
Pin 19 to ROM pin 1 (A7)
Pin 20 to ROM pin 4 (A10)
Pin 21 to ROM pin 3 (A9)
Pin 22 to ROM pin 2 (A8)

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

    TODO:

    - NE555 discrete sound

*/

#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "sound/beep.h"
#include "sound/cdp1864.h"
#include "sound/discrete.h"
#include "video/cdp1861.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

#define CDP1802_TAG     "ic1"
#define CDP1861_TAG     "ic2"
#define CDP1864_TAG     "cdp1864"
#define SCREEN_TAG      "screen"

class studio2_state : public driver_device
{
public:

	studio2_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, CDP1802_TAG),
			m_beeper(*this, "beeper"),
			m_vdc(*this, CDP1861_TAG),
			m_cart(*this, "cartslot"),
			m_clear(*this, "CLEAR"),
			m_a(*this, "A"),
			m_b(*this, "B"),
			m_screen(*this, "screen")
	{ }

	required_device<cosmac_device> m_maincpu;
	required_device<beep_device> m_beeper;
	optional_device<cdp1861_device> m_vdc;
	required_device<generic_slot_device> m_cart;
	required_ioport m_clear;
	required_ioport m_a;
	required_ioport m_b;
	required_device<screen_device> m_screen;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER( cart_400 );
	DECLARE_READ8_MEMBER( cart_a00 );
	DECLARE_READ8_MEMBER( cart_e00 );
	DECLARE_READ8_MEMBER( dispon_r );
	DECLARE_WRITE8_MEMBER( keylatch_w );
	DECLARE_WRITE8_MEMBER( dispon_w );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_READ_LINE_MEMBER( ef4_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_INPUT_CHANGED_MEMBER( reset_w );
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( studio2_cart_load );

	/* keyboard state */
	UINT8 m_keylatch;
	DECLARE_DRIVER_INIT(studio2);
};

class visicom_state : public studio2_state
{
public:
	visicom_state(const machine_config &mconfig, device_type type, std::string tag)
		: studio2_state(mconfig, type, tag),
			m_color0_ram(*this, "color0_ram"),
			m_color1_ram(*this, "color1_ram")
	{ }

	virtual UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_shared_ptr<UINT8> m_color0_ram;
	required_shared_ptr<UINT8> m_color1_ram;

	DECLARE_WRITE8_MEMBER( dma_w );
};

class mpt02_state : public studio2_state
{
public:
	mpt02_state(const machine_config &mconfig, device_type type, std::string tag)
		: studio2_state(mconfig, type, tag),
			m_cti(*this, CDP1864_TAG),
			m_color_ram(*this, "color_ram")
	{ }

	required_device<cdp1864_device> m_cti;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER( cart_c00 );
	DECLARE_WRITE8_MEMBER( dma_w );
	DECLARE_READ_LINE_MEMBER( rdata_r );
	DECLARE_READ_LINE_MEMBER( bdata_r );
	DECLARE_READ_LINE_MEMBER( gdata_r );

	/* video state */
	required_shared_ptr<UINT8> m_color_ram;
	UINT8 m_color;
};


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 0

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

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
	AM_RANGE(0x0800, 0x0fff) AM_DEVREAD("cartslot", generic_slot_device, read_rom)
	AM_RANGE(0x1000, 0x10ff) AM_RAM
	AM_RANGE(0x1100, 0x11ff) AM_RAM AM_SHARE("color0_ram")
	AM_RANGE(0x1300, 0x13ff) AM_RAM AM_SHARE("color1_ram")
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

static const rgb_t VISICOM_PALETTE[] =
{
	rgb_t(0x00, 0x40, 0x00),
	rgb_t(0xaf, 0xdf, 0xe4),
	rgb_t(0xb9, 0xc4, 0x2f),
	rgb_t(0xef, 0x45, 0x4a)
};

UINT32 visicom_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_vdc->screen_update(screen, bitmap, cliprect);

	m_vdc->m_bitmap.fill(VISICOM_PALETTE[0], cliprect);

	return 0;
}

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

/* CDP1802 Configuration */

READ_LINE_MEMBER( studio2_state::clear_r )
{
	return BIT(m_clear->read(), 0);
}

READ_LINE_MEMBER( studio2_state::ef3_r )
{
	return BIT(m_a->read(), m_keylatch);
}

READ_LINE_MEMBER( studio2_state::ef4_r )
{
	return BIT(m_b->read(), m_keylatch);
}

WRITE_LINE_MEMBER( studio2_state::q_w )
{
	m_beeper->set_state(state);
}

WRITE8_MEMBER( visicom_state::dma_w )
{
	int sx = m_screen->hpos() + 4;
	int y = m_screen->vpos();

	UINT8 addr = offset & 0xff;
	UINT8 color0 = m_color0_ram[addr];
	UINT8 color1 = m_color1_ram[addr];

	for (int x = 0; x < 8; x++)
	{
		int color = (BIT(color1, 7) << 1) | BIT(color0, 7);
		m_vdc->m_bitmap.pix32(y, sx + x) = VISICOM_PALETTE[color];
		color0 <<= 1;
		color1 <<= 1;
	}
}

WRITE8_MEMBER( mpt02_state::dma_w )
{
	UINT8 addr = ((offset & 0xe0) >> 2) | (offset & 0x07);

	m_color = m_color_ram[addr];

	m_cti->con_w(0); // HACK
	m_cti->dma_w(space, offset, data);
}

/* Machine Initialization */

// trampolines to cartridge
READ8_MEMBER( studio2_state::cart_400 ) { return m_cart->read_rom(space, offset); }
READ8_MEMBER( studio2_state::cart_a00 ) { return m_cart->read_rom(space, offset + 0x600); }
READ8_MEMBER( studio2_state::cart_e00 ) { return m_cart->read_rom(space, offset + 0xa00); }
READ8_MEMBER( mpt02_state::cart_c00 ) { return m_cart->read_rom(space, offset + 0x800); }

void studio2_state::machine_start()
{
	if (m_cart->exists())
	{
		// these have to be installed only if a cart is present, because they partially overlap the built-in game
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0400, 0x07ff, read8_delegate(FUNC(studio2_state::cart_400), this));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0a00, 0x0bff, read8_delegate(FUNC(studio2_state::cart_a00), this));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0e00, 0x0fff, read8_delegate(FUNC(studio2_state::cart_e00), this));
	}

	// register for state saving
	save_item(NAME(m_keylatch));
}

void mpt02_state::machine_start()
{
	if (m_cart->exists())
	{
		// these have to be installed only if a cart is present, because they partially overlap the built-in game
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0400, 0x07ff, read8_delegate(FUNC(studio2_state::cart_400), this));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0c00, 0x0fff, read8_delegate(FUNC(mpt02_state::cart_c00), this));
	}

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

DEVICE_IMAGE_LOAD_MEMBER( studio2_state, studio2_cart_load )
{
	UINT32 size;

	// always alloc 3K, even if range $400-$600 is not read by the system (RAM is present there)
	m_cart->rom_alloc(0xc00, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);

	if (image.software_entry() == nullptr)
	{
		if (!strcmp(image.filetype(), "st2"))
		{
			UINT8 header[0x100];
			UINT8 catalogue[10], title[32], pages[64];
			UINT8 blocks;

			if (image.length() <= 0x100)
			{
				image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid ROM file");
				return IMAGE_INIT_FAIL;
			}

			image.fread(&header, 0x100);

			// validate
			if (strncmp((const char *)header, "RCA2", 4))
			{
				image.seterror(IMAGE_ERROR_UNSPECIFIED, "Not an .ST2 file");
				return IMAGE_INIT_FAIL;
			}

			blocks = header[4];
			memcpy(&catalogue, &header[16], 10);
			memcpy(&title, &header[32], 32);
			memcpy(&pages, &header[64], 64);

			/* read ST2 cartridge into memory */
			for (int block = 0; block < (blocks - 1); block++)
			{
				if (pages[block] < 4)
					logerror("ST2 invalid block %u to %04x\n", block, pages[block] << 8);
				else
				{
					UINT16 offset = (pages[block] << 8) - 0x400;
					logerror("ST2 Reading block %u to %04x\n", block, offset);
					image.fread(m_cart->get_rom_base() + offset, 0x100);
				}
			}

			logerror("ST2 Catalogue: %s\n", catalogue);
			logerror("ST2 Title: %s\n", title);
		}
		else
		{
			size = image.length();
			if (size > 0x400)
			{
				image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
				return IMAGE_INIT_FAIL;
			}
			else
				image.fread(m_cart->get_rom_base(), size);
		}
	}
	else
	{
		// Studio II carts might map their data at $400-$7ff, $a00-$bff and $e00-$fff
		// MPT-2 carts might map their data at $400-$7ff and $c00-$fff
		if (image.get_software_region("rom_400"))
			memcpy(m_cart->get_rom_base() + 0x000, image.get_software_region("rom_400"), image.get_software_region_length("rom_400"));
		if (image.get_software_region("rom_a00"))
			memcpy(m_cart->get_rom_base() + 0x600, image.get_software_region("rom_a00"), image.get_software_region_length("rom_a00"));
		if (image.get_software_region("rom_c00"))
			memcpy(m_cart->get_rom_base() + 0x800, image.get_software_region("rom_c00"), image.get_software_region_length("rom_c00"));
		if (image.get_software_region("rom_e00"))
			memcpy(m_cart->get_rom_base() + 0xa00, image.get_software_region("rom_e00"), image.get_software_region_length("rom_e00"));
	}

	return IMAGE_INIT_PASS;
}


/* Machine Drivers */

static MACHINE_CONFIG_FRAGMENT( studio2_cartslot )
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "studio2_cart")
	MCFG_GENERIC_EXTENSIONS("st2,bin,rom")
	MCFG_GENERIC_LOAD(studio2_state, studio2_cart_load)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "studio2")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( studio2, studio2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, 1760000) /* the real clock is derived from an oscillator circuit */
	MCFG_CPU_PROGRAM_MAP(studio2_map)
	MCFG_CPU_IO_MAP(studio2_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(studio2_state, clear_r))
	MCFG_COSMAC_EF3_CALLBACK(READLINE(studio2_state, ef3_r))
	MCFG_COSMAC_EF4_CALLBACK(READLINE(studio2_state, ef4_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(studio2_state, q_w))
	MCFG_COSMAC_DMAW_CALLBACK(DEVWRITE8(CDP1861_TAG, cdp1861_device, dma_w))

	/* video hardware */
	MCFG_DEVICE_ADD(CDP1861_TAG, CDP1861, 1760000)
	MCFG_CDP1861_IRQ_CALLBACK(INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_INT))
	MCFG_CDP1861_DMA_OUT_CALLBACK(INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_DMAOUT))
	MCFG_CDP1861_EFX_CALLBACK(INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_EF1))
	MCFG_CDP1861_SCREEN_ADD(CDP1861_TAG, SCREEN_TAG, 1760000)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_FRAGMENT_ADD( studio2_cartslot )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( visicom, visicom_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, XTAL_3_579545MHz/2)
	MCFG_CPU_PROGRAM_MAP(visicom_map)
	MCFG_CPU_IO_MAP(visicom_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(visicom_state, clear_r))
	MCFG_COSMAC_EF3_CALLBACK(READLINE(visicom_state, ef3_r))
	MCFG_COSMAC_EF4_CALLBACK(READLINE(visicom_state, ef4_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(visicom_state, q_w))
	MCFG_COSMAC_DMAW_CALLBACK(WRITE8(visicom_state, dma_w))

	/* video hardware */
	MCFG_DEVICE_ADD(CDP1861_TAG, CDP1861, XTAL_3_579545MHz/2)
	MCFG_CDP1861_IRQ_CALLBACK(INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_INT))
	MCFG_CDP1861_DMA_OUT_CALLBACK(INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_DMAOUT))
	MCFG_CDP1861_EFX_CALLBACK(INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_EF1))
	MCFG_CDP1861_SCREEN_ADD(CDP1861_TAG, SCREEN_TAG, XTAL_3_579545MHz/2)
	MCFG_SCREEN_UPDATE_DRIVER(visicom_state, screen_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "visicom_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "visicom")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mpt02, mpt02_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, CDP1864_CLOCK)
	MCFG_CPU_PROGRAM_MAP(mpt02_map)
	MCFG_CPU_IO_MAP(mpt02_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(mpt02_state, clear_r))
	MCFG_COSMAC_EF3_CALLBACK(READLINE(mpt02_state, ef3_r))
	MCFG_COSMAC_EF4_CALLBACK(READLINE(mpt02_state, ef4_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(mpt02_state, q_w))
	MCFG_COSMAC_DMAW_CALLBACK(WRITE8(mpt02_state, dma_w))

	/* video hardware */
	MCFG_CDP1864_SCREEN_ADD(SCREEN_TAG, CDP1864_CLOCK)
	MCFG_SCREEN_UPDATE_DEVICE(CDP1864_TAG, cdp1864_device, screen_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_CDP1864_ADD(CDP1864_TAG, SCREEN_TAG, CDP1864_CLOCK, GND, INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_INT), INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_DMAOUT), INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_EF1), NULL, READLINE(mpt02_state, rdata_r), READLINE(mpt02_state, bdata_r), READLINE(mpt02_state, gdata_r))
	MCFG_CDP1864_CHROMINANCE(RES_K(4.7), RES_K(8.2), RES_K(4.7), RES_K(22))

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_FRAGMENT_ADD( studio2_cartslot )
MACHINE_CONFIG_END

/* ROMs */

ROM_START( studio2 )
	ROM_REGION( 0x1000, CDP1802_TAG, 0 )
	ROM_LOAD( "84932.ic11", 0x000, 0x200, CRC(283b7e65) SHA1(4b6d21cde59712ecb5941ff63d8eb161420b0aac) )
	ROM_LOAD( "84933.ic12", 0x200, 0x200, CRC(a396b77c) SHA1(023517f67af61790e6916b6c4dbe2d9dc07ae3ff) )
	ROM_LOAD( "85456.ic13", 0x400, 0x200, CRC(d25cf97f) SHA1(d489f41f1125c76cc8ed9defa82a877ae014ef21) )
	ROM_LOAD( "85457.ic14", 0x600, 0x200, CRC(74aa724f) SHA1(085832f29e0d2a387c75463d66c54fb6c1e9e72c) )
ROM_END

ROM_START( visicom )
	ROM_REGION( 0x1000, CDP1802_TAG, 0 )
	ROM_LOAD( "visicom.q003", 0x000, 0x800, CRC(23d22074) SHA1(a0a8be23f70621a2bd8010b1134e8a0019075bf1) )
ROM_END

ROM_START( mpt02 )
	ROM_REGION( 0x1000, CDP1802_TAG, 0 )
	ROM_LOAD( "86676.ic13",  0x000, 0x400, CRC(a7d0dd3b) SHA1(e1881ab4d67a5d735dd2c8d7e924e41df6f2aeec) )
	ROM_LOAD( "86677b.ic14", 0x400, 0x400, CRC(82a2d29e) SHA1(37e02089d611db10bad070d89c8801de41521189) )
	ROM_LOAD( "87201.ic12",  0xc00, 0x400, CRC(8006a1e3) SHA1(b67612d98231485fce55d604915abd19b6d64eac) )
ROM_END

ROM_START( mpt02h )
	ROM_REGION( 0x1000, CDP1802_TAG, 0 )
	ROM_LOAD( "86676.ic13",  0x000, 0x400, CRC(a7d0dd3b) SHA1(e1881ab4d67a5d735dd2c8d7e924e41df6f2aeec) )
ROM_END

#define rom_mtc9016 rom_mpt02
#define rom_shmc1200 rom_mpt02
#define rom_cm1200 rom_mpt02
#define rom_apollo80 rom_mpt02

/* Driver Initialization */

DRIVER_INIT_MEMBER(studio2_state,studio2)
{
	m_beeper->set_state(0);
	m_beeper->set_frequency(300);
}

/* Game Drivers */

//    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT    INIT                       COMPANY    FULLNAME                                         FLAGS
CONS( 1977, studio2,    0,      0,      studio2,    studio2, studio2_state, studio2,    "RCA",      "Studio II",                                    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
CONS( 1978, visicom,    studio2,0,      visicom,    studio2, studio2_state, studio2,    "Toshiba",  "Visicom COM-100 (Japan)",                      MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
CONS( 1978, mpt02,      studio2,0,      mpt02,      studio2, studio2_state, studio2,    "Soundic",  "Victory MPT-02 Home TV Programmer (Austria)",  MACHINE_SUPPORTS_SAVE )
CONS( 1978, mpt02h,     studio2,0,      mpt02,      studio2, studio2_state, studio2,    "Hanimex",  "MPT-02 Jeu TV Programmable (France)",          MACHINE_SUPPORTS_SAVE )
CONS( 1978, mtc9016,    studio2,0,      mpt02,      studio2, studio2_state, studio2,    "Mustang",  "9016 Telespiel Computer (Germany)",            MACHINE_SUPPORTS_SAVE )
CONS( 1978, shmc1200,   studio2,0,      mpt02,      studio2, studio2_state, studio2,    "Sheen",    "M1200 Micro Computer (Australia)",             MACHINE_SUPPORTS_SAVE )
CONS( 1978, cm1200,     studio2,0,      mpt02,      studio2, studio2_state, studio2,    "Conic",    "M-1200 (?)",                                   MACHINE_SUPPORTS_SAVE )
CONS( 1978, apollo80,   studio2,0,      mpt02,      studio2, studio2_state, studio2,    "Academy",  "Apollo 80 (Germany)",                          MACHINE_SUPPORTS_SAVE )
