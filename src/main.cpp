/*
	Copyright (c) 2020 Tobias Blum. All rights reserved.

	ESPRandomMatrix - Random animations, images and more on a RGB led matrix
	https://github.com/toblum/ArduinoMSGraph

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// ***************************************************
// * Initialize PxMatrix
// ***************************************************

// This is how many color levels the display shows - the more the slower the update
//#define PxMATRIX_COLOR_DEPTH 4

// Defines the speed of the SPI bus (reducing this may help if you experience noisy images)
//#define PxMATRIX_SPI_FREQUENCY 20000000

// Creates a second buffer for backround drawing (doubles the required RAM)
#define PxMATRIX_double_buffer true

#include <PxMatrix.h>

// Pins for LED MATRIX
#ifdef ESP32
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 2
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
#endif

#ifdef ESP8266
#include <Ticker.h>
Ticker display_ticker;
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2
#endif

#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 32

unsigned long ms_current = 0;
unsigned long ms_previous = 0;
unsigned long ms_animation_max_duration = 20 * 1000; // 20 seconds
unsigned long next_frame = 0;

uint8_t mode = 0;
uint8_t anim_index = 0;

// This defines the 'on' time of the display is us. The larger this number,
// the brighter the display. If too large the ESP will crash
uint8_t display_draw_time = 40; //30-60 is usually fine

//PxMATRIX display(32,16,P_LAT, P_OE,P_A,P_B,P_C);
//PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);
PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);

#include <FastLED.h> // Aurora needs fastled

#include "AnimData.h"
#include "Effects.h"
Effects effects;

#include "Drawable.h"
#include "Playlist.h"
//#include "Geometry.h"

#include "Patterns.h"
Patterns patterns;


// ***************************************************
// * Initialize WiFi / NTP / Tetris Clock
// ***************************************************
#include "../_credentials.h"
#include <WiFi.h>
#include <TetrisMatrixDraw.h>
#include <ezTime.h>

#define double_buffer true

// Set a timezone using the following list
// https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
#define MYTIMEZONE "Europe/Berlin"

// Sets whether the clock should be 12 hour format or not.
bool twelveHourFormat = false;
// If true, every minute, all numbers are redrawn
bool forceRefresh = true;

hw_timer_t *animationTimer = NULL;
TetrisMatrixDraw tetris(display);  // Main clock
TetrisMatrixDraw tetris2(display); // The "M" of AM/PM
TetrisMatrixDraw tetris3(display); // The "P" or "A" of AM/PM

Timezone myTZ;

unsigned long oneSecondLoopDue = 0;
bool showColon = true;
volatile bool finishedAnimating = false;

String lastDisplayedTime = "";
String lastDisplayedAmPm = "";


#include "FunctionsDisplay.h"
#include "FunctionsTetrisClock.h"
#include "FunctionsAuroraAnimations.h"
#include "FunctionsAnimatedImages.h"


/*****************************************************
 * Main
 *****************************************************/
