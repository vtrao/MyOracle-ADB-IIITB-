#include "VSBufferManager.h"
#include "standards.h"

FIFO* FIFO :: singleInstance_C = 0;

FIFO* FIFO :: getInstance()
	{
        if(singleInstance_C==0)
                singleInstance_C=new FIFO();
        return singleInstance_C;
        }

FIFO :: FIFO()
	{
#ifdef DEBUG
	printf("\n\tFIFO Constructed");
#endif	
	}


int FIFO :: getLeastTimeStampFrame(FrameClass *frames,int numberOfFrames)
        {
        int i=0,leastTimeStampFrameNumber=0,leastTimeStamp;
        leastTimeStamp=(frames + i)->getTimeStamp();
#ifdef DEBUG
	printf("\n\tFIFO : getLeastTimeStampFrame()");
#endif		
        for(i=1;i<numberOfFrames;i++)
                {
                if(leastTimeStamp > (frames + i)->getTimeStamp())
                        {
                        leastTimeStamp= (frames + i)->getTimeStamp();
                        leastTimeStampFrameNumber=i;
                        }
                }
#ifdef DEBUG
        printf("\n\t\tThe Least timestamp Frame Number is %d\n",leastTimeStampFrameNumber);
#endif
        return leastTimeStampFrameNumber; //
        }


int  FIFO :: replaceFrame(FrameClass *frames,int pageNumber,int numberOfFrames,DiskSpaceManagerLinux *dsml)
       {
       int leastTimeStampFrameNumber,size=0;
       leastTimeStampFrameNumber=getLeastTimeStampFrame(frames,numberOfFrames);
#ifdef DEBUG
	printf("\n\tFIFO : replaceFrame()");
#endif         
       if((frames + leastTimeStampFrameNumber)->getDirtyBit()==1)
                {
                size=dsml->writePageToHDD((frames + leastTimeStampFrameNumber)->getPageNumber(),(frames + leastTimeStampFrameNumber)->getData());
#ifdef DEBUG
        printf("\n\t\tFrame Replaced is %d and Number of bytes successfully writtem %d\n",
                                         leastTimeStampFrameNumber,size);
#endif
                }
        return leastTimeStampFrameNumber;
       }








  ��0