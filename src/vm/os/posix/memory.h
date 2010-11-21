/***************************************************************************
 * VM memory manager. Implements a simple "mark and sweep" collection algorithm.
 *
 * Copyright (c) 2007, 2008, Randy Hollines
 * All rights reserved.
 *CollectMemory(o
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

#ifndef __MEM_MGR_H__
#define __MEM_MGR_H__

#include "../../common.h"

// basic vm tuning parameters
// #define MEM_MAX 512
// #define MEM_MAX 1024 * 3
#define MEM_MAX 1024
#define UNCOLLECTED_COUNT 4
#define COLLECTED_COUNT 8

struct CollectionInfo {
  long* op_stack;
  long stack_pos;
};

struct ClassMethodId {
  long* self;
  long* mem;
  long cls_id;
  long mthd_id;
};

class MemoryManager {
  static MemoryManager* instance;
  static StackProgram* prgm;
  
  static list<ClassMethodId*> jit_roots;
  static list<StackFrame*> pda_roots; // deleted elsewhere
  static map<long*, long> allocated_memory;
  static map<long*, long> static_memory;
  static vector<long*> marked_memory;
  
#ifndef _GC_SERIAL
  static pthread_mutex_t static_mutex;
  static pthread_mutex_t jit_mutex;
  static pthread_mutex_t pda_mutex;
  static pthread_mutex_t allocated_mutex;
  static pthread_mutex_t marked_mutex;
  static pthread_mutex_t marked_sweep_mutex;
#endif
    
  // note: protected by 'allocated_mutex'
  static long allocation_size;
  static long mem_max_size;
  static long uncollected_count;
  static long collected_count;

  MemoryManager() {
  }

  // if return true, trace memory otherwise do not
  static inline bool MarkMemory(long* mem);

public:
  static void Initialize(StackProgram* p);
  static MemoryManager* Instance();

  static void Clear() {
    while(!jit_roots.empty()) {
      ClassMethodId* tmp = jit_roots.front();
      jit_roots.erase(jit_roots.begin());
      // delete
      delete tmp;
      tmp = NULL;
    }

    map<long*, long>::iterator iter;
    for(iter = allocated_memory.begin(); iter != allocated_memory.end(); iter++) {
      long* temp = iter->first;

      --temp;
      free(temp);
      temp = NULL;
    }
    allocated_memory.clear();

    delete instance;
    instance = NULL;
  }

  static void AddStaticMemory(long* mem);
  
  // add and remove jit roots
  static void AddJitMethodRoot(long cls_id, long mthd_id, long* self, long* mem, long offset);
  static void RemoveJitMethodRoot(long* mem);

  // add and remove pda roots
  void AddPdaMethodRoot(StackFrame* frame);
  void RemovePdaMethodRoot(StackFrame* frame);

  // recover memory
  static void CollectMemory(long* op_stack, long stack_pos);

#ifdef _WIN32
  static DWORD WINAPI CollectMemory(LPVOID arg);
  static DWORD WINAPI CheckStack(LPVOID arg);
  static DWORD WINAPI CheckJitRoots(LPVOID arg);
  static DWORD WINAPI CheckPdaRoots(LPVOID arg);
#else
  static void* CollectMemory(void* arg);
  static void* CheckStack(void* arg);
  static void* CheckJitRoots(void* arg);
  static void* CheckPdaRoots(void* arg);
#endif
  
  static void CheckMemory(long* mem, StackDclr** dclrs, const long dcls_size, const long depth);
  static void CheckObject(long* mem, bool is_obj, const long depth);

  // TODO: change to static
  // memory allocation
  long* AllocateObject(const long obj_id, long* op_stack, long stack_pos);
  long* AllocateArray(const long size, const MemoryType type, long* op_stack, long stack_pos);

  // object verification
  long* ValidObjectCast(long* mem, const long to_id, int* cls_hierarchy);
  
  inline long GetObjectID(long* mem) {
#ifndef _GC_SERIAL
    pthread_mutex_lock(&allocated_mutex);
#endif
    map<long*, long>::iterator result = allocated_memory.find(mem);
    if(result != allocated_memory.end()) {
#ifndef _GC_SERIAL
      pthread_mutex_unlock(&allocated_mutex);
#endif
      return -result->second;
    } 
    else {
#ifndef _GC_SERIAL
      pthread_mutex_unlock(&allocated_mutex);
#endif
      return -1;
    }
  }

  static inline StackClass* GetClass(long* mem) {
    if(mem) {
#ifndef _GC_SERIAL
      pthread_mutex_lock(&allocated_mutex);
#endif
      map<long*, long>::iterator result = allocated_memory.find(mem);
      if(result != allocated_memory.end()) {
#ifndef _GC_SERIAL
	pthread_mutex_unlock(&allocated_mutex);
#endif
        return prgm->GetClass(-result->second);
      }
    }
#ifndef _GC_SERIAL
    pthread_mutex_unlock(&allocated_mutex);
#endif
    return NULL;
  }

  ~MemoryManager() {
  }
};

#endif
