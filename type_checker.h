
#ifndef _TYPE_CHECKER_
#define _TYPE_CHECKER_

#include <iostream>
#include <set>
#include <string>
#include "astree.h"
#include "lyutils.h"
#define VOID_TYPE "void"
#define INT_TYPE "int"
#define STRING_TYPE "string"
#define TOK_STRUCT "TOK_STRUCT"
#define TOK_IDENT "TOK_IDENT"
#define TOK_ARRAY "TOK_ARRAY"
#define TOK_POS "TOK_POS"
#define TOK_NEG "TOK_NEG"
#define TOK_NEW "TOK_NEW"
#define TOK_TYPEID "TOK_TYPEID"
#define TOK_NEWARRAY "TOK_NEWARRAY"
#define TOK_NEWSTRING "TOK_NEWSTRING"
#define TOK_FUNCTION "TOK_FUNCTION"
#define TOK_BLOCK "TOK_BLOCK"
#define TOK_VARDECL "TOK_VARDECL"
#define TOK_DECLID "TOK_DECLID"
#define TOK_WHILE "TOK_WHILE"
#define TOK_IFELSE "TOK_IFELSE"
#define TOK_IF "TOK_IF"
#define TOK_RETURN "TOK_RETURN"
#define TOK_RETURNVOID "TOK_RETURNVOID"
#define TOK_SEMICOLON ";"
#define TOK_DOT "."
#define TOK_BRACE "["
#define TOK_CALL "TOK_CALL"
#define TOK_INTCON "TOK_INTCON"
#define TOK_STRINGCON "TOK_STRINGCON"
#define TOK_CHARCON "TOK_CHARCON"
#define TOK_INT "TOK_INT"
#define TOK_CHAR "TOK_CHAR"
#define TOK_STRING "TOK_STRING"
#define TOK_NULL "TOK_NULL"
#define TOK_EQ "="
#define PROTOTYPE 0
#define VARIABLE 1

struct type_checker{
  static string current_sym_ident;
  static string current_return_type;
  static bool isReturnArr;
  static size_t current_block_nr;
  static symbol_table type_name_table;
  static set<string> primitive_types;
  static set<string> overloaded_ops;
  static set<string> non_overloaded_ops;
  static set<string> allocator_toks;
  static vector<symbol_table*> symbol_stack;
  static void traverse(astree* root);
  static void initialize_name_table();
  static void handle_struct_defs(astree* node);
  static bool is_struct_complete(symbol* s);
  static void handle_var_defs(astree* node, 
       symbol_table* field_table, bool isArr, bool isVar); //
  static void fill_in_fields(const string* new_typeid);
  static symbol* find_field(symbol* struct_type, const string* field );
  static void handle_block(astree* node, symbol_table parameters);
  static void handle_vardecl(astree* node);
  static void handle_statement(astree* node);
  static void handle_allocator(astree* node);
  static void handle_variable(astree* node);
  static void handle_constant(astree* node);
  static void handle_expr(astree* node);
  static symbol* check_function_type(astree* node, 
         vector<symbol*>* parameter_vec, bool isArr); //
  static symbol_table initialize_parameters(astree* node,  
          vector<symbol*>*& param_vec); //
  static void handle_function(astree* node);
  static void handle_call(astree* node);
  static symbol* isFunctionDefined(const string* ident);
  static bool compare_types(int what_attribute, symbol* sym);
  static bool compatible_types(astree* left, astree* right);
  static symbol* isVariableDefined(const string* ident);
  static bool isEquivalent(const symbol* prototype, 
              const symbol* func_def, int type); //
};

struct sym_args{
  string ident;
  attr_bitset sym;
  FILE* sym_file;
  string identation;
  size_t filenr, linenr, offset;
  size_t block_nr;
  string father_name;
  string type;
};
void output_sym_line(sym_args args);
void output_attributes(astree* tree);
#endif


