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

    Hang Pilot                     | GN685     | GN672(A)     | 2x ??          | 3dfx-based CG boards


Konami 'GTI Club' Hardware
Konami, 1996-1998

Known games on this hardware include....

Game                        (C)      Year
-----------------------------------------
GTI Club                    Konami   1996
Jet Wave / Wave Shark       Konami   1996 (video board only)
Operation Thunder Hurricane Konami   1997
Solar Assault               Konami   1997
Solar Assault : Revised     Konami   1998


PCB Layouts
-----------

Top Board

GN672 PWB(A)3020088
Konami 1996
|--------------------------------------------------------------|
|                                    DRAM1M   MASKROM.2S       |
|CN13            NJM5532    PAL(002616)                        |
|                    SM5877                   MASKROM.5S       |
|       AN7395S  NJM5532                                       |
|                                   RF5C400   MASKROM.7S       |
|                                                              |
|             056800            SRAM256K      MASKROM.9S       |
|   058232                      SRAM256K                       |
|       RESET_SW        EPROM.13K                   MASKROM.12U|
|5                                                             |
|6                                                  MASKROM.14U|
|W  TEST_SW              68EC000FN16                           |
|A                                         EPROM.19R  EPROM.19U|
|Y                                         EPROM.19R  EPROM.19U|
|              33.868MHz                                       |
|                     PAL(002248)                  |---------| |
|                     PAL(002249)                  |PPC403GA | |
|             93C56                                |         | |
|LED        PAL(002247)      DRAM4MX16             |         | |
|                                                  |---------| |
|           056230           DRAM4MX16                         |
|SRAM64K                                                       |
|                   64MHz            MACH111                   |
|CN4                                                           |
|        PAL(056787A)                                          |
|CN5                                                           |
|DSW(4)                        CN12                            |
|--------------------------------------------------------------|
Notes:
         DRAM1M - OKI M514256 1Mx4 DRAM (SOJ26/20)
        SRAM64K - 8kx8 SRAM (DIP28)
       SRAM256K - Fujitsu 84256 32kx8 SRAM (DIP28)
      DRAM4MX16 - Hitachi HM514260 4Mx16 DRAM (SOJ42)
        RF5C400 - Ricoh RF5C400 PCM 32Ch, 44.1 kHz Stereo, 3D Effect Spatializer, clock input 16.934MHz (33.868/2)
         056800 - Konami Custom (QFP80)
         056230 - Konami Custom (QFP80)
         058232 - Konami Custom Ceramic Package (SIL14, D/A filter?)
        MACH111 - AMD MACH111 PLCC44 CPLD (stamped '002246')
        68EC000 - Motorola MC68EC000, running at 16.0MHz (64/4)
          93C56 - EEPROM (DIP8)
       PPC403GA - IBM PowerPC 403GA CPU, clock input 32.0MHz (64/2) (QFP160)
       SM5877AM - Nippon Precision Circuits 3rd Order 2-Channel D/A Converter (SOIC24)
      NJM5532AN - Dual Low-Noise High-Speed Audio OP Amp (DIP8)
        AN7395S - Panasonic AN7395S Spatializer Audio Processor IC for 3D surround (SOIC20)
          CN4/5 - Network connectors
           CN12 - DIN96 connector joining to lower PCB
           CN13 - Audio OUT connector
            LED - Alpha-numeric 7-segment LED

ROM Usage
---------
                            |--------------------------------------- ROM Locations --------------------------------------|
Game                        13K     2S      5S      7S      9S      12U     14U     19R       19U       21R       21U
--------------------------------------------------------------------------------------------------------------------------
GTI Club                    688A07  688A12  688A11  688A10  688A09  688A06  688A05  688AAA04  688AAA02  688AAA03  688AAA01
Jet Wave                    - see note -
Hang Pilot                  685A07  -       -       685A10  685A09  685A06  685A05  685JAB04  685JAB02  685JAB03  685JAB01
Operation Thunder Hurricane 680A07  680A12  680A11  680A10  680A09  680A06  680A05  680UAA04  680UAA02  680UAA03  680UAA01
Solar Assault               792A07  792A12  792A11  792A10  792A09  792A06  792A05  792UAA04  792UAA02  792UAA03  792UAA01
Solar Assault : Revised     - N/A -

Note : Jet Wave uses the lower board (GN678) from GTI Club, but it uses a different top board (ZR107 PWB(A)300769A)
Check zr107.c for details on the top board.

Operation Thunder Hurricane uses an additional top board for sound, network and analog
control functions...

GN680 PWB(E)403381B
|------------------------------------------|
|CN11  CN12    CN8      CN9    CN10  DSW(4)|
|                 NRPS11     NRPS11        |
|                                          |
|                        LM1881   LM1881   |
|                                          |
|LED(x4)                                   |
|                                          |
|           68EC000FN16  8464              |
|    RESET_SW            8464              |
|32MHz                           680C22.20K|
|8464                 PAL(002962)          |
|CN4   056230         PAL(002961)          |
|   PAL(056787A)      PAL(002960)          |
|CN5                                       |
|------------------------------------------|
Notes:
      68000 @ 16MHz (32/2)
      CN11/12 - Power connectors
      CN8/9   - 6-pin analog control connectors
      CN10    - 4-pin sound output connector
      NRPS11  - Idec NRPS11 PC Board circuit protector
      LM1881  - Video sync separator (DIP8)


Bottom Board

GN678 PWB(B)302009A
Konami 1996
|-------------------------------------------------------------------------------------------|
|CN4      MASKROM.2D      |--------|  SDR4M16  SDR4M16      |--------|  SDR4M16  SDR4M16    |
|                         |KS10081 |                        |KS10081 |                      |
|         MASKROM.4D      |        |                        |        |               SDR4M16|
|                         |--------|     |----------|       |--------|    |----------|      |
|CN2      MASKROM.6D                     | KS10071  | SDR4M16             | KS10071  |      |
|                                        |          |                     |          |      |
|         MASKROM.9D                     |          | SDR4M16             |          |      |
|                                        |----------|                     |----------|      |
|         MASKROM.11D                                                                SDR4M16|
|                                          SDR4M16                          SDR4M16         |
|         MASKROM.13D                                                                       |
|                                                      MC88916                              |
|         MASKROM.16D                                         PAL(002304)                   |
| MC44200                                                                 PAL(002303)       |
|         MASKROM.18D                                                                       |
|                                                                                           |
|                                                                         36MHz             |
|                                AM7203  AM7203  AM7203  AM7203         |-------------|     |
|                                                  256KSRAM  256KSRAM   |ANALOG       |     |
|                                                                       |DEVICES      |     |
|              PAL(002305)  64KSRAM   64KSRAM      256KSRAM  256KSRAM   |ADSP-21062   |     |
|                                                                       |SHARC        |     |
|         |--------|     MACH110                                        |KS-160X      |     |
|1MSRAM   |KONAMI  |                                                    |-------------|     |
|1MSRAM   |001604  |                                                                        |
|1MSRAM   |        |                                                        1MSRAM  1MSRAM  |
|1MSRAM   |--------|                                                                        |
|1MSRAM       256KSRAM                                                      1MSRAM  1MSRAM  |
|1MSRAM   256KSRAM 256KSRAM                    CN1                                          |
|-------------------------------------------------------------------------------------------|
Notes:
     SDR4M16 - Fujitsu 81141622-015 4M SDRAM (TSOP50)
      1MSRAM - Sharp LH521007 128kx8 SRAM (SOJ32)
    256KSRAM - Cypress CY7C199 32kx8 SRAM (SOJ28)
     64KSRAM - Cypress CY7C185 8kx8 SRAM (DIP28)
     KS10071 - Konami Custom video chip
     KS10081 - Konami Custom video chip
      001604 - Konami Custom (QFP208)
   MC44200FT - Motorola MC44200FT 3 Channel Video D/A Converter (QFP44)
     MACH110 - AMD MACH110 or MACH111 PLCC44 CPLD (Jet Wave stamped '002302')
                                                  (GTI Club stamped '003161')
                                                  (Thund.Hurr. stamped '003161')
      AM7203 - AMD AM7203 FIFO (PLCC32)
     MC88916 - Motorola MC88916 Low Skew CMOS PLL Clock Driver
         CN1 - 96 Pin joining connector to upper PCB
         CN2 - 8-Pin 24kHz RGB OUT
         CN4 - 6-Pin Power Connector

