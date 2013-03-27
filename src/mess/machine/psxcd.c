#include "emu.h"
#include "psxcd.h"
#include "debugger.h"

//
//
//

//#define debug_cdrom
//#define debug_cdrom_registers
//#define skip_reads
//#define dump_subheader

//
//
//

enum cdrom_events
{
	event_cmd_complete=0,
	event_preread_sector,
	event_read_sector,
	event_play_sector,
	event_change_disk
};

//
//
//

enum intr_status
{
	intr_nointr=0,
	intr_dataready,
	intr_acknowledge,
	intr_complete,
	intr_dataend,
	intr_diskerror
};

enum mode_flags
{
	mode_double_speed=0x80,
	mode_adpcm=0x40,
	mode_size=0x20,
	mode_size2=0x10,
	mode_size_shift=4,
	mode_size_mask=(3<<mode_size_shift),
	mode_channel=0x08,
	mode_report=0x04,
	mode_autopause=0x02,
	mode_cdda=0x01
};

enum status_f
{
	status_playing=0x80,
	status_seeking=0x40,
	status_reading=0x20,
	status_shellopen=0x10,
	status_invalid=0x08,
	status_seekerror=0x04,
	status_standby=0x02,
	status_error=0x01
};

struct subheader
{
	unsigned char file,
								channel,
								submode,
								coding;
};

enum submode_flags
{
	submode_eof=0x80,
	submode_realtime=0x40,
	submode_form=0x20,
	submode_trigger=0x10,
	submode_data=0x08,
	submode_audio=0x04,
	submode_video=0x02,
	submode_eor=0x01
};

//
//
//

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PSXCD = &device_creator<psxcd_device>;

static struct cdrom_interface psx_cdrom =
{
	"psx_cdrom",
	NULL
};

psxcd_device::psxcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	cdrom_image_device(mconfig, PSXCD, "Cdrom", tag, owner, clock, "psx_cd", __FILE__),
	m_irq_handler(*this)
{
	static_set_static_config(*this, &psx_cdrom);
}


void psxcd_device::device_start()
{
	cdrom_image_device::device_start();
	m_irq_handler.resolve_safe();

	m_maincpu = machine().device<cpu_device>("maincpu");
	m_spu = machine().device<spu_device>("spu");

	unsigned int sysclk=m_maincpu->clock()/2;
	start_read_delay=(sysclk/60);
	read_sector_cycles=(sysclk/75);
	preread_delay=(read_sector_cycles>>2)-500;

	m_sysclock = sysclk;

	secleft = 0;
	secsize = 2048;
	res_queue = NULL;
	cur_res = NULL;
	open = false;
	streaming = false;
	sechead = 0;
	sectail = 0;
	secskip = 0;
	next_read_event = -1;
	cbp = cmdbuf;
	m_mute = false;

	status=status_shellopen;
	sr=8|1;
	res=0;
	ir=0;
	mode=0;

	for (int i = 0; i < MAX_PSXCD_TIMERS; i++)
	{
		m_timers[i] = timer_alloc(i);
		m_timerinuse[i] = false;
	}

	curpos.w = 0;
	m_param_count = 0;
}

//
//
//

void psxcd_device::device_reset()
{
	stop_read();
	open=false;

	for (int i = 0; i < MAX_PSXCD_TIMERS; i++)
	{
		m_timerinuse[i] = false;
	}

	if(cur_res)
	{
		global_free(cur_res);
		cur_res = NULL;
	}

	while(res_queue)
	{
		cur_res = res_queue->next;
		global_free(res_queue);
		res_queue = cur_res;
	}

	m_param_count = 0;
}

//
//
//

bool psxcd_device::call_load()
{
	bool ret = cdrom_image_device::call_load();
	open = true;
	if(ret == IMAGE_INIT_PASS)
		add_system_event(event_change_disk, m_sysclock, NULL); // 1 sec to spin up the disk
	return ret;
}

void psxcd_device::call_unload()
{
	stop_read();
	cdrom_image_device::call_unload();
	open = true;
	status = status_shellopen;
	send_result(intr_diskerror);
}

