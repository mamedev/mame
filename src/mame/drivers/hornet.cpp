// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*  Konami Hornet System
    Driver by Ville Linde


    Konami 'Hornet' Hardware
    Konami, 1997-1999

    Known games on this hardware include....

    Game                             (C)      Year
    ----------------------------------------------
    Gradius 4 : Fukkatsu             Konami   1999
    NBA Play by Play                 Konami   1998
    Silent Scope                     Konami   1999
    Silent Scope 2 : Fatal Judgement Konami   2000
    Silent Scope 2 : Dark Silhouette Konami   2000
    Terraburst                       Konami   1998

    Hardware overview:

    GN715 CPU Board:
    ----------------
        IBM PowerPC 403GA at 32MHz (main CPU)
        Motorola MC68EC000 at 16MHz (sound CPU)
        Konami K056800 (MIRAC), sound system interface
        Ricoh RF5c400 sound chip

    GN715 GFX Board:
    ----------------
        Analog Devices ADSP-21062 SHARC DSP at 36MHz
        Konami 0000037122 (2D Tilemap)
        Konami 0000033906 (PCI bridge)
        3DFX 500-0003-03 (Voodoo) FBI with 2MB RAM
        3DFX 500-0004-02 (Voodoo) TMU with 2MB RAM

    GQ871 GFX Board:
    ----------------
        Analog Devices ADSP-21062 SHARC DSP at 36MHz
        Konami 0000037122 (2D Tilemap)
        Konami 0000033906 (PCI bridge)
        3DFX 500-0009-01 (Voodoo 2) FBI with 2MB RAM
        3DFX 500-0010-01 (Voodoo 2) TMU with 4MB RAM


    Hardware configurations:
    ------------------------

    Game              KONAMI ID  CPU PCB    GFX Board(s)  LAN PCB
    --------------------------------------------------------------
    Gradius 4         GX837      GN715(A)   GN715(B)
    NBA Play By Play  GX778      GN715(A)   GN715(B)
    Silent Scope      GQ830      GN715(A)   2x GN715(B)
    Silent Scope 2    GQ931      GN715(A)   2x GQ871(B)   GQ931(H)


    PCB Layouts
    -----------

    Top Board
    GN715 PWB(A)A
    |--------------------------------------------------------------|
    | SP485CS CN10       CN11        CN9          JP8 JP9 JP10 JP11|
    |CN19                                                    PAL1  |
    |CN21       JP13 PAL2             68EC000          EPROM.7S    |
    |   NE5532       PAL3                                      CN12|
    |           JP12  JP16    DRM1M4SJ8                        CN13|
    |   NE5532                            MASKROM.9P    MASKROM.9T |
    |     SM5877 JP15         RF5C400                              |
    |CN18                                 MASKROM.12P   MASKROM.12T|
    |     SM5877     16.9344MHz                                    |
    |CN14            SRAM256K             MASKROM.14P   MASKROM.14T|
    |                                                              |
    |CN16            SRAM256K             MASKROM.16P   MASKROM.16T|
    |  ADC12138                                                    |
    |             056800            JP5                            |
    |                               JP4                            |
    |                      MACH111  JP3                |---------| |
    |   TEST_SW                         EPROM.22P      |         | |
    |CN1                   DRAM16X16                   |PPC403GA | |
    |                                   EPROM.25P      |         | |
    |                                                  |         | |
    |                      DRAM16X16    EPROM.27P      |---------| |
    | 4AK16                                                     JP6|
    |                                                              |
    |CN3                                                           |
    | 0038323  PAL4                                       7.3728MHz|
    | E9825    058232           CN2                                |
    |                                                     50.000MHz|
    |    RESET_SW               CN5                    JP1  JP2    |
    |M48T58Y-70PC1  CN4                 CN6               64.000MHz|
    |--------------------------------------------------------------|
    Notes:
          DRM1M4SJ8 - Fujitsu 81C4256 256Kx4 DRAM (SOJ24)
           SRAM256K - Cypress CY7C199 32kx8 SRAM (SOJ28)
          DRAM16X16 - Fujitsu 8118160A-60 16megx16 DRAM (SOJ42)
      0038323 E9825 - SOIC8 (Secured PIC?). I've seen a similar chip in the security cart of System573
      M48T58Y-70PC1 - ST Timekeeper RAM
            RF5C400 - Ricoh RF5C400 PCM 32Ch, 44.1 kHz Stereo, 3D Effect Spatializer, clock input 16.9344MHz
             056800 - Konami Custom (QFP80)
             058232 - Konami Custom Ceramic Package (SIL14)
           ADC12138 - National Semiconductor ADC12138 A/D Converter, 12-bit + Serial I/O With MUX (SOP28)
            MACH111 - AMD MACH111 CPLD (Stamped 'N676A1', PLCC44)
            68EC000 - Motorola MC68EC000, running at 16.0MHz (64/4)
           PPC403GA - IBM PowerPC 403GA CPU, clock input 32.0MHz (QFP160)
           SM5877AM - Nippon Precision Circuits 3rd Order 2-Channel D/A Converter (SOIC24)
              4AK16 - Hitachi 4AK16 Silicon N-Channel Power MOS FET Array (SIL10)
           NE5532AN - Philips, Dual Low-Noise High-Speed Audio OP Amp (DIP8)
            SP485CS - Sipex SP485CS Low Power Half Duplex RS485 Transceiver (DIP8)
               PAL1 - AMD PALCE16V8 (stamped 'N676A4', DIP20)
               PAL2 - AMD PALCE16V8 (stamped 'N676A2', DIP20)
               PAL3 - AMD PALCE16V8 (stamped 'N676A3', DIP20)
               PAL4 - AMD PALCE16V8 (stamped 'N676A5', DIP20)
                JP1 -       25M O O-O 32M
                JP2 -       25M O O-O 32M
                JP3 -        RW O O O RO
                JP4 - PROG  32M O O-O 16M
                JP5 - DATA  32M O-O O 16M
                JP6 - BOOT   16 O-O O 32
                JP7 - SRC DOUT2 O O-O 0
                JP8 -   64M&32M O-O O 16M
                JP9 -       64M O O-O 32M&16M
               JP10 -   64M&32M O-O O 16M
               JP11 -       64M O O-O 32M&16M
               JP12 -   through O-O O SP
               JP13 -   through O-O O SP
               JP14 - WDT       O O
               JP15 -      MONO O-O O SURR
               JP16 -      HIGH O O O MID (N/C LOW)
        CN1 to  CN3 - Multi-pin Flat Cable Connector
                CN4 - Multi-pin Connector for Network PCB
                CN5 - Multi-pin Flat Cable Connector
                CN6 - 96-Pin To Lower PCB, Joining Connector
        CN7 to  CN8 - Not used
        CN9 to CN11 - 6-Pin Power Connectors
               CN19 - USB Connector
               CN21 - 5-Pin Analog Controls Connector (Tied to USB Connector via the Filter Board)
               CN18 - RCA Mono Audio OUT
        CN14 & CN16 - RCA Stereo Audio OUT


    ROM Usage
    ---------
                 |------------------------------- ROM Locations ----------------------------------|
    Game         27P     25P  22P   16P     14P     12P     9P      16T     14T     12T  9T  7S
    -----------------------------------------------------------------------------------------------
    Gradius 4    837C01  -    -     837A09  837A10  -       778A12  837A04  837A05  -    -   837A08
    NBA P/Play   778A01  -    -     778A09  778A10  778A11  778A12  778A04  778A05  -    -   778A08
    S/Scope      830B01  -    -     830A09  830A10  -       -       -       -       -    -   830A08
    S/Scope 2    931D01  -    -     931A09  931A10  931A11  -       931A04  -       -    -   931A08
    Terraburst


    Bottom Board
    GN715 PWB(B)A
    |--------------------------------------------------------------|
    |CN4        CN2    CN8               CN6                    CN5|
    |JP1                 |---------|              4M_EDO   4M_EDO  |
    |                    |         |     |----------|              |
    |  4M_EDO 4M_EDO     | TEXELFX |     |          |              |
    |                    |         |     | PIXELFX  |       4M_EDO |
    |  4M_EDO 4M_EDO     |         |     |          |       4M_EDO |
    |                    |---------|     |          | |--------|   |
    |  4M_EDO 4M_EDO                     |----------| |KONAMI  |   |
    |CN3                                50MHz JP7     |33906   |   |
    |  4M_EDO 4M_EDO                          JP6     |        |   |
    |                       256KSRAM 256KSRAM         |--------|   |
    |CN7                                                           |
    |         AV9170                     1MSRAM 1MSRAM             |
    | MC44200                                                      |
    |                       256KSRAM 256KSRAM                      |
    |                                    1MSRAM 1MSRAM             |
    |  |-------|                                    MASKROM.24U    |
    |  |KONAMI |  MACH111  |-------------|              MASKROM.24V|
    |  |37122  |           |ANALOG       |   1MSRAM 1MSRAM         |
    |  |       |           |DEVICES      |                         |
    |  |-------|       JP5 |ADSP-21062   |   36.00MHz              |
    |1MSRAM                |SHARC        |   1MSRAM 1MSRAM         |
    |                      |             |                         |
    |1MSRAM                |             |                         |
    |           256KSRAM   |-------------|          MASKROM.32U    |
    |1MSRAM     256KSRAM                                MASKROM.32V|
    |           256KSRAM     PAL1  PAL2            JP4             |
    |1MSRAM                                                        |
    |           JP2                 CN1            JP3             |
    |--------------------------------------------------------------|
    Notes:
          4M_EDO - Silicon Magic SM81C256K16CJ-35 EDO DRAM 66MHz (SOJ40)
          1MSRAM - Cypress CY7C109-25VC 1Meg SRAM (SOJ32)
        256KSRAM - Winbond W24257AJ-15 256K SRAM (SOJ28)
         TEXELFX - 3DFX 500-0004-02 BD0665.1 TMU (QFP208)
         PIXELFX - 3DFX 500-0003-03 F001701.1 FBI (QFP240)
      0000037122 - Konami Custom (QFP208)
       MC44200FT - Motorola MC44200FT 3 Channel Video D/A Converter (QFP44)
         MACH111 - AMD MACH111 CPLD (Stamped 'N715B1', PLCC44)
          AV9170 - Integrated Circuit Systems Inc. Clock Multiplier (SOIC8)
            PAL1 - AMD PALCE16V8 (stamped 'N676B4', DIP20)
            PAL2 - AMD PALCE16V8 (stamped 'N676B5', DIP20)
             JP1 -    SCR O O-O TWN
             JP2 - MASTER O-O O SLAVE
             JP3 -    16M O O-O 32M
             JP4 -    32M O-O O 16M
             JP5 -  ASYNC O O-O SYNC
             JP6 -    DSP O O-O ADCK
             JP7 -    MCK O-O O SCK
             CN1 - 96 Pin To Lower PCB, Joining Connector
             CN2 - 8-Pin RGB OUT
             CN3 - 15-Pin DSUB VGA Video MAIN OUT
             CN4 - 6-Pin Power Connector
             CN5 - 4-Pin Power Connector
             CN6 - 2-Pin Connector (Not Used)
             CN7 - 15-Pin DSUB VGA Video MAIN OUT
             CN8 - 6-Pin Connector (Not Used)

    ROM Usage
    ---------
                 |------ ROM Locations -------|
    Game         24U     24V     32U     32V
    -------------------------------------------
    Gradius 4    837A13  837A15  837A14  837A16
    NBA P/Play   778A13  778A15  778A14  778A16
    S/Scope      -       -       -       -          (no ROMs, not used)
    S/Scope 2    -       -       -       -          (no ROMs, not used)
    Terraburst



    LAN PCB: GQ931 PWB(H)      (C) 1999 Konami
    ------------------------------------------

    2 x LAN ports, LANC(1) & LANC(2)
    1 x 32.0000MHz Oscillator

         HYC2485S  SMC ARCNET Media Transceiver, RS485 5Mbps-2.5Mbps
    8E   931A19    Konami 32meg masked ROM, ROM0 (compressed GFX data?)
    6E   931A20    Konami 32meg masked ROM, ROM1 (compressed GFX data?)
    12F  XC9536    Xilinx  CPLD,  44 pin PLCC, Konami no. Q931H1
    12C  XCS10XL   Xilinx  FPGA, 100 pin PQFP, Konami no. 4C
    12B  CY7C199   Cypress 32kx8 SRAM
    8B   AT93C46   Atmel 1K serial EEPROM, 8 pin SOP
    16G  DS2401    Dallas Silicon Serial Number IC, 6 pin SOP



    GFX PCB:  GQ871 PWB(B)A    (C) 1999 Konami
    ------------------------------------------

    There are no ROMs on the two GFX PCBs, all sockets are empty.
    Prior to the game starting there is a message saying downloading data.


    Jumpers set on GFX PCB to main monitor:
    4A   set to TWN (twin GFX PCBs)
    37D  set to Master


    Jumpers set on GFX PCB to scope monitor:
    4A   set to TWN (twin GFX PCBs)
    37D  set to Slave


    1 x 64.0000MHz
    1 x 36.0000MHz  (to 27L, ADSP)

    21E  AV9170           ICS, Clock synchroniser and multiplier

    27L  ADSP-21062       Analog Devices SHARC ADSP, 512k flash, Konami no. 022M16C
    15T  0000033906       Konami Custom, 160 pin PQFP
    19R  W241024AI-20     Winbond, 1Meg SRAM
    22R  W241024AI-20     Winbond, 1Meg SRAM
    25R  W241024AI-20     Winbond, 1Meg SRAM
    29R  W241024AI-20     Winbond, 1Meg SRAM
    19P  W241024AI-20     Winbond, 1Meg SRAM
    22P  W241024AI-20     Winbond, 1Meg SRAM
    25P  W241024AI-20     Winbond, 1Meg SRAM
    29P  W241024AI-20     Winbond, 1Meg SRAM
    18N  W24257AJ-15      Winbond, 256K SRAM
    14N  W24257AJ-15      Winbond, 256K SRAM
    18M  W24257AJ-15      Winbond, 256K SRAM
    14M  W24257AJ-15      Winbond, 256K SRAM

    28D  000037122        Konami Custom, 208 pin PQFP
    33E  W24257AJ-15      Winbond, 256K SRAM
    33D  W24257AJ-15      Winbond, 256K SRAM
    33C  W24257AJ-15      Winbond, 256K SRAM
    27A  W241024AI-20     Winbond, 1Meg SRAM
    30A  W241024AI-20     Winbond, 1Meg SRAM
    32A  W241024AI-20     Winbond, 1Meg SRAM
    35A  W241024AI-20     Winbond, 1Meg SRAM

    7K   500-0010-01      3DFX, Texture processor
    16F  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    13F  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    9F   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    5F   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    16D  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    13D  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    9D   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    5D   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg

    9P   500-0009-01      3DFX, Pixel processor
    10U  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    7U   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    3S   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    3R   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg

    27G  XC9536           Xilinx, CPLD, Konami no. Q830B1
    21C  MC44200FT        Motorola, 3 Channel video D/A converter
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/sharc/sharc.h"
#include "machine/adc1213x.h"
#include "machine/eepromser.h"
#include "machine/k033906.h"
#include "machine/konppc.h"
#include "machine/timekpr.h"
#include "machine/ds2401.h"
#include "sound/rf5c400.h"
#include "sound/k056800.h"
#include "video/voodoo.h"
#include "video/k037122.h"
#include "rendlay.h"


