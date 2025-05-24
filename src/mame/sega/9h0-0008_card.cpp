// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/******************************************************************************

    9h0-0008_card.cpp

    Sega Toys 9H0-0008 barcode card loader

    TODO:
    * Decode barcode from the scan so it doesn't need to be in a software
      part feature and loose files can be supported.

*******************************************************************************/

#include "emu.h"

#include "9h0-0008_card.h"

#include "rendutil.h"
#include "softlist_dev.h"

#include "ioprocs.h"

#include <locale>
#include <sstream>


DEFINE_DEVICE_TYPE(SEGA_9H0_0008_CARD, sega_9h0_0008_card_device, "sega_9h0_0008_card", "Sega Toys 9H0-0008 barcode card")


sega_9h0_0008_card_device::sega_9h0_0008_card_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock)
	: sega_9h0_0008_card_device(mconfig, SEGA_9H0_0008_CARD, tag, owner, clock)
{
}


sega_9h0_0008_card_device::sega_9h0_0008_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: sega_9h0_0008_iox_slot_device(mconfig, type, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, m_barcode(0)
{
}


sega_9h0_0008_card_device::~sega_9h0_0008_card_device()
{
}


std::pair<std::error_condition, std::string> sega_9h0_0008_card_device::call_load()
{
	memory_region *cardrgn;
	if (loaded_through_softlist())
	{
		// ensure card region is present
		cardrgn = memregion("card");
		if (!cardrgn)
		{
			return std::make_pair(
					image_error::BADSOFTWARE,
					"Software list item has no 'card' data area");
		}

		// try to parse the barcode number
		char const *const barcodestr = get_feature("barcode");
		if (!barcodestr)
		{
			machine().memory().region_free(cardrgn->name());
			return std::make_pair(
					image_error::BADSOFTWARE,
					"Software list item has no 'barcode' feature");
		}
		std::istringstream stream;
		stream.imbue(std::locale::classic());
		if (('0' == barcodestr[0]) && (('x' == barcodestr[1]) || ('X' == barcodestr[1])))
		{
			stream.str(barcodestr + 2);
			stream >> std::hex;
		}
		else
		{
			stream.str(barcodestr);
		}
		stream >> m_barcode;
		if (!stream)
		{
			machine().memory().region_free(cardrgn->name());
			m_barcode = 0;
			return std::make_pair(
					image_error::BADSOFTWARE,
					util::string_format("Software list item has invalid 'barcode' feature '%s'", barcodestr));
		}
	}
	else
	{
		// attempt to read the file into memory
		auto const len = length();
		cardrgn = machine().memory().region_alloc(subtag("card"), len, 1, ENDIANNESS_LITTLE);
		if (!cardrgn)
			return std::make_pair(std::errc::not_enough_memory, std::string());
		if (fread(cardrgn->base(), len) != len)
		{
			machine().memory().region_free(cardrgn->name());
			return std::make_pair(
					std::errc::io_error,
					"Error reading card scan file");
		}

		// TODO: support reading barcode from image
		m_barcode = 0;
		osd_printf_warning(
				"%s: Sega Toys 9H0-0008 barcodes are only supported for software list items.\n",
				tag());
	}

	// sanity check that the card scan looks like a supported bitmap
	auto io = util::ram_read(cardrgn->base(), cardrgn->bytes());
	if (!io)
	{
		machine().memory().region_free(cardrgn->name());
		m_barcode = 0;
		return std::make_pair(std::errc::not_enough_memory, std::string());
	}
	switch (render_detect_image(*io))
	{
	case RENDUTIL_IMGFORMAT_PNG:
	case RENDUTIL_IMGFORMAT_JPEG:
	case RENDUTIL_IMGFORMAT_MSDIB:
		break;
	default:
		machine().memory().region_free(cardrgn->name());
		m_barcode = 0;
		return std::make_pair(
				image_error::INVALIDIMAGE,
				"Card scan does not appear to be a PNG, JPEG or Microsoft DIB (BMP) bitmap");
	}
	io.reset();

	return std::make_pair(std::error_condition(), std::string());
}


void sega_9h0_0008_card_device::call_unload()
{
	memory_region *const cardrgn = memregion("card");
	if (cardrgn)
		machine().memory().region_free(cardrgn->name());

	m_barcode = 0;
}


void sega_9h0_0008_card_device::device_start()
{
}


software_list_loader const &sega_9h0_0008_card_device::get_software_list_loader() const
{
	return rom_software_list_loader::instance();
}
