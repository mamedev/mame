// license:BSD-3-Clause
// copyright-holders:Katherine Rohl

/**********************************************
 *
 * PS/2 Model 50
 *  - 286-based system
 *  - 72X8299 Southbridge
 *  - 72X7377 DMAC
 *  - 72X7385 Bus Controller (contains the PICs, not sure what else it does)
 * 
 *  - Type 1 Planar
 *  -- Model FC, Submodel 04, Revision 00
 * 
 *  - Type 2 Planar
 *  -- Model FC, Submodel 04, Revision 03
 **********************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "bus/mca/mca.h"
#include "bus/mca/mca_cards.h"
#include "bus/mca/planar_uart.h"
#include "bus/mca/planar_lpt.h"
#include "bus/mca/planar_fdc.h"
#include "bus/mca/planar_vga.h"
#include "imagedev/floppy.h"
#include "machine/ibmps2.h"
#include "cpu/i86/i286.h"
#include "formats/pc_dsk.h"
#include "speaker.h"
#include "xtal.h"

#include "ps2m50.h"

#define LOG_PORT80  0

#define LOG_SYSPORTS    (1U <<  2)
#define LOG_NVRAM       (1U <<  3)
#define LOG_TIMERS      (1U <<  4)
#define LOG_POST        (1U <<  5)
#define LOG_POS         (1U <<  6)
#define LOG_SHADOW		(1U <<	7)

#define VERBOSE (LOG_SYSPORTS|LOG_NVRAM|LOG_TIMERS|LOG_POST|LOG_POS|LOG_SHADOW)
#include "logmacro.h"

#define LOGSYSPORTS(...)    LOGMASKED(LOG_SYSPORTS, __VA_ARGS__)
#define LOGNVRAM(...)       LOGMASKED(LOG_NVRAM, __VA_ARGS__)
#define LOGTIMERS(...)      LOGMASKED(LOG_TIMERS, __VA_ARGS__)
#define LOGPOST(...)        LOGMASKED(LOG_POST, __VA_ARGS__)
#define LOGPOS(...)         LOGMASKED(LOG_POS, __VA_ARGS__)
#define LOGSHADOW(...)      LOGMASKED(LOG_SHADOW, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

DEFINE_DEVICE_TYPE(PS2_M50_T1_MB,   ps2_m50_t1_mb_device,   "ps2_m50_t1_mb",    "PS/2 Model 50 Type 1 Planar")
DEFINE_DEVICE_TYPE(PS2_M60_MB,   	ps2_m60_mb_device,   	"ps2_m60_mb",   	"PS/2 Model 60 Planar")

void ps2_m50_t1_mb_device::map(address_map &map)
{
	ps2_mb_device::map(map);
}

void ps2_m50_t1_mb_device::device_start()
{
	ps2_mb_device::device_start();
	
    // Install System Board RAM above 640K at 1MB.
    // Channel RAM gets installed immediately above the end of System Board RAM.
    m_maincpu->space(AS_PROGRAM).install_ram(0x100000, 0x15ffff, m_ram->pointer() + 0xa0000);
}

void ps2_m50_t1_mb_device::device_reset()
{
	ps2_mb_device::device_reset();
}

void ps2_m50_t1_mb_device::device_add_mconfig(machine_config &config)
{
	printf("ibm8550 device_add_config...\n");
	ps2_mb_device::device_add_mconfig(config);

	add_mca16(config);
    add_dmac_72x7377(config);
	add_southbridge_72x8299(config);

	MCA16_SLOT(config, m_mcaslot[0], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA16_SLOT(config, m_mcaslot[1], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA16_SLOT(config, m_mcaslot[2], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA16_SLOT(config, m_mcaslot[3], 0, m_mcabus, pc_mca16_cards, nullptr, false);
}

void ps2_m50_t1_mb_device::device_config_complete()
{
	ps2_mb_device::device_config_complete();
	printf("ibm8550 device_add_config complete...\n");
}

uint8_t ps2_m50_t1_mb_device::planar_pos_r(offs_t offset)
{
	LOGPOS("%s: O:%02X\n", FUNCNAME, offset);

	switch(offset)
	{
		case MCABus::POS::ADAPTER_ID_LO: return m_planar_id & 0xFF;
		case MCABus::POS::ADAPTER_ID_HI: return (m_planar_id & 0xFF00) >> 8;
		case MCABus::POS::OPTION_SELECT_DATA_1: return m_io_controller->system_board_io_r();

		// TODO: Figure out what the memory bits are - type 1 can only ever have 1MB onboard (2x512K 30-pin)
		//		 this seems to get it seeing 1MB
		case MCABus::POS::OPTION_SELECT_DATA_2: return 0b01010100 | m_system_board_memory_enabled;
		default: return 0x00;
	}
}

void ps2_m50_t1_mb_device::planar_pos_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 2:
		// System Board I/O Byte.
			LOGPOS("PS/2 system board I/O enable bits now %02X\n", data);
			m_io_controller->system_board_io_w(data);
			break;
		case 3:
		// Memory Control Register
			LOGPOS("PS/2 memory control register now %02X\n", data);
			update_memory_control_register(data);
			break;
		default:
			LOGPOS("Unhandled planar POS write: reg %02X data %02X\n", offset, data);
	}
}

void ps2_m50_t1_mb_device::update_memory_control_register(uint8_t data)
{
    m_system_board_memory_enabled = BIT(data, 0);

    // The system board enable/disable bit controls the low 640K.
    if(m_system_board_memory_enabled)
    {
        m_maincpu->space(AS_PROGRAM).install_ram(0x000000, 0x09ffff, m_ram->pointer()); 
    }
    else
    {
        m_maincpu->space(AS_PROGRAM).unmap_readwrite(0x000000, 0x09ffff);
    }
}

/****************************************
 * Model 60
 * 
 * - Basically a Model 50 in a tower case.
 * - 8 slots means it gains 2KB of SRAM.
 * - Same BIOS ROM as the Model 50.
 ****************************************/

void ps2_m60_mb_device::device_start()
{
	ps2_m50_t1_mb_device::device_start();

	m_sram = make_unique_clear<uint8_t[]>(m_sram_size);
	m_nvram->set_base(m_sram.get(), m_sram_size);
	save_pointer(NAME(m_sram), m_sram_size);
}

void ps2_m60_mb_device::device_reset()
{
	ps2_m50_t1_mb_device::device_reset();
}

void ps2_m60_mb_device::device_add_mconfig(machine_config &config)
{
	ps2_mb_device::device_add_mconfig(config);

	add_mca16(config);
    add_dmac_72x7377(config);
	add_southbridge_72x8299(config);

	MCA16_SLOT(config, m_mcaslot[0], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA16_SLOT(config, m_mcaslot[1], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA16_SLOT(config, m_mcaslot[2], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA16_SLOT(config, m_mcaslot[3], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA16_SLOT(config, m_mcaslot[4], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA16_SLOT(config, m_mcaslot[5], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA16_SLOT(config, m_mcaslot[6], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA16_SLOT(config, m_mcaslot[7], 0, m_mcabus, pc_mca16_cards, nullptr, false);
}