// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/*****************************************************************************
 *
 * includes/pc.h
 *
 ****************************************************************************/

#ifndef MAME_MACHINE_GENPC_H
#define MAME_MACHINE_GENPC_H

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


// ======================> ibm5160_mb_device
class ibm5160_mb_device : public device_t
{
public:
	// construction/destruction
	ibm5160_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_cputag(T &&tag)
	{
		m_maincpu.set_tag(std::forward<T>(tag));
		subdevice<isa8_device>("isa")->set_memspace(std::forward<T>(tag), AS_PROGRAM);
		subdevice<isa8_device>("isa")->set_iospace(std::forward<T>(tag), AS_IO);
	}

	void map(address_map &map);

	uint8_t m_pit_out2;

	DECLARE_WRITE8_MEMBER(pc_page_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);

	DECLARE_WRITE_LINE_MEMBER( pc_speaker_set_spkrdata );

	DECLARE_WRITE_LINE_MEMBER( pc_pit8253_out1_changed );
	virtual DECLARE_WRITE_LINE_MEMBER( pc_pit8253_out2_changed );

	DECLARE_WRITE_LINE_MEMBER( pic_int_w );

protected:
	ibm5160_mb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

protected:
	required_device<cpu_device>             m_maincpu;
public:
	required_device<pic8259_device>         m_pic8259;
	required_device<pit8253_device>         m_pit8253;
	required_device<am9517a_device>         m_dma8237;
protected:
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
	DECLARE_WRITE_LINE_MEMBER( iochck_w );

	void pc_select_dma_channel(int channel, bool state);
};


// device type definition
DECLARE_DEVICE_TYPE(IBM5160_MOTHERBOARD, ibm5160_mb_device)


// ======================> ibm5150_mb_device
class ibm5150_mb_device : public ibm5160_mb_device
{
public:
	// construction/destruction
	ibm5150_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER( keyboard_clock_w );

	virtual DECLARE_WRITE_LINE_MEMBER( pc_pit8253_out2_changed ) override;

protected:
	ibm5150_mb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<cassette_image_device>  m_cassette;

	DECLARE_READ8_MEMBER ( pc_ppi_porta_r );
	DECLARE_READ8_MEMBER ( pc_ppi_portc_r );
	DECLARE_WRITE8_MEMBER( pc_ppi_portb_w );
};


// device type definition
DECLARE_DEVICE_TYPE(IBM5150_MOTHERBOARD, ibm5150_mb_device)


class ec1841_mb_device : public ibm5160_mb_device
{
public:
	// construction/destruction
	ec1841_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	ec1841_mb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

private:
	DECLARE_READ8_MEMBER ( pc_ppi_portc_r );
	DECLARE_WRITE8_MEMBER( pc_ppi_portb_w );

	DECLARE_WRITE_LINE_MEMBER( keyboard_clock_w );
};

DECLARE_DEVICE_TYPE(EC1841_MOTHERBOARD, ec1841_mb_device)


class ec1840_mb_device : public ec1841_mb_device
{
public:
	// construction/destruction
	ec1840_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

private:
	DECLARE_READ8_MEMBER ( pc_ppi_portc_r );
	DECLARE_WRITE8_MEMBER( pc_ppi_portb_w );
};

DECLARE_DEVICE_TYPE(EC1840_MOTHERBOARD, ec1840_mb_device)


class pc_noppi_mb_device : public ibm5160_mb_device
{
public:
	// construction/destruction
	pc_noppi_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t pit_out2() { return m_pit_out2; } // helper for near-clones with multifunction ics instead of 8255s

	void map(address_map &map);

protected:
	pc_noppi_mb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
};

DECLARE_DEVICE_TYPE(PCNOPPI_MOTHERBOARD, pc_noppi_mb_device)

#endif // MAME_MACHINE_GENPC_H
