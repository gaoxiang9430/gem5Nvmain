/*
 * Copyright (c) 2012-2013 ARM Limited
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2003-2005,2014 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Erik Hallnor
 */

/**
 * @file
 * Definitions of a HAPTEST tag store.
 */

#include "debug/CacheRepl.hh"
#include "mem/cache/tags/hap_test.hh"
#include "mem/cache/base.hh"
//light
#include "debug/HAPTEST.hh"
#include "base/bitfield.hh"
//end
HAPTEST::HAPTEST(const Params *p)
    : BaseSetAssoc(p),bitsOfSets(p->bitsOfSets)
{
//light
    //sc = new SC[5];
    flag = PolicyFlag::HAPTEST;
    totalSampleSet = (1 << (tagShift-setShift-bitsOfSets));
    sc = new SC_Test[totalSampleSet];
    //for(int i = 0;i < 5; i++)
    for(int i = 0;i < totalSampleSet; i++)
    {
        (*(sc+i)).access_counter = 0;
        (*(sc+i)).cost_counter = 0;
        (*(sc+i)).pcmLineCounter = 0;
    }
    minPcmLineCounter = 0 * (numBlocks - 5 * (pow(2,bitsOfSets))* 16);
    maxPcmLineCounter = 1 * (numBlocks - 5 * (pow(2,bitsOfSets)) * 16);
    currentPcmLineCounter = 0;
//end
}
//light
HAPTEST::~HAPTEST()
{
    delete [] sc;
}
//end
BaseSetAssoc::BlkType*
HAPTEST::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id,MemCmd cmd,int portId)
{
    BlkType *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id);
//light
    int set = extractSet(addr);
    int low = bits(set,tagShift-setShift-(bitsOfSets+1),0);
    //if(low <=4)
    if(warmedUp)
        (*(sc+low)).access_counter++;
  
//end
    if (blk != NULL) {
        // move this block to head of the MRU list
        sets[blk->set].moveToHead(blk);
        DPRINTF(CacheRepl, "set %x: moving blk %x (%s) to MRU\n",
                blk->set, regenerateBlkAddr(blk->tag, blk->set),
                is_secure ? "s" : "ns");
    }
    else
    {
      //light
        //if(low <=4)
        //{
        /*if(warmedUp)
        {
            if(addr >= PhysicalMemory::watershed)
                (*(sc+low)).cost_counter++;
            else
                (*(sc+low)).cost_counter+=6;
        }*/
        //}
    //end
    }

    return blk;
}