class hornet_state : public driver_device
{
public:
	hornet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_sharc_dataram0(*this, "sharc_dataram0"),
		m_sharc_dataram1(*this, "sharc_dataram1"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k056800(*this, "k056800"),
		m_gn680(*this, "gn680"),
		m_dsp(*this, "dsp"),
		m_dsp2(*this, "dsp2"),
		m_k037122_1(*this, "k037122_1"),
		m_k037122_2(*this, "k037122_2" ),
		m_adc12138(*this, "adc12138"),
		m_konppc(*this, "konppc"),
		m_lan_eeprom(*this, "lan_eeprom"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_in2(*this, "IN2"),
		m_dsw(*this, "DSW"),
		m_eepromout(*this, "EEPROMOUT"),
		m_analog1(*this, "ANALOG1"),
		m_analog2(*this, "ANALOG2")
	{ }

	// TODO: Needs verification on real hardware
	static const int m_sound_timer_usec = 2800;

	required_shared_ptr<UINT32> m_workram;
	required_shared_ptr<UINT32> m_sharc_dataram0;
	optional_shared_ptr<UINT32> m_sharc_dataram1;
	required_device<ppc4xx_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k056800_device> m_k056800;
	optional_device<cpu_device> m_gn680;
	required_device<cpu_device> m_dsp;
	optional_device<cpu_device> m_dsp2;
	optional_device<k037122_device> m_k037122_1;
	optional_device<k037122_device> m_k037122_2;
	required_device<adc12138_device> m_adc12138;
	required_device<konppc_device> m_konppc;
	optional_device<eeprom_serial_93cxx_device> m_lan_eeprom;
	required_ioport m_in0, m_in1, m_in2, m_dsw;
	optional_ioport m_eepromout, m_analog1, m_analog2;

	emu_timer *m_sound_irq_timer;
	UINT8 m_led_reg0;
	UINT8 m_led_reg1;
	UINT8 *m_jvs_sdata;
	UINT32 m_jvs_sdata_ptr;
	UINT16 m_gn680_latch;
	UINT16 m_gn680_ret0;
	UINT16 m_gn680_ret1;

	DECLARE_READ32_MEMBER(hornet_k037122_sram_r);
	DECLARE_WRITE32_MEMBER(hornet_k037122_sram_w);
	DECLARE_READ32_MEMBER(hornet_k037122_char_r);
	DECLARE_WRITE32_MEMBER(hornet_k037122_char_w);
	DECLARE_READ32_MEMBER(hornet_k037122_reg_r);
	DECLARE_WRITE32_MEMBER(hornet_k037122_reg_w);
	DECLARE_READ8_MEMBER(sysreg_r);
	DECLARE_WRITE8_MEMBER(sysreg_w);
	DECLARE_WRITE32_MEMBER(comm1_w);
	DECLARE_WRITE32_MEMBER(comm_rombank_w);
	DECLARE_READ32_MEMBER(comm0_unk_r);
	DECLARE_READ32_MEMBER(gun_r);
	DECLARE_WRITE32_MEMBER(gun_w);
	DECLARE_WRITE16_MEMBER(gn680_sysctrl);
	DECLARE_READ16_MEMBER(gn680_latch_r);
	DECLARE_WRITE16_MEMBER(gn680_latch_w);
	DECLARE_READ32_MEMBER(dsp_dataram0_r);
	DECLARE_WRITE32_MEMBER(dsp_dataram0_w);
	DECLARE_READ32_MEMBER(dsp_dataram1_r);
	DECLARE_WRITE32_MEMBER(dsp_dataram1_w);
	DECLARE_WRITE_LINE_MEMBER(voodoo_vblank_0);
	DECLARE_WRITE_LINE_MEMBER(voodoo_vblank_1);
	DECLARE_WRITE16_MEMBER(soundtimer_en_w);
	DECLARE_WRITE16_MEMBER(soundtimer_count_w);
	ADC12138_IPT_CONVERT_CB(adc12138_input_callback);
	DECLARE_WRITE8_MEMBER(jamma_jvs_w);

	DECLARE_DRIVER_INIT(hornet);
	DECLARE_DRIVER_INIT(hornet_2board);
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_MACHINE_RESET(hornet_2board);
	UINT32 screen_update_hornet(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_hornet_2board(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(sound_irq);
	int jvs_encode_data(UINT8 *in, int length);
	int jvs_decode_data(UINT8 *in, UINT8 *out, int length);
	void jamma_jvs_cmd_exec();
};




READ32_MEMBER(hornet_state::hornet_k037122_sram_r)
{
	k037122_device *k037122 = m_konppc->get_cgboard_id() ? m_k037122_2 : m_k037122_1;
	return k037122->sram_r(space, offset, mem_mask);
}

WRITE32_MEMBER(hornet_state::hornet_k037122_sram_w)
{
	k037122_device *k037122 = m_konppc->get_cgboard_id() ? m_k037122_2 : m_k037122_1;
	k037122->sram_w(space, offset, data, mem_mask);
}


READ32_MEMBER(hornet_state::hornet_k037122_char_r)
{
	k037122_device *k037122 = m_konppc->get_cgboard_id() ? m_k037122_2 : m_k037122_1;
	return k037122->char_r(space, offset, mem_mask);
}

WRITE32_MEMBER(hornet_state::hornet_k037122_char_w)
{
	k037122_device *k037122 = m_konppc->get_cgboard_id() ? m_k037122_2 : m_k037122_1;
	k037122->char_w(space, offset, data, mem_mask);
}

READ32_MEMBER(hornet_state::hornet_k037122_reg_r)
{
	k037122_device *k037122 = m_konppc->get_cgboard_id() ? m_k037122_2 : m_k037122_1;
	return k037122->reg_r(space, offset, mem_mask);
}

WRITE32_MEMBER(hornet_state::hornet_k037122_reg_w)
{
	k037122_device *k037122 = m_konppc->get_cgboard_id() ? m_k037122_2 : m_k037122_1;
	k037122->reg_w(space, offset, data, mem_mask);
}

WRITE_LINE_MEMBER(hornet_state::voodoo_vblank_0)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}

WRITE_LINE_MEMBER(hornet_state::voodoo_vblank_1)
{
}

UINT32 hornet_state::screen_update_hornet(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	device_t *voodoo = machine().device("voodoo0");

	voodoo_update(voodoo, bitmap, cliprect);

	m_k037122_1->tile_draw(screen, bitmap, cliprect);

	draw_7segment_led(bitmap, 3, 3, m_led_reg0);
	draw_7segment_led(bitmap, 9, 3, m_led_reg1);
	return 0;
}

UINT32 hornet_state::screen_update_hornet_2board(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (strcmp(screen.tag(), ":lscreen") == 0)
	{
		device_t *voodoo = machine().device("voodoo0");
		voodoo_update(voodoo, bitmap, cliprect);

		/* TODO: tilemaps per screen */
		m_k037122_1->tile_draw(screen, bitmap, cliprect);
	}
	else if (strcmp(screen.tag(), ":rscreen") == 0)
	{
		device_t *voodoo = machine().device("voodoo1");
		voodoo_update(voodoo, bitmap, cliprect);

		/* TODO: tilemaps per screen */
		m_k037122_2->tile_draw(screen, bitmap, cliprect);
	}

	draw_7segment_led(bitmap, 3, 3, m_led_reg0);
	draw_7segment_led(bitmap, 9, 3, m_led_reg1);
	return 0;
}

/*****************************************************************************/

READ8_MEMBER(hornet_state::sysreg_r)
{
	UINT8 r = 0;

	switch (offset)
	{
		case 0: /* I/O port 0 */
			r = m_in0->read();
			break;
		case 1: /* I/O port 1 */
			r = m_in1->read();
			break;
		case 2: /* I/O port 2 */
			r = m_in2->read();
			break;

		case 3: /* I/O port 3 */
			/*
			    0x80 = JVSINIT (JAMMA I/F SENSE)
			    0x40 = COMMST
			    0x20 = GSENSE
			    0x08 = EEPDO (EEPROM DO)
			    0x04 = ADEOC (ADC EOC)
			    0x02 = ADDOR (ADC DOR)
			    0x01 = ADDO (ADC DO)
			*/
			r = 0xf0;
			if (m_lan_eeprom)
				r |= m_lan_eeprom->do_read() << 3;
			r |= m_adc12138->do_r(space, 0) | (m_adc12138->eoc_r(space, 0) << 2);
			break;

		case 4: /* I/O port 4 - DIP switches */
			r = m_dsw->read();
			break;
	}
	return r;
}

WRITE8_MEMBER(hornet_state::sysreg_w)
{
	switch (offset)
	{
		case 0: /* LED Register 0 */
			m_led_reg0 = data;
			break;

		case 1: /* LED Register 1 */
			m_led_reg1 = data;
			break;

		case 2: /* Parallel data register */
			osd_printf_debug("Parallel data = %02X\n", data);
			break;

		case 3: /* System Register 0 */
			/*
			    0x80 = EEPWEN (EEPROM write enable)
			    0x40 = EEPCS (EEPROM CS)
			    0x20 = EEPSCL (EEPROM SCL?)
			    0x10 = EEPDT (EEPROM data)
			    0x08 = JVSTXEN / LAMP3 (something about JAMMA interface)
			    0x04 = LAMP2
			    0x02 = LAMP1
			    0x01 = LAMP0
			*/
			if (m_eepromout)
				m_eepromout->write(data, 0xff);
			osd_printf_debug("System register 0 = %02X\n", data);
			break;

		case 4: /* System Register 1 */
			/*
			    0x80 = SNDRES (sound reset)
			    0x40 = COMRES (COM reset)
			    0x20 = COINRQ2 (EEPROM SCL?)
			    0x10 = COINRQ1 (EEPROM data)
			    0x08 = ADCS (ADC CS)
			    0x04 = ADCONV (ADC CONV)
			    0x02 = ADDI (ADC DI)
			    0x01 = ADDSCLK (ADC SCLK)
			*/
			m_adc12138->cs_w(space, 0, (data >> 3) & 0x1);
			m_adc12138->conv_w(space, 0, (data >> 2) & 0x1);
			m_adc12138->di_w(space, 0, (data >> 1) & 0x1);
			m_adc12138->sclk_w(space, 0, data & 0x1);

			m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
			osd_printf_debug("System register 1 = %02X\n", data);
			break;

		case 5: /* Sound Control Register */
			/*
			    0x80 = MODE1
			    0x40 = MUTE1
			    0x20 = DEEN1
			    0x10 = ATCK1
			    0x08 = MODE0
			    0x04 = MUTE0
			    0x02 = DEEN0
			    0x01 = ATCK0
			*/
			osd_printf_debug("Sound control register = %02X\n", data);
			break;

		case 6: /* WDT Register */
			/*
			    0x80 = WDTCLK
			*/
			if (data & 0x80)
				machine().watchdog_reset();
			break;

		case 7: /* CG Control Register */
			/*
			    0x80 = EXRES1
			    0x40 = EXRES0
			    0x20 = EXID1
			    0x10 = EXID0
			    0x01 = EXRGB
			*/
			if (data & 0x80)
				m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
			if (data & 0x40)
				m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
			m_konppc->set_cgboard_id((data >> 4) & 3);
			break;
	}
}

/*****************************************************************************/

WRITE32_MEMBER(hornet_state::comm1_w)
{
	printf("comm1_w: %08X, %08X, %08X\n", offset, data, mem_mask);
}

WRITE32_MEMBER(hornet_state::comm_rombank_w)
{
	int bank = data >> 24;
	UINT8 *usr3 = memregion("user3")->base();
	if (usr3 != NULL)
		membank("bank1")->set_entry(bank & 0x7f);
}

READ32_MEMBER(hornet_state::comm0_unk_r)
{
//  printf("comm0_unk_r: %08X, %08X\n", offset, mem_mask);
	return 0xffffffff;
}


READ32_MEMBER(hornet_state::gun_r)
{
	return m_gn680_ret0<<16 | m_gn680_ret1;
}

WRITE32_MEMBER(hornet_state::gun_w)
{
	if (mem_mask == 0xffff0000)
	{
		m_gn680_latch = data>>16;
		m_gn680->set_input_line(M68K_IRQ_6, HOLD_LINE);
	}
}

/******************************************************************/

TIMER_CALLBACK_MEMBER(hornet_state::sound_irq)
{
	m_audiocpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
}


WRITE16_MEMBER(hornet_state::soundtimer_en_w)
{
	if (data & 1)
	{
		// Reset and disable timer
		m_sound_irq_timer->adjust(attotime::from_usec(m_sound_timer_usec));
		m_sound_irq_timer->enable(false);
	}
	else
	{
		// Enable timer
		m_sound_irq_timer->enable(true);
	}
}

WRITE16_MEMBER(hornet_state::soundtimer_count_w)
{
	// Reset the count
	m_sound_irq_timer->adjust(attotime::from_usec(m_sound_timer_usec));
	m_audiocpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}

/*****************************************************************************/

static ADDRESS_MAP_START( hornet_map, AS_PROGRAM, 32, hornet_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("workram")     /* Work RAM */
	AM_RANGE(0x74000000, 0x740000ff) AM_READWRITE(hornet_k037122_reg_r, hornet_k037122_reg_w)
	AM_RANGE(0x74020000, 0x7403ffff) AM_READWRITE(hornet_k037122_sram_r, hornet_k037122_sram_w)
	AM_RANGE(0x74040000, 0x7407ffff) AM_READWRITE(hornet_k037122_char_r, hornet_k037122_char_w)
	AM_RANGE(0x74080000, 0x7408000f) AM_READWRITE(gun_r, gun_w)
	AM_RANGE(0x78000000, 0x7800ffff) AM_DEVREADWRITE("konppc", konppc_device, cgboard_dsp_shared_r_ppc, cgboard_dsp_shared_w_ppc)
	AM_RANGE(0x780c0000, 0x780c0003) AM_DEVREADWRITE("konppc", konppc_device, cgboard_dsp_comm_r_ppc, cgboard_dsp_comm_w_ppc)
	AM_RANGE(0x7d000000, 0x7d00ffff) AM_READ8(sysreg_r, 0xffffffff)
	AM_RANGE(0x7d010000, 0x7d01ffff) AM_WRITE8(sysreg_w, 0xffffffff)
	AM_RANGE(0x7d020000, 0x7d021fff) AM_DEVREADWRITE8("m48t58", timekeeper_device, read, write, 0xffffffff)  /* M48T58Y RTC/NVRAM */
	AM_RANGE(0x7d030000, 0x7d03000f) AM_DEVREADWRITE8("k056800", k056800_device, host_r, host_w, 0xffffffff)
	AM_RANGE(0x7d042000, 0x7d043fff) AM_RAM             /* COMM BOARD 0 */
	AM_RANGE(0x7d044000, 0x7d044007) AM_READ(comm0_unk_r)
	AM_RANGE(0x7d048000, 0x7d048003) AM_WRITE(comm1_w)
	AM_RANGE(0x7d04a000, 0x7d04a003) AM_WRITE(comm_rombank_w)
	AM_RANGE(0x7d050000, 0x7d05ffff) AM_ROMBANK("bank1")        /* COMM BOARD 1 */
	AM_RANGE(0x7e000000, 0x7e7fffff) AM_ROM AM_REGION("user2", 0)       /* Data ROM */
	AM_RANGE(0x7f000000, 0x7f3fffff) AM_ROM AM_SHARE("share2")
	AM_RANGE(0x7fc00000, 0x7fffffff) AM_ROM AM_REGION("user1", 0) AM_SHARE("share2")    /* Program ROM */
ADDRESS_MAP_END

/*****************************************************************************/

static ADDRESS_MAP_START( sound_memmap, AS_PROGRAM, 16, hornet_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM     /* Work RAM */
	AM_RANGE(0x200000, 0x200fff) AM_DEVREADWRITE("rfsnd", rf5c400_device, rf5c400_r, rf5c400_w)      /* Ricoh RF5C400 */
	AM_RANGE(0x300000, 0x30001f) AM_DEVREADWRITE8("k056800", k056800_device, sound_r, sound_w, 0x00ff)
	AM_RANGE(0x480000, 0x480001) AM_WRITENOP
	AM_RANGE(0x4c0000, 0x4c0001) AM_WRITENOP
	AM_RANGE(0x500000, 0x500001) AM_WRITE(soundtimer_en_w) AM_READNOP
	AM_RANGE(0x600000, 0x600001) AM_WRITE(soundtimer_count_w) AM_READNOP
ADDRESS_MAP_END

/*****************************************************************************/

WRITE16_MEMBER(hornet_state::gn680_sysctrl)
{
	// bit 15 = watchdog toggle
	// lower 4 bits = LEDs?
}

READ16_MEMBER(hornet_state::gn680_latch_r)
{
	m_gn680->set_input_line(M68K_IRQ_6, CLEAR_LINE);

	return m_gn680_latch;
}

WRITE16_MEMBER(hornet_state::gn680_latch_w)
{
	if (offset)
	{
		m_gn680_ret1 = data;
	}
	else
	{
		m_gn680_ret0 = data;
	}
}

// WORD at 30000e: IRQ 4 tests bits 6 and 7, IRQ5 tests bits 4 and 5
// (vsync and hsync status for each of the two screens?)

static ADDRESS_MAP_START( gn680_memmap, AS_PROGRAM, 16, hornet_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x200000, 0x203fff) AM_RAM
	AM_RANGE(0x300000, 0x300001) AM_WRITE(gn680_sysctrl)
	AM_RANGE(0x314000, 0x317fff) AM_RAM
	AM_RANGE(0x400000, 0x400003) AM_READWRITE(gn680_latch_r, gn680_latch_w)
	AM_RANGE(0x400008, 0x400009) AM_WRITENOP    // writes 0001 00fe each time IRQ 6 triggers
ADDRESS_MAP_END

/*****************************************************************************/

READ32_MEMBER(hornet_state::dsp_dataram0_r)
{
	return m_sharc_dataram0[offset] & 0xffff;
}

WRITE32_MEMBER(hornet_state::dsp_dataram0_w)
{
	m_sharc_dataram0[offset] = data;
}

READ32_MEMBER(hornet_state::dsp_dataram1_r)
{
	return m_sharc_dataram1[offset] & 0xffff;
}

WRITE32_MEMBER(hornet_state::dsp_dataram1_w)
{
	m_sharc_dataram1[offset] = data;
}

static ADDRESS_MAP_START( sharc0_map, AS_DATA, 32, hornet_state )
	AM_RANGE(0x0400000, 0x041ffff) AM_DEVREADWRITE("konppc", konppc_device, cgboard_0_shared_sharc_r, cgboard_0_shared_sharc_w)
	AM_RANGE(0x0500000, 0x05fffff) AM_READWRITE(dsp_dataram0_r, dsp_dataram0_w) AM_SHARE("sharc_dataram0")
	AM_RANGE(0x1400000, 0x14fffff) AM_RAM
	AM_RANGE(0x2400000, 0x27fffff) AM_DEVREADWRITE("voodoo0", voodoo_device, voodoo_r, voodoo_w)
	AM_RANGE(0x3400000, 0x34000ff) AM_DEVREADWRITE("konppc", konppc_device, cgboard_0_comm_sharc_r, cgboard_0_comm_sharc_w)
	AM_RANGE(0x3500000, 0x35000ff) AM_DEVREADWRITE("konppc", konppc_device, K033906_0_r, K033906_0_w)
	AM_RANGE(0x3600000, 0x37fffff) AM_ROMBANK("bank5")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sharc1_map, AS_DATA, 32, hornet_state )
	AM_RANGE(0x0400000, 0x041ffff) AM_DEVREADWRITE("konppc", konppc_device, cgboard_1_shared_sharc_r, cgboard_1_shared_sharc_w)
	AM_RANGE(0x0500000, 0x05fffff) AM_READWRITE(dsp_dataram1_r, dsp_dataram1_w) AM_SHARE("sharc_dataram1")
	AM_RANGE(0x1400000, 0x14fffff) AM_RAM
	AM_RANGE(0x2400000, 0x27fffff) AM_DEVREADWRITE("voodoo1", voodoo_device, voodoo_r, voodoo_w)
	AM_RANGE(0x3400000, 0x34000ff) AM_DEVREADWRITE("konppc", konppc_device, cgboard_1_comm_sharc_r, cgboard_1_comm_sharc_w)
	AM_RANGE(0x3500000, 0x35000ff) AM_DEVREADWRITE("konppc", konppc_device, K033906_1_r, K033906_1_w)
	AM_RANGE(0x3600000, 0x37fffff) AM_ROMBANK("bank6")
ADDRESS_MAP_END

/*****************************************************************************/

static INPUT_PORTS_START( hornet )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) // greyed out in gradius4 test mode, but does work
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x80, 0x00, "Test Mode" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Screen Flip (H)" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Screen Flip (V)" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP5" ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Harness" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING( 0x04, "JVS" )
	PORT_DIPSETTING( 0x00, "JAMMA" )
	PORT_DIPNAME( 0x02, 0x02, "DIP7" ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "Monitor Type" ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING( 0x01, "24KHz" )
	PORT_DIPSETTING( 0x00, "15KHz" )
INPUT_PORTS_END

static INPUT_PORTS_START( sscope )
	PORT_INCLUDE( hornet )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gun Trigger")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ANALOG1") // Gun Yaw
	PORT_BIT( 0x7ff, 0x400, IPT_AD_STICK_X ) PORT_MINMAX(0x000, 0x7ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG2") // Gun Pitch
	PORT_BIT( 0x7ff, 0x3ff, IPT_AD_STICK_Y ) PORT_MINMAX(0x000, 0x7ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5) PORT_INVERT
INPUT_PORTS_END

static INPUT_PORTS_START( sscope2 )
	PORT_INCLUDE( sscope )

	// LAN board EEPROM
	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lan_eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lan_eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lan_eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END


/* PowerPC interrupts

    IRQ0:   Vblank CG Board 0
    IRQ1:   Vblank CG Board 1
    IRQ2:   LANC
    DMA0
    NMI:    SCI

*/


void hornet_state::machine_start()
{
	m_jvs_sdata_ptr = 0;
	m_jvs_sdata = auto_alloc_array_clear(machine(), UINT8, 1024);

	/* set conservative DRC options */
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	/* configure fast RAM regions for DRC */
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x003fffff, FALSE, m_workram);

	save_item(NAME(m_led_reg0));
	save_item(NAME(m_led_reg1));
	save_pointer(NAME(m_jvs_sdata), 1024);
	save_item(NAME(m_jvs_sdata_ptr));

	m_sound_irq_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(hornet_state::sound_irq), this));
}

void hornet_state::machine_reset()
{
	UINT8 *usr3 = memregion("user3")->base();
	UINT8 *usr5 = memregion("user5")->base();
	if (usr3 != NULL)
	{
		membank("bank1")->configure_entries(0, memregion("user3")->bytes() / 0x10000, usr3, 0x10000);
		membank("bank1")->set_entry(0);
	}

	m_dsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	if (usr5)
		membank("bank5")->set_base(usr5);
}

ADC12138_IPT_CONVERT_CB(hornet_state::adc12138_input_callback)
{
	int value = 0;
	switch (input)
	{
		case 0: value = (m_analog1) ? m_analog1->read() : 0; break;
		case 1: value = (m_analog2) ? m_analog2->read() : 0; break;
	}

	return (double)(value) / 2047.0;
}

static MACHINE_CONFIG_START( hornet, hornet_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC403GA, XTAL_64MHz/2)   /* PowerPC 403GA 32MHz */
	MCFG_CPU_PROGRAM_MAP(hornet_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(hornet_state, irq1_line_assert,  1000)

	MCFG_CPU_ADD("audiocpu", M68000, XTAL_64MHz/4)    /* 16MHz */
	MCFG_CPU_PROGRAM_MAP(sound_memmap)

	MCFG_CPU_ADD("dsp", ADSP21062, XTAL_36MHz)
	MCFG_SHARC_BOOT_MODE(BOOT_MODE_EPROM)
	MCFG_CPU_DATA_MAP(sharc0_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

//  PCB description at top doesn't mention any EEPROM on the base board...
//  MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	MCFG_DEVICE_ADD("voodoo0", VOODOO_1, STD_VOODOO_1_CLOCK)
	MCFG_VOODOO_FBMEM(2)
	MCFG_VOODOO_TMUMEM(4,0)
	MCFG_VOODOO_SCREEN_TAG("screen")
	MCFG_VOODOO_CPU_TAG("dsp")
	MCFG_VOODOO_VBLANK_CB(WRITELINE(hornet_state,voodoo_vblank_0))

	MCFG_DEVICE_ADD("k033906_1", K033906, 0)
	MCFG_K033906_VOODOO("voodoo0")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(64*8, 48*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 48*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(hornet_state, screen_update_hornet)

	MCFG_PALETTE_ADD("palette", 65536)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)

	MCFG_K037122_ADD("k037122_1", "screen", 0)
	MCFG_K037122_GFXDECODE("gfxdecode")
	MCFG_K037122_PALETTE("palette")

	MCFG_K056800_ADD("k056800", XTAL_16_9344MHz)
	MCFG_K056800_INT_HANDLER(INPUTLINE("audiocpu", M68K_IRQ_2))

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_RF5C400_ADD("rfsnd", XTAL_16_9344MHz)  // value from Guru readme, gives 44100 Hz sample rate
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_M48T58_ADD( "m48t58" )

	MCFG_DEVICE_ADD("adc12138", ADC12138, 0)
	MCFG_ADC1213X_IPT_CONVERT_CB(hornet_state, adc12138_input_callback)

	MCFG_DEVICE_ADD("konppc", KONPPC, 0)
	MCFG_KONPPC_CGBOARD_NUMBER(1)
	MCFG_KONPPC_CGBOARD_TYPE(CGBOARD_TYPE_HORNET)
MACHINE_CONFIG_END


MACHINE_RESET_MEMBER(hornet_state,hornet_2board)
{
	UINT8 *usr3 = memregion("user3")->base();
	UINT8 *usr5 = memregion("user5")->base();

	if (usr3 != NULL)
	{
		membank("bank1")->configure_entries(0, memregion("user3")->bytes() / 0x10000, usr3, 0x10000);
		membank("bank1")->set_entry(0);
	}
	m_dsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_dsp2->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	if (usr5)
	{
		membank("bank5")->set_base(usr5);
		membank("bank6")->set_base(usr5);
	}
}

static MACHINE_CONFIG_DERIVED( hornet_2board, hornet )

	MCFG_CPU_ADD("dsp2", ADSP21062, XTAL_36MHz)
	MCFG_SHARC_BOOT_MODE(BOOT_MODE_EPROM)
	MCFG_CPU_DATA_MAP(sharc1_map)

	MCFG_MACHINE_RESET_OVERRIDE(hornet_state,hornet_2board)


	MCFG_DEVICE_REMOVE("k037122_1")
	MCFG_K037122_ADD("k037122_1", "lscreen", 0)
	MCFG_K037122_GFXDECODE("gfxdecode")
	MCFG_K037122_PALETTE("palette")

	MCFG_K037122_ADD("k037122_2", "rscreen", 1)
	MCFG_K037122_GFXDECODE("gfxdecode")
	MCFG_K037122_PALETTE("palette")

	MCFG_DEVICE_REMOVE("voodoo0")

	MCFG_DEVICE_ADD("voodoo0", VOODOO_1, STD_VOODOO_1_CLOCK)
	MCFG_VOODOO_FBMEM(2)
	MCFG_VOODOO_TMUMEM(4,0)
	MCFG_VOODOO_SCREEN_TAG("lscreen")
	MCFG_VOODOO_CPU_TAG("dsp")
	MCFG_VOODOO_VBLANK_CB(WRITELINE(hornet_state,voodoo_vblank_0))

	MCFG_DEVICE_ADD("voodoo1", VOODOO_1, STD_VOODOO_1_CLOCK)
	MCFG_VOODOO_FBMEM(2)
	MCFG_VOODOO_TMUMEM(4,0)
	MCFG_VOODOO_SCREEN_TAG("rscreen")
	MCFG_VOODOO_CPU_TAG("dsp2")
	MCFG_VOODOO_VBLANK_CB(WRITELINE(hornet_state,voodoo_vblank_1))

	MCFG_DEVICE_ADD("k033906_2", K033906, 0)
	MCFG_K033906_VOODOO("voodoo1")

	/* video hardware */
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(65536)

	MCFG_DEVICE_REMOVE("screen")

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 384)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 383)
	MCFG_SCREEN_UPDATE_DRIVER(hornet_state, screen_update_hornet_2board)

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 384)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 383)
	MCFG_SCREEN_UPDATE_DRIVER(hornet_state, screen_update_hornet_2board)

	MCFG_DEVICE_REMOVE("konppc")
	MCFG_DEVICE_ADD("konppc", KONPPC, 0)
	MCFG_KONPPC_CGBOARD_NUMBER(2)
	MCFG_KONPPC_CGBOARD_TYPE(CGBOARD_TYPE_HORNET)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( terabrst, hornet_2board )

	MCFG_CPU_ADD("gn680", M68000, XTAL_32MHz/2)   /* 16MHz */
	MCFG_CPU_PROGRAM_MAP(gn680_memmap)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hornet_2board_v2, hornet_2board )
	MCFG_DEVICE_REMOVE("voodoo0")
	MCFG_DEVICE_ADD("voodoo0", VOODOO_2, STD_VOODOO_2_CLOCK)
	MCFG_VOODOO_FBMEM(2)
	MCFG_VOODOO_TMUMEM(4,0)
	MCFG_VOODOO_SCREEN_TAG("lscreen")
	MCFG_VOODOO_CPU_TAG("dsp")
	MCFG_VOODOO_VBLANK_CB(WRITELINE(hornet_state,voodoo_vblank_0))

	MCFG_DEVICE_REMOVE("voodoo1")
	MCFG_DEVICE_ADD("voodoo1", VOODOO_2, STD_VOODOO_2_CLOCK)
	MCFG_VOODOO_FBMEM(2)
	MCFG_VOODOO_TMUMEM(4,0)
	MCFG_VOODOO_SCREEN_TAG("rscreen")
	MCFG_VOODOO_CPU_TAG("dsp2")
	MCFG_VOODOO_VBLANK_CB(WRITELINE(hornet_state,voodoo_vblank_1))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sscope2, hornet_2board_v2)

	MCFG_DS2401_ADD("lan_serial_id")
	MCFG_EEPROM_SERIAL_93C46_ADD("lan_eeprom")
