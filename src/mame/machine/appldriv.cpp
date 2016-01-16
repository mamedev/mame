// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    appldriv.c

    Apple 5.25" floppy drive emulation (to be interfaced with applefdc.c)

*********************************************************************/
#include "emu.h"
#include "appldriv.h"
#include "imagedev/flopdrv.h"
#include "formats/ap2_dsk.h"

// our parent's device is the Disk II card (Apple II) or main driver (Mac, IIgs)
// either way, get the drive from there.
#define PARENT_FLOPPY_0 "^floppy0"
#define PARENT_FLOPPY_1 "^floppy1"
#define PARENT_FLOPPY_2 "^floppy2"
#define PARENT_FLOPPY_3 "^floppy3"

static inline apple525_floppy_image_device *get_device(device_t *device)
{
	assert(device != nullptr);
	assert(device->type() == FLOPPY_APPLE);

	return (apple525_floppy_image_device *) downcast<apple525_floppy_image_device *>(device);
}

static int apple525_enable_mask = 1;

legacy_floppy_image_device *apple525_get_subdevice(device_t *device, int drive)
{
	switch(drive) {
		case 0 : return device->subdevice<legacy_floppy_image_device>(PARENT_FLOPPY_0);
		case 1 : return device->subdevice<legacy_floppy_image_device>(PARENT_FLOPPY_1);
		case 2 : return device->subdevice<legacy_floppy_image_device>(PARENT_FLOPPY_2);
		case 3 : return device->subdevice<legacy_floppy_image_device>(PARENT_FLOPPY_3);
	}
	return nullptr;
}

device_t *apple525_get_device_by_type(device_t *device, int ftype, int drive)
{
	int i;
	int cnt = 0;
	for (i=0;i<4;i++) {
		legacy_floppy_image_device *disk = apple525_get_subdevice(device, i);
		if (disk->floppy_get_drive_type()==ftype) {
			if (cnt==drive) {
				return disk;
			}
			cnt++;
		}
	}
	return nullptr;
}

void apple525_set_enable_lines(device_t *device,int enable_mask)
{
	apple525_enable_mask = enable_mask;
}

/* ----------------------------------------------------------------------- */

static void apple525_load_current_track(device_t *image)
{
	int len;
	apple525_floppy_image_device *disk;

	disk = get_device(image);
	len = sizeof(disk->track_data);

	disk->floppy_drive_read_track_data_info_buffer(0, disk->track_data, &len);
	disk->track_loaded = 1;
	disk->track_dirty = 0;
}

static void apple525_save_current_track(device_t *image, int unload)
{
	int len;
	apple525_floppy_image_device *disk;

	disk = get_device(image);

	if (disk->track_dirty)
	{
		len = sizeof(disk->track_data);
		disk->floppy_drive_write_track_data_info_buffer(0, disk->track_data, &len);
		disk->track_dirty = 0;
	}
	if (unload)
		disk->track_loaded = 0;
}

static void apple525_seek_disk(apple525_floppy_image_device *img, signed int step)
{
	int track;
	int pseudo_track;
	apple525_floppy_image_device *disk;

	disk = get_device(img);

	apple525_save_current_track(img, FALSE);

	track = img->floppy_drive_get_current_track();
	pseudo_track = (track * 2) + disk->tween_tracks;

	pseudo_track += step;
	if (pseudo_track < 0)
		pseudo_track = 0;
	else if (pseudo_track/2 >= APPLE2_TRACK_COUNT)
		pseudo_track = APPLE2_TRACK_COUNT*2-1;

	if (pseudo_track/2 != track)
	{
		img->floppy_drive_seek(pseudo_track/2 - img->floppy_drive_get_current_track());
		disk->track_loaded = 0;
	}

	if (pseudo_track & 1)
		disk->tween_tracks = 1;
	else
		disk->tween_tracks = 0;
}

static void apple525_disk_set_lines(device_t *device,device_t *image, UINT8 new_state)
{
	apple525_floppy_image_device *cur_disk;
	UINT8 old_state;
	unsigned int phase;

	cur_disk = get_device(image);

	old_state = cur_disk->state;
	cur_disk->state = new_state;

	if ((new_state & 0x0F) > (old_state & 0x0F))
	{
		phase = 0;
		switch((old_state ^ new_state) & 0x0F)
		{
			case 1: phase = 0; break;
			case 2: phase = 1; break;
			case 4: phase = 2; break;
			case 8: phase = 3; break;
		}

		phase -= cur_disk->floppy_drive_get_current_track() * 2;
		if (cur_disk->tween_tracks)
			phase--;
		phase %= 4;

		switch(phase)
		{
			case 1:
				apple525_seek_disk(cur_disk, +1);
				break;
			case 3:
				apple525_seek_disk(cur_disk, -1);
				break;
		}
	}
}

