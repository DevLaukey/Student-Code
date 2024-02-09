#include <iostream>
#include <fstream>
#include <sstream>
#include "DataMemory.h"
#include "InstructionMemory.h"
#include "RegisterFile.h"
#include "Cpu.h" // Include the Cpu class header

int main(int argc, char *argv[])
{
    // Check if a filename and the number of cycles are provided as command-line arguments
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <filename> <num_cycles>" << std::endl;
        return 1; // Exit with an error code
    }

    // Extract the filename and the number of cycles from the command-line arguments
    std::string filename = argv[1];
    int numCycles;
    std::istringstream(argv[2]) >> numCycles;

    // Print a startup banner
    std::cout << "Startup Banner:" << std::endl;
    std::cout << "Author: Your Name" << std::endl;
    std::cout << "Course Number: Your Course Number" << std::endl;
    std::cout << "--------------------------------" << std::endl;

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
    for (unsigned int cycle = 0; cycle < numCycles; ++cycle)
    {
        // Print the current clock cycle
        std::cout << "Clock Cycle: " << cycle << std::endl;

        // Fetch instruction from instruction memory
        unsigned int pc = cycle * 4;
        unsigned int instruction = instructionMemory.value(pc);

        // Set the program counter using the Cpu's setImem function
        cpu.setImem(pc, instruction);

        // Update the Cpu for one clock cycle
        cpu.update();

        // Optionally, print the state of the Cpu (uncomment if needed)
        // cpu.dump();
    }

    // Print the final state of the register file using the Cpu's dump function
    std::cout << "Final Register File State:" << std::endl;
    cpu.dump();

    return 0;
}
