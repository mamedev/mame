// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    rendlay.h

    Core rendering layout parser and manager.

***************************************************************************/

#ifndef MAME_EMU_RENDLAY_H
#define MAME_EMU_RENDLAY_H

#pragma once

#include "rendertypes.h"
#include "screen.h"

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

namespace emu::render::detail {

struct bounds_step
{
	render_bounds get() const { return bounds; }

	int             state;
	render_bounds   bounds;
	render_bounds   delta;
};
using bounds_vector = std::vector<bounds_step>;

struct color_step
{
	render_color get() const { return color; }

	int             state;
	render_color    color;
	render_color    delta;
};
using color_vector = std::vector<color_step>;


class layout_environment;
class view_environment;

} // namespace emu::render::detail


/// \brief A description of a piece of visible artwork
///
/// Most view items (except for those referencing screens) have exactly
/// one layout_element which describes the contents of the item.
/// Elements are separate from items because they can be re-used
/// multiple times within a layout.  Even though an element can contain
/// a number of components, they are drawn as a single textured quad.
class layout_element
{
public:
	using environment = emu::render::detail::layout_environment;

	// construction/destruction
	layout_element(environment &env, util::xml::data_node const &elemnode);
	virtual ~layout_element();

	// getters
	running_machine &machine() const { return m_machine; }
	int default_state() const { return m_defstate; }
	render_texture *state_texture(int state);

	// operations
	void preload();

private:
	/// \brief A drawing component within a layout element
	///
	/// Each #layout_element contains one or more components. Each
	/// component can describe either an image or a rectangle/disk
	/// primitive.  A component can also have a state mask and value
	/// for controlling visibility.  If the state of the item
	/// instantiating the element matches the component's state value
	/// for the bits that are set in the mask, the component is visible.
	class component
	{
	public:
		typedef std::unique_ptr<component> ptr;

		// construction/destruction
		component(environment &env, util::xml::data_node const &compnode);
		virtual ~component() = default;

		// setup
		void normalize_bounds(float xoffs, float yoffs, float xscale, float yscale);

		// getters
		int statemask() const { return m_statemask; }
		int stateval() const { return m_stateval; }
		std::pair<int, bool> statewrap() const;
		render_bounds overall_bounds() const;
		render_bounds bounds(int state) const;
		render_color color(int state) const;

		// operations
		virtual void preload(running_machine &machine);
		virtual void draw(running_machine &machine, bitmap_argb32 &dest, int state);

	protected:
		// helpers
		virtual int maxstate() const;
		virtual void draw_aligned(running_machine &machine, bitmap_argb32 &dest, const rectangle &bounds, int state);

		// drawing helpers
		static void draw_text(render_font &font, bitmap_argb32 &dest, const rectangle &bounds, std::string_view str, int align, const render_color &color);
		static void draw_segment_horizontal_caps(bitmap_argb32 &dest, int minx, int maxx, int midy, int width, int caps, rgb_t color);
		static void draw_segment_horizontal(bitmap_argb32 &dest, int minx, int maxx, int midy, int width, rgb_t color);
		static void draw_segment_vertical_caps(bitmap_argb32 &dest, int miny, int maxy, int midx, int width, int caps, rgb_t color);
		static void draw_segment_vertical(bitmap_argb32 &dest, int miny, int maxy, int midx, int width, rgb_t color);
		static void draw_segment_diagonal_1(bitmap_argb32 &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color);
		static void draw_segment_diagonal_2(bitmap_argb32 &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color);
		static void draw_segment_decimal(bitmap_argb32 &dest, int midx, int midy, int width, rgb_t color);
		static void draw_segment_comma(bitmap_argb32 &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color);
		static void apply_skew(bitmap_argb32 &dest, int skewwidth);

	private:
		using bounds_vector = emu::render::detail::bounds_vector;
		using color_vector = emu::render::detail::color_vector;

