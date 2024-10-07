// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edström
/***************************************************************************************************
 *
 *   Ericsson Information Systems PC "compatibles"
 *
 * The Ericsson PC was the first original Ericsson design for the office PC market replacing the
 * Step/One which was an OEM:ed clone of the Matsushita Mybrain 3000 (see myb3k.cpp driver).
 *
 **************************************************************
 * Ericsson PC
 *------------
 * Links: https://youtu.be/6uilOdMJc24
 * Form Factor: Desktop
 * CPU: 8088 @ 4.77MHz
 * RAM: 256K
 * Bus: 6x ISA
 * Video: Monchrome or Color 80x25 character mode. 320x200 and 640x400 grahics modes
 * Display: Orange Gas Plasma (GP) display
 * Mass storage: 2 x 5.25" 360K or 1 20Mb HDD
 * On board ports: Beeper,
 * Ports: serial, parallel
 * Internal Options: Up to 640K RAM through add-on RAM card
 * Misc: The hardware was not 100% PC compatible so non BIOS based software would not always run. 50.000+ units sold
 *
 * TODO
 * - Complete the Ericsson 1070 MDA ISA board and test all the graphics modes including 640x400 (aka HR)
 * - Add the Ericsson 1065 HDC and boot from a hard drive
 * - Add softlist
 * - Pass the diagnostics software system test at EPC2.IMD, it currently hangs the keyboard.
 *   A later version of the test on EPC5.IMD works though so need to verify EPC2.IMD on real hardware first.
 *
 * CREDITS  The driver code is inspired from m24.cpp, myb3k.cpp and genpc.cpp. Information about the EPC has
 *          been contributed by many, mainly the people at Dalby Computer museum http://www.datormuseum.se/
 *          A dead pcb was donated by rfka01 and rom dumps by ZnaxQue@sweclockers.com
 *
 ************************************************************************************************************/
/*
 Links
 -----

 */

#include "emu.h"

#include "eispc_kb.h"
#include "epc.lh"

// Devices
#include "cpu/i86/i86.h"
#include "machine/am9517a.h"
#include "machine/i8087.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/upd765.h"
#include "machine/ins8250.h"

// Expansion cards
//#include "bus/isa/isa.h"
//#include "bus/isa/isa_cards.h"
#include "bus/isa/ega.h"
#include "bus/isa/eis_hgb107x.h"
#include "bus/isa/eis_twib.h"
#include "machine/pc_lpt.h"

#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "speaker.h"
#include "imagedev/floppy.h"
#include "bus/rs232/rs232.h"

#define LOG_PPI     (1U << 1)
#define LOG_PIT     (1U << 2)
#define LOG_PIC     (1U << 3)
#define LOG_KBD     (1U << 4)
#define LOG_DMA     (1U << 5)
#define LOG_IRQ     (1U << 6)
#define LOG_FDC     (1U << 7)
#define LOG_LPT     (1U << 8)
#define LOG_NMI     (1U << 9)
#define LOG_BITS    (1U << 10)
#define LOG_FPU     (1U << 11)
#define LOG_COM     (1U << 12)

//#define VERBOSE (LOG_LPT)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGPPI(...)  LOGMASKED(LOG_PPI,  __VA_ARGS__)
#define LOGPIT(...)  LOGMASKED(LOG_PIT,  __VA_ARGS__)
#define LOGPIC(...)  LOGMASKED(LOG_PIC,  __VA_ARGS__)
#define LOGKBD(...)  LOGMASKED(LOG_KBD,  __VA_ARGS__)
#define LOGDMA(...)  LOGMASKED(LOG_DMA,  __VA_ARGS__)
#define LOGIRQ(...)  LOGMASKED(LOG_IRQ,  __VA_ARGS__)
#define LOGFDC(...)  LOGMASKED(LOG_FDC,  __VA_ARGS__)
#define LOGLPT(...)  LOGMASKED(LOG_LPT,  __VA_ARGS__)
#define LOGNMI(...)  LOGMASKED(LOG_NMI,  __VA_ARGS__)
#define LOGBITS(...) LOGMASKED(LOG_BITS, __VA_ARGS__)
#define LOGFPU(...)  LOGMASKED(LOG_FPU,  __VA_ARGS__)
#define LOGCOM(...)  LOGMASKED(LOG_COM,  __VA_ARGS__)


namespace {

class epc_state : public driver_device
{
public:
	epc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_isabus(*this, "isabus")
		, m_dma8237a(*this, "dma8237")
		, m_ppi8255(*this, "ppi8255")
		, m_io_dsw(*this, "DSW")
		, m_io_j10(*this, "J10")
		, m_io_s21(*this, "S21")
		, m_lpt(*this, "lpt")
		, m_kbd8251(*this, "kbd8251")
		, m_keyboard(*this, "keyboard")
		, m_leds(*this, "kbled%u")
		, m_pic8259(*this, "pic8259")
		, m_pit8253(*this, "pit8253")
		, m_speaker(*this, "speaker")
		, m_fdc(*this, "fdc")
		, m_floppy_connectors(*this, "fdc:%u", 0)
		, m_uart(*this, "uart")
	{ }

	void epc(machine_config &config);
	void init_epc();


protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<i8086_cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<isa8_device> m_isabus;

	// DMA
	void dma_tc_w(int state);
	void dreq0_ck_w(int state);
	void epc_dma_hrq_changed(int state);
	void epc_dma8237_out_eop(int state);
	uint8_t epc_dma_read_byte(offs_t offset);
	void epc_dma_write_byte(offs_t offset, uint8_t data);
	template <int Channel> uint8_t epc_dma8237_io_r(offs_t offset);
	template <int Channel> void epc_dma8237_io_w(offs_t offset, uint8_t data);
	template <int Channel> void epc_dack_w(int state);
	required_device<am9517a_device> m_dma8237a;
	uint8_t m_dma_segment[4];
	uint8_t m_dma_active;
	bool m_tc;
	bool m_txd;
	bool m_rxrdy;
	bool m_int;
	bool m_dreq0_ck;

