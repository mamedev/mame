// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*
    Konami 'ZR107' Hardware
    Konami, 1995-1996

    Driver by Ville Linde

    TODO:
    - segfaults on soft reset;
    - jetwave motors/sensors

    Hardware overview:

    ZR107 CPU board:
    ----------------
        IBM PowerPC 403GA at 32MHz (main CPU)
        Motorola MC68EC000 at 8MHz (sound CPU)
        Konami K056800 (MIRAC), sound system interface
        Konami K056230 (LANC), LAN interface
        Konami K058141 sound chip (same as 2x K054539)

    ZR107 GFX board:
    ----------------
        Analog Devices ADSP-21062 SHARC DSP at 36MHz
        Konami K056832 tilemap chip
        KS10071 (custom 3D pixel unit)
        KS10081 (custom 3D texel unit)

    GN678 GFX board (same as in gticlub.cpp):
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
     DIP42: Unpopulated position for 2Mx8 DIP42 mask ROM
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
      DIP42   : Unpopulated position for 1Mx8 DIP42 mask ROM
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
Check drivers/gticlub.cpp for details on the bottom board.

*/

#include "emu.h"

#include "k001005.h"
#include "k001006.h"
#include "k001604.h"
#include "k054156_k054157_k056832.h"
#include "konami_helper.h"
#include "konppc.h"

#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/sharc/sharc.h"
#include "machine/adc083x.h"
#include "machine/eepromser.h"
#include "machine/k056230.h"
#include "machine/watchdog.h"
#include "sound/k054539.h"
#include "sound/k056800.h"

#include "emupal.h"
#include "speaker.h"

#define LOG_SYSREG (1 << 1)

#define LOG_ALL (LOG_SYSREG)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGSYSREG(...) LOGMASKED(LOG_SYSREG, __VA_ARGS__)

namespace {

class zr107_state : public driver_device
{
public:
	zr107_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dsp(*this, "dsp"),
		m_watchdog(*this, "watchdog"),
		m_k056230(*this, "k056230"),
		m_k056800(*this, "k056800"),
		m_workram(*this, "workram"),
		m_k001005(*this, "k001005"),
		m_k001006_1(*this, "k001006_1"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_sharc_dataram(*this, "sharc_dataram"),
		m_konppc(*this, "konppc"),
		m_in(*this, "IN%u", 0U),
		m_out4(*this, "OUT4"),
		m_eepromout(*this, "EEPROMOUT"),
		m_analog(*this, "ANALOG%u", 1U),
		m_pcb_digit(*this, "pcbdigit%u", 0U)
	{ }

	void zr107(machine_config &config);

	void driver_init();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<ppc_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<adsp21062_device> m_dsp;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<k056230_device> m_k056230;
	required_device<k056800_device> m_k056800;
	required_shared_ptr<uint32_t> m_workram;
	required_device<k001005_device> m_k001005;
	required_device<k001006_device> m_k001006_1;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint32_t> m_sharc_dataram;
	required_device<konppc_device> m_konppc;
	required_ioport_array<5> m_in;
	required_ioport m_out4, m_eepromout;
	optional_ioport_array<3> m_analog;
	output_finder<2> m_pcb_digit;

	int32_t m_ccu_vcth = 0;
	int32_t m_ccu_vctl = 0;
	uint8_t m_sound_ctrl = 0;
	uint8_t m_sound_intck = 0;

	uint8_t sysreg_r(offs_t offset);
	void sysreg_w(offs_t offset, uint8_t data);
	uint32_t ccu_r(offs_t offset, uint32_t mem_mask = ~0);
	void ccu_w(uint32_t data);
	void sound_ctrl_w(uint8_t data);

	void vblank(int state);
	void k054539_irq_gen(int state);
	double adc0838_callback(uint8_t input);

	void sharc_memmap(address_map &map) ATTR_COLD;
	void sound_memmap(address_map &map) ATTR_COLD;
};

class midnrun_state : public zr107_state
{
public:
	midnrun_state(const machine_config &mconfig, device_type type, const char *tag) :
		zr107_state(mconfig, type, tag),
		m_k056832(*this, "k056832")
	{ }

	void midnrun(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void main_memmap(address_map &map) ATTR_COLD;

	K056832_CB_MEMBER(tile_callback);

	required_device<k056832_device> m_k056832;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

class jetwave_state : public zr107_state
{
public:
	jetwave_state(const machine_config &mconfig, device_type type, const char *tag) :
		zr107_state(mconfig, type, tag),
		m_k001604(*this, "k001604"),
		m_k001006_2(*this, "k001006_2")
	{ }

	void jetwave(machine_config &config);

private:
	void main_memmap(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<k001604_device> m_k001604;
	required_device<k001006_device> m_k001006_2;
};

uint32_t jetwave_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->pen(0), cliprect);

	m_k001604->draw_back_layer(screen, bitmap, cliprect);
	m_k001005->draw(bitmap, cliprect);
	m_k001604->draw_front_layer(screen, bitmap, cliprect);

	return 0;
}


/*****************************************************************************/

K056832_CB_MEMBER(midnrun_state::tile_callback)
{
	*color += layer * 0x40;
}

void midnrun_state::video_start()
{
	m_k056832->set_layer_offs(0, -29, -27);
	m_k056832->set_layer_offs(1, -25, -27);
	m_k056832->set_layer_offs(2, -29, -27);
	m_k056832->set_layer_offs(3, -29, -27);
	m_k056832->set_layer_offs(4, -29, -27);
	m_k056832->set_layer_offs(5, -29, -27);
	m_k056832->set_layer_offs(6, -29, -27);
	m_k056832->set_layer_offs(7, -29, -27);
}

uint32_t midnrun_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->pen(0), cliprect);

	m_k056832->tilemap_draw_dj(screen, bitmap, cliprect, 1, 0, 0);

	if (m_konppc->output_3d_enabled())
	{
		m_k001005->draw(bitmap, cliprect);
	}

	m_k056832->tilemap_draw_dj(screen, bitmap, cliprect, 0, 0, 0);

	return 0;
}

/******************************************************************/

uint8_t zr107_state::sysreg_r(offs_t offset)
{
	uint32_t r = 0;

	switch (offset)
	{
		case 0: // I/O port 0
			r = m_in[0]->read();
			break;
		case 1: // I/O port 1
			r = m_in[1]->read();
			break;
		case 2: // I/O port 2
			r = m_in[2]->read();
			break;
		case 3: // System Port 0
			r = m_in[3]->read();
			break;
		case 4: // System Port 1
			r = m_in[4]->read();
			break;
		case 5: // Parallel data port
			break;
		default:
			break;
	}
	return r;
}

void zr107_state::sysreg_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: // 7seg LEDs on PCB
		case 1:
			m_pcb_digit[offset] = bitswap<7>(~data,0,1,2,3,4,5,6);
			break;

		case 2: // Parallel data register
			LOGSYSREG("Parallel data = %02X\n", data);
			break;

		case 3: // System Register 0
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
			m_eepromout->write(data & 0x07, 0xff);
			m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
			LOGSYSREG("System register 0 = %02X\n", data);
			break;

		case 4: // System Register 1
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
			if (BIT(data, 7))    // CG Board 1 IRQ Ack
				m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
			if (BIT(data, 6))    // CG Board 0 IRQ Ack
				m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
			m_konppc->set_cgboard_id((data >> 4) & 3);
			m_out4->write(data, 0xff);
			LOGSYSREG("System register 1 = %02X\n", data);
			break;

		case 5: // System Register 2
			/*
			    0x01 = AFE
			*/
			m_watchdog->reset_line_w(BIT(data, 0));
			break;

