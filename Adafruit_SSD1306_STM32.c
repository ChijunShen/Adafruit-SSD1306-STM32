#include "Adafruit_SSD1306_STM32.h"

/* SSD1306 data buffer */
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * ((SSD1306_HEIGHT + 7) / 8)];

int8_t vccstate; // VCC selection, set by begin method.
uint8_t contrast = 255; ///< normal contrast setting for this device

char labeltext[5];
char *ptr_val;
float ox,oy;
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

uint8_t SSD1306_checkDevice(uint8_t address){
	return SW_I2C_Check_SlaveAddr(1,SSD1306_I2C_ADDR);
}

uint8_t SSD1306_begin(uint8_t vcs, uint8_t addr, uint8_t reset, uint8_t periphBegin){
	if (SSD1306_checkDevice(SSD1306_I2C_ADDR) != 1) {
		/* Return false */
		return 0;
	}
	SSD1306_clearDisplay();
	vccstate = vcs;
	 // Init sequence
	static const uint8_t init1[] = {SSD1306_DISPLAYOFF,         // 0xAE
	                                SSD1306_SETDISPLAYCLOCKDIV, // 0xD5
	                                0x80, // the suggested ratio 0x80
	                                SSD1306_SETMULTIPLEX}; // 0xA8
	SSD1306_commandList(init1, sizeof(init1));
	SSD1306_command1(SSD1306_HEIGHT - 1);

	static const uint8_t init2[] = {SSD1306_SETDISPLAYOFFSET, // 0xD3
	                                0x0,                      // no offset
	                                SSD1306_SETSTARTLINE | 0x0, // line #0
	                                SSD1306_CHARGEPUMP};        // 0x8D
	SSD1306_commandList(init2, sizeof(init2));
	SSD1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);

	static const uint8_t init3[] = {SSD1306_MEMORYMODE, // 0x20
	                                0x00, // 0x0 act like ks0108
	                                SSD1306_SEGREMAP | 0x1,
	                                SSD1306_COMSCANDEC};
	SSD1306_commandList(init3, sizeof(init3));
	uint8_t comPins = 0x12;
	uint8_t contrast = (vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF;
    SSD1306_command1(SSD1306_SETCOMPINS);
    SSD1306_command1(comPins);
    SSD1306_command1(SSD1306_SETCONTRAST);
    SSD1306_command1(contrast);
    SSD1306_command1(SSD1306_SETPRECHARGE); // 0xd9
    SSD1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);
    static const uint8_t init5[] = {
          SSD1306_SETVCOMDETECT, // 0xDB
          0x40,
          SSD1306_DISPLAYALLON_RESUME, // 0xA4
          SSD1306_NORMALDISPLAY,       // 0xA6
          SSD1306_DEACTIVATE_SCROLL,
          SSD1306_DISPLAYON}; // Main screen turn on
    SSD1306_commandList(init5, sizeof(init5));
	return 1;
}

void SSD1306_clearDisplay(void){
	memset(SSD1306_Buffer, 0, SSD1306_WIDTH * ((SSD1306_HEIGHT + 7) / 8));
}

void SSD1306_display(void){
	  static const uint8_t dlist1[] = {
	      SSD1306_PAGEADDR,
	      0,                      // Page start address
	      0xFF,                   // Page end (not really, but works here)
	      SSD1306_COLUMNADDR, 0}; // Column start address
	  SSD1306_commandList(dlist1, sizeof(dlist1));
	  SSD1306_command1(SSD1306_WIDTH - 1); // Column end address
	  uint16_t count = SSD1306_WIDTH * ((SSD1306_HEIGHT + 7) / 8);
	  SSD1306_I2C_WriteMultiByte(SSD1306_I2C_ADDR, 0x40, SSD1306_Buffer,count);
}

