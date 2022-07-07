#include "processor.h"
using namespace std;

int clk = 0;

string Processor::readNthLine(string filename, short int line_number){//read from input files
    ifstream File;
    File.open(filename);
    string tmp;
    for(int i=0; i<=line_number; i++) getline(File, tmp);
    File.close();
    return tmp; 
}

bool Processor::nextStage()//pipeline flow
{
    bool isHLT = WriteBack();
    if (isHLT)
        return false;//end program
    Memory();
    ExecuteInstruction();
    InstructionDecode();
    InstructionFetch();

    if (CHstall > 0)//control stall
        CHstall--;
    return true;
}

short int Processor::HexToDec(string h)//converting a hexadecimal number to a decimal number 
{
    short int ret = 0;

    short int n1 = h[0] - '0';
    if (n1 < 10)
        ret += 16 * n1;
    else
    {
        n1 = n1 + '0' - 'a';
        ret += 16 * (n1 + 10);
    }

    short int n2 = h[1] - '0';
    if (n2 < 10)
        ret += n2;
    else
    {
        n2 = n2 + '0' - 'a';
        ret += (n2 + 10);
    }
    if (ret >= 128)
        ret -= 256;
    return ret;
}

string Processor::DecToHex(short int d)//converting a decimal number to a hexadecimal number
{
    if (d < 0)
        d += 256;
    int firstByte = d >> 4;
    int secondByte = d % 16;

    string s;

    if (firstByte < 10)
    {
        char c = '0' + firstByte;
        s.push_back(c);
    }
    else
    {
        char c = 'a' + firstByte - 10;
        s.push_back(c);
    }

    if (secondByte < 10)
    {
        char c = '0' + secondByte;
        s.push_back(c);
    }
    else
    {
        char c = 'a' + secondByte - 10;
        s.push_back(c);
    }

    return s;
}
void Processor::InstructionFetch()
{
    if (RAWstall > 0)//stall
        return;
    if (CHstall > 0)//stall
        return;
    // go to line programcounter/2 + 1
    IFInst.buffer11 = readNthLine(ICache, programCounter);
    IFInst.buffer12 = readNthLine(ICache, programCounter+1);
    programCounter += 2;//each instruction is 2 bytes
    op[1] = SOP;

    IDInst = IFInst;
}

void Processor::InstructionDecode()
{
    if (op[1] == NOP)
    {
        op[2] = op[1];//no operation to decode
        return;
    }
    assert(IDInst.buffer11.size() == 2);
    assert(IDInst.buffer12.size() == 2);
    short int firstbyte = HexToDec(IDInst.buffer11), secondbyte = HexToDec(IDInst.buffer12);
    if (firstbyte < 0)
        firstbyte += 256;
    if (secondbyte < 0)
        secondbyte += 256;
    switch (firstbyte / 16)
    {
    case 0:
        op[2] = ADD;
        break;
    case 1:
        op[2] = SUB;
        break;
    case 2:
        op[2] = MUL;
        break;
    case 3:
        op[2] = INC;
        break;
    case 4:
        op[2] = AND;
        break;
    case 5:
        op[2] = OR;
        break;
    case 6:
        op[2] = NOT;
        break;
    case 7:
        op[2] = XOR;
        break;
    case 8:
        op[2] = LD;
        break;
    case 9:
        op[2] = SD;
        break;
    case 10:
        op[2] = JMP;
        break;
    case 11:
        op[2] = BEQZ;
        break;
    case 15:
        op[2] = HLT;
        break;
    default:
        ;
    }
    IDInst.destinationRegister = firstbyte % 16;
    IDInst.A = secondbyte / 16;
    IDInst.B = secondbyte % 16;

    //RAW vector stores all destination register for the succeeding instructions
    //the find function searches RAW if the source register of the current instructions
    //is one of the destination registers of the succeeding instructions in the pipeline.
    //if the iterator returned is RAW.end(), the source register is not present in the RAW vector.
    if (op[2] == SD && find(RAW.begin(), RAW.end(), IDInst.destinationRegister) != RAW.end())//RAW hazard due to store as source register appears first in instruction
    {
        RAWstall = 1;
        totalRAW ++;
        op[2] = NOP;
        return;
    } else {
        RAWstall = 0;
    }
    if (op[2] != SD && op[2]!=JMP && op[2]!=BEQZ && op[2]!=HLT  && (find(RAW.begin(), RAW.end(), IDInst.A) != RAW.end() || find(RAW.begin(), RAW.end(), IDInst.B) != RAW.end()))
    {//RAW hazard
        RAWstall = 1;
        totalRAW ++;
        op[2] = NOP;
        return;
    }
    else {
        RAWstall = 0;
    }
    if (op[2] != SD && op[2] != JMP && op[2] != BEQZ && op[2] != HLT)
    {
        if(IDInst.destinationRegister != 0) RAW.push_back(IDInst.destinationRegister);
    }
    string reg = readNthLine(RegFile, IDInst.destinationRegister);
    IDInst.destinationRegisterValue = HexToDec(reg);//reading register value from the RF file
    string regA = readNthLine(RegFile, IDInst.A);
    IDInst.registerA = HexToDec(regA);
    string regB = readNthLine(RegFile, IDInst.B);
    IDInst.registerB = HexToDec(regB);
    op[1] = NOP;
    if (op[2] == JMP || op[2] == BEQZ)
    {
        CHstall = 2;//control stall
        totalCH += 2;
    }
    EXInst = IDInst;
}

