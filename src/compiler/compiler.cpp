/***************************************************************************
 * Starting point of the language compiler
 *
 * Copyright (c) 2008-2011, Randy Hollines
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

#include "compiler.h"

using namespace std;

#define SUCCESS 0
#define COMMAND_ERROR 1
#define PARSE_ERROR 2
#define CONTEXT_ERROR 3

/****************************
 * Starts the compilation
 * process.
 ****************************/
int Compile(map<const string, string> arguments, const string usage)
{
  // check source input
  map<const string, string>::iterator result = arguments.find("src");
  if(result == arguments.end()) {
    cerr << usage << endl << endl;
    return COMMAND_ERROR;
  }
  // check program output
  result = arguments.find("dest");
  if(result == arguments.end()) {
    cerr << usage << endl << endl;
    return COMMAND_ERROR;
  }
  // check program libraries path
  string libs_path = "lang.obl";
  result = arguments.find("lib");
  if(result != arguments.end()) {
    libs_path += "," + result->second;
  }
  // check for optimize flag
  string optimize;
  result = arguments.find("opt");
  if(result != arguments.end()) {
    optimize = result->second;
    if(optimize != "s0" && optimize != "s1" && optimize != "s2" && 
       optimize != "s3" && optimize != "s4") {
      cerr << usage << endl << endl;
      return COMMAND_ERROR;
    }
  }
  // check program libraries path
  string target;
  result = arguments.find("tar");
  if(result != arguments.end()) {
    target = result->second;
    if(target != "lib" && target != "exe") {
      cerr << usage << endl << endl;
      return COMMAND_ERROR;
    }
  }
  // check for debug flag
  bool is_debug = false;
  result = arguments.find("debug");
  if(result != arguments.end()) {
    is_debug = true;
  }
  
  // parse source code
  Parser parser(arguments["src"]);
  if(parser.Parse()) {
    bool is_lib = target == "lib";
    // analyze parse tree
    ParsedProgram* program = parser.GetProgram();
    ContextAnalyzer analyzer(program, libs_path, is_lib);
    if(analyzer.Analyze()) {
      // emit intermediate code
      IntermediateEmitter intermediate(program, is_lib, is_debug);
      intermediate.Translate();
      // intermediate optimizer
      ItermediateOptimizer optimizer(intermediate.GetProgram(), arguments["opt"]);
      optimizer.Optimize();
      // emit target code
      TargetEmitter target(optimizer.GetProgram(), is_lib, arguments["dest"]);;
      target.Emit();
      
      return SUCCESS;
    }
    else {
      return CONTEXT_ERROR;
    }
  }
  else {
    return PARSE_ERROR;
  }
}
