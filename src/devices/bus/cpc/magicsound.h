// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * magicsound.h
 *
 *  Magic Sound Board for the Aleste 520EX
 *
 *  DMA-based 4-channel sound board
 *
 *  1x K1810WT37 DMA controller (i8237/AM9517A)
 *  2x K1810WT54 programmable timers  (i8254)
 *  1x K1118PA1 DAC  (MC10318)
 *
 *  I/O Ports:
 *  FxDx: selects the board
 *  F8Dx: DMA controller (R/w)
 *  F9Dx: PIT timers (A2 active for channels 0-2, A3 active for channels 3-5) (W/O)
 *  FADx: Volume control (A1-A0 = channel) (W/O, 6-bit)
 *  FBDx: Mapper (A1-A0 = mapper page number, A3-A2 = channel, D5-D0 = inverted page number) (W/O)
 *
 *  Further info available here:  http://cpcwiki.eu/index.php/Magic_Sound_Board
 *
 */

#ifndef MAGICSOUND_H_
#define MAGICSOUND_H_

#include "emu.h"
#include "cpcexp.h"
#include "sound/dmadac.h"
#include "sound/dac.h"
#include "machine/am9517a.h"
#include "machine/pit8253.h"
#include "machine/ram.h"

class al_magicsound_device  : public device_t,
							public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	al_magicsound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_READ8_MEMBER(dmac_r);
	DECLARE_WRITE8_MEMBER(dmac_w);
	DECLARE_WRITE8_MEMBER(timer_w);
	DECLARE_WRITE8_MEMBER(volume_w);
	DECLARE_WRITE8_MEMBER(mapper_w);
	DECLARE_WRITE_LINE_MEMBER(da0_w);
	DECLARE_READ8_MEMBER(dma_read_byte);
	DECLARE_WRITE8_MEMBER(dma_write_byte);
	DECLARE_WRITE_LINE_MEMBER(dack0_w);
	DECLARE_WRITE_LINE_MEMBER(dack1_w);
	DECLARE_WRITE_LINE_MEMBER(dack2_w);
	DECLARE_WRITE_LINE_MEMBER(dack3_w);
	DECLARE_WRITE_LINE_MEMBER(sam0_w);
	DECLARE_WRITE_LINE_MEMBER(sam1_w);
	DECLARE_WRITE_LINE_MEMBER(sam2_w);
	DECLARE_WRITE_LINE_MEMBER(sam3_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	cpc_expansion_slot_device *m_slot;

	required_device<dac_device> m_dac1;
	required_device<am9517a_device> m_dmac;
	required_device<pit8254_device> m_timer1;
	required_device<pit8254_device> m_timer2;

	void set_timer_gate(bool state);

	UINT8 m_volume[4];
	UINT32 m_page[4][4];
	UINT8 m_output[4];
	bool m_dack[4];
	INT8 m_current_channel;
	ram_device* m_ramptr;
	UINT8 m_current_output;
};

// device type definition
extern const device_type AL_MAGICSOUND;


#endif /* MAGICSOUND_H_ */
