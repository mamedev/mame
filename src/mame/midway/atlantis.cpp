// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Midway "Atlantis" hardware

    skeleton by R. Belmont

    Games supported:
        * Midway Skins Game
        * Midway Skins Game Tournament Edition (not dumped)
        * Midway Swingers Tour (not dumped)

    Hardware overview:
        * VR4310 CPU (similar to the N64's VR4300)
        * VR4373 "Nile 3" system controller / PCI bridge
        * CMD 646U2 Ultra DMA IDE controller
        * M4T28-8R128H1 TimeKeeper RTC/CMOS
        * PLX PCI9050 Bus Target Interface Chip (interfaces ISA-style designs to PCI)
        * Midway ZeusII-series custom video
        * Actiontec PM560LKI PCI Data/Fax Modem (PCI\VEN_11C1&DEV_0480&SUBSYS_04801668)
        * TL16c552 dual UART
        * ADSP-2181 based DCS2 audio (unclear which variant)
        * Micron MT48LC1M16A1 1Mx16 SDRAM, 2X (4MB) for audio
        * ICS AV9110 Serially Programmable Frequency Generator.  Programmed through ADSP FL0,FL1,FL2 pins.
        * Cirrus Logic CS4338 16 bit stereo audio serial DAC, PCB has space for 3 chips (6-channels), only 1 is populated
        * Maxim MAX192 8 channel 10 bit serial ADC
        * PIC16C57 (protection? serial #?)
        * Quantum Fireball CX 6.4GB IDE HDD (C/H/S 13328/15/63)

    TODO:
        * PCI peripherals

    NOTES:
        * Skins Game is Linux based; the kernel is a customized 2.2.10 build of Linux-MIPS with Midway PCBs
          added as recognized system types

***************************************************************************/

#include "emu.h"

#include "midwayic.h"

#include "dcs.h"

#include "bus/rs232/rs232.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/mips/mips3.h"
#include "machine/idectrl.h"
#include "machine/ins8250.h"
#include "machine/pci-ide.h"
#include "machine/pci.h"
#include "machine/pci9050.h"
#include "machine/terminal.h"
#include "machine/timekpr.h"
#include "machine/vrc4373.h"
#include "video/zeus2.h"

#include "emupal.h"
#include "speaker.h"

#define LOG_RTC         (1U << 1)
#define LOG_PORT        (1U << 2)
#define LOG_IRQ         (1U << 3)
#define LOG_IRQ_VERBOSE (1U << 4)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

// Reset bits
#define RESET_IOASIC        0x01
#define RESET_ROMBUS        0x02
#define RESET_ZEUS          0x04
#define RESET_ROMBUS_IN     0x08
#define RESET_IDE           0x10
#define RESET_DUART         0x20

// IRQ  Status Bits
#define IOASIC_IRQ_SHIFT    0
#define ROMBUS_IRQ_SHIFT    1
#define ZEUS_IRQ_SHIFT      2
#define DUART_IRQ_SHIFT     3
// PCI Slot and USB Chip
#define PCI_IRQ_SHIFT       4
#define WDOG_IRQ_SHIFT      5
#define A2D_IRQ_SHIFT       6
#define VBLANK_IRQ_SHIFT    7

/* static interrupts */
#define GALILEO_IRQ_NUM         MIPS3_IRQ0
#define IDE_IRQ_NUM             MIPS3_IRQ4

// From linux header
// IDSEL = AD17, USB chip
// IDSEL = AD18, CMD646 chip
// IDSEL = AD19, onboard PCI slot
// IDSEL = AD20, Nile 3 chip
// IDSEL = AD21, PLX9050 chip
// IDSEL = AD22, PCI expansion
// IDSEL = AD23, PCI expansion
// IDSEL = AD24, PCI expansion
#define PCI_ID_IDE      "pci:08.0"
#define PCI_ID_NILE     "pci:0a.0"
#define PCI_ID_9050     "pci:0b.0"

#define DEBUG_CONSOLE   (0)

class atlantis_state : public driver_device
{
public:
	atlantis_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_zeus(*this, "zeus2"),
		m_dcs(*this, "dcs"),
		m_ioasic(*this, "ioasic"),
		m_uart0(*this, "uart0"),
		m_uart1(*this, "uart1"),
		m_uart2(*this, "uart2"),
		m_ide(*this, PCI_ID_IDE),
		m_rtc(*this, "rtc"),
		m_io_analog(*this, "AN.%u", 0)
	{
		std::fill(std::begin(board_ctrl), std::end(board_ctrl), 0);
	}

	void mwskins(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(adc_ready);

private:
	required_device<mips3_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	required_device<zeus2_device> m_zeus;
	required_device<dcs_audio_device> m_dcs;
	required_device<midway_ioasic_device> m_ioasic;
	optional_device<generic_terminal_device> m_uart0;
	required_device<ns16550_device> m_uart1;
	required_device<ns16550_device> m_uart2;
	required_device<ide_pci_device> m_ide;
	required_device<m48t37_device> m_rtc;
	optional_ioport_array<8> m_io_analog;
	emu_timer *m_adc_ready_timer = nullptr;

	uint8_t cmos_r(offs_t offset);
	void cmos_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint32_t m_cmos_write_enabled = 0;
	uint32_t m_serial_count = 0;


	[[maybe_unused]] void asic_fifo_w(uint32_t data);

	uint8_t exprom_r(offs_t offset);
	void exprom_w(offs_t offset, uint8_t data);

	void user_io_output(uint32_t data);
	uint32_t user_io_input();
	int m_user_io_state = 0;

	// Board Ctrl Reg Offsets
	enum {
		PLD_REV, RESET, VSYNC_CLEAR, IRQ_MAP1, IRQ_MAP2, IRQ_MAP3,
		IRQ_EN = 7, CAUSE, STATUS, SIZE, LED, CMOS_UNLOCK, WDOG, TRACKBALL_CTL,
		CTRL_SIZE
	};
	uint32_t board_ctrl_r(offs_t offset, uint32_t mem_mask = ~0);
	void board_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t m_irq_state = 0;
	uint32_t board_ctrl[CTRL_SIZE]{};
	void update_asic_irq();

	void vblank_irq(int state);
	void zeus_irq(int state);
	void ide_irq(int state);
	void ioasic_irq(int state);
	void watchdog_irq(int state);
	void watchdog_reset(int state);

	void duart_irq_callback(int state);

	ioport_value port_mod_r();
	uint16_t port_ctrl_r(offs_t offset);
	void port_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t m_port_data = 0;
	uint16_t m_a2d_data = 0;

	uint16_t a2d_ctrl_r();
	void a2d_ctrl_w(offs_t offset, uint16_t data);

	uint16_t a2d_data_r();
	void a2d_data_w(uint16_t data);

	uint8_t parallel_r(offs_t offset);
	void parallel_w(offs_t offset, uint8_t data);

	void map0(address_map &map) ATTR_COLD;
	void map1(address_map &map) ATTR_COLD;
	void map2(address_map &map) ATTR_COLD;
	void map3(address_map &map) ATTR_COLD;
};

// Parallel Port
uint8_t atlantis_state::parallel_r(offs_t offset)
{
	uint8_t result = 0x7;
	logerror("%s: parallel_r %08x = %02x\n", machine().describe_context(), offset, result);
	return result;
}

void atlantis_state::parallel_w(offs_t offset, uint8_t data)
{
	logerror("%s: parallel_w %08x = %02x\n", machine().describe_context(), offset, data);
}

// Expansion ROM
uint8_t atlantis_state::exprom_r(offs_t offset)
{
	logerror("%s: exprom_r %08x = %02x\n", machine().describe_context(), offset, 0xff);
	return 0xff;
}

void atlantis_state::exprom_w(offs_t offset, uint8_t data)
{
	logerror("%s: exprom_w %08x = %02x\n", machine().describe_context(), offset, data);
}

// Board PLD
uint32_t atlantis_state::board_ctrl_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t newOffset = offset >> 17;
	uint32_t data = board_ctrl[newOffset];
	switch (newOffset) {
	case PLD_REV:
		// ???
		data = 0x1;
		LOGMASKED(LOG_IRQ, "%s:board_ctrl_r read from PLD_REV offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	case STATUS:
		LOGMASKED(LOG_IRQ, "%s:board_ctrl_r read from STATUS offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	default:
		LOGMASKED(LOG_IRQ, "%s:board_ctrl_r read from offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	}
	return data;
}

void atlantis_state::board_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t newOffset = offset >> 17;
	uint32_t changeData = board_ctrl[newOffset] ^ data;
	COMBINE_DATA(&board_ctrl[newOffset]);
	switch (newOffset) {
	case RESET:
		// 0x1 IOASIC Reset
		// 0x4 Zeus2 Reset
		// 0x10 IDE Reset
		if (changeData & RESET_IOASIC) {
			if ((data & RESET_IOASIC) == 0) {
				m_ioasic->ioasic_reset();
				m_dcs->reset_w(0);
			}
			else {
				m_dcs->reset_w(1);
			}
		}
		if (changeData & RESET_IDE) {
			if (!(data & RESET_IDE)) {
				logerror("%s:board_ctrl_w IDE Reset\n", machine().describe_context());
				m_ide->reset();
			}
		}
		if (changeData & RESET_ZEUS) {
			if (!(data & RESET_ZEUS)) {
				logerror("%s:board_ctrl_w Zeus Reset\n", machine().describe_context());
				m_zeus->reset();
			}
		}
		LOGMASKED(LOG_IRQ, "%s:board_ctrl_w write to RESET offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	case VSYNC_CLEAR:
		//VSYNC_IE (0x1)
		//VSYNC_POL (0x2)   off=negative true, on=positive true
		// 0x1 VBlank clear?
		if (changeData & 0x1) {
			if ((data & 0x0001) == 0) {
				board_ctrl[STATUS] &= ~(1 << VBLANK_IRQ_SHIFT);
				update_asic_irq();
			}
			else {
			}
		}
		LOGMASKED(LOG_IRQ_VERBOSE, "%s:board_ctrl_w write to CTRL_VSYNC_CLEAR offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	case IRQ_EN:
		// Zero bit will clear cause
		update_asic_irq();
		LOGMASKED(LOG_IRQ, "%s:board_ctrl_w write to IRQ_EN offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	case LED:
		{
			char digit = 'U';
			switch (board_ctrl[LED] & 0xff) {
			case 0xc0: digit = '0'; break;
			case 0xf9: digit = '1'; break;
			case 0xa4: digit = '2'; break;
			case 0xb0: digit = '3'; break;
			case 0x99: digit = '4'; break;
			case 0x92: digit = '5'; break;
			case 0x82: digit = '6'; break;
			case 0xf8: digit = '7'; break;
			case 0x80: digit = '8'; break;
			case 0x90: digit = '9'; break;
			case 0x88: digit = 'A'; break;
			case 0x83: digit = 'B'; break;
			case 0xc6: digit = 'C'; break;
			case 0xa7: digit = 'c'; break;
			case 0xa1: digit = 'D'; break;
			case 0x86: digit = 'E'; break;
			case 0x87: digit = 'F'; break;
			case 0x7f: digit = '.'; break;
			case 0xf7: digit = '_'; break;
			case 0xbf: digit = '|'; break;
			case 0xfe: digit = '-'; break;
			case 0xff: digit = 'Z'; break;
				if (0) logerror("%s: status_leds_w digit: %c %08x = %02x\n", machine().describe_context(), digit, offset, data);
			}
		}
		break;
	case CMOS_UNLOCK:
		m_cmos_write_enabled = true;
		break;
	case WDOG:
		m_rtc->watchdog_write();
		break;
	default:
		LOGMASKED(LOG_IRQ, "%s:board_ctrl_w write to offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	}
}


uint8_t atlantis_state::cmos_r(offs_t offset)
{
	uint8_t result = m_rtc->read(offset);
	// Initial RTC check expects reads to the RTC to take some time
	if (offset == 0x7ff9)
		m_maincpu->eat_cycles(30);
	else if (offset >= 0x7ff0 && offset != 0x7ff9)
		LOGMASKED(LOG_RTC, "%s:RTC read from offset %04X = %08X\n", machine().describe_context(), offset, result);
	return result;
}

void atlantis_state::cmos_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	system_time systime;
	// User I/O 0 = Allow write to cmos[0]. Serial Write Enable?
	if (offset == 0 && (m_user_io_state & 0x1)) {
		// Data written is shifted by 1 bit each time.  Maybe a serial line output?
		if (m_serial_count == 0)
			LOGMASKED(LOG_RTC, "%s: cmos_w[0] start serial %08x = %02x\n", machine().describe_context(), offset, data);
		m_serial_count++;
		if (m_serial_count == 8)
			m_serial_count = 0;
	}
	else if (m_cmos_write_enabled) {
		m_rtc->write(offset, data);
		m_cmos_write_enabled = false;
		LOGMASKED(LOG_RTC, "%s:RTC write to offset %04X = %08X & %08X\n", machine().describe_context(), offset, data, mem_mask);
	}
}

void atlantis_state::asic_fifo_w(uint32_t data)
{
	m_ioasic->fifo_w(data);
}

/*************************************
*  PCI9050 User I/O handlers
*************************************/
void atlantis_state::user_io_output(uint32_t data)
{
	m_user_io_state = data;
	logerror("atlantis_state::user_io_output m_user_io_state = %1x\n", m_user_io_state);
}

uint32_t atlantis_state::user_io_input()
{
	// user io 0: 6016 nCONFIG -- output
	// user io 1: 6016 nSTATUS -- input
	// user io 2: 6016 config done -- input

	// Set user i/o (2) Config Done
	m_user_io_state |= 1 << 2;

	// Loop user_io(0) to user_io(1)
	m_user_io_state = (m_user_io_state & ~(0x2)) | ((m_user_io_state & 1) << 1);
	if (0)
		logerror("atlantis_state::user_io_input m_user_io_state = %1x\n", m_user_io_state);
	return m_user_io_state;
}

/*************************************
*  DUART interrupt handler
*************************************/
void atlantis_state::duart_irq_callback(int state)
{
	uint32_t status_bit = 1 << DUART_IRQ_SHIFT;
	if (state && !(board_ctrl[STATUS] & status_bit)) {
		board_ctrl[STATUS] |= status_bit;
		update_asic_irq();
	}
	else if (!state && (board_ctrl[STATUS] & status_bit)) {
		board_ctrl[STATUS] &= ~status_bit;
		update_asic_irq();
	}
	logerror("atlantis_state::duart_irq_callback state = %1x\n", state);
}

/*************************************
*  Video interrupts
*************************************/
void atlantis_state::vblank_irq(int state)
{
	//logerror("%s: atlantis_state::vblank state = %i\n", machine().describe_context(), state);
	if (state) {
		board_ctrl[STATUS] |= (1 << VBLANK_IRQ_SHIFT);
	}
	else {
		board_ctrl[STATUS] &= ~(1 << VBLANK_IRQ_SHIFT);
	}
	update_asic_irq();
}

void atlantis_state::zeus_irq(int state)
{
	//logerror("%s: atlantis_state::zeus_irq state = %i\n", machine().describe_context(), state);
	if (state) {
		board_ctrl[STATUS] |= (1 << ZEUS_IRQ_SHIFT);
	}
	else {
		board_ctrl[STATUS] &= ~(1 << ZEUS_IRQ_SHIFT);
	}
	update_asic_irq();
}

/*************************************
*  IDE interrupts
*************************************/
void atlantis_state::ide_irq(int state)
{
	if (state) {
		m_maincpu->set_input_line(IDE_IRQ_NUM, ASSERT_LINE);
		m_irq_state |= (1 << IDE_IRQ_NUM);
	}
	else {
		m_maincpu->set_input_line(IDE_IRQ_NUM, CLEAR_LINE);
		m_irq_state &= ~(1 << IDE_IRQ_NUM);
	}
	LOGMASKED(LOG_IRQ, "%s: atlantis_state::ide_irq state = %i\n", machine().describe_context(), state);
}

/*************************************
*  I/O ASIC interrupts
*************************************/
void atlantis_state::ioasic_irq(int state)
{
	LOGMASKED(LOG_IRQ, "%s: atlantis_state::ioasic_irq state = %i\n", machine().describe_context(), state);
	if (state) {
		board_ctrl[STATUS] |= (1 << IOASIC_IRQ_SHIFT);
	}
	else {
		board_ctrl[STATUS] &= ~(1 << IOASIC_IRQ_SHIFT);
	}
	update_asic_irq();
}

/*************************************
*  Watchdog interrupts
*************************************/
void atlantis_state::watchdog_irq(int state)
{
	LOGMASKED(LOG_IRQ, "%s: atlantis_state::watchdog_irq state = %i\n", machine().describe_context(), state);
	if (state) {
		board_ctrl[STATUS] |= (1 << WDOG_IRQ_SHIFT);
	}
	else {
		board_ctrl[STATUS] &= ~(1 << WDOG_IRQ_SHIFT);
	}
	update_asic_irq();
}

/*************************************
*  Watchdog Reset
*************************************/
void atlantis_state::watchdog_reset(int state)
{
	if (state) {
		printf("atlantis_state::watchdog_reset!!!\n");
		logerror("atlantis_state::watchdog_reset!!!\n");
		machine().schedule_soft_reset();
	}
}

/*************************************
*  Programmable interrupt control
*************************************/
void atlantis_state::update_asic_irq()
{
	// Update cause register
	board_ctrl[CAUSE] = board_ctrl[IRQ_EN] & board_ctrl[STATUS];
	// Check the MAP1, MAP2, and MAP3 and signal interrupt
	for (int irqIndex = 0; irqIndex < 3; irqIndex++) {
		uint32_t causeBits = board_ctrl[CAUSE] & board_ctrl[IRQ_MAP1 + irqIndex];
		uint32_t currState = m_irq_state & (2 << irqIndex);
		if (causeBits && !currState) {
			m_maincpu->set_input_line(MIPS3_IRQ1 + irqIndex, ASSERT_LINE);
			m_irq_state |= (2 << irqIndex);
			LOGMASKED(LOG_IRQ, "atlantis_state::update_asic_irq Asserting IRQ(%d) CAUSE = %02X\n", irqIndex, board_ctrl[CAUSE]);
		}
		else if (!(causeBits) && currState) {
			m_maincpu->set_input_line(MIPS3_IRQ1 + irqIndex, CLEAR_LINE);
			m_irq_state &= ~(2 << irqIndex);
			LOGMASKED(LOG_IRQ, "atlantis_state::update_asic_irq Clearing IRQ(%d) CAUSE = %02X\n", irqIndex, board_ctrl[CAUSE]);
		}
	}
}
/*************************************
*  I/O Port control
*************************************/
uint16_t atlantis_state::port_ctrl_r(offs_t offset)
{
	uint32_t newOffset = offset >> 17;
	uint32_t result = m_port_data;
	LOGMASKED(LOG_PORT, "%s: port_ctrl_r newOffset = %02X data = %08X\n", machine().describe_context(), newOffset, result);
	return result;
}

void atlantis_state::port_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint32_t newOffset = offset >> 17;

	switch (newOffset) {
	case 1:
	{
		uint32_t bits = ioport("KEYPAD")->read();
		m_port_data = 0;
		if (!(data & 0x8))
			m_port_data = bits & 7; // Row 0
		else if (!(data & 0x10))
			m_port_data = (bits >> 4) & 7; // Row 1
		else if (!(data & 0x20))
			m_port_data = (bits >> 8) & 7; // Row 2
		else if (!(data & 0x40))
			m_port_data = (bits >> 12) & 7; // Row 3
		LOGMASKED(LOG_PORT, "%s: port_ctrl_w Keypad Row Sel = %04X bits = %08X\n", machine().describe_context(), data, bits);
		break;
	}
	default:
		LOGMASKED(LOG_PORT, "%s: port_ctrl_w write to offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	}
}

/*************************************
*  A2D
*************************************/
#define A2D_CTRL_COMPLETE   0x1
#define A2D_CTRL_ENABLE     0x2
#define A2D_CTRL_SINGLEND   0x4
#define A2D_CTRL_UNIPOLAR   0x8
#define A2D_CTRL_CHAN_SHIFT 4
#define A2D_CTRL_CHAN_MASK  0x70
#define A2D_CTRL_START      0x80

uint16_t atlantis_state::a2d_ctrl_r()
{
	return A2D_CTRL_COMPLETE;
}

void atlantis_state::a2d_ctrl_w(offs_t offset, uint16_t data)
{
	int index = (data & A2D_CTRL_CHAN_MASK) >> A2D_CTRL_CHAN_SHIFT;
	m_a2d_data = (m_io_analog[index].read_safe(0));
	if (board_ctrl[IRQ_EN] & (1 << A2D_IRQ_SHIFT)) {
		// Set adc ready timer to fire
		m_adc_ready_timer->adjust(attotime::from_usec(5));
	}
	//logerror("a2d_ctrl_w: offset = %08x index = %d data = %04x\n", offset, index, data);
}

uint16_t atlantis_state::a2d_data_r()
{
	// Clear interrupt if enabled
	if (board_ctrl[IRQ_EN] & (1 << A2D_IRQ_SHIFT)) {
		board_ctrl[STATUS] &= ~(1 << A2D_IRQ_SHIFT);
		update_asic_irq();
	}
	return m_a2d_data;
}

void atlantis_state::a2d_data_w(uint16_t data)
{

}

/*************************************
*  Machine start
*************************************/
void atlantis_state::machine_start()
{
	/* set the fastest DRC options */
	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS | MIPS3DRC_STRICT_VERIFY | MIPS3DRC_EXTRA_INSTR_CHECK);

	// Allocate adc timer
	m_adc_ready_timer = timer_alloc(FUNC(atlantis_state::adc_ready), this);

	// Save states
	save_item(NAME(m_cmos_write_enabled));
	save_item(NAME(m_serial_count));
	save_item(NAME(m_user_io_state));
	save_item(NAME(m_irq_state));
	save_item(NAME(board_ctrl));
	save_item(NAME(m_port_data));
	save_item(NAME(m_a2d_data));

}

/*************************************
*  Machine init
*************************************/
void atlantis_state::machine_reset()
{
	m_dcs->reset_w(0);
	m_dcs->reset_w(1);
	m_user_io_state = 0;
	m_cmos_write_enabled = false;
	m_serial_count = 0;
	m_irq_state = 0;
	memset(board_ctrl, 0, sizeof(board_ctrl));
	m_adc_ready_timer->reset();
}

/*************************************
*  Timer
*************************************/
TIMER_CALLBACK_MEMBER(atlantis_state::adc_ready)
{
	// ADC Ready Timer
	board_ctrl[STATUS] |= (1 << A2D_IRQ_SHIFT);
	update_asic_irq();
}

/*************************************
 *  Address Maps
 *************************************/
void atlantis_state::map0(address_map &map)
{
	map(0x00000000, 0x0001ffff).rw(FUNC(atlantis_state::cmos_r), FUNC(atlantis_state::cmos_w)).umask32(0x000000ff);
	//map(0x00080000, 0x000?0000).rw(FUNC(atlantis_state::zeus debug_r), FUNC(atlantis_state::zeus debug_w));
	map(0x00100000, 0x0010001f).rw(m_uart1, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w)).umask32(0x000000ff); // Serial UART1 (TL16C552 CS0)
	map(0x00180000, 0x0018001f).rw(m_uart2, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w)).umask32(0x000000ff); // Serial UART2 (TL16C552 CS1)
	map(0x00200000, 0x0020001f).rw(FUNC(atlantis_state::parallel_r), FUNC(atlantis_state::parallel_w)).umask32(0x000000ff); // Parallel UART (TL16C552 CS2)
	map(0x00400000, 0x007fffff).rw(FUNC(atlantis_state::exprom_r), FUNC(atlantis_state::exprom_w)).umask32(0x000000ff); // EXPROM
	map(0x00800000, 0x00f00003).rw(FUNC(atlantis_state::board_ctrl_r), FUNC(atlantis_state::board_ctrl_w));
	//map(0x00d80000, 0x00d80003).rw(FUNC(atlantis_state::status_leds_r, FUNC(atlantis_state::status_leds_w));
	//map(0x00e00000, 0x00e00003).rw(FUNC(atlantis_state::cmos_protect_r, FUNC(atlantis_state::cmos_protect_w));
	//map(0x00e80000, 0x00e80003).noprw(); // Watchdog
	//map(0x00f00000, 0x00f00003).noprw(); // Trackball ctrl
	}

void atlantis_state::map1(address_map &map)
{
	map(0x00000000, 0x0000003f).rw(m_ioasic, FUNC(midway_ioasic_device::read), FUNC(midway_ioasic_device::write));
	map(0x00200000, 0x00200003).w(m_ioasic, FUNC(midway_ioasic_device::fifo_full_w));
	map(0x00400000, 0x00400003).w(m_dcs, FUNC(dcs_audio_device::dsio_idma_addr_w));
	map(0x00600000, 0x00600003).rw(m_dcs, FUNC(dcs_audio_device::dsio_idma_data_r), FUNC(dcs_audio_device::dsio_idma_data_w));
	map(0x00800000, 0x00900003).rw(FUNC(atlantis_state::port_ctrl_r), FUNC(atlantis_state::port_ctrl_w)).umask32(0x0000ffff);
	//map(0x00880000, 0x00880003) // AUX Output Initial write 0000fff0, follow by sequence ffef, ffdf, ffbf, fff7. Row Select?
	//map(0x00900000, 0x00900003) // AUX Input Read once before each sequence write to 0x00880000. Code checks bits 0,1,2. Keypad?
	map(0x00980000, 0x00980001).rw(FUNC(atlantis_state::a2d_ctrl_r), FUNC(atlantis_state::a2d_ctrl_w)); // A2D Control Read / Write.  Bytes written 0x8f, 0xcf. Code if read 0x1 then read 00a00000.
	map(0x00a00000, 0x00a00001).rw(FUNC(atlantis_state::a2d_data_r), FUNC(atlantis_state::a2d_data_w)); // A2D Data
	//map(0x00a80000, 0x00a80003) // Trackball Chan 0 16 bits
	//map(0x00b00000, 0x00b00003) // Trackball Chan 1 16 bits
	//map(0x00b80000, 0x00b80003) // Trackball Error 16 bits
	//map(0x00c00000, 0x00c00003) // Trackball Pins 16 bits
}

void atlantis_state::map2(address_map &map)
{
	map(0x00000000, 0x000001ff).rw(m_zeus, FUNC(zeus2_device::zeus2_r), FUNC(zeus2_device::zeus2_w));
}

void atlantis_state::map3(address_map &map)
{
	//map(0x000000, 0xffffff) ROMBUS
}

/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( mwskins )
	PORT_START("DIPS")
	PORT_DIPNAME(0x0003, 0x0003, "Boot Mode")
	PORT_DIPSETTING(0x0003, "Run Game")
	PORT_DIPSETTING(0x0002, "Boot Disk Based Self Test")
	PORT_DIPSETTING(0x0001, "Boot EEPROM Based Self Test")
	PORT_DIPSETTING(0x0000, "Run Interactive Tests")
	PORT_DIPNAME(0x0004, 0x0004, "Console Enable")
	PORT_DIPSETTING(0x0004, DEF_STR(No))
	PORT_DIPSETTING(0x0000, DEF_STR(Yes))
	PORT_DIPUNUSED(0xfff8, 0xfff8)

	PORT_START("SYSTEM")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_TILT)
	PORT_SERVICE_NO_TOGGLE(0x0010, IP_ACTIVE_LOW)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_COIN4)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_START3)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_START4)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP)
	PORT_BIT(0x6000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_BILL1)

	PORT_START("IN1")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1) PORT_NAME("Camera")
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1) PORT_NAME("Club Up")
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1) PORT_NAME("Aim Right")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2) PORT_NAME("Aim Left")
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2) PORT_NAME("Club Down")
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	//PORT_BIT(0x0007, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(atlantis_state, port_mod_r)
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("AN.0")
	PORT_BIT(0x3ff, 0x200, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(50) PORT_PLAYER(1)

	PORT_START("AN.4")
	PORT_BIT(0x3ff, 0x200, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(50) PORT_PLAYER(1)

	PORT_START("KEYPAD")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)   /* keypad 1 */
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD)   /* keypad 2 */
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)   /* keypad 3 */
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD)   /* keypad 4 */
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)   /* keypad 5 */
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CODE(KEYCODE_6_PAD)   /* keypad 6 */
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)   /* keypad 7 */
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD)   /* keypad 8 */
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)   /* keypad 9 */
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_OTHER) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) PORT_CODE(KEYCODE_ASTERISK)    /* keypad * */
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)   /* keypad 0 */
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Keypad #") PORT_CODE(KEYCODE_PLUS_PAD)   /* keypad + */