ROM Usage
---------
                            |---------------------- ROM Locations -----------------------|
Game                        2D      4D      6D      9D      11D     13D     16D     18D
------------------------------------------------------------------------------------------
GTI Club                    -       688A16  -       688A15  -       688A14  -       688A13
Jet Wave                    -       678A16  -       678A15  -       678A14  -       678A13
Operation Thunder Hurricane -       680A16  -       680A15  -       680A14  -       680A13
Solar Assault               -       792A16  -       792A15  -       792A14  -       792A13
Solar Assault : Revised     - N/A -

Hang Pilot (uses an unknown but similar video board)                12W             4W
                            -       -       -       -       -       678A14  -       678A13

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eeprom.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/sharc/sharc.h"
#include "machine/konppc.h"
#include "machine/adc1038.h"
#include "machine/k056230.h"
#include "machine/k033906.h"
#include "sound/rf5c400.h"
#include "sound/k056800.h"
#include "video/voodoo.h"
#include "video/gticlub.h"
#include "video/konicdev.h"

#include "rendlay.h"

class gticlub_state : public driver_device
{
public:
	gticlub_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_work_ram(*this, "work_ram")
	{ }

	required_shared_ptr<UINT32> m_work_ram;
	UINT32 *m_sharc_dataram_0;
	UINT32 *m_sharc_dataram_1;
	DECLARE_WRITE32_MEMBER(paletteram32_w);
	DECLARE_READ32_MEMBER(gticlub_k001604_tile_r);
	DECLARE_WRITE32_MEMBER(gticlub_k001604_tile_w);
	DECLARE_READ32_MEMBER(gticlub_k001604_char_r);
	DECLARE_WRITE32_MEMBER(gticlub_k001604_char_w);
	DECLARE_READ32_MEMBER(gticlub_k001604_reg_r);
	DECLARE_WRITE32_MEMBER(gticlub_k001604_reg_w);
	DECLARE_READ8_MEMBER(sysreg_r);
	DECLARE_WRITE8_MEMBER(sysreg_w);
	DECLARE_READ32_MEMBER(dsp_dataram0_r);
	DECLARE_WRITE32_MEMBER(dsp_dataram0_w);
	DECLARE_READ32_MEMBER(dsp_dataram1_r);
	DECLARE_WRITE32_MEMBER(dsp_dataram1_w);
	void init_hangplt_common();
	DECLARE_DRIVER_INIT(hangplt);
	DECLARE_DRIVER_INIT(hangpltu);
	DECLARE_DRIVER_INIT(gticlub);
	DECLARE_MACHINE_START(gticlub);
	DECLARE_MACHINE_RESET(gticlub);
	DECLARE_MACHINE_RESET(hangplt);
	INTERRUPT_GEN_MEMBER(gticlub_vblank);
	TIMER_CALLBACK_MEMBER(irq_off);
};


WRITE32_MEMBER(gticlub_state::paletteram32_w)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	data = m_generic_paletteram_32[offset];
	palette_set_color_rgb(machine(), offset, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

static void voodoo_vblank_0(device_t *device, int param)
{
	device->machine().device("maincpu")->execute().set_input_line(INPUT_LINE_IRQ0, param ? ASSERT_LINE : CLEAR_LINE);
}

static void voodoo_vblank_1(device_t *device, int param)
{
	device->machine().device("maincpu")->execute().set_input_line(INPUT_LINE_IRQ1, param ? ASSERT_LINE : CLEAR_LINE);
}

READ32_MEMBER(gticlub_state::gticlub_k001604_tile_r)
{
	device_t *k001604 = machine().device(get_cgboard_id() ? "k001604_2" : "k001604_1");
	return k001604_tile_r(k001604, space, offset, mem_mask);
}

WRITE32_MEMBER(gticlub_state::gticlub_k001604_tile_w)
{
	device_t *k001604 = machine().device(get_cgboard_id() ? "k001604_2" : "k001604_1");
	k001604_tile_w(k001604, space, offset, data, mem_mask);
}


READ32_MEMBER(gticlub_state::gticlub_k001604_char_r)
{
	device_t *k001604 = machine().device(get_cgboard_id() ? "k001604_2" : "k001604_1");
	return k001604_char_r(k001604, space, offset, mem_mask);
}

WRITE32_MEMBER(gticlub_state::gticlub_k001604_char_w)
{
	device_t *k001604 = machine().device(get_cgboard_id() ? "k001604_2" : "k001604_1");
	k001604_char_w(k001604, space, offset, data, mem_mask);
}

READ32_MEMBER(gticlub_state::gticlub_k001604_reg_r)
{
	device_t *k001604 = machine().device(get_cgboard_id() ? "k001604_2" : "k001604_1");
	return k001604_reg_r(k001604, space, offset, mem_mask);
}

WRITE32_MEMBER(gticlub_state::gticlub_k001604_reg_w)
{
	device_t *k001604 = machine().device(get_cgboard_id() ? "k001604_2" : "k001604_1");
	k001604_reg_w(k001604, space, offset, data, mem_mask);
}


/******************************************************************/

/* 93C56 EEPROM */
static const eeprom_interface eeprom_intf =
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

READ8_MEMBER(gticlub_state::sysreg_r)
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3" };
	device_t *adc1038 = machine().device("adc1038");
	eeprom_device *eeprom = machine().device<eeprom_device>("eeprom");

	switch (offset)
	{
		case 0:
		case 1:
		case 3:
			return ioport(portnames[offset])->read();

		case 2:
			return adc1038_sars_read(adc1038) << 7;

		case 4:
		{
			// 7        0
			// |?????ae?|
			//
			// a = ADC readout
			// e = EEPROM data out

			UINT32 eeprom_bit = (eeprom->read_bit() << 1);
			UINT32 adc_bit = (adc1038_do_read(adc1038) << 2);
			return (eeprom_bit | adc_bit);
		}

		default:
			mame_printf_debug("sysreg_r %d\n", offset);
			break;
	}
	return 0;
}

