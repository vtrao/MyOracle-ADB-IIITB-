#include "VIndexManager.h"
#include "BufferManager/standards.h"
#include "math.h"
//#define VIMDEBUG 15
Comparision* Comparision :: singleInstance_C=0;

Comparision* Comparision :: getInstance()
	{
	if(singleInstance_C==0)
		{
		singleInstance_C=new Comparision();
		}
	return singleInstance_C;	
	}
	
Comparision :: Comparision()
	{
	}
		
int Comparision :: dtcmp(char* d1,char* d2)
	{
	int i,j,date1,date2,result=0;
	char *d3=0,*d4=0;
	d3=(char*)MSF::myMalloc(9);
	d4=(char*)MSF::myMalloc(9);
	for(j=0,i=0;i<strlen(d1);i++)
		if(d1[i]>=48 && d1[i]<=57)	d3[j++]=d1[i];
	d3[j]='\0';
	for(j=0,i=0;i<strlen(d2);i++)
		if(d2[i]>=48 && d2[i]<=57)	d4[j++]=d2[i];
        d4[j]='\0';
	date1=atoi(d3);
	date2=atoi(d4);
	if(date1==date2)	result=CE;
	else if(date1<date2)	result=CL;
	else			result=CG;
	if(d3!=0)	MSF::myFree(d3);	
	if(d4!=0)	MSF::myFree(d4);
	return result;
	}
	
int Comparision :: compareLEG(KeyList *khead,char *key) //LEG- LESS EQUAL GREATER, for KEYS AND NONKEYS
	{
	int result;
	while(khead!=NULL)
		{
		if(khead->type==DBINTTYPE)
			{
			if(atoi(khead->value)==atoi(key))       return CE;
			else if(atoi(khead->value)<atoi(key))	return CL;
			else				     	return CG;
			}	
		else if(khead->type==DBCHARTYPE)
			{
			result=strcmp(khead->value,key);			
			if(0==result)	  return CE;
			else if(0>result) return CL;
			else  		  return CG;
			}	//got to add code for data and floatsssss, support -ves	
		else if(khead->type==DBDATETYPE)
			result=dtcmp(khead->value,key);	
		khead=khead->next;
		}
	}

int Comparision :: compareLEG(KeyList *khead,char *key,int dummyforoverloading)//LEG- LESS EQUAL GREATER, for COMPOSITE INDEXES
	{
	int result=0,i=0,j=0,flag=0;
	char *temp=0;
	temp=(char*)MSF::myMalloc(strlen(key));
	//VIndexManager::displayKeyList(khead);
	while(khead!=NULL)
		{j=0;
		for(;key[i]!='#';i++)
			{
			temp[j++]=key[i];
			if(i==strlen(key)) 
				{
				flag=1;
				break;
				}
			}
		temp[j]='\0';
		#ifdef VIMDEBUG
		printf("\n%s",temp);	
		#endif
		i++;	
		if(khead->type==DBINTTYPE)
			{
			if(atoi(khead->value)==atoi(temp))	result=CE;
			else if(atoi(khead->value)<atoi(temp))  result=CL;
			else					result=CG;
			#ifdef VIMDEBUG
			printf("\n\tIN INTCOMP  result %d khead %s==%s temp",result,khead->value,temp);
			#endif
			}
		else if (khead->type==DBCHARTYPE)
			{
			result=strcmp(khead->value,temp);
			if(0==result)		result=CE;
			else if(result<0)	result=CL;
			else			result=CG;
			#ifdef VIMDEBUG
			printf("\n\tIN CHARCOMP result %d khead %s==%s temp",result,khead->value,temp);
			#endif
			}	
		else if (khead->type==DBDATETYPE)
			{
			result=dtcmp(khead->value,temp);
			#ifdef VIMDEBUG
			printf("\n\tIN DATECOMP result %d khead %s==%s temp",result,khead->value,temp);
			#endif
			}
		khead=khead->next;	
		if(result!=CE||khead==NULL)	goto CLEND;
		if(flag==1) 			goto CLEND;		
		}	
CLEND:	//printf("\n VENKI HERER");
	if(temp!=0)	MSF::myFree(temp);	
	return result;
	}
			
//*****************************************INDEX MANAGER*************************************************************			
VIndexManager :: VIndexManager()
	{
	//printf("\n\tIn Constructor of VIndexManager");
	vSB_C=VSBufferManager :: getInstance();	
	com_C=Comparision :: getInstance();
	equalityFlag_C=0;
	duplicateFlag_C=-10;
	fLH_C=FLH::getInstance();
	TempDatumC=MSF::myMalloc(PAGESIZE);
	DatumC=MSF::myMalloc(PAGESIZE);
	iMHC=(IMHeaderPage*)MSF::myMalloc(5*sizeof(long)+8*sizeof(short));
	}

void VIndexManager :: insertToRIDList(RID **rID,long pgno,short slotID)
	{
	if(*rID==NULL)
		{
		*rID=(RID*)MSF::myMalloc(sizeof(RID));
		(*rID)->pageNumber=pgno; (*rID)->slotID=slotID;
		(*rID)->next=NULL;
		}
	else
		{
		RID *t=*rID;
		while(t->next!=NULL)
			t=t->next;
		RID *temp=(RID*)MSF::myMalloc(sizeof(RID));
		temp->pageNumber=pgno; temp->slotID=slotID;
		t->next=temp;
		temp->next=NULL;	
		}
	}

void VIndexManager :: displayRIDList(RID *rID)
	{
	while(rID!=NULL)
		{
		printf("\n\tPGNO %ld \t SlotID %d",rID->pageNumber,rID->slotID);
		rID=rID->next;
		}
	}

void VIndexManager :: freeRIDList(RID *rID)
	{
	RID *temp=rID;
	while(rID!=NULL)
		{
		temp=rID;
		rID=rID->next;
		MSF::myFree(temp);
		}	
	}
	
//These methods are for building the data structure for Creation of Index for a (composite) attribute				
void VIndexManager :: addToFieldList(FieldNameType **head,short fieldType,char *fieldName,short size) //static
	{
	if(*head==NULL)
		{
		*head=(FieldNameType*)MSF::myMalloc(sizeof(FieldNameType));
		(*head)->fieldType=fieldType;
		(*head)->size=size;
		(*head)->fieldName=(char*)MSF::myMalloc(strlen(fieldName));
		strcpy((*head)->fieldName,fieldName);
		(*head)->next=NULL;
		}
	else
		{
		FieldNameType *t=*head;
		while(t->next!=NULL)
			t=t->next;
		FieldNameType *temp=(FieldNameType*)MSF::myMalloc(sizeof(FieldNameType));
		temp->fieldType=fieldType;
		temp->size=size;
		temp->fieldName=(char*)MSF::myMalloc(strlen(fieldName));
		strcpy(temp->fieldName,fieldName);
		temp->next=NULL;
		t->next=temp;	
		}	
	}
		
void VIndexManager :: displayFieldList(FieldNameType *head) //static
	{
	printf("\n\tFieldName\t\tFieldType\t  Size");
	printf("\n-------------------------------------------");
	while(head!=NULL)
		{
		printf("\n\t%-15s\t%5d\t%5d",head->fieldName,head->fieldType,head->size);
		head=head->next;
		}
	}
	
void VIndexManager :: freeFieldList(FieldNameType *head)
	{
	FieldNameType *temp=head;
	if(head!=0)
		while(head!=NULL)
			{
			temp=head;
			MSF::myFree(head->fieldName);
			head=head->next;
			MSF::myFree(temp);
			}	
	}	

//These methods are for building the datastructure for inserting or searching a key in the tree index		
void VIndexManager :: addToKeyList(KeyList **head,short type,char *valuep)
	{
	if(*head==NULL)
		{
		*head=(KeyList*)MSF::myMalloc(sizeof(KeyList));			
		(*head)->value=(char*)MSF::myMalloc(strlen(valuep));
		strcpy((*head)->value,valuep);
		(*head)->type=type;
		(*head)->next=NULL;
		//printf("\n%s %s",valuep,(*head)->value);
		}
	else
		{
		KeyList *t=(*head);	
		while(t->next!=NULL)
			{
			t=t->next;
			}
		KeyList *temp;	
		temp=(KeyList*)MSF::myMalloc(sizeof(KeyList));
        	temp->value=(char*)MSF::myMalloc(strlen(valuep));
		strcpy(temp->value,valuep);
		temp->type=type;
		t->next=temp;
		temp->next=NULL;
		}
	}
	
void VIndexManager :: displayKeyList(KeyList *head)
	{
	printf("\nDisplaying the contents of Key List DataStructure\t");
	while(head!=NULL)
		{
		if(head->type==DBCHARTYPE || head->type==DBDATETYPE)
			printf("\n\t%s %d",head->value,head->type);
		else if(head->type==DBINTTYPE)
			printf("\n\t%d %d",atoi(head->value),head->type);	
		head=head->next;
		}
	}

void VIndexManager :: copyKeyList(KeyList *source,KeyList **destination)
	{
	while(source!=NULL)
		{
		addToKeyList(destination,source->type,source->value);
		source=source->next;
		}
	}	
	
void VIndexManager :: copyKeyList(KeyList *source,KeyList **destination,RID *rid_P)
	{
	int i=0;
	while(source!=NULL)
		{
		#ifdef VIMDEBUG
		printf("\nIN COPY LIST");
		#endif
		addToKeyList(destination,source->type,source->value);
		if(i==0 && (keyType_C==COMPOSITENONKEY || keyType_C==NONKEY) )
			{
			char *df=(char*)MSF::myMalloc(6);
			df=MSF::itoa(rid_P->pageNumber,df);
			addToKeyList(destination,DBINTTYPE,df);
			df=MSF::itoa(rid_P->slotID,df);
			addToKeyList(destination,DBINTTYPE,df);
			#ifdef VIMDEBUG
			printf("\nFREE :: IN COPY LIST");
			#endif
			MSF::myFree(df);
			}
		source=source->next;
		i++;
		}
	#ifdef VIMDEBUG	
	printf("\nEND :: IN COPY LIST");	
	#endif
	}	

void VIndexManager :: freeKeyList(KeyList *head)
	{
	KeyList *temp;
	if(head!=0)
		{
		while(head!=NULL)
			{
			temp=head;
			MSF::myFree(head->value);
			head=head->next;
			MSF::myFree(temp);
			}
		}			
	}
		
