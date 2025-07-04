// license:BSD-3-Clause
// copyright-holders:Grant Galitz, Katelyn Gadd
/***************************************************************************

	JSMAME web audio backend v0.4

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
	//Workarounds for browser issues:
	initializeWatchDog();
};

function initializeWatchDog() {
	watchDogDateLast = (new Date()).getTime();
	if (watchDogTimerEvent === null) {
		watchDogTimerEvent = setInterval(function () {
			var timeDiff = (new Date()).getTime() - watchDogDateLast;
			if (timeDiff > 500) {
				//WORKAROUND FOR FIREFOX BUG:
				//TODO: decide if we want to user agent sniff Firefox here,
				//since Google Chrome doesn't need this:
				disconnect_old_event();
				init_event();

				//Work around autoplay restrictions in Chrome 71+ https://developers.google.com/web/updates/2017/09/autoplay-policy-changes#webaudio
				if (context) {
					context.resume();
				}
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

function stream_sink_update (
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
			// divide by sizeof(int16_t) since pBuffer is offset
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
	//Pad with latest if we're underrunning:
	var idx = (index == 0 ? bufferSize : index) - 1;
	while (index < 4096) {
		buffers[0][index] = buffers[0][idx];
		buffers[1][index++] = buffers[1][idx];
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
	stream_sink_update: stream_sink_update,
	get_context: get_context,
	sample_count: sample_count
};

})();

window.jsmame_stream_sink_update = jsmame_web_audio.stream_sink_update;
window.jsmame_sample_count = jsmame_web_audio.sample_count;
