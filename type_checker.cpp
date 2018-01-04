#include <iostream>
#include "type_checker.h"

void output_sym_line(symbol* sym_node, string ident, 
            const string* symbol_name){ //
  if(sym_node == nullptr || symbol_name == nullptr)
      throw "null arguments";
  FILE* sym_file = tok_str_file_inter::sym_file_;
  if(sym_file == nullptr) throw "file is closed";
  enumString enums;
  string sym_name = *(symbol_name);
  fprintf(sym_file,"%s%s  (%zd.%zd.%zd)  {%zd} ", ident.c_str(), 
       sym_name.c_str(), sym_node->filenr,
          sym_node->linenr, sym_node->offset,
       sym_node->block_nr);

  for(int attr = ATTR_void; attr < ATTR_bitset_size; ++attr){
      if(sym_node == nullptr) throw "sym is null";
     if(sym_node->attributes[attr] == 1){
       fprintf(sym_file,
           "%s ", enums.enumStrings[attr]);
       if(attr == ATTR_struct){
       if(sym_node != nullptr)
          fprintf(sym_file,"%s ", sym_node->type_id->c_str());
       }
     }
  }
  fprintf(sym_file, "\n");
}

void output_separator(){
  FILE* sym_file = tok_str_file_inter::sym_file_;
  fprintf(sym_file, "\n");
  fprintf(sym_file, "\n");
}

void output_attributes(attr_bitset attr){
  enumString en;
   for(int i = 0; i < ATTR_bitset_size ; i++){
      if(attr[i] == 1)
           cout << en.enumStrings[i] << endl;
    }
    cout << endl;
}

set<string> type_checker::primitive_types;
symbol_table type_checker::type_name_table;
set<string> type_checker::overloaded_ops;
set<string> type_checker::non_overloaded_ops;
vector<symbol_table*> type_checker::symbol_stack;
set<string> type_checker::allocator_toks;
size_t type_checker::current_block_nr = 0;
string type_checker::current_return_type = VOID_TYPE;
bool type_checker::isReturnArr = false;
string type_checker::current_sym_ident = " ";

void type_checker::initialize_name_table(){
   symbol_table* global_table = new symbol_table();
   type_checker::symbol_stack.push_back(global_table);
   primitive_types.insert(VOID_TYPE);
   primitive_types.insert(INT_TYPE);
   primitive_types.insert(STRING_TYPE);
   overloaded_ops.insert({"=", "==", "!=", "<", "<=", ">",">="});
   non_overloaded_ops.insert({"+", "-", "*", "/", "%", "!"});
   allocator_toks.insert({TOK_NEW,TOK_NEWARRAY,TOK_NEWSTRING});
}

void type_checker::handle_var_defs(astree* node, 
    symbol_table* field_table, bool isArr, bool isVar){ //
    const string* type = node->lexinfo;
    const string tok_type = parser::get_tname(node->symbol);
    int first_child_index =  0;
    int second_child_index = 1; 
    if(tok_type == TOK_ARRAY){
       astree* type_node = new astree();
       *(type_node) = *(node->children.at(first_child_index));
       type_node->children.push_back(
           node->children.at(second_child_index)); //
       handle_var_defs(type_node, field_table,true, isVar);
       node->attributes = type_node->attributes;
       node->block_nr = type_node->block_nr;
       node->sym_node.sym = type_node->sym_node.sym;
     }
    else{
     auto ident_node = (node->children).front();
     const string* identifier = ident_node->lexinfo;
     symbol* new_var = new symbol;
     new_var->filenr = ident_node->lloc.filenr;
     new_var->linenr = ident_node->lloc.linenr;
     new_var->offset = ident_node->lloc.offset;
     new_var->block_nr = current_block_nr;
     if(isVar == false) new_var->attributes.set(ATTR_field);
     else {
       new_var->attributes.set(ATTR_variable);
       new_var->attributes.set(ATTR_lval);
     }
     if(isArr) new_var->attributes.set(ATTR_array);
     auto find_ident = field_table->find(identifier);
     if(find_ident == field_table->end()){
      auto find_type = type_name_table.find(type);
      auto find_prim_type = primitive_types.find(*type);
      if(find_type != type_name_table.end() ||
         find_prim_type != primitive_types.end()){
       if(*type == VOID_TYPE)
         throw "void type for field declarations is not allowed";
       else if (*type == INT_TYPE) 
      new_var->attributes.set(ATTR_int);
       else if (*type == STRING_TYPE){
         new_var->attributes.set(ATTR_string);
       }
       else {new_var->type_id = type;
      new_var->attributes.set(ATTR_struct);
       }
       field_table->insert({identifier,new_var});
      }
      else {
        if(isVar == false){
     new_var->attributes.set(ATTR_struct);
     new_var->isDefined = false;
     new_var->type_id = type;
        field_table->insert({identifier,new_var});
        }
        else throw "type not found";
      }
     }
     else{
       string error = "identifier: " + *(ident_node->lexinfo) +
                   " was already declared in struct ";
       const char* error_c = error.c_str();
       throw error_c;
     }
     ident_node->block_nr = current_block_nr; 
     ident_node->attributes = new_var->attributes;
     ident_node->sym_node.sym = new_var;
    
     node->block_nr = current_block_nr;
     node->attributes = new_var->attributes;
     node->sym_node.sym = new_var;

     if(isVar == false)
       output_sym_line(new_var, "    ", identifier);
     else output_sym_line(new_var,current_sym_ident,identifier);
   }
}

