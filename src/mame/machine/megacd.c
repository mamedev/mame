
#include "includes/megadriv.h"
#include "megacd.lh"
#include "sound/cdda.h"
#include "sound/rf5c68.h"

// the main MD emulation needs to know the state of these because it appears in the MD regs / affect DMA operations
int sega_cd_connected = 0x00;
int segacd_wordram_mapped = 0;





const device_type SEGA_SEGACD_US = &device_creator<sega_segacd_us_device>;
const device_type SEGA_SEGACD_JAPAN = &device_creator<sega_segacd_japan_device>;
const device_type SEGA_SEGACD_EUROPE = &device_creator<sega_segacd_europe_device>;

sega_segacd_device::sega_segacd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, device_type type)
	: device_t(mconfig, type, "sega_segacd_device", tag, owner, clock)
{

}

sega_segacd_us_device::sega_segacd_us_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_segacd_device(mconfig, tag, owner, clock, SEGA_SEGACD_US)
{

}

sega_segacd_japan_device::sega_segacd_japan_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_segacd_device(mconfig, tag, owner, clock, SEGA_SEGACD_JAPAN)
{

}

sega_segacd_europe_device::sega_segacd_europe_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_segacd_device(mconfig, tag, owner, clock, SEGA_SEGACD_EUROPE)
{

}


static MACHINE_CONFIG_FRAGMENT( segacd_fragment )

	MCFG_CPU_ADD("segacd_68k", M68000, SEGACD_CLOCK ) /* 12.5 MHz */
	MCFG_CPU_PROGRAM_MAP(segacd_map)

	MCFG_DEVICE_ADD("cdc", LC89510, 0) // cd controller

	MCFG_TIMER_ADD("sw_timer", NULL) //stopwatch timer

	MCFG_DEFAULT_LAYOUT( layout_megacd )

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, ":lspeaker", 0.50 ) // TODO: accurate volume balance
	MCFG_SOUND_ROUTE( 1, ":rspeaker", 0.50 )

	MCFG_SOUND_ADD("rfsnd", RF5C68, SEGACD_CLOCK) // RF5C164!
	MCFG_SOUND_ROUTE( 0, ":lspeaker", 0.50 )
	MCFG_SOUND_ROUTE( 1, ":rspeaker", 0.50 )

	MCFG_TIMER_ADD("scd_dma_timer", scd_dma_timer_callback)

	MCFG_NVRAM_HANDLER_CLEAR()
	MCFG_NVRAM_ADD_0FILL("backupram")

	MCFG_QUANTUM_PERFECT_CPU("segacd_68k") // perfect sync to the fastest cpu
MACHINE_CONFIG_END



machine_config_constructor sega_segacd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( segacd_fragment );
}


/* Sega CD stuff */
cpu_device *_segacd_68k_cpu;

UINT16 segacd_irq_mask;
static UINT16 *segacd_backupram;
static timer_device *stopwatch_timer;
static UINT8 segacd_font_color;
static UINT16* segacd_font_bits;
static UINT16 scd_rammode;
static UINT32 scd_mode_dmna_ret_flags ;

static emu_timer *segacd_gfx_conversion_timer;
//static emu_timer *segacd_dmna_ret_timer;
static emu_timer *segacd_irq3_timer;
static emu_timer *segacd_hock_timer;
static UINT8 hock_cmd;
static TIMER_CALLBACK( segacd_irq3_timer_callback );
timer_device* scd_dma_timer;

static void segacd_mark_tiles_dirty(running_machine& machine, int offset);


/*************************************************************************************************
 Sega CD related
*************************************************************************************************/

// The perfect syncs should make this unnecessary: forcing syncs on reads is a flawed design pattern anyway,
// because the sync will only happen AFTER the read, by which time it's too late.
#define SEGACD_FORCE_SYNCS 0

static UINT8 segacd_ram_writeprotect_bits;
//int segacd_ram_mode;
//static int segacd_ram_mode_old;

//static int segacd_maincpu_has_ram_access = 0;
static int segacd_4meg_prgbank = 0; // which bank the MainCPU can see of the SubCPU PrgRAM
static int segacd_memory_priority_mode = 0;
static int segacd_stampsize;
//int segacd_dmna = 0;
//int segacd_ret = 0;

#define READ_MAIN (0x0200)
#define READ_SUB  (0x0300)
#define DMA_PCM  (0x0400)
#define DMA_PRG  (0x0500)
#define DMA_WRAM (0x0700)

#define REG_W_SBOUT  (0x0)
#define REG_W_IFCTRL (0x1)
#define REG_W_DBCL   (0x2)
#define REG_W_DBCH   (0x3)
#define REG_W_DACL   (0x4)
#define REG_W_DACH   (0x5)
#define REG_W_DTTRG  (0x6)
#define REG_W_DTACK  (0x7)
#define REG_W_WAL    (0x8)
#define REG_W_WAH    (0x9)
#define REG_W_CTRL0  (0xA)
#define REG_W_CTRL1  (0xB)
#define REG_W_PTL    (0xC)
#define REG_W_PTH    (0xD)
#define REG_W_CTRL2  (0xE)
#define REG_W_RESET  (0xF)

#define REG_R_COMIN  (0x0)
#define REG_R_IFSTAT (0x1)
#define REG_R_DBCL   (0x2)
#define REG_R_DBCH   (0x3)
#define REG_R_HEAD0  (0x4)
#define REG_R_HEAD1  (0x5)
#define REG_R_HEAD2  (0x6)
#define REG_R_HEAD3  (0x7)
#define REG_R_PTL    (0x8)
#define REG_R_PTH    (0x9)
#define REG_R_WAL    (0xa)
#define REG_R_WAH    (0xb)
#define REG_R_STAT0  (0xc)
#define REG_R_STAT1  (0xd)
#define REG_R_STAT2  (0xe)
#define REG_R_STAT3  (0xf)

#define CMD_STATUS   (0x0)
#define CMD_STOPALL  (0x1)
#define CMD_GETTOC   (0x2)
#define CMD_READ     (0x3)
#define CMD_SEEK     (0x4)
//                   (0x5)
#define CMD_STOP     (0x6)
#define CMD_RESUME   (0x7)
#define CMD_FF       (0x8)
#define CMD_RW       (0x9)
#define CMD_INIT     (0xa)
//                   (0xb)
#define CMD_CLOSE    (0xc)
#define CMD_OPEN     (0xd)
//                   (0xe)
//                   (0xf)


#define TOCCMD_CURPOS    (0x0)
#define TOCCMD_TRKPOS	 (0x1)
#define TOCCMD_CURTRK    (0x2)
#define TOCCMD_LENGTH    (0x3)
#define TOCCMD_FIRSTLAST (0x4)
#define TOCCMD_TRACKADDR (0x5)

struct segacd_t
{
	cdrom_file	*cd;
	const cdrom_toc   *toc;
	UINT32 current_frame;
};

segacd_t segacd;

#define SECTOR_SIZE (2352)

#define SET_CDD_DATA_MODE \
	CDD_CONTROL |= 0x0100; \

#define SET_CDD_AUDIO_MODE \
	CDD_CONTROL &= ~0x0100; \

#define STOP_CDC_READ \
	SCD_STATUS_CDC &= ~0x01; \

#define SET_CDC_READ \
	SCD_STATUS_CDC |= 0x01; \

#define SET_CDC_DMA \
	SCD_STATUS_CDC |= 0x08; \

#define STOP_CDC_DMA \
	SCD_STATUS_CDC &= ~0x08; \

#define SCD_READ_ENABLED \
	(SCD_STATUS_CDC & 1)

#define SCD_DMA_ENABLED \
	(SCD_STATUS_CDC & 0x08)

#define CLEAR_CDD_RESULT \
	CDD_MIN = CDD_SEC = CDD_FRAME = CDD_EXT = 0; \

#define CHECK_SCD_LV5_INTERRUPT \
	if (segacd_irq_mask & 0x20) \
	{ \
		machine.device(":segacd:segacd_68k")->execute().set_input_line(5, HOLD_LINE); \
	} \

#define CHECK_SCD_LV4_INTERRUPT \
	if (segacd_irq_mask & 0x10) \
	{ \
		machine.device(":segacd:segacd_68k")->execute().set_input_line(4, HOLD_LINE); \
	} \

#define CHECK_SCD_LV3_INTERRUPT \
	if (segacd_irq_mask & 0x08) \
	{ \
		machine.device(":segacd:segacd_68k")->execute().set_input_line(3, HOLD_LINE); \
	} \

#define CHECK_SCD_LV2_INTERRUPT \
	if (segacd_irq_mask & 0x04) \
	{ \
		machine.device(":segacd:segacd_68k")->execute().set_input_line(2, HOLD_LINE); \
	} \

#define CHECK_SCD_LV1_INTERRUPT \
	if (segacd_irq_mask & 0x02) \
	{ \
		machine.device(":segacd:segacd_68k")->execute().set_input_line(1, HOLD_LINE); \
	} \

#define CURRENT_TRACK_IS_DATA \
	(segacd.toc->tracks[SCD_CURTRK - 1].trktype != CD_TRACK_AUDIO) \



INLINE int to_bcd(int val, bool byte)
{
	if (val > 99) val = 99;

	if (byte) return (((val) / 10) << 4) + ((val) % 10);
	else return (((val) / 10) << 8) + ((val) % 10);
}



UINT16* segacd_4meg_prgram;  // pointer to SubCPU PrgRAM
UINT16* segacd_dataram;

#define RAM_MODE_2MEG (0)
#define RAM_MODE_1MEG (2)



INLINE void write_pixel(running_machine& machine, UINT8 pix, int pixeloffset )
{

	int shift = 12-(4*(pixeloffset&0x3));
	UINT16 datamask = (0x000f) << shift;

	int offset = pixeloffset>>3;
	if (pixeloffset&4) offset++;

	offset &=0x1ffff;

	switch (segacd_memory_priority_mode)
	{
		case 0x00: // normal write, just write the data
			segacd_dataram[offset] = segacd_dataram[offset] &~ datamask;
			segacd_dataram[offset] |= pix << shift;
			break;

		case 0x01: // underwrite, only write if the existing data is 0
			if ((segacd_dataram[offset]&datamask) == 0x0000)
			{
				segacd_dataram[offset] = segacd_dataram[offset] &~ datamask;
				segacd_dataram[offset] |= pix << shift;
			}
			break;

		case 0x02: // overwrite, only write non-zero data
			if (pix)
			{
				segacd_dataram[offset] = segacd_dataram[offset] &~ datamask;
				segacd_dataram[offset] |= pix << shift;
			}
			break;

		default:
		case 0x03:
			pix = machine.rand() & 0x000f;
			segacd_dataram[offset] = segacd_dataram[offset] &~ datamask;
			segacd_dataram[offset] |= pix << shift;
			break;

	}
}

// 1meg / 2meg swap is interleaved swap, not half/half of the ram?
// Wily Beamish and Citizen X appear to rely on this
// however, it breaks the megacdj bios (megacd2j still works!)
//  (maybe that's a timing issue instead?)
UINT16 segacd_1meg_mode_word_read(int offset, UINT16 mem_mask)
{
	offset *= 2;

	if ((offset&0x20000))
		offset +=1;

	offset &=0x1ffff;

	return segacd_dataram[offset];
}


void segacd_1meg_mode_word_write(running_machine& machine, int offset, UINT16 data, UINT16 mem_mask, int use_pm)
{
	offset *= 2;

	if ((offset&0x20000))
		offset +=1;

	offset &=0x1ffff;

	if (use_pm)
	{
		// priority mode can apply when writing with the double up buffer mode
		// Jaguar XJ220 relies on this
		switch (segacd_memory_priority_mode)
		{
			case 0x00: // normal write, just write the data
				COMBINE_DATA(&segacd_dataram[offset]);
				break;

			case 0x01: // underwrite, only write if the existing data is 0
				if (ACCESSING_BITS_8_15)
				{
					if ((segacd_dataram[offset]&0xf000) == 0x0000) segacd_dataram[offset] |= (data)&0xf000;
					if ((segacd_dataram[offset]&0x0f00) == 0x0000) segacd_dataram[offset] |= (data)&0x0f00;
				}
				if (ACCESSING_BITS_0_7)
				{
					if ((segacd_dataram[offset]&0x00f0) == 0x0000) segacd_dataram[offset] |= (data)&0x00f0;
					if ((segacd_dataram[offset]&0x000f) == 0x0000) segacd_dataram[offset] |= (data)&0x000f;
				}
				break;

			case 0x02: // overwrite, only write non-zero data
				if (ACCESSING_BITS_8_15)
				{
					if ((data)&0xf000) segacd_dataram[offset] = (segacd_dataram[offset] & 0x0fff) | ((data)&0xf000);
					if ((data)&0x0f00) segacd_dataram[offset] = (segacd_dataram[offset] & 0xf0ff) | ((data)&0x0f00);
				}
				if (ACCESSING_BITS_0_7)
				{
					if ((data)&0x00f0) segacd_dataram[offset] = (segacd_dataram[offset] & 0xff0f) | ((data)&0x00f0);
					if ((data)&0x000f) segacd_dataram[offset] = (segacd_dataram[offset] & 0xfff0) | ((data)&0x000f);
				}
				break;

			default:
			case 0x03: // invalid?
				COMBINE_DATA(&segacd_dataram[offset]);
				break;

		}
	}
	else
	{
		COMBINE_DATA(&segacd_dataram[offset]);
	}
}


