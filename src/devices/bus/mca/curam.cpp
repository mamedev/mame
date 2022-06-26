// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    Cumulus CuRAM-16 Plus Memory Multifunction Adapter
    MCA ID @6025

***************************************************************************/

#include "emu.h"
#include "curam.h"

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

DEFINE_DEVICE_TYPE(MCA16_CURAM16_PLUS, mca16_curam16_plus_device, "mca_curam16_plus", "Cumulus CuRAM-16 Plus Memory Multifunction Adapter (@6025)")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_curam16_plus_device::device_add_mconfig(machine_config &config)
{
	RAM(config, RAM_TAG).set_default_size("12M");
}

//-------------------------------------------------
//  mca16_curam16_plus_device - constructor
//-------------------------------------------------

mca16_curam16_plus_device::mca16_curam16_plus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_curam16_plus_device(mconfig, MCA16_CURAM16_PLUS, tag, owner, clock)
{
}

mca16_curam16_plus_device::mca16_curam16_plus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, 0x6025),
	m_ram(*this, RAM_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_curam16_plus_device::device_start()
{
    m_is_mapped = 0;
    m_xms_start = 0;
    m_xms_end = 0;
	set_mca_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_curam16_plus_device::device_reset()
{
    remap();
}

uint8_t mca16_curam16_plus_device::ems_200_r(offs_t offset)
{
    assert_card_feedback();
    LOG("%s: O:%02X\n", FUNCNAME, offset);
    uint8_t data = m_ems_regs_200h[offset];
    return data;
}

uint8_t mca16_curam16_plus_device::ems_4200_r(offs_t offset)
{
    assert_card_feedback();
    LOG("%s: O:%02X\n", FUNCNAME, offset);
    uint8_t data = m_ems_regs_4200h[offset];
    return data;
}

uint8_t mca16_curam16_plus_device::ems_8200_r(offs_t offset)
{
    assert_card_feedback();
    LOG("%s: O:%02X\n", FUNCNAME, offset);
    uint8_t data = m_ems_regs_8200h[offset];
    return data;
}

uint8_t mca16_curam16_plus_device::ems_c200_r(offs_t offset)
{
    assert_card_feedback();
    LOG("%s: O:%02X\n", FUNCNAME, offset);
    uint8_t data = m_ems_regs_c200h[offset];
    return data;
}

void mca16_curam16_plus_device::ems_200_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    m_ems_regs_200h[offset] = data;
    LOG("%s: O:%02X D:%02X\n", FUNCNAME, offset, data);
}

void mca16_curam16_plus_device::ems_4200_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    m_ems_regs_4200h[offset] = data;    
    LOG("%s: O:%02X D:%02X\n", FUNCNAME, offset, data);
}

void mca16_curam16_plus_device::ems_8200_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    m_ems_regs_8200h[offset] = data;
    LOG("%s: O:%02X D:%02X\n", FUNCNAME, offset, data);
}

void mca16_curam16_plus_device::ems_c200_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    m_ems_regs_c200h[offset] = data;
    LOG("%s: O:%02X D:%02X\n", FUNCNAME, offset, data);
}

uint8_t mca16_curam16_plus_device::io8_r(offs_t offset)
{
    assert_card_feedback();
    LOG("%s: O:%02X\n", FUNCNAME, offset);
    return 0x00;
}

void mca16_curam16_plus_device::io8_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    LOG("%s: O:%02X D:%02X\n", FUNCNAME, offset, data);
}