MACHINE_CONFIG_END


/*****************************************************************************/

WRITE8_MEMBER(hornet_state::jamma_jvs_w)
{
	if (m_jvs_sdata_ptr == 0 && data != 0xe0)
		return;
	m_jvs_sdata[m_jvs_sdata_ptr] = data;
	m_jvs_sdata_ptr++;

	if (m_jvs_sdata_ptr >= 3 && m_jvs_sdata_ptr >= 3 + m_jvs_sdata[2])
		jamma_jvs_cmd_exec();
}

int hornet_state::jvs_encode_data(UINT8 *in, int length)
{
	int inptr = 0;
	int sum = 0;

	while (inptr < length)
	{
		UINT8 b = in[inptr++];
		if (b == 0xe0)
		{
			sum += 0xd0 + 0xdf;
			m_maincpu->ppc4xx_spu_receive_byte(0xd0);
			m_maincpu->ppc4xx_spu_receive_byte(0xdf);
		}
		else if (b == 0xd0)
		{
			sum += 0xd0 + 0xcf;
			m_maincpu->ppc4xx_spu_receive_byte(0xd0);
			m_maincpu->ppc4xx_spu_receive_byte(0xcf);
		}
		else
		{
			sum += b;
			m_maincpu->ppc4xx_spu_receive_byte(b);
		}
	}
	return sum;
}