READ8_MEMBER( psxcd_device::read )
{
	unsigned char ret = 0;

	switch (offset&3)
	{
		/*
		x--- ---- command/parameter busy flag
		-x-- ---- data fifo full (active low)
		--x- ---- response fifo empty (active low)
		---x ---- parameter fifo full (active low)
		---- x--- parameter fifo empty (active high)
		---- --xx cmd mode
		*/
		case 0: ret=sr; break;
		case 1:
			ret=res;
			if ((cur_res) && (rdp<cur_res->sz))
			{
				res=cur_res->data[rdp++];
				sr|=(1<<5);
			} else
			{
				if ((cur_res) && (cur_res->res&0x10))
				{
					global_free(cur_res);
					cur_res=NULL;
				}

				sr&=~(1<<5);
			}
			break;
		case 2: ret=0; break;
		case 3: ret=ir; break;
	}

	#ifdef debug_cdrom_registers
		printf("cdrom: read byte %08x = %02x (PC=%08x)\n",offset,ret,space.device().safe_pc());
	#endif

	return ret;
}

//
//
//

WRITE8_MEMBER( psxcd_device::write )
{
	#ifdef debug_cdrom_registers
		printf("cdrom: write byte %08x = %02x (PC=%08x)\n",offset,data,space.device().safe_pc());
	#endif

	switch (offset&3)
	{
		case 0:
			//if(data & 2)
			//  popmessage("cmdmode = %02x, contact MESSdev",data);

			cmdmode=data&1;
			if (cmdmode==0)
			{
				cbp=cmdbuf;
			} else
			{
				if (! cur_res)
				{
					if (cur_res) global_free(cur_res);

					if (res_queue)
					{
						#ifdef debug_cdrom_registers
							printf("cdrom: nextres\n");
						#endif

						cur_res=res_queue;
						res_queue=res_queue->next;
						ir=cur_res->res&0xf;
						rdp=0;

						if (cur_res->sz)
						{
							res=cur_res->data[rdp++];
							sr|=(1<<5);
						} else
						{
							sr&=~(1<<5);
						}
					} else
					{
						//ir=0;
						cur_res=NULL;
					}
				}
				/*else
				{
				    if (rdp>=cur_res->sz)
				    {
				        sr&=~(1<<5);
				    } else
				    {
				        sr|=~(1<<5);
				        res=cur_res->data[rdp++];
				    }
				}
				*/
			}
			break;

		case 1:
			if (cmdmode==0)
			{
				write_command(data);
			}
			break;

		case 2:
			if (cmdmode==0)
			{
				*cbp++=data;
				m_param_count++;
			} else
			{
				// ?flush buffer?
				//if(data & 0xf8)
				//popmessage("Interrupt enable register mode 1 [%02x] -> %02x",offset,data);
			}
			break;

		/*
		x--- ---- unknown
		-x-- ---- Reset parameter FIFO
		--x- ---- unknown (used on transitions, so it certainly resets something)
		---x ---- Command start
		---- -xxx Response received
		*/
		case 3:
			//if(data & 0x78)
			//  popmessage("IRQ flag = %02x, contact MESSdev",data);

			if (data==0x07)
			{
				if (cur_res)
				{
					global_free(cur_res);
					cur_res=NULL;
					sr&=~(1<<5);
				}
				ir=0;
			}
	}
}