WRITE8_MEMBER(gticlub_state::sysreg_w)
{
	device_t *adc1038 = machine().device("adc1038");
	eeprom_device *eeprom = machine().device<eeprom_device>("eeprom");

	switch (offset)
	{
		case 0:
		case 1:
			gticlub_led_setreg(offset, data);
			break;

		case 3:
			eeprom->write_bit((data & 0x01) ? 1 : 0);
			eeprom->set_clock_line((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
			eeprom->set_cs_line((data & 0x04) ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 4:
			if (data & 0x80)	/* CG Board 1 IRQ Ack */
				machine().device("maincpu")->execute().set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);

			if (data & 0x40)	/* CG Board 0 IRQ Ack */
				machine().device("maincpu")->execute().set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

			adc1038_di_write(adc1038, (data >> 0) & 1);
			adc1038_clk_write(adc1038, (data >> 1) & 1);

			set_cgboard_id((data >> 4) & 0x3);
			break;
	}
}

/******************************************************************/

MACHINE_START_MEMBER(gticlub_state,gticlub)
{
	/* set conservative DRC options */
	ppcdrc_set_options(machine().device("maincpu"), PPCDRC_COMPATIBLE_OPTIONS);

	/* configure fast RAM regions for DRC */
	ppcdrc_add_fastram(machine().device("maincpu"), 0x00000000, 0x000fffff, FALSE, m_work_ram);
}

static ADDRESS_MAP_START( gticlub_map, AS_PROGRAM, 32, gticlub_state )
	AM_RANGE(0x00000000, 0x000fffff) AM_RAM AM_SHARE("work_ram")		/* Work RAM */
	AM_RANGE(0x74000000, 0x740000ff) AM_READWRITE(gticlub_k001604_reg_r, gticlub_k001604_reg_w)
	AM_RANGE(0x74010000, 0x7401ffff) AM_RAM_WRITE(paletteram32_w) AM_SHARE("paletteram")
	AM_RANGE(0x74020000, 0x7403ffff) AM_READWRITE(gticlub_k001604_tile_r, gticlub_k001604_tile_w)
	AM_RANGE(0x74040000, 0x7407ffff) AM_READWRITE(gticlub_k001604_char_r, gticlub_k001604_char_w)
	AM_RANGE(0x78000000, 0x7800ffff) AM_READWRITE_LEGACY(cgboard_dsp_shared_r_ppc, cgboard_dsp_shared_w_ppc)
	AM_RANGE(0x78040000, 0x7804000f) AM_READWRITE_LEGACY(K001006_0_r, K001006_0_w)
	AM_RANGE(0x78080000, 0x7808000f) AM_READWRITE_LEGACY(K001006_1_r, K001006_1_w)
	AM_RANGE(0x780c0000, 0x780c0003) AM_READWRITE_LEGACY(cgboard_dsp_comm_r_ppc, cgboard_dsp_comm_w_ppc)
	AM_RANGE(0x7e000000, 0x7e003fff) AM_READWRITE8(sysreg_r, sysreg_w, 0xffffffff)
	AM_RANGE(0x7e008000, 0x7e009fff) AM_DEVREADWRITE8_LEGACY("k056230", k056230_r, k056230_w, 0xffffffff)
	AM_RANGE(0x7e00a000, 0x7e00bfff) AM_DEVREADWRITE_LEGACY("k056230", lanc_ram_r, lanc_ram_w)
	AM_RANGE(0x7e00c000, 0x7e00c007) AM_DEVWRITE_LEGACY("k056800", k056800_host_w)
	AM_RANGE(0x7e00c000, 0x7e00c007) AM_DEVREAD_LEGACY("k056800", k056800_host_r)		// Hang Pilot
	AM_RANGE(0x7e00c008, 0x7e00c00f) AM_DEVREAD_LEGACY("k056800", k056800_host_r)
	AM_RANGE(0x7f000000, 0x7f3fffff) AM_ROM AM_REGION("user2", 0)	/* Data ROM */
	AM_RANGE(0x7f800000, 0x7f9fffff) AM_ROM AM_SHARE("share2")
	AM_RANGE(0x7fe00000, 0x7fffffff) AM_ROM AM_REGION("user1", 0) AM_SHARE("share2")	/* Program ROM */
ADDRESS_MAP_END

/**********************************************************************/

static ADDRESS_MAP_START( sound_memmap, AS_PROGRAM, 16, gticlub_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_DEVREADWRITE_LEGACY("k056800", k056800_sound_r, k056800_sound_w)
	AM_RANGE(0x400000, 0x400fff) AM_DEVREADWRITE_LEGACY("rfsnd", rf5c400_r, rf5c400_w)		/* Ricoh RF5C400 */
	AM_RANGE(0x580000, 0x580001) AM_WRITENOP
	AM_RANGE(0x600000, 0x600001) AM_WRITENOP
ADDRESS_MAP_END

/*****************************************************************************/

READ32_MEMBER(gticlub_state::dsp_dataram0_r)
{
	return m_sharc_dataram_0[offset] & 0xffff;
}

WRITE32_MEMBER(gticlub_state::dsp_dataram0_w)
{
	m_sharc_dataram_0[offset] = data;
}

READ32_MEMBER(gticlub_state::dsp_dataram1_r)
{
	return m_sharc_dataram_1[offset] & 0xffff;
}

WRITE32_MEMBER(gticlub_state::dsp_dataram1_w)
{
	m_sharc_dataram_1[offset] = data;
}

static ADDRESS_MAP_START( sharc_map, AS_DATA, 32, gticlub_state )
	AM_RANGE(0x400000, 0x41ffff) AM_READWRITE_LEGACY(cgboard_0_shared_sharc_r, cgboard_0_shared_sharc_w)
	AM_RANGE(0x500000, 0x5fffff) AM_READWRITE(dsp_dataram0_r, dsp_dataram0_w)
	AM_RANGE(0x600000, 0x6fffff) AM_READWRITE_LEGACY(K001005_r, K001005_w)
	AM_RANGE(0x700000, 0x7000ff) AM_READWRITE_LEGACY(cgboard_0_comm_sharc_r, cgboard_0_comm_sharc_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hangplt_sharc0_map, AS_DATA, 32, gticlub_state )
	AM_RANGE(0x0400000, 0x041ffff) AM_READWRITE_LEGACY(cgboard_0_shared_sharc_r, cgboard_0_shared_sharc_w)
	AM_RANGE(0x0500000, 0x05fffff) AM_READWRITE(dsp_dataram0_r, dsp_dataram0_w)
	AM_RANGE(0x1400000, 0x14fffff) AM_RAM
	AM_RANGE(0x2400000, 0x27fffff) AM_DEVREADWRITE_LEGACY("voodoo0", nwk_voodoo_0_r, voodoo_w)
	AM_RANGE(0x3400000, 0x34000ff) AM_READWRITE_LEGACY(cgboard_0_comm_sharc_r, cgboard_0_comm_sharc_w)
	AM_RANGE(0x3401000, 0x34fffff) AM_DEVWRITE_LEGACY("voodoo0", nwk_fifo_0_w)
	AM_RANGE(0x3500000, 0x3507fff) AM_READWRITE_LEGACY(K033906_0_r, K033906_0_w)
	AM_RANGE(0x3600000, 0x37fffff) AM_ROMBANK("bank5")
ADDRESS_MAP_END

static ADDRESS_MAP_START( hangplt_sharc1_map, AS_DATA, 32, gticlub_state )
	AM_RANGE(0x0400000, 0x041ffff) AM_READWRITE_LEGACY(cgboard_1_shared_sharc_r, cgboard_1_shared_sharc_w)
	AM_RANGE(0x0500000, 0x05fffff) AM_READWRITE(dsp_dataram1_r, dsp_dataram1_w)
	AM_RANGE(0x1400000, 0x14fffff) AM_RAM
	AM_RANGE(0x2400000, 0x27fffff) AM_DEVREADWRITE_LEGACY("voodoo1", nwk_voodoo_1_r, voodoo_w)
	AM_RANGE(0x3400000, 0x34000ff) AM_READWRITE_LEGACY(cgboard_1_comm_sharc_r, cgboard_1_comm_sharc_w)
	AM_RANGE(0x3401000, 0x34fffff) AM_DEVWRITE_LEGACY("voodoo1", nwk_fifo_1_w)
	AM_RANGE(0x3500000, 0x3507fff) AM_READWRITE_LEGACY(K033906_1_r, K033906_1_w)
	AM_RANGE(0x3600000, 0x37fffff) AM_ROMBANK("bank6")
ADDRESS_MAP_END

/*****************************************************************************/

static INPUT_PORTS_START( gticlub )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )		// View switch
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )		// Shift Down
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )		// Shift Up
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )		// AT/MT switch
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x0b, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
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

	PORT_START("AN0")	/* mask default type             sens delta min max */
	PORT_BIT( 0x3ff, 0x200, IPT_PADDLE ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("AN1")
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("AN2")
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL2 ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("AN3")
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL3 ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

INPUT_PORTS_END

static INPUT_PORTS_START( slrasslt )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )						// View Shift
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)		// Trigger
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)		// Missile
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)		// Power Up

	PORT_START("IN1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
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

	PORT_START("AN0")
	PORT_BIT( 0x3ff, 0x000, IPT_AD_STICK_Y ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("AN1")
	PORT_BIT( 0x3ff, 0x000, IPT_AD_STICK_X ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("AN2")
	PORT_BIT( 0x3ff, 0x000, IPT_UNUSED )

	PORT_START("AN3")
	PORT_BIT( 0x3ff, 0x000, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( thunderh )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
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
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)		// Push limit switch
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)		// Pull limit switch

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
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
	PORT_DIPNAME( 0x01, 0x01, "Disable Machine Init" )				// NOTE: Disabling Machine Init also disables analog controls
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("AN0")			// Rudder
	PORT_BIT( 0x3ff, 0x000, IPT_AD_STICK_X ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("AN1")			// Control Bar
	PORT_BIT( 0x3ff, 0x000, IPT_AD_STICK_Y ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("AN2")
	PORT_BIT( 0x3ff, 0x000, IPT_UNKNOWN )

	PORT_START("AN3")
	PORT_BIT( 0x3ff, 0x000, IPT_UNKNOWN )
INPUT_PORTS_END

/* PowerPC interrupts

    IRQ0:  Vblank
    IRQ2:  LANC
    DMA0

*/
INTERRUPT_GEN_MEMBER(gticlub_state::gticlub_vblank)
{
	device.execute().set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}


static const sharc_config sharc_cfg =
{
	BOOT_MODE_EPROM
};


TIMER_CALLBACK_MEMBER(gticlub_state::irq_off)
{
	machine().device("audiocpu")->execute().set_input_line(param, CLEAR_LINE);
}

static void sound_irq_callback( running_machine &machine, int irq )
{
	gticlub_state *state = machine.driver_data<gticlub_state>();
	int line = (irq == 0) ? INPUT_LINE_IRQ1 : INPUT_LINE_IRQ2;

	machine.device("audiocpu")->execute().set_input_line(line, ASSERT_LINE);
	machine.scheduler().timer_set(attotime::from_usec(1), timer_expired_delegate(FUNC(gticlub_state::irq_off),state), line);
}

static const k056800_interface gticlub_k056800_interface =
{
	sound_irq_callback
};


static int adc1038_input_callback( device_t *device, int input )
{
	int value = 0;
	switch (input)
	{
	case 0: value = device->machine().root_device().ioport("AN0")->read(); break;
	case 1: value = device->machine().root_device().ioport("AN1")->read(); break;
	case 2: value = device->machine().root_device().ioport("AN2")->read(); break;
	case 3: value = device->machine().root_device().ioport("AN3")->read(); break;
	case 4: value = 0x000; break;
	case 5: value = 0x000; break;
	case 6: value = 0x000; break;
	case 7: value = 0x000; break;
	}

	return value;
}

static const adc1038_interface gticlub_adc1038_intf =
{
	1,
	adc1038_input_callback
};

static const adc1038_interface thunderh_adc1038_intf =
{
	0,
	adc1038_input_callback
};

static const k056230_interface gticlub_k056230_intf =
{
	"maincpu",
	0
};

static const k056230_interface thunderh_k056230_intf =
{
	"maincpu",
	1
};

static const k001604_interface gticlub_k001604_intf =
{
	1, 2,	/* gfx index 1 & 2 */
	1, 1,		/* layer_size, roz_size */
	0		/* slrasslt hack */
};

static const k001604_interface slrasslt_k001604_intf =
{
	1, 2,	/* gfx index 1 & 2 */
	0, 0,		/* layer_size, roz_size */
	1		/* slrasslt hack */
};

static const k001604_interface hangplt_k001604_intf_l =
{
	1, 2,	/* gfx index 1 & 2 */
	0, 1,		/* layer_size, roz_size */
	0		/* slrasslt hack */
};

static const k001604_interface hangplt_k001604_intf_r =
{
	3, 4,	/* gfx index 1 & 2 */
	0, 1,		/* layer_size, roz_size */
	0		/* slrasslt hack */
};


MACHINE_RESET_MEMBER(gticlub_state,gticlub)
{
	machine().device("dsp")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

static MACHINE_CONFIG_START( gticlub, gticlub_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC403GA, 64000000/2)	/* PowerPC 403GA 32MHz */
	MCFG_CPU_PROGRAM_MAP(gticlub_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gticlub_state,  gticlub_vblank)

	MCFG_CPU_ADD("audiocpu", M68000, 64000000/4)	/* 16MHz */
	MCFG_CPU_PROGRAM_MAP(sound_memmap)

	MCFG_CPU_ADD("dsp", ADSP21062, 36000000)
	MCFG_CPU_CONFIG(sharc_cfg)
	MCFG_CPU_DATA_MAP(sharc_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_EEPROM_ADD("eeprom", eeprom_intf)

	MCFG_MACHINE_START_OVERRIDE(gticlub_state,gticlub)
	MCFG_MACHINE_RESET_OVERRIDE(gticlub_state,gticlub)

	MCFG_ADC1038_ADD("adc1038", gticlub_adc1038_intf)

	MCFG_K056230_ADD("k056230", gticlub_k056230_intf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 384)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 383)
	MCFG_SCREEN_UPDATE_STATIC(gticlub)

	MCFG_PALETTE_LENGTH(65536)

	MCFG_VIDEO_START(gticlub)

	MCFG_K001604_ADD("k001604_1", gticlub_k001604_intf)

	MCFG_K056800_ADD("k056800", gticlub_k056800_interface)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("rfsnd", RF5C400, 64000000/4)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( thunderh, gticlub )

	MCFG_DEVICE_REMOVE("adc1038")
	MCFG_ADC1038_ADD("adc1038", thunderh_adc1038_intf)

	MCFG_DEVICE_REMOVE("k056230")
	MCFG_K056230_ADD("k056230", thunderh_k056230_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( slrasslt, gticlub )

	MCFG_DEVICE_REMOVE("adc1038")
	MCFG_ADC1038_ADD("adc1038", thunderh_adc1038_intf)

	MCFG_DEVICE_REMOVE("k001604_1")
	MCFG_K001604_ADD("k001604_1", slrasslt_k001604_intf)
MACHINE_CONFIG_END


static const k033906_interface hangplt_k033906_intf_0 =
{
	"voodoo0"
};

static const k033906_interface hangplt_k033906_intf_1 =
{
	"voodoo1"
};

MACHINE_RESET_MEMBER(gticlub_state,hangplt)
{
	machine().device("dsp")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	machine().device("dsp2")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

static const voodoo_config voodoo_l_intf =
{
	2, //               fbmem;
	2,//                tmumem0;
	2,//                tmumem1;
	"lscreen",//        screen;
	"dsp",//            cputag;
	voodoo_vblank_0,//  vblank;
	NULL,//             stall;
};

static const voodoo_config voodoo_r_intf =
{
	2, //               fbmem;
	2,//                tmumem0;
	2,//                tmumem1;
	"rscreen",//        screen;
	"dsp2",//           cputag;
	voodoo_vblank_1,//  vblank;
	NULL,//             stall;
};

static MACHINE_CONFIG_START( hangplt, gticlub_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC403GA, 64000000/2)	/* PowerPC 403GA 32MHz */
	MCFG_CPU_PROGRAM_MAP(gticlub_map)

	MCFG_CPU_ADD("audiocpu", M68000, 64000000/4)	/* 16MHz */
	MCFG_CPU_PROGRAM_MAP(sound_memmap)

	MCFG_CPU_ADD("dsp", ADSP21062, 36000000)
	MCFG_CPU_CONFIG(sharc_cfg)
	MCFG_CPU_DATA_MAP(hangplt_sharc0_map)

	MCFG_CPU_ADD("dsp2", ADSP21062, 36000000)
	MCFG_CPU_CONFIG(sharc_cfg)
	MCFG_CPU_DATA_MAP(hangplt_sharc1_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_EEPROM_ADD("eeprom", eeprom_intf)

	MCFG_MACHINE_START_OVERRIDE(gticlub_state,gticlub)
	MCFG_MACHINE_RESET_OVERRIDE(gticlub_state,hangplt)

	MCFG_ADC1038_ADD("adc1038", thunderh_adc1038_intf)
	MCFG_K056230_ADD("k056230", gticlub_k056230_intf)

	MCFG_3DFX_VOODOO_1_ADD("voodoo0", STD_VOODOO_1_CLOCK, voodoo_l_intf)
	MCFG_3DFX_VOODOO_1_ADD("voodoo1", STD_VOODOO_1_CLOCK, voodoo_r_intf)

	MCFG_K033906_ADD("k033906_1", hangplt_k033906_intf_0)
	MCFG_K033906_ADD("k033906_2", hangplt_k033906_intf_1)

	/* video hardware */
	MCFG_PALETTE_LENGTH(65536)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 384)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 383)
	MCFG_SCREEN_UPDATE_STATIC(hangplt)

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 384)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 383)
	MCFG_SCREEN_UPDATE_STATIC(hangplt)

	MCFG_K001604_ADD("k001604_1", hangplt_k001604_intf_l)
	MCFG_K001604_ADD("k001604_2", hangplt_k001604_intf_r)

	MCFG_K056800_ADD("k056800", gticlub_k056800_interface)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("rfsnd", RF5C400, 64000000/4)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

/*************************************************************************/

ROM_START( gticlub ) /* Euro version EAA - Reports: GTI CLUB(TM) System ver 1.00(EUR) */
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE("688eaa01.21u", 0x000003, 0x80000, CRC(824944ad) SHA1(a7bb86a2495e0579f5d82808aeed4895be2dbe3b) )
	ROM_LOAD32_BYTE("688eaa02.19u", 0x000002, 0x80000, CRC(88e7bfb9) SHA1(fc0e945291204ee0c82bbd2c81ff241e1565c6ae) )
	ROM_LOAD32_BYTE("688eaa03.21r", 0x000001, 0x80000, CRC(ea1c696b) SHA1(fd778afaa1de3a35b38a67b8e4c9a08fe9cf1b9e) )
	ROM_LOAD32_BYTE("688eaa04.19r", 0x000000, 0x80000, CRC(94fa2334) SHA1(04edf840f841b9713fa93e7ebb6aad2000b738c0) )

	ROM_REGION32_BE(0x400000, "user2", 0)	/* data roms */
	ROM_LOAD32_WORD_SWAP("688a05.14u", 0x000000, 0x200000, CRC(7caa3f80) SHA1(28409dc17c4e010173396fdc069a409fbea0d58d) )
	ROM_LOAD32_WORD_SWAP("688a06.12u", 0x000002, 0x200000, CRC(83e7ce0a) SHA1(afe185f6ed700baaf4c8affddc29f8afdfec4423) )

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68k program */
	ROM_LOAD16_WORD_SWAP( "688a07.13k", 0x000000, 0x040000, CRC(f0805f06) SHA1(4b87e02b89e7ea812454498603767668e4619025) )

	ROM_REGION(0x800000, "rfsnd", 0)	/* sound roms */
	ROM_LOAD( "688a09.9s", 0x000000, 0x200000, CRC(fb582963) SHA1(ce8fe6a4d7ac7d7f4b6591f9150b1d351e636354) )
	ROM_LOAD( "688a10.7s", 0x200000, 0x200000, CRC(b3ddc5f1) SHA1(a3f76c86e85eb17f20efb037c1ad64e9cb8566c8) )
	ROM_LOAD( "688a11.5s", 0x400000, 0x200000, CRC(fc706183) SHA1(c8ce6de0588be1023ef48577bc88a4e5effdcd25) )
	ROM_LOAD( "688a12.2s", 0x600000, 0x200000, CRC(510c70e3) SHA1(5af77bc98772ab7961308c3af0a80cb1bca690e3) )

	ROM_REGION(0x800000, "gfx1", 0)	/* texture roms */
	ROM_LOAD64_WORD( "688a13.18d", 0x000000, 0x200000, CRC(c8f04f91) SHA1(9da8cc3a94dbf0a1fce87c2bc9249df712ae0b0d) )
	ROM_LOAD64_WORD( "688a14.13d", 0x000002, 0x200000, CRC(b9932735) SHA1(2492244d2acb350974202a6718bc7121325d2121) )
	ROM_LOAD64_WORD( "688a15.9d",  0x000004, 0x200000, CRC(8aadee51) SHA1(be9020a47583da9d4ff586d227836dc5b7dc31f0) )
	ROM_LOAD64_WORD( "688a16.4d",  0x000006, 0x200000, CRC(7f4e1893) SHA1(585be7b31ab7a48300c22b00443b00d631f4c49d) )

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "gticlub.nv", 0x0000, 0x0200, CRC(eca78a49) SHA1(3dcaccc4bee58f7ff5d6ecae551887cc967deaf7) )
ROM_END

ROM_START( gticlubu ) /* USA version UAA - Reports: GTI CLUB(TM) System ver 1.02(USA) */
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE("688uaa01.21u", 0x000003, 0x80000, CRC(4e2ea7ad) SHA1(cc517df7c4df098896a2a88843fef97c9beb46f3) )
	ROM_LOAD32_BYTE("688uaa02.19u", 0x000002, 0x80000, CRC(c0212ce1) SHA1(7716acfa1b1391e9d7a321ed46785c144d27fdd8) )
	ROM_LOAD32_BYTE("688uaa03.21r", 0x000001, 0x80000, CRC(030246fe) SHA1(70d3591159b07aaeca60141db44f7c28d1b2dac9) )
	ROM_LOAD32_BYTE("688uaa04.19r", 0x000000, 0x80000, CRC(9394e0b2) SHA1(9ff4ff22a307352bf127fc2b5ef9c56ecacf0aab) )

	ROM_REGION32_BE(0x400000, "user2", 0)	/* data roms */
	ROM_LOAD32_WORD_SWAP("688a05.14u", 0x000000, 0x200000, CRC(7caa3f80) SHA1(28409dc17c4e010173396fdc069a409fbea0d58d) )
	ROM_LOAD32_WORD_SWAP("688a06.12u", 0x000002, 0x200000, CRC(83e7ce0a) SHA1(afe185f6ed700baaf4c8affddc29f8afdfec4423) )

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68k program */
	ROM_LOAD16_WORD_SWAP( "688a07.13k", 0x000000, 0x040000, CRC(f0805f06) SHA1(4b87e02b89e7ea812454498603767668e4619025) )

	ROM_REGION(0x800000, "rfsnd", 0)	/* sound roms */
	ROM_LOAD( "688a09.9s", 0x000000, 0x200000, CRC(fb582963) SHA1(ce8fe6a4d7ac7d7f4b6591f9150b1d351e636354) )
	ROM_LOAD( "688a10.7s", 0x200000, 0x200000, CRC(b3ddc5f1) SHA1(a3f76c86e85eb17f20efb037c1ad64e9cb8566c8) )
	ROM_LOAD( "688a11.5s", 0x400000, 0x200000, CRC(fc706183) SHA1(c8ce6de0588be1023ef48577bc88a4e5effdcd25) )
	ROM_LOAD( "688a12.2s", 0x600000, 0x200000, CRC(510c70e3) SHA1(5af77bc98772ab7961308c3af0a80cb1bca690e3) )

	ROM_REGION(0x800000, "gfx1", 0)	/* texture roms */
	ROM_LOAD64_WORD( "688a13.18d", 0x000000, 0x200000, CRC(c8f04f91) SHA1(9da8cc3a94dbf0a1fce87c2bc9249df712ae0b0d) )
	ROM_LOAD64_WORD( "688a14.13d", 0x000002, 0x200000, CRC(b9932735) SHA1(2492244d2acb350974202a6718bc7121325d2121) )
	ROM_LOAD64_WORD( "688a15.9d",  0x000004, 0x200000, CRC(8aadee51) SHA1(be9020a47583da9d4ff586d227836dc5b7dc31f0) )
	ROM_LOAD64_WORD( "688a16.4d",  0x000006, 0x200000, CRC(7f4e1893) SHA1(585be7b31ab7a48300c22b00443b00d631f4c49d) )

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "gticlub.nv", 0x0000, 0x0200, CRC(eca78a49) SHA1(3dcaccc4bee58f7ff5d6ecae551887cc967deaf7) )
ROM_END

ROM_START( gticluba ) /* Asia version AAA - Reports: GTI CLUB(TM) System ver 1.00(ASI) */
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE("688aaa01.21u", 0x000003, 0x80000, CRC(06a56474) SHA1(3a457b885a35e3ee030fd51d847bcf75fce46208) )
	ROM_LOAD32_BYTE("688aaa02.19u", 0x000002, 0x80000, CRC(3c1e714a) SHA1(557f8542b855b2b35f242c8db7396017aca6dbd8) )
	ROM_LOAD32_BYTE("688aaa03.21r", 0x000001, 0x80000, CRC(e060580b) SHA1(50242f3f3b949cc03082e4e75d9dcc89e17f0a75) )
	ROM_LOAD32_BYTE("688aaa04.19r", 0x000000, 0x80000, CRC(928c23cd) SHA1(cce54398e1e5b98bfb717839cc422f1f60502788) )

	ROM_REGION32_BE(0x400000, "user2", 0)	/* data roms */
	ROM_LOAD32_WORD_SWAP("688a05.14u", 0x000000, 0x200000, CRC(7caa3f80) SHA1(28409dc17c4e010173396fdc069a409fbea0d58d) )
	ROM_LOAD32_WORD_SWAP("688a06.12u", 0x000002, 0x200000, CRC(83e7ce0a) SHA1(afe185f6ed700baaf4c8affddc29f8afdfec4423) )

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68k program */
	ROM_LOAD16_WORD_SWAP( "688a07.13k", 0x000000, 0x040000, CRC(f0805f06) SHA1(4b87e02b89e7ea812454498603767668e4619025) )

	ROM_REGION(0x800000, "rfsnd", 0)	/* sound roms */
	ROM_LOAD( "688a09.9s", 0x000000, 0x200000, CRC(fb582963) SHA1(ce8fe6a4d7ac7d7f4b6591f9150b1d351e636354) )
	ROM_LOAD( "688a10.7s", 0x200000, 0x200000, CRC(b3ddc5f1) SHA1(a3f76c86e85eb17f20efb037c1ad64e9cb8566c8) )
	ROM_LOAD( "688a11.5s", 0x400000, 0x200000, CRC(fc706183) SHA1(c8ce6de0588be1023ef48577bc88a4e5effdcd25) )
	ROM_LOAD( "688a12.2s", 0x600000, 0x200000, CRC(510c70e3) SHA1(5af77bc98772ab7961308c3af0a80cb1bca690e3) )

	ROM_REGION(0x800000, "gfx1", 0)	/* texture roms */
	ROM_LOAD64_WORD( "688a13.18d", 0x000000, 0x200000, CRC(c8f04f91) SHA1(9da8cc3a94dbf0a1fce87c2bc9249df712ae0b0d) )
	ROM_LOAD64_WORD( "688a14.13d", 0x000002, 0x200000, CRC(b9932735) SHA1(2492244d2acb350974202a6718bc7121325d2121) )
	ROM_LOAD64_WORD( "688a15.9d",  0x000004, 0x200000, CRC(8aadee51) SHA1(be9020a47583da9d4ff586d227836dc5b7dc31f0) )
	ROM_LOAD64_WORD( "688a16.4d",  0x000006, 0x200000, CRC(7f4e1893) SHA1(585be7b31ab7a48300c22b00443b00d631f4c49d) )

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "gticlub.nv", 0x0000, 0x0200, CRC(eca78a49) SHA1(3dcaccc4bee58f7ff5d6ecae551887cc967deaf7) )
ROM_END

ROM_START( gticlubj ) /* Japan version JAA - Reports: GTI CLUB(TM) System ver 1.00(JPN) */
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE("688jaa01.21u", 0x000003, 0x80000, CRC(1492059c) SHA1(176dbd87f23f4cd8e1397e67da501738e20e5a57) )
	ROM_LOAD32_BYTE("688jaa02.19u", 0x000002, 0x80000, CRC(7896dd69) SHA1(a3ab7b872132a5e66238e414f4b497cf7beb8b1c) )
	ROM_LOAD32_BYTE("688jaa03.21r", 0x000001, 0x80000, CRC(94e2be50) SHA1(f206ac201903f3aae29196ab6fccdef104859346) )
	ROM_LOAD32_BYTE("688jaa04.19r", 0x000000, 0x80000, CRC(ff539bb6) SHA1(1a225eca4377d82a2b6cb99c1d16580b9ccf2f08) )

	ROM_REGION32_BE(0x400000, "user2", 0)	/* data roms */
	ROM_LOAD32_WORD_SWAP("688a05.14u", 0x000000, 0x200000, CRC(7caa3f80) SHA1(28409dc17c4e010173396fdc069a409fbea0d58d) )
	ROM_LOAD32_WORD_SWAP("688a06.12u", 0x000002, 0x200000, CRC(83e7ce0a) SHA1(afe185f6ed700baaf4c8affddc29f8afdfec4423) )

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68k program */
	ROM_LOAD16_WORD_SWAP( "688a07.13k", 0x000000, 0x040000, CRC(f0805f06) SHA1(4b87e02b89e7ea812454498603767668e4619025) )

	ROM_REGION(0x800000, "rfsnd", 0)	/* sound roms */
	ROM_LOAD( "688a09.9s", 0x000000, 0x200000, CRC(fb582963) SHA1(ce8fe6a4d7ac7d7f4b6591f9150b1d351e636354) )
	ROM_LOAD( "688a10.7s", 0x200000, 0x200000, CRC(b3ddc5f1) SHA1(a3f76c86e85eb17f20efb037c1ad64e9cb8566c8) )
	ROM_LOAD( "688a11.5s", 0x400000, 0x200000, CRC(fc706183) SHA1(c8ce6de0588be1023ef48577bc88a4e5effdcd25) )
	ROM_LOAD( "688a12.2s", 0x600000, 0x200000, CRC(510c70e3) SHA1(5af77bc98772ab7961308c3af0a80cb1bca690e3) )

	ROM_REGION(0x800000, "gfx1", 0)	/* texture roms */
	ROM_LOAD64_WORD( "688a13.18d", 0x000000, 0x200000, CRC(c8f04f91) SHA1(9da8cc3a94dbf0a1fce87c2bc9249df712ae0b0d) )
	ROM_LOAD64_WORD( "688a14.13d", 0x000002, 0x200000, CRC(b9932735) SHA1(2492244d2acb350974202a6718bc7121325d2121) )
	ROM_LOAD64_WORD( "688a15.9d",  0x000004, 0x200000, CRC(8aadee51) SHA1(be9020a47583da9d4ff586d227836dc5b7dc31f0) )
	ROM_LOAD64_WORD( "688a16.4d",  0x000006, 0x200000, CRC(7f4e1893) SHA1(585be7b31ab7a48300c22b00443b00d631f4c49d) )

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "gticlub.nv", 0x0000, 0x0200, CRC(eca78a49) SHA1(3dcaccc4bee58f7ff5d6ecae551887cc967deaf7) )
ROM_END

