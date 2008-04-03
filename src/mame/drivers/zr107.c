/*  Konami ZR107 System

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


    Hardware configurations:

    Game                           | ID        | CPU PCB      | CG Board(s)
    --------------------------------------------------------------------------
    Midnight Run                   | GX???     | ZR107        | ZR107
    Winding Heat                   | GX677     | ZR107        | ZR107
    Jetwave / Waveshark            | GX678     | ZR107        | GN678

*/

#include "driver.h"
#include "deprecat.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/sharc/sharc.h"
#include "sound/k054539.h"
#include "machine/konppc.h"
#include "machine/konamiic.h"
#include "machine/adc083x.h"
#include "machine/eeprom.h"
#include "video/konamiic.h"
#include "video/gticlub.h"


static UINT8 led_reg0 = 0x7f, led_reg1 = 0x7f;


// defined in drivers/gticlub.c
extern READ32_HANDLER(lanc_r);
extern WRITE32_HANDLER(lanc_w);
extern READ32_HANDLER(lanc_ram_r);
extern WRITE32_HANDLER(lanc_ram_w);


// defined in video/gticlub.c
VIDEO_START( gticlub );
VIDEO_UPDATE( gticlub );


// defined in drivers/nwk-tr.c
int K001604_vh_start(running_machine *machine, int chip);
void K001604_tile_update(running_machine *machine, int chip);
void K001604_draw_front_layer(int chip, bitmap_t *bitmap, const rectangle *cliprect);
void K001604_draw_back_layer(int chip, bitmap_t *bitmap, const rectangle *cliprect);
READ32_HANDLER(K001604_tile_r);
READ32_HANDLER(K001604_char_r);
WRITE32_HANDLER(K001604_tile_w);
WRITE32_HANDLER(K001604_char_w);
WRITE32_HANDLER(K001604_reg_w);
READ32_HANDLER(K001604_reg_r);




static VIDEO_START( jetwave )
{
	K001005_init(machine);
	K001604_vh_start(machine, 0);
}


static VIDEO_UPDATE( jetwave )
{
	fillbitmap(bitmap, screen->machine->pens[0], cliprect);

	K001604_tile_update(screen->machine, 0);
	K001005_draw(bitmap, cliprect);

	K001604_draw_front_layer(0, bitmap, cliprect);

	draw_7segment_led(bitmap, 3, 3, led_reg0);
	draw_7segment_led(bitmap, 9, 3, led_reg1);

	cpuintrf_push_context(2);
	sharc_set_flag_input(1, ASSERT_LINE);
	cpuintrf_pop_context();
	return 0;
}


/*****************************************************************************/

