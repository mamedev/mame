/****************************************************f***********************

    Amiga floppy disk controller emulation

    - The drive spins at 300 RPM = 1 Rev every 200ms
    - At 2usec per bit, that's 12500 bytes per track, or 6250 words per track
    - At 4usec per bit, that's 6250 bytes per track, or 3125 words per track
    - Each standard amiga sector has 544 words per sector, and 11 sectors per track = 11968 bytes
    - ( 12500(max) - 11968(actual) ) = 532 per track gap bytes

***************************************************************************/


#include "emu.h"
#include "includes/amiga.h"
#include "imagedev/flopdrv.h"
#include "formats/ami_dsk.h"
#include "amigafdc.h"
#include "machine/6526cia.h"


#define MAX_TRACK_BYTES			12500
#define ACTUAL_TRACK_BYTES		11968
#define GAP_TRACK_BYTES			( MAX_TRACK_BYTES - ACTUAL_TRACK_BYTES )
#define ONE_SECTOR_BYTES		(544*2)
#define ONE_REV_TIME			200 /* ms */
#define MAX_WORDS_PER_DMA_CYCLE	32 /* 64 bytes per dma cycle */
#define DISK_DETECT_DELAY		1
#define MAX_TRACKS				160
#define MAX_MFM_TRACK_LEN		16384

#define NUM_DRIVES 2

/* required prototype */
static void setup_fdc_buffer( device_t *device,int drive );

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/
typedef struct {
	int motor_on;
	int side;
	int dir;
	int wprot;
	int disk_changed;
	device_image_interface *f;
	UINT32 extinfo[MAX_TRACKS];
	UINT32 extoffs[MAX_TRACKS];
	int is_ext_image;
	int cyl;
	int tracklen;
	UINT8 mfm[MAX_MFM_TRACK_LEN];
	int	cached;
	emu_timer *rev_timer;
	emu_timer *sync_timer;
	emu_timer *dma_timer;
	int rev_timer_started;
	int pos;
	int len;
	offs_t ptr;
} fdc_def;

typedef struct _amiga_fdc_t amiga_fdc_t;
struct _amiga_fdc_t
{
	fdc_def fdc_status[NUM_DRIVES];

	/* signals */
	int fdc_sel;
	int fdc_dir;
	int fdc_side;
	int fdc_step;
	int fdc_rdy;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/
INLINE amiga_fdc_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == AMIGA_FDC);

	return (amiga_fdc_t *)downcast<legacy_device_base *>(device)->token();
}

static TIMER_CALLBACK(fdc_rev_proc);
static TIMER_CALLBACK(fdc_dma_proc);
static TIMER_CALLBACK(fdc_sync_proc);
static void check_extended_image( device_t *device, int id );

static void amiga_load_proc(device_image_interface &image)
{
	amiga_fdc_t *fdc = get_safe_token(image.device().owner());
	int id = floppy_get_drive(&image.device());
	fdc->fdc_status[id].disk_changed = DISK_DETECT_DELAY;
	fdc->fdc_status[id].f = &image;
	fdc->fdc_status[id].cached = -1;
	fdc->fdc_status[id].motor_on = 0;
	fdc->fdc_status[id].rev_timer_started = 0;
	fdc->fdc_status[id].rev_timer->reset(  );
	fdc->fdc_status[id].sync_timer->reset(  );
	fdc->fdc_status[id].dma_timer->reset(  );
	fdc->fdc_rdy = 0;

	check_extended_image( image.device().owner(), id );
}

static void amiga_unload_proc(device_image_interface &image)
{
	amiga_fdc_t *fdc = get_safe_token(image.device().owner());
	int id = floppy_get_drive(&image.device());
	fdc->fdc_status[id].disk_changed = DISK_DETECT_DELAY;
	fdc->fdc_status[id].f = NULL;
	fdc->fdc_status[id].cached = -1;
	fdc->fdc_status[id].motor_on = 0;
	fdc->fdc_status[id].rev_timer_started = 0;
	fdc->fdc_status[id].rev_timer->reset(  );
	fdc->fdc_status[id].sync_timer->reset(  );
	fdc->fdc_status[id].dma_timer->reset(  );
	fdc->fdc_rdy = 0;
}

