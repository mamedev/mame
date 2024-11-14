// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Olivier Galibert, David Haywood, Samuele Zannoli, R. Belmont, ElSemi

#ifndef MAME_SEGA_DC_H
#define MAME_SEGA_DC_H

#pragma once

#include "dc_g2if.h"
#include "maple-dc.h"
#include "naomig1.h"
#include "powervr2.h"

#include "cpu/arm7/arm7.h"
#include "cpu/sh/sh4.h"
#include "machine/timer.h"
#include "sound/aica.h"

class dc_state : public driver_device
{
public:
	dc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, dc_framebuffer_ram(*this, "frameram")
		, dc_texture_ram(*this, "dc_texture_ram")
		, dc_sound_ram(*this, "dc_sound_ram")
		, dc_ram(*this, "dc_ram")
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_powervr2(*this, "powervr2")
		, m_maple(*this, "maple_dc")
		, m_naomig1(*this, "rom_board")
		, m_g2if(*this, "sb_g2if")
		, m_aica(*this, "aica")
	{ }

	required_shared_ptr<uint64_t> dc_framebuffer_ram; // '32-bit access area'
	required_shared_ptr<uint64_t> dc_texture_ram; // '64-bit access area'

	required_shared_ptr<uint16_t> dc_sound_ram;
	required_shared_ptr<uint64_t> dc_ram;

	/* machine related */
	uint32_t dc_sysctrl_regs[0x200/4]{};
	uint8_t m_armrst = 0U;
	emu_timer *m_ch2_dma_irq_timer = nullptr;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void g2_dma_end_w(offs_t channel, u8 state);
	void g2_dma_error_ia_w(offs_t channel, u8 state);
	void g2_dma_error_ov_w(offs_t channel, u8 state);
	TIMER_CALLBACK_MEMBER(ch2_dma_irq);
	uint32_t dc_aica_reg_r(offs_t offset, uint32_t mem_mask = ~0);
	void dc_aica_reg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dc_arm_aica_r(offs_t offset);
	void dc_arm_aica_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	inline int decode_reg32_64(uint32_t offset, uint64_t mem_mask, uint64_t *shift);
	inline int decode_reg3216_64(uint32_t offset, uint64_t mem_mask, uint64_t *shift);
	int dc_compute_interrupt_level();
	void dc_update_interrupt_status();
	inline int decode_reg_64(uint32_t offset, uint64_t mem_mask, uint64_t *shift);
	uint64_t dc_sysctrl_r(offs_t offset, uint64_t mem_mask = ~0);
	void dc_sysctrl_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t dc_gdrom_r(offs_t offset, uint64_t mem_mask = ~0);
	void dc_gdrom_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t dc_modem_r(offs_t offset, uint64_t mem_mask = ~0);
	void dc_modem_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void g1_irq(uint8_t data);
	void pvr_irq(uint8_t data);
	void maple_irq(uint8_t data);
	uint16_t soundram_r(offs_t offset);
	void soundram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void aica_irq(int state);
	void sh4_aica_irq(int state);
	void external_irq(int state);


	required_device<sh4_base_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<powervr2_device> m_powervr2;
	required_device<maple_dc_device> m_maple;
	optional_device<naomi_g1_device> m_naomig1;
	required_device<dc_g2if_device> m_g2if;
	required_device<aica_device> m_aica;

	void generic_dma(uint32_t main_adr, void *dma_ptr, uint32_t length, uint32_t size, bool to_mainram);
	TIMER_DEVICE_CALLBACK_MEMBER(dc_scanline);
	DECLARE_MACHINE_RESET(dc_console);

	void naomi_aw_base(machine_config &config);
	void aica_map(address_map &map) ATTR_COLD;
	void dc_audio_map(address_map &map) ATTR_COLD;

protected:
	void system_bus_config(machine_config &config, const char *cpu_tag);
};

