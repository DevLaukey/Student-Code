/*************************************************************************
* InstructionMemory.h
*
* This file contains the class definition for the instruction memory
* component of the single-cycle machine defined in "Computer Organization and
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
#ifndef INSTRUCTIONMEMORY_H_INCLUDED
#define INSTRUCTIONMEMORY_H_INCLUDED

class InstructionMemory {
public:
    unsigned int value(unsigned int pc);
    void setAt(unsigned int addr, unsigned int value);
private:
    unsigned int memory[2048];

};

#endif // INSTRUCTIONMEMORY_H_INCLUDED
