#include "BufferManager/standards.h"
#include "VSDBEngine.h"

/*********************************CreateTable Query Handler***********************************/
CTQH* CTQH :: singleInstance_C=0;

CTQH* CTQH :: getInstance()
        {
        if(singleInstance_C==0)
		{
#ifdef DBCONDES		
		printf("\n\t\tIn Static Invoker of CTQH of TLQH");
#endif			
                singleInstance_C=new CTQH();	
		}
        return singleInstance_C;
        }

CTQH :: CTQH() // Constructor
        {
#ifdef DBCONDES
	printf("\n\t\t  In Constructor of CTQH of TLQH"); 
#endif	
	sCH_C=SysCatHan::getInstance();
	iQH_C=IQH::getInstance();
	dPH_C=DPH::getInstance();
	fLH_C=0;//FLH::getInstance();
	vSB_C=VSBufferManager::getInstance();
        }
	
CTQH :: ~CTQH()
	{
#ifdef DBCONDES
	printf("\n\t\t  In Destructor of CTQH of TLQH");
#endif	
	}	
	
int CTQH :: executeCreateT(TLQds *t)
	{
	long hPN=0;
	void *datum=0;
	void *dPData=0;
	HeaderPage *hd=0;
	short numberOfAttributes=0;
	FieldNameType *pK=NULL,*tempFieldNameType;
	char *temp=0;
	if(fLH_C==0)	fLH_C=FLH::getInstance();         //instantiating here to avoid conflict of creating database if mydb is not present
	long result=0;
#ifdef VERBOSEMODE
	printf("\n\t\tInside executeCreateT method of CTQH class:%s database %s tableName",t->dbname,t->objectName);	
#endif	
	if(strcmp(t->dbname,"sys")) //cant create table in sys database
		{
		result=sCH_C->returnTableExistence(t->objectName,t->dbname,CHECK);//store the result to delete the total table structure
		if(result!=FAILED) //note the problem with the return value of return Table existence.. gives a non -1 value for success,c the function 
			printf("\n\tTable \"%s\" exists in the Current database \"%s\" ",t->objectName,t->dbname);
		else
			{
			//check for existence of pKeyFields and uniqueFields in the list of columns specified  
			createColumns *cC=t->columns;
			short okFlag=0;
			FieldNames *tempF;
			tempF=t->pKeyFields;
			while(tempF!=NULL)
				{
				cC=t->columns;
				while(cC!=NULL)
					{
					if(!strcmp(cC->columnName,tempF->name))
						{
						cC->primaryKey=YES;
						okFlag=1;
						break;
						}
					cC=cC->next;
					}
				if(okFlag==0)
					{
					printf("\n\t!! ERROR: %s column does not Exists in columns defination part,check Primary Keys",tempF->name);
					goto CEND;	
					}
				else
					okFlag=0;
				tempF=tempF->next;		
				}
			tempF=t->uniqueFields;
			while(tempF!=NULL)
				{
				cC=t->columns;
				while(cC!=NULL)
					{
					if(!strcmp(cC->columnName,tempF->name))
						{
						cC->unique=YES;
						okFlag=1;
						break;
						}
					cC=cC->next;
					}
				if(okFlag==0)
					{
					printf("\n\t!! ERROR: %s column does not Exists in columns defination part, check Unique elements",tempF->name);
					goto CEND;	
					}
				else
					okFlag=0;
				tempF=tempF->next;		
				}		
			hPN=fLH_C->getFreePage();
			datum=MSF::myMalloc(PAGESIZE); //to store header page details
			dPData=MSF::myMalloc(PAGESIZE);
			hd=(HeaderPage*)datum;
			hd->totNumRec=0;
			hd->totNumDP=1;
			hd->maxDEInDP=MAXDE;
			//printf("\n CALL FOR CREATE TABLE 2");
			hd->pNFirstDP=fLH_C->getFreePage();
			dPH_C->initializeDPandDE(dPData);
			vSB_C->writeToBufferPool(hd->pNFirstDP,dPData);
			vSB_C->writeToBufferPool(hPN,datum);
			//Registering the table in the sys catalog and building the index structure, there by registering it with the icatalog 
			RLQds *rDS=0; //firstly registerng it in to the tcat
			rDS=(RLQds*)MSF::myMalloc(sizeof(RLQds)); // inserting sys value
			rDS->objectName=(char*)MSF::myMalloc(strlen("tcat"));
			strcpy(rDS->objectName,"tcat");
			rDS->dbname=NULL;
			rDS->values=NULL;
			rDS->names=NULL;
			RLQH :: insertToAttList(&rDS->values,DLQH::dBName);
			RLQH :: insertToAttList(&rDS->values,t->objectName);
			temp=(char*)MSF::myMalloc(12);
			RLQH :: insertToAttList(&rDS->values,MSF::itoa(hPN,temp));//**************inserting to tcat***********************	
			iQH_C->insertToCatTables(rDS);  
			RLQH :: freeList(rDS);
			MSF::myFree(temp);
			
			iQH_C->insertToSysCat(t); 				//************************inserting to acat*******************
			
			cC=t->columns;            //************************inserting to icat and iacat*******************
			while(cC!=NULL)
				{
				RLQds *iRDS=0;
				iRDS=(RLQds*)MSF::myMalloc(sizeof(RLQds)); 
				iRDS->objectName=(char*)MSF::myMalloc(strlen("icat"));
				strcpy(iRDS->objectName,"icat");
				iRDS->dbname=NULL;
				iRDS->names=NULL;
				iRDS->values=NULL;
				
				RLQds *iaRDS=0;
				iaRDS=(RLQds*)MSF::myMalloc(sizeof(RLQds)); 
				iaRDS->objectName=(char*)MSF::myMalloc(strlen("iacat"));
				strcpy(iaRDS->objectName,"iacat");
				iaRDS->dbname=NULL;
				iaRDS->names=NULL;
				iaRDS->values=NULL;
				if(cC->unique==YES)// for each of the primary key and unique attribute create an index
					{
					VIndexManager *vIMTemp=new VIndexManager();				
					FieldNameType *f=NULL;
					short type=0;
					VIndexManager::addToFieldList(&f,cC->dataType,cC->columnName,cC->attLen);
					hPN=vIMTemp->createIndexTree(f,IKEY);
					#ifdef VERBOSEMODE
					vIMTemp->displayIndexTreeStats(hPN);
					#endif
					//printf("\nFree Page %d\t",hPN);
					temp=(char*)MSF::myMalloc(strlen(t->objectName)+10);
					temp=MSF::itoa(hPN,temp);
					strcat(temp,t->objectName);
					RLQH :: insertToAttList(&iRDS->values,t->dbname);
					RLQH :: insertToAttList(&iRDS->values,t->objectName);
					RLQH :: insertToAttList(&iRDS->values,temp);
					temp=MSF::itoa(hPN,temp);
					RLQH :: insertToAttList(&iRDS->values,temp);
					
					iQH_C->insertToCatTables(iRDS);//****************inserting into icat**************************
					
					strcat(temp,t->objectName);
					RLQH :: insertToAttList(&iaRDS->values,temp);
					RLQH :: insertToAttList(&iaRDS->values,cC->columnName);
					RLQH :: insertToAttList(&iaRDS->values,"1");
					
					iQH_C->insertToCatTables(iaRDS);//****************inserting into iacat**************************
					RLQH::freeList(iRDS);
					RLQH::freeList(iaRDS);
					MSF::myFree(temp);
					delete vIMTemp;
					}
				if(cC->primaryKey==YES)
					{
					VIndexManager::addToFieldList(&pK,cC->dataType,cC->columnName,cC->attLen);
					numberOfAttributes++;
					}
				cC=cC->next;
				}		
			if(pK!=NULL)	//all primary key attributes are collected
				{
				long pno=0;
				//VIndexManager::displayFieldList(pK);
				//create the index structure for the primary key here, distinguish between composite key ana an IKEY
				//printf("\n %d is the number of attributes\n",numberOfAttributes);
				VIndexManager *vIMT=new VIndexManager();
				if(numberOfAttributes>1)				pno=vIMT->createIndexTree(pK,COMPOSITEKEY);
				else							pno=vIMT->createIndexTree(pK,IKEY);
				#ifdef VERBOSEMODE
				vIMT->displayIndexTreeStats(pno);
				#endif
				//************************inserting to icat*******************
				RLQds *iRDST=0;
				iRDST=(RLQds*)MSF::myMalloc(sizeof(RLQds)); 
				iRDST->objectName=(char*)MSF::myMalloc(strlen("icat"));
				strcpy(iRDST->objectName,"icat");
				iRDST->dbname=NULL;
				iRDST->names=NULL;
				iRDST->values=NULL;
				temp=(char*)MSF::myMalloc(strlen(t->objectName)+10);
				temp=MSF::itoa(pno,temp);
				strcat(temp,t->objectName);
				RLQH :: insertToAttList(&iRDST->values,t->dbname);
				RLQH :: insertToAttList(&iRDST->values,t->objectName);
				RLQH :: insertToAttList(&iRDST->values,temp);
				temp=MSF::itoa(pno,temp);
				RLQH :: insertToAttList(&iRDST->values,temp);
				iQH_C->insertToCatTables(iRDST);
				strcat(temp,t->objectName);
				numberOfAttributes=0;
				char *numTemp=(char*)MSF::myMalloc(4);
				//************************inserting to iacat*******************
				while(pK!=NULL)
					{
					RLQds *iaRDST=0;
					iaRDST=(RLQds*)MSF::myMalloc(sizeof(RLQds)); 
					iaRDST->objectName=(char*)MSF::myMalloc(strlen("iacat"));
					strcpy(iaRDST->objectName,"iacat");
					iaRDST->dbname=NULL;
					iaRDST->names=NULL;
					iaRDST->values=NULL;
					RLQH :: insertToAttList(&iaRDST->values,temp);
					RLQH :: insertToAttList(&iaRDST->values,pK->fieldName);
					MSF::itoa(++numberOfAttributes,numTemp);
					RLQH :: insertToAttList(&iaRDST->values,numTemp);
				
					iQH_C->insertToCatTables(iaRDST);//****************inserting into iacat**************************
				
					RLQH::freeList(iaRDST);
					pK=pK->next;
					}
				RLQH :: freeList(iRDST);
				MSF::myFree(temp);
				MSF::myFree(numTemp);
				VIndexManager::freeFieldList(pK);
				delete vIMT;			
				}
			/*	
			TableScan *tS=TableScan::getInstance();//testing for the creation of table
			RID *r=NULL;
			printf("\n\n Contents of Attribute Catalog after creation of this table\n");
			tS->doTableScan("acat","sys",&r,NULL,DISPLAY,"tname",t->objectName,EQ);	
			printf("\n\n Contents of Table Catalog after creation of this table\n");
			tS->doTableScan("tcat","sys",&r,NULL,DISPLAY,"tname",t->objectName,EQ);
			printf("\n\n Contents of Index Catalog after creation of this table\n");
			tS->doTableScan("icat","sys",&r,NULL,DISPLAY,"dbname",t->dbname,EQ);
			printf("\n\n Contents of Index Name Catalog after creation of this table\n");
			tS->doTableScan("iacat","sys",&r,NULL,DISPLAY,NULL,NULL,EQ);
			printf("\n RIDS retrieved during the Table Scan");
			RH::displayRIDList(r);
			RH::freeRIDList(r);
			*/
			printf("\n\n\tTable \"%s\" has been Successfully Created in Current Database \"%s\" ",t->objectName,t->dbname);		
			}
		}
	else
		{
		printf("\n\t!! Table \"%s\" cant be created in the Current database \"%s\" :: Access Denied ",t->objectName,t->dbname);
		}
CEND:			
	if(datum!=0)	MSF::myFree(datum);
	if(dPData!=0)	MSF::myFree(dPData);
	}	