/*--------- Ch2-DMA Control Registers ----------*/
#define SB_C2DSTAT  ((0x005f6800-0x005f6800)/4)
#define SB_C2DLEN   ((0x005f6804-0x005f6800)/4)
#define SB_C2DST    ((0x005f6808-0x005f6800)/4)
/*-------- Sort-DMA Control Registers ----------*/
#define SB_SDSTAW   ((0x005f6810-0x005f6800)/4)
#define SB_SDBAAW   ((0x005f6814-0x005f6800)/4)
#define SB_SDWLT    ((0x005f6818-0x005f6800)/4)
#define SB_SDLAS    ((0x005f681c-0x005f6800)/4)
#define SB_SDST     ((0x005f6820-0x005f6800)/4)
/*-- DDT I/F Block & System Control Registers --*/
#define SB_DBREQM   ((0x005f6840-0x005f6800)/4)
#define SB_BAVLWC   ((0x005f6844-0x005f6800)/4)
#define SB_C2DPRYC  ((0x005f6848-0x005f6800)/4)
#define SB_C2DMAXL  ((0x005f684c-0x005f6800)/4)
#define SB_TFREM    ((0x005f6880-0x005f6800)/4)
#define SB_LMMODE0  ((0x005f6884-0x005f6800)/4)
#define SB_LMMODE1  ((0x005f6888-0x005f6800)/4)
#define SB_FFST     ((0x005f688c-0x005f6800)/4)
#define SB_SFRES    ((0x005f6890-0x005f6800)/4)
#define SB_SBREV    ((0x005f689c-0x005f6800)/4)
#define SB_RBSPLT   ((0x005f68a0-0x005f6800)/4)
/*-------- Interrupt Control Registers ---------*/
#define SB_ISTNRM   ((0x005f6900-0x005f6800)/4)
#define SB_ISTEXT   ((0x005f6904-0x005f6800)/4)
#define SB_ISTERR   ((0x005f6908-0x005f6800)/4)
#define SB_IML2NRM  ((0x005f6910-0x005f6800)/4)
#define SB_IML2EXT  ((0x005f6914-0x005f6800)/4)
#define SB_IML2ERR  ((0x005f6918-0x005f6800)/4)
#define SB_IML4NRM  ((0x005f6920-0x005f6800)/4)
#define SB_IML4EXT  ((0x005f6924-0x005f6800)/4)
#define SB_IML4ERR  ((0x005f6928-0x005f6800)/4)
#define SB_IML6NRM  ((0x005f6930-0x005f6800)/4)
#define SB_IML6EXT  ((0x005f6934-0x005f6800)/4)
#define SB_IML6ERR  ((0x005f6938-0x005f6800)/4)
#define SB_PDTNRM   ((0x005f6940-0x005f6800)/4)
#define SB_PDTEXT   ((0x005f6944-0x005f6800)/4)
#define SB_G2DTNRM  ((0x005f6950-0x005f6800)/4)
#define SB_G2DTEXT  ((0x005f6954-0x005f6800)/4)


/*-------- Maple-DMA Control Registers ---------*/
#define SB_MDSTAR   ((0x005f6c04-0x005f6c00)/4)
#define SB_MDTSEL   ((0x005f6c10-0x005f6c00)/4)
#define SB_MDEN     ((0x005f6c14-0x005f6c00)/4)
#define SB_MDST     ((0x005f6c18-0x005f6c00)/4)
/*---- Maple I/F Block HW Control Registers ----*/
#define SB_MSYS     ((0x005f6c80-0x005f6c00)/4)
#define SB_MST      ((0x005f6c84-0x005f6c00)/4)
#define SB_MSHTCL   ((0x005f6c88-0x005f6c00)/4)
#define SB_MDAPRO   ((0x005f6c8c-0x005f6c00)/4)
#define SB_MMSEL    ((0x005f6ce8-0x005f6c00)/4)
/*-------- Maple-DMA Debug Registers -----------*/
#define SB_MTXDAD   ((0x005f6cf4-0x005f6c00)/4)
#define SB_MRXDAD   ((0x005f6cf8-0x005f6c00)/4)
#define SB_MRXDBD   ((0x005f6cfc-0x005f6c00)/4)

