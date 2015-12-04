// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont
/*********************************************************************

    sonydriv.c

    Apple/Sony 3.5" floppy drive emulation (to be interfaced with iwm.c)

    Nate Woods, Raphael Nabet, R. Belmont

    This floppy drive was present in all variants of Lisa 2 (including Mac XL),
    all Apple IIgs and IIc Plus machines, and in all Macintoshes in production
    before 1988, when SWIM and SuperDrive were introduced.

    There were three major variants :
    - A single-sided 400k unit which was used on Lisa 2/Mac XL, and Macintosh
      128k/512k.  This unit needs the computer to send the proper pulses to
      control the drive motor rotation.  It can be connected to all early
      Macintosh (but not Mac Classic?) as an external unit.
    - A double-sided 800k unit which was used on Macintosh Plus, 512ke, and
      early SE and II*.  This unit generates its own drive motor rotation
      control signals.  It can be connected to earlier (and later) Macintosh as
      an external or internal unit.  Some Lisa2/10 and Mac XL were upgraded to
      use it, too, but a fdc ROM upgrade was required.
    - A double-sided 1440k unit.  This is fully back compatible with the 800k
      drive, and adds 1440k MFM capability.  This drive, called FDHD or
      SuperDrive by Apple, came in automatic and manual-inject versions.

    TODO :
    * support for other image formats?
    * should we support more than 2 floppy disk units? (Mac SE supported 3 drives)

*********************************************************************/

#include "emu.h"
#include "machine/applefdc.h"
#include "sonydriv.h"
#include "formats/ap_dsk35.h"
#include "imagedev/flopdrv.h"


#ifdef MAME_DEBUG
#define LOG_SONY        1
#define LOG_SONY_EXTRA  0
#else
#define LOG_SONY        0
#define LOG_SONY_EXTRA  0
#endif

/*
    These lines are normally connected to the PHI0-PHI3 lines of the IWM
*/
enum
{
	SONY_CA0        = 0x01,
	SONY_CA1        = 0x02,
	SONY_CA2        = 0x04,
	SONY_LSTRB      = 0x08
};

/*
    Structure that describes the state of a floppy drive, and the associated
    disk image
*/
struct floppy_t
{
	device_t *img;
	emu_file *fd;

	unsigned int disk_switched : 1; /* disk-in-place status bit */
	unsigned int head : 1;          /* active head (-> floppy side) */
	unsigned int step : 1;
	int motor_on;

	unsigned int loadedtrack_valid : 1; /* is data in track buffer valid ? */
	unsigned int loadedtrack_dirty : 1; /* has data in track buffer been modified? */
	size_t loadedtrack_size;        /* size of loaded track */
	size_t loadedtrack_pos;         /* position within loaded track */
	UINT8 *loadedtrack_data;        /* pointer to track buffer */

	int is_fdhd;                /* is drive an FDHD? */
	int is_400k;                /* drive is single-sided, which means 400K */
};

struct sonydriv_t
{
	int lines;              /* four lines SONY_CA0 - SONY_LSTRB */

	int floppy_enable;  /* whether a drive is enabled or not (-> enable line) */
	int floppy_select;  /* which drive is enabled */

	int sel_line;           /* one single line Is 0 or 1 */

	unsigned int rotation_speed;        /* drive rotation speed - ignored if ext_speed_control == 0 */
	floppy_t floppy[2];         /* data for two floppy disk units */
};
static sonydriv_t sony;

/* bit of code used in several places - I am unsure why it is here */
static int sony_enable2(void)
{
	return (sony.lines & SONY_CA1) && (sony.lines & SONY_LSTRB);
}

static void load_track_data(device_t *device,int floppy_select)
{
	int track_size;
	legacy_floppy_image_device *cur_image;
	UINT8 *new_data;
	floppy_t *f;

	f = &sony.floppy[floppy_select];
	cur_image = floppy_get_device_by_type(device->machine(), FLOPPY_TYPE_SONY, floppy_select);

	floppy_image_legacy *fimg = cur_image->flopimg_get_image();

	if (!fimg)
	{
		return;
	}

	track_size = floppy_get_track_size(fimg, f->head, cur_image->floppy_drive_get_current_track());
	if (f->loadedtrack_data) auto_free(device->machine(),f->loadedtrack_data);
	new_data = auto_alloc_array(device->machine(),UINT8,track_size);
	if (!new_data)
	{
		return;
	}

	cur_image->floppy_drive_read_track_data_info_buffer(f->head, new_data, &track_size);
	f->loadedtrack_valid = 1;
	f->loadedtrack_dirty = 0;
	f->loadedtrack_size = track_size;
	f->loadedtrack_data = new_data;
	f->loadedtrack_pos = 0;
}



