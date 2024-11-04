// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edström
/***************************************************************************

Alfaskop 41 series

This driver is a part of a revivel project for Alfaskop 41 series where
no known working system exists today because of its distributed nature.
All parts network boots over SS3 (SDLC) from a Floppy Disk unit and nothing
works unless there is a floppy in that unit. These floppies are rare and
many parts have been discarded because they are useless stand alone.

The goal is to emulate missing parts so a full system can be demonstrated again.

Links and credits
-----------------
Project home page: https://github.com/MattisLind/alfaskop_emu
Dalby Datormusem - http://www.datormuseum.se/peripherals/terminals/alfaskop
Bitsavers - http://bitsavers.org/pdf/ericsson/alfaskop/
Dansk Datahistorisk Forening - http://datamuseum.dk/

****************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/input_merger.h"
#include "machine/mc6844.h"
#include "machine/mc6854.h"
#include "machine/output_latch.h"
#include "machine/pla.h"
#include "video/mc6845.h"

#include "screen.h"

//#include "bus/rs232/rs232.h"
//#include "machine/clock.h"

#define LOG_IO    (1U << 1)
#define LOG_NVRAM (1U << 2)
#define LOG_MIC   (1U << 3)
#define LOG_DIA   (1U << 4)
#define LOG_DMA   (1U << 5)
#define LOG_IRQ   (1U << 6)
#define LOG_ADLC  (1U << 7)

//#define VERBOSE (LOG_MIC|LOG_ADLC|LOG_IRQ|LOG_DMA|LOG_IO)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGIO(...)    LOGMASKED(LOG_IO,    __VA_ARGS__)
#define LOGNVRAM(...) LOGMASKED(LOG_NVRAM, __VA_ARGS__)
#define LOGMIC(...)   LOGMASKED(LOG_MIC,   __VA_ARGS__)
#define LOGDIA(...)   LOGMASKED(LOG_DIA,   __VA_ARGS__)
#define LOGDMA(...)   LOGMASKED(LOG_DMA,   __VA_ARGS__)
#define LOGIRQ(...)   LOGMASKED(LOG_IRQ,   __VA_ARGS__)
#define LOGADLC(...)  LOGMASKED(LOG_ADLC,  __VA_ARGS__)


namespace {

#define PLA1_TAG "ic50"
#define PLA1_INUSE 0 // 0=disabled until a PLA converter between DATAIO and MAXLOADER (mame format) exists

class alfaskop4110_state : public driver_device
{
public:
	alfaskop4110_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_kbd_acia(*this, "kbd_acia")
		, m_mic_pia(*this, "mic_pia")
		, m_dia_pia(*this, "dia_pia")
		, m_crtc(*this, "crtc")
		, m_screen(*this, "screen")
		, m_vram(*this, "vram")
		, m_pla(*this, PLA1_TAG)
		, m_chargen(*this, "chargen")
		, m_tia_adlc(*this, "tia_adlc")
		, m_tia_dma(*this, "tia_dma")
		, m_irq(0)
		, m_imsk(0)
	{ }

	void alfaskop4110(machine_config &config);
private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_kbd_acia;
	required_device<pia6821_device> m_mic_pia;
	required_device<pia6821_device> m_dia_pia;
	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_vram;
	required_device<pls100_device> m_pla;

	/* Video controller */
	required_region_ptr<uint8_t> m_chargen;
	MC6845_UPDATE_ROW(crtc_update_row);

	/* TIA */
	required_device<mc6854_device> m_tia_adlc;
	required_device<mc6844_device> m_tia_dma;

	/* Interrupt handling */
	template <unsigned N> void irq_w(int state);
	uint8_t m_irq;
	uint8_t m_imsk;

	/* Debug stuff */
	/* Timer callbacks */
	TIMER_CALLBACK_MEMBER(poll_start);
	TIMER_CALLBACK_MEMBER(poll_bit);

	// DEBUG stuff, will be removed when hooked up towards remote peer
	/* zero extended SDLC poll message frame to feed into receiver as a test
	   0 1 1 1 1 1 1 0   ; opening flag 0x7e
	   0 0 0 0 0 0 0 0   ; 0x00
	   1 1 1 1 1 0 1 1 1 ; 0xff <- a zero needs to be inserted, done by test code
	   0 0 0 0 0 0 1 1   ; 0xc0
	   0 0 0 0 0 1 0 1   ; 0xa0
	   1 0 1 1 0 0 0 1   ; CRC 0x8d
	   1 0 1 0 1 0 1 0   ; CRC 0x55
	   0 1 1 1 1 1 1 0   ; closing flag 0x7e
	*/
	uint8_t txBuf[10] = {0x7e, 0x00, 0xff, 0xc0, 0xa0, 0x8d, 0x55, 0x7e};
	emu_timer *m_poll_start_timer = nullptr;
	emu_timer *m_poll_bit_timer = nullptr;
	int index = 0;
	int pos   = 0;
	int ones  = 0;
	bool flank = false;
};

