// license:BSD-3-Clause
// copyright-holders:smf,R. Belmont,pSXAuthor,Carl
#include "emu.h"
#include "psxcd.h"
#include "debugger.h"

#define LOG_CMD  (1U << 1)
#define LOG_MISC (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

enum cdrom_events
{
	EVENT_CMD_COMPLETE,
	EVENT_READ_SECTOR,
	EVENT_PLAY_SECTOR,
	EVENT_CHANGE_DISK
};

enum intr_status
{
	INTR_NOINTR,
	INTR_DATAREADY,
	INTR_ACKNOWLEDGE,
	INTR_COMPLETE,
	INTR_DATAEND,
	INTR_DISKERROR
};

enum mode_flags
{
	MODE_DOUBLE_SPEED = 0x80,
	MODE_ADPCM = 0x40,
	MODE_SIZE = 0x20,
	MODE_SIZE2 = 0x10,
	MODE_SIZE_MASK = 0x30,
	MODE_CHANNEL = 0x08,
	MODE_REPORT = 0x04,
	MODE_AUTOPAUSE = 0x02,
	MODE_CDDA = 0x01
};

enum status_f
{
	STATUS_PLAYING = 0x80,
	STATUS_SEEKING = 0x40,
	STATUS_READING = 0x20,
	STATUS_SHELLOPEN = 0x10,
	STATUS_INVALID = 0x08,
	STATUS_SEEKERROR = 0x04,
	STATUS_STANDBY = 0x02,
	STATUS_ERROR = 0x01
};

struct subheader
{
	uint8_t file;
	uint8_t channel;
	uint8_t submode;
	uint8_t coding;
};

enum submode_flags
{
	SUBMODE_EOF = 0x80,
	SUBMODE_REALTIME = 0x40,
	SUBMODE_FORM = 0x20,
	SUBMODE_TRIGGER = 0x10,
	SUBMODE_DATA = 0x08,
	SUBMODE_AUDIO = 0x04,
	SUBMODE_VIDEO = 0x02,
	SUBMODE_EOR = 0x01
};

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSXCD, psxcd_device, "psx_cd", "PSX CD-ROM")

psxcd_device::psxcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cdrom_image_device(mconfig, PSXCD, tag, owner, clock),
	m_irq_handler(*this),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_spu(*this, finder_base::DUMMY_TAG)
{
	set_interface("cdrom");
}


void psxcd_device::device_start()
{
	cdrom_image_device::device_start();

	uint32_t sysclk = m_maincpu->clock() / 2;
	start_read_delay = (sysclk / 60);
	read_sector_cycles = (sysclk / 75);
	preread_delay = (read_sector_cycles >> 2) - 500;

	m_sysclock = sysclk;

	res_queue = nullptr;
	rdp = 0;
	status = STATUS_SHELLOPEN;
	mode = 0;

	for (int i = 0; i < MAX_PSXCD_TIMERS; i++)
	{
		m_timers[i] = timer_alloc(FUNC(psxcd_device::handle_event), this);
		m_timerinuse[i] = false;
		m_results[i] = nullptr;
	}

	save_item(NAME(cmdbuf));
	save_item(NAME(mode));
	save_item(NAME(secbuf));
	save_item(NAME(filter_file));
	save_item(NAME(filter_channel));
	save_item(NAME(lastsechdr));
	save_item(NAME(status));
	save_item(NAME(rdp));
	save_item(NAME(m_cursec));
	save_item(NAME(sectail));
	save_item(NAME(m_transcurr));
	save_item(NAME(m_transbuf));
	save_item(NAME(loc.w));
	save_item(NAME(curpos.w));
	save_item(NAME(open));
	save_item(NAME(m_mute));
	save_item(NAME(m_dmaload));
	save_item(NAME(next_read_event));
	save_item(NAME(next_sector_t));
	save_item(NAME(autopause_sector));
	save_item(NAME(m_param_count));
}

void psxcd_device::device_stop()
{
	for (int i = 0; i < MAX_PSXCD_TIMERS; i++)
	{
		if (m_timerinuse[i] && m_results[i])
			delete m_results[i];
	}
	while (res_queue)
	{
		command_result *res = res_queue->next;
		delete res_queue;
		res_queue = res;
	}
}

void psxcd_device::device_reset()
{
	next_read_event = -1;
	stop_read();

	for (int i = 0; i < MAX_PSXCD_TIMERS; i++)
	{
		if (m_timerinuse[i] && m_results[i])
			delete m_results[i];
		m_timers[i]->adjust(attotime::never);
		m_timerinuse[i] = false;
	}
	open = true;
	if (m_cdrom_handle)
		add_system_event(EVENT_CHANGE_DISK, m_sysclock, nullptr);

	while (res_queue)
	{
		command_result *res = res_queue->next;
		delete res_queue;
		res_queue = res;
	}

	m_param_count = 0;
	m_regs.sr = 0x18;
	m_regs.ir = 0;
	m_regs.imr = 0x1f;
	sectail = 0;
	m_cursec = 0;
	m_mute = false;
	m_dmaload = false;
	m_int1 = nullptr;
	curpos.w = 0;
}

std::pair<std::error_condition, std::string> psxcd_device::call_load()
{
	auto ret = cdrom_image_device::call_load();
	open = true;
	if (!ret.first)
		add_system_event(EVENT_CHANGE_DISK, m_sysclock, nullptr); // 1 sec to spin up the disk
	return ret;
}

