/***************************************************************************
 * Defines the VM execution model.
 *
 * Copyright (c) 2008-2012, Randy Hollines
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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stack>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "../shared/instrs.h"
#include "../shared/sys.h"
#include "../shared/traps.h"

#if defined(_WIN32) && !defined(_MINGW)
#include <windows.h>
#include <unordered_map>
using namespace stdext;
#else
#include <tr1/unordered_map>
#include <pthread.h>
#include <stdint.h>
namespace std {
  using namespace tr1;
}
#ifndef _MINGW
#include <dlfcn.h>
#endif
#endif

using namespace std;
using namespace instructions;

class StackClass;

inline string IntToString(int v)
{
  ostringstream str;
  str << v;
  return str.str();
}

/********************************
 * StackDclr struct
 ********************************/
struct StackDclr 
{
  string name;
  ParamType type;
  long id;
};

/********************************
 * StackInstr class
 ********************************/
class StackInstr 
{
  InstructionType type;
  long operand;
  long operand2;
  long operand3;
  FLOAT_VALUE float_operand;
  long native_offset;
  int line_num;
  
 public:
  StackInstr(int l, InstructionType t) {
    line_num = l;
    type = t;
    operand = native_offset = 0;
  }

  StackInstr(int l, InstructionType t, long o) {
    line_num = l;
    type = t;
    operand = o;
    native_offset = 0;
  }

  StackInstr(int l, InstructionType t, FLOAT_VALUE fo) {
    line_num = l;
    type = t;
    float_operand = fo;
    operand = native_offset = 0;
  }

  StackInstr(int l, InstructionType t, long o, long o2) {
    line_num = l;
    type = t;
    operand = o;
    operand2 = o2;
    native_offset = 0;
  }

  StackInstr(int l, InstructionType t, long o, long o2, long o3) {
    line_num = l;
    type = t;
    operand = o;
    operand2 = o2;
    operand3 = o3;
    native_offset = 0;
  }

  ~StackInstr() {
  }  

  inline InstructionType GetType() const {
    return type;
  }

  int GetLineNumber() const {
    return line_num;
  }

  inline void SetType(InstructionType t) {
    type = t;
  }

  inline long GetOperand() const {
    return operand;
  }

  inline long GetOperand2() const {
    return operand2;
  }

  inline long GetOperand3() const {
    return operand3;
  }

  inline void SetOperand(long o) {
    operand = o;
  }

  inline void SetOperand2(long o2) {
    operand2 = o2;
  }

  inline void SetOperand3(long o3) {
    operand3 = o3;
  }

  inline FLOAT_VALUE GetFloatOperand() const {
    return float_operand;
  }

  inline long GetOffset() const {
    return native_offset;
  }

  inline void SetOffset(long o) {
    native_offset = o;
  }
};

/********************************
 * JIT compile code
 ********************************/
class NativeCode 
{
  BYTE_VALUE* code;
  long size;
  FLOAT_VALUE* floats;

 public:
  NativeCode(BYTE_VALUE* c, long s, FLOAT_VALUE* f) {
    code = c;
    size = s;
    floats = f;
  }

  ~NativeCode() {
#ifdef _WIN32
    // TODO: needs to be fixed... hard to debug
    free(code);
    code = NULL;
#endif

#ifdef _X64
    free(code);
    code = NULL;
#endif

#ifndef _WIN32
    free(floats);
#else
    delete[] floats;
#endif
    floats = NULL;
  }

  BYTE_VALUE* GetCode() const {
    return code;
  }
  long GetSize() {
    return size;
  }
  FLOAT_VALUE* GetFloats() const {
    return floats;
  }
};

/********************************
 * StackMethod class
 ********************************/
class StackMethod {
  long id;
  string name;
  bool is_virtual;
  bool has_and_or;
  vector<StackInstr*> instrs;
  unordered_map<long, long> jump_table;
  
  long param_count;
  long mem_size;
  NativeCode* native_code;
  MemoryType rtrn_type;
  StackDclr** dclrs;
  long num_dclrs;
  StackClass* cls;
  
