// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "dynamicclass.ipp"

#include <locale>
#include <new>
#include <sstream>


namespace util {

namespace detail {

/// \brief Complete object locator equivalent structure
///
/// Structure used for locating the complete object and type information
/// for the MSVC C++ ABI on 64-bit targets.  A pointer to this structure
/// appears immediately before the first virtual member function entry
/// in the virtual function table.
struct dynamic_derived_class_base::msvc_complete_object_locator_equiv
{
	unsigned long signature;        ///< Magic number, always 1
	unsigned long offset;           ///< Offset from complete object to current object
	unsigned long ctor_disp_offset; ///< For avoiding extra constructor virtual tables for virtual inheritance
	int type_info_offs;             ///< Offset to type info
	int class_info_offs;            ///< Offset to class hierarchy info
	int self_offs;                  ///< Offset to this structure for calculating module base
};


/// \brief Type info equivalent structure
///
/// Structure equivalent to the implementation of std::type_info for the
/// MSVC C++ ABI.  The structure is followed immediately by the
/// decorated name.  The pointer to the undecorated name is normally
/// populated lazily when the \c name member function is called.
struct dynamic_derived_class_base::msvc_type_info_equiv
{
	void const *vptr;           ///< Pointer to virtual table
	char const *undecorated;    ///< Pointer to the undecorated name
	char decorated[1];          ///< First character of the decorated name
};


/// \brief Construct dynamic derived class base
///
/// Creates type info for a class with a single base class using the
/// specified name.  If the name contains multiple components separated
/// by \c :: separators, it is interpreted as a nested class name.
/// \param [in] name The name for the dynamic class.  Components must
///   start with an alphabetic character or an underscore, and may
///   contain only alphanumeric characters and underscores.
/// \exception std::invalid_argument Thrown if the class name is invalid
///   or unsupported.
/// \exception std::bad_alloc Thrown if allocating memory for the type
///   info fails.
dynamic_derived_class_base::dynamic_derived_class_base(std::string_view name) :
	m_base_vtable(nullptr)
{
	assert(!reinterpret_cast<void *>(std::uintptr_t(static_cast<void (*)()>(nullptr))));
	assert(!reinterpret_cast<void (*)()>(std::uintptr_t(static_cast<void *>(nullptr))));

	std::locale const &clocale(std::locale::classic());
	if (name.empty() || (('_' != name[0]) && !std::isalpha(name[0], clocale)) || (std::find_if_not(name.begin(), name.end(), [&clocale] (char c) { return (':' == c) || ('_' == c) || std::isalnum(c, clocale); }) != name.end()))
		throw std::invalid_argument("Invalid class name");

	std::ostringstream str;
	str.imbue(clocale);

#if MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC
	str << "class " << name;
	m_name = std::move(str).str();
	str.str(std::string());

	str << ".?AV";
	std::string_view::size_type found = name.find(':');
	while (std::string_view::npos != found)
	{
		if (((found + 2) >= name.length()) || (name[found + 1] != ':') || (('_' != name[found + 2]) && !std::isalpha(name[found + 2], clocale)))
			throw std::invalid_argument("Invalid class name");

		str.write(&name[0], found);
		str << '@';
		name.remove_prefix(found + 2);
		found = name.find(':');
	}
	str << name << "@@";
	std::string const mangled = std::move(str).str();

	m_type_info = reinterpret_cast<msvc_type_info_equiv *>(
			operator new (
				offsetof(msvc_type_info_equiv, decorated) + mangled.length() + 1,
				std::align_val_t(alignof(msvc_type_info_equiv))));
	m_type_info->vptr = *reinterpret_cast<void const *const *>(&typeid(dynamic_derived_class_base));
	m_type_info->undecorated = m_name.c_str();
	std::copy_n(mangled.c_str(), mangled.length() + 1, m_type_info->decorated);
#else
	class base { };
	class derived : base { };

	m_type_info.vptr = *reinterpret_cast<void const *const *>(&typeid(derived));

	std::string_view::size_type found = name.find(':');
	bool const qualified = std::string::npos != found;
	if (qualified)
		str << 'N';
	while (std::string_view::npos != found)
	{
		if (((found + 2) >= name.length()) || (name[found + 1] != ':') || (('_' != name[found + 2]) && !std::isalpha(name[found + 2], clocale)))
			throw std::invalid_argument("Invalid class name");

		str << found;
		str.write(&name[0], found);
		name.remove_prefix(found + 2);
		found = name.find(':');
	}
	str << name.length() << name;
	if (qualified)
		str << 'E';
	m_name = std::move(str).str();

	m_type_info.name = m_name.c_str();
#endif
}


dynamic_derived_class_base::~dynamic_derived_class_base()
{
#if MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC
	operator delete (m_type_info, std::align_val_t(alignof(msvc_type_info_equiv)));
#endif
}


/// \brief Get virtual table index for member function
///
/// Gets the virtual table index represented by a pointer to a virtual
/// member function.  The \p slot argument must refer to a virtual
/// member function returning a supported type that does not require
/// \c this pointer adjustment.
/// \param [in] slot Internal representation of pointer to a virtual
///   member function.  May be modified.
/// \param [in] size Size of the member function pointer type for the
///   \p slot argument.
/// \return The virtual table index of the member function, in terms of
///   the size of a virtual member function in the virtual table.
/// \exception std::invalid_argument Thrown if the \p slot argument is
///   not a supported virtual member function.
std::size_t dynamic_derived_class_base::resolve_virtual_member_slot(
		member_function_pointer_equiv &slot,
		std::size_t size)
{
#if MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC
	if ((sizeof(msvc_mi_member_function_pointer_equiv) <= size) && slot.adj)
		throw std::invalid_argument("Member function requires this pointer adjustment");
#if defined(__x86_64__) || defined(_M_X64)
	std::uint8_t const *func = reinterpret_cast<std::uint8_t const *>(slot.ptr);
	while (0xe9 == func[0]) // relative jump with 32-bit displacement (typically a resolved PLT entry)
		func += std::ptrdiff_t(5) + *reinterpret_cast<std::int32_t const *>(func + 1);
	if ((0x48 == func[0]) && (0x8b == func[1]) && (0x01 == func[2]))
	{
		if ((0xff == func[3]) && ((0x20 == func[4]) || (0x60 == func[4]) || (0xa0 == func[4])))
		{
			// MSVC virtual function call thunk - mov rax,QWORD PTR [rcx] ; jmp QWORD PTR [rax+...]
			if (0x20 == func[4]) // no displacement
			{
				return 0;
			}
			else if (0x60 == func[4]) // 8-bit displacement
			{
				auto const index = *reinterpret_cast<std::int8_t const *>(func + 5);
				if (index % (sizeof(std::uintptr_t) * MEMBER_FUNCTION_SIZE))
					throw std::invalid_argument("Invalid member function virtual table index");
				return index / sizeof(std::uintptr_t) / MEMBER_FUNCTION_SIZE;
			}
			else // 32-bit displacement
			{
				auto const index = *reinterpret_cast<std::int32_t const *>(func + 5);
				if (index % (sizeof(std::uintptr_t) * MEMBER_FUNCTION_SIZE))
					throw std::invalid_argument("Invalid member function virtual table index");
				return index / sizeof(std::uintptr_t) / MEMBER_FUNCTION_SIZE;
			}
		}
		else if ((0x48 == func[3]) && (0x8b == func[4]))
		{
			// clang virtual function call thunk - mov rax,QWORD PTR [rcx] ; mov rax,QWORD PTR [rax+...] ; jmp rax
			if  ((0x00 == func[5]) && (0x48 == func[6]) && (0xff == func[7]) && (0xe0 == func[8]))
			{
				// no displacement
				return 0;
			}
			else if  ((0x40 == func[5]) && (0x48 == func[7]) && (0xff == func[8]) && (0xe0 == func[9]))
			{
				// 8-bit displacement
				auto const index = *reinterpret_cast<std::int8_t const *>(func + 6);
				if (index % (sizeof(std::uintptr_t) * MEMBER_FUNCTION_SIZE))
					throw std::invalid_argument("Invalid member function virtual table index");
				return index / sizeof(std::uintptr_t) / MEMBER_FUNCTION_SIZE;
			}
			else if ((0x80 == func[5]) && (0x48 == func[10]) && (0xff == func[11]) && (0xe0 == func[12]))
			{
				// 32-bit displacement
				auto const index = *reinterpret_cast<std::int32_t const *>(func + 6);
				if (index % (sizeof(std::uintptr_t) * MEMBER_FUNCTION_SIZE))
					throw std::invalid_argument("Invalid member function virtual table index");
				return index / sizeof(std::uintptr_t) / MEMBER_FUNCTION_SIZE;
			}
		}
	}
	throw std::invalid_argument("Not a supported pointer to a virtual member function");
#else
	throw std::runtime_error("Unsupported architecture");
#endif
#else
	if (!slot.is_virtual())
		throw std::invalid_argument("Not a pointer to a virtual member function");
	if (slot.this_pointer_offset())
		throw std::invalid_argument("Member function requires this pointer adjustment");
	if (MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_STANDARD)
		slot.ptr -= 1;
	if (slot.ptr % (sizeof(std::uintptr_t) * MEMBER_FUNCTION_SIZE))
		throw std::invalid_argument("Invalid member function virtual table index");
	return slot.ptr / sizeof(std::uintptr_t) / MEMBER_FUNCTION_SIZE;
#endif
}

} // namespace detail

} // namespace util
