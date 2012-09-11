#include "emu.h"
#include "includes/psx.h"
#include "psxcd.h"
#include "machine/devhelpr.h"
#include "sound/spu.h"
#include "debugger.h"

//
//
//

//#define debug_cdrom
//#define debug_cdrom_registers
//#define skip_reads
//#define dump_subheader
//#define disable_xa_prefetch
//#define disable_cdda_prefetch

//
//
//

enum cdrom_events
{
	event_cmd_complete=0,
	event_preread_sector,
	event_read_sector,
	event_play_sector
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

static const unsigned int max_xa_prefetch_distance=32,
													max_cdda_prefetch_distance=32;

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PSXCD = &device_creator<psxcd_device>;

void psxcd_device::static_set_devname(device_t &device, const char *devname)
{
	psxcd_device &psxcd = downcast<psxcd_device &>(device);
	psxcd.m_devname = devname;
}

psxcd_device::psxcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PSXCD, "PSXCD", tag, owner, clock)
{
}

void psxcd_device::device_start()
{
	unsigned int sysclk=machine().device<cpu_device>("maincpu")->clock()/2;
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
	secin = 0;
	secskip = 0;
	next_read_event = NULL;
	cbp = cmdbuf;
	first_open = true;

	status=status_standby;
	sr=8|1;
	res=0;
	ir=0;
	mode=0;

	driver = NULL;

	for (int i = 0; i < MAX_PSXCD_TIMERS; i++)
	{
		m_timers[i] = timer_alloc(i, NULL);
		m_timerinuse[i] = false;
	}
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

	m_cddevice = machine().device<cdrom_image_device>(m_devname);
	if (m_cddevice)
	{
		m_cd = m_cddevice->get_cdrom_file();

		if (m_cd)
		{
//          printf("psxcd: found disc!\n");
			driver = open_mess_drv();
			driver->set_machine(machine());
			driver->set_cdrom_file(m_cd);
		}
		else
		{
			driver = NULL;
//          printf("psxcd: Found device, but no disc\n");
		}
	}
	else
	{
		driver = NULL;
//      printf("psxcd: Device [%s] not found!\n", m_devname);
	}

}

//
//
//

unsigned char psxcd_device::read_byte(const unsigned int addr)
{
	unsigned char ret = 0;

	switch (addr&3)
	{
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
		printf("cdrom: read byte %08x = %02x (PC=%x)\n",addr,ret,machine().device("maincpu")->safe_pc());
	#endif

	return ret;
}

//
//
//

void psxcd_device::write_byte(const unsigned int addr, const unsigned char byte)
{
	#ifdef debug_cdrom_registers
		printf("cdrom: write byte %08x = %02x (PC=%x)\n",addr,byte,machine().device("maincpu")->safe_pc());
	#endif

	switch (addr&3)
	{
		case 0:
			cmdmode=byte&1;
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
				write_command(byte);
			}
			break;

		case 2:
			if (cmdmode==0)
			{
				*cbp++=byte;
			} else
			{
				// ?flush buffer?
			}
			break;

		case 3:
			if (byte==0x07)
			{
				if (cur_res)
				{
					global_free(cur_res);
					cur_res=NULL;
					sr&=~(1<<5);
				}
				ir=0;
			}
			break;
	}
}

