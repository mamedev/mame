// license:BSD-3-Clause
// copyright-holders:David Haywood
/* CD controller code from megacd.c, used by Sega CD / Mega CD */

/* todo: cleanup(!!), make more generic, unifiy implementation with NeoCD, turn into a device and move to the proper lc89510.c file
  currently this is a bit of a mix of system specific bits, incomplete implementations etc. as well as a rather kludgy combination
  of the CDC and drive emulation. */


#include "emu.h"
#include "megacdcd.h"

#define LOG_WARN           (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LOGWARN(...)         LOGMASKED(LOG_WARN, __VA_ARGS__)

#define READ_MAIN (0x0200)
#define READ_SUB  (0x0300)

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
#define TOCCMD_TRKPOS    (0x1)
#define TOCCMD_CURTRK    (0x2)
#define TOCCMD_LENGTH    (0x3)
#define TOCCMD_FIRSTLAST (0x4)
#define TOCCMD_TRACKADDR (0x5)




#define SET_CDD_DATA_MODE \
	CDD_CONTROL |= 0x0100;
#define SET_CDD_AUDIO_MODE \
	CDD_CONTROL &= ~0x0100;
#define STOP_CDC_READ \
	SCD_STATUS_CDC &= ~0x01;
#define SET_CDC_READ \
	SCD_STATUS_CDC |= 0x01;
#define SET_CDC_DMA \
	SCD_STATUS_CDC |= 0x08;
#define STOP_CDC_DMA \
	SCD_STATUS_CDC &= ~0x08;
#define SCD_READ_ENABLED \
	(SCD_STATUS_CDC & 1)

#define SCD_DMA_ENABLED \
	(SCD_STATUS_CDC & 0x08)

#define CLEAR_CDD_RESULT \
	CDD_MIN = CDD_SEC = CDD_FRAME = CDD_EXT = 0;
#define CHECK_SCD_LV5_INTERRUPT \
	if (segacd_irq_mask & 0x20) \
	{ \
		m_68k->set_input_line(5, HOLD_LINE); \
	}
#define CHECK_SCD_LV4_INTERRUPT \
	if (segacd_irq_mask & 0x10) \
	{ \
		m_68k->set_input_line(4, HOLD_LINE); \
	}
#define CHECK_SCD_LV4_INTERRUPT_A \
	if (segacd_irq_mask & 0x10) \
	{ \
		m_68k->set_input_line(4, HOLD_LINE); \
	}


#define CURRENT_TRACK_IS_DATA \
	(segacd.toc->tracks[SCD_CURTRK - 1].trktype != cdrom_file::CD_TRACK_AUDIO)

#define CDD_PLAYINGCDDA 0x0100
#define CDD_READY       0x0400
#define CDD_STOPPED     0x0900


/* neocd */

#define CD_FRAMES_MINUTE (60 * 75)
#define CD_FRAMES_SECOND (     75)
#define CD_FRAMES_PREGAP ( 2 * 75)

#define SEK_IRQSTATUS_NONE (0x0000)
#define SEK_IRQSTATUS_AUTO (0x2000)
#define SEK_IRQSTATUS_ACK  (0x1000)


DEFINE_DEVICE_TYPE(LC89510_TEMP, lc89510_temp_device, "lc89510_temp", "lc89510_temp_device")

lc89510_temp_device::lc89510_temp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LC89510_TEMP, tag, owner, clock)
	, m_segacd_dma_callback(*this, FUNC(lc89510_temp_device::Fake_CDC_Do_DMA))
	, m_type1_interrupt_callback(*this, FUNC(lc89510_temp_device::dummy_interrupt_callback))
	, m_type2_interrupt_callback(*this, FUNC(lc89510_temp_device::dummy_interrupt_callback))
	, m_type3_interrupt_callback(*this, FUNC(lc89510_temp_device::dummy_interrupt_callback))
	, m_cdrom(*this, finder_base::DUMMY_TAG)
	, m_cdda(*this, "cdda")
	, m_68k(*this, finder_base::DUMMY_TAG)
{
	is_neoCD = false;

	nff0002 = 0;
	for (auto & elem : CDD_TX)
		elem = 0;
	for (auto & elem : CDD_RX)
		elem = 0;
	NeoCDCommsWordCount = 0;
	NeoCD_StatusHack = 0;
	SCD_CURLBA = 0;

	CDC_REG0 = 0;

	segacd_irq_mask = 0;
}

void lc89510_temp_device::dummy_interrupt_callback()
{
}

// HACK for DMA handling, this gets replaced
void lc89510_temp_device::Fake_CDC_Do_DMA(int &dmacount, uint8_t *CDC_BUFFER, uint16_t &dma_addrc, uint16_t &destination )
{
	fatalerror("Fake_CDC_Do_DMA\n");
}

void lc89510_temp_device::device_start()
{
	m_segacd_dma_callback.resolve();
	m_type1_interrupt_callback.resolve();
	m_type2_interrupt_callback.resolve();
	m_type3_interrupt_callback.resolve();
}

void lc89510_temp_device::device_reset()
{
	m_cdda->stop_audio();
}


inline int lc89510_temp_device::to_bcd(int val, bool byte)
{
	if (val > 99) val = 99;

	if (byte) return (((val) / 10) << 4) + ((val) % 10);
	else return (((val) / 10) << 8) + ((val) % 10);
}



void lc89510_temp_device::set_data_audio_mode(void)
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

void lc89510_temp_device::CDD_DoChecksum(void)
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
		CDD_RX[8];

	if (is_neoCD) checksum += 0x5; // why??
	checksum &= 0xf;
	checksum ^= 0xf;

	CDD_RX[9] = checksum;
}

