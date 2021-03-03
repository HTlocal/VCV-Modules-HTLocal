#include "htLocal.hpp"

#define nCHANNELS 8

struct htFader : Module
{
    enum ParamIds 
    {
		PARAM_SPEED_IN,
		PARAM_SPEED_OUT	= PARAM_SPEED_IN + nCHANNELS,
        PARAM_CURVE_V   = PARAM_SPEED_OUT + nCHANNELS,
        nPARAMS			= PARAM_CURVE_V + nCHANNELS
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
    MyLEDButton    *m_pTrigButton[ nCHANNELS ] = {};
    int             m_State[ nCHANNELS ] = {STATE_OFF};
    float           m_fFade[ nCHANNELS ] = {};
    float           m_fPos[ nCHANNELS ] = {};
    Label           *m_pTextLabel = NULL;
    

    htFader() {
        config(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS);

        for( int i = 0; i < nCHANNELS; i++ )
        {
            configParam( PARAM_SPEED_IN + i, 0.05f, 100.0f, 20.0f, "Fade In Speed" );
            configParam( PARAM_SPEED_OUT + i, 0.05f, 100.0f, 20.0f, "Fade Out Speed" );
            configParam( PARAM_CURVE_V + i, -1.f, 1.f, -0.7f, "Curve shape" );
        }
    }

    bool            processFade( int ch, bool bfin, float sampleRate, float sampleTime);
    
    struct spd_Knob : Hippieknob
    {
    	htFader *fader;
        int param;
        char strVal[ 10 ] = {};

        void onChange( const event::Change &e ) override
        {
            fader = (htFader*)paramQuantity->module;
            sprintf( strVal, "[%.2f]", paramQuantity->getValue() );
            fader->m_pTextLabel->text = strVal;
		    RoundKnob::onChange( e );
	    }
    };

    

