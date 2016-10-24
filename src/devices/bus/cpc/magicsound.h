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
	al_magicsound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	uint8_t dmac_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dmac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void timer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void volume_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mapper_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void da0_w(int state);
	uint8_t dma_read_byte(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dma_write_byte(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dack0_w(int state);
	void dack1_w(int state);
	void dack2_w(int state);
	void dack3_w(int state);
	void sam0_w(int state);
	void sam1_w(int state);
	void sam2_w(int state);
	void sam3_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	cpc_expansion_slot_device *m_slot;

	required_device<dac_byte_interface> m_dac;
	required_device<am9517a_device> m_dmac;
	required_device<pit8254_device> m_timer1;
	required_device<pit8254_device> m_timer2;

	void set_timer_gate(bool state);

	uint8_t m_volume[4];
	uint32_t m_page[4][4];
	uint8_t m_output[4];
	bool m_dack[4];
	int8_t m_current_channel;
	ram_device* m_ramptr;
	uint8_t m_current_output;
};

// device type definition
extern const device_type AL_MAGICSOUND;


#endif /* MAGICSOUND_H_ */
