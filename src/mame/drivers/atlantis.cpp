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
#include "audio/dcs.h"
#include "machine/pci.h"
#include "machine/vrc4373.h"
#include "machine/pci9050.h"
#include "machine/pci-ide.h"
#include "includes/midzeus.h"
#include "includes/midzeus2.h"
#include "machine/nvram.h"
#include "coreutil.h"

// Board Ctrl Reg Offsets
#define CTRL_POWER0         0
#define CTRL_POWER1         1
#define CTRL_IRQ1_EN        2
#define CTRL_IRQ2_EN        3
#define CTRL_IRQ3_EN        4
#define CTRL_IRQ4_EN        5
#define CTRL_GLOBAL_EN      6
#define CTRL_CAUSE          7
#define CTRL_STATUS         8
#define CTRL_SIZE           9

// These need more verification
#define IOASIC_IRQ_SHIFT    0
#define PARALLEL_IRQ_SHIFT  3
#define UART0_SHIFT         4
#define UART1_SHIFT         5
#define ZEUS_IRQ_SHIFT      2
#define VBLANK_IRQ_SHIFT    7
#define GALILEO_IRQ_SHIFT   0

/* static interrupts */
#define GALILEO_IRQ_NUM         MIPS3_IRQ0
#define IDE_IRQ_NUM             MIPS3_IRQ4

#define LOG_RTC         (1)
#define LOG_ZEUS        (0)

class atlantis_state : public driver_device
{
public:
	atlantis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_dcs(*this, "dcs"),
		m_ioasic(*this, "ioasic"),
		m_uart0(*this, "uart0"),
		m_uart1(*this, "uart1"),
		m_rtc(*this, "rtc")
	{ }
	DECLARE_DRIVER_INIT(mwskins);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_mwskins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<mips3_device> m_maincpu;
	required_device<screen_device> m_screen;
	//required_device<dcs2_audio_dsio_device> m_dcs;
	//required_device<dcs2_audio_denver_device> m_dcs;
	required_device<dcs_audio_device> m_dcs;
	required_device<midway_ioasic_device> m_ioasic;
	required_device<ns16550_device> m_uart0;
	required_device<ns16550_device> m_uart1;
	required_device<nvram_device> m_rtc;
	UINT8 m_rtc_data[0x800];

	UINT32 m_last_offset;
	READ8_MEMBER(cmos_r);
	WRITE8_MEMBER(cmos_w);
	DECLARE_WRITE32_MEMBER(cmos_protect_w);
	DECLARE_READ32_MEMBER(cmos_protect_r);
	UINT32 m_cmos_write_enabled;

	DECLARE_READ32_MEMBER(status_leds_r);
	DECLARE_WRITE32_MEMBER(status_leds_w);
	UINT8 m_status_leds;

	DECLARE_WRITE32_MEMBER(asic_fifo_w);
	DECLARE_WRITE32_MEMBER(dcs3_fifo_full_w);

	DECLARE_WRITE32_MEMBER(zeus_w);
	DECLARE_READ32_MEMBER(zeus_r);
	UINT32 m_zeus_data[0x80];

	READ8_MEMBER (red_r);
	WRITE8_MEMBER(red_w);
	UINT8 m_red_data[0x1000];
	int m_red_count;

	READ32_MEMBER (green_r);
	WRITE32_MEMBER(green_w);
	READ8_MEMBER (blue_r);
	WRITE8_MEMBER(blue_w);

	WRITE32_MEMBER(user_io_output);
	READ32_MEMBER(user_io_input);
	int m_user_io_state;

	DECLARE_READ32_MEMBER(board_ctrl_r);
	DECLARE_WRITE32_MEMBER(board_ctrl_w);
	UINT32 m_irq_state;
	UINT8 board_ctrl[CTRL_SIZE];
	void update_asic_irq();

	DECLARE_WRITE_LINE_MEMBER(ide_irq);
	DECLARE_WRITE_LINE_MEMBER(ioasic_irq);

	DECLARE_WRITE_LINE_MEMBER(uart0_irq_callback);
	DECLARE_WRITE_LINE_MEMBER(uart1_irq_callback);

	DECLARE_CUSTOM_INPUT_MEMBER(port_mod_r);
	DECLARE_READ32_MEMBER(port_ctrl_r);
	DECLARE_WRITE32_MEMBER(port_ctrl_w);
	UINT32 m_port_ctrl_reg[0x8];
};

