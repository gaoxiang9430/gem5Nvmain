#ifndef __HSETBLOOMFILTER_HH__
#define __HSETBLOOMFILTER_HH__

class HSetBloomFilter
{
public:
    HSetBloomFilter(int cnt,int numBFBits,int numTagBits);
    ~HSetBloomFilter();
    void clearbloomfilter();
    void insertTags(int tag);

    bool testTags(int tag);
private:
    int counter;
    char* bloomfilter;
    int FILTE_NUM;
    int FILTE_NUM_BIT;
    int TAG_NUM_BIT;
    int getFliter(unsigned short* tags);
};
#endif