uint8_t mca16_curam16_plus_device::pos_r(offs_t offset)
{
	LOG("%s\n", FUNCNAME);

	switch(offset)
	{
		case 0:
			// Adapter Identification b0-b7
			return get_card_id() & 0xFF;
		case 1:
			// Adapter Identification b8-b15
			return (get_card_id() & 0xFF00) >> 8;
		case 2:
			// Option Select Data 1
            return m_option_select[MCABus::POS::OPTION_SELECT_DATA_1];
			break;
		case 3:
			// Option Select Data 2
            return m_option_select[MCABus::POS::OPTION_SELECT_DATA_2];
			break;
		case 4:
			// Option Select Data 3
            return m_option_select[MCABus::POS::OPTION_SELECT_DATA_3];
			break;
		case 5:
			// Option Select Data 4
            return m_option_select[MCABus::POS::OPTION_SELECT_DATA_4];
			break;
		case 6:
			// Subaddress Extension Low
			return 0xff;
			break;
		case 7:
			// Subaddress Extension High
			return 0xff;
			break;
		default:
			break;
	}

	return 0xFF;
}

void mca16_curam16_plus_device::pos_w(offs_t offset, uint8_t data)
{
	LOG("%s\n", FUNCNAME);

	switch(offset)
	{
		case 0:
			// Adapter Identification b0-b7
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
			break;
		case 7:
			// Subaddress Extension High
			break;
		default:
			break;
	}
}

void mca16_curam16_plus_device::unmap()
{
    if(m_xms_start != 0) m_mca->unmap_readwrite(m_xms_start+0x60000, m_xms_end+0x60000);

	m_is_mapped = false;
}

offs_t mca16_curam16_plus_device::calculate_xms_start_address()
{
    // XMS disabled?
    if((m_option_select[MCABus::POS::OPTION_SELECT_DATA_4] & 0b11100000) == 0b11100000)
    {
        return 0; // Disabled.
    }

    // XMS enabled, so where's it start?
    uint8_t pos4 = m_option_select[MCABus::POS::OPTION_SELECT_DATA_4] & 0b11111100;
    uint8_t pos2 = m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] & 0b01111000;

    // TODO: Table
    if(pos4 == 0b11011100 && pos2 == 0b00000000) return 0x100000;
    else if(pos4 == 0b11011100 && pos2 == 0b01000000) return 0x200000;
    else if(pos4 == 0b11011000 && pos2 == 0b00001000) return 0x300000;
    else if(pos4 == 0b11011000 && pos2 == 0b01001000) return 0x400000;
    else if(pos4 == 0b11010100 && pos2 == 0b00010000) return 0x500000;
    else if(pos4 == 0b11010100 && pos2 == 0b01010000) return 0x600000;
    else if(pos4 == 0b11010000 && pos2 == 0b00011000) return 0x700000;
    else if(pos4 == 0b11010000 && pos2 == 0b01011000) return 0x800000;
    else if(pos4 == 0b11001100 && pos2 == 0b00100000) return 0x900000;
    else if(pos4 == 0b11001100 && pos2 == 0b01100000) return 0xa00000;
    else if(pos4 == 0b11001000 && pos2 == 0b00101000) return 0xb00000;
    else if(pos4 == 0b11001000 && pos2 == 0b01101000) return 0xc00000;
    else if(pos4 == 0b11000100 && pos2 == 0b00110000) return 0xd00000;
    else if(pos4 == 0b11000100 && pos2 == 0b01110000) return 0xe00000;

    return 0; // ?
}

offs_t mca16_curam16_plus_device::calculate_xms_end_address()
{
    switch(m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] & 0b01000111)
    {
        case 0b00000001: return 0x300000-1;
        case 0b01000001: return 0x400000-1;
        case 0b00000010: return 0x500000-1;
        case 0b01000010: return 0x600000-1;
        case 0b00000011: return 0x700000-1;
        case 0b01000011: return 0x800000-1;
        case 0b00000100: return 0x900000-1;
        case 0b01000100: return 0xa00000-1;
        case 0b00000101: return 0xb00000-1;
        case 0b01000101: return 0xc00000-1;
        case 0b00000110: return 0xd00000-1;
        case 0b01000110: return 0xe00000-1;
        case 0b00000111: return 0xf00000-1;
        case 0b01000000: return 0xf80000-1;
    }

    return 0;
}