bool lc89510_temp_device::CDD_Check_TX_Checksum(void)
{
	int checksum =
		CDD_TX[0] +
		CDD_TX[1] +
		CDD_TX[2] +
		CDD_TX[3] +
		CDD_TX[4] +
		CDD_TX[5] +
		CDD_TX[6] +
		CDD_TX[7] +
		CDD_TX[8];

	if (is_neoCD) checksum += 0x5; // why??
	checksum &= 0xf;
	checksum ^= 0xf;

	if (CDD_TX[9] == checksum)
		return true;

	return false;
}

// converts our 16-bit working regs to 8-bit regs and checksums them
void lc89510_temp_device::CDD_Export(bool neocd_hack)
{
	if (!neocd_hack)
		CDD_RX[0] = (CDD_STATUS  & 0xff00)>>8;
	else
	{
	//  LOG("was %02x returning %02x\n", (CDD_STATUS  & 0xff00)>>8, NeoCD_StatusHack);
		CDD_RX[0] = NeoCD_StatusHack;
	}

	CDD_RX[1] = (CDD_STATUS  & 0x00ff)>>0;
	CDD_RX[2] = (CDD_MIN  & 0xff00)>>8;
	CDD_RX[3] = (CDD_MIN  & 0x00ff)>>0;
	CDD_RX[4] = (CDD_SEC & 0xff00)>>8;
	CDD_RX[5] = (CDD_SEC & 0x00ff)>>0;
	CDD_RX[6] = (CDD_FRAME   & 0xff00)>>8;
	CDD_RX[7] = (CDD_FRAME   & 0x00ff)>>0;
	CDD_RX[8] = (CDD_EXT     & 0x00ff)>>0;
	/* 9 = checksum */

	CDD_DoChecksum();

	CDD_CONTROL &= ~4; // Clear HOCK bit

}






void lc89510_temp_device::CDD_GetStatus(void)
{
	uint16_t s = (CDD_STATUS & 0x0f00);

	if ((s == 0x0200) || (s == 0x0700) || (s == 0x0e00))
		CDD_STATUS = (SCD_STATUS & 0xff00) | (CDD_STATUS & 0x00ff);
}


void lc89510_temp_device::CDD_Stop()
{
	CLEAR_CDD_RESULT
	STOP_CDC_READ
	SCD_STATUS = CDD_STOPPED;
	CDD_STATUS = 0x0000;
	SET_CDD_DATA_MODE
	m_cdda->stop_audio(); //stop any pending CD-DA

	//neocd
	NeoCD_StatusHack = 0x0E;


}


void lc89510_temp_device::CDD_GetPos(void)
{
	CLEAR_CDD_RESULT
	uint32_t msf;
	CDD_STATUS &= 0xFF;
	if(!m_cdrom->exists()) // no CD is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	msf = cdrom_file::lba_to_msf_alt(SCD_CURLBA+150);
	CDD_MIN = to_bcd(((msf & 0x00ff0000)>>16),false);
	CDD_SEC = to_bcd(((msf & 0x0000ff00)>>8),false);
	CDD_FRAME = to_bcd(((msf & 0x000000ff)>>0),false);
}

void lc89510_temp_device::CDD_GetTrackPos(void)
{
	CLEAR_CDD_RESULT
	int elapsedlba;
	uint32_t msf;
	CDD_STATUS &= 0xFF;
	//  uint32_t end_msf = ;
	if(!m_cdrom->exists()) // no CD is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	elapsedlba = SCD_CURLBA - segacd.toc->tracks[ m_cdrom->get_track(SCD_CURLBA) ].logframeofs;
	msf = cdrom_file::lba_to_msf_alt (elapsedlba);
	//popmessage("%08x %08x",SCD_CURLBA,segacd.toc->tracks[ cdrom_get_track(m_cdrom, SCD_CURLBA) + 1 ].logframeofs);
	CDD_MIN = to_bcd(((msf & 0x00ff0000)>>16),false);
	CDD_SEC = to_bcd(((msf & 0x0000ff00)>>8),false);
	CDD_FRAME = to_bcd(((msf & 0x000000ff)>>0),false);
}

void lc89510_temp_device::CDD_GetTrack(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS &= 0xFF;
	if(!m_cdrom->exists()) // no CD is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	SCD_CURTRK = m_cdrom->get_track(SCD_CURLBA)+1;
	CDD_MIN = to_bcd(SCD_CURTRK, false);
}

void lc89510_temp_device::CDD_Length(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS &= 0xFF;
	if(!m_cdrom->exists()) // no CD is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;

	uint32_t startlba = (segacd.toc->tracks[m_cdrom->get_last_track()].logframeofs);
	uint32_t startmsf = cdrom_file::lba_to_msf_alt( startlba );

	CDD_MIN = to_bcd((startmsf&0x00ff0000)>>16,false);
	CDD_SEC = to_bcd((startmsf&0x0000ff00)>>8,false);
	CDD_FRAME = to_bcd((startmsf&0x000000ff)>>0,false);
}


void lc89510_temp_device::CDD_FirstLast(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS &= 0xFF;
	if(!m_cdrom->exists()) // no CD is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	CDD_MIN = 1; // first
	CDD_SEC = to_bcd(m_cdrom->get_last_track(),false); // last
}