class alfaskop4120_state : public driver_device
{
public:
	alfaskop4120_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mic_pia(*this, "mic_pia")
		, m_fdapia(*this, "dia_pia")
	{ }

	void alfaskop4120(machine_config &config);
private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_mic_pia;
	required_device<pia6821_device> m_fdapia;
};

class alfaskop4101_state : public driver_device
{
public:
	alfaskop4101_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mic_pia(*this, "mic_pia")
	{ }

	void alfaskop4101(machine_config &config);
private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_mic_pia;
};

void alfaskop4110_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();
	map(0x7800, 0x7fff).ram().share(m_vram); // TODO: Video RAM base address is configurable via NVRAM - this is the default
	map(0x8000, 0xefff).ram();

	// NVRAM
	map(0xf600, 0xf6ff).lrw8(NAME([this](offs_t offset) -> uint8_t { LOGNVRAM("nvram_r %04x: %02x\n", offset, 0); return (uint8_t) 0; }),
				 NAME( [this](offs_t offset, uint8_t data) {    LOGNVRAM("nvram_w %04x: %02x\n", offset, data); }));
	// TIA board
	map(0xf700, 0xf71f).mirror(0x00).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGDMA("TIA DMA_r %04x: %02x\n", offset, 0); return m_tia_dma->read(offset); }),
						  NAME([this](offs_t offset, uint8_t data) { LOGDMA("TIA DMA_w %04x: %02x\n", offset, data); m_tia_dma->write(offset, data); }));
	map(0xf720, 0xf723).mirror(0x04).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGADLC("TIA ADLC_r %04x: %02x\n", offset, 0); return m_tia_adlc->read(offset); }),
						  NAME([this](offs_t offset, uint8_t data) { LOGADLC("TIA ADLC_w %04x: %02x\n", offset, data); m_tia_adlc->write(offset, data); }));

	// Main PCB
	map(0xf7d9, 0xf7d9).mirror(0x06).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGIO("CRTC reg r %04x: %02x\n", offset, 0); return m_crtc->register_r(); }),
						  NAME([this](offs_t offset, uint8_t data) { LOGIO("CRTC reg w %04x: %02x\n", offset, data); m_crtc->register_w(data);}));
	map(0xf7d8, 0xf7d8).mirror(0x06).lw8(NAME([this](offs_t offset, uint8_t data) { LOGIO("CRTC adr w %04x: %02x\n", offset, data); m_crtc->address_w(data); }));
	map(0xf7d0, 0xf7d3).mirror(0x04).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGDIA("DIA pia_r %04x: %02x\n", offset, 0); return m_dia_pia->read(offset & 3); }),
						  NAME([this](offs_t offset, uint8_t data) { LOGDIA("DIA pia_w %04x: %02x\n", offset, data); m_dia_pia->write(offset & 3, data); }));
	map(0xf7c4, 0xf7c7).mirror(0x00).lrw8(NAME([this](offs_t offset) -> uint8_t    { uint8_t tmp = m_mic_pia->read(offset & 3); LOGMIC("\nMIC pia_r %04x: %02x\n", offset, tmp); return tmp; }),
						  NAME([this](offs_t offset, uint8_t data) { LOGMIC("\nMIC pia_w %04x: %02x\n", offset, data); m_mic_pia->write(offset & 3, data); }));
	map(0xf7c0, 0xf7c1).mirror(0x02).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGIO("KBD acia_r %04x: %02x\n", offset, 0); return m_kbd_acia->read(offset & 1); }),
						  NAME([this](offs_t offset, uint8_t data) { LOGIO("KBD acia_w %04x: %02x\n", offset, data); m_kbd_acia->write(offset & 1, data); }));

	map(0xf7fc, 0xf7fc).mirror(0x00).lr8(NAME([this](offs_t offset) -> uint8_t { LOGIO("Address Switch 0-7\n"); return 0; }));

