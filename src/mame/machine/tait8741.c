/*

Taito 8741 emulation

1.The pair chip for the PIO and serial communication between MAIN CPU and the sub CPU
2.The PIO for DIP SW and the controller reading.

*/

#include "emu.h"
#include "tait8741.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) printf x; } while (0)

/****************************************************************************

gladiatr and Great Swordsman set.

  -comminucation main and sub cpu
  -dipswitch and key handling x 2chip

  Total 4chip

  It was supposed from the schematic of gladiator.
  Now, because dump is done, change the MCU code of gladiator to the CPU emulation.

****************************************************************************/

#define CMD_IDLE 0
#define CMD_08 1
#define CMD_4a 2

struct I8741 {
	UINT8 toData;    /* to host data      */
	UINT8 fromData;  /* from host data    */
	UINT8 fromCmd;   /* from host command */
	UINT8 status;    /* b0 = rd ready,b1 = wd full,b2 = cmd ?? */
	UINT8 mode;
	UINT8 phase;
	UINT8 txd[8];
	UINT8 rxd[8];
	UINT8 parallelselect;
	UINT8 txpoint;
	int connect;
	UINT8 pending4a;
	int serial_out;
	int coins;
	read8_space_func portHandler;
	const  char *portName;
};

static const struct TAITO8741interface *intf;
//static I8741 *taito8741;
static I8741 taito8741[MAX_TAITO8741];

/* for host data , write */
static void taito8741_hostdata_w(I8741 *st,int data)
{
	st->toData = data;
	st->status |= 0x01;
}

/* from host data , read */
static int taito8741_hostdata_r(I8741 *st)
{
	if( !(st->status & 0x02) ) return -1;
	st->status &= 0xfd;
	return st->fromData;
}

/* from host command , read */
static int taito8741_hostcmd_r(I8741 *st)
{
	if(!(st->status & 0x04)) return -1;
	st->status &= 0xfb;
	return st->fromCmd;
}


/* TAITO8741 I8741 emulation */

static void taito8741_serial_rx(I8741 *st,UINT8 *data)
{
	memcpy(st->rxd,data,8);
}

/* timer callback of serial tx finish */
static TIMER_CALLBACK( taito8741_serial_tx )
{
	int num = param;
	I8741 *st = &taito8741[num];
	I8741 *sst;

	if( st->mode==TAITO8741_MASTER)
		st->serial_out = 1;

	st->txpoint = 1;
	if(st->connect >= 0 )
	{
		sst = &taito8741[st->connect];
		/* transfer data */
		taito8741_serial_rx(sst,st->txd);
		LOG(("8741-%d Serial data TX to %d\n",num,st->connect));
		if( sst->mode==TAITO8741_SLAVE)
			sst->serial_out = 1;
	}
}

void TAITO8741_reset(int num)
{
	I8741 *st = &taito8741[num];
	st->status = 0x00;
	st->phase = 0;
	st->parallelselect = 0;
	st->txpoint = 1;
	st->pending4a = 0;
	st->serial_out = 0;
	st->coins = 0;
	memset(st->rxd,0,8);
	memset(st->txd,0,8);
}

