/*
 * Copyright (c) 2011-2014 ARM Limited
 * All rights reserved
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
 * Copyright (c) 2006 The Regents of The University of Michigan
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
 * Authors: Ali Saidi
 *          Andreas Hansson
 *          William Wang
 */

/**
 * @file
 * Definition of a crossbar object.
 */
//light
#include <string>

#include "debug/CachePort.hh"
#include "debug/XBarWriteBack.hh"
#include "debug/Migration.hh"
#include "base/types.hh"
#include "mem/physical.hh"
#include "cpu/thread_context.hh"
#include "cpu/base.hh"
#include "sim/process.hh"
//end
#include "base/misc.hh"
#include "base/trace.hh"
#include "debug/AddrRanges.hh"
#include "debug/XBarControl.hh"
#include "mem/xbar_control.hh"
#include "sim/system.hh"

XBarControl::XBarControl(const XBarControlParams *p)
    : BaseXBar(p), system(p->system),haveRefuse(false),numCpu(1),pcmReadMigLocation(0),
    dramReadMigLocation(0),pcmWriteMigLocation(0),dramWriteMigLocation(0),
    readDone(true),writeDone(true),
    pcm_empty(true),dram_empty(true),
    numOfSets(p->num_of_sets),tempPkt(NULL),tempId(0),acrossThreadId(0),lockedThreadId(0),
    pcmMigrationLeft(64),dramMigrationLeft(64),migrationStartTick(0),
    migrationEndTick(0),WaitingRespondEvent(this),invalidWbMigEvent(this),readMigEvent(this),
    writeMigEvent(this),MigratingEvent(this),snoopFilter(p->snoop_filter)
    //InvalidPcmWbEvent(this),InvalidDramWbEvent(this),
    //ReqEvent(this),RespondEvent(this),
{
    // create the ports based on the size of the master and slave
    // vector ports, and the presence of the default port, the ports
    // are enumerated starting from zero
    for (int i = 0; i < p->port_master_connection_count; ++i) {
        std::string portName = csprintf("%s.master[%d]", name(), i);
        MasterPort* bp = new XBarControlMasterPort(portName, *this, i);
        masterPorts.push_back(bp);
        reqLayers.push_back(new ReqLayer(*bp, *this,
                                         csprintf(".reqLayer%d", i)));
        snoopLayers.push_back(new SnoopLayer(*bp, *this,
                                             csprintf(".snoopLayer%d", i)));
    }

    // see if we have a default slave device connected and if so add
    // our corresponding master port
    if (p->port_default_connection_count) {
        defaultPortID = masterPorts.size();
        std::string portName = name() + ".default";
        MasterPort* bp = new XBarControlMasterPort(portName, *this,
                                                   defaultPortID);
        masterPorts.push_back(bp);
        reqLayers.push_back(new ReqLayer(*bp, *this, csprintf(".reqLayer%d",
                                             defaultPortID)));
        snoopLayers.push_back(new SnoopLayer(*bp, *this,
                                             csprintf(".snoopLayer%d",
                                                      defaultPortID)));
    }

    // create the slave ports, once again starting at zero
    for (int i = 0; i < p->port_slave_connection_count; ++i) {
        std::string portName = csprintf("%s.slave[%d]", name(), i);
        SlavePort* bp = new XBarControlSlavePort(portName, *this, i);
        slavePorts.push_back(bp);
        respLayers.push_back(new RespLayer(*bp, *this,
                                           csprintf(".respLayer%d", i)));
        snoopRespPorts.push_back(new SnoopRespPort(*bp, *this));
    }

    if (snoopFilter)
        snoopFilter->setSlavePorts(slavePorts);

    clearPortCache();
    //light
   pcmBuffer = new char*[64];
   for(int i = 0;i<64;i++)
    pcmBuffer[i] = new char[64];
    
   dramBuffer = new char*[64];
    for(int j = 0;j<64;j++)
      dramBuffer[j] = new char[64];
    
    memset(MQ_head,0,sizeof(MQ_head));
    memset(MQ_tail,0,sizeof(MQ_tail));
    //end
}

XBarControl::~XBarControl()
{
    for (auto l: reqLayers)
        delete l;
    for (auto l: respLayers)
        delete l;
    for (auto l: snoopLayers)
        delete l;
    for (auto p: snoopRespPorts)
        delete p;
    //light
    for(int i=0;i<64;i++)
        delete pcmBuffer[i];
    for(int j = 0;j<64;j++)
        delete dramBuffer[j];
        
     delete [] pcmBuffer;
     delete [] dramBuffer;   
    //delete [] dramDataValid;
    //delete [] pcmDataValid;
    //delete [] MQ_head;
    //delete [] MQ_tail;
    //end
}

void XBarControl::doMigrating()
{
    DPRINTF(Migration,"have arrived doMigrating()\n");
    system->migrating = true;
	migrationStartTick = curTick();
    readFromMemory(system->lockedPcmNumber,system->lockedDramNumber);
}
void
XBarControl::init()
{
//light
memset(pcmDataValid,false,sizeof(pcmDataValid));
memset(dramDataValid,false,sizeof(dramDataValid));
//end
    // the base class is responsible for determining the block size
    BaseXBar::init();

    // iterate over our slave ports and determine which of our
    // neighbouring master ports are snooping and add them as snoopers
    for (const auto& p: slavePorts) {
        // check if the connected master port is snooping
        if (p->isSnooping()) {
            DPRINTF(AddrRanges, "Adding snooping master %s\n",
                    p->getMasterPort().name());
            snoopPorts.push_back(p);
        }
    }

    if (snoopPorts.empty())
        warn("XBarControl %s has no snooping ports attached!\n", name());
}
//light
//true,pcm;false,dram;
bool XBarControl::PcmOrDram(Addr addr)
{
    return (addr >= PhysicalMemory::watershed) ? false : true;
}
page_info* XBarControl::addr_to_pageinfo(Addr addr)
{
    int index = addr >> 12;
    if(index >= system->numPcmPages)
    {
        return system->dramPagePtr[index - system->numPcmPages];
    }
    else
    {
        return system->pcmPagePtr[index];
    }
}

void XBarControl::insert_into_MQ(page_info* page_temp,int MQ_number)
{
    page_temp->queue_number = MQ_number;
    if(!MQ_tail[MQ_number])
    {
        MQ_head[MQ_number] = MQ_tail[MQ_number] = page_temp;
        page_temp->pre = page_temp->next = NULL;
    }
    else
    {
        MQ_head[MQ_number]->pre = page_temp;
        page_temp->next = MQ_head[MQ_number];
        page_temp->pre = NULL;
        MQ_head[MQ_number] = page_temp;
    }
}

void XBarControl::change_queue(page_info *page_change)
{
    int Queue_number = page_change->queue_number;
    
    page_info *current_MQ = MQ_head[Queue_number];
    
    if(page_change != current_MQ)
    {
        if(page_change->pre)
            page_change->pre->next = page_change->next;
        if(page_change->next)
            page_change->next->pre = page_change->pre;
        else
            MQ_tail[Queue_number] = page_change->pre;
            
        page_change->pre = NULL;
        page_change->next = current_MQ;
        current_MQ->pre = page_change;
        MQ_head[Queue_number] = page_change;
    }
}

void XBarControl::insert_into_victim(page_info *page_temp)
{
    page_info *victim_tail = system->DRAM_victim_tail;
    if(victim_tail)
    {
        victim_tail->next = page_temp;
        page_temp->pre = victim_tail;
        page_temp->next = NULL;
        system->DRAM_victim_tail = page_temp;
    }
    else
    {
        page_temp->pre = NULL;
        page_temp->next = NULL;
        system->DRAM_victim_tail = system->DRAM_victim_list = page_temp;
    }
}

void XBarControl::insert_into_unranked(page_info *page_temp)
{
    page_info *unranked_tail = system->PCM_unranked_tail;
    if(unranked_tail)
    {
        unranked_tail->next = page_temp;
        page_temp->pre = unranked_tail;
        page_temp->next = NULL;
        system->PCM_unranked_tail = page_temp;
    }
    else
    {
        page_temp->pre = NULL;
        page_temp->next = NULL;
        system->PCM_unranked_tail = system->PCM_unranked_list = page_temp;
    }
}