ROM_START( thunderh ) /* Euro version EAA */
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE( "680eaa01.21u", 0x000003, 0x080000, CRC(796e2678) SHA1(8051a228aa6d1a3f1fef26de15f4fdb785c2c8ee) )
	ROM_LOAD32_BYTE( "680eaa02.19u", 0x000002, 0x080000, CRC(767e6db0) SHA1(0f29f56fe485f30100ce54e64bda5d5a124c1d09) )
	ROM_LOAD32_BYTE( "680eaa03.21r", 0x000001, 0x080000, CRC(5a5b59b5) SHA1(542c0722437f40829559b09120fde995246d52ae) )
	ROM_LOAD32_BYTE( "680eaa04.19r", 0x000000, 0x080000, CRC(4a973a5c) SHA1(1d84f6416c3b5a85d7ebfbc15fc08e0dd8dc2414) )

	ROM_REGION32_BE(0x400000, "user2", 0)	/* data roms */
	ROM_LOAD32_WORD_SWAP( "680a05.14u", 0x000000, 0x200000, CRC(0c9f334d) SHA1(99ac622a04a7140244d81031df69a796b6fd2657) )
	ROM_LOAD32_WORD_SWAP( "680a06.12u", 0x000002, 0x200000, CRC(83074217) SHA1(bbf782ac125cd98d9147ef4e0373bf61f74726f7) )

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68k program */
	ROM_LOAD16_WORD_SWAP( "680a07.13k", 0x000000, 0x080000, CRC(12247a3e) SHA1(846cd9423efd3c9b17fce08393c6c83307d72f92) )

	ROM_REGION(0x20000, "dsp", 0)		/* 68k program for outboard sound? network? board */
	ROM_LOAD16_WORD_SWAP( "680c22.20k", 0x000000, 0x020000, CRC(d93c0ee2) SHA1(4b58418cbb01b51e12d6e7c86b2c81cd35d86248) )

	ROM_REGION(0x800000, "rfsnd", 0)	/* sound roms */
	ROM_LOAD( "680a09.9s", 0x000000, 0x200000, CRC(71c2b049) SHA1(ce360172c8774b31edf16a80104c35b1caf26cd9) )
	ROM_LOAD( "680a10.7s", 0x200000, 0x200000, CRC(19882bf3) SHA1(7287da58853c84cbadbfb42bed37f2b0032c4b4d) )
	ROM_LOAD( "680a11.5s", 0x400000, 0x200000, CRC(0c74fe3f) SHA1(2e69f8d37552a74bbda65b134f747b4380ed33b0) )
	ROM_LOAD( "680a12.2s", 0x600000, 0x200000, CRC(b052919d) SHA1(a61c8eaf378ab7d780478db61217302d1b9f8f73) )

	ROM_REGION(0x800000, "gfx1", 0)	/* texture roms */
	ROM_LOAD64_WORD( "680a13.18d", 0x000000, 0x200000, CRC(233f9074) SHA1(78ce42c35407d61df37cc0d16cdee84f8de968fa) )
	ROM_LOAD64_WORD( "680a14.13d", 0x000002, 0x200000, CRC(9ae15033) SHA1(12e291114629632b81f53811a6c8666aff4e92f3) )
	ROM_LOAD64_WORD( "680a15.9d",  0x000004, 0x200000, CRC(dc47c86f) SHA1(71af9b21f1ecc063135f501b1561869ee910c236) )
	ROM_LOAD64_WORD( "680a16.4d",  0x000006, 0x200000, CRC(ea388143) SHA1(3de5314a009d702186d5e285c8edefdd48139eab) )
