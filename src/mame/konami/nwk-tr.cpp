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
Racing Jam : Chapter II   Konami   1998
Thrill Drive              Konami   1998


PCB Layouts
-----------

Note, the top board is virtually identical to GN715 used on Hornet.
Some extra RCA connectors have been added (for dual sound output), the LED and
DIPSW are present on the main board (instead of on the filter board) and the
SOIC8 chip (an XICOR X76F041 Secure SerialFlash) is not populated (the solder pads are there though).
There's an extra sound IC AN7395S (not populated on Hornet).
The PALs/PLDs are the same on both NWK-TR and Hornet.
Both Racing JAM/Chapter II and Thrill Drive use two video boards.
The top video board is set to MASTER/TWIN on both JP1 and JP2, lower video board is set to SLAVE for the same two jumpers
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
      M48T58Y-70PC1 - ST Timekeeper RAM: This determines the cabinet type and region
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
For more info on the network boards and its usage, see konami_gn676_lan.cpp.


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
         JP1 - SLV O O-O MST/TWN (top board); SLV O-O O MST/TWN (bottom board) (sets board to MASTER/TWIN or SLAVE)
         JP2 - SLV O O-O MST (top board); SLV O-O O MST (bottom board) (sets board to MASTER or SLAVE)
         CN1 - 96 Pin joining connector to upper PCB
         CN2 - 8-Pin 24kHz RGB OUT
         CN3 - 15-Pin DSUB VGA Video MAIN OUT
         CN4 - 6-Pin Power Connector
         CN5 - 4-Pin Power Connector
         CN6 - 2-Pin Connector (Not Used)
         CN7 - 6-Pin Connector
         CN8 - 6-Pin Connector 24kHz RGB IN (thrilld's manual depicts the top board's CN2 and bottom board's CN8 connected)


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

#include "k001604.h"
#include "k573mcal.h"
#include "konami_gn676_lan.h"
#include "konppc.h"
#include "konppc_jvshost.h"

#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/sharc/sharc.h"
#include "machine/adc1213x.h"
#include "machine/k033906.h"
#include "machine/timekpr.h"
#include "machine/x76f041.h"
#include "sound/k056800.h"
#include "sound/rf5c400.h"
#include "video/voodoo.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class nwktr_state : public driver_device
{
public:
	nwktr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_work_ram(*this, "work_ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dsp(*this, {"dsp", "dsp2"}), // TODO: hardcoded tags in machine/konpc.cpp
		m_k056800(*this, "k056800"),
		m_k001604(*this, "k001604_%u", 1U),
		m_konppc(*this, "konppc"),
		m_adc12138(*this, "adc12138"),
		m_voodoo(*this, "voodoo%u", 0U),
		m_in(*this, "IN%u", 0U),
		m_dsw(*this, "DSW"),
		m_analog(*this, "ANALOG%u", 1U),
		m_pcb_digit(*this, "pcbdigit%u", 0U),
		m_palette(*this, "palette"),
		m_generic_paletteram_32(*this, "paletteram"),
		m_sharc_dataram(*this, "sharc%u_dataram", 0U),
		m_cg_view(*this, "cg_view"),
		m_jvs_host(*this, "jvs_host"),
		m_gn676_lan(*this, "gn676_lan")
	{ }

	void nwktr(machine_config &config);
	void nwktr_lan_b(machine_config &config);

	void init_nwktr();
	void init_racingj();
	void init_thrilld();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// TODO: Needs verification on real hardware
	static const int m_sound_timer_usec = 2400;
	static constexpr int JVS_BUFFER_SIZE = 1024;

	required_shared_ptr<uint32_t> m_work_ram;
	required_device<ppc4xx_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device_array<adsp21062_device, 2> m_dsp;
	required_device<k056800_device> m_k056800;
	required_device_array<k001604_device, 2> m_k001604;
	required_device<konppc_device> m_konppc;
	required_device<adc12138_device> m_adc12138;
	required_device_array<generic_voodoo_device, 2> m_voodoo;
	required_ioport_array<3> m_in;
	required_ioport m_dsw;
	required_ioport_array<5> m_analog;
	output_finder<2> m_pcb_digit;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint32_t> m_generic_paletteram_32;
	optional_shared_ptr_array<uint32_t, 2> m_sharc_dataram;
	memory_view m_cg_view;
	required_device<konppc_jvs_host_device> m_jvs_host;
	required_device<konami_gn676_lan_device> m_gn676_lan;

	emu_timer *m_sound_irq_timer;
	bool m_exrgb;

	void paletteram32_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t sysreg_r(offs_t offset);
	void sysreg_w(offs_t offset, uint8_t data);
	void soundtimer_en_w(uint16_t data);
	void soundtimer_count_w(uint16_t data);
	double adc12138_input_callback(uint8_t input);
	void jamma_jvs_w(uint8_t data);

	TIMER_CALLBACK_MEMBER(sound_irq);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ppc_map(address_map &map) ATTR_COLD;
	void sharc0_map(address_map &map) ATTR_COLD;
	void sharc1_map(address_map &map) ATTR_COLD;
	void sound_memmap(address_map &map) ATTR_COLD;
};

void nwktr_state::paletteram32_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	data = m_generic_paletteram_32[offset];
	m_palette->set_pen_color(offset, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

uint32_t nwktr_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->pen(0), cliprect);

	int board = m_exrgb ? 1 : 0;

	m_voodoo[board]->update(bitmap, cliprect);
	m_k001604[0]->draw_front_layer(screen, bitmap, cliprect);   // K001604 on slave board doesn't seem to output anything. Bug or intended?

	return 0;
}


/*****************************************************************************/

uint8_t nwktr_state::sysreg_r(offs_t offset)
{
	uint8_t r = 0;

	switch (offset)
	{
		case 0:
			r = m_in[0]->read();
			break;
		case 1:
			r = m_in[1]->read();
			break;
		case 2:
			r = m_in[2]->read();
			break;
		case 3:
			r |= m_jvs_host->sense() << 7;
			r |= m_adc12138->do_r() | (m_adc12138->eoc_r() << 2);
			break;
		case 4:
			r = m_dsw->read();
			break;
		default:
			break;
	}

	return r;
}

void nwktr_state::sysreg_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
		case 1:
			m_pcb_digit[offset] = bitswap<7>(~data , 0, 1, 2, 3, 4, 5, 6);
			break;

		case 3:
			/*
			    The bit used for JVSTXEN changes between 3 and 4 based on the lower 2 bits of IN2.
			    If m_in[2]->read() & 3 != 0, bit 4 is used. Otherwise, bit 3 is used.
			*/
			break;

		case 4:
		{
			m_gn676_lan->reset_fpga_state(BIT(data, 6));

			int cs = BIT(data, 3);
			int conv = BIT(data, 2);
			int di = BIT(data, 1);
			int sclk = BIT(data, 0);

			m_adc12138->cs_w(cs);
			m_adc12138->conv_w(conv);
			m_adc12138->di_w(di);
			m_adc12138->sclk_w(sclk);
			break;
		}

		case 7:
			/*
			    0x80 = EXRES1
			    0x40 = EXRES0
			    0x20 = EXID1
			    0x10 = EXID0
			    0x01 = EXRGB
			*/
			if (data & 0x80) // CG Board 1 IRQ Ack
				m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
			if (data & 0x40) // CG Board 0 IRQ Ack
				m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

			m_konppc->set_cgboard_id((data >> 4) & 0x3);

			// Racing Jam sets CG board ID to 2 when writing to the tilemap chip.
			// This could mean broadcast to both CG boards?

			m_exrgb = data & 0x1;       // Select which CG Board outputs signal

			m_cg_view.select(m_konppc->get_cgboard_id() ? 1 : 0);
			break;

		default:
			break;
	}
}