READ8_MEMBER (atlantis_state::red_r)
{
	UINT8 data = m_red_data[offset];
	logerror("%06X: red_r %08x = %02x\n", machine().device("maincpu")->safe_pc(), offset, data);
	m_last_offset = offset | 0x10000;
	return data;
}

WRITE8_MEMBER(atlantis_state::red_w)
{
	COMBINE_DATA(&m_red_data[offset]);

	switch (offset) {
	case 0:
		// User I/O 0 = Allow write to red[0]. Serial Write Enable?
		if (m_user_io_state & 0x1) {
			// Data written is shifted by 1 bit each time.  Maybe a serial line output?
			if (m_red_count == 0)
				logerror("%06X: red_w start serial %08x = %02x\n", machine().device("maincpu")->safe_pc(), offset, data);
			m_red_count++;
			if (m_red_count == 8)
				m_red_count = 0;
			break;
		}  // Fall through to default if not enabled
	default:
		logerror("%06X: red_w %08x = %02x\n", machine().device("maincpu")->safe_pc(), offset, data);
		break;
	}
	m_last_offset = offset | 0x10000;
}

READ32_MEMBER(atlantis_state::green_r)
{
	// If not 0x80 cpu writes to 00e80000 = 0
	if ((offset | 0x20000) != m_last_offset)
		logerror("%06X: green_r %08x = %02x\n", machine().device("maincpu")->safe_pc(), offset, 0x80);
	m_last_offset = offset | 0x20000;
	return 0x80;
}

WRITE32_MEMBER(atlantis_state::green_w)
{
	logerror("%06X: green_w %08x = %02x\n", machine().device("maincpu")->safe_pc(), offset, data);
	m_last_offset = offset | 0x20000;
}

READ8_MEMBER (atlantis_state::blue_r)
{
	//UINT8 data = m_red_data[offset];
	logerror("%06X: blue_r %08x = %02x\n", machine().device("maincpu")->safe_pc(), offset, 0);
	//return data;
	return 0;
}

WRITE8_MEMBER(atlantis_state::blue_w)
{
	logerror("%06X: blue_w %08x = %02x\n", machine().device("maincpu")->safe_pc(), offset, data);
}