void XBarControl::move_to_low(page_info *page_temp)
{
    int MQ_number = page_temp->queue_number;
    bool isPcm = (page_temp->page_number >= system->numPcmPages) ? false : true;
    if(page_temp == MQ_head[MQ_number])
        MQ_head[MQ_number] = MQ_tail[MQ_number] = NULL;
    else
    {
        page_temp->pre->next = NULL;
        MQ_tail[MQ_number] = page_temp->pre;
    }
    if(MQ_number == 0 || (!isPcm && page_temp->flag == 1))
    {
        if(!isPcm)
            insert_into_victim(page_temp);
        else
            insert_into_unranked(page_temp);
            
        page_temp->flag = 0;
        page_temp->access_counter = 0;
    }    
    else
    {
        insert_into_MQ(page_temp,MQ_number-1);
        page_temp->flag = 1;
    } 

}

void XBarControl::move_to_up(page_info* page_temp)
{
	int MQ_number = page_temp->queue_number;
	if(page_temp == MQ_tail[MQ_number])				//删除，该级队列中只有一个元素的情况
		MQ_head[MQ_number] = MQ_tail[MQ_number] = NULL;
	else{
	    page_temp->next->pre = NULL;
		MQ_head[MQ_number] = page_temp->next;
	}
	insert_into_MQ(page_temp,MQ_number+1);
}

void XBarControl::MQ_expiration()
{
    uint64_t current_time = curTick();
    for(int i = 0 ; i< 15;i++)
    {
        if(MQ_tail[i])
        {
            if((current_time - MQ_tail[i]->last_access_time) > 100000000)
            {
                if(MQ_tail[i]->access_counter > pow(2,i))
                    MQ_tail[i]->access_counter = pow(2,i) - 1;
                    
                MQ_tail[i]->last_access_time = current_time;
                move_to_low(MQ_tail[i]);
            }
        }
    }
}

void XBarControl::MQ_upgrade()
{
    for(int i=0;i<14;i++)
    {
        if(MQ_head[i])
        {
            if(MQ_head[i]->access_counter >= pow(2,i+1))
            {
                if(i==4 && MQ_head[i]->page_number < system->numPcmPages)
                {
                    page_info *page_temp;

                    page_temp = MQ_head[i];
                    if(page_temp == MQ_tail[4])
                        MQ_head[4] = MQ_tail[4] = NULL;
                    else
                        MQ_head[4] = page_temp->next;
                    
                    page_temp->access_counter = 0;
                    if(!system->PCM_unranked_tail)
                        system->PCM_unranked_tail = system->PCM_unranked_list = page_temp;
                    else
                    {
                        system->PCM_unranked_tail->next = page_temp;
                        page_temp->pre = system->PCM_unranked_tail;
                        page_temp->next = NULL;
                        system->PCM_unranked_tail = page_temp;
                    }
                    //存放待迁移页面的threadId和pageNumber;
                     int threadId = (tempPkt->req->masterId() - 7) / 6;
                     int page_number = page_temp->page_number;
                     migratingPage.push_back(std::make_pair(threadId,page_number));
                     DPRINTF(Migration,"threadId:%d,pageNumber:%d\n",threadId,page_number);
                    //放到待迁移的数组中;    
                }
                else
                {
                    move_to_up(MQ_head[i]);
                }
                
            }
        }
    }
}