		default:
			break;
	}
}

uint32_t zr107_state::ccu_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t r = 0;
	switch (offset)
	{
		case 0x1c/4:
		{
			// Midnight Run polls the vertical counter in vblank
			if (ACCESSING_BITS_24_31)
			{
				int32_t const vcth = m_ccu_vcth ^ 0xff;
				if (!machine().side_effects_disabled())
					m_ccu_vcth = vcth;
				r |= vcth << 24;
			}
			if (ACCESSING_BITS_8_15)
			{
				int32_t const vctl = (m_ccu_vctl + 1) & 0x1ff;
				if (!machine().side_effects_disabled())
					m_ccu_vctl = vctl;
				r |= (vctl >> 2) << 8;
			}
		}
	}

	return r;
}

void zr107_state::ccu_w(uint32_t data)
{
}

/******************************************************************/

void zr107_state::machine_start()
{
	m_pcb_digit.resolve();

	// set conservative DRC options
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	// configure fast RAM regions for DRC
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x000fffff, false, m_workram);

	m_ccu_vcth = m_ccu_vctl = 0;

	save_item(NAME(m_ccu_vcth));
	save_item(NAME(m_ccu_vctl));
	save_item(NAME(m_sound_ctrl));
	save_item(NAME(m_sound_intck));
}

void midnrun_state::main_memmap(address_map &map)
{
	map(0x00000000, 0x000fffff).ram().share(m_workram);
	map(0x74000000, 0x74001fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w)).mirror(0x2000);
	map(0x74020000, 0x7402003f).rw(m_k056832, FUNC(k056832_device::word_r), FUNC(k056832_device::word_w));
	map(0x74060000, 0x7406003f).rw(FUNC(midnrun_state::ccu_r), FUNC(midnrun_state::ccu_w));
	map(0x74080000, 0x74081fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x740a0000, 0x740a3fff).r(m_k056832, FUNC(k056832_device::rom_word_r));
	map(0x78000000, 0x7800ffff).rw(m_konppc, FUNC(konppc_device::cgboard_dsp_shared_r_ppc), FUNC(konppc_device::cgboard_dsp_shared_w_ppc)); // 21N 21K 23N 23K
	map(0x78010000, 0x7801ffff).w(m_konppc, FUNC(konppc_device::cgboard_dsp_shared_w_ppc));
	map(0x78040000, 0x7804000f).rw(m_k001006_1, FUNC(k001006_device::read), FUNC(k001006_device::write));
	map(0x780c0000, 0x780c0007).rw(m_konppc, FUNC(konppc_device::cgboard_dsp_comm_r_ppc), FUNC(konppc_device::cgboard_dsp_comm_w_ppc));
	map(0x7e000000, 0x7e003fff).rw(FUNC(midnrun_state::sysreg_r), FUNC(midnrun_state::sysreg_w));
	map(0x7e008000, 0x7e009fff).m(m_k056230, FUNC(k056230_device::regs_map));  // LANC registers
	map(0x7e00a000, 0x7e00bfff).rw(m_k056230, FUNC(k056230_device::ram_r), FUNC(k056230_device::ram_w)); // LANC Buffer RAM (27E)
	map(0x7e00c000, 0x7e00c00f).rw(m_k056800, FUNC(k056800_device::host_r), FUNC(k056800_device::host_w));
	map(0x7f800000, 0x7f9fffff).rom().region("prgrom", 0);
	map(0x7fe00000, 0x7fffffff).rom().region("prgrom", 0);
}


void jetwave_state::main_memmap(address_map &map)
{
	map(0x00000000, 0x000fffff).ram().share(m_workram);
	map(0x74000000, 0x740000ff).rw(m_k001604, FUNC(k001604_device::reg_r), FUNC(k001604_device::reg_w));
	map(0x74010000, 0x7401ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x74020000, 0x7403ffff).rw(m_k001604, FUNC(k001604_device::tile_r), FUNC(k001604_device::tile_w));
	map(0x74040000, 0x7407ffff).rw(m_k001604, FUNC(k001604_device::char_r), FUNC(k001604_device::char_w));
	map(0x78000000, 0x7800ffff).rw(m_konppc, FUNC(konppc_device::cgboard_dsp_shared_r_ppc), FUNC(konppc_device::cgboard_dsp_shared_w_ppc)); // 21N 21K 23N 23K
	map(0x78010000, 0x7801ffff).w(m_konppc, FUNC(konppc_device::cgboard_dsp_shared_w_ppc));
	map(0x78040000, 0x7804000f).rw(m_k001006_1, FUNC(k001006_device::read), FUNC(k001006_device::write));
	map(0x78080000, 0x7808000f).rw(m_k001006_2, FUNC(k001006_device::read), FUNC(k001006_device::write));
	map(0x780c0000, 0x780c0007).rw(m_konppc, FUNC(konppc_device::cgboard_dsp_comm_r_ppc), FUNC(konppc_device::cgboard_dsp_comm_w_ppc));
	map(0x7e000000, 0x7e003fff).rw(FUNC(jetwave_state::sysreg_r), FUNC(jetwave_state::sysreg_w));
	map(0x7e008000, 0x7e009fff).m(m_k056230, FUNC(k056230_device::regs_map));  // LANC registers
	map(0x7e00a000, 0x7e00bfff).rw(m_k056230, FUNC(k056230_device::ram_r), FUNC(k056230_device::ram_w)); // LANC Buffer RAM (27E)
	map(0x7e00c000, 0x7e00c00f).rw(m_k056800, FUNC(k056800_device::host_r), FUNC(k056800_device::host_w));
	map(0x7f000000, 0x7f3fffff).rom().region("datarom", 0);
	map(0x7f800000, 0x7f9fffff).rom().region("prgrom", 0);
	map(0x7fe00000, 0x7fffffff).rom().region("prgrom", 0);
}



/**********************************************************************/

void zr107_state::sound_ctrl_w(uint8_t data)
{
	if (BIT(~data, 0))
		m_audiocpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);

	m_sound_ctrl = data;
}

void zr107_state::sound_memmap(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x100000, 0x103fff).ram(); // Work RAM
	map(0x200000, 0x2004ff).rw("k054539_1", FUNC(k054539_device::read), FUNC(k054539_device::write)).umask16(0xff00);
	map(0x200000, 0x2004ff).rw("k054539_2", FUNC(k054539_device::read), FUNC(k054539_device::write)).umask16(0x00ff);
	map(0x400000, 0x40001f).rw(m_k056800, FUNC(k056800_device::sound_r), FUNC(k056800_device::sound_w)).umask16(0x00ff);
	map(0x500001, 0x500001).w(FUNC(zr107_state::sound_ctrl_w));
	map(0x580000, 0x580001).nopw(); // 'NRES' - D2: K056602 /RESET
}

/*****************************************************************************/

void zr107_state::sharc_memmap(address_map &map)
{
	map(0x400000, 0x41ffff).rw(m_konppc, FUNC(konppc_device::cgboard_shared_sharc_r<0>), FUNC(konppc_device::cgboard_shared_sharc_w<0>));
	map(0x500000, 0x5fffff).ram().share(m_sharc_dataram).lr32(NAME([this](offs_t offset) { return m_sharc_dataram[offset] & 0xffff; }));
	map(0x600000, 0x6fffff).rw(m_k001005, FUNC(k001005_device::read), FUNC(k001005_device::write));
	map(0x700000, 0x7000ff).rw(m_konppc, FUNC(konppc_device::cgboard_comm_sharc_r<0>), FUNC(konppc_device::cgboard_comm_sharc_w<0>));
}