		// internal state
		int const           m_statemask;                // bits of state used to control visibility
		int const           m_stateval;                 // masked state value to make component visible
		bounds_vector       m_bounds;                   // bounds of the element
		color_vector        m_color;                    // color of the element
	};

	// component implementations
	class image_component;
	class rect_component;
	class disk_component;
	class text_component;
	class led7seg_component;
	class led14seg_component;
	class led16seg_component;
	class led14segsc_component;
	class led16segsc_component;
	class simplecounter_component;
	class reel_component;

	// a texture encapsulates a texture for a given element in a given state
	class texture
	{
	public:
		texture();
		texture(texture const &that) = delete;
		texture(texture &&that);

		~texture();

		texture &operator=(texture const &that) = delete;
		texture &operator=(texture &&that);

		layout_element *    m_element;      // pointer back to the element
		render_texture *    m_texture;      // texture for this state
		int                 m_state;        // associated state number
	};

	typedef component::ptr (*make_component_func)(environment &env, util::xml::data_node const &compnode);
	typedef std::map<std::string, make_component_func> make_component_map;

	// internal helpers
	static void element_scale(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);
	template <typename T> static component::ptr make_component(environment &env, util::xml::data_node const &compnode);

	static make_component_map const s_make_component; // maps component XML names to creator functions

	// internal state
	running_machine &           m_machine;      // reference to the owning machine
	std::vector<component::ptr> m_complist;     // list of components
	int const                   m_defstate;     // default state of this element
	int                         m_statemask;    // mask to apply to state values
	bool                        m_foldhigh;     // whether we need to fold state values above the mask range
	std::vector<texture>        m_elemtex;      // array of element textures used for managing the scaled bitmaps
};


/// \brief A reusable group of items
///
/// Views expand/flatten groups into their component elements applying
/// an optional coordinate transform.  This is useful for duplicating
/// the same sublayout in multiple views, or grouping related items to
/// simplify overall view arrangement.  Groups only exist while parsing
/// a layout file - no information about element grouping is preserved
/// after the views have been built.
class layout_group
{
public:
	using environment = emu::render::detail::layout_environment;
	using group_map = std::unordered_map<std::string, layout_group>;
	using transform = std::array<std::array<float, 3>, 3>;

	layout_group(util::xml::data_node const &groupnode);
	~layout_group();

	util::xml::data_node const &get_groupnode() const { return m_groupnode; }

	transform make_transform(int orientation, render_bounds const &dest) const;
	transform make_transform(int orientation, transform const &trans) const;
	transform make_transform(int orientation, render_bounds const &dest, transform const &trans) const;

	void set_bounds_unresolved();
	void resolve_bounds(environment &env, group_map &groupmap);

private:
	void resolve_bounds(environment &env, group_map &groupmap, std::vector<layout_group const *> &seen);
	void resolve_bounds(
			environment &env,
			util::xml::data_node const &parentnode,
			group_map &groupmap,
			std::vector<layout_group const *> &seen,
			bool &empty,
			bool vistoggle,
			bool repeat,
			bool init);

	util::xml::data_node const &    m_groupnode;
	render_bounds                   m_bounds;
	bool                            m_bounds_resolved;
};


/// \brief A single item in a view
///
/// Each view has a list of item structures describing the visual
/// elements to draw, where they are located, additional blending modes,
/// and bindings for inputs and outputs.
class layout_view_item
{
	friend class layout_view;

public:
	using view_environment = emu::render::detail::view_environment;
	using element_map = std::unordered_map<std::string, layout_element>;
	using state_delegate = delegate<int ()>;
	using bounds_delegate = delegate<render_bounds ()>;
	using color_delegate = delegate<render_color ()>;
	using scroll_size_delegate = delegate<float ()>;
	using scroll_pos_delegate = delegate<float ()>;

	// construction/destruction
	layout_view_item(
			view_environment &env,
			util::xml::data_node const &itemnode,
			element_map &elemmap,
			int orientation,
			layout_group::transform const &trans,
			render_color const &color);
	~layout_view_item();

