# VCV-Modules-HTLocal

## htFader

The fader allows you to precisely time a fade-in or fade out. Depending on the signal or the mix the slope can be adjusted per channel.
The fades can be triggered with an LED button or via trigger-in.

### inputs connected

When the inputs of a channel are connected this signal is attenuated for the fade modes and sent to the outputs.

### only outputs

When the inputs of a channel are NOT connected the fade will be a voltage between 0-10V.

### slope parameter

Sometimes when you want to bring new sounds into a mix the threshold until you hear them can be subjectively a big higher. To compensate for that you can bring in the sound a bit earlier in the fade by adjusting the slope parameter to a value between -1 and 0.
For the opposite, to fade in slower in the beginning and faster in the end, adjust between 0 and 1.
The setting is available for every channel, defaulting to -0.7.

V = -1 | V = -0.7 | V = 0 | V = 1
-------|----------|-------|------
<image here> | <image here> | <image here> | <image here>
