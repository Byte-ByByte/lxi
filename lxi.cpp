#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <unordered_map>

/*
!! WARNING !!
This is an UNRELEASED PREVIEW version. Versions like these test the newest features WITHOUT TESTING.
Use this version at your own risk.
Any bugs or issues that arise from using this version are not the responsibility of the developer.
By continuing to use this version, you acknowledge that you understand the risks and are willing to accept them.
*/ 

// Define structures
struct Process {
    int id; // Process ID
    std::string name; // Process name
    ProcessStatus status; // Process status (running, stopped, etc.)
};

struct IOObjectResponse {
    std::string content; // Content of the object
    std::string name; // Name of the object
    int size; // Size of the object
};

struct IOObjectRequest {
    std::string name; // Name of the IO object
    std::string path; // Path to the IO object
    std::string content; // Content of the IO object
    int pointer; // Pointer to the IO object
    IOType type; // Type of the IO request
};

// Define enums
enum ProcessStatus {
    RUNNING,
    STOPPED,
    SUSPENDED
};

enum IOType {
    FILE,
    SOCKET,
    MEMORY
};

// Define constants
#define MAX_VARIABLES = 100; // Maximum number of variables
#define MAX_PROCESSES = 1000; // Maximum number of processes
#define MAX_PARAMETERS = 100; // Maximum number of parameters
#define MAX_LINES = 10000; // Maximum number of lines in a program
std::vector<char> base90chars;

// Initialize base90chars with ASCII characters from 33 to 126, excluding quotes
// This is used for encoding and decoding purposes
void initializeBase90Chars() {
    for (int i = 33; i <= 126; i++) {
        if (i != 34 && i != 39) base90chars.push_back(static_cast<char>(i));
    }
}


// Initialize variables
std::string line; // Line to execute
std::vector<std::string> paramarray; // Array of parameters
std::vector<std::string> lines; // Lines to execute
std::vector<std::string> sharedSockets; // Shared sockets that are shared with other processes
std::vector<std::vector<std::string>> pooledMemory;
std::vector<int> axfarray; // Array of axf values used by the program
std::vector<int> variableIndices; // Array of variable indexes used by the program
std::vector<int> variables; // Array of variables used by the program
std::vector<Process> processList; // List of processes used by the program
int variableIndex = 0; // Index of the variable to use
int pc = 0; // Program counter
int wait = 0; // Wait time for the interpreter's if statement
bool runningInterpreter = true; // Flag to indicate if either the interpreter or the program is running

// Helper function to split a string by a delimiter
std::vector<std::string> split(const std::string &str, char delimiter) {
    size_t start = 0;
    size_t end = str.find(delimiter);
    std::vector<std::string> result;

    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    result.push_back(str.substr(start, end));

    return result;
}

// Function to respond to an IO object request
IOObjectResponse respondToIOObjectRequest(const IOObjectRequest &request) {
    IOObjectResponse response;
    
    // Handle the request based on its type
    if (request.type == IOType::FILE) {
        response.content = "File content"; // Placeholder for file content
    } else if (request.type == IOType::SOCKET) {
        // Get the content from the sharedSockets vector
        response.content = sharedSockets[request.pointer];
    } else if (request.type == IOType::MEMORY) {
        // Get the content from the process's pooledMemory vector
        response.content = "Memory content"; // Placeholder for memory content
    } else {
        response.content = "Unknown type"; // Handle unknown types
    }
    return response; // Return the response
}

// Function to encode an unsigned 8-bit integer to a string
std::string uint8MSafeEncode(uint8_t value) {
    std::string encoded;
    


    return encoded; // Return the encoded string
}

// Helper function to find a substring in a string
size_t findString(const std::string& str, const std::string& toFind) {
    return str.find(toFind);
}

