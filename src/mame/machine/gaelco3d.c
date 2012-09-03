/***************************************************************************

    Gaelco 3D serial hardware

    Couriersud, early 2010

    Not all lines are fully understood. There is some handshaking going
    on on send which is not fully understood. Those wishing to have a look
    at this:

    Serial send:          0x1fca (radikalb, 68020) 1866e (surfplnt)
    Serial receive:       0x1908 (radikalb, 68020) 185ae

    The receive is interrupt driven (interrupt 6) and the send is
    initiated out of the normal program loop. There is the chance
    of race conditions in mame.

    To run two instances of radikalb on *nix, use the following

    a) Uncomment SHARED_MEM_DRIVER below
    b) Open two terminals
    c) In terminal 1: mkdir /tmp/x1; cd /tmp/x1; /path/to/mame64 -np 2 -mt -rp /mnt/mame/romlib/r -inipath . radikalb -w -nomaximize -inipath .
    d) In terminal 2: mkdir /tmp/x2; cd /tmp/x2; /path/to/mame64 -np 2 -mt -rp /mnt/mame/romlib/r -inipath . radikalb -w -nomaximize -inipath .
    e) Set one instance to be master and one to be slave in service mode
    f) Have fun

***************************************************************************/

#include "emu.h"
#include "gaelco3d.h"

//#define SHARED_MEM_DRIVER      (1)

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _MSC_VER
#include <io.h>
#define	S_IRWXU	(_S_IREAD | _S_IWRITE | _S_IEXEC)
#else
#include <unistd.h>
#endif
#include <errno.h>
#ifdef SHARED_MEM_DRIVER
#include <sys/mman.h>
#endif

#define VERBOSE		(0)
#if VERBOSE
#define LOGMSG(x)	logerror x
#else
#define LOGMSG(x)	do {} while (0);
#endif

/*
 * 115200 seems plausible, radikalb won't work below this speed
 * surfplnt will not work below 460800 ....
 * speedup will not work above 115200
 * 10 bits = 8 data + 1 start + 1 stop
 */

#define LINK_BAUD (115200*4)
//Only for speedup
//#define LINK_BAUD (115200)
#define LINK_BITS 10

#define LINK_FREQ (LINK_BAUD / LINK_BITS)

/* Sync up the instances 8 times for each byte transfered */
#define SYNC_MULT (4)

#define SYNC_FREQ (15000000 / 20) //(LINK_FREQ * SYNC_MULT)

/* allow for slight differences in execution speed */

#define LINK_SLACK ((SYNC_MULT / 4) + 1)
#define LINK_SLACK_B ((LINK_SLACK / 3) + 1)


typedef struct _buf_t buf_t;
struct _buf_t
{
	volatile UINT8 data;
	volatile UINT8 stat;
	volatile int cnt;
	volatile int data_cnt;
};

typedef struct _shmem_t shmem_t;
struct _shmem_t
{
	volatile INT32	lock;
	buf_t				buf[2];
};

typedef struct _osd_shared_mem osd_shared_mem;

typedef struct _gaelco_serial_state gaelco_serial_state;
struct _gaelco_serial_state
{
	device_t *m_device;
	devcb_resolved_write_line m_irq_func;

	UINT8 m_status;
	int m_last_in_msg_cnt;
	int m_slack_cnt;

	emu_timer *m_sync_timer;

	buf_t *m_in_ptr;
	buf_t *m_out_ptr;
	osd_shared_mem *m_os_shmem;
	shmem_t *m_shmem;
};

struct _osd_shared_mem
{
	char *fn;
	size_t size;
	void *ptr;
	int creator;
};