void XBarControl::access_page(Addr addr)
{
    page_info *page_temp = addr_to_pageinfo(addr);
    bool isPcm = PcmOrDram(addr);
    if(isPcm)
    {
        for(int i = 0;i < migratingPage.size();i++)
        {
            if((migratingPage[i]).second == (addr >> 12))
            {
                DPRINTF(Migration,"access MigratingPage,return\n");
                return;
                
            }
        }
    }
    page_temp->last_access_time = curTick();
    
    if(!page_temp->access_counter)
    {
        if(page_temp->next)
            page_temp->next->pre = page_temp->pre;
        else
        {
            if(isPcm)
                system->PCM_unranked_tail = page_temp->pre;
            else
                system->DRAM_victim_tail = page_temp->pre;
        }
        if(page_temp->pre)
            page_temp->pre->next = page_temp->next;
        else
        {
            if(isPcm)
                system->PCM_unranked_list = page_temp->next;
            else
                system->DRAM_victim_list = page_temp->next;
        }
        
        insert_into_MQ(page_temp,0);
    }
    else
    {
        change_queue(page_temp);
    }
    page_temp->access_counter++;
    MQ_upgrade();
    MQ_expiration(); 
}
void XBarControl::output_MQ()
{
    for(int j=0;j<15;j++)
	{
		page_info* current = MQ_head[j];
		while(current)
		{
			DPRINTFN("queue:%d,page_number:%d,access_counter:%d,accss_time:%d\n",j,current->page_number,current->access_counter,current->last_access_time);
			current = current->next;
		}
		
	}
	DPRINTFN("dramLocation:%d\n",system->dramLocation);
	return ;
}
/*void XBarControl::processPcmInvalidWb()
{
    assert(!invalidPcmWbPacket.empty());
    for(int i = 0;i < invalidPcmWbPacket.size();i++)
    {
        PacketPtr pkt = invalidPcmWbPacket[i];
        PortID master_port_id = findPort(pkt->getAddr());
        DPRINTF(Migration,"master_pord_id:%d\n",master_port_id);
        bool success = masterPorts[master_port_id]->sendTimingReq(pkt);
        
        if(success)
        {
            invalidPcmWbPacket.erase(invalidPcmWbPacket.begin()+i);
            i--;
        }
    }
    

}

void XBarControl::processDramInvalidWb()
{
    assert(!invalidDramWbPacket.empty());
    for(int j = 0; j < invalidDramWbPacket.size();j++)
    {
        PacketPtr pkt = invalidDramWbPacket[j];
        PortID master_port_id = findPort(pkt->getAddr());
        bool success = masterPorts[master_port_id]->sendTimingReq(pkt);
        if(success)
        {
            invalidDramWbPacket.erase(invalidDramWbPacket.begin() + j );
            j--;
        }
        
   }
    

}
*/
void XBarControl::doSomeEndThing()
{
    assert(system->migrating && readDone && writeDone);
    assert(invalidWbPacket.empty());
    if(!system->both)
    {
		system->clearPhysPages(true,system->lockedPcmNumber);
    }
    //把申请到的DRAM页放到victim_list尾部。
    page_info *page_temp = system->DRAM_victim_list;
    system->DRAM_victim_list = page_temp->next;
    page_temp->next->pre = NULL;
    if(!system->DRAM_victim_list)
        system->DRAM_victim_tail = NULL;
    page_temp->access_counter = 32;
    page_temp->last_access_time = curTick();
    insert_into_MQ(page_temp,5);
    //over;
	DPRINTF(Migration,"migration completed\n");
	system->migrating = false;
    migrationEndTick = curTick();
    migrationTick +=migrationEndTick - migrationStartTick;
    pageMigrationNumber++;
    if(haveRefuse)
    {
        slavePorts[1]->sendRetry();
    }
}
void XBarControl::processInvalidWb()
{
    if(!invalidWbPacket.empty())
    {
        for(int j = 0; j < invalidWbPacket.size();j++)
        {
            PacketPtr pkt = invalidWbPacket[j];
            PortID master_port_id = findPort(pkt->getAddr());
            DPRINTF(Migration,"invalidWb|Addr:0x%x\n",pkt->getAddr());
            //DDUMP(Migration, pkt->getPtr<uint8_t>(), pkt->getSize());
            bool success = masterPorts[master_port_id]->sendTimingReq(pkt);
            if(success)
            {
                invalidWbPacket.erase(invalidWbPacket.begin() + j );
                j--;
            }
            else
            {
                break;
            }
       }
       if(!invalidWbPacket.empty())
       {
            schedule(invalidWbMigEvent, nextCycle());
       }
       else
       {
            doSomeEndThing();
       }
    }
    else
    {
        doSomeEndThing();
    }

}
bool XBarControl::processReqEvent()
{
    // determine the source port based on the id
    SlavePort *src_port = slavePorts[tempId];

    // remember if the packet is an express snoop
    bool is_express_snoop = tempPkt->isExpressSnoop();

    // determine the destination based on the address
    PortID master_port_id = findPort(tempPkt->getAddr());

    // test if the crossbar should be considered occupied for the current
    // port, and exclude express snoops from the check
    /*if (!is_express_snoop && !reqLayers[master_port_id]->tryTiming(src_port)) {
        DPRINTF(XBarControl, "recvTimingReq: src %s %s 0x%x BUSY\n",
                src_port->name(), tempPkt->cmdString(), tempPkt->getAddr());
        //return false;
        return ;
    }*/
//light
/*if(migrating == true)
{
   reqLayers[master_port_id]->failedTiming(src_port,clockEdge(headerCycles));
   return false;
}
else
{*/
//end
    DPRINTF(XBarControl, "recvTimingReq: src %s %s expr %d 0x%x\n",
            src_port->name(), tempPkt->cmdString(), is_express_snoop,
            tempPkt->getAddr());

    // store size and command as they might be modified when
    // forwarding the packet
    unsigned int pkt_size = tempPkt->hasData() ? tempPkt->getSize() : 0;
    unsigned int pkt_cmd = tempPkt->cmdToIndex();

    // set the source port for routing of the response
    //tempPkt->setSrc(tempId);

    calcPacketTiming(tempPkt);
    Tick packetFinishTime = tempPkt->lastWordDelay + curTick();

    // uncacheable requests need never be snooped
    if (!tempPkt->req->isUncacheable() && !system->bypassCaches()) {
        // the packet is a memory-mapped request and should be
        // broadcasted to our snoopers but the source
        if (snoopFilter) {
            // check with the snoop filter where to forward this packet
            auto sf_res = snoopFilter->lookupRequest(tempPkt, *src_port);
            packetFinishTime += sf_res.second * clockPeriod();
            DPRINTF(XBarControl, "recvTimingReq: src %s %s 0x%x"\
                    " SF size: %i lat: %i\n", src_port->name(),
                    tempPkt->cmdString(), tempPkt->getAddr(), sf_res.first.size(),
                    sf_res.second);
            forwardTiming(tempPkt, tempId,sf_res.first);
        } else {
            forwardTiming(tempPkt, tempId);
        }
    }

    // remember if we add an outstanding req so we can undo it if
    // necessary, if the packet needs a response, we should add it
    // as outstanding and express snoops never fail so there is
    // not need to worry about them
    bool add_outstanding = !is_express_snoop && tempPkt->needsResponse();

    // keep track that we have an outstanding request packet
    // matching this request, this is used by the coherency
    // mechanism in determining what to do with snoop responses
    // (in recvTimingSnoop)
    if (add_outstanding) {
        // we should never have an exsiting request outstanding
        assert(outstandingReq.find(tempPkt->req) == outstandingReq.end());
        outstandingReq.insert(tempPkt->req);
    }

    // Note: Cannot create a copy of the full packet, here.
    MemCmd orig_cmd(tempPkt->cmd);

    // since it is a normal request, attempt to send the packet
    bool success = masterPorts[master_port_id]->sendTimingReq(tempPkt);

    if (snoopFilter && !tempPkt->req->isUncacheable()
        && !system->bypassCaches()) {
        // The packet may already be overwritten by the sendTimingReq function.
        // The snoop filter needs to see the original request *and* the return
        // status of the send operation, so we need to recreate the original
        // request.  Atomic mode does not have the issue, as there the send
        // operation and the response happen instantaneously and don't need two
        // phase tracking.
        MemCmd tmp_cmd(tempPkt->cmd);
        tempPkt->cmd = orig_cmd;
        // Let the snoop filter know about the success of the send operation
        snoopFilter->updateRequest(tempPkt, *src_port, !success);
        tempPkt->cmd = tmp_cmd;
    }

    // if this is an express snoop, we are done at this point
    if (is_express_snoop) {
        assert(success);
        snoops++;
    } else {
        // for normal requests, check if successful
        if (!success)  {
            // inhibited packets should never be forced to retry
            assert(!tempPkt->memInhibitAsserted());

            // if it was added as outstanding and the send failed, then
            // erase it again
            if (add_outstanding)
                outstandingReq.erase(tempPkt->req);

            // undo the calculation so we can check for 0 again
            tempPkt->firstWordDelay = tempPkt->lastWordDelay = 0;

            DPRINTF(XBarControl, "recvTimingReq: src %s %s 0x%x RETRY\n",
                    src_port->name(), tempPkt->cmdString(), tempPkt->getAddr());

            // update the layer state and schedule an idle event
            reqLayers[master_port_id]->failedTiming(src_port,
                                                    clockEdge(headerCycles));
        } else {
            // update the layer state and schedule an idle event
            reqLayers[master_port_id]->succeededTiming(packetFinishTime);
        }
    }

    // stats updates only consider packets that were successfully sent
    if (success) {
        pktCount[tempId][master_port_id]++;
        pktSize[tempId][master_port_id]+=pkt_size;
        transDist[pkt_cmd]++;
        //light
        if(master_port_id == 0)
        {   
            pcm_empty=false;
        }
        else
        {
            dram_empty=false;
        }
        //end
        //migrating = false;
    }

    return success;
    //return ;
}
bool
XBarControl::recvTimingReq(PacketPtr pkt, PortID slave_port_id)
{
/*if(!system->migrating)
{
    assert(readWaitingPacket.empty());
    assert(writeWaitingPacket.empty());
}*/
if(slave_port_id == 0)
{//cache失效写回；
    DPRINTF(Migration,"Cache write backs have been received\n");
    //assert(system->migrating && writeDone);
    //assert(system->migrating);
    //assert(readWaitingPacket.empty() && writeWaitingPacket.empty());
    assert(!pkt->isRead());
    Addr addr = pkt->getAddr();
    DPRINTF(Migration,"wb|old_addr:0x%x\n",addr);
    pkt->setSrc(slave_port_id);
    Addr newaddr;
    if(PcmOrDram(addr))
    {
        newaddr = (system->lockedDramNumber << 12) + (addr & mask(12)); 
        //pkt->setAddr(newaddr);
        //invalidPcmWbPacket.push_back(pkt);
        //processPcmInvalidWb();
    }
    else
    {
        newaddr = (system->lockedPcmNumber << 12) + (addr & mask(12));
        //pkt->setAddr(newaddr);
        //invalidDramWbPacket.push_back(pkt);
        //processDramInvalidWb();
    }
    pkt->setAddr(newaddr);
    DPRINTF(Migration,"pkt->getAddr():0x%x\n",pkt->getAddr());
    invalidWbPacket.push_back(pkt);
    DPRINTF(Migration,"wb|new_addr:0x%x\n",newaddr);
    return true;
}
else
{
    /*if(system->migrating && readDone && !writeDone)
    {
        processWriteToMemory();
        return false;
    }
    if(!invalidPcmWbPacket.empty())
    {
        processPcmInvalidWb();
        return false;
    }
    if(!invalidDramWbPacket.empty())
    {
        processDramInvalidWb();
        return false;
    }*/
    DPRINTF(Migration,"recvTimingReq:addr:0x%x,read:%s\n",pkt->getAddr(),pkt->isRead() ? "yes":"no");
    if(system->migrating)
    {
        int pageNumber = pkt->getAddr() >> 12;
        if((pageNumber == system->lockedPcmNumber) || (pageNumber == system->lockedDramNumber))
        {
           /* pkt->setSrc(slave_port_id);
            if(pkt->isRead())
            {
                readWaitingPacket.push_back(pkt);
                schedule(WaitingRespondEvent, clockEdge(headerCycles));   
            }
            else
            {
                writeWaitingPacket.push_back(pkt);
                DPRINTF(Migration,"have received wb during migration,addr:0x%x\n",pkt->getAddr());
            }
            return true;*/
            DPRINTF(Migration,"refuse pkt|Addr:0x%x\n",pkt->getAddr());
            haveRefuse = true;
            return false;
            
        }
    }
}
tempPkt = pkt;
tempId = slave_port_id;
tempPkt->setSrc(slave_port_id);
//Addr addr = tempPkt->getAddr();
SlavePort *src_port = slavePorts[slave_port_id];
bool is_express_snoop = tempPkt->isExpressSnoop();
// determine the destination based on the address
PortID master_port_id = findPort(tempPkt->getAddr());

// test if the crossbar should be considered occupied for the current
// port, and exclude express snoops from the check
if (!is_express_snoop && !reqLayers[master_port_id]->tryTiming(src_port)) 
{
DPRINTF(XBarControl, "recvTimingReq: src %s %s 0x%x BUSY\n",
             src_port->name(), tempPkt->cmdString(), tempPkt->getAddr());
        return false;
}
//access_page(addr);

//schedule(ReqEvent, curTick());
bool success = processReqEvent();
return success;
}

