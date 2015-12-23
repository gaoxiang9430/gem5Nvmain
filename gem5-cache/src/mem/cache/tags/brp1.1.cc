/*
 * Authors: Erik Hallnor
 * Time: 2015/10/31
 */

/**
 * @file
 * Definitions of a BRP tag store.
 */
#include "debug/HAP.hh"
#include "mem/cache/tags/brp.hh"
#include "mem/cache/base.hh"

#define NVM_r_e 78     //define the energy consumption of NVM/DRAM operation
#define NVM_w_e 773
#define DRAM_r_e 210
#define DRAM_w_e 195

#define NVM_r_t 75	   //define the latency of NVM/DRAM operation
#define NVM_w_t 1000
#define DRAM_r_t 50
#define DRAM_w_t 35

BRP::BRP(const Params *p)
    : BaseSetAssoc(p)
{
    flag = PolicyFlag::BRP;
    
    number_set = numSets;
    
    hit = 0;
    core = new int*[number_set];
    for(int i=0;i<number_set;i++)
    {
    	core[i] = new int[NUM_CORE];
    	for(int j=0;j<NUM_CORE;j++)
    		core[i][j]=assoc/4;
    }
	for(int i=0;i<NUM_CORE;i++)
	{
		access[i] = 0;
		hits[i] = 0;
	}
	
	save_overhead = new int*[NUM_CORE];
	for(int i=0;i<NUM_CORE;i++)
	{
		save_overhead[i] = new int[assoc];
		for(int j=0;j<assoc;j++)
			save_overhead[i][j]=0;
	}
	
    record_sets = new SetType*[NUM_CORE];
    for(int i=0;i<NUM_CORE;i++)
	    record_sets[i] = new SetType[number_set]; //create a cache line emulator for each core
    
    blks = new BlkType[NUM_CORE*number_set * assoc];

   	int blkIndex = 0;
   	for(int k=0;k<NUM_CORE;k++)
   	{
   		SetType * Temp_sets = record_sets[k];
		for (unsigned i = 0; i < number_set; i++) {
		    Temp_sets[i].assoc = assoc;

			Temp_sets[i].blks = new BlkType*[assoc];

			for (unsigned j = 0; j < assoc; j++) {
			  
				BlkType *blk = &blks[blkIndex];
		        ++blkIndex;

				blk->tag = -1;
				blk->asid = 0;
				blk->asid_dirty = 0;
				
				blk->status = 0;  //present the block is clean
				
				Temp_sets[i].blks[j]=blk;
			}
		}
	}
}