offs_t mca16_curam16_plus_device::calculate_ems_ports()
{
    return 0x200 + (((m_option_select[MCABus::POS::OPTION_SELECT_DATA_3] & 0b01111110) >> 1) * 0x10);
}

void mca16_curam16_plus_device::remap()
{
    if(m_is_mapped) unmap();

    m_xms_start = calculate_xms_start_address();
    m_xms_end = calculate_xms_end_address();

    LOG("%s: XMS mapping %06X-%06X\n", FUNCNAME, m_xms_start, m_xms_end);

    // This is a 16-bit card that only works with 286 models that map the high 384K to 100000h.
    if(m_xms_start != 0 && m_xms_end != 0)
    {
        // This is not the right behavior but I don't know what is.
        m_mca->install_memory(m_xms_start+0x60000, m_xms_end+0x60000,
            read8sm_delegate(*m_ram, FUNC(ram_device::read)), write8sm_delegate(*m_ram, FUNC(ram_device::write)));
        m_is_mapped = true;
    }

    offs_t ems_base = calculate_ems_ports();
    LOG("%s: installing EMS at %04X\n", FUNCNAME, ems_base);

    m_mca->install_device(ems_base, ems_base+0xf, 
        read8sm_delegate(*this, FUNC(mca16_curam16_plus_device::ems_200_r)),
        write8sm_delegate(*this, FUNC(mca16_curam16_plus_device::ems_200_w)));
    m_mca->install_device(ems_base+0x4000, ems_base+0x400f, 
        read8sm_delegate(*this, FUNC(mca16_curam16_plus_device::ems_4200_r)),
        write8sm_delegate(*this, FUNC(mca16_curam16_plus_device::ems_4200_w)));
    m_mca->install_device(ems_base+0x8000, ems_base+0x800f, 
        read8sm_delegate(*this, FUNC(mca16_curam16_plus_device::ems_8200_r)),
        write8sm_delegate(*this, FUNC(mca16_curam16_plus_device::ems_8200_w)));
    m_mca->install_device(ems_base+0xc000, ems_base+0xc00f, 
        read8sm_delegate(*this, FUNC(mca16_curam16_plus_device::ems_c200_r)),
        write8sm_delegate(*this, FUNC(mca16_curam16_plus_device::ems_c200_w)));
}

bool mca16_curam16_plus_device::map_has_changed()
{
    // EMS mapping
    // TODO: Set up EMS.
    
    // XMS mapping
    offs_t calculated_xms_start = calculate_xms_start_address();
    offs_t calculated_xms_end = calculate_xms_end_address();

    return (calculated_xms_start != m_xms_start || calculated_xms_end != m_xms_end);
}

void mca16_curam16_plus_device::update_pos_data_1(uint8_t data)
{
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] = data;
	//if(map_has_changed()) remap();
}

void mca16_curam16_plus_device::update_pos_data_2(uint8_t data)
{
    // POS 0 and POS 2 together control the mapping.
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] = data;
	//if(map_has_changed()) remap();
}

void mca16_curam16_plus_device::update_pos_data_3(uint8_t data)
{
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_3] = data;
    //if(map_has_changed()) remap();
}

void mca16_curam16_plus_device::update_pos_data_4(uint8_t data)
{
	m_option_select[MCABus::POS::OPTION_SELECT_DATA_4] = data;
    if(map_has_changed()) remap();
}

uint8_t mca16_curam16_plus_device::ems_r(offs_t offset)
{
    // figure out paging...

    /*
        I think the mapping is something like:
        register offset 01: select first 64K page
        register offset 03: select second 64K page
        register offset 05: select third 64K page
        register offset 07: select fourth 64K page
    */ 

    uint8_t page = m_ems_regs_200h[1];
    return m_ram->read(0x400000 + (page * 65536) + offset);
}

void mca16_curam16_plus_device::ems_w(offs_t offset, uint8_t data)
{
    uint8_t page = m_ems_regs_200h[1];
    return m_ram->write(0x400000 + (page * 65536) + offset, data);
}