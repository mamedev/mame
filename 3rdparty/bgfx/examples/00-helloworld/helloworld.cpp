/*
 * Copyright 2011-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <bx/uint32_t.h>
#include "common.h"
#include "bgfx_utils.h"
#include "logo.h"
#include "imgui/imgui.h"

class ExampleHelloWorld : public entry::AppI
{
	void init(int _argc, char** _argv) BX_OVERRIDE
	{
		Args args(_argc, _argv);

		m_width  = 1280;
		m_height = 720;
		m_debug  = BGFX_DEBUG_TEXT;
		m_reset  = BGFX_RESET_VSYNC;

		bgfx::init(args.m_type, args.m_pciId);
		bgfx::reset(m_width, m_height, m_reset);

		// Enable debug text.
		bgfx::setDebug(m_debug);

		// Set view 0 clear state.
		bgfx::setViewClear(0
				, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
				, 0x000000ff
				, 1.0f
				, 0
				);
		imguiCreate();
		ImGui::GetIO().FontGlobalScale = 1.5;
	}

	virtual int shutdown() BX_OVERRIDE
	{
		// Cleanup.
		imguiDestroy();

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	void displayMainMenu()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Left"))
			{
				if (ImGui::MenuItem("Brief", "CTRL+1")) {}
				if (ImGui::MenuItem("Medium", "CTRL+2")) {}
				if (ImGui::MenuItem("Two columns", "CTRL+3")) {}
				if (ImGui::MenuItem("Full (name)", "CTRL+4")) {}
				if (ImGui::MenuItem("Full (size, time)", "CTRL+5")) {}
				if (ImGui::MenuItem("Full (access)", "CTRL+6")) {}
				ImGui::Separator();
				if (ImGui::BeginMenu("Sort mode"))
				{
					ImGui::MenuItem("Name");
					ImGui::MenuItem("Extension");
					ImGui::MenuItem("Modif. Time");
					ImGui::MenuItem("Size");
					ImGui::MenuItem("Unsorted");
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Change source")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Files"))
			{
				if (ImGui::MenuItem("User menu", "F2")) {}
				if (ImGui::MenuItem("View", "F3")) {}
				if (ImGui::MenuItem("Edit", "F4")) {}
				if (ImGui::MenuItem("Copy", "F5")) {}
				if (ImGui::MenuItem("Rename or move", "F6")) {}
				if (ImGui::MenuItem("Make directory", "F7")) {}
				if (ImGui::MenuItem("Delete", "F8")) {}
				ImGui::Separator();
				if (ImGui::MenuItem("File attributes", "CTRL+A")) {}
				if (ImGui::MenuItem("Apply command", "CTRL+G")) {}
				ImGui::Separator();
				if (ImGui::MenuItem("Select group")) {}
				if (ImGui::MenuItem("Unselect group")) {}
				if (ImGui::MenuItem("Invert selection")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Commands"))
			{
				if (ImGui::MenuItem("Find file", "ALT+F7")) {}
				if (ImGui::MenuItem("History", "ALT+F8")) {}
				if (ImGui::MenuItem("Maximize window", "ALT+F9")) {}
				ImGui::Separator();
				if (ImGui::MenuItem("Panel on/off", "CTRL+O")) {}
				if (ImGui::MenuItem("Equal panels", "CTRL+=")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Options"))
			{
				if (ImGui::MenuItem("Settings")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Right"))
			{
				if (ImGui::MenuItem("Brief", "CTRL+1")) {}
				if (ImGui::MenuItem("Medium", "CTRL+2")) {}
				if (ImGui::MenuItem("Two columns", "CTRL+3")) {}
				if (ImGui::MenuItem("Full (name)", "CTRL+4")) {}
				if (ImGui::MenuItem("Full (size, time)", "CTRL+5")) {}
				if (ImGui::MenuItem("Full (access)", "CTRL+6")) {}
				ImGui::Separator();
				if (ImGui::BeginMenu("Sort mode"))
				{
					ImGui::MenuItem("Name");
					ImGui::MenuItem("Extension");
					ImGui::MenuItem("Modif. Time");
					ImGui::MenuItem("Size");
					ImGui::MenuItem("Unsorted");
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Change source")) {}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	bool update() BX_OVERRIDE
	{
		if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
		{
			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, m_width, m_height);

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::touch(0);

			imguiBeginFrame(m_mouseState.m_mx
				, m_mouseState.m_my
				, (m_mouseState.m_buttons[entry::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0)
				, m_mouseState.m_mz
				, m_width
				, m_height
				);
			displayMainMenu();
			ImGui::SetNextWindowPos(ImVec2(0, 32));
			ImGui::SetNextWindowSize(ImVec2(m_width/2, m_height - 32));
			if (ImGui::Begin("Window1", nullptr, ImVec2(m_width/2, m_height-32), 1.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
				ImGui::BeginChild("Sub1", ImVec2(0, m_height - 48), true);


				ImGui::Columns(4, "mycolumns");
				ImGui::Separator();
				ImGui::Text("ID"); ImGui::NextColumn();
				ImGui::Text("Name"); ImGui::NextColumn();
				ImGui::Text("Path"); ImGui::NextColumn();
				ImGui::Text("Flags"); ImGui::NextColumn();
				ImGui::Separator();
				const char* names[3] = { "One", "Two", "Three" };
				const char* paths[3] = { "/path/one", "/path/two", "/path/three" };
				static int selected = -1;
				for (int i = 0; i < 50; i++)
				{
					char label[32];
					sprintf(label, "%04d", i);
					if (ImGui::Selectable(label, selected == i, ImGuiSelectableFlags_SpanAllColumns))
						selected = i;
					ImGui::NextColumn();
					ImGui::Text(names[i%3]); ImGui::NextColumn();
					ImGui::Text(paths[i % 3]); ImGui::NextColumn();
					ImGui::Text("...."); ImGui::NextColumn();
				}
				ImGui::Columns(1);
				ImGui::Separator();



				ImGui::EndChild();
				ImGui::PopStyleVar();
			}
			ImGui::End();
			ImGui::SameLine();
			ImGui::SetNextWindowPos(ImVec2(m_width / 2, 32));
			ImGui::SetNextWindowSize(ImVec2(m_width / 2, m_height - 32));
			if (ImGui::Begin("Window2", nullptr, ImVec2(m_width/2, m_height - 32), 1.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
				ImGui::BeginChild("Sub2", ImVec2(0, m_height - 48), true);

				ImGui::Columns(4, "mycolumns");
				ImGui::Separator();
				ImGui::Text("ID"); ImGui::NextColumn();
				ImGui::Text("Name"); ImGui::NextColumn();
				ImGui::Text("Path"); ImGui::NextColumn();
				ImGui::Text("Flags"); ImGui::NextColumn();
				ImGui::Separator();
				const char* names[3] = { "One", "Two", "Three" };
				const char* paths[3] = { "/path/one", "/path/two", "/path/three" };
				static int selected = -1;
				for (int i = 0; i < 3; i++)
				{
					char label[32];
					sprintf(label, "%04d", i);
					if (ImGui::Selectable(label, selected == i, ImGuiSelectableFlags_SpanAllColumns))
						selected = i;
					ImGui::NextColumn();
					ImGui::Text(names[i]); ImGui::NextColumn();
					ImGui::Text(paths[i]); ImGui::NextColumn();
					ImGui::Text("...."); ImGui::NextColumn();
				}
				ImGui::Columns(1);
				ImGui::Separator();


				ImGui::EndChild();
				ImGui::PopStyleVar();
			}
			ImGui::End();
			imguiEndFrame();
			// Advance to next frame. Rendering thread will be kicked to
			// process submitted rendering primitives.
			bgfx::frame();

			return true;
		}

		return false;
	}

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
	entry::MouseState m_mouseState;
};

ENTRY_IMPLEMENT_MAIN(ExampleHelloWorld);