void Processor::ExecuteInstruction()
{
    switch (op[2])//executing each instruction opcode
    {
    case LD:
    case SD:
        EXInst.ALUOutput = EXInst.registerA + EXInst.B;
        break;
    case ADD:
        EXInst.ALUOutput = EXInst.registerA + EXInst.registerB;
        break;
    case SUB:
        EXInst.ALUOutput = EXInst.registerA - EXInst.registerB;
        break;
    case MUL:
        EXInst.ALUOutput = EXInst.registerA * EXInst.registerB;
        break;
    case INC:
        EXInst.ALUOutput = EXInst.destinationRegisterValue + 1;
        break;
    case AND:
        EXInst.ALUOutput = (EXInst.registerA & EXInst.registerB);
        break;
    case OR:
        EXInst.ALUOutput = (EXInst.registerA | EXInst.registerB);
        break;
    case NOT:
        EXInst.ALUOutput = (~EXInst.registerA);
        break;
    case XOR:
        EXInst.ALUOutput = (EXInst.registerA ^ EXInst.registerB);
        break;
    case JMP:
        EXInst.ALUOutput = programCounter + ((EXInst.destinationRegister * 16 + EXInst.A) << 1);
        programCounter = EXInst.ALUOutput;
        break;
    case BEQZ:
        EXInst.ALUOutput = programCounter + ((EXInst.A * 16 + EXInst.B) << 1);
        if (EXInst.destinationRegisterValue == 0)
            programCounter = EXInst.ALUOutput;
        break;
    case HLT:
        break;
    default:;
    }
    op[3] = op[2];
    MEInst = EXInst;
}

void Processor::Memory()
{
    if (op[3] == LD)
    {
        string LMD = readNthLine(DCache, MEInst.ALUOutput);//read from data cache for load instruction
        LoadMemoryRegister = HexToDec(LMD);
    }
    else if (op[3] == SD)
    {
        string replace = DecToHex(MEInst.destinationRegisterValue);
        OutputCache.open(ODCache);
        string tmp;
        for (int i = 0; i < MEInst.ALUOutput; i++)
            getline(OutputCache, tmp);//write in output file for store instruction
        OutputCache << replace;
        OutputCache.close();
    }
    op[4] = op[3];
    WBInst = MEInst;
}

bool Processor::WriteBack()
{
    try{
        if (op[4] == LD)
        {
            RegisterFile.open(RegFile);
            string tmp;
            string replace = DecToHex(LoadMemoryRegister);
            for (int i = 0; i < WBInst.destinationRegister; i++)
                getline(RegisterFile, tmp);
            RegisterFile << replace;//change register value in register file
            RegisterFile.close();
            if (find(RAW.begin(), RAW.end(), WBInst.destinationRegister) != RAW.end())
                RAW.erase(find(RAW.begin(), RAW.end(), WBInst.destinationRegister));
            DataInst++;
        }
        else if (op[4] != SD && op[4] != HLT && op[4] != BEQZ && op[4] != JMP && op[4] != NOP)
        {
            RegisterFile.open(RegFile);
            string tmp;
            string replace = DecToHex(WBInst.ALUOutput);
            for (int i = 0; i < WBInst.destinationRegister; i++)
                getline(RegisterFile, tmp);
            RegisterFile << replace;
            RegisterFile.close();
            if (find(RAW.begin(), RAW.end(), WBInst.destinationRegister) != RAW.end())
                RAW.erase(find(RAW.begin(), RAW.end(), WBInst.destinationRegister));
            if (op[4] == ADD || op[4] == SUB || op[4] == MUL || op[4] == INC)//arithmetic instruction
                arithmetricInst++;
            else
                LogicalInst++;
        }
        else if (op[4] == HLT)
        {
            throw Halt();//exception class for halt
        }
        if (op[4] != NOP)//stall i.e. no instruction
            instructions++;
        if (op[4] == SD)
            DataInst++;
        if (op[4] == JMP || op[4] == BEQZ)
            ControlInst++;
    }
    catch(Halt &e){
        HaltInst++;
        instructions++;
        return true;//end program
    }
    return false;
}

void Processor::printStats(string outputFile)
{

    ofstream Statistics;
    Statistics.open(outputFile);
    Statistics << "Total number of instructions executed:   " << instructions << "\n";
    Statistics << "Number of instructions in each class:    \n";
    Statistics << "Arithmetric instructions:                " << arithmetricInst << "\n";
    Statistics << "Logical instructions:                    " << LogicalInst << "\n";
    Statistics << "Data instructions:                       " << DataInst << "\n";
    Statistics << "Control instructions:                    " << ControlInst << "\n";
    Statistics << "Halt instructions:                       " << HaltInst << "\n";
    Statistics << "Cycles per instructions:                 " << (clk + 1) * 1.0 / instructions << "\n";
    Statistics << "Total number of stalls:                  " << totalRAW + totalCH << "\n";
    Statistics << "Data stalls (RAW):                       " << totalRAW << "\n";
    Statistics << "Control stalls:                          " << totalCH << "\n";
    Statistics.close();
}