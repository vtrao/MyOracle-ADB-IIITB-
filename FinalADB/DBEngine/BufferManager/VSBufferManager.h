#include "time.h"
#define RPLIFO 1
#define RPFIFO 2
#define RPLRU  3
#define RPACP  6
#define RPMRU  4
#define RPRANDOM  5

class DiskSpaceManagerLinux 
	{
	private:	
		static DiskSpaceManagerLinux* singleInstance_C;
		DiskSpaceManagerLinux(char *fileName);
		char *fileName_C;
	public:	
		static DiskSpaceManagerLinux* getInstance(char *fileName);
		int readPageFromHDD(int pageNumber,void *data);
		int writePageToHDD(int pageNumber,void *data);
		virtual ~DiskSpaceManagerLinux();
	};
	
typedef struct frame
        {
        int dBit;
        int pCount;
        int fNumber;
        int pNumber;
        int pity;
        int aCount;
        void *data;
        }Frame;

class FrameClass
	{
	private:
	        int dirty_Bit;
	        int pin_Count;
	        int frame_Number;
	        int page_Number;
	        unsigned int priority_c;
	        int access_Count;
	        void *data;
		time_t timeStamp;
		time_t lastTimeStamp;

	public:
	        FrameClass();
	        FrameClass(Frame f);
	        virtual ~FrameClass();
	        void setDirtyBit(int dBit);
	        void setPinCount(int pCount);
	        void setFrameNumber(int fNumber);
	        void setPageNumber(int pNumber);
		void setPriority(unsigned int pty);
	        void setAccessCount(int aCount);
	        void setData(void *rData);
		void setTimeStamp(time_t);
		void setLastTimeStamp(time_t);

	        int getDirtyBit();
	        int getPinCount();
	        int getFrameNumber();
	        int getPageNumber();
	        unsigned int getPriority();
	        int getAccessCount();
	        void *getData();
		time_t getTimeStamp();
		time_t getLastTimeStamp();
	};

class ReplacementPolicy
	{
	public:	
		virtual int replaceFrame(FrameClass *frames,int pageNumber,int numberOfFrames,
			         DiskSpaceManagerLinux *dsml)=0; //pure virtual function to make this class  abstract. 
	};
	

class LRU : public ReplacementPolicy
	{
	private:	
		static LRU *singleInstance_C;
		LRU();
	        int getLeastReferencedFrame(FrameClass *frames,int numberOfFrames);
	public:	
		static LRU* getInstance();
		int replaceFrame(FrameClass *frames,int pageNumber,int numberOfFrames,
					DiskSpaceManagerLinux *dsml);	
	};


class MRU : public ReplacementPolicy
	{
	private:	
		static MRU *singleInstance_C;
		MRU();
	        int getMostReferencedFrame(FrameClass *frames,int numberOfFrames);
	public:	
		static MRU* getInstance();
		int replaceFrame(FrameClass *frames,int pageNumber,int numberOfFrames,
					DiskSpaceManagerLinux *dsml);	
	};

class LIFO : public ReplacementPolicy
       {
       private:
	        static LIFO *singleInstance_C;
		LIFO();
		int getRecentTimeStampFrame(FrameClass *frames,int numberOfFrames);
       public: 		  
	        static LIFO* getInstance();
       		int replaceFrame(FrameClass *frames,int pageNumber,int numberOfFrames,
				                                            DiskSpaceManagerLinux *dsml);
       };

class FIFO : public ReplacementPolicy
       {
       private:
	       static FIFO *singleInstance_C;
	       FIFO();
	       int getLeastTimeStampFrame(FrameClass *frames,int numberOfFrames);
       public: 	
	        static FIFO* getInstance();
       		int replaceFrame(FrameClass *frames,int pageNumber,int numberOfFrames,
				                                            DiskSpaceManagerLinux *dsml);
       };

class RANDOM : public ReplacementPolicy
       {
       private:
	       static RANDOM *singleInstance_C;
	       RANDOM();
	       int getRandomFrame(FrameClass *frames,int numberOfFrames);
       public: 	
	        static RANDOM* getInstance();
       		int replaceFrame(FrameClass *frames,int pageNumber,int numberOfFrames,
				                                            DiskSpaceManagerLinux *dsml);
       };

class ACP : public ReplacementPolicy
       {
       private:
	       static ACP *singleInstance_C;
	       ACP();
	       int getLowPriorityFrame(FrameClass *frames,int numberOfFrames);
       public: 	
	        static ACP* getInstance();
       		int replaceFrame(FrameClass *frames,int pageNumber,int numberOfFrames,
				                                            DiskSpaceManagerLinux *dsml);
       };


class VSBufferManager
	{
	private:
		static VSBufferManager* singleInstance_C;
		ReplacementPolicy *replacement_Policy_C;
		DiskSpaceManagerLinux *dSML;
		FrameClass *frames_C; // Every pointer is assigned to a value 0
		int number_Of_Frames_C;
		int number_Of_Hits_C ;
		int number_Of_Requests_C;
		int numPagesRead_C;
		int numPagesWrite_C;
		int numDiskWrites_C;
		int numDiskReads_C;
		int numberOfDiskAccesses_C;
		int totNumPagesRead_C;
		int totNumPagesWritten_C;
		int searchBufferPool(int pageNumber); // returns SUCCESS or FAILED
		void getPageIntoBuffer(int pageNumber, int frameNumber);
		int getEmptyFrame();              //search for Empty Frame and return its num or FAILED
		int commitFrame(int frameNumber); //if dirty bit is set then commit the frame to HDD
		void incrementAccessCount(int frameNumber);
		int incrementPriority(int frameNumber);
		VSBufferManager();

	public:
		static VSBufferManager* getInstance();
		void queryStats();
	        void initializeBufferPool(int numberOfFrames,DiskSpaceManagerLinux *dsml,ReplacementPolicy *rp);
		int setPagePriority(int pageNumber,unsigned int priority);
		int readFromBufferPool(int pageNumber,void *data);
		int writeToBufferPool(int pageNumber,void *data);
//CommitMethods		
		int commitBufferPool();		
		int commitAndReleasePool();	
		int dontCommitAndReleasePool();
//Replacement Policy Methods	
		void setReplacementPolicy(ReplacementPolicy *replacementPolicy);		
		ReplacementPolicy* getReplacementPolicy();
		ReplacementPolicy* getRPFromFactory(int type);
//For debugging Purpose
		int displayBufferPool();
//For statistics Purpose to know the efficiency of ReplacementPolicy		
		void displayBufferStatisitics();
		int resetBufferPool(int numberOfFrames); //resets the total BufferPool and returns SUCCESS or FAILED defined in standards.h
		virtual ~VSBufferManager();
	};

 