	// PPI
	required_device<i8255_device> m_ppi8255;
	void ppi_portb_w(uint8_t data);
	uint8_t ppi_portc_r();
	uint8_t m_ppi_portb;
	required_ioport m_io_dsw;
	required_ioport m_io_j10;
	required_ioport m_io_s21;

	// Printer port
	optional_device<pc_lpt_device> m_lpt;

	// Keyboard Controller/USART
	required_device<i8251_device> m_kbd8251;
	required_device<eispc_keyboard_device> m_keyboard;
	emu_timer *m_kbdclk_timer;
	TIMER_CALLBACK_MEMBER(rxtxclk_w);
	bool m_8251rxtx_clk_state;
	bool m_kbdclk_state;
	bool m_8251dtr_state;
	int m_kbdclk;
	output_finder<3> m_leds;

	// Interrupt Controller
	required_device<pic8259_device> m_pic8259;
	void int_w(int state);
	uint8_t m_nmi_enabled;
	uint8_t m_8087_int = 0;
	uint8_t m_parer_int = 0;
	uint8_t m_iochck_int = 0;
	void update_nmi();

	// Timer
	required_device<pit8253_device> m_pit8253;

	// Speaker
	void speaker_ck_w(int state);
	required_device<speaker_sound_device> m_speaker;
	bool m_pc4;
	bool m_pc5;

	void epc_map(address_map &map) ATTR_COLD;
	void epc_io(address_map &map) ATTR_COLD;

	// FDC
	void check_fdc_irq();
	void check_fdc_drq();
	required_device<i8272a_device> m_fdc;
	uint8_t m_ocr;
	bool m_irq;     // System signal after glue logic
	bool m_drq;     // System signal after glue logic
	bool m_fdc_irq; // FDC output pin
	bool m_fdc_drq; // FDC output pin

	optional_device_array<floppy_connector, 4> m_floppy_connectors;

