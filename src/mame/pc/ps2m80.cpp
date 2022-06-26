// license:BSD-3-Clause
// copyright-holders:Katherine Rohl

/**********************************************
 *
 * PS/2 Model 80
 * 
 * Type 1, 2401 error = BIOS is calculating display height incorrectly.
 * 	Error occurs at E000:2824 in the type 1 BIOS
 * 
 * The CuRAM-16 doesn't work in a Model 80 due to the memory split logic.
 * 
 * Type 1: 
 * 	- Baseline configuration.
 *	- Requires at least 1MB of planar RAM present to POST.
 * 	- 2MB RAM cards are not supported.
 * 	- Parallel port is not bidirectional.
 * 
 * Type 2:
 * 	- CPU speed upgraded to 40/2 = 20MHz.
 * 	- Upgraded memory controller
 * 		- Adds support for 2MB RAM cards, 1MB RAM still the minimum.
 * 		- The BIOS region (E0000-FFFF0h) can now be shadowed, with the top 128K of planar RAM always mapped there.
 * 			- When the BIOS ROM is enabled, writes to that region go to RAM.
 * 			- When the BIOS ROM is disabled, writes are disabled and reads go to RAM.
 * 	- The UART now supports FIFO mode.
 *  - Parallel port is not bidirectional.
 * 
 * Type 3:
 * 	- Complete redesign compared to Types 1 and 2.
 *
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
#include "cpu/i386/i386.h"
#include "formats/pc_dsk.h"
#include "speaker.h"
#include "xtal.h"

#include "ps2m80.h"

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

DEFINE_DEVICE_TYPE(PS2_M80_T1_MB,   ps2_m80_t1_mb_device,   "ps2_m80_t1_mb",    "PS/2 Model 80 Type 1 Planar")
DEFINE_DEVICE_TYPE(PS2_M80_T2_MB,   ps2_m80_t2_mb_device,   "ps2_m80_t2_mb",    "PS/2 Model 80 Type 2 Planar")
DEFINE_DEVICE_TYPE(PS2_M80_T3_MB,   ps2_m80_t3_mb_device,   "ps2_m80_t3_mb",    "PS/2 Model 80 Type 3 Planar")

void ps2_m80_t1_mb_device::map(address_map &map)
{
	ps2_mb_device::map(map);

	map(0x0061, 0x0061).rw(FUNC(ps2_m80_t1_mb_device::portb_r), FUNC(ps2_m80_t1_mb_device::portb_w));
	map(0x00e0, 0x00e0).rw(FUNC(ps2_m80_t1_mb_device::split_address_r), FUNC(ps2_m80_t1_mb_device::split_address_w));
	map(0x00e1, 0x00e1).rw(FUNC(ps2_m80_t1_mb_device::memory_encoding_r), FUNC(ps2_m80_t1_mb_device::memory_encoding_w));
}

void ps2_m80_t1_mb_device::device_start()
{
	ps2_mb_device::device_start();

	m_split_address_reg = 0xff;
	m_memory_encoding_reg = 0xff;

	m_memory_split_active = false;
	m_memory_split_base = 0;
	m_split_size = 0;

	m_sram = make_unique_clear<uint8_t[]>(m_sram_size);
	m_nvram->set_base(m_sram.get(), m_sram_size);
	save_pointer(NAME(m_sram), m_sram_size);

	// Map RAM above 1MB. Don't map RAM between 640K and 1MB.
	if (m_ram->size() > 0x100000)
	{
		LOG("%s: install RAM\n", FUNCNAME);
		address_space& space = m_maincpu->space(AS_PROGRAM);
		offs_t ram_limit = m_ram->size() - 0x100000;
		space.install_ram(0x100000, 0x100000 + ram_limit - 1, m_ram->pointer() + 0x100000);
	}
}

void ps2_m80_t1_mb_device::device_reset()
{
	ps2_mb_device::device_reset();
	
    if (m_memory_split_active) disable_memory_split();
	m_memory_encoding_reg = 0b11111111;
    update_memory_split();
}

void ps2_m80_t1_mb_device::disable_memory_split()
{
	LOG("%s: Disabling memory split at %06X\n", FUNCNAME, m_memory_split_base);

    address_space& space = m_maincpu->space(AS_PROGRAM);

    space.unmap_readwrite(m_memory_split_base, m_memory_split_base + m_split_size - 1);
	m_memory_split_active = false;
}

void ps2_m80_t1_mb_device::device_add_mconfig(machine_config &config)
{
	ps2_mb_device::device_add_mconfig(config);

	add_mca32(config);
	add_dmac_72x7377(config);
	add_southbridge_72x8299(config);

	MCA16_SLOT(config, m_mcaslot[0], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA16_SLOT(config, m_mcaslot[1], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA16_SLOT(config, m_mcaslot[2], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA16_SLOT(config, m_mcaslot[3], 0, m_mcabus, pc_mca16_cards, nullptr, false);
	MCA32_SLOT(config, m_mcaslot[4], 0, m_mcabus, pc_mca16_cards, nullptr, false);   // 32
	MCA16_SLOT(config, m_mcaslot[5], 0, m_mcabus, pc_mca16_cards, nullptr, false);   // with AVE
	MCA32_SLOT(config, m_mcaslot[6], 0, m_mcabus, pc_mca16_cards, nullptr, false);   // 32
	MCA32_SLOT(config, m_mcaslot[7], 0, m_mcabus, pc_mca32_cards, nullptr, false);   // 32
}

void ps2_m80_t1_mb_device::device_config_complete()
{
	ps2_mb_device::device_config_complete();
}

uint8_t ps2_m80_t1_mb_device::split_address_r()
{
	if(!machine().side_effects_disabled()) LOGPOS("%s\n", FUNCNAME);
	return m_split_address_reg;
}

void ps2_m80_t1_mb_device::split_address_w(uint8_t data)
{
	if(!machine().side_effects_disabled()) LOGPOS("*** %s: place split memory at %06X (%02X)\n", FUNCNAME, data * 0x100000, data & 0xf);
	m_split_address_reg = data;

	update_memory_split();
}

uint8_t ps2_m80_t1_mb_device::memory_encoding_r()
{
	if(!machine().side_effects_disabled()) LOGPOS("%s\n", FUNCNAME);
	return m_memory_encoding_reg;
}

void ps2_m80_t1_mb_device::memory_encoding_w(uint8_t data)
{
	if(!machine().side_effects_disabled()) LOGPOS("*** %s: %02X (split? %d; split 1MB at %d; ROMEN %d)\n", FUNCNAME, data,
		!BIT(data, 3),
		BIT(data, 2) ? 512 : 640,
		BIT(data, 1));

	m_memory_encoding_reg = data;

	update_memory_split();
}

void ps2_m80_t1_mb_device::update_memory_split()
{
	/*
		Type 1 motherboard memory split configuration is as follows:
		- The configuration of the first 1MB of memory is configured by the MER at I/O port 0E1h.
		- Bit 3
			- When asserted:	Memory between 512K or 640K is disabled.
			- When cleared:		Memory above 512K or 640K and below 1M is moved to the location specified in the Split Address Register.
		- Bit 2
			- When asserted: 	There is 512K of contiguous memory at 00000h to 7FFFFh.
			- When cleared:  	There is 640K of contiguous memory at 00000h to 9FFFFh.
		- Bit 1
			- When asserted: 	Address space E0000h to FFFFFh is mapped to the BIOS.
			- When cleared:  	Address space E0000h to FFFFFh is mapped to RAM.
								Everything between 512K or 640K and 896K is unused.
	 */

	//LOGPOS("%s: memory encoding reg %02X\n", FUNCNAME, m_memory_encoding_reg);

	static offs_t current_memory_split_end = 0x9ffff;

	// Is shadow RAM enabled at entry into update_memory_split()?
	static bool shadow_ram_enabled = false;
	// Is the memory split enabled at entry into update_memory_split()?
	static bool high_memory_enabled = false;

	address_space& space = m_maincpu->space(AS_PROGRAM);

	// We've just entered this function. Is the memory split enabled?
	if (high_memory_enabled)
	{
		// Unmap the existing block if so.
		//LOGPOS("disabling split at %06X-%06X\n", m_memory_split_base, m_memory_split_base+m_split_size-1);
		space.unmap_readwrite(m_memory_split_base, m_memory_split_base + m_split_size - 1);
        high_memory_enabled = false;
	}

	// Step 1: Always map low memory.
	offs_t conventional_memory_split = BIT(m_memory_encoding_reg, 2) ? 0x7ffff : 0x9ffff;
	space.unmap_readwrite(0, current_memory_split_end);
	space.install_ram(0, conventional_memory_split, m_ram->pointer());

	// Step 2: Are we mapping ROM?
	if (BIT(m_memory_encoding_reg, 1))
	{
		// Asserted: E0000h is still the BIOS.
		if(shadow_ram_enabled)
		{
			//LOGPOS("mapping ROM at %06X-%06X\n", 0xe0000, 0xfffff);
			// RAM to ROM.
			space.unmap_readwrite(0xe0000, 0xfffff);
			space.install_rom(0xe0000, 0xfffff, memregion(":bios")->base());
		}
		else
		{
			// ROM to ROM.
		}
	}
	else
	{
		// Cleared: E0000h-FFFFFh is RAM.
		if(shadow_ram_enabled)
		{
			// RAM to RAM.
		}
		else
		{
			//LOGPOS("mapping RAM at 0xe0000, 0xfffff\n");
			// ROM to RAM.
			space.unmap_read(0xe0000, 0xfffff);
			space.install_ram(0xe0000, 0xfffff, m_ram->pointer() + conventional_memory_split + 1);
		}
	}

	current_memory_split_end = conventional_memory_split;
	shadow_ram_enabled = BIT(m_memory_encoding_reg, 1);

	// Are we disabling the split?
	if(!BIT(m_memory_encoding_reg, 3))
	{
        if(high_memory_enabled)
        {
            // On to on.
        }
        else
        {
            // Off to on.
            m_memory_split_base = (m_split_address_reg & 0x0F) * 0x100000;
            m_split_size = BIT(m_memory_encoding_reg, 2) ? 0x80000 : 0x60000;

            assert(m_memory_split_base != 0);

            LOGPOS("%s: Memory split now %06X-%06X\n", FUNCNAME, m_memory_split_base, m_memory_split_base + m_split_size);

            if (m_ram->size() > 0xa0000)
            {
                offs_t ram_limit = m_memory_split_base + m_split_size;
                space.install_ram(m_memory_split_base, ram_limit-1, m_ram->pointer() + 0x100000 - m_split_size);
            }
        }
	}
	else
	{
        if(high_memory_enabled)
        {
            // On to off.
            //LOGPOS("%s: Memory split now disabled\n", FUNCNAME);
            offs_t ram_limit = m_memory_split_base + m_split_size;
		    space.unmap_readwrite(m_memory_split_base, ram_limit-1);
        }
        else
        {
            // Off to off.
        }
	}

	high_memory_enabled = !BIT(m_memory_encoding_reg, 3);
}