void lc89510_temp_device::CDD_GetTrackAdr(void)
{
	CLEAR_CDD_RESULT

	CDD_STATUS &= 0xFF;
	if(!m_cdrom->exists()) // no CD is there, bail out
		return;

	int track = (CDD_TX[5] & 0xF) + (CDD_TX[4] & 0xF) * 10;
	int last_track = m_cdrom->get_last_track();

	CDD_STATUS |= SCD_STATUS;

	if (track > last_track)
		track = last_track;

	if (track < 1)
		track = 1;

	uint32_t startlba = (segacd.toc->tracks[track-1].logframeofs);
	uint32_t startmsf = cdrom_file::lba_to_msf_alt( startlba+150 );

	CDD_MIN = to_bcd((startmsf&0x00ff0000)>>16,false);
	CDD_SEC = to_bcd((startmsf&0x0000ff00)>>8,false);
	CDD_FRAME = to_bcd((startmsf&0x000000ff)>>0,false);
	CDD_EXT = track % 10;

	if (segacd.toc->tracks[track - 1].trktype != cdrom_file::CD_TRACK_AUDIO)
		CDD_FRAME |= 0x0800;
}

// verify what this is.. the NeoCd emu code only checked the track type but
// checked it where we put CDD_EXT, not CDD_FRAME (RX[7] vs RX[8] in Export)
void lc89510_temp_device::CDD_GetTrackType(void)
{
	CLEAR_CDD_RESULT

	CDD_STATUS &= 0xFF;
	if(!m_cdrom->exists()) // no CD is there, bail out
		return;

	int track = (CDD_TX[5] & 0xF) + (CDD_TX[4] & 0xF) * 10;
	int last_track = m_cdrom->get_last_track();

	CDD_STATUS |= SCD_STATUS;

	if (track > last_track)
		track = last_track;

	if (track < 1)
		track = 1;

	if (segacd.toc->tracks[track - 1].trktype != cdrom_file::CD_TRACK_AUDIO)
	{
		CDD_EXT = 0x08;
		CDD_FRAME |= 0x0800;
	}



}

uint32_t lc89510_temp_device::getmsf_from_regs(void)
{
	uint32_t msf = 0;

	msf  = ((CDD_TX[3] & 0xF) + (CDD_TX[2] & 0xF) * 10) << 16;
	msf |= ((CDD_TX[5] & 0xF) + (CDD_TX[4] & 0xF) * 10) << 8;
	msf |= ((CDD_TX[7] & 0xF) + (CDD_TX[6] & 0xF) * 10) << 0;

	return msf;
}

void lc89510_temp_device::CDD_Play()
{
	CLEAR_CDD_RESULT
	uint32_t msf = getmsf_from_regs();
	SCD_CURLBA = cdrom_file::msf_to_lba(msf)-150;
	if(!m_cdrom->exists()) // no CD is there, bail out
		return;
	// TODO: audio CD player stops after first track
	// should really read the full CD length rather than one track only
	uint32_t track_length = segacd.toc->tracks[ m_cdrom->get_track(SCD_CURLBA) ].logframes;
	SCD_CURTRK = m_cdrom->get_track(SCD_CURLBA)+1;
	LC8951UpdateHeader();
	SCD_STATUS = CDD_PLAYINGCDDA;
	CDD_STATUS = 0x0102;
	set_data_audio_mode();
	LOG("%d Track played\n",SCD_CURTRK);
	CDD_MIN = to_bcd(SCD_CURTRK, false);
	if(!(CURRENT_TRACK_IS_DATA))
		m_cdda->start_audio(SCD_CURLBA, SCD_CURLBA + track_length);
	SET_CDC_READ


	NeoCD_StatusHack = 1;

}


void lc89510_temp_device::CDD_Seek(void)
{
	CLEAR_CDD_RESULT
	uint32_t msf = getmsf_from_regs();
	SCD_CURLBA = cdrom_file::msf_to_lba(msf)-150;
	if(!m_cdrom->exists()) // no CD is there, bail out
		return;
	SCD_CURTRK = m_cdrom->get_track(SCD_CURLBA)+1;
	LC8951UpdateHeader();
	STOP_CDC_READ
	SCD_STATUS = CDD_READY;
	CDD_STATUS = 0x0200;
	set_data_audio_mode();
}


void lc89510_temp_device::CDD_Pause()
{
	CLEAR_CDD_RESULT
	STOP_CDC_READ
	SCD_STATUS = CDD_READY;
	CDD_STATUS = SCD_STATUS;
	SET_CDD_DATA_MODE

	//segacd.current_frame = cdda_get_audio_lba( machine.device( "cdda" ) );
	//if(!(CURRENT_TRACK_IS_DATA))
	m_cdda->pause_audio(1);


	NeoCD_StatusHack = 4;


}

void lc89510_temp_device::CDD_Resume()
{
	CLEAR_CDD_RESULT
	STOP_CDC_READ
	if(!m_cdrom->exists()) // no CD is there, bail out
		return;
	SCD_CURTRK = m_cdrom->get_track(SCD_CURLBA)+1;
	SCD_STATUS = CDD_PLAYINGCDDA;
	CDD_STATUS = 0x0102;
	set_data_audio_mode();
	CDD_MIN = to_bcd (SCD_CURTRK, false);
	SET_CDC_READ
	//if(!(CURRENT_TRACK_IS_DATA))
	m_cdda->pause_audio(0);

	NeoCD_StatusHack = 1;
}


void lc89510_temp_device::CDD_FF()
{
	fatalerror("Fast Forward unsupported\n");
}


void lc89510_temp_device::CDD_RW()
{
	fatalerror("Fast Rewind unsupported\n");
}


void lc89510_temp_device::CDD_Open(void)
{
	fatalerror("Close Tray unsupported\n");
	/* TODO: re-read CD-ROM buffer here (Mega CD has multi disc games iirc?) */
}


void lc89510_temp_device::CDD_Close(void)
{
	fatalerror("Open Tray unsupported\n");
	/* TODO: clear CD-ROM buffer here */
}


