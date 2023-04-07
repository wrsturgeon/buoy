#include "big-ascii.h"
#include "lcd.h"

#define IMPLFN(FNNAME) void big_##FNNAME(uint8_t x, uint8_t y, uint16_t color)

void big_integer(uint8_t i, uint8_t x, uint8_t y, uint16_t color) {
  // Array of function pointers indexed into & called in O(1)
  return (big_ascii_fn[]){big_0, big_1, big_2, big_3, big_4, big_5, big_6, big_7, big_8, big_9}[i](x, y, color);
}

IMPLFN(heart) {
  // Symmetric
  // y: +0 / +6
  for (uint8_t i = 1; i != 6; ++i) {
    lcd_pixel(x - i, y, color);
    lcd_pixel(x - i, y + 6, color);
  }
  // y: +1 / +5
  for (uint8_t i = 0; i != 7; ++i) {
    lcd_pixel(x - i, y + 1, color);
    lcd_pixel(x - i, y + 5, color);
  }
  // y: +2 / +4
  for (uint8_t i = 1; i != 8; ++i) {
    lcd_pixel(x - i, y + 2, color);
    lcd_pixel(x - i, y + 4, color);
  }
  // y: +3
  for (uint8_t i = 2; i != 9; ++i) {
    lcd_pixel(x - i, y + 3, color);
  }
}

IMPLFN(question) {
  lcd_pixel(x - 8, y + 3, color);
  lcd_pixel(x - 8, y + 4, color);
  lcd_pixel(x - 7, y + 3, color);
  lcd_pixel(x - 7, y + 4, color);
  lcd_pixel(x - 5, y + 3, color);
  lcd_pixel(x - 5, y + 4, color);
  lcd_pixel(x - 4, y + 3, color);
  lcd_pixel(x - 4, y + 4, color);
  lcd_pixel(x - 3, y + 4, color);
  lcd_pixel(x - 3, y + 5, color);
  lcd_pixel(x - 2, y + 5, color);
  lcd_pixel(x - 2, y + 6, color);
  lcd_pixel(x - 1, y + 5, color);
  lcd_pixel(x - 1, y + 6, color);
  lcd_pixel(x, y + 1, color);
  lcd_pixel(x, y + 2, color);
  lcd_pixel(x, y + 3, color);
  lcd_pixel(x, y + 4, color);
  lcd_pixel(x, y + 5, color);
  lcd_pixel(x - 1, y, color);
  lcd_pixel(x - 1, y + 1, color);
  lcd_pixel(x - 2, y, color);
  lcd_pixel(x - 2, y + 1, color);
}

IMPLFN(lparen) {
  lcd_pixel(x, y + 5, color);
  lcd_pixel(x, y + 6, color);
  lcd_pixel(x - 1, y + 4, color);
  lcd_pixel(x - 1, y + 5, color);
  lcd_pixel(x - 2, y + 4, color);
  lcd_pixel(x - 2, y + 5, color);
  lcd_pixel(x - 3, y + 4, color);
  lcd_pixel(x - 3, y + 5, color);
  lcd_pixel(x - 4, y + 4, color);
  lcd_pixel(x - 4, y + 5, color);
  lcd_pixel(x - 5, y + 4, color);
  lcd_pixel(x - 5, y + 5, color);
  lcd_pixel(x - 6, y + 4, color);
  lcd_pixel(x - 6, y + 5, color);
  lcd_pixel(x - 7, y + 4, color);
  lcd_pixel(x - 7, y + 5, color);
  lcd_pixel(x - 8, y + 5, color);
  lcd_pixel(x - 8, y + 6, color);
}

IMPLFN(rparen) {
  lcd_pixel(x, y, color);
  lcd_pixel(x, y + 1, color);
  lcd_pixel(x - 1, y + 1, color);
  lcd_pixel(x - 1, y + 2, color);
  lcd_pixel(x - 2, y + 1, color);
  lcd_pixel(x - 2, y + 2, color);
  lcd_pixel(x - 3, y + 1, color);
  lcd_pixel(x - 3, y + 2, color);
  lcd_pixel(x - 4, y + 1, color);
  lcd_pixel(x - 4, y + 2, color);
  lcd_pixel(x - 5, y + 1, color);
  lcd_pixel(x - 5, y + 2, color);
  lcd_pixel(x - 6, y + 1, color);
  lcd_pixel(x - 6, y + 2, color);
  lcd_pixel(x - 7, y + 1, color);
  lcd_pixel(x - 7, y + 2, color);
  lcd_pixel(x - 8, y, color);
  lcd_pixel(x - 8, y + 1, color);
}

