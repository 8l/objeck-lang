/***************************************************************************
 * Translates a parse tree into an intermediate format.  This format is
 * used for optimizations and target output.
 *
 * Copyright (c) 2008, 2009, 2010 Randy Hollines
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

#ifndef __INTERMEDIATE_H__
#define __INTERMEDIATE_H__

#include "tree.h"
#include "target.h"

using namespace frontend;
using namespace backend;

class IntermediateEmitter;

/****************************
 * Creates a binary search tree
 * for case statements.
 * Searches are log2(n).
 ****************************/
enum SelectOperation {
  CASE_LESS,
  CASE_EQUAL,
  CASE_LESS_OR_EQUAL
};

class SelectNode {
  int id;
  int value;
  int value2;
  SelectOperation operation;
  SelectNode* left;
  SelectNode* right;

 public:
  SelectNode(int i, int v) {
    id = i;
    value = v;
    operation = CASE_EQUAL;
    left = NULL;
    right = NULL;
  }

  SelectNode(int i, int v, SelectNode* l, SelectNode* r) {
    id = i;
    operation = CASE_LESS;
    value = v;
    left = l;
    right = r;
  }

  SelectNode(int i, int v, int v2, SelectNode* l, SelectNode* r) {
    id = i;
    operation = CASE_LESS_OR_EQUAL;
    value = v;
    value2 = v2;
    left = l;
    right = r;
  }

  ~SelectNode() {
    if(left) {
      delete left;
      left = NULL;
    }

    if(right) {
      delete right;
      right = NULL;
    }
  }

  int GetId() {
    return id;
  }

  int GetValue() {
    return value;
  }

  int GetValue2() {
    return value2;
  }

  SelectOperation GetOperation() {
    return operation;
  }

  SelectNode* GetLeft() {
    return left;
  }

  SelectNode* GetRight() {
    return right;
  }
};

class SelectArrayTree {
  int* values;
  SelectNode* root;
  IntermediateEmitter* emitter;
  map<int, int> value_label_map;
  Select* select;
  int other_label;

  SelectNode* divide(int start, int end);
  void Emit(SelectNode* node, int end_label);

 public:
  SelectArrayTree(Select* s, IntermediateEmitter* e);

  ~SelectArrayTree() {
    delete[] values;
    values = NULL;

    delete root;
    root = NULL;
  }

  void Emit();
};

/****************************
 * Translates a parse tree
 * into intermediate code. Also
 * identifies basic bloks
 * for optimization.
 ****************************/
class IntermediateEmitter {  
  ParsedProgram* parsed_program;
  ParsedBundle* parsed_bundle;
  IntermediateBlock* imm_block;
  IntermediateProgram* imm_program;
  IntermediateClass* imm_klass;
  IntermediateMethod* imm_method;
  Class* current_class;
  Method* current_method;
  SymbolTable* current_table;
  int conditional_label;
  int unconditional_label;
  bool is_lib;
  bool is_debug;
  friend class SelectArrayTree;
  bool is_new_inst;
  // NOTE: used to determine if two instantiated 
  // objects instances need to be swapped as 
  // method parameters
  int new_char_str_count; 
  int cur_line_num;
  stack<int> break_labels;
  
  // emit operations
  void EmitStrings();
  void EmitLibraries(Linker* linker);
  void EmitBundles();
  IntermediateEnum* EmitEnum(Enum* eenum);
  IntermediateClass* EmitClass(Class* klass);
  IntermediateMethod* EmitMethod(Method* method);
  void EmitStatement(Statement* statement);
  void EmitIf(If* if_stmt);
  void EmitIf(If* if_stmt, int next_label, int end_label);
  void EmitDoWhile(DoWhile* do_while_stmt);
  void EmitWhile(While* while_stmt);
  void EmitSelect(Select* select_stmt);
  void EmitFor(For* for_stmt);
  void EmitCriticalSection(CriticalSection* critical_stmt);
  void EmitIndices(ExpressionList* indices);
  void EmitExpressions(ExpressionList* parameters);
  void EmitExpression(Expression* expression);
  void EmitStaticArray(StaticArray* array);
  void EmitCharacterString(CharacterString* char_str);
  void EmitAndOr(CalculatedExpression* expression);
  void EmitCalculation(CalculatedExpression* expression);
  void EmitCast(Expression* expression);
  void EmitVariable(Variable* variable);
  void EmitAssignment(Assignment* assignment);
  void EmitDeclaration(Declaration* declaration);
  void EmitMethodCallParameters(MethodCall* method_call);
  void EmitMethodCall(MethodCall* method_call, bool is_nested, bool is_cast);
  void EmitSystemDirective(SystemStatement* statement);
  int CalculateEntrySpace(SymbolTable* table, int &index,
                          IntermediateDeclarations* parameters, bool is_static);
  int CalculateEntrySpace(IntermediateDeclarations* parameters, bool is_static);

