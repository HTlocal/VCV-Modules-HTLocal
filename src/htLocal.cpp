#include "htLocal.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	p->addModel( modelhtFader );
    p->addModel( modelhtAdder );

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}

//-----------------------------------------------------
// Procedure: JsonDataInt  
//
//-----------------------------------------------------
void JsonDataInt( bool bTo, std::string strName, json_t *root, int *pdata, int len )
{
    int i;
    json_t *jsarray, *js;

    if( !pdata || !root || len <= 0 )
        return;

    if( bTo )
    {
        jsarray = json_array();

        for ( i = 0; i < len; i++ )
        {
	        js = json_integer( pdata[ i ] );
	        json_array_append_new( jsarray, js );
        }

        json_object_set_new( root, strName.c_str(), jsarray );
    }
    else
    {
        jsarray = json_object_get( root, strName.c_str() );

        if( jsarray )
        {
		    for ( i = 0; i < len; i++)
            {
			    js = json_array_get( jsarray, i );

			    if( js )
				    pdata[ i ] = json_integer_value( js );
		    }
        }
    }
}

//-----------------------------------------------------
// Procedure: JsonDataBool  
//
//-----------------------------------------------------
void JsonDataBool( bool bTo, std::string strName, json_t *root, bool *pdata, int len )
{
    int i;
    json_t *jsarray, *js;

    if( !pdata || !root || len <= 0 )
        return;

    if( bTo )
    {
        jsarray = json_array();

        for ( i = 0; i < len; i++ )
        {
	        js = json_boolean( pdata[ i ] );
	        json_array_append_new( jsarray, js );
        }

        json_object_set_new( root, strName.c_str(), jsarray );
    }
    else
    {
        jsarray = json_object_get( root, strName.c_str() );

        if( jsarray )
        {
		    for ( i = 0; i < len; i++)
            {
			    js = json_array_get( jsarray, i );

			    if( js )
				    pdata[ i ] = json_boolean_value( js );
		    }
        }
    }
}

//-----------------------------------------------------
// Procedure: JsonDataString
//
//-----------------------------------------------------
void JsonDataString( bool bTo, std::string strName, json_t *root, std::string *strText )
{
    json_t *textJ;

    if( !root )
        return;

    if( bTo )
    {
    	json_object_set_new( root, strName.c_str(), json_string( strText->c_str() ) );
    }
    else
    {
    	textJ = json_object_get( root, strName.c_str() );

		if( textJ )
			*strText = json_string_value( textJ );
    }
}