	// UART
	required_device<ins8250_device> m_uart;
};

void epc_state::check_fdc_irq()
{
	bool pirq = m_irq;
	m_irq = m_fdc_irq && (m_ocr & 4) && (m_ocr & 8);  // IRQ enabled and not in reset?
	if(m_irq != pirq) // has the state changed?
	{
		LOGIRQ("FDC: IRQ6 request: %d\n", m_irq);
		m_pic8259->ir6_w(m_irq);
	}
}

void epc_state::check_fdc_drq()
{
	bool pdrq = m_drq;
	m_drq = m_fdc_drq && (m_ocr & 4) && (m_ocr & 8); // DREQ enabled and not in reset?
	if(m_drq != pdrq) // has the state changed?
	{
		LOGDMA("FDC: DMA channel 2 request: %d\n", m_drq);
		m_dma8237a->dreq2_w(m_drq);
	}
}

void epc_state::epc_map(address_map &map)
{
	map.unmap_value_high();
	map(0x20000, 0x9ffff).noprw(); // Base RAM - mapped to avoid unmaped errors when BIOS is probing RAM size
	// 0xa0000-0xaffff is reserved
	map(0xb0000, 0xb7fff).noprw(); // Monochrome RAM - mapped to avoid unaped errors when BIOS is probing RAM size
	map(0xb0000, 0xb7fff).noprw(); // Monochrome RAM - mapped to avoid unaped errors when BIOS is probing RAM size
	map(0xb8000, 0xbffff).noprw(); // Color/Graphics RAM - mapped to avoid unaped errors when BIOS is probing RAM size
	map(0xc0000, 0xeffff).noprw(); // Expansion ROM area - Hard Disk BIOS etc
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void epc_state::epc_io(address_map &map)
{
	map(0x0000, 0x000f).mirror(0x10).lrw8(
		[this](offs_t offset) -> uint8_t
		{
			uint8_t data = m_dma8237a->read(offset);
			LOGDMA("dma8237_r %04x\n", offset);
			return data;
		},
		"dma8237_r",
		[this](offs_t offset, uint8_t data)
		{
			LOGDMA("dma8237_w %04x: %02x\n", offset, data);
			m_dma8237a->write(offset, data);
		},
		"dma8237_w"
	);

	map(0x0020, 0x0021).mirror(0x1e).lrw8(
		[this](offs_t offset) -> uint8_t
		{
			uint8_t data = m_pic8259->read(offset);
			LOGPIC("pic8259_r %04x: %02x\n", offset, data);
			return data;
		},
		"pic8259_r",
		[this](offs_t offset, uint8_t data)
		{
			LOGPIC("pic8259_w %04x: %02x\n", offset, data);
			m_pic8259->write(offset, data);
		},
		"pic8259_w"
	);

	map(0x0040, 0x0043).mirror(0x1c).lrw8(
		[this](offs_t offset) -> uint8_t
		{
			uint8_t data = m_pit8253->read(offset);
			LOGPIT("pit8253_r %04x\n", offset);
			return data;
		},
		"pit8253_r",
		[this](offs_t offset, uint8_t data)
		{
			LOGPIT("pit8253_w %04x: %02x\n", offset, data);
			m_pit8253->write(offset, data);
		},
		"pit8253_w"
	);

	map(0x0060, 0x0060).mirror(0x1c).lrw8(
		[this]() -> uint8_t
		{
			uint8_t data = m_kbd8251->data_r();
			LOGKBD("kbd8251_r %02x\n", data);
			return data;
		},
		"kbd_8251_data_r",
		[this](offs_t offset, uint8_t data)
		{
			LOGKBD("kbd8251_w 0x60 %02x\n", data);
			m_kbd8251->data_w(data);
		},
		"kbd_8251_data_w"
	);
									// NOTE: PPI Port A is not mapped
	map(0x0061, 0x0061).mirror(0x1c).lrw8(              // PPI Port B
		[this](offs_t offset) -> uint8_t
		{
			uint8_t data = m_ppi8255->read(1);
			LOGPPI("ppi8255_r Port B: %02x\n", data);
			return data;
		},
		"ppi8255_r",
		[this](offs_t offset, uint8_t data)
		{
			LOGPPI("ppi8255_w Port B: %02x\n", data);
			m_ppi8255->write(1, data);
		},
		"ppi8255_w"
	);

	map(0x0062, 0x0062).mirror(0x1c).lrw8(              // PPI Port C
		[this](offs_t offset) -> uint8_t
		{
			uint8_t data = m_ppi8255->read(2);
			LOGPPI("ppi8255_r Port C: %02x\n", data);
			return data;
		},
		"ppi8255_r",
		[this](offs_t offset, uint8_t data)
		{
			LOGPPI("ppi8255_w Port C: %02x\n", data);
			m_ppi8255->write(2, data);
		},
		"ppi8255_w"
	);

	map(0x0063, 0x0063).lrw8(               // PPI Control register
		[this](offs_t offset) -> uint8_t
		{
			uint8_t data = m_ppi8255->read(3);
			LOGPPI("ppi8255_r Control: %02x\n", data);
			return data;
		},
		"ppi8255_r",
		[this](offs_t offset, uint8_t data)
		{
			LOGPPI("ppi8255_w Control: %02x\n", data);
			m_ppi8255->write(3, data);
		},
		"ppi8255_w"
	);

	map(0x0070, 0x0070).mirror(0x0e).lw8(
		[this](offs_t offset, uint8_t data)
		{
			LOGKBD("kbd8251_w 0x70: %02x\n", data);
			m_kbd8251->data_w(data);
		},
		"i8251_data_w"
	);

	map(0x0071, 0x0071).mirror(0x0e).lrw8(
		[this](offs_t offset) -> uint8_t
		{
			uint8_t stat = m_kbd8251->status_r();
			//LOGKBD("kbd8251_status_r %02x\n", stat);
			return stat;
		},
		"kbd_8251_stat_ctrl_r",
		[this](offs_t offset, uint8_t data)
		{
			LOGKBD("kbd8251_control_w 0x71: %02x\n", data);
			m_kbd8251->control_w(data);
		},
		"kbd_8251_stat_ctrl_w"
	);

	map(0x0080, 0x0083).mirror(0xc).lw8(
		[this](offs_t offset, uint8_t data)
		{
			LOGDMA("dma_segment_w %04x: %02x\n", offset, data);
			m_dma_segment[offset] = data & 0x0f;
		},
		"dma_segement_w"
	);

	map(0x00a0, 0x00a1).mirror(0xe).lw8(
		[this](offs_t offset, uint8_t data)
		{
			LOGNMI("nmi_enable_w %04x: %02x\n", offset, data);
			m_nmi_enabled = BIT(data,7);
			update_nmi();
		},
		"nmi_enable_w"
	);

	// FDC Output Control Register (same as PC XT DOR)
	map(0x03f2, 0x03f3).lw8(                // B0-B1 Drive select 0-3
		[this](offs_t offset, uint8_t data) // B2 FDC Reset line
		{                   // B3 Enable FDC DMA/IRQ
			LOGFDC("FDC OCR: %02x\n", data);// B4-B7 Motor on for selected drive
			uint8_t pocr = m_ocr;
			uint8_t fid = m_ocr & 3;
			m_ocr = data;
			if ((m_ocr & 4) && m_floppy_connectors[fid]) // Not in reset and there is a floppy drive attached
			{
				floppy_image_device *floppy = m_floppy_connectors[fid]->get_device(); // try to retrieve the floppy
				if (floppy)
				{
					LOGFDC(" - Motor %s for drive %d\n", (m_ocr & (0x10 << fid)) ? "ON" : "OFF", fid);
					floppy->mon_w(!(m_ocr & (0x10 << fid)));
					LOGFDC(" - Setting a floppy for drive %d\n", fid);
					m_fdc->set_floppy((m_ocr & (0x10 << fid)) ? floppy : nullptr);
				}
			}
			if (((pocr ^ m_ocr) & 4) && (m_ocr & 4) == 0) // If FDC reset state bit has changed to low then reset the FDC
				m_fdc->reset();
			check_fdc_irq();
			check_fdc_drq();
		},
		"ocr_w"
	);

	map(0x03f4, 0x03f5).m(m_fdc, FUNC(i8272a_device::map));

	map(0x03bc, 0x03be).lrw8(
		[this](offs_t offset, uint8_t mem_mask) -> uint8_t
		{
			uint8_t data = m_lpt->read(offset);
			LOGLPT("LPT read offset %02x: %02x\n", offset, data);
			return data;
		},
		"lpt_r",
		[this](offs_t offset, uint8_t data)
		{
			LOGLPT("LPT write offset %02x: %02x\n", offset, data);
			m_lpt->write(offset, data);
		},
		"lpt_w"
	);

	map(0x03f8, 0x03ff).rw(m_uart, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
}

void epc_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->size() - 1, m_ram->pointer());

	std::fill_n(&m_dma_segment[0], 4, 0);

	save_item(NAME(m_dma_segment));
	save_item(NAME(m_dma_active));
	save_item(NAME(m_tc));
	save_item(NAME(m_txd));
	save_item(NAME(m_rxrdy));
	save_item(NAME(m_int));
	save_item(NAME(m_dreq0_ck));
	save_item(NAME(m_ppi_portb));
	save_item(NAME(m_8251rxtx_clk_state));
	save_item(NAME(m_kbdclk_state));
	save_item(NAME(m_kbdclk));
	save_item(NAME(m_8251dtr_state));
	save_item(NAME(m_nmi_enabled));
	save_item(NAME(m_8087_int));
	save_item(NAME(m_parer_int));
	save_item(NAME(m_iochck_int));
	save_item(NAME(m_pc4));
	save_item(NAME(m_pc5));
	save_item(NAME(m_ocr));
	save_item(NAME(m_irq));
	save_item(NAME(m_drq));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_drq));

	m_leds.resolve();
}

