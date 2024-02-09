// main.cpp
#include <iostream>
#include "Cpu.h"
#include <string>

int main(int argc, char *argv[])
{
    // Check for the correct number of command line arguments
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file> <num_cycles>\n";
        return 1;
    }

    // Print startup banner
    std::cout << "SER450 - Project 4a" << std::endl;
    std::cout << "Akhil Matthews" << std::endl;

    // Parse command line arguments
    std::string inputFile = argv[1];
    int numCycles = std::stoi(argv[2]);

    // Initialize the CPU
    Cpu cpu;

    // Set up the initial state, e.g., load instructions and data into memory

    // Run the simulation for the specified number of cycles
    for (int cycle = 1; cycle <= numCycles; ++cycle)
    {
        // Display the number of clock cycles before each call to Cpu.update()
        std::cout << "Clock Cycle: " << cycle << std::endl;

        // Run a single clock cycle of the CPU
        cpu.update();

        // Optional: You may add additional logic to print the state of the CPU at each cycle
        // For example: cpu.dump();
    }

    // Display the final results using Cpu.dump() function
    std::cout << "Final State of the CPU:\n";
    cpu.dump();

    return 0;
}