void type_checker::handle_struct_defs(astree* node){
    const string* identifier;
    auto& children = node->children;
    astree* ident_node = children.front();
    identifier = ident_node->lexinfo;
    auto it = type_name_table.find(identifier);
    if(it == type_name_table.end()){
      symbol* new_struct = new symbol;
      new_struct->attributes.set(ATTR_typeid);
      new_struct->attributes.set(ATTR_struct);
      new_struct->filenr = node->lloc.filenr;
      new_struct->linenr = node->lloc.linenr;
      new_struct->offset = node->lloc.offset;
      new_struct->block_nr = current_block_nr;
      new_struct->type_id = identifier;
      new_struct->fields = new symbol_table;
      output_sym_line(new_struct," ",identifier); 
      for(auto it = children.begin() + 1; it != children.end(); ++it)
        handle_var_defs(*it,new_struct->fields, false, false);
      type_name_table.insert({identifier, new_struct});
      ident_node->block_nr = current_block_nr;
      ident_node->attributes = new_struct->attributes;
      ident_node->sym_node.sym = new_struct;
      fill_in_fields(identifier);
      output_separator();
    }
    else{
      string error = "error: the type: " + 
          (*identifier) + " was already declared"; //
      const char* error_c = error.c_str();
      throw error_c;
    }
}

bool type_checker::is_struct_complete(symbol* s){
    auto fields = *(s->fields);
    for(auto it : fields)
        if(it.second->isDefined == false) return false;
    return true;
}

void type_checker::fill_in_fields(const string* new_typeid){
   for(auto& pair : type_name_table){
      if(pair.second->attributes[ATTR_typeid] == 1){
         for(auto& field_pair : *(pair.second->fields)){
        bool isDefined = field_pair.second->isDefined;
        const string* field_typeid = field_pair.second->type_id;
           if(isDefined == false && field_typeid == new_typeid)
          field_pair.second->isDefined = true;
         }
      }
   }
}

