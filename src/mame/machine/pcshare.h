#include "machine/8237dma.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"

class pcat_base_state : public driver_device
{
public:
	pcat_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dma8237_1(*this, "dma8237_1"),
		m_pic8259_1(*this, "pic8259_1"),
		m_pic8259_2(*this, "pic8259_2"),
		m_pit8254(*this, "pit8254") { }

	IRQ_CALLBACK_MEMBER(irq_callback);

	required_device<cpu_device> m_maincpu;
	required_device<i8237_device> m_dma8237_1;
	required_device<pic8259_device> m_pic8259_1;
	required_device<pic8259_device> m_pic8259_2;
	required_device<pit8254_device> m_pit8254;

	DECLARE_WRITE_LINE_MEMBER(pc_dma_hrq_changed);
	DECLARE_READ8_MEMBER(pc_dma_read_byte);
	DECLARE_WRITE8_MEMBER(pc_dma_write_byte);
	DECLARE_READ8_MEMBER(dma_page_select_r);
	DECLARE_WRITE8_MEMBER(dma_page_select_w);
	void set_dma_channel(int channel, int state);
	DECLARE_WRITE_LINE_MEMBER( pc_dack0_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack1_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack2_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack3_w );
	DECLARE_READ8_MEMBER( get_slave_ack );
	DECLARE_WRITE_LINE_MEMBER( at_pit8254_out0_changed );
	DECLARE_WRITE_LINE_MEMBER( at_pit8254_out2_changed );
	DECLARE_READ8_MEMBER(get_out2);
	int m_dma_channel;
	UINT8 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];
};

ADDRESS_MAP_EXTERN(pcat32_io_common, 32);
MACHINE_CONFIG_EXTERN(pcat_common);
