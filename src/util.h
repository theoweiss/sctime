#ifndef UTIL_H
#define UTIL_H

inline float roundTo(float f, float step) { return int(f/step+0.5)*step; }
inline double roundTo(double f, double step){ return int(f/step+0.5)*step;}

#endif