/*****************************************************************************/

TIMER_CALLBACK_MEMBER(nwktr_state::sound_irq)
{
	m_audiocpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
}


void nwktr_state::soundtimer_en_w(uint16_t data)
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

void nwktr_state::soundtimer_count_w(uint16_t data)
{
	// Reset the count
	m_sound_irq_timer->adjust(attotime::from_usec(m_sound_timer_usec));
	m_audiocpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}


void nwktr_state::machine_start()
{
	m_pcb_digit.resolve();

	// set conservative DRC options
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	// configure fast RAM regions for DRC
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x003fffff, false, m_work_ram);

	m_sound_irq_timer = timer_alloc(FUNC(nwktr_state::sound_irq), this);

	m_exrgb = false;

	save_item(NAME(m_exrgb));
}

void nwktr_state::ppc_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share(m_work_ram);
	map(0x74000000, 0x7407ffff).view(m_cg_view);
	m_cg_view[0](0x74000000, 0x740000ff).rw(m_k001604[0], FUNC(k001604_device::reg_r), FUNC(k001604_device::reg_w));
	m_cg_view[1](0x74000000, 0x740000ff).rw(m_k001604[1], FUNC(k001604_device::reg_r), FUNC(k001604_device::reg_w));
	map(0x74010000, 0x7401ffff).ram().w(FUNC(nwktr_state::paletteram32_w)).share("paletteram");
	m_cg_view[0](0x74020000, 0x7403ffff).rw(m_k001604[0], FUNC(k001604_device::tile_r), FUNC(k001604_device::tile_w));
	m_cg_view[0](0x74040000, 0x7407ffff).rw(m_k001604[0], FUNC(k001604_device::char_r), FUNC(k001604_device::char_w));
	m_cg_view[1](0x74020000, 0x7403ffff).rw(m_k001604[1], FUNC(k001604_device::tile_r), FUNC(k001604_device::tile_w));
	m_cg_view[1](0x74040000, 0x7407ffff).rw(m_k001604[1], FUNC(k001604_device::char_r), FUNC(k001604_device::char_w));
	map(0x78000000, 0x7800ffff).rw(m_konppc, FUNC(konppc_device::cgboard_dsp_shared_r_ppc), FUNC(konppc_device::cgboard_dsp_shared_w_ppc));
	map(0x780c0000, 0x780c0003).rw(m_konppc, FUNC(konppc_device::cgboard_dsp_comm_r_ppc), FUNC(konppc_device::cgboard_dsp_comm_w_ppc));
	map(0x7d000000, 0x7d00ffff).r(FUNC(nwktr_state::sysreg_r));
	map(0x7d010000, 0x7d01ffff).w(FUNC(nwktr_state::sysreg_w));
	map(0x7d020000, 0x7d021fff).rw("m48t58", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)); // M48T58Y RTC/NVRAM
	map(0x7d030000, 0x7d03000f).rw(m_k056800, FUNC(k056800_device::host_r), FUNC(k056800_device::host_w));
	map(0x7d040000, 0x7d04ffff).rw(m_gn676_lan, FUNC(konami_gn676_lan_device::lanc1_r), FUNC(konami_gn676_lan_device::lanc1_w));
	map(0x7d050000, 0x7d05ffff).rw(m_gn676_lan, FUNC(konami_gn676_lan_device::lanc2_r), FUNC(konami_gn676_lan_device::lanc2_w));
	map(0x7e000000, 0x7e7fffff).rom().region("datarom", 0);
	map(0x7f000000, 0x7f1fffff).rom().region("prgrom", 0);
	map(0x7fe00000, 0x7fffffff).rom().region("prgrom", 0);
}

/*****************************************************************************/