uint8_t ps2_m80_t1_mb_device::planar_pos_r(offs_t offset)
{
	LOGPOS("%s: O:%02X\n", FUNCNAME, offset);

	switch(offset)
	{
		case MCABus::POS::ADAPTER_ID_LO: return m_planar_id & 0xFF;
		case MCABus::POS::ADAPTER_ID_HI: return (m_planar_id & 0xFF00) >> 8;
		case MCABus::POS::OPTION_SELECT_DATA_1: return m_io_controller->system_board_io_r();
		case MCABus::POS::OPTION_SELECT_DATA_2:
		{
			if      (m_ram->size() == 0x100000) return 0b00001100;
			else if (m_ram->size() == 0x200000) return 0b00000000;
			else    fatalerror("RAM size not supported by planar");
		}
		default: return 0xFF;
	}
}

void ps2_m80_t1_mb_device::planar_pos_w(offs_t offset, uint8_t data)
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
			m_memory_control = data;
			
			m_timer_slow_refresh->enable(0);
			m_timer_fast_refresh->enable(0);

			if(BIT(m_memory_control, 1))
			{
				LOGPOS("PS/2 slow refresh\n");
				m_timer_slow_refresh->enable(1);
			}
			else
			{
				LOGPOS("PS/2 fast refresh\n");
				m_timer_fast_refresh->enable(1);
			}

			break;
		default:
			LOGPOS("Unhandled planar POS write: reg %02X data %02X\n", offset, data);
	}
}