/*****************************************************************************/


static INPUT_PORTS_START( zr107 )
	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc0838", FUNC(adc083x_device::do_read))

	PORT_START("IN4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) // PARAACK
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc0838", FUNC(adc083x_device::sars_read))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	PORT_START("EEPROMOUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))

	PORT_START("OUT4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("adc0838", FUNC(adc083x_device::cs_write))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("adc0838", FUNC(adc083x_device::di_write))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("adc0838", FUNC(adc083x_device::clk_write))
INPUT_PORTS_END

static INPUT_PORTS_START( midnrun )
	PORT_INCLUDE( zr107 )

	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )        PORT_NAME("Start/View")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )   PORT_NAME("Shift Up")   PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Shift Down") PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Auto Shift") PORT_4WAY PORT_TOGGLE PORT_CONDITION("IN3", 0x02, EQUALS, 0x02)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )       PORT_NAME("AT/MT Switch")                     PORT_CONDITION("IN3", 0x02, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0b, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0c, 0x0c, "Network ID" ) PORT_DIPLOCATION("SW:4,3")
	PORT_DIPSETTING( 0x0c, "1" )
	PORT_DIPSETTING( 0x08, "2" )
	PORT_DIPSETTING( 0x04, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0x02, 0x00, "Transmission Type" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x02, "'T'Gate" )
	PORT_DIPSETTING( 0x00, "Button" )
	PORT_DIPNAME( 0x01, 0x01, "CG Board Type" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x01, "Single" )
	PORT_DIPSETTING( 0x00, "Twin" ) // unused

	PORT_START("ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_NAME("Steering Wheel") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(16)

	PORT_START("ANALOG2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_NAME("Gas Pedal") PORT_MINMAX(0x00,0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(16)

	PORT_START("ANALOG3")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_NAME("Brake Pedal") PORT_MINMAX(0x00,0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(16)
INPUT_PORTS_END

static INPUT_PORTS_START( jetwave )
	PORT_INCLUDE( zr107 )

	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start/View")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CONDITION("IN0", 0x18, NOTEQUALS, 0x18) // T-Center
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Angle")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Tilt Left") PORT_PLAYER(2) PORT_CONDITION("IN3", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Tilt Right") PORT_PLAYER(2) PORT_CONDITION("IN3", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// digital controls when in dev mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Accelerator (Dev)") PORT_CONDITION("IN3", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Handle Left (Dev)") PORT_CONDITION("IN3", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Handle Right (Dev)") PORT_CONDITION("IN3", 0x01, EQUALS, 0x00)

	PORT_START("IN3")
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0c, 0x0c, "Network ID" ) PORT_DIPLOCATION("SW:4,3")
	PORT_DIPSETTING( 0x0c, "1" )
	PORT_DIPSETTING( 0x08, "2" )
	PORT_DIPSETTING( 0x04, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0x02, 0x00, "Drive System" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) ) // Disables and bypasses all sensor checks. This disables the force feedback on actual hardware.
	PORT_DIPSETTING( 0x02, DEF_STR( On ) ) // Enables the sensors/normal use.
	PORT_DIPNAME( 0x01, 0x01, "Running Mode" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x01, "Production" ) // Enables the analog inputs; normal usage
	PORT_DIPSETTING( 0x00, "Developer" ) // Disables them for use with a JAMMA interface; intended for development purposes.

	PORT_START("ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_NAME("Steering Handle") PORT_MINMAX(0x70,0x90) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_REVERSE

	PORT_START("ANALOG2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_NAME("Accelerator") PORT_MINMAX(0x00,0x20) PORT_SENSITIVITY(25) PORT_KEYDELTA(4)
INPUT_PORTS_END


// ADC0838 Interface

double zr107_state::adc0838_callback(uint8_t input)
{
	switch (input)
	{
	case ADC083X_CH0:
		return (double)(5 * m_analog[0].read_safe(0)) / 255.0;
	case ADC083X_CH1:
		return (double)(5 * m_analog[1].read_safe(0)) / 255.0;
	case ADC083X_CH2:
		return (double)(5 * m_analog[2].read_safe(0)) / 255.0;
	case ADC083X_VREF:
		return 5.0;
	case ADC083X_AGND: default:
		return 0.0;
	}
}



void zr107_state::k054539_irq_gen(int state)
{
	if (m_sound_ctrl & 1)
	{
		// Trigger an interrupt on the rising edge
		if (!m_sound_intck && state)
			m_audiocpu->set_input_line(M68K_IRQ_2, ASSERT_LINE);
	}

	m_sound_intck = state;
}


/* PowerPC interrupts

    IRQ0:  Vblank
    IRQ2:  LANC
    DMA0

*/
void zr107_state::vblank(int state)
{
	if (state)
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		m_dsp->set_flag_input(1, ASSERT_LINE);
	}
}

void zr107_state::machine_reset()
{
	m_dsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void zr107_state::zr107(machine_config &config)
{
	// basic machine hardware
	PPC403GA(config, m_maincpu, XTAL(64'000'000)/2); // PowerPC 403GA 32MHz

	M68000(config, m_audiocpu, XTAL(64'000'000)/8); // 8MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &zr107_state::sound_memmap);

	ADSP21062(config, m_dsp, XTAL(36'000'000));
	m_dsp->set_boot_mode(adsp21062_device::BOOT_MODE_EPROM);
	m_dsp->set_addrmap(AS_DATA, &zr107_state::sharc_memmap);

	EEPROM_93C46_16BIT(config, "eeprom");

	K056230(config, m_k056230);
	m_k056230->irq_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ2);

	WATCHDOG_TIMER(config, m_watchdog);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(64*8, 48*8);
	m_screen->set_visarea(0*8, 64*8-1, 0*8, 48*8-1);
	m_screen->screen_vblank().set(FUNC(zr107_state::vblank));

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 4096);

	K001005(config, m_k001005, 0, m_k001006_1);

	K001006(config, m_k001006_1, 0);
	m_k001006_1->set_gfx_region("textures");

	K056800(config, m_k056800, XTAL(18'432'000));
	m_k056800->int_callback().set_inputline(m_audiocpu, M68K_IRQ_1);

	SPEAKER(config, "speaker", 2).front();

	k054539_device &k054539_1(K054539(config, "k054539_1", XTAL(18'432'000)));
	k054539_1.set_device_rom_tag("k054539");
	k054539_1.timer_handler().set(FUNC(zr107_state::k054539_irq_gen));
	k054539_1.add_route(0, "speaker", 0.75, 0);
	k054539_1.add_route(1, "speaker", 0.75, 1);

	k054539_device &k054539_2(K054539(config, "k054539_2", XTAL(18'432'000)));
	k054539_2.set_device_rom_tag("k054539");
	k054539_2.add_route(0, "speaker", 0.75, 0);
	k054539_2.add_route(1, "speaker", 0.75, 1);

	adc0838_device &adc(ADC0838(config, "adc0838"));
	adc.set_input_callback(FUNC(zr107_state::adc0838_callback));

	KONPPC(config, m_konppc, 0);
	m_konppc->set_dsp_tag(0, m_dsp);
	m_konppc->set_num_boards(1);
	m_konppc->set_cgboard_type(konppc_device::CGBOARD_TYPE_ZR107);
}

void midnrun_state::midnrun(machine_config &config)
{
	zr107(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &midnrun_state::main_memmap);

	config.set_maximum_quantum(attotime::from_hz(15000));

	// video hardware
	m_screen->set_screen_update(FUNC(midnrun_state::screen_update));

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(midnrun_state::tile_callback));
	m_k056832->set_config(K056832_BPP_8, 1, 0);
	m_k056832->set_palette(m_palette);
}