void nwktr_state::sound_memmap(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x200fff).rw("rfsnd", FUNC(rf5c400_device::rf5c400_r), FUNC(rf5c400_device::rf5c400_w)); // Ricoh RF5C400
	map(0x300000, 0x30001f).rw(m_k056800, FUNC(k056800_device::sound_r), FUNC(k056800_device::sound_w)).umask16(0x00ff);
	map(0x500000, 0x500001).w(FUNC(nwktr_state::soundtimer_en_w)).nopr();
	map(0x600000, 0x600001).w(FUNC(nwktr_state::soundtimer_count_w)).nopr();
}

/*****************************************************************************/

void nwktr_state::sharc0_map(address_map &map)
{
	map(0x0400000, 0x041ffff).rw(m_konppc, FUNC(konppc_device::cgboard_0_shared_sharc_r), FUNC(konppc_device::cgboard_0_shared_sharc_w));
	map(0x0500000, 0x05fffff).ram().share(m_sharc_dataram[0]).lr32(NAME([this](offs_t offset) { return m_sharc_dataram[0][offset] & 0xffff; }));
	map(0x1400000, 0x14fffff).ram();
	map(0x2400000, 0x27fffff).rw(m_konppc, FUNC(konppc_device::nwk_voodoo_0_r), FUNC(konppc_device::nwk_voodoo_0_w));
	map(0x3400000, 0x34000ff).rw(m_konppc, FUNC(konppc_device::cgboard_0_comm_sharc_r), FUNC(konppc_device::cgboard_0_comm_sharc_w));
	map(0x3500000, 0x35000ff).rw(m_konppc, FUNC(konppc_device::K033906_0_r), FUNC(konppc_device::K033906_0_w));
	map(0x3600000, 0x37fffff).bankr("master_cgboard_bank");
}

void nwktr_state::sharc1_map(address_map &map)
{
	map(0x0400000, 0x041ffff).rw(m_konppc, FUNC(konppc_device::cgboard_1_shared_sharc_r), FUNC(konppc_device::cgboard_1_shared_sharc_w));
	map(0x0500000, 0x05fffff).ram().share(m_sharc_dataram[1]).lr32(NAME([this](offs_t offset) { return m_sharc_dataram[1][offset] & 0xffff; }));
	map(0x1400000, 0x14fffff).ram();
	map(0x2400000, 0x27fffff).rw(m_konppc, FUNC(konppc_device::nwk_voodoo_1_r), FUNC(konppc_device::nwk_voodoo_1_w));
	map(0x3400000, 0x34000ff).rw(m_konppc, FUNC(konppc_device::cgboard_1_comm_sharc_r), FUNC(konppc_device::cgboard_1_comm_sharc_w));
	map(0x3500000, 0x35000ff).rw(m_konppc, FUNC(konppc_device::K033906_1_r), FUNC(konppc_device::K033906_1_w));
	map(0x3600000, 0x37fffff).bankr("slave_cgboard_bank");
}

/*****************************************************************************/

static INPUT_PORTS_START( nwktr_gq )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Gear Shift Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Gear Shift Down")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_7)
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x80, 0x00, "Skip Post" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Disable Machine Init" ) PORT_DIPLOCATION("SW:2") // Enabling this disables the wheel feedback
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

	PORT_START("ANALOG1")
	PORT_BIT( 0xfff, 0x800, IPT_PADDLE ) PORT_NAME("Steering Wheel") PORT_MINMAX(0x000, 0xfff) PORT_SENSITIVITY(100) PORT_KEYDELTA(60)

	PORT_START("ANALOG2")
	PORT_BIT( 0xfff, 0x000, IPT_PEDAL ) PORT_NAME("Gas Pedal") PORT_MINMAX(0x000, 0xfff) PORT_SENSITIVITY(100) PORT_KEYDELTA(60)

	PORT_START("ANALOG3")
	PORT_BIT( 0xfff, 0x000, IPT_PEDAL2 ) PORT_NAME("Brake Pedal") PORT_MINMAX(0x000, 0xfff) PORT_SENSITIVITY(100) PORT_KEYDELTA(60)

	PORT_START("ANALOG4")
	PORT_BIT( 0xfff, 0x000, IPT_UNKNOWN )

	PORT_START("ANALOG5")
	PORT_BIT( 0xfff, 0x000, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( nwktr_gm )
	PORT_INCLUDE(nwktr_gq)

	PORT_MODIFY("ANALOG4")
	PORT_BIT( 0xfff, 0x000, IPT_PEDAL3 ) PORT_NAME("Handbrake Lever") PORT_MINMAX(0x000, 0xfff) PORT_SENSITIVITY(100) PORT_KEYDELTA(60)
INPUT_PORTS_END

static INPUT_PORTS_START( nwktr_gn )
	PORT_INCLUDE(nwktr_gm)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Gear Shift Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Gear Shift Right")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("ANALOG5")
	PORT_BIT( 0xfff, 0x000, IPT_PEDAL ) PORT_NAME("Clutch Pedal") PORT_MINMAX(0x000, 0xfff) PORT_SENSITIVITY(100) PORT_KEYDELTA(60) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( thrillde )
	PORT_INCLUDE(nwktr_gn)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Gear Shift Up/1st")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Gear Shift Down/2nd")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Gear Shift Left/3rd")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Gear Shift Right/4th")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

double nwktr_state::adc12138_input_callback(uint8_t input)
{
	int value = 0;
	switch (input)
	{
		case 0: value = m_analog[0]->read(); break;
		case 1: value = m_analog[1]->read(); break;
		case 2: value = m_analog[2]->read(); break;
		case 3: value = m_analog[3]->read(); break;
		case 4: value = m_analog[4]->read(); break;
	}

	return (double)(value) / 4095.0;
}