/*********************************Drop  Table Query Handler***********************************/
DTQH* DTQH :: singleInstance_C=0;

DTQH* DTQH :: getInstance()
        {
        if(singleInstance_C==0)
		{
#ifdef DBCONDES		
		printf("\n\t\tIn Static Invoker of DTQH of TLQH");
#endif			
                singleInstance_C=new DTQH();		
		}
        return singleInstance_C;
        }

DTQH :: DTQH()
        {
#ifdef DBCONDES
	printf("\n\t\t  In Constructor of DTQH of TLQH");
#endif		
	sCH_C=SysCatHan::getInstance();
	tS_C =TableScan::getInstance();
	rH_C =RH::getInstance();
	}

DTQH :: ~DTQH()
	{
#ifdef DBCONDES
	printf("\n\t\t  In Destructor of DTQH of TLQH");
#endif	
	}		

int DTQH :: executeDropT(TLQds *t)
	{
#ifdef VERBOSEMODE
	printf("\n\t\tInside executeDropT method of DTQH class:%s database %s tableName",t->dbname,t->objectName);	
#endif	
	long result=0;
	DPH *dPH=DPH::getInstance();
	Fetch *fetch=Fetch::getInstance();
	AttributeValues *avalues=NULL;
	AttributeValues *indvalues=NULL;
	RID *r=NULL;
	RID *s1RID=NULL;
	RID *s2RID=NULL;
	RID *tempRID;//for freeing
	createColumns *columns=NULL;
	result=sCH_C->returnTableExistence(t->objectName,t->dbname,CHECK);//store the result to delete the total table structure
	if(result==FAILED)
		printf("\n\tTable \"%s\" does not exist in the Current database \"%s\" ",t->objectName,t->dbname);
	else
		{	
		if(!strcmp(t->dbname,"sys"))
			{
			printf("\n\tNothing can be deleted from the sys catalog i.e.., \"sys\" database\n");
			goto END;
			}
		//do Table Scan of acat and tcat	
		tS_C->doTableScan("acat","sys",&r,NULL,CHECK,"tname",t->objectName,EQ);	
		tS_C->doTableScan("tcat","sys",&r,NULL,CHECK,"tname",t->objectName,EQ);//r holds all the rids of acat, tcat entries for t->objectName table
		
		//do process to retrieve the icat and iacat contents
		tS_C->doTableScan("icat","sys",&s1RID,NULL,CHECK,"dbname",t->dbname,EQ);		//step 1  Filtering the dbname	
		sCH_C->getDetailsOfFields("icat","sys",&columns);						//step 2a Getting the column structure of icat	
		fetch->fetchRIDs(s1RID,&s2RID,CHECK,"tname",EQ,t->objectName,columns);			//step 2b Filtering the tablename		
		fetch->fetchColumnValues(s2RID,&avalues,&indvalues,"iname","indpn",columns);		//step 3  populating the avalues with inames	
		RH::freeRIDList(s1RID);	s1RID=NULL;			//
											//s2RID holds the rids of entries in icat	//
		
		AttributeValues *temp=avalues; //retrieve the entries of iacat
		while(temp!=NULL)
			{
			tS_C->doTableScan("iacat","sys",&s1RID,NULL,CHECK,"iname",temp->value,EQ);
			temp=temp->next;  //this holds the index names
			}								//s1RID holds the rids of entries in iacat
		
		//freeing all the indexes	
		temp=indvalues;
		while(temp!=NULL)
			{
			VIndexManager *vIM=new VIndexManager();
			//printf("\n %s index structure",temp->value);
			vIM->freeIndexTree(atoi(temp->value));
			delete vIM;	
			temp=temp->next;
			}
		
		//deleting the contents in sys catalog	
		tempRID=r; //acat+tcat
		while(tempRID!=NULL)
			{
			//printf("\n%ld %d",tempRID->pageNumber,tempRID->slotID);
			rH_C->deleteRecord(tempRID);
			tempRID=tempRID->next;
			}			
		
		tempRID=s1RID;//iacat
		while(tempRID!=NULL)
			{
			//printf("\n%ld %d",tempRID->pageNumber,tempRID->slotID);
			rH_C->deleteRecord(tempRID);
			tempRID=tempRID->next;
			}	
		
		tempRID=s2RID;//icat
		while(tempRID!=NULL)
			{
			//printf("\n%ld %d",tempRID->pageNumber,tempRID->slotID);
			rH_C->deleteRecord(tempRID);
			tempRID=tempRID->next;
			}		
		//RH::displayRIDList(s1RID);	
		//freeing the Table structure
		
		dPH->freeTableStructure(result);//main deleting
		
		TLQH::freeCColumns(columns); columns=NULL;	
		RH::freeRIDList(s1RID);	s1RID=NULL;
		RH::freeRIDList(s2RID);	s1RID=NULL;
		RH::freeRIDList(r);	s1RID=NULL;
		RLQH::freeAttList(avalues);
		RLQH::freeAttList(indvalues);									
		//printf("\n drop the table dude");	
		}	
END:	{}
	}	

