// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "generalplus_gpl951xx_soc.h"

#define LOG_SPIFC     (1U << 1)
#define LOG_TFT       (1U << 2)
#define LOG_OTHER     (1U << 3)

#define VERBOSE     (LOG_SPIFC | LOG_TFT | LOG_OTHER)

#include "logmacro.h"


// SPIFC - the directly mapped SPI interface
// provides 'hardware accelerated' SPI features (composing and sending packets to the SPI device)
// including XIP (eXecute In Place) support for flat view of SPI ROM

void generalplus_gpl951xx_device::recieve_spi_fifo_data(u8 data)
{
	if (m_bytes_in_spifc_rx_fifo < (16 * 2))
	{
		m_spifc_rx_fifo[m_bytes_in_spifc_rx_fifo] = data;
		m_bytes_in_spifc_rx_fifo++;
	}
}

u8 generalplus_gpl951xx_device::get_byte_from_rx_fifo()
{
	u8 ret = m_spifc_rx_fifo[0];
	if (m_bytes_in_spifc_rx_fifo > 0)
	{

		for (int i = 1; i < (16 * 2); i++)
		{
			m_spifc_rx_fifo[i - 1] = m_spifc_rx_fifo[i];
		}
		m_spifc_rx_fifo[(16 * 2) - 1] = 0;
		m_bytes_in_spifc_rx_fifo--;
	}
	return ret;
}

// this provides hardware accelerated SPI support, handling much of the underlying SPI
// access, allowing 'easier' use of the device, as well as allowing it to run in XIP
// mode (eXecute In Place) so the CPU can see it as a flat space

// P_SPIFC_Ctrl1

// 15  tx_done
// 14  rx_empty
// 13
// 12
//
// 11
// 10
// 9   ig_clk
// 8   manual
//
// 7   cmio[1]
// 6   cmio[0]
// 5   amio[1]
// 4   amio[0]
//
// 3   mio[1]
// 2   mio[0]
// 1
// 0   back2idle

u16 generalplus_gpl951xx_device::spifc_ctrl_r()
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_ctrl_r\n", machine().describe_context());
	u16 ret = m_spifc_ctrl;

	ret |= 0x8000; // tx_done
	if (m_bytes_in_spifc_rx_fifo == 0) ret |= 0x4000; // rx_empty
	ret |= 0x0001; // back2idle

	return ret;
}

void generalplus_gpl951xx_device::spifc_ctrl_w(u16 data)
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_ctrl_w %04x\n", machine().describe_context(), data);
	m_spifc_ctrl = data;
}

// P_SPIFC_CMD
//
// 15
// 14
// 13  one_cmd
// 12
//
// 11
// 10
//  9  cmd_only
//  8  wo_cmd
//
//  7  wpif_cmd[7]
//  6  wpif_cmd[6]
//  5  wpif_cmd[5]
//  4  wpif_cmd[4]
//
//  3  wpif_cmd[3]
//  2  wpif_cmd[2]
//  1  wpif_cmd[1]
//  0  wpif_cmd[0]

u16 generalplus_gpl951xx_device::spifc_cmd_r()
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_cmd_r\n", machine().describe_context());
	return m_spifc_cmd;
}

void generalplus_gpl951xx_device::spifc_cmd_w(u16 data)
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_cmd_w %02x %02x with param %04x\n", machine().describe_context(), (data & 0xff00) >> 8, data & 0xff, m_spifc_para);

	m_spifc_cmd = data;

	m_spi_reset(1);
	m_spi_out_cmd(data & 0x00ff);

	// how does byte count work? the FIFO is apparently in words
	for (int i = 0; i < m_spifc_rx_bc; i++)
	{
		// clock it this many times to push data into the fifo?
		m_spi_out(0x00);
	}

}

// P_SPIFC_PARA
//
// 15
// 14  wo_enha
// 13  to_addr
// 12  wo_addr
//
// 11  dummy_ck_cnt[3]
// 10  dummy_ck_cnt[2]
// 9   dummy_ck_cnt[1]
// 8   dummy_ck_cnt[0]
//
// 7   enhan_by[7]
// 6   enhan_by[6]
// 5   enhan_by[5]
// 4   enhan_by[4]
//
// 3   enhan_by[4]
// 2   enhan_by[3]
// 1   enhan_by[2]
// 0   enhan_by[1]


u16 generalplus_gpl951xx_device::spifc_para_r()
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_para_r\n", machine().describe_context());
	return m_spifc_para;
}

void generalplus_gpl951xx_device::spifc_para_w(u16 data)
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_para_w %04x\n", machine().describe_context(), data);
	m_spifc_para = data;
}

// P_SPIFC_ADDRL
// 15-0 low 16 bits of SPIF Address

u16 generalplus_gpl951xx_device::spifc_addrl_r()
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_addrl_r\n", machine().describe_context());
	return m_spifc_addr & 0x0000ffff;
}

void generalplus_gpl951xx_device::spifc_addrl_w(u16 data)
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_addrl_w %04x\n", machine().describe_context(), data);
	m_spifc_addr = (m_spifc_addr & 0xffff0000) | data;
}

// P_SPIFC_ADDRH
// 15-0  high 16 bits of SPIF Address

u16 generalplus_gpl951xx_device::spifc_addrh_r()
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_addrh_r\n", machine().describe_context());
	return (m_spifc_addr & 0xffff0000) >> 16;
}

void generalplus_gpl951xx_device::spifc_addrh_w(u16 data)
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_addrh_w %04x\n", machine().describe_context(), data);
	m_spifc_addr = (m_spifc_addr & 0x0000ffff) | (data << 16);
}

u16 generalplus_gpl951xx_device::spifc_txdat_r()
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_txdat_r\n", machine().describe_context());
	return 0xffff;
}

void generalplus_gpl951xx_device::spifc_txdat_w(u16 data)
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_txdat_w %04x\n", machine().describe_context(), data);
	m_spi_out(data & 0xff);
}

// P_SPIFC_RX_Data
//
// 15-0  2 bytes of RX_Data
//
// write the register once before reading the register once

u16 generalplus_gpl951xx_device::spifc_rxdat_r()
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_rxdat_r\n", machine().describe_context());

	/*
	for bfpacman

	after auto command bite spifc_cmd_w is set to 9f (ident)

	at 672 R1 will be xx00 (results from device)
	R2 will be xxxx (results from device)

	continuing to 232 it restores these values and it checks if R1 is 0000 (it shouldn't be)

	at 237 it gets a value of C814 from 0d69 (RAM) and puts it in R1
	at 239 is compares R1 with R2
	*/

	return m_spifc_rx_read_latch;
}


void generalplus_gpl951xx_device::spifc_rxdat_w(u16 data)
{
	// write here to latch a word from the fifo into the read register
	u16 word = get_byte_from_rx_fifo();
	word = (get_byte_from_rx_fifo() << 8) | word;

	m_spifc_rx_read_latch = word;
}


// P_SPIFC_TX_BC - SPIFC TX byte count register
//
// 15-9 unused
// 8-0  9-bit byte count (TX_BC)

u16 generalplus_gpl951xx_device::spifc_tx_bc_r()
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_tx_bc_r\n", machine().describe_context());
	return m_spifc_tx_bc;
}

void generalplus_gpl951xx_device::spifc_tx_bc_w(u16 data)
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_tx_bc_w %04x\n", machine().describe_context(), data);
	m_spifc_tx_bc = data;
}

// P_SPIFC_RX_BC - SPIFC RX byte count register
//
// 15-9 unused
// 8-0  9-bit byte count (RX_BC)

u16 generalplus_gpl951xx_device::spifc_rx_bc_r()
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_rx_bc_r\n", machine().describe_context());
	return m_spifc_rx_bc;
}

void generalplus_gpl951xx_device::spifc_rx_bc_w(u16 data)
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_rx_bc_w %04x\n", machine().describe_context(), data);
	m_spifc_rx_bc = data;
}

u16 generalplus_gpl951xx_device::spifc_timing_r()
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_timing_r\n", machine().describe_context());
	return m_spifc_timing;
}

void generalplus_gpl951xx_device::spifc_timing_w(u16 data)
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_timing_w %04x\n", machine().describe_context(), data);
	m_spifc_timing = data;
}

// P_SPIFC_Ctrl2
//
// 15-8  Ear[7:0] - Extend address range from 24 bits to 32-bits
// 7-1
// 0     en - SPF flash controller enable

u16 generalplus_gpl951xx_device::spifc_ctrl2_r()
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_ctrl2_r\n", machine().describe_context());
	return m_spifc_ctrl2;
}

void generalplus_gpl951xx_device::spifc_ctrl2_w(u16 data)
{
	LOGMASKED(LOG_SPIFC, "%s: spifc_ctrl2_w %04x\n", machine().describe_context(), data);
	m_spifc_ctrl2 = data;
}

u16 generalplus_gpl951xx_device::spi_improve_r()
{
	LOGMASKED(LOG_SPIFC, "%s: spi_improve_r\n", machine().describe_context());
	return 0x0000;
}

void generalplus_gpl951xx_device::spi_improve_w(u16 data)
{
	LOGMASKED(LOG_SPIFC, "%s: spi_improve_w %04x\n", machine().describe_context(), data);
}

