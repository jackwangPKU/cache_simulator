/*
 * This program simulates a LRU-based last level cache with only one slice.
 * The target is to count the misses of all the sets.
 * Usage: g++ -std=c++11 cal_set.cpp -o cal_set
 *        ./cal_set [benmark1] [benchmark2]
 * Input: follow the hints
 * Output: the misses of all the sets, saved as [benchmark1]_[benchmark2]
 * Author: Jack Wang
 * Date: 2019.10.11
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <unordered_map>

using namespace std;
char benchname1[20], benchname2[20];
char filename1[30], filename2[30], outfilename[100];
FILE *file1, *file2, *outfile;
int set_bits = 11, block_bits = 6, ways = 11;
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

Node *head[SETS], *tail[SETS];
unordered_map<unsigned long long, int> tag_line_no[SETS];
unordered_map<int, Node*> line_no_ptr[SETS];

bool belong(unsigned long long tag)
{
    if((tag>>(53-set_bits-block_bits)) == 1)
        return true;
    else
        return false;
}

void Init()
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

void Refresh(unsigned long long set_no, unsigned long long tag)
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

void Replace(unsigned long long set_no, unsigned long long newtag)
{
    int line_no_tmp = tail[set_no]->line_no;
    unsigned long long oldtag = tail[set_no]->tag;
    tag_line_no[set_no].erase(oldtag);
    tag_line_no[set_no][newtag] = line_no_tmp;
    tail[set_no]->tag = newtag;
    Refresh(set_no, newtag);
    return;
}

void Start()
{
    file1 = fopen(filename1, "r");
    file2 = fopen(filename2, "r");
    outfile = fopen(outfilename, "w");
    if(file1 == NULL || file2 == NULL || outfile == NULL)
    {
        printf("cannot open files\n");
        exit(1);
    }

    Init();

    return;
}

void Finish()
{
    fclose(file1);
    fclose(file2);
    fclose(outfile);
    return;
}

int main(int argc, char *argv[])
{
    unsigned long long miss_count[SETS];
    for(int i = 0; i<SETS; i++)
        miss_count[i] = 0;
    printf("please input ratio: ");
    scanf("%d", &ratio);
    strcpy(benchname1, argv[1]);
    strcpy(benchname2, argv[2]);
    strcpy(filename1, benchname1);
    strcpy(filename2, benchname2);
    strcat(filename1, ".out");
    strcat(filename2, ".out");
    strcpy(outfilename, benchname1);
    strcat(outfilename, "_");
    strcat(outfilename, benchname2);

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
        // printf("addr1: %llu\n", addr1);
        // printf("addr2: %llu\n", addr2);
        // printf("set_no1: %llu\n", set_no1);
        // printf("set_no2: %llu\n\n", set_no2);

        if(tag_line_no[set_no1].find(tag1) != tag_line_no[set_no1].end()) // found
            Refresh(set_no1, tag1);
        else    // not found
        {
            miss_count[set_no1]++;
            if(tag_line_no[set_no1].size() < ways)   // not full
            {
                int allocated_line_no = tag_line_no[set_no1].size();
                tag_line_no[set_no1][tag1] = allocated_line_no;
                Node* tmp = line_no_ptr[set_no1][allocated_line_no];
                tmp->tag = tag1;
                Refresh(set_no1, tag1);
            }
            else // full
            {
                Replace(set_no1, tag1);
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

                if(tag_line_no[set_no1].find(tag1) != tag_line_no[set_no1].end()) // found
                    Refresh(set_no1, tag1);
                else    // not found
                {
                    miss_count[set_no1]++;
                    if(tag_line_no[set_no1].size() < ways)   // not full
                    {
                        int allocated_line_no = tag_line_no[set_no1].size();
                        tag_line_no[set_no1][tag1] = allocated_line_no;
                        Node* tmp = line_no_ptr[set_no1][allocated_line_no];
                        tmp->tag = tag1;
                        Refresh(set_no1, tag1);
                    }
                    else // full
                    {
                        Replace(set_no1, tag1);
                    }
                }
            }
            else
                break;
        }

        if(tag_line_no[set_no2].find(tag2) != tag_line_no[set_no2].end()) // found
            Refresh(set_no2, tag2);
        else    // not found
        {
            miss_count[set_no2]++;
            if(tag_line_no[set_no2].size() < ways)   // not full
            {
                int allocated_line_no = tag_line_no[set_no2].size();
                tag_line_no[set_no2][tag2] = allocated_line_no;
                Node* tmp = line_no_ptr[set_no2][allocated_line_no];
                tmp->tag = tag2;
                Refresh(set_no2, tag2);
            }
            else // full
            {
                Replace(set_no2, tag2);
            }
        }
    }

    for(int i = 0; i<SETS; i++)
        fprintf(outfile, "%llu\n", miss_count[i]);

    Finish();
    return 0;
}