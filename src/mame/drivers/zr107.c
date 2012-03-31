/*
    Konami 'ZR107' Hardware
    Konami, 1995-1996

    Driver by Ville Linde


    Hardware overview:

    ZR107 CPU board:
    ----------------
        IBM PowerPC 403GA at 32MHz (main CPU)
        Motorola MC68EC000 at 16MHz (sound CPU)
        Konami K056800 (MIRAC), sound system interface
        Konami K056230 (LANC), LAN interface
        Konami K058141 sound chip (same as 2x K054539)

    ZR107 GFX board:
    ----------------
        Analog Devices ADSP-21062 SHARC DSP at 36MHz
        Konami K056832 tilemap chip
        KS10071 (custom 3D pixel unit)
        KS10081 (custom 3D texel unit)

    GN678 GFX board (same as in gticlub.c):
    ----------------
        Analog Devices ADSP-21062 SHARC DSP at 36MHz
        Konami K001604 (2D tilemaps + 2x ROZ)
        2x KS10071 (custom 3D pixel unit)
        2x KS10081 (custom 3D texel unit)


    Known games on this hardware:

    Game                     | Year | ID    | CPU PCB | CG Board(s)
    --------------------------------------------------------------------------
    Midnight Run             | 1995 | GX476 | ZR107   | ZR107
    Winding Heat             | 1996 | GX677 | ZR107   | ZR107
    Jet Wave / Wave Shark    | 1996 | GX678 | ZR107   | GN678


PCB Layouts
-----------

Top Board

ZR107 PWB(A)300769A
|------------------------------------------------------------|
|                                         MASKROM.3R         |
|                             MASKROM.5N  MASKROM.5R         |
|056602  056800      058141    68EC000FN8   TSOP56   DIP42   |
| RESET_SW                                  TSOP56   DIP42   |
|058232                                                      |
|   PAL(001535) PAL(001536)  8464  8464           EPROM.13U  |
|                                                            |
|                                      SOJ40      EPROM.15U  |
|                                                            |
|                         DIP32        SOJ40      EPROM.17U  |
|                        EPROM.19L  18.432MHz                |
|        93C46.20E                                EPROM.20U  |
|               LED                   814260-70              |
|                         PAL(001534)                        |
|     ADC0838                                                |
|                         PAL(001533) 814260-70              |
| TEST_SW                                                    |
|                         PAL(001532)                        |
| PAL(056787A)                                               |
|                                                            |
|     8464                                     |--------|    |
|                                              |IBM     |    |
|                          64MHz               |POWERPC |    |
|     056230   QFP44                           |403GA   |    |
|                                              |--------|    |
| DSW(4)                                                     |
|------------------------------------------------------------|
Notes:
     403GA: clock 32.000MHz (64/2)
     68000: clock 8.000MHz (64/8)
    TSOP56: Unpopulated position for 2Mx8 TSOP56 FlashROM
     DIP42: Unpopulated position for 2Mx8 DIP42 MASKROM
     DIP32: Unpopulated position for 512kx8 EPROM
     SOJ40: Unpopulated position for DRAM 814260-70
     QFP44: Unpopulated position for MB89371FL
    056230: Konami custom, also marked KS40011, used for network functions
    058141: Konami custom
    056800: Konami custom
    058232: Konami custom filter/DAC?
    056602: Konami custom sound ceramic module (contains a small IC, some OP amps, resistors, caps etc)
      8464: 8kx8 SRAM (NDIP28)
    814260: 256kx16 DRAM (SOJ40)
       LED: 2 digit alpha-numeric 7-segment LED

ROM Usage
---------
                            |-------------------------- ROM Locations -------------------------------|
Game                        5R      3R      5N      13U        15U        17U        20U        19L
------------------------------------------------------------------------------------------------------
Midnight Run                477A08  477A09  477A10  476EA1A04  476EA1A03  476EA1A02  476EA1A01  477A07
Jet Wave                    678A08  678A09  678A10  678UAB04   678UAB03   678UAB02   678UAB01   678A07
Winding Heat                677A08  677A09  677A10  677UBC04   677UBC03   677UBC02   677UBC01   677A07


Bottom Board

ZR107  PWB(B)300816D
|------------------------------------------------------------|
|                          |-------|                         |
| DIP42     MASKROM.2H     |KS10081| 81141622  81141622      |
|                          |-------|                         |
| DIP42     MASKROM.5H              |---------|   81141622   |
|                                   | KS10071 |              |
| DIP42     MASKROM.7H              |         |   81141622   |
|                                   |---------|              |
| DIP42     MASKROM.9H               81141622                |
|                                                            |
|                           MC88916                          |
| PAL(001785)                  AM7203  AM7203  AM7203 AM7203 |
|                                                            |
|                                                            |
|                                         PAL(001782)        |
|                                                 PAL(001781)|
|                                                            |
|                                                            |
| MC44200   CY7C128  CY7C128  CY7C199  CY7C199               |
|                             CY7C199  CY7C199  36MHz        |
| MACH110                                                    |
|(001779)    056832                           |---------|    |
|                                             |ADSP21062|    |
|   PAL(001784) 058143                        |SHARC    |    |
|MASKROM.35A                MACH110           |KS-160   |    |
|            62256  62256  (001780)           |---------|    |
|MASKROM.35B      62256                     CY7C109  CY7C109 |
|                         DSW(4)            CY7C109  CY7C109 |
|------------------------------------------------------------|
Notes:
      KS10081 : Konami custom video chip, also marked 001006
      KS10071 : Konami custom video chip, also marked 001005. Chip is heatsinked
      056832  : Konami custom
      058143  : Konami custom
      AM7203  : AMD AM7203 2kx9 FIFO (PLCC32)
      MACH110 : MACH110 CPLD stamped 001779 & 001780
      DSW(4)  : 4 position DIP SWITCH
      PAL     : PALCE16V8H stamped 001781, 001782, 001784, 001785
      81141622: 256kx16 SDRAM
      CY7C128 : 2kx8 SRAM
      CY7C199 : 32kx8 SRAM
      CY7C109 : 128kx8 SRAM
      62256   : 32kx8 SRAM
      DIP42   : Unpopulated position for 1Mx8 DIP42 MASKROM
      MC88916 : Motorola MC88916 Low Skew CMOS PLL Clock Driver

ROM Usage
---------
                 |--------------- ROM Locations ---------------|
Game             35A     35B     2H      5H      7H      9H
---------------------------------------------------------------
Midnight Run     477A12  477A11  477A16  477A15  477A14  477A13
Jet Wave         - see note -
Winding Heat     677A12  677A11  677A16  677A15  677A14  677A13

Note: Jet Wave uses the lower board from GTI Club (GN678), and a ZR107(PWB(A)300769A top board.
Check gticlub.c for details on the bottom board.

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/sharc/sharc.h"
#include "machine/konppc.h"
#include "machine/adc083x.h"
#include "machine/k056230.h"
#include "machine/eeprom.h"
#include "sound/k056800.h"
#include "sound/k054539.h"
#include "video/konicdev.h"
#include "video/gticlub.h"


class zr107_state : public driver_device
{
public:
	zr107_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_led_reg0;
	UINT8 m_led_reg1;
	int m_ccu_vcth;
	int m_ccu_vctl;
	UINT32 *m_workram;
	UINT32 *m_sharc_dataram;
};





static VIDEO_START( jetwave )
{
	K001005_init(machine);
	K001006_init(machine);
}


static SCREEN_UPDATE_RGB32( jetwave )
{
	zr107_state *state = screen.machine().driver_data<zr107_state>();
	device_t *k001604 = screen.machine().device("k001604");

	bitmap.fill(screen.machine().pens[0], cliprect);

	K001005_draw(bitmap, cliprect);

	k001604_draw_front_layer(k001604, bitmap, cliprect);

	draw_7segment_led(bitmap, 3, 3, state->m_led_reg0);
	draw_7segment_led(bitmap, 9, 3, state->m_led_reg1);

	sharc_set_flag_input(screen.machine().device("dsp"), 1, ASSERT_LINE);
	return 0;
}


/*****************************************************************************/