static void save_track_data(device_t *device, int floppy_select)
{
	legacy_floppy_image_device *cur_image;
	floppy_t *f;
	int len;

	f = &sony.floppy[floppy_select];
	cur_image = floppy_get_device_by_type(device->machine(), FLOPPY_TYPE_SONY, floppy_select);

	if (f->loadedtrack_dirty)
	{
		len = f->loadedtrack_size;
		cur_image->floppy_drive_write_track_data_info_buffer(f->head, f->loadedtrack_data, &len);
		f->loadedtrack_dirty = 0;
	}
}



UINT8 sony_read_data(device_t *device)
{
	UINT8 result = 0;
	legacy_floppy_image_device *cur_image;
	floppy_t *f;

	if (sony_enable2() || (! sony.floppy_enable))
		return 0xFF;            /* right ??? */

	f = &sony.floppy[sony.floppy_select];
	cur_image = floppy_get_device_by_type(device->machine(), FLOPPY_TYPE_SONY, sony.floppy_select);
	if (!cur_image->exists())
		return 0xFF;

	if (!f->loadedtrack_valid)
		load_track_data(device, sony.floppy_select);

	if (!f->loadedtrack_data)
	{
		return 0xFF;
	}

	result = sony_fetchtrack(f->loadedtrack_data, f->loadedtrack_size, &f->loadedtrack_pos);
	return result;
}



void sony_write_data(device_t *device,UINT8 data)
{
	device_image_interface *cur_image;
	floppy_t *f;

	f = &sony.floppy[sony.floppy_select];
	cur_image = dynamic_cast<device_image_interface *>(floppy_get_device_by_type(device->machine(), FLOPPY_TYPE_SONY, sony.floppy_select));
	if (!cur_image->exists())
		return;

	if (!f->loadedtrack_valid)
		load_track_data(device,sony.floppy_select);

	if (!f->loadedtrack_data)
	{
		return;
	}

	sony_filltrack(f->loadedtrack_data, f->loadedtrack_size, &f->loadedtrack_pos, data);
	f->loadedtrack_dirty = 1;
}



static int sony_rpm(floppy_t *f, legacy_floppy_image_device *cur_image)
{
	int result = 0;
	/*
	 * The Mac floppy controller was interesting in that its speed was adjusted
	 * while the thing was running.  On the tracks closer to the rim, it was
	 * sped up so that more data could be placed on it.  Hence, this function
	 * has different results depending on the track number
	 *
	 * The Mac Plus (and probably the other Macs that use the IWM) verify that
	 * the speed of the floppy drive is within a certain range depending on
	 * what track the floppy is at.  These RPM values are just guesses and are
	 * probably not fully accurate, but they are within the range that the Mac
	 * Plus expects and thus are probably in the right ballpark.
	 *
	 * Note - the timing values are the values returned by the Mac Plus routine
	 * that calculates the speed; I'm not sure what units they are in
	 */

	if ((f->is_400k) && (sony.rotation_speed))
	{
		/* 400k unit : rotation speed should be controlled by computer */
		result = sony.rotation_speed;
	}
	else
	{   /* 800k unit : rotation speed controlled by drive */
#if 1   /* Mac Plus */
		static const int speeds[] =
		{
			500,    /* 00-15:   timing value 117B (acceptable range {1135-11E9} */
			550,    /* 16-31:   timing value ???? (acceptable range {12C6-138A} */
			600,    /* 32-47:   timing value ???? (acceptable range {14A7-157F} */
			675,    /* 48-63:   timing value ???? (acceptable range {16F2-17E2} */
			750     /* 64-79:   timing value ???? (acceptable range {19D0-1ADE} */
		};
#else   /* Lisa 2 */
		/* 237 + 1.3*(256-reg) */
		static const int speeds[] =
		{
			293,    /* 00-15:   timing value ???? (acceptable range {0330-0336} */
			322,    /* 16-31:   timing value ???? (acceptable range {02ED-02F3} */
			351,    /* 32-47:   timing value ???? (acceptable range {02A7-02AD} */
			394,    /* 48-63:   timing value ???? (acceptable range {0262-0266} */
			439     /* 64-79:   timing value ???? (acceptable range {021E-0222} */
		};
#endif
		if (cur_image && cur_image->exists())
			result = speeds[cur_image->floppy_drive_get_current_track() / 16];
	}
	return result;
}