  const string& ParseName(const string &name) const {
    int state;
    size_t index = name.find_last_of(':');
    if(index > 0) {
      string params_name = name.substr(index + 1);

      // check return type
      index = 0;
      while(index < params_name.size()) {
        ParamType param;
        switch(params_name[index]) {
        case 'l':
          param = INT_PARM;
          state = 0;
          index++;
          break;

        case 'b':
          param = INT_PARM;
          state = 1;
          index++;
          break;

        case 'i':
          param = INT_PARM;
          state = 2;
          index++;
          break;

        case 'f':
          param = FLOAT_PARM;
          state = 3;
          index++;
          break;

        case 'c':
          param = INT_PARM;
          state = 4;
          index++;
          break;

        case 'o':
          param = OBJ_PARM;
          state = 5;
          index++;
          while(index < params_name.size() && params_name[index] != ',') {
            index++;
          }
          break;

	case 'm':
          param = FUNC_PARM;
          state = 6;
          index++;
          while(index < params_name.size() && params_name[index] != '~') {
            index++;
          }
	  while(index < params_name.size() && params_name[index] != ',') {
            index++;
          }
          break;

	default:
#ifdef _DEBUG
	  assert(false);
#endif
	  break;
        }

        // check array
        int dimension = 0;
        while(index < params_name.size() && params_name[index] == '*') {
          dimension++;
          index++;
        }

        if(dimension) {
          switch(state) {
          case 1:
            param = BYTE_ARY_PARM;
            break;

          case 0:
          case 2:
          case 4:
            param = INT_ARY_PARM;
            break;

          case 3:
            param = FLOAT_ARY_PARM;
            break;

          case 5:
            param = OBJ_ARY_PARM;
            break;
          }
        }

#ifdef _DEBUG
        switch(param) {
        case INT_PARM:
          cout << "  INT_PARM" << endl;
          break;

        case FLOAT_PARM:
          cout << "  FLOAT_PARM" << endl;
          break;

        case BYTE_ARY_PARM:
          cout << "  BYTE_ARY_PARM" << endl;
          break;

        case INT_ARY_PARM:
          cout << "  INT_ARY_PARM" << endl;
          break;

        case FLOAT_ARY_PARM:
          cout << "  FLOAT_ARY_PARM" << endl;
          break;

        case OBJ_PARM:
          cout << "  OBJ_PARM" << endl;
          break;

        case OBJ_ARY_PARM:
          cout << "  OBJ_ARY_PARM" << endl;
          break;

	case FUNC_PARM:
          cout << "  FUNC_PARM" << endl;
          break;

	default:
	  assert(false);
	  break;
        }
#endif

        // match ','
        index++;
      }
    }

    return name;
  }
  
 public:
  // mutex variable used to support 
  // concurrent JIT compiling
#ifdef _WIN32
  CRITICAL_SECTION jit_cs;
#else 
  pthread_mutex_t jit_mutex;
#endif

  StackMethod(long i, const string &n, bool v, bool h, StackDclr** d, long nd,
              long p, long m, MemoryType r, StackClass* k) {
#ifdef _WIN32
    InitializeCriticalSection(&jit_cs);
#else
    pthread_mutex_init(&jit_mutex, NULL);
#endif
    id = i;
    name = ParseName(n);
    is_virtual = v;
    has_and_or = h;
    native_code = NULL;
    dclrs = d;
    num_dclrs = nd;
    param_count = p;
    mem_size = m;
    rtrn_type = r;
    cls = k;
  }
  
  ~StackMethod() {
    // clean up
    if(dclrs) {
      for(int i = 0; i < num_dclrs; i++) {
        StackDclr* tmp = dclrs[i];
        delete tmp;
        tmp = NULL;
      }
      delete[] dclrs;
      dclrs = NULL;
    }
    
#ifdef _WIN32
    DeleteCriticalSection(&jit_cs); 
#endif
    
    // clean up
    if(native_code) {
      delete native_code;
      native_code = NULL;
    }
    
    // clean up
    while(!instrs.empty()) {
      StackInstr* tmp = instrs.front();
      instrs.erase(instrs.begin());
      // delete
      delete tmp;
      tmp = NULL;
    }
  }
  
