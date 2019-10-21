// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_SEGACDCD_H
#define MAME_MACHINE_SEGACDCD_H

#include "imagedev/chd_cd.h"
#include "machine/timer.h"
#include "sound/cdda.h"


typedef device_delegate<void (int&, uint8_t*, uint16_t&, uint16_t&)> segacd_dma_delegate;

typedef device_delegate<void (void)> interrupt_delegate;

class lc89510_temp_device : public device_t
{
public:
	void set_is_neoCD(bool new_is_neoCD) { is_neoCD = new_is_neoCD; }

	template <typename... T> void set_type1_interrupt_callback(T &&... args) { type1_interrupt_callback = interrupt_delegate(std::forward<T>(args)...); }
	template <typename... T> void set_type2_interrupt_callback(T &&... args) { type2_interrupt_callback = interrupt_delegate(std::forward<T>(args)...); }
	template <typename... T> void set_type3_interrupt_callback(T &&... args) { type3_interrupt_callback = interrupt_delegate(std::forward<T>(args)...); }

	template <typename... T> void set_cdc_do_dma_callback(T &&... args) { segacd_dma_callback = segacd_dma_delegate(std::forward<T>(args)...); }

	template <typename T> void set_cdrom_tag(T &&tag) { m_cdrom.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_68k_tag(T &&tag) { m_68k.set_tag(std::forward<T>(tag)); }


	lc89510_temp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t get_segacd_irq_mask() const { return segacd_irq_mask; }

	DECLARE_READ16_MEMBER( segacd_irq_mask_r );
	DECLARE_WRITE16_MEMBER( segacd_irq_mask_w );
	DECLARE_READ16_MEMBER( segacd_cdd_ctrl_r );
	DECLARE_WRITE16_MEMBER( segacd_cdd_ctrl_w );
	DECLARE_READ8_MEMBER( segacd_cdd_rx_r );
	DECLARE_WRITE8_MEMBER( segacd_cdd_tx_w );
	READ16_MEMBER( segacd_cdfader_r );
	WRITE16_MEMBER( segacd_cdfader_w );

	WRITE16_MEMBER( segacd_cdc_mode_address_w );
	READ16_MEMBER( segacd_cdc_mode_address_r );
	WRITE16_MEMBER( segacd_cdc_data_w );
	READ16_MEMBER( segacd_cdc_data_r );
	READ16_MEMBER( cdc_data_sub_r );
	READ16_MEMBER( cdc_data_main_r );

	void CDC_Do_DMA(running_machine& machine, int rate);

	uint8_t CDC_Reg_r(void);
	void CDC_Reg_w(uint8_t data);

	void reset_cd();

	// NeoGeo CD stuff
	void neocd_cdd_tx_w(uint8_t data);
	uint8_t neocd_cdd_rx_r();
	void NeoCDCommsControl(uint8_t clock, uint8_t send);
	void NeoCDCommsReset();

	uint16_t nff0016_r();
	void nff0016_set(uint16_t wordValue);
	void nff0002_set(uint16_t wordValue);

	char* LC8915InitTransfer(int NeoCDDMACount);
	void LC8915EndTransfer();

protected:
	static constexpr unsigned SECTOR_SIZE = 2352;
	static constexpr unsigned EXTERNAL_BUFFER_SIZE = (32 * 1024 * 2) + SECTOR_SIZE;

	// HACK for DMA handling
	segacd_dma_delegate segacd_dma_callback;
	interrupt_delegate type1_interrupt_callback;
	interrupt_delegate type2_interrupt_callback;
	interrupt_delegate type3_interrupt_callback;

	void Fake_CDC_Do_DMA(int &dmacount, uint8_t *CDC_BUFFER, uint16_t &dma_addrc, uint16_t &destination );

	void dummy_interrupt_callback(void);


	required_device<cdrom_image_device> m_cdrom;
	required_device<cdda_device> m_cdda;

	// HACK for neoCD handling
	optional_device<cpu_device> m_68k;
	bool is_neoCD;


	struct segacd_t
	{
		cdrom_file  *cd;
		const cdrom_toc   *toc;
		uint32_t current_frame;
	};


	segacd_t segacd;

	uint8_t    SCD_BUFFER[2560];

	uint32_t   SCD_STATUS;
	uint32_t   SCD_STATUS_CDC;
	int32_t    SCD_CURLBA;
	uint8_t    SCD_CURTRK;


	uint16_t CDC_DECODE;
	uint16_t CDC_REG0;
	uint16_t CDC_REG1;

	uint8_t CDC_BUFFER[EXTERNAL_BUFFER_SIZE];


	uint8_t CDD_RX[10];
	uint8_t CDD_TX[10];

	uint32_t CDD_STATUS;
	uint32_t CDD_MIN;
	uint32_t CDD_SEC;
	uint32_t CDD_FRAME;
	uint32_t CDD_EXT;

	uint16_t CDD_CONTROL;

	int16_t  CDD_DONE;

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
	uint32_t getmsf_from_regs(void);
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
	uint16_t CDC_Host_r(running_machine& machine, uint16_t type);
	void CDD_Process(running_machine& machine, int reason);
	void CDD_Handle_TOC_Commands(void);
	bool CDD_Import(running_machine& machine);

	uint16_t segacd_irq_mask;

	/* NeoCD */
	uint16_t nff0002;
	uint16_t nff0016;


	int32_t LC8951RegistersR[16];
	int32_t LC8951RegistersW[16];



	bool bNeoCDCommsClock;

	int32_t NeoCDCommsWordCount;

	int32_t NeoCD_StatusHack;

	void LC8951UpdateHeader();


	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	TIMER_DEVICE_CALLBACK_MEMBER( segacd_access_timer_callback );
};

DECLARE_DEVICE_TYPE(LC89510_TEMP, lc89510_temp_device)

#endif // MAME_MACHINE_SEGACDCD_H
