// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Bryan McPhail
/**************************************************************************************

    deco6280.cpp

    Functions to emulate the Data East HuC6280( Sticker says "45" at most Data East PCBs ) CPU Based Sound HWs

	Original code by Bryan McPhail

***************************************************************************************

    Configuration with YM2151 + Single OKI, used by:
		
		dietgo.cpp
		* Diet Go Go
		
		funkyjet.cpp
		* Funky Jet
		* Sotsugyo Shousho (Mitchell / Atlus license)

		supbtime.cpp
		* China Town
		* Super Burger Time
		* Tumble Pop

	Configuration with YM2151 + Dual OKI, used by:

		boogwing.cpp
		* Boogie Wings (also knowns The Great Ragtime Show in japan)

		cninja.cpp
		* Mutant Fighter (also knowns Death Brade in japan)

		deco32.cpp
		* Captain America and The Avengers
		* Dragon Gun (with Third OKIM6295 Controlled with DECO101(ARM Based) Custom CPU)
		* Fighters History (DE-0380-2, DE-0395-1 PCB)
		* Locked'n Loaded (Dragon Gun Conversion)
		* Night Slashers (DE-0395-1 PCB)

		rohga.cpp
		* Rohga Armor Force (also knowns Wolf Fang - Kuhga 2001 in japan)
		* Nitro Ball (also knowns Gun Ball in japan)
		* Hangzo (Hot-B, Unreleased)
		* Wizard Fire (also knowns Dark Seal II in japan)
		* Schmeiser Robo (Hot-B)
		
	Configuration with YM2203 + YM2151 + Dual OKI, used by:

		madmotor.cpp
		* Mad Motor (Mitchell)

		vaportra.cpp
		* Vapor Trail (also knowns Kuhga in japan)

		cbuster.cpp
		* Crude Buster (also knowns Two Crudes in US)

		cninja.cpp
		* Caveman Ninja (also knowns Joe & Mac in japan)
		* Edward Randy
		* Robocop 2

		darkseal.cpp
		* Dark Seal (also knowns Gate of Doom in US)

		dassault.cpp
		* Thunder Zone (also knowns Desert Assault in US)

***************************************************************************************

HuC6280 MEMORY MAP

Function                                                     Address       Bank   R/W
---------------------------------------------------------------------------------------
Program ROM (64K bytes)                                      000000-00FFFF 00-07  R

YM2203 OPN (Sound Effects)                                   100000-100001 80     R/W
YM2151 OPM (Music)                                           110000-110001 88     R/W
First OKIM6295 (Used as an Sound Effects at most case)*      120000-120001 90     R/W
Second OKIM6295 (Used as an instruments at most case)*       130000-130001 98     R/W
Read Latch                                                   140000-140001 A0     R
Program RAM                                                  1F0000-1F1FFF F8     R/W
CPU Internal Functions                                       1FE000-1FFFFF FF     R/W

* OKIM6295 Banks is selected by YM2151 CT pins.

***************************************************************************************/

#include "emu.h"
#include "audio/deco6280.h"

#define MASTER_CLOCK            XTAL(32'220'000)

#define YM2203_TAG "ym2203"
#define YM2151_TAG "ym2151"
#define OKI1_TAG "oki1"
#define OKI2_TAG "oki2"
#define LATCH_TAG "soundlatch"
#define CPU_TAG "cpu"
#define OKI1BANK_TAG "oki1bank"
#define OKI2BANK_TAG "oki2bank"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DECO6280,               deco_6280_base_device,          "deco6280",  "DECO HuC6280 Based Sound System")
DEFINE_DEVICE_TYPE(DECO6280_2XOKI,         deco_6280_2xoki_device,         "deco2oki",  "DECO HuC6280 Based Sound System (with 2X OKIM6295)")
DEFINE_DEVICE_TYPE(DECO6280_YM2203_2XOKI,  deco_6280_ym2203_2xoki_device,  "deco2203",  "DECO HuC6280 Based Sound System (with YM2203, 2X OKIM6295)")