	// getters
	std::string const &id() const { return m_id; }
	layout_element *element() const { return m_element; }
	screen_device *screen() const { return m_screen; }
	bool bounds_animated() const { return m_bounds.size() > 1U; }
	bool color_animated() const { return m_color.size() > 1U; }
	render_bounds bounds() const { return m_get_bounds(); }
	render_color color() const { return m_get_color(); }
	bool scroll_wrap_x() const { return m_scrollwrapx; }
	bool scroll_wrap_y() const { return m_scrollwrapy; }
	float scroll_size_x() const { return m_get_scroll_size_x(); }
	float scroll_size_y() const { return m_get_scroll_size_y(); }
	float scroll_pos_x() const { return m_get_scroll_pos_x(); }
	float scroll_pos_y() const { return m_get_scroll_pos_y(); }
	int blend_mode() const { return m_blend_mode; }
	u32 visibility_mask() const { return m_visibility_mask; }
	int orientation() const { return m_orientation; }
	render_container *screen_container() const { return m_screen ? &m_screen->container() : nullptr; }

	// interactivity
	bool has_input() const { return bool(m_input_port); }
	std::pair<ioport_port *, ioport_value> input_tag_and_mask() const { return std::make_pair(m_input_port, m_input_mask); }
	bool clickthrough() const { return m_clickthrough; }

	// fetch state based on configured source
	int element_state() const { return m_get_elem_state(); }
	int animation_state() const { return m_get_anim_state(); }

	// set state
	void set_state(int state) { m_elem_state = state; }
	void set_scroll_size_x(float size) { m_scrollsizex = std::clamp(size, 0.01f, 1.0f); }
	void set_scroll_size_y(float size) { m_scrollsizey = std::clamp(size, 0.01f, 1.0f); }
	void set_scroll_pos_x(float pos) { m_scrollposx = pos; }
	void set_scroll_pos_y(float pos) { m_scrollposy = pos; }

	// set handlers
	void set_element_state_callback(state_delegate &&handler);
	void set_animation_state_callback(state_delegate &&handler);
	void set_bounds_callback(bounds_delegate &&handler);
	void set_color_callback(color_delegate &&handler);
	void set_scroll_size_x_callback(scroll_size_delegate &&handler);
	void set_scroll_size_y_callback(scroll_size_delegate &&handler);
	void set_scroll_pos_x_callback(scroll_pos_delegate &&handler);
	void set_scroll_pos_y_callback(scroll_pos_delegate &&handler);

	// resolve tags, if any
	void resolve_tags();

private:
	using bounds_vector = emu::render::detail::bounds_vector;
	using color_vector = emu::render::detail::color_vector;

	state_delegate default_get_elem_state();
	state_delegate default_get_anim_state();
	bounds_delegate default_get_bounds();
	color_delegate default_get_color();
	scroll_size_delegate default_get_scroll_size_x();
	scroll_size_delegate default_get_scroll_size_y();
	scroll_pos_delegate default_get_scroll_pos_x();
	scroll_pos_delegate default_get_scroll_pos_y();
	int get_state() const;
	int get_output() const;
	int get_input_raw() const;
	int get_input_field_cached() const;
	int get_input_field_conditional() const;
	int get_anim_output() const;
	int get_anim_input() const;
	float get_scrollsizex() const;
	float get_scrollsizey() const;
	float get_scrollposx() const;
	float get_scrollposy() const;
	template <bool Wrap> float get_scrollx_output() const;
	template <bool Wrap> float get_scrolly_output() const;
	template <bool Wrap> float get_scrollx_input() const;
	template <bool Wrap> float get_scrolly_input() const;
	render_bounds get_interpolated_bounds() const;
	render_color get_interpolated_color() const;

	static layout_element *find_element(view_environment &env, util::xml::data_node const &itemnode, element_map &elemmap);
	static bounds_vector make_bounds(view_environment &env, util::xml::data_node const &itemnode, layout_group::transform const &trans);
	static color_vector make_color(view_environment &env, util::xml::data_node const &itemnode, render_color const &mult);

