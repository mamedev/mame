/***************************************************************************

  ISA 8 bit IDE controller

As implemented by the Vintage computer forums XT-IDE controller.

Card has jumpers for I/O base address, and ROM base address, for the time
being we'll emulate an I/O base of 0x300 and a ROM base of 0xC8000.

If the I/O address is changed then the ROM will need to be patched.
The opensource bios is available from :
http://code.google.com/p/xtideuniversalbios/

The data high register is connected to a pair of latches that have the MSB of
the 16 bit data word latched into so that 16 bit IO may be performmed, this
involves the following :

A Data read will read the data register first, and obtain the bottom 8 bits
of the data word, the top 8 bits will be latched at the same time these are
then read from the latch to the processor.

A data write will first write the top 8 bits to the latch, and then the bottom
8 bits to the normal data register, this will also transfer the top 8 bits to
the drive.

IDE Register                XTIDE rev 1     rev 2 or modded rev 1
Data (XTIDE Data Low)       0               0
Error (in), Features (out)  1               8
Sector Count                2               2
Sector Number,
LBA bits 0...7,
LBA48 bits 24...31          3               10
Low Cylinder,
LBA bits 8...15,
LBA48 bits 32...39          4               4
High Cylinder,
LBA bits 16...23,
LBA48 bits 40...47          5               12
Drive and Head Select,
LBA28 bits 24...27          6               6
Status (in), Command (out)  7               14
XTIDE Data High             8               1
Alternative Status (in),
Device Control (out)        14              7

***************************************************************************/



#include "emu.h"
#include "machine/isa_ide8.h"
#include "machine/idectrl.h"
#include "imagedev/harddriv.h"


static READ8_DEVICE_HANDLER( ide8_r )
{
	ide_controller_device   *ide = (ide_controller_device *) device;
	isa8_ide_device         *ide8_d = downcast<isa8_ide_device *>(device->owner());
	UINT8   result  = 0;

	if(offset == 0)
	{
		// Data register transfer low byte and latch high
		result=ide->ide_controller_read(0, (offset & 0x07), 1);
		ide8_d->set_latch_in(ide->ide_controller_read(0, (offset & 0x07), 1));
	}
	else if((offset > 0) && (offset < 8))
		result=ide->ide_controller_read(0, (offset & 0x07), 1);
	else if(offset == 8)
		result=ide8_d->get_latch_in();
	else if(offset == 14)
		result=ide->ide_controller_read(1, (offset & 0x07), 1);

//  logerror("%s ide8_r: offset=%d, result=%2X\n",device->machine().describe_context(),offset,result);

	return result;
}

static WRITE8_DEVICE_HANDLER( ide8_w )
{
	ide_controller_device   *ide = (ide_controller_device *) device;
	isa8_ide_device         *ide8_d = downcast<isa8_ide_device *>(device->owner());

//  logerror("%s ide8_w: offset=%d, data=%2X\n",device->machine().describe_context(),offset,data);

	if(offset == 0)
	{
		// Data register transfer low byte and latched high
		ide->ide_controller_write(0, (offset & 7), 1, data);
		ide->ide_controller_write(0, (offset & 7), 1, ide8_d->get_latch_out());
	}
	else if((offset > 0) && (offset < 8))
		ide->ide_controller_write(0, (offset & 7), 1, data);
	else if(offset == 8)
		ide8_d->set_latch_out(data);
	else if(offset == 14)
		ide->ide_controller_write(1, (offset & 7), 1, data);
}


WRITE_LINE_MEMBER(isa8_ide_device::ide_interrupt)
{
	switch(irq)
	{
		case 0x02 : m_isa->irq2_w(state); break;
		case 0x03 : m_isa->irq3_w(state); break;
		case 0x04 : m_isa->irq4_w(state); break;
		case 0x05 : m_isa->irq5_w(state); break;
		case 0x07 : m_isa->irq7_w(state); break;
		default : ;
	}
}

static MACHINE_CONFIG_FRAGMENT( ide8_config )
	MCFG_IDE_CONTROLLER_ADD("ide", ide_devices, "hdd", "hdd", false)
	MCFG_IDE_CONTROLLER_IRQ_HANDLER(WRITELINE(isa8_ide_device, ide_interrupt))
MACHINE_CONFIG_END