psxcd_device::command_info psxcd_device::cmd_table[num_commands]=
{
	{ &psxcd_device::cdcmd_sync,                "sync"          },  // 00
	{ &psxcd_device::cdcmd_nop,             "nop"               },  // 01
	{ &psxcd_device::cdcmd_setloc,          "setloc"        },  // 02
	{ &psxcd_device::cdcmd_play,                "play"          },  // 03
	{ &psxcd_device::cdcmd_forward,     "forward"       },  // 04
	{ &psxcd_device::cdcmd_backward,        "backward"  },  // 05
	{ &psxcd_device::cdcmd_readn,           "readn"         },  // 06
	{ &psxcd_device::cdcmd_standby,     "standby"       },  // 07
	{ &psxcd_device::cdcmd_stop,                "stop"          },  // 08
	{ &psxcd_device::cdcmd_pause,           "pause"         },  // 09
	{ &psxcd_device::cdcmd_init,                "init"          },  // 0a
	{ &psxcd_device::cdcmd_mute,                "mute"          },  // 0b
	{ &psxcd_device::cdcmd_demute,          "demute"        },  // 0c
	{ &psxcd_device::cdcmd_setfilter,   "setfilter" },  // 0d
	{ &psxcd_device::cdcmd_setmode,     "setmode"       },  // 0e
	{ &psxcd_device::cdcmd_getparam,        "getparam"  },  // 0f
	{ &psxcd_device::cdcmd_getlocl,     "getlocl"       },  // 10
	{ &psxcd_device::cdcmd_getlocp,     "getlocp"       },  // 11
	{ &psxcd_device::cdcmd_illegal,     "illegal"       },  // 12
	{ &psxcd_device::cdcmd_gettn,           "gettn"         },  // 13
	{ &psxcd_device::cdcmd_gettd,           "gettd"         },  // 14
	{ &psxcd_device::cdcmd_seekl,           "seekl"         },  // 15
	{ &psxcd_device::cdcmd_seekp,           "seekp"         },  // 16
	{ &psxcd_device::cdcmd_illegal,     "illegal"       },  // 17
	{ &psxcd_device::cdcmd_illegal,     "illegal"       },  // 18
	{ &psxcd_device::cdcmd_test,                "test"          },  // 19
	{ &psxcd_device::cdcmd_id,                  "id"                },  // 1a
	{ &psxcd_device::cdcmd_reads,           "reads"         },  // 1b
	{ &psxcd_device::cdcmd_reset,           "reset"         },  // 1c
	{ &psxcd_device::cdcmd_illegal,     "illegal"       },  // 1d
	{ &psxcd_device::cdcmd_readtoc,     "readtoc"       },  // 1e
};

//
//
//

void psxcd_device::write_command(const unsigned char byte)
{
	assert(byte<num_commands);
	(this->*cmd_table[byte].func)();
	m_param_count = 0;
}

//
//
//

void psxcd_device::cdcmd_sync()
{
	#ifdef debug_cdrom
		printf("cdrom: sync\n");
	#endif

	stop_read();
	send_result(intr_acknowledge);
}

void psxcd_device::cdcmd_nop()
{
	#ifdef debug_cdrom
		printf("cdrom: nop\n");
	#endif

	//stop_read();

	if (!open)
		status &= ~status_shellopen;

	send_result(intr_complete);
}

void psxcd_device::cdcmd_setloc()
{
	#ifdef debug_cdrom
		printf("cdrom: setloc %08x:%08x:%08x\n",
				cmdbuf[0],
				cmdbuf[1],
				cmdbuf[2]);
	#endif

	stop_read();


	CDPOS l;
	l.b[M]=bcd_to_decimal(cmdbuf[0]);
	l.b[S]=bcd_to_decimal(cmdbuf[1]);
	l.b[F]=bcd_to_decimal(cmdbuf[2]);

	if ((l.b[M]>0) || (l.b[S]>=2))
		loc.w=l.w;
	else
		logerror("setloc out of range: %02d:%02d:%02d\n",l.b[M],l.b[S],l.b[F]);

	send_result(intr_complete);
}

void psxcd_device::cdcmd_play()
{
	if(cmdbuf[0] && m_param_count)
		 loc.w = lba_to_msf_ps(cdrom_get_track_start(m_cdrom_handle, bcd_to_decimal(cmdbuf[0]) - 1));

	curpos.w = loc.w;
	if (!curpos.w)
		curpos.b[S] = 2;

#ifdef debug_cdrom
	printf("cdrom: play %02x %02x %02x => %d\n", loc.b[M], loc.b[S], loc.b[F], msf_to_lba_ps(loc.w));
#endif

	stop_read();
	start_play();
	send_result(intr_complete);
}

void psxcd_device::cdcmd_forward()
{
	#ifdef debug_cdrom
		printf("cdrom: forward\n");
	#endif
}

void psxcd_device::cdcmd_backward()
{
	#ifdef debug_cdrom
		printf("cdrom: backward\n");
	#endif
}

