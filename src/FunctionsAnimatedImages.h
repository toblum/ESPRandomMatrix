/*****************************************************
 * Animated images
 *****************************************************/
#define RGB 565
#if RGB == 565
#define frame_size MATRIX_WIDTH *MATRIX_HEIGHT * 2
#else
#define frame_size MATRIX_WIDTH *MATRIX_HEIGHT * 3
#endif

uint8_t line_buffer[MATRIX_WIDTH * 2];
unsigned long anim_offset = 0;
uint16_t frame_no = 0;
union single_double
{
	uint8_t two[2];
	uint16_t one;
} this_single_double;

// This draws the pixel animation to the frame buffer in animation view
void draw_image()
{
	unsigned long frame_offset = anim_offset + frame_no * frame_size;

	for (int yy = 0; yy < MATRIX_HEIGHT; yy++)
	{
		memcpy_P(line_buffer, animations + frame_offset + yy * MATRIX_WIDTH * 2, MATRIX_WIDTH * 2);
		for (int xx = 0; xx < MATRIX_WIDTH; xx++)
		{
			this_single_double.two[0] = line_buffer[xx * 2];
			this_single_double.two[1] = line_buffer[xx * 2 + 1];

			display.drawPixelRGB565(xx, yy, this_single_double.one);
		}
		yield();
	}
}

unsigned long getAnimOffset(uint8_t anim_no)
{
	unsigned long offset = 0;
	for (uint8_t count = 0; count < anim_no; count++)
	{
		offset = offset + animation_lengths[count] * frame_size;
	}
	//Serial.println("anim_no: " + String(anim_no) + ", length: " + String(animation_lengths[anim_no])+ ", offset: " + String(offset));
	return offset;
}