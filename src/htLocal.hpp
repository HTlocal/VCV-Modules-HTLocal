#pragma once

#include <rack.hpp>
#include "logger.hpp"

#define CV_MAX10 (10.0f)
#define TOJSON true
#define FROMJSON false

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

#include "Controls.hpp"

// Declare each Model, defined in each module source file
// extern Model* modelMyModule;
extern Model *modelhtFader;


void JsonDataInt( bool bTo, std::string strName, json_t *root, int *pdata, int len );
void JsonDataBool( bool bTo, std::string strName, json_t *root, bool *pdata, int len );
void JsonDataString( bool bTo, std::string strName, json_t *root, std::string *strText );