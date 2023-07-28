// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    IBM 72X8299 I/O controller
    Provides the interface between the Micro Channel bus and planar peripherals.
	Effectively the southbridge chip, controlling the onboard peripherals.

	From the perspective of the BIOS, the 72X8299 is an MCA device, in charge
	of the System Board device.
	
	Controls:
	- One 16550 UART
	- One 72X8287 VGA controller
	- One Centronics parallel port
	- One uPD72065 FDC
	- Two PITs, AT-compatible

***************************************************************************/

#include "emu.h"
#include "bus/mca/mca.h"
#include "bus/mca/mca_cards.h"
#include "bus/mca/planar_uart.h"
#include "bus/mca/planar_lpt.h"
#include "bus/mca/planar_fdc.h"
#include "bus/mca/planar_vga.h"
#include "bus/mca/ibm72x8299.h"
#include "machine/ibmps2.h"

#define LOG_PORT80  0

#define LOG_SYSPORTS    (1U <<  2)
#define LOG_NVRAM       (1U <<  3)
#define LOG_TIMERS      (1U <<  4)
#define LOG_POST        (1U <<  5)
#define LOG_POS         (1U <<  6)

#define VERBOSE (LOG_SYSPORTS|LOG_NVRAM|LOG_TIMERS|LOG_POST|LOG_POS)
#include "logmacro.h"

#define LOGSYSPORTS(...)    LOGMASKED(LOG_SYSPORTS, __VA_ARGS__)
#define LOGNVRAM(...)       LOGMASKED(LOG_NVRAM, __VA_ARGS__)
#define LOGTIMERS(...)      LOGMASKED(LOG_TIMERS, __VA_ARGS__)
#define LOGPOST(...)        LOGMASKED(LOG_POST, __VA_ARGS__)
#define LOGPOS(...)         LOGMASKED(LOG_POS, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define XTAL_U153   25.175_MHz_XTAL

DEFINE_DEVICE_TYPE(IBM72X8299, ibm72x8299_device, "ibm72x8299", "IBM 72X8299 Micro Channel I/O Controller")

ibm72x8299_device::ibm72x8299_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, ps2_mb_device *planar, mca16_device *bus)
	: ibm72x8299_device(mconfig, tag, owner, clock)
{
	m_planar = planar;
	m_mca = bus;
	m_planar_id = planar->get_planar_id();
}

ibm72x8299_device::ibm72x8299_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IBM72X8299, tag, owner, clock)
	, device_mca16_card_interface(mconfig, *this, m_planar_id)
    , m_planar_vga(*this, "planar_vga")
    , m_planar_fdc(*this, "planar_fdc")
    , m_planar_uart(*this, "planar_uart")
    , m_planar_lpt(*this, "planar_lpt")
    , m_pit(*this, "pit")
    , m_pit_ch_cb(*this)
{
}

void ibm72x8299_device::device_start()
{
    m_system_board_io_enable = 0;
    m_system_board_setup = 0xff;
}

void ibm72x8299_device::device_reset()
{
    m_system_board_io_enable = 0;
    m_system_board_setup = 0xff;        // page 2-32: resets to 0xFF
    m_adapter_setup = 0;
    
    m_cd_sfdbk = 0;
}

