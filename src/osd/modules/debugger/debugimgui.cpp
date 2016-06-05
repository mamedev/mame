// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// ImGui based debugger

#include "emu.h"
#include "imgui/imgui.h"
#include "render.h"
#include "uiinput.h"

#include "debug/debugvw.h"
#include "debug/dvdisasm.h"
#include "debug/dvmemory.h"
#include "debug/dvbpoints.h"
#include "debug/dvwpoints.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"

#include "config.h"
#include "debugger.h"
#include "modules/lib/osdobj_common.h"
#include "debug_module.h"
#include "modules/osdmodule.h"

class debug_area
{
	DISABLE_COPYING(debug_area);

public:
	debug_area(running_machine &machine, debug_view_type type)
		: next(nullptr),
			type(0),
			ofs_x(0),
			ofs_y(0),
			is_collapsed(false),
			exec_cmd(false)
		{
		this->view = machine.debug_view().alloc_view(type, nullptr, this);
		this->type = type;
		this->m_machine = &machine;
		this->width = 300;
		this->height = 300;

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

	debug_area *             next;

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
	char                console_input[512];
};

class debug_imgui : public osd_module, public debug_module
{
public:
	debug_imgui()
	: osd_module(OSD_DEBUG_PROVIDER, "imgui"), debug_module(),
		m_machine(nullptr),
		m_mouse_x(0),
		m_mouse_y(0),
		m_mouse_button(false),
		m_prev_mouse_button(false),
		m_running(false),
		font_name(nullptr),
		font_size(0),
		m_key_char(0),
		m_hide(false),
		m_win_count(0)
	{
	}

	virtual ~debug_imgui() { }

	virtual int init(const osd_options &options) override { return 0; }
	virtual void exit() override {};

	virtual void init_debugger(running_machine &machine) override;
	virtual void wait_for_debugger(device_t &device, bool firststop) override;
	virtual void debugger_update() override;

private:
	void handle_mouse();
	void handle_mouse_views();
	void handle_keys();
	void handle_keys_views();
	void handle_console(running_machine* machine);
	void update();
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
	void update_cpu_view(device_t* device);
	static bool get_view_source(void* data, int idx, const char** out_text);

	running_machine* m_machine;
	INT32            m_mouse_x;
	INT32            m_mouse_y;
	bool             m_mouse_button;
	bool             m_prev_mouse_button;
	bool             m_running;
	const char*      font_name;
	float            font_size;
	ImVec2           m_text_size;  // size of character (assumes monospaced font is in use)
	ImguiFontHandle  m_font;
	UINT8            m_key_char;
	bool             m_hide;
	int              m_win_count;  // number of active windows, does not decrease, used to ID individual windows
};

// globals
static std::vector<debug_area*> view_list;
static debug_area* view_main_console = nullptr;
static debug_area* view_main_disasm = nullptr;
static debug_area* view_main_regs = nullptr;

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
	debug_area *dv;

	dv = global_alloc(debug_area(machine, type));

