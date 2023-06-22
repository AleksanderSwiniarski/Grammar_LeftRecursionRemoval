#include <stdio.h>
#include <iostream>
#include "grammar.hpp"
#include <algorithm>

bool cmdOptExists(char **begin, char **end, const std::string &option)
{
    return std::find(begin, end, option) != end;
};

int main(int argc, char *argv[])
{
    Grammar processedGrammar;
    
    // Checking for descriptive flag
    if (cmdOptExists(argv, argv + argc, "-d") || cmdOptExists(argv, argv + argc, "--descriptive"))
    {
        processedGrammar.refDescriptiveMode() = true;
    }

    // Reading grammar files
    int numbOfGrammars = 0;
    int numbOfWrittenGrammars = 0;
    for (int i = 1; i < argc; i++)
    {
        int j = 1;
        while (processedGrammar.readGrammar(argv[i], j, numbOfGrammars))
        {
            if(processedGrammar.refDescriptiveMode()){
                std::cout << "Grammar read: " << std::endl;
                processedGrammar.displayGrammar();
            }            
            std::cout << "Error checking grammar nr." << numbOfGrammars << std::endl;
            if (processedGrammar.verifyGrammar())
            {
                if (processedGrammar.isLeftRecursion())
                {
                    std::cout << "Handling epsilon productions from grammar nr." << numbOfGrammars << std::endl;
                    processedGrammar.handleEpsilonProductions();
                    std::cout << "Handling cycles from grammar nr." << numbOfGrammars << std::endl;
                    processedGrammar.handleCycles();
                    if (processedGrammar.isLeftRecursion())
                    {
                        std::cout << "Removing left recursion from grammar nr." << numbOfGrammars << std::endl;
                        processedGrammar.removeLeftRecursion();
                    }
                }
                std::cout << "Writing grammar nr." << numbOfGrammars << " to the file" << std::endl;
                processedGrammar.writeGrammarToFile(numbOfWrittenGrammars, numbOfGrammars);
            }
            processedGrammar.clear();
            j++;
        };
    }

    return 0;
}