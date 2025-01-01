// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// ImGui based debugger

#include "emu.h"
#include "debug_module.h"

#include "imgui/imgui.h"

#include "imagedev/floppy.h"

#include "debug/debugvw.h"
#include "debug/dvdisasm.h"
#include "debug/dvmemory.h"
#include "debug/dvbpoints.h"
#include "debug/dvwpoints.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debugger.h"
#include "render.h"
#include "ui/uimain.h"
#include "uiinput.h"

#include "formats/flopimg.h"

#include "config.h"
#include "modules/lib/osdobj_common.h"
#include "modules/osdmodule.h"
#include "zippath.h"

namespace osd {

namespace {

class debug_area
{
	DISABLE_COPYING(debug_area);

public:
	debug_area(running_machine &machine, debug_view_type type) :
		next(nullptr),
		type(0),
		ofs_x(0),
		ofs_y(0),
		is_collapsed(false),
		exec_cmd(false),
		scroll_end(false),
		scroll_follow(false)
	{
		this->view = machine.debug_view().alloc_view(type, nullptr, this);
		this->type = type;
		this->m_machine = &machine;
		this->width = 300;
		this->height = 300;
		this->console_prev.clear();

		/* specials */
		switch (type)
		{
		case DVT_DISASSEMBLY:
			/* set up disasm view */
			downcast<debug_view_disasm *>(this->view)->set_expression("curpc");
			break;
		default:
			break;
		}
	}
	~debug_area()
	{
		//this->target->debug_free(*this->container);
		machine().debug_view().free_view(*this->view);
	}

	running_machine &machine() const { assert(m_machine != nullptr); return *m_machine; }

	debug_area *        next;

	int                 type;
	debug_view *        view;
	running_machine *   m_machine;
	// drawing
	int                 ofs_x;
	int                 ofs_y;
	int                 width;
	int                 height;  // initial view size
	std::string         title;
	float               view_width;
	float               view_height;
	bool                has_focus;
	bool                is_collapsed;
	bool                exec_cmd;  // console only
	int                 src_sel;
	bool                scroll_end;
	bool                scroll_follow;  // set if view is to stay at the end of a scrollable area (like a log window)
	char                console_input[512];
	std::vector<std::string> console_history;
	std::string         console_prev;
};

class debug_imgui : public osd_module, public debug_module
{
public:
	debug_imgui() :
		osd_module(OSD_DEBUG_PROVIDER, "imgui"), debug_module(),
		m_machine(nullptr),
		m_take_ui(false),
		m_current_pointer(-1),
		m_mouse_x(0),
		m_mouse_y(0),
		m_mouse_button(false),
		m_prev_mouse_button(false),
		m_running(false),
		font_name(nullptr),
		font_size(0),
		m_key_char(0),
		m_hide(false),
		m_win_count(0),
		m_has_images(false),
		m_initialised(false),
		m_dialog_image(nullptr),
		m_filelist_refresh(false),
		m_mount_open(false),
		m_create_open(false),
		m_create_confirm_wait(false),
		m_selected_file(nullptr),
		m_format_sel(0)
	{
	}

	virtual ~debug_imgui() { }

	virtual int init(osd_interface &osd, const osd_options &options) override { return 0; }
	virtual void exit() override {};

	virtual void init_debugger(running_machine &machine) override;
	virtual void wait_for_debugger(device_t &device, bool firststop) override;
	virtual void debugger_update() override;

private:
	enum file_entry_type
	{
		DRIVE,
		DIRECTORY,
		FILE
	};

	struct file_entry
	{
		file_entry_type type;
		std::string basename;
		std::string fullpath;
	};

	struct image_type_entry
	{
		const floppy_image_format_t* format;
		std::string shortname;
		std::string longname;
	};

	void handle_events();
	void handle_mouse_views();
	void handle_keys_views();
	void handle_console(running_machine* machine);
	void update();
	void draw_images_menu();
	void draw_console();
	void add_disasm(int id);
	void add_memory(int id);
	void add_bpoints(int id);
	void add_wpoints(int id);
	void add_log(int id);
	void draw_disasm(debug_area* view_ptr, bool* opened);
	void draw_memory(debug_area* view_ptr, bool* opened);
	void draw_bpoints(debug_area* view_ptr, bool* opened);
	void draw_log(debug_area* view_ptr, bool* opened);
	void draw_view(debug_area* view_ptr, bool exp_change);
	void draw_mount_dialog(const char* label);
	void draw_create_dialog(const char* label);
	void mount_image();
	void create_image();
	void refresh_filelist();
	void refresh_typelist();
	void update_cpu_view(device_t* device);
	static const char* get_view_source(void* user_data, int idx);
	static int history_set(ImGuiInputTextCallbackData* data);

	running_machine* m_machine;
	bool             m_take_ui;
	int32_t          m_current_pointer;
	int32_t          m_mouse_x;
	int32_t          m_mouse_y;
	bool             m_mouse_button;
	bool             m_prev_mouse_button;
	bool             m_running;
	const char*      font_name;
	float            font_size;
	ImVec2           m_text_size;  // size of character (assumes monospaced font is in use)
	uint8_t          m_key_char;
	bool             m_hide;
	int              m_win_count;  // number of active windows, does not decrease, used to ID individual windows
	bool             m_has_images; // true if current system has any image devices
	bool             m_initialised;  // true after initial views are created
	device_image_interface* m_dialog_image;
	bool             m_filelist_refresh;  // set to true to refresh mount/create dialog file lists
	bool             m_mount_open;  // true when opening a mount dialog
	bool             m_create_open;  // true when opening a create dialog
	bool             m_create_confirm_wait;  // true if waiting for confirmation of the above
	std::vector<file_entry> m_filelist;
	std::vector<image_type_entry> m_typelist;
	file_entry*      m_selected_file;
	int              m_format_sel;
	char             m_path[1024];  // path text field buffer
	std::unordered_map<input_item_id,ImGuiKey> m_mapping;
};

// globals
static std::vector<debug_area*> view_list;
static debug_area* view_main_console = nullptr;
static debug_area* view_main_disasm = nullptr;
static debug_area* view_main_regs = nullptr;
static int history_pos;

static void view_list_add(debug_area* item)
{
	view_list.push_back(item);
}

static void view_list_remove(debug_area* item)
{
	std::vector<debug_area*>::iterator it;
	if(view_list.empty())
		return;
	it = std::find(view_list.begin(),view_list.end(),item);
	view_list.erase(it);

}

static debug_area *dview_alloc(running_machine &machine, debug_view_type type)
{
	return new debug_area(machine, type);
}

static inline void map_attr_to_fg_bg(unsigned char attr, rgb_t *fg, rgb_t *bg)
{
	*bg = rgb_t(0xe6,0xff,0xff,0xff);
	*fg = rgb_t(0xff,0x00,0x00,0x00);

	if(attr & DCA_ANCILLARY)
		*bg = rgb_t(0xcc,0xd0,0xd0,0xd0);
	if(attr & DCA_SELECTED) {
		*bg = rgb_t(0xcc,0xff,0x80,0x80);
	}
	if(attr & DCA_CURRENT) {
		*bg = rgb_t(0xcc,0xff,0xff,0x00);
	}
	if(attr & DCA_CHANGED) {
		*fg = rgb_t(0xff,0xff,0x00,0x00);
	}
	if(attr & DCA_INVALID) {
		*fg = rgb_t(0xff,0x00,0x00,0xff);
	}
	if(attr & DCA_DISABLED) {
		*fg = rgb_t(fg->a(), (fg->r() + bg->r()) >> 1, (fg->g() + bg->g()) >> 1, (fg->b() + bg->b()) >> 1);
	}
	if(attr & DCA_COMMENT) {
		*fg = rgb_t(0xff,0x00,0x80,0x00);
	}
}

const char* debug_imgui::get_view_source(void* user_data, int idx)
{
	auto* vw = static_cast<debug_view*>(user_data);
	return vw->source(idx)->name();
}

void debug_imgui::handle_events()
{
	ImGuiIO& io = ImGui::GetIO();

	// find view that has focus (should only be one at a time)
	debug_area* focus_view = nullptr;
	for(auto view_ptr = view_list.begin();view_ptr != view_list.end(); ++view_ptr)
		if((*view_ptr)->has_focus)
			focus_view = *view_ptr;

	// check views in main views also (only the disassembler view accepts inputs)
	if(view_main_disasm)
		if(view_main_disasm->has_focus)
			focus_view = view_main_disasm;

	if(m_machine->input().code_pressed(KEYCODE_LCONTROL))
		io.KeyCtrl = true;
	else
		io.KeyCtrl = false;
	if(m_machine->input().code_pressed(KEYCODE_LSHIFT))
		io.KeyShift = true;
	else
		io.KeyShift = false;
	if(m_machine->input().code_pressed(KEYCODE_LALT))
		io.KeyAlt = true;
	else
		io.KeyAlt = false;

	for(input_item_id id = ITEM_ID_A; id <= ITEM_ID_CANCEL; ++id)
	{
		if(m_machine->input().code_pressed(input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, id)))
		{
			if(m_mapping.count(id))
				io.AddKeyEvent(m_mapping[id], true);
		}
		else
		{
			if(m_mapping.count(id))
				io.AddKeyEvent(m_mapping[id], false);
		}
	}