	return dv;
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

bool debug_imgui::get_view_source(void* data, int idx, const char** out_text)
{
	debug_view* vw = static_cast<debug_view*>(data);
	*out_text = vw->source_list().find(idx)->name();
	return true;
}

void debug_imgui::handle_mouse()
{
	m_prev_mouse_button = m_mouse_button;
	m_machine->ui_input().find_mouse(&m_mouse_x, &m_mouse_y, &m_mouse_button);
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
	for(std::vector<debug_area*>::iterator it = view_list.begin();it != view_list.end();++it)
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

void debug_imgui::handle_keys()
{
	ImGuiIO& io = ImGui::GetIO();
	ui_event event;

	// global keys
	if(m_machine->input().code_pressed_once(KEYCODE_F3))
	{
		if(m_machine->input().code_pressed(KEYCODE_LSHIFT))
			m_machine->schedule_hard_reset();
		else
		{
			m_machine->schedule_soft_reset();
			debug_cpu_get_visible_cpu(*m_machine)->debug()->go();
		}
	}

	if(m_machine->input().code_pressed_once(KEYCODE_F5))
	{
		debug_cpu_get_visible_cpu(*m_machine)->debug()->go();
		m_running = true;
	}
	if(m_machine->input().code_pressed_once(KEYCODE_F6))
	{
		debug_cpu_get_visible_cpu(*m_machine)->debug()->go_next_device();
		m_running = true;
	}
	if(m_machine->input().code_pressed_once(KEYCODE_F7))
	{
		debug_cpu_get_visible_cpu(*m_machine)->debug()->go_interrupt();
		m_running = true;
	}
	if(m_machine->input().code_pressed_once(KEYCODE_F8))
		debug_cpu_get_visible_cpu(*m_machine)->debug()->go_vblank();
	if(m_machine->input().code_pressed_once(KEYCODE_F9))
		debug_cpu_get_visible_cpu(*m_machine)->debug()->single_step_out();
	if(m_machine->input().code_pressed_once(KEYCODE_F10))
		debug_cpu_get_visible_cpu(*m_machine)->debug()->single_step_over();
	if(m_machine->input().code_pressed_once(KEYCODE_F11))
		debug_cpu_get_visible_cpu(*m_machine)->debug()->single_step();
	if(m_machine->input().code_pressed_once(KEYCODE_F12))
	{
		debug_cpu_get_visible_cpu(*m_machine)->debug()->go();
		m_hide = true;
	}

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
			io.KeysDown[id] = true;
		else
			io.KeysDown[id] = false;
	}

	m_key_char = 0;
	while (m_machine->ui_input().pop_event(&event))
	{
		switch (event.event_type)
		{
		case UI_EVENT_CHAR:
			m_key_char = event.ch;
			return;
		default:
			break;
		}
	}

	if(ImGui::IsKeyPressed(ITEM_ID_D) && ImGui::IsKeyDown(ITEM_ID_LCONTROL))
		add_disasm(++m_win_count);
	if(ImGui::IsKeyPressed(ITEM_ID_M) && ImGui::IsKeyDown(ITEM_ID_LCONTROL))
		add_memory(++m_win_count);
	if(ImGui::IsKeyPressed(ITEM_ID_B) && ImGui::IsKeyDown(ITEM_ID_LCONTROL))
		add_bpoints(++m_win_count);
	if(ImGui::IsKeyPressed(ITEM_ID_W) && ImGui::IsKeyDown(ITEM_ID_LCONTROL))
		add_wpoints(++m_win_count);
	if(ImGui::IsKeyPressed(ITEM_ID_L) && ImGui::IsKeyDown(ITEM_ID_LCONTROL))
		add_log(++m_win_count);

}

void debug_imgui::handle_keys_views()
{
	debug_area* focus_view = nullptr;
	// find view that has focus (should only be one at a time)
	for(std::vector<debug_area*>::iterator view_ptr = view_list.begin();view_ptr != view_list.end();++view_ptr)
		if((*view_ptr)->has_focus)
			focus_view = *view_ptr;

	// check views in main views also (only the disassembler view accepts inputs)
	if(view_main_disasm->has_focus)
		focus_view = view_main_disasm;

	// if no view has focus, then there's nothing to do
	if(focus_view == nullptr)
		return;

	// pass keypresses to debug view with focus
	if(ImGui::IsKeyPressed(ITEM_ID_UP))
		focus_view->view->process_char(DCH_UP);
	if(ImGui::IsKeyPressed(ITEM_ID_DOWN))
		focus_view->view->process_char(DCH_DOWN);
	if(ImGui::IsKeyPressed(ITEM_ID_LEFT))
	{
		if(ImGui::IsKeyDown(ITEM_ID_LCONTROL))
			focus_view->view->process_char(DCH_CTRLLEFT);
		else
			focus_view->view->process_char(DCH_LEFT);
	}
	if(ImGui::IsKeyPressed(ITEM_ID_RIGHT))
	{
		if(ImGui::IsKeyDown(ITEM_ID_LCONTROL))
			focus_view->view->process_char(DCH_CTRLRIGHT);
		else
			focus_view->view->process_char(DCH_RIGHT);
	}
	if(ImGui::IsKeyPressed(ITEM_ID_PGUP))
		focus_view->view->process_char(DCH_PUP);
	if(ImGui::IsKeyPressed(ITEM_ID_PGDN))
		focus_view->view->process_char(DCH_PDOWN);
	if(ImGui::IsKeyPressed(ITEM_ID_HOME))
	{
		if(ImGui::IsKeyDown(ITEM_ID_LCONTROL))
			focus_view->view->process_char(DCH_CTRLHOME);
		else
			focus_view->view->process_char(DCH_HOME);
	}
	if(ImGui::IsKeyPressed(ITEM_ID_END))
	{
		if(ImGui::IsKeyDown(ITEM_ID_LCONTROL))
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
			debug_cpu_get_visible_cpu(*m_machine)->debug()->single_step();
			view_main_console->exec_cmd = false;
			return;
		}
		debug_console_execute_command(*m_machine, view_main_console->console_input, 1);
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
		strcpy(view_main_console->console_input,"");
		view_main_console->exec_cmd = false;
	}
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
		ImGui::SetCursorPosY((totalsize.y) * fsize.y);

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
			ImU32 fg_col = ImGui::ColorConvertFloat4ToU32(ImVec4(fg.r()/255.0f,fg.g()/255.0f,fg.b()/255.0f,fg.a()/255.0f));
			str[0] = v = viewdata->byte;
			str[1] = '\0';
			if(bg != base)
			{
				ImU32 bg_col = ImGui::ColorConvertFloat4ToU32(ImVec4(bg.r()/255.0f,bg.g()/255.0f,bg.b()/255.0f,bg.a()/255.0f));
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
	ImGui::PopStyleVar(2);
}

void debug_imgui::draw_bpoints(debug_area* view_ptr, bool* opened)
{
	ImGui::SetNextWindowSize(ImVec2(view_ptr->width,view_ptr->height + ImGui::GetTextLineHeight()),ImGuiSetCond_Once);
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
	ImGui::SetNextWindowSize(ImVec2(view_ptr->width,view_ptr->height + ImGui::GetTextLineHeight()),ImGuiSetCond_Once);
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
	view_list_add(new_view);
}

void debug_imgui::draw_disasm(debug_area* view_ptr, bool* opened)
{
	const debug_view_source* src;

	ImGui::SetNextWindowSize(ImVec2(view_ptr->width,view_ptr->height + ImGui::GetTextLineHeight()),ImGuiSetCond_Once);
	if(ImGui::Begin(view_ptr->title.c_str(),opened,ImGuiWindowFlags_MenuBar))
	{
		int idx;
		bool done = false;
		bool exp_change = false;

		view_ptr->is_collapsed = false;
		if(ImGui::BeginMenuBar())
		{
			if(ImGui::BeginMenu("Options"))
			{
				debug_view_disasm* disasm = downcast<debug_view_disasm*>(view_ptr->view);
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
		ImGui::Combo("##cpu",&view_ptr->src_sel,get_view_source,view_ptr->view,view_ptr->view->source_list().count());
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
		src = view_ptr->view->first_source();
		idx = 0;
		while (!done)
		{
			if(view_ptr->src_sel == idx)
				view_ptr->view->set_source(*src);
			idx++;
			src = src->next();
			if(src == nullptr)
				done = true;
		}

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
	const debug_view_source* src;

	ImGui::SetNextWindowSize(ImVec2(view_ptr->width,view_ptr->height + ImGui::GetTextLineHeight()),ImGuiSetCond_Once);
	if(ImGui::Begin(view_ptr->title.c_str(),opened,ImGuiWindowFlags_MenuBar))
	{
		int idx;
		bool done = false;
		bool exp_change = false;

		view_ptr->is_collapsed = false;
		if(ImGui::BeginMenuBar())
		{
			if(ImGui::BeginMenu("Options"))
			{
				debug_view_memory* mem = downcast<debug_view_memory*>(view_ptr->view);
				bool physical = mem->physical();
				bool rev = mem->reverse();
				int format = mem->get_data_format();
				UINT32 chunks = mem->chunks_per_row();

				if(ImGui::MenuItem("1-byte chunks", nullptr,(format == 1) ? true : false))
					mem->set_data_format(1);
				if(ImGui::MenuItem("2-byte chunks", nullptr,(format == 2) ? true : false))
					mem->set_data_format(2);
				if(ImGui::MenuItem("4-byte chunks", nullptr,(format == 4) ? true : false))
					mem->set_data_format(4);
				if(ImGui::MenuItem("8-byte chunks", nullptr,(format == 8) ? true : false))
					mem->set_data_format(8);
				if(ImGui::MenuItem("32-bit floating point", nullptr,(format == 9) ? true : false))
					mem->set_data_format(9);
				if(ImGui::MenuItem("64-bit floating point", nullptr,(format == 10) ? true : false))
					mem->set_data_format(10);
				if(ImGui::MenuItem("80-bit floating point", nullptr,(format == 11) ? true : false))
					mem->set_data_format(11);
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
		ImGui::Combo("##region",&view_ptr->src_sel,get_view_source,view_ptr->view,view_ptr->view->source_list().count());
		ImGui::PopItemWidth();
		ImGui::Separator();

		// memory editor portion
		src = view_ptr->view->first_source();
		idx = 0;
		while (!done)
		{
			if(view_ptr->src_sel == idx)
				view_ptr->view->set_source(*src);
			idx++;
			src = src->next();
			if(src == nullptr)
				done = true;
		}

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

void debug_imgui::draw_console()
{
	rgb_t bg, fg;
	rgb_t base(0xe6, 0xff, 0xff, 0xff);
	ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	ImGui::SetNextWindowSize(ImVec2(view_main_regs->width + view_main_disasm->width,view_main_disasm->height + view_main_console->height + ImGui::GetTextLineHeight()*3),ImGuiSetCond_Once);
	if(ImGui::Begin(view_main_console->title.c_str(), nullptr,flags))
	{
		std::string str;

		if(ImGui::BeginMenuBar())
		{
			if(ImGui::BeginMenu("Debug"))
			{
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
					debug_cpu_get_visible_cpu(*m_machine)->debug()->go();
					m_running = true;
				}
				if(ImGui::MenuItem("Go to next CPU", "F6"))
				{
					debug_cpu_get_visible_cpu(*m_machine)->debug()->go_next_device();
					m_running = true;
				}
				if(ImGui::MenuItem("Run until next interrupt", "F7"))
				{
					debug_cpu_get_visible_cpu(*m_machine)->debug()->go_interrupt();
					m_running = true;
				}
				if(ImGui::MenuItem("Run until VBLANK", "F8"))
					debug_cpu_get_visible_cpu(*m_machine)->debug()->go_vblank();
				if(ImGui::MenuItem("Run and hide debugger", "F12"))
				{
					debug_cpu_get_visible_cpu(*m_machine)->debug()->go();
					m_hide = true;
				}
				ImGui::Separator();
				if(ImGui::MenuItem("Single step", "F11"))
					debug_cpu_get_visible_cpu(*m_machine)->debug()->single_step();
				if(ImGui::MenuItem("Step over", "F10"))
					debug_cpu_get_visible_cpu(*m_machine)->debug()->single_step_over();
				if(ImGui::MenuItem("Step out", "F9"))
					debug_cpu_get_visible_cpu(*m_machine)->debug()->single_step_out();

				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Window"))
			{
				if(ImGui::MenuItem("Show all"))
				{
					for(std::vector<debug_area*>::iterator view_ptr = view_list.begin();view_ptr != view_list.end();++view_ptr)
						ImGui::SetWindowCollapsed((*view_ptr)->title.c_str(),false);
				}
				ImGui::Separator();
				// list all extra windows, so we can un-collapse the windows if necessary
				for(std::vector<debug_area*>::iterator view_ptr = view_list.begin();view_ptr != view_list.end();++view_ptr)
				{
					bool collapsed = false;
					if((*view_ptr)->is_collapsed)
						collapsed = true;
					if(ImGui::MenuItem((*view_ptr)->title.c_str(), nullptr,!collapsed))
						ImGui::SetWindowCollapsed((*view_ptr)->title.c_str(),false);
				}
				ImGui::EndMenu();
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
		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
		if(m_running)
			flags |= ImGuiInputTextFlags_ReadOnly;
		ImGui::PushItemWidth(-1.0f);
		if(ImGui::InputText("##console_input",view_main_console->console_input,512,flags))
			view_main_console->exec_cmd = true;
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
	int count = 0;
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
		count++;
	}
	// check for a closed window
	if(to_delete != nullptr)
	{
		view_list_remove(to_delete);
		global_free(to_delete);
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

	io.KeyMap[ImGuiKey_A] = ITEM_ID_A;
	io.KeyMap[ImGuiKey_C] = ITEM_ID_C;
	io.KeyMap[ImGuiKey_V] = ITEM_ID_V;
	io.KeyMap[ImGuiKey_X] = ITEM_ID_X;
	io.KeyMap[ImGuiKey_C] = ITEM_ID_C;
	io.KeyMap[ImGuiKey_Y] = ITEM_ID_Y;
	io.KeyMap[ImGuiKey_Z] = ITEM_ID_Z;
	io.KeyMap[ImGuiKey_Backspace] = ITEM_ID_BACKSPACE;
	io.KeyMap[ImGuiKey_Delete] = ITEM_ID_DEL;
	io.KeyMap[ImGuiKey_Tab] = ITEM_ID_TAB;
	io.KeyMap[ImGuiKey_PageUp] = ITEM_ID_PGUP;
	io.KeyMap[ImGuiKey_PageDown] = ITEM_ID_PGDN;
	io.KeyMap[ImGuiKey_Home] = ITEM_ID_HOME;
	io.KeyMap[ImGuiKey_End] = ITEM_ID_END;
	io.KeyMap[ImGuiKey_Escape] = ITEM_ID_ESC;
	io.KeyMap[ImGuiKey_Enter] = ITEM_ID_ENTER;
	io.KeyMap[ImGuiKey_LeftArrow] = ITEM_ID_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = ITEM_ID_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = ITEM_ID_UP;
	io.KeyMap[ImGuiKey_DownArrow] = ITEM_ID_DOWN;

	font_name = (downcast<osd_options &>(m_machine->options()).debugger_font());
	font_size = (downcast<osd_options &>(m_machine->options()).debugger_font_size());

	if(font_size == 0)
		font_size = 12;

	io.Fonts->Clear();
	if(!strcmp(font_name, OSDOPTVAL_AUTO))
		io.Fonts->AddFontDefault();
	else
		io.Fonts->AddFontFromFileTTF(font_name,font_size);  // for now, font name must be a path to a TTF file
	m_font = imguiCreate();
	imguiSetFont(m_font);
}

void debug_imgui::wait_for_debugger(device_t &device, bool firststop)
{
	UINT32 width = m_machine->render().ui_target().width();
	UINT32 height = m_machine->render().ui_target().height();
	if(firststop && view_list.empty())
	{
		view_main_console = dview_alloc(device.machine(), DVT_CONSOLE);
		view_main_console->title = "MAME Debugger";
		view_main_console->width = 500;
		view_main_console->height = 200;
		view_main_console->ofs_x = 0;
		view_main_console->ofs_y = 0;
		view_main_disasm = dview_alloc(device.machine(), DVT_DISASSEMBLY);
		view_main_disasm->title = "Main Disassembly";
		view_main_disasm->width = 500;
		view_main_disasm->height = 200;
		view_main_regs = dview_alloc(device.machine(), DVT_STATE);
		view_main_regs->title = "Main State";
		view_main_regs->width = 180;
		view_main_regs->height = 440;
		strcpy(view_main_console->console_input,"");  // clear console input
	}
	if(firststop)
	{
//      debug_show_all();
		device.machine().ui_input().reset();
		m_running = false;
	}
	m_hide = false;
//  m_machine->ui_input().frame_update();
	handle_mouse();
	handle_keys();
	handle_console(m_machine);
	update_cpu_view(&device);
	imguiBeginFrame(m_mouse_x,m_mouse_y,m_mouse_button ? IMGUI_MBUT_LEFT : 0, 0, width, height,m_key_char);
	update();
	imguiEndFrame();
	handle_mouse_views();
	handle_keys_views();
	m_machine->ui_input().reset();  // clear remaining inputs, so they don't fall through to the UI
	device.machine().osd().update(false);
}


void debug_imgui::debugger_update()
{
	if ((m_machine != nullptr) && (!debug_cpu_is_stopped(*m_machine)) && (m_machine->phase() == MACHINE_PHASE_RUNNING) && !m_hide)
	{
		UINT32 width = m_machine->render().ui_target().width();
		UINT32 height = m_machine->render().ui_target().height();
		handle_mouse();
		handle_keys();
		imguiBeginFrame(m_mouse_x,m_mouse_y,m_mouse_button ? IMGUI_MBUT_LEFT : 0, 0, width, height, m_key_char);
		update();
		imguiEndFrame();
	}
}

MODULE_DEFINITION(DEBUG_IMGUI, debug_imgui)
