#include <freertos/FreeRTOS.h>
#include "DisplayCharter.h"
#include <esp_log.h>


DisplayCharter::DisplayCharter(){
	Init();
}
void DisplayCharter::Init(){
	for (__uint8_t i=0 ; i<RING_LEDCOUNT ; i++)
		mLedSet[i] = false;
	mBackgroundRed = 0;
	mBackgroundGreen = 0;
	mBackgroundBlue = 0;

	offset = 0;
	whirlSpeed = 0;
	morphPeriod = 0;
	morphingState = 0;

	morphingPercentage = 0;
}

void DisplayCharter::SetLeds(__uint8_t pos, __uint8_t count, __uint8_t r, __uint8_t g, __uint8_t b){
	for (__uint8_t i=0 ; i<count ; i++){
		__uint8_t o = (pos + i) % RING_LEDCOUNT;
		mLedSet[o] = true;
		mLedRed[o] = r;
		mLedGreen[o] = g;
		mLedBlue[o] = b;
	}
}

void DisplayCharter::SetBackground( __uint8_t r, __uint8_t g, __uint8_t b){
	mBackgroundRed = r;
	mBackgroundGreen = g;
	mBackgroundBlue = b;
}

void DisplayCharter::SetWhirl(__uint8_t wspeed, bool clockwise){
	whirlSpeed = wspeed;
	whirlTick = 0xFF - whirlSpeed;
	whirlClockwise = clockwise;
}

void DisplayCharter::SetMorph(__uint16_t period, __uint8_t mspeed){
	morphPeriod = period;
	morphPeriodTick = morphPeriod;

	if (mspeed > 10)
		morphSpeed = 10;
	else
		morphSpeed = mspeed;

	ESP_LOGD("DisplayCharter", "SetMorph %d, %d", period, mspeed);
}

__uint16_t DisplayCharter::ParseLedArg(std::string& argument, __uint16_t iPos){
	__uint8_t seg = 0;
	std::string pos;
	std::string count;
	std::string color;
	__uint16_t i=iPos;
	while ((i < argument.length()) && (seg < 3)){
		char c = argument.at(i);
		if (c == '|')
			seg++;
		else switch(seg){
			case 0:
				pos+= c;
				break;
			case 1:
				count += c;
				break;
			case 2:
				color += c;
		}
		i++;
	}
	//pos.trim();
	//count.trim();
	//color.trim();
	ESP_LOGD("DisplayCharter", "ParseLedArg %s, %s, %s", pos.data(), count.data(), color.data());


	if ((pos.length() > 0) && (count.length() > 0) && (color.length() == 6)){
		SetLeds(atoi(pos.data()), atoi(count.data()), strtol(color.substr(0, 2).data(), NULL, 16),
												  	  strtol(color.substr(2, 2).data(), NULL, 16),
													  strtol(color.substr(4, 2).data(), NULL, 16));
	}
	return i;
}

void DisplayCharter::ParseBgArg(std::string& argument){
	if (argument.length() == 6)
		SetBackground(	strtol(argument.substr(0, 2).data(), NULL, 16),
						strtol(argument.substr(2, 2).data(), NULL, 16),
						strtol(argument.substr(4, 2).data(), NULL, 16));
}

void DisplayCharter::ParseWhirlArg(std::string& argument){
	__uint8_t seg = 0;
	std::string wspeed;

	for (__uint8_t i=0 ; i< argument.length() ; i++){
		char c = argument.at(i);
		if (c == '|'){
			if (++seg == 2)
				break;
		}
		else switch(seg){
			case 0:
				wspeed += c;
				break;
		}
	}
	//wspeed.trim();

	if (wspeed.length() > 0){
		SetWhirl(atoi(wspeed.data()), !seg);
	}
}

void DisplayCharter::ParseMorphArg(std::string& argument){
	__uint8_t seg = 0;
	std::string period;
	std::string mspeed;

	for (__uint8_t i=0 ; i< argument.length() ; i++){
		char c = argument.at(i);
		if (c == '|'){
			if (++seg == 2)
				break;
		}
		else switch(seg){
			case 0:
				period += c;
				break;
			case 1:
				mspeed += c;
				break;
		}
	}
  //period.trim();
  //mspeed.trim();

	if ((period.length() > 0) && (mspeed.length() > 0)){
		SetMorph(atoi(period.data()), atoi(mspeed.data()));
	}
}