// Other bits

void generalplus_gpl951xx_device::pm_ctrl_w(u16 data)
{
	LOGMASKED(LOG_SPIFC, "%s: pm_ctrl_w %04x\n", machine().describe_context(), data);
}

u16 generalplus_gpl951xx_device::pllsel_r()
{
	logerror("%s: pllsel_r\n", machine().describe_context());
	return m_pllchange;
}

void generalplus_gpl951xx_device::pllsel_w(u16 data)
{
	// very similar to pllchange on GPL162xx, but with extra SPI bits
	logerror("%s: generalplus_gpl951xx_device::pllsel_w %04x\n", machine().describe_context(), data);
	m_pllchange = data;
}

u16 generalplus_gpl951xx_device::byte_swap_r()
{
	return m_byteswap;
}

void generalplus_gpl951xx_device::byte_swap_w(u16 data)
{
	// words read from ROM are written here during the checksum routine in RAM, and must
	// be shifted for the checksum to pass.
	m_byteswap = ((data & 0xff00) >> 8) | ((data & 0x00ff) << 8);
}

void generalplus_gpl951xx_device::device_start()
{
	unsp_20_device::device_start();

	save_item(NAME(m_byteswap));
	save_item(NAME(m_timerg_ctrl));
	save_item(NAME(m_timerg_preload));
	save_item(NAME(m_timerh_ctrl));
	save_item(NAME(m_timerh_preload));
	save_item(NAME(m_spifc_ctrl));
	save_item(NAME(m_spifc_ctrl2));
	save_item(NAME(m_spifc_addr));
	save_item(NAME(m_spifc_cmd));
	save_item(NAME(m_spifc_para));
	save_item(NAME(m_spifc_rx_bc));
	save_item(NAME(m_spifc_tx_bc));
	save_item(NAME(m_spifc_timing));
	save_item(NAME(m_bytes_in_spifc_rx_fifo));
	save_item(NAME(m_spi_bank));
	save_item(NAME(m_memmode_wcmd));
	save_item(NAME(m_spifc_rx_fifo));
	save_item(NAME(m_spifc_rx_read_latch));
	save_item(NAME(m_io_dir));
	save_item(NAME(m_io_attrib));
	save_item(NAME(m_sys_ctrl));
	save_item(NAME(m_clock_ctrl));
	save_item(NAME(m_cache_ctrl));
	save_item(NAME(m_int_priority_1));
	save_item(NAME(m_int_priority_2));
	save_item(NAME(m_int_priority_3));
	save_item(NAME(m_misc_int_ctrl));
	save_item(NAME(m_pllchange));
}

void generalplus_gpl951xx_device::device_reset()
{
	unsp_20_device::device_reset();
	m_spg_video->reset();

	m_byteswap = 0;
	m_timerg_ctrl = 0;
	m_timerg_preload = 0;
	m_timerh_ctrl = 0;
	m_timerh_preload = 0;
	m_spifc_ctrl = 0;
	m_spifc_ctrl2 = 0;
	m_spifc_addr = 0;
	m_spifc_cmd = 0;
	m_spifc_para = 0;
	m_spifc_rx_bc = 0;
	m_spifc_tx_bc = 0;
	m_spifc_timing = 0;
	m_bytes_in_spifc_rx_fifo = 0;
	m_spifc_rx_read_latch = 0;
	m_memmode_wcmd = 0;
	m_spi_bank = 0;
	m_sys_ctrl = 0;
	m_clock_ctrl = 0;
	m_cache_ctrl = 0;
	m_int_priority_1 = 0;
	m_int_priority_2 = 0;
	m_int_priority_3 = 0;
	m_misc_int_ctrl = 0;
	m_pllchange = 0;

	for (int i = 0; i < 6; i++)
	{
		m_io_dir[i] = 0;
		m_io_attrib[i] = 0;
	}

	for (int i = 0; i < 16 * 2; i++)
		m_spifc_rx_fifo[i] = 0;
}

// Timers

void generalplus_gpl951xx_device::dac_0_w(uint16_t data)
{
	m_dac0->write(data);
}

TIMER_DEVICE_CALLBACK_MEMBER( generalplus_gpl951xx_device::timer_g_cb )
{
	m_timerg_ctrl |= 0x8000;
	update_interrupts(1);
	m_gpl_chx->process_cha_fifo(); // is cha hardwired to timerg overflow?
}

void generalplus_gpl951xx_device::dac_1_w(uint16_t data)
{
	m_dac1->write(data);
}

TIMER_DEVICE_CALLBACK_MEMBER( generalplus_gpl951xx_device::timer_h_cb )
{
	m_timerh_ctrl |= 0x8000;
	update_interrupts(1);
	m_gpl_chx->process_chb_fifo(); // is chb hardwired to timerh overflow?
}

u16 generalplus_gpl951xx_device::timerg_preload_r()
{
	logerror("%s: timerg_preload_r\n", machine().describe_context());
	return m_timerg_preload;
}

void generalplus_gpl951xx_device::timerg_preload_w(u16 data)
{
	logerror("%s: timerg_preload_w %04x\n", machine().describe_context(), data);
	m_timerg_preload = data;
}

// P_TimerG_Ctrl

// 15  TMGIF/C
// 14  TMGIE
// 13  TMGEN
// 12
// 11  EXT0SEL[1]
// 10  EXT0SEL[0]
// 9   EXT1SEL[1]
// 8   EXT1SEL[0]
// 7
// 6   SRCBSEL[2]
// 5   SRCBSEL[1]
// 4   SRCBSEL[0]
// 3   SRCASEL[3]
// 2   SRCASEL[2]
// 1   SRCASEL[1]
// 0   SRCASEL[0]

u16 generalplus_gpl951xx_device::timerg_ctrl_r()
{
	logerror("%s: timerg_ctrl_r\n", machine().describe_context());
	u16 ret = m_timerg_ctrl;
	return ret;
}

void generalplus_gpl951xx_device::timerg_ctrl_w(u16 data)
{
	u8 tmgif_clear = (data & 0x8000) >> 15;
	u8 tmgie = (data & 0x4000) >> 14;
	u8 tmgen = (data & 0x2000) >> 13;
	u8 ext0sel = (data & 0x0c00) >> 10;
	u8 ext1sel = (data & 0x0300) >> 8;
	u8 srcbsel = (data & 0x0070) >> 4;
	u8 srcasel = (data & 0x000f) >> 0;

	logerror("%s: timerg_ctrl_w %04x (tmgif_clear %01x) (interrupt enabled %01x) (timer enabled %01x) (ext0sel %01x) (ext1sel %01x) (srcbsel %s) (srcasel %s)\n", machine().describe_context(), data, tmgif_clear, tmgie, tmgen, ext0sel, ext1sel, m_srcb[srcbsel], m_srca[srcasel]);

	if (data & 0x8000)
	{
		m_timerg_ctrl &= 0x7fff;
	}

	if ((data & 0x2000) != (m_timerg_ctrl & 0x2000))
	{
		if (data & 0x2000)
		{
			m_timer_g->adjust(attotime::zero, 0, attotime::from_hz(8000));
		}
		else
		{
			m_timer_g->adjust(attotime::never);
		}
	}

	m_timerg_ctrl = (m_timerg_ctrl & 0x8000) | (data & 0x7fff);
	update_interrupts(1);
}

u16 generalplus_gpl951xx_device::timerh_preload_r()
{
	logerror("%s: timerh_preload_r\n", machine().describe_context());
	return m_timerh_preload;
}

void generalplus_gpl951xx_device::timerh_preload_w(u16 data)
{
	logerror("%s: timerh_preload_w %04x\n", machine().describe_context(), data);
	m_timerh_preload = data;
}

// P_TimerH_Ctrl

// 15  TMHIF/C
// 14  TMHIE
// 13  TMHEN
// 12
//
// 11  EXT0SEL[1]
// 10  EXT0SEL[0]
// 9   EXT1SEL[1]
// 8   EXT1SEL[0]
//
// 7
// 6   SRCBSEL[2]
// 5   SRCBSEL[1]
// 4   SRCBSEL[0]
//
// 3   SRCASEL[3]
// 2   SRCASEL[2]
// 1   SRCASEL[1]
// 0   SRCASEL[0]

u16 generalplus_gpl951xx_device::timerh_ctrl_r()
{
	u16 ret = m_timerh_ctrl;
	logerror("%s: timerh_ctrl_r (returning %04x)\n", machine().describe_context(), ret);
	return ret;
}

