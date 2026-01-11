#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>
#include <set>
#include <map>
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;

class ClassTable {
private:
  int semant_errors;
  void install_basic_classes();
  ostream& error_stream;
  
  // 类符号表
  SymbolTable<Symbol, Class_> *class_table;
  
  // 存储所有类定义
  Classes all_classes;
  
  // 基本类的指针
  Class_ Object_class_ptr;
  Class_ IO_class_ptr;
  Class_ Int_class_ptr;
  Class_ Bool_class_ptr;
  Class_ Str_class_ptr;
  
  // 当前处理的信息
  Symbol current_filename;
  Class_ current_class;
  
  // 方法查找表
  std::map<Symbol, std::map<Symbol, method_class*>> method_table;
  
  // 辅助函数
  void build_inheritance_graph();
  void check_inheritance();
  void check_main();
  void collect_methods();
  
  // 类型检查函数
  void type_check_class(Class_ c);
  Symbol type_check_expression(Expression expr, SymbolTable<Symbol, Symbol> *env);
  Symbol type_check_method(method_class *method, SymbolTable<Symbol, Symbol> *env);
  
  // 类型关系函数
  bool is_subtype(Symbol child, Symbol parent);
  Symbol lub(Symbol type1, Symbol type2);
  method_class* find_method(Symbol class_name, Symbol method_name);
  
  // 获取类名
  Symbol get_class_name(Class_ c);
  
public:
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
};

#endif