/*-------------------------------------------------
    DEVICE_START(amiga_fdc)
-------------------------------------------------*/
device_t *amiga_floppy_get_device_child(device_t *device,int drive)
{
	switch(drive) {
		case 0 : return device->subdevice(FLOPPY_0);
		case 1 : return device->subdevice(FLOPPY_1);
	}
	return NULL;
}

static DEVICE_START(amiga_fdc)
{
	amiga_fdc_t *fdc = get_safe_token(device);
	int id;
	for(id=0;id<2;id++)
	{
		fdc->fdc_status[id].rev_timer = device->machine().scheduler().timer_alloc(FUNC(fdc_rev_proc), (void*)device);
		fdc->fdc_status[id].dma_timer = device->machine().scheduler().timer_alloc(FUNC(fdc_dma_proc), (void*)device);
		fdc->fdc_status[id].sync_timer = device->machine().scheduler().timer_alloc(FUNC(fdc_sync_proc), (void*)device);
		floppy_install_load_proc(amiga_floppy_get_device_child(device, id), amiga_load_proc);
		floppy_install_unload_proc(amiga_floppy_get_device_child(device, id), amiga_unload_proc);
		fdc->fdc_status[id].motor_on = 0;
		fdc->fdc_status[id].side = 0;
		fdc->fdc_status[id].dir = 0;
		fdc->fdc_status[id].wprot = 1;
		fdc->fdc_status[id].cyl = 0;
		fdc->fdc_status[id].rev_timer_started = 0;
		fdc->fdc_status[id].cached = -1;
		fdc->fdc_status[id].pos = 0;
		fdc->fdc_status[id].len = 0;
		fdc->fdc_status[id].ptr = 0;
		fdc->fdc_status[id].f = NULL;
		fdc->fdc_status[id].is_ext_image = 0;
		fdc->fdc_status[id].tracklen = MAX_TRACK_BYTES;
		fdc->fdc_status[id].disk_changed = DISK_DETECT_DELAY;
		fdc->fdc_status[id].rev_timer->reset(  );
		fdc->fdc_status[id].sync_timer->reset(  );
		fdc->fdc_status[id].dma_timer->reset(  );
		memset( fdc->fdc_status[id].mfm, 0xaa, MAX_MFM_TRACK_LEN );
	}
	fdc->fdc_sel = 0x0f;
	fdc->fdc_dir = 0;
	fdc->fdc_side = 1;
	fdc->fdc_step = 1;
	fdc->fdc_rdy = 0;

}


/*-------------------------------------------------
    DEVICE_RESET(amiga_fdc)
-------------------------------------------------*/

static DEVICE_RESET(amiga_fdc)
{
}

static void check_extended_image( device_t *device, int id )
{
	UINT8	header[8], data[4];
	amiga_fdc_t *fdc = get_safe_token(device);

	fdc->fdc_status[id].is_ext_image = 0;

	if ( fdc->fdc_status[id].f->fseek( 0, SEEK_SET ) )
	{
		logerror("FDC: image_fseek failed!\n" );
		return;
	}

	fdc->fdc_status[id].f->fread( &header, sizeof( header ) );

	if ( memcmp( header, "UAE--ADF", sizeof( header ) ) == 0 )
	{
		int		i;

		for( i = 0; i < MAX_TRACKS; i++ )
		{
			fdc->fdc_status[id].f->fread( &data, sizeof( data ) );

			/* data[0,1] = SYNC - data[2,3] = LEN */

			fdc->fdc_status[id].extinfo[i] = data[0];
			fdc->fdc_status[id].extinfo[i] <<= 8;
			fdc->fdc_status[id].extinfo[i] |= data[1];
			fdc->fdc_status[id].extinfo[i] <<= 8;
			fdc->fdc_status[id].extinfo[i] |= data[2];
			fdc->fdc_status[id].extinfo[i] <<= 8;
			fdc->fdc_status[id].extinfo[i] |= data[3];

			if ( i == 0 )
				fdc->fdc_status[id].extoffs[i] = sizeof( header ) + ( MAX_TRACKS * sizeof( data ) );
			else
				fdc->fdc_status[id].extoffs[i] = fdc->fdc_status[id].extoffs[i-1] + ( fdc->fdc_status[id].extinfo[i-1] & 0xffff );
		}

		fdc->fdc_status[id].is_ext_image = 1;
	}
}

