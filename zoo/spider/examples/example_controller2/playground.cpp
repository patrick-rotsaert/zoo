#if 0
// Source - https://stackoverflow.com/a
// Posted by sehe, modified by community. See post 'Timeline' for change history
// Retrieved 2025-11-17, License - CC BY-SA 4.0

#include <boost/describe.hpp>
#include <boost/mp11.hpp>
#include <iomanip>
#include <iostream>

namespace bd   = boost::describe;
namespace mp11 = boost::mp11;

namespace MyLib {
struct Metadata
{
	bool             some_flag = false;
	std::string_view some_text = "";
	static void      some_action()
	{
		std::cout << "default action" << std::endl;
	}
};

template<typename D>
constexpr inline Metadata meta_v{};

template<class T,
         class D1 = bd::describe_members<T, bd::mod_public | bd::mod_protected>,
         class En = std::enable_if_t<!std::is_union<T>::value>>
void demo(T const&)
{
	mp11::mp_for_each<D1>([&](auto D) {
		auto const& m = meta_v<decltype(D)>;
		std::cout << "meta for " << D.name << ": { " << m.some_flag << ", " << quoted(m.some_text) << " }" << std::endl;
		if (m.some_flag)
			m.some_action();
	});
}
} // namespace MyLib

// application
namespace MyLib {
struct Foo
{
	int         bar;
	std::string baz;
	double      qux;
};

BOOST_DESCRIBE_STRUCT(Foo, (), (bar, baz, qux))

template<typename C, typename T>
constexpr C deduce_class(T(C::*));

template<auto Mem>
using Class = decltype(deduce_class(Mem));

// a shorthand, you might want to make this more generically elegant by deducing `Foo` instead
template<auto Mem>
using Desc = bd::descriptor_by_pointer<bd::describe_members<Class<Mem>, bd::mod_any_access>, Mem>;

// specialize some metadata
template<>
constexpr inline Metadata meta_v<Desc<&Foo::baz>>{ true, "hello" };

struct Special
{
	double           some_flag = 42e-2;
	std::string_view some_text = "specialized!";
	static void      some_action()
	{
		std::cerr << "stderr instead" << std::endl;
	}
};
template<>
constexpr inline Special meta_v<Desc<&Foo::qux>>{};

} // namespace MyLib

int main()
{
	std::cout << std::boolalpha;
	demo(MyLib::Foo{});
}
#endif

#if 0
// Source - https://stackoverflow.com/a
// Posted by sehe, modified by community. See post 'Timeline' for change history
// Retrieved 2025-11-17, License - CC BY-SA 4.0

#include <boost/describe.hpp>
#include <boost/mp11.hpp>
#include <iomanip>
#include <iostream>

namespace bd   = boost::describe;
namespace mp11 = boost::mp11;

namespace MyLib {
template<typename D>
constexpr inline bool flag_v{};

template<class T,
         class D1 = bd::describe_members<T, bd::mod_public | bd::mod_protected>,
         class F1 = bd::describe_members<T, bd::mod_public | bd::mod_protected | bd::mod_function>,
         class En = std::enable_if_t<!std::is_union<T>::value>>
void demo(T const&)
{
	mp11::mp_for_each<D1>([&](auto D) { std::cout << "flag for " << D.name << ": " << flag_v<decltype(D)> << std::endl; });
	mp11::mp_for_each<F1>([&](auto D) { std::cout << "flag for " << D.name << ": " << flag_v<decltype(D)> << std::endl; });
}

namespace detail {

template<typename C, typename T>
constexpr C deduce_class(T(C::*));

template<auto Mem>
using Class = decltype(deduce_class(Mem));

template<auto Mem, typename /*Enable*/ = void>
struct DescF;

template<auto Mem>
struct DescF<Mem, std::enable_if_t<std::is_member_function_pointer_v<decltype(Mem)>>>
{
	using type = bd::descriptor_by_pointer<bd::describe_members<Class<Mem>, bd::mod_any_access | bd::mod_function>, Mem>;
};

template<auto Mem>
struct DescF<Mem, std::enable_if_t<not std::is_member_function_pointer_v<decltype(Mem)>>>
{
	using type = bd::descriptor_by_pointer<bd::describe_members<Class<Mem>, bd::mod_any_access>, Mem>;
};
} // namespace detail

template<auto Mem>
using Desc = typename detail::DescF<Mem>::type;
} // namespace MyLib