void epc_state::machine_reset()
{
	m_dma_active = 0;
	m_tc = false;
	m_txd = false;
	m_rxrdy = false;
	m_int = 1;
	m_dreq0_ck = true;
	m_ppi_portb = 0;
	m_8251rxtx_clk_state = 0;
	m_kbdclk_state = 0;
	m_kbdclk = 0;
	m_8251dtr_state = 1;
	m_nmi_enabled = 0;
	m_8087_int = 0;
	m_parer_int = 0;
	m_iochck_int = 0;
	m_pc4 = 0;
	m_pc5 = 0;
	m_ocr = 0;
	m_irq = 0;
	m_drq = 0;
	m_fdc_irq = 0;
	m_fdc_drq = 0;

	m_keyboard->rst_line_w(ASSERT_LINE);
	m_kbd8251->write_cts(0); // Tied to GND
}

void epc_state::init_epc()
{
	/* Keyboard UART Rxc/Txc is 19.2 kHz from x960 divider */
	m_kbdclk_timer = timer_alloc(FUNC(epc_state::rxtxclk_w), this);
	m_kbdclk_timer->adjust(attotime::from_hz(XTAL(18'432'000) / 960) / 2);
}

TIMER_CALLBACK_MEMBER(epc_state::rxtxclk_w)
{
	m_kbd8251->write_rxc(m_8251rxtx_clk_state);
	m_kbd8251->write_txc(m_8251rxtx_clk_state);

	// The EPC PCB has an option to support a custom receive clock for the INS8250 apart from the TX clock through a mux controlled
	// by the DTR pin of the I8251. The ins8250 device doesn't support RCLK as it is considerd implicitly as the same as BAUDOUT
	// First attempt to support this in INS8250 by lifting out the BRG from deserial was reverted due to lots of regressions.
	// We probably need to remove diserial dependencies completely from ins8250 or implement BRG hooks in diserial.cpp.
	// if (!m_8251dtr_state) m_uart->rclk_w(m_8251rxtx_clk_state); // TODO: fix RCLK support in INS8250

	m_8251rxtx_clk_state = !m_8251rxtx_clk_state;

	// If CLK signal is jumpered in instead of reset signal for the keyboard
	if ((m_io_s21->read() & 0x01) == 0x01)
	{
		if (m_kbdclk++ >= 4) // Frequncy is taken out of the same divider as the rxtx clock but 2 steps later
		{
			m_keyboard->rst_line_w(m_kbdclk_state);
			m_kbdclk = 0;
			m_kbdclk_state = !m_kbdclk_state;
		}
	}

	/* Keyboard UART Rxc/Txc is 19.2 kHz from x960 divider ( 15 (74ls161) * 4 (74ls393.1) * 16 (74ls393) ) */
	m_kbdclk_timer->adjust(attotime::from_hz(XTAL(18'432'000) / 960) / 2);
}

template <int Channel>
uint8_t epc_state::epc_dma8237_io_r(offs_t offset)
{
	LOGDMA("epc_dma8237_io_r: %d\n", Channel);
	if (Channel == 2)
		return m_fdc->dma_r();
	else
		return m_isabus->dack_r(Channel);
}

template <int Channel>
void epc_state::epc_dma8237_io_w(offs_t offset, uint8_t data)
{
	LOGDMA("epc_dma8237_io_w: %d - %02x\n", Channel, data);
	if (Channel == 2)
		m_fdc->dma_w(data);
	else
		m_isabus->dack_w(Channel, data);
}

template <int Channel>
void epc_state::epc_dack_w(int state)
{
	LOGDMA("epc_dack_w: %d - %d\n", Channel, state);

	m_isabus->dack_line_w(Channel, state);

	if (!state)
	{
		m_dma_active |= 1 << Channel;
		if (Channel == 0)
			m_dma8237a->dreq0_w(0);
		if (m_tc)
			m_isabus->eop_w(Channel, ASSERT_LINE);
	}
	else
	{
		m_dma_active &= ~(1 << Channel);
		if (m_tc)
			m_isabus->eop_w(Channel, CLEAR_LINE);
	}
}

void epc_state::dma_tc_w(int state)
{
	m_tc = (state == ASSERT_LINE);
	for (int channel = 0; channel < 4; channel++)
	{
		if (BIT(m_dma_active, channel))
		{
			LOGDMA("dma_tc_w ch %d: %d\n", channel, state);
			m_isabus->eop_w(channel, state);
		}
	}

	// Special treatment for on board FDC
	if (BIT(m_dma_active, 2))
	{
		m_fdc->tc_w(0);
	}
	else
	{
		m_fdc->tc_w(1);
	}
}

void epc_state::dreq0_ck_w(int state)
{
	if (state && !m_dreq0_ck && !BIT(m_dma_active, 0))
		m_dma8237a->dreq0_w(1);

	m_dreq0_ck = state;
}

void epc_state::speaker_ck_w(int state)
{
	m_pc5 = state;
	m_pc4 = (m_ppi_portb & 0x02) && state ? 1 : 0;
	m_speaker->level_w(m_pc4);
}

