// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
/*

Taito 8741 emulation

1.The pair chip for the PIO and serial communication between MAIN CPU and the sub CPU
2.The PIO for DIP SW and the controller reading.

*/

#include "emu.h"
#include "tait8741.h"

#define VERBOSE 0
#include "logmacro.h"

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

DEFINE_DEVICE_TYPE(TAITO8741_4PACK, taito8741_4pack_device, "taito8741_4pack", "I8741 MCU Simulation (Taito 4Pack)")

taito8741_4pack_device::taito8741_4pack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TAITO8741_4PACK, tag, owner, clock),
	m_port_handler_0_r(*this),
	m_port_handler_1_r(*this),
	m_port_handler_2_r(*this),
	m_port_handler_3_r(*this)
{
}

/* for host data , write */
void taito8741_4pack_device::hostdata_w(I8741 *st,int data)
{
	st->toData = data;
	st->status |= 0x01;
}

/* from host data , read */
int taito8741_4pack_device::hostdata_r(I8741 *st)
{
	if( !(st->status & 0x02) ) return -1;
	st->status &= 0xfd;
	return st->fromData;
}

/* from host command , read */
int taito8741_4pack_device::hostcmd_r(I8741 *st)
{
	if(!(st->status & 0x04)) return -1;
	st->status &= 0xfb;
	return st->fromCmd;
}


/* TAITO8741 I8741 emulation */

void taito8741_4pack_device::serial_rx(I8741 *st,uint8_t *data)
{
	memcpy(st->rxd,data,8);
}

/* timer callback of serial tx finish */
TIMER_CALLBACK_MEMBER( taito8741_4pack_device::serial_tx )
{
	int num = param;
	I8741 *st = &m_taito8741[num];
	I8741 *sst;

	if( st->mode==TAITO8741_MASTER)
		st->serial_out = 1;

	st->txpoint = 1;
	if(st->connect >= 0 )
	{
		sst = &m_taito8741[st->connect];
		/* transfer data */
		serial_rx(sst,st->txd);
		LOG("8741-%d Serial data TX to %d\n",num,st->connect);
		if( sst->mode==TAITO8741_SLAVE)
			sst->serial_out = 1;
	}
}

