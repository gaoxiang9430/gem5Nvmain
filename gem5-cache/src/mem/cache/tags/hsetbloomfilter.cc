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
 * Definitions of a Hsetbloomfilter
 */
//light
#include "debug/BloomFilter.hh"
 #include "base/trace.hh"
//end
#include "mem/cache/tags/hsetbloomfilter.hh"
#include "base/intmath.hh"
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>
#define FUNC_NUM 3
HSetBloomFilter::HSetBloomFilter(int cnt,int numBFBits,int numTagBits)
{
    counter = cnt;
    bloomfilter = (char*)malloc(sizeof(char) * numBFBits);
    memset(bloomfilter,0,numBFBits);
    FILTE_NUM = ceilLog2(numBFBits);
    FILTE_NUM_BIT = numBFBits;
    TAG_NUM_BIT = numTagBits; 
}

HSetBloomFilter::~HSetBloomFilter()
{
    free(bloomfilter);
}
void HSetBloomFilter::clearbloomfilter()
{
    counter = 0;
    memset(bloomfilter,0,FILTE_NUM_BIT);
    DPRINTF(BloomFilter,"clear bloom filter\n");
}

int
HSetBloomFilter:: getFliter(unsigned short* tags)
{
    short* fliter = new short[FILTE_NUM];          
    int group1_start=0,group2_start=TAG_NUM_BIT - 1;

    for(int i=0;i<FILTE_NUM ;i++)
    {
        fliter[i] = tags[group1_start] ^ tags[group2_start];   //对原始二进制数组进行异或运算
        group1_start++;
        group2_start--;
    }
    int temp = 1,result = 0;
    for(int i=FILTE_NUM-1;i>=0;i--)
    {
        if(fliter[i])
             result += temp;          //将二进制数组转换成10进制
        temp = temp<<1;
    }
    delete(fliter);
    return result;
}

void
HSetBloomFilter::insertTags(int tag)
{
    if(counter > 16)
    {
        DPRINTF(BloomFilter,"this set have insert 16 tags\n");
        clearbloomfilter();
    }

    counter ++;
    int tag_number = TAG_NUM_BIT;
    //int skip = (tag_number-(FILTE_NUM<<1) )/2;
   //int start = 0,end = (FILTE_NUM<<1) -1;
    for(int j=0;j<FUNC_NUM;j++)
    {
        unsigned short tags[tag_number];
        for(int k=0;k<tag_number;k++)
            tags[k] = 0;
        int tag_temp = tag;
        for(int i=0;i<tag_number&&tag_temp;i++)  //将10进制数变成二进制数组
        {
            tags[i] = tag_temp&1;
            tag_temp = tag_temp>>1;
        }
        
        int result = 0;
        result = getFliter(tags); //获取过滤器哈希后的二进制数组 
        bloomfilter[result] = 1;
        tag =(tag>>(tag_number-1)) | (tag<<1);  //将原始tag循环移位
       //start += skip;
      //end += skip;
    }
    return;
}
bool
HSetBloomFilter::testTags(int tag)
{
    int tag_number = TAG_NUM_BIT;
    //int skip = (tag_number-(FILTE_NUM<<1) )/2;
    //int start = 0,end = (FILTE_NUM<<1) -1;
    for(int j=0;j<FUNC_NUM;j++)
    {
        unsigned short tags[tag_number];
        for(int k=0;k<tag_number;k++)
            tags[k] = 0;
        int tag_temp = tag;
        for(int i=0;i<tag_number&&tag_temp;i++)  //将10进制数变成二进制数组
        {
            tags[i] = tag_temp&1;
            tag_temp = tag_temp>>1;
        }
        
        int result = 0;
        result = getFliter(tags); //获取过滤器哈希后的二进制数组 
        if(!bloomfilter[result]) return false;
        tag =(tag>>(tag_number-1)) | (tag<<1);  //将原始tag循环移位    
                //start += skip;
        //end += skip;
    }
    return true;
}

