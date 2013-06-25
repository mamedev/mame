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

#include "precompiled.h"
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/StreamMemory.h>
#include <Rocket/Core.h>
#include "DocumentHeader.h"
#include "ElementStyle.h"
#include "EventDispatcher.h"
#include "LayoutEngine.h"
#include "StreamFile.h"
#include "StyleSheetFactory.h"
#include "Template.h"
#include "TemplateCache.h"
#include "XMLParseTools.h"

namespace Rocket {
namespace Core {

ElementDocument::ElementDocument(const String& tag) : Element(tag)
{
	style_sheet = NULL;
	context = NULL;

	modal = false;
	layout_dirty = true;
	lock_layout = 0;

	ForceLocalStackingContext();

	SetProperty(POSITION, "absolute");
}

ElementDocument::~ElementDocument()
{
	if (style_sheet != NULL)
		style_sheet->RemoveReference();
}

void ElementDocument::ProcessHeader(const DocumentHeader* document_header)
{	
	// Store the source address that we came from
	source_url = document_header->source;

	// Construct a new header and copy the template details across
	DocumentHeader header;
	header.MergePaths(header.template_resources, document_header->template_resources, document_header->source);

	// Merge in any templates, note a merge may cause more templates to merge
	for (size_t i = 0; i < header.template_resources.size(); i++)
	{
		Template* merge_template = TemplateCache::LoadTemplate(URL(header.template_resources[i]).GetURL());	

		if (merge_template)
			header.MergeHeader(*merge_template->GetHeader());
		else
			Log::Message(Log::LT_WARNING, "Template %s not found", header.template_resources[i].CString());
	}

	// Merge the document's header last, as it is the most overriding.
	header.MergeHeader(*document_header);

	// Set the title to the document title.
	title = document_header->title;

	// If a style-sheet (or sheets) has been specified for this element, then we load them and set the combined sheet
	// on the element; all of its children will inherit it by default.
	StyleSheet* style_sheet = NULL;
	if (header.rcss_external.size() > 0)
		style_sheet = StyleSheetFactory::GetStyleSheet(header.rcss_external);

	// Combine any inline sheets.
	if (header.rcss_inline.size() > 0)
	{			
		for (size_t i = 0;i < header.rcss_inline.size(); i++)
		{			
			StyleSheet* new_sheet = new StyleSheet();
			StreamMemory* stream = new StreamMemory((const byte*) header.rcss_inline[i].CString(), header.rcss_inline[i].Length());
			stream->SetSourceURL(document_header->source);

			if (new_sheet->LoadStyleSheet(stream))
			{
				if (style_sheet)
				{
					StyleSheet* combined_sheet = style_sheet->CombineStyleSheet(new_sheet);
					style_sheet->RemoveReference();
					new_sheet->RemoveReference();
					style_sheet = combined_sheet;
				}
				else
					style_sheet = new_sheet;
			}
			else
				new_sheet->RemoveReference();

			stream->RemoveReference();
		}		
	}

	// If a style sheet is available, set it on the document and release it.
	if (style_sheet)
	{
		SetStyleSheet(style_sheet);
		style_sheet->RemoveReference();
	}

	// Load external scripts.
	for (size_t i = 0; i < header.scripts_external.size(); i++)
	{
		StreamFile* stream = new StreamFile();
		if (stream->Open(header.scripts_external[i]))
			LoadScript(stream, header.scripts_external[i]);

		stream->RemoveReference();
	}

	// Load internal scripts.
	for (size_t i = 0; i < header.scripts_inline.size(); i++)
	{
		StreamMemory* stream = new StreamMemory((const byte*) header.scripts_inline[i].CString(), header.scripts_inline[i].Length());
		LoadScript(stream, "");
		stream->RemoveReference();
	}

	// Hide this document.
	SetProperty(VISIBILITY, "hidden");
}

ElementDocument* ElementDocument::GetOwnerDocument()
{
	return this;
}

// Returns the document's context.
Context* ElementDocument::GetContext()
{
	return context;
}

// Sets the document's title.
void ElementDocument::SetTitle(const String& _title)
{
	title = _title;
}

const String& ElementDocument::GetTitle() const
{
	return title;
}

const String& ElementDocument::GetSourceURL() const
{
	return source_url;
}

// Sets the style sheet this document, and all of its children, uses.
void ElementDocument::SetStyleSheet(StyleSheet* _style_sheet)
{
	if (style_sheet == _style_sheet)
		return;

	if (style_sheet != NULL)
		style_sheet->RemoveReference();

	style_sheet = _style_sheet;
	if (style_sheet != NULL)
	{
		style_sheet->AddReference();
		style_sheet->BuildNodeIndex();
	}

	GetStyle()->DirtyDefinition();
}

// Returns the document's style sheet.
StyleSheet* ElementDocument::GetStyleSheet() const
{
	return style_sheet;
}

// Brings the document to the front of the document stack.
void ElementDocument::PullToFront()
{
	if (context != NULL)
		context->PullDocumentToFront(this);
}

// Sends the document to the back of the document stack.
void ElementDocument::PushToBack()
{
	if (context != NULL)
		context->PushDocumentToBack(this);
}

void ElementDocument::Show(int focus_flags)
{
	// Store the modal attribute
	modal = (focus_flags & MODAL) > 0;

	// Set to visible and switch focus if necessary
	SetProperty(VISIBILITY, "visible");
	if (focus_flags & FOCUS || focus_flags & MODAL)
	{
		// If no element could be focused, focus the window
		if (!FocusNextTabElement(this, true))
		{
			Focus();
		}
	}

	DispatchEvent("show", Dictionary(), false);
}

void ElementDocument::Hide()
{
	SetProperty(VISIBILITY, "hidden");
	DispatchEvent("hide", Dictionary(), false);
}

// Close this document
void ElementDocument::Close()
{
	if (context != NULL)
		context->UnloadDocument(this);
}

Element* ElementDocument::CreateElement(const String& name)
{
	return Factory::InstanceElement(NULL, name, name, XMLAttributes());
}

// Create a text element.
ElementText* ElementDocument::CreateTextNode(const String& text)
{
	// Create the element.
	Element* element = CreateElement("#text");
	if (!element)
	{
		Log::Message(Log::LT_ERROR, "Failed to create text element, instancer returned NULL.");
		return NULL;
	}

	// Cast up
	ElementText* element_text = dynamic_cast< ElementText* >(element);
	if (!element_text)
	{
		Log::Message(Log::LT_ERROR, "Failed to create text element, instancer didn't return a derivative of ElementText.");
		element->RemoveReference();
		return NULL;
	}
	
	// Set the text
	element_text->SetText(text);

	return element_text;
}

// Is the current document modal
bool ElementDocument::IsModal() const
{
	return modal;
}

// Default load script implementation
void ElementDocument::LoadScript(Stream* ROCKET_UNUSED(stream), const String& ROCKET_UNUSED(source_name))
{
}

// Updates the layout if necessary.
void ElementDocument::_UpdateLayout()
{
	layout_dirty = false;
	lock_layout++;

	Vector2f containing_block(0, 0);
	if (GetParentNode() != NULL)
		containing_block = GetParentNode()->GetBox().GetSize();

	LayoutEngine layout_engine;
	layout_engine.FormatElement(this, containing_block);
	
	lock_layout--;
}

// Updates the position of the document based on the style properties.
void ElementDocument::UpdatePosition()
{
	// We are only positioned relative to our parent, so if we're not parented we may as well bail now.
	if (GetParentNode() == NULL)
		return;

	Vector2f position;
	// Work out our containing block; relative offsets are calculated against it.
	Vector2f containing_block = GetParentNode()->GetBox().GetSize(Box::CONTENT);

	const Property *left = GetLocalProperty(LEFT);
	const Property *right = GetLocalProperty(RIGHT);
	if (left != NULL && left->unit != Property::KEYWORD)
		position.x = ResolveProperty(LEFT, containing_block.x);
	else if (right != NULL && right->unit != Property::KEYWORD)
		position.x = (containing_block.x - GetBox().GetSize(Box::MARGIN).x) - ResolveProperty(RIGHT, containing_block.x);
	else
		position.x = GetBox().GetEdge(Box::MARGIN, Box::LEFT);

	const Property *top = GetLocalProperty(TOP);
	const Property *bottom = GetLocalProperty(BOTTOM);
	if (top != NULL && top->unit != Property::KEYWORD)
		position.y = ResolveProperty(TOP, containing_block.y);
	else if (bottom != NULL && bottom->unit != Property::KEYWORD)
		position.y = (containing_block.y - GetBox().GetSize(Box::MARGIN).y) - ResolveProperty(BOTTOM, containing_block.y);
	else
		position.y = GetBox().GetEdge(Box::MARGIN, Box::TOP);

	SetOffset(position, NULL);
}
	
void ElementDocument::LockLayout(bool lock)
{
	if (lock)
		lock_layout++;
	else
		lock_layout--;
	
	ROCKET_ASSERT(lock_layout >= 0);
}

void ElementDocument::DirtyLayout()
{
	layout_dirty = true;
}

// Refreshes the document layout if required.
void ElementDocument::OnUpdate()
{
	UpdateLayout();
}

// Repositions the document if necessary.
void ElementDocument::OnPropertyChange(const PropertyNameList& changed_properties)
{
	Element::OnPropertyChange(changed_properties);

	if (changed_properties.find(TOP) != changed_properties.end() ||
		changed_properties.find(RIGHT) != changed_properties.end() ||
		changed_properties.find(BOTTOM) != changed_properties.end() ||
		changed_properties.find(LEFT) != changed_properties.end())
		UpdatePosition();
}

// Processes the 'onpropertychange' event, checking for a change in position or size.
void ElementDocument::ProcessEvent(Event& event)
{
	Element::ProcessEvent(event);

	// Process generic keyboard events for this window in capture phase
	if (event.GetPhase() == Event::PHASE_BUBBLE && event == KEYDOWN)
	{
		int key_identifier = event.GetParameter<int>("key_identifier", Input::KI_UNKNOWN);

		// Process TAB
		if (key_identifier == Input::KI_TAB)
		{
			FocusNextTabElement(event.GetTargetElement(), !event.GetParameter<bool>("shift_key", false));
		}
		// Process ENTER being pressed on a focusable object (emulate click)
		else if (key_identifier == Input::KI_RETURN ||
				 key_identifier == Input::KI_NUMPADENTER)
		{
			Element* focus_node = GetFocusLeafNode();

			if (focus_node && focus_node->GetProperty<int>(TAB_INDEX) == TAB_INDEX_AUTO)
			{
				focus_node->Click();
			}
		}
	}
	else if (event.GetTargetElement() == this)
	{
		if (event == RESIZE)
			UpdatePosition();
	}
}

// Find the next element to focus, starting at the current element
//
// This algorithm is quite sneaky, I originally thought a depth first search would
// work, but it appears not. What is required is to cut the tree in half along the nodes
// from current_element up the root and then either traverse the tree in a clockwise or
// anticlock wise direction depending if you're searching forward or backward respectively
bool ElementDocument::FocusNextTabElement(Element* current_element, bool forward)
{
	// If we're searching forward, check the immediate children of this node first off
	if (forward)
	{
		for (int i = 0; i < current_element->GetNumChildren(); i++)
			if (SearchFocusSubtree(current_element->GetChild(i), forward))
				return true;
	}

	// Now walk up the tree, testing either the bottom or top
	// of the tree, depending on whether we're going forwards
	// or backwards respectively
	//
	// If we make it all the way up to the document, then
	// we search the entire tree (to loop back round)
	bool search_enabled = false;
	Element* document = current_element->GetOwnerDocument();
	Element* child = current_element;
	Element* parent = current_element->GetParentNode();
	while (child != document)
	{
		for (int i = 0; i < parent->GetNumChildren(); i++)
		{
			// Calculate index into children
			int child_index = i;
			if (!forward)
				child_index = parent->GetNumChildren() - i - 1;
			Element* search_child = parent->GetChild(child_index);

			// Do a search if its enabled
			if (search_enabled && SearchFocusSubtree(search_child, forward))
				return true;

			// If we find the child, enable searching
			if (search_child == child)
				search_enabled = true;
		}

		// Advance up the tree
		child = parent;
		parent = parent->GetParentNode();

		// If we hit the top, enable searching the entire tree
		if (parent == document)
			search_enabled = true;
		else // otherwise enable searching if we're going backward and disable if we're going forward
			search_enabled = false;
	}

	return false;
}

bool ElementDocument::SearchFocusSubtree(Element* element, bool forward)
{
	// Skip disabled elements
	if (element->IsPseudoClassSet("disabled"))
	{
		return false;
	}

	// Check if this is the node we're looking for
	if (element->GetProperty<int>(TAB_INDEX) == TAB_INDEX_AUTO)
	{
		element->Focus();
		return true;
	}

	// Check all children
	for (int i = 0; i < element->GetNumChildren(); i++)
	{
		int child_index = i;
		if (!forward)
			child_index = element->GetNumChildren() - i - 1;
		if (SearchFocusSubtree(element->GetChild(child_index), forward))
			return true;
	}

	return false;
}

}
}
