import threading
import numpy as np

try:
  import sounddevice as sd
except Exception as e:
    raise RuntimeError("sounddevice required: pip install sounddevice") from e


class ElectricMotorSynth:
    """Continuous synth that models a futuristic electric motor.
    set_level(x) with x in [0.0, 1.0] controls pitch/amplitude/texture.
    """

    def __init__(self, samplerate=48000, device=None, blocksize=1024):
        self.sr = int(samplerate)
        self.device = device
        self.block = int(blocksize)
        self._phase = 0.0
        self._phase_h = 0.0
        self._lock = threading.Lock()
        self._target = 0.0
        self._level = 0.0  # smoothed level
        self._running = True

        self.stream = sd.OutputStream(
            samplerate=self.sr,
            blocksize=self.block,
            channels=1,
            dtype='float32',
            callback=self._callback,
            device=self.device,
        )
        self.stream.start()

    def set_level(self, v: float):
        """Set throttle/engagement level 0..1"""
        v = float(v)
        if v < 0.0:
            v = 0.0
        if v > 1.0:
            v = 1.0
        with self._lock:
            self._target = v

    def _callback(self, outdata, frames, time, status):
        if status:
            # ignore underrun warnings for now
            pass
        t = (np.arange(frames) + 0).astype(np.float32) / self.sr

        with self._lock:
            target = self._target

        # smooth the level to avoid clicks
        self._level += (target - self._level) * 0.02  # smoothing constant

        # base frequency mapping (Hz): low hum -> high whine
        base_freq = 80.0 + self._level * 1200.0  # 80 .. 1280 Hz
        # add a gear-like pitch curve (exponential-ish)
        pitch = base_freq * (1.0 + 0.5 * (self._level ** 1.5))

        # small LFO to simulate rotor modulation, rate increases with level
        lfo_rate = 0.5 + 8.0 * self._level
        lfo = np.sin(2.0 * np.pi * (self._phase_h + t * lfo_rate)).astype(np.float32)
        self._phase_h += frames * (lfo_rate / self.sr)
        self._phase_h %= 1.0

        # FM index grows with level for gritty texture
        fm_index = 0.5 + 8.0 * self._level

        # create carrier with mild FM by LFO
        carrier_phase = 2.0 * np.pi * (self._phase + np.cumsum((pitch * (1.0 + fm_index * lfo)) / self.sr))
        # integrate phase for continuity fallback if cumsum huge
        carrier = np.sin(carrier_phase).astype(np.float32)

        # update phase for next callback
        self._phase += (pitch.mean() * frames) / self.sr
        self._phase %= 1.0

        # add harmonic content: sum of a few harmonics with decreasing amplitude
        harmonics = carrier * (0.6 + 0.25 * np.sin(2.0 * np.pi * (self._phase + t * pitch * 2.0)))
        harmonics += 0.3 * np.sin(2.0 * np.pi * (2.0 * pitch) * t + 0.2 * lfo)
        harmonics += 0.15 * np.sin(2.0 * np.pi * (3.0 * pitch) * t + 0.4 * lfo)

        # add high-frequency shimmer/noise that grows with level
        noise = (np.random.normal(0.0, 1.0, frames).astype(np.float32))
        hf = 0.002 * noise * (1.0 + 8.0 * self._level)

        # amplitude envelope and brightness
        amp = 0.02 + 0.40 * (self._level ** 1.2)  # overall loudness
        bright = 1.0 + 2.5 * self._level

        out = amp * (harmonics * bright + hf)

        # soft clip to avoid harsh distortion
        out = np.tanh(out).astype('float32')

        # write to outdata (mono)
        outdata[:, 0] = out

    def stop(self):
        self._running = False
        try:
            self.stream.stop()
            self.stream.close()
        except Exception:
            pass

    def __del__(self):
        try:
            self.stop()
        except Exception:
            pass