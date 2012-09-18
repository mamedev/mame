/*********************************************************/
/*    Konami PCM controller                              */
/*********************************************************/

/*
  Changelog, Hiromitsu Shioya 02/05/2002
  fix start address decode timing. (sample loop bug.)

    Changelog, Mish, August 1999:
        Removed interface support for different memory regions per channel.
        Removed interface support for differing channel volume.

        Added bankswitching.
        Added support for multiple chips.

        (Nb:  Should different memory regions per channel be needed
        the bankswitching function can set this up).

NS990821
support for the k007232_VOL() macro.
added external port callback, and functions to set the volume of the channels

*/


#include "emu.h"
#include "k007232.h"


#define  KDAC_A_PCM_MAX    (2)		/* Channels per chip */


struct KDAC_A_PCM
{
	UINT8			vol[KDAC_A_PCM_MAX][2];	/* volume for the left and right channel */
	UINT32			addr[KDAC_A_PCM_MAX];
	UINT32			start[KDAC_A_PCM_MAX];
	UINT32			step[KDAC_A_PCM_MAX];
	UINT32			bank[KDAC_A_PCM_MAX];
	int				play[KDAC_A_PCM_MAX];

	UINT8			wreg[0x10];	/* write data */
	UINT8 *			pcmbuf[2];	/* Channel A & B pointers */

	UINT32  		clock;          /* chip clock */
	UINT32  		pcmlimit;

	sound_stream *	stream;
	const k007232_interface *intf;
	UINT32			fncode[0x200];
};


#define   BASE_SHIFT    (12)


INLINE KDAC_A_PCM *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == K007232);
	return (KDAC_A_PCM *)downcast<k007232_device *>(device)->token();
}


#if 0
static const int kdac_note[] = {
  261.63/8, 277.18/8,
  293.67/8, 311.13/8,
  329.63/8,
  349.23/8, 369.99/8,
  392.00/8, 415.31/8,
  440.00/8, 466.16/8,
  493.88/8,

  523.25/8,
};

static const float kdaca_fn[][2] = {
  /* B */
  { 0x03f, 493.88/8 },		/* ?? */
  { 0x11f, 493.88/4 },		/* ?? */
  { 0x18f, 493.88/2 },		/* ?? */
  { 0x1c7, 493.88   },
  { 0x1e3, 493.88*2 },
  { 0x1f1, 493.88*4 },		/* ?? */
  { 0x1f8, 493.88*8 },		/* ?? */
  /* A+ */
  { 0x020, 466.16/8 },		/* ?? */
  { 0x110, 466.16/4 },		/* ?? */
  { 0x188, 466.16/2 },
  { 0x1c4, 466.16   },
  { 0x1e2, 466.16*2 },
  { 0x1f1, 466.16*4 },		/* ?? */
  { 0x1f8, 466.16*8 },		/* ?? */
  /* A */
  { 0x000, 440.00/8 },		/* ?? */
  { 0x100, 440.00/4 },		/* ?? */
  { 0x180, 440.00/2 },
  { 0x1c0, 440.00   },
  { 0x1e0, 440.00*2 },
  { 0x1f0, 440.00*4 },		/* ?? */
  { 0x1f8, 440.00*8 },		/* ?? */
  { 0x1fc, 440.00*16},		/* ?? */
  { 0x1fe, 440.00*32},		/* ?? */
  { 0x1ff, 440.00*64},		/* ?? */
  /* G+ */
  { 0x0f2, 415.31/4 },
  { 0x179, 415.31/2 },
  { 0x1bc, 415.31   },
  { 0x1de, 415.31*2 },
  { 0x1ef, 415.31*4 },		/* ?? */
  { 0x1f7, 415.31*8 },		/* ?? */
  /* G */
  { 0x0e2, 392.00/4 },
  { 0x171, 392.00/2 },
  { 0x1b8, 392.00   },
  { 0x1dc, 392.00*2 },
  { 0x1ee, 392.00*4 },		/* ?? */
  { 0x1f7, 392.00*8 },		/* ?? */
  /* F+ */
  { 0x0d0, 369.99/4 },		/* ?? */
  { 0x168, 369.99/2 },
  { 0x1b4, 369.99   },
  { 0x1da, 369.99*2 },
  { 0x1ed, 369.99*4 },		/* ?? */
  { 0x1f6, 369.99*8 },		/* ?? */
  /* F */
  { 0x0bf, 349.23/4 },		/* ?? */
  { 0x15f, 349.23/2 },
  { 0x1af, 349.23   },
  { 0x1d7, 349.23*2 },
  { 0x1eb, 349.23*4 },		/* ?? */
  { 0x1f5, 349.23*8 },		/* ?? */
  /* E */
  { 0x0ac, 329.63/4 },
  { 0x155, 329.63/2 },		/* ?? */
  { 0x1ab, 329.63   },
  { 0x1d5, 329.63*2 },
  { 0x1ea, 329.63*4 },		/* ?? */
  { 0x1f4, 329.63*8 },		/* ?? */
  /* D+ */
  { 0x098, 311.13/4 },		/* ?? */
  { 0x14c, 311.13/2 },
  { 0x1a6, 311.13   },
  { 0x1d3, 311.13*2 },
  { 0x1e9, 311.13*4 },		/* ?? */
  { 0x1f4, 311.13*8 },		/* ?? */
  /* D */
  { 0x080, 293.67/4 },		/* ?? */
  { 0x140, 293.67/2 },		/* ?? */
  { 0x1a0, 293.67   },
  { 0x1d0, 293.67*2 },
  { 0x1e8, 293.67*4 },		/* ?? */
  { 0x1f4, 293.67*8 },		/* ?? */
  { 0x1fa, 293.67*16},		/* ?? */
  { 0x1fd, 293.67*32},		/* ?? */
  /* C+ */
  { 0x06d, 277.18/4 },		/* ?? */
  { 0x135, 277.18/2 },		/* ?? */
  { 0x19b, 277.18   },
  { 0x1cd, 277.18*2 },
  { 0x1e6, 277.18*4 },		/* ?? */
  { 0x1f2, 277.18*8 },		/* ?? */
  /* C */
  { 0x054, 261.63/4 },
  { 0x12a, 261.63/2 },
  { 0x195, 261.63   },
  { 0x1ca, 261.63*2 },
  { 0x1e5, 261.63*4 },
  { 0x1f2, 261.63*8 },		/* ?? */

  { -1, -1 },
};
#endif

