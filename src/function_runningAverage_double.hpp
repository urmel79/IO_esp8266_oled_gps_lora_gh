//
//    FILE: function_runningAverage_double.hpp
//  AUTHOR: Rob.Tillaart@gmail.com + Bjoern Kasper
// VERSION: 0.2.13 => 0.2.13d
//    DATE: 2016-dec-01 => 2020-jan-05
// PURPOSE: RunningAverage library for Arduino
//     URL: https://github.com/RobTillaart/Arduino/tree/master/libraries/RunningAverage
// HISTORY: See RunningAverage.cpp
//
// Released to the public domain
//
// backwards compatibility
// clr()   clear()
// add(x)  addValue(x)
// avg()   getAverage()

#ifndef function_runningAverage_double_hpp
#define function_runningAverage_double_hpp

#define RUNNINGAVERAGE_LIB_VERSION "0.2.13d"

#include "Arduino.h"

class RunningAverage
{
public:
  RunningAverage(void);
  explicit RunningAverage(const uint8_t);
  ~RunningAverage();

  void    clear();
  void    addValue(const double);
  void    fillValue(const double, const uint8_t);

  double   getAverage() const;      // does iterate over all elements.
  double   getFastAverage() const;  // reuses previous values.

  // return statistical characteristics of the running average
  double   getStandardDeviation() const;
  double   getStandardError() const;

  // returns min/max added to the data-set since last clear
  double   getMin() const { return _min; };
  double   getMax() const { return _max; };

  // returns min/max from the values in the internal buffer
  double   getMinInBuffer() const;
  double   getMaxInBuffer() const;

  // return true if buffer is full
  bool    bufferIsFull() const { return _cnt == _size; };

  double   getElement(uint8_t idx) const;

  uint8_t getSize() const { return _size; }
  uint8_t getCount() const { return _cnt; }



protected:
  uint8_t _size;
  uint8_t _cnt;
  uint8_t _idx;
  double   _sum;
  double*  _ar;
  double   _min;
  double   _max;
};

#endif // function_runningAverage_double_hpp
// END OF FILE