INPUT_PORTS_END

#if 0
static DEVICE_INPUT_DEFAULTS_START(mwskins_comm)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_14400)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_14400)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END
#endif

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void atlantis_state::mwskins(machine_config &config)
{
	/* basic machine hardware */
	VR4310LE(config, m_maincpu, 166666666);
	m_maincpu->set_icache_size(16384);
	m_maincpu->set_dcache_size(16384);
	m_maincpu->set_system_clock(66666666);

	PCI_ROOT(config, "pci", 0);

	vrc4373_device &vrc4373(VRC4373(config, PCI_ID_NILE, 0, m_maincpu));
	vrc4373.set_ram_size(0x00800000);

	pci9050_device &pci9050(PCI9050(config, PCI_ID_9050, 0));
	pci9050.set_map(0, address_map_constructor(&atlantis_state::map0, "map0", this), this);
	pci9050.set_map(1, address_map_constructor(&atlantis_state::map1, "map1", this), this);
	pci9050.set_map(2, address_map_constructor(&atlantis_state::map2, "map2", this), this);
	pci9050.set_map(3, address_map_constructor(&atlantis_state::map3, "map3", this), this);
	pci9050.user_output_callback().set(FUNC(atlantis_state::user_io_output));
	pci9050.user_input_callback().set(FUNC(atlantis_state::user_io_input));

	M48T37(config, m_rtc);
	m_rtc->reset_cb().set(FUNC(atlantis_state::watchdog_reset));
	m_rtc->irq_cb().set(FUNC(atlantis_state::watchdog_irq));

	IDE_PCI(config, m_ide, 0, 0x10950646, 0x07, 0x0, PCI_ID_NILE, AS_DATA).irq_handler().set(FUNC(atlantis_state::ide_irq));

	/* video hardware */
	ZEUS2(config, m_zeus, ZEUS2_VIDEO_CLOCK);
	m_zeus->set_float_mode(1);
	m_zeus->irq_callback().set(FUNC(atlantis_state::zeus_irq));
	m_zeus->vblank_callback().set(FUNC(atlantis_state::vblank_irq));
	m_zeus->set_screen(m_screen);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(ZEUS2_VIDEO_CLOCK / 8, 529, 0, 400, 278, 0, 256);
	m_screen->set_screen_update("zeus2", FUNC(zeus2_device::screen_update));

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DCS2_AUDIO_DENVER_2CH(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(4);
	m_dcs->set_polling_offset(0xe33);
	m_dcs->add_route(0, "rspeaker", 1.0);
	m_dcs->add_route(1, "lspeaker", 1.0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag(m_dcs);
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_STANDARD);
	m_ioasic->set_yearoffs(80);
	m_ioasic->set_upper(342); // 325
	m_ioasic->irq_handler().set(FUNC(atlantis_state::ioasic_irq));
	m_ioasic->set_auto_ack(1);
	if DEBUG_CONSOLE {
		m_ioasic->serial_tx_handler().set(m_uart0, FUNC(generic_terminal_device::write));
		GENERIC_TERMINAL(config, m_uart0, 0);
		m_uart0->set_keyboard_callback("ioasic", FUNC(midway_ioasic_device::serial_rx_w));
	}

	// TL16C552 UART
	NS16550(config, m_uart1, XTAL(1'843'200));
	m_uart1->out_tx_callback().set("com1", FUNC(rs232_port_device::write_txd));
	m_uart1->out_dtr_callback().set("com1", FUNC(rs232_port_device::write_dtr));
	m_uart1->out_rts_callback().set("com1", FUNC(rs232_port_device::write_rts));
	m_uart1->out_int_callback().set(FUNC(atlantis_state::duart_irq_callback));

	NS16550(config, m_uart2, XTAL(1'843'200));
	m_uart2->out_tx_callback().set("com2", FUNC(rs232_port_device::write_txd));
	m_uart2->out_dtr_callback().set("com2", FUNC(rs232_port_device::write_dtr));
	m_uart2->out_rts_callback().set("com2", FUNC(rs232_port_device::write_rts));
	m_uart2->out_int_callback().set(FUNC(atlantis_state::duart_irq_callback));

	rs232_port_device &com1(RS232_PORT(config, "com1", default_rs232_devices, nullptr));
	com1.rxd_handler().set(m_uart1, FUNC(ins8250_uart_device::rx_w));
	com1.dcd_handler().set(m_uart1, FUNC(ins8250_uart_device::dcd_w));
	com1.dsr_handler().set(m_uart1, FUNC(ins8250_uart_device::dsr_w));
	com1.ri_handler().set(m_uart1, FUNC(ins8250_uart_device::ri_w));
	com1.cts_handler().set(m_uart1, FUNC(ins8250_uart_device::cts_w));
	//com1.set_option_device_input_defaults("com1", DEVICE_INPUT_DEFAULTS_NAME(mwskins_comm));

	rs232_port_device &com2(RS232_PORT(config, "com2", default_rs232_devices, nullptr));
	com2.rxd_handler().set(m_uart2, FUNC(ins8250_uart_device::rx_w));
	com2.dcd_handler().set(m_uart2, FUNC(ins8250_uart_device::dcd_w));
	com2.dsr_handler().set(m_uart2, FUNC(ins8250_uart_device::dsr_w));
	com2.ri_handler().set(m_uart2, FUNC(ins8250_uart_device::ri_w));
	com2.cts_handler().set(m_uart2, FUNC(ins8250_uart_device::cts_w));
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mwskins )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )  /* 512k for R4310 code */
	ROM_LOAD( "skins_game_u4_boot_1.00.u4", 0x000000, 0x080000, CRC(0fe87720) SHA1(4b24abbe662a2d7b61e6a3f079e28b73605ba19f) ) // EPR 1.00 Feb 14 2000 12:20:22

	DISK_REGION(PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "mwskins", 0, SHA1(5cb293a6fdb2478293f48ddfc93cdd018acb2bb5) )
