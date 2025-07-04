// license:BSD-3-Clause
// copyright-holders:Ville Linde
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
    Operation: Thunder Hurricane   | GX680     | GN672(A)     | GN678(B)       | GN680(E) I/O board
    Solar Assault                  | GX792     | GN672(A)     | GN678(B)       |

    Hang Pilot                     | GN685     | GN672(A)     | 2x ??          | 3dfx-based CG boards


Konami 'GTI Club' Hardware
Konami, 1996-1998

Known games on this hardware include....

Game                        (C)      Year
-----------------------------------------
GTI Club: Rally Cote D'Azur Konami   1996
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
GTI Club: Rally Cote D'Azur 688A07  688A12  688A11  688A10  688A09  688A06  688A05  688AAA04  688AAA02  688AAA03  688AAA01
Jet Wave                    - see note -
Hang Pilot                  685A07  -       -       685A10  685A09  685A06  685A05  685JAB04  685JAB02  685JAB03  685JAB01
Operation Thunder Hurricane 680A07  680A12  680A11  680A10  680A09  680A06  680A05  680UAA04  680UAA02  680UAA03  680UAA01
Solar Assault               792A07  792A12  792A11  792A10  792A09  792A06  792A05  792UAA04  792UAA02  792UAA03  792UAA01
Solar Assault : Revised     - N/A -

Note : Jet Wave uses the lower board (GN678) from GTI Club, but it uses a different top board (ZR107 PWB(A)300769A)
Check zr107.cpp for details on the top board.

Operation Thunder Hurricane uses an additional top board gun/analog controls. Analog inputs are controlled by two CCD
cameras, one from each gun. This specific variation uses a K056230 for networking between the cpu board to receive
the analog values that way. Teraburst uses a different variation of this I/O board replacing the K056230 with a K056800 (see konami/hornet.cpp).

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
      68EC000 @ 16MHz (32/2)
      CN11/12 - Power connectors
      CN8/9   - 6-pin analog control connectors (to CCD cameras)
      CN10    - 4-pin power connector for IR emitters
      CN4/5   - Pin jack/network connectors (to cpu board)
      NRPS11  - Idec NRPS11 PC Board circuit protector
      LM1881  - Video sync separator (DIP8)
      056230  - Konami Custom (QFP80)



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
GTI Club: Rally Cote D'Azur -       688A16  -       688A15  -       688A14  -       688A13
Jet Wave                    -       678A16  -       678A15  -       678A14  -       678A13
Operation Thunder Hurricane -       680A16  -       680A15  -       680A14  -       680A13
Solar Assault               -       792A16  -       792A15  -       792A14  -       792A13
Solar Assault : Revised     - N/A -

Hang Pilot (uses an unknown but similar video board)                12W             4W
                            -       -       -       -       -       678A14  -       678A13

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/sharc/sharc.h"
#include "machine/adc1038.h"
#include "machine/eepromser.h"
#include "machine/k033906.h"
#include "machine/k056230.h"
#include "konppc.h"
#include "sound/k056800.h"
#include "sound/rf5c400.h"
#include "k001005.h"
#include "k001006.h"
#include "k001604.h"
#include "video/voodoo.h"

#include "emupal.h"
#include "layout/generic.h"
#include "speaker.h"


#define LOG_SYSREG (1 << 1)

#define LOG_ALL (LOG_SYSREG)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGSYSREG(...) LOGMASKED(LOG_SYSREG, __VA_ARGS__)

namespace {

#define DEBUG_GTI (0)

class gticlub_base_state : public driver_device
{
protected:
	gticlub_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_dsp(*this, "dsp%u", 1U)
		, m_k056800(*this, "k056800")
		, m_adc1038(*this, "adc1038")
		, m_eeprom(*this, "eeprom")
		, m_palette(*this, "palette%u", 1U)
		, m_konppc(*this, "konppc")
		, m_k056230(*this, "k056230")
		, m_k001604(*this, "k001604%u", 1U)
		, m_work_ram(*this, "work_ram")
		, m_sharc_dataram(*this, "sharc%u_dataram", 0U)
		, m_cgboard_bank(*this, "cgboard_%u_bank", 0U)
		, m_analog(*this, "AN%u", 0U)
		, m_ports(*this, "IN%u", 0)
		, m_pcb_digit(*this, "pcbdigit%u", 0U)
		, m_cg_view(*this, "cg_view")
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// TODO: Needs verification on real hardware
	static const int m_sound_timer_usec = 2400;

	required_device<ppc_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device_array<adsp21062_device, 2> m_dsp;
	required_device<k056800_device> m_k056800;
	required_device<adc1038_device> m_adc1038;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device_array<palette_device, 2> m_palette;
	required_device<konppc_device> m_konppc;
	required_device<k056230_device> m_k056230;
	optional_device_array<k001604_device, 2> m_k001604;
	required_shared_ptr<uint32_t> m_work_ram;
	optional_shared_ptr_array<uint32_t, 2> m_sharc_dataram;
	optional_memory_bank_array<2> m_cgboard_bank;
	optional_ioport_array<4> m_analog;
	required_ioport_array<4> m_ports;
	output_finder<2> m_pcb_digit;
	memory_view m_cg_view;

	emu_timer *m_sound_irq_timer = nullptr;

	uint8_t sysreg_r(offs_t offset);
	void sysreg_w(offs_t offset, uint8_t data);
	void soundtimer_en_w(uint16_t data);
	void soundtimer_count_w(uint16_t data);

	TIMER_CALLBACK_MEMBER(sound_irq);

	int adc1038_input_callback(int input);

	void sound_memmap(address_map &map) ATTR_COLD;
};

// with GN678 Video board
class gticlub_state : public gticlub_base_state
{
public:
	gticlub_state(const machine_config &mconfig, device_type type, const char *tag)
		: gticlub_base_state(mconfig, type, tag)
		, m_k001005(*this, "k001005")
		, m_k001006(*this, "k001006%u", 1U)
	{ }

	void slrasslt(machine_config &config);
	void gticlub(machine_config &config);

	void init_gticlub();

protected:
	required_device<k001005_device> m_k001005;
	required_device_array<k001006_device, 2> m_k001006;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void gticlub_map(address_map &map) ATTR_COLD;
	void sharc_map(address_map &map) ATTR_COLD;

#if DEBUG_GTI
	uint8_t m_tick = 0;
	int8_t m_debug_tex_page = 0;
	int8_t m_debug_tex_palette = 0;
#endif
};

// with GN680 I/O board
class thunderh_state : public gticlub_state
{
public:
	thunderh_state(const machine_config &mconfig, device_type type, const char *tag)
		: gticlub_state(mconfig, type, tag)
		, m_gn680(*this, "gn680")
	{ }

	void thunderh(machine_config &config);

private:
	required_device<cpu_device> m_gn680;

	void gn680_sysctrl_w(uint16_t data);

	void gn680_memmap(address_map &map) ATTR_COLD;
};

// with Voodoo based video board
class hangplt_state : public gticlub_base_state
{
public:
	hangplt_state(const machine_config &mconfig, device_type type, const char *tag)
		: gticlub_base_state(mconfig, type, tag)
		, m_voodoo(*this, "voodoo%u", 0U)
	{ }

	void hangplt(machine_config &config);

	void init_hangplt_common();
	void init_hangplt();
	void init_hangpltu();

private:
	required_device_array<generic_voodoo_device, 2> m_voodoo;

	template <uint8_t Which> uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void hangplt_map(address_map &map) ATTR_COLD;
	template <unsigned Board> void hangplt_sharc_map(address_map &map) ATTR_COLD;
};


/******************************************************************/

uint8_t gticlub_base_state::sysreg_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
		case 1:
		case 3:
			return m_ports[offset]->read();

		case 2:
			return m_adc1038->sars_read() << 7;

		case 4:
		{
			// 7        0
			// |?????ae?|
			//
			// a = ADC readout
			// e = EEPROM data out

			uint32_t const eeprom_bit = (m_eeprom->do_read() << 1);
			uint32_t const adc_bit = (m_adc1038->do_read() << 2);
			return (eeprom_bit | adc_bit);
		}

		default:
			if (!machine().side_effects_disabled())
				LOGSYSREG("sysreg_r %d\n", offset);
			break;
	}
	return 0;
}

void gticlub_base_state::sysreg_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
		case 1:
			m_pcb_digit[offset] = bitswap<7>(~data,0,1,2,3,4,5,6);
			break;

		case 3:
			m_eeprom->di_write(BIT(data, 0));
			m_eeprom->clk_write(BIT(data, 1));
			m_eeprom->cs_write(BIT(data, 2));
			break;

		case 4:
			if (BIT(data, 7))    // CG Board 1 IRQ Ack
				m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);

			if (BIT(data, 6))    // CG Board 0 IRQ Ack
				m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

			m_adc1038->di_write(BIT(data, 0));
			m_adc1038->clk_write(BIT(data, 1));

			m_konppc->set_cgboard_id((data >> 4) & 0x3);
			m_cg_view.select(m_konppc->get_cgboard_id() ? 1 : 0);
			break;

		default:
			break;
	}
}

