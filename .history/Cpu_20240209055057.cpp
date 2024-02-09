/*************************************************************************
 * Cpu.cpp
 *
 * This file contains the class implementation for the pipelined CPU
 * defined in "Computer Organization and Design, the Hardware Software Interface",
 * Patterson & Hennessy, Fifth Edition.
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

#define BITS(x, start, end) ((x >> start) & ((1 << (end - start + 1)) - 1))
#define SIGN_EXT(x) ((x & 0x8000) ? (0xFFFF0000 | x) : x)

#define OP_LW 0x23
#define OP_SW 0x2B
#define OP_RTYPE 0x00
#define OP_BEQ 0x04
#define OP_JMP 0x02

Cpu::Cpu()
{
    regIFID_IDside = {0};
    regIDEX_EXside = {0};
    regEXMEM_MEMside = {0};
    regMEMWB_WBside = {0};
}

void Cpu::setImem(unsigned int address, unsigned int value)
{
    imem.setAt(address, value);
}

void Cpu::setDmem(unsigned int address, unsigned int value)
{
    dmem.update(address, value, true);
}

void Cpu::thread_if_start()
{
    unsigned int pc = regIDEX_EXside.next_pc;
    unsigned int instruction = imem.value(pc);
    regIFID_IFside.instruction = instruction;
}

void Cpu::thread_id_start()
{
    unsigned int instruction = regIFID_IDside.instruction;
    unsigned int pc = regIDEX_EXside.next_pc;

    // Extracted fields from the instruction
    unsigned int opcode = BITS(instruction, 26, 31);
    unsigned int funct = BITS(instruction, 0, 5);
    unsigned int rdidx = BITS(instruction, 11, 15);
    unsigned int rtidx = BITS(instruction, 16, 20);
    unsigned int rsidx = BITS(instruction, 21, 25);
    signed int immed = SIGN_EXT(BITS(instruction, 0, 15));
    unsigned int jmpaddr = BITS(instruction, 0, 25);

    // ALU operation and control combinational logic
    int ALUOp = 0x0;
    if ((opcode == OP_LW) || (opcode == OP_SW))
        ALUOp = 0x0; // Load or store instructions
    if (opcode == OP_BEQ)
        ALUOp = 0x1; // Branch on equal instruction
    if (opcode == OP_RTYPE)
        ALUOp = 0x2; // R-type instructions

    int ALUControl = 0xF;
    if (ALUOp == 0x0)
        ALUControl = (opcode == OP_LW) ? 0x2 : 0x6; // lw/sw -> add/subtract
    if (ALUOp == 0x01)
        ALUControl = 0x1; // Branch on equal instruction
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
        if (funct == 0x2A)
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

    // ALU source selection multiplexer
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

    // Read the data memory if required
    unsigned int memData = dmem.read(ALUResult, memRead);

    // Register write data multiplexer
    unsigned int regWrData = memToReg ? memData : ALUResult;

    // Register write destination multiplexer
    unsigned int regWrAddr = regDest ? rdidx : rtidx;

    // Multiplexors to select the next program counter value
    unsigned int branchAddr = (immed << 2) + pc;
    unsigned int fullJumpAddr = ((pc) & 0xF0000000) | (jmpaddr << 2);
    unsigned int next_pc = (branch && (ALUResult == 0x00000000)) ? branchAddr : pc + 4;
    next_pc = (jump) ? fullJumpAddr : next_pc;

    // Assign outputs to inputs of the next-stage pipeline registers
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

void Cpu::thread_ex_start()
{
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

    // Forwarding Unit
    unsigned int forwardA = 0;
    unsigned int forwardB = 0;

    // ForwardA logic
    if (regIDEX_EXside.regWrite && (regDest == rsidx))
    {
        printf("Forwarding RS from EX to ALU input\n");
        forwardA = 2; // Forward from EX
    }
    else if (mem_regWrite && (mem_registerNum != 0) && (mem_registerNum == rsidx))
    {
        printf("Forwarding RS from MEM to ALU input\n");
        forwardA = 1; // Forward from MEM
    }

    // ForwardB logic
    if (regIDEX_EXside.regWrite && (regDest == rtidx))
    {
        printf("Forwarding RT from EX to ALU input\n");
        forwardB = 2; // Forward from EX
    }
    else if (mem_regWrite && (mem_registerNum != 0) && (mem_registerNum == rtidx))
    {
        printf("Forwarding RT from MEM to ALU input\n");
        forwardB = 1; // Forward from MEM
    }

    // ALU source selection multiplexor and forwarding
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
        ALUResult = operand1 & operand2; // AND
    if (ALUControl == 0x1)
        ALUResult = operand1 | operand2; // OR
    if (ALUControl == 0x2)
        ALUResult = operand1 + operand2; // ADD
    if (ALUControl == 0x6)
        ALUResult = operand1 - operand2; // SUBTRACT
    if (ALUControl == 0x7)
        ALUResult = (operand1 < operand2) ? 1 : 0; // SET LESS THAN

    // Register write destination multiplexor
    unsigned int regWrAddr = regDest ? rdidx : rtidx;

    // Assign outputs to inputs of the next-stage pipeline registers
    regEXMEM_EXside.aluResult = ALUResult;
    regEXMEM_EXside.dat2 = dat2;
    regEXMEM_EXside.registerNum = regWrAddr;
    regEXMEM_EXside.memRead = regIDEX_EXside.memRead;
    regEXMEM_EXside.memToReg = regIDEX_EXside.memToReg;
    regEXMEM_EXside.memWrite = regIDEX_EXside.memWrite;
    regEXMEM_EXside.regWrite = regIDEX_EXside.regWrite;
    regEXMEM_EXside.instruction = regIDEX_EXside.instruction;
}

void Cpu::thread_mem_start()
{
    unsigned int ALUResult = regEXMEM_MEMside.aluResult;
    unsigned int memRead = regEXMEM_MEMside.memRead;
    unsigned int memToReg = regEXMEM_MEMside.memToReg;
    unsigned int memWrite = regEXMEM_MEMside.memWrite;
    unsigned int dat2 = regEXMEM_MEMside.dat2;

    // Read the data memory if required
    unsigned int memData = dmem.read(ALUResult, memRead);

    // Register write data multiplexer
    unsigned int regWrData = memToReg ? memData : ALUResult;

    // Assign outputs to inputs of the next-stage pipeline registers
    regMEMWB_MEMside.registerNum = regEXMEM_MEMside.registerNum;
    regMEMWB_MEMside.regWrite = regEXMEM_MEMside.regWrite;
    regMEMWB_MEMside.regWrData = regWrData;
    regMEMWB_MEMside.instruction = regEXMEM_MEMside.instruction;

    // Update memory at the end of this clock cycle
    dmem.update(ALUResult, dat2, memWrite);
}

void Cpu::thread_wb_start()
{
    unsigned int regWrAddr = regMEMWB_WBside.registerNum;
    unsigned int regWrite = regMEMWB_WBside.regWrite;
    unsigned int regWrData = regMEMWB_WBside.regWrData;

    // Update the register contents at the first half of the clock cycle
    regs.update(regWrAddr, regWrData, regWrite);
}

void Cpu::update()
{
    // Run the pipeline threads and wait for them to all complete.
    thread_wb_start();
    thread_if_start();
    thread_id_start();
    thread_ex_start();
    thread_mem_start();

    // Update the pipeline registers on the rising edge of the clock
    regIFID_IDside = regIFID_IFside;
    regIDEX_EXside = regIDEX_IDside;
    regEXMEM_MEMside = regEXMEM_EXside;
    regMEMWB_WBside = regMEMWB_MEMside;
}

void Cpu::dump()
{
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