void ibm72x8299_device::device_add_mconfig(machine_config &config)
{
   	MCA16_SLOT(config, m_planar_vga,    0, *m_mca, pc_mca16_cards, "planar_vga", true);
	MCA16_SLOT(config, m_planar_fdc,    0, *m_mca, pc_mca16_cards, "planar_fdc", true);
	MCA16_SLOT(config, m_planar_uart,   0, *m_mca, pc_mca16_cards, "planar_uart", true);
	MCA16_SLOT(config, m_planar_lpt,    0, *m_mca, pc_mca16_cards, "planar_lpt", true);

	m_mca->cs_feedback_callback().set(*this, FUNC(ibm72x8299_device::cd_sfdbk_w));
	printf("assert card feedback r %p\n", m_mca);

    PS2_PIT(config, m_pit);
    m_pit->set_clk<0>(XTAL_U153 / 21); /* heartbeat IRQ */
	m_pit->out_handler<0>().set(FUNC(ibm72x8299_device::pit_ch0_w));
	m_pit->set_clk<1>(XTAL_U153 / 21); /* dram refresh */
	m_pit->set_clk<2>(XTAL_U153 / 21); /* pio port c pin 4 and speaker polling */
	m_pit->out_handler<2>().set(FUNC(ibm72x8299_device::pit_ch2_w));
	// CH 3 behavior: IRQ0 asserted enables the gate.
	// CLK IN 3 is tied to ~(CLK OUT 0).
	// If CLK OUT 3 is high, the NMI is asserted.
	// Effectively, it asserts NMI when IRQ 0 is active for more than one CH 0 tick.
	m_pit->ch3_out_handler().set(FUNC(ibm72x8299_device::pit_ch3_w));
}

void ibm72x8299_device::device_config_complete()
{

}

void ibm72x8299_device::device_resolve_objects()
{
	
}

/* PIT write lines and gates */
void ibm72x8299_device::pit_ch0_w(int state)
{
    m_pit_ch_cb[0](state);
}

void ibm72x8299_device::pit_ch2_w(int state)
{
    m_pit_ch_cb[2](state);
}

void ibm72x8299_device::pit_ch3_w(int state)
{
    m_pit_ch_cb[3](state);
}

void ibm72x8299_device::pit_ch2_gate_w(int state)
{
    m_pit->write_gate2(state);
}

void ibm72x8299_device::pit_ch3_gate_w(int state)
{
    m_pit->write_gate3(state);
}

void ibm72x8299_device::pit_ch3_clk_w(int state)
{
    m_pit->write_clk3(state);
}

// POS

uint8_t ibm72x8299_device::adapter_pos_r()
{
	if(!machine().side_effects_disabled()) LOGPOS("%s: %02X\n", FUNCNAME, m_adapter_setup | 0x70);
	return m_adapter_setup | 0x70; // Always OR with $70 when reading this.
}

void ibm72x8299_device::adapter_pos_w(uint8_t data)
{
	LOGPOS("%s: %02X\n", FUNCNAME, data);
	if(BIT(data, 3))
	{
		LOGPOS("%s: Selected channel %d for setup\n", FUNCNAME, (data & 7) + 1);
	}
	if(BIT(data, 7))
	{
		LOGPOS("%s: Channel reset asserted\n", FUNCNAME);
	}

	m_adapter_setup = data;
}


uint8_t ibm72x8299_device::system_board_pos_r()
{
	if(!machine().side_effects_disabled()) LOGPOS("%s: %02X\n", FUNCNAME, m_system_board_setup);
	return m_system_board_setup;
}

void ibm72x8299_device::system_board_pos_w(uint8_t data)
{
	if(!machine().side_effects_disabled()) LOGPOS("%s: %02X\n", FUNCNAME, data);
	m_system_board_setup = data;
}

std::string posreg_name(offs_t offset)
{
	switch(offset)
	{
		case 0: return "ADAPTER_ID_LO";
		case 1: return "ADAPTER_ID_HI";
		case 2: return "OPTION_SELECT_DATA_1";
		case 3: return "OPTION_SELECT_DATA_2";
		case 4: return "OPTION_SELECT_DATA_3";
		case 5: return "OPTION_SELECT_DATA_4";
		case 6: return "SUBADDRESS_EXT_LO";
		case 7:	return "SUBADDRESS_EXT_HI";
		default: return "(error)";
	}
}

