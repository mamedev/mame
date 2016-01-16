// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


  Bally Astrocade RAM expansion emulation

      RAM Expansions (info below courtesy of Paul Thacker)

      Several third party RAM expansions have been made for the Astrocade.  These
      allow access to various ranges of the expansion memory ($5000 to $FFFF).
      A RAM expansion is required to use extended BASIC programs like Blue RAM BASIC
      and VIPERSoft BASIC.  All of the expansions also have a RAM protect switch, which
      can be flipped at any time to make the RAM act like ROM.  Extended BASIC
      programs need access to the RAM and won't work with RAM protect enabled, but
      this can be useful with Bally and Astrocade BASIC.  They also have a range switch
      (not implemented).  The default position is 6K, but it can be switched to
      2K.  This means that the expanded memory starting at $6000 will instead be
      mapped to the cartridge memory starting at $2000.  So it would be possible to
      load a cartridge program from tape into the expansion memory, then flip the range
      switch and run it as a cartridge.  This is useful for cartridge development.

      Blue RAM -- available in 4K, 16K, and 32K.  These also use an INS8154 chip,
      (not yet implemented) which has an additional $80 bytes of RAM mapped
      immediately after the end of the expansion address space.  This memory
      can't be write protected.  The INS8154 has I/O features needed for loading
      tape programs into Blue RAM BASIC, as well as running the Blue RAM Utility cart.
      4K:  $6000 to $6FFF (can't run VIPERSoft BASIC, because this program needs memory
      past this range)
      16K:  $6000 to $9FFF
      32K:  $6000 to $DFFF

      VIPER System 1 -- This is available in 16K only.  It also includes a keyboard (not implemented).
      16K:  $6000 to $9FFF

      Lil' WHITE RAM -- This is available in 32K only.  Attempts to read and write
      to memory outside of its address range ($D000 to $FFFF) are mapped to the expansion
      memory $5000 to $7FFF.  The current implementation won't allow the shadow RAM area
      to be accessed when RAM protect is on, but there is no known software that will
      access the upper range of the expansion RAM when RAM protect is enabled.
      32K:  $5000 to $CFFF

      R&L 64K RAM Board -- This is a highly configurable kit.  RAM can be installed in
      2K increments.  So, the entire 44K expansion memory can be filled.  It is also
      possible to override the rest of the memory map with RAM (not implemented).
      There are 32 switches allowing users to activate and deactivate each 2K block (not implemented).
      RAM write protection can be implemented in three ranges through jumpers or by
      installing switches.  The ranges are $0000 to $0FFF (first 4K), $0000 to $3FFF (first 16K),
      and $0000 to $FFFF (all 64K).  The current implementation is for 44K expansion memory mapped from
      $5000 to $FFFF, with only a single write protect covering this entire range.

 ***********************************************************************************************************/


#include "emu.h"
#include "ram.h"


//-------------------------------------------------
//  astrocade_rom_device - constructor
//-------------------------------------------------

const device_type ASTROCADE_BLUERAM_4K  = &device_creator<astrocade_blueram_4k_device>;
const device_type ASTROCADE_BLUERAM_16K = &device_creator<astrocade_blueram_16k_device>;
const device_type ASTROCADE_BLUERAM_32K = &device_creator<astrocade_blueram_32k_device>;
const device_type ASTROCADE_VIPER_SYS1  = &device_creator<astrocade_viper_sys1_device>;
const device_type ASTROCADE_WHITERAM    = &device_creator<astrocade_whiteram_device>;
const device_type ASTROCADE_RL64RAM     = &device_creator<astrocade_rl64ram_device>;


astrocade_blueram_4k_device::astrocade_blueram_4k_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_astrocade_card_interface(mconfig, *this),
						m_write_prot(*this, "RAM_PROTECT")
{
}

astrocade_blueram_4k_device::astrocade_blueram_4k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, ASTROCADE_BLUERAM_4K, "Bally Astrocade Blue RAM 4K", tag, owner, clock, "astrocade_br4", __FILE__),
						device_astrocade_card_interface(mconfig, *this),
						m_write_prot(*this, "RAM_PROTECT")
{
}