static int fdc_get_curpos( device_t *device, int drive )
{
	amiga_state *state = device->machine().driver_data<amiga_state>();
	double elapsed;
	int speed;
	int	bytes;
	int pos;
	amiga_fdc_t *fdc = get_safe_token(device);

	if ( fdc->fdc_status[drive].rev_timer_started == 0 ) {
		logerror("Rev timer not started on drive %d, cant get position!\n", drive );
		return 0;
	}

	elapsed = fdc->fdc_status[drive].rev_timer ->elapsed( ).as_double();
	speed = ( CUSTOM_REG(REG_ADKCON) & 0x100 ) ? 2 : 4;

	bytes = elapsed /  attotime::from_usec( speed * 8 ).as_double();
	pos = bytes % ( fdc->fdc_status[drive].tracklen );

	return pos;
}

UINT16 amiga_fdc_get_byte (device_t *device)
{
	amiga_state *state = device->machine().driver_data<amiga_state>();
	int pos;
	int i, drive = -1;
	UINT16 ret;
	amiga_fdc_t *fdc = get_safe_token(device);

	ret = ( ( CUSTOM_REG(REG_DSKLEN) >> 1 ) & 0x4000 ) & ( ( CUSTOM_REG(REG_DMACON) << 10 ) & 0x4000 );
	ret |= ( CUSTOM_REG(REG_DSKLEN) >> 1 ) & 0x2000;

	for ( i = 0; i < NUM_DRIVES; i++ ) {
		if ( !( fdc->fdc_sel & ( 1 << i ) ) )
			drive = i;
	}

	if ( drive == -1 )
		return ret;

	if ( fdc->fdc_status[drive].disk_changed )
		return ret;

	setup_fdc_buffer( device,drive );

	pos = fdc_get_curpos( device, drive );

	if ( fdc->fdc_status[drive].mfm[pos] == ( CUSTOM_REG(REG_DSRSYNC) >> 8 ) &&
		 fdc->fdc_status[drive].mfm[pos+1] == ( CUSTOM_REG(REG_DSRSYNC) & 0xff ) )
			ret |= 0x1000;

	if ( pos != fdc->fdc_status[drive].pos ) {
		ret |= 0x8000;
		fdc->fdc_status[drive].pos = pos;
	}

	ret |= fdc->fdc_status[drive].mfm[pos];

	return ret;
}

static TIMER_CALLBACK(fdc_sync_proc)
{
	amiga_state *state = machine.driver_data<amiga_state>();
	int drive = param;
	UINT16			sync = CUSTOM_REG(REG_DSRSYNC);
	int				cur_pos;
	int				sector;
	int				time;
	amiga_fdc_t *fdc = get_safe_token((device_t*)ptr);

	/* if floppy got ejected, stop */
	if ( fdc->fdc_status[drive].disk_changed )
		goto bail;

	if ( fdc->fdc_status[drive].motor_on == 0 )
		goto bail;

	cur_pos = fdc_get_curpos( (device_t*)ptr, drive );

	if ( cur_pos <= ( GAP_TRACK_BYTES + 6 ) )
	{
		sector = 0;
	}
	else
	{
		sector = ( cur_pos - ( GAP_TRACK_BYTES + 6 ) ) / ONE_SECTOR_BYTES;
	}

	setup_fdc_buffer( (device_t*)ptr, drive );

	if ( cur_pos < 2 )
		cur_pos = 2;

	if ( fdc->fdc_status[drive].mfm[cur_pos-2] == ( ( sync >> 8 ) & 0xff ) &&
		 fdc->fdc_status[drive].mfm[cur_pos-1] == ( sync & 0xff ) )
	{
		address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

		amiga_custom_w(space, REG_INTREQ, 0x8000 | INTENA_DSKSYN, 0xffff);
	}

	if ( sector < 10 )
	{
		/* start the sync timer */
		time = ONE_SECTOR_BYTES;
		time *= ( CUSTOM_REG(REG_ADKCON) & 0x0100 ) ? 2 : 4;
		time *= 8;
		fdc->fdc_status[drive].sync_timer->adjust(attotime::from_usec( time ), drive);
		return;
	}

bail:
	fdc->fdc_status[drive].sync_timer->reset(  );
}

