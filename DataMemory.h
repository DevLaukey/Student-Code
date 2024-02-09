/*************************************************************************
* DataMemory.h
*
* This file contains the class definition for the data memory component
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
#ifndef DATAMEMORY_H
#define DATAMEMORY_H


class DataMemory
{
    public:
        void update(unsigned int address, unsigned int data, bool write);
        unsigned int read(unsigned int addr, bool read);
    protected:

    private:
        unsigned char memory[2048];
};

#endif // DATAMEMORY_H