ROM_END

ROM_START( thunderhu ) /* USA version UAA */
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE( "680uaa01.21u", 0x000003, 0x080000, CRC(f2bb2ba1) SHA1(311e88d63179486014376c4af4ff0ef28673ee5a) )
	ROM_LOAD32_BYTE( "680uaa02.19u", 0x000002, 0x080000, CRC(52f617b5) SHA1(fda3133d3a7e04eb4432c69becdcf1872b3660d9) )
	ROM_LOAD32_BYTE( "680uaa03.21r", 0x000001, 0x080000, CRC(086a0574) SHA1(32fb93dbb93d2fe6af743ea4310b50a6cd03647d) )
	ROM_LOAD32_BYTE( "680uaa04.19r", 0x000000, 0x080000, CRC(85e1f8e3) SHA1(9172c54b6663f1bf390795068271198083a6860d) )

	ROM_REGION32_BE(0x400000, "user2", 0)	/* data roms */
	ROM_LOAD32_WORD_SWAP( "680a05.14u", 0x000000, 0x200000, CRC(0c9f334d) SHA1(99ac622a04a7140244d81031df69a796b6fd2657) )
	ROM_LOAD32_WORD_SWAP( "680a06.12u", 0x000002, 0x200000, CRC(83074217) SHA1(bbf782ac125cd98d9147ef4e0373bf61f74726f7) )

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68k program */
	ROM_LOAD16_WORD_SWAP( "680a07.13k", 0x000000, 0x080000, CRC(12247a3e) SHA1(846cd9423efd3c9b17fce08393c6c83307d72f92) )

	ROM_REGION(0x20000, "dsp", 0)		/* 68k program for outboard sound? network? board */
	ROM_LOAD16_WORD_SWAP( "680c22.20k", 0x000000, 0x020000, CRC(d93c0ee2) SHA1(4b58418cbb01b51e12d6e7c86b2c81cd35d86248) )

	ROM_REGION(0x800000, "rfsnd", 0)	/* sound roms */
	ROM_LOAD( "680a09.9s", 0x000000, 0x200000, CRC(71c2b049) SHA1(ce360172c8774b31edf16a80104c35b1caf26cd9) )
	ROM_LOAD( "680a10.7s", 0x200000, 0x200000, CRC(19882bf3) SHA1(7287da58853c84cbadbfb42bed37f2b0032c4b4d) )
	ROM_LOAD( "680a11.5s", 0x400000, 0x200000, CRC(0c74fe3f) SHA1(2e69f8d37552a74bbda65b134f747b4380ed33b0) )
	ROM_LOAD( "680a12.2s", 0x600000, 0x200000, CRC(b052919d) SHA1(a61c8eaf378ab7d780478db61217302d1b9f8f73) )

	ROM_REGION(0x800000, "gfx1", 0)	/* texture roms */
	ROM_LOAD64_WORD( "680a13.18d", 0x000000, 0x200000, CRC(233f9074) SHA1(78ce42c35407d61df37cc0d16cdee84f8de968fa) )
	ROM_LOAD64_WORD( "680a14.13d", 0x000002, 0x200000, CRC(9ae15033) SHA1(12e291114629632b81f53811a6c8666aff4e92f3) )
	ROM_LOAD64_WORD( "680a15.9d",  0x000004, 0x200000, CRC(dc47c86f) SHA1(71af9b21f1ecc063135f501b1561869ee910c236) )
	ROM_LOAD64_WORD( "680a16.4d",  0x000006, 0x200000, CRC(ea388143) SHA1(3de5314a009d702186d5e285c8edefdd48139eab) )
