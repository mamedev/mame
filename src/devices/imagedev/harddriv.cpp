// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, Miodrag Milanovic
/*********************************************************************

    Code to interface the image code with harddisk core.

    Raphael Nabet 2003

    Update: 23-Feb-2004 - Unlike floppy disks, for which we support
    myriad formats on many systems, it is my intention for MESS to
    standardize on the CHD file format for hard drives so I made a few
    changes to support this

*********************************************************************/

#include "emu.h"
#include "harddriv.h"

#include "emuopts.h"
#include "fileio.h"
#include "harddisk.h"
#include "romload.h"

#include "multibyte.h"
#include "opresolv.h"


OPTION_GUIDE_START(hd_option_guide)
	OPTION_INT('C', "cylinders",        "Cylinders")
	OPTION_INT('H', "heads",            "Heads")
	OPTION_INT('S', "sectors",          "Sectors")
	OPTION_INT('L', "sectorlength",     "Sector Bytes")
	OPTION_INT('K', "hunksize",         "Hunk Bytes")
OPTION_GUIDE_END

static char const *const hd_option_spec =
	"C1-[512]-1024;H1/2/[4]/8;S1-[16]-64;L128/256/[512]/1024;K512/1024/2048/[4096]";


// device type definition
DEFINE_DEVICE_TYPE(HARDDISK, harddisk_image_device, "harddisk_image", "Harddisk")

//-------------------------------------------------
//  harddisk_image_base_device - constructor
//-------------------------------------------------

harddisk_image_base_device::harddisk_image_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  harddisk_image_device - constructor
//-------------------------------------------------

harddisk_image_device::harddisk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: harddisk_image_device(mconfig, HARDDISK, tag, owner, clock)
{
}

//-------------------------------------------------
//  harddisk_image_device - constructor for subclasses
//-------------------------------------------------

harddisk_image_device::harddisk_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: harddisk_image_base_device(mconfig, type, tag, owner, clock)
	, m_chd(nullptr)
	, m_hard_disk_handle()
	, m_device_image_load(*this)
	, m_device_image_unload(*this)
	, m_interface(nullptr)
{
}

//-------------------------------------------------
//  harddisk_image_device - destructor
//-------------------------------------------------

harddisk_image_device::~harddisk_image_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void harddisk_image_device::device_config_complete()
{
	add_format("chd", "CHD Hard drive", "chd,hd,hdv,2mg,hdi", hd_option_spec);
}