  inline const string& GetName() {
    return name;
  }

  inline bool IsVirtual() {
    return is_virtual;
  }

  inline bool HasAndOr() {
    return has_and_or;
  }

  inline StackClass* GetClass() const {
    return cls;
  }

#ifdef _DEBUGGER
  int GetDeclaration(const string& name, StackDclr& found) {
    if(name.size() > 0) {
      // search for name
      int index = 0;
      for(int i = 0; i < num_dclrs; i++, index++) {
	StackDclr* dclr = dclrs[i];
	const string &dclr_name = dclr->name.substr(dclr->name.find_last_of(':') + 1);       
	if(dclr_name == name) {
	  found.name = dclr->name;
	  found.type = dclr->type;
	  found.id = index;	  
	  return true;
	}
	
	if(dclr->type == FLOAT_PARM || dclr->type == FUNC_PARM) {
	  index++;
	}
      }
    }
    
    return false;
  }
#endif
  
  inline StackDclr** GetDeclarations() const {
    return dclrs;
  }

  inline const int GetNumberDeclarations() {
    return num_dclrs;
  }
  
  void SetNativeCode(NativeCode* c) {
    native_code = c;
  }

  NativeCode* GetNativeCode() const {
    return native_code;
  }

  MemoryType GetReturn() const {
    return rtrn_type;
  }
  
  inline void AddLabel(long label_id, long index) {
    jump_table.insert(pair<long, long>(label_id, index));
  }
  
  inline long GetLabelIndex(long label_id) {
    unordered_map<long, long>::iterator found = jump_table.find(label_id);
    if(found != jump_table.end()) {
      return found->second;
    }    
    
    return -1;
  }
  
  void SetInstructions(vector<StackInstr*> i) {
    instrs = i;
  }

  void AddInstruction(StackInstr* i) {
    instrs.push_back(i);
  }

  long GetId() const {
    return id;
  }

  long GetParamCount() const {
    return param_count;
  }

  void SetParamCount(long c) {
    param_count = c;
  }

  long* NewMemory() {
    // +1 is for instance variable
    const long size = mem_size + 2;
    long* mem = new long[size];
    memset(mem, 0, size * sizeof(long));

    return mem;
  }

  long GetMemorySize() const {
    return mem_size;
  }

  long GetInstructionCount() const {
    return instrs.size();
  }

  StackInstr* GetInstruction(long i) const {
    return instrs[i];
  }
};

/********************************
 * ByteBuffer class
 ********************************/
class ByteBuffer {
  BYTE_VALUE* buffer;
  int pos;
  int max;

 public:
  ByteBuffer() {
    pos = 0;
    max = 4;
    buffer = new BYTE_VALUE[sizeof(long) * max];
  }

  ~ByteBuffer() {
    delete buffer;
    buffer = NULL;
  }

  inline void AddByte(BYTE_VALUE b) {
    if(pos >= max) {
      max *= 2;
      BYTE_VALUE* temp = new BYTE_VALUE[sizeof(long) * max];
      int i = pos;
      while(--i > -1) { 
        temp[i] = buffer[i];
      }
      delete buffer;
      buffer = temp;
    }
    buffer[pos++] = b;
  }

  inline BYTE_VALUE* GetBuffer() const {
    return buffer;
  }

  inline int GetSize() const {
    return pos;
  }
};

/********************************
 * StackClass class
 ********************************/
class StackClass {
  // map<long, StackMethod*> method_map;
  StackMethod** methods;
  int method_num;
  long id;
  string name;
  string file_name;
  long pid;
  bool is_virtual;
  long cls_space;
  long inst_space;
  StackDclr** dclrs;
  long num_dclrs;
  long* cls_mem;
  bool is_debug;
  
  map<const string, StackMethod*> method_name_map;

  long InitMemory(long size) {
    cls_mem = new long[size];
    memset(cls_mem, 0, size * sizeof(long));
    
    return size;
  }
  
