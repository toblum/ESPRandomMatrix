/*****************************************************
 * Tetris clock
 *****************************************************/
void animationHandler()
{
	// Not clearing the display and redrawing it when you
	// dont need to improves how the refresh rate appears
	if (!finishedAnimating)
	{
		display.fillScreen(tetris.tetrisBLACK);

		if (twelveHourFormat)
		{
			// Place holders for checking are any of the tetris objects
			// currently still animating.
			bool tetris1Done = false;
			bool tetris2Done = false;
			bool tetris3Done = false;

			tetris1Done = tetris.drawNumbers(-6, 26, showColon);
			tetris2Done = tetris2.drawText(56, 25);

			// Only draw the top letter once the bottom letter is finished.
			if (tetris2Done)
			{
				tetris3Done = tetris3.drawText(56, 15);
			}

			finishedAnimating = tetris1Done && tetris2Done && tetris3Done;
		}
		else
		{
			finishedAnimating = tetris.drawNumbers(2, 26, showColon);
		}
		display.showBuffer();
	}
}

void setMatrixTime(bool checkForUpdate = false)
{
	String timeString = "";
	String AmPmString = "";
	if (twelveHourFormat)
	{
		// Get the time in format "1:15" or 11:15 (12 hour, no leading 0)
		// Check the EZTime Github page for info on
		// time formatting
		timeString = myTZ.dateTime("g:i");

		//If the length is only 4, pad it with
		// a space at the beginning
		if (timeString.length() == 4)
		{
			timeString = " " + timeString;
		}

		//Get if its "AM" or "PM"
		AmPmString = myTZ.dateTime("A");
		if (lastDisplayedAmPm != AmPmString)
		{
			Serial.println(AmPmString);
			lastDisplayedAmPm = AmPmString;
			// Second character is always "M"
			// so need to parse it out
			tetris2.setText("M", forceRefresh);

			// Parse out first letter of String
			tetris3.setText(AmPmString.substring(0, 1), forceRefresh);
		}
	}
	else
	{
		// Get time in format "01:15" or "22:15"(24 hour with leading 0)
		timeString = myTZ.dateTime("H:i");
	}

	// Only update Time if its different
	if (lastDisplayedTime != timeString)
	{
		if (checkForUpdate) {
			Serial.println(timeString);
			lastDisplayedTime = timeString;
			tetris.setTime(timeString, forceRefresh);
		}

		// Must set this to false so animation knows
		// to start again
		finishedAnimating = false;
	}
}

void handleColonAfterAnimation()
{
	// It will draw the colon every time, but when the colour is black it
	// should look like its clearing it.
	uint16_t colour = showColon ? tetris.tetrisWHITE : tetris.tetrisBLACK;
	// The x position that you draw the tetris animation object
	int x = twelveHourFormat ? -6 : 2;
	// The y position adjusted for where the blocks will fall from
	// (this could be better!)
	int y = 26 - (TETRIS_Y_DROP_DEFAULT * tetris.scale);
	tetris.drawColon(x, y, colour);
}