void lc89510_temp_device::CDD_Init(void)
{
	CLEAR_CDD_RESULT
	STOP_CDC_READ
	SCD_STATUS = CDD_READY;
	CDD_STATUS = SCD_STATUS;
	CDD_SEC = 1;
	CDD_FRAME = 1;
}


void lc89510_temp_device::CDD_Default(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS = SCD_STATUS;


	NeoCD_StatusHack = 9;
}


void lc89510_temp_device::CDD_Reset(void)
{
	CLEAR_CDD_RESULT
	CDD_CONTROL = CDD_STATUS = 0;

	for (int i = 0; i < 10; i++)
		CDD_RX[i] = CDD_TX[i] = 0;

	CDD_DoChecksum();

	SCD_CURTRK = SCD_CURLBA = 0;
	SCD_STATUS = CDD_READY;
}

void lc89510_temp_device::CDC_Reset(void)
{
	memset(CDC_BUFFER, 0x00, ((16 * 1024 * 2) + SECTOR_SIZE));

	LC8951RegistersW[REG_W_DACL] = LC8951RegistersW[REG_W_DACH] = LC8951RegistersW[REG_W_DBCL] = LC8951RegistersW[REG_W_DBCH] = LC8951RegistersW[REG_W_PTH] = LC8951RegistersW[REG_W_PTL] = LC8951RegistersW[REG_W_SBOUT] = LC8951RegistersW[REG_W_IFCTRL] = LC8951RegistersW[REG_W_CTRL0] = LC8951RegistersW[REG_W_CTRL1] =
		LC8951RegistersW[REG_W_CTRL2] = LC8951RegistersR[REG_R_HEAD1] = LC8951RegistersR[REG_R_HEAD2] = LC8951RegistersR[REG_R_HEAD3] = LC8951RegistersR[REG_R_STAT0] = LC8951RegistersR[REG_R_STAT1] = LC8951RegistersR[REG_R_STAT2] = CDC_DECODE = 0;

	LC8951RegistersR[REG_R_IFSTAT] = 0xFF;
	int wa = SECTOR_SIZE * 2;
	LC8951RegistersW[REG_W_WAL] = wa & 0xff; LC8951RegistersW[REG_W_WAH] = (wa >> 8) &0xff;
	LC8951RegistersR[REG_R_HEAD0] = 0x01;
	LC8951RegistersR[REG_R_STAT3] = 0x80;

	LC8951UpdateHeader();
}


void lc89510_temp_device::lc89510_Reset(void)
{
	CDD_Reset();
	CDC_Reset();

	CDC_REG0 = CDC_REG1 = SCD_STATUS_CDC = CDD_DONE = 0;
}


void lc89510_temp_device::CDC_Do_DMA(int rate)
{
	uint32_t length;

	uint16_t destination = CDC_REG0 & 0x0700;

	if (!(SCD_DMA_ENABLED))
		return;

	if ((destination == READ_MAIN) || (destination==READ_SUB))
	{
		CDC_REG0 |= 0x4000;
		return;
	}

	int dma_count_register = LC8951RegistersW[REG_W_DBCL] | (LC8951RegistersW[REG_W_DBCH]<<8);

	if (dma_count_register <= (rate * 2))
	{
		length = (dma_count_register + 1) >> 1;
		CDC_End_Transfer();
	}
	else
		length = rate;


	int dmacount = length;

	uint16_t dma_addrc = LC8951RegistersW[REG_W_DACL] | (LC8951RegistersW[REG_W_DACH]<<8);

	// HACK
	m_segacd_dma_callback(dmacount, CDC_BUFFER, dma_addrc, destination );


	dma_addrc += length*2;
	LC8951RegistersW[REG_W_DACL] = dma_addrc & 0xff; LC8951RegistersW[REG_W_DACH] = (dma_addrc >> 8) & 0xff;

	if (SCD_DMA_ENABLED)
		dma_count_register -= length*2;
	else
		dma_count_register = 0;

	LC8951RegistersW[REG_W_DBCL] = dma_count_register & 0xff; LC8951RegistersW[REG_W_DBCH] = (dma_count_register>>8) & 0xff;

}




uint16_t lc89510_temp_device::CDC_Host_r(uint16_t type)
{
	uint16_t destination = CDC_REG0 & 0x0700;

	if (SCD_DMA_ENABLED)
	{
		if (destination == type)
		{
			int dma_count_register = LC8951RegistersW[REG_W_DBCL] | (LC8951RegistersW[REG_W_DBCH]<<8);

			dma_count_register -= 2;

			if (dma_count_register <= 0)
			{
				if (type==READ_SUB) dma_count_register = 0;

				CDC_End_Transfer();
			}

			LC8951RegistersW[REG_W_DBCL] = dma_count_register & 0xff; LC8951RegistersW[REG_W_DBCH] = (dma_count_register>>8) & 0xff;

			uint16_t dma_addrc = LC8951RegistersW[REG_W_DACL] | (LC8951RegistersW[REG_W_DACH]<<8);

			uint16_t data = (CDC_BUFFER[dma_addrc]<<8) | CDC_BUFFER[dma_addrc+1];
			dma_addrc += 2;

			LC8951RegistersW[REG_W_DACL] = dma_addrc & 0xff; LC8951RegistersW[REG_W_DACH] = (dma_addrc >> 8) & 0xff;


			return data;
		}
	}

	return 0;
}