static TIMER_CALLBACK(fdc_dma_proc)
{
	amiga_state *state = machine.driver_data<amiga_state>();
	int drive = param;
	amiga_fdc_t *fdc = get_safe_token((device_t*)ptr);

	/* if DMA got disabled by the time we got here, stop operations */
	if ( ( CUSTOM_REG(REG_DSKLEN) & 0x8000 ) == 0 )
		goto bail;

	if ( ( CUSTOM_REG(REG_DMACON) & 0x0210 ) == 0 )
		goto bail;

	/* if floppy got ejected, also stop */
	if ( fdc->fdc_status[drive].disk_changed )
		goto bail;

	if ( fdc->fdc_status[drive].motor_on == 0 )
		goto bail;

	setup_fdc_buffer( (device_t*)ptr, drive );

	if ( CUSTOM_REG(REG_DSKLEN) & 0x4000 ) /* disk write case, unsupported yet */
	{
		if ( fdc->fdc_status[drive].len > 0 )
		{
			address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

			logerror("Write to disk unsupported yet\n" );

			amiga_custom_w(space, REG_INTREQ, 0x8000 | INTENA_DSKBLK, 0xffff);
		}
	}
	else
	{
		offs_t offset = fdc->fdc_status[drive].ptr;
		int cur_pos = fdc->fdc_status[drive].pos;
		int len = fdc->fdc_status[drive].len;

		if ( len > MAX_WORDS_PER_DMA_CYCLE ) len = MAX_WORDS_PER_DMA_CYCLE;

		fdc->fdc_status[drive].len -= len;

		while ( len-- )
		{
			int dat = ( fdc->fdc_status[drive].mfm[cur_pos++] ) << 8;

			cur_pos %= ( fdc->fdc_status[drive].tracklen );

			dat |= fdc->fdc_status[drive].mfm[cur_pos++];

			cur_pos %= ( fdc->fdc_status[drive].tracklen );

			(*state->m_chip_ram_w)(state, offset, dat);

			offset += 2;
		}

		fdc->fdc_status[drive].ptr = offset;
		fdc->fdc_status[drive].pos = cur_pos;

		if ( fdc->fdc_status[drive].len <= 0 )
		{
			address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

			amiga_custom_w(space, REG_INTREQ, 0x8000 | INTENA_DSKBLK, 0xffff);
		}
		else
		{
			int time;
			double	len_words = fdc->fdc_status[drive].len;
			if ( len_words > MAX_WORDS_PER_DMA_CYCLE ) len_words = MAX_WORDS_PER_DMA_CYCLE;

			time = len_words * 2;
			time *= ( CUSTOM_REG(REG_ADKCON) & 0x0100 ) ? 2 : 4;
			time *= 8;
			fdc->fdc_status[drive].dma_timer->adjust(attotime::from_usec( time ), drive);
			return;
		}
	}

bail:
	fdc->fdc_status[drive].dma_timer->reset(  );
}