/******************************************************************/

TIMER_CALLBACK_MEMBER(gticlub_base_state::sound_irq)
{
	m_audiocpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
}


void gticlub_base_state::soundtimer_en_w(uint16_t data)
{
	if (BIT(data, 0))
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

void gticlub_base_state::soundtimer_count_w(uint16_t data)
{
	// Reset the count
	m_sound_irq_timer->adjust(attotime::from_usec(m_sound_timer_usec));
	m_audiocpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}

/******************************************************************/

void gticlub_base_state::machine_start()
{
	m_pcb_digit.resolve();

	// set conservative DRC options
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	// configure fast RAM regions for DRC
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x000fffff, false, m_work_ram);

	m_sound_irq_timer = timer_alloc(FUNC(gticlub_state::sound_irq), this);
}

void gticlub_state::gticlub_map(address_map &map)
{
	map(0x00000000, 0x000fffff).ram().share(m_work_ram);
	map(0x74000000, 0x7407ffff).view(m_cg_view);
	m_cg_view[0](0x74000000, 0x740000ff).rw(m_k001604[0], FUNC(k001604_device::reg_r), FUNC(k001604_device::reg_w));
	m_cg_view[0](0x74010000, 0x7401ffff).ram().w(m_palette[0], FUNC(palette_device::write32)).share("palette1");
	m_cg_view[0](0x74020000, 0x7403ffff).rw(m_k001604[0], FUNC(k001604_device::tile_r), FUNC(k001604_device::tile_w));
	m_cg_view[0](0x74040000, 0x7407ffff).rw(m_k001604[0], FUNC(k001604_device::char_r), FUNC(k001604_device::char_w));
	map(0x78000000, 0x7800ffff).rw(m_konppc, FUNC(konppc_device::cgboard_dsp_shared_r_ppc), FUNC(konppc_device::cgboard_dsp_shared_w_ppc));
	map(0x78040000, 0x7804000f).rw(m_k001006[0], FUNC(k001006_device::read), FUNC(k001006_device::write));
	map(0x78080000, 0x7808000f).rw(m_k001006[1], FUNC(k001006_device::read), FUNC(k001006_device::write));
	map(0x780c0000, 0x780c0003).rw(m_konppc, FUNC(konppc_device::cgboard_dsp_comm_r_ppc), FUNC(konppc_device::cgboard_dsp_comm_w_ppc));
	map(0x7e000000, 0x7e003fff).rw(FUNC(gticlub_state::sysreg_r), FUNC(gticlub_state::sysreg_w));
	map(0x7e008000, 0x7e009fff).m(m_k056230, FUNC(k056230_device::regs_map));
	map(0x7e00a000, 0x7e00bfff).rw(m_k056230, FUNC(k056230_device::ram_r), FUNC(k056230_device::ram_w));
	map(0x7e00c000, 0x7e00c00f).rw(m_k056800, FUNC(k056800_device::host_r), FUNC(k056800_device::host_w));
	map(0x7f000000, 0x7f3fffff).rom().region("datarom", 0);
	map(0x7f800000, 0x7f9fffff).rom().region("prgrom", 0);
	map(0x7fe00000, 0x7fffffff).rom().region("prgrom", 0);
}

void hangplt_state::hangplt_map(address_map &map)
{
	map(0x00000000, 0x000fffff).ram().share(m_work_ram);
	map(0x74000000, 0x7407ffff).view(m_cg_view);
	m_cg_view[0](0x74000000, 0x740000ff).rw(m_k001604[0], FUNC(k001604_device::reg_r), FUNC(k001604_device::reg_w));
	m_cg_view[0](0x74010000, 0x7401ffff).ram().w(m_palette[0], FUNC(palette_device::write32)).share("palette1");
	m_cg_view[0](0x74020000, 0x7403ffff).rw(m_k001604[0], FUNC(k001604_device::tile_r), FUNC(k001604_device::tile_w));
	m_cg_view[0](0x74040000, 0x7407ffff).rw(m_k001604[0], FUNC(k001604_device::char_r), FUNC(k001604_device::char_w));
	m_cg_view[1](0x74000000, 0x740000ff).rw(m_k001604[1], FUNC(k001604_device::reg_r), FUNC(k001604_device::reg_w));
	m_cg_view[1](0x74010000, 0x7401ffff).ram().w(m_palette[1], FUNC(palette_device::write32)).share("palette2");
	m_cg_view[1](0x74020000, 0x7403ffff).rw(m_k001604[1], FUNC(k001604_device::tile_r), FUNC(k001604_device::tile_w));
	m_cg_view[1](0x74040000, 0x7407ffff).rw(m_k001604[1], FUNC(k001604_device::char_r), FUNC(k001604_device::char_w));
	map(0x78000000, 0x7800ffff).rw(m_konppc, FUNC(konppc_device::cgboard_dsp_shared_r_ppc), FUNC(konppc_device::cgboard_dsp_shared_w_ppc));
	map(0x780c0000, 0x780c0003).rw(m_konppc, FUNC(konppc_device::cgboard_dsp_comm_r_ppc), FUNC(konppc_device::cgboard_dsp_comm_w_ppc));
	map(0x7e000000, 0x7e003fff).rw(FUNC(hangplt_state::sysreg_r), FUNC(hangplt_state::sysreg_w));
	map(0x7e008000, 0x7e009fff).m(m_k056230, FUNC(k056230_device::regs_map));
	map(0x7e00a000, 0x7e00bfff).rw(m_k056230, FUNC(k056230_device::ram_r), FUNC(k056230_device::ram_w));
	map(0x7e00c000, 0x7e00c00f).rw(m_k056800, FUNC(k056800_device::host_r), FUNC(k056800_device::host_w));
	map(0x7f000000, 0x7f3fffff).rom().region("datarom", 0);
	map(0x7f800000, 0x7f9fffff).rom().region("prgrom", 0);
	map(0x7fe00000, 0x7fffffff).rom().region("prgrom", 0);
}

/**********************************************************************/

void gticlub_base_state::sound_memmap(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x20ffff).ram();
	map(0x300000, 0x30001f).rw(m_k056800, FUNC(k056800_device::sound_r), FUNC(k056800_device::sound_w)).umask16(0x00ff);
	map(0x400000, 0x400fff).rw("rfsnd", FUNC(rf5c400_device::rf5c400_r), FUNC(rf5c400_device::rf5c400_w));      // Ricoh RF5C400
	map(0x500000, 0x500001).w(FUNC(gticlub_state::soundtimer_en_w)).nopr();
	map(0x600000, 0x600001).w(FUNC(gticlub_state::soundtimer_count_w)).nopr();
}

/*****************************************************************************/

void thunderh_state::gn680_sysctrl_w(uint16_t data)
{
	// bit 15 = watchdog toggle
	// lower 4 bits = LEDs?
}

// WORD at 30000e: IRQ 5 tests bits 6 and 7, IRQ 6 tests bits 4 and 5
// IRQ 3 tests for network/056230 at 310000 to communicate with the main pcb

void thunderh_state::gn680_memmap(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x200000, 0x203fff).ram();
	map(0x300000, 0x300001).w(FUNC(thunderh_state::gn680_sysctrl_w));
//  map(0x310000, 0x311fff).nopw(); //056230 regs?
//  map(0x312000, 0x313fff).nopw(); //056230 ram?
}

/*****************************************************************************/

void gticlub_state::sharc_map(address_map &map)
{
	map(0x400000, 0x41ffff).rw(m_konppc, FUNC(konppc_device::cgboard_shared_sharc_r<0>), FUNC(konppc_device::cgboard_shared_sharc_w<0>));
	map(0x500000, 0x5fffff).ram().share(m_sharc_dataram[0]).lr32(NAME([this](offs_t offset) { return m_sharc_dataram[0][offset] & 0xffff; }));
	map(0x600000, 0x6fffff).rw(m_k001005, FUNC(k001005_device::read), FUNC(k001005_device::write));
	map(0x700000, 0x7000ff).rw(m_konppc, FUNC(konppc_device::cgboard_comm_sharc_r<0>), FUNC(konppc_device::cgboard_comm_sharc_w<0>));
}