void type_checker::handle_allocator(astree* node){
   node->attributes.set(ATTR_vreg);
   string tok_type = parser::get_tname(node->symbol);
   auto& children = node->children;
   if(tok_type == TOK_NEW){
     int type_child = 0;
     const string* type = (children[type_child]->lexinfo);
     auto it = type_name_table.find(type);
     if(it != type_name_table.end()){
       if(is_struct_complete(it->second) == false )
      throw "the struct following new is not complete";
       symbol* dummy_symbol = new symbol();
       dummy_symbol->type_id = type;
       node->attributes.set(ATTR_struct);
       node->sym_node.sym = dummy_symbol;
     }
     else{
      string error = "the type: " + *(children[type_child]->lexinfo)
                  + "has not been defined";
      throw error.c_str();
     }
   }
   else if(tok_type == TOK_NEWARRAY){
      int type_child = 0;
      int expr_child = 1;
      const string* type = children[type_child]->lexinfo;
      auto isTypeid = type_name_table.find(type);
      auto isPrimitive = primitive_types.find(*type);
      if(isTypeid != type_name_table.end() || 
          isPrimitive != primitive_types.end()){ //
        node->attributes.set(ATTR_array);
     if(*type == VOID_TYPE)
           throw "void type for reference types is not allowed";
     else if(*type == INT_TYPE)
        node->attributes.set(ATTR_int);
     else if(*type == STRING_TYPE)
        node->attributes.set(ATTR_string);
     else{
           symbol* dummy_sym = new symbol();
        node->attributes.set(ATTR_struct);
        dummy_sym->type_id = type;
        node->sym_node.sym = dummy_sym;
        }
      }
      else throw "unknown type";
      astree* expr_node = children[expr_child];
      handle_expr(expr_node);
      if(expr_node->attributes[ATTR_int] != 1 ||
      expr_node->attributes[ATTR_array] == 1 ||
      expr_node->attributes[ATTR_vreg] != 1)
         throw "invalid expression in array index"; 
   }
   else if(tok_type == TOK_NEWSTRING){
      node->attributes.set(ATTR_string);
      node->attributes.set(ATTR_vreg);
      astree* expr_node = children[0];
      handle_expr(expr_node);
      if(expr_node->attributes[ATTR_int] != 1)
       throw "expected int type";
      if(expr_node->attributes[ATTR_array] == 1) 
       throw "expression can't be a type array";
      if(expr_node->attributes[ATTR_vreg] != 1)
         throw "invalid expression in array index";
   }
   
}

symbol* type_checker::isFunctionDefined(const string* ident){
   attr_bitset empty_set;
   symbol_table global_table = *(symbol_stack[0]);
   auto func = global_table.find(ident);
   if(func != global_table.end()){
     auto& func_attrs = func->second->attributes;
     if(func_attrs[ATTR_function] == 1)
          return func->second;
   }
   return nullptr;

}

bool type_checker::compare_types(int what_attribute, symbol* sym){
    if(what_attribute == ATTR_null)
     return sym->attributes[ATTR_int] != 1;
    return sym->attributes[what_attribute] == 1;  
}

symbol* type_checker::isVariableDefined(const string* ident){
    for(int block_nr = current_block_nr; block_nr >= 0; --block_nr){
      auto it = symbol_stack[block_nr]->find(ident);
      if(it != symbol_stack[block_nr]->end()){
         if(it->second->attributes[ATTR_function] != 1)
             return it->second;
      }
    }
    return nullptr;
}
void type_checker::handle_call(astree* node){
   auto ident_child = node->children[0];
   const string* ident = ident_child->lexinfo;
   symbol* func_sym = isFunctionDefined(ident);
   if(func_sym != nullptr){
     auto& children = node->children;
     auto& symbol_vec = func_sym->parameters;
     size_t vec_counter = 0;
     for(auto it = children.begin() + 1; it != children.end(); ++it){
       if(vec_counter == symbol_vec->size())
          throw "invalid number of arguments on function";
       handle_expr(*it);
       string token_code = parser::get_tname((*it)->symbol);
       bool result = false;
       if((*it)->attributes[ATTR_int] == 1){
       result = compare_types(ATTR_int,symbol_vec->at(vec_counter));
       }
       else if((*it)->attributes[ATTR_string] == 1){
        result = compare_types(ATTR_string, 
                 symbol_vec->at(vec_counter)); //
       }
       else if((*it)->attributes[ATTR_null] == 1){
         result = compare_types(ATTR_null, symbol_vec->at(vec_counter));
       }
       else{
       symbol* sym = (*it)->sym_node.sym;
       if(sym != nullptr){
            if(symbol_vec->at(vec_counter)->type_id == sym->type_id)
             result = true;
            else result = false;
          }
       else throw ("identifier: " + 
                   *((*it)->lexinfo) + " wasn't found").c_str(); //
       }
       if(!result) throw "incompatible types on function"; 
      ++vec_counter; 
     }
     if(vec_counter < symbol_vec->size())
      throw "invalid number of arguments on function";
       node->attributes = func_sym->attributes;
       node->sym_node.sym = func_sym;
       node->attributes[ATTR_vreg] = 1;
   }
   else 
      throw ("the func: " + *ident + " wasn't found").c_str();
}

