The template family tree is an attempt to ease the pain to write CPUs/drivers/devices
from scratch (useful especially for smaller targets).

===
Usage:
- Any "xxx" name is case-sensitive (i.e., XXX -> NAMEOFDEVICE, xxx -> nameofdevice);

===
License:
Copyright (c) 2014, Angelo Salese & the MAME team
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===
TODO:
- Define convention for template inputs (i.e. optimal for each host);
- Write template drivers for different endianesses;
- Write template header for drivers;
- Write tool program that auto-generate contents;
- Fix SCREEN_RAW_PARAMS, perhaps actually link to a framework class/macro list, a couple of references are:
 http://www.ntsc-tv.com/ntsc-index-02.htm
 http://www.intersil.com/content/dam/Intersil/documents/tb36/tb368.pdf
