// license:GPL-2.0+
// copyright-holders:Fabio Priuli, byuu
/***********************************************************************************************************

 Super NES/Famicom (LoROM) cartridge emulation (for SNES/SFC)

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  sns_rom_device - constructor
//-------------------------------------------------

const device_type SNS_LOROM = &device_creator<sns_rom_device>;
const device_type SNS_LOROM_OBC1 = &device_creator<sns_rom_obc1_device>;
// LoROM pirate carts with protection
const device_type SNS_LOROM_POKEMON = &device_creator<sns_rom_pokemon_device>;
const device_type SNS_LOROM_TEKKEN2 = &device_creator<sns_rom_tekken2_device>;
const device_type SNS_LOROM_SOULBLAD = &device_creator<sns_rom_soulblad_device>;
const device_type SNS_LOROM_BANANA = &device_creator<sns_rom_banana_device>;
const device_type SNS_LOROM_BUGSLIFE = &device_creator<sns_rom_bugs_device>;
// LoROM pirate multicarts
const device_type SNS_LOROM_MCPIR1 = &device_creator<sns_rom_mcpirate1_device>;
const device_type SNS_LOROM_MCPIR2 = &device_creator<sns_rom_mcpirate2_device>;
const device_type SNS_LOROM_20COL = &device_creator<sns_rom_20col_device>;


sns_rom_device::sns_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_sns_cart_interface( mconfig, *this )
{
}

sns_rom_device::sns_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, SNS_LOROM, "SNES Cart (LoROM)", tag, owner, clock, "sns_rom", __FILE__),
						device_sns_cart_interface( mconfig, *this )
{
}

sns_rom_obc1_device::sns_rom_obc1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_OBC1, "SNES Cart (LoROM) + OBC-1", tag, owner, clock, "sns_rom_obc1", __FILE__), m_address(0), m_offset(0), m_shift(0)
				{
}



// Pirate LoROM 'mappers'
sns_rom_pokemon_device::sns_rom_pokemon_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_POKEMON, "SNES Pirate Carts with Protection", tag, owner, clock, "sns_rom_pokemon", __FILE__), m_latch(0)
				{
}

sns_rom_tekken2_device::sns_rom_tekken2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_TEKKEN2, "SNES Tekken 2", tag, owner, clock, "sns_rom_tekken2", __FILE__), m_prot(0)
				{
}

sns_rom_soulblad_device::sns_rom_soulblad_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_SOULBLAD, "SNES Soul Blade", tag, owner, clock, "sns_rom_soulblad", __FILE__)
{
}

sns_rom_banana_device::sns_rom_banana_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_BANANA, "SNES Banana de Pijamas", tag, owner, clock, "sns_rom_banana", __FILE__)
{
}

sns_rom_bugs_device::sns_rom_bugs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_BUGSLIFE, "SNES A Bug's Life", tag, owner, clock, "sns_rom_bugslife", __FILE__)
{
}

// Multigame LoROM 'mappers'
sns_rom_mcpirate1_device::sns_rom_mcpirate1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_MCPIR1, "SNES Pirate Multigame Carts Type 1", tag, owner, clock, "sns_rom_mcpirate1", __FILE__), m_base_bank(0)
				{
}

sns_rom_mcpirate2_device::sns_rom_mcpirate2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_MCPIR2, "SNES Pirate Multigame Carts Type 2", tag, owner, clock, "sns_rom_mcpirate2", __FILE__), m_base_bank(0)
				{
}

sns_rom_20col_device::sns_rom_20col_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_20COL, "SNES Super 20 Collection", tag, owner, clock, "sns_rom_20col", __FILE__), m_base_bank(0)
				{
}


void sns_rom_device::device_start()
{
}

void sns_rom_obc1_device::device_start()
{
	save_item(NAME(m_ram));
	save_item(NAME(m_address));
	save_item(NAME(m_offset));
	save_item(NAME(m_shift));
}

void sns_rom_obc1_device::device_reset()
{
	memset(m_ram, 0xff, sizeof(m_ram));
	// or from rom?
	m_offset  = (m_ram[0x1ff5] & 0x01) ? 0x1800 : 0x1c00;
	m_address = (m_ram[0x1ff6] & 0x7f);
	m_shift   = (m_ram[0x1ff6] & 0x03) << 1;
}


void sns_rom_pokemon_device::device_start()
{
	save_item(NAME(m_latch));
}

void sns_rom_pokemon_device::device_reset()
{
	m_latch = 0;
}

void sns_rom_tekken2_device::device_start()
{
	save_item(NAME(m_prot));
}

void sns_rom_tekken2_device::device_reset()
{
	m_prot = 0;
}

void sns_rom_bugs_device::device_start()
{
	save_item(NAME(m_latch));
}

void sns_rom_bugs_device::device_reset()
{
	memset(m_latch, 0, sizeof(m_latch));
}



void sns_rom_mcpirate1_device::device_start()
{
	m_base_bank = 0;
	save_item(NAME(m_base_bank));
}

void sns_rom_mcpirate1_device::device_reset()
{
}

void sns_rom_mcpirate2_device::device_start()
{
	m_base_bank = 0;
	save_item(NAME(m_base_bank));
}

void sns_rom_mcpirate2_device::device_reset()
{
}

void sns_rom_20col_device::device_start()
{
	m_base_bank = 4;
	save_item(NAME(m_base_bank));
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(sns_rom_device::read_l)
{
	return read_h(space, offset);
}

READ8_MEMBER(sns_rom_device::read_h)
{
	int bank = offset / 0x10000;
	return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
}



// Lo-ROM + OBC-1 (used by Metal Combat - Falcon's Revenge)
// same as above but additional read/write handling for the add-on chip

/***************************************************************************

 Based on C++ implementation by Byuu in BSNES.

 Byuu's code is released under GNU General Public License
 version 2 as published by the Free Software Foundation.

 ***********************************************************************************************************/


