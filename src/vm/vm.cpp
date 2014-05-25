/***************************************************************************
 * Starting point for the VM.
 *
 * Copyright (c) 2008-2013, Randy Hollines
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
 * - Neither the name of the Objeck Team nor the names of its
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

#include "vm.h"

#define SUCCESS 0
#define USAGE_ERROR -1

// common execution point for all platforms
int Execute(const int argc, const char* argv[])
{
  if(argc > 1) {
    srand((unsigned int)time(NULL)); rand(); // calling rand() once improves random number generation
    wchar_t** commands = ProcessCommandLine(argc, argv);
    Loader loader(argc, commands);
    loader.Load();

    // ignore web applications
    if(loader.IsWeb()) {
      cerr << L"Web applications must be executed in a FCGI environment." << endl;
      exit(1);
    }

    // execute
    long* op_stack = new long[CALC_STACK_SIZE];
    long* stack_pos = new long;
    (*stack_pos) = 0;
    
#ifdef _TIMING
    clock_t start = clock();
#endif
    // start the interpreter...
    Runtime::StackInterpreter* intpr = new Runtime::StackInterpreter(Loader::GetProgram());
    Runtime::StackInterpreter::AddThread(intpr);
    intpr->Execute(op_stack, stack_pos, 0, loader.GetProgram()->GetInitializationMethod(), NULL, false);
    
#ifdef _DEBUG
    wcout << L"# final stack: pos=" << (*stack_pos) << L" #" << endl;
    if((*stack_pos) > 0) {
      for(int i = 0; i < (*stack_pos); i++) {
        wcout << L"dump: value=" << (void*)(*stack_pos) << endl;
      } 
    }
#endif
    
    // clean up
    delete[] op_stack;
    op_stack = NULL;
    
    delete stack_pos;
    stack_pos = NULL;
    
    Runtime::StackInterpreter::RemoveThread(intpr);
    delete intpr;
    intpr = NULL;
    
    Runtime::StackInterpreter::HaltAll();

#ifdef _SANITIZE
    wcout << L"# final stack: pos=" << (*stack_pos) << L" #" << endl;
    if((*stack_pos) > 0) {
      for(int i = 0; i < (*stack_pos); i++) {
        wcout << L"dump: value=" << (void*)(*stack_pos) << endl;
      } 
    }
    
		Runtime::StackInterpreter::Clear();
    MemoryManager::Clear();    
#endif
    
#ifdef _TIMING    
    clock_t end = clock();
    wcout << L"---------------------------" << endl;
    wcout << L"CPU Time: " << (double)(end - start) / CLOCKS_PER_SEC
          << L" second(s)." << endl;
#endif

    CleanUpCommandLine(argc, commands);
    return SUCCESS;
  } 
  else {
    return USAGE_ERROR;
  }
}