/* 8741 update */
static void taito8741_update(address_space &space, int num)
{
	I8741 *st,*sst;
	int next = num;
	int data;

	do{
		num = next;
		st = &taito8741[num];
		if( st->connect != -1 )
			 sst = &taito8741[st->connect];
		else sst = 0;
		next = -1;
		/* check pending command */
		switch(st->phase)
		{
		case CMD_08: /* serial data latch */
			if( st->serial_out)
			{
				st->status &= 0xfb; /* patch for gsword */
				st->phase = CMD_IDLE;
				next = num; /* continue this chip */
			}
			break;
		case CMD_4a: /* wait for syncronus ? */
			if(!st->pending4a)
			{
				taito8741_hostdata_w(st,0);
				st->phase = CMD_IDLE;
				next = num; /* continue this chip */
			}
			break;
		case CMD_IDLE:
			/* ----- data in port check ----- */
			data = taito8741_hostdata_r(st);
			if( data != -1 )
			{
				switch(st->mode)
				{
				case TAITO8741_MASTER:
				case TAITO8741_SLAVE:
					/* buffering transmit data */
					if( st->txpoint < 8 )
					{
//if (st->txpoint == 0 && num==1 && data&0x80) logerror("Coin Put\n");
						st->txd[st->txpoint++] = data;
					}
					break;
				case TAITO8741_PORT:
					if( data & 0xf8)
					{ /* ?? */
					}
					else
					{ /* port select */
						st->parallelselect = data & 0x07;
						taito8741_hostdata_w(st,st->portHandler ? st->portHandler(space,st->parallelselect) : st->portName ? space.machine().root_device().ioport(st->portName)->read() : 0);
					}
				}
			}
			/* ----- new command fetch ----- */
			data = taito8741_hostcmd_r(st);
			switch( data )
			{
			case -1: /* no command data */
				break;
			case 0x00: /* read from parallel port */
				taito8741_hostdata_w(st,st->portHandler ? st->portHandler(space,0) : st->portName ? space.machine().root_device().ioport(st->portName)->read() : 0 );
				break;
			case 0x01: /* read receive buffer 0 */
			case 0x02: /* read receive buffer 1 */
			case 0x03: /* read receive buffer 2 */
			case 0x04: /* read receive buffer 3 */
			case 0x05: /* read receive buffer 4 */
			case 0x06: /* read receive buffer 5 */
			case 0x07: /* read receive buffer 6 */
//if (data == 2 && num==0 && st->rxd[data-1]&0x80) logerror("Coin Get\n");
				taito8741_hostdata_w(st,st->rxd[data-1]);
				break;
			case 0x08:	/* latch received serial data */
				st->txd[0] = st->portHandler ? st->portHandler(space,0) : st->portName ? space.machine().root_device().ioport(st->portName)->read() : 0;
				if( sst )
				{
					space.machine().scheduler().synchronize(FUNC(taito8741_serial_tx), num);
					st->serial_out = 0;
					st->status |= 0x04;
					st->phase = CMD_08;
				}
				break;
			case 0x0a:	/* 8741-0 : set serial comminucation mode 'MASTER' */
				//st->mode = TAITO8741_MASTER;
				break;
			case 0x0b:	/* 8741-1 : set serial comminucation mode 'SLAVE'  */
				//st->mode = TAITO8741_SLAVE;
				break;
			case 0x1f:  /* 8741-2,3 : ?? set parallelport mode ?? */
			case 0x3f:  /* 8741-2,3 : ?? set parallelport mode ?? */
			case 0xe1:  /* 8741-2,3 : ?? set parallelport mode ?? */
				st->mode = TAITO8741_PORT;
				st->parallelselect = 1; /* preset read number */
				break;
			case 0x62:  /* 8741-3   : ? */
				break;
			case 0x4a:	/* ?? syncronus with other cpu and return 00H */
				if( sst )
				{
					if(sst->pending4a)
					{
						sst->pending4a = 0; /* syncronus */
						taito8741_hostdata_w(st,0); /* return for host */
						next = st->connect;
					}
					else st->phase = CMD_4a;
				}
				break;
			case 0x80:	/* 8741-3 : return check code */
				taito8741_hostdata_w(st,0x66);
				break;
			case 0x81:	/* 8741-2 : return check code */
				taito8741_hostdata_w(st,0x48);
				break;
			case 0xf0:  /* GSWORD 8741-1 : initialize ?? */
				break;
			case 0x82:  /* GSWORD 8741-2 unknown */
				break;
			}
			break;
		}
	}while(next>=0);
}

int TAITO8741_start(const struct TAITO8741interface *taito8741intf)
{
	int i;

	intf = taito8741intf;

	//taito8741 = (I8741 *)malloc(intf->num*sizeof(I8741));
	//if( taito8741 == 0 ) return 1;

	for(i=0;i<intf->num;i++)
	{
		taito8741[i].connect     = intf->serial_connect[i];
		taito8741[i].portHandler = intf->portHandler_r[i];
		taito8741[i].portName    = intf->portName_r[i];
		taito8741[i].mode        = intf->mode[i];
		TAITO8741_reset(i);
	}
	return 0;
}