static WRITE32_HANDLER( paletteram32_w )
{
	COMBINE_DATA(&paletteram32[offset]);
	data = paletteram32[offset];
	palette_set_color_rgb(Machine, (offset * 2) + 0, pal5bit(data >> 26), pal5bit(data >> 21), pal5bit(data >> 16));
	palette_set_color_rgb(Machine, (offset * 2) + 1, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

#define NUM_LAYERS	2

static void game_tile_callback(int layer, int *code, int *color, int *flags)
{
	*color += layer * 0x40;
}

static VIDEO_START( zr107 )
{
	static int scrolld[NUM_LAYERS][4][2] = {
	 	{{ 0, 0}, {0, 0}, {0, 0}, {0, 0}},
	 	{{ 0, 0}, {0, 0}, {0, 0}, {0, 0}}
	};

	K056832_vh_start(machine, REGION_GFX2, K056832_BPP_8, 1, scrolld, game_tile_callback, 0);
	K001005_init(machine);
}

static VIDEO_UPDATE( zr107 )
{
	fillbitmap(bitmap, screen->machine->pens[0], cliprect);

	K056832_set_LayerOffset(0, -29, -27);
	K056832_set_LayerOffset(1, -29, -27);
	K056832_set_LayerOffset(2, -29, -27);
	K056832_set_LayerOffset(3, -29, -27);
	K056832_set_LayerOffset(4, -29, -27);
	K056832_set_LayerOffset(5, -29, -27);
	K056832_set_LayerOffset(6, -29, -27);
	K056832_set_LayerOffset(7, -29, -27);

	K056832_tilemap_draw(screen->machine, bitmap, cliprect, 1, 0, 0);
	K001005_draw(bitmap, cliprect);
	K056832_tilemap_draw(screen->machine, bitmap, cliprect, 0, 0, 0);

	draw_7segment_led(bitmap, 3, 3, led_reg0);
	draw_7segment_led(bitmap, 9, 3, led_reg1);

	cpuintrf_push_context(2);
	sharc_set_flag_input(1, ASSERT_LINE);
	cpuintrf_pop_context();
	return 0;
}

/******************************************************************/

static READ32_HANDLER( sysreg_r )
{
	UINT32 r = 0;
	if (offset == 0)
	{
		if (ACCESSING_BITS_24_31)
		{
			r |= readinputport(0) << 24;
		}
		if (ACCESSING_BITS_16_23)
		{
			r |= readinputport(1) << 16;
		}
		if (ACCESSING_BITS_8_15)
		{
			int adc_bit = adc083x_do_read(0);
			r |= ((readinputport(2) & 0x7f) | (adc_bit << 7)) << 8;
		}
		if (ACCESSING_BITS_0_7)
		{
			r |= readinputport(3) << 0;
		}
	}
	else if (offset == 1)
	{
		if (ACCESSING_BITS_24_31)
		{
			r |= ((adc083x_sars_read(0) << 5) | (EEPROM_read_bit() << 4)) << 24;
		}
	}
	//mame_printf_debug("sysreg_r: %08X, %08X at %08X\n", offset, mem_mask, activecpu_get_pc());
	return r;
}

static WRITE32_HANDLER( sysreg_w )
{
	if( offset == 0 )
	{
		if (ACCESSING_BITS_24_31)
		{
			led_reg0 = (data >> 24) & 0xff;
		}
		if (ACCESSING_BITS_16_23)
		{
			led_reg1 = (data >> 16) & 0xff;
		}
		if (ACCESSING_BITS_0_7)
		{
			EEPROM_write_bit((data & 0x1) ? 1 : 0);
			EEPROM_set_clock_line((data & 0x2) ? ASSERT_LINE : CLEAR_LINE);
			EEPROM_set_cs_line((data & 0x4) ? CLEAR_LINE : ASSERT_LINE);

			if (data & 0x10)
				cpunum_set_input_line(Machine, 1, INPUT_LINE_RESET, CLEAR_LINE);
			else
				cpunum_set_input_line(Machine, 1, INPUT_LINE_RESET, ASSERT_LINE);
		}
		return;
	}
	else if( offset == 1 )
	{
		if (ACCESSING_BITS_24_31)
		{
			if (data & 0x80000000)	/* CG Board 1 IRQ Ack */
				cpunum_set_input_line(Machine, 0, INPUT_LINE_IRQ1, CLEAR_LINE);

			if (data & 0x40000000)	/* CG Board 0 IRQ Ack */
				cpunum_set_input_line(Machine, 0, INPUT_LINE_IRQ0, CLEAR_LINE);

			set_cgboard_id((data >> 28) & 0x3);

			adc083x_cs_write(0, (data >> 26) & 1);
			adc083x_di_write(0, (data >> 25) & 1);
			adc083x_clk_write(0, (data >> 24) & 1);
		}
		return;
	}
	mame_printf_debug("sysreg_w: %08X, %08X, %08X\n", offset, data, mem_mask);
}

static double adc0838_callback(int input)
{
	switch (input)
	{
		case ADC083X_CH0:
			return (double)(5 * readinputport(4)) / 255.0;
		case ADC083X_CH1:
			return (double)(5 * readinputport(5)) / 255.0;
		case ADC083X_CH2:
			return (double)(5 * readinputport(6)) / 255.0;
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

static int ccu_vcth = 0;
static int ccu_vctl = 0;
static READ32_HANDLER( ccu_r )
{
	UINT32 r = 0;
	switch (offset)
	{
		case 0x1c/4:
		{
			// Midnight Run polls the vertical counter in vblank
			if (ACCESSING_BITS_24_31)
			{
				ccu_vcth ^= 0xff;
				r |= ccu_vcth << 24;
			}
			if (ACCESSING_BITS_8_15)
			{
				ccu_vctl++;
				ccu_vctl &= 0x1ff;
				r |= (ccu_vctl >> 2) << 8;
			}
		}
	}

	return r;
}

static WRITE32_HANDLER( ccu_w )
{

}

/******************************************************************/

static ADDRESS_MAP_START( zr107_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x000fffff) AM_MIRROR(0x80000000) AM_RAM		/* Work RAM */
	AM_RANGE(0x74000000, 0x74003fff) AM_MIRROR(0x80000000) AM_READWRITE(K056832_ram_long_r, K056832_ram_long_w)
	AM_RANGE(0x74020000, 0x7402003f) AM_MIRROR(0x80000000) AM_READWRITE(K056832_long_r, K056832_long_w)
	AM_RANGE(0x74060000, 0x7406003f) AM_MIRROR(0x80000000) AM_READWRITE(ccu_r, ccu_w)
	AM_RANGE(0x74080000, 0x74081fff) AM_MIRROR(0x80000000) AM_READWRITE(SMH_RAM, paletteram32_w) AM_BASE(&paletteram32)
	AM_RANGE(0x740a0000, 0x740a3fff) AM_MIRROR(0x80000000) AM_READ(K056832_rom_long_r)
	AM_RANGE(0x78000000, 0x7800ffff) AM_MIRROR(0x80000000) AM_READWRITE(cgboard_dsp_shared_r_ppc, cgboard_dsp_shared_w_ppc)		/* 21N 21K 23N 23K */
	AM_RANGE(0x78010000, 0x7801ffff) AM_MIRROR(0x80000000) AM_WRITE(cgboard_dsp_shared_w_ppc)
	AM_RANGE(0x78040000, 0x7804000f) AM_MIRROR(0x80000000) AM_READWRITE(K001006_0_r, K001006_0_w)
	AM_RANGE(0x780c0000, 0x780c0007) AM_MIRROR(0x80000000) AM_READWRITE(cgboard_dsp_comm_r_ppc, cgboard_dsp_comm_w_ppc)
	AM_RANGE(0x7e000000, 0x7e003fff) AM_MIRROR(0x80000000) AM_READWRITE(sysreg_r, sysreg_w)
	AM_RANGE(0x7e008000, 0x7e009fff) AM_MIRROR(0x80000000) AM_READWRITE(lanc_r, lanc_w)				/* LANC registers */
	AM_RANGE(0x7e00a000, 0x7e00bfff) AM_MIRROR(0x80000000) AM_READWRITE(lanc_ram_r, lanc_ram_w)		/* LANC Buffer RAM (27E) */
	AM_RANGE(0x7e00c000, 0x7e00c007) AM_MIRROR(0x80000000) AM_WRITE(K056800_host_w)
	AM_RANGE(0x7e00c008, 0x7e00c00f) AM_MIRROR(0x80000000) AM_READ(K056800_host_r)
	AM_RANGE(0x7f800000, 0x7f9fffff) AM_MIRROR(0x80000000) AM_ROM AM_SHARE(2)
	AM_RANGE(0x7fe00000, 0x7fffffff) AM_MIRROR(0x80000000) AM_ROM AM_REGION(REGION_USER1, 0) AM_SHARE(2)	/* Program ROM */
ADDRESS_MAP_END


static WRITE32_HANDLER( jetwave_palette_w )
{
	COMBINE_DATA(&paletteram32[offset]);
	data = paletteram32[offset];
	palette_set_color_rgb(Machine, offset, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

static ADDRESS_MAP_START( jetwave_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x000fffff) AM_MIRROR(0x80000000) AM_RAM		/* Work RAM */
	AM_RANGE(0x74000000, 0x740000ff) AM_MIRROR(0x80000000) AM_READWRITE(K001604_reg_r, K001604_reg_w)
	AM_RANGE(0x74010000, 0x7401ffff) AM_MIRROR(0x80000000) AM_READWRITE(SMH_RAM, jetwave_palette_w) AM_BASE(&paletteram32)
	AM_RANGE(0x74020000, 0x7403ffff) AM_MIRROR(0x80000000) AM_READWRITE(K001604_tile_r, K001604_tile_w)
	AM_RANGE(0x74040000, 0x7407ffff) AM_MIRROR(0x80000000) AM_READWRITE(K001604_char_r, K001604_char_w)
	AM_RANGE(0x78000000, 0x7800ffff) AM_MIRROR(0x80000000) AM_READWRITE(cgboard_dsp_shared_r_ppc, cgboard_dsp_shared_w_ppc)		/* 21N 21K 23N 23K */
	AM_RANGE(0x78010000, 0x7801ffff) AM_MIRROR(0x80000000) AM_WRITE(cgboard_dsp_shared_w_ppc)
	AM_RANGE(0x78040000, 0x7804000f) AM_MIRROR(0x80000000) AM_READWRITE(K001006_0_r, K001006_0_w)
	AM_RANGE(0x78080000, 0x7808000f) AM_MIRROR(0x80000000) AM_READWRITE(K001006_1_r, K001006_1_w)
	AM_RANGE(0x780c0000, 0x780c0007) AM_MIRROR(0x80000000) AM_READWRITE(cgboard_dsp_comm_r_ppc, cgboard_dsp_comm_w_ppc)
	AM_RANGE(0x7e000000, 0x7e003fff) AM_MIRROR(0x80000000) AM_READWRITE(sysreg_r, sysreg_w)
	AM_RANGE(0x7e008000, 0x7e009fff) AM_MIRROR(0x80000000) AM_READWRITE(lanc_r, lanc_w)				/* LANC registers */
	AM_RANGE(0x7e00a000, 0x7e00bfff) AM_MIRROR(0x80000000) AM_READWRITE(lanc_ram_r, lanc_ram_w)		/* LANC Buffer RAM (27E) */
	AM_RANGE(0x7e00c000, 0x7e00c007) AM_MIRROR(0x80000000) AM_WRITE(K056800_host_w)
	AM_RANGE(0x7e00c008, 0x7e00c00f) AM_MIRROR(0x80000000) AM_READ(K056800_host_r)
	AM_RANGE(0x7f000000, 0x7f3fffff) AM_MIRROR(0x80000000) AM_ROM AM_REGION(REGION_USER2, 0)
	AM_RANGE(0x7f800000, 0x7f9fffff) AM_MIRROR(0x80000000) AM_ROM AM_SHARE(2)
	AM_RANGE(0x7fe00000, 0x7fffffff) AM_MIRROR(0x80000000) AM_ROM AM_REGION(REGION_USER1, 0) AM_SHARE(2)	/* Program ROM */
ADDRESS_MAP_END



/**********************************************************************/

static READ16_HANDLER( dual539_r )
{
	UINT16 ret = 0;

	if (ACCESSING_BITS_0_7)
		ret |= K054539_1_r(machine, offset);
	if (ACCESSING_BITS_8_15)
		ret |= K054539_0_r(machine, offset)<<8;

	return ret;
}

static WRITE16_HANDLER( dual539_w )
{
	if (ACCESSING_BITS_0_7)
		K054539_1_w(machine, offset, data);
	if (ACCESSING_BITS_8_15)
		K054539_0_w(machine, offset, data>>8);
}

static ADDRESS_MAP_START( sound_memmap, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM		/* Work RAM */
	AM_RANGE(0x200000, 0x2004ff) AM_READWRITE(dual539_r, dual539_w)
	AM_RANGE(0x400000, 0x40000f) AM_WRITE(K056800_sound_w)
	AM_RANGE(0x400010, 0x40001f) AM_READ(K056800_sound_r)
	AM_RANGE(0x580000, 0x580001) AM_WRITENOP
ADDRESS_MAP_END

static const struct K054539interface k054539_interface =
{
	REGION_SOUND1
};

/*****************************************************************************/

static UINT32 *sharc_dataram;

static READ32_HANDLER( dsp_dataram_r )
{
	return sharc_dataram[offset] & 0xffff;
}

static WRITE32_HANDLER( dsp_dataram_w )
{
	sharc_dataram[offset] = data;
}

static ADDRESS_MAP_START( sharc_map, ADDRESS_SPACE_DATA, 32 )
	AM_RANGE(0x400000, 0x41ffff) AM_READWRITE(cgboard_0_shared_sharc_r, cgboard_0_shared_sharc_w)
	AM_RANGE(0x500000, 0x5fffff) AM_READWRITE(dsp_dataram_r, dsp_dataram_w)
	AM_RANGE(0x600000, 0x6fffff) AM_READWRITE(K001005_r, K001005_w)
	AM_RANGE(0x700000, 0x7000ff) AM_READWRITE(cgboard_0_comm_sharc_r, cgboard_0_comm_sharc_w)
ADDRESS_MAP_END

/*****************************************************************************/


static INPUT_PORTS_START( midnrun )
	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )		// View switch
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )		// Shift up
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )		// Shift down
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_TOGGLE		// AT/MT switch
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x0b, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0c, 0x00, "Network ID" )
	PORT_DIPSETTING( 0x0c, "1" )
	PORT_DIPSETTING( 0x08, "2" )
	PORT_DIPSETTING( 0x04, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0x02, 0x02, "Transmission Type" )
	PORT_DIPSETTING( 0x02, "Button" )
	PORT_DIPSETTING( 0x00, "'T'Gate" )
	PORT_DIPNAME( 0x01, 0x01, "CG Board Type" )
	PORT_DIPSETTING( 0x01, DEF_STR( Single ) )
	PORT_DIPSETTING( 0x00, "Twin" )

	PORT_START_TAG("ANALOG1")		// Steering wheel
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START_TAG("ANALOG2")		// Acceleration pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START_TAG("ANALOG3")		// Brake pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

INPUT_PORTS_END

static INPUT_PORTS_START( windheat )
	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )		// View switch
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )		// Shift up
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )		// Shift down
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_TOGGLE		// AT/MT switch
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x0b, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0c, 0x00, "Network ID" )
	PORT_DIPSETTING( 0x0c, "1" )
	PORT_DIPSETTING( 0x08, "2" )
	PORT_DIPSETTING( 0x04, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0x02, 0x02, "Transmission Type" )
	PORT_DIPSETTING( 0x02, "Button" )
	PORT_DIPSETTING( 0x00, "'T'Gate" )
	PORT_DIPNAME( 0x01, 0x01, "CG Board Type" )
	PORT_DIPSETTING( 0x01, DEF_STR( Single ) )
	PORT_DIPSETTING( 0x00, "Twin" )

	PORT_START_TAG("ANALOG1")		// Steering wheel
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START_TAG("ANALOG2")		// Acceleration pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START_TAG("ANALOG3")		// Brake pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