/*********************************Alter Table Query Handler***********************************/
ATQH* ATQH :: singleInstance_C=0;

ATQH* ATQH :: getInstance()
	{
	if(singleInstance_C==0)
		{
#ifdef DBCONDES		
		printf("\n\t\tIn Static Invoker of ATQH of TLQH");
#endif				
                singleInstance_C=new ATQH();	
		}
        return singleInstance_C;		 
	}	

ATQH :: ATQH()
	{
#ifdef DBCONDES
	printf("\n\t\t  In Constructor of ATQH of TLQH");
#endif		
	sCH_C=SysCatHan::getInstance();	
	}	

	
ATQH :: ~ATQH()
	{
#ifdef DBCONDES
	printf("\n\t\t  In Destructor of ATQH of TLQH");
#endif	
	}
	
int ATQH :: executeAlterT(TLQds *t)
	{
#ifdef VERBOSEMODE
	printf("\n\t\tInside executeAlterT method of ATQH class:%s database %s tableName",t->dbname,t->objectName);	
#endif	
	}

/*********************************Show Table Query Handler***********************************/
STQH* STQH :: singleInstance_C=0;

STQH* STQH :: getInstance()
	{
	if(singleInstance_C==0)
		{
#ifdef DBCONDES		
		printf("\n\t\tIn Static Invoker of STQH of TLQH");
#endif				
                singleInstance_C=new STQH();	
		}
        return singleInstance_C;		 
	}	