//**************************************************************************
//  MEMORY MAPS
//**************************************************************************

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, deco_6280_base_device )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_NOP
	AM_RANGE(0x110000, 0x110001) AM_DEVREADWRITE(YM2151_TAG, ym2151_device, read, write)
	AM_RANGE(0x120000, 0x120001) AM_DEVREADWRITE(OKI1_TAG, okim6295_device, read, write)
	AM_RANGE(0x130000, 0x130001) AM_NOP
	AM_RANGE(0x140000, 0x140001) AM_READ(soundlatch_r)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x1fec00, 0x1fec01) AM_DEVWRITE(CPU_TAG, h6280_device, timer_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_DEVWRITE(CPU_TAG, h6280_device, irq_status_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_2xoki_map, AS_PROGRAM, 8, deco_6280_2xoki_device )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_NOP
	AM_RANGE(0x110000, 0x110001) AM_DEVREADWRITE(YM2151_TAG, ym2151_device, read, write)
	AM_RANGE(0x120000, 0x120001) AM_DEVREADWRITE(OKI1_TAG, okim6295_device, read, write)
	AM_RANGE(0x130000, 0x130001) AM_DEVREADWRITE(OKI2_TAG, okim6295_device, read, write)
	AM_RANGE(0x140000, 0x140001) AM_READ(soundlatch_r)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x1fec00, 0x1fec01) AM_DEVWRITE(CPU_TAG, h6280_device, timer_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_DEVWRITE(CPU_TAG, h6280_device, irq_status_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_ym2203_2xoki_map, AS_PROGRAM, 8, deco_6280_ym2203_2xoki_device )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_DEVREADWRITE(YM2203_TAG, ym2203_device, read, write)
	AM_RANGE(0x110000, 0x110001) AM_DEVREADWRITE(YM2151_TAG, ym2151_device, read, write)
	AM_RANGE(0x120000, 0x120001) AM_DEVREADWRITE(OKI1_TAG, okim6295_device, read, write)
	AM_RANGE(0x130000, 0x130001) AM_DEVREADWRITE(OKI2_TAG, okim6295_device, read, write)
	AM_RANGE(0x140000, 0x140001) AM_READ(soundlatch_r)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x1fec00, 0x1fec01) AM_DEVWRITE(CPU_TAG, h6280_device, timer_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_DEVWRITE(CPU_TAG, h6280_device, irq_status_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( deco6280_oki1_map, 0, 8, deco_6280_2xoki_device )
	AM_RANGE(0x00000, 0x3ffff) AM_ROMBANK(OKI1BANK_TAG)
ADDRESS_MAP_END


static ADDRESS_MAP_START( deco6280_oki2_map, 0, 8, deco_6280_2xoki_device )
	AM_RANGE(0x00000, 0x3ffff) AM_ROMBANK(OKI2BANK_TAG)
ADDRESS_MAP_END


//**************************************************************************
//  BASE DEVICE CLASS
//**************************************************************************

//-------------------------------------------------
//  deco_6280_base_device - constructor
//-------------------------------------------------

deco_6280_base_device::deco_6280_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: deco_6280_base_device(mconfig, DECO6280, tag, owner, clock, 3)
{
}

deco_6280_base_device::deco_6280_base_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner, uint32_t clock, int channels)
	: device_t(mconfig, devtype, tag, owner, clock),
		device_mixer_interface(mconfig, *this, channels),
		m_cpu(*this, CPU_TAG),
		m_ym2151(*this, YM2151_TAG),
		m_oki1(*this, OKI1_TAG),
		m_soundlatch(*this, LATCH_TAG),
		m_oki1_region(*this, OKI1_TAG),
		m_soundlatch_cb(*this)
{
}

//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void deco_6280_base_device::device_start()
{
	// resolve devices
	m_soundlatch_cb.resolve_safe(0);
}


//-------------------------------------------------
//  device_reset: Reset the device
//-------------------------------------------------

void deco_6280_base_device::device_reset()
{
}


//-------------------------------------------------
//  soundlatch_r: Handle soundlatch reads
//-------------------------------------------------

READ8_MEMBER( deco_6280_base_device::soundlatch_r )
{
	if (m_soundlatch_cb.isnull())
	{
		return m_soundlatch->read(space,0);
	}

	return m_soundlatch_cb();
}


//-------------------------------------------------
//  soundlatch_w: Handle soundlatch writes
//-------------------------------------------------

WRITE8_MEMBER( deco_6280_base_device::soundlatch_w )
{
	m_soundlatch->write(space,0,data,mem_mask);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(deco_6280_base_device::device_add_mconfig)

	// basic machine hardware
	MCFG_CPU_ADD(CPU_TAG, H6280, DERIVED_CLOCK(1,1))
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_GENERIC_LATCH_8_ADD(LATCH_TAG)
	MCFG_GENERIC_LATCH_DATA_PENDING_CB(INPUTLINE(CPU_TAG, 0))

	// sound hardware
	MCFG_YM2151_ADD(YM2151_TAG, MASTER_CLOCK/9)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE(CPU_TAG, 1)) // IRQ2
	MCFG_MIXER_ROUTE(0, DEVICE_SELF_OWNER, 1, DECO_YM2151_OUT0)
	MCFG_MIXER_ROUTE(1, DEVICE_SELF_OWNER, 1, DECO_YM2151_OUT1)

	MCFG_OKIM6295_ADD(OKI1_TAG, MASTER_CLOCK/32, PIN7_HIGH)
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1, DECO_OKI1_OUT)
MACHINE_CONFIG_END