 public:
  StackClass(long i, const string &ne, const string &fn, long p, 
	     bool v, StackDclr** d, long n, long cs, long is, bool b) {
    id = i;
    name = ne;
    file_name = fn;
    pid = p;
    is_virtual = v;
    dclrs = d;
    num_dclrs = n;
    cls_space = InitMemory(cs);
    inst_space  = is;
    is_debug = b;
  }

  ~StackClass() {
    // clean up
    if(dclrs) {
      for(int i = 0; i < num_dclrs; i++) {
        StackDclr* tmp = dclrs[i];
        delete tmp;
        tmp = NULL;
      }
      delete[] dclrs;
      dclrs = NULL;
    }

    for(int i = 0; i < method_num; i++) {
      StackMethod* method = methods[i];
      delete method;
      method = NULL;
    }
    delete[] methods;
    methods = NULL;

    if(cls_mem) {
      delete[] cls_mem;
      cls_mem = NULL;
    }
  }

  inline long GetId() const {
    return id;
  }

  bool IsDebug() {
    return is_debug;
  }

  inline const string& GetName() {
    return name;
  }

  inline const string& GetFileName() {
    return file_name;
  }

  inline StackDclr** GetDeclarations() const {
    return dclrs;
  }

  inline int GetNumberDeclarations() const {
    return num_dclrs;
  }

  inline long GetParentId() const {
    return pid;
  }

  inline bool IsVirtual() {
    return is_virtual;
  }

  inline long* GetClassMemory() const {
    return cls_mem;
  }

  inline long GetInstanceMemorySize() const {
    return inst_space;
  }

  void SetMethods(StackMethod** mthds, const int num) {
    methods = mthds;
    method_num = num;
    // add method names to map for virutal calls
    for(int i = 0; i < num; i++) {
      method_name_map.insert(make_pair(mthds[i]->GetName(), mthds[i]));
    }
  }

  inline StackMethod* GetMethod(long id) const {
#ifdef _DEBUG
    assert(id > -1 && id < method_num);
#endif
    return methods[id];
  }
  
  vector<StackMethod*> GetMethods(const string &n) {
    vector<StackMethod*> found;
    for(int i = 0; i < method_num; i++) {
      if(methods[i]->GetName().find(n) != string::npos) {
	found.push_back(methods[i]);
      }
    }

    return found;
  }
  
#ifdef _UTILS
  void List() {
    map<const string, StackMethod*>::iterator iter;
    for(iter = method_name_map.begin(); iter != method_name_map.end(); iter++) {
      StackMethod* mthd = iter->second;
      cout << "  method='" << mthd->GetName() << "'" << endl;
    }
  }
#endif

  inline StackMethod* GetMethod(const string &n) {
    map<const string, StackMethod*>::iterator result = method_name_map.find(n);
    if(result != method_name_map.end()) {
      return result->second;
    }

    return NULL;
  }

  inline StackMethod** GetMethods() const {
    return methods;
  }

  inline int GetMethodCount() const {
    return method_num;
  }

#ifdef _DEBUGGER
  bool GetDeclaration(const string& name, StackDclr& found) {
    if(name.size() > 0) {
      // search for name
      int index = 0;
      for(int i = 0; i < num_dclrs; i++, index++) {
	StackDclr* dclr = dclrs[i];
	const string &dclr_name = dclr->name.substr(dclr->name.find_last_of(':') + 1);       
	if(dclr_name == name) {
	  found.name = dclr->name;
	  found.type = dclr->type;
	  found.id = index;
	  return true;
	}
	// update
	if(dclr->type == FLOAT_PARM || dclr->type == FUNC_PARM) {
	  index++;
	}
      }
    }
    
    return false;
  }
#endif
};

/********************************
 * StackProgram class
 ********************************/
class StackProgram {
  map<string, StackClass*> cls_map;
  StackClass** classes;
  int class_num;
  int* cls_hierarchy;
  int** cls_interfaces;
  int string_cls_id;
  int cls_cls_id;
  int mthd_cls_id;
  int data_type_cls_id;
  
  FLOAT_VALUE** float_strings;
  int num_float_strings;

  INT_VALUE** int_strings;
  int num_int_strings;
  
  BYTE_VALUE** char_strings;
  int num_char_strings;