void SSD1306_invertDisplay(uint8_t i){
	SSD1306_command1(i ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
}

void SSD1306_dim(uint8_t dim){
	  SSD1306_command1(SSD1306_SETCONTRAST);
	  SSD1306_command1(dim ? 0 : contrast);
}

void SSD1306_commandList(const uint8_t *c, uint8_t n){
	SSD1306_I2C_WriteMultiConstByte(SSD1306_I2C_ADDR,0x00,c,n);

}

void SSD1306_command1(uint8_t c){
	SSD1306_I2C_WriteByte(SSD1306_I2C_ADDR,0x00,c);
}

void SSD1306_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color){
	// Update in subclasses if desired!
	if (x0 == x1) {
		if (y0 > y1) _swap_int16_t(y0, y1);
		SSD1306_drawFastVLine(x0, y0, y1 - y0 + 1, color);
	} else if (y0 == y1) {
		if (x0 > x1) _swap_int16_t(x0, x1);
		SSD1306_drawFastHLine(x0, y0, x1 - x0 + 1, color);
	} else {
		SSD1306_writeLine(x0, y0, x1, y1, color);
	}
}
void SSD1306_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
//  bool bSwap = false;
//  switch (rotation) {
//  case 1:
//    // 90 degree rotation, swap x & y for rotation, then invert x
//    bSwap = true;
//    ssd1306_swap(x, y);
//    x = WIDTH - x - 1;
//    break;
//  case 2:
//    // 180 degree rotation, invert x and y, then shift y around for height.
//    x = WIDTH - x - 1;
//    y = HEIGHT - y - 1;
//    x -= (w - 1);
//    break;
//  case 3:
//    // 270 degree rotation, swap x & y for rotation,
//    // then invert y and adjust y for w (not to become h)
//    bSwap = true;
//    ssd1306_swap(x, y);
//    y = HEIGHT - y - 1;
//    y -= (w - 1);
//    break;
//  }
//
//  if (bSwap)
//    drawFastVLineInternal(x, y, w, color);
//  else
    SSD1306_drawFastHLineInternal(x, y, w, color);
}
void SSD1306_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color){
//	bool bSwap = false;
//	switch (rotation) {
//	case 1:
//	// 90 degree rotation, swap x & y for rotation,
//	// then invert x and adjust x for h (now to become w)
//	bSwap = true;
//	ssd1306_swap(x, y);
//	x = WIDTH - x - 1;
//	x -= (h - 1);
//	break;
//	case 2:
//	// 180 degree rotation, invert x and y, then shift y around for height.
//	x = WIDTH - x - 1;
//	y = HEIGHT - y - 1;
//	y -= (h - 1);
//	break;
//	case 3:
//	// 270 degree rotation, swap x & y for rotation, then invert y
//	bSwap = true;
//	ssd1306_swap(x, y);
//	y = HEIGHT - y - 1;
//	break;
//	}

//	if (bSwap)
//	drawFastHLineInternal(x, y, h, color);
//	else
	SSD1306_drawFastVLineInternal(x, y, h, color);
}
void SSD1306_writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color){
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		_swap_int16_t(x0, y0);
		_swap_int16_t(x1, y1);
	}

	if (x0 > x1) {
		_swap_int16_t(x0, x1);
		_swap_int16_t(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
	ystep = 1;
	} else {
	ystep = -1;
	}

	for (; x0 <= x1; x0++) {
	if (steep) {
	  SSD1306_drawPixel(y0, x0, color);
	} else {
	  SSD1306_drawPixel(x0, y0, color);
	}
	err -= dy;
	if (err < 0) {
	  y0 += ystep;
	  err += dx;
	}
	}
}
void SSD1306_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
	SSD1306_drawFastHLine(x, y, w, color);
	SSD1306_drawFastHLine(x, y + h - 1, w, color);
	SSD1306_drawFastVLine(x, y, h, color);
	SSD1306_drawFastVLine(x + w - 1, y, h, color);
}

void SSD1306_drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color){
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	SSD1306_drawPixel(x0, y0 + r, color);
	SSD1306_drawPixel(x0, y0 - r, color);
	SSD1306_drawPixel(x0 + r, y0, color);
	SSD1306_drawPixel(x0 - r, y0, color);

	while (x < y) {
	if (f >= 0) {
	  y--;
	  ddF_y += 2;
	  f += ddF_y;
	}
	x++;
	ddF_x += 2;
	f += ddF_x;

	SSD1306_drawPixel(x0 + x, y0 + y, color);
	SSD1306_drawPixel(x0 - x, y0 + y, color);
	SSD1306_drawPixel(x0 + x, y0 - y, color);
	SSD1306_drawPixel(x0 - x, y0 - y, color);
	SSD1306_drawPixel(x0 + y, y0 + x, color);
	SSD1306_drawPixel(x0 - y, y0 + x, color);
	SSD1306_drawPixel(x0 + y, y0 - x, color);
	SSD1306_drawPixel(x0 - y, y0 - x, color);
	}
}