template <unsigned Board>
void hangplt_state::hangplt_sharc_map(address_map &map)
{
	map(0x0400000, 0x041ffff).rw(m_konppc, FUNC(konppc_device::cgboard_shared_sharc_r<Board>), FUNC(konppc_device::cgboard_shared_sharc_w<Board>));
	map(0x0500000, 0x05fffff).ram().share(m_sharc_dataram[Board]).lr32(NAME([this](offs_t offset) { return m_sharc_dataram[Board][offset] & 0xffff; }));
	map(0x1400000, 0x14fffff).ram();
	map(0x2400000, 0x27fffff).r(m_konppc, FUNC(konppc_device::nwk_voodoo_r<Board>)).w(m_voodoo[Board], FUNC(generic_voodoo_device::write));
	map(0x3400000, 0x34000ff).rw(m_konppc, FUNC(konppc_device::cgboard_comm_sharc_r<Board>), FUNC(konppc_device::cgboard_comm_sharc_w<Board>));
	map(0x3401000, 0x34fffff).w(m_konppc, FUNC(konppc_device::nwk_voodoo_fifo_w<Board>));
	map(0x3500000, 0x3507fff).rw(m_konppc, FUNC(konppc_device::cgboard_k033906_r<Board>), FUNC(konppc_device::cgboard_k033906_w<Board>));
	map(0x3600000, 0x37fffff).bankr(m_cgboard_bank[Board]);
}

/*****************************************************************************/

static INPUT_PORTS_START( gticlub )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("Start/View")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Shift Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Shift Down")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("AT/MT Switch") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_9)
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
	PORT_DIPNAME( 0x03, 0x03, "Network ID" ) PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x01, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "DIP3" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("AN0")   // mask default type             sens delta min max
	PORT_BIT( 0x3ff, 0x200, IPT_PADDLE ) PORT_NAME("Steering Wheel") PORT_MINMAX(0x004,0x3fb) PORT_SENSITIVITY(50) PORT_KEYDELTA(25)

	PORT_START("AN1")
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL ) PORT_NAME("Gas Pedal") PORT_MINMAX(0x000,0x25f) PORT_SENSITIVITY(50) PORT_KEYDELTA(25)

	PORT_START("AN2")
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL2 ) PORT_NAME("Brake Pedal") PORT_MINMAX(0x000,0x25f) PORT_SENSITIVITY(50) PORT_KEYDELTA(25)

	PORT_START("AN3")
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL3 ) PORT_NAME("Handbrake Lever") PORT_MINMAX(0x000,0x25f) PORT_SENSITIVITY(50) PORT_KEYDELTA(25)
INPUT_PORTS_END

static INPUT_PORTS_START( slrasslt )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start/View")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Trigger")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Missile")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Power Up")

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x01, 0x01, "DIP1" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP2" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP3" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("AN0")
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_Y ) PORT_MINMAX(0x008,0x3f7) PORT_SENSITIVITY(50) PORT_KEYDELTA(25)

	PORT_START("AN1")
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_X ) PORT_MINMAX(0x008,0x3f7) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_REVERSE

	PORT_START("AN2")
	PORT_BIT( 0x3ff, 0x000, IPT_UNUSED )

	PORT_START("AN3")
	PORT_BIT( 0x3ff, 0x000, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( thunderh )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1) PORT_NAME("P1 Trigger")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1) PORT_NAME("P1 Bomb")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2) PORT_NAME("P2 Trigger")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2) PORT_NAME("P2 Bomb")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x01, 0x00, "DIP1" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP2" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP3" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hangplt )
	PORT_INCLUDE( slrasslt )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start/View")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Select Left") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Select Right") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Push limit switch")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Pull limit switch")

	// TODO: The test mode for this game shows 8 dip switches
	// verify if they are read anywhere (or physically mapped for that matter).
	PORT_MODIFY("IN3")
	PORT_DIPNAME( 0x01, 0x01, "Disable Machine Init" ) PORT_DIPLOCATION("SW:1") // NOTE: Disabling Machine Init also disables analog controls
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Skip Post" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_MODIFY("AN0")
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_X ) PORT_NAME("Rudder") PORT_MINMAX(0x1c5,0x24a) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_MODIFY("AN1")
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_Y ) PORT_NAME("Control Bar") PORT_MINMAX(0x100,0x2ff) PORT_SENSITIVITY(50) PORT_KEYDELTA(20) PORT_REVERSE
INPUT_PORTS_END

int gticlub_base_state::adc1038_input_callback(int input)
{
	switch (input)
	{
	case 0:  return m_analog[0]->read();
	case 1:  return m_analog[1]->read();
	case 2:  return m_analog[2]->read();
	case 3:  return m_analog[3]->read();
	default: return 0;
	}
}

void gticlub_base_state::machine_reset()
{
	m_dsp[0]->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	if (m_dsp[1].found())
		m_dsp[1]->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

uint32_t gticlub_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_k001604[0]->draw_back_layer(screen, bitmap, cliprect);

	if (m_konppc[0].output_3d_enabled())
	{
		m_k001005->draw(bitmap, cliprect);
	}

	m_k001604[0]->draw_front_layer(screen, bitmap, cliprect);

#if DEBUG_GTI // TODO: won't compile since k001006 has been devicified and its palette is private
	m_tick++;
	if( m_tick >= 5 ) {
		m_tick = 0;

		if( machine().input().code_pressed(KEYCODE_O) )
			m_debug_tex_page++;

		if( machine().input().code_pressed(KEYCODE_I) )
			m_debug_tex_page--;

		if (machine().input().code_pressed(KEYCODE_U))
			m_debug_tex_palette++;
		if (machine().input().code_pressed(KEYCODE_Y))
			m_debug_tex_palette--;

		if (m_debug_tex_page < 0)
			m_debug_tex_page = 32;
		if (m_debug_tex_page > 32)
			m_debug_tex_page = 0;

		if (m_debug_tex_palette < 0)
			m_debug_tex_palette = 15;
		if (m_debug_tex_palette > 15)
			m_debug_tex_palette = 0;
	}

	if (m_debug_tex_page > 0)
	{
		char string[200];
		int x,y;
		int index = (m_debug_tex_page - 1) * 0x40000;
		int pal = m_debug_tex_palette & 7;
		int tp = (m_debug_tex_palette >> 3) & 1;
		uint8_t *rom = memregion("textures")->base();

		for (y=0; y < 384; y++)
		{
			for (x=0; x < 512; x++)
			{
				uint8_t pixel = rom[index + (y*512) + x];
				bitmap.pix(y, x) = K001006_palette[tp][(pal * 256) + pixel];
			}
		}

		sprintf(string, "Texture page %d\nPalette %d", m_debug_tex_page, m_debug_tex_palette);
		//popmessage("%s", string);
	}
#endif

	//m_dsp[0]->set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
	m_dsp[0]->set_flag_input(1, ASSERT_LINE);
	return 0;
}

template <uint8_t Which>
uint32_t hangplt_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette[Which]->pen(0), cliprect);

	// FIXME: service mode cross hatch comes from this layer (which somehow also disables 3d render)
//  m_k001604[Which]->draw_back_layer(screen, bitmap, cliprect);
	m_voodoo[Which]->update(bitmap, cliprect);
	m_k001604[Which]->draw_front_layer(screen, bitmap, cliprect);

	return 0;
}

/* PowerPC interrupts

    IRQ0:  Vblank
    IRQ2:  LANC
    DMA0

*/

void gticlub_state::gticlub(machine_config &config)
{
	// basic machine hardware
	PPC403GA(config, m_maincpu, XTAL(64'000'000)/2);   // PowerPC 403GA 32MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &gticlub_state::gticlub_map);
	m_maincpu->set_vblank_int("screen", FUNC(gticlub_state::irq0_line_assert));

	M68000(config, m_audiocpu, XTAL(64'000'000)/4);    // 16MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &gticlub_state::sound_memmap);

	ADSP21062(config, m_dsp[0], XTAL(36'000'000));
	m_dsp[0]->set_boot_mode(adsp21062_device::BOOT_MODE_EPROM);
	m_dsp[0]->set_addrmap(AS_DATA, &gticlub_state::sharc_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	EEPROM_93C56_16BIT(config, "eeprom");

	ADC1038(config, m_adc1038, 0);
	m_adc1038->set_input_callback(FUNC(gticlub_state::adc1038_input_callback));
	m_adc1038->set_gti_club_hack(true);

	K056230(config, m_k056230);
	m_k056230->irq_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ2);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(1024, 1024);
	screen.set_visarea(40, 511+40, 28, 383+28);     // needs CRTC emulation
	screen.set_screen_update(FUNC(gticlub_state::screen_update));

	PALETTE(config, m_palette[0]).set_format(4, raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 10,5,0>, 16384);

	K001604(config, m_k001604[0], 0);
	m_k001604[0]->set_palette(m_palette[0]);

	K001005(config, m_k001005, 0, m_k001006[0]);

	K001006(config, m_k001006[0], 0);
	m_k001006[0]->set_gfx_region("textures");

	// The second K001006 chip connects to the second K001005 chip.
	// Hook this up when the K001005 separation is understood (seems the load balancing is done on hardware).
	K001006(config, m_k001006[1], 0);
	m_k001006[1]->set_gfx_region("textures");

	K056800(config, m_k056800, XTAL(33'868'800)/2);
	m_k056800->int_callback().set_inputline(m_audiocpu, M68K_IRQ_2);

	SPEAKER(config, "speaker", 2).front();

	rf5c400_device &rfsnd(RF5C400(config, "rfsnd", XTAL(33'868'800)/2));
	rfsnd.add_route(0, "speaker", 1.0, 0);
	rfsnd.add_route(1, "speaker", 1.0, 1);

	KONPPC(config, m_konppc, 0);
	m_konppc->set_dsp_tag(0, m_dsp[0]);
	m_konppc->set_num_boards(1);
	m_konppc->set_cgboard_type(konppc_device::CGBOARD_TYPE_GTICLUB);
}

