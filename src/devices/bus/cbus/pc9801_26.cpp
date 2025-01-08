// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    NEC PC-9801-26 sound card

    Legacy sound card for PC-98xx family, composed by a single YM2203

    TODO:
    - verify sound irq;
    - understand if dips can be read by SW;
    - configurable irq level needs a binding flush in C-bus handling;

**************************************************************************************************/

#include "emu.h"
#include "bus/cbus/pc9801_26.h"

#include "sound/ymopn.h"
#include "speaker.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(PC9801_26, pc9801_26_device, "pc9801_26", "NEC PC-9801-26")

void pc9801_26_device::sound_irq(int state)
{
	m_bus->int_w<5>(state);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void pc9801_26_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	YM2203(config, m_opn, 15.9744_MHz_XTAL / 4); // divider not verified
	m_opn->irq_handler().set(FUNC(pc9801_26_device::sound_irq));
	m_opn->port_a_read_callback().set(FUNC(pc9801_26_device::opn_porta_r));
	//m_opn->port_b_read_callback().set(FUNC(pc8801_state::opn_portb_r));
	//m_opn->port_a_write_callback().set(FUNC(pc8801_state::opn_porta_w));
	m_opn->port_b_write_callback().set(FUNC(pc9801_26_device::opn_portb_w));
	// TODO: verify mixing on HW
	// emerald stage 1 BGM uses ch. 3 for bassline, which sounds way more prominent
	// than the others combined (0.25 1/4 ratio even?)
	m_opn->add_route(0, "mono", 0.50);
	m_opn->add_route(1, "mono", 0.50);
	m_opn->add_route(2, "mono", 0.50);
	m_opn->add_route(3, "mono", 1.00);
}

// to load a different bios for slots:
// -cbus0 pc9801_26,bios=N
ROM_START( pc9801_26 )
	ROM_REGION( 0x4000, "sound_bios", ROMREGION_ERASEFF )
	// PC9801_26k is a minor change that applies to 286+ CPUs
	ROM_SYSTEM_BIOS( 0,  "26k",     "nec26k" )
	ROMX_LOAD( "26k_wyka01_00.bin", 0x0000, 0x2000, CRC(f071bf69) SHA1(f3cdef94e9fee116cf4a9b54881e77c6cd903815), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "26k_wyka02_00.bin", 0x0001, 0x2000, CRC(eaa01052) SHA1(5d47edae49aad591f139d5599fe04b61aefd5ecd), ROM_SKIP(1) | ROM_BIOS(0) )
	// regular BIOS, for V30 and downward CPUs
	ROM_SYSTEM_BIOS( 1,  "26",      "nec26" )
	ROMX_LOAD( "sound.rom",       0x0000, 0x4000, CRC(80eabfde) SHA1(e09c54152c8093e1724842c711aed6417169db23), ROM_BIOS(1) )
	// following rom is unchecked and of dubious quality
	// we also currently mark it based off where they originally belonged to, lacking a better info
	ROM_SYSTEM_BIOS( 2,  "26_9821", "nec26_9821" )
	ROMX_LOAD( "sound_9821.rom",  0x0000, 0x4000, BAD_DUMP CRC(a21ef796) SHA1(34137c287c39c44300b04ee97c1e6459bb826b60), ROM_BIOS(2) )
ROM_END

