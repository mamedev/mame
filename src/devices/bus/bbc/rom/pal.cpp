// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro PALPROM carrier boards

  Computer Concepts PALPROM carrier boards (PAL16R4):
    These were the first to provide a 32K ROM banked into a 16K slot using
    a PAL to perform the switching upon reads from pre-programmed zones. In
    addition to being able to provide larger ROM based applications such
    as Inter-Word and Inter-Base, it also served as copy protection since
    the carrier board and PAL would have to be reproduced to support the
    ROM.
    Other publishers such as Beebug and PMS also used the carrier boards
    and PAL provided by Computer Concepts.

  Watford Electronics PALPROM carrier boards (PAL16L8):
    The PALPROM device provides a means of running 32K software within the
    space allocated to a 16K sideways ROM whilst providing a good degree of
    software protection.
    Within a PALPROM, a 32K EPROM is divided into 4 banks of 8K. These are
    arranged in a 3 plus 1 arrangement. Bank 0, which occupies &8000 to
    &9FFF is permanently enabled, whilst banks 1 to 3, which occupy the
    region &A000 to &BFFF, are swapped in one at a time. This swapping is
    made by performing an access to a special switching zone, of which there
    are 8 in total. Accessing a switching zone, which is 32 bytes in length
    and aligned to start on a 32 byte boundary, selects a pre-specified bank
    (1 to 3)

  P.R.E.S. PALPROM carrier boards:
    This was based on the Computer Concepts carrier board.

  Instant Mini Office 2:
    Not a PALPROM carrier board but a larger ROM carrier containing 4x32K
    and TTL circuits to enable and page each ROM into 16K banks.

  Acornsoft Trilogy Emulator (PAL20R4)
    An unreleased product that combines the View family of ROMs into a
    single banked 64K ROM, using a PAL to perform 16K bank switches upon
    reads from the last 4 bytes of ROM space &BFFC to &BFFF. Only known
    example loaned by Stuart Swales.

***************************************************************************/

#include "emu.h"
#include "pal.h"


namespace {

// ======================> bbc_pal_device

class bbc_pal_device : public device_t, public device_bbc_rom_interface
{
protected:
	bbc_pal_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_bbc_rom_interface(mconfig, *this)
		, m_bank(0)
	{
	}

	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// device_bbc_rom_interface overrides
	virtual uint32_t get_rom_size() override { return 0x4000; }

	uint8_t m_bank;
};


// ======================> bbc_cciword_device

class bbc_cciword_device : public bbc_pal_device
{
public:
	bbc_cciword_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_pal_device(mconfig, BBC_CCIWORD, tag, owner, clock)
	{
	}

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		if (!machine().side_effects_disabled())
		{
			// switching zones for Inter-Word
			switch (offset & 0x3fe0)
			{
			case 0x0060:
			case 0x3fc0: m_bank = 0; break;
			case 0x0040:
			case 0x3fa0:
			case 0x3fe0: m_bank = 1; break;
			}
		}

		return get_rom_base()[(offset & 0x3fff) | (m_bank << 14)];
	}
};


// ======================> bbc_ccibase_device

class bbc_ccibase_device : public bbc_pal_device
{
public:
	bbc_ccibase_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_pal_device(mconfig, BBC_CCIBASE, tag, owner, clock)
	{
	}

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		if (!machine().side_effects_disabled())
		{
			// switching zones for Inter-Base
			switch (offset & 0x3fe0)
			{
			case 0x3f80: m_bank = 0; break;
			case 0x3fa0: m_bank = 1; break;
			case 0x3fc0: m_bank = 2; break;
			case 0x3fe0: m_bank = 3; break;
			}
		}

		return get_rom_base()[(offset & 0x3fff) | (m_bank << 14)];
	}
};


// ======================> bbc_ccispell_device

class bbc_ccispell_device : public bbc_pal_device
{
public:
	bbc_ccispell_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_pal_device(mconfig, BBC_CCISPELL, tag, owner, clock)
	{
	}

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		if (!machine().side_effects_disabled())
		{
			// switching zones for SpellMaster
			if (offset == 0x3fe0)
			{
				m_bank = 0;
			}
			else if (m_bank == 0)
			{
				switch (offset & 0x3fe0)
				{
				case 0x3fc0: m_bank = 1; break;
				case 0x3fa0: m_bank = 2; break;
				case 0x3f80: m_bank = 3; break;
				case 0x3f60: m_bank = 4; break;
				case 0x3f40: m_bank = 5; break;
				case 0x3f20: m_bank = 6; break;
				case 0x3f00: m_bank = 7; break;
				}
			}
		}

		return get_rom_base()[(offset & 0x3fff) | (m_bank << 14)];
	}
};


// ======================> bbc_palqst_device

class bbc_palqst_device : public bbc_pal_device
{
public:
	bbc_palqst_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_pal_device(mconfig, BBC_PALQST, tag, owner, clock)
	{
	}

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		if (!machine().side_effects_disabled())
		{
			// switching zones for Quest Paint and ConQuest
			switch (offset & 0x3fe0)
			{
			case 0x0820: m_bank = 2; break;
			case 0x11e0: m_bank = 1; break;
			case 0x12c0: m_bank = 3; break;
			case 0x1340: m_bank = 0; break;
			}
		}

		if (offset & 0x2000)
		{
			return get_rom_base()[(offset & 0x1fff) | (m_bank << 13)];
		}
		else
		{
			return get_rom_base()[offset & 0x1fff];
		}
	}
};


// ======================> bbc_palwap_device

