/*  Konami GTI Club System

    Driver by Ville Linde



    Hardware overview:

    GN672 CPU board:
    ----------------
        IBM PowerPC 403GA at 32MHz (main CPU)
        Motorola MC68EC000 at 16MHz (sound CPU)
        Konami K056800 (MIRAC), sound system interface
        Konami K056230 (LANC), LAN interface
        Ricoh RF5c400 sound chip

    GN678 GFX board:
    ----------------
        Analog Devices ADSP-21062 SHARC DSP at 36MHz
        Konami K001604 (2D tilemaps + 2x ROZ)
        2x KS10071 (custom 3D pixel unit)
        2x KS10081 (custom 3D texel unit)



    Hardware configurations:

    Game                           | ID        | CPU PCB      | CG Board(s)    | notes
    -----------------------------------------------------------------------------------------------
    GTI Club                       | GX688     | GN672(A)     | GN678(B)       |
    Operation Thunder Hurricane    | GX792     | GN672(A)     | GN678(B)       | extra board for gun controls(?)
    Solar Assault                  | GX680     | GN672(A)     | GN678(B)       |

    Hang Pilot                     | GN685     | ??           | 2x ??          | 3dfx-based CG boards
*/

#include "driver.h"
#include "deprecat.h"
#include "machine/eeprom.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/sharc/sharc.h"
#include "machine/konppc.h"
#include "machine/konamiic.h"
#include "machine/adc083x.h"
#include "sound/rf5c400.h"
#include "video/voodoo.h"
#include "video/gticlub.h"
#include "rendlay.h"

static UINT32 *work_ram;

UINT8 gticlub_led_reg0 = 0x7f;
UINT8 gticlub_led_reg1 = 0x7f;

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



VIDEO_START( gticlub );
VIDEO_UPDATE( gticlub );



static WRITE32_HANDLER( paletteram32_w )
{
	COMBINE_DATA(&paletteram32[offset]);
	data = paletteram32[offset];
	palette_set_color_rgb(machine, offset, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

static void voodoo_vblank_0(running_machine *machine, int param)
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_IRQ0, ASSERT_LINE);
}

static void voodoo_vblank_1(running_machine *machine, int param)
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_IRQ1, ASSERT_LINE);
}

static VIDEO_START( hangplt )
{
	const device_config *left_screen = device_list_find_by_tag(machine->config->devicelist, VIDEO_SCREEN, "left");
	const device_config *right_screen = device_list_find_by_tag(machine->config->devicelist, VIDEO_SCREEN, "right");

	voodoo_start(0, left_screen,  VOODOO_1, 2, 4, 4);
	voodoo_start(1, right_screen, VOODOO_1, 2, 4, 4);

	voodoo_set_vblank_callback(0, voodoo_vblank_0);
	voodoo_set_vblank_callback(1, voodoo_vblank_1);

	K001604_vh_start(machine, 0);
	K001604_vh_start(machine, 1);
}


static VIDEO_UPDATE( hangplt )
{
	const device_config *left_screen = device_list_find_by_tag(screen->machine->config->devicelist, VIDEO_SCREEN, "left");
	const device_config *right_screen = device_list_find_by_tag(screen->machine->config->devicelist, VIDEO_SCREEN, "right");

	fillbitmap(bitmap, screen->machine->pens[0], cliprect);

	if (screen == left_screen)
	{
		K001604_tile_update(screen->machine, 0);
	//  K001604_draw_back_layer(bitmap, cliprect);

		voodoo_update(0, bitmap, cliprect);

		K001604_draw_front_layer(0, bitmap, cliprect);
	}
	else if (screen == right_screen)
	{
		K001604_tile_update(screen->machine, 1);
	//  K001604_draw_back_layer(bitmap, cliprect);

		voodoo_update(1, bitmap, cliprect);

		K001604_draw_front_layer(1, bitmap, cliprect);
	}

	draw_7segment_led(bitmap, 3, 3, gticlub_led_reg0);
	draw_7segment_led(bitmap, 9, 3, gticlub_led_reg1);

	return 0;
}

/******************************************************************/

/* 93C56 EEPROM */
static const struct EEPROM_interface eeprom_interface =
{
	8,				/* address bits */
	16,				/* data bits */
	"*110",			/*  read command */
	"*101",			/* write command */
	"*111",			/* erase command */
	"*10000xxxxxx",	/* lock command */
	"*10011xxxxxx",	/* unlock command */
	1,				/* enable_multi_read */
	0				/* reset_delay */
};

static void eeprom_handler(mame_file *file, int read_or_write)
{
	if (read_or_write)
	{
		if (file)
		{
			EEPROM_save(file);
		}
	}
	else
	{
		EEPROM_init(&eeprom_interface);
		if (file)
		{
			EEPROM_load(file);
		}
		else
		{
			// set default eeprom
			UINT8 eepdata[0x200];
			memset(eepdata, 0xff, 0x200);

			if (mame_stricmp(Machine->gamedrv->name, "slrasslt") == 0)
			{
				// magic number
				eepdata[0x4] = 0x96;
				eepdata[0x5] = 0x72;
			}

			EEPROM_set_data(eepdata, 0x200);
		}
	}
}

static int adc1038_cycle;
static int adc1038_clk;
static int adc1038_adr;
static int adc1038_data_in;
static int adc1038_data_out;
static int adc1038_adc_data;
static int adc1038_sars = 1;
static int adc1038_gticlub_hack = 0;

static int adc1038_do_r(void)
{
	//printf("ADC DO\n");
	return adc1038_data_out & 1;
}

static void adc1038_di_w(int bit)
{
	adc1038_data_in = bit & 1;
}

