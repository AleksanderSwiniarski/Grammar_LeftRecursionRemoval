#include "grammar.hpp"
#include <bits/stdc++.h>
#include <string>
#include <fstream>
#include <map>
#include <algorithm>

void skipLine(std::fstream &file)
{
    char character;
    while ((file.get(character)) && !(character == '\r' || character == '\n')){}
}

Grammar::Grammar()
{
    this->descriptiveMode = false;
    this->epsilon = "e";
    this->mappedGrammarTokens["//"] = Comment;
    this->mappedGrammarTokens["#G"] = GrammarDefinition;
    this->mappedGrammarTokens["#Eps"] = EpsilonDefinition;
    this->mappedGrammarTokens["#NT"] = NonTerminal;
    this->mappedGrammarTokens["#T"] = Terminal;
    this->mappedGrammarTokens["#P"] = Production;
    this->mappedGrammarTokens["|"] = ProductionSeparator;
    this->mappedGrammarTokens["#Seq"] = SequenceDefinition;
};

Grammar::~Grammar(){};

bool &Grammar::refDescriptiveMode()
{
    return descriptiveMode;
}

bool Grammar::readGrammar(char *fileName, int nthGrammar, int &numbOfGrammars)
{
    grammarToken currentToken = NotDefined;
    std::string currentNonTerminal;
    std::fstream grammarFile;
    std::string word;
    std::vector<std::string> newProduction;
    std::streampos startPosition;

    grammarFile.open(fileName, std::ios::in | std::ios::binary);
    if (!grammarFile)
    {
        return false;
    }

    // Looking for nthGrammar in the file
    while (nthGrammar != 0 && grammarFile >> word)
    {
        if (grammarFile.eof())
            return false;
        if (word == "#G")
        {
            nthGrammar--;
        }
    }
    if (nthGrammar != 0)
    {
        return false;
    }

    std::cout << "Scanning grammar nr." << ++numbOfGrammars << std::endl;

    // Scanning words till the eof or #G is detected
    while (grammarFile >> word)
    {
        // Taking respective action if word is defined gramamr token
        switch (mappedGrammarTokens[word])
        {
        case Comment: // Ignoring rest of the line
            skipLine(grammarFile);
            continue;
        case GrammarDefinition:
            if (sequence.empty())
            {
                this->sequence = nonTerminals;
            }
            if (descriptiveMode)
            {
                std::cout << "\tNew order of nonterminals detected: ";
                for (auto symbolIT = sequence.begin(); symbolIT != sequence.end(); symbolIT++)
                {
                    std::cout << *symbolIT << " ";
                }
                std::cout << std::endl;
            }
            return true;
        case EpsilonDefinition:
            if ((grammarFile >> word) && (mappedGrammarTokens.find(word) == mappedGrammarTokens.end()))
            {
                if (descriptiveMode)
                {
                    std::cout << "\tNew definition of epsilon: " << word << std::endl;
                }
                epsilon = word;
            }
            currentToken = NotDefined;
            continue;
        case NonTerminal:
            currentToken = NonTerminal;
            continue;
        case Terminal:
            currentToken = Terminal;
            continue;
        case Production:
            currentToken = Production;
            grammarFile >> word;
            currentNonTerminal = word;
            continue;
        case SequenceDefinition:
            sequence.clear();
            currentToken = SequenceDefinition;
            continue;
        case ProductionSeparator:
            if (descriptiveMode)
            {
                std::cout << "\tNew production for " << currentNonTerminal << " detected: ";
                for (auto symbolsIT = newProduction.begin(); symbolsIT != newProduction.end(); symbolsIT++)
                {
                    std::cout << *symbolsIT << " ";
                }
                std::cout << std::endl;
            }
            productions[currentNonTerminal].push_back(newProduction);
            newProduction.clear();
            continue;
        default: // Word is non defined grammar token
            mappedGrammarTokens.erase(word);
            switch (currentToken)
            {
            case NonTerminal:
                if (descriptiveMode)
                {
                    std::cout << "\tNew nonterminal detected: " << word << std::endl;
                }
                nonTerminals.push_back(word);
                break;
            case Terminal:
                if (descriptiveMode)
                {
                    std::cout << "\tNew terminal detected: " << word << std::endl;
                }
                terminals.push_back(word);
                break;
            case Production:
                newProduction.push_back(word);
                // Check if next word is defined token
                startPosition = grammarFile.tellg();
                grammarFile >> word;
                if ((mappedGrammarTokens.find(word) != mappedGrammarTokens.end() && word != "|") || grammarFile.eof())
                {
                    if (descriptiveMode)
                    {
                        std::cout << "\tNew production for " << currentNonTerminal << " detected: ";
                        for (auto symbolsIT = newProduction.begin(); symbolsIT != newProduction.end(); symbolsIT++)
                        {
                            std::cout << *symbolsIT << " ";
                        }
                        std::cout << std::endl;
                    }
                    productions[currentNonTerminal].push_back(newProduction);
                    newProduction.clear();
                }
                grammarFile.seekg(startPosition);
                break;
            case SequenceDefinition:
                sequence.push_back(word);
                break;
            default:
                break;
            }
            break;
        }
    }
    if (sequence.empty())
    {
        this->sequence = nonTerminals;
    }
    if (descriptiveMode)
    {
        std::cout << "\tNew order of nonterminals detected: ";
        for (auto symbolIT = sequence.begin(); symbolIT != sequence.end(); symbolIT++)
        {
            std::cout << *symbolIT << " ";
        }
        std::cout << std::endl;
    }
    return true;
};

