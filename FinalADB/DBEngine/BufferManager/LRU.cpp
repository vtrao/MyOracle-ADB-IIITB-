#include "VSBufferManager.h"
#include "standards.h"

LRU* LRU :: singleInstance_C = 0;

LRU* LRU :: getInstance()
	{
	if(singleInstance_C==0)
		singleInstance_C=new LRU();
	return singleInstance_C;
	}	

LRU :: LRU()
	{
#ifdef DEBUG
	printf("\n\tLRU Constructed");
#endif	
	}	

int LRU :: getLeastReferencedFrame(FrameClass *frames,int numberOfFrames)
	{
	int i=0,leastReferencedFrameNumber=0,leastReferenced=0, leastTimeStamp=0;
	leastReferenced= (frames + i)->getLastTimeStamp();
        leastTimeStamp= (frames + i)->getTimeStamp();
#ifdef DEBUG
	printf("\n\tLRU : getLeastReferencedFrameNumber()");
#endif	
	for(i=1;i<numberOfFrames;i++)
		{
		if(leastReferenced != 0)
			{
      			if(leastReferenced > (frames + i)->getLastTimeStamp())
				{			
				leastReferenced= (frames + i)->getLastTimeStamp();
				leastReferencedFrameNumber=i;
				}
			}	
		else if(leastReferenced == 0)
			{
			if(leastTimeStamp > (frames + i)->getTimeStamp())
				{				 		
				leastTimeStamp= (frames + i)->getTimeStamp();
				leastReferencedFrameNumber=i;			
				}
			}
	   	}
#ifdef DEBUG
	printf("\n\t\tLeast Referenced Frame Number is %d\n",leastReferencedFrameNumber);
#endif	
	return leastReferencedFrameNumber; 
	}	

int LRU :: replaceFrame(FrameClass *frames,int pageNumber,int numberOfFrames,DiskSpaceManagerLinux *dsml)
       {
       int leastReferencedFrameNumber,size=0;
       leastReferencedFrameNumber=getLeastReferencedFrame(frames,numberOfFrames);
#ifdef DEBUG
	printf("\n\tLRU : replaceFrame()");
#endif	
       if((frames + leastReferencedFrameNumber)->getDirtyBit()==1)
       		{
		size=dsml->writePageToHDD((frames + leastReferencedFrameNumber)->getPageNumber(),(frames + leastReferencedFrameNumber)->getData());

#ifdef DEBUG
	printf("\n\t\tFrame Replaced is %d and Number of bytes successfully writtem is %d", leastReferencedFrameNumber,size);		
#endif		
		}

	return leastReferencedFrameNumber;
       }
  x`