void thunderh_state::thunderh(machine_config &config)
{
	gticlub(config);

	m_adc1038->set_gti_club_hack(false);

	// TODO: replace K056230 from main gticlub config with a LANC tied to gn680 I/O board

	M68000(config, m_gn680, XTAL(32'000'000) / 2); // 16MHz
	m_gn680->set_addrmap(AS_PROGRAM, &thunderh_state::gn680_memmap);
}

void gticlub_state::slrasslt(machine_config &config)
{
	gticlub(config);

	m_adc1038->set_gti_club_hack(false);
}

void hangplt_state::hangplt(machine_config &config)
{
	// basic machine hardware
	PPC403GA(config, m_maincpu, XTAL(64'000'000)/2);   // PowerPC 403GA 32MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &hangplt_state::hangplt_map);

	M68000(config, m_audiocpu, XTAL(64'000'000)/4);    // 16MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &hangplt_state::sound_memmap);

	ADSP21062(config, m_dsp[0], XTAL(36'000'000));
	m_dsp[0]->set_boot_mode(adsp21062_device::BOOT_MODE_EPROM);
	m_dsp[0]->set_addrmap(AS_DATA, &hangplt_state::hangplt_sharc_map<0>);

	ADSP21062(config, m_dsp[1], XTAL(36'000'000));
	m_dsp[1]->set_boot_mode(adsp21062_device::BOOT_MODE_EPROM);
	m_dsp[1]->set_addrmap(AS_DATA, &hangplt_state::hangplt_sharc_map<1>);

	config.set_maximum_quantum(attotime::from_hz(6000));

	EEPROM_93C56_16BIT(config, "eeprom");

	ADC1038(config, m_adc1038, 0);
	m_adc1038->set_input_callback(FUNC(hangplt_state::adc1038_input_callback));

	K056230(config, m_k056230);
	m_k056230->irq_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ2);

	VOODOO_1(config, m_voodoo[0], voodoo_1_device::NOMINAL_CLOCK);
	m_voodoo[0]->set_fbmem(2);
	m_voodoo[0]->set_tmumem(2,2);
	m_voodoo[0]->set_status_cycles(1000); // optimization to consume extra cycles when polling status
	m_voodoo[0]->set_screen("lscreen");
	m_voodoo[0]->set_cpu(m_dsp[0]);
	m_voodoo[0]->vblank_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_voodoo[0]->stall_callback().set(m_dsp[0], FUNC(adsp21062_device::write_stall));

	VOODOO_1(config, m_voodoo[1], voodoo_1_device::NOMINAL_CLOCK);
	m_voodoo[1]->set_fbmem(2);
	m_voodoo[1]->set_tmumem(2,2);
	m_voodoo[1]->set_status_cycles(1000); // optimization to consume extra cycles when polling status
	m_voodoo[1]->set_screen("rscreen");
	m_voodoo[1]->set_cpu(m_dsp[1]);
	m_voodoo[1]->vblank_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	m_voodoo[1]->stall_callback().set(m_dsp[1], FUNC(adsp21062_device::write_stall));

	K033906(config, "k033906_1", 0, m_voodoo[0]);
	K033906(config, "k033906_2", 0, m_voodoo[1]);

	// video hardware
	PALETTE(config, m_palette[0]).set_format(4, raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 10,5,0>, 16384);
	PALETTE(config, m_palette[1]).set_format(4, raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 10,5,0>, 16384);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_size(600, 420);
	lscreen.set_visarea(44, 555, 27, 410);
	lscreen.set_screen_update(FUNC(hangplt_state::screen_update<0>));

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_size(600, 420);
	rscreen.set_visarea(44, 555, 27, 410);
	rscreen.set_screen_update(FUNC(hangplt_state::screen_update<1>));

	K001604(config, m_k001604[0], 0);
	m_k001604[0]->set_palette(m_palette[0]);

	K001604(config, m_k001604[1], 0);
	m_k001604[1]->set_palette(m_palette[1]);

	K056800(config, m_k056800, XTAL(33'868'800)/2);
	m_k056800->int_callback().set_inputline(m_audiocpu, M68K_IRQ_2);

	SPEAKER(config, "speaker", 2).front();

	rf5c400_device &rfsnd(RF5C400(config, "rfsnd", XTAL(33'868'800)/2));
	rfsnd.add_route(0, "speaker", 1.0, 0);
	rfsnd.add_route(1, "speaker", 1.0, 1);

	KONPPC(config, m_konppc, 0);
	m_konppc->set_dsp_tag(0, m_dsp[0]);
	m_konppc->set_dsp_tag(1, m_dsp[1]);
	m_konppc->set_k033906_tag(0, "k033906_1");
	m_konppc->set_k033906_tag(1, "k033906_2");
	m_konppc->set_voodoo_tag(0, m_voodoo[0]);
	m_konppc->set_voodoo_tag(1, m_voodoo[1]);
	m_konppc->set_texture_bank_tag(0, m_cgboard_bank[0]);
	m_konppc->set_texture_bank_tag(1, m_cgboard_bank[1]);
	m_konppc->set_num_boards(2);
	m_konppc->set_cgboard_type(konppc_device::CGBOARD_TYPE_HANGPLT);
}

/*************************************************************************/

