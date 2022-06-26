// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    IBM 2-8MB Memory Expansion Adapter
    FRU 15F8292
	MCA ID $F7F7

    16-bit memory expansion card. 4 72-pin SIMM slots, 8MB max capacity.

***************************************************************************/

#include "emu.h"
#include "ibm_memory_exp_16.h"

#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA16_IBM_MEMORY_EXP, mca16_ibm_memory_exp_device, "mca_f7f7", "IBM 2-8MB Memory Expansion Adapter, 16-bit (@F7F7)")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_ibm_memory_exp_device::device_add_mconfig(machine_config &config)
{
	RAM(config, m_expansion_ram).set_default_size("2M");
}

//-------------------------------------------------
//  mca16_planar_lpt_device - constructor
//-------------------------------------------------

mca16_ibm_memory_exp_device::mca16_ibm_memory_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_ibm_memory_exp_device(mconfig, MCA16_IBM_MEMORY_EXP, tag, owner, clock)
{
}

mca16_ibm_memory_exp_device::mca16_ibm_memory_exp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, 0xf7f7),
	m_expansion_ram(*this, "expansion_ram")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_ibm_memory_exp_device::device_start()
{
    m_is_mapped = false;
    m_rom_base = 0;
    m_rom_enabled = false;
    m_ram_enabled = false;

    m_consecutive_reads = 0;

	// POS data 3 is the memory present bits, see https://github.com/86Box/86Box/commit/8e6497f01dcf1abe72aea1f317bbba6165e7d026
	set_mca_device();

    // TODO: POS
    m_mca->install_rom(this, 0xda000, 0xda000+0x1fff, "option");

    // What are these for, exactly?
    m_mca->install_device(0x31a0, 0x31a5, 
        read16sm_delegate(*this, FUNC(mca16_ibm_memory_exp_device::io_31a0_r)),
        write16sm_delegate(*this, FUNC(mca16_ibm_memory_exp_device::io_31a0_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_ibm_memory_exp_device::device_reset()
{
    LOG("%s\n", FUNCNAME);

	if(m_is_mapped) unmap();

	reset_option_select();
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] = 0x00;
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_3] = 0b00000001;  // low 4 bits are RAM presence detect?
	m_option_select[MCABus::POS::OPTION_SELECT_DATA_4] = 0x00;

    m_in_mode_e = false;
}

void mca16_ibm_memory_exp_device::remap()
{
    LOG("%s: %02X %02X %02X %02X\n", FUNCNAME,
        m_option_select[MCABus::POS::OPTION_SELECT_DATA_1],
        m_option_select[MCABus::POS::OPTION_SELECT_DATA_2],
        m_option_select[MCABus::POS::OPTION_SELECT_DATA_3],
        m_option_select[MCABus::POS::OPTION_SELECT_DATA_4]);

    bool rom_enabled = (m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] & 0b00100000) == 0b00100000;
    bool ram_enabled = BIT(m_option_select[MCABus::POS::OPTION_SELECT_DATA_1], 0);
    
    offs_t multiplier = (m_option_select[MCABus::POS::OPTION_SELECT_DATA_3] & 0b11110000) >> 4;
    offs_t rom_base = 0xc0000 + (0x2000 * multiplier);

    LOG("%s: rom_enabled %d ram_enabled %d multiplier %d\n", FUNCNAME, rom_enabled, ram_enabled, multiplier);

    if(multiplier >= 4 && multiplier <= 15)
    {
        if(rom_enabled)
        {
            m_mca->install_rom(this, rom_base, rom_base+0x1fff, "option");
            m_is_mapped = true;

            m_rom_base = rom_base;
            m_rom_enabled = rom_enabled;
        }

        if(ram_enabled)
        {
            // TODO: What tells the RAM expansion card how much system memory is present?
            m_mca->install_memory(0x160000, 0x15ffff+0x800000,
                read8sm_delegate(*m_expansion_ram, FUNC(ram_device::read)), write8sm_delegate(*m_expansion_ram, FUNC(ram_device::write)));
        }
    }
}

void mca16_ibm_memory_exp_device::unmap()
{
    m_mca->unmap_rom(m_rom_base, m_rom_base+0x1fff);

    if(m_ram_enabled)
    {
        m_mca->unmap_readwrite(0x160000, 0x15ffff+0x800000);
    }

	m_is_mapped = false;
}

bool mca16_ibm_memory_exp_device::map_has_changed()
{
    bool rom_enabled = (m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] & 0b00100110) == 0b00100000;
    
    offs_t multiplier = (m_option_select[MCABus::POS::OPTION_SELECT_DATA_3] & 0b11110000) >> 4;
    offs_t rom_base = 0xc0000 + (0x2000 * multiplier);

    bool ram_enabled = BIT(m_option_select[MCABus::POS::OPTION_SELECT_DATA_1], 0);

    return (m_rom_enabled != rom_enabled) || (rom_base != m_rom_base) || (ram_enabled != m_ram_enabled);
}

