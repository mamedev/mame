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

static const char* info_rcss =
"body\n"
"{\n"
"	width: 250px;\n"
"	min-width: 250px;\n"
"	min-height: 150px;\n"
"	margin-top: 42px;\n"
"	margin-right: 20px;\n"
"	margin-left: auto;\n"
"}\n"
"div#content\n"
"{\n"
"   height: auto;\n"
"	max-height: 500px;\n"
"}\n"
"div#content div div\n"
"{\n"
"	font-size: 10;\n"
"}\n"
"div#ancestors p:hover,\n"
"div#children p:hover\n"
"{\n"
"	background-color: #ddd;\n"
"}\n"
"scrollbarvertical\n"
"{\n"
"	scrollbar-margin: 0px;\n"
"}\n";

static const char* info_rml =
"<h1>\n"
"	<handle id=\"position_handle\" move_target=\"#document\">\n"
"		<div id=\"close_button\">X</div>\n"
"		<div id=\"title-content\" style=\"width: 200px;\">Element Information</div>\n"
"	</handle>\n"
"</h1>\n"
"<div id=\"content\">\n"
"	<div id =\"attributes\">\n"
"		<h2>Attributes</h2>\n"
"		<div id=\"attributes-content\">\n"
"		</div>\n"
"	</div>\n"
"	<div id =\"properties\">\n"
"		<h2>Properties</h2>\n"
"		<div id=\"properties-content\">\n"
"		</div>\n"
"	</div>\n"
"	<div id =\"position\">\n"
"		<h2>Position</h2>\n"
"		<div id=\"position-content\">\n"
"		</div>\n"
"	</div>\n"
"	<div id =\"ancestors\">\n"
"		<h2>Ancestors</h2>\n"
"		<div id=\"ancestors-content\">\n"
"		</div>\n"
"	</div>\n"
"	<div id =\"children\">\n"
"		<h2>Children</h2>\n"
"		<div id=\"children-content\">\n"
"		</div>\n"
"	</div>\n"
"</div>\n";