IMPLFN(0) {
  // Symmetric in both dimensions
  for (uint8_t i = 1; i != 8; ++i) {
    lcd_pixel(x - i, y, color);
    lcd_pixel(x - i, y + 1, color);
    lcd_pixel(x - i, y + 5, color);
    lcd_pixel(x - i, y + 6, color);
  }
  for (uint8_t j = 1; j != 6; ++j) {
    lcd_pixel(x, y + j, color);
    lcd_pixel(x - 8, y + j, color);
  }
}

IMPLFN(1) {
  for (uint8_t i = 0; i != 9; ++i) {
    for (uint8_t j = 3; j != 5; ++j) {
      lcd_pixel(x - i, y + j, color);
      lcd_pixel(x - i, y + j, color);
    }
  }
  lcd_pixel(x - 1, y + 1, color);
  lcd_pixel(x, y + 2, color);
  lcd_pixel(x - 8, y, color);
  lcd_pixel(x - 8, y + 1, color);
  lcd_pixel(x - 8, y + 2, color);
  lcd_pixel(x - 8, y + 5, color);
  lcd_pixel(x - 8, y + 6, color);
}

IMPLFN(2) {
  lcd_pixel(x - 8, y, color);
  lcd_pixel(x - 8, y + 1, color);
  lcd_pixel(x - 8, y + 2, color);
  lcd_pixel(x - 8, y + 3, color);
  lcd_pixel(x - 8, y + 4, color);
  lcd_pixel(x - 8, y + 5, color);
  lcd_pixel(x - 8, y + 6, color);
  lcd_pixel(x - 7, y, color);
  lcd_pixel(x - 7, y + 1, color);
  lcd_pixel(x - 7, y + 5, color);
  lcd_pixel(x - 7, y + 6, color);
  lcd_pixel(x - 6, y + 1, color);
  lcd_pixel(x - 6, y + 2, color);
  lcd_pixel(x - 6, y + 5, color);
  lcd_pixel(x - 6, y + 6, color);
  lcd_pixel(x - 5, y + 2, color);
  lcd_pixel(x - 5, y + 3, color);
  lcd_pixel(x - 4, y + 3, color);
  lcd_pixel(x - 4, y + 4, color);
  lcd_pixel(x - 3, y + 4, color);
  lcd_pixel(x - 3, y + 5, color);
  lcd_pixel(x - 2, y + 5, color);
  lcd_pixel(x - 2, y + 6, color);
  lcd_pixel(x - 1, y + 5, color);
  lcd_pixel(x - 1, y + 6, color);
  lcd_pixel(x, y + 1, color);
  lcd_pixel(x, y + 2, color);
  lcd_pixel(x, y + 3, color);
  lcd_pixel(x, y + 4, color);
  lcd_pixel(x, y + 5, color);
  lcd_pixel(x - 1, y, color);
  lcd_pixel(x - 1, y + 1, color);
  lcd_pixel(x - 2, y, color);
  lcd_pixel(x - 2, y + 1, color);
}

IMPLFN(3) {
  lcd_pixel(x, y + 1, color);
  lcd_pixel(x, y + 2, color);
  lcd_pixel(x, y + 3, color);
  lcd_pixel(x, y + 4, color);
  lcd_pixel(x, y + 5, color);
  lcd_pixel(x - 1, y, color);
  lcd_pixel(x - 1, y + 1, color);
  lcd_pixel(x - 1, y + 5, color);
  lcd_pixel(x - 1, y + 6, color);
  lcd_pixel(x - 2, y, color);
  lcd_pixel(x - 2, y + 1, color);
  lcd_pixel(x - 2, y + 5, color);
  lcd_pixel(x - 2, y + 6, color);
  lcd_pixel(x - 3, y + 5, color);
  lcd_pixel(x - 3, y + 6, color);
  lcd_pixel(x - 4, y + 2, color);
  lcd_pixel(x - 4, y + 3, color);
  lcd_pixel(x - 4, y + 4, color);
  lcd_pixel(x - 4, y + 5, color);
  lcd_pixel(x - 5, y + 5, color);
  lcd_pixel(x - 5, y + 6, color);
  lcd_pixel(x - 6, y, color);
  lcd_pixel(x - 6, y + 1, color);
  lcd_pixel(x - 6, y + 5, color);
  lcd_pixel(x - 6, y + 6, color);
  lcd_pixel(x - 7, y, color);
  lcd_pixel(x - 7, y + 1, color);
  lcd_pixel(x - 7, y + 5, color);
  lcd_pixel(x - 7, y + 6, color);
  lcd_pixel(x - 8, y + 1, color);
  lcd_pixel(x - 8, y + 2, color);
  lcd_pixel(x - 8, y + 3, color);
  lcd_pixel(x - 8, y + 4, color);
  lcd_pixel(x - 8, y + 5, color);
}