void SSD1306_drawFastVLineInternal(int16_t x, int16_t __y,int16_t __h, uint16_t color){
  if ((x >= 0) && (x < SSD1306_WIDTH)) { // X coord in bounds?
	if (__y < 0) {               // Clip top
	  __h += __y;
	  __y = 0;
	}
	if ((__y + __h) > SSD1306_HEIGHT) { // Clip bottom
	  __h = (SSD1306_HEIGHT - __y);
	}
	if (__h > 0) { // Proceed only if height is now positive
	  // this display doesn't need ints for coordinates,
	  // use local byte registers for faster juggling
	  uint8_t y = __y, h = __h;
	  uint8_t *pBuf = &SSD1306_Buffer[(y / 8) * SSD1306_WIDTH + x];

	  // do the first partial byte, if necessary - this requires some masking
	  uint8_t mod = (y & 7);
	  if (mod) {
		// mask off the high n bits we want to set
		mod = 8 - mod;
		// note - lookup table results in a nearly 10% performance
		// improvement in fill* functions
		// uint8_t mask = ~(0xFF >> mod);
		static const uint8_t premask[8] = {0x00, 0x80, 0xC0, 0xE0,
												   0xF0, 0xF8, 0xFC, 0xFE};
		uint8_t mask = premask[mod];
		// adjust the mask if we're not going to reach the end of this byte
		if (h < mod)
		  mask &= (0XFF >> (mod - h));

		switch (color) {
		case SSD1306_WHITE:
		  *pBuf |= mask;
		  break;
		case SSD1306_BLACK:
		  *pBuf &= ~mask;
		  break;
		case SSD1306_INVERSE:
		  *pBuf ^= mask;
		  break;
		}
		pBuf += SSD1306_WIDTH;
	  }

	  if (h >= mod) { // More to go?
		h -= mod;
		// Write solid bytes while we can - effectively 8 rows at a time
		if (h >= 8) {
		  if (color == SSD1306_INVERSE) {
			// separate copy of the code so we don't impact performance of
			// black/white write version with an extra comparison per loop
			do {
			  *pBuf ^= 0xFF; // Invert byte
			  pBuf += SSD1306_WIDTH; // Advance pointer 8 rows
			  h -= 8;        // Subtract 8 rows from height
			} while (h >= 8);
		  } else {
			// store a local value to work with
			uint8_t val = (color != SSD1306_BLACK) ? 255 : 0;
			do {
			  *pBuf = val;   // Set byte
			  pBuf += SSD1306_WIDTH; // Advance pointer 8 rows
			  h -= 8;        // Subtract 8 rows from height
			} while (h >= 8);
		  }
		}

		if (h) { // Do the final partial byte, if necessary
		  mod = h & 7;
		  // this time we want to mask the low bits of the byte,
		  // vs the high bits we did above
		  // uint8_t mask = (1 << mod) - 1;
		  // note - lookup table results in a nearly 10% performance
		  // improvement in fill* functions
		  static const uint8_t postmask[8] = {0x00, 0x01, 0x03, 0x07,
													  0x0F, 0x1F, 0x3F, 0x7F};
		  uint8_t mask = postmask[mod];
		  switch (color) {
		  case SSD1306_WHITE:
			*pBuf |= mask;
			break;
		  case SSD1306_BLACK:
			*pBuf &= ~mask;
			break;
		  case SSD1306_INVERSE:
			*pBuf ^= mask;
			break;
		  }
		}
	  }
	} // endif positive height
  }   // endif x in bounds
}
void SSD1306_drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color){
	if ((y >= 0) && (y < SSD1306_HEIGHT)) { // Y coord in bounds?
	    if (x < 0) {                  // Clip left
	      w += x;
	      x = 0;
	    }
	    if ((x + w) > SSD1306_WIDTH) { // Clip right
	      w = (SSD1306_WIDTH - x);
	    }
	    if (w > 0) { // Proceed only if width is positive
	      uint8_t *pBuf = &SSD1306_Buffer[(y / 8) * SSD1306_WIDTH + x], mask = 1 << (y & 7);
	      switch (color) {
	      case SSD1306_WHITE:
	        while (w--) {
	          *pBuf++ |= mask;
	        };
	        break;
	      case SSD1306_BLACK:
	        mask = ~mask;
	        while (w--) {
	          *pBuf++ &= mask;
	        };
	        break;
	      case SSD1306_INVERSE:
	        while (w--) {
	          *pBuf++ ^= mask;
	        };
	        break;
	      }
	    }
	  }

}

void SSD1306_startscrollright(uint8_t start, uint8_t stop){
	static const uint8_t scrollList1a[] = {SSD1306_RIGHT_HORIZONTAL_SCROLL, 0X00};
	SSD1306_commandList(scrollList1a, sizeof(scrollList1a));
	SSD1306_command1(start);
	SSD1306_command1(0X00);
	SSD1306_command1(stop);
	static const uint8_t scrollList1b[] = {0X00, 0XFF,SSD1306_ACTIVATE_SCROLL};
	SSD1306_commandList(scrollList1b, sizeof(scrollList1b));
}