INPUT_PORTS_END

static INPUT_PORTS_START( jetwave )
	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x0b, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x08, 0x00, "DIP 1" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP 2" )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP 3" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x00, "DIP 4" )
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START_TAG("ANALOG1")		// Steering wheel
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START_TAG("ANALOG2")		// Acceleration pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START_TAG("ANALOG3")		// Brake pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

INPUT_PORTS_END

static const ppc_config zr107_ppc_cfg =
{
	PPC_MODEL_403GA
};

static sharc_config sharc_cfg =
{
	BOOT_MODE_EPROM
};

/* PowerPC interrupts

    IRQ0:  Vblank
    IRQ2:  LANC
    DMA0

*/
static INTERRUPT_GEN( zr107_vblank )
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_IRQ0, ASSERT_LINE);
}
static MACHINE_RESET( zr107 )
{
	cpunum_set_input_line(machine, 2, INPUT_LINE_RESET, ASSERT_LINE);
}

static MACHINE_DRIVER_START( zr107 )

	/* basic machine hardware */
	MDRV_CPU_ADD(PPC403, 64000000/2)	/* PowerPC 403GA 32MHz */
	MDRV_CPU_CONFIG(zr107_ppc_cfg)
	MDRV_CPU_PROGRAM_MAP(zr107_map, 0)
	MDRV_CPU_VBLANK_INT("main", zr107_vblank)

	MDRV_CPU_ADD(M68000, 64000000/8)	/* 8MHz */
	MDRV_CPU_PROGRAM_MAP(sound_memmap, 0)

	MDRV_CPU_ADD(ADSP21062, 36000000)
	MDRV_CPU_CONFIG(sharc_cfg)
	MDRV_CPU_DATA_MAP(sharc_map, 0)

	MDRV_INTERLEAVE(500)

	MDRV_NVRAM_HANDLER(93C46)
	MDRV_MACHINE_RESET(zr107)

 	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 48*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 48*8-1)

	MDRV_PALETTE_LENGTH(65536)

	MDRV_VIDEO_START(zr107)
	MDRV_VIDEO_UPDATE(zr107)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(K054539, 48000)
	MDRV_SOUND_CONFIG(k054539_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.75)
	MDRV_SOUND_ROUTE(1, "right", 0.75)

	MDRV_SOUND_ADD(K054539, 48000)
	MDRV_SOUND_CONFIG(k054539_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.75)
	MDRV_SOUND_ROUTE(1, "right", 0.75)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( jetwave )

	/* basic machine hardware */
	MDRV_CPU_ADD(PPC403, 64000000/2)	/* PowerPC 403GA 32MHz */
	MDRV_CPU_CONFIG(zr107_ppc_cfg)
	MDRV_CPU_PROGRAM_MAP(jetwave_map, 0)
	MDRV_CPU_VBLANK_INT("main", zr107_vblank)

	MDRV_CPU_ADD(M68000, 64000000/8)	/* 8MHz */
	MDRV_CPU_PROGRAM_MAP(sound_memmap, 0)

	MDRV_CPU_ADD(ADSP21062, 36000000)
	MDRV_CPU_CONFIG(sharc_cfg)
	MDRV_CPU_DATA_MAP(sharc_map, 0)

	MDRV_INTERLEAVE(500)

	MDRV_NVRAM_HANDLER(93C46)
	MDRV_MACHINE_RESET(zr107)

 	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 48*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 48*8-1)

	MDRV_PALETTE_LENGTH(65536)

	MDRV_VIDEO_START(jetwave)
	MDRV_VIDEO_UPDATE(jetwave)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(K054539, 48000)
	MDRV_SOUND_CONFIG(k054539_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.75)
	MDRV_SOUND_ROUTE(1, "right", 0.75)

	MDRV_SOUND_ADD(K054539, 48000)
	MDRV_SOUND_CONFIG(k054539_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.75)
	MDRV_SOUND_ROUTE(1, "right", 0.75)
