#include "VSBufferManager.h"
#include "standards.h"

FrameClass :: FrameClass(Frame f)
{
	dirty_Bit=f.dBit;
	pin_Count=f.pCount;
	frame_Number=f.fNumber;
	page_Number=f.pNumber;
	priority_c=f.pity;
	access_Count=f.aCount;
	data=f.data;
}

FrameClass :: FrameClass()
{
	
}

FrameClass :: ~FrameClass()
{
}

void FrameClass :: setDirtyBit(int dBit)
	{
	dirty_Bit=dBit;	
	}	
	
void FrameClass :: setPinCount(int pC)
	{
	pin_Count=pC;	
	}	
	
void FrameClass :: setFrameNumber(int fNumber)
	{
	frame_Number=fNumber;
	}	
	
void FrameClass :: setPageNumber(int pNumber)
	{
	page_Number=pNumber;	
	}
	
void FrameClass :: setPriority(unsigned int pty)
	{
	priority_c=pty;	
	}
	
void FrameClass :: setAccessCount(int aCount)
	{
	access_Count=aCount;	
	}

void FrameClass :: setData(void *rData)
	{
	data=rData;
	}	

void FrameClass :: setTimeStamp(time_t tStamp)
	{
	timeStamp=tStamp;	
	}

void FrameClass :: setLastTimeStamp(time_t LastStamp)
	{
	lastTimeStamp=LastStamp;	
	}	

int FrameClass :: getDirtyBit()
	{
	return(dirty_Bit);	
	}
	
int FrameClass :: getPinCount()
	{
	return(pin_Count);	
	}
		
int FrameClass :: getFrameNumber()
	{
	return(frame_Number);	
	}
		
int FrameClass :: getPageNumber()
	{
	return(page_Number);	
	}
		
unsigned int FrameClass :: getPriority()
	{
	return(priority_c);	
	}
		
int FrameClass :: getAccessCount()	
	{
	return(access_Count);	
	}

void* FrameClass :: getData()
	{
	return(data);	
	}	

time_t FrameClass :: getTimeStamp()
	{
	return timeStamp;	
	}	

time_t FrameClass :: getLastTimeStamp()
	{
	return lastTimeStamp;	
	}	
  tyB