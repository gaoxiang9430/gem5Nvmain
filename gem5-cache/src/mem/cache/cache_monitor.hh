#include <stdint.h>
#ifndef __CACHEMONITOR_HH__
#define __CACHEMONITOR_HH__
typedef struct 
	{
		uint8_t pcm;
		uint8_t dram;
	}Line;
class CacheMonitor
{
friend class CacheBlk;
friend class BaseSetAssoc;

public:
static Line *lineArray;
	unsigned int  numSets;
	
public:
	CacheMonitor(unsigned int number);

 	virtual ~CacheMonitor();

};
#endif
