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
 * Definitions of a HYBBF tag store.
 */

#include "debug/CacheRepl.hh"
#include "mem/cache/tags/hybbf.hh"
#include "mem/cache/base.hh"
//light
#include "debug/HYBBF.hh"
#include "debug/BloomFilter.hh"
//end

HYBBF::HYBBF(const Params *p)
    : BaseSetAssoc(p),counter(p->counter),numBits(p->numbits)
{
	setbloomfilter = new HSetBloomFilter*[numSets];
    for (unsigned i = 0; i < numSets; ++i) {
        setbloomfilter[i] = new HSetBloomFilter(counter,numBits,(32 - tagShift));
    }
    flag = PolicyFlag::HYBBF;
}
//light
HYBBF::~HYBBF()
{

    for(unsigned i = 0;i<numSets;++i)
    {
        delete setbloomfilter[i];

    }
    delete [] setbloomfilter;
}
//end
BaseSetAssoc::BlkType*
HYBBF::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id,MemCmd cmd,int portId)
{
    BlkType *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id);

    if (blk != NULL) {
        // move this block to head of the MRU list
        if(warmedUp)
        {
		    if(!(blk->isPcm))
		    {
		        //sets[blk->set].moveOneStep(blk);
		        sets[blk->set].moveNStep(blk,3);
		    }
		    else
		    {
		        sets[blk->set].moveToHead(blk);
		    }
		}
		else
		{
			sets[blk->set].moveToHead(blk);
		}
	/*if(cmd = MemCmd::WriteBack)
	{
		sets[blk->set].moveOneStep(blk);
	}*/
        DPRINTF(CacheRepl, "set %x: moving blk %x (%s) to MRU\n",
                blk->set, regenerateBlkAddr(blk->tag, blk->set),
                is_secure ? "s" : "ns");
    }

    return blk;
}

BaseSetAssoc::BlkType*
HYBBF::findVictim(Addr addr) const
{
    int set = extractSet(addr);
    // grab a replacement candidate
    BlkType *blk = sets[set].blks[assoc - 1];

    if (blk->isValid()) {
        DPRINTF(CacheRepl, "set %x: selecting blk %x for replacement\n",
                set, regenerateBlkAddr(blk->tag, set));
    }

    return blk;
}

void
HYBBF::insertBlock(PacketPtr pkt, BlkType *blk)
{
    

    int set = extractSet(pkt->getAddr());
    //light
    if(warmedUp)
    {
		if(blk->isValid() & (!blk->isPcm))
   		{
			DPRINTF(BloomFilter,"insertBF | set:%x ,tag:%x\n",blk->set,blk->tag);
        	insertBloomFilter(blk->set,blk->tag);
    	}
    	BaseSetAssoc::insertBlock(pkt, blk);
    	if( (blk->isPcm))
        {
            DPRINTF(BloomFilter,"hit or Pcm\n");

            sets[set].moveToHead(blk);
        }
        else if(testBloomFilter(set,extractTag(pkt->getAddr())))
        {
            DPRINTF(BloomFilter,"hit\n");
            sets[set].insertFixLocation(blk,3);
        }
        else
        {
            DPRINTF(BloomFilter,"not hit\n");
            sets[set].insertFixLocation(blk,7);
        }
    }
    else
    {
    	BaseSetAssoc::insertBlock(pkt, blk);
        sets[set].moveToHead(blk);
    }
    //end
    //sets[set].moveToHead(blk);
}

void
HYBBF::invalidate(BlkType *blk)
{
    BaseSetAssoc::invalidate(blk);

    // should be evicted before valid blocks
    int set = blk->set;
    sets[set].moveToTail(blk);
}

HYBBF*
HYBBFParams::create()
{
    return new HYBBF(this);
}
