#include "FLH.h"
#include "standards.h"
#include <fcntl.h>
#include <sys/types.h> // for disk space manager i.e.., lseek ,off_t and size_t
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/************************************SingleLinkedList*****************************************/
FLH* FLH :: singleInstance_C=0;

  
FLH* FLH :: getInstance()
	{
	if(singleInstance_C==0)
		singleInstance_C= new FLH();
	return singleInstance_C;
	}	

FLH :: FLH()
	{
	fLHH=0;
	FL=0;
	INDEX=0;
	fLData=0;
	PNO=1;
	MAXIND= (PAGESIZE-2*sizeof(long)-2*sizeof(short))/sizeof(long);
	VSB = VSBufferManager :: getInstance();
	//printf("\nIN CONSTRUCTOR OF FLH");
	fLData=MSF::myMalloc(PAGESIZE);
	VSB->readFromBufferPool(1,fLData);
	fLHH=(FLHH*)fLData;			
	while(fLHH->flag==FLCHAINED) // if true read the contents of next page into fLData
		{
		//printf("\n\tIn CHAINED index=%d  prevPN=%ld  nextPN=%ld ",fLHH->index,fLHH->prevPN,fLHH->nextPN);
		PNO=fLHH->nextPN; 
		//printf("\tPNO %d",PNO);
		VSB->readFromBufferPool(fLHH->nextPN,fLData);
		fLHH=(FLHH*)fLData; //point the fLHH to new page;
		}
	
	if(fLHH->flag==FLEMPTY) //initial state , move FLpointer to list starting and index is 0  
		{
		//printf("\n\tIn EMPTY  index=%d  prevPN=%ld  nextPN=%ld",fLHH->index,fLHH->prevPN,fLHH->nextPN);
		if(fLHH->prevPN==-1) // this is page 1 of FL
			{
			FL=(long*) ( (char*)fLData + 2*sizeof(long) + 2*sizeof(short));
			//FLH::INDEX=0;
			}	
		}
					
	if(fLHH->flag==FLGROUNDED) // initialize the FL to point to the  
		{
		//printf("\n\tIn GROUNDED index=%d  prevPN=%ld  nextPN=%ld ",fLHH->index,fLHH->prevPN,fLHH->nextPN);
		FL=(long*) ( (char*)fLData+ 2*sizeof(long) + 2*sizeof(short) );
		FLH::INDEX=fLHH->index;
		//printf("\tPNO %ld, index %d",PNO,INDEX);
		}
	FLH::INDEX=fLHH->index;	 //bug cleared now
#ifdef VERBOSEMODE	
	displayFL();	
#endif	
	}

FLH :: ~FLH()
	{
	//printf("\n B$ FRE");
#ifdef VERBOSEMODE	
	displayFL();
#endif	
	VSB->writeToBufferPool(PNO,fLData); //bug cleared now
	if(fLData!=0)	MSF::myFree(fLData);
	
	//printf("\n IN destructor of FLH");
	}
	
void FLH :: cleanit(long pn)
	{
	void *cleandata=0;
	cleandata=MSF::myMalloc(PAGESIZE);
	VSB->writeToBufferPool(pn,cleandata);
	MSF::myFree(cleandata);
	}
			
void FLH :: addToFL(long pageNumber)// returns SUCCESS or FAILURE
	{
	DBHeader *dBH=0;
	void *tempData=0;
	cleanit(pageNumber);//to clear the data resident on it, so that no errors creep in
	if(INDEX<=MAXIND)
		{
		FL [ INDEX++ ] = pageNumber;
		fLHH->index++;
#ifdef VERBOSEMODE
		printf("\n\t\t\t\t\tIn AddToFL method of FLH Class;Free Page Added=%ld",FL[INDEX-1]);
#endif		
		VSB->writeToBufferPool(PNO,fLData);
		}
	else
		{
		//printf("\nPAGE IS FULL");
		
		tempData=MSF::myMalloc(PAGESIZE);
		VSB->readFromBufferPool(0,tempData);
		dBH=(DBHeader*)tempData;
		
		fLHH->nextPN=dBH->assignedPN;
		fLHH->flag=FLCHAINED;
		
		VSB->writeToBufferPool(PNO,fLData);
		
		fLHH->flag=FLGROUNDED;
		fLHH->prevPN=PNO;
		PNO=dBH->assignedPN++;
		fLHH->nextPN=FLEMPTY;
		fLHH->index=1;
		INDEX=1;
		FL=(long*) ( (char*)fLData+ 2*sizeof(long) + 2*sizeof(short) );
		FL[INDEX-1]=pageNumber;
#ifdef VERBOSEMODE
		printf("\n\t\t\t\t\tIn AddToFL method of FLH Class;Free Page Added=%ld",FL[INDEX-1]);
#endif		
		VSB->writeToBufferPool(0,tempData);
		}	
	if(tempData!=0)		MSF::myFree(tempData);	
	}	
	
	