	m_prev_mouse_button = m_mouse_button;
	m_key_char = 0;
	ui_event event;
	while(m_machine->ui_input().pop_event(&event))
	{
		switch (event.event_type)
		{
		case ui_event::type::POINTER_UPDATE:
			if(&m_machine->render().ui_target() != event.target)
				break;
			if(event.pointer_id != m_current_pointer)
			{
				if((0 > m_current_pointer) || ((event.pointer_pressed & 1) && !m_mouse_button))
					m_current_pointer = event.pointer_id;
			}
			if(event.pointer_id == m_current_pointer)
			{
				bool changed = (m_mouse_x != event.pointer_x) || (m_mouse_y != event.pointer_y) || (m_mouse_button != bool(event.pointer_buttons & 1));
				m_mouse_x = event.pointer_x;
				m_mouse_y = event.pointer_y;
				m_mouse_button = bool(event.pointer_buttons & 1);
				if(changed)
				{
					io.MousePos = ImVec2(m_mouse_x,m_mouse_y);
					io.MouseDown[0] = m_mouse_button;
				}
			}
			break;
		case ui_event::type::POINTER_LEAVE:
		case ui_event::type::POINTER_ABORT:
			if((&m_machine->render().ui_target() == event.target) && (event.pointer_id == m_current_pointer))
			{
				m_current_pointer = -1;
				bool changed = (m_mouse_x != event.pointer_x) || (m_mouse_y != event.pointer_y) || m_mouse_button;
				m_mouse_x = event.pointer_x;
				m_mouse_y = event.pointer_y;
				m_mouse_button = false;
				if(changed)
				{
					io.MousePos = ImVec2(m_mouse_x,m_mouse_y);
					io.MouseDown[0] = m_mouse_button;
				}
			}
			break;
		case ui_event::type::IME_CHAR:
			m_key_char = event.ch; // FIXME: assigning 4-byte UCS4 character to 8-bit variable
			if(focus_view)
				focus_view->view->process_char(m_key_char);
			return;
		default:
			break;
		}
	}

	// global keys
	if(ImGui::IsKeyPressed(ImGuiKey_F3,false))
	{
		if(ImGui::IsKeyDown(ImGuiKey_LeftShift))
			m_machine->schedule_hard_reset();
		else
		{
			m_machine->schedule_soft_reset();
			m_machine->debugger().console().get_visible_cpu()->debug()->go();
		}
	}

	if(ImGui::IsKeyPressed(ImGuiKey_F5,false))
	{
		m_machine->debugger().console().get_visible_cpu()->debug()->go();
		m_running = true;
	}
	if(ImGui::IsKeyPressed(ImGuiKey_F6,false))
	{
		m_machine->debugger().console().get_visible_cpu()->debug()->go_next_device();
		m_running = true;
	}
	if(ImGui::IsKeyPressed(ImGuiKey_F7,false))
	{
		m_machine->debugger().console().get_visible_cpu()->debug()->go_interrupt();
		m_running = true;
	}
	if(ImGui::IsKeyPressed(ImGuiKey_F8,false))
		m_machine->debugger().console().get_visible_cpu()->debug()->go_vblank();
	if(ImGui::IsKeyPressed(ImGuiKey_F9,false))
		m_machine->debugger().console().get_visible_cpu()->debug()->single_step_out();
	if(ImGui::IsKeyPressed(ImGuiKey_F10,false))
		m_machine->debugger().console().get_visible_cpu()->debug()->single_step_over();
	if(ImGui::IsKeyPressed(ImGuiKey_F11,false))
		m_machine->debugger().console().get_visible_cpu()->debug()->single_step();
	if(ImGui::IsKeyPressed(ImGuiKey_F12,false))
	{
		m_machine->debugger().console().get_visible_cpu()->debug()->go();
		m_hide = true;
	}