STQH :: STQH()
	{
#ifdef DBCONDES
	printf("\n\t\t  In Constructor of ATQH of TLQH");
#endif		
	sCH_C=SysCatHan::getInstance();	
	vSB_C=VSBufferManager::getInstance();
	tS_C=TableScan::getInstance();
	}	

	
STQH :: ~STQH()
	{
#ifdef DBCONDES
	printf("\n\t\t  In Destructor of STQH of TLQH");
#endif	
	}
	
int STQH :: executeShowT(TLQds *t)
	{
#ifdef VERBOSEMODE
	printf("\n\t\tInside executeShowT method of STQH class:%s database %s tableName",t->dbname,t->objectName);	
#endif	
	if(sCH_C->returnTableExistence(t->objectName,t->dbname,CHECK)==FAILED) //table doesnot exist in the current database, so create a new one
		{
		printf("\n\t!!\"%s\" table doesnot exist in the \"%s\" database in use",t->objectName,t->dbname);
		}
	else	//table exists so describe it
		{
		printf("\n\tDetails of \"%s\" table of \"%s\" database",t->objectName,t->dbname);
		printf("\n   --------------------------------------------------------------------------------------------------------------");
		Fetch *fetch=Fetch::getInstance();
		IndexOnTable *iOT=NULL;
		SysCatHan *sCH=SysCatHan::getInstance();
		RID *r=NULL,*r1=NULL;
		createColumns *columns=NULL;
		tS_C->doTableScan("acat","sys",&r,NULL,CHECK,"dbname",t->dbname,EQ); //filtering with dbname
		sCH->getDetailsOfFields("acat","sys",&columns);		
		fetch->fetchRIDs(r,&r1,DISPLAY,"tname",EQ,t->objectName,columns);
		sCH->returnIndexExistence(t->dbname,t->objectName,&iOT);
		printf("\n\n\tDetails of Indexes defined on table:");
		SysCatHan::displayIOT(iOT);
		SysCatHan::freeIOT(iOT);
		//RH::displayRIDList(r1);
		TLQH::freeCColumns(columns);
		RH::freeRIDList(r1);
		RH::freeRIDList(r);
		}	
	}	
