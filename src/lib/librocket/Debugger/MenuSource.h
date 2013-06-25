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

static const char* menu_rcss =
"body\n"
"{\n"
"	width: 100%;\n"
"	height: 32px;\n"
"	position: absolute;\n"
"	z-index: top;\n"
"	background: #888;\n"
"	font-family: Lacuna;\n"
"	font-size: 14px;\n"
"	color: black;\n"
"}\n"
"div\n"
"{\n"
"	display: block;\n"
"}\n"
"div#button-group\n"
"{\n"
"	margin-top: 4px;\n"
"}\n"
"button\n"
"{\n"
"	border-width: 1px;\n"
"	border-color: #666;\n"
"	background: #ddd;\n"
"	margin-left: 6px;\n"
"	display: inline-block;\n"
"	width: 100px;\n"
"	text-align: center;\n"
"}\n"
"button:hover\n"
"{\n"
"	background: #eee;\n"
"}\n"
"div#version-info\n"
"{\n"
"	padding: 0px;\n"
"	margin-top: 0px;\n"
"	font-size: 20px;\n"
"	float: right;\n"
"	margin-right: 20px;\n"
"	width: 200px;"
"	text-align: right;"
"	color: white;\n"
"}\n"
"span#version-number\n"
"{\n"
"	font-size: 15px;\n"
"}\n"
;

static const char* menu_rml =
"<div id=\"version-info\">libRocket <span id=\"version-number\"></span></div>\n"
"<div id =\"button-group\">\n"
"	<button id =\"event-log-button\">Event Log</button>\n"
"	<button id =\"debug-info-button\">Element Info</button>\n"
"	<button id =\"outlines-button\">Outlines</button>\n"
"</div>\n";
