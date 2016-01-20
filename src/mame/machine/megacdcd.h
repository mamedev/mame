// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "sound/cdda.h"
#include "imagedev/chd_cd.h"


typedef device_delegate<void (int&, UINT8*, UINT16&, UINT16&)> segacd_dma_delegate;

typedef device_delegate<void (void)> interrupt_delegate;


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




#define SECTOR_SIZE (2352)

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
		machine.device(":segacd:segacd_68k")->execute().set_input_line(5, HOLD_LINE); \
	}
#define CHECK_SCD_LV4_INTERRUPT \
	if (segacd_irq_mask & 0x10) \
	{ \
		machine.device(":segacd:segacd_68k")->execute().set_input_line(4, HOLD_LINE); \
	}
#define CHECK_SCD_LV4_INTERRUPT_A \
	if (segacd_irq_mask & 0x10) \
	{ \
		machine().device(":segacd:segacd_68k")->execute().set_input_line(4, HOLD_LINE); \
	}


#define CURRENT_TRACK_IS_DATA \
	(segacd.toc->tracks[SCD_CURTRK - 1].trktype != CD_TRACK_AUDIO)

#define CDD_PLAYINGCDDA 0x0100
#define CDD_READY       0x0400
#define CDD_STOPPED     0x0900


#define MCFG_SEGACD_HACK_SET_CDC_DO_DMA( _class, _method) \
	lc89510_temp_device::set_CDC_Do_DMA(*device, segacd_dma_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));
#define MCFG_SEGACD_HACK_SET_NEOCD \
	lc89510_temp_device::set_is_neoCD(*device, true);
#define MCFG_SET_TYPE1_INTERRUPT_CALLBACK( _class, _method) \
	lc89510_temp_device::set_type1_interrupt_callback(*device, interrupt_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));
#define MCFG_SET_TYPE2_INTERRUPT_CALLBACK( _class, _method) \
	lc89510_temp_device::set_type2_interrupt_callback(*device, interrupt_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));
#define MCFG_SET_TYPE3_INTERRUPT_CALLBACK( _class, _method) \
	lc89510_temp_device::set_type3_interrupt_callback(*device, interrupt_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));
/* neocd */

#define CD_FRAMES_MINUTE (60 * 75)
#define CD_FRAMES_SECOND (     75)
#define CD_FRAMES_PREGAP ( 2 * 75)

#define SEK_IRQSTATUS_NONE (0x0000)
#define SEK_IRQSTATUS_AUTO (0x2000)
#define SEK_IRQSTATUS_ACK  (0x1000)

#define LC89510_EXTERNAL_BUFFER_SIZE ((32 * 1024 * 2) + SECTOR_SIZE)

class lc89510_temp_device : public device_t
{
public:
	lc89510_temp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// HACK for DMA handling
	segacd_dma_delegate segacd_dma_callback;
	interrupt_delegate type1_interrupt_callback;
	interrupt_delegate type2_interrupt_callback;
	interrupt_delegate type3_interrupt_callback;

	void Fake_CDC_Do_DMA(int &dmacount, UINT8 *CDC_BUFFER, UINT16 &dma_addrc, UINT16 &destination );
	static void set_CDC_Do_DMA(device_t &device,segacd_dma_delegate new_segacd_dma_callback);

	void dummy_interrupt_callback(void);
	static void set_type1_interrupt_callback(device_t &device,interrupt_delegate interrupt_callback);
	static void set_type2_interrupt_callback(device_t &device,interrupt_delegate interrupt_callback);
	static void set_type3_interrupt_callback(device_t &device,interrupt_delegate interrupt_callback);


	static void set_is_neoCD(device_t &device, bool is_neoCD);


	// HACK for neoCD handling
	bool is_neoCD;


	struct segacd_t
	{
		cdrom_file  *cd;
		const cdrom_toc   *toc;
		UINT32 current_frame;
	};


	segacd_t segacd;

	UINT8    SCD_BUFFER[2560];

	UINT32   SCD_STATUS;
	UINT32   SCD_STATUS_CDC;
	INT32    SCD_CURLBA;
	UINT8    SCD_CURTRK;


	UINT16 CDC_DECODE;
	UINT16 CDC_REG0;
	UINT16 CDC_REG1;

