# FFT-based Audio Visualization

Developed by Jonathan MacArt 2012-2020

Dependencies:

- [FastLED](https://github.com/FastLED/FastLED)
- [fix_fft](https://www.arduino.cc/reference/en/libraries/fix_fft/)

Tested using an Arduino Uno and WS2811 LEDs, but other combinations are possible. In-use examples:

- https://www.youtube.com/watch?v=B0CPVkpbbqc
- https://www.youtube.com/watch?v=mZiZcwaqly0


## Summary

This provides a ready-to-use Arduino program for audio visualization via the fast Fourier transform (FFT). Key features are:

- The program reads pin A0 for an audio signal, which it expects to be conditioned to 2.5 V mean with +/- 2.5 V peak-to-peak (assuming you use a 5 V microcontroller). I describe my signal-conditioning circuit below.

- The `fix_fft` library does most of the heavy lifting; it computes a fixed-point-precision, in-place FFT with significantly higher performance than a floating-point FFT.

- The program computes an exponential moving average of the FFT bins and sends this to user-defined LED bins using `FastLED`.

- The program has two modes: active, FFT-based visualization, which it enters when an input signal is detected, and "standby" pre-programmed visualizations, to which it reverts otherwise. My principal use has been Christmas tree lighting, so the included visualizations are mostly along these lines. Others are certainly possible; just use the included ones as templates.


## Signal conditioning

Your audio signal most likely has a mean of 0 V; we need to shift this to the center of your ADC's range (e.g., 2.5 V for 5-volt devices). Unless you're using a 1.6-volt microcontroller, we also likely need to apply some gain. Headphone preamps, for example, seem to output about 0.6 V peak-to-peak.

The following circuit applies a gain of 6x and shifts the signal to approximately 2.5 V mean. I use a Texas Instruments TLE2021 Excalibur Op-Amp, but others will likely work too.

![circuit](./media/circuit.pdf =300x)

Here's the built-up circuit:

![circuit](./media/mod_circuit.jpg =300x)

And a final shot showing the conditioned output of a 1 kHz sine wave:

![circuit](./media/mod_scope.jpg =300x)