/* code below currently works on unix only */
#ifdef SHARED_MEM_DRIVER
static osd_shared_mem *osd_sharedmem_alloc(const char *path, int create, size_t size)
{
	int fd;
	osd_shared_mem *os_shmem = (osd_shared_mem *) osd_malloc(sizeof(osd_shared_mem));

	if (create)
	{
		char *buf = (char *) osd_malloc_array(size);
		memset(buf,0, size);

		fd = open(path, O_RDWR | O_CREAT, S_IRWXU);
		write(fd, buf, size);
		os_shmem->creator = 1;
	}
	else
	{
		fd = open(path, O_RDWR);
		if (fd == -1)
		{
			osd_free(os_shmem);
			return NULL;
		}
		os_shmem->creator = 0;
	}
	os_shmem->fn = (char *) osd_malloc_array(strlen(path)+1);
	strcpy(os_shmem->fn, path);

	assert(fd != -1);

	os_shmem->ptr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	os_shmem->size = size;
	close(fd);
	return os_shmem;
}

static void osd_sharedmem_free(osd_shared_mem *os_shmem)
{
	munmap(os_shmem->ptr, os_shmem->size);
	if (os_shmem->creator)
		unlink(os_shmem->fn);
	osd_free(os_shmem->fn);
	osd_free(os_shmem);
}

static void *osd_sharedmem_ptr(osd_shared_mem *os_shmem)
{
	return os_shmem->ptr;
}
#else
static osd_shared_mem *osd_sharedmem_alloc(const char *path, int create, size_t size)
{
	osd_shared_mem *os_shmem = (osd_shared_mem *) osd_malloc(sizeof(osd_shared_mem));

	os_shmem->creator = 0;

	os_shmem->ptr = (void *) osd_malloc_array(size);
	os_shmem->size = size;
	return os_shmem;
}

static void osd_sharedmem_free(osd_shared_mem *os_shmem)
{
	osd_free(os_shmem->ptr);
	osd_free(os_shmem);
}

static void *osd_sharedmem_ptr(osd_shared_mem *os_shmem)
{
	return os_shmem->ptr;
}
#endif

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - convert a device's token
    into a gaelco_serial_state
-------------------------------------------------*/

INLINE gaelco_serial_state *get_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == GAELCO_SERIAL);
	return (gaelco_serial_state *) downcast<gaelco_serial_device *>(device)->token();
}

INLINE const gaelco_serial_interface *get_interface(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == GAELCO_SERIAL);
	return (gaelco_serial_interface *) downcast<gaelco_serial_device *>(device)->static_config();
}

INLINE void shmem_lock(shmem_t *shmem)
{
	while (atomic_exchange32(&shmem->lock,1) == 0)
		;
}

INLINE void shmem_unlock(shmem_t *shmem)
{
	atomic_exchange32(&shmem->lock,0);
}

/***************************************************************************
    STATIC FUNCTIONS
***************************************************************************/


static TIMER_CALLBACK( set_status_cb )
{
	gaelco_serial_state *state = (gaelco_serial_state *) ptr;
	UINT8 mask = param >> 8;
	UINT8 set = param & 0xff;

	state->m_status &= mask;
	state->m_status |= set;
}

static void set_status(gaelco_serial_state *state, UINT8 mask, UINT8 set, int wait)
{
	state->m_device->machine().scheduler().timer_set(attotime::from_hz(wait), FUNC(set_status_cb), (mask << 8)|set,
			state);
}

static void process_in(gaelco_serial_state *state)
{
	int t;

	if ((state->m_in_ptr->stat & GAELCOSER_STATUS_RESET) != 0)
		state->m_out_ptr->cnt = 0;

	/* new data available ? */
	t = state->m_in_ptr->data_cnt;
	if (t != state->m_last_in_msg_cnt)
	{
		state->m_last_in_msg_cnt = t;
		if (state->m_in_ptr->cnt > 10)
		{
			state->m_status &= ~GAELCOSER_STATUS_READY;
			LOGMSG(("command receive %02x at %d (%d)\n", state->m_in_ptr->data, state->m_out_ptr->cnt, state->m_in_ptr->cnt));
			if ((state->m_status & GAELCOSER_STATUS_IRQ_ENABLE) != 0)
			{
				state->m_irq_func(1);
				LOGMSG(("irq!\n"));
			}
		}
	}
}

