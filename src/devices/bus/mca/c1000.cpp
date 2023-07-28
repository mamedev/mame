// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

	Custom Computer Systems C1000
    MCA ID @6213
    MCA IDE adapter

    POS configuration:
    - POS 0 bit 7: I/O port
    -- 0 = 11F0h-11F7h (primary)
    -- 1 = 1170h-117Fh (secondary)

    - POS 0 bits 3-6: option ROM base
    -- X1111XXX = DC000h-DFFFFh
    -- X1101XXX = D8000h-DBFFFh
    -- X1011XXX = D4000h-D7FFFh
    -- X1001XXX = D0000h-D3FFFh
    -- X0111XXX = CC000h-CFFFFh
    -- X0011XXX = C8000h-CBFFFh

    - POS 1 bits 0-6: IRQ line
    -- XX001001 = IRQ 10 
    -- XX001011 = IRQ 15

    - POS 2 bits 6-7: CO-RESIDENT INSTALLATION
    -- 00XXXXXX = no
    -- 01XXXXXX = yes

    - POS 2 bits 0-3: Drive 0 Type Selection
    -- XXXX0001 = "Identify and Translate"
    -- XXXX0010 = "Identify and Truncate"
    -- XXXX0000 = "Not Installed"

    - POS 2 bits 7-4: Drive 0 Type Selection
    -- 0001XXXX = "Identify and Translate"
    -- 0010XXXX = "Identify and Truncate"
    -- 0000XXXX = "Not Installed"

***************************************************************************/

#include "emu.h"
#include "c1000.h"

#include "bus/ata/atadev.h"
#include "imagedev/harddriv.h"

//#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA16_C1000_IDE, mca16_c1000_ide_device, "mca_c1000_ide", "CCS C1000 IDE Adapter (@6213)")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_c1000_ide_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
	m_ata->irq_handler().set(FUNC(mca16_c1000_ide_device::ide_interrupt));
}

//-------------------------------------------------
//  isa8_com_device - constructor
//-------------------------------------------------

mca16_c1000_ide_device::mca16_c1000_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_c1000_ide_device(mconfig, MCA16_C1000_IDE, tag, owner, clock)
{
}

mca16_c1000_ide_device::mca16_c1000_ide_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, 0x6213),
    m_is_primary(true),
	m_ata(*this, "ata")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_c1000_ide_device::device_start()
{
	set_mca_device();
    m_is_mapped = false;
    m_is_latched = false;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_c1000_ide_device::device_reset()
{
    if(m_is_mapped) unmap();

    reset_option_select();
    m_is_mapped = false;
}

void mca16_c1000_ide_device::map(address_map &map)
{
	map(0x0, 0x7).rw("ide", FUNC(ide_controller_device::cs0_r), FUNC(ide_controller_device::cs0_w));
}

void mca16_c1000_ide_device::unmap()
{
    LOG("%s: Unmapping MCA card\n", FUNCNAME);
    m_mca->unmap_device(m_cur_io_start, m_cur_io_end);
    m_mca->unmap_device(m_cur_io_start+0xc, m_cur_io_start+0xf);

    offs_t cur_rom_base = rom_base();
    if(cur_rom_base != 0) m_mca->unmap_rom(cur_rom_base, cur_rom_base+0x3fff);
    m_is_mapped = false;
}

void mca16_c1000_ide_device::pos_w(offs_t offset, uint8_t data)
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
            // not used
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

void mca16_c1000_ide_device::update_pos_data_1(uint8_t data)
{
    // POS 0 controls the I/O mapping and option ROM base address.

    LOG("%s: C1000 POS data 1 write %02X\n", FUNCNAME, data);
    
    // Unmap the I/O ports and option ROM.
    if(m_is_mapped) unmap();

    // Remap the I/O ports.
    if(BIT(data, 7)) 
    {
        // Secondary
        m_cur_io_start = 0x1170;
        m_cur_io_end = 0x1177;
        m_is_primary = false;
    }
    else
    {
        // Primary
        m_cur_io_start = 0x11f0;
        m_cur_io_end = 0x11f7;
        m_is_primary = true;
    }

    LOG("%s: Mapping MCA card to %04Xh-%04Xh\n", FUNCNAME, m_cur_io_start, m_cur_io_end);
    m_mca->install_device(m_cur_io_start, m_cur_io_end, 
            read8sm_delegate(*this, FUNC(mca16_c1000_ide_device::io8_r)),
            write8sm_delegate(*this, FUNC(mca16_c1000_ide_device::io8_w)));
    m_mca->install_device(m_cur_io_start+0xc, m_cur_io_start+0xf,
            read8sm_delegate(*this, FUNC(mca16_c1000_ide_device::alt_r)),
            write8sm_delegate(*this, FUNC(mca16_c1000_ide_device::alt_w)));       

    // So rom_base() picks up the new address.
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] = data;

    // Apply the updated ROM base address.
    offs_t cur_rom_base = rom_base();
    if(cur_rom_base != 0)
    {
        m_mca->install_rom(this, cur_rom_base, cur_rom_base+0x3fff, "option");
    }

    m_is_mapped = true;
}