void executeLine(const std::string &line) {
    // This function can be used to execute a line of code
    if (line == "quit") {
        std::cout << "Exiting..." << std::endl;
        exit(0); // Exit the program
    } else if (line == "help") {
        std::cout << "Available commands: quit, help, print" << std::endl;
    } else if (line == "debug//printparams") {
        // This is a debug command to print the parameters
        std::cout << "\033[30;45mDEBUG:\033[30;43mWARNING:\033[40;1m 'debug//printparams' is deprecated and only used for debugging the LXI interpreter. You may only see this operation in preview versions.\033[0m" << std::endl;
        for (const std::string& param : paramarray) {
            std::cout << param;
            if (param != paramarray.back()) {
                std::cout << ", "; // Add a comma between parameters
            }
        }
        std::cout << std::endl;
    } else if (line == "print") {
        std::string message = paramarray[0];
        if (message.empty()) { // Check if the message is empty
            std::cout << "\033[30;41mERROR:\033[40;1m 0x0000000B INVALID_MESSAGE\033[0m" << std::endl;
            std::cout << "The message is empty." << std::endl;
            return;
        }
        // Clear paramarray for the next command
        paramarray.clear();
        std::cout << message << std::endl;
    } else if (line == "setparameter") {
        if (lines.size() < 2) { // Ensure there is a second part in the input
            std::cout << "\033[30;41mERROR:\033[40;1m 0x0000000C INVALID_PARAMETER\033[0m" << std::endl;
            std::cout << "The parameter is empty." << std::endl;
            return;
        }
        std::string parameter = lines[1];
        paramarray.push_back(parameter); // Add the parameter to the paramarray
    } else if (line == "removeparameter") {
        if (lines.size() < 2) { // Ensure there is a second part in the input
            std::cout << "\033[30;41mERROR:\033[40;1m 0x0000000D INVALID_PARAMETER\033[0m" << std::endl;
            return;
        }
        try {
            int index = std::stoi(lines[1]); // Convert the second part to an integer index
            if (index < 0 || index >= static_cast<int>(paramarray.size())) {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x0000000F INDEX_OUT_OF_BOUNDS\033[0m" << std::endl;
            std::cout << "The index " << index << " is out of bounds." << std::endl;
            return;
            }
            paramarray.erase(paramarray.begin() + index); // Remove the parameter at the specified index
        } catch (const std::invalid_argument&) {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x00000010 INVALID_INDEX\033[0m" << std::endl;
            std::cout << "The provided index is not a valid number." << std::endl;
        } catch (const std::out_of_range&) {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x00000011 INDEX_OUT_OF_RANGE\033[0m" << std::endl;
            std::cout << "The provided index is out of range." << std::endl;
        }
    } else if (line == "changeparameter") {
        if (lines.size() < 3) { // Ensure there are enough parts in the input
            std::cout << "\033[30;41mERROR:\033[40;1m 0x00000012 INVALID_PARAMETER\033[0m" << std::endl;
            std::cout << "The parameter is empty." << std::endl;
            return;
        }
        try {
            int index = std::stoi(lines[1]); // Convert the second part to an integer index
            if (index < 0 || index >= static_cast<int>(paramarray.size())) {
                std::cout << "\033[30;41mERROR:\033[40;1m 0x00000013 INDEX_OUT_OF_BOUNDS\033[0m" << std::endl;
                std::cout << "The index " << index << " is out of bounds." << std::endl;
                return;
            }
            paramarray[index] = lines[2]; // Change the parameter at the specified index
        } catch (const std::invalid_argument&) {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x00000014 INVALID_INDEX\033[0m" << std::endl;
            std::cout << "The provided index is not a valid number." << std::endl;
        } catch (const std::out_of_range&) {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x00000015 INDEX_OUT_OF_RANGE\033[0m" << std::endl;
            std::cout << "The provided index is out of range." << std::endl;
        }
    } else if (line == "clearparameters") {
        paramarray.clear(); // Clear all parameters
    } else if (line == "math") {
        if (lines.size() < 2 || paramarray.size() < 2) {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x00000016 INVALID_PARAMETERS\033[0m" << std::endl;
            std::cout << "Insufficient parameters for math operation." << std::endl;
            return;
        }
    
        std::string mode = lines[1];
        int a = std::stoi(paramarray[0]);
        int b = std::stoi(paramarray[1]);
        paramarray.clear();
    
        if (mode == "+") {
            paramarray.push_back(std::to_string(a + b));
        } else if (mode == "-") {
            paramarray.push_back(std::to_string(a - b));
        } else if (mode == "*") {
            paramarray.push_back(std::to_string(a * b));
        } else if (mode == "/") {
            if (b == 0) {
                std::cout << "\033[30;41mERROR:\033[40;1m 0x00000017 DIVISION_BY_ZERO\033[0m" << std::endl;
                std::cout << "Division by zero is not allowed." << std::endl;
                return;
            }
            paramarray.push_back(std::to_string(a / b));
        } else if (mode == "%") {
            if (b == 0) {
                std::cout << "\033[30;41mERROR:\033[40;1m 0x00000017 DIVISION_BY_ZERO\033[0m" << std::endl;
                std::cout << "Modulo by zero is not allowed." << std::endl;
                return;
            }
            paramarray.push_back(std::to_string(a % b));
        } else if (mode == "m") {
            paramarray.push_back(std::to_string(std::min(a, b)));
        } else if (mode == "M") {
            paramarray.push_back(std::to_string(std::max(a, b)));
        } else if (mode == "s") {
            paramarray.push_back(std::to_string(std::sin(a)));
        } else if (mode == "c") {
            paramarray.push_back(std::to_string(std::cos(a)));
        } else {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x00000018 INVALID_MATH_OPERATION\033[0m" << std::endl;
            std::cout << "The math operation '" << mode << "' is not supported." << std::endl;
        }
    } else if (line == "expression") {
        if (lines.size() < 2 || paramarray.size() < 2) {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x00000019 INVALID_PARAMETERS\033[0m" << std::endl;
            std::cout << "Insufficient parameters for expression evaluation." << std::endl;
            return;
        }
    
        std::string mode2 = lines[1];
        std::string c = paramarray[0];
        std::string d = paramarray[1];
        paramarray.clear();
    
        if (mode2 == "=") {
            paramarray.push_back((c == d) ? "true" : "false");
        } else if (mode2 == ">") {
            paramarray.push_back((std::stoi(c) > std::stoi(d)) ? "true" : "false");
        } else if (mode2 == "<") {
            paramarray.push_back((std::stoi(c) < std::stoi(d)) ? "true" : "false");
        } else if (mode2 == ">=") {
            paramarray.push_back((std::stoi(c) >= std::stoi(d)) ? "true" : "false");
        } else if (mode2 == "<=") {
            paramarray.push_back((std::stoi(c) <= std::stoi(d)) ? "true" : "false");
        } else if (mode2 == "&") {
            paramarray.push_back((c == "true" && d == "true") ? "true" : "false");
        } else if (mode2 == "|") {
            paramarray.push_back((c == "true" || d == "true") ? "true" : "false");
        } else if (mode2 == "x") {
            paramarray.push_back(((c == "false" && d == "true") || (c == "true" && d == "false")) ? "true" : "false");
        } else if (mode2 == "!") {
            paramarray.push_back((c == "false") ? "true" : "false");
        } else {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x00000009 INVALID_EXPRESSION_TYPE\033[0m" << std::endl;
            std::cout << "The expression type '" << mode2 << "' is not supported." << std::endl;
        }
    } else if (line == "nop") {
        // Do nothing
    } else if (line == "let") {
        variableIndex = std::find(variableIndices.begin(), variableIndices.end(), std::stoi(paramarray[0])) - variableIndices.begin(); // Find the index of the variable in the variableIndices array
        if (variableIndex == variableIndices.size()) {
            variableIndices.push_back(std::stoi(paramarray[0])); // Add the variable index to the variableIndices array
            variables.push_back(0); // Initialize the variable to 0
            variableIndex = variableIndices.size() - 1; // Set the variableIndex to the last index
        }
        variables[variableIndex] = std::stoi(paramarray[1]); // Set the variable to the value of the second parameter
    } else if (line == "get") {
        variableIndex = std::find(variableIndices.begin(), variableIndices.end(), std::stoi(paramarray[0])) - variableIndices.begin(); // Find the index of the variable in the variableIndices array
        if (variableIndex == variableIndices.size()) {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x0000000E VARIABLE_NOT_FOUND\033[0m" << std::endl;
            std::cout << "The variable '" << paramarray[0] << "' was not found." << std::endl;
            return;
        }
        paramarray.push_back(std::to_string(variables[variableIndex])); // Add the value of the variable to the paramarray
    } else if (line == "if") {
        std::string expression = paramarray[0]; // Get the expression to evaluate
        int ifLength = std::stoi(lines[1]); // Get the length of the condition

        if (expression == "false") {
            pc += ifLength; // Skip the next lines if the expression is false
            wait += ifLength; // Add the length to skip the lines after the if statement
        }
    } else if (line == "goto") {
        if (paramarray.size() < 1) { // Ensure there is a parameter to go to
            std::cout << "\033[30;41mERROR:\033[40;1m 0x0000000F INVALID_PARAMETER\033[0m" << std::endl;
            std::cout << "The parameter is empty." << std::endl;
            return;
        }
        try {
            pc = std::stoi(paramarray[0]); // Set the program counter to the specified line number
        } catch (const std::invalid_argument&) {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x00000010 INVALID_PARAMETER\033[0m" << std::endl;
            std::cout << "The parameter is not a valid number." << std::endl;
            return;
        } catch (const std::out_of_range&) {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x00000011 PARAMETER_OUT_OF_RANGE\033[0m" << std::endl;
            std::cout << "The parameter is out of range." << std::endl;
            return;
        }
        if (runningInterpreter) {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x00000012 FLOW_CONTROL_ERROR\033[0m" << std::endl;
            std::cout << "The command 'goto' cannot be run in the interpreter." << std::endl;
            return;
        }
        pc = std::stoi(paramarray[0]); // Set the program counter to the specified line number
    } else if (line == "openinputobject") {
    } else {
        std::cout << "\033[30;41mERROR:\033[40;1m 0x0000000A INVALID_OPERATION\033[0m" << std::endl;
        std::cout << "The command '" << line << "' is not a supported operation." << std::endl;
    }
}

int main(int argc, char* argv[]) {

    // Create the default process
    Process defaultProcess;
    defaultProcess.id = 0; // Set the default process ID to 0
    defaultProcess.name = "executable"; // Set the default process name to "executable". Can be changed by the program.
    defaultProcess.status = ProcessStatus::RUNNING; // Set the default process status to PROCESS_STATUS_RUNNING. Can be changed by the program or the interpreter.

    processList.push_back(defaultProcess); // Initialize the process list with a default value

    if (argc < 2) {
        // Launch the interpreter if no arguments are provided

        // Print the welcome message
        std::cout << "lxi Interpreter (04-05-25/0.0.0a-preview[unreleased]) [x64]" << std::endl;
        std::cout << "Copyright (C) 2024-2025 Miguel Ignacio" << std::endl;
        std::cout << "Type help or license to view information about the interpreter." << std::endl;
        std::cout << "Type quit to exit the interpreter." << std::endl;
        std::cout << std::endl;
        std::cout << "\033[30;43mWARNING:\033[40;1m This is an UNRELEASED PREVIEW version. Versions like these test the newest features WITHOUT TESTING. By continuing to use this version, you may be subject to major bugs. For more information, please refer to the text shown on screen.\033[0m" << std::endl; // NOTE: Use revised version in released preview releases
        
        // Revised version:
        // std::cout << "\033[30;43mWARNING:\033[40;1m This is a PREVIEW version. Versions like these test the newest features with mild testing. You may use this version to help beta-test the interpreter.\033[0m" << std::endl;
        
        // NOTE: Remove from public/released preview release
        std::ifstream warningTextFile ("warning.txt"); // Open the warning text file
        if (warningTextFile.is_open()) {
            std::string line;
            while (std::getline(warningTextFile, line)) {
                std::cout << line << std::endl; // Print each line of the warning text file
            }
            warningTextFile.close(); // Close the file after reading
        } else {
            std::cout << "\033[30;41mERROR:\033[40;1m 0x00000001 WARNING_FILE_NOT_FOUND\033[0m" << std::endl;
            std::cout << "The warning text file (warning.txt) was not found. Exiting..." << std::endl;
            return 1; // Exit with an error code
        }

        std::cout << "\033[31mDo you understand the risks of using this version? (y/n)\033[0m ";

        std::string response;
        response = std::cin.get(); // Get the response from the user
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the rest of the line

        if (response == "y" || response == "Y") {
            std::cout << "\033[30;42mContinuing...\033[0m" << std::endl;
        } else if (response == "n" || response == "N") {
            std::cout << "\033[30;41mExiting...\033[0m" << std::endl;
            return 0; // Exit the program
        } else {
            std::cout << "\033[30;41mInvalid response. Exiting...\033[0m" << std::endl;
            return 1; // Exit with an error code
        }

        std::cout << std::endl; // Seperate the welcome message from the prompt

        // Main loop
        while (true) {
            std::cout << "\33[30;42mlxi:>\33[0m "; // Print the prompt
            std::getline(std::cin, line); // Read the entire line of input

            if (line.empty()) {
                continue; // Skip empty lines
            }

            if (line == "help") {
                std::cout << "lxi Interpreter (04-05-25/0.0.0a-preview[unreleased]) [x64]" << std::endl;
                std::cout << "Copyright (C) 2024-2025 Miguel Ignacio" << std::endl;
                std::cout << "Type help or license to view information about the interpreter." << std::endl;
            } else if (line == "license") {
                std::cout << "lxi Interpreter (04-05-25/0.0.0a-preview[unreleased]) [x64]" << std::endl;
                std::cout << "Copyright (C) 2024-2025 Miguel Ignacio" << std::endl;
                std::cout << "This program is licensed under the MIT License." << std::endl;
                std::cout << "\33[30;47m" << std::endl;
                std::cout << "Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:" << std::endl;
                std::cout << "The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software." << std::endl;
                std::cout << "THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE." << std::endl;
                std::cout << "\33[0m" << std::endl;
                std::cout << std::endl;
            } else {
                // Split the line into lines
                lines = split(line, ';'); // Split the line by semicolon
                if (wait > 0) { // If there is a wait time, skip the first line
                    wait--; // Decrease the wait time
                } else {
                    executeLine(lines[0]); // Execute the line of code
                }
            }
            
        }

        return 0;
    } else {
        // If arguments are provided, treat the first argument as a file name
        std::ifstream file(argv[1]); // Open the file for reading
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << argv[1] << std::endl;
            return 1; // Exit with an error code
        }

        std::string lxi;
        // Get a single line from the file
        while (std::getline(file, lxi)) {
            // Extract assets substring
            size_t executePos = findString(lxi, "execute:");
            std::string assetsSubstr = lxi.substr(11, executePos - 11);
            std::vector<std::string> assetPackageList = split(assetsSubstr, ';');

            // Extract code substring
            size_t codeLength = lxi.length() - (executePos + 8);
            std::string codeSubstr = lxi.substr(executePos + 8, codeLength);
            std::vector<std::string> code = split(codeSubstr, ';');

            for (const auto& singleLine : code) {
                executeLine(singleLine); // Execute each line of code
            }
        }
    }
}