#ifndef IEEE_FLOAT_H__
#define IEEE_FLOAT_H__

#include	<math.h>

typedef float Single;

#ifndef applec
 typedef double defdouble;
#else
 typedef long double defdouble;
#endif

#ifndef THINK_C
 typedef double Double;
#else
 typedef short double Double;
#endif

#define	kFloatLength	4
#define	kDoubleLength	8
#define	kExtendedLength	10

extern defdouble ConvertFromIeeeSingle(char *bytes);
extern void ConvertToIeeeSingle(defdouble num, char *bytes);
extern defdouble ConvertFromIeeeDouble(char *bytes);
extern void ConvertToIeeeDouble(defdouble num, char *bytes);
extern defdouble ConvertFromIeeeExtended(char *bytes);
extern void ConvertToIeeeExtended(defdouble num, char *bytes);
#endif