int hornet_state::jvs_decode_data(UINT8 *in, UINT8 *out, int length)
{
	int outptr = 0;
	int inptr = 0;

	while (inptr < length)
	{
		UINT8 b = in[inptr++];
		if (b == 0xd0)
		{
			UINT8 b2 = in[inptr++];
			out[outptr++] = b2 + 1;
		}
		else
		{
			out[outptr++] = b;
		}
	};

	return outptr;
}

void hornet_state::jamma_jvs_cmd_exec()
{
	UINT8 byte_num;
	UINT8 data[1024], rdata[1024];
#if 0
	int length;
#endif
	int rdata_ptr;
	int sum;

//  sync = m_jvs_sdata[0];
//  node = m_jvs_sdata[1];
	byte_num = m_jvs_sdata[2];

#if 0
	length =
#endif
		jvs_decode_data(&m_jvs_sdata[3], data, byte_num-1);
#if 0
	printf("jvs input data:\n");
	for (i=0; i < byte_num; i++)
	{
		printf("%02X ", m_jvs_sdata[3+i]);
	}
	printf("\n");

	printf("jvs data decoded to:\n");
	for (i=0; i < length; i++)
	{
		printf("%02X ", data[i]);
	}
	printf("\n\n");
#endif

	// clear return data
	memset(rdata, 0, sizeof(rdata));
	rdata_ptr = 0;

	// status
	rdata[rdata_ptr++] = 0x01;      // normal

	// handle the command
	switch (data[0])
	{
		case 0xf0:      // Reset
		{
			break;
		}
		case 0xf1:      // Address setting
		{
			rdata[rdata_ptr++] = 0x01;      // report data (normal)
			break;
		}
		case 0xfa:
		{
			break;
		}
		default:
		{
			fatalerror("jamma_jvs_cmd_exec: unknown command %02X\n", data[0]);
		}
	}

	// write jvs return data
	sum = 0x00 + (rdata_ptr+1);
	m_maincpu->ppc4xx_spu_receive_byte(0xe0);           // sync
	m_maincpu->ppc4xx_spu_receive_byte(0x00);           // node
	m_maincpu->ppc4xx_spu_receive_byte(rdata_ptr + 1);  // num of bytes
	sum += jvs_encode_data(rdata, rdata_ptr);
	m_maincpu->ppc4xx_spu_receive_byte(sum - 1);        // checksum

	m_jvs_sdata_ptr = 0;
}

