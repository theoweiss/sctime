#ifndef utils_h
#define utils_h

/** Rundet f auf das naechste Vielfache von step*/
float roundTo(float f, float step);

// Quotes the given argument
#define QUOTEME(x) QUOTEME_HELPER(x)
#define QUOTEME_HELPER(x) #x

#endif

