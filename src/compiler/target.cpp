/***************************************************************************
 * Defines how the intermediate code is written to output files
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

#include "target.h"

using namespace backend;

/****************************
 * IntermediateFactory class
 ****************************/
IntermediateFactory* IntermediateFactory::instance;

IntermediateFactory* IntermediateFactory::Instance()
{
  if(!instance) {
    instance = new IntermediateFactory;
  }

  return instance;
}

/****************************
 * Writes target code to an
 * output file.
 ****************************/
void TargetEmitter::Emit()
{
#ifdef _DEBUG
  cout << "\n--------- Emitting Target Code ---------" << endl;
  program->Debug();
#endif

  if(as_lib) {
    // TODO: better error handling
    if(file_name.rfind(".obl") == string::npos) {
      cerr << "Error: Libraries must end in '.obl'" << endl;
      exit(1);
    }
  } 
  else {
    // TODO: better error handling
    if(file_name.rfind(".obe") == string::npos) {
      cerr << "Error: Executables must end in '.obe'" << endl;
      exit(1);
    }
  }
  
  ofstream* file_out = new ofstream(file_name.c_str(), ofstream::binary);
  if(file_out && file_out->is_open()) {
    program->Write(file_out, as_lib);
    file_out->close();
    cout << "Wrote target file: '" << file_name << "'" << endl;
  }
  else {
    cerr << "Unable to write file: '" << file_name << "'" << endl;
  }
  
  delete file_out;
  file_out = NULL;
}
