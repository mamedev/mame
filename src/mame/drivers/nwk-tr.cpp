// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*  Konami NWK-TR System

    Driver by Ville Linde



    Hardware overview:

    GN676 CPU Board:
    ----------------
        IBM PowerPC 403GA at 32MHz (main CPU)
        Motorola MC68EC000 at 16MHz (sound CPU)
        Konami K056800 (MIRAC), sound system interface
        Ricoh RF5c400 sound chip
        National Semiconductor ADC12138

    GN676 GFX Board:
    ----------------
        Analog Devices ADSP-21062 SHARC DSP at 36MHz
        Konami K001604 (2D tilemaps + 2x ROZ)
        Konami 0000033906 (PCI bridge)
        3DFX 500-0003-03 (Voodoo) FBI with 2MB RAM
        2x 3DFX 500-0004-02 (Voodoo) TMU with 2MB RAM

    GN676 LAN Board:
    ----------------
        Xilinx XC5210 FPGA
        Xilinx XC5204 FPGA


Konami 'NWK-TR' Hardware
Konami, 1998-1999

Known games on this hardware include....

Game                      (C)      Year
---------------------------------------
Racing Jam                Konami   1998
Racing Jam : Chapter 2    Konami   1999
Thrill Drive              Konami   1998

PCB Layouts
-----------

Note, the top board is virtually identical to GN715 used on Hornet.
Some extra RCA connectors have been added (for dual sound output), the LED and
DIPSW are present on the main board (instead of on the filter board) and the
SOIC8 chip (a secured PIC?) is not populated (the solder pads are there though).
There's an extra sound IC AN7395S (it's not populated on Hornet).
The PALs/PLDs are the same on NWK-TR and Hornet.
Both Racing JAM/Chapter 2 and Thrill Drive use two video boards.
The top video board is set to MASTER/TWIN, lower video board is set to SLAVE
They are otherwise identical.


Top Board (CPU PCB)
GN676 PWB(A)B
Konami 1997
|--------------------------------------------------------------|
| SP485CS CN10       CN11  7805  CN9          JP8 JP9 JP10 JP11|
|CN19  7809                                              PAL1  |
|CN21       JP13 PAL2             68EC000          EPROM.7S    |
|   NE5532       PAL3                                      CN12|
|           JP12  JP16    DRM1M4SJ8                        CN13|
|   NE5532    AN7395S                 MASKROM.9P    MASKROM.9T |
|     SM5877 JP15         RF5C400                              |
|CN18                                 MASKROM.12P   MASKROM.12T|
|     SM5877     16.9344MHz                                 CN7|
|CN14            SRAM256K             MASKROM.14P   MASKROM.14T|
|                                                              |
|CN16            SRAM256K             MASKROM.16P   MASKROM.16T|
|  ADC12138                                                    |
|CN15         056800            JP5                            |
|                               JP4                            |
|CN17                  MACH111  JP3                |---------| |
|   TEST_SW                         EPROM.22P      |         | |
|CN1                   DRAM16X16                   |PPC403GA | |
|                                   EPROM.25P      |         | |
|                                                  |         | |
|                      DRAM16X16    EPROM.27P      |---------| |
| 4AK16                                                     JP6|
|                                                              |
|CN3                                                           |
|          PAL4                     CN5               7.3728MHz|
|          058232                                              |
|                                                     50.000MHz|
|CN2  RESET_SW                                     JP1  JP2    |
|M48T58Y-70PC1  CN4          DSW(8) CN6               64.000MHz|
|--------------------------------------------------------------|
Notes:
      DRM1M4SJ8 - Fujitsu 81C4256 256kx4 DRAM (SOJ24)
       SRAM256K - Cypress CY7C199 32kx8 SRAM (SOJ28)
      DRAM16X16 - Fujitsu 8118160A-60 16megx16 DRAM (SOJ42)
      M48T58Y-70PC1 - ST Timekeeper RAM
        RF5C400 - Ricoh RF5C400 PCM 32Ch, 44.1 kHz Stereo, 3D Effect Spatializer, clock input 16.9344MHz
         056800 - Konami Custom (QFP80)
         058232 - Konami Custom Ceramic Package (SIL14)
       ADC12138 - National Semiconductor ADC12138 A/D Converter, 12-bit + Serial I/O With MUX (SOP28)
        MACH111 - AMD MACH111 CPLD (Stamped 'N676A1', PLCC44)
        68EC000 - Motorola MC68EC000, running at 16.0MHz (64/4)
       PPC403GA - IBM PowerPC 403GA CPU, clock input 32.0MHz (64/2) (QFP160)
       SM5877AM - Nippon Precision Circuits 3rd Order 2-Channel D/A Converter (SOIC24)
          4AK16 - Hitachi 4AK16 Silicon N-Channel Power MOS FET Array (SIL10)
       NE5532AN - Philips, Dual Low-Noise High-Speed Audio OP Amp (DIP8)
        SP485CS - Sipex SP485CS Low Power Half Duplex RS485 Transceiver (DIP8)
        AN7395S - Panasonic AM7395S Spatializer Audio Processor IC for 3D surround (SOIC20)
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
           JP14 -       WDT O O
           JP15 -      MONO O-O O SURR
           JP16 -      HIGH O O O MID (N/C LOW)
    CN1 to  CN3 - D-SUB Connectors
            CN4 - Multi-pin Connector for Network PCB
            CN5 - DIN96 connector (pads only, not used)
            CN6 - DIN96 joining connector to lower PCB
            CN7 - Multi-pin connector (pads only, not used)
    CN9 to CN13 - Power Connectors
       CN14 to CN17 - RCA Stereo Audio OUT
           CN18 - RCA Mono Audio OUT
           CN19 - USB Connector