static UINT16* segacd_dataram2;

UINT8    SCD_BUFFER[2560];
UINT32   SCD_STATUS;
UINT32   SCD_STATUS_CDC;
INT32    SCD_CURLBA;
UINT8    SCD_CURTRK;

UINT16 CDC_DECODE;
INT16 CDC_DMACNT; // can go negative
UINT16 CDC_DMA_ADDRC;
UINT16 CDC_PT;
UINT16 CDC_WA;
UINT16 CDC_REG0;
UINT16 CDC_REG1;
UINT16 CDC_DMA_ADDR;
UINT16 CDC_IFSTAT;
UINT8 CDC_HEADB0;
UINT8 CDC_HEADB1;
UINT8 CDC_HEADB2;
UINT8 CDC_HEADB3;
UINT8 CDC_STATB0;
UINT8 CDC_STATB1;
UINT8 CDC_STATB2;
UINT8 CDC_STATB3;
UINT16 CDC_SBOUT;
UINT16 CDC_IFCTRL;
UINT8 CDC_CTRLB0;
UINT8 CDC_CTRLB1;
UINT8 CDC_CTRLB2;
UINT8 CDC_BUFFER[(32 * 1024 * 2) + SECTOR_SIZE];

UINT32 CDD_STATUS;
UINT32 CDD_MIN;
UINT32 CDD_SEC;

UINT8 CDD_RX[10];
UINT8 CDD_TX[10];
UINT32 CDD_FRAME;
UINT32 CDD_EXT;
UINT16 CDD_CONTROL;
INT16  CDD_DONE;

static void set_data_audio_mode(void)
{
	if (CURRENT_TRACK_IS_DATA)
	{
		SET_CDD_DATA_MODE
	}
	else
	{
		SET_CDD_AUDIO_MODE
		//fatalerror("CDDA unsupported\n");
	}
}


#define CDD_PLAYINGCDDA	0x0100
#define CDD_READY		0x0400
#define CDD_STOPPED		0x0900

void CDD_DoChecksum(void)
{
	int checksum =
		CDD_RX[0] +
		CDD_RX[1] +
		CDD_RX[2] +
		CDD_RX[3] +
		CDD_RX[4] +
		CDD_RX[5] +
		CDD_RX[6] +
		CDD_RX[7] +
		CDD_RX[9];

	checksum &= 0xf;
	checksum ^= 0xf;

	CDD_RX[8] = checksum;
}

void CDD_Export(void)
{
	CDD_RX[0] = (CDD_STATUS  & 0x00ff)>>0;
	CDD_RX[1] = (CDD_STATUS  & 0xff00)>>8;
	CDD_RX[2] = (CDD_MIN  & 0x00ff)>>0;
	CDD_RX[3] = (CDD_MIN  & 0xff00)>>8;
	CDD_RX[4] = (CDD_SEC & 0x00ff)>>0;
	CDD_RX[5] = (CDD_SEC & 0xff00)>>8;
	CDD_RX[6] = (CDD_FRAME   & 0x00ff)>>0;
	CDD_RX[7] = (CDD_FRAME   & 0xff00)>>8;
	/* 8 = checksum */
	CDD_RX[9] = (CDD_EXT     & 0x00ff)>>0;

	CDD_DoChecksum();

	CDD_CONTROL &= ~4; // Clear HOCK bit

}



void CDC_UpdateHEAD(void)
{
	if (CDC_CTRLB1 & 0x01)
	{
		CDC_HEADB0 = CDC_HEADB1 = CDC_HEADB2 = CDC_HEADB3 = 0x00;
	}
	else
	{
		UINT32 msf = lba_to_msf_alt(SCD_CURLBA+150);
		CDC_HEADB0 = to_bcd (((msf & 0x00ff0000)>>16), true);
		CDC_HEADB1 = to_bcd (((msf & 0x0000ff00)>>8), true);
		CDC_HEADB2 = to_bcd (((msf & 0x000000ff)>>0), true);
		CDC_HEADB3 = 0x01;
	}
}


void scd_ctrl_checks(running_machine& machine)
{
	CDC_STATB0 = 0x80;

	(CDC_CTRLB0 & 0x10) ? (CDC_STATB2 = CDC_CTRLB1 & 0x08) : (CDC_STATB2 = CDC_CTRLB1 & 0x0C);
	(CDC_CTRLB0 & 0x02) ? (CDC_STATB3 = 0x20) : (CDC_STATB3 = 0x00);

	if (CDC_IFCTRL & 0x20)
	{
		CHECK_SCD_LV5_INTERRUPT
		CDC_IFSTAT &= ~0x20;
		CDC_DECODE = 0;
	}
}

void scd_advance_current_readpos(void)
{
	SCD_CURLBA++;

	CDC_WA += SECTOR_SIZE;
	CDC_PT += SECTOR_SIZE;

	CDC_WA &= 0x7fff;
	CDC_PT &= 0x7fff;
}

int Read_LBA_To_Buffer(running_machine& machine)
{
	bool data_track = false;
	if (CDD_CONTROL & 0x0100) data_track = true;

	if (data_track)
		cdrom_read_data(segacd.cd, SCD_CURLBA, SCD_BUFFER, CD_TRACK_MODE1);

	CDC_UpdateHEAD();

	if (!data_track)
	{
		scd_advance_current_readpos();
	}

	if (CDC_CTRLB0 & 0x80)
	{
		if (CDC_CTRLB0 & 0x04)
		{
			if (data_track)
			{
				scd_advance_current_readpos();

				memcpy(&CDC_BUFFER[CDC_PT + 4], SCD_BUFFER, 2048);
				CDC_BUFFER[CDC_PT+0] = CDC_HEADB0;
				CDC_BUFFER[CDC_PT+1] = CDC_HEADB1;
				CDC_BUFFER[CDC_PT+2] = CDC_HEADB2;
				CDC_BUFFER[CDC_PT+3] = CDC_HEADB3;
			}
			else
			{
				memcpy(&CDC_BUFFER[CDC_PT], SCD_BUFFER, SECTOR_SIZE);
			}
		}

		scd_ctrl_checks(machine);
	}


	return 0;
}

static void CheckCommand(running_machine& machine)
{
	if (CDD_DONE)
	{
		CDD_DONE = 0;
		CDD_Export();
		CHECK_SCD_LV4_INTERRUPT
	}

	if (SCD_READ_ENABLED)
	{
		set_data_audio_mode();
		Read_LBA_To_Buffer(machine);
	}
}


void CDD_GetStatus(void)
{
	UINT16 s = (CDD_STATUS & 0x0f00);

	if ((s == 0x0200) || (s == 0x0700) || (s == 0x0e00))
		CDD_STATUS = (SCD_STATUS & 0xff00) | (CDD_STATUS & 0x00ff);
}


void CDD_Stop(running_machine &machine)
{
	CLEAR_CDD_RESULT
	STOP_CDC_READ
	SCD_STATUS = CDD_STOPPED;
	CDD_STATUS = 0x0000;
	SET_CDD_DATA_MODE
	cdda_stop_audio( machine.device( ":segacd:cdda" ) ); //stop any pending CD-DA
}


void CDD_GetPos(void)
{
	CLEAR_CDD_RESULT
	UINT32 msf;
	CDD_STATUS &= 0xFF;
	if(segacd.cd == NULL) // no cd is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	msf = lba_to_msf_alt(SCD_CURLBA+150);
	CDD_MIN = to_bcd(((msf & 0x00ff0000)>>16),false);
	CDD_SEC = to_bcd(((msf & 0x0000ff00)>>8),false);
	CDD_FRAME = to_bcd(((msf & 0x000000ff)>>0),false);
}

void CDD_GetTrackPos(void)
{
	CLEAR_CDD_RESULT
	int elapsedlba;
	UINT32 msf;
	CDD_STATUS &= 0xFF;
	//  UINT32 end_msf = ;
	if(segacd.cd == NULL) // no cd is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	elapsedlba = SCD_CURLBA - segacd.toc->tracks[ cdrom_get_track(segacd.cd, SCD_CURLBA) ].physframeofs;
	msf = lba_to_msf_alt (elapsedlba);
	//popmessage("%08x %08x",SCD_CURLBA,segacd.toc->tracks[ cdrom_get_track(segacd.cd, SCD_CURLBA) + 1 ].physframeofs);
	CDD_MIN = to_bcd(((msf & 0x00ff0000)>>16),false);
	CDD_SEC = to_bcd(((msf & 0x0000ff00)>>8),false);
	CDD_FRAME = to_bcd(((msf & 0x000000ff)>>0),false);
}

void CDD_GetTrack(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS &= 0xFF;
	if(segacd.cd == NULL) // no cd is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	SCD_CURTRK = cdrom_get_track(segacd.cd, SCD_CURLBA)+1;
	CDD_MIN = to_bcd(SCD_CURTRK, false);
}

void CDD_Length(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS &= 0xFF;
	if(segacd.cd == NULL) // no cd is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;

	UINT32 startlba = (segacd.toc->tracks[cdrom_get_last_track(segacd.cd)].physframeofs);
	UINT32 startmsf = lba_to_msf_alt( startlba );

	CDD_MIN = to_bcd((startmsf&0x00ff0000)>>16,false);
	CDD_SEC = to_bcd((startmsf&0x0000ff00)>>8,false);
	CDD_FRAME = to_bcd((startmsf&0x000000ff)>>0,false);
}


void CDD_FirstLast(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS &= 0xFF;
	if(segacd.cd == NULL) // no cd is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	CDD_MIN = 1; // first
	CDD_SEC = to_bcd(cdrom_get_last_track(segacd.cd),false); // last
}

void CDD_GetTrackAdr(void)
{
	CLEAR_CDD_RESULT

	int track = (CDD_TX[4] & 0xF) + (CDD_TX[5] & 0xF) * 10;
	int last_track = cdrom_get_last_track(segacd.cd);

	CDD_STATUS &= 0xFF;
	if(segacd.cd == NULL) // no cd is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;

	if (track > last_track)
		track = last_track;

	if (track < 1)
		track = 1;

	UINT32 startlba = (segacd.toc->tracks[track-1].physframeofs);
	UINT32 startmsf = lba_to_msf_alt( startlba+150 );

	CDD_MIN = to_bcd((startmsf&0x00ff0000)>>16,false);
	CDD_SEC = to_bcd((startmsf&0x0000ff00)>>8,false);
	CDD_FRAME = to_bcd((startmsf&0x000000ff)>>0,false);
	CDD_EXT = track % 10;

	if (segacd.toc->tracks[track - 1].trktype != CD_TRACK_AUDIO)
		CDD_FRAME |= 0x0800;
}

static UINT32 getmsf_from_regs(void)
{
	UINT32 msf = 0;

	msf  = ((CDD_TX[2] & 0xF) + (CDD_TX[3] & 0xF) * 10) << 16;
	msf |= ((CDD_TX[4] & 0xF) + (CDD_TX[5] & 0xF) * 10) << 8;
	msf |= ((CDD_TX[6] & 0xF) + (CDD_TX[7] & 0xF) * 10) << 0;

	return msf;
}

void CDD_Play(running_machine &machine)
{
	CLEAR_CDD_RESULT
	UINT32 msf = getmsf_from_regs();
	SCD_CURLBA = msf_to_lba(msf)-150;
	UINT32 end_msf = segacd.toc->tracks[ cdrom_get_track(segacd.cd, SCD_CURLBA) + 1 ].physframeofs;
	SCD_CURTRK = cdrom_get_track(segacd.cd, SCD_CURLBA)+1;
	CDC_UpdateHEAD();
	SCD_STATUS = CDD_PLAYINGCDDA;
	CDD_STATUS = 0x0102;
	set_data_audio_mode();
	printf("%d Track played\n",SCD_CURTRK);
	CDD_MIN = to_bcd(SCD_CURTRK, false);
	if(!(CURRENT_TRACK_IS_DATA))
		cdda_start_audio( machine.device( ":segacd:cdda" ), SCD_CURLBA, end_msf - SCD_CURLBA );
	SET_CDC_READ
}


void CDD_Seek(void)
{
	CLEAR_CDD_RESULT
	UINT32 msf = getmsf_from_regs();
	SCD_CURLBA = msf_to_lba(msf)-150;
	SCD_CURTRK = cdrom_get_track(segacd.cd, SCD_CURLBA)+1;
	CDC_UpdateHEAD();
	STOP_CDC_READ
	SCD_STATUS = CDD_READY;
	CDD_STATUS = 0x0200;
	set_data_audio_mode();
}


void CDD_Pause(running_machine &machine)
{
	CLEAR_CDD_RESULT
	STOP_CDC_READ
	SCD_STATUS = CDD_READY;
	CDD_STATUS = SCD_STATUS;
	SET_CDD_DATA_MODE

	//segacd.current_frame = cdda_get_audio_lba( machine.device( ":segacd:cdda" ) );
	//if(!(CURRENT_TRACK_IS_DATA))
	cdda_pause_audio( machine.device( ":segacd:cdda" ), 1 );
}