/************************************************************/
/* Type 2 planar											*/
/************************************************************/
void ps2_m80_t2_mb_device::device_start()
{
	ps2_m80_t1_mb_device::device_start();
}

void ps2_m80_t2_mb_device::device_reset()
{
	ps2_mb_device::device_reset();
	
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe0000, 0xfffff, write8sm_delegate(*this, FUNC(ps2_m80_t2_mb_device::shadow_bios_w)));

    if (m_memory_split_active) disable_memory_split();
	m_memory_encoding_reg = 0b11111111;
    update_memory_split();

	// Install shadow BIOS write region.
}

void ps2_m80_t2_mb_device::map(address_map &map)
{
	ps2_mb_device::map(map);

	map(0x0061, 0x0061).rw(FUNC(ps2_m80_t2_mb_device::portb_r), FUNC(ps2_m80_t2_mb_device::portb_w));
	map(0x00e0, 0x00e0).rw(FUNC(ps2_m80_t2_mb_device::split_address_r), FUNC(ps2_m80_t2_mb_device::split_address_w));
	map(0x00e1, 0x00e1).rw(FUNC(ps2_m80_t2_mb_device::memory_encoding_r), FUNC(ps2_m80_t2_mb_device::memory_encoding_w));
}

uint8_t ps2_m80_t2_mb_device::planar_pos_r(offs_t offset)
{
	LOGPOS("%s: O:%02X\n", FUNCNAME, offset);

	switch(offset)
	{
		case MCABus::POS::ADAPTER_ID_LO: return m_planar_id & 0xFF;
		case MCABus::POS::ADAPTER_ID_HI: return (m_planar_id & 0xFF00) >> 8;
		case MCABus::POS::OPTION_SELECT_DATA_1: return m_io_controller->system_board_io_r();
		case MCABus::POS::OPTION_SELECT_DATA_2:
		{
			if      (m_ram->size() == 0x200000) return 0b11110010;
			else if (m_ram->size() == 0x400000) return 0b11111010;
			else    fatalerror("RAM size not supported by planar");
		}
		default: return 0xFF;
	}
}