ROM Usage
---------
          |------------------------------- ROM Locations -------------------------------------|
Game         27P     25P  22P   16P     14P     12P     9P      16T     14T     12T     9T  7S
--------------------------------------------------------------------------------------------------
Racing Jam   676NC01 -    -     676A09  676A10  -       -       676A04  676A05  -       -   676A08
Racing Jam 2 888A01  -    -     888A09  888A10  -       -       676A04  676A05  888A06  -   888A08
Thrill Drive 713BE01 -    -     713A09  713A10  -       -       713A04  713A05  -       -   713A08


Network PCB
-----------
GN676-PWB(H)B
MADE IN JAPAN
(C)1998 KONAMI
sticker - GC713AC
|------------------------|
|  CY7C199       N676H1  |
|                      2G|
|CN3                     |
|  HYC2485S              |
|   XC5204        XC5210 |
|CN2                     |
|         CN1            |
|------------------------|
Notes:
      CN1      - Connector joining to CPU board CN4
      CN2/3    - RCA jacks for network cable
      2G       - Small SOIC8 chip with number 0038323 at location 2G. An identical chip is present on
                 *some* Hornet games on the GN715 CPU board at location 30C. It may be a PIC or EEPROM.
                 On Hornet, the chip seems to refresh the data in the Timekeeper RAM when the battery
                 dies and keeps the game working. It's purpose on the network board is unknown but it may
                 'upgrade' the data in the NVRAM to the network version of the game for a twin cabinet set-up.
      HYC2485S - Hybrid ceramic module for RS485
      CY7C199  - 32k x8 SRAM
      XC5204   - Xilinx XC5204 FPGA
      XC5210   - Xilink XC5210 FPGA
      N676H1   - PALCE16V8Q-15 stamped 'N676H1'


Bottom Board (VIDEO PCB)
GN676 PWB(B)B
|-------------------------------------------------------------------------------------------|
|CN4          CN2      CN8               CN6                                             CN5|
|JP1                        |---------|          4M_EDO   4M_EDO                            |
|                           |         |     |----------|                                    |
|  4M_EDO   4M_EDO          | TEXELFX |     |          |       4M_EDO    MASKROM.8X         |
|CN3                        |         |     | PIXELFX  |                        MASKROM.8Y  |
|  4M_EDO   4M_EDO          |         |     |          |                                    |
|                           |---------|     |          |       4M_EDO                       |
|  4M_EDO   4M_EDO                          |----------|                                    |
|                           |---------|    50MHz         |--------|                         |
|  4M_EDO   4M_EDO          |         |                  |KONAMI  |                         |
|                           | TEXELFX |                  |33906   |      MASKROM.16X        |
|                           |         |                  |        |            MASKROM.16Y  |
|                           |         |  PLCC44_SOCKET   |--------| AM7201                  |
| MC44200                   |---------|                                                     |
|                                                                                           |
|                                                                                           |
|                             PAL3       256KSRAM                         36MHz             |
|                                        256KSRAM    AM7201   AM7201    |-------------|     |
|                                        256KSRAM                       |ANALOG       |     |
|         256KSRAM         MACH111       256KSRAM    AM7201   AM7201    |DEVICES      |     |
|         256KSRAM  AV9170                                              |ADSP-21062   |     |
|                                                                       |SHARC        |     |
|         |--------|                                                    |KS-160       |     |
|         |KONAMI  |                                                    |-------------|     |
|         |001604  |                        1MSRAM  1MSRAM  1MSRAM  1MSRAM                  |
|1MSRAM   |        |                                        1MSRAM  1MSRAM  1MSRAM  1MSRAM  |
|         |--------|                                                                        |
|1MSRAM       256KSRAM                                               PAL1                   |
|         256KSRAM 256KSRAM              JP2   CN1                   PAL2                   |
|-------------------------------------------------------------------------------------------|
Notes:
      4M_EDO - Silicon Magic SM81C256K16CJ-35 EDO DRAM 66MHz (SOJ40)
      1MSRAM - Cypress CY7C109-25VC 1Meg SRAM (SOJ32)
    256KSRAM - Winbond W24257AJ-15 256k SRAM (SOJ28)
     TEXELFX - 3DFX 500-0004-02 BD0665.1 TMU (QFP208)
     PIXELFX - 3DFX 500-0003-03 F001701.1 FBI (QFP240)
      001604 - Konami Custom (QFP208)
       MC44200FT - Motorola MC44200FT 3 Channel Video D/A Converter (QFP44)
     MACH111 - AMD MACH111 CPLD (Stamped '03161A', PLCC44)
    PLCC44_SOCKET - empty PLCC44 socket
      AV9170 - Integrated Circuit Systems Inc. Clock Multiplier (SOIC8)
      AM7201 - AMD AM7201 FIFO (PLCC32)
        PAL1 - AMD PALCE16V8 (stamped 'N676B4', DIP20)
        PAL2 - AMD PALCE16V8 (stamped 'N676B5', DIP20)
        PAL3 - AMD PALCE16V8 (stamped 'N676B2', DIP20)
         JP1 - SLV O O-O MST,TWN (sets board to MASTER TWIN or SLAVE)
         JP2 - SLV O O-O MST (sets board to MASTER or SLAVE)
         CN1 - 96 Pin joining connector to upper PCB
         CN2 - 8-Pin 24kHz RGB OUT
         CN3 - 15-Pin DSUB VGA Video MAIN OUT
         CN4 - 6-Pin Power Connector
         CN5 - 4-Pin Power Connector
         CN6 - 2-Pin Connector (Not Used)
         CN7 - 6-Pin Connector


