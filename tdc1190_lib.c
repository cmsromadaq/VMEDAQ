#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h> 
#include <time.h> 
#include <iostream> 

#include "CAENVMElib.h"
#include "CAENVMEtypes.h" 
#include "CAENVMEoslib.h"
#include "tdc1190_lib.h"

using namespace std;

std::vector<unsigned long> addrs;

/*------------------------------------------------------------------*/

unsigned short init_tdc1190(int32_t BHandle)
{
  unsigned long address;
  unsigned short data;
  short  status; int caenst;
  int WindowWidth =  0x28;    /* Value * 25ns : 0x28 --> 1mus*/
  int WindowOffset = -0x14;    /* Time Window Offset */
  unsigned long trMode = 0x0000;
  status = 1;

  addrs.clear();
  addrs.push_back(TDC1190_ADDRESS);
  if(NUMBOARDS >1) addrs.push_back(TDC1190_ADDRESS2);

  //Initialize all the boards
  for(int iBo = 0; iBo<NUMBOARDS; iBo++) {
    /* 0 passo: card reset*/
    /* Edge detection rising by default after reset*/
    
    //software Clear
    address = addrs.at(iBo) + 0x1016;
    data = 0x1;
    
    //  status *= vme_write_dt(address,&data,AD32,D16);
    caenst = CAENVME_WriteCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
    status = 1-caenst;
    if(td1190_debug) printf("Issued a Software Clear. Status:%d\n",status);
    
    sleep(2); /* wait until the inizialization is complete */
    
    /* 0 step: debugging. Printout the status register */
    /*
      if(td1190_debug) {
      address = TDC1190_ADDRESS + 0x1002;
      //status *= vme_read_dt(address,&data,AD32,D16);
      caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
      status = 1-caenst;
      printf("Reading status register. Status:%u\t%X\n",status,(unsigned int)data);
      }
    */
    /* I step: set TRIGGER Matching mode via OPCODE 00xx */
    if(td1190_debug) printf("Going to set Tr mode :%d\n",(int)trMode);
    status *=opwriteTDC(BHandle, addrs.at(iBo), trMode);
    if(td1190_debug) printf("Setting trigger matching. Status:%d\n",status);
    
    /* I step: set Edge detection via OPCODE 22xx */
    data =   0x2200;
    status *=opwriteTDC(BHandle, addrs.at(iBo), data);
    data = 0x2;
    status *=opwriteTDC(BHandle, addrs.at(iBo), data);

    /* I step: set Time Reso via OPCODE 24xx */
    data =   0x2400;
    status *=opwriteTDC(BHandle, addrs.at(iBo), data);
    data = 0x2;
    status *=opwriteTDC(BHandle, addrs.at(iBo), data);

    if(td1190_debug) printf("Setting Time Reso 100ps. Status:%d\n",status);
    
    /* II step: set TRIGGER Window Width to value n */
    data = 0x1000;
    status *=opwriteTDC(BHandle, addrs.at(iBo), data); 
    
    status *=opwriteTDC(BHandle, addrs.at(iBo), WindowWidth);
    
    /* III step: set TRIGGER Window Offset to value -n */
    data = 0x1100;
    status *=opwriteTDC(BHandle, addrs.at(iBo), 0x1100); 
    status *=opwriteTDC(BHandle, addrs.at(iBo), WindowOffset);
    
    /* III-bis step: enable BLT mode */
    /*
    data = 0x1100;
    status *=opwriteTDC(BHandle, addrs.at(iBo), 0x1100); 
    status *=opwriteTDC(BHandle, addrs.at(iBo), WindowOffset);
    */
    
    /* IV step: Enable trigger time subtraction */
    data = 0x1400;
    status *=opwriteTDC(BHandle, addrs.at(iBo), 0x1400); 
    
    //Disable tr time subtr
    //  status *=opwriteTDC(BHandle,0x1500); 
    
    if(td1190_debug) printf("Settings Status:: %d\n",status);
    
    /* TDC Event Reset */
    address = addrs.at(iBo) + 0x1018;  data=0x1;
    //  status *= vme_write_dt(address,&data,AD32,D16);
    caenst = CAENVME_WriteCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
    status = 1-caenst;
    if(td1190_debug) printf("Setup Done :: %d\t. Ready for acquisition.\n",status);
  }

  return (status);
}