void ps2_m80_t2_mb_device::planar_pos_w(offs_t offset, uint8_t data)
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
			m_memory_control = data;
			
			m_timer_slow_refresh->enable(0);
			m_timer_fast_refresh->enable(0);

			if(BIT(m_memory_control, 1))
			{
				LOGPOS("PS/2 slow refresh\n");
				m_timer_slow_refresh->enable(1);
			}
			else
			{
				LOGPOS("PS/2 fast refresh\n");
				m_timer_fast_refresh->enable(1);
			}

			break;
		default:
			LOGPOS("Unhandled planar POS write: reg %02X data %02X\n", offset, data);
	}
}

uint8_t ps2_m80_t2_mb_device::memory_encoding_r()
{
	if(!machine().side_effects_disabled()) LOGPOS("%s\n", FUNCNAME);
	return m_memory_encoding_reg;
}

void ps2_m80_t2_mb_device::memory_encoding_w(uint8_t data)
{
	if(!machine().side_effects_disabled()) LOGPOS("*** %s: %02X (split? %d; split 1MB at %d, ROMEN %d)\n", FUNCNAME, data,
		!BIT(data, 3),
		BIT(data, 2) ? 512 : 640,
		BIT(data, 1));

	m_memory_encoding_reg = data;

	update_memory_split();
}

uint8_t ps2_m80_t2_mb_device::split_address_r()
{
	if(!machine().side_effects_disabled()) LOGPOS("%s\n", FUNCNAME);
	return m_split_address_reg;
}

void ps2_m80_t2_mb_device::split_address_w(uint8_t data)
{
	// TODO: Type 2 can enable/disable the planar RAM cards with bit 4/bit 5

	if(!machine().side_effects_disabled()) LOGPOS("*** %s: place split memory at %06X (%02X)\n", FUNCNAME, (data & 0xf) * 0x100000, data & 0xf);
	m_split_address_reg = data;

	update_memory_split();
}

uint8_t ps2_m80_t2_mb_device::shadow_bios_r(offs_t offset)
{
	// A read to this region always goes to the top of onboard RAM.
	// LOGSHADOW("%s: reading %04X from %06X\n", FUNCNAME, m_ram->read(offset+0xe0000), offset+0xe0000);

	return m_ram->read(offset+0xe0000);
}

void ps2_m80_t2_mb_device::shadow_bios_w(offs_t offset, uint8_t data)
{
	// A write to this region always goes to the top of onboard RAM.
	// LOGSHADOW("%s: writing %04X to %06X\n", FUNCNAME, data, offset+0xe0000);

	m_ram->write(offset+0xe0000, data);
}