BaseSetAssoc::BlkType*
HAPTEST::findVictim(Addr addr) const
{
    int set = extractSet(addr);
    // grab a replacement candidate
//light
    BlkType *blk = sets[set].blks[assoc-1];
if(warmedUp)
{
    if(blk->isValid())
    {
            int low = bits(set,tagShift-setShift-(bitsOfSets+1),0);
            //if((low > 4 && currentPcmLineCounter > maxPcmLineCounter) || (low <=4 && (*(sc+low)).pcmLineCounter > (low * 0.25 * 16*(pow(2,bitsOfSets)))))
                       //else if((low > 4 && currentPcmLineCounter < minPcmLineCounter) || ((low <=4 )&& (*(sc+low)).pcmLineCounter<(low*0.25*16*(pow(2,bitsOfSets)))))
            if(((*(sc+low)).pcmLineCounter) > ((low % 5) * 0.25 * 16 *(pow(2,bitsOfSets))))
            {
                //DPRINTFR(HAPTEST,"Evic Pcm,low:%d,percent:%d\n",low,(low%5));
                if(addr < PhysicalMemory::watershed)
                {
                    //BlkType *tempBlk = NULL;
                    for(int j = (assoc - 1); j >=0; j--)
                    {
                        if((sets[set].blks[j])->isPcm)
                        {
                            blk = sets[set].blks[j];
                            break;
                        }
                    }          
                }
            }
            /*else if(((*(sc+low)).pcmLineCounter) < ((low % 5) * 0.25 * 16 *(pow(2,bitsOfSets))))
            {
                //DPRINTFR(HAPTEST,"Evic Dram,low:%d,percent:%d\n",low,(low%5));
                if(addr >= PhysicalMemory::watershed)
                {
                    for(int j =(assoc-1);j>=0;j--)
                    {
                        if(!((sets[set].blks[j])->isPcm))
                        {
                            blk = sets[set].blks[j];
                            break;
                        }
                    }
                }
            }*/   
       DPRINTFR(CacheRepl, "set %x: selecting blk %x for replacement\n",
                set, regenerateBlkAddr(blk->tag, set));
    }
}
    return blk;
}
//light
void
HAPTEST::decPcmLineCounter(unsigned numSet)
{
    int low = bits(numSet,tagShift-setShift-(bitsOfSets+1),0);
    //if(low < 5)
    //{
            (*(sc+low)).pcmLineCounter--;
    //}

 
            currentPcmLineCounter--;
   
}
void
HAPTEST::incPcmLineCounter(unsigned numSet)
{
    int low = bits(numSet,tagShift-setShift-(bitsOfSets+1),0);
    //if(low < 5)
    //{
            (*(sc+low)).pcmLineCounter++;
    //}
    //else
    //{
            currentPcmLineCounter++;
    //}
}
//end
void
HAPTEST::insertBlock(PacketPtr pkt, BlkType *blk)
{
//light,copy from base_set_assoc.hh;
      if(warmedUp && pkt->cmd !=MemCmd::Writeback)
      {
      	int low = bits(blk->set,tagShift-setShift-(bitsOfSets+1),0);
        if(low <=4)
        {
            if(pkt->getAddr() >= PhysicalMemory::watershed)
                (*(sc+low)).cost_counter++;
            else
                (*(sc+low)).cost_counter+=6;
        }
      }
         Addr addr = pkt->getAddr();
         MasterID master_id = pkt->req->masterId();
         uint32_t task_id = pkt->req->taskId();

         if (!blk->isTouched) {
             tagsInUse++;
             blk->isTouched = true;
             /*if (!warmedUp && tagsInUse.value() >= warmupBound) {
                 warmedUp = true;
                 warmupCycle = curTick();
             }*/
         }

         // If we're replacing a block that was previously valid update
         // stats for it. This can't be done in findBlock() because a
         // found block might not actually be replaced there if the
         // coherence protocol says it can't be.
         if (blk->isValid()) {
             replacements[0]++;
             totalRefs += blk->refCount;
             ++sampledRefs;
             blk->refCount = 0;

             // deal with evicted block
             assert(blk->srcMasterId < cache->system->maxMasters());
             occupancies[blk->srcMasterId]--;
//light
decSetLine(blk->set,blk->isPcm);
if(blk->isPcm)
{
    decPcmLineCounter(blk->set);
}
//end
             blk->invalidate();
         }
//light
if(addr >= PhysicalMemory::watershed)
{
	blk->isPcm=false;
}
else
{
	blk->isPcm=true;
	incPcmLineCounter(blk->set);
}

incSetLine(blk->set,blk->isPcm);
//end
         blk->isTouched = true;

         // Set tag for new block.  Caller is responsible for setting status.
         blk->tag = extractTag(addr);

         // deal with what we are bringing in
         assert(master_id < cache->system->maxMasters());
         occupancies[master_id]++;
         blk->srcMasterId = master_id;
         blk->task_id = task_id;
         blk->tickInserted = curTick();

         // We only need to write into one tag and one data block.
         tagAccesses += 1;
         dataAccesses += 1;

    int set = extractSet(addr);
    sets[set].moveToHead(blk);
}
//light
void
HAPTEST::updateSomething()
{
    double tmpki[5];
    //int min = 0,max = 0,temp = 0;
    int first = 0, second = 1,temp = 0;
    //int group = totalSampleSet / 5;
   //bool isFirstAround = true,isExchange = false;
    //for(int i=0;i<5;i++)
    for(int i = 0;i<totalSampleSet;i++)
    {
        int index = i % 5;
        
        if(i !=0 && i % 5 == 0)
        {
           /*DPRINTFR(HAPTEST,"-----------------------\n");
           for(int j = 0; j < 5; j++)
           {
            DPRINTFR(HAPTEST,"%d %lf\n",j,tmpki[j]);
           }
            DPRINTFR(HAPTEST,"-----------------------\n");*/
                first = 0; second = 1;
                if(tmpki[1] < tmpki[0])
                {
                    first = 1;
                    second = 0;
                }
                for(int k = 2; k < 5;k++)
                {
                        if(tmpki[k] < tmpki[first])
                        {
                               second = first;
                                first = k;
                        }
                        else if(tmpki[k] < tmpki[second])
                        {
                                second = k;
                        }
                }
                if(first > second)
                {
                    temp = first;
                    first = second;
                    second = temp;
                }
                DPRINTFR(HAPTEST,"%d %d\n",first,second);
        }
        
        if((*(sc+i)).access_counter == 0)
        {
            assert((*(sc+i)).cost_counter == 0);
            tmpki[index] = 0;
        }
        else
        {
            tmpki[index] = ((double)((*(sc+i)).cost_counter)) / ((*(sc+i)).access_counter); 
            (*(sc+i)).cost_counter = 0;//reset;
            (*(sc+i)).access_counter = 0;//reset;
        }
        
      
                    
    }
    //DRPINTFR(HAPTEST,"************************");
    //printSetLine();
    /*if(tmpki[1] < tmpki[0])
    {
        first = 1;
        second = 0;
    }
    for(int i = 2; i < 5;i++)
    {
            if(tmpki[i] < tmpki[first])
            {
                   second = first;
                    first = i;
            }
            else if(tmpki[i] < tmpki[second])
            {
                    second = i;
            }
    }
    if(first > second)
    {
        temp = first;
        first = second;
        second = temp;
    }
minPcmLineCounter=0.25 * (numBlocks - 5 * (pow(2,bitsOfSets)) * 16) * first;
maxPcmLineCounter=0.25 * (numBlocks - 5 * (pow(2,bitsOfSets)) * 16) * second;
DPRINTFR(HAPTEST,"after update |  low: %d,high:%d\n",first,second);*/
}
//end
void
HAPTEST::invalidate(BlkType *blk)
{
//light
if(blk->isPcm)
    decPcmLineCounter(blk->set);
//end
    BaseSetAssoc::invalidate(blk);

    // should be evicted before valid blocks
    int set = blk->set;
    sets[set].moveToTail(blk);
}

HAPTEST*
HAPTESTParams::create()
{
    return new HAPTEST(this);
}
