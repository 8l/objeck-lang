/***************************************************************************
 * Debugger parser.
 *
 * Copyright (c) 2010 Randy Hollines
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 * - Neither the name of the StackVM Team nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#ifndef __PARSER_H__
#define __PARSER_H__

#include "scanner.h"
#include "tree.h"

using namespace frontend;

#define SECOND_INDEX 1
#define THIRD_INDEX 2

/****************************
 * Parsers source files.
 ****************************/
class Parser {
  string current_file_name;
  Scanner* scanner;
  ParsedCommand* input;
  map<TokenType, string> error_msgs;
  vector<string> errors;
  
  inline void NextToken() {
    scanner->NextToken();
  }

  inline bool Match(TokenType type, int index = 0) {
    return scanner->GetToken(index)->GetType() == type;
  }

  inline TokenType GetToken(int index = 0) {
    return scanner->GetToken(index)->GetType();
  }

  void Show(const string &msg, int depth) {
    for(int i = 0; i < depth; i++) {
      cout << "  ";
    }
    cout << msg << endl;
  }
  
  inline string ToString(int v) {
    ostringstream str;
    str << v;
    return str.str();
  }

  // error processing
  void LoadErrorCodes();
  void ProcessError(const TokenType type);
  void ProcessError(const string &msg);
  bool CheckErrors();

  // parsing operations
  void ParseLine(const string& file_name);
  Command* ParseStatement(int depth);
  Command* ParseBreak(int depth);
  Command* ParsePrint(int depth);
  Command* ParseType(int depth);
  Command* ParseStack(int depth);
  Command* ParseFrame(int depth);
  ExpressionList* ParseIndices(int depth);
  Expression* ParseExpression(int depth);
  Expression* ParseLogic(int depth);
  Expression* ParseMathLogic(int depth);
  Expression* ParseTerm(int depth);
  Expression* ParseFactor(int depth);
  Expression* ParseSimpleExpression(int depth);
  Reference* ParseReference(const string &ident, int depth);
  void ParseReference(Reference* reference, int depth);
  
 public:
  Parser(const string &cf) {
    current_file_name = cf;
    input = new ParsedCommand;
    LoadErrorCodes();
  }

  ~Parser() {
  }

  bool Parse(const string &line);
  
  ParsedCommand* GetCommand() {
    return input;
  }
};

#endif
