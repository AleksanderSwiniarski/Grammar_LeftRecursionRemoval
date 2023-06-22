# Grammar_LeftRecursionRemoval

Program reads CFG grammars provided by the user in the files and removes the left recursion if it has one and fulfils conditions of the algorithm.

Assumptions:
  -Grammars provided in the input file are context free grammars (CFG)
  -Grammars provided in the input file will be written according to the specific format
  -Grammar may contain errors which will be handled by the program accordingly
  -Grammar does not have to contain left recursion or be proper candidate for algorithm as it will be analysed under this aspects
  -Grammar’s terminal and nonterminals may be words of various lengths. They cannot be the same as the grammar tokens (e.g.:#G) or comment symbol (//).
  -Program will treat “e” by default as an epsilon symbol “ε” but user may change its definition
  -Program for the removal of left recursion algorithm will use by default the order of nonterminals that corresponds to the order in which they were defined in the file but user may define its own sequence

Detailed description of the program is provided in the Program_Documentation.pdf file