//**************************************************************************
//  BASE DEVICE CLASS FOR DUAL OKI6295-BASED VERSIONS
//**************************************************************************

//-------------------------------------------------
//  deco_6280_2xoki_device: Constructor
//-------------------------------------------------

deco_6280_2xoki_device::deco_6280_2xoki_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: deco_6280_2xoki_device(mconfig, DECO6280_2XOKI, tag, owner, clock, 4)
{
}

deco_6280_2xoki_device::deco_6280_2xoki_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner, uint32_t clock, int channels)
	: deco_6280_base_device(mconfig, devtype, tag, owner, clock, channels),
		m_oki2(*this, OKI2_TAG),
		m_oki2_region(*this, OKI2_TAG),
		m_oki1_bank(*this, OKI1BANK_TAG),
		m_oki2_bank(*this, OKI2BANK_TAG),
		m_oki1_bankbase(0),
		m_oki2_bankbase(0),
		m_ym2151_cb(*this)
{
}

//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void deco_6280_2xoki_device::device_start()
{
	m_ym2151_cb.resolve_safe();

	// call the parent
	deco_6280_base_device::device_start();
	
	// configure ADPCM banking
	uint32_t size;
	size = m_oki1_region->bytes();
	if (size > 0x40000)
		m_oki1_bank->configure_entries(0, size / 0x40000, m_oki1_region->base(), 0x40000);
	else
		m_oki1_bank->set_base(m_oki1_region->base());

	size = m_oki2_region->bytes();
	if (size > 0x40000)
		m_oki2_bank->configure_entries(0, size / 0x40000, m_oki2_region->base(), 0x40000);
	else
		m_oki2_bank->set_base(m_oki2_region->base());

	// save states
	save_item(NAME(m_oki1_bankbase));
	save_item(NAME(m_oki2_bankbase));

}


//-------------------------------------------------
//  device_reset: Reset the device
//-------------------------------------------------

