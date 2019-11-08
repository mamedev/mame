
///
/// \defgroup compiledefine Compilation defines
///
/// Compilation defines can be specified using -DDEFINE=VALUE during compilation.
/// If this is not done, a default is set, e.g.
/// \code
/// #ifndef DEFINE
/// #define DEFINE (0)
/// #endif
/// \endcode
///
/// The define statements below thus show the default value.
///
///\{
///\}

/// \brief The netlist namespace.
/// All netlist related code is contained in the netlist namespace
namespace netlist
{
	/// \brief The netlist::devices namespace.
	/// All netlist devices are contained in the netlist::devices namespace.
	namespace devices
	{
	}

	/// \brief The netlist::analog namespace.
	/// Netlist analog devices and related code are contained in the
	/// netlist::analog namespace.
	namespace analog
	{
	}

	/// \brief The netlist::solver namespace.
	/// All linear equation solving code and related code is contained in the
	/// netlist::solver namespace.
	namespace solver
	{
	}

	/// \brief Internal support classes you should not use in code using netlist api.
	/// This namespace contains all internal support classes not intended
	/// to be used outside of the core.
	namespace detail
	{
	}
}