void amiga_fdc_setup_dma( device_t *device ) {
	amiga_state *state = device->machine().driver_data<amiga_state>();
	int i, cur_pos, drive = -1, len_words = 0;
	int time = 0;
	amiga_fdc_t *fdc = get_safe_token(device);

	for ( i = 0; i < NUM_DRIVES; i++ ) {
		if ( !( fdc->fdc_sel & ( 1 << i ) ) )
			drive = i;
	}

	if ( drive == -1 ) {
		address_space *space = device->machine().device("maincpu")->memory().space(AS_PROGRAM);

		logerror("Disk DMA started with no drive selected!\n" );
		amiga_custom_w(space, REG_INTREQ, 0x8000 | INTENA_DSKBLK, 0xffff);
		return;
	}

	if ( ( CUSTOM_REG(REG_DSKLEN) & 0x8000 ) == 0 )
		goto bail;

	if ( ( CUSTOM_REG(REG_DMACON) & 0x0210 ) == 0 )
		goto bail;

	if ( fdc->fdc_status[drive].disk_changed )
		goto bail;

	setup_fdc_buffer( device,drive );

	fdc->fdc_status[drive].len = CUSTOM_REG(REG_DSKLEN) & 0x3fff;
	fdc->fdc_status[drive].ptr = CUSTOM_REG_LONG(REG_DSKPTH);
	fdc->fdc_status[drive].pos = cur_pos = fdc_get_curpos( device, drive );

	/* we'll do a set amount at a time */
	len_words = fdc->fdc_status[drive].len;
	if ( len_words > MAX_WORDS_PER_DMA_CYCLE ) len_words = MAX_WORDS_PER_DMA_CYCLE;

	if ( CUSTOM_REG(REG_ADKCON) & 0x0400 ) { /* Wait for sync */
		if ( CUSTOM_REG(REG_DSRSYNC) != 0x4489 ) {
			logerror("Attempting to read a non-standard SYNC\n" );
		}

		i = cur_pos;
		do {
			if ( fdc->fdc_status[drive].mfm[i] == ( CUSTOM_REG(REG_DSRSYNC) >> 8 ) &&
				 fdc->fdc_status[drive].mfm[i+1] == ( CUSTOM_REG(REG_DSRSYNC) & 0xff ) )
					break;

			i++;
			i %= ( fdc->fdc_status[drive].tracklen );
			time++;
		} while( i != cur_pos );

		if ( i == cur_pos && time != 0 ) {
			logerror("SYNC not found on track!\n" );
			return;
		} else {
			fdc->fdc_status[drive].pos = i + 2;
			time += 2;
		}
	}

	time += len_words * 2;
	time *= ( CUSTOM_REG(REG_ADKCON) & 0x0100 ) ? 2 : 4;
	time *= 8;
	fdc->fdc_status[drive].dma_timer->adjust(attotime::from_usec( time ), drive);

	return;

bail:
	fdc->fdc_status[drive].dma_timer->reset(  );
}