void generalplus_gpl951xx_device::timerh_ctrl_w(u16 data)
{
	u8 tmhif_clear = (data & 0x8000) >> 15;
	u8 tmhie = (data & 0x4000) >> 14;
	u8 tmhen = (data & 0x2000) >> 13;
	u8 ext0sel = (data & 0x0c00) >> 10;
	u8 ext1sel = (data & 0x0300) >> 8;
	u8 srcbsel = (data & 0x0070) >> 4;
	u8 srcasel = (data & 0x000f) >> 0;

	logerror("%s: timerh_ctrl_w %04x (tmhif_clear %01x) (interrupt enabled %01x) (timer enabled %01x) (ext0sel %01x) (ext1sel %01x) (srcbsel %01x) (srcasel %01x)\n", machine().describe_context(), data, tmhif_clear, tmhie, tmhen, ext0sel, ext1sel, m_srcb[srcbsel], m_srca[srcasel]);

	if (data & 0x8000)
	{
		logerror("cleared timerh flag\n");
		m_timerh_ctrl &= 0x7fff;
	}

	if ((data & 0x2000) != (m_timerh_ctrl & 0x2000))
	{
		logerror("changed\n");
		if (data & 0x2000)
		{
			logerror("started timerh\n");
			m_timer_h->adjust(attotime::zero, 0, attotime::from_hz(1000));
		}
		else
		{
			m_timer_h->adjust(attotime::never);
		}

	}

	m_timerh_ctrl = (m_timerh_ctrl & 0x8000) | (data & 0x7fff);
	update_interrupts(1);
}


// TFT

// 15
// 14
// 13
// 12
//
// 11
// 10
//  9
//  8
//
//  7  MEM_STATE[3] or HSTS[1] - horizontal status register
//  6  MEM_STATE[2] or HSTS[0]
//  5  MEM_STATE[1] or VSTS[1] - vertical status register
//  4  MEM_STATE[0] or VSTS[0]
//
//  3
//  2
//  1  Frame Interrupt Occured
//  0

u16 generalplus_gpl951xx_device::tft_status_r()
{
	u16 ret = 0x0000;
	LOGMASKED(LOG_TFT, "%s: tft_status_r\n", machine().describe_context());
	return ret;
}

// 15  VSU
// 14  INLA
// 13  BEN
// 12  HCMP
//
// 11  DINV
// 10  CINV
//  9  HINV
//  8  VINV
//
//  7  MODE[3]
//  6  MODE[2]
//  5  MODE[1]
//  4  MODE[0]
//
//  3  CLK_SEL[2]
//  2  CLK_SEL[1]
//  1  CLK_SEL[0]
//  0  TFTEN

void generalplus_gpl951xx_device::tft_ctrl_w(u16 data)
{
	u8 vsu   = (data & 0x8000) >> 15;
	u8 inla  = (data & 0x4000) >> 14;
	u8 ben   = (data & 0x2000) >> 13;
	u8 hcmp  = (data & 0x1000) >> 12;
	u8 dinv  = (data & 0x0800) >> 11;
	u8 cinv  = (data & 0x0400) >> 10;
	u8 hinv  = (data & 0x0200) >> 9;
	u8 vinv  = (data & 0x0100) >> 8;
	u8 mode  = (data & 0x00f0) >> 4;
	u8 clk_s = (data & 0x000e) >> 1;
	u8 tften = (data & 0x0001) >> 0;

	static const char* modenames[16] =
	{
		"0: UPS051 mode",
		"1: UPS052 mode",
		"2: CCIR656 mode",
		"3: PARALLEL mode",
		"4: TOCN mode",
		"5: reserved",
		"6: reserved",
		"7: reserved",
		"8: I80 CMD write",
		"9: I80 CMD read",
		"a: I80 DATA write",
		"b: I80 DATA read",
		"c: I80 Continuous mode",
		"d: I80 Single mode",
		"e: reserved",
		"f: reserved"
	};

	LOGMASKED(LOG_TFT, "%s: tft_ctrl_w %04x (vsa %1x inla %1x ben %1x hcmp %1x dinv %1x cinv %1x hinv %1x vinv %1x mode (%s) clck_s %1x tft_en %1x\n", machine().describe_context(), data,
		vsu, inla, ben, hcmp, dinv, cinv, hinv, vinv, modenames[mode], clk_s, tften);

	if (tften)
	{
		if (mode == 0x8)
			m_i80_cmd_out(m_memmode_wcmd);
		else if (mode == 0x0a)
			m_i80_data_out(m_memmode_wcmd);
	}
}

void generalplus_gpl951xx_device::tft_memmode_wcmd_w(u16 data)
{
	LOGMASKED(LOG_TFT, "%s: tft_memmode_wcmd_w %04x\n", machine().describe_context(), data);
	m_memmode_wcmd = data;
}

//

u16 generalplus_gpl951xx_device::spi_bank_r()
{
	LOGMASKED(LOG_SPIFC, "%s: spi_bank_r\n", machine().describe_context());
	return m_spi_bank;
}

void generalplus_gpl951xx_device::spi_bank_w(u16 data)
{
	LOGMASKED(LOG_SPIFC, "%s: spi_bank_w %04x\n", machine().describe_context(), data);
	m_spi_bank = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(generalplus_gpl951xx_device::timer_a_cb)
{
}

TIMER_DEVICE_CALLBACK_MEMBER(generalplus_gpl951xx_device::timer_b_cb)
{
}

TIMER_DEVICE_CALLBACK_MEMBER(generalplus_gpl951xx_device::timer_c_cb)
{
}

TIMER_DEVICE_CALLBACK_MEMBER(generalplus_gpl951xx_device::timer_d_cb)
{
}

TIMER_DEVICE_CALLBACK_MEMBER(generalplus_gpl951xx_device::timer_e_cb)
{
}

TIMER_DEVICE_CALLBACK_MEMBER(generalplus_gpl951xx_device::timer_f_cb)
{
}


template<int Port>
u16 generalplus_gpl951xx_device::io_data_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_data_r\n", machine().describe_context(), 'a' + Port);
	return m_port_in[Port]();
}

template<int Port>
void generalplus_gpl951xx_device::io_data_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_data_w %04x\n", machine().describe_context(), 'a' + Port, data);
	m_port_out[Port](data);
}

template<int Port>
u16 generalplus_gpl951xx_device::io_buffer_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_buffer_r\n", machine().describe_context(), 'a' + Port);
	return m_port_in[Port]();
}

template<int Port>
void generalplus_gpl951xx_device::io_buffer_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_buffer_w %04x\n", machine().describe_context(), 'a' + Port, data);
	m_port_out[Port](data);
}

template<int Port>
u16 generalplus_gpl951xx_device::io_dir_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_dir_r\n", machine().describe_context(), 'a' + Port);
	return m_io_dir[Port];
}

template<int Port>
void generalplus_gpl951xx_device::io_dir_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_dir_w %04x\n", machine().describe_context(), 'a' + Port, data);
	m_io_dir[Port] = data;
}

template<int Port>
u16 generalplus_gpl951xx_device::io_attrib_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_attrib_r\n", machine().describe_context(), 'a' + Port);
	return m_io_attrib[Port];
}

template<int Port>
void generalplus_gpl951xx_device::io_attrib_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_attrib_w %04x\n", machine().describe_context(), 'a' + Port, data);
	m_io_attrib[Port] = data;
}

template<int Port>
u16 generalplus_gpl951xx_device::io_drv_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_drv_r\n", machine().describe_context(), 'a' + Port);
	return 0xffff;
}

template<int Port>
void generalplus_gpl951xx_device::io_drv_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_drv_w %04x\n", machine().describe_context(), 'a' + Port, data);
}

template<int Port>
u16 generalplus_gpl951xx_device::io_mux_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_mux_r\n", machine().describe_context(), 'a' + Port);
	return 0xffff;
}

template<int Port>
void generalplus_gpl951xx_device::io_mux_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_mux_w %04x\n", machine().describe_context(), 'a' + Port, data);
}

template<int Port>
u16 generalplus_gpl951xx_device::io_latch_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_latch_r\n", machine().describe_context(), 'a' + Port);
	return 0xffff;
}

template<int Port>
void generalplus_gpl951xx_device::io_latch_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_latch_w %04x\n", machine().describe_context(), 'a' + Port, data);
}

template<int Port>
u16 generalplus_gpl951xx_device::io_keyen_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_keyen_r\n", machine().describe_context(), 'a' + Port);
	return 0xffff;
}

template<int Port>
void generalplus_gpl951xx_device::io_keyen_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::io_%c_keyen_w %04x\n", machine().describe_context(), 'a' + Port, data);
}

// Misc

u16 generalplus_gpl951xx_device::sys_ctrl_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::sys_ctrl_r\n", machine().describe_context());
	return m_sys_ctrl;
}

void generalplus_gpl951xx_device::sys_ctrl_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::sys_ctrl_w %04x\n", machine().describe_context(), data);
	m_sys_ctrl = data;
}

void generalplus_gpl951xx_device::clock_ctrl_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::clock_ctrl_w %04x\n", machine().describe_context(), data);
	m_clock_ctrl = data;
}

u16 generalplus_gpl951xx_device::clk_ctrl0_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::clk_ctrl0_r\n", machine().describe_context());
	return 0x0000;
}

void generalplus_gpl951xx_device::clk_ctrl0_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::clk_ctrl0 %04x\n", machine().describe_context(), data);
}

void generalplus_gpl951xx_device::watchdog_ctrl_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::watchdog_ctrl_w %04x\n", machine().describe_context(), data);
}

u16 generalplus_gpl951xx_device::power_state_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::power_state_r\n", machine().describe_context());
	return 0x0002;
}

u16 generalplus_gpl951xx_device::pllclkwait_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::pllclkwait_r\n", machine().describe_context());
	return 0x0000;
}

