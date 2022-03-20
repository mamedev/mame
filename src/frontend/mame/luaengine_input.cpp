// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Luca Bruno
/***************************************************************************

    luaengine_input.cpp

    Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "luaengine.ipp"

#include "iptseqpoll.h"

#include "inputdev.h"
#include "natkeyboard.h"
#include "render.h"
#include "uiinput.h"

#include <cstring>


namespace {

struct natkbd_kbd_dev
{
	natkbd_kbd_dev(natural_keyboard &m, std::size_t i) : manager(m), index(i) { }

	natural_keyboard &manager;
	std::size_t index;
};


struct natkbd_kbd_list
{
	natkbd_kbd_list(natural_keyboard &m) : manager(m) { }

	natural_keyboard &manager;
};

} // anonymous namespace


namespace sol {

template <> struct is_container<natkbd_kbd_list> : std::true_type { };


template <>
struct usertype_container<natkbd_kbd_list> : lua_engine::immutable_container_helper<natkbd_kbd_list>
{
private:
	template <bool Indexed>
	static int next_pairs(lua_State *L)
	{
		natkbd_kbd_dev &i(stack::unqualified_get<user<natkbd_kbd_dev> >(L, 1));
		if (i.manager.keyboard_count() <= i.index)
			return stack::push(L, lua_nil);
		int result;
		if constexpr (Indexed)
			result = stack::push(L, i.index + 1);
		else
			result = stack::push(L, i.manager.keyboard_device(i.index).tag());
		result += stack::push(L, i);
		++i.index;
		return result;
	}

	template <bool Indexed>
	static int start_pairs(lua_State *L)
	{
		natkbd_kbd_list &self(get_self(L));
		stack::push(L, next_pairs<Indexed>);
		stack::push<user<natkbd_kbd_dev> >(L, self.manager, 0);
		stack::push(L, lua_nil);
		return 3;
	}

public:
	static int at(lua_State *L)
	{
		natkbd_kbd_list &self(get_self(L));
		std::ptrdiff_t const index(stack::unqualified_get<std::ptrdiff_t>(L, 2));
		if ((0 < index) && (self.manager.keyboard_count() >= index))
			return stack::push(L, natkbd_kbd_dev(self.manager, index - 1));
		else
			return stack::push(L, lua_nil);
	}

	static int get(lua_State *L)
	{
		natkbd_kbd_list &self(get_self(L));
		char const *const tag(stack::unqualified_get<char const *>(L));
		for (std::size_t i = 0; self.manager.keyboard_count() > i; ++i)
		{
			if (!std::strcmp(self.manager.keyboard_device(i).tag(), tag))
				return stack::push(L, natkbd_kbd_dev(self.manager, i));
		}
		return stack::push(L, lua_nil);
	}

	static int index_get(lua_State *L)
	{
		return get(L);
	}

	static int size(lua_State *L)
	{
		natkbd_kbd_list &self(get_self(L));
		return stack::push(L, self.manager.keyboard_count());
	}

	static int empty(lua_State *L)
	{
		natkbd_kbd_list &self(get_self(L));
		return stack::push(L, !self.manager.keyboard_count());
	}

	static int next(lua_State *L) { return stack::push(L, next_pairs<false>); }
	static int pairs(lua_State *L) { return start_pairs<false>(L); }
	static int ipairs(lua_State *L) { return start_pairs<true>(L); }
};

} // namespace sol


//-------------------------------------------------
//  initialize_input - register input user types
//-------------------------------------------------

void lua_engine::initialize_input(sol::table &emu)
{

	static const enum_parser<input_seq_type, 3> s_seq_type_parser =
	{
		{ "standard", SEQ_TYPE_STANDARD },
		{ "increment", SEQ_TYPE_INCREMENT },
		{ "decrement", SEQ_TYPE_DECREMENT },
	};


	auto ioport_manager_type = sol().registry().new_usertype<ioport_manager>("ioport", sol::no_constructor);
	ioport_manager_type["count_players"] = &ioport_manager::count_players;
	ioport_manager_type["type_pressed"] = sol::overload(
			&ioport_manager::type_pressed,
			[] (ioport_manager &im, ioport_type type) { return im.type_pressed(type, 0); },
			[] (ioport_manager &im, input_type_entry const &type) { return im.type_pressed(type.type(), type.player()); });
	ioport_manager_type["type_name"] = sol::overload(
			&ioport_manager::type_name,
			[] (ioport_manager &im, ioport_type type) { return im.type_name(type, 0); });
	ioport_manager_type["type_group"] = sol::overload(
			&ioport_manager::type_group,
			[] (ioport_manager &im, ioport_type type) { return im.type_group(type, 0); });
	ioport_manager_type["type_seq"] = sol::overload(
			[] (ioport_manager &im, ioport_type type, std::optional<int> player, std::optional<char const *> seq_type_string)
			{
				if (!player)
					player = 0;
				input_seq_type seq_type = seq_type_string ? s_seq_type_parser(*seq_type_string) : SEQ_TYPE_STANDARD;
				return im.type_seq(type, *player, seq_type);
			},
			[] (ioport_manager &im, input_type_entry const &type, std::optional<char const *> seq_type_string)
			{
				input_seq_type seq_type = seq_type_string ? s_seq_type_parser(*seq_type_string) : SEQ_TYPE_STANDARD;
				return im.type_seq(type.type(), type.player(), seq_type);
			});
	ioport_manager_type["set_type_seq"] = sol::overload(
			[] (ioport_manager &im, ioport_type type, std::optional<int> player, std::optional<char const *> seq_type_string, input_seq const &seq)
			{
				if (!player)
					player = 0;
				input_seq_type seq_type = seq_type_string ? s_seq_type_parser(*seq_type_string) : SEQ_TYPE_STANDARD;
				im.set_type_seq(type, *player, seq_type, seq);
			},
			[] (ioport_manager &im, input_type_entry const &type, std::optional<char const *> seq_type_string, input_seq const &seq)
			{
				input_seq_type seq_type = seq_type_string ? s_seq_type_parser(*seq_type_string) : SEQ_TYPE_STANDARD;
				im.set_type_seq(type.type(), type.player(), seq_type, seq);
			});
	ioport_manager_type["token_to_input_type"] =
			[] (ioport_manager &im, std::string const &string)
			{
				int player;
				ioport_type const type = im.token_to_input_type(string.c_str(), player);
				return std::make_tuple(type, player);
			};
	ioport_manager_type["input_type_to_token"] = sol::overload(
				&ioport_manager::input_type_to_token,
				[] (ioport_manager &im, ioport_type type) { return im.input_type_to_token(type, 0); });
	ioport_manager_type["types"] = sol::property(&ioport_manager::types);
	ioport_manager_type["ports"] = sol::property([] (ioport_manager &im) { return tag_object_ptr_map<ioport_list>(im.ports()); });


	auto natkeyboard_type = sol().registry().new_usertype<natural_keyboard>("natkeyboard", sol::no_constructor);
	natkeyboard_type["post"] = [] (natural_keyboard &nat, std::string const &text) { nat.post_utf8(text); };
	natkeyboard_type["post_coded"] = [] (natural_keyboard &nat, std::string const &text) { nat.post_coded(text); };
	natkeyboard_type["paste"] = &natural_keyboard::paste;
	natkeyboard_type["dump"] = static_cast<std::string (natural_keyboard::*)() const>(&natural_keyboard::dump);
	natkeyboard_type["empty"] = sol::property(&natural_keyboard::empty);
	natkeyboard_type["full"] = sol::property(&natural_keyboard::full);
	natkeyboard_type["can_post"] = sol::property(&natural_keyboard::can_post);
	natkeyboard_type["is_posting"] = sol::property(&natural_keyboard::is_posting);
	natkeyboard_type["in_use"] = sol::property(&natural_keyboard::in_use, &natural_keyboard::set_in_use);
	natkeyboard_type["keyboards"] = sol::property([] (natural_keyboard &nat) { return natkbd_kbd_list(nat); });


	auto natkbddev_type = sol().registry().new_usertype<natkbd_kbd_dev>("natkeyboard_device", sol::no_constructor);
	natkbddev_type["device"] = sol::property([] (natkbd_kbd_dev const &kbd) -> device_t & { return kbd.manager.keyboard_device(kbd.index); });
	natkbddev_type["tag"] = sol::property([] (natkbd_kbd_dev const &kbd) { return kbd.manager.keyboard_device(kbd.index).tag(); });
	natkbddev_type["basetag"] = sol::property([] (natkbd_kbd_dev const &kbd) { return kbd.manager.keyboard_device(kbd.index).basetag(); });
	natkbddev_type["name"] = sol::property([] (natkbd_kbd_dev const &kbd) { return kbd.manager.keyboard_device(kbd.index).name(); });
	natkbddev_type["shortname"] = sol::property([] (natkbd_kbd_dev const &kbd) { return kbd.manager.keyboard_device(kbd.index).shortname(); });
	natkbddev_type["is_keypad"] = sol::property([] (natkbd_kbd_dev const &kbd) { return kbd.manager.keyboard_is_keypad(kbd.index); });
	natkbddev_type["enabled"] = sol::property(
			[] (natkbd_kbd_dev const &kbd) { return kbd.manager.keyboard_enabled(kbd.index); },
			[] (natkbd_kbd_dev &kbd, bool enable)
			{
				if (enable)
					kbd.manager.enable_keyboard(kbd.index);
				else
					kbd.manager.disable_keyboard(kbd.index);
			});


	auto ioport_port_type = sol().registry().new_usertype<ioport_port>("ioport_port", sol::no_constructor);
	ioport_port_type["read"] = &ioport_port::read;
	ioport_port_type["write"] = &ioport_port::write;
	ioport_port_type["field"] = &ioport_port::field;
	ioport_port_type["device"] = sol::property(&ioport_port::device);
	ioport_port_type["tag"] = sol::property(&ioport_port::tag);
	ioport_port_type["active"] = sol::property(&ioport_port::active);
	ioport_port_type["live"] = sol::property(&ioport_port::live);
	ioport_port_type["fields"] = sol::property(
			[this] (ioport_port &p)
			{
				sol::table f_table = sol().create_table();
				// parse twice for custom and default names, default has priority
				for (ioport_field &field : p.fields())
				{
					if (field.type_class() != INPUT_CLASS_INTERNAL)
						f_table[field.name()] = &field;
				}
				for (ioport_field &field : p.fields())
				{
					if (field.type_class() != INPUT_CLASS_INTERNAL)
					{
						if (field.specific_name())
							f_table[field.specific_name()] = &field;
						else
							f_table[field.manager().type_name(field.type(), field.player())] = &field;
					}
				}
				return f_table;
			});


	auto ioport_field_type = sol().registry().new_usertype<ioport_field>("ioport_field", sol::no_constructor);
	ioport_field_type["set_value"] = &ioport_field::set_value;
	ioport_field_type["clear_value"] = &ioport_field::clear_value;
	ioport_field_type["set_input_seq"] =
		[] (ioport_field &f, std::string const &seq_type_string, const input_seq &seq)
		{
			input_seq_type seq_type = s_seq_type_parser(seq_type_string);
			ioport_field::user_settings settings;
			f.get_user_settings(settings);
			settings.seq[seq_type] = seq;
			if (seq.is_default())
				settings.cfg[seq_type].clear();
			else if (!seq.length())
				settings.cfg[seq_type] = "NONE";
			else
				settings.cfg[seq_type] = f.port().device().machine().input().seq_to_tokens(seq);
			f.set_user_settings(settings);
		};
	ioport_field_type["input_seq"] =
		[] (ioport_field &f, std::string const &seq_type_string)
		{
			input_seq_type seq_type = s_seq_type_parser(seq_type_string);
			return f.seq(seq_type);
		};
	ioport_field_type["set_default_input_seq"] =
		[] (ioport_field &f, std::string const &seq_type_string, input_seq const &seq)
		{
			input_seq_type seq_type = s_seq_type_parser(seq_type_string);
			f.set_defseq(seq_type, seq);
		};
	ioport_field_type["default_input_seq"] =
		[] (ioport_field &f, const std::string &seq_type_string)
		{
			input_seq_type seq_type = s_seq_type_parser(seq_type_string);
			return f.defseq(seq_type);
		};
	ioport_field_type["keyboard_codes"] =
		[this] (ioport_field &f, int which)
		{
			sol::table result = sol().create_table();
			int index = 1;
			for (char32_t code : f.keyboard_codes(which))
				result[index++] = code;
			return result;
		};
	ioport_field_type["device"] = sol::property(&ioport_field::device);
	ioport_field_type["port"] = sol::property(&ioport_field::port);
	ioport_field_type["live"] = sol::property(&ioport_field::live);
	ioport_field_type["type"] = sol::property(&ioport_field::type);
	ioport_field_type["name"] = sol::property(&ioport_field::name);
	ioport_field_type["default_name"] = sol::property(
			[] (ioport_field const &f)
			{
				return f.specific_name() ? f.specific_name() : f.manager().type_name(f.type(), f.player());
			});
	ioport_field_type["player"] = sol::property(&ioport_field::player, &ioport_field::set_player);
	ioport_field_type["mask"] = sol::property(&ioport_field::mask);
	ioport_field_type["defvalue"] = sol::property(&ioport_field::defvalue);
	ioport_field_type["minvalue"] = sol::property(
			[] (ioport_field const &f)
			{
				return f.is_analog() ? std::make_optional(f.minval()) : std::nullopt;
			});
	ioport_field_type["maxvalue"] = sol::property(
			[] (ioport_field const &f)
			{
				return f.is_analog() ? std::make_optional(f.maxval()) : std::nullopt;
			});
	ioport_field_type["sensitivity"] = sol::property(
			[] (ioport_field const &f)
			{
				return f.is_analog() ? std::make_optional(f.sensitivity()) : std::nullopt;
			});
	ioport_field_type["way"] = sol::property(&ioport_field::way);
	ioport_field_type["type_class"] = sol::property(
			[] (ioport_field const &f)
			{
				switch (f.type_class())
				{
				case INPUT_CLASS_KEYBOARD:      return "keyboard";
				case INPUT_CLASS_CONTROLLER:    return "controller";
				case INPUT_CLASS_CONFIG:        return "config";
				case INPUT_CLASS_DIPSWITCH:     return "dipswitch";
				case INPUT_CLASS_MISC:          return "misc";
				default:                        break;
				}
				throw false;
			});
	ioport_field_type["is_analog"] = sol::property(&ioport_field::is_analog);
	ioport_field_type["is_digital_joystick"] = sol::property(&ioport_field::is_digital_joystick);
	ioport_field_type["enabled"] = sol::property(&ioport_field::enabled);
	ioport_field_type["optional"] = sol::property(&ioport_field::optional);
	ioport_field_type["cocktail"] = sol::property(&ioport_field::cocktail);
	ioport_field_type["toggle"] = sol::property(&ioport_field::toggle);
	ioport_field_type["rotated"] = sol::property(&ioport_field::rotated);
	ioport_field_type["analog_reverse"] = sol::property(&ioport_field::analog_reverse);
	ioport_field_type["analog_reset"] = sol::property(&ioport_field::analog_reset);
	ioport_field_type["analog_wraps"] = sol::property(&ioport_field::analog_wraps);
	ioport_field_type["analog_invert"] = sol::property(&ioport_field::analog_invert);
	ioport_field_type["impulse"] = sol::property(&ioport_field::impulse);
	ioport_field_type["crosshair_scale"] = sol::property(&ioport_field::crosshair_scale, &ioport_field::set_crosshair_scale);
	ioport_field_type["crosshair_offset"] = sol::property(&ioport_field::crosshair_offset, &ioport_field::set_crosshair_offset);
	ioport_field_type["user_value"] = sol::property(
			[] (ioport_field const &f)
			{
				ioport_field::user_settings settings;
				f.get_user_settings(settings);
				return settings.value;
			},
			[] (ioport_field &f, ioport_value val)
			{
				ioport_field::user_settings settings;
				f.get_user_settings(settings);
				settings.value = val;
				f.set_user_settings(settings);
			});
	ioport_field_type["settings"] = sol::property(
			[this] (ioport_field &f)
			{
				sol::table result = sol().create_table();
				for (ioport_setting const &setting : f.settings())
					if (setting.enabled())
						result[setting.value()] = setting.name();
				return result;
			});


	auto ioport_field_live_type = sol().registry().new_usertype<ioport_field_live>("ioport_field_live", sol::no_constructor);
	ioport_field_live_type["name"] = &ioport_field_live::name;


	auto input_type_entry_type = sol().registry().new_usertype<input_type_entry>("input_type_entry", sol::no_constructor);
	input_type_entry_type["type"] = sol::property(&input_type_entry::type);
	input_type_entry_type["group"] = sol::property(&input_type_entry::group);
	input_type_entry_type["player"] = sol::property(&input_type_entry::player);
	input_type_entry_type["token"] = sol::property(&input_type_entry::token);
	input_type_entry_type["name"] = sol::property(&input_type_entry::name);
	input_type_entry_type["is_analog"] = sol::property([] (input_type_entry const &type) { return ioport_manager::type_is_analog(type.type()); });


	auto input_type = sol().registry().new_usertype<input_manager>("input", sol::no_constructor);
	input_type["code_value"] = &input_manager::code_value;
	input_type["code_pressed"] = &input_manager::code_pressed;
	input_type["code_pressed_once"] = &input_manager::code_pressed_once;
	input_type["code_name"] = &input_manager::code_name;
	input_type["code_to_token"] = &input_manager::code_to_token;
	input_type["code_from_token"] = &input_manager::code_from_token;
	input_type["seq_pressed"] = &input_manager::seq_pressed;
	input_type["seq_clean"] = &input_manager::seq_clean;
	input_type["seq_name"] = &input_manager::seq_name;
	input_type["seq_to_tokens"] = &input_manager::seq_to_tokens;
	input_type["seq_from_tokens"] =
		[] (input_manager &input, std::string_view tokens)
		{
			input_seq seq;
			input.seq_from_tokens(seq, tokens);
			return seq;
		};
	input_type["axis_code_poller"] = [] (input_manager &input) { return std::unique_ptr<input_code_poller>(new axis_code_poller(input)); };
	input_type["switch_code_poller"] = [] (input_manager &input) { return std::unique_ptr<input_code_poller>(new switch_code_poller(input)); };
	input_type["keyboard_code_poller"] = [] (input_manager &input) { return std::unique_ptr<input_code_poller>(new keyboard_code_poller(input)); };
	input_type["axis_sequence_poller"] = [] (input_manager &input) { return std::unique_ptr<input_sequence_poller>(new axis_sequence_poller(input)); };
	input_type["switch_sequence_poller"] = [] (input_manager &input) { return std::unique_ptr<input_sequence_poller>(new switch_sequence_poller(input)); };
	input_type["device_classes"] = sol::property(
			[this] (input_manager &input)
			{
				sol::table result = sol().create_table();
				for (input_device_class devclass_id = DEVICE_CLASS_FIRST_VALID; devclass_id <= DEVICE_CLASS_LAST_VALID; devclass_id++)
				{
					input_class &devclass = input.device_class(devclass_id);
					result[devclass.name()] = &devclass;
				}
				return result;
			});


	auto codepoll_type = sol().registry().new_usertype<input_code_poller>("input_code_poller", sol::no_constructor);
	codepoll_type["reset"] = &input_code_poller::reset;
	codepoll_type["poll"] = &input_code_poller::poll;


	auto seqpoll_type = sol().registry().new_usertype<input_sequence_poller>("input_seq_poller", sol::no_constructor);
	seqpoll_type["start"] = sol::overload(
			[] (input_sequence_poller &poller) { return poller.start(); },
			[] (input_sequence_poller &poller, input_seq const &seq) { return poller.start(seq); });
	seqpoll_type["poll"] = &input_sequence_poller::poll;
	seqpoll_type["sequence"] = sol::property(&input_sequence_poller::sequence);
	seqpoll_type["valid"] = sol::property(&input_sequence_poller::valid);
	seqpoll_type["modified"] = sol::property(&input_sequence_poller::modified);


	auto iptseq_type = emu.new_usertype<input_seq>(
			"input_seq",
			sol::call_constructor, sol::constructors<input_seq(), input_seq(input_seq const &)>());
	iptseq_type["reset"] = &input_seq::reset;
	iptseq_type["set_default"] = &input_seq::set_default;
	iptseq_type["empty"] = sol::property(&input_seq::empty);
	iptseq_type["length"] = sol::property(&input_seq::length);
	iptseq_type["is_valid"] = sol::property(&input_seq::is_valid);
	iptseq_type["is_default"] = sol::property(&input_seq::is_default);


	auto input_class_type = sol().registry().new_usertype<input_class>("input_class", sol::no_constructor);
	input_class_type["name"] = sol::property(&input_class::name);
	input_class_type["enabled"] = sol::property(&input_class::enabled);
	input_class_type["multi"] = sol::property(&input_class::multi);
	input_class_type["devices"] = sol::property(
			[this] (input_class &devclass)
			{
				sol::table result = sol().create_table();
				int index = 1;
				for (int devindex = 0; devindex <= devclass.maxindex(); devindex++)
				{
					input_device *const dev = devclass.device(devindex);
					if (dev)
						result[index++] = dev;
				}
				return result;
			});


	auto input_device_type = sol().registry().new_usertype<input_device>("input_device", sol::no_constructor);
	input_device_type["name"] = sol::property(&input_device::name);
	input_device_type["id"] = sol::property(&input_device::id);
	input_device_type["devindex"] = sol::property(&input_device::devindex);
	input_device_type["items"] = sol::property(
			[this] (input_device &dev)
			{
				sol::table result = sol().create_table();
				for (input_item_id id = ITEM_ID_FIRST_VALID; id < dev.maxitem(); id++)
				{
					input_device_item *item = dev.item(id);
					if (item)
						result[id] = dev.item(id);
				}
				return result;
			});


	auto input_device_item_type = sol().registry().new_usertype<input_device_item>("input_device_item", sol::no_constructor);
	input_device_item_type["name"] = sol::property(&input_device_item::name);
	input_device_item_type["code"] = sol::property(&input_device_item::code);
	input_device_item_type["token"] = sol::property(&input_device_item::token);
	input_device_item_type["current"] = sol::property(&input_device_item::current);


	auto uiinput_type = sol().registry().new_usertype<ui_input_manager>("uiinput", sol::no_constructor);
	uiinput_type["find_mouse"] =
		[] (ui_input_manager &ui)
		{
			int32_t x, y;
			bool button;
			render_target *rt = ui.find_mouse(&x, &y, &button);
			return std::make_tuple(x, y, button, rt);
		};
	uiinput_type["pressed"] = &ui_input_manager::pressed;
	uiinput_type["pressed_repeat"] = &ui_input_manager::pressed_repeat;
	uiinput_type["presses_enabled"] = sol::property(&ui_input_manager::presses_enabled, &ui_input_manager::set_presses_enabled);

}
