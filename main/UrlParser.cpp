/*
 * UrlParser.cpp
 *
 *  Created on: 03.04.2017
 *      Author: helmut.spiegl
 */

#include "UrlParser.h"
#include <_ansi.h>
#include <ctype.h>


//---------------------------------------------------------


UrlParser::UrlParser() {
	Init();
}

UrlParser::~UrlParser() {
}

void UrlParser::Init(){
	muState = STATE_ParseUrl;
}

void UrlParser::ConsumeChar(char c, std::string& url, TParam* pParam){

	if (!c)
		return;
	c = tolower(c);

	switch(muState){
		case STATE_ParseUrl:
			if (c == '?')
				muState = STATE_UrlComplete;
			else
				url.append(1, c);
			break;
		case STATE_UrlComplete:
		case STATE_ParamComplete:
		case STATE_ParseParamName:
			if (c == '&')
				muState = STATE_ParamComplete;
			else if (c == '=')
				muState = STATE_ParseParamValue;
			else if (pParam){
				pParam->paramName.append(1, c);
				muState = STATE_ParseParamName;
			}
			break;
		case STATE_ParseParamValue:
			if (c == '&')
				muState = STATE_ParamComplete;
			else if (pParam)
				pParam->paramValue.append(1, c);
			break;

	}
}

void UrlParser::SignalEnd(){
	switch(muState){
		case STATE_ParseUrl:
			muState = STATE_UrlComplete;
			break;
		case STATE_ParseParamName:
		case STATE_ParseParamValue:
			muState = STATE_ParamComplete;
			break;
	}
}