static void sync_link(gaelco_serial_state *state)
{
	volatile buf_t *buf = state->m_in_ptr;
	int breakme = 1;
	do
	{
		shmem_lock(state->m_shmem);
		process_in(state);
		/* HACK: put some timing noise on the line */
		if (buf->cnt + state->m_slack_cnt > state->m_out_ptr->cnt)
			breakme = 0;
		/* stop if not connected .. */
		if ((state->m_out_ptr->stat & GAELCOSER_STATUS_RESET) != 0)
			breakme = 0;
		shmem_unlock(state->m_shmem);
	} while (breakme);

	state->m_slack_cnt++;
	state->m_slack_cnt = (state->m_slack_cnt % LINK_SLACK) + LINK_SLACK_B;

	shmem_lock(state->m_shmem);
	state->m_out_ptr->stat &= ~GAELCOSER_STATUS_RESET;
	shmem_unlock(state->m_shmem);
}

static TIMER_CALLBACK( link_cb )
{
	gaelco_serial_state *state = (gaelco_serial_state *) ptr;

	shmem_lock(state->m_shmem);
	state->m_out_ptr->cnt++;
	sync_link(state);
	shmem_unlock(state->m_shmem);
}


/***************************************************************************
    INTERFACE FUNCTIONS
***************************************************************************/



WRITE8_DEVICE_HANDLER( gaelco_serial_irq_enable )
{
	LOGMSG(("???? irq enable %d\n", data));
}

READ8_DEVICE_HANDLER( gaelco_serial_status_r)
{
	gaelco_serial_state *serial = get_token(device);
	UINT8 ret = 0;

	shmem_lock(serial->m_shmem);
	process_in(serial);
	if ((serial->m_status & GAELCOSER_STATUS_READY) != 0)
		ret |= 0x01;
	if ((serial->m_in_ptr->stat & GAELCOSER_STATUS_RTS) != 0)
		ret |= 0x02;
	shmem_unlock(serial->m_shmem);
	return ret;
}

WRITE8_DEVICE_HANDLER( gaelco_serial_data_w)
{
	gaelco_serial_state *serial = get_token(device);

	shmem_lock(serial->m_shmem);

	serial->m_out_ptr->data = data;
	serial->m_status &= ~GAELCOSER_STATUS_READY;
	serial->m_out_ptr->data_cnt++;

	set_status(serial, ~GAELCOSER_STATUS_READY, GAELCOSER_STATUS_READY, LINK_FREQ );

	shmem_unlock(serial->m_shmem);
	LOGMSG(("command send %02x at %d\n", data, serial->m_out_ptr->cnt));
}

READ8_DEVICE_HANDLER( gaelco_serial_data_r)
{
	gaelco_serial_state *serial = get_token(device);
	UINT8 ret;

	shmem_lock(serial->m_shmem);
	process_in(serial);
	ret = (serial->m_in_ptr->data & 0xff);

	serial->m_irq_func(0);
	LOGMSG(("read %02x at %d (%d)\n", ret, serial->m_out_ptr->cnt, serial->m_in_ptr->cnt));

	/* if we are not sending, mark as as ready */
	if ((serial->m_status & GAELCOSER_STATUS_SEND) == 0)
		serial->m_status |= GAELCOSER_STATUS_READY;

	shmem_unlock(serial->m_shmem);
	return ret;
}

WRITE8_DEVICE_HANDLER( gaelco_serial_unknown_w)
{
	gaelco_serial_state *serial = get_token(device);

	shmem_lock(serial->m_shmem);
	LOGMSG(("???? unknown serial access %d\n", data));
	shmem_unlock(serial->m_shmem);

}

WRITE8_DEVICE_HANDLER( gaelco_serial_rts_w )
{
	gaelco_serial_state *serial = get_token(device);

	shmem_lock(serial->m_shmem);

	if (data == 0)
		serial->m_out_ptr->stat |= GAELCOSER_STATUS_RTS;
	else
	{
		//Commented out for now
		//serial->m_status |= GAELCOSER_STATUS_READY;
		serial->m_out_ptr->stat &= ~GAELCOSER_STATUS_RTS;
	}

	shmem_unlock(serial->m_shmem);
}