void generalplus_gpl951xx_device::pllclkwait_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::pllclkwait_w %04x\n", machine().describe_context(), data);
}

// sets bit 0x0002 then expects it to have cleared
u16 generalplus_gpl951xx_device::cache_ctrl_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::cache_ctrl_r\n", machine().describe_context());
	return m_cache_ctrl & ~ 0x0002;
}

void generalplus_gpl951xx_device::cache_ctrl_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::cache_ctrl_w %04x\n", machine().describe_context(), data);
	m_cache_ctrl = data;
}

// P_INT_Status1
//
// 15  KEYIF  - Key-change interrupt status (R/W)
// 14  ADCRIF - Line ADC Converstion Ready interrupt status
// 13  PwrONIF - Power on key change event status
// 12  OTIF    - Over temperature event status
//
// 11  UARTIF  - UART Interrupt status
// 10  SPI0    - Serial Peripheral Interface 0 interrupt status
//  9  FPIF    - FP Interrupt Status (LCD)
//  8
//
//  7  ASIF    - Line ADC Auto Sample Mode FIFO full interrupt status
//  6
//  5  AUDBIF  - Audio Channel B (CHB) FIFO Empty Interrupt status
//  4  AUDAIF  - Audio Channel A (CHA) FIFO Empty Interrupt status
//
//  3  USB     - USB interrupt status
//  2  DMA     - System DMA interrupt status
//  1  EXT1IF  - External interrupt 1 status (R/W)
//  0  EXT0IF  - External interrupt 0 status (R/W)

u16 generalplus_gpl951xx_device::int_status1_r()
{
	u16 ret = 0x0000;
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::int_status1_r\n", machine().describe_context());

	if (m_gpl_chx->is_cha_fifo_empty_irq())
		ret |= 0x0010;

	if (m_gpl_chx->is_chb_fifo_empty_irq())
		ret |= 0x0020;

	return ret;
}

void generalplus_gpl951xx_device::int_status1_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::int_status1_w %04x\n", machine().describe_context(), data);
	//m_int_status1 = data;
}

// P_INT_Status2
//
// 15
// 14
// 13
// 12
//
// 11  KSIF - Key-Scan Interrupt status
// 10  TMBCIF - TimebaseC interrupt status
//  9  TMBBIF - TimebaseB interrupt status
//  8  TMBAIF - TimebaseA interrupt status
//
//  7  CTSIF  - CTS Controller interrupt status
//  6
//  5  I2C - I2C Controller interrupt status
//  4  NAND - NAND Flash Controller interrupt status
//
//  3
//  2 SPI1    - Serial Peripheral Interface 1 interrupt status
//  1 RTC     - RTC Controller interrupt status
//  0 ADCFF   - MIC ADC Auto Sample Mode FIFO Full interrupt status

u16 generalplus_gpl951xx_device::int_status2_r()
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::int_status2_r\n", machine().describe_context());
	u16 ret = 0;

	if (m_gpl_timebase->timebasea_irq_flag())
		ret |= 0x0100;

	if (m_gpl_timebase->timebaseb_irq_flag())
		ret |= 0x0200;

	if (m_gpl_timebase->timebasec_irq_flag())
		ret |= 0x0400;

	return ret;
}

void generalplus_gpl951xx_device::int_status2_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::int_status2_w %04x\n", machine().describe_context(), data);
	// none of this bits are listed as writeable for GPL95xx or GPL162xx
}

// P_INT_Status3
//
// 15  TMHIF - TimerH Interrupt status
// 14  TMGIF - TimerG Interrupt status
// 13  TMFIF - TimerF Interrupt status
// 12  TMEIF - TimerE Interrupt status
//
// 11  TMDIF - TimerD Interrupt status
// 10  TMCIF - TimerC Interrupt status
//  9  TMBIF - TimerB Interrupt status
//  8  TMAIF - TimerA Interrupt status
//
//  7  UNEXP - unexpected access Interrupt status
//  6
//  5
//  4
//
//  3
//  2 BEAT - SPU beat interrupt status (R/W)
//  1
//  0

u16 generalplus_gpl951xx_device::int_status3_r()
{
	u16 ret = 0;
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::int_status3_r\n", machine().describe_context());

	if (m_timerh_ctrl & 0x8000)
		ret |= 0x8000;

	if (m_timerg_ctrl & 0x8000)
		ret |= 0x4000;

	return ret;
}

void generalplus_gpl951xx_device::int_status3_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::int_status3_w %04x\n", machine().describe_context(), data);
	// bit 2 (SPU Beat Interrupt) is listed as R/W for GPL95, but not GPL162? (verify)
}

void generalplus_gpl951xx_device::int_priority_1_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::int_priority_1_w %04x\n", machine().describe_context(), data);
	m_int_priority_1 = data;
}

void generalplus_gpl951xx_device::int_priority_2_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::int_priority_2_w %04x\n", machine().describe_context(), data);
	m_int_priority_2 = data;
}

void generalplus_gpl951xx_device::int_priority_3_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::int_priority_3_w %04x\n", machine().describe_context(), data);
	m_int_priority_3 = data;
}

void generalplus_gpl951xx_device::mint_ctrl_w(u16 data)
{
	LOGMASKED(LOG_OTHER, "%s: generalplus_gpl951xx_device::mint_ctrl_w %04x\n", machine().describe_context(), data);
	m_misc_int_ctrl = data;
}


u16 generalplus_gpl951xx_device::spi_direct_r(offs_t offset)
{
	// The GPL951xx chips can see a bank of SPI memory as a flat space
	// as they will automatically generate SPI signals for random memory
	// access on demand
	//
	// Emulating it this way is impractical for emulation (DASM wouldn't
	// work) so we just present it as a flat space directly.
	if (!m_spiregion)
		return 0x0000;

	u16 ret = (m_spiregion[((offset * 2) + 0) & (m_spisize-1)]) | (m_spiregion[((offset * 2) + 1) & (m_spisize-1)] << 8);
	return ret;
}

u16 generalplus_gpl951xx_device::spi_direct_bank_r(offs_t offset)
{
	if (!m_spiregion)
		return 0x0000;

	offset += 0x1f7000;
	offset += (m_spi_bank & 0x3f) * 0x200000;

	u16 ret = (m_spiregion[((offset * 2) + 0) & (m_spisize-1)]) | (m_spiregion[((offset * 2) + 1) & (m_spisize-1)] << 8);
	return ret;

}

template<int Port>
void generalplus_gpl951xx_device::add_port(address_map &map, u32 base)
{
	map(base + 0x0, base + 0x0).rw(FUNC(generalplus_gpl951xx_device::io_data_r<Port>), FUNC(generalplus_gpl951xx_device::io_data_w<Port>));
	map(base + 0x1, base + 0x1).rw(FUNC(generalplus_gpl951xx_device::io_buffer_r<Port>), FUNC(generalplus_gpl951xx_device::io_buffer_w<Port>));
	map(base + 0x2, base + 0x2).rw(FUNC(generalplus_gpl951xx_device::io_dir_r<Port>), FUNC(generalplus_gpl951xx_device::io_dir_w<Port>));
	map(base + 0x3, base + 0x3).rw(FUNC(generalplus_gpl951xx_device::io_attrib_r<Port>), FUNC(generalplus_gpl951xx_device::io_attrib_w<Port>));
	map(base + 0x4, base + 0x4).rw(FUNC(generalplus_gpl951xx_device::io_drv_r<Port>), FUNC(generalplus_gpl951xx_device::io_drv_w<Port>));
	map(base + 0x5, base + 0x5).rw(FUNC(generalplus_gpl951xx_device::io_mux_r<Port>), FUNC(generalplus_gpl951xx_device::io_mux_w<Port>));
	map(base + 0x6, base + 0x6).rw(FUNC(generalplus_gpl951xx_device::io_latch_r<Port>), FUNC(generalplus_gpl951xx_device::io_latch_w<Port>));
	map(base + 0x7, base + 0x7).rw(FUNC(generalplus_gpl951xx_device::io_keyen_r<Port>), FUNC(generalplus_gpl951xx_device::io_keyen_w<Port>));
}

