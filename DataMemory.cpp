/*************************************************************************
* DataMemory.cpp
*
* This file contains the class implementation for the data memory component
* of the single-cycle machine defined in "Computer Organization and
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
#include "DataMemory.h"

void DataMemory::update(unsigned int address, unsigned int data, bool write)
// called for each system clock tick to update the state of the component
// if write is true, store the data in memory.  otherwise, do nothing
{
    if (address > 2048) return;
    if (write) {
        memory[address] = data & 0xff;
        memory[address+1] = (data>>8)&0xff;
        memory[address+2] = (data>>16)&0xff;
        memory[address+3] = (data>>24)&0xff;
    }
}

unsigned int DataMemory::read(unsigned int address, bool read)
{
    // check to make sure the address is within our memory space
    if (address>2048) return 0;

    // returns the value of the currently addressed word
    return (((unsigned int)memory[address+3])<<24) +
            (((unsigned int)memory[address+2])<<16) +
            (((unsigned int)memory[address+1])<<8) +
            ((unsigned int)memory[address+0]);
}