static void setup_fdc_buffer( device_t *device,int drive )
{
	int		sector, offset, len;
	UINT8	temp_cyl[512*11];
	amiga_fdc_t *fdc = get_safe_token(device);

	/* no disk in drive */
	if ( fdc->fdc_status[drive].f == NULL ) {
		memset( &fdc->fdc_status[drive].mfm[0], 0xaa, 32 );
		fdc->fdc_status[drive].disk_changed = DISK_DETECT_DELAY;
		return;
	}

	len = 512*11;

	offset = ( fdc->fdc_status[drive].cyl << 1 ) | fdc->fdc_side;

	if ( fdc->fdc_status[drive].cached == offset )
		return;

#if 0
	popmessage("T:%d S:%d", fdc->fdc_status[drive].cyl, fdc_side );
#endif

	if ( fdc->fdc_status[drive].is_ext_image )
	{
		len = fdc->fdc_status[drive].extinfo[offset] & 0xffff;

		if ( len > ( MAX_MFM_TRACK_LEN - 2 ) )
			len = MAX_MFM_TRACK_LEN - 2;

		if ( fdc->fdc_status[drive].f->fseek( fdc->fdc_status[drive].extoffs[offset], SEEK_SET ) )
		{
			logerror("FDC: image_fseek failed!\n" );
			fdc->fdc_status[drive].f = NULL;
			fdc->fdc_status[drive].disk_changed = DISK_DETECT_DELAY;
			return;
		}

		/* if SYNC is 0000, then its a regular amiga dos track image */
		if ( ( fdc->fdc_status[drive].extinfo[offset] >> 16 ) == 0x0000 )
		{
			fdc->fdc_status[drive].f->fread( temp_cyl, len );
			/* fall through to regular load */
		}
		else /* otherwise, it's a raw track */
		{
			fdc->fdc_status[drive].mfm[0] = ( fdc->fdc_status[drive].extinfo[offset] >> 24 ) & 0xff;
			fdc->fdc_status[drive].mfm[1] = ( fdc->fdc_status[drive].extinfo[offset] >> 16 ) & 0xff;
			fdc->fdc_status[drive].f->fread( &fdc->fdc_status[drive].mfm[2], len );
			fdc->fdc_status[drive].tracklen = len + 2;
			fdc->fdc_status[drive].cached = offset;
			return;
		}
	}
	else
	{
		if ( fdc->fdc_status[drive].f->fseek( offset * len, SEEK_SET ) )
		{
			logerror("FDC: image_fseek failed!\n" );
			fdc->fdc_status[drive].f = NULL;
			fdc->fdc_status[drive].disk_changed = DISK_DETECT_DELAY;
			return;
		}

		fdc->fdc_status[drive].f->fread( temp_cyl, len );
	}

	fdc->fdc_status[drive].tracklen = MAX_TRACK_BYTES;

	memset( &fdc->fdc_status[drive].mfm[ONE_SECTOR_BYTES*11], 0xaa, GAP_TRACK_BYTES );

	for ( sector = 0; sector < 11; sector++ ) {
		int x;
	    UINT8 *dest = ( &fdc->fdc_status[drive].mfm[(ONE_SECTOR_BYTES*sector)] );
	    UINT8 *src = &temp_cyl[sector*512];
		UINT32 tmp;
		UINT32 even, odd;
		UINT32 hck = 0, dck = 0;

		/* Preamble and sync */
		*(dest + 0) = 0xaa;
		*(dest + 1) = 0xaa;
		*(dest + 2) = 0xaa;
		*(dest + 3) = 0xaa;
		*(dest + 4) = 0x44;
		*(dest + 5) = 0x89;
		*(dest + 6) = 0x44;
		*(dest + 7) = 0x89;

		/* Track and sector info */

		tmp = 0xff000000 | (offset<<16) | (sector<<8) | (11 - sector);
		odd = (tmp & 0x55555555) | 0xaaaaaaaa;
		even = ( ( tmp >> 1 ) & 0x55555555 ) | 0xaaaaaaaa;
		*(dest +  8) = (UINT8) ((even & 0xff000000)>>24);
		*(dest +  9) = (UINT8) ((even & 0xff0000)>>16);
		*(dest + 10) = (UINT8) ((even & 0xff00)>>8);
		*(dest + 11) = (UINT8) ((even & 0xff));
		*(dest + 12) = (UINT8) ((odd & 0xff000000)>>24);
		*(dest + 13) = (UINT8) ((odd & 0xff0000)>>16);
		*(dest + 14) = (UINT8) ((odd & 0xff00)>>8);
		*(dest + 15) = (UINT8) ((odd & 0xff));

		/* Fill unused space */

		for (x = 16 ; x < 48; x++)
			*(dest + x) = 0xaa;

		/* Encode data section of sector */

		for (x = 64 ; x < 576; x++)
		{
			tmp = *(src + x - 64);
			odd = (tmp & 0x55);
			even = (tmp>>1) & 0x55;
			*(dest + x) = (UINT8) (even | 0xaa);
			*(dest + x + 512) = (UINT8) (odd | 0xaa);
		}

		/* Calculate checksum for unused space */

		for(x = 8; x < 48; x += 4)
		    hck ^= (((UINT32) *(dest + x))<<24) | (((UINT32) *(dest + x + 1))<<16) |
		    	   (((UINT32) *(dest + x + 2))<<8) | ((UINT32) *(dest + x + 3));

		even = odd = hck;
		odd >>= 1;
		even |= 0xaaaaaaaa;
		odd |= 0xaaaaaaaa;

		*(dest + 48) = (UINT8) ((odd & 0xff000000)>>24);
		*(dest + 49) = (UINT8) ((odd & 0xff0000)>>16);
		*(dest + 50) = (UINT8) ((odd & 0xff00)>>8);
		*(dest + 51) = (UINT8) (odd & 0xff);
		*(dest + 52) = (UINT8) ((even & 0xff000000)>>24);
		*(dest + 53) = (UINT8) ((even & 0xff0000)>>16);
		*(dest + 54) = (UINT8) ((even & 0xff00)>>8);
		*(dest + 55) = (UINT8) (even & 0xff);

		/* Calculate checksum for data section */

		for(x = 64; x < 1088; x += 4)
			dck ^= (((UINT32) *(dest + x))<<24) | (((UINT32) *(dest + x + 1))<<16) |
				   (((UINT32) *(dest + x + 2))<< 8) |  ((UINT32) *(dest + x + 3));
		even = odd = dck;
		odd >>= 1;
		even |= 0xaaaaaaaa;
		odd |= 0xaaaaaaaa;
		*(dest + 56) = (UINT8) ((odd & 0xff000000)>>24);
		*(dest + 57) = (UINT8) ((odd & 0xff0000)>>16);
		*(dest + 58) = (UINT8) ((odd & 0xff00)>>8);
		*(dest + 59) = (UINT8) (odd & 0xff);
		*(dest + 60) = (UINT8) ((even & 0xff000000)>>24);
		*(dest + 61) = (UINT8) ((even & 0xff0000)>>16);
		*(dest + 62) = (UINT8) ((even & 0xff00)>>8);
		*(dest + 63) = (UINT8) (even & 0xff);
	}

	// update MFM data with proper CLK signal
	int lastbit= 0;
	for(int i=0;i<MAX_TRACK_BYTES ;i++)
	{
		UINT8 c=fdc->fdc_status[drive].mfm[i];
		UINT8 dat = 0;
		for(int j=0;j<8;j=j+2)
		{
			UINT8 c1=(c>>(6-j))&0x3;
			if(c1&0x1)
			{
				dat |= (0x01<<(6-j));
				lastbit=1;
			}
			else
			{
				if(lastbit==0 && (c1&0x2))
				{
					dat |= (0x02<<(6-j));
				}
				else
				{
					dat |= (0x00<<(6-j));
				}
				lastbit=0;
			}
		}
		fdc->fdc_status[drive].mfm[i] = dat;
	}
	fdc->fdc_status[drive].cached = offset;
}

