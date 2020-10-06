/*****************************************************
 * PxMatrix display functions
 *****************************************************/
#ifdef ESP8266
// ISR for display refresh
void display_updater()
{
	display.display(display_draw_time);
}
#endif

#ifdef ESP32
void IRAM_ATTR display_updater()
{
	// Increment the counter and set the time of ISR
	portENTER_CRITICAL_ISR(&timerMux);
	display.display(display_draw_time);
	portEXIT_CRITICAL_ISR(&timerMux);
}
#endif

void display_update_enable(bool is_enable)
{
#ifdef ESP8266
	if (is_enable)
		display_ticker.attach(0.001, display_updater);
	else
		display_ticker.detach();
#endif

#ifdef ESP32
	if (is_enable)
	{
		timer = timerBegin(0, 80, true);
		timerAttachInterrupt(timer, &display_updater, true);
		timerAlarmWrite(timer, 2000, true);
		timerAlarmEnable(timer);
	}
	else
	{
		timerDetachInterrupt(timer);
		timerAlarmDisable(timer);
	}
#endif
}