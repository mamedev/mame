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


// ======================> ibm5160_mb_device
class ibm5160_mb_device : public device_t
{
public:
	// construction/destruction
	ibm5160_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// inline configuration
	template <typename T> void set_cputag(T &&tag)
	{
		m_maincpu.set_tag(std::forward<T>(tag));
		m_isabus.lookup()->set_memspace(m_maincpu, AS_PROGRAM);
		m_isabus.lookup()->set_iospace(m_maincpu, AS_IO);
		m_isabus.lookup()->iochrdy_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	}

	auto int_callback() { return m_int_callback.bind(); }
	auto nmi_callback() { return m_nmi_callback.bind(); }
	auto kbdclk_callback() { return m_kbdclk_callback.bind(); }
	auto kbddata_callback() { return m_kbddata_callback.bind(); }

	void map(address_map &map) ATTR_COLD;

	uint8_t m_pit_out2;

	void pc_page_w(offs_t offset, uint8_t data);
	void nmi_enable_w(uint8_t data);

	void pc_speaker_set_spkrdata(int state);

	void pc_pit8253_out1_changed(int state);
	virtual void pc_pit8253_out2_changed(int state);

	void pic_int_w(int state);

	// interface to the keyboard
	void keyboard_clock_w(int state);
	void keyboard_data_w(int state);

protected:
	ibm5160_mb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

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
	required_device<ram_device>             m_ram;

	devcb_write_line m_int_callback;
	devcb_write_line m_nmi_callback;
	devcb_write_line m_kbdclk_callback;
	devcb_write_line m_kbddata_callback;

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
	uint8_t                 m_ppi_keyb_clock;
	uint8_t                 m_ppi_portb;
	uint8_t                 m_ppi_clock_signal;
	uint8_t                 m_ppi_data_signal;
	uint8_t                 m_ppi_shift_register;
	uint8_t                 m_ppi_shift_enable;

	uint8_t pc_ppi_porta_r();
	uint8_t pc_ppi_portc_r();
	void pc_ppi_portb_w(uint8_t data);

	void pc_dma_hrq_changed(int state);
	void pc_dma8237_out_eop(int state);
	uint8_t pc_dma_read_byte(offs_t offset);
	void pc_dma_write_byte(offs_t offset, uint8_t data);
	uint8_t pc_dma8237_1_dack_r();
	uint8_t pc_dma8237_2_dack_r();
	uint8_t pc_dma8237_3_dack_r();
	void pc_dma8237_1_dack_w(uint8_t data);
	void pc_dma8237_2_dack_w(uint8_t data);
	void pc_dma8237_3_dack_w(uint8_t data);
	void pc_dma8237_0_dack_w(uint8_t data);
	void pc_dack0_w(int state);
	void pc_dack1_w(int state);
	void pc_dack2_w(int state);
	void pc_dack3_w(int state);
	void iochck_w(int state);

	void pc_select_dma_channel(int channel, bool state);
};


// device type definition
DECLARE_DEVICE_TYPE(IBM5160_MOTHERBOARD, ibm5160_mb_device)


// ======================> ibm5150_mb_device
class ibm5150_mb_device : public ibm5160_mb_device
{
public:
	// construction/destruction
	ibm5150_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void keyboard_clock_w(int state);

	virtual void pc_pit8253_out2_changed(int state) override;

protected:
	ibm5150_mb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<cassette_image_device>  m_cassette;

	uint8_t pc_ppi_porta_r();
	uint8_t pc_ppi_portc_r();
	void pc_ppi_portb_w(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(IBM5150_MOTHERBOARD, ibm5150_mb_device)


class ec1841_mb_device : public ibm5160_mb_device
{
public:
	// construction/destruction
	ec1841_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void keyboard_clock_w(int state);

protected:
	ec1841_mb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t pc_ppi_portc_r();
	void pc_ppi_portb_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(EC1841_MOTHERBOARD, ec1841_mb_device)


class ec1840_mb_device : public ec1841_mb_device
{
public:
	// construction/destruction
	ec1840_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t pc_ppi_portc_r();
	void pc_ppi_portb_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(EC1840_MOTHERBOARD, ec1840_mb_device)


class pc_noppi_mb_device : public ibm5160_mb_device
{
public:
	// construction/destruction
	pc_noppi_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t pit_out2() { return m_pit_out2; } // helper for near-clones with multifunction ics instead of 8255s

	void map(address_map &map) ATTR_COLD;

	uint8_t pc_ppi_portb_r();

protected:
	pc_noppi_mb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	uint8_t pc_ppi_porta_r();
};

DECLARE_DEVICE_TYPE(PCNOPPI_MOTHERBOARD, pc_noppi_mb_device)

#endif // MAME_MACHINE_GENPC_H