bool Grammar::verifyGrammar()
{
    bool verifyFlag = true;
    while (verifyFlag) // Verification is repeated till it passes all of verifications steps successfully
                       // If step is failed, verification will strat again from the top
    {
        // Error nr.1: Grammar does not have any terminal, nonterminal or productions defined
        if (!areElementsDefined())
        {
            return false;
        }

        // Error nr.2: Word defined as a terminal and nonterminal
        if (isDoubleWordDefinition())
        {
            continue;
        }

        // Error nr.3: Production consists of undefined terminals or nonterminals
        if (isUndefinedSymbolInProduction())
        {
            continue;
        }

        // Error nr.4: Repetition of productions for the same nonterminals
        if (areRepeatedProductions())
        {
            continue;
        }

        // Error nr.5: Left side of production is not defined nonterminal
        if (areLeftNonTerminals())
        {
            continue;
        }

        // Error nr.6: Nonterminal does not have defined any productions
        if (nonTerminalsHaveProductions())
        {
            continue;
        }

        // Error nr.7: Epsilon is defined like one of terminals or nonterminals
        if (!isEpsilonUnique())
        {
            return false;
        }

        // Error nr.8: Suggested sequence of nonterminals by user does not contain all nonterminals or consists of not defined ones
        if (isSequenceNotProper())
        {
            continue;
        }

        verifyFlag = false;
    }
    if (descriptiveMode)
    {
        std::cout << "Grammar after verification: " << std::endl;
        displayGrammar();
    }
    return true;
}

bool Grammar::areElementsDefined()
{
    if (terminals.empty() || nonTerminals.empty() || productions.empty())
    {
        bool commaFlag = false;
        std::cout << "\t{CRITICAL ERROR} Grammar does not have any";
        if (terminals.empty())
        {
            std::cout << " terminals";
            commaFlag = true;
        };
        if (nonTerminals.empty())
        {
            if (commaFlag)
            {
                std::cout << ",";
            }
            std::cout << " nonterminals";
            commaFlag = true;
        };
        if (productions.empty())
        {
            if (commaFlag)
            {
                std::cout << ",";
            }
            std::cout << " productions";
        };
        std::cout << " defined! It is being rejected!" << std::endl;
        return false;
    };
    return true;
}

bool Grammar::isDoubleWordDefinition()
{
    bool isDoubleDefenition = false;
    std::vector<std::string>::iterator nonTermIterator;
    for (auto termIterator = terminals.begin(); termIterator != terminals.end(); termIterator++)
    {
        if ((nonTermIterator = std::find(nonTerminals.begin(), nonTerminals.end(), *termIterator)) != nonTerminals.end())
        {
            isDoubleDefenition = true;
            nonTermIterator = nonTerminals.erase(nonTermIterator);
            std::cout << "\t{Error} " << *termIterator << " has definition of terminal and nonterminal. " << *termIterator << " will be removed from the nonterminals definition of the grammar." << std::endl;
        }
    }
    return isDoubleDefenition;
}