ROM Usage
---------
          |------ ROM Locations -------|
Game         8X      8Y      16X     16Y
-------------------------------------------
Racing Jam   676A13  -       676A14  -
Racing Jam 2 888A13  -       888A14  -
Thrill Drive 713A13  -       713A14  -

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/sharc/sharc.h"
#include "machine/adc1213x.h"
#include "machine/k033906.h"
#include "machine/konppc.h"
#include "machine/timekpr.h"
#include "sound/rf5c400.h"
#include "sound/k056800.h"
#include "video/voodoo.h"
#include "video/k001604.h"


class nwktr_state : public driver_device
{
public:
	nwktr_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_work_ram(*this, "work_ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dsp(*this, "dsp"),
		m_k056800(*this, "k056800"),
		m_k001604(*this, "k001604"),
		m_adc12138(*this, "adc12138"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_in2(*this, "IN2"),
		m_dsw(*this, "DSW"),
		m_analog1(*this, "ANALOG1"),
		m_analog2(*this, "ANALOG2"),
		m_analog3(*this, "ANALOG3"),
		m_analog4(*this, "ANALOG4"),
		m_analog5(*this, "ANALOG5"),
		m_palette(*this, "palette"),
		m_generic_paletteram_32(*this, "paletteram") { }

	// TODO: Needs verification on real hardware
	static const int m_sound_timer_usec = 2400;

	UINT8 m_led_reg0;
	UINT8 m_led_reg1;
	required_shared_ptr<UINT32> m_work_ram;
	required_device<ppc_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_dsp;
	required_device<k056800_device> m_k056800;
	required_device<k001604_device> m_k001604;
	required_device<adc12138_device> m_adc12138;
	required_ioport m_in0, m_in1, m_in2, m_dsw, m_analog1, m_analog2, m_analog3, m_analog4, m_analog5;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT32> m_generic_paletteram_32;
	emu_timer *m_sound_irq_timer;
	int m_fpga_uploaded;
	int m_lanc2_ram_r;
	int m_lanc2_ram_w;
	std::unique_ptr<UINT8[]> m_lanc2_ram;
	std::unique_ptr<UINT32[]> m_sharc_dataram;
	DECLARE_WRITE32_MEMBER(paletteram32_w);
	DECLARE_READ32_MEMBER(sysreg_r);
	DECLARE_WRITE32_MEMBER(sysreg_w);
	DECLARE_READ32_MEMBER(lanc1_r);
	DECLARE_WRITE32_MEMBER(lanc1_w);
	DECLARE_READ32_MEMBER(lanc2_r);
	DECLARE_WRITE32_MEMBER(lanc2_w);
	DECLARE_READ32_MEMBER(dsp_dataram_r);
	DECLARE_WRITE32_MEMBER(dsp_dataram_w);
	DECLARE_WRITE16_MEMBER(soundtimer_en_w);
	DECLARE_WRITE16_MEMBER(soundtimer_count_w);
	DECLARE_WRITE_LINE_MEMBER(voodoo_vblank_0);
	ADC12138_IPT_CONVERT_CB(adc12138_input_callback);

	TIMER_CALLBACK_MEMBER(sound_irq);
	DECLARE_DRIVER_INIT(nwktr);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_nwktr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void lanc2_init();
};



WRITE32_MEMBER(nwktr_state::paletteram32_w)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	data = m_generic_paletteram_32[offset];
	m_palette->set_pen_color(offset, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

WRITE_LINE_MEMBER(nwktr_state::voodoo_vblank_0)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}


UINT32 nwktr_state::screen_update_nwktr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	device_t *voodoo = machine().device("voodoo0");

	bitmap.fill(m_palette->pen(0), cliprect);

	voodoo_update(voodoo, bitmap, cliprect);

	const rectangle &visarea = screen.visible_area();
	const rectangle tilemap_rect(visarea.min_x, visarea.max_x, visarea.min_y+16, visarea.max_y);

	m_k001604->draw_front_layer(screen, bitmap, tilemap_rect);

	draw_7segment_led(bitmap, 3, 3, m_led_reg0);
	draw_7segment_led(bitmap, 9, 3, m_led_reg1);
	return 0;
}

/*****************************************************************************/

READ32_MEMBER(nwktr_state::sysreg_r)
{
	UINT32 r = 0;
	if (offset == 0)
	{
		if (ACCESSING_BITS_24_31)
		{
			r |= m_in0->read() << 24;
		}
		if (ACCESSING_BITS_16_23)
		{
			r |= m_in1->read() << 16;
		}
		if (ACCESSING_BITS_8_15)
		{
			r |= m_in2->read() << 8;
		}
		if (ACCESSING_BITS_0_7)
		{
			r |= m_adc12138->do_r(space, 0) | (m_adc12138->eoc_r(space, 0) << 2);
		}
	}
	else if (offset == 1)
	{
		if (ACCESSING_BITS_24_31)
		{
			r |= m_dsw->read() << 24;
		}
	}
	return r;
}

WRITE32_MEMBER(nwktr_state::sysreg_w)
{
	if( offset == 0 )
	{
		if (ACCESSING_BITS_24_31)
		{
			m_led_reg0 = (data >> 24) & 0xff;
		}
		if (ACCESSING_BITS_16_23)
		{
			m_led_reg1 = (data >> 16) & 0xff;
		}
		return;
	}
	if( offset == 1 )
	{
		if (ACCESSING_BITS_24_31)
		{
			int cs = (data >> 27) & 0x1;
			int conv = (data >> 26) & 0x1;
			int di = (data >> 25) & 0x1;
			int sclk = (data >> 24) & 0x1;

			m_adc12138->cs_w(space, 0, cs);
			m_adc12138->conv_w(space, 0, conv);
			m_adc12138->di_w(space, 0, di);
			m_adc12138->sclk_w(space, 0, sclk);
		}
		if (ACCESSING_BITS_0_7)
		{
			if (data & 0x80)    // CG Board 1 IRQ Ack
				m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
			if (data & 0x40)    // CG Board 0 IRQ Ack
				m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

			//set_cgboard_id((data >> 4) & 3);
		}
		return;
	}
}


void nwktr_state::lanc2_init()
{
	m_fpga_uploaded = 0;
	m_lanc2_ram_r = 0;
	m_lanc2_ram_w = 0;
	m_lanc2_ram = std::make_unique<UINT8[]>(0x8000);
}

READ32_MEMBER(nwktr_state::lanc1_r)
{
	switch (offset)
	{
		case 0x40/4:
		{
			UINT32 r = 0;

			r |= (m_fpga_uploaded) ? (1 << 6) : 0;
			r |= 1 << 5;

			return (r) << 24;
		}

		default:
		{
			//printf("lanc1_r: %08X, %08X at %08X\n", offset, mem_mask, space.device().safe_pc());
			return 0xffffffff;
		}
	}
}

WRITE32_MEMBER(nwktr_state::lanc1_w)
{
	//printf("lanc1_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, space.device().safe_pc());
}

READ32_MEMBER(nwktr_state::lanc2_r)
{
	UINT32 r = 0;

	if (offset == 0)
	{
		if (ACCESSING_BITS_0_7)
		{
			r |= m_lanc2_ram[m_lanc2_ram_r & 0x7fff];
			m_lanc2_ram_r++;
		}
		else
		{
			r |= 0xffffff00;
		}
	}

	if (offset == 4)
	{
		if (ACCESSING_BITS_24_31)
		{
			r |= 0x00000000;
		}
	}

	//printf("lanc2_r: %08X, %08X at %08X\n", offset, mem_mask, space.device().safe_pc());

	return r;
}

WRITE32_MEMBER(nwktr_state::lanc2_w)
{
	if (offset == 0)
	{
		if (ACCESSING_BITS_24_31)
		{
			UINT8 value = data >> 24;

			value = ((value >> 7) & 0x01) |
					((value >> 5) & 0x02) |
					((value >> 3) & 0x04) |
					((value >> 1) & 0x08) |
					((value << 1) & 0x10) |
					((value << 3) & 0x20) |
					((value << 5) & 0x40) |
					((value << 7) & 0x80);

			m_fpga_uploaded = 1;

			//printf("lanc2_fpga_w: %02X at %08X\n", value, space.device().safe_pc());
		}
		else if (ACCESSING_BITS_8_15)
		{
			m_lanc2_ram_r = 0;
			m_lanc2_ram_w = 0;
		}
		else if (ACCESSING_BITS_16_23)
		{
			m_lanc2_ram[2] = (data >> 20) & 0xf;
			m_lanc2_ram[3] = 0;
		}
		else if (ACCESSING_BITS_0_7)
		{
			m_lanc2_ram[m_lanc2_ram_w & 0x7fff] = data & 0xff;
			m_lanc2_ram_w++;
		}
		else
		{
			//printf("lanc2_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, space.device().safe_pc());
		}
	}
	if (offset == 4)
	{
		// TODO: check if these should be transferred via PPC DMA.

		if (core_stricmp(machine().system().name, "thrilld") == 0 ||
			core_stricmp(machine().system().name, "thrilldb") == 0 ||
			core_stricmp(machine().system().name, "thrilldae") == 0)
		{
			m_work_ram[(0x3ffed0/4) + 0] = 0x472a3731;      // G*71
			m_work_ram[(0x3ffed0/4) + 1] = 0x33202020;      // 3
			m_work_ram[(0x3ffed0/4) + 2] = 0x2d2d2a2a;      // --**
			m_work_ram[(0x3ffed0/4) + 3] = 0x2a207878;      // *

			m_work_ram[(0x3fff40/4) + 0] = 0x47433731;      // GC71
			m_work_ram[(0x3fff40/4) + 1] = 0x33000000;      // 3
			m_work_ram[(0x3fff40/4) + 2] = 0x19994a41;      //   JA
			m_work_ram[(0x3fff40/4) + 3] = 0x4100a9b1;      // A
		}
		else if (core_stricmp(machine().system().name, "racingj2") == 0)
		{
			m_work_ram[(0x3ffc80/4) + 0] = 0x47453838;      // GE88
			m_work_ram[(0x3ffc80/4) + 1] = 0x38003030;      // 8 00
			m_work_ram[(0x3ffc80/4) + 2] = 0x39374541;      // 97EA
			m_work_ram[(0x3ffc80/4) + 3] = 0x410058da;      // A
		}
	}

	//printf("lanc2_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, space.device().safe_pc());
}

/*****************************************************************************/

TIMER_CALLBACK_MEMBER(nwktr_state::sound_irq)
{
	m_audiocpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
}


WRITE16_MEMBER(nwktr_state::soundtimer_en_w)
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

WRITE16_MEMBER(nwktr_state::soundtimer_count_w)
{
	// Reset the count
	m_sound_irq_timer->adjust(attotime::from_usec(m_sound_timer_usec));
	m_audiocpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}


void nwktr_state::machine_start()
{
	/* set conservative DRC options */
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	/* configure fast RAM regions for DRC */
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x003fffff, FALSE, m_work_ram);