uint8_t lc89510_temp_device::CDC_Reg_r(void)
{
	int reg = CDC_REG0 & 0xF;
	uint8_t ret = 0;


	uint16_t decoderegs = 0x73F2;

	if ((decoderegs>>reg)&1)
		CDC_DECODE |= (1 << reg);


	CDC_REG0 = (CDC_REG0 & 0xFFF0) | ((reg+1)&0xf);

	switch (reg)
	{
		case REG_R_COMIN:  ret = 0/*COMIN*/;                        break;
		case REG_R_IFSTAT: ret = LC8951RegistersR[REG_R_IFSTAT];    break;
		case REG_R_DBCL:   ret = LC8951RegistersW[REG_W_DBCL];      break;
		case REG_R_DBCH:
//          LC8951RegistersR[REG_R_DBCH] &=  0x0F; // NeoCD?
//          LC8951RegistersR[REG_R_DBCH] |=  (LC8951RegistersR[REG_R_IFSTAT] & 0x40) ? 0x00 : 0xF0; // NeoCD?
			ret = LC8951RegistersW[REG_W_DBCH];     break;
		case REG_R_HEAD0:  ret = LC8951RegistersR[REG_R_HEAD0];     break;
		case REG_R_HEAD1:  ret = LC8951RegistersR[REG_R_HEAD1];     break;
		case REG_R_HEAD2:  ret = LC8951RegistersR[REG_R_HEAD2];     break;
		case REG_R_HEAD3:  ret = LC8951RegistersR[REG_R_HEAD3];     break;
		case REG_R_PTL:    ret = LC8951RegistersW[REG_W_PTL];       break;
		case REG_R_PTH:    ret = LC8951RegistersW[REG_W_PTH];       break;
		case REG_R_WAL:    ret = LC8951RegistersW[REG_W_WAL];       break;
		case REG_R_WAH:    ret = LC8951RegistersW[REG_W_WAH];       break;
		case REG_R_STAT0:  ret = LC8951RegistersR[REG_R_STAT0];     break;
		case REG_R_STAT1:  ret = LC8951RegistersR[REG_R_STAT1];
	//      LC8951RegistersR[REG_R_IFSTAT] |= 0x20;  // NeoCD? // reset DECI
			break;
		case REG_R_STAT2:  ret = LC8951RegistersR[REG_R_STAT2];     break;
		case REG_R_STAT3:  ret = LC8951RegistersR[REG_R_STAT3];

			LC8951RegistersR[REG_R_IFSTAT] |= 0x20; // SegaCD? // reset DECI
			if (!is_neoCD)
			{
				if ((LC8951RegistersW[REG_W_CTRL0] & 0x80) && (LC8951RegistersW[REG_W_IFCTRL] & 0x20))
				{
					if ((CDC_DECODE & decoderegs) == decoderegs)
					LC8951RegistersR[REG_R_STAT3] = 0x80;
				}
			}
			break;

	}


	return ret;
}

void lc89510_temp_device::CDC_Reg_w(uint8_t data)
{
	int reg = CDC_REG0 & 0xF;

	int changers0[0x10] = { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0};

	if (changers0[reg])
		CDC_REG0 = (CDC_REG0 & 0xFFF0) | (reg+1);

	switch (reg)
	{
	case REG_W_SBOUT:
			LC8951RegistersW[REG_W_SBOUT] = data;
			break;

	case REG_W_IFCTRL:
			LC8951RegistersW[REG_W_IFCTRL] = data;

			if (!(LC8951RegistersW[REG_W_IFCTRL] & 0x02))
			{
				LC8951RegistersW[REG_W_DBCL] = 0;  LC8951RegistersW[REG_W_DBCH] = 0;
				STOP_CDC_DMA;
				LC8951RegistersR[REG_R_IFSTAT] |= 0x08;
			}
			break;

	case REG_W_DBCL: LC8951RegistersW[REG_W_DBCL] = data; break;
	case REG_W_DBCH: LC8951RegistersW[REG_W_DBCH] = data; break;
	case REG_W_DACL: LC8951RegistersW[REG_W_DACL] = data; break;
	case REG_W_DACH: LC8951RegistersW[REG_W_DACH] = data; break;

	case REG_W_DTTRG:
			if (!is_neoCD)
			{
				if (LC8951RegistersW[REG_W_IFCTRL] & 0x02)
				{
					LC8951RegistersR[REG_R_IFSTAT] &= ~0x08;
					SET_CDC_DMA;
					CDC_REG0 &= ~0x8000;
				}
			}
			else
			{
				LC8951RegistersW[REG_W_DTTRG]  = ~0x00;
				LC8951RegistersR[REG_R_IFSTAT] &= ~0x08;
			}
			break;

	case REG_W_DTACK:
			//if (!is_neoCD)
			{
				LC8951RegistersR[REG_R_IFSTAT] |= 0x40;
			}
			//else
			//{
			//  LC8951RegistersW[REG_W_DTACK]  = ~0x00;
			//  LC8951RegistersR[REG_R_IFSTAT] &= ~0x40;
			//}
			break;
	case REG_W_WAL: LC8951RegistersW[REG_W_WAL] = data; break;
	case REG_W_WAH: LC8951RegistersW[REG_W_WAH] = data; break;
	case REG_W_CTRL0: LC8951RegistersW[REG_W_CTRL0] = data; break;
	case REG_W_CTRL1: LC8951RegistersW[REG_W_CTRL1] = data;
		//LC8951UpdateHeader(); // NeoCD
		break;
	case REG_W_PTL: LC8951RegistersW[REG_W_PTL] = data; break;
	case REG_W_PTH: LC8951RegistersW[REG_W_PTH] = data; break;
	case REG_W_CTRL2: LC8951RegistersW[REG_W_CTRL2] = data; break;
	case REG_W_RESET: CDC_Reset();       break;
	}
}