const tiny_rom_entry *pc9801_26_device::device_rom_region() const
{
	return ROM_NAME( pc9801_26 );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( pc9801_26 )
	PORT_INCLUDE( pc9801_joy_port )

	// On-board jumpers
	// TODO: any way to actually read these from HW?
	PORT_START("OPN_JP6A1_JP6A3")
	PORT_CONFNAME( 0x03, 0x02, "PC-9801-26: Interrupt level")
	PORT_CONFSETTING(    0x00, "IRQ 0" ) // 2-3, 2-3
	PORT_CONFSETTING(    0x01, "IRQ 4" ) // 2-3, 1-2
	PORT_CONFSETTING(    0x02, "IRQ 5" ) // 1-2, 1-2
	PORT_CONFSETTING(    0x03, "IRQ 6" ) // 1-2, 2-3

	PORT_START("OPN_JP6A2")
	PORT_CONFNAME( 0x07, 0x01, "PC-9801-26: Sound ROM address")
	PORT_CONFSETTING(    0x00, "0xc8000" )    // 1-10
	PORT_CONFSETTING(    0x01, "0xcc000" )    // 2-9
	PORT_CONFSETTING(    0x02, "0xd0000" )    // 3-8
	PORT_CONFSETTING(    0x03, "0xd4000" )    // 4-7
	PORT_CONFSETTING(    0x04, "Disable ROM") // 5-6

	PORT_START("OPN_JP6A4")
	PORT_CONFNAME( 0x01, 0x01, "PC-9801-26: Port Base" )
	PORT_CONFSETTING(    0x00, "0x088" ) // 1-4
	PORT_CONFSETTING(    0x01, "0x188" ) // 2-3
INPUT_PORTS_END

ioport_constructor pc9801_26_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_26 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc9801_26_device - constructor
//-------------------------------------------------

pc9801_26_device::pc9801_26_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_snd_device(mconfig, PC9801_26, tag, owner, clock)
	, m_bus(*this, DEVICE_SELF_OWNER)
	, m_opn(*this, "opn")
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void pc9801_26_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

u16 pc9801_26_device::read_io_base()
{
	return ((ioport("OPN_JP6A4")->read() & 1) << 8) + 0x0088;
}

void pc9801_26_device::device_start()
{
	m_rom_base = 0;
	m_io_base = 0;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc9801_26_device::device_reset()
{
	// install the ROM to the physical program space
	u8 rom_setting = ioport("OPN_JP6A2")->read() & 7;
	static const u32 rom_addresses[8] = { 0xc8000, 0xcc000, 0xd0000, 0xd4000, 0, 0, 0, 0 };
	u32 current_rom = rom_addresses[rom_setting & 7];
	memory_region *rom_region = memregion(this->subtag("sound_bios").c_str());
	const u32 rom_size = rom_region->bytes() - 1;

	if (m_rom_base == 0)
		m_rom_base = current_rom;

	if (m_rom_base != 0)
	{
		logerror("%s: uninstall ROM at %08x-%08x\n", machine().describe_context(), m_rom_base, m_rom_base + rom_size);
		m_bus->program_space().unmap_readwrite(m_rom_base, m_rom_base + rom_size);
	}
	if (current_rom != 0)
	{
		logerror("%s: install ROM at %08x-%08x\n", machine().describe_context(), current_rom, current_rom + rom_size);
		m_bus->program_space().unmap_readwrite(current_rom, current_rom + rom_size);
		m_bus->program_space().install_rom(
			current_rom,
			current_rom + rom_size,
			rom_region->base()
		);
	}
	m_rom_base = current_rom;

	// install I/O ports
	u16 current_io = read_io_base();
	m_bus->flush_install_io(
		this->tag(),
		m_io_base,
		current_io,
		3,
		read8sm_delegate(*this, FUNC(pc9801_26_device::opn_r)),
		write8sm_delegate(*this, FUNC(pc9801_26_device::opn_w))
	);
	m_io_base = current_io;

	// install IRQ line
//  static const u8 irq_levels[4] = {0, 4, 5, 6};
//  m_irq_level = irq_levels[ioport("OPN_JP6A1_JP6A3")->read() & 3];
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

// TODO: leftover mirrors? Doesn't match to what installs above
uint8_t pc9801_26_device::opn_r(offs_t offset)
{
	if((offset & 1) == 0)
	{
		return offset & 4 ? 0xff : m_opn->read(offset >> 1);
	}
	else // odd
	{
		logerror("Read to undefined port [%02x]\n", offset+0x188);
		return 0xff;
	}
}


void pc9801_26_device::opn_w(offs_t offset, uint8_t data)
{
	if((offset & 5) == 0)
		m_opn->write(offset >> 1, data);
	else // odd
		logerror("PC9801-26: Write to undefined port [%02x] %02x\n", offset+0x188, data);
}