/*********************************Table Level Query Handler***********************************/
TLQH* TLQH :: singleInstance_C=0;

TLQH* TLQH :: getInstance()
	{
	if(singleInstance_C==0)
		{
#ifdef DBCONDES		
		printf("\n\tIn Static Invoker of TLQH");
#endif			
		singleInstance_C=new TLQH();		
		}
	return singleInstance_C;
	}	

TLQH :: TLQH()
	{
#ifdef DBCONDES
	printf("\n\t  In Constructor of TLQH");
#endif
	cTQH_C= CTQH::getInstance();
	dTQH_C= DTQH::getInstance();
	aTQH_C= ATQH::getInstance();
	sTQH_C= STQH::getInstance();
	}
	
TLQH :: ~TLQH()
	{
#ifdef DBCONDES
	printf("\n\t  In Destructor of TLQH");
#endif
	cTQH_C->~CTQH();
	dTQH_C->~DTQH();
	aTQH_C->~ATQH();
	sTQH_C->~STQH();	
	}
			
int TLQH :: testAndUpdateList(createColumns** destColumns,createColumns* sourceColumns,FieldNames* fNames_P)
	{	
	int returnFlag=0,flag=0;
	createColumns *temp=sourceColumns;
	while(fNames_P!=NULL)
		{
		sourceColumns=temp;
		while(sourceColumns!=NULL)		
			{
			if(!strcmp(sourceColumns->columnName,fNames_P->name))
				{
				//printf("\nMatched %s",fNames_P->name);
				flag=1;
				if(*destColumns==NULL)
					{
					*destColumns=(createColumns*)MSF::myMalloc(sizeof(createColumns));
					(*destColumns)->columnName=(char*)MSF::myMalloc(strlen(sourceColumns->columnName));
					strcpy((*destColumns)->columnName,sourceColumns->columnName);
					(*destColumns)->dataType=sourceColumns->dataType;
					(*destColumns)->attLen      =sourceColumns->attLen;       (*destColumns)->precision =sourceColumns->precision; 
					if(sourceColumns->defaultValue==0)
						(*destColumns)->defaultValue=0;
					else
						{
						(*destColumns)->defaultValue=(char*)MSF::myMalloc(strlen(sourceColumns->defaultValue));
						strcpy((*destColumns)->defaultValue,sourceColumns->defaultValue);
						}	
					(*destColumns)->notNull   =sourceColumns->notNull;	(*destColumns)->unique      =sourceColumns->unique;
					(*destColumns)->primaryKey=sourceColumns->primaryKey;
					(*destColumns)->position    =sourceColumns->position;	  (*destColumns)->next=NULL;
					}
				else
					{
					createColumns *t=(*destColumns);	
					while(t->next!=NULL)
						t=t->next;
					createColumns *temp;	
					temp=(createColumns*)MSF::myMalloc(sizeof(createColumns));
					temp->columnName=(char*)MSF::myMalloc(strlen(sourceColumns->columnName));
					strcpy(temp->columnName,sourceColumns->columnName);
					temp->dataType=sourceColumns->dataType;
					temp->attLen      =sourceColumns->attLen;       temp->precision =sourceColumns->precision; 
					if(sourceColumns->defaultValue==0)
						temp->defaultValue=0;
					else
						{
						temp->defaultValue=(char*)MSF::myMalloc(strlen(sourceColumns->defaultValue));
						strcpy(temp->defaultValue,sourceColumns->defaultValue);
						}
					temp->notNull   =sourceColumns->notNull;	
					temp->unique      =sourceColumns->unique;       temp->primaryKey=sourceColumns->primaryKey;
					temp->position    =sourceColumns->position;	   temp->next   =NULL;
					t->next=temp;
					}				
				goto NEXTWHILE;
				}
			sourceColumns=sourceColumns->next;
			}
		NEXTWHILE:	
		if(flag==0)
			{
			printf("\n\t\"%s\" Column Does not Exist",fNames_P->name);
			returnFlag=1;
			goto TAULEND;
			}
		else
			flag=0;		
		fNames_P=fNames_P->next;	
		}	
TAULEND:
	if(returnFlag==1) 	return FAILED;
	else			return SUCCESS;		
	}