READ8_MEMBER( sns_rom_obc1_device::chip_read )
{
	UINT16 address = offset & 0x1fff;
	UINT8 value;

	switch (address)
	{
		case 0x1ff0:
			value = m_ram[m_offset + (m_address << 2) + 0];
			break;

		case 0x1ff1:
			value = m_ram[m_offset + (m_address << 2) + 1];
			break;

		case 0x1ff2:
			value = m_ram[m_offset + (m_address << 2) + 2];
			break;

		case 0x1ff3:
			value = m_ram[m_offset + (m_address << 2) + 3];
			break;

		case 0x1ff4:
			value = m_ram[m_offset + (m_address >> 2) + 0x200];
			break;

		default:
			value = m_ram[address];
			break;
	}

	return value;
}


WRITE8_MEMBER( sns_rom_obc1_device::chip_write )
{
	UINT16 address = offset & 0x1fff;
	UINT8 temp;

	switch(address)
	{
		case 0x1ff0:
			m_ram[m_offset + (m_address << 2) + 0] = data;
			break;

		case 0x1ff1:
			m_ram[m_offset + (m_address << 2) + 1] = data;
			break;

		case 0x1ff2:
			m_ram[m_offset + (m_address << 2) + 2] = data;
			break;

		case 0x1ff3:
			m_ram[m_offset + (m_address << 2) + 3] = data;
			break;

		case 0x1ff4:
			temp = m_ram[m_offset + (m_address >> 2) + 0x200];
			temp = (temp & ~(3 << m_shift)) | ((data & 0x03) << m_shift);
			m_ram[m_offset + (m_address >> 2) + 0x200] = temp;
			break;

		case 0x1ff5:
			m_offset = (data & 0x01) ? 0x1800 : 0x1c00;
			m_ram[address & 0x1fff] = data;
			break;

		case 0x1ff6:
			m_address = data & 0x7f;
			m_shift = (data & 0x03) << 1;
			m_ram[address & 0x1fff] = data;
			break;

		default:
			m_ram[address & 0x1fff] = data;
			break;
	}
}



// Lo-ROM + Protection devices used in pirate carts


// Pokemon (and many others): a byte is written and a permutation of its bits must be returned.
// Address range for read/write depends on the game (check snes.xml)
READ8_MEMBER( sns_rom_pokemon_device::chip_read )
{
	return BITSWAP8(m_latch,0,6,7,1,2,3,4,5);
}

WRITE8_MEMBER( sns_rom_pokemon_device::chip_write )
{
	m_latch = data;
}


// Tekken 2: It accesses the protection in a very strange way, always reading/writing the same data $f0 times,
// because each access must be repeated a couple of times to be registered (typically around 7-30 times)
// They probably used a microcontroller here.
// The protection itself is accessed in banks $80-$bf. Accessing (read/write, doesn't matter) address lines
// A8,A9,A10 in these banks in a certain sequence makes the mc return a 4bit value. [d4s]
// Details on a possible algorythm behind the sequence of accesses were provided by nocash. Thanks!
void sns_rom_tekken2_device::update_prot(UINT32 offset)
{
	// accesses to [80-bf][8000-87ff] ranges update the protection value
	offset &= 0x7ff;

	switch (offset & 0x700)
	{
		case 0x000:
			m_prot = 0;
			break;
		case 0x100:
			// used for read access
			break;
		case 0x200: // BIT 0
			m_prot |= 1;
			break;
		case 0x300: // BIT 1
			m_prot |= 2;
			break;
		case 0x400: // BIT 2
			m_prot |= 4;
			break;
		case 0x500: // BIT 3
			m_prot |= 8;
			break;
		case 0x600: // DIRECTION
			m_prot |= 0x10;
			break;
		case 0x700: // FUNCTION
			m_prot |= 0x20;
			break;
	}
}

READ8_MEMBER( sns_rom_tekken2_device::chip_read )
{
	update_prot(offset);

	if ((offset & 0x700) == 0x100)
	{
		if (BIT(m_prot, 5))     // FUNCTION = 1 means Shift
		{
			if (BIT(m_prot, 4))     // DIRECTION = 1 means Right
				return (m_prot & 0x0f) >> 1;
			else                    // DIRECTION = 0 means Left
				return (m_prot & 0x0f) << 1;
		}
		else                        // FUNCTION = 0 means Add/Sub
		{
			if (BIT(m_prot, 4))     // DIRECTION = 1 means Minus
				return (m_prot & 0x0f) - 1;
			else                    // DIRECTION = 0 means Plus
				return (m_prot & 0x0f) + 1;
		}
	}

	return 0xff; // should be open_bus
}

