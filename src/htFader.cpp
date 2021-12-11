#include "htLocal.hpp"

#define nCHANNELS 8
#define FADE_GATE_LVL (0.01f)


struct htFader : Module
{
    enum ParamIds 
    {
		PARAM_SPEED_IN,
		PARAM_SPEED_OUT	    = PARAM_SPEED_IN + nCHANNELS,
        PARAM_CURVE_V       = PARAM_SPEED_OUT + nCHANNELS,
        PARAM_TRIG_BUTTON   = PARAM_CURVE_V + nCHANNELS,
        nPARAMS			    = PARAM_TRIG_BUTTON + nCHANNELS
    };

    enum InputIds 
    {
		IN_TRIGS,
		IN_AUDIOL	= IN_TRIGS + nCHANNELS,
		IN_AUDIOR	= IN_AUDIOL + nCHANNELS,
        nINPUTS 	= IN_AUDIOR + nCHANNELS
	};

    enum OutputIds 
    {
		OUT_AUDIOL,
		OUT_AUDIOR	= OUT_AUDIOL + nCHANNELS,
        nOUTPUTS	= OUT_AUDIOR + nCHANNELS
	};

    enum LightIds 
    {
        nLIGHTS
	};

	enum FadeStates
    {
        STATE_OFF,
		STATE_FIN,
		STATE_ON,
		STATE_FOUT
	};

    bool            m_bInitialized = false;
    int             m_State[ nCHANNELS ] = {STATE_OFF};
    float           m_fFade[ nCHANNELS ] = {};
    float           m_fPos[ nCHANNELS ] = {};
    Label           *m_pTextLabel = NULL;
    bool            m_inputStateTrigger[nCHANNELS] = {};
    

    htFader() {
        config(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS);

        for( int i = 0; i < nCHANNELS; i++ )
        {
            configParam( PARAM_SPEED_IN + i, 0.05f, 100.0f, 20.0f, "Fade In Speed" );
            configParam( PARAM_SPEED_OUT + i, 0.05f, 100.0f, 20.0f, "Fade Out Speed" );
            configParam( PARAM_CURVE_V + i, -1.f, 3.f, -0.7f, "Curve shape" );
            configParam(PARAM_TRIG_BUTTON + i, 0.f, 1.f, 0.f, "Trigger for fade in / out");
            m_fFade[i] = 0.0f;
            m_inputStateTrigger[i] = false;
        }
    }

    bool            processFade( int ch, bool bfin, float sampleRate, float sampleTime);
    
    struct spd_Knob : Hippieknob
    {
        htFader* fader;
        int param;
        char strVal[ 10 ] = {};

        void onChange( const event::Change &e ) override
        {
            ParamQuantity* paramQuantity = getParamQuantity();
            fader = (htFader*)paramQuantity->module;
            sprintf( strVal, "[%.2f]", paramQuantity->getValue() );
            fader->m_pTextLabel->text = strVal;
		    RoundKnob::onChange( e );
	    }
    };

    struct slope_Knob : Hippieknob
    {
        htFader* fader;
        int param;
        char strVal[10] = {};

        void onChange(const event::Change& e) override
        {
            RoundKnob::onChange(e);
        }
    };

    struct fade_pushButton : Hippieswitch_pushbutton
    {
        htFader* fader;
    };
    

    // Overrides 
    void    JsonParams( bool bTo, bool bIst, json_t *root);
    void    process(const ProcessArgs &args) override;
    json_t* dataToJson() override;
    void    dataFromJson(json_t *rootJ) override;
    void    onRandomize() override;
    void    onReset() override;

};


htFader g_Fader_Browser;

