// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/* V53 */

// V33 / V33A cores with onboard peripherals

// Interrupt Controller is uPD71059 equivalent (a PIC8259 clone?)
// DMA Controller can operate in modes providing a subset of the uPD71071 or uPD71037 functionality (some modes unavailable / settings ignored) (uPD71071 mode is an extended 8237A, uPD71037 mode is plain 8237A)
// Serial Controller is based on the uPD71051 but with some changes (i8251 clone?)
// Timer Unit is functionally identical to uPD71054 (which in turn is said to be the same as a pit8253)

#include "emu.h"
#include "v53.h"

#include "necpriv.h"



DEFINE_DEVICE_TYPE(V53,  v53_device,  "v53",  "NEC V53")
DEFINE_DEVICE_TYPE(V53A, v53a_device, "v53a", "NEC V53A")

WRITE8_MEMBER(v53_base_device::BSEL_w)
{
	//printf("v53: BSEL_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::BADR_w)
{
	//printf("v53: BADR_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::BRC_w)
{
	//printf("v53: BRC_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WMB0_w)
{
	//printf("v53: WMB0_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WCY1_w)
{
	//printf("v53: WCY1_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WCY0_w)
{
	//printf("v53: WCY0_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WAC_w)
{
	//printf("v53: WAC_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::TCKS_w)
{
	//printf("v53: TCKS_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::SBCR_w)
{
	//printf("v53: SBCR_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::REFC_w)
{
	//printf("v53: REFC_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WMB1_w)
{
	//printf("v53: WMB1_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WCY2_w)
{
	//printf("v53: WCY2_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WCY3_w)
{
	//printf("v53: WCY3_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WCY4_w)
{
	//printf("v53: WCY4_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::SULA_w)
{
	//printf("v53: SULA_w %02x\n", data);
	m_SULA = data;
	install_peripheral_io();
}

WRITE8_MEMBER(v53_base_device::TULA_w)
{
	//printf("v53: TULA_w %02x\n", data);
	m_TULA = data;
	install_peripheral_io();
}

WRITE8_MEMBER(v53_base_device::IULA_w)
{
	//printf("v53: IULA_w %02x\n", data);
	m_IULA = data;
	install_peripheral_io();
}

WRITE8_MEMBER(v53_base_device::DULA_w)
{
	//printf("v53: DULA_w %02x\n", data);
	m_DULA = data;
	install_peripheral_io();
}

WRITE8_MEMBER(v53_base_device::OPHA_w)
{
	//printf("v53: OPHA_w %02x\n", data);
	m_OPHA = data;
	install_peripheral_io();
}

WRITE8_MEMBER(v53_base_device::OPSEL_w)
{
	//printf("v53: OPSEL_w %02x\n", data);
	m_OPSEL = data;
	install_peripheral_io();
}

WRITE8_MEMBER(v53_base_device::SCTL_w)
{
	// bit 7: unused
	// bit 6: unused
	// bit 5: unused
	// bit 4: SCU input clock source
	// bit 3: uPD71037 DMA mode - Carry A20
	// bit 2: uPD71037 DMA mode - Carry A16
	// bit 1: uPD71037 DMA mode enable (otherwise in uPD71071 mode)
	// bit 0: Onboard pripheral I/O maps to 8-bit boundaries? (otherwise 16-bit)

	//printf("v53: SCTL_w %02x\n", data);
	m_SCTL = data;
	install_peripheral_io();
}
/*
m_WCY0 = 0x07;
m_WCY1 = 0x77;
m_WCY2 = 0x77;
m_WCY3 = 0x77;
m_WCY4 = 0x77;
m_WMB0 = 0x77;
m_WMB1 = 0x77;
m_WAC =  0x00;
m_TCKS = 0x00;
m_RFC =  0x80;
m_SBCR = 0x00;
m_BRC =  0x00;
// SCU
m_SMD =  0x4b;
m_SCM =  0x00;
m_SIMK = 0x03;
m_SST = 0x04;
// DMA
m_DCH = 0x01;
m_DMD = 0x00;
m_DCC = 0x0000;
m_DST = 0x00;
m_DMK = 0x0f;
*/

void v53_base_device::device_reset()
{
	v33_base_device::device_reset();

	m_SCTL = 0x00;
	m_OPSEL= 0x00;

	// peripheral addresses
	m_SULA = 0x00;
	m_TULA = 0x00;
	m_IULA = 0x00;
	m_DULA = 0x00;
	m_OPHA = 0x00;

	m_simk = 0x03;
}

void v53_base_device::device_start()
{
	v33_base_device::device_start();

	m_txd_handler.resolve_safe();
	m_rts_handler.resolve_safe();
	m_dtr_handler.resolve_safe();
	m_rxrdy_handler.resolve_safe();
	m_txrdy_handler.resolve_safe();
	m_txempty_handler.resolve_safe();

	m_out0_handler.resolve_safe();
	m_out1_handler.resolve_safe();
	m_out2_handler.resolve_safe();

	m_out_hreq_cb.resolve_safe();
	m_out_eop_cb.resolve_safe();
	m_in_memr_cb.resolve_safe(0);
	m_out_memw_cb.resolve_safe();
	m_in_ior_0_cb.resolve_safe(0);
	m_in_ior_1_cb.resolve_safe(0);
	m_in_ior_2_cb.resolve_safe(0);
	m_in_ior_3_cb.resolve_safe(0);
	m_out_iow_0_cb.resolve_safe();
	m_out_iow_1_cb.resolve_safe();
	m_out_iow_2_cb.resolve_safe();
	m_out_iow_3_cb.resolve_safe();
	m_out_dack_0_cb.resolve_safe();
	m_out_dack_1_cb.resolve_safe();
	m_out_dack_2_cb.resolve_safe();
	m_out_dack_3_cb.resolve_safe();

	set_irq_acknowledge_callback(device_irq_acknowledge_delegate(FUNC(pic8259_device::inta_cb), (pic8259_device*)m_v53icu));

	save_item(NAME(m_SCTL));
	save_item(NAME(m_OPSEL));
	save_item(NAME(m_SULA));
	save_item(NAME(m_TULA));
	save_item(NAME(m_IULA));
	save_item(NAME(m_DULA));
	save_item(NAME(m_OPHA));

	save_item(NAME(m_simk));
}

void v53_base_device::device_post_load()
{
	install_peripheral_io();
}

void v53_base_device::install_peripheral_io()
{
	// unmap everything in I/O space up to the fixed position registers (we avoid overwriting them, it isn't a valid config)
	space(AS_IO).unmap_readwrite(0x1000, 0xfeff); // todo, we need to have a way to NOT unmap things defined in the drivers, but instead have this act as an overlay mapping / unampping only!!

	// IOAG determines if the handlers used 8-bit or 16-bit access
	// the hng64.c games first set everything up in 8-bit mode, then
	// do the procedure again in 16-bit mode before using them?!

	int IOAG = m_SCTL & 1;

	if (m_OPSEL & 0x01) // DMA Unit available
	{
		uint16_t base = (m_OPHA << 8) | m_DULA;
		base &= 0xfffe;

		if (m_SCTL & 0x02) // uPD71037 mode
		{
			if (IOAG) // 8-bit
			{
			}
			else
			{
			}
		}
		else // uPD71071 mode
		{
			space(AS_IO).install_readwrite_handler(base+0x00, base+0x0f, read8_delegate(FUNC(upd71071_v53_device::read), (upd71071_v53_device*)m_v53dmau), write8_delegate(FUNC(upd71071_v53_device::write),  (upd71071_v53_device*)m_v53dmau), 0xffff);
		}
	}

	if (m_OPSEL & 0x02) // Interrupt Control Unit available
	{
		uint16_t base = (m_OPHA << 8) | m_IULA;
		base &= 0xfffe;

		if (IOAG) // 8-bit
		{
			space(AS_IO).install_readwrite_handler(base+0x00, base+0x01, read8sm_delegate(FUNC(pic8259_device::read), (pic8259_device*)m_v53icu), write8sm_delegate(FUNC(pic8259_device::write), (pic8259_device*)m_v53icu), 0xffff);
		}
		else
		{
			space(AS_IO).install_readwrite_handler(base+0x00, base+0x03, read8sm_delegate(FUNC(pic8259_device::read), (pic8259_device*)m_v53icu), write8sm_delegate(FUNC(pic8259_device::write), (pic8259_device*)m_v53icu), 0x00ff);
		}
	}

	if (m_OPSEL & 0x04) // Timer Control Unit available
	{
		uint16_t base = (m_OPHA << 8) | m_TULA;
		//printf("installing TCU to %04x\n", base);
		base &= 0xfffe;

		if (IOAG) // 8-bit
		{
			space(AS_IO).install_readwrite_handler(base+0x00, base+0x01, read8_delegate(FUNC(v53_base_device::tmu_tst0_r), this), write8_delegate(FUNC(v53_base_device::tmu_tct0_w), this), 0x00ff);
			space(AS_IO).install_readwrite_handler(base+0x00, base+0x01, read8_delegate(FUNC(v53_base_device::tmu_tst1_r), this), write8_delegate(FUNC(v53_base_device::tmu_tct1_w), this), 0xff00);
			space(AS_IO).install_readwrite_handler(base+0x02, base+0x03, read8_delegate(FUNC(v53_base_device::tmu_tst2_r), this), write8_delegate(FUNC(v53_base_device::tmu_tct2_w), this), 0x00ff);
			space(AS_IO).install_write_handler(base+0x02, base+0x03, write8_delegate(FUNC(v53_base_device::tmu_tmd_w), this), 0xff00);
		}
		else
		{
			space(AS_IO).install_readwrite_handler(base+0x00, base+0x01, read8_delegate(FUNC(v53_base_device::tmu_tst0_r), this), write8_delegate(FUNC(v53_base_device::tmu_tct0_w), this), 0x00ff);
			space(AS_IO).install_readwrite_handler(base+0x02, base+0x03, read8_delegate(FUNC(v53_base_device::tmu_tst1_r), this), write8_delegate(FUNC(v53_base_device::tmu_tct1_w), this), 0x00ff);
			space(AS_IO).install_readwrite_handler(base+0x04, base+0x05, read8_delegate(FUNC(v53_base_device::tmu_tst2_r), this), write8_delegate(FUNC(v53_base_device::tmu_tct2_w), this), 0x00ff);
			space(AS_IO).install_write_handler(base+0x06, base+0x07, write8_delegate(FUNC(v53_base_device::tmu_tmd_w), this), 0x00ff);
		}
	}

	if (m_OPSEL & 0x08) // Serial Control Unit available
	{
		uint16_t base = (m_OPHA << 8) | m_SULA;
		base &= 0xfffe;

		if (IOAG) // 8-bit
		{
			space(AS_IO).install_readwrite_handler(base+0x00, base+0x01, read8smo_delegate(FUNC(v53_scu_device::data_r), m_v53scu.target()), write8smo_delegate(FUNC(v53_scu_device::data_w), m_v53scu.target()), 0x00ff);
			space(AS_IO).install_readwrite_handler(base+0x00, base+0x01, read8smo_delegate(FUNC(v53_scu_device::status_r),  m_v53scu.target()), write8smo_delegate(FUNC(v53_scu_device::command_w),  m_v53scu.target()), 0xff00);
			space(AS_IO).install_write_handler(base+0x02, base+0x03, write8smo_delegate(FUNC(v53_scu_device::mode_w), m_v53scu.target()), 0x00ff);
			space(AS_IO).install_readwrite_handler(base+0x02, base+0x03, read8_delegate(FUNC(v53_base_device::scu_simk_r), this), write8_delegate(FUNC(v53_base_device::scu_simk_w), this), 0xff00);
		}
		else
		{
			space(AS_IO).install_readwrite_handler(base+0x00, base+0x01, read8smo_delegate(FUNC(v53_scu_device::data_r), m_v53scu.target()), write8smo_delegate(FUNC(v53_scu_device::data_w), m_v53scu.target()), 0x00ff);
			space(AS_IO).install_readwrite_handler(base+0x02, base+0x03, read8smo_delegate(FUNC(v53_scu_device::status_r),  m_v53scu.target()), write8smo_delegate(FUNC(v53_scu_device::command_w),  m_v53scu.target()), 0x00ff);
			space(AS_IO).install_write_handler(base+0x04, base+0x05, write8smo_delegate(FUNC(v53_scu_device::mode_w), m_v53scu.target()), 0x00ff);
			space(AS_IO).install_readwrite_handler(base+0x06, base+0x07, read8_delegate(FUNC(v53_base_device::scu_simk_r), this), write8_delegate(FUNC(v53_base_device::scu_simk_w), this), 0x00ff);

		}
	}

}



/*** SCU ***/


READ8_MEMBER(v53_base_device::scu_simk_r)
{
	//printf("v53: scu_simk_r\n");
	return m_simk;
}

WRITE8_MEMBER(v53_base_device::scu_simk_w)
{
	m_simk = data;
	//printf("v53: scu_simk_w %02x\n", data);
}



/*** TCU ***/

WRITE8_MEMBER(v53_base_device::tmu_tct0_w) { m_v53tcu->write(0, data); }
WRITE8_MEMBER(v53_base_device::tmu_tct1_w) { m_v53tcu->write(1, data); }
WRITE8_MEMBER(v53_base_device::tmu_tct2_w) { m_v53tcu->write(2, data); }
WRITE8_MEMBER(v53_base_device::tmu_tmd_w)  { m_v53tcu->write(3, data); }


READ8_MEMBER(v53_base_device::tmu_tst0_r) { return m_v53tcu->read(0); }
READ8_MEMBER(v53_base_device::tmu_tst1_r) { return m_v53tcu->read(1); }
READ8_MEMBER(v53_base_device::tmu_tst2_r) { return m_v53tcu->read(2); }





/*** DMA ***/

// could be wrong / nonexistent
WRITE_LINE_MEMBER(v53_base_device::dreq0_w)
{
	if (!(m_SCTL & 0x02))
	{
		m_v53dmau->dreq0_w(state);
	}
	else
	{
		//printf("v53: dreq0 not in 71071mode\n");
	}
}

WRITE_LINE_MEMBER(v53_base_device::dreq1_w)
{
	if (!(m_SCTL & 0x02))
	{
		m_v53dmau->dreq1_w(state);
	}
	else
	{
		//printf("v53: dreq1 not in 71071mode\n");
	}
}

WRITE_LINE_MEMBER(v53_base_device::dreq2_w)
{
	if (!(m_SCTL & 0x02))
	{
		m_v53dmau->dreq2_w(state);
	}
	else
	{
		//printf("v53: dreq2 not in 71071mode\n");
	}
}

WRITE_LINE_MEMBER(v53_base_device::dreq3_w)
{
	if (!(m_SCTL & 0x02))
	{
		m_v53dmau->dreq3_w(state);
	}
	else
	{
		//printf("v53: dreq3 not in 71071mode\n");
	}
}

WRITE_LINE_MEMBER(v53_base_device::hack_w)
{
	if (!(m_SCTL & 0x02))
	{
		m_v53dmau->hack_w(state);
	}
	else
	{
		//printf("v53: hack_w not in 71071mode\n");
	}
}

/* General stuff */

void v53_base_device::v53_internal_port_map(address_map &map)
{
	v33_internal_port_map(map);
	map(0xffe0, 0xffe0).w(FUNC(v53_base_device::BSEL_w)); // 0xffe0 // uPD71037 DMA mode bank selection register
	map(0xffe1, 0xffe1).w(FUNC(v53_base_device::BADR_w)); // 0xffe1 // uPD71037 DMA mode bank register peripheral mapping (also uses OPHA)
//  AM_RANGE(0xffe2, 0xffe3) // (reserved     ,  0x00ff) // 0xffe2
//  AM_RANGE(0xffe2, 0xffe3) // (reserved     ,  0xff00) // 0xffe3
//  AM_RANGE(0xffe4, 0xffe5) // (reserved     ,  0x00ff) // 0xffe4
//  AM_RANGE(0xffe4, 0xffe5) // (reserved     ,  0xff00) // 0xffe5
//  AM_RANGE(0xffe6, 0xffe7) // (reserved     ,  0x00ff) // 0xffe6
//  AM_RANGE(0xffe6, 0xffe7) // (reserved     ,  0xff00) // 0xffe7
//  AM_RANGE(0xffe8, 0xffe9) // (reserved     ,  0x00ff) // 0xffe8
	map(0xffe9, 0xffe9).w(FUNC(v53_base_device::BRC_w)); // 0xffe9 // baud rate counter (used for serial peripheral)
	map(0xffea, 0xffea).w(FUNC(v53_base_device::WMB0_w)); // 0xffea // waitstate control
	map(0xffeb, 0xffeb).w(FUNC(v53_base_device::WCY1_w)); // 0xffeb // waitstate control
	map(0xffec, 0xffec).w(FUNC(v53_base_device::WCY0_w)); // 0xffec // waitstate control
	map(0xffed, 0xffed).w(FUNC(v53_base_device::WAC_w)); // 0xffed // waitstate control
//  AM_RANGE(0xffee, 0xffef) // (reserved     ,  0x00ff) // 0xffee
//  AM_RANGE(0xffee, 0xffef) // (reserved     ,  0xff00) // 0xffef
	map(0xfff0, 0xfff0).w(FUNC(v53_base_device::TCKS_w)); // 0xfff0 // timer clocks
	map(0xfff1, 0xfff1).w(FUNC(v53_base_device::SBCR_w)); // 0xfff1 // internal clock divider, halt behavior etc.
	map(0xfff2, 0xfff2).w(FUNC(v53_base_device::REFC_w)); // 0xfff2 // ram refresh control
	map(0xfff3, 0xfff3).w(FUNC(v53_base_device::WMB1_w)); // 0xfff3 // waitstate control
	map(0xfff4, 0xfff4).w(FUNC(v53_base_device::WCY2_w)); // 0xfff4 // waitstate control
	map(0xfff5, 0xfff5).w(FUNC(v53_base_device::WCY3_w)); // 0xfff5 // waitstate control
	map(0xfff6, 0xfff6).w(FUNC(v53_base_device::WCY4_w)); // 0xfff6 // waitstate control
//  AM_RANGE(0xfff6, 0xfff7) // (reserved     ,  0xff00) // 0xfff7
	map(0xfff8, 0xfff8).w(FUNC(v53_base_device::SULA_w)); // 0xfff8 // peripheral mapping
	map(0xfff9, 0xfff9).w(FUNC(v53_base_device::TULA_w)); // 0xfff9 // peripheral mapping
	map(0xfffa, 0xfffa).w(FUNC(v53_base_device::IULA_w)); // 0xfffa // peripheral mapping
	map(0xfffb, 0xfffb).w(FUNC(v53_base_device::DULA_w)); // 0xfffb // peripheral mapping
	map(0xfffc, 0xfffc).w(FUNC(v53_base_device::OPHA_w)); // 0xfffc // peripheral mapping (upper bits, common)
	map(0xfffd, 0xfffd).w(FUNC(v53_base_device::OPSEL_w)); // 0xfffd // peripheral enabling
	map(0xfffe, 0xfffe).w(FUNC(v53_base_device::SCTL_w)); // 0xfffe // peripheral configuration (& byte / word mapping)
//  AM_RANGE(0xfffe, 0xffff) // (reserved     ,  0xff00) // 0xffff
}




READ8_MEMBER(v53_base_device::get_pic_ack)
{
	return 0;
}



// the external interface provides no external access to the usual IRQ line of the V33, everything goes through the interrupt controller
void v53_base_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
		case INPUT_LINE_IRQ0: m_v53icu->ir0_w(state); break;
		case INPUT_LINE_IRQ1: m_v53icu->ir1_w(state); break;
		case INPUT_LINE_IRQ2: m_v53icu->ir2_w(state); break;
		case INPUT_LINE_IRQ3: m_v53icu->ir3_w(state); break;
		case INPUT_LINE_IRQ4: m_v53icu->ir4_w(state); break;
		case INPUT_LINE_IRQ5: m_v53icu->ir5_w(state); break;
		case INPUT_LINE_IRQ6: m_v53icu->ir6_w(state); break;
		case INPUT_LINE_IRQ7: m_v53icu->ir7_w(state); break;

		case INPUT_LINE_NMI: nec_common_device::execute_set_input(irqline, state); break;
		case NEC_INPUT_LINE_POLL: nec_common_device::execute_set_input(irqline, state); break;
	}
}

// for hooking the interrupt controller output up to the core
WRITE_LINE_MEMBER(v53_base_device::internal_irq_w)
{
	nec_common_device::execute_set_input(0, state);
}


void v53_base_device::device_add_mconfig(machine_config &config)
{
	PIT8254(config, m_v53tcu, 0); // functionality identical to uPD71054
	m_v53tcu->set_clk<0>(16000000); // manual implicitly claims that these runs at same speed as the CPU
	m_v53tcu->set_clk<1>(16000000);
	m_v53tcu->set_clk<2>(16000000);
	m_v53tcu->out_handler<0>().set([this] (int state) { m_out0_handler(state); });
	m_v53tcu->out_handler<1>().set([this] (int state) { m_out1_handler(state); });
	m_v53tcu->out_handler<2>().set([this] (int state) { m_out2_handler(state); });

	V53_DMAU(config, m_v53dmau, 4000000);
	m_v53dmau->out_hreq_callback().set([this] (int state) { m_out_hreq_cb(state); });
	m_v53dmau->out_eop_callback().set([this] (int state) { m_out_eop_cb(state); });
	m_v53dmau->in_memr_callback().set([this] (address_space &space, offs_t offset) { return m_in_memr_cb(space, offset); });
	m_v53dmau->out_memw_callback().set([this] (address_space &space, offs_t offset, uint8_t data) { m_out_memw_cb(space, offset, data); });
	m_v53dmau->in_ior_callback<0>().set([this] (address_space &space, offs_t offset) { return m_in_ior_0_cb(space, offset); });
	m_v53dmau->in_ior_callback<1>().set([this] (address_space &space, offs_t offset) { return m_in_ior_1_cb(space, offset); });
	m_v53dmau->in_ior_callback<2>().set([this] (address_space &space, offs_t offset) { return m_in_ior_2_cb(space, offset); });
	m_v53dmau->in_ior_callback<3>().set([this] (address_space &space, offs_t offset) { return m_in_ior_3_cb(space, offset); });
	m_v53dmau->out_iow_callback<0>().set([this] (address_space &space, offs_t offset, uint8_t data) { m_out_iow_0_cb(space, offset, data); });
	m_v53dmau->out_iow_callback<1>().set([this] (address_space &space, offs_t offset, uint8_t data) { m_out_iow_1_cb(space, offset, data); });
	m_v53dmau->out_iow_callback<2>().set([this] (address_space &space, offs_t offset, uint8_t data) { m_out_iow_2_cb(space, offset, data); });
	m_v53dmau->out_iow_callback<3>().set([this] (address_space &space, offs_t offset, uint8_t data) { m_out_iow_3_cb(space, offset, data); });
	m_v53dmau->out_dack_callback<0>().set([this] (int state) { m_out_dack_0_cb(state); });
	m_v53dmau->out_dack_callback<1>().set([this] (int state) { m_out_dack_1_cb(state); });
	m_v53dmau->out_dack_callback<2>().set([this] (int state) { m_out_dack_2_cb(state); });
	m_v53dmau->out_dack_callback<3>().set([this] (int state) { m_out_dack_3_cb(state); });

	PIC8259(config, m_v53icu, 0);
	m_v53icu->out_int_callback().set(FUNC(v53_base_device::internal_irq_w));
	m_v53icu->in_sp_callback().set_constant(1);
	m_v53icu->read_slave_ack_callback().set(FUNC(v53_base_device::get_pic_ack));

	V53_SCU(config, m_v53scu, 0);
	m_v53scu->txd_handler().set(FUNC(v53_base_device::scu_txd_trampoline_cb));
	m_v53scu->dtr_handler().set(FUNC(v53_base_device::scu_dtr_trampoline_cb));
	m_v53scu->rts_handler().set(FUNC(v53_base_device::scu_rts_trampoline_cb));
	m_v53scu->rxrdy_handler().set(FUNC(v53_base_device::scu_rxrdy_trampoline_cb));
	m_v53scu->txrdy_handler().set(FUNC(v53_base_device::scu_txrdy_trampoline_cb));
	m_v53scu->txempty_handler().set(FUNC(v53_base_device::scu_txempty_trampoline_cb));
	m_v53scu->syndet_handler().set(FUNC(v53_base_device::scu_syndet_trampoline_cb));
}


v53_base_device::v53_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	v33_base_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(v53_base_device::v53_internal_port_map), this)),
	m_v53tcu(*this, "pit"),
	m_v53dmau(*this, "upd71071dma"),
	m_v53icu(*this, "upd71059pic"),
	m_v53scu(*this, "v53scu"),
	// SCU
	m_txd_handler(*this),
	m_dtr_handler(*this),
	m_rts_handler(*this),
	m_rxrdy_handler(*this),
	m_txrdy_handler(*this),
	m_txempty_handler(*this),
	m_syndet_handler(*this),
	// TCU
	m_out0_handler(*this),
	m_out1_handler(*this),
	m_out2_handler(*this),
	// DMAU
	m_out_hreq_cb(*this),
	m_out_eop_cb(*this),
	m_in_memr_cb(*this),
	m_out_memw_cb(*this),
	m_in_ior_0_cb(*this),
	m_in_ior_1_cb(*this),
	m_in_ior_2_cb(*this),
	m_in_ior_3_cb(*this),
	m_out_iow_0_cb(*this),
	m_out_iow_1_cb(*this),
	m_out_iow_2_cb(*this),
	m_out_iow_3_cb(*this),
	m_out_dack_0_cb(*this),
	m_out_dack_1_cb(*this),
	m_out_dack_2_cb(*this),
	m_out_dack_3_cb(*this)
{
}


v53_device::v53_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: v53_base_device(mconfig, V53, tag, owner, clock)
{
}


v53a_device::v53a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: v53_base_device(mconfig, V53A, tag, owner, clock)
{
}
