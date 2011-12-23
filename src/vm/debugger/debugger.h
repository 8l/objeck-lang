/***************************************************************************
 * Runtime debugger
 *
 * Copyright (c) 2010-2012 Randy Hollines
 * All rights reserved.
 *
 * Reistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *tree.o scanner.o parser.o test.o
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

#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include "../common.h"
#include "../loader.h"
#include "../interpreter.h"
#include "tree.h"
#include "parser.h"
#include <iomanip>
#ifdef _WIN32
#include "windows.h"
#else
#include <dirent.h>
#endif

using namespace std;

namespace Runtime {
  class StackInterpreter;

  typedef struct _UserBreak {
    int line_num;
    string file_name;
  } UserBreak;

  /********************************
   * Source file
   ********************************/
  class SourceFile {
    string file_name;
    vector<string> lines;
    int cur_line_num;

  public:
    SourceFile(const string &fn, int l) {
      file_name = fn;
      cur_line_num = l;

      ifstream file_in (fn.c_str());
      while(file_in.good()) {
	string line;
	getline(file_in, line);
	lines.push_back(line);
      }
      file_in.close();
    }

    ~SourceFile() {
    }

    bool IsLoaded() {
      return lines.size() > 0;
    }

    bool Print(int start) {
      const int window = 5;
      int end = start + window * 2;
      start--;

      if(start >= end || start >= (int)lines.size()) {
	return false;
      }

      if(start - window > 0) {
        start -= window;
        end -= window;
      }
      else {
        start = 0;
	end = window * 2;
      }

      for(int i = start; i < (int)lines.size() && i < (int)end; i++) {
	if(i + 1 == cur_line_num) {
	  cout << right << "=>" << setw(window) << (i + 1) << ": " << lines[i] << endl;
	}
	else {
	  cout << right << setw(window + 2) << (i + 1) << ": " << lines[i] << endl;
	}
      }

      return true;
    }

    const string& GetFileName() {
      return file_name;
    }
  };

  /********************************
   * Interactive command line
   * debugger
   ********************************/
  class Debugger {
    string program_file;
    string base_path;
    bool quit;
    // break info
    list<UserBreak*> breaks;
    int cur_line_num;
    string cur_file_name;
    StackFrame** cur_call_stack;
    long cur_call_stack_pos;
    bool is_error;
    bool is_next;
    bool is_next_line;
    bool is_jmp_out;
    long* ref_mem;
    StackClass* ref_klass;
    // interpreter variables
    vector<string> arguments;
    StackInterpreter* interpreter;
    StackProgram* cur_program;
    StackFrame* cur_frame;
    long* op_stack;
    long* stack_pos;

    bool FileExists(const string &file_name, bool is_exe = false) {
      ifstream touch(file_name.c_str(), ios::binary);
      if(touch.is_open()) {
	if(is_exe) {
	  int magic_num;
	  touch.read((char*)&magic_num, sizeof(int));
	  touch.close();
	  return magic_num == 0xdddd;
	}
	touch.close();
	return true;
      }

      return false;
    }

    string PrintMethod(StackMethod* method) {
      string mthd_name = method->GetName();
      unsigned long index = mthd_name.find_last_of(':');
      if(index != string::npos) {
	mthd_name.replace(index, 1, 1, '(');
	if(mthd_name[mthd_name.size() - 1] == ',') {
	  mthd_name.replace(mthd_name.size() - 1, 1, 1, ')');
	}
	else {
	  mthd_name += ')';
	}
      }

      index = mthd_name.find_last_of(':');
      if(index != string::npos) {
	mthd_name = mthd_name.substr(index + 1);
      }
      
      return mthd_name;
    }
    
    bool DirectoryExists(const string &dir_name) {
#ifdef _WIN32
      HANDLE file = CreateFile(dir_name.c_str(), GENERIC_READ,
			       FILE_SHARE_READ, NULL, OPEN_EXISTING,
			       FILE_FLAG_BACKUP_SEMANTICS, NULL);

      if(file == INVALID_HANDLE_VALUE) {
	return false;
      }
      CloseHandle(file);

      return true;
#else
      DIR* dir = opendir(dir_name.c_str());
      if(dir) {
        closedir(dir);
        return true;
      }

      return false;
#endif
    }

    bool DeleteBreak(int line_num, const string &file_name) {
      UserBreak* user_break = FindBreak(line_num, file_name);
      if(user_break) {
	breaks.remove(user_break);
	return true;
      }

      return false;
    }

    UserBreak* FindBreak(int line_num, const string &file_name) {
      for(list<UserBreak*>::iterator iter = breaks.begin(); iter != breaks.end(); iter++) {
	UserBreak* user_break = (*iter);
	if(user_break->line_num == line_num && user_break->file_name == file_name) {
	  return *iter;
	}
      }

      return NULL;
    }

    bool AddBreak(int line_num, const string &file_name) {
      if(!FindBreak(line_num, file_name)) {
	UserBreak* user_break = new UserBreak;
	user_break->line_num = line_num;
	user_break->file_name = file_name;
	breaks.push_back(user_break);
	return true;
      }

      return false;
    }

    void ListBreaks() {
      cout << "breaks:" << endl;
      list<UserBreak*>::iterator iter;
      for(iter = breaks.begin(); iter != breaks.end(); iter++) {
	cout << "  break: file='" << (*iter)->file_name << ":" << (*iter)->line_num << "'" << endl;
      }
    }

    void PrintDeclarations(StackDclr** dclrs, int dclrs_num, const string& cls_name) {
      for(int i = 0; i < dclrs_num; i++) {
	StackDclr* dclr = dclrs[i];

	// parse name
	int param_name_index = dclrs[i]->name.find_last_of(':');
	const string &param_name = dclrs[i]->name.substr(param_name_index + 1);
	cout << "    parameter: name='" << param_name << "', ";

	// parse type
	switch(dclr->type) {
	case INT_PARM:
	  cout << "type=Int" << endl;
	  break;

	case FLOAT_PARM:
	  cout << "type=Float" << endl;
	  break;

	case BYTE_ARY_PARM:
	  cout << "type=Byte[]" << endl;
	  break;

	case INT_ARY_PARM:
	  cout << "type=Int[]" << endl;
	  break;

	case FLOAT_ARY_PARM:
	  cout << "type=Float[]" << endl;
	  break;

	case OBJ_PARM:
	  cout << "type=" << cur_program->GetClass(cls_name)->GetName() << endl;
	  break;

	case OBJ_ARY_PARM:
	  cout << "type=Object[]" << endl;
	  break;

    case FUNC_PARM:
	  cout << "type=Function" << endl;
	  break;
	}
      }
    }

    Command* ProcessCommand(const string &line);
    void ProcessRun();
    void ProcessExe(Load* load);
    void ProcessSrc(Load* load);
    void ProcessArgs(Load* load);
    void ProcessInfo(Info* info);
    void ProcessBreak(FilePostion* break_command);
    void ProcessBreaks();
    void ProcessDelete(FilePostion* break_command);
    void ProcessList(FilePostion* break_command);
    void ProcessPrint(Print* print);
    void ClearProgram();
    void ClearBreaks();

    void EvaluateExpression(Expression* expression);
    void EvaluateReference(Reference* reference, bool is_instance);
    void EvaluateObjectReference(Reference* reference, int index);
    void EvaluateByteReference(Reference* reference, int index);
    void EvaluateIntFloatReference(Reference* reference, int index, bool is_float);
    void EvaluateCalculation(CalculatedExpression* expression);

  public:
    Debugger(const string &fn, const string &bp) {
      program_file = fn;
      base_path = bp;
      quit = false;
      // clear program
      interpreter = NULL;
      op_stack = NULL;
      stack_pos = NULL;
      cur_line_num = -2;
      cur_frame = NULL;
      cur_program = NULL;
      cur_call_stack = NULL;
      cur_call_stack_pos = 0;
      is_error = false;
      ref_mem = NULL;
      ref_mem = NULL;
      is_jmp_out = false;
    }

    ~Debugger() {
      ClearProgram();
      ClearBreaks();
    }

    // start debugger
    void Debug();

    // runtime callback
    void ProcessInstruction(StackInstr* instr, long ip, StackFrame** call_stack,
			    long call_stack_pos, StackFrame* frame);
  };
}
#endif