	UINT8 CDC_BUFFER[LC89510_EXTERNAL_BUFFER_SIZE];


	UINT8 CDD_RX[10];
	UINT8 CDD_TX[10];

	UINT32 CDD_STATUS;
	UINT32 CDD_MIN;
	UINT32 CDD_SEC;
	UINT32 CDD_FRAME;
	UINT32 CDD_EXT;

	UINT16 CDD_CONTROL;

	INT16  CDD_DONE;

	inline int to_bcd(int val, bool byte);
	void set_data_audio_mode(void);
	void CDD_DoChecksum(void);
	bool CDD_Check_TX_Checksum(void);
	void CDD_Export(bool neocd_hack = false);
	void scd_ctrl_checks(running_machine& machine);
	void scd_advance_current_readpos(void);
	int Read_LBA_To_Buffer(running_machine& machine);
	void CDD_GetStatus(void);
	void CDD_Stop(running_machine &machine);
	void CDD_GetPos(void);
	void CDD_GetTrackPos(void);
	void CDD_GetTrack(void);
	void CDD_Length(void);
	void CDD_FirstLast(void);
	void CDD_GetTrackAdr(void);
	void CDD_GetTrackType(void);
	UINT32 getmsf_from_regs(void);
	void CDD_Play(running_machine &machine);
	void CDD_Seek(void);
	void CDD_Pause(running_machine &machine);
	void CDD_Resume(running_machine &machine);
	void CDD_FF(running_machine &machine);
	void CDD_RW(running_machine &machine);
	void CDD_Open(void);
	void CDD_Close(void);
	void CDD_Init(void);
	void CDD_Default(void);
	void CDD_Reset(void);
	void CDC_Reset(void);
	void lc89510_Reset(void);
	void CDC_End_Transfer(running_machine& machine);
	void CDC_Do_DMA(running_machine& machine, int rate);
	UINT16 CDC_Host_r(running_machine& machine, UINT16 type);
	UINT8 CDC_Reg_r(void);
	void CDC_Reg_w(UINT8 data);
	void CDD_Process(running_machine& machine, int reason);
	void CDD_Handle_TOC_Commands(void);
	bool CDD_Import(running_machine& machine);
	READ16_MEMBER( segacd_irq_mask_r );
	WRITE16_MEMBER( segacd_irq_mask_w );
	READ16_MEMBER( segacd_cdd_ctrl_r );
	WRITE16_MEMBER( segacd_cdd_ctrl_w );
	READ8_MEMBER( segacd_cdd_rx_r );
	WRITE8_MEMBER( segacd_cdd_tx_w );
	READ16_MEMBER( segacd_cdfader_r );
	WRITE16_MEMBER( segacd_cdfader_w );

	void reset_cd(void);

	WRITE16_MEMBER( segacd_cdc_mode_address_w );
	READ16_MEMBER( segacd_cdc_mode_address_r );
	WRITE16_MEMBER( segacd_cdc_data_w );
	READ16_MEMBER( segacd_cdc_data_r );
	READ16_MEMBER( cdc_data_sub_r );
	READ16_MEMBER( cdc_data_main_r );

	TIMER_DEVICE_CALLBACK_MEMBER( segacd_access_timer_callback );

	UINT16 get_segacd_irq_mask(void) { return segacd_irq_mask; }
	UINT16 segacd_irq_mask;
	cdda_device* m_cdda;

	/* NeoCD */
	UINT16 nff0002;
	UINT16 nff0016;


	INT32 LC8951RegistersR[16];
	INT32 LC8951RegistersW[16];



	bool bNeoCDCommsClock;

	INT32 NeoCDCommsWordCount;

	INT32 NeoCD_StatusHack;




	void NeoCDCommsControl(UINT8 clock, UINT8 send);
	void LC8951UpdateHeader();
	char* LC8915InitTransfer(int NeoCDDMACount);
	void LC8915EndTransfer();

	void neocd_cdd_tx_w(UINT8 data);
	UINT8 neocd_cdd_rx_r();
	void NeoCDCommsReset();


	UINT16 nff0016_r(void);
	void nff0016_set(UINT16 wordValue);
	void nff0002_set(UINT16 wordValue);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
private:


};

extern const device_type LC89510_TEMP;
