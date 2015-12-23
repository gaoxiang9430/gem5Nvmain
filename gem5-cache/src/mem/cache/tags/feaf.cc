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
 * Definitions of a FEAF tag store.
 */

#include "debug/CacheRepl.hh"
#include "mem/cache/tags/feaf.hh"
#include "mem/cache/base.hh"

FEAF::FEAF(const Params *p)
    : BaseSetAssoc(p)
{
    setbloomfilter = new SetBloomFilter[numSets];
    totalCounter = 0;
    flag = PolicyFlag::FEAF;
    for (unsigned i = 0; i < numSets; ++i) {
        //light
        setbloomfilter[i].eaf = new EAF[16];

        for(unsigned k = 0;k < 16; ++k)
        {
            (setbloomfilter[i].eaf[k]).valid = false;
            (setbloomfilter[i].eaf[k]).tags = 0;
        }
    }
//end
}
//light
FEAF::~FEAF()
{
    for (unsigned i=0; i<numSets; ++i)
    {
        delete [] setbloomfilter[i].eaf;
    }
    delete [] setbloomfilter;
}
//end
BaseSetAssoc::BlkType*
FEAF::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id,MemCmd cmd,int portId)
{
    BlkType *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id);

    if (blk != NULL) {
        // move this block to head of the MRU list
        sets[blk->set].moveToHead(blk);
        DPRINTF(CacheRepl, "set %x: moving blk %x (%s) to MRU\n",
                blk->set, regenerateBlkAddr(blk->tag, blk->set),
                is_secure ? "s" : "ns");
    }

    return blk;
}

BaseSetAssoc::BlkType*
FEAF::findVictim(Addr addr) const
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
FEAF::insertBlock(PacketPtr pkt, BlkType *blk)
{
//light
bool isValid = false;
if(blk->isValid())
{
    isValid = true;
    insertBloomFilter(blk->set,blk->tag);
}
//end
    BaseSetAssoc::insertBlock(pkt, blk);

    int set = extractSet(pkt->getAddr());
//light
if(isValid)
{
    if(testBloomFilter(set,extractTag(pkt->getAddr())))
    {
        sets[set].moveToHead(blk);
    }
    else
    {
        sets[set].insertFixLocation(blk,14);
    }
}
else
{

    sets[set].moveToHead(blk);
}
}

void
FEAF::invalidate(BlkType *blk)
{
    BaseSetAssoc::invalidate(blk);

    // should be evicted before valid blocks
    int set = blk->set;
    sets[set].moveToTail(blk);
}

FEAF*
FEAFParams::create()
{
    return new FEAF(this);
}