bool Grammar::isUndefinedSymbolInProduction()
{
    bool isUndefinedSymbolInProduction = false;
    for (auto mapIT = productions.begin(); mapIT != productions.end(); mapIT++)
    {
        for (auto prodsIT = (mapIT->second).begin(); prodsIT != (mapIT->second).end(); prodsIT++)
        {
            for (auto symbolsIT = (*prodsIT).begin(); symbolsIT != (*prodsIT).end(); symbolsIT++)
            {
                if (!isDesfinedAsTermOrNonTerm(*symbolsIT) && *symbolsIT != epsilon)
                {
                    std::cout << "\t{Error} Production: ";
                    for (auto errorSymbolsIT = (*prodsIT).begin(); errorSymbolsIT != (*prodsIT).end(); errorSymbolsIT++)
                    {
                        std::cout << *errorSymbolsIT << " ";
                    }
                    std::cout << "consists of undefined word: " << *symbolsIT << ". Production will be removed" << std::endl;
                    prodsIT = productions[mapIT->first].erase(prodsIT);
                    prodsIT--;
                    isUndefinedSymbolInProduction = true;
                    break;
                }
            }
        }
    }
    return isUndefinedSymbolInProduction;
}

bool Grammar::areRepeatedProductions()
{
    bool isRepetition = false;
    for (auto mapIT = productions.begin(); mapIT != productions.end(); mapIT++)
    {
        for (auto prodsIT = (mapIT->second).begin(); prodsIT != (mapIT->second).end(); prodsIT++)
        {
            auto compIT = prodsIT;
            compIT++;
            for (compIT; compIT != (mapIT->second).end(); compIT++)
            {
                // Checking if next productions are the same as the currently analysed one
                if (*compIT == *prodsIT)
                {
                    std::cout << "\t{Error} ";
                    for (auto symbolsIT = (*prodsIT).begin(); symbolsIT != (*prodsIT).end(); symbolsIT++)
                    {
                        std::cout << *symbolsIT << " ";
                    }
                    std::cout << "production repeated. Duplicate will be removed." << std::endl;
                    compIT = (mapIT->second).erase(compIT);
                    compIT = prodsIT;
                    compIT++;
                    isRepetition = true;
                }
            }
        }
    }
    return isRepetition;
}

bool Grammar::areLeftNonTerminals()
{
    bool failedTest = false;
    for (auto mapIT = productions.begin(); mapIT != productions.end(); mapIT++)
    {
        // Checking if obtained key is not defined as nonterminal
        if (!isDefinedAsNonTerminal(mapIT->first))
        {
            std::cout << "\t{Error} Left side of production: " << mapIT->first << " is not a single defined nonterminal. ";
            failedTest = true;
            // Checking if it conatins any already defined nonterminal. If so its production are added to exisitng one
            for (auto nonTermIT = nonTerminals.begin(); nonTermIT != nonTerminals.end(); nonTermIT++)
            {
                if (mapIT->first.find(*nonTermIT) != std::string::npos)
                {
                    for (auto prodsIT = (mapIT->second).begin(); prodsIT != (mapIT->second).end(); prodsIT++)
                    {
                        productions[*nonTermIT].push_back(*prodsIT);
                    }
                    std::cout << "Within was detected nonterminal: " << *nonTermIT << ". Productions are moved to detected nonterminal. ";
                    break;
                }
            }
            std::cout << "Productions for " << mapIT->first << " nonterminal are being removed!" << std::endl;
            mapIT = productions.erase(mapIT);
            mapIT--;
        }
    }
    return failedTest;
}

