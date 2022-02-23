// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANC01 6502 2nd Processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANC01_65022ndproc.html

    Acorn 6502 2nd Processor (pre-production)
      Pre-production Iss.C and Iss.F boards have been seen running earlier versions
      of the Tube OS. The boards are different in that they map the Tube ULA at
      &FF78-&FF7F instead of the usual &FEF8-&FEFF. They also require an earlier
      version of the Tube Host which can be found in NFS 3.34, the usual DNFS 1.20
      will hang at the Tube startup banner.

    Acorn Extended 6502 2nd Processor (non-commercial)
      Used internally at Acorn, it extends the addressing of indexed indirect Y instructions
      to address 256K RAM. Only two applications were created to make use of this, which
      were turboMASM and turboBASIC.

    Acorn ADC06 65C102 Co-processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ADC06_65C102CoPro.html

**********************************************************************/


#include "emu.h"
#include "tube_6502.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_6502,    bbc_tube_6502_device,    "bbc_tube_6502",    "Acorn 6502 2nd Processor")
DEFINE_DEVICE_TYPE(BBC_TUBE_6502P,   bbc_tube_6502p_device,   "bbc_tube_6502p",   "Acorn 6502 2nd Processor (pre-production)")
DEFINE_DEVICE_TYPE(BBC_TUBE_6502E,   bbc_tube_6502e_device,   "bbc_tube_6502e",   "Acorn Extended 6502 2nd Processor")
DEFINE_DEVICE_TYPE(BBC_TUBE_65C102,  bbc_tube_65c102_device,  "bbc_tube_65c102",  "Acorn 65C102 Co-Processor")


//-------------------------------------------------
//  ADDRESS_MAP( tube_6502_mem )
//-------------------------------------------------

void bbc_tube_6502_device::tube_6502_mem(address_map &map)
{
	map(0x0000, 0xffff).view(m_view);
	// ROM enabled
	m_view[0](0x0000, 0xffff).ram().share("ram");
	m_view[0](0xf000, 0xffff).rom().region("rom", 0);
	// ROM disabled
	m_view[1](0x0000, 0xffff).ram().share("ram");
	map(0xfef8, 0xfeff).rw(FUNC(bbc_tube_6502_device::tube_r), FUNC(bbc_tube_6502_device::tube_w));
}

void bbc_tube_6502p_device::tube_6502p_mem(address_map &map)
{
	map(0x0000, 0xffff).view(m_view);
	// ROM enabled
	m_view[0](0x0000, 0xffff).ram().share("ram");
	m_view[0](0xf000, 0xffff).rom().region("rom", 0);
	// ROM disabled
	m_view[1](0x0000, 0xffff).ram().share("ram");
	map(0xff78, 0xff7f).rw(FUNC(bbc_tube_6502p_device::tube_r), FUNC(bbc_tube_6502p_device::tube_w));
}

void bbc_tube_6502e_device::tube_6502e_mem(address_map &map)
{
	map(0x0000, 0xffff).view(m_view);
	// ROM enabled
	m_view[0](0x0000, 0xffff).rw(FUNC(bbc_tube_6502e_device::ram_r), FUNC(bbc_tube_6502e_device::ram_w));
	m_view[0](0xf000, 0xffff).rom().region("rom", 0);
	// ROM disabled
	m_view[1](0x0000, 0xffff).rw(FUNC(bbc_tube_6502e_device::ram_r), FUNC(bbc_tube_6502e_device::ram_w));
	map(0xfef8, 0xfeff).rw(FUNC(bbc_tube_6502e_device::tube_r), FUNC(bbc_tube_6502e_device::tube_w));
}


//-------------------------------------------------
//  ROM( tube_6502 )
//-------------------------------------------------