struct htFader_Widget : ModuleWidget
{
    htFader_Widget(htFader *module )
    {
        htFader *pmod;
        setModule(module);
        if( !module )
            pmod = &g_Fader_Browser;
        else
            pmod = module;
        setPanel(APP->window->loadSvg(asset::plugin( pluginInstance, "res/htFader8.svg")));
        pmod->m_pTextLabel = new Label();
        pmod->m_pTextLabel->box.pos = Vec( 105, 93 );
        pmod->m_pTextLabel->color = nvgRGB(0, 0, 0);
        pmod->m_pTextLabel->text = "----";
	    addChild( pmod->m_pTextLabel );
        
        int x = 3;
        int y = 120;
        for( int ch = 0; ch < nCHANNELS; ch++ )
	    {
            // inputs
            addInput(createInput<MyPortInSmall>( Vec( x + 8.0f, y ), module, htFader::IN_AUDIOL + ch ) );
		    addInput(createInput<MyPortInSmall>( Vec( x + 28.5f, y ), module, htFader::IN_AUDIOR + ch ) );
            // trigger input
            addInput(createInput<MyPortInSmall>( Vec( x + 63.5f, y ), module, htFader::IN_TRIGS + ch ) );
            // trigger button
            addParam(createParam <htFader::fade_pushButton>(Vec(x + 81.5f, y + 4), module, htFader::PARAM_TRIG_BUTTON + ch ));
            // speed knobs
            addParam(createParam<htFader::spd_Knob>( Vec( x + 114, y ), module, htFader::PARAM_SPEED_IN + ch  ) );
            addParam(createParam<htFader::spd_Knob>( Vec( x + 130, y ), module, htFader::PARAM_SPEED_OUT + ch ) );
            // curve parameter
            addParam(createParam<htFader::slope_Knob>(Vec(x + 167, y), module, htFader::PARAM_CURVE_V + ch));
            // outputs
            addOutput(createOutput<MyPortOutSmall>( Vec( x + 204, y ), module, htFader::OUT_AUDIOL + ch ) );
            addOutput(createOutput<MyPortOutSmall>( Vec( x + 225, y ), module, htFader::OUT_AUDIOR + ch ) );
            y += 23;
        }
        addChild(createWidget<ScrewSilver>(Vec(15, 0)));
	    addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
	    addChild(createWidget<ScrewSilver>(Vec(15, 365))); 
	    addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

        if( module )
        {
            module->m_bInitialized = true;
        }

    }
};

bool htFader::processFade( int ch, bool bfin, float sampleRate, float sampleTime )
{
    float x;
    float v = params[PARAM_CURVE_V + ch].getValue();

    if( bfin ) {
		x = m_fPos[ ch ];
		m_fFade[ ch ] = simd::exp(v * (x - 1.0f)) * x;
        m_fPos[ch] += (sampleTime / params[PARAM_SPEED_IN + ch].getValue());
    }
    else {
		x = m_fPos[ ch ];
		m_fFade[ ch ] = simd::exp( (v * -1.0f) * x) * (1.0f - x);
        m_fPos[ch] += (sampleTime / params[PARAM_SPEED_OUT + ch].getValue());
	}
    
    if (m_fPos[ch] >= 1.f)
        return true;

    return false;
}


