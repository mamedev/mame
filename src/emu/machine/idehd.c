#include "idehd.h"

#define PRINTF_IDE_COMMANDS         0

//**************************************************************************
//  IDE DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  ide_device_interface - constructor
//-------------------------------------------------

ide_device_interface::ide_device_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
	master_password(NULL),
	user_password(NULL)
{
}



/*************************************
 *
 *  Compute the LBA address
 *
 *************************************/

UINT32 ide_device_interface::lba_address()
{
	/* LBA direct? */
	if (cur_head_reg & 0x40)
		return cur_sector + cur_cylinder * 256 + cur_head * 16777216;

	/* standard CHS */
	else
		return (cur_cylinder * get_heads() + cur_head) * get_sectors() + cur_sector - 1;
}


/*************************************
 *
 *  Build a features page
 *
 *************************************/

static void swap_strncpy(UINT8 *dst, const char *src, int field_size_in_words)
{
	int i;

	assert(strlen(src) <= (field_size_in_words*2));

	for (i = 0; i < strlen(src); i++)
		dst[i ^ 1] = src[i];
	for ( ; i < field_size_in_words * 2; i++)
		dst[i ^ 1] = ' ';
}


void ide_hdd_device::ide_build_features()
{
	memset(m_features, 0, IDE_DISK_SECTOR_SIZE);
	int total_sectors = m_num_cylinders * m_num_heads * m_num_sectors;

	/* basic geometry */
	m_features[ 0*2+0] = 0x5a;                      /*  0: configuration bits */
	m_features[ 0*2+1] = 0x04;
	m_features[ 1*2+0] = m_num_cylinders & 0xff;    /*  1: logical cylinders */
	m_features[ 1*2+1] = m_num_cylinders >> 8;
	m_features[ 2*2+0] = 0;                         /*  2: reserved */
	m_features[ 2*2+1] = 0;
	m_features[ 3*2+0] = m_num_heads & 0xff;        /*  3: logical heads */
	m_features[ 3*2+1] = 0;/*num_heads >> 8;*/
	m_features[ 4*2+0] = 0;                         /*  4: vendor specific (obsolete) */
	m_features[ 4*2+1] = 0;
	m_features[ 5*2+0] = 0;                         /*  5: vendor specific (obsolete) */
	m_features[ 5*2+1] = 0;
	m_features[ 6*2+0] = m_num_sectors & 0xff;  /*  6: logical sectors per logical track */
	m_features[ 6*2+1] = 0;/*num_sectors >> 8;*/
	m_features[ 7*2+0] = 0;                         /*  7: vendor-specific */
	m_features[ 7*2+1] = 0;
	m_features[ 8*2+0] = 0;                         /*  8: vendor-specific */
	m_features[ 8*2+1] = 0;
	m_features[ 9*2+0] = 0;                         /*  9: vendor-specific */
	m_features[ 9*2+1] = 0;
	swap_strncpy(&m_features[10*2+0],               /* 10-19: serial number */
			"00000000000000000000", 10);
	m_features[20*2+0] = 0;                         /* 20: vendor-specific */
	m_features[20*2+1] = 0;
	m_features[21*2+0] = 0;                         /* 21: vendor-specific */
	m_features[21*2+1] = 0;
	m_features[22*2+0] = 4;                         /* 22: # of vendor-specific bytes on read/write long commands */
	m_features[22*2+1] = 0;
	swap_strncpy(&m_features[23*2+0],               /* 23-26: firmware revision */
			"1.0", 4);
	swap_strncpy(&m_features[27*2+0],               /* 27-46: model number */
			"MAME Compressed Hard Disk", 20);
	m_features[47*2+0] = 0x01;                      /* 47: read/write multiple support */
	m_features[47*2+1] = 0x80;
	m_features[48*2+0] = 0;                         /* 48: reserved */
	m_features[48*2+1] = 0;
	m_features[49*2+0] = 0x03;                      /* 49: capabilities */
	m_features[49*2+1] = 0x0f;
	m_features[50*2+0] = 0;                         /* 50: reserved */
	m_features[50*2+1] = 0;
	m_features[51*2+0] = 2;                         /* 51: PIO data transfer cycle timing mode */
	m_features[51*2+1] = 0;
	m_features[52*2+0] = 2;                         /* 52: single word DMA transfer cycle timing mode */
	m_features[52*2+1] = 0;
	m_features[53*2+0] = 3;                         /* 53: field validity */
	m_features[53*2+1] = 0;
	m_features[54*2+0] = m_num_cylinders & 0xff;    /* 54: number of current logical cylinders */
	m_features[54*2+1] = m_num_cylinders >> 8;
	m_features[55*2+0] = m_num_heads & 0xff;        /* 55: number of current logical heads */
	m_features[55*2+1] = 0;/*num_heads >> 8;*/
	m_features[56*2+0] = m_num_sectors & 0xff;  /* 56: number of current logical sectors per track */
	m_features[56*2+1] = 0;/*num_sectors >> 8;*/
	m_features[57*2+0] = total_sectors & 0xff;  /* 57-58: current capacity in sectors (ATA-1 through ATA-5; obsoleted in ATA-6) */
	m_features[57*2+1] = total_sectors >> 8;
	m_features[58*2+0] = total_sectors >> 16;
	m_features[58*2+1] = total_sectors >> 24;
	m_features[59*2+0] = 0;                         /* 59: multiple sector timing */
	m_features[59*2+1] = 0;
	m_features[60*2+0] = total_sectors & 0xff;      /* 60-61: total user addressable sectors for LBA mode (ATA-1 through ATA-7) */
	m_features[60*2+1] = total_sectors >> 8;
	m_features[61*2+0] = total_sectors >> 16;
	m_features[61*2+1] = total_sectors >> 24;
	m_features[62*2+0] = 0x07;                      /* 62: single word dma transfer */
	m_features[62*2+1] = 0x00;
	m_features[63*2+0] = 0x07;                      /* 63: multiword DMA transfer */
	m_features[63*2+1] = 0x04;
	m_features[64*2+0] = 0x03;                      /* 64: flow control PIO transfer modes supported */
	m_features[64*2+1] = 0x00;
	m_features[65*2+0] = 0x78;                      /* 65: minimum multiword DMA transfer cycle time per word */
	m_features[65*2+1] = 0x00;
	m_features[66*2+0] = 0x78;                      /* 66: mfr's recommended multiword DMA transfer cycle time */
	m_features[66*2+1] = 0x00;
	m_features[67*2+0] = 0x4d;                      /* 67: minimum PIO transfer cycle time without flow control */
	m_features[67*2+1] = 0x01;
	m_features[68*2+0] = 0x78;                      /* 68: minimum PIO transfer cycle time with IORDY */
	m_features[68*2+1] = 0x00;
	m_features[69*2+0] = 0x00;                      /* 69-70: reserved */
	m_features[69*2+1] = 0x00;
	m_features[71*2+0] = 0x00;                      /* 71: reserved for IDENTIFY PACKET command */
	m_features[71*2+1] = 0x00;
	m_features[72*2+0] = 0x00;                      /* 72: reserved for IDENTIFY PACKET command */
	m_features[72*2+1] = 0x00;
	m_features[73*2+0] = 0x00;                      /* 73: reserved for IDENTIFY PACKET command */
	m_features[73*2+1] = 0x00;
	m_features[74*2+0] = 0x00;                      /* 74: reserved for IDENTIFY PACKET command */
	m_features[74*2+1] = 0x00;
	m_features[75*2+0] = 0x00;                      /* 75: queue depth */
	m_features[75*2+1] = 0x00;
	m_features[76*2+0] = 0x00;                      /* 76-79: reserved */
	m_features[76*2+1] = 0x00;
	m_features[80*2+0] = 0x00;                      /* 80: major version number */
	m_features[80*2+1] = 0x00;
	m_features[81*2+0] = 0x00;                      /* 81: minor version number */
	m_features[81*2+1] = 0x00;
	m_features[82*2+0] = 0x00;                      /* 82: command set supported */
	m_features[82*2+1] = 0x00;
	m_features[83*2+0] = 0x00;                      /* 83: command sets supported */
	m_features[83*2+1] = 0x00;
	m_features[84*2+0] = 0x00;                      /* 84: command set/feature supported extension */
	m_features[84*2+1] = 0x00;
	m_features[85*2+0] = 0x00;                      /* 85: command set/feature enabled */
	m_features[85*2+1] = 0x00;
	m_features[86*2+0] = 0x00;                      /* 86: command set/feature enabled */
	m_features[86*2+1] = 0x00;
	m_features[87*2+0] = 0x00;                      /* 87: command set/feature default */
	m_features[87*2+1] = 0x00;
	m_features[88*2+0] = 0x00;                      /* 88: additional DMA modes */
	m_features[88*2+1] = 0x00;
	m_features[89*2+0] = 0x00;                      /* 89: time required for security erase unit completion */
	m_features[89*2+1] = 0x00;
	m_features[90*2+0] = 0x00;                      /* 90: time required for enhanced security erase unit completion */
	m_features[90*2+1] = 0x00;
	m_features[91*2+0] = 0x00;                      /* 91: current advanced power management value */
	m_features[91*2+1] = 0x00;
	m_features[92*2+0] = 0x00;                      /* 92: master password revision code */
	m_features[92*2+1] = 0x00;
	m_features[93*2+0] = 0x00;                      /* 93: hardware reset result */
	m_features[93*2+1] = 0x00;
	m_features[94*2+0] = 0x00;                      /* 94: acoustic management values */
	m_features[94*2+1] = 0x00;
	m_features[95*2+0] = 0x00;                      /* 95-99: reserved */
	m_features[95*2+1] = 0x00;
	m_features[100*2+0] = total_sectors & 0xff;     /* 100-103: maximum 48-bit LBA */
	m_features[100*2+1] = total_sectors >> 8;
	m_features[101*2+0] = total_sectors >> 16;
	m_features[101*2+1] = total_sectors >> 24;
	m_features[102*2+0] = 0x00;
	m_features[102*2+1] = 0x00;
	m_features[103*2+0] = 0x00;
	m_features[103*2+1] = 0x00;
	m_features[104*2+0] = 0x00;                     /* 104-126: reserved */
	m_features[104*2+1] = 0x00;
	m_features[127*2+0] = 0x00;                     /* 127: removable media status notification */
	m_features[127*2+1] = 0x00;
	m_features[128*2+0] = 0x00;                     /* 128: security status */
	m_features[128*2+1] = 0x00;
	m_features[129*2+0] = 0x00;                     /* 129-159: vendor specific */
	m_features[129*2+1] = 0x00;
	m_features[160*2+0] = 0x00;                     /* 160: CFA power mode 1 */
	m_features[160*2+1] = 0x00;
	m_features[161*2+0] = 0x00;                     /* 161-175: reserved for CompactFlash */
	m_features[161*2+1] = 0x00;
	m_features[176*2+0] = 0x00;                     /* 176-205: current media serial number */
	m_features[176*2+1] = 0x00;
	m_features[206*2+0] = 0x00;                     /* 206-254: reserved */
	m_features[206*2+1] = 0x00;
	m_features[255*2+0] = 0x00;                     /* 255: integrity word */
	m_features[255*2+1] = 0x00;
}

