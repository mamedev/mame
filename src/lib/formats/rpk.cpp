// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    rpk.cpp

    RPK format support

    A RPK file ("rompack") contains a collection of dump files and a layout
    file that defines the kind of circuit board (PCB) used in the cartridge
    and the mapping of dumps to sockets on the board.

Example:
    <?xml version="1.0" encoding="utf-8"?>
    <romset>
        <resources>
            <rom id="gromimage" file="ed-assmg.bin" />
        </resources>
        <configuration>
            <pcb type="standard">
                <socket id="grom_socket" uses="gromimage"/>
            </pcb>
        </configuration>
    </romset>

DTD:
    <!ELEMENT romset (resources, configuration)>
    <!ELEMENT resources (rom|ram)+>
    <!ELEMENT rom EMPTY>
    <!ELEMENT ram EMPTY>
    <!ELEMENT configuration (pcb)>
    <!ELEMENT pcb (socket)+>
    <!ELEMENT socket EMPTY>
    <!ATTLIST romset version CDATA #IMPLIED>
    <!ATTLIST rom id ID #REQUIRED
    <!ATTLIST rom file CDATA #REQUIRED>
    <!ATTLIST rom crc CDATA #IMPLIED>
    <!ATTLIST rom sha1 CDATA #IMPLIED>
    <!ATTLIST ram id ID #REQUIRED>
    <!ATTLIST ram type (volatile|persistent) #IMPLIED>
    <!ATTLIST ram store (internal|external) #IMPLIED>
    <!ATTLIST ram file CDATA #IMPLIED>
    <!ATTLIST ram length CDATA #REQUIRED>
    <!ATTLIST pcb type CDATA #REQUIRED>
    <!ATTLIST socket id ID #REQUIRED>
    <!ATTLIST socket uses IDREF #REQUIRED>

***************************************************************************/

#include "rpk.h"
#include "xmlfile.h"


namespace
{

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class rpk_category_impl : public std::error_category
{
public:
	virtual char const *name() const noexcept override { return "rpk"; }
	virtual std::string message(int condition) const override;
};


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

rpk_category_impl const f_rpk_category_instance;
};