	// internal state
	layout_element *const   m_element;              // pointer to the associated element (non-screens only)
	state_delegate          m_get_elem_state;       // resolved element state function
	state_delegate          m_get_anim_state;       // resolved animation state function
	bounds_delegate         m_get_bounds;           // resolved bounds function
	color_delegate          m_get_color;            // resolved color function
	scroll_size_delegate    m_get_scroll_size_x;    // resolved horizontal scroll window size function
	scroll_size_delegate    m_get_scroll_size_y;    // resolved vertical scroll window size function
	scroll_pos_delegate     m_get_scroll_pos_x;     // resolved horizontal scroll position function
	scroll_pos_delegate     m_get_scroll_pos_y;     // resolved vertical scroll position function
	output_finder<>         m_output;               // associated output
	output_finder<>         m_animoutput;           // associated output for animation if different
	output_finder<>         m_scrollxoutput;        // associated output for horizontal scroll position
	output_finder<>         m_scrollyoutput;        // associated output for vertical scroll position
	ioport_port *           m_animinput_port;       // input port used for animation
	ioport_port *           m_scrollxinput_port;    // input port used for horizontal scrolling
	ioport_port *           m_scrollyinput_port;    // input port used for vertical scrolling
	bool const              m_scrollwrapx;          // whether horizontal scrolling works like a loop
	bool const              m_scrollwrapy;          // whether vertical scrolling works like a loop
	int                     m_elem_state;           // element state used in absence of bindings
	float                   m_scrollsizex;          // horizontal scroll window size used in absence of bindings
	float                   m_scrollsizey;          // vertical scroll window size used in absence of bindings
	float                   m_scrollposx;           // horizontal scroll position used in absence of bindings
	float                   m_scrollposy;           // vertical scroll position used in absence of bindings
	ioport_value const      m_animmask;             // mask for animation state
	ioport_value const      m_scrollxmask;          // mask for horizontal scroll position
	ioport_value const      m_scrollymask;          // mask for vertical scroll position
	ioport_value const      m_scrollxmin;           // minimum value for horizontal scroll position
	ioport_value const      m_scrollymin;           // minimum value for vertical scroll position
	ioport_value const      m_scrollxmax;           // maximum value for horizontal scroll position
	ioport_value const      m_scrollymax;           // maximum value for vertical scroll position
	u8 const                m_animshift;            // shift for animation state
	u8 const                m_scrollxshift;         // shift for horizontal scroll position
	u8 const                m_scrollyshift;         // shift for vertical scroll position
	ioport_port *           m_input_port;           // input port of this item
	ioport_field const *    m_input_field;          // input port field of this item
	ioport_value const      m_input_mask;           // input mask of this item
	u8 const                m_input_shift;          // input mask rightshift for raw (trailing 0s)
	bool                    m_clickthrough;         // should click pass through to lower elements
	screen_device *         m_screen;               // pointer to screen
	int const               m_orientation;          // orientation of this item
	bounds_vector           m_bounds;               // bounds of the item
	color_vector const      m_color;                // color of the item
	int                     m_blend_mode;           // blending mode to use when drawing
	u32                     m_visibility_mask;      // combined mask of parent visibility groups

	// cold items
	std::string const       m_id;                   // optional unique item identifier
	std::string const       m_input_tag;            // input tag of this item
	std::string const       m_animinput_tag;        // tag of input port for animation state
	std::string const       m_scrollxinput_tag;     // tag of input port for horizontal scroll position
	std::string const       m_scrollyinput_tag;     // tag of input port for vertical scroll position
	bounds_vector const     m_rawbounds;            // raw (original) bounds of the item
	bool const              m_have_output;          // whether we actually have an output
	bool const              m_input_raw;            // get raw data from input port
	bool const              m_have_animoutput;      // whether we actually have an output for animation
	bool const              m_have_scrollxoutput;   // whether we actually have an output for horizontal scroll
	bool const              m_have_scrollyoutput;   // whether we actually have an output for vertical scroll
	bool const              m_has_clickthrough;     // whether clickthrough was explicitly configured
};