uint8_t ibm72x8299_device::pos_registers_r(offs_t offset)
{
	// Programmable Option Select registers.
	// 100: adapter identification low
	// 101: adapter identification high
	// 102: option select data 1
	// 103: option select data 2
	// 104: option select data 3
	// 105: option select data 4
	// 106: subaddress extension low
	// 107: subaddress extension high

	// Special case: Reading the planar VGA's registers.
	if(BIT(m_system_board_setup, 5) == 0)
	{
		if(!machine().side_effects_disabled()) LOGPOS("%s: reading VGA POS register %s\n", FUNCNAME, posreg_name(offset));

		// VGA I/O byte.
		if(offset == MCABus::POS::OPTION_SELECT_DATA_1)
			return dynamic_cast<mca16_planar_vga_device *>(m_planar_vga->get_card_device())->sleep_r();
		else return 0xFF;
	}
	// Special case: Reading the planar's registers rather than an adapter.
	if(!BIT(m_system_board_setup, 7))
	{
		if(!machine().side_effects_disabled()) LOGPOS("%s: reading planar POS register %s\n", FUNCNAME, posreg_name(offset));
		return m_planar->planar_pos_r(offset);
	}

	if(m_planar->get_mca_slot(m_adapter_setup & 7) != NULL)
	{
		auto card = dynamic_cast<device_mca16_card_interface *>(m_planar->get_mca_slot(m_adapter_setup & 7)->get_card_device());
		if(card)
		{
			uint8_t value = (card ? card->pos_r(offset) : 0xFF);
			LOGPOS("%s: reading adapter %d POS register %s: %02X\n", FUNCNAME, (m_adapter_setup & 7) + 1, posreg_name(offset), value);
			return value;
		}
		else
		{
			LOGPOS("%s: no card in slot %d\n", FUNCNAME, (m_adapter_setup & 7) + 1);
			return 0xFF;
		}
	}
	else return 0xFF;
}

void ibm72x8299_device::pos_registers_w(offs_t offset, uint8_t data)
{

	if(BIT(m_system_board_setup, 5) == 0)
	{
		if(!machine().side_effects_disabled()) LOGPOS("%s: writing VGA POS register %s: %02X\n", FUNCNAME, posreg_name(offset), data);

		// VGA I/O byte.
		if(offset == MCABus::POS::OPTION_SELECT_DATA_1)
			dynamic_cast<mca16_planar_vga_device *>(m_planar_vga->get_card_device())->sleep_w(data);
		return;
	}

	if(BIT(m_system_board_setup, 7) == 0)
	{
		if(!machine().side_effects_disabled()) LOGPOS("%s: writing planar POS register %s: %02X\n", FUNCNAME, posreg_name(offset), data);

		m_planar->planar_pos_w(offset, data);
		return;
	}

	if(m_planar->get_mca_slot(m_adapter_setup & 7) != NULL)
	{
		auto card = dynamic_cast<device_mca16_card_interface *>(m_planar->get_mca_slot(m_adapter_setup & 7)->get_card_device());

		if(card)
		{
			LOGPOS("%s: writing adapter %d POS register %s: %02X\n", FUNCNAME, (m_adapter_setup & 7) + 1, posreg_name(offset), data);
			card->pos_w(offset, data);
		}
		else
		{
			LOGPOS("%s: no card in slot %d\n", FUNCNAME, (m_adapter_setup & 7) + 1);
		}
	}
}

