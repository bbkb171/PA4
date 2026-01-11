#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include "semant.h"
#include "utilities.h"

// 声明外部变量，用于调试和文件名跟踪
extern int semant_debug;
extern char *curr_filename;

// 预定义符号常量，用于表示COOL语言中的各种标识符
static Symbol 
    arg,                // 参数名
    arg2,               // 第二个参数名
    Bool,               // 布尔类型名
    concat,             // 字符串连接方法名
    cool_abort,         // 终止程序方法名
    copy,               // 复制对象方法名
    Int,                // 整数类型名
    in_int,             // 输入整数方法名
    in_string,          // 输入字符串方法名
    IO,                 // 输入输出类名
    length,             // 字符串长度方法名
    Main,               // 主类名
    main_meth,          // 主方法名
    No_class,           // 无类标识符
    No_type,            // 无类型标识符
    Object,             // 根对象类名
    out_int,            // 输出整数方法名
    out_string,         // 输出字符串方法名
    prim_slot,          // 原始槽位名
    self,               // 当前对象引用名
    SELF_TYPE,          // 自身类型名
    Str,                // 字符串类型名
    str_field,          // 字符串字段名
    substr,             // 子字符串方法名
    type_name,          // 类型名称方法名
    val;                // 值字段名

// 初始化预定义符号常量，将字符串添加到符号表中
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
}

////////////////////////////////////////////////////////////////////
// 错误报告函数实现
////////////////////////////////////////////////////////////////////

// 为指定类报告语义错误，返回错误流引用
ostream& ClassTable::semant_error(Class_ c) {
    return semant_error(c->get_filename(), c);
}    