void SSD1306_startscrollleft(uint8_t start, uint8_t stop){
	static const uint8_t scrollList2a[] = {SSD1306_LEFT_HORIZONTAL_SCROLL, 0X00};
	SSD1306_commandList(scrollList2a, sizeof(scrollList2a));
	SSD1306_command1(start);
	SSD1306_command1(0X00);
	SSD1306_command1(stop);
	static const uint8_t scrollList2b[] = {0X00, 0XFF, SSD1306_ACTIVATE_SCROLL};
	SSD1306_commandList(scrollList2b, sizeof(scrollList2b));
}
void SSD1306_startscrolldiagright(uint8_t start, uint8_t stop){
	static const uint8_t scrollList3a[] = {
	SSD1306_SET_VERTICAL_SCROLL_AREA, 0X00};
	SSD1306_commandList(scrollList3a, sizeof(scrollList3a));
	SSD1306_command1(SSD1306_HEIGHT);
	static const uint8_t scrollList3b[] = {
	SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL, 0X00};
	SSD1306_commandList(scrollList3b, sizeof(scrollList3b));
	SSD1306_command1(start);
	SSD1306_command1(0X00);
	SSD1306_command1(stop);
	static const uint8_t scrollList3c[] = {0X01, SSD1306_ACTIVATE_SCROLL};
	SSD1306_commandList(scrollList3c, sizeof(scrollList3c));
}
void SSD1306_startscrolldiagleft(uint8_t start, uint8_t stop){
	static const uint8_t scrollList4a[] = {
	SSD1306_SET_VERTICAL_SCROLL_AREA, 0X00};
	SSD1306_commandList(scrollList4a, sizeof(scrollList4a));
	SSD1306_command1(SSD1306_HEIGHT);
	static const uint8_t scrollList4b[] = {
	SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL, 0X00};
	SSD1306_commandList(scrollList4b, sizeof(scrollList4b));
	SSD1306_command1(start);
	SSD1306_command1(0X00);
	SSD1306_command1(stop);
	static const uint8_t scrollList4c[] = {0X01, SSD1306_ACTIVATE_SCROLL};
	SSD1306_commandList(scrollList4c, sizeof(scrollList4c));

}
void SSD1306_stopscroll(void){
	SSD1306_command1(SSD1306_DEACTIVATE_SCROLL);
}
uint8_t SSD1306_getPixel(int16_t x, int16_t y){
//	if ((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
//	// Pixel is in-bounds. Rotate coordinates if needed.
//	switch (getRotation()) {
//	case 1:
//	  ssd1306_swap(x, y);
//	  x = WIDTH - x - 1;
//	  break;
//	case 2:
//	  x = WIDTH - x - 1;
//	  y = HEIGHT - y - 1;
//	  break;
//	case 3:
//	  ssd1306_swap(x, y);
//	  y = HEIGHT - y - 1;
//	  break;
//	}
	return (SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] & (1 << (y & 7)));
//	}
//	return 2; // Pixel out of bounds
}
uint8_t * SSD1306_getBuffer(void){
	return SSD1306_Buffer;
}
void SSD1306_drawPixel(int16_t x, int16_t y, uint16_t color){
	if ((x >= 0) && (x < SSD1306_WIDTH) && (y >= 0) && (y < SSD1306_HEIGHT)) {
//    // Pixel is in-bounds. Rotate coordinates if needed.
//    switch (getRotation()) {
//    case 1:
//      ssd1306_swap(x, y);
//      x = WIDTH - x - 1;
//      break;
//    case 2:
//      x = WIDTH - x - 1;
//      y = HEIGHT - y - 1;
//      break;
//    case 3:
//      ssd1306_swap(x, y);
//      y = HEIGHT - y - 1;
//      break;
//    }
    switch (color) {
    case SSD1306_WHITE:
      SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y & 7));
      break;
    case SSD1306_BLACK:
      SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y & 7));
      break;
    case SSD1306_INVERSE:
      SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] ^= (1 << (y & 7));
      break;
    }
  }


}