bool Grammar::nonTerminalsHaveProductions()
{
    bool hasProductionFlag = false;
    for (auto nonTermIT = nonTerminals.begin(); nonTermIT != nonTerminals.end(); nonTermIT++)
    {
        bool hasProductions = false;
        auto productionsToRemove = productions.end();
        for (auto mapIT = productions.begin(); mapIT != productions.end(); mapIT++)
        {
            // Checking if map has a key defined as a searched nonterminal
            if (mapIT->first == *nonTermIT)
            {
                // Checking if for found nonterminal are any productions defined
                if (!mapIT->second.empty())
                {
                    hasProductions = true;
                    productionsToRemove = mapIT;
                }
                break;
            }
        }
        if (!hasProductions)
        {
            hasProductionFlag = true;
            std::cout << "\t{Error} " << *nonTermIT << " does not have defined productions. Nonterminal will be removed" << std::endl;
            nonTermIT = nonTerminals.erase(nonTermIT);
            nonTermIT = nonTerminals.begin();
            if(productionsToRemove != productions.end()){productions.erase(productionsToRemove);}            
        }
    }
    return hasProductionFlag;
}

bool Grammar::isEpsilonUnique()
{
    bool isEpsilonUnique = true;
    if (isDesfinedAsTermOrNonTerm(epsilon))
    {
        isEpsilonUnique = false;
        std::cout << "\t{Error} " << epsilon << " is also defined as terminal or nonterminal. ";
        // Checking if epsilon can be resotred to default value
        if (!isDesfinedAsTermOrNonTerm("e"))
        {
            epsilon = "e";
            std::cout << "Epsilon has been restored to the default value!" << std::endl;
            isEpsilonUnique = true;
        }
        // If epsilon wasn't restored to main value it is bein generated
        if (!isEpsilonUnique)
        {
            for (int i = 97; i <= 122; i++)
            {
                for (int j = 97; j <= 122; j++)
                {
                    char ch1 = static_cast<char>(i);
                    char ch2 = static_cast<char>(j);
                    std::string generated;
                    generated.push_back(ch1);
                    generated.push_back(ch2);
                    if (!isDesfinedAsTermOrNonTerm(generated))
                    {
                        std::cout << "Epsilon has been restored to the: " << generated << " value!" << std::endl;
                        epsilon = generated;
                        isEpsilonUnique = true;
                    }
                    if (isEpsilonUnique)
                    {
                        break;
                    }
                }
                if (isEpsilonUnique)
                {
                    break;
                }
            }
        }
    }
    if (!isEpsilonUnique)
    {
        std::cout << "Unable to restore epislon value. Terminated processing of grammar!" << std::endl;
    }
    return isEpsilonUnique;
}

bool Grammar::isSequenceNotProper()
{
    bool isSeqNotDefProperly = false;

    if (sequence.size() != nonTerminals.size())
    {
        isSeqNotDefProperly = true;
    }
    else
    {
        for (auto seqIT = sequence.begin(); seqIT != sequence.end(); seqIT++)
        {
            if (!isDefinedAsNonTerminal(*seqIT))
            {
                isSeqNotDefProperly = true;
            }
        }
    }
    if (isSeqNotDefProperly)
    {
        std::cout << "\t{Error} Sequence: ";
        for (auto nonTermIT = sequence.begin(); nonTermIT != sequence.end(); nonTermIT++)
        {
            std::cout << *nonTermIT << " ";
        }
        std::cout << "is improperly defined. Using default sequence of nonterminals" << std::endl;
        sequence = nonTerminals;
    }

    return isSeqNotDefProperly;
}

bool Grammar::isLeftRecursion()
{
    bool leftRecursionFlag = false;
    for (auto mapIT = productions.begin(); mapIT != productions.end(); mapIT++)
    {
        for (auto prodsIT = (mapIT->second).begin(); prodsIT != (mapIT->second).end(); prodsIT++)
        {
            if (mapIT->first == *(*prodsIT).begin())
            {
                leftRecursionFlag = true;
                break;
            }
        }
        if (leftRecursionFlag)
        {
            break;
        }
    }
    return leftRecursionFlag;
}

