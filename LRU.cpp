/*
 * This program simulates a LRU-based cache, which focuses on only one set.
 * The target is to observe the occupancies of two co-run benchmarks.
 * Usage: g++ -std=c++11 LRU.cpp -o LRU
 *        ./LRU [benmark1] [benchmark2]
 * Input: follow the hints
 * Output: the occupancy of benmark1, saved as [benchamar1]_[benchmark2]_[set_no]
 * Author: Jack Wang
 * Date: 2019.10.9
 */

#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<unordered_map>
using namespace std;
char benchname1[20], benchname2[20];
char filename1[30], filename2[30], outfilename[100];
FILE *file1, *file2, *outfile;
int set_bits = 11, block_bits = 6, ways = 11;
int step;    // the step of printing
unsigned long long count = 0, set_no, addr1, addr2;
int occupancy1 = 0;  // the occupancy of benchmark1
int ratio;    // benchmark1:benchmark2

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
Node *head, *tail;
unordered_map<unsigned long long, int> tag_line_no;
unordered_map<int, Node*> line_no_ptr;

bool belong(unsigned long long tag)
{
    if((tag>>(53-set_bits-block_bits)) == 1)
        return true;
    else
        return false;
}

void Init()
{
    Node *tmp_pre = NULL, *tmp_next = NULL;
    head = new Node(0);
    tmp_pre = head;
    for(int i = 1; i < ways; i++)
    {
        tmp_next = new Node(i);
        tmp_pre->next = tmp_next;
        tmp_next->pre = tmp_pre;
        tmp_pre = tmp_next;
    }
    tail = tmp_pre;

    Node *tmp = head;
    for(int i = 0; i < ways; i++)
    {
        line_no_ptr.insert(make_pair(i, tmp));
        tmp = tmp->next;
    } 
}

void Refresh(unsigned long long tag)
{
    int line_no_tmp = tag_line_no[tag];
    if(line_no_tmp >= ways || line_no_tmp < 0)
    {
        printf("line_no_tmp >= ways || line_no_tmp < 0\n");
        exit(1);
    }
    Node *tmp = line_no_ptr[line_no_tmp];
    if(head != tmp)
    {
        if(tmp->pre != NULL)
            tmp->pre->next = tmp->next;
        if(tmp->next != NULL)
            tmp->next->pre = tmp->pre;
        if(tail == tmp)
            tail = tmp->pre;
        tmp->next = head;
        head->pre = tmp;
        tmp->pre = NULL;
        head = tmp;
    }
    return;
}

void Replace(unsigned long long newtag)
{
    int line_no_tmp = tail->line_no;
    unsigned long long oldtag = tail->tag;
    tag_line_no.erase(oldtag);
    tag_line_no[newtag] = line_no_tmp;
    tail->tag = newtag;
    Refresh(newtag);
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
    printf("please input set number: ");
    scanf("%llu", &set_no);
    printf("please input step: ");
    scanf("%d", &step);
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
    strcat(outfilename, "_");
    char tmp[100];
    sprintf(tmp, "%llu", set_no);
    strcat(outfilename, tmp);

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
        if(set_no1 == set_no)
        {
            if(tag_line_no.find(tag1) != tag_line_no.end()) // found
                Refresh(tag1);
            else    // not found
            {
                if(tag_line_no.size() < ways)   // not full
                {
                    int allocated_line_no = tag_line_no.size();
                    tag_line_no[tag1] = allocated_line_no;
                    Node* tmp = line_no_ptr[allocated_line_no];
                    tmp->tag = tag1;
                    Refresh(tag1);
                    occupancy1++;
                }
                else // full
                {
                    if(!belong(tail->tag))
                        occupancy1++;
                    Replace(tag1);
                }
            }
            
            if(count % step == 0)
                fprintf(outfile, "%d\n", occupancy1);
            count++;
        }

        int counter = ratio - 1;
        while(counter--)
        {
            if(fgets(tmp1, 99, file1) != NULL)
            {
                addr1 = strtoull(tmp1, NULL, 10);
                addr1 = addr1 + ((unsigned long long)1<<53);  // distinguish different benchmark
                unsigned long long tag1 = addr1 >> (set_bits+block_bits);
                unsigned long long set_no1 = (addr1 >> block_bits) & 0B011111111111;    // set_bits
                if(set_no1 == set_no)
                {
                    if(tag_line_no.find(tag1) != tag_line_no.end()) // found
                        Refresh(tag1);
                    else    // not found
                    {
                        if(tag_line_no.size() < ways)   // not full
                        {
                            int allocated_line_no = tag_line_no.size();
                            tag_line_no[tag1] = allocated_line_no;
                            Node* tmp = line_no_ptr[allocated_line_no];
                            tmp->tag = tag1;
                            Refresh(tag1);
                            occupancy1++;
                        }
                        else // full
                        {
                            if(!belong(tail->tag))
                                occupancy1++;
                            Replace(tag1);
                        }
                    }
                    
                    if(count % step == 0)
                        fprintf(outfile, "%d\n", occupancy1);
                    count++;
                }
            }
            else
                break;
        }

        if(set_no2 == set_no)
        {
            if(tag_line_no.find(tag2) != tag_line_no.end()) // found
                Refresh(tag2);
            else    // not found
            {
                if(tag_line_no.size() < ways)   // not full
                {
                    int allocated_line_no = tag_line_no.size();
                    tag_line_no[tag2] = allocated_line_no;
                    Node* tmp = line_no_ptr[allocated_line_no];
                    tmp->tag = tag2;
                    Refresh(tag2);
                }
                else // full
                {
                    if(belong(tail->tag))
                        occupancy1--;
                    Replace(tag2);
                }
            }

            if(count % step == 0)
                fprintf(outfile, "%d\n", occupancy1);
            count++;
        }
    }

    Finish();
    printf("count: %llu\n", count);
    return 0;
}