/**
 * @file
 * Declaration of a BRP tag store.
 * The BRP tags guarantee that the partitions occupied by NVM and DRAM is best for performance.
 */

#ifndef __MEM_CACHE_TAGS_BRP_HH__
#define __MEM_CACHE_TAGS_BRP_HH__

#include "mem/cache/tags/base_set_assoc.hh"
#include "params/BRP.hh"

#define NUM_CORE 4
class BRP : public BaseSetAssoc
{
  public:
    /** Convenience typedef. */
    typedef BRPParams Params;

    /**
     * Construct and initialize this tag store.
     */
    BRP(const Params *p);

    /**
     * Destructor
     */
    ~BRP() {}

    BlkType* accessBlock(Addr addr, bool is_secure, Cycles &lat,
                         int context_src,MemCmd cmd,int portId);
    BlkType* findVictim(Addr addr) const;
    void insertBlock(PacketPtr pkt, BlkType *blk);
    void invalidate(BlkType *blk);
    void setRecord();
    void find_best(int set_index,int maxRatioIndex);
    void MoveTagHead(SetType *Temp_sets,int set_index, int index);
	void MoveTagTail(SetType *Temp_sets,int set_index, int index);
  private:
  	void record_brp(Addr addr,MemCmd cmd,int portId);
  	int subBest(int setIndex,int level, int tempBest, int remain);
  	
  	SetType** record_sets;
	int number_set;
	int access[NUM_CORE];
	int hits[NUM_CORE];
	int isStream[NUM_CORE];
	
  	int** save_overhead;
  	int hit;
  	int best;
  	int **core;
  	
  	int core_sampling[NUM_CORE];
  	int skip_set;
  	
  	int **accessPCMTimes;
  	int **accessDRAMTimes;
  	int accessPCM[NUM_CORE];
  	int accessDRAM[NUM_CORE];
};

#endif // __MEM_CACHE_TAGS_LRU_HH__