#if PLA1_INUSE
	map(0xf800, 0xffe7).rom().region("roms", 0);

	// IRQ mask setting
	map(0xffe8, 0xfff7).rom().lrw8( NAME([this](offs_t offset) -> uint8_t
					{
						if (!machine().side_effects_disabled()) LOGIO("AMSK read set %04x\n", offset >> 1);
						m_imsk = (offset >> 1) & 7;
						return ((uint8_t *) memregion("roms")->base() + 0x7e8)[offset];
					}),
					NAME([this](offs_t offset, uint8_t data)
					{
						if (!machine().side_effects_disabled()) LOGIO("AMSK write set %04x\n", offset);
						m_imsk = (offset >> 1) & 7;
					}));

	// Address modification
	map(0xfff8, 0xfff9).rom().lrw8( NAME([this](offs_t offset) -> uint8_t
					{
						uint16_t ploffs = (~m_irq & 0xff) | ((m_imsk & 0x07) << 8) | 0x000 | (0x18 << 11);
						uint8_t tmp =  ((uint8_t *) memregion("roms")->base())[0x7e0 | offset | ((m_pla->read(ploffs) & 0xf0) >> 3)];
						if (!machine().side_effects_disabled())
						{
							LOGIO("AMOD read %04x: %02x\n", offset, tmp);
							LOGIO("AMOD pla read %04x: %02x ==> %04x\n", ploffs, m_pla->read(ploffs), (0xffe0 | offset | ((m_pla->read(ploffs) & 0xf0) >> 3)));
						}
						return tmp;
					}),
					NAME([this](offs_t offset, uint8_t data) // TODO: Check what a write does if anything
					{
						if (!machine().side_effects_disabled()) LOGIO("AMOD write %04x\n", offset);
					}));

	map(0xfffa, 0xffff).rom().region("roms", 0x7fa);
#else
	map(0xf800, 0xffff).rom().region("roms", 0);
#endif
}

void alfaskop4120_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xefff).ram();
	map(0xf600, 0xf6ff).lrw8(NAME([this](offs_t offset) -> uint8_t { LOGNVRAM("nvram_r %04x: %02x\n", offset, 0); return (uint8_t) 0; }), // TODO: Move to MRO board
				 NAME([this](offs_t offset, uint8_t data) { LOGNVRAM("nvram_w %04x: %02x\n", offset, data); }));
	map(0xf740, 0xf743).mirror(0x0c).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGIO("FDA pia_r %04x: %02x\n", offset, 0); return m_fdapia->read(offset & 3); }),
						  NAME([this](offs_t offset, uint8_t data) { LOGIO("FDA pia_w %04x: %02x\n", offset, data); m_fdapia->write(offset & 3, data); }));
	map(0xf7c4, 0xf7c7).mirror(0x00).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGMIC("MIC pia_r %04x: %02x\n", offset, 0); return m_mic_pia->read(offset & 3); }),
						  NAME([this](offs_t offset, uint8_t data) { LOGMIC("MIC pia_w %04x: %02x\n", offset, data); m_mic_pia->write(offset & 3, data); }));
	map(0xf800, 0xffff).rom().region("roms", 0);
}