/**********************************************************
 *
 * PPI8255 interface
 *
 *
 * PORT A (not used)
 *
 * Reads of port A is shadowed by UART8251A's read register
 * gaining some compatibility with PC software. The UART8251
 * communicates with the serial keyboard and extends it with
 * write capability enabling keyboard led control as with a
 * PC AT keyboard.
 *
 * PORT B (output)
 * 0 - PB0 -             - Control signal for the sound generator (short beeps)
 * 1 - PB1 -             - Control signal for the sound generator
 * 2 - PB2 -             - Unused
 * 3 - PB3 -             - Data select for the configuration switches 0=SW1-4 1=SW5-8
 * 4 - PB4 - *           - Enable ram parity check
 * 5 - PB5 - *           - Enable expansion I/O check
 * 6 - PB6 - *           - Keyboard reset
 * 7 - PB7 -             - Reset keyboard interrupt
 *
 * PORT C
 * 0 - PC0 -         - Dipswitch SW 1/5 PB3=0/PB3=1
 * 1 - PC1 -         - Dipswitch SW 2/6 PB3=0/PB3=1
 * 2 - PC2 -         - Dipswitch SW 3/7 PB3=0/PB3=1
 * 3 - PC3 -         - Dipswitch SW 4/8 PB3=0/PB3=1
 * 4 - PC4 - SPK     - Speaker/cassette data (spare in PC XT spec)
 * 5 - PC5 - OUT2    - OUT2 from 8253 (ibmpcjr compatible)
 * 6 - PC6 -
 * 7 - PC7 -
 *
 * Ericsson PC SW:
 * 1   - Not used. Must be set to OFF
 * 2   - OFF - 8087 present
 *       ON  - No 8087 present *)
 * 3   - Not Used. Don't care but OFF *)
 * 4   - Not Used. Must be set to ON
 * 5+6 - Used to select display
 *       OFF OFF - Monochrome HR graphics monitor 3111 installed + 1020 color secondary monitor
 *       ON  OFF - Monochrome HR graphics monitor 3111 installed + optional 1020 color main monitor *)
 *       OFF ON  - Not used
 *       ON  ON  - Not used
 * 7+8 - Used to select number of disk drives
 *       OFF OFF - Not used
 *       ON  OFF - Not used
 *       OFF ON  - two disk drives, system units 1030-1 and 1030-2
 *       ON  ON  - one disk drive, system units 1030-3, 1030-4, 1031-1 and 1031-2
 *
 *           *)  - Factory settings
 *
 **********************************************************/

uint8_t epc_state::ppi_portc_r()
{
	uint8_t data;

	// Read 4 configurations dip switches depending on PB3
	data = (m_io_dsw->read() >> ((m_ppi_portb & 0x08) ? 4 : 0) & 0x0f);

	data |= (m_pc4 ? 1U << 4 : 0); // Feedback from gated speaker beep
	data |= (m_pc5 ? 1U << 5 : 0); // Feedback from timer source for speaker beep

	LOGPPI("PPI Port C read: %02x\n", data);

	return data;
}

void epc_state::ppi_portb_w(uint8_t data)
{
	LOGPPI("PPI Port B write: %02x\n", data);
	LOGPPI(" PB0 - Enable beeper             : %d\n", (data & 0x01)  ? 1 : 0);
	LOGPPI(" PB1 - Beeper data               : %d\n", (data & 0x02)  ? 1 : 0);
	LOGPPI(" PB2 - Unused                    : %d\n", (data & 0x04)  ? 1 : 0);
	LOGPPI(" PB3 - Port C dip switch select  : %d\n", (data & 0x08)  ? 1 : 0);
	LOGPPI(" PB4 - RAM parity enable         : %d\n", (data & 0x10)  ? 1 : 0);
	LOGPPI(" PB5 - ISA error checking enable : %d\n", (data & 0x20)  ? 1 : 0);
	LOGPPI(" PB6 - Reset keyboard            : %d\n", (data & 0x40)  ? 1 : 0);
	LOGPPI(" PB7 - Reset keyboard interrupt  : %d\n", (data & 0x80)  ? 1 : 0);

	uint8_t changed = m_ppi_portb ^ data;

	m_ppi_portb = data;

	if (changed & 0x40)
	{
		if ((m_io_s21->read() & 0x01) == 0x00)
		{
			if (m_ppi_portb & 0x40)
			{
				LOGKBD("PB6 set, clearing Keyboard RESET\n");
				m_keyboard->rst_line_w(CLEAR_LINE);
			}
			else
			{
				LOGKBD("PB6 cleared, asserting Keyboard RESET\n");
				m_keyboard->rst_line_w(ASSERT_LINE);
			}
		}
	}

	if (changed & m_ppi_portb & 0x80)
	{
		LOGIRQ("PB7 set, clearing IRQ1 and releasing HOLD\n");
		m_pic8259->ir1_w(CLEAR_LINE);
		m_keyboard->hold_w(ASSERT_LINE);
	}
}

void epc_state::int_w(int state)
{
	if (m_int != state)
	{
		LOGIRQ("int_w: %d\n", state);
		m_int = state;
		m_maincpu->set_input_line(0, m_int);
	}
}

static void epc_isa8_cards(device_slot_interface &device)
{
	device.option_add("epc_mda", ISA8_EPC_MDA);
	device.option_add("ega", ISA8_EGA);
	device.option_add("epc_twib", ISA8_EIS_TWIB);
	// device.option_add("epc_hdc1065", ISA8_EPC_HDC1065);
	// device.option_add("epc_mb1080", ISA8_EPC_MB1080);
}

static void epc_sd_floppies(device_slot_interface &device)
{
	device.option_add("525sd", FLOPPY_525_SD);
}