/*------------------------------------------------------------*/

short opwriteTDC(int32_t BHandle, unsigned long addr, unsigned short data) 
/* It allows to write an opcode or an operand  */
{
  unsigned long address;
  unsigned short rdata;
  unsigned long time=0;
  short status=1; int caenst;
  const int TIMEOUT = 100000;

  /* Check the Write OK bit */
  do {
    address = addr + MICROHANDREG; 
    //status *= vme_read_dt(address, &rdata,AD32,D16);
    caenst = CAENVME_ReadCycle(BHandle,address,&rdata,cvA32_U_DATA,cvD16);
    status = 1-caenst;
    time++;
   } while ((rdata != 0x1) && ((int)time < TIMEOUT));

  if ((int)time == TIMEOUT) {
    printf("\n TDC opwrite failed: TIMEOUT! \n");  
    status = 0;
    return(status);
   }
   usleep(10000);    /* 10ms sleep before sending real data */

  address = addr + MICROREG; /* OPCODE register */
  //status *= vme_write_dt(address, &data,AD32,D16);
  caenst = CAENVME_WriteCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status = 1-caenst;

  return (status);
}

/*----------------------------------------------------------------------*/

unsigned short opreadTDC(int32_t BHandle, unsigned long addr,
			 unsigned short *p2data) 
/* It allows to read the data; CAEN MANUAL like  */
{
  unsigned long address;
  unsigned short rdata;
  int time=0;
  unsigned short status=1; int caenst;
  const int TIMEOUT = 100000;

  /* Wait the DATA READY bit set to 1 */
  do {
    address = addr + MICROHANDREG; 
    //status *= vme_read_dt(address, &rdata,AD32,D16);
    caenst = CAENVME_ReadCycle(BHandle,address,&rdata,cvA32_U_DATA,cvD16);
    status = 1-caenst ;
    time++;
   } while (!(rdata & 0x2) && (time < TIMEOUT));

  if (time == TIMEOUT) {
    printf("\n TDC opread failed: TIMEOUT! \n");  
    status = 0;
    return(status);
   }

  address = addr + MICROREG; /* OPCODE register */
  //status *= vme_read_dt(address,p2data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,p2data,cvA32_U_DATA,cvD16);
  status = 1-caenst;
  return(status);
}

/*-------------------------------------------------------------*/