void lc89510_temp_device::CDD_Process(int reason)
{
	CDD_Export();
	CHECK_SCD_LV4_INTERRUPT
}

void lc89510_temp_device::CDD_Handle_TOC_Commands(void)
{
	int subcmd = CDD_TX[3];
	CDD_STATUS = (CDD_STATUS & 0xFF00) | subcmd;

	switch (subcmd)
	{
		case TOCCMD_CURPOS:    CDD_GetPos();      break;
		case TOCCMD_TRKPOS:    CDD_GetTrackPos(); break;
		case TOCCMD_CURTRK:    CDD_GetTrack();   break;
		case TOCCMD_LENGTH:    CDD_Length();      break;
		case TOCCMD_FIRSTLAST: CDD_FirstLast();   break;
		case TOCCMD_TRACKADDR: CDD_GetTrackAdr(); break;
		case 6:                CDD_GetTrackType(); break; // NGCD, might be wrong, make sure Sega CD doesn't hate it
		default:               CDD_GetStatus();   break;
	}
}

static const char *const CDD_import_cmdnames[] =
{
	"Get Status",           // 0
	"Stop ALL",             // 1
	"Handle TOC",           // 2
	"Play",                 // 3
	"Seek",                 // 4
	"<undefined> (5)",          // 5
	"Pause",                // 6
	"Resume",               // 7
	"FF",                   // 8
	"RWD",                  // 9
	"INIT",                 // A
	"<undefined> (b)",          // B
	"Close Tray",           // C
	"Open Tray",            // D
	"<undefined> (e)",          // E
	"<undefined> (f)"           // F
};

bool lc89510_temp_device::CDD_Import()
{
	// don't execute the command if the checksum isn't valid
	if (!CDD_Check_TX_Checksum())
	{
		LOG("invalid checksum\n");
		return false;
	}

	if(CDD_TX[0] != 2 && CDD_TX[0] != 0)
		LOG("%s\n",CDD_import_cmdnames[CDD_TX[0]]);

	switch (CDD_TX[0])
	{
		case CMD_STATUS:    CDD_GetStatus();           break;
		case CMD_STOPALL:   CDD_Stop();                break;
		case CMD_GETTOC:    CDD_Handle_TOC_Commands(); break;
		case CMD_READ:      CDD_Play();                break;
		case CMD_SEEK:      CDD_Seek();                break;
		case CMD_STOP:      CDD_Pause();               break;
		case CMD_RESUME:    CDD_Resume();              break;
		case CMD_FF:        CDD_FF();                  break;
		case CMD_RW:        CDD_RW();                  break;
		case CMD_INIT:      CDD_Init();                break;
		case CMD_CLOSE:     CDD_Open();                break;
		case CMD_OPEN:      CDD_Close();               break;
		default:            CDD_Default();             break;
	}

	CDD_DONE = 1;
	return true;
}


/**************************************************************
 CDC Stuff ********
**************************************************************/



void lc89510_temp_device::segacd_cdc_mode_address_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&CDC_REG0);
}

uint16_t lc89510_temp_device::segacd_cdc_mode_address_r()
{
	return CDC_REG0;
}

void lc89510_temp_device::segacd_cdc_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&CDC_REG1);

	if (ACCESSING_BITS_0_7)
		CDC_Reg_w(data);
}

uint16_t lc89510_temp_device::segacd_cdc_data_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t retdat = 0x0000;

	if (ACCESSING_BITS_0_7)
		retdat |= CDC_Reg_r();

	return retdat;
}


uint16_t lc89510_temp_device::cdc_data_sub_r()
{
	return CDC_Host_r(READ_SUB);
}

uint16_t lc89510_temp_device::cdc_data_main_r()
{
	return CDC_Host_r(READ_MAIN);
}




uint16_t lc89510_temp_device::segacd_irq_mask_r()
{
	return segacd_irq_mask;
}

void lc89510_temp_device::segacd_irq_mask_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		uint16_t control = CDD_CONTROL;

	//  LOG("segacd_irq_mask_w %04x %04x (CDD control is %04x)\n",data, mem_mask, control);

		if (data & 0x10)
		{
			if (control & 0x04)
			{
				if (!(segacd_irq_mask & 0x10))
				{
					segacd_irq_mask = data & 0x7e;
					CDD_Process(0);
					return;
				}
			}
		}

		segacd_irq_mask = data & 0x7e;
	}
	else
	{
		LOG("segacd_irq_mask_w only MSB written\n");

	}
}

uint16_t lc89510_temp_device::segacd_cdd_ctrl_r()
{
	return CDD_CONTROL;
}


void lc89510_temp_device::segacd_cdd_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		uint16_t control = CDD_CONTROL;


		//LOG("segacd_cdd_ctrl_w %04x %04x (control %04x irq %04x\n", data, mem_mask, control, segacd_irq_mask);

		data &=0x4; // only HOCK bit is writable

		if (data & 0x4)
		{
			if (!(control & 0x4))
			{
				if (segacd_irq_mask&0x10)
				{
					CDD_Process(1);
				}
			}
		}

		CDD_CONTROL |= data;
	}
	else
	{
		LOG("segacd_cdd_ctrl_w only MSB written\n");
	}
}


// mapped as serial
uint8_t lc89510_temp_device::neocd_cdd_rx_r()
{
	uint8_t ret = 0;

	if (NeoCDCommsWordCount >= 0 && NeoCDCommsWordCount < 10) {
		ret = CDD_RX[NeoCDCommsWordCount] & 0x0F;
	}

	if (bNeoCDCommsClock) {
		ret |= 0x10;
	}

	return ret;
}