    // Overrides 
    void    JsonParams( bool bTo, json_t *root);
    void    process(const ProcessArgs &args) override;
    json_t* dataToJson() override;
    void    dataFromJson(json_t *rootJ) override;
    void    onRandomize() override;
    void    onReset() override;

};

void htFader_TrigButton(void* pClass, int id, bool bOn)
{
    htFader* fader;
    fader = (htFader*)pClass;

    if (bOn)
        fader->m_pTrigButton[id]->Set(true);
    else
        fader->m_pTrigButton[id]->Set(false);
}



htFader g_Fader_Browser;

struct htFader_Widget : ModuleWidget 
{
    htFader_Widget( htFader *module )
    {
        htFader *pmod;
        setModule(module);
        if( !module )
            pmod = &g_Fader_Browser;
        else
            pmod = module;
        setPanel(APP->window->loadSvg(asset::plugin( pluginInstance, "res/htFader.svg")));
        pmod->m_pTextLabel = new Label();
        pmod->m_pTextLabel->box.pos = Vec( 105, 50 );
        pmod->m_pTextLabel->color = nvgRGB(0, 0, 0);
        pmod->m_pTextLabel->text = "----";
	    addChild( pmod->m_pTextLabel );
        
        int x = 3;
        int y = 97;
        for( int ch = 0; ch < nCHANNELS; ch++ )
	    {
            // inputs
            addInput(createInput<MyPortInSmall>( Vec( x + 1, y ), module, htFader::IN_AUDIOL + ch ) );
		    addInput(createInput<MyPortInSmall>( Vec( x + 22, y ), module, htFader::IN_AUDIOR + ch ) );
            // trigger input
            addInput(createInput<MyPortInSmall>( Vec( x + 47, y ), module, htFader::IN_TRIGS + ch ) );
            // trigger button
            pmod->m_pTrigButton[ ch ] = new MyLEDButton(x + 68, 
                                                        y - 1, 
                                                        19, 
                                                        19, 
                                                        15.0, 
                                                        DWRGB( 180, 180, 180 ), 
                                                        DWRGB( 219, 142, 250 ), 
                                                        MyLEDButton::TYPE_SWITCH, 
                                                        ch, 
                                                        module, 
                                                        htFader_TrigButton );
	        addChild( pmod->m_pTrigButton[ ch ] );
            // speed knobs
            addParam(createParam<htFader::spd_Knob>( Vec( x + 94, y ), module, htFader::PARAM_SPEED_IN + ch  ) );
            addParam(createParam<htFader::spd_Knob>( Vec( x + 115, y ), module, htFader::PARAM_SPEED_OUT + ch ) );
            // curve parameter
            addParam(createParam<htFader::spd_Knob>(Vec(x + 139, y), module, htFader::PARAM_CURVE_V + ch));
            // outputs
            addOutput(createOutput<MyPortOutSmall>( Vec( x + 165, y ), module, htFader::OUT_AUDIOL + ch ) );
            addOutput(createOutput<MyPortOutSmall>( Vec( x + 186, y ), module, htFader::OUT_AUDIOR + ch ) );
            y += 33;
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
    float x = 0.0f;
    float v = 0.7f;

    v = params[PARAM_CURVE_V + ch].getValue();

    if( bfin ) {
		// x = m_fPos[ ch ]  / params[ PARAM_SPEED_IN + ch ].getValue();
		x = m_fPos[ ch ];
		m_fFade[ ch ] = simd::exp(v * (x - 1.0f)) * x;
		// char aaa[16];
		// sprintf(aaa, "%.9f", m_fPos[ ch ]);
		// WARN("pos: %s", aaa);
    }
    else {
		// x = m_fPos[ ch ] / params[ PARAM_SPEED_OUT + ch ].getValue();
		x = m_fPos[ ch ];
		m_fFade[ ch ] = simd::exp( (v * -1.0f) * x) * (1.0f - x);
	}

    

    if (bfin) {
		m_fPos[ ch ] += (sampleTime / params[ PARAM_SPEED_IN + ch ].getValue());
        if( m_fPos[ ch ] >= 1.f) // params[ PARAM_SPEED_IN + ch ].getValue()) // || m_fFade[ ch ] >= 1.0f )
			//INFO("done");
			return true;
	}
    else {
		m_fPos[ ch ] += (sampleTime / params[ PARAM_SPEED_OUT + ch ].getValue());
		if( m_fPos[ ch ] >=  1.f) // params[ PARAM_SPEED_OUT + ch ].getValue()) // || m_fFade[ ch ] <= 0.0f)
			//INFO("Done.");
			return true;
	}

    return false;
}

#define FADE_GATE_LVL (0.01f)

void htFader::process(const ProcessArgs &args)
{
	if( !m_bInitialized )
		return;

	for( int ch = 0; ch < nCHANNELS; ch++ )
	{
		switch( m_State[ ch ] )
		{
		case STATE_FOUT:

			// Trigger Fade-In when we are already fading out
			if( inputs[ IN_TRIGS + ch ].getNormalVoltage( 0.0f ) >= FADE_GATE_LVL || m_pTrigButton[ ch ]->m_bOn )
			{
				m_pTrigButton[ ch ]->Set( true );
				m_fPos[ ch ] = 1.0f - m_fPos[ ch ];
				m_State[ ch ] = STATE_FIN;
				break;
			}

			if( processFade( ch, false, args.sampleRate, args.sampleTime ) )
			{
				m_fFade[ ch ] = 0.0f;
				m_State[ ch ] = STATE_OFF;
			}
			break;

		case STATE_OFF:

			// Trigger Fade-In when Off
			if( inputs[ IN_TRIGS + ch ].getNormalVoltage( 0.0f ) >= FADE_GATE_LVL || m_pTrigButton[ ch ]->m_bOn )
			{
				m_pTrigButton[ ch ]->Set( true );
				m_State[ ch ] = STATE_FIN;
				m_fPos[ ch ] = 0.0f;
				break;
			}

			m_fFade[ ch ] = 0.0f;
			break;

		case STATE_FIN:

			if( inputs[ IN_TRIGS + ch ].getNormalVoltage( 1.0f ) < FADE_GATE_LVL || !m_pTrigButton[ ch ]->m_bOn )
			{
				m_pTrigButton[ ch ]->Set( false );
				m_fPos[ ch ] = 1.0f - m_fPos[ ch ];
				m_State[ ch ] = STATE_FOUT;
				break;
			}

			if( processFade( ch, true, args.sampleRate, args.sampleTime)  )
			{
				m_fFade[ ch ] = 1.0f;
				m_State[ ch ] = STATE_ON;
			}
			break;

		case STATE_ON:

			if( inputs[ IN_TRIGS + ch ].getNormalVoltage( 1.0f ) < FADE_GATE_LVL || !m_pTrigButton[ ch ]->m_bOn )
			{
				m_pTrigButton[ ch ]->Set( false );
				m_State[ ch ] = STATE_FOUT;
				m_fPos[ ch ] = 0.0f;
				break;
			}

			m_fFade[ ch ] = 1.0f;
			break;
		}

		if( inputs[ IN_AUDIOL + ch ].isConnected() )
			outputs[ OUT_AUDIOL + ch ].setVoltage( inputs[ IN_AUDIOL + ch ].getVoltageSum() * m_fFade[ ch ] );
		else
			outputs[ OUT_AUDIOL + ch ].value = CV_MAX10 * m_fFade[ ch ];

		if( inputs[ IN_AUDIOR + ch ].isConnected() )
			outputs[ OUT_AUDIOR + ch ].setVoltage( inputs[ IN_AUDIOR + ch ].getVoltageSum() * m_fFade[ ch ] );
		else
			outputs[ OUT_AUDIOR + ch ].value = CV_MAX10 * m_fFade[ ch ];
	}
}

//-----------------------------------------------------
// Procedure: JsonParams  
//
//-----------------------------------------------------
void htFader::JsonParams( bool bTo, json_t *root)
{
    JsonDataInt( bTo, "m_State", root, m_State, nCHANNELS );
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

    JsonParams( TOJSON, root );
    
	return root;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void htFader::dataFromJson( json_t *root )
{
    JsonParams( FROMJSON, root );

    if( !m_bInitialized )
        return;

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
    	if( m_State[ ch ] == STATE_OFF || m_State[ ch ] == STATE_FOUT )
    	{
    		m_pTrigButton[ ch ]->Set( false );
    		m_State[ ch ] = STATE_OFF;
    		m_fFade[ ch ] = 0.0f;
    	}
    	else
    	{
    		m_pTrigButton[ ch ]->Set( true );
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