void psxcd_device::cdcmd_readn()
{
	if(!open)
	{
		#ifdef debug_cdrom
			printf("cdrom: readn\n");
		#endif

		curpos.w=loc.w;

		stop_read();
		start_read();
		send_result(intr_complete);
	} else
	{
		send_result(intr_diskerror);
	}
}

void psxcd_device::cdcmd_standby()
{
	#ifdef debug_cdrom
		printf("cdrom: standby\n");
	#endif

	stop_read();
	send_result(intr_acknowledge);
}

void psxcd_device::cdcmd_stop()
{
	#ifdef debug_cdrom
		printf("cdrom: stop\n");
	#endif

	stop_read();
	send_result(intr_acknowledge);
}

void psxcd_device::cdcmd_pause()
{
	#ifdef debug_cdrom
		printf("cdrom: pause\n");
	#endif

	stop_read();

	send_result(intr_acknowledge);
}

void psxcd_device::cdcmd_init()
{
	#ifdef debug_cdrom
		printf("cdrom: init\n");
	#endif

	stop_read();
	mode=0;
	sr|=0x10;

	send_result(intr_acknowledge);
}

void psxcd_device::cdcmd_mute()
{
	#ifdef debug_cdrom
		printf("cdrom: mute\n");
	#endif

	m_mute = true;
	send_result(intr_acknowledge);
}

void psxcd_device::cdcmd_demute()
{
	#ifdef debug_cdrom
		printf("cdrom: demute\n");
	#endif

	m_mute = false;
	send_result(intr_acknowledge);
}

void psxcd_device::cdcmd_setfilter()
{
	#ifdef debug_cdrom
		printf("cdrom: setfilter %08x,%08x\n",cmdbuf[0],cmdbuf[1]);
	#endif

	filter_file=cmdbuf[0];
	filter_channel=cmdbuf[1];

	send_result(intr_acknowledge);
}

void psxcd_device::cdcmd_setmode()
{
	#ifdef debug_cdrom
		printf("cdrom: setmode %08x\n",cmdbuf[0]);
	#endif

	mode=cmdbuf[0];

	switch ((mode&mode_size_mask)>>mode_size_shift)
	{
		case 1:
			secsize=2328;
			secskip=24;
			break;

		case 2:
			secsize=2340;
			secskip=12;
			break;

		default:
			secsize=2048;
			secskip=24;
			break;
	}

	send_result(intr_complete);
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

	#ifdef debug_cdrom
		printf("cdrom: getparam [%02x %02x %02x %02x %02x %02x]\n",
									data[0],
									data[1],
									data[2],
									data[3],
									data[4],
									data[5]);
	#endif

	send_result(intr_complete,data,6);
}

UINT32 psxcd_device::sub_loc(CDPOS src1, CDPOS src2)
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
	#ifdef debug_cdrom
		printf("cdrom: getlocl\n");
	#endif

	#ifdef debug_cdrom
		printf("cdrom: getlocl [%02x %02x %02x %02x %02x %02x %02x %02x]\n",
									lastsechdr[0],
									lastsechdr[1],
									lastsechdr[2],
									lastsechdr[3],
									lastsechdr[4],
									lastsechdr[5],
									lastsechdr[6],
									lastsechdr[7]);
	#endif

	send_result(intr_complete,lastsechdr,8);
}

void psxcd_device::cdcmd_getlocp()
{
	CDPOS tloc, start;
	UINT8 track = cdrom_get_track(m_cdrom_handle, msf_to_lba_ps(loc.w) + 150) + 1;
	start.w = (track == 1) ? 0x000200 : lba_to_msf_ps(cdrom_get_track_start(m_cdrom_handle, track - 1));
	tloc.w = sub_loc(loc, start);

	unsigned char data[8]=
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

	//unsigned char data[8]={ 2,1,0,0xff,0xff,0xff,0xff,0xff };

	#ifdef debug_cdrom
		printf("cdrom: getlocp [%02x %02x %02x %02x %02x %02x %02x %02x]\n",
									data[0],
									data[1],
									data[2],
									data[3],
									data[4],
									data[5],
									data[6],
									data[7]);
	#endif

	send_result(intr_complete,data,8);
}

void psxcd_device::cdcmd_illegal()
{
	assert(0);
}