ROM_START( gticlub ) // Euro version EAA - Reports: GTI CLUB(TM) System ver 1.00(EUR)
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE("688eaa01.21u", 0x000000, 0x80000, CRC(824944ad) SHA1(a7bb86a2495e0579f5d82808aeed4895be2dbe3b) )
	ROM_LOAD32_BYTE("688eaa02.19u", 0x000001, 0x80000, CRC(88e7bfb9) SHA1(fc0e945291204ee0c82bbd2c81ff241e1565c6ae) )
	ROM_LOAD32_BYTE("688eaa03.21r", 0x000002, 0x80000, CRC(ea1c696b) SHA1(fd778afaa1de3a35b38a67b8e4c9a08fe9cf1b9e) )
	ROM_LOAD32_BYTE("688eaa04.19r", 0x000003, 0x80000, CRC(94fa2334) SHA1(04edf840f841b9713fa93e7ebb6aad2000b738c0) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP("688a05.14u", 0x000000, 0x200000, CRC(7caa3f80) SHA1(28409dc17c4e010173396fdc069a409fbea0d58d) )
	ROM_LOAD32_WORD_SWAP("688a06.12u", 0x000002, 0x200000, CRC(83e7ce0a) SHA1(afe185f6ed700baaf4c8affddc29f8afdfec4423) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68k program
	ROM_LOAD16_WORD_SWAP( "688a07.13k", 0x000000, 0x040000, CRC(f0805f06) SHA1(4b87e02b89e7ea812454498603767668e4619025) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)    // sound roms
	ROM_LOAD( "688a09.9s", 0x000000, 0x200000, CRC(fb582963) SHA1(ce8fe6a4d7ac7d7f4b6591f9150b1d351e636354) )
	ROM_LOAD( "688a10.7s", 0x200000, 0x200000, CRC(b3ddc5f1) SHA1(a3f76c86e85eb17f20efb037c1ad64e9cb8566c8) )
	ROM_LOAD( "688a11.5s", 0x400000, 0x200000, CRC(fc706183) SHA1(c8ce6de0588be1023ef48577bc88a4e5effdcd25) )
	ROM_LOAD( "688a12.2s", 0x600000, 0x200000, CRC(510c70e3) SHA1(5af77bc98772ab7961308c3af0a80cb1bca690e3) )

	ROM_REGION(0x800000, "textures", 0) // texture roms
	ROM_LOAD64_WORD( "688a13.18d", 0x000000, 0x200000, CRC(c8f04f91) SHA1(9da8cc3a94dbf0a1fce87c2bc9249df712ae0b0d) )
	ROM_LOAD64_WORD( "688a14.13d", 0x000002, 0x200000, CRC(b9932735) SHA1(2492244d2acb350974202a6718bc7121325d2121) )
	ROM_LOAD64_WORD( "688a15.9d",  0x000004, 0x200000, CRC(8aadee51) SHA1(be9020a47583da9d4ff586d227836dc5b7dc31f0) )
	ROM_LOAD64_WORD( "688a16.4d",  0x000006, 0x200000, CRC(7f4e1893) SHA1(585be7b31ab7a48300c22b00443b00d631f4c49d) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c56.24g", 0x0000, 0x0100, CRC(9564a685) SHA1(ec19f3d6e3a55eac4dab6da5ede7216f002b3186) )
ROM_END

ROM_START( gticlubu ) // USA version UAA - Reports: GTI CLUB(TM) System ver 1.02(USA)
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE("688uaa01.21u", 0x000000, 0x80000, CRC(4e2ea7ad) SHA1(cc517df7c4df098896a2a88843fef97c9beb46f3) )
	ROM_LOAD32_BYTE("688uaa02.19u", 0x000001, 0x80000, CRC(c0212ce1) SHA1(7716acfa1b1391e9d7a321ed46785c144d27fdd8) )
	ROM_LOAD32_BYTE("688uaa03.21r", 0x000002, 0x80000, CRC(030246fe) SHA1(70d3591159b07aaeca60141db44f7c28d1b2dac9) )
	ROM_LOAD32_BYTE("688uaa04.19r", 0x000003, 0x80000, CRC(9394e0b2) SHA1(9ff4ff22a307352bf127fc2b5ef9c56ecacf0aab) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP("688a05.14u", 0x000000, 0x200000, CRC(7caa3f80) SHA1(28409dc17c4e010173396fdc069a409fbea0d58d) )
	ROM_LOAD32_WORD_SWAP("688a06.12u", 0x000002, 0x200000, CRC(83e7ce0a) SHA1(afe185f6ed700baaf4c8affddc29f8afdfec4423) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68k program
	ROM_LOAD16_WORD_SWAP( "688a07.13k", 0x000000, 0x040000, CRC(f0805f06) SHA1(4b87e02b89e7ea812454498603767668e4619025) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)    // sound roms
	ROM_LOAD( "688a09.9s", 0x000000, 0x200000, CRC(fb582963) SHA1(ce8fe6a4d7ac7d7f4b6591f9150b1d351e636354) )
	ROM_LOAD( "688a10.7s", 0x200000, 0x200000, CRC(b3ddc5f1) SHA1(a3f76c86e85eb17f20efb037c1ad64e9cb8566c8) )
	ROM_LOAD( "688a11.5s", 0x400000, 0x200000, CRC(fc706183) SHA1(c8ce6de0588be1023ef48577bc88a4e5effdcd25) )
	ROM_LOAD( "688a12.2s", 0x600000, 0x200000, CRC(510c70e3) SHA1(5af77bc98772ab7961308c3af0a80cb1bca690e3) )

	ROM_REGION(0x800000, "textures", 0) // texture roms
	ROM_LOAD64_WORD( "688a13.18d", 0x000000, 0x200000, CRC(c8f04f91) SHA1(9da8cc3a94dbf0a1fce87c2bc9249df712ae0b0d) )
	ROM_LOAD64_WORD( "688a14.13d", 0x000002, 0x200000, CRC(b9932735) SHA1(2492244d2acb350974202a6718bc7121325d2121) )
	ROM_LOAD64_WORD( "688a15.9d",  0x000004, 0x200000, CRC(8aadee51) SHA1(be9020a47583da9d4ff586d227836dc5b7dc31f0) )
	ROM_LOAD64_WORD( "688a16.4d",  0x000006, 0x200000, CRC(7f4e1893) SHA1(585be7b31ab7a48300c22b00443b00d631f4c49d) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP("93c56.24g", 0x0000, 0x0100, CRC(9564a685) SHA1(ec19f3d6e3a55eac4dab6da5ede7216f002b3186))
ROM_END

ROM_START( gticluba ) // Asia version AAA - Reports: GTI CLUB(TM) System ver 1.00(ASI)
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE("688aaa01.21u", 0x000000, 0x80000, CRC(06a56474) SHA1(3a457b885a35e3ee030fd51d847bcf75fce46208) )
	ROM_LOAD32_BYTE("688aaa02.19u", 0x000001, 0x80000, CRC(3c1e714a) SHA1(557f8542b855b2b35f242c8db7396017aca6dbd8) )
	ROM_LOAD32_BYTE("688aaa03.21r", 0x000002, 0x80000, CRC(e060580b) SHA1(50242f3f3b949cc03082e4e75d9dcc89e17f0a75) )
	ROM_LOAD32_BYTE("688aaa04.19r", 0x000003, 0x80000, CRC(928c23cd) SHA1(cce54398e1e5b98bfb717839cc422f1f60502788) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP("688a05.14u", 0x000000, 0x200000, CRC(7caa3f80) SHA1(28409dc17c4e010173396fdc069a409fbea0d58d) )
	ROM_LOAD32_WORD_SWAP("688a06.12u", 0x000002, 0x200000, CRC(83e7ce0a) SHA1(afe185f6ed700baaf4c8affddc29f8afdfec4423) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68k program
	ROM_LOAD16_WORD_SWAP( "688a07.13k", 0x000000, 0x040000, CRC(f0805f06) SHA1(4b87e02b89e7ea812454498603767668e4619025) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)    // sound roms
	ROM_LOAD( "688a09.9s", 0x000000, 0x200000, CRC(fb582963) SHA1(ce8fe6a4d7ac7d7f4b6591f9150b1d351e636354) )
	ROM_LOAD( "688a10.7s", 0x200000, 0x200000, CRC(b3ddc5f1) SHA1(a3f76c86e85eb17f20efb037c1ad64e9cb8566c8) )
	ROM_LOAD( "688a11.5s", 0x400000, 0x200000, CRC(fc706183) SHA1(c8ce6de0588be1023ef48577bc88a4e5effdcd25) )
	ROM_LOAD( "688a12.2s", 0x600000, 0x200000, CRC(510c70e3) SHA1(5af77bc98772ab7961308c3af0a80cb1bca690e3) )

	ROM_REGION(0x800000, "textures", 0) // texture roms
	ROM_LOAD64_WORD( "688a13.18d", 0x000000, 0x200000, CRC(c8f04f91) SHA1(9da8cc3a94dbf0a1fce87c2bc9249df712ae0b0d) )
	ROM_LOAD64_WORD( "688a14.13d", 0x000002, 0x200000, CRC(b9932735) SHA1(2492244d2acb350974202a6718bc7121325d2121) )
	ROM_LOAD64_WORD( "688a15.9d",  0x000004, 0x200000, CRC(8aadee51) SHA1(be9020a47583da9d4ff586d227836dc5b7dc31f0) )
	ROM_LOAD64_WORD( "688a16.4d",  0x000006, 0x200000, CRC(7f4e1893) SHA1(585be7b31ab7a48300c22b00443b00d631f4c49d) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP("93c56.24g", 0x0000, 0x0100, CRC(9564a685) SHA1(ec19f3d6e3a55eac4dab6da5ede7216f002b3186))
ROM_END

ROM_START( gticlubj ) // Japan version JAA - Reports: GTI CLUB(TM) System ver 1.00(JPN)
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE("688jaa01.21u", 0x000000, 0x80000, CRC(1492059c) SHA1(176dbd87f23f4cd8e1397e67da501738e20e5a57) )
	ROM_LOAD32_BYTE("688jaa02.19u", 0x000001, 0x80000, CRC(7896dd69) SHA1(a3ab7b872132a5e66238e414f4b497cf7beb8b1c) )
	ROM_LOAD32_BYTE("688jaa03.21r", 0x000002, 0x80000, CRC(94e2be50) SHA1(f206ac201903f3aae29196ab6fccdef104859346) )
	ROM_LOAD32_BYTE("688jaa04.19r", 0x000003, 0x80000, CRC(ff539bb6) SHA1(1a225eca4377d82a2b6cb99c1d16580b9ccf2f08) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP("688a05.14u", 0x000000, 0x200000, CRC(7caa3f80) SHA1(28409dc17c4e010173396fdc069a409fbea0d58d) )
	ROM_LOAD32_WORD_SWAP("688a06.12u", 0x000002, 0x200000, CRC(83e7ce0a) SHA1(afe185f6ed700baaf4c8affddc29f8afdfec4423) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68k program
	ROM_LOAD16_WORD_SWAP( "688a07.13k", 0x000000, 0x040000, CRC(f0805f06) SHA1(4b87e02b89e7ea812454498603767668e4619025) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)    // sound roms
	ROM_LOAD( "688a09.9s", 0x000000, 0x200000, CRC(fb582963) SHA1(ce8fe6a4d7ac7d7f4b6591f9150b1d351e636354) )
	ROM_LOAD( "688a10.7s", 0x200000, 0x200000, CRC(b3ddc5f1) SHA1(a3f76c86e85eb17f20efb037c1ad64e9cb8566c8) )
	ROM_LOAD( "688a11.5s", 0x400000, 0x200000, CRC(fc706183) SHA1(c8ce6de0588be1023ef48577bc88a4e5effdcd25) )
	ROM_LOAD( "688a12.2s", 0x600000, 0x200000, CRC(510c70e3) SHA1(5af77bc98772ab7961308c3af0a80cb1bca690e3) )

	ROM_REGION(0x800000, "textures", 0) // texture roms
	ROM_LOAD64_WORD( "688a13.18d", 0x000000, 0x200000, CRC(c8f04f91) SHA1(9da8cc3a94dbf0a1fce87c2bc9249df712ae0b0d) )
	ROM_LOAD64_WORD( "688a14.13d", 0x000002, 0x200000, CRC(b9932735) SHA1(2492244d2acb350974202a6718bc7121325d2121) )
	ROM_LOAD64_WORD( "688a15.9d",  0x000004, 0x200000, CRC(8aadee51) SHA1(be9020a47583da9d4ff586d227836dc5b7dc31f0) )
	ROM_LOAD64_WORD( "688a16.4d",  0x000006, 0x200000, CRC(7f4e1893) SHA1(585be7b31ab7a48300c22b00443b00d631f4c49d) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP("93c56.24g", 0x0000, 0x0100, CRC(9564a685) SHA1(ec19f3d6e3a55eac4dab6da5ede7216f002b3186))
ROM_END

ROM_START( thunderh ) // Euro version EAA
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "680eaa01.21u", 0x000000, 0x080000, CRC(796e2678) SHA1(8051a228aa6d1a3f1fef26de15f4fdb785c2c8ee) )
	ROM_LOAD32_BYTE( "680eaa02.19u", 0x000001, 0x080000, CRC(767e6db0) SHA1(0f29f56fe485f30100ce54e64bda5d5a124c1d09) )
	ROM_LOAD32_BYTE( "680eaa03.21r", 0x000002, 0x080000, CRC(5a5b59b5) SHA1(542c0722437f40829559b09120fde995246d52ae) )
	ROM_LOAD32_BYTE( "680eaa04.19r", 0x000003, 0x080000, CRC(4a973a5c) SHA1(1d84f6416c3b5a85d7ebfbc15fc08e0dd8dc2414) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP( "680a05.14u", 0x000000, 0x200000, CRC(0c9f334d) SHA1(99ac622a04a7140244d81031df69a796b6fd2657) )
	ROM_LOAD32_WORD_SWAP( "680a06.12u", 0x000002, 0x200000, CRC(83074217) SHA1(bbf782ac125cd98d9147ef4e0373bf61f74726f7) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68k program
	ROM_LOAD16_WORD_SWAP( "680a07.13k", 0x000000, 0x080000, CRC(12247a3e) SHA1(846cd9423efd3c9b17fce08393c6c83307d72f92) )

	ROM_REGION(0x20000, "gn680", 0)       // GN680 program
	ROM_LOAD16_WORD_SWAP( "680c22.20k", 0x000000, 0x020000, CRC(d93c0ee2) SHA1(4b58418cbb01b51e12d6e7c86b2c81cd35d86248) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)    // sound roms
	ROM_LOAD( "680a09.9s", 0x000000, 0x200000, CRC(71c2b049) SHA1(ce360172c8774b31edf16a80104c35b1caf26cd9) )
	ROM_LOAD( "680a10.7s", 0x200000, 0x200000, CRC(19882bf3) SHA1(7287da58853c84cbadbfb42bed37f2b0032c4b4d) )
	ROM_LOAD( "680a11.5s", 0x400000, 0x200000, CRC(0c74fe3f) SHA1(2e69f8d37552a74bbda65b134f747b4380ed33b0) )
	ROM_LOAD( "680a12.2s", 0x600000, 0x200000, CRC(b052919d) SHA1(a61c8eaf378ab7d780478db61217302d1b9f8f73) )

	ROM_REGION(0x800000, "textures", 0) // texture roms
	ROM_LOAD64_WORD( "680a13.18d", 0x000000, 0x200000, CRC(233f9074) SHA1(78ce42c35407d61df37cc0d16cdee84f8de968fa) )
	ROM_LOAD64_WORD( "680a14.13d", 0x000002, 0x200000, CRC(9ae15033) SHA1(12e291114629632b81f53811a6c8666aff4e92f3) )
	ROM_LOAD64_WORD( "680a15.9d",  0x000004, 0x200000, CRC(dc47c86f) SHA1(71af9b21f1ecc063135f501b1561869ee910c236) )
	ROM_LOAD64_WORD( "680a16.4d",  0x000006, 0x200000, CRC(ea388143) SHA1(3de5314a009d702186d5e285c8edefdd48139eab) )

	ROM_REGION16_BE(0x100, "eeprom", 0)
	ROM_LOAD16_WORD_SWAP("93c56.24g", 0x0000, 0x0100, CRC(e6524263) SHA1(de6c614fad8049fa6cebe09722cadae656457d69))
ROM_END

ROM_START( thunderhu ) // USA version UAA
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "680uaa01.21u", 0x000000, 0x080000, CRC(f2bb2ba1) SHA1(311e88d63179486014376c4af4ff0ef28673ee5a) )
	ROM_LOAD32_BYTE( "680uaa02.19u", 0x000001, 0x080000, CRC(52f617b5) SHA1(fda3133d3a7e04eb4432c69becdcf1872b3660d9) )
	ROM_LOAD32_BYTE( "680uaa03.21r", 0x000002, 0x080000, CRC(086a0574) SHA1(32fb93dbb93d2fe6af743ea4310b50a6cd03647d) )
	ROM_LOAD32_BYTE( "680uaa04.19r", 0x000003, 0x080000, CRC(85e1f8e3) SHA1(9172c54b6663f1bf390795068271198083a6860d) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP( "680a05.14u", 0x000000, 0x200000, CRC(0c9f334d) SHA1(99ac622a04a7140244d81031df69a796b6fd2657) )
	ROM_LOAD32_WORD_SWAP( "680a06.12u", 0x000002, 0x200000, CRC(83074217) SHA1(bbf782ac125cd98d9147ef4e0373bf61f74726f7) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68k program
	ROM_LOAD16_WORD_SWAP( "680a07.13k", 0x000000, 0x080000, CRC(12247a3e) SHA1(846cd9423efd3c9b17fce08393c6c83307d72f92) )

	ROM_REGION(0x20000, "gn680", 0)       // GN680 program
	ROM_LOAD16_WORD_SWAP( "680c22.20k", 0x000000, 0x020000, CRC(d93c0ee2) SHA1(4b58418cbb01b51e12d6e7c86b2c81cd35d86248) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)    // sound roms
	ROM_LOAD( "680a09.9s", 0x000000, 0x200000, CRC(71c2b049) SHA1(ce360172c8774b31edf16a80104c35b1caf26cd9) )
	ROM_LOAD( "680a10.7s", 0x200000, 0x200000, CRC(19882bf3) SHA1(7287da58853c84cbadbfb42bed37f2b0032c4b4d) )
	ROM_LOAD( "680a11.5s", 0x400000, 0x200000, CRC(0c74fe3f) SHA1(2e69f8d37552a74bbda65b134f747b4380ed33b0) )
	ROM_LOAD( "680a12.2s", 0x600000, 0x200000, CRC(b052919d) SHA1(a61c8eaf378ab7d780478db61217302d1b9f8f73) )

	ROM_REGION(0x800000, "textures", 0) // texture roms
	ROM_LOAD64_WORD( "680a13.18d", 0x000000, 0x200000, CRC(233f9074) SHA1(78ce42c35407d61df37cc0d16cdee84f8de968fa) )
	ROM_LOAD64_WORD( "680a14.13d", 0x000002, 0x200000, CRC(9ae15033) SHA1(12e291114629632b81f53811a6c8666aff4e92f3) )
	ROM_LOAD64_WORD( "680a15.9d",  0x000004, 0x200000, CRC(dc47c86f) SHA1(71af9b21f1ecc063135f501b1561869ee910c236) )
	ROM_LOAD64_WORD( "680a16.4d",  0x000006, 0x200000, CRC(ea388143) SHA1(3de5314a009d702186d5e285c8edefdd48139eab) )

	ROM_REGION16_BE(0x100, "eeprom", 0)
	ROM_LOAD16_WORD_SWAP("93c56.24g", 0x0000, 0x0100, CRC(e6524263) SHA1(de6c614fad8049fa6cebe09722cadae656457d69))
ROM_END

ROM_START( slrasslt ) // USA version UAA
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "792uaa01.21u", 0x000000, 0x080000, CRC(c73bf7fb) SHA1(ffe0fea155473827929339a9261a158287ce30a8) ) // ROM check screen shows version as:  SOLAR ASSAULT DR2  VER UA-A
	ROM_LOAD32_BYTE( "792uaa02.19u", 0x000001, 0x080000, CRC(a940bb9b) SHA1(65a60157697a21cc2485c02c689c9addb3ac91f1) ) // Based on "Revised" code but title screen only shows Solar Assault
	ROM_LOAD32_BYTE( "792uaa03.21r", 0x000002, 0x080000, CRC(363e8411) SHA1(b9c70033d8e3de4b339b61a66172bfecb7c2b3ab) )
	ROM_LOAD32_BYTE( "792uaa04.19r", 0x000003, 0x080000, CRC(7910d99c) SHA1(e2114d369060528998b58331d590c086d306f541) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP( "792a05.14u", 0x000000, 0x200000, CRC(9a27edfc) SHA1(c028b6440eb1b0c814c4db45918e580662ac2d9a) )
	ROM_LOAD32_WORD_SWAP( "792a06.12u", 0x000002, 0x200000, CRC(c272f171) SHA1(df492287eadc5e8668fe46cfa3ed3ca77c57feca) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68k program
	ROM_LOAD16_WORD_SWAP( "792a07.10k", 0x000000, 0x080000, CRC(89a65ad1) SHA1(d814ef0b560c8e68da57ad5c6096e4fc05e9913e) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)    // sound roms
	ROM_LOAD( "792a09.9s", 0x000000, 0x200000, CRC(7d7ea427) SHA1(a9a311a7c17223cc87140fe2890e20a321464831) )
	ROM_LOAD( "792a10.7s", 0x200000, 0x200000, CRC(e585e5d9) SHA1(ec44ad324a66eeea4c45933dda5a8a9a4398879d) )
	ROM_LOAD( "792a11.5s", 0x400000, 0x200000, CRC(c9c3a04c) SHA1(f834659f67712c9fcd93b7407669d7f35517b790) )
	ROM_LOAD( "792a12.2s", 0x600000, 0x200000, CRC(da8fcdd5) SHA1(daa7b3a086ada69e93c3d7cd9130befc79e422dc) )

	ROM_REGION(0x800000, "textures", 0) // texture roms
	ROM_LOAD64_WORD( "792a13.18d", 0x000000, 0x200000, CRC(16d6a134) SHA1(3f53f3c6759d7c5f40aa25a598df899fbac35a60) )
	ROM_LOAD64_WORD( "792a14.13d", 0x000002, 0x200000, CRC(cf57e830) SHA1(607b4dec3b8180a63e29d9dab1ca28d7226dda1e) )
	ROM_LOAD64_WORD( "792a15.9d",  0x000004, 0x200000, CRC(1c5531cb) SHA1(1b514f181c92e16d07bfe4719604f1e4caf15377) )
	ROM_LOAD64_WORD( "792a16.4d",  0x000006, 0x200000, CRC(df89e392) SHA1(af37c5460d43bf8d8a1ab4213c4528083a7363c2) )

	ROM_REGION16_BE(0x100, "eeprom", 0) // default eeprom with magic number
	ROM_LOAD16_WORD( "eeprom-slrasslt.bin", 0x0000, 0x0100, CRC(51eb4d93) SHA1(bc1359daccad80b0e16eb144a0bae715a4fb2e8d) )
ROM_END

ROM_START( slrassltj ) // Japan version JAA
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "792jaa01.21u", 0x000000, 0x080000, CRC(112717c6) SHA1(be5066e1aefef20b6eab2340abc1bdc3d7a5a6e3) ) // ROM check screen shows version as:  SOLAR ASSAULT DR2  VER JA-A
	ROM_LOAD32_BYTE( "792jaa02.19u", 0x000001, 0x080000, CRC(c48582bd) SHA1(194dfd51704ed5eeecb2b56b6bbf651c7cf7701e) ) // Title screen shows Solar Assault Revised
	ROM_LOAD32_BYTE( "792jaa03.21r", 0x000002, 0x080000, CRC(e691009d) SHA1(c8ae58fd280a18151b0e33511269c3685e30fe63) )
	ROM_LOAD32_BYTE( "792jaa04.19r", 0x000003, 0x080000, CRC(1e73a145) SHA1(e519d17d22b5a61570a9bf72ea840f6398928952) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP( "792a05.14u", 0x000000, 0x200000, CRC(9a27edfc) SHA1(c028b6440eb1b0c814c4db45918e580662ac2d9a) )
	ROM_LOAD32_WORD_SWAP( "792a06.12u", 0x000002, 0x200000, CRC(c272f171) SHA1(df492287eadc5e8668fe46cfa3ed3ca77c57feca) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68k program
	ROM_LOAD16_WORD_SWAP( "792a07.10k", 0x000000, 0x080000, CRC(89a65ad1) SHA1(d814ef0b560c8e68da57ad5c6096e4fc05e9913e) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)    // sound roms
	ROM_LOAD( "792a09.9s", 0x000000, 0x200000, CRC(7d7ea427) SHA1(a9a311a7c17223cc87140fe2890e20a321464831) )
	ROM_LOAD( "792a10.7s", 0x200000, 0x200000, CRC(e585e5d9) SHA1(ec44ad324a66eeea4c45933dda5a8a9a4398879d) )
	ROM_LOAD( "792a11.5s", 0x400000, 0x200000, CRC(c9c3a04c) SHA1(f834659f67712c9fcd93b7407669d7f35517b790) )
	ROM_LOAD( "792a12.2s", 0x600000, 0x200000, CRC(da8fcdd5) SHA1(daa7b3a086ada69e93c3d7cd9130befc79e422dc) )

	ROM_REGION(0x800000, "textures", 0) // texture roms
	ROM_LOAD64_WORD( "792a13.18d", 0x000000, 0x200000, CRC(16d6a134) SHA1(3f53f3c6759d7c5f40aa25a598df899fbac35a60) )
	ROM_LOAD64_WORD( "792a14.13d", 0x000002, 0x200000, CRC(cf57e830) SHA1(607b4dec3b8180a63e29d9dab1ca28d7226dda1e) )
	ROM_LOAD64_WORD( "792a15.9d",  0x000004, 0x200000, CRC(1c5531cb) SHA1(1b514f181c92e16d07bfe4719604f1e4caf15377) )
	ROM_LOAD64_WORD( "792a16.4d",  0x000006, 0x200000, CRC(df89e392) SHA1(af37c5460d43bf8d8a1ab4213c4528083a7363c2) )

	ROM_REGION16_BE(0x100, "eeprom", 0) // default eeprom with magic number
	ROM_LOAD16_WORD( "eeprom-slrasslt.bin", 0x0000, 0x0100, CRC(407871d6) SHA1(17a311c412f450edb206750bf7d1055bd16a2135) )
ROM_END

ROM_START( slrassltj1 ) // Japan version JAA
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "672jaa_a01.21u", 0x000000, 0x080000, CRC(e2821f51) SHA1(20c6c2402ba2b564b8f77bcf452abe2d7e023417) ) // ROM check screen shows version as:  SOLAR ASSAULT VER JA-A
	ROM_LOAD32_BYTE( "672jaa_a02.19u", 0x000001, 0x080000, CRC(e3ac7031) SHA1(268588ac6e80463e51a399f53b2396b23faaddba) ) // Title screen shows subtitle "GRADIUS"
	ROM_LOAD32_BYTE( "672jaa_a03.21r", 0x000002, 0x080000, CRC(52711d79) SHA1(8c89fbff9de21cc1e5f17c4ea08870faea648465) )
	ROM_LOAD32_BYTE( "672jaa_a04.19r", 0x000003, 0x080000, CRC(f7419454) SHA1(44cef7f1181cb9c11b013ab0b7e26aa1e95d3746) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP( "672a05.14u", 0x000000, 0x200000, CRC(f6f296e4) SHA1(2ba4ede36f3392aa53a730614272fa80df65281c) )
	ROM_LOAD32_WORD_SWAP( "672a06.12u", 0x000002, 0x200000, CRC(2126227f) SHA1(22615023025453772239b4b21e276fbcbb1cc2bf) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68k program
	ROM_LOAD16_WORD_SWAP( "672a07.10k", 0x000000, 0x080000, CRC(a757309d) SHA1(bc1c8b327815f70bc8bf94adafc8bb0c215e7d1f) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)    // sound roms
	ROM_LOAD( "672a09.9s", 0x000000, 0x200000, CRC(484355ef) SHA1(2af65565d10b058ab95888f36ff28bb7909181d5) )
	ROM_LOAD( "672a10.7s", 0x200000, 0x200000, CRC(1ec1d5d1) SHA1(ebf5417a117f352fff36806a748305270adb70d8) )
	ROM_LOAD( "672a11.5s", 0x400000, 0x200000, CRC(1126753e) SHA1(c7b91e2514329799944e1eb608b5b7e2eb87bea9) )
	ROM_LOAD( "672a12.2s", 0x600000, 0x200000, CRC(432ec7fd) SHA1(a671625400f3837cbe8ddb06dbc37c574d75e281) )

	ROM_REGION(0x800000, "textures", 0) // texture roms
	ROM_LOAD64_WORD( "672a13.18d", 0x000000, 0x200000, CRC(ded2f06d) SHA1(851a896e156fb736bcb7cdfc7db2340bb819c092) )
	ROM_LOAD64_WORD( "672a14.13d", 0x000002, 0x200000, CRC(cd311cfc) SHA1(ead708eaa4e3f1f2b9a17c41ddd0fbc016911527) )
	ROM_LOAD64_WORD( "672a15.9d",  0x000004, 0x200000, CRC(7bb6c271) SHA1(9eb928a52e482b7718c723fa8a14b2c2faaf4425) )
	ROM_LOAD64_WORD( "672a16.4d",  0x000006, 0x200000, CRC(6fa5c0ee) SHA1(ee40179a46f8529fab1c58c05f413a516e7d53ff) )

	ROM_REGION16_BE(0x100, "eeprom", 0) // default eeprom with magic number
	ROM_LOAD16_WORD( "eeprom-slrassltj.bin", 0x0000, 0x0100, CRC(407871d6) SHA1(17a311c412f450edb206750bf7d1055bd16a2135) )
ROM_END

ROM_START( hangplt ) // Japan version JAB
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "685jab01.21u", 0x000000, 0x080000, CRC(f98a3e82) SHA1(94ebaa172b0e98c5cd08efaea5f56e707e5032b4) )
	ROM_LOAD32_BYTE( "685jab02.19u", 0x000001, 0x080000, CRC(20730cdc) SHA1(71b2cf7077ab7db875f9030e21afd05905f57ce5) )
	ROM_LOAD32_BYTE( "685jab03.21r", 0x000002, 0x080000, CRC(77fa2248) SHA1(a662b84945b3d268fed15952cc793d821233735e) )
	ROM_LOAD32_BYTE( "685jab04.19r", 0x000003, 0x080000, CRC(ab6773df) SHA1(91d3f849a1cc5fa4b2fbd876d53402a548198c41) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP( "685a05.14u", 0x000000, 0x200000, CRC(ba1c8f40) SHA1(ce4ed641c1d6d44447eaaada16f305f1d7fb9ee2) )
	ROM_LOAD32_WORD_SWAP( "685a06.12u", 0x000002, 0x200000, CRC(2429935c) SHA1(4da9e169adcac81ea1bc135d727c2bd13ad372fa) )

	ROM_REGION(0x80000, "audiocpu", 0)  // 68k program
	ROM_LOAD16_WORD_SWAP( "685a07.13k", 0x000000, 0x080000, CRC(5b72fd80) SHA1(a150837fa0d66dc0c3832495a4c8ce4f9b92cd98) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)   // sound roms
	ROM_LOAD( "685a09.9s", 0x000000, 0x400000, CRC(b8ae40aa) SHA1(eee27a8929e0e805f1045fd9638e661b36a1e3c7) )
	ROM_LOAD( "685a10.7s", 0x400000, 0x400000, CRC(fef3dc36) SHA1(566c7469fc452b5965a31fa42291082ec8e48a24) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // texture roms
	ROM_LOAD32_WORD( "685a13.4w",  0x000002, 0x400000, CRC(06329af4) SHA1(76cad9db604751ce48bb67bfd29e57bac0ee9a16) )
	ROM_LOAD32_WORD( "685a14.12w", 0x000000, 0x400000, CRC(87437739) SHA1(0d45637af40938a54d5efd29c125b0fafd55f9a4) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD( "hangplt.nv", 0x0000, 0x0100, CRC(30285221) SHA1(a08d06a0d7966f483e4c691a9bd5a98e48294aab) )
ROM_END

ROM_START( hangpltu ) // USA version UAA
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "685uaa01.21u", 0x000000, 0x080000, CRC(83a5b866) SHA1(6859590f212c7debb19924f0174e4cd1bfc011bc) )
	ROM_LOAD32_BYTE( "685uaa02.19u", 0x000001, 0x080000, CRC(765906d6) SHA1(9085a2346756b3b628fef91b7afc131aba434654) )
	ROM_LOAD32_BYTE( "685uaa03.21r", 0x000002, 0x080000, CRC(cb0147a3) SHA1(7bcab760c01ea7e24f4ca5793e081aafa97f68a3) )
	ROM_LOAD32_BYTE( "685uaa04.19r", 0x000003, 0x080000, CRC(a5fda56b) SHA1(4d86f488f411ec16fa3be830206a44214941d1fe) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP( "685a05.14u", 0x000000, 0x200000, CRC(ba1c8f40) SHA1(ce4ed641c1d6d44447eaaada16f305f1d7fb9ee2) )
	ROM_LOAD32_WORD_SWAP( "685a06.12u", 0x000002, 0x200000, CRC(2429935c) SHA1(4da9e169adcac81ea1bc135d727c2bd13ad372fa) )

	ROM_REGION(0x80000, "audiocpu", 0)  // 68k program
	ROM_LOAD16_WORD_SWAP( "685a07.13k", 0x000000, 0x080000, CRC(5b72fd80) SHA1(a150837fa0d66dc0c3832495a4c8ce4f9b92cd98) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)   // sound roms
	ROM_LOAD( "685a09.9s", 0x000000, 0x400000, CRC(b8ae40aa) SHA1(eee27a8929e0e805f1045fd9638e661b36a1e3c7) )
	ROM_LOAD( "685a10.7s", 0x400000, 0x400000, CRC(fef3dc36) SHA1(566c7469fc452b5965a31fa42291082ec8e48a24) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // texture roms
	ROM_LOAD32_WORD( "685a13.4w",  0x000002, 0x400000, CRC(06329af4) SHA1(76cad9db604751ce48bb67bfd29e57bac0ee9a16) )
	ROM_LOAD32_WORD( "685a14.12w", 0x000000, 0x400000, CRC(87437739) SHA1(0d45637af40938a54d5efd29c125b0fafd55f9a4) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD( "hangpltu.nv", 0x0000, 0x0100, CRC(cfea8a2b) SHA1(94162441d401c95b8c6a761d32b656ce22c3eeb6) )
ROM_END


void gticlub_state::init_gticlub()
{
	m_dsp[0]->enable_recompiler();
}

void hangplt_state::init_hangplt_common()
{
	m_cgboard_bank[0]->configure_entries(0, 2, memregion("cgboard_0")->base(), 0x800000);
	m_cgboard_bank[1]->configure_entries(0, 2, memregion("cgboard_0")->base(), 0x800000);
}

void hangplt_state::init_hangplt() //fixme: remove hacks and actually emulate the step lock. Possibly similar to Alpine Racer 1/2 and Alpine Surfer?
{
	init_hangplt_common();

	uint32_t *rom = (uint32_t*)memregion("prgrom")->base();
	rom[(0x153ac^4) / 4] = 0x4e800020;
	rom[(0x15428^4) / 4] = 0x4e800020;
}

void hangplt_state::init_hangpltu()
{
	init_hangplt_common();

	uint32_t *rom = (uint32_t*)memregion("prgrom")->base();
	rom[(0x153d0^4) / 4] = 0x4e800020;
	rom[(0x15428^4) / 4] = 0x4e800020;
}

} // Anonymous namespace


/*************************************************************************/

GAME( 1996, gticlub,    0,        gticlub,  gticlub,  gticlub_state,  init_gticlub,  ROT0, "Konami", "GTI Club: Rally Cote D'Azur (ver EAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NODEVICE_LAN )
GAME( 1996, gticlubu,   gticlub,  gticlub,  gticlub,  gticlub_state,  init_gticlub,  ROT0, "Konami", "GTI Club: Rally Cote D'Azur (ver UAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NODEVICE_LAN )
GAME( 1996, gticluba,   gticlub,  gticlub,  gticlub,  gticlub_state,  init_gticlub,  ROT0, "Konami", "GTI Club: Rally Cote D'Azur (ver AAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NODEVICE_LAN )
GAME( 1996, gticlubj,   gticlub,  gticlub,  gticlub,  gticlub_state,  init_gticlub,  ROT0, "Konami", "GTI Club: Rally Cote D'Azur (ver JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NODEVICE_LAN )
GAME( 1997, thunderh,   0,        thunderh, thunderh, thunderh_state, init_gticlub,  ROT0, "Konami", "Operation Thunder Hurricane (ver EAA)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NODEVICE_LAN )
GAME( 1997, thunderhu,  thunderh, thunderh, thunderh, thunderh_state, init_gticlub,  ROT0, "Konami", "Operation Thunder Hurricane (ver UAA)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NODEVICE_LAN )
GAME( 1997, slrasslt,   0,        slrasslt, slrasslt, gticlub_state,  init_gticlub,  ROT0, "Konami", "Solar Assault (ver UAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // Based on Revised code
GAME( 1997, slrassltj,  slrasslt, slrasslt, slrasslt, gticlub_state,  init_gticlub,  ROT0, "Konami", "Solar Assault Revised (ver JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, slrassltj1, slrasslt, slrasslt, slrasslt, gticlub_state,  init_gticlub,  ROT0, "Konami", "Solar Assault (ver JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAMEL(1997, hangplt,    0,        hangplt,  hangplt,  hangplt_state,  init_hangplt,  ROT0, "Konami", "Hang Pilot (ver JAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND, layout_dualhovu )
GAMEL(1997, hangpltu,   hangplt,  hangplt,  hangplt,  hangplt_state,  init_hangpltu, ROT0, "Konami", "Hang Pilot (ver UAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND, layout_dualhovu )