void Grammar::handleEpsilonProductions()
{
    // Find nullable nonterminals
    bool newNullFoundFlag = false;
    std::vector<std::string> nullables;
    do
    {
        newNullFoundFlag = findNullables(nullables);
    } while (newNullFoundFlag);

    if (nullables.empty())
    {
        return;
    }

    // Remove epsilon Productions if we have nullables
    std::map<std::string, std::vector<std::vector<std::string>>> updatedProductions;
    for (auto mapIT = productions.begin(); mapIT != productions.end(); mapIT++)
    {
        for (auto prodsIT = (mapIT->second).begin(); prodsIT != (mapIT->second).end(); prodsIT++)
        {
            // Checking if production is single epsilon
            if ((*prodsIT).size() == 1 && *(*prodsIT).begin() == epsilon)
            {
                continue; // If so we are skipping it
            }
            // Creating subests out of production where nullable can be skipped or not
            std::vector<std::vector<std::string>> subsets;
            std::vector<std::string> currentSubset;
            createSubsets(currentSubset, subsets, (*prodsIT).begin(), prodsIT, nullables);
            for (const auto &production : subsets)
            {
                if (!production.empty())
                {
                    updatedProductions[mapIT->first].push_back(production);
                }
            }
        }
    }
    this->productions = updatedProductions;

    // Remove nonterminals which are not found in productions
    for (auto nonTermIT = nonTerminals.begin(); nonTermIT != nonTerminals.end(); nonTermIT++)
    {
        bool found = false;
        for (auto mapIT = productions.begin(); mapIT != productions.end(); mapIT++)
        {
            if (*nonTermIT == mapIT->first)
            {
                found = true;
            }
        }
        if (!found)
        {
            nonTerminals.erase(nonTermIT);
            nonTermIT--;
        }
    }
    if (descriptiveMode)
    {
        std::cout << "Grammar after handling of epsilon productions: " << std::endl;
        displayGrammar();
    }
};

bool Grammar::findNullables(std::vector<std::string> &nullables)
{
    bool newNullFoundFlag = false;
    for (auto mapIT = productions.begin(); mapIT != productions.end(); mapIT++)
    {
        for (auto prodsIT = (mapIT->second).begin(); prodsIT != (mapIT->second).end(); prodsIT++)
        {
            // Checking if nonterminal is nullable by simple epsilon production
            if ((*prodsIT).size() == 1 && *(*prodsIT).begin() == epsilon)
            {
                if (std::find(nullables.begin(), nullables.end(), mapIT->first) == nullables.end())
                {
                    nullables.push_back(mapIT->first);
                    newNullFoundFlag = true;
                }
                break;
            }
            else
            {
                bool nonNullableFound = false;
                for (auto symbolsIT = (*prodsIT).begin(); symbolsIT != (*prodsIT).end(); symbolsIT++)
                {
                    // Checking if there is nonNullable symbol in production
                    if (std::find(nullables.begin(), nullables.end(), *symbolsIT) == nullables.end())
                    {
                        nonNullableFound = true;
                    }
                }
                // If not the nonTerminal is nullable
                if (!nonNullableFound)
                {
                    if (std::find(nullables.begin(), nullables.end(), mapIT->first) == nullables.end())
                    {
                        nullables.push_back(mapIT->first);
                        newNullFoundFlag = true;
                    }
                }
            }
        }
    }
    return newNullFoundFlag;
}

void Grammar::handleCycles()
{
    for (auto mapIT = productions.begin(); mapIT != productions.end(); mapIT++)
    {
        for (auto prodsIT = (mapIT->second).begin(); prodsIT != (mapIT->second).end(); prodsIT++)
        {
            if ((*prodsIT).size() == 1 && isDefinedAsNonTerminal(*(*prodsIT).begin()))
            {
                // Single Nonterminal in production detected
                std::string unitNonTerminal(*(*prodsIT).begin());
                // Remvoing unit production
                (mapIT->second).erase(prodsIT);
                prodsIT--;
                // Adding productions of detected nonterminal
                if (unitNonTerminal != mapIT->first)
                {
                    for (auto unitProdsIT = productions[unitNonTerminal].begin(); unitProdsIT != productions[unitNonTerminal].end(); unitProdsIT++)
                    {
                        // Not adding if it would create duplicate production or it is next unit production
                        bool eligible = true;
                        for (prodsIT = (mapIT->second).begin(); prodsIT != (mapIT->second).end(); prodsIT++)
                        {
                            if (*prodsIT == *unitProdsIT || ((*unitProdsIT).size() == 1 && isDefinedAsNonTerminal(*(*unitProdsIT).begin())))
                            {
                                eligible = false;
                                break;
                            }
                        }
                        if (eligible)
                        {
                            (mapIT->second).push_back(*unitProdsIT);
                        }
                    }
                }
                prodsIT = (mapIT->second).begin();
            }
        }
    }
    if (descriptiveMode)
    {
        std::cout << "Grammar after handling cycles: " << std::endl;
        displayGrammar();
    }
};

