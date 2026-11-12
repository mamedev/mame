// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    REX Data Technology ProLogic-DOS Classic

**********************************************************************/

#include "emu.h"
#include "prologicdos.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "ucd5"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1541_PROLOGIC_DOS_CLASSIC, c1541_prologic_dos_classic_device, "c1541pdc", "REX Data Technology ProLogic-DOS Classic")


//-------------------------------------------------
//  ROM( c1541pdc )
//-------------------------------------------------

ROM_START( c1541pdc )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "325302-01.uab4", 0x0000, 0x2000, CRC(29ae9752) SHA1(8e0547430135ba462525c224e76356bd3d430f11) )
	ROM_LOAD( "901229-06 aa.uab5", 0x2000, 0x2000, CRC(3a235039) SHA1(c7f94f4f51d6de4cdc21ecbb7e57bb209f0530c0) )

	ROM_REGION( 0x8000, "ic4", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("prologic")
	ROM_SYSTEM_BIOS( 0, "prologic", "ProLogic-DOS Classic" )
	ROMX_LOAD( "kernal.ic4", 0x4000, 0x4000, CRC(79032ed5) SHA1(0ca4d5ef41c7e3d18d8945476d1481573af3e27c), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "dolphin", "Dolphin-DOS 2.0" )
	ROMX_LOAD( "dd20.bin", 0x0000, 0x8000, CRC(94c7fe19) SHA1(e4d5b9ad6b719dd988276214aa4536d3525d313c), ROM_BIOS(1) )

	ROM_REGION( 0x2000, "ic5", 0 )
	ROM_LOAD( "mmu.ic5", 0x0000, 0x2000, CRC(4c41392c) SHA1(78846af2ee6a56fceee44f9246659685ab2cbb7e) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1541_prologic_dos_classic_device::device_rom_region() const
{
	return ROM_NAME( c1541pdc );
}


//-------------------------------------------------
//  INPUT_PORTS( c1541pdc )
//-------------------------------------------------

static INPUT_PORTS_START( c1541pdc )
	PORT_START("ADDRESS")
	PORT_DIPNAME( 0x03, 0x00, "Device Address" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "11" )

	PORT_START("JP5")
	PORT_CONFNAME( 0x01, 0x01, "DD Select (JP5)" )
	PORT_CONFSETTING(    0x01, "ProLogic-DOS" )
	PORT_CONFSETTING(    0x00, "Dolphin-DOS" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c1541_prologic_dos_classic_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1541pdc );
}


//-------------------------------------------------
//  mmu_r -
//-------------------------------------------------

uint8_t c1541_prologic_dos_classic_device::mmu_r(offs_t offset)
{
	u8 const mmu = m_mmu[mmu_address(offset)];
	offs_t const mb_offset = mainboard_address(offset, mmu);
	u8 data = 0;

	if (BIT(mb_offset, 15))
	{
		data = m_rom[mb_offset & 0x3fff];
	}
	else
	{
		switch ((mb_offset >> 10) & 0x07)
		{
		case 0:
		case 1:
			data = m_mb_ram[mb_offset & 0x7ff];
			break;

		case 6:
			data = via0_r(mb_offset & 0x0f);
			break;

		case 7:
			data = m_via1->read(mb_offset & 0x0f);
			break;
		}
	}

	if (!BIT(mmu, 0))
	{
		data = m_ram[offset & 0x1fff];
	}

	if (!BIT(mmu, 1))
	{
		data = m_kernal[(BIT(mmu, 7) << 14) | (BIT(mmu, 3) << 13) | (offset & 0x1fff)];
	}

	if (!BIT(mmu, 2))
	{
		data = m_pia->read((offset >> 2) & 0x03);
	}

	return data;
}


//-------------------------------------------------
//  mmu_w -
//-------------------------------------------------

void c1541_prologic_dos_classic_device::mmu_w(offs_t offset, uint8_t data)
{
	u8 const mmu = m_mmu[mmu_address(offset)];
	offs_t const mb_offset = mainboard_address(offset, mmu);

	if (!BIT(mb_offset, 15))
	{
		switch ((mb_offset >> 10) & 0x07)
		{
		case 0:
		case 1:
			m_mb_ram[mb_offset & 0x7ff] = data;
			break;

		case 6:
			m_via0->write(mb_offset & 0x0f, data);
			break;

		case 7:
			m_via1->write(mb_offset & 0x0f, data);
			break;
		}
	}

	if (!BIT(mmu, 0))
	{
		m_ram[offset & 0x1fff] = data;
	}

	if (!BIT(mmu, 2))
	{
		m_pia->write((offset >> 2) & 0x03, data);
	}
}


//-------------------------------------------------
//  ADDRESS_MAP( c1541pdc_mem )
//-------------------------------------------------

void c1541_prologic_dos_classic_device::c1541pdc_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(c1541_prologic_dos_classic_device::mmu_r), FUNC(c1541_prologic_dos_classic_device::mmu_w));
}