void CDD_Resume(running_machine &machine)
{
	CLEAR_CDD_RESULT
	STOP_CDC_READ
	SCD_CURTRK = cdrom_get_track(segacd.cd, SCD_CURLBA)+1;
	SCD_STATUS = CDD_PLAYINGCDDA;
	CDD_STATUS = 0x0102;
	set_data_audio_mode();
	CDD_MIN = to_bcd (SCD_CURTRK, false);
	SET_CDC_READ
	//if(!(CURRENT_TRACK_IS_DATA))
	cdda_pause_audio( machine.device( ":segacd:cdda" ), 0 );
}


void CDD_FF(running_machine &machine)
{
	fatalerror("Fast Forward unsupported\n");
}


void CDD_RW(running_machine &machine)
{
	fatalerror("Fast Rewind unsupported\n");
}


void CDD_Open(void)
{
	fatalerror("Close Tray unsupported\n");
	/* TODO: re-read CD-ROM buffer here (Mega CD has multi disc games iirc?) */
}


void CDD_Close(void)
{
	fatalerror("Open Tray unsupported\n");
	/* TODO: clear CD-ROM buffer here */
}


void CDD_Init(void)
{
	CLEAR_CDD_RESULT
	STOP_CDC_READ
	SCD_STATUS = CDD_READY;
	CDD_STATUS = SCD_STATUS;
	CDD_SEC = 1;
	CDD_FRAME = 1;
}


void CDD_Default(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS = SCD_STATUS;
}


static void CDD_Reset(void)
{
	CLEAR_CDD_RESULT
	CDD_CONTROL = CDD_STATUS = 0;

	for (int i = 0; i < 10; i++)
		CDD_RX[i] = CDD_TX[i] = 0;

	CDD_DoChecksum();

	SCD_CURTRK = SCD_CURLBA = 0;
	SCD_STATUS = CDD_READY;
}

static void CDC_Reset(void)
{
	memset(CDC_BUFFER, 0x00, ((16 * 1024 * 2) + SECTOR_SIZE));
	CDC_UpdateHEAD();

	CDC_DMA_ADDRC = CDC_DMACNT = CDC_PT = CDC_SBOUT = CDC_IFCTRL = CDC_CTRLB0 = CDC_CTRLB1 =
		CDC_CTRLB2 = CDC_HEADB1 = CDC_HEADB2 = CDC_HEADB3 = CDC_STATB0 = CDC_STATB1 = CDC_STATB2 = CDC_DECODE = 0;

	CDC_IFSTAT = 0xFF;
	CDC_WA = SECTOR_SIZE * 2;
	CDC_HEADB0 = 0x01;
	CDC_STATB3 = 0x80;
}


void lc89510_Reset(void)
{
	CDD_Reset();
	CDC_Reset();

	CDC_REG0 = CDC_REG1 = CDC_DMA_ADDR = SCD_STATUS_CDC = CDD_DONE = 0;
}

void CDC_End_Transfer(running_machine& machine)
{
	STOP_CDC_DMA
	CDC_REG0 |= 0x8000;
	CDC_REG0 &= ~0x4000;
	CDC_IFSTAT |= 0x08;

	if (CDC_IFCTRL & 0x40)
	{
		CDC_IFSTAT &= ~0x40;
		CHECK_SCD_LV5_INTERRUPT
	}
}

void CDC_Do_DMA(running_machine& machine, int rate)
{
	address_space& space = machine.device(":segacd:segacd_68k")->memory().space(AS_PROGRAM);

	UINT32 dstoffset, length;
	UINT8 *dest;
	UINT16 destination = CDC_REG0 & 0x0700;

	if (!(SCD_DMA_ENABLED))
		return;

	if ((destination == READ_MAIN) || (destination==READ_SUB))
	{
		CDC_REG0 |= 0x4000;
		return;
	}

	if (CDC_DMACNT <= (rate * 2))
	{
		length = (CDC_DMACNT + 1) >> 1;
		CDC_End_Transfer(machine);
	}
	else
		length = rate;


	int dmacount = length;

	bool PCM_DMA = false;

	if (destination==DMA_PCM)
	{
		dstoffset = (CDC_DMA_ADDR & 0x03FF) << 2;
		PCM_DMA = true;
	}
	else
	{
		dstoffset = (CDC_DMA_ADDR & 0xFFFF) << 3;
	}

	int srcoffset = 0;

	while (dmacount--)
	{
		UINT16 data = (CDC_BUFFER[CDC_DMA_ADDRC+srcoffset]<<8) | CDC_BUFFER[CDC_DMA_ADDRC+srcoffset+1];

		if (destination==DMA_PRG)
		{
			dest = (UINT8 *) segacd_4meg_prgram;
		}
		else if (destination==DMA_WRAM)
		{
			dest = (UINT8*)segacd_dataram;
		}
		else if (destination==DMA_PCM)
		{
			dest = 0;//fatalerror("PCM RAM DMA unimplemented!\n");
		}
		else
		{
			fatalerror("Unknown DMA Destination!!\n");
		}

		if (PCM_DMA)
		{
			space.write_byte(0xff2000+(((dstoffset*2)+1)&0x1fff),data >> 8);
			space.write_byte(0xff2000+(((dstoffset*2)+3)&0x1fff),data & 0xff);
		//  printf("PCM_DMA writing %04x %04x\n",0xff2000+(dstoffset*2), data);
		}
		else
		{
			if (dest)
			{
				if (destination==DMA_WRAM)
				{

					if ((scd_rammode&2)==RAM_MODE_2MEG)
					{
						dstoffset &= 0x3ffff;

						dest[dstoffset+1] = data >>8;
						dest[dstoffset+0] = data&0xff;

						segacd_mark_tiles_dirty(space.machine(), dstoffset/2);
					}
					else
					{
						dstoffset &= 0x1ffff;

						if (!(scd_rammode & 1))
						{
							segacd_1meg_mode_word_write(space.machine(),(dstoffset+0x20000)/2, data, 0xffff, 0);
						}
						else
						{
							segacd_1meg_mode_word_write(space.machine(),(dstoffset+0x00000)/2, data, 0xffff, 0);
						}
					}

				}
				else
				{
					// main ram
					dest[dstoffset+1] = data >>8;
					dest[dstoffset+0] = data&0xff;
				}

			}
		}

		srcoffset += 2;
		dstoffset += 2;
	}

	if (PCM_DMA)
	{
		CDC_DMA_ADDR += length >> 1;
	}
	else
	{
		CDC_DMA_ADDR += length >> 2;
	}

	CDC_DMA_ADDRC += length*2;

	if (SCD_DMA_ENABLED)
		CDC_DMACNT -= length*2;
	else
		CDC_DMACNT = 0;
}




UINT16 CDC_Host_r(running_machine& machine, UINT16 type)
{
	UINT16 destination = CDC_REG0 & 0x0700;

	if (SCD_DMA_ENABLED)
	{
		if (destination == type)
		{
			CDC_DMACNT -= 2;

			if (CDC_DMACNT <= 0)
			{
				if (type==READ_SUB) CDC_DMACNT = 0;

				CDC_End_Transfer(machine);
			}

			UINT16 data = (CDC_BUFFER[CDC_DMA_ADDRC]<<8) | CDC_BUFFER[CDC_DMA_ADDRC+1];
			CDC_DMA_ADDRC += 2;

			return data;
		}
	}

	return 0;
}


UINT8 CDC_Reg_r(void)
{
	int reg = CDC_REG0 & 0xF;
	UINT8 ret = 0;
	UINT16 decoderegs = 0x73F2;

	if ((decoderegs>>reg)&1)
		CDC_DECODE |= (1 << reg);

	//if (reg!=REG_R_STAT3)
		CDC_REG0 = (CDC_REG0 & 0xFFF0) | ((reg+1)&0xf);


	switch (reg)
	{
		case REG_R_COMIN:  ret = 0/*COMIN*/;            break;
		case REG_R_IFSTAT: ret = CDC_IFSTAT;           break;
		case REG_R_DBCL:   ret = CDC_DMACNT & 0xff;       break;
		case REG_R_DBCH:   ret = (CDC_DMACNT >>8) & 0xff; break;
		case REG_R_HEAD0:  ret = CDC_HEADB0;           break;
		case REG_R_HEAD1:  ret = CDC_HEADB1;           break;
		case REG_R_HEAD2:  ret = CDC_HEADB2;           break;
		case REG_R_HEAD3:  ret = CDC_HEADB3;           break;
		case REG_R_PTL:	   ret = CDC_PT & 0xff;        break;
		case REG_R_PTH:	   ret = (CDC_PT >>8) & 0xff;  break;
		case REG_R_WAL:    ret = CDC_WA & 0xff;        break;
		case REG_R_WAH:    ret = (CDC_WA >>8) & 0xff;  break;
		case REG_R_STAT0:  ret = CDC_STATB0;           break;
		case REG_R_STAT1:  ret = CDC_STATB1;           break;
		case REG_R_STAT2:  ret = CDC_STATB2;           break;
		case REG_R_STAT3:  ret = CDC_STATB3;

			CDC_IFSTAT |= 0x20;

			// ??
			if ((CDC_CTRLB0 & 0x80) && (CDC_IFCTRL & 0x20))
			{
				if ((CDC_DECODE & decoderegs) == decoderegs)
				CDC_STATB3 = 0x80;
			}
			break;
	}

	return ret;
}

void CDC_Reg_w(UINT8 data)
{
	int reg = CDC_REG0 & 0xF;

	int changers0[0x10] = { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0};

	if (changers0[reg])
		CDC_REG0 = (CDC_REG0 & 0xFFF0) | (reg+1);

	switch (reg)
	{
	case REG_W_SBOUT:
			CDC_SBOUT = data;
			break;

	case REG_W_IFCTRL:
			CDC_IFCTRL = data;

			if (!(CDC_IFCTRL & 0x02))
			{
				CDC_DMACNT = 0;
				STOP_CDC_DMA;
				CDC_IFSTAT |= 0x08;
			}
			break;

	case REG_W_DBCL: CDC_DMACNT = (CDC_DMACNT &~ 0x00ff) | (data & 0x00ff) << 0; break;
	case REG_W_DBCH: CDC_DMACNT = (CDC_DMACNT &~ 0xff00) | (data & 0x00ff) << 8; break;
	case REG_W_DACL: CDC_DMA_ADDRC = (CDC_DMA_ADDRC &~ 0x00ff) | (data & 0x00ff) << 0; break;
	case REG_W_DACH: CDC_DMA_ADDRC = (CDC_DMA_ADDRC &~ 0xff00) | (data & 0x00ff) << 8; break;

	case REG_W_DTTRG:
			if (CDC_IFCTRL & 0x02)
			{
				CDC_IFSTAT &= ~0x08;
				SET_CDC_DMA;
				CDC_REG0 &= ~0x8000;
			}
			break;

	case REG_W_DTACK: CDC_IFSTAT |= 0x40; break;
	case REG_W_WAL: CDC_WA = (CDC_WA &~ 0x00ff) | (data & 0x00ff) << 0; break;
	case REG_W_WAH:	CDC_WA = (CDC_WA &~ 0xff00) | (data & 0x00ff) << 8;	break;
	case REG_W_CTRL0: CDC_CTRLB0 = data; break;
	case REG_W_CTRL1: CDC_CTRLB1 = data; break;
	case REG_W_PTL: CDC_PT = (CDC_PT &~ 0x00ff) | (data & 0x00ff) << 0; break;
	case REG_W_PTH: CDC_PT = (CDC_PT &~ 0xff00) | (data & 0x00ff) << 8;	break;
	case REG_W_CTRL2: CDC_CTRLB2 = data; break;
	case REG_W_RESET: CDC_Reset();       break;
	}
}

void CDD_Process(running_machine& machine, int reason)
{
	CDD_Export();
	CHECK_SCD_LV4_INTERRUPT
}

void CDD_Handle_TOC_Commands(void)
{
	int subcmd = CDD_TX[2];
	CDD_STATUS = (CDD_STATUS & 0xFF00) | subcmd;

	switch (subcmd)
	{
		case TOCCMD_CURPOS:	   CDD_GetPos();	  break;
		case TOCCMD_TRKPOS:	   CDD_GetTrackPos(); break;
		case TOCCMD_CURTRK:    CDD_GetTrack();   break;
		case TOCCMD_LENGTH:    CDD_Length();      break;
		case TOCCMD_FIRSTLAST: CDD_FirstLast();   break;
		case TOCCMD_TRACKADDR: CDD_GetTrackAdr(); break;
		default:               CDD_GetStatus();   break;
	}
}

static const char *const CDD_import_cmdnames[] =
{
	"Get Status",			// 0
	"Stop ALL",				// 1
	"Handle TOC",			// 2
	"Play",					// 3
	"Seek",					// 4
	"<undefined>",			// 5
	"Pause",				// 6
	"Resume",				// 7
	"FF",					// 8
	"RWD",					// 9
	"INIT",					// A
	"<undefined>",			// B
	"Close Tray",			// C
	"Open Tray",			// D
	"<undefined>",			// E
	"<undefined>"			// F
};