/* read status port */
static int I8741_status_r(address_space &space, int num)
{
	I8741 *st = &taito8741[num];
	taito8741_update(space, num);
	LOG(("%s:8741-%d ST Read %02x\n",space.machine().describe_context(),num,st->status));
	return st->status;
}

/* read data port */
static int I8741_data_r(address_space &space, int num)
{
	I8741 *st = &taito8741[num];
	int ret = st->toData;
	st->status &= 0xfe;
	LOG(("%s:8741-%d DATA Read %02x\n",space.machine().describe_context(),num,ret));

	/* update chip */
	taito8741_update(space, num);

	switch( st->mode )
	{
	case TAITO8741_PORT: /* parallel data */
		taito8741_hostdata_w(st,st->portHandler ? st->portHandler(space, st->parallelselect) : st->portName ? space.machine().root_device().ioport(st->portName)->read() : 0);
		break;
	}
	return ret;
}

/* Write data port */
static void I8741_data_w(address_space &space, int num, int data)
{
	I8741 *st = &taito8741[num];
	LOG(("%s:8741-%d DATA Write %02x\n",space.machine().describe_context(),num,data));
	st->fromData = data;
	st->status |= 0x02;
	/* update chip */
	taito8741_update(space, num);
}

/* Write command port */
static void I8741_command_w(address_space &space, int num, int data)
{
	I8741 *st = &taito8741[num];
	LOG(("%s:8741-%d CMD Write %02x\n",space.machine().describe_context(),num,data));
	st->fromCmd = data;
	st->status |= 0x04;
	/* update chip */
	taito8741_update(space,num);
}

/* Write port handler */
WRITE8_HANDLER( TAITO8741_0_w )
{
	if(offset&1) I8741_command_w(space,0,data);
	else         I8741_data_w(space,0,data);
}
WRITE8_HANDLER( TAITO8741_1_w )
{
	if(offset&1) I8741_command_w(space,1,data);
	else         I8741_data_w(space,1,data);
}
WRITE8_HANDLER( TAITO8741_2_w )
{
	if(offset&1) I8741_command_w(space,2,data);
	else         I8741_data_w(space,2,data);
}
WRITE8_HANDLER( TAITO8741_3_w )
{
	if(offset&1) I8741_command_w(space,3,data);
	else         I8741_data_w(space,3,data);
}

/* Read port handler */
READ8_HANDLER( TAITO8741_0_r )
{
	if(offset&1) return I8741_status_r(space,0);
	return I8741_data_r(space,0);
}
READ8_HANDLER( TAITO8741_1_r )
{
	if(offset&1) return I8741_status_r(space,1);
	return I8741_data_r(space,1);
}
READ8_HANDLER( TAITO8741_2_r )
{
	if(offset&1) return I8741_status_r(space,2);
	return I8741_data_r(space,2);
}
READ8_HANDLER( TAITO8741_3_r )
{
	if(offset&1) return I8741_status_r(space,3);
	return I8741_data_r(space,3);
}
/****************************************************************************

joshi Vollyball set.

  Only the chip of the communication between MAIN and the SUB.
  For the I/O, there may be I8741 of the addition.

  MCU code is not dumped.
  There is some HACK operation because the emulation is imperfect.

****************************************************************************/

static int josvolly_nmi_enable;

struct JV8741  {
	UINT8 cmd;
	UINT8 sts;
	UINT8 txd;
	UINT8 outport;
	UINT8 rxd;
	UINT8 connect;

	UINT8 rst;

	const char *initReadPort;
};

static JV8741 i8741[4];

