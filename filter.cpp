/* 
 * This program filters the traces given slice_no and set_no.
 * Precondition: The .out file including all the traces of the benchmark.
 * Usage: g++ filter.cpp -o filter
 *        ./filter [benchmark]
 * Input: follow the hints
 * Output: the traces of a certain set of the slice, saved as [benchmark]_[slice_no]_[set_no].out
 * Author: Jack Wang
 * Date: 2019.10.21
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
using namespace std;

char benchname[20];
char filename[30], outfilename[100];
FILE *file, *outfile;
int slices = 8, set_bits = 11, block_bits = 6, ways = 11;
unsigned long long addr, chosen_set_no;
int chosen_slice_no;
#define SETS 2048    // 2^set_bits

void Start()
{
    file = fopen(filename, "r");
    outfile = fopen(outfilename, "w");
    if(file == NULL || outfile == NULL)
    {
        printf("cannot open files\n");
        exit(1);
    }

    return;
}

void Finish()
{
    fclose(file);
    fclose(outfile);

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
    printf("please input slice number(0~%d): ", slices-1);
    scanf("%d", &chosen_slice_no);
    printf("please input set number(0~%d): ", SETS-1);
    scanf("%llu", &chosen_set_no);
    strcpy(benchname, argv[1]);
    strcpy(filename, benchname);
    strcat(filename, ".out");
    strcpy(outfilename, benchname);
    strcat(outfilename, "_");
    char tmp[100];
    sprintf(tmp, "%d", chosen_slice_no);
    strcat(outfilename, tmp);
    strcat(outfilename, "_");
    sprintf(tmp, "%llu", chosen_set_no);
    strcat(outfilename, tmp);
    strcat(outfilename, ".out");

    Start();

    char tmp_addr[100];
    while(fgets(tmp_addr, 99, file) != NULL)   // end with the file finished
    {
        addr = strtoull(tmp_addr, NULL, 10);
        // addr1 = addr1 + ((unsigned long long)1<<53);  // distinguish different benchmark
        unsigned long long tag = addr >> (set_bits+block_bits);
        unsigned long long set_no = (addr >> block_bits) & 0B11111111111;    // set_bits
        int slice = Cal_slice(addr);

        if((slice == chosen_slice_no) && (set_no == chosen_set_no))
            fprintf(outfile, "%llu\n", addr);
    }
    
    Finish();
    
    return 0;
}