class bbc_palwap_device : public bbc_pal_device
{
public:
	bbc_palwap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_pal_device(mconfig, BBC_PALWAP, tag, owner, clock)
	{
	}

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		if (!machine().side_effects_disabled())
		{
			// switching zones for Wapping Editor
			switch (offset & 0x3fe0)
			{
			case 0x1f00: m_bank = 0; break;
			case 0x1f20: m_bank = 1; break;
			case 0x1f40: m_bank = 2; break;
			case 0x1f60: m_bank = 3; break;
			case 0x1f80: m_bank = 4; break;
			case 0x1fa0: m_bank = 5; break;
			case 0x1fc0: m_bank = 6; break;
			case 0x1fe0: m_bank = 7; break;
			}
		}

		if (offset & 0x2000)
		{
			return get_rom_base()[(offset & 0x1fff) | (m_bank << 13)];
		}
		else
		{
			return get_rom_base()[offset & 0x1fff];
		}
	}
};


// ======================> bbc_palted_device

class bbc_palted_device : public bbc_pal_device
{
public:
	bbc_palted_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_pal_device(mconfig, BBC_PALTED, tag, owner, clock)
	{
	}

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		if (!machine().side_effects_disabled())
		{
			// switching zones for TED
			switch (offset & 0x3fe0)
			{
			case 0x1f80: m_bank = 0; break;
			case 0x1fa0: m_bank = 1; break;
			case 0x1fc0: m_bank = 2; break;
			case 0x1fe0: m_bank = 3; break;
			}
		}

		if (offset & 0x2000)
		{
			return get_rom_base()[(offset & 0x1fff) | (m_bank << 13)];
		}
		else
		{
			return get_rom_base()[offset & 0x1fff];
		}
	}
};


// ======================> bbc_palabep_device

class bbc_palabep_device : public bbc_pal_device
{
public:
	bbc_palabep_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_pal_device(mconfig, BBC_PALABEP, tag, owner, clock)
	{
	}

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		if (!machine().side_effects_disabled())
		{
			// switching zones for Advanced BASIC Editor Plus
			switch (offset & 0x3ffc)
			{
			case 0x3ff8: m_bank = 0; break;
			case 0x3ffc: m_bank = 1; break;
			}
		}

		return get_rom_base()[(offset & 0x3fff) | (m_bank << 14)];
	}
};


// ======================> bbc_palabe_device

class bbc_palabe_device : public bbc_pal_device
{
public:
	bbc_palabe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_pal_device(mconfig, BBC_PALABE, tag, owner, clock)
	{
	}

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		if (!machine().side_effects_disabled())
		{
			// switching zones for Advanced BASIC Editor
			switch (offset & 0x3ffc)
			{
			case 0x3ff8: m_bank = 1; break;
			case 0x3ffc: m_bank = 0; break;
			}
		}

		return get_rom_base()[(offset & 0x3fff) | (m_bank << 14)];
	}
};


// ======================> bbc_palmo2_device

class bbc_palmo2_device : public bbc_pal_device
{
public:
	bbc_palmo2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_pal_device(mconfig, BBC_PALMO2, tag, owner, clock)
	{
	}

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		if (!machine().side_effects_disabled())
		{
			// switching zones for Instant Mini Office 2
			switch (offset & 0x3ff0)
			{
			case 0x2000: m_bank = offset & 0x0f; break;
			}
		}

		return get_rom_base()[(offset & 0x3fff) | (m_bank << 13)];
	}
};


// ======================> bbc_trilogy_device

class bbc_trilogy_device : public bbc_pal_device
{
public:
	bbc_trilogy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_pal_device(mconfig, BBC_TRILOGY, tag, owner, clock)
	{
	}

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		if (!machine().side_effects_disabled())
		{
			// switching zones for Acornsoft Trilogy Emulator
			switch (offset & 0x3ff8)
			{
			case 0x3ff8: m_bank = offset & 0x03; break;
			}
		}

		return get_rom_base()[(offset & 0x3fff) | (m_bank << 14)];
	}
};


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_pal_device::device_start()
{
	save_item(NAME(m_bank));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_CCIWORD, device_bbc_rom_interface, bbc_cciword_device, "bbc_cciword", "Computer Concepts 32K ROM Carrier (Inter-Word)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_CCIBASE, device_bbc_rom_interface, bbc_ccibase_device, "bbc_ccibase", "Computer Concepts 64K ROM Carrier (Inter-Base)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_CCISPELL, device_bbc_rom_interface, bbc_ccispell_device, "bbc_ccispell", "Computer Concepts 128K ROM Carrier (SpellMaster)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_PALQST, device_bbc_rom_interface, bbc_palqst_device, "bbc_palqst", "Watford Electronics ROM Carrier (Quest Paint)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_PALWAP, device_bbc_rom_interface, bbc_palwap_device, "bbc_palwap", "Watford Electronics ROM Carrier (Wapping Editor)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_PALTED, device_bbc_rom_interface, bbc_palted_device, "bbc_palted", "Watford Electronics ROM Carrier (TED)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_PALABEP, device_bbc_rom_interface, bbc_palabep_device, "bbc_palabep", "P.R.E.S. 32K ROM Carrier (ABE+)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_PALABE, device_bbc_rom_interface, bbc_palabe_device, "bbc_palabe", "P.R.E.S. 32K ROM Carrier (ABE)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_PALMO2, device_bbc_rom_interface, bbc_palmo2_device, "bbc_palmo2", "Instant Mini Office 2 ROM Carrier")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_TRILOGY, device_bbc_rom_interface, bbc_trilogy_device, "bbc_trilogy", "Acornsoft Trilogy Emulator")