/***************************************************************************
    RPK READER
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

rpk_reader::rpk_reader(char const *const *pcb_types, bool supports_ram)
	: m_pcb_types(pcb_types)
	, m_supports_ram(supports_ram)
{
}


//-------------------------------------------------
//  read
//-------------------------------------------------

std::error_condition rpk_reader::read(std::unique_ptr<util::random_read> &&stream, rpk_file::ptr &result) const
{
	// open the RPK (as a zip file)
	util::archive_file::ptr zipfile;
	std::error_condition ziperr = util::archive_file::open_zip(std::move(stream), zipfile);
	if (ziperr)
		return ziperr;

	// open the layout XML
	if (zipfile->search("layout.xml", false) < 0)
		return error::MISSING_LAYOUT;

	// determine the uncompressed length
	uint64_t uncompressed_length_uint64 = zipfile->current_uncompressed_length();
	size_t uncompressed_length = (size_t)uncompressed_length_uint64;
	if (uncompressed_length != uncompressed_length_uint64)
		return std::errc::not_enough_memory;

	// prepare a buffer for the layout XML
	std::unique_ptr<char[]> layout_xml_text(new (std::nothrow) char[uncompressed_length + 1]);
	if (!layout_xml_text)
		return std::errc::not_enough_memory;

	// and decompress it
	ziperr = zipfile->decompress(&layout_xml_text[0], uncompressed_length);
	if (ziperr)
		return ziperr;
	layout_xml_text[uncompressed_length] = 0;

	// parse the layout text
	util::xml::file::ptr const layout_xml = util::xml::file::string_read(&layout_xml_text[0], nullptr);
	if (!layout_xml)
		return error::XML_ERROR;
	layout_xml_text.reset();

	// now we work within the XML tree

	// romset is the root node
	util::xml::data_node const *const romset_node = layout_xml->get_child("romset");
	if (!romset_node)
		return error::INVALID_LAYOUT; // document element must be <romset>

	// resources is a child of romset
	util::xml::data_node const *const resources_node = romset_node->get_child("resources");
	if (!resources_node)
		return error::INVALID_LAYOUT; // <romset> must have a <resources> child

	// configuration is a child of romset; we're actually interested in ...
	util::xml::data_node const *const configuration_node = romset_node->get_child("configuration");
	if (!configuration_node)
		return error::INVALID_LAYOUT; // <romset> must have a <configuration> child

	// ... pcb, which is a child of configuration
	util::xml::data_node const *const pcb_node = configuration_node->get_child("pcb");
	if (!pcb_node)
		return error::INVALID_LAYOUT; // <configuration> must have a <pcb> child

	// we'll try to find the PCB type on the provided type list.
	std::string const *const pcb_type_string = pcb_node->get_attribute_string_ptr("type");
	if (!pcb_type_string)
		return error::INVALID_LAYOUT; // <pcb> must have a 'type' attribute";
	int pcb_type = 0;
	while (m_pcb_types[pcb_type] && strcmp(m_pcb_types[pcb_type], pcb_type_string->c_str()))
		pcb_type++;
	if (!m_pcb_types[pcb_type])
		return error::UNKNOWN_PCB_TYPE;

	// create the rpk_file object
	rpk_file::ptr file = std::make_unique<rpk_file>(std::move(zipfile), pcb_type);

	// find the sockets and load their respective resource
	for (util::xml::data_node const *socket_node = pcb_node->get_first_child(); socket_node; socket_node = socket_node->get_next_sibling())
	{
		if (strcmp(socket_node->get_name(), "socket") != 0)
			return error::INVALID_LAYOUT; // <pcb> element has only <socket> children

		std::string const *const id = socket_node->get_attribute_string_ptr("id");
		if (!id)
			return error::INVALID_LAYOUT; // <socket> must have an 'id' attribute

		std::string const *const uses_name = socket_node->get_attribute_string_ptr("uses");
		if (!uses_name)
			return error::INVALID_LAYOUT; // <socket> must have a 'uses' attribute"

		// locate the resource node
		util::xml::data_node const *resource_node = nullptr;
		for (util::xml::data_node const *this_resource_node = resources_node->get_first_child(); this_resource_node; this_resource_node = this_resource_node->get_next_sibling())
		{
			std::string const *const resource_name = this_resource_node->get_attribute_string_ptr("id");
			if (!resource_name)
				return error::INVALID_LAYOUT; // resource node must have an 'id' attribute

			if (*resource_name == *uses_name)
			{
				resource_node = this_resource_node;
				break;
			}
		}
		if (!resource_node)
			return error::INVALID_RESOURCE_REF; // *uses_name

		// process the resource
		if (!strcmp(resource_node->get_name(), "rom"))
		{
			std::error_condition err = file->add_rom_socket(std::string(*id), *resource_node);
			if (err)
				return err;
		}
		else if (!strcmp(resource_node->get_name(), "ram"))
		{
			if (!m_supports_ram)
				return error::UNSUPPORTED_RPK_FEATURE; // <ram> is not supported by this system
			std::error_condition err = file->add_ram_socket(std::string(*id), *resource_node);
			if (err)
				return err;
		}
		else
			return error::INVALID_LAYOUT; // resource node must be <rom> or <ram>
	}

	// and we're done!
	result = std::move(file);
	return std::error_condition();
}


/***************************************************************************
    RPK FILE
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

rpk_file::rpk_file(util::archive_file::ptr &&zipfile, int pcb_type)
	: m_zipfile(std::move(zipfile))
	, m_pcb_type(pcb_type)
{
}



//-------------------------------------------------
//  dtor
//-------------------------------------------------

rpk_file::~rpk_file()
{
}


//-------------------------------------------------
//  add_rom_socket
//-------------------------------------------------

std::error_condition rpk_file::add_rom_socket(std::string &&id, const util::xml::data_node &rom_resource_node)
{
	// find the file attribute (required)
	std::string const *const file = rom_resource_node.get_attribute_string_ptr("file");
	if (!file)
		return rpk_reader::error::INVALID_LAYOUT; // <rom> must have a 'file' attribute

	// check for crc (optional)
	std::optional<util::hash_collection> hashes;
	std::string const *const crcstr = rom_resource_node.get_attribute_string_ptr("crc");
	if (crcstr)
	{
		if (!hashes)
			hashes.emplace();
		hashes->add_from_string(util::hash_collection::HASH_CRC, *crcstr);
	}

	// check for sha1 (optional)
	std::string const *const sha1 = rom_resource_node.get_attribute_string_ptr("sha1");
	if (sha1)
	{
		if (!hashes.has_value())
			hashes.emplace();
		hashes->add_from_string(util::hash_collection::HASH_SHA1, *sha1);
	}

	// finally add the socket
	m_sockets.emplace_back(*this, std::move(id), rpk_socket::socket_type::ROM, std::string(*file), std::move(hashes));
	return std::error_condition();
}


//-------------------------------------------------
//  add_ram_socket
//-------------------------------------------------

std::error_condition rpk_file::add_ram_socket(std::string &&id, const util::xml::data_node &ram_resource_node)
{
	// find the length attribute
	std::string const *const length_string = ram_resource_node.get_attribute_string_ptr("length");
	if (!length_string)
		return rpk_reader::error::MISSING_RAM_LENGTH;

	// parse it
	unsigned int length;
	char suffix;
	switch (sscanf(length_string->c_str(), "%u%c", &length, &suffix))
	{
	case 1:
		// fall through
		break;

	case 2:
		switch (tolower(suffix))
		{
		case 'k':
			// kilobytes
			length *= 1024;
			break;

		case 'm':
			// megabytes
			length *= 1024 * 1024;
			break;

		default:  // failed
			return rpk_reader::error::INVALID_RAM_SPEC;
		}
		break;

	default:
		return rpk_reader::error::INVALID_RAM_SPEC;
	}

	// determine the type of RAM
	rpk_socket::socket_type type;
	std::string const *const ram_type = ram_resource_node.get_attribute_string_ptr("type");
	if (ram_type && *ram_type == "persistent")
		type = rpk_socket::socket_type::PERSISTENT_RAM;
	else
		type = rpk_socket::socket_type::RAM;

	// persistent RAM needs a file name
	std::string file;
	if (type == rpk_socket::socket_type::PERSISTENT_RAM)
	{
		std::string const *const ram_filename = ram_resource_node.get_attribute_string_ptr("file");
		if (ram_filename == nullptr)
			return rpk_reader::error::INVALID_RAM_SPEC; // <ram type='persistent'> must have a 'file' attribute
		file = *ram_filename;
	}

	// finally add the socket
	m_sockets.emplace_back(*this, std::move(id), type, std::move(file), std::nullopt, length);
	return std::error_condition();
}


/***************************************************************************
    RPK SOCKET
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

rpk_socket::rpk_socket(rpk_file &rpk, std::string &&id, socket_type type, std::string &&filename, std::optional<util::hash_collection> &&hashes, std::uint32_t length)
	: m_rpk(rpk)
	, m_id(id)
	, m_type(type)
	, m_filename(filename)
	, m_hashes(std::move(hashes))
	, m_length(length)
{
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

rpk_socket::~rpk_socket()
{
}


//-------------------------------------------------
//  read_file
//-------------------------------------------------

std::error_condition rpk_socket::read_file(std::vector<std::uint8_t> &result) const
{
	// find the file
	if (m_rpk.zipfile().search(m_filename, false) < 0)
		return rpk_reader::error::INVALID_FILE_REF;

	// prepare a buffer
	result.clear();
	try
	{
		result.resize(m_rpk.zipfile().current_uncompressed_length());
	}
	catch (std::bad_alloc const &)
	{
		return std::errc::not_enough_memory;
	}

	// read the file
	std::error_condition const ziperr = m_rpk.zipfile().decompress(&result[0], m_rpk.zipfile().current_uncompressed_length());
	if (ziperr)
		return ziperr;

	// perform hash checks, if appropriate
	if (m_hashes.has_value())
	{
		util::hash_collection actual_hashes;
		actual_hashes.compute(&result[0], result.size(), m_hashes->hash_types().c_str());

		if (actual_hashes != m_hashes)
			return rpk_reader::error::INVALID_FILE_REF; // Hash check failed
	}

	// success!
	return std::error_condition();
}


/***************************************************************************
    RPK EXCEPTION HANDLING
***************************************************************************/

