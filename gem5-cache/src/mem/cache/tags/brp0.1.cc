/*
 * Authors: Erik Hallnor
 * Time: 2015/10/31
 */

/**
 * @file
 * Definitions of a BRP tag store.
 */
#include "debug/CacheRepl.hh"
#include "mem/cache/tags/brp.hh"
#include "mem/cache/base.hh"

BRP::BRP(const Params *p)
    : BaseSetAssoc(p)
{
    flag = PolicyFlag::BRP;
    
    number_set = numSets;
    access_Time = 0; //inialize the access time to 0
    
    DRAM_sets = new SetType[numSets]; //create a cache line emulator for NVM and DRAM respectively
    NVM_sets = new SetType[numSets];
    
    blks = new BlkType[2*numSets * assoc];

	SetType * Temp_sets = DRAM_sets;
   	int blkIndex = 0;
   	for(int k=0;k<2;k++)
   	{
		for (unsigned i = 0; i < numSets; ++i) {
		    Temp_sets[i].assoc = assoc;

			Temp_sets[i].blks = new BlkType*[assoc];

			for (unsigned j = 0; j < assoc; ++j) {
			  
				BlkType *blk = &blks[blkIndex];
		        ++blkIndex;

				blk->tag = 0;
				blk->asid = 0;
				
				blk->status = 0;  //present the block is clean
				
				Temp_sets[i].blks[j]=blk;
			}
		}
		Temp_sets = NVM_sets;
	}
}

BaseSetAssoc::BlkType*
BRP::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id,MemCmd cmd)
{
    BlkType *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id);
	record_brp(addr,cmd);
    if (blk != NULL) {
        // move this block to head of the MRU list
        if(cmd != MemCmd::Writeback)
        	sets[blk->set].moveToHead(blk);
        DPRINTF(CacheRepl, "xiang_set %x: moving blk %x (%s) to MRU\n",
                blk->set, regenerateBlkAddr(blk->tag, blk->set),
                is_secure ? "s" : "ns");
    }

    return blk;
}

//xiang
void 
BRP::record_brp(Addr addr,MemCmd cmd)
{
	Addr tag = extractTag(addr);
	int set = extractSet(addr);
	
	SetType *Temp_sets;
	if(addr >= PhysicalMemory::watershed) 
	{
		Temp_sets = DRAM_sets;  //the memory accessed is DRAM
	}
	else
	{
		Temp_sets = NVM_sets;   //the memory accessed is NVM
	}
	
	access_Time ++;
	if(access_Time==400)
	{
		DPRINTF(CacheRepl, "\nrecord the each cache line hit times:\n");
		for(int i=0;i<number_set;i++)
		{
			char tmp[100];
			char tmp2[5];
			tmp[0]='\0';
			for(int j=0;j<assoc;j++)
			{
				sprintf(tmp2,"%d ",DRAM_sets[i].blks[j]->asid);
				strcat(tmp,tmp2);
				DRAM_sets[i].blks[j]->asid = 0;
			}
			DPRINTF(CacheRepl, "DRAM set %d : %s\n",i,tmp);
			
			tmp[0]='\0';
			for(int j=0;j<assoc;j++)
			{
				sprintf(tmp2,"%d ",NVM_sets[i].blks[j]->asid);
				strcat(tmp,tmp2);
				NVM_sets[i].blks[j]->asid = 0;
			}
			DPRINTF(CacheRepl, "NVM set %d : %s\n",i,tmp);
		}
        access_Time = 0;
	}
	
	for(unsigned i=0;i<assoc;i++)
	{
		if(Temp_sets[set].blks[i]->tag == tag)  //hit the emulator cache line when access cache lines
		{
			if(cmd == MemCmd::Writeback)
				Temp_sets[set].blks[i]->status = 1;	//set this block as dirty
			Temp_sets[set].blks[i]->asid ++;
			MoveTagHead(Temp_sets, set, i);
			return;
		}
	}
	//miss the emulator cache line when access cache lines
	if(cmd == MemCmd::Writeback)
		Temp_sets[set].blks[assoc-1]->status = 1;  	//set this block as dirty
	else
		Temp_sets[set].blks[assoc-1]->status = 0;	
	
	Temp_sets[set].blks[assoc-1]->tag = tag;
	Temp_sets[set].blks[assoc-1]->asid = 0;
	MoveTagHead(Temp_sets,set,assoc-1);
	
	
}

void 
BRP::MoveTagHead(SetType *Temp_sets,int set_index, int index)
{
	SetType Move_Set = Temp_sets[set_index];
	if(index == 0)
		return;
	
	Addr Move_tag = Move_Set.blks[index]->tag;
	unsigned int Move_status = Move_Set.blks[index]->status;
	
	for(unsigned i=index;i>0;i--)
	{
		Move_Set.blks[index]->tag = Move_Set.blks[index-1]->tag;
		Move_Set.blks[index]->status = Move_Set.blks[index-1]->status;
	}
	Move_Set.blks[0]->tag = Move_tag;
	Move_Set.blks[0]->status = Move_status;
	return;
}
//end

BaseSetAssoc::BlkType*
BRP::findVictim(Addr addr) const
{
    int set = extractSet(addr);
    // grab a replacement candidate
    BlkType *blk = sets[set].blks[assoc - 1];

    if (blk->isValid()) {
        DPRINTF(CacheRepl, "xiang_set %x: selecting blk %x for replacement\n",
                set, regenerateBlkAddr(blk->tag, set));
    }

    return blk;
}

void
BRP::insertBlock(PacketPtr pkt, BlkType *blk)
{

    BaseSetAssoc::insertBlock(pkt, blk);

    int set = extractSet(pkt->getAddr());

    sets[set].moveToHead(blk);
}

void
BRP::invalidate(BlkType *blk)
{
    BaseSetAssoc::invalidate(blk);

    // should be evicted before valid blocks
    int set = blk->set;
    sets[set].moveToTail(blk);
}

BRP*
BRPParams::create()
{
    return new BRP(this);
}