	if(ImGui::IsKeyPressed(ImGuiKey_D,false) && io.KeyCtrl)
		add_disasm(++m_win_count);
	if(ImGui::IsKeyPressed(ImGuiKey_M,false) && io.KeyCtrl)
		add_memory(++m_win_count);
	if(ImGui::IsKeyPressed(ImGuiKey_B,false) && io.KeyCtrl)
		add_bpoints(++m_win_count);
	if(ImGui::IsKeyPressed(ImGuiKey_W,false) && io.KeyCtrl)
		add_wpoints(++m_win_count);
	if(ImGui::IsKeyPressed(ImGuiKey_L,false) && io.KeyCtrl)
		add_log(++m_win_count);

}

void debug_imgui::handle_mouse_views()
{
	rectangle rect;
	bool clicked = false;
	if(m_mouse_button == true && m_prev_mouse_button == false)
		clicked = true;

	// check all views, and pass mouse clicks to them
	if(!m_mouse_button)
		return;
	rect.min_x = view_main_disasm->ofs_x;
	rect.min_y = view_main_disasm->ofs_y;
	rect.max_x = view_main_disasm->ofs_x + view_main_disasm->view_width;
	rect.max_y = view_main_disasm->ofs_y + view_main_disasm->view_height;
	if(rect.contains(m_mouse_x,m_mouse_y) && clicked && view_main_disasm->has_focus)
	{
		debug_view_xy topleft = view_main_disasm->view->visible_position();
		debug_view_xy newpos;
		newpos.x = topleft.x + (m_mouse_x-view_main_disasm->ofs_x) / m_text_size.x;
		newpos.y = topleft.y + (m_mouse_y-view_main_disasm->ofs_y) / m_text_size.y;
		view_main_disasm->view->set_cursor_position(newpos);
		view_main_disasm->view->set_cursor_visible(true);
	}
	for(auto it = view_list.begin();it != view_list.end();++it)
	{
		rect.min_x = (*it)->ofs_x;
		rect.min_y = (*it)->ofs_y;
		rect.max_x = (*it)->ofs_x + (*it)->view_width;
		rect.max_y = (*it)->ofs_y + (*it)->view_height;
		if(rect.contains(m_mouse_x,m_mouse_y) && clicked && (*it)->has_focus)
		{
			if((*it)->view->cursor_supported())
			{
				debug_view_xy topleft = (*it)->view->visible_position();
				debug_view_xy newpos;
				newpos.x = topleft.x + (m_mouse_x-(*it)->ofs_x) / m_text_size.x;
				newpos.y = topleft.y + (m_mouse_y-(*it)->ofs_y) / m_text_size.y;
				(*it)->view->set_cursor_position(newpos);
				(*it)->view->set_cursor_visible(true);
			}
		}
	}
}

void debug_imgui::handle_keys_views()
{
	debug_area* focus_view = nullptr;
	// find view that has focus (should only be one at a time)
	for(auto view_ptr = view_list.begin();view_ptr != view_list.end();++view_ptr)
		if((*view_ptr)->has_focus)
			focus_view = *view_ptr;

	// check views in main views also (only the disassembler view accepts inputs)
	if(view_main_disasm != nullptr)
		if(view_main_disasm->has_focus)
			focus_view = view_main_disasm;

	// if no view has focus, then there's nothing to do
	if(focus_view == nullptr)
		return;

	// pass keypresses to debug view with focus
	if(ImGui::IsKeyPressed(ImGuiKey_UpArrow))
		focus_view->view->process_char(DCH_UP);
	if(ImGui::IsKeyPressed(ImGuiKey_DownArrow))
		focus_view->view->process_char(DCH_DOWN);
	if(ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
	{
		if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
			focus_view->view->process_char(DCH_CTRLLEFT);
		else
			focus_view->view->process_char(DCH_LEFT);
	}
	if(ImGui::IsKeyPressed(ImGuiKey_RightArrow))
	{
		if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
			focus_view->view->process_char(DCH_CTRLRIGHT);
		else
			focus_view->view->process_char(DCH_RIGHT);
	}
	if(ImGui::IsKeyPressed(ImGuiKey_PageUp))
		focus_view->view->process_char(DCH_PUP);
	if(ImGui::IsKeyPressed(ImGuiKey_PageDown))
		focus_view->view->process_char(DCH_PDOWN);
	if(ImGui::IsKeyPressed(ImGuiKey_Home))
	{
		if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
			focus_view->view->process_char(DCH_CTRLHOME);
		else
			focus_view->view->process_char(DCH_HOME);
	}
	if(ImGui::IsKeyPressed(ImGuiKey_End))
	{
		if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
			focus_view->view->process_char(DCH_CTRLEND);
		else
			focus_view->view->process_char(DCH_END);
	}

}

void debug_imgui::handle_console(running_machine* machine)
{
	if(view_main_console->exec_cmd && view_main_console->type == DVT_CONSOLE)
	{
		// if console input is empty, then do a single step
		if(strlen(view_main_console->console_input) == 0)
		{
			m_machine->debugger().console().get_visible_cpu()->debug()->single_step();
			view_main_console->exec_cmd = false;
			history_pos = view_main_console->console_history.size();
			return;
		}
		m_machine->debugger().console().execute_command(view_main_console->console_input, true);
		// check for commands that start execution (so that input fields can be disabled)
		if(strcmp(view_main_console->console_input,"g") == 0)
			m_running = true;
		if(strncmp(view_main_console->console_input,"g ",2) == 0)
			m_running = true;
		if(strcmp(view_main_console->console_input,"go") == 0)
			m_running = true;
		if(strncmp(view_main_console->console_input,"go ",3) == 0)
			m_running = true;
		if(strcmp(view_main_console->console_input,"gi") == 0)
			m_running = true;
		if(strncmp(view_main_console->console_input,"gi ",3) == 0)
			m_running = true;
		if(strcmp(view_main_console->console_input,"gint") == 0)
			m_running = true;
		if(strncmp(view_main_console->console_input,"gint ",5) == 0)
			m_running = true;
		if(strcmp(view_main_console->console_input,"gt") == 0)
			m_running = true;
		if(strncmp(view_main_console->console_input,"gt ",3) == 0)
			m_running = true;
		if(strcmp(view_main_console->console_input,"gtime") == 0)
			m_running = true;
		if(strncmp(view_main_console->console_input,"gtime ",6) == 0)
			m_running = true;
		if(strcmp(view_main_console->console_input,"n") == 0)
			m_running = true;
		if(strcmp(view_main_console->console_input,"next") == 0)
			m_running = true;
		// don't bother adding to history if the current command matches the previous one
		if(view_main_console->console_prev != view_main_console->console_input)
		{
			view_main_console->console_history.emplace_back(std::string(view_main_console->console_input));
			view_main_console->console_prev = view_main_console->console_input;
		}
		history_pos = view_main_console->console_history.size();
		strcpy(view_main_console->console_input,"");
		view_main_console->exec_cmd = false;
	}
}

int debug_imgui::history_set(ImGuiInputTextCallbackData* data)
{
	if(view_main_console->console_history.size() == 0)
		return 0;

	switch(data->EventKey)
	{
		case ImGuiKey_UpArrow:
			if(history_pos > 0)
				history_pos--;
			break;
		case ImGuiKey_DownArrow:
			if(history_pos < view_main_console->console_history.size())
				history_pos++;
			break;
		default:
			break;
	}

	if(history_pos == view_main_console->console_history.size())
		data->CursorPos = data->BufTextLen = (int)snprintf(data->Buf, (size_t)data->BufSize, "%s", "");
	else
		data->CursorPos = data->BufTextLen = (int)snprintf(data->Buf, (size_t)data->BufSize, "%s", view_main_console->console_history[history_pos].c_str());

	data->BufDirty = true;
	return 0;
}

void debug_imgui::update_cpu_view(device_t* device)
{
	const debug_view_source *source;
	source = view_main_disasm->view->source_for_device(device);
	view_main_disasm->view->set_source(*source);
	source = view_main_regs->view->source_for_device(device);
	view_main_regs->view->set_source(*source);
}

void debug_imgui::draw_view(debug_area* view_ptr, bool exp_change)
{
	const debug_view_char *viewdata;
	ImDrawList* drawlist;
	debug_view_xy vsize,totalsize,pos;
	unsigned char v;
	int x,y;
	ImVec2 xy1,xy2;
	ImVec2 fsize = ImGui::CalcTextSize("A"); // any character will do, we should be using a monospaced font
	rgb_t bg, fg;
	rgb_t base(0xe6, 0xff, 0xff, 0xff);

	totalsize = view_ptr->view->total_size();

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));

	// if the view has changed its expression (disasm, memory), then update scroll bar and view cursor
	if(exp_change)
	{
		if(view_ptr->view->cursor_supported())
		{
			view_ptr->view->set_cursor_visible(true);
			view_ptr->view->set_cursor_position(debug_view_xy(0,view_ptr->view->visible_position().y));
		}
		if(view_ptr->type != DVT_MEMORY)  // no scroll bars in memory views
			ImGui::SetScrollY(view_ptr->view->visible_position().y * fsize.y);
	}

	// update view location, while the cursor is at 0,0.
	view_ptr->ofs_x = ImGui::GetCursorScreenPos().x;
	view_ptr->ofs_y = ImGui::GetCursorScreenPos().y;
	view_ptr->view_width = ImGui::GetContentRegionAvail().x;
	view_ptr->view_height = ImGui::GetContentRegionAvail().y;
	view_ptr->has_focus = ImGui::IsWindowFocused();
	drawlist = ImGui::GetWindowDrawList();

	// temporarily set cursor to the last line, this will set the scroll bar range
	if(view_ptr->type != DVT_MEMORY)  // no scroll bars in memory views
	{
		ImGui::SetCursorPosY((totalsize.y) * fsize.y);
		ImGui::Dummy(ImVec2(0,0)); // some object is required for validation
	}

	// set the visible area to be displayed
	vsize.x = view_ptr->view_width / fsize.x;
	vsize.y = (view_ptr->view_height / fsize.y) + 1;
	view_ptr->view->set_visible_size(vsize);