psxcd_device::command_info psxcd_device::cmd_table[num_commands]=
{
	{ &psxcd_device::cdcmd_sync,				"sync"			},	// 00
	{ &psxcd_device::cdcmd_nop,				"nop"				},	// 01
	{ &psxcd_device::cdcmd_setloc,			"setloc"		},	// 02
	{ &psxcd_device::cdcmd_play,				"play"			},	// 03
	{ &psxcd_device::cdcmd_forward,		"forward"		},	// 04
	{ &psxcd_device::cdcmd_backward,		"backward"	},	// 05
	{ &psxcd_device::cdcmd_readn,			"readn"			},	// 06
	{ &psxcd_device::cdcmd_standby,		"standby"		},	// 07
	{ &psxcd_device::cdcmd_stop,				"stop"			},	// 08
	{ &psxcd_device::cdcmd_pause,			"pause"			},	// 09
	{ &psxcd_device::cdcmd_init,				"init"			},	// 0a
	{ &psxcd_device::cdcmd_mute,				"mute"			},	// 0b
	{ &psxcd_device::cdcmd_demute,			"demute"		},	// 0c
	{ &psxcd_device::cdcmd_setfilter,	"setfilter"	},	// 0d
	{ &psxcd_device::cdcmd_setmode,		"setmode"		},	// 0e
	{ &psxcd_device::cdcmd_getparam,		"getparam"	},	// 0f
	{ &psxcd_device::cdcmd_getlocl,		"getlocl"		},	// 10
	{ &psxcd_device::cdcmd_getlocp,		"getlocp"		},	// 11
	{ &psxcd_device::cdcmd_illegal,		"illegal"		},	// 12
	{ &psxcd_device::cdcmd_gettn,			"gettn"			},	// 13
	{ &psxcd_device::cdcmd_gettd,			"gettd"			},	// 14
	{ &psxcd_device::cdcmd_seekl,			"seekl"			},	// 15
	{ &psxcd_device::cdcmd_seekp,			"seekp"			},	// 16
	{ &psxcd_device::cdcmd_illegal,		"illegal"		},	// 17
	{ &psxcd_device::cdcmd_illegal,		"illegal"		},	// 18
	{ &psxcd_device::cdcmd_test,				"test"			},	// 19
	{ &psxcd_device::cdcmd_id,					"id"				},	// 1a
	{ &psxcd_device::cdcmd_reads,			"reads"			},	// 1b
	{ &psxcd_device::cdcmd_reset,			"reset"			},	// 1c
	{ &psxcd_device::cdcmd_illegal,		"illegal"		},	// 1d
	{ &psxcd_device::cdcmd_readtoc,		"readtoc"		},	// 1e
};

//
//
//

