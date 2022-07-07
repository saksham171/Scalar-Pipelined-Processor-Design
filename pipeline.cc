#include "processor.h"
using namespace std;


int main() {
    //open instruction file
    Processor p("input/ICache.txt", "input/DCache.txt", "input/RF.txt", "output/ODCache.txt");

    clk = 0;
    while(p.nextStage()){
        clk++;//number of clock cycles
    }
    p.printStats("output/Output.txt");//write stats in output file
    return 0;
}