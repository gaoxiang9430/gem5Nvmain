#include "mem/cache/cache_monitor.hh"



CacheMonitor::CacheMonitor(unsigned int number)
{
		numSets = number;
		lineArray = new Line[numSets];
		for(unsigned i=0;i<numSets;i++)
		{
			lineArray[i].pcm = 0;
			lineArray[i].dram = 0;
		}
}
CacheMonitor::~CacheMonitor()
{
	delete [] lineArray;
}