ROM_START( tube_6502 )
	ROM_REGION(0x1000, "rom", 0)
	ROM_DEFAULT_BIOS("110")
	ROM_SYSTEM_BIOS(0, "110", "Tube 1.10")
	ROMX_LOAD("6502tube_110.rom", 0x0000, 0x1000, CRC(98b5fe42) SHA1(338269d03cf6bfa28e09d1651c273ea53394323b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "121", "Tube 1.21 (ReCo6502)")
	ROMX_LOAD("reco6502tube.rom", 0x0000, 0x1000, CRC(75b2a466) SHA1(9ecef24de58a48c3fbe01b12888c3f6a5d24f57f), ROM_BIOS(1))
ROM_END

ROM_START( tube_6502p )
	ROM_REGION(0x1000, "rom", 0)
	ROM_DEFAULT_BIOS("005")
	ROM_SYSTEM_BIOS(0, "005", "Tube 0.05") // Iss.F
	ROMX_LOAD("6502tube_005.rom", 0x0000, 0x1000, CRC(0d4cd088) SHA1(f68a74f2529e2719193f81032af298e606792ce8), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "004", "Tube 0.04")
	ROMX_LOAD("6502tube_004.rom", 0x0000, 0x1000, CRC(64698ffa) SHA1(b7d47ac65291a7d7bd03b6b82ee08cff291c8609), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "001", "Tube 0.01") // Iss.C
	ROMX_LOAD("6502tube_001.rom", 0x0000, 0x1000, CRC(83bee05d) SHA1(4a00d4d3deec0ab43dc6647ac591bd71f4b24a51), ROM_BIOS(2))
ROM_END

ROM_START( tube_6502e )
	ROM_REGION(0x1000, "rom", 0)
	ROM_DEFAULT_BIOS("120")
	ROM_SYSTEM_BIOS(0, "120", "Tube 1.20") // built from source, not an actual dump
	ROMX_LOAD("6502tube256_120.rom", 0x0000, 0x1000, BAD_DUMP CRC(967ee712) SHA1(ebca4bb471cd18608882668edda811ed88b4cee5), ROM_BIOS(0))
ROM_END

ROM_START( tube_65c102 )
	ROM_REGION(0x1000, "rom", 0)
	ROM_DEFAULT_BIOS("110")
	ROM_SYSTEM_BIOS(0, "110", "Tube 1.10")
	ROMX_LOAD("65c102_boot_110.rom", 0x0000, 0x1000, CRC(ad5b70cc) SHA1(0ac9a1c70e55a79e2c81e102afae1d016af229fa), ROM_BIOS(0)) // 2201,243-01
	ROM_SYSTEM_BIOS(1, "120", "Tube 1.20")
	ROMX_LOAD("65c102_boot_120.rom", 0x0000, 0x1000, CRC(1462f0f7) SHA1(7dc03d3c4862159baefebf1662de7e05987ddf7b), ROM_BIOS(1)) // 2201,243-02
	ROM_SYSTEM_BIOS(2, "121", "Tube 1.21 (ReCo6502)")
	ROMX_LOAD("reco6502tube.rom", 0x0000, 0x1000, CRC(75b2a466) SHA1(9ecef24de58a48c3fbe01b12888c3f6a5d24f57f), ROM_BIOS(2))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_6502_device::device_add_mconfig(machine_config &config)
{
	M65C02(config, m_maincpu, 12_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_tube_6502_device::tube_6502_mem);

	TUBE(config, m_ula);
	m_ula->pnmi_handler().set_inputline(m_maincpu, M65C02_NMI_LINE);
	m_ula->pirq_handler().set_inputline(m_maincpu, M65C02_IRQ_LINE);

	RAM(config, m_ram).set_default_size("64K").set_default_value(0);

	SOFTWARE_LIST(config, "flop_ls_6502").set_original("bbc_flop_6502");
}

void bbc_tube_6502p_device::device_add_mconfig(machine_config &config)
{
	bbc_tube_6502_device::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_tube_6502p_device::tube_6502p_mem);
}

void bbc_tube_6502e_device::device_add_mconfig(machine_config &config)
{
	bbc_tube_6502_device::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_tube_6502e_device::tube_6502e_mem);

	m_ram->set_default_size("256K").set_default_value(0);
}