//light
//bool XBarControl::satisfyWaitingRequest(PacketPtr pkt)
void XBarControl::satisfyWaitingRequest()
{
    DPRINTF(Migration,"negativePacketSatisfy\n");
  for(int i = 0;i < readWaitingPacket.size();i++)
  {
        PacketPtr pkt = readWaitingPacket[i];
        Addr requestAddr = pkt->getAddr();
		bool isPcm = PcmOrDram(requestAddr);
		uint8_t requestIndex; 
		bool isFound = false;
	    for(int j = 0;j< writeWaitingPacket.size();j++)
		{
		    if((writeWaitingPacket[j])->getAddr() == requestAddr)
		    {
		        DPRINTF(Migration,"read hit in wb\n");
		        std::memcpy(pkt->getPtr<uint8_t>(),(writeWaitingPacket[j])->getPtr<uint8_t>(),64);
		        isFound = true;
		        break;
		    }
		}
		if(!isFound)
		{
		    if(isPcm)
		    {
		
		        requestIndex = (requestAddr - (system->lockedPcmNumber << 12)) >> 6;
		        DPRINTF(Migration,"requestIndex:%d\n",requestIndex);
			    if(!pcmDataValid[requestIndex])
				    continue;
			    else
				    std::memcpy(pkt->getPtr<uint8_t>(),pcmBuffer[requestIndex],pkt->getSize());
		    }
		    else
		    {
		        requestIndex = (requestAddr - (system->lockedDramNumber << 12)) >> 6;
			    if(!dramDataValid[requestIndex])
				    continue;
			    else
				    std::memcpy(pkt->getPtr<uint8_t>(),dramBuffer[requestIndex],pkt->getSize());
		    }
        }
		pkt->makeResponse();
		bool success M5_VAR_USED = slavePorts[pkt->getDest()]->sendTimingResp(pkt);
		assert(success);
        readWaitingPacket.erase(readWaitingPacket.begin() + i);
        i--;
        DPRINTF(Migration,"requestIndex:%d have been served\n",requestIndex);
  }
}
void XBarControl::satisfyWaitingRequest(uint8_t index,bool Pcm)
{
    for(int i = 0;i < readWaitingPacket.size();i++)
		{
		DPRINTF(Migration,"positivePacketSatisfy\n");
			Addr requestAddr = readWaitingPacket[i]->getAddr();
			bool isPcm = PcmOrDram(requestAddr);
			uint8_t requestIndex;
			if(isPcm)
			   requestIndex = (requestAddr - (system->lockedPcmNumber << 12)) >> 6;
			else
			   requestIndex = (requestAddr - (system->lockedDramNumber << 12)) >> 6;
			   
			if(requestIndex == index && isPcm == Pcm)
			{
			 DPRINTF(Migration,"requestIndex:%d\n",index);
			    PacketPtr pkt = readWaitingPacket[i];
				if(isPcm)
				{
					std::memcpy(pkt->getPtr<uint8_t>(),pcmBuffer[requestIndex],pkt->getSize());
				}
				else
				{
					std::memcpy(pkt->getPtr<uint8_t>(),dramBuffer[requestIndex],pkt->getSize());
				}
				pkt->makeResponse();
				slavePorts[pkt->getDest()]->sendTimingResp(pkt);
				readWaitingPacket.erase(readWaitingPacket.begin() + i);
				i--;
				DPRINTF(Migration,"requestIndex:%d have been served\n",requestIndex);
			}
		}
}
void XBarControl::satisfyWaitingWriteRequest()
{
    assert(writeDone);
    for(int i = 0;i < writeWaitingPacket.size();i++)
    {
        DPRINTF(Migration,"have waitingWriteRequest\n");
        PacketPtr pkt = writeWaitingPacket.front();
        Addr addr = pkt->getAddr();
        DPRINTF(Migration,"waitingWB|oldAddr:0x%x\n",addr);
        Addr newaddr;
        if(PcmOrDram(addr))
        {
            newaddr = (system->lockedDramNumber << 12) + (addr & mask(12));
        }
        else
        {
            newaddr = (system->lockedPcmNumber << 12) + (addr & mask(12));
        }
        DPRINTF(Migration,"waitingWB|newAddr:0x%x\n",newaddr);
        pkt->setAddr(newaddr);
        PortID master_port_id = findPort(newaddr);
        DPRINTF(Migration,"master_port_id:%d\n",master_port_id);
        bool success = masterPorts[master_port_id]->sendTimingReq(pkt);
        if(success)
        {
            writeWaitingPacket.erase(writeWaitingPacket.begin() + i);
            i--;
        }
  }
}