  StackMethod* init_method;
#ifdef _WIN32
  static list<HANDLE> thread_ids;
  static CRITICAL_SECTION program_cs;
#else
  static list<pthread_t> thread_ids;
  static pthread_mutex_t program_mutex;
#endif

 public:
  StackProgram() {
    cls_hierarchy = NULL;
    cls_interfaces = NULL;
    classes = NULL;
    char_strings = NULL;
    string_cls_id = cls_cls_id = mthd_cls_id = data_type_cls_id = -1;
#ifdef _WIN32
    InitializeCriticalSection(&program_cs);
#endif
  }
  
  ~StackProgram() {
    if(classes) {
      for(int i = 0; i < class_num; i++) {
	StackClass* klass = classes[i];
	delete klass;
	klass = NULL;
      }
      delete[] classes;
      classes = NULL;
    }

    if(cls_hierarchy) {
      delete[] cls_hierarchy;
      cls_hierarchy = NULL;
    }

    if(cls_interfaces) {
      for(int i = 0; i < class_num; i++) {
	delete[] cls_interfaces[i];
	cls_interfaces[i] = NULL;
      }
      delete[] cls_interfaces;
      cls_interfaces = NULL;
    }
    
    if(float_strings) {
      for(int i = 0; i < num_float_strings; i++) {
        FLOAT_VALUE* tmp = float_strings[i];
        delete[] tmp;
        tmp = NULL;
      }
      delete[] float_strings;
      float_strings = NULL;
    }

    if(int_strings) {
      for(int i = 0; i < num_int_strings; i++) {
        INT_VALUE* tmp = int_strings[i];
        delete[] tmp;
        tmp = NULL;
      }
      delete[] int_strings;
      int_strings = NULL;
    }

    if(char_strings) {
      for(int i = 0; i < num_char_strings; i++) {
	BYTE_VALUE* tmp = char_strings[i];
	delete [] tmp;
	tmp = NULL;
      }
      delete[] char_strings;
      char_strings = NULL;
    }
    
    if(init_method) {
      delete init_method;
      init_method = NULL;
    }

#ifdef _WIN32
    DeleteCriticalSection(&program_cs);
#endif
  }

#ifdef _WIN32
  static void AddThread(HANDLE h) {
    EnterCriticalSection(&program_cs);
    thread_ids.push_back(h);
    LeaveCriticalSection(&program_cs);
  }

  static void RemoveThread(HANDLE h) {
    EnterCriticalSection(&program_cs);
    thread_ids.remove(h);
    LeaveCriticalSection(&program_cs);
  }
  
  static list<HANDLE> GetThreads() {
    list<HANDLE> temp;
    EnterCriticalSection(&program_cs);
    temp = thread_ids;
    LeaveCriticalSection(&program_cs);
    
    return temp;
  }
#else
  static void AddThread(pthread_t t) {
    pthread_mutex_lock(&program_mutex);
    thread_ids.push_back(t);
    pthread_mutex_unlock(&program_mutex);
  }

  static void RemoveThread(pthread_t t) {
    pthread_mutex_lock(&program_mutex);
    thread_ids.remove(t);
    pthread_mutex_unlock(&program_mutex);
  }
  
  static list<pthread_t> GetThreads() {
    list<pthread_t> temp;
    pthread_mutex_lock(&program_mutex);
    temp = thread_ids;
    pthread_mutex_unlock(&program_mutex);
    
    return temp;
  }
#endif
  
  void SetInitializationMethod(StackMethod* i) {
    init_method = i;
  }

  StackMethod* GetInitializationMethod() const {
    return init_method;
  }
  
  int GetStringObjectId() const {
    return string_cls_id;
  }
  
  void SetStringObjectId(int id) {
    string_cls_id = id;
  }
  
  int GetClassObjectId() {
    if(cls_cls_id < 0) {
      StackClass* cls = GetClass("Introspection.Class");
      if(!cls) {
	cerr << ">>> Internal error: unable to find class: Introspection.Class <<<" << endl;
	exit(1);
      }
      cls_cls_id = cls->GetId();
    }
    
    return cls_cls_id;
  }
  
