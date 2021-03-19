# VCV-Modules-HTLocal

Modules
* [htFader / htFader-16](#htFader)
* [htAdder](#htAdder)

## htFader

<img src="res/htFader-ui.png~" width="168" height="250" align="left">

The fader is used to bring an audio signal in or out, or can alternatively also be used as CV. <br>
Based on a standard exponential function, the slope parameter can be used to adjust the characteristics of the fade. <br>
The fades can be triggered with an LED button or via trigger-in. <br><br>
Use the Fader-16 for big patches or save some CPU cycles with the 8 port version.

<br>
<br>
<br>
<br>

### inputs

When the inputs of a channel are connected the signal is used for the fades. <br>
If only one input is connected only the corresponding output will be active.

### outputs

Without a signal on the inputs the output will be between 0V and 10V.

### triggers

Press the buttons or provide a trigger pulse to toggle fades.

### fade time parameters

The fade in and fade out time can be set in seconds using the knobs.

### slope parameter

As indicated at the top of the column the slope parameter defines how the fade curve will "feel".
Values >1 will feel more natural, whereas values <1 will give you a fade-in that can punch though existing sounds in the frequency range earlier.
A value of 0 would be linear.

V = -1 | V = -0.7 | V = 0 | V = 1
-------|----------|-------|------
<img src="res/v-1.PNG" align="left" width="200" height="200"> | <img src="res/v-p7.PNG" align="left" width="200" height="200"> | <img src="res/v0.PNG" align="left" width="200" height="200"> | <img src="res/v1.PNG" align="left" width="200" height="200">

## htAdder

<img src="res/htAdder-ui.png~" width="68" height="250" align="left">

The htAdder is an implementation of the Doepfler A-185-2 Precision CV Adder, plus the additional feature of controlling the mode switches with a CV input.<br><br>
There are 5 CV inputs which can, depending on the mode (-/0/+) be subtracted, bypassed or added on top of each other. The resulting sequence will be sent to the output.<br>
If there is no input present, but a mode of either + or - is selected the switch becomes an octave switch, since the voltage for each channel (except ch.1) is +/-1V by default.

<br>
<br>

### inputs

The first input port has, other than the other 4, a default voltage of 1V * the knob value, that means a total of 0-1V when there is no input connected. <br>
If a signal is connected to the first input a fraction of 1V (* knob value) will be added or subtracted to / from it. This can be used as a fine tune. <br><br>

All other inputs default to a voltage of 1V or the CV that is coming in through the input.<br>
The fine tune knob does not work on them.<br><br>

### outputs

The output is the total of all inputs that have selected a mode other than 0 (bypass).<br>
There is also an inverted output port available below the original output.

### mode switch

A mode switch can have 3 states:<br>
0 (bypass) - Does not add or subtract the input CV or default voltage<br>
\+ (add) - Adds the input CV or the default voltage to the total CV<br>
\- (sub) - Subtracts the input CV or the default voltage from the total CV<br>

### mode switch CV

A mode switch can be itself controlled by CV as follows:<br>
0V - select bypass<br>
&gt;0V - select add mode<br>
<0V - select sub mode<br>