/*************************************************************/
static void KDAC_A_make_fncode( KDAC_A_PCM *info ){
  int i;
#if 0
  int i, j, k;
  float fn;
  for( i = 0; i < 0x200; i++ )  fncode[i] = 0;

  i = 0;
  while( (int)kdaca_fn[i][0] != -1 ){
    fncode[(int)kdaca_fn[i][0]] = kdaca_fn[i][1];
    i++;
  }

  i = j = 0;
  while( i < 0x200 ){
    if( fncode[i] != 0 ){
      if( i != j ){
	fn = (fncode[i] - fncode[j]) / (i - j);
	for( k = 1; k < (i-j); k++ )
	  fncode[k+j] = fncode[j] + fn*k;
	j = i;
      }
    }
    i++;
  }
#if 0
  for( i = 0; i < 0x200; i++ )
    logerror("fncode[%04x] = %.2f\n", i, fncode[i] );
#endif

#else
  for( i = 0; i < 0x200; i++ ){
    //fncode[i] = (0x200 * 55) / (0x200 - i);
    info->fncode[i] = (32 << BASE_SHIFT) / (0x200 - i);
//  info->fncode[i] = ((0x200 * 55.2 / 880) / (0x200 - i));
    // = 512 * 55.2 / 220 / (512 - i) = 128 / (512 - i)
    //    logerror("2 : fncode[%04x] = %.2f\n", i, fncode[i] );
  }

#endif
}


/************************************************/
/*    Konami PCM update                         */
/************************************************/

static STREAM_UPDATE( KDAC_A_update )
{
  KDAC_A_PCM *info = (KDAC_A_PCM *)param;
  int i;

  memset(outputs[0],0,samples * sizeof(*outputs[0]));
  memset(outputs[1],0,samples * sizeof(*outputs[1]));

  for( i = 0; i < KDAC_A_PCM_MAX; i++ )
    {
      if (info->play[i])
	{
	  int volA,volB,j,out;
	  unsigned int addr, old_addr;
	  //int cen;

	  /**** PCM setup ****/
	  addr = info->start[i] + ((info->addr[i]>>BASE_SHIFT)&0x000fffff);
	  volA = info->vol[i][0] * 2;
	  volB = info->vol[i][1] * 2;
#if 0
	   cen = (volA + volB) / 2;
	  volA = (volA + cen) < 0x1fe ? (volA + cen) : 0x1fe;
	  volB = (volB + cen) < 0x1fe ? (volB + cen) : 0x1fe;
#endif

	  for( j = 0; j < samples; j++ )
	    {
	      old_addr = addr;
	      addr = info->start[i] + ((info->addr[i]>>BASE_SHIFT)&0x000fffff);
	      while (old_addr <= addr)
		{
		  if( (info->pcmbuf[i][old_addr] & 0x80) || old_addr >= info->pcmlimit )
		    {
		      /* end of sample */

		      if( info->wreg[0x0d]&(1<<i) )
			{
			  /* loop to the beginning */
			  info->start[i] =
			    ((((unsigned int)info->wreg[i*0x06 + 0x04]<<16)&0x00010000) |
			     (((unsigned int)info->wreg[i*0x06 + 0x03]<< 8)&0x0000ff00) |
			     (((unsigned int)info->wreg[i*0x06 + 0x02]    )&0x000000ff) |
			     info->bank[i]);
			  addr = info->start[i];
			  info->addr[i] = 0;
			  old_addr = addr; /* skip loop */
			}
		      else
			{
			  /* stop sample */
			  info->play[i] = 0;
			}
		      break;
		    }

		  old_addr++;
		}

	      if (info->play[i] == 0)
		break;

	      info->addr[i] += info->step[i];

	      out = (info->pcmbuf[i][addr] & 0x7f) - 0x40;

	      outputs[0][j] += out * volA;
	      outputs[1][j] += out * volB;
	    }
	}
    }
}