void jetwave_state::jetwave(machine_config &config)
{
	zr107(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jetwave_state::main_memmap);

	config.set_maximum_quantum(attotime::from_hz(15000));

	// video hardware
	m_screen->set_size(1024, 1024);
	m_screen->set_visarea(40, 511 + 40, 27, 383 + 27); // needs CRTC emulation
	m_screen->set_screen_update(FUNC(jetwave_state::screen_update));

	m_palette->set_format(4, raw_to_rgb_converter::standard_rgb_decoder<5,5,5, 10,5,0>, 16384);

	K001604(config, m_k001604, 0);
	m_k001604->set_palette(m_palette);

	// The second K001006 chip connects to the second K001005 chip.
	// Hook this up when the K001005 separation is understood (seems the load balancing is done on hardware).
	K001006(config, m_k001006_2, 0);
	m_k001006_2->set_gfx_region("textures");

	m_konppc->set_cgboard_type(konppc_device::CGBOARD_TYPE_GTICLUB);
}

/*****************************************************************************/

void zr107_state::driver_init()
{
	m_dsp->enable_recompiler();
}

/*****************************************************************************/

ROM_START( midnrun )
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "476ea1a01.20u", 0x000000, 0x80000, CRC(ea70edf2) SHA1(51c882383a150ba118ccd39eb869525fcf5eee3c) ) // Program version EAA, v1.11 (EUR)
	ROM_LOAD32_BYTE( "476ea1a02.17u", 0x000001, 0x80000, CRC(1462994f) SHA1(c8614c6c416f81737cc77c46eea6d8d440bc8cf3) )
	ROM_LOAD32_BYTE( "476ea1a03.15u", 0x000002, 0x80000, CRC(b770ae46) SHA1(c61daa8353802957eb1c2e2c6204c3a98569627e) )
	ROM_LOAD32_BYTE( "476ea1a04.13u", 0x000003, 0x80000, CRC(9644b277) SHA1(b9cb812b6035dfd93032d277c8aa0037cf6b3dbe) )

	ROM_REGION(0x20000, "audiocpu", 0)      // M68K program
	ROM_LOAD16_WORD_SWAP( "477a07.19l", 0x000000, 0x20000, CRC(a82c0ba1) SHA1(dad69f2e5e75009d70cc2748477248ec47627c30) )

	ROM_REGION(0x100000, "k056832", 0) // Tilemap
	ROM_LOAD16_BYTE( "477a11.35b", 0x000000, 0x80000, CRC(85eef04b) SHA1(02e26d2d4a8b29894370f28d2a49fdf5c7d23f95) )
	ROM_LOAD16_BYTE( "477a12.35a", 0x000001, 0x80000, CRC(451d7777) SHA1(0bf280ca475100778bbfd3f023547bf0413fc8b7) )

	ROM_REGION(0x800000, "textures", 0) // Texture data
	ROM_LOAD64_WORD( "477a13.9h", 0x000000, 0x200000, CRC(b1ee901d) SHA1(b1432cb1379b35d99d3f2b7f6409db6f7e88121d) )
	ROM_LOAD64_WORD( "477a14.7h", 0x000002, 0x200000, CRC(9ffa8cc5) SHA1(eaa19e26df721bec281444ca1c5ccc9e48df1b0b) )
	ROM_LOAD64_WORD( "477a15.5h", 0x000004, 0x200000, CRC(e337fce7) SHA1(c84875f3275efd47273508b340231721f5a631d2) )
	ROM_LOAD64_WORD( "477a16.2h", 0x000006, 0x200000, CRC(2c03ee63) SHA1(6b74d340dddf92bb4e4b1e037f003d58c65d8d9b) )

	ROM_REGION(0x600000, "k054539", 0)   // Sound data
	ROM_LOAD( "477a08.5r", 0x000000, 0x200000, CRC(d320dbde) SHA1(eb602cad6ac7c7151c9f29d39b10041d5a354164) )
	ROM_LOAD( "477a09.3r", 0x200000, 0x200000, CRC(f431e29f) SHA1(e6082d88f86abb63d02ac34e70873b58f88b0ddc) )
	ROM_LOAD( "477a10.5n", 0x400000, 0x200000, CRC(8db31bd4) SHA1(d662d3bb6e8b44a01ffa158f5d7425454aad49a3) )
ROM_END

ROM_START( midnrunj )
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "476ja1d01.20u", 0x000000, 0x80000, CRC(68d05950) SHA1(d0ff9b9b628563e18a3eaa7b96b7e9e442c001a9) ) // Program version JAD, v1.10 (JPN)
	ROM_LOAD32_BYTE( "476ja1d02.17u", 0x000001, 0x80000, CRC(b12a14be) SHA1(d65281791874b90351442b94173d96582cfacd10) )
	ROM_LOAD32_BYTE( "476ja1d03.15u", 0x000002, 0x80000, CRC(f768c8f1) SHA1(b8242995bdb4f9ac078fd59ffc70c31014396c92) )
	ROM_LOAD32_BYTE( "476ja1d04.13u", 0x000003, 0x80000, CRC(6fd4fce7) SHA1(0ef25ec98a13f7beca1231db5a4db9004caadb0b) )

	ROM_REGION(0x20000, "audiocpu", 0)      // M68K program
	ROM_LOAD16_WORD_SWAP( "477a07.19l", 0x000000, 0x20000, CRC(a82c0ba1) SHA1(dad69f2e5e75009d70cc2748477248ec47627c30) )

	ROM_REGION(0x100000, "k056832", 0) // Tilemap
	ROM_LOAD16_BYTE( "477a11.35b", 0x000000, 0x80000, CRC(85eef04b) SHA1(02e26d2d4a8b29894370f28d2a49fdf5c7d23f95) )
	ROM_LOAD16_BYTE( "477a12.35a", 0x000001, 0x80000, CRC(451d7777) SHA1(0bf280ca475100778bbfd3f023547bf0413fc8b7) )

	ROM_REGION(0x800000, "textures", 0) // Texture data
	ROM_LOAD64_WORD( "477a13.9h", 0x000000, 0x200000, CRC(b1ee901d) SHA1(b1432cb1379b35d99d3f2b7f6409db6f7e88121d) )
	ROM_LOAD64_WORD( "477a14.7h", 0x000002, 0x200000, CRC(9ffa8cc5) SHA1(eaa19e26df721bec281444ca1c5ccc9e48df1b0b) )
	ROM_LOAD64_WORD( "477a15.5h", 0x000004, 0x200000, CRC(e337fce7) SHA1(c84875f3275efd47273508b340231721f5a631d2) )
	ROM_LOAD64_WORD( "477a16.2h", 0x000006, 0x200000, CRC(2c03ee63) SHA1(6b74d340dddf92bb4e4b1e037f003d58c65d8d9b) )

	ROM_REGION(0x600000, "k054539", 0)   // Sound data
	ROM_LOAD( "477a08.5r", 0x000000, 0x200000, CRC(d320dbde) SHA1(eb602cad6ac7c7151c9f29d39b10041d5a354164) )
	ROM_LOAD( "477a09.3r", 0x200000, 0x200000, CRC(f431e29f) SHA1(e6082d88f86abb63d02ac34e70873b58f88b0ddc) )
	ROM_LOAD( "477a10.5n", 0x400000, 0x200000, CRC(8db31bd4) SHA1(d662d3bb6e8b44a01ffa158f5d7425454aad49a3) )
ROM_END

