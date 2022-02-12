#include <iostream>
#include <algorithm>
#include <bitset>
#include <string>
#include <vector>
#include <limits>
#include "scanner.h"
using namespace std;

class MipsError {
    string message = "ERROR: ";

  public:
    MipsError(string error) { message += error; }
    string getMessage() { return message; }
};

bool isValidId(string id, vector<string> validIDs) {
    for (auto & validId : validIDs) {
        if (id == validId) {
            return true;
        }
    }
    return false;
}

bool checkREG(string reg) {
    int regNum = stoi(reg);
    if (regNum >= 0 && regNum <= 31) return true;
    return false;
}

// also modifies arguments and makes it store registers and i's
bool isValidInstr(vector<Token> instruction, int idx, int checkType, vector<int> &arguments) {
    string lexeme;
    string reg;

    if (checkType == 1) { // one register (jr, jalr, lis, mflo and mfhi)

        Token::Kind kind = instruction[idx + 1].getKind();
        lexeme = instruction[idx + 1].getLexeme();
        reg = lexeme.substr(1, lexeme.length()); // remove $
        if (instruction.size() - idx != 2) return false; // not one arguments
        if (kind != Token::Kind::REG) return false;
        if (! checkREG(reg)) return false; // reg not in 0 - 31
        arguments.push_back(stoi(reg));

    } else if (checkType == 2) { // two registers

        if (instruction.size() - idx != 4) return false; // not 5 arguments
        if (instruction[idx + 1].getKind() != Token::Kind::REG) return false;
        lexeme = instruction[idx + 1].getLexeme();
        reg = lexeme.substr(1, lexeme.length());
        if (! checkREG(reg)) return false;
        arguments.push_back(stoi(reg));

        if (instruction[idx + 3].getKind() != Token::Kind::REG) return false;
        lexeme = instruction[idx + 3].getLexeme();
        reg = lexeme.substr(1, lexeme.length());
        if (! checkREG(reg)) return false;
        arguments.push_back(stoi(reg));

        if (instruction[idx + 2].getKind() != Token::Kind::COMMA) return false;

    } else if (checkType == 3) { // three registers

        if (instruction.size() - idx != 6) return false; // not 5 arguments
        if (instruction[idx + 1].getKind() != Token::Kind::REG) return false;
        lexeme = instruction[idx + 1].getLexeme();
        reg = lexeme.substr(1, lexeme.length());
        if (! checkREG(reg)) return false;
        arguments.push_back(stoi(reg));

        if (instruction[idx + 3].getKind() != Token::Kind::REG) return false;
        lexeme = instruction[idx + 3].getLexeme();
        reg = lexeme.substr(1, lexeme.length());
        if (! checkREG(reg)) return false;
        arguments.push_back(stoi(reg));

        if (instruction[idx + 5].getKind() != Token::Kind::REG) return false;
        lexeme = instruction[idx + 5].getLexeme();
        reg = lexeme.substr(1, lexeme.length());
        if (! checkREG(reg)) return false;
        arguments.push_back(stoi(reg));

        if (instruction[idx + 2].getKind() != Token::Kind::COMMA) return false;
        if (instruction[idx + 4].getKind() != Token::Kind::COMMA) return false;

    } else if (checkType == 4) {

        if (instruction.size() - idx != 6) return false; // not 5 arguments
        if (instruction[idx + 1].getKind() != Token::Kind::REG) return false;
        lexeme = instruction[idx + 1].getLexeme();
        reg = lexeme.substr(1, lexeme.length());
        if (! checkREG(reg)) return false;
        arguments.push_back(stoi(reg));

        if (instruction[idx + 3].getKind() != Token::Kind::REG) return false;
        lexeme = instruction[idx + 3].getLexeme();
        reg = lexeme.substr(1, lexeme.length());
        if (! checkREG(reg)) return false;
        arguments.push_back(stoi(reg));

        if (instruction[idx + 5].getKind() != Token::Kind::INT &&
            instruction[idx + 5].getKind() != Token::Kind::HEXINT &&
            instruction[idx + 5].getKind() != Token::Kind::ID) return false;
        Token word = instruction[idx + 5];
        Token::Kind wordKind = word.getKind();
        string wordLexeme = word.getLexeme();
        int64_t bin = word.toNumber();
        if (wordKind == Token::Kind::INT) {
            if (bin > 32767 || bin < -32768) return false;
        } else if (wordKind == Token::Kind::HEXINT) {
            if (bin > 65535 || bin < 0) return false;
        }
        arguments.push_back(bin);

        if (instruction[idx + 2].getKind() != Token::Kind::COMMA) return false;
        if (instruction[idx + 4].getKind() != Token::Kind::COMMA) return false;

    } else if (checkType == 5) {

        if (instruction.size() - idx != 7) return false; // not 5 arguments
        if (instruction[idx + 1].getKind() != Token::Kind::REG) return false;
        lexeme = instruction[idx + 1].getLexeme();
        reg = lexeme.substr(1, lexeme.length());
        if (! checkREG(reg)) return false;
        arguments.push_back(stoi(reg));

        if (instruction[idx + 3].getKind() != Token::Kind::INT &&
            instruction[idx + 3].getKind() != Token::Kind::HEXINT) return false;
        Token word = instruction[idx + 3];
        Token::Kind wordKind = word.getKind();
        string wordLexeme = word.getLexeme();
        int64_t bin = word.toNumber();
        if (wordKind == Token::Kind::INT) {
            if (bin > 32767 || bin < -32768) return false;
        } else if (wordKind == Token::Kind::HEXINT) {
            if (bin > 65535 || bin < 0) return false;
        }
        arguments.push_back(bin);

        if (instruction[idx + 5].getKind() != Token::Kind::REG) return false;
        lexeme = instruction[idx + 5].getLexeme();
        reg = lexeme.substr(1, lexeme.length());
        if (! checkREG(reg)) return false;
        arguments.push_back(stoi(reg));

        if (instruction[idx + 2].getKind() != Token::Kind::COMMA) return false;
        if (instruction[idx + 4].getKind() != Token::Kind::LPAREN) return false;
        if (instruction[idx + 6].getKind() != Token::Kind::RPAREN) return false;
    }
    return true;
}

bool containsLabel(pair<string,int> newLabel, vector<pair<string,int>> labels) {
    for (auto & label : labels) {
        if (newLabel.first == label.first) {
            return true;
        }
    }
    return false;
}

int findLabelLine(string labelToFind, vector<pair<string,int>> labels) {
    for (auto & label : labels) {
        if (labelToFind == label.first) {
            return label.second;
        }
    }
    return -1;   
}

int main() {
    string line;
    int instr = 0;
    int count = 0;
    // labels stores the label name and the line number of each label
    vector<pair<string,int>> labels;
    // instructions stores instr(int), line number(int), instr name(first string) and possible label name (second string)
    vector<pair<pair<int,int>, pair<string,string>>> instructions;
    vector<string> validIDs({"add", "sub", "mult", "multu", "div", "divu", "mfhi", "mflo", "lis", "lw", "sw", "slt", "sltu", "beq", "bne", "jr", "jalr"});

    try {
        while (getline(std::cin, line)) {

            bool lineCounted = false; // check if a line has had instruction already
            bool isLabel = true;

            std::vector<Token> tokenLine = scan(line);

            pair<pair<int,int>,pair<string,string>> instruction;

            for(size_t i = 0; i < tokenLine.size(); ++i) {
                Token::Kind kind = tokenLine[i].getKind();
                string lexeme = tokenLine[i].getLexeme();
                instruction.first.second = count; // stores line number

                if (kind == Token::Kind::ID) {
                    
                    isLabel = false;
                    bool idValid = isValidId(lexeme, validIDs);
                    if (! idValid) throw(MipsError("Invalid ID"));
                    instruction.second.first = lexeme;
                    instruction.second.second = "";
                    vector<int> arguments;

                    int checktype = 0;
                    if (lexeme == "jr" || lexeme == "jalr" || lexeme == "lis" || lexeme == "mfhi" || lexeme == "mflo") { // one register
                        checktype = 1;
                    } else if (lexeme == "mult" || lexeme == "multu" || lexeme == "div" || lexeme == "divu") { // two registers
                        checktype = 2;
                    } else if (lexeme == "add" || lexeme == "sub" || lexeme == "slt" || lexeme == "sltu") { // three registers
                        checktype = 3;
                    } else if (lexeme == "beq" || lexeme == "bne") { // two registers and one i
                        checktype = 4;
                    } else if (lexeme == "sw" || lexeme == "lw") { // two registers, one i and two parenthesis
                        checktype = 5;
                    }

                    if (! isValidInstr(tokenLine, i, checktype, arguments)) throw(MipsError("Invalid Instruction Format"));


                    if (lexeme == "jr") {
                        instr = (arguments[0] << 21) | (8 << 0) ;
                    } else if (lexeme == "jalr") {
                        instr = (arguments[0] << 21) | (9 << 0) ;
                    } else if (lexeme == "add") {
                        instr = (arguments[1] << 21) | (arguments[2] << 16) | (arguments[0] << 11) | (2 << 4) ;
                    } else if (lexeme == "sub") {
                        instr = (arguments[1] << 21) | (arguments[2] << 16) | (arguments[0] << 11) | (2 << 4) | (2 << 0) ;
                    } else if (lexeme == "slt") {
                        instr = (arguments[1] << 21) | (arguments[2] << 16) | (arguments[0] << 11) | (2 << 4) | (10 << 0) ;
                    } else if (lexeme == "sltu") {
                        instr = (arguments[1] << 21) | (arguments[2] << 16) | (arguments[0] << 11) | (2 << 4) | (11 << 0) ;
                    } else if (lexeme == "beq") {

                        if (tokenLine[i + 5].getKind() == Token::Kind::ID) { // i is label name
                            instruction.second.second = tokenLine[i + 5].getLexeme();
                            instr = (1 << 28) | (arguments[0] << 21) | (arguments[1] << 16) ;
                        } else {
                            instr = (1 << 28) | (arguments[0] << 21) | (arguments[1] << 16) | (arguments[2] & 0xFFFF) ;
                        }

                    } else if (lexeme == "bne") {

                        if (tokenLine[i + 5].getKind() == Token::Kind::ID) { // i is label name
                            instruction.second.second = tokenLine[i + 5].getLexeme();
                            instr = (1 << 28) | (4 << 24) | (arguments[0] << 21) | (arguments[1] << 16) ;
                        } else {
                            instr = (1 << 28) | (4 << 24) | (arguments[0] << 21) | (arguments[1] << 16) | (arguments[2] & 0xFFFF) ;
                        }

                    } else if (lexeme == "lis") {
                        instr = (arguments[0] << 11) | 20;
                    } else if (lexeme == "mfhi") {
                        instr = (arguments[0] << 11) | 16;
                    } else if (lexeme == "mflo") {
                        instr = (arguments[0] << 11) | 18;
                    } else if (lexeme == "mult") {
                        instr = (arguments[0] << 21) | (arguments[1] << 16) | 24 ;
                    } else if (lexeme == "multu") {
                        instr = (arguments[0] << 21) | (arguments[1] << 16) | 25 ;
                    } else if (lexeme == "div") {
                        instr = (arguments[0] << 21) | (arguments[1] << 16) | 26 ;
                    } else if (lexeme == "divu") {
                        instr = (arguments[0] << 21) | (arguments[1] << 16) | 27 ;
                    } else if (lexeme == "lw") {
                        instr = (35 << 26) | (arguments[2] << 21) | (arguments[0] << 16) | (arguments[1] & 0xFFFF) ;
                    } else if (lexeme == "sw") {
                        instr = (43 << 26) | (arguments[2] << 21) | (arguments[0] << 16) | (arguments[1] & 0xFFFF) ;
                    }

                    instruction.first.first = instr; 

                } else if (kind == Token::Kind::WORD) {

                    isLabel = false;
                    if (tokenLine.size() - i != 2) throw(MipsError(".word should have one and only one argument"));
                    instruction.second.first = lexeme;
                    instruction.second.second = ""; // initialize label name to be empty
                    Token word = tokenLine[i + 1];
                    int64_t uint_max = 4294967295;
                    int64_t long_min = -2147483648;
                    Token::Kind wordKind = word.getKind();
                    string wordLexeme = word.getLexeme();
                    int64_t bin = word.toNumber();
                    ++i;
                    if (wordKind == Token::Kind::INT) {
                        if (bin > uint_max || bin < long_min) throw(MipsError("INT Out of Bound"));
                    } else if (wordKind == Token::Kind::HEXINT) {
                        if (bin > uint_max || bin < 0) throw(MipsError("HEXINT Out of Bound"));
                    } else if (wordKind == Token::Kind::ID) {
                        instruction.second.second = wordLexeme;
                    } else {
                        throw(MipsError(".word has invalid instruction"));
                    }
                    instruction.first.first = bin;


                } else if (kind == Token::Kind::LABEL) {

                    instruction.first.first = -1; // -1 tells assemblr to skip
                    lexeme = lexeme.substr(0, lexeme.length() - 1); // remove colon
                    pair<string,int> label = make_pair(lexeme, count);
                    if (containsLabel(label, labels)) throw(MipsError("Label Already Existed"));
                    labels.push_back(label);

                } else {
                    break;
                }

                if (kind != Token::Kind::LABEL && ! lineCounted) { // keep track of line number for labels
                    count += 4;
                    lineCounted = true;
                }

            }

            if (lineCounted && ! isLabel) instructions.push_back(instruction);

        }

    } catch (ScanningFailure &f) {
        cerr << f.what() << endl;
        return 1;
    } catch (MipsError &e) {
        cerr << e.getMessage() << endl;
    }

    for (auto & instruction : instructions) {
        // add label if needed
        int instr = instruction.first.first;
        int instrLineNum = instruction.first.second;
        string instrName  = instruction.second.first;
        string label = instruction.second.second;

        if (instrName == ".word" && label != "") {
            int lineNum = findLabelLine(label, labels);
            if (lineNum == -1) cerr << MipsError(".word Label Not Found").getMessage() << endl;
            instr = lineNum;
        } else if ((instrName == "beq" || instrName == "bne") && (label != "")) {
            int lineNum = findLabelLine(label, labels);
            if (lineNum == -1) cerr << MipsError("beq/bne Label Not Found").getMessage() << endl;
            // if label is valid, find number of lines to jump up or down
            int jump = (lineNum - instrLineNum) / 4;
            if (jump > 32767 || jump < -32768) cerr << MipsError("beq/bne jump out of range").getMessage() << endl;;
            instr = instr | (jump & 0xFFFF) ;
        }
        // send to output
        unsigned char c = instr >> 24;
        cout << c;
        c = instr >> 16;
        cout << c;
        c = instr >> 8;
        cout << c;
        c = instr;
        cout << c;
    }
    
    return 0;
}