void TLQH :: freeCColumns(createColumns *head)
	{
	createColumns *temp;
	if(head!=NULL)
		{
		while(head!=NULL)
			{
			temp=head;
			head=head->next;
			MSF::myFree(temp->columnName);
			if(temp->defaultValue!=0)	MSF::myFree(temp->defaultValue);
			MSF::myFree(temp);	
			}
		}		
	}
			
void TLQH :: freeList(TLQds *r)
	{
	if(r!=0)
		{
#ifdef INSTALLDEBUG		
		printf("\n\t TLQds *r: of SysCatalog Initialization");	
#endif		
		createColumns *temp;
		if(r->columns!=NULL)
			{
			while(r->columns!=NULL)
				{
				temp=r->columns;
				r->columns=(r->columns)->next;
				MSF::myFree(temp->columnName);
				if(temp->defaultValue!=0)	MSF::myFree(temp->defaultValue);
				MSF::myFree(temp);	
				}
			}		
		FieldNames *tempF;
		while(r->pKeyFields!=NULL)
			{
			tempF=r->pKeyFields;
			r->pKeyFields=(r->pKeyFields)->next;
			MSF::myFree(tempF->name);
			MSF::myFree(tempF);
			}		
		while(r->uniqueFields!=NULL)
			{
			tempF=r->uniqueFields;
			r->uniqueFields=(r->uniqueFields)->next;
			MSF::myFree(tempF->name);
			MSF::myFree(tempF);
			}		
		if(r->objectName!=NULL)		MSF::myFree(r->objectName);
		if(r->dbname!=NULL)	MSF::myFree(r->dbname);	
		MSF::myFree(r);
		}
	r=0;	
	}
	
