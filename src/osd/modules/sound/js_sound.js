// license:BSD-3-Clause
// copyright-holders:Grant Galitz, Katelyn Gadd
/***************************************************************************

	JSMAME web audio backend

	Original by katelyn gadd - kg at luminance dot org ; @antumbral on twitter
	Substantial changes by taisel

	Playback runs on an AudioWorklet ring (the browser's audio thread, which
	main-thread load cannot starve) where available, with a ScriptProcessorNode
	fallback.  The emulator paces itself on the reported ring level.

***************************************************************************/

var jsmame_web_audio = (function () {

var TARGET_FRAMES = 4096;     // steady-state ring level the pacer aims for (~85 ms)
var sampleScale = 32766;

var context = null;
var gain_node = null;
var eventNode = null;
// 16384 frames (~340 ms at 48 kHz): wide enough to bridge one long
// main-loop callback during input-event storms while the pacer still
// regulates the steady-state level down to TARGET_FRAMES
var inputBuffer = new Float32Array(32768);
var bufferSize = 32768;
var start = 0;
var rear = 0;
// frames currently stored in the ring, maintained exactly on every write,
// read, drop and trim.  Pointer arithmetic cannot tell a completely full
// ring (rear == start after wrapping) from an empty one; the pacer
// reading a saturated ring as "empty" drives production into a permanent
// overflow/drop/underrun loop (audible as a stuck repeating snippet).
var avail = 0;
var CAPACITY = bufferSize / 2;
var lastL = 0;
var lastR = 0;
var underrunCount = 0;
var droppedCount = 0;
var watchDogDateLast = null;
var watchDogTimerEvent = null;
var padRun = 0;
var ffActive = false;
var lastKeyAt = 0;
if (typeof window !== "undefined")
	window.addEventListener("keydown", function () { lastKeyAt = Date.now(); }, true);

// ---- AudioWorklet sink (primary) ------------------------------------
// The ScriptProcessorNode above renders on the main thread and starves
// together with the emulation loop under input-event load (the browser
// then repeats its last rendered quantum: audible as a looping snippet).
// The worklet renders on the browser's audio thread, which main-thread
// load cannot starve, so playback continuity no longer depends on the
// emulator leaving gaps.  Feeding goes through port messages (no
// SharedArrayBuffer: the page is served without COOP/COEP headers).
var workletNode = null;
var workletReady = false;
// latest ring level / counters reported by the worklet
var wlAvail = 0;
var wlUnderruns = 0;
var wlDropped = 0;
var wlReportAt = 0;

var WORKLET_SRC = `
class MAMERingProcessor extends AudioWorkletProcessor {
	constructor() {
		super();
		this.cap = 16384;                 // frames
		this.buf = new Float32Array(this.cap * 2);
		this.start = 0;
		this.avail = 0;
		this.lastL = 0;
		this.lastR = 0;
		this.padRun = 0;
		this.ff = false;
		this.underruns = 0;
		this.dropped = 0;
		this.padFrames = 0;
		this.port.onmessage = (e) => {
			var d = e.data;
			if (d.cmd === "ff") {
				this.ff = d.on;
				return;
			}
			if (d.cmd === "trim") {
				var drop = this.avail - d.frames;
				if (drop > 0) {
					this.start = (this.start + drop * 2) % (this.cap * 2);
					this.avail -= drop;
					this.dropped += drop;
				}
				return;
			}
			// audio chunk: interleaved float frames
			var f = d.samples;
			if (this.ff) {
				// fast forward: keep only the freshest sliver
				this.start = 0;
				this.avail = 0;
				this.padRun = 0;
			}
			for (var i = 0; i + 1 < f.length; i += 2) {
				if (this.avail === this.cap) {
					this.start += 2;
					if (this.start === this.cap * 2) this.start = 0;
					this.dropped++;
				} else {
					this.avail++;
				}
				var w = (this.start + this.avail * 2 - 2) % (this.cap * 2);
				this.buf[w] = f[i];
				this.buf[w + 1] = f[i + 1];
			}
		};
	}
	process(inputs, outputs) {
		var out = outputs[0];
		var left = out[0], right = out[1] || out[0];
		var quantum = left.length;
		var index = 0;
		while (index < quantum && this.avail !== 0) {
			this.lastL = left[index] = this.buf[this.start];
			this.lastR = right[index] = this.buf[this.start + 1];
			this.start += 2;
			if (this.start === this.cap * 2) this.start = 0;
			this.avail--;
			index++;
		}
		if (index < quantum) {
			// brief fade of the last frame, then hard silence (see the
			// script-processor pad above for why the decay must be short)
			this.underruns++;
			while (index < quantum) {
				var scale = this.padRun < 192 ? (1 - this.padRun / 192) : 0;
				left[index] = this.lastL * scale;
				right[index] = this.lastR * scale;
				this.padRun++;
				index++;
			}
		} else {
			this.padRun = 0;
		}
		this.port.postMessage({
			level: this.avail,
			underruns: this.underruns,
			dropped: this.dropped,
			ff: this.ff
		});
		return true;
	}
}
registerProcessor("mame-ring", MAMERingProcessor);
`;

function init_worklet() {
	if (workletNode || !context || !context.audioWorklet)
		return;
	// Firefox delivers port messages to worklets with load-sensitive lag,
	// so the audio-thread ring drains between delayed feeds (measured:
	// constant underruns with the pacer pinned fast).  Firefox instead
	// keeps the script processor, whose delivery stoppage the watchdog
	// above exists to fix
	var isFirefox = typeof navigator != "undefined" &&
		navigator.userAgent.indexOf("Firefox") !== -1;
	if (isFirefox)
		return;
	try {
		var blob = new Blob([WORKLET_SRC], { type: "application/javascript" });
		var url = URL.createObjectURL(blob);
		context.audioWorklet.addModule(url).then(function () {
			workletNode = new AudioWorkletNode(context, "mame-ring", {
				outputChannelCount: [2]
			});
			workletNode.port.onmessage = function (e) {
				wlAvail = e.data.level;
				wlUnderruns = e.data.underruns;
				wlDropped = e.data.dropped;
				wlReportAt = Date.now();
			};
			workletNode.connect(gain_node);
			workletReady = true;
			// hand playback over now: do this here, not from the
			// script processor's own callback — Firefox stops
			// delivering onaudioprocess entirely (the bug its watchdog
			// exists for), so that callback may never fire again and
			// both sinks would stay connected, the worklet starving
			if (eventNode) {
				try { eventNode.disconnect(); } catch (e) {}
				eventNode.onaudioprocess = null;
				eventNode = null;
			}
		}).catch(function (e) {
			// stay on the script-processor fallback
			if (typeof console !== "undefined")
				console.warn("AudioWorklet unavailable, using ScriptProcessorNode: " + e);
		});
	} catch (e) {
		// stay on the script-processor fallback
	}
};

function lazy_init () {
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
	if (typeof context.createScriptProcessor == "function") {
		//Current standard compliant way:
		eventNode = context.createScriptProcessor(2048, 0, 2);
	}
	else {
		//Deprecated way:
		eventNode = context.createJavaScriptNode(2048, 0, 2);
	}
	//Make our tick function the audio callback function:
	eventNode.onaudioprocess = tick;
	//Connect stream to volume control node:
	eventNode.connect(gain_node);
	// render on the audio thread when the browser supports it; the
	// script processor stays connected as the fallback
	init_worklet();

	//Workarounds for browser issues:
	initializeWatchDog();
};

function initializeWatchDog() {
	watchDogDateLast = (new Date()).getTime();
	if (watchDogTimerEvent === null) {
		// If onaudioprocess has not run recently, the browser's audio
		// task queue for this node is starved (Firefox stops delivering
		// events entirely; Chromium deprioritizes them under main-thread
		// load).  Recreating the node re-registers it with the audio
		// graph and restarts delivery.
		//
		// Without hysteresis this becomes a self-sustaining recreate
		// storm while the main thread is loaded: the watchdog interval
		// itself only fires in the rare breaths, and each recreate adds
		// work.  So: recreate at most once per few seconds, and only
		// after a genuinely long silence (longer on Chromium, where
		// delivery is merely degraded rather than stopped).
		var isFirefox = typeof navigator != "undefined" &&
			navigator.userAgent.indexOf("Firefox") !== -1;
		var staleThreshold = isFirefox ? 500 : 1500;
		var minInterval = isFirefox ? 1000 : 3000;
		var lastRecreate = 0;
		var recreateCount = 0;
		watchDogTimerEvent = setInterval(function () {
			if (workletReady)
				return;
			var now = (new Date()).getTime();
			var timeDiff = now - watchDogDateLast;
			if (timeDiff > staleThreshold && now - lastRecreate > minInterval) {
				lastRecreate = now;
				disconnect_old_event();
				init_event();

				// Work around autoplay restrictions in Chrome 71+ https://developers.google.com/web/updates/2017/09/autoplay-policy-changes#webaudio
				if (context) {
					context.resume();
				}
			}
		}, 250);
	}
};

function disconnect_old_event() {
	if (!eventNode)
		return;
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

	var frames = samples_this_frame | 0;
	if (frames <= 0) return;

	if (ffActive) {
		// fast forward: each pushed chunk replaces the whole ring, so the
		// sink always plays the freshest sliver of the sped-up stream.
		// Rendering 5-14x realtime audio would cost most of the fast
		// forward budget; this keeps the sound indicative (fragmented,
		// stepping pitch) at a fraction of the work
		start = 0;
		rear = 0;
		avail = 0;
		padRun = 0;
	}
	if (workletReady) {
		// post an interleaved copy to the audio-thread ring; the
		// script-processor path above is kept fed as a fallback but its
		// node is disconnected once the worklet is running
		var woffset = ((pBuffer / 2) | 0);
		var out = new Float32Array(frames * 2);
		for (var q = 0; q < frames; q++) {
			out[q * 2] = HEAP16[woffset + q * 2] / sampleScale;
			out[q * 2 + 1] = HEAP16[woffset + q * 2 + 1] / sampleScale;
		}
		workletNode.port.postMessage({ samples: out, ff: ffActive });
		workletNode.port.postMessage({ cmd: "ff", on: ffActive });
		return;
	}
	var offset =
		// divide by sizeof(int16_t) since pBuffer is offset
		//  in bytes
		((pBuffer / 2) | 0);

	for (
		var j = 0;
		j < frames;
		j++
	) {
		inputBuffer[rear++] = HEAP16[offset + j * 2] / sampleScale;
		inputBuffer[rear++] = HEAP16[offset + j * 2 + 1] / sampleScale;
		if (rear == bufferSize) {
			rear = 0;
		}
		if (avail == CAPACITY) {
			// ring full: drop the oldest frame so playback stays recent
			start += 2;
			if (start == bufferSize) {
				start = 0;
			}
			droppedCount++;
		} else {
			avail++;
		}
	}
};


function tick (event) {
	//Find all output channels:
	for (var bufferCount = 0, buffers = []; bufferCount < 2; ++bufferCount) {
		buffers[bufferCount] = event.outputBuffer.getChannelData(bufferCount);
	}
	//Copy samples from the input buffer to the Web Audio API:
	var quantum = buffers[0].length;
	var index = 0;
	while (index < quantum && avail != 0) {

		lastL = buffers[0][index] = inputBuffer[start++];
		lastR = buffers[1][index] = inputBuffer[start++];
		avail--;
		index++;
		if (start == bufferSize) {
			start = 0;
		}
	}
	if (index < quantum) {
		// brief linear fade of the last played frame, then hard silence:
		// the browser delivers backlogged quanta in one batch under load
		// and any longer decay of the last sample is audible as a loop
		underrunCount++;
		while (index < quantum) {
			var scale = padRun < 192 ? (1 - padRun / 192) : 0;
			buffers[0][index] = lastL * scale;
			buffers[1][index] = lastR * scale;
			padRun++;
			index++;
		}
	} else {
		padRun = 0;
	}
	//Deep inside the bowels of vendors bugs,
	//we're using watchdog for a firefox bug,
	//where the user agent decides to stop firing events
	//if the user agent lags out due to system load.
	//Don't even ask....
	watchDogDateLast = (new Date()).getTime();
};

function get_sample_rate () {
	lazy_init();
	if (!context) return 44100;
	return context.sampleRate | 0;
};

function buffered_ms () {
	if (workletReady) {
		if (!context || context.state !== "running")
			return -1;
		return (wlAvail * 1000 / context.sampleRate) | 0;
	}
	// Milliseconds of audio sitting in the ring, for the emulator main
	// loop's pacing feedback.  -1 when there is no running clock yet.
	if (!context || context.state !== "running") {
		return -1;
	}
	// avail is exact even when the ring is completely full (where pointer
	// arithmetic would read zero)
	return (avail * 1000 / context.sampleRate) | 0;
};

function target_ms () {
	if (!context) return 0;
	var frames = TARGET_FRAMES;
	// key storms (OS autorepeat at 50-60 Hz) starve the main loop into
	// bursts separated by 250 ms gaps of no production at all; a target
	// level sized for steady callbacks underruns in every gap and the
	// underrun pad then re-triggers on each burst (audible as a loop).
	// While keys are active, aim near the full ring instead and let the
	// emulator's rate controller fill it; latency during a held key is
	// irrelevant compared to broken audio
	if (Date.now() - lastKeyAt < 600)
		frames = CAPACITY - TARGET_FRAMES;
	return (frames * 1000 / context.sampleRate) | 0;
};

function trim_to_target () {
	if (workletReady) {
		workletNode.port.postMessage({ cmd: "trim", frames: TARGET_FRAMES });
		return;
	}
	// Drop buffered audio back to the steady-state level (called when fast
	// forward ends, so latency does not linger after unthrottled flooding)
	var drop = avail - TARGET_FRAMES;
	if (drop > 0) {
		start = (start + drop * 2) % bufferSize;
		avail -= drop;
		droppedCount += drop | 0;
	}
};

function audio_stats () {
	return {
		mode: workletReady ? "worklet" : (eventNode ? "script-processor" : "none"),
		buffered_ms: buffered_ms(),
		target_ms: target_ms(),
		underruns: workletReady ? wlUnderruns : underrunCount,
		dropped: workletReady ? wlDropped : droppedCount,
		state: context ? context.state : "none"
	};
};




function set_fastforward (on) {
	ffActive = !!on;
};

return {
	stream_sink_update: stream_sink_update,
	get_sample_rate: get_sample_rate,
	buffered_ms: buffered_ms,
	target_ms: target_ms,
	trim_to_target: trim_to_target,
	audio_stats: audio_stats,
	set_fastforward: set_fastforward
};

})();

if (typeof window !== "undefined") {
	window.jsmame_stream_sink_update = jsmame_web_audio.stream_sink_update;
	window.jsmame_get_audio_rate = jsmame_web_audio.get_sample_rate;
	window.jsmame_audio_buffered_ms = jsmame_web_audio.buffered_ms;
	window.jsmame_audio_target_ms = jsmame_web_audio.target_ms;
	window.jsmame_audio_trim = jsmame_web_audio.trim_to_target;
	window.jsmame_audio_stats = jsmame_web_audio.audio_stats;
	window.jsmame_audio_set_ff = jsmame_web_audio.set_fastforward;
}
