#include <cstdio>
#include <cstring>
#include <cstdlib>
using namespace std;
int main()
{
    FILE* outfile = fopen("0_0.out", "w");
    int cycle = 1000;
    unsigned long long addr1=0;
    unsigned long long addr2=0;
    while(cycle--)
    {
        for(int i = 0;i<5;i++)
        {
            unsigned long long addr = (addr1<<17);
            addr = addr + ((unsigned long long)1<<53);
            fprintf(outfile, "%llu\n", addr);
            addr1 = (addr1+1)%11;
        }
        unsigned long long addr = (addr2<<17);
        fprintf(outfile, "%llu\n", addr);
    }
    
    return 0;
}