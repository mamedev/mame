// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_SHARED_MEGACDCD_H
#define MAME_SHARED_MEGACDCD_H

#include "imagedev/cdromimg.h"
#include "machine/timer.h"
#include "sound/cdda.h"


class lc89510_temp_device : public device_t
{
public:
	typedef device_delegate<void (int &, uint8_t *, uint16_t &, uint16_t &)> dma_delegate;
	typedef device_delegate<void ()> interrupt_delegate;

	void set_is_neoCD(bool new_is_neoCD) { is_neoCD = new_is_neoCD; }

	template <typename... T> void set_type1_interrupt_callback(T &&... args) { m_type1_interrupt_callback.set(std::forward<T>(args)...); }
	template <typename... T> void set_type2_interrupt_callback(T &&... args) { m_type2_interrupt_callback.set(std::forward<T>(args)...); }
	template <typename... T> void set_type3_interrupt_callback(T &&... args) { m_type3_interrupt_callback.set(std::forward<T>(args)...); }

	template <typename... T> void set_cdc_do_dma_callback(T &&... args) { m_segacd_dma_callback.set(std::forward<T>(args)...); }

	template <typename T> void set_cdrom_tag(T &&tag) { m_cdrom.set_tag(std::forward<T>(tag)); m_cdda.lookup()->set_cdrom_tag(std::forward<T>(tag)); }
	template <typename T> void set_68k_tag(T &&tag) { m_68k.set_tag(std::forward<T>(tag)); }


	lc89510_temp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t get_segacd_irq_mask() const { return segacd_irq_mask; }

	uint16_t segacd_irq_mask_r();
	void segacd_irq_mask_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_cdd_ctrl_r();
	void segacd_cdd_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t segacd_cdd_rx_r(offs_t offset);
	void segacd_cdd_tx_w(offs_t offset, uint8_t data);
	uint16_t segacd_cdfader_r();
	void segacd_cdfader_w(uint16_t data);

	void segacd_cdc_mode_address_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_cdc_mode_address_r();
	void segacd_cdc_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t segacd_cdc_data_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t cdc_data_sub_r();
	uint16_t cdc_data_main_r();

	void CDC_Do_DMA(int rate);

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
	dma_delegate m_segacd_dma_callback;
	interrupt_delegate m_type1_interrupt_callback;
	interrupt_delegate m_type2_interrupt_callback;
	interrupt_delegate m_type3_interrupt_callback;

	void Fake_CDC_Do_DMA(int &dmacount, uint8_t *CDC_BUFFER, uint16_t &dma_addrc, uint16_t &destination );

	void dummy_interrupt_callback();


	required_device<cdrom_image_device> m_cdrom;
	required_device<cdda_device> m_cdda;

	// HACK for neoCD handling
	optional_device<cpu_device> m_68k;
	bool is_neoCD;


	struct segacd_t
	{
		const cdrom_file::toc   *toc;
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
	void scd_ctrl_checks();
	void scd_advance_current_readpos(void);
	int Read_LBA_To_Buffer();
	void CDD_GetStatus(void);
	void CDD_Stop();
	void CDD_GetPos(void);
	void CDD_GetTrackPos(void);
	void CDD_GetTrack(void);
	void CDD_Length(void);
	void CDD_FirstLast(void);
	void CDD_GetTrackAdr(void);
	void CDD_GetTrackType(void);
	uint32_t getmsf_from_regs(void);
	void CDD_Play();
	void CDD_Seek(void);
	void CDD_Pause();
	void CDD_Resume();
	void CDD_FF();
	void CDD_RW();
	void CDD_Open(void);
	void CDD_Close(void);
	void CDD_Init(void);
	void CDD_Default(void);
	void CDD_Reset(void);
	void CDC_Reset(void);
	void lc89510_Reset(void);
	void CDC_End_Transfer();
	uint16_t CDC_Host_r(uint16_t type);
	void CDD_Process(int reason);
	void CDD_Handle_TOC_Commands(void);
	bool CDD_Import();

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


	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER( segacd_access_timer_callback );
};

DECLARE_DEVICE_TYPE(LC89510_TEMP, lc89510_temp_device)

#endif // MAME_SHARED_MEGACDCD_H