WRITE8_MEMBER( sns_rom_tekken2_device::chip_write )
{
	update_prot(offset);
}


// Soul Blade: Adresses $xxx0-$xxx3 in banks $80-$bf always read $55, $0f, $aa, $f0.
// Banks $c0-$ff return open bus.
READ8_MEMBER( sns_rom_soulblad_device::chip_read )
{
	UINT8 value;
	offset &= 3;
	switch (offset)
	{
		case 0:
		default:
			value = 0x55;
			break;
		case 1:
			value = 0x0f;
			break;
		case 2:
			value = 0xaa;
			break;
		case 3:
			value = 0xf0;
			break;
	}
	return value;
}

// Multicart pirate banking emulation
// LoROM games, writes to [ff][ff00-ffff] control bankswitch
// The actual banks depends on the last 8bits of the address accessed.

// Type 1: bits0-4 of the address are used as base bank (256KB chunks)
READ8_MEMBER(sns_rom_mcpirate1_device::read_l)
{
	return read_h(space, offset);
}

READ8_MEMBER(sns_rom_mcpirate1_device::read_h)
{
	int bank = (offset / 0x10000) + (m_base_bank * 8);
	return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
}

WRITE8_MEMBER( sns_rom_mcpirate1_device::chip_write )
{
	m_base_bank = offset & 0x1f;
//  printf("offset %X data %X bank %X\n", offset, data, m_base_bank);
}

// Type 2: bits0-3 & bit5 of the address are used as base bank (256KB chunks)
READ8_MEMBER(sns_rom_mcpirate2_device::read_l)
{
	return read_h(space, offset);
}

READ8_MEMBER(sns_rom_mcpirate2_device::read_h)
{
	int bank = (offset / 0x10000) + (m_base_bank * 8);
	return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
}

WRITE8_MEMBER( sns_rom_mcpirate2_device::chip_write )
{
	m_base_bank = (offset & 0x0f) | ((offset & 0x20) >> 1);
//  printf("offset %X data %X bank %X\n", offset, data, m_base_bank);
}

// Korean 20 in 1 collection with NES games
// - base bank is selected (in 32KB chunks) by bits 0-4 of data written at 0x808000
// - bits 6-7 seem related to prg size: 0x00 means 4*32KB, 0xc0 means 2*32KB, 0x80 means 1*32KB
//   (they are used to setup how large is the ROM to be accessed, games 15-20 don't work well if
//   accesses in [01-3f] don't go to the only 32KB bank)
// - bit 5 is always 0
// it's worth to notice that for FC games size of bank is twice the size of original FC PRG
READ8_MEMBER(sns_rom_20col_device::read_l)
{
	return read_h(space, offset);
}

READ8_MEMBER(sns_rom_20col_device::read_h)
{
	int prg32k = (!BIT(m_base_bank, 6) && BIT(m_base_bank, 7));
	int bank = prg32k ? 0 : (offset / 0x10000);
	return m_rom[((m_base_bank & 0x1f) + bank) * 0x8000 + (offset & 0x7fff)];
}

WRITE8_MEMBER( sns_rom_20col_device::chip_write )
{
	// [#]  game - written bank value
	// [01] spartan x - c6
	// [02] smb - c8
	// [03] antarcitc adv - 8e
	// [04] twinbee - ca
	// [05] battle city - 8f
	// [06] circus charlie - 90
	// [07] galaga - 91
	// [08] yie ar kungfu - 92
	// [09] star force - 93
	// [10] road fighter - 94
	// [11] pinball - 95
	// [12] bomberman - 96
	// [13] new tetris?? - 0
	// [14] arkanoid - cc
	// [15] balloon fight - 97
	// [16] donkey kong - 98
	// [17] donkey kong 3 - 99
	// [18] donkey kong jr - 9a
	// [19] mario bros - 9b
	// [20] popeye - 9c
	m_base_bank = data & 0xdf;
//  printf("offset %X data %X bank %X\n", offset, data, m_base_bank);
}



// Work in progress (probably very wrong)

READ8_MEMBER( sns_rom_banana_device::chip_read )
{
	return BITSWAP8(m_latch[0xf],0,6,7,1,2,3,4,5);
}

WRITE8_MEMBER( sns_rom_banana_device::chip_write )
{
//  printf("write addr %X data %X\n", offset, data);
	m_latch[0xf] = data;
}

READ8_MEMBER( sns_rom_bugs_device::chip_read )
{
	return BITSWAP8(m_latch[offset & 0xff],0,6,7,1,2,3,4,5);
}

WRITE8_MEMBER( sns_rom_bugs_device::chip_write )
{
	m_latch[offset & 0xff] = data;
}