// application
namespace MyApp {

struct Foo
{
	int         bar;
	std::string baz;
	double      qux;
};

struct Bar
{
	int quuz(double)
	{
		return 42;
	}
};

BOOST_DESCRIBE_STRUCT(Foo, (), (bar, baz, qux))
BOOST_DESCRIBE_STRUCT(Bar, (), (quuz))

} // namespace MyApp

using MyLib::Desc;

// specialize some metadata
template<>
auto MyLib::flag_v<Desc<&MyApp::Foo::baz>> = true;
template<>
auto MyLib::flag_v<Desc<&MyApp::Foo::qux>> = 42;
template<>
auto MyLib::flag_v<Desc<&MyApp::Bar::quuz>> = "fun";

int main()
{
	std::cout << std::boolalpha;
	MyLib::demo(MyApp::Foo{});
	MyLib::demo(MyApp::Bar{});
}
#endif

#if 0
#include <boost/describe.hpp>
#include <boost/mp11.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <type_traits>

namespace describe = boost::describe;
namespace mp11     = boost::mp11;
using namespace std::literals; // Enables usage of "..."sv

// -------------------------------------------------------------
// 1. APPLICATION STRUCT DEFINITION
// -------------------------------------------------------------

namespace MyApp {
struct UserProfile
{
	int         id;
	std::string username;
	bool        is_active;
};
} // namespace MyApp

// 2. APPLY BOOST_DESCRIBE_STRUCT (Declare descriptor helper functions)
BOOST_DESCRIBE_STRUCT(MyApp::UserProfile, (), (id, username, is_active))

// -------------------------------------------------------------
// 3. METADATA SYSTEM DEFINITION (Minimalist and Robust)
// -------------------------------------------------------------

namespace MyMetadata {
// Primary Variable Template Definition
template<typename D>
constexpr inline std::string_view description_v{};

// --- Deduction Helpers (Simple version required by Desc) ---
// This function is never defined; it's only used for decltype deduction.
template<class C, class T>
constexpr C deduce_class(T C::*);

template<auto Mem>
using ClassType = decltype(deduce_class(Mem));
// --- End Deduction Helpers ---

/**
     * Alias to derive the unique Descriptor Type for a pointer-to-member.
     * This relies on the BOOST_DESCRIBE_STRUCT helpers being declared above.
     */
template<auto Mem>
using Desc = typename describe::descriptor_by_pointer<describe::describe_members<ClassType<Mem>, // Use the deduced class type
                                                                                 describe::mod_any_access>,
                                                      Mem>::type;
} // namespace MyMetadata

// -------------------------------------------------------------
// 4. EXPLICIT METADATA SPECIALIZATION
// -------------------------------------------------------------

using MyMetadata::Desc;
using namespace std::literals;

// Specialize for 'id'
template<>
inline constexpr std::string_view MyMetadata::description_v<Desc<&MyApp::UserProfile::id>> = "Unique numerical identifier for the user."sv;

// Specialize for 'username'
template<>
inline constexpr std::string_view MyMetadata::description_v<Desc<&MyApp::UserProfile::username>> =
    "User's chosen display name (must be unique)."sv;

// -------------------------------------------------------------
// 5. USAGE AND TESTING FUNCTION
// -------------------------------------------------------------

void print_described_metadata()
{
	using T = MyApp::UserProfile;

	std::cout << "--- Metadata for UserProfile ---\n";

	// Iterate over all public members of UserProfile
	mp11::mp_for_each<describe::describe_members<T, describe::mod_public>>([&](auto D_instance) {
		// 1. Get the descriptor TYPE from the instance D_instance
		using DescriptorType = decltype(D_instance);

		// 2. Access the metadata
		std::string_view description = MyMetadata::description_v<DescriptorType>;

		// 3. Get the Member Type (Fixing the D::pointer issue you had before)
		using MemberPointerType = decltype(DescriptorType::pointer);
		using MemberType        = typename std::remove_pointer_t<MemberPointerType>::type;

		std::cout << "Member: " << std::setw(12) << std::left << D_instance::name << " | Type: " << std::setw(10) << std::left
		          << typeid(MemberType).name() << " | Description: " << (description.empty() ? "(No custom description)" : description)
		          << "\n";
	});
	std::cout << "------------------------------------------\n";
}