vector<int> readEventTDC(int32_t BHandle, int idB, int status) {
  unsigned long data,address,geo_add,measurement,channel,trailing;
  double tdc_time;
  int dataReady = 0; int TRG_matched = 0, evt_num = 0, tra_stat = 0;
  int caenst; int TIMEOUT = 10000;
  vector<int> outD;
  /* 
     Events Words
     1) Header
     2) Tdc evt, ch, time
     3) Trailer + error
  */
  int tdc_hea = 0; int tdc_mea = 0;
  int tdc_tra = 0; int glb_hea = 0, glb_tra = 0;
  int nTry (0);

  if(idB<0 || idB>NUMBOARDS-1) {
    cout<<" Accssing Board number"<<idB<<" while only "<<NUMBOARDS<<" are initialized!!! Check your configuration!"<<endl;
    status = 0;
    return outD;
  }

  /*
    First of all check the Status Register to see 
    if there's Data Ready
  */
  status = 1;
  do {
    address =  addrs.at(idB) + STATUSREGADD;
    //status *= vme_read_dt(address,&data,AD32,D16);
    caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
    status = 1-caenst;
    if((data>>0) & 0x1) { 
      dataReady = 1;
    }
    if((data>>3) & 0x1) { TRG_matched = 1; }
    nTry++;
  } while (!dataReady && nTry<TIMEOUT);
  if(nTry > TIMEOUT-10) printf("Timeout!!!! :%d\n",nTry);
  
  /* Wait the DATA READY bit set to 1 */
  if(dataReady) {
    address= addrs.at(idB) + 0x0;
    //status *= vme_read_dt(address,&data,AD32,D32);
    caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
    status = 1-caenst;
    if(td1190_debug) printf("Read New event!\n");

    if( (data>>27) & 0x8 ) {
      glb_hea = 1;
      geo_add = data>>27;
      evt_num = data>>5 & 0x3fffff;
      outD.push_back((int) evt_num);
    }

    /*Check if trailer is found. Then loop until trailer is found and
      start over*/
    if( (data>>27) & 0x10 ) glb_tra = 1;

    while(status && !glb_tra) 
      {
	//status *= vme_read_dt(address,&data,AD32,D32);
	caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
	status = 1-caenst;
	glb_tra = (data>>27) & 0x10 ;
	tdc_tra = (data>>27) & 0x3 ;
	tdc_hea = (data>>27) & 0x1 ;
	tdc_mea = (data>>27) & 0x0 ;

	if(glb_tra == 0x10) {
	  /* Global Trailer!*/
	  if(td1190_debug) printf("Global trailer:: %X\n",(unsigned int)data);
	  tra_stat = data>>24 & 0x1;
	  outD.push_back((int) tra_stat);
	} else if(tdc_tra == 0x3) {
	  /* TDC Trailer!*/
	  if(td1190_debug) printf("TDC trailer:: %X\n",(unsigned int)data);
	} else if(tdc_hea) {
	  /* TDC Header!*/
	  if(td1190_debug) printf("TDC header:: %X\n",(unsigned int)data);
	} else if(!tdc_mea) {
	  /* TDC measurement!*/
	  measurement = data & 0x3ffff;
	  channel = (data>>19) & 0x7f;
	  trailing = (data>>26) & 0x1;
	  tdc_time = (double)measurement/10;
	  //	  if(td1190_debug)
	  if(td1190_debug) 
	    printf("TDC header:: %d %d %d\n",(int)evt_num,(int)channel,(int)measurement);
	  outD.push_back((int) evt_num);
	  outD.push_back((int) channel);
	  outD.push_back((int) measurement);
	}
	
      }
  }

  return outD;
}

/*-------------------------------------------------------------*/