void josvolly_8741_reset(void)
{
	int i;

	josvolly_nmi_enable = 0;

	for(i=0;i<4;i++)
	{
		i8741[i].cmd = 0;
		i8741[i].sts = 0; // 0xf0; /* init flag */
		i8741[i].txd = 0;
		i8741[i].outport = 0xff;
		i8741[i].rxd = 0;

		i8741[i].rst = 1;

	}
	i8741[0].connect = 1;
	i8741[1].connect = 0;

	i8741[0].initReadPort = "DSW1";  /* DSW1 */
	i8741[1].initReadPort = "DSW2";  /* DSW2 */
	i8741[2].initReadPort = "DSW1";  /* DUMMY */
	i8741[3].initReadPort = "DSW2";  /* DUMMY */

}

/* transmit data finish callback */
static TIMER_CALLBACK( josvolly_8741_tx )
{
	int num = param;
	JV8741 *src = &i8741[num];
	JV8741 *dst = &i8741[src->connect];

	dst->rxd = src->txd;

	src->sts &= ~0x02; /* TX full ? */
	dst->sts |=  0x01; /* RX ready  ? */
}

static void josvolly_8741_do(running_machine &machine, int num)
{
	if( (i8741[num].sts & 0x02) )
	{
		/* transmit data */
		machine.scheduler().timer_set (attotime::from_usec(1), FUNC(josvolly_8741_tx), num);
	}
}

static void josvolly_8741_w(address_space &space, int num, int offset, int data)
{
	JV8741 *mcu = &i8741[num];

	if(offset==1)
	{
		LOG(("%s:8741[%d] CW %02X\n", space.machine().describe_context(), num, data));

		/* read pointer */
		mcu->cmd = data;
		/* CMD */
		switch(data)
		{
		case 0:
			mcu->txd = data ^ 0x40;
			mcu->sts |= 0x02;
			break;
		case 1:
			mcu->txd = data ^ 0x40;
			mcu->sts |= 0x02;
#if 1
			/* ?? */
			mcu->rxd = 0;  /* SBSTS ( DIAG ) , killed */
			mcu->sts |= 0x01; /* RD ready */
#endif
			break;
		case 2:
#if 1
			mcu->rxd = space.machine().root_device().ioport("DSW2")->read();
			mcu->sts |= 0x01; /* RD ready */
#endif
			break;
		case 3: /* normal mode ? */
			break;

		case 0xf0: /* clear main sts ? */
			mcu->txd = data ^ 0x40;
			mcu->sts |= 0x02;
			break;
		}
	}
	else
	{
		/* data */
		LOG(("%s:8741[%d] DW %02X\n", space.machine().describe_context(), num, data));

		mcu->txd = data ^ 0x40; /* parity reverce ? */
		mcu->sts |= 0x02;     /* TXD busy         */
#if 1
		/* interrupt ? */
		if(num == 0)
		{
			if(josvolly_nmi_enable)
			{
				space.machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
				josvolly_nmi_enable = 0;
			}
		}
#endif
	}
	josvolly_8741_do(space.machine(), num);
}

static INT8 josvolly_8741_r(address_space &space,int num,int offset)
{
	JV8741 *mcu = &i8741[num];
	int ret;

	if(offset==1)
	{
		if(mcu->rst)
			mcu->rxd = space.machine().root_device().ioport(mcu->initReadPort)->read(); /* port in */
		ret = mcu->sts;
		LOG(("%s:8741[%d]       SR %02X\n",space.machine().describe_context(),num,ret));
	}
	else
	{
		/* clear status port */
		mcu->sts &= ~0x01; /* RD ready */
		ret = mcu->rxd;
		LOG(("%s:8741[%d]       DR %02X\n",space.machine().describe_context(),num,ret));
		mcu->rst = 0;
	}
	return ret;
}

WRITE8_HANDLER( josvolly_8741_0_w ){ josvolly_8741_w(space,0,offset,data); }
READ8_HANDLER( josvolly_8741_0_r ) { return josvolly_8741_r(space,0,offset); }
WRITE8_HANDLER( josvolly_8741_1_w ) { josvolly_8741_w(space,1,offset,data); }
READ8_HANDLER( josvolly_8741_1_r ) { return josvolly_8741_r(space,1,offset); }

WRITE8_HANDLER( josvolly_nmi_enable_w )
{
	josvolly_nmi_enable = 1;
}
