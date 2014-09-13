#include "V1495_patternUnit.h"

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "CAENVMElib.h"
#include "CAENVMEtypes.h" 
#include "CAENVMEoslib.h"

typedef struct V1495_patternUnit_config_t {
  unsigned int baseAddress;
  unsigned int ctrlRegWord;
  unsigned int maskA;
  unsigned int maskB;
  unsigned int maskE;
  unsigned int maskF;
  unsigned int sigDelay;
} patternUnitConfig_t;

patternUnitConfig_t V1495_patternUnit_config;


#define V1495_DEBUG 

int init_V1495_patternUnit(int handle)
{
  // will be read by config file later
  V1495_patternUnit_config.baseAddress=0x33330000;
  V1495_patternUnit_config.ctrlRegWord=0x00001010;
  V1495_patternUnit_config.maskA=0xFFFFFFFF;
  V1495_patternUnit_config.maskB=0xFFFFFFFF;
  V1495_patternUnit_config.maskE=0xFFFFFFFF;
  V1495_patternUnit_config.maskF=0xFFFFFFFF;
  V1495_patternUnit_config.sigDelay=0x0;

  std::cout << "+++++ V1495 PATTERN UNIT @0x" <<  std::hex << V1495_patternUnit_config.baseAddress << std::dec << " CONFIGURATION +++++" << std::endl;
  short status(1),caenst;
  unsigned int data;
  caenst = CAENVME_WriteCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_MODULERESET_ADDRESS, &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst);
  sleep(1);
  caenst = CAENVME_ReadCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_VMEFPGA_FWVERSION_ADDRESS, &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
  short vmefwMajorVersion=data>>8 & V1495_PATTERNUNIT_VMEFPGAFWVERSION_MAJORNUMBER_BITMASK;
  short vmefwMinorVersion=data & V1495_PATTERNUNIT_VMEFPGAFWVERSION_MINORNUMBER_BITMASK;
  caenst = CAENVME_ReadCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_USERFPGA_FWVERSION_ADDRESS, &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
  short userfpgafwMajorVersion=data>>4 & V1495_PATTERNUNIT_USERFPGAFWVERSION_MAJORNUMBER_BITMASK;
  short userfpgafwMinorVersion=data & V1495_PATTERNUNIT_USERFPGAFWVERSION_MINORNUMBER_BITMASK;

  if (status !=1 )
    {
      std::cout << "[V1495_patternUnit]::[ERROR]::Communicator error for device @0x" << std::hex << V1495_patternUnit_config.baseAddress << " " << CAENVME_DecodeError((CVErrorCodes)caenst) << std::endl;
      return 2;
    }
  else
    {
      std::cout << "[V1495_patternUnit]::[INFO]::VME FPGA FW Version " << vmefwMajorVersion << "." << vmefwMinorVersion << " USER FPGA FW Version " << userfpgafwMajorVersion << "." << userfpgafwMinorVersion << std::endl;
    }

  caenst = CAENVME_WriteCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_CTRLREG_ADDRESS , &V1495_patternUnit_config.ctrlRegWord ,cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
  caenst = CAENVME_WriteCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_MASKA_ADDRESS , &V1495_patternUnit_config.maskA ,cvA32_U_DATA,cvD32);
  status *= (1-caenst);
  caenst = CAENVME_WriteCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_MASKB_ADDRESS , &V1495_patternUnit_config.maskB ,cvA32_U_DATA,cvD32);
  status *= (1-caenst);
  caenst = CAENVME_WriteCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_MASKE_ADDRESS , &V1495_patternUnit_config.maskE ,cvA32_U_DATA,cvD32);
  status *= (1-caenst);
  caenst = CAENVME_WriteCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_MASKF_ADDRESS , &V1495_patternUnit_config.maskF ,cvA32_U_DATA,cvD32);
  status *= (1-caenst);

  if (userfpgafwMajorVersion>1 && userfpgafwMinorVersion>0)
    {
      caenst = CAENVME_WriteCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_DELAY_ADDRESS , &V1495_patternUnit_config.sigDelay ,cvA32_U_DATA,cvD32);
      status *= (1-caenst);
    }
  if (status !=1 )
    {
      std::cout << "[V1495_patternUnit]::[ERROR]::Cannot initialize device @0x" <<std::hex <<  V1495_patternUnit_config.baseAddress << std::dec << " " << CAENVME_DecodeError((CVErrorCodes)caenst) << std::endl;
      return 3;
    }

  //  std::cout << "[V1495_patternUnit]::[INFO]::Initializing completed for device @0x" <<std::hex <<  V1495_patternUnit_config.baseAddress << std::dec << std::endl;
  std::cout << "+++++ V1495 PATTERN UNIT @0x" <<  std::hex << V1495_patternUnit_config.baseAddress << std::dec << " END CONFIGURATION +++++" << std::endl;  
  return status;
}

