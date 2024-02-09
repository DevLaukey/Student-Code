/*************************************************************************
* Cpu.h
*
* This file contains the class definition for the CPU
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

#ifndef CPU_H
#define CPU_H
#include "DataMemory.h"
#include "InstructionMemory.h"
#include "RegisterFile.h"


class Cpu
{
private:
    // typedefs for the pipeline registers
    typedef struct {
        // control and data signals from the IF stage
        unsigned int instruction;
    } ifid_reg;

    typedef struct {
        // control and data from the ID stage 
        unsigned char regWrite;
        unsigned char memToReg;
        unsigned char memRead;
        unsigned char memWrite;
        unsigned char regDst;
        unsigned char aluOperation;
        unsigned char aluSrc;
        unsigned int  regRsDat;
        unsigned int  regRtDat;
        unsigned int  rs;
        unsigned int  rt;
        unsigned int  rd;
        unsigned int  immed_se;
        unsigned int  next_pc;  // this pipeline field is used to hold
                                // the value of the PC register
        unsigned int  instruction; // this is only used for dump support
    } idex_reg;

    typedef struct {
        // control and data from the EX stage
        unsigned char regWrite;
        unsigned char memToReg;
        unsigned char memRead;
        unsigned char memWrite;
        unsigned int  aluResult;
        unsigned int  dat2;         // the value of the second operand 
                                    // (used in store operations)
        unsigned char registerNum;  // the register index to write back to
                                    // in the writeback stage
        unsigned int  instruction; // this is only used for dump support
    } exmem_reg;

    typedef struct {
        unsigned char regWrite;
        unsigned int  regWrData;   // the data to be written back
        unsigned char registerNum; // the regter index to write back to
        unsigned int  instruction; // this is only used for dump support
    } memwb_reg;

    // data members for the class
    InstructionMemory imem;
    DataMemory dmem; 
    RegisterFile regs;
    
    // pipeline registers - each one is represented by an input side
    // and an output side.  At the end of each clock cycle, the output
    // side values will be set to the input side values.
    // combinational logic will use the output side of the registers
    //    as input to the combinational logic.
    //
    //     regY_xside  -->  x_combinational_logic  --> regZ_xside
    ifid_reg   regIFID_IFside,   regIFID_IDside;
    idex_reg   regIDEX_IDside,   regIDEX_EXside;
    exmem_reg  regEXMEM_EXside,  regEXMEM_MEMside;
    memwb_reg  regMEMWB_MEMside, regMEMWB_WBside;

    // "thread functions" for each of the 
    // pipelined combinational logic.
    void thread_if_start();
    void thread_id_start();
    void thread_ex_start();
    void thread_mem_start();
    void thread_wb_start();

public:
    Cpu();
    void update(); // run the simulation
    void setDmem(unsigned int addr, unsigned int data);  // place a value in data memory
    void setImem(unsigned int addr, unsigned int data);  // place a value in instruction memory
    void dump(); // dump the cpu state to the standard output device
};

#endif // CPU_H
