#pragma once

struct Hippieknob : RoundKnob 
{
    Hippieknob() 
    {
        setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/htLocal_hippieknob_orange.svg")) );
    }
};

struct Hippieknob_big: RoundKnob
{
    Hippieknob_big()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/htLocal_hippieknob_orange-big.svg")));
    }
};

struct Hippieswitch_orange : SvgSwitch
{
    Hippieswitch_orange()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/htLocal_hippieswitch_orange-left.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/htLocal_hippieswitch_orange-middle.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/htLocal_hippieswitch_orange-right.svg")));
        shadow->opacity = 0.0f;
        box.size.x = 20.0f;
        box.size.y = 20.0f;
    }
};

struct Hippieswitch_pushbutton : SvgSwitch
{
    Hippieswitch_pushbutton()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/htLocal_2wbutton_off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/htLocal_2wbutton_on.svg")));
        shadow->opacity = 0.0f;
    }
};

//-----------------------------------------------------
// Procedure:   MyPortInSmall
//
//-----------------------------------------------------
struct MyPortInSmall : SvgPort 
{
    MyPortInSmall() 
    {
        setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/htLocal_PortIn.svg")) );
        shadow->opacity = 0.0f;
    }
};

//-----------------------------------------------------
// Procedure:   MyPortOutSmall
//
//-----------------------------------------------------
struct MyPortOutSmall : SvgPort 
{
    MyPortOutSmall() 
    {
        setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/htLocal_PortOut.svg")) );
        shadow->opacity = 0.0f;
    }
};