const util::option_guide &harddisk_image_device::create_option_guide() const
{
	return hd_option_guide;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void harddisk_image_device::device_start()
{
	m_device_image_load.resolve();
	m_device_image_unload.resolve();

	m_chd = nullptr;

	if (has_preset_images())
		setup_current_preset_image();
	else
		m_hard_disk_handle.reset();
}

void harddisk_image_device::device_stop()
{
	m_hard_disk_handle.reset();
}

std::pair<std::error_condition, std::string> harddisk_image_device::call_load()
{
	std::error_condition our_result = internal_load_hd();

	// Check if there is an image_load callback defined
	if (!our_result && !m_device_image_load.isnull())
	{
		// Let the override do some additional work/checks
		our_result = m_device_image_load(*this);
	}
	return std::make_pair(our_result, std::string());

}

std::pair<std::error_condition, std::string> harddisk_image_device::call_create(int create_format, util::option_resolution *create_args)
{
	if (!create_args)
		throw emu_fatalerror("harddisk_image_device::call_create: Expected create_args to not be nullptr");

	const uint32_t cylinders   = create_args->lookup_int('C');
	const uint32_t heads       = create_args->lookup_int('H');
	const uint32_t sectors     = create_args->lookup_int('S');
	const uint32_t sectorsize  = create_args->lookup_int('L');
	const uint32_t hunksize    = create_args->lookup_int('K');

	const uint32_t totalsectors = cylinders * heads * sectors;

	// create the CHD file
	chd_codec_type compression[4] = { CHD_CODEC_NONE };
	util::core_file::ptr proxy;
	std::error_condition err = util::core_file::open_proxy(image_core_file(), proxy);
	if (!err)
		err = m_origchd.create(std::move(proxy), uint64_t(totalsectors) * uint64_t(sectorsize), hunksize, sectorsize, compression);
	if (err)
		return std::make_pair(err, std::string());

	// if we created the image and hence, have metadata to set, set the metadata
	err = m_origchd.write_metadata(HARD_DISK_METADATA_TAG, 0, string_format(HARD_DISK_METADATA_FORMAT, cylinders, heads, sectors, sectorsize));
	m_origchd.close();

	if (err)
		return std::make_pair(err, std::string());

	return std::make_pair(internal_load_hd(), std::string());
}

void harddisk_image_device::setup_current_preset_image()
{
	chd_file *chd = current_preset_image_chd();
	m_hard_disk_handle.reset(new hard_disk_file(chd));
}

void harddisk_image_device::call_unload()
{
	// Check if there is an image_unload callback defined
	if (!m_device_image_unload.isnull())
		m_device_image_unload(*this);

	m_hard_disk_handle.reset();

	if (m_chd)
	{
		m_origchd.close();
		m_diffchd.close();
	}
	m_chd = nullptr;
}

/*-------------------------------------------------
    open_disk_diff - open a DISK diff file
-------------------------------------------------*/

static std::error_condition open_disk_diff(emu_options &options, const char *name, chd_file &source, chd_file &diff_chd)
{
	std::string fname = std::string(name).append(".dif");

	/* try to open the diff */
	//printf("Opening differencing image file: %s\n", fname.c_str());
	emu_file diff_file(options.diff_directory(), OPEN_FLAG_READ | OPEN_FLAG_WRITE);
	std::error_condition filerr = diff_file.open(fname);
	if (!filerr)
	{
		std::string fullpath(diff_file.fullpath());
		diff_file.close();

		//printf("Opening differencing image file: %s\n", fullpath.c_str());
		return diff_chd.open(fullpath, true, &source);
	}

	/* didn't work; try creating it instead */
	//printf("Creating differencing image: %s\n", fname.c_str());
	diff_file.set_openflags(OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	filerr = diff_file.open(fname);
	if (!filerr)
	{
		std::string fullpath(diff_file.fullpath());
		diff_file.close();

		/* create the CHD */
		//printf("Creating differencing image file: %s\n", fupointllpath.c_str());
		chd_codec_type compression[4] = { CHD_CODEC_NONE };
		std::error_condition err = diff_chd.create(fullpath, source.logical_bytes(), source.hunk_bytes(), compression, source);
		if (err)
			return err;

		return diff_chd.clone_all_metadata(source);
	}

	return std::errc::no_such_file_or_directory;
}

std::error_condition harddisk_image_device::internal_load_hd()
{
	if (has_preset_images())
	{
		setup_current_preset_image();
		return std::error_condition();
	}

	std::error_condition err;
	m_chd = nullptr;
	uint8_t header[64];

	m_hard_disk_handle.reset();

	// open the CHD file
	if (loaded_through_softlist())
	{
		m_chd = machine().rom_load().get_disk_handle(device().subtag("harddriv").c_str());
	}
	else
	{
		fseek(0, SEEK_SET);
		fread(header, 64);
		fseek(0, SEEK_SET);

		if (!memcmp("MComprHD", header, 8))
		{
			util::core_file::ptr proxy;
			err = util::core_file::open_proxy(image_core_file(), proxy);
			if (!err)
				err = m_origchd.open(std::move(proxy), true);

			if (!err)
			{
				m_chd = &m_origchd;
			}
			else if (err == chd_file::error::FILE_NOT_WRITEABLE)
			{
				err = util::core_file::open_proxy(image_core_file(), proxy);
				if (!err)
					err = m_origchd.open(std::move(proxy), false);

				if (!err)
				{
					err = open_disk_diff(device().machine().options(), basename_noext(), m_origchd, m_diffchd);
					if (!err)
					{
						m_chd = &m_diffchd;
					}
				}
			}
		}
	}

	if (m_chd)
	{
		// open the hard disk file
		try
		{
			m_hard_disk_handle.reset(new hard_disk_file(m_chd));
			if (m_hard_disk_handle)
				return std::error_condition();
		}
		catch (...)
		{
			err = image_error::INVALIDIMAGE;
		}
	}
	else if (!is_open())
	{
		err = image_error::UNSPECIFIED;
	}
	else
	{
		uint32_t skip = 0;

		if (!memcmp(header, "2IMG", 4)) // check for 2MG format
		{
			skip = get_u32le(&header[0x18]);
			osd_printf_verbose("harddriv: detected 2MG, creator is %c%c%c%c, data at %08x\n", header[4], header[5], header[6], header[7], skip);
		}
		else if (is_filetype("hdi")) // check for HDI format
		{
			skip = get_u32le(&header[0x8]);
			uint32_t data_size = get_u32le(&header[0xc]);
			if (data_size == length() - skip)
			{
				osd_printf_verbose("harddriv: detected Anex86 HDI, data at %08x\n", skip);
			}
			else
			{
				skip = 0;
			}
		}

		try
		{
			m_hard_disk_handle.reset(new hard_disk_file(image_core_file(), skip));
			if (m_hard_disk_handle)
				return std::error_condition();
		}
		catch (...)
		{
			err = image_error::INVALIDIMAGE;
		}
	}

	/* if we had an error, close out the CHD */
	m_diffchd.close();
	m_origchd.close();
	m_chd = nullptr;

	if (err)
		return err;
	else
		return image_error::UNSPECIFIED;
}

const hard_disk_file::info &harddisk_image_device::get_info() const
{
	return m_hard_disk_handle->get_info();
}

bool harddisk_image_device::read(uint32_t lbasector, void *buffer)
{
	return m_hard_disk_handle->read(lbasector, buffer);
}

bool harddisk_image_device::write(uint32_t lbasector, const void *buffer)
{
	return m_hard_disk_handle->write(lbasector, buffer);
}


bool harddisk_image_device::set_block_size(uint32_t blocksize)
{
	return m_hard_disk_handle->set_block_size(blocksize);
}

std::error_condition harddisk_image_device::get_inquiry_data(std::vector<uint8_t> &data) const
{
	return m_hard_disk_handle->get_inquiry_data(data);
}

std::error_condition harddisk_image_device::get_cis_data(std::vector<uint8_t> &data) const
{
	return m_hard_disk_handle->get_cis_data(data);
}

std::error_condition harddisk_image_device::get_disk_key_data(std::vector<uint8_t> &data) const
{
	return m_hard_disk_handle->get_disk_key_data(data);
}