	// set the visible position
	if(view_ptr->type != DVT_MEMORY)  // since ImGui cannot handle huge memory views, we'll just let the view control the displayed area
	{
		pos.x = 0;
		pos.y = ImGui::GetScrollY() / fsize.y;
		view_ptr->view->set_visible_position(pos);
	}

	viewdata = view_ptr->view->viewdata();

	xy1.x = view_ptr->ofs_x;
	xy1.y = view_ptr->ofs_y + ImGui::GetScrollY();
	xy2 = fsize;
	xy2.x += view_ptr->ofs_x;
	xy2.y += view_ptr->ofs_y + ImGui::GetScrollY();
	for(y=0;y<vsize.y;y++)
	{
		for(x=0;x<vsize.x;x++)
		{
			char str[2];
			map_attr_to_fg_bg(viewdata->attrib,&fg,&bg);
			ImU32 fg_col = IM_COL32(fg.r(),fg.g(),fg.b(),fg.a());
			str[0] = v = viewdata->byte;
			str[1] = '\0';
			if(bg != base)
			{
				ImU32 bg_col = IM_COL32(bg.r(),bg.g(),bg.b(),bg.a());
				xy1.x++; xy2.x++;
				drawlist->AddRectFilled(xy1,xy2,bg_col);
				xy1.x--; xy2.x--;
			}
			drawlist->AddText(xy1,fg_col,str);
			xy1.x += fsize.x;
			xy2.x += fsize.x;
			viewdata++;
		}
		xy1.x = view_ptr->ofs_x;
		xy2.x = view_ptr->ofs_x + fsize.x;
		xy1.y += fsize.y;
		xy2.y += fsize.y;
	}

	// draw a rect around a view if it has focus
	if(view_ptr->has_focus)
	{
		ImU32 col = IM_COL32(127,127,127,76);
		drawlist->AddRect(ImVec2(view_ptr->ofs_x,view_ptr->ofs_y + ImGui::GetScrollY()),
			ImVec2(view_ptr->ofs_x + view_ptr->view_width,view_ptr->ofs_y + ImGui::GetScrollY() + view_ptr->view_height),col);
	}

	// if the vertical scroll bar is at the end, then force it to the maximum value in case of an update
	if(view_ptr->scroll_end)
		ImGui::SetScrollY(ImGui::GetScrollMaxY());
	// and update the scroll end flag
	view_ptr->scroll_end = false;
	if(view_ptr->scroll_follow)
		if(ImGui::GetScrollY() == ImGui::GetScrollMaxY() || ImGui::GetScrollMaxY() < 0)
			view_ptr->scroll_end = true;

	ImGui::PopStyleVar(2);
}

void debug_imgui::draw_bpoints(debug_area* view_ptr, bool* opened)
{
	ImGui::SetNextWindowSize(ImVec2(view_ptr->width,view_ptr->height + ImGui::GetTextLineHeight()),ImGuiCond_Once);
	if(ImGui::Begin(view_ptr->title.c_str(),opened))
	{
		view_ptr->is_collapsed = false;
		ImGui::BeginChild("##break_output", ImVec2(ImGui::GetWindowWidth() - 16,ImGui::GetWindowHeight() - ImGui::GetTextLineHeight() - ImGui::GetCursorPosY()));  // account for title bar and widgets already drawn
		draw_view(view_ptr,false);
		ImGui::EndChild();

		ImGui::End();
	}
	else
		view_ptr->is_collapsed = true;
}

void debug_imgui::add_bpoints(int id)
{
	std::stringstream str;
	debug_area* new_view;
	new_view = dview_alloc(*m_machine, DVT_BREAK_POINTS);
	str << id;
	str << ": Breakpoints";
	new_view->title = str.str();
	new_view->width = 500;
	new_view->height = 300;
	new_view->ofs_x = 0;
	new_view->ofs_y = 0;
	view_list_add(new_view);
}

void debug_imgui::add_wpoints(int id)
{
	std::stringstream str;
	debug_area* new_view;
	new_view = dview_alloc(*m_machine, DVT_WATCH_POINTS);
	str << id;
	str << ": Watchpoints";
	new_view->title = str.str();
	new_view->width = 500;
	new_view->height = 300;
	new_view->ofs_x = 0;
	new_view->ofs_y = 0;
	view_list_add(new_view);
}

void debug_imgui::draw_log(debug_area* view_ptr, bool* opened)
{
	ImGui::SetNextWindowSize(ImVec2(view_ptr->width,view_ptr->height + ImGui::GetTextLineHeight()),ImGuiCond_Once);
	if(ImGui::Begin(view_ptr->title.c_str(),opened))
	{
		view_ptr->is_collapsed = false;
		ImGui::BeginChild("##log_output", ImVec2(ImGui::GetWindowWidth() - 16,ImGui::GetWindowHeight() - ImGui::GetTextLineHeight() - ImGui::GetCursorPosY()));  // account for title bar and widgets already drawn
		draw_view(view_ptr,false);
		ImGui::EndChild();

		ImGui::End();
	}
	else
		view_ptr->is_collapsed = true;
}

void debug_imgui::add_log(int id)
{
	std::stringstream str;
	debug_area* new_view;
	new_view = dview_alloc(*m_machine, DVT_LOG);
	str << id;
	str << ": Error log";
	new_view->title = str.str();
	new_view->width = 500;
	new_view->height = 300;
	new_view->ofs_x = 0;
	new_view->ofs_y = 0;
	new_view->scroll_follow = true;
	view_list_add(new_view);
}