void XBarControl::processMigrationRespond(PacketPtr respondPacket)
{
    DPRINTF(Migration,"%s\n",__func__);
    Addr respondAddr = respondPacket->getAddr();
    uint8_t cmdIndex = respondPacket->cmdToIndex();
    bool isPcm = PcmOrDram(respondAddr);
    uint8_t index;
    if(cmdIndex != 6)
	{
	    
	    assert(respondPacket->getSize() == 64);
		//read response;
		if(isPcm)
		{
		    index = (respondAddr - (system->lockedPcmNumber << 12)) >> 6;
		    DPRINTF(Migration,"Pcm read response received.index:%d,respondAddr:0x%x\n",index,respondAddr);
			std::memcpy(pcmBuffer[index],respondPacket->getPtr<uint8_t>(),64);
			pcmDataValid[index] = true;
			//satisfyWaitingRequest(index,true);
			pcmMigrationLeft--;
		}
		else
		{
		    index = (respondAddr - (system->lockedDramNumber << 12)) >> 6;
		    DPRINTF(Migration,"Dram read response received.index:%d,respondAddr:0x%x\n",index,respondAddr);
			std::memcpy(dramBuffer[index],respondPacket->getPtr<uint8_t>(),64);
			dramDataValid[index] = true;
			//satisfyWaitingRequest(index,false);
			dramMigrationLeft--;
		}
		DPRINTF(Migration,"pcmMigrationLeft:%d,dramMigrationLeft:%d\n",pcmMigrationLeft,dramMigrationLeft);	   
		if (respondPacket) 
	    {
		    delete respondPacket->req;
		    delete respondPacket;
	    }
		assert(pcmMigrationLeft>=0 && dramMigrationLeft>=0);
		if(pcmMigrationLeft == 0 && dramMigrationLeft == 0)
		{
		    readDone = true;
		    DPRINTF(Migration,"migration read complete\n");
		    writeToMemory(system->lockedDramNumber,system->lockedPcmNumber);
		}
		

	}
	else
	{//write response;
	   DPRINTF(Migration,"migration write response received\n");
        assert(!writeDone);
	        if(isPcm)
			     pcmMigrationLeft--;
		    else
			     dramMigrationLeft--;
	DPRINTF(Migration,"pcmMigrationLeft:%d,dramMigrationLeft:%d\n",pcmMigrationLeft,dramMigrationLeft);
            assert(pcmMigrationLeft>=0 && dramMigrationLeft>=0);
		    if(!pcmMigrationLeft && !dramMigrationLeft)
		    {
		        writeDone = true;
		        processInvalidWb();
	        }	
	   
   }
		

}
void XBarControl::processReadFromMemory()
{
    DPRINTF(Migration,"processReadFromMemory,have arrived here\n");
    assert(!readDone & system->migrating);
    
   for(int i = pcmReadMigLocation;i<64;i++)
	{
		Addr requestAddr = ((system->lockedPcmNumber) << 12) + (i * 64);
		Request *dataReq = new Request(requestAddr,64,0,Request::funcMasterId);
		Packet *dataPkt = new Packet(dataReq,MemCmd::ReadReq);
		dataPkt->allocate();
		PortID master_port_id = findPort(requestAddr);
		bool success = masterPorts[master_port_id]->sendTimingReq(dataPkt);
		if(!success)
		{
		    DPRINTF(Migration,"readFromPcm not all served.pcmReadMigLocation:%d\n",pcmReadMigLocation);
			//fatal("migration,readFromPcmMemory failed\n");
			break;
		}
		pcmReadMigLocation = i+1;
	}
	if(system->both)
	{
		for(int j = dramReadMigLocation;j<64;j++)
		{
			Addr requestAddr = ((system->lockedDramNumber) << 12) + (j * 64);
			Request *dataReq = new Request(requestAddr,64,0,Request::funcMasterId);
			Packet *dataPkt = new Packet(dataReq,MemCmd::ReadReq);
			dataPkt->allocate();
			PortID master_port_id = findPort(requestAddr);
			bool success = masterPorts[master_port_id]->sendTimingReq(dataPkt);
			if(!success)
			{
DPRINTF(Migration,"readFromDram not all served.dramReadMigLocation:%d\n",dramReadMigLocation);
				//fatal("migration,readFromDramMemory failed\n");
				break;
			}
			dramReadMigLocation = j + 1;
		}
	}
   
    if(system->both & (pcmReadMigLocation < 64 || dramReadMigLocation < 64))
    {
        schedule(readMigEvent,nextCycle());   
	}
	if(!system->both & (pcmReadMigLocation < 64))
	{
	    schedule(readMigEvent,nextCycle());
	}
    
}
void XBarControl::processWriteToMemory()
{
    DPRINTF(Migration,"processWriteToMemory,have arrived here\n");
    assert(!writeDone & system->migrating);
    for(int i = pcmWriteMigLocation;i<64;i++)
    {
        Addr requestAddr = ((system->lockedDramNumber) << 12) + (i * 64);
		Request *dataReq = new Request(requestAddr,64,0,Request::funcMasterId);
		Packet *dataPkt = new Packet(dataReq,MemCmd::Writeback);
		dataPkt->allocate();
		std::memcpy(dataPkt->getPtr<uint8_t>(),pcmBuffer[i],64);
		PortID master_port_id = findPort(requestAddr);
		DPRINTF(Migration,"writeToMemory|requestAddr:0x%x,masterid:%d\n",requestAddr,master_port_id);
		bool success = masterPorts[master_port_id]->sendTimingReq(dataPkt);
		if(!success)
		{
DPRINTF(Migration,"processWTDram not all served.pcmWriteMigLocation:%d\n",pcmWriteMigLocation);
			break;
		}
		pcmWriteMigLocation = i+1;
    }
    if(system->both)
    {
        for(int j = dramWriteMigLocation;j<64;j++)
		{
		    assert(dramDataValid[j]);
			Addr requestAddr = ((system->lockedPcmNumber) << 12) + (j * 64);
			Request *dataReq = new Request(requestAddr,64,0,Request::funcMasterId);
			Packet *dataPkt = new Packet(dataReq,MemCmd::Writeback);
			dataPkt->allocate();
			std::memcpy(dataPkt->getPtr<uint8_t>(),dramBuffer[j],64);
			PortID master_port_id = findPort(requestAddr);
			bool success = masterPorts[master_port_id]->sendTimingReq(dataPkt);
			if(!success)
			{
DPRINTF(Migration,"processWTPcm not all served.dramWriteMigLocation:%d\n",dramWriteMigLocation);
				break;
			}
			dramWriteMigLocation = j+1;
		}
    }
    if(system->both & (pcmWriteMigLocation < 64 || dramWriteMigLocation < 64))
    {
        schedule(writeMigEvent,nextCycle());   
	}
	if(!system->both & (pcmWriteMigLocation < 64))
	{
	    schedule(writeMigEvent,nextCycle());
	}
}
void XBarControl::writeToMemory(int dramDes,int pcmDes)
{
    writeDone = false;
    dram_empty = false;
    dramMigrationLeft = 64;
    pcmMigrationLeft = 0;
    pcmWriteMigLocation = 0;
    assert(system->migrating);
    for(int i = 0;i<64;i++)
	{
	    
		Addr requestAddr = (dramDes << 12) + (i * 64);
		Request *dataReq = new Request(requestAddr,64,0,Request::funcMasterId);
		Packet *dataPkt = new Packet(dataReq,MemCmd::Writeback);
		dataPkt->allocate();
		std::memcpy(dataPkt->getPtr<uint8_t>(),pcmBuffer[i],64);
		PortID master_port_id = findPort(requestAddr);
		DPRINTF(Migration,"writeToMemory|requestAddr:0x%x,masterid:%d\n",requestAddr,master_port_id);
		bool success = masterPorts[master_port_id]->sendTimingReq(dataPkt);
		if(!success)
		{
		DPRINTF(Migration,"writeToDram not all served.pcmWriteMigLocation:%d\n",pcmWriteMigLocation);
			break;
		}
		pcmWriteMigLocation = i+1;
	}
	if(system->both)
	{
	    pcmMigrationLeft = 64;
	    pcm_empty = false;
	    dramWriteMigLocation = 0;
		for(int j = 0;j<64;j++)
		{
		    assert(dramDataValid[j]);
			Addr requestAddr = (pcmDes << 12) + (j * 64);
			Request *dataReq = new Request(requestAddr,64,0,Request::funcMasterId);
			Packet *dataPkt = new Packet(dataReq,MemCmd::Writeback);
			dataPkt->allocate();
			std::memcpy(dataPkt->getPtr<uint8_t>(),dramBuffer[j],64);
			PortID master_port_id = findPort(requestAddr);
					DPRINTF(Migration,"writeToMemory|requestAddr:0x%x,masterid:%d\n",requestAddr,master_port_id);
			bool success = masterPorts[master_port_id]->sendTimingReq(dataPkt);
			if(!success)
			{
DPRINTF(Migration,"writeToPcm not all served.dramWriteMigLocation:%d\n",dramWriteMigLocation);
				break;
			}
			dramWriteMigLocation = j+1;
		}
	}
	
    if(system->both & (pcmWriteMigLocation < 64 || dramWriteMigLocation < 64))
    {
        schedule(writeMigEvent,nextCycle());   
	}
	if(!system->both & (pcmWriteMigLocation < 64))
	{
	    schedule(writeMigEvent,nextCycle());
	}
    //DPRINTF(Migration,"writeToMemory has completed\n");
}