ROM_START( midnruna )
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "477aaa01.20u", 0x000000, 0x80000, CRC(3aa31517) SHA1(315d9c3c930493e39bc497ceafa0c4ef6fa64e4d) ) // Program version AAA, v1.10 (ASA)
	ROM_LOAD32_BYTE( "477aaa02.17u", 0x000001, 0x80000, CRC(c506bd3d) SHA1(d44ed2cb39f0da44f681190132c7603dfca813d9) )
	ROM_LOAD32_BYTE( "477aaa03.15u", 0x000002, 0x80000, CRC(53f8e898) SHA1(ba83a60a411bb307cb0e424099716ccf888a4f39) )
	ROM_LOAD32_BYTE( "477aaa04.13u", 0x000003, 0x80000, CRC(0eb264b7) SHA1(179a3d58c0f554fd1b283ee3640ce09d5142b288) )

	ROM_REGION(0x20000, "audiocpu", 0)      // M68K program
	ROM_LOAD16_WORD_SWAP( "477a07.19l", 0x000000, 0x20000, CRC(a82c0ba1) SHA1(dad69f2e5e75009d70cc2748477248ec47627c30) )

	ROM_REGION(0x100000, "k056832", 0) // Tilemap
	ROM_LOAD16_BYTE( "477a11.35b", 0x000000, 0x80000, CRC(85eef04b) SHA1(02e26d2d4a8b29894370f28d2a49fdf5c7d23f95) )
	ROM_LOAD16_BYTE( "477a12.35a", 0x000001, 0x80000, CRC(451d7777) SHA1(0bf280ca475100778bbfd3f023547bf0413fc8b7) )

	ROM_REGION(0x800000, "textures", 0) // Texture data
	ROM_LOAD64_WORD( "477a13.9h", 0x000000, 0x200000, CRC(b1ee901d) SHA1(b1432cb1379b35d99d3f2b7f6409db6f7e88121d) )
	ROM_LOAD64_WORD( "477a14.7h", 0x000002, 0x200000, CRC(9ffa8cc5) SHA1(eaa19e26df721bec281444ca1c5ccc9e48df1b0b) )
	ROM_LOAD64_WORD( "477a15.5h", 0x000004, 0x200000, CRC(e337fce7) SHA1(c84875f3275efd47273508b340231721f5a631d2) )
	ROM_LOAD64_WORD( "477a16.2h", 0x000006, 0x200000, CRC(2c03ee63) SHA1(6b74d340dddf92bb4e4b1e037f003d58c65d8d9b) )

	ROM_REGION(0x600000, "k054539", 0)   // Sound data
	ROM_LOAD( "477a08.5r", 0x000000, 0x200000, CRC(d320dbde) SHA1(eb602cad6ac7c7151c9f29d39b10041d5a354164) )
	ROM_LOAD( "477a09.3r", 0x200000, 0x200000, CRC(f431e29f) SHA1(e6082d88f86abb63d02ac34e70873b58f88b0ddc) )
	ROM_LOAD( "477a10.5n", 0x400000, 0x200000, CRC(8db31bd4) SHA1(d662d3bb6e8b44a01ffa158f5d7425454aad49a3) )
ROM_END

ROM_START( midnruna2 )
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "477ab1d01.20u", 0x000000, 0x80000, CRC(3aa31517) SHA1(315d9c3c930493e39bc497ceafa0c4ef6fa64e4d) ) // labeled AB1, but still program version AAA, v1.10 (ASA)
	ROM_LOAD32_BYTE( "477ab1d02.17u", 0x000001, 0x80000, CRC(c506bd3d) SHA1(d44ed2cb39f0da44f681190132c7603dfca813d9) )
	ROM_LOAD32_BYTE( "477ab1d03.15u", 0x000002, 0x80000, CRC(53f8e898) SHA1(ba83a60a411bb307cb0e424099716ccf888a4f39) )
	ROM_LOAD32_BYTE( "477ab1d04.13u", 0x000003, 0x80000, CRC(0eb264b7) SHA1(179a3d58c0f554fd1b283ee3640ce09d5142b288) )

	ROM_REGION(0x20000, "audiocpu", 0)      // M68K program
	ROM_LOAD16_WORD_SWAP( "477b07.19l", 0x000000, 0x20000, CRC(2d00cf76) SHA1(152bed061c59e29864d735f8beba2a49136f7212) )

	ROM_REGION(0x100000, "k056832", 0) // Tilemap
	ROM_LOAD16_BYTE( "477a11.35b", 0x000000, 0x80000, CRC(85eef04b) SHA1(02e26d2d4a8b29894370f28d2a49fdf5c7d23f95) )
	ROM_LOAD16_BYTE( "477a12.35a", 0x000001, 0x80000, CRC(451d7777) SHA1(0bf280ca475100778bbfd3f023547bf0413fc8b7) )

	ROM_REGION(0x800000, "textures", 0) // Texture data
	ROM_LOAD64_WORD( "477a13.9h", 0x000000, 0x200000, CRC(b1ee901d) SHA1(b1432cb1379b35d99d3f2b7f6409db6f7e88121d) )
	ROM_LOAD64_WORD( "477a14.7h", 0x000002, 0x200000, CRC(9ffa8cc5) SHA1(eaa19e26df721bec281444ca1c5ccc9e48df1b0b) )
	ROM_LOAD64_WORD( "477a15.5h", 0x000004, 0x200000, CRC(e337fce7) SHA1(c84875f3275efd47273508b340231721f5a631d2) )
	ROM_LOAD64_WORD( "477a16.2h", 0x000006, 0x200000, CRC(2c03ee63) SHA1(6b74d340dddf92bb4e4b1e037f003d58c65d8d9b) )

	ROM_REGION(0x600000, "k054539", 0)   // Sound data
	ROM_LOAD( "477a08.5r", 0x000000, 0x200000, CRC(d320dbde) SHA1(eb602cad6ac7c7151c9f29d39b10041d5a354164) )
	ROM_LOAD( "477a09.3r", 0x200000, 0x200000, CRC(f431e29f) SHA1(e6082d88f86abb63d02ac34e70873b58f88b0ddc) )
	ROM_LOAD( "477a10.5n", 0x400000, 0x200000, CRC(8db31bd4) SHA1(d662d3bb6e8b44a01ffa158f5d7425454aad49a3) )
ROM_END