/*****************************************************************************/


DRIVER_INIT_MEMBER(hornet_state,hornet)
{
	m_konppc->set_cgboard_texture_bank(0, "bank5", memregion("user5")->base());
	m_led_reg0 = m_led_reg1 = 0x7f;

	m_maincpu->ppc4xx_spu_set_tx_handler(write8_delegate(FUNC(hornet_state::jamma_jvs_w), this));
}

DRIVER_INIT_MEMBER(hornet_state,hornet_2board)
{
	m_konppc->set_cgboard_texture_bank(0, "bank5", memregion("user5")->base());
	m_konppc->set_cgboard_texture_bank(1, "bank6", memregion("user5")->base());
	m_led_reg0 = m_led_reg1 = 0x7f;

	m_maincpu->ppc4xx_spu_set_tx_handler(write8_delegate(FUNC(hornet_state::jamma_jvs_w), this));
}

/*****************************************************************************/

ROM_START(sscope)
	ROM_REGION32_BE(0x400000, "user1", 0)   /* PowerPC program */
	ROM_LOAD16_WORD_SWAP("830d01.27p", 0x200000, 0x200000, CRC(de9b3dfa) SHA1(660652a5f745cb04687481c3626d8a43cd169193) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "user2", ROMREGION_ERASE00)   /* Data roms */

	ROM_REGION(0x80000, "audiocpu", 0)      /* 68K Program */
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "user5", 0)       /* CG Board texture roms */
	ROM_LOAD32_WORD( "830a14.u32", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.u24", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       /* PCM sample roms */
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y.35d",   0x000000, 0x002000, CRC(b077e262) SHA1(5cdcc1b742bf23562f4558216063fea903f045ab) ) // this is set to the JXD, I don't think it's valid.
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(ee815325) SHA1(91b10802791b68a8360c0cd6c376c0c4bbbc6fa0) ) // so just load over it with the US one, we know works.
ROM_END