void alfaskop4101_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xefff).ram();
	map(0xf600, 0xf6ff).lrw8(NAME([this](offs_t offset) -> uint8_t { LOGNVRAM("nvram_r %04x: %02x\n", offset, 0); return (uint8_t) 0; }),
				 NAME([this](offs_t offset, uint8_t data) { LOGNVRAM("nvram_w %04x: %02x\n", offset, data); }));
	map(0xf7c4, 0xf7c7).mirror(0x00).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGMIC("MIC pia_r %04x: %02x\n", offset, 0); return m_mic_pia->read(offset & 3); }),
						  NAME([this](offs_t offset, uint8_t data) { LOGMIC("MIC pia_w %04x: %02x\n", offset, data); m_mic_pia->write(offset & 3, data); }));
	map(0xf800, 0xffff).rom().region("roms", 0);
}

/* Input ports */
static INPUT_PORTS_START( alfaskop4110 )
INPUT_PORTS_END

static INPUT_PORTS_START( alfaskop4120 )
INPUT_PORTS_END

static INPUT_PORTS_START( alfaskop4101 )
INPUT_PORTS_END

/* Interrupt handling - vector address modifyer, irq prioritizer and irq mask */
template <unsigned N> void alfaskop4110_state::irq_w(int state)
{
	m_irq = (m_irq & ~(1 << N)) | ((state ? 1 : 0) << N);
	LOGIRQ("4110 IRQ %d: %d ==> %02x\n", N, state, m_irq);
	m_maincpu->set_input_line(M6800_IRQ_LINE, state);
}

/* Simplified chargen, no attributes or special formats/features yet  */
MC6845_UPDATE_ROW( alfaskop4110_state::crtc_update_row )
{
	offs_t const base = ma + 0x4000;
	u32 *px = &bitmap.pix(y);

	for (int i = 0; i < x_count; i++)
	{
		u8 chr = m_vram[(base + i) & 0x07ff] & 0x7f;
		rgb_t const bg = rgb_t::white();
		rgb_t const fg = rgb_t::black();

		u8 dots = m_chargen[chr * 16 + ra];

		for (int n = 8; n > 0; n--, dots <<= 1)
			*px++ = BIT(dots, 7) ? fg : bg;
	}
}