ROM_END

ROM_START( slrasslt ) /* USA version UAA */
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE( "792uaa01.21u", 0x000003, 0x080000, CRC(c73bf7fb) SHA1(ffe0fea155473827929339a9261a158287ce30a8) )
	ROM_LOAD32_BYTE( "792uaa02.19u", 0x000002, 0x080000, CRC(a940bb9b) SHA1(65a60157697a21cc2485c02c689c9addb3ac91f1) )
	ROM_LOAD32_BYTE( "792uaa03.21r", 0x000001, 0x080000, CRC(363e8411) SHA1(b9c70033d8e3de4b339b61a66172bfecb7c2b3ab) )
	ROM_LOAD32_BYTE( "792uaa04.19r", 0x000000, 0x080000, CRC(7910d99c) SHA1(e2114d369060528998b58331d590c086d306f541) )

	ROM_REGION32_BE(0x400000, "user2", 0)	/* data roms */
	ROM_LOAD32_WORD_SWAP( "792a05.14u", 0x000000, 0x200000, CRC(9a27edfc) SHA1(c028b6440eb1b0c814c4db45918e580662ac2d9a) )
	ROM_LOAD32_WORD_SWAP( "792a06.12u", 0x000002, 0x200000, CRC(c272f171) SHA1(df492287eadc5e8668fe46cfa3ed3ca77c57feca) )

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68k program */
	ROM_LOAD16_WORD_SWAP( "792a07.10k", 0x000000, 0x080000, CRC(89a65ad1) SHA1(d814ef0b560c8e68da57ad5c6096e4fc05e9913e) )

	ROM_REGION(0x800000, "rfsnd", 0)	/* sound roms */
	ROM_LOAD( "792a09.9s", 0x000000, 0x200000, CRC(7d7ea427) SHA1(a9a311a7c17223cc87140fe2890e20a321464831) )
	ROM_LOAD( "792a10.7s", 0x200000, 0x200000, CRC(e585e5d9) SHA1(ec44ad324a66eeea4c45933dda5a8a9a4398879d) )
	ROM_LOAD( "792a11.5s", 0x400000, 0x200000, CRC(c9c3a04c) SHA1(f834659f67712c9fcd93b7407669d7f35517b790) )
	ROM_LOAD( "792a12.2s", 0x600000, 0x200000, CRC(da8fcdd5) SHA1(daa7b3a086ada69e93c3d7cd9130befc79e422dc) )

	ROM_REGION(0x800000, "gfx1", 0)	/* texture roms */
	ROM_LOAD64_WORD( "792a13.18d", 0x000000, 0x200000, CRC(16d6a134) SHA1(3f53f3c6759d7c5f40aa25a598df899fbac35a60) )
	ROM_LOAD64_WORD( "792a14.13d", 0x000002, 0x200000, CRC(cf57e830) SHA1(607b4dec3b8180a63e29d9dab1ca28d7226dda1e) )
	ROM_LOAD64_WORD( "792a15.9d",  0x000004, 0x200000, CRC(1c5531cb) SHA1(1b514f181c92e16d07bfe4719604f1e4caf15377) )
	ROM_LOAD64_WORD( "792a16.4d",  0x000006, 0x200000, CRC(df89e392) SHA1(af37c5460d43bf8d8a1ab4213c4528083a7363c2) )

	ROM_REGION16_BE(0x200, "eeprom", 0) /* default eeprom with magic number */
	ROM_LOAD16_WORD( "eeprom-slrasslt.bin", 0x0000, 0x0200, CRC(924b4ed8) SHA1(247bf0c1394cbab3af03c26b9c016302b9b5723c) )
