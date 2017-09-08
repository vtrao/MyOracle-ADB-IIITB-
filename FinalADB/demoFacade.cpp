#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main()
	{
	char commands[100][200];
	char fileName[20];
	int choice,index=0,j=0,from,to;
	char executeString[50];
	FILE *fp,*fpp;
	char ch;
	system("clear");
	while(1)
		{
		//system("clear");
		printf("\n\t\t\t\tMAIN MENU OF RDBMS SOFTWARE DEMO\n");
		printf("-----------------------------------------------------------------------------------");
		printf("\n\t\tEnter 1 to Give Input to RDBMS Software from file(Silent Mode)");
		printf("\n\t\tEnter 2 to Give Input to RDBMS Software from file(Verbose Mode)");
		printf("\n\t\tEnter 3 to goto the Command Interpreter of RDBMS software(Silent Mode)");
		printf("\n\t\tEnter 4 to goto the Command Interpreter of RDBMS software(Verbose Mode)");
		printf("\n\t\tEnter 5 to Delete the rdbms database file and reinstall");
		printf("\n\t\tEnter 6 to test the DBEngine");
		printf("\n\t\tEnter 7 to test the Indexer");
		printf("\n\t\tEnter 8 to test the BufferManager");
		printf("\n\t\tEnter 9 to exit\n\t\t");
		scanf("%d",&choice);
		switch(choice)
			{
			case 1:
				printf("\n\t\t\t(Silent Mode) Enter the filename to read the inputs from\n\t\t");
				scanf("%s",&fileName);
				index=0;j=0;
				fp=fopen(fileName,"r");
				if(fp==NULL)
					{
					printf("\n Invalied input file! \"%s\"\n",fileName);
					goto OUT;
					}
				while((ch=fgetc(fp))!=EOF)
					{
					if(ch==';'){commands[index][j]='\0';index++;j=0;}
					else if(ch=='\n'){}
					else	 commands[index][j++]=ch;	
					}
				fclose(fp);	
				while(1)
					{
					printf("\n Commands in input File\n");
					for(j=0;j<index;j++)printf("\t\t%d. %s\n",j+1,commands[j]);
					printf("\n\tEnter the index number \"from\" where \"to\" where to execute the commands -1 to exit\n\t\t");
					scanf("%d",&from);
					if(from<0) goto OUT;
					scanf("%d",&to);
					if(to<from || to>(index) ) {printf("\n\t\tINVALID INPUT \n");goto OUT;}
					for(j=from-1;j<to;j++)
						{
						printf("\n************** Executing \"%s\" command************************",commands[j]);
						fpp=fopen("temp","w");
						fprintf(fpp,"%s;\nquit;\n",commands[j]);
						fclose(fpp);
						
						strcpy(executeString,"./mainS temp");
						system(executeString);
						}				
					strcpy(executeString,"./mainS ");
					system(executeString);		
					}
				OUT: 
				break;
			case 2:
				printf("\n\t\t\t(Verbose Mode) Enter the filename to read the inputs from\n\t\t");
				scanf("%s",&fileName);
				index=0;j=0;
				fp=fopen(fileName,"r");
				if(fp==NULL)
					{
					printf("\n Invalied input file! \"%s\"\n",fileName);
					goto OUT1;
					}
				while((ch=fgetc(fp))!=EOF)
					{
					if(ch==';'){commands[index][j]='\0';index++;j=0;}
					else if(ch=='\n'){}
					else	 commands[index][j++]=ch;	
					}
				fclose(fp);	
				while(1)
					{
					printf("\n Commands in input File\n");
					for(j=0;j<index;j++)printf("\t\t%d. %s\n",j+1,commands[j]);
					printf("\n\tEnter the index number \"from\" where \"to\" where to execute the commands -1 to exit\n\t\t");
					scanf("%d",&from);
					if(from<0) goto OUT1;
					scanf("%d",&to);
					if(to<from || to>(index) ) {printf("\n\t\tINVALID INPUT \n");goto OUT1;}
					for(j=from-1;j<to;j++)
						{
						printf("\n************** Executing \"%s\" command************************",commands[j]);
						fpp=fopen("temp","w");
						if(!strcmp(commands[j],"quit;") || !strcmp(commands[j],"quit")) fprintf(fpp,"%s;\n",commands[j]);
						else		fprintf(fpp,"%s;\nquit;\n",commands[j]);
						fclose(fpp);
						
						strcpy(executeString,"./mainV temp");
						system(executeString);
						}				
					strcpy(executeString,"./mainV ");
					system(executeString);		
					}
				OUT1: 
				break;
			case 3:	
				printf("\n\t\t\t(Silent Mode)Switching over to Command Interpreter\n");
				strcpy(executeString,"./mainS ");
				system(executeString);
				break;
			case 4:
				printf("\n\t\t\t(Verbose Mode)Switching over to Command Interpreter\n");
				strcpy(executeString,"./mainV ");
				system(executeString);
				break;
			case 5:
				printf("\n\t\t\tDeleting the Source db file and Reinstalling the RDBMS Software\n\t");
				system("rm rdbfile");
				strcpy(executeString,"./mainV ");
				system(executeString);	
				break;	
			case 6:
			        printf("\n\t\t\tTesting the DBEngine(VERBOSE MODE)");
				system("./dbtesterV ");
			        break;
			case 7:
				printf("\n\t\t\tTesting the Indexer");
				system("./VIM ");
				break;
			case 8:
				printf("\n\t\t\tTesting the BufferManager");	
				system("./VSB ");
				break;
			case 9:	
				printf("\n \t\t\tThank Q\n\t\t\tExiting....\n\n\n");		
				exit(0);
				break;
			default:
				break;			
			}
		}
	}	
   [Dq