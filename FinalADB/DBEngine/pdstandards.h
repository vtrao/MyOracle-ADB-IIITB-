//*****************Record level Queries identification terms

#define SELECTQUERY          1
#define SELECTWHERE          101
#define DELETEQUERY          2
#define DELETEWHERE          102
#define DELETESPECIALCASE    103 //for Drop table thing
#define UPDATEQUERY          3
#define INSERTQUERY          4

//*****************Table  level Queries identification terms
#define CREATETABLE          5
#define DROPTABLE            6
#define ALTERTABLE           7
#define SHOWTABLE            123
#define RENAMETABLE          124 //to be

#define CREATESPECIALCASE    104
#define USETABLE    0
//*****************Database level Queries identification terms
#define CREATEDATABASE       8
#define DROPDATABASE         9
#define USEDATABASE          10
#define SHOWDATABASE         11
#define SHOWD	             15
#define CHECKDB	             20
#define RENAMEDATABASE       12 //to be

//*****************Miscellaneous Standard identification terms
#define ORCONDITION          20
#define ANDCONDITION         21
#define INTTYPE              22
#define VARCHARTYPE          23
#define DATETYPE             24
#define ALTERADD             25
#define ALTERDROP            26
#define ALTERMODIFY          27
#define NO                  'n' 
#define YES                 'y'
//we can delete the following entries***********************##########################
#define CDTYPE              "char" 
#define IDTYPE              "int"
#define VDTYPE              "void"
#define DDTYPE		    "date"	
#define DISPLAY             12
#define CHECK               10
#define RET                 121
//*****************Standard RLQ breakdown struture******************
typedef struct AttributeValues
	{
	char* value;
	AttributeValues *next;
	};

typedef struct FieldNames
	{
	char* name;
	FieldNames *next;
	};	

typedef struct RLQds
	{
	char *dbname;        //added fi
	char *objectName;
        int queryType;
	AttributeValues* values;
	FieldNames* names;
	};

//*****************Standard TLQ breakdown struture******************
typedef struct createColumns 
	{
	char *columnName;
	int dataType; //types editted fi
	int attLen;
	int precision;
	char *defaultValue; 
	int position;
	int primaryKey; //YES or NO
	int notNull;    //YES or NO
	int unique;     //YES or NO 
	createColumns *next;
	};	

typedef struct TLQds
	{
	char *dbname;         //added fi
	char *objectName;
	int queryType;
	int addOrDeleteOrModify;
	createColumns *columns;
	FieldNames *uniqueFields;
	FieldNames *pKeyFields;
	};

//*****************Standard DLQ breakdown struture******************
typedef struct DLQds
	{
	char *objectName;
        int queryType;
	};	
 