void debug_imgui::draw_disasm(debug_area* view_ptr, bool* opened)
{
	ImGui::SetNextWindowSize(ImVec2(view_ptr->width,view_ptr->height + ImGui::GetTextLineHeight()),ImGuiCond_Once);
	if(ImGui::Begin(view_ptr->title.c_str(),opened,ImGuiWindowFlags_MenuBar))
	{
		bool exp_change = false;

		view_ptr->is_collapsed = false;
		if(ImGui::BeginMenuBar())
		{
			if(ImGui::BeginMenu("Options"))
			{
				auto* disasm = downcast<debug_view_disasm*>(view_ptr->view);
				int rightcol = disasm->right_column();

				if(ImGui::MenuItem("Raw opcodes", nullptr,(rightcol == DASM_RIGHTCOL_RAW) ? true : false))
					disasm->set_right_column(DASM_RIGHTCOL_RAW);
				if(ImGui::MenuItem("Encrypted opcodes", nullptr,(rightcol == DASM_RIGHTCOL_ENCRYPTED) ? true : false))
					disasm->set_right_column(DASM_RIGHTCOL_ENCRYPTED);
				if(ImGui::MenuItem("No opcodes", nullptr,(rightcol == DASM_RIGHTCOL_NONE) ? true : false))
					disasm->set_right_column(DASM_RIGHTCOL_NONE);
				if(ImGui::MenuItem("Comments", nullptr,(rightcol == DASM_RIGHTCOL_COMMENTS) ? true : false))
					disasm->set_right_column(DASM_RIGHTCOL_COMMENTS);

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
		if(m_running)
			flags |= ImGuiInputTextFlags_ReadOnly;
		ImGui::Combo("##cpu",&view_ptr->src_sel,get_view_source,view_ptr->view,view_ptr->view->source_count());
		ImGui::SameLine();
		ImGui::PushItemWidth(-1.0f);
		if(ImGui::InputText("##addr",view_ptr->console_input,512,flags))
		{
			downcast<debug_view_disasm *>(view_ptr->view)->set_expression(view_ptr->console_input);
			exp_change = true;
		}
		ImGui::PopItemWidth();
		ImGui::Separator();

		// disassembly portion
		unsigned idx = 0;
		const debug_view_source* src = view_ptr->view->source(idx);
		do
		{
			if(view_ptr->src_sel == idx)
				view_ptr->view->set_source(*src);
			src = view_ptr->view->source(++idx);
		}
		while (src);

		ImGui::BeginChild("##disasm_output", ImVec2(ImGui::GetWindowWidth() - 16,ImGui::GetWindowHeight() - ImGui::GetTextLineHeight() - ImGui::GetCursorPosY()));  // account for title bar and widgets already drawn
		draw_view(view_ptr,exp_change);
		ImGui::EndChild();

		ImGui::End();
	}
	else
		view_ptr->is_collapsed = true;
}

void debug_imgui::add_disasm(int id)
{
	std::stringstream str;
	debug_area* new_view;
	new_view = dview_alloc(*m_machine, DVT_DISASSEMBLY);
	str << id;
	str << ": Disassembly";
	new_view->title = str.str();
	new_view->width = 500;
	new_view->height = 300;
	new_view->ofs_x = 0;
	new_view->ofs_y = 0;
	new_view->src_sel = 0;
	strcpy(new_view->console_input,"curpc");
	view_list_add(new_view);
}

void debug_imgui::draw_memory(debug_area* view_ptr, bool* opened)
{
	ImGui::SetNextWindowSize(ImVec2(view_ptr->width,view_ptr->height + ImGui::GetTextLineHeight()),ImGuiCond_Once);
	if(ImGui::Begin(view_ptr->title.c_str(),opened,ImGuiWindowFlags_MenuBar))
	{
		bool exp_change = false;

		view_ptr->is_collapsed = false;
		if(ImGui::BeginMenuBar())
		{
			if(ImGui::BeginMenu("Options"))
			{
				auto* mem = downcast<debug_view_memory*>(view_ptr->view);
				bool physical = mem->physical();
				bool rev = mem->reverse();
				debug_view_memory::data_format format = mem->get_data_format();
				uint32_t chunks = mem->chunks_per_row();
				int radix = mem->address_radix();

				if(ImGui::MenuItem("1-byte hexadecimal", nullptr,(format == debug_view_memory::data_format::HEX_8BIT) ? true : false))
					mem->set_data_format(debug_view_memory::data_format::HEX_8BIT);
				if(ImGui::MenuItem("2-byte hexadecimal", nullptr,(format == debug_view_memory::data_format::HEX_16BIT) ? true : false))
					mem->set_data_format(debug_view_memory::data_format::HEX_16BIT);
				if(ImGui::MenuItem("4-byte hexadecimal", nullptr,(format == debug_view_memory::data_format::HEX_32BIT) ? true : false))
					mem->set_data_format(debug_view_memory::data_format::HEX_32BIT);
				if(ImGui::MenuItem("8-byte hexadecimal", nullptr,(format == debug_view_memory::data_format::HEX_64BIT) ? true : false))
					mem->set_data_format(debug_view_memory::data_format::HEX_64BIT);
				if(ImGui::MenuItem("1-byte octal", nullptr,(format == debug_view_memory::data_format::OCTAL_8BIT) ? true : false))
					mem->set_data_format(debug_view_memory::data_format::OCTAL_8BIT);
				if(ImGui::MenuItem("2-byte octal", nullptr,(format == debug_view_memory::data_format::OCTAL_16BIT) ? true : false))
					mem->set_data_format(debug_view_memory::data_format::OCTAL_16BIT);
				if(ImGui::MenuItem("4-byte octal", nullptr,(format == debug_view_memory::data_format::OCTAL_32BIT) ? true : false))
					mem->set_data_format(debug_view_memory::data_format::OCTAL_32BIT);
				if(ImGui::MenuItem("8-byte octal", nullptr,(format == debug_view_memory::data_format::OCTAL_64BIT) ? true : false))
					mem->set_data_format(debug_view_memory::data_format::OCTAL_64BIT);
				if(ImGui::MenuItem("32-bit floating point", nullptr,(format == debug_view_memory::data_format::FLOAT_32BIT) ? true : false))
					mem->set_data_format(debug_view_memory::data_format::FLOAT_32BIT);
				if(ImGui::MenuItem("64-bit floating point", nullptr,(format == debug_view_memory::data_format::FLOAT_64BIT) ? true : false))
					mem->set_data_format(debug_view_memory::data_format::FLOAT_64BIT);
				if(ImGui::MenuItem("80-bit floating point", nullptr,(format == debug_view_memory::data_format::FLOAT_80BIT) ? true : false))
					mem->set_data_format(debug_view_memory::data_format::FLOAT_80BIT);
				ImGui::Separator();
				if(ImGui::MenuItem("Hexadecimal Addresses", nullptr,(radix == 16)))
					mem->set_address_radix(16);
				if(ImGui::MenuItem("Decimal Addresses", nullptr,(radix == 10)))
					mem->set_address_radix(10);
				if(ImGui::MenuItem("Octal Addresses", nullptr,(radix == 8)))
					mem->set_address_radix(8);
				ImGui::Separator();
				if(ImGui::MenuItem("Logical addresses", nullptr,!physical))
					mem->set_physical(false);
				if(ImGui::MenuItem("Physical addresses", nullptr,physical))
					mem->set_physical(true);
				ImGui::Separator();
				if(ImGui::MenuItem("Reverse view", nullptr,rev))
					mem->set_reverse(!rev);
				ImGui::Separator();
				if(ImGui::MenuItem("Increase bytes per line"))
					mem->set_chunks_per_row(chunks+1);
				if(ImGui::MenuItem("Decrease bytes per line"))
					mem->set_chunks_per_row(chunks-1);

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
		ImGui::PushItemWidth(100.0f);
		if(m_running)
			flags |= ImGuiInputTextFlags_ReadOnly;
		if(ImGui::InputText("##addr",view_ptr->console_input,512,flags))
		{
			downcast<debug_view_memory *>(view_ptr->view)->set_expression(view_ptr->console_input);
			exp_change = true;
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(-1.0f);
		ImGui::Combo("##region",&view_ptr->src_sel,get_view_source,view_ptr->view,view_ptr->view->source_count());
		ImGui::PopItemWidth();
		ImGui::Separator();

		// memory editor portion
		unsigned idx = 0;
		const debug_view_source* src = view_ptr->view->source(idx);
		do
		{
			if(view_ptr->src_sel == idx)
				view_ptr->view->set_source(*src);
			src = view_ptr->view->source(++idx);
		}
		while (src);

		ImGui::BeginChild("##memory_output", ImVec2(ImGui::GetWindowWidth() - 16,ImGui::GetWindowHeight() - ImGui::GetTextLineHeight() - ImGui::GetCursorPosY()));  // account for title bar and widgets already drawn
		draw_view(view_ptr,exp_change);
		ImGui::EndChild();

		ImGui::End();
	}
	else
		view_ptr->is_collapsed = true;
}

void debug_imgui::add_memory(int id)
{
	std::stringstream str;
	debug_area* new_view;
	new_view = dview_alloc(*m_machine, DVT_MEMORY);
	str << id;
	str << ": Memory";
	new_view->title = str.str();
	new_view->width = 500;
	new_view->height = 300;
	new_view->ofs_x = 0;
	new_view->ofs_y = 0;
	new_view->src_sel = 0;
	strcpy(new_view->console_input,"0");
	view_list_add(new_view);
}

void debug_imgui::mount_image()
{
	if(m_selected_file != nullptr)
	{
		std::error_condition err;
		switch(m_selected_file->type)
		{
			case file_entry_type::DRIVE:
			case file_entry_type::DIRECTORY:
				{
					util::zippath_directory::ptr dir;
					err = util::zippath_directory::open(m_selected_file->fullpath, dir);
				}
				if(!err)
				{
					m_filelist_refresh = true;
					strcpy(m_path,m_selected_file->fullpath.c_str());
				}
				break;
			case file_entry_type::FILE:
				m_dialog_image->load(m_selected_file->fullpath);
				ImGui::CloseCurrentPopup();
				m_mount_open = false;
				break;
		}
	}
}

void debug_imgui::create_image()
{
	std::pair<std::error_condition, std::string> res;

	auto *fd = dynamic_cast<floppy_image_device *>(m_dialog_image);
	if(fd != nullptr)
	{
		res = fd->create(m_path,nullptr,nullptr);
		if(!res.first)
			fd->setup_write(m_typelist.at(m_format_sel).format);
	}
	else
		res = m_dialog_image->create(m_path,nullptr,nullptr);
	if(!res.first)
		ImGui::CloseCurrentPopup();
	// TODO: add a messagebox to display on an error
}

void debug_imgui::refresh_filelist()
{
	uint8_t first = 0;

	// todo
	m_filelist.clear();
	m_filelist_refresh = false;

	util::zippath_directory::ptr dir;
	std::error_condition const err = util::zippath_directory::open(m_path,dir);
	if(!err)
	{
		// add drives
		for(std::string const &volume_name : osd_get_volume_names())
		{
			file_entry temp;
			temp.type = file_entry_type::DRIVE;
			temp.basename = volume_name;
			temp.fullpath = volume_name;
			m_filelist.emplace_back(std::move(temp));
		}
		first = m_filelist.size();
		const directory::entry *dirent;
		while((dirent = dir->readdir()) != nullptr)
		{
			file_entry temp;
			switch(dirent->type)
			{
				case directory::entry::entry_type::FILE:
					temp.type = file_entry_type::FILE;
					break;
				case directory::entry::entry_type::DIR:
					temp.type = file_entry_type::DIRECTORY;
					break;
				default:
					break;
			}
			temp.basename = std::string(dirent->name);
			temp.fullpath = util::zippath_combine(m_path,dirent->name);
			m_filelist.emplace_back(std::move(temp));
		}
	}
	dir.reset();

	// sort file list, as it is not guaranteed to be in any particular order
	std::sort(m_filelist.begin()+first,m_filelist.end(),[](file_entry x, file_entry y) { return x.basename < y.basename; } );
}

void debug_imgui::refresh_typelist()
{
	auto *fd = static_cast<floppy_image_device *>(m_dialog_image);

	m_typelist.clear();
	if(m_dialog_image->formatlist().empty())
		return;
	if(fd == nullptr)
		return;

	for(const floppy_image_format_t* flist : fd->get_formats())
	{
		if(flist->supports_save())
		{
			image_type_entry temp;
			temp.format = flist;
			temp.shortname = flist->name();
			temp.longname = flist->description();
			m_typelist.emplace_back(std::move(temp));
		}
	}
}

void debug_imgui::draw_images_menu()
{
	if(ImGui::BeginMenu("Images"))
	{
		int x = 0;
		for (device_image_interface &img : image_interface_enumerator(m_machine->root_device()))
		{
			x++;
			std::string str = string_format(" %s : %s##%i",img.device().name(),img.exists() ? img.filename() : "[Empty slot]",x);
			if(ImGui::BeginMenu(str.c_str()))
			{
				if(ImGui::MenuItem("Mount..."))
				{
					m_dialog_image = &img;
					m_filelist_refresh = true;
					m_mount_open = true;
					m_selected_file = nullptr;  // start with no file selected
					if (img.exists())  // use image path if one is already mounted
						strcpy(m_path,util::zippath_parent(m_dialog_image->filename()).c_str());
					else
						strcpy(m_path,img.working_directory().c_str());
				}
				if(ImGui::MenuItem("Unmount"))
					img.unload();
				ImGui::Separator();
				if(img.is_creatable())
				{
					if(ImGui::MenuItem("Create..."))
					{
						m_dialog_image = &img;
						m_create_open = true;
						m_create_confirm_wait = false;
						refresh_typelist();
						strcpy(m_path,img.working_directory().c_str());
					}
				}
				// TODO: Cassette controls
				ImGui::EndMenu();
			}
		}
		ImGui::EndMenu();
	}
}

void debug_imgui::draw_mount_dialog(const char* label)
{
	// render dialog
	//ImGui::SetNextWindowContentWidth(200.0f);
	if(ImGui::BeginPopupModal(label,nullptr,ImGuiWindowFlags_AlwaysAutoResize))
	{
		if(m_filelist_refresh)
			refresh_filelist();
		if(ImGui::InputText("##mountpath",m_path,1024,ImGuiInputTextFlags_EnterReturnsTrue))
			m_filelist_refresh = true;
		ImGui::Separator();

		ImVec2 listbox_size;
		listbox_size.x = 0.0f;
		listbox_size.y = ImGui::GetTextLineHeightWithSpacing() * 15.25f;

		if(ImGui::BeginListBox("##filelist",listbox_size))
		{
			for(auto f = m_filelist.begin();f != m_filelist.end();++f)
			{
				std::string txt_name;
				bool sel = false;
				switch((*f).type)
				{
					case file_entry_type::DRIVE:
						txt_name.assign("[DRIVE] ");
						break;
					case file_entry_type::DIRECTORY:
						txt_name.assign("[DIR]   ");
						break;
					case file_entry_type::FILE:
						txt_name.assign("[FILE]  ");
						break;
				}
				txt_name.append((*f).basename);
				if(m_selected_file == &(*f))
					sel = true;
				if(ImGui::Selectable(txt_name.c_str(),sel,ImGuiSelectableFlags_AllowDoubleClick))
				{
					m_selected_file = &(*f);
					if(ImGui::IsMouseDoubleClicked(0))
					{
						mount_image();
					}
				}
			}
			ImGui::EndListBox();
		}
		ImGui::Separator();
		if(ImGui::Button("Cancel##mount"))
		{
			ImGui::CloseCurrentPopup();
			m_mount_open = false;
		}
		ImGui::SameLine();
		if(ImGui::Button("OK##mount"))
			mount_image();
		ImGui::EndPopup();
	}
}

void debug_imgui::draw_create_dialog(const char* label)
{
	// render dialog
	//ImGui::SetNextWindowContentWidth(200.0f);
	if(ImGui::BeginPopupModal(label,nullptr,ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::LabelText("##static1","Filename:");
		ImGui::SameLine();
		if(ImGui::InputText("##createfilename",m_path,1024,ImGuiInputTextFlags_EnterReturnsTrue))
		{
			auto entry = osd_stat(m_path);
			auto file_type = (entry != nullptr) ? entry->type : directory::entry::entry_type::NONE;
			if(file_type == directory::entry::entry_type::NONE)
				create_image();
			if(file_type == directory::entry::entry_type::FILE)
				m_create_confirm_wait = true;
			// cannot overwrite a directory, so nothing will be none in that case.
		}

		// format combo box for floppy devices
		auto *fd = dynamic_cast<floppy_image_device *>(m_dialog_image);
		if(fd != nullptr)
		{
			std::string combo_str;
			combo_str.clear();
			for(auto f = m_typelist.begin();f != m_typelist.end();++f)
			{
				// TODO: perhaps do this at the time the format list is generated, rather than every frame
				combo_str.append((*f).longname);
				combo_str.append(1,'\0');
			}
			combo_str.append(1,'\0');
			ImGui::Separator();
			ImGui::LabelText("##static2","Format:");
			ImGui::SameLine();
			ImGui::Combo("##formatcombo",&m_format_sel,combo_str.c_str(),m_typelist.size());
		}

		if(m_create_confirm_wait)
		{
			ImGui::Separator();
			ImGui::Text("File already exists.  Are you sure you wish to overwrite it?");
			ImGui::Separator();
			if(ImGui::Button("Cancel##mount"))
				ImGui::CloseCurrentPopup();
			ImGui::SameLine();
			if(ImGui::Button("OK##mount"))
				create_image();
		}
		else
		{
			ImGui::Separator();
			if(ImGui::Button("Cancel##mount"))
			{
				ImGui::CloseCurrentPopup();
				m_create_open = false;
			}
			ImGui::SameLine();
			if(ImGui::Button("OK##mount"))
			{
				auto entry = osd_stat(m_path);
				auto file_type = (entry != nullptr) ? entry->type : directory::entry::entry_type::NONE;
				if(file_type == directory::entry::entry_type::NONE)
					create_image();
				if(file_type == directory::entry::entry_type::FILE)
					m_create_confirm_wait = true;
				// cannot overwrite a directory, so nothing will be none in that case.
				m_create_open = false;
			}
		}
		ImGui::EndPopup();
	}
}

void debug_imgui::draw_console()
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	bool show_menu = false;

	if(view_main_disasm == nullptr || view_main_regs == nullptr || view_main_console == nullptr)
		return;

	ImGui::SetNextWindowSize(ImVec2(view_main_regs->width + view_main_disasm->width,view_main_disasm->height + view_main_console->height + ImGui::GetTextLineHeight()*3),ImGuiCond_Once);
	if(ImGui::Begin(view_main_console->title.c_str(), nullptr,flags))
	{
		std::string str;

		if(ImGui::BeginMenuBar())
		{
			if(ImGui::BeginMenu("Debug"))
			{
				show_menu = true;
				if(ImGui::MenuItem("New disassembly window", "Ctrl+D"))
					add_disasm(++m_win_count);
				if(ImGui::MenuItem("New memory window", "Ctrl+M"))
					add_memory(++m_win_count);
				if(ImGui::MenuItem("New breakpoints window", "Ctrl+B"))
					add_bpoints(++m_win_count);
				if(ImGui::MenuItem("New watchpoints window", "Ctrl+W"))
					add_wpoints(++m_win_count);
				if(ImGui::MenuItem("New log window", "Ctrl+L"))
					add_log(++m_win_count);
				ImGui::Separator();
				if(ImGui::MenuItem("Run", "F5"))
				{
					m_machine->debugger().console().get_visible_cpu()->debug()->go();
					m_running = true;
				}
				if(ImGui::MenuItem("Go to next CPU", "F6"))
				{
					m_machine->debugger().console().get_visible_cpu()->debug()->go_next_device();
					m_running = true;
				}
				if(ImGui::MenuItem("Run until next interrupt", "F7"))
				{
					m_machine->debugger().console().get_visible_cpu()->debug()->go_interrupt();
					m_running = true;
				}
				if(ImGui::MenuItem("Run until VBLANK", "F8"))
					m_machine->debugger().console().get_visible_cpu()->debug()->go_vblank();
				if(ImGui::MenuItem("Run and hide debugger", "F12"))
				{
					m_machine->debugger().console().get_visible_cpu()->debug()->go();
					m_hide = true;
				}
				ImGui::Separator();
				if(ImGui::MenuItem("Single step", "F11"))
					m_machine->debugger().console().get_visible_cpu()->debug()->single_step();
				if(ImGui::MenuItem("Step over", "F10"))
					m_machine->debugger().console().get_visible_cpu()->debug()->single_step_over();
				if(ImGui::MenuItem("Step out", "F9"))
					m_machine->debugger().console().get_visible_cpu()->debug()->single_step_out();

				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Window"))
			{
				show_menu = true;
				if(ImGui::MenuItem("Show all"))
				{
					for(auto view_ptr = view_list.begin();view_ptr != view_list.end();++view_ptr)
						ImGui::SetWindowCollapsed((*view_ptr)->title.c_str(),false);
				}
				ImGui::Separator();
				// list all extra windows, so we can un-collapse the windows if necessary
				for(auto view_ptr = view_list.begin();view_ptr != view_list.end();++view_ptr)
				{
					bool collapsed = false;
					if((*view_ptr)->is_collapsed)
						collapsed = true;
					if(ImGui::MenuItem((*view_ptr)->title.c_str(), nullptr,!collapsed))
						ImGui::SetWindowCollapsed((*view_ptr)->title.c_str(),false);
				}
				ImGui::EndMenu();
			}
			if(m_has_images)
			{
				show_menu = true;
				draw_images_menu();
			}
			ImGui::EndMenuBar();
		}

		// CPU state portion
		ImGui::BeginChild("##state_output", ImVec2(180,ImGui::GetWindowHeight() - ImGui::GetTextLineHeight()*4));  // account for title bar and menu
		draw_view(view_main_regs,false);
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("##right_side", ImVec2(ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - 8,ImGui::GetWindowHeight() - ImGui::GetTextLineHeight()*2));
		// disassembly portion
		ImGui::BeginChild("##disasm_output", ImVec2(ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - 8,(ImGui::GetWindowHeight() - ImGui::GetTextLineHeight()*4)/2));
		draw_view(view_main_disasm,false);
		ImGui::EndChild();

		ImGui::Separator();

		// console portion
		ImGui::BeginChild("##console_output", ImVec2(ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - 8,(ImGui::GetWindowHeight() - ImGui::GetTextLineHeight()*4)/2 - ImGui::GetTextLineHeight()));
		draw_view(view_main_console,false);
		ImGui::EndChild();
		ImGui::Separator();

		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;
		if(m_running)
			flags |= ImGuiInputTextFlags_ReadOnly;
		ImGui::PushItemWidth(-1.0f);
		if(ImGui::InputText("##console_input",view_main_console->console_input,512,flags,history_set))
			view_main_console->exec_cmd = true;
		if ((ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0) && !show_menu))
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		if(m_mount_open)
		{
			ImGui::OpenPopup("Mount Image");
			draw_mount_dialog("Mount Image");  // draw mount image dialog if open
		}
		if(m_create_open)
		{
			ImGui::OpenPopup("Create Image");
			draw_create_dialog("Create Image");  // draw create image dialog if open
		}
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::End();
	}
}

void debug_imgui::update()
{
	debug_area* to_delete = nullptr;
	//debug_area* view_ptr = view_list;
	std::vector<debug_area*>::iterator view_ptr;
	bool opened;
	ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(1.0f,1.0f,1.0f,0.9f));
	ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.0f,0.0f,0.0f,1.0f));
	ImGui::PushStyleColor(ImGuiCol_TextDisabled,ImVec4(0.0f,0.0f,1.0f,1.0f));
	ImGui::PushStyleColor(ImGuiCol_MenuBarBg,ImVec4(0.5f,0.5f,0.5f,0.8f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg,ImVec4(0.6f,0.6f,0.8f,0.8f));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive,ImVec4(0.7f,0.7f,0.95f,0.8f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg,ImVec4(0.5f,0.5f,0.5f,0.8f));
	ImGui::PushStyleColor(ImGuiCol_PopupBg,ImVec4(0.8f,0.8f,0.8f,0.8f));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab,ImVec4(0.6f,0.6f,0.6f,0.8f));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered,ImVec4(0.7f,0.7f,0.7f,0.8f));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive,ImVec4(0.9f,0.9f,0.9f,0.8f));
	ImGui::PushStyleColor(ImGuiCol_Border,ImVec4(0.7f,0.7f,0.7f,0.8f));
	m_text_size = ImGui::CalcTextSize("A");  // hopefully you're using a monospaced font...
	draw_console();  // We'll always have a console window

	view_ptr = view_list.begin();
	while(view_ptr != view_list.end())
	{
		opened = true;
		switch((*view_ptr)->type)
		{
		case DVT_DISASSEMBLY:
			draw_disasm((*view_ptr),&opened);
			if(opened == false)
				to_delete = (*view_ptr);
			break;
		case DVT_MEMORY:
			draw_memory((*view_ptr),&opened);
			if(opened == false)
				to_delete = (*view_ptr);
			break;
		case DVT_LOG:
			draw_log((*view_ptr),&opened);
			if(opened == false)
				to_delete = (*view_ptr);
			break;
		case DVT_BREAK_POINTS:
		case DVT_WATCH_POINTS:  // watchpoints window uses same drawing code as breakpoints window
			draw_bpoints((*view_ptr),&opened);
			if(opened == false)
				to_delete = (*view_ptr);
			break;
		}
		++view_ptr;
	}
	// check for a closed window
	if(to_delete != nullptr)
	{
		view_list_remove(to_delete);
		delete to_delete;
	}

	ImGui::PopStyleColor(12);
}