void psxcd_device::call_unload()
{
	stop_read();
	cdrom_image_device::call_unload();
	open = true;
	status = STATUS_SHELLOPEN;
	send_result(INTR_DISKERROR);
}

uint8_t psxcd_device::read(offs_t offset)
{
	uint8_t ret = 0;
	switch (offset & 3)
	{
		/*
		x--- ---- command/parameter busy flag
		-x-- ---- data fifo full (active low)
		--x- ---- response fifo empty (active low)
		---x ---- parameter fifo full (active low)
		---- x--- parameter fifo empty (active high)
		---- --xx cmd mode
		*/
		case 0:
			ret = m_regs.sr;
			break;

		case 1:
			if ((res_queue) && (rdp < res_queue->sz))
			{
				ret = res_queue->data[rdp++];
				if (rdp == res_queue->sz)
					m_regs.sr &= ~0x20;
				else
					m_regs.sr |= 0x20;
			}
			else
				ret = 0;
			break;

		case 2:
			if (!m_dmaload)
				ret = 0;
			else
			{
				ret = m_transbuf[m_transcurr++];
				if (m_transcurr >= raw_sector_size)
				{
					m_dmaload = false;
					m_regs.sr &= ~0x40;
				}
			}
			break;

		case 3:
			if (m_regs.sr & 1)
				ret = m_regs.ir | 0xe0;
			else
				ret = m_regs.imr | 0xe0;
			break;
	}

	LOGMASKED(LOG_MISC, "%s: read byte %08x = %02x\n", machine().describe_context(), offset, ret);

	return ret;
}

void psxcd_device::write(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MISC, "%s: write byte %08x = %02x\n", machine().describe_context(), offset, data);

	switch ((offset & 3) | ((m_regs.sr & 3) << 4))
	{
		case 0x00:
		case 0x10:
		case 0x20:
		case 0x30:
			m_regs.sr = (m_regs.sr & ~3) | (data & 3);
			break;

		case 0x01:
			write_command(data);
			break;

		case 0x11:
		case 0x21:
			break;

		case 0x31:
			m_regs.vol.rr = data;
			break;

		case 0x02:
			cmdbuf[m_param_count] = data;
			m_param_count++;
			break;

		case 0x12:
			m_regs.imr = data & 0x1f;
			break;

		case 0x22:
			m_regs.vol.ll = data;
			break;

		case 0x32:
			m_regs.vol.rl = data;
			break;
		/*
		x--- ---- unknown
		-x-- ---- Reset parameter FIFO
		--x- ---- unknown (used on transitions, so it certainly resets something)
		---x ---- Command start
		---- -xxx Response received
		*/
		case 0x03:
			if ((data & 0x80) && !m_dmaload)
			{
				m_dmaload = true;
				memcpy(m_transbuf, secbuf[m_cursec], raw_sector_size);
				m_regs.sr |= 0x40;

				m_cursec++;
				m_cursec %= sector_buffer_size;

				switch (mode & MODE_SIZE_MASK)
				{
					case 0x00:
					default:
						m_transcurr = 24;
						break;
					case 0x10:
						m_transcurr = 24;
						break;
					case 0x20:
						m_transcurr = 12;
						break;
				}
				if (VERBOSE & LOG_CMD)
				{
					char str[1024];
					for (int i=0; i<12; i++)
						sprintf(&str[i*4], "%02x  ", m_transbuf[i+12]);
					LOGMASKED(LOG_CMD, "%s: request data=%s\n", machine().describe_context(), str);
				}
			}
			else if (!(data & 0x80))
			{
				m_dmaload = false;
				m_regs.sr &= ~0x40;
			}
			break;

		case 0x13:
			if (data & 0x1f)
			{
				m_regs.ir &= ~(data & 0x1f);

				if (res_queue && !m_regs.ir)
				{
					command_result *res = res_queue;
					if (res == m_int1)
						m_int1 = nullptr;

					res_queue = res->next;
					delete res;
					m_regs.sr &= ~0x20;
					rdp = 0;
					if (res_queue)
					{
						m_regs.sr |= 0x20;
						m_regs.ir = res_queue->res;
					}
					LOGMASKED(LOG_CMD, "%s: nextres\n", machine().describe_context());
				}
			}
			if (data & 0x40)
				m_param_count = 0;
			break;

		case 0x23:
			m_regs.vol.lr = data;
			break;

		case 0x33:
			break;
	}
}

const psxcd_device::cdcmd psxcd_device::cmd_table[]=
{
	&psxcd_device::cdcmd_sync,
	&psxcd_device::cdcmd_nop,
	&psxcd_device::cdcmd_setloc,
	&psxcd_device::cdcmd_play,
	&psxcd_device::cdcmd_forward,
	&psxcd_device::cdcmd_backward,
	&psxcd_device::cdcmd_readn,
	&psxcd_device::cdcmd_standby,
	&psxcd_device::cdcmd_stop,
	&psxcd_device::cdcmd_pause,
	&psxcd_device::cdcmd_init,
	&psxcd_device::cdcmd_mute,
	&psxcd_device::cdcmd_demute,
	&psxcd_device::cdcmd_setfilter,
	&psxcd_device::cdcmd_setmode,
	&psxcd_device::cdcmd_getparam,
	&psxcd_device::cdcmd_getlocl,
	&psxcd_device::cdcmd_getlocp,
	&psxcd_device::cdcmd_unknown12,
	&psxcd_device::cdcmd_gettn,
	&psxcd_device::cdcmd_gettd,
	&psxcd_device::cdcmd_seekl,
	&psxcd_device::cdcmd_seekp,
	&psxcd_device::cdcmd_illegal17,
	&psxcd_device::cdcmd_illegal18,
	&psxcd_device::cdcmd_test,
	&psxcd_device::cdcmd_id,
	&psxcd_device::cdcmd_reads,
	&psxcd_device::cdcmd_reset,
	&psxcd_device::cdcmd_illegal1d,
	&psxcd_device::cdcmd_readtoc
};