ROM_START( windheat )
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "677eaa01.20u", 0x000000, 0x080000, CRC(500b61f4) SHA1(ec39165412978c0dbd3cbf1f7b6989b5d7ba20a0) ) // Program version EAA, v2.11 (EUR)
	ROM_LOAD32_BYTE( "677eaa02.17u", 0x000001, 0x080000, CRC(99f9fd3b) SHA1(aaec5d7f4e46648aab3738ab09e46b312caee58f) )
	ROM_LOAD32_BYTE( "677eaa03.15u", 0x000002, 0x080000, CRC(c46eba6b) SHA1(80fea082d09071875d30a6a838736cf3a3e4501d) )
	ROM_LOAD32_BYTE( "677eaa04.13u", 0x000003, 0x080000, CRC(20dfcf4e) SHA1(4de8e22507f4719441f14fe96e25f0e0712dfa95) )

	ROM_REGION(0x20000, "audiocpu", 0)      // M68K program
	ROM_LOAD16_WORD_SWAP( "677a07.19l", 0x000000, 0x020000, CRC(05b14f2d) SHA1(3753f71173594ee741980e08eed0f7c3fc3588c9) )

	ROM_REGION(0x100000, "k056832", 0) // Tilemap
	ROM_LOAD16_BYTE( "677a11.35b", 0x000000, 0x080000, CRC(bf34f00f) SHA1(ca0d390c8b30d0cfdad4cfe5a601cc1f6e8c263d) )
	ROM_LOAD16_BYTE( "677a12.35a", 0x000001, 0x080000, CRC(458f0b1d) SHA1(8e11023c75c80b496dfc62b6645cfedcf2a80db4) )

	ROM_REGION(0x800000, "textures", 0) // Texture data
	ROM_LOAD64_WORD( "677a13.9h", 0x000000, 0x200000, CRC(7937d226) SHA1(c2ba777292c293e31068eeb3a27353ad2595b413) )
	ROM_LOAD64_WORD( "677a14.7h", 0x000002, 0x200000, CRC(2568cf41) SHA1(6ed01922943486dafbdc863b76b2036c1fbe5281) )
	ROM_LOAD64_WORD( "677a15.5h", 0x000004, 0x200000, CRC(62e2c3dd) SHA1(c9127ed70bdff947c3da2908a08974091615a685) )
	ROM_LOAD64_WORD( "677a16.2h", 0x000006, 0x200000, CRC(7cc75539) SHA1(4bd8d88debf7489f30008bd4cbded67cb1a20ab0) )

	ROM_REGION(0x600000, "k054539", 0)   // Sound data
	ROM_LOAD( "677a08.5r", 0x000000, 0x200000, CRC(bde38850) SHA1(aaf1bdfc25ecdffc1f6076c9c1b2edbe263171d2) )
	ROM_LOAD( "677a09.3r", 0x200000, 0x200000, CRC(4dfc1ea9) SHA1(4ab264c1902b522bc0589766e42f2b6ca276808d) )
	ROM_LOAD( "677a10.5n", 0x400000, 0x200000, CRC(d8f77a68) SHA1(ff251863ef096f0864f6cbe6caa43b0aa299d9ee) )
ROM_END

ROM_START( windheatu )
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "677ubc01.20u", 0x000000, 0x080000, CRC(63198721) SHA1(7f34131bf51d573d0c683b28df2567a0b911c98c) ) // Program version UBC, v2.22 (USA)
	ROM_LOAD32_BYTE( "677ubc02.17u", 0x000001, 0x080000, CRC(bdb00e2d) SHA1(c54b2250047576e12e9936300989e40494b4659d) )
	ROM_LOAD32_BYTE( "677ubc03.15u", 0x000002, 0x080000, CRC(0f7d8c1f) SHA1(63de03c7be794b6dae8d0af69e894ac573dbbc11) )
	ROM_LOAD32_BYTE( "677ubc04.13u", 0x000003, 0x080000, CRC(4e42791c) SHA1(a53c6374c6b46db578be4ced2ee7c2af7062d961) )

	ROM_REGION(0x20000, "audiocpu", 0)      // M68K program
	ROM_LOAD16_WORD_SWAP( "677a07.19l", 0x000000, 0x020000, CRC(05b14f2d) SHA1(3753f71173594ee741980e08eed0f7c3fc3588c9) )

	ROM_REGION(0x100000, "k056832", 0) // Tilemap
	ROM_LOAD16_BYTE( "677a11.35b", 0x000000, 0x080000, CRC(bf34f00f) SHA1(ca0d390c8b30d0cfdad4cfe5a601cc1f6e8c263d) )
	ROM_LOAD16_BYTE( "677a12.35a", 0x000001, 0x080000, CRC(458f0b1d) SHA1(8e11023c75c80b496dfc62b6645cfedcf2a80db4) )

	ROM_REGION(0x800000, "textures", 0) // Texture data
	ROM_LOAD64_WORD( "677a13.9h", 0x000000, 0x200000, CRC(7937d226) SHA1(c2ba777292c293e31068eeb3a27353ad2595b413) )
	ROM_LOAD64_WORD( "677a14.7h", 0x000002, 0x200000, CRC(2568cf41) SHA1(6ed01922943486dafbdc863b76b2036c1fbe5281) )
	ROM_LOAD64_WORD( "677a15.5h", 0x000004, 0x200000, CRC(62e2c3dd) SHA1(c9127ed70bdff947c3da2908a08974091615a685) )
	ROM_LOAD64_WORD( "677a16.2h", 0x000006, 0x200000, CRC(7cc75539) SHA1(4bd8d88debf7489f30008bd4cbded67cb1a20ab0) )

	ROM_REGION(0x600000, "k054539", 0)   // Sound data
	ROM_LOAD( "677a08.5r", 0x000000, 0x200000, CRC(bde38850) SHA1(aaf1bdfc25ecdffc1f6076c9c1b2edbe263171d2) )
	ROM_LOAD( "677a09.3r", 0x200000, 0x200000, CRC(4dfc1ea9) SHA1(4ab264c1902b522bc0589766e42f2b6ca276808d) )
	ROM_LOAD( "677a10.5n", 0x400000, 0x200000, CRC(d8f77a68) SHA1(ff251863ef096f0864f6cbe6caa43b0aa299d9ee) )
ROM_END

ROM_START( windheatj )
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "677jaa01.20u", 0x000000, 0x080000, CRC(559b8def) SHA1(6f2e8f29b0d9a950e71015270560813adc20b689) ) // Program version JAA, v2.11 (JPN)
	ROM_LOAD32_BYTE( "677jaa02.17u", 0x000001, 0x080000, CRC(cc230575) SHA1(be2da67600ab5edad2e8b7711c4cf985befe28bf) )
	ROM_LOAD32_BYTE( "677jaa03.15u", 0x000002, 0x080000, CRC(20b04701) SHA1(463be36c7f65b4aa3c3f2b1f37d1e6c1f5106cbb) )
	ROM_LOAD32_BYTE( "677jaa04.13u", 0x000003, 0x080000, CRC(f563b2a5) SHA1(b55b486b6af926eff4729f402116d45b61c5d25a) )

	ROM_REGION(0x20000, "audiocpu", 0)      // M68K program
	ROM_LOAD16_WORD_SWAP( "677a07.19l", 0x000000, 0x020000, CRC(05b14f2d) SHA1(3753f71173594ee741980e08eed0f7c3fc3588c9) )

	ROM_REGION(0x100000, "k056832", 0) // Tilemap
	ROM_LOAD16_BYTE( "677a11.35b", 0x000000, 0x080000, CRC(bf34f00f) SHA1(ca0d390c8b30d0cfdad4cfe5a601cc1f6e8c263d) )
	ROM_LOAD16_BYTE( "677a12.35a", 0x000001, 0x080000, CRC(458f0b1d) SHA1(8e11023c75c80b496dfc62b6645cfedcf2a80db4) )

	ROM_REGION(0x800000, "textures", 0) // Texture data
	ROM_LOAD64_WORD( "677a13.9h", 0x000000, 0x200000, CRC(7937d226) SHA1(c2ba777292c293e31068eeb3a27353ad2595b413) )
	ROM_LOAD64_WORD( "677a14.7h", 0x000002, 0x200000, CRC(2568cf41) SHA1(6ed01922943486dafbdc863b76b2036c1fbe5281) )
	ROM_LOAD64_WORD( "677a15.5h", 0x000004, 0x200000, CRC(62e2c3dd) SHA1(c9127ed70bdff947c3da2908a08974091615a685) )
	ROM_LOAD64_WORD( "677a16.2h", 0x000006, 0x200000, CRC(7cc75539) SHA1(4bd8d88debf7489f30008bd4cbded67cb1a20ab0) )

	ROM_REGION(0x600000, "k054539", 0)   // Sound data
	ROM_LOAD( "677a08.5r", 0x000000, 0x200000, CRC(bde38850) SHA1(aaf1bdfc25ecdffc1f6076c9c1b2edbe263171d2) )
	ROM_LOAD( "677a09.3r", 0x200000, 0x200000, CRC(4dfc1ea9) SHA1(4ab264c1902b522bc0589766e42f2b6ca276808d) )
	ROM_LOAD( "677a10.5n", 0x400000, 0x200000, CRC(d8f77a68) SHA1(ff251863ef096f0864f6cbe6caa43b0aa299d9ee) )