void alfaskop4110_state::alfaskop4110(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(19'170'000) / 18); // Verified from service manual
	m_maincpu->set_addrmap(AS_PROGRAM, &alfaskop4110_state::mem_map);

	/* Interrupt controller and address modifier PLA */
	/*
	 * 82S100 data sheet
	 * -----------------
	 *
	 * The 82S100 is a bipolar, fuse-link programmable logic array. It uses the
	 * standard AND/OR/invert architecture to directly implement custom
	 * um-of-product logic equations.
	 *
	 * Each device consists of 16 dedicated inputs and 8 dedicated outputs. Each
	 * output is capable of being actively controlled by any or all of the 48
	 * product terms. The true, complement, or don't care condition of each of the
	 * 16 inputs ANDed together comprise one P-Term. All 48 P-Terms are then OR-d
	 * to each output. The user must then only select which P-Terms will activate
	 * an output by disconnecting terms which do not affect the output. In addition,
	 * each output can be fused as active high or active low.
	 *
	 * The 82S100 is fully TTL compatible and includes chip-enable control for
	 * expansion of input variables and output inhibit. It features three state
	 * outputs.
	 *
	 * Field programmable Ni-Cr links
	 * 16 inputs
	 * 8 outputs
	 * 48 product terms
	 * Commercial verion - N82S100 - 50ns max address access time
	 * Power dissipation - 600mW typ
	 * Input loading - 100uA max
	 * Chip enable input
	 * Three state outputs
	 *
	 *
	 */
	/*                   _____   _____
	 *        nc FE   1 |*    \_/     | 28  Vcc
	 *      IRQ7 I7   2 |             | 27  I8  mask 1
	 *      IRQ6 I6   3 |             | 26  I9  mask 2
	 *      IRQ5 I5   4 |             | 25  I10 mask 3
	 *      IRQ4 I4   5 |             | 24  I11 Address &== 1111 1111 111x xxxx
	 *      IRQ3 I3   6 |    82S100   | 23  I12 AI 1 A1
	 *      IRQ2 I2   7 |             | 22  I13 AI 2 A2
	 *      IRQ1 I1   8 |     IC50    | 21  I14 AI 3 A3
	 *      IRQ0 I0   9 |             | 20  I15 AI 4 A4
	 *        P4 F7  10 |  Interrupt  | 19  _CE
	 *   mask P3 F6  11 |  Controller | 18  F0   IRQ
	 *   mask P2 F5  12 |     PLA     | 17  F1   mask register
	 *   mask P1 F4  13 |             | 16  F2   interrupt latch
	 *          GND  14 |_____________| 15  F3   nc
	 */
	PLS100(config, m_pla);

	MC6845(config, m_crtc, XTAL(19'170'000) / 9);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(alfaskop4110_state::crtc_update_row));
	// VSYNC should goto IRQ1 through some logic involving MIC PIA CRA bits 0 ( 1 == enable) & 1 (1 == positive edge)
	//m_crtc->out_vsync_callback().set(FUNC(alfaskop4110_state::crtc_vsync);
	//m_crtc->out_vsync_callback().set([this](bool state) { LOGIRQ("CRTC VSYNC: %d\n", state); });
	//m_crtc->out_vsync_callback().set("irq1", FUNC(input_merger_device::in_w<1>));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(19'170'000, 80 * 8, 0, 80 * 8, 400, 0, 400);
	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PIA6821(config, m_mic_pia); // Main board PIA
	m_mic_pia->cb1_w(0);
	m_mic_pia->cb2_handler().set([this](offs_t offset, uint8_t data) { LOGMIC("->MIC PIA: CB2 write %d\n", data); });

	/*
	 * MIC PIA interface
	 *
	 * Port A (DDRA=0x7a)
	 * 0 - PA0 input  - not used
	 * 1 - PA1 output - KB reset P11 pin 23 at connector  1 == KB reset           0 == no KB reset
	 * 2 - PA2 input  - MCP test mode                     1 == no test mode       0 == in test mode,
	 * 3 - PA3 output - not used (in DTC)
	 * 4 - PA4 output - not used (in DTC)
	 * 5 - PA5 output - Interrupt enable                  1 == Int. out on P1:7   0 == no Int. out
	 * 6 - PA6 output - I4 latch enable                   1 == I4 will be latched 0 == no I4 latch
	 * 7 - PA7 input  - Button/MCP NMI                    1 == NMI from DU button 0 == NMI from MCP P4:1=low
	 * Note: At initialization a KB reset pulse will be sent as DDRA contains all zeros: PA I functions as a
	 *       high impedance input: "active level" for KB reset generation.
	 *
	 * Port B (DDRB=0xff)
	 * 0 - PB0 output - Reset PC-error                    1 == Reset PC error FF  0 == Memory PC used
	 *                                                         or PC not used
	 * 1 - PB1 output - VMAX/VMA 1 MPU                    1 == VMAX gen by MPU    0 == VMA 1 gen by MPU
	 * 2 - PB2 output - VMAX/VMA 1 DMA                    1 == VMAX gen by DMA    0 == VMA 1 gen by DMA
	 * 3 - PB3 output - Display Memory                    1 == 4KB Display Memory 0 == 8KB Display Memory
	 * 4 - PB4 output - Option Character Generator        1 == Enabled to MIC bus 0 == Disabled from MIC bus
	 * 5 - PB5 output - MPU Addr                          1 == Mode 1             0 == Mode 0
	 * 6 - PB6 output - Reset                             1 == Reset all but MPU  0 == No reset
	 *                                                         and MIC PIA
	 * 7 - PB7 output - not used
	 */
	m_mic_pia->writepa_handler().set([this](offs_t offset, uint8_t data)
					{
						LOGMIC("->MIC PIA: Port A write %02x\n", data);
						LOGMIC(" PA1 - KBD reset %s\n", BIT(data, 1) ? "active" : "inactive");
						LOGMIC(" PA5 - Int out %s\n", BIT(data, 5) ? "enabled": "disabled");
						LOGMIC(" PA6 - I4 latch %s\n", BIT(data, 6) ? "enabled": "disabled");
					});

	m_mic_pia->writepb_handler().set([this](offs_t offset, uint8_t data)
					{
						LOGMIC("->MIC PIA: Port B write %02x\n", data);
						LOGMIC(" PB0 - Reset PC-error %s\n", BIT(data, 0) ? "active" : "inactive");
						LOGMIC(" PB1 - %s generated by MPU\n", BIT(data, 1) ? "VMAX" : "VMA 1");
						LOGMIC(" PB2 - %s generated by DMA\n", BIT(data, 2) ? "VMAX" : "VMA 1");
						LOGMIC(" PB3 - %sKB Display Memory\n", BIT(data, 3) ? "4" : "8");
						LOGMIC(" PB4 - Option Char Generator %s\n", BIT(data, 4) ? "enabled" : "disabled");
						LOGMIC(" PB5 - MPU Address Mode %s\n", BIT(data, 5) ? "1" : "0");
						LOGMIC(" PB6 - Reset of devices %s\n", BIT(data, 6) ? "active" : "inactive");
					});

	m_mic_pia->readpa_handler().set([this](offs_t offset) -> uint8_t
					{
						uint8_t data = (1U << 2); // MCU is not in test mode
						LOGMIC("<-MIC PIA: Port A read\n");
						LOGMIC(" PA2 - MCU test mode %s\n", BIT(data, 2) ? "inactive" : "active");
						return 0;
					});
	m_mic_pia->readpb_handler().set([this](offs_t offset) -> uint8_t { LOGMIC("<-MIC PIA: Port B read\n"); return 0;});
	m_mic_pia->ca1_w(0);
	m_mic_pia->ca2_w(0);

	PIA6821(config, m_dia_pia); // Display PIA, controls how the CRTC accesses memory etc
	m_dia_pia->cb1_w(0);
	m_dia_pia->cb2_handler().set([this](offs_t offset, uint8_t data) { LOGDIA("DIA PIA: CB2_w %d\n", data); });
	m_dia_pia->writepa_handler().set([this](offs_t offset, uint8_t data) { LOGDIA("DIA PIA: PA_w %02x\n", data); });
	m_dia_pia->writepb_handler().set([this](offs_t offset, uint8_t data) { LOGDIA("DIA PIA: PB_w %02x\n", data); });
	m_dia_pia->readpa_handler().set([this](offs_t offset) -> uint8_t { LOGDIA("DIA PIA: PA_r\n"); return 0;});
	m_dia_pia->readpb_handler().set([this](offs_t offset) -> uint8_t { LOGDIA("DIA PIA: PB_r\n"); return 0;});
	m_dia_pia->ca1_w(0);
	m_dia_pia->ca2_w(0);

	ACIA6850(config, m_kbd_acia, 0);
	//CLOCK(config, "acia_clock", ACIA_CLOCK).signal_handler().set(FUNC(alfaskop4110_state::write_acia_clock));
	m_kbd_acia->irq_handler().set("irq3", FUNC(input_merger_device::in_w<3>));

	MC6854(config, m_tia_adlc, XTAL(19'170'000) / 18); // TODO: attach IRQ by IRQ 7 through descrete interrupt prioritization instead
	//m_tia_adlc->out_irq_cb().set([this](bool state){ LOGDMA("TIA ADLC IRQ: %s\n", state == ASSERT_LINE ? "asserted" : "cleared"); m_maincpu->set_input_line(M6800_IRQ_LINE, state); });
	//m_tia_adlc->out_irq_cb().set([this](bool state){ LOGDMA("TIA ADLC IRQ: %s\n", state == ASSERT_LINE ? "asserted" : "cleared"); m_maincpu->set_input_line(M6800_IRQ_LINE, state); });
	m_tia_adlc->out_irq_cb().set("irq7", FUNC(input_merger_device::in_w<7>));
	m_tia_adlc->out_rdsr_cb().set([this](bool state){ LOGDMA("TIA ADLC RDSR: %d\n", state); m_tia_dma->dreq_w<1>(state); });
	m_tia_adlc->out_tdsr_cb().set([this](bool state){ LOGDMA("TIA ADLC TDSR: %d\n", state); m_tia_dma->dreq_w<0>(state); });

	MC6844(config, m_tia_dma, XTAL(19'170'000) / 18);
	//m_tia_dma->out_int_callback().set([this](bool state){ LOGDMA("TIA DMA IRQ: %d\n", state); }); // Used as DEND (end of dma) towards the ADLC through some logic
	m_tia_dma->out_drq1_callback().set([this](bool state){ LOGDMA("TIA DMA DRQ1: %d\n", state); m_tia_dma->dgrnt_w(state); });
	//m_tia_dma->out_drq2_callback().set([this](bool state){ LOGDMA("TIA DMA DRQ2: %d\n", state); }); // Not connected
	m_tia_dma->in_ior_callback<1>().set([this](offs_t offset) -> uint8_t { return m_tia_adlc->dma_r(); });
	m_tia_dma->out_memw_callback().set([this](offs_t offset, uint8_t data) { m_maincpu->space(AS_PROGRAM).write_byte(offset, data); });

	/* 74LS273 latch inputs of interruptt sources */
	INPUT_MERGER_ANY_HIGH(config, "irq0").output_handler().set(FUNC(alfaskop4110_state::irq_w<0>));
	INPUT_MERGER_ANY_HIGH(config, "irq1").output_handler().set(FUNC(alfaskop4110_state::irq_w<1>));
	INPUT_MERGER_ANY_HIGH(config, "irq2").output_handler().set(FUNC(alfaskop4110_state::irq_w<2>));
	INPUT_MERGER_ANY_HIGH(config, "irq3").output_handler().set(FUNC(alfaskop4110_state::irq_w<3>));
	INPUT_MERGER_ANY_HIGH(config, "irq4").output_handler().set(FUNC(alfaskop4110_state::irq_w<4>));
	INPUT_MERGER_ANY_HIGH(config, "irq5").output_handler().set(FUNC(alfaskop4110_state::irq_w<5>));
	INPUT_MERGER_ANY_HIGH(config, "irq6").output_handler().set(FUNC(alfaskop4110_state::irq_w<6>));
	INPUT_MERGER_ANY_HIGH(config, "irq7").output_handler().set(FUNC(alfaskop4110_state::irq_w<7>));
}

