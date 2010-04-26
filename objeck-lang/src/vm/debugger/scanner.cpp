/**************************************************************************
 * Debugger scanner.
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

#include "scanner.h"

#define EOB '\0'

/****************************
* Scanner constructor
****************************/
Scanner::Scanner(const string &line)
{
  // create tokens
  for(int i = 0; i < LOOK_AHEAD; i++) {
    tokens[i] = new Token;
  }
  // load identifiers into map
  LoadKeywords();
  ReadLine(line);
}

/****************************
* Scanner destructor
****************************/
Scanner::~Scanner()
{
  // delete buffer
  if(buffer) {
    delete[] buffer;
    buffer = NULL;
  }
  
  for(int i = 0; i < LOOK_AHEAD; i++) {
    Token* temp = tokens[i];
    delete temp;
    temp = NULL;
  }
}

/****************************
* Loads language keywords
****************************/
void Scanner::LoadKeywords()
{
  ident_map["load"] = TOKEN_LOAD_ID;
  ident_map["l"] = TOKEN_LOAD_ID;
  ident_map["quit"] = TOKEN_QUIT_ID;
  ident_map["q"] = TOKEN_QUIT_ID;  
  ident_map["break"] = TOKEN_BREAK_ID;
  ident_map["b"] = TOKEN_BREAK_ID;
  ident_map["print"] = TOKEN_PRINT_ID;
  ident_map["p"] = TOKEN_PRINT_ID;
  ident_map["info"] = TOKEN_INFO_ID;
  ident_map["i"] = TOKEN_INFO_ID;
  ident_map["frame"] = TOKEN_FRAME_ID;
  ident_map["f"] = TOKEN_FRAME_ID;
}

/****************************
* Processes language
* identifies
****************************/
void Scanner::CheckIdentifier(int index)
{
  // copy string
  const int length = end_pos - start_pos;
  string ident(buffer, start_pos, length);
  // check string
  TokenType ident_type = ident_map[ident];
  switch(ident_type) {
  case TOKEN_LOAD_ID:
  case TOKEN_QUIT_ID:
  case TOKEN_BREAK_ID:
  case TOKEN_PRINT_ID:
  case TOKEN_INFO_ID:
  case TOKEN_FRAME_ID:
    tokens[index]->SetType(ident_type);
    break;
  default:
    tokens[index]->SetType(TOKEN_IDENT);
    tokens[index]->SetIdentifier(ident);
    break;
  }
}

/****************************
* Reads a source input file
****************************/
void Scanner::ReadLine(const string &line)
{
  buffer_pos = 0;
  buffer = new char[line.size() + 1];
  const char* src = line.c_str();
  strcpy(buffer, src);
  buffer_size = line.size() + 1;
#ifdef _DEBUG
  cout << "---------- Source ---------" << endl;
  cout << buffer << endl;
  cout << "---------------------------" << endl;
#endif
}

/****************************
* Processes the next token
****************************/
void Scanner::NextToken()
{
  if(buffer_pos == 0) {
    NextChar();
    for(int i = 0; i < LOOK_AHEAD; i++) {
      ParseToken(i);
    }
  } else {
    int i = 1;
    for(; i < LOOK_AHEAD; i++) {
      tokens[i - 1]->Copy(tokens[i]);
    }
    ParseToken(i - 1);
  }
}

/****************************
* Gets the current token
****************************/
Token* Scanner::GetToken(int index)
{
  if(index < LOOK_AHEAD) {
    return tokens[index];
  }
  
  return NULL;
}

/****************************
* Gets the next character.
* Note, EOB is returned at
* end of a stream
****************************/
void Scanner::NextChar()
{
  if(buffer_pos < buffer_size) {
    // current character
    cur_char = buffer[buffer_pos++];
    // next character
    if(buffer_pos < buffer_size) {
      nxt_char = buffer[buffer_pos];
      // next next character
      if(buffer_pos + 1 < buffer_size) {
        nxt_nxt_char = buffer[buffer_pos + 1];
      }
      // end of file
      else {
        nxt_nxt_char = EOB;
      }
    }
    // end of file
    else {
      nxt_char = EOB;
    }
  }
  // end of file
  else {
    cur_char = EOB;
  }
}

/****************************
* Processes white space
****************************/
void Scanner::Whitespace()
{
  while(WHITE_SPACE && cur_char != EOB) {

    NextChar();
  }
}