astrocade_blueram_16k_device::astrocade_blueram_16k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: astrocade_blueram_4k_device(mconfig, ASTROCADE_BLUERAM_16K, "Bally Astrocade Blue RAM 16K", tag, owner, clock, "astrocade_br16", __FILE__)
{
}

astrocade_blueram_32k_device::astrocade_blueram_32k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: astrocade_blueram_4k_device(mconfig, ASTROCADE_BLUERAM_32K, "Bally Astrocade Blue RAM 32K", tag, owner, clock, "astrocade_br32", __FILE__)
{
}

astrocade_viper_sys1_device::astrocade_viper_sys1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, ASTROCADE_VIPER_SYS1, "Bally Astrocade Viper System 1", tag, owner, clock, "astrocade_vs1", __FILE__),
						device_astrocade_card_interface(mconfig, *this),
						m_write_prot(*this, "RAM_PROTECT")
{
}

astrocade_whiteram_device::astrocade_whiteram_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, ASTROCADE_WHITERAM, "Bally Astrocade Lil' White RAM 32K", tag, owner, clock, "astrocade_lwr", __FILE__),
						device_astrocade_card_interface(mconfig, *this),
						m_write_prot(*this, "RAM_PROTECT")
{
}

astrocade_rl64ram_device::astrocade_rl64ram_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, ASTROCADE_RL64RAM, "Bally Astrocade R&L RAM 64K", tag, owner, clock, "astrocade_rl64", __FILE__),
						device_astrocade_card_interface(mconfig, *this),
						m_write_prot(*this, "RAM_PROTECT")
{
}


//-------------------------------------------------
//  RAM Write protect switch
//-------------------------------------------------

static INPUT_PORTS_START( exp_switches )
	PORT_START("RAM_PROTECT")
	PORT_CONFNAME( 0x01, 0x00, "Write Protect RAM")
	PORT_CONFSETTING( 0x00, DEF_STR(Off))
	PORT_CONFSETTING( 0x01, DEF_STR(On))
INPUT_PORTS_END


ioport_constructor astrocade_blueram_4k_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( exp_switches );
}

ioport_constructor astrocade_viper_sys1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( exp_switches );
}

ioport_constructor astrocade_whiteram_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( exp_switches );
}

ioport_constructor astrocade_rl64ram_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( exp_switches );
}

/*-------------------------------------------------
 specific handlers
 -------------------------------------------------*/

// Blue RAM expansions have RAM starting at 0x6000, up to the RAM size
READ8_MEMBER(astrocade_blueram_4k_device::read)
{
	if (offset >= 0x1000 && offset < 0x1000 + m_ram.size())
		return m_ram[offset - 0x1000];
	else
		return 0;
}

WRITE8_MEMBER(astrocade_blueram_4k_device::write)
{
	if (offset >= 0x1000 && offset < 0x1000 + m_ram.size() && !m_write_prot->read())
		m_ram[offset - 0x1000] = data;
}



// Viper System 1 expansion has RAM in 0x6000-0x9fff
READ8_MEMBER(astrocade_viper_sys1_device::read)
{
	if (offset >= 0x1000 && offset < 0xa000)
		return m_ram[offset - 0x1000];
	else
		return 0;
}

WRITE8_MEMBER(astrocade_viper_sys1_device::write)
{
	if (offset >= 0x1000 && offset < 0xa000 && !m_write_prot->read())
		m_ram[offset - 0x1000] = data;
}



// Lil' WHITE RAM expansion has RAM in 0x5000-0xcfff + a mirror of the first 0x3000 bytes up to 0xffff
READ8_MEMBER(astrocade_whiteram_device::read)
{
	return m_ram[offset % 0x8000];
}

WRITE8_MEMBER(astrocade_whiteram_device::write)
{
	if (!m_write_prot->read())
		m_ram[offset % 0x8000] = data;
}



// R&L 64K RAM Board (44KB installed) has RAM in 0x5000-0xffff
READ8_MEMBER(astrocade_rl64ram_device::read)
{
	return m_ram[offset];
}

WRITE8_MEMBER(astrocade_rl64ram_device::write)
{
	if (!m_write_prot->read())
		m_ram[offset] = data;
}