void bbc_tube_65c102_device::device_add_mconfig(machine_config &config)
{
	bbc_tube_6502_device::device_add_mconfig(config);

	m_maincpu->set_clock(16_MHz_XTAL / 4);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_6502_device::device_rom_region() const
{
	return ROM_NAME( tube_6502 );
}

const tiny_rom_entry *bbc_tube_6502p_device::device_rom_region() const
{
	return ROM_NAME( tube_6502p );
}

const tiny_rom_entry *bbc_tube_6502e_device::device_rom_region() const
{
	return ROM_NAME( tube_6502e );
}

const tiny_rom_entry *bbc_tube_65c102_device::device_rom_region() const
{
	return ROM_NAME( tube_65c102 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_6502_device - constructor
//-------------------------------------------------

bbc_tube_6502_device::bbc_tube_6502_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_tube_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_view(*this, "view")
	, m_ula(*this, "ula")
	, m_ram(*this, "ram")
	, m_rom(*this, "rom")
{
}

bbc_tube_6502_device::bbc_tube_6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_tube_6502_device(mconfig, BBC_TUBE_6502, tag, owner, clock)
{
}

bbc_tube_6502p_device::bbc_tube_6502p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_tube_6502_device(mconfig, BBC_TUBE_6502P, tag, owner, clock)
{
}

bbc_tube_6502e_device::bbc_tube_6502e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_tube_6502_device(mconfig, BBC_TUBE_6502E, tag, owner, clock)
	, m_opcode_ind_y(false)
	, m_page(0)
	, m_cycles(0)
{
}

bbc_tube_65c102_device::bbc_tube_65c102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_tube_6502_device(mconfig, BBC_TUBE_65C102, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_6502_device::device_start()
{
}

void bbc_tube_6502e_device::device_start()
{
	save_item(NAME(m_opcode_ind_y));
	save_item(NAME(m_page));
	save_item(NAME(m_cycles));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_6502_device::device_reset()
{
	m_view.select(0);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_tube_6502_device::host_r(offs_t offset)
{
	return m_ula->host_r(offset);
}

void bbc_tube_6502_device::host_w(offs_t offset, uint8_t data)
{
	m_ula->host_w(offset, data);
}


uint8_t bbc_tube_6502_device::tube_r(offs_t offset)
{
	// Disable ROM on first access
	if (!machine().side_effects_disabled())
		m_view.select(1);

	return m_ula->parasite_r(offset);
}

void bbc_tube_6502_device::tube_w(offs_t offset, uint8_t data)
{
	m_ula->parasite_w(offset, data);
}


uint8_t bbc_tube_6502e_device::ram_r(offs_t offset)
{
	uint8_t data =  m_ram->pointer()[offset];

	if (!machine().side_effects_disabled())
	{
		/* opcode fetch */
		if (m_maincpu->get_sync())
		{
			/* indexed indirect Y addressing */
			m_opcode_ind_y = ((data & 0x1f) == 0x11 || (data & 0x1f) == 0x12) ? true : false;

			m_cycles = m_maincpu->total_cycles();
		}

		if (m_opcode_ind_y)
		{
			if (m_maincpu->total_cycles() - m_cycles == 1)
			{
				/* fetch extra byte from &3xx where xx is low byte of ZP address */
				m_page = m_ram->pointer()[0x301 + data] & 0x03;
			}
			else if (m_maincpu->total_cycles() - m_cycles > 3)
			{
				/* read data from extended address */
				data = m_ram->pointer()[m_page << 16 | offset];
			}
		}
	}

	return data;
}

void bbc_tube_6502e_device::ram_w(offs_t offset, uint8_t data)
{
	if (m_opcode_ind_y)
	{
		/* write data to extended address */
		m_ram->pointer()[m_page << 16 | offset] = data;
	}
	else
	{
		m_ram->pointer()[offset] = data;
	}
}