ROM_START(sscopec)
	ROM_REGION32_BE(0x400000, "user1", 0)   /* PowerPC program */
	ROM_LOAD16_WORD_SWAP("830c01.27p", 0x200000, 0x200000, CRC(87682449) SHA1(6ccaa5bac86e947e01a6aae568a75f002421fe5b) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "user2", ROMREGION_ERASE00)   /* Data roms */

	ROM_REGION(0x80000, "audiocpu", 0)      /* 68K Program */
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "user5", 0)       /* CG Board texture roms */
	ROM_LOAD32_WORD( "830a14.u32", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.u24", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       /* PCM sample roms */
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(ee815325) SHA1(91b10802791b68a8360c0cd6c376c0c4bbbc6fa0) )
ROM_END

ROM_START(sscopeb)
	ROM_REGION32_BE(0x400000, "user1", 0)   /* PowerPC program */
	ROM_LOAD16_WORD_SWAP("830b01.27p", 0x200000, 0x200000, CRC(3b6bb075) SHA1(babc134c3a20c7cdcaa735d5f1fd5cab38667a14) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "user2", ROMREGION_ERASE00)   /* Data roms */

	ROM_REGION(0x80000, "audiocpu", 0)      /* 68K Program */
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "user5", 0)       /* CG Board texture roms */
	ROM_LOAD32_WORD( "830a14.u32", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.u24", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       /* PCM sample roms */
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(ee815325) SHA1(91b10802791b68a8360c0cd6c376c0c4bbbc6fa0) )
ROM_END