/*--------- GD-DMA Control Registers -----------*/
#define SB_GDSTAR   ((0x005f7404-0x005f7400)/4)
#define SB_GDLEN    ((0x005f7408-0x005f7400)/4)
#define SB_GDDIR    ((0x005f740c-0x005f7400)/4)
#define SB_GDEN     ((0x005f7414-0x005f7400)/4)
#define SB_GDST     ((0x005f7418-0x005f7400)/4)
/*----- G1 I/F Block HW Control Registers ------*/
#define SB_G1RRC    ((0x005f7480-0x005f7400)/4)
#define SB_G1RWC    ((0x005f7484-0x005f7400)/4)
#define SB_G1FRC    ((0x005f7488-0x005f7400)/4)
#define SB_G1FWC    ((0x005f748c-0x005f7400)/4)
#define SB_G1CRC    ((0x005f7490-0x005f7400)/4)
#define SB_G1CWC    ((0x005f7494-0x005f7400)/4)
#define SB_G1GDRC   ((0x005f74a0-0x005f7400)/4)
#define SB_G1GDWC   ((0x005f74a4-0x005f7400)/4)
#define SB_G1SYSM   ((0x005f74b0-0x005f7400)/4)
#define SB_G1CRDYC  ((0x005f74b4-0x005f7400)/4)
#define SB_GDAPRO   ((0x005f74b8-0x005f7400)/4)

/*-------- BIOS security Registers ---------*/
#define SB_SECUR_EADR  ((0x005f74e4-0x005f7400)/4)
#define SB_SECUR_STATE ((0x005f74ec-0x005f7400)/4)
/*---------- GD-DMA Debug Registers ------------*/
#define SB_GDSTARD  ((0x005f74f4-0x005f7400)/4)
#define SB_GDLEND   ((0x005f74f8-0x005f7400)/4)

/*-------- Wave DMA Control Registers ----------*/
#define SB_ADSTAG   ((0x005f7800-0x005f7800)/4)
#define SB_ADSTAR   ((0x005f7804-0x005f7800)/4)
#define SB_ADLEN    ((0x005f7808-0x005f7800)/4)
#define SB_ADDIR    ((0x005f780c-0x005f7800)/4)
#define SB_ADTSEL   ((0x005f7810-0x005f7800)/4)
#define SB_ADTRG    SB_ADTSEL
#define SB_ADEN     ((0x005f7814-0x005f7800)/4)
#define SB_ADST     ((0x005f7818-0x005f7800)/4)
#define SB_ADSUSP   ((0x005f781c-0x005f7800)/4)

/*----- External 1 DMA Control Registers -------*/
#define SB_E1STAG   ((0x005f7820-0x005f7800)/4)
#define SB_E1STAR   ((0x005f7824-0x005f7800)/4)
#define SB_E1LEN    ((0x005f7828-0x005f7800)/4)
#define SB_E1DIR    ((0x005f782c-0x005f7800)/4)
#define SB_E1TSEL   ((0x005f7830-0x005f7800)/4)
#define SB_E1TRG    SB_E1TSEL
#define SB_E1EN     ((0x005f7834-0x005f7800)/4)
#define SB_E1ST     ((0x005f7838-0x005f7800)/4)
#define SB_E1SUSP   ((0x005f783c-0x005f7800)/4)

/*----- External 2 DMA Control Registers -------*/
#define SB_E2STAG   ((0x005f7840-0x005f7800)/4)
#define SB_E2STAR   ((0x005f7844-0x005f7800)/4)
#define SB_E2LEN    ((0x005f7848-0x005f7800)/4)
#define SB_E2DIR    ((0x005f784c-0x005f7800)/4)
#define SB_E2TSEL   ((0x005f7850-0x005f7800)/4)
#define SB_E2TRG    SB_E2TSEL
#define SB_E2EN     ((0x005f7854-0x005f7800)/4)
#define SB_E2ST     ((0x005f7858-0x005f7800)/4)
#define SB_E2SUSP   ((0x005f785c-0x005f7800)/4)

