// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/*****************************************************************************
 *
 * includes/pc.h
 *
 ****************************************************************************/

#ifndef GENPC_H_
#define GENPC_H_

#include "machine/ins8250.h"
#include "machine/i8255.h"
#include "machine/am9517a.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "sound/speaker.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"

#define MCFG_IBM5160_MOTHERBOARD_ADD(_tag, _cputag) \
	MCFG_DEVICE_ADD(_tag, IBM5160_MOTHERBOARD, 0) \
	ibm5160_mb_device::static_set_cputag(*device, _cputag);
// ======================> ibm5160_mb_device
class ibm5160_mb_device : public device_t
{
public:
	// construction/destruction
	ibm5160_mb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	// inline configuration
	static void static_set_cputag(device_t &device, std::string tag);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler);
public:
	required_device<cpu_device>  m_maincpu;
	required_device<pic8259_device>  m_pic8259;
	required_device<am9517a_device>  m_dma8237;
	required_device<pit8253_device>  m_pit8253;
	optional_device<i8255_device>  m_ppi8255;
	required_device<speaker_sound_device>  m_speaker;
	required_device<isa8_device>  m_isabus;
	optional_device<pc_kbdc_device>  m_pc_kbdc;
	required_device<ram_device> m_ram;

	/* U73 is an LS74 - dual flip flop */
	/* Q2 is set by OUT1 from the 8253 and goes to DRQ1 on the 8237 */
	UINT8   m_u73_q2;
	UINT8   m_out1;
	int m_dma_channel;
	UINT8 m_dma_offset[4];
	UINT8 m_pc_spkrdata;
	UINT8 m_pit_out2;
	bool m_cur_eop;

	UINT8 m_nmi_enabled;

	int                     m_ppi_portc_switch_high;
	int                     m_ppi_speaker;
	int                     m_ppi_keyboard_clear;
	UINT8                   m_ppi_keyb_clock;
	UINT8                   m_ppi_portb;
	UINT8                   m_ppi_clock_signal;
	UINT8                   m_ppi_data_signal;
	UINT8                   m_ppi_shift_register;
	UINT8                   m_ppi_shift_enable;

	// interface to the keyboard
	virtual DECLARE_WRITE_LINE_MEMBER( keyboard_clock_w );
	DECLARE_WRITE_LINE_MEMBER( keyboard_data_w );

	virtual DECLARE_READ8_MEMBER ( pc_ppi_porta_r );
	virtual DECLARE_READ8_MEMBER ( pc_ppi_portc_r );
	virtual DECLARE_WRITE8_MEMBER( pc_ppi_portb_w );

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

	DECLARE_READ8_MEMBER(pc_page_r);
	DECLARE_WRITE8_MEMBER(pc_page_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);

	std::string m_cputag;

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
	ibm5150_mb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual DECLARE_WRITE_LINE_MEMBER( keyboard_clock_w ) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<cassette_image_device>  m_cassette;
public:
	virtual DECLARE_READ8_MEMBER ( pc_ppi_porta_r ) override;
	virtual DECLARE_READ8_MEMBER ( pc_ppi_portc_r ) override;
	virtual DECLARE_WRITE8_MEMBER( pc_ppi_portb_w ) override;
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
	ec1841_mb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

public:
	virtual DECLARE_READ8_MEMBER ( pc_ppi_portc_r ) override;
	virtual DECLARE_WRITE8_MEMBER( pc_ppi_portb_w ) override;

	virtual DECLARE_WRITE_LINE_MEMBER( keyboard_clock_w ) override;
};

extern const device_type EC1841_MOTHERBOARD;

#define MCFG_PCNOPPI_MOTHERBOARD_ADD(_tag, _cputag) \
	MCFG_DEVICE_ADD(_tag, PCNOPPI_MOTHERBOARD, 0) \
	pc_noppi_mb_device::static_set_cputag(*device, _cputag);

class pc_noppi_mb_device : public ibm5160_mb_device
{
public:
	// construction/destruction
	pc_noppi_mb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
protected:
	// device-level overrides
	void device_start() override;
};

extern const device_type PCNOPPI_MOTHERBOARD;

#endif /* GENPC_H_ */
