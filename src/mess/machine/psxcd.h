#ifndef _included_psxcdrom_
#define _included_psxcdrom_

#include "imagedev/chd_cd.h"
#include "sound/spu.h"

#define MAX_PSXCD_TIMERS    (4)
const int num_commands=0x20;

//
//
//

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PSXCD_ADD(_tag, _devname) \
	MCFG_DEVICE_ADD(_tag, PSXCD, 0)

#define MCFG_PSXCD_IRQ_HANDLER(_devcb) \
	devcb = &psxcd_device::set_irq_handler(*device, DEVCB2_##_devcb);

class psxcd_device : public cdrom_image_device
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
	static const unsigned int raw_sector_size=2352;

	unsigned char sr,res,ir,cmdmode,
								cmdbuf[64],*cbp,
								mode,
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

	union CDPOS {
		UINT8 b[4];
		UINT32 w;
	};
	CDPOS loc, curpos;

#ifdef LSB_FIRST
	enum {
		M = 2,
		S = 1,
		F = 0
	};
#else
	enum {
		M = 1,
		S = 2,
		F = 3
	};
#endif

	bool open,
				streaming,
				first_open;
	device_timer_id next_read_event;
	INT64 next_sector_t;
	unsigned int autopause_sector,
								xa_prefetch_sector,
								cdda_prefetch_sector;

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
	void send_result(const unsigned int res,
											const unsigned char *data=NULL,
											const unsigned int sz=0,
											const unsigned int delay=default_irq_delay);

	void start_read();
	void start_play();
	void start_streaming();
	void stop_read();
	void read_sector();
	bool read_next_sector();
	bool play_cdda_sector(const unsigned int sector, unsigned char *rawsec);
	void play_sector();
	void preread_sector();
	UINT32 sub_loc(CDPOS src1, CDPOS src2);

public:

	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual bool call_load();
	virtual void call_unload();

	void start_dma(UINT8 *mainram, UINT32 size);

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

private:
	UINT32 m_param_count;
	UINT32 m_sysclock;
	emu_timer *m_timers[MAX_PSXCD_TIMERS];
	bool m_timerinuse[MAX_PSXCD_TIMERS];

	int add_system_event(int type, UINT64 t, void *ptr);

	devcb2_write_line m_irq_handler;
	cpu_device *m_maincpu;
	spu_device *m_spu;

	UINT8 bcd_to_decimal(const UINT8 bcd) { return ((bcd>>4)*10)+(bcd&0xf); }
	UINT8 decimal_to_bcd(const UINT8 dec) { return ((dec/10)<<4)|(dec%10); }
	UINT32 msf_to_lba_ps(UINT32 msf) { msf = msf_to_lba(msf); return (msf>150)?(msf-150):msf; }
	UINT32 lba_to_msf_ps(UINT32 lba) { return lba_to_msf_alt(lba+150); }

};

// device type definition
extern const device_type PSXCD;

#endif