ROM_START(sscopea)
	ROM_REGION32_BE(0x400000, "user1", 0)   /* PowerPC program */
	ROM_LOAD16_WORD_SWAP("830a01.27p", 0x200000, 0x200000, CRC(39e353f1) SHA1(569b06969ae7a690f6d6e63cc3b5336061663a37) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "user2", ROMREGION_ERASE00)   /* Data roms */

	ROM_REGION(0x80000, "audiocpu", 0)      /* 68K Program */
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "user5", 0)       /* CG Board texture roms */
	ROM_LOAD32_WORD( "830a14.u32", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.u24", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       /* PCM sample roms */
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(ee815325) SHA1(91b10802791b68a8360c0cd6c376c0c4bbbc6fa0) )
ROM_END

ROM_START(sscope2)
	ROM_REGION32_BE(0x400000, "user1", 0)   /* PowerPC program */
	ROM_LOAD16_WORD_SWAP("931d01.bin", 0x200000, 0x200000, CRC(4065fde6) SHA1(84f2dedc3e8f61651b22c0a21433a64993e1b9e2) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "user2", 0)   /* Data roms */
	ROM_LOAD32_WORD_SWAP("931a04.bin", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "user3", 0)   /* Comm board roms */
	ROM_LOAD("931a19.bin", 0x000000, 0x400000, CRC(8b25a6f1) SHA1(41f9c2046a6aae1e9f5f3ffa3e0ffb15eba46211) )
	ROM_LOAD("931a20.bin", 0x400000, 0x400000, CRC(ecf665f6) SHA1(5a73e87435560a7bb2d0f9be7fba12254b18708d) )

	ROM_REGION(0x800000, "user5", ROMREGION_ERASE00)    /* CG Board texture roms */

	ROM_REGION(0x80000, "audiocpu", 0)      /* 68K Program */
	ROM_LOAD16_WORD_SWAP("931a08.bin", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        /* PCM sample roms */
	ROM_LOAD( "931a09.bin",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.bin",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.bin",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(d4e69d7a) SHA1(1e29eecf4886e5e098a388dedd5f3901c2bb65e5) )

	ROM_REGION(0x8, "lan_serial_id", 0)     /* LAN Board DS2401 */
	ROM_LOAD( "ds2401.8b", 0x000000, 0x000008, NO_DUMP )

	ROM_REGION(0x80, "lan_eeprom", 0)       /* LAN Board AT93C46 */
	ROM_LOAD( "at93c46.16g", 0x000000, 0x000080, NO_DUMP )
ROM_END

ROM_START(gradius4)
	ROM_REGION32_BE(0x400000, "user1", 0)   /* PowerPC program */
	ROM_LOAD16_WORD_SWAP( "837c01.27p",   0x200000, 0x200000, CRC(ce003123) SHA1(15e33997be2c1b3f71998627c540db378680a7a1) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "user2", 0)   /* Data roms */
	ROM_LOAD32_WORD_SWAP( "837a04.16t",   0x000000, 0x200000, CRC(18453b59) SHA1(3c75a54d8c09c0796223b42d30fb3867a911a074) )
	ROM_LOAD32_WORD_SWAP( "837a05.14t",   0x000002, 0x200000, CRC(77178633) SHA1(ececdd501d0692390325c8dad6dbb068808a8b26) )

	ROM_REGION32_BE(0x1000000, "user5", 0)  /* CG Board texture roms */
	ROM_LOAD32_WORD_SWAP( "837a14.32u",   0x000002, 0x400000, CRC(ff1b5d18) SHA1(7a38362170133dcc6ea01eb62981845917b85c36) )
	ROM_LOAD32_WORD_SWAP( "837a13.24u",   0x000000, 0x400000, CRC(d86e10ff) SHA1(6de1179d7081d9a93ab6df47692d3efc190c38ba) )
	ROM_LOAD32_WORD_SWAP( "837a16.32v",   0x800002, 0x400000, CRC(bb7a7558) SHA1(8c8cc062793c2dcfa72657b6ea0813d7223a0b87) )
	ROM_LOAD32_WORD_SWAP( "837a15.24v",   0x800000, 0x400000, CRC(e0620737) SHA1(c14078cdb44f75c7c956b3627045d8494941d6b4) )

	ROM_REGION(0x80000, "audiocpu", 0)      /* 68K Program */
	ROM_LOAD16_WORD_SWAP( "837a08.7s",    0x000000, 0x080000, CRC(c3a7ff56) SHA1(9d8d033277d560b58da151338d14b4758a9235ea) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)        /* PCM sample roms */
	ROM_LOAD( "837a09.16p",   0x000000, 0x400000, CRC(fb8f3dc2) SHA1(69e314ac06308c5a24309abc3d7b05af6c0302a8) )
	ROM_LOAD( "837a10.14p",   0x400000, 0x400000, CRC(1419cad2) SHA1(a6369a5c29813fa51e8246d0c091736f32994f3d) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(935f9d05) SHA1(c3a787dff1b2ac4942858ffa1574405db01292b6) )
ROM_END

ROM_START(nbapbp)
	ROM_REGION32_BE(0x400000, "user1", 0)   /* PowerPC program */
	ROM_LOAD16_WORD_SWAP( "778a01.27p",   0x200000, 0x200000, CRC(e70019ce) SHA1(8b187b6e670fdc88771da08a56685cd621b139dc) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "user2", 0)   /* Data roms */
	ROM_LOAD32_WORD_SWAP( "778a04.16t",   0x000000, 0x400000, CRC(62c70132) SHA1(405aed149fc51e0adfa3ace3c644e47d53cf1ee3) )
	ROM_LOAD32_WORD_SWAP( "778a05.14t",   0x000002, 0x400000, CRC(03249803) SHA1(f632a5f1dfa0a8500407214df0ec8d98ce09bc2b) )

	ROM_REGION32_BE(0x1000000, "user5", 0)  /* CG Board texture roms */
	ROM_LOAD32_WORD_SWAP( "778a14.32u",   0x000002, 0x400000, CRC(db0c278d) SHA1(bb9884b6cdcdb707fff7e56e92e2ede062abcfd3) )
	ROM_LOAD32_WORD_SWAP( "778a13.24u",   0x000000, 0x400000, CRC(47fda9cc) SHA1(4aae01c1f1861b4b12a3f9de6b39eb4d11a9736b) )
	ROM_LOAD32_WORD_SWAP( "778a16.32v",   0x800002, 0x400000, CRC(6c0f46ea) SHA1(c6b9fbe14e13114a91a5925a0b46496260539687) )
	ROM_LOAD32_WORD_SWAP( "778a15.24v",   0x800000, 0x400000, CRC(d176ad0d) SHA1(2be755dfa3f60379d396734809bbaaaad49e0db5) )

	ROM_REGION(0x80000, "audiocpu", 0)      /* 68K Program */
	ROM_LOAD16_WORD_SWAP( "778a08.7s",    0x000000, 0x080000, CRC(6259b4bf) SHA1(d0c38870495c9a07984b4b85e736d6477dd44832) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       /* PCM sample roms */
	ROM_LOAD( "778a09.16p",   0x000000, 0x400000, CRC(e8c6fd93) SHA1(dd378b67b3b7dd932e4b39fbf4321e706522247f) )
	ROM_LOAD( "778a10.14p",   0x400000, 0x400000, CRC(c6a0857b) SHA1(976734ba56460fcc090619fbba043a3d888c4f4e) )
	ROM_LOAD( "778a11.12p",   0x800000, 0x400000, CRC(40199382) SHA1(bee268adf9b6634a4f6bb39278ecd02f2bdcb1f4) )
	ROM_LOAD( "778a12.9p",    0xc00000, 0x400000, CRC(27d0c724) SHA1(48e48cbaea6db0de8c3471a2eda6faaa16eed46e) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(3cff1b1d) SHA1(bed0fc657a785be0c69bb21ad52365635c83d751) )
ROM_END

ROM_START(terabrst)
	ROM_REGION32_BE(0x400000, "user1", 0)   /* PowerPC program */
		ROM_LOAD32_WORD_SWAP( "715l02.25p",   0x000000, 0x200000, CRC(79586f19) SHA1(8dcfed5d101ebe49d958a7a38d5472323f75dd1d) )
	ROM_LOAD32_WORD_SWAP( "715l03.22p",   0x000002, 0x200000, CRC(c193021e) SHA1(c934b7c4bdab0ceff0f1699fcf2fb7d90e2e8962) )

	ROM_REGION32_BE(0x800000, "user2", 0)   /* Data roms */
	ROM_LOAD32_WORD_SWAP( "715a04.16t",   0x000000, 0x200000, CRC(00d9567e) SHA1(fe372399ad0ae89d557c93c3145b38e3ed0f714d) )
	ROM_LOAD32_WORD_SWAP( "715a05.14t",   0x000002, 0x200000, CRC(462d53bf) SHA1(0216a84358571de6791365c69a1fa8fe2784148d) )

	ROM_REGION32_BE(0x1000000, "user5", 0)  /* CG Board texture roms */
	ROM_LOAD32_WORD_SWAP( "715a14.32u",   0x000002, 0x400000, CRC(bbb36be3) SHA1(c828d0af0546db02e87afe68423b9447db7c7e51) )
	ROM_LOAD32_WORD_SWAP( "715a13.24u",   0x000000, 0x400000, CRC(dbff58a1) SHA1(f0c60bb2cbf268cfcbdd65606ebb18f1b4839c0e) )

	ROM_REGION(0x80000, "audiocpu", 0)      /* 68K Program */
	ROM_LOAD16_WORD_SWAP( "715a08.7s",    0x000000, 0x080000, CRC(3aa2f4a5) SHA1(bb43e5f5ef4ac51f228d4d825be66d3c720d51ea) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       /* PCM sample roms */
	ROM_LOAD( "715a09.16p",   0x000000, 0x400000, CRC(65845866) SHA1(d2a63d0deef1901e6fa21b55c5f96e1f781dceda) )
	ROM_LOAD( "715a10.14p",   0x400000, 0x400000, CRC(294fe71b) SHA1(ac5fff5627df1cee4f1e1867377f208b34334899) )

	ROM_REGION(0x20000, "gn680", 0)     /* 68K Program */
	ROM_LOAD16_WORD_SWAP( "715a17.20k",    0x000000, 0x020000, CRC(f0b7ba0c) SHA1(863b260824b0ae2f890ba84d1c9a8f436891b1ff) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "715uel_m48t58y.35d", 0x000000, 0x002000, CRC(57322db4) SHA1(59cb8cd6ab446bf8781e3dddf902a4ff2484068e) )
ROM_END

ROM_START(terabrsta)
	ROM_REGION32_BE(0x400000, "user1", 0)   /* PowerPC program */
	ROM_LOAD32_WORD_SWAP( "715a02.25p",   0x000000, 0x200000, CRC(070c48b3) SHA1(066cefbd34d8f6476083417471114f782bef97fb) )
	ROM_LOAD32_WORD_SWAP( "715a03.22p",   0x000002, 0x200000, CRC(f77d242f) SHA1(7680e4abcccd549b3f6d1d245f64631fab57e80d) )

	ROM_REGION32_BE(0x800000, "user2", 0)   /* Data roms */
	ROM_LOAD32_WORD_SWAP( "715a04.16t",   0x000000, 0x200000, CRC(00d9567e) SHA1(fe372399ad0ae89d557c93c3145b38e3ed0f714d) )
	ROM_LOAD32_WORD_SWAP( "715a05.14t",   0x000002, 0x200000, CRC(462d53bf) SHA1(0216a84358571de6791365c69a1fa8fe2784148d) )

	ROM_REGION32_BE(0x1000000, "user5", 0)  /* CG Board texture roms */
	ROM_LOAD32_WORD_SWAP( "715a14.32u",   0x000002, 0x400000, CRC(bbb36be3) SHA1(c828d0af0546db02e87afe68423b9447db7c7e51) )
	ROM_LOAD32_WORD_SWAP( "715a13.24u",   0x000000, 0x400000, CRC(dbff58a1) SHA1(f0c60bb2cbf268cfcbdd65606ebb18f1b4839c0e) )

	ROM_REGION(0x80000, "audiocpu", 0)      /* 68K Program */
	ROM_LOAD16_WORD_SWAP( "715a08.7s",    0x000000, 0x080000, CRC(3aa2f4a5) SHA1(bb43e5f5ef4ac51f228d4d825be66d3c720d51ea) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       /* PCM sample roms */
	ROM_LOAD( "715a09.16p",   0x000000, 0x400000, CRC(65845866) SHA1(d2a63d0deef1901e6fa21b55c5f96e1f781dceda) )
	ROM_LOAD( "715a10.14p",   0x400000, 0x400000, CRC(294fe71b) SHA1(ac5fff5627df1cee4f1e1867377f208b34334899) )

	ROM_REGION(0x20000, "gn680", 0)     /* 68K Program */
	ROM_LOAD16_WORD_SWAP( "715a17.20k",    0x000000, 0x020000, CRC(f0b7ba0c) SHA1(863b260824b0ae2f890ba84d1c9a8f436891b1ff) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(62fecb78) SHA1(09509be8a947cf2d38e12a6ea755ec0de4aa9bd4) )
ROM_END

/*************************************************************************/

GAME(  1998, gradius4,  0,        hornet,           hornet,  hornet_state, hornet,        ROT0, "Konami", "Gradius 4: Fukkatsu", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, nbapbp,    0,        hornet,           hornet,  hornet_state, hornet,        ROT0, "Konami", "NBA Play By Play", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAMEL( 1998, terabrst,  0,        terabrst,         hornet,  hornet_state, hornet_2board, ROT0, "Konami", "Teraburst (1998/07/17 ver UEL)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1998, terabrsta, terabrst, terabrst,         hornet,  hornet_state, hornet_2board, ROT0, "Konami", "Teraburst (1998/02/25 ver AAA)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )

// The region comes from the Timekeeper NVRAM, without a valid default all sets except 'xxD, Ver 1.33' will init their NVRAM to UAx versions, the xxD set seems to incorrectly init it to JXD, which isn't a valid
// version, and thus can't be booted.  If you copy the NVRAM from another already initialized set, it will boot as UAD.
// to get the actual game to boot you must calibrate the guns etc.
GAMEL( 2000, sscope,    0,        hornet_2board,    sscope,  hornet_state, hornet_2board, ROT0, "Konami", "Silent Scope (ver xxD, Ver 1.33)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 2000, sscopec,   sscope,   hornet_2board,    sscope,  hornet_state, hornet_2board, ROT0, "Konami", "Silent Scope (ver xxC, Ver 1.30)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 2000, sscopeb,   sscope,   hornet_2board,    sscope,  hornet_state, hornet_2board, ROT0, "Konami", "Silent Scope (ver xxB, Ver 1.20)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 2000, sscopea,   sscope,   hornet_2board,    sscope,  hornet_state, hornet_2board, ROT0, "Konami", "Silent Scope (ver xxA, Ver 1.00)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )

GAMEL( 2000, sscope2,   0,        sscope2,          sscope2, hornet_state, hornet_2board, ROT0, "Konami", "Silent Scope 2", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