	m_sound_irq_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(nwktr_state::sound_irq), this));
}

static ADDRESS_MAP_START( nwktr_map, AS_PROGRAM, 32, nwktr_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("work_ram")        /* Work RAM */
	AM_RANGE(0x74000000, 0x740000ff) AM_DEVREADWRITE("k001604", k001604_device, reg_r, reg_w)
	AM_RANGE(0x74010000, 0x74017fff) AM_RAM_WRITE(paletteram32_w) AM_SHARE("paletteram")
	AM_RANGE(0x74020000, 0x7403ffff) AM_DEVREADWRITE("k001604", k001604_device, tile_r, tile_w)
	AM_RANGE(0x74040000, 0x7407ffff) AM_DEVREADWRITE("k001604", k001604_device, char_r, char_w)
	AM_RANGE(0x78000000, 0x7800ffff) AM_DEVREADWRITE("konppc", konppc_device, cgboard_dsp_shared_r_ppc, cgboard_dsp_shared_w_ppc)
	AM_RANGE(0x780c0000, 0x780c0003) AM_DEVREADWRITE("konppc", konppc_device, cgboard_dsp_comm_r_ppc, cgboard_dsp_comm_w_ppc)
	AM_RANGE(0x7d000000, 0x7d00ffff) AM_READ(sysreg_r)
	AM_RANGE(0x7d010000, 0x7d01ffff) AM_WRITE(sysreg_w)
	AM_RANGE(0x7d020000, 0x7d021fff) AM_DEVREADWRITE8("m48t58", timekeeper_device, read, write, 0xffffffff)  /* M48T58Y RTC/NVRAM */
	AM_RANGE(0x7d030000, 0x7d03000f) AM_DEVREADWRITE8("k056800", k056800_device, host_r, host_w, 0xffffffff)
	AM_RANGE(0x7d040000, 0x7d04ffff) AM_READWRITE(lanc1_r, lanc1_w)
	AM_RANGE(0x7d050000, 0x7d05ffff) AM_READWRITE(lanc2_r, lanc2_w)
	AM_RANGE(0x7e000000, 0x7e7fffff) AM_ROM AM_REGION("user2", 0)   /* Data ROM */
	AM_RANGE(0x7f000000, 0x7f1fffff) AM_ROM AM_SHARE("share2")
	AM_RANGE(0x7fe00000, 0x7fffffff) AM_ROM AM_REGION("user1", 0) AM_SHARE("share2")    /* Program ROM */
ADDRESS_MAP_END

/*****************************************************************************/

static ADDRESS_MAP_START( sound_memmap, AS_PROGRAM, 16, nwktr_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x200000, 0x200fff) AM_DEVREADWRITE("rfsnd", rf5c400_device, rf5c400_r, rf5c400_w)      /* Ricoh RF5C400 */
	AM_RANGE(0x300000, 0x30001f) AM_DEVREADWRITE8("k056800", k056800_device, sound_r, sound_w, 0x00ff)
	AM_RANGE(0x500000, 0x500001) AM_WRITE(soundtimer_en_w) AM_READNOP
	AM_RANGE(0x600000, 0x600001) AM_WRITE(soundtimer_count_w) AM_READNOP
ADDRESS_MAP_END

/*****************************************************************************/


READ32_MEMBER(nwktr_state::dsp_dataram_r)
{
	return m_sharc_dataram[offset] & 0xffff;
}

WRITE32_MEMBER(nwktr_state::dsp_dataram_w)
{
	m_sharc_dataram[offset] = data;
}

static ADDRESS_MAP_START( sharc_map, AS_DATA, 32, nwktr_state )
	AM_RANGE(0x0400000, 0x041ffff) AM_DEVREADWRITE("konppc", konppc_device, cgboard_0_shared_sharc_r, cgboard_0_shared_sharc_w)
	AM_RANGE(0x0500000, 0x05fffff) AM_READWRITE(dsp_dataram_r, dsp_dataram_w)
	AM_RANGE(0x1400000, 0x14fffff) AM_RAM
	AM_RANGE(0x2400000, 0x27fffff) AM_DEVREADWRITE("konppc", konppc_device, nwk_voodoo_0_r, nwk_voodoo_0_w)
	AM_RANGE(0x3400000, 0x34000ff) AM_DEVREADWRITE("konppc", konppc_device, cgboard_0_comm_sharc_r, cgboard_0_comm_sharc_w)
	AM_RANGE(0x3500000, 0x35000ff) AM_DEVREADWRITE("konppc", konppc_device, K033906_0_r, K033906_0_w)
	AM_RANGE(0x3600000, 0x37fffff) AM_ROMBANK("bank5")
ADDRESS_MAP_END

/*****************************************************************************/

static INPUT_PORTS_START( nwktr )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_7)
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x80, 0x00, "Test Mode" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Disable Machine Init" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIP3" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP5" ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP6" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP7" ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "DIP8" ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("ANALOG1")       // Steering
	PORT_BIT( 0xfff, 0x800, IPT_PADDLE ) PORT_MINMAX(0x000, 0xfff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG2")       // Acceleration pedal
	PORT_BIT( 0xfff, 0x000, IPT_PEDAL ) PORT_MINMAX(0x000, 0xfff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG3")       // Foot brake pedal
	PORT_BIT( 0xfff, 0x000, IPT_PEDAL2 ) PORT_MINMAX(0x000, 0xfff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG4")       // Hand brake lever
	PORT_BIT( 0xfff, 0x000, IPT_AD_STICK_Y ) PORT_MINMAX(0x000, 0xfff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG5")       // Clutch pedal
	PORT_BIT( 0xfff, 0x000, IPT_PEDAL3 ) PORT_MINMAX(0x000, 0xfff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

INPUT_PORTS_END


ADC12138_IPT_CONVERT_CB(nwktr_state::adc12138_input_callback)
{
	int value = 0;
	switch (input)
	{
		case 0: value = m_analog1->read(); break;
		case 1: value = m_analog2->read(); break;
		case 2: value = m_analog3->read(); break;
		case 3: value = m_analog4->read(); break;
		case 4: value = m_analog5->read(); break;
	}

	return (double)(value) / 4095.0;
}

void nwktr_state::machine_reset()
{
	m_dsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

static MACHINE_CONFIG_START( nwktr, nwktr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC403GA, XTAL_64MHz/2)   /* PowerPC 403GA 32MHz */
	MCFG_CPU_PROGRAM_MAP(nwktr_map)

	MCFG_CPU_ADD("audiocpu", M68000, XTAL_64MHz/4)    /* 16MHz */
	MCFG_CPU_PROGRAM_MAP(sound_memmap)

	MCFG_CPU_ADD("dsp", ADSP21062, XTAL_36MHz)
	MCFG_SHARC_BOOT_MODE(BOOT_MODE_EPROM)
	MCFG_CPU_DATA_MAP(sharc_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(9000))

	MCFG_M48T58_ADD( "m48t58" )

	MCFG_DEVICE_ADD("adc12138", ADC12138, 0)
	MCFG_ADC1213X_IPT_CONVERT_CB(nwktr_state, adc12138_input_callback)

	MCFG_DEVICE_ADD("k033906_1", K033906, 0)
	MCFG_K033906_VOODOO("voodoo0")

	/* video hardware */
	MCFG_DEVICE_ADD("voodoo0", VOODOO_1, STD_VOODOO_1_CLOCK)
	MCFG_VOODOO_FBMEM(2)
	MCFG_VOODOO_TMUMEM(2,2)
	MCFG_VOODOO_SCREEN_TAG("screen")
	MCFG_VOODOO_CPU_TAG("dsp")
	MCFG_VOODOO_VBLANK_CB(WRITELINE(nwktr_state,voodoo_vblank_0))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 384)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 383)
	MCFG_SCREEN_UPDATE_DRIVER(nwktr_state, screen_update_nwktr)

	MCFG_PALETTE_ADD("palette", 65536)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)

	MCFG_DEVICE_ADD("k001604", K001604, 0)
	MCFG_K001604_GFX_INDEX1(0)
	MCFG_K001604_GFX_INDEX2(1)
	MCFG_K001604_LAYER_SIZE(0)
	MCFG_K001604_ROZ_SIZE(1)
	MCFG_K001604_TXT_OFFSET(0)  // correct?
	MCFG_K001604_ROZ_OFFSET(0)  // correct?
	MCFG_K001604_GFXDECODE("gfxdecode")
	MCFG_K001604_PALETTE("palette")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_K056800_ADD("k056800", XTAL_16_9344MHz)
	MCFG_K056800_INT_HANDLER(INPUTLINE("audiocpu", M68K_IRQ_2))

	MCFG_RF5C400_ADD("rfsnd", XTAL_16_9344MHz)  // as per Guru readme above
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_DEVICE_ADD("konppc", KONPPC, 0)
	MCFG_KONPPC_CGBOARD_NUMBER(2)
	MCFG_KONPPC_CGBOARD_TYPE(CGBOARD_TYPE_NWKTR)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( thrilld, nwktr )

	MCFG_DEVICE_REMOVE("k001604")
	MCFG_DEVICE_ADD("k001604", K001604, 0)
	MCFG_K001604_GFX_INDEX1(0)
	MCFG_K001604_GFX_INDEX2(1)
	MCFG_K001604_LAYER_SIZE(1)
	MCFG_K001604_ROZ_SIZE(1)
	MCFG_K001604_TXT_OFFSET(0)  // correct?
	MCFG_K001604_ROZ_OFFSET(0)  // correct?
	MCFG_K001604_GFXDECODE("gfxdecode")
	MCFG_K001604_PALETTE("palette")
MACHINE_CONFIG_END

/*****************************************************************************/

DRIVER_INIT_MEMBER(nwktr_state, nwktr)
{
	machine().device<konppc_device>("konppc")->set_cgboard_texture_bank(0, "bank5", memregion("user5")->base());

	m_sharc_dataram = std::make_unique<UINT32[]>(0x100000/4);
	m_led_reg0 = m_led_reg1 = 0x7f;

	lanc2_init();
}

/*****************************************************************************/

ROM_START(racingj)
	ROM_REGION32_BE(0x200000, "user1", 0)   /* PowerPC program roms */
	ROM_LOAD16_WORD_SWAP("676gnc01.27p", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8) )

	ROM_REGION32_BE(0x800000, "user2", 0)   /* Data roms */
	ROM_LOAD32_WORD_SWAP("676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP("676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )

	ROM_REGION32_BE(0x800000, "user5", 0)   /* CG Board texture roms */
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, "audiocpu", 0)  /* 68k program roms */
	ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)   /* PCM sample roms */
	ROM_LOAD( "676a09.16p", 0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
	ROM_LOAD( "676a10.14p", 0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "676jac_m48t58y.35d", 0x000000, 0x002000, CRC(47e1628c) SHA1(7c42d06ae2f2cd24d083890f333552cbf4f1d3c9) )
ROM_END

ROM_START(racingj2j)
	ROM_REGION32_BE(0x200000, "user1", 0)   /* PowerPC program roms */
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "user2", 0)   /* Data roms */
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "user5", 0)   /* CG Board Texture roms */
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0)  /* 68k program roms */
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)   /* PCM sample roms */
	ROM_LOAD( "888a09.16p",   0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p",   0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "676eae_m48t58y.35d", 0x000000, 0x002000, CRC(f691f5ab) SHA1(e81f652c5caa2caa8bd1c6d6db488d849bda058e) )
ROM_END

ROM_START(racingj2)
	ROM_REGION32_BE(0x200000, "user1", 0)   /* PowerPC program roms */
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "user2", 0) /* Data roms */
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "user5", 0)   /* CG Board Texture roms */
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0)  /* 68k program roms */
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)   /* PCM sample roms */
	ROM_LOAD( "888a09.16p", 0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p", 0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "676jae_m48t58y.35d", 0x000000, 0x002000, CRC(1aa43a1f) SHA1(814b691b8a358bf1545a13d595d17070e612e9a4) )
ROM_END

ROM_START(thrilld)
	ROM_REGION32_BE(0x200000, "user1", 0)   /* PowerPC program roms */
	ROM_LOAD16_WORD_SWAP("713be01.27p", 0x000000, 0x200000, CRC(d84a7723) SHA1(f4e9e08591b7e5e8419266dbe744d56a185384ed) )

	ROM_REGION32_BE(0x800000, "user2", 0)   /* Data roms */
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "user5", 0)   /* CG Board Texture roms */
	ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0)  /* 68k program roms */
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)   /* PCM sample roms */
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "713jae_m48t58y.35d", 0x000000, 0x002000, CRC(5d8fbcb2) SHA1(74ad91544d2a200cf599a565005476623075e7d6) )
ROM_END