symbol* type_checker::find_field(symbol* struct_type, 
             const string* field){ //
  auto typeid_pair = type_name_table.find(struct_type->type_id);
  if(typeid_pair == type_name_table.end())
     throw "type not found";
  symbol* typeid_sym = typeid_pair->second;
  auto field_pair = typeid_sym->fields->find(field);
  if(field_pair == typeid_sym->fields->end())
       return nullptr;
  else return field_pair->second;   
}

void type_checker::handle_variable(astree* node){

    auto find_var = [](const string* ident){
       symbol* sym = nullptr;
       for(int block_nr = current_block_nr; block_nr >= 0; --block_nr){
         auto it = symbol_stack[block_nr]->find(ident);
         if(it != symbol_stack[block_nr]->end())
            sym = it->second;
      }
      return sym;
    };  


    string tok_code = parser::get_tname(node->symbol);
    if(tok_code == TOK_IDENT){
      symbol* sym = find_var(node->lexinfo);
      if(sym == nullptr)
       throw "variable not found";
      node->attributes = sym->attributes;
      node->sym_node.sym = sym;
      node->block_nr = current_block_nr;
    }
    else if(*(node->lexinfo) == TOK_BRACE){
       astree* left_child = node->children[0];
       astree* right_child = node->children[1];
       handle_expr(left_child);
       handle_expr(right_child);
      if(left_child->attributes[ATTR_array] == 0 &&
         left_child->attributes[ATTR_string] == 0)
        throw "variable not an array";
      if(right_child->attributes[ATTR_int] == 0)
        throw "expected expression of type int";
      node->attributes = left_child->attributes;
      node->attributes[ATTR_array] = 0;
      node->attributes[ATTR_vaddr] = 1;
      node->attributes[ATTR_lval] = 1;
      
     if(left_child->attributes[ATTR_array] == 0 &&
        left_child->attributes[ATTR_string] == 1){
        node->attributes[ATTR_string] = 0;
        node->attributes[ATTR_int] = 1;
      } 
    }
    else if(*(node->lexinfo) == TOK_DOT){
      astree* left_child = node->children[0];
      astree* right_child = node->children[1];
      handle_expr(left_child);
      if(left_child->attributes[ATTR_struct] == 0)
        throw "variable not a struct";
      symbol* child_sym = left_child->sym_node.sym;
      if(child_sym == nullptr) 
         throw "associated identifier wasn't found"; //
      auto it = type_name_table.find(child_sym->type_id);
      if(it == type_name_table.end()) 
        {throw "struct symbol wasn't found";} //
      symbol* struct_sym = it->second;
      symbol* struct_field = find_field(struct_sym, 
              right_child->lexinfo); //
      if(struct_field == nullptr)
           throw "field not found";
       right_child->attributes = struct_field->attributes;
       right_child->sym_node.sym = struct_field;
       right_child->block_nr = current_block_nr;
       
       node->block_nr = current_block_nr;
       node->sym_node.sym = struct_field;
       node->attributes = struct_field->attributes;
       node->attributes[ATTR_field] = 0;
       node->attributes[ATTR_vaddr] = 1;
       node->attributes[ATTR_lval] = 1;
    }
}

void type_checker::handle_constant(astree* node){
       string token_code = parser::get_tname(node->symbol);
       node->attributes[ATTR_const] = 1;
       node->attributes[ATTR_vreg] = 1;
       if(token_code == TOK_INTCON){
          node->attributes[ATTR_int] = 1;
       }
       else if(token_code == TOK_STRINGCON){
           node->attributes[ATTR_string] = 1;
       }
       else if(token_code == TOK_CHARCON){
           node->attributes[ATTR_int] = 1;
       }
       else if(token_code == TOK_NULL){
            node->attributes[ATTR_null] = 1;
       }
       else{
          symbol* sym = isVariableDefined(node->lexinfo);
          if(sym != nullptr){
             node->sym_node.sym = sym;
             node->attributes[ATTR_struct] = 1;
          } 
          else throw ("identifier: " + *(node->lexinfo) + 
                      " wasn't found").c_str(); //
       }
}