void alfaskop4110_state::machine_start()
{
	save_item(NAME(m_irq));
	save_item(NAME(m_imsk));

	m_poll_start_timer = timer_alloc(FUNC(alfaskop4110_state::poll_start), this);
	m_poll_start_timer->adjust(attotime::from_msec(5000));

	m_poll_bit_timer = timer_alloc(FUNC(alfaskop4110_state::poll_bit), this);
	m_poll_bit_timer->adjust(attotime::never);
}

// Debug - inserts a poll SDLC frame through the ADLC, it ends up at address 0x140 in RAM through DMA
TIMER_CALLBACK_MEMBER(alfaskop4110_state::poll_start)
{
	/* The serial transfer of 8 bits is complete. Now trigger INT7. */
	LOGADLC("Starting poll message\n");
	m_tia_adlc->set_rx(0);
	m_poll_bit_timer->adjust(attotime::from_hz(300000));
}

TIMER_CALLBACK_MEMBER(alfaskop4110_state::poll_bit)
{
	if (flank)
	{
		if (index != 0 && index != 7 && BIT(txBuf[index], (pos % 8)) && ones == 5)
		{
			LOGADLC("%d%c", 2, (pos % 8) == 7 ? '\n' : ' ');
			m_tia_adlc->set_rx(0);
			ones = 0;
		}
		else
		{
			LOGADLC("%d%c", BIT(txBuf[index], (pos % 8)), (pos % 8) == 7 ? '\n' : ' ');
			m_tia_adlc->set_rx(BIT(txBuf[index], (pos % 8)));
			if (index != 0 && index != 7 && BIT(txBuf[index], (pos % 8)))
				ones++;
			else
				ones = 0;
			pos++;
			index = pos / 8;
		}
	}
	m_tia_adlc->rxc_w(flank ? 1 : 0);
	if (index < 8)
		m_poll_bit_timer->adjust(attotime::from_hz(300000) / 2);
	flank = !flank;
}

