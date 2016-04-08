// MAME-JavaScript function mappings
var JSMAME = JSMAME || {};
JSMAME.get_machine = Module.cwrap('_Z14js_get_machinev', 'number');
JSMAME.get_ui = Module.cwrap('_Z9js_get_uiv', 'number');
JSMAME.get_sound = Module.cwrap('_Z12js_get_soundv', 'number');
JSMAME.ui_set_show_fps = Module.cwrap('_ZN10ui_manager12set_show_fpsEb', '', ['number', 'number']);
JSMAME.ui_get_show_fps = Module.cwrap('_ZNK10ui_manager8show_fpsEv', 'number', ['number']);
JSMAME.sound_manager_mute = Module.cwrap('_ZN13sound_manager4muteEbh', '', ['number', 'number', 'number']);
JSMAME.sdl_pauseaudio = Module.cwrap('SDL_PauseAudio', '', ['number']);
var JSMESS = JSMAME;