void SSD1306_drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color){
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;
	while (x < y) {
		if (f >= 0) {
		  y--;
		  ddF_y += 2;
		  f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		if (cornername & 0x4) {
			SSD1306_drawPixel(x0 + x, y0 + y, color);
			SSD1306_drawPixel(x0 + y, y0 + x, color);
		}
		if (cornername & 0x2) {
			SSD1306_drawPixel(x0 + x, y0 - y, color);
			SSD1306_drawPixel(x0 + y, y0 - x, color);
		}
		if (cornername & 0x8) {
			SSD1306_drawPixel(x0 - y, y0 + x, color);
			SSD1306_drawPixel(x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) {
			SSD1306_drawPixel(x0 - y, y0 - x, color);
			SSD1306_drawPixel(x0 - x, y0 - y, color);
		}
	}
}

void SSD1306_fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color){
	  SSD1306_drawFastVLine(x0, y0 - r, 2 * r + 1, color);
	  SSD1306_fillCircleHelper(x0, y0, r, 3, 0, color);
}

void SSD1306_fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color){
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;
	int16_t px = x;
	int16_t py = y;

	delta++; // Avoid some +1's in the loop

	while (x < y) {
		if (f >= 0) {
		  y--;
		  ddF_y += 2;
		  f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		// These checks avoid double-drawing certain lines, important
		// for the SSD1306 library which has an INVERT drawing mode.
		if (x < (y + 1)) {
		  if (corners & 1)
			  SSD1306_drawFastVLine(x0 + x, y0 - y, 2 * y + delta, color);
		  if (corners & 2)
			  SSD1306_drawFastVLine(x0 - x, y0 - y, 2 * y + delta, color);
		}
		if (y != py) {
		  if (corners & 1)
			  SSD1306_drawFastVLine(x0 + py, y0 - px, 2 * px + delta, color);
		  if (corners & 2)
			  SSD1306_drawFastVLine(x0 - py, y0 - px, 2 * px + delta, color);
		  py = y;
		}
		px = x;
	}
}

void SSD1306_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color){
	SSD1306_drawLine(x0, y0, x1, y1, color);
	SSD1306_drawLine(x1, y1, x2, y2, color);
	SSD1306_drawLine(x2, y2, x0, y0, color);
}

void SSD1306_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color){
	  int16_t a, b, y, last;

	  // Sort coordinates by Y order (y2 >= y1 >= y0)
	  if (y0 > y1) {
	    _swap_int16_t(y0, y1);
	    _swap_int16_t(x0, x1);
	  }
	  if (y1 > y2) {
	    _swap_int16_t(y2, y1);
	    _swap_int16_t(x2, x1);
	  }
	  if (y0 > y1) {
	    _swap_int16_t(y0, y1);
	    _swap_int16_t(x0, x1);
	  }

	  if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
	    a = b = x0;
	    if (x1 < a)
	      a = x1;
	    else if (x1 > b)
	      b = x1;
	    if (x2 < a)
	      a = x2;
	    else if (x2 > b)
	      b = x2;
	    SSD1306_drawFastHLine(a, y0, b - a + 1, color);
	    return;
	  }

	  int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
	          dx12 = x2 - x1, dy12 = y2 - y1;
	  int32_t sa = 0, sb = 0;

	  // For upper part of triangle, find scanline crossings for segments
	  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	  // is included here (and second loop will be skipped, avoiding a /0
	  // error there), otherwise scanline y1 is skipped here and handled
	  // in the second loop...which also avoids a /0 error here if y0=y1
	  // (flat-topped triangle).
	  if (y1 == y2)
	    last = y1; // Include y1 scanline
	  else
	    last = y1 - 1; // Skip it

	  for (y = y0; y <= last; y++) {
	    a = x0 + sa / dy01;
	    b = x0 + sb / dy02;
	    sa += dx01;
	    sb += dx02;
	    /* longhand:
	    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
	    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
	    */
	    if (a > b)
	      _swap_int16_t(a, b);
	      SSD1306_drawFastHLine(a, y, b - a + 1, color);
	  }

	  // For lower part of triangle, find scanline crossings for segments
	  // 0-2 and 1-2.  This loop is skipped if y1=y2.
	  sa = (int32_t)dx12 * (y - y1);
	  sb = (int32_t)dx02 * (y - y0);
	  for (; y <= y2; y++) {
	    a = x1 + sa / dy12;
	    b = x0 + sb / dy02;
	    sa += dx12;
	    sb += dx02;
	    /* longhand:
	    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
	    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
	    */
	    if (a > b)
	      _swap_int16_t(a, b);
	      SSD1306_drawFastHLine(a, y, b - a + 1, color);
	  }

}

void SSD1306_drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color){
	  int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
	  if (r > max_radius)
	    r = max_radius;
	  // smarter version
	  SSD1306_drawFastHLine(x + r, y, w - 2 * r, color);         // Top
	  SSD1306_drawFastHLine(x + r, y + h - 1, w - 2 * r, color); // Bottom
	  SSD1306_drawFastVLine(x, y + r, h - 2 * r, color);         // Left
	  SSD1306_drawFastVLine(x + w - 1, y + r, h - 2 * r, color); // Right
	  // draw four corners
	  SSD1306_drawCircleHelper(x + r, y + r, r, 1, color);
	  SSD1306_drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
	  SSD1306_drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
	  SSD1306_drawCircleHelper(x + r, y + h - r - 1, r, 8, color);

}