void XBarControl::readFromMemory(int pcmSrc,int dramSrc)
{
DPRINTF(Migration,"have arrived readFromMemory\n");
    readDone = false;
    pcm_empty = false;
	pcmMigrationLeft = 64;
	dramMigrationLeft = 0;
//从PCM读取pcmSrc页面的内容；
    pcmReadMigLocation = 0;
	for(int i = 0;i<64;i++)
		pcmDataValid[i] = false;		
	for(int i = 0;i<64;i++)
	{
		Addr requestAddr = (pcmSrc << 12) + (i * 64);
		Request *dataReq = new Request(requestAddr,64,0,Request::funcMasterId);
		Packet *dataPkt = new Packet(dataReq,MemCmd::ReadReq);
		dataPkt->allocate();	
		PortID master_port_id = findPort(requestAddr);
		bool success = masterPorts[master_port_id]->sendTimingReq(dataPkt);
		if(!success)
		{
		    DPRINTF(Migration,"readFromPcm not all served.pcmReadMigLocation:%d\n",pcmReadMigLocation);
			//fatal("migration,readFromPcmMemory failed\n");
			break;
		}
		pcmReadMigLocation = i+1;
	}
	DPRINTF(Migration,"pcmReadMigLocation :%d\n",pcmReadMigLocation);
	if(system->both)
	{
	    dramMigrationLeft = 64;
        dram_empty = false;
        dramReadMigLocation = 0;
		for(int j = 0;j<64;j++)
			dramDataValid[j] = false;
		
		for(int j = 0;j<64;j++)
		{
			Addr requestAddr = (dramSrc << 12) + (j * 64);
			Request *dataReq = new Request(requestAddr,64,0,Request::funcMasterId);
			Packet *dataPkt = new Packet(dataReq,MemCmd::ReadReq);
			dataPkt->allocate();
			PortID master_port_id = findPort(requestAddr);
			bool success = masterPorts[master_port_id]->sendTimingReq(dataPkt);
			if(!success)
			{
DPRINTF(Migration,"readFromDram not all served.dramReadMigLocation:%d\n",dramReadMigLocation);
				//fatal("migration,readFromDramMemory failed\n");
				break;
			}
			dramReadMigLocation = j + 1;
		}
		DPRINTF(Migration,"dramReadMigLocation :%d\n",dramReadMigLocation);
	}

    if(system->both & (pcmReadMigLocation < 64 || dramReadMigLocation < 64))
    {
        schedule(readMigEvent,nextCycle());   
	}
	if(!system->both & (pcmReadMigLocation < 64))
	{
	    schedule(readMigEvent,nextCycle());
	}
}
void XBarControl::migratingFunc()
{
    if(!migratingPage.empty() && !system->migrating)
	{
	    //assert(invalidPcmWbPacket.empty());
	    assert(invalidWbPacket.empty());
	    
		page_info  *page_temp = system->DRAM_victim_list;
		if(!page_temp)
		    return;
		else
		{
		    assert(page_temp->access_counter == 0);
		    std::pair<int,int> temp = migratingPage.front();
			migratingPage.erase(migratingPage.begin());
			system->lockedPcmNumber = temp.second;
			lockedThreadId = temp.first;
			numCpu = 1;
			haveRefuse = false;
			system->acrossTwoCpu = false;
			system->lockedDramNumber = page_temp->page_number;
			DPRINTF(Migration,"lockedDramNumber:%d\n",system->lockedDramNumber);
			if(page_temp->valid == 1)
			{
				system->both= true;
				uint8_t threadId;
				DPRINTF(Migration,"numContexts():%d\n",system->numContexts());
				for(int i = 0;i< system->numContexts();i++)
				{
				    threadId = i;
				    if(threadId == lockedThreadId)
				        continue;
				    else
				    {
				        Process *p = system->getThreadContext(threadId)->getProcessPtr();
	                    bool success = p->pTable->findPhyNumber(system->lockedDramNumber);
	                    if(success)
	                    {
	                        system->acrossTwoCpu = true;
	                        break;
	                    }
				    }
				}
				DPRINTF(Migration,"acrossTwoCpu:%s\n",system->acrossTwoCpu ? "yes" : "no");
				if(system->acrossTwoCpu)
				{
				DPRINTF(Migration,"acrossTwoCpu,acrossThreadId:%d\n",threadId);
				    assert(threadId != lockedThreadId);
				    acrossThreadId = threadId;
				    numCpu = 2;
				}
				else
				{
				    Process *p = system->getThreadContext(lockedThreadId)->getProcessPtr();
	                bool success = p->pTable->findPhyNumber(system->lockedDramNumber);
	                assert(success);
				}
			}
			else
			{
			    system->both= false;
				system->setPhysPages(false,system->lockedDramNumber - system->numPcmPages);
			}
			
			//system->migrating = true;
			//migrationStartTick = curTick();
			DPRINTF(Migration,"Start|pcmNumber:%d,dramNumber:%d,both:%s\n",system->lockedPcmNumber,system->lockedDramNumber,system->both ? "yes" : "no");
			
			system->getThreadContext(lockedThreadId)->getCpuPtr()->setMigrating(true,system->lockedPcmNumber);
			//system->getThreadContext(lockedThreadId)->getCpuPtr()->setPageNumber(system->lockedPcmNumber);
			if(system->acrossTwoCpu)
			{
			    system->getThreadContext(acrossThreadId)->getCpuPtr()->setMigrating(true,system->lockedDramNumber);
			}
			//readFromMemory(system->lockedPcmNumber,system->lockedDramNumber);
			return;
		}
	}
}
//end
bool XBarControl::processRespondEvent()
{
    // determine the source port based on the id
    MasterPort *src_port = masterPorts[tempId];

    // determine the destination based on what is stored in the packet
    PortID slave_port_id = tempPkt->getDest();

    // test if the crossbar should be considered occupied for the
    // current port
    /*if (!respLayers[slave_port_id]->tryTiming(src_port)) {
        DPRINTF(XBarControl, "recvTimingResp: src %s %s 0x%x BUSY\n",
                src_port->name(), tempPkt->cmdString(), tempPkt->getAddr());
        return;
    }*/

    DPRINTF(XBarControl, "recvTimingResp: src %s %s 0x%x\n",
            src_port->name(), tempPkt->cmdString(), tempPkt->getAddr());

    // store size and command as they might be modified when
    // forwarding the packet
    unsigned int pkt_size = tempPkt->hasData() ? tempPkt->getSize() : 0;
    unsigned int pkt_cmd = tempPkt->cmdToIndex();

    calcPacketTiming(tempPkt);
    Tick packetFinishTime = tempPkt->lastWordDelay + curTick();

    // the packet is a normal response to a request that we should
    // have seen passing through the crossbar
    assert(outstandingReq.find(tempPkt->req) != outstandingReq.end());

    if (snoopFilter && !tempPkt->req->isUncacheable() && !system->bypassCaches()) {
        // let the snoop filter inspect the response and update its state
        snoopFilter->updateResponse(tempPkt, *slavePorts[slave_port_id]);
    }

    // remove it as outstanding
    outstandingReq.erase(tempPkt->req);

    // send the packet through the destination slave port
    bool success M5_VAR_USED = slavePorts[slave_port_id]->sendTimingResp(tempPkt);

    // currently it is illegal to block responses... can lead to
    // deadlock
    assert(success);

    respLayers[slave_port_id]->succeededTiming(packetFinishTime);

    // stats updates
    pktCount[slave_port_id][tempId]++;
    pktSize[slave_port_id][tempId] += pkt_size;
    transDist[pkt_cmd]++;

    return success;


}
//end
bool
XBarControl::recvTimingResp(PacketPtr pkt, PortID master_port_id)
{
if(pkt->cmdToIndex() == 24)
{
    if(tempId == 0)
    {
        DPRINTF(Migration,"pcm queue is empty!\n");
        pcm_empty = true;
    }
    else
    {
        DPRINTF(Migration,"dram queue is empty!\n");
        dram_empty = true;
    }
    if(pcm_empty & dram_empty)
    {
        DPRINTF(Migration,"Migration is possible\n");
        //migratingFunc();
    }
    delete pkt->req;
    delete pkt;
    return true;
}
Addr respondAddr = pkt->getAddr();
int respondNumber = respondAddr >> 12;

if(system->migrating && ((respondNumber == system->lockedDramNumber) || (respondNumber == system->lockedPcmNumber)))
{
    processMigrationRespond(pkt);
    return true;
}
if(pkt->cmdToIndex() == 6)
{
  //非迁移过程中收到的writeback respond
    return true;
}
DPRINTF(Migration,"recvTimingResp:addr:0x%x\n",pkt->getAddr());
//DDUMP(Migration, pkt->getPtr<uint8_t>(), pkt->getSize());
tempPkt=pkt;
tempId=master_port_id;
MasterPort *src_port = masterPorts[tempId];
    // determine the source port based on the id
    // determine the destination based on what is stored in the packet
     PortID slave_port_id = tempPkt->getDest();

    // test if the crossbar should be considered occupied for the
    // current port
    if (!respLayers[slave_port_id]->tryTiming(src_port)) {
        DPRINTF(XBarControl, "recvTimingResp: src %s %s 0x%x BUSY\n",
                src_port->name(), tempPkt->cmdString(), tempPkt->getAddr());
        return false;
    }
bool success = processRespondEvent();
return success;
}

void
XBarControl::recvTimingSnoopReq(PacketPtr pkt, PortID master_port_id)
{
    DPRINTF(XBarControl, "recvTimingSnoopReq: src %s %s 0x%x\n",
            masterPorts[master_port_id]->name(), pkt->cmdString(),
            pkt->getAddr());

    // update stats here as we know the forwarding will succeed
    transDist[pkt->cmdToIndex()]++;
    snoops++;

    // we should only see express snoops from caches
    assert(pkt->isExpressSnoop());

    // set the source port for routing of the response
    pkt->setSrc(master_port_id);

    if (snoopFilter) {
        // let the Snoop Filter work its magic and guide probing
        auto sf_res = snoopFilter->lookupSnoop(pkt);
        // No timing here: packetFinishTime += sf_res.second * clockPeriod();
        DPRINTF(XBarControl, "recvTimingSnoopReq: src %s %s 0x%x"\
                " SF size: %i lat: %i\n", masterPorts[master_port_id]->name(),
                pkt->cmdString(), pkt->getAddr(), sf_res.first.size(),
                sf_res.second);

        // forward to all snoopers
        forwardTiming(pkt, InvalidPortID, sf_res.first);
    } else {
        forwardTiming(pkt, InvalidPortID);
    }

    // a snoop request came from a connected slave device (one of
    // our master ports), and if it is not coming from the slave
    // device responsible for the address range something is
    // wrong, hence there is nothing further to do as the packet
    // would be going back to where it came from
    assert(master_port_id == findPort(pkt->getAddr()));
}