int apple525_get_count(device_t *device)
{
	int cnt = 0;
	if ((device->subdevice("^" FLOPPY_0)!=nullptr) && (device->subdevice<legacy_floppy_image_device>("^" FLOPPY_0)->floppy_get_drive_type() == FLOPPY_TYPE_APPLE) && (get_device(device->subdevice(PARENT_FLOPPY_0))!=nullptr)) cnt++;
	if ((device->subdevice("^" FLOPPY_1)!=nullptr) && (device->subdevice<legacy_floppy_image_device>("^" FLOPPY_1)->floppy_get_drive_type() == FLOPPY_TYPE_APPLE) && (get_device(device->subdevice(PARENT_FLOPPY_1))!=nullptr)) cnt++;
	if ((device->subdevice("^" FLOPPY_2)!=nullptr) && (device->subdevice<legacy_floppy_image_device>("^" FLOPPY_2)->floppy_get_drive_type() == FLOPPY_TYPE_APPLE) && (get_device(device->subdevice(PARENT_FLOPPY_2))!=nullptr)) cnt++;
	if ((device->subdevice("^" FLOPPY_3)!=nullptr) && (device->subdevice<legacy_floppy_image_device>("^" FLOPPY_3)->floppy_get_drive_type() == FLOPPY_TYPE_APPLE) && (get_device(device->subdevice(PARENT_FLOPPY_3))!=nullptr)) cnt++;

	return cnt;
}

void apple525_set_lines(device_t *device, UINT8 lines)
{
	int i, count;
	device_t *image;

	count = apple525_get_count(device);
	for (i = 0; i < count; i++)
	{
		if (apple525_enable_mask & (1 << i))
		{
			image = apple525_get_device_by_type(device, FLOPPY_TYPE_APPLE, i);
			if (image)
				apple525_disk_set_lines(device,image, lines);
		}
	}
}

/* reads/writes a byte; write_value is -1 for read only */
static UINT8 apple525_process_byte(device_t *img, int write_value)
{
	UINT8 read_value;
	apple525_floppy_image_device *disk;
	int spinfract_divisor;
	int spinfract_dividend;
	apple525_floppy_image_device *config = get_device(img);
	device_image_interface *image = dynamic_cast<device_image_interface *>(img);

	disk = get_device(img);
	spinfract_dividend = config->get_dividend();
	spinfract_divisor = config->get_divisor();

	/* no image initialized for that drive ? */
	if (!image->exists())
		return 0xFF;

	/* check the spin count if reading*/
	if (write_value < 0)
	{
		disk->spin_count++;
		disk->spin_count %= spinfract_divisor;
		if (disk->spin_count >= spinfract_dividend)
			return 0x00;
	}

	/* load track if need be */
	if (disk->track_loaded == 0)
		apple525_load_current_track(img);

	/* perform the read */
	read_value = disk->track_data[disk->position];

	/* perform the write, if applicable */
	if (write_value >= 0)
	{
		disk->track_data[disk->position] = write_value;
		disk->track_dirty = 1;
	}

	disk->position++;
	disk->position %= ARRAY_LENGTH(disk->track_data);

	/* when writing; save the current track after every full sector write */
	if ((write_value >= 0) && ((disk->position % APPLE2_NIBBLE_SIZE) == 0))
		apple525_save_current_track(img, FALSE);

	return read_value;
}

static device_t *apple525_selected_image(device_t *device)
{
	int i,count;

	count = apple525_get_count(device);

	for (i = 0; i < count; i++)
	{
		if (apple525_enable_mask & (1 << i))
			return apple525_get_device_by_type(device, FLOPPY_TYPE_APPLE, i);
	}
	return nullptr;
}

UINT8 apple525_read_data(device_t *device)
{
	device_t *image;
	image = apple525_selected_image(device);
	return image ? apple525_process_byte(image, -1) : 0xFF;
}

void apple525_write_data(device_t *device,UINT8 data)
{
	device_t *image;
	image = apple525_selected_image(device);
	if (image)
		apple525_process_byte(image, data);
}

int apple525_read_status(device_t *device)
{
	int i, count, result = 0;
	device_image_interface *image;

	count = apple525_get_count(device);

	for (i = 0; i < count; i++)
	{
		if (apple525_enable_mask & (1 << i))
		{
			image = dynamic_cast<device_image_interface *>(apple525_get_device_by_type(device, FLOPPY_TYPE_APPLE, i));
			if (image && image->is_readonly())
				result = 1;
		}
	}
	return result;
}

// device type definition
const device_type FLOPPY_APPLE = &device_creator<apple525_floppy_image_device>;

//-------------------------------------------------
//  apple525_floppy_image_device - constructor
//-------------------------------------------------

apple525_floppy_image_device::apple525_floppy_image_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: legacy_floppy_image_device(mconfig, FLOPPY_APPLE, "Apple Disk II", tag, owner, clock, "floppy_apple", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apple525_floppy_image_device::device_start()
{
	legacy_floppy_image_device::device_start();
	floppy_set_type(FLOPPY_TYPE_APPLE);

	state = 0;
	tween_tracks = 0;
	track_loaded = 0;
	track_dirty = 0;
	position = 0;
	spin_count = 0;
	memset(track_data, 0, sizeof(track_data));
}

bool apple525_floppy_image_device::call_load()
{
	int result = legacy_floppy_image_device::call_load();
	floppy_drive_seek(-999);
	floppy_drive_seek(+35/2);
	return result;
}

void apple525_floppy_image_device::call_unload()
{
	apple525_save_current_track(this, TRUE);

	legacy_floppy_image_device::call_unload();
}
