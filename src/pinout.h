#ifndef PINOUT_H
#define PINOUT_H

#ifdef KNOMIV2
    #include "pinout_knomi_v2.h"
#elif defined(KNOMIV1_C3)
    #include "pinout_knomi_v1_c3.h"
#else
    #include "pinout_knomi_v1.h"
#endif
#endif