static INPUT_PORTS_START( ide8_port )
	PORT_START("BIOS_BASE")
	PORT_DIPNAME( 0x0F, 0x02, "XT-IDE ROM base segment")
	PORT_DIPSETTING(    0x00, "C000" )
	PORT_DIPSETTING(    0x01, "C400" )
	PORT_DIPSETTING(    0x02, "C800" )
	PORT_DIPSETTING(    0x03, "CC00" )
	PORT_DIPSETTING(    0x04, "D000" )
	PORT_DIPSETTING(    0x05, "D400" )
	PORT_DIPSETTING(    0x06, "D800" )
	PORT_DIPSETTING(    0x07, "DC00" )
	PORT_DIPSETTING(    0x08, "E000" )
	PORT_DIPSETTING(    0x09, "E400" )
	PORT_DIPSETTING(    0x0A, "E800" )
	PORT_DIPSETTING(    0x0B, "EC00" )
	PORT_DIPSETTING(    0x0C, "F000" )
	PORT_DIPSETTING(    0x0D, "F400" )
	PORT_DIPSETTING(    0x0E, "F800" )
	PORT_DIPSETTING(    0x0F, "FC00" )

	PORT_START("IO_ADDRESS")
	PORT_DIPNAME( 0x0F, 0x08, "XT-IDE I/O address")
	PORT_DIPSETTING(    0x00, "200" )
	PORT_DIPSETTING(    0x01, "220" )
	PORT_DIPSETTING(    0x02, "240" )
	PORT_DIPSETTING(    0x03, "260" )
	PORT_DIPSETTING(    0x04, "280" )
	PORT_DIPSETTING(    0x05, "2A0" )
	PORT_DIPSETTING(    0x06, "2C0" )
	PORT_DIPSETTING(    0x07, "2E0" )
	PORT_DIPSETTING(    0x08, "300" )
	PORT_DIPSETTING(    0x09, "320" )
	PORT_DIPSETTING(    0x0A, "340" )
	PORT_DIPSETTING(    0x0B, "360" )
	PORT_DIPSETTING(    0x0C, "380" )
	PORT_DIPSETTING(    0x0D, "3A0" )
	PORT_DIPSETTING(    0x0E, "3C0" )
	PORT_DIPSETTING(    0x0F, "3E0" )

	PORT_START("IRQ")
	PORT_DIPNAME( 0x07, 0x05, "XT-IDE IRQ")
	PORT_DIPSETTING(    0x02, "IRQ 2" )
	PORT_DIPSETTING(    0x03, "IRQ 3" )
	PORT_DIPSETTING(    0x04, "IRQ 4" )
	PORT_DIPSETTING(    0x05, "IRQ 5" )
	PORT_DIPSETTING(    0x07, "IRQ 7" )
INPUT_PORTS_END

ROM_START( ide8 )
	ROM_REGION(0x04000,"ide8", 0)
	// XT-IDE universal bios from : http://code.google.com/p/xtideuniversalbios/
	ROM_LOAD("ide_xtl.bin",  0x00000, 0x03800, CRC(68801d16) SHA1(f3f5bed385d00ac444d85f492c879aa68a864160))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_IDE = &device_creator<isa8_ide_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_ide_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ide8_config );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor isa8_ide_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ide8_port );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa8_ide_device::device_rom_region() const
{
	return ROM_NAME( ide8 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_ide_device - constructor
//-------------------------------------------------

isa8_ide_device::isa8_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, ISA8_IDE, "XT-IDE Fixed Drive Adapter", tag, owner, clock, "isa8_ide", __FILE__),
		device_isa8_card_interface( mconfig, *this )
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_ide_device::device_start()
{
	set_isa_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_ide_device::device_reset()
{
	int base_address    = ((ioport("BIOS_BASE")->read() & 0x0F) * 16 * 1024) + 0xC0000;
	int io_address      = ((ioport("IO_ADDRESS")->read() & 0x0F) * 0x20) + 0x200;
	irq                 = (ioport("IRQ")->read() & 0x07);

	m_isa->install_rom(this, base_address, base_address + (16*1024) -1 , 0, 0, "ide8", "ide8");
	m_isa->install_device(subdevice("ide"), io_address, io_address+15, 0, 0, FUNC(ide8_r), FUNC(ide8_w) );

	//logerror("isa8_ide_device::device_reset(), bios_base=0x%5X to 0x%5X, I/O=0x%3X, IRQ=%d\n",base_address,base_address + (16*1024)  -1 ,io_address,irq);
}