/****************************
* Parses a token
****************************/
void Scanner::ParseToken(int index)
{
  // unable to load buffer
  if(!buffer) {
    tokens[index]->SetType(TOKEN_NO_INPUT);
    return;
  }
  // ignore white space
  Whitespace();
  // ignore comments
  while(cur_char == COMMENT && cur_char != EOB) {
    NextChar();
    // extended comment
    if(cur_char == EXTENDED_COMMENT) {
      NextChar();
      while(!(cur_char == EXTENDED_COMMENT && nxt_char == COMMENT) && cur_char != EOB) {
        NextChar();
      }
      NextChar();
      NextChar();
    }
    // line comment
    else {
      while(cur_char != '\n' && cur_char != EOB) {
        NextChar();
      }
    }
    Whitespace();
  }
  // character string
  if(cur_char == '\"') {
    NextChar();
    // mark
    start_pos = buffer_pos - 1;
    while(cur_char != '\"' && cur_char != EOB) {
      if(cur_char == '\\') {
        NextChar();
        switch(cur_char) {
        case '"':
          break;

        case '\\':
          break;

        case 'n':
          break;

        case 'r':
          break;

        case 't':
          break;

        case '0':
          break;

        default:
          tokens[index]->SetType(TOKEN_UNKNOWN);
          NextChar();
          break;
        }
      }
      NextChar();
    }
    // mark
    end_pos = buffer_pos - 1;
    // check string
    NextChar();
    CheckString(index);
    return;
  }
  // character
  else if(cur_char == '\'') {
    NextChar();
    // escape or hex/unicode encoding
    if(cur_char == '\\') {
      NextChar();
      // read unicode string
      if(cur_char == 'u') {
        NextChar();
        start_pos = buffer_pos - 1;
        while(isdigit(cur_char) || (cur_char >= 'a' && cur_char <= 'f') ||
              (cur_char >= 'A' && cur_char <= 'F')) {
          NextChar();
        }
        end_pos = buffer_pos - 1;
        ParseUnicodeChar(index);
        if(cur_char != '\'') {
          tokens[index]->SetType(TOKEN_UNKNOWN);
        }
        NextChar();
        return;
      }
      // escape
      else if(nxt_char == '\'') {
        switch(cur_char) {
        case 'n':
          tokens[index]->SetType(TOKEN_CHAR_LIT);
          tokens[index]->SetCharLit('\n');
          NextChar();
          NextChar();
          return;
        case 'r':
          tokens[index]->SetType(TOKEN_CHAR_LIT);
          tokens[index]->SetCharLit('\r');
          NextChar();
          NextChar();
          return;
        case 't':
          tokens[index]->SetType(TOKEN_CHAR_LIT);
          tokens[index]->SetCharLit('\t');
          NextChar();
          NextChar();
          return;
        case '\\':
          tokens[index]->SetType(TOKEN_CHAR_LIT);
          tokens[index]->SetCharLit('\\');
          NextChar();
          NextChar();
          return;
        case '0':
          tokens[index]->SetType(TOKEN_CHAR_LIT);
          tokens[index]->SetCharLit('\0');
          NextChar();
          NextChar();
          return;
        }
      }
      // error
      else {
        tokens[index]->SetType(TOKEN_UNKNOWN);
        NextChar();
        return;
      }
    } else {
      // error
      if(nxt_char != '\'') {
        tokens[index]->SetType(TOKEN_UNKNOWN);
        NextChar();
        return;
      } else {
        tokens[index]->SetType(TOKEN_CHAR_LIT);
        tokens[index]->SetCharLit(cur_char);
        NextChar();
        NextChar();
        return;
      }
    }
  }
  // identifier
  else if(isalpha(cur_char) || cur_char == '@') {
    // mark
    start_pos = buffer_pos - 1;
    
    while((isalpha(cur_char) || isdigit(cur_char) || cur_char == '_' || 
	   cur_char == '@' || cur_char == '.') && cur_char != EOB) {
      NextChar();
    }
    // mark
    end_pos = buffer_pos - 1;
    // check identifier
    CheckIdentifier(index);
    return;
  }
  // number
  else if(isdigit(cur_char) || (cur_char == '.' && isdigit(nxt_char))) {
    bool is_double = false;
    int hex_state = 0;
    // mark
    start_pos = buffer_pos - 1;
    
    // test hex state
    if(cur_char == '0') {
      hex_state = 1;
    }
    while(isdigit(cur_char) || (cur_char == '.' && isdigit(nxt_char)) || cur_char == 'x' ||
          (cur_char >= 'a' && cur_char <= 'f') ||
          (cur_char >= 'A' && cur_char <= 'F')) {
      // decimal double
      if(cur_char == '.') {
        // error
        if(is_double) {
          tokens[index]->SetType(TOKEN_UNKNOWN);
          NextChar();
          break;
        }
        is_double = true;
      }
      // hex integer
      if(cur_char == 'x') {
	if(hex_state == 1) {
	  hex_state = 2;
	}
	else {
	  hex_state = 1;
	}
      }
      else {
	hex_state = 0;
      }
      // next character
      NextChar();
    }
    // mark
    end_pos = buffer_pos - 1;
    if(is_double) {
      ParseDouble(index);
    } 
    else if(hex_state == 2) {
      ParseInteger(index, 16);
    }
    else if(hex_state) {
      tokens[index]->SetType(TOKEN_UNKNOWN);
    }
    else {
      ParseInteger(index);
    }
    return;
  }
  // other
  else {
    switch(cur_char) {
    case ':':
      if(nxt_char == '=') {
        NextChar();
        tokens[index]->SetType(TOKEN_ASSIGN);
        NextChar();
      } 
      else {
        tokens[index]->SetType(TOKEN_COLON);
        NextChar();
      }
      break;

    case '-':
      if(nxt_char == '>') {
        NextChar();
        tokens[index]->SetType(TOKEN_ASSESSOR);
        NextChar();
      } 
      else {
        tokens[index]->SetType(TOKEN_SUB);
        NextChar();
      }
      break;

    case '{':
      tokens[index]->SetType(TOKEN_OPEN_BRACE);
      NextChar();
      break;

    case '.':
      tokens[index]->SetType(TOKEN_PERIOD);
      NextChar();
      break;

    case '}':
      tokens[index]->SetType(TOKEN_CLOSED_BRACE);
      NextChar();
      break;

    case '[':
      tokens[index]->SetType(TOKEN_OPEN_BRACKET);
      NextChar();
      break;

    case ']':
      tokens[index]->SetType(TOKEN_CLOSED_BRACKET);
      NextChar();
      break;

    case '(':
      tokens[index]->SetType(TOKEN_OPEN_PAREN);
      NextChar();
      break;

    case ')':
      tokens[index]->SetType(TOKEN_CLOSED_PAREN);
      NextChar();
      break;

    case ',':
      tokens[index]->SetType(TOKEN_COMMA);
      NextChar();
      break;

    case ';':
      tokens[index]->SetType(TOKEN_SEMI_COLON);
      NextChar();
      break;

    case '&':
      tokens[index]->SetType(TOKEN_AND);
      NextChar();
      break;

    case '|':
      tokens[index]->SetType(TOKEN_OR);
      NextChar();
      break;

    case '=':
      tokens[index]->SetType(TOKEN_EQL);
      NextChar();
      break;

    case '<':
      if(nxt_char == '>') {
        NextChar();
        tokens[index]->SetType(TOKEN_NEQL);
        NextChar();
      } else if(nxt_char == '=') {
        NextChar();
        tokens[index]->SetType(TOKEN_LEQL);
        NextChar();
      } else {
        tokens[index]->SetType(TOKEN_LES);
        NextChar();
      }
      break;

    case '>':
      if(nxt_char == '=') {
        NextChar();
        tokens[index]->SetType(TOKEN_GEQL);
        NextChar();
      } else {
        tokens[index]->SetType(TOKEN_GTR);
        NextChar();
      }
      break;

    case '+':
      tokens[index]->SetType(TOKEN_ADD);
      NextChar();
      break;

    case '*':
      tokens[index]->SetType(TOKEN_MUL);
      NextChar();
      break;

    case '/':
      tokens[index]->SetType(TOKEN_DIV);
      NextChar();
      break;

    case '%':
      tokens[index]->SetType(TOKEN_MOD);
      NextChar();
      break;

    case EOB:
      tokens[index]->SetType(TOKEN_END_OF_STREAM);
      break;

    default:
      ProcessWarning();
      tokens[index]->SetType(TOKEN_UNKNOWN);
      NextChar();
      break;
    }
    return;
  }
}