/************************************************/
/*    Konami PCM start                          */
/************************************************/
static DEVICE_START( k007232 )
{
	static const k007232_interface defintrf = { 0 };
	int i;
	KDAC_A_PCM *info = get_safe_token(device);

	info->intf = (device->static_config() != NULL) ? (const k007232_interface *)device->static_config() : &defintrf;

	/* Set up the chips */

	info->pcmbuf[0] = *device->region();
	info->pcmbuf[1] = *device->region();
	info->pcmlimit  = device->region()->bytes();

	info->clock = device->clock();

	for( i = 0; i < KDAC_A_PCM_MAX; i++ )
	{
		info->start[i] = 0;
		info->step[i] = 0;
		info->play[i] = 0;
		info->bank[i] = 0;
	}
	info->vol[0][0] = 255;	/* channel A output to output A */
	info->vol[0][1] = 0;
	info->vol[1][0] = 0;
	info->vol[1][1] = 255;	/* channel B output to output B */

	for( i = 0; i < 0x10; i++ )  info->wreg[i] = 0;

	info->stream = device->machine().sound().stream_alloc(*device,0,2,device->clock()/128,info,KDAC_A_update);

	KDAC_A_make_fncode(info);
}

/************************************************/
/*    Konami PCM write register                 */
/************************************************/
WRITE8_DEVICE_HANDLER( k007232_w )
{
  KDAC_A_PCM *info = get_safe_token(device);
  int r = offset;
  int v = data;

  info->stream->update();

  info->wreg[r] = v;			/* stock write data */

  if (r == 0x0c){
    /* external port, usually volume control */
    if (info->intf->portwritehandler) (*info->intf->portwritehandler)(device,v);
    return;
  }
  else if( r == 0x0d ){
    /* loopflag. */
    return;
  }
  else{
    int  reg_port;

    reg_port = 0;
    if (r >= 0x06){
      reg_port = 1;
      r -= 0x06;
    }

    switch (r){
    case 0x00:
    case 0x01:
	{
				/**** address step ****/
      int idx = (((((unsigned int)info->wreg[reg_port*0x06 + 0x01])<<8)&0x0100) | (((unsigned int)info->wreg[reg_port*0x06 + 0x00])&0x00ff));
#if 0
      if( !reg_port && r == 1 )
	logerror("%04x\n" ,idx );
#endif

      info->step[reg_port] = info->fncode[idx];
      break;
	}
    case 0x02:
    case 0x03:
    case 0x04:
      break;
    case 0x05:
				/**** start address ****/
      info->start[reg_port] =
	((((unsigned int)info->wreg[reg_port*0x06 + 0x04]<<16)&0x00010000) |
	 (((unsigned int)info->wreg[reg_port*0x06 + 0x03]<< 8)&0x0000ff00) |
	 (((unsigned int)info->wreg[reg_port*0x06 + 0x02]    )&0x000000ff) |
	 info->bank[reg_port]);
      if (info->start[reg_port] < info->pcmlimit ){
	info->play[reg_port] = 1;
	info->addr[reg_port] = 0;
      }
      break;
    }
  }
}

/************************************************/
/*    Konami PCM read register                  */
/************************************************/
READ8_DEVICE_HANDLER( k007232_r )
{
  KDAC_A_PCM *info = get_safe_token(device);
  int r = offset;
  int  ch = 0;

  if( r == 0x0005 || r == 0x000b ){
    ch = r/0x0006;
    r  = ch * 0x0006;

    info->start[ch] =
      ((((unsigned int)info->wreg[r + 0x04]<<16)&0x00010000) |
       (((unsigned int)info->wreg[r + 0x03]<< 8)&0x0000ff00) |
       (((unsigned int)info->wreg[r + 0x02]    )&0x000000ff) |
       info->bank[ch]);

    if (info->start[ch] <  info->pcmlimit ){
      info->play[ch] = 1;
      info->addr[ch] = 0;
    }
  }
  return 0;
}

/*****************************************************************************/

void k007232_set_volume(device_t *device,int channel,int volumeA,int volumeB)
{
  KDAC_A_PCM *info = get_safe_token(device);
  info->vol[channel][0] = volumeA;
  info->vol[channel][1] = volumeB;
}

void k007232_set_bank( device_t *device, int chABank, int chBBank )
{
  KDAC_A_PCM *info = get_safe_token(device);
  info->bank[0] = chABank<<17;
  info->bank[1] = chBBank<<17;
}

/*****************************************************************************/

const device_type K007232 = &device_creator<k007232_device>;

k007232_device::k007232_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K007232, "K007232", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(KDAC_A_PCM);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k007232_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k007232_device::device_start()
{
	DEVICE_START_NAME( k007232 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k007232_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


