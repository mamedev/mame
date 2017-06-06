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
        * Midway Zeus-series custom video
        * TL16c552 dual UART
        * ADSP-2181 based DCS2 audio (unclear which variant)
        * PIC16C57 (protection? serial #?)
        * Quantum Fireball CX 6.4GB IDE HDD (C/H/S 13328/15/63)

    TODO:
        * PCI peripherals

    NOTES:
        * Skins Game is Linux based; the kernel is a customized 2.2.10 build of Linux-MIPS with Midway PCBs
          added as recognized system types

***************************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"
#include "cpu/adsp2100/adsp2100.h"
#include "machine/idectrl.h"
#include "machine/midwayic.h"
#include "machine/ins8250.h"
#include "bus/rs232/rs232.h"
#include "machine/terminal.h"
#include "audio/dcs.h"
#include "machine/pci.h"
#include "machine/vrc4373.h"
#include "machine/pci9050.h"
#include "machine/pci-ide.h"
#include "video/zeus2.h"
#include "machine/nvram.h"
#include "coreutil.h"

// Reset bits
#define RESET_IOASIC        0x01
#define RESET_ROMBUS        0x02
#define RESET_ZEUS          0x04
#define RESET_ROMBUS_IN     0x08
#define RESET_WDOG          0x10

// IRQ Bits
#define IOASIC_IRQ_SHIFT    0
#define ROMBUS_IRQ_SHIFT    1
#define ZEUS0_IRQ_SHIFT     2
#define ZEUS1_IRQ_SHIFT     3
#define ZEUS2_IRQ_SHIFT     4
#define WDOG_IRQ_SHIFT      5
#define A2D_IRQ_SHIFT       6
#define VBLANK_IRQ_SHIFT    7

// DUART mapped to int3 (map3 = 0x08)
#define UART1_IRQ_SHIFT     ZEUS1_IRQ_SHIFT
#define UART2_IRQ_SHIFT     ZEUS1_IRQ_SHIFT
// PCI mapped to int2 (map2 = 0x10)
#define PCI_IRQ_SHIFT       ZEUS2_IRQ_SHIFT

/* static interrupts */
#define GALILEO_IRQ_NUM         MIPS3_IRQ0
#define IDE_IRQ_NUM             MIPS3_IRQ4

#define DEBUG_CONSOLE   (0)
#define LOG_RTC         (0)
#define LOG_PORT        (0)
#define LOG_IRQ         (0)

class atlantis_state : public driver_device
{
public:
	atlantis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_zeus(*this, "zeus2"),
		m_dcs(*this, "dcs"),
		m_ioasic(*this, "ioasic"),
		m_uart0(*this, "uart0"),
		m_uart1(*this, "uart1"),
		m_uart2(*this, "uart2"),
		m_rtc(*this, "rtc")
	{ }
	DECLARE_DRIVER_INIT(mwskins);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<mips3_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	required_device<zeus2_device> m_zeus;
	required_device<dcs_audio_device> m_dcs;
	required_device<midway_ioasic_device> m_ioasic;
	optional_device<generic_terminal_device> m_uart0;
	required_device<ns16550_device> m_uart1;
	required_device<ns16550_device> m_uart2;
	required_device<nvram_device> m_rtc;
	uint8_t m_rtc_data[0x8000];

	READ8_MEMBER(cmos_r);
	WRITE8_MEMBER(cmos_w);
	uint32_t m_cmos_write_enabled;
	uint32_t m_serial_count;


	DECLARE_WRITE32_MEMBER(asic_fifo_w);
	DECLARE_WRITE32_MEMBER(dcs3_fifo_full_w);

	READ8_MEMBER (exprom_r);
	WRITE8_MEMBER(exprom_w);

	WRITE32_MEMBER(user_io_output);
	READ32_MEMBER(user_io_input);
	int m_user_io_state;

	// Board Ctrl Reg Offsets
	enum {
		PLD_REV, RESET, VSYNC_CLEAR, IRQ_MAP1, IRQ_MAP2, IRQ_MAP3,
		IRQ_EN = 7, CAUSE, STATUS, SIZE, LED, CMOS_UNLOCK, WDOG, TRACKBALL_CTL,
		CTRL_SIZE
	};
	DECLARE_READ32_MEMBER(board_ctrl_r);
	DECLARE_WRITE32_MEMBER(board_ctrl_w);
	uint32_t m_irq_state;
	uint32_t board_ctrl[CTRL_SIZE];
	void update_asic_irq();

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	DECLARE_WRITE_LINE_MEMBER(zeus_irq);
	DECLARE_WRITE_LINE_MEMBER(ide_irq);
	DECLARE_WRITE_LINE_MEMBER(ioasic_irq);

	DECLARE_WRITE_LINE_MEMBER(uart1_irq_callback);
	DECLARE_WRITE_LINE_MEMBER(uart2_irq_callback);

	DECLARE_CUSTOM_INPUT_MEMBER(port_mod_r);
	DECLARE_READ16_MEMBER(port_ctrl_r);
	DECLARE_WRITE16_MEMBER(port_ctrl_w);
	uint16_t m_port_data;
	uint16_t m_a2d_data;

	DECLARE_READ16_MEMBER(a2d_ctrl_r);
	DECLARE_WRITE16_MEMBER(a2d_ctrl_w);

	DECLARE_READ16_MEMBER(a2d_data_r);
	DECLARE_WRITE16_MEMBER(a2d_data_w);

	DECLARE_READ8_MEMBER(parallel_r);
	DECLARE_WRITE8_MEMBER(parallel_w);
};

// Parallel Port
READ8_MEMBER(atlantis_state::parallel_r)
{
	logerror("%06X: parallel_r %08x = %02x\n", machine().device("maincpu")->safe_pc(), offset, 0);
	return 0;
}

WRITE8_MEMBER(atlantis_state::parallel_w)
{
	logerror("%06X: parallel_w %08x = %02x\n", machine().device("maincpu")->safe_pc(), offset, data);
}

// Expansion ROM
READ8_MEMBER (atlantis_state::exprom_r)
{
	logerror("%06X: exprom_r %08x = %02x\n", machine().device("maincpu")->safe_pc(), offset, 0);
	return 0;
}

WRITE8_MEMBER(atlantis_state::exprom_w)
{
	logerror("%06X: exprom_w %08x = %02x\n", machine().device("maincpu")->safe_pc(), offset, data);
}

// Board PLD
READ32_MEMBER(atlantis_state::board_ctrl_r)
{
	uint32_t newOffset = offset >> 17;
	uint32_t data = board_ctrl[newOffset];
	switch (newOffset) {
	case PLD_REV:
		// ???
		data = 0x1;
	case STATUS:
		if (LOG_IRQ)
			logerror("%s:board_ctrl_r read from STATUS offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	default:
		if (LOG_IRQ)
			logerror("%s:board_ctrl_r read from offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	}
	return data;
}

WRITE32_MEMBER(atlantis_state::board_ctrl_w)
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
				m_dcs->reset_w(ASSERT_LINE);
			}
			else {
				m_dcs->reset_w(CLEAR_LINE);
			}
		}
		if ((changeData & RESET_WDOG) || LOG_IRQ)
			logerror("%s:board_ctrl_w write to RESET_WDOG offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	case VSYNC_CLEAR:
		//VSYNC_IE (0x1)
		//VSYNC_POL (0x2)   off=negative true, on=positive true
		// 0x1 VBlank clear?
		if (changeData & 0x1) {
			if ((data & 0x0001) == 0) {
				uint32_t status_bit = (1 << VBLANK_IRQ_SHIFT);
				board_ctrl[CAUSE] &= ~status_bit;
				board_ctrl[STATUS] &= ~status_bit;
				update_asic_irq();
			}
			else {
			}
		}
		if (0 && LOG_IRQ)
			logerror("%s:board_ctrl_w write to CTRL_VSYNC_CLEAR offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	case IRQ_EN:
		// Zero bit will clear cause
		board_ctrl[CAUSE] &= data;
		update_asic_irq();
		if (LOG_IRQ)
			logerror("%s:board_ctrl_w write to IRQ_EN offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
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
			case 0xa7: digit = 'C'; break;
			case 0xa1: digit = 'D'; break;
			case 0x86: digit = 'E'; break;
			case 0x87: digit = 'F'; break;
			case 0x7f: digit = '.'; break;
			case 0xf7: digit = '_'; break;
			case 0xbf: digit = '|'; break;
			case 0xfe: digit = '-'; break;
			case 0xff: digit = 'Z'; break;
				if (0) logerror("%06X: status_leds_w digit: %c %08x = %02x\n", machine().device("maincpu")->safe_pc(), digit, offset, data);
			}
		}
		break;
	case CMOS_UNLOCK:
		m_cmos_write_enabled = true;
		break;
	default:
		if (LOG_IRQ)
			logerror("%s:board_ctrl_w write to offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	}
}


READ8_MEMBER(atlantis_state::cmos_r)
{
	uint8_t result = m_rtc_data[offset];

	switch (offset) {
	case 0x7FF9:
	case 0x7FFA:
	case 0x7FFB:
	case 0x7FFC:
	case 0x7FFD:
	case 0x7FFE:
	case 0x7FFF:
		if ((m_rtc_data[0x7FF8] & 0x40)==0) {
			system_time systime;
			// get the current date/time from the core
			machine().current_datetime(systime);
			m_rtc_data[0x7FF9] = dec_2_bcd(systime.local_time.second);
			m_rtc_data[0x7FFA] = dec_2_bcd(systime.local_time.minute);
			m_rtc_data[0x7FFB] = dec_2_bcd(systime.local_time.hour);

			m_rtc_data[0x7FFC] = dec_2_bcd((systime.local_time.weekday != 0) ? systime.local_time.weekday : 7);
			m_rtc_data[0x7FFD] = dec_2_bcd(systime.local_time.mday);
			m_rtc_data[0x7FFE] = dec_2_bcd(systime.local_time.month + 1);
			m_rtc_data[0x7FFF] = dec_2_bcd(systime.local_time.year - 1900); // Epoch is 1900
			result = m_rtc_data[offset];
		}
		break;
	default:
		if (LOG_RTC)
			logerror("%s:RTC read from offset %04X = %08X m_rtc_data[0x7FF8] %02X\n", machine().describe_context(), offset, result, m_rtc_data[0x7FF8]);
		break;
	}
	return result;
}

WRITE8_MEMBER(atlantis_state::cmos_w)
{
	system_time systime;
	// User I/O 0 = Allow write to cmos[0]. Serial Write Enable?
	if (offset == 0 && (m_user_io_state & 0x1)) {
		// Data written is shifted by 1 bit each time.  Maybe a serial line output?
		if (LOG_RTC && m_serial_count == 0)
			logerror("%06X: cmos_w[0] start serial %08x = %02x\n", machine().device("maincpu")->safe_pc(), offset, data);
		m_serial_count++;
		if (m_serial_count == 8)
			m_serial_count = 0;
	}
	else if (m_cmos_write_enabled) {
		COMBINE_DATA(&m_rtc_data[offset]);
		m_cmos_write_enabled = false;
		switch (offset) {
		case 0x7FF8: // M48T02 time
			if (data & 0x40) {
				// get the current date/time from the core
				machine().current_datetime(systime);
				m_rtc_data[0x7FF9] = dec_2_bcd(systime.local_time.second);
				m_rtc_data[0x7FFA] = dec_2_bcd(systime.local_time.minute);
				m_rtc_data[0x7FFB] = dec_2_bcd(systime.local_time.hour);

				m_rtc_data[0x7FFC] = dec_2_bcd((systime.local_time.weekday != 0) ? systime.local_time.weekday : 7);
				m_rtc_data[0x7FFD] = dec_2_bcd(systime.local_time.mday);
				m_rtc_data[0x7FFE] = dec_2_bcd(systime.local_time.month + 1);
				m_rtc_data[0x7FFF] = dec_2_bcd(systime.local_time.year - 1900); // Epoch is 1900
			}
			if (LOG_RTC)
				logerror("%s:RTC write to offset %04X = %08X & %08X\n", machine().describe_context(), offset, data, mem_mask);

			break;
		default:
			if (LOG_RTC)
				logerror("%s:RTC write to offset %04X = %08X & %08X\n", machine().describe_context(), offset, data, mem_mask);
			break;
		}
	}
}

WRITE32_MEMBER(atlantis_state::asic_fifo_w)
{
	m_ioasic->fifo_w(data);
}

WRITE32_MEMBER(atlantis_state::dcs3_fifo_full_w)
{
	m_ioasic->fifo_full_w(data);
}

/*************************************
*  PCI9050 User I/O handlers
*************************************/
WRITE32_MEMBER(atlantis_state::user_io_output)
{
	m_user_io_state = data;
	logerror("atlantis_state::user_io_output m_user_io_state = %1x\n", m_user_io_state);
}

READ32_MEMBER(atlantis_state::user_io_input)
{
	// Set user i/o (2) Power Detect?
	m_user_io_state |= 1 << 2;

	// User I/O 0 = Allow write to red[0]. Serial Write Enable?
	// Loop user_io(0) to user_io(1)
	m_user_io_state = (m_user_io_state & ~(0x2)) | ((m_user_io_state & 1) << 1);
	if (0)
		logerror("atlantis_state::user_io_input m_user_io_state = %1x\n", m_user_io_state);
	return m_user_io_state;
}

/*************************************
*  UART1 interrupt handler
*************************************/
WRITE_LINE_MEMBER(atlantis_state::uart1_irq_callback)
{
	uint32_t status_bit = UART1_IRQ_SHIFT;
	if (state && !(board_ctrl[STATUS] & status_bit)) {
		board_ctrl[STATUS] |= status_bit;
		update_asic_irq();
	}
	else if (!state && (board_ctrl[STATUS] & status_bit)) {
		board_ctrl[STATUS] &= ~status_bit;
		board_ctrl[CAUSE] &= ~status_bit;
		update_asic_irq();
	}
	logerror("atlantis_state::uart1_irq_callback state = %1x\n", state);
}

/*************************************
*  UART2 interrupt handler
*************************************/
WRITE_LINE_MEMBER(atlantis_state::uart2_irq_callback)
{
	uint32_t status_bit = UART2_IRQ_SHIFT;
	if (state && !(board_ctrl[STATUS] & status_bit)) {
		board_ctrl[STATUS] |= status_bit;
		update_asic_irq();
	}
	else if (!state && (board_ctrl[STATUS] & status_bit)) {
		board_ctrl[STATUS] &= ~status_bit;
		board_ctrl[CAUSE] &= ~status_bit;
		update_asic_irq();
	}
	logerror("atlantis_state::uart2_irq_callback state = %1x\n", state);
}

/*************************************
*  Video interrupts
*************************************/
WRITE_LINE_MEMBER(atlantis_state::vblank_irq)
{
	//logerror("%s: atlantis_state::vblank state = %i\n", machine().describe_context(), state);
	if (state) {
		board_ctrl[STATUS] |= (1 << VBLANK_IRQ_SHIFT);
		update_asic_irq();
	}
	else {
		board_ctrl[STATUS] &= ~(1 << VBLANK_IRQ_SHIFT);
		board_ctrl[CAUSE] &= ~(1 << VBLANK_IRQ_SHIFT);
		update_asic_irq();
	}
}

WRITE_LINE_MEMBER(atlantis_state::zeus_irq)
{
	//logerror("%s: atlantis_state::zeus_irq state = %i\n", machine().describe_context(), state);
	if (state) {
		board_ctrl[STATUS] |= (1 << ZEUS0_IRQ_SHIFT);
		update_asic_irq();
	}
	else {
		board_ctrl[STATUS] &= ~(1 << ZEUS0_IRQ_SHIFT);
		board_ctrl[CAUSE] &= ~(1 << ZEUS0_IRQ_SHIFT);
		update_asic_irq();
	}
}

/*************************************
*  IDE interrupts
*************************************/
WRITE_LINE_MEMBER(atlantis_state::ide_irq)
{
	m_maincpu->set_input_line(IDE_IRQ_NUM, state);
	if (LOG_IRQ)
		logerror("%s: atlantis_state::ide_irq state = %i\n", machine().describe_context(), state);
}

/*************************************
*  I/O ASIC interrupts
*************************************/
WRITE_LINE_MEMBER(atlantis_state::ioasic_irq)
{
	if (LOG_IRQ)
		logerror("%s: atlantis_state::ioasic_irq state = %i\n", machine().describe_context(), state);
	if (state) {
		board_ctrl[STATUS] |= (1 << IOASIC_IRQ_SHIFT);
		update_asic_irq();
	}
	else {
		board_ctrl[STATUS] &= ~(1 << IOASIC_IRQ_SHIFT);
		board_ctrl[CAUSE] &= ~(1 << IOASIC_IRQ_SHIFT);
		update_asic_irq();
	}
}

/*************************************
*  Programmable interrupt control
*************************************/
void atlantis_state::update_asic_irq()
{
	for (int irqIndex = 0; irqIndex < 3; irqIndex++) {
		uint32_t irqBits = (board_ctrl[IRQ_EN] & board_ctrl[IRQ_MAP1 + irqIndex] & board_ctrl[STATUS]);
		uint32_t causeBits = (board_ctrl[IRQ_EN] & board_ctrl[IRQ_MAP1 + irqIndex] & board_ctrl[CAUSE]);
		uint32_t currState = m_irq_state & (2 << irqIndex);
		board_ctrl[CAUSE] |= irqBits;
		if (irqBits && !currState) {
			m_maincpu->set_input_line(MIPS3_IRQ1 + irqIndex, ASSERT_LINE);
			m_irq_state |= (2 << irqIndex);
			if (LOG_IRQ)
				logerror("atlantis_state::update_asic_irq Asserting IRQ(%d) CAUSE = %02X\n", irqIndex, board_ctrl[CAUSE]);
		}
		else if (!(causeBits) && currState) {
			m_maincpu->set_input_line(MIPS3_IRQ1 + irqIndex, CLEAR_LINE);
			m_irq_state &= ~(2 << irqIndex);
			if (LOG_IRQ)
				logerror("atlantis_state::update_asic_irq Clearing IRQ(%d) CAUSE = %02X\n", irqIndex, board_ctrl[CAUSE]);
		}
	}
}
/*************************************
*  I/O Port control
*************************************/
READ16_MEMBER(atlantis_state::port_ctrl_r)
{
	uint32_t newOffset = offset >> 17;
	uint32_t result = m_port_data;
	if (LOG_PORT)
		logerror("%s: port_ctrl_r newOffset = %02X data = %08X\n", machine().describe_context(), newOffset, result);
	return result;
}

WRITE16_MEMBER(atlantis_state::port_ctrl_w)
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
		if (LOG_PORT)
			logerror("%s: port_ctrl_w Keypad Row Sel = %04X bits = %08X\n", machine().describe_context(), data, bits);
		break;
	}
	default:
		if (LOG_PORT)
			logerror("%s: port_ctrl_w write to offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
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

READ16_MEMBER(atlantis_state::a2d_ctrl_r)
{
	return A2D_CTRL_COMPLETE;
}

WRITE16_MEMBER(atlantis_state::a2d_ctrl_w)
{
	if (data  == 0x8f)
		m_a2d_data = ioport("AN.1")->read();
	else
		m_a2d_data = ioport("AN.0")->read();
}

READ16_MEMBER(atlantis_state::a2d_data_r)
{
	return m_a2d_data;
}

WRITE16_MEMBER(atlantis_state::a2d_data_w)
{

}

/*************************************
*  Machine start
*************************************/
void atlantis_state::machine_start()
{
	m_rtc->set_base(m_rtc_data, sizeof(m_rtc_data));

	/* set the fastest DRC options */
	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS);

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
	m_dcs->reset_w(1);
	m_dcs->reset_w(0);
	m_user_io_state = 0;
	m_cmos_write_enabled = false;
	m_serial_count = 0;
	m_irq_state = 0;
	memset(board_ctrl, 0, sizeof(board_ctrl));
}

/*************************************
 *  Address Maps
 *************************************/
static ADDRESS_MAP_START( map0, AS_PROGRAM, 32, atlantis_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_READWRITE8(cmos_r, cmos_w, 0xff)
	//AM_RANGE(0x00080000, 0x000?0000) AM_READWRITE8(zeus debug)
	AM_RANGE(0x00100000, 0x0010001f) AM_DEVREADWRITE8("uart1", ns16550_device, ins8250_r, ins8250_w, 0xff) // Serial UART1 (TL16C552 CS0)
	AM_RANGE(0x00180000, 0x0018001f) AM_DEVREADWRITE8("uart2", ns16550_device, ins8250_r, ins8250_w, 0xff) // Serial UART2 (TL16C552 CS1)
	AM_RANGE(0x00200000, 0x0020001f) AM_READWRITE8(parallel_r, parallel_w, 0xff) // Parallel UART (TL16C552 CS2)
	AM_RANGE(0x00400000, 0x007fffff) AM_READWRITE8(exprom_r, exprom_w, 0xff) // EXPROM
	AM_RANGE(0x00800000, 0x00f00003) AM_READWRITE(board_ctrl_r, board_ctrl_w)
	//AM_RANGE(0x00d80000, 0x00d80003) AM_READWRITE(status_leds_r, status_leds_w)
	//AM_RANGE(0x00e00000, 0x00e00003) AM_READWRITE(cmos_protect_r, cmos_protect_w)
	//AM_RANGE(0x00e80000, 0x00e80003) AM_NOP // Watchdog
	//AM_RANGE(0x00f00000, 0x00f00003) AM_NOP // Trackball ctrl
	ADDRESS_MAP_END

static ADDRESS_MAP_START( map1, AS_PROGRAM, 32, atlantis_state )
	AM_RANGE(0x00000000, 0x0000003f) AM_DEVREADWRITE("ioasic", midway_ioasic_device, read, write)
	AM_RANGE(0x00200000, 0x00200003) AM_WRITE(dcs3_fifo_full_w)
	AM_RANGE(0x00400000, 0x00400003) AM_DEVWRITE("dcs", dcs_audio_device, dsio_idma_addr_w)
	AM_RANGE(0x00600000, 0x00600003) AM_DEVREADWRITE("dcs", dcs_audio_device, dsio_idma_data_r, dsio_idma_data_w)
	AM_RANGE(0x00800000, 0x00900003) AM_READWRITE16(port_ctrl_r, port_ctrl_w, 0xffff)
	//AM_RANGE(0x00880000, 0x00880003) // AUX Output Initial write 0000fff0, follow by sequence ffef, ffdf, ffbf, fff7. Row Select?
	//AM_RANGE(0x00900000, 0x00900003) // AUX Input Read once before each sequence write to 0x00880000. Code checks bits 0,1,2. Keypad?
	AM_RANGE(0x00980000, 0x00980003) AM_READWRITE16(a2d_ctrl_r, a2d_ctrl_w, 0xffff) // A2D Control Read / Write.  Bytes written 0x8f, 0xcf. Code if read 0x1 then read 00a00000.
	AM_RANGE(0x00a00000, 0x00a00003) AM_READWRITE16(a2d_data_r, a2d_data_w, 0xffff) // A2D Data
	//AM_RANGE(0x00a80000, 0x00a80003) // Trackball Chan 0 16 bits
	//AM_RANGE(0x00b00000, 0x00b00003) // Trackball Chan 1 16 bits
	//AM_RANGE(0x00b80000, 0x00b80003) // Trackball Error 16 bits
	//AM_RANGE(0x00c00000, 0x00c00003) // Trackball Pins 16 bits
ADDRESS_MAP_END

static ADDRESS_MAP_START(map2, AS_PROGRAM, 32, atlantis_state)
	AM_RANGE(0x00000000, 0x000001ff) AM_DEVREADWRITE("zeus2", zeus2_device, zeus2_r, zeus2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( map3, AS_PROGRAM, 32, atlantis_state )
	//AM_RANGE(0x000000, 0xffffff) ROMBUS
ADDRESS_MAP_END

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
	PORT_DIPNAME(0x0004, 0x0004, "Boot Message")
	PORT_DIPSETTING(0x0004, "Quiet")
	PORT_DIPSETTING(0x0000, "Squawk During Boot")
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
	//PORT_BIT(0x0007, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, atlantis_state, port_mod_r, "KEYPAD")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("AN.0")
	PORT_BIT(0x1ff, 0x100, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN.1")
	PORT_BIT(0x1ff, 0x100, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("KEYPAD")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)   /* keypad 1 */
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)   /* keypad 2 */
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)   /* keypad 3 */
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)   /* keypad 4 */
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)   /* keypad 5 */
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)   /* keypad 6 */
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)   /* keypad 7 */
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)   /* keypad 8 */
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)   /* keypad 9 */
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad *") PORT_CODE(KEYCODE_MINUS_PAD)    /* keypad - */
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)   /* keypad 0 */
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad #") PORT_CODE(KEYCODE_PLUS_PAD)   /* keypad + */