void deco_6280_2xoki_device::device_reset()
{
	// call the parent
	deco_6280_base_device::device_reset();
	
	m_oki1_bankbase = 0;
	m_oki2_bankbase = 0;
	m_oki1_bank->set_entry(m_oki1_bankbase);
	m_oki2_bank->set_entry(m_oki2_bankbase);
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void deco_6280_2xoki_device::device_post_load()
{
	m_oki1_bank->set_entry(m_oki1_bankbase);
	m_oki2_bank->set_entry(m_oki2_bankbase);
}


//-------------------------------------------------
//  ym2151_port_w: Handle writes from the YM2151
//  output port
//-------------------------------------------------

WRITE8_MEMBER( deco_6280_2xoki_device::ym2151_port_w )
{
	if (m_ym2151_cb.isnull())
		return;

	m_ym2151_cb(data);
}


//-------------------------------------------------
//  default_banked_oki12_w: Default both OKI
//  bankswitch configuration
//-------------------------------------------------

WRITE8_MEMBER( deco_6280_2xoki_device::default_banked_oki12_w )
{
	oki1_bank_w((data >> 0) & 1, 1);
	oki2_bank_w((data >> 1) & 1, 1);
}


//-------------------------------------------------
//  default_banked_oki12_w: Default 2nd OKI
//  bankswitch configuration
//-------------------------------------------------

WRITE8_MEMBER( deco_6280_2xoki_device::default_banked_oki2_w )
{
	oki2_bank_w(data & 1, 1);
}


//-------------------------------------------------
//  oki1_bank_w: Handle writes to OKIM6295 - 1 Bank
//-------------------------------------------------

void deco_6280_2xoki_device::oki1_bank_w(uint32_t bank, uint32_t mask)
{
	if (m_oki1_bank.found())
	{
		m_oki1_bankbase = (m_oki1_bankbase & ~mask) | (bank & mask);
		m_oki1_bank->set_entry(m_oki1_bankbase);
	}
}


//-------------------------------------------------
//  oki2_bank_w: Handle writes to OKIM6295 - 2 Bank
//-------------------------------------------------

void deco_6280_2xoki_device::oki2_bank_w(uint32_t bank, uint32_t mask)
{
	if (m_oki2_bank.found())
	{
		m_oki2_bankbase = (m_oki2_bankbase & ~mask) | (bank & mask);
		m_oki2_bank->set_entry(m_oki2_bankbase);
	}
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(deco_6280_2xoki_device::device_add_mconfig)

	deco_6280_base_device::device_add_mconfig(config);

	// basic machine hardware
	MCFG_CPU_MODIFY(CPU_TAG)
	MCFG_CPU_PROGRAM_MAP(sound_2xoki_map)

	// sound hardware
	MCFG_SOUND_MODIFY(YM2151_TAG)
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(deco_6280_2xoki_device, ym2151_port_w))

	MCFG_SOUND_MODIFY(OKI1_TAG)
	MCFG_DEVICE_ADDRESS_MAP(0, deco6280_oki1_map)

	MCFG_OKIM6295_ADD(OKI2_TAG, MASTER_CLOCK/16, PIN7_HIGH)
	MCFG_DEVICE_ADDRESS_MAP(0, deco6280_oki2_map)
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1, DECO_OKI2_OUT)
MACHINE_CONFIG_END


//**************************************************************************
//  DECO6280_YM2203_2XOKI SPECIFIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  deco_6280_ym2203_2xoki_device: Constructor
//-------------------------------------------------

deco_6280_ym2203_2xoki_device::deco_6280_ym2203_2xoki_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: deco_6280_2xoki_device(mconfig, DECO6280_YM2203_2XOKI, tag, owner, clock, 5),
		m_ym2203(*this, YM2203_TAG)
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(deco_6280_ym2203_2xoki_device::device_add_mconfig)

	deco_6280_2xoki_device::device_add_mconfig(config);

	// basic machine hardware
	MCFG_CPU_MODIFY(CPU_TAG)
	MCFG_CPU_PROGRAM_MAP(sound_ym2203_2xoki_map)

	// sound hardware
	MCFG_SOUND_ADD(YM2203_TAG, YM2203, MASTER_CLOCK/8)
	MCFG_MIXER_ROUTE(0, DEVICE_SELF_OWNER, 1, DECO_YM2203_OUT)
	MCFG_MIXER_ROUTE(1, DEVICE_SELF_OWNER, 1, DECO_YM2203_OUT)
	MCFG_MIXER_ROUTE(2, DEVICE_SELF_OWNER, 1, DECO_YM2203_OUT)
	MCFG_MIXER_ROUTE(3, DEVICE_SELF_OWNER, 1, DECO_YM2203_OUT)
MACHINE_CONFIG_END


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void deco_6280_ym2203_2xoki_device::device_start()
{
	// call the parent
	deco_6280_2xoki_device::device_start();
}


//-------------------------------------------------
//  device_reset: Reset the device
//-------------------------------------------------

void deco_6280_ym2203_2xoki_device::device_reset()
{
	// call the parent
	deco_6280_2xoki_device::device_reset();
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void deco_6280_ym2203_2xoki_device::device_post_load()
{
	deco_6280_2xoki_device::device_post_load();
}