void generalplus_gpl951xx_device::gpspi_direct_internal_map(address_map &map)
{
	map(0x000000, 0x0027ff).ram().share("mainram");

	map(0x007000, 0x007007).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap2_regs_r), FUNC(gcm394_base_video_device::tmap2_regs_w)); // 7000 - Tx3_X_Position

	map(0x007010, 0x007015).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap0_regs_r), FUNC(gcm394_base_video_device::tmap0_regs_w));
	map(0x007016, 0x00701b).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap1_regs_r), FUNC(gcm394_base_video_device::tmap1_regs_w));
	map(0x00701c, 0x00701c).w(m_spg_video, FUNC(gcm394_base_video_device::vcomp_value_w)); // 701c - VComValue
	map(0x00701d, 0x00701d).w(m_spg_video, FUNC(gcm394_base_video_device::vcomp_offset_w)); // 701d - VComOffset
	map(0x00701e, 0x00701e).w(m_spg_video, FUNC(gcm394_base_video_device::vcomp_step_w)); // 701e - VComStep

	map(0x007020, 0x007020).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap0_tilebase_lsb_r), FUNC(gcm394_base_video_device::tmap0_tilebase_lsb_w));           // 7020 - Segment_Tx1
	map(0x007021, 0x007021).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap1_tilebase_lsb_r), FUNC(gcm394_base_video_device::tmap1_tilebase_lsb_w));           // 7021 - Segment_Tx2
	map(0x007022, 0x007022).rw(m_spg_video, FUNC(gcm394_base_video_device::sprite_7022_gfxbase_lsb_r), FUNC(gcm394_base_video_device::sprite_7022_gfxbase_lsb_w)); // 7022 - Segment_sp
	map(0x007023, 0x007023).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap2_tilebase_lsb_r), FUNC(gcm394_base_video_device::tmap2_tilebase_lsb_w));           // 7023 - Segment_Tx3

	//
	//
	//
	//
	//
	map(0x00702a, 0x00702a).w(m_spg_video, FUNC(gcm394_base_video_device::blending_w)); // 702a - Blending
	map(0x00702b, 0x00702b).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap0_tilebase_msb_r), FUNC(gcm394_base_video_device::tmap0_tilebase_msb_w));           // 702b - Segment_Tx1H
	map(0x00702c, 0x00702c).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap1_tilebase_msb_r), FUNC(gcm394_base_video_device::tmap1_tilebase_msb_w));           // 702c - Segment_Tx2H
	map(0x00702d, 0x00702d).rw(m_spg_video, FUNC(gcm394_base_video_device::sprite_702d_gfxbase_msb_r), FUNC(gcm394_base_video_device::sprite_702d_gfxbase_msb_w)); // 702d - Segment_spH
	map(0x00702e, 0x00702e).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap2_tilebase_msb_r), FUNC(gcm394_base_video_device::tmap2_tilebase_msb_w));           // 702e - Segment_Tx3H

	//
	map(0x007030, 0x007030).rw(m_spg_video, FUNC(gcm394_base_video_device::video_7030_brightness_r), FUNC(gcm394_base_video_device::video_7030_brightness_w)); // 7030 - Fade_Control
	//
	map(0x00703a, 0x00703a).rw(m_spg_video, FUNC(gcm394_base_video_device::video_703a_palettebank_r), FUNC(gcm394_base_video_device::video_703a_palettebank_w)); // 703a - Palette_Control
	//
	map(0x007042, 0x007042).rw(m_spg_video, FUNC(gcm394_base_video_device::sprite_7042_extra_r), FUNC(gcm394_base_video_device::sprite_7042_extra_w)); // 7042 - SControl
	//
	map(0x007050, 0x007050).w(FUNC(generalplus_gpl951xx_device::tft_ctrl_w)); // 7050 - TFT_Ctrl
	// 7051 - TFT_V_Width
	// 7052 - TFT_VSync_Setup
	// 7053 - TFT_V_Start
	// 7054 - TFT_V_End
	// 7055 - TFT_H_Width
	// 7056 - TFT_HSync_Setup
	// 7057 - TFT_H_Start
	// 7058 - TFT_H_End
	// 7059 - TFT_RGB_Ctrl
	map(0x00705a, 0x00705a).r(FUNC(generalplus_gpl951xx_device::tft_status_r)); // 705a - TFT_Status
	map(0x00705b, 0x00705b).w(FUNC(generalplus_gpl951xx_device::tft_memmode_wcmd_w)); // 705b - TFT_MemMode_WCmd
	// 705c - TFT_MemMode_RCmd
	//
	// 705e - STN_PIC_SEG
	// 705f - STN_Ctrl1
	//
	map(0x007062, 0x007062).rw(m_spg_video, FUNC(gcm394_base_video_device::videoirq_source_enable_r), FUNC(gcm394_base_video_device::videoirq_source_enable_w));             // 7062 - TFT_INT_EN
	map(0x007063, 0x007063).rw(m_spg_video, FUNC(gcm394_base_video_device::video_7063_videoirq_source_r), FUNC(gcm394_base_video_device::video_7063_videoirq_source_ack_w)); // 7063 - TFT_INT_CLR
	// 7064 - US_Ctrl
	// 7065 - US_Hscaling
	// 7066 - US_Vscaling
	// 7067 - US_Width
	// 7068 - US_Height
	// 7069 - US_Hoffset
	// 706a - US_Voffset
	//
	// 706c - TFT_V_Show_Start
	// 706d - TFT_V_Show_End
	// 706e - TFT_H_Show_Start
	// 706f - TFT_H_Show_End
	//
	map(0x007070, 0x007070).w(m_spg_video, FUNC(gcm394_base_video_device::video_dma_source_w)); // 7070 - SPDMA_Source
	map(0x007071, 0x007071).w(m_spg_video, FUNC(gcm394_base_video_device::video_dma_dest_w));   // 7071 - SPDMA_Target
	map(0x007072, 0x007072).rw(m_spg_video, FUNC(gcm394_base_video_device::video_dma_size_busy_r), FUNC(gcm394_base_video_device::video_dma_size_trigger_w)); // 7072 - SPDMA_Number
	// 7073 - HB_Ctrl
	// 7074 - HB_GO
	//
	// 707d - BLD_Color
	//
	map(0x00707e, 0x00707e).w(m_spg_video, FUNC(gcm394_base_video_device::ppu_ram_bank_w));    // 707e - PPU_RAM_BANK
	map(0x00707f, 0x00707f).rw(m_spg_video, FUNC(gcm394_base_video_device::ppu_enable_r), FUNC(gcm394_base_video_device::ppu_enable_w));// 707f - PPU_Enable
	//
	// 7080 - STN_SEG
	// 7081 - STN_COM
	// 7082 - STN_PIC_COM
	// 7083 - STN_CPWait
	// 7084 - STN_Ctrl2
	// 7085 - STN_GTG_SEG
	// 7086 - STN_GTG_COM
	//
	// 70b4 - Tx1_N_PTRH
	// 70b5 - Tx1_A_PTRH
	// 70b6 - Tx2_N_PTRH
	// 70b7 - Tx2_A_PTRH
	// 70b8 - Tx3_N_PTRH
	// 70b9 - Tx3_A_PTRH
	//
	// 70db - Free_Height
	// 70dc - Free_Width
	//
	map(0x0070e0, 0x0070e0).r(m_spg_video, FUNC(gcm394_base_video_device::video_70e0_prng_r)); // 70e0 - Random0 (15-bit)
	// 70e1 - Random1 (15-bit)
	//
	map(0x007100, 0x0071ff).ram().share("rowscroll"); // 7100 to 71ff - Tx_Hvoffset
	map(0x007200, 0x0072ff).ram().share("rowzoom"); // 7200 to 72ff - HCMValue
	map(0x007300, 0x0073ff).rw(m_spg_video, FUNC(gcm394_base_video_device::palette_r), FUNC(gcm394_base_video_device::palette_w)); // 7300 to 73ff - Palette (banked)
	map(0x007400, 0x0077ff).rw(m_spg_video, FUNC(gcm394_base_video_device::spriteram_r), FUNC(gcm394_base_video_device::spriteram_w)); // 7400 to 74ff - Sprite Ram (banked)
	//
	// 7800 - BodyID
	// 7801 - unused
	// 7802 - PwrKey_State
	map(0x007803, 0x007803).rw(FUNC(generalplus_gpl951xx_device::sys_ctrl_r), FUNC(generalplus_gpl951xx_device::sys_ctrl_w)); // 7803 - SYS_CTRL (seems quite different between SoCs)
	map(0x007804, 0x007804).rw(FUNC(generalplus_gpl951xx_device::clk_ctrl0_r), FUNC(generalplus_gpl951xx_device::clk_ctrl0_w)); // 7804 - CLK_Ctrl0
	// 7805 - CLK_Ctrl1
	// 7806 - Reset_Flag
	map(0x007807, 0x007807).w(FUNC(generalplus_gpl951xx_device::clock_ctrl_w)); // 7807 - Clock_Ctrl
	// 7808 - LVR_Ctrl
	map(0x00780a, 0x00780a).w(FUNC(generalplus_gpl951xx_device::pm_ctrl_w)); // 7809 - PM_Ctrl
	map(0x00780a, 0x00780a).w(FUNC(generalplus_gpl951xx_device::watchdog_ctrl_w)); // 780a - Watchdog_Ctrl
	map(0x00780b, 0x00780b).nopw(); // Watchdog_Clear
	// 780c - WAIT
	// 780d - HALT
	// 780e - unused
	map(0x00780f, 0x00780f).r(FUNC(generalplus_gpl951xx_device::power_state_r)); // 780f - Power_State
	map(0x007810, 0x007810).rw(FUNC(generalplus_gpl951xx_device::spi_bank_r), FUNC(generalplus_gpl951xx_device::spi_bank_w)); // 7810 - BankSwitch
	// 7811 - unused
	// 7812 - unused
	// 7813 - unused
	// 7814 - unused
	// 7815 - unused
	// 7816 - unused
	map(0x007817, 0x007817).rw(FUNC(generalplus_gpl951xx_device::pllsel_r), FUNC(generalplus_gpl951xx_device::pllsel_w)); // 7817 - PLL_Sel
	map(0x007818, 0x007818).rw(FUNC(generalplus_gpl951xx_device::pllclkwait_r), FUNC(generalplus_gpl951xx_device::pllclkwait_w)); // 7818 - PLLWaitCLK
	map(0x007819, 0x007819).rw(FUNC(generalplus_gpl951xx_device::cache_ctrl_r), FUNC(generalplus_gpl951xx_device::cache_ctrl_w)); // 7819 - Cache_Ctrl
	// 781a - Cache_HitRate
	// 781b - unused
	// 781c - unused
	// 781d - unused
	// 781e - unused
	// 781f - SYS_Misc

	// 7825 - Unexpect_Flag

	// 7830 - CHECKSUM0_LB
	// 7831 - CHECKSUM1_LB
	// 7832 - CHECKSUM0_HB
	// 7833 - CHECKSUM1_HB
	//
	map(0x007840, 0x007840).rw(FUNC(generalplus_gpl951xx_device::spi_improve_r), FUNC(generalplus_gpl951xx_device::spi_improve_w));// 7840 - P_SPI_Improve
	//
	// 7848 - ECC_LPRL_HB
	// 7849 - ECC_LPRH_HB
	// 784a - ECC_CPR_HB
	// 784b - ECC_LPR_CKL_HB
	// 784c - ECC_LPR_CKH_HB
	// 784d - ECC_CPCKR_HB
	// 784e - ECC_ERR0_HB
	// 784f - ECC_ERR1_HB

	// 7850 - NF_Ctrl
	// 7851 - NF_CMD
	// 7852 - NF_AddrL
	// 7853 - NF_AddrH
	// 7854 - NF_Data
	// 7855 - NF_INT_Ctrl
	// 7856 - unused            or BCH_Control
	// 7857 - ECC_Ctrl
	// 7858 - ECC_LPRL_LB     or BCH_Error
	// 7859 - ECC_LPRH_LB     or BCH_Parity0
	// 785a - ECC_CPR_LB      or BCH_Parity1
	// 785b - ECC_LPR_CKL_LB  or BCH_Parity2
	// 785c - ECC_LPR_CKH_LB  or BCH_Parity3
	// 785d - ECC_CPCKR_LB    or BCH_Parity4
	// 785e - ECC_ERR0_LB     or BCH_Parity5
	// 785f - ECC_ERR1_LB     or BCH_Parity6

	// Ports addresses on GPL951xx have more logical arrangement compared to GPL162xx
	// and there is an additional 'Port F'

	add_port<0>(map, 0x007860); // 0x7860 - 0x7867 - Port A
	add_port<1>(map, 0x007868); // 0x7868 - 0x786f - Port B
	add_port<2>(map, 0x007870); // 0x7870 - 0x7877 - Port C
	add_port<3>(map, 0x007878); // 0x7878 - 0x787f - Port D
	add_port<4>(map, 0x007880); // 0x7880 - 0x7887 - Port E
	add_port<5>(map, 0x007888); // 0x7888 - 0x788f - Port F

	map(0x0078a0, 0x0078a0).rw(FUNC(generalplus_gpl951xx_device::int_status1_r), FUNC(generalplus_gpl951xx_device::int_status1_w)); // 78a0 - INT_Status1
	map(0x0078a1, 0x0078a1).rw(FUNC(generalplus_gpl951xx_device::int_status2_r), FUNC(generalplus_gpl951xx_device::int_status2_w)); // 78a1 - INT_Status2
	map(0x0078a2, 0x0078a2).rw(FUNC(generalplus_gpl951xx_device::int_status3_r), FUNC(generalplus_gpl951xx_device::int_status3_w)); // 78a2 - INT_Status3
	map(0x0078a3, 0x0078a3).w(FUNC(generalplus_gpl951xx_device::int_priority_1_w)); // 78a3 - INT_Priority1
	map(0x0078a4, 0x0078a4).w(FUNC(generalplus_gpl951xx_device::int_priority_2_w)); // 78a4 - INT_Priority2
	map(0x0078a5, 0x0078a5).w(FUNC(generalplus_gpl951xx_device::int_priority_3_w)); // 78a5 - INT_Priority3
	map(0x0078a6, 0x0078a6).w(FUNC(generalplus_gpl951xx_device::mint_ctrl_w)); // 78a6 - MINT_Ctrl
	// 78a7 - IOAB_KCIEN
	// 78a8 - IOC_KCIEN
	// 78a9 - IOE_KCIEN
	// 78aa - IOF_KCIEN
	// 78ab - IOAB_KCIFC
	// 78ac - IOC_ KCIFC
	// 78ad - IOE_ KCIFC
	// 78ae - IOF_ KCIFC

	map(0x0078b0, 0x0078b0).rw(m_gpl_timebase, FUNC(gpl_timebase_device::timebasea_ctrl_r), FUNC(gpl_timebase_device::timebasea_ctrl_w));  // 78b0 TimeBase A Control Register (P_TimeBaseA_Ctrl)
	map(0x0078b1, 0x0078b1).rw(m_gpl_timebase, FUNC(gpl_timebase_device::timebaseb_ctrl_r), FUNC(gpl_timebase_device::timebaseb_ctrl_w));  // 78b1 TimeBase B Control Register (P_TimeBaseB_Ctrl)
	map(0x0078b2, 0x0078b2).rw(m_gpl_timebase, FUNC(gpl_timebase_device::timebasec_ctrl_r), FUNC(gpl_timebase_device::timebasec_ctrl_w));  // 78b2 TimeBase C Control Register (P_TimeBaseC_Ctrl)

	map(0x0078b8, 0x0078b8).w(m_gpl_timebase, FUNC(gpl_timebase_device::timebase_reset_w)); // 78b8 - TimeBase_Reset

	// 78c0 - I2C_Ctrl
	// 78c1 - I2C_Status
	// 78c2 - I2C_Address
	// 78c3 - I2C_Data
	// 78c4 - I2C_Debounce
	// 78c5 - I2C_Clk
	// 78c6 - I2C_MISC

	map(0x0078e0, 0x0078e0).rw(FUNC(generalplus_gpl951xx_device::timerg_ctrl_r), FUNC(generalplus_gpl951xx_device::timerg_ctrl_w)); // 78e0 - timerg_Ctrl
	// 78e1
	map(0x0078e2, 0x0078e2).rw(FUNC(generalplus_gpl951xx_device::timerg_preload_r), FUNC(generalplus_gpl951xx_device::timerg_preload_w)); // 78e2 - timerg_Preload
	// 78e3
	// 78e4 - TimerG_UpCount
	// 78e5
	// 78e6
	// 78e7
	map(0x0078e8, 0x0078e8).rw(FUNC(generalplus_gpl951xx_device::timerh_ctrl_r), FUNC(generalplus_gpl951xx_device::timerh_ctrl_w)); // timerh_Ctrl
	// 78e9
	map(0x0078ea, 0x0078ea).rw(FUNC(generalplus_gpl951xx_device::timerh_preload_r), FUNC(generalplus_gpl951xx_device::timerh_preload_w)); // 78ea - timerh_Preload
	// 78eb
	// 78ec - TimerH_UpCount
	// 78ed
	// 78ee
	// 78ef

	map(0x0078f0, 0x0078f0).rw(m_gpl_chx, FUNC(gpl_chx_device::cha_ctrl_r), FUNC(gpl_chx_device::cha_ctrl_w)); // 78f0 - CHA_Ctrl
	map(0x0078f1, 0x0078f1).rw(m_gpl_chx, FUNC(gpl_chx_device::cha_data_r), FUNC(gpl_chx_device::cha_data_w)); // 78f1 - CHA_Data
	map(0x0078f2, 0x0078f2).rw(m_gpl_chx, FUNC(gpl_chx_device::cha_fifo_r), FUNC(gpl_chx_device::cha_fifo_w)); // 78f2 - CHA_FIFO
	// 78f3
	// 78f4
	// 78f5
	// 78f6
	// 78f7
	map(0x0078f8, 0x0078f8).rw(m_gpl_chx, FUNC(gpl_chx_device::chb_ctrl_r), FUNC(gpl_chx_device::chb_ctrl_w)); // 78f8 - CHB_Ctrl
	map(0x0078f9, 0x0078f9).rw(m_gpl_chx, FUNC(gpl_chx_device::chb_data_r), FUNC(gpl_chx_device::chb_data_w)); // 78f9 - CHB_Data
	map(0x0078fa, 0x0078fa).rw(m_gpl_chx, FUNC(gpl_chx_device::chb_fifo_r), FUNC(gpl_chx_device::chb_fifo_w)); // 78fa - CHB_FIFO
	// 78fb
	// 78fc
	// 78fd
	// 78fe
	// 78ff

	// 7900 - UART_Data
	// 7901 - UART_RXStatus
	// 7902 - UAR_Ctrl
	// 7903 - UART_BaudRate
	// 7904 - UART_Status
	// 7905 - UART_FIFO
	// 7906 - UART_TXDelay

	// 7920 - SPI1_Ctrl
	// 7921 - SPI1_TXStatus
	// 7922 - SPI1_TXData
	// 7923 - SPI1_RXStatus
	// 7924 - SPI1_RXData
	// 7925 - SPI1_Misc

	// 7940 - SPI0_Ctrl
	// 7941 - SPI0_TXStatus
	// 7942 - SPI0_TXData
	// 7943 - SPI0_RXStatus
	// 7944 - SPI0_RXData
	// 7945 - SPI0_Misc

	// 79a0 - ADC_Setup
	// 79a1 - MADC_Ctrl
	// 79a2 - MADC_Data
	// 79a3 - ASADC_Ctrl
	// 79a4 - ASDAC_Data
	// 79a5
	// 79a6 - ADC_LineCH_En
	// 79a7 - ADC_SH_Wait

	// 79b0 - MICADC_Setup
	// 79b1 - MICGAIN_Ctrl
	// 79b2
	// 79b3 - ASMICADC_Ctrl
	// 79b4 - ASMICDAC_Data
	// 79b5 - MICAGC_UpThres
	// 79b6
	// 79b7 - MICADC_SH_WAIT
	// 79b8 - MICADC_DataMAX
	// 79b9 - MICADC_DataMIN
	// 79ba - MICADC_FLAG
	// 79bb - MICADC_GAIN
	// 79bc - MICAGC_Ctrl
	// 79bd - MICAGC_Time
	// 79be - MICAGC_Enable
	// 79bf - MICAGC_Status

	map(0x0079f0, 0x0079f0).w(m_rtc, FUNC(gpl951xx_rtc_device::rtc_ctrl_w)); // 79f0 - RTC_Ctrl
	map(0x0079f1, 0x0079f1).w(m_rtc, FUNC(gpl951xx_rtc_device::rtc_addr_w)); // 79f1 - RTC_Addr
	map(0x0079f2, 0x0079f2).w(m_rtc, FUNC(gpl951xx_rtc_device::rtc_writedata_w)); // 79f2 - RTC_WriteData
	map(0x0079f3, 0x0079f3).w(m_rtc, FUNC(gpl951xx_rtc_device::rtc_request_w)); // 79f3 - RTC_Request
	map(0x0079f4, 0x0079f4).r(m_rtc, FUNC(gpl951xx_rtc_device::rtc_ready_r)); // RTC_Ready
	map(0x0079f5, 0x0079f5).r(m_rtc, FUNC(gpl951xx_rtc_device::rtc_readdata_r)); // RTC_ReadData
	// 79f6
	// 79f7
	// 79f8
	// 79fa
	// 79fb - RTC_ClkDiv

	// 7a00 - TimerA_Ctrl
	// 7a01 - TimerA_CCPB_Ctrl
	// 7a02 - TimerA_Preload
	// 7a03 - TimerA_CCPB_Reg
	// 7a04 - TimerA_UpCount

	// 7a08 - TimerB_Ctrl
	// 7a09 - TimerB_CCPB_Ctrl
	// 7a0a - TimerB_Preload
	// 7a0b - TimerB_CCPB_Reg
	// 7a0c - TimerB_UpCount

	// 7a10 - TimerC_Ctrl
	// 7a11 - TimerC_CCPB_Ctrl
	// 7a12 - TimerC_Preload
	// 7a13 - TimerC_CCPB_Reg
	// 7a14 - TimerC_UpCount

	// 7a18 - TimerD_Ctrl
	// 7a19 - TimerD_CCPB_Ctrl
	// 7a1a - TimerD_Preload
	// 7a1b - TimerD_CCPB_Reg
	// 7a1c - TimerD_UpCount

	// 7a20 - TimerE_Ctrl
	// 7a21 - TimerF_Ctrl
	// 7a22 - TimerE_CCPB_Ctrl
	// 7a23 - TimerF_CCPB_Ctrl
	// 7a24 - TimerE_Preload
	// 7a25 - TimerF_Preload
	// 7a26 - TimerEF_CCPB4_Reg
	// 7a27 - TimerEF_CCPB5_Reg
	// 7a28 - TimerEF_CCPB6_Reg
	// 7a29 - TimerEF_CCPB7_Reg
	// 7a2a - TimerE_UpCount
	// 7a2b - TimerF_UpCount
	// 7a2c - TimerEF_CCPB_Se

	// 7a40 - USBD_Config
	// 7a41 - USBD_Function
	// 7a42 - USBD_PMR
	// 7a43 - USBD_EP0Data
	// 7a44 - USBD_BIData
	// 7a45 - USBD_BOData
	// 7a46 - USBD_INTINData
	// 7a47 - USBD_EPEvent
	// 7a48 - USBD_GLOINT
	// 7a49 - USBD_INTEN
	// 7a4a - USBD_INT
	// 7a4b - USBD_SCI NTEN
	// 7a4c - USBD_SCINT
	// 7a4d - USBD_EPAutoSet
	// 7a4e - USBD_EPSetStall
	// 7a4f - USBD_EPBufClear
	// 7a50 - USBD_EPEvntClear
	// 7a51 - USBD_EP0WrtCount
	// 7a52 - USBD_BOWrtCount
	// 7a53 - USBD_EP0BufPointer
	// 7a54 - USBD_BIBufPointer
	// 7a55 - USBD_BOBufPointer
	// 7a56 - USBD_EP0RTR
	// 7a57 - USBD_EP0RR
	// 7a58 - USBD_EP0VR
	// 7a59 - USBD_EP0IR
	// 7a5a - USBD_EP0LR
	//
	// 7a60 - USBD_DMAWrtCountL
	// 7a61 - USBD_DMAWrtCountH
	// 7a62 - USBD_DMAAckL
	// 7a63 - USBD_DMAAckH
	// 7a64 - USBD_EPStall
	//
	// 7a67 - USBD_Device
	// 7a68 - USBD_NullPkt
	// 7a69 - USBD_DMAINT
	//
	// 7a6c - USBD_INTF

	map(0x007a80, 0x007a87).rw(m_gpl_dma, FUNC(gpl_dma_device::system_dma_params_channel0_r), FUNC(gpl_dma_device::system_dma_params_channel0_w));
	map(0x007a88, 0x007a8f).rw(m_gpl_dma, FUNC(gpl_dma_device::system_dma_params_channel1_r), FUNC(gpl_dma_device::system_dma_params_channel1_w));
	//
	// 7ab0 - DMA_SPRISize0
	// 7ab1 - DMA_SPRISize1
	//
	// 7abd - DMA_LineLength
	map(0x007abe, 0x007abe).rw(m_gpl_dma, FUNC(gpl_dma_device::system_dma_memtype_r), FUNC(gpl_dma_device::system_dma_memtype_w)); // 7abe - DMA_SS
	map(0x007abf, 0x007abf).rw(m_gpl_dma, FUNC(gpl_dma_device::system_dma_status_r), FUNC(gpl_dma_device::system_dma_status_w)); // 7abf - DMA_INT
	//
	// 7ac0 - CTS_Ctrl1
	// 7ac1 - CTS_CH
	// 7ac2 - CTS_DIV
	// 7ac3 - CTS_CYCLE
	// 7ac4 - CTS_Ctrl2
	// 7ac5 - CTS_Status
	// 7ac6 - CTS_Ctrl3
	//
	// 7ac8 - CTS_FIFOLevel
	// 7ac9 - CTS_CNT

	map(0x007af0, 0x007af0).rw(FUNC(generalplus_gpl951xx_device::byte_swap_r), FUNC(generalplus_gpl951xx_device::byte_swap_w)); // Byte_Swap
	// 7af1 - Nibble_Swap
	// 7af2 - TwoBit_Swap
	// 7af3 - Bit_Reverse

	// 7b20 - KS_Ctrl1
	// 7b21 - KS_Ctrl2
	//
	// 7b28 - KS_Data0
	// 7b29 - KS_Data1
	// 7b2a - KS_Data2
	// 7b2b - KS_Data3
	// 7b2c - KS_Data4
	// 7b2d - KS_Data5
	// 7b2e - KS_Data6
	// 7b2f - KS_Data7
	// 7b30 - KS_Data8
	// 7b31 - KS_Data9
	// 7b32 - KS_Data10

	map(0x007b40, 0x007b40).rw(FUNC(generalplus_gpl951xx_device::spifc_ctrl_r), FUNC(generalplus_gpl951xx_device::spifc_ctrl_w)); // SPIFC_Ctrl1
	map(0x007b41, 0x007b41).rw(FUNC(generalplus_gpl951xx_device::spifc_cmd_r), FUNC(generalplus_gpl951xx_device::spifc_cmd_w)); // SPIFC_CMD
	map(0x007b42, 0x007b42).rw(FUNC(generalplus_gpl951xx_device::spifc_para_r), FUNC(generalplus_gpl951xx_device::spifc_para_w)); // SPIFC_PARA
	map(0x007b43, 0x007b43).rw(FUNC(generalplus_gpl951xx_device::spifc_addrl_r), FUNC(generalplus_gpl951xx_device::spifc_addrl_w)); // 7b43 - SPIFC_ADDRL
	map(0x007b44, 0x007b44).rw(FUNC(generalplus_gpl951xx_device::spifc_addrh_r), FUNC(generalplus_gpl951xx_device::spifc_addrh_w)); // 7b44 - SPIFC_ADDRH
	map(0x007b45, 0x007b45).rw(FUNC(generalplus_gpl951xx_device::spifc_txdat_r), FUNC(generalplus_gpl951xx_device::spifc_txdat_w)); // 7b45 - SPIFC_TX_Dat
	map(0x007b46, 0x007b46).rw(FUNC(generalplus_gpl951xx_device::spifc_rxdat_r), FUNC(generalplus_gpl951xx_device::spifc_rxdat_w)); // SPIFC_RX_Data - values must be written and read from here, but is there any transformation?
	map(0x007b47, 0x007b47).rw(FUNC(generalplus_gpl951xx_device::spifc_tx_bc_r), FUNC(generalplus_gpl951xx_device::spifc_tx_bc_w)); // SPIFC_TX_BC
	map(0x007b48, 0x007b48).rw(FUNC(generalplus_gpl951xx_device::spifc_rx_bc_r), FUNC(generalplus_gpl951xx_device::spifc_rx_bc_w)); // SPIFC_RX_BC
	map(0x007b49, 0x007b49).rw(FUNC(generalplus_gpl951xx_device::spifc_timing_r), FUNC(generalplus_gpl951xx_device::spifc_timing_w)); // SPIFC_TIMING
	// 7b4a
	map(0x007b4b, 0x007b4b).rw(FUNC(generalplus_gpl951xx_device::spifc_ctrl2_r), FUNC(generalplus_gpl951xx_device::spifc_ctrl2_w)); // 7b4b - SPIFC_Ctrl2


	map(0x007b80, 0x007bbf).rw(m_spg_audio, FUNC(sunplus_gcm394_audio_device::control_r), FUNC(sunplus_gcm394_audio_device::control_w));
	map(0x007c00, 0x007dff).rw(m_spg_audio, FUNC(sunplus_gcm394_audio_device::audio_r), FUNC(sunplus_gcm394_audio_device::audio_w));
	map(0x007e00, 0x007fff).rw(m_spg_audio, FUNC(sunplus_gcm394_audio_device::audio_phase_r), FUNC(sunplus_gcm394_audio_device::audio_phase_w));

	// 8000 - 8fff internal boot ROM (same on all devices of the same type, not OTP)

	map(0x009000, 0x1fffff).r(FUNC(generalplus_gpl951xx_device::spi_direct_r));
	map(0x200000, 0x3fffff).r(FUNC(generalplus_gpl951xx_device::spi_direct_bank_r));
}