ROM_END

ROM_START( windheata )
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "677aaa01.20u", 0x000000, 0x080000, CRC(0d88d0e2) SHA1(93da258bfdb2baa1796916ea8350fff521d43373) ) // Program version AAA, 2.11 (ASA)
	ROM_LOAD32_BYTE( "677aaa02.17u", 0x000001, 0x080000, CRC(f71044a3) SHA1(a88990d4a65b610f695f4a6ff42868d04f6ba1b3) )
	ROM_LOAD32_BYTE( "677aaa03.15u", 0x000002, 0x080000, CRC(3c897588) SHA1(718b0eb57f23a3117d2ad3c58e53196f72fc61bf) )
	ROM_LOAD32_BYTE( "677aaa04.13u", 0x000003, 0x080000, CRC(aee84b7d) SHA1(b69a44e51e21f28bcd5cd87297066fc7ba7b5043) )

	ROM_REGION(0x20000, "audiocpu", 0)      // M68K program
	ROM_LOAD16_WORD_SWAP( "677a07.19l", 0x000000, 0x020000, CRC(05b14f2d) SHA1(3753f71173594ee741980e08eed0f7c3fc3588c9) )

	ROM_REGION(0x100000, "k056832", 0) // Tilemap
	ROM_LOAD16_BYTE( "677a11.35b", 0x000000, 0x080000, CRC(bf34f00f) SHA1(ca0d390c8b30d0cfdad4cfe5a601cc1f6e8c263d) )
	ROM_LOAD16_BYTE( "677a12.35a", 0x000001, 0x080000, CRC(458f0b1d) SHA1(8e11023c75c80b496dfc62b6645cfedcf2a80db4) )

	ROM_REGION(0x800000, "textures", 0) // Texture data
	ROM_LOAD64_WORD( "677a13.9h", 0x000000, 0x200000, CRC(7937d226) SHA1(c2ba777292c293e31068eeb3a27353ad2595b413) )
	ROM_LOAD64_WORD( "677a14.7h", 0x000002, 0x200000, CRC(2568cf41) SHA1(6ed01922943486dafbdc863b76b2036c1fbe5281) )
	ROM_LOAD64_WORD( "677a15.5h", 0x000004, 0x200000, CRC(62e2c3dd) SHA1(c9127ed70bdff947c3da2908a08974091615a685) )
	ROM_LOAD64_WORD( "677a16.2h", 0x000006, 0x200000, CRC(7cc75539) SHA1(4bd8d88debf7489f30008bd4cbded67cb1a20ab0) )

	ROM_REGION(0x600000, "k054539", 0)   // Sound data
	ROM_LOAD( "677a08.5r", 0x000000, 0x200000, CRC(bde38850) SHA1(aaf1bdfc25ecdffc1f6076c9c1b2edbe263171d2) )
	ROM_LOAD( "677a09.3r", 0x200000, 0x200000, CRC(4dfc1ea9) SHA1(4ab264c1902b522bc0589766e42f2b6ca276808d) )
	ROM_LOAD( "677a10.5n", 0x400000, 0x200000, CRC(d8f77a68) SHA1(ff251863ef096f0864f6cbe6caa43b0aa299d9ee) )
ROM_END

