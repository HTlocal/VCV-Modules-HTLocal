# VCV-Modules-HTLocal

## htFader

<img src="res/htFader-ui.png" width="168" height="250" align="left">

The fader is used to bring an audio signal in or out, or can alternatively also be used as CV. <br>
Based on a standard exponential function, the slope parameter can be used to adjust the characteristics of the fade. <br>
The fades can be triggered with an LED button or via trigger-in.

<br>
<br>
<br>
<br>
<br>
<br>
<br>

### inputs

When the inputs of a channel are connected the signal is used for the fades. <br>
If only one input is connected only the corresponding output will be active.

### triggers

Press the buttons or provide a trigger pulse to toggle fades.

### outputs

Without a signal on the inputs the output will be between 0V and 10V.

### fade time parameters

The fade in and fade out time can be set in seconds using the knobs.

### slope parameter

As indicated at the top of the column the slope parameter defines how the fade curve will "feel".
Values >1 will feel more natural, whereas values <1 will give you a fade-in that can punch though existing sounds in the frequency range earlier.
A value of 0 would be linear.

V = -1 | V = -0.7 | V = 0 | V = 1
-------|----------|-------|------
<img src="res/v-1.PNG" align="left" width="200" height="200"> | <image here> | <image here> | <img src="res/v1.PNG" align="left" width="200" height="200">