ROM_END

ROM_START( hangplt ) /* Japan version JAB */
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE( "685jab01.21u", 0x000003, 0x080000, CRC(f98a3e82) SHA1(94ebaa172b0e98c5cd08efaea5f56e707e5032b4) )
	ROM_LOAD32_BYTE( "685jab02.19u", 0x000002, 0x080000, CRC(20730cdc) SHA1(71b2cf7077ab7db875f9030e21afd05905f57ce5) )
	ROM_LOAD32_BYTE( "685jab03.21r", 0x000001, 0x080000, CRC(77fa2248) SHA1(a662b84945b3d268fed15952cc793d821233735e) )
	ROM_LOAD32_BYTE( "685jab04.19r", 0x000000, 0x080000, CRC(ab6773df) SHA1(91d3f849a1cc5fa4b2fbd876d53402a548198c41) )

	ROM_REGION32_BE(0x400000, "user2", 0)	/* data roms */
	ROM_LOAD32_WORD_SWAP( "685a05.14u", 0x000000, 0x200000, CRC(ba1c8f40) SHA1(ce4ed641c1d6d44447eaaada16f305f1d7fb9ee2) )
	ROM_LOAD32_WORD_SWAP( "685a06.12u", 0x000002, 0x200000, CRC(2429935c) SHA1(4da9e169adcac81ea1bc135d727c2bd13ad372fa) )

	ROM_REGION(0x80000, "audiocpu", 0)	/* 68k program */
	ROM_LOAD16_WORD_SWAP( "685a07.13k", 0x000000, 0x080000, CRC(5b72fd80) SHA1(a150837fa0d66dc0c3832495a4c8ce4f9b92cd98) )

	ROM_REGION(0x1000000, "rfsnd", 0)	/* sound roms */
	ROM_LOAD( "685a09.9s", 0x000000, 0x400000, CRC(b8ae40aa) SHA1(eee27a8929e0e805f1045fd9638e661b36a1e3c7) )
	ROM_LOAD( "685a10.7s", 0x400000, 0x400000, CRC(fef3dc36) SHA1(566c7469fc452b5965a31fa42291082ec8e48a24) )

	ROM_REGION(0x800000, "user5", 0)	/* texture roms */
	ROM_LOAD32_WORD( "685a13.4w",  0x000002, 0x400000, CRC(06329af4) SHA1(76cad9db604751ce48bb67bfd29e57bac0ee9a16) )
	ROM_LOAD32_WORD( "685a14.12w", 0x000000, 0x400000, CRC(87437739) SHA1(0d45637af40938a54d5efd29c125b0fafd55f9a4) )

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "hangplt.nv", 0x0000, 0x0200, CRC(35f482c8) SHA1(445918156770449dce1a010aab9d310f15670092) )
ROM_END