//xiang
void 
BRP::record_brp(Addr addr,MemCmd cmd,int portId)
{
	Addr tag = extractTag(addr);
	int set = extractSet(addr);
	
	SetType *Temp_sets;
	Temp_sets = record_sets[portId];

	for(unsigned i=0;i<assoc;i++)
	{
		if(Temp_sets[set].blks[i]->tag == tag)  //hit the emulator cache line when access cache lines
		{
			hit++;
			if(Temp_sets[set].blks[i]->status)		
				Temp_sets[set].blks[i]->asid_dirty ++;
			else
				Temp_sets[set].blks[i]->asid ++;
		
			if(cmd == MemCmd::Writeback)
				Temp_sets[set].blks[i]->status = 1;	//set this block as dirty		
			
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
	
	//Temp_sets[set].blks[assoc-1]->asid = 0; // reset the couter of the clean and dirty line
	//Temp_sets[set].blks[assoc-1]->asid_dirty = 0;
	MoveTagHead(Temp_sets,set,assoc-1);
	
}

//find the best partition strategy
int 
BRP::subBest(int setIndex, int level, int tempBest, int remain)
{
	int res,ret=0;
	
	if(remain==0 || level>=NUM_CORE)
	{
		if(tempBest>best)
		{
			if(remain==0 && level<NUM_CORE)
			{
				for(int i=level;i<NUM_CORE;i++)
					core[setIndex][i] = 0;
			}
			best = tempBest;
			return 1;
		}
		return 0;
	}
	
	res = subBest(setIndex, level+1,tempBest,remain);
	ret = res+ret;
	if(res)
	{
		res = 0;
		core[setIndex][level] = 0;
	}
	
	for(int i=0;i<remain;i++)
	{
		if(remain-i-1>=0)
		{
			res = subBest(setIndex,level+1,tempBest+save_overhead[level][i],remain-i-1);
			ret = res+ret;
			if(res)
			{
				res = 0;
				core[setIndex][level] = i+1;
			}
		}
	}
	return ret;
}

void 
BRP::find_best(int set_index,int maxRatioIndex)
{
	best=0;
	/*for(int i=0;i<NUM_CORE;i++)   
	{
		if(isStream[i])
		{
			for(int j=0;j<assoc;j++)
				save_overhead[i][j] = 0;
		}
	}*/
	subBest(set_index,0,0,assoc);
	int remain = assoc;
	for(int i=0;i<NUM_CORE;i++)
		remain = remain-core[set_index][i];
	if(remain>0)
		core[set_index][maxRatioIndex] += remain;
	//if the NUM_CORE change, you should modify the code here
	DPRINTF(HAP, "set %d: %d %d %d %d\n",set_index,core[set_index][0],core[set_index][1],core[set_index][2],core[set_index][3]);
}

void 
BRP::MoveTagHead(SetType *Temp_sets,int set_index, int index) //move the index line of set_index set in Tempsets to the head(0 position)
{
	SetType Move_Set = Temp_sets[set_index];
	if(index == 0)
		return;
	
	Addr Move_tag = Move_Set.blks[index]->tag;                //record current tag and statue(dirty or not)
	unsigned int Move_status = Move_Set.blks[index]->status;
	
	for(unsigned i=index;i>0;i--)                             //move operation
	{
		Move_Set.blks[i]->tag = Move_Set.blks[i-1]->tag;
		Move_Set.blks[i]->status = Move_Set.blks[i-1]->status;
	}
	Move_Set.blks[0]->tag = Move_tag;
	Move_Set.blks[0]->status = Move_status;
	return;
}

BaseSetAssoc::BlkType*
BRP::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id,MemCmd cmd,int portId)
{
    BlkType *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id);
	record_brp(addr,cmd,portId);  //call emulator cache to collect access information
	access[portId]++;             //record access times
    if (blk != NULL) {
        // move this block to head of the MRU list
        //if(cmd != MemCmd::Writeback)
        if(blk->isPcm)
	        sets[blk->set].moveNStep(blk,core[blk->set][portId]/2+1);
	        //sets[blk->set].moveToHead(blk);
    	else
        	sets[blk->set].moveNStep(blk,core[blk->set][portId]/4+1);
        
        hits[portId]++;       //record hit times
    }
	//DPRINTF(HAP, "port %d visit %x\n", portId, addr);
    return blk;
}

//xiang
void 
BRP::setRecord()                  //cpu will call this function every 50M instructions
{
	int allStream = 0,maxRatioIndex = 0;
	double maxRatio = 0;
	for(int i=0;i<NUM_CORE;i++)
	{
		double temp_ratio=0;
		if(access[i]>0)
			temp_ratio = (double)hits[i]/access[i];
		DPRINTF(HAP, "core %d: access %d, hits %d, hit rate:%f\n", i,access[i],hits[i],temp_ratio);
		if(temp_ratio < 0.125)  //determain whether this benchmark is stream or not
			isStream[i] = 1;
		else isStream[i] = 0;
		if(temp_ratio > maxRatio)
		{
			maxRatio = temp_ratio;
			maxRatioIndex = i;
		}
		allStream += isStream[i]; 
		access[i] = 0;
		hits[i] = 0;
	}
	if(allStream==NUM_CORE) //if all the benchmark is stream, we will not distinguish stream
	{
		for(int i=0;i<NUM_CORE;i++)
			isStream[i] = 0;
	}
	DPRINTF(HAP, "hit :  %d\n",hit); // print the hit ratio
	hit = 0;
	
	char tmp[200];
	char tmp2[10];
	
	SetType *Temp_sets;
	for(int i=0;i<number_set;i++)
	{
		for(int k=0;k<NUM_CORE;k++)
		{
			tmp[0]='\0';
			Temp_sets = record_sets[k];
			for(int j=0;j<assoc;j++)
			{
				//sprintf(tmp2,"%d %d ",Temp_sets[i].blks[j]->asid,Temp_sets[i].blks[j]->asid_dirty);
				//unsigned int ttt = (unsigned int)Temp_sets[i].blks[j]->tag;
				//sprintf(tmp2,"%x ",ttt);
				
				//calculator the total over head if the cache was ejected
				//save_overhead[k][j] = Temp_sets[i].blks[j]->asid * DRAM_r_e + Temp_sets[i].blks[j]->asid_dirty * (DRAM_r_e + DRAM_w_e);
				save_overhead[k][j] = Temp_sets[i].blks[j]->asid+Temp_sets[i].blks[j]->asid_dirty;   //calculat the cost saved by hit for each set
				if(j>0)
					save_overhead[k][j] += save_overhead[k][j-1];
				sprintf(tmp2,"%d ",save_overhead[k][j]);
				Temp_sets[i].blks[j]->asid = 0;
				Temp_sets[i].blks[j]->asid_dirty = 0;
				strcat(tmp,tmp2);
			}
			DPRINTF(HAP, "raw data core %d set %d : %s\n",k,i,tmp);
		}
		find_best(i,maxRatioIndex);  //find the best partition strategy
	}
	
}    
//end