static WRITE32_HANDLER( paletteram32_w )
{
	COMBINE_DATA(&space->machine().generic.paletteram.u32[offset]);
	data = space->machine().generic.paletteram.u32[offset];
	palette_set_color_rgb(space->machine(), (offset * 2) + 0, pal5bit(data >> 26), pal5bit(data >> 21), pal5bit(data >> 16));
	palette_set_color_rgb(space->machine(), (offset * 2) + 1, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

#define NUM_LAYERS	2

static void game_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags)
{
	*color += layer * 0x40;
}

static VIDEO_START( zr107 )
{
	device_t *k056832 = machine.device("k056832");

	k056832_set_layer_offs(k056832, 0, -29, -27);
	k056832_set_layer_offs(k056832, 1, -29, -27);
	k056832_set_layer_offs(k056832, 2, -29, -27);
	k056832_set_layer_offs(k056832, 3, -29, -27);
	k056832_set_layer_offs(k056832, 4, -29, -27);
	k056832_set_layer_offs(k056832, 5, -29, -27);
	k056832_set_layer_offs(k056832, 6, -29, -27);
	k056832_set_layer_offs(k056832, 7, -29, -27);

	K001006_init(machine);
	K001005_init(machine);
}

static SCREEN_UPDATE_RGB32( zr107 )
{
	zr107_state *state = screen.machine().driver_data<zr107_state>();
	device_t *k056832 = screen.machine().device("k056832");
	bitmap.fill(screen.machine().pens[0], cliprect);

	k056832_tilemap_draw(k056832, bitmap, cliprect, 1, 0, 0);
	K001005_draw(bitmap, cliprect);
	k056832_tilemap_draw(k056832, bitmap, cliprect, 0, 0, 0);

	draw_7segment_led(bitmap, 3, 3, state->m_led_reg0);
	draw_7segment_led(bitmap, 9, 3, state->m_led_reg1);

	sharc_set_flag_input(screen.machine().device("dsp"), 1, ASSERT_LINE);
	return 0;
}

/******************************************************************/

static READ8_HANDLER( sysreg_r )
{
	UINT32 r = 0;
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3", "IN4" };

	switch (offset)
	{
		case 0:	/* I/O port 0 */
		case 1:	/* I/O port 1 */
		case 2:	/* I/O port 2 */
		case 3:	/* System Port 0 */
		case 4:	/* System Port 1 */
			r = input_port_read(space->machine(), portnames[offset]);
			break;

		case 5:	/* Parallel data port */
			break;
	}
	return r;
}

static WRITE8_HANDLER( sysreg_w )
{
	zr107_state *state = space->machine().driver_data<zr107_state>();
	switch (offset)
	{
		case 0:	/* LED Register 0 */
			state->m_led_reg0 = data;
			break;

		case 1:	/* LED Register 1 */
			state->m_led_reg1 = data;
			break;

		case 2: /* Parallel data register */
			mame_printf_debug("Parallel data = %02X\n", data);
			break;

		case 3:	/* System Register 0 */
			/*
                0x80 = unused
                0x40 = COINREQ1
                0x20 = COINREQ2
                0x10 = SNDRES
                0x08 = unused
                0x04 = EEPCS
                0x02 = EEPCLK
                0x01 = EEPDI
            */
			input_port_write(space->machine(), "EEPROMOUT", data & 0x07, 0xff);
			cputag_set_input_line(space->machine(), "audiocpu", INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
			mame_printf_debug("System register 0 = %02X\n", data);
			break;

		case 4:	/* System Register 1 */
			/*
                0x80 = EXRES1
                0x40 = EXRES0
                0x20 = EXID1
                0x10 = EXID0
                0x08 = unused
                0x04 = ADCS (ADC CS)
                0x02 = ADDI (ADC DI)
                0x01 = ADDSCLK (ADC SCLK)
            */
			if (data & 0x80)	/* CG Board 1 IRQ Ack */
				cputag_set_input_line(space->machine(), "maincpu", INPUT_LINE_IRQ1, CLEAR_LINE);
			if (data & 0x40)	/* CG Board 0 IRQ Ack */
				cputag_set_input_line(space->machine(), "maincpu", INPUT_LINE_IRQ0, CLEAR_LINE);
			set_cgboard_id((data >> 4) & 3);
			input_port_write(space->machine(), "OUT4", data, 0xff);
			mame_printf_debug("System register 1 = %02X\n", data);
			break;

		case 5:	/* System Register 2 */
			/*
                0x01 = AFE
            */
			if (data & 0x01)
				watchdog_reset(space->machine());
			break;

	}
}

static READ32_HANDLER( ccu_r )
{
	zr107_state *state = space->machine().driver_data<zr107_state>();
	UINT32 r = 0;
	switch (offset)
	{
		case 0x1c/4:
		{
			// Midnight Run polls the vertical counter in vblank
			if (ACCESSING_BITS_24_31)
			{
				state->m_ccu_vcth ^= 0xff;
				r |= state->m_ccu_vcth << 24;
			}
			if (ACCESSING_BITS_8_15)
			{
				state->m_ccu_vctl++;
				state->m_ccu_vctl &= 0x1ff;
				r |= (state->m_ccu_vctl >> 2) << 8;
			}
		}
	}

	return r;
}

static WRITE32_HANDLER( ccu_w )
{

}

/******************************************************************/

static MACHINE_START( zr107 )
{
	zr107_state *state = machine.driver_data<zr107_state>();
	/* set conservative DRC options */
	ppcdrc_set_options(machine.device("maincpu"), PPCDRC_COMPATIBLE_OPTIONS);

	/* configure fast RAM regions for DRC */
	ppcdrc_add_fastram(machine.device("maincpu"), 0x00000000, 0x000fffff, FALSE, state->m_workram);
}

static ADDRESS_MAP_START( zr107_map, AS_PROGRAM, 32, zr107_state )
	AM_RANGE(0x00000000, 0x000fffff) AM_RAM	AM_BASE_MEMBER(zr107_state, m_workram)	/* Work RAM */
	AM_RANGE(0x74000000, 0x74003fff) AM_DEVREADWRITE("k056832", k056832_ram_long_r, k056832_ram_long_w)
	AM_RANGE(0x74020000, 0x7402003f) AM_DEVREADWRITE("k056832", k056832_long_r, k056832_long_w)
	AM_RANGE(0x74060000, 0x7406003f) AM_READWRITE(ccu_r, ccu_w)
	AM_RANGE(0x74080000, 0x74081fff) AM_RAM_WRITE(paletteram32_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x740a0000, 0x740a3fff) AM_DEVREAD("k056832", k056832_rom_long_r)
	AM_RANGE(0x78000000, 0x7800ffff) AM_READWRITE(cgboard_dsp_shared_r_ppc, cgboard_dsp_shared_w_ppc)		/* 21N 21K 23N 23K */
	AM_RANGE(0x78010000, 0x7801ffff) AM_WRITE(cgboard_dsp_shared_w_ppc)
	AM_RANGE(0x78040000, 0x7804000f) AM_READWRITE(K001006_0_r, K001006_0_w)
	AM_RANGE(0x780c0000, 0x780c0007) AM_READWRITE(cgboard_dsp_comm_r_ppc, cgboard_dsp_comm_w_ppc)
	AM_RANGE(0x7e000000, 0x7e003fff) AM_READWRITE8(sysreg_r, sysreg_w, 0xffffffff)
	AM_RANGE(0x7e008000, 0x7e009fff) AM_DEVREADWRITE8("k056230", k056230_r, k056230_w, 0xffffffff)				/* LANC registers */
	AM_RANGE(0x7e00a000, 0x7e00bfff) AM_DEVREADWRITE("k056230", lanc_ram_r, lanc_ram_w)		/* LANC Buffer RAM (27E) */
	AM_RANGE(0x7e00c000, 0x7e00c007) AM_DEVWRITE("k056800", k056800_host_w)
	AM_RANGE(0x7e00c008, 0x7e00c00f) AM_DEVREAD("k056800", k056800_host_r)
	AM_RANGE(0x7f800000, 0x7f9fffff) AM_ROM AM_SHARE("share2")
	AM_RANGE(0x7fe00000, 0x7fffffff) AM_ROM AM_REGION("user1", 0) AM_SHARE("share2")	/* Program ROM */
ADDRESS_MAP_END


static WRITE32_HANDLER( jetwave_palette_w )
{
	COMBINE_DATA(&space->machine().generic.paletteram.u32[offset]);
	data = space->machine().generic.paletteram.u32[offset];
	palette_set_color_rgb(space->machine(), offset, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

static ADDRESS_MAP_START( jetwave_map, AS_PROGRAM, 32, zr107_state )
	AM_RANGE(0x00000000, 0x000fffff) AM_MIRROR(0x80000000) AM_RAM		/* Work RAM */
	AM_RANGE(0x74000000, 0x740000ff) AM_MIRROR(0x80000000) AM_DEVREADWRITE("k001604", k001604_reg_r, k001604_reg_w)
	AM_RANGE(0x74010000, 0x7401ffff) AM_MIRROR(0x80000000) AM_RAM_WRITE(jetwave_palette_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x74020000, 0x7403ffff) AM_MIRROR(0x80000000) AM_DEVREADWRITE("k001604", k001604_tile_r, k001604_tile_w)
	AM_RANGE(0x74040000, 0x7407ffff) AM_MIRROR(0x80000000) AM_DEVREADWRITE("k001604", k001604_char_r, k001604_char_w)
	AM_RANGE(0x78000000, 0x7800ffff) AM_MIRROR(0x80000000) AM_READWRITE(cgboard_dsp_shared_r_ppc, cgboard_dsp_shared_w_ppc)		/* 21N 21K 23N 23K */
	AM_RANGE(0x78010000, 0x7801ffff) AM_MIRROR(0x80000000) AM_WRITE(cgboard_dsp_shared_w_ppc)
	AM_RANGE(0x78040000, 0x7804000f) AM_MIRROR(0x80000000) AM_READWRITE(K001006_0_r, K001006_0_w)
	AM_RANGE(0x78080000, 0x7808000f) AM_MIRROR(0x80000000) AM_READWRITE(K001006_1_r, K001006_1_w)
	AM_RANGE(0x780c0000, 0x780c0007) AM_MIRROR(0x80000000) AM_READWRITE(cgboard_dsp_comm_r_ppc, cgboard_dsp_comm_w_ppc)
	AM_RANGE(0x7e000000, 0x7e003fff) AM_MIRROR(0x80000000) AM_READWRITE8(sysreg_r, sysreg_w, 0xffffffff)
	AM_RANGE(0x7e008000, 0x7e009fff) AM_MIRROR(0x80000000) AM_DEVREADWRITE8("k056230", k056230_r, k056230_w, 0xffffffff)				/* LANC registers */
	AM_RANGE(0x7e00a000, 0x7e00bfff) AM_MIRROR(0x80000000) AM_DEVREADWRITE("k056230", lanc_ram_r, lanc_ram_w)	/* LANC Buffer RAM (27E) */
	AM_RANGE(0x7e00c000, 0x7e00c007) AM_MIRROR(0x80000000) AM_DEVWRITE("k056800", k056800_host_w)
	AM_RANGE(0x7e00c008, 0x7e00c00f) AM_MIRROR(0x80000000) AM_DEVREAD("k056800", k056800_host_r)
	AM_RANGE(0x7f000000, 0x7f3fffff) AM_MIRROR(0x80000000) AM_ROM AM_REGION("user2", 0)
	AM_RANGE(0x7f800000, 0x7f9fffff) AM_MIRROR(0x80000000) AM_ROM AM_SHARE("share2")
	AM_RANGE(0x7fe00000, 0x7fffffff) AM_MIRROR(0x80000000) AM_ROM AM_REGION("user1", 0) AM_SHARE("share2")	/* Program ROM */
ADDRESS_MAP_END



/**********************************************************************/

static ADDRESS_MAP_START( sound_memmap, AS_PROGRAM, 16, zr107_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM		/* Work RAM */
	AM_RANGE(0x200000, 0x2004ff) AM_DEVREADWRITE8_MODERN("konami1", k054539_device, read, write, 0xff00)
	AM_RANGE(0x200000, 0x2004ff) AM_DEVREADWRITE8_MODERN("konami2", k054539_device, read, write, 0x00ff)
	AM_RANGE(0x400000, 0x40000f) AM_DEVWRITE("k056800", k056800_sound_w)
	AM_RANGE(0x400010, 0x40001f) AM_DEVREAD("k056800", k056800_sound_r)
	AM_RANGE(0x580000, 0x580001) AM_WRITENOP
ADDRESS_MAP_END

static const k054539_interface k054539_config =
{
	"shared"
};

/*****************************************************************************/


static READ32_HANDLER( dsp_dataram_r )
{
	zr107_state *state = space->machine().driver_data<zr107_state>();
	return state->m_sharc_dataram[offset] & 0xffff;
}

static WRITE32_HANDLER( dsp_dataram_w )
{
	zr107_state *state = space->machine().driver_data<zr107_state>();
	state->m_sharc_dataram[offset] = data;
}

static ADDRESS_MAP_START( sharc_map, AS_DATA, 32, zr107_state )
	AM_RANGE(0x400000, 0x41ffff) AM_READWRITE(cgboard_0_shared_sharc_r, cgboard_0_shared_sharc_w)
	AM_RANGE(0x500000, 0x5fffff) AM_READWRITE(dsp_dataram_r, dsp_dataram_w)
	AM_RANGE(0x600000, 0x6fffff) AM_READWRITE(K001005_r, K001005_w)
	AM_RANGE(0x700000, 0x7000ff) AM_READWRITE(cgboard_0_comm_sharc_r, cgboard_0_comm_sharc_w)
ADDRESS_MAP_END

/*****************************************************************************/


static INPUT_PORTS_START( zr107 )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("View Button")		// View switch
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Shift Up")		// Shift up
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Shift Down")		// Shift down
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("AT/MT Switch")	PORT_TOGGLE	// AT/MT switch
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x0b, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("adc0838", adc083x_do_read)

	PORT_START("IN4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* PARAACK */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("adc0838",adc083x_sars_read)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom",eeprom_device,read_bit)

	PORT_START("EEPROMOUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_clock_line)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_cs_line)

	PORT_START("OUT4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("adc0838", adc083x_cs_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("adc0838", adc083x_di_write)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("adc0838", adc083x_clk_write)
INPUT_PORTS_END

static INPUT_PORTS_START( midnrun )
	PORT_INCLUDE( zr107 )

	PORT_START("IN3")
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // COIN2?
	PORT_DIPNAME( 0x0c, 0x00, "Network ID" ) PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING( 0x0c, "1" )
	PORT_DIPSETTING( 0x08, "2" )
	PORT_DIPSETTING( 0x04, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0x02, 0x02, "Transmission Type" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x02, "Button" )
	PORT_DIPSETTING( 0x00, "'T'Gate" )
	PORT_DIPNAME( 0x01, 0x01, "CG Board Type" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x01, DEF_STR( Single ) )
	PORT_DIPSETTING( 0x00, "Twin" )

	PORT_START("ANALOG1")		// Steering wheel
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG2")		// Acceleration pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG3")		// Brake pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

INPUT_PORTS_END

static INPUT_PORTS_START( windheat )
	PORT_INCLUDE( zr107 )

	PORT_START("IN3")
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // COIN2?
	PORT_DIPNAME( 0x0c, 0x00, "Network ID" ) PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING( 0x0c, "1" )
	PORT_DIPSETTING( 0x08, "2" )
	PORT_DIPSETTING( 0x04, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0x02, 0x02, "Transmission Type" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x02, "Button" )
	PORT_DIPSETTING( 0x00, "'T'Gate" )
	PORT_DIPNAME( 0x01, 0x01, "CG Board Type" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x01, DEF_STR( Single ) )
	PORT_DIPSETTING( 0x00, "Twin" )

	PORT_START("ANALOG1")		// Steering wheel
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG2")		// Acceleration pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG3")		// Brake pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

INPUT_PORTS_END

static INPUT_PORTS_START( jetwave )
	PORT_INCLUDE( zr107 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("View Shift")		// View Shift
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("T-Center")		// T-Center
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_BUTTON3 ) PORT_NAME("Angle")			// Angle
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Left Turn")		// Left Turn
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Right Turn")		// Right Turn
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x08, 0x00, "DIP 1" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP 2" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP 3" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x00, "DIP 4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("ANALOG1")		// Steering wheel
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("ANALOG2")		// Acceleration pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG3")		// Brake pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

INPUT_PORTS_END

static const sharc_config sharc_cfg =
{
	BOOT_MODE_EPROM
};


/* ADC0838 Interface */

static double adc0838_callback( device_t *device, UINT8 input )
{
	switch (input)
	{
	case ADC083X_CH0:
		return (double)(5 * input_port_read(device->machine(), "ANALOG1")) / 255.0;
	case ADC083X_CH1:
		return (double)(5 * input_port_read(device->machine(), "ANALOG2")) / 255.0;
	case ADC083X_CH2:
		return (double)(5 * input_port_read(device->machine(), "ANALOG3")) / 255.0;
	case ADC083X_CH3:
		return 0;
	case ADC083X_COM:
		return 0;
	case ADC083X_AGND:
		return 0;
	case ADC083X_VREF:
		return 5;
	}
	return 0;
}


static const adc083x_interface zr107_adc_interface = {
	adc0838_callback
};


static TIMER_CALLBACK( irq_off )
{
	cputag_set_input_line(machine, "audiocpu", param, CLEAR_LINE);
}

static void sound_irq_callback( running_machine &machine, int irq )
{
	int line = (irq == 0) ? INPUT_LINE_IRQ1 : INPUT_LINE_IRQ2;

	cputag_set_input_line(machine, "audiocpu", line, ASSERT_LINE);
	machine.scheduler().timer_set(attotime::from_usec(1), FUNC(irq_off), line);
}

static const k056800_interface zr107_k056800_interface =
{
	sound_irq_callback
};

static const k056832_interface zr107_k056832_intf =
{
	"gfx2", 1,
	K056832_BPP_8,
	1, 0,
	KONAMI_ROM_DEINTERLEAVE_NONE,
	game_tile_callback, "none"
};

static const k056230_interface zr107_k056230_intf =
{
	"maincpu",
	0
};

/* PowerPC interrupts

    IRQ0:  Vblank
    IRQ2:  LANC
    DMA0

*/
static INTERRUPT_GEN( zr107_vblank )
{
	device_set_input_line(device, INPUT_LINE_IRQ0, ASSERT_LINE);
}

static MACHINE_RESET( zr107 )
{
	cputag_set_input_line(machine, "dsp", INPUT_LINE_RESET, ASSERT_LINE);
}

static MACHINE_CONFIG_START( zr107, zr107_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC403GA, 64000000/2)	/* PowerPC 403GA 32MHz */
	MCFG_CPU_PROGRAM_MAP(zr107_map)
	MCFG_CPU_VBLANK_INT("screen", zr107_vblank)

	MCFG_CPU_ADD("audiocpu", M68000, 64000000/8)	/* 8MHz */
	MCFG_CPU_PROGRAM_MAP(sound_memmap)

	MCFG_CPU_ADD("dsp", ADSP21062, 36000000)
	MCFG_CPU_CONFIG(sharc_cfg)
	MCFG_CPU_DATA_MAP(sharc_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(30000))

	MCFG_EEPROM_93C46_ADD("eeprom")
	MCFG_MACHINE_START(zr107)
	MCFG_MACHINE_RESET(zr107)

	MCFG_K056230_ADD("k056230", zr107_k056230_intf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(64*8, 48*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 48*8-1)
	MCFG_SCREEN_UPDATE_STATIC(zr107)

	MCFG_PALETTE_LENGTH(65536)

	MCFG_VIDEO_START(zr107)

	MCFG_K056832_ADD("k056832", zr107_k056832_intf)

	MCFG_K056800_ADD("k056800", zr107_k056800_interface)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_K054539_ADD("konami1", 48000, k054539_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.75)

	MCFG_K054539_ADD("konami2", 48000, k054539_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.75)

	MCFG_ADC0838_ADD("adc0838", zr107_adc_interface)
MACHINE_CONFIG_END


static const k001604_interface jetwave_k001604_intf =
{
	0, 1,	/* gfx index 1 & 2 */
	0, 1,		/* layer_size, roz_size */
	0		/* slrasslt hack */
};

static MACHINE_CONFIG_START( jetwave, zr107_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC403GA, 64000000/2)	/* PowerPC 403GA 32MHz */
	MCFG_CPU_PROGRAM_MAP(jetwave_map)
	MCFG_CPU_VBLANK_INT("screen", zr107_vblank)

	MCFG_CPU_ADD("audiocpu", M68000, 64000000/8)	/* 8MHz */
	MCFG_CPU_PROGRAM_MAP(sound_memmap)

	MCFG_CPU_ADD("dsp", ADSP21062, 36000000)
	MCFG_CPU_CONFIG(sharc_cfg)
	MCFG_CPU_DATA_MAP(sharc_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(30000))

	MCFG_EEPROM_93C46_ADD("eeprom")
	MCFG_MACHINE_START(zr107)
	MCFG_MACHINE_RESET(zr107)

	MCFG_K056230_ADD("k056230", zr107_k056230_intf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(64*8, 48*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 48*8-1)
	MCFG_SCREEN_UPDATE_STATIC(jetwave)

	MCFG_PALETTE_LENGTH(65536)

	MCFG_VIDEO_START(jetwave)

	MCFG_K001604_ADD("k001604", jetwave_k001604_intf)

	MCFG_K056800_ADD("k056800", zr107_k056800_interface)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("konami1", K054539, 48000)
	MCFG_SOUND_CONFIG(k054539_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.75)

	MCFG_SOUND_ADD("konami2", K054539, 48000)
	MCFG_SOUND_CONFIG(k054539_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.75)

	MCFG_ADC0838_ADD("adc0838", zr107_adc_interface)
MACHINE_CONFIG_END

/*****************************************************************************/

static void init_zr107(running_machine &machine)
{
	zr107_state *state = machine.driver_data<zr107_state>();
	state->m_sharc_dataram = auto_alloc_array(machine, UINT32, 0x100000/4);
	state->m_led_reg0 = state->m_led_reg1 = 0x7f;
	state->m_ccu_vcth = state->m_ccu_vctl = 0;

	K001005_preprocess_texture_data(machine.region("gfx1")->base(), machine.region("gfx1")->bytes(), 0);
}

static DRIVER_INIT(zr107)
{
	init_konami_cgboard(machine, 1, CGBOARD_TYPE_ZR107);
	init_zr107(machine);
}

static DRIVER_INIT(jetwave)
{
	init_konami_cgboard(machine, 1, CGBOARD_TYPE_GTICLUB);
	init_zr107(machine);
}

/*****************************************************************************/

ROM_START( midnrun )
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE( "476ea1a01.20u", 0x000003, 0x80000, CRC(ea70edf2) SHA1(51c882383a150ba118ccd39eb869525fcf5eee3c) )
	ROM_LOAD32_BYTE( "476ea1a02.17u", 0x000002, 0x80000, CRC(1462994f) SHA1(c8614c6c416f81737cc77c46eea6d8d440bc8cf3) )
	ROM_LOAD32_BYTE( "476ea1a03.15u", 0x000001, 0x80000, CRC(b770ae46) SHA1(c61daa8353802957eb1c2e2c6204c3a98569627e) )
	ROM_LOAD32_BYTE( "476ea1a04.13u", 0x000000, 0x80000, CRC(9644b277) SHA1(b9cb812b6035dfd93032d277c8aa0037cf6b3dbe) )

	ROM_REGION(0x20000, "audiocpu", 0)		/* M68K program */
	ROM_LOAD16_WORD_SWAP( "477a07.19l", 0x000000, 0x20000, CRC(a82c0ba1) SHA1(dad69f2e5e75009d70cc2748477248ec47627c30) )

	ROM_REGION(0x100000, "gfx2", 0)	/* Tilemap */
	ROM_LOAD16_BYTE( "477a11.35b", 0x000000, 0x80000, CRC(85eef04b) SHA1(02e26d2d4a8b29894370f28d2a49fdf5c7d23f95) )
	ROM_LOAD16_BYTE( "477a12.35a", 0x000001, 0x80000, CRC(451d7777) SHA1(0bf280ca475100778bbfd3f023547bf0413fc8b7) )

	ROM_REGION(0x800000, "gfx1", 0)	/* Texture data */
	ROM_LOAD32_BYTE( "477a13.9h", 0x000000, 0x200000, CRC(b1ee901d) SHA1(b1432cb1379b35d99d3f2b7f6409db6f7e88121d) )
	ROM_LOAD32_BYTE( "477a14.7h",  0x000001, 0x200000, CRC(9ffa8cc5) SHA1(eaa19e26df721bec281444ca1c5ccc9e48df1b0b) )
	ROM_LOAD32_BYTE( "477a15.5h",  0x000002, 0x200000, CRC(e337fce7) SHA1(c84875f3275efd47273508b340231721f5a631d2) )
	ROM_LOAD32_BYTE( "477a16.2h", 0x000003, 0x200000, CRC(2c03ee63) SHA1(6b74d340dddf92bb4e4b1e037f003d58c65d8d9b) )

	ROM_REGION(0x600000, "shared", 0)	/* Sound data */
	ROM_LOAD( "477a08.5r", 0x000000, 0x200000, CRC(d320dbde) SHA1(eb602cad6ac7c7151c9f29d39b10041d5a354164) )
	ROM_LOAD( "477a09.3r", 0x200000, 0x200000, CRC(f431e29f) SHA1(e6082d88f86abb63d02ac34e70873b58f88b0ddc) )
	ROM_LOAD( "477a10.5n", 0x400000, 0x200000, CRC(8db31bd4) SHA1(d662d3bb6e8b44a01ffa158f5d7425454aad49a3) )
ROM_END

ROM_START( windheat )
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE( "677eaa01.20u", 0x000003, 0x080000, CRC(500b61f4) SHA1(ec39165412978c0dbd3cbf1f7b6989b5d7ba20a0) ) /* Program version EAA, Euro v2.11 */
	ROM_LOAD32_BYTE( "677eaa02.17u", 0x000002, 0x080000, CRC(99f9fd3b) SHA1(aaec5d7f4e46648aab3738ab09e46b312caee58f) )
	ROM_LOAD32_BYTE( "677eaa03.15u", 0x000001, 0x080000, CRC(c46eba6b) SHA1(80fea082d09071875d30a6a838736cf3a3e4501d) )
	ROM_LOAD32_BYTE( "677eaa04.13u", 0x000000, 0x080000, CRC(20dfcf4e) SHA1(4de8e22507f4719441f14fe96e25f0e0712dfa95) )

	ROM_REGION(0x20000, "audiocpu", 0)		/* M68K program */
	ROM_LOAD16_WORD_SWAP( "677a07.19l", 0x000000, 0x020000, CRC(05b14f2d) SHA1(3753f71173594ee741980e08eed0f7c3fc3588c9) )

	ROM_REGION(0x100000, "gfx2", 0)	/* Tilemap */
	ROM_LOAD16_BYTE( "677a11.35b", 0x000000, 0x080000, CRC(bf34f00f) SHA1(ca0d390c8b30d0cfdad4cfe5a601cc1f6e8c263d) )
	ROM_LOAD16_BYTE( "677a12.35a", 0x000001, 0x080000, CRC(458f0b1d) SHA1(8e11023c75c80b496dfc62b6645cfedcf2a80db4) )

	ROM_REGION(0x800000, "gfx1", 0)	/* Texture data */
	ROM_LOAD32_BYTE( "677a13.9h", 0x000000, 0x200000, CRC(7937d226) SHA1(c2ba777292c293e31068eeb3a27353ad2595b413) )
	ROM_LOAD32_BYTE( "677a14.7h", 0x000001, 0x200000, CRC(2568cf41) SHA1(6ed01922943486dafbdc863b76b2036c1fbe5281) )
	ROM_LOAD32_BYTE( "677a15.5h", 0x000002, 0x200000, CRC(62e2c3dd) SHA1(c9127ed70bdff947c3da2908a08974091615a685) )
	ROM_LOAD32_BYTE( "677a16.2h", 0x000003, 0x200000, CRC(7cc75539) SHA1(4bd8d88debf7489f30008bd4cbded67cb1a20ab0) )

	ROM_REGION(0x600000, "shared", 0)	/* Sound data */
	ROM_LOAD( "677a08.5r", 0x000000, 0x200000, CRC(bde38850) SHA1(aaf1bdfc25ecdffc1f6076c9c1b2edbe263171d2) )
	ROM_LOAD( "677a09.3r", 0x200000, 0x200000, CRC(4dfc1ea9) SHA1(4ab264c1902b522bc0589766e42f2b6ca276808d) )
	ROM_LOAD( "677a10.5n", 0x400000, 0x200000, CRC(d8f77a68) SHA1(ff251863ef096f0864f6cbe6caa43b0aa299d9ee) )
ROM_END

ROM_START( windheatu )
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE( "677ubc01.20u", 0x000003, 0x080000, CRC(63198721) SHA1(7f34131bf51d573d0c683b28df2567a0b911c98c) ) /* Program version UBC, USA v2.22 */
	ROM_LOAD32_BYTE( "677ubc02.17u", 0x000002, 0x080000, CRC(bdb00e2d) SHA1(c54b2250047576e12e9936300989e40494b4659d) )
	ROM_LOAD32_BYTE( "677ubc03.15u", 0x000001, 0x080000, CRC(0f7d8c1f) SHA1(63de03c7be794b6dae8d0af69e894ac573dbbc11) )
	ROM_LOAD32_BYTE( "677ubc04.13u", 0x000000, 0x080000, CRC(4e42791c) SHA1(a53c6374c6b46db578be4ced2ee7c2af7062d961) )

	ROM_REGION(0x20000, "audiocpu", 0)		/* M68K program */
	ROM_LOAD16_WORD_SWAP( "677a07.19l", 0x000000, 0x020000, CRC(05b14f2d) SHA1(3753f71173594ee741980e08eed0f7c3fc3588c9) )

	ROM_REGION(0x100000, "gfx2", 0)	/* Tilemap */
	ROM_LOAD16_BYTE( "677a11.35b", 0x000000, 0x080000, CRC(bf34f00f) SHA1(ca0d390c8b30d0cfdad4cfe5a601cc1f6e8c263d) )
	ROM_LOAD16_BYTE( "677a12.35a", 0x000001, 0x080000, CRC(458f0b1d) SHA1(8e11023c75c80b496dfc62b6645cfedcf2a80db4) )

	ROM_REGION(0x800000, "gfx1", 0)	/* Texture data */
	ROM_LOAD32_BYTE( "677a13.9h", 0x000000, 0x200000, CRC(7937d226) SHA1(c2ba777292c293e31068eeb3a27353ad2595b413) )
	ROM_LOAD32_BYTE( "677a14.7h", 0x000001, 0x200000, CRC(2568cf41) SHA1(6ed01922943486dafbdc863b76b2036c1fbe5281) )
	ROM_LOAD32_BYTE( "677a15.5h", 0x000002, 0x200000, CRC(62e2c3dd) SHA1(c9127ed70bdff947c3da2908a08974091615a685) )
	ROM_LOAD32_BYTE( "677a16.2h", 0x000003, 0x200000, CRC(7cc75539) SHA1(4bd8d88debf7489f30008bd4cbded67cb1a20ab0) )

	ROM_REGION(0x600000, "shared", 0)	/* Sound data */
	ROM_LOAD( "677a08.5r", 0x000000, 0x200000, CRC(bde38850) SHA1(aaf1bdfc25ecdffc1f6076c9c1b2edbe263171d2) )
	ROM_LOAD( "677a09.3r", 0x200000, 0x200000, CRC(4dfc1ea9) SHA1(4ab264c1902b522bc0589766e42f2b6ca276808d) )
	ROM_LOAD( "677a10.5n", 0x400000, 0x200000, CRC(d8f77a68) SHA1(ff251863ef096f0864f6cbe6caa43b0aa299d9ee) )
ROM_END

ROM_START( windheatj )
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE( "677jaa01.20u", 0x000003, 0x080000, CRC(559b8def) SHA1(6f2e8f29b0d9a950e71015270560813adc20b689) ) /* Program version JAA, JPN v2.11 */
	ROM_LOAD32_BYTE( "677jaa02.17u", 0x000002, 0x080000, CRC(cc230575) SHA1(be2da67600ab5edad2e8b7711c4cf985befe28bf) )
	ROM_LOAD32_BYTE( "677jaa03.15u", 0x000001, 0x080000, CRC(20b04701) SHA1(463be36c7f65b4aa3c3f2b1f37d1e6c1f5106cbb) )
	ROM_LOAD32_BYTE( "677jaa04.13u", 0x000000, 0x080000, CRC(f563b2a5) SHA1(b55b486b6af926eff4729f402116d45b61c5d25a) )

	ROM_REGION(0x20000, "audiocpu", 0)		/* M68K program */
	ROM_LOAD16_WORD_SWAP( "677a07.19l", 0x000000, 0x020000, CRC(05b14f2d) SHA1(3753f71173594ee741980e08eed0f7c3fc3588c9) )

	ROM_REGION(0x100000, "gfx2", 0)	/* Tilemap */
	ROM_LOAD16_BYTE( "677a11.35b", 0x000000, 0x080000, CRC(bf34f00f) SHA1(ca0d390c8b30d0cfdad4cfe5a601cc1f6e8c263d) )
	ROM_LOAD16_BYTE( "677a12.35a", 0x000001, 0x080000, CRC(458f0b1d) SHA1(8e11023c75c80b496dfc62b6645cfedcf2a80db4) )

	ROM_REGION(0x800000, "gfx1", 0)	/* Texture data */
	ROM_LOAD32_BYTE( "677a13.9h", 0x000000, 0x200000, CRC(7937d226) SHA1(c2ba777292c293e31068eeb3a27353ad2595b413) )
	ROM_LOAD32_BYTE( "677a14.7h", 0x000001, 0x200000, CRC(2568cf41) SHA1(6ed01922943486dafbdc863b76b2036c1fbe5281) )
	ROM_LOAD32_BYTE( "677a15.5h", 0x000002, 0x200000, CRC(62e2c3dd) SHA1(c9127ed70bdff947c3da2908a08974091615a685) )
	ROM_LOAD32_BYTE( "677a16.2h", 0x000003, 0x200000, CRC(7cc75539) SHA1(4bd8d88debf7489f30008bd4cbded67cb1a20ab0) )

	ROM_REGION(0x600000, "shared", 0)	/* Sound data */
	ROM_LOAD( "677a08.5r", 0x000000, 0x200000, CRC(bde38850) SHA1(aaf1bdfc25ecdffc1f6076c9c1b2edbe263171d2) )
	ROM_LOAD( "677a09.3r", 0x200000, 0x200000, CRC(4dfc1ea9) SHA1(4ab264c1902b522bc0589766e42f2b6ca276808d) )
	ROM_LOAD( "677a10.5n", 0x400000, 0x200000, CRC(d8f77a68) SHA1(ff251863ef096f0864f6cbe6caa43b0aa299d9ee) )
ROM_END

ROM_START( waveshrk )
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE( "678uab01.20u", 0x000003, 0x080000, CRC(a9b9ceed) SHA1(36f0d18481d7c3e7358e02473e54bc6b52d5c26b) )
	ROM_LOAD32_BYTE( "678uab02.17u", 0x000002, 0x080000, CRC(5ed24ac8) SHA1(d659c751558d4f8d89314466a37c04ac2df46879) )
	ROM_LOAD32_BYTE( "678uab03.15u", 0x000001, 0x080000, CRC(f4a595e7) SHA1(e05e7ea6613ecf70d8470af5fe0c6a7274c6e45b) )
	ROM_LOAD32_BYTE( "678uab04.13u", 0x000000, 0x080000, CRC(fd3320a7) SHA1(03a50a7bba9eb7cdb9f84953d6fb5c09f2d4b2db) )

	ROM_REGION(0x20000, "audiocpu", 0)		/* M68K program */
	ROM_LOAD16_WORD_SWAP( "678a07.19l", 0x000000, 0x020000, CRC(bb3f5875) SHA1(97f80d9b55d4177217b7cd1ba14e8ed2d64376bb) )

	ROM_REGION32_BE(0x400000, "user2", 0)	/* data roms */
	ROM_LOAD32_WORD_SWAP( "685a05.10u", 0x000000, 0x200000, CRC(00e59741) SHA1(d799910d4e85482b0e92a3cc9043f81d97b2fb02) )
	ROM_LOAD32_WORD_SWAP( "685a06.8u",  0x000002, 0x200000, CRC(fc98c6a5) SHA1(a84583bb7296fa9e0c284b2ac59e2dc7b2689eee) )

	ROM_REGION(0x800000, "gfx1", 0)	/* Texture data */
	ROM_LOAD32_BYTE( "678a13.18d", 0x000000, 0x200000, CRC(ccf75722) SHA1(f48d21dfc4f82adbb4c9c841a809267cfd028a3d) )
	ROM_LOAD32_BYTE( "678a14.13d", 0x000001, 0x200000, CRC(333a1ab4) SHA1(79df4a98b7871eba4157307a7709da8f8b5da39b) )
	ROM_LOAD32_BYTE( "678a15.9d",  0x000002, 0x200000, CRC(58b670f8) SHA1(5d4facb00e34de3ad11ed60c19835918a9cf6cb9) )
	ROM_LOAD32_BYTE( "678a16.4d",  0x000003, 0x200000, CRC(137b9bff) SHA1(5052c1fa30cc1d6affd78f48d483415dca89d10b) )

	ROM_REGION(0x600000, "shared", 0)	/* Sound data */
	ROM_LOAD( "678a08.5r", 0x000000, 0x200000, CRC(4aeb61ad) SHA1(ec6872cb2e4776849963f48c1c245ca7697849e0) )
	ROM_LOAD( "678a09.3r", 0x200000, 0x200000, CRC(39baef23) SHA1(9f7bda0f9c06eee94703f9ceb06975c8e28338cc) )
	ROM_LOAD( "678a10.5n", 0x400000, 0x200000, CRC(0508280e) SHA1(a36c5dc377b0ba597f131bd9dfc6019e7fc2d243) )
ROM_END

/*****************************************************************************/

GAME( 1995, midnrun,  0,        zr107,   midnrun,  zr107,   ROT0, "Konami", "Midnight Run (Euro v1.11)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1996, windheat, 0,        zr107,   windheat, zr107,   ROT0, "Konami", "Winding Heat (EAA, Euro v2.11)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1996, windheatu,windheat, zr107,   windheat, zr107,   ROT0, "Konami", "Winding Heat (UBC, USA v2.22)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1996, windheatj,windheat, zr107,   windheat, zr107,   ROT0, "Konami", "Winding Heat (JAA, Japan v2.11)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1996, waveshrk, 0,        jetwave, jetwave,  jetwave, ROT0, "Konami", "Wave Shark (UAB, USA v1.04)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
