// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
// thanks-to:Derek Mathieson (original author of Genie)
/***************************************************************************

    PMS Genie ROM Board

***************************************************************************/

#include "emu.h"
#include "genie.h"


namespace {

class bbc_pmsgenie_device
	: public device_t
	, public device_bbc_rom_interface
	, public device_nvram_interface
{
public:
	bbc_pmsgenie_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_PMSGENIE, tag, owner, clock)
		, device_bbc_rom_interface(mconfig, *this)
		, device_nvram_interface(mconfig, *this)
		, m_nvram(*this, "nvram", 0x8000, ENDIANNESS_LITTLE)
		, m_ram(*this, "ram", 0x8000, ENDIANNESS_LITTLE)
		, m_write_latch(0)
		, m_bank_latch(0)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
	virtual uint32_t get_rom_size() override { return 0x4000; }

private:
	memory_share_creator<uint8_t> m_nvram;
	memory_share_creator<uint8_t> m_ram;

	uint8_t m_write_latch;
	uint8_t m_bank_latch;
};


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_pmsgenie_device::device_start()
{
	save_item(NAME(m_write_latch));
	save_item(NAME(m_bank_latch));
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void bbc_pmsgenie_device::nvram_default()
{
	// initial data from Genie formatter
	static const uint8_t format[256] =
	{
		0x37, 0x1b, 0x36, 0x1e, 0x1e, 0x35, 0x12, 0x10, 0x36, 0x34, 0x00, 0x00, 0x03, 0x01, 0x01, 0x01,
		0xff, 0x01, 0x01, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xf7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x44, 0x65, 0x72, 0x65, 0x6b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	// clear the Genie
	memset(&m_nvram[0], 0x00, sizeof(m_nvram));

	// set password to "GENIE", and initialise structures
	memcpy(&m_nvram[0], format, sizeof(format));
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  specified file
//-------------------------------------------------

bool bbc_pmsgenie_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = util::read(file, m_nvram, m_nvram.bytes());
	return !err && (actual == m_nvram.bytes());
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  specified file
//-------------------------------------------------

bool bbc_pmsgenie_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = util::write(file, m_nvram, m_nvram.bytes());
	return !err;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t bbc_pmsgenie_device::read(offs_t offset)
{
	uint8_t data = 0xff;

	if (!machine().side_effects_disabled())
	{
		switch (offset >> 8)
		{
		case 0x1e:
			// &9E00-&9EFF - Write value latch
			m_write_latch = offset & 0xff;
			break;

		case 0x1f:
			// &9F00-&9FFF - Bank select latch
			// Bit
			// 0-2: RAM bank select
			//   3: Not used
			// 4-5: ROM bank select
			//   6: Not used
			//   7: Read / NOT Write for internal RAM
			m_bank_latch = offset & 0xff;
			break;
		}
	}

	switch (offset & 0x2000)
	{
	case 0x0000:
		// &8000-&9FFF - 4 Pages of 8K ROM (32K in total)
		data = get_rom_base()[(offset & 0x1fff) | (m_bank_latch & 0x30) << 9];
		break;

	case 0x2000:
		// &A000-&BFFF - 8 Pages of 8K RAM (64K in total)
		if (m_bank_latch & 0x80)
		{
			// RAM read
			if (m_bank_latch & 0x04)
				data = m_nvram[(offset & 0x1fff) | (m_bank_latch & 0x03) << 13];
			else
				data = m_ram[(offset & 0x1fff) | (m_bank_latch & 0x03) << 13];
		}
		else
		{
			// RAM write
			if (m_bank_latch & 0x04)
				m_nvram[(offset & 0x1fff) | (m_bank_latch & 0x03) << 13] = m_write_latch;
			else
				m_ram[(offset & 0x1fff) | (m_bank_latch & 0x03) << 13] = m_write_latch;

			data = m_write_latch;
		}
		break;
	}

	return data;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_PMSGENIE, device_bbc_rom_interface, bbc_pmsgenie_device, "bbc_pmsgenie", "PMS Genie ROM Board")