bool type_checker::compatible_types(astree* left, astree* right){
    vector<int> types = 
     {ATTR_int, ATTR_string, ATTR_struct, ATTR_array}; //
    int isTypeLeft = 0;
    int isTypeRight = 0;

    auto isReference = [](astree* tree){
        return (tree->attributes[ATTR_string] ||
                tree->attributes[ATTR_array] ||
          tree->attributes[ATTR_struct]) == 1;
     };
    
    if(right->attributes[ATTR_null] == 1 ){
     bool isRef = isReference(left);
     if(isRef == false)
         return false;
     return true;
    }


    for(auto it: types){
       isTypeLeft = left->attributes[it];
       isTypeRight = right->attributes[it];
       if((isTypeLeft && isTypeRight) == 1){
          if(left->attributes[ATTR_struct] == 1){
         symbol* sym_left = left->sym_node.sym;
         symbol* sym_right = right->sym_node.sym;
         if(sym_left == nullptr || sym_right == nullptr)
            throw "TYPE NOT SET";
         const string* type_left = sym_left->type_id;
         const string* type_right = sym_right->type_id;
         if(type_left == type_right)
            return true;
         else return false;
       }
       else return true;
       }       
       else if(isTypeLeft != isTypeRight)
       return false;
    }
  return false;
}

void type_checker::handle_expr(astree* node){
  
   auto isCon = [](string tok_type) { 
       return tok_type == TOK_INTCON ||
           tok_type == TOK_STRINGCON ||
              tok_type == TOK_CHARCON ||
           tok_type == TOK_NULL;
    };

   string tok_type = parser::get_tname(node->symbol);
   if(allocator_toks.find(tok_type) != 
      allocator_toks.end())
      handle_allocator(node);
   else if(tok_type == TOK_CALL){
      handle_call(node);
   }
   else if(tok_type == TOK_IDENT ||
       *(node->lexinfo) == TOK_BRACE ||
        *(node->lexinfo) == TOK_DOT)
     handle_variable(node);
   else if(isCon(tok_type) == true)
      handle_constant(node);  
   else if(tok_type == TOK_POS || tok_type == TOK_NEG ||
        *(node->lexinfo) == "!" ) {
     astree* only_child = node->children[0];
     handle_expr(only_child);
     if(only_child->attributes[ATTR_int] != 1)
       throw "binomial ops just accept ints";
     if(only_child->attributes[ATTR_array] == 1)
       throw "binomial ops don't accept arrays";
     node->attributes[ATTR_int] = 1;
     node->attributes[ATTR_vreg] = 1;
   } 
   else if(*(node->lexinfo) == TOK_EQ){
      
      astree* left_child = node->children[0];
      astree* right_child = node->children[1];
      handle_expr(left_child);
      handle_expr(right_child);
      if(left_child->attributes[ATTR_lval] == 0)
         throw "left expression not an lval";
      bool types_compatible = compatible_types(left_child, right_child);
         if(types_compatible == false)
           throw "incompatible types for equal";
      bool attr_arr_left = left_child->attributes[ATTR_array] == 1;
      bool attr_arr_right = right_child->attributes[ATTR_array] == 1;
      if(attr_arr_left != attr_arr_right)
       throw "Error: One is an array and the other one a variable";
      node->attributes = right_child->attributes;
      node->attributes[ATTR_vreg] = 1;
   }
   else{
     auto find_op_overloaded = overloaded_ops.find(*(node->lexinfo));
     if(find_op_overloaded != overloaded_ops.end()){
       astree* left_child = node->children[0];
       astree* right_child = node->children[1];
       handle_expr(left_child);
       handle_expr(right_child);
       bool types_compatible = compatible_types(left_child, 
            right_child); //
       if(types_compatible == false)
      throw "incompatible types for overloaded ops";
       bool attr_arr_left = left_child->attributes[ATTR_array] == 1;
       bool attr_arr_right = right_child->attributes[ATTR_array] == 1;
       if(attr_arr_left != attr_arr_right)
      throw "Error: One is an array and the other one a variable";
       node->attributes[ATTR_int] = 1;
       node->attributes[ATTR_vreg] = 1;
      }
      else{
        auto find_op_noverloaded = 
            non_overloaded_ops.find(*(node->lexinfo)); //
     if(find_op_noverloaded != non_overloaded_ops.end()){
           astree* left_child = node->children[0];
           astree* right_child = node->children[1];
           handle_expr(left_child);
           handle_expr(right_child);
           bool attr_int_left = left_child->attributes[ATTR_int] == 1;
           bool attr_int_right = right_child->attributes[ATTR_int] == 1;
           if((attr_int_left && attr_int_right) != 1)
              throw "variables not of type int type";
           bool attr_arr_left = left_child->attributes[ATTR_array] == 1;
           bool attr_arr_right = 
                right_child->attributes[ATTR_array] == 1; //
          if((attr_arr_left || attr_arr_right) == true)
             throw "expressions can't be arrays";
           node->attributes[ATTR_int] = 1;
           node->attributes[ATTR_vreg] = 1;  
        }
     else if(*(node->lexinfo) == ";") return;
        else{cout <<*(node->lexinfo) <<endl; 
             throw "operator not found, stopping program...";} //
     }   
   }
}