void alfaskop4110_state::machine_reset()
{
	m_irq = 0x00;
}

void alfaskop4120_state::alfaskop4120(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(19'170'000) / 18); // Verified from service manual
	m_maincpu->set_addrmap(AS_PROGRAM, &alfaskop4120_state::mem_map);

	PIA6821(config, m_mic_pia); // Main Board PIA
	PIA6821(config, m_fdapia); // Floppy Disk PIA
}

void alfaskop4101_state::alfaskop4101(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(19'170'000) / 18); // Verified from service manual
	m_maincpu->set_addrmap(AS_PROGRAM, &alfaskop4101_state::mem_map);

	PIA6821(config, m_mic_pia); // Main board PIA
}

/* ROM definitions */
ROM_START( alfaskop4110 ) // Display Unit
	ROM_REGION( 0x800, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "e3405870205201.bin", 0x0000, 0x0800, CRC(23f20f7f) SHA1(6ed008e309473ab966c6b0d42a4f87c76a7b1d6e))
	ROM_REGION( 0x800, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "e3405972067500.bin", 0x0000, 0x0400, CRC(fb12b549) SHA1(53783f62c5e51320a53e053fbcf8b3701d8a805f))
	ROM_LOAD( "e3405972067600.bin", 0x0400, 0x0400, CRC(c7069d65) SHA1(587efcbee036d4c0c5b936cc5d7b1f97b6fe6dba))

	ROM_REGION( 0xff, PLA1_TAG, 0 )
	ROM_LOAD( "dtc_a_e34062_0100_ic50_e3405970303601.bin", 0x00, 0xfa, CRC(16339b7a) SHA1(9b313a7526460dc9bcedfda25bece91c924f0ddc) ) // Signetics_N82S100N.bin DATAIO format