vector<int> readNEventsTDC(int32_t BHandle, int idB, int status, int nevents, vector<int> &outW) {
  unsigned long data,address,geo_add,measurement,channel,trailing;
  double tdc_time;
  int dataReady = 0; int TRG_matched = 0, evt_num = 0, tra_stat = 0;
  int caenst; int TIMEOUT = 10000;
  vector<int> outD;
  /* 
     Events Words
     1) Header
     2) Tdc evt, ch, time
     3) Trailer + error
  */
  int tdc_hea = 0; int tdc_mea = 0;
  int tdc_tra = 0; int glb_hea = 0, glb_tra = 0;
  int nTry (0);

  if(idB<0 || idB>NUMBOARDS-1) {
    cout<<" Accssing Board number"<<idB<<" while only "<<NUMBOARDS<<" are initialized!!! Check your configuration!"<<endl;
    status = 0;
    return outD;
  }

  /*Access to control REGISTER
  address= addrs.at(idB) + CONREGADD;
  status *= vme_read_dt(address,&data,AD32,D16);

  if(td1190_debug) printf("Access to control register. Status:%d\n",status);
  */

  /*
    First of all check the Status Register to see 
    if there's Data Ready
  */
  status = 1;
  do {
    address =  addrs.at(idB) + STATUSREGADD;
    //status *= vme_read_dt(address,&data,AD32,D16);
    caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
    status = 1-caenst;
    if((data>>0) & 0x1) { 
      dataReady = 1;
    }
    if((data>>3) & 0x1) { TRG_matched = 1; }
    nTry++;
  } while (!dataReady && nTry<TIMEOUT);
  if(nTry > TIMEOUT-10) printf("Timeout!!!! :%d\n",nTry);
  
  /* Wait the DATA READY bit set to 1 */
  int tmpev(0),tmpW(0);
  if(dataReady) {
    while(tmpev<nevents) {
      tmpW = 0;
      address= addrs.at(idB) + 0x0;
      //status *= vme_read_dt(address,&data,AD32,D32);
      caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
      status = 1-caenst;
      if(td1190_debug) printf("Read New event!\n");
      glb_hea = 0;
      if( (data>>27) & 0x8 ) {
	glb_hea = 1;
	geo_add = data>>27;
	evt_num = data>>5 & 0x3fffff;
	outD.push_back((int) evt_num);
	tmpW++;
      }
      
      //      cout<<"First read::  "<<glb_hea<<" "<<evt_num<<endl;
      
      /*Check if trailer is found. Then loop until trailer is found and
	start over*/
      glb_tra = 0;
      if( (data>>27) & 0x10 ) glb_tra = 1;
      
      while(status && !glb_tra) 
	{
	  //status *= vme_read_dt(address,&data,AD32,D32);
	  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
	  status = 1-caenst;
	  glb_tra = (data>>27) & 0x10 ;
	  tdc_tra = (data>>27) & 0x3 ;
	  tdc_hea = (data>>27) & 0x1 ;
	  tdc_mea = (data>>27) & 0x0 ;
	  
	  if(glb_tra == 0x10) {
	    /* Global Trailer!*/
	    if(td1190_debug) printf("Global trailer:: %X\n",(unsigned int)data);
	    tra_stat = data>>24 & 0x1;
	    outD.push_back((int) tra_stat);
	    tmpW++;
	  } else if(tdc_tra == 0x3) {
	    /* TDC Trailer!*/
	    if(td1190_debug) printf("TDC trailer:: %X\n",(unsigned int)data);
	  } else if(tdc_hea) {
	    /* TDC Header!*/
	    if(td1190_debug) printf("TDC header:: %X\n",(unsigned int)data);
	  } else if(!tdc_mea) {
	    /* TDC measurement!*/
	    measurement = data & 0x3ffff;
	    channel = (data>>19) & 0x7f;
	    trailing = (data>>26) & 0x1;
	    tdc_time = (double)measurement/10;
	    //	  if(td1190_debug)
	    if(td1190_debug) printf("TDC header:: %d %d %d\n",(int)evt_num,(int)channel,(int)measurement);
	    outD.push_back((int) evt_num); tmpW++;
	    outD.push_back((int) channel); tmpW++;
	    outD.push_back((int) measurement); tmpW++;
	  }
	  
	}
      outW.push_back(tmpW);
      tmpev++;
    }//multiple events block
  }

  return outD;
}


/*-------------------------------------------------------------*/

