#ifndef _V1495_patternUnit_H_
#define _V1495_patternUnit_H_

#include <vector>

#define V1495_PATTERNUNIT_MASKA_ADDRESS 0x1010
#define V1495_PATTERNUNIT_MASKB_ADDRESS 0x1014
#define V1495_PATTERNUNIT_MASKE_ADDRESS 0x101C
#define V1495_PATTERNUNIT_MASKF_ADDRESS 0x1020
#define V1495_PATTERNUNIT_CTRLREG_ADDRESS 0x1018
#define V1495_PATTERNUNIT_DELAY_ADDRESS 0x1018
#define V1495_PATTERNUNIT_PATTERNA_ADDRESS 0x103C
#define V1495_PATTERNUNIT_PATTERNB_ADDRESS 0x1040
#define V1495_PATTERNUNIT_PATTERNE_ADDRESS 0x1044
#define V1495_PATTERNUNIT_PATTERNF_ADDRESS 0x1048
#define V1495_PATTERNUNIT_STATUS_ADDRESS 0x1050
#define V1495_PATTERNUNIT_FWVERSION_ADDRESS 0x100C
#define V1495_PATTERNUNIT_FWVERSION_MINORNUMBER_BITMASK 0x000000FF

/* #include <CAENDigitizer.h> */
int init_V1495_patternUnit(int handle);
int read_V1495_patternUnit(int handle, std::vector<unsigned int>& event);
/* //int read_V1495patternUnit(int handle); */
/* int writeEventToOutputBuffer_V1495patternUnit(std::vector<unsigned int> *eventBuffer, CAEN_DGTZ_EventInfo_t *EventInfo, CAEN_DGTZ_X742_EVENT_t *Event); */
/* int stop_V1495patternUnit(int handle); */

#endif 
