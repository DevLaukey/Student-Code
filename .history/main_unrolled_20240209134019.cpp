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
    unsigned int pc = 0;
    unsigned int clockCycles = 0;

    while (true)
    {
        // Fetch instruction from instruction memory
        unsigned int instruction = instructionMemory.value(pc);

        // Print debug information
        std::cout << "Clock Cycle " << clockCycles << ": Instruction " << std::hex << instruction << " at PC " << pc << std::dec << std::endl;

        // Set the program counter using the Cpu's setImem function
        cpu.setImem(pc, instruction);

        // Update the Cpu for one clock cycle
        cpu.update();

        // Increment the clock cycle count
        clockCycles++;

        // print instruction
        std::cout << "Instruction: " << std::hex << instruction << std::dec << std::endl;
        // Check if the "done:" instruction has completed the pipeline
        if (instruction == 0x08000062) // Check if the current instruction is "done:"
        {
            std::cout << "Program completed in " << clockCycles << " clock cycles." << std::endl;
            break;
        }

        // Print additional debug information if needed
        // Example: std::cout << "Register File State:" << std::endl; cpu.dump();
    }

    // Print the final state of the register file using the Cpu's dump function
    std::cout << "Final Register File State:" << std::endl;
    cpu.dump();


    return 0;
}