static void adc1038_clk_w(running_machine *machine, int bit)
{
	// GTI Club doesn't sync on SARS
	if (adc1038_gticlub_hack)
	{
		if (adc1038_clk == 0 && bit == 0)
		{
			adc1038_cycle = 0;

			switch (adc1038_adr)
			{
				case 0: adc1038_adc_data = input_port_read_indexed(machine, 4); break;
				case 1: adc1038_adc_data = input_port_read_indexed(machine, 5); break;
				case 2: adc1038_adc_data = input_port_read_indexed(machine, 6); break;
				case 3: adc1038_adc_data = input_port_read_indexed(machine, 7); break;
				case 4: adc1038_adc_data = 0x000; break;
				case 5: adc1038_adc_data = 0x000; break;
				case 6: adc1038_adc_data = 0x000; break;
				case 7: adc1038_adc_data = 0x000; break;
			}
		}
	}

	if (bit == 1)
	{
		//printf("ADC CLK, DI = %d, cycle = %d\n", adc1038_data_in, adc1038_cycle);

		if (adc1038_cycle == 0)			// A2
		{
			adc1038_adr = 0;
			adc1038_adr |= (adc1038_data_in << 2);
		}
		else if (adc1038_cycle == 1)	// A1
		{
			adc1038_adr |= (adc1038_data_in << 1);
		}
		else if (adc1038_cycle == 2)	// A0
		{
			adc1038_adr |= (adc1038_data_in << 0);
		}

		adc1038_data_out = (adc1038_adc_data & 0x200) ? 1 : 0;
		adc1038_adc_data <<= 1;

		adc1038_cycle++;
	}

	adc1038_clk = bit;
}

static int adc1038_sars_r(running_machine *machine)
{
	adc1038_cycle = 0;

	switch (adc1038_adr)
	{
		case 0: adc1038_adc_data = input_port_read_indexed(machine, 4); break;
		case 1: adc1038_adc_data = input_port_read_indexed(machine, 5); break;
		case 2: adc1038_adc_data = input_port_read_indexed(machine, 6); break;
		case 3: adc1038_adc_data = input_port_read_indexed(machine, 7); break;
		case 4: adc1038_adc_data = 0x000; break;
		case 5: adc1038_adc_data = 0x000; break;
		case 6: adc1038_adc_data = 0x000; break;
		case 7: adc1038_adc_data = 0x000; break;
	}

	adc1038_data_out = (adc1038_adc_data & 0x200) ? 1 : 0;
	adc1038_adc_data <<= 1;

	adc1038_sars ^= 1;
	return adc1038_sars;
}

static READ32_HANDLER( sysreg_r )
{
	UINT32 r = 0;
	if (offset == 0)
	{
		if (ACCESSING_BITS_24_31)
		{
			r |= input_port_read_indexed(machine, 0) << 24;
		}
		if (ACCESSING_BITS_16_23)
		{
			r |= input_port_read_indexed(machine, 1) << 16;
		}
		if (ACCESSING_BITS_8_15)
		{
			r |= (adc1038_sars_r(machine) << 7) << 8;
		}
		if (ACCESSING_BITS_0_7)
		{
			r |= input_port_read_indexed(machine, 3) << 0;
		}
		return r;
	}
	else if (offset == 1)
	{
		if (ACCESSING_BITS_24_31 )
		{
			// 7        0
			// |?????ae?|
			//
			// a = ADC readout
			// e = EEPROM data out

			UINT32 eeprom_bit = (EEPROM_read_bit() << 1);
			UINT32 adc_bit = (adc1038_do_r() << 2);
			r |= (eeprom_bit | adc_bit) << 24;
		}
		else
		{
			mame_printf_debug("sysreg_r %d, %08X\n", offset, mem_mask);
		}
		return r;
	}
	return r;
}

