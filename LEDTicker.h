/*
 *  LED Ticker library v0.2
 *  by Sebastian Wallin, March 2010, http://sewa.se
 *
 *  This is free software. You can redistribute it and/or modify it under
 *  the terms of Creative Commons Attribution 3.0 United States License. 
 *  To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/us/ 
 *  or send a letter to Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
 *
 */

#include "WProgram.h"

#ifndef LEDTicker_h
#define LEDTicker_h

/* IO write max value */
#define LEDTICKER_MAX_LED_VALUE 255 /* 8-bit */

/*
 * States of a LED Channel
 */
typedef enum {
  RUNNING,
  IDLE
} LEDState_t;

/*
 * LED Behaviours 
 */
typedef enum {
  FADE_UP,
  FADE_DOWN,
  OFF,
  ON
} LEDAction_t;


/*
 * A fadecurve: values and maximum index of value array 
 */
typedef struct {
  unsigned int *values;
  unsigned int maxIdx;
} LEDFadeCurve_t;


/*
 * A LED Command. Contains which LED to operate on, the action
 * to perform, how long to wait after the command is sent and 
 * what fadecurve to use
 */
typedef struct {
  int LEDNo;
  LEDAction_t action;
  unsigned long waitTime;
  LEDFadeCurve_t fadeCurve;
  unsigned int interval;
} LEDCommand_t;

/*
 * Descriptor for a LED channel. Things like current state,
 * which fadecurve it's running, io pin etc.
 */
typedef struct {
  int pin;
  LEDState_t state;
  LEDCommand_t *cmd;
  //LEDFadeCurve_t curve;  
  unsigned int idx;
  unsigned long nextTime;
  //LEDAction_t currentAct;
  //unsigned int interval;
} LEDChannel_t;

/*
 * A set of LED commands. 
 */
typedef struct {
  LEDCommand_t (*cmds);
  unsigned int size;
  unsigned int laps;
} LEDCommandSet_t;

/*
 * Singleton class for managing a set of LED Channels.
 */
class LEDTckr
{
  private:
    unsigned int * _defaultCurve[];
    unsigned long _cTime; 
    LEDCommand_t *_commandList;
    unsigned long _nextCommandTime;
    int  _commandIdx;
    unsigned int  _maxIdx;
    
    unsigned int _listLaps;
    unsigned int _listLapIdx;
    
    void next(LEDChannel_t *led);
    void command(LEDCommand_t *cmd); 
    void (*listFinishedCB)();
  
  public:
    LEDTckr();
    void tick();  
    void runList(LEDCommandSet_t* cl);
    void registerCallback(void (*CBFunc)());
    void removeCallback();   
};

extern LEDTckr LEDTicker;

#endif //LEDTicker_h