static TIMER_CALLBACK(fdc_rev_proc)
{
	amiga_state *state = machine.driver_data<amiga_state>();
	int drive = param;
	int time;
	device_t *cia;
	amiga_fdc_t *fdc = get_safe_token((device_t*)ptr);

	/* Issue a index pulse when a disk revolution completes */
	cia = machine.device("cia_1");
	mos6526_flag_w(cia, 0);
	mos6526_flag_w(cia, 1);

	fdc->fdc_status[drive].rev_timer->adjust(attotime::from_msec( ONE_REV_TIME ), drive);
	fdc->fdc_status[drive].rev_timer_started = 1;

	if ( fdc->fdc_status[drive].is_ext_image == 0 )
	{
		/* also start the sync timer */
		time = GAP_TRACK_BYTES + 6;
		time *= ( CUSTOM_REG(REG_ADKCON) & 0x0100 ) ? 2 : 4;
		time *= 8;
		fdc->fdc_status[drive].sync_timer->adjust(attotime::from_usec( time ), drive);
	}
}

static void start_rev_timer( device_t *device,int drive ) {
//  double time;
	amiga_fdc_t *fdc = get_safe_token(device);

	if ( fdc->fdc_status[drive].rev_timer_started ) {
		logerror("Revolution timer started twice?!\n" );
		return;
	}

	fdc->fdc_status[drive].rev_timer->adjust(attotime::from_msec( ONE_REV_TIME ), drive);
	fdc->fdc_status[drive].rev_timer_started = 1;
}

static void stop_rev_timer( device_t *device,int drive ) {
	amiga_fdc_t *fdc = get_safe_token(device);

	if ( fdc->fdc_status[drive].rev_timer_started == 0 ) {
		logerror("Revolution timer never started?!\n" );
		return;
	}

	fdc->fdc_status[drive].rev_timer->reset(  );
	fdc->fdc_status[drive].rev_timer_started = 0;
	fdc->fdc_status[drive].dma_timer->reset(  );
	fdc->fdc_status[drive].sync_timer->reset(  );
}

static void fdc_setup_leds(device_t *device, int drive ) {

	char portname[12];
	amiga_fdc_t *fdc = get_safe_token(device);
	sprintf(portname, "drive_%d_led", drive);
	output_set_value(portname, fdc->fdc_status[drive].motor_on == 0 ? 0 : 1);

	if ( drive == 0 )
		set_led_status(device->machine(), 1, fdc->fdc_status[drive].motor_on ); /* update internal drive led */

	if ( drive == 1 )
		set_led_status(device->machine(), 2, fdc->fdc_status[drive].motor_on ); /* update external drive led */
}