ROM_END

ROM_START( alfaskop4120 ) // Flexible Disk Unit
	ROM_REGION( 0x800, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "alfaskop4120.bin", 0x0000, 0x0800, NO_DUMP)
ROM_END

ROM_START( alfaskop4101 ) // Communication Processor Unit
	ROM_REGION( 0x800, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "alfaskop4101.bin", 0x0000, 0x0800, NO_DUMP)
ROM_END

} // anonymous namespace


/* Driver(S) */

// Only 4101 may exist as a driver in the end making the 4110 and 4120 as slots devices on the SS3 bus, time will tell

//    YEAR  NAME          PARENT  COMPAT  MACHINE       INPUT         CLASS               INIT        COMPANY      FULLNAME       FLAGS
COMP( 1984, alfaskop4110, 0,      0,      alfaskop4110, alfaskop4110, alfaskop4110_state, empty_init, "Ericsson",  "Alfaskop Display Unit 4110", MACHINE_IS_SKELETON)
COMP( 1984, alfaskop4120, 0,      0,      alfaskop4120, alfaskop4120, alfaskop4120_state, empty_init, "Ericsson",  "Alfaskop Flexible Disk Unit 4120", MACHINE_IS_SKELETON)
COMP( 1984, alfaskop4101, 0,      0,      alfaskop4101, alfaskop4101, alfaskop4101_state, empty_init, "Ericsson",  "Alfaskop Communication Processor 4101", MACHINE_IS_SKELETON)