//-------------------------------------------------
//  rpk_category - gets the RPK error category instance
//-------------------------------------------------

std::error_category const &rpk_category() noexcept
{
	return f_rpk_category_instance;
}

//-------------------------------------------------
//  error_message
//-------------------------------------------------

std::string rpk_category_impl::message(int condition) const
{
	using namespace std::literals;

	std::string result;
	switch ( condition)
	{
	case 0:
		result = "No error"s;
		break;
	case (int)rpk_reader::error::XML_ERROR:
		result = "XML format error"s;
		break;
	case (int)rpk_reader::error::MISSING_RAM_LENGTH:
		result = "Missing RAM length"s;
		break;
	case (int)rpk_reader::error::INVALID_RAM_SPEC:
		result = "Invalid RAM specification"s;
		break;
	case (int)rpk_reader::error::INVALID_RESOURCE_REF:
		result = "Invalid resource reference"s;
		break;
	case (int)rpk_reader::error::INVALID_LAYOUT:
		result = "layout.xml not valid"s;
		break;
	case (int)rpk_reader::error::MISSING_LAYOUT:
		result = "Missing layout"s;
		break;
	case (int)rpk_reader::error::UNKNOWN_PCB_TYPE:
		result = "Unknown pcb type"s;
		break;
	case (int)rpk_reader::error::UNSUPPORTED_RPK_FEATURE:
		result = "RPK feature not supported"s;
		break;
	default:
		result = "Unknown error"s;
		break;
	}
	return result;
}