void Grammar::removeLeftRecursion()
{
    for (int i = 0; i < sequence.size(); i++)
    {
        for (int j = 0; j < i; j++)
        {
            bool modified = false;
            for (auto prodsIT = productions[sequence[i]].begin(); prodsIT != productions[sequence[i]].end(); prodsIT++)
            {
                if (modified)
                {
                    prodsIT = productions[sequence[i]].begin();
                    modified = false;
                }
                if ((*(*prodsIT).begin()) == sequence[j])
                {
                    // Extracting symbols after Nonterminal
                    std::vector<std::string> gamma;
                    gamma = (*prodsIT);
                    gamma.erase(gamma.begin());
                    // Removing production
                    productions[sequence[i]].erase(prodsIT);
                    prodsIT--;
                    // Adding modified productions
                    for (auto seqProds = productions[sequence[j]].begin(); seqProds != productions[sequence[j]].end(); seqProds++)
                    {
                        std::vector<std::string> modifiedProduction;
                        if (!((*seqProds).size() == 1 && (*(*seqProds).begin()) == epsilon))
                        {
                            modifiedProduction = (*seqProds);
                        }
                        for (auto seqSymbol = gamma.begin(); seqSymbol != gamma.end(); seqSymbol++)
                        {
                            modifiedProduction.push_back(*seqSymbol);
                        }
                        productions[sequence[i]].push_back(modifiedProduction);
                    }
                    prodsIT = productions[sequence[i]].begin();
                    modified = true;
                }
            }
        }
        // Remove immediate recursion
        std::vector<std::vector<std::string>> alfas;
        std::vector<std::vector<std::string>> betas;
        // Find alfa and beta strings
        for (auto prodsIT = productions[sequence[i]].begin(); prodsIT != productions[sequence[i]].end(); prodsIT++)
        {
            if ((*(*prodsIT).begin()) == sequence[i])
            {
                std::vector<std::string> alfa;
                for (auto symbolsIT = ++(*prodsIT).begin(); symbolsIT != (*prodsIT).end(); symbolsIT++)
                {
                    alfa.push_back(*symbolsIT);
                }
                alfas.push_back(alfa);
            }
            else
            {
                betas.push_back(*prodsIT);
            }
        }
        // If immediate recursion is present, productions will be replaced
        if (!alfas.empty())
        {
            if (!betas.empty())
            {
                for (auto betaIT = betas.begin(); betaIT != betas.end(); betaIT++)
                {
                    (*betaIT).push_back(sequence[i] + "'");
                    productions[sequence[i]] = betas;
                }
                for (auto alfaIT = alfas.begin(); alfaIT != alfas.end(); alfaIT++)
                {
                    (*alfaIT).push_back(sequence[i] + "'");
                }
                alfas.push_back({epsilon});
                productions[sequence[i] + "'"] = alfas;
                nonTerminals.push_back(sequence[i] + "'");
            }
            else
            {
                for (auto alfaIT = alfas.begin(); alfaIT != alfas.end(); alfaIT++)
                {
                    (*alfaIT).push_back(sequence[i]);
                }
                alfas.push_back({epsilon});
                productions[sequence[i]] = alfas;
            }
        }
    }
    if (descriptiveMode)
    {
        std::cout << "Grammar after left recursion removal: " << std::endl;
        displayGrammar();
    }
}