IMPLFN(4) {
  for (uint8_t i = 0; i != 9; ++i) {
    for (uint8_t j = 5; j != 7; ++j) {
      lcd_pixel(x - i, y + j, color);
      lcd_pixel(x - i, y + j, color);
    }
  }
  for (uint8_t i = 0; i != 4; ++i) {
    for (uint8_t j = 0; j != 2; ++j) {
      lcd_pixel(x - i, y + j, color);
      lcd_pixel(x - i, y + j, color);
    }
  }
  lcd_pixel(x - 4, y + 1, color);
  lcd_pixel(x - 4, y + 2, color);
  lcd_pixel(x - 4, y + 3, color);
  lcd_pixel(x - 4, y + 4, color);
}

IMPLFN(5) {
  for (uint8_t i = 0; i != 5; ++i) {
    for (uint8_t j = 0; j != 2; ++j) {
      lcd_pixel(x - i, y + j, color);
      lcd_pixel(x - i, y + j, color);
    }
  }
  lcd_pixel(x, y + 2, color);
  lcd_pixel(x, y + 3, color);
  lcd_pixel(x, y + 4, color);
  lcd_pixel(x, y + 5, color);
  lcd_pixel(x, y + 6, color);
  lcd_pixel(x - 1, y + 5, color);
  lcd_pixel(x - 1, y + 6, color);
  lcd_pixel(x - 2, y + 5, color);
  lcd_pixel(x - 2, y + 6, color);
  lcd_pixel(x - 4, y + 2, color);
  lcd_pixel(x - 4, y + 3, color);
  lcd_pixel(x - 4, y + 4, color);
  lcd_pixel(x - 4, y + 5, color);
  lcd_pixel(x - 5, y + 5, color);
  lcd_pixel(x - 5, y + 6, color);
  lcd_pixel(x - 6, y + 5, color);
  lcd_pixel(x - 6, y + 6, color);
  lcd_pixel(x - 7, y + 5, color);
  lcd_pixel(x - 7, y + 6, color);
  lcd_pixel(x - 8, y + 1, color);
  lcd_pixel(x - 8, y + 2, color);
  lcd_pixel(x - 8, y + 3, color);
  lcd_pixel(x - 8, y + 4, color);
  lcd_pixel(x - 8, y + 5, color);
  lcd_pixel(x - 7, y, color);
  lcd_pixel(x - 7, y + 1, color);
  lcd_pixel(x - 6, y, color);
  lcd_pixel(x - 6, y + 1, color);
}

IMPLFN(6) {
  for (uint8_t i = 1; i != 8; ++i) {
    lcd_pixel(x - i, y, color);
    lcd_pixel(x - i, y + 1, color);
  }
  for (uint8_t j = 1; j != 6; ++j) {
    lcd_pixel(x, y + j, color);
    lcd_pixel(x - 8, y + j, color);
  }
  lcd_pixel(x - 1, y + 5, color);
  lcd_pixel(x - 1, y + 6, color);
  lcd_pixel(x - 2, y + 5, color);
  lcd_pixel(x - 2, y + 6, color);
  lcd_pixel(x - 4, y + 2, color);
  lcd_pixel(x - 4, y + 3, color);
  lcd_pixel(x - 4, y + 4, color);
  lcd_pixel(x - 4, y + 5, color);
  lcd_pixel(x - 5, y + 5, color);
  lcd_pixel(x - 5, y + 6, color);
  lcd_pixel(x - 6, y + 5, color);
  lcd_pixel(x - 6, y + 6, color);
  lcd_pixel(x - 7, y + 5, color);
  lcd_pixel(x - 7, y + 6, color);
}

