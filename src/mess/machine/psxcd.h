#ifndef _included_psxcdrom_
#define _included_psxcdrom_

#include "imagedev/chd_cd.h"
#include "psxcddrv.h"

#define MAX_PSXCD_TIMERS    (4)

class event;

//
//
//

const int num_commands=0x20;

//
//
//

#define PSXCD_TAG   "psxcd"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PSXCD_ADD(_devname) \
	MCFG_DEVICE_ADD(PSXCD_TAG, PSXCD, 0) \
	MCFG_PSXCD_DEVNAME(_devname)

#define MCFG_PSXCD_IRQ_HANDLER(_devcb) \
	devcb = &psxcd_device::set_irq_handler(*device, DEVCB2_##_devcb);

#define MCFG_PSXCD_DEVNAME(_name) \
	psxcd_device::static_set_devname(*device, _name);

struct psxcd_interface
{
};

class psxcd_device : public device_t,
						public psxcd_interface
{
public:

	psxcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb2_base &set_irq_handler(device_t &device, _Object object) { return downcast<psxcd_device &>(device).m_irq_handler.set_callback(object); }
	static void static_set_devname(device_t &device, const char *devname);
private:
	struct command_result
	{
		unsigned char data[32], sz, res;
		command_result *next;
	};

	static const unsigned int sector_buffer_size=16, default_irq_delay=16000;   //480;  //8000; //2000<<2;

	unsigned char sr,res,ir,cmdmode,
								cmdbuf[64],*cbp,
								mode,
								loc[3],
								curpos[3],
								secbuf[raw_sector_size*sector_buffer_size],
								*secptr,
								filter_file,
								filter_channel,
								lastsechdr[8],
								status;
	int rdp,secsize,secleft,secskip,
			sechead,sectail,
			secin;
	command_result *res_queue,
									*cur_res;
	bool open,
				streaming,
				first_open;
	event *next_read_event;
	INT64 next_sector_t;
	unsigned int autopause_sector,
								xa_prefetch_sector,
								cdda_prefetch_sector;

	cdrom_driver *driver;

	unsigned int start_read_delay,
								read_sector_cycles,
								preread_delay;

	void write_command(const unsigned char byte);

	struct command_info
	{
		void (psxcd_device::*func)();
		const char *name;
	};

	void cdcmd_sync();
	void cdcmd_nop();
	void cdcmd_setloc();
	void cdcmd_play();
	void cdcmd_forward();
	void cdcmd_backward();
	void cdcmd_readn();
	void cdcmd_standby();
	void cdcmd_stop();
	void cdcmd_pause();
	void cdcmd_init();
	void cdcmd_mute();
	void cdcmd_demute();
	void cdcmd_setfilter();
	void cdcmd_setmode();
	void cdcmd_getparam();
	void cdcmd_getlocl();
	void cdcmd_getlocp();
	void cdcmd_illegal();
	void cdcmd_gettn();
	void cdcmd_gettd();
	void cdcmd_seekl();
	void cdcmd_seekp();
	void cdcmd_test();
	void cdcmd_id();
	void cdcmd_reads();
	void cdcmd_reset();
	void cdcmd_readtoc();

	static command_info cmd_table[num_commands];

	void cmd_complete(command_result *res);
	void add_result(command_result *res);
	event *send_result(const unsigned int res,
											const unsigned char *data=NULL,
											const unsigned int sz=0,
											const unsigned int delay=default_irq_delay);

	void start_read();
	void start_play();
	void start_streaming();
	void stop_read();
	void read_sector();
	void prefetch_next_sector();
	bool read_next_sector(const bool block=true);
	bool play_cdda_sector(const unsigned int sector, unsigned char *rawsec);
	void play_sector();
	void preread_sector();

public:
	void set_driver(cdrom_driver *d);
	cdrom_driver *get_driver() const { return driver; }

	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void start_dma(UINT8 *mainram, UINT32 size);

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

private:
	emu_timer *m_timer;
	UINT32 m_sysclock;
	const char *m_devname;
	cdrom_image_device *m_cddevice;
	cdrom_file  *m_cd;
	emu_timer *m_timers[MAX_PSXCD_TIMERS];
	event *m_eventfortimer[MAX_PSXCD_TIMERS];
	bool m_timerinuse[MAX_PSXCD_TIMERS];

	void add_system_event(event *ev);

	devcb2_write_line m_irq_handler;
};


// miniature version of pSX's event class, which we emulate with device timers
const int max_event_data=64;

class event
{
public:
	UINT64 t;               // expire time
	unsigned int type;
	unsigned char data[max_event_data];
	emu_timer *timer;
};


// device type definition
extern const device_type PSXCD;

#endif