vector<int> readFastNEventsTDC(int32_t BHandle, int idB, int status, int nevents, vector<int> &outW, bool t1290) {
  unsigned long data,address,geo_add,measurement,channel,trailing;
  double tdc_time;
  int dataReady = 0; int TRG_matched = 0, evt_num = 0, tra_stat = 0;
  int caenst; int TIMEOUT = 10000;
  vector<int> outD;
  /* 
     Events Words
     1) Header
     2) Tdc evt, ch, time
     3) Trailer + error
  */
  int nbytes_tran = 0;
  int tdc_tst = 0;
  int glb_tra = 0;
  int nTry (0);
  int wr; int maxW = 150;
  if(idB == 1) maxW =  75; //Second TDC has only 16 channels

  unsigned long dataV[maxW*nevents];
  if(idB<0 || idB>NUMBOARDS-1) {
    cout<<" Accssing Board number"<<idB<<" while only "<<NUMBOARDS<<" are initialized!!! Check your configuration!"<<endl;
    status = 0;
    return outD;
  }

  /*Access to control REGISTER
  address= addrs.at(idB) + CONREGADD;
  status *= vme_read_dt(address,&data,AD32,D16);

  if(td1190_debug) printf("Access to control register. Status:%d\n",status);
  */

  /*
    First of all check the Status Register to see 
    if there's Data Ready
  */
  status = 1;
  do {
    address =  addrs.at(idB) + STATUSREGADD;
    //status *= vme_read_dt(address,&data,AD32,D16);
    caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
    status = 1-caenst;
    if((data>>0) & 0x1) { 
      dataReady = 1;
    }
    if((data>>3) & 0x1) { TRG_matched = 1; }
    nTry++;
  } while (!dataReady && nTry<TIMEOUT);
  if(nTry > TIMEOUT-10) printf("Timeout!!!! :%d\n",nTry);
  
  /* Wait the DATA READY bit set to 1 */
  int tmpW(0), idV(0);
  if(dataReady) {

    tmpW = 0;
    address= addrs.at(idB) + 0x0;

    //Vector reset
    idV = 0; while(idV<(maxW)*nevents) { dataV[idV] = 0; idV++; }
    wr = maxW*4*nevents; 
    
    //Performing BLT access
    caenst = CAENVME_BLTReadCycle(BHandle,address,dataV,wr,
				  cvA32_U_DATA,cvD32,&nbytes_tran);
    status *= (1-caenst); 
    
    
    //      caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
    //      status = 1-caenst;
    
    if(td1190_debug) printf("Read New event!\n");

    //Vector dump into output
    idV = 0; while(idV<(maxW)*nevents) {

      geo_add = dataV[idV]>>27;
      tdc_tst = geo_add & 0x1f ;

      if(tdc_tst == 0x18)  {
	idV++;  
	continue;
      }

      if(td1190_debug) 
	printf("Found word: %d %lx :: %lx %d\n",idV,dataV[idV],geo_add,tdc_tst); 

      glb_tra = (dataV[idV]>>30) & 0x2 ;

      if(tdc_tst == 0x8 ) {
	tmpW = 0;
	evt_num = dataV[idV]>>5 & 0x3fffff;
	if(td1190_debug) 
	  printf("Found GLB header: %d %d %lx :: %d\n",idB,idV,dataV[idV],evt_num); 

	outD.push_back((int) evt_num);
	if(td1190_debug) 
	  cout<<evt_num<<endl;
	tmpW++;
      }

      /*
	Check if trailer is found. Then loop until trailer is found and
	start over
      */

      if(tdc_tst == 0x10) {
	/* Global Trailer!*/
	if(td1190_debug) 
	  printf("Global trailer:: %d %X\n",idB,(unsigned int)dataV[idV]);
	tra_stat = dataV[idV]>>24 & 0x1;
	outD.push_back((int) tra_stat);
	if(td1190_debug) 
	  cout<<"trail "<<tra_stat<<endl;
	tmpW++;
	outW.push_back(tmpW);
      } 

      if(tdc_tst == 0x3) {
	/* TDC Trailer!*/
	if(td1190_debug) 
	  printf("TDC trailer:: %d %X\n",idB,(unsigned int)dataV[idV]);

      } 

      if(tdc_tst == 0x1) {
	/* TDC Header!*/
	if(td1190_debug) 
	  printf("TDC header:: %d %X\n",idB,(unsigned int)dataV[idV]);
      } 

      if(tdc_tst == 0x0) {

	/* TDC measurement!*/
	trailing = (dataV[idV]>>26) & 0x1; //1190 & 1290
	if(!t1290) {
	  measurement = dataV[idV] & 0x3ffff;
	  channel = (dataV[idV]>>19) & 0x7f;
	} else {
	  measurement = dataV[idV] & 0x1fffff;
	  channel = (dataV[idV]>>21) & 0x1f;
	}
	tdc_time = (double)measurement/10;

	if(td1190_debug) 
	  printf("TDC %d Measurement:: %d %d %d\n",(int)idB,(int)evt_num,(int)channel,(int)measurement);
	outD.push_back((int) evt_num); tmpW++;
	outD.push_back((int) channel); tmpW++;
	outD.push_back((int) measurement); tmpW++;
	if(td1190_debug) 
	  cout<<"eve:: "<<evt_num<<" "<<channel<<" "<<measurement<<endl;
      }

      //      outD.push_back((int)dataV[idV]); 
      idV++;  
    }

  }
  //Protection against timeout.
  usleep(20);
  return outD;
}



