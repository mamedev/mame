// license:BSD-3-Clause
// copyright-holders:David Haywood
/* CD controller code from megacd.c, used by Sega CD / Mega CD */

/* todo: cleanup(!!), make more generic, unifiy implementation with NeoCD, turn into a device and move to the proper lc89510.c file
  currently this is a bit of a mix of system specific bits, incomplete implementations etc. as well as a rather kludgy combination
  of the CDC and drive emulation. */


#include "emu.h"
#include "machine/megacdcd.h"

const device_type LC89510_TEMP = &device_creator<lc89510_temp_device>;

lc89510_temp_device::lc89510_temp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, LC89510_TEMP, "lc89510_temp_device", tag, owner, clock, "lc89510_temp", __FILE__)
{
	segacd_dma_callback =  segacd_dma_delegate(FUNC(lc89510_temp_device::Fake_CDC_Do_DMA), this);
	type1_interrupt_callback =  interrupt_delegate(FUNC(lc89510_temp_device::dummy_interrupt_callback), this);
	type2_interrupt_callback =  interrupt_delegate(FUNC(lc89510_temp_device::dummy_interrupt_callback), this);
	type3_interrupt_callback =  interrupt_delegate(FUNC(lc89510_temp_device::dummy_interrupt_callback), this);

	is_neoCD = false;

	nff0002 = 0;
	for (int i=0;i<10;i++)
		CDD_TX[i] = 0;
	for (int i=0;i<10;i++)
		CDD_RX[i] = 0;
	NeoCDCommsWordCount = 0;
	NeoCD_StatusHack = 0;
	SCD_CURLBA = 0;

	CDC_REG0 = 0;

	segacd_irq_mask = 0;
}

void lc89510_temp_device::dummy_interrupt_callback(void)
{
}

void lc89510_temp_device::set_CDC_Do_DMA(device_t &device,segacd_dma_delegate new_segacd_dma_callback)
{
	lc89510_temp_device &dev = downcast<lc89510_temp_device &>(device);
	dev.segacd_dma_callback = new_segacd_dma_callback;
}

void lc89510_temp_device::set_type1_interrupt_callback(device_t &device,interrupt_delegate interrupt_callback)
{
	lc89510_temp_device &dev = downcast<lc89510_temp_device &>(device);
	dev.type1_interrupt_callback = interrupt_callback;
}

void lc89510_temp_device::set_type2_interrupt_callback(device_t &device,interrupt_delegate interrupt_callback)
{
	lc89510_temp_device &dev = downcast<lc89510_temp_device &>(device);
	dev.type2_interrupt_callback = interrupt_callback;
}

void lc89510_temp_device::set_type3_interrupt_callback(device_t &device,interrupt_delegate interrupt_callback)
{
	lc89510_temp_device &dev = downcast<lc89510_temp_device &>(device);
	dev.type3_interrupt_callback = interrupt_callback;
}

void lc89510_temp_device::set_is_neoCD(device_t &device, bool is_neoCD)
{
	lc89510_temp_device &dev = downcast<lc89510_temp_device &>(device);
	dev.is_neoCD = is_neoCD;
}

// HACK for DMA handling, this gets replaced
void lc89510_temp_device::Fake_CDC_Do_DMA(int &dmacount, UINT8 *CDC_BUFFER, UINT16 &dma_addrc, UINT16 &destination )
{
	fatalerror("Fake_CDC_Do_DMA\n");
}

void lc89510_temp_device::device_start()
{
	segacd_dma_callback.bind_relative_to(*owner());
	type1_interrupt_callback.bind_relative_to(*owner());
	type2_interrupt_callback.bind_relative_to(*owner());
	type3_interrupt_callback.bind_relative_to(*owner());

	m_cdda = (cdda_device*)subdevice("cdda");
}

