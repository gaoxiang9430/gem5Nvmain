#ifndef __SETBLOOMFILTER_HH__
#define __SETBLOOMFILTER_HH__

#include "base/bitfield.hh"
typedef struct
{
    bool valid;
    int tags;
}EAF;
class SetBloomFilter
{
public:
    //int counter;
    //uint64_t boomfilter;
    EAF *eaf;
    /*SetBloomFilter()
    {
        counter = 0;
        bloomfilter = 0;
    }
    
    ~SetBloomFilter() {};*/
    void insertTags(int tag)
    {
        bool flags = false;
        for(unsigned i = 0;i<16;i++)
        {
            if((eaf[i]).valid == false)
            {
                (eaf[i]).valid = true;
                (eaf[i]).tags = tag;
                flags = true;
                break;
            }
        }
        
        if(!flags)
        {
            for(unsigned i = 0 ; i < 15;i++)
            {
                eaf[i] = eaf[i+1];
            }
            (eaf[15]).valid = true;
            (eaf[15]).tags = tag; 
        }
            
    }
    bool testTags(int tag)
    {
        bool isFound = false;
        for(unsigned i = 0;i<16;i++)
        {
            if((eaf[i]).valid == true && (eaf[i]).tags == tag)
            {
                isFound = true;
                break;
            }
        }
        if(isFound)
            return true;
       return false;
    }

};



#endif