void CDD_Import(running_machine& machine)
{
	if(CDD_TX[1] != 2 && CDD_TX[1] != 0)
		printf("%s\n",CDD_import_cmdnames[CDD_TX[1]]);

	switch (CDD_TX[1])
	{
		case CMD_STATUS:	CDD_GetStatus();	       break;
		case CMD_STOPALL:	CDD_Stop(machine);		   break;
		case CMD_GETTOC:	CDD_Handle_TOC_Commands(); break;
		case CMD_READ:		CDD_Play(machine);         break;
		case CMD_SEEK:		CDD_Seek();	               break;
		case CMD_STOP:		CDD_Pause(machine);	       break;
		case CMD_RESUME:	CDD_Resume(machine);       break;
		case CMD_FF:		CDD_FF(machine);           break;
		case CMD_RW:		CDD_RW(machine);           break;
		case CMD_INIT:		CDD_Init();	               break;
		case CMD_CLOSE:		CDD_Open();                break;
		case CMD_OPEN:		CDD_Close();	           break;
		default:			CDD_Default();	           break;
	}

	CDD_DONE = 1;
}





static UINT16 segacd_hint_register;
static UINT16 segacd_imagebuffer_vdot_size;
static UINT16 segacd_imagebuffer_vcell_size;
static UINT16 segacd_imagebuffer_hdot_size;

static UINT16 a12000_halt_reset_reg = 0x0000;
int segacd_conversion_active = 0;
static UINT16 segacd_stampmap_base_address;
static UINT16 segacd_imagebuffer_start_address;
static UINT16 segacd_imagebuffer_offset;
static tilemap_t    *segacd_stampmap[4];
//static void segacd_mark_stampmaps_dirty(void);



static WRITE16_HANDLER( scd_a12000_halt_reset_w )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();

	UINT16 old_halt = a12000_halt_reset_reg;

	COMBINE_DATA(&a12000_halt_reset_reg);

	if (ACCESSING_BITS_0_7)
	{
		// reset line
		if (a12000_halt_reset_reg&0x0001)
		{
			space.machine().device(":segacd:segacd_68k")->execute().set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			if (!(old_halt&0x0001)) printf("clear reset slave\n");
		}
		else
		{
			space.machine().device(":segacd:segacd_68k")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			if ((old_halt&0x0001)) printf("assert reset slave\n");
		}

		// request BUS
		if (a12000_halt_reset_reg&0x0002)
		{
			space.machine().device(":segacd:segacd_68k")->execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			if (!(old_halt&0x0002)) printf("halt slave\n");
		}
		else
		{
			space.machine().device(":segacd:segacd_68k")->execute().set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			if ((old_halt&0x0002)) printf("resume slave\n");
		}
	}

	if (ACCESSING_BITS_8_15)
	{
		if (a12000_halt_reset_reg&0x0100)
		{
			running_machine& machine = space.machine();
			CHECK_SCD_LV2_INTERRUPT
		}

		if (a12000_halt_reset_reg&0x8000)
		{
			// not writable.. but can read irq mask here?
			//printf("a12000_halt_reset_reg & 0x8000 set\n"); // irq2 mask?
		}


	}
}

static READ16_HANDLER( scd_a12000_halt_reset_r )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();

	return a12000_halt_reset_reg;
}


/********************************************************************************
 MEMORY MODE CONTROL
  - main / sub sides differ!
********************************************************************************/

//
// we might need a delay on the segacd_maincpu_has_ram_access registers, as they actually indicate requests being made
// so probably don't change instantly...
//


static READ16_HANDLER( scd_a12002_memory_mode_r )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();

	int temp = scd_rammode;
	int temp2 = 0;

	temp2 |= (scd_mode_dmna_ret_flags>>(temp*4))&0x7;

	return (segacd_ram_writeprotect_bits << 8) |
		   (segacd_4meg_prgbank << 6) |
			temp2;

}


/* I'm still not 100% clear how this works, the sources I have are a bit vague,
   it might still be incorrect in both modes

  for a simple way to swap blocks of ram between cpus this is stupidly convoluted

 */

// DMNA = Decleration Mainram No Access (bit 0)
// RET = Return access (bit 1)


static WRITE8_HANDLER( scd_a12002_memory_mode_w_8_15 )
{
	if (data & 0xff00)
	{
		printf("write protect bits set %02x\n", data);
	}

	segacd_ram_writeprotect_bits = data;
}


static WRITE8_HANDLER( scd_a12002_memory_mode_w_0_7 )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();


	//printf("scd_a12002_memory_mode_w_0_7 %04x\n",data);

	segacd_4meg_prgbank = (data&0x00c0)>>6;

	if (scd_rammode&0x2)
	{ // ==0x2 (1 meg mode)
		if (!(data&2)) // check DMNA bit
		{
			scd_mode_dmna_ret_flags |= 0x2200;
		}
	}
	else // == 0x0 (2 meg mode)
	{
		if (data&2) // check DMNA bit
		{
			scd_rammode = 1;
		}
	}
}


static WRITE16_HANDLER( scd_a12002_memory_mode_w )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();

	if (ACCESSING_BITS_8_15)
		scd_a12002_memory_mode_w_8_15(space, 0, data>>8, mem_mask>>8);

	if (ACCESSING_BITS_0_7)
		scd_a12002_memory_mode_w_0_7(space, 0, data&0xff, mem_mask&0xff);
}




static READ16_HANDLER( segacd_sub_memory_mode_r )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();

	int temp = scd_rammode;
	int temp2 = 0;

	temp2 |= (scd_mode_dmna_ret_flags>>(temp*4))&0x7;

	return (segacd_ram_writeprotect_bits << 8) |
		   (segacd_memory_priority_mode << 3) |
			temp2;
}


WRITE8_HANDLER( segacd_sub_memory_mode_w_8_15 )
{
	/* setting write protect bits from sub-cpu has no effect? */
}



WRITE8_HANDLER( segacd_sub_memory_mode_w_0_7 )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();


	segacd_memory_priority_mode = (data&0x0018)>>3;

	// If the mode bit is 0 then we're requesting a change to
	// 2Meg mode?

	//printf("segacd_sub_memory_mode_w_0_7 %04x\n",data);

	if (!(data&4)) // check ram mode bit
	{	// == 0x0 - 2 meg mode
		scd_mode_dmna_ret_flags &= 0xddff;

		if (data&1) // check RET
		{
			// If RET is set and the Mode bit in the write is set to 2M then we want to change to 2M mode
			// If we're already in 2M mode it has no effect
			scd_rammode = 0;

		}
		else
		{
			// == 0x4 - 1 meg mode

			int temp = scd_rammode;
			if (temp&2) // check ram mode
			{ // == 0x2 - 1 meg mode
				scd_mode_dmna_ret_flags &= 0xffde;
				scd_rammode = temp &1;
			}
		}
	}
	else
	{	// == 0x4 - 1 meg mode
		data &=1;
		int temp = data;
		int scd_rammode_old = scd_rammode;
		data |=2;

		temp ^= scd_rammode_old;
		scd_rammode = data;

		if (scd_rammode_old & 2)
		{ // == 0x2 - already in 1 meg mode
			if (temp & 1) // ret bit
			{
				scd_mode_dmna_ret_flags &= 0xddff;
			}
		}
		else
		{ // == 0x0 - currently in 2 meg mode
			scd_mode_dmna_ret_flags &= 0xddff;
		}
	}
}

static WRITE16_HANDLER( segacd_sub_memory_mode_w )
{
	//printf("segacd_sub_memory_mode_w %04x %04x\n", data, mem_mask);
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();

	if (ACCESSING_BITS_8_15)
		segacd_sub_memory_mode_w_8_15(space, 0, data>>8, mem_mask>>8);

	if (ACCESSING_BITS_0_7)
		segacd_sub_memory_mode_w_0_7(space, 0, data&0xff, mem_mask&0xff);
}


/********************************************************************************
 END MEMORY MODE CONTROL
********************************************************************************/

/********************************************************************************
 COMMUNICATION FLAGS
  - main / sub sides differ in which bits are write only
********************************************************************************/

static UINT16 segacd_comms_flags = 0x0000;

static READ16_HANDLER( segacd_comms_flags_r )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();
	return segacd_comms_flags;
}

static WRITE16_HANDLER( segacd_comms_flags_subcpu_w )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();

	if (ACCESSING_BITS_8_15) // Dragon's Lair
	{
		segacd_comms_flags = (segacd_comms_flags & 0xff00) | ((data >> 8) & 0x00ff);
	}

	// flashback needs low bits to take priority in word writes
	if (ACCESSING_BITS_0_7)
	{
		segacd_comms_flags = (segacd_comms_flags & 0xff00) | (data & 0x00ff);
	}
}

static WRITE16_HANDLER( segacd_comms_flags_maincpu_w )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();

	if (ACCESSING_BITS_8_15)
	{
		segacd_comms_flags = (segacd_comms_flags & 0x00ff) | (data & 0xff00);
	}

	// flashback needs low bits to take priority in word writes
	if (ACCESSING_BITS_0_7)
	{
		segacd_comms_flags = (segacd_comms_flags & 0x00ff) | ((data << 8) & 0xff00);
	}
}

static READ16_HANDLER( scd_4m_prgbank_ram_r )
{
	UINT16 realoffset = ((segacd_4meg_prgbank * 0x20000)/2) + offset;
	return segacd_4meg_prgram[realoffset];

}

static WRITE16_HANDLER( scd_4m_prgbank_ram_w )
{
	UINT16 realoffset = ((segacd_4meg_prgbank * 0x20000)/2) + offset;

	// todo:
	// check for write protection? (or does that only apply to writes on the SubCPU side?

	COMBINE_DATA(&segacd_4meg_prgram[realoffset]);

}


/* Callback when the genesis enters interrupt code */
static IRQ_CALLBACK(segacd_sub_int_callback)
{
	if (irqline==2)
	{
		// clear this bit
		a12000_halt_reset_reg &= ~0x0100;
		device->machine().device(":segacd:segacd_68k")->execute().set_input_line(2, CLEAR_LINE);
	}

	return (0x60+irqline*4)/4; // vector address
}

UINT16 segacd_comms_part1[0x8];
UINT16 segacd_comms_part2[0x8];

static READ16_HANDLER( segacd_comms_main_part1_r )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();
	return segacd_comms_part1[offset];
}

static WRITE16_HANDLER( segacd_comms_main_part1_w )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();
	COMBINE_DATA(&segacd_comms_part1[offset]);
}

static READ16_HANDLER( segacd_comms_main_part2_r )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();
	return segacd_comms_part2[offset];
}

static WRITE16_HANDLER( segacd_comms_main_part2_w )
{
	printf("Sega CD main CPU attempting to write to read only comms regs\n");
}


static READ16_HANDLER( segacd_comms_sub_part1_r )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();
	return segacd_comms_part1[offset];
}

static WRITE16_HANDLER( segacd_comms_sub_part1_w )
{
	printf("Sega CD sub CPU attempting to write to read only comms regs\n");
}

static READ16_HANDLER( segacd_comms_sub_part2_r )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();
	return segacd_comms_part2[offset];
}

static WRITE16_HANDLER( segacd_comms_sub_part2_w )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();
	COMBINE_DATA(&segacd_comms_part2[offset]);
}

/**************************************************************
 CDC Stuff ********
**************************************************************/



static WRITE16_HANDLER( segacd_cdc_mode_address_w )
{
	COMBINE_DATA(&CDC_REG0);
}

static READ16_HANDLER( segacd_cdc_mode_address_r )
{
	return CDC_REG0;
}

static WRITE16_HANDLER( segacd_cdc_data_w )
{
	COMBINE_DATA(&CDC_REG1);

	if (ACCESSING_BITS_0_7)
		CDC_Reg_w(data);
}

static READ16_HANDLER( segacd_cdc_data_r )
{
	UINT16 retdat = 0x0000;

	if (ACCESSING_BITS_0_7)
		retdat |= CDC_Reg_r();

	return retdat;
}





static READ16_HANDLER( segacd_main_dataram_part1_r )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		// is this correct?
		if (!(scd_rammode&1))
		{
			//printf("segacd_main_dataram_part1_r in mode 0 %08x %04x\n", offset*2, segacd_dataram[offset]);

			return segacd_dataram[offset];

		}
		else
		{
			printf("Illegal: segacd_main_dataram_part1_r in mode 0 without permission\n");
			return 0xffff;
		}

	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{

		if (offset<0x20000/2)
		{
			// wordram accees
			//printf("Unspported: segacd_main_dataram_part1_r (word RAM) in mode 1\n");

			// ret bit set by sub cpu determines which half of WorkRAM we have access to?
			if (scd_rammode&1)
			{
				return segacd_1meg_mode_word_read(offset+0x20000/2, mem_mask);
			}
			else
			{
				return segacd_1meg_mode_word_read(offset+0x00000/2, mem_mask);
			}

		}
		else
		{
			// converts data stored in bitmap format (in dataram) to be read out as tiles (for dma->vram purposes)
			// used by Heart of the Alien

			if(offset<0x30000/2)		/* 0x20000 - 0x2ffff */ // 512x256 bitmap. tiles
				offset = BITSWAP24(offset,23,22,21,20,19,18,17,16,15,8,7,6,5,4,3,2,1,14,13,12,11,10,9,0);
			else if(offset<0x38000/2)	/* 0x30000 - 0x37fff */  // 512x128 bitmap. tiles
				offset = BITSWAP24(offset,23,22,21,20,19,18,17,16,15,14,7,6,5,4,3,2,1,13,12,11,10,9,8,0);
			else if(offset<0x3c000/2)	/* 0x38000 - 0x3bfff */  // 512x64 bitmap. tiles
				offset = BITSWAP24(offset,23,22,21,20,19,18,17,16,15,14,13,6,5,4,3,2,1,12,11,10,9,8,7,0);
			else  /* 0x3c000 - 0x3dfff and 0x3e000 - 0x3ffff */  // 512x32 bitmap (x2) -> tiles
				offset = BITSWAP24(offset,23,22,21,20,19,18,17,16,15,14,13,12,5,4,3,2,1,11,10,9,8,7,6,0);

			offset &=0xffff;
			// HOTA cares about this
			if (!(scd_rammode&1))
			{
				return segacd_1meg_mode_word_read(offset+0x00000/2, mem_mask);
			}
			else
			{
				return segacd_1meg_mode_word_read(offset+0x20000/2, mem_mask);
			}
		}
	}

	return 0x0000;
}

