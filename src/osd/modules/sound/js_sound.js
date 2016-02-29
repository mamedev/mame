// license:BSD-3-Clause
// copyright-holders:Grant Galitz, Katelyn Gadd
/***************************************************************************

	JSMAME web audio backend v0.3

	Original by katelyn gadd - kg at luminance dot org ; @antumbral on twitter
	Substantial changes by taisel

***************************************************************************/

var jsmame_web_audio = (function () {

var context = null;
var gain_node = null;
var eventNode = null;
var sampleScale = 32766;
var inputBuffer = new Float32Array(44100);
var bufferSize = 44100;
var start = 0;
var rear = 0;
var watchDogDateLast = null;
var watchDogTimerEvent = null;

function lazy_init () {
	//Make
	if (context) {
		//Return if already created:
		return;
	}
	if (typeof AudioContext != "undefined") {
		//Standard context creation:
		context = new AudioContext();
	}
	else if (typeof webkitAudioContext != "undefined") {
		//Older webkit context creation:
		context = new webkitAudioContext();
	}
	else {
		//API not found!
		return;
	}
	//Generate a volume control node:
	gain_node = context.createGain();
	//Set initial volume to 1:
	gain_node.gain.value = 1.0;
	//Connect volume node to output:
	gain_node.connect(context.destination);
	//Initialize the streaming event:
	init_event();
};

function init_event() {
	//Generate a streaming node point:
	if (typeof context.createScriptProcessor == "function") {
		//Current standard compliant way:
		eventNode = context.createScriptProcessor(4096, 0, 2);
	}
	else {
		//Deprecated way:
		eventNode = context.createJavaScriptNode(4096, 0, 2);
	}
	//Make our tick function the audio callback function:
	eventNode.onaudioprocess = tick;
	//Connect stream to volume control node:
	eventNode.connect(gain_node);
	//WORKAROUND FOR FIREFOX BUG:
	initializeWatchDogForFirefoxBug();
};

function initializeWatchDogForFirefoxBug() {
	//TODO: decide if we want to user agent sniff firefox here,
	//since Google Chrome doesn't need this:
	watchDogDateLast = (new Date()).getTime();
	if (watchDogTimerEvent === null) {
		watchDogTimerEvent = setInterval(function () {
			var timeDiff = (new Date()).getTime() - watchDogDateLast;
			if (timeDiff > 500) {
				disconnect_old_event();
				init_event();
			}
		}, 500);
	}
};

function disconnect_old_event() {
	//Disconnect from audio graph:
	eventNode.disconnect();
	//IIRC there was a firefox bug that did not GC this event when nulling the node itself:
	eventNode.onaudioprocess = null;
	//Null the glitched/unused node:
	eventNode = null;
};

function set_mastervolume (
	// even though it's 'attenuation' the value is negative, so...
	attenuation_in_decibels
) {
	lazy_init();
	if (!context) return;

	// http://stackoverflow.com/questions/22604500/web-audio-api-working-with-decibels
	// seemingly incorrect/broken. figures. welcome to Web Audio
	// var gain_web_audio = 1.0 - Math.pow(10, 10 / attenuation_in_decibels);

	// HACK: Max attenuation in JSMESS appears to be 32.
	// Hit ' then left/right arrow to test.
	// FIXME: This is linear instead of log10 scale.
	var gain_web_audio = 1.0 + (+attenuation_in_decibels / +32);
	if (gain_web_audio < +0)
		gain_web_audio = +0;
	else if (gain_web_audio > +1)
		gain_web_audio = +1;

	gain_node.gain.value = gain_web_audio;
};

function update_audio_stream (
	pBuffer,           // pointer into emscripten heap. int16 samples
	samples_this_frame // int. number of samples at pBuffer address.
) {
	lazy_init();
	if (!context) return;

	for (
		var i = 0,
		l = samples_this_frame | 0;
		i < l;
		i++
	) {
		var offset =
			// divide by sizeof(INT16) since pBuffer is offset
			//  in bytes
			((pBuffer / 2) | 0) +
			((i * 2) | 0);

		var left_sample = HEAP16[offset];
		var right_sample = HEAP16[(offset + 1) | 0];

		// normalize from signed int16 to signed float
		var left_sample_float = left_sample / sampleScale;
		var right_sample_float = right_sample / sampleScale;

		inputBuffer[rear++] = left_sample_float;
		inputBuffer[rear++] = right_sample_float;
		if (rear == bufferSize) {
			rear = 0;
		}
		if (start == rear) {
			start += 2;
			if (start == bufferSize) {
				start = 0;
			}
		}
	}
};
function tick (event) {
	//Find all output channels:
	for (var bufferCount = 0, buffers = []; bufferCount < 2; ++bufferCount) {
		buffers[bufferCount] = event.outputBuffer.getChannelData(bufferCount);
	}
	//Copy samples from the input buffer to the Web Audio API:
	for (var index = 0; index < 4096 && start != rear; ++index) {
		buffers[0][index] = inputBuffer[start++];
		buffers[1][index] = inputBuffer[start++];
		if (start == bufferSize) {
			start = 0;
		}
	}
	//Pad with silence if we're underrunning:
	while (index < 4096) {
		buffers[0][index] = 0;
		buffers[1][index++] = 0;
	}
	//Deep inside the bowels of vendors bugs,
	//we're using watchdog for a firefox bug,
	//where the user agent decides to stop firing events
	//if the user agent lags out due to system load.
	//Don't even ask....
	watchDogDateLast = (new Date()).getTime();
}

function get_context() {
	return context;
};

function sample_count() {
	//TODO get someone to call this from the emulator,
	//so the emulator can do proper audio buffering by
	//knowing how many samples are left:
	if (!context) {
		//Use impossible value as an error code:
		return -1;
	}
	var count = rear - start;
	if (start > rear) {
		count += bufferSize;
	}
	return count;
}

return {
	set_mastervolume: set_mastervolume,
	update_audio_stream: update_audio_stream,
	get_context: get_context,
	sample_count: sample_count
};

})();

window.jsmame_set_mastervolume = jsmame_web_audio.set_mastervolume;
window.jsmame_update_audio_stream = jsmame_web_audio.update_audio_stream;
window.jsmame_sample_count = jsmame_web_audio.sample_count;
