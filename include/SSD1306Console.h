#pragma once

#include <Print.h>
#include "SSD1306.h"

template<class OLED>
class SSD1306Console : public OLED, public Print {
public:
#ifdef SSD1306_USE_UTF8
  SSD1306Console() : _wcharExtra(0), _col(0), _row(0) {}
#else
  SSD1306Console() : _col(0), _row(0) {}
#endif

  bool begin(bool reset = false);

  uint8_t columns() const;
  uint8_t rows() const;
  uint8_t col() const {
    return _col;
  }
  uint8_t row() const {
    return _row;
  }
  void position(uint8_t c, uint8_t r);
  void home() {
    _col = _row = 0;
  }
  bool clear();

  size_t write(uint8_t c) override;
  using Print::write;

protected:
  static const uint8_t TAB_WIDTH = 4; // 8

  bool writeChar(uint8_t c);
  void preparePosition();

#ifdef SSD1306_USE_UTF8
  utf8_t _wchar;
  uint8_t _wcharExtra : 2;
#endif
  uint8_t _col : 5;
  uint8_t _row : 4;
};

template<class OLED>
bool SSD1306Console<OLED>::begin(bool reset) {
  return OLED::begin(reset) && clear();
}

template<class OLED>
uint8_t SSD1306Console<OLED>::columns() const {
  return this->width() / this->charWidth();
}

template<class OLED>
uint8_t SSD1306Console<OLED>::rows() const {
  return this->height() / this->charHeight();
}

template<class OLED>
void SSD1306Console<OLED>::position(uint8_t c, uint8_t r) {
  if ((c < columns()) && (r < rows())) {
    _col = c;
    _row = r;
  }
}

template<class OLED>
bool SSD1306Console<OLED>::clear() {
  _col = _row = 0;
  return OLED::clear();
}

template<class OLED>
size_t SSD1306Console<OLED>::write(uint8_t c) {
  if (writeChar(c))
    return sizeof(c);
  return 0;
}

template<class OLED>
bool SSD1306Console<OLED>::writeChar(uint8_t c) {
  bool result = true;

  if (c == '\a') { // Negative
    this->_negative = ! this->_negative;
  } else if (c == '\b') { // Backspace
    if (_col)
      --_col;
  } else if (c == '\t') { // Tab
    uint8_t spaces;

    preparePosition();
    spaces = ((_col + TAB_WIDTH) / TAB_WIDTH) * TAB_WIDTH - _col;
    while ((_col < columns()) && spaces--) {
      result = this->printChar(this->charWidth() * _col, this->charHeight() * _row, ' ');
      if (! result)
        break;
      ++_col;
    }
  } else if (c == '\f') { // Form feed (clear)
    result = clear();
  } else if (c == '\r') { // Carriage return
    _col = 0;
  } else if (c == '\n') { // New line
    if (_row < rows())
      ++_row;
    else // Scroll screen
      result = this->scroll(this->charHeight() / 8);
  } else if (c >= ' ') { // Ordinary char
    preparePosition();
#ifdef SSD1306_USE_UTF8
    if (c & 0B10000000) { // UTF8 code
      if (! _wcharExtra) { // First byte
        if ((c & 0B11100000) == 0B11000000) { // 2 bytes code
          _wchar = c & 0B00011111;
          _wcharExtra = 1;
        } else if ((c & 0B11110000) == 0B11100000) { // 3 bytes code
          _wchar = c & 0B00001111;
          _wcharExtra = 2;
        } else { // Error
          _wchar = ' '; // Error
          result = false;
        }
      } else { // Next byte
        if ((c & 0B11000000) == 0B10000000) {
          _wchar <<= 6;
          _wchar |= (c & 0B00111111);
          if (! --_wcharExtra) { // Last byte
            result = this->printChar(this->charWidth() * _col, this->charHeight() * _row, _wchar);
            ++_col;
          }
        } else {
          _wchar = ' '; // Error
          _wcharExtra = 0;
          result = false;
        }
      }
    } else { // ACSII char
      result = this->printChar(this->charWidth() * _col, this->charHeight() * _row, c);
      ++_col;
    }
#else
    result = this->printChar(this->charWidth() * _col, this->charHeight() * _row, c);
    ++_col;
#endif
  }
  return result;
}

template<class OLED>
void SSD1306Console<OLED>::preparePosition() {
  if (_col >= columns()) {
    _col = 0;
    ++_row;
  }
  if (_row >= rows()) {
    this->scroll(this->charHeight() / 8);
    _row = rows() - 1;
  }
}