void debug_imgui::init_debugger(running_machine &machine)
{
	ImGuiIO& io = ImGui::GetIO();
	m_machine = &machine;
	m_mouse_button = false;
	if(strcmp(downcast<osd_options &>(m_machine->options()).video(),"bgfx") != 0)
		fatalerror("Error: ImGui debugger requires the BGFX renderer.\n");

	// check for any image devices (cassette, floppy, etc...)
	image_interface_enumerator iter(m_machine->root_device());
	if (iter.first() != nullptr)
		m_has_images = true;

	// map keys to ImGui inputs
	m_mapping[ITEM_ID_A] = ImGuiKey_A;
	m_mapping[ITEM_ID_C] = ImGuiKey_C;
	m_mapping[ITEM_ID_V] = ImGuiKey_V;
	m_mapping[ITEM_ID_X] = ImGuiKey_X;
	m_mapping[ITEM_ID_Y] = ImGuiKey_Y;
	m_mapping[ITEM_ID_Z] = ImGuiKey_Z;
	m_mapping[ITEM_ID_D] = ImGuiKey_D;
	m_mapping[ITEM_ID_M] = ImGuiKey_M;
	m_mapping[ITEM_ID_B] = ImGuiKey_B;
	m_mapping[ITEM_ID_W] = ImGuiKey_W;
	m_mapping[ITEM_ID_L] = ImGuiKey_L;
	m_mapping[ITEM_ID_BACKSPACE] = ImGuiKey_Backspace;
	m_mapping[ITEM_ID_DEL] = ImGuiKey_Delete;
	m_mapping[ITEM_ID_TAB] = ImGuiKey_Tab;
	m_mapping[ITEM_ID_PGUP] = ImGuiKey_PageUp;
	m_mapping[ITEM_ID_PGDN] = ImGuiKey_PageDown;
	m_mapping[ITEM_ID_HOME] = ImGuiKey_Home;
	m_mapping[ITEM_ID_END] = ImGuiKey_End;
	m_mapping[ITEM_ID_ESC] = ImGuiKey_Escape;
	m_mapping[ITEM_ID_ENTER] = ImGuiKey_Enter;
	m_mapping[ITEM_ID_LEFT] = ImGuiKey_LeftArrow;
	m_mapping[ITEM_ID_RIGHT] = ImGuiKey_RightArrow;
	m_mapping[ITEM_ID_UP] = ImGuiKey_UpArrow;
	m_mapping[ITEM_ID_DOWN] = ImGuiKey_DownArrow;
	m_mapping[ITEM_ID_F3] = ImGuiKey_F3;
	m_mapping[ITEM_ID_F5] = ImGuiKey_F5;
	m_mapping[ITEM_ID_F6] = ImGuiKey_F6;
	m_mapping[ITEM_ID_F7] = ImGuiKey_F7;
	m_mapping[ITEM_ID_F8] = ImGuiKey_F8;
	m_mapping[ITEM_ID_F9] = ImGuiKey_F9;
	m_mapping[ITEM_ID_F10] = ImGuiKey_F10;
	m_mapping[ITEM_ID_F11] = ImGuiKey_F11;
	m_mapping[ITEM_ID_F12] = ImGuiKey_F12;

	// set key delay and repeat rates
	io.KeyRepeatDelay = 0.400f;
	io.KeyRepeatRate = 0.050f;

	font_name = (downcast<osd_options &>(m_machine->options()).debugger_font());
	font_size = (downcast<osd_options &>(m_machine->options()).debugger_font_size());

	if(font_size == 0)
		font_size = 12;

	io.Fonts->Clear();
	if(!strcmp(font_name, OSDOPTVAL_AUTO))
		io.Fonts->AddFontDefault();
	else
		io.Fonts->AddFontFromFileTTF(font_name,font_size);  // for now, font name must be a path to a TTF file
	imguiCreate();
}

