VSB : VSBufferManager.o DiskSpaceManagerLinux.o FrameClass.o LIFO.o FIFO.o LRU.o MRU.o RANDOM.o ACP.o standards.o
	g++ -g standards.o FrameClass.o LIFO.o FIFO.o LRU.o MRU.o RANDOM.o ACP.o DiskSpaceManagerLinux.o VSBufferManager.o -o VSB
VSBufferManager.o:  VSBufferManager.cpp	
	g++ -c VSBufferManager.cpp
DiskSpaceManagerLinux.o : DiskSpaceManagerLinux.cpp
	g++ -c DiskSpaceManagerLinux.cpp
FrameClass.o: FrameClass.cpp
	g++ -c FrameClass.cpp
LIFO.o : LIFO.cpp
	g++ -c LIFO.cpp
FIFO.o : FIFO.cpp
	g++ -c FIFO.cpp
LRU.o : LRU.cpp
	g++ -c LRU.cpp
MRU.o : MRU.cpp
	g++ -c MRU.cpp
RANDOM.o : RANDOM.cpp
	g++ -c RANDOM.cpp
ACP.o : ACP.cpp
	g++ -c ACP.cpp
standards.o : standards.cpp standards.h
	g++ -c standards.cpp
	
	
	
	
  