void VIndexManager :: dispI(void *data,int nol,int pageNumber)
	{
	int i=0;
	void *datum=MSF::myMalloc(PAGESIZE);
	IndexHeader *iH;
	IndexKeyPagePair *iKPP;
	iH=(IndexHeader*)data;
	LeafHeader *lH;
	LeafKeyRIDPair *lKRP;
	iKPP=(IndexKeyPagePair*)((char*)data+2*sizeof(short)+1*sizeof(long));
	for(i=nol-iH->level;i>=0;i--) printf("\t");
	if(iH->level!=0)
		{
		printf("INDEX: %3d-->",pageNumber);
		printf("%3d|%3d|%3d",iH->level,iH->noe,iH->page0);
		for(i=0;i<iH->noe;i++)
			{
			printf(" |[%s]|%3d ",iKPP->key,iKPP->pageNumber);
			iKPP=(IndexKeyPagePair*)((char*)data+8+(i+1)*(4+keySize_C));
			}
		printf("\n\n");
		iKPP=(IndexKeyPagePair*)((char*)data+2*sizeof(short)+1*sizeof(long));
		if(iH->page0!=-1)
			{
			vSB_C->readFromBufferPool(iH->page0,datum);
			dispI(datum,nol,iH->page0);printf("\n");
			}
		for(i=0;i<iH->noe;i++)
			{
			vSB_C->readFromBufferPool(iKPP->pageNumber,datum);
			dispI(datum,nol,iKPP->pageNumber);
			iKPP=(IndexKeyPagePair*)((char*)data+8+(i+1)*(4+keySize_C));
			printf("\n");
			}
		}
	else
		{
		printf("LEAF: %3d-->",pageNumber);
		lH=(LeafHeader*)data;
		lKRP=(LeafKeyRIDPair*)((char*)data+2*sizeof(long)+2*sizeof(short));
		printf("%3d|%3d|%3d|%3d",lH->level,lH->noe,lH->prevPage,lH->nextPage);
		for(i=0;i<lH->noe;i++)
			{
			printf(" |[%s(%3d,%3d)] ",lKRP->key,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
			lKRP=(LeafKeyRIDPair*)((char*)data+12+(i+1)*(6+keySize_C));
			}	
		}				
	MSF::myFree(datum);	
	}
		
void VIndexManager :: displayTree(long indexPN)
	{
	printf("\n Displaying the Index Tree at %d\n",indexPN);
	void *indexData=0;
	void *data=0;
	indexData=MSF::myMalloc(PAGESIZE);
	data=MSF::myMalloc(PAGESIZE);
	vSB_C->readFromBufferPool(indexPN,indexData);
	IMHeaderPage *imph;
	imph=(IMHeaderPage*)indexData;
	printf("\nRoot PageNumber is at\n");
	vSB_C->readFromBufferPool(imph->rootPN,data);
	dispI(data,imph->nol,imph->rootPN);
	if(data!=0)		MSF::myFree(data);	
	if(indexData!=0)	MSF::myFree(indexData);
	}

	
	
void VIndexManager :: freeI(void *data,int pageNumber)
	{
	int i=0;
	void *datum=MSF::myMalloc(PAGESIZE);
	IndexHeader *iH;
	IndexKeyPagePair *iKPP;
	iH=(IndexHeader*)data;
	LeafHeader *lH;
	LeafKeyRIDPair *lKRP;
	iKPP=(IndexKeyPagePair*)((char*)data+2*sizeof(short)+1*sizeof(long));
	if(iH->level!=0)
		{
		iKPP=(IndexKeyPagePair*)((char*)data+2*sizeof(short)+1*sizeof(long));
		if(iH->page0!=-1)
			{
			vSB_C->readFromBufferPool(iH->page0,datum);
			freeI(datum,iH->page0);
			}
		for(i=0;i<iH->noe;i++)
			{
			vSB_C->readFromBufferPool(iKPP->pageNumber,datum);
			freeI(datum,iKPP->pageNumber);
			iKPP=(IndexKeyPagePair*)((char*)data+8+(i+1)*(4+keySize_C));
			}
		}
	else
		{
		}
	printf("\n Adding this to FL %d",pageNumber);
	fLH_C->addToFL(pageNumber);					
	MSF::myFree(datum);	
	}
		
void VIndexManager :: freeIndexTree(long indexPN)
	{
	void *indexData=0;
	void *data=0;
	indexData=MSF::myMalloc(PAGESIZE);
	data=MSF::myMalloc(PAGESIZE);
	vSB_C->readFromBufferPool(indexPN,indexData);
	IMHeaderPage *imph;
	imph=(IMHeaderPage*)indexData;
	vSB_C->readFromBufferPool(imph->rootPN,data);
	freeI(data,imph->rootPN);
	printf("\n Adding this to FL %d",indexPN);
	fLH_C->addToFL(indexPN);
	if(data!=0)		MSF::myFree(data);	
	if(indexData!=0)	MSF::myFree(indexData);
	}
			
//***********************Actual functionality of INDEX MANAGER***********************************************//			
int VIndexManager :: createIndexTree(FieldNameType *fields,short keyType)
	{
#ifdef VERBOSEMODE
	printf("\nInside createIndexTree method of VIndexManager class");	
#endif		
	int number=0,i=0; // number of fields		
	FieldNameType *tempf=fields; 
	while(tempf!=NULL) 
		{
		tempf=tempf->next;
		number++;
		}		
	long pageNumber=fLH_C->getFreePage();
	void *iHData=0,*rPData=0;;
	iHData=MSF::myMalloc(PAGESIZE);
	rPData=MSF::myMalloc(PAGESIZE);
	IMHeaderPage *imhp=0;
	imhp=(IMHeaderPage*)MSF::myMalloc(8*sizeof(short)+5*sizeof(long));
	imhp->rootPN=fLH_C->getFreePage();
	imhp->leafPN=-1;       // means not yet created
	imhp->nop=1;           //number of pages
	imhp->nol=1;           //number of levels
	imhp->nok=0;           //number of keys
	imhp->keyType=keyType; //i.e.., COMPOSITE or SIngle or key of non key
	imhp->nof=number;      //i.e.., number of fields
	imhp->keySize=0;
	Fields *fds=0;
	fds=(Fields*)MSF::myMalloc(2*sizeof(short)+MAXFIELDNAME);	
	while(fields!=NULL)
		{
		fds->fieldType=fields->fieldType;
		fds->size=fields->size;
		imhp->keySize+=fds->size+1; // think for composite key;
		strcpy(fds->fieldName,fields->fieldName);
		memcpy((char*)iHData+8*sizeof(short)+5*sizeof(long)+i*(2*sizeof(short)+MAXFIELDNAME),fds,
		                                                       2*sizeof(short)+MAXFIELDNAME);
		if(i==0 && (keyType==COMPOSITENONKEY || keyType==NONKEY) )
			{
			i++;
			imhp->keySize+=1*sizeof(short)+1*sizeof(long)+2;
			fds->fieldType=DBINTTYPE;
			fds->size=sizeof(long);
			strcpy(fds->fieldName,"PN");
			memcpy((char*)iHData+8*sizeof(short)+5*sizeof(long)+i*(2*sizeof(short)+MAXFIELDNAME),fds,
		                                                       2*sizeof(short)+MAXFIELDNAME);
			i++;
			fds->fieldType=DBINTTYPE;
			fds->size=sizeof(short);
			strcpy(fds->fieldName,"SID");
			memcpy((char*)iHData+8*sizeof(short)+5*sizeof(long)+i*(2*sizeof(short)+MAXFIELDNAME),fds,
		                                                       2*sizeof(short)+MAXFIELDNAME);	
			imhp->nof+=2;					       				       
			}	
		i++;
		fields=fields->next;
		}
	imhp->keySize-=1;	
	imhp->iFanout=(PAGESIZE-2*sizeof(short)-1*sizeof(long))/(imhp->keySize+1*sizeof(long));
	imhp->lFanout=(PAGESIZE-2*sizeof(short)-2*sizeof(long))/(imhp->keySize+1*sizeof(short)+1*sizeof(long));
	imhp->mIK=(long)pow(imhp->iFanout,3);		
	memcpy(iHData,imhp,8*sizeof(short)+5*sizeof(long));	
	IndexHeader *iH=(IndexHeader*)rPData;
	iH->level=1;
	iH->noe=0;
	iH->page0=-1; // indicates that the page is empty and no keys are inserted till now
	vSB_C->writeToBufferPool(imhp->rootPN,rPData);	 //writing the root page contents to bufferpool
	vSB_C->writeToBufferPool(pageNumber,iHData);	//writing the index header page contents to bufferpool
	if(iHData!=0)		MSF::myFree(iHData);
	if(rPData!=0)		MSF::myFree(rPData);
	if(imhp!=0)		MSF::myFree(imhp);
	if(fds!=0)		MSF::myFree(fds);	
	return pageNumber;	
	}

//***********************Actual functionality of INDEX MANAGER***********************************************//			
void VIndexManager :: displayIndexTreeStats(int indexPN)
	{
	void *iHData=0;
	int i=0;
	iHData=MSF::myMalloc(PAGESIZE);
	vSB_C->readFromBufferPool(indexPN,iHData);
	IMHeaderPage *imph;
	imph=(IMHeaderPage*)iHData;
	printf("\n\n\tIndex Stats of INDEX STRUCTURE AT %ld",indexPN);
	printf("\n-----------------------------------------------------");
	printf("\n\tRoot Page is at        %ld",imph->rootPN);
	printf("\n\tFirst Leaf Page is at  %ld",imph->leafPN);
	printf("\n\tNumber of Pages        %ld",imph->nop);
	printf("\n\tNumber of Levels       %d",imph->nol);
	printf("\n\tNumber of Keys         %ld",imph->nok);
	printf("\n\tNumber of Fields       %d",imph->nof);
	if(imph->keyType==COMPOSITEKEY)
		printf("\n\tKey Type :             COMPOSITEKEY");
	else if(imph->keyType==IKEY)
		printf("\n\tKey Type :             KEY");
	else if(imph->keyType==NONKEY)
		printf("\n\tKey Type :             NONKEY FIELD");
	else if(imph->keyType==COMPOSITENONKEY)
		printf("\n\tKey Type :             COMPOSITENONKEY");
	printf("\n\tFanout of IndexPages   %d",imph->iFanout);
	printf("\n\tFanout of LeafPages    %d",imph->lFanout);
	printf("\n\tSize of Key            %d",imph->keySize);
	printf("\n\tMaxi Indexable Keys    %ld (When the height of tree is considered as 3)",imph->mIK);	
	Fields *fds=0;
	fds=(Fields*)((char*)iHData + 5*sizeof(long) +8*sizeof(short));
	int number=imph->nof;
	printf("\n\n\t\tFIELDNAME\tFIELDSIZE\tFIELDTYPE");
	printf("\n---------------------------------------------------------------------");
	while(number!=0)
		{i++;
		printf("\n\t%15s\t\t%5d",fds->fieldName,fds->size);
		if(fds->fieldType==DBINTTYPE)
			printf("\t\tINT");
		else if(fds->fieldType==DBDATETYPE)
			printf("\t\tDATE");
		else if(fds->fieldType==DBCHARTYPE)
			printf("\t\tCHAR");
		fds=(Fields*)((char*)iHData+8*sizeof(short)+5*sizeof(long)+i*(2*sizeof(short)+MAXFIELDNAME));
		number--;
		}
	if(iHData!=0)		MSF::myFree(iHData);
	}
	
void VIndexManager :: displayAllLeaf(long indexPN)
	{
	void *iHData=0,*data=0;
	iHData=MSF::myMalloc(PAGESIZE);
	vSB_C->readFromBufferPool(indexPN,iHData);
	int i=0;  
	IMHeaderPage *imph;
	imph=(IMHeaderPage*)iHData;
	LeafHeader *lH;
	LeafKeyRIDPair *lKRP;
	data=MSF::myMalloc(PAGESIZE);
	printf("\nI DISPLAY ALL LEAF %3d",indexPN);
	if(imph->leafPN!=-1)
		{
		long nextPage=imph->leafPN;
		printf("\nIN Display All Leaf of Index at %3d\n", indexPN);
		do
			{
			printf("\nLeaf Values at page %3d\n",nextPage);
			vSB_C->readFromBufferPool(nextPage,data);
			lH=(LeafHeader*)data;
			lKRP=(LeafKeyRIDPair*)((char*)data+2*sizeof(long)+2*sizeof(short));
			for(i=0;i<lH->noe;i++)
				{
				printf("Key=%s;RID(%3d,%3d)\t   ",lKRP->key,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
				lKRP=(LeafKeyRIDPair*)((char*)data+12+(i+1)*(6+imph->keySize));
				}	
			nextPage=lH->nextPage;			
			}while(nextPage!=-1);
		}
	if(iHData!=0)		MSF::myFree(iHData);
	if(data!=0)		MSF::myFree(data);
	}	

void VIndexManager :: displayNode(long pN)
	{
	void *data=0;
	data=MSF::myMalloc(PAGESIZE);
	int i=0;
	vSB_C->readFromBufferPool(pN,data);
	IndexHeader *iH;
	IndexKeyPagePair *iKPP;
	iH=(IndexHeader*)data;
	LeafHeader *lH;
	LeafKeyRIDPair *lKRP;
	if(iH->level!=0)
		{
		iKPP=(IndexKeyPagePair*)((char*)data+2*sizeof(short)+1*sizeof(long));
		printf("\nIN DISPLAY INDEX NODE\n\t level=%d; noe=%d; page0=%d\n",iH->level,iH->noe,iH->page0);
		for(i=0;i<iH->noe;i++)
			{
			printf("Key=%s;PageNumber %d\t",iKPP->key,iKPP->pageNumber);
			iKPP=(IndexKeyPagePair*)((char*)data+8+(i+1)*(4+keySize_C));
			}
		}
	else
		{
		lH=(LeafHeader*)data;
		lKRP=(LeafKeyRIDPair*)((char*)data+2*sizeof(long)+2*sizeof(short));
		printf("\nIN DISPLAY LEAF NODE\n\t level=%d; noe=%d; prevPage=%d; nextPage=%d\n",lH->level,lH->noe,lH->prevPage,lH->nextPage);
		for(i=0;i<lH->noe;i++)
			{
			//printf("\n\t\tdata %u,lKRP %u\n",data,lKRP);
			printf("Key=%s;RID(%d,%d)\t   ",lKRP->key,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
			lKRP=(LeafKeyRIDPair*)((char*)data+12+(i+1)*(6+keySize_C));
			}	
		}		
	if(data!=0)	MSF::myFree(data);	
	}

	
	
void VIndexManager :: updateIndexHeader()
	{
	
	vSB_C->readFromBufferPool(indexPN_C,TempDatumC);
	
	iMHC->rootPN = rootPN_C;
	iMHC->leafPN = leafPN_C;
	iMHC->nop = nop_C;
	iMHC->nol = nol_C;
	iMHC->nok = nok_C;
	iMHC->nof = nof_C;
	#ifdef VIMDEBUG
	printf("\n b4 entry");
	#endif
	if(duplicateFlag_C==COMPOSITENONKEY || duplicateFlag_C==NONKEY)
		{
		#ifdef VIMDEBUG
		printf("\n Entered for redset");
		#endif
		iMHC->keyType=duplicateFlag_C;
		}
	else
		iMHC->keyType = keyType_C;
	iMHC->iFanout = iFanout_C;
	iMHC->lFanout = lFanout_C;
	iMHC->keySize = keySize_C;
	iMHC->mIK = mIK_C;
	memcpy(DatumC,iMHC,8*sizeof(short)+5*sizeof(long));
	memcpy((char*)DatumC+8*sizeof(short)+5*sizeof(long),(char*)TempDatumC+8*sizeof(short)+5*sizeof(long),PAGESIZE-(8*sizeof(short)+5*sizeof(long)));
	vSB_C->writeToBufferPool(indexPN_C,DatumC);
	
	
	}			
//***********************Actual functionality of INDEX MANAGER***********************************************//
int VIndexManager :: find(long indexPN,KeyList* khead,short operation,RID **rid)
	{
#ifdef VERBOSEMODE
	printf("\nInside find method of VIndexManager class");	
#endif		
	LeafHeader *lH=0;
	LeafKeyRIDPair *lKRP=0;
	int index=0;
	int returnFlag=0,i=0;
	void *iHData=0;long leafPN=0;
	void *leafData=0;
	leafData=MSF::myMalloc(PAGESIZE);
	iHData=MSF::myMalloc(PAGESIZE);
	vSB_C->readFromBufferPool(indexPN,iHData); //reading the Index Header
	IMHeaderPage *imph;
	imph=(IMHeaderPage*)iHData;	
	Fields *fds;
	fds=(Fields*)((char*)iHData + 5*sizeof(long) +8*sizeof(short));	
	int number=imph->nof;
	KeyList *temp=khead;
	keySize_C=imph->keySize; // -1 because already 1 is in the size of either IndexKeyPagePair or LeafKeyRIDPair
	keyType_C=imph->keyType;
	iFanout_C=imph->iFanout;
	lFanout_C=imph->lFanout;
	rootPN_C =imph->rootPN;
	indexPN_C=indexPN;
	leafPN_C =imph->leafPN;
	duplicateFlag_C=-10;
	nop_C    =imph->nop;
	nol_C    =imph->nol;
	nok_C    =imph->nok;
	nof_C    =imph->nof;
	mIK_C    =imph->mIK; 
	while(number>0 && temp!=NULL)  //step 1 typechecking of the keys if success process contines else it terminates without inserting
		{
		fds=(Fields*)((char*)iHData+8*sizeof(short)+5*sizeof(long)+i*(2*sizeof(short)+MAXFIELDNAME));	
		if(fds->fieldType!=temp->type)
			{
			//printf("\nVenki problem is here buddy");
			returnFlag=1;
			goto SITEND;
			}
		
		if(number==imph->nof && (keyType_C==COMPOSITENONKEY || keyType_C==NONKEY) )
			{
			i=i+2;
			number-=2;
			}			
		number--;
		i++;
		temp=temp->next;
		}
	//printf("\n Venki is here naaa\n");		
	leafPN=treeSearch(imph->rootPN,khead);	
	vSB_C->readFromBufferPool(leafPN,leafData);
	lH=(LeafHeader*)leafData;
	index=binarySearchLeaf(leafData,khead,1,lH->noe); //finding the right place for the new key
	lKRP=(LeafKeyRIDPair*)((char*)leafData + (2*sizeof(long)+2*sizeof(short)) +     (index-1) * (1*sizeof(long)+1*sizeof(short)+keySize_C));
	//printf("\n%d is the index and %d is pn and %d is slotID",index,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
	if(rid!=NULL)
		{
		if(operation == EQ)
			{
			if(equalityFlag_C==1)
				VIndexManager::insertToRIDList(rid,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
			else
				returnFlag=1;
			}		
		else if(operation == LE || operation == LT)
			{
			if(operation == LE && equalityFlag_C==1)
				{
				VIndexManager::insertToRIDList(rid,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
				//printf("\t%s",lKRP->key);
				}
			for(i=index-1;i>0;i--)
				{
				lKRP=(LeafKeyRIDPair*)((char*)leafData + (2*sizeof(long)+2*sizeof(short)) +     
				                             (i-1) * (1*sizeof(long)+1*sizeof(short)+keySize_C));
				VIndexManager::insertToRIDList(rid,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
				//printf("\t%s",lKRP->key);
				}
			while(lH->prevPage!=-1)
				{
				//printf("\n Venki is here %d is leaf pn\n",lH->prevPage);	
				vSB_C->readFromBufferPool(lH->prevPage,leafData);
				lH=(LeafHeader*)leafData;
				for(i=lH->noe;i>0;i--)
					{
					lKRP=(LeafKeyRIDPair*)((char*)leafData + (2*sizeof(long)+2*sizeof(short)) +     
				                             (i-1) * (1*sizeof(long)+1*sizeof(short)+keySize_C));
					//printf("\t%s",lKRP->key);
					VIndexManager::insertToRIDList(rid,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
					}	
				}	
			}
		else if(operation == GE || operation == GT)
			{
			if(operation == GE && equalityFlag_C==1)
				{
				VIndexManager::insertToRIDList(rid,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
				//printf("\t%s",lKRP->key);
				}
			for(i=index;i<lH->noe;i++)
				{
				lKRP=(LeafKeyRIDPair*)((char*)leafData + (2*sizeof(long)+2*sizeof(short)) +     
				                             (i) * (1*sizeof(long)+1*sizeof(short)+keySize_C));
				VIndexManager::insertToRIDList(rid,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
				//printf("\t%s",lKRP->key);
				}
			while(lH->nextPage!=-1)
				{
				//printf("\n Venki is here %d is leaf pn\n",lH->nextPage);	
				vSB_C->readFromBufferPool(lH->nextPage,leafData);
				lH=(LeafHeader*)leafData;
				for(i=1;i<=lH->noe;i++)
					{
					lKRP=(LeafKeyRIDPair*)((char*)leafData + (2*sizeof(long)+2*sizeof(short)) +     
				                             (i-1) * (1*sizeof(long)+1*sizeof(short)+keySize_C));
					//printf("\t%s",lKRP->key);
					VIndexManager::insertToRIDList(rid,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
					}	
				}	
			}
		else //not equal to case
			{
			if(equalityFlag_C!=1)
				{
				VIndexManager::insertToRIDList(rid,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
				//printf("\t%s",lKRP->key);
				}
			void *tempData=MSF::myMalloc(PAGESIZE);
			for(i=index-1;i>0;i--) //printing the lesser values
				{
				lKRP=(LeafKeyRIDPair*)((char*)leafData + (2*sizeof(long)+2*sizeof(short)) +     
				                             (i-1) * (1*sizeof(long)+1*sizeof(short)+keySize_C));
				VIndexManager::insertToRIDList(rid,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
				//printf("\t%s",lKRP->key);
				}
				
			for(i=index;i<lH->noe;i++) //printing the greater values
				{
				lKRP=(LeafKeyRIDPair*)((char*)leafData + (2*sizeof(long)+2*sizeof(short)) +     
				                             (i) * (1*sizeof(long)+1*sizeof(short)+keySize_C));
				VIndexManager::insertToRIDList(rid,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
				//printf("\t%s",lKRP->key);
				}	
				
			memcpy(tempData,leafData,PAGESIZE);	
			while(lH->prevPage!=-1)
				{
			//	printf("\n Venki is here %d is leaf pn\n",lH->prevPage);	
				vSB_C->readFromBufferPool(lH->prevPage,tempData);
				lH=(LeafHeader*)tempData;
				for(i=lH->noe;i>0;i--)
					{
					lKRP=(LeafKeyRIDPair*)((char*)tempData + (2*sizeof(long)+2*sizeof(short)) +     
				                             (i-1) * (1*sizeof(long)+1*sizeof(short)+keySize_C));
					//printf("\t%s",lKRP->key);
					VIndexManager::insertToRIDList(rid,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
					}	
				}		
			
			memcpy(tempData,leafData,PAGESIZE);
			lH=(LeafHeader*)leafData;
			while(lH->nextPage!=-1)
				{
				//printf("\n Venki is here %d is leaf pn\n",lH->nextPage);	
				vSB_C->readFromBufferPool(lH->nextPage,tempData);
				lH=(LeafHeader*)tempData;
				for(i=1;i<=lH->noe;i++)
					{
					lKRP=(LeafKeyRIDPair*)((char*)tempData + (2*sizeof(long)+2*sizeof(short)) +     
				                             (i-1) * (1*sizeof(long)+1*sizeof(short)+keySize_C));
					//printf("\t%s",lKRP->key);
					VIndexManager::insertToRIDList(rid,(lKRP->rID).pageNumber,(lKRP->rID).slotID);
					}	
				}
			MSF::myFree(tempData);		
			}		
		}	
	if(equalityFlag_C==1)	
		{
		equalityFlag_C=0;
		//printf("\n EQULA");
		}	
	else
		{
		returnFlag=1;
		}	
SITEND:				
	if(iHData!=0)			MSF::myFree(iHData);	
	if(leafData!=0)			MSF::myFree(leafData);
	if(returnFlag==1)		return FAILED;
	else            		return leafPN; //actual searching starts here		
	}

//*********************************************Damn Perfect upto thisssssssss***************************************/	
int VIndexManager :: treeSearch(long pN,KeyList *khead) // can be called from insert as well
	{
	int returnFlag=0,i=0,indexx=0;
	long result=0;
	void *nodeData=0;
	nodeData=MSF::myMalloc(PAGESIZE);
	vSB_C->readFromBufferPool(pN,nodeData);
	IndexHeader *iH;
	iH=(IndexHeader*)nodeData;
	IndexKeyPagePair *iKPP=0,*tempIKPP=0;
	if(iH->level==0) //if leaf node return the pagenumber of leaf
		{
		returnFlag=1; //1 means return the pN
		goto TSEND;
		}
	else            //non leaf i.e.., index pages so proceed
		{
		if(iH->noe==0)                              //no keys are found in root node
			{
			result=treeSearch(iH->page0,khead);
			returnFlag=3; //specifies to return the result
			goto TSEND;
			}
		iKPP=(IndexKeyPagePair*)( (char*)nodeData + 2*sizeof(short)+1*sizeof(long));
		if(keyType_C==IKEY || keyType_C==NONKEY)
			result=com_C->compareLEG(khead,iKPP->key);
		else
			result=com_C->compareLEG(khead,iKPP->key,1);	
		if(result == CL)  //if khead < = the first key
			{
			result=treeSearch(iH->page0,khead);
			returnFlag=3; //specifies to return the result
			goto TSEND;
			}
		else if(result == CE)
			{
			result=treeSearch(iKPP->pageNumber,khead);
			returnFlag=3; //specifies to return the result
			goto TSEND;
			}	
		else                              // if khead > last key else binary search
			{
			tempIKPP=iKPP;
			iKPP=(IndexKeyPagePair*)((char*)nodeData+8+(iH->noe-1)*(4+keySize_C));
			if(keyType_C==IKEY || keyType_C==NONKEY)
				result=com_C->compareLEG(khead,iKPP->key);
			else	
				result=com_C->compareLEG(khead,iKPP->key,1);
			if(result == CG || result ==CE)         // if khead > last key
				{
				result=treeSearch(iKPP->pageNumber,khead);
				returnFlag=3; //specifies to return the result
				goto TSEND;
				}	
			else
				{
				if(iH->noe==2)
					{
					returnFlag=7;
					goto TSEND;
					}
				indexx=binarySearchIndex(nodeData,khead,1,iH->noe-1);
				if(equalityFlag_C==1)	equalityFlag_C=0;
				iKPP=(IndexKeyPagePair*)( (char*)nodeData + 2*sizeof(short)+1*sizeof(long)+(indexx-1)*(4+keySize_C));
				result=treeSearch(iKPP->pageNumber,khead);
				returnFlag=3; //specifies to return the result
				goto TSEND;
				}	
			}	
		}	
		
TSEND:		
	if(nodeData!=0)			MSF::myFree(nodeData);
	if(returnFlag==1)		return pN;
	else if(returnFlag==2)		return NEWINDEX;		
	else if(returnFlag==3)		return result;	
	else if(returnFlag==7)		return treeSearch(tempIKPP->pageNumber,khead);
	else              		return FAILED;	
	
	}	
	
int VIndexManager :: treeSearchFI(long pN,KeyList *khead) // can be called from insert as well
	{
	int returnFlag=0,i=0,indexx=0;
	long result=0;
	void *nodeData=0;
	nodeData=MSF::myMalloc(PAGESIZE);
	vSB_C->readFromBufferPool(pN,nodeData);
	IndexHeader *iH;
	iH=(IndexHeader*)nodeData;
	IndexKeyPagePair *iKPP=0,*tempIKPP=0;
	if(iH->level==0) //if leaf node return the pagenumber of leaf
		{
		returnFlag=1; //1 means return the pN
		goto TSEND;
		}
	else            //non leaf i.e.., index pages so proceed
		{
		if(iH->noe==0)                              //no keys are found in root node
			{
			result=iH->page0;//treeSearchFI(iH->page0,khead);
			returnFlag=3; //specifies to return the result
			goto TSEND;
			}
		iKPP=(IndexKeyPagePair*)( (char*)nodeData + 2*sizeof(short)+1*sizeof(long));
		if(keyType_C==IKEY || keyType_C==NONKEY)
			result=com_C->compareLEG(khead,iKPP->key);
		else
			result=com_C->compareLEG(khead,iKPP->key,1);	
		if(result == CL)  //if khead < the first key
			{
			result=iH->page0;//treeSearchFI(iH->page0,khead);
			returnFlag=3; //specifies to return the result
			goto TSEND;
			}
		else if( result == CE)	
			{
			result=iKPP->pageNumber;//treeSearchFI(iH->page0,khead);
			returnFlag=3; //specifies to return the result
			goto TSEND;
			}
		else                              // if khead > last key else binary search
			{
			tempIKPP=iKPP;
			iKPP=(IndexKeyPagePair*)((char*)nodeData+8+(iH->noe-1)*(4+keySize_C));
			if(keyType_C==IKEY || keyType_C==NONKEY)
				result=com_C->compareLEG(khead,iKPP->key);
			else
				result=com_C->compareLEG(khead,iKPP->key,1);	
			if(result == CG || result == CE )         // if khead > last key
				{
				result=iKPP->pageNumber;//treeSearchFI(iKPP->pageNumber,khead);
				returnFlag=3; //specifies to return the result
				goto TSEND;
				}	
			else
				{//key is less than the last one so binary search from 
				if(iH->noe==2) //if noe ==2 means so no more keys left so return the left pointer
					{
					result=tempIKPP->pageNumber;
					returnFlag=3;
					goto TSEND;
					}
				indexx=binarySearchIndex(nodeData,khead,1,iH->noe-1);
				if(equalityFlag_C==1)	equalityFlag_C=0;
				iKPP=(IndexKeyPagePair*)( (char*)nodeData + 2*sizeof(short)+1*sizeof(long)+(indexx-1)*(4+keySize_C));
				result=iKPP->pageNumber;
				returnFlag=3; //specifies to return the result
				goto TSEND;
				}	
			}	
		}	
		
TSEND:		
	if(nodeData!=0)			MSF::myFree(nodeData);
	if(returnFlag==1)		return pN;
	else if(returnFlag==2)		return NEWINDEX;		
	else if(returnFlag==3)		return result;	
	else              		return FAILED;	
	}		

int VIndexManager :: binarySearchIndex(void *data,KeyList *khead,int low,int high) // to be tested and integrated with treeSearch module just above
	{
	IndexKeyPagePair *iPMid=0;
	int comp=0,mid=0;
	if(low > high) 
		return high;
	mid=(low+high)/2;
	iPMid=(IndexKeyPagePair*)((char*)data+8+(mid-1)*(4+keySize_C));
	if(keyType_C==IKEY || keyType_C==NONKEY)		//printf("\n HERE INDEX");
		comp=com_C->compareLEG(khead,iPMid->key);
	else
		comp=com_C->compareLEG(khead,iPMid->key,1);	
	if(comp==CE)
		{
		equalityFlag_C=1;
		return mid;
		}
	else if(comp==CL)
		return(binarySearchIndex(data,khead,low,mid-1));
	else if(comp==CG) 
		return(binarySearchIndex(data,khead,mid+1,high));		
	}	

short VIndexManager :: binarySearchLeaf(void *data,KeyList *khead,int low,int high) // low 1 to high n
	{
	LeafKeyRIDPair *lPMid=0;
	int comp=0,mid=0,i=0; // to store the comparision of key and the value
	if(low>high)
		return high;
	mid=(low+high)/2;
	lPMid=(LeafKeyRIDPair*)((char*)data+12+(mid-1)*(6+keySize_C));
	if(keyType_C==IKEY || keyType_C==NONKEY) //printf("\nHERE LEAF");
		comp=com_C->compareLEG(khead,lPMid->key); //returns either CL or CE or CG			
	else
		comp=com_C->compareLEG(khead,lPMid->key,1);	
	if(comp==CE)
		{
		equalityFlag_C=1;
		return mid;
		}
	else if(comp==CL)
		return(binarySearchLeaf(data,khead,low,mid-1));			
	else if(comp==CG)
		return(binarySearchLeaf(data,khead,mid+1,high));
	}

int  VIndexManager :: leafCopy(void *data,void *tempData,KeyList *khead,RID *rid_P,short noe,long prevPage,long nextPage)
	{
	int returnFlag=0;
	int index=0;
	LeafHeader *lH=0;
	LeafKeyRIDPair *lKRP=0;
	lH=(LeafHeader*)MSF::myMalloc(2*sizeof(long)+2*sizeof(short));
	lH->level=0;	        lH->noe=noe+1;  //noe+1, considering the current entry as well
	lH->prevPage=prevPage;	lH->nextPage=nextPage;
	lKRP=(LeafKeyRIDPair*)MSF::myMalloc(1*sizeof(long)+1*sizeof(short)+keySize_C);
	(lKRP->rID).pageNumber=rid_P->pageNumber;	(lKRP->rID).slotID=rid_P->slotID;
	KeyList *tempList=khead;
	if(nof_C==1)
		strcpy(lKRP->key,khead->value);
	else
		{
		while(khead->next!=NULL)
			{
			if(index==0)
				{
				strcpy(lKRP->key,khead->value);
				index++;
				}
			else	
				strcat(lKRP->key,khead->value);
			strcat(lKRP->key,"#");
			khead=khead->next;
			}
		strcat(lKRP->key,khead->value);	
		}index=0;	
	khead=tempList;		
	memcpy(tempData,lH,2*sizeof(long)+2*sizeof(short)); 
	index=binarySearchLeaf(data,khead,1,noe); //finding the right place for the new key
	if(equalityFlag_C==1)	
		{
		equalityFlag_C=0;
		//printf("\n EQULA");
		if(keyType_C==COMPOSITEKEY || keyType_C==IKEY)
			{
			returnFlag=1;
			goto LCEND;
			}
		}
		
	memcpy((char*)tempData + (2*sizeof(long)+2*sizeof(short)),(char*)data + (2*sizeof(long)+2*sizeof(short)),
			   						index * (1*sizeof(long)+1*sizeof(short)+keySize_C));
	memcpy((char*)tempData + (2*sizeof(long)+2*sizeof(short)) +     index * (1*sizeof(long)+1*sizeof(short)+keySize_C),lKRP,
	                                                                        (1*sizeof(long)+1*sizeof(short)+keySize_C));
	if(index<noe)
		memcpy((char*)tempData + (2*sizeof(long)+2*sizeof(short)) + (index+1) * (1*sizeof(long)+1*sizeof(short)+keySize_C),
			       (char*)data + (2*sizeof(long)+2*sizeof(short)) + index * (1*sizeof(long)+1*sizeof(short)+keySize_C),
			   					          (noe-index) * (1*sizeof(long)+1*sizeof(short)+keySize_C));				
LCEND:									  
	if(lH!=0)		MSF::myFree(lH);
	if(lKRP!=0)		MSF::myFree(lKRP);
	if(returnFlag==1)	return FAILED;
	else			return SUCCESS;
	}

void VIndexManager :: indexCopy(void *data,void *tempData,KeyList *khead,RID *rid_P,short level,short noe,long page0)
	{
	//printf("\n\t In Index COPY");
	int index=0,i=0;
	IndexHeader *cIH=0;
	IndexKeyPagePair *cIKPP=0;
	cIH=(IndexHeader*)MSF::myMalloc(2*sizeof(short)+1*sizeof(long));
	cIH->level=level;	cIH->noe=noe+1;	cIH->page0=page0;
	cIKPP=(IndexKeyPagePair*)MSF::myMalloc(1*sizeof(long)+keySize_C);
	cIKPP->pageNumber=sPN_C;//got to get as parameter, work for that
	KeyList *ktemp=khead;
	if(nof_C==1)
		strcpy(cIKPP->key,khead->value);
	else
		{
		while(khead->next!=NULL)
			{
			if(index==0)
				{
				strcpy(cIKPP->key,khead->value);
				index++;
				}
			else	
				strcat(cIKPP->key,khead->value);
			strcat(cIKPP->key,"#");
			khead=khead->next;
			}
		strcat(cIKPP->key,khead->value);	
		}index=0;
	memcpy(tempData,cIH,1*sizeof(long)+2*sizeof(short));	
	if(noe!=0)
		{
		index=binarySearchIndex(data,ktemp,1,noe);// got to do
		if(equalityFlag_C==1) equalityFlag_C=0;
		}
	#ifdef VIMDEBUG	
	printf("\n%d is the index",index);	
	#endif
	memcpy((char*)tempData + (1*sizeof(long)+2*sizeof(short)), (char*)data + (1*sizeof(long)+2*sizeof(short)),
			   						            index*(1*sizeof(long)+keySize_C));
	memcpy((char*)tempData + (1*sizeof(long)+2*sizeof(short)) + index * (1*sizeof(long)+keySize_C),cIKPP,                                                                                                                          (1*sizeof(long)+keySize_C));
	if(index<noe)
		memcpy((char*)tempData + (1*sizeof(long)+2*sizeof(short)) + (index+1) * (1*sizeof(long)+keySize_C),
			       (char*)data + (1*sizeof(long)+2*sizeof(short)) + index * (1*sizeof(long)+keySize_C),
			   					           (noe-index)* (1*sizeof(long)+keySize_C));					
	if(cIH!=0)	MSF::myFree(cIH);
	if(cIKPP!=0)	MSF::myFree(cIKPP);
	//printf("\n\t In Index COPY Done");
	}

void VIndexManager :: addToListONBREAKING(KeyList **destination,char *key)
	{
	#ifdef VIMDEBUG
	printf("\nVenkiiiiiii %s key %d",key,indexPN_C);
	printf("\nVenkiiiiiii");
	#endif
	int i=0,j=0,nof=0;
	void *iHData=0;
	char *temp=0;

	temp=(char*)MSF::myMalloc(strlen(key));
	iHData=MSF::myMalloc(PAGESIZE);
	
	vSB_C->readFromBufferPool(indexPN_C,iHData);
	
	Fields *fds=0;
	for(i=0;i<strlen(key);i++)
		{
		if(key[i]!='#')	temp[j++]=key[i];
		if(key[i]=='#') 
			{
			fds=(Fields*)((char*)iHData+8*sizeof(short)+5*sizeof(long)+(nof)*(2*sizeof(short)+MAXFIELDNAME));nof++;
			temp[j]='\0';j=0;
			addToKeyList(destination,fds->fieldType,temp);
			//printf("\n\t%s is added %d",temp,fds->fieldType);
			}
		//printf("\n iii %c",key[i]);	
		}
	temp[j]='\0';
	//printf("\n %s is temp",temp);
	fds=(Fields*)((char*)iHData+8*sizeof(short)+5*sizeof(long)+(nof)*(2*sizeof(short)+MAXFIELDNAME));
	addToKeyList(destination,fds->fieldType,temp);
	#ifdef VIMDEBUG
	displayKeyList(*destination);
	#endif
	MSF::myFree(temp);
	MSF::myFree(iHData);
	}			
		
int VIndexManager :: treeInsert(long nodePointer,KeyList **entry,RID *rentry)
	{
//printf("\n IN TEMP INTERFACE");
	int returnFlag=SUCCESS;
	int result=0,i=0;
	void *data=0,*tempData=0,*dataSplit1=0,*dataSplit2=0,*nextLeaf=0;
	void *rootNode=0;
	data=MSF::myMalloc(PAGESIZE); //printf("\ninsertion 6");
	vSB_C->readFromBufferPool(nodePointer,data);
	IndexHeader *iH=(IndexHeader*)data;
	IndexHeader *sIH=0;
	IndexHeader *rIH=0; // for root node creation
	IndexKeyPagePair *sIKPP=0;
	IndexKeyPagePair *rIKPP=0;
	LeafHeader *lH=0;
	LeafHeader *sLH=0;        //For splitting purpose
	LeafHeader *sILH=0;
	LeafKeyRIDPair *sLKRP=0;
	long tPN=0;
	short sLF1=0,sLF2=0,sValueType=0;
	KeyList *tempList;
//NONLEAF
	if(iH->level!=0) 		 //NODE IS INTERNAL NODE i.e.., NON LEAF NODE
		{
		result=treeSearchFI(nodePointer,*entry);
		returnFlag=treeInsert(result,entry,rentry);
		if(returnFlag==FAILED)
			goto TIEND;
		if((*entry)==NULL)                     //ENTRY MADE NULL MEANS NO SPLIT OCCURED IN THE LEAF
			{ }
		else                                   //ENTRY IS NOT NULL MEANS A SPLIT OCCURED IN THE LEAF
			{	
			// ................./////////////@@@@@@@@############************Leftttttttttttttttttttttttttt
			if(iH->noe < iFanout_C)        //INDEX NODE HAS SPACE TO ACCOMODATE THE SPLIT VALUE ENTRY
				{
				//printf("\n IN NODE NON-SPLIT");
				tempData=MSF::myMalloc(PAGESIZE); //printf("\ninsertion 7");
				indexCopy(data,tempData,*entry,rentry,iH->level,iH->noe,iH->page0);
				vSB_C->writeToBufferPool(nodePointer,tempData);
				freeKeyList(*entry);
				*entry=NULL;
				//printf("\n IN NODE NON-SPLIT DONE");
				}
			else                           //INDEX NODE HAS NO SPACE TO ACCOMODATE THE SPLIT VALUE,SO SPLIT AND PROCEED
				{
				//printf("\n IN NODE SPLIT");
				tempData=MSF::myMalloc(PAGESIZE+4+keySize_C); //printf("\ninsertion 8");
				indexCopy(data,tempData,*entry,rentry,iH->level,iH->noe,iH->page0); //sPN_C replicated in the leaf split is used here 
				sLF1=(iFanout_C+1)/2;
				sLF2=(iFanout_C+1) - ((iFanout_C+1)/2);	
				dataSplit1=MSF::myMalloc(PAGESIZE); //printf("\ninsertion 9");
				dataSplit2=MSF::myMalloc(PAGESIZE); //printf("\ninsertion 10");
				sPN_C=fLH_C->getFreePage();         //sPN_C is reassigned as the previous value is already used
				sIH=(IndexHeader*)MSF::myMalloc(2*sizeof(short)+1*sizeof(long));
				sIH->level=iH->level;	sIH->noe=sLF1;	sIH->page0=iH->page0;
				memcpy(dataSplit1,sIH,1*sizeof(long)+2*sizeof(short));
				memcpy((char*)dataSplit1 + (1*sizeof(long)+2*sizeof(short)), (char*)tempData + (1*sizeof(long)+2*sizeof(short)),
			  		                					         sLF1 * (1*sizeof(long)+keySize_C));
				sIH->noe=sLF2; sIH->page0=-1;
				memcpy(dataSplit2,sIH,1*sizeof(long)+2*sizeof(short));
				memcpy((char*)dataSplit2 + (1*sizeof(long)+2*sizeof(short)),
			  		 (char*)tempData + (1*sizeof(long)+2*sizeof(short)) + sLF1 * (1*sizeof(long)+keySize_C),
			  	                					      sLF2 * (1*sizeof(long)+keySize_C));
				vSB_C->writeToBufferPool(nodePointer,dataSplit1);
				vSB_C->writeToBufferPool(sPN_C,dataSplit2);
				sIKPP=(IndexKeyPagePair*)((char*)dataSplit2+8);
				sValueType=(*entry)->type;
				VIndexManager :: freeKeyList(*entry);
				*entry=NULL;
				#ifdef VIMDEBUG
				printf("\n In Second Level indexer");
				#endif
				if(keyType_C==IKEY)//major bug cleared
					VIndexManager :: addToKeyList(entry,sValueType,sIKPP->key);
				else
					VIndexManager :: addToListONBREAKING(entry,sIKPP->key);
				#ifdef VIMDEBUG	
				displayKeyList(*entry);	
				#endif
				if(nodePointer==rootPN_C)
					{
					tPN=fLH_C->getFreePage();
					rootNode=MSF::myMalloc(PAGESIZE); //printf("\ninsertion 11");
					rIH=(IndexHeader*)MSF::myMalloc(1*sizeof(long)+2*sizeof(short));
					rIH->level=iH->level+1;		rIH->noe=1;	rIH->page0=nodePointer;
					rIKPP=(IndexKeyPagePair*)MSF::myMalloc(4+keySize_C);
					rIKPP->pageNumber=sPN_C;
					tempList=*entry;
					if(nof_C==1)
						strcpy(rIKPP->key,tempList->value);
					else
						{
						while(tempList->next!=NULL)
							{
							if(i==0)
								{
								strcpy(rIKPP->key,tempList->value);
								i++;
								}
							else	
								strcat(rIKPP->key,tempList->value);
							strcat(rIKPP->key,"#");
							tempList=tempList->next;
							}
						strcat(rIKPP->key,tempList->value);	
						}i=0;
					memcpy(rootNode, rIH,  2*sizeof(short)+1*sizeof(long));
					memcpy((char*)rootNode+2*sizeof(short)+1*sizeof(long),rIKPP,4+keySize_C);
					vSB_C->writeToBufferPool(tPN,rootNode);
					rootPN_C=tPN;
					nol_C+=1;
					nop_C+=1;
					freeKeyList(*entry);
					}
				nop_C+=1;			
				updateIndexHeader();		
				}	
			}
		}	
/*LEAF*/else                             //NODE IS OF TYPE LEAF
		{
		lH=(LeafHeader*)data;
		if(lH->noe < lFanout_C)                //LEAF HAS SPACE SO PLACE THE NEW ENTRY HERE
			{
			//printf("\nIn Leaf Non-split");
			tempData=MSF::myMalloc(PAGESIZE); //printf("\ninsertion 12");
			returnFlag=leafCopy(data,tempData,*entry,rentry,lH->noe,lH->prevPage,lH->nextPage);//returns FAILED for duplicate entry for Key
			if(returnFlag!=FAILED)		
				{
				vSB_C->writeToBufferPool(nodePointer,tempData);
				nok_C+=1;
				updateIndexHeader();
				}
			freeKeyList(*entry);
			*entry=NULL;			
/*FAILED*/		}
		else                                   //LEAF HAS NO SPACE SO SPLIT AND INTIMATE ITS PARENT ABOUT THIS
			{
			tempData=MSF::myMalloc(PAGESIZE+6+keySize_C); //printf("\ninsertion 13");
			returnFlag=leafCopy(data,tempData,*entry,rentry,lH->noe,lH->prevPage,lH->nextPage);
/*FAILED*/		if(returnFlag==FAILED)		
				{
				freeKeyList(*entry);
				*entry=NULL;
				goto TIEND;
				}
			sLF1=(lFanout_C+1)/2;
			sLF2=(lFanout_C+1)-((lFanout_C+1)/2);
			dataSplit1=MSF::myMalloc(PAGESIZE); //printf("\ninsertion 14");
			dataSplit2=MSF::myMalloc(PAGESIZE); //printf("\ninsertion 15");
			sPN_C=fLH_C->getFreePage(); //getting a new page for new leaf page
			sLH=(LeafHeader*)MSF::myMalloc(2*sizeof(long)+2*sizeof(short)); //previously a graceful bug, allocated PAGESIZE data
			sLH->level=0;	sLH->noe=sLF1; //printf("\ninsertion 16");
			sLH->prevPage=lH->prevPage;	sLH->nextPage=sPN_C; // setting the next page to new leaf page
			memcpy(dataSplit1,sLH,2*sizeof(long)+2*sizeof(short));
			sLH->noe=sLF2;
			sLH->prevPage=nodePointer;	sLH->nextPage=lH->nextPage;
			memcpy(dataSplit2,sLH,2*sizeof(long)+2*sizeof(short));
			memcpy((char*)dataSplit1 + (2*sizeof(long)+2*sizeof(short)), (char*)tempData + (2*sizeof(long)+2*sizeof(short)),
				                					sLF1*(1*sizeof(long)+1*sizeof(short)+keySize_C));
			memcpy((char*)dataSplit2 + (2*sizeof(long)+2*sizeof(short)),
			         (char*)tempData + (2*sizeof(long)+2*sizeof(short))+ sLF1 * (1*sizeof(long)+1*sizeof(short)+keySize_C),
			  		               			             sLF2 * (1*sizeof(long)+1*sizeof(short)+keySize_C));
			vSB_C->writeToBufferPool(nodePointer,dataSplit1);
			vSB_C->writeToBufferPool(sPN_C,dataSplit2);
			
			if(lH->nextPage==-1)	{}
			else
				{
				nextLeaf=MSF::myMalloc(PAGESIZE); //printf("\ninsertion 17");
				vSB_C->readFromBufferPool(lH->nextPage,nextLeaf);
				sILH=(LeafHeader*)nextLeaf;
				sILH->prevPage=sPN_C;
				vSB_C->writeToBufferPool(lH->nextPage,nextLeaf);
				}
			sLKRP=(LeafKeyRIDPair*)((char*)dataSplit2+12);
			sValueType=(*entry)->type;
			VIndexManager :: freeKeyList(*entry);
			*entry=NULL;
			#ifdef VIMDEBUG
			printf("\n%s %d",sLKRP->key,indexPN_C);
			printf("\nVENKI");
			#endif
			if(keyType_C==IKEY) //major bug cleared
				VIndexManager :: addToKeyList(entry,sValueType,sLKRP->key);
			else
				VIndexManager :: addToListONBREAKING(entry,sLKRP->key);
			rentry->slotID=(sLKRP->rID).slotID;rentry->pageNumber=(sLKRP->rID).pageNumber;
			nop_C+=1;
			nok_C+=1;
			updateIndexHeader();
			//printf("\nIn LeafSplit Done");
			}	
		}	
TIEND:				
	if(data!=0)		MSF::myFree(data);
	if(tempData!=0)		MSF::myFree(tempData);	
	if(dataSplit1!=0)	MSF::myFree(dataSplit1);
	if(dataSplit2!=0)	MSF::myFree(dataSplit2);	
	if(sLH!=0)		MSF::myFree(sLH);
	if(nextLeaf!=0)		MSF::myFree(nextLeaf);
	if(sIH!=0)		MSF::myFree(sIH);
	if(rootNode!=0)		MSF::myFree(rootNode);
	if(rIH!=0)		MSF::myFree(rIH);
	if(rIKPP!=0)		MSF::myFree(rIKPP);
	return returnFlag;
	}
		
//***********************Actual functionality of INDEX MANAGER***********************************************//
int VIndexManager :: insert(long indexPN,KeyList *khead,RID *rid_P)
	{
#ifdef VERBOSEMODE
	printf("\nInside insert method of VIndexManager class");	
#endif	
	int returnFlag=0,i=0,j=0;
	void *iHData=0,*data=0,*iData=0;
	iHData=MSF::myMalloc(PAGESIZE); 
	#ifdef VIMDEBUG
	printf("\ninsertion 1");
	#endif
	vSB_C->readFromBufferPool(indexPN,iHData);	  
	IMHeaderPage *imph;
	imph=(IMHeaderPage*)iHData;	
	Fields *fds;
	fds=(Fields*)((char*)iHData + 5*sizeof(long) +8*sizeof(short));	
	KeyList *temp=khead;
	iFanout_C=imph->iFanout;
	lFanout_C=imph->lFanout;
	keyType_C=imph->keyType;
	keySize_C=imph->keySize; // -1 because already 1 is in the size of either IndexKeyPagePair or LeafKeyRIDPair
	rootPN_C =imph->rootPN;
	nof_C=imph->nof;	
	indexPN_C=indexPN;
	leafPN_C =imph->leafPN;
	nop_C    =imph->nop;
	nol_C    =imph->nol;
	nok_C    =imph->nok;
	mIK_C    =imph->mIK; 
	duplicateFlag_C=-10;
	LeafHeader *lH=0; 
	LeafKeyRIDPair *lKRP=0; 
	IndexHeader *iH=0;
	int number=imph->nof;
	KeyList *tempList=NULL;
	char *cptemp=0;
	int xx=0;
	while(number>0 && temp!=NULL)  //step 1 typechecking of the keys if success process contines else it terminates without inserting
		{
		fds=(Fields*)((char*)iHData+8*sizeof(short)+5*sizeof(long)+i*(2*sizeof(short)+MAXFIELDNAME));	
		if(fds->fieldType!=temp->type)
			{
			returnFlag=1;
			goto IEND;
			}		
		if(number==imph->nof && (keyType_C==COMPOSITENONKEY || keyType_C==NONKEY) )
			{
			i=i+2;
			number-=2;
			}			
		number--;
		i++;
		temp=temp->next;
		}					
	//if the tree is a new one then insert it directly here
	iData=MSF::myMalloc(PAGESIZE); //printf("\ninsertion 2");
	vSB_C->readFromBufferPool(imph->rootPN,iData);
	iH=(IndexHeader*)iData;
	if(iH->page0==-1) //special case for new index
		{
		imph->leafPN=fLH_C->getFreePage();
		data=MSF::myMalloc(PAGESIZE); //printf("\ninsertion 3");
		lH=(LeafHeader*)MSF::myMalloc(2*sizeof(long)+2*sizeof(short)); //printf("\ninsertion 4");
		lH->level=0;	        lH->noe=1;
		lH->prevPage=-1;	lH->nextPage=-1;
		lKRP=(LeafKeyRIDPair*)MSF::myMalloc(1*sizeof(long)+1*sizeof(short)+keySize_C);  //printf("\ninsertion 5");
		(lKRP->rID).pageNumber=rid_P->pageNumber;(lKRP->rID).slotID=rid_P->slotID;
		if(imph->nof==1)
			{
			strcpy(lKRP->key,khead->value);
			#ifdef VIMDEBUG
			printf("\nHERE %s %s",lKRP->key,khead->value);
			#endif
			}
		else
			{
			cptemp=(char*)MSF::myMalloc(keySize_C);
			while(khead!=NULL)
				{
				if(j==0)
					{
					strcpy(lKRP->key,khead->value);
					j++;
					}
				else
					strcat(lKRP->key,khead->value);
				if(0==xx && ( keyType_C==COMPOSITENONKEY || keyType_C==NONKEY))
					{
					strcat(lKRP->key,"#");
					#ifdef VIMDEBUG
					//printf("\n in here");
					#endif
					cptemp=MSF::itoa(rid_P->pageNumber,cptemp);
					strcat(lKRP->key,cptemp);
					strcat(lKRP->key,"#");
					cptemp=MSF::itoa(rid_P->slotID,cptemp);
					strcat(lKRP->key,cptemp);
					}
				if(keyType_C!=IKEY);	
					if(khead->next!=NULL)	
						strcat(lKRP->key,"#");	
				xx++;	
				khead=khead->next;
				}
			}	
		#ifdef VIMDEBUG	
		printf("\n%s",lKRP->key);	
		#endif
		memcpy(data,lH,2*sizeof(long)+2*sizeof(short));
		memcpy((char*)data+2*sizeof(long)+2*sizeof(short),lKRP,1*sizeof(long)+1*sizeof(short)+keySize_C);
		vSB_C->writeToBufferPool(imph->leafPN,data);		
		iH->page0=imph->leafPN;
		vSB_C->writeToBufferPool(imph->rootPN,iData);
		vSB_C->writeToBufferPool(indexPN,iHData);
		leafPN_C=imph->leafPN;
		nop_C+=1;
		nok_C+=1;
		updateIndexHeader();
		returnFlag=2;
		}	
IEND:	 
	if(lH!=0)		MSF::myFree(lH);
	if(lKRP!=0)		MSF::myFree(lKRP);	
	if(iData!=0)		MSF::myFree(iData);
	if(iHData!=0)		MSF::myFree(iHData);
	if(data!=0)		MSF::myFree(data);
	if(cptemp!=0)		MSF::myFree(cptemp);	
	if(returnFlag==1)
		return FAILED;
	else if(returnFlag==2)
		return SUCCESS;	
	else 
		{
//printf("\n\t IN INSERT INTERFACE");
		if(keyType_C==COMPOSITENONKEY || keyType_C==NONKEY)	
			{
			copyKeyList(khead,&tempList,rid_P);
			duplicateFlag_C=keyType_C;
			keyType_C=COMPOSITEKEY;
			}
		else	
			copyKeyList(khead,&tempList); //for KEY and COMPOSITEKEY no need of RID stuffing
		#ifdef VIMDEBUG	
		displayKeyList(tempList);	
		#endif
		return treeInsert(rootPN_C,&tempList,rid_P); //actual insertion of keys into an index with more than one key starts here
		}
	}

int VIndexManager :: treeDeleteEntry(long parentPointer,long nodePointer,KeyList **entry,RID *rentry)
	{
	#ifdef VIMDEBUG
	printf("\nIn Tree Delete Entry %d is parent Pointer, %d is the node Pointer",parentPointer,nodePointer); 
	#endif
	void *data=0,*tempData=0,*copyData=0,*sourceData=0;
	int result=0,returnFlag=0;
	long pgNum=0;
	data=MSF::myMalloc(PAGESIZE);
	vSB_C->readFromBufferPool(nodePointer,data);
	IndexHeader *iH=(IndexHeader*)data;
	IndexHeader *dIH=0,*dDIH=0;
	LeafHeader *lH=0;
	LeafHeader *dLH=0; //for allocation purpose
	LeafHeader *tLH=0;
	short flag=0;     //used for duplicates handling
	if(iH->level!=0) //NODE IS A NON LEAF NODE i.e.., INTERNAL or INDEX NODE
		{
		result=treeSearchFI(nodePointer,*entry);
		#ifdef VIMDEBUG
		printf("\n\tBack to DeleteEntry From Search ## %d is the result",result);
		#endif
		result=treeDeleteEntry(nodePointer,result,entry,rentry);
		if(result==FAILED)	
			{
			returnFlag=5;
			goto TDEND;
			}
		if((*entry)==NULL)                     //USUAL CASE CHILD NOT DELETED
			{
			#ifdef VIMDEBUG
			printf("\n########### Entry Made NULL, so no split occured ");
			#endif
			goto TDEND;
			}
		else 				       //WE DISCARDED CHILD NODE
			{
			#ifdef VIMDEBUG
			printf("\n*************Entry NOT Made NULL, so split occured %s is the key to be removed",(*entry)->value);
			#endif
			if(iH->noe==1 && nodePointer!=rootPN_C && iH->page0==-1)
				{
				fLH_C->addToFL(nodePointer);
				goto TDEND;
				}
			else		//for rootNode and 
				{	
				result=binarySearchIndex(data,*entry,1,iH->noe);
				if(equalityFlag_C==1) equalityFlag_C=0;
				#ifdef VIMDEBUG
				printf("\n%s is at or less result %d",(*entry)->value,result);
				#endif
				dIH=(IndexHeader*)MSF::myMalloc(1*sizeof(long)+2*sizeof(short));
				dIH->level=iH->level;	dIH->page0=iH->page0;
				dIH->noe=iH->noe-1;
				sourceData=MSF::myMalloc(PAGESIZE);
				memcpy(sourceData, dIH     ,1*sizeof(long)+2*sizeof(short));
				memcpy((char*)sourceData + (1*sizeof(long)+2*sizeof(short)), 
				       (char*)data + (1*sizeof(long)+2*sizeof(short)),
			   						            (result-1)*(1*sizeof(long)+keySize_C));
				if(result < iH->noe)
				memcpy((char*)sourceData + (1*sizeof(long)+2*sizeof(short)) + (result-1) * (1*sizeof(long)+keySize_C),
			       	       (char*)data     + (1*sizeof(long)+2*sizeof(short)) +  result    * (1*sizeof(long)+keySize_C),
			   					                    (iH->noe - result) * (1*sizeof(long)+keySize_C));	
				vSB_C->writeToBufferPool(nodePointer,sourceData);
				#ifdef VIMDEBUG
				displayNode(nodePointer);
				#endif
				freeKeyList(*entry);
				*entry=NULL;
				goto TDEND;					
				}			
			}	
		}	
	else		//NODE IS A LEAF NODE
		{
		lH=(LeafHeader*)data;
#ifdef VIMDEBUG		
printf("\n\tI am here %d is lfanout %d is iFanout %d is keySize, %d is nof,%d is noe in leaf at %d",lFanout_C,iFanout_C,keySize_C,nof_C,lH->noe,nodePointer);
#endif
		result=binarySearchLeaf(data,*entry,1,lH->noe);
		if(equalityFlag_C!=1)   
			{
			#ifdef VIMDEBUG
			printf("\nNOT FOUND THE ENTRY AT %d LOCATION IN %d LEAF\n",result,nodePointer);
			#endif
			returnFlag=5;
			goto TDEND;
			}
		else
			{	
			#ifdef VIMDEBUG
			printf("\nFOUND THE ENTRY AT %d LOCATION IN %d LEAF\n",result,nodePointer);
			#endif
			equalityFlag_C=0;
			}
		tempData=MSF::myMalloc(PAGESIZE); //tempData to retrieve index entry
		vSB_C->readFromBufferPool(parentPointer,tempData);
		dDIH=(IndexHeader*)tempData;
		#ifdef VIMDEBUG
		printf("\nLeaf lo checking INDEX at %d :  %d is the level,%d is noe,%d is page0",parentPointer,dDIH->level,dDIH->noe,dDIH->page0);
		#endif
		if(parentPointer==rootPN_C && dDIH->noe==0)//root node and so dont drop the leaf page 
			{
			dLH=(LeafHeader*)MSF::myMalloc(2*sizeof(long)+2*sizeof(short));
			dLH->noe=lH->noe-1;		dLH->level=lH->level;
			dLH->prevPage=lH->prevPage;	dLH->nextPage=lH->nextPage;
			copyData=MSF::myMalloc(PAGESIZE);
			memcpy(copyData,dLH,      2*sizeof(long)+2*sizeof(short));			
			memcpy((char*)copyData + (2*sizeof(long)+2*sizeof(short)),
			   		 (char*)data   + (2*sizeof(long)+2*sizeof(short)),(result-1)*(1*sizeof(long)+1*sizeof(short)+keySize_C));
			if((result-1)<dLH->noe)
				memcpy((char*)copyData + (2*sizeof(long)+2*sizeof(short))+(result-1)*(1*sizeof(long)+1*sizeof(short)+keySize_C),
			 		(char*)data   + (2*sizeof(long)+2*sizeof(short))+(result  )*(1*sizeof(long)+1*sizeof(short)+keySize_C),
			      	                               (dLH->noe-(result-1))*(1*sizeof(long)+1*sizeof(short)+keySize_C));
			vSB_C->writeToBufferPool(nodePointer,copyData);
			nok_C-=1;
			freeKeyList(*entry);
			*entry=NULL;
			goto TDEND;	 	
			}
		else	// may be the root page but there are entries in the index
			{
			if(lH->noe==1)
				{
				dLH=(LeafHeader*)MSF::myMalloc(2*sizeof(long)+2*sizeof(short));
				sourceData=MSF::myMalloc(PAGESIZE);
				if(lH->prevPage==-1)
					{//means the left most leaf node, dont delete it
					dLH->level=lH->level;						dLH->prevPage=lH->prevPage;
					dLH->nextPage=lH->nextPage;/*setting the next page value*/	dLH->noe=lH->noe-1;
					memcpy(sourceData,dLH,(2*sizeof(long)+2*sizeof(short)));
					vSB_C->writeToBufferPool(nodePointer,sourceData);
					nok_C-=1;
					freeKeyList(*entry);
					*entry=NULL;
					goto TDEND;
					}
				else
					{	
					vSB_C->readFromBufferPool(lH->prevPage,sourceData);
					#ifdef VIMDEBUG
					printf("\n Retrieving the prev page %d",lH->prevPage);
					#endif
					tLH=(LeafHeader*)sourceData;
					dLH->level=tLH->level;						dLH->prevPage=tLH->prevPage;
					dLH->nextPage=lH->nextPage;/*setting the next page value*/	dLH->noe=tLH->noe;
					memcpy(sourceData,dLH,(2*sizeof(long)+2*sizeof(short)));
					#ifdef VIMDEBUG
					printf("\n New values level %d  noe %d  p %d  n %d",dLH->level,dLH->noe,dLH->prevPage,dLH->nextPage);
					#endif
					vSB_C->writeToBufferPool(lH->prevPage,sourceData);
					if(lH->nextPage!=-1)
						{
						vSB_C->readFromBufferPool(lH->nextPage,sourceData);
						#ifdef VIMDEBUG
						printf("\n Retrieving the next page %d",lH->nextPage);
						#endif
						tLH=(LeafHeader*)sourceData;
						dLH->level=tLH->level;			dLH->noe=tLH->noe;
						dLH->nextPage=tLH->nextPage;		dLH->prevPage=lH->prevPage;  /*setting the prev page value*/
						memcpy(sourceData,dLH,(2*sizeof(long)+2*sizeof(short)));
						#ifdef VIMDEBUG
						printf("\n New values level %d  noe %d  p %d  n %d",dLH->level,dLH->noe,dLH->prevPage,dLH->nextPage);
						#endif
						vSB_C->writeToBufferPool(lH->nextPage,sourceData);
						}
					nok_C-=1;	
					fLH_C->addToFL(nodePointer);
					goto TDEND;
					}
				}
			else
				{
				dLH=(LeafHeader*)MSF::myMalloc(2*sizeof(long)+2*sizeof(short));
				dLH->noe=lH->noe-1;		dLH->level=lH->level;
				dLH->prevPage=lH->prevPage;	dLH->nextPage=lH->nextPage;
				copyData=MSF::myMalloc(PAGESIZE);
				memcpy(copyData,dLH,      2*sizeof(long)+2*sizeof(short));			
				memcpy((char*)copyData + (2*sizeof(long)+2*sizeof(short)),
			   		 (char*)data   + (2*sizeof(long)+2*sizeof(short)),(result-1)*(1*sizeof(long)+1*sizeof(short)+keySize_C));
				if((result-1)<dLH->noe)
					memcpy((char*)copyData + (2*sizeof(long)+2*sizeof(short))+(result-1)*(1*sizeof(long)+1*sizeof(short)+keySize_C),
			 			(char*)data   + (2*sizeof(long)+2*sizeof(short))+(result  )*(1*sizeof(long)+1*sizeof(short)+keySize_C),
			      	                               (dLH->noe-(result-1))*(1*sizeof(long)+1*sizeof(short)+keySize_C));
				vSB_C->writeToBufferPool(nodePointer,copyData);
				nok_C-=1;
				freeKeyList(*entry);
				*entry=NULL;
				goto TDEND;
				}	
			}						
		}
TDEND:		
	if(data!=0)		MSF::myFree(data);
	if(dLH!=0)		MSF::myFree(dLH);
	if(tempData!=0) 	MSF::myFree(tempData);
	if(copyData!=0)		MSF::myFree(copyData);
	if(sourceData!=0)	MSF::myFree(sourceData);
	if(dIH!=0)		MSF::myFree(dIH);
	if(nodePointer==rootPN_C)	updateIndexHeader();
	if(returnFlag==5)	return FAILED;
	else			return SUCCESS;	
	}

//***********************Actual functionality of INDEX MANAGER***********************************************//			
int VIndexManager :: deleteEntry(long indexPN,KeyList *khead,RID *rid) //no need to specify rid in case of deletion on key or unique attribute
	{
#ifdef VERBOSEMODE
	printf("\nInside delete method of VIndexManager class");	
#endif
	int returnFlag=0,i=0;
	void *iHData=0,*iData=0;
	iHData=MSF::myMalloc(PAGESIZE);
	vSB_C->readFromBufferPool(indexPN,iHData);
	IMHeaderPage *imph;
	imph=(IMHeaderPage*)iHData;	
	Fields *fds;
	fds=(Fields*)((char*)iHData + 5*sizeof(long) +8*sizeof(short));	
	int number=imph->nof;
	KeyList *temp=khead;
	KeyList *tempList=NULL;
	iFanout_C=imph->iFanout;
	lFanout_C=imph->lFanout;
	keyType_C=imph->keyType;
	keySize_C=imph->keySize; // -1 because already 1 is in the size of either IndexKeyPagePair or LeafKeyRIDPair
	rootPN_C =imph->rootPN;
	nof_C=imph->nof;	
	indexPN_C=indexPN;
	leafPN_C =imph->leafPN;
	nop_C    =imph->nop;
	nol_C    =imph->nol;
	nok_C    =imph->nok;
	mIK_C    =imph->mIK; 
	duplicateFlag_C=-10;
	IndexHeader *iH=0;
	while(number>0 && temp!=NULL)  //step 1 typechecking of the keys if success process contines else it terminates without inserting
		{
		fds=(Fields*)((char*)iHData+8*sizeof(short)+5*sizeof(long)+i*(2*sizeof(short)+MAXFIELDNAME));	
		if(fds->fieldType!=temp->type)
			{
			returnFlag=1;
			goto DEND;
			}
		
		if(number==imph->nof && (keyType_C==COMPOSITENONKEY || keyType_C==NONKEY) )
			{
			i=i+2;
			number-=2;
			}			
		number--;
		i++;
		temp=temp->next;
		}	
	iData=MSF::myMalloc(PAGESIZE);
	vSB_C->readFromBufferPool(imph->rootPN,iData);
	iH=(IndexHeader*)iData;	
	if(iH->page0==-1) //special case for new index
		{
		#ifdef VIMDEBUG
		printf("\n NEW INDEX NO ENTRIES EXIST TO DELETE\n");
		#endif
		returnFlag=2;
		}
DEND:			
	if(iHData!=0)		MSF::myFree(iHData);
	if(iData!=0)		MSF::myFree(iData);
	if(returnFlag==1)	return FAILED;
	if(returnFlag==2)	return NEWINDEX;
	else	
		{
		if(keyType_C==COMPOSITENONKEY || keyType_C==NONKEY)	
			{
			copyKeyList(khead,&tempList,rid);
			duplicateFlag_C=keyType_C;
			keyType_C=COMPOSITEKEY;
			#ifdef VIMDEBUG
			displayKeyList(tempList);
			#endif
			returnFlag=treeDeleteEntry(-1,rootPN_C,&tempList,rid);
			if(returnFlag==FAILED)	freeKeyList(tempList);
			return returnFlag;	
			}	
		else	
			{
			copyKeyList(khead,&tempList);
			#ifdef VIMDEBUG
			displayKeyList(tempList);
			#endif
			returnFlag=treeDeleteEntry(-1,rootPN_C,&tempList,rid);
			if(returnFlag==FAILED)	freeKeyList(tempList);
			return returnFlag;		
			}
		}
	}
	
VIndexManager :: ~VIndexManager()
	{
	if(DatumC!=0)		MSF::myFree(DatumC);
	if(TempDatumC!=0)	MSF::myFree(TempDatumC);
	if(iMHC!=0)		MSF::myFree(iMHC);
	}						
//************************************************************************************************************	

//Functions to test the working of the Index Structure	
void testCompositeKey()
	{
	long pno;
	VIndexManager *vIM=new VIndexManager();
	FieldNameType *knt=NULL;
	VIndexManager::addToFieldList(&knt,DBINTTYPE,"XYZ",10);
	VIndexManager::addToFieldList(&knt,DBCHARTYPE,"VEN",10);
	pno=vIM->createIndexTree(knt,COMPOSITEKEY);
	VIndexManager::freeFieldList(knt);
	vIM->displayIndexTreeStats(pno);
	RID rid={14,15};
	KeyList *khead=NULL;
	short choice=0;
	int in=0;
	char *df=0;
	while(1)
		{
		printf("\n\tEnter 1 for insertion of key");
		printf("\n\tEnter 2 for Display of stats");
		printf("\n\tEnter 3 for Display of Tree");
		printf("\n\tEnter 4 for Search");
		printf("\n\tEnter 5 for Deletion");
		printf("\n\tEnter 6 for display of node");
		printf("\n\tEnter 7 to exit\n\t");
		scanf("%d",&choice);
		switch(choice)
			{
			case 1:
				df=(char*)MSF::myMalloc(10);
				printf("\n\t\tEnter the values to insert, int and then string\n\t\t");
				khead=NULL;
				scanf("%d",&in);
				df=MSF::itoa(in,df);
				printf("\n");
				VIndexManager::addToKeyList(&khead,DBINTTYPE,df);
				scanf("%s",df);
				VIndexManager::addToKeyList(&khead,DBCHARTYPE,df);
				VIndexManager::displayKeyList(khead);
				printf("\nReturn Value 1 for Success and 2 for FAILED ---> %d",vIM->insert(pno,khead,&rid));
				VIndexManager::freeKeyList(khead);
				MSF::myFree(df);
				break;
			case 2:
				vIM->displayIndexTreeStats(pno);
				break;
			case 3:
				vIM->displayTree(pno);	
				break;
			case 4: 
				df=(char*)MSF::myMalloc(10);
				printf("\n\t\t Enter the value to search,int and then string\n\t\t");
				khead=NULL;
				scanf("%d",&in);
				df=MSF::itoa(in,df);
				printf("\n");
				VIndexManager::addToKeyList(&khead,DBINTTYPE,df);
				scanf("%s",df);
				VIndexManager::addToKeyList(&khead,DBCHARTYPE,df);
				VIndexManager::displayKeyList(khead);				
				printf("\nIn Bucket %d",vIM->find(pno,khead,0,NULL));
				VIndexManager::freeKeyList(khead);
				MSF::myFree(df);
				break;			
			case 5:
				printf("\n Enter the value to Delete, int and then string");
				df=(char*)MSF::myMalloc(10);
				khead=NULL;
				scanf("%d",&in);
				df=MSF::itoa(in,df);
				printf("\n");
				VIndexManager::addToKeyList(&khead,DBINTTYPE,df);
				scanf("%s",df);
				VIndexManager::addToKeyList(&khead,DBCHARTYPE,df);
				VIndexManager::displayKeyList(khead);				
				printf("\nOperation is %d",vIM->deleteEntry(pno,khead,NULL));
				VIndexManager::freeKeyList(khead);
				MSF::myFree(df);
				break;			
			case 6:
				printf("\nEnter the page number to view it\n\t");
				scanf("\n\t%d",&in);
				vIM->displayNode(in);
				break;
			case 7: 
				goto CKTEND;	
			default:
				printf("\n\t Enter Correct choice\n");						
			}	
		}	
CKTEND:	
	vIM->~VIndexManager();
	}
	
void testKey(int type)
	{
	long pno;
	VIndexManager *vIM=new VIndexManager();
	FieldNameType *knt=NULL;
	if(type==DBINTTYPE)
		VIndexManager::addToFieldList(&knt,DBINTTYPE,"XYZ",10);
	else if(type==DBCHARTYPE)
		VIndexManager::addToFieldList(&knt,DBCHARTYPE,"XYZ",10);	
	pno=vIM->createIndexTree(knt,IKEY);
	VIndexManager::freeFieldList(knt);
	vIM->displayIndexTreeStats(pno);
	RID rid={14,15};
	KeyList *khead1=NULL;
	short choice=0;
	int in=0;
	char *df=0;
	while(1)
		{
		printf("\n\tEnter 1 for insertion of key");
		printf("\n\tEnter 2 for Display of stats");
		printf("\n\tEnter 3 for Display of Tree");
		printf("\n\tEnter 4 for Search");
		printf("\n\tEnter 5 for Deletion");
		printf("\n\tEnter 6 for display of node");
		printf("\n\tEnter 7 to exit\n\t");
		scanf("%d",&choice);
		switch(choice)
			{
			case 1:
				df=(char*)MSF::myMalloc(10);
				printf("\n\t\tEnter the values to insert, int and then string\n\t\t");
				khead1=NULL;
				if(type==DBINTTYPE)
					{
					scanf("%d",&in);
					df=MSF::itoa(in,df);
					printf("\n%s",df);
					VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					}
				else if(type==DBCHARTYPE)
					{
					scanf("%s",df);
					printf("\n%s",df);
					VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					}	
				//VIndexManager::displayKeyList(khead1);
				printf("\nReturn Value 1 for Success and 2 for FAILED ---> %d",vIM->insert(pno,khead1,&rid));
				VIndexManager::freeKeyList(khead1);
				MSF::myFree(df);
				break;
			case 2:
				vIM->displayIndexTreeStats(pno);
				break;
			case 3:
				vIM->displayTree(pno);	
				break;
			case 4: 
				df=(char*)MSF::myMalloc(10);
				printf("\n\t\t Enter the value to search,int \n\t\t");
				khead1=NULL;
				if(type==DBINTTYPE)
					{
					scanf("%d",&in);
					df=MSF::itoa(in,df);
					VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					}
				else if(type==DBCHARTYPE)
					{
					scanf("%s",df);
					printf("\n%s",df);
					VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					}
				VIndexManager::displayKeyList(khead1);				
				printf("\nIn Bucket %d",vIM->find(pno,khead1,0,NULL));
				VIndexManager::freeKeyList(khead1);
				MSF::myFree(df);
				break;			
			case 5:
				printf("\n Enter the value to Delete, int ");
				df=(char*)MSF::myMalloc(10);
				khead1=NULL;
				if(type==DBINTTYPE)
					{
					scanf("%d",&in);
					df=MSF::itoa(in,df);
					VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					}
				else if(type==DBCHARTYPE)
					{
					scanf("%s",df);
					printf("\n%s",df);
					VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					}
				VIndexManager::displayKeyList(khead1);				
				printf("\nOperation is %d",vIM->deleteEntry(pno,khead1,NULL));
				VIndexManager::freeKeyList(khead1);
				MSF::myFree(df);
				break;			
			case 6:
				printf("\nEnter the page number to view it\n\t");
				scanf("\n\t%d",&in);
				vIM->displayNode(in);
				break;
			case 7: 
				goto CKTEND;	
			default:
				printf("\n\t Enter Correct choice\n");						
			}	
		}	
CKTEND:	
	vIM->~VIndexManager();
	}	

void testLoadKey(int type)
	{
	long pno;
	VIndexManager *vIM=new VIndexManager();
	VSBufferManager *vsbb=VSBufferManager::getInstance();
	FieldNameType *knt=NULL;
	if(type==DBINTTYPE)
		VIndexManager::addToFieldList(&knt,DBINTTYPE,"XYZ",10);
	else if(type==DBCHARTYPE)
		VIndexManager::addToFieldList(&knt,DBCHARTYPE,"XYZ",10);	
	pno=vIM->createIndexTree(knt,IKEY);
	VIndexManager::freeFieldList(knt);
	vIM->displayIndexTreeStats(pno);
	RID rid={14,15};
	RID *rx=NULL;
	int x=0,y=0,step=0,loopCounter=0;
	KeyList *khead1=NULL;
	short choice=0;
	int in=0;
	char *df=0;
	while(1)
		{
		printf("\n\tEnter 1 for Bulk insertion of keys(Iterative not BULK INSERTION ALGORITHM)");
		printf("\n\tEnter 2 for Insertion of key by key");
		printf("\n\tEnter 3 for Display of stats of Index");
		printf("\n\tEnter 4 for Display of Tree");
		printf("\n\tEnter 5 for Search for a key in bucket");
		printf("\n\tEnter 6 for Deletion of KEYS IN BULK");
		printf("\n\tEnter 7 for Deletion of a key");
		printf("\n\tEnter 8 for display of page");
		printf("\n\tEnter 9 to exit\n\t\t");
		scanf("%d",&choice);
		switch(choice)
			{
			case 1:
				df=(char*)MSF::myMalloc(10);
				printf("\n\t\tEnter the Start Value\n\t\t\t");
				scanf("%d",&x);
				printf("\n\t\tEnter the End Value\n\t\t\t");
				scanf("%d",&y);
				printf("\n\t\tEnter the Step Size\n\t\t\t");
				scanf("%d",&step);
				for(loopCounter=x;loopCounter<=y;loopCounter+=step)
					{
					khead1=NULL;
					df=MSF::itoa(loopCounter,df);
					if(type==DBINTTYPE)
						VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					else if(type==DBCHARTYPE)	
						VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					//VIndexManager::displayKeyList(khead1);	
					//printf("\t %d",vIM->insert(pno,khead1,&rid));
					if(loopCounter%2==0)	{rid.pageNumber= 9;rid.slotID=10;}
					else			{rid.pageNumber=14;rid.slotID=15;}
					vIM->insert(pno,khead1,&rid);
					VIndexManager::freeKeyList(khead1);
					}
				MSF::myFree(df);
				vsbb->displayBufferStatisitics();
				break;	
			case 2:
				df=(char*)MSF::myMalloc(10);
				printf("\n\t\tEnter the values to insert\n\t\t");
				khead1=NULL;
				if(type==DBINTTYPE)
					{
					scanf("%d",&in);
					df=MSF::itoa(in,df);
					printf("\n%s",df);
					VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					}
				else if(type==DBCHARTYPE)
					{
					scanf("%s",df);
					printf("\n%s",df);
					VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					}	
				VIndexManager::displayKeyList(khead1);
				printf("\nReturn Value 1 for Success and 2 for FAILED ---> %d",vIM->insert(pno,khead1,&rid));
				VIndexManager::freeKeyList(khead1);
				MSF::myFree(df);
				break;
			case 3:
				vIM->displayIndexTreeStats(pno);
				break;
			case 4:
				vIM->displayTree(pno);	
				break;
			case 5: 
				df=(char*)MSF::myMalloc(10);
				printf("\n\t\t Enter the value to search\n\t\t");
				khead1=NULL;
				if(type==DBINTTYPE)
					{
					scanf("%d",&in);
					df=MSF::itoa(in,df);
					VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					}
				else if(type==DBCHARTYPE)
					{
					scanf("%s",df);
					printf("\n%s",df);
					VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					}
				VIndexManager::displayKeyList(khead1);				
				printf("\nIn Bucket %d",vIM->find(pno,khead1,NE,&rx));
				//VIndexManager::displayRIDList(rx);
				VIndexManager::freeRIDList(rx);
				rx=NULL;
				VIndexManager::freeKeyList(khead1);
				MSF::myFree(df);
				break;	
			case 6:
				df=(char*)MSF::myMalloc(10);
				printf("\n\t\tEnter the Start Value\n\t\t\t");
				scanf("%d",&x);
				printf("\n\t\tEnter the End Value\n\t\t\t");
				scanf("%d",&y);
				printf("\n\t\tEnter the Step Size\n\t\t\t");
				scanf("%d",&step);
				for(loopCounter=x;loopCounter<=y;loopCounter+=step)
					{
					khead1=NULL;
					df=MSF::itoa(loopCounter,df);
					if(type==DBINTTYPE)
						VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					else if(type==DBCHARTYPE)	
						VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					//VIndexManager::displayKeyList(khead1);	
					//printf("\t %d",vIM->deleteEntry(pno,khead1,NULL));
					vIM->deleteEntry(pno,khead1,NULL);
					VIndexManager::freeKeyList(khead1);
					}
				MSF::myFree(df);
				vsbb->displayBufferStatisitics();
				break;	
						
			case 7:
				printf("\n Enter the value to Delete");
				df=(char*)MSF::myMalloc(10);
				khead1=NULL;
				if(type==DBINTTYPE)
					{
					scanf("%d",&in);
					df=MSF::itoa(in,df);
					VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					}
				else if(type==DBCHARTYPE)
					{
					scanf("%s",df);
					printf("\n%s",df);
					VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					}
				VIndexManager::displayKeyList(khead1);				
				printf("\nOperation is %d",vIM->deleteEntry(pno,khead1,NULL));
				VIndexManager::freeKeyList(khead1);
				MSF::myFree(df);
				break;			
			case 8:
				printf("\nEnter the page number to view it\n\t");
				scanf("\n\t%d",&in);
				vIM->displayNode(in);
				break;
			case 9: 
				goto CKTEND;	
			default:
				printf("\n\t Enter Correct choice\n");		
			}
		}
CKTEND:	
	vIM->~VIndexManager();		
	}

void testLoadCompositeKey(int order) //1 or 2
	{
	long pno;
	VIndexManager *vIM=new VIndexManager();
	VSBufferManager *vsbb=VSBufferManager::getInstance();
	FieldNameType *knt=NULL;
	if(order==1)
		{
		VIndexManager::addToFieldList(&knt,DBINTTYPE,"ABC",10);
		VIndexManager::addToFieldList(&knt,DBCHARTYPE,"XYZ",10);	
		}
	else
		{
		VIndexManager::addToFieldList(&knt,DBCHARTYPE,"XYZ",10);
		VIndexManager::addToFieldList(&knt,DBINTTYPE,"ABC",10);
		}			
	pno=vIM->createIndexTree(knt,COMPOSITEKEY);
	VIndexManager::freeFieldList(knt);
	vIM->displayIndexTreeStats(pno);
	RID rid={14,15};
	int x=0,y=0,step=0,loopCounter=0;
	KeyList *khead1=NULL;
	short choice=0;
	int in=0;
	char *df=0;
	while(1)
		{
		printf("\n\tEnter 1 for Bulk insertion of keys(Iterative not BULK INSERTION ALGORITHM)");
		printf("\n\tEnter 2 for Insertion of key by key");
		printf("\n\tEnter 3 for Display of stats of Index");
		printf("\n\tEnter 4 for Display of Tree");
		printf("\n\tEnter 5 for Search for a key in bucket");
		printf("\n\tEnter 6 for Deletion of KEYS IN BULK");
		printf("\n\tEnter 7 for Deletion of a key");
		printf("\n\tEnter 8 for display of page");
		printf("\n\tEnter 9 to exit\n\t\t");
		scanf("%d",&choice);
		switch(choice)
			{
			case 1:
				df=(char*)MSF::myMalloc(10);
				printf("\n\t\tEnter the Start Value\n\t\t\t");
				scanf("%d",&x);
				printf("\n\t\tEnter the End Value\n\t\t\t");
				scanf("%d",&y);
				printf("\n\t\tEnter the Step Size\n\t\t\t");
				scanf("%d",&step);
				for(loopCounter=x;loopCounter<=y;loopCounter+=step)
					{
					khead1=NULL;
					df=MSF::itoa(loopCounter,df);
					if(order==1)
						{
						VIndexManager::addToKeyList(&khead1,DBINTTYPE,"12");
						VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
						}
					else
						{
						VIndexManager::addToKeyList(&khead1,DBCHARTYPE,"12");
						VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
						}	
					printf(" %d",vIM->insert(pno,khead1,&rid));
					VIndexManager::freeKeyList(khead1);
					}
				MSF::myFree(df);
				vsbb->displayBufferStatisitics();
				break;	
			case 2:
				df=(char*)MSF::myMalloc(10);
				printf("\n\t\tEnter the values to insert\n\t\t");
				khead1=NULL;
				if(order==1)
					{printf("\n first int followed by char\n\t\t");
					scanf("%d",&in);
					df=MSF::itoa(in,df);
					VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					scanf("%s",df);
					VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					}	
				else
					{
					printf("\n first char followed by int\n\t\t");
					scanf("%s",df);
					VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					scanf("%d",&in);
					df=MSF::itoa(in,df);
					VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					}	
				//VIndexManager::displayKeyList(khead1);
				printf("\nReturn Value 1 for Success and 2 for FAILED ---> %d",vIM->insert(pno,khead1,&rid));
				VIndexManager::freeKeyList(khead1);
				MSF::myFree(df);
				break;
			case 3:
				vIM->displayIndexTreeStats(pno);
				break;
			case 4:
				vIM->displayTree(pno);	
				break;
			case 5: 
				df=(char*)MSF::myMalloc(10);
				printf("\n\t\t Enter the value to search\n\t\t");
				khead1=NULL;
				if(order==1)
					{printf("\n first int followed by char\n\t\t");
					scanf("%d",&in);
					df=MSF::itoa(in,df);
					VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					scanf("%s",df);
					VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					}	
				else
					{
					printf("\n first char followed by int\n\t\t");
					scanf("%s",df);
					VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					scanf("%d",&in);
					df=MSF::itoa(in,df);
					VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					}	
				VIndexManager::displayKeyList(khead1);				
				printf("\nIn Bucket %d",vIM->find(pno,khead1,0,NULL));
				VIndexManager::freeKeyList(khead1);
				MSF::myFree(df);
				break;	
			case 6:
				df=(char*)MSF::myMalloc(10);
				printf("\n\t\tEnter the Start Value\n\t\t\t");
				scanf("%d",&x);
				printf("\n\t\tEnter the End Value\n\t\t\t");
				scanf("%d",&y);
				printf("\n\t\tEnter the Step Size\n\t\t\t");
				scanf("%d",&step);
				for(loopCounter=x;loopCounter<=y;loopCounter+=step)
					{
					khead1=NULL;
					df=MSF::itoa(loopCounter,df);
					if(order==1)
						{
						VIndexManager::addToKeyList(&khead1,DBINTTYPE,"12");
						VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
						}
					else
						{
						VIndexManager::addToKeyList(&khead1,DBCHARTYPE,"12");
						VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
						}	
					printf(" %d",vIM->deleteEntry(pno,khead1,NULL));
					VIndexManager::freeKeyList(khead1);
					}
				MSF::myFree(df);
				vsbb->displayBufferStatisitics();
				break;	
						
			case 7:
				printf("\n Enter the value to Delete");
				df=(char*)MSF::myMalloc(10);
				khead1=NULL;
				if(order==1)
					{printf("\n first int followed by char\n\t\t");
					scanf("%d",&in);
					df=MSF::itoa(in,df);
					VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					scanf("%s",df);
					VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					}	
				else
					{
					printf("\n first char followed by int\n\t\t");
					scanf("%s",df);
					VIndexManager::addToKeyList(&khead1,DBCHARTYPE,df);
					scanf("%d",&in);
					df=MSF::itoa(in,df);
					VIndexManager::addToKeyList(&khead1,DBINTTYPE,df);
					}	
				VIndexManager::displayKeyList(khead1);				
				printf("\nOperation is %d",vIM->deleteEntry(pno,khead1,NULL));
				VIndexManager::freeKeyList(khead1);
				MSF::myFree(df);
				break;			
			case 8:
				printf("\nEnter the page number to view it\n\t");
				scanf("\n\t%d",&in);
				vIM->displayNode(in);
				break;
			case 9: 
				goto CKTEND;	
			default:
				printf("\n\t Enter Correct choice\n");		
			}
		}
CKTEND:	
	vIM->~VIndexManager();		
	}
/*					
int main()
	{
	VSBufferManager *vsb=VSBufferManager::getInstance();
	DiskSpaceManagerLinux *dSML_C=DiskSpaceManagerLinux::getInstance(DBFILENAME);
	ReplacementPolicy *rP_C= vsb->getRPFromFactory(RPLRU);//Random~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	vsb->initializeBufferPool(500,dSML_C,rP_C);
	DBHeader *dBH_C;
	FLHH *flh;
	void * da=0;
	da=MSF::myMalloc(PAGESIZE);
	
	dBH_C=(DBHeader*)da;
	dBH_C->fpListPointer=1;
       	dBH_C->dbCatPointer=2;
        dBH_C->tCatPointer=3;
       	dBH_C->aCatPointer=4;
        dBH_C->iCatPointer=5;
       	dBH_C->assignedPN=6;	
	vsb->writeToBufferPool(0,da);
	
	flh=(FLHH*)da;
	flh->flag=FLEMPTY;
	flh->prevPN=FLEMPTY;
	flh->nextPN=FLEMPTY;
	flh->index=0;
	vsb->writeToBufferPool(1,da);
	printf("\n");
	//testCompositeKey();
	//testKey(DBCHARTYPE);
	testLoadKey(DBINTTYPE);
	//testLoadCompositeKey(1);
	FLH *fSH=FLH::getInstance();
	fSH->printFLH();
	MSF::myFree(da);
	fSH->~FLH();
	vsb->~VSBufferManager(); // remove this on integrating
	}
*/
  n��=0