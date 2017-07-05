var fp = (function() {
  var my = {};

  if ('indexOf' in Array.prototype) {
    my.indexOf = function(array, item) {
      return array.indexOf(item);
    }
  } else {
    my.indexOf = function(array, item) {
      for (var i = 0; i < array.length; i++) {
        if (array[i] == item) {
          return i;
        }
      }
      return -1;
    }
  }

  my.Shade = {
    LIGHT: "#bbbbbb",
    MEDIUM: "#777777",
    DARK: "#333333"
  };

  my.LabelPosition = {
    ABOVE: 1,
    ABOVE_CENTERED: 2,
    BELOW: 3
  };

  my.Light = {
    OFF: 0,
    ON: 1,
    BLINK: 2
  };

  my.DisplayBlinkState = {
    OFF: 0,
    UNDERLINE: 1,
    CHAR: 2
  };

  my.Keyboard = {
    VFX: 'VFX',
    VFX_SD: 'VFX-SD',
    SD1: 'SD-1',
    SD1_32: 'SD-1/32'
  }

  my.segmentPaths = [
    "M1053 705 c-43 19 -57 47 -43 89 23 70 87 106 189 106 38 0 70 8 106 25 79 39 111 41 183 11 80 -34 119 -33 205 6 68 31 78 33 192 33 116 0 123 -1 195 -35 67 -31 87 -35 182 -40 101 -5 108 -7 137 -34 40 -38 50 -89 25 -118 -11 -11 -37 -29 -59 -39 -37 -17 -79 -19 -660 -18 -505 0 -626 2 -652 14z",
    "M2519 963 c-20 13 -46 47 -63 81 -28 53 -31 69 -37 199 -7 155 -20 211 -75 319 -50 99 -68 199 -54 301 23 167 52 217 126 217 37 0 47 -5 77 -40 53 -63 74 -151 97 -410 5 -63 16 -167 24 -230 42 -326 45 -374 21 -419 -24 -47 -63 -54 -116 -18z",
    "M2144 1089 c-59 43 -88 78 -135 161 -23 41 -75 112 -115 156 -108 119 -132 188 -136 386 -3 107 -1 118 17 132 11 9 28 16 37 16 25 0 92 -63 154 -145 29 -39 100 -129 158 -200 58 -72 113 -144 121 -162 19 -40 32 -106 41 -214 5 -68 3 -91 -9 -116 -28 -52 -74 -57 -133 -14z",
    "M1515 1089 c-70 43 -69 41 -77 285 -3 121 -11 259 -18 306 -6 47 -13 142 -17 211 -6 141 5 183 54 195 78 20 124 -53 135 -216 13 -192 26 -274 61 -385 77 -245 76 -359 -3 -400 -39 -20 -99 -19 -135 4z",
    "M1108 1087 c-32 36 -42 71 -50 163 -5 52 -11 122 -14 156 -6 55 -1 75 41 200 53 152 59 165 87 183 16 10 24 9 44 -4 31 -20 43 -51 55 -135 5 -36 17 -97 26 -137 14 -63 15 -81 3 -145 -37 -205 -43 -222 -88 -271 -30 -32 -80 -37 -104 -10z",
    "M797 938 c-32 36 -44 102 -67 377 -19 222 -30 337 -42 428 -17 138 12 277 67 313 55 36 123 -6 173 -109 52 -106 54 -167 12 -292 -27 -78 -30 -102 -30 -205 0 -79 7 -147 20 -210 11 -51 20 -111 20 -133 0 -123 -97 -231 -153 -169z",
    "M1940 2120 c-14 4 -56 8 -94 9 -80 1 -141 26 -181 73 -32 38 -32 78 1 118 48 56 84 67 249 74 146 7 151 7 195 -17 52 -27 99 -89 100 -130 0 -33 -31 -81 -63 -98 -27 -15 -125 -38 -157 -38 -14 1 -36 5 -50 9z",
    "M1099 2129 c-51 10 -110 43 -132 73 -28 37 -16 88 32 138 36 38 41 40 95 40 64 0 115 -22 159 -68 32 -34 46 -97 28 -130 -23 -43 -109 -68 -182 -53z",
    "M2279 2467 c-56 50 -69 80 -80 186 -6 51 -16 127 -24 169 -14 83 -10 123 25 213 36 95 44 146 31 203 -14 66 -14 205 -1 254 12 41 70 98 100 98 52 0 75 -100 100 -435 6 -77 22 -241 36 -364 28 -255 27 -268 -37 -325 -54 -49 -94 -49 -150 1z",
    "M1701 2579 c-24 24 -40 122 -44 261 -2 95 1 112 27 178 15 41 44 94 63 119 19 25 57 92 84 149 58 121 94 164 137 164 38 0 78 -32 90 -73 19 -60 22 -181 7 -238 -20 -77 -116 -277 -180 -376 -30 -46 -66 -106 -80 -133 -35 -69 -69 -86 -104 -51z",
    "M1372 2456 c-40 28 -52 66 -52 166 0 92 -27 323 -55 468 -21 108 -19 246 3 290 21 44 59 65 96 56 47 -12 123 -92 146 -152 28 -77 28 -203 -1 -281 -21 -55 -22 -62 -10 -218 6 -88 14 -178 16 -201 6 -54 -11 -110 -38 -129 -28 -19 -76 -19 -105 1z",
    "M1067 2721 c-19 11 -122 161 -156 228 -42 81 -51 129 -51 276 0 113 3 147 18 175 39 80 102 35 199 -141 28 -52 56 -112 62 -134 6 -22 11 -114 11 -205 0 -134 -3 -170 -16 -188 -16 -24 -39 -27 -67 -11z",
    "M695 2447 c-45 23 -76 54 -91 90 -8 18 -18 101 -24 190 -18 298 -21 328 -52 516 -26 164 -29 194 -18 235 23 91 68 107 130 44 46 -45 59 -86 71 -217 5 -55 13 -143 18 -195 11 -120 37 -199 101 -302 48 -78 50 -85 50 -153 0 -95 -15 -143 -60 -187 -42 -42 -75 -48 -125 -21z",
    "M1550 3539 c-14 5 -57 24 -97 44 -107 54 -134 56 -218 12 -79 -42 -105 -41 -170 3 -35 23 -53 28 -145 33 -131 8 -181 24 -194 62 -14 39 9 78 54 94 49 17 1278 18 1315 1 51 -23 42 -87 -18 -132 -21 -15 -48 -21 -115 -26 -77 -4 -94 -9 -140 -38 -85 -55 -195 -76 -272 -53z",
    "M2619 3393 c-19 12 -45 43 -59 67 -36 65 -36 183 0 255 48 93 136 107 207 33 60 -61 76 -152 48 -257 -17 -63 -45 -97 -94 -111 -52 -14 -64 -13 -102 13z",
    "M512 4422 c-38 8 -46 15 -63 51 -37 83 -18 153 51 181 36 14 127 16 863 16 642 0 827 -3 847 -13 16 -8 31 -31 44 -64 16 -46 17 -57 5 -94 -8 -24 -26 -51 -42 -63 -28 -21 -34 -21 -845 -23 -501 0 -834 3 -860 9z",
  ];
  my.charWidth = 342;
  my.charHeight = 572;
  my.segmentScale = 0.1;

  my.createElement = function(tag) {
    return document.createElementNS("http://www.w3.org/2000/svg", tag);
  }

  my.showElement = function(e) {
    e.removeAttribute("display");
  }

  my.hideElement = function(e) {
    e.setAttribute("display", "none");
  }

  my.svg = function() {
    if (my._svg == null) {
      my._svg = document.getElementsByTagName('svg')[0];
    }
    return my._svg;
  }

  my.pt = function() {
    if (my._pt == null) {
      my._pt = my.svg().createSVGPoint();
    }
    return my._pt;
  }

  my.pointIn = function(el, x, y) {
    var pt = my.pt();
    pt.x = x; pt.y = y;
    return pt.matrixTransform(el.getScreenCTM().inverse());
  }

  my.Display = function(parent, rows, cols) {
    this.cells = new Array();
    this.width = my.charWidth * cols;
    this.height = my.charHeight * rows;
    this.blinkPhase = true;

    var templateCell = my.createElement("g");
    templateCell.setAttribute('transform', 'scale(' + my.segmentScale + ',' + my.segmentScale + ')');
    for (var i = 0; i < my.segmentPaths.length; i++) {
      var segmentPath = my.createElement("path");
      segmentPath.setAttribute('d', my.segmentPaths[i]);
      templateCell.appendChild(segmentPath);
    }

    for (var row = 0; row < 2; row++) {
      this.cells[row] = new Array();
      for (var col = 0; col < 40; col++) {
        this.cells[row][col] = {
          char: ' ',
          blink: false,
          underline: false,
          segments: new Array(),
        };
        var charCell = templateCell.cloneNode(true);
        var ctm = "translate(" + col * my.charWidth + ", " + row * my.charHeight + ") " + charCell.getAttribute("transform");
        charCell.setAttribute("transform", ctm);
        parent.appendChild(charCell);

        var segs = charCell.getElementsByTagName("path");
        for (var cc = 0; cc < segs.length; cc++) {
          this.cells[row][col].segments[cc] = segs[cc];
        }
      }
    }

    parent.setAttribute("viewBox", "0 0 " + this.width + " " + this.height);
  }

  my.Display.segmentsByCharacter = [
    0x0000, //  0000 0000 0000 0000 SPACE
    0x7927, //  0011 1001 0010 0111 '0.'
    0x0028, //  0000 0000 0010 1000 '"'
    0x4408, //  0000 0100 0000 1000 '1.'
    0x25e9, //  0010 0101 1110 1001 '$'
    0x70c3, //  0011 0000 1100 0011 '2.'
    0x0000, //  0000 0000 0000 0000 '&'
    0x0010, //  0000 0000 0001 0000 '''
    0x61c3, //  0010 0001 1100 0011 '3.'
    0x41e2, //  0000 0001 1110 0010 '4.'
    0x0edc, //  0000 1110 1101 1100 '*'
    0x04c8, //  0000 0100 1100 1000 '+'
    0x0000, //  0000 0000 0000 0000 ','
    0x00c0, //  0000 0000 1100 0000 '-'
    0x4000, //  0100 0000 0000 0000 '.'
    0x0804, //  0000 1000 0000 0100 '/'
    0x3927, //  0011 1001 0010 0111 '0'
    0x0408, //  0000 0100 0000 1000 '1'
    0x30c3, //  0011 0000 1100 0011 '2'
    0x21c3, //  0010 0001 1100 0011 '3'
    0x01e2, //  0000 0001 1110 0010 '4'
    0x21e1, //  0010 0001 1110 0001 '5'
    0x31e1, //  0011 0001 1110 0001 '6'
    0x0103, //  0000 0001 0000 0011 '7'
    0x31e3, //  0011 0001 1110 0011 '8'
    0x21e3, //  0010 0001 1110 0011 '9'
    0x0000, //  0000 0000 0000 0000 ':'
    0x71e1, //  0011 0001 1110 0001 '6.'
    0x0204, //  0000 0010 0000 0100 '('
    0x20c0, //  0010 0000 1100 0000 '='
    0x0810, //  0000 1000 0001 0000 ')'
    0x0000, //  0000 0000 0000 0000 '?'
    0x3583, //  0011 0101 1000 0011 '@'
    0x11e3, //  0001 0001 1110 0011 'A'
    0x254b, //  0010 0101 0100 1011 'B'
    0x3021, //  0011 0000 0010 0001 'C'
    0x250b, //  0010 0101 0000 1011 'D'
    0x30e1, //  0011 0000 1110 0001 'E'
    0x10e1, //  0001 0000 1110 0001 'F'
    0x3161, //  0011 0001 0110 0001 'G'
    0x11e2, //  0001 0001 1110 0010 'H'
    0x2409, //  0010 0100 0000 1001 'I'
    0x3102, //  0011 0001 0000 0010 'J'
    0x12a4, //  0001 0010 1010 0100 'K'
    0x3020, //  0011 0000 0010 0000 'L'
    0x1136, //  0001 0001 0011 0110 'M'
    0x1332, //  0001 0011 0011 0010 'N'
    0x3123, //  0011 0001 0010 0011 'O'
    0x10e3, //  0001 0000 1110 0011 'P'
    0x3323, //  0011 0011 0010 0011 'Q'
    0x12e3, //  0001 0010 1110 0011 'R'
    0x21e1, //  0010 0001 1110 0001 'S'
    0x0409, //  0000 0100 0000 1001 'T'
    0x3122, //  0011 0001 0010 0010 'U'
    0x1824, //  0001 1000 0010 0100 'V'
    0x1b22, //  0001 1011 0010 0010 'W'
    0x0a14, //  0000 1010 0001 0100 'X'
    0x0414, //  0000 0100 0001 0100 'Y'
    0x2805, //  0010 1000 0000 0101 'Z'
    0x3021, //  0011 0000 0010 0001 '['
    0x71e3, //  0011 0001 1110 0011 '8.'
    0x2103, //  0010 0001 0000 0011 ']'
    0x0a00, //  0000 1010 0000 0000 '^'
    0x2000, //  0010 0000 0000 0000 '_'
    0x0010, //  0000 0000 0001 0000 '`'
    0x11e3, //  0001 0001 1110 0011 'a'
    0x254b, //  0010 0101 0100 1011 'b'
    0x3021, //  0011 0000 0010 0001 'c'
    0x250b, //  0010 0101 0000 1011 'd'
    0x30e1, //  0011 0000 1110 0001 'e'
    0x10e1, //  0001 0000 1110 0001 'f'
    0x3161, //  0011 0001 0110 0001 'g'
    0x11e2, //  0001 0001 1110 0010 'h'
    0x2409, //  0010 0100 0000 1001 'i'
    0x3102, //  0011 0001 0000 0010 'j'
    0x12a4, //  0001 0010 1010 0100 'k'
    0x3020, //  0011 0000 0010 0000 'l'
    0x1136, //  0001 0001 0011 0110 'm'
    0x1332, //  0001 0011 0011 0010 'n'
    0x3123, //  0011 0001 0010 0011 'o'
    0x10e3, //  0001 0000 1110 0011 'p'
    0x3323, //  0011 0011 0010 0011 'q'
    0x12e3, //  0001 0010 1110 0011 'r'
    0x21e1, //  0010 0001 1110 0001 's'
    0x0409, //  0000 0100 0000 1001 't'
    0x3122, //  0011 0001 0010 0010 'u'
    0x1824, //  0001 1000 0010 0100 'v'
    0x1b22, //  0001 1011 0010 0010 'w'
    0x0a14, //  0000 1010 0001 0100 'x'
    0x0414, //  0000 0100 0001 0100 'y'
    0x2805, //  0010 1000 0000 0101 'z'
    0x3021, //  0011 0000 0010 0001 '{'
    0x0408, //  0000 0100 0000 1000 '|'
    0x2103, //  0010 0001 0000 0011 '}'
    0x0a00, //  0000 1010 0000 0000 '~'
    0x0000, //  0000 0000 0000 0000 DEL
  ];

  my.Display.colorOn = "#00ffbb";
  my.Display.colorOff = "#002211";
  my.Display.overdraw = 0;

  my.Display.prototype.showSegments = function(segments, lit) {
    // debugger;
    var mask = 1;
    var i;
    for (i = 0; i < 16; i++) {
      var on = (lit & mask) != 0;
      segments[i].setAttribute("fill", on ? my.Display.colorOn : my.Display.colorOff);
      if (my.Display.overdraw) {
        segments[i].setAttribute("stroke-width", my.Display.overdraw);
        if (on) {
          segments[i].setAttribute("stroke", my.Display.colorOn);
        } else {
          segments[i].setAttribute("stroke", "none");
        }
      } else {
        segments[i].setAttribute("stroke", "none");
      }
      mask <<= 1;
    }
  }

  my.Display.segmentsForCharacter = function(c, underline, blink, blinkPhase) {
    var lit = (c < 32 || 127 < c) ? 0 : my.Display.segmentsByCharacter[c - 32];
    if (blink && !blinkPhase) {
      if (underline) {
        return lit;
      } else {
        return 0;
      }
    } else {
      if (underline) {
        return lit | 0x8000;
      } else {
        return lit;
      }
    }
  }

  my.Display.prototype.setChar = function(y, x, c, underline, blink) {
    var cell = this.cells[y][x];
    cell.char = c;
    cell.underline = underline;
    cell.blink = blink;

    this.showSegments(cell.segments, my.Display.segmentsForCharacter(c, underline, blink, this.blinkPhase));
  }

  my.Display.prototype.showString = function(y, x, s) {
    for (var i = 0; i < s.length; i++) {
      this.setChar(y, x, s.charCodeAt(i), false, false);
      x++;
      if (x >= this.cells[y].length) {
        x = 0;
        y++;
      }
      if (y >= this.cells.length) {
        y = 0;
      }
    }
  }

  my.Display.prototype.clear = function() {
    for (var row = 0; row < this.cells.length; row++) {
      var line = this.cells[row];
      for (var col = 0; col < line.length; col++) {
        this.setChar(row, col, ' ', false, false);
      }
    }
  }

  my.Display.prototype.blink = function(y, x) {
    return this.cells[y][x].blink;
  }

  my.Display.prototype.underline = function(y, x) {
    return this.cells[y][x].underline;
  }

  my.Display.prototype.setBlinkPhase = function(phase) {
    this.blinkPhase = phase;
    for (var row = 0; row < this.cells.length; row++) {
      var line = this.cells[row];
      for (var col = 0; col < line.length; col++) {
        var cell = line[col];
        if (cell.blink) {
          this.showSegments(cell.segments,
            my.Display.segmentsForCharacter(cell.char, cell.underline, cell.blink, this.blinkPhase));
          }
        }
      }
    }

    my.Rect = function(x, y, w, h) {
      this.x = x;
      this.y = y;
      this.w = w;
      this.h = h;
    }

    my.Rect.prototype.union = function(other) {
      if (this.w == 0 || this.h == 0) {
        return other;
      } else if (other.w == 0 || other.h == 0) {
        return this;
      } else {
        minX = Math.min(this.x, other.x);
        maxX = Math.max(this.x+this.w, other.x+other.w);
        minY = Math.min(this.y, other.y);
        maxY = Math.max(this.y+this.h, other.y+other.h);
        return new my.Rect(minX, minY, maxX-minX, maxY-minY);
      }
    }

    my.Rect.prototype.inset = function(dx, dy) {
      return new my.Rect(this.x + dx, this.y + dy, this.w - 2*dx, this.h - 2*dy);
    }

    my.Rect.prototype.offset = function(dx, dy) {
      return new my.Rect(this.x+dx, this.y+dy, this.w, this.h);
    }

    my.Rect.prototype.toPath = function(r) {
      var rect = my.createElement("rect");
      rect.setAttribute("x", this.x);
      rect.setAttribute("y", this.y);
      rect.setAttribute("width", this.w);
      rect.setAttribute("height", this.h);
      if (r != null) {
        rect.setAttribute("rx", r);
        rect.setAttribute("ry", r);
      }
      return rect;
    }

    my.Rect.prototype.getX = function(d) {
      return this.x + d * this.w;
    }

    my.Rect.prototype.getY = function(d) {
      return this.y + d * this.h;
    }

    my.displayRect = new my.Rect(15, 7, 82, 13);

    my.Button = function(x, y, w, h, label, labelPosition, value, color, multiPage, lightId) {
      var that = this;
      this.rect = new my.Rect(x, y, w, h);

      var rect = this.rect.inset(0.1, 0.1);
      var translation = "translate(" + x + "," + y + ")";
      this.halo = rect.toPath(0.5);
      this.halo.setAttribute("stroke", "#666666");
      this.halo.setAttribute("stroke-width", "2");
      this.halo.setAttribute("fill", "none");
      my.hideElement(this.halo);

      rect = rect.offset(-x, -y);
      this.outline = rect.toPath(0.5);
      this.outline.setAttribute("fill", color);
      this.outline.setAttribute("stroke", "none");

      this.group = my.createElement("g");
      this.group.setAttribute("transform", translation);
      this.group.appendChild(this.outline);

      this.label = label;
      this.labelPosition = labelPosition;
      this.value = value;
      this.color = color;
      this.multiPage = multiPage;
      this.lightId = lightId;

      if (label != undefined) {
        var labelLines = label.split("\n");
        var fontSize = 1.4;
        var labelText = my.createElement("text");
        labelText.setAttribute('fill', 'white');
        labelText.setAttribute('stroke', 'none');
        labelText.setAttribute('font-size', fontSize);
        labelText.setAttribute('font-family', 'Helvetica');
        labelText.setAttribute('font-style', 'italic');
        labelText.setAttribute('width', w);
        labelText.setAttribute('dominant-baseline', 'bottom');
        var x0 = 0;
        var y0 = (1 - labelLines.length) * fontSize;
        switch(labelPosition) {
        case my.LabelPosition.ABOVE_CENTERED:
          labelText.setAttribute('text-anchor', 'middle');
          x0 = w/2;
          // fall through
        case my.LabelPosition.ABOVE:
          y0 = (1 - labelLines.length) * fontSize - 0.3;
          break;
        case my.LabelPosition.BELOW:
          y0 = h + fontSize - 0.3;
          break;
        }
        for (var i = 0; i < labelLines.length; i++) {
          var tspan = my.createElement("tspan");
          tspan.setAttribute('x', x0);
          tspan.setAttribute('y', y0 + i * fontSize);
          tspan.appendChild(document.createTextNode(labelLines[i]));
          labelText.appendChild(tspan);
        }
        this.group.appendChild(labelText);
      }

      if (lightId >= 0) {
        this.lightOn = my.createElement("path");
        this.lightOn.setAttribute("d", "M" + (rect.w/3) + "," + (rect.y+rect.h/25) + " h" + (rect.w/3) + " v" + (rect.h/3) + "  h" + (-rect.w/3) + " z");
        this.lightOff = this.lightOn.cloneNode(true);
        this.lightOn.setAttribute("fill", "#22ff22");
        this.lightOff.setAttribute("fill", "#112211");
        my.hideElement(this.lightOn);

        this.group.appendChild(this.lightOn);
        this.group.appendChild(this.lightOff);
      }

      this.group.addEventListener("touchstart", function(e) { that.press(e); }, true);
      this.group.addEventListener("touchend", function(e) { that.release(e); }, true);
      this.group.addEventListener("mousedown", function(e) { that.press(e); }, true);
      this.group.addEventListener("mouseout", function(e) { that.release(e); }, true);
      this.group.addEventListener("mouseup", function(e) { that.release(e); }, true);

      this.isPressed = false;
      this.lightState = my.Light.OFF;
      this.lightIsOn = false;
      this.blinkPhase = true;

      this.onPress = undefined;
      this.onRelease = undefined;
    }

    my.Button.prototype.showPressed = function(isPressed) {
      if (isPressed) {
        my.showElement(this.halo);
      } else {
        my.hideElement(this.halo);
      }
    }

    my.Button.prototype.press = function(e) {
      e.preventDefault();

      if (!this.isPressed) {
        this.isPressed = true;
        this.showPressed(true);

        if (this.onPress != undefined) {
          this.onPress(this);
        }
      }

      return false;
    }

    my.Button.prototype.release = function(e) {
      e.preventDefault();

      if (this.isPressed) {
        this.isPressed = false;
        this.showPressed(false);

        if (this.onRelease != undefined) {
          this.onRelease(this);
        }
      }

      return false;
    }

    my.Button.prototype.updateLight = function() {
      var on = this.lightState == my.Light.ON || (this.blinkPhase && this.lightState == my.Light.BLINK);
      if (on != this.lightIsOn) {
        my.hideElement(this.lightIsOn ? this.lightOn : this.lightOff);
        this.lightIsOn = on;
        my.showElement(this.lightIsOn ? this.lightOn : this.lightOff);
      }
    }

    my.Button.prototype.setLight = function(state) {
      this.lightState = state;
      this.updateLight();
    }

    my.Button.prototype.setBlinkPhase = function(phase) {
      this.blinkPhase = phase;
      this.updateLight();
    }

    my.Touch = function(x, y) {
      this.x = x;
      this.y = y;
    }

    my.makeTouch = function(e) {
      return new my.Touch(e.clientX, e.clientY);
    }

    my.Slider = function(x, y, w, h, channel, value) {

      function makeRectPath(x, y, w, h, color) {
        path = new my.Rect(x, y, w, h).toPath();
        path.setAttribute("fill", color);
        return path;
      }

      var that = this;
      this.channel = channel;
      this.value = value;

      this.rect = new my.Rect(x, y, w, h);
      var rect = this.rect.offset(-x, -y);
      var translation = "translate(" + x + "," + y + ")";
      this.group = my.createElement("g");
      this.group.setAttribute("transform", translation);

      this.frameColor = "#333333";
      this.frameActiveColor = "#666666";
      this.frame = rect.inset(0.25, 0.25).toPath();
      this.frame.setAttribute("stroke", this.frameColor);
      this.frame.setAttribute("stroke-width", "0.5");
      this.group.appendChild(this.frame);

      this.handleX = 0.75;
      this.handleW = w - 1.5;
      this.handleH = 4;
      this.handleMinY = 0.75;
      this.handleMaxY = h - 0.75 - this.handleH;

      this.handle = my.createElement("g");
      this.handle.appendChild(makeRectPath(0, 0, this.handleW, this.handleH, "#333333"));
      this.handle.appendChild(makeRectPath(0, 0, this.handleW, 0.75, "#444444"));
      this.handle.appendChild(makeRectPath(0, 1.75, this.handleW, 0.25, "#222222"));
      this.handle.appendChild(makeRectPath(0, 2, this.handleW, 0.25, "#444444"));
      this.handle.appendChild(makeRectPath(0, 3.25, this.handleW, 0.75, "#222222"));
      this.group.appendChild(this.handle);

      this.setValue(value);

      this.handle.addEventListener("touchstart", function(e) { that.touchstart(e); }, true);
      this.group.addEventListener("touchmove", function(e) { that.touchmove(e); }, true);
      this.group.addEventListener("touchend", function(e) { that.touchend(e); }, true);
      this.group.addEventListener("touchcancel", function(e) { that.touchend(e); }, true);

      this.handle.addEventListener("mousedown", function(e) { that.grab(e.clientX, e.clientY); }, true);
      this.group.addEventListener("mousemove", function(e) { that.drag(e.clientX, e.clientY); }, true);
      this.group.addEventListener("mouseup", function(e) { that.release(); }, true);

      this.onValueChanged = undefined;
      this.isGrabbed = false;
      this.activeTouches = new Map();

    }

    my.Slider.prototype.setValue = function(value) {
      this.value = Math.max(0.0, Math.min((1.0, value)));
      this.handleY = this.handleMinY + (1.0 - value) * (this.handleMaxY - this.handleMinY);
      this.handle.setAttribute("transform", "translate(" + this.handleX + "," + this.handleY + ")");
    }

    my.Slider.prototype.setHandleY = function(handleY) {
      this.handleY = Math.max(this.handleMinY, Math.min(this.handleMaxY, handleY));
      // console.log("Setting handleY to " + handleY + " => " + this.handleY);
      this.value = 1.0 - (this.handleY - this.handleMinY) / (this.handleMaxY - this.handleMinY);
      this.handle.setAttribute("transform", "translate(" + this.handleX + "," + this.handleY + ")");
    }

    my.Slider.prototype.grab = function(x, y) {
      this.isGrabbed = true;
      this.frame.setAttribute("stroke", this.frameActiveColor);
      var p = my.pointIn(this.group, x, y);
      this.dragOffset = p.y - this.handleY;
      // console.log("Grabbing with handleY=" + this.handleY + ", p.y=" + p.y + " => dragOffset=" + this.dragOffset);
    }

    my.Slider.prototype.drag = function(x, y) {
      if (this.isGrabbed) {
        var p = my.pointIn(this.group, x, y);
        var newHandleY = p.y - this.dragOffset;
        // console.log("Dragged with p.y=" + p.y + ", dragOffset=" + this.dragOffset + " => new handleY=" + newHandleY);
        this.setHandleY(newHandleY);
        if (this.onValueChanged != null) {
          this.onValueChanged(this);
        }
      }
    }

    my.Slider.prototype.release = function(e) {
      this.isGrabbed = false;
      this.frame.setAttribute("stroke", this.frameColor);
    }

    my.Slider.prototype.activeTouchCenter = function() {
      var n = this.activeTouches.size;
     if (n <= 0) {
        return undefined;
      }
      var x = 0;
      var y = 0;

      for (const touch of this.activeTouches.values()) {
        x += touch.x;
        y += touch.y;
      }

      return new my.Touch(x / n, y / n);
    }

    my.Slider.prototype.touchstart = function(e) {
      e.preventDefault();

      var wasEmpty = this.activeTouches.size == 0;
      for (var i = 0; i < e.targetTouches.length; i++) {
        var touch = e.targetTouches.item(i);
        this.activeTouches.set(touch.identifier, my.makeTouch(touch));
      }

      center = this.activeTouchCenter();
      if (center != null) {
        this.grab(center.x, center.y);
      }

      return false;
    }

    my.Slider.prototype.touchmove = function(e) {
      e.preventDefault();

      for (var i = 0; i < e.changedTouches.length; i++) {
        var touch = e.changedTouches.item(i);
        if (this.activeTouches.has(touch.identifier)) {
          this.activeTouches.set(touch.identifier, my.makeTouch(touch));
        }
      }
      center = this.activeTouchCenter();
      if (center != null) {
        this.drag(center.x, center.y);
      }

      return false;
    }

    my.Slider.prototype.touchend = function(e) {
      e.preventDefault();

      for (var i = 0; i < e.changedTouches.length; i++) {
        var touch = e.changedTouches.item(i);
        this.activeTouches.delete(touch.identifier)
      }
      if (this.activeTouches.size == 0) {
        this.release();
      } else {
        center = this.activeTouchCenter();
        if (center != null) {
          this.grab(center.x, center.y);
        }
      }

      return false;
    }

    my.Panel = function(serverUrl, keyboard, version) {
      this.serverUrl = serverUrl;
      this.keyboard = keyboard;
      this.version = version;

      this.container = my.createElement("svg");
      this.container.setAttribute("preserveAspectRatio", "xMidYMid meet");
      this.container.setAttribute("width", "2000");
      this.container.setAttribute("height", "375");
      this.container.setAttribute("overflow", "scroll");

      this.haloContainer = my.createElement("g");
      this.container.appendChild(this.haloContainer);

      this.mainContainer = my.createElement("g");
      this.container.appendChild(this.mainContainer);

      this.displayContainer = my.createElement("svg");
      this.display = new my.Display(this.displayContainer, 2, 40);
      this.displayContainer.setAttribute("preserveAspectRatio", "xMidYMid meet");
      this.displayContainer.setAttribute("x", my.displayRect.x);
      this.displayContainer.setAttribute("y", my.displayRect.y);
      this.displayContainer.setAttribute("width", my.displayRect.w);
      this.displayContainer.setAttribute("height", my.displayRect.h);
      this.container.appendChild(this.displayContainer);

      this.buttons = new Array();
      this.lightButtons = new Array();
      this.analogControls = new Array();
      this.addControls(keyboard);

      this.cursorX = 0;
      this.cursorY = 0;
      this.savedCursorX = 0;
      this.savedCursorY = 0;
      this.blink = false;
      this.underline = false;

      this.serverUrl = serverUrl;
      try {
        this.connect();
      } catch (e) {
        console.log("Unable to connect to '" + serverUrl + "': " + e);
      }

      var that = this;
      this.blinkPhase = 0;
      setInterval(function() { that.updateBlink(); }, 250);
    }

    my.Panel.prototype.updateBlink = function() {
      this.blinkPhase = (this.blinkPhase + 1) % 4;
      this.display.setBlinkPhase(this.blinkPhase < 2);
      var buttonPhase = (this.blinkPhase % 2) == 0;
      for (var i = 0; i < this.lightButtons.length; i++) {
        this.lightButtons[i].setBlinkPhase(buttonPhase);
      }
    }

    my.Panel.prototype.connect = function() {
      var that = this;
      var panel = this;
      var reconnect = function() {
        that.connect();
      }

      this.socket = new WebSocket(this.serverUrl);
      this.socket.binaryType = "arraybuffer";

      this.socket.onopen = function(event) {
        console.log("opened: " + event);
        panel.sendString("I"); // Request server information
      };

      this.socket.onmessage = function(event) {
        var data = new Uint8Array(event.data);
        var c = String.fromCharCode(data[0]);

        if (c == 'A') {
          console.log("handling analog value")
          panel.handleAnalogValue(data.slice(1));
        } else if (c == 'B') {
          console.log("handling button state")
          panel.handleButtonState(data.slice(1));
        } else if (c == 'D') {
          console.log("handling display data")
          panel.handleDisplayData(data.slice(1));
        } else if (c == 'I') {
          console.log("handling server information");
          panel.handleServerInformation(data.slice(1));
        }
      };

      this.socket.onclose = function(event) {
        console.log("closed: ", event);
        // reconnect after 1 second
        setTimeout(reconnect, 1000);
      };

      this.socket.onerror = function(event) {
        console.log("error: ", event);
      };
    }

    my.Panel.prototype.addButton = function(x, y, w, h, label, labelPosition, value, color, multiPage, lightId) {
      var that = this;
      var button = new my.Button(x, y, w, h, label, labelPosition, value, color, multiPage, lightId);
      this.haloContainer.appendChild(button.halo);

      if (lightId >= 0) {
        if (lightId >= this.lightButtons.length) {
          this.lightButtons.length = lightId + 1;
        }
        this.lightButtons[lightId] = button;
      }

      this.mainContainer.appendChild(button.group);
      this.buttons[value] = button;

      button.onPress = function(b) {
        that.onButtonPressed(b);
      }
      button.onRelease = function(b) {
        that.onButtonReleased(b);
      }

      return button;
    }

    my.Panel.prototype.addSlider = function(x, y, w, h, channel, value) {
      var that = this;
      var slider = new my.Slider(x, y, w, h, channel, value);

      this.mainContainer.appendChild(slider.group);
      this.analogControls[channel] = slider;

      slider.onValueChanged = function(s) {
        that.onAnalogValueChanged(s);
      }

      return slider;
    }

    my.Panel.prototype.addButtonBelowDisplay = function(x, y, label, value, shade) {
      return this.addButton(x, y, 6, 4, label, my.LabelPosition.BELOW, value, shade, false, -1);
    }

    my.Panel.prototype.addButtonWithLightBelowDisplay = function(x, y, label, value, shade, lightId) {
      return this.addButton(x, y, 6, 4, label, my.LabelPosition.BELOW, value, shade, false, lightId);
    }

    my.Panel.prototype.addLargeButton = function(x, y, label, value, shade) {
      return this.addButton(x, y, 6, 4, label, my.LabelPosition.ABOVE, value, shade, false, -1);
    }

    my.Panel.prototype.addLargeButtonWithLight = function(x, y, label, value, shade, lightId) {
      return this.addButton(x, y, 6, 4, label, my.LabelPosition.ABOVE, value, shade, false, lightId);
    }

    my.Panel.prototype.addSmallButton = function(x, y, label, value, shade, multiPage) {
      return this.addButton(x, y, 6, 2, label, my.LabelPosition.ABOVE, value, shade, multiPage, -1);
    }

    my.Panel.prototype.addIncDecButton = function(x, y, label, value, shade, multiPage) {
      return this.addButton(x, y, 6, 2, label, my.LabelPosition.ABOVE_CENTERED, value, shade, multiPage, -1);
    }

    my.Panel.prototype.addControls = function(keyboard) {
      console.log("keyboard is '" + keyboard + "'");

      // Normalize the keyboard string.
      var hasSeq = false;
      var hasBankSet = false;
      keyboard = keyboard.toLowerCase();
      if (keyboard.indexOf('sd') != -1) {
        hasSeq = true;

        if (keyboard.indexOf('1') != -1) {
          hasBankSet = true;

          if (keyboard.indexOf('32') != -1) {
            keyboard = my.Keyboard.SD1_32;
          } else {
            keyboard = my.Keyboard.SD1;
          }
        } else {
          keyboard = my.Keyboard.VFX_SD;
        }
      } else {
        keyboard = my.Keyboard.VFX;
      }

      console.log("normalized keyboard is '" + keyboard + "'");

      var cartString = hasBankSet ? "BankSet" : "Cart";

      this.addButtonWithLightBelowDisplay(10, 29, cartString, 52, my.Shade.LIGHT, 0xf);
      this.addButtonWithLightBelowDisplay(16, 29, "Sounds",   53, my.Shade.LIGHT, 0xd);
      this.addButtonWithLightBelowDisplay(22, 29, "Presets",  54, my.Shade.LIGHT, 0x7);
      if (hasSeq) {
        this.addButtonBelowDisplay     (28, 29, "Seq",      51, my.Shade.LIGHT);
      }

      this.addButtonWithLightBelowDisplay(42, 29, "0", 55, my.Shade.MEDIUM, 0xe);
      this.addButtonWithLightBelowDisplay(48, 29, "1", 56, my.Shade.MEDIUM, 0x6);
      this.addButtonWithLightBelowDisplay(54, 29, "2", 57, my.Shade.MEDIUM, 0x4);
      this.addButtonWithLightBelowDisplay(60, 29, "3", 46, my.Shade.MEDIUM, 0xc);
      this.addButtonWithLightBelowDisplay(66, 29, "4", 47, my.Shade.MEDIUM, 0x3);
      this.addButtonWithLightBelowDisplay(72, 29, "5", 48, my.Shade.MEDIUM, 0xb);
      this.addButtonWithLightBelowDisplay(78, 29, "6", 49, my.Shade.MEDIUM, 0x2);
      this.addButtonWithLightBelowDisplay(84, 29, "7", 35, my.Shade.MEDIUM, 0xa);
      this.addButtonWithLightBelowDisplay(90, 29, "8", 34, my.Shade.MEDIUM, 0x1);
      this.addButtonWithLightBelowDisplay(96, 29, "9", 25, my.Shade.MEDIUM, 0x9);

      // Large buttons on the main panel part
      this.addLargeButton         (108, 29, "Replace\nProgram", 29, my.Shade.MEDIUM);
      this.addLargeButtonWithLight(114, 29, "1-6",              30, my.Shade.MEDIUM, 0x0);
      this.addLargeButtonWithLight(120, 29, "7-12",             31, my.Shade.MEDIUM, 0x8);

      this.addLargeButton         (154, 29, "Select\nVoice", 5, my.Shade.MEDIUM);
      this.addLargeButton         (160, 29, "Copy",          9, my.Shade.MEDIUM);
      this.addLargeButton         (166, 29, "Write",         3, my.Shade.MEDIUM);
      this.addLargeButtonWithLight(172, 29, "Compare",       8, my.Shade.MEDIUM, 0x5);

      // Small buttons, main panel
      // -- Performance:
      this.addSmallButton(108, 20, "Patch\nSelect",   26, my.Shade.DARK, true);
      this.addSmallButton(114, 20, "MIDI",            27, my.Shade.DARK, true);
      this.addSmallButton(120, 20, "Effects",         28, my.Shade.DARK, true);

      this.addSmallButton(108, 13, "Key\nZone",       39, my.Shade.DARK, false);
      this.addSmallButton(114, 13, "Trans-\npose",    40, my.Shade.DARK, false);
      this.addSmallButton(120, 13, "Release",         41, my.Shade.DARK, false);

      this.addSmallButton(108,  6, "Volume",          36, my.Shade.DARK, false);
      this.addSmallButton(114,  6, "Pan",             37, my.Shade.DARK, false);
      this.addSmallButton(120,  6, "Timbre",          38, my.Shade.DARK, false);

      // Sequencer / System, both large and small buttons:
      if (hasSeq) {
        // The 'Master', 'Storage' and 'MIDI Control' buttons are small & at the to,
        // the sequencer buttons are big and at the bottom.
        this.addLargeButton(131, 29, "Rec",           19, my.Shade.MEDIUM);
        this.addLargeButton(137, 29, "Stop\n/Cont",   22, my.Shade.MEDIUM);
        this.addLargeButton(143, 29, "Play",          23, my.Shade.MEDIUM);

        this.addSmallButton(131, 20, "Click",         32, my.Shade.DARK, false);
        this.addSmallButton(137, 20, "Seq\nControl",  18, my.Shade.DARK, true);
        this.addSmallButton(143, 20, "Locate",        33, my.Shade.DARK, true);

        this.addSmallButton(131, 13, "Song",          60, my.Shade.DARK, false);
        this.addSmallButton(137, 13, "Seq",           59, my.Shade.DARK, false);
        this.addSmallButton(143, 13, "Track",         61, my.Shade.DARK, false);

        this.addSmallButton(131,  6, "Master",        20, my.Shade.LIGHT, true);
        this.addSmallButton(137,  6, "Storage",       21, my.Shade.LIGHT, false);
        this.addSmallButton(143,  6, "MIDI\nControl", 24, my.Shade.LIGHT, true);
      } else {
        // The 'Master', 'Storage' and 'MIDI Control' buttons are large & at the bottom,
        // and there are no sequencer buttons
        this.addLargeButton(131, 29, "Master",        20, my.Shade.LIGHT, true);
        this.addLargeButton(137, 29, "Storage",       21, my.Shade.LIGHT, false);
        this.addLargeButton(143, 29, "MIDI\nControl", 24, my.Shade.LIGHT, true);
      }

      // -- Programming:
      this.addSmallButton(154, 20, "Wave",             4, my.Shade.DARK, false);
      this.addSmallButton(160, 20, "Mod\nMixer",       6, my.Shade.DARK, false);
      this.addSmallButton(166, 20, "Program\nControl", 2, my.Shade.DARK, false);
      this.addSmallButton(172, 20, "Effects",          7, my.Shade.DARK, true);

      this.addSmallButton(154, 13, "Pitch",           11, my.Shade.DARK, false);
      this.addSmallButton(160, 13, "Pitch\nMod",      13, my.Shade.DARK, false);
      this.addSmallButton(166, 13, "Filters",         15, my.Shade.DARK, true);
      this.addSmallButton(172, 13, "Output",          17, my.Shade.DARK, true);

      this.addSmallButton(154,  6, "LFO",             10, my.Shade.DARK, true);
      this.addSmallButton(160,  6, "Env1",            12, my.Shade.DARK, true);
      this.addSmallButton(166,  6, "Env2",            14, my.Shade.DARK, true);
      this.addSmallButton(172,  6, "Env3",            16, my.Shade.DARK, true);

      // Display buttons - approximate:
      this.addSmallButton(32, 21, "", 50, my.Shade.DARK, false);
      this.addSmallButton(57, 21, "", 44, my.Shade.DARK, false);
      this.addSmallButton(82, 21, "", 45, my.Shade.DARK, false);

      this.addSmallButton(32,  4, "", 58, my.Shade.DARK, false);
      this.addSmallButton(57,  4, "", 42, my.Shade.DARK, false);
      this.addSmallButton(82,  4, "", 43, my.Shade.DARK, false);

      // Value slider
      var valueSlider = this.addSlider(-2.75, 4, 7, 22, 3, 0.7);

      // Increment and Decrement
      this.addIncDecButton(-12.5, 22, "\u25BC", 63, my.Shade.DARK, false);
      this.addIncDecButton(-12.5, 12, "\u25B2", 62, my.Shade.DARK, false);

      // Volume slider
      var volumeSlider = this.addSlider(-30, 4, 7, 22, 5, 1.0);

      var r = undefined;
      for (var i = 1; i < this.buttons.length; i++) {
        var button = this.buttons[i];
        if (button != null) {
          if (r != null) {
            r = r.union(button.rect);
          } else {
            r = button.rect;
          }
        }
      }
      r = r.union(valueSlider.rect);
      r = r.union(volumeSlider.rect);

      r.x -= 2;
      r.y -= 2;
      r.w += 4;
      r.h += 4;

      var viewBox = "" + r.x + " " + r.y + " " + r.w + " " + r.h;
      this.container.setAttribute("viewBox", viewBox);
    }

    my.Panel.prototype.sendString = function(s) {
      if (this.socket != undefined && this.socket.readyState == WebSocket.OPEN) {
        var b = new Uint8Array(s.length);
        for (var i = 0; i < s.length; i++) {
          b[i] = s.charCodeAt(i);
        }
        this.socket.send(b);
      }
    }

    my.Panel.prototype.onButtonPressed = function(button) {
      this.sendString("BD " + button.value);
    }

    my.Panel.prototype.onButtonReleased = function(button) {
      this.sendString("BU " + button.value);
    }

    my.Panel.prototype.onAnalogValueChanged = function(slider) {
      // 0.05 == 0; 0.95 == 760
      var value = (slider.value - 0.05) / 0.9;
      value = 760 * value;
      value = Math.round(Math.max(0, Math.min(1023, value)));
      var s = "A" + slider.channel + " " + value;

      // console.log(s);
      this.sendString(s);
    }

    my.Panel.prototype.handleDisplayData = function(data) {
      console.log("Handling display data " + data.length + " : " + data);
      for (var i = 0; i < data.length; i++) {
        var received = data[i];

        if (this.ignoreNext > 0) {
          console.log("skipping byte: 0x" + received.toString(16));
          this.ignoreNext--;
          continue;
        }

        console.log("handling byte: 0x" + received.toString(16));
        if (this.light) {
          var whichLight = received & 0x3f;
          var button = this.lightButtons[whichLight];
          if (button != null) {
            var state = (received & 0xc0);
            if (state == 0x80) {
              button.setLight(my.Light.ON);
            } else if (state == 0xc0) {
              button.setLight(my.Light.BLINK);
            } else {
              button.setLight(my.Light.OFF);
            }
          }
          this.light = false;
        } else if ((received >= 0x80) && (received < 0xd0)) {
          this.cursorY = ((received & 0x7f) >= 40) ? 1 : 0;
          this.cursorX = (received & 0x7f) % 40;
          this.blink = this.display.blink(this.cursorY, this.cursorX);
          this.underline = this.display.underline(this.cursorY, this.cursorX);
          console.log("moving to row " + this.cursorY + ", column " + this.cursorX);
        } else if (received >= 0xd0) {
          switch (received) {
          case 0xd0:  // blink start
            console.log("blink on");
            this.blink = true;
            break;

          case 0xd1:  // blink stop (cancel all attribs on VFX+)
            console.log("attrs off");
            this.blink = false;
            this.underline = false;
            break;

          case 0xd2:  // blinking underline?
            console.log("blinking underline on");
            this.blink = true;
            this.underline = true;
            break;

          case 0xd3:  // start underline
            console.log("underline on");
            this.underline = true;
            break;

          case 0xd6:  // clear screen
            console.log("clear screen");
            this.cursorX = this.cursorY = 0;
            this.blink = this.underline = false;
            this.display.clear();
            break;

          case 0xf5:  // save cursor position
            this.savedCursorX = this.cursorX;
            this.savedCursorY = this.cursorY;
            console.log("saving cursor position (row " + this.savedCursorY + ", col " + this.savedCursorX + ")");
            break;

          case 0xf6:  // restore cursor position
            this.cursorX = this.savedCursorX;
            this.cursorY = this.savedCursorY;
            this.blink = this.display.blink(this.cursorY, this.cursorX);
            this.underline = this.display.underline(this.cursorY, this.cursorX);
            console.log("restored cursor position (row " + this.savedCursorY + ", col " + this.savedCursorX + ")");
            break;

          case 0xff: // Specify a button light state
            this.light = true;
            break;

          default:
            console.log("Unexpected control code: " + received);
            break;
          }
        } else if ((received >= 0x20) && (received <= 0x5f)) {
          // var attrs = this.blink ? this.underline ? " with blink & underline" : " with blink" : this.underline ? " with underline" : "";
          // console.log("at (" + this.cursorY + ", " + this.cursorX + ") char " + received  + attrs);
          this.display.setChar(this.cursorY, this.cursorX, received, this.underline, this.blink);
          this.cursorX = Math.min(this.cursorX + 1, 39);
        } else {
          console.log("Unexpected byte: " + received.toString(16));
        }
      }
    }

    my.Panel.prototype.handleAnalogValue = function(data) {
      var s = String.fromCharCode.apply(null, data);
      console.log("Handling analog value: '" + s + "'");
      var parts = s.split(" ");
      if (parts.length == 2) {
        var channel = parseInt(parts[0]);
        var value = parseInt(parts[1]);

        var analogControl = this.analogControls[channel];
        if (analogControl != null) {
          if (analogControl instanceof my.Slider) {
            // 0.05 == 0; 0.95 == 760
            value = value / 760.0;
            value = 0.05 + 0.9 * value;
            analogControl.setValue(value);
          }
        }
      }
    }

    my.Panel.prototype.handleButtonState = function(data) {
      var s = String.fromCharCode.apply(null, data);
      var parts = s.split(" ");
      if (parts.length == 2) {
        var pressed = parts[0] == 'D';
        var number = parseInt(parts[1]);
        var button = this.buttons[number];
        if (button != null && button instanceof my.Button) {
          button.showPressed(pressed);
        }
      }
    }

    my.Panel.prototype.handleServerInformation = function(data) {
      var s = String.fromCharCode.apply(null, data);
      var parts = s.split(",");
      var keyboard = "none";
      var version = -1;
      if (parts.length == 2) {
        keyboard = parts[0];
        version = parseInt(parts[1]);
      }
      if (keyboard == this.keyboard && version == this.version) {
        // same keyboard type version - proceed!
        this.sendString("CA1B1D1"); // Send me analog data, buttons, and display data
      } else {
        // we need to reload, forcing a refresh from the server.
        document.location.reload(true);
      }
    }

    return my;
  })();
  