// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ICT Mini Chief MC-20 Floppy/Hard Disk Drive

	Commodore 1571 floppy disk drive with an internal WD1002A-WX1
	hard disk controller and a Seagate ST225 hard disk partitioned into
	120 volumes of 170KB each to match the capacity of a 1541 floppy.

**********************************************************************/

/*

	Select floppy
	-------------
	OPEN15,8,15,"H0":CLOSE15

	Select hard disk partition
	--------------------------
	OPEN15,8,15,"H1":CLOSE15
	...
	OPEN15,8,15,"H120":CLOSE15

	Other commands
	---------------
	HCH 	full disk copy from floppy to hard disk
	HCHB 	BAM copy from floppy to hard disk
	HCF 	copy full disk from hard drive to floppy
	HI 		update the autosave table in chain mode
	HP 		park drive head for transport
	HM0		disable mode function (disables chain)
	HM1		chain mode (turns chain on)
	HM2 	source/destination mode (used for copy programs)
	HM3	    redirect anchor (used to boot protected software)
	HM4	    initialize chain (used to create a chain)
	HM5     CP/M mode (H2=A:, H3=B:, H4=C:, H5=D:)

*/

#include "emu.h"
#include "minichief.h"

#define M6502_TAG "u1"

DEFINE_DEVICE_TYPE(MINI_CHIEF, mini_chief_device, "minichif", "ICT Mini Chief Disk Drive")

mini_chief_device::mini_chief_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c1571_device(mconfig, MINI_CHIEF, tag, owner, clock),
	m_isa(*this, "isa"),
	m_boss(*this, "BOSS")
{
}

void mini_chief_device::device_start()
{
	c1571_device::device_start();

	save_item(NAME(m_isa_offs));
	save_item(NAME(m_isa_data));
}

ROM_START( minichief )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "ictdos710.u2", 0x0000, 0x8000, CRC(aaacf7e9) SHA1(c1296995238ef23f18e7fec70a144a0566a25a27) )
ROM_END

const tiny_rom_entry *mini_chief_device::device_rom_region() const
{
	return ROM_NAME( minichief );
}

void mini_chief_device::cia_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0       ISA A0
	    1       ISA A1
	    2       ISA A2
	    3       ISA /SMEMR
	    4       ISA /SMEMW
	    5       ISA RESET
	    6
	    7

	*/

	m_isa_offs = data & 0x07;

	if (BIT(data, 4) && !BIT(data, 3))
	{
		m_isa->io_w(0x320 | m_isa_offs, m_isa_data);
	}
	else if (BIT(data, 3) && !BIT(data, 4))
	{
		m_isa_data = m_isa->io_r(0x320 | m_isa_offs);
	}

	if (BIT(data, 5))
	{
		m_isa->reset();
	}
}

void mini_chief_device::mini_chief_mem(address_map &map)
{
	c1571_mem(map);
	
	map(0x4000, 0x400f).mirror(0xff0).rw(m_cia, FUNC(mos6526_device::read), FUNC(mos6526_device::write));
	map(0x5000, 0x5fff).mirror(0x2000).ram();
	map(0x6000, 0x6fff).ram();
	map(0x8000, 0xffff).rom().region(M6502_TAG, 0);
}

static void mini_chief_isa8_cards(device_slot_interface &device)
{
	device.option_add("wd1002a_wx1", ISA8_WD1002A_WX1);
}

void mini_chief_device::device_add_mconfig(machine_config &config)
{
	add_base_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mini_chief_device::mini_chief_mem);
	m_via0->readpa_handler().set(FUNC(mini_chief_device::via0_pa_r));

	add_cia_mconfig(config);
	m_cia->pa_rd_callback().set(FUNC(mini_chief_device::cia_pa_r));
	m_cia->pa_wr_callback().set(FUNC(mini_chief_device::cia_pa_w));
	m_cia->pb_wr_callback().set(FUNC(mini_chief_device::cia_pb_w));

	ISA8(config, m_isa);
	m_isa->set_custom_spaces();
	ISA8_SLOT(config, "isa1", 0, m_isa, mini_chief_isa8_cards, "wd1002a_wx1", false);
}

static INPUT_PORTS_START( mini_chief )
	PORT_START("ADDRESS")
	PORT_DIPNAME( 0x03, 0x00, "Device Address" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "11" )

	PORT_START("BOSS")
	PORT_CONFNAME( 0x01, 0x00, "BOS Toggle Switch" )
	PORT_DIPSETTING(    0x00, "Floppy" )
	PORT_DIPSETTING(    0x01, "Hard drive" )
INPUT_PORTS_END

ioport_constructor mini_chief_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mini_chief );
}
