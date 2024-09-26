// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "patchbox.h"

#include <algorithm>
#include <iterator>


namespace {

enum : ioport_value
{
	//              DB25    DE9     V.24 circuit
	TXD,        //   2      3       103
	RXD,        //   3      2       104
	RTS,        //   4      7       105
	CTS,        //   5      8       106
	DSR,        //   6      6       107
	DTR,        //  20      4       108/2
	DCD,        //   8      1       109
	SPDS,       //  23              111
	SI,         //                  112
	ETC,        //  24              113
	TXC,        //  15              114
	RXC,        //  17              115
	RI,         //  22      9       125
	ASSERT,
	DEASSERT,

	SIGNAL_COUNT
};


class rs232_patch_box_device: public device_t, public device_rs232_port_interface
{
public:
	rs232_patch_box_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			uint32_t clock)
		: device_t(mconfig, RS232_PATCH_BOX, tag, owner, clock)
		, device_rs232_port_interface(mconfig, *this)
		, m_dce_port(*this, "dce")
		, m_conf(*this, "CONF%u", 0)
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(source_changed)
	{
		unsigned const newsource = m_conf[param]->read() & 0x0f;
		u8 const newsig = m_signals[newsource];
		bool const changed = m_signals[m_sources[param]] != newsig;
		m_sources[param] = newsource;
		if (changed)
		{
			u8 const newout = newsig ^ m_invert[param];
			switch (param)
			{
			case RXD: output_rxd(newout); break;
			case DCD: output_dcd(newout); break;
			case DSR: output_dsr(newout); break;
			case RI:  output_ri(newout);  break;
			case SI:  output_si(newout);  break;
			case CTS: output_cts(newout); break;
			case RXC: output_rxc(newout); break;
			case TXC: output_txc(newout); break;

			case TXD:  m_dce_port->write_txd(newout);  break;
			case DTR:  m_dce_port->write_dtr(newout);  break;
			case RTS:  m_dce_port->write_rts(newout);  break;
			case ETC:  m_dce_port->write_etc(newout);  break;
			case SPDS: m_dce_port->write_spds(newout); break;
			}
		}
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		RS232_PORT(config, m_dce_port, default_rs232_devices, nullptr);
		m_dce_port->rxd_handler().set(FUNC(rs232_patch_box_device::signal_in<RXD>));
		m_dce_port->dcd_handler().set(FUNC(rs232_patch_box_device::signal_in<DCD>));
		m_dce_port->dsr_handler().set(FUNC(rs232_patch_box_device::signal_in<DSR>));
		m_dce_port->ri_handler().set(FUNC(rs232_patch_box_device::signal_in<RI>));
		m_dce_port->si_handler().set(FUNC(rs232_patch_box_device::signal_in<SI>));
		m_dce_port->cts_handler().set(FUNC(rs232_patch_box_device::signal_in<CTS>));
		m_dce_port->rxc_handler().set(FUNC(rs232_patch_box_device::signal_in<RXC>));
		m_dce_port->txc_handler().set(FUNC(rs232_patch_box_device::signal_in<TXC>));
	}

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_resolve_objects() override
	{
		std::fill(std::begin(m_sources), std::end(m_sources), DEASSERT);
		std::fill(std::begin(m_invert), std::end(m_invert), 0);
		std::fill(std::begin(m_signals), std::end(m_signals), 1);
		m_signals[ASSERT] = 0;
	}

	virtual void device_start() override
	{
		save_item(NAME(m_signals));
	}

	virtual void device_reset() override
	{
		for (unsigned i = 0; m_conf.size() > i; ++i)
			m_sources[i] = m_conf[i]->read() & 0x0f;

		output_rxd(m_signals[m_sources[RXD]] ^ m_invert[RXD]);
		output_dcd(m_signals[m_sources[DCD]] ^ m_invert[DCD]);
		output_dsr(m_signals[m_sources[DSR]] ^ m_invert[DSR]);
		output_ri(m_signals[m_sources[RI]] ^ m_invert[RI]);
		output_si(m_signals[m_sources[SI]] ^ m_invert[SI]);
		output_cts(m_signals[m_sources[CTS]] ^ m_invert[CTS]);
		output_rxc(m_signals[m_sources[RXC]] ^ m_invert[RXC]);
		output_txc(m_signals[m_sources[TXC]] ^ m_invert[TXC]);

		m_dce_port->write_txd(m_signals[m_sources[TXD]] ^ m_invert[TXD]);
		m_dce_port->write_dtr(m_signals[m_sources[DTR]] ^ m_invert[DTR]);
		m_dce_port->write_rts(m_signals[m_sources[RTS]] ^ m_invert[RTS]);
		m_dce_port->write_etc(m_signals[m_sources[ETC]] ^ m_invert[ETC]);
		m_dce_port->write_spds(m_signals[m_sources[SPDS]] ^ m_invert[SPDS]);
	}

