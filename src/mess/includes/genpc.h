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
#include "machine/isa.h"
#include "machine/pc_kbdc.h"
#include "imagedev/cassette.h"

#define MCFG_IBM5160_MOTHERBOARD_ADD(_tag, _cputag) \
    MCFG_DEVICE_ADD(_tag, IBM5160_MOTHERBOARD, 0) \
	ibm5160_mb_device::static_set_cputag(*device, _cputag); \

// ======================> ibm5160_mb_device
class ibm5160_mb_device : public device_t
{
public:
	// construction/destruction
	ibm5160_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	void install_device(device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_device_func rhandler, const char* rhandler_name, write8_device_func whandler, const char *whandler_name);
	void install_device_write(device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, write8_device_func whandler, const char *whandler_name);
	void install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler);
public:
	required_device<cpu_device>  m_maincpu;
	required_device<device_t>  m_pic8259;
	required_device<am9517a_device>  m_dma8237;
	required_device<device_t>  m_pit8253;
	required_device<i8255_device>  m_ppi8255;
	required_device<device_t>  m_speaker;
	required_device<isa8_device>  m_isabus;
	required_device<pc_kbdc_device>  m_pc_kbdc;

	/* U73 is an LS74 - dual flip flop */
	/* Q2 is set by OUT1 from the 8253 and goes to DRQ1 on the 8237 */
	UINT8	m_u73_q2;
	UINT8	m_out1;
	int m_dma_channel;
	UINT8 m_dma_offset[4];
	UINT8 m_pc_spkrdata;
	UINT8 m_pc_input;

	UINT8 m_nmi_enabled;

	int						m_ppi_portc_switch_high;
	int						m_ppi_speaker;
	int						m_ppi_keyboard_clear;
	UINT8					m_ppi_keyb_clock;
	UINT8					m_ppi_portb;
	UINT8					m_ppi_clock_signal;
	UINT8					m_ppi_data_signal;
	UINT8					m_ppi_shift_register;
	UINT8					m_ppi_shift_enable;

	static IRQ_CALLBACK(pc_irq_callback);

	// interface to the keyboard
	DECLARE_WRITE_LINE_MEMBER( keyboard_clock_w );
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

	DECLARE_WRITE_LINE_MEMBER( pc_cpu_line );
	DECLARE_WRITE_LINE_MEMBER( pc_speaker_set_spkrdata );

	const char *m_cputag;
};


// device type definition
extern const device_type IBM5160_MOTHERBOARD;


#define MCFG_IBM5150_MOTHERBOARD_ADD(_tag, _cputag) \
    MCFG_DEVICE_ADD(_tag, IBM5150_MOTHERBOARD, 0) \
    ibm5150_mb_device::static_set_cputag(*device, _cputag); \


// ======================> ibm5150_mb_device
class ibm5150_mb_device : public ibm5160_mb_device
{
public:
	// construction/destruction
	ibm5150_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	required_device<cassette_image_device>	m_cassette;
public:
	virtual DECLARE_READ8_MEMBER ( pc_ppi_porta_r );
	virtual DECLARE_READ8_MEMBER ( pc_ppi_portc_r );
	virtual DECLARE_WRITE8_MEMBER( pc_ppi_portb_w );
};


// device type definition
extern const device_type IBM5150_MOTHERBOARD;

#endif /* GENPC_H_ */