BaseSetAssoc::BlkType*
BRP::findVictim(Addr addr) const
{
    int set = extractSet(addr);
    // grab a replacement candidate
    BlkType *blk = sets[set].blks[assoc - 1];

    //if (blk->isValid()) {
        //DPRINTF(HAP, "xiang_set %x: selecting blk %x for replacement\n",set, regenerateBlkAddr(blk->tag, set));
    //}

    return blk;
}

void
BRP::insertBlock(PacketPtr pkt, BlkType *blk)
{
    BaseSetAssoc::insertBlock(pkt, blk);

    int set = extractSet(pkt->getAddr());
    
        /*	if(pkt->cmd == MemCmd::Writeback)
        	{
            	sets[set].moveToHead(blk);
           	}
           	else
           	{
           		//unsigned char location = bits(setLocation[blk->set],7,4);
        		//assert(location<assoc);
            	//sets[set].insertFixLocation(blk,location);
            	sets[set].insertFixLocation(blk,assoc>>1);
           	}*/
//xiang
    int portId = (pkt->req->masterId()-7)/6;  //distinguish the access belongs to whick core
    //MemCmd cmd = pkt->cmd;
    int position = 0;
    if(blk->isPcm)
    	position = (core[set][portId]+4 > assoc-1)?assoc-1:core[set][portId]+4;
    else
    	position = (core[set][portId]-2>0)?core[set][portId]-2:0;
    sets[set].insertFixLocation(blk,assoc-1-position);
//end
    //sets[set].moveToHead(blk);
}

void
BRP::invalidate(BlkType *blk)
{
    BaseSetAssoc::invalidate(blk);

    // should be evicted before valid blocks
    int set = blk->set;
    sets[set].moveToTail(blk);
    
    for(int j=0;j<NUM_CORE;j++)
    {	
    	SetType *Temp_sets = record_sets[j];
		for(int i=0;i<assoc;i++)
		{
			if(Temp_sets[set].blks[i]->tag == blk->tag)
			{
				Temp_sets[set].blks[i]->tag=-1;
				MoveTagTail(Temp_sets,set,i);
				return;
			}
		}
	}
}

void 
BRP::MoveTagTail(SetType *Temp_sets,int set_index, int index)
{
	SetType Move_Set = Temp_sets[set_index];
	if(index == 0)
		return;
	
	Addr Move_tag = Move_Set.blks[index]->tag;
	unsigned int Move_status = Move_Set.blks[index]->status;
	
	for(unsigned i=index;i<assoc-1;i++)
	{
		Move_Set.blks[i]->tag = Move_Set.blks[i+1]->tag;
		Move_Set.blks[i]->status = Move_Set.blks[i+1]->status;
	}
	Move_Set.blks[assoc-1]->tag = Move_tag;
	Move_Set.blks[assoc-1]->status = Move_status;
	return;
}

BRP*
BRPParams::create()
{
    return new BRP(this);
}