int main()
{
	print_described_metadata();
	return 0;
}
#endif

#if 0
// Copyright(C) 2021 Samuel Debionne, ESRF.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0.(See accompanying file LICENSE_1_0.txt or copy at
// http: //www.boost.org/LICENSE_1_0.txt)

#include <type_traits>

#include <boost/describe.hpp>
#include <boost/mp11.hpp>
#include <boost/callable_traits/class_of.hpp>
#include <boost/callable_traits/return_type.hpp>

#include <chrono>
#include <iostream>

// namespace boost
// {
// namespace describe
// {
//     // descriptor_by_pointer<>
//     namespace detail
//     {
//         template <auto pointer>
//         struct by_pointer
//         {
//             template <typename D, typename T1, typename T2>
//             struct equal { static constexpr bool value = false; };

//             template <typename D, typename T>
//             struct equal<D, T, T> { static constexpr bool value = (pointer == D::pointer); };

//             template <typename D>
//             struct fn
//             {
//                 static constexpr bool value =
//                     equal<D, std::decay_t<decltype(pointer)>, std::decay_t<decltype(D::pointer)>>::value;
//             };
//         };

//     } //namespace detail

//     template <typename Md, auto pointer, typename P = detail::by_pointer<pointer>>
//     using descriptor_by_pointer = mp11::mp_at<Md, mp11::mp_find_if_q<Md, P>>;

//     // is_described<>
//     template <typename T, typename Enable = void>
//     struct is_described : std::false_type {};

//     template <typename T>
//     struct is_described<T, std::void_t<describe_members<T, mod_any_access>>> : std::true_type {};

//     template <class T>
//     inline constexpr bool is_described_v = is_described<T>::value;

// } //namespace describe
// } //namespace boost

template<auto pointer>
using descriptor_by_pointer = boost::describe::descriptor_by_pointer<
    boost::describe::describe_members<boost::callable_traits::class_of_t<decltype(pointer)>, boost::describe::mod_any_access>,
    pointer>;

/// Annotations of struct members
///
/// \tparam Md Member Descriptor
template<typename Md>
struct annotations;

#define APP_ANNOTATE(ptm, arg_desc, arg_doc, arg_validate)                                                                                 \
	template<>                                                                                                                             \
	struct annotations<descriptor_by_pointer<ptm>>                                                                                         \
	{                                                                                                                                      \
		inline static const char* desc = (arg_desc);                                                                                       \
		inline static const char* doc  = (arg_doc);                                                                                        \
                                                                                                                                           \
		inline static auto validate = (arg_validate);                                                                                      \
	};

namespace app {
template<typename T>
struct point
{
	T x;
	T y;
};

template<typename T>
struct rectangle
{
	point<T> topleft;
	point<T> dimensions;
};

BOOST_DESCRIBE_STRUCT(rectangle<std::ptrdiff_t>, (), (topleft, dimensions))

enum class acq_mode_enum : int
{
	normal,      //!< Single image
	accumulation //!< Multiple image accumulated (over time)
};

BOOST_DESCRIBE_ENUM(acq_mode_enum, normal, accumulation)

struct acquisition
{
	int                        nb_frames = 1;                       // POD
	std::chrono::duration<int> expo_time = std::chrono::seconds(1); // Class
	acq_mode_enum              acq_mode  = acq_mode_enum::normal;   // Enumerator
	rectangle<std::ptrdiff_t>  roi;                                 // Described class
};

BOOST_DESCRIBE_STRUCT(acquisition, (), (nb_frames, expo_time, acq_mode, roi))

} //namespace app

APP_ANNOTATE(&app::acquisition::nb_frames,
             "number of frames",
             "The number of frames to acquire (0 = continuous acquisition)",
             [](auto const& val) { return val > 0; })

APP_ANNOTATE(&app::acquisition::expo_time, "exposure time", "The exposure time [s]", [](auto const& val) { return val > 0; })

APP_ANNOTATE(&app::acquisition::acq_mode, "acquisition mode", "The acquistion mode [normal, accumulation]", [](auto const& val) {
	return true;
})

APP_ANNOTATE(&app::acquisition::roi, "region of interest", "The region of interest to transfer", [](auto const& val) {
	return val.validate();
})

APP_ANNOTATE(&app::rectangle<std::ptrdiff_t>::topleft,
             "top left corner coordinate",
             "The top left corner coordinate of the region of interest to transfer",
             [](auto const& val) { return val.validate(); })

