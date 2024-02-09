#include <iostream>
#include <fstream>
#include <sstream>
#include "DataMemory.h"
#include "InstructionMemory.h"
#include "RegisterFile.h"
#include "Cpu.h" // Include the Cpu class header

int main(int argc, char *argv[])
{
    // Check if a filename is provided as a command-line argument
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1; // Exit with an error code
    }

    // Extract the filename from the command-line arguments
    std::string filename = argv[1];

    // Create instances of each class
    DataMemory dataMemory;
    InstructionMemory instructionMemory;
    RegisterFile registerFile;
    Cpu cpu(dataMemory, registerFile); // Create an instance of the Cpu class

    // Load program code into instruction memory from the specified file
    std::ifstream inputFile(filename);
    if (!inputFile.is_open())
    {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return 1; // Exit with an error code
    }

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

    // Simulate the fetch-decode-execute cycle using the Cpu class
    for (unsigned int pc = 0; pc < 2048; pc += 4)
    {
        // Fetch instruction from instruction memory
        unsigned int instruction = instructionMemory.value(pc);

        // Set the program counter using the Cpu's setImem function
        cpu.setImem(pc, instruction);

        // Update the Cpu for one clock cycle
        cpu.update();
    }

    // Print the final state of the register file using the Cpu's dump function
    cpu.dump();

    return 0;
}