MACHINE_DRIVER_END

/*****************************************************************************/

static void sound_irq_callback(int irq)
{
	if (irq == 0)
		cpunum_set_input_line(Machine, 1, INPUT_LINE_IRQ1, PULSE_LINE);
	else
		cpunum_set_input_line(Machine, 1, INPUT_LINE_IRQ2, PULSE_LINE);
}

static DRIVER_INIT(zr107)
{
	init_konami_cgboard(1, CGBOARD_TYPE_ZR107);
	sharc_dataram = auto_malloc(0x100000);

	K001005_preprocess_texture_data(memory_region(REGION_GFX1), memory_region_length(REGION_GFX1), 0);

	K056800_init(sound_irq_callback);

	adc083x_init(0, ADC0838, adc0838_callback);
}

static DRIVER_INIT(jetwave)
{
	init_konami_cgboard(1, CGBOARD_TYPE_GTICLUB);
	sharc_dataram = auto_malloc(0x100000);

	K001005_preprocess_texture_data(memory_region(REGION_GFX1), memory_region_length(REGION_GFX1), 0);

	K056800_init(sound_irq_callback);

	adc083x_init(0, ADC0838, adc0838_callback);
}

/*****************************************************************************/

#define ROM_LOAD64_WORD(name,offset,length,hash)      ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(6))

