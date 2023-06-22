#ifndef GRAMMAR
#define GRAMMAR

#include <vector>
#include <map>
#include <string>

enum grammarToken
{
    NotDefined,
    Comment,
    GrammarDefinition,
    EpsilonDefinition,
    NonTerminal,
    Terminal,
    Production,
    ProductionSeparator,
    SequenceDefinition,
    NotDefinedEnd
};

class Grammar
{

    std::vector<std::string> terminals;                                         // Terminals of a grammar
    std::vector<std::string> nonTerminals;                                      // Nonterminals of a grammar
    /* nonTerminals vector can be avoided as
    the same sequence of data is repeated in the
    productions map (key values). In the current 
    state we have to take into consideration changes 
    about nonterminals in both of data structures.
    Although the use nonterminal vector simplifies 
    the verification of the grammar.
    */
    std::map<std::string, std::vector<std::vector<std::string>>> productions;   // Productions of a grammar which are mapped to their respective nonterminal
    std::vector<std::string> sequence;                                          // Sequence of nonterminals used in removal of left recursion
    std::string epsilon;                                                        // Epsilon symbol of a grammar (DEFAULT: e)
    std::map<std::string, grammarToken> mappedGrammarTokens;                    // Mapped strings (Grammar tokens) to enums
    bool descriptiveMode;                                                       // Boolean informing if grammar should descrive all of its actions

public:
    Grammar();                                      // Class Constructor
    ~Grammar();                                     // Class Destructor

    bool readGrammar(char *, int, int &);           // Method reading single grammar definition from the file

    bool writeGrammarToFile(int&, int);             // Method wrtiing processed grammar to the file

    // Methods for analysing read grammar
    bool verifyGrammar();                           // Checking if read grammar is proper CFG grammar

    bool isLeftRecursion();                         // Checking if read gramamr has left recursion

    void handleEpsilonProductions();                // Checking if read grammar has epsilon productions. If so they will be removed

    void handleCycles();                            // Checking if read grammar has cycles. If so they will be removed
    /*The removal of unit productions is already 
    handled by the algorith which removes the left 
    recursion from the grammar. Although still
    can be used as it will shorten the operation
    on grammar if only left recursion apparent
    in the grammar were unit productions.*/

    void removeLeftRecursion();                     // Removing left recursion from the read grammar

    void clear();                                   // Removing all elements of a grammar from the object or reseting them to default values

    bool& refDescriptiveMode();                     // Reference method to boolean variable of grammar object

    void displayGrammar();                          // Display grammar in terminal using the same format how it is displayed in the file
    
private:

    // Methods used for verification of grammars
    bool areElementsDefined();                      // Checks if grammar has every element defined

    bool isDoubleWordDefinition();                  // Checks if any word is defined as terminal and nonterminal

    bool isUndefinedSymbolInProduction();           // Checks if any production has undefined word

    bool areRepeatedProductions();                  // Checks if any of nonterminals have repeating productions

    bool areLeftNonTerminals();                     // Checks if left side of production is defined and single nonterminal

    bool nonTerminalsHaveProductions();             // Checks if every nonterminal has defined productions for it

    bool isEpsilonUnique();                         // Checks if word was defined as epsilon and terminal/nonterminal

    bool isDefinedAsTerminal(std::string);          // Checks if word is defined as terminal

    bool isDefinedAsNonTerminal(std::string);       // Checks if word is defined as nonterminal

    bool isDesfinedAsTermOrNonTerm(std::string);    // Checks if word is defined as terminal or nonterminal

    bool isSequenceNotProper();                     // Checks if sequence consists of all defined nonterminals

    bool findNullables(std::vector<std::string> &); // Find symbols that are nullable in the grammar

    void createSubsets(std::vector<std::string>, std::vector<std::vector<std::string>> &,
                       std::vector<std::string>::iterator, std::vector<std::vector<std::string>>::iterator,
                       std::vector<std::string>);   // Creates subsets of productions where nullable symbol can be eliminated or not

};

#endif // GRAMMAR