/* 
 * This program simulates a LRU-based last level cache with several slices, based on cal_set.cpp.
 * The target is to count the accesses and misses of all the sets.
 * Precondition: same as cal_set.cpp.
 * Usage: g++ -std=c++11 cal_set_slice.cpp -o cal_set_slice
 *        ./cal_set_slice [benchmark1] [benchmark2]
 * Input: follow the hints
 * Output: the accesses and misses of all the sets, saved as [benchmark1]_[benchmark2]_access and
 *         [benchmark1]_[benchmark2]_miss
 * Author: Jack Wang
 * Date: 2019.10.16
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <unordered_map>
using namespace std;

char benchname1[20], benchname2[20];
char filename1[30], filename2[30], outfilename1[100], outfilename2[100];
FILE *file1, *file2, *outfile1, *outfile2;
int slices = 8, set_bits = 11, block_bits = 6, ways = 11;
unsigned long long addr1, addr2;
int ratio;    // benchmark1:benchmark2
#define SETS 2048    // 2^set_bits

struct Node
{
    Node *pre;
    Node *next;
    int line_no;
    unsigned long long tag;
    Node(int a) 
    {
        line_no = a;
        pre = NULL;
        next = NULL;
    }
};

class Cache_slice
{
public:
    Node *head[SETS], *tail[SETS];
    unordered_map<unsigned long long, int> tag_line_no[SETS];
    unordered_map<int, Node*> line_no_ptr[SETS];

    Cache_slice();
    // ~Cache_slice();
    void Refresh(unsigned long long set_no, unsigned long long tag);
    void Replace(unsigned long long set_no, unsigned long long newtag);

};

Cache_slice::Cache_slice()
{
    for(int k = 0; k<SETS; k++)
    {
        Node *tmp_pre = NULL, *tmp_next = NULL;
        head[k] = new Node(0);
        tmp_pre = head[k];
        for(int i = 1; i<ways; i++)
        {
            tmp_next = new Node(i);
            tmp_pre->next = tmp_next;
            tmp_next->pre = tmp_pre;
            tmp_pre = tmp_next;
        }
        tail[k] = tmp_pre;

        Node *tmp = head[k];
        for(int i = 0; i<ways; i++)
        {
            line_no_ptr[k].insert(make_pair(i, tmp));
            tmp = tmp->next;
        }
    }
}

void Cache_slice::Refresh(unsigned long long set_no, unsigned long long tag)
{
    int line_no_tmp = tag_line_no[set_no][tag];
    if(line_no_tmp >= ways || line_no_tmp < 0)
    {
        printf("line_no_tmp >= ways || line_no_tmp < 0\n");
        exit(1);
    }
    Node *tmp = line_no_ptr[set_no][line_no_tmp];
    if(head[set_no] != tmp)
    {
        if(tmp->pre != NULL)
            tmp->pre->next = tmp->next;
        if(tmp->next != NULL)
            tmp->next->pre = tmp->pre;
        if(tail[set_no] == tmp)
            tail[set_no] = tmp->pre;
        tmp->next = head[set_no];
        head[set_no]->pre = tmp;
        tmp->pre = NULL;
        head[set_no] = tmp;
    }
    return;
}

void Cache_slice::Replace(unsigned long long set_no, unsigned long long newtag)
{
    int line_no_tmp = tail[set_no]->line_no;
    unsigned long long oldtag = tail[set_no]->tag;
    tag_line_no[set_no].erase(oldtag);
    tag_line_no[set_no][newtag] = line_no_tmp;
    tail[set_no]->tag = newtag;
    Refresh(set_no, newtag);
    return;
}

Cache_slice *Cache;

void Start()
{
    file1 = fopen(filename1, "r");
    file2 = fopen(filename2, "r");
    outfile1 = fopen(outfilename1, "w");
    outfile2 = fopen(outfilename2, "w");
    if(file1 == NULL || file2 == NULL || outfile1 == NULL || outfile2 == NULL)
    {
        printf("cannot open files\n");
        exit(1);
    }

    Cache = new Cache_slice[slices];

    return;
}

void Finish()
{
    delete []Cache;

    fclose(file1);
    fclose(file2);
    fclose(outfile1);
    fclose(outfile2);

    return;
}

int Cal_slice(unsigned long long addr)
{
    unsigned long long result = 0;
    result += ((addr>>37)&1) ^ ((addr>>36)&1) ^ ((addr>>35)&1) ^ ((addr>>34)&1) ^ ((addr>>31)&1) ^ ((addr>>30)&1)
              ^ ((addr>>27)&1) ^ ((addr>>26)&1) ^ ((addr>>23)&1) ^ ((addr>>22)&1) ^ ((addr>>19)&1) ^ ((addr>>16)&1)
              ^ ((addr>>13)&1) ^ ((addr>>12)&1) ^ ((addr>>8)&1);
    result = result << 1;
    result += ((addr>>37)&1) ^ ((addr>>35)&1) ^ ((addr>>34)&1) ^ ((addr>>33)&1) ^ ((addr>>31)&1) ^ ((addr>>29)&1)
              ^ ((addr>>28)&1) ^ ((addr>>26)&1) ^ ((addr>>24)&1) ^ ((addr>>23)&1) ^ ((addr>>22)&1) ^ ((addr>>21)&1)
              ^ ((addr>>20)&1) ^ ((addr>>19)&1) ^ ((addr>>17)&1) ^ ((addr>>15)&1) ^ ((addr>>13)&1) ^ ((addr>>11)&1)
              ^ ((addr>>7)&1);
    result = result << 1;
    result += ((addr>>36)&1) ^ ((addr>>35)&1) ^ ((addr>>33)&1) ^ ((addr>>32)&1) ^ ((addr>>30)&1) ^ ((addr>>28)&1)
              ^ ((addr>>27)&1) ^ ((addr>>26)&1) ^ ((addr>>25)&1) ^ ((addr>>24)&1) ^ ((addr>>22)&1) ^ ((addr>>20)&1)
              ^ ((addr>>18)&1) ^ ((addr>>17)&1) ^ ((addr>>16)&1) ^ ((addr>>14)&1) ^ ((addr>>12)&1) ^ ((addr>>10)&1)
              ^ ((addr>>6)&1);
    return result;
}

int main(int argc, char *argv[])
{
    unsigned long long count[slices][SETS];
    unsigned long long miss_count[slices][SETS];
    for(int i = 0; i<slices; i++)
        for(int j = 0; j<SETS; j++)
        {
            count[i][j] = 0;
            miss_count[i][j] = 0;
        }
        
    printf("please input ratio: ");
    scanf("%d", &ratio);
    strcpy(benchname1, argv[1]);
    strcpy(benchname2, argv[2]);
    strcpy(filename1, benchname1);
    strcpy(filename2, benchname2);
    strcat(filename1, ".out");
    strcat(filename2, ".out");
    strcpy(outfilename1, benchname1);
    strcat(outfilename1, "_");
    strcat(outfilename1, benchname2);
    strcat(outfilename1, "_access");
    strcpy(outfilename2, benchname1);
    strcat(outfilename2, "_");
    strcat(outfilename2, benchname2);
    strcat(outfilename2, "_miss");

    Start();

    char tmp1[100], tmp2[100];
    while(fgets(tmp1, 99, file1) != NULL && fgets(tmp2, 99, file2) != NULL)   // end with either file finished
    {
        addr1 = strtoull(tmp1, NULL, 10);
        addr2 = strtoull(tmp2, NULL, 10);
        addr1 = addr1 + ((unsigned long long)1<<53);  // distinguish different benchmark
        unsigned long long tag1 = addr1 >> (set_bits+block_bits);
        unsigned long long tag2 = addr2 >> (set_bits+block_bits);
        unsigned long long set_no1 = (addr1 >> block_bits) & 0B11111111111;    // set_bits
        unsigned long long set_no2 = (addr2 >> block_bits) & 0B11111111111;
        int slice1 = Cal_slice(addr1);
        int slice2 = Cal_slice(addr2);

        count[slice1][set_no1]++;
        if(Cache[slice1].tag_line_no[set_no1].find(tag1) != Cache[slice1].tag_line_no[set_no1].end()) // found
            Cache[slice1].Refresh(set_no1, tag1);
        else    // not found
        {
            miss_count[slice1][set_no1]++;
            if(Cache[slice1].tag_line_no[set_no1].size() < ways)   // not full
            {
                int allocated_line_no = Cache[slice1].tag_line_no[set_no1].size();
                Cache[slice1].tag_line_no[set_no1][tag1] = allocated_line_no;
                Node* tmp = Cache[slice1].line_no_ptr[set_no1][allocated_line_no];
                tmp->tag = tag1;
                Cache[slice1].Refresh(set_no1, tag1);
            }
            else // full
            {
                Cache[slice1].Replace(set_no1, tag1);
            }
        }


        int counter = ratio - 1;
        while(counter--)
        {
            if(fgets(tmp1, 99, file1) != NULL)
            {
                addr1 = strtoull(tmp1, NULL, 10);
                addr1 = addr1 + ((unsigned long long)1<<53);  // distinguish different benchmark
                tag1 = addr1 >> (set_bits+block_bits);
                set_no1 = (addr1 >> block_bits) & 0B011111111111;    // set_bits
                slice1 = Cal_slice(addr1);

                count[slice1][set_no1]++;
                if(Cache[slice1].tag_line_no[set_no1].find(tag1) != Cache[slice1].tag_line_no[set_no1].end()) // found
                    Cache[slice1].Refresh(set_no1, tag1);
                else    // not found
                {
                    miss_count[slice1][set_no1]++;
                    if(Cache[slice1].tag_line_no[set_no1].size() < ways)   // not full
                    {
                        int allocated_line_no = Cache[slice1].tag_line_no[set_no1].size();
                        Cache[slice1].tag_line_no[set_no1][tag1] = allocated_line_no;
                        Node* tmp = Cache[slice1].line_no_ptr[set_no1][allocated_line_no];
                        tmp->tag = tag1;
                        Cache[slice1].Refresh(set_no1, tag1);
                    }
                    else // full
                    {
                        Cache[slice1].Replace(set_no1, tag1);
                    }
                }
            }
            else
                break;
        }

        count[slice2][set_no2]++;
        if(Cache[slice2].tag_line_no[set_no2].find(tag2) != Cache[slice2].tag_line_no[set_no2].end()) // found
            Cache[slice2].Refresh(set_no2, tag2);
        else    // not found
        {
            miss_count[slice2][set_no2]++;
            if(Cache[slice2].tag_line_no[set_no2].size() < ways)   // not full
            {
                int allocated_line_no = Cache[slice2].tag_line_no[set_no2].size();
                Cache[slice2].tag_line_no[set_no2][tag2] = allocated_line_no;
                Node* tmp = Cache[slice2].line_no_ptr[set_no2][allocated_line_no];
                tmp->tag = tag2;
                Cache[slice2].Refresh(set_no2, tag2);
            }
            else // full
            {
                Cache[slice2].Replace(set_no2, tag2);
            }
        }
    }

    for(int i = 0; i<slices; i++)
        for(int j = 0; j<SETS; j++)
        {
            fprintf(outfile1, "%llu\n", count[i][j]);
            fprintf(outfile2, "%llu\n", miss_count[i][j]);
        }
    
    Finish();
    
    return 0;
}