void psxcd_device::cdcmd_gettn()
{
	#ifdef debug_cdrom
		printf("cdrom: gettn\n");
	#endif


	if(!open)
	{
		unsigned char data[3]=
		{
				status,
				decimal_to_bcd(1),
				decimal_to_bcd(cdrom_get_last_track(m_cdrom_handle))
		};

		//stop_read();
		send_result(intr_complete,data,3);
	}
	else
	{
		status |= status_error;
		send_result(intr_diskerror);
	}
}

void psxcd_device::cdcmd_gettd()
{
	UINT8 track = bcd_to_decimal(cmdbuf[0]);
	UINT8 last = cdrom_get_last_track(m_cdrom_handle);
	if(track <= last)
	{
		CDPOS trkstart;
		if(!track) // length of disk
			trkstart.w = lba_to_msf_ps(cdrom_get_track_start(m_cdrom_handle, 0xaa));
		else
			trkstart.w = lba_to_msf_ps(cdrom_get_track_start(m_cdrom_handle, track - 1));

		unsigned char data[3]=
		{
			status,
			decimal_to_bcd(trkstart.b[M]),
			decimal_to_bcd(trkstart.b[S])
		};

		#ifdef debug_cdrom
			printf("cdrom: gettd %02x [%02x %02x %02x]\n",
														cmdbuf[0],
														data[0],
														data[1],
														data[2]);
		#endif

		//stop_read();
		send_result(intr_acknowledge,data,3);
	}
	else
	{
		status |= status_error;
		send_result(intr_diskerror);
	}
}

void psxcd_device::cdcmd_seekl()
{
	#ifdef debug_cdrom
		printf("cdrom: seekl [%02d:%02d:%02d]\n",loc.b[M],loc.b[S],loc.b[F]);
	#endif

	curpos.w=loc.w;

	send_result(intr_acknowledge);
}

void psxcd_device::cdcmd_seekp()
{
	#ifdef debug_cdrom
		printf("cdrom: seekp\n");
	#endif

	curpos.w=loc.w;

	send_result(intr_acknowledge);
}

void psxcd_device::cdcmd_test()
{
	#ifdef debug_cdrom
		printf("cdrom: test %08x\n",cmdbuf[0]);
	#endif

	static unsigned char data[4]=
	{
		0x95,
		0x07,
		0x06,
		0xff
	};

	send_result(intr_complete,data,4);
}