bool
XBarControl::recvTimingSnoopResp(PacketPtr pkt, PortID slave_port_id)
{
    // determine the source port based on the id
    SlavePort* src_port = slavePorts[slave_port_id];

    // get the destination from the packet
    PortID dest_port_id = pkt->getDest();

    // determine if the response is from a snoop request we
    // created as the result of a normal request (in which case it
    // should be in the outstandingReq), or if we merely forwarded
    // someone else's snoop request
    bool forwardAsSnoop = outstandingReq.find(pkt->req) ==
        outstandingReq.end();

    // test if the crossbar should be considered occupied for the
    // current port, note that the check is bypassed if the response
    // is being passed on as a normal response since this is occupying
    // the response layer rather than the snoop response layer
    if (forwardAsSnoop) {
        if (!snoopLayers[dest_port_id]->tryTiming(src_port)) {
            DPRINTF(XBarControl, "recvTimingSnoopResp: src %s %s 0x%x BUSY\n",
                    src_port->name(), pkt->cmdString(), pkt->getAddr());
            return false;
        }
    } else {
        // get the master port that mirrors this slave port internally
        MasterPort* snoop_port = snoopRespPorts[slave_port_id];
        if (!respLayers[dest_port_id]->tryTiming(snoop_port)) {
            DPRINTF(XBarControl, "recvTimingSnoopResp: src %s %s 0x%x BUSY\n",
                    snoop_port->name(), pkt->cmdString(), pkt->getAddr());
            return false;
        }
    }

    DPRINTF(XBarControl, "recvTimingSnoopResp: src %s %s 0x%x\n",
            src_port->name(), pkt->cmdString(), pkt->getAddr());

    // store size and command as they might be modified when
    // forwarding the packet
    unsigned int pkt_size = pkt->hasData() ? pkt->getSize() : 0;
    unsigned int pkt_cmd = pkt->cmdToIndex();

    // responses are never express snoops
    assert(!pkt->isExpressSnoop());

    calcPacketTiming(pkt);
    Tick packetFinishTime = pkt->lastWordDelay + curTick();

    // forward it either as a snoop response or a normal response
    if (forwardAsSnoop) {
        // this is a snoop response to a snoop request we forwarded,
        // e.g. coming from the L1 and going to the L2, and it should
        // be forwarded as a snoop response

        if (snoopFilter) {
            // update the probe filter so that it can properly track the line
            snoopFilter->updateSnoopForward(pkt, *slavePorts[slave_port_id],
                                            *masterPorts[dest_port_id]);
        }

        bool success M5_VAR_USED =
            masterPorts[dest_port_id]->sendTimingSnoopResp(pkt);
        pktCount[slave_port_id][dest_port_id]++;
        pktSize[slave_port_id][dest_port_id] += pkt_size;
        assert(success);

        snoopLayers[dest_port_id]->succeededTiming(packetFinishTime);
    } else {
        // we got a snoop response on one of our slave ports,
        // i.e. from a coherent master connected to the crossbar, and
        // since we created the snoop request as part of recvTiming,
        // this should now be a normal response again
        outstandingReq.erase(pkt->req);

        // this is a snoop response from a coherent master, with a
        // destination field set on its way through the crossbar as
        // request, hence it should never go back to where the snoop
        // response came from, but instead to where the original
        // request came from
        assert(slave_port_id != dest_port_id);

        if (snoopFilter) {
            // update the probe filter so that it can properly track the line
            snoopFilter->updateSnoopResponse(pkt, *slavePorts[slave_port_id],
                                    *slavePorts[dest_port_id]);
        }

        DPRINTF(XBarControl, "recvTimingSnoopResp: src %s %s 0x%x"\
                " FWD RESP\n", src_port->name(), pkt->cmdString(),
                pkt->getAddr());

        // as a normal response, it should go back to a master through
        // one of our slave ports, at this point we are ignoring the
        // fact that the response layer could be busy and do not touch
        // its state
        bool success M5_VAR_USED =
            slavePorts[dest_port_id]->sendTimingResp(pkt);

        // @todo Put the response in an internal FIFO and pass it on
        // to the response layer from there

        // currently it is illegal to block responses... can lead
        // to deadlock
        assert(success);

        respLayers[dest_port_id]->succeededTiming(packetFinishTime);
    }

    // stats updates
    transDist[pkt_cmd]++;
    snoops++;

    return true;
}


void
XBarControl::forwardTiming(PacketPtr pkt, PortID exclude_slave_port_id,
                           const std::vector<SlavePort*>& dests)
{
    DPRINTF(XBarControl, "%s for %s address %x size %d\n", __func__,
            pkt->cmdString(), pkt->getAddr(), pkt->getSize());

    // snoops should only happen if the system isn't bypassing caches
    assert(!system->bypassCaches());

    unsigned fanout = 0;

    for (const auto& p: dests) {
        // we could have gotten this request from a snooping master
        // (corresponding to our own slave port that is also in
        // snoopPorts) and should not send it back to where it came
        // from
        if (exclude_slave_port_id == InvalidPortID ||
            p->getId() != exclude_slave_port_id) {
            // cache is not allowed to refuse snoop
            p->sendTimingSnoopReq(pkt);
            fanout++;
        }
    }

    // Stats for fanout of this forward operation
    snoopFanout.sample(fanout);
}

void
XBarControl::recvRetry(PortID master_port_id)
{
    // responses and snoop responses never block on forwarding them,
    // so the retry will always be coming from a port to which we
    // tried to forward a request
    /*if(system->migrating && readDone && !writeDone)
    {
      if((master_port_id == 0 && dramMigLocation < 64) || (master_port_id ==1 && pcmMigLocation < 64))
      {
        DPRINTF(Migration,"sendRetry,processWriteToMemory()\n");
            processWriteToMemory();
            return;
      }
    }
    if(master_port_id == 0)
    {//PCM
        if(!invalidPcmWbPacket.empty())
            processPcmInvalidWb();
        else
            reqLayers[master_port_id]->recvRetry();
    }
    else
    {
        if(!invalidDramWbPacket.empty())
            processDramInvalidWb();
        else
            reqLayers[master_port_id]->recvRetry();
    }*/
    reqLayers[master_port_id]->recvRetry();

}

Tick
XBarControl::recvAtomic(PacketPtr pkt, PortID slave_port_id)
{
    DPRINTF(XBarControl, "recvAtomic: packet src %s addr 0x%x cmd %s\n",
            slavePorts[slave_port_id]->name(), pkt->getAddr(),
            pkt->cmdString());

    unsigned int pkt_size = pkt->hasData() ? pkt->getSize() : 0;
    unsigned int pkt_cmd = pkt->cmdToIndex();

    MemCmd snoop_response_cmd = MemCmd::InvalidCmd;
    Tick snoop_response_latency = 0;

    // uncacheable requests need never be snooped
    if (!pkt->req->isUncacheable() && !system->bypassCaches()) {
        // forward to all snoopers but the source
        std::pair<MemCmd, Tick> snoop_result;
        if (snoopFilter) {
            // check with the snoop filter where to forward this packet
            auto sf_res =
                snoopFilter->lookupRequest(pkt, *slavePorts[slave_port_id]);
            snoop_response_latency += sf_res.second * clockPeriod();
            DPRINTF(XBarControl, "%s: src %s %s 0x%x"\
                    " SF size: %i lat: %i\n", __func__,
                    slavePorts[slave_port_id]->name(), pkt->cmdString(),
                    pkt->getAddr(), sf_res.first.size(), sf_res.second);
            snoop_result = forwardAtomic(pkt, slave_port_id, InvalidPortID,
                                         sf_res.first);
        } else {
            snoop_result = forwardAtomic(pkt, slave_port_id);
        }
        snoop_response_cmd = snoop_result.first;
        snoop_response_latency += snoop_result.second;
    }

    // even if we had a snoop response, we must continue and also
    // perform the actual request at the destination
    PortID master_port_id = findPort(pkt->getAddr());

    // stats updates for the request
    pktCount[slave_port_id][master_port_id]++;
    pktSize[slave_port_id][master_port_id] += pkt_size;
    transDist[pkt_cmd]++;

    // forward the request to the appropriate destination
    Tick response_latency = masterPorts[master_port_id]->sendAtomic(pkt);

    // Lower levels have replied, tell the snoop filter
    if (snoopFilter && !pkt->req->isUncacheable() && !system->bypassCaches() &&
        pkt->isResponse()) {
        snoopFilter->updateResponse(pkt, *slavePorts[slave_port_id]);
    }

    // if we got a response from a snooper, restore it here
    if (snoop_response_cmd != MemCmd::InvalidCmd) {
        // no one else should have responded
        assert(!pkt->isResponse());
        pkt->cmd = snoop_response_cmd;
        response_latency = snoop_response_latency;
    }

    // add the response data
    if (pkt->isResponse()) {
        pkt_size = pkt->hasData() ? pkt->getSize() : 0;
        pkt_cmd = pkt->cmdToIndex();

        // stats updates
        pktCount[slave_port_id][master_port_id]++;
        pktSize[slave_port_id][master_port_id] += pkt_size;
        transDist[pkt_cmd]++;
    }

    // @todo: Not setting first-word time
    pkt->lastWordDelay = response_latency;
    return response_latency;
}