void type_checker::handle_block(astree* node, symbol_table parameters){
  string old_sym_ident = current_sym_ident;
  current_sym_ident = current_sym_ident + "   ";
  symbol_table* new_scope = new symbol_table();
  symbol_stack.push_back(new_scope);
  ++current_block_nr;
  for(auto it : parameters)
    symbol_stack.at(current_block_nr)->insert({it.first, it.second});
  for(auto it : node->children)
      handle_statement(it);  
  symbol_stack.pop_back();
  --current_block_nr;
   current_sym_ident = old_sym_ident;
}

void type_checker::handle_vardecl(astree* node){
    astree* left_child = node->children[0];
    astree* right_child = node->children[1];
    symbol_table* current_table = symbol_stack[current_block_nr];
    handle_var_defs(left_child,current_table,false, true);
    handle_expr(right_child);
    bool compatible = compatible_types(left_child,right_child); 
    if(compatible == false)
     throw "incompatible types in vardeclaration";
}

void type_checker::handle_statement(astree* node){
   symbol_table dummy_table; // not used for closures
   string node_type = parser::get_tname(node->symbol);
   if(node_type == TOK_BLOCK){
     handle_block(node, dummy_table);}
   else if (node_type == TOK_VARDECL){
     handle_vardecl(node);      
   }
   else if (node_type == TOK_WHILE){
     astree* left_child = node->children[0];
     astree* right_child = node->children[1];
     handle_expr(left_child);
     handle_statement(right_child);
   }
   else if( node_type == TOK_IF){
     astree* left_child = node->children[0];
     astree* right_child = node->children[1];
     handle_expr(left_child);
     handle_statement(right_child);
   }
   else if (node_type == TOK_IFELSE){
     astree* left_child = node->children[0];
     astree* right_child = node->children[1];
     astree* third_child = node->children[2];
     handle_expr(left_child);
     handle_statement(right_child);
     handle_statement(third_child);
   }
   else if (node_type == TOK_RETURN ||
         node_type == TOK_RETURNVOID){

    if(current_return_type == VOID_TYPE){
      if(node->children.size() > 0)
           throw "void type doesn't have a return argument";
      else return;
    }

    if(node->children.size() < 1)
        throw "invalid return argument";

     vector<int> attrs = 
     {ATTR_int, ATTR_void, ATTR_string, ATTR_struct}; //
     enumString enumTypes;
     astree* only_child = node->children[0];
     handle_expr(only_child);
    
     for(auto it : attrs){
       if(only_child->attributes[it] == 1){
         if(it == ATTR_struct){
            if(*(only_child->sym_node.sym->type_id) !=
               current_return_type)
               throw "incompatible return type"; 
         }
         else if(enumTypes.enumStrings[it] != current_return_type)
                throw "incompatible return type";
       }
     }
     
     if(only_child->attributes[ATTR_array] == 1){
        if(isReturnArr == false)
          throw "return variable not an array";
     }
     
   }
   else{
     handle_expr(node);
   }  
}

