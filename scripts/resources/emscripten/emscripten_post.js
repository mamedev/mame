// MAME-JavaScript function mappings
var JSMAME = JSMAME || {};
JSMAME.get_machine = Module.cwrap('_ZN15running_machine30emscripten_get_running_machineEv', 'number');
JSMAME.get_ui = Module.cwrap('_ZN15running_machine17emscripten_get_uiEv', 'number');
JSMAME.get_sound = Module.cwrap('_ZN15running_machine20emscripten_get_soundEv', 'number');
JSMAME.ui_set_show_fps = Module.cwrap('_ZN15mame_ui_manager12set_show_fpsEb', '', ['number', 'number']);
JSMAME.ui_get_show_fps = Module.cwrap('_ZNK15mame_ui_manager8show_fpsEv', 'number', ['number']);
JSMAME.sound_manager_mute = Module.cwrap('_ZN13sound_manager4muteEbh', '', ['number', 'number', 'number']);
JSMAME.sdl_pauseaudio = Module.cwrap('SDL_PauseAudio', '', ['number']);
JSMAME.sdl_sendkeyboardkey = Module.cwrap('SDL_SendKeyboardKey', '', ['number', 'number']);

JSMAME.soft_reset = Module.cwrap('_ZN15running_machine21emscripten_soft_resetEv', null);
JSMAME.hard_reset = Module.cwrap('_ZN15running_machine21emscripten_hard_resetEv', null);
JSMAME.exit = Module.cwrap('_ZN15running_machine15emscripten_exitEv', null, []);
JSMAME.save = Module.cwrap('_ZN15running_machine15emscripten_saveEPKc', null, ['string']);
JSMAME.load = Module.cwrap('_ZN15running_machine15emscripten_loadEPKc', null, ['string']);

var JSMESS = JSMAME;