ROM_START(thrilldb)
	ROM_REGION32_BE(0x200000, "user1", 0)   /* PowerPC program roms */
	ROM_LOAD16_WORD_SWAP("713bb01.27p", 0x000000, 0x200000, CRC(535fe4e8) SHA1(acd8194a4dafce289dbdfd874f0b799f25aeb73f) )

	ROM_REGION32_BE(0x800000, "user2", 0)   /* Data roms */
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "user5", 0)   /* CG Board Texture roms */
	ROM_LOAD32_WORD_SWAP( "713a13.8x",  0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x", 0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0)  /* 68k program roms */
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)   /* PCM sample roms */
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "713jab_m48t58y.35d", 0x000000, 0x002000, CRC(5d8fbcb2) SHA1(74ad91544d2a200cf599a565005476623075e7d6) )
ROM_END

ROM_START(thrilldae)
	ROM_REGION32_BE(0x200000, "user1", 0)   /* PowerPC program roms */
	ROM_LOAD16_WORD_SWAP("713bb01.27p", 0x000000, 0x200000, CRC(535fe4e8) SHA1(acd8194a4dafce289dbdfd874f0b799f25aeb73f) )

	ROM_REGION32_BE(0x800000, "user2", 0)   /* Data roms */
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "user5", 0)   /* CG Board Texture roms */
	ROM_LOAD32_WORD_SWAP( "713a13.8x",  0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x", 0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0)  /* 68k program roms */
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)   /* PCM sample roms */
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "713eaa_m48t58y.35d", 0x000000, 0x002000, CRC(056ea8fa) SHA1(23574e0c1d011dab8644f3d98763d4a2d11a05b3)  )
ROM_END

/*****************************************************************************/

GAME( 1998, racingj,    0,       nwktr,   nwktr, nwktr_state, nwktr, ROT0, "Konami", "Racing Jam (JAC)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )
GAME( 1999, racingj2,   racingj, nwktr,   nwktr, nwktr_state, nwktr, ROT0, "Konami", "Racing Jam: Chapter 2 (EAE)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )
GAME( 1999, racingj2j,  racingj, nwktr,   nwktr, nwktr_state, nwktr, ROT0, "Konami", "Racing Jam: Chapter 2 (JAE)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )
GAME( 1998, thrilld,    0,       thrilld, nwktr, nwktr_state, nwktr, ROT0, "Konami", "Thrill Drive (JAE)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, thrilldb,   thrilld, thrilld, nwktr, nwktr_state, nwktr, ROT0, "Konami", "Thrill Drive (JAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, thrilldae,  thrilld, thrilld, nwktr, nwktr_state, nwktr, ROT0, "Konami", "Thrill Drive (EAA)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