void lc89510_temp_device::device_reset()
{
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
	//  printf("was %02x returning %02x\n", (CDD_STATUS  & 0xff00)>>8, NeoCD_StatusHack);
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
	UINT16 s = (CDD_STATUS & 0x0f00);

	if ((s == 0x0200) || (s == 0x0700) || (s == 0x0e00))
		CDD_STATUS = (SCD_STATUS & 0xff00) | (CDD_STATUS & 0x00ff);
}


void lc89510_temp_device::CDD_Stop(running_machine &machine)
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
	UINT32 msf;
	CDD_STATUS &= 0xFF;
	if(segacd.cd == nullptr) // no CD is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	msf = lba_to_msf_alt(SCD_CURLBA+150);
	CDD_MIN = to_bcd(((msf & 0x00ff0000)>>16),false);
	CDD_SEC = to_bcd(((msf & 0x0000ff00)>>8),false);
	CDD_FRAME = to_bcd(((msf & 0x000000ff)>>0),false);
}

void lc89510_temp_device::CDD_GetTrackPos(void)
{
	CLEAR_CDD_RESULT
	int elapsedlba;
	UINT32 msf;
	CDD_STATUS &= 0xFF;
	//  UINT32 end_msf = ;
	if(segacd.cd == nullptr) // no CD is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	elapsedlba = SCD_CURLBA - segacd.toc->tracks[ cdrom_get_track(segacd.cd, SCD_CURLBA) ].logframeofs;
	msf = lba_to_msf_alt (elapsedlba);
	//popmessage("%08x %08x",SCD_CURLBA,segacd.toc->tracks[ cdrom_get_track(segacd.cd, SCD_CURLBA) + 1 ].logframeofs);
	CDD_MIN = to_bcd(((msf & 0x00ff0000)>>16),false);
	CDD_SEC = to_bcd(((msf & 0x0000ff00)>>8),false);
	CDD_FRAME = to_bcd(((msf & 0x000000ff)>>0),false);
}

void lc89510_temp_device::CDD_GetTrack(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS &= 0xFF;
	if(segacd.cd == nullptr) // no CD is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	SCD_CURTRK = cdrom_get_track(segacd.cd, SCD_CURLBA)+1;
	CDD_MIN = to_bcd(SCD_CURTRK, false);
}

void lc89510_temp_device::CDD_Length(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS &= 0xFF;
	if(segacd.cd == nullptr) // no CD is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;

	UINT32 startlba = (segacd.toc->tracks[cdrom_get_last_track(segacd.cd)].logframeofs);
	UINT32 startmsf = lba_to_msf_alt( startlba );

	CDD_MIN = to_bcd((startmsf&0x00ff0000)>>16,false);
	CDD_SEC = to_bcd((startmsf&0x0000ff00)>>8,false);
	CDD_FRAME = to_bcd((startmsf&0x000000ff)>>0,false);
}


void lc89510_temp_device::CDD_FirstLast(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS &= 0xFF;
	if(segacd.cd == nullptr) // no CD is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	CDD_MIN = 1; // first
	CDD_SEC = to_bcd(cdrom_get_last_track(segacd.cd),false); // last
}

void lc89510_temp_device::CDD_GetTrackAdr(void)
{
	CLEAR_CDD_RESULT

	int track = (CDD_TX[5] & 0xF) + (CDD_TX[4] & 0xF) * 10;
	int last_track = cdrom_get_last_track(segacd.cd);

	CDD_STATUS &= 0xFF;
	if(segacd.cd == nullptr) // no CD is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;

	if (track > last_track)
		track = last_track;

	if (track < 1)
		track = 1;

	UINT32 startlba = (segacd.toc->tracks[track-1].logframeofs);
	UINT32 startmsf = lba_to_msf_alt( startlba+150 );

	CDD_MIN = to_bcd((startmsf&0x00ff0000)>>16,false);
	CDD_SEC = to_bcd((startmsf&0x0000ff00)>>8,false);
	CDD_FRAME = to_bcd((startmsf&0x000000ff)>>0,false);
	CDD_EXT = track % 10;

	if (segacd.toc->tracks[track - 1].trktype != CD_TRACK_AUDIO)
		CDD_FRAME |= 0x0800;
}

