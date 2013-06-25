/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "ElementLog.h"
#include <Rocket/Core.h>
#include "CommonSource.h"
#include "BeaconSource.h"
#include "LogSource.h"

namespace Rocket {
namespace Debugger {

const int MAX_LOG_MESSAGES = 50;

ElementLog::ElementLog(const Core::String& tag) : Core::ElementDocument(tag)
{
	dirty_logs = false;
	beacon = NULL;
	current_beacon_level = Core::Log::LT_MAX;
	auto_scroll = true;
	message_content = NULL;
	current_index = 0;

	// Set up the log type buttons.
	log_types[Core::Log::LT_ALWAYS].visible = true;
	log_types[Core::Log::LT_ALWAYS].class_name = "error";
	log_types[Core::Log::LT_ALWAYS].alert_contents = "A";

	log_types[Core::Log::LT_ERROR].visible = true;
	log_types[Core::Log::LT_ERROR].class_name = "error";
	log_types[Core::Log::LT_ERROR].alert_contents = "!";
	log_types[Core::Log::LT_ERROR].button_name = "error_button";

	log_types[Core::Log::LT_ASSERT].visible = true;
	log_types[Core::Log::LT_ASSERT].class_name = "error";
	log_types[Core::Log::LT_ASSERT].alert_contents = "!";

	log_types[Core::Log::LT_WARNING].visible = true;
	log_types[Core::Log::LT_WARNING].class_name = "warning";
	log_types[Core::Log::LT_WARNING].alert_contents = "!";
	log_types[Core::Log::LT_WARNING].button_name = "warning_button";

	log_types[Core::Log::LT_INFO].visible = false;
	log_types[Core::Log::LT_INFO].class_name = "info";
	log_types[Core::Log::LT_INFO].alert_contents = "i";
	log_types[Core::Log::LT_INFO].button_name = "info_button";

	log_types[Core::Log::LT_DEBUG].visible = true;
	log_types[Core::Log::LT_DEBUG].class_name = "debug";
	log_types[Core::Log::LT_DEBUG].alert_contents = "?";
	log_types[Core::Log::LT_DEBUG].button_name = "debug_button";
}

ElementLog::~ElementLog()
{
}

// Initialises the log element.
bool ElementLog::Initialise()
{
	SetInnerRML(log_rml);
	SetId("rkt-debug-log");

	message_content = GetElementById("content");
	if (message_content)
	{
		message_content->AddEventListener("resize", this);
	}

	Core::StyleSheet* style_sheet = Core::Factory::InstanceStyleSheetString(Core::String(common_rcss) + Core::String(log_rcss));
	if (style_sheet == NULL)
		return false;

	SetStyleSheet(style_sheet);
	style_sheet->RemoveReference();

	// Create the log beacon.
	beacon = GetContext()->CreateDocument();
	if (beacon == NULL)
		return false;

	beacon->SetId("rkt-debug-log-beacon");
	beacon->SetProperty("visibility", "hidden");
	beacon->SetInnerRML(beacon_rml);

	// Remove the initial reference on the beacon.
	beacon->RemoveReference();

	Core::Element* button = beacon->GetFirstChild();
	if (button != NULL)
		beacon->GetFirstChild()->AddEventListener("click", this);

	style_sheet = Core::Factory::InstanceStyleSheetString(Core::String(common_rcss) + Core::String(beacon_rcss));
	if (style_sheet == NULL)
	{
		GetContext()->UnloadDocument(beacon);
		beacon = NULL;

		return false;
	}

	beacon->SetStyleSheet(style_sheet);
	style_sheet->RemoveReference();

	return true;
}

// Adds a log message to the debug log.
void ElementLog::AddLogMessage(Core::Log::Type type, const Core::String& message)
{
	// Add the message to the list of messages for the specified log type.
	LogMessage log_message;
	log_message.index = current_index++;
	log_message.message = Core::String(message).Replace("<", "&lt;").Replace(">", "&gt;");
	log_types[type].log_messages.push_back(log_message);
	if (log_types[type].log_messages.size() >= MAX_LOG_MESSAGES)
	{
		log_types[type].log_messages.pop_front();
	}

	// If this log type is invisible, and there is a button for this log type, then change its text from
	// "Off" to "Off*" to signal that there are unread logs.
	if (!log_types[type].visible)
	{
		if (!log_types[type].button_name.Empty())
		{
			Rocket::Core::Element* button = GetElementById(log_types[type].button_name);
			if (button)
			{
				button->SetInnerRML("Off*");
			}
		}
	}
	// Trigger the beacon if we're hidden. Override any lower-level log type if it is already visible.
	else
	{
		if (!IsVisible())
		{
			if (beacon != NULL)
			{
				if (type < current_beacon_level)
				{
					beacon->SetProperty("visibility", "visible");

					current_beacon_level = type;
					Rocket::Core::Element* beacon_button = beacon->GetFirstChild();
					if (beacon_button)
					{
						beacon_button->SetClassNames(log_types[type].class_name);
						beacon_button->SetInnerRML(log_types[type].alert_contents);
					}
				}
			}
		}
	}

	// Force a refresh of the RML.
	dirty_logs = true;
}

void ElementLog::OnRender()
{
	Core::ElementDocument::OnRender();

	if (dirty_logs)
	{
		// Set the log content:
		Core::String messages;
		if (message_content)
		{
			unsigned int log_pointers[Core::Log::LT_MAX];
			for (int i = 0; i < Core::Log::LT_MAX; i++)
				log_pointers[i] = 0;
			int next_type = FindNextEarliestLogType(log_pointers);
			int num_messages = 0;
			while (next_type != -1 && num_messages < MAX_LOG_MESSAGES)
			{
				messages.Append(Core::String(128, "<div class=\"log-entry\"><div class=\"icon %s\">%s</div><p class=\"message\">", log_types[next_type].class_name.CString(), log_types[next_type].alert_contents.CString()));
				messages.Append(log_types[next_type].log_messages[log_pointers[next_type]].message);
				messages.Append("</p></div>");
				
				log_pointers[next_type]++;
				next_type = FindNextEarliestLogType(log_pointers);
				num_messages++;
			}

			if (message_content->HasChildNodes())
			{
				float last_element_top = message_content->GetLastChild()->GetAbsoluteTop();
				auto_scroll = message_content->GetAbsoluteTop() + message_content->GetAbsoluteTop() > last_element_top;
			}
			else
				auto_scroll = true;

			message_content->SetInnerRML(messages);		

			dirty_logs = false;
		}
	}
}

void ElementLog::ProcessEvent(Core::Event& event)
{
	Core::Element::ProcessEvent(event);

	// Only process events if we're visible
	if (beacon != NULL)
	{
		if (event == "click")
		{
			if (event.GetTargetElement() == beacon->GetFirstChild())
			{
				if (!IsVisible())
					SetProperty("visibility", "visible");

				beacon->SetProperty("visibility", "hidden");
				current_beacon_level = Core::Log::LT_MAX;
			}
			else if (event.GetTargetElement()->GetId() == "close_button")
			{
				if (IsVisible())
					SetProperty("visibility", "hidden");
			}
			else
			{
				for (int i = 0; i < Core::Log::LT_MAX; i++)
				{
					if (!log_types[i].button_name.Empty() && event.GetTargetElement()->GetId() == log_types[i].button_name)
					{
						log_types[i].visible = !log_types[i].visible;
						if (log_types[i].visible)
							event.GetTargetElement()->SetInnerRML("On");
						else
							event.GetTargetElement()->SetInnerRML("Off");
						dirty_logs = true;
					}
				}
			}
		}
	}

	if (event == "resize" && auto_scroll)
	{
		if (message_content != NULL &&
			message_content->HasChildNodes())
			message_content->GetLastChild()->ScrollIntoView();
	}
}

int ElementLog::FindNextEarliestLogType(unsigned int log_pointers[Core::Log::LT_MAX])
{
	int log_channel = -1;
	unsigned int index = UINT_MAX;

	for (int i = 0; i < Core::Log::LT_MAX; i++)
	{
		if (log_types[i].visible)
		{
			if (log_pointers[i] < log_types[i].log_messages.size())
			{
				if (log_types[i].log_messages[log_pointers[i]].index < index)
				{
					index = log_types[i].log_messages[log_pointers[i]].index;
					log_channel = i;
				}
			}
		}
	}

	return log_channel;
}

}
}
