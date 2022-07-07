#include <iostream>
#include <cassert>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
using namespace std;
extern int clk;             //global clock variable represents the clock cycles executed so far
struct BufferValues
{
    short int destinationRegister;
    short int destinationRegisterValue;
    short int A, B;
    short int registerA, registerB;
    short int ALUOutput;
    string buffer11, buffer12;
    BufferValues(string b1, string b2)
    {
        buffer11 = b1;
        buffer12 = b2;
    }
    BufferValues() {}
};

class Processor
{
private:
    ifstream InstructionCache;
    fstream DataCache;
    fstream RegisterFile;
    fstream OutputCache;
    string ICache, DCache, RegFile, ODCache;
    short int programCounter;
    short int LoadMemoryRegister;
    enum
    {
        LD,SD,ADD,SUB,MUL,INC,AND,OR,XOR,NOT,JMP,BEQZ,HLT,NOP,SOP
    } op[5];
    BufferValues IFInst, IDInst, EXInst, MEInst, WBInst;
    int instructions;
    int arithmetricInst;
    int LogicalInst;
    int ControlInst;
    int HaltInst;
    int DataInst;
    vector<short int> RAW;//stores all destination register for the succeeding instructions
    int RAWstall;       //is 1 when there is a RAW stall and 0 otherwise
    int CHstall;        //represents the numbers of stalls remaining - is initialised at 2 when a control hazard is detected
    int totalRAW;
    int totalCH;

    class Halt{
    public:
        Halt(){
            cout<<(clk + 1)<<endl;
        };
        
    };

public:
    Processor(string I, string D, string RF, string OD)
    {
        ICache = I;
        DCache = D;
        RegFile = RF;
        ODCache = OD;
        instructions = 0;
        arithmetricInst = 0;
        ControlInst = 0;
        DataInst = 0;
        LogicalInst = 0;
        HaltInst = 0;
        totalRAW = 0;
        totalCH = 0;
        programCounter = 0;
        RAW = {};
        RAWstall = 0;
        CHstall = 0;
        op[0] = op[1] = op[2] = op[3] = op[4] = NOP;
    }
    
    bool nextStage();
    short int HexToDec(string);
    string DecToHex(short int);
    string readNthLine(string filename, short int line_number);
    void InstructionFetch();
    void InstructionDecode();
    void ExecuteInstruction();
    void Memory();
    bool WriteBack();
    void printStats(string);
};
