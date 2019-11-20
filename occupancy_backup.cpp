/*
 * This program simulates a LRU-based cache, which focuses on only one set.
 * The simulator supports overlapping cache allocation, as intel CAT does.
 * The target is to observe the occupancies of two co-run benchmarks.
 * Cache allocation requirement: benchmark1 begins with way0 while benchmark2 ends with way(ways-1). 
 * Usage: g++ -std=c++11 occupancy.cpp -o occupancy
 *        ./occupancy [benchmark1] [benchmark2]
 * Input: follow the hints
 * Output: the occupancies of benchmark1 and benchmark2, saved as 
 *         [benchmark1]_[benchmark2]_[slice_no]_[set_no]_[overlap]_1 and [benchmark1]_[benchmark2]_[slice_no]_[set_no]_[overlap]_2 
 * Author: Jack Wang
 * Date: 2019.10.22
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unordered_map>
using namespace std;
char benchname1[100], benchname2[100];
char filename1[100], filename2[100], outfilename1[100], outfilename2[100];
FILE *file1, *file2, *outfile1, *outfile2;
int slices = 8, set_bits = 11, block_bits = 6, ways = 11;
int step;    // the step of printing
int ratio;    // benchmark1:benchmark2
unsigned long long count = 0, addr1, addr2;
int occupancy1 = 0, occupancy2 = 0; 
int overlap;  // the number of overlapping ways
int begin_way1 = 0, end_way1, begin_way2, end_way2 = ways-1;
unsigned long long chosen_set_no;
int chosen_slice_no;
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

Node *head1, *tail1;
unordered_map<unsigned long long, int> tag_line_no1;
unordered_map<int, Node*> line_no_ptr1;
Node *head2, *tail2;
unordered_map<unsigned long long, int> tag_line_no2;
unordered_map<int, Node*> line_no_ptr2;

bool belong(unsigned long long tag)     // addr belongs to benchmark1
{
    if((tag>>(53-set_bits-block_bits)) == 1)
        return true;
    else
        return false;
}

void Init()
{
    // benchmark1
    Node *tmp_pre = NULL, *tmp_next = NULL;
    head1 = new Node(begin_way1);
    tmp_pre = head1;
    for(int i = begin_way1+1; i<=end_way1; i++)
    {
        tmp_next = new Node(i);
        tmp_pre->next = tmp_next;
        tmp_next->pre = tmp_pre;
        tmp_pre = tmp_next;
    }
    tail1 = tmp_pre;

    Node *tmp = head1;
    for(int i = begin_way1; i<=end_way1; i++)
    {
        line_no_ptr1.insert(make_pair(i, tmp));
        tmp = tmp->next;
    }

    // benchmark2
    tmp_pre = NULL;
    tmp_next = NULL;
    head2 = new Node(end_way2);
    tmp_pre = head2;
    for(int i = end_way2-1; i>=begin_way2; i--)
    {
        tmp_next = new Node(i);
        tmp_pre->next = tmp_next;
        tmp_next->pre = tmp_pre;
        tmp_pre = tmp_next;
    }
    tail2 = tmp_pre;

    tmp = head2;
    for(int i = end_way2; i>=begin_way2; i--)
    {
        line_no_ptr2.insert(make_pair(i, tmp));
        tmp = tmp->next;
    }
}

void Refresh1(int line_no_tmp)
{
    if(line_no_tmp > end_way1 || line_no_tmp < begin_way1)
    {
        printf("Refresh1 error\n");
        exit(1);
    }
    Node *tmp = line_no_ptr1[line_no_tmp];
    if(head1 != tmp)
    {
        if(tmp->pre != NULL)
            tmp->pre->next = tmp->next;
        if(tmp->next != NULL)
            tmp->next->pre = tmp->pre;
        if(tail1 == tmp)
            tail1 = tmp->pre;
        tmp->next = head1;
        head1->pre = tmp;
        tmp->pre = NULL;
        head1 = tmp;
    }
    return;
}

void Refresh2(int line_no_tmp)
{
    if(line_no_tmp > end_way2 || line_no_tmp < begin_way2)
    {
        printf("Refresh2 error\n");
        exit(1);
    }
    Node *tmp = line_no_ptr2[line_no_tmp];
    if(head2 != tmp)
    {
        if(tmp->pre != NULL)
            tmp->pre->next = tmp->next;
        if(tmp->next != NULL)
            tmp->next->pre = tmp->pre;
        if(tail2 == tmp)
            tail2 = tmp->pre;
        tmp->next = head2;
        head2->pre = tmp;
        tmp->pre = NULL;
        head2 = tmp;
    }
    return;
}

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

    Init();

    return;
}

void Finish()
{
    fclose(file1);
    fclose(file2);
    fclose(outfile1);
    fclose(outfile2);
    return;
}

int main(int argc, char *argv[])
{
    printf("please input slice number(0~%d): ", slices-1);
    scanf("%d", &chosen_slice_no);
    printf("please input set number(0~%d): ", SETS-1);
    scanf("%llu", &chosen_set_no);
    printf("please input ratio: ");
    scanf("%d", &ratio);
    printf("please input the step: ");
    scanf("%d", &step);
    printf("please input the allocation(end_way1 and begin_way2): ");
    scanf("%d %d", &end_way1, &begin_way2);
    overlap = (end_way1-begin_way2)<0? 0:(end_way1-begin_way2)+1;
    strcpy(benchname1, argv[1]);
    strcpy(filename1, benchname1);
    strcpy(benchname2, argv[2]);
    strcpy(filename2, benchname2);
    char tmp[100];
    sprintf(tmp, "%d", chosen_slice_no);
    strcat(filename1, "_");
    strcat(filename1, tmp);
    strcat(filename2, "_");
    strcat(filename2, tmp);
    sprintf(tmp, "%llu", chosen_set_no);
    strcat(filename1, "_");
    strcat(filename1, tmp);
    strcat(filename2, "_");
    strcat(filename2, tmp);
    strcat(filename1, ".out");
    strcat(filename2, ".out");

    strcpy(outfilename1, argv[1]);
    strcat(outfilename1, "_");
    strcat(outfilename1, argv[2]);
    strcat(outfilename1, "_");
    sprintf(tmp, "%d", chosen_slice_no);
    strcat(outfilename1, tmp);
    strcat(outfilename1, "_");
    sprintf(tmp, "%llu", chosen_set_no);
    strcat(outfilename1, tmp);
    strcat(outfilename1, "_");
    sprintf(tmp, "%d", overlap);
    strcat(outfilename1, tmp);
    strcpy(outfilename2, outfilename1);
    strcat(outfilename1, "_1");
    strcat(outfilename2, "_2");

    Start();
    
    char tmp_addr1[100], tmp_addr2[100];
    while(fgets(tmp_addr1, 99, file1) != NULL && fgets(tmp_addr2, 99, file2) != NULL)   // end with either file finished
    {
        addr1 = strtoull(tmp_addr1, NULL, 10);
        addr2 = strtoull(tmp_addr2, NULL, 10);
        addr1 = addr1 + ((unsigned long long)1<<53);  // distinguish different benchmark 
        unsigned long long tag1 = addr1 >> (set_bits+block_bits);
        unsigned long long tag2 = addr2 >> (set_bits+block_bits);
        
        if(tag_line_no1.find(tag1) != tag_line_no1.end()) // found
        {
            int line_no = tag_line_no1[tag1];
            Refresh1(line_no);
            if(line_no >= begin_way2)
                Refresh2(line_no);
        }   
        else    // not found
        {
            if(tag_line_no1.size() < ((end_way1-begin_way1)+1))   // not full
            {
                // printf("?\n");
                int allocated_line_no = occupancy1;
                tag_line_no1[tag1] = allocated_line_no;
                Node* tmp = line_no_ptr1[allocated_line_no];
                tmp->tag = tag1;
                Refresh1(allocated_line_no);
                occupancy1++;
                if(allocated_line_no >= begin_way2)
                {
                    tag_line_no2[tag1] = allocated_line_no;
                    tmp = line_no_ptr2[allocated_line_no];
                    tmp->tag = tag1;
                    Refresh2(allocated_line_no);
                }
            }
            else // full
            {
                int line_no = tail1->line_no;
                unsigned long long oldtag = tail1->tag;
                tag_line_no1.erase(oldtag);
                tag_line_no1[tag1] = line_no;
                tail1->tag = tag1;
                Refresh1(line_no);
                if(line_no >= begin_way2)
                {
                    Node* tmp = line_no_ptr2[line_no];
                    tag_line_no2.erase(oldtag);
                    tag_line_no2[tag1] = line_no;
                    tmp->tag = tag1;
                    Refresh2(line_no);
                }
                if(!belong(oldtag))
                {
                    // printf("????%llu\n", oldtag);
                    occupancy1++;
                    occupancy2--;
                }
            }
        }

        int counter = ratio - 1;
        while(counter--)
        {
            if(fgets(tmp_addr1, 99, file1) != NULL)
            {
                addr1 = strtoull(tmp_addr1, NULL, 10);
                addr1 = addr1 + ((unsigned long long)1<<53);  // distinguish different benchmark
                unsigned long long tag1 = addr1 >> (set_bits+block_bits);
                if(tag_line_no1.find(tag1) != tag_line_no1.end()) // found
                {
                    int line_no = tag_line_no1[tag1];
                    Refresh1(line_no);
                    if(line_no >= begin_way2)
                        Refresh2(line_no);
                }   
                else    // not found
                {
                    if(tag_line_no1.size() < ((end_way1-begin_way1)+1))   // not full
                    {
                        // printf("?\n");
                        int allocated_line_no = occupancy1;
                        tag_line_no1[tag1] = allocated_line_no;
                        Node* tmp = line_no_ptr1[allocated_line_no];
                        tmp->tag = tag1;
                        Refresh1(allocated_line_no);
                        occupancy1++;
                        if(allocated_line_no >= begin_way2)
                        {
                            tag_line_no2[tag1] = allocated_line_no;
                            tmp = line_no_ptr2[allocated_line_no];
                            tmp->tag = tag1;
                            Refresh2(allocated_line_no);
                        }
                    }
                    else // full
                    {
                        int line_no = tail1->line_no;
                        unsigned long long oldtag = tail1->tag;
                        tag_line_no1.erase(oldtag);
                        tag_line_no1[tag1] = line_no;
                        tail1->tag = tag1;
                        Refresh1(line_no);
                        if(line_no >= begin_way2)
                        {
                            Node* tmp = line_no_ptr2[line_no];
                            tag_line_no2.erase(oldtag);
                            tag_line_no2[tag1] = line_no;
                            tmp->tag = tag1;
                            Refresh2(line_no);
                        }
                        if(!belong(oldtag))
                        {
                            // printf("????%llu\n", oldtag);
                            occupancy1++;
                            occupancy2--;
                        }
                    }
                }
            }
        }
        
        
        if(tag_line_no2.find(tag2) != tag_line_no2.end()) // found
        {
            int line_no = tag_line_no2[tag2];
            Refresh2(line_no);
            if(line_no <= end_way1)
                Refresh1(line_no);
        }   
        else    // not found
        {
            if(tag_line_no2.size() < ((end_way2-begin_way2)+1))   // not full
            {
                // printf("!\n");
                int allocated_line_no = end_way2-occupancy2;
                tag_line_no2[tag2] = allocated_line_no;
                Node* tmp = line_no_ptr2[allocated_line_no];
                tmp->tag = tag2;
                Refresh2(allocated_line_no);
                occupancy2++;
                if(allocated_line_no <= end_way1)
                {
                    tag_line_no1[tag2] = allocated_line_no;
                    tmp = line_no_ptr1[allocated_line_no];
                    tmp->tag = tag2;
                    Refresh1(allocated_line_no);
                }
            }
            else // full
            {
                int line_no = tail2->line_no;
                unsigned long long oldtag = tail2->tag;
                tag_line_no2.erase(oldtag);
                tag_line_no2[tag2] = line_no;
                tail2->tag = tag2;
                Refresh2(line_no);
                if(line_no <= end_way1)
                {
                    Node* tmp = line_no_ptr1[line_no];
                    tag_line_no1.erase(oldtag);
                    tag_line_no1[tag2] = line_no;
                    tmp->tag = tag2;
                    Refresh1(line_no);
                }
                if(belong(oldtag))
                {
                    // printf("!!!!!!!\n");
                    occupancy1--;
                    occupancy2++;
                }
            }
        }
        
        if(count % step == 0)
        {
            fprintf(outfile1, "%d\n", occupancy1);
            fprintf(outfile2, "%d\n", occupancy2);
        }
        count++;
    }
        

    Finish();
    printf("count: %llu\n", count);
    return 0;
}