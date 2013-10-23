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
#include "LEDTicker.h"

#define LIST_FINISHED if(listFinishedCB != NULL) { listFinishedCB(); }

/* Setup some default values used for initialization */
unsigned int defValues[] = {0, LEDTICKER_MAX_LED_VALUE};
LEDFadeCurve_t defCurve = {defValues, 2};

LEDCommand_t defCmds[] = {
  {0, OFF, 0, defCurve, 20}
};

/* All the available LEDs */
#define LEDTICKER_NO_OF_LEDS (4)
static LEDChannel_t _LEDs[] = {
    { 5,  IDLE, defCmds, 0, 0 },
    { 3,  IDLE, defCmds, 0, 0 },
    { 6,  IDLE, defCmds, 0, 0 },
    { 10, IDLE, defCmds, 0, 0 }    
};

/* Pre-instantiate the LEDTckr class (only need one) */
LEDTckr LEDTicker;

/* 
 * Constructor/initialization
 */
LEDTckr::LEDTckr() {
  _nextCommandTime = 0;
  _commandIdx = -1;
  _maxIdx = 0;
  _commandList = defCmds;
  listFinishedCB = NULL;
}

/* 
 * Advances LED actions/commands. Could be called from main loop or
 * from a timer ISR
 */
void LEDTckr::tick() {
  /* Quick (compare to zero) check to see if there is anything to do */
  if(_commandList == NULL) {
    return;
  }
  /* Renew the time */
  _cTime = millis();
  
  /* Check if a new command should be sent */
  if(_cTime > _nextCommandTime) {
    /* Gather concurrent commands */
    do {
      _commandIdx++; 
      /* Have we reached the end of the commandlist */    
      if(_commandIdx > _maxIdx) {
        _listLapIdx++;
        /* Shall we run the commandlist more times?
         * If not, call the callback and return  */
        if(_listLapIdx >= _listLaps) {
          _commandList = NULL;
          LIST_FINISHED
          return;
        }
        /* Reset command index */
        _commandIdx = 0;
      }
      /* Run the command and set the time for the next command */
      command(&_commandList[_commandIdx]);
      _nextCommandTime = _cTime + _commandList[_commandIdx].waitTime;
    } while(_commandList[_commandIdx].waitTime == 0);
  }
  
  /* Give each LED Channel a chance to run one more step */
  {
    int i = LEDTICKER_NO_OF_LEDS-1;
    for(; i >= 0; i--) {
      if(_cTime > _LEDs[i].nextTime &&
         _LEDs[i].state == RUNNING) {
        next(&_LEDs[i]);    
      }
    }
  }  
}

/* 
 * Runs a set of LED Commands
 */
void LEDTckr::runList(LEDCommandSet_t* cl) {
  _maxIdx = cl->size - 1;
  _commandIdx = -1;
  _nextCommandTime = 0;
  
  _listLaps = cl->laps;
  _listLapIdx = 0;
  
  _commandList = cl->cmds;
}

/*
 * Registers a user function to run when commands are finished
 */
void LEDTckr::registerCallback(void (*CBFunc)()) {
  listFinishedCB = CBFunc;  
}

void LEDTckr::removeCallback() {
  listFinishedCB = NULL;
}


/************* Private methods ***********/

/* 
 * Runs a single command 
 */
void LEDTckr::command(LEDCommand_t *cmd) {
  LEDChannel_t *led = &_LEDs[cmd->LEDNo];
  unsigned int oldMax = led->cmd->fadeCurve.maxIdx;
  unsigned int oldIdx = led->idx;
  led->state = RUNNING;
  led->cmd = cmd;
  led->nextTime = 0;
  /* Calculate new index */
  if(oldMax != led->cmd->fadeCurve.maxIdx) {
    led->idx = (oldIdx * led->cmd->fadeCurve.maxIdx) / oldMax;    
  }
}

/*
 * Let a LED do it's next thing
 */
void LEDTckr::next(LEDChannel_t *led) {
  /* What are we doing? */
  switch(led->cmd->action) {
    case FADE_UP:
      led->idx++;
      break;
    case FADE_DOWN:
      led->idx--;
      break;
    case OFF:
      led->idx = 0;
      break;
    case ON:
      led->idx = led->cmd->fadeCurve.maxIdx;
      break;
  }
 
  /* Go into idle of we've reached the end of the curve */
  if(led->idx <= 0) {
    led->idx = 0;
    led->state = IDLE;
  }
  else if(led->idx >= led->cmd->fadeCurve.maxIdx) {
    led->idx = led->cmd->fadeCurve.maxIdx;
    led->state = IDLE;
  }
  /* Write new value to LED and set the time for next write */
  analogWrite(led->pin, led->cmd->fadeCurve.values[led->idx]);  
  led->nextTime = _cTime + led->cmd->interval;
   
}