void ibm72x8299_device::system_board_io_w(uint8_t data)
{
	LOGPOS("%s\n", FUNCNAME);

	m_system_board_io_enable = data;

	// Bit 7: Parallel port is bidirectional when asserted.

	// Bits 5-6: LPT1/LPT2/LPT3?
	uint8_t lpt_select = (data & 0x60) >> 5;
	switch(lpt_select)
	{
		case 0:
			LOGPOS("%s: reassigning planar UART %p to LPT1\n", FUNCNAME, m_planar_lpt->get_card_device());
			dynamic_cast<mca16_planar_lpt_device *>(m_planar_lpt->get_card_device())->planar_remap(AS_IO, 0x3BC, 0x3BF);
			break;
		case 1:
			LOGPOS("%s: reassigning planar UART %p to LPT2\n", FUNCNAME, m_planar_lpt->get_card_device());
			dynamic_cast<mca16_planar_lpt_device *>(m_planar_lpt->get_card_device())->planar_remap(AS_IO, 0x378, 0x37B);
			break;
		case 2:
			LOGPOS("%s: reassigning planar UART %p to LPT3\n", FUNCNAME, m_planar_lpt->get_card_device());
			dynamic_cast<mca16_planar_lpt_device *>(m_planar_lpt->get_card_device())->planar_remap(AS_IO, 0x278, 0x27B);
			break;
		default:
			break;
	}

	// Bit 4: Parallel port enabled when asserted.
	if(BIT(m_system_board_io_enable, 4))
	{
		dynamic_cast<mca16_planar_lpt_device *>(m_planar_lpt->get_card_device())->enable();
	}
	else
	{
		dynamic_cast<mca16_planar_lpt_device *>(m_planar_lpt->get_card_device())->disable();
	}

	// Bit 3: COM1 or COM2?
	if(BIT(m_system_board_io_enable, 3) == 1)
	{
		LOGPOS("%s: reassigning planar UART %p to COM1\n", FUNCNAME, m_planar_uart->get_card_device());
		dynamic_cast<mca16_planar_uart_device *>(m_planar_uart->get_card_device())->planar_remap(AS_IO, 0x3F8, 0x3F8+7);
		dynamic_cast<mca16_planar_uart_device *>(m_planar_uart->get_card_device())->planar_remap_irq(4);
	}
	else
	{
		LOGPOS("%s: reassigning planar UART %p to COM2\n", FUNCNAME, m_planar_uart->get_card_device());
		dynamic_cast<mca16_planar_uart_device *>(m_planar_uart->get_card_device())->planar_remap(AS_IO, 0x2F8, 0x2F8+7);
		dynamic_cast<mca16_planar_uart_device *>(m_planar_uart->get_card_device())->planar_remap_irq(3);
	}

	// Bit 2: Serial port enabled when asserted.
	if(BIT(m_system_board_io_enable, 2))
	{
		dynamic_cast<mca16_planar_uart_device *>(m_planar_uart->get_card_device())->enable();
	}
	else
	{
		dynamic_cast<mca16_planar_uart_device *>(m_planar_uart->get_card_device())->disable();
	}

	// Bit 1: Floppy drive enabled when asserted.
	if(BIT(m_system_board_io_enable, 1))
	{
		LOGPOS("FDC enabled\n");
		dynamic_cast<mca16_planar_fdc_device *>(m_planar_fdc->get_card_device())->enable();
	}
	else
	{
		LOGPOS("FDC disabled\n");
		dynamic_cast<mca16_planar_fdc_device *>(m_planar_fdc->get_card_device())->disable();
	}

	// Bit 0: If cleared, all devices disabled.
	if(!BIT(m_system_board_io_enable, 0))
	{
		dynamic_cast<mca16_planar_fdc_device *>(m_planar_fdc->get_card_device())->disable();
		dynamic_cast<mca16_planar_uart_device *>(m_planar_uart->get_card_device())->disable();
		dynamic_cast<mca16_planar_lpt_device *>(m_planar_lpt->get_card_device())->disable();
	}
}

void ibm72x8299_device::cd_sfdbk_w(int state)
{
	// An MCA adapter asserts this on any I/O access.
	if(state) m_cd_sfdbk = true;
}

uint8_t ibm72x8299_device::card_select_feedback_r(offs_t offset)
{
	// Reading automatically resets the feedback bit.
	uint8_t feedback = m_cd_sfdbk;
	m_cd_sfdbk = 0;
	return feedback; // Return state as bit 0.
}