void SSD1306_fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color){
	  int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
	  if (r > max_radius)
	    r = max_radius;
	  // smarter version
	  SSD1306_fillRect(x + r, y, w - 2 * r, h, color);
	  // draw four corners
	  SSD1306_fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
	  SSD1306_fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

void SSD1306_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
	  for (int16_t i = x; i < x + w; i++) {
	    SSD1306_drawFastVLine(i, y, h, color);
	  }
}
void SSD1306_drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color){
	  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
	  uint8_t b = 0;

	  for (int16_t j = 0; j < h; j++, y++) {
	    for (int16_t i = 0; i < w; i++) {
	      if (i & 7)
	        b <<= 1;
	      else
	        b = bitmap[j * byteWidth + i / 8];
	      if (b & 0x80)
	        SSD1306_drawPixel(x + i, y, color);
	    }
	  }

}
char SSD1306_drawChar(int16_t x, int16_t y, char ch, FontDef_t* Font, uint16_t color) {
	uint32_t i, b, j;

	/* Check available space in LCD */
	if (
		SSD1306_WIDTH <= (x + Font->FontWidth) ||
		SSD1306_HEIGHT <= (y + Font->FontHeight)
	) {
		/* Error */
		return 0;
	}

	/* Go through font */
	for (i = 0; i < Font->FontHeight; i++) {
		b = Font->data[(ch - 32) * Font->FontHeight + i];
		for (j = 0; j < Font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				SSD1306_drawPixel(x + j, (y + i),  color);
			} else {
				SSD1306_drawPixel(x + j, (y + i), !color);
			}
		}
	}
	/* Return character written */
	return ch;
}

char SSD1306_print(int16_t x, int16_t y, char* str, FontDef_t* Font, uint16_t color){
	/* Write characters */
	while (*str) {
		/* Write character by character */
		if (SSD1306_drawChar(x, y, *str, Font, color) != *str) {
			/* Return error */
			return *str;
		}

		/* Increase string pointer */
		str++;
		x+=Font->FontWidth;
	}

	/* Everything OK, zero should be returned */
	return *str;

}

void SSD1306_drawArc(int16_t x, int16_t y, int16_t start_angle, int16_t seg_count, int16_t rx, int16_t ry, int16_t w, uint16_t color){
    uint8_t seg = 1;
    uint8_t inc = 1;
    // Calculate first pair of coordinates for segment start
    float sx = cos((start_angle - 90) * DEG_TO_RAD);
    float sy = sin((start_angle - 90) * DEG_TO_RAD);
    uint16_t x0 = sx * (rx - w) + x;
    uint16_t y0 = sy * (ry - w) + y;
    uint16_t x1 = sx * rx + x;
    uint16_t y1 = sy * ry + y;
    uint16_t x2,y2,x3,y3;
    float sx2,sy2;
    // Draw colour blocks every inc degrees
    for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {
        // Calculate pair of coordinates for segment end
        sx2 = cos((i + seg - 90) * DEG_TO_RAD);
        sy2 = sin((i + seg - 90) * DEG_TO_RAD);
        x2 = sx2 * (rx - w) + x;
        y2 = sy2 * (ry - w) + y;
        x3 = sx2 * rx + x;
        y3 = sy2 * ry + y;
        SSD1306_fillTriangle(x0, y0, x1, y1, x2, y2, color);
        SSD1306_fillTriangle(x1, y1, x2, y2, x3, y3, color);
		// Copy segment end to sgement start for next segment
		x0 = x2;
		y0 = y2;
		x1 = x3;
		y1 = y3;
    }
}

void SSD1306_drawCircleDegree(int16_t x, int16_t y, int16_t radius, int16_t degree , int16_t degreeOld, int16_t width , int16_t color){
    if (degree < 0){
        degree = 0;
    }else if(degree >= 359){
        degree = 359;
    }
    if(degreeOld == 0){//draw complete Circle
    	SSD1306_drawArc(x, y, 0, degree, radius, radius, width, color);
    	SSD1306_drawArc(x, y, degree, 359 - degree , radius, radius, width, SSD1306_BLACK);

    }
    else if (degree == degreeOld){
        return;
    }
    else if(degree < degreeOld){
    	SSD1306_drawArc(x, y, degree, degreeOld - degree , radius, radius, width, SSD1306_BLACK);
    }
    else{
    	SSD1306_drawArc(x, y, degreeOld, degree - degreeOld , radius, radius, width, color);

    }
}