// APP_ANNOTATE(&app::rectangle<std::ptrdiff_t>::dimensions,
//              "dimensions",
//              "The dimensions of the region of interest to transfer",
//              [](auto const& val) { return val.validate(); })

// Insert stream operator for std::chrono::duration
template<typename Rep>
std::ostream& operator<<(std::ostream& os, std::chrono::duration<Rep> const& duration)
{
	os << duration.count() << "s";
	return os;
}

// Generic insert stream operator for enumerators
template<typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
std::ostream& operator<<(std::ostream& os, E const& e)
{
	char const* r = "(unnamed)";

	boost::mp11::mp_for_each<boost::describe::describe_enumerators<E>>([&](auto D) {
		if (e == D.value)
			os << D.name;
	});

	return os;
}

// Insert stream operator for app structures
template<typename T>
std::ostream& operator<<(std::ostream& os, app::point<T> const& p)
{
	os << "{" << p.x << "," << p.x << "}";
	return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, app::rectangle<T> const& r)
{
	os << "{" << r.topleft.x << "," << r.topleft.y << "}," << "{" << r.dimensions.x << "x" << r.dimensions.y << "}";
	return os;
}

// Recursively print described structures including annotations
template<class T, typename Md = boost::describe::describe_members<T, boost::describe::mod_any_access>>
void print_with_annotation(std::ostream& os, T const& t, int indent = 0)
{
	bool first = true;
	boost::mp11::mp_for_each<Md>([&](auto D) {
		for (int i = 0; i < indent; i++)
			os << "  ";

		using A = annotations<decltype(D)>;
		os << "." << D.name << " = " << t.*D.pointer << "\n\tdesc: " << A::desc << "\n\tdoc: " << A::doc << std::endl;

		// Recursively print class members if they are described
		using return_t = std::decay_t<boost::callable_traits::return_type_t<decltype(D.pointer)>>;
		if constexpr (std::is_class_v<return_t> && boost::describe::has_describe_members<return_t>::value)
			print_with_annotation(os, t.*D.pointer, ++indent);
	});
}

int main(int argc, const char* argv[])
{
	using namespace std::chrono_literals;
	using namespace app;

	acquisition acq{ 100, 1s, acq_mode_enum::accumulation, { { 0, 0 }, { 1024, 1024 } } };

	print_with_annotation(std::cout, acq);

	return 0;
}

//$ ./gist
//.nb_frames = 100
//        desc: number of frames
//        doc: The number of frames to acquire (0 = continuous acquisition)
//.expo_time = 1s
//        desc: exposure time
//        doc: The exposure time [s]
//.acq_mode = accumulation
//        desc: acquisition mode
//        doc: The acquistion mode [normal, accumulation]
//.roi = {0,0},{1024x1024}
//        desc: region of interest
//        doc: The region of interest to transfer
//  .topleft = {0,0}
//        desc: top left corner coordinate
//        doc: The top left corner coordinate of the region of interest to transfer
//  .dimensions = {1024,1024}
//        desc: dimensions
//        doc: The dimensions of the region of interest to transfer

#endif

#if 0
// Source - https://stackoverflow.com/a
// Posted by sehe, modified by community. See post 'Timeline' for change history
// Retrieved 2025-11-17, License - CC BY-SA 4.0

#include <boost/describe.hpp>
#include <boost/mp11.hpp>
#include <iomanip>
#include <iostream>

namespace bd   = boost::describe;
namespace mp11 = boost::mp11;

namespace MyLib {

struct Metadata
{
	bool             some_flag = false;
	std::string_view some_text = "";
	static void      some_action()
	{
		std::cout << "default action" << std::endl;
	}
};

template<typename D>
constexpr inline Metadata meta_v{};

template<class T,
         class D1 = bd::describe_members<T, bd::mod_public | bd::mod_protected>,
         class En = std::enable_if_t<!std::is_union<T>::value>>
void demo(T const&)
{
	mp11::mp_for_each<D1>([&](auto D) {
		auto const& m = meta_v<decltype(D)>;
		std::cout << "meta for " << D.name << ": { " << m.some_flag << ", " << quoted(m.some_text) << " }" << std::endl;
		if (m.some_flag)
			m.some_action();
	});
}

namespace detail {

template<typename C, typename T>
constexpr C deduce_class(T(C::*));

template<auto Mem>
using Class = decltype(deduce_class(Mem));

} // namespace detail

template<auto Mem>
using Desc = bd::descriptor_by_pointer<bd::describe_members<detail::Class<Mem>, bd::mod_any_access>, Mem>;

} // namespace MyLib