void c1541_prologic_dos_classic_device::pia_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0       1/2 MHz
	    1
	    2
	    3
	    4
	    5
	    6
	    7       MMU A8

	*/

	const XTAL clock = 4_MHz_XTAL / (BIT(data, 0) ? 4 : 2);

	m_maincpu->set_unscaled_clock(clock);
	m_via0->set_unscaled_clock(clock);
	m_via1->set_unscaled_clock(clock);

	m_mmu_a8 = BIT(data, 7);
}

void c1541_prologic_dos_classic_device::pia_pb_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (m_other)
		m_other->parallel_data_w(data | ~mem_mask);
}

void c1541_prologic_dos_classic_device::pia_cb2_w(int state)
{
	if (m_other)
		m_other->parallel_strobe_w(state);
}

void c1541_prologic_dos_classic_device::parallel_data_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(c1541_prologic_dos_classic_device::parallel_data_sync), this), data);
}

TIMER_CALLBACK_MEMBER(c1541_prologic_dos_classic_device::parallel_data_sync)
{
	m_parallel_data = param;
}

void c1541_prologic_dos_classic_device::parallel_select_w(int state)
{
	m_pd_select = state;
}

void c1541_prologic_dos_classic_device::parallel_strobe_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(c1541_prologic_dos_classic_device::parallel_strobe_sync), this), state);
}

TIMER_CALLBACK_MEMBER(c1541_prologic_dos_classic_device::parallel_strobe_sync)
{
	m_pia->cb1_w(param);
	m_via0->write_cb1(param);
}

void c1541_prologic_dos_classic_device::via0_pa_w(uint8_t data)
{
	if (m_other)
		m_other->parallel_data_w(data);
}

void c1541_prologic_dos_classic_device::via0_ca2_w(int state)
{
	if (m_other)
		m_other->parallel_strobe_w(state);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1541_prologic_dos_classic_device::device_add_mconfig(machine_config &config)
{
	c1541_device_base::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &c1541_prologic_dos_classic_device::c1541pdc_mem);

	PIA6821(config, m_pia);
	m_pia->readpb_handler().set([this] () -> u8 { return m_parallel_data; });
	m_pia->writepa_handler().set(FUNC(c1541_prologic_dos_classic_device::pia_pa_w));
	m_pia->writepb_handler().set(FUNC(c1541_prologic_dos_classic_device::pia_pb_w));
	m_pia->cb2_handler().set(FUNC(c1541_prologic_dos_classic_device::pia_cb2_w));
	m_pia->ca2_handler().set(FUNC(c1541_prologic_dos_classic_device::mmu_a5_w));

	m_via0->readpa_handler().set([this] () -> u8 { return m_parallel_data; });
	m_via0->writepa_handler().set(FUNC(c1541_prologic_dos_classic_device::via0_pa_w));
	m_via0->ca2_handler().set(FUNC(c1541_prologic_dos_classic_device::via0_ca2_w));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1541_prologic_dos_classic_device - constructor
//-------------------------------------------------

c1541_prologic_dos_classic_device::c1541_prologic_dos_classic_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	c1541_device_base(mconfig, C1541_PROLOGIC_DOS_CLASSIC, tag, owner, clock),
	device_c64_floppy_parallel_interface(mconfig, *this),
	m_pia(*this, "ic2"),
	m_ram(*this, "ram", 0x2000, ENDIANNESS_LITTLE),
	m_mb_ram(*this, "mb_ram", 0x800, ENDIANNESS_LITTLE),
	m_rom(*this, M6502_TAG),
	m_mmu(*this, "ic5"),
	m_kernal(*this, "ic4"),
	m_jp5(*this, "JP5"),
	m_mmu_a5(1),
	m_mmu_a8(1),
	m_dd_select(1),
	m_pd_select(1)
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c1541_prologic_dos_classic_device::device_reset()
{
	c1541_device_base::device_reset();

	m_dd_select = BIT(m_jp5->read(), 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1541_prologic_dos_classic_device::device_start()
{
	c1541_device_base::device_start();

	m_parallel_data = 0xff;

	save_item(NAME(m_mmu_a5));
	save_item(NAME(m_mmu_a8));
	save_item(NAME(m_dd_select));
	save_item(NAME(m_pd_select));
	save_item(NAME(m_parallel_data));
}