void epc_state::epc(machine_config &config)
{
	config.set_default_layout(layout_epc);

	// CPU
	I8088(config, m_maincpu, XTAL(14'318'181) / 3.0); // TWE crystal marked X1 verified divided through a 82874
	m_maincpu->set_addrmap(AS_PROGRAM, &epc_state::epc_map);
	m_maincpu->set_addrmap(AS_IO, &epc_state::epc_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));
	m_maincpu->esc_opcode_handler().set("fpu8087", FUNC(i8087_device::insn_w));
	m_maincpu->esc_data_handler().set("fpu8087", FUNC(i8087_device::addr_w));

	i8087_device &i8087(I8087(config, "fpu8087", XTAL(14'318'181) / 3.0));
	i8087.set_space_88(m_maincpu, AS_PROGRAM);
	i8087.irq().set([this](bool state)
	{
		LOGFPU("8087 INT: %d\n", state);
		m_8087_int = state;
		update_nmi();
	});
	i8087.busy().set_inputline(m_maincpu, INPUT_LINE_TEST);

	// DMA
	AM9517A(config, m_dma8237a, XTAL(14'318'181) / 3.0); // TWE crystal marked X1 verified
	m_dma8237a->out_hreq_callback().set(FUNC(epc_state::epc_dma_hrq_changed));
	m_dma8237a->out_eop_callback().set(FUNC(epc_state::dma_tc_w));
	m_dma8237a->in_memr_callback().set(FUNC(epc_state::epc_dma_read_byte));
	m_dma8237a->out_memw_callback().set(FUNC(epc_state::epc_dma_write_byte));
	m_dma8237a->in_ior_callback<1>().set(FUNC(epc_state::epc_dma8237_io_r<1>));
	m_dma8237a->in_ior_callback<2>().set(FUNC(epc_state::epc_dma8237_io_r<2>));
	m_dma8237a->in_ior_callback<3>().set(FUNC(epc_state::epc_dma8237_io_r<3>));
	m_dma8237a->out_iow_callback<0>().set(FUNC(epc_state::epc_dma8237_io_w<0>));
	m_dma8237a->out_iow_callback<1>().set(FUNC(epc_state::epc_dma8237_io_w<1>));
	m_dma8237a->out_iow_callback<2>().set(FUNC(epc_state::epc_dma8237_io_w<2>));
	m_dma8237a->out_iow_callback<3>().set(FUNC(epc_state::epc_dma8237_io_w<3>));
	m_dma8237a->out_dack_callback<0>().set(FUNC(epc_state::epc_dack_w<0>));
	m_dma8237a->out_dack_callback<1>().set(FUNC(epc_state::epc_dack_w<1>));
	m_dma8237a->out_dack_callback<2>().set(FUNC(epc_state::epc_dack_w<2>));
	m_dma8237a->out_dack_callback<3>().set(FUNC(epc_state::epc_dack_w<3>));

	// TTL-level serial keyboard callback
	EISPC_KB(config, m_keyboard);
	m_keyboard->txd_cb().set([this](bool state)
	{
		LOGBITS("KBD->EPC: %d\n", state);
		m_kbd8251->write_rxd(state);
	});
	m_keyboard->caps_cb().set(  [this](bool state){ m_leds[0] = state; });
	m_keyboard->num_cb().set(   [this](bool state){ m_leds[1] = state; });
	m_keyboard->scroll_cb().set([this](bool state){ m_leds[2] = state; });

	// Keyboard USART
	I8251( config, m_kbd8251, XTAL(14'318'181) / 6.0 ); // TWE crystal marked X1 verified divided through a 82874

	m_kbd8251->txd_handler().set([this](bool state)
	{
		if (m_txd != state)
		{
			LOGBITS("EPC->KBD: %d\n", state);
			m_txd = state;
			m_keyboard->rxd_w(m_txd);
		}
	});

	m_kbd8251->rxrdy_handler().set([this](bool state)
	{
		m_rxrdy = state;
		LOGKBD("KBD RxRdy: %d HOLD: %d\n", m_rxrdy ? 1 : 0, m_rxrdy ? 0 : 1);
		m_keyboard->hold_w(!m_rxrdy);
		if (m_rxrdy)
		{
			LOGIRQ("RxRdy set, asserting IRQ1\n");
			m_pic8259->ir1_w(ASSERT_LINE); // Cleared by setting PB7
		}
	});
	m_kbd8251->dtr_handler().set([this](bool state) // Controls RCLK for INS8250, either 19.2KHz or INS8250 BAUDOUT
	{
		LOGCOM("KBD DTR: %d\n", state ? 1 : 0);
		m_8251dtr_state = state;
	});

	// Interrupt Controller
	PIC8259(config, m_pic8259);
	m_pic8259->out_int_callback().set(FUNC(epc_state::int_w));

	// Parallel port
	I8255A(config, m_ppi8255);
	m_ppi8255->out_pa_callback().set([this] (uint8_t data) { LOGPPI("PPI: write %02x to unused Port A\n", data); } ); // Port A is not used
	m_ppi8255->out_pb_callback().set(FUNC(epc_state::ppi_portb_w));
	m_ppi8255->in_pc_callback().set(FUNC(epc_state::ppi_portc_r));

	// system board Parallel port
	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set([this](int state)
	{   // Jumper field J10 decides what IRQ to pull
		if ((m_io_j10->read() & 0x03) == 0x01) { LOGIRQ("LPT IRQ2: %d\n", state); m_pic8259->ir2_w(state); }
		if ((m_io_j10->read() & 0x0c) == 0x04) { LOGIRQ("LPT IRQ3: %d\n", state); m_pic8259->ir3_w(state); }
		if ((m_io_j10->read() & 0x30) == 0x10) { LOGIRQ("LPT IRQ4: %d\n", state); m_pic8259->ir4_w(state); }
		if ((m_io_j10->read() & 0xc0) == 0x40) { LOGIRQ("LPT IRQ7: %d\n", state); m_pic8259->ir7_w(state); } // Factory setting
	});

	// Timer
	PIT8253(config, m_pit8253);
	m_pit8253->set_clk<0>((XTAL(14'318'181) / 3.0) / 2.0 );
	m_pit8253->set_clk<1>((XTAL(14'318'181) / 3.0) / 2.0 );
	m_pit8253->set_clk<2>((XTAL(14'318'181) / 3.0) / 2.0 );
	m_pit8253->out_handler<0>().set(m_pic8259, FUNC(pic8259_device::ir0_w));
	m_pit8253->out_handler<1>().set(FUNC(epc_state::dreq0_ck_w));
	m_pit8253->out_handler<2>().set(FUNC(epc_state::speaker_ck_w));

	// Speaker
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	// ISA bus
	ISA8(config, m_isabus,  XTAL(14'318'181) / 3.0); // TEW crystal marked X1 verified
	m_isabus->set_memspace(m_maincpu, AS_PROGRAM);
	m_isabus->set_iospace(m_maincpu, AS_IO);
	m_isabus->irq2_callback().set(m_pic8259, FUNC(pic8259_device::ir2_w)); // Reserved in service manual
	m_isabus->irq3_callback().set(m_pic8259, FUNC(pic8259_device::ir3_w));
	m_isabus->irq4_callback().set(m_pic8259, FUNC(pic8259_device::ir4_w));
	m_isabus->irq5_callback().set(m_pic8259, FUNC(pic8259_device::ir5_w));
	m_isabus->irq6_callback().set(m_pic8259, FUNC(pic8259_device::ir6_w));
	m_isabus->irq7_callback().set(m_pic8259, FUNC(pic8259_device::ir7_w));
	m_isabus->drq1_callback().set(m_dma8237a, FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_dma8237a, FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_dma8237a, FUNC(am9517a_device::dreq3_w));
	m_isabus->iochck_callback().set([this] (int state)
	{
		if (m_nmi_enabled && !state && 0)
		{
			LOGNMI("IOCHCK: NMI Requested\n");
			update_nmi();
		}
	});

	ISA8_SLOT(config, "isa1", 0, m_isabus, epc_isa8_cards, "epc_mda", false);
	ISA8_SLOT(config, "isa2", 0, m_isabus, epc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa3", 0, m_isabus, epc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa4", 0, m_isabus, epc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa5", 0, m_isabus, epc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa6", 0, m_isabus, epc_isa8_cards, nullptr, false);

	// System board has 128kB memory with parity, expansion can be achieved through the
	// 128kB Memory Expansion Board 1090 and/or the 128kB Multifunction Board MB1080-001
	// and/or the 384kB MB1080-002. The MB1080 DRAM might need to be dynamically added as
	// base address and also a video memory hole is configurable.
	RAM(config, m_ram).set_default_size("128K").set_extra_options("256K, 384K, 512K, 640K");

	// FDC
	I8272A(config, m_fdc, XTAL(16'000'000) / 2, false); // TEW crystal marked X3 verified
	m_fdc->intrq_wr_callback().set([this] (int state){ m_fdc_irq = state; check_fdc_irq(); });
	m_fdc->drq_wr_callback().set([this] (int state){ m_fdc_drq = state; check_fdc_drq(); });
	FLOPPY_CONNECTOR(config, m_floppy_connectors[0], epc_sd_floppies, "525sd", floppy_image_device::default_pc_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy_connectors[1], epc_sd_floppies, "525sd", floppy_image_device::default_pc_floppy_formats);
	//SOFTWARE_LIST(config, "epc_flop_list").set_original("epc_flop");

	// system board UART
	INS8250(config, m_uart, XTAL(18'432'000) / 10); // TEW crystal marked X2 verified. TODO: Let 8051 DTR control RCLK (see above)
	m_uart->out_tx_callback().set("com1", FUNC(rs232_port_device::write_txd));
	m_uart->out_dtr_callback().set("com1", FUNC(rs232_port_device::write_dtr));
	m_uart->out_rts_callback().set("com1", FUNC(rs232_port_device::write_rts));
	m_uart->out_int_callback().set([this](int state)
	{   // Jumper field J10 decides what IRQ to pull
		if ((m_io_j10->read() & 0x03) == 0x02) { LOGCOM("UART IRQ2: %d\n", state); m_pic8259->ir2_w(state); }
		if ((m_io_j10->read() & 0x0c) == 0x08) { LOGCOM("UART IRQ3: %d\n", state); m_pic8259->ir3_w(state); }
		if ((m_io_j10->read() & 0x30) == 0x20) { LOGCOM("UART IRQ4: %d\n", state); m_pic8259->ir4_w(state); } // Factory setting
		if ((m_io_j10->read() & 0xc0) == 0x80) { LOGCOM("UART IRQ7: %d\n", state); m_pic8259->ir7_w(state); }
	});
	// m_uart->out_baudout_callback().set([this](int state){ if (m_8251dtr_state) m_uart->rclk_w(state); }); // TODO: Fix INS8250 BAUDOUT pin support

	rs232_port_device &rs232(RS232_PORT(config, "com1", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(ins8250_uart_device::rx_w));
	rs232.dcd_handler().set(m_uart, FUNC(ins8250_uart_device::dcd_w));
	rs232.dsr_handler().set(m_uart, FUNC(ins8250_uart_device::dsr_w));
	rs232.ri_handler().set(m_uart, FUNC(ins8250_uart_device::ri_w));
	rs232.cts_handler().set(m_uart, FUNC(ins8250_uart_device::cts_w));
}