void setup()
{
	Serial.begin(115200);

	/*****************************************************
	 * Initialize WiFi
	 *****************************************************/
	// Attempt to connect to Wifi network:
	Serial.print("Connecting Wifi: ");
	Serial.println(ssid);

	// Set WiFi to station mode and disconnect from an AP if it was Previously
	// connected
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED)
	{
		Serial.print(".");
		delay(500);
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());


	/*****************************************************
	 * Initialize display
	 *****************************************************/
	// Define your display layout here, e.g. 1/8 step, and optional SPI pins begin(row_pattern, CLK, MOSI, MISO, SS)
	display.begin(16);
	//display.begin(8, 14, 13, 12, 4);

	// Define multiplex implemention here {BINARY, STRAIGHT} (default is BINARY)
	//display.setMuxPattern(BINARY);

	// Set the multiplex pattern {LINE, ZIGZAG,ZZAGG, ZAGGIZ, WZAGZIG, VZAG, ZAGZIG} (default is LINE)
	//display.setScanPattern(LINE);

	// Rotate display
	//display.setRotate(true);

	// Flip display
	//display.setFlip(true);

	// Helps to reduce display update latency on larger displays
	display.setFastUpdate(true);

	// Control the minimum color values that result in an active pixel
	//display.setColorOffset(5, 5,5);

	// Set the multiplex implemention {BINARY, STRAIGHT} (default is BINARY)
	//display.setMuxPattern(BINARY);

	// Set the color order {RRGGBB, RRBBGG, GGRRBB, GGBBRR, BBRRGG, BBGGRR} (default is RRGGBB)
	//display.setColorOrder(RRGGBB);

	// Set the time in microseconds that we pause after selecting each mux channel
	// (May help if some rows are missing / the mux chip is too slow)
	display.setMuxDelay(0,1,0,0,0); // Needed for ICN2037 / RUC7258 chipsets, see: https://github.com/2dom/PxMatrix/issues/119

	// Set the number of panels that make up the display area width (default is 1)
	//display.setPanelsWidth(2);

	// Set the brightness of the panels (default is 255)
	//display.setBrightness(50);

	// Set driver chip type
	//display.setDriverChip(FM6124);

	//display.setFastUpdate(true);
	display.clearDisplay();
	display_update_enable(true);


	/*****************************************************
	 * Initialize patters for effects
	 *****************************************************/
	delay(3000);
	// setup the effects generator
	effects.Setup();
	delay(500);
	Serial.println("Effects being loaded: ");
	listPatterns();

	patterns.setPattern(0); //   // simple noise
	patterns.start();

	Serial.print("Starting with pattern: ");
	Serial.println(patterns.getCurrentPatternName());


	/*****************************************************
	 * Initialize tetris clock animations / NTP time
	 *****************************************************/
	animationTimer = timerBegin(1, 80, true);
	timerAttachInterrupt(animationTimer, &animationHandler, true);
	timerAlarmWrite(animationTimer, 100000, true);
	timerAlarmEnable(animationTimer);

	tetris.scale = 2;

	// Setup EZ Time
	setDebug(INFO);
	waitForSync();

	Serial.println();
	Serial.println("UTC: " + UTC.dateTime());

	myTZ.setLocation(F(MYTIMEZONE));
	Serial.print(F("Time in your set timezone: "));
	Serial.println(myTZ.dateTime());
}

void loop()
{
	ms_current = millis();

	// Handle mode changes:
	// 0 - Image animations
	// 1 - Tetris clock
	// 2 - Aurora animations
	// Change mode after ms_animation_max_duration
	if ((ms_current - ms_previous) > ms_animation_max_duration)
	{
		uint8_t next_mode = 0;
		// Image --> Tetris
		if (mode == 0)
		{
			Serial.println("Change to tetris clock mode");

			next_mode = 1;

			// Update time before animation 
			setMatrixTime(true);
			finishedAnimating = false;
		}

		// Tetris --> Aurora
		if (mode == 1)
		{
			// Stop tetris animations
			finishedAnimating = true;

			Serial.println("Change to aurora animation mode");

			// patterns.moveRandom(1);
			patterns.stop();
			patterns.move(1);
			patterns.start();

			Serial.print("Changing pattern to:  ");
			Serial.println(patterns.getCurrentPatternName());

			// Select a random palette as well
			effects.RandomPalette();

			next_mode = 2;
		}

		// Aurora --> Image
		if (mode == 2)
		{
			Serial.println("Change to image mode");

			anim_index++;
			if (anim_index > sizeof(animation_lengths) - 1)
			{
				anim_index = 0;
			}

			anim_offset = getAnimOffset(anim_index);
			frame_no = 0;

			next_mode = 0;
		}

		mode = next_mode;
		ms_previous = ms_current;
	}

	// Image
	if (mode == 0)
	{
		if (next_frame < ms_current)
		{
			draw_image();
			display.showBuffer();
			next_frame = ms_current + 100;

			frame_no++;
			if (frame_no >= animation_lengths[anim_index])
				frame_no = 0;
		}
	}

	// Tetris clock
	if (mode == 1)
	{
		unsigned long now = millis();
		if (now > oneSecondLoopDue)
		{
			// We can call this often, but it will only
			// update when it needs to
			setMatrixTime();
			showColon = !showColon;

			// To reduce flicker on the screen we stop clearing the screen
			// when the animation is finished, but we still need the colon to
			// to blink
			if (finishedAnimating)
			{
				handleColonAfterAnimation();
			}
			oneSecondLoopDue = now + 1000;
		}
	}

	// Animation
	if (mode == 2)
	{
		if (next_frame < ms_current)
		{
			next_frame = patterns.drawFrame() + ms_current;
			display.showBuffer();
		}
	}
}