symbol_table type_checker::initialize_parameters(astree* node, 
        vector<symbol*>*& param_vec){ //
    symbol_table param_table;
    auto& children = node->children;
    param_vec = new vector<symbol*>;
    bool isArr = false;
    astree* dummy_child;
    for(auto it : children){
        auto child = it;
     string tok_name = parser::get_tname(it->symbol);
        if(tok_name == TOK_ARRAY){
         dummy_child = new astree();
        *dummy_child = *(it->children[0]);
        child = dummy_child;
        child->children.push_back(it->children[1]);
        isArr = true;
     }

       auto is_typeid = type_name_table.find(child->lexinfo);
       auto is_primitive = primitive_types.find(*(child->lexinfo));
       if(is_typeid != type_name_table.end() || 
          is_primitive != primitive_types.end()){ //
       if(is_typeid != type_name_table.end() && 
          !is_struct_complete(is_typeid->second))  //
             throw "incomplete struct";
       astree* only_child = child->children.front();
       if(only_child == nullptr) throw "children vector empty";
       auto exists = param_table.find(only_child->lexinfo);
       if(exists == param_table.end()){
          const string* type = child->lexinfo;
          symbol* param_symbol = new symbol();
          if(isArr) param_symbol->attributes.set(ATTR_array);
          param_symbol->attributes.set(ATTR_param);
          param_symbol->attributes.set(ATTR_lval);
          param_symbol->attributes.set(ATTR_vreg);
          param_symbol->filenr = only_child->lloc.filenr;
          param_symbol->linenr = only_child->lloc.linenr;
          param_symbol->offset = only_child->lloc.offset;
          param_symbol->block_nr = current_block_nr;
         if(*type == VOID_TYPE)
           throw "void declaration not allowed";
         else if(*type == INT_TYPE)
           param_symbol->attributes.set(ATTR_int);
         else if (*type == STRING_TYPE){
           param_symbol->attributes.set(ATTR_string);
         }
         else{
            param_symbol->type_id = type; 
            param_symbol->attributes.set(ATTR_struct);
         }
         param_table.insert({only_child->lexinfo, param_symbol});
         param_vec->push_back(param_symbol);
         only_child->block_nr = current_block_nr;
         only_child->attributes = param_symbol->attributes;
         only_child->sym_node.sym = param_symbol;
       }
       else {
        string error = "duplicate parameter: " + *(only_child->lexinfo);
        throw error.c_str();
       }
              
       }
       else {
        string error = "The type: " + *(child->lexinfo) +
                "hasn't been defined ";
     throw error.c_str();
       }    
    }
    return param_table;
}

bool type_checker::isEquivalent(const symbol* prototype, 
          const symbol* func_def, int type){ //
   if(prototype->isDefined == true && type == PROTOTYPE)
     return false;
   

   for(int i = 0; i < ATTR_bitset_size; ++i){
       if(prototype->attributes[i] != func_def->attributes[i])
       return false; 
   }
 
   if(prototype->attributes[ATTR_struct] == 1){
     if (prototype->type_id != func_def->type_id)
     return false;
    }
   if(type == PROTOTYPE){
   size_t first_param_size = (prototype->parameters)->size();
   size_t second_param_size = (func_def->parameters)->size();
   if(first_param_size != second_param_size)
       return false;
    for(size_t i = 0; i < first_param_size ; i++){
      symbol* first_param = (prototype->parameters)->at(i);
      symbol* second_param = (func_def->parameters)->at(i);
      bool param_equivalence =  
        isEquivalent(first_param,second_param, VARIABLE); //
      if(param_equivalence == false)
        return false;
    }
   }
   return true;
}