uint8_t mca16_c1000_ide_device::io8_r(offs_t offset)
{
    assert_card_feedback();
    uint8_t data;

    // 0 does a 16-bit read. 1 is latched to the other data byte.
    switch(offset)
    {
        case 0:
            m_ata_data_latch = m_ata->cs0_r(offset, 0xffff);
            data = m_ata_data_latch & 0xff;
            m_is_latched = true;
            break;
        case 1:
            if(m_is_latched)
            {
                data = m_ata_data_latch >> 8;
                m_is_latched = false;
            }
            else
            {
                data = m_ata->cs0_r(1, 0xff);
            }
            break;
        default: 
            data = m_ata->cs0_r(offset, 0xff);
            break;
    }

    LOG("IDE CS0 R %02X %02X\n", offset, data);
    return data;
}

void mca16_c1000_ide_device::io8_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();

    switch(offset)
    {
        case 0:
            // The BIOS writes 0 then 1, so latch 0 then write 1?
            m_ata_data_latch = data;
            m_is_latched = true;
            break;
        case 1:
            if(m_is_latched)
            {
                m_ata_data_latch |= ((uint16_t)data) << 8;
                m_ata->cs0_w(0, m_ata_data_latch, 0xffff);
                m_is_latched = false;
                break;
            }
            else
            {
                m_ata->cs0_w(1, data, 0xff);
                break;
            }
        default: 
            m_ata->cs0_w(offset, data, 0xff);
            break;
    }

    LOG("IDE CS0 W %02X %02X\n", offset, data);
}

uint8_t mca16_c1000_ide_device::alt_r(offs_t offset)
{
    assert_card_feedback();
    uint8_t data;

    switch(offset)
    {
        case 0: data = m_ata->cs1_r(7, 0xff); break;
        case 2: data = m_ata->cs1_r(6, 0xff); break;
        default: data = 0xff;
    }

    LOG("IDE ALT R %02X %02X\n", offset, data);

    return data;
}

void mca16_c1000_ide_device::alt_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();

    LOG("IDE ALT W %02X %02X\n", offset, data);
    switch(offset)
    {
        case 0: m_ata->cs1_w(7, data, 0xff); return;
        case 2: m_ata->cs1_w(6, data, 0xff); return;
        default: return;
    }
}

void mca16_c1000_ide_device::update_pos_data_2(uint8_t data)
{
    // IRQ is handled by ide_interrupt callback, just set the bits correctly.
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] = data;
}

void mca16_c1000_ide_device::update_pos_data_3(uint8_t data)
{
    // Adapter-specific stuff. Not sure what this does in hardware if anything.
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_3] = data;
}

void mca16_c1000_ide_device::ide_interrupt(int state)
{
    switch(m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] & 0x3f)
    {
        case 0b001001: m_mca->ireq_w<10>(state); break;
        case 0b001011: m_mca->ireq_w<15>(state); break;
        default:
            LOG("%s: IDE interrupt with invalid POS 1 selection %02X\n", FUNCNAME, m_option_select[MCABus::POS::OPTION_SELECT_DATA_2]);
            break;
    }
}

offs_t mca16_c1000_ide_device::rom_base()
{
    offs_t base = 0;
    switch(pos_r(MCABus::POS::OPTION_SELECT_DATA_1) & 0b01111000)
    {
        case 0b01111000: base = 0xdc000; break;
        case 0b01101000: base = 0xd8000; break;
        case 0b01011000: base = 0xd4000; break;
        case 0b01001000: base = 0xd0000; break;
        case 0b00111000: base = 0xcc000; break;
        case 0b00011000: base = 0xc8000; break;
        default: base = 0;
    }

    LOG("%s: ROM base is %06Xh\n", FUNCNAME, base);

    return base;
}

ROM_START( c1000 )
	ROM_REGION( 0xc8000, "option", 0 )
	ROM_LOAD( "c1000.bin", 0x0000, 0x4000, CRC(e805b055) SHA1(3434fe34d982c54a68d225d4f50fa0aec96470d1) )
ROM_END

const tiny_rom_entry *mca16_c1000_ide_device::device_rom_region() const
{
	return ROM_NAME( c1000 );
}