/*------- Debug DMA Control Registers ----------*/
#define SB_DDSTAG   ((0x005f7860-0x005f7800)/4)
#define SB_DDSTAR   ((0x005f7864-0x005f7800)/4)
#define SB_DDLEN    ((0x005f7868-0x005f7800)/4)
#define SB_DDDIR    ((0x005f786c-0x005f7800)/4)
#define SB_DDTSEL   ((0x005f7870-0x005f7800)/4)
#define SB_DDTRG    SB_DDTSEL
#define SB_DDEN     ((0x005f7874-0x005f7800)/4)
#define SB_DDST     ((0x005f7878-0x005f7800)/4)
#define SB_DDSUSP   ((0x005f787c-0x005f7800)/4)
/*----- G2 I/F Block HW Control Registers ------*/
#define SB_G2ID     ((0x005f7880-0x005f7800)/4)
#define SB_G2DSTO   ((0x005f7890-0x005f7800)/4)
#define SB_G2TRTO   ((0x005f7894-0x005f7800)/4)
#define SB_G2MDMTO  ((0x005f7898-0x005f7800)/4)
#define SB_G2MDMW   ((0x005f789c-0x005f7800)/4)
#define SB_G2APRO   ((0x005f78bc-0x005f7800)/4)

/*---------- G2 DMA Debug Registers ------------*/
#define SB_ADSTAGD  ((0x005f78c0-0x005f7800)/4)
#define SB_ADSTARD  ((0x005f78c4-0x005f7800)/4)
#define SB_ADLEND   ((0x005f78c8-0x005f7800)/4)
#define SB_E1STAGD  ((0x005f78d0-0x005f7800)/4)
#define SB_E1STARD  ((0x005f78d4-0x005f7800)/4)
#define SB_E1LEND   ((0x005f78d8-0x005f7800)/4)
#define SB_E2STAGD  ((0x005f78e0-0x005f7800)/4)
#define SB_E2STARD  ((0x005f78e4-0x005f7800)/4)
#define SB_E2LEND   ((0x005f78e8-0x005f7800)/4)
#define SB_DDSTAGD  ((0x005f78f0-0x005f7800)/4)
#define SB_DDSTARD  ((0x005f78f4-0x005f7800)/4)
#define SB_DDLEND   ((0x005f78f8-0x005f7800)/4)

#define RTC1        ((0x00710000-0x00710000)/4)
#define RTC2        ((0x00710004-0x00710000)/4)
#define RTC3        ((0x00710008-0x00710000)/4)


/* ------------- normal interrupts ------------- */
#define IST_EOR_VIDEO    0x00000001
#define IST_EOR_ISP      0x00000002
#define IST_EOR_TSP      0x00000004
#define IST_VBL_IN       0x00000008
#define IST_VBL_OUT      0x00000010
#define IST_HBL_IN       0x00000020
#define IST_EOXFER_YUV   0x00000040
#define IST_EOXFER_OPLST 0x00000080
#define IST_EOXFER_OPMV  0x00000100
#define IST_EOXFER_TRLST 0x00000200
#define IST_EOXFER_TRMV  0x00000400
#define IST_DMA_PVR      0x00000800
#define IST_DMA_MAPLE    0x00001000
#define IST_DMA_MAPLEVB  0x00002000
#define IST_DMA_GDROM    0x00004000
#define IST_DMA_AICA     0x00008000
#define IST_DMA_EXT1     0x00010000
#define IST_DMA_EXT2     0x00020000
#define IST_DMA_DEV      0x00040000
#define IST_DMA_CH2      0x00080000
#define IST_DMA_SORT     0x00100000
#define IST_EOXFER_PTLST 0x00200000
#define IST_G1G2EXTSTAT  0x40000000
#define IST_ERROR        0x80000000
/* ------------ external interrupts ------------ */
#define IST_EXT_EXTERNAL    0x00000008
#define IST_EXT_MODEM   0x00000004
#define IST_EXT_AICA    0x00000002
#define IST_EXT_GDROM   0x00000001
/* -------------- error interrupts ------------- */
#define IST_ERR_ISP_LIMIT        0x00000004
#define IST_ERR_PVRIF_ILL_ADDR   0x00000040

#endif // MAME_SEGA_DC_H