WRITE8_DEVICE_HANDLER( gaelco_serial_tr_w)
{
	gaelco_serial_state *serial = get_token(device);

	LOGMSG(("set transmit %d\n", data));
	shmem_lock(serial->m_shmem);
	if ((data & 0x01) != 0)
		serial->m_status |= GAELCOSER_STATUS_SEND;
	else
		serial->m_status &= ~GAELCOSER_STATUS_SEND;

	shmem_unlock(serial->m_shmem);
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

#define PATH_NAME "/tmp/gaelco_serial"

static DEVICE_START( gaelco_serial )
{
	gaelco_serial_state *state = get_token(device);
	const gaelco_serial_interface *intf = get_interface(device);

	/* validate arguments */
	assert(device != NULL);
	assert(strlen(device->tag()) < 20);

	/* clear out CIA structure, and copy the interface */
	memset(state, 0, sizeof(*state));
	state->m_device = device;

	state->m_irq_func.resolve(intf->irq_func, *device);
	state->m_sync_timer = device->machine().scheduler().timer_alloc(FUNC(link_cb), state);

	/* register for save states */
	//device->save_item(NAME(earom->offset));
	//device->save_item(NAME(earom->data));

#ifdef SHARED_MEM_DRIVER
	state->m_sync_timer->adjust(attotime::zero,0,attotime::from_hz(SYNC_FREQ));
#endif

	state->m_os_shmem = osd_sharedmem_alloc(PATH_NAME, 0, sizeof(shmem_t));
	if (state->m_os_shmem == NULL)
	{
		state->m_os_shmem = osd_sharedmem_alloc(PATH_NAME, 1, sizeof(shmem_t));
		state->m_shmem = (shmem_t *) osd_sharedmem_ptr(state->m_os_shmem);

		state->m_in_ptr = &state->m_shmem->buf[0];
		state->m_out_ptr = &state->m_shmem->buf[1];
	}
	else
	{
		state->m_shmem = (shmem_t *) osd_sharedmem_ptr(state->m_os_shmem);
		state->m_in_ptr = &state->m_shmem->buf[1];
		state->m_out_ptr = &state->m_shmem->buf[0];
	}
}

static void buf_reset(buf_t *buf)
{
	buf->stat = GAELCOSER_STATUS_RTS| GAELCOSER_STATUS_RESET;
	buf->data = 0;
	buf->data_cnt = -1;
	buf->cnt = 0;
}

static DEVICE_RESET( gaelco_serial )
{
	gaelco_serial_state *state = get_token(device);

	state->m_status = GAELCOSER_STATUS_READY	|GAELCOSER_STATUS_IRQ_ENABLE ;

	state->m_last_in_msg_cnt = -1;
	state->m_slack_cnt = LINK_SLACK_B;

	shmem_lock(state->m_shmem);
	buf_reset(state->m_out_ptr);
	buf_reset(state->m_in_ptr);
	shmem_unlock(state->m_shmem);
}

static DEVICE_STOP( gaelco_serial )
{
	gaelco_serial_state *state = get_token(device);

	shmem_lock(state->m_shmem);
	buf_reset(state->m_out_ptr);
	buf_reset(state->m_in_ptr);
	shmem_unlock(state->m_shmem);

	osd_sharedmem_free(state->m_os_shmem);
}

const device_type GAELCO_SERIAL = &device_creator<gaelco_serial_device>;

gaelco_serial_device::gaelco_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GAELCO_SERIAL, "gaelco_serial", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(gaelco_serial_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void gaelco_serial_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gaelco_serial_device::device_start()
{
	DEVICE_START_NAME( gaelco_serial )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void gaelco_serial_device::device_reset()
{
	DEVICE_RESET_NAME( gaelco_serial )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void gaelco_serial_device::device_stop()
{
	DEVICE_STOP_NAME( gaelco_serial )(this);
}