ROM_START(midnrun)
	ROM_REGION(0x200000, REGION_USER1, 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE("midnight.20u", 0x000003, 0x80000, CRC(ea70edf2) SHA1(51c882383a150ba118ccd39eb869525fcf5eee3c))
	ROM_LOAD32_BYTE("midnight.17u", 0x000002, 0x80000, CRC(1462994f) SHA1(c8614c6c416f81737cc77c46eea6d8d440bc8cf3))
	ROM_LOAD32_BYTE("midnight.15u", 0x000001, 0x80000, CRC(b770ae46) SHA1(c61daa8353802957eb1c2e2c6204c3a98569627e))
	ROM_LOAD32_BYTE("midnight.13u", 0x000000, 0x80000, CRC(9644b277) SHA1(b9cb812b6035dfd93032d277c8aa0037cf6b3dbe))

	ROM_REGION(0x20000, REGION_CPU2, 0)		/* M68K program */
	ROM_LOAD16_WORD_SWAP("midnight.19l", 0x000000, 0x20000, CRC(a82c0ba1) SHA1(dad69f2e5e75009d70cc2748477248ec47627c30))

	ROM_REGION(0x100000, REGION_GFX2, 0)	/* Tilemap */
	ROM_LOAD16_BYTE("midnight.35b", 0x000000, 0x80000, CRC(85eef04b) SHA1(02e26d2d4a8b29894370f28d2a49fdf5c7d23f95))
	ROM_LOAD16_BYTE("midnight.35a", 0x000001, 0x80000, CRC(451d7777) SHA1(0bf280ca475100778bbfd3f023547bf0413fc8b7))

	ROM_REGION(0x800000, REGION_GFX1, 0)	/* Texture data */
	ROM_LOAD32_BYTE("midnight.m9h", 0x000000, 0x200000, CRC(b1ee901d) SHA1(b1432cb1379b35d99d3f2b7f6409db6f7e88121d))
	ROM_LOAD32_BYTE("midnight.7h",  0x000001, 0x200000, CRC(9ffa8cc5) SHA1(eaa19e26df721bec281444ca1c5ccc9e48df1b0b))
	ROM_LOAD32_BYTE("midnight.5h",  0x000002, 0x200000, CRC(e337fce7) SHA1(c84875f3275efd47273508b340231721f5a631d2))
	ROM_LOAD32_BYTE("midnight.m2h", 0x000003, 0x200000, CRC(2c03ee63) SHA1(6b74d340dddf92bb4e4b1e037f003d58c65d8d9b))

	ROM_REGION(0x600000, REGION_SOUND1, 0)	/* Sound data */
	ROM_LOAD("midnight.m3r", 0x000000, 0x200000, CRC(f431e29f) SHA1(e6082d88f86abb63d02ac34e70873b58f88b0ddc))
	ROM_LOAD("midnight.m5n", 0x200000, 0x200000, CRC(8db31bd4) SHA1(d662d3bb6e8b44a01ffa158f5d7425454aad49a3))
	ROM_LOAD("midnight.m5r", 0x400000, 0x200000, CRC(d320dbde) SHA1(eb602cad6ac7c7151c9f29d39b10041d5a354164))
ROM_END

ROM_START(windheat)
	ROM_REGION(0x200000, REGION_USER1, 0)	/* PowerPC program roms */
        ROM_LOAD32_BYTE( "677ubc01.20u", 0x000003, 0x080000, CRC(63198721) SHA1(7f34131bf51d573d0c683b28df2567a0b911c98c) )
        ROM_LOAD32_BYTE( "677ubc02.17u", 0x000002, 0x080000, CRC(bdb00e2d) SHA1(c54b2250047576e12e9936300989e40494b4659d) )
        ROM_LOAD32_BYTE( "677ubc03.15u", 0x000001, 0x080000, CRC(0f7d8c1f) SHA1(63de03c7be794b6dae8d0af69e894ac573dbbc11) )
        ROM_LOAD32_BYTE( "677ubc04.13u", 0x000000, 0x080000, CRC(4e42791c) SHA1(a53c6374c6b46db578be4ced2ee7c2af7062d961) )

	ROM_REGION(0x20000, REGION_CPU2, 0)		/* M68K program */
        ROM_LOAD16_WORD_SWAP( "677a07.19l",   0x000000, 0x020000, CRC(05b14f2d) SHA1(3753f71173594ee741980e08eed0f7c3fc3588c9) )

	ROM_REGION(0x100000, REGION_GFX2, 0)	/* Tilemap */
        ROM_LOAD16_BYTE( "677a11.35b",   0x000000, 0x080000, CRC(bf34f00f) SHA1(ca0d390c8b30d0cfdad4cfe5a601cc1f6e8c263d) )
        ROM_LOAD16_BYTE( "677a12.35a",   0x000001, 0x080000, CRC(458f0b1d) SHA1(8e11023c75c80b496dfc62b6645cfedcf2a80db4) )

	ROM_REGION(0x800000, REGION_GFX1, 0)	/* Texture data */
        ROM_LOAD32_BYTE( "677a13.9h",    0x000000, 0x200000, CRC(7937d226) SHA1(c2ba777292c293e31068eeb3a27353ad2595b413) )
        ROM_LOAD32_BYTE( "677a14.7h",    0x000001, 0x200000, CRC(2568cf41) SHA1(6ed01922943486dafbdc863b76b2036c1fbe5281) )
        ROM_LOAD32_BYTE( "677a15.5h",    0x000002, 0x200000, CRC(62e2c3dd) SHA1(c9127ed70bdff947c3da2908a08974091615a685) )
        ROM_LOAD32_BYTE( "677a16.2h",    0x000003, 0x200000, CRC(7cc75539) SHA1(4bd8d88debf7489f30008bd4cbded67cb1a20ab0) )

	ROM_REGION(0x600000, REGION_SOUND1, 0)	/* Sound data */
        ROM_LOAD( "677a09.3r",    0x000000, 0x200000, CRC(4dfc1ea9) SHA1(4ab264c1902b522bc0589766e42f2b6ca276808d) )
        ROM_LOAD( "677a10.5n",    0x200000, 0x200000, CRC(d8f77a68) SHA1(ff251863ef096f0864f6cbe6caa43b0aa299d9ee) )
        ROM_LOAD( "677a08.5r",    0x400000, 0x200000, CRC(bde38850) SHA1(aaf1bdfc25ecdffc1f6076c9c1b2edbe263171d2) )
ROM_END

ROM_START(jetwave)
	ROM_REGION(0x200000, REGION_USER1, 0)	/* PowerPC program roms */
        ROM_LOAD32_BYTE( "678uab01.20u", 0x000003, 0x080000, CRC(a9b9ceed) SHA1(36f0d18481d7c3e7358e02473e54bc6b52d5c26b) )
        ROM_LOAD32_BYTE( "678uab02.17u", 0x000002, 0x080000, CRC(5ed24ac8) SHA1(d659c751558d4f8d89314466a37c04ac2df46879) )
        ROM_LOAD32_BYTE( "678uab03.15u", 0x000001, 0x080000, CRC(f4a595e7) SHA1(e05e7ea6613ecf70d8470af5fe0c6a7274c6e45b) )
        ROM_LOAD32_BYTE( "678uab04.13u", 0x000000, 0x080000, CRC(fd3320a7) SHA1(03a50a7bba9eb7cdb9f84953d6fb5c09f2d4b2db) )

	ROM_REGION(0x20000, REGION_CPU2, 0)		/* M68K program */
        ROM_LOAD16_WORD_SWAP( "678a07.19l",   0x000000, 0x020000, CRC(bb3f5875) SHA1(97f80d9b55d4177217b7cd1ba14e8ed2d64376bb) )

	ROM_REGION32_BE(0x400000, REGION_USER2, 0)	/* data roms */
        ROM_LOAD32_WORD_SWAP( "685a05.10u",   0x000000, 0x200000, CRC(00e59741) SHA1(d799910d4e85482b0e92a3cc9043f81d97b2fb02) )
        ROM_LOAD32_WORD_SWAP( "685a06.8u",   0x000002, 0x200000, CRC(fc98c6a5) SHA1(a84583bb7296fa9e0c284b2ac59e2dc7b2689eee) )

	ROM_REGION(0x800000, REGION_GFX1, 0)	/* Texture data */
        ROM_LOAD64_WORD( "678a13.18d",   0x000000, 0x200000, CRC(ccf75722) SHA1(f48d21dfc4f82adbb4c9c841a809267cfd028a3d) )
        ROM_LOAD64_WORD( "678a14.13d",   0x000002, 0x200000, CRC(333a1ab4) SHA1(79df4a98b7871eba4157307a7709da8f8b5da39b) )
        ROM_LOAD64_WORD( "678a15.9d",    0x000004, 0x200000, CRC(58b670f8) SHA1(5d4facb00e34de3ad11ed60c19835918a9cf6cb9) )
        ROM_LOAD64_WORD( "678a16.4d",    0x000006, 0x200000, CRC(137b9bff) SHA1(5052c1fa30cc1d6affd78f48d483415dca89d10b) )

	ROM_REGION(0x600000, REGION_SOUND1, 0)	/* Sound data */
	    ROM_LOAD( "678a09.3r",    0x000000, 0x200000, CRC(39baef23) SHA1(9f7bda0f9c06eee94703f9ceb06975c8e28338cc) )
        ROM_LOAD( "678a10.5n",    0x200000, 0x200000, CRC(0508280e) SHA1(a36c5dc377b0ba597f131bd9dfc6019e7fc2d243) )
        ROM_LOAD( "678a08.5r",    0x400000, 0x200000, CRC(4aeb61ad) SHA1(ec6872cb2e4776849963f48c1c245ca7697849e0) )
ROM_END

/*****************************************************************************/

GAME( 1995, midnrun,  0, zr107, midnrun,   zr107,    ROT0, "Konami", "Midnight Run", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1996, windheat, 0, zr107, windheat,  zr107,    ROT0, "Konami", "Winding Heat", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1996, jetwave,  0, jetwave, jetwave, jetwave,  ROT0, "Konami", "Jet Wave", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