static WRITE16_HANDLER( segacd_main_dataram_part1_w )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		// is this correct?
		if (!(scd_rammode&1))
		{
			COMBINE_DATA(&segacd_dataram[offset]);
			segacd_mark_tiles_dirty(space.machine(), offset);
		}
		else
		{
			printf("Illegal: segacd_main_dataram_part1_w in mode 0 without permission\n");
		}

	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		if (offset<0x20000/2)
		{
			//printf("Unspported: segacd_main_dataram_part1_w (word RAM) in mode 1\n");
			// wordram accees

			// ret bit set by sub cpu determines which half of WorkRAM we have access to?
			if (scd_rammode&1)
			{
				segacd_1meg_mode_word_write(space.machine(), offset+0x20000/2, data, mem_mask, 0);
			}
			else
			{
				segacd_1meg_mode_word_write(space.machine(), offset+0x00000/2, data, mem_mask, 0);
			}
		}
		else
		{
		//  printf("Unspported: segacd_main_dataram_part1_w (Cell rearranged RAM) in mode 1 (illega?)\n"); // is this legal??
		}
	}
}

static READ16_HANDLER( scd_hint_vector_r )
{
//  printf("read HINT offset %d\n", offset);

	switch (offset&1)
	{
		case 0x00:
			//return 0x00ff; // doesn't make much sense..
			return 0xffff;
		case 0x01:
			return segacd_hint_register;
	}

	return 0;

}

static READ16_HANDLER( scd_a12006_hint_register_r )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();
	return segacd_hint_register;
}

static WRITE16_HANDLER( scd_a12006_hint_register_w )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();
	COMBINE_DATA(&segacd_hint_register);
}


static TIMER_CALLBACK( segacd_gfx_conversion_timer_callback )
{
	//printf("segacd_gfx_conversion_timer_callback\n");

	CHECK_SCD_LV1_INTERRUPT

	segacd_conversion_active = 0;

	// this ends up as 0 after processing (soniccd bonus stage)
	segacd_imagebuffer_vdot_size = 0;

}


// the tiles in RAM are 8x8 tiles
// they are referenced in the cell look-up map as either 16x16 or 32x32 tiles (made of 4 / 16 8x8 tiles)

#define SEGACD_BYTES_PER_TILE16 (128)
#define SEGACD_BYTES_PER_TILE32 (512)

#define SEGACD_NUM_TILES16 (0x40000/SEGACD_BYTES_PER_TILE16)
#define SEGACD_NUM_TILES32 (0x40000/SEGACD_BYTES_PER_TILE32)

/*
static const gfx_layout sega_8x8_layout =
{
    8,8,
    SEGACD_NUM_TILES16,
    4,
    { 0,1,2,3 },
    { 8,12,0,4,24,28,16,20 },
    { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
    8*32
};
*/

/* also create pre-rotated versions.. - it might still be possible to use these decodes with our own copying routines */


#define _16x16_SEQUENCE_1  { 8,12,0,4,24,28,16,20, 512+8, 512+12, 512+0, 512+4, 512+24, 512+28, 512+16, 512+20 },
#define _16x16_SEQUENCE_1_FLIP  { 512+20,512+16,512+28,512+24,512+4,512+0, 512+12,512+8, 20,16,28,24,4,0,12,8 },

#define _16x16_SEQUENCE_2  { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32,10*32,11*32,12*32,13*32,14*32,15*32 },
#define _16x16_SEQUENCE_2_FLIP  { 15*32, 14*32, 13*32, 12*32, 11*32, 10*32, 9*32, 8*32, 7*32, 6*32, 5*32, 4*32, 3*32, 2*32, 1*32, 0*32 },


#define _16x16_START \
{ \
	16,16, \
	SEGACD_NUM_TILES16, \
	4, \
	{ 0,1,2,3 }, \

#define _16x16_END \
		8*128 \
}; \

#define _32x32_START \
{ \
	32,32, \
	SEGACD_NUM_TILES32, \
	4, \
	{ 0,1,2,3 }, \


#define _32x32_END \
	8*512 \
}; \



#define _32x32_SEQUENCE_1 \
	{ 8,12,0,4,24,28,16,20, \
	1024+8, 1024+12, 1024+0, 1024+4, 1024+24, 1024+28, 1024+16, 1024+20, \
	2048+8, 2048+12, 2048+0, 2048+4, 2048+24, 2048+28, 2048+16, 2048+20, \
	3072+8, 3072+12, 3072+0, 3072+4, 3072+24, 3072+28, 3072+16, 3072+20  \
	}, \

#define _32x32_SEQUENCE_1_FLIP \
{ 3072+20, 3072+16, 3072+28, 3072+24, 3072+4, 3072+0, 3072+12, 3072+8, \
  2048+20, 2048+16, 2048+28, 2048+24, 2048+4, 2048+0, 2048+12, 2048+8, \
  1024+20, 1024+16, 1024+28, 1024+24, 1024+4, 1024+0, 1024+12, 1024+8, \
  20, 16, 28, 24, 4, 0, 12, 8}, \


#define _32x32_SEQUENCE_2 \
		{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, \
    	8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32, \
	 16*32,17*32,18*32,19*32,20*32,21*32,22*32,23*32, \
	 24*32,25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32}, \

#define _32x32_SEQUENCE_2_FLIP \
{ 31*32, 30*32, 29*32, 28*32, 27*32, 26*32, 25*32, 24*32, \
  23*32, 22*32, 21*32, 20*32, 19*32, 18*32, 17*32, 16*32, \
  15*32, 14*32, 13*32, 12*32, 11*32, 10*32, 9*32 , 8*32 , \
  7*32 , 6*32 , 5*32 , 4*32 , 3*32 , 2*32 , 1*32 , 0*32}, \


/* 16x16 decodes */
static const gfx_layout sega_16x16_r00_f0_layout =
_16x16_START
	_16x16_SEQUENCE_1
	_16x16_SEQUENCE_2
_16x16_END

static const gfx_layout sega_16x16_r01_f0_layout =
_16x16_START
	_16x16_SEQUENCE_2
	_16x16_SEQUENCE_1_FLIP
_16x16_END

static const gfx_layout sega_16x16_r10_f0_layout =
_16x16_START
	_16x16_SEQUENCE_1_FLIP
	_16x16_SEQUENCE_2_FLIP
_16x16_END

static const gfx_layout sega_16x16_r11_f0_layout =
_16x16_START
	_16x16_SEQUENCE_2_FLIP
	_16x16_SEQUENCE_1
_16x16_END

static const gfx_layout sega_16x16_r00_f1_layout =
_16x16_START
	_16x16_SEQUENCE_1_FLIP
	_16x16_SEQUENCE_2
_16x16_END

static const gfx_layout sega_16x16_r01_f1_layout =
_16x16_START
	_16x16_SEQUENCE_2
	_16x16_SEQUENCE_1
_16x16_END

static const gfx_layout sega_16x16_r10_f1_layout =
_16x16_START
	_16x16_SEQUENCE_1
	_16x16_SEQUENCE_2_FLIP
_16x16_END

static const gfx_layout sega_16x16_r11_f1_layout =
_16x16_START
	_16x16_SEQUENCE_2_FLIP
	_16x16_SEQUENCE_1_FLIP
_16x16_END

/* 32x32 decodes */
static const gfx_layout sega_32x32_r00_f0_layout =
_32x32_START
	_32x32_SEQUENCE_1
	_32x32_SEQUENCE_2
_32x32_END

static const gfx_layout sega_32x32_r01_f0_layout =
_32x32_START
	_32x32_SEQUENCE_2
	_32x32_SEQUENCE_1_FLIP
_32x32_END

static const gfx_layout sega_32x32_r10_f0_layout =
_32x32_START
	_32x32_SEQUENCE_1_FLIP
	_32x32_SEQUENCE_2_FLIP
_32x32_END

static const gfx_layout sega_32x32_r11_f0_layout =
_32x32_START
	_32x32_SEQUENCE_2_FLIP
	_32x32_SEQUENCE_1
_32x32_END

static const gfx_layout sega_32x32_r00_f1_layout =
_32x32_START
	_32x32_SEQUENCE_1_FLIP
	_32x32_SEQUENCE_2
_32x32_END

static const gfx_layout sega_32x32_r01_f1_layout =
_32x32_START
	_32x32_SEQUENCE_2
	_32x32_SEQUENCE_1
_32x32_END

static const gfx_layout sega_32x32_r10_f1_layout =
_32x32_START
	_32x32_SEQUENCE_1
	_32x32_SEQUENCE_2_FLIP
_32x32_END

static const gfx_layout sega_32x32_r11_f1_layout =
_32x32_START
	_32x32_SEQUENCE_2_FLIP
	_32x32_SEQUENCE_1_FLIP
_32x32_END


static void segacd_mark_tiles_dirty(running_machine& machine, int offset)
{
	machine.gfx[0]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	machine.gfx[1]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	machine.gfx[2]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	machine.gfx[3]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	machine.gfx[4]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	machine.gfx[5]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	machine.gfx[6]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	machine.gfx[7]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));

	machine.gfx[8]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	machine.gfx[9]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	machine.gfx[10]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	machine.gfx[11]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	machine.gfx[12]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	machine.gfx[13]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	machine.gfx[14]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	machine.gfx[15]->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
}


// mame specific.. map registers to which tilemap cache we use
static int segacd_get_active_stampmap_tilemap(void)
{
	return (segacd_stampsize & 0x6)>>1;
}

#if 0
static void segacd_mark_stampmaps_dirty(void)
{
	segacd_stampmap[segacd_get_active_stampmap_tilemap(->mark_all_dirty()]);

	//segacd_stampmap[0]->mark_all_dirty();
	//segacd_stampmap[1]->mark_all_dirty();
	//segacd_stampmap[2]->mark_all_dirty();
	//segacd_stampmap[3]->mark_all_dirty();
}
#endif

void SCD_GET_TILE_INFO_16x16_1x1( int& tile_region, int& tileno, int tile_index )
{
	tile_region = 0; // 16x16 tiles
	int tile_base = (segacd_stampmap_base_address & 0xff80) * 4;

	int tiledat = segacd_dataram[((tile_base>>1)+tile_index) & 0x1ffff];
	tileno = tiledat & 0x07ff;
	int xflip =  tiledat & 0x8000;
	int roll  =  (tiledat & 0x6000)>>13;

	if (xflip) tile_region += 4;
	tile_region+=roll;
}

void SCD_GET_TILE_INFO_32x32_1x1( int& tile_region, int& tileno, int tile_index )
{
	tile_region = 8; // 32x32 tiles
	int tile_base = (segacd_stampmap_base_address & 0xffe0) * 4;

	int tiledat = segacd_dataram[((tile_base>>1)+tile_index) & 0x1ffff];
	tileno = (tiledat & 0x07fc)>>2;
	int xflip =  tiledat & 0x8000;
	int roll  =  (tiledat & 0x6000)>>13;

	if (xflip) tile_region += 4;
	tile_region+=roll;
}


void SCD_GET_TILE_INFO_16x16_16x16( int& tile_region, int& tileno, int tile_index )
{
	tile_region = 0; // 16x16 tiles
	int tile_base = (0x8000) * 4; // fixed address in this mode

	int tiledat = segacd_dataram[((tile_base>>1)+tile_index) & 0x1ffff];
	tileno = tiledat & 0x07ff;
	int xflip =  tiledat & 0x8000;
	int roll  =  (tiledat & 0x6000)>>13;

	if (xflip) tile_region += 4;
	tile_region+=roll;
}


void SCD_GET_TILE_INFO_32x32_16x16( int& tile_region, int& tileno, int tile_index )
{
	tile_region = 8; // 32x32 tiles
	int tile_base = (segacd_stampmap_base_address & 0xe000) * 4;

	int tiledat = segacd_dataram[((tile_base>>1)+tile_index) & 0x1ffff];
	tileno = (tiledat & 0x07fc)>>2;
	int xflip =  tiledat & 0x8000;
	int roll  =  (tiledat & 0x6000)>>13;

	if (xflip) tile_region += 4;
	tile_region+=roll;
}

/* Tilemap callbacks (we don't actually use the tilemaps due to the excessive overhead */