// application
namespace MyApp {

struct Foo
{
	int         bar;
	std::string baz;
	double      qux;
};

BOOST_DESCRIBE_STRUCT(Foo, (), (bar, baz, qux))

// template<typename C, typename T>
// constexpr C deduce_class(T(C::*));

// template<auto Mem>
// using Class = decltype(deduce_class(Mem));

// // a shorthand, you might want to make this more generically elegant by deducing `Foo` instead
// template<auto Mem>
// using Desc = bd::descriptor_by_pointer<bd::describe_members<Class<Mem>, bd::mod_any_access>, Mem>;

// specialize some metadata

// struct Special
// {
// 	double           some_flag = 42e-2;
// 	std::string_view some_text = "specialized!";
// 	static void      some_action()
// 	{
// 		std::cerr << "stderr instead" << std::endl;
// 	}
// };
// template<>
// constexpr inline Special meta_v<Desc<&Foo::qux>>{};

} // namespace MyApp

using MyLib::Desc;
using MyLib::Metadata;

template<>
constexpr inline Metadata MyLib::meta_v<Desc<&MyApp::Foo::baz>>{ true, "hello" };

int main()
{
	std::cout << std::boolalpha;
	MyLib::demo(MyApp::Foo{});
}
#endif

#if 1
// Source - https://stackoverflow.com/a
// Posted by sehe, modified by community. See post 'Timeline' for change history
// Retrieved 2025-11-17, License - CC BY-SA 4.0

#include <boost/describe.hpp>
#include <boost/mp11.hpp>
#include <string_view>
#include <iomanip>
#include <iostream>

namespace bd   = boost::describe;
namespace mp11 = boost::mp11;

namespace MyLib {

template<typename D>
constexpr inline std::string_view annotation_v{};

template<class T,
         class D1 = bd::describe_members<T, bd::mod_public | bd::mod_protected>,
         class En = std::enable_if_t<!std::is_union<T>::value>>
void demo(T const&)
{
	mp11::mp_for_each<D1>([&](auto D) {
		auto const& m = annotation_v<decltype(D)>;
		std::cout << "meta for " << D.name << ": " << m << std::endl;
	});
}

namespace detail {

template<typename C, typename T>
constexpr C deduce_class(T(C::*));

template<auto Mem>
using Class = decltype(deduce_class(Mem));

} // namespace detail

template<auto Mem>
using Desc = bd::descriptor_by_pointer<bd::describe_members<detail::Class<Mem>, bd::mod_any_access>, Mem>;

} // namespace MyLib

// application
namespace MyApp {

struct Foo
{
	int         bar;
	std::string baz;
	double      qux;
};

BOOST_DESCRIBE_STRUCT(Foo, (), (bar, baz, qux))

// template<typename C, typename T>
// constexpr C deduce_class(T(C::*));

// template<auto Mem>
// using Class = decltype(deduce_class(Mem));

// // a shorthand, you might want to make this more generically elegant by deducing `Foo` instead
// template<auto Mem>
// using Desc = bd::descriptor_by_pointer<bd::describe_members<Class<Mem>, bd::mod_any_access>, Mem>;

// specialize some metadata

// struct Special
// {
// 	double           some_flag = 42e-2;
// 	std::string_view some_text = "specialized!";
// 	static void      some_action()
// 	{
// 		std::cerr << "stderr instead" << std::endl;
// 	}
// };
// template<>
// constexpr inline Special meta_v<Desc<&Foo::qux>>{};

} // namespace MyApp

using MyLib::Desc;
// using MyLib::Metadata;

template<>
constexpr inline std::string_view MyLib::annotation_v<Desc<&MyApp::Foo::baz>>{ "hello" };

struct named_parameter
{
	std::string name;
	std::string description;
};

struct path_parameter : named_parameter
{
};

int main()
{
	std::cout << std::boolalpha;
	MyLib::demo(MyApp::Foo{});

	auto p1 = path_parameter{ "name", "desc" };
	(void)p1;

	auto p2 = path_parameter{ "name" };
	(void)p2;
}
#endif