// verify what this is.. the NeoCd emu code only checked the track type but
// checked it where we put CDD_EXT, not CDD_FRAME (RX[7] vs RX[8] in Export)
void lc89510_temp_device::CDD_GetTrackType(void)
{
	CLEAR_CDD_RESULT

	int track = (CDD_TX[5] & 0xF) + (CDD_TX[4] & 0xF) * 10;
	int last_track = cdrom_get_last_track(segacd.cd);

	CDD_STATUS &= 0xFF;
	if(segacd.cd == nullptr) // no CD is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;

	if (track > last_track)
		track = last_track;

	if (track < 1)
		track = 1;

	if (segacd.toc->tracks[track - 1].trktype != CD_TRACK_AUDIO)
	{
		CDD_EXT = 0x08;
		CDD_FRAME |= 0x0800;
	}



}

UINT32 lc89510_temp_device::getmsf_from_regs(void)
{
	UINT32 msf = 0;

	msf  = ((CDD_TX[3] & 0xF) + (CDD_TX[2] & 0xF) * 10) << 16;
	msf |= ((CDD_TX[5] & 0xF) + (CDD_TX[4] & 0xF) * 10) << 8;
	msf |= ((CDD_TX[7] & 0xF) + (CDD_TX[6] & 0xF) * 10) << 0;

	return msf;
}

void lc89510_temp_device::CDD_Play(running_machine &machine)
{
	CLEAR_CDD_RESULT
	UINT32 msf = getmsf_from_regs();
	SCD_CURLBA = msf_to_lba(msf)-150;
	if(segacd.cd == nullptr) // no CD is there, bail out
		return;
	UINT32 end_msf = segacd.toc->tracks[ cdrom_get_track(segacd.cd, SCD_CURLBA) + 1 ].logframeofs;
	SCD_CURTRK = cdrom_get_track(segacd.cd, SCD_CURLBA)+1;
	LC8951UpdateHeader();
	SCD_STATUS = CDD_PLAYINGCDDA;
	CDD_STATUS = 0x0102;
	set_data_audio_mode();
	printf("%d Track played\n",SCD_CURTRK);
	CDD_MIN = to_bcd(SCD_CURTRK, false);
	if(!(CURRENT_TRACK_IS_DATA))
		m_cdda->start_audio(SCD_CURLBA, end_msf - SCD_CURLBA);
	SET_CDC_READ


	NeoCD_StatusHack = 1;

}


void lc89510_temp_device::CDD_Seek(void)
{
	CLEAR_CDD_RESULT
	UINT32 msf = getmsf_from_regs();
	SCD_CURLBA = msf_to_lba(msf)-150;
	if(segacd.cd == nullptr) // no CD is there, bail out
		return;
	SCD_CURTRK = cdrom_get_track(segacd.cd, SCD_CURLBA)+1;
	LC8951UpdateHeader();
	STOP_CDC_READ
	SCD_STATUS = CDD_READY;
	CDD_STATUS = 0x0200;
	set_data_audio_mode();
}


void lc89510_temp_device::CDD_Pause(running_machine &machine)
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

void lc89510_temp_device::CDD_Resume(running_machine &machine)
{
	CLEAR_CDD_RESULT
	STOP_CDC_READ
	if(segacd.cd == nullptr) // no CD is there, bail out
		return;
	SCD_CURTRK = cdrom_get_track(segacd.cd, SCD_CURLBA)+1;
	SCD_STATUS = CDD_PLAYINGCDDA;
	CDD_STATUS = 0x0102;
	set_data_audio_mode();
	CDD_MIN = to_bcd (SCD_CURTRK, false);
	SET_CDC_READ
	//if(!(CURRENT_TRACK_IS_DATA))
	m_cdda->pause_audio(0);

	NeoCD_StatusHack = 1;
}


void lc89510_temp_device::CDD_FF(running_machine &machine)
{
	fatalerror("Fast Forward unsupported\n");
}