//**************************************************************************
//  IDE HARD DISK DEVICE
//**************************************************************************

// device type definition
const device_type IDE_HARDDISK = &device_creator<ide_hdd_device>;

//-------------------------------------------------
//  ide_hdd_device - constructor
//-------------------------------------------------

ide_hdd_device::ide_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, IDE_HARDDISK, "IDE Hard Disk", tag, owner, clock, "hdd", __FILE__),
		ide_device_interface( mconfig, *this )
{
}

ide_hdd_device::ide_hdd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		ide_device_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ide_hdd_device::device_start()
{
//  save_item(NAME(features));

	save_item(NAME(cur_cylinder));
	save_item(NAME(cur_sector));
	save_item(NAME(cur_head));
	save_item(NAME(cur_head_reg));

	save_item(NAME(cur_lba));

	save_item(NAME(buffer));
	save_item(NAME(buffer_offset));

	save_item(NAME(adapter_control));
	save_item(NAME(precomp_offset));
	save_item(NAME(sector_count));

	save_item(NAME(interrupt_pending));
	save_item(NAME(sectors_until_int));

	save_item(NAME(master_password_enable));
	save_item(NAME(user_password_enable));

	save_item(NAME(gnetreadlock));
	save_item(NAME(block_count));

	save_item(NAME(dma_active));
	save_item(NAME(verify_only));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ide_hdd_device::device_reset()
{
	m_handle = subdevice<harddisk_image_device>("harddisk")->get_chd_file();

	if (m_handle)
	{
		m_disk = subdevice<harddisk_image_device>("harddisk")->get_hard_disk_file();
	}
	else
	{
		m_handle = get_disk_handle(machine(), tag());
		m_disk = hard_disk_open(m_handle);
	}

	if (m_disk != NULL)
	{
		const hard_disk_info *hdinfo = hard_disk_get_info(m_disk);
		if (hdinfo->sectorbytes == IDE_DISK_SECTOR_SIZE)
		{
			m_num_cylinders = hdinfo->cylinders;
			m_num_sectors = hdinfo->sectors;
			m_num_heads = hdinfo->heads;
			if (PRINTF_IDE_COMMANDS) mame_printf_debug("CHS: %d %d %d\n", m_num_cylinders, m_num_heads, m_num_sectors);
			mame_printf_debug("CHS: %d %d %d\n", m_num_cylinders, m_num_heads, m_num_sectors);
		}
		// build the features page
		UINT32 metalength;
		if (m_handle->read_metadata (HARD_DISK_IDENT_METADATA_TAG, 0, m_features, IDE_DISK_SECTOR_SIZE, metalength) != CHDERR_NONE)
			ide_build_features();
	}

	buffer_offset = 0;
	gnetreadlock = 0;
	master_password_enable = (master_password != NULL);
	user_password_enable = (user_password != NULL);
}

//-------------------------------------------------
//  read device key
//-------------------------------------------------

void ide_hdd_device::read_key(UINT8 key[])
{
	UINT32 metalength;
	m_handle->read_metadata(HARD_DISK_KEY_METADATA_TAG, 0, key, 5, metalength);
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------
static MACHINE_CONFIG_FRAGMENT( hdd_image )
	MCFG_HARDDISK_ADD( "harddisk" )
MACHINE_CONFIG_END

machine_config_constructor ide_hdd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( hdd_image );
}