ROM_START( jetwave )
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "678eab01.20u", 0x000000, 0x080000, CRC(bc657198) SHA1(e521bb2c1b1a3ae934c98ce1656d35821fc287c9) ) // Program version EAB, EUR v1.04
	ROM_LOAD32_BYTE( "678eab02.17u", 0x000001, 0x080000, CRC(a9a57090) SHA1(ae0273b00c64687f8f835aba531580654edd1097) )
	ROM_LOAD32_BYTE( "678eab03.15u", 0x000002, 0x080000, CRC(483aaff0) SHA1(86e011337532f6ff0174393758784b276143ba10) )
	ROM_LOAD32_BYTE( "678eab04.13u", 0x000003, 0x080000, CRC(c7580d72) SHA1(6a5652365a85917ac48b0f1ced70b9c311e89a4f) )

	ROM_REGION(0x20000, "audiocpu", 0)      // M68K program
	ROM_LOAD16_WORD_SWAP( "678a07.19l", 0x000000, 0x020000, CRC(bb3f5875) SHA1(97f80d9b55d4177217b7cd1ba14e8ed2d64376bb) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP( "685a05.10u", 0x000000, 0x200000, CRC(00e59741) SHA1(d799910d4e85482b0e92a3cc9043f81d97b2fb02) )
	ROM_LOAD32_WORD_SWAP( "685a06.8u",  0x000002, 0x200000, CRC(fc98c6a5) SHA1(a84583bb7296fa9e0c284b2ac59e2dc7b2689eee) )

	ROM_REGION(0x800000, "textures", 0) // Texture data
	ROM_LOAD64_WORD( "678a13.18d", 0x000000, 0x200000, CRC(ccf75722) SHA1(f48d21dfc4f82adbb4c9c841a809267cfd028a3d) )
	ROM_LOAD64_WORD( "678a14.13d", 0x000002, 0x200000, CRC(333a1ab4) SHA1(79df4a98b7871eba4157307a7709da8f8b5da39b) )
	ROM_LOAD64_WORD( "678a15.9d",  0x000004, 0x200000, CRC(58b670f8) SHA1(5d4facb00e34de3ad11ed60c19835918a9cf6cb9) )
	ROM_LOAD64_WORD( "678a16.4d",  0x000006, 0x200000, CRC(137b9bff) SHA1(5052c1fa30cc1d6affd78f48d483415dca89d10b) )

	ROM_REGION(0x600000, "k054539", 0)   // Sound data
	ROM_LOAD( "678a08.5r", 0x000000, 0x200000, CRC(4aeb61ad) SHA1(ec6872cb2e4776849963f48c1c245ca7697849e0) )
	ROM_LOAD( "678a09.3r", 0x200000, 0x200000, CRC(39baef23) SHA1(9f7bda0f9c06eee94703f9ceb06975c8e28338cc) )
	ROM_LOAD( "678a10.5n", 0x400000, 0x200000, CRC(0508280e) SHA1(a36c5dc377b0ba597f131bd9dfc6019e7fc2d243) )
ROM_END

ROM_START( waveshrk )
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "678uab01.20u", 0x000000, 0x080000, CRC(a9b9ceed) SHA1(36f0d18481d7c3e7358e02473e54bc6b52d5c26b) ) // Program version UAB, USA v1.04
	ROM_LOAD32_BYTE( "678uab02.17u", 0x000001, 0x080000, CRC(5ed24ac8) SHA1(d659c751558d4f8d89314466a37c04ac2df46879) )
	ROM_LOAD32_BYTE( "678uab03.15u", 0x000002, 0x080000, CRC(f4a595e7) SHA1(e05e7ea6613ecf70d8470af5fe0c6a7274c6e45b) )
	ROM_LOAD32_BYTE( "678uab04.13u", 0x000003, 0x080000, CRC(fd3320a7) SHA1(03a50a7bba9eb7cdb9f84953d6fb5c09f2d4b2db) )

	ROM_REGION(0x20000, "audiocpu", 0)      // M68K program
	ROM_LOAD16_WORD_SWAP( "678a07.19l", 0x000000, 0x020000, CRC(bb3f5875) SHA1(97f80d9b55d4177217b7cd1ba14e8ed2d64376bb) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP( "685a05.10u", 0x000000, 0x200000, CRC(00e59741) SHA1(d799910d4e85482b0e92a3cc9043f81d97b2fb02) )
	ROM_LOAD32_WORD_SWAP( "685a06.8u",  0x000002, 0x200000, CRC(fc98c6a5) SHA1(a84583bb7296fa9e0c284b2ac59e2dc7b2689eee) )

	ROM_REGION(0x800000, "textures", 0) // Texture data
	ROM_LOAD64_WORD( "678a13.18d", 0x000000, 0x200000, CRC(ccf75722) SHA1(f48d21dfc4f82adbb4c9c841a809267cfd028a3d) )
	ROM_LOAD64_WORD( "678a14.13d", 0x000002, 0x200000, CRC(333a1ab4) SHA1(79df4a98b7871eba4157307a7709da8f8b5da39b) )
	ROM_LOAD64_WORD( "678a15.9d",  0x000004, 0x200000, CRC(58b670f8) SHA1(5d4facb00e34de3ad11ed60c19835918a9cf6cb9) )
	ROM_LOAD64_WORD( "678a16.4d",  0x000006, 0x200000, CRC(137b9bff) SHA1(5052c1fa30cc1d6affd78f48d483415dca89d10b) )

	ROM_REGION(0x600000, "k054539", 0)   // Sound data
	ROM_LOAD( "678a08.5r", 0x000000, 0x200000, CRC(4aeb61ad) SHA1(ec6872cb2e4776849963f48c1c245ca7697849e0) )
	ROM_LOAD( "678a09.3r", 0x200000, 0x200000, CRC(39baef23) SHA1(9f7bda0f9c06eee94703f9ceb06975c8e28338cc) )
	ROM_LOAD( "678a10.5n", 0x400000, 0x200000, CRC(0508280e) SHA1(a36c5dc377b0ba597f131bd9dfc6019e7fc2d243) )
ROM_END

ROM_START( jetwavej )
	ROM_REGION32_BE(0x200000, "prgrom", 0)    // PowerPC program roms
	ROM_LOAD32_BYTE( "678jab01.20u", 0x000000, 0x080000, CRC(fa3da5cc) SHA1(33307e701e6eb28d44e0653ac3f1de47fc17779d) ) // Program version JAB, JPN v1.04
	ROM_LOAD32_BYTE( "678jab02.17u", 0x000001, 0x080000, CRC(01c6713e) SHA1(68e27c018f974e820ba2e99d89a743e53faf1e65) )
	ROM_LOAD32_BYTE( "678jab03.15u", 0x000002, 0x080000, CRC(21c757cb) SHA1(1de6df8e4c52d40882cbf771ff7215ed7b53f251) )
	ROM_LOAD32_BYTE( "678jab04.13u", 0x000003, 0x080000, CRC(fdcc1ecc) SHA1(206cb98a6587cd8e5a9287037d85f392bd2f6e82) )

	ROM_REGION(0x20000, "audiocpu", 0)      // M68K program
	ROM_LOAD16_WORD_SWAP( "678a07.19l", 0x000000, 0x020000, CRC(bb3f5875) SHA1(97f80d9b55d4177217b7cd1ba14e8ed2d64376bb) )

	ROM_REGION32_BE(0x400000, "datarom", 0)   // data roms
	ROM_LOAD32_WORD_SWAP( "685a05.10u", 0x000000, 0x200000, CRC(00e59741) SHA1(d799910d4e85482b0e92a3cc9043f81d97b2fb02) )
	ROM_LOAD32_WORD_SWAP( "685a06.8u",  0x000002, 0x200000, CRC(fc98c6a5) SHA1(a84583bb7296fa9e0c284b2ac59e2dc7b2689eee) )

	ROM_REGION(0x800000, "textures", 0) // Texture data
	ROM_LOAD64_WORD( "678a13.18d", 0x000000, 0x200000, CRC(ccf75722) SHA1(f48d21dfc4f82adbb4c9c841a809267cfd028a3d) )
	ROM_LOAD64_WORD( "678a14.13d", 0x000002, 0x200000, CRC(333a1ab4) SHA1(79df4a98b7871eba4157307a7709da8f8b5da39b) )
	ROM_LOAD64_WORD( "678a15.9d",  0x000004, 0x200000, CRC(58b670f8) SHA1(5d4facb00e34de3ad11ed60c19835918a9cf6cb9) )
	ROM_LOAD64_WORD( "678a16.4d",  0x000006, 0x200000, CRC(137b9bff) SHA1(5052c1fa30cc1d6affd78f48d483415dca89d10b) )

	ROM_REGION(0x600000, "k054539", 0)   // Sound data
	ROM_LOAD( "678a08.5r", 0x000000, 0x200000, CRC(4aeb61ad) SHA1(ec6872cb2e4776849963f48c1c245ca7697849e0) )
	ROM_LOAD( "678a09.3r", 0x200000, 0x200000, CRC(39baef23) SHA1(9f7bda0f9c06eee94703f9ceb06975c8e28338cc) )
	ROM_LOAD( "678a10.5n", 0x400000, 0x200000, CRC(0508280e) SHA1(a36c5dc377b0ba597f131bd9dfc6019e7fc2d243) )
ROM_END

} // Anonymous namespace


/*****************************************************************************/

GAME( 1995, midnrun,   0,        midnrun, midnrun,  midnrun_state, driver_init,  ROT0, "Konami", "Midnight Run: Road Fighter 2 (EAA, Euro v1.11)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN )
GAME( 1995, midnrunj,  midnrun,  midnrun, midnrun,  midnrun_state, driver_init,  ROT0, "Konami", "Midnight Run: Road Fighter 2 (JAD, Japan v1.10)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN )
GAME( 1995, midnruna,  midnrun,  midnrun, midnrun,  midnrun_state, driver_init,  ROT0, "Konami", "Midnight Run: Road Fighter 2 (AAA, Asia v1.10, older sound program)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN )
GAME( 1995, midnruna2, midnrun,  midnrun, midnrun,  midnrun_state, driver_init,  ROT0, "Konami", "Midnight Run: Road Fighter 2 (AAA, Asia v1.10, newer sound program)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN )
GAME( 1996, windheat,  0,        midnrun, midnrun,  midnrun_state, driver_init,  ROT0, "Konami", "Winding Heat (EAA, Euro v2.11)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN )
GAME( 1996, windheatu, windheat, midnrun, midnrun,  midnrun_state, driver_init,  ROT0, "Konami", "Winding Heat (UBC, USA v2.22)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN )
GAME( 1996, windheatj, windheat, midnrun, midnrun,  midnrun_state, driver_init,  ROT0, "Konami", "Winding Heat (JAA, Japan v2.11)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN )
GAME( 1996, windheata, windheat, midnrun, midnrun,  midnrun_state, driver_init,  ROT0, "Konami", "Winding Heat (AAA, Asia v2.11)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN )
GAME( 1996, jetwave,   0,        jetwave, jetwave,  jetwave_state, driver_init,  ROT0, "Konami", "Jet Wave (EAB, Euro v1.04)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN )
GAME( 1996, waveshrk,  jetwave,  jetwave, jetwave,  jetwave_state, driver_init,  ROT0, "Konami", "Wave Shark (UAB, USA v1.04)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN )
GAME( 1996, jetwavej,  jetwave,  jetwave, jetwave,  jetwave_state, driver_init,  ROT0, "Konami", "Jet Wave (JAB, Japan v1.04)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN )
