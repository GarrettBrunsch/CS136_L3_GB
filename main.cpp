// Garrett, Brunsch
// Lab #3 Stack S25
// CS_136_14728SP25P
// Due 3/9/25

#include <iostream>
#include <string>
#include <fstream>
#include <limits>

using namespace std;

enum MenuChoice { PROCESS_EXPRESSION = 1, QUIT };

struct StackItem
{
    char symbolType = '\0';
    int position = -1;

    StackItem() {};
    StackItem(char type, int pos);
};

struct SymbolHandler
{
    string symbol = "";
    char matchKey = '\0';
    bool isOpening = false;

    SymbolHandler() {};
    SymbolHandler(const string& sym, char key, bool opening);

};

class Stack
{
private:
    static const int MAX_SIZE = 100;
    StackItem items[MAX_SIZE];
    int top = -1;

public:
    Stack();
    bool isEmpty() const { return (top == -1); }
    bool isFull() const { return (top == MAX_SIZE - 1); }
    bool push(const StackItem& item);
    bool pop(StackItem& item);
    bool peek(StackItem& item) const;
};

void initSymbolHandlers(SymbolHandler handlers[], int& count);
void analyzeExprSymbols(const string& expr, string& symbolsOnly, bool& isBalanced);

void displayResults(const string expressions[], const string symbolsArray[], const bool balancedArray[], int count);
void displayMenu();

void clearInputError(const string& errorMessage);
void processFile();

int findSymbolHandler(const SymbolHandler handlers[], int count, const string& symbol);
int processFoundSymbol(const string& expr, int pos, const string& symbol, SymbolHandler& handler,
    Stack& stack, bool& isBalanced);

bool detectSymbolAtPosition(const string& expr, int pos, const SymbolHandler symbolHandlers[],
    int handlerCount, string& foundSymbol, int& handlerIndex);
bool readFile(ifstream& inputFile, string expressions[], int& count, const int MAX_EXP_SIZE);

int main()
{
    int choice = -1;

    do
    {
        displayMenu();
        cin >> choice;

        switch (choice)
        {
        case PROCESS_EXPRESSION:
            processFile();
            break;

        case QUIT:
            cout << "Now exiting program...\n\n";
            break;

        default:
            clearInputError("Invalid entry. Please input either 1 or 2");
            break;
        }

    } while (choice != QUIT);

    return 0;

}

StackItem::StackItem(char type, int pos) : symbolType(type), position(pos) {}
SymbolHandler::SymbolHandler(const string& sym, char key, bool opening)
    : symbol(sym), matchKey(key), isOpening(opening) {}

void initSymbolHandlers(SymbolHandler handlers[], int& count)
{
    count = 0;

    // Pre-determined set of symbols being (x), {x}, [x], <!-- x -->, 'x', "x", and /*x*/
    // Format: input, key, isOpening
    handlers[count++] = SymbolHandler("(", '(', true);
    handlers[count++] = SymbolHandler("{", '{', true);
    handlers[count++] = SymbolHandler("[", '[', true);
    handlers[count++] = SymbolHandler("<!--", '<', true);
    handlers[count++] = SymbolHandler("/*", '/', true);
    handlers[count++] = SymbolHandler("\"", '"', true);
    handlers[count++] = SymbolHandler("'", '\'', true);

    handlers[count++] = SymbolHandler(")", '(', false);
    handlers[count++] = SymbolHandler("}", '{', false);
    handlers[count++] = SymbolHandler("]", '[', false);
    handlers[count++] = SymbolHandler("-->", '<', false);
    handlers[count++] = SymbolHandler("*/", '/', false);
}

int findSymbolHandler(const SymbolHandler handlers[], int count, const string& symbol)
{
    int index = -1;
    bool found = false;

    for (int i = 0; i < count && !found; i++)
    {
        if (handlers[i].symbol == symbol)
        {
            index = i;
            found = true;
        }
    }
    return index;
}

Stack::Stack()
{
    top = -1;
}

bool Stack::push(const StackItem& item)
{
    bool success = false;

    if (!isFull())
    {
        items[++top] = item;
        success = true;
    }
    return success;
}

bool Stack::pop(StackItem& item)
{
    bool success = false;

    if (!isEmpty())
    {
        item = items[top--];
        success = true;
    }
    return success;
}

bool Stack::peek(StackItem& item) const
{
    bool success = false;

    if (!isEmpty())
    {
        item = items[top];
        success = true;
    }
    return success;
}

void processFile()
{
    const int MAX_EXPRESSIONS_SIZE = 100; // Processess in chunks of X size to allow for "unlimited" entries without DMA
    const int MAX_EXPRESSION_LENGTH = 60;
    const string FILENAME = "expressions.txt";

    string expressions[MAX_EXPRESSIONS_SIZE] = {};
    string symbolsArray[MAX_EXPRESSIONS_SIZE] = {};
    bool balancedArray[MAX_EXPRESSIONS_SIZE] = {};
    int expressionCount = 0;
    int totalProcessed = 0;
    bool readSuccess = true;
    bool hasMoreData = true;

    cout << "Reading expressions from file: " << FILENAME << "\n";
    ifstream inputFile(FILENAME);

    if (inputFile.is_open())
    {
        while (hasMoreData && readSuccess)
        {
            expressionCount = 0;
            readSuccess = readFile(inputFile, expressions, expressionCount, MAX_EXPRESSIONS_SIZE);
            hasMoreData = (expressionCount > 0);

            if (expressionCount > 0)
            {
                for (int i = 0; i < expressionCount; i++)
                {
                    if (expressions[i].length() > MAX_EXPRESSION_LENGTH)
                    {
                        symbolsArray[i] = "ERROR: Entry must be < 60 chars";
                        balancedArray[i] = false;
                    }
                    else
                    {
                        analyzeExprSymbols(expressions[i], symbolsArray[i], balancedArray[i]);
                    }
                }
                displayResults(expressions, symbolsArray, balancedArray, expressionCount);
                totalProcessed += expressionCount;
            }
        }

        inputFile.close(); // Handled here instead of readFile so that data can be processed in pieces
        if (totalProcessed > 0)
        {
            cout << "Successfully processed " << totalProcessed << " expressions from file\n";
        }
    }
    else
    {
        cout << "Failed to read from file. Please check if '" << FILENAME << "' exists\n";
    }
}

bool readFile(ifstream& inputFile, string expressions[], int& count, const int MAX_EXP_SIZE)
{
    bool success = true;
    count = 0;
    string line = "";

    if (inputFile.is_open() && !inputFile.eof())
    {
        while (getline(inputFile, line) && count < MAX_EXP_SIZE)
        {
            if (!line.empty())
            {
                expressions[count++] = line;
            }
        }
    }
    else
    {
        success = false;
    }
    return success;
}

string getDisplayFormat(const string& symbol)
{
    return symbol + " ";
}

bool detectSymbolAtPosition(const string& expr, int pos, const SymbolHandler symbolHandlers[],
    int handlerCount, string& foundSymbol, int& handlerIndex)
{
    const int MAX_SYMBOL_LENGTH = 4; // Currently <!--
    const int MIN_SYMBOL_LENGTH = 1; // Currently /*
    bool symbolFound = false;

    for (int len = MAX_SYMBOL_LENGTH; len >= MIN_SYMBOL_LENGTH && !symbolFound; len--)
    {
        if (pos + len <= expr.length())
        {
            foundSymbol = expr.substr(pos, len);
            handlerIndex = findSymbolHandler(symbolHandlers, handlerCount, foundSymbol);

            if (handlerIndex != -1)
            {
                symbolFound = true;
            }
        }
    }

    return symbolFound;
}

int processFoundSymbol(const string& expr, int pos, const string& symbol, SymbolHandler& handler,
    Stack& stack, bool& isBalanced)
{
    // Special handling for quotes (they match themselves)
    if (symbol == "\"" || symbol == "'")
    {
        StackItem top;
        bool isClosingQuote = stack.peek(top) && top.symbolType == expr[pos];

        if (isClosingQuote)
        {
            // Exactly one character is permitted in single quotes according to class module
            if (expr[pos] == '\'' && (pos - top.position > 2))
            {
                isBalanced = false;
            }
            else
            {
                stack.pop(top);
            }
        }
        else
        {
            StackItem item(expr[pos], pos);
            stack.push(item);
        }
    }
    else if (handler.isOpening)
    {
        StackItem item(handler.matchKey, pos);
        stack.push(item);
    }
    else
    {
        StackItem top;
        bool hasMatchingOpening = stack.peek(top) && top.symbolType == handler.matchKey;

        if (!hasMatchingOpening)
        {
            isBalanced = false;
        }
        else
        {
            stack.pop(top);
        }
    }
    return pos + symbol.length();
}

void analyzeExprSymbols(const string& expr, string& symbolsOnly, bool& isBalanced)
{
    const int MAX_SYMBOLS = 12;
    SymbolHandler symbolHandlers[MAX_SYMBOLS];
    int handlerCount = 0;

    initSymbolHandlers(symbolHandlers, handlerCount);

    Stack stack;
    symbolsOnly = "";
    isBalanced = true;

    int i = 0;
    while (i < expr.length())
    {
        string foundSymbol = "";
        int handlerIndex = -1;

        if (detectSymbolAtPosition(expr, i, symbolHandlers, handlerCount, foundSymbol, handlerIndex))
        {
            symbolsOnly += getDisplayFormat(foundSymbol);
            i = processFoundSymbol(expr, i, foundSymbol, symbolHandlers[handlerIndex], stack, isBalanced);
        }
        else
        {
            i++;
        }
    }

    if (!stack.isEmpty())
    {
        isBalanced = false;
    }
}

void displayResults(const string expressions[], const string symbolsArray[], const bool balancedArray[], int count)
{
    const int exprColWidth = 22;
    const int symbolsColWidth = 25;

    cout << "\n-------------------------------------------------------------------\n"
        << "| Original Expression    | Symbols Only              | Status      |\n"
        << "-------------------------------------------------------------------\n";

    for (int i = 0; i < count; i++)
    {
        const string& expr = expressions[i];
        const string& symbolsOnly = symbolsArray[i];
        bool balanced = balancedArray[i];

        // Calculate how many rows needed for this expression
        int exprRows = (expr.length() + exprColWidth - 1) / exprColWidth;
        int symbolRows = (symbolsOnly.length() + symbolsColWidth - 1) / symbolsColWidth;
        int totalRows = max(exprRows, symbolRows);
        totalRows = max(totalRows, 1);

        for (int row = 0; row < totalRows; row++)
        {
            cout << "| ";

            // Original Expression column
            if (row < exprRows)
            {
                int startPos = row * exprColWidth;
                int len = min(exprColWidth, (int)expr.length() - startPos);
                cout << expr.substr(startPos, len) << string(exprColWidth - len, ' ');
            }
            else
            {
                cout << string(exprColWidth, ' ');
            }

            cout << " | ";

            // Symbols Only column
            if (row < symbolRows)
            {
                int startPos = row * symbolsColWidth;
                int len = min(symbolsColWidth, (int)symbolsOnly.length() - startPos);
                cout << symbolsOnly.substr(startPos, len) << string(symbolsColWidth - len, ' ');
            }
            else
            {
                cout << string(symbolsColWidth, ' ');
            }

            // Status column
            if (row == 0)
            {
                cout << " | " << (balanced ? "Well-formed " : "Imbalanced  ") << "|";
            }
            else
            {
                cout << " |             |";
            }

            cout << "\n";
        }

        if (i < count - 1)
        {
            cout << "|" << string(24, '-') << "|" << string(27, '-') << "|" << string(13, '-') << "|\n";
        }
    }

    cout << "-------------------------------------------------------------------\n";
}

void displayMenu()
{
    cout << "\n\n --- Main Menu ---\n"
        "1. Process expressions\n"
        "2. Quit\n"
        "Choice: ";
}

void clearInputError(const string& errorMessage)
{
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << errorMessage << "\n";
}