void mca16_ibm_memory_exp_device::update_pos_data_1(uint8_t data)
{
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] &= ~(0x3f);
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] |= data & 0x3f;
	if(map_has_changed())
    {
        if(m_is_mapped) unmap();
        remap();
    }
}

void mca16_ibm_memory_exp_device::update_pos_data_2(uint8_t data)
{
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] = data;
	if(map_has_changed())
    {
        if(m_is_mapped) unmap();
        remap();
    }
}

void mca16_ibm_memory_exp_device::update_pos_data_3(uint8_t data)
{
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_3] &= 0x0f;
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_3] |= (data & 0xf0);
	if(map_has_changed())
    {
        if(m_is_mapped) unmap();
        remap();
    }
}   

void mca16_ibm_memory_exp_device::update_pos_data_4(uint8_t data)
{
	// What do the bits do, exactly? 0x3F means enable, I think.

    m_option_select[MCABus::POS::OPTION_SELECT_DATA_4] &= ~0xc0;
	m_option_select[MCABus::POS::OPTION_SELECT_DATA_4] = data & 0xc0;
	LOG("%s: D:%02X\n", FUNCNAME, data);
	if(map_has_changed())
    {
        if(m_is_mapped) unmap();
        remap();
    }
}

void mca16_ibm_memory_exp_device::pos_w(offs_t offset, uint8_t data)
{
	LOG("%s\n", FUNCNAME);

	switch(offset)
	{
		case 0:
			// Adapter Identification b0-b7
            break;
		case 1:
			// Adapter Identification b8-b15
			break;
		case 2:
			// Option Select Data 1
            update_pos_data_1(data);
			break;
		case 3:
			// Option Select Data 2
            update_pos_data_2(data);
			break;
		case 4:
			// Option Select Data 3
            update_pos_data_3(data);
			break;
		case 5:
			// Option Select Data 4
            update_pos_data_4(data);
			break;
		case 6:
			// Subaddress Extension Low
            m_option_select[MCABus::POS::SUBADDRESS_EXT_LO] = data;
			break;
		case 7:
			// Subaddress Extension High
            m_option_select[MCABus::POS::SUBADDRESS_EXT_HI] = data;
			break;
		default:
			break;
	}
}



uint16_t mca16_ibm_memory_exp_device::io_31a0_r(offs_t offset)
{
    uint16_t *ram = (uint16_t *)m_expansion_ram->pointer();
    uint16_t data = 0xffff;

    switch(offset)
    {
        case 0:     // address word 0?
        {
            data = m_address_register & 0x0000FFFF;
            break;
        }
        case 1:     
        {
            if(m_in_mode_e)
            {
                data = ram[m_address_register];
            }
            else
            {
                data = (m_address_register & 0xFFFF0000) >> 16; break;              // address word 1?
            }
            break;
        }
        case 2:     // data word?   
        {
            if(m_address_register < 0x800000) data = ram[m_address_register];
            m_address_register++;
            m_consecutive_reads++;
            if(m_consecutive_reads == 3 && m_command == 0xE)
            {
                m_address_register = 0;
                m_command = 0;
                m_in_mode_e = true;
            }
            break;
        }
        default:    data = 0xffff; break;
    }

    LOG("%s: O:%02X D:%04X %08X\n", FUNCNAME, offset, data, m_address_register);

    return data;
}

void mca16_ibm_memory_exp_device::io_31a0_w(offs_t offset, uint16_t data)
{
    uint16_t *ram = (uint16_t *)m_expansion_ram->pointer();

    LOG("%s: O:%02X D:%04X %08X\n", FUNCNAME, offset, data, m_address_register);

    switch(offset)
    {
        case 0: 
        {
            m_command = data;
            
            m_address_register &= 0xFFFF0000; 
            m_address_register |= data;
            break;
        }
        case 1: m_address_register &= 0x0000FFFF; m_address_register |= (((uint32_t)data) << 16); break;
        case 2: if(m_address_register < 0x800000) ram[m_address_register] = data; m_address_register++; break;
    }

    return;
}

ROM_START( ibm_f7f7 )
	ROM_REGION( 0xc0000, "option", 0 )
	ROM_LOAD( "57f2905.bin", 0x0000, 0x2000, CRC(e805b055) SHA1(3434fe34d982c54a68d225d4f50fa0aec96470d1) )
ROM_END

const tiny_rom_entry *mca16_ibm_memory_exp_device::device_rom_region() const
{
	return ROM_NAME( ibm_f7f7 );
}