static WRITE32_HANDLER( sysreg_w )
{
	if (offset == 0)
	{
		if( ACCESSING_BITS_24_31 )
		{
			gticlub_led_reg0 = (data >> 24) & 0xff;
		}
		if( ACCESSING_BITS_16_23 )
		{
			gticlub_led_reg1 = (data >> 16) & 0xff;
		}
		if( ACCESSING_BITS_0_7 )
		{
			EEPROM_write_bit((data & 0x01) ? 1 : 0);
			EEPROM_set_clock_line((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
			EEPROM_set_cs_line((data & 0x04) ? CLEAR_LINE : ASSERT_LINE);
		}
	}
	if( offset == 1 )
	{
		if (ACCESSING_BITS_24_31)
		{
			if (data & 0x80000000)	/* CG Board 1 IRQ Ack */
			{
				cpunum_set_input_line(machine, 0, INPUT_LINE_IRQ1, CLEAR_LINE);
			}
			if (data & 0x40000000)	/* CG Board 0 IRQ Ack */
			{
				cpunum_set_input_line(machine, 0, INPUT_LINE_IRQ0, CLEAR_LINE);
			}

			adc1038_di_w((data >> 24) & 1);
			adc1038_clk_w(machine, (data >> 25) & 1);

			set_cgboard_id((data >> 28) & 0x3);
		}
		return;
	}
}

/* Konami K056230 (LANC) */
static UINT8 K056230_r(int reg)
{
	switch (reg)
	{
		case 0:		// Status register
		{
			return 0x08;
		}
	}

//  mame_printf_debug("K056230_r: %d at %08X\n", reg, activecpu_get_pc());

	return 0;
}

static void K056230_w(int reg, UINT8 data)
{
	switch (reg)
	{
		case 0:		// Mode register
		{
			break;
		}
		case 1:		// Control register
		{
			if (data & 0x20)
			{
				// Thunder Hurricane breaks otherwise...
				if (mame_stricmp(Machine->gamedrv->name, "thunderh") != 0)
				{
					cpunum_set_input_line(Machine, 0, INPUT_LINE_IRQ2, ASSERT_LINE);
				}
			}
			break;
		}
		case 2:		// Sub ID register
		{
			break;
		}
	}
//  mame_printf_debug("K056230_w: %d, %02X at %08X\n", reg, data, activecpu_get_pc());
}

READ32_HANDLER( lanc_r )
{
	UINT32 r = 0;
	int reg = offset * 4;

	if (ACCESSING_BITS_24_31)
	{
		r |= K056230_r(reg+0) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= K056230_r(reg+1) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= K056230_r(reg+2) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= K056230_r(reg+3) << 0;
	}

	return r;
}

WRITE32_HANDLER( lanc_w )
{
	int reg = offset * 4;

	if (ACCESSING_BITS_24_31)
	{
		K056230_w(reg+0, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		K056230_w(reg+1, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		K056230_w(reg+2, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		K056230_w(reg+3, (data >> 0) & 0xff);
	}
}

static UINT32 lanc_ram[0x2000/4];
READ32_HANDLER( lanc_ram_r )
{
//  mame_printf_debug("LANC_RAM_r: %08X, %08X at %08X\n", offset, mem_mask, activecpu_get_pc());
	return lanc_ram[offset & 0x7ff];
}

WRITE32_HANDLER( lanc_ram_w )
{
//  mame_printf_debug("LANC_RAM_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, activecpu_get_pc());
	COMBINE_DATA(lanc_ram + (offset & 0x7ff));
}

/******************************************************************/

static ADDRESS_MAP_START( gticlub_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x000fffff) AM_MIRROR(0x80000000) AM_RAM AM_BASE(&work_ram)		/* Work RAM */
	AM_RANGE(0x74000000, 0x740000ff) AM_MIRROR(0x80000000) AM_READWRITE(K001604_reg_r, K001604_reg_w)
	AM_RANGE(0x74010000, 0x7401ffff) AM_MIRROR(0x80000000) AM_RAM_WRITE(paletteram32_w) AM_BASE(&paletteram32)
	AM_RANGE(0x74020000, 0x7403ffff) AM_MIRROR(0x80000000) AM_READWRITE(K001604_tile_r, K001604_tile_w)
	AM_RANGE(0x74040000, 0x7407ffff) AM_MIRROR(0x80000000) AM_READWRITE(K001604_char_r, K001604_char_w)
	AM_RANGE(0x78000000, 0x7800ffff) AM_MIRROR(0x80000000) AM_READWRITE(cgboard_dsp_shared_r_ppc, cgboard_dsp_shared_w_ppc)
	AM_RANGE(0x78040000, 0x7804000f) AM_MIRROR(0x80000000) AM_READWRITE(K001006_0_r, K001006_0_w)
	AM_RANGE(0x78080000, 0x7808000f) AM_MIRROR(0x80000000) AM_READWRITE(K001006_1_r, K001006_1_w)
	AM_RANGE(0x780c0000, 0x780c0003) AM_MIRROR(0x80000000) AM_READWRITE(cgboard_dsp_comm_r_ppc, cgboard_dsp_comm_w_ppc)
	AM_RANGE(0x7e000000, 0x7e003fff) AM_MIRROR(0x80000000) AM_READWRITE(sysreg_r, sysreg_w)
	AM_RANGE(0x7e008000, 0x7e009fff) AM_MIRROR(0x80000000) AM_READWRITE(lanc_r, lanc_w)
	AM_RANGE(0x7e00a000, 0x7e00bfff) AM_MIRROR(0x80000000) AM_READWRITE(lanc_ram_r, lanc_ram_w)
	AM_RANGE(0x7e00c000, 0x7e00c007) AM_MIRROR(0x80000000) AM_WRITE(K056800_host_w)
	AM_RANGE(0x7e00c000, 0x7e00c007) AM_MIRROR(0x80000000) AM_READ(K056800_host_r)		// Hang Pilot
	AM_RANGE(0x7e00c008, 0x7e00c00f) AM_MIRROR(0x80000000) AM_READ(K056800_host_r)
	AM_RANGE(0x7f000000, 0x7f3fffff) AM_MIRROR(0x80000000) AM_ROM AM_REGION(REGION_USER2, 0)	/* Data ROM */
	AM_RANGE(0x7f800000, 0x7f9fffff) AM_MIRROR(0x80000000) AM_ROM AM_SHARE(2)
	AM_RANGE(0x7fe00000, 0x7fffffff) AM_MIRROR(0x80000000) AM_ROM AM_REGION(REGION_USER1, 0) AM_SHARE(2)	/* Program ROM */
ADDRESS_MAP_END

/**********************************************************************/

static ADDRESS_MAP_START( sound_memmap, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_READWRITE(K056800_sound_r, K056800_sound_w)
	AM_RANGE(0x400000, 0x400fff) AM_READWRITE(RF5C400_0_r, RF5C400_0_w)		/* Ricoh RF5C400 */
	AM_RANGE(0x580000, 0x580001) AM_WRITENOP
	AM_RANGE(0x600000, 0x600001) AM_WRITENOP
ADDRESS_MAP_END

/*****************************************************************************/

static UINT32 *sharc_dataram_0;
static UINT32 *sharc_dataram_1;

static READ32_HANDLER( dsp_dataram0_r )
{
	return sharc_dataram_0[offset] & 0xffff;
}

static WRITE32_HANDLER( dsp_dataram0_w )
{
	sharc_dataram_0[offset] = data;
}

static READ32_HANDLER( dsp_dataram1_r )
{
	return sharc_dataram_1[offset] & 0xffff;
}

static WRITE32_HANDLER( dsp_dataram1_w )
{
	sharc_dataram_1[offset] = data;
}

static ADDRESS_MAP_START( sharc_map, ADDRESS_SPACE_DATA, 32 )
	AM_RANGE(0x400000, 0x41ffff) AM_READWRITE(cgboard_0_shared_sharc_r, cgboard_0_shared_sharc_w)
	AM_RANGE(0x500000, 0x5fffff) AM_READWRITE(dsp_dataram0_r, dsp_dataram0_w)
	AM_RANGE(0x600000, 0x6fffff) AM_READWRITE(K001005_r, K001005_w)
	AM_RANGE(0x700000, 0x7000ff) AM_READWRITE(cgboard_0_comm_sharc_r, cgboard_0_comm_sharc_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hangplt_sharc0_map, ADDRESS_SPACE_DATA, 32 )
	AM_RANGE(0x0400000, 0x041ffff) AM_READWRITE(cgboard_0_shared_sharc_r, cgboard_0_shared_sharc_w)
	AM_RANGE(0x0500000, 0x05fffff) AM_READWRITE(dsp_dataram0_r, dsp_dataram0_w)
	AM_RANGE(0x1400000, 0x14fffff) AM_RAM
	AM_RANGE(0x2400000, 0x27fffff) AM_READWRITE(nwk_voodoo_0_r, voodoo_0_w)
	AM_RANGE(0x3400000, 0x34000ff) AM_READWRITE(cgboard_0_comm_sharc_r, cgboard_0_comm_sharc_w)
	AM_RANGE(0x3401000, 0x34fffff) AM_WRITE(nwk_fifo_0_w)
	AM_RANGE(0x3500000, 0x3507fff) AM_READWRITE(K033906_0_r, K033906_0_w)
	AM_RANGE(0x3600000, 0x37fffff) AM_ROMBANK(5)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hangplt_sharc1_map, ADDRESS_SPACE_DATA, 32 )
	AM_RANGE(0x0400000, 0x041ffff) AM_READWRITE(cgboard_1_shared_sharc_r, cgboard_1_shared_sharc_w)
	AM_RANGE(0x0500000, 0x05fffff) AM_READWRITE(dsp_dataram1_r, dsp_dataram1_w)
	AM_RANGE(0x1400000, 0x14fffff) AM_RAM
	AM_RANGE(0x2400000, 0x27fffff) AM_READWRITE(nwk_voodoo_1_r, voodoo_1_w)
	AM_RANGE(0x3400000, 0x34000ff) AM_READWRITE(cgboard_1_comm_sharc_r, cgboard_1_comm_sharc_w)
	AM_RANGE(0x3401000, 0x34fffff) AM_WRITE(nwk_fifo_1_w)
	AM_RANGE(0x3500000, 0x3507fff) AM_READWRITE(K033906_1_r, K033906_1_w)
	AM_RANGE(0x3600000, 0x37fffff) AM_ROMBANK(6)
ADDRESS_MAP_END

/*****************************************************************************/

static NVRAM_HANDLER(gticlub)
{
	eeprom_handler(file, read_or_write);
}


static INPUT_PORTS_START( gticlub )
	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )		// View switch
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) 		// Shift Down
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )		// Shift Up
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )		// AT/MT switch
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x0b, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x03, 0x03, "Network ID" )
	PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x01, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "DIP3" )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP4" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START /* mask default type                     sens delta min max */
	PORT_BIT( 0x3ff, 0x200, IPT_PADDLE ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL2 ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL3 ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

INPUT_PORTS_END

static INPUT_PORTS_START( slrasslt )
	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )						// View Shift
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)		// Trigger
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)		// Missile
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)		// Power Up

	PORT_START
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x01, 0x01, "DIP1" )
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP2" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP3" )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP4" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START
	PORT_BIT( 0x3ff, 0x000, IPT_AD_STICK_Y ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START
	PORT_BIT( 0x3ff, 0x000, IPT_AD_STICK_X ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5) PORT_REVERSE

INPUT_PORTS_END

static INPUT_PORTS_START( thunderh )
	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x01, 0x00, "DIP1" )
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP2" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP3" )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP4" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( hangplt )
	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)		// Push limit switch
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)		// Pull limit switch

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x08, 0x08, "DIP4" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP3" )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Disable Test Mode" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x00, "Disable Machine Init" )
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

