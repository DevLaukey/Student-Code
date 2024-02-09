#include <iostream>
#include "DataMemory.h"
#include "InstructionMemory.h"
#include "RegisterFile.h"

int main()
{
    // Create instances of each class
    DataMemory dataMemory;
    InstructionMemory instructionMemory;
    RegisterFile registerFile;

    // Load program code into instruction memory
    instructionMemory.setAt(0, 0x01234567); // Example instruction

    // Simulate the fetch-decode-execute cycle
    for (unsigned int pc = 0; pc < 2048; pc += 4)
    {
        // Fetch instruction from instruction memory
        unsigned int instruction = instructionMemory.value(pc);

        // Decode the instruction (implement based on your architecture)
        // Example: Decode the instruction to get register numbers and immediate value
        unsigned int rs = (instruction >> 21) & 0x1F;
        unsigned int rt = (instruction >> 16) & 0x1F;
        unsigned int rd = (instruction >> 11) & 0x1F;
        unsigned int immediate = instruction & 0xFFFF;

        // Read from register file
        unsigned int data1 = registerFile.readData1(rs);
        unsigned int data2 = registerFile.readData2(rt);

        // Example: Perform an operation (implement based on your instructions)
        unsigned int result = data1 + data2;

        // Write result back to register file
        registerFile.update(rd, result, true);
    }

    // Print the final state of the register file (for demonstration purposes)
    std::cout << "Register File:" << std::endl;
    registerFile.dump();

    return 0;
}