static void fdc_stepdrive( device_t *device,int drive ) {
	amiga_fdc_t *fdc = get_safe_token(device);

	if ( fdc->fdc_dir ) {
		if ( fdc->fdc_status[drive].cyl )
			fdc->fdc_status[drive].cyl--;
	} else {
		if ( fdc->fdc_status[drive].cyl < 79 )
			fdc->fdc_status[drive].cyl++;
	}

	/* Update disk detection if applicable */
	if ( fdc->fdc_status[drive].f != NULL )
	{
		if ( fdc->fdc_status[drive].disk_changed > 0 )
			fdc->fdc_status[drive].disk_changed--;
	}
}

static void fdc_motor( device_t *device,int drive, int off ) {
	amiga_fdc_t *fdc = get_safe_token(device);
	int on = !off;

	if ( ( fdc->fdc_status[drive].motor_on == 0 ) && on ) {
		fdc->fdc_status[drive].pos = 0;

		start_rev_timer( device,drive );

	} else {
		if ( fdc->fdc_status[drive].motor_on && off )
			stop_rev_timer(device, drive );
	}

	fdc->fdc_status[drive].motor_on = on;
}

WRITE8_DEVICE_HANDLER( amiga_fdc_control_w )
{
	int step_pulse;
	int drive;

	amiga_fdc_t *fdc = get_safe_token(device);

	if ( fdc->fdc_sel != ( ( data >> 3 ) & 15 ) )
		fdc->fdc_rdy = 0;

	fdc->fdc_sel = ( data >> 3 ) & 15;
    fdc->fdc_side = 1 - ( ( data >> 2 ) & 1 );
	fdc->fdc_dir = ( data >> 1 ) & 1;

	step_pulse = data & 1;

    if ( fdc->fdc_step != step_pulse ) {
		fdc->fdc_step = step_pulse;

    	if ( fdc->fdc_step == 0 ) {
		    for ( drive = 0; drive < NUM_DRIVES; drive++ ) {
				if ( !( fdc->fdc_sel & ( 1 << drive ) ) )
				    fdc_stepdrive( device, drive );
			}
		}
	}

	for ( drive = 0; drive < NUM_DRIVES; drive++ ) {
		if ( !( fdc->fdc_sel & ( 1 << drive ) ) ) {
			fdc_motor( device, drive, ( data >> 7 ) & 1 );
			fdc_setup_leds( device,drive );
		}
    }
}

UINT8  amiga_fdc_status_r (device_t *device)
{
	int drive = -1, ret = 0x3c;
	amiga_fdc_t *fdc = get_safe_token(device);
	for ( drive = 0; drive < NUM_DRIVES; drive++ ) {
		if ( !( fdc->fdc_sel & ( 1 << drive ) ) ) {

			if ( fdc->fdc_status[drive].motor_on ) {
				if ( fdc->fdc_rdy ) ret &= ~0x20;
				fdc->fdc_rdy = 1;
			} else {
				/* return drive id */
				ret &= ~0x20;
			}

			if ( fdc->fdc_status[drive].cyl == 0 )
				ret &= ~0x10;

			if ( fdc->fdc_status[drive].wprot )
				ret &= ~0x08;

			if ( fdc->fdc_status[drive].disk_changed )
				ret &= ~0x04;

			return ret;
		}
	}

	return ret;
}

static const floppy_config amiga_floppy_config =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_3_5_DSHD,
	FLOPPY_OPTIONS_NAME(amiga_only),
	NULL
};

static MACHINE_CONFIG_FRAGMENT( amiga_fdc )
	MCFG_FLOPPY_2_DRIVES_ADD(amiga_floppy_config)
MACHINE_CONFIG_END


DEVICE_GET_INFO( amiga_fdc )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;												break;
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(amiga_fdc_t);								break;

		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_MACHINE_CONFIG:				info->machine_config = MACHINE_CONFIG_NAME(amiga_fdc);		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(amiga_fdc);					break;
		case DEVINFO_FCT_STOP:							/* Nothing */												break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(amiga_fdc);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Amiga FDC");								break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Amiga FDC");								break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");										break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);									break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright the MESS Team"); 				break;
	}
}

DEFINE_LEGACY_DEVICE(AMIGA_FDC, amiga_fdc);