void psxcd_device::write_command(uint8_t byte)
{
	if (byte > 30)   // coverity 140157
		illegalcmd(byte);
	else
		(this->*cmd_table[byte])();
	m_param_count = 0;
}

void psxcd_device::cdcmd_sync()
{
	LOGMASKED(LOG_CMD, "%s: sync\n", machine().describe_context());

	stop_read();
	send_result(INTR_ACKNOWLEDGE);
}

void psxcd_device::cdcmd_nop()
{
	LOGMASKED(LOG_CMD, "%s: nop\n", machine().describe_context());

	if (!open)
		status &= ~STATUS_SHELLOPEN;

	send_result(INTR_COMPLETE);
}

void psxcd_device::cdcmd_setloc()
{
	LOGMASKED(LOG_CMD, "%s: setloc %08x:%08x:%08x\n", machine().describe_context(), cmdbuf[0], cmdbuf[1], cmdbuf[2]);

	stop_read();


	CDPOS l;
	l.b[M]=bcd_to_decimal(cmdbuf[0]);
	l.b[S]=bcd_to_decimal(cmdbuf[1]);
	l.b[F]=bcd_to_decimal(cmdbuf[2]);

	if ((l.b[M]>0) || (l.b[S]>=2))
		loc.w=l.w;
	else
		logerror("%s: setloc out of range: %02d:%02d:%02d\n", machine().describe_context(), l.b[M], l.b[S], l.b[F]);

	send_result(INTR_COMPLETE);
}

void psxcd_device::cdcmd_play()
{
	if (cmdbuf[0] && m_param_count)
			loc.w = lba_to_msf_ps(m_cdrom_handle->get_track_start(bcd_to_decimal(cmdbuf[0]) - 1));

	curpos.w = loc.w;
	if (!curpos.w)
		curpos.b[S] = 2;

	LOGMASKED(LOG_CMD, "%s: play %02x %02x %02x => %d\n", machine().describe_context(), decimal_to_bcd(loc.b[M]), decimal_to_bcd(loc.b[S]), decimal_to_bcd(loc.b[F]), msf_to_lba_ps(loc.w));

	stop_read();
	start_play();
	send_result(INTR_COMPLETE);
}

void psxcd_device::cdcmd_forward()
{
	LOGMASKED(LOG_CMD, "%s: forward\n", machine().describe_context());
}

void psxcd_device::cdcmd_backward()
{
	LOGMASKED(LOG_CMD, "%s: backward\n", machine().describe_context());
}

void psxcd_device::cdcmd_readn()
{
	if (!open)
	{
		LOGMASKED(LOG_CMD, "%s: readn\n", machine().describe_context());

		curpos.w=loc.w;

		stop_read();
		start_read();
	}
	else
	{
		send_result(INTR_DISKERROR, nullptr, 0, 0x80);
	}
}

void psxcd_device::cdcmd_standby()
{
	LOGMASKED(LOG_CMD, "%s: standby\n", machine().describe_context());

	stop_read();
	send_result(INTR_ACKNOWLEDGE);
}

void psxcd_device::cdcmd_stop()
{
	LOGMASKED(LOG_CMD, "%s: stop\n", machine().describe_context());

	stop_read();
	send_result(INTR_ACKNOWLEDGE);
}

void psxcd_device::cdcmd_pause()
{
	LOGMASKED(LOG_CMD, "%s: pause\n", machine().describe_context());

	stop_read();

	send_result(INTR_ACKNOWLEDGE);
}

void psxcd_device::cdcmd_init()
{
	LOGMASKED(LOG_CMD, "%s: init\n", machine().describe_context());

	stop_read();
	mode=0;
	m_regs.sr |= 0x10;

	send_result(INTR_ACKNOWLEDGE);
}

void psxcd_device::cdcmd_mute()
{
	LOGMASKED(LOG_CMD, "%s: mute\n", machine().describe_context());

	m_mute = true;
	send_result(INTR_COMPLETE);
}

void psxcd_device::cdcmd_demute()
{
	LOGMASKED(LOG_CMD, "%s: demute\n", machine().describe_context());

	m_mute = false;
	send_result(INTR_COMPLETE);
}

void psxcd_device::cdcmd_setfilter()
{
	LOGMASKED(LOG_CMD, "%s: setfilter %08x,%08x\n", machine().describe_context(), cmdbuf[0], cmdbuf[1]);

	filter_file=cmdbuf[0];
	filter_channel=cmdbuf[1];

	send_result(INTR_ACKNOWLEDGE);
}

void psxcd_device::cdcmd_setmode()
{
	LOGMASKED(LOG_CMD, "%s: setmode %08x\n", machine().describe_context(), cmdbuf[0]);

	mode=cmdbuf[0];
	send_result(INTR_COMPLETE);
}

void psxcd_device::cdcmd_getparam()
{
	unsigned char data[6]=
	{
		status,
		mode,
		filter_file,
		filter_channel,
		0,
		0
	};

	LOGMASKED(LOG_CMD, "%s: getparam [%02x %02x %02x %02x %02x %02x]\n", machine().describe_context(),
								data[0], data[1], data[2], data[3], data[4], data[5]);

	send_result(INTR_COMPLETE, data, 6);
}