ROM_END

ROM_START( mwskinsa )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )  /* 512k for R4310 code */
	ROM_LOAD( "skins_game_u4_boot_1.00.u4", 0x000000, 0x080000, CRC(0fe87720) SHA1(4b24abbe662a2d7b61e6a3f079e28b73605ba19f) )

	DISK_REGION(PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "mwskinsa", 0, SHA1(72497917b31156eb11a46bbcc6f22a254dcec044) )
ROM_END

ROM_START( mwskinso )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )  /* 512k for R4310 code */
	ROM_LOAD( "skins_game_u4_boot_1.00.u4", 0x000000, 0x080000, CRC(0fe87720) SHA1(4b24abbe662a2d7b61e6a3f079e28b73605ba19f) )

	DISK_REGION(PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "mwskins104", 0, SHA1(6917f66718999c144c854795c5856bf5659b85fa) )
ROM_END

ROM_START( mwskinst )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )  /* 512k for R4310 code */
	ROM_LOAD( "boot101.bin", 0x000000, 0x080000, CRC(8b02035f) SHA1(e7c6c2711052230c117a9342f38c185e311f2950) ) // EPR 1.01 Apr 13 2000 11:18:32

	DISK_REGION(PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "mwskinst", 0, SHA1(1edcf05bd9d5c9d1422e84bd713d1d120940e365) )
	// another dump with data SHA1 cba09f0240dd797b554ae28b74416472d2327e5b is available, both have latest version 1.15 in the changelog at 0x934687
ROM_END

} // Anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 2000, mwskins,  0,       mwskins, mwskins, atlantis_state, empty_init, ROT0, "Midway", "Skins Game (1.06)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2000, mwskinsa, mwskins, mwskins, mwskins, atlantis_state, empty_init, ROT0, "Midway", "Skins Game (1.06, alt)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
GAME( 2000, mwskinso, mwskins, mwskins, mwskins, atlantis_state, empty_init, ROT0, "Midway", "Skins Game (1.04)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
GAME( 2000, mwskinst, mwskins, mwskins, mwskins, atlantis_state, empty_init, ROT0, "Midway", "Skins Game Tournament Edition", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