// mapped like 'ram'
uint8_t lc89510_temp_device::segacd_cdd_rx_r(offs_t offset)
{
	return CDD_RX[offset];
}

// mapped as serial
void lc89510_temp_device::neocd_cdd_tx_w(uint8_t data)
{
	//LOG("neocd_cdd_tx_w %d, %02x\n", NeoCDCommsWordCount, data);

	if (NeoCDCommsWordCount >= 0 && NeoCDCommsWordCount < 10) {
		CDD_TX[NeoCDCommsWordCount] = data & 0x0F;
	}
}

void lc89510_temp_device::segacd_cdd_tx_w(offs_t offset, uint8_t data)
{
	CDD_TX[offset] = data;

	if(offset == 9)
	{
		CDD_Import();
	}
}






uint16_t lc89510_temp_device::segacd_cdfader_r()
{
	return 0;
}

void  lc89510_temp_device::segacd_cdfader_w(uint16_t data)
{
	static double cdfader_vol;
	if(data & 0x800f)
		LOG("CD Fader register write %04x\n",data);

	cdfader_vol = (double)((data & 0x3ff0) >> 4);

	if(data & 0x4000)
		cdfader_vol = 1.0;
	else
		cdfader_vol = cdfader_vol / 1024.0;

	//LOG("%f\n",cdfader_vol);

	m_cdda->set_output_gain(ALL_OUTPUTS, cdfader_vol);
}

void lc89510_temp_device::reset_cd(void)
{
	/* init cd-rom device */

	lc89510_Reset();

	{
		if ( m_cdrom->exists() )
		{
			segacd.toc = &m_cdrom->get_toc();
			m_cdda->stop_audio(); //stop any pending CD-DA
		}
	}
}



TIMER_DEVICE_CALLBACK_MEMBER( lc89510_temp_device::segacd_access_timer_callback )
{
	if (!is_neoCD)
	{
		if (CDD_DONE)
		{
			CDD_DONE = 0;
			CDD_Export();
			CHECK_SCD_LV4_INTERRUPT_A
		}
	}
	else
	{
		if (nff0002 & 0x0050)
		{
			m_type2_interrupt_callback();
		}
	}

	if (SCD_READ_ENABLED) // if (nff0002 & 0x0050) if (nff0002 & 0x0500);
	{
		set_data_audio_mode();
		Read_LBA_To_Buffer();
	}

}


void lc89510_temp_device::device_add_mconfig(machine_config &config)
{
	TIMER(config, "hock_timer").configure_periodic(FUNC(lc89510_temp_device::segacd_access_timer_callback), attotime::from_hz(75));

	cdda_device &cdda(CDDA(config, "cdda"));
	cdda.add_route(0, ":speaker", 0.50, 0); // TODO: accurate volume balance
	cdda.add_route(1, ":speaker", 0.50, 1);
	cdda.set_cdrom_tag(m_cdrom);
}



/* Neo CD */


void lc89510_temp_device::NeoCDCommsReset()
{
	bNeoCDCommsClock = true;

	memset(CDD_TX, 0, sizeof(CDD_TX));
	memset(CDD_RX,  0, sizeof(CDD_RX));

	NeoCDCommsWordCount = 0;

	NeoCD_StatusHack = 9;


	nff0016 = 0;
}




void lc89510_temp_device::NeoCDCommsControl(uint8_t clock, uint8_t send)
{
	if (clock && !bNeoCDCommsClock) {
		NeoCDCommsWordCount++;
		if (NeoCDCommsWordCount >= 10) {
			NeoCDCommsWordCount = 0;

			if (send)
			{
				if (CDD_TX[0])
				{
					if (!CDD_Import())
						return;

					CDD_Export(true); // true == neocd hack,
				}

			}

		}
	}
	bNeoCDCommsClock = clock;
}


void lc89510_temp_device::LC8951UpdateHeader() // neocd
{
	if (LC8951RegistersW[REG_W_CTRL1] & 1) {
		// HEAD registers have sub-header

		LC8951RegistersR[REG_R_HEAD0] = 0;                                                  // HEAD0
		LC8951RegistersR[REG_R_HEAD1] = 0;                                                  // HEAD1
		LC8951RegistersR[REG_R_HEAD2] = 0;                                                  // HEAD2
		LC8951RegistersR[REG_R_HEAD3] = 0;                                                  // HEAD3

	} else {
		// HEAD registers have header
		uint32_t msf = cdrom_file::lba_to_msf_alt(SCD_CURLBA+150);

		LC8951RegistersR[REG_R_HEAD0] = to_bcd (((msf & 0x00ff0000)>>16), true);    // HEAD0
		LC8951RegistersR[REG_R_HEAD1] = to_bcd (((msf & 0x0000ff00)>>8), true);     // HEAD1
		LC8951RegistersR[REG_R_HEAD2] = to_bcd (((msf & 0x000000ff)>>0), true);     // HEAD2
		LC8951RegistersR[REG_R_HEAD3] = 0x1;                                        // HEAD3
	}
}

char* lc89510_temp_device::LC8915InitTransfer(int NeoCDDMACount)
{
	if (!LC8951RegistersW[REG_W_DTTRG]) {
		LOGWARN("LC8951 DTTRG status invalid\n");
		return nullptr;
	}
	if (!(LC8951RegistersW[REG_W_IFCTRL] & 0x02)) {
		LOGWARN("LC8951 DOUTEN status invalid\n");
		return nullptr;
	}
	if (((LC8951RegistersW[REG_W_DACH] << 8) | LC8951RegistersW[REG_W_DACL]) + (NeoCDDMACount << 1) > EXTERNAL_BUFFER_SIZE) {
		LOGWARN("DMA transfer exceeds current sector in LC8951 external buffer\n");
		return nullptr;
	}

	char* addr = (char*)CDC_BUFFER + ((LC8951RegistersW[REG_W_DACH] << 8) | LC8951RegistersW[REG_W_DACL]);
	return  addr;
}