void generalplus_gpl951xx_device::update_interrupts(int state)
{
	if (m_gpl_timebase->timebasea_irq_flag() || m_gpl_timebase->timebaseb_irq_flag())
	{
		set_state_unsynced(UNSP_IRQ7_LINE, ASSERT_LINE);
	}
	else
	{
		set_state_unsynced(UNSP_IRQ7_LINE, CLEAR_LINE);
	}

	if (m_gpl_timebase->timebasec_irq_flag())
	{
		set_state_unsynced(UNSP_IRQ6_LINE, ASSERT_LINE);
	}
	else
	{
		set_state_unsynced(UNSP_IRQ6_LINE, CLEAR_LINE);
	}

	if (((m_timerg_ctrl & 0x8000) && (m_timerg_ctrl & 0x4000)) ||
		((m_timerh_ctrl & 0x8000) && (m_timerh_ctrl & 0x4000)))
	{
		set_state_unsynced(UNSP_IRQ4_LINE, ASSERT_LINE);
	}
	else
	{
		set_state_unsynced(UNSP_IRQ4_LINE, CLEAR_LINE);
	}

	if ((m_gpl_chx->is_cha_fifo_empty_irq()) || (m_gpl_chx->is_chb_fifo_empty_irq()))
	{
		set_state_unsynced(UNSP_IRQ1_LINE, ASSERT_LINE);
	}
	else
	{
		set_state_unsynced(UNSP_IRQ1_LINE, CLEAR_LINE);
	}

}