  int GetMethodObjectId() {
    if(mthd_cls_id < 0) {
      StackClass* cls = GetClass("Introspection.Method");
      if(!cls) {
	cerr << ">>> Internal error: unable to find class: Introspection.Method <<<" << endl;
	exit(1);
      }
      mthd_cls_id = cls->GetId();
    }
    
    return mthd_cls_id;
  }
  
  int GetDataTypeObjectId() {
    if(data_type_cls_id < 0) {
      StackClass* cls = GetClass("Introspection.DataType");
      if(!cls) {
	cerr << ">>> Internal error: unable to find class: Introspection.DataType <<<" << endl;
	exit(1);
      }
      data_type_cls_id = cls->GetId();
    }
    
    return data_type_cls_id;
  }
  
  void SetFloatStrings(FLOAT_VALUE** s, int n) {
    float_strings = s;
    num_float_strings = n;
  }
  
  void SetIntStrings(INT_VALUE** s, int n) {
    int_strings = s;
    num_int_strings = n;
  }

  void SetCharStrings(BYTE_VALUE** s, int n) {
    char_strings = s;
    num_char_strings = n;
  }  
  
  FLOAT_VALUE** GetFloatStrings() const {
    return float_strings;
  }

  INT_VALUE** GetIntStrings() const {
    return int_strings;
  }

  BYTE_VALUE** GetCharStrings() const {
    return char_strings;
  }
  
  void SetClasses(StackClass** clss, const int num) {
    classes = clss;
    class_num = num;
    
    for(int i = 0; i < num; i++) {
      const string &name = clss[i]->GetName();
      if(name.size() > 0) {	
	cls_map.insert(pair<string, StackClass*>(name, clss[i]));
      }
    }
  }

#ifdef _UTILS
  void List() {
    map<string, StackClass*>::iterator iter;
    for(iter = cls_map.begin(); iter != cls_map.end(); iter++) {
      StackClass* cls = iter->second;
      cout << "==================================" << endl;
      cout << "class='" << cls->GetName() << "'" << endl;
      cout << "==================================" << endl;
      cls->List();
    }
  }
#endif
  
  StackClass* GetClass(const string &n) {
    if(classes) {
      map<string, StackClass*>::iterator find = cls_map.find(n);
      if(find != cls_map.end()) {
	return find->second;
      }
    }
    
    return NULL;
  }
  
  void SetHierarchy(int* h) {
    cls_hierarchy = h;
  }

  inline int* GetHierarchy() const {
    return cls_hierarchy;
  }

  void SetInterfaces(int** i) {
    cls_interfaces = i;
  }

  inline int** GetInterfaces() const {
    return cls_interfaces;
  }  

  inline StackClass* GetClass(long id) const {
    if(id > -1 && id < class_num) {
      return classes[id];
    }

    return NULL;
  }

#ifdef _DEBUGGER
  bool HasFile(const string &fn) {
    for(int i = 0; i < class_num; i++) {
      if(classes[i]->GetFileName() == fn) {
        return true;
      }
    }

    return false;
  }
#endif
};

/********************************
 * StackFrame class
 ********************************/
class StackFrame {
  StackMethod* method;
  long* mem;
  long ip;
  bool jit_called;
  
 public:
  StackFrame(StackMethod* md, long* inst) {
    method = md;
    mem = md->NewMemory();
    mem[0] = (long)inst;
    ip = -1;
    jit_called = false;
  }
  
  ~StackFrame() {
    delete[] mem;
    mem = NULL;
  }
  
  inline StackMethod* GetMethod() const {
    return method;
  }

  inline long* GetMemory() const {
    return mem;
  }

  inline void SetIp(long i) {
    ip = i;
  }

  inline long GetIp() const {
    return ip;
  }

  inline void SetJitCalled(bool j) {
    jit_called = j;
  }

  inline bool IsJitCalled() {
    return jit_called;
  }
};

/********************************
 * ObjectSerializer class
 ********************************/
class ObjectSerializer 
{
  vector<BYTE_VALUE> values;
  map<long*, long> serial_ids;
  long next_id;
  long cur_id;
  
