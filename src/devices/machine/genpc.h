// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/*****************************************************************************
 *
 * includes/pc.h
 *
 ****************************************************************************/

#ifndef MAME_DEVICES_MACHINE_GENPC_H
#define MAME_DEVICES_MACHINE_GENPC_H

#include "imagedev/cassette.h"
#include "machine/am9517a.h"
#include "machine/i8255.h"
#include "machine/ins8250.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"

#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/pc_kbdc.h"


#define MCFG_IBM5160_MOTHERBOARD_ADD(_tag, _cputag) \
	MCFG_DEVICE_ADD(_tag, IBM5160_MOTHERBOARD, 0) \
	ibm5160_mb_device::static_set_cputag(*device, _cputag);
// ======================> ibm5160_mb_device
class ibm5160_mb_device : public device_t
{
public:
	// construction/destruction
	ibm5160_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_ADDRESS_MAP(map, 8);
protected:
	ibm5160_mb_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
public:
	required_device<cpu_device>             m_maincpu;
	required_device<pic8259_device>         m_pic8259;
	required_device<am9517a_device>         m_dma8237;
	required_device<pit8253_device>         m_pit8253;
	optional_device<i8255_device>           m_ppi8255;
	required_device<speaker_sound_device>   m_speaker;
	required_device<isa8_device>            m_isabus;
	optional_device<pc_kbdc_device>         m_pc_kbdc;
	required_device<ram_device>             m_ram;

	/* U73 is an LS74 - dual flip flop */
	/* Q2 is set by OUT1 from the 8253 and goes to DRQ1 on the 8237 */
	uint8_t   m_u73_q2;
	uint8_t   m_out1;
	int m_dma_channel;
	uint8_t m_dma_offset[4];
	uint8_t m_pc_spkrdata;
	uint8_t m_pit_out2;
	bool m_cur_eop;

	uint8_t m_nmi_enabled;

	int                     m_ppi_portc_switch_high;
	int                     m_ppi_speaker;
	int                     m_ppi_keyboard_clear;
	uint8_t                   m_ppi_keyb_clock;
	uint8_t                   m_ppi_portb;
	uint8_t                   m_ppi_clock_signal;
	uint8_t                   m_ppi_data_signal;
	uint8_t                   m_ppi_shift_register;
	uint8_t                   m_ppi_shift_enable;

	// interface to the keyboard
	DECLARE_WRITE_LINE_MEMBER( keyboard_clock_w );
	DECLARE_WRITE_LINE_MEMBER( keyboard_data_w );

	DECLARE_READ8_MEMBER ( pc_ppi_porta_r );
	DECLARE_READ8_MEMBER ( pc_ppi_portc_r );
	DECLARE_WRITE8_MEMBER( pc_ppi_portb_w );

	DECLARE_WRITE_LINE_MEMBER( pc_pit8253_out1_changed );
	DECLARE_WRITE_LINE_MEMBER( pc_pit8253_out2_changed );

	DECLARE_WRITE_LINE_MEMBER( pc_dma_hrq_changed );
	DECLARE_WRITE_LINE_MEMBER( pc_dma8237_out_eop );
	DECLARE_READ8_MEMBER( pc_dma_read_byte );
	DECLARE_WRITE8_MEMBER( pc_dma_write_byte );
	DECLARE_READ8_MEMBER( pc_dma8237_1_dack_r );
	DECLARE_READ8_MEMBER( pc_dma8237_2_dack_r );
	DECLARE_READ8_MEMBER( pc_dma8237_3_dack_r );
	DECLARE_WRITE8_MEMBER( pc_dma8237_1_dack_w );
	DECLARE_WRITE8_MEMBER( pc_dma8237_2_dack_w );
	DECLARE_WRITE8_MEMBER( pc_dma8237_3_dack_w );
	DECLARE_WRITE8_MEMBER( pc_dma8237_0_dack_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack0_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack1_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack2_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack3_w );

	DECLARE_WRITE_LINE_MEMBER( pc_speaker_set_spkrdata );

	DECLARE_WRITE8_MEMBER(pc_page_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);

	const char *m_cputag;

private:
	void pc_select_dma_channel(int channel, bool state);
};


// device type definition
extern const device_type IBM5160_MOTHERBOARD;


#define MCFG_IBM5150_MOTHERBOARD_ADD(_tag, _cputag) \
	MCFG_DEVICE_ADD(_tag, IBM5150_MOTHERBOARD, 0) \
	ibm5150_mb_device::static_set_cputag(*device, _cputag);

// ======================> ibm5150_mb_device
class ibm5150_mb_device : public ibm5160_mb_device
{
public:
	// construction/destruction
	ibm5150_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_WRITE_LINE_MEMBER( keyboard_clock_w );

protected:
	ibm5150_mb_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// device-level overrides

	required_device<cassette_image_device>  m_cassette;
public:
	DECLARE_READ8_MEMBER ( pc_ppi_porta_r );
	DECLARE_READ8_MEMBER ( pc_ppi_portc_r );
	DECLARE_WRITE8_MEMBER( pc_ppi_portb_w );
};


// device type definition
extern const device_type IBM5150_MOTHERBOARD;


#define MCFG_EC1841_MOTHERBOARD_ADD(_tag, _cputag) \
	MCFG_DEVICE_ADD(_tag, EC1841_MOTHERBOARD, 0) \
	ec1841_mb_device::static_set_cputag(*device, _cputag);

class ec1841_mb_device : public ibm5160_mb_device
{
public:
	// construction/destruction
	ec1841_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

public:
	DECLARE_READ8_MEMBER ( pc_ppi_portc_r );
	DECLARE_WRITE8_MEMBER( pc_ppi_portb_w );

	DECLARE_WRITE_LINE_MEMBER( keyboard_clock_w );
};

extern const device_type EC1841_MOTHERBOARD;

#define MCFG_PCNOPPI_MOTHERBOARD_ADD(_tag, _cputag) \
	MCFG_DEVICE_ADD(_tag, PCNOPPI_MOTHERBOARD, 0) \
	pc_noppi_mb_device::static_set_cputag(*device, _cputag);

class pc_noppi_mb_device : public ibm5160_mb_device
{
public:
	// construction/destruction
	pc_noppi_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	uint8_t pit_out2() { return m_pit_out2; } // helper for near-clones with multifunction ics instead of 8255s

	DECLARE_ADDRESS_MAP(map, 8);

protected:
	pc_noppi_mb_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
};

extern const device_type PCNOPPI_MOTHERBOARD;

#endif // MAME_DEVICES_MACHINE_GENPC_H