INPUT_PORTS_END

/* PowerPC interrupts

    IRQ0:  Vblank
    IRQ2:  LANC
    DMA0

*/
static INTERRUPT_GEN( gticlub_vblank )
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_IRQ0, ASSERT_LINE);
}


static const ppc_config gticlub_ppc_cfg =
{
	PPC_MODEL_403GA
};

static const sharc_config sharc_cfg =
{
	BOOT_MODE_EPROM
};

static const struct RF5C400interface rf5c400_interface =
{
	REGION_SOUND1
};

static MACHINE_RESET( gticlub )
{
	cpunum_set_input_line(machine, 2, INPUT_LINE_RESET, ASSERT_LINE);
}

static MACHINE_DRIVER_START( gticlub )

	/* basic machine hardware */
	MDRV_CPU_ADD(PPC403, 64000000/2)	/* PowerPC 403GA 32MHz */
	MDRV_CPU_CONFIG(gticlub_ppc_cfg)
	MDRV_CPU_PROGRAM_MAP(gticlub_map, 0)
	MDRV_CPU_VBLANK_INT("main", gticlub_vblank)

	MDRV_CPU_ADD(M68000, 64000000/4)	/* 16MHz */
	MDRV_CPU_PROGRAM_MAP(sound_memmap, 0)

	MDRV_CPU_ADD(ADSP21062, 36000000)
	MDRV_CPU_CONFIG(sharc_cfg)
	MDRV_CPU_DATA_MAP(sharc_map, 0)

	MDRV_INTERLEAVE(100)

	MDRV_NVRAM_HANDLER(gticlub)
	MDRV_MACHINE_RESET(gticlub)

 	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(512, 384)
	MDRV_SCREEN_VISIBLE_AREA(0, 511, 0, 383)

	MDRV_PALETTE_LENGTH(65536)

	MDRV_VIDEO_START(gticlub)
	MDRV_VIDEO_UPDATE(gticlub)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(RF5C400, 64000000/4)
	MDRV_SOUND_CONFIG(rf5c400_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END

static MACHINE_RESET( hangplt )
{
	cpunum_set_input_line(machine, 2, INPUT_LINE_RESET, ASSERT_LINE);
	cpunum_set_input_line(machine, 3, INPUT_LINE_RESET, ASSERT_LINE);
}

static MACHINE_DRIVER_START( hangplt )

	/* basic machine hardware */
	MDRV_CPU_ADD(PPC403, 64000000/2)	/* PowerPC 403GA 32MHz */
	MDRV_CPU_CONFIG(gticlub_ppc_cfg)
	MDRV_CPU_PROGRAM_MAP(gticlub_map, 0)

	MDRV_CPU_ADD(M68000, 64000000/4)	/* 16MHz */
	MDRV_CPU_PROGRAM_MAP(sound_memmap, 0)

	MDRV_CPU_ADD(ADSP21062, 36000000)
	MDRV_CPU_CONFIG(sharc_cfg)
	MDRV_CPU_DATA_MAP(hangplt_sharc0_map, 0)

	MDRV_CPU_ADD(ADSP21062, 36000000)
	MDRV_CPU_CONFIG(sharc_cfg)
	MDRV_CPU_DATA_MAP(hangplt_sharc1_map, 0)

	MDRV_INTERLEAVE(100)

	MDRV_NVRAM_HANDLER(gticlub)
	MDRV_MACHINE_RESET(hangplt)

 	/* video hardware */
	MDRV_PALETTE_LENGTH(65536)

	MDRV_SCREEN_ADD("left", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_SIZE(512, 384)
	MDRV_SCREEN_VISIBLE_AREA(0, 511, 0, 383)

	MDRV_SCREEN_ADD("right", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_SIZE(512, 384)
	MDRV_SCREEN_VISIBLE_AREA(0, 511, 0, 383)

	MDRV_VIDEO_START(hangplt)
	MDRV_VIDEO_UPDATE(hangplt)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(RF5C400, 64000000/4)
	MDRV_SOUND_CONFIG(rf5c400_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END

/*************************************************************************/

#define ROM_LOAD64_WORD(name,offset,length,hash)      ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(6))

ROM_START(gticlub)
	ROM_REGION(0x200000, REGION_USER1, 0)	/* PowerPC program roms */
		ROM_LOAD32_BYTE("688aaa01.21u", 0x000003, 0x80000, CRC(06a56474) SHA1(3a457b885a35e3ee030fd51d847bcf75fce46208))
		ROM_LOAD32_BYTE("688aaa02.19u", 0x000002, 0x80000, CRC(3c1e714a) SHA1(557f8542b855b2b35f242c8db7396017aca6dbd8))
		ROM_LOAD32_BYTE("688aaa03.21r", 0x000001, 0x80000, CRC(e060580b) SHA1(50242f3f3b949cc03082e4e75d9dcc89e17f0a75))
		ROM_LOAD32_BYTE("688aaa04.19r", 0x000000, 0x80000, CRC(928c23cd) SHA1(cce54398e1e5b98bfb717839cc422f1f60502788))

	ROM_REGION32_BE(0x400000, REGION_USER2, 0)	/* data roms */
		ROM_LOAD32_WORD_SWAP("688a05.14u", 0x000000, 0x200000, CRC(7caa3f80) SHA1(28409dc17c4e010173396fdc069a409fbea0d58d))
		ROM_LOAD32_WORD_SWAP("688a06.12u", 0x000002, 0x200000, CRC(83e7ce0a) SHA1(afe185f6ed700baaf4c8affddc29f8afdfec4423))

	ROM_REGION(0x80000, REGION_CPU2, 0)		/* 68k program */
		ROM_LOAD16_WORD_SWAP( "688a07.13k",   0x000000, 0x040000, CRC(f0805f06) SHA1(4b87e02b89e7ea812454498603767668e4619025) )

	ROM_REGION(0x800000, REGION_SOUND1, 0)	/* sound roms */
		ROM_LOAD( "688a09.9s",    0x000000, 0x200000, CRC(fb582963) SHA1(ce8fe6a4d7ac7d7f4b6591f9150b1d351e636354) )
		ROM_LOAD( "688a10.7s",    0x200000, 0x200000, CRC(b3ddc5f1) SHA1(a3f76c86e85eb17f20efb037c1ad64e9cb8566c8) )
		ROM_LOAD( "688a11.5s",    0x400000, 0x200000, CRC(fc706183) SHA1(c8ce6de0588be1023ef48577bc88a4e5effdcd25) )
		ROM_LOAD( "688a12.2s",    0x600000, 0x200000, CRC(510c70e3) SHA1(5af77bc98772ab7961308c3af0a80cb1bca690e3) )

    ROM_REGION(0x800000, REGION_GFX1, 0)	/* texture roms */
		ROM_LOAD64_WORD( "688a13.18d",   0x000000, 0x200000, CRC(c8f04f91) SHA1(9da8cc3a94dbf0a1fce87c2bc9249df712ae0b0d) )
		ROM_LOAD64_WORD( "688a14.13d",   0x000002, 0x200000, CRC(b9932735) SHA1(2492244d2acb350974202a6718bc7121325d2121) )
		ROM_LOAD64_WORD( "688a15.9d",    0x000004, 0x200000, CRC(8aadee51) SHA1(be9020a47583da9d4ff586d227836dc5b7dc31f0) )
		ROM_LOAD64_WORD( "688a16.4d",    0x000006, 0x200000, CRC(7f4e1893) SHA1(585be7b31ab7a48300c22b00443b00d631f4c49d) )
ROM_END

ROM_START(gticlubj)
	ROM_REGION(0x200000, REGION_USER1, 0)	/* PowerPC program roms */
		ROM_LOAD32_BYTE("688jaa01.bin", 0x000003, 0x80000, CRC(1492059c) SHA1(176dbd87f23f4cd8e1397e67da501738e20e5a57))
		ROM_LOAD32_BYTE("688jaa02.bin", 0x000002, 0x80000, CRC(7896dd69) SHA1(a3ab7b872132a5e66238e414f4b497cf7beb8b1c))
		ROM_LOAD32_BYTE("688jaa03.bin", 0x000001, 0x80000, CRC(94e2be50) SHA1(f206ac201903f3aae29196ab6fccdef104859346))
		ROM_LOAD32_BYTE("688jaa04.bin", 0x000000, 0x80000, CRC(ff539bb6) SHA1(1a225eca4377d82a2b6cb99c1d16580b9ccf2f08))

	ROM_REGION32_BE(0x400000, REGION_USER2, 0)	/* data roms */
		ROM_LOAD32_WORD_SWAP("688a05.14u", 0x000000, 0x200000, CRC(7caa3f80) SHA1(28409dc17c4e010173396fdc069a409fbea0d58d))
		ROM_LOAD32_WORD_SWAP("688a06.12u", 0x000002, 0x200000, CRC(83e7ce0a) SHA1(afe185f6ed700baaf4c8affddc29f8afdfec4423))

	ROM_REGION(0x80000, REGION_CPU2, 0)		/* 68k program */
        ROM_LOAD16_WORD_SWAP( "688a07.13k",   0x000000, 0x040000, CRC(f0805f06) SHA1(4b87e02b89e7ea812454498603767668e4619025) )

	ROM_REGION(0x800000, REGION_SOUND1, 0)	/* sound roms */
        ROM_LOAD( "688a09.9s",    0x000000, 0x200000, CRC(fb582963) SHA1(ce8fe6a4d7ac7d7f4b6591f9150b1d351e636354) )
        ROM_LOAD( "688a10.7s",    0x200000, 0x200000, CRC(b3ddc5f1) SHA1(a3f76c86e85eb17f20efb037c1ad64e9cb8566c8) )
        ROM_LOAD( "688a11.5s",    0x400000, 0x200000, CRC(fc706183) SHA1(c8ce6de0588be1023ef48577bc88a4e5effdcd25) )
        ROM_LOAD( "688a12.2s",    0x600000, 0x200000, CRC(510c70e3) SHA1(5af77bc98772ab7961308c3af0a80cb1bca690e3) )

    ROM_REGION(0x800000, REGION_GFX1, 0)	/* texture roms */
        ROM_LOAD64_WORD( "688a13.18d",   0x000000, 0x200000, CRC(c8f04f91) SHA1(9da8cc3a94dbf0a1fce87c2bc9249df712ae0b0d) )
        ROM_LOAD64_WORD( "688a14.13d",   0x000002, 0x200000, CRC(b9932735) SHA1(2492244d2acb350974202a6718bc7121325d2121) )
        ROM_LOAD64_WORD( "688a15.9d",    0x000004, 0x200000, CRC(8aadee51) SHA1(be9020a47583da9d4ff586d227836dc5b7dc31f0) )
        ROM_LOAD64_WORD( "688a16.4d",    0x000006, 0x200000, CRC(7f4e1893) SHA1(585be7b31ab7a48300c22b00443b00d631f4c49d) )
ROM_END

ROM_START( thunderh )
	ROM_REGION(0x200000, REGION_USER1, 0)	/* PowerPC program roms */
        ROM_LOAD32_BYTE( "680uaa01.21u", 0x000003, 0x080000, CRC(f2bb2ba1) SHA1(311e88d63179486014376c4af4ff0ef28673ee5a) )
        ROM_LOAD32_BYTE( "680uaa02.19u", 0x000002, 0x080000, CRC(52f617b5) SHA1(fda3133d3a7e04eb4432c69becdcf1872b3660d9) )
        ROM_LOAD32_BYTE( "680uaa03.21r", 0x000001, 0x080000, CRC(086a0574) SHA1(32fb93dbb93d2fe6af743ea4310b50a6cd03647d) )
        ROM_LOAD32_BYTE( "680uaa04.19r", 0x000000, 0x080000, CRC(85e1f8e3) SHA1(9172c54b6663f1bf390795068271198083a6860d) )

	ROM_REGION32_BE(0x400000, REGION_USER2, 0)	/* data roms */
        ROM_LOAD32_WORD_SWAP( "680a05.14u",   0x000000, 0x200000, CRC(0c9f334d) SHA1(99ac622a04a7140244d81031df69a796b6fd2657) )
        ROM_LOAD32_WORD_SWAP( "680a06.12u",   0x000002, 0x200000, CRC(83074217) SHA1(bbf782ac125cd98d9147ef4e0373bf61f74726f7) )

	ROM_REGION(0x80000, REGION_CPU2, 0)		/* 68k program */
        ROM_LOAD16_WORD_SWAP( "680a07.13k",   0x000000, 0x080000, CRC(12247a3e) SHA1(846cd9423efd3c9b17fce08393c6c83307d72f92) )

	ROM_REGION(0x20000, REGION_CPU3, 0)		/* 68k program for outboard sound? network? board */
        ROM_LOAD16_WORD_SWAP( "680c22.20k",   0x000000, 0x020000, CRC(d93c0ee2) SHA1(4b58418cbb01b51e12d6e7c86b2c81cd35d86248) )

	ROM_REGION(0x800000, REGION_SOUND1, 0)	/* sound roms */
        ROM_LOAD( "680a09.9s",    0x000000, 0x200000, CRC(71c2b049) SHA1(ce360172c8774b31edf16a80104c35b1caf26cd9) )
        ROM_LOAD( "680a10.7s",    0x200000, 0x200000, CRC(19882bf3) SHA1(7287da58853c84cbadbfb42bed37f2b0032c4b4d) )
        ROM_LOAD( "680a11.5s",    0x400000, 0x200000, CRC(0c74fe3f) SHA1(2e69f8d37552a74bbda65b134f747b4380ed33b0) )
        ROM_LOAD( "680a12.2s",    0x600000, 0x200000, CRC(b052919d) SHA1(a61c8eaf378ab7d780478db61217302d1b9f8f73) )

    ROM_REGION(0x800000, REGION_GFX1, 0)	/* texture roms */
        ROM_LOAD64_WORD( "680a13.18d",   0x000000, 0x200000, CRC(233f9074) SHA1(78ce42c35407d61df37cc0d16cdee84f8de968fa) )
        ROM_LOAD64_WORD( "680a14.13d",   0x000002, 0x200000, CRC(9ae15033) SHA1(12e291114629632b81f53811a6c8666aff4e92f3) )
        ROM_LOAD64_WORD( "680a15.9d",    0x000004, 0x200000, CRC(dc47c86f) SHA1(71af9b21f1ecc063135f501b1561869ee910c236) )
        ROM_LOAD64_WORD( "680a16.4d",    0x000006, 0x200000, CRC(ea388143) SHA1(3de5314a009d702186d5e285c8edefdd48139eab) )
ROM_END

ROM_START( slrasslt )
	ROM_REGION(0x200000, REGION_USER1, 0)	/* PowerPC program roms */
        ROM_LOAD32_BYTE( "792uaa01.21u", 0x000003, 0x080000, CRC(c73bf7fb) SHA1(ffe0fea155473827929339a9261a158287ce30a8) )
        ROM_LOAD32_BYTE( "792uaa02.19u", 0x000002, 0x080000, CRC(a940bb9b) SHA1(65a60157697a21cc2485c02c689c9addb3ac91f1) )
        ROM_LOAD32_BYTE( "792uaa03.21r", 0x000001, 0x080000, CRC(363e8411) SHA1(b9c70033d8e3de4b339b61a66172bfecb7c2b3ab) )
        ROM_LOAD32_BYTE( "792uaa04.19r", 0x000000, 0x080000, CRC(7910d99c) SHA1(e2114d369060528998b58331d590c086d306f541) )

	ROM_REGION32_BE(0x400000, REGION_USER2, 0)	/* data roms */
        ROM_LOAD32_WORD_SWAP( "792a05.14u",   0x000000, 0x200000, CRC(9a27edfc) SHA1(c028b6440eb1b0c814c4db45918e580662ac2d9a) )
        ROM_LOAD32_WORD_SWAP( "792a06.12u",   0x000002, 0x200000, CRC(c272f171) SHA1(df492287eadc5e8668fe46cfa3ed3ca77c57feca) )

	ROM_REGION(0x80000, REGION_CPU2, 0)		/* 68k program */
        ROM_LOAD16_WORD_SWAP( "792a07.10k",   0x000000, 0x080000, CRC(89a65ad1) SHA1(d814ef0b560c8e68da57ad5c6096e4fc05e9913e) )

	ROM_REGION(0x800000, REGION_SOUND1, 0)	/* sound roms */
        ROM_LOAD( "792a09.9s",    0x000000, 0x200000, CRC(7d7ea427) SHA1(a9a311a7c17223cc87140fe2890e20a321464831) )
        ROM_LOAD( "792a10.7s",    0x200000, 0x200000, CRC(e585e5d9) SHA1(ec44ad324a66eeea4c45933dda5a8a9a4398879d) )
        ROM_LOAD( "792a11.5s",    0x400000, 0x200000, CRC(c9c3a04c) SHA1(f834659f67712c9fcd93b7407669d7f35517b790) )
        ROM_LOAD( "792a12.2s",    0x600000, 0x200000, CRC(da8fcdd5) SHA1(daa7b3a086ada69e93c3d7cd9130befc79e422dc) )

    ROM_REGION(0x800000, REGION_GFX1, 0)	/* texture roms */
        ROM_LOAD64_WORD( "792a13.18d",   0x000000, 0x200000, CRC(16d6a134) SHA1(3f53f3c6759d7c5f40aa25a598df899fbac35a60) )
        ROM_LOAD64_WORD( "792a14.13d",   0x000002, 0x200000, CRC(cf57e830) SHA1(607b4dec3b8180a63e29d9dab1ca28d7226dda1e) )
        ROM_LOAD64_WORD( "792a15.9d",    0x000004, 0x200000, CRC(1c5531cb) SHA1(1b514f181c92e16d07bfe4719604f1e4caf15377) )
        ROM_LOAD64_WORD( "792a16.4d",    0x000006, 0x200000, CRC(df89e392) SHA1(af37c5460d43bf8d8a1ab4213c4528083a7363c2) )
ROM_END

ROM_START( hangplt )
	ROM_REGION(0x200000, REGION_USER1, 0)	/* PowerPC program roms */
        ROM_LOAD32_BYTE( "685jab01.21u", 0x000003, 0x080000, CRC(f98a3e82) SHA1(94ebaa172b0e98c5cd08efaea5f56e707e5032b4) )
        ROM_LOAD32_BYTE( "685jab02.19u", 0x000002, 0x080000, CRC(20730cdc) SHA1(71b2cf7077ab7db875f9030e21afd05905f57ce5) )
        ROM_LOAD32_BYTE( "685jab03.21r", 0x000001, 0x080000, CRC(77fa2248) SHA1(a662b84945b3d268fed15952cc793d821233735e) )
        ROM_LOAD32_BYTE( "685jab04.19r", 0x000000, 0x080000, CRC(ab6773df) SHA1(91d3f849a1cc5fa4b2fbd876d53402a548198c41) )

	ROM_REGION32_BE(0x400000, REGION_USER2, 0)	/* data roms */
        ROM_LOAD32_WORD_SWAP( "685a05.14u",   0x000000, 0x200000, CRC(ba1c8f40) SHA1(ce4ed641c1d6d44447eaaada16f305f1d7fb9ee2) )
        ROM_LOAD32_WORD_SWAP( "685a06.12u",   0x000002, 0x200000, CRC(2429935c) SHA1(4da9e169adcac81ea1bc135d727c2bd13ad372fa) )

	ROM_REGION(0x80000, REGION_CPU2, 0)	/* 68k program */
        ROM_LOAD16_WORD_SWAP( "685a07.13k",   0x000000, 0x080000, CRC(5b72fd80) SHA1(a150837fa0d66dc0c3832495a4c8ce4f9b92cd98) )

	ROM_REGION(0x1000000, REGION_SOUND1, 0)	/* other roms */
        ROM_LOAD( "685a09.9s",    0x000000, 0x200000, CRC(653821cf) SHA1(625abb7769a52c9ac61cfddaa084b9c9539f3b15) )
        ROM_LOAD( "685a10.7s",    0x200000, 0x200000, CRC(71eb06e5) SHA1(3c5953e87df63fb7680d7a04267ff2208f49838f) )

    ROM_REGION(0x800000, REGION_USER5, 0)	/* texture roms */
        ROM_LOAD32_WORD( "685a13.4w",    0x000002, 0x400000, CRC(06329af4) SHA1(76cad9db604751ce48bb67bfd29e57bac0ee9a16) )
        ROM_LOAD32_WORD( "685a14.12w",   0x000000, 0x400000, CRC(87437739) SHA1(0d45637af40938a54d5efd29c125b0fafd55f9a4) )
ROM_END

static void sound_irq_callback(running_machine *machine, int irq)
{
	if (irq == 0)
		cpunum_set_input_line(machine, 1, INPUT_LINE_IRQ1, PULSE_LINE);
	else
		cpunum_set_input_line(machine, 1, INPUT_LINE_IRQ2, PULSE_LINE);
}

static DRIVER_INIT(gticlub)
{
	init_konami_cgboard(1, CGBOARD_TYPE_GTICLUB);
	sharc_dataram_0 = auto_malloc(0x100000);

	K001005_preprocess_texture_data(memory_region(REGION_GFX1), memory_region_length(REGION_GFX1), 1);

	K056800_init(sound_irq_callback);

	// we'll need this for now
	if (mame_stricmp(machine->gamedrv->name, "gticlub") == 0 ||
		mame_stricmp(machine->gamedrv->name, "gticlubj") == 0)
	{
		adc1038_gticlub_hack = 1;
	}
}

static DRIVER_INIT(hangplt)
{
	init_konami_cgboard(2, CGBOARD_TYPE_HANGPLT);
	set_cgboard_texture_bank(0, 5, memory_region(REGION_USER5));
	set_cgboard_texture_bank(1, 6, memory_region(REGION_USER5));

	sharc_dataram_0 = auto_malloc(0x100000);
	sharc_dataram_1 = auto_malloc(0x100000);

	K056800_init(sound_irq_callback);
	K033906_init();
}

static DRIVER_INIT(slrasslt)
{
	DRIVER_INIT_CALL(gticlub);

	// enable self-modifying code checks
	cpunum_set_info_int(0, CPUINFO_INT_PPC_DRC_OPTIONS, PPCDRC_OPTIONS_CHECK_SELFMOD_CODE);
}

/*************************************************************************/

GAME( 1996, gticlub,	0,		 gticlub, gticlub,  gticlub,  ROT0,	"Konami",	"GTI Club (ver AAA)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1996, gticlubj,	gticlub, gticlub, gticlub,  gticlub,  ROT0,	"Konami",	"GTI Club (ver JAA)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1996, thunderh,	0,		 gticlub, thunderh, gticlub,  ROT0,	"Konami",	"Operation Thunder Hurricane (ver UAA)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1997, slrasslt,	0,		 gticlub, slrasslt, slrasslt, ROT0,	"Konami",	"Solar Assault (ver UAA)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAMEL( 1997, hangplt,	0,		 hangplt, hangplt,  hangplt,  ROT0,	"Konami",	"Hang Pilot", GAME_NOT_WORKING|GAME_IMPERFECT_SOUND, layout_dualhovu )
