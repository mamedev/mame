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

#ifndef ROCKETCORESTYLESHEETKEYWORDS_H
#define ROCKETCORESTYLESHEETKEYWORDS_H

namespace Rocket {
namespace Core {

const int POSITION_STATIC = 0;
const int POSITION_RELATIVE = 1;
const int POSITION_ABSOLUTE = 2;
const int POSITION_FIXED = 3;

const int FLOAT_NONE = 0;
const int FLOAT_LEFT = 1;
const int FLOAT_RIGHT = 2;

const int CLEAR_NONE = 0;
const int CLEAR_LEFT = 1;
const int CLEAR_RIGHT = 2;
const int CLEAR_BOTH = 3;

const int DISPLAY_NONE = 0;
const int DISPLAY_BLOCK = 1;
const int DISPLAY_INLINE = 2;
const int DISPLAY_INLINE_BLOCK = 3;

const int VISIBILITY_VISIBLE = 0;
const int VISIBILITY_HIDDEN = 1;

const int OVERFLOW_VISIBLE = 0;
const int OVERFLOW_HIDDEN = 1;
const int OVERFLOW_AUTO = 2;
const int OVERFLOW_SCROLL = 3;

const int CLIP_AUTO = 0;
const int CLIP_NONE = 1;

const int FONT_STYLE_NORMAL = 0;
const int FONT_STYLE_ITALIC = 1;

const int FONT_WEIGHT_NORMAL = 0;
const int FONT_WEIGHT_BOLD = 1;

const int TEXT_ALIGN_LEFT = 0;
const int TEXT_ALIGN_RIGHT = 1;
const int TEXT_ALIGN_CENTER = 2;
const int TEXT_ALIGN_JUSTIFY = 3;

const int TEXT_DECORATION_NONE = 0;
const int TEXT_DECORATION_UNDERLINE = 1;
const int TEXT_DECORATION_OVERLINE = 2;
const int TEXT_DECORATION_LINE_THROUGH = 3;

const int TEXT_TRANSFORM_NONE = 0;
const int TEXT_TRANSFORM_CAPITALIZE = 1;
const int TEXT_TRANSFORM_UPPERCASE = 2;
const int TEXT_TRANSFORM_LOWERCASE = 3;

const int WHITE_SPACE_NORMAL = 0;
const int WHITE_SPACE_PRE = 1;
const int WHITE_SPACE_NOWRAP = 2;
const int WHITE_SPACE_PRE_WRAP = 3;
const int WHITE_SPACE_PRE_LINE = 4;

const int VERTICAL_ALIGN_BASELINE = 0;
const int VERTICAL_ALIGN_MIDDLE = 1;
const int VERTICAL_ALIGN_SUB = 2;
const int VERTICAL_ALIGN_SUPER = 3;
const int VERTICAL_ALIGN_TEXT_TOP = 4;
const int VERTICAL_ALIGN_TEXT_BOTTOM = 5;
const int VERTICAL_ALIGN_TOP = 6;
const int VERTICAL_ALIGN_BOTTOM = 7;

const int Z_INDEX_AUTO = 0;
const int Z_INDEX_TOP = 1;
const int Z_INDEX_BOTTOM = 2;

const int DRAG_NONE = 0;
const int DRAG_DRAG = 1;
const int DRAG_DRAG_DROP = 2;
const int DRAG_BLOCK = 3;
const int DRAG_CLONE = 4;

const int TAB_INDEX_NONE = 0;
const int TAB_INDEX_AUTO = 1;

const int FOCUS_NONE = 0;
const int FOCUS_AUTO = 1;

}
}

#endif