void lc89510_temp_device::LC8915EndTransfer()
{
	LC8951RegistersW[REG_W_DTTRG]  = 0x00;                                              // reset DTTRG

	LC8951RegistersR[REG_R_IFSTAT] |= 0x48;                                             //   set DTEI & DTBSY
	if (LC8951RegistersW[REG_W_IFCTRL] & 0x40) {
		// trigger DTE interrupt

		// the Neo Geo CD doesn't use the DTE interrupt
		// nIRQAcknowledge &= ~0x20;
		// NeoCDIRQUpdate(0);

	}
}


void lc89510_temp_device::CDC_End_Transfer()
{
	STOP_CDC_DMA
	CDC_REG0 |= 0x8000;
	CDC_REG0 &= ~0x4000;
	LC8951RegistersR[REG_R_IFSTAT] |= 0x08;

	if (LC8951RegistersW[REG_W_IFCTRL] & 0x40)
	{
		LC8951RegistersR[REG_R_IFSTAT] &= ~0x40;
		CHECK_SCD_LV5_INTERRUPT
	}
}






void lc89510_temp_device::scd_ctrl_checks()
{
	LC8951RegistersR[REG_R_STAT0] = 0x80;

	(LC8951RegistersW[REG_W_CTRL0] & 0x10) ? (LC8951RegistersR[REG_R_STAT2] = LC8951RegistersW[REG_W_CTRL1] & 0x08) : (LC8951RegistersR[REG_R_STAT2] = LC8951RegistersW[REG_W_CTRL1] & 0x0C);
	(LC8951RegistersW[REG_W_CTRL0] & 0x02) ? (LC8951RegistersR[REG_R_STAT3] = 0x20) : (LC8951RegistersR[REG_R_STAT3] = 0x00);

	if (LC8951RegistersW[REG_W_IFCTRL] & 0x20)
	{
		if (is_neoCD)
		{
			m_type1_interrupt_callback();
		}
		else
		{
			// todo: make callback
			CHECK_SCD_LV5_INTERRUPT
		}



		LC8951RegistersR[REG_R_IFSTAT] &= ~0x20;
		CDC_DECODE = 0;
	}
}

void lc89510_temp_device::scd_advance_current_readpos(void)
{
	SCD_CURLBA++;

	int pt = LC8951RegistersW[REG_W_PTL] | (LC8951RegistersW[REG_W_PTH] << 8);
	int wa = LC8951RegistersW[REG_W_WAL] | (LC8951RegistersW[REG_W_WAH] << 8);

	wa += SECTOR_SIZE;
	pt += SECTOR_SIZE;

	wa &= 0x7fff;
	pt &= 0x7fff;

	LC8951RegistersW[REG_W_PTL] = pt & 0xff; LC8951RegistersW[REG_W_PTH] = (pt >> 8) &0xff;
	LC8951RegistersW[REG_W_WAL] = wa & 0xff; LC8951RegistersW[REG_W_WAH] = (wa >> 8) &0xff;

}

int lc89510_temp_device::Read_LBA_To_Buffer()
{
	bool data_track = false;
	if (CDD_CONTROL & 0x0100) data_track = true;

	if (data_track)
		m_cdrom->read_data(SCD_CURLBA, SCD_BUFFER, cdrom_file::CD_TRACK_MODE1);

	LC8951UpdateHeader();

	if (!data_track)
	{
		scd_advance_current_readpos();
	}

	if (LC8951RegistersW[REG_W_CTRL0] & 0x80)
	{
		if (LC8951RegistersW[REG_W_CTRL0] & 0x04)
		{
			if (data_track)
			{
				scd_advance_current_readpos();

				int pt = LC8951RegistersW[REG_W_PTL] | (LC8951RegistersW[REG_W_PTH] << 8);

				memcpy(&CDC_BUFFER[pt + 4], SCD_BUFFER, 2048);
				CDC_BUFFER[pt+0] = LC8951RegistersR[REG_R_HEAD0];
				CDC_BUFFER[pt+1] = LC8951RegistersR[REG_R_HEAD1];
				CDC_BUFFER[pt+2] = LC8951RegistersR[REG_R_HEAD2];
				CDC_BUFFER[pt+3] = LC8951RegistersR[REG_R_HEAD3];


				if (is_neoCD)
				{
					// This simulates the protection used by the NeoCDZ, a number of games (samsrpg for example)
					// will not be recognized unless this happens.  Is this part of the CDC error correction
					// mechanism?
					char *buffer_hack = (char*)&CDC_BUFFER[pt];
					if (buffer_hack[4 + 64] == 'g' && !strncmp(buffer_hack + 4, "Copyright by SNK", 16))
					{
						buffer_hack[4 + 64] = 'f';
					}
				}

			}
			else
			{
				int pt = LC8951RegistersW[REG_W_PTL] | (LC8951RegistersW[REG_W_PTH] << 8);

				memcpy(&CDC_BUFFER[pt], SCD_BUFFER, SECTOR_SIZE);
			}
		}

		scd_ctrl_checks();


	}


	return 0;
}






void lc89510_temp_device::nff0002_set(uint16_t wordValue)
{
	nff0002 = wordValue;
}

void lc89510_temp_device::nff0016_set(uint16_t wordValue)
{
	nff0016 = wordValue;
}

uint16_t lc89510_temp_device::nff0016_r(void) { return nff0016; }
