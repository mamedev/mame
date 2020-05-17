// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_roms.cpp
 *
 */

#include "netlist/nl_base.h"
#include "netlist/nl_setup.h"

template <typename N, typename T>
constexpr bool TOR(N n, T &a)
{
	return (n == 0 ? false : TOR(n-1, a) || a[n-1]());
}

template <typename T>
constexpr bool TOR(T &a)
{
	return TOR(a.size(), a);
}

namespace netlist
{
	namespace devices
	{

		template <typename D>
		NETLIB_OBJECT(generic_prom)
		{
			NETLIB_CONSTRUCTOR(generic_prom)
			, m_enabled(*this, "m_enabled", true)
			, m_A(*this, 0, "A{}", NETLIB_DELEGATE(generic_prom, addr))
			, m_CEQ(*this, 1, D::chip_enable_mask ^ static_cast<size_t>(0xffff), pstring("CE{}"),
				std::array<nldelegate, 3>{ NETLIB_DELEGATE(generic_prom, ce<0>),
				  NETLIB_DELEGATE(generic_prom, ce<1>),
				  NETLIB_DELEGATE(generic_prom, ce<2>)})
			, m_O(*this, D::data_name_offset, "O{}")
			, m_ROM(*this, "ROM")
			, m_power_pins(*this)
			{
			}

			using data_type = typename plib::least_type_for_bits<D::data_width>::type;

		private:

			template <std::size_t N>
			inline NETLIB_HANDLERI(ce)
			{
				using cet = typename D::chip_enable_time;
				m_enabled = (m_CEQ() == D::chip_enable_mask);
				const auto delay = m_enabled ? D::access_time::value() : cet::value(N);
				const data_type o = m_enabled ? m_ROM[m_A()] : (1 << D::data_width) - 1; // FIXME tristate !

				m_O.push(o, delay);
			}

			inline
			NETLIB_HANDLERI(addr)
			{
				if (m_enabled)
				{
					// FIXME: Outputs are tristate. This needs to be properly implemented
					m_O.push(m_ROM[m_A()], D::access_time::value());
				}
			}

			NETLIB_UPDATEI()
			{
				ce<0>(); // only called during setup
			}

			state_var<bool> m_enabled;
			object_array_t<logic_input_t, D::address_width> m_A;
			object_array_t<logic_input_t, D::chip_enable_inputs> m_CEQ;
			object_array_t<typename D::output_type, D::data_width> m_O;

			param_rom_t<uint8_t, D::address_width, D::data_width> m_ROM;
			nld_power_pins m_power_pins;
		};

		struct desc_82S126
		{
			static constexpr const size_t address_width = 8;
			static constexpr const size_t data_width = 4;
			static constexpr const size_t data_name_offset = 1; // O1, O2, ..
			static constexpr const size_t chip_enable_inputs = 2;
			// MATCH_MASK : all 0 ==> all bits inverted
			static constexpr const size_t chip_enable_mask = 0x00;

			using chip_enable_time = times_ns2<25, 25>;
			using access_time = time_ns<40>;

			using output_type = logic_output_t;
		};

		struct desc_74S287 : public desc_82S126
		{
			using chip_enable_time = times_ns2<15, 15>;
			using access_time = time_ns<35>;
		};

		struct desc_82S123
		{
			// FIXME: tristate outputs, add 82S23 (open collector)
			static constexpr const size_t address_width = 5;
			static constexpr const size_t data_width = 8;
			static constexpr const size_t data_name_offset = 1; // O1, O2, ..
			static constexpr const size_t chip_enable_inputs = 1;
			// MATCH_MASK : all 0 ==> all bits inverted
			static constexpr const size_t chip_enable_mask = 0x00;

			using chip_enable_time = times_ns1<35>;
			using access_time = time_ns<45>;

			using output_type = logic_output_t;
		};

		struct desc_2716
		{
			// FIXME: tristate outputs
			static constexpr const size_t address_width = 11;
			static constexpr const size_t data_width = 8;
			static constexpr const size_t data_name_offset = 0; // O0, O1, ..

			static constexpr const size_t chip_enable_inputs = 2;
			// MATCH_MASK : all 0 ==> all bits inverted
			static constexpr const size_t chip_enable_mask = 0x00;

			using chip_enable_time = times_ns2<450, 100>; //CE, OE
			using access_time = time_ns<450>;

			using output_type = logic_output_t;
		};


	using NETLIB_NAME(82S123) = NETLIB_NAME(generic_prom)<desc_82S123>; // 256 bits, 32x8, used as 256x4
	using NETLIB_NAME(82S126) = NETLIB_NAME(generic_prom)<desc_82S126>; // 1024 bits, 32x32, used as 256x4
	using NETLIB_NAME(74S287) = NETLIB_NAME(generic_prom)<desc_74S287>; // 1024 bits, 32x32, used as 256x4
	using NETLIB_NAME(2716)   = NETLIB_NAME(generic_prom)<desc_2716>;   // CE2Q = OE, CE1Q = CE

	NETLIB_DEVICE_IMPL(82S126,     "PROM_82S126",     "+CE1Q,+CE2Q,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74S287,     "PROM_74S287",     "+CE1Q,+CE2Q,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,@VCC,@GND")
	NETLIB_DEVICE_IMPL(82S123,     "PROM_82S123",     "+CEQ,+A0,+A1,+A2,+A3,+A4,@VCC,@GND")
	NETLIB_DEVICE_IMPL(2716,       "EPROM_2716",      "+CE2Q,+CE1Q,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+A9,+A10,@VCC,@GND")

	} //namespace devices
} // namespace netlist