int sony_read_status(device_t *device)
{
	int result = 1;
	int action;
	floppy_t *f;
	legacy_floppy_image_device *cur_image;

	action = ((sony.lines & (SONY_CA1 | SONY_CA0)) << 2) | (sony.sel_line << 1) | ((sony.lines & SONY_CA2) >> 2);

	if (LOG_SONY_EXTRA)
	{
		printf("sony.status(): action=%x pc=0x%08x%s\n",
			action, (int) device->machine().firstcpu->pc(), sony.floppy_enable ? "" : " (no drive enabled)");
	}

	if ((! sony_enable2()) && sony.floppy_enable)
	{
		f = &sony.floppy[sony.floppy_select];
		cur_image = floppy_get_device_by_type(device->machine(), FLOPPY_TYPE_SONY, sony.floppy_select);
		if (!cur_image->exists())
			cur_image = nullptr;

		switch(action) {
		case 0x00:  /* Step direction */
			result = f->step;
			break;
		case 0x01:  /* Lower head activate */
			if (f->head != 0)
			{
				save_track_data(device,sony.floppy_select);
				f->head = 0;
				f->loadedtrack_valid = 0;
			}
			result = 0;
			break;
		case 0x02:  /* Disk in place */
			result = cur_image ? 0 : 1; /* 0=disk 1=nodisk */
			break;
		case 0x03:  /* Upper head activate (not on 400k) */
			if ((f->head != 1) && !(f->is_400k))
			{
				save_track_data(device,sony.floppy_select);
				f->head = 1;
				f->loadedtrack_valid = 0;
			}
			result = 0;
			break;
		case 0x04:  /* Disk is stepping 0=stepping 1=not stepping*/
			result = 1;
			break;
		case 0x05:  /* Drive is SuperDrive: 0 = 400/800k, 1 = SuperDrive */
			result = f->is_fdhd ? 1: 0;
			break;
		case 0x06:  /* Disk is locked 0=locked 1=unlocked */
			if (cur_image)
				result = cur_image->floppy_wpt_r();
			else
				result = 0;
			break;
		case 0x08:  /* Motor on 0=on 1=off */
			result = f->motor_on;
			break;
		case 0x09:  /* Number of sides: 0=single sided, 1=double sided */
			if (cur_image)
			{
				floppy_image_legacy *fimg = cur_image->flopimg_get_image();
				if (fimg)
				{
					result = floppy_get_heads_per_disk(fimg) - 1;
					f->is_400k = result ? 0 : 1;
				}
			}
			break;
		case 0x0a:  /* At track 0: 0=track zero 1=not track zero */
			device->logerror("%s sony.status(): reading Track 0\n", device->machine().describe_context());
			if (cur_image)
				result = cur_image->floppy_tk00_r();
			else
				result = 0;
			break;
		case 0x0b:  /* Disk ready: 0=ready, 1=not ready */
			result = 0;
			break;
		case 0x0c:  /* Disk switched */
			{
				if (cur_image)
				{
					if (!cur_image->floppy_dskchg_r())
					{
						f->disk_switched = 1;
					}
				}
				result = f->disk_switched;
			}
			break;
		case 0x0d:  /* Unknown */
			/* I'm not sure what this one does, but the Mac Plus executes the
			 * following code that uses this status:
			 *
			 *  417E52: moveq   #$d, D0     ; Status 0x0d
			 *  417E54: bsr     4185fe      ; Query IWM status
			 *  417E58: bmi     417e82      ; If result=1, then skip
			 *
			 * This code is called in the Sony driver's open method, and
			 * _AddDrive does not get called if this status 0x0d returns 1.
			 * Hence, we are returning 0
			 */
			result = 0;
			break;
		case 0x0e:  /* Tachometer */
			/* (time in seconds) / (60 sec/minute) * (rounds/minute) * (60 pulses) * (2 pulse phases) */
			if (cur_image)
			{
				result = ((int) (device->machine().time().as_double() / 60.0 * sony_rpm(f, cur_image) * 60.0 * 2.0)) & 1;
			}
			break;
		case 0x0f:  /* 400k/800k: Drive installed: 0=drive connected, 1=drive not connected */
				/* FDHD: Inserted disk density: 0=HD, 1=DD */
			if (f->is_fdhd)
			{
				result = 1;
			}
			else
			{
				result = 0;
			}
			break;
		default:
			if (LOG_SONY)
				device->logerror("sony_status(): unknown action\n");
			break;
		}
	}

	return result;
}

