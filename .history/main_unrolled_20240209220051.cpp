#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

// Instruction structure to store information about each instruction
struct Instruction
{
    int opcode;
    int rs, rt, rd, immediate;
    string label;

    Instruction(int opc, int s, int t, int d, int imm, string lab) : opcode(opc), rs(s), rt(t), rd(d), immediate(imm), label(lab) {}
};

// Pipeline stage enum
enum PipelineStage
{
    IF,
    ID,
    EX,
    MEM,
    WB
};

// Pipeline structure to store information about each stage in the pipeline
struct Pipeline
{
    PipelineStage stage;
    Instruction instruction;
    int remainingClocks;

    Pipeline(PipelineStage st, Instruction instr, int clocks) : stage(st), instruction(instr), remainingClocks(clocks) {}
};

// Function to parse the input file and return a vector of instructions
vector<Instruction> parseInputFile(const string &filename)
{
    vector<Instruction> instructions;
    ifstream inputFile(filename);

    if (inputFile.is_open())
    {
        string line;
        while (getline(inputFile, line))
        {
            istringstream iss(line);
            int opcode, rs, rt, rd, immediate;
            string label;

            iss >> opcode >> rs >> rt >> rd >> immediate >> label;
            instructions.emplace_back(opcode, rs, rt, rd, immediate, label);
        }

        inputFile.close();
    }
    else
    {
        cerr << "Error opening file: " << filename << endl;
    }

    return instructions;
}

// Function to simulate the processor pipeline and calculate clock cycles
int simulatePipeline(const vector<Instruction> &instructions)
{
    map<string, int> labelIndices; // Map to store the index of each label
    vector<Pipeline> pipeline;     // Vector to store the pipeline stages
    int clockCycles = 0;
    int currentIndex = 0;

    // Initialize label indices
    for (size_t i = 0; i < instructions.size(); ++i)
    {
        labelIndices[instructions[i].label] = i;
    }

    while (true)
    {
        // Execute each stage of the pipeline
        for (int i = pipeline.size() - 1; i >= 0; --i)
        {
            Pipeline &currentStage = pipeline[i];
            if (currentStage.remainingClocks == 0)
            {
                switch (currentStage.stage)
                {
                case WB:
                    // Write Back Stage
                    // Update register values if necessary
                    break;
                case MEM:
                    // Memory Access Stage
                    // Perform memory operations if necessary
                    break;
                case EX:
                    // Execute Stage
                    // Perform ALU operations if necessary
                    break;
                case ID:
                    // Instruction Decode Stage
                    // Decode the instruction and move to the Execute stage
                    currentStage.stage = EX;
                    currentStage.remainingClocks = 1; // Example: 1 clock cycle for execution
                    break;
                case IF:
                    // Instruction Fetch Stage
                    // Fetch the instruction and move to the Decode stage
                    currentStage.stage = ID;
                    currentStage.remainingClocks = 1; // Example: 1 clock cycle for decoding
                    break;
                }
            }
            else
            {
                currentStage.remainingClocks--;
            }
        }

        // Check if the program is completed
        if (currentIndex < instructions.size())
        {
            // Fetch the next instruction and add it to the pipeline
            pipeline.insert(pipeline.begin(), Pipeline(IF, instructions[currentIndex], 0));
            currentIndex++;
        }

        // Check if the first instruction has completed the pipeline
        if (!pipeline.empty() && pipeline.back().stage == WB && pipeline.back().remainingClocks == 0)
        {
            break; // The program has completed
        }

        // Increment clock cycle
        clockCycles++;
    }

    return clockCycles;
}

int main()
{
    // Parse the input file
    vector<Instruction> instructions = parseInputFile("input_unrolled.txt");

    // Simulate the processor pipeline
    int clockCycles = simulatePipeline(instructions);

    // Output the result
    cout << "Processor clock cycles required: " << clockCycles << endl;

    return 0;
}
