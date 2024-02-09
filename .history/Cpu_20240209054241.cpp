/*************************************************************************
 * Cpu.cpp
 *
 * This file contains the class implementation for the CPU
 * for the single-cycle machine defined in "Computer Organization and
 * Design, the Hardware Software Interface", Patterson & Hennessy, Fifth Edition.
 *
 * This code was developed as a demonstration for SER450 and is provided
 * as-is as a learning aid for Chapter 4 of the class text.
 *
 * COPYRIGHT (C) 2019, Arizona State University
 * ALL RIGHTS RESERVED
 *
 * Author: Doug Sandy
 *
 **************************************************************************/
#include <stdio.h>
#include "Cpu.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include "Cpu.h"

/*************************************************************************
 * Student modifications should go here to give the proper values for these
 * macro definitions
 **************************************************************************/
// BITS(x, start, end) a macro function that takes three integer arguments
//    the output of the function will be bits start-end of of value x but
//    the bits in the output will be shifted so that bit start is at bit
//    position 0.  Example: the input values 0x12345678, 8,12 will yield
//    the result 0x00000016
#define BITS(x, start, end)

// SIGN_EXT(x) a macro to sign extend a 16 bit signed integer to 32 bits
#define SIGN_EXT(x)

// each of the following are the opcodes for teh specified instruction from
// the MIPS instruction set.
#define OP_LW
#define OP_SW
#define OP_RTYPE
#define OP_BEQ
#define OP_JMP

//********************************************
// Constructor
// initialize the cpu object
Cpu::Cpu()
{
    // initialize the program counter to 0
    pc = 0;
}

//********************************************
// setImem
// set the value of a 32-bit word at a specified
// address in the CPU's instruction memory
void Cpu::setImem(unsigned int address, unsigned int value)
{
    imem.setAt(address, value);
}

//********************************************
// setImem
// set the value of a 32-bit word at a specified
// address in the CPU's data memory
void Cpu::setDmem(unsigned int address, unsigned int value)
{
    dmem.update(address, value, true);
}

//*************************************************
// update()
// This function simulates a single clock cycle of the
// CPU.  This was covered in the class lecture notes
void Cpu::update()
{
    // Increment instruction count
    totalInstructions++;

    // Fetch the current instruction
    unsigned int instruction = imem.value(pc);

    // Extract the fields from the instruction
    unsigned int opcode = BITS(instruction, 26, 31);
    unsigned int funct = BITS(instruction, 0, 5);
    unsigned int rdidx = BITS(instruction, 11, 15);
    unsigned int rtidx = BITS(instruction, 16, 20);
    unsigned int rsidx = BITS(instruction, 21, 25);
    signed int immed = SIGN_EXT(BITS(instruction, 0, 15));
    unsigned int jmpaddr = BITS(instruction, 0, 25);

    // ALU operation combinational logic
    int ALUOp = 0x0;
    if ((opcode == 0x23) || (opcode == 0x2b))
        ALUOp = 0x0; // Load or store instructions
    if (opcode == 0x04)
        ALUOp = 0x1; // Branch on equal instruction
    if (opcode == 0x00)
        ALUOp = 0x2; // R-type instructions

    // ALU control combinational logic
    int ALUControl = 0xF;
    if (ALUOp == 0x0)
        ALUControl = 0x2; // lw/sw -> add
    if (ALUOp == 0x01)
        ALUControl = 0x6; // lw/sw -> subtract
    if (ALUOp == 0x02)
    {
        if (funct == 0x20)
            ALUControl = 0x2; // add
        if (funct == 0x22)
            ALUControl = 0x6; // subtract
        if (funct == 0x24)
            ALUControl = 0x0; // AND
        if (funct == 0x25)
            ALUControl = 0x1; // OR
        if (funct == 0x2a)
            ALUControl = 0x7; // set-on-less-than
    }

    // Additional control signals based on opcode
    unsigned int aluSrc = (opcode == OP_LW) || (opcode == OP_SW) ? 1 : 0;
    unsigned int regDest = (opcode == OP_RTYPE) ? 1 : 0;
    unsigned int branch = (opcode == OP_BEQ) ? 1 : 0;
    unsigned int memRead = (opcode == OP_LW) ? 1 : 0;
    unsigned int memToReg = (opcode == OP_LW) ? 1 : 0;
    unsigned int memWrite = (opcode == OP_SW) ? 1 : 0;
    unsigned int regWrite = (opcode == OP_LW) || (opcode == OP_RTYPE) ? 1 : 0;
    unsigned int jump = (opcode == OP_JMP) ? 1 : 0;

    // Register file read operation based on rt and rd indices
    unsigned int regRs = regs.readData1(rsidx);
    unsigned int regRt = regs.readData2(rtidx);

    // ALU source selection multiplexor
    unsigned int operand1 = regRs;
    unsigned int operand2 = aluSrc ? immed : regRt;

    // ALU combinational logic
    int ALUResult = 0;
    if (ALUControl == 0x0)
        ALUResult = operand1 & operand2; // AND
    if (ALUControl == 0x1)
        ALUResult = operand1 | operand2; // OR
    if (ALUControl == 0x2)
        ALUResult = operand1 + operand2; // ADD
    if (ALUControl == 0x6)
        ALUResult = operand1 - operand2; // SUBTRACT
    if (ALUControl == 0x7)
        ALUResult = (operand1 < operand2) ? 1 : 0; // SET LESS THAN
    if (ALUControl == 0xc)
        ALUResult = ~(operand1 | operand2); // NOR
    int zero = (ALUResult == 0x00000000) ? 1 : 0;

    // Read the data memory if required
    unsigned int memData = dmem.read(ALUResult, memRead);

    // Register write data multiplexor
    unsigned int regWrData = memToReg ? memData : ALUResult;

    // Register write destination multiplexor
    unsigned int regWrAddr = regDest ? rdidx : rtidx;

    // Update the CPU state based on the rising edge of the system clock
    // In real hardware, these updates would all happen simultaneously
    regs.update(regWrAddr, regWrData, regWrite);
    dmem.update(ALUResult, regRt, memWrite);
    pc = next_pc;

    // Increment register write instructions count
    if (regWrite)
        totalRegWriteInstructions++;

    // Increment memory instructions count
    if (memRead || memWrite)
        totalMemoryInstructions++;

    // Multiplexors to select the next program counter value
    next_pc = (branch && zero) ? branchAddr : pc + 4;
    next_pc = (jump) ? fullJumpAddr : next_pc;

    // Update the program counter
    pc = next_pc;
}

//**********************************************************************
// dump()
// dump the state of the CPU object to the standard output device
void Cpu::dump()
{
    printf("PROGRAM COUNTER = %08x   INSTRUCTION = %08x\n", pc, imem.value(pc));
    printf("REGISTER FILE\n");
    regs.dump();
    printf("\n");

    /*****************************************************************
     * STUDENT CODE
     * This would be a great place to output your instrumention results
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