#include <iostream>
#include <fstream>
#include <sstream>
#include "DataMemory.h"
#include "InstructionMemory.h"
#include "RegisterFile.h"
#include "Cpu.h" // Include the Cpu header file

int main()
{
    // Create instances of each class
    DataMemory dataMemory;
    InstructionMemory instructionMemory;
    RegisterFile registerFile;
    Cpu cpu(dataMemory, registerFile); // Use the Cpu class

    // Load program code into instruction memory from input_debug.txt
    std::ifstream inputFile("input_debug.txt");
    std::string line;

    while (std::getline(inputFile, line))
    {
        // Parse the line into instruction address, instruction type, and instruction value
        unsigned int address, type, value;
        std::istringstream iss(line);
        iss >> std::hex >> address >> type >> value;

        // Store the instruction in instruction memory
        instructionMemory.setAt(address, value);
    }

    // Run the simulation
    cpu.update();

    return 0;
}