/*
 --- Main Menu ---
1. Process expressions
2. Quit
Choice: 1
Reading expressions from file: expressions.txt
Successfully read 43 expressions from file


    -------------------------------------------------------------------
    | Original Expression    | Symbols Only              | Status      |
    -------------------------------------------------------------------
    | #VALID TESTS:          |                           | Well-formed |
    |------------------------|---------------------------|-------------|
    | CASE #1: (balanced par | ( )                       | Well-formed |
    | entheses)              |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #2: {balanced cur | { }                       | Well-formed |
    | ly braces}             |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #3: [balanced squ | [ ]                       | Well-formed |
    | are brackets]          |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #4: <!--balanced  | <!-- -->                  | Well-formed |
    | HTML comments-->       |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #5: /balanced C-s |                           | Well-formed |
    | tyle comments/         |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #6: "balanced dou | " "                       | Well-formed |
    | ble quotes"            |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #7: 'a' (single q | ' ' ( )                   | Well-formed |
    | uote with one char)    |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #8: '' (empty sin | ' ' ( )                   | Well-formed |
    | gle quotes)            |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #9: {outer [inner | { [ ( ) ] }               | Well-formed |
    |  (complex nesting)]}   |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #10: ([{}]) (mixi | ( [ { } ] ) ( )           | Well-formed |
    | ng different symbols)  |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #11: "Text with ( | " ( ) { } " ( )           | Well-formed |
    | parens) and {braces}"  |                           |             |
    | (mixed content)        |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #12: <!-- "quoted | <!-- " " --> ( )          | Well-formed |
    | " text --> (quotes in  |                           |             |
    | comments)              |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #13: (){}[] (empt | ( ) { } [ ] ( )           | Well-formed |
    | y expressions with sym |                           |             |
    | bols)                  |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #14: No symbols a |                           | Well-formed |
    | t all in this line     |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #15: ()[]{}""''<! | ( ) [ ] { } " " ' ' <!--  | Well-formed |
    | ---->// (consecutive s | --> ( )                   |             |
    | ymbols)                |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #16: (one) {two}  | ( ) { } [ ] ( )           | Well-formed |
    | [three] (multiple matc |                           |             |
    | hes)                   |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #17: " spaced out | " " ( )                   | Well-formed |
    |  " (whitespace in quot |                           |             |
    | es)                    |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #18: (({{[[]]}})) | ( ( { { [ [ ] ] } } ) ) ( | Well-formed |
    |  (consecutive similar  |  )                        |             |
    | symbols)               |                           |             |
    |------------------------|---------------------------|-------------|
    | CASE #19: Test under 6 | ( )                       | Well-formed |
    | 0 chars (length is fin |                           |             |
    | e)                     |                           |             |
    |------------------------|---------------------------|-------------| */
    //  | CASE #20: <!-- Nested  | <!-- /* */ -- > ()        | Well-formed |
    //  | /* comments */ inside  |                           |             | 
    /*  | -- > (test)            |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #21: "outer 'a' q | " ' ' " ( )               | Well-formed |
        | uote" (nested quotes)  |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #22: (Deeper{nest | ( { [ ] } ) ( )           | Well-formed |
        | ed[content]}) (case)   |                           |             |
        |------------------------|---------------------------|-------------|
        | #INVALID TESTS:        |                           | Well-formed |
        |------------------------|---------------------------|-------------|
        | CASE #23: {["'('')]} ( | { [ " ' ( ' ' ) ] } ( )   | Imbalanced  |
        | unbalanced quotes)     |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #24: '\'' (single | ' ' ' ( )                 | Imbalanced  |
        |  quotes with unmatched |                           |             |
        |  quotes)               |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #25: "\"" (double | " " " ( )                 | Imbalanced  |
        |  quotes with unmatched |                           |             |
        |  quotes)               |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #26: (){}"'<!---- | ERROR: Entry must be < 60 | Imbalanced  |
        | >// (adjacent differen |  chars                    |             |
        | t unbalanced symbols)  |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #27: "'"'" (multi | " ' " ' " ( )             | Imbalanced  |
        | ple unmatched quotes)  |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #28: (imbalanced  | (                         | Imbalanced  |
        | parentheses            |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #29: {imbalanced  | {                         | Imbalanced  |
        | curly braces           |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #30: [imbalanced  | [                         | Imbalanced  |
        | brackets               |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #31: <!--imbalanc | <!--                      | Imbalanced  |
        | ed HTML comments       |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #32: /*imbalanced | /*                        | Imbalanced  |
        |  C-style comments      |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #33: "imbalanced  | "                         | Imbalanced  |
        | double quotes          |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #34: 'abc' (multi | ' ' ( )                   | Imbalanced  |
        | ple chars in single qu |                           |             |
        | otes)                  |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #35: {first [seco | { [ } ] ( )               | Imbalanced  |
        | nd} third] (improper n |                           |             |
        | esting)                |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #36: ([{< (mixing | ( [ { ( )                 | Imbalanced  |
        |  different unclosed sy |                           |             |
        | mbols)                 |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #37: " (untermina | " ( )                     | Imbalanced  |
        | ted quote at end)      |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #38: <!-- (unterm | <!-- ( )                  | Imbalanced  |
        | inated comment at end) |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #39: ([)] (improp | ( [ ) ] ( )               | Imbalanced  |
        | er order of closing sy |                           |             |
        | mbols)                 |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #40: ([{)]}" (mul | ( [ { ) ] } " ( )         | Imbalanced  |
        | tiple mismatches)      |                           |             |
        |------------------------|---------------------------|-------------|
        | CASE #41: Test exceedi | ERROR: Entry must be < 60 | Imbalanced  |
        | ng max length of 60 ch |  chars                    |             |
        | ars should trigger err |                           |             |
        | or message for being t |                           |             |
        | oo long for processing |                           |             |
        -------------------------------------------------------------------
        Successfully processed 43 expressions from file


     --- Main Menu ---
    1. Process expressions
    2. Quit
    Choice: g
    Invalid entry. Please double check the input and try again


     --- Main Menu ---
    1. Process expressions
    2. Quit
    Choice: 2
    Now exiting program...
    */
