// $Id: astree.h,v 1.7 2016-10-06 16:13:39-07 - - $
#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
#include <bitset>
#include <unordered_map>
using namespace std;

#include "auxlib.h"

struct enumString{
  vector<const char*> enumStrings =
   { "void", "int", "null", "string",
     "struct", "array", "function", "variable",
     "field", "typeid", "param", "lval", "const",
     "vreg", "vaddr",
  };
};

enum {ATTR_void, ATTR_int, ATTR_null, ATTR_string,
      ATTR_struct, ATTR_array, ATTR_function, ATTR_variable,
      ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval, ATTR_const,
      ATTR_vreg, ATTR_vaddr, ATTR_bitset_size,
};

struct symbol;
using symbol_table = unordered_map<const string*,symbol*>;
using symbol_entry = symbol_table::value_type;
using attr_bitset = bitset<ATTR_bitset_size>;

struct symbol{
    attr_bitset attributes;
    symbol_table* fields;
    size_t filenr, linenr, offset;
    size_t block_nr;
    vector<symbol*>* parameters;
    const string* type_id;
    bool isDefined = true;
};

struct table_node{
  symbol* sym = nullptr;
};

struct location {
   size_t filenr;
   size_t linenr;
   size_t offset;
};

struct astree {
   // Fields.
   int symbol;               // token code
   location lloc;            // source location
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node
   attr_bitset attributes;
   table_node sym_node;
   size_t block_nr = 0;
   // Functions.
   astree (int symbol, const location&, const char* lexinfo);
   astree() = default;
   ~astree();
   astree* adopt (astree* child1, astree* child2 = nullptr);
   astree* adopt_sym (astree* child, int symbol);
   astree* cannibalize(astree* node);
   void swap_token_code (int tok_code);
   void dump_node (FILE*);
   void dump_tree (FILE*, int depth = 0);
   static void dump (FILE* outfile, astree* tree);
   static void print (FILE* outfile, astree* tree, int depth = 0);
};


void destroy (astree* tree1, astree* tree2 = nullptr);

void errllocprintf (const location&, const char* format, const char*);

#endif