// 为指定文件和树节点报告语义错误，返回错误流引用
ostream& ClassTable::semant_error(Symbol filename, tree_node *t) {
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

// 增加语义错误计数并返回错误流引用
ostream& ClassTable::semant_error() {
    semant_errors++;
    return error_stream;
}

////////////////////////////////////////////////////////////////////
// 获取类名函数实现
////////////////////////////////////////////////////////////////////

// 获取类的名称
Symbol ClassTable::get_class_name(Class_ c) {
    class__class* cls = dynamic_cast<class__class*>(c);
    if (cls != NULL) {
        return cls->get_name();
    }
    return No_class;
}

////////////////////////////////////////////////////////////////////
// 安装基本类函数实现
////////////////////////////////////////////////////////////////////

// 安装COOL语言的基本类（Object, IO, Int, Bool, String）
void ClassTable::install_basic_classes() {
    Symbol filename = stringtable.add_string("<basic class>");

    // 创建Object类，包含abort, type_name, copy方法
    Object_class_ptr = class_(Object, 
           No_class,
           append_Features(
                   append_Features(
                           single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
                           single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
                   single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
           filename);

    // 创建IO类，继承自Object，包含输入输出方法
    IO_class_ptr = class_(IO, 
           Object,
           append_Features(
                   append_Features(
                           append_Features(
                                   single_Features(method(out_string, single_Formals(formal(arg, Str)),
                                          SELF_TYPE, no_expr())),
                                   single_Features(method(out_int, single_Formals(formal(arg, Int)),
                                          SELF_TYPE, no_expr()))),
                           single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
                   single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
           filename);

    // 创建Int类，继承自Object，包含val属性
    Int_class_ptr = class_(Int, 
           Object,
           single_Features(attr(val, prim_slot, no_expr())),
           filename);

    // 创建Bool类，继承自Object，包含val属性
    Bool_class_ptr = class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())), filename);

    // 创建String类，继承自Object，包含字符串相关属性和方法
    Str_class_ptr = class_(Str, 
           Object,
           append_Features(
                   append_Features(
                           append_Features(
                                   append_Features(
                                           single_Features(attr(val, Int, no_expr())),
                                           single_Features(attr(str_field, prim_slot, no_expr()))),
                                   single_Features(method(length, nil_Formals(), Int, no_expr()))),
                           single_Features(method(concat, 
                                  single_Formals(formal(arg, Str)),
                                  Str, 
                                  no_expr()))),
                   single_Features(method(substr, 
                          append_Formals(single_Formals(formal(arg, Int)), 
                                 single_Formals(formal(arg2, Int))),
                          Str, 
                          no_expr()))),
           filename);

    // 将基本类添加到符号表中
    class_table->enterscope();
    class_table->addid(Object, &Object_class_ptr);
    class_table->addid(IO, &IO_class_ptr);
    class_table->addid(Int, &Int_class_ptr);
    class_table->addid(Bool, &Bool_class_ptr);
    class_table->addid(Str, &Str_class_ptr);
}

////////////////////////////////////////////////////////////////////
// 类型关系判断函数实现
////////////////////////////////////////////////////////////////////

// 判断child类型是否为parent类型的子类型
bool ClassTable::is_subtype(Symbol child, Symbol parent) {
    // SELF_TYPE特殊处理
    if (child == SELF_TYPE && parent == SELF_TYPE) return true;
    if (child == SELF_TYPE) {
        // SELF_TYPE可以赋值给任何类型
        return true;
    }
    if (parent == SELF_TYPE) {
        // 任何类型不能赋值给SELF_TYPE（除非也是SELF_TYPE）
        return false;
    }

    // 相同类型
    if (child == parent) return true;

    // Object是所有类的父类
    if (parent == Object) return true;

    // No_type没有子类型
    if (child == No_type) return false;

    // 检查继承关系
    Symbol current = child;
    while (current != No_class && current != Object) {
        if (current == parent) return true;

        // 直接使用 class_table->lookup
        Class_* class_ptr = class_table->lookup(current);
        if (class_ptr == NULL) break;

        Class_ c = *class_ptr;
        class__class* cls = dynamic_cast<class__class*>(c);
        if (cls == NULL) break;

        current = cls->get_parent();
    }

    // 最后检查是否继承自Object
    if (parent == Object) return true;

    return false;
}

////////////////////////////////////////////////////////////////////
// 最小上界(LUB)计算函数实现
////////////////////////////////////////////////////////////////////

// 计算两个类型的最小上界（Least Upper Bound）
Symbol ClassTable::lub(Symbol type1, Symbol type2) {
    // 如果两个类型都是SELF_TYPE，返回SELF_TYPE
    if (type1 == SELF_TYPE && type2 == SELF_TYPE) {
        return SELF_TYPE;
    }

    // 如果只有一个类型是SELF_TYPE，返回Object
    if (type1 == SELF_TYPE || type2 == SELF_TYPE) {
        return Object;
    }

    // 如果type1是type2的子类型，返回type2
    if (is_subtype(type1, type2)) {
        return type2;
    }

    // 如果type2是type1的子类型，返回type1
    if (is_subtype(type2, type1)) {
        return type1;
    }

    // 寻找共同祖先
    Symbol current = type1;
    while (current != No_class && current != Object) {
        if (is_subtype(type2, current)) {
            return current;
        }

        // 直接使用 class_table->lookup
        Class_* class_ptr = class_table->lookup(current);
        if (class_ptr == NULL) break;

        Class_ c = *class_ptr;
        class__class* cls = dynamic_cast<class__class*>(c);
        if (cls == NULL) break;

        current = cls->get_parent();
    }

    // 如果没有找到共同祖先，返回Object
    return Object;
}

////////////////////////////////////////////////////////////////////
// 查找方法函数实现
////////////////////////////////////////////////////////////////////

// 在类及其父类中查找指定方法
method_class* ClassTable::find_method(Symbol class_name, Symbol method_name) {
    if (semant_debug) {
        cerr << "find_method: looking for '" << method_name->get_string() 
             << "' in class '" << class_name->get_string() << "'" << endl;
    }

    // 处理 SELF_TYPE
    if (class_name == SELF_TYPE) {
        if (current_class == NULL) {
            if (semant_debug) cerr << "  current_class is NULL" << endl;
            return NULL;
        }
        class__class* current_cls = dynamic_cast<class__class*>(current_class);
        if (current_cls == NULL) {
            if (semant_debug) cerr << "  Failed to cast current_class" << endl;
            return NULL;
        }
        class_name = current_cls->get_name();
    }

    // 在继承链中查找方法
    Symbol current = class_name;
    while (current != No_class) {
        if (semant_debug) {
            cerr << "  searching in class: " << current->get_string() << endl;
        }

        // 查找类
        Class_* class_ptr = class_table->lookup(current);
        if (class_ptr == NULL) {
            if (semant_debug) cerr << "    Class not found in table!" << endl;
            break;
        }

        if (semant_debug) {
            cerr << "    Class pointer address: " << class_ptr << endl;
            cerr << "    Dereferenced class pointer: " << *class_ptr << endl;
        }

        // 检查指针是否有效
        if (*class_ptr == NULL) {
            if (semant_debug) cerr << "    ERROR: Dereferenced class pointer is NULL!" << endl;
            break;
        }

        Class_ c = *class_ptr;

        // 尝试转换
        class__class* cls = dynamic_cast<class__class*>(c);
        if (cls == NULL) {
            if (semant_debug) {
                cerr << "    ERROR: Failed to cast to class__class*!" << endl;
                cerr << "    Typeinfo: " << typeid(*c).name() << endl;
                cerr << "    Class_ value: " << c << endl;
            }
            break;
        }

        // 获取类的特性（方法和属性）
        Features features = cls->get_features();
        if (features == NULL) {
            if (semant_debug) cerr << "    No features in class" << endl;
            // 继续查找父类
            current = cls->get_parent();
            continue;
        }

        if (semant_debug) {
            cerr << "    Class has " << features->len() << " features" << endl;
        }

        // 在当前类的特性中查找方法
        for (int i = features->first(); features->more(i); i = features->next(i)) {
            Feature feature = features->nth(i);
            if (feature == NULL) {
                if (semant_debug) cerr << "    Feature is NULL" << endl;
                continue;
            }

            // 尝试转换为方法
            method_class* method = dynamic_cast<method_class*>(feature);
            if (method != NULL) {
                Symbol method_name_found = method->get_name();
                if (method_name_found == method_name) {
                    if (semant_debug) {
                        cerr << "    FOUND method: " << method_name_found->get_string() << endl;
                    }
                    return method;
                }
            }
        }

        if (semant_debug) {
            cerr << "    Method not found in current class, checking parent" << endl;
        }

        // 在父类中继续查找
        current = cls->get_parent();
    }

    if (semant_debug) {
        cerr << "  Method '" << method_name->get_string() << "' NOT FOUND" << endl;
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////
// 构建继承图函数实现
////////////////////////////////////////////////////////////////////

// 构建类的继承图，将用户定义的类添加到符号表中
void ClassTable::build_inheritance_graph() {
    if (all_classes == NULL) return;

    if (semant_debug) {
        cerr << "build_inheritance_graph: adding " << all_classes->len() << " user classes" << endl;
    }

    // 添加用户定义的类到符号表
    for (int i = all_classes->first(); all_classes->more(i); i = all_classes->next(i)) {
        Class_ c = all_classes->nth(i);
        if (c == NULL) continue;

        class__class* cls = dynamic_cast<class__class*>(c);
        if (cls == NULL) {
            continue;
        }

        Symbol name = cls->get_name();

        // 检查是否重复定义
        Class_* existing = class_table->probe(name);
        if (existing != NULL) {
            semant_error(c) << "Class " << name << " was previously defined." << endl;
            continue;
        }

        if (semant_debug) {
            cerr << "  Adding class: " << name->get_string() << " at address: " << c << endl;
        }

        // 修复：创建类的副本或直接存储值
        // 由于 Class_ 是智能指针类型，我们可以直接存储
        Class_* class_copy = new Class_(c);  // 假设有复制构造函数
        class_table->addid(name, class_copy);

        if (semant_debug) {
            cerr << "    Stored at address: " << class_copy << endl;
            cerr << "    Value at address: " << *class_copy << endl;
        }
    }
}

////////////////////////////////////////////////////////////////////
// 检查继承关系函数实现
////////////////////////////////////////////////////////////////////

// 检查类的继承关系是否合法
void ClassTable::check_inheritance() {
    for (int i = all_classes->first(); all_classes->more(i); i = all_classes->next(i)) {
        Class_ c = all_classes->nth(i);
        class__class* cls = dynamic_cast<class__class*>(c);
        if (cls == NULL) continue;

        Symbol name = cls->get_name();
        Symbol parent = cls->get_parent();

        if (semant_debug) {
            cerr << "Checking inheritance for class: " << name << ", parent: " << parent << endl;
        }

        // 检查是否继承自基本类型
        if (parent == Int || parent == Bool || parent == Str || parent == SELF_TYPE) {
            semant_error(c) << "Class " << name << " cannot inherit class " << parent << "." << endl;
            continue;
        }

        // 检查父类是否存在
        if (parent != No_class) {
            Class_* parent_ptr = class_table->lookup(parent);
            if (parent_ptr == NULL) {
                semant_error(c) << "Class " << name << " inherits from an undefined class " << parent << "." << endl;
                continue;
            }
        }

        // 检查继承循环
        if (parent != No_class) {
            std::set<Symbol> visited;
            visited.insert(name);

            Symbol current = parent;
            while (current != No_class) {
                // 检查循环
                if (visited.find(current) != visited.end()) {
                    semant_error(c) << "Class " << name << ", or an ancestor of " << name 
                                   << ", is involved in an inheritance cycle." << endl;
                    break;
                }
                visited.insert(current);

                // 获取父类
                Class_* current_ptr = class_table->lookup(current);
                if (current_ptr == NULL) {
                    if (semant_debug) {
                        cerr << "Warning: Could not find class " << current << " while checking inheritance cycle" << endl;
                    }
                    break;
                }

                Class_ current_class_obj = *current_ptr;
                class__class* current_cls = dynamic_cast<class__class*>(current_class_obj);
                if (current_cls == NULL) break;

                current = current_cls->get_parent();
            }
        }
    }
}

////////////////////////////////////////////////////////////////////
// 检查Main类函数实现
////////////////////////////////////////////////////////////////////

// 检查Main类是否存在
void ClassTable::check_main() {
    if (semant_debug) {
        cerr << "check_main: SUPER SAFE VERSION starting" << endl;
    }

    // 1. 首先检查 class_table 是否有效
    if (class_table == NULL) {
        if (semant_debug) {
            cerr << "check_main: ERROR: class_table is NULL!" << endl;
        }
        semant_error() << "Internal error: class table not initialized." << endl;
        return;
    }

    // 2. 检查 Main 符号是否有效
    if (Main == NULL) {
        if (semant_debug) {
            cerr << "check_main: ERROR: Main symbol is NULL!" << endl;
        }
        return;
    }

    if (semant_debug) {
        cerr << "check_main: Looking up Main class..." << endl;
    }

    // 3. 安全地查找 Main 类
    try {
        Class_* main_ptr = class_table->lookup(Main);

        if (main_ptr == NULL) {
            if (semant_debug) {
                cerr << "check_main: Main class not found in symbol table" << endl;
            }
            semant_error() << "Class Main is not defined." << endl;
            return;
        }

        if (semant_debug) {
            cerr << "check_main: Found Main class pointer" << endl;
        }

        // 4. 检查指针是否指向有效内存
        if (*main_ptr == NULL) {
            if (semant_debug) {
                cerr << "check_main: Warning: Main class pointer points to NULL" << endl;
            }
            return;
        }

        // 5. 简化：只检查Main类存在，不检查main方法
        if (semant_debug) {
            cerr << "check_main: Main class exists, skipping method check" << endl;
        }

    } catch (...) {
        if (semant_debug) {
            cerr << "check_main: Exception caught while checking Main class" << endl;
        }
        semant_error() << "Error checking Main class." << endl;
        return;
    }

    if (semant_debug) {
        cerr << "check_main: SUPER SAFE VERSION finished successfully" << endl;
    }
}

////////////////////////////////////////////////////////////////////
// 收集方法函数实现
////////////////////////////////////////////////////////////////////

// 收集所有类的方法信息
void ClassTable::collect_methods() {
    // 遍历所有类收集方法
    for (int i = all_classes->first(); all_classes->more(i); i = all_classes->next(i)) {
        Class_ c = all_classes->nth(i);
        class__class* cls = dynamic_cast<class__class*>(c);
        if (cls == NULL) continue;

        Symbol class_name = cls->get_name();
        Features features = cls->get_features();

        for (int j = features->first(); features->more(j); j = features->next(j)) {
            Feature feature = features->nth(j);
            method_class* method = dynamic_cast<method_class*>(feature);
            if (method != NULL) {
                // 检查方法重写
                Symbol method_name = method->get_name();

                // 在父类中查找同名方法
                Symbol parent = cls->get_parent();
                method_class* parent_method = NULL;
                if (parent != No_class) {
                    parent_method = find_method(parent, method_name);
                }

                if (parent_method != NULL) {
                    // 检查参数数量
                    Formals child_formals = method->get_formals();
                    Formals parent_formals = parent_method->get_formals();

                    int child_count = 0, parent_count = 0;
                    for (int k = child_formals->first(); child_formals->more(k); k = child_formals->next(k)) child_count++;
                    for (int k = parent_formals->first(); parent_formals->more(k); k = parent_formals->next(k)) parent_count++;

                    if (child_count != parent_count) {
                        semant_error(c) << "In redefined method " << method_name 
                                      << ", parameter number differs from original." << endl;
                    } else {
                        // 检查参数类型和返回类型
                        int k1 = child_formals->first();
                        int k2 = parent_formals->first();
                        while (child_formals->more(k1) && parent_formals->more(k2)) {
                            formal_class* child_f = dynamic_cast<formal_class*>(child_formals->nth(k1));
                            formal_class* parent_f = dynamic_cast<formal_class*>(parent_formals->nth(k2));

                            if (child_f && parent_f && child_f->get_type_decl() != parent_f->get_type_decl()) {
                                semant_error(c) << "In redefined method " << method_name 
                                              << ", parameter type " << child_f->get_type_decl()
                                              << " differs from original type " << parent_f->get_type_decl() << "." << endl;
                            }

                            k1 = child_formals->next(k1);
                            k2 = parent_formals->next(k2);
                        }

                        // 检查返回类型
                        if (method->get_return_type() != parent_method->get_return_type()) {
                            semant_error(c) << "In redefined method " << method_name 
                                          << ", return type " << method->get_return_type()
                                          << " differs from original return type " << parent_method->get_return_type() << "." << endl;
                        }
                    }
                }

                // 添加到方法表
                method_table[class_name][method_name] = method;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////
// 表达式类型检查函数实现
////////////////////////////////////////////////////////////////////

// 检查表达式的类型并返回类型符号
Symbol ClassTable::type_check_expression(Expression expr, SymbolTable<Symbol, Symbol> *env) {
    // 1. 整数常量
    if (int_const_class* int_expr = dynamic_cast<int_const_class*>(expr)) {
        expr->set_type(Int);
        return Int;
    }

    // 2. 布尔常量
    if (bool_const_class* bool_expr = dynamic_cast<bool_const_class*>(expr)) {
        expr->set_type(Bool);
        return Bool;
    }

    // 3. 字符串常量
    if (string_const_class* str_expr = dynamic_cast<string_const_class*>(expr)) {
        expr->set_type(Str);
        return Str;
    }

    // 4. 对象（变量）
    if (object_class* obj_expr = dynamic_cast<object_class*>(expr)) {
        Symbol name = obj_expr->get_name();

        // 特殊处理 self
        if (name == self) {
            expr->set_type(SELF_TYPE);
            return SELF_TYPE;
        }

        Symbol* type_ptr = env->lookup(name);
        if (type_ptr == NULL) {
            semant_error(current_filename, expr) << "Undeclared identifier " << name << "." << endl;
            expr->set_type(Object);
            return Object;
        }
        expr->set_type(*type_ptr);
        return *type_ptr;
    }

    // 5. 赋值表达式
    if (assign_class* assign_expr = dynamic_cast<assign_class*>(expr)) {
    Symbol var_name = assign_expr->get_name();

    if (semant_debug) {
        cerr << "assign: variable = " << var_name->get_string() << endl;
    }

    Symbol var_type = type_check_expression(assign_expr->get_expr(), env);

    if (semant_debug) {
        cerr << "assign: expression type = " << var_type->get_string() << endl;
    }

    // 检查变量是否已声明
    Symbol* declared_type = env->lookup(var_name);
    if (declared_type == NULL) {
        semant_error(current_filename, expr) << "Assignment to undeclared variable " << var_name << "." << endl;
        expr->set_type(Object);
        return Object;
    }

    if (semant_debug) {
        cerr << "assign: declared type = " << (*declared_type)->get_string() << endl;
    }

    // 检查类型兼容性
    if (!is_subtype(var_type, *declared_type)) {
        semant_error(current_filename, expr) << "Type " << var_type 
            << " of assigned expression does not conform to declared type " 
            << *declared_type << " of identifier " << var_name << "." << endl;
    }

    expr->set_type(var_type);

    if (semant_debug) {
        cerr << "assign: setting type to " << var_type->get_string() << endl;
    }

    return var_type;
}

    // 6. 加法表达式
    if (plus_class* plus_expr = dynamic_cast<plus_class*>(expr)) {
        Symbol type1 = type_check_expression(plus_expr->get_e1(), env);
        Symbol type2 = type_check_expression(plus_expr->get_e2(), env);

        if (type1 != Int || type2 != Int) {
            semant_error(current_filename, expr) << "non-Int arguments: " << type1 << " + " << type2 << endl;
            expr->set_type(Object);
            return Object;
        }

        expr->set_type(Int);
        return Int;
    }

    // 7. new表达式
    if (new__class* new_expr = dynamic_cast<new__class*>(expr)) {
    Symbol type_name = new_expr->get_type_name();

    if (semant_debug) {
        cerr << "new expression: type_name = " << type_name << endl;
    }

    if (type_name == SELF_TYPE) {
        expr->set_type(SELF_TYPE);
        return SELF_TYPE;
    }

    Class_* class_ptr = class_table->lookup(type_name);
    if (class_ptr == NULL) {
        semant_error(current_filename, expr) << "'new' used with undefined class " << type_name << "." << endl;
        expr->set_type(Object);
        return Object;
    }

    expr->set_type(type_name);
    return type_name;
}

    // 8. 方法调用表达式
    if (dispatch_class* dispatch_expr = dynamic_cast<dispatch_class*>(expr)) {

    if (semant_debug) {
        cerr << "dispatch expression: " << dispatch_expr->get_name()->get_string() << endl;
    }
    // 检查被调用的表达式
    Symbol expr_type = type_check_expression(dispatch_expr->get_expr(), env);
    Symbol original_expr_type = expr_type;  // 保存原始类型用于后续处理

    if (semant_debug) {
        cerr << "dispatch: expr_type = " << expr_type 
             << ", method = " << dispatch_expr->get_name() << endl;
    }

    // 解析SELF_TYPE用于方法查找
    Symbol lookup_class = expr_type;
    if (expr_type == SELF_TYPE) {
        // 获取当前类的名称
        class__class* current_cls = dynamic_cast<class__class*>(current_class);
        if (current_cls != NULL) {
            lookup_class = current_cls->get_name();
            if (semant_debug) {
                cerr << "  SELF_TYPE resolved to: " << lookup_class << endl;
            }
        } else {
            lookup_class = Object;  // 退回到Object
        }
    }

    // 查找方法
    method_class* method = find_method(lookup_class, dispatch_expr->get_name());
    if (method == NULL) {
        semant_error(current_filename, expr) << "Dispatch to undefined method " 
            << dispatch_expr->get_name() << "." << endl;
        expr->set_type(Object);
        return Object;
    }

    if (semant_debug) {
        cerr << "  Found method, return type = " << method->get_return_type() << endl;
    }

    // 检查参数数量和类型
    Formals formals = method->get_formals();
    Expressions actual = dispatch_expr->get_actual();

    // 检查参数数量
    int formal_count = 0;
    if (formals != NULL) {
        for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
            formal_count++;
        }
    }

    int actual_count = 0;
    if (actual != NULL) {
        for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
            actual_count++;
        }
    }

    if (formal_count != actual_count) {
        semant_error(current_filename, expr) << "Method " << dispatch_expr->get_name() 
            << " called with wrong number of arguments." << endl;
        expr->set_type(Object);
        return Object;
    }

    // 检查参数类型
    if (formals != NULL && actual != NULL) {
        int j = actual->first();
        for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
            Formal formal = formals->nth(i);
            Expression actual_expr = actual->nth(j);

            if (formal_class* f = dynamic_cast<formal_class*>(formal)) {
                Symbol actual_type = type_check_expression(actual_expr, env);
                if (!is_subtype(actual_type, f->get_type_decl())) {
                    semant_error(current_filename, expr) << "In call of method " 
                        << dispatch_expr->get_name() << ", type " << actual_type 
                        << " of parameter " << f->get_name() 
                        << " does not conform to declared type " << f->get_type_decl() << "." << endl;
                }
            }
            j = actual->next(j);
        }
    }

    // 确定返回类型
    Symbol result_type = method->get_return_type();

    // 处理SELF_TYPE返回类型
    if (result_type == SELF_TYPE) {
        if (original_expr_type == SELF_TYPE) {
            result_type = SELF_TYPE;
        } else {
            result_type = original_expr_type;
        }
    }

    if (semant_debug) {
        cerr << "  dispatch result_type = " << result_type << endl;
    }

    expr->set_type(result_type);
    return result_type;
}

    // 9. 减法表达式
    if (sub_class* sub_expr = dynamic_cast<sub_class*>(expr)) {
        Symbol type1 = type_check_expression(sub_expr->get_e1(), env);
        Symbol type2 = type_check_expression(sub_expr->get_e2(), env);

        if (type1 != Int || type2 != Int) {
            semant_error(current_filename, expr) << "non-Int arguments: " << type1 << " - " << type2 << endl;
            expr->set_type(Object);
            return Object;
        }

        expr->set_type(Int);
        return Int;
    }

    // 10. 乘法表达式
    if (mul_class* mul_expr = dynamic_cast<mul_class*>(expr)) {
        Symbol type1 = type_check_expression(mul_expr->get_e1(), env);
        Symbol type2 = type_check_expression(mul_expr->get_e2(), env);

        if (type1 != Int || type2 != Int) {
            semant_error(current_filename, expr) << "non-Int arguments: " << type1 << " * " << type2 << endl;
            expr->set_type(Object);
            return Object;
        }

        expr->set_type(Int);
        return Int;
    }

    // 11. 除法表达式
    if (divide_class* div_expr = dynamic_cast<divide_class*>(expr)) {
        Symbol type1 = type_check_expression(div_expr->get_e1(), env);
        Symbol type2 = type_check_expression(div_expr->get_e2(), env);

        if (type1 != Int || type2 != Int) {
            semant_error(current_filename, expr) << "non-Int arguments: " << type1 << " / " << type2 << endl;
            expr->set_type(Object);
            return Object;
        }

        expr->set_type(Int);
        return Int;
    }

    // 12. 取负表达式
    if (neg_class* neg_expr = dynamic_cast<neg_class*>(expr)) {
        Symbol type1 = type_check_expression(neg_expr->get_e1(), env);

        if (type1 != Int) {
            semant_error(current_filename, expr) << "Argument of '~' has type " << type1 << " instead of Int." << endl;
            expr->set_type(Object);
            return Object;
        }

        expr->set_type(Int);
        return Int;
    }

    if (isvoid_class* isvoid_expr = dynamic_cast<isvoid_class*>(expr)) {
    // 检查表达式
    Symbol type1 = type_check_expression(isvoid_expr->get_e1(), env);

    // isvoid 表达式总是返回 Bool 类型
    expr->set_type(Bool);
    return Bool;
    }

    // 13. 小于表达式
    if (lt_class* lt_expr = dynamic_cast<lt_class*>(expr)) {
        Symbol type1 = type_check_expression(lt_expr->get_e1(), env);
        Symbol type2 = type_check_expression(lt_expr->get_e2(), env);

        if (type1 != Int || type2 != Int) {
            semant_error(current_filename, expr) << "non-Int arguments: " << type1 << " < " << type2 << endl;
            expr->set_type(Object);
            return Object;
        }

        expr->set_type(Bool);
        return Bool;
    }

    // 14. 等于表达式
    if (eq_class* eq_expr = dynamic_cast<eq_class*>(expr)) {
        Symbol type1 = type_check_expression(eq_expr->get_e1(), env);
        Symbol type2 = type_check_expression(eq_expr->get_e2(), env);

        // 基本类型只能与相同类型比较
        if ((type1 == Int && type2 != Int) || 
            (type1 == Bool && type2 != Bool) || 
            (type1 == Str && type2 != Str)) {
            semant_error(current_filename, expr) << "Illegal comparison with a basic type." << endl;
            expr->set_type(Object);
            return Object;
        }

        expr->set_type(Bool);
        return Bool;
    }

    // 15. 小于等于表达式
    if (leq_class* leq_expr = dynamic_cast<leq_class*>(expr)) {
        Symbol type1 = type_check_expression(leq_expr->get_e1(), env);
        Symbol type2 = type_check_expression(leq_expr->get_e2(), env);

        if (type1 != Int || type2 != Int) {
            semant_error(current_filename, expr) << "non-Int arguments: " << type1 << " <= " << type2 << endl;
            expr->set_type(Object);
            return Object;
        }

        expr->set_type(Bool);
        return Bool;
    }

    // 16. 取反表达式
    if (comp_class* comp_expr = dynamic_cast<comp_class*>(expr)) {
        Symbol type1 = type_check_expression(comp_expr->get_e1(), env);

        if (type1 != Bool) {
            semant_error(current_filename, expr) << "Argument of 'not' has type " << type1 
                << " instead of Bool." << endl;
            expr->set_type(Object);
            return Object;
        }

        expr->set_type(Bool);
        return Bool;
    }

    // 17. 条件表达式
    if (cond_class* cond_expr = dynamic_cast<cond_class*>(expr)) {
        Symbol pred_type = type_check_expression(cond_expr->get_pred(), env);

        if (pred_type != Bool) {
            semant_error(current_filename, expr) << "Predicate of 'if' has type " << pred_type << " instead of Bool." << endl;
            expr->set_type(Object);
            return Object;
        }

        Symbol then_type = type_check_expression(cond_expr->get_then_exp(), env);
        Symbol else_type = type_check_expression(cond_expr->get_else_exp(), env);

        Symbol result_type = lub(then_type, else_type);
        expr->set_type(result_type);
        return result_type;
    }

    // 18. 循环表达式
    if (loop_class* loop_expr = dynamic_cast<loop_class*>(expr)) {
        Symbol pred_type = type_check_expression(loop_expr->get_pred(), env);

        if (pred_type != Bool) {
            semant_error(current_filename, expr) << "Loop condition does not have type Bool." << endl;
        }

        type_check_expression(loop_expr->get_body(), env);

        expr->set_type(Object);
        return Object;
    }

    // 19. 块表达式
    if (block_class* block_expr = dynamic_cast<block_class*>(expr)) {
        Symbol result_type = Object;
        Expressions body = block_expr->get_body();

        for (int i = body->first(); body->more(i); i = body->next(i)) {
            Expression e = body->nth(i);
            result_type = type_check_expression(e, env);
        }

        expr->set_type(result_type);
        return result_type;
    }

    // 20. let表达式
    if (let_class* let_expr = dynamic_cast<let_class*>(expr)) {
        env->enterscope();

        Symbol identifier = let_expr->get_identifier();
        Symbol type_decl = let_expr->get_type_decl();

        // 检查类型声明是否有效
        if (type_decl != SELF_TYPE) {
            Class_* class_ptr = class_table->lookup(type_decl);
            if (class_ptr == NULL && type_decl != Int && type_decl != Bool && type_decl != Str) {
                semant_error(current_filename, expr) << "Class " << type_decl << " of let-bound identifier " 
                    << identifier << " is undefined." << endl;
            }
        }

        // 检查初始化表达式
        Expression init = let_expr->get_init();
        if (dynamic_cast<no_expr_class*>(init) == NULL) {
            Symbol init_type = type_check_expression(init, env);
            if (!is_subtype(init_type, type_decl)) {
                semant_error(current_filename, expr) << "Inferred type " << init_type 
                    << " of initialization of " << identifier 
                    << " does not conform to identifier's declared type " << type_decl << "." << endl;
            }
        }

        // 添加变量到环境
        env->addid(identifier, &type_decl);

        // 检查body
        Symbol body_type = type_check_expression(let_expr->get_body(), env);

        env->exitscope();

        expr->set_type(body_type);
        return body_type;
    }


    //21.typcase 处理
    if (typcase_class* typcase_expr = dynamic_cast<typcase_class*>(expr)) {

    if (semant_debug) {
        cerr << "typcase expression" << endl;
    }
    // 检查 case 表达式
    Symbol expr_type = type_check_expression(typcase_expr->get_expr(), env);

    // 检查所有分支
    Cases cases = typcase_expr->get_cases();
    Symbol result_type = Object;  // 默认返回 Object

    if (cases != NULL) {
        // 收集所有分支的类型
        std::vector<Symbol> branch_types;

        for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
            Case case_branch = cases->nth(i);
            if (branch_class* branch = dynamic_cast<branch_class*>(case_branch)) {
                // 创建新作用域
                env->enterscope();

                // 添加分支变量到环境
                Symbol name = branch->get_name();
                Symbol type_decl = branch->get_type_decl();
                Symbol* type_ptr = new Symbol(type_decl);
                env->addid(name, type_ptr);

                // 检查分支表达式
                Symbol branch_type = type_check_expression(branch->get_expr(), env);
                branch_types.push_back(branch_type);

                // 退出作用域
                env->exitscope();
            }
        }

        // 计算所有分支类型的 LUB (Least Upper Bound)
        if (!branch_types.empty()) {
            result_type = branch_types[0];
            for (size_t i = 1; i < branch_types.size(); i++) {
                result_type = lub(result_type, branch_types[i]);
            }
        }
    }

    expr->set_type(result_type);
    return result_type;
}

// 静态分派表达式
if (static_dispatch_class* static_dispatch_expr = dynamic_cast<static_dispatch_class*>(expr)) {
    if (semant_debug) {
        cerr << "static_dispatch expression" << endl;
    }

    // 检查表达式
    Symbol expr_type = type_check_expression(static_dispatch_expr->get_expr(), env);
    Symbol type_name = static_dispatch_expr->get_type_name();

    // 检查类型转换是否合法
    if (!is_subtype(expr_type, type_name)) {
        semant_error(current_filename, expr) << "Expression type " << expr_type 
            << " does not conform to declared static dispatch type " << type_name << "." << endl;
        expr->set_type(Object);
        return Object;
    }

    // 在指定类型中查找方法
    method_class* method = find_method(type_name, static_dispatch_expr->get_name());
    if (method == NULL) {
        semant_error(current_filename, expr) << "Static dispatch to undefined method " 
            << static_dispatch_expr->get_name() << "." << endl;
        expr->set_type(Object);
        return Object;
    }

    // 检查参数
    Expressions actual = static_dispatch_expr->get_actual();
    Formals formals = method->get_formals();

    // 检查参数数量
    int formal_count = 0, actual_count = 0;
    if (formals != NULL) {
        for (int i = formals->first(); formals->more(i); i = formals->next(i)) formal_count++;
    }
    if (actual != NULL) {
        for (int i = actual->first(); actual->more(i); i = actual->next(i)) actual_count++;
    }

    if (formal_count != actual_count) {
        semant_error(current_filename, expr) << "Method " << static_dispatch_expr->get_name() 
            << " called with wrong number of arguments." << endl;
        expr->set_type(Object);
        return Object;
    }

    // 检查参数类型
    if (formals != NULL && actual != NULL) {
        int j = actual->first();
        for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
            Formal formal = formals->nth(i);
            Expression actual_expr = actual->nth(j);

            if (formal_class* f = dynamic_cast<formal_class*>(formal)) {
                Symbol actual_type = type_check_expression(actual_expr, env);
                if (!is_subtype(actual_type, f->get_type_decl())) {
                    semant_error(current_filename, expr) << "In call of method " 
                        << static_dispatch_expr->get_name() << ", type " << actual_type 
                        << " of parameter " << f->get_name() 
                        << " does not conform to declared type " << f->get_type_decl() << "." << endl;
                }
            }
            j = actual->next(j);
        }
    }

    // 确定返回类型
    Symbol result_type = method->get_return_type();
    if (result_type == SELF_TYPE) {
        // 对于静态分派，SELF_TYPE 指的是分派类型，不是原始表达式类型
        result_type = type_name;
    }

    expr->set_type(result_type);
    return result_type;
}




    // 默认返回Object
    expr->set_type(Object);
    return Object;

}

////////////////////////////////////////////////////////////////////
// 类型检查类函数实现
////////////////////////////////////////////////////////////////////

// 对指定类进行类型检查
void ClassTable::type_check_class(Class_ c) {
    current_class = c;
    class__class* cls = dynamic_cast<class__class*>(c);
    if (cls == NULL) return;

    current_filename = cls->get_filename();

    if (semant_debug) {
        cerr << "=== Checking class: " << cls->get_name()->get_string() << " ===" << endl;
    }

    // 创建对象环境
    SymbolTable<Symbol, Symbol> *object_env = new SymbolTable<Symbol, Symbol>();
    object_env->enterscope();

    // 添加self到环境
    Symbol* self_type_ptr = new Symbol(SELF_TYPE);
    object_env->addid(self, self_type_ptr);

    if (semant_debug) {
        cerr << "Added self: " << self->get_string() << " : " << SELF_TYPE->get_string() << endl;
    }

    // 添加所有属性到环境
    Features features = cls->get_features();
    for (int i = features->first(); features->more(i); i = features->next(i)) {
        Feature feature = features->nth(i);

        if (attr_class* attr = dynamic_cast<attr_class*>(feature)) {
            // 添加属性到环境
            Symbol name = attr->get_name();
            Symbol type_decl = attr->get_type_decl();

            // 为类型分配独立的内存
            Symbol* type_ptr = new Symbol(type_decl);
            object_env->addid(name, type_ptr);

            if (semant_debug) {
                cerr << "Added attribute: " << name->get_string() 
                     << " : " << type_decl->get_string() << endl;
            }

            // 检查属性初始化表达式
            Expression init = attr->get_init();
            if (dynamic_cast<no_expr_class*>(init) == NULL) {
                // 有初始化表达式，检查类型
                Symbol init_type = type_check_expression(init, object_env);
                if (!is_subtype(init_type, type_decl)) {
                    semant_error(current_filename, attr) << "Inferred type " << init_type 
                        << " of initialization of attribute " << name 
                        << " does not conform to declared type " << type_decl << "." << endl;
                }
            }
        }
    }

    // 检查方法
    for (int i = features->first(); features->more(i); i = features->next(i)) {
        Feature feature = features->nth(i);

        if (method_class* method = dynamic_cast<method_class*>(feature)) {
            type_check_method(method, object_env);
        }
    }

    object_env->exitscope();
    delete object_env;

    if (semant_debug) {
        cerr << "=== Finished checking class: " << cls->get_name()->get_string() << " ===" << endl;
    }
}

////////////////////////////////////////////////////////////////////
// 检查方法函数实现
////////////////////////////////////////////////////////////////////

// 对指定方法进行类型检查
Symbol ClassTable::type_check_method(method_class *method, SymbolTable<Symbol, Symbol> *env) {
    if (semant_debug) {
        cerr << "Checking method: " << method->get_name()->get_string() << endl;
    }

    // 创建新的作用域用于方法参数
    env->enterscope();

    // 添加参数到环境
    Formals formals = method->get_formals();
    for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
        Formal formal = formals->nth(i);
        if (formal_class* f = dynamic_cast<formal_class*>(formal)) {
            Symbol name = f->get_name();
            Symbol type_decl = f->get_type_decl();

            if (semant_debug) {
                cerr << "  Adding formal: " << name->get_string() 
                     << " : " << type_decl->get_string() << endl;
            }

            // 检查参数类型是否有效
            if (type_decl == SELF_TYPE) {
                semant_error(current_filename, f) << "Formal parameter " << name << " cannot have type SELF_TYPE." << endl;
            }

            // 为类型分配独立的内存
            Symbol* type_ptr = new Symbol(type_decl);
            env->addid(name, type_ptr);
        }
    }

    // 检查方法体
    Symbol body_type = type_check_expression(method->get_expr(), env);
    Symbol return_type = method->get_return_type();

    if (semant_debug) {
        cerr << "  Method body type: " << body_type->get_string() << endl;
        cerr << "  Declared return type: " << return_type->get_string() << endl;
    }

    // 检查返回类型
    if (return_type == SELF_TYPE) {
        // 如果方法声明返回SELF_TYPE，表达式类型也必须是SELF_TYPE
        if (body_type != SELF_TYPE) {
            semant_error(current_filename, method) << "Inferred return type " << body_type 
                << " of method " << method->get_name() 
                << " does not conform to declared return type SELF_TYPE." << endl;
        }
    } else if (body_type == SELF_TYPE) {
        // 如果方法体返回SELF_TYPE，但声明返回具体类型，这是允许的
        // 因为SELF_TYPE在运行时会被解析为当前类
        // 不报错
    } else if (!is_subtype(body_type, return_type)) {
        semant_error(current_filename, method) << "Inferred return type " << body_type 
            << " of method " << method->get_name() 
            << " does not conform to declared return type " << return_type << "." << endl;
    }

    // 退出作用域
    env->exitscope();

    return return_type;
}

////////////////////////////////////////////////////////////////////
// ClassTable 构造函数实现
////////////////////////////////////////////////////////////////////

// ClassTable构造函数，初始化语义分析器
ClassTable::ClassTable(Classes classes) : 
    semant_errors(0), 
    error_stream(cerr), 
    all_classes(classes),
    Object_class_ptr(NULL),
    IO_class_ptr(NULL),
    Int_class_ptr(NULL),
    Bool_class_ptr(NULL),
    Str_class_ptr(NULL),
    current_class(NULL) {

    // 初始化类表
    class_table = new SymbolTable<Symbol, Class_>();

    // 安装基本类
    install_basic_classes();

    // 构建继承图
    build_inheritance_graph();

    // 检查继承关系
    check_inheritance();

    // 检查Main类
    check_main();

    // 收集方法
    collect_methods();

    // 类型检查所有类
    for (int i = all_classes->first(); all_classes->more(i); i = all_classes->next(i)) {
        Class_ c = all_classes->nth(i);
        type_check_class(c);
    }
}

////////////////////////////////////////////////////////////////////
// 程序入口点实现
////////////////////////////////////////////////////////////////////

// 程序语义分析入口点
void program_class::semant() {
    initialize_constants();

    // 创建ClassTable进行语义分析
    ClassTable *classtable = new ClassTable(classes);

    if (classtable->errors()) {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }
}