void SSD1306_drawVerticalBarChart(float curval, float x , float y ,float w, float h , float loval , float hival , float inc)
{
	float stepval, my,  i, level, data,tmp;
	// step val basically scales the hival and low val to the height
	// deducting a small value to eliminate round off errors
	// this val may need to be adjusted
	if(curval > 99999 || hival > 99999) return; // labeltext[5] can hold maximal 99999
	stepval = inc * (h / (hival - loval)) - 0.001;
	for (i = 0; i <= h; i += stepval) {
		my =  y - h + i;
		SSD1306_drawFastHLine(x + w + 1, my,  5, SSD1306_WHITE);
		// draw lables
		tmp = inc / stepval;
		data = hival -  i * tmp;
		memset(labeltext,' ',sizeof(labeltext));
		_float_to_char(data, labeltext);
		SSD1306_print(x + w + 12, my - 3, labeltext, &Font_7x10,SSD1306_WHITE);
	}
	// compute level of bar graph that is scaled to the  height and the hi and low vals
	  // this is needed to accompdate for +/- range
	level = (h * (((curval - loval) / (hival - loval))));
	// draw the bar graph
	// write a upper and lower bar to minimize flicker cause by blanking out bar and redraw on update
	SSD1306_drawRect(x, y - h, w, h, SSD1306_WHITE);
	SSD1306_fillRect(x, y - h, w, h - level,  SSD1306_BLACK);
	SSD1306_drawRect(x, y - h, w, h, SSD1306_WHITE);
	SSD1306_fillRect(x, y - level, w,  level, SSD1306_WHITE);
	// up until now print sends data to a video buffer NOT the screen
	// this call sends the data to the screen
	//SSD1306_display();
}
void SSD1306_drawVerticalBar(float curval, float x , float y ,float w, float h , float loval , float hival , float inc)
{
	float stepval, my,  i, level, data,tmp;
	// step val basically scales the hival and low val to the height
	// deducting a small value to eliminate round off errors
	// this val may need to be adjusted
	// compute level of bar graph that is scaled to the  height and the hi and low vals
	  // this is needed to accompdate for +/- range
	level = (h * (((curval - loval) / (hival - loval))));
	// draw the bar graph
	// write a upper and lower bar to minimize flicker cause by blanking out bar and redraw on update
	SSD1306_drawRect(x, y - h, w, h, SSD1306_WHITE);
	SSD1306_fillRect(x, y - h, w, h - level,  SSD1306_BLACK);
	SSD1306_drawRect(x, y - h, w, h, SSD1306_WHITE);
	SSD1306_fillRect(x, y - level, w,  level, SSD1306_WHITE);
	// up until now print sends data to a video buffer NOT the screen
	// this call sends the data to the screen
	//SSD1306_display();
}

void SSD1306_drawHorizontalBarChart(float curval, float x , float y , float w, float h , float loval , float hival , float inc)
{
	float stepval, mx, level, i, data, tmp;


    // step val basically scales the hival and low val to the width
	if(curval > 99999 || hival > 99999) return; // labeltext[5] can hold maximal 99999
    stepval =  inc * (w / (hival - loval)) - .00001;
    // draw the text
    for (i = 0; i <= w; i += stepval) {
    	SSD1306_drawFastVLine(i + x , y ,  5, SSD1306_WHITE);
      // draw lables
      // addling a small value to eliminate round off errors
      // this val may need to be adjusted
		tmp = inc / stepval;
        data =  ( i * tmp) + loval + 0.00001;
		memset(labeltext,' ',sizeof(labeltext));
		_float_to_char(data, labeltext);
		ptr_val = &labeltext[1];
	    while(*ptr_val == ' '){
	    	ptr_val++;
	    }
        SSD1306_print(i + x, y + 10, ptr_val , &Font_7x10,SSD1306_WHITE);
    }

  // compute level of bar graph that is scaled to the width and the hi and low vals
  // this is needed to accompdate for +/- range capability
  // draw the bar graph
  // write a upper and lower bar to minimize flicker cause by blanking out bar and redraw on update
	level = (w * (((curval - loval) / (hival - loval))));
	SSD1306_fillRect(x + level, y - h, w - level, h,  SSD1306_BLACK);
	SSD1306_drawRect(x, y - h, w,  h, SSD1306_WHITE);
	SSD1306_fillRect(x, y - h, level,  h, SSD1306_WHITE);
	// up until now print sends data to a video buffer NOT the screen
	// this call sends the data to the screen
	//d.display();

}