void nwktr_state::machine_reset()
{
	m_dsp[0]->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_dsp[1]->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void nwktr_state::nwktr(machine_config &config)
{
	// basic machine hardware
	PPC403GA(config, m_maincpu, XTAL(64'000'000)/2); // PowerPC 403GA 32MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &nwktr_state::ppc_map);

	M68000(config, m_audiocpu, XTAL(64'000'000)/4); // 16MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &nwktr_state::sound_memmap);

	ADSP21062(config, m_dsp[0], XTAL(36'000'000));
	m_dsp[0]->set_boot_mode(adsp21062_device::BOOT_MODE_EPROM);
	m_dsp[0]->set_addrmap(AS_DATA, &nwktr_state::sharc0_map);

	ADSP21062(config, m_dsp[1], XTAL(36'000'000));
	m_dsp[1]->set_boot_mode(adsp21062_device::BOOT_MODE_EPROM);
	m_dsp[1]->set_addrmap(AS_DATA, &nwktr_state::sharc1_map);

	config.set_maximum_quantum(attotime::from_hz(9000));

	M48T58(config, "m48t58", 0);

	ADC12138(config, m_adc12138, 0);
	m_adc12138->set_ipt_convert_callback(FUNC(nwktr_state::adc12138_input_callback));

	K033906(config, "k033906_1", 0, m_voodoo[0]);
	K033906(config, "k033906_2", 0, m_voodoo[1]);

	// video hardware
	VOODOO_1(config, m_voodoo[0], XTAL(50'000'000));
	m_voodoo[0]->set_fbmem(2);
	m_voodoo[0]->set_tmumem(2,2);
	m_voodoo[0]->set_status_cycles(1000); // optimization to consume extra cycles when polling status
	m_voodoo[0]->set_screen("screen");
	m_voodoo[0]->set_cpu(m_dsp[0]);
	m_voodoo[0]->vblank_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_voodoo[0]->stall_callback().set(m_dsp[0], FUNC(adsp21062_device::write_stall));

	VOODOO_1(config, m_voodoo[1], XTAL(50'000'000));
	m_voodoo[1]->set_fbmem(2);
	m_voodoo[1]->set_tmumem(2,2);
	m_voodoo[1]->set_status_cycles(1000); // optimization to consume extra cycles when polling status
	m_voodoo[1]->set_screen("screen");
	m_voodoo[1]->set_cpu(m_dsp[1]);
	m_voodoo[1]->vblank_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	m_voodoo[1]->stall_callback().set(m_dsp[1], FUNC(adsp21062_device::write_stall));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// default 24KHz parameter in both 001604 and voodoo, input clock correct? (58~Hz Vsync, 50MHz/3 or 64MHz/4?)
	screen.set_raw(XTAL(64'000'000) / 4, 644, 44, 44 + 512, 450, 31, 31 + 400);
	screen.set_screen_update(FUNC(nwktr_state::screen_update));

	PALETTE(config, m_palette).set_entries(65536);

	K001604(config, m_k001604[0], 0);
	m_k001604[0]->set_palette(m_palette);

	K001604(config, m_k001604[1], 0);
	m_k001604[1]->set_palette(m_palette);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	K056800(config, m_k056800, XTAL(16'934'400));
	m_k056800->int_callback().set_inputline(m_audiocpu, M68K_IRQ_2);

	rf5c400_device &rfsnd(RF5C400(config, "rfsnd", XTAL(16'934'400)));  // as per Guru readme above
	rfsnd.add_route(0, "lspeaker", 1.0);
	rfsnd.add_route(1, "rspeaker", 1.0);

	KONPPC(config, m_konppc, 0);
	m_konppc->set_num_boards(2);
	m_konppc->set_cbboard_type(konppc_device::CGBOARD_TYPE_NWKTR);

	KONAMI_GN676A_LAN(config, m_gn676_lan, 0);

	KONPPC_JVS_HOST(config, m_jvs_host, 0);
	m_jvs_host->output_callback().set([this](uint8_t c) { m_maincpu->ppc4xx_spu_receive_byte(c); });
}

void nwktr_state::nwktr_lan_b(machine_config &config)
{
	nwktr(config);

	KONAMI_GN676B_LAN(config.replace(), m_gn676_lan, 0);
}

/*****************************************************************************/

void nwktr_state::jamma_jvs_w(uint8_t data)
{
	bool accepted = m_jvs_host->write(data);
	if (accepted)
		m_jvs_host->read();
}

/*****************************************************************************/

void nwktr_state::init_nwktr()
{
	m_maincpu->ppc4xx_spu_set_tx_handler(write8smo_delegate(*this, FUNC(nwktr_state::jamma_jvs_w)));
}

void nwktr_state::init_racingj()
{
	init_nwktr();
	m_konppc->set_cgboard_texture_bank(0, "master_cgboard_bank", memregion("master_cgboard")->base());
	m_konppc->set_cgboard_texture_bank(0, "slave_cgboard_bank", memregion("slave_cgboard")->base()); // for some reason, additional CG roms are located on the slave CG board...
}

void nwktr_state::init_thrilld()
{
	init_nwktr();
	m_konppc->set_cgboard_texture_bank(0, "master_cgboard_bank", memregion("master_cgboard")->base());
	m_konppc->set_cgboard_texture_bank(0, "slave_cgboard_bank", memregion("master_cgboard")->base()); // ...while this is not the case for thrilld
}

/*****************************************************************************/

ROM_START(racingj)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("676gnc01.27p", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP("676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "676a09.16p", 0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
	ROM_LOAD( "676a10.14p", 0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gq676ua_m48t58y.35d", 0x000000, 0x002000, CRC(cd182438) SHA1(101a4c4b7a9b4a4bb79ec793275c90b050780f6c) )
ROM_END

ROM_START(racingje)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("676gnc01.27p", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP("676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "676a09.16p", 0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
	ROM_LOAD( "676a10.14p", 0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gq676ea_m48t58y.35d", 0x000000, 0x002000, CRC(c490ffc1) SHA1(170d736b7a07300f4e560cd384c8fd41f2aaaeff) )
ROM_END

ROM_START(racingjj)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("676gnc01.27p", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP("676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "676a09.16p", 0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
	ROM_LOAD( "676a10.14p", 0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gq676ja_m48t58y.35d", 0x000000, 0x002000, CRC(0b83d595) SHA1(852600a82f34b3cde378c166368c1fb07004203d) )
ROM_END

ROM_START(racingja)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("676gnc01.27p", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP("676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "676a09.16p", 0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
	ROM_LOAD( "676a10.14p", 0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gq676aa_m48t58y.35d", 0x000000, 0x002000, CRC(0eb8209d) SHA1(eb5bc411378423f05f2708d673d5b06687c59dbf) )
ROM_END

ROM_START(racingjm)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("676gnc01.27p", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP("676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "676a09.16p", 0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
	ROM_LOAD( "676a10.14p", 0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676ua_m48t58y.35d", 0x000000, 0x002000, CRC(a91dac63) SHA1(60e8508d783afba3c4fec4f25e80832c2af370b4) )
ROM_END

ROM_START(racingjme)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("676gnc01.27p", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP("676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "676a09.16p", 0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
	ROM_LOAD( "676a10.14p", 0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676ea_m48t58y.35d", 0x000000, 0x002000, CRC(06e27c2e) SHA1(c7950bed19b84b7bc4a1f6a4eabbaae965938c3a) )
ROM_END

ROM_START(racingjmj)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("676gnc01.27p", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP("676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "676a09.16p", 0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
	ROM_LOAD( "676a10.14p", 0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676ja_m48t58y.35d", 0x000000, 0x002000, CRC(6d810934) SHA1(f017970cea9f898460877426805db3df9a614995) )
ROM_END

ROM_START(racingjma)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("676gnc01.27p", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP("676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "676a09.16p", 0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
	ROM_LOAD( "676a10.14p", 0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676aa_m48t58y.35d", 0x000000, 0x002000, CRC(0b6ac906) SHA1(6728351fa26c15501c3125d2108d7904627bb430) )
ROM_END

ROM_START(racingjn)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("676gnc01.27p", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP("676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "676a09.16p", 0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
	ROM_LOAD( "676a10.14p", 0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gn676ua_m48t58y.35d", 0x000000, 0x002000, CRC(9e2f5814) SHA1(381e5f0c9e85e00af3d0bad08fa43358f9952de5) )
ROM_END

ROM_START(racingjne)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("676gnc01.27p", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP("676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "676a09.16p", 0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
	ROM_LOAD( "676a10.14p", 0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gn676ea_m48t58y.35d", 0x000000, 0x002000, CRC(f477b6d7) SHA1(8342fca954dbbf24ee7ddcd9c9e02ba63cbd2e8d) )
ROM_END

ROM_START(racingjnj)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("676gnc01.27p", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP("676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "676a09.16p", 0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
	ROM_LOAD( "676a10.14p", 0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gn676ja_m48t58y.35d", 0x000000, 0x002000, CRC(3b649c83) SHA1(513ce58a848edb6e4f673542a71aed4afa85976b) )
ROM_END

ROM_START(racingjna)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("676gnc01.27p", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP("676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "676a13.8x",  0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
	ROM_LOAD32_WORD_SWAP( "676a14.16x", 0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "676a09.16p", 0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
	ROM_LOAD( "676a10.14p", 0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gn676aa_m48t58y.35d", 0x000000, 0x002000, CRC(fbf85705) SHA1(935a28174884a42a5d3e2f4530a0366018f7e60d) )
ROM_END

ROM_START(racingj2)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "888a09.16p",   0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p",   0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gq888ua_m48t58y.35d", 0x000000, 0x002000, CRC(1903f6c1) SHA1(f8b6dedf585c014044c530b73014915874d6fb71) )
ROM_END

ROM_START(racingj2e)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "888a09.16p", 0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p", 0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gq888ea_m48t58y.35d", 0x000000, 0x002000, CRC(108b2d38) SHA1(7c55f592a0fc2b6809ec5d128e78283b77694345) )
ROM_END

ROM_START(racingj2j)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "888a09.16p", 0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p", 0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gq888ja_m48t58y.35d", 0x000000, 0x002000, CRC(0fd00769) SHA1(7d7ea94066bca2e589c0fe0f69c620a8f97916cc) )
ROM_END

ROM_START(racingj2a)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "888a09.16p",   0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p",   0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gq888aa_m48t58y.35d", 0x000000, 0x002000, CRC(85e8a67d) SHA1(fd799918437ed5d80247c58dfec7006781af657f) )
ROM_END

ROM_START(racingj2m)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "888a09.16p",   0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p",   0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm888ua_m48t58y.35d", 0x000000, 0x002000, CRC(3f611190) SHA1(b6095b15526049c280e97c4016fcd80608278723) )
ROM_END

ROM_START(racingj2me)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "888a09.16p", 0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p", 0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm888ea_m48t58y.35d", 0x000000, 0x002000, CRC(ac1ef970) SHA1(58cb6373978b6c51bba95e235e681279c38d166a) )
ROM_END

ROM_START(racingj2mj)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "888a09.16p", 0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p", 0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm888ja_m48t58y.35d", 0x000000, 0x002000, CRC(04d34fea) SHA1(bce580ee5db6db923ed2257b4569cb60b59fc8b7) )
ROM_END

ROM_START(racingj2ma)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "888a09.16p",   0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p",   0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm888aa_m48t58y.35d", 0x000000, 0x002000, CRC(dcd3c159) SHA1(999d1f0941d7e5ac23a789878a8abfabc1cbec0e) )
ROM_END

ROM_START(racingj2n)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "888a09.16p",   0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p",   0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gn888ua_m48t58y.35d", 0x000000, 0x002000, CRC(d6810390) SHA1(7ea3c2c8a5cc962891454a3c3e10d39ee30c96b9) )
ROM_END

ROM_START(racingj2ne)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "888a09.16p", 0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p", 0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gn888ea_m48t58y.35d", 0x000000, 0x002000, CRC(9f902919) SHA1(ee9fa72583adb31df8ae914273a3c1434008218e) )
ROM_END

ROM_START(racingj2nj)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "888a09.16p", 0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p", 0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gn888ja_m48t58y.35d", 0x000000, 0x002000, CRC(548daab9) SHA1(41601a5f96fe0d6e3cfe28584e5fbbcd253f4353) )
ROM_END

ROM_START(racingj2na)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP( "676a04.16t", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f) )
	ROM_LOAD32_WORD_SWAP( "676a05.14t", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5) )
	ROM_LOAD32_WORD_SWAP( "888a06.12t", 0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // Master CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION32_BE(0x800000, "slave_cgboard", 0) // Slave CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "888a13.8x",  0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
	ROM_LOAD32_WORD_SWAP( "888a14.16x", 0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "888a08.7s", 0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "888a09.16p",   0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
	ROM_LOAD( "888a10.14p",   0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gn888aa_m48t58y.35d", 0x000000, 0x002000, CRC(8c8d240a) SHA1(99fbcaccdc0afc5e4882887c7a4e26c82c824ff9) )
ROM_END

ROM_START(thrilld)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713be01.27p", 0x000000, 0x200000, CRC(d84a7723) SHA1(f4e9e08591b7e5e8419266dbe744d56a185384ed) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION( 0x000224, "gn676_lan:eeprom", 0)
	ROM_LOAD( "gc713ud.2g",   0x000000, 0x000224, BAD_DUMP CRC(fc9563f5) SHA1(1ed08482024f6d4353d4e4ea6c8092f6625d699b) ) // hand built

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676ua_m48t58y.35d", 0x000000, 0x002000, BAD_DUMP CRC(2aeeda76) SHA1(ed63b5ebbd9ebb90afd7fdebc9ad4b4d9966012b) ) // hand built
ROM_END

ROM_START(thrilldj)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713be01.27p", 0x000000, 0x200000, CRC(d84a7723) SHA1(f4e9e08591b7e5e8419266dbe744d56a185384ed) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION( 0x000224, "gn676_lan:eeprom", 0)
	ROM_LOAD( "gc713jc.2g",   0x000000, 0x000224, BAD_DUMP CRC(98e326b7) SHA1(af532cf84eca02a3ed0bd0d49c2b142c7f21760c) ) // hand built

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676ja_m48t58y.35d", 0x000000, 0x002000, BAD_DUMP CRC(e6eafce5) SHA1(167d957f7cf1e94a12070739a4103977512f6737) ) // hand built
ROM_END

ROM_START(thrilldja)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713be01.27p", 0x000000, 0x200000, CRC(d84a7723) SHA1(f4e9e08591b7e5e8419266dbe744d56a185384ed) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION( 0x000224, "gn676_lan:eeprom", 0)
	ROM_LOAD( "gc713ja.2g",   0x000000, 0x000224, BAD_DUMP CRC(600e21e1) SHA1(ca7b30ed4b564aee17a326d1d9729656b5101249) ) // hand built

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676ja_m48t58y.35d", 0x000000, 0x002000, BAD_DUMP CRC(e6eafce5) SHA1(167d957f7cf1e94a12070739a4103977512f6737) ) // hand built
ROM_END

ROM_START(thrillde)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713be01.27p", 0x000000, 0x200000, CRC(d84a7723) SHA1(f4e9e08591b7e5e8419266dbe744d56a185384ed) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION( 0x000224, "gn676_lan:eeprom", 0)
	ROM_LOAD( "gc713ed.2g",   0x000000, 0x000224, BAD_DUMP CRC(8a5bdf3c) SHA1(7cb9b0e30177caf8a623af5ee6ff95ead5eae4f2) ) // hand built

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676ea_m48t58y.35d", 0x000000, 0x002000, BAD_DUMP CRC(b4afab84) SHA1(e8c653df7ce0a5cb77bf7aedeca4c4ff91669047) ) // hand built
ROM_END

ROM_START(thrillda)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713be01.27p", 0x000000, 0x200000, CRC(d84a7723) SHA1(f4e9e08591b7e5e8419266dbe744d56a185384ed) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION( 0x000224, "gn676_lan:eeprom", 0)
	ROM_LOAD( "gc713ac.2g",   0x000000, 0x000224, BAD_DUMP CRC(8db4eed6) SHA1(f99120c18435866354c5deadfacd47453fb53c15) ) // hand built

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676aa_m48t58y.35d", 0x000000, 0x002000, BAD_DUMP CRC(c011dcea) SHA1(39cbbe518bfc256cdb72bdaece03c539f705c807) ) // hand built
ROM_END

ROM_START(thrilldab)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713be01.27p", 0x000000, 0x200000, CRC(d84a7723) SHA1(f4e9e08591b7e5e8419266dbe744d56a185384ed) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION( 0x000224, "gn676_lan:eeprom", 0)
	ROM_LOAD( "gc713ab.2g",   0x000000, 0x000224, BAD_DUMP CRC(f1c26d7d) SHA1(2a3f45d18bc8e9278d077118d478a501531011f4) ) // hand built

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676aa_m48t58y.35d", 0x000000, 0x002000, BAD_DUMP CRC(c011dcea) SHA1(39cbbe518bfc256cdb72bdaece03c539f705c807) ) // hand built
ROM_END

ROM_START(thrilldb)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713bb01.27p", 0x000000, 0x200000, CRC(535fe4e8) SHA1(acd8194a4dafce289dbdfd874f0b799f25aeb73f) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION( 0x000224, "gn676_lan:eeprom", 0)
	ROM_LOAD( "gc713ud.2g",   0x000000, 0x000224, BAD_DUMP CRC(fc9563f5) SHA1(1ed08482024f6d4353d4e4ea6c8092f6625d699b) ) // hand built

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676ua_m48t58y.35d", 0x000000, 0x002000, BAD_DUMP CRC(2aeeda76) SHA1(ed63b5ebbd9ebb90afd7fdebc9ad4b4d9966012b) ) // hand built
ROM_END

ROM_START(thrilldbj)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713bb01.27p", 0x000000, 0x200000, CRC(535fe4e8) SHA1(acd8194a4dafce289dbdfd874f0b799f25aeb73f) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION( 0x000224, "gn676_lan:eeprom", 0)
	ROM_LOAD( "gc713jc.2g",   0x000000, 0x000224, BAD_DUMP CRC(98e326b7) SHA1(af532cf84eca02a3ed0bd0d49c2b142c7f21760c) ) // hand built

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676ja_m48t58y.35d", 0x000000, 0x002000, BAD_DUMP CRC(e6eafce5) SHA1(167d957f7cf1e94a12070739a4103977512f6737) ) // hand built
ROM_END

ROM_START(thrilldbja)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713bb01.27p", 0x000000, 0x200000, CRC(535fe4e8) SHA1(acd8194a4dafce289dbdfd874f0b799f25aeb73f) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION( 0x000224, "gn676_lan:eeprom", 0)
	ROM_LOAD( "gc713ja.2g",   0x000000, 0x000224, BAD_DUMP CRC(600e21e1) SHA1(ca7b30ed4b564aee17a326d1d9729656b5101249) ) // hand built

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676ja_m48t58y.35d", 0x000000, 0x002000, BAD_DUMP CRC(e6eafce5) SHA1(167d957f7cf1e94a12070739a4103977512f6737) ) // hand built
ROM_END

ROM_START(thrilldbe)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713bb01.27p", 0x000000, 0x200000, CRC(535fe4e8) SHA1(acd8194a4dafce289dbdfd874f0b799f25aeb73f) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION( 0x000224, "gn676_lan:eeprom", 0)
	ROM_LOAD( "gc713ed.2g",   0x000000, 0x000224, BAD_DUMP CRC(8a5bdf3c) SHA1(7cb9b0e30177caf8a623af5ee6ff95ead5eae4f2) ) // hand built

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676ea_m48t58y.35d", 0x000000, 0x002000, BAD_DUMP CRC(b4afab84) SHA1(e8c653df7ce0a5cb77bf7aedeca4c4ff91669047) ) // hand built
ROM_END

ROM_START(thrilldba)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713bb01.27p", 0x000000, 0x200000, CRC(535fe4e8) SHA1(acd8194a4dafce289dbdfd874f0b799f25aeb73f) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION( 0x000224, "gn676_lan:eeprom", 0)
	ROM_LOAD( "gc713ac.2g",   0x000000, 0x000224, BAD_DUMP CRC(8db4eed6) SHA1(f99120c18435866354c5deadfacd47453fb53c15) ) // hand built

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676aa_m48t58y.35d", 0x000000, 0x002000, BAD_DUMP CRC(c011dcea) SHA1(39cbbe518bfc256cdb72bdaece03c539f705c807) ) // hand built
ROM_END

ROM_START(thrilldbab)
	ROM_REGION32_BE(0x200000, "prgrom", 0) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713bb01.27p", 0x000000, 0x200000, CRC(535fe4e8) SHA1(acd8194a4dafce289dbdfd874f0b799f25aeb73f) )

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x800000, "master_cgboard", 0) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION( 0x000224, "gn676_lan:eeprom", 0)
	ROM_LOAD( "gc713ab.2g",   0x000000, 0x000224, BAD_DUMP CRC(f1c26d7d) SHA1(2a3f45d18bc8e9278d077118d478a501531011f4) ) // hand built

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm676aa_m48t58y.35d", 0x000000, 0x002000, BAD_DUMP CRC(c011dcea) SHA1(39cbbe518bfc256cdb72bdaece03c539f705c807) ) // hand built
ROM_END

} // Anonymous namespace


/*****************************************************************************/

#define GAME_FLAGS (MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// GNxxx -> 5+R gear shift (8-way joystick), clutch pedal, hand brake lever
// GYxxx -> Same as GN but intended for the 'super deluxe' cab (Cobra system). GY676 RTC is interchangable between Cobra and NWK-TR Racing Jam
// GPxxx, GExxx, GMxxx -> up/down gear shift, no clutch, hand brake lever
// GQxxx -> up/down gear shift, no clutch, no hand brake lever
// Change the first two bytes in the NVRAM and fix the checksum at 0x0e-0x0f (calculated as the negated sum of 0x00-0x0e as 16-bit big endian values)
// to generate new NVRAMs.
GAME( 1998, racingj,    0,        nwktr, nwktr_gq, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam (GQ676UAC)",               GAME_FLAGS )
GAME( 1998, racingje,   racingj,  nwktr, nwktr_gq, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam (GQ676EAC)",               GAME_FLAGS )
GAME( 1998, racingjj,   racingj,  nwktr, nwktr_gq, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam (GQ676JAC)",               GAME_FLAGS )
GAME( 1998, racingja,   racingj,  nwktr, nwktr_gq, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam (GQ676AAC)",               GAME_FLAGS )
GAME( 1998, racingjm,   racingj,  nwktr, nwktr_gm, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam (GM676UAC)",               GAME_FLAGS )
GAME( 1998, racingjme,  racingj,  nwktr, nwktr_gm, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam (GM676EAC)",               GAME_FLAGS )
GAME( 1998, racingjmj,  racingj,  nwktr, nwktr_gm, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam (GM676JAC)",               GAME_FLAGS )
GAME( 1998, racingjma,  racingj,  nwktr, nwktr_gm, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam (GM676AAC)",               GAME_FLAGS )
GAME( 1998, racingjn,   racingj,  nwktr, nwktr_gn, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam (GN676UAC)",               GAME_FLAGS )
GAME( 1998, racingjne,  racingj,  nwktr, nwktr_gn, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam (GN676EAC)",               GAME_FLAGS )
GAME( 1998, racingjnj,  racingj,  nwktr, nwktr_gn, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam (GN676JAC)",               GAME_FLAGS )
GAME( 1998, racingjna,  racingj,  nwktr, nwktr_gn, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam (GN676AAC)",               GAME_FLAGS )

GAME( 1998, racingj2,   0,        nwktr, nwktr_gq, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam: Chapter II (GQ888UAA)",   GAME_FLAGS )
GAME( 1998, racingj2e,  racingj2, nwktr, nwktr_gq, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam: Chapter II (GQ888EAA)",   GAME_FLAGS )
GAME( 1998, racingj2j,  racingj2, nwktr, nwktr_gq, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam: Chapter II (GQ888JAA)",   GAME_FLAGS )
GAME( 1998, racingj2a,  racingj2, nwktr, nwktr_gq, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam: Chapter II (GQ888AAA)",   GAME_FLAGS )
GAME( 1998, racingj2m,  racingj2, nwktr, nwktr_gm, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam: Chapter II (GM888UAA)",   GAME_FLAGS )
GAME( 1998, racingj2me, racingj2, nwktr, nwktr_gm, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam: Chapter II (GM888EAA)",   GAME_FLAGS )
GAME( 1998, racingj2mj, racingj2, nwktr, nwktr_gm, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam: Chapter II (GM888JAA)",   GAME_FLAGS )
GAME( 1998, racingj2ma, racingj2, nwktr, nwktr_gm, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam: Chapter II (GM888AAA)",   GAME_FLAGS )
GAME( 1998, racingj2n,  racingj2, nwktr, nwktr_gn, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam: Chapter II (GN888UAA)",   GAME_FLAGS )
GAME( 1998, racingj2ne, racingj2, nwktr, nwktr_gn, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam: Chapter II (GN888EAA)",   GAME_FLAGS )
GAME( 1998, racingj2nj, racingj2, nwktr, nwktr_gn, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam: Chapter II (GN888JAA)",   GAME_FLAGS )
GAME( 1998, racingj2na, racingj2, nwktr, nwktr_gn, nwktr_state, init_racingj, ROT0, "Konami", "Racing Jam: Chapter II (GN888AAA)",   GAME_FLAGS )

// JAx, ABx --> 5+R gear shift (8-way joystick), clutch pedal, hand brake lever
// JCx, ACx, UDx --> up/down gear shift, no clutch, hand brake lever
// EDx --> prompts you to select if you have a hand brake lever installed, a clutch pedal installed,
// gear shifter type (up/down, 4 pos, or 5+R), and gear shifter's display position.
GAME( 1998, thrilld,    0,       nwktr_lan_b, nwktr_gm, nwktr_state, init_thrilld, ROT0, "Konami", "Thrill Drive (UDE)",             GAME_FLAGS )
GAME( 1998, thrilldj,   thrilld, nwktr_lan_b, nwktr_gm, nwktr_state, init_thrilld, ROT0, "Konami", "Thrill Drive (JCE)",             GAME_FLAGS )
GAME( 1998, thrilldja,  thrilld, nwktr_lan_b, nwktr_gn, nwktr_state, init_thrilld, ROT0, "Konami", "Thrill Drive (JAE)",             GAME_FLAGS )
GAME( 1998, thrillde,   thrilld, nwktr_lan_b, thrillde, nwktr_state, init_thrilld, ROT0, "Konami", "Thrill Drive (EDE)",             GAME_FLAGS )
GAME( 1998, thrillda,   thrilld, nwktr_lan_b, nwktr_gm, nwktr_state, init_thrilld, ROT0, "Konami", "Thrill Drive (ACE)",             GAME_FLAGS )
GAME( 1998, thrilldab,  thrilld, nwktr_lan_b, nwktr_gn, nwktr_state, init_thrilld, ROT0, "Konami", "Thrill Drive (ABE)",             GAME_FLAGS )
GAME( 1998, thrilldb,   thrilld, nwktr_lan_b, nwktr_gm, nwktr_state, init_thrilld, ROT0, "Konami", "Thrill Drive (UDB)",             GAME_FLAGS )
GAME( 1998, thrilldbj,  thrilld, nwktr_lan_b, nwktr_gm, nwktr_state, init_thrilld, ROT0, "Konami", "Thrill Drive (JCB)",             GAME_FLAGS )
GAME( 1998, thrilldbja, thrilld, nwktr_lan_b, nwktr_gn, nwktr_state, init_thrilld, ROT0, "Konami", "Thrill Drive (JAB)",             GAME_FLAGS )
GAME( 1998, thrilldbe,  thrilld, nwktr_lan_b, thrillde, nwktr_state, init_thrilld, ROT0, "Konami", "Thrill Drive (EDB)",             GAME_FLAGS )
GAME( 1998, thrilldba,  thrilld, nwktr_lan_b, nwktr_gm, nwktr_state, init_thrilld, ROT0, "Konami", "Thrill Drive (ACB)",             GAME_FLAGS )
GAME( 1998, thrilldbab, thrilld, nwktr_lan_b, nwktr_gn, nwktr_state, init_thrilld, ROT0, "Konami", "Thrill Drive (ABB)",             GAME_FLAGS )