TILE_GET_INFO_MEMBER( md_base_state::get_stampmap_16x16_1x1_tile_info )
{
	int tile_region, tileno;
	SCD_GET_TILE_INFO_16x16_1x1(tile_region,tileno,(int)tile_index);
	SET_TILE_INFO_MEMBER(tile_region, tileno, 0, 0);
}

TILE_GET_INFO_MEMBER( md_base_state::get_stampmap_32x32_1x1_tile_info )
{
	int tile_region, tileno;
	SCD_GET_TILE_INFO_32x32_1x1(tile_region,tileno,(int)tile_index);
	SET_TILE_INFO_MEMBER(tile_region, tileno, 0, 0);
}


TILE_GET_INFO_MEMBER( md_base_state::get_stampmap_16x16_16x16_tile_info )
{
	int tile_region, tileno;
	SCD_GET_TILE_INFO_16x16_16x16(tile_region,tileno,(int)tile_index);
	SET_TILE_INFO_MEMBER(tile_region, tileno, 0, 0);
}

TILE_GET_INFO_MEMBER( md_base_state::get_stampmap_32x32_16x16_tile_info )
{
	int tile_region, tileno;
	SCD_GET_TILE_INFO_32x32_16x16(tile_region,tileno,(int)tile_index);
	SET_TILE_INFO_MEMBER(tile_region, tileno, 0, 0);
}

// non-tilemap functions to get a pixel from a 'tilemap' based on the above, but looking up each pixel, as to avoid the heavy cache bitmap

INLINE UINT8 get_stampmap_16x16_1x1_tile_info_pixel(running_machine& machine, int xpos, int ypos)
{
	const int tilesize = 4; // 0xf pixels
	const int tilemapsize = 0x0f;

	int wraparound = segacd_stampsize&1;

	int xtile = xpos / (1<<tilesize);
	int ytile = ypos / (1<<tilesize);

	if (wraparound)
	{
		// wrap...
		xtile &= tilemapsize;
		ytile &= tilemapsize;
	}
	else
	{
		if (xtile>tilemapsize) return 0;
		if (xtile<0) return 0;

		if (ytile>tilemapsize) return 0;
		if (ytile<0) return 0;
	}

	int tile_index = (ytile * (tilemapsize+1)) + xtile;

	int tile_region, tileno;
	SCD_GET_TILE_INFO_16x16_1x1(tile_region,tileno,(int)tile_index);

	gfx_element *gfx = machine.gfx[tile_region];
	tileno %= gfx->elements();

	if (tileno==0) return 0x00;

	const UINT8* srcdata = gfx->get_data(tileno);
	return srcdata[((ypos&((1<<tilesize)-1))*(1<<tilesize))+(xpos&((1<<tilesize)-1))];
}

INLINE UINT8 get_stampmap_32x32_1x1_tile_info_pixel(running_machine& machine, int xpos, int ypos)
{
	const int tilesize = 5; // 0x1f pixels
	const int tilemapsize = 0x07;

	int wraparound = segacd_stampsize&1;

	int xtile = xpos / (1<<tilesize);
	int ytile = ypos / (1<<tilesize);

	if (wraparound)
	{
		// wrap...
		xtile &= tilemapsize;
		ytile &= tilemapsize;
	}
	else
	{
		if (xtile>tilemapsize) return 0;
		if (xtile<0) return 0;

		if (ytile>tilemapsize) return 0;
		if (ytile<0) return 0;
	}

	int tile_index = (ytile * (tilemapsize+1)) + xtile;

	int tile_region, tileno;
	SCD_GET_TILE_INFO_32x32_1x1(tile_region,tileno,(int)tile_index);

	gfx_element *gfx = machine.gfx[tile_region];
	tileno %= gfx->elements();

	if (tileno==0) return 0x00; // does this apply in this mode?

	const UINT8* srcdata = gfx->get_data(tileno);
	return srcdata[((ypos&((1<<tilesize)-1))*(1<<tilesize))+(xpos&((1<<tilesize)-1))];
}

INLINE UINT8 get_stampmap_16x16_16x16_tile_info_pixel(running_machine& machine, int xpos, int ypos)
{
	const int tilesize = 4; // 0xf pixels
	const int tilemapsize = 0xff;

	int wraparound = segacd_stampsize&1;

	int xtile = xpos / (1<<tilesize);
	int ytile = ypos / (1<<tilesize);

	if (wraparound)
	{
		// wrap...
		xtile &= tilemapsize;
		ytile &= tilemapsize;
	}
	else
	{
		if (xtile>tilemapsize) return 0;
		if (xtile<0) return 0;

		if (ytile>tilemapsize) return 0;
		if (ytile<0) return 0;
	}

	int tile_index = (ytile * (tilemapsize+1)) + xtile;

	int tile_region, tileno;
	SCD_GET_TILE_INFO_16x16_16x16(tile_region,tileno,(int)tile_index);

	gfx_element *gfx = machine.gfx[tile_region];
	tileno %= gfx->elements();

	if (tileno==0) return 0x00; // does this apply in this mode

	const UINT8* srcdata = gfx->get_data(tileno);
	return srcdata[((ypos&((1<<tilesize)-1))*(1<<tilesize))+(xpos&((1<<tilesize)-1))];
}

INLINE UINT8 get_stampmap_32x32_16x16_tile_info_pixel(running_machine& machine, int xpos, int ypos)
{
	const int tilesize = 5; // 0x1f pixels
	const int tilemapsize = 0x7f;

	int wraparound = segacd_stampsize&1;

	int xtile = xpos / (1<<tilesize);
	int ytile = ypos / (1<<tilesize);

	if (wraparound)
	{
		// wrap...
		xtile &= tilemapsize;
		ytile &= tilemapsize;
	}
	else
	{
		if (xtile>tilemapsize) return 0;
		if (xtile<0) return 0;

		if (ytile>tilemapsize) return 0;
		if (ytile<0) return 0;
	}

	int tile_index = (ytile * (tilemapsize+1)) + xtile;

	int tile_region, tileno;
	SCD_GET_TILE_INFO_32x32_16x16(tile_region,tileno,(int)tile_index);

	gfx_element *gfx = machine.gfx[tile_region];
	tileno %= gfx->elements();

	if (tileno==0) return 0x00;

	const UINT8* srcdata = gfx->get_data(tileno);
	return srcdata[((ypos&((1<<tilesize)-1))*(1<<tilesize))+(xpos&((1<<tilesize)-1))];
}

static TIMER_CALLBACK( segacd_access_timer_callback )
{
	CheckCommand(machine);
}

READ16_HANDLER( cdc_data_sub_r )
{
	return CDC_Host_r(space.machine(), READ_SUB);
}

READ16_HANDLER( cdc_data_main_r )
{
	return CDC_Host_r(space.machine(), READ_MAIN);
}



WRITE16_HANDLER( segacd_stopwatch_timer_w )
{
	if(data == 0)
		stopwatch_timer->reset();
	else
		printf("Stopwatch timer %04x\n",data);
}

READ16_HANDLER( segacd_stopwatch_timer_r )
{
	INT32 result = (stopwatch_timer->time_elapsed() * ATTOSECONDS_TO_HZ(ATTOSECONDS_IN_USEC(30.72))).as_double();

	return result & 0xfff;
}


/* main CPU map set up in INIT */
void segacd_init_main_cpu( running_machine& machine )
{
	address_space& space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	
	segacd_font_bits = reinterpret_cast<UINT16 *>(machine.root_device().memshare(":segacd:segacd_font")->ptr());
	segacd_backupram = reinterpret_cast<UINT16 *>(machine.root_device().memshare(":segacd:backupram")->ptr());
	segacd_dataram = reinterpret_cast<UINT16 *>(machine.root_device().memshare(":segacd:dataram")->ptr());
	segacd_dataram2 = reinterpret_cast<UINT16 *>(machine.root_device().memshare(":segacd:dataram2")->ptr());
	segacd_4meg_prgram = reinterpret_cast<UINT16 *>(machine.root_device().memshare(":segacd:segacd_program")->ptr());
	
	segacd_4meg_prgbank = 0;


	space.unmap_readwrite        (0x020000,0x3fffff);

//  space.install_read_bank(0x0020000, 0x003ffff, "scd_4m_prgbank");
//  space.machine().root_device().membank("scd_4m_prgbank")->set_base(segacd_4meg_prgram + segacd_4meg_prgbank * 0x20000 );
	space.install_legacy_read_handler (0x0020000, 0x003ffff, FUNC(scd_4m_prgbank_ram_r) );
	space.install_legacy_write_handler (0x0020000, 0x003ffff, FUNC(scd_4m_prgbank_ram_w) );
	segacd_wordram_mapped = 1;


	space.machine().device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0x200000, 0x23ffff, FUNC(segacd_main_dataram_part1_r), FUNC(segacd_main_dataram_part1_w)); // RAM shared with sub

	space.machine().device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0xa12000, 0xa12001, FUNC(scd_a12000_halt_reset_r), FUNC(scd_a12000_halt_reset_w)); // sub-cpu control
	space.machine().device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0xa12002, 0xa12003, FUNC(scd_a12002_memory_mode_r), FUNC(scd_a12002_memory_mode_w)); // memory mode / write protect
	//space.machine().device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0xa12004, 0xa12005, FUNC(segacd_cdc_mode_address_r), FUNC(segacd_cdc_mode_address_w));
	space.machine().device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0xa12006, 0xa12007, FUNC(scd_a12006_hint_register_r), FUNC(scd_a12006_hint_register_w)); // where HINT points on main CPU
	//space.machine().device("maincpu")->memory().space(AS_PROGRAM).install_legacy_read_handler     (0xa12008, 0xa12009, FUNC(cdc_data_main_r));


	space.machine().device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0xa1200c, 0xa1200d, FUNC(segacd_stopwatch_timer_r), FUNC(segacd_stopwatch_timer_w)); // starblad

	space.machine().device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0xa1200e, 0xa1200f, FUNC(segacd_comms_flags_r), FUNC(segacd_comms_flags_maincpu_w)); // communication flags block

	space.machine().device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0xa12010, 0xa1201f, FUNC(segacd_comms_main_part1_r), FUNC(segacd_comms_main_part1_w));
	space.machine().device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0xa12020, 0xa1202f, FUNC(segacd_comms_main_part2_r), FUNC(segacd_comms_main_part2_w));



	machine.device(":segacd:segacd_68k")->execute().set_irq_acknowledge_callback(segacd_sub_int_callback);

	space.install_legacy_read_handler (0x0000070, 0x0000073, FUNC(scd_hint_vector_r) );

	segacd_gfx_conversion_timer = machine.scheduler().timer_alloc(FUNC(segacd_gfx_conversion_timer_callback));
	segacd_gfx_conversion_timer->adjust(attotime::never);

	//segacd_dmna_ret_timer = machine.scheduler().timer_alloc(FUNC(segacd_dmna_ret_timer_callback));
	segacd_gfx_conversion_timer->adjust(attotime::never);

	segacd_hock_timer = machine.scheduler().timer_alloc(FUNC(segacd_access_timer_callback));