void psxcd_device::cdcmd_id()
{
	#ifdef debug_cdrom
		printf("cdrom: id\n");
	#endif

	if (!open)
	{
		static unsigned char gamedata[8] = { 0x00, 0x00, 0x00, 0x00, 'S', 'C', 'E', 'A' };
		static unsigned char audiodata[8] = { 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // drops into the audio CD player.  08 80 goes to the menu.

		if(cdrom_get_track_type(m_cdrom_handle, 0) == CD_TRACK_AUDIO)
		{
			audiodata[0] = status | status_invalid;
			send_result(intr_acknowledge,audiodata,8);
		}
		else
		{
			gamedata[0] = status;
			send_result(intr_acknowledge,gamedata,8);
		}
	} else
	{
		status |= status_error;
		send_result(intr_diskerror);
	}
}

void psxcd_device::cdcmd_reads()
{
	curpos.w=loc.w;

	#ifdef skip_reads
		#ifdef debug_cdrom
			log("cdrom: reads [SKIPPING - RETURN COMPLETE]\n");
		#endif

		send_result(intr_complete);
	#else
		#ifdef debug_cdrom
			printf("cdrom: reads\n");
		#endif

		stop_read();
		start_streaming();
	#endif
}

void psxcd_device::cdcmd_reset()
{
	#ifdef debug_cdrom
		printf("cdrom: reset\n");
	#endif
}

void psxcd_device::cdcmd_readtoc()
{
	#ifdef debug_cdrom
		printf("cdrom: readtoc\n");
	#endif

	static unsigned char data[4]=
	{
		0x00,
		0x00,
		0xff,
		0xfe
	};

	send_result(intr_complete|0x10,data,2);
}

//
//
//

void psxcd_device::cmd_complete(command_result *res)
{
	bool doint=((res->res&0x10)==0);
	#ifdef debug_cdrom
	command_result *rf;
	#endif

	if (doint)
	{
		m_irq_handler(1);
	}

	add_result(res);

	#ifdef debug_cdrom
		if (doint)
		{
			printf("cdrom: irq [");
			for (rf=res_queue; ((rf) && (rf->next)); rf=rf->next);
				printf("%d ",rf->res);
			printf("]\n");
		}
	#endif
}

//
//
//

void psxcd_device::add_result(command_result *res)
{
	command_result *rf;

	if (res_queue)
	{
		for (rf=res_queue; rf->next; rf=rf->next);
		rf->next=res;
	} else
	{
		res_queue=res;
	}

	res->next=NULL;
}

//
//
//

void psxcd_device::send_result(const unsigned int res,
														const unsigned char *data,
														const unsigned int sz,
															const unsigned int delay)
{
	command_result *cr=global_alloc(command_result);

	cr->res=res;
	if (sz)
	{
		assert(sz<sizeof(cr->data));
		memcpy(cr->data,data,sz);
		cr->sz=sz;
	} else
	{
		cr->data[0]=status;
		cr->sz=1;
	}
	status &= ~status_error;

	// Avoid returning results after sector read results -
	// delay the sector read slightly if necessary

	UINT64 systime = m_maincpu->total_cycles();
	if ((next_read_event != -1) && ((systime+delay)>(next_sector_t)))
	{
		UINT32 hz = m_sysclock / (delay + 2000);
		m_timers[next_read_event]->adjust(attotime::from_hz(hz), 0, attotime::never);
	}

	add_system_event(event_cmd_complete, delay, (void *)cr);
}

//
//
//

void psxcd_device::start_dma(UINT8 *mainram, UINT32 size)
{
	if ((int)size>secleft)
	{
		logerror("cdrom: dma past end of sector (secleft=%d sz=%d)\n",secleft,size);
	}

//  printf("cdrom: start dma %d bytes, %d remaining, secptr %p\n", size, secleft, secptr);
	#ifdef debug_cdrom
	if (size==12)
	{
		char poo[1024],*pp=poo;
		for (unsigned int i=0; i<12; i++)
		{
			sprintf(pp,"%02x ",secptr[i]);
			pp+=3;
		}
		printf("cdrom: data=%s\n",poo);
	}
	#endif

	memcpy(mainram, secptr, size);
	memset(secptr, 0xff, size);

	secptr += size;
	secleft -= size;

	if (secleft<0) secleft=0;
	if (secleft==0) sr&=~0x40;
}

bool psxcd_device::read_next_sector()
{
	UINT32 pos=msf_to_lba_ps(curpos.w);
	unsigned char *buf=secbuf[sechead];

//  printf("read_next_sector: sec %d, sechead %d, raw_sector_size %d\n", pos, sechead, raw_sector_size);
	if (cdrom_read_data(m_cdrom_handle, pos, buf, CD_TRACK_RAW_DONTCARE))
	{
//      printf("buf contents = %02x %02x | %02x %02x\n", buf[0], buf[1], buf[0x20], buf[0x21]);

		sechead=(sechead+1)&(sector_buffer_size-1);

		memcpy(lastsechdr,&secbuf[sectail][12],8);

		return true;
	} else
	{
		return false;
	}
}

//
//
//

void psxcd_device::read_sector()
{
	next_read_event=-1;

	if (status & status_reading)
	{
		bool isend=false;

		if (read_next_sector())
		{
			unsigned char *rawsec;

			if ((mode&mode_adpcm) && (streaming))
			{
				rawsec=secbuf[sectail];
				secptr=rawsec+24;
				secleft=2048;
			}
			else
			{
				rawsec=secbuf[sectail];
				secptr=rawsec+secskip;
				secleft=secsize;
			}

			sectail=(sectail+1)&(sector_buffer_size-1);

			subheader *sub=(subheader *)(rawsec+16);

			#ifdef dump_subheader
				printf("cdrom: subheader file=%02x chan=%02x submode=%02x coding=%02x [%02x%02x%02x%02x]\n",
						sub->file,
						sub->channel,
						sub->submode,
						sub->coding,
						rawsec[0xc],
						rawsec[0xd],
						rawsec[0xe],
						rawsec[0xf]);
			#endif

			status&=~status_playing;
			bool isxa=((mode&mode_adpcm) && (sub->submode&submode_audio));

			if (((mode&mode_channel)==0) ||
					((sub->file==filter_file) && (sub->channel==filter_channel)))
			{
				if (isxa)
				{
					if (sub->submode&submode_eof)
					{
						isend=true;
						//printf("end of file\n");
					}
					m_spu->play_xa(0,rawsec+16);

					status|=status_playing;
				}
			}

			if ((mode&mode_autopause)==0)
				isend=false;

			//

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

			loc.w=curpos.w;

			sr|=0x40;

			if(!(streaming && isxa))
			{
				command_result *res=global_alloc(command_result);
				res->res=isend?intr_dataend:intr_dataready;
				res->data[0]=status;
				res->sz=1;
				cmd_complete(res);
			}
		}

		if (! isend)
		{
			unsigned int cyc=read_sector_cycles;
			if (mode&mode_double_speed) cyc>>=1;

			next_sector_t+=cyc;

			next_read_event = add_system_event(event_read_sector, cyc, NULL);

			//read_next_sector();
		} else
		{
#ifdef debug_cdrom
			printf("autopause xa\n");
#endif
			stop_read();
		}
	}
}

//
//
//

void psxcd_device::play_sector()
{
	next_read_event=-1;

	if (status&status_playing)
	{
		if(!m_mute)
			m_spu->play_cdda(0,secbuf[sectail]);
		sectail=(sectail+1)&(sector_buffer_size-1);

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

		loc.w=curpos.w;

		//

		UINT32 sector = msf_to_lba_ps(loc.w);

		if (mode&mode_autopause)
		{
			if (sector>=autopause_sector)
			{
#ifdef debug_cdrom
				printf("autopause cdda\n");
#endif
				stop_read();
				command_result *res=global_alloc(command_result);
				res->res=intr_dataend;
				res->data[0]=status;
				res->sz=1;
				cmd_complete(res);
				return;
			}
		}

		if ((mode&mode_report) && !(sector & 15)) // slow the int rate
		{
			command_result *res=global_alloc(command_result);
			UINT8 track = cdrom_get_track(m_cdrom_handle, sector) + 1;
			res->res=intr_dataready;

			res->data[0]=status;
			res->data[1]=decimal_to_bcd(track);
			res->data[2]=1;
			if(sector & 0x10)
			{
				CDPOS tloc, start;
				start.w = (track == 1) ? 0x000200 : lba_to_msf_ps(cdrom_get_track_start(m_cdrom_handle, track - 1));
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

		unsigned int cyc=read_sector_cycles;

		next_sector_t+=cyc>>1;

		next_read_event = add_system_event(event_play_sector, next_sector_t - m_maincpu->total_cycles(), NULL);

		if(!read_next_sector())
		{
			stop_read(); // assume we've reached the end
			command_result *res=global_alloc(command_result);
			res->res=intr_dataend;
			res->data[0]=status;
			res->sz=1;
			cmd_complete(res);
		}
	}
}


//
//
//

void psxcd_device::preread_sector()
{
	UINT64 next_clock;
	int type;
	next_read_event=-1;

	UINT32 pos=msf_to_lba_ps(curpos.w);
	//
	if(!(mode & mode_cdda) && (cdrom_get_track_type(m_cdrom_handle, cdrom_get_track(m_cdrom_handle, pos + 150)) == CD_TRACK_AUDIO))
	{
		command_result *res=global_alloc(command_result);
		res->res=intr_diskerror;
		res->data[0]=status | status_error;
		res->data[1]=0x40;
		res->sz=2;
		cmd_complete(res);
		return;
	}

	unsigned char *buf=secbuf[sechead];
	if (! cdrom_read_data(m_cdrom_handle, pos, buf, CD_TRACK_RAW_DONTCARE))
	{
		next_clock=(m_sysclock/60);
		type=event_preread_sector;

		unsigned int cyc=read_sector_cycles;
		if (mode&mode_double_speed) cyc>>=1;
		next_sector_t=next_clock+(cyc-preread_delay)+m_maincpu->total_cycles();
	} else
	{
		memcpy(lastsechdr,buf+12,8);

		//

		command_result *res=global_alloc(command_result);
		res->res=intr_complete;
		res->data[0]=status;
		res->sz=1;

		#ifdef debug_cdrom
			printf("cdrom: read acknowledge\n");
		#endif

		cmd_complete(res);

		//

		next_clock=next_sector_t - m_maincpu->total_cycles();
		type=event_read_sector;

		//read_next_sector();
	}

	next_read_event = add_system_event(type, next_clock, NULL);
}

//
//
//

void psxcd_device::start_read()
{
	#ifdef debug_cdrom
		printf("cdrom: start read\n");
	#endif

	assert((status&(status_reading|status_playing))==0);

	status|=status_reading;

	sechead=sectail=0;

	unsigned int cyc=read_sector_cycles;
	if (mode&mode_double_speed) cyc>>=1;

	INT64 systime=m_maincpu->total_cycles();

	systime+=start_read_delay;

	next_sector_t=systime+cyc;
	next_read_event = add_system_event(event_preread_sector, start_read_delay+preread_delay, NULL);
}

//
//
//

void psxcd_device::start_streaming()
{
	assert(! streaming);

	streaming=true;
	start_read();
}

//
//
//

void psxcd_device::start_play()
{
	#ifdef debug_cdrom
		printf("cdrom: start play\n");
	#endif

	status|=status_playing;

	sechead=sectail=0;

	if (mode&mode_autopause)
	{
		UINT8 track = cdrom_get_track(m_cdrom_handle, msf_to_lba_ps(curpos.w) + 150);
		autopause_sector = cdrom_get_track_start(m_cdrom_handle, track) + cdrom_get_toc(m_cdrom_handle)->tracks[track].frames;
//      printf("pos=%d auto=%d\n",pos,autopause_sector);
	}

	unsigned int cyc=read_sector_cycles;

	next_sector_t=m_maincpu->total_cycles()+cyc;

	next_sector_t+=cyc>>1;

	next_read_event = add_system_event(event_play_sector, next_sector_t - m_maincpu->total_cycles(), NULL);

	read_next_sector();
}

//
//
//

void psxcd_device::stop_read()
{
	#ifdef debug_cdrom
		if (status&status_reading)
			printf("cdrom: stop read\n");
	#endif
	status&=~(status_reading|status_playing);
	streaming=false;

	if (next_read_event != -1)
	{
		m_timers[next_read_event]->adjust(attotime::never, 0, attotime::never);
		m_timerinuse[next_read_event] = false;
		next_read_event = -1;
	}

	UINT32 sector=msf_to_lba_ps(curpos.w);
	m_spu->flush_xa(sector);
	m_spu->flush_cdda(sector);
}

//
//
//
void psxcd_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	if (!m_timerinuse[tid])
	{
		printf("cdrom:: timer fired for free event\n");
		return;
	}

	m_timerinuse[tid] = false;
	switch (param)
	{
		case event_cmd_complete:
		{
			#ifdef debug_cdrom
				printf("cdrom:: event cmd complete\n");
			#endif

			cmd_complete((command_result *)ptr);
			break;
		}

		case event_preread_sector:
			preread_sector();
			break;

		case event_read_sector:
			read_sector();
			break;

		case event_play_sector:
			play_sector();
			break;

		case event_change_disk:
			open = false;
			status |= status_standby;
			break;
	}
}

int psxcd_device::add_system_event(int type, UINT64 t, void *ptr)
{
	// t is in maincpu clock cycles
	UINT32 hz = m_sysclock / t;
//  printf("add_system_event: event type %d for %d hz (using timer %d)\n", ev->type, hz, tnum);
	for(int i = 0; i < MAX_PSXCD_TIMERS; i++)
	{
		if(!m_timerinuse[i])
		{
			m_timers[i]->adjust(attotime::from_hz(hz), type, attotime::never);
			m_timers[i]->set_ptr(ptr);
			m_timerinuse[i] = true;
			return i;
		}
	}
	fatalerror("psxcd: out of timers\n");
	return 0;
}