long FLH :: getFreePage()// returns pageNumber if freepage exists else -1
	{
	int returnFlag=0;
	long retPN;
	void *tempData=0;
	DBHeader *dBH=0;
	if(INDEX>0)
		{
		retPN=FL[INDEX-1];
		//printf("\n\t\t >0 hiiiiiiiiii %ld %ld INDEX %d",retPN,FL[INDEX-1],INDEX);
		fLHH->index--;
		INDEX--;
		VSB->writeToBufferPool(PNO,fLData);
		}
	else
		{
		if(fLHH->prevPN==FLEMPTY)
			{
			tempData=MSF::myMalloc(PAGESIZE);
			VSB->readFromBufferPool(0,tempData);
			dBH=(DBHeader*)tempData;
			//printf("\n\t\tFLH::PageNumber is %d",dBH->assignedPN);
			retPN=dBH->assignedPN++;
			//printf("\n\t\tCurrent PageNumber is %d",dBH->assignedPN);
			VSB->writeToBufferPool(0,tempData);
			//printf("\nFAILED to Return Free Page Number: as I am new I donot have any PageNumbers");		
			goto GFPEND;	
			}
		else
			{
			retPN=PNO;	
			PNO=fLHH->prevPN;
			//printf("\nPrevious Page Number Assigned %d",PNO);
			VSB->readFromBufferPool(fLHH->prevPN,fLData);
			fLHH=(FLHH*)fLData;
			INDEX=fLHH->index;
			//printf("\tINDEX %d",INDEX);
			}
		//printf("\t <0 hiiiiiiiiii %d",retPN);	
		}
GFPEND:		
	if(tempData!=0)
		MSF::myFree(tempData);
#ifdef VERBOSEMODE
		printf("\n\t\t\t\t\tIn GetFreePage method of FLH class; free page returned=%ld",retPN);
#endif				
	return retPN;	
	}

void FLH :: displayFL()
	{
	int i=0;
	printf("\n\tIn Display FL");
	if(INDEX==0) printf("\t FL is Empty\n");
	for(i=0;i<INDEX;i++)	printf("\t %ld",FL[i]);
	}	

void FLH :: printFLH()
	{
	printf("\n\tIN FLH: printFLH");
	printf("\n\t\tfLHH->flag=%d,fLHH->index=%d,fLHH->prevPN=%d,fLHH->nextPN=%d",fLHH->flag,fLHH->index,fLHH->prevPN,fLHH->nextPN);
	}
/*	
int main()
	{
	VSBufferManager *vsb=VSBufferManager::getInstance();
	DiskSpaceManagerLinux *dSML_C=DiskSpaceManagerLinux::getInstance(DBFILENAME);
	ReplacementPolicy *rP_C= vsb->getRPFromFactory(RPFIFO);
	vsb->initializeBufferPool(15,dSML_C,rP_C);
	DBHeader *dBH_C;
	FLHH *flh;
	void * da,*datum;
	
	da=MSF::myMalloc(PAGESIZE);
	datum=MSF::myMalloc(PAGESIZE);
	
	dBH_C=(DBHeader*)da;
	dBH_C->fpListPointer=1;
       	dBH_C->dbCatPointer=2;
        dBH_C->tCatPointer=3;
       	dBH_C->aCatPointer=4;
        dBH_C->iCatPointer=5;
       	dBH_C->assignedPN=6;	
	vsb->writeToBufferPool(0,da);
	
	
	flh=(FLHH*)datum;
	flh->flag=FLEMPTY;
	flh->prevPN=FLEMPTY;
	flh->nextPN=FLEMPTY;
	flh->index=0;
	vsb->writeToBufferPool(1,datum);
	
	int i;
	
	FLH *fLH=FLH::getInstance();
	for(i=0;i<15;i++)
		printf("freepageNumber %ld",fLH->getFreePage());
	fLH->displayFL();
	
	//for(i=0;i<500;i++)
	//	fLH->addToFL(i+2);
	//fLH->displayFL();
	//for(i=0;i<150;i++)
	//	printf("freepageNumber %ld",fLH->getFreePage());
	printf("\nMAX %d\n",FLH::MAXIND);
	fLH->displayFL();
	fLH->~FLH();
	vsb->readFromBufferPool(0,datum);
	dBH_C=(DBHeader*)datum;
	printf("\n%d is the assigned PN\n",dBH_C->assignedPN);
	vsb->readFromBufferPool(1,da);
	flh=(FLHH*)da;
	printf("\n\tIn flag=%d  index=%d  prevPN=%ld  nextPN=%ld\n",flh->flag,flh->index,flh->prevPN,flh->nextPN);
	vsb->readFromBufferPool(6,da);
	flh=(FLHH*)da;
	printf("\n\tIn flag=%d  index=%d  prevPN=%ld  nextPN=%ld\n",flh->flag,flh->index,flh->prevPN,flh->nextPN);
	vsb->displayBufferPool();
	vsb->~VSBufferManager();
	printf("\n");
	}*/

  E