void lc89510_temp_device::CDD_RW(running_machine &machine)
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


void lc89510_temp_device::CDC_Do_DMA(running_machine& machine, int rate)
{
	UINT32 length;

	UINT16 destination = CDC_REG0 & 0x0700;

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
		CDC_End_Transfer(machine);
	}
	else
		length = rate;


	int dmacount = length;

	UINT16 dma_addrc = LC8951RegistersW[REG_W_DACL] | (LC8951RegistersW[REG_W_DACH]<<8);

	// HACK
	segacd_dma_callback(dmacount, CDC_BUFFER, dma_addrc, destination );


	dma_addrc += length*2;
	LC8951RegistersW[REG_W_DACL] = dma_addrc & 0xff; LC8951RegistersW[REG_W_DACH] = (dma_addrc >> 8) & 0xff;

	if (SCD_DMA_ENABLED)
		dma_count_register -= length*2;
	else
		dma_count_register = 0;

	LC8951RegistersW[REG_W_DBCL] = dma_count_register & 0xff; LC8951RegistersW[REG_W_DBCH] = (dma_count_register>>8) & 0xff;

}




UINT16 lc89510_temp_device::CDC_Host_r(running_machine& machine, UINT16 type)
{
	UINT16 destination = CDC_REG0 & 0x0700;

	if (SCD_DMA_ENABLED)
	{
		if (destination == type)
		{
			int dma_count_register = LC8951RegistersW[REG_W_DBCL] | (LC8951RegistersW[REG_W_DBCH]<<8);

			dma_count_register -= 2;

			if (dma_count_register <= 0)
			{
				if (type==READ_SUB) dma_count_register = 0;

				CDC_End_Transfer(machine);
			}

			LC8951RegistersW[REG_W_DBCL] = dma_count_register & 0xff; LC8951RegistersW[REG_W_DBCH] = (dma_count_register>>8) & 0xff;

			UINT16 dma_addrc = LC8951RegistersW[REG_W_DACL] | (LC8951RegistersW[REG_W_DACH]<<8);

			UINT16 data = (CDC_BUFFER[dma_addrc]<<8) | CDC_BUFFER[dma_addrc+1];
			dma_addrc += 2;

			LC8951RegistersW[REG_W_DACL] = dma_addrc & 0xff; LC8951RegistersW[REG_W_DACH] = (dma_addrc >> 8) & 0xff;


			return data;
		}
	}

	return 0;
}