void epc_state::update_nmi()
{
	if (m_nmi_enabled &&
		((m_8087_int && (m_io_dsw->read() & 0x02)) || // FPU int only if FPU is enabled by DSW2
		 (m_parer_int != 0) || // Parity error is always false as it is an emulator, at least for now
		 (m_iochck_int != 0))) // Same goes for ISA board errors
	{
		LOGNMI(" NMI Asserted\n");
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
	else
	{
		LOGNMI(" NMI Cleared\n");
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}

void epc_state::epc_dma_hrq_changed(int state)
{
	LOGDMA("epc_dma_hrq_changed %d\n", state);

	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237a->hack_w(state);
}


uint8_t epc_state::epc_dma_read_byte(offs_t offset)
{
	if ((m_dma_active & 0x0f) == 0)
	{
		LOGDMA("epc_dma_read_byte failed\n");
		return 0xff;
	}

	const int seg = (BIT(m_dma_active, 2) ? 0 : 2) | (BIT(m_dma_active, 3) ? 0 : 1);
	return m_maincpu->space(AS_PROGRAM).read_byte(offset | u32(m_dma_segment[seg]) << 16);
}

void epc_state::epc_dma_write_byte(offs_t offset, uint8_t data)
{
	if ((m_dma_active & 0x0f) == 0)
	{
		LOGDMA("epc_dma_write_byte failed\n");
		return;
	}

	const int seg = (BIT(m_dma_active, 2) ? 0 : 2) | (BIT(m_dma_active, 3) ? 0 : 1);
	m_maincpu->space(AS_PROGRAM).write_byte(offset | u32(m_dma_segment[seg]) << 16, data);
}


static INPUT_PORTS_START( epc_ports )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Not used")
	PORT_DIPSETTING(    0x00, "ON - Don't use")
	PORT_DIPSETTING(    0x01, "OFF - Factory Setting")
	PORT_DIPNAME( 0x02, 0x00, "8087 installed")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x02, DEF_STR(Yes) )
	PORT_DIPNAME( 0x04, 0x04, "Not used")
	PORT_DIPSETTING(    0x00, "ON - Don't care")
	PORT_DIPSETTING(    0x04, "OFF - Factory Setting")
	PORT_DIPNAME( 0x08, 0x00, "Not used")
	PORT_DIPSETTING(    0x00, "ON - Factory Setting")
	PORT_DIPSETTING(    0x08, "OFF - Don't use")
	PORT_DIPNAME( 0x30, 0x30, "Main monitor")
	PORT_DIPSETTING(    0x00, "Not used" )
	PORT_DIPSETTING(    0x10, "Optional 1020 color" )
	PORT_DIPSETTING(    0x20, "Not used" )
	PORT_DIPSETTING(    0x30, "3111 HR Monochrome" )
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "Not used" )
	PORT_DIPSETTING(    0xc0, "Not used" )

	PORT_START("J10") // Jumper area, field 0=no jumper 1=LPT 2=COM 3=n/a
	PORT_DIPNAME(0x03, 0x00, "IRQ2")
	PORT_DIPSETTING(0x00, "no jumper")
	PORT_DIPSETTING(0x01, "LPT")
	PORT_DIPSETTING(0x02, "COM")
	PORT_DIPNAME(0x0c, 0x00, "IRQ3")
	PORT_DIPSETTING(0x00, "no jumper")
	PORT_DIPSETTING(0x04, "LPT")
	PORT_DIPSETTING(0x08, "COM")
	PORT_DIPNAME(0x30, 0x20, "IRQ4")
	PORT_DIPSETTING(0x00, "no jumper")
	PORT_DIPSETTING(0x10, "LPT")
	PORT_DIPSETTING(0x20, "COM")
	PORT_DIPNAME(0xc0, 0x40, "IRQ7")
	PORT_DIPSETTING(0x00, "no jumper")
	PORT_DIPSETTING(0x40, "LPT")
	PORT_DIPSETTING(0x80, "COM")

	PORT_START("S21") // Jumper 0=PB6 reset, 1=KBCLK 4.8kHz - what to send to keyboard pin 3
	PORT_DIPNAME(0x01, 0x00, "Keyboard Clock/Reset pin")
	PORT_DIPSETTING(0x00, "PB6")
	PORT_DIPSETTING(0x01, "4.8kHz") // This setting is apparantly for another keyboard, currently unknown
