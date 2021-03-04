#pragma once

#define DWRGB( r, g, b ) (b | g<<8 | r<<16)


typedef struct
{
    union
    {
        unsigned int dwCol;
        unsigned char Col[ 4 ];
    };
}RGB_STRUCT;

typedef struct
{
    int x, y, x2, y2;
}RECT_STRUCT;


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

    //void onChange(const event::Change& e) override
    //{
    //    step();
    //}

    //void step() override
    //{
    //}
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
        //setSvg( SVG::load(asset::plugin(thePlugin, "res/mschack_PortIn_small.svg" ) ) );
        //wrap();
        //box.size = background->box.size;
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
        //setSvg( SVG::load(asset::plugin(thePlugin, "res/mschack_PortOut_small.svg" ) ) );
        //background->wrap();
        //box.size = background->box.size;
    }
};

//-----------------------------------------------------
// MyLEDButton
//-----------------------------------------------------
struct MyLEDButton : OpaqueWidget 
{
    typedef void MyLEDButtonCALLBACK ( void *pClass, int id, bool bOn );

    bool            m_bInitialized = false;
    int             m_Id;
    int             m_Type;
    int             m_StepCount = 0;
    bool            m_bOn = false;
    RGB_STRUCT      m_Colour;
    RGB_STRUCT      m_LEDColour;
    float           m_fLEDsize;
    float           m_fLEDsize_d2;

    MyLEDButtonCALLBACK *m_pCallback;
    void              *m_pClass;

    RECT_STRUCT     m_Rect;

	enum MyLEDButton_Types 
    {
        TYPE_SWITCH,
        TYPE_MOMENTARY
	};

    //-----------------------------------------------------
    // Procedure:   Constructor
    //-----------------------------------------------------
    MyLEDButton( int x, int y, int w, int h, float LEDsize, int colour, int LEDcolour, int type, int id, void *pClass, MyLEDButtonCALLBACK *pCallback )
    {
        m_Id = id;
        m_pCallback = pCallback;
        m_pClass = pClass;
        m_Type = type;
        m_Colour.dwCol = colour;
        m_LEDColour.dwCol = LEDcolour;
        m_fLEDsize = LEDsize;
        m_fLEDsize_d2 = LEDsize / 2.0f;

		box.pos = Vec( x, y );
        box.size = Vec( w, h );

        m_Rect.x = 0;
        m_Rect.y = 0;
        m_Rect.x2 = w - 1;
        m_Rect.y2 = h - 1;

        m_bInitialized = true;
    }

    //-----------------------------------------------------
    // Procedure:   Set
    //-----------------------------------------------------
    void Set( bool bOn )
    {
        m_bOn = bOn;

        if( m_Type == TYPE_MOMENTARY && bOn )
            m_StepCount = 8;//(int)( engineGetSampleRate() * 0.05 );
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void draw(const DrawArgs &args) override
    {
        float xi, yi;

        if( !m_bInitialized )
            return;

        nvgFillColor( args.vg, nvgRGB( m_Colour.Col[ 2 ], m_Colour.Col[ 1 ], m_Colour.Col[ 0 ] ) );

		nvgBeginPath( args.vg );
        nvgRect( args.vg, 0, 0, box.size.x - 1, box.size.y - 1 );
		nvgClosePath( args.vg );
		nvgFill( args.vg );

        if( !m_bOn )
            nvgFillColor( args.vg, nvgRGB(0xd3, 0xd3, 0xd3) );
        else
            nvgFillColor( args.vg, nvgRGB( m_LEDColour.Col[ 2 ], m_LEDColour.Col[ 1 ], m_LEDColour.Col[ 0 ] ) );

        xi = ( ( (float)m_Rect.x2 + (float)m_Rect.x ) / 2.0f ) - m_fLEDsize_d2 ;
        yi = ( ( (float)m_Rect.y2 + (float)m_Rect.y ) / 2.0f ) - m_fLEDsize_d2 ;

		nvgBeginPath( args.vg );
        nvgRoundedRect( args.vg, xi, yi, m_fLEDsize, m_fLEDsize, 2.5 );
		nvgClosePath( args.vg );
		nvgFill( args.vg );
	}

    //-----------------------------------------------------
    // Procedure:   isPoint
    //-----------------------------------------------------
    bool isPoint( RECT_STRUCT *prect, int x, int y )
    {
        if( x < prect->x || x > prect->x2 || y < prect->y || y > prect->y2 )
            return false;

        return true;
    }

    //-----------------------------------------------------
    // Procedure:   onMouseDown
    //-----------------------------------------------------
    void onButton(const event::Button &e) override
    {
        if( !m_bInitialized || e.button != 0 || e.action != GLFW_PRESS )
            return;

        //if( isPoint( &m_Rect, (int)e.pos.x, (int)e.pos.y ) )
        //{
            m_bOn = !m_bOn;

            if( m_Type == TYPE_MOMENTARY )
            {
                if( m_pCallback && m_pClass )
                {
                    m_bOn = true;
                    m_StepCount = 8;//(int)( engineGetSampleRate() * 0.05 );
                    m_pCallback( m_pClass, m_Id, true );
                }
            }
            else
            {
                if( m_pCallback && m_pClass )
                    m_pCallback( m_pClass, m_Id, m_bOn );
            }

            //return;
        //}

        //return;
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void step() override
    {
        if( m_StepCount && ( m_Type == TYPE_MOMENTARY ) )
        {
            if( --m_StepCount <= 0 )
            {
                m_bOn = false;
                m_StepCount = 0;
            }
        }
    }
};