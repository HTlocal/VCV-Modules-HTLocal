#include "htLocal.hpp"
#include "logger.hpp"

#define nCHANNELS 5

struct htAdder : Module
{
    enum ParamIds
    {
        PARAM_ATTENUATE,
        PARAM_ADDER_MODE,
        nPARAMS = PARAM_ADDER_MODE + nCHANNELS
    };

    enum InputIds
    {
        IN_SIGNAL,
        IN_MODE_CV = IN_SIGNAL + nCHANNELS,
        nINPUTS = IN_MODE_CV + nCHANNELS
    };

    enum OutputIds
    {
        OUT_AUDIOL,
        OUT_AUDIOR,
        nOUTPUTS
    };

    enum LightIds
    {
        nLIGHTS
    };

    bool            m_bInitialized  = false;
    Label* m_pTextLabel             = NULL;

    htAdder() {
        config(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS);
        
        // attenuator knob
        configParam(PARAM_ATTENUATE, 0.f, 1.f, 1.f, "Attenuate Input 1");

        // mode switches 
        for (int ch = 0; ch < nCHANNELS; ch++)
        {
            configParam(PARAM_ADDER_MODE + ch, -1.0f, 1.0f, 0.0f, "Substract, bypass or add mode");
        }
    }

    // knob that updates the label
    struct attenuateKnob : Hippieknob_big
    {
        htAdder *adder;
        int param;
        char strVal[10] = {};

        void onChange(const event::Change& e) override
        {
            ParamQuantity* paramQuantity = getParamQuantity();
            adder = (htAdder*)paramQuantity->module;
            sprintf(strVal, "[%.2f]", paramQuantity->getValue());
            adder->m_pTextLabel->text = strVal;
            RoundKnob::onChange(e);
        }
    };

    // 3-way flip switch
    struct modeSwitch : Hippieswitch_orange
    {
        htAdder* adder;
    };

    // Overrides 
    void    JsonParams(bool bTo, json_t* root);
    void    process(const ProcessArgs& args) override;
    json_t* dataToJson() override;
    void    dataFromJson(json_t* rootJ) override;
    void    onRandomize() override;
    void    onReset() override;
};

htAdder g_Adder_Browser;

struct htAdder_Widget : ModuleWidget
{
    htAdder_Widget(htAdder* module)
    {

        htAdder* pmod;
        setModule(module);
        if (!module)
            pmod = &g_Adder_Browser;
        else
            pmod = module;
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/htAdder.svg")));

        // number display of attenuator knob value
        pmod->m_pTextLabel = new Label();
        pmod->m_pTextLabel->box.pos = Vec(27.5f, 99);
        pmod->m_pTextLabel->color = nvgRGB(0, 0, 0);
        pmod->m_pTextLabel->text = "----";
        addChild(pmod->m_pTextLabel);

        // draw all the components with a loop
        int y = 145.0f;
        int x = 13.0f;
        int x_switches = 47.9f;

        // inputs, CV and mode switches
        for (int ch = 0; ch < nCHANNELS; ch++)
        {
            // input CVs
            addInput(createInput<MyPortInSmall>(Vec(x, y), module, htAdder::IN_SIGNAL + ch ));
            // switches for adding / subtract / bypass
            addParam(createParam<htAdder::modeSwitch>(Vec(x_switches, y), module, htAdder::PARAM_ADDER_MODE + ch));
            // CVs for mode switches
            addInput(createInput<MyPortInSmall>(Vec(79.2f, y), module, htAdder::IN_MODE_CV + ch));
            y += 30.1f;
        }

        // big knob for channel 1 attenuator
        addParam(createParam<htAdder::attenuateKnob>(Vec(33.0f, 59.0f), module, htAdder::PARAM_ATTENUATE));

        // outputs
        addOutput(createOutput<MyPortOutSmall>(Vec(74.0f, 322.0f), module, htAdder::OUT_AUDIOL));
        addOutput(createOutput<MyPortOutSmall>(Vec(74.0f, 343.0f), module, htAdder::OUT_AUDIOR));

        // screws
        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(0, 365)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

        if (module)
        {
            module->m_bInitialized = true;
        }
    }
};

void htAdder::process(const ProcessArgs& args)
{
    if (!m_bInitialized)
        return;

    // base pitch
    float htPitch = 0.0f;

    for (int ch = 0; ch < nCHANNELS; ch++)
    {
        // set the switches based on CV input
        if (inputs[IN_MODE_CV + ch].isConnected())
        {
            if (inputs[IN_MODE_CV + ch].value < 0.0f)
                params[PARAM_ADDER_MODE + ch].value = -1.0f;
            else if (inputs[IN_MODE_CV + ch].value > 0.0f)
                params[PARAM_ADDER_MODE + ch].value = 1.0f;
            else if (inputs[IN_MODE_CV + ch].value == 0.0f)
                params[PARAM_ADDER_MODE + ch].value = 0.0f;
        }

        // if no outputs connected do nothing
        if (outputs[OUT_AUDIOL].isConnected() || outputs[OUT_AUDIOR].isConnected())
        {
            // on first channel we have the attenuator
            if (ch == 0)
            {
                // if input is connected we add a scaled value of up to +/-1V
                // unless the mode is 0 in which case we bypass
                if (inputs[IN_SIGNAL + ch].isConnected())
                {
                    if (params[PARAM_ADDER_MODE + ch].value != 0)
                        htPitch = (inputs[IN_SIGNAL + ch].value) + 1.0f * params[PARAM_ADDER_MODE + ch].value * (params[PARAM_ATTENUATE].value);
                    else
                        htPitch = inputs[IN_SIGNAL + ch].value;
                }
                // if there is no input the pitch will be a scaled value of up to +/-1V
                else
                    if (params[PARAM_ADDER_MODE + ch].value != 0)
                        htPitch = 1.0f * params[PARAM_ADDER_MODE + ch].value * (params[PARAM_ATTENUATE].value);
            }
            else
            {
                // on the lower 4 channels:
                // if signal is connected add / subtract it IF the switch is set
                // if the switch is 0 -> do nothing
                if (inputs[IN_SIGNAL + ch].isConnected())
                {
                    if (params[PARAM_ADDER_MODE + ch].value != 0)
                        htPitch += inputs[IN_SIGNAL + ch].value * params[PARAM_ADDER_MODE + ch].getValue();
                }
                // if no input connected add -1 / +1 V or zero
                else
                    htPitch += 1.0f * params[PARAM_ADDER_MODE + ch].value;
            }
        }

        // one output will be the total of all channels
        // the other output is an inverted signal
        outputs[OUT_AUDIOL].value = htPitch;
        outputs[OUT_AUDIOR].value = -1.0f * htPitch;
    }
}

//-----------------------------------------------------
// Procedure: JsonParams  
//
//-----------------------------------------------------
void htAdder::JsonParams(bool bTo, json_t* root)
{
    //JsonDataInt(bTo, "m_State", root, m_State, nCHANNELS);
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t* htAdder::dataToJson()
{
    json_t* root = json_object();

    if (!root)
        return NULL;

    JsonParams(TOJSON, root);

    return root;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void htAdder::dataFromJson(json_t* root)
{
    JsonParams(FROMJSON, root);

    if (!m_bInitialized)
        return;
}

//-----------------------------------------------------
// Procedure:   onReset
//
//-----------------------------------------------------
void htAdder::onReset()
{
}

//-----------------------------------------------------
// Procedure:   onRandomize
//
//-----------------------------------------------------
void htAdder::onRandomize()
{
}

Model* modelhtAdder = createModel<htAdder, htAdder_Widget>("htAdder");