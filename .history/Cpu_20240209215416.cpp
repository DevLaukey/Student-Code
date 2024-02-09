/*************************************************************************
 * Cpu.cpp
 *
 * This file contains the class implementation for the CPU
 * for the pipelined machine defined in "Computer Organization and
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

// BITS(x, start, end) a macro function that takes three integer arguments
//    the output of the function will be bits start-end of of value x but
//    the bits in the output will be shifted so that bit start is at bit
//    position 0.  Example: the input values 0x12345678, 8,12 will yield
//    the result 0x00000006
#define BITS(x, start, end) ((x >> start) & ((1 << (end - start + 1)) - 1))

// SIGN_EXT(x) a macro to sign extend a 16 bit signed integer to 32 bits
#define SIGN_EXT(x) ((x & 0x8000) ? (0xffff0000 | x) : x)

// each of the following are the opcodes for teh specified instruction from
// the MIPS instruction set.
#define OP_LW 0x23
#define OP_SW 0x2B
#define OP_RTYPE 0x00
#define OP_BEQ 0x04
#define OP_JMP 0x02

// Forwarding messages for debugging
#define FORWARD_RS_MEM "Forwarding RS from MEM to Equality Unit"
#define FORWARD_RT_MEM "Forwarding RT from MEM to Equality Unit"
#define FORWARD_RS_WB "Forwarding RS from WB to ALU input"
#define FORWARD_RT_WB "Forwarding RT from WB to ALU input"

//********************************************
// Constructor
Cpu::Cpu(DataMemory &dmem, RegisterFile &regs)
    : dmem(dmem), regs(regs)
{
    // initialize the register pipeline register outputs to 0
    regIFID_IDside = {
        0,
    };
    regIDEX_EXside = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    regEXMEM_MEMside = {0, 0, 0, 0, 0, 0, 0, 0};
    regMEMWB_WBside = {0, 0, 0, 0};
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
// setDmem
// set the value of a 32-bit word at a specified
// address in the CPU's data memory
void Cpu::setDmem(unsigned int address, unsigned int value)
{
    dmem.update(address, value, true);
}

//*******************************************
// thread_if_start
// This function simulates a thread that
// runs in parallel with the other pipeline
// stage combinational logic.
void Cpu::thread_if_start()
{
    // set the signal values from the outputs of the
    // appropriate pipeline registers
    unsigned int pc = regIDEX_EXside.next_pc;

    // fetch the current instruction
    unsigned int instruction = imem.value(pc);

    // assign our outputs to the inputs of the associated
    // next-stage pipeline registers.
    regIFID_IFside.instruction = instruction;
}

//*******************************************
// thread_id_start
// This function simulates a thread that
// runs in parallel with the other pipeline
// stage combinational logic.
void Cpu::thread_id_start()
{
    // set the signal values from the outputs of the
    // appropriate pipeline registers
    unsigned int instruction = regIFID_IDside.instruction;
    unsigned int pc = regIDEX_EXside.next_pc;
    unsigned int mem_regWrite = regEXMEM_MEMside.regWrite;       // for forwarding
    unsigned int mem_registerNum = regEXMEM_MEMside.registerNum; // for forwarding
    unsigned int mem_aluResult = regEXMEM_MEMside.aluResult;     // for forwarding

    // extract the fields from the instruction
    unsigned int opcode = BITS(instruction, 26, 31);
    unsigned int funct = BITS(instruction, 0, 5);
    unsigned int rdidx = BITS(instruction, 11, 15);
    unsigned int rtidx = BITS(instruction, 16, 20);
    unsigned int rsidx = BITS(instruction, 21, 25);
    signed int immed = SIGN_EXT(BITS(instruction, 0, 15));
    unsigned int jmpaddr = BITS(instruction, 0, 25);

    // alu operation combinational logic
    int ALUOp = 0x0;
    if ((opcode == 0x23) || (opcode == 0x2b))
        ALUOp = 0x0; // load or store instructions
    if (opcode == 0x04)
        ALUOp = 0x1; // branch on equal instruction
    if (opcode == 0x00)
        ALUOp = 0x2; // r-type instructions

    // alu control combinational logic
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

    // additional control signals based on opcode
    unsigned int aluSrc = (opcode == OP_LW) || (opcode == OP_SW) ? 1 : 0;
    unsigned int regDest = (opcode == OP_RTYPE) ? 1 : 0;
    unsigned int branch = (opcode == OP_BEQ) ? 1 : 0;
    unsigned int memRead = (opcode == OP_LW) ? 1 : 0;
    unsigned int memToReg = (opcode == OP_LW) ? 1 : 0;
    unsigned int memWrite = (opcode == OP_SW) ? 1 : 0;
    unsigned int regWrite = (opcode == OP_LW) || (opcode == OP_RTYPE) ? 1 : 0;
    unsigned int jump = (opcode == OP_JMP) ? 1 : 0;

    // register file read operation based on rt and rd indicies
    unsigned int regRs = regs.readData1(rsidx);
    unsigned int regRt = regs.readData2(rtidx);

    // calculate the branch or jump address
    unsigned int branchAddr = (immed << 2) + pc;
    unsigned int fullJumpAddr = ((pc) & 0xF0000000) | (jmpaddr << 2);
    unsigned int next_pc;

    //****************************************************
    // The following code implements an "equality unit".
    // The definition of this unit is based on the course
    // text, pages 318 and 319.
    // - Values for rs and rt are forwarded from the mem if
    //   necessary
    // - Forwarding from the EX phase is not possible because
    //   the results of the ALU computations are not computed
    //   until the end of the clock cycle. This needs to be
    //   checked for by the compiler.
    unsigned equalityRs = regRs;
    if ((mem_regWrite) && (mem_registerNum != 0) && (mem_registerNum == rsidx))
    {
        printf("Forwarding RS from MEM to Equality Unit\n");
        equalityRs = mem_aluResult;
    }
    unsigned equalityRt = regRt;
    if ((mem_regWrite) && (mem_registerNum != 0) && (mem_registerNum == rtidx))
    {
        printf("Forwarding RT from MEM to Equality Unit\n");
        equalityRt = mem_aluResult;
    }
    unsigned int equal = (equalityRs == equalityRt) ? 1 : 0;
    //**** END EQUALITY UNIT ********

    // multiplexors to select the next program counter value
    next_pc = (branch && equal) ? branchAddr : pc + 4;
    next_pc = (jump) ? fullJumpAddr : next_pc;

    // assign our outputs to the inputs of the associated
    // next-stage pipeline registers.
    regIDEX_IDside.aluOperation = ALUControl;
    regIDEX_IDside.aluSrc = aluSrc;
    regIDEX_IDside.immed_se = immed;
    regIDEX_IDside.memRead = memRead;
    regIDEX_IDside.memWrite = memWrite;
    regIDEX_IDside.memToReg = memToReg;
    regIDEX_IDside.next_pc = next_pc;
    regIDEX_IDside.rd = rdidx;
    regIDEX_IDside.rs = rsidx;
    regIDEX_IDside.rt = rtidx;
    regIDEX_IDside.regDst = regDest;
    regIDEX_IDside.regRsDat = regRs;
    regIDEX_IDside.regRtDat = regRt;
    regIDEX_IDside.regWrite = regWrite;
    regIDEX_IDside.instruction = regIFID_IDside.instruction;
}

//*******************************************
// thread_if_start
// This function simulates a thread that
// runs in parallel with the other pipeline
// stage combinational logic.

// Updated thread_ex_start function
void Cpu::thread_ex_start()
{
    // set the signal values from the outputs of the
    // appropriate pipeline registers
    unsigned int ALUControl = regIDEX_EXside.aluOperation;
    unsigned int regDest = regIDEX_EXside.regDst;
    unsigned int regRs = regIDEX_EXside.regRsDat;
    unsigned int regRt = regIDEX_EXside.regRtDat;
    unsigned int immed = regIDEX_EXside.immed_se;
    unsigned int aluSrc = regIDEX_EXside.aluSrc;
    unsigned int rsidx = regIDEX_EXside.rs;
    unsigned int rtidx = regIDEX_EXside.rt;
    unsigned int rdidx = regIDEX_EXside.rd;
    unsigned int wb_regWrite = regMEMWB_WBside.regWrite;
    unsigned int wb_registerNum = regMEMWB_WBside.registerNum;
    unsigned int wb_regWrData = regMEMWB_WBside.regWrData;
    unsigned int mem_regWrite = regEXMEM_MEMside.regWrite;
    unsigned int mem_registerNum = regEXMEM_MEMside.registerNum;
    unsigned int mem_aluResult = regEXMEM_MEMside.aluResult;

    // Forwarding Unit Logic
    unsigned int forwardA = 0;
    unsigned int forwardB = 0;

    // Check for forwarding from the MEM stage
    if (mem_regWrite && mem_registerNum != 0)
    {
        if (mem_registerNum == rsidx)
        {
            forwardA = 2;
            forwardingMessage = "Forwarding RS from MEM to ALU input";
        }
        if (mem_registerNum == rtidx)
        {
            forwardB = 2;
            forwardingMessage = "Forwarding RT from MEM to ALU input";
        }
    }

    // Check for forwarding from the WB stage
    if (wb_regWrite && wb_registerNum != 0)
    {
        if (wb_registerNum == rsidx && forwardA == 0)
        {
            forwardA = 1;
            forwardingMessage = "Forwarding RS from WB to ALU input";
        }
        if (wb_registerNum == rtidx && forwardB == 0)
        {
            forwardB = 1;
            forwardingMessage = "Forwarding RT from WB to ALU input";
        }
    }

    // alu source selection multiplexor and forwarding
    unsigned int operand1 = regRs;
    if (forwardA == 1)
        operand1 = wb_regWrData;
    if (forwardA == 2)
        operand1 = mem_aluResult;

    unsigned int dat2 = regRt;
    if (forwardB == 1)
        dat2 = wb_regWrData;
    if (forwardB == 2)
        dat2 = mem_aluResult;
    unsigned int operand2 = aluSrc ? immed : dat2;

    // ALU combinational logic
    int ALUResult = 0;
    if (ALUControl == 0x0)
        ALUResult = operand1 & operand2; // and
    if (ALUControl == 0x1)
        ALUResult = operand1 | operand2; // or
    if (ALUControl == 0x2)
        ALUResult = operand1 + operand2; // add
    if (ALUControl == 0x6)
        ALUResult = operand1 - operand2; // subtract
    if (ALUControl == 0x7)
        ALUResult = (operand1 < operand2) ? 1 : 0; // set less than
    if (ALUControl == 0xc)
        ALUResult = ~(operand1 | operand2); // nor
    int zero = (ALUResult == 0x00000000) ? 1 : 0;

    // register write destination multiplexor
    unsigned int regWrAddr = regDest ? rdidx : rtidx;

    // assign our outputs to the inputs of the associated
    // next-stage pipeline registers.
    regEXMEM_EXside.aluResult = ALUResult;
    regEXMEM_EXside.dat2 = dat2;
    regEXMEM_EXside.registerNum = regWrAddr;
    regEXMEM_EXside.memRead = regIDEX_EXside.memRead;
    regEXMEM_EXside.memToReg = regIDEX_EXside.memToReg;
    regEXMEM_EXside.memWrite = regIDEX_EXside.memWrite;
    regEXMEM_EXside.regWrite = regIDEX_EXside.regWrite;
    regEXMEM_EXside.instruction = regIDEX_EXside.instruction;
}

//*******************************************
// thread_mem_start
// This function simulates a thread that
// runs in parallel with the other pipeline
// stage combinational logic.
void Cpu::thread_mem_start()
{
    // set the signal values from the outputs of the
    // appropriate pipeline registers
    unsigned int ALUResult = regEXMEM_MEMside.aluResult;
    unsigned int memRead = regEXMEM_MEMside.memRead;
    unsigned int memToReg = regEXMEM_MEMside.memToReg;
    unsigned int memWrite = regEXMEM_MEMside.memWrite;
    unsigned int dat2 = regEXMEM_MEMside.dat2;

    // read the data memory if required
    unsigned int memData = dmem.read(ALUResult, memRead);

    // register write data multipelexor
    unsigned int regWrData = memToReg ? memData : ALUResult;

    // assign our outputs to the inputs of the associated
    // next-stage pipeline registers.
    regMEMWB_MEMside.registerNum = regEXMEM_MEMside.registerNum;
    regMEMWB_MEMside.regWrite = regEXMEM_MEMside.regWrite;
    regMEMWB_MEMside.regWrData = regWrData;
    regMEMWB_MEMside.instruction = regEXMEM_MEMside.instruction;

    // update memory at the end of this clock cycle
    dmem.update(ALUResult, dat2, memWrite);
}

//*******************************************
// thread_wb_start
// This function simulates a thread that
// runs in parallel with the other pipeline
// stage combinational logic.
void Cpu::thread_wb_start()
{
    // set the signal values from the outputs of the
    // appropriate pipeline registers
    unsigned int regWrAddr = regMEMWB_WBside.registerNum;
    unsigned int regWrite = regMEMWB_WBside.regWrite;
    unsigned int regWrData = regMEMWB_WBside.regWrData;

    // update the register contents at the first halof of
    // the clock cycle
    regs.update(regWrAddr, regWrData, regWrite);
}

//*************************************************
// update()
// This function simulates a single clock cycle of the
// CPU.
void Cpu::update()
{
    // run the pipeline threads and wait for them to all complete.
    // This isnt really threaded (in order to keep the code simple)
    // however, in real hardware each of these "threads" would run
    // in parallel with the others.
    thread_wb_start();
    thread_if_start();
    thread_id_start();
    thread_ex_start();
    thread_mem_start();

    // update the pipeline registers on the rising edge of the clock
    // in real hardware, these updates would all happen simultaneously
    regIFID_IDside = regIFID_IFside;
    regIDEX_EXside = regIDEX_IDside;
    regEXMEM_MEMside = regEXMEM_EXside;
    regMEMWB_WBside = regMEMWB_MEMside;

    clockCycle++;   
}

//**********************************************************************
// dump()
// dump the state of the CPU object to the standard output device
// Updated dump() function
void Cpu::dump()
{
    printf("Clock Cycle: %d\n", clockCycle);

    // Print forwarding messages
    if (forwardingMessage[0] != '\0')
    {
        printf("%s\n", forwardingMessage);
        forwardingMessage[0] = '\0'; // Clear the message
    }

    printf("PIPELINE\n");
    printf("IF:  %08x (PC = %08x)\n", imem.value(regIDEX_EXside.next_pc), regIDEX_EXside.next_pc);
    printf("ID:  %08x\n", regIFID_IDside.instruction);
    printf("EXE: %08x\n", regIDEX_EXside.instruction);
    printf("MEM: %08x\n", regEXMEM_MEMside.instruction);
    printf("WB:  %08x\n", regMEMWB_WBside.instruction);

    printf("REGISTER FILE\n");
    regs.dump();
    printf("\n");
}

bool Cpu::isProgramComplete() const
{
    // Check if the program is complete by comparing the PC with the address of "done:" instruction
    return (regIFID_IDside.next_pc== 0x08000062); // Assuming 0x88 is the address of "done:" instruction
}

void Cpu::updatePipeline()
{
    // 1. Update MEM/WB stage
    regMEMWB_WBside.regWrite = regEXMEM_MEMside.regWrite;
    regMEMWB_WBside.registerNum = regEXMEM_MEMside.registerNum;
    regMEMWB_WBside.regWrData = regEXMEM_MEMside.aluResult;
    regMEMWB_WBside.instruction = regEXMEM_MEMside.instruction;

    // 2. Update EX/MEM stage
    regEXMEM_MEMside.regWrite = regIDEX_EXside.regWrite;
    regEXMEM_MEMside.registerNum = regIDEX_EXside.registerNum;
    regEXMEM_MEMside.aluResult = regIDEX_EXside.aluResult;
    regEXMEM_MEMside.instruction = regIDEX_EXside.instruction;

    // 3. Update ID/EX stage
    regIDEX_EXside.regWrite = regIFID_IDside.regWrite;
    regIDEX_EXside.registerNum = regIFID_IDside.rt;
    regIDEX_EXside.aluResult = regs.read(regIFID_IDside.rs) + regIFID_IDside.immed_se;
    regIDEX_EXside.instruction = regIFID_IDside.instruction;

    // 4. Update IF/ID stage
    regIFID_IDside.regWrite = 1; // Set to 1 for simplicity, update according to your needs
    regIFID_IDside.rt = 1;       // Set to 1 for simplicity, update according to your needs
    regIFID_IDside.rs = 2;       // Set to 2 for simplicity, update according to your needs
    regIFID_IDside.instruction = imem.value(regIFID_IFside.next_pc);

    // 5. Call the thread functions to start new instructions in each stage
    thread_if_start();
    thread_id_start();
    thread_ex_start();
    thread_mem_start();
    thread_wb_start();
}