void debug_imgui::wait_for_debugger(device_t &device, bool firststop)
{
	uint32_t width = m_machine->render().ui_target().width();
	uint32_t height = m_machine->render().ui_target().height();
	if(firststop && !m_initialised)
	{
		view_main_console = dview_alloc(device.machine(), DVT_CONSOLE);
		view_main_console->title = "MAME Debugger";
		view_main_console->width = 500;
		view_main_console->height = 200;
		view_main_console->ofs_x = 0;
		view_main_console->ofs_y = 0;
		view_main_console->scroll_follow = true;
		view_main_disasm = dview_alloc(device.machine(), DVT_DISASSEMBLY);
		view_main_disasm->title = "Main Disassembly";
		view_main_disasm->width = 500;
		view_main_disasm->height = 200;
		view_main_regs = dview_alloc(device.machine(), DVT_STATE);
		view_main_regs->title = "Main State";
		view_main_regs->width = 180;
		view_main_regs->height = 440;
		strcpy(view_main_console->console_input,"");  // clear console input
		m_initialised = true;
	}
	if(firststop)
	{
		//debug_show_all();
		m_running = false;
	}
	if(!m_take_ui)
	{
		if (!m_machine->ui().set_ui_event_handler([this] () { return m_take_ui; }))
		{
			// can't break if we can't take over UI input
			m_machine->debugger().console().get_visible_cpu()->debug()->go();
			m_running = true;
			return;
		}
		m_take_ui = true;

	}
	m_hide = false;
	m_machine->osd().input_update(false);
	handle_events();
	handle_console(m_machine);
	update_cpu_view(&device);
	imguiBeginFrame(m_mouse_x, m_mouse_y, m_mouse_button ? IMGUI_MBUT_LEFT : 0, 0, width, height,m_key_char);
	handle_mouse_views();
	handle_keys_views();
	update();
	imguiEndFrame();
	device.machine().osd().update(false);
	osd_sleep(osd_ticks_per_second() / 1000 * 50);
}


void debug_imgui::debugger_update()
{
	if(!view_main_disasm || !view_main_regs || !view_main_console || !m_machine || (m_machine->phase() != machine_phase::RUNNING))
		return;

	if(!m_machine->debugger().cpu().is_stopped())
	{
		if(m_take_ui)
		{
			m_take_ui = false;
			m_current_pointer = -1;
			m_prev_mouse_button = m_mouse_button;
			if(m_mouse_button)
			{
				m_mouse_button = false;
				ImGuiIO& io = ImGui::GetIO();
				io.MouseDown[0] = false;
			}
		}
		if(!m_hide)
		{
			uint32_t width = m_machine->render().ui_target().width();
			uint32_t height = m_machine->render().ui_target().height();
			imguiBeginFrame(m_mouse_x, m_mouse_y, 0, 0, width, height, m_key_char);
			update();
			imguiEndFrame();
		}
	}
}

} // anonymous namespace

} // namespace osd

MODULE_DEFINITION(DEBUG_IMGUI, osd::debug_imgui)
