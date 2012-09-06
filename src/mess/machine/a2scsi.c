/*********************************************************************

    a2scsi.c

    Implementation of the Apple II SCSI Card

    Schematic at:
    http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/SCSI%20Controllers/Apple%20II%20SCSI%20Card/Schematics/Rev.%20C%20SCSI%20Schematic%20-%20Updated%202-23-6.jpg


    Notes:

    C0n0-C0n7 = NCR5380 registers in normal order
    C0n9 = DIP switches
    C0na = RAM and ROM bank switching
    C0nb = reset 5380
    C0ne = into the PAL stuff

*********************************************************************/

#include "a2scsi.h"
#include "includes/apple2.h"
#include "machine/scsibus.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_SCSI = &device_creator<a2bus_scsi_device>;

#define SCSI_ROM_REGION  "scsi_rom"
#define SCSI_5380_TAG    "scsi:ncr5380"

static const struct NCR5380interface a2scsi_5380_intf =
{
	NULL        // IRQ handler (unconnected according to schematic)
};

MACHINE_CONFIG_FRAGMENT( scsi )
	MCFG_SCSIBUS_ADD("scsi")
	MCFG_NCR5380_ADD(SCSI_5380_TAG, (XTAL_28_63636MHz/4), a2scsi_5380_intf)
MACHINE_CONFIG_END

ROM_START( scsi )
	ROM_REGION(0x4000, SCSI_ROM_REGION, 0)  // this is the Rev. C ROM
    ROM_LOAD( "341-0437-a.bin", 0x0000, 0x4000, CRC(5aff85d3) SHA1(451c85c46b92e6ad2ad930f055ccf0fe3049936d) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_scsi_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( scsi );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_scsi_device::device_rom_region() const
{
	return ROM_NAME( scsi );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_scsi_device::a2bus_scsi_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
    device_t(mconfig, type, name, tag, owner, clock),
    device_a2bus_card_interface(mconfig, *this),
    m_ncr5380(*this, SCSI_5380_TAG)
{
	m_shortname = "a2scsi";
}

a2bus_scsi_device::a2bus_scsi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
    device_t(mconfig, A2BUS_SCSI, "Apple II SCSI Card", tag, owner, clock),
    device_a2bus_card_interface(mconfig, *this),
    m_ncr5380(*this, SCSI_5380_TAG)
{
	m_shortname = "a2scsi";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_scsi_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	astring tempstring;
	m_rom = device().machine().root_device().memregion(this->subtag(tempstring, SCSI_ROM_REGION))->base();

    memset(m_ram, 0, 8192);

	save_item(NAME(m_ram));
	save_item(NAME(m_rambank));
	save_item(NAME(m_rombank));
}

void a2bus_scsi_device::device_reset()
{
    m_rambank = m_rombank = 0;      // CLR on 74LS273 at U3E is connected to RES, so these clear on reset
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_scsi_device::read_c0nx(address_space &space, UINT8 offset)
{
    printf("Read c0n%x (PC=%x)\n", offset, cpu_get_pc(&space.device()));

    switch (offset)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            return m_ncr5380->ncr5380_read_reg(offset);

    case 9:         // card's ID?
        return 7;
    }

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_scsi_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
    printf("Write %02x to c0n%x (PC=%x)\n", data, offset, cpu_get_pc(&space.device()));

    switch (offset)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            m_ncr5380->ncr5380_write_reg(offset, data);
            break;

        case 0xa:  // ROM and RAM banking (74LS273 at U3E)
            /*
                ROM banking:
                (bits EA8-EA13 are all zeroed when /IOSEL is asserted, so CnXX always gets the first page of the ROM)
                EA10 = bit 0
                EA11 = bit 1
                EA12 = bit 2
                EA13 = bit 3 (N/C)

                RAM banking:
                RA10 = bit 4
                RA11 = bit 5
                RA12 = bit 6
            */

            m_rambank = ((data>>4) & 0x7) * 0x400;
            m_rombank = (data & 0xf) * 0x400;
            printf("RAM bank to %x, ROM bank to %x\n", m_rambank, m_rombank);
            break;

        case 0xb:
            printf("Reset NCR5380\n");
            break;
    }
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_scsi_device::read_cnxx(address_space &space, UINT8 offset)
{
    // one slot image at the start of the ROM, it appears
    return m_rom[offset];
}

void a2bus_scsi_device::write_cnxx(address_space &space, UINT8 offset, UINT8 data)
{
    printf("Write %02x to cn%02x (PC=%x)\n", data, offset, cpu_get_pc(&space.device()));
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

UINT8 a2bus_scsi_device::read_c800(address_space &space, UINT16 offset)
{
    // bankswitched RAM at c800-cbff
    // bankswitched ROM at cc00-cfff
    if (offset < 0x400)
    {
        printf("Read RAM at %x = %02x\n", offset+m_rambank, m_ram[offset + m_rambank]);
        return m_ram[offset + m_rambank];
    }
    else
    {
        return m_rom[(offset-0x400) + m_rombank];
    }
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_scsi_device::write_c800(address_space &space, UINT16 offset, UINT8 data)
{
    if (offset < 0x400)
    {
        printf("%02x to RAM at %x\n", data, offset+m_rambank);
        m_ram[offset + m_rambank] = data;
    }
}