void ps2_m80_t2_mb_device::update_memory_split()
{
	/*
		Type 2 motherboard memory split configuration is as follows:
		- The configuration of the first 1MB of memory is configured by the MER at I/O port 0E1h.
		- Bit 3
			- When asserted:	Memory between 512K or 640K is disabled.
			- When cleared:		Memory above 512K or 640K and below 1M is moved to the location specified in the Split Address Register.
		- Bit 2
			- When asserted: 	There is 512K of contiguous memory at 00000h to 7FFFFh.
			- When cleared:  	There is 640K of contiguous memory at 00000h to 9FFFFh.
		- Bit 1
			- When asserted: 	Address space E0000h to FFFFFh is mapped to the BIOS.
			- When cleared:  	Address space E0000h to FFFFFh is mapped to RAM.
			- In either case, writes to E0000h-FFFFFh are always mapped to the top 128K of the split memory.
	 */

	//LOGPOS("%s: memory encoding reg %02X\n", FUNCNAME, m_memory_encoding_reg);

	static offs_t current_memory_split_end = 0x9ffff;

	// Is the memory split enabled at entry into update_memory_split()?
	static bool high_memory_enabled = false;

	address_space& space = m_maincpu->space(AS_PROGRAM);

	// We've just entered this function. Is the memory split enabled?
	if (high_memory_enabled)
	{
		// Unmap the existing block if so.
		//LOGPOS("disabling split at %06X-%06X\n", m_memory_split_base, m_memory_split_base+m_split_size-1);
		space.unmap_readwrite(m_memory_split_base, m_memory_split_base + m_split_size - 1);
        high_memory_enabled = false;
	}

	// Step 1: Always map low memory.
	offs_t conventional_memory_split = BIT(m_memory_encoding_reg, 2) ? 0x7ffff : 0x9ffff;
	space.unmap_readwrite(0, current_memory_split_end);
	space.install_ram(0, conventional_memory_split, m_ram->pointer());

	// Step 2: Are we mapping ROM?
	if(BIT(m_memory_encoding_reg, 1))
	{
		// ROMEN 1: Reads go to ROM, writes go to RAM.
		LOGSHADOW("%s: reads to ROM, writes to RAM\n", FUNCNAME);
		space.unmap_readwrite(0xe0000, 0xfffff);
		space.install_rom(0xe0000, 0xfffff, memregion(":bios")->base());
		space.install_write_handler(0xe0000, 0xfffff, write8sm_delegate(*this, FUNC(ps2_m80_t2_mb_device::shadow_bios_w)));
	}
	else
	{
		// ROMEN 0: Reads go to RAM, writes go nowhere.
		LOGSHADOW("%s: reads to RAM, writes disabled\n", FUNCNAME);
		space.unmap_readwrite(0xe0000, 0xfffff);
		space.install_read_handler(0xe0000, 0xfffff, read8sm_delegate(*this, FUNC(ps2_m80_t2_mb_device::shadow_bios_r)));
	}

	current_memory_split_end = conventional_memory_split;

	// Are we disabling the split?
	if(!BIT(m_memory_encoding_reg, 3))
	{
        if(high_memory_enabled)
        {
            // On to on.
        }
        else
        {
            // Off to on.
            m_memory_split_base = (m_split_address_reg & 0x0F) * 0x100000;
 
            m_split_size = BIT(m_memory_encoding_reg, 2) ? 0x60000 : 0x40000;

            assert(m_memory_split_base != 0);

            LOGPOS("%s: Memory split now %06X-%06X\n", FUNCNAME, m_memory_split_base, m_memory_split_base + m_split_size);

            if (m_ram->size() > 0xa0000)
            {
                offs_t ram_limit = m_memory_split_base + m_split_size;
                space.install_ram(m_memory_split_base, ram_limit-1, m_ram->pointer() + 0x100000 - 0x20000 - m_split_size);
            }
        }
	}
	else
	{
        if(high_memory_enabled)
        {
            // On to off.
            //LOGPOS("%s: Memory split now disabled\n", FUNCNAME);
            offs_t ram_limit = m_memory_split_base + m_split_size;
		    space.unmap_readwrite(m_memory_split_base, ram_limit-1);
        }
        else
        {
            // Off to off.
        }
	}

	high_memory_enabled = !BIT(m_memory_encoding_reg, 3);
}