UINT8 lc89510_temp_device::CDC_Reg_r(void)
{
	int reg = CDC_REG0 & 0xF;
	UINT8 ret = 0;


	UINT16 decoderegs = 0x73F2;

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

void lc89510_temp_device::CDC_Reg_w(UINT8 data)
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



void lc89510_temp_device::CDD_Process(running_machine& machine, int reason)
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

bool lc89510_temp_device::CDD_Import(running_machine& machine)
{
	// don't execute the command if the checksum isn't valid
	if (!CDD_Check_TX_Checksum())
	{
		printf("invalid checksum\n");
		return false;
	}

	if(CDD_TX[0] != 2 && CDD_TX[0] != 0)
		printf("%s\n",CDD_import_cmdnames[CDD_TX[0]]);

	switch (CDD_TX[0])
	{
		case CMD_STATUS:    CDD_GetStatus();           break;
		case CMD_STOPALL:   CDD_Stop(machine);         break;
		case CMD_GETTOC:    CDD_Handle_TOC_Commands(); break;
		case CMD_READ:      CDD_Play(machine);         break;
		case CMD_SEEK:      CDD_Seek();                break;
		case CMD_STOP:      CDD_Pause(machine);        break;
		case CMD_RESUME:    CDD_Resume(machine);       break;
		case CMD_FF:        CDD_FF(machine);           break;
		case CMD_RW:        CDD_RW(machine);           break;
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



WRITE16_MEMBER( lc89510_temp_device::segacd_cdc_mode_address_w )
{
	COMBINE_DATA(&CDC_REG0);
}

READ16_MEMBER( lc89510_temp_device::segacd_cdc_mode_address_r )
{
	return CDC_REG0;
}

WRITE16_MEMBER( lc89510_temp_device::segacd_cdc_data_w )
{
	COMBINE_DATA(&CDC_REG1);

	if (ACCESSING_BITS_0_7)
		CDC_Reg_w(data);
}

READ16_MEMBER( lc89510_temp_device::segacd_cdc_data_r )
{
	UINT16 retdat = 0x0000;

	if (ACCESSING_BITS_0_7)
		retdat |= CDC_Reg_r();

	return retdat;
}


READ16_MEMBER( lc89510_temp_device::cdc_data_sub_r )
{
	return CDC_Host_r(space.machine(), READ_SUB);
}

READ16_MEMBER( lc89510_temp_device::cdc_data_main_r )
{
	return CDC_Host_r(space.machine(), READ_MAIN);
}




READ16_MEMBER( lc89510_temp_device::segacd_irq_mask_r )
{
	return segacd_irq_mask;
}

WRITE16_MEMBER( lc89510_temp_device::segacd_irq_mask_w )
{
	if (ACCESSING_BITS_0_7)
	{
		UINT16 control = CDD_CONTROL;

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

READ16_MEMBER( lc89510_temp_device::segacd_cdd_ctrl_r )
{
	return CDD_CONTROL;
}


WRITE16_MEMBER( lc89510_temp_device::segacd_cdd_ctrl_w )
{
	if (ACCESSING_BITS_0_7)
	{
		UINT16 control = CDD_CONTROL;


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


// mapped as serial
UINT8 lc89510_temp_device::neocd_cdd_rx_r()
{
	UINT8 ret = 0;

	if (NeoCDCommsWordCount >= 0 && NeoCDCommsWordCount < 10) {
		ret = CDD_RX[NeoCDCommsWordCount] & 0x0F;
	}

	if (bNeoCDCommsClock) {
		ret |= 0x10;
	}

	return ret;
}

// mapped like 'ram'
READ8_MEMBER( lc89510_temp_device::segacd_cdd_rx_r )
{
	return CDD_RX[offset];
}

// mapped as serial
void lc89510_temp_device::neocd_cdd_tx_w(UINT8 data)
{
	//printf("neocd_cdd_tx_w %d, %02x\n", NeoCDCommsWordCount, data);

	if (NeoCDCommsWordCount >= 0 && NeoCDCommsWordCount < 10) {
		CDD_TX[NeoCDCommsWordCount] = data & 0x0F;
	}
}

WRITE8_MEMBER( lc89510_temp_device::segacd_cdd_tx_w )
{
	CDD_TX[offset] = data;

	if(offset == 9)
	{
		CDD_Import(space.machine());
	}
}






READ16_MEMBER( lc89510_temp_device::segacd_cdfader_r )
{
	return 0;
}

WRITE16_MEMBER( lc89510_temp_device::segacd_cdfader_w )
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

	m_cdda->set_volume(cdfader_vol);
}

void lc89510_temp_device::reset_cd(void)
{
	/* init cd-rom device */

	lc89510_Reset();

	{
		cdrom_image_device *cddevice = machine().device<cdrom_image_device>("cdrom");
		if ( cddevice )
		{
			segacd.cd = cddevice->get_cdrom_file();
			if ( segacd.cd )
			{
				segacd.toc = cdrom_get_toc( segacd.cd );
				m_cdda->set_cdrom(segacd.cd);
				m_cdda->stop_audio(); //stop any pending CD-DA
			}
		}
	}



//  if (segacd.cd)
//      printf("cd found\n");
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
			type2_interrupt_callback();
		}
	}

	if (SCD_READ_ENABLED) // if (nff0002 & 0x0050) if (nff0002 & 0x0500);
	{
		set_data_audio_mode();
		Read_LBA_To_Buffer(machine());
	}

}




static MACHINE_CONFIG_FRAGMENT( lc89510_temp_fragment )
	MCFG_TIMER_DRIVER_ADD_PERIODIC("hock_timer", lc89510_temp_device, segacd_access_timer_callback, attotime::from_hz(75))

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, ":lspeaker", 0.50 ) // TODO: accurate volume balance
	MCFG_SOUND_ROUTE( 1, ":rspeaker", 0.50 )
MACHINE_CONFIG_END

machine_config_constructor lc89510_temp_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( lc89510_temp_fragment );
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




void lc89510_temp_device::NeoCDCommsControl(UINT8 clock, UINT8 send)
{
	if (clock && !bNeoCDCommsClock) {
		NeoCDCommsWordCount++;
		if (NeoCDCommsWordCount >= 10) {
			NeoCDCommsWordCount = 0;

			if (send)
			{
				if (CDD_TX[0])
				{
					if (!CDD_Import(machine()))
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
		UINT32 msf = lba_to_msf_alt(SCD_CURLBA+150);

		LC8951RegistersR[REG_R_HEAD0] = to_bcd (((msf & 0x00ff0000)>>16), true);    // HEAD0
		LC8951RegistersR[REG_R_HEAD1] = to_bcd (((msf & 0x0000ff00)>>8), true);     // HEAD1
		LC8951RegistersR[REG_R_HEAD2] = to_bcd (((msf & 0x000000ff)>>0), true);     // HEAD2
		LC8951RegistersR[REG_R_HEAD3] = 0x1;                                        // HEAD3
	}
}

char* lc89510_temp_device::LC8915InitTransfer(int NeoCDDMACount)
{
	if (!LC8951RegistersW[REG_W_DTTRG]) {
		//bprintf(PRINT_ERROR, _T("    LC8951 DTTRG status invalid\n"));
		return nullptr;
	}
	if (!(LC8951RegistersW[REG_W_IFCTRL] & 0x02)) {
		//bprintf(PRINT_ERROR, _T("    LC8951 DOUTEN status invalid\n"));
		return nullptr;
	}
	if (((LC8951RegistersW[REG_W_DACH] << 8) | LC8951RegistersW[REG_W_DACL]) + (NeoCDDMACount << 1) > LC89510_EXTERNAL_BUFFER_SIZE) {
		//bprintf(PRINT_ERROR, _T("    DMA transfer exceeds current sector in LC8951 external buffer\n"));

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


void lc89510_temp_device::CDC_End_Transfer(running_machine& machine)
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






void lc89510_temp_device::scd_ctrl_checks(running_machine& machine)
{
	LC8951RegistersR[REG_R_STAT0] = 0x80;

	(LC8951RegistersW[REG_W_CTRL0] & 0x10) ? (LC8951RegistersR[REG_R_STAT2] = LC8951RegistersW[REG_W_CTRL1] & 0x08) : (LC8951RegistersR[REG_R_STAT2] = LC8951RegistersW[REG_W_CTRL1] & 0x0C);
	(LC8951RegistersW[REG_W_CTRL0] & 0x02) ? (LC8951RegistersR[REG_R_STAT3] = 0x20) : (LC8951RegistersR[REG_R_STAT3] = 0x00);

	if (LC8951RegistersW[REG_W_IFCTRL] & 0x20)
	{
		if (is_neoCD)
		{
			type1_interrupt_callback();
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

int lc89510_temp_device::Read_LBA_To_Buffer(running_machine& machine)
{
	bool data_track = false;
	if (CDD_CONTROL & 0x0100) data_track = true;

	if (data_track)
		cdrom_read_data(segacd.cd, SCD_CURLBA, SCD_BUFFER, CD_TRACK_MODE1);

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

		scd_ctrl_checks(machine);


	}


	return 0;
}






void lc89510_temp_device::nff0002_set(UINT16 wordValue)
{
	nff0002 = wordValue;
}

void lc89510_temp_device::nff0016_set(UINT16 wordValue)
{
	nff0016 = wordValue;
}

UINT16 lc89510_temp_device::nff0016_r(void) { return nff0016; }
