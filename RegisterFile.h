/*************************************************************************
* RegisterFile.h
*
* This file contains the class definition for the register file
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
#ifndef REGISTERFILE_H
#define REGISTERFILE_H

class RegisterFile
{
    public:
        RegisterFile();
        void update(unsigned int addr, unsigned int data, bool write);
        unsigned int readData1(unsigned int addr);
        unsigned int readData2(unsigned int addr);

        void dump(); // dump the contents of the register file to the standard output for debugging
    private:
        unsigned int regs[32];
};

#endif // REGISTERFILE_H