/// \brief A single view within a #layout_file
///
/// The view is described using arbitrary coordinates that are scaled to
/// fit within the render target.  Pixels within a view are assumed to
/// be square.
class layout_view
{
public:
	using layout_environment = emu::render::detail::layout_environment;
	using view_environment = emu::render::detail::view_environment;
	using element_map = layout_view_item::element_map;
	using group_map = std::unordered_map<std::string, layout_group>;
	using screen_ref_vector = std::vector<std::reference_wrapper<screen_device const>>;
	using prepare_items_delegate = delegate<void ()>;
	using preload_delegate = delegate<void ()>;
	using recomputed_delegate = delegate<void ()>;

	using item = layout_view_item;
	using item_list = std::list<item>;
	using item_ref_vector = std::vector<std::reference_wrapper<item> >;

	/// \brief A subset of items in a view that can be hidden or shown
	///
	/// Visibility toggles allow the user to show or hide selected parts
	/// of a view.
	class visibility_toggle
	{
	public:
		// construction/destruction/assignment
		visibility_toggle(std::string &&name, u32 mask);
		visibility_toggle(visibility_toggle const &) = default;
		visibility_toggle(visibility_toggle &&) = default;
		visibility_toggle &operator=(visibility_toggle const &) = default;
		visibility_toggle &operator=(visibility_toggle &&) = default;

		// getters
		std::string const &name() const { return m_name; }
		u32 mask() const { return m_mask; }

	private:
		std::string             m_name;             // display name for the toggle
		u32                     m_mask;             // toggle combination to show
	};
	using visibility_toggle_vector = std::vector<visibility_toggle>;

	/// \brief An edge of an item in a view
	class edge
	{
	public:
		// construction/destruction
		constexpr edge(unsigned index, float position, bool trailing)
			: m_index(index)
			, m_position(position)
			, m_trailing(trailing)
		{
		}

		// getters
		constexpr unsigned index() const { return m_index; }
		constexpr float position() const { return m_position; }
		constexpr bool trailing() const { return m_trailing; }

		// comparison
		constexpr bool operator<(edge const &that) const
		{
			return std::make_tuple(m_position, m_trailing, m_index) < std::make_tuple(that.m_position, that.m_trailing, that.m_index);
		}

	private:
		unsigned                m_index;            // index of item in some collection
		float                   m_position;         // position of edge on given axis
		bool                    m_trailing;         // false for edge at lower position on axis
	};
	using edge_vector = std::vector<edge>;

	// construction/destruction
	layout_view(
			layout_environment &env,
			util::xml::data_node const &viewnode,
			element_map &elemmap,
			group_map &groupmap);
	~layout_view();

	// getters
	item *get_item(std::string const &id);
	item_list &items() { return m_items; }
	bool has_screen(screen_device const &screen) const;
	const std::string &name() const { return m_name; }
	const std::string &unqualified_name() const { return m_unqualified_name; }
	size_t visible_screen_count() const { return m_screens.size(); }
	float effective_aspect() const { return m_effaspect; }
	const render_bounds &bounds() const { return m_bounds; }
	bool has_visible_screen(screen_device const &screen) const;
	const item_ref_vector &visible_items() const { return m_visible_items; }
	const item_ref_vector &visible_screen_items() const { return m_screen_items; }
	const item_ref_vector &interactive_items() const { return m_interactive_items; }
	const edge_vector &interactive_edges_x() const { return m_interactive_edges_x; }
	const edge_vector &interactive_edges_y() const { return m_interactive_edges_y; }
	const screen_ref_vector &visible_screens() const { return m_screens; }
	const visibility_toggle_vector &visibility_toggles() const { return m_vistoggles; }
	u32 default_visibility_mask() const { return m_defvismask; }
	bool has_art() const { return m_has_art; }

