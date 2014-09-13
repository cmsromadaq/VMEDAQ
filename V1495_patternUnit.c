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
  V1495_patternUnit_config.baseAddress=0x22220000;
  V1495_patternUnit_config.ctrlRegWord=0x00001010;
  V1495_patternUnit_config.maskA=0xFFFFFFFF;
  V1495_patternUnit_config.maskB=0xFFFFFFFF;
  V1495_patternUnit_config.maskE=0xFFFFFFFF;
  V1495_patternUnit_config.maskF=0xFFFFFFFF;
  V1495_patternUnit_config.sigDelay=0x0;

  short status(1),caenst;
  unsigned int data;
  caenst = CAENVME_WriteCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_MODULERESET_ADDRESS, &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst);
  sleep(1);
  caenst = CAENVME_ReadCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_VMEFPGA_FWVERSION_ADDRESS, &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
  if (status !=1 )
    {
      std::cout << "[V1495_patternUnit]::[ERROR]::Communicator error for device @0x" << std::hex << V1495_patternUnit_config.baseAddress << " " << CAENVME_DecodeError((CVErrorCodes)caenst) << std::endl;
      return 2;
    }
  else
    {
      short fwMajorVersion=data>>8 & 0xFF;
      short fwMinorVersion=data & V1495_PATTERNUNIT_FWVERSION_MINORNUMBER_BITMASK;
      std::cout << "[V1495_patternUnit]::[INFO]::Initializing device @" << std::hex << V1495_patternUnit_config.baseAddress << std::dec << " VME FPGA FW Version " << fwMajorVersion << "." << fwMinorVersion << std::endl;
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

  if (status !=1 )
    {
      std::cout << "[V1495_patternUnit]::[ERROR]::Cannot initialize device @0x" <<std::hex <<  V1495_patternUnit_config.baseAddress << std::dec << " " << CAENVME_DecodeError((CVErrorCodes)caenst) << std::endl;
      return 3;
    }

  std::cout << "[V1495_patternUnit]::[INFO]::Initializing completed for device @0x" <<std::hex <<  V1495_patternUnit_config.baseAddress << std::dec << std::endl;
  
  return status;
}

int read_V1495_patternUnit(int handle, std::vector<unsigned int>& eventBuffer)
{
  short status(1),caenst;
  unsigned int data;
  bool gate(0);
  while (!gate)
    {
      caenst = CAENVME_ReadCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_STATUS_ADDRESS , &data ,cvA32_U_DATA,cvD32);
      status *= (1-caenst); 
      if (status !=1 )
	{
	  std::cout << "[V1495_patternUnit]::[ERROR]::Error reading status for device @0x" <<std::hex <<  V1495_patternUnit_config.baseAddress << std::dec <<  CAENVME_DecodeError((CVErrorCodes)caenst) << std::endl;
	  return 2;
	}
      else
	  gate=(data>>20)&0x1;
    }

  eventBuffer.push_back(data);

  caenst = CAENVME_ReadCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_PATTERNA_ADDRESS , &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
  eventBuffer.push_back(data);
  caenst = CAENVME_ReadCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_PATTERNB_ADDRESS , &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
  eventBuffer.push_back(data);
  caenst = CAENVME_ReadCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_PATTERNE_ADDRESS , &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
  eventBuffer.push_back(data);
  caenst = CAENVME_ReadCycle(handle,V1495_patternUnit_config.baseAddress + V1495_PATTERNUNIT_PATTERNF_ADDRESS , &data ,cvA32_U_DATA,cvD32);
  status *= (1-caenst); 
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
   
  return status;
}