//  segacd_hock_timer->adjust( attotime::from_nsec(20000000), 0, attotime::from_nsec(20000000));
	segacd_hock_timer->adjust( attotime::from_hz(75),0, attotime::from_hz(75));

	segacd_irq3_timer = machine.scheduler().timer_alloc(FUNC(segacd_irq3_timer_callback));
	segacd_irq3_timer->adjust(attotime::never);



	/* create the char set (gfx will then be updated dynamically from RAM) */
	machine.gfx[0] = auto_alloc(machine, gfx_element(machine, sega_16x16_r00_f0_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[1] = auto_alloc(machine, gfx_element(machine, sega_16x16_r01_f0_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[2] = auto_alloc(machine, gfx_element(machine, sega_16x16_r10_f0_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[3] = auto_alloc(machine, gfx_element(machine, sega_16x16_r11_f0_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[4] = auto_alloc(machine, gfx_element(machine, sega_16x16_r00_f1_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[5] = auto_alloc(machine, gfx_element(machine, sega_16x16_r11_f1_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[6] = auto_alloc(machine, gfx_element(machine, sega_16x16_r10_f1_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[7] = auto_alloc(machine, gfx_element(machine, sega_16x16_r01_f1_layout, (UINT8 *)segacd_dataram, 0, 0));

	machine.gfx[8] = auto_alloc(machine, gfx_element(machine, sega_32x32_r00_f0_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[9] = auto_alloc(machine, gfx_element(machine, sega_32x32_r01_f0_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[10]= auto_alloc(machine, gfx_element(machine, sega_32x32_r10_f0_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[11]= auto_alloc(machine, gfx_element(machine, sega_32x32_r11_f0_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[12]= auto_alloc(machine, gfx_element(machine, sega_32x32_r00_f1_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[13]= auto_alloc(machine, gfx_element(machine, sega_32x32_r11_f1_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[14]= auto_alloc(machine, gfx_element(machine, sega_32x32_r10_f1_layout, (UINT8 *)segacd_dataram, 0, 0));
	machine.gfx[15]= auto_alloc(machine, gfx_element(machine, sega_32x32_r01_f1_layout, (UINT8 *)segacd_dataram, 0, 0));

	md_base_state *state = machine.driver_data<md_base_state>();
	segacd_stampmap[0] = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(md_base_state::get_stampmap_16x16_1x1_tile_info),state), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	segacd_stampmap[1] = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(md_base_state::get_stampmap_32x32_1x1_tile_info),state), TILEMAP_SCAN_ROWS, 32, 32, 8, 8);
	segacd_stampmap[2] = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(md_base_state::get_stampmap_16x16_16x16_tile_info),state), TILEMAP_SCAN_ROWS, 16, 16, 256, 256); // 128kb!
	segacd_stampmap[3] = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(md_base_state::get_stampmap_32x32_16x16_tile_info),state), TILEMAP_SCAN_ROWS, 32, 32, 128, 128); // 32kb!
}




TIMER_DEVICE_CALLBACK( scd_dma_timer_callback )
{
	// todo: accurate timing of this!

	#define RATE 256
	if (sega_cd_connected)
		CDC_Do_DMA(timer.machine(), RATE);

	// timed reset of flags
	scd_mode_dmna_ret_flags |= 0x0021;

	scd_dma_timer->adjust(attotime::from_hz(megadriv_framerate) / megadrive_total_scanlines);
}


MACHINE_RESET( segacd )
{
	_segacd_68k_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	_segacd_68k_cpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	segacd_hint_register = 0xffff; // -1

	/* init cd-rom device */

	lc89510_Reset();

	{
		cdrom_image_device *device = machine.device<cdrom_image_device>("cdrom");
		if ( device )
		{
			segacd.cd = device->get_cdrom_file();
			if ( segacd.cd )
			{
				segacd.toc = cdrom_get_toc( segacd.cd );
				cdda_set_cdrom( machine.device(":segacd:cdda"), segacd.cd );
				cdda_stop_audio( machine.device( ":segacd:cdda" ) ); //stop any pending CD-DA
			}
		}
	}


	if (segacd.cd)
		printf("cd found\n");

	scd_rammode = 0;
	scd_mode_dmna_ret_flags = 0x5421;


	hock_cmd = 0;
	stopwatch_timer = machine.device<timer_device>(":segacd:sw_timer");

	scd_dma_timer->adjust(attotime::zero);


	// HACK!!!! timegal, anettfut, roadaven end up with the SubCPU waiting in a loop for *something*
	// overclocking the CPU, even at the point where the game is hung, allows them to continue and boot
	// I'm not sure what the source of this timing problem is, it's not using IRQ3 or StopWatch at the
	// time.  Changing the CDHock timer to 50hz from 75hz also stops the hang, but then the video is
	// too slow and has bad sound.  -- Investigate!

	_segacd_68k_cpu->set_clock_scale(1.5000f);

}


static int segacd_redled = 0;
static int segacd_greenled = 0;
static int segacd_ready = 1; // actually set 100ms after startup?

static READ16_HANDLER( segacd_sub_led_ready_r )
{
	UINT16 retdata = 0x0000;

	if (ACCESSING_BITS_0_7)
	{
		retdata |= segacd_ready;
	}

	if (ACCESSING_BITS_8_15)
	{
		retdata |= segacd_redled << 8;
		retdata |= segacd_greenled << 9;
	}

	return retdata;
}

static WRITE16_HANDLER( segacd_sub_led_ready_w )
{
	if (ACCESSING_BITS_0_7)
	{
		if ((data&0x01) == 0x00)
		{
			// reset CD unit
		}
	}

	if (ACCESSING_BITS_8_15)
	{
		segacd_redled = (data >> 8)&1;
		segacd_greenled = (data >> 9)&1;

		output_set_value("red_led",segacd_redled ^ 1);
		output_set_value("green_led",segacd_greenled ^ 1);

		//popmessage("%02x %02x",segacd_greenled,segacd_redled);
	}
}



static READ16_HANDLER( segacd_sub_dataram_part1_r )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		// is this correct?
		if (scd_rammode&1)
			return segacd_dataram[offset];
		else
		{
			printf("Illegal: segacd_sub_dataram_part1_r in mode 0 without permission\n");
			return 0x0000;
		}
	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
//      printf("Unspported: segacd_sub_dataram_part1_r in mode 1 (Word RAM Expander - 1 Byte Per Pixel)\n");
		UINT16 data;

		if (scd_rammode&1)
		{
			data = segacd_1meg_mode_word_read(offset/2+0x00000/2, 0xffff);
		}
		else
		{
			data = segacd_1meg_mode_word_read(offset/2+0x20000/2, 0xffff);
		}

		if (offset&1)
		{
			return ((data & 0x00f0) << 4) | ((data & 0x000f) << 0);
		}
		else
		{
			return ((data & 0xf000) >> 4) | ((data & 0x0f00) >> 8);
		}


	}

	return 0x0000;
}

static WRITE16_HANDLER( segacd_sub_dataram_part1_w )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		// is this correct?
		if (scd_rammode&1)
		{
			COMBINE_DATA(&segacd_dataram[offset]);
			segacd_mark_tiles_dirty(space.machine(), offset);
		}
		else
		{
			printf("Illegal: segacd_sub_dataram_part1_w in mode 0 without permission\n");
		}
	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		//if (mem_mask==0xffff)
		//  printf("Unspported: segacd_sub_dataram_part1_w in mode 1 (Word RAM Expander - 1 Byte Per Pixel) %04x %04x\n", data, mem_mask);

		data = (data & 0x000f) | (data & 0x0f00)>>4;
		mem_mask = (mem_mask & 0x000f) | (mem_mask & 0x0f00)>>4;

//      data = ((data & 0x00f0) >>4) | (data & 0xf000)>>8;
//      mem_mask = ((mem_mask & 0x00f0)>>4) | ((mem_mask & 0xf000)>>8);


		if (!(offset&1))
		{
			data <<=8;
			mem_mask <<=8;
		}

		if (scd_rammode&1)
		{
			segacd_1meg_mode_word_write(space.machine(), offset/2+0x00000/2, data , mem_mask, 1);
		}
		else
		{
			segacd_1meg_mode_word_write(space.machine(), offset/2+0x20000/2, data, mem_mask, 1);
		}

	//  printf("Unspported: segacd_sub_dataram_part1_w in mode 1 (Word RAM Expander - 1 Byte Per Pixel) %04x\n", data);
	}
}

static READ16_HANDLER( segacd_sub_dataram_part2_r )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		printf("ILLEGAL segacd_sub_dataram_part2_r in mode 0\n"); // not mapped to anything in mode 0
		return 0x0000;
	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		//printf("Unsupported: segacd_sub_dataram_part2_r in mode 1 (Word RAM)\n");
		// ret bit set by sub cpu determines which half of WorkRAM we have access to?
		if (scd_rammode&1)
		{
			return segacd_1meg_mode_word_read(offset+0x00000/2, mem_mask);
		}
		else
		{
			return segacd_1meg_mode_word_read(offset+0x20000/2, mem_mask);
		}

	}

	return 0x0000;
}

static WRITE16_HANDLER( segacd_sub_dataram_part2_w )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		printf("ILLEGAL segacd_sub_dataram_part2_w in mode 0\n"); // not mapped to anything in mode 0
	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		//printf("Unsupported: segacd_sub_dataram_part2_w in mode 1 (Word RAM)\n");
		// ret bit set by sub cpu determines which half of WorkRAM we have access to?
		if (scd_rammode&1)
		{
			segacd_1meg_mode_word_write(space.machine(),offset+0x00000/2, data, mem_mask, 0);
		}
		else
		{
			segacd_1meg_mode_word_write(space.machine(),offset+0x20000/2, data, mem_mask, 0);
		}

	}
}



static READ16_HANDLER( segacd_irq_mask_r )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();
	return segacd_irq_mask;
}

static WRITE16_HANDLER( segacd_irq_mask_w )
{
	if (ACCESSING_BITS_0_7)
	{
		UINT16 control = CDD_CONTROL;
		if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();
	//  printf("segacd_irq_mask_w %04x %04x (CDD control is %04x)\n",data, mem_mask, control);

		if (data & 0x10)
		{
			if (control & 0x04)
			{
				if (!(segacd_irq_mask & 0x10))
				{
					segacd_irq_mask = data & 0x7e;
					CDD_Process(space.machine(), 0);
					return;
				}
			}
		}

		segacd_irq_mask = data & 0x7e;
	}
	else
	{

		printf("segacd_irq_mask_w only MSB written\n");

	}
}

static READ16_HANDLER( segacd_cdd_ctrl_r )
{
	if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();
	return CDD_CONTROL;
}


static WRITE16_HANDLER( segacd_cdd_ctrl_w )
{
	if (ACCESSING_BITS_0_7)
	{
		UINT16 control = CDD_CONTROL;
		if (SEGACD_FORCE_SYNCS) space.machine().scheduler().synchronize();

		//printf("segacd_cdd_ctrl_w %04x %04x (control %04x irq %04x\n", data, mem_mask, control, segacd_irq_mask);

		data &=0x4; // only HOCK bit is writable

		if (data & 0x4)
		{
			if (!(control & 0x4))
			{
				if (segacd_irq_mask&0x10)
				{
					CDD_Process(space.machine(), 1);
				}
			}
		}

		CDD_CONTROL |= data;
	}
	else
	{
		printf("segacd_cdd_ctrl_w only MSB written\n");
	}
}



static READ8_HANDLER( segacd_cdd_rx_r )
{
	return CDD_RX[offset^1];
}

static WRITE8_HANDLER( segacd_cdd_tx_w )
{
	CDD_TX[offset^1] = data;

	if(offset == 9)
	{
		CDD_Import(space.machine());
	}
}



static READ16_HANDLER( segacd_stampsize_r )
{
	UINT16 retdata = 0x0000;

	retdata |= segacd_conversion_active<<15;

	retdata |= segacd_stampsize & 0x7;

	return retdata;

}

static WRITE16_HANDLER( segacd_stampsize_w )
{
	//printf("segacd_stampsize_w %04x %04x\n",data, mem_mask);
	if (ACCESSING_BITS_0_7)
	{
		segacd_stampsize = data & 0x07;
		//if (data & 0xf8)
		//  printf("    unused bits (LSB) set in stampsize!\n");

		//if (data&1) printf("    Repeat On\n");
		//else printf("    Repeat Off\n");

		//if (data&2) printf("    32x32 dots\n");
		//else printf("    16x16 dots\n");

		//if (data&4) printf("    16x16 screens\n");
		//else printf("    1x1 screen\n");
	}

	if (ACCESSING_BITS_8_15)
	{
		//if (data&0xff00) printf("    unused bits (MSB) set in stampsize!\n");
	}
}

// these functions won't cope if
//
// the lower 3 bits of segacd_imagebuffer_hdot_size are set

// this really needs to be doing it's own lookups rather than depending on the inefficient MAME cache..
INLINE UINT8 read_pixel_from_stampmap( running_machine& machine, bitmap_ind16* srcbitmap, int x, int y)
{
/*
    if (!srcbitmap)
    {
        return machine.rand();
    }

    if (x >= srcbitmap->width) return 0;
    if (y >= srcbitmap->height) return 0;

    UINT16* cacheptr = &srcbitmap->pix16(y, x);

    return cacheptr[0] & 0xf;
*/

	switch (segacd_get_active_stampmap_tilemap()&3)
	{
		case 0x00: return get_stampmap_16x16_1x1_tile_info_pixel( machine, x, y );
		case 0x01: return get_stampmap_32x32_1x1_tile_info_pixel( machine, x, y );
		case 0x02: return get_stampmap_16x16_16x16_tile_info_pixel( machine, x, y );
		case 0x03: return get_stampmap_32x32_16x16_tile_info_pixel( machine, x, y );
	}

	return 0;
}





// this triggers the conversion operation, which will cause an IRQ1 when finished
WRITE16_HANDLER( segacd_trace_vector_base_address_w )
{
	if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		printf("ILLEGAL: segacd_trace_vector_base_address_w %04x %04x in mode 1!\n",data,mem_mask);
	}

	//printf("segacd_trace_vector_base_address_w %04x %04x\n",data,mem_mask);

	{
		int base = (data & 0xfffe) * 4;

		//printf("actual base = %06x\n", base + 0x80000);

		// nasty nasty nasty
		//segacd_mark_stampmaps_dirty();

		segacd_conversion_active = 1;

		// todo: proper time calculation
		segacd_gfx_conversion_timer->adjust(attotime::from_nsec(30000));



		int line;
		//bitmap_ind16 *srcbitmap = segacd_stampmap[segacd_get_active_stampmap_tilemap(->pixmap()]);
		bitmap_ind16 *srcbitmap = 0;
		UINT32 bufferstart = ((segacd_imagebuffer_start_address&0xfff8)*2)<<3;

		for (line=0;line<segacd_imagebuffer_vdot_size;line++)
		{
			int currbase = base + line * 0x8;

			// are the 256x256 tile modes using the same sign bits?
			INT16 tilemapxoffs,tilemapyoffs;
			INT16 deltax,deltay;

			tilemapxoffs = segacd_dataram[(currbase+0x0)>>1];
			tilemapyoffs = segacd_dataram[(currbase+0x2)>>1];
			deltax = segacd_dataram[(currbase+0x4)>>1]; // x-zoom
			deltay = segacd_dataram[(currbase+0x6)>>1]; // rotation

			//printf("%06x:  %04x (%d) %04x (%d) %04x %04x\n", currbase, tilemapxoffs, tilemapxoffs>>3, tilemapyoffs, tilemapyoffs>>3, deltax, deltay);

			int xbase = tilemapxoffs * 256;
			int ybase = tilemapyoffs * 256;
			int count;

			for (count=0;count<(segacd_imagebuffer_hdot_size);count++)
			{
				//int i;
				UINT8 pix = 0x0;

				pix = read_pixel_from_stampmap(space.machine(), srcbitmap, xbase>>(3+8), ybase>>(3+8));

				xbase += deltax;
				ybase += deltay;

				// clamp to 24-bits, seems to be required for all the intro effects to work
				xbase &= 0xffffff;
				ybase &= 0xffffff;

				int countx = count + (segacd_imagebuffer_offset&0x7);

				UINT32 offset;

				offset = bufferstart+((((segacd_imagebuffer_vcell_size+1)*0x10)*(countx>>3))<<3);

				offset+= ((line*2)<<3);
				offset+=(segacd_imagebuffer_offset&0x38)<<1;

				offset+=countx & 0x7;

				write_pixel( space.machine(), pix, offset );

				segacd_mark_tiles_dirty(space.machine(), (offset>>3));
				segacd_mark_tiles_dirty(space.machine(), (offset>>3)+1);

			}

		}
	}

}

// actually just the low 8 bits?
READ16_HANDLER( segacd_imagebuffer_vdot_size_r )
{
	return segacd_imagebuffer_vdot_size;
}

WRITE16_HANDLER( segacd_imagebuffer_vdot_size_w )
{
	//printf("segacd_imagebuffer_vdot_size_w %04x %04x\n",data,mem_mask);
	COMBINE_DATA(&segacd_imagebuffer_vdot_size);
}


// basically the 'tilemap' base address, for the 16x16 / 32x32 source tiles
static READ16_HANDLER( segacd_stampmap_base_address_r )
{
	// different bits are valid in different modes, but I'm guessing the register
	// always returns all the bits set, even if they're not used?
	return segacd_stampmap_base_address;

}

static WRITE16_HANDLER( segacd_stampmap_base_address_w )
{ // WORD ACCESS

	// low 3 bitsa aren't used, are they stored?
	COMBINE_DATA(&segacd_stampmap_base_address);
}

// destination for 'rendering' the section of the tilemap(stampmap) requested
static READ16_HANDLER( segacd_imagebuffer_start_address_r )
{
	return segacd_imagebuffer_start_address;
}

static WRITE16_HANDLER( segacd_imagebuffer_start_address_w )
{
	COMBINE_DATA(&segacd_imagebuffer_start_address);

	//int base = (segacd_imagebuffer_start_address & 0xfffe) * 4;
	//printf("segacd_imagebuffer_start_address_w %04x %04x (actual base = %06x)\n", data, segacd_imagebuffer_start_address, base);
}

static READ16_HANDLER( segacd_imagebuffer_offset_r )
{
	return segacd_imagebuffer_offset;
}

static WRITE16_HANDLER( segacd_imagebuffer_offset_w )
{
	COMBINE_DATA(&segacd_imagebuffer_offset);
//  printf("segacd_imagebuffer_offset_w %04x\n", segacd_imagebuffer_offset);
}

static READ16_HANDLER( segacd_imagebuffer_vcell_size_r )
{
	return segacd_imagebuffer_vcell_size;
}

static WRITE16_HANDLER( segacd_imagebuffer_vcell_size_w )
{
	COMBINE_DATA(&segacd_imagebuffer_vcell_size);
}


static READ16_HANDLER( segacd_imagebuffer_hdot_size_r )
{
	return segacd_imagebuffer_hdot_size;
}

static WRITE16_HANDLER( segacd_imagebuffer_hdot_size_w )
{
	COMBINE_DATA(&segacd_imagebuffer_hdot_size);
}

static UINT16 segacd_irq3_timer_reg;

static READ16_HANDLER( segacd_irq3timer_r )
{
	return segacd_irq3_timer_reg; // always returns value written, not current counter!
}

#define SEGACD_IRQ3_TIMER_SPEED (attotime::from_nsec(segacd_irq3_timer_reg*30720))

static WRITE16_HANDLER( segacd_irq3timer_w )
{
	if (ACCESSING_BITS_0_7)
	{
		segacd_irq3_timer_reg = data & 0xff;

		// time = reg * 30.72 us

		if (segacd_irq3_timer_reg)
			segacd_irq3_timer->adjust(SEGACD_IRQ3_TIMER_SPEED);
		else
			segacd_irq3_timer->adjust(attotime::never);

		//printf("segacd_irq3timer_w %02x\n", segacd_irq3_timer_reg);
	}
}



static TIMER_CALLBACK( segacd_irq3_timer_callback )
{
	CHECK_SCD_LV3_INTERRUPT

	segacd_irq3_timer->adjust(SEGACD_IRQ3_TIMER_SPEED);
}



READ16_HANDLER( cdc_dmaaddr_r )
{
	return CDC_DMA_ADDR;
}

WRITE16_HANDLER( cdc_dmaaddr_w )
{
	COMBINE_DATA(&CDC_DMA_ADDR);
}

READ16_HANDLER( segacd_cdfader_r )
{
	return 0;
}

WRITE16_HANDLER( segacd_cdfader_w )
{
	static double cdfader_vol;
	if(data & 0x800f)
		printf("CD Fader register write %04x\n",data);

	cdfader_vol = (double)((data & 0x3ff0) >> 4);

	if(data & 0x4000)
		cdfader_vol = 100.0;
	else
		cdfader_vol = (cdfader_vol / 1024.0) * 100.0;

	//printf("%f\n",cdfader_vol);

	cdda_set_volume(space.machine().device(":segacd:cdda"), cdfader_vol);
}

READ16_HANDLER( segacd_backupram_r )
{
	if(ACCESSING_BITS_8_15 && !(space.debugger_access()))
		printf("Warning: read to backupram even bytes! [%04x]\n",offset);

	return segacd_backupram[offset] & 0xff;
}

WRITE16_HANDLER( segacd_backupram_w )
{
	if(ACCESSING_BITS_0_7)
		segacd_backupram[offset] = data;

	if(ACCESSING_BITS_8_15 && !(space.debugger_access()))
		printf("Warning: write to backupram even bytes! [%04x] %02x\n",offset,data);
}

READ16_HANDLER( segacd_font_color_r )
{
	return segacd_font_color;
}

WRITE16_HANDLER( segacd_font_color_w )
{
	if (ACCESSING_BITS_0_7)
	{
		segacd_font_color = data & 0xff;
	}
}

READ16_HANDLER( segacd_font_converted_r )
{
	int scbg = (segacd_font_color & 0x0f);
	int scfg = (segacd_font_color & 0xf0)>>4;
	UINT16 retdata = 0;
	int bit;

	for (bit=0;bit<4;bit++)
	{
		if (*segacd_font_bits&((0x1000>>offset*4)<<bit))
			retdata |= scfg << (bit*4);
		else
			retdata |= scbg << (bit*4);
	}

	return retdata;
}

ADDRESS_MAP_START( segacd_map, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x000000, 0x07ffff) AM_RAM AM_SHARE("segacd_program")

	AM_RANGE(0x080000, 0x0bffff) AM_READWRITE_LEGACY(segacd_sub_dataram_part1_r, segacd_sub_dataram_part1_w) AM_SHARE("dataram")
	AM_RANGE(0x0c0000, 0x0dffff) AM_READWRITE_LEGACY(segacd_sub_dataram_part2_r, segacd_sub_dataram_part2_w) AM_SHARE("dataram2")

	AM_RANGE(0xfe0000, 0xfe3fff) AM_READWRITE_LEGACY(segacd_backupram_r,segacd_backupram_w) AM_SHARE("backupram") // backup RAM, odd bytes only!

	AM_RANGE(0xff0000, 0xff001f) AM_DEVWRITE8_LEGACY("rfsnd", rf5c68_w, 0x00ff)  // PCM, RF5C164
	AM_RANGE(0xff0020, 0xff003f) AM_DEVREAD8_LEGACY("rfsnd", rf5c68_r, 0x00ff)
	AM_RANGE(0xff2000, 0xff3fff) AM_DEVREADWRITE8_LEGACY("rfsnd", rf5c68_mem_r, rf5c68_mem_w,0x00ff)  // PCM, RF5C164


	AM_RANGE(0xff8000 ,0xff8001) AM_READWRITE_LEGACY(segacd_sub_led_ready_r, segacd_sub_led_ready_w)
	AM_RANGE(0xff8002 ,0xff8003) AM_READWRITE_LEGACY(segacd_sub_memory_mode_r, segacd_sub_memory_mode_w)

	AM_RANGE(0xff8004 ,0xff8005) AM_READWRITE_LEGACY(segacd_cdc_mode_address_r, segacd_cdc_mode_address_w)
	AM_RANGE(0xff8006 ,0xff8007) AM_READWRITE_LEGACY(segacd_cdc_data_r, segacd_cdc_data_w)
	AM_RANGE(0xff8008, 0xff8009) AM_READ_LEGACY(cdc_data_sub_r)
	AM_RANGE(0xff800a, 0xff800b) AM_READWRITE_LEGACY(cdc_dmaaddr_r,cdc_dmaaddr_w) // CDC DMA Address
	AM_RANGE(0xff800c, 0xff800d) AM_READWRITE_LEGACY(segacd_stopwatch_timer_r, segacd_stopwatch_timer_w)// Stopwatch timer
	AM_RANGE(0xff800e ,0xff800f) AM_READWRITE_LEGACY(segacd_comms_flags_r, segacd_comms_flags_subcpu_w)
	AM_RANGE(0xff8010 ,0xff801f) AM_READWRITE_LEGACY(segacd_comms_sub_part1_r, segacd_comms_sub_part1_w)
	AM_RANGE(0xff8020 ,0xff802f) AM_READWRITE_LEGACY(segacd_comms_sub_part2_r, segacd_comms_sub_part2_w)
	AM_RANGE(0xff8030, 0xff8031) AM_READWRITE_LEGACY(segacd_irq3timer_r, segacd_irq3timer_w) // Timer W/INT3
	AM_RANGE(0xff8032, 0xff8033) AM_READWRITE_LEGACY(segacd_irq_mask_r,segacd_irq_mask_w)
	AM_RANGE(0xff8034, 0xff8035) AM_READWRITE_LEGACY(segacd_cdfader_r,segacd_cdfader_w) // CD Fader
	AM_RANGE(0xff8036, 0xff8037) AM_READWRITE_LEGACY(segacd_cdd_ctrl_r,segacd_cdd_ctrl_w)
	AM_RANGE(0xff8038, 0xff8041) AM_READ8_LEGACY(segacd_cdd_rx_r,0xffff)
	AM_RANGE(0xff8042, 0xff804b) AM_WRITE8_LEGACY(segacd_cdd_tx_w,0xffff)
	AM_RANGE(0xff804c, 0xff804d) AM_READWRITE_LEGACY(segacd_font_color_r, segacd_font_color_w)
	AM_RANGE(0xff804e, 0xff804f) AM_RAM AM_SHARE("segacd_font")
	AM_RANGE(0xff8050, 0xff8057) AM_READ_LEGACY(segacd_font_converted_r)
	AM_RANGE(0xff8058, 0xff8059) AM_READWRITE_LEGACY(segacd_stampsize_r, segacd_stampsize_w) // Stamp size
	AM_RANGE(0xff805a, 0xff805b) AM_READWRITE_LEGACY(segacd_stampmap_base_address_r, segacd_stampmap_base_address_w) // Stamp map base address
	AM_RANGE(0xff805c, 0xff805d) AM_READWRITE_LEGACY(segacd_imagebuffer_vcell_size_r, segacd_imagebuffer_vcell_size_w)// Image buffer V cell size
	AM_RANGE(0xff805e, 0xff805f) AM_READWRITE_LEGACY(segacd_imagebuffer_start_address_r, segacd_imagebuffer_start_address_w) // Image buffer start address
	AM_RANGE(0xff8060, 0xff8061) AM_READWRITE_LEGACY(segacd_imagebuffer_offset_r, segacd_imagebuffer_offset_w)
	AM_RANGE(0xff8062, 0xff8063) AM_READWRITE_LEGACY(segacd_imagebuffer_hdot_size_r, segacd_imagebuffer_hdot_size_w) // Image buffer H dot size
	AM_RANGE(0xff8064, 0xff8065) AM_READWRITE_LEGACY(segacd_imagebuffer_vdot_size_r, segacd_imagebuffer_vdot_size_w ) // Image buffer V dot size
	AM_RANGE(0xff8066, 0xff8067) AM_WRITE_LEGACY(segacd_trace_vector_base_address_w)// Trace vector base address
//  AM_RANGE(0xff8068, 0xff8069) // Subcode address

//  AM_RANGE(0xff8100, 0xff817f) // Subcode buffer area
//  AM_RANGE(0xff8180, 0xff81ff) // mirror of subcode buffer area

ADDRESS_MAP_END




void sega_segacd_device::device_start()
{

}

void sega_segacd_device::device_reset()
{

}