READ32_MEMBER(atlantis_state::board_ctrl_r)
{
	UINT32 newOffset = offset >> 17;
	UINT32 data = board_ctrl[newOffset];
	switch (newOffset) {
	case CTRL_STATUS:
		if (1 && m_screen->vblank())
			data |= 0x80;
		if (m_last_offset != (newOffset | 0x40000))
			logerror("%s:board_ctrl_r read from CTRL_STATUS offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	default:
		logerror("%s:board_ctrl_r read from offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	}
	m_last_offset = newOffset | 0x40000;
	return data;
}

WRITE32_MEMBER(atlantis_state::board_ctrl_w)
{
	UINT32 newOffset = offset >> 17;
	UINT32 changeData = board_ctrl[newOffset] ^ data;
	COMBINE_DATA(&board_ctrl[newOffset]);
	switch (newOffset) {
	case CTRL_POWER0:
		// 0x1 IOASIC Reset
		// 0x4 Zeus2 Reset
		// 0x10 IDE Reset
		if (changeData & 0x1) {
			if ((data & 0x0001) == 0) {
				m_ioasic->ioasic_reset();
				m_dcs->reset_w(ASSERT_LINE);
			}
			else {
				m_dcs->reset_w(CLEAR_LINE);
			}
		}
		logerror("%s:board_ctrl_w write to CTRL_POWER0 offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	case CTRL_POWER1:
		// 0x1 VBlank clear?
		if (changeData & 0x1) {
			if ((data & 0x0001) == 0) {
			}
			else {
			}
		}
		logerror("%s:board_ctrl_w write to CTRL_POWER1 offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	case CTRL_GLOBAL_EN:
		// Zero bit will clear cause
		board_ctrl[CTRL_CAUSE] &= data;
		update_asic_irq();
		logerror("%s:board_ctrl_w write to CTRL_GLOBAL_EN offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	default:
		logerror("%s:board_ctrl_w write to offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	}
}


WRITE32_MEMBER(atlantis_state::asic_fifo_w)
{
	m_ioasic->fifo_w(data);
}

READ8_MEMBER(atlantis_state::cmos_r)
{
	UINT8 result = m_rtc_data[offset];

	switch (offset) {
	case 0x7F9:
	case 0x7FA:
	case 0x7FB:
	case 0x7FC:
	case 0x7FD:
	case 0x7FE:
	case 0x7FF:
		if ((m_rtc_data[0x7F8] & 0x40)==0) {
			system_time systime;
			// get the current date/time from the core
			machine().current_datetime(systime);
			m_rtc_data[0x7F9] = dec_2_bcd(systime.local_time.second);
			m_rtc_data[0x7FA] = dec_2_bcd(systime.local_time.minute);
			m_rtc_data[0x7FB] = dec_2_bcd(systime.local_time.hour);

			m_rtc_data[0x7FC] = dec_2_bcd((systime.local_time.weekday != 0) ? systime.local_time.weekday : 7);
			m_rtc_data[0x7FD] = dec_2_bcd(systime.local_time.mday);
			m_rtc_data[0x7FE] = dec_2_bcd(systime.local_time.month + 1);
			m_rtc_data[0x7FF] = dec_2_bcd(systime.local_time.year - 1900); // Epoch is 1900
			result = m_rtc_data[offset];
		}
		break;
	default:
		if (LOG_RTC)
			logerror("%s:RTC read from offset %04X = %08X m_rtc_data[0x7F8] %02X\n", machine().describe_context(), offset, result, m_rtc_data[0x7F8]);
		break;
	}
	return result;
}

WRITE8_MEMBER(atlantis_state::cmos_w)
{
	system_time systime;

	if (m_cmos_write_enabled) {
		COMBINE_DATA(&m_rtc_data[offset]);
		m_cmos_write_enabled = FALSE;
		switch (offset) {
		case 0x7F8: // M48T02 time
			if (data & 0x40) {
				// get the current date/time from the core
				machine().current_datetime(systime);
				m_rtc_data[0x7F9] = dec_2_bcd(systime.local_time.second);
				m_rtc_data[0x7FA] = dec_2_bcd(systime.local_time.minute);
				m_rtc_data[0x7FB] = dec_2_bcd(systime.local_time.hour);

				m_rtc_data[0x7FC] = dec_2_bcd((systime.local_time.weekday != 0) ? systime.local_time.weekday : 7);
				m_rtc_data[0x7FD] = dec_2_bcd(systime.local_time.mday);
				m_rtc_data[0x7FE] = dec_2_bcd(systime.local_time.month + 1);
				m_rtc_data[0x7FF] = dec_2_bcd(systime.local_time.year - 1900); // Epoch is 1900
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

WRITE32_MEMBER(atlantis_state::cmos_protect_w)
{
	m_cmos_write_enabled = TRUE;
}

READ32_MEMBER(atlantis_state::status_leds_r)
{
	return m_status_leds | 0xffffff00;
}


WRITE32_MEMBER(atlantis_state::status_leds_w)
{
	if (ACCESSING_BITS_0_7) {
		m_status_leds = data;
		if (1) {
			char digit = 'U';
			switch (m_status_leds) {
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
			}
			popmessage("LED: %c", digit);
			osd_printf_debug("%06X: status_leds_w digit: %c %08x = %02x\n", machine().device("maincpu")->safe_pc(), digit, offset, data);
			logerror("%06X: status_leds_w digit: %c %08x = %02x\n", machine().device("maincpu")->safe_pc(), digit, offset, data);
		}
	}
}

READ32_MEMBER(atlantis_state::zeus_r)
{
	UINT32 result = m_zeus_data[offset];
	switch (offset) {
	case 0x1:
		/* bit  $000C0070 are tested in a loop until 0 */
		/* bits $00080000 is tested in a loop until 0 */
		/* bit  $00000004 is tested for toggling; probably VBLANK */
		// zeus is reset if 0x80 is read
		//if (m_screen->vblank())
			m_zeus_data[offset] = (m_zeus_data[offset] + 1) & 0xf;
		break;
	case 0x41:
		// CPU resets map2, writes 0xffffffff here, and then expects this read
		result &= 0x1fff03ff;
		break;
	}
	if (LOG_ZEUS)
		logerror("%s:zeus_r read from offset %04X = %08X & %08X\n", machine().describe_context(), offset, result, mem_mask);
	return result;
}

WRITE32_MEMBER(atlantis_state::zeus_w)
{
	COMBINE_DATA(&m_zeus_data[offset]);
	if (LOG_ZEUS)
		logerror("%s:zeus_w write to offset %04X = %08X & %08X\n", machine().describe_context(), offset, data, mem_mask);
	m_last_offset = offset | 0x30000;
}


READ32_MEMBER(atlantis_state::cmos_protect_r)
{
	return m_cmos_write_enabled;
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
*  UART0 interrupt handler
*************************************/
WRITE_LINE_MEMBER(atlantis_state::uart0_irq_callback)
{
	UINT32 status_bit = (1 << UART0_SHIFT);
	if (state && !(board_ctrl[CTRL_STATUS] & status_bit)) {
		board_ctrl[CTRL_STATUS] |= status_bit;
		update_asic_irq();
	}
	else if (!state && (board_ctrl[CTRL_STATUS] & status_bit)) {
		board_ctrl[CTRL_STATUS] &= ~status_bit;
		board_ctrl[CTRL_CAUSE] &= ~status_bit;
		update_asic_irq();
	}
	logerror("atlantis_state::uart0_irq_callback state = %1x\n", state);
}

/*************************************
*  UART1 interrupt handler
*************************************/
WRITE_LINE_MEMBER(atlantis_state::uart1_irq_callback)
{
	UINT32 status_bit = (1 << UART1_SHIFT);
	if (state && !(board_ctrl[CTRL_STATUS] & status_bit)) {
		board_ctrl[CTRL_STATUS] |= status_bit;
		update_asic_irq();
	}
	else if (!state && (board_ctrl[CTRL_STATUS] & status_bit)) {
		board_ctrl[CTRL_STATUS] &= ~status_bit;
		board_ctrl[CTRL_CAUSE] &= ~status_bit;
		update_asic_irq();
	}
	logerror("atlantis_state::uart1_irq_callback state = %1x\n", state);
}

/*************************************
*  IDE interrupts
*************************************/
WRITE_LINE_MEMBER(atlantis_state::ide_irq)
{
	m_maincpu->set_input_line(IDE_IRQ_NUM, state);
	logerror("%s: atlantis_state::ide_irq state = %i\n", machine().describe_context(), state);
}

/*************************************
*  I/O ASIC interrupts
*************************************/
WRITE_LINE_MEMBER(atlantis_state::ioasic_irq)
{
	logerror("%s: atlantis_state::ioasic_irq state = %i\n", machine().describe_context(), state);
	if (state) {
		board_ctrl[CTRL_STATUS] |= (1 << IOASIC_IRQ_SHIFT);
		update_asic_irq();
	}
	else {
		board_ctrl[CTRL_STATUS] &= ~(1 << IOASIC_IRQ_SHIFT);
		board_ctrl[CTRL_CAUSE] &= ~(1 << IOASIC_IRQ_SHIFT);
		update_asic_irq();
	}
}

/*************************************
*  Programmable interrupt control
*************************************/
void atlantis_state::update_asic_irq()
{
	// Uknown if CTRL_POWER1 is actually a separate power register.  Skip it for now.
	for (int irqIndex = 1; irqIndex <= 4; irqIndex++) {
		UINT32 irqBits = (board_ctrl[CTRL_GLOBAL_EN] & board_ctrl[CTRL_POWER1 + irqIndex] & board_ctrl[CTRL_STATUS]);
		UINT32 causeBits = (board_ctrl[CTRL_GLOBAL_EN] & board_ctrl[CTRL_POWER1 + irqIndex] & board_ctrl[CTRL_CAUSE]);
		UINT32 currState = m_irq_state & (1 << irqIndex);
		board_ctrl[CTRL_CAUSE] |= irqBits;
		if (irqBits && !currState) {
			m_maincpu->set_input_line(MIPS3_IRQ0 + irqIndex, ASSERT_LINE);
			m_irq_state |= (1 << irqIndex);
			if (1)
				logerror("atlantis_state::update_asic_irq Asserting IRQ(%d) CAUSE = %02X\n", irqIndex, board_ctrl[CTRL_CAUSE]);
		}
		else if (!(causeBits) && currState) {
			m_maincpu->set_input_line(MIPS3_IRQ0 + irqIndex, CLEAR_LINE);
			m_irq_state &= ~(1 << irqIndex);
			if (1)
				logerror("atlantis_state::update_asic_irq Clearing IRQ(%d) CAUSE = %02X\n", irqIndex, board_ctrl[CTRL_CAUSE]);
		}
	}
}
/*************************************
*  I/O Port control
*************************************/
READ32_MEMBER(atlantis_state::port_ctrl_r)
{
	UINT32 newOffset = offset >> 17;
	UINT32 result = m_port_ctrl_reg[newOffset];
	logerror("%s: port_ctrl_r newOffset = %02X data = %08X\n", machine().describe_context(), newOffset, result);
	return result;
}

WRITE32_MEMBER(atlantis_state::port_ctrl_w)
{
	UINT32 newOffset = offset >> 17;
	COMBINE_DATA(&m_port_ctrl_reg[newOffset]);

	switch (newOffset) {
	//case 1:
	//	m_port_ctrl_reg[newOffset] = 0;
	//	if (!(data & 0x8))
	//		m_port_ctrl_reg[newOffset] = 0*3; // Row 0
	//	else if (!(data & 0x10))
	//		m_port_ctrl_reg[newOffset] = 1*3; // Row 1
	//	else if (!(data & 0x20))
	//		m_port_ctrl_reg[newOffset] = 2*3; // Row 2
	//	else if (!(data & 0x40))
	//		m_port_ctrl_reg[newOffset] = 3*3; // Row 3
	//	logerror("%s: port_ctrl_w Keypad Row Sel = %04X data = %08X\n", machine().describe_context(), m_port_ctrl_reg[newOffset], data);
	//	break;
	default:
		logerror("%s: port_ctrl_w write to offset %04X = %08X & %08X bus offset = %08X\n", machine().describe_context(), newOffset, data, mem_mask, offset);
		break;
	}
}

CUSTOM_INPUT_MEMBER(atlantis_state::port_mod_r)
{
	UINT32 bits = ioport((const char *)param)->read();
	//bits &= m_port_ctrl_reg[1];
	//bits >>= m_port_ctrl_reg[1];
	logerror("%s: port_mod_r read data %s = %08X m_port_ctrl_reg[1] = %08X\n", machine().describe_context(), (const char *)param, bits, m_port_ctrl_reg[1]);
	return bits;
}

/*************************************
 *  Video refresh
 *************************************/
UINT32 atlantis_state::screen_update_mwskins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
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
	save_item(NAME(m_irq_state));
	save_item(NAME(board_ctrl));
	save_item(NAME(m_port_ctrl_reg));
}



/*************************************
*  Machine init
*************************************/
void atlantis_state::machine_reset()
{
	m_dcs->reset_w(1);
	m_dcs->reset_w(0);
	m_user_io_state = 0;
	m_cmos_write_enabled = FALSE;
	memset(m_zeus_data, 0, sizeof(m_zeus_data));
	m_red_count = 0;
	m_irq_state = 0;
	memset(board_ctrl, 0, sizeof(board_ctrl));
	memset(m_port_ctrl_reg, 0, sizeof(m_port_ctrl_reg));
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( map0, AS_PROGRAM, 32, atlantis_state )
	AM_RANGE(0x00000000, 0x00000fff) AM_READWRITE8(red_r, red_w, 0xff)
	AM_RANGE(0x0001e000, 0x0001ffff) AM_READWRITE8(cmos_r, cmos_w, 0xff)
	AM_RANGE(0x00100000, 0x0010001f) AM_DEVREADWRITE8("uart0", ns16550_device, ins8250_r, ins8250_w, 0xff) // Serial UART0 (TL16C552 CS0)
	AM_RANGE(0x00180000, 0x0018001f) AM_DEVREADWRITE8("uart1", ns16550_device, ins8250_r, ins8250_w, 0xff) // Serial UART1 (TL16C552 CS1)
	//AM_RANGE(0x00200000, 0x0020001f) // Parallel UART (TL16C552 CS2)
	AM_RANGE(0x00400000, 0x004000bf) AM_READWRITE8(blue_r, blue_w, 0xff)
	AM_RANGE(0x00880000, 0x00c80003) AM_READWRITE(board_ctrl_r, board_ctrl_w)
	//AM_RANGE(0x00880000, 0x00880003) // Sub-module Power0
	//AM_RANGE(0x00900000, 0x00900003) // Sub_module Power1 (Zeus or vblank?)
	//AM_RANGE(0x00980000, 0x00980003) // IRQ1 Enable
	//AM_RANGE(0x00a00000, 0x00a00003) // IRQ2 Enable
	//AM_RANGE(0x00a80000, 0x00a80003) // IRQ3 Enable
	//AM_RANGE(0x00b00000, 0x00b00003) // IRQ4 Enable Not Seen (Hardcoded to IDE?)
	//AM_RANGE(0x00b80000, 0x00b80003) // IRQ Global Enable
	//AM_RANGE(0x00c00000, 0x00c00003) // IRQ Cause
	//AM_RANGE(0x00c80000, 0x00c80003) // IRQ Status
	AM_RANGE(0x00d80000, 0x00d80003) AM_READWRITE(status_leds_r, status_leds_w)
	AM_RANGE(0x00e00000, 0x00e00003) AM_READWRITE(cmos_protect_r, cmos_protect_w)
	AM_RANGE(0x00e80000, 0x00e80003) AM_NOP // Watchdog?
ADDRESS_MAP_END

static ADDRESS_MAP_START( map1, AS_PROGRAM, 32, atlantis_state )
	AM_RANGE(0x00000000, 0x0000003f) AM_DEVREADWRITE("ioasic", midway_ioasic_device, read, write)
	// asic_fifo_w
	// dcs3_fifo_full_w
	//AM_RANGE(0x00200000, 0x00200003)
	AM_RANGE(0x00400000, 0x00400003) AM_DEVWRITE("dcs", dcs_audio_device, dsio_idma_addr_w)
	AM_RANGE(0x00600000, 0x00600003) AM_DEVREADWRITE("dcs", dcs_audio_device, dsio_idma_data_r, dsio_idma_data_w)
	//AM_RANGE(0x00800000, 0x00800003) AM_WRITE(dcs3_fifo_full_w)
	//AM_RANGE(0x00800000, 0x00800003) AM_WRITE(asic_fifo_w)
	//AM_RANGE(0x00800000, 0x00a00003) AM_READWRITE(port_ctrl_r, port_ctrl_w)
	//AM_RANGE(0x00800000, 0x00800003) // Written once = 0000fff8
	//AM_RANGE(0x00880000, 0x00880003) // Initial write 0000fff0, follow by sequence ffef, ffdf, ffbf, fff7. Row Select?
	//AM_RANGE(0x00900000, 0x00900003) // Read once before each sequence write to 0x00880000. Code checks bits 0,1,2. Keypad?
	//AM_RANGE(0x00980000, 0x00980003) // Read / Write.  Bytes written 0x8f, 0xcf. Code if read 0x1 then read 00a00000. POTs?
	//AM_RANGE(0x00a00000, 0x00a00003)
	ADDRESS_MAP_END

static ADDRESS_MAP_START(map2, AS_PROGRAM, 32, atlantis_state)
	AM_RANGE(0x00000000, 0x000001ff) AM_READWRITE(zeus_r, zeus_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( map3, AS_PROGRAM, 32, atlantis_state )
	//AM_RANGE(0x000000, 0xffffff) AM_READWRITE(blue_r, blue_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( mwskins )
	PORT_START("DIPS")
	PORT_DIPNAME(0x0003, 0x0003, "Boot Mode")
	PORT_DIPSETTING(0x0003, "Normal Boot")
	PORT_DIPSETTING(0x0002, "Boot EEPROM Based Self Test")
	PORT_DIPSETTING(0x0001, "Boot Disk Based Self Test")
	PORT_DIPSETTING(0x0000, "Run Factory Tests")
	PORT_DIPNAME(0x0004, 0x0004, "Unknown0004")
	PORT_DIPSETTING(0x0004, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0008, 0x0008, "Unknown0008")
	PORT_DIPSETTING(0x0008, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0010, 0x0010, "Unknown0010")
	PORT_DIPSETTING(0x0010, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0020, 0x0020, "Unknown0020")
	PORT_DIPSETTING(0x0020, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0040, 0x0040, "Unknown0040")
	PORT_DIPSETTING(0x0040, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0080, 0x0080, "Unknown0080")
	PORT_DIPSETTING(0x0080, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0100, 0x0100, "Unknown0100")
	PORT_DIPSETTING(0x0100, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0200, 0x0200, "Unknown0200")
	PORT_DIPSETTING(0x0200, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0400, 0x0400, "Unknown0400")
	PORT_DIPSETTING(0x0400, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0800, 0x0800, "Unknown0800")
	PORT_DIPSETTING(0x0800, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x1000, 0x1000, "Unknown1000")
	PORT_DIPSETTING(0x1000, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x2000, 0x2000, "Unknown2000")
	PORT_DIPSETTING(0x2000, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x4000, 0x4000, "Unknown4000")
	PORT_DIPSETTING(0x4000, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x8000, 0x8000, "Unknown8000")
	PORT_DIPSETTING(0x8000, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))

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

	PORT_START("KEYPAD")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)   /* keypad 3 */
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)   /* keypad 1 */
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)   /* keypad 2 */
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)   /* keypad 6 */
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)   /* keypad 4 */
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)   /* keypad 5 */
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)   /* keypad 9 */
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)   /* keypad 7 */
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)   /* keypad 8 */
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad #") PORT_CODE(KEYCODE_PLUS_PAD)    /* keypad # */
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad *") PORT_CODE(KEYCODE_MINUS_PAD)   /* keypad * */
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)   /* keypad 0 */

INPUT_PORTS_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/
#define PCI_ID_NILE     ":pci:00.0"
#define PCI_ID_9050     ":pci:0b.0"
#define PCI_ID_IDE      ":pci:0c.0"

static MACHINE_CONFIG_START( mwskins, atlantis_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", VR4310LE, 166666666)    // clock is TRUSTED
	MCFG_MIPS3_ICACHE_SIZE(16384)
	MCFG_MIPS3_DCACHE_SIZE(16384)

	MCFG_PCI_ROOT_ADD(                ":pci")
	MCFG_VRC4373_ADD(                 PCI_ID_NILE, ":maincpu")
	MCFG_PCI9050_ADD(                 PCI_ID_9050)
	MCFG_PCI9050_SET_MAP(0, map0)
	MCFG_PCI9050_SET_MAP(1, map1)
	MCFG_PCI9050_SET_MAP(2, map2)
	MCFG_PCI9050_SET_MAP(3, map3)
	MCFG_PCI9050_USER_OUTPUT_CALLBACK(DEVWRITE32(":", atlantis_state, user_io_output))
	MCFG_PCI9050_USER_INPUT_CALLBACK(DEVREAD32(":", atlantis_state, user_io_input))
	
	MCFG_NVRAM_ADD_0FILL("rtc")

	MCFG_IDE_PCI_ADD(PCI_ID_IDE, 0x10950646, 0x03, 0x0)
	MCFG_IDE_PCI_IRQ_HANDLER(DEVWRITELINE(":", atlantis_state, ide_irq))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(atlantis_state, screen_update_mwskins)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BBBBBGGGGGRRRRR("palette")

	/* sound hardware */
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_DSIO, 0)
	//MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_DENVER, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(8)
	MCFG_DCS2_AUDIO_POLLING_OFFSET(0) /* no place to hook :-( */

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(MIDWAY_IOASIC_STANDARD)
	MCFG_MIDWAY_SERIAL_PIC2_YEAR_OFFS(80)
	MCFG_MIDWAY_IOASIC_UPPER(325)
	MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(WRITELINE(atlantis_state, ioasic_irq))
	MCFG_MIDWAY_IOASIC_AUTO_ACK(1)

	// TL16C552 UART
	MCFG_DEVICE_ADD("uart0", NS16550, XTAL_24MHz)
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE(":", atlantis_state, uart0_irq_callback))
	MCFG_DEVICE_ADD("uart1", NS16550, XTAL_24MHz)
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE(":", atlantis_state, uart1_irq_callback))

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

GAME( 2000, mwskins,    0,      mwskins, mwskins, atlantis_state,  mwskins,   ROT0, "Midway", "Skins Game (1.06)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2000, mwskinsa, mwskins,  mwskins, mwskins, atlantis_state,  mwskins,   ROT0, "Midway", "Skins Game (1.06, alt)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2000, mwskinso, mwskins,  mwskins, mwskins, atlantis_state,  mwskins,   ROT0, "Midway", "Skins Game (1.04)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