void taito8741_4pack_device::device_reset()
{
	for (int i=0;i<4;i++)
	{
		I8741 *st = &m_taito8741[i];
		st->number = i;
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
}

/* 8741 update */
void taito8741_4pack_device::update(int num)
{
	I8741 *st,*sst;
	int next = num;
	int data;

	do{
		num = next;
		st = &m_taito8741[num];
		if( st->connect != -1 )
				sst = &m_taito8741[st->connect];
		else sst = nullptr;
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
				hostdata_w(st,0);
				st->phase = CMD_IDLE;
				next = num; /* continue this chip */
			}
			break;
		case CMD_IDLE:
			/* ----- data in port check ----- */
			data = hostdata_r(st);
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
						hostdata_w(st,port_read(st->number,st->parallelselect));
					}
				}
			}
			/* ----- new command fetch ----- */
			data = hostcmd_r(st);
			switch( data )
			{
			case -1: /* no command data */
				break;
			case 0x00: /* read from parallel port */
				hostdata_w(st,port_read(st->number,0));
				break;
			case 0x01: /* read receive buffer 0 */
			case 0x02: /* read receive buffer 1 */
			case 0x03: /* read receive buffer 2 */
			case 0x04: /* read receive buffer 3 */
			case 0x05: /* read receive buffer 4 */
			case 0x06: /* read receive buffer 5 */
			case 0x07: /* read receive buffer 6 */
//if (data == 2 && num==0 && st->rxd[data-1]&0x80) logerror("Coin Get\n");
				hostdata_w(st,st->rxd[data-1]);
				break;
			case 0x08:  /* latch received serial data */
				st->txd[0] = port_read(st->number,0);
				if( sst )
				{
					machine().scheduler().synchronize(timer_expired_delegate(FUNC(taito8741_4pack_device::serial_tx),this), num);
					st->serial_out = 0;
					st->status |= 0x04;
					st->phase = CMD_08;
				}
				break;
			case 0x0a:  /* 8741-0 : set serial comminucation mode 'MASTER' */
				//st->mode = TAITO8741_MASTER;
				break;
			case 0x0b:  /* 8741-1 : set serial comminucation mode 'SLAVE'  */
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
			case 0x4a:  /* ?? syncronus with other cpu and return 00H */
				if( sst )
				{
					if(sst->pending4a)
					{
						sst->pending4a = 0; /* syncronus */
						hostdata_w(st,0); /* return for host */
						next = st->connect;
					}
					else st->phase = CMD_4a;
				}
				break;
			case 0x80:  /* 8741-3 : return check code */
				hostdata_w(st,0x66);
				break;
			case 0x81:  /* 8741-2 : return check code */
				hostdata_w(st,0x48);
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

void taito8741_4pack_device::device_start()
{
	m_port_handler_0_r.resolve_safe(0);
	m_port_handler_1_r.resolve_safe(0);
	m_port_handler_2_r.resolve_safe(0);
	m_port_handler_3_r.resolve_safe(0);

	for (int i = 0; i < 4; i++)
	{
		save_item(NAME(m_taito8741[i].toData), i);
		save_item(NAME(m_taito8741[i].fromData), i);
		save_item(NAME(m_taito8741[i].fromCmd), i);
		save_item(NAME(m_taito8741[i].status), i);
		save_item(NAME(m_taito8741[i].phase), i);
		save_item(NAME(m_taito8741[i].txd), i);
		save_item(NAME(m_taito8741[i].rxd), i);
		save_item(NAME(m_taito8741[i].parallelselect), i);
		save_item(NAME(m_taito8741[i].txpoint), i);
		//save_item(NAME(m_taito8741[i].pending4a), i); //currently initialized to 0, never changes
		save_item(NAME(m_taito8741[i].serial_out), i);
		//save_item(NAME(m_taito8741[i].coins), i); // currently initialized but otherwise unused
	};
}

/* read status port */
int taito8741_4pack_device::status_r(int num)
{
	I8741 *st = &m_taito8741[num];
	update(num);
	LOG("%s:8741-%d ST Read %02x\n",machine().describe_context(),num,st->status);
	return st->status;
}

/* read data port */
int taito8741_4pack_device::data_r(int num)
{
	I8741 *st = &m_taito8741[num];
	int ret = st->toData;
	st->status &= 0xfe;
	LOG("%s:8741-%d DATA Read %02x\n",machine().describe_context(),num,ret);

	/* update chip */
	update(num);

	switch( st->mode )
	{
	case TAITO8741_PORT: /* parallel data */
		hostdata_w(st,port_read(st->number,st->parallelselect));
		break;
	}
	return ret;
}

/* Write data port */
void taito8741_4pack_device::data_w(int num, int data)
{
	I8741 *st = &m_taito8741[num];
	LOG("%s:8741-%d DATA Write %02x\n",machine().describe_context(),num,data);
	st->fromData = data;
	st->status |= 0x02;
	/* update chip */
	update(num);
}

/* Write command port */
void taito8741_4pack_device::command_w(int num, int data)
{
	I8741 *st = &m_taito8741[num];
	LOG("%s:8741-%d CMD Write %02x\n",machine().describe_context(),num,data);
	st->fromCmd = data;
	st->status |= 0x04;
	/* update chip */
	update(num);
}

uint8_t taito8741_4pack_device::port_read(int num, int offset)
{
	switch(num)
	{
		case 0 : return m_port_handler_0_r(offset);
		case 1 : return m_port_handler_1_r(offset);
		case 2 : return m_port_handler_2_r(offset);
		case 3 : return m_port_handler_3_r(offset);
		default : return 0;
	}
}