IMPLFN(7) {
  lcd_pixel(x, y + 6, color);
  lcd_pixel(x, y + 5, color);
  lcd_pixel(x, y + 4, color);
  lcd_pixel(x, y + 3, color);
  lcd_pixel(x, y + 2, color);
  lcd_pixel(x, y + 1, color);
  lcd_pixel(x, y, color);
  lcd_pixel(x - 1, y + 5, color);
  lcd_pixel(x - 1, y + 6, color);
  lcd_pixel(x - 1, y + 1, color);
  lcd_pixel(x - 1, y, color);
  lcd_pixel(x - 2, y + 5, color);
  lcd_pixel(x - 2, y + 6, color);
  lcd_pixel(x - 2, y + 1, color);
  lcd_pixel(x - 2, y, color);
  lcd_pixel(x - 3, y + 4, color);
  lcd_pixel(x - 3, y + 5, color);
  lcd_pixel(x - 4, y + 3, color);
  lcd_pixel(x - 4, y + 4, color);
  lcd_pixel(x - 5, y + 3, color);
  lcd_pixel(x - 5, y + 4, color);
  lcd_pixel(x - 6, y + 3, color);
  lcd_pixel(x - 6, y + 4, color);
  lcd_pixel(x - 7, y + 3, color);
  lcd_pixel(x - 7, y + 4, color);
  lcd_pixel(x - 8, y + 3, color);
  lcd_pixel(x - 8, y + 4, color);
}

IMPLFN(8) {
  lcd_pixel(x, y + 1, color);
  lcd_pixel(x, y + 2, color);
  lcd_pixel(x, y + 3, color);
  lcd_pixel(x, y + 4, color);
  lcd_pixel(x, y + 5, color);
  lcd_pixel(x - 1, y, color);
  lcd_pixel(x - 1, y + 1, color);
  lcd_pixel(x - 1, y + 5, color);
  lcd_pixel(x - 1, y + 6, color);
  lcd_pixel(x - 2, y, color);
  lcd_pixel(x - 2, y + 1, color);
  lcd_pixel(x - 2, y + 5, color);
  lcd_pixel(x - 2, y + 6, color);
  lcd_pixel(x - 3, y, color);
  lcd_pixel(x - 3, y + 1, color);
  lcd_pixel(x - 3, y + 5, color);
  lcd_pixel(x - 3, y + 6, color);
  lcd_pixel(x - 4, y + 1, color);
  lcd_pixel(x - 4, y + 2, color);
  lcd_pixel(x - 4, y + 3, color);
  lcd_pixel(x - 4, y + 4, color);
  lcd_pixel(x - 4, y + 5, color);
  lcd_pixel(x - 5, y, color);
  lcd_pixel(x - 5, y + 1, color);
  lcd_pixel(x - 5, y + 5, color);
  lcd_pixel(x - 5, y + 6, color);
  lcd_pixel(x - 6, y, color);
  lcd_pixel(x - 6, y + 1, color);
  lcd_pixel(x - 6, y + 5, color);
  lcd_pixel(x - 6, y + 6, color);
  lcd_pixel(x - 7, y, color);
  lcd_pixel(x - 7, y + 1, color);
  lcd_pixel(x - 7, y + 5, color);
  lcd_pixel(x - 7, y + 6, color);
  lcd_pixel(x - 8, y + 1, color);
  lcd_pixel(x - 8, y + 2, color);
  lcd_pixel(x - 8, y + 3, color);
  lcd_pixel(x - 8, y + 4, color);
  lcd_pixel(x - 8, y + 5, color);
}