void psxcd_device::write_command(const unsigned char byte)
{
	assert(byte<num_commands);
	(this->*cmd_table[byte].func)();
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

static int open_nops=0;

void psxcd_device::cdcmd_nop()
{
	#ifdef debug_cdrom
		printf("cdrom: nop\n");
	#endif

	//stop_read();

	if ((! open) && (driver))
	{
		if (open_nops==0)
		{
			open=driver->read_toc();
		} else
		{
			open_nops--;
		}
	}

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

	unsigned char l[3];
	l[0]=bcd_to_decimal(cmdbuf[0]);
	l[1]=bcd_to_decimal(cmdbuf[1]);
	l[2]=bcd_to_decimal(cmdbuf[2]);

	if ((l[0]>0) || (l[1]>=2))
	{
		loc[0]=l[0];
		loc[1]=l[1];
		loc[2]=l[2];
	} else
	{
		printf("setloc out of range: %02d:%02d:%02d\n",
					 l[0],l[1],l[2]);
	}

	send_result(intr_complete);
}

void psxcd_device::cdcmd_play()
{
	#ifdef debug_cdrom
		printf("cdrom: play %02x %02x %02x => %d\n", loc[0], loc[1], loc[2], msf_to_sector(loc));
	#endif

	curpos[0]=loc[0];
	curpos[1]=loc[1];
	curpos[2]=loc[2];

	if ((curpos[0]==0) &&
			(curpos[1]==0) &&
			(curpos[2]==0))
	{
		send_result(intr_acknowledge);
	} else
	{
		stop_read();
		start_play();
		send_result(intr_acknowledge);
	}
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
	if (driver)
	{
		#ifdef debug_cdrom
			printf("cdrom: readn\n");
		#endif

		curpos[0]=loc[0];
		curpos[1]=loc[1];
		curpos[2]=loc[2];

		stop_read();
		start_read();
		send_result(intr_complete);
	} else
	{
		printf("read without driver\n");
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

	send_result(intr_acknowledge);
}

void psxcd_device::cdcmd_demute()
{
	#ifdef debug_cdrom
		printf("cdrom: demute\n");
	#endif

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

#if 0
static void add_loc(unsigned char *dst, const unsigned char *src1, const unsigned char *src2)
{
	int f=src1[2]+src2[2],
			s=src1[1]+src2[1],
			m=src1[0]+src2[0];
	while (f>=75) { s++; f-=75; }
	while (s>=60) { m++; s-=60; }

	dst[0]=m;
	dst[1]=s;
	dst[2]=f;
}
#endif
static void sub_loc(unsigned char *dst, const unsigned char *src1, const unsigned char *src2)
{
	int f=src1[2]-src2[2],
			s=src1[1]-src2[1],
			m=src1[0]-src2[0];
	while (f<0) { s--; f+=75; }
	while (s<0) { m--; s+=60; }

	if (m<0)
		m=s=f=0;

	dst[0]=m;
	dst[1]=s;
	dst[2]=f;
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
	unsigned char tloc[3],
								twosec[3]={ 0,2,0 };
	sub_loc(tloc,loc,twosec);

	unsigned char data[8]=
	{
		0x01,							// track
		0x01,							// index
		decimal_to_bcd(tloc[0]),	// min
		decimal_to_bcd(tloc[1]),	// sec
		decimal_to_bcd(tloc[2]),	// frame
		decimal_to_bcd(loc[0]),	// amin
		decimal_to_bcd(loc[1]),	// asec
		decimal_to_bcd(loc[2])	// aframe
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

	assert(driver);

	unsigned char data[3]=
	{
		status,
		decimal_to_bcd(driver->get_first_track()),
		decimal_to_bcd(driver->get_num_tracks())
	};

	//stop_read();
	send_result(intr_acknowledge,data,3);
}

void psxcd_device::cdcmd_gettd()
{
	unsigned char addr[3];
	driver->get_track_address(bcd_to_decimal(cmdbuf[0]), addr);

	unsigned char data[3]=
	{
		status,
		decimal_to_bcd(addr[0]),
		decimal_to_bcd(addr[1])
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

void psxcd_device::cdcmd_seekl()
{
	#ifdef debug_cdrom
		printf("cdrom: seekl [%02d:%02d:%02d]\n",loc[0],loc[1],loc[2]);
	#endif

	curpos[0]=loc[0];
	curpos[1]=loc[1];
	curpos[2]=loc[2];

	send_result(intr_acknowledge);
}

void psxcd_device::cdcmd_seekp()
{
	#ifdef debug_cdrom
		printf("cdrom: seekp\n");
	#endif

	curpos[0]=loc[0];
	curpos[1]=loc[1];
	curpos[2]=loc[2];

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

	if (! open)
	{
		if (driver)
			open=driver->read_toc();
	}

	if (open)
	{
		static unsigned char gamedata[8] = { 0x08, 0x00, 0x00, 0x00, 'S', 'C', 'E', 'A' };
		static unsigned char audiodata[8] = { 0x08, 0x90, 0x00, 0x00, 'S', 'C', 'E', 'A' };	// drops into the audio CD player.  08 80 goes to the menu.

		if (cdrom_get_track_type(m_cd, 0) == CD_TRACK_AUDIO)
		{
			send_result(intr_acknowledge,audiodata,8);
		}
		else
		{
			send_result(intr_acknowledge,gamedata,8);
		}
	} else
	{
		status=status_error|status_shellopen;
		send_result(intr_diskerror);
	}
}

void psxcd_device::cdcmd_reads()
{
	curpos[0]=loc[0];
	curpos[1]=loc[1];
	curpos[2]=loc[2];

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
		psx_irq_set(machine(), 0x0004);
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

event *psxcd_device::send_result(const unsigned int res,
														const unsigned char *data,
														const unsigned int sz,
															const unsigned int delay)
{
	// Update shell open status

	if (! open)
	{
		status=status_error|status_shellopen;
	}
	else
	{
		if (status&status_shellopen)
			status=status_standby;
	}

	// Prepare event

	event *ev=new event;
	ev->t=delay;
	ev->type=event_cmd_complete;

	command_result *cr=(command_result *)ev->data;

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

	// Avoid returning results after sector read results -
	// delay the sector read slightly if necessary

	UINT64 systime = machine().device<cpu_device>("maincpu")->total_cycles();
	if ((next_read_event) && ((systime+ev->t)>(next_sector_t)))
	{
		UINT32 hz = m_sysclock / (delay + 2000);
		next_read_event->timer->adjust(attotime::from_hz(hz), 0, attotime::never);

		for (int i = 0; i < MAX_PSXCD_TIMERS; i++)
		{
			if (m_eventfortimer[i] == next_read_event)
			{
				printf("Adjusting timer %d to %d hz]n", i, hz);
			}
		}
	}

	add_system_event(ev);
	return ev;
}

//
//
//

void psxcd_device::start_dma(UINT8 *mainram, UINT32 size)
{
	if ((int)size>secleft)
	{
		printf("cdrom: dma past end of sector (secleft=%d sz=%d)\n",secleft,size);
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

//
//
//

void psxcd_device::prefetch_next_sector()
{
	unsigned int pos=msf_to_sector(curpos);
	assert(driver);
	driver->prefetch_sector(pos);
}

//
//
//

bool psxcd_device::read_next_sector(const bool block)
{
	unsigned int pos=msf_to_sector(curpos);
	unsigned char *buf=&secbuf[sechead*raw_sector_size];
	assert(driver);
	assert(secin<sector_buffer_size);

//  printf("read_next_sector: sec %d, sechead %d, raw_sector_size %d\n", pos, sechead, raw_sector_size);
	if (driver->read_sector(pos, buf, block))
	{
//      printf("buf contents = %02x %02x | %02x %02x\n", buf[0], buf[1], buf[0x20], buf[0x21]);

		sechead=(sechead+1)&(sector_buffer_size-1);
		secin++;

		memcpy(lastsechdr,&secbuf[raw_sector_size*sectail]+12,8);

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
	next_read_event=NULL;

	if (status & status_reading)
	{
		bool isend=false;

		if (read_next_sector(false))
		{
			unsigned char *rawsec;

			if ((mode&mode_adpcm) && (streaming))
			{
				rawsec=&secbuf[raw_sector_size*sectail];
				secptr=rawsec+24;
				secleft=2048;
			}
			else
			{
				rawsec=&secbuf[raw_sector_size*sectail];
				secptr=rawsec+secskip;
				secleft=secsize;
			}

			secin--;
			sectail=(sectail+1)&(sector_buffer_size-1);

			//

			bool isxa=false;

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
			isxa=((mode&mode_adpcm) && (sub->submode&submode_audio));

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

					#ifndef disable_xa_prefetch
						unsigned int cursec=msf_to_sector(curpos);

						if (xa_prefetch_sector==-1)
						{
							xa_prefetch_sector=cursec;
							machine().device<spu_device>("spu")->flush_xa();
						}

						unsigned char *xaptr;
						unsigned int xasecsz;

						while (1)
						{
							int dist=xa_prefetch_sector-cursec;
							if (dist>max_xa_prefetch_distance) break;

							xaptr=driver->get_prefetch_sector(xa_prefetch_sector,&xasecsz);
							if (! xaptr) break;

							switch (xasecsz)
							{
								case 2336:
									break;

								default:
									xaptr+=16;
									break;
							}

							subheader *xasub=(subheader *)xaptr;

							if ((xasub->submode&submode_audio) &&
									(((mode&mode_channel)==0) ||
									 ((xasub->file==sub->file) &&
										(xasub->channel==sub->channel))))
							{
								if (! machine().device<spu_device>("spu")->play_xa(xa_prefetch_sector,xaptr))
									break;
							}
							xa_prefetch_sector++;
						}
					#else
						machine().device<spu_device>("spu")->play_xa(0,rawsec+16);
					#endif

					status|=status_playing;
				}
			}

			if ((mode&mode_autopause)==0)
				isend=false;

			//

			curpos[2]++;
			if (curpos[2]==75)
			{
				curpos[2]=0;
				curpos[1]++;
				if (curpos[1]==60)
				{
					curpos[1]=0;
					curpos[0]++;
				}
			}

			loc[0]=curpos[0];
			loc[1]=curpos[1];
			loc[2]=curpos[2];

			//

			command_result *res=new command_result;
			res->res=isend?intr_dataend:intr_dataready;
			res->data[0]=status;
			res->sz=1;

			sr|=0x40;

			if ((streaming) && (isxa))
			{
				global_free(res);
				res=NULL;
			}

			if (res)
			{
				#ifdef debug_cdrom
					printf("cdrom:: data ready\n");
				#endif

				cmd_complete(res);
			}
		}

		if (! isend)
		{
			unsigned int cyc=read_sector_cycles;
			if (mode&mode_double_speed) cyc>>=1;

			next_sector_t+=cyc;

			event *ev=new event;
			ev->t=cyc;
			ev->type=event_read_sector;
			next_read_event=ev;
			add_system_event(ev);

			//read_next_sector();
		} else
		{
			printf("autopause xa\n");
			stop_read();
		}
	}
}

//
//
//

bool psxcd_device::play_cdda_sector(const unsigned int sector,
																 unsigned char *rawsec)
{
	bool isdata=true;

	if (rawsec[0]!=0)
	{
		isdata=false;
	} else
	{
		for (int i=0; i<10; i++)
			if (rawsec[i+1]!=0xff)
			{
				isdata=false;
				break;
			}
	}

	if (! isdata)
	{
		return machine().device<spu_device>("spu")->play_cdda(sector,rawsec);
	} else
	{
		return true;
	}
}

//
//
//

void psxcd_device::play_sector()
{
	next_read_event=NULL;

	if (status&status_playing)
	{
		#ifdef disable_cdda_prefetch
		unsigned char *rawsec=&secbuf[raw_sector_size*sectail];
		#endif
		secin--;
		sectail=(sectail+1)&(sector_buffer_size-1);

		//

		#ifndef disable_cdda_prefetch
			unsigned int cursec=msf_to_sector(curpos);

			if (cdda_prefetch_sector==-1)
			{
				cdda_prefetch_sector=cursec;
				machine().device<spu_device>("spu")->flush_cdda();
			}

			unsigned char *cddaptr;
//          bool full=false;
			unsigned int cddasecsz;

			while (1)
			{
				int dist=cdda_prefetch_sector-cursec;
				if (dist>max_cdda_prefetch_distance) break;

				cddaptr=driver->get_prefetch_sector(cdda_prefetch_sector,&cddasecsz);
				if (! cddaptr) break;

				if (! play_cdda_sector(cdda_prefetch_sector,cddaptr))
					break;

				cdda_prefetch_sector++;
			}
		#else
			play_cdda_sector(0,rawsec);
		#endif

		//

		curpos[2]++;
		if (curpos[2]==75)
		{
			curpos[2]=0;
			curpos[1]++;
			if (curpos[1]==60)
			{
				curpos[1]=0;
				curpos[0]++;
			}
		}

		loc[0]=curpos[0];
		loc[1]=curpos[1];
		loc[2]=curpos[2];

		//

		if (mode&mode_autopause)
		{
			if (msf_to_sector(loc)>=autopause_sector)
			{
				printf("autopause cdda\n");

				command_result *res=new command_result;
				res->res=intr_dataend;
				res->data[0]=status_standby;
				res->sz=1;
				cmd_complete(res);
				stop_read();
				return;
			}
		}

		if (mode&mode_report)
		{
			command_result *res=new command_result;
			res->res=intr_complete;	//dataready;

			res->data[0]=status_playing|status_standby;
			res->data[1]=0x01;
			res->data[2]=0x80;
			res->data[3]=decimal_to_bcd(loc[0]);
			res->data[4]=decimal_to_bcd(loc[1])|0x80;
			res->data[5]=decimal_to_bcd(loc[2]);

			res->sz=8;

			cmd_complete(res);
		}

		unsigned int cyc=read_sector_cycles;

		event *ev=new event;
		ev->t=next_sector_t - machine().device<cpu_device>("maincpu")->total_cycles();
		ev->type=event_play_sector;

		next_sector_t+=cyc;

		next_read_event=ev;
		add_system_event(ev);

		read_next_sector();
	}
}


//
//
//

void psxcd_device::preread_sector()
{
	next_read_event=NULL;

	//

	event *ev=new event;

	unsigned int pos=msf_to_sector(curpos);
	unsigned char *buf=&secbuf[sechead*raw_sector_size];
	if (! driver->read_sector(pos,buf,false))
	{
		ev->t=(m_sysclock/60);
		ev->type=event_preread_sector;

		unsigned int cyc=read_sector_cycles;
		if (mode&mode_double_speed) cyc>>=1;
		next_sector_t=ev->t+(cyc-preread_delay)+machine().device<cpu_device>("maincpu")->total_cycles();
	} else
	{
		memcpy(lastsechdr,buf+12,8);

		//

		command_result *res=new command_result;
		res->res=intr_complete;
		res->data[0]=status;
		res->sz=1;

		#ifdef debug_cdrom
			printf("cdrom: read acknowledge\n");
		#endif

		cmd_complete(res);

		//

		ev->t=next_sector_t - machine().device<cpu_device>("maincpu")->total_cycles();
		ev->type=event_read_sector;

		//read_next_sector();
	}

	next_read_event=ev;
	add_system_event(ev);
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

	secin=sechead=sectail=0;
	xa_prefetch_sector=-1;

	unsigned int cyc=read_sector_cycles;
	if (mode&mode_double_speed) cyc>>=1;

	INT64 systime=machine().device<cpu_device>("maincpu")->total_cycles();

	systime+=start_read_delay;

	event *ev=new event;
	ev->t=start_read_delay+preread_delay;
	ev->type=event_preread_sector;

	next_sector_t=systime+cyc;
	next_read_event=ev;
	add_system_event(ev);

	prefetch_next_sector();
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

	secin=sechead=sectail=0;
	cdda_prefetch_sector=-1;

	if (mode&mode_autopause)
	{
		unsigned int pos=msf_to_sector(curpos);
		driver->find_track(pos+150,NULL,&autopause_sector);
//      printf("pos=%d auto=%d\n",pos,autopause_sector);
	}

	unsigned int cyc=read_sector_cycles;

	next_sector_t=machine().device<cpu_device>("maincpu")->total_cycles()+cyc;

	event *ev=new event;
	ev->t=next_sector_t - machine().device<cpu_device>("maincpu")->total_cycles();
	ev->type=event_play_sector;

	next_sector_t+=cyc;

	next_read_event=ev;
	add_system_event(ev);

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

	if (next_read_event)
	{
		next_read_event->timer->adjust(attotime::never, 0, attotime::never);
		for (int i = 0; i < MAX_PSXCD_TIMERS; i++)
		{
			if (m_timers[i] == next_read_event->timer)
			{
				m_timerinuse[i] = false;
				m_eventfortimer[i] = NULL;
				break;
			}
		}
		global_free(next_read_event);
		next_read_event=NULL;
	}

	unsigned int sector=msf_to_sector(curpos);
	machine().device<spu_device>("spu")->flush_xa(sector);
	machine().device<spu_device>("spu")->flush_cdda(sector);
}

//
//
//

void psxcd_device::set_driver(cdrom_driver *d)
{
	char err[1024];

	if (d)
	{
		if (d->is_usable(err,1024))
		{
			driver=d;
			open=false;

			if (! first_open)
			{
				command_result *res=new command_result;
				res->res=intr_acknowledge;
				res->data[0]=status_standby;
				res->sz=1;

				cmd_complete(res);

				open_nops=10;
			}

			first_open=false;
		}
	} else
	{
		driver=NULL;
		open=false;

		command_result *res=new command_result;
		res->res=intr_acknowledge;
		res->data[0]=status_shellopen;
		res->sz=1;

		cmd_complete(res);
	}
}

//
//
//
void psxcd_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	event *ev = m_eventfortimer[tid];

//  printf("timer %d fired, performing event type %d\n", tid, ev->type);

	switch (ev->type)
	{
		case event_cmd_complete:
		{
			#ifdef debug_cdrom
				printf("cdrom:: event cmd complete\n");
			#endif

			command_result *res=new command_result;
			memcpy(res,ev->data,sizeof(command_result));
			cmd_complete(res);
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
	}

	// free the timer
//  printf("Freeing timer %d\n", tid);
	m_timers[tid]->adjust(attotime::never, 0, attotime::never);
	m_timerinuse[tid] = false;
	m_eventfortimer[tid] = NULL;

	global_free(ev);
}

void psxcd_device::add_system_event(event *ev)
{
	emu_timer *timer = NULL;
	int tnum = -1;

	// allocate a timer for this event
	for (int i = 0; i < MAX_PSXCD_TIMERS; i++)
	{
		if (!m_timerinuse[i])
		{
			tnum = i;
			timer = m_timers[i];
			m_timerinuse[i] = true;
			m_eventfortimer[i] = ev;
			break;
		}
	}

	if (tnum == -1)
	{
		fatalerror("PSXCD: ran out of timers!\n");
	}

	// ev->t is in maincpu clock cycles
	UINT32 hz = m_sysclock / ev->t;
//  printf("add_system_event: event type %d for %d hz (using timer %d)\n", ev->type, hz, tnum);
	timer->adjust(attotime::from_hz(hz), tnum, attotime::never);

	// back-reference the timer from the event
	ev->timer = timer;
}

