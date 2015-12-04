// license:BSD-3-Clause
// copyright-holders:smf,R. Belmont,pSXAuthor,Carl
#ifndef _included_psxcdrom_
#define _included_psxcdrom_

#include "imagedev/chd_cd.h"
#include "sound/spu.h"

#define MAX_PSXCD_TIMERS    (4)

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PSXCD_ADD(_tag, _devname) \
	MCFG_DEVICE_ADD(_tag, PSXCD, 0)

#define MCFG_PSXCD_IRQ_HANDLER(_devcb) \
	devcb = &psxcd_device::set_irq_handler(*device, DEVCB_##_devcb);

class psxcd_device : public cdrom_image_device
{
public:
	psxcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<psxcd_device &>(device).m_irq_handler.set_callback(object); }
	static void static_set_devname(device_t &device, const char *devname);
	virtual bool call_load();
	virtual void call_unload();

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	void start_dma(UINT8 *mainram, UINT32 size);

protected:
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual const rom_entry *device_rom_region() const;

private:
	void write_command(UINT8 byte);

	typedef void (psxcd_device::*cdcmd)();
	struct command_result
	{
		UINT8 data[32], sz, res;
		command_result *next;
	};
	union CDPOS {
		UINT8 b[4];
		UINT32 w;
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
	void cdcmd_gettn();
	void cdcmd_gettd();
	void cdcmd_seekl();
	void cdcmd_seekp();
	void cdcmd_test();
	void cdcmd_id();
	void cdcmd_reads();
	void cdcmd_reset();
	void cdcmd_readtoc();
	void cdcmd_unknown12();
	void cdcmd_illegal17();
	void cdcmd_illegal18();
	void cdcmd_illegal1d();

	static const cdcmd cmd_table[31];
	void illegalcmd(UINT8 cmd);

	void cmd_complete(command_result *res);
	void send_result(UINT8 res, UINT8 *data=nullptr, int sz=0, int delay=default_irq_delay, UINT8 errcode = 0);
	command_result *prepare_result(UINT8 res, UINT8 *data=nullptr, int sz=0, UINT8 errcode = 0);

	void start_read();
	void start_play();
	void stop_read();
	void read_sector();
	void play_sector();
	UINT32 sub_loc(CDPOS src1, CDPOS src2);
	int add_system_event(int type, UINT64 t, command_result *ptr);

	UINT8 bcd_to_decimal(const UINT8 bcd) { return ((bcd>>4)*10)+(bcd&0xf); }
	UINT8 decimal_to_bcd(const UINT8 dec) { return ((dec/10)<<4)|(dec%10); }
	UINT32 msf_to_lba_ps(UINT32 msf) { msf = msf_to_lba(msf); return (msf>150)?(msf-150):msf; }
	UINT32 lba_to_msf_ps(UINT32 lba) { return lba_to_msf_alt(lba+150); }

	static const unsigned int sector_buffer_size=16, default_irq_delay=16000;   //480;  //8000; //2000<<2;
	static const unsigned int raw_sector_size=2352;

	UINT8 cmdbuf[64], mode, secbuf[sector_buffer_size][raw_sector_size];
	UINT8 filter_file, filter_channel, lastsechdr[8], status;
	int rdp;
	UINT8 m_cursec, sectail;
	UINT16 m_transcurr;
	UINT8 m_transbuf[raw_sector_size];
	command_result *res_queue, *m_int1;

	struct {
		UINT8 sr, ir, imr;
		struct {
			UINT8 ll, lr, rl, rr;
		} vol;
	} m_regs;

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

	bool open, m_mute, m_dmaload;
	device_timer_id next_read_event;
	INT64 next_sector_t;
	unsigned int autopause_sector, start_read_delay, read_sector_cycles, preread_delay;

	UINT32 m_param_count;
	UINT32 m_sysclock;
	emu_timer *m_timers[MAX_PSXCD_TIMERS];
	bool m_timerinuse[MAX_PSXCD_TIMERS];


	devcb_write_line m_irq_handler;
	cpu_device *m_maincpu;
	spu_device *m_spu;
};

// device type definition
extern const device_type PSXCD;

#endif
