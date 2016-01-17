// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/****************************************************************************

    psion_pack.c

    Psion Organiser II Datapack emulation


    Datapack pinout from Psion's documentation
              __  __
    SD1    1 |  \/  | 2  SD0
    SD3    3 |      | 4  SD2
    SD5    5 |      | 6  SD4
    SD7    7 |      | 8  SD6
    SMR    9 |      | 10 SCLK
    SOE_B 11 |      | 12 SS_B
    GND   13 |      | 14 SPGM_B
    SVCC  15 |______| 16 SVPP

****************************************************************************/

#include "emu.h"
#include "psion_pack.h"

// Datapack control lines
#define DP_LINE_CLOCK           0x01
#define DP_LINE_RESET           0x02
#define DP_LINE_PROGRAM         0x04
#define DP_LINE_OUTPUT_ENABLE   0x08
#define DP_LINE_SLOT_SELECT     0x10

// Datapack ID
#define DP_ID_EPROM             0x02
#define DP_ID_PAGED             0x04
#define DP_ID_WRITE             0x08
#define DP_ID_BOOT              0x10
#define DP_ID_COPY              0x20

#define OPK_HEAD_SIZE           6


static OPTION_GUIDE_START( datapack_option_guide )
	OPTION_INT('S', "size", "Datapack size" )
	OPTION_INT('R', "ram", "RAM/EPROM" )
	OPTION_INT('P', "paged", "Paged" )
	OPTION_INT('W', "protect", "Write-protected" )
	OPTION_INT('B', "boot", "Bootable" )
	OPTION_INT('C', "copy", "Copyable" )
OPTION_GUIDE_END

static const char *datapack_option_spec =
	"S1/2/4/[8]/16;R0/[1];P[0]/1;W[0]/1;B[0]/1;C0/[1]";


// device type definition
const device_type PSION_DATAPACK = &device_creator<datapack_device>;

//-------------------------------------------------
//  datapack_device - constructor
//-------------------------------------------------

datapack_device::datapack_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PSION_DATAPACK, "Psion Datapack", tag, owner, clock, "datapack", __FILE__),
		device_image_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  datapack_device - destructor
//-------------------------------------------------