symbol* type_checker::check_function_type(astree* node, 
           vector<symbol*>* param_vector, bool isArr){ //
  string symbol_tok = parser::get_tname(node->symbol);
  if(symbol_tok == TOK_ARRAY){
    astree* only_child = new astree();
       (*only_child) =  *(node->children[0]);
    only_child->children.push_back(node->children[1]);
    return check_function_type(only_child, param_vector, true);
  }

  auto is_typeid = type_name_table.find(node->lexinfo);
  auto is_primitive = primitive_types.find(*(node->lexinfo));
  if(is_typeid != type_name_table.end() || 
     is_primitive != primitive_types.end()){ //
      if(is_typeid != type_name_table.end() && 
        !is_struct_complete(is_typeid->second))  //
          throw "incomplete struct"; 
      auto& children = node->children;
      astree* only_child = children.front();
      symbol* new_func = new symbol();
      new_func->attributes.set(ATTR_function);
      new_func->filenr = node->lloc.filenr;
      new_func->linenr = node->lloc.linenr;
      new_func->offset = node->lloc.offset;
      new_func->block_nr = current_block_nr;
      new_func->parameters = param_vector;
      const string* type = node->lexinfo;
      if(*type == VOID_TYPE)
      new_func->attributes.set(ATTR_void);
      else if (*type == INT_TYPE)
      new_func->attributes.set(ATTR_int);
      else if (*type == STRING_TYPE)
       new_func->attributes.set(ATTR_string);
      else{
        new_func->attributes.set(ATTR_struct);
     new_func->type_id = type;
       }
       if(isArr == true)
         new_func->attributes[ATTR_array] = 1;
       only_child->block_nr = current_block_nr;
       only_child->attributes = new_func->attributes;
       only_child->sym_node.sym = new_func;

       auto isIdentDef = 
          symbol_stack[current_block_nr]->find(only_child->lexinfo); //
       if(isIdentDef != symbol_stack[current_block_nr]->end()){ 
        bool isPrototype = 
            isEquivalent(isIdentDef->second, new_func, PROTOTYPE); //
         if(isPrototype == false){
            string error = "the identifier: " + *(only_child->lexinfo) +
                              "was declared already";
               throw error.c_str();
          }
         }
        output_sym_line(new_func," ",only_child->lexinfo);
        symbol_stack[current_block_nr]->insert(
               {only_child->lexinfo, new_func});    // 
        return new_func; 
   }
   else{
     string error = "the type: " + 
       *(node->lexinfo) + "hasn't been define yet"; //
     throw error.c_str();
   }

}

void type_checker::handle_function(astree* node){

   auto setReturnType =[](symbol* sym){
     if(sym->attributes[ATTR_int] == 1)
        current_return_type = INT_TYPE;
     else if(sym->attributes[ATTR_string] == 1)
         current_return_type = STRING_TYPE;
     else if(sym->attributes[ATTR_void] == 1)
         current_return_type = VOID_TYPE;
     else if(sym->attributes[ATTR_struct] == 1)
         current_return_type = *(sym->type_id);   

     if(sym->attributes[ATTR_array] == 1)
        isReturnArr = true;  
   };


   int type_index = 0;
   int param_child_index = 1;
   int block_child_index = 2;
   astree *param_child = node->children[param_child_index]; 
   astree* type_child = node->children[type_index];
   astree* block_child = node->children[block_child_index];
   vector<symbol*>* param_vec = nullptr;
   symbol_table parameters = 
        initialize_parameters(param_child, param_vec); //
  if(param_vec == nullptr) throw "uninitialized_vec"; 
   symbol* func_sym = check_function_type(type_child,param_vec, false);
    for(auto it : parameters)
       output_sym_line(it.second,"    ", it.first);
   setReturnType(func_sym);
   string tok_info = parser::get_tname(block_child->symbol);
   if(tok_info == TOK_BLOCK){
      handle_block(block_child, parameters);
      current_return_type = VOID_TYPE;
      isReturnArr = false;
   }
   else func_sym->isDefined = false;
   output_separator();
}

void type_checker::traverse(astree* root){
    string node_name;
    for(auto& it : root->children){
      node_name = parser::get_tname(it->symbol);
      try{
      if(node_name == TOK_STRUCT)
      handle_struct_defs(it);
      else if (node_name == TOK_FUNCTION){
       handle_function(it);
      }
      else {handle_statement(it);}
      }
      catch(const char* msg){
     std::cerr << msg << std::endl;
      }
    } 
}



