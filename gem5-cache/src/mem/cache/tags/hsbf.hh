#ifndef __HSETBLOOMFILTER_HH__
#define __HSETBLOOMFILTER_HH__
#include "base/intmath.hh"
#define FUNC_NUM 3
class HSetBloomFilter
{
private:
    int counter;
    char* bloomfilter;
    int FILTE_NUM;
    int FILTE_NUM_BIT;
    int getFliter(unsigned short* tags,int start, int end)
    {
        short* fliter = new short[FILTE_NUM];          
        int group1_start=start,group2_start=end;

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
public:
    HSetBloomFilter(int cnt,int numBits)
    {
        counter = cnt;
        bloomfilter = (char*)malloc(sizeof(char) * numBits);
        memset(bloomfilter,0,numBits);
        FILTE_NUM = ceilLog2(numBits);
        FILTE_NUM_BIT = numBits; 
    }
    ~HSetBloomFilter()
    {
        free(bloomfilter);
    }
    void clearbloomfilter()
    {
        counter = 0;
        memset(bloomfilter,0,FILTE_NUM_BIT);
    }
    void insertTags(int tag)
    {
        if(counter > 16)
            clearbloomfilter();
        counter ++;
        int tag_number = 22;
        //int skip = (tag_number-(FILTE_NUM<<1) )/2;
        int start = 0,end = tag_number -1;
        
        unsigned short tags[22];
        for(int k=0;k<tag_number;k++)
            tags[k] = 0;
        int tag_temp = tag;
        for(int i=0;i<tag_number&&tag_temp;i++)  //将10进制数变成二进制数组
        {
            tags[i] = tag_temp&1;
            tag_temp = tag_temp>>1;
        } 

        for(int j=0;j<FUNC_NUM;j++)
        {            
            int result = 0;
            result = getFliter(tags,start,end); //获取过滤器哈希后的二进制数组 
            bloomfilter[result] = 1;
           //tag =(tag>>(number-1)) | (tag<<1);  //将原始tag循环移位
            start = (start+FILTE_NUM)%tag_number;
            end = (end+FILTE_NUM)%tag_number;
        }
        return;
    }
    bool testTags(int tag)
    {
        int tag_number = 22;
        //int skip = (tag_number-(FILTE_NUM<<1) )/2;
        int start = 0,end = tag_number -1;
        
        unsigned short tags[22];
        for(int k=0;k<tag_number;k++)
            tags[k] = 0;
        int tag_temp = tag;
        for(int i=0;i<tag_number&&tag_temp;i++)  //将10进制数变成二进制数组
        {
            tags[i] = tag_temp&1;
            tag_temp = tag_temp>>1;
        } 

        for(int j=0;j<FUNC_NUM;j++)
        {            
            int result = 0;
            result = getFliter(tags,start,end); //获取过滤器哈希后的二进制数组 
            if(!bloomfilter[result]) return false;
           //tag =(tag>>(number-1)) | (tag<<1);  //将原始tag循环移位
            start = (start+FILTE_NUM)%tag_number;
            end = (end+FILTE_NUM)%tag_number;
        }
        return true;
   }

};
#endif


