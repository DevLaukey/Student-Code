#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include "Cpu.h"

// Definitions of pipeline registers
typedef struct
{
    unsigned int instruction;
} ifid_reg;

typedef struct
{
    unsigned char regWrite;
    unsigned char memToReg;
    unsigned char memRead;
    unsigned char memWrite;
    unsigned char regDst;
    unsigned char aluOperation;
    unsigned char aluSrc;
    unsigned int regRsDat;
    unsigned int regRtDat;
    unsigned int rs;
    unsigned int rt;
    unsigned int rd;
    unsigned int immed_se;
    unsigned int next_pc;
    unsigned int instruction;
} idex_reg;

typedef struct
{
    unsigned char regWrite;
    unsigned char memToReg;
    unsigned char memRead;
    unsigned char memWrite;
    unsigned int aluResult;
    unsigned int dat2;
    unsigned char registerNum;
    unsigned int instruction;
} exmem_reg;

typedef struct
{
    unsigned char regWrite;
    unsigned int regWrData;
    unsigned char registerNum;
    unsigned int instruction;
} memwb_reg;

Cpu::Cpu() : pc(0) {}

void Cpu::setImem(unsigned int address, unsigned int value)
{
    imem.setAt(address, value);
}

void Cpu::setDmem(unsigned int address, unsigned int value)
{
    dmem.update(address, value, true);
}

void Cpu::update()
{
    // Increment instruction count
    totalInstructions++;

    // Execute the pipeline stages
    thread_wb_start();
    thread_mem_start();
    thread_ex_start();
    thread_id_start();
    thread_if_start();

    // Increment register write instructions count
    if (regMEMWB_WBside.regWrite)
        totalRegWriteInstructions++;

    // Increment memory instructions count
    if (regEXMEM_MEMside.memRead || regEXMEM_MEMside.memWrite)
        totalMemoryInstructions++;

    // Update program counter
    pc = regIFID_IDside.next_pc;
}

void Cpu::thread_if_start()
{
    regIFID_IFside.instruction = imem.value(pc);
    // Increment the program counter for the next cycle
    regIFID_IFside.next_pc = pc + 4;
}

void Cpu::thread_id_start()
{
    // Extract fields from the instruction in the IF/ID pipeline register
    unsigned int instruction = regIFID_IFside.instruction;

    // Populate ID/EX pipeline register
    regIDEX_IDside.regWrite = (opcode == OP_LW) || (opcode == OP_RTYPE) ? 1 : 0;
    regIDEX_IDside.memToReg = (opcode == OP_LW) ? 1 : 0;
    // ... (Set other fields in regIDEX_IDside)
}

void Cpu::thread_ex_start()
{
    // Extract fields from the instruction in the ID/EX pipeline register
    unsigned int instruction = regIDEX_IDside.instruction;

    // Perform ALU operation and set EX/MEM pipeline register
    // ... (ALU operation logic)
    regEXMEM_EXside.regWrite = (opcode == OP_LW) || (opcode == OP_RTYPE) ? 1 : 0;
    regEXMEM_EXside.memToReg = (opcode == OP_LW) ? 1 : 0;
    // ... (Set other fields in regEXMEM_EXside)
}

void Cpu::thread_mem_start()
{
    // Extract fields from the instruction in the EX/MEM pipeline register
    unsigned int instruction = regEXMEM_EXside.instruction;

    // Access data memory and set MEM/WB pipeline register
    // ... (Data memory access logic)
    regMEMWB_MEMside.regWrite = (opcode == OP_LW) || (opcode == OP_RTYPE) ? 1 : 0;
    regMEMWB_MEMside.memToReg = (opcode == OP_LW) ? 1 : 0;
    // ... (Set other fields in regMEMWB_MEMside)
}

void Cpu::thread_wb_start()
{
    // Extract fields from the instruction in the MEM/WB pipeline register
    unsigned int instruction = regMEMWB_MEMside.instruction;

    // Write back to the register file
    // ... (Write back logic)
}

void Cpu::dump()
{
    printf("PROGRAM COUNTER = %08x   INSTRUCTION = %08x\n", pc, regIFID_IFside.instruction);
    printf("REGISTER FILE\n");
    regs.dump();
    printf("\n");

    /*****************************************************************
     * STUDENT CODE
     * This would be a great place to output your instrumentation results
     ******************************************************************/
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file> <num_cycles>" << std::endl;
        return EXIT_FAILURE;
    }

    // Display startup banner
    std::cout << "Simulation by [Your Name] - SER450" << std::endl;

    // Extract command line arguments
    const char *inputFileName = argv[1];
    int numCycles = std::atoi(argv[2]);

    // Create CPU object
    Cpu cpu;

    // Read the input file and initialize memory
    std::ifstream inputFile(inputFileName, std::ios::binary);
    if (!inputFile.is_open())
    {
        std::cerr << "Error: Unable to open input file." << std::endl;
        return EXIT_FAILURE;
    }

    unsigned int address, value;
    while (inputFile >> std::hex >> address >> value)
    {
        cpu.setImem(address, value);
    }

    // Close the input file
    inputFile.close();

    // Run the simulation for the specified number of cycles
    for (int cycle = 0; cycle < numCycles; ++cycle)
    {
        std::cout << "Cycle " << cycle << ":" << std::endl;
        cpu.update();
    }

    // Display final results
    std::cout << "Final State:" << std::endl;
    cpu.dump();

    return EXIT_SUCCESS;
}