int read_V1495_patternUnit(int handle, std::vector<unsigned int>& eventBuffer)
{
  short status(1),caenst;
  unsigned int data;
  bool gate(0);
#ifdef V1495_DEBUG
  std::cout << "--- V1495 START DEBUG ---" << std::endl;
#endif 
  data=0x50000004; //BOE + nPatternsReadout
  eventBuffer.push_back(data);
  /* while (!gate) */
  /*   { */
  caenst = CAENVME_ReadCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_STATUS_ADDRESS , &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
  if (status !=1 )
    {
      std::cout << "[V1495_patternUnit]::[ERROR]::Error reading status for device @0x" << std::hex <<  V1495_patternUnit_config.baseAddress << std::dec <<  CAENVME_DecodeError((CVErrorCodes)caenst) << std::endl;
      return 2;
    }
  else
    {
      gate=(data>>20)&0x1;
    }
    /* } */

  eventBuffer.push_back(data);
#ifdef V1495_DEBUG
  std::cout << "V1495 STATUS ===>" << std::hex << data << std::endl;
#endif
  caenst = CAENVME_ReadCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_PATTERNA_ADDRESS , &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
#ifdef V1495_DEBUG
  std::cout << "V1495 PATTERNA ===>" << std::hex << data << std::endl;
#endif
  eventBuffer.push_back(data);
  caenst = CAENVME_ReadCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_PATTERNB_ADDRESS , &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
#ifdef V1495_DEBUG
  std::cout << "V1495 PATTERNB ===>" << std::hex << data << std::endl;
#endif
  eventBuffer.push_back(data);
  caenst = CAENVME_ReadCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_PATTERNE_ADDRESS , &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
#ifdef V1495_DEBUG
  std::cout << "V1495 PATTERNE ===>" << std::hex << data << std::endl;
#endif
  eventBuffer.push_back(data);
  caenst = CAENVME_ReadCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_PATTERNF_ADDRESS , &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
#ifdef V1495_DEBUG
  std::cout << "V1495 PATTERNF ===>" << std::hex << data << std::endl;
#endif
  eventBuffer.push_back(data);

  data=0x30000007; //EOE + eventSize
  eventBuffer.push_back(data);

  if (status !=1 )
    {
      std::cout << "[V1495_patternUnit]::[ERROR]::Error reading patterns for device @0x" <<std::hex <<  V1495_patternUnit_config.baseAddress << std::dec <<  CAENVME_DecodeError((CVErrorCodes)caenst) << std::endl;
      return 3;
    }

  //Clear pattern registers & busy
  data=V1495_patternUnit_config.ctrlRegWord | 0x100;
  caenst = CAENVME_WriteCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_CTRLREG_ADDRESS , &data, cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
#ifdef V1495_DEBUG
  std::cout << "--- V1495 END DEBUG ---" << std::endl;
#endif   
  return status;
}

int find_V1495_patternUnit_eventSize(std::vector<unsigned int>& events,unsigned int evtStart)
{
  short dt_type_boe = events.at(evtStart)>>28 & 0xF;
  if (dt_type_boe != 5)
    {
      std::cout << "[V1495_patternUnit]::[ERROR]::NOT AT BEGIN OF EVENT. DATA PROBABLY CORRUPTED" << std::endl;
      return -1;
    }
  short nPatternWords = events.at(evtStart)&0xF;

  short dt_type_eoe = events.at(evtStart+nPatternWords+2)>>28 & 0xF;
  if (dt_type_eoe != 3)
    {
      std::cout << "[V1495_patternUnit]::[ERROR]::NOT AT END OF EVENT. DATA PROBABLY CORRUPTED" << std::endl;
      return -1;
    }

  short nTotalWords = events.at(evtStart+nPatternWords+2)&0xFF;
  if (nTotalWords != nPatternWords+3)
    {
      std::cout << "[V1495_patternUnit]::[ERROR]::DATA SIZE MISMATCH. DATA PROBABLY CORRUPTED" << std::endl;
      return -1;
    }

  return nTotalWords;
}