u16 generalplus_gpl951xx_device::read_space(offs_t offset)
{
	address_space &space = this->space(AS_PROGRAM);
	u16 val = space.read_word(offset);
	return val;
}

void generalplus_gpl951xx_device::write_space(offs_t offset, u16 data)
{
	address_space &space = this->space(AS_PROGRAM);
	space.write_word(offset, data);
}

IRQ_CALLBACK_MEMBER(generalplus_gpl951xx_device::irq_vector_cb)
{
	if (irqline == UNSP_IRQ4_LINE)
		set_state_unsynced(UNSP_IRQ4_LINE, CLEAR_LINE);

	return 0;
}

void generalplus_gpl951xx_device::audioirq_w(int state)
{
	//set_state_unsynced(UNSP_IRQ5_LINE, state);
}

void generalplus_gpl951xx_device::videoirq_w(int state)
{
	set_state_unsynced(UNSP_IRQ5_LINE, state);
}

void generalplus_gpl951xx_device::device_add_mconfig(machine_config &config)
{
	SUNPLUS_GCM394_AUDIO(config, m_spg_audio, DERIVED_CLOCK(1, 1));
	m_spg_audio->write_irq_callback().set(FUNC(generalplus_gpl951xx_device::audioirq_w));
	m_spg_audio->space_read_callback().set(FUNC(generalplus_gpl951xx_device::read_space));
	m_spg_audio->add_route(0, *this, 1.0, 0);
	m_spg_audio->add_route(1, *this, 1.0, 1);

	// technically I think this gets mixed down to the same 'DAC' as the regular audio output
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac0, 0).add_route(0, *this, 1.0, 0);
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac1, 0).add_route(0, *this, 1.0, 1);

	GPL_DMA(config, m_gpl_dma, 0);
	m_gpl_dma->space_read_callback().set(FUNC(generalplus_gpl951xx_device::read_space));
	m_gpl_dma->space_write_callback().set(FUNC(generalplus_gpl951xx_device::write_space));

	GPL_CHX(config, m_gpl_chx, 0);
	m_gpl_chx->cha_write_callback().set(FUNC(generalplus_gpl951xx_device::dac_0_w));
	m_gpl_chx->chb_write_callback().set(FUNC(generalplus_gpl951xx_device::dac_1_w));

	GPL_TIMEBASE(config, m_gpl_timebase, 0);
	m_gpl_timebase->updateirqs_callback().set(FUNC(generalplus_gpl951xx_device::update_interrupts));

	GCM394_VIDEO(config, m_spg_video, DERIVED_CLOCK(1, 1), DEVICE_SELF, m_screen);
	m_spg_video->write_video_irq_callback().set(FUNC(generalplus_gpl951xx_device::videoirq_w));
	m_spg_video->space_read_callback().set(FUNC(generalplus_gpl951xx_device::read_space));
	m_spg_video->set_video_space(DEVICE_SELF, AS_PROGRAM);

	TIMER(config, "timer_a").configure_generic(FUNC(generalplus_gpl951xx_device::timer_a_cb));
	TIMER(config, "timer_b").configure_generic(FUNC(generalplus_gpl951xx_device::timer_b_cb));
	TIMER(config, "timer_c").configure_generic(FUNC(generalplus_gpl951xx_device::timer_c_cb));
	TIMER(config, "timer_d").configure_generic(FUNC(generalplus_gpl951xx_device::timer_d_cb));
	TIMER(config, "timer_e").configure_generic(FUNC(generalplus_gpl951xx_device::timer_e_cb));
	TIMER(config, "timer_f").configure_generic(FUNC(generalplus_gpl951xx_device::timer_f_cb));

	TIMER(config, "timer_g").configure_generic(FUNC(generalplus_gpl951xx_device::timer_g_cb));
	TIMER(config, "timer_h").configure_generic(FUNC(generalplus_gpl951xx_device::timer_h_cb));

	GPL951XX_RTC(config, m_rtc, 0);
}

