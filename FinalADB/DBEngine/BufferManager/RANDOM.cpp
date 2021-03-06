#include "VSBufferManager.h"
#include "standards.h"
#include <stdlib.h>
#include <time.h>

RANDOM* RANDOM :: singleInstance_C = 0;

RANDOM* RANDOM :: getInstance()
	{
        if(singleInstance_C==0)
                singleInstance_C=new RANDOM();
        return singleInstance_C;
        }

RANDOM :: RANDOM()
	{
#ifdef DEBUG
	printf("\n\tRANDOM Constructed");
#endif	
	}


int RANDOM :: getRandomFrame(FrameClass *frames,int numberOfFrames)
        {
        int i=0,randomFrameNumber=0;
            double randomNumber=0;
		
        //randomNumber= ((double)(RAND_MAX) + double(1))/(double)numberOfFrames;
	  
 	 /* initialize random generator */
	  srand ( time(NULL) );
	  randomFrameNumber=rand()%numberOfFrames;	
	  /* generate some random numbers */
  
	  printf("\n\t\tThe Random Frame Number is %d\n",randomFrameNumber);

#ifdef DEBUG
	printf("\n\tRANDOM : getRandomFrame()");
#endif		
        
#ifdef DEBUG
        printf("\n\t\tThe Random Frame Number is %d\n",randomFrameNumber);
#endif
        return randomFrameNumber; //
        }


int  RANDOM :: replaceFrame(FrameClass *frames,int pageNumber,int numberOfFrames,DiskSpaceManagerLinux *dsml)
       {
       int randomFrameNumber,size=0;
       randomFrameNumber=getRandomFrame(frames,numberOfFrames);
#ifdef DEBUG
	printf("\n\tRANDOM : replaceFrame()");
#endif         
       if((frames + randomFrameNumber)->getDirtyBit()==1)
                {
                size=dsml->writePageToHDD((frames + randomFrameNumber)->getPageNumber(),(frames + randomFrameNumber)->getData());
#ifdef DEBUG
        printf("\n\t\tFrame Replaced is %d and Number of bytes successfully writtem %d\n",
                                         randomFrameNumber,size);
#endif
                }
        return randomFrameNumber;
       }




/*int main()
       {
       ReplacementPolicy *l=new ACP();
       FrameClass *f;
       DiskSpaceManagerLinux *dsml;
       printf("\n%d ---\n",l->replaceFrame(f,"venki",1,1,dsml));
       }*/




  "