datapack_device::~datapack_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void datapack_device::device_start()
{
	call_unload();
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void datapack_device::device_config_complete()
{
	m_formatlist.append(*global_alloc(image_device_format("opk", "Psion Datapack image", "opk", datapack_option_spec)));

	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  option_guide for create new image
//-------------------------------------------------

const option_guide *datapack_device::create_option_guide() const
{
	return datapack_option_guide;
}


/*-------------------------------------------------
    update the internal state
-------------------------------------------------*/

void datapack_device::update()
{
	UINT32 pack_addr = m_counter + ((m_id & DP_ID_PAGED) ? (m_page << 8) : 0);

	// if the datapack is 128k or more is treated as segmented
	if (m_size >= 0x10)
		pack_addr += (m_segment << 14);

	if (pack_addr < (m_size << 13))
	{
		if ((m_control & DP_LINE_OUTPUT_ENABLE) && !(m_control & DP_LINE_RESET))
		{
			// write data
			if (software_entry() == nullptr && (m_id & DP_ID_WRITE))
			{
				fseek(pack_addr + OPK_HEAD_SIZE, SEEK_SET);
				fwrite(&m_data, 1);
			}
		}
		else if ((m_control & DP_LINE_OUTPUT_ENABLE) && (m_control & DP_LINE_RESET))
		{
			// write datapack segment
			if (m_size <= 0x10)
				m_segment = m_data & 0x07;
			else if (m_size <= 0x20)
				m_segment = m_data & 0x0f;
			else if (m_size <= 0x40)
				m_segment = m_data & 0x1f;
			else if (m_size <= 0x80)
				m_segment = m_data & 0x3f;
			else
				m_segment = m_data;
		}
		else if (!(m_control & DP_LINE_OUTPUT_ENABLE) && !(m_control & DP_LINE_RESET))
		{
			// read data
			if ((pack_addr + OPK_HEAD_SIZE) < length())
			{
				fseek(pack_addr + OPK_HEAD_SIZE, SEEK_SET);
				fread(&m_data, 1);
			}
			else
			{
				m_data = 0xff;
			}
		}
		else if (!(m_control & DP_LINE_OUTPUT_ENABLE) && (m_control & DP_LINE_RESET))
		{
			// read datapack ID
			if ((m_id & DP_ID_EPROM) || software_entry() != nullptr)
				m_data = m_id;
			else
				m_data = 0x01;      // for identify RAM pack
		}
	}
}


/*-------------------------------------------------
    read datapack data lines
-------------------------------------------------*/

UINT8 datapack_device::data_r()
{
	return (!(m_control & DP_LINE_SLOT_SELECT)) ? m_data : 0;
}


/*-------------------------------------------------
    write datapack data lines
-------------------------------------------------*/

void  datapack_device::data_w(UINT8 data)
{
	if (is_loaded())
	{
		m_data = data;

		// updates the internal state
		if (!(data & DP_LINE_SLOT_SELECT))
			update();
	}
}


/*-------------------------------------------------
    read datapack control lines
-------------------------------------------------*/

UINT8 datapack_device::control_r()
{
	return m_control;
}


/*-------------------------------------------------
    write datapack control lines
-------------------------------------------------*/

void datapack_device::control_w(UINT8 data)
{
	if (is_loaded())
	{
		if ((m_control & DP_LINE_CLOCK) != (data & DP_LINE_CLOCK))
		{
			// increments the counter
			m_counter++;

			if (m_counter >= ((m_id & DP_ID_PAGED) ? 0x100 : (m_size << 13)))
				m_counter = 0;
		}

		if ((m_control & DP_LINE_PROGRAM) && !(data & DP_LINE_PROGRAM))
		{
			// increments the page
			m_page++;

			if (m_page >= (m_size << 5))
				m_page = 0;
		}

		if (data & DP_LINE_RESET)
		{
			// reset counter and page
			m_counter = 0;
			m_page = 0;
		}

		m_control = data;

		// updates the internal state
		if (!(data & DP_LINE_SLOT_SELECT))
			update();
	}
}


/*-------------------------------------------------
    DEVICE_IMAGE_LOAD( datapack )
-------------------------------------------------*/

bool datapack_device::call_load()
{
	UINT8 data[0x10];

	fread(data, 0x10);

	// check the OPK head
	if(strncmp((const char*)data, "OPK", 3))
		return IMAGE_INIT_FAIL;

	// get datapack ID and size
	m_id   = data[OPK_HEAD_SIZE + 0];
	m_size = data[OPK_HEAD_SIZE + 1];

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
    DEVICE_IMAGE_CREATE( datapack )
-------------------------------------------------*/

bool datapack_device::call_create(int format_type, option_resolution *create_args)
{
	static const UINT8 opk_head[6] = {'O', 'P', 'K', 0x00, 0x00, 0x00};

	if (create_args != nullptr)
	{
		m_id = 0x40;
		m_id |= (option_resolution_lookup_int(create_args, 'R')) ? 0x00 : 0x02;
		m_id |= (option_resolution_lookup_int(create_args, 'P')) ? 0x04 : 0x00;
		m_id |= (option_resolution_lookup_int(create_args, 'W')) ? 0x00 : 0x08;
		m_id |= (option_resolution_lookup_int(create_args, 'B')) ? 0x00 : 0x10;
		m_id |= (option_resolution_lookup_int(create_args, 'C')) ? 0x20 : 0x00;
		m_size = option_resolution_lookup_int(create_args, 'S');
	}
	else
	{
		// 64k RAM datapack by default
		m_id = 0x7c;
		m_size = 0x08;
	}

	fwrite(opk_head, 6);
	fwrite(&m_id, 1);
	fwrite(&m_size, 1);

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
    DEVICE_IMAGE_UNLOAD( datapack )
-------------------------------------------------*/

void datapack_device::call_unload()
{
	m_id      = 0;
	m_size    = 0;
	m_counter = 0;
	m_page    = 0;
	m_segment = 0;
	m_control = 0;
	m_data    = 0;
}
