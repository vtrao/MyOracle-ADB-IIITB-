#include "VSBufferManager.h"
#include "standards.h"

MRU* MRU :: singleInstance_C = 0;

MRU* MRU :: getInstance()
	{
	if(singleInstance_C==0)
		singleInstance_C=new MRU();
	return singleInstance_C;
	}	

MRU :: MRU()
	{
#ifdef DEBUG
	printf("\n\tLRU : Constructed");
#endif	
	}	

int MRU :: getMostReferencedFrame(FrameClass *frames,int numberOfFrames)
	{
	int i=0,mostlyReferencedFrameNumber=0,mostlyReferenced, mostlyTimeStamp;
	mostlyReferenced= (frames + i)->getLastTimeStamp();
	mostlyTimeStamp= (frames + i)->getTimeStamp();
#ifdef DEBUG
	printf("\n\tLRU : getMostReferencedFrameNumber()");
#endif		
	for(i=1;i<numberOfFrames;i++)
		{
		if(mostlyReferenced != 0)
			{
      			if(mostlyReferenced < (frames + i)->getLastTimeStamp())
				{			
				mostlyReferenced= (frames + i)->getLastTimeStamp();
				mostlyReferencedFrameNumber=i;
				}
			}	
		else if(mostlyReferenced == 0)
			{
			if(mostlyTimeStamp < (frames + i)->getTimeStamp())
				{				 		
				mostlyTimeStamp= (frames + i)->getTimeStamp();
				mostlyReferencedFrameNumber=i;				
				}	
			}
	   	}      
#ifdef DEBUG
	printf("\n\t\tMost Referenced Frame Number is %d\n",mostlyReferencedFrameNumber);
#endif	
	return mostlyReferencedFrameNumber; //
	}	

int MRU :: replaceFrame(FrameClass *frames,int pageNumber,int numberOfFrames,DiskSpaceManagerLinux *dsml)
       {
       int mostlyReferencedFrameNumber,size=0;
       mostlyReferencedFrameNumber=getMostReferencedFrame(frames,numberOfFrames);
#ifdef DEBUG
	printf("\n\tMRU : replaceFrame()");
#endif	
       if((frames + mostlyReferencedFrameNumber)->getDirtyBit()==1)
       		{
		size=dsml->writePageToHDD((frames + mostlyReferencedFrameNumber)->getPageNumber(),(frames + mostlyReferencedFrameNumber)->getData());
#ifdef DEBUG
	printf("\n\t\tFrame Replaced is %d and Number of bytes successfully writtem is %d", mostlyReferencedFrameNumber,size);		
#endif		
		}
	return mostlyReferencedFrameNumber;
       }


  3ٝ