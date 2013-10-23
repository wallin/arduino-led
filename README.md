= Arduino LED controller library =
This is a simple library for easy and flexible controlling/creating of LEDs light sequences on the ATMega 328 (Arduino) microcontroller.

So far it's just working with the internal PWM outputs on the ATMEGA328, but I'm planning on extending it to work with the TLC5940 PWM controller

Some demos:

* [In a boombox](https://www.youtube.com/watch?v=kUcbP9cDgZs)
* [VU-meter](https://www.youtube.com/watch?v=IwiZFkM0mhY)
* [Two-color crossfade](https://www.youtube.com/watch?v=AOxNgVRFAr4)

= Installation =

Create a directory called `LEDTicker` in your Libraries-folder. Download and unzip the .cpp and .h files in this directory.

You should now be able to include 'ledticker.h' in your application.

= Usage =
The basic way of working with the library is to send a list of LED commands via the `runList` method. A LED command consists of the following:
  * LED number (not same as pin number)
  * LED Action (Fade up/down, on/off)
  * Time in milliseconds to wait until next command in list is executed
  * The fade curve to use
  * The time interval in milliseconds between fade steps

When the list has finished a user defined function will be called if it has been registred (with `registerCallback` method).

A new list of commands can be sent before the current list has finished. The old list will then be discarded.

= Reference=

==Interfaces==
Basically there are three public interfaces:
```
/*
 * Advances LED actions/commands. Could be called from main loop or
 * from a timer ISR
 */
void LEDTckr::tick();

/*
 * Runs a set of LED Commands
 */
void LEDTckr::runList(LEDCommandSet_t* cl);

/*
 * Registers a user function to run when commands in current list are finished
 */
void LEDTckr::registerCallback(void (*CBFunc)());
```

==Data types==

```

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
 * Descriptor for a LED channel. Things like current state,
 * which fadecurve it's running, io pin etc.
 */
typedef struct {
  int pin;
  LEDState_t     state;
  LEDFadeCurve_t curve;
  unsigned int   idx;
  unsigned long  nextTime;
  LEDAction_t    currentAct;
  unsigned int   interval;
} LEDChannel_t;


/*
 * A LED Command. Contains which LED to operate on, the action
 * to perform, how long to wait after the command is sent and
 * what fadecurve to use. Also the time interval between fade
 * steps
 */
typedef struct {
  int LEDNo;
  LEDAction_t    action;
  unsigned long  waitTime;
  LEDFadeCurve_t fadeCurve;
  unsigned int   interval;
} LEDCommand_t;


typedef struct {
  LEDCommand_t *cmds;
  unsigned int size;
  unsigned int laps;
} LEDCommandSet_t;

```

= Example code =
```
/*
 * Example usage of the LED Ticker libarary
 */
#include "LEDTicker.h"

/* Define some fade curves */
unsigned int fadeCurve[]     = {0,1,1,2,3,4,6,8,10,15,20,30,40,60,70,80,100,120,160,200,255};
unsigned int longCurve[]     = {0,1,1,1,2,2,3,4,5,6,7,8,9,10,13,15,18,20,25,30,35,40,60,70,80,90,100,120,140,160,200,230,255};
unsigned int fastFadeCurve[] = {0,10,20,30,80,255};
unsigned int onOffCurve[]    = {0,255};

LEDFadeCurve_t fadeCurves[] = {
  {fadeCurve    , 20},
  {fastFadeCurve,  5},
  {onOffCurve   ,  1},
  {longCurve    , 32}
};


/* Convenient macros for easy reading */
#define SLOW_FADE fadeCurves[0]
#define FAST_FADE fadeCurves[1]
#define ONOFF     fadeCurves[2]
#define LONG_FADE fadeCurves[3]

#define FAST_INTERVAL (20)
#define SLOW_INTERVAL (40)

/* Some command-sets */
LEDCommand_t small_delay[] = {
  /* Fade up LED 0 and 2, slow curve, fast interval. Then wait 1000ms */
  {0, FADE_UP,    0, SLOW_FADE, FAST_INTERVAL},
  {2, FADE_UP, 1000, SLOW_FADE, FAST_INTERVAL},
  /* etc. */
  {0, FADE_DOWN,  0, LONG_FADE, SLOW_INTERVAL},
  {2, FADE_DOWN,  0, LONG_FADE, SLOW_INTERVAL},
  {1, FADE_UP,    0, SLOW_FADE, FAST_INTERVAL},
  {3, FADE_UP, 1000, SLOW_FADE, FAST_INTERVAL},
  {1, FADE_DOWN,  0, LONG_FADE, SLOW_INTERVAL},
  {3, FADE_DOWN,  0, LONG_FADE, SLOW_INTERVAL}
};

LEDCommand_t fade_up[] = {
  {0, FADE_UP,    0, SLOW_FADE, SLOW_INTERVAL},
  {2, FADE_UP,    0, SLOW_FADE, SLOW_INTERVAL},
  {1, FADE_UP,    0, SLOW_FADE, SLOW_INTERVAL},
  {3, FADE_UP, 2000, SLOW_FADE, SLOW_INTERVAL},

  {0, FADE_DOWN,    0, SLOW_FADE, FAST_INTERVAL},
  {2, FADE_DOWN,    0, SLOW_FADE, FAST_INTERVAL},
  {1, FADE_DOWN,    0, SLOW_FADE, FAST_INTERVAL},
  {3, FADE_DOWN, 1000, SLOW_FADE, FAST_INTERVAL}
};

/* Command-set list */
LEDCommandSet_t clp[] = {
  {small_delay, 8, 1}, //Length is 8, run one time
  {fade_up,     8, 1}
};

/* Max index of the command-set list */
int clp_maxIdx = 1;

/* Current command-set */
int index = 0;


/***********************
 * Setup and main loop
 ***********************/

void setup()
{
  LEDTicker.registerCallback(&queue_next_list);
}

void loop()
{
  /* Advance LEDs as fast as possible */
  LEDTicker.tick();
}

/* Run the next commandset */
void queue_next_list() {
  LEDTicker.runList(&clp[index++]);
  index = index > clp_maxIdx ? 0 : index;
}

```