	// set handlers
	void set_prepare_items_callback(prepare_items_delegate &&handler);
	void set_preload_callback(preload_delegate &&handler);
	void set_recomputed_callback(recomputed_delegate &&handler);

	// operations
	void prepare_items() { if (!m_prepare_items.isnull()) m_prepare_items(); }
	void recompute(u32 visibility_mask, bool zoom_to_screens);
	void preload();

	// resolve tags, if any
	void resolve_tags();

private:
	struct layer_lists;

	using item_id_map = std::unordered_map<
			std::reference_wrapper<std::string const>,
			item &,
			std::hash<std::string>,
			std::equal_to<std::string> >;

	// add items, recursing for groups
	void add_items(
			layer_lists &layers,
			view_environment &env,
			util::xml::data_node const &parentnode,
			element_map &elemmap,
			group_map &groupmap,
			int orientation,
			layout_group::transform const &trans,
			render_color const &color,
			bool root,
			bool repeat,
			bool init);

	static std::string make_name(layout_environment &env, util::xml::data_node const &viewnode);

	// internal state
	float                       m_effaspect;        // X/Y of the layout in current configuration
	render_bounds               m_bounds;           // computed bounds of the view in current configuration
	item_list                   m_items;            // list of layout items
	item_ref_vector             m_visible_items;    // all visible items
	item_ref_vector             m_screen_items;     // visible items that represent screens to draw
	item_ref_vector             m_interactive_items;// visible items that can accept pointer input
	edge_vector                 m_interactive_edges_x;
	edge_vector                 m_interactive_edges_y;
	screen_ref_vector           m_screens;          // list screens visible in current configuration

	// handlers
	prepare_items_delegate      m_prepare_items;    // prepare items for adding to render container
	preload_delegate            m_preload;          // additional actions when visible items change
	recomputed_delegate         m_recomputed;       // additional actions on resizing/visibility change

	// cold items
	std::string                 m_name;             // display name for the view
	std::string                 m_unqualified_name; // the name exactly as specified in the layout file
	item_id_map                 m_items_by_id;      // items with non-empty ID indexed by ID
	visibility_toggle_vector    m_vistoggles;       // collections of items that can be shown/hidden
	render_bounds               m_expbounds;        // explicit bounds of the view
	u32                         m_defvismask;       // default visibility mask
	bool                        m_has_art;          // true if the layout contains non-screen elements
};


/// \brief Layout description file
///
/// Comprises a list of elements and a list of views.  The elements are
/// reusable items that the views reference.
class layout_file
{
public:
	using element_map = std::unordered_map<std::string, layout_element>;
	using group_map = std::unordered_map<std::string, layout_group>;
	using view_list = std::list<layout_view>;
	using resolve_tags_delegate = delegate<void ()>;

	// construction/destruction
	layout_file(device_t &device, util::xml::data_node const &rootnode, char const *searchpath, char const *dirname);
	~layout_file();

	// getters
	device_t &device() const { return m_device; }
	element_map const &elements() const { return m_elemmap; }
	view_list &views() { return m_viewlist; }
	view_list const &views() const { return m_viewlist; }

	// resolve tags, if any
	void resolve_tags();

	// set handlers
	void set_resolve_tags_callback(resolve_tags_delegate &&handler);

private:
	using environment = emu::render::detail::layout_environment;

	// add elements and parameters
	void add_elements(
			environment &env,
			util::xml::data_node const &parentnode,
			group_map &groupmap,
			bool repeat,
			bool init);

	// internal state
	device_t &              m_device;       // device that caused file to be loaded
	element_map             m_elemmap;      // list of shared layout elements
	view_list               m_viewlist;     // list of views
	resolve_tags_delegate   m_resolve_tags; // additional actions after resolving tags
};

#endif // MAME_EMU_RENDLAY_H