void DisplayCharter::GetPixelColor(__uint8_t i, __uint8_t& ruR, __uint8_t& ruG, __uint8_t& ruB){

	if(! mLedSet[i]){
		ruR = mBackgroundRed;
		ruG = mBackgroundGreen;
		ruB = mBackgroundBlue;
	}
	else{
		ruR = (mLedRed[i] * (100 - morphingPercentage) / 100 + mBackgroundRed * morphingPercentage / 100);
		ruG = (mLedGreen[i] * (100 - morphingPercentage) / 100 + mBackgroundGreen * morphingPercentage / 100);
		ruB = (mLedBlue[i] * (100 - morphingPercentage) / 100 + mBackgroundBlue * morphingPercentage / 100);
	}
}


void DisplayCharter::Display(DotstarStripe &dotstar){

	//taskENTER_CRITICAL(&mMutex);

	__uint8_t r;
	__uint8_t g;
	__uint8_t b;
	for (__uint8_t i=0 ; i<RING_LEDCOUNT ; i++){
		GetPixelColor(i, r, g, b);
		dotstar.SetLeds(i, 1, r, g, b);
	}
	dotstar.SetStartPos(offset);
	dotstar.Show();

   
	if (whirlSpeed){
		if (!whirlTick--){
			if (whirlClockwise){
				if (++offset >= RING_LEDCOUNT)
					offset = 0;
			}
			else{
				if (!offset)
					offset = RING_LEDCOUNT - 1;
				else
					offset--;
			}
			whirlTick = 0xFF - whirlSpeed;
		}
	}

	switch (morphingState){
		case 0:
			if (morphPeriod){
				if (!--morphPeriodTick){
					morphingState = 1;
					morphingPercentage = 0;
					morphSpeedTick = 11 - morphSpeed;

					morphPeriodTick = morphPeriod;
				}
			}
			break;
		case 1:
			if (!--morphSpeedTick){
				if (morphingPercentage < 100)
					morphingPercentage+=2;
				else
					morphingState = 2;

				morphSpeedTick = 11 - morphSpeed;
			}
			break;
		case 2:
			if (!--morphSpeedTick){
				if (morphingPercentage > 0)
					morphingPercentage-=2;
				else
					morphingState = 0;

				morphSpeedTick = 11 - morphSpeed;
			}
			break;
	}
}



//------------------------------------------------------------------------------------------------------

/*

IPDisplay::IPDisplay(){
	ipspeed = 1100;
	displayCharter = 0;
}


void IPDisplay::ShowIp(const char* sIp, __uint8_t uLen, DisplayCharter* displayCh){
	displayCharter = displayCh;
	ipAddress = sIp;
	pos = 0;
	color = 0;
	colorValue[0] = 0xFF;
	colorValue[1] = 0x00;
	colorValue[2] = 0x00;
	tick = ipspeed;
	shortBreak = false;
}

void IPDisplay::ProcessTick()
{
	if (!displayCharter)
		return;
    
	if (!--tick){
		if (shortBreak){
			displayCharter->Init();
			displayCharter->SetLeds(0, 15, 0x30, 0x30, 0x30);
			tick = 100;
			shortBreak = false;
			return;
		}
		shortBreak = true;
		tick = ipspeed;

		if (pos >= ipAddress.length()){
			pos = 0;
			if (++color >= 3)
				color = 0;
			switch (color){
				case 0:
					colorValue[0] = 0xFF;
					colorValue[1] = 0x00;
					colorValue[2] = 0x00;
					break;
				case 1:
					colorValue[0] = 0x00;
					colorValue[1] = 0xFF;
					colorValue[2] = 0x00;
					break;
				case 2:
					colorValue[0] = 0x00;
					colorValue[1] = 0x00;
					colorValue[2] = 0xFF;
				 	break;
			}
		}
		displayCharter->Init();
		char c = ipAddress.at(pos);
		pos++;
    
		switch (c){
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
				displayCharter->SetLeds(0, c - '0', colorValue[0], colorValue[1], colorValue[2]);
				break;
			case '6':
			case '7':
			case '8':
			case '9':
				displayCharter->SetLeds(0, 5, colorValue[0], colorValue[1], colorValue[2]);
				displayCharter->SetLeds(8, c - '5', colorValue[0], colorValue[1], colorValue[2]);
				break;
			case '.':
				displayCharter->SetLeds(0, 1, 0xb0, 0xb0, 0xb0);
				displayCharter->SetLeds(5, 1, 0xb0, 0xb0, 0xb0);
				displayCharter->SetLeds(10, 1, 0xb0, 0xb0, 0xb0);
				break;
		}
	}
}
void IPDisplay::StopShowingIp(){ 
	if (displayCharter)
		displayCharter->Init();
	displayCharter = 0;
}*/