bool Grammar::writeGrammarToFile(int& nthWrittenGrammar, int nthGrammar)
{
    std::fstream grammarFile;
    std::string fileName = "modified.txt";

    if (nthWrittenGrammar == 0)
    {
        grammarFile.open(fileName, std::ios::out | std::ios::trunc);
    }
    else
    {
        grammarFile.open(fileName, std::ios::out | std::ios::app);
    }
    if (!grammarFile)
    {
        std::cout << "Error: Cannot open output file!" << std::endl;
        return false;
    }
    nthWrittenGrammar++;

    // Beginning of grammar definition
    grammarFile << "Grammar " << std::to_string(nthGrammar) << ": " << std::endl;
    // Nonterminals of grammar
    grammarFile << "Nonterminals: ";
    for (auto nonTermIT = nonTerminals.begin(); nonTermIT != nonTerminals.end(); nonTermIT++)
    {
        grammarFile << *nonTermIT << " ";
    }
    grammarFile << std::endl;
    // Terminals of grammar
    grammarFile << "Terminals: ";
    for (auto termIT = terminals.begin(); termIT != terminals.end(); termIT++)
    {
        grammarFile << *termIT << " ";
    }
    grammarFile << std::endl;
    // Productions of grammar
    grammarFile << "Productions: " << std::endl;
    int i = 0;
    for (auto mapIT = productions.begin(); mapIT != productions.end(); mapIT++)
    {
        grammarFile << "\t" << std::to_string(++i) << ". " << mapIT->first << " -> ";
        for (auto prodsIT = (mapIT->second).begin(); prodsIT != (mapIT->second).end(); prodsIT++)
        {
            for (auto symbolsIT = (*prodsIT).begin(); symbolsIT != (*prodsIT).end(); symbolsIT++)
            {
                grammarFile << *symbolsIT << " ";
            }
            if (++prodsIT != (mapIT->second).end())
            {
                grammarFile << "| ";
            }
            prodsIT--;
        }
        grammarFile << std::endl;
    }
    grammarFile << std::endl;

    return true;
}

bool Grammar::isDesfinedAsTermOrNonTerm(std::string word)
{
    return isDefinedAsTerminal(word) || isDefinedAsNonTerminal(word);
}

bool Grammar::isDefinedAsTerminal(std::string word)
{
    return std::find(terminals.begin(), terminals.end(), word) != terminals.end();
}

bool Grammar::isDefinedAsNonTerminal(std::string word)
{
    return std::find(nonTerminals.begin(), nonTerminals.end(), word) != nonTerminals.end();
}

void Grammar::createSubsets(std::vector<std::string> currentSubset, std::vector<std::vector<std::string>> &subsets,
                            std::vector<std::string>::iterator symbolIT, std::vector<std::vector<std::string>>::iterator prodsIT,
                            std::vector<std::string> nullables)
{
    if (symbolIT != (*prodsIT).end())
    {
        // Cheking if symbol is nullable
        if (std::find(nullables.begin(), nullables.end(), *symbolIT) != nullables.end())
        {
            // If so we are addding it and not
            createSubsets(currentSubset, subsets, ++symbolIT, prodsIT, nullables); // Not adding
            --symbolIT;
            currentSubset.push_back(*symbolIT);
            createSubsets(currentSubset, subsets, ++symbolIT, prodsIT, nullables); // Adding
        }
        else
        {
            // If not we have to add it
            currentSubset.push_back(*symbolIT);
            createSubsets(currentSubset, subsets, ++symbolIT, prodsIT, nullables);
        }
    }
    else
    {
        // We reached the end of production
        subsets.push_back(currentSubset);
        return;
    }
}

void Grammar::clear()
{
    this->terminals.clear();
    this->nonTerminals.clear();
    this->productions.clear();
    this->sequence.clear();
    this->epsilon = "e";
}

void Grammar::displayGrammar()
{
    std::cout << "Nonterminals: ";
    for (auto nonTermIT = nonTerminals.begin(); nonTermIT != nonTerminals.end(); nonTermIT++)
    {
        std::cout << *nonTermIT << " ";
    }
    std::cout << std::endl;
    std::cout << "Terminals: ";
    for (auto termsIT = terminals.begin(); termsIT != terminals.end(); termsIT++)
    {
        std::cout << *termsIT << " ";
    }
    std::cout << std::endl;
    std::cout << "Productions: " << std::endl;
    for (auto mapIT = productions.begin(); mapIT != productions.end(); mapIT++)
    {
        std::cout << "\t" << mapIT->first << " -> ";
        for (auto prodsIT = (mapIT->second).begin(); prodsIT != (mapIT->second).end(); prodsIT++)
        {
            for (auto symbolsIT = (*prodsIT).begin(); symbolsIT != (*prodsIT).end(); symbolsIT++)
            {
                std::cout << *symbolsIT << " ";
            }
            if (++prodsIT != (mapIT->second).end())
            {
                std::cout << "| ";
            }
            prodsIT--;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