void SSD1306_drawHorizontalBar(float curval, float x , float y , float w, float h , float loval , float hival , float inc)
{
	float stepval, mx, level, i, data, tmp;


    // step val basically scales the hival and low val to the width
  // compute level of bar graph that is scaled to the width and the hi and low vals
  // this is needed to accompdate for +/- range capability
  // draw the bar graph
  // write a upper and lower bar to minimize flicker cause by blanking out bar and redraw on update
	level = (w * (((curval - loval) / (hival - loval))));
	SSD1306_fillRect(x + level, y - h, w - level, h,  SSD1306_BLACK);
	SSD1306_drawRect(x, y - h, w,  h, SSD1306_WHITE);
	SSD1306_fillRect(x, y - h, level,  h, SSD1306_WHITE);
	// up until now print sends data to a video buffer NOT the screen
	// this call sends the data to the screen
	//d.display();

}

void SSD1306_plotData(float *xData,float *yData,int size, float xlo, float xhi, uint8_t xNumofSplit, float ylo, float yhi, uint8_t yNumofSplit){
   float x_inc = (xhi-xlo)/xNumofSplit;
   float y_inc = (yhi-ylo)/yNumofSplit;
   SSD1306_clearDisplay();
   SSD1306_drawCGraph(xData[0], yData[0], 30, 40, 75, 30, xlo, xhi, x_inc, ylo, yhi, y_inc,1);
   for(uint16_t i=1; i < size;i++){
	   SSD1306_drawCGraph(xData[i], yData[i], 30, 40, 75, 30, xlo, xhi, x_inc, ylo, yhi, y_inc,0);
   }
}

void SSD1306_drawCGraph(float x, float y, float gx, float gy, float w, float h, float xlo, float xhi, float xinc, float ylo, float yhi, double yinc, uint8_t drawAchse) {

  float i;
  float temp;
  int rot, newrot;

  if (drawAchse == 1) {
    ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
    oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
    // draw y scale
    for ( i = ylo; i <= yhi; i += yinc) {
      // compute the transform
      // note my transform funcition is the same as the map function, except the map uses long and we nfloatubles
      temp =  (i - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
      if (i == 0) {
    	  SSD1306_drawFastHLine(gx - 3, temp, w + 3, SSD1306_WHITE);
      }
      else {
    	  SSD1306_drawFastHLine(gx - 3, temp, 3, SSD1306_WHITE);
      }
	  memset(labeltext,' ',sizeof(labeltext));
	  _float_to_char(i, labeltext);
	  ptr_val = &labeltext[1];
	  while(*ptr_val == ' '){
		ptr_val++;
	 }

     SSD1306_print(gx - 27, temp - 3,ptr_val,&Font_7x10,SSD1306_WHITE);
    }
    // draw x scale
    for (i = xlo; i <= xhi; i += xinc) {
      // compute the transform
      temp =  (i - xlo) * ( w) / (xhi - xlo) + gx;
      if (i == 0) {
    	  SSD1306_drawFastVLine(temp, gy - h, h + 3, SSD1306_WHITE);
      }
      else {
    	  SSD1306_drawFastVLine(temp, gy, 3, SSD1306_WHITE);
      }
	  memset(labeltext,' ',sizeof(labeltext));
	  _float_to_char(i, labeltext);
	  ptr_val = &labeltext[1];
	  while(*ptr_val == ' '){
		ptr_val++;
	 }
      SSD1306_print(temp, gy + 6,ptr_val,&Font_7x10,SSD1306_WHITE);
    }
  }

  // graph drawn now plot the data
  // the entire plotting code are these few lines...

  x =  (x - xlo) * ( w) / (xhi - xlo) + gx;
  y =  (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  SSD1306_drawLine(ox, oy, x, y, SSD1306_WHITE);
  SSD1306_drawLine(ox, oy - 1, x, y - 1, SSD1306_WHITE);
  ox = x;
  oy = y;

  // up until now print sends data to a video buffer NOT the screen
  // this call sends the data to the screen
  //d.display();

}



void SSD1306_I2C_WriteMultiConstByte(uint8_t address, uint8_t reg, const uint8_t* data, uint16_t count) {
	uint8_t data_send[255];
	data_send[0] = reg;
	for(uint8_t i = 0; i < count; i++){
		data_send[i+1] = data[i];
	}
	//HAL_I2C_Master_Transmit(&hi2c1, address, dt, count+1, 10);
	SW_I2C_Write_8addr(1, address, data_send[0], &data_send[1], count);
}
void SSD1306_I2C_WriteMultiByte(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) {
	SW_I2C_Write_8addr(1, address, reg, data, count);
}


void SSD1306_I2C_WriteByte(uint8_t address, uint8_t reg, uint8_t data) {
	uint8_t data_send[2];
	data_send[0] = reg;
	data_send[1] = data;
	SW_I2C_Write_8addr(1, address, data_send[0], &data_send[1], 1);
	//HAL_I2C_Master_Transmit(&hi2c1, address, dt, 2, 10);
}