	virtual void input_txd(int state) override { signal_in<TXD>(state); }
	virtual void input_dtr(int state) override { signal_in<DTR>(state); }
	virtual void input_rts(int state) override { signal_in<RTS>(state); }
	virtual void input_etc(int state) override { signal_in<ETC>(state); }
	virtual void input_spds(int state) override { signal_in<SPDS>(state); }

private:
	template <ioport_value Signal>
	void signal_in(int state)
	{
		state = state ? 1 : 0;
		if (m_signals[Signal] != state)
		{
			m_signals[Signal] = state;

			if (Signal == m_sources[RXD]) output_rxd(state ^ m_invert[RXD]);
			if (Signal == m_sources[DCD]) output_dcd(state ^ m_invert[DCD]);
			if (Signal == m_sources[DSR]) output_dsr(state ^ m_invert[DSR]);
			if (Signal == m_sources[RI])  output_ri(state ^ m_invert[RI]);
			if (Signal == m_sources[SI])  output_si(state ^ m_invert[SI]);
			if (Signal == m_sources[CTS]) output_cts(state ^ m_invert[CTS]);
			if (Signal == m_sources[RXC]) output_rxc(state ^ m_invert[RXC]);
			if (Signal == m_sources[TXC]) output_txc(state ^ m_invert[TXC]);

			if (Signal == m_sources[TXD])  m_dce_port->write_txd(state ^ m_invert[TXD]);
			if (Signal == m_sources[DTR])  m_dce_port->write_dtr(state ^ m_invert[DTR]);
			if (Signal == m_sources[RTS])  m_dce_port->write_rts(state ^ m_invert[RTS]);
			if (Signal == m_sources[ETC])  m_dce_port->write_etc(state ^ m_invert[ETC]);
			if (Signal == m_sources[SPDS]) m_dce_port->write_spds(state ^ m_invert[SPDS]);
		}
	}

	required_device<rs232_port_device> m_dce_port;
	required_ioport_array<13> m_conf;
	u8 m_sources[13];
	u8 m_invert[SIGNAL_COUNT];
	u8 m_signals[SIGNAL_COUNT];
};


#define SOURCE_OPTIONS(prtname, dflt, cnfname) \
		PORT_START(prtname) \
		PORT_CONFNAME(0x0f, dflt, cnfname) PORT_CHANGED_MEMBER(DEVICE_SELF, rs232_patch_box_device, source_changed, dflt) \
		PORT_CONFSETTING(TXD,      "TxD") \
		PORT_CONFSETTING(RXD,      "RxD") \
		PORT_CONFSETTING(RTS,      "RTS") \
		PORT_CONFSETTING(CTS,      "CTS") \
		PORT_CONFSETTING(DSR,      "DSR") \
		PORT_CONFSETTING(DTR,      "DTR") \
		PORT_CONFSETTING(DCD,      "DCD") \
		PORT_CONFSETTING(SPDS,     "SpdS") \
		PORT_CONFSETTING(SI,       "SI") \
		PORT_CONFSETTING(ETC,      "ETC") \
		PORT_CONFSETTING(TXC,      "TxC") \
		PORT_CONFSETTING(RXC,      "RxC") \
		PORT_CONFSETTING(RI,       "RI") \
		PORT_CONFSETTING(ASSERT,   "Asserted") \
		PORT_CONFSETTING(DEASSERT, "Deasserted")

INPUT_PORTS_START(patchbox)
	SOURCE_OPTIONS("CONF0",  TXD, "TxD Source (V.24 103)")
	SOURCE_OPTIONS("CONF1",  RXD, "RxD Source (V.24 104)")
	SOURCE_OPTIONS("CONF2",  RTS, "RTS Source (V.24 105)")
	SOURCE_OPTIONS("CONF3",  CTS, "CTS Source (V.24 106)")
	SOURCE_OPTIONS("CONF4",  DSR, "DSR Source (V.24 107)")
	SOURCE_OPTIONS("CONF5",  DTR, "DTR Source (V.24 108/2)")
	SOURCE_OPTIONS("CONF6",  DCD, "DCD Source (V.24 109)")
	SOURCE_OPTIONS("CONF7",  SPDS,"SpdS Source (V.24 111)")
	SOURCE_OPTIONS("CONF8",  SI,  "SI Source (V.24 112)")
	SOURCE_OPTIONS("CONF9",  ETC, "ETC Source (V.24 113)")
	SOURCE_OPTIONS("CONF10", TXC, "TxC Source (V.24 114)")
	SOURCE_OPTIONS("CONF11", RXC, "RxC Source (V.24 115)")
	SOURCE_OPTIONS("CONF12", RI,  "RI Source (V.24 125)")
INPUT_PORTS_END


ioport_constructor rs232_patch_box_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(patchbox);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(RS232_PATCH_BOX, device_rs232_port_interface, rs232_patch_box_device, "rs232_patch_box", "RS-232 Patch Box")