static void sony_doaction(device_t *device)
{
	int action;
	floppy_t *f;
	legacy_floppy_image_device *cur_image;

	action = ((sony.lines & (SONY_CA1 | SONY_CA0)) << 2) | ((sony.lines & SONY_CA2) >> 2) | (sony.sel_line << 1);

	if (LOG_SONY)
	{
		device->logerror("%s sony_doaction(): action=%d %s\n",
			device->machine().describe_context(), action, (sony.floppy_enable) ? "" : " (MOTOR OFF)");
	}

	if (sony.floppy_enable)
	{
		f = &sony.floppy[sony.floppy_select];
		cur_image = floppy_get_device_by_type(device->machine(), FLOPPY_TYPE_SONY, sony.floppy_select);
		if (!cur_image->exists())
			cur_image = nullptr;

		switch(action)
		{
		case 0x00:  /* Set step inward (higher tracks) */
			f->step = 0;
			break;
		case 0x01:  /* Set step outward (lower tracks) */
			f->step = 1;
			break;
		case 0x03:  /* Reset diskswitched */
			f->disk_switched = 0;
			break;
		case 0x04:  /* Step disk */
			if (cur_image)
			{
				save_track_data(device,sony.floppy_select);
				if (f->step)
					cur_image->floppy_drive_seek(-1);
				else
					cur_image->floppy_drive_seek(+1);
				f->loadedtrack_valid = 0;
			}
			break;
		case 0x08:  /* Turn motor on */
			f->motor_on = CLEAR_LINE;
			if (cur_image)
				cur_image->floppy_mon_w(f->motor_on);
			break;
		case 0x09:  /* Turn motor off */
			f->motor_on = ASSERT_LINE;
			if (cur_image)
				cur_image->floppy_mon_w(f->motor_on);
			break;
		case 0x0d:  /* Eject disk */
			if (cur_image)
				cur_image->unload();
			break;
		default:
			if (LOG_SONY)
				device->logerror("sony_doaction(): unknown action %d\n", action);
			break;
		}
	}
}

void sony_set_lines(device_t *device,UINT8 lines)
{
	int old_sony_lines = sony.lines;

	sony.lines = lines & 0x0F;

	{
		//int action = ((sony.lines & (SONY_CA1 | SONY_CA0)) << 2) | (sony.sel_line << 1) | ((sony.lines & SONY_CA2) >> 2);
		//printf("sony.set_lines: %02x, action now %d\n", lines&0xf, action);
	}

	/* have we just set LSTRB ? */
	if ((sony.lines & ~old_sony_lines) & SONY_LSTRB)
	{
		/* if so, write drive reg */
		sony_doaction(device);
	}

	if (LOG_SONY_EXTRA)
		device->logerror("sony.set_lines(): %d\n", lines);
}

void sony_set_enable_lines(device_t *device,int enable_mask)
{
	switch (enable_mask)
	{
	case 0:
	default:    /* well, we have to do something, right ? */
		sony.floppy_enable = 0;
		break;
	case 1:
		sony.floppy_enable = 1;
		sony.floppy_select = 0;
		break;
	case 2:
		sony.floppy_enable = 1;
		sony.floppy_select = 1;
		break;
	}

	if (LOG_SONY_EXTRA)
		device->logerror("sony.set_enable_lines(): %d\n", enable_mask);
}

void sony_set_sel_line(device_t *device,int sel)
{
	sony.sel_line = sel ? 1 : 0;

	{
		//int action = ((sony.lines & (SONY_CA1 | SONY_CA0)) << 2) | (sony.sel_line << 1) | ((sony.lines & SONY_CA2) >> 2);
		//printf("sony.set_sel_line: %d, action now %d\n", sony.sel_line, action);
	}

	if (LOG_SONY_EXTRA)
		device->logerror("sony.set_sel_line(): %s line IWM_SEL\n", sony.sel_line ? "setting" : "clearing");
}

void sony_set_speed(int speed)
{
	sony.rotation_speed = speed;
}

// device type definition
const device_type FLOPPY_SONY = &device_creator<sonydriv_floppy_image_device>;

//-------------------------------------------------
//  sonydriv_floppy_image_device - constructor
//-------------------------------------------------

sonydriv_floppy_image_device::sonydriv_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: legacy_floppy_image_device(mconfig, FLOPPY_SONY, "Floppy Disk [Sony]", tag, owner, clock, "floppy_sonny", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sonydriv_floppy_image_device::device_start()
{
	legacy_floppy_image_device::device_start();
	floppy_set_type(FLOPPY_TYPE_SONY);

	sony.floppy[0].is_fdhd = 0;
	sony.floppy[1].is_fdhd = 0;
	sony.floppy[0].is_400k = 0;
	sony.floppy[1].is_400k = 0;
	sony.floppy[0].loadedtrack_data = nullptr;
	sony.floppy[1].loadedtrack_data = nullptr;
	sony.floppy[0].head = 0;
	sony.floppy[1].head = 0;
	sony.rotation_speed = 0;
}

void sonydriv_floppy_image_device::call_unload()
{
	int id;
	device_t *fdc;

	/* locate the FDC */
	fdc = machine().device("fdc");

	id = floppy_get_drive_by_type(this,FLOPPY_TYPE_SONY);
	save_track_data(fdc, id);
	memset(&sony.floppy[id], 0, sizeof(sony.floppy[id]));

	legacy_floppy_image_device::call_unload();
}