Tick
XBarControl::recvAtomicSnoop(PacketPtr pkt, PortID master_port_id)
{
    DPRINTF(XBarControl, "recvAtomicSnoop: packet src %s addr 0x%x cmd %s\n",
            masterPorts[master_port_id]->name(), pkt->getAddr(),
            pkt->cmdString());

    // add the request snoop data
    snoops++;

    // forward to all snoopers
    std::pair<MemCmd, Tick> snoop_result;
    Tick snoop_response_latency = 0;
    if (snoopFilter) {
        auto sf_res = snoopFilter->lookupSnoop(pkt);
        snoop_response_latency += sf_res.second * clockPeriod();
        DPRINTF(XBarControl, "%s: src %s %s 0x%x SF size: %i lat: %i\n",
                __func__, masterPorts[master_port_id]->name(), pkt->cmdString(),
                pkt->getAddr(), sf_res.first.size(), sf_res.second);
        snoop_result = forwardAtomic(pkt, InvalidPortID, master_port_id,
                                     sf_res.first);
    } else {
        snoop_result = forwardAtomic(pkt, InvalidPortID);
    }
    MemCmd snoop_response_cmd = snoop_result.first;
    snoop_response_latency += snoop_result.second;

    if (snoop_response_cmd != MemCmd::InvalidCmd)
        pkt->cmd = snoop_response_cmd;

    // add the response snoop data
    if (pkt->isResponse()) {
        snoops++;
    }

    // @todo: Not setting first-word time
    pkt->lastWordDelay = snoop_response_latency;
    return snoop_response_latency;
}

std::pair<MemCmd, Tick>
XBarControl::forwardAtomic(PacketPtr pkt, PortID exclude_slave_port_id,
                           PortID source_master_port_id,
                           const std::vector<SlavePort*>& dests)
{
    // the packet may be changed on snoops, record the original
    // command to enable us to restore it between snoops so that
    // additional snoops can take place properly
    MemCmd orig_cmd = pkt->cmd;
    MemCmd snoop_response_cmd = MemCmd::InvalidCmd;
    Tick snoop_response_latency = 0;

    // snoops should only happen if the system isn't bypassing caches
    assert(!system->bypassCaches());

    unsigned fanout = 0;

    for (const auto& p: dests) {
        // we could have gotten this request from a snooping master
        // (corresponding to our own slave port that is also in
        // snoopPorts) and should not send it back to where it came
        // from
        if (exclude_slave_port_id != InvalidPortID &&
            p->getId() == exclude_slave_port_id)
            continue;

        Tick latency = p->sendAtomicSnoop(pkt);
        fanout++;

        // in contrast to a functional access, we have to keep on
        // going as all snoopers must be updated even if we get a
        // response
        if (!pkt->isResponse())
            continue;

        // response from snoop agent
        assert(pkt->cmd != orig_cmd);
        assert(pkt->memInhibitAsserted());
        // should only happen once
        assert(snoop_response_cmd == MemCmd::InvalidCmd);
        // save response state
        snoop_response_cmd = pkt->cmd;
        snoop_response_latency = latency;

        if (snoopFilter) {
            // Handle responses by the snoopers and differentiate between
            // responses to requests from above and snoops from below
            if (source_master_port_id != InvalidPortID) {
                // Getting a response for a snoop from below
                assert(exclude_slave_port_id == InvalidPortID);
                snoopFilter->updateSnoopForward(pkt, *p,
                             *masterPorts[source_master_port_id]);
            } else {
                // Getting a response for a request from above
                assert(source_master_port_id == InvalidPortID);
                snoopFilter->updateSnoopResponse(pkt, *p,
                             *slavePorts[exclude_slave_port_id]);
            }
        }
        // restore original packet state for remaining snoopers
        pkt->cmd = orig_cmd;
    }

    // Stats for fanout
    snoopFanout.sample(fanout);

    // the packet is restored as part of the loop and any potential
    // snoop response is part of the returned pair
    return std::make_pair(snoop_response_cmd, snoop_response_latency);
}

void
XBarControl::recvFunctional(PacketPtr pkt, PortID slave_port_id)
{
//light
if(pkt->cmd==MemCmd::PrintReq)
{
DPRINTF(Migration,"have received functional\n");
  numCpu--;
  if(numCpu == 0)
  {
    schedule(MigratingEvent, nextCycle());
  }
  
}
else
{
        if (!pkt->isPrint()) {
            // don't do DPRINTFs on PrintReq as it clutters up the output
            DPRINTF(XBarControl,
                    "recvFunctional: packet src %s addr 0x%x cmd %s\n",
                    slavePorts[slave_port_id]->name(), pkt->getAddr(),
                    pkt->cmdString());

        }

        // uncacheable requests need never be snooped
        if (!pkt->req->isUncacheable() && !system->bypassCaches()) {

            // forward to all snoopers but the source
            forwardFunctional(pkt, slave_port_id);
        }

        // there is no need to continue if the snooping has found what we
        // were looking for and the packet is already a response
        if (!pkt->isResponse()) {
            PortID dest_id = findPort(pkt->getAddr());

            masterPorts[dest_id]->sendFunctional(pkt);
        }
}

//end
}

void
XBarControl::recvFunctionalSnoop(PacketPtr pkt, PortID master_port_id)
{
    if (!pkt->isPrint()) {
        // don't do DPRINTFs on PrintReq as it clutters up the output
        DPRINTF(XBarControl,
                "recvFunctionalSnoop: packet src %s addr 0x%x cmd %s\n",
                masterPorts[master_port_id]->name(), pkt->getAddr(),
                pkt->cmdString());
    }

    // forward to all snoopers
    forwardFunctional(pkt, InvalidPortID);
}

void
XBarControl::forwardFunctional(PacketPtr pkt, PortID exclude_slave_port_id)
{
    // snoops should only happen if the system isn't bypassing caches
    assert(!system->bypassCaches());

    for (const auto& p: snoopPorts) {
        // we could have gotten this request from a snooping master
        // (corresponding to our own slave port that is also in
        // snoopPorts) and should not send it back to where it came
        // from
        if (exclude_slave_port_id == InvalidPortID ||
            p->getId() != exclude_slave_port_id)
            p->sendFunctionalSnoop(pkt);

        // if we get a response we are done
        if (pkt->isResponse()) {
            break;
        }
    }
}

unsigned int
XBarControl::drain(DrainManager *dm)
{
    // sum up the individual layers
    unsigned int total = 0;
    for (auto l: reqLayers)
        total += l->drain(dm);
    for (auto l: respLayers)
        total += l->drain(dm);
    for (auto l: snoopLayers)
        total += l->drain(dm);
    return total;
}

void
XBarControl::regStats()
{
    // register the stats of the base class and our layers
    BaseXBar::regStats();
    for (auto l: reqLayers)
        l->regStats();
    for (auto l: respLayers)
        l->regStats();
    for (auto l: snoopLayers)
        l->regStats();

    snoops
        .name(name() + ".snoops")
        .desc("Total snoops (count)")
    ;
//light
    migrationTick
        .name(name() +".ticks")
        .desc("average migaration tick")
    ;
    pageMigrationNumber
        .name(name() + ".pageMigrations")
        .desc("total pageMigrations")
    ;
//end
    snoopFanout
        .init(0, snoopPorts.size(), 1)
        .name(name() + ".snoop_fanout")
        .desc("Request fanout histogram")
    ;
}

XBarControl *
XBarControlParams::create()
{
    return new XBarControl(this);
}