DEFINE_DEVICE_TYPE(GPL951XX, generalplus_gpl951xx_device, "gpl951xx", "GeneralPlus GPL951xx")

generalplus_gpl951xx_device::generalplus_gpl951xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal) :
	unsp_20_device(mconfig, type, tag, owner, clock, internal),
	device_mixer_interface(mconfig, *this),
	m_spi_out(*this),
	m_spi_out_cmd(*this),
	m_spi_reset(*this),
	m_i80_cmd_out(*this),
	m_i80_data_out(*this),
	m_port_in(*this, 0),
	m_port_out(*this),
	m_timer_g(*this, "timer_g"),
	m_timer_h(*this, "timer_h"),
	m_rtc(*this, "rtc"),
	m_gpl_chx(*this, "gpl_chx"),
	m_dac0(*this, "dac0"),
	m_dac1(*this, "dac1"),
	m_gpl_dma(*this, "gpl_dma"),
	m_gpl_timebase(*this, "gpl_timebase"),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_spg_video(*this, "spgvideo"),
	m_spg_audio(*this, "spgaudio"),
	m_mainram(*this, "mainram")
{
}

generalplus_gpl951xx_device::generalplus_gpl951xx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	generalplus_gpl951xx_device(mconfig, GPL951XX, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpl951xx_device::gpspi_direct_internal_map), this))
{
}