void htFader::process(const ProcessArgs &args)
{
	if( !m_bInitialized )
		return;

    for (int ch = 0; ch < nCHANNELS; ch++)
    {        
        if (inputs[IN_TRIGS + ch].getNormalVoltage(0.0f) < FADE_GATE_LVL && m_inputStateTrigger[ch] == true)
        {
            m_inputStateTrigger[ch] = false;
        }
        else if (inputs[IN_TRIGS + ch].getNormalVoltage(0.0f) >= FADE_GATE_LVL && m_inputStateTrigger[ch] == false)
        {
            m_inputStateTrigger[ch] = true;
            if (params[PARAM_TRIG_BUTTON + ch].value == 1.f) {
                params[PARAM_TRIG_BUTTON + ch].value = 0.f;
            }
            else
                params[PARAM_TRIG_BUTTON + ch].value = 1.f;
        }
        
        switch (m_State[ch])
        {
            case STATE_FOUT:

                // Trigger Fade-In when we are already fading out
                if (params[PARAM_TRIG_BUTTON + ch].value == 1)
                {
                    m_fPos[ch] = 1.0f - m_fPos[ch];
                    m_State[ch] = STATE_FIN;
                    break;
                }

                if (processFade(ch, false, args.sampleRate, args.sampleTime))
                {
                    m_State[ch] = STATE_OFF;
                    m_fFade[ch] = 0.f;
                    outputs[OUT_AUDIOL + ch].setVoltage(0.f);
                    outputs[OUT_AUDIOR + ch].setVoltage(0.f);
                }
                break;

            case STATE_OFF:

                // Trigger Fade-In when Off
                if (params[PARAM_TRIG_BUTTON + ch].value == 1)
                {
                    m_State[ch] = STATE_FIN;
                    m_fPos[ch] = 0.0f;
                    break;
                }

                //m_fFade[ch] = 0.0f;
                break;

            case STATE_FIN:

                if (params[PARAM_TRIG_BUTTON + ch].value == 0)
                {
                    m_fPos[ch] = 1.0f - m_fPos[ch];
                    m_State[ch] = STATE_FOUT;
                    break;
                }

                if (processFade(ch, true, args.sampleRate, args.sampleTime))
                {
                    m_fFade[ch] = 1.0f;
                    m_State[ch] = STATE_ON;
                }
                break;

            case STATE_ON:

                if (params[PARAM_TRIG_BUTTON + ch].value == 0)
                {
                    m_State[ch] = STATE_FOUT;
                    m_fPos[ch] = 0.0f;
                    break;
                }

                //m_fFade[ch] = 1.0f;
                break;
        }

        if (outputs[OUT_AUDIOL + ch].isConnected() || outputs[OUT_AUDIOR + ch].isConnected())
        {
            if (m_State[ch] != STATE_OFF)
            {
                if (inputs[IN_AUDIOL + ch].isConnected())
                    outputs[OUT_AUDIOL + ch].setVoltage(inputs[IN_AUDIOL + ch].getVoltageSum() * m_fFade[ch]);
                else
                    outputs[OUT_AUDIOL + ch].setVoltage(CV_MAX10 * m_fFade[ch]);

                if (inputs[IN_AUDIOR + ch].isConnected())
                    outputs[OUT_AUDIOR + ch].setVoltage(inputs[IN_AUDIOR + ch].getVoltageSum() * m_fFade[ch]);
                else
                    outputs[OUT_AUDIOR + ch].setVoltage(CV_MAX10 * m_fFade[ch]);
            }
        }
    }
}

//-----------------------------------------------------
// Procedure: JsonParams  
//
//-----------------------------------------------------
void htFader::JsonParams( bool bTo, bool bIst, json_t *root)
{
    JsonDataInt( bTo, "m_State", root, m_State, nCHANNELS );
    JsonDataBool(bIst, "m_inputStateTrigger", root, m_inputStateTrigger, nCHANNELS);
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *htFader::dataToJson()
{
	json_t *root = json_object();

    if( !root )
        return NULL;

    JsonParams( TOJSON, TOJSON, root );
    
	return root;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void htFader::dataFromJson( json_t *root )
{
    JsonParams( FROMJSON, FROMJSON, root );

    if( !m_bInitialized )
        return;

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
    	if( m_State[ ch ] == STATE_OFF || m_State[ ch ] == STATE_FOUT )
    	{
    		m_State[ ch ] = STATE_OFF;
    		m_fFade[ ch ] = 0.0f;
    	}
    	else
    	{
    		m_State[ ch ] = STATE_ON;
    		m_fFade[ ch ] = 1.0f;
    	}
    }
}

//-----------------------------------------------------
// Procedure:   onReset
//
//-----------------------------------------------------
void htFader::onReset()
{
}

//-----------------------------------------------------
// Procedure:   onRandomize
//
//-----------------------------------------------------
void htFader::onRandomize()
{
}

Model *modelhtFader = createModel<htFader, htFader_Widget>( "htFader" );