#include <string>
#include <string.h>
#include <unistd.h>
#include <iostream>
using namespace std;
#include "auxlib.h"
#include "cppstrtok.h"
#include "astree.h"
#include "lyutils.h"
#include "type_checker.h"
const char* delete_tok(int symbol){
   const char *tname = parser::get_tname (symbol);
   if (strstr (tname, "TOK_") == tname) tname += 4;
   return tname;
}

void dump_ast(FILE* trace_file, string ident, astree* node){
  if(node != nullptr){
  enumString enums;
  string lexinfo = *(node->lexinfo);
  fprintf(trace_file, "%s%s \"%s\" %zd.%zd.%zd  ",ident.c_str(),
         delete_tok(node->symbol),lexinfo.c_str(),
         node->lloc.filenr,node->lloc.linenr,
         node->lloc.offset);
   for(int attr = ATTR_void ; attr < ATTR_bitset_size; ++attr){
         if(node->attributes[attr] == 1 ){
           fprintf(trace_file, "%s ", enums.enumStrings[attr]);
           if(attr == ATTR_struct){
              symbol* sym = node->sym_node.sym;
              if(sym != nullptr)
              fprintf(trace_file, "%s ", 
              node->sym_node.sym->type_id->c_str());
           }
         }
      }
   fprintf(trace_file,"\n");

  string local_ident = ident + "|" + " ";
  for(auto it : node->children){
       dump_ast(trace_file,local_ident,it);
  }
  }
}


bool is_oc(const char* filename){
    const string filename_str = filename;
    std::size_t found_oc = filename_str.rfind(".oc");
    return found_oc != std::string::npos;
}

string append_flags(string flags, string filename){
    string command = CPP;
    if(!command.empty())
        command = command + " " + filename;
    else command = command + " " + flags + " " + filename;
    return command; 
}

const string suffix_file_name(const char* filename, string suffix){
     string filename_holder = filename;
     size_t dot_pos = filename_holder.rfind(".oc");
     filename_holder = filename_holder.substr(0,dot_pos);
     filename_holder += suffix;
     return filename_holder;
}

int main (int argc, char** argv) {
  exec::execname = basename(argv[0]);
  int c;
  string flags = "";
  yy_flex_debug = 0;
  yydebug = 0;
  while((c = getopt(argc,argv,"ly@:D:")) != -1){
      switch(c){
          case 'l': 
             yy_flex_debug = 1;
             break;
          case 'y':
             yydebug = 1;
             break;
          case 'D':
             flags += optarg;
          case '@':
             set_debugflags(optarg);
             break;
          case '?':
             std::cerr << "FLAG_ERROR" << std::endl;
              abort();
             break;   
      }
   }

   if (optind > argc) {
      errprintf ("Usage: %s [-ly] [filename]\n",
                 exec::execname.c_str());
      exit (exec::exit_status);
   }

   const char* filename = optind == argc ? "-" : argv[optind]; 
   int exit_status = EXIT_SUCCESS;  

 if(is_oc(filename) == false){
            std::cerr<< "INVALID FILE SUFFIX" << std::endl;
            printf("filename: %s\n", filename);
            exit_status = EXIT_FAILURE;
  }
  else{
     string command = append_flags(flags,filename);
        yyin = popen(command.c_str(), "r");
      if(yyin == NULL){
          exit_status = EXIT_FAILURE;
           cerr<< "error opening the file" << endl;
      }else{
          string_set set;
          string tok_file_name = suffix_file_name(filename,".tok");
          tok_str_file_inter::set_tok_file(tok_file_name);
          yyparse();
          int pclose_rc = pclose(yyin);
          if (pclose_rc != 0) exit_status = EXIT_FAILURE;
          string trace_file_name = suffix_file_name(filename,".str");
          FILE* trace_file = fopen(trace_file_name.c_str(),"w");
          set.dump(trace_file); 
          string ast_file_name = suffix_file_name(filename,".ast");
          FILE* ast_file = fopen(ast_file_name.c_str(),"w");
          fclose(trace_file);
          fclose(tok_str_file_inter::tok_file_);
          string sym_file_name = suffix_file_name(filename,".sym");
          tok_str_file_inter::set_sym_file(sym_file_name);
          type_checker::initialize_name_table();
          type_checker::traverse(parser::root);
          dump_ast(ast_file," ",parser::root);
          fclose(ast_file);
          fclose(tok_str_file_inter::sym_file_);

      }
  }
  return exit_status;
}