uint32_t psxcd_device::sub_loc(CDPOS src1, CDPOS src2)
{
	CDPOS dst;
	int f=src1.b[F]-src2.b[F],
			s=src1.b[S]-src2.b[S],
			m=src1.b[M]-src2.b[M];
	while (f<0) { s--; f+=75; }
	while (s<0) { m--; s+=60; }

	if (m<0)
		m=s=f=0;

	dst.b[M]=m;
	dst.b[S]=s;
	dst.b[F]=f;

	return dst.w;
}

void psxcd_device::cdcmd_getlocl()
{
	LOGMASKED(LOG_CMD, "%s: getlocl [%02x %02x %02x %02x %02x %02x %02x %02x]\n", machine().describe_context(),
							lastsechdr[0], lastsechdr[1], lastsechdr[2], lastsechdr[3],
							lastsechdr[4], lastsechdr[5], lastsechdr[6], lastsechdr[7]);

	send_result(INTR_COMPLETE, lastsechdr, 8);
}

void psxcd_device::cdcmd_getlocp()
{
	CDPOS tloc, start;
	uint8_t track = m_cdrom_handle->get_track(msf_to_lba_ps(loc.w)) + 1;
	start.w = (track == 1) ? 0x000200 : lba_to_msf_ps(m_cdrom_handle->get_track_start(track - 1));
	tloc.w = sub_loc(loc, start);

	unsigned char data[8] =
	{
		decimal_to_bcd(track),                          // track
		0x01,                           // index
		decimal_to_bcd(tloc.b[M]),    // min
		decimal_to_bcd(tloc.b[S]),    // sec
		decimal_to_bcd(tloc.b[F]),    // frame
		decimal_to_bcd(loc.b[M]), // amin
		decimal_to_bcd(loc.b[S]), // asec
		decimal_to_bcd(loc.b[F])  // aframe
	};

	LOGMASKED(LOG_CMD, "%s: getlocp [%02x %02x %02x %02x %02x %02x %02x %02x]\n", machine().describe_context(),
						data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

	send_result(INTR_COMPLETE, data, 8);
}

void psxcd_device::cdcmd_gettn()
{
	LOGMASKED(LOG_CMD, "%s: gettn\n", machine().describe_context());


	if (!open)
	{
		unsigned char data[3] =
		{
			status,
			decimal_to_bcd(1),
			decimal_to_bcd(m_cdrom_handle->get_last_track())
		};

		send_result(INTR_COMPLETE, data, 3);
	}
	else
	{
		send_result(INTR_DISKERROR, nullptr, 0, 0x80);
	}
}

void psxcd_device::cdcmd_gettd()
{
	uint8_t track = bcd_to_decimal(cmdbuf[0]);
	uint8_t last = m_cdrom_handle->get_last_track();
	if (track <= last)
	{
		CDPOS trkstart;
		if (!track) // length of disk
			trkstart.w = lba_to_msf_ps(m_cdrom_handle->get_track_start(0xaa));
		else
			trkstart.w = lba_to_msf_ps(m_cdrom_handle->get_track_start(track - 1));

		unsigned char data[3] =
		{
			status,
			decimal_to_bcd(trkstart.b[M]),
			decimal_to_bcd(trkstart.b[S])
		};

		LOGMASKED(LOG_CMD, "%s: gettd %02x [%02x %02x %02x]\n", machine().describe_context(), cmdbuf[0], data[0], data[1], data[2]);

		send_result(INTR_ACKNOWLEDGE, data, 3);
	}
	else
	{
		send_result(INTR_DISKERROR, nullptr, 0, 0x80);
	}
}

void psxcd_device::cdcmd_seekl()
{
	LOGMASKED(LOG_CMD, "%s: seekl [%02d:%02d:%02d]\n", machine().describe_context(), loc.b[M], loc.b[S], loc.b[F]);

	curpos.w = loc.w;

	send_result(INTR_ACKNOWLEDGE);
}

void psxcd_device::cdcmd_seekp()
{
	LOGMASKED(LOG_CMD, "%s: seekp\n", machine().describe_context());

	curpos.w = loc.w;

	send_result(INTR_ACKNOWLEDGE);
}

void psxcd_device::cdcmd_test()
{
	LOGMASKED(LOG_CMD, "%s: test %02x\n", machine().describe_context(), cmdbuf[0]);

	switch (cmdbuf[0])
	{
		case 0x20:
		{
			static uint8_t data[4] =
			{
				0x95,
				0x07,
				0x06,
				0xff
			};

			send_result(INTR_COMPLETE, data, 4);
			break;
		}

		default:
			logerror("%s: unimplemented test cmd %02x\n", machine().describe_context(), cmdbuf[0]);
			cmd_complete(prepare_result(INTR_DISKERROR, nullptr, 0, 0x10));
			break;
	}
}

void psxcd_device::cdcmd_id()
{
	LOGMASKED(LOG_CMD, "%s: id\n", machine().describe_context());

	if (!open)
	{
		uint8_t cdid[8];
		int irq;
		memset(cdid, '\0', 8);

		send_result(INTR_COMPLETE);
		if (m_cdrom_handle->get_track_type(0) == cdrom_file::CD_TRACK_AUDIO)
		{
			irq = INTR_DISKERROR;
			cdid[0] = status | STATUS_INVALID;
			cdid[1] = 0x90;
		}
		else
		{
			irq = INTR_ACKNOWLEDGE;
			cdid[0] = status;
			cdid[4] = 'S';
			cdid[5] = 'C';
			cdid[6] = 'E';
			cdid[7] = 'A';
		}
		send_result(irq, cdid, 8, default_irq_delay * 3);
	}
	else
	{
		send_result(INTR_DISKERROR, nullptr, 0, 0x80);
	}
}

void psxcd_device::cdcmd_reads()
{
	if (!open)
	{
		LOGMASKED(LOG_CMD, "%s: reads\n", machine().describe_context());

		curpos.w=loc.w;

		stop_read();
		start_read();
	}
	else
	{
		send_result(INTR_DISKERROR, nullptr, 0, 0x80);
	}
}

void psxcd_device::cdcmd_reset()
{
	LOGMASKED(LOG_CMD, "%s: reset\n", machine().describe_context());
}

void psxcd_device::cdcmd_readtoc()
{
	LOGMASKED(LOG_CMD, "%s: readtoc\n", machine().describe_context());

	send_result(INTR_COMPLETE);
	send_result(INTR_ACKNOWLEDGE, nullptr, 0, default_irq_delay * 3); // ?
}

void psxcd_device::cdcmd_unknown12()
{
	LOGMASKED(LOG_CMD, "%s: unknown 12\n", machine().describe_context());
	// set session? readt?
	if (cmdbuf[0] == 1)
		send_result(INTR_COMPLETE);
	else
		send_result(INTR_DISKERROR, nullptr, 0, 0x40);
}

void psxcd_device::cdcmd_illegal17()
{
	illegalcmd(0x17); // set clock?
}

void psxcd_device::cdcmd_illegal18()
{
	illegalcmd(0x18); // get clock?
}

void psxcd_device::cdcmd_illegal1d()
{
	illegalcmd(0x1d); // read q subchannel
}

void psxcd_device::illegalcmd(uint8_t cmd)
{
	logerror("%s: unimplemented cd command %02x\n", machine().describe_context(), cmd);

	send_result(INTR_DISKERROR, nullptr, 0, 0x40);
}

void psxcd_device::cmd_complete(command_result *res)
{
	command_result *rf;

	LOGMASKED(LOG_CMD, "%s: irq [%d]\n", machine().describe_context(), res->res);

	if (res_queue)
	{
		for (rf=res_queue; rf->next; rf=rf->next);
		rf->next=res;
	}
	else
	{
		res_queue = res;
		m_regs.ir = res_queue->res;
		if (m_regs.ir & m_regs.imr)
			m_irq_handler(1);
		m_regs.sr |= 0x20;
	}
	res->next = nullptr;
}

psxcd_device::command_result *psxcd_device::prepare_result(uint8_t res, uint8_t *data, int sz, uint8_t errcode)
{
	auto cr = new command_result;

	cr->res = res;
	if (sz)
	{
		assert(sz<sizeof(cr->data));
		memcpy(cr->data, data, sz);
		cr->sz = sz;
	}
	else
	{
		if ((res == INTR_DISKERROR) && errcode)
		{
			cr->data[0] = status | STATUS_ERROR;
			cr->data[1] = errcode;
			cr->sz = 2;
		}
		else
		{
			cr->data[0]=status;
			cr->sz=1;
		}
	}
	status &= ~STATUS_ERROR;

	return cr;
}

void psxcd_device::send_result(uint8_t res, uint8_t *data, int sz, int delay, uint8_t errcode)
{
	// Avoid returning results after sector read results -
	// delay the sector read slightly if necessary

	uint64_t systime = m_maincpu->total_cycles();
	if ((next_read_event != -1) && ((systime + delay)>(next_sector_t)))
	{
		uint32_t hz = m_sysclock / (delay + 2000);
		if (status & STATUS_PLAYING)
		{
			m_timers[next_read_event]->adjust(attotime::from_hz(hz), (next_read_event << 2) | EVENT_PLAY_SECTOR);
		}
		else if (status & STATUS_READING)
		{
			m_timers[next_read_event]->adjust(attotime::from_hz(hz), (next_read_event << 2) | EVENT_READ_SECTOR);
		}
	}

	add_system_event(EVENT_CMD_COMPLETE, delay, prepare_result(res, data, sz, errcode));
}

void psxcd_device::start_dma(uint8_t *mainram, uint32_t size)
{
	uint32_t sector_size;
	LOGMASKED(LOG_CMD, "%s: start dma %d bytes at %d\n", machine().describe_context(), size, m_transcurr);

	if (!m_dmaload)
		return;

	switch (mode & MODE_SIZE_MASK)
	{
		case 0x00:
		default:
			sector_size = 2048 + 24;
			break;
		case 0x10:
			sector_size = 2328 + 12;
			break;
		case 0x20:
			sector_size = 2340 + 12;
			break;
	}

	if (size > (sector_size - m_transcurr))
		size = (sector_size - m_transcurr);

	memcpy(mainram, &m_transbuf[m_transcurr], size);
	m_transcurr += size;

	if (sector_size <= m_transcurr)
	{
		m_dmaload = false;
		m_regs.sr &= ~0x40;
	}
}

void psxcd_device::read_sector()
{
	next_read_event=-1;

	if (status & STATUS_READING)
	{
		bool isend = false;
		uint32_t sector = msf_to_lba_ps(curpos.w);
		uint8_t *buf = secbuf[sectail];
		if (m_cdrom_handle->read_data(sector, buf, cdrom_file::CD_TRACK_RAW_DONTCARE))
		{
			subheader *sub = (subheader *)(buf + 16);
			memcpy(lastsechdr, buf + 12, 8);

			LOGMASKED(LOG_MISC, "%s: subheader file=%02x chan=%02x submode=%02x coding=%02x [%02x%02x%02x%02x]\n", machine().describe_context(),
						sub->file, sub->channel, sub->submode, sub->coding, buf[0xc], buf[0xd], buf[0xe], buf[0xf]);

			if ((mode & MODE_ADPCM) && (sub->submode & SUBMODE_AUDIO))
			{
				if ((sub->submode & SUBMODE_EOF) && (mode & MODE_AUTOPAUSE))
				{
					isend=true;
					//printf("end of file\n");
				}
				if ((!(mode & MODE_CHANNEL) || ((sub->file == filter_file) && (sub->channel == filter_channel))) && !m_mute)
					m_spu->play_xa(0, buf + 16);
			}
			else
			{
				if (!m_int1)
					m_cursec = sectail;

				m_int1 = prepare_result(INTR_DATAREADY);
				cmd_complete(m_int1);
				sectail++;
				sectail %= sector_buffer_size;
			}

			curpos.b[F]++;
			if (curpos.b[F] == 75)
			{
				curpos.b[F] = 0;
				curpos.b[S]++;
				if (curpos.b[S] == 60)
				{
					curpos.b[S] = 0;
					curpos.b[M]++;
				}
			}

			loc.w=curpos.w;
		}
		else
			isend = true;

		if (!isend)
		{
			uint32_t cyc = read_sector_cycles;
			if (mode & MODE_DOUBLE_SPEED)
				cyc >>= 1;

			next_sector_t += cyc;

			next_read_event = add_system_event(EVENT_READ_SECTOR, cyc, nullptr);
		}
		else
		{
			LOGMASKED(LOG_CMD, "%s: autopause xa\n", machine().describe_context());

			cmd_complete(prepare_result(INTR_DATAEND));
			stop_read();
		}
	}
}

void psxcd_device::play_sector()
{
	next_read_event = -1;

	if (status & STATUS_PLAYING)
	{
		uint32_t sector = msf_to_lba_ps(curpos.w);

		if (m_cdrom_handle->read_data(sector, secbuf[sectail], cdrom_file::CD_TRACK_AUDIO))
		{
			if (!m_mute)
				m_spu->play_cdda(0, secbuf[sectail]);
		}
		else
		{
			if (!m_cdrom_handle->read_data(sector, secbuf[sectail], cdrom_file::CD_TRACK_RAW_DONTCARE))
			{
				stop_read(); // assume we've reached the end
				cmd_complete(prepare_result(INTR_DATAEND));
				return;
			}
		}

		curpos.b[F]++;
		if (curpos.b[F]==75)
		{
			curpos.b[F]=0;
			curpos.b[S]++;
			if (curpos.b[S]==60)
			{
				curpos.b[S]=0;
				curpos.b[M]++;
			}
		}

		loc.w = curpos.w;
		sector++;
		sectail++;
		sectail %= sector_buffer_size;

		if (mode & MODE_AUTOPAUSE)
		{
			if (sector >= autopause_sector)
			{
				LOGMASKED(LOG_CMD, "%s: autopause cdda\n", machine().describe_context());

				stop_read();
				cmd_complete(prepare_result(INTR_DATAEND));
				return;
			}
		}

		if ((mode & MODE_REPORT) && !(sector & 15)) // slow the int rate
		{
			auto res = new command_result;
			uint8_t track = m_cdrom_handle->get_track(sector) + 1;
			res->res = INTR_DATAREADY;

			res->data[0]=status;
			res->data[1]=decimal_to_bcd(track);
			res->data[2]=1;
			if (sector & 0x10)
			{
				CDPOS tloc, start;
				start.w = (track == 1) ? 0x000200 : lba_to_msf_ps(m_cdrom_handle->get_track_start(track - 1));
				tloc.w = sub_loc(loc, start);
				res->data[3]=decimal_to_bcd(tloc.b[M]);
				res->data[4]=decimal_to_bcd(tloc.b[S]) | 0x80;
				res->data[5]=decimal_to_bcd(tloc.b[F]);
			}
			else
			{
				res->data[3]=decimal_to_bcd(loc.b[M]);
				res->data[4]=decimal_to_bcd(loc.b[S]);
				res->data[5]=decimal_to_bcd(loc.b[F]);
			}

			res->sz=8;

			cmd_complete(res);
		}

		uint32_t cyc = read_sector_cycles;

		next_sector_t += cyc >> 1;

		next_read_event = add_system_event(EVENT_PLAY_SECTOR, next_sector_t - m_maincpu->total_cycles(), nullptr);
	}
}

void psxcd_device::start_read()
{
	uint32_t sector = msf_to_lba_ps(curpos.w);

	assert((status & (STATUS_READING | STATUS_PLAYING)) == 0);

	if (!(mode & MODE_CDDA) && (m_cdrom_handle->get_track_type(m_cdrom_handle->get_track(sector + 150)) == cdrom_file::CD_TRACK_AUDIO))
	{
		stop_read();
		cmd_complete(prepare_result(INTR_DISKERROR, nullptr, 0, 0x40));
		return;
	}
	send_result(INTR_COMPLETE);
	status |= STATUS_READING;

	m_cursec=sectail=0;

	uint32_t cyc = read_sector_cycles;
	if (mode & MODE_DOUBLE_SPEED)
		cyc >>= 1;

	int64_t systime = m_maincpu->total_cycles();

	systime += start_read_delay;

	next_sector_t = systime + cyc;
	next_read_event = add_system_event(EVENT_READ_SECTOR, start_read_delay + preread_delay, nullptr);
}

void psxcd_device::start_play()
{
	uint8_t track = m_cdrom_handle->get_track(msf_to_lba_ps(curpos.w) + 150);

	if (m_cdrom_handle->get_track_type(track) != cdrom_file::CD_TRACK_AUDIO)
		logerror("%s: playing data track\n", machine().describe_context());

	status |= STATUS_PLAYING;

	m_cursec = sectail = 0;

	if (mode & MODE_AUTOPAUSE)
	{
		const auto &toc = m_cdrom_handle->get_toc();
		autopause_sector = m_cdrom_handle->get_track_start(track) + toc.tracks[track].logframes;
//      printf("pos=%d auto=%d\n",pos,autopause_sector);
	}

	uint32_t cyc = read_sector_cycles;

	next_sector_t = m_maincpu->total_cycles() + cyc;

	next_sector_t += cyc >> 1;

	next_read_event = add_system_event(EVENT_PLAY_SECTOR, next_sector_t - m_maincpu->total_cycles(), nullptr);
}

void psxcd_device::stop_read()
{
	if (status & (STATUS_READING | STATUS_PLAYING))
		LOGMASKED(LOG_CMD, "%s: stop read\n", machine().describe_context());

	status &= ~(STATUS_READING | STATUS_PLAYING);

	if (next_read_event != -1)
	{
		m_timers[next_read_event]->adjust(attotime::never);
		m_timerinuse[next_read_event] = false;
		next_read_event = -1;
	}

	uint32_t sector = msf_to_lba_ps(curpos.w);
	m_spu->flush_xa(sector);
	m_spu->flush_cdda(sector);
}

TIMER_CALLBACK_MEMBER(psxcd_device::handle_event)
{
	int tid = param >> 2;
	if (!m_timerinuse[tid])
	{
		logerror("%s: timer fired for free event\n", machine().describe_context());
		return;
	}

	m_timerinuse[tid] = false;
	switch (param & 3)
	{
		case EVENT_CMD_COMPLETE:
			LOGMASKED(LOG_CMD, "%s: event cmd complete\n", machine().describe_context());
			cmd_complete(m_results[tid]);
			break;

		case EVENT_READ_SECTOR:
			read_sector();
			break;

		case EVENT_PLAY_SECTOR:
			play_sector();
			break;

		case EVENT_CHANGE_DISK:
			open = false;
			status |= STATUS_STANDBY;
			break;
	}
}

int psxcd_device::add_system_event(int type, uint64_t t, command_result *ptr)
{
	// t is in maincpu clock cycles
	uint32_t hz = m_sysclock / t;
	for (int i = 0; i < MAX_PSXCD_TIMERS; i++)
	{
		if (!m_timerinuse[i])
		{
			m_timers[i]->adjust(attotime::from_hz(hz), (i << 2) | type, attotime::never);
			m_results[i] = ptr;
			m_timerinuse[i] = true;
			return i;
		}
	}
	fatalerror("psxcd: out of timers\n");
}

/*
 * TODO:
 * Declare the MCU (Motorola 68HC05 [?] at 4MHz [?]) and hookup
 * these ROM images so that we actually emulate the PSX CDROM unit
 *
 * More technical info is available at these links:
 *  http://www.psxdev.net/forum/viewtopic.php?f=60&t=573
 *  http://www.psxdev.net/forum/viewtopic.php?f=70&t=557
 *
 */
ROM_START( psxcd )
	ROM_REGION( 0x10000, "cdrom_mcu", 0 )

	/* Retail PlayStation CD-ROM Firmware:
	 *
	 * Still missing:
	 * SCPH-1001, 3000, 3500, 5003, 5502, 5503, 7000W, 7001, 7002, 7003, 7503, 9003, 100, 101, 102 and 103.
	 */

	ROM_SYSTEM_BIOS( 0, "scph-1000-later", "SCPH-1000 NTSC:J (Later Ver.) [424660]" )
	ROMX_LOAD( "424660.bin", 0x0000, 0x4200, CRC(f82a2a46) SHA1(095434948d4c71cdfaa069e91053443887a6d139), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "scph-1000-early", "SCPH-1000 NTSC:J (Early Ver.) [424666]" )
	ROMX_LOAD( "424666.bin", 0x0000, 0x4200, CRC(60bc954e) SHA1(80674353daf95ffb4bd15cc4bb8cfa713370dd45), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 2, "scph-1002-early", "SCPH-1002 PAL (Early PU-8) [424684]" )
	ROMX_LOAD( "424684.bin", 0x0000, 0x4200, CRC(84d46b2a) SHA1(9b06b1d407b784095ddbd45aeabafd689d2ee347), ROM_BIOS(2) )

	/* Chip markings: C 1021 / SC430916PB / G63C 185 / JSAB9624F */
	ROM_SYSTEM_BIOS( 3, "scph-5000", "SCPH-5000 NTSC:J [SC430916]" )
	ROMX_LOAD( "sc430916.s19", 0x0000, 0xb195, CRC(487c8a40) SHA1(0ae8348fb43ab80845b0166494edc3e1565a3ef7), ROM_BIOS(3) )

	/* Chip markings: C 1030 / SC430925PB / G63C 185 / JSBK9708C
	   Board Type: PU-18 / 1-664-537-11 */
	ROM_SYSTEM_BIOS( 4, "scph-5500", "SCPH-5500 NTSC:J [SC430925]" )
	ROMX_LOAD( "sc430925.s19", 0x0000, 0xb195, CRC(c09aa0c2) SHA1(b9ad66cc8ea4d6e2eb2709ffb77c9647f679097a), ROM_BIOS(4) )

	/* Chip markings: C 2030 / SC430930PB / G63C 185 / SSJZ9748A
	   Board Type: PU-18 / 1-664-537-62 */
	ROM_SYSTEM_BIOS( 5, "scph-5501", "SCPH-5501 NTSC:U/C [SC430930]" )
	ROMX_LOAD( "sc430930.s19", 0x0000, 0xb195, CRC(587b84c2) SHA1(556c3adc37e4eb64fd463c54f7a310c483e0e835), ROM_BIOS(5) )

	/* ROM dump is the same as SCPH-5552 */
	ROM_SYSTEM_BIOS( 6, "scph-5502", "SCPH-5502 PAL [SC430929]" )
	ROMX_LOAD( "sc430929.bin", 0x0000, 0x4200, CRC(ba87a3e0) SHA1(f23458d13a518616a8592b8ddd668c052bc9be5a), ROM_BIOS(6) )

	ROM_SYSTEM_BIOS( 7, "scph-5903", "SCPH-5903 NTSC:J [SC430924PB]" )
	ROMX_LOAD( "sc430924.s19", 0x0000, 0xb195, CRC(dbe694b2) SHA1(ac72cb616b1449fe29e52faf6aad389118852d73), ROM_BIOS(7) )

	/* Chip markings: C 1040 / SC430934PB / G63C 185 / SSDG9745D */
	ROM_SYSTEM_BIOS( 8, "scph-7000", "SCPH-7000 NTSC:J [SC430934]" )
	ROMX_LOAD( "sc430934.s19", 0x0000, 0xb195, CRC(6443740c) SHA1(d9734c7135c75dbe7733079a2d4244a28c9e966e), ROM_BIOS(8) )

	/* Chip markings: C 1050 / SC430938PB / G63C 185 / SSAM9850C */
	ROM_SYSTEM_BIOS( 9, "scph-7500", "SCPH-7500 NTSC:J [SC430938]" )
	ROMX_LOAD( "sc430938.s19", 0x0000, 0xb195, CRC(9744977a) SHA1(f017d34a98a8a023f6752ba9ed749bb9e2b836d5), ROM_BIOS(9) )

	/* Chip markings: C 2050 / SC430940PB / G63C 185 / SSDL9838A */
	ROM_SYSTEM_BIOS( 10, "scph-7501", "SCPH-7501 NTSC:U/C [SC430940]" )
	ROMX_LOAD( "sc430940.s19", 0x0000, 0xb195, CRC(fd1c6ee7) SHA1(e72b5093a3e25de1548be7668179ff3e001e3ec5), ROM_BIOS(10) )

	ROM_SYSTEM_BIOS( 11, "scph-7502", "SCPH-7502 PAL [SC430939]" )
	ROMX_LOAD( "sc430939.bin", 0x0000, 0x4200, CRC(9eafb045) SHA1(25d98454e567e064c06f840d57f763fb7c8b7219), ROM_BIOS(11) )

	ROM_SYSTEM_BIOS( 12, "scph-9000", "SCPH-9000 NTSC:J [SC430942]" )
	ROMX_LOAD( "sc430942.bin", 0x0000, 0x4200, NO_DUMP, ROM_BIOS(12) )

	/* Chip markings: C 2060 / SC430944PB / G63C 185 / SSBR9924C */
	ROM_SYSTEM_BIOS( 13, "scph-9001", "SCPH-9001 NTSC:U/C [SC430944]" )
	ROMX_LOAD( "sc430944.s19", 0x0000, 0xb195, CRC(24011dfd) SHA1(db72ba02466942d1a1a07c4d855edd18f84de92e), ROM_BIOS(13) )

	ROM_SYSTEM_BIOS( 14, "scph-9002", "SCPH-9002 PAL [SC430943]" )
	ROMX_LOAD( "sc430943.bin", 0x0000, 0x4200, CRC(2669a1a7) SHA1(62999e7f8429f381e19d44d2399b6017959f4f13), ROM_BIOS(14) )

	/* Development PlayStation CD-ROM Firmware:
	 *
	 * Still missing:
	 * DTL-H1000, 1000H, 1001, 1001H, 1002, 1100, 1101, 1102, 1200, 1201, 2000, 2500, 2700, 3000, 3001 and 3002.
	 */

	/* Chip markings: D 2021 / SC430920PB / G63C 185 / JSAA9810A
	 * Note: Although this is a PAL unit, the ID string in the
	 * code is "for US/AEP", so it may be the same for all the
	 * debug consoles.
	 */
	ROM_SYSTEM_BIOS( 15, "dtl-h1202", "DTL-H1202 PAL [SC430920]" )
	ROMX_LOAD( "sc430920.s19", 0x0000, 0xb195, CRC(8380a5a2) SHA1(6fe45fd6fb96b12a25a45f39b5efd0be5e3f3e86), ROM_BIOS(15) )
ROM_END

const tiny_rom_entry *psxcd_device::device_rom_region() const
{
	return ROM_NAME( psxcd );
}