ROM_START( hangpltu ) /* USA version UAA */
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE( "685uaa01.21u", 0x000003, 0x080000, CRC(83a5b866) SHA1(6859590f212c7debb19924f0174e4cd1bfc011bc) )
	ROM_LOAD32_BYTE( "685uaa02.19u", 0x000002, 0x080000, CRC(765906d6) SHA1(9085a2346756b3b628fef91b7afc131aba434654) )
	ROM_LOAD32_BYTE( "685uaa03.21r", 0x000001, 0x080000, CRC(cb0147a3) SHA1(7bcab760c01ea7e24f4ca5793e081aafa97f68a3) )
	ROM_LOAD32_BYTE( "685uaa04.19r", 0x000000, 0x080000, CRC(a5fda56b) SHA1(4d86f488f411ec16fa3be830206a44214941d1fe) )

	ROM_REGION32_BE(0x400000, "user2", 0)	/* data roms */
	ROM_LOAD32_WORD_SWAP( "685a05.14u", 0x000000, 0x200000, CRC(ba1c8f40) SHA1(ce4ed641c1d6d44447eaaada16f305f1d7fb9ee2) )
	ROM_LOAD32_WORD_SWAP( "685a06.12u", 0x000002, 0x200000, CRC(2429935c) SHA1(4da9e169adcac81ea1bc135d727c2bd13ad372fa) )

	ROM_REGION(0x80000, "audiocpu", 0)	/* 68k program */
	ROM_LOAD16_WORD_SWAP( "685a07.13k", 0x000000, 0x080000, CRC(5b72fd80) SHA1(a150837fa0d66dc0c3832495a4c8ce4f9b92cd98) )

	ROM_REGION(0x1000000, "rfsnd", 0)	/* sound roms */
	ROM_LOAD( "685a09.9s", 0x000000, 0x400000, CRC(b8ae40aa) SHA1(eee27a8929e0e805f1045fd9638e661b36a1e3c7) )
	ROM_LOAD( "685a10.7s", 0x400000, 0x400000, CRC(fef3dc36) SHA1(566c7469fc452b5965a31fa42291082ec8e48a24) )

	ROM_REGION(0x800000, "user5", 0)	/* texture roms */
	ROM_LOAD32_WORD( "685a13.4w",  0x000002, 0x400000, CRC(06329af4) SHA1(76cad9db604751ce48bb67bfd29e57bac0ee9a16) )
	ROM_LOAD32_WORD( "685a14.12w", 0x000000, 0x400000, CRC(87437739) SHA1(0d45637af40938a54d5efd29c125b0fafd55f9a4) )

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "hangpltu.nv", 0x0000, 0x0200, CRC(8d74baf0) SHA1(297c0a064c6f8f8281d566629d896b49c6e85096) )
ROM_END


DRIVER_INIT_MEMBER(gticlub_state,gticlub)
{
	init_konami_cgboard(machine(), 1, CGBOARD_TYPE_GTICLUB);

	m_sharc_dataram_0 = auto_alloc_array(machine(), UINT32, 0x100000/4);

	K001005_preprocess_texture_data(memregion("gfx1")->base(), memregion("gfx1")->bytes(), 1);
}

void gticlub_state::init_hangplt_common()
{
	init_konami_cgboard(machine(), 2, CGBOARD_TYPE_HANGPLT);
	set_cgboard_texture_bank(machine(), 0, "bank5", memregion("user5")->base());
	set_cgboard_texture_bank(machine(), 1, "bank6", memregion("user5")->base());

	m_sharc_dataram_0 = auto_alloc_array(machine(), UINT32, 0x100000/4);
	m_sharc_dataram_1 = auto_alloc_array(machine(), UINT32, 0x100000/4);
}

DRIVER_INIT_MEMBER(gticlub_state,hangplt)
{
	init_hangplt_common();

	// workaround for lock/unlock errors
	UINT32 *rom = (UINT32*)machine().root_device().memregion("user1")->base();
	rom[(0x153ac^4) / 4] = 0x4e800020;
	rom[(0x15428^4) / 4] = 0x4e800020;
}

DRIVER_INIT_MEMBER(gticlub_state,hangpltu)
{
	init_hangplt_common();

	// workaround for lock/unlock errors
	UINT32 *rom = (UINT32*)machine().root_device().memregion("user1")->base();
	rom[(0x153d0^4) / 4] = 0x4e800020;
	rom[(0x15428^4) / 4] = 0x4e800020;
}

/*************************************************************************/

GAME( 1996, gticlub,  0,        gticlub,  gticlub,  gticlub_state, gticlub,  ROT0, "Konami", "GTI Club (ver EAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1996, gticlubu, gticlub,  gticlub,  gticlub,  gticlub_state, gticlub,  ROT0, "Konami", "GTI Club (ver UAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1996, gticluba, gticlub,  gticlub,  gticlub,  gticlub_state, gticlub,  ROT0, "Konami", "GTI Club (ver AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1996, gticlubj, gticlub,  gticlub,  gticlub,  gticlub_state, gticlub,  ROT0, "Konami", "GTI Club (ver JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1996, thunderh, 0,        thunderh, thunderh, gticlub_state, gticlub,  ROT0, "Konami", "Operation Thunder Hurricane (ver EAA)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND )
GAME( 1996, thunderhu,thunderh, thunderh, thunderh, gticlub_state, gticlub,  ROT0, "Konami", "Operation Thunder Hurricane (ver UAA)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND )
GAME( 1997, slrasslt, 0,        slrasslt, slrasslt, gticlub_state, gticlub,  ROT0, "Konami", "Solar Assault (ver UAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEL(1997, hangplt,  0,        hangplt,  hangplt,  gticlub_state, hangplt,  ROT0, "Konami", "Hang Pilot (ver JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND, layout_dualhovu )
GAMEL(1997, hangpltu, hangplt,  hangplt,  hangplt,  gticlub_state, hangpltu, ROT0, "Konami", "Hang Pilot (ver UAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND, layout_dualhovu )
