// license:BSD-3-Clause
// copyright-holders:smf,R. Belmont,pSXAuthor,Carl
#ifndef MAME_MACHINE_PSXCD_H
#define MAME_MACHINE_PSXCD_H

#pragma once

#include "imagedev/chd_cd.h"
#include "sound/spu.h"


class psxcd_device : public cdrom_image_device
{
public:
	template <typename T, typename U>
	psxcd_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag, U &&spu_tag)
		: psxcd_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
		m_spu.set_tag(std::forward<U>(spu_tag));
	}

	psxcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }
	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);
	void start_dma(uint8_t *mainram, uint32_t size);

protected:
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	static constexpr unsigned MAX_PSXCD_TIMERS = 4;

	void write_command(uint8_t byte);

	typedef void (psxcd_device::*cdcmd)();
	struct command_result
	{
		uint8_t data[32], sz, res;
		command_result *next;
	};
	union CDPOS {
		uint8_t b[4];
		uint32_t w;
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
	void illegalcmd(uint8_t cmd);

	void cmd_complete(command_result *res);
	void send_result(uint8_t res, uint8_t *data=nullptr, int sz=0, int delay=default_irq_delay, uint8_t errcode = 0);
	command_result *prepare_result(uint8_t res, uint8_t *data=nullptr, int sz=0, uint8_t errcode = 0);

	void start_read();
	void start_play();
	void stop_read();
	void read_sector();
	void play_sector();
	uint32_t sub_loc(CDPOS src1, CDPOS src2);
	int add_system_event(int type, uint64_t t, command_result *ptr);

	uint8_t bcd_to_decimal(const uint8_t bcd) { return ((bcd>>4)*10)+(bcd&0xf); }
	uint8_t decimal_to_bcd(const uint8_t dec) { return ((dec/10)<<4)|(dec%10); }
	uint32_t msf_to_lba_ps(uint32_t msf) { msf = cdrom_file::msf_to_lba(msf); return (msf>150)?(msf-150):msf; }
	uint32_t lba_to_msf_ps(uint32_t lba) { return cdrom_file::lba_to_msf_alt(lba+150); }

	static const unsigned int sector_buffer_size=16, default_irq_delay=16000;   //480;  //8000; //2000<<2;
	static const unsigned int raw_sector_size=2352;

	uint8_t cmdbuf[64]{}, mode = 0, secbuf[sector_buffer_size][raw_sector_size]{};
	uint8_t filter_file = 0, filter_channel = 0, lastsechdr[8]{}, status = 0;
	int rdp = 0;
	uint8_t m_cursec = 0, sectail = 0;
	uint16_t m_transcurr = 0;
	uint8_t m_transbuf[raw_sector_size]{};
	command_result *res_queue = nullptr, *m_int1 = nullptr;

	struct {
		uint8_t sr = 0, ir = 0, imr = 0;
		struct {
			uint8_t ll = 0, lr = 0, rl = 0, rr = 0;
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

	bool open = false, m_mute = false, m_dmaload = false;
	device_timer_id next_read_event{};
	int64_t next_sector_t = 0;
	unsigned int autopause_sector = 0, start_read_delay = 0, read_sector_cycles = 0, preread_delay = 0;

	uint32_t m_param_count = 0;
	uint32_t m_sysclock = 0;
	emu_timer *m_timers[MAX_PSXCD_TIMERS]{};
	bool m_timerinuse[MAX_PSXCD_TIMERS]{};
	command_result *m_results[MAX_PSXCD_TIMERS]{};

	devcb_write_line m_irq_handler;
	required_device<cpu_device> m_maincpu;
	required_device<spu_device> m_spu;
};

// device type definition
DECLARE_DEVICE_TYPE(PSXCD, psxcd_device)

#endif // MAME_MACHINE_PSXCD_H
