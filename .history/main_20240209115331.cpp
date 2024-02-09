#include <iostream>
#include <fstream>
#include "Cpu.h"          // Include necessary header files for Cpu, Datamemory, RegisterFile, etc.
#include "DataMemory.h"   // Replace with your actual DataMemory header
#include "RegisterFile.h" // Replace with your actual RegisterFile header
#include <string>

void printBanner()
{
    std::cout << "SER450 - Project 4a" << std::endl;
    std::cout << "Akhil Matthews" << std::endl;
}

int main(int argc, char *argv[])
{
    // Check if the correct number of command line arguments is provided
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file> <num_cycles>\n";
        return 1;
    }

    // Extract command line arguments
    std::string inputFileName = argv[1];
    int numCycles = std::stoi(argv[2]);

    // Print startup banner
    printBanner();

    // Read the input file
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open())
    {
        std::cerr << "Error: Unable to open input file\n";
        return 1;
    }

    // Initialize memory
    // Assuming DataMemory and RegisterFile are your classes representing memory components
    DataMemory dmem;
    RegisterFile regs;

    // Initialize CPU
    Cpu cpu(dmem, regs); // Assuming Cpu constructor requires DataMemory and RegisterFile

    // Run the simulation for the specified number of cycles
    for (int cycle = 1; cycle <= numCycles; ++cycle)
    {
        std::cout << "Cycle " << cycle << "\n";

        // Perform any necessary updates or operations before calling Cpu.update()
        // e.g., reading instructions from the input file and setting them in instruction memory

        // Call Cpu.update()
        cpu.update();

        // Display the number of clock cycles before each call to Cpu.update()
        // This may involve modifying Cpu.update() or using a separate counter in Cpu

        // Perform any necessary updates or operations after calling Cpu.update()
    }

    // Display the final results using Cpu.dump()
    cpu.dump();

    return 0;
}