INPUT_PORTS_END

ROM_START( epc )
	ROM_REGION(0x10000,"bios", 0)
	ROM_DEFAULT_BIOS("p860110")
	ROM_SYSTEM_BIOS(0, "p840705", "P840705")
	ROMX_LOAD("ericsson_8088.bin", 0xe000, 0x2000, CRC(3953c38d) SHA1(2bfc1f1d11d0da5664c3114994fc7aa3d6dd010d), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "p860110", "P860110")
	ROMX_LOAD("epcbios1.bin",  0xe000, 0x02000, CRC(79a83706) SHA1(33528c46a24d7f65ef5a860fbed05afcf797fc55), ROM_BIOS(1))
	ROMX_LOAD("epcbios2.bin",  0xa000, 0x02000, CRC(3ca764ca) SHA1(02232fedef22d31a641f4b65933b9e269afce19e), ROM_BIOS(1))
	ROMX_LOAD("epcbios3.bin",  0xc000, 0x02000, CRC(70483280) SHA1(b44b09da94d77b0269fc48f07d130b2d74c4bb8f), ROM_BIOS(1))
ROM_END

} // anonymous namespace


COMP( 1985, epc,     0,      0,      epc,     epc_ports, epc_state, init_epc,    "Ericsson Information System",     "Ericsson PC" ,          0)
//COMP( 1985, eppc,   ibm5150, 0,  pccga,         pccga,  pc_state, empty_init,    "Ericsson Information System",     "Ericsson Portable PC",  MACHINE_NOT_WORKING )