  void CheckObject(long* mem, bool is_obj, long depth);
  void CheckMemory(long* mem, StackDclr** dclrs, const long dcls_size, long depth);
  void Serialize(long* inst);

  bool WasSerialized(long* mem) {
    map<long*, long>::iterator find = serial_ids.find(mem);
    if(find != serial_ids.end()) {
      cur_id = find->second;
      SerializeInt(cur_id);
      
      return true;
    }
    next_id++;
    cur_id = next_id * -1;
    serial_ids.insert(pair<long*, long>(mem, next_id));
    SerializeInt(cur_id);
    
    return false;
  }
  
  inline void SerializeByte(BYTE_VALUE v) {
    BYTE_VALUE* bp = (BYTE_VALUE*)&v;
    for(size_t i = 0; i < sizeof(v); i++) {
      values.push_back(*(bp + i));
    }
  }
  
  inline void SerializeInt(INT_VALUE v) {
    BYTE_VALUE* bp = (BYTE_VALUE*)&v;
    for(size_t i = 0; i < sizeof(v); i++) {
      values.push_back(*(bp + i));
    }
  }
  
  inline void SerializeFloat(FLOAT_VALUE v) {
    BYTE_VALUE* bp = (BYTE_VALUE*)&v;
    for(size_t i = 0; i < sizeof(v); i++) {
      values.push_back(*(bp + i));
    }
  }

  inline void SerializeBytes(void* array, const long len) {
    BYTE_VALUE* bp = (BYTE_VALUE*)array;
    for(long i = 0; i < len; i++) {
      values.push_back(*(bp + i));
    }
  }
  
 public:
  ObjectSerializer(long* i) {
    Serialize(i);
  }
  
  ~ObjectSerializer() {
  }
  
  vector<BYTE_VALUE>& GetValues() {
    return values;
  }
};

/********************************
 *  ObjectDeserializer class
 ********************************/
class ObjectDeserializer 
{
  const BYTE_VALUE* buffer;
  long buffer_offset;
  long buffer_array_size;
  long* op_stack;
  long* stack_pos;
  StackClass* cls;
  long* instance;
  long instance_pos;
  map<INT_VALUE, long*> mem_cache;
  
  BYTE_VALUE DeserializeByte() {
    BYTE_VALUE value;
    memcpy(&value, buffer + buffer_offset, sizeof(value));
    buffer_offset += sizeof(value);

    return value;
  }

  INT_VALUE DeserializeInt() {
    INT_VALUE value;
    memcpy(&value, buffer + buffer_offset, sizeof(value));
    buffer_offset += sizeof(value);

    return value;
  }
  
  FLOAT_VALUE DeserializeFloat() {
    FLOAT_VALUE value;
    memcpy(&value, buffer + buffer_offset, sizeof(value));
    buffer_offset += sizeof(value);

    return value;
  }
  
 public:
  ObjectDeserializer(const BYTE_VALUE* b, long s, long* stack, long* pos) {
    op_stack = stack;
    stack_pos = pos;
    buffer = b;
    buffer_array_size = s;
    buffer_offset = 0;
    cls = NULL;
    instance = NULL;
    instance_pos = 0;
  }
  
  ObjectDeserializer(const BYTE_VALUE* b, long o, map<INT_VALUE, long*> &c, 
		     long s, long* stack, long* pos) {
    op_stack = stack;
    stack_pos = pos;
    buffer = b;
    buffer_array_size = s;
    buffer_offset = o;
    mem_cache = c;
    cls = NULL;
    instance = NULL;
    instance_pos = 0;
  }
  
  ~ObjectDeserializer() {    
  }
  
  inline long GetOffset() const {
    return buffer_offset;
  }

  map<INT_VALUE, long*> GetMemoryCache() {
    return mem_cache;
  }
  
  long* DeserializeObject();
};

// call back for DLL method calls
void APITools_MethodCall(long* op_stack, long *stack_pos, long* instance, 
			 const char* cls_id, const char* mthd_id);
void APITools_MethodCallId(long* op_stack, long *stack_pos, long *instance, 
			 const int cls_id, const int mthd_id);			 

#endif