  int OrphanReturn(MethodCall* method_call) {
    if(!method_call) {
      return -1;
    }

    if(method_call->GetCallType() == ENUM_CALL) {
      return 0;
    }

    Type* rtrn = NULL;
    if(method_call->GetMethod()) {
      rtrn = method_call->GetMethod()->GetReturn();
    } 
    else if(method_call->GetLibraryMethod()) {
      rtrn = method_call->GetLibraryMethod()->GetReturn();
    }
    else if(method_call->IsDynamicFunctionCall()) {
      rtrn =  method_call->GetDynamicFunctionEntry()->GetType()->GetFunctionReturn();
    }
    
    if(rtrn) {
      if(rtrn->GetType() != frontend::NIL_TYPE) {
        if(rtrn->GetDimension() > 0) {
          return 0;
        } else {
          switch(rtrn->GetType()) {
          case frontend::BOOLEAN_TYPE:
          case frontend::BYTE_TYPE:
          case frontend::CHAR_TYPE:
          case frontend::INT_TYPE:
          case frontend::CLASS_TYPE:
            return 0;

          case frontend::FLOAT_TYPE:
            return 1;

	  case frontend::FUNC_TYPE:
            return 2;

#ifdef _DEBUG
	  default:
	    assert(false);
	    break;
#endif
          }
        }
      }
    }

    return -1;
  }

  inline Class* SearchProgramClasses(const string &klass_name) {
    Class* klass = parsed_program->GetClass(klass_name);
    if(!klass) {
      klass = parsed_program->GetClass(parsed_bundle->GetName() + "." + klass_name);
      if(!klass) {
        vector<string> uses = parsed_program->GetUses();
        for(unsigned int i = 0; !klass && i < uses.size(); i++) {
          klass = parsed_program->GetClass(uses[i] + "." + klass_name);
        }
      }
    }

    return klass;
  }

  inline Enum* SearchProgramEnums(const string &eenum_name) {
    Enum* eenum = parsed_program->GetEnum(eenum_name);
    if(!eenum) {
      eenum = parsed_program->GetEnum(parsed_bundle->GetName() + "." + eenum_name);
      if(!eenum) {
        vector<string> uses = parsed_program->GetUses();
        for(unsigned int i = 0; !eenum && i < uses.size(); i++) {
          eenum = parsed_program->GetEnum(uses[i] + "." + eenum_name);
        }
      }
    }

    return eenum;
  }

  void Show(const string &msg, const int line_num, int depth) {
    cout << setw(4) << line_num << ": ";
    for(int i = 0; i < depth; i++) {
      cout << "  ";
    }
    cout << msg << endl;
  }

  string ToString(int v) {
    ostringstream str;
    str << v;
    return str.str();
  }

  string ToString(double d) {
    ostringstream str;
    str << d;
    return str.str();
  }

  // creates a new basic block
  void NewBlock() {
    // new basic block
    imm_block = new IntermediateBlock;
    if(current_method) {
      imm_method->AddBlock(imm_block);
    } else {
      imm_klass->AddBlock(imm_block);
    }
  }

  inline bool IntStringHolderEqual(IntStringHolder* lhs, IntStringHolder* rhs) {
    if(lhs->length != rhs->length) {
      return false;
    }
    
    for(int i = 0; i < lhs->length; i++) {
      if(lhs->value[i] != rhs->value[i]) {
	return false;
      }
    }
    
    return true;
  }

  inline bool FloatStringHolderEqual(FloatStringHolder* lhs, FloatStringHolder* rhs) {
    if(lhs->length != rhs->length) {
      return false;
    }
    
    for(int i = 0; i < lhs->length; i++) {
      if(lhs->value[i] != rhs->value[i]) {
	return false;
      }
    }
    
    return true;
  }

 public:
  IntermediateEmitter(ParsedProgram* p, bool l, bool d) {      
    parsed_program = p;
    is_lib = l;
    is_debug = d;
    // TODO: use an unsigned integer
    // note: negative numbers are used
    // for method inlining in VM
    imm_program = new IntermediateProgram;
    // 1,073,741,824 conditional labels
    conditional_label = -1;
    // 1,073,741,824 unconditional labels
    unconditional_label = (2 << 29) - 1;
    is_new_inst = false;
    new_char_str_count = 0;
    cur_line_num = -1;
  }

  ~IntermediateEmitter() {
  }

  void Translate();

  IntermediateProgram* GetProgram() {
    return imm_program;
  }

  int GetUnconditionalLabel() {
    return unconditional_label;
  }
};

#endif