void TLQH :: displayList(createColumns *head)
	{
	while(head!=NULL)
		{
		printf("\n%15s %15d %4d %4d %5s %4d %4c %4c %4c",head->columnName,head->dataType,head->attLen,
    		        head->precision,head->defaultValue,head->position,head->primaryKey,head->notNull,head->unique);
		head=head->next;	
		}	
	printf("\n");	
	}	

void TLQH :: InsertToList(createColumns **head,char *cname,int dtype,int attlen,int preci,
		                       char *defValue,int nNull,int uniq,int pKey,int pos)
	{
	short i=0;
	if(*head==NULL)
		{
#ifdef INSTALLDEBUG		
		printf("\n\t In Head is NULL");	
#endif		
		*head=(createColumns*)MSF::myMalloc(sizeof(createColumns));
		
		(*head)->columnName=(char*)MSF::myMalloc(strlen(cname));
		strcpy((*head)->columnName,cname);
		(*head)->dataType=dtype;
		(*head)->attLen=attlen; (*head)->precision=preci; 
		if(defValue==0)
			(*head)->defaultValue=0;
		else
			{
			(*head)->defaultValue=(char*)MSF::myMalloc(strlen(defValue));
			if(strchr(defValue,'\"')!=NULL)
				{
				for(i=1;defValue[i]!='\"';i++)
					(*head)->defaultValue[i-1]=defValue[i];
				(*head)->defaultValue[i-1]='\0';	
				}	
			else	
				strcpy((*head)->defaultValue,defValue);
			}	
		(*head)->notNull=nNull;	(*head)->unique=uniq;     (*head)->primaryKey=pKey;
		(*head)->position=pos;
		(*head)->next=NULL;
		}	
	else
		{
		createColumns *t=(*head);	
		while(t->next!=NULL)
			t=t->next;
		createColumns *temp;	
		temp=(createColumns*)MSF::myMalloc(sizeof(createColumns));
        		
		temp->columnName=(char*)MSF::myMalloc(strlen(cname));   
		strcpy(temp->columnName,cname);
		temp->dataType=dtype;
		temp->attLen=attlen; temp->precision=preci; 
		if(defValue==0)
			temp->defaultValue=0;
		else
			{
			temp->defaultValue=(char*)MSF::myMalloc(strlen(defValue));
			if(strchr(defValue,'\"')!=NULL)
				{
				for(i=1;defValue[i]!='\"';i++)
					temp->defaultValue[i-1]=defValue[i];
				temp->defaultValue[i-1]='\0';	
				}	
			else	
				strcpy(temp->defaultValue,defValue);
			} 
		temp->notNull=nNull;
        	temp->unique=uniq; temp->primaryKey=pKey; temp->position=pos; 
		temp->next=NULL;
		t->next=temp;
		}
	}	
	
int TLQH :: executeTLQ(TLQds *t)
	{
#ifdef VERBOSEMODE
	printf("\n\tInside executeTLQ method of TLQH class");	
#endif	
	if(t->queryType==USETABLE)
		;//****************NOTE: to be handled...........Show table also................
        else if(t->queryType==CREATETABLE)
                cTQH_C->executeCreateT(t);
        else if(t->queryType==DROPTABLE)
                dTQH_C->executeDropT(t);
        else if(t->queryType==ALTERTABLE)
                aTQH_C->executeAlterT(t);
	else if(t->queryType==SHOWTABLE)
                sTQH_C->executeShowT(t);	
	}	
  