IMPLFN(9) {
  lcd_pixel(x, y + 1, color);
  lcd_pixel(x, y + 2, color);
  lcd_pixel(x, y + 3, color);
  lcd_pixel(x, y + 4, color);
  lcd_pixel(x, y + 5, color);
  lcd_pixel(x, y + 6, color);
  lcd_pixel(x - 1, y, color);
  lcd_pixel(x - 1, y + 1, color);
  lcd_pixel(x - 1, y + 5, color);
  lcd_pixel(x - 1, y + 6, color);
  lcd_pixel(x - 2, y, color);
  lcd_pixel(x - 2, y + 1, color);
  lcd_pixel(x - 2, y + 5, color);
  lcd_pixel(x - 2, y + 6, color);
  lcd_pixel(x - 3, y, color);
  lcd_pixel(x - 3, y + 1, color);
  lcd_pixel(x - 3, y + 5, color);
  lcd_pixel(x - 3, y + 6, color);
  lcd_pixel(x - 4, y + 1, color);
  lcd_pixel(x - 4, y + 2, color);
  lcd_pixel(x - 4, y + 3, color);
  lcd_pixel(x - 4, y + 4, color);
  lcd_pixel(x - 4, y + 5, color);
  lcd_pixel(x - 4, y + 6, color);
  lcd_pixel(x - 5, y + 5, color);
  lcd_pixel(x - 5, y + 6, color);
  lcd_pixel(x - 6, y + 5, color);
  lcd_pixel(x - 6, y + 6, color);
  lcd_pixel(x - 7, y + 5, color);
  lcd_pixel(x - 7, y + 6, color);
  lcd_pixel(x - 8, y + 5, color);
  lcd_pixel(x - 8, y + 6, color);
}

IMPLFN(B) {
  // left stroke
  for (uint8_t i = 0; i != 9; ++i) {
    for (uint8_t j = 0; j != 2; ++j) {
      lcd_pixel(x - i, y + j, color);
      lcd_pixel(x - i, y + j, color);
    }
  }
  // sideways mcdonalds arches
  lcd_pixel(x, y + 2, color);
  lcd_pixel(x, y + 3, color);
  lcd_pixel(x, y + 4, color);
  lcd_pixel(x, y + 5, color);
  lcd_pixel(x - 1, y + 5, color);
  lcd_pixel(x - 1, y + 6, color);
  lcd_pixel(x - 2, y + 5, color);
  lcd_pixel(x - 2, y + 6, color);
  lcd_pixel(x - 3, y + 5, color);
  lcd_pixel(x - 3, y + 6, color);
  lcd_pixel(x - 4, y + 2, color);
  lcd_pixel(x - 4, y + 3, color);
  lcd_pixel(x - 4, y + 4, color);
  lcd_pixel(x - 4, y + 5, color);
  lcd_pixel(x - 5, y + 5, color);
  lcd_pixel(x - 5, y + 6, color);
  lcd_pixel(x - 6, y + 5, color);
  lcd_pixel(x - 6, y + 6, color);
  lcd_pixel(x - 7, y + 5, color);
  lcd_pixel(x - 7, y + 6, color);
  lcd_pixel(x - 8, y + 2, color);
  lcd_pixel(x - 8, y + 3, color);
  lcd_pixel(x - 8, y + 4, color);
  lcd_pixel(x - 8, y + 5, color);
}

IMPLFN(P) {
  // left stroke
  for (uint8_t i = 0; i != 9; ++i) {
    for (uint8_t j = 0; j != 2; ++j) {
      lcd_pixel(x - i, y + j, color);
      lcd_pixel(x - i, y + j, color);
    }
  }
  // sideways mcdonalds arches
  lcd_pixel(x, y + 2, color);
  lcd_pixel(x, y + 3, color);
  lcd_pixel(x, y + 4, color);
  lcd_pixel(x, y + 5, color);
  lcd_pixel(x - 1, y + 5, color);
  lcd_pixel(x - 1, y + 6, color);
  lcd_pixel(x - 2, y + 5, color);
  lcd_pixel(x - 2, y + 6, color);
  lcd_pixel(x - 3, y + 5, color);
  lcd_pixel(x - 3, y + 6, color);
  lcd_pixel(x - 4, y + 2, color);
  lcd_pixel(x - 4, y + 3, color);
  lcd_pixel(x - 4, y + 4, color);
  lcd_pixel(x - 4, y + 5, color);
}

IMPLFN(M) {
  // Symmetric
  // y: +0 / +6
  for (uint8_t i = 0; i != 9; ++i) {
    lcd_pixel(x - i, y, color);
    lcd_pixel(x - i, y + 6, color);
  }
  // y: +1 / +5
  for (uint8_t i = 1; i != 9; ++i) {
    lcd_pixel(x - i, y + 1, color);
    lcd_pixel(x - i, y + 5, color);
  }
  // y: +2 / +4
  lcd_pixel(x - 2, y + 2, color);
  lcd_pixel(x - 2, y + 4, color);
  // y: +3
  for (uint8_t i = 3; i != 5; ++i) {
    lcd_pixel(x - i, y + 3, color);
  }
}