INPUT_PORTS_END

#if 0
static DEVICE_INPUT_DEFAULTS_START(mwskins_comm)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_14400)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_14400)
	DEVICE_INPUT_DEFAULTS("RS232_STARTBITS", 0xff, RS232_STARTBITS_1)
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

#define PCI_ID_NILE     ":pci:00.0"
#define PCI_ID_9050     ":pci:0b.0"
#define PCI_ID_IDE      ":pci:0c.0"

static MACHINE_CONFIG_START( mwskins )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", VR4310LE, 166666666)    // clock is TRUSTED
	MCFG_MIPS3_ICACHE_SIZE(16384)
	MCFG_MIPS3_DCACHE_SIZE(16384)

	MCFG_PCI_ROOT_ADD(                ":pci")
	MCFG_VRC4373_ADD(                 PCI_ID_NILE, ":maincpu")
	MCFG_VRC4373_SET_RAM(0x00800000)
	MCFG_PCI9050_ADD(                 PCI_ID_9050)
	MCFG_PCI9050_SET_MAP(0, map0)
	MCFG_PCI9050_SET_MAP(1, map1)
	MCFG_PCI9050_SET_MAP(2, map2)
	MCFG_PCI9050_SET_MAP(3, map3)
	MCFG_PCI9050_USER_OUTPUT_CALLBACK(DEVWRITE32(":", atlantis_state, user_io_output))
	MCFG_PCI9050_USER_INPUT_CALLBACK(DEVREAD32(":", atlantis_state, user_io_input))

	MCFG_NVRAM_ADD_0FILL("rtc")

	MCFG_IDE_PCI_ADD(PCI_ID_IDE, 0x10950646, 0x07, 0x0)
	MCFG_IDE_PCI_IRQ_HANDLER(DEVWRITELINE(":", atlantis_state, ide_irq))

	/* video hardware */
	MCFG_DEVICE_ADD("zeus2", ZEUS2, ZEUS2_VIDEO_CLOCK)
	MCFG_ZEUS2_FLOAT_MODE(1)
	MCFG_ZEUS2_IRQ_CB(WRITELINE(atlantis_state, zeus_irq))
	MCFG_ZEUS2_VBLANK_CB(WRITELINE(atlantis_state, vblank_irq))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(ZEUS2_VIDEO_CLOCK / 8, 529, 0, 400, 278, 0, 256)
	MCFG_SCREEN_UPDATE_DEVICE("zeus2", zeus2_device, screen_update)

	/* sound hardware */
	//MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_DSIO, 0)
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_DENVER, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(8)
	//MCFG_DCS2_AUDIO_POLLING_OFFSET(0) /* no place to hook :-( */

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(MIDWAY_IOASIC_STANDARD)
	MCFG_MIDWAY_SERIAL_PIC2_YEAR_OFFS(80)
	MCFG_MIDWAY_IOASIC_UPPER(325)
	MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(WRITELINE(atlantis_state, ioasic_irq))
	MCFG_MIDWAY_IOASIC_AUTO_ACK(1)
	if DEBUG_CONSOLE {
		MCFG_MIDWAY_IOASIC_OUT_TX_CB(DEVWRITE8("uart0", generic_terminal_device, write))
		MCFG_DEVICE_ADD("uart0", GENERIC_TERMINAL, 0)
		MCFG_GENERIC_TERMINAL_KEYBOARD_CB(DEVPUT("ioasic", midway_ioasic_device, serial_rx_w))
	}

	// TL16C552 UART
	MCFG_DEVICE_ADD("uart1", NS16550, XTAL_24MHz)
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("com1", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(DEVWRITELINE("com1", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(DEVWRITELINE("com1", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE(":", atlantis_state, uart1_irq_callback))

	MCFG_DEVICE_ADD("uart2", NS16550, XTAL_24MHz)
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("com2", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(DEVWRITELINE("com2", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(DEVWRITELINE("com2", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE(":", atlantis_state, uart2_irq_callback))

	MCFG_RS232_PORT_ADD("com1", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("uart1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart1", ins8250_uart_device, cts_w))
	//MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("com1", mwskins_comm)

	MCFG_RS232_PORT_ADD("com2", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart2", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart2", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart2", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("uart2", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart2", ins8250_uart_device, cts_w))
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mwskins )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )  /* 512k for R4310 code */
	ROM_LOAD( "skins_game_u4_boot_1.00.u4", 0x000000, 0x080000, CRC(0fe87720) SHA1(4b24abbe662a2d7b61e6a3f079e28b73605ba19f) )

	DISK_REGION(PCI_ID_IDE":ide:0:hdd:image" )
	DISK_IMAGE( "mwskins", 0, SHA1(5cb293a6fdb2478293f48ddfc93cdd018acb2bb5) )
ROM_END

ROM_START( mwskinsa )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )  /* 512k for R4310 code */
	ROM_LOAD( "skins_game_u4_boot_1.00.u4", 0x000000, 0x080000, CRC(0fe87720) SHA1(4b24abbe662a2d7b61e6a3f079e28b73605ba19f) )

	DISK_REGION(PCI_ID_IDE":ide:0:hdd:image" )
	DISK_IMAGE( "mwskinsa", 0, SHA1(72497917b31156eb11a46bbcc6f22a254dcec044) )
ROM_END

ROM_START( mwskinso )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )  /* 512k for R4310 code */
	ROM_LOAD( "skins_game_u4_boot_1.00.u4", 0x000000, 0x080000, CRC(0fe87720) SHA1(4b24abbe662a2d7b61e6a3f079e28b73605ba19f) )

	DISK_REGION(PCI_ID_IDE":ide:0:hdd:image" )
	DISK_IMAGE( "mwskins104", 0, SHA1(6917f66718999c144c854795c5856bf5659b85fa) )
ROM_END

ROM_START( mwskinst )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )  /* 512k for R4310 code */
	 // not dumped, using the one from mwskins, should be checked, even if it seems to work
	ROM_LOAD( "skins_game_u4_boot_1.00.u4", 0x000000, 0x080000, BAD_DUMP CRC(0fe87720) SHA1(4b24abbe662a2d7b61e6a3f079e28b73605ba19f) )

	DISK_REGION(PCI_ID_IDE":ide:0:hdd:image" )
	DISK_IMAGE( "mwskinst", 0, SHA1(1edcf05bd9d5c9d1422e84bd713d1d120940e365) )
ROM_END

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(atlantis_state,mwskins)
{
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 2000, mwskins,    0,      mwskins, mwskins, atlantis_state,  mwskins,   ROT0, "Midway", "Skins Game (1.06)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2000, mwskinsa, mwskins,  mwskins, mwskins, atlantis_state,  mwskins,   ROT0, "Midway", "Skins Game (1.06, alt)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
GAME( 2000, mwskinso, mwskins,  mwskins, mwskins, atlantis_state,  mwskins,   ROT0, "Midway", "Skins Game (1.04)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
GAME( 2000, mwskinst, mwskins,  mwskins, mwskins, atlantis_state,  mwskins,   ROT0, "Midway", "Skins Game Tournament Edition", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
