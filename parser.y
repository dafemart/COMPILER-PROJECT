%{

#include "astree.h"
#include "lyutils.h"
astree* helper = nullptr;
%}

%debug
%defines
%error-verbose
%token-table 
%verbose

%destructor { destroy ($$); } <>


%initial-action{
   parser::root = new astree (TOK_ROOT, {0,0,0}, "");
}

%token TOK_VOID TOK_CHAR TOK_INT TOK_STRING
%token TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_NULL TOK_ARRAY
%token TOK_LT TOK_GT
%token TOK_IDENT TOK_CHARCON TOK_STRINGCON TOK_INTCON TOK_DECLID

%token TOK_INITDECL TOK_PARAM TOK_FUNCTION TOK_VARDECL
%token TOK_NEWARRAY TOK_TYPEID TOK_FIELD TOK_NEWSTRING
%token TOK_ORD TOK_CHR TOK_ROOT TOK_BLOCK TOK_IFELSE TOK_RETURNVOID
%nonassoc then
%right TOK_IF TOK_ELSE
%right '='
%left  TOK_EQ TOK_NE '<' TOK_LE '>' TOK_GE
%left  '+' '-'
%left  '*' '/' '%'
%right TOK_POS TOK_NEG '!' TOK_NEW
%left  '[' '.' TOK_CALL
%nonassoc  '('

%start start

%%
start      : program            {$$ = $1 = nullptr;}
           ;
program    : program structdef  {$$ = $1->adopt ($2);}
           | program statement  {$$ = $1->adopt ($2);}
           | program function   {$$ = $1->adopt ($2);}
           | program error '}'  {destroy($3);$$ = $1; }
           | program error ';'  {destroy($3);$$ = $1; }
           |  {$$ = parser::root;}
           ;
structdef  : TOK_STRUCT TOK_IDENT '{' fielddecls '}' {
             destroy($3); destroy($5);
             $2->swap_token_code(TOK_TYPEID); 
             $$ = $1->adopt($2)->cannibalize($4);
             }
           ;
fielddecls : fielddecls fielddecl {$$ = $1->adopt($2);}
           | {$$ = new astree ('{',{0,0,0}, "}");}
           ;
fielddecl  : basetype arr_block TOK_IDENT ';' {
            destroy($4);
            $3->swap_token_code(TOK_DECLID);
            if($2 == nullptr)
               $$ = $1->adopt($3);
            else
               $$ = $2->adopt($1,$3);}
           ;
arr_block  : TOK_ARRAY {$$ = $1;}
           | {$$ = nullptr;}
           ;

basetype   : TOK_VOID    {$$ = $1;}
           | TOK_INT     {$$ = $1;}
           | TOK_STRING  {$$ = $1;}
           | TOK_IDENT   {
             $1->swap_token_code(TOK_TYPEID);
             $$ = $1;}
           ;
function   : identdecl '(' identdecls ')' block{
             destroy($4);
             astree* func_node = new astree (TOK_FUNCTION,$1->lloc,"");
             $2->swap_token_code(TOK_PARAM);
             $2->cannibalize($3);
             func_node->adopt($1,$2);
             $$ = func_node->adopt($5);
            }
           ; 
identdecls : identdecl moredecls {
           astree* dummy = new astree ('{',{0,0,0}, "}");
           $$ = dummy->adopt($1)->cannibalize($2);}
           | {$$ = nullptr;}
           ;
moredecls  : moredecls ',' identdecl {
           destroy($2);
           $$ = $1->adopt($3);}
           |{$$ = new astree ('{',{0,0,0}, "}");}
           ;

statement  : block   {$$ = $1;}
           | vardecl {$$ = $1;}
           | while   {$$ = $1;}
           | ifelse  {$$ = $1;}
           | return  {$$ = $1;}
           | expr ';'{destroy($2); $$ = $1;}
           ;
return     : TOK_RETURN expression ';' {
           destroy($3);
           if($2 != nullptr)
              $1->adopt($2);
           else
              $1->swap_token_code(TOK_RETURNVOID);
           $$ = $1;
           }
           ;
expression : expr {$$ = $1;}
           | {$$ = nullptr;}
           ;
ifelse     : if   {$$=$1;}
           | ifandelse {$$ = $1;}
           ;
if         :TOK_IF '(' expr ')' statement %prec then{
            destroy($2); destroy($4);
            $1->adopt($3,$5);
            $$ = $1;
           }
           ;
ifandelse  : TOK_IF '(' expr ')' statement TOK_ELSE statement{
             destroy($2); destroy($4); destroy($6);
             $1->adopt($3,$5);
             $1->swap_token_code(TOK_IFELSE);
             $1->adopt($7);
             $$ = $1;
             }
           ;
while      : TOK_WHILE '(' expr ')' statement{
             destroy($2); destroy($4);
             $$ = $1->adopt($3,$5);
             }
           ; 
block      : '{' statements '}' {
             destroy($3);
             $1->swap_token_code(TOK_BLOCK);
             $$ = $1->cannibalize($2);}
           | ';' {$$ = $1;}
           ;
vardecl    : identdecl '=' expr ';' {
             $2->swap_token_code(TOK_VARDECL);
             destroy($4);
             $$ = $2->adopt($1,$3);}
           ;
identdecl  : basetype arr_block TOK_IDENT{
             $3->swap_token_code(TOK_DECLID);
             if($2 == nullptr)
               $$ = $1->adopt($3);
             else
               $$ = $2->adopt($1,$3);
             }
           ;
statements : statements statement {$$ = $1->adopt($2);}
           | {$$ = new astree ('{',{0,0,0}, "}");}
           ;
expr       : expr '=' expr    {$$=$2->adopt($1,$3);}
           | expr TOK_EQ expr {$$=$2->adopt($1,$3);}
           | expr TOK_NE expr {$$=$2->adopt($1,$3);}
           | expr '<' expr    {
             $2->swap_token_code(TOK_LT);
             $$=$2->adopt($1,$3);}
           | expr TOK_LE expr {$$=$2->adopt($1,$3);}
           | expr '>' expr    {
             $2->swap_token_code(TOK_GT);
             $$=$2->adopt($1,$3);}
           | expr TOK_GE expr {$$=$2->adopt($1,$3);}
           | expr '+' expr    {$$=$2->adopt($1,$3);}
           | expr '-' expr    {$$=$2->adopt($1,$3);}
           | expr '*' expr    {$$=$2->adopt($1,$3);}
           | expr '/' expr    {$$=$2->adopt($1,$3);}
           | expr '%' expr    {$$=$2->adopt($1,$3);}
           |'+' expr %prec TOK_POS {
              $1->swap_token_code(TOK_POS);
              $$ = $1->adopt($2);
              }
           | '-' expr %prec TOK_NEG {
              $1->swap_token_code(TOK_NEG);
              $$ = $1->adopt($2);
              }
           | '!' expr {$$ = $1->adopt($2);}
           | allocator         {$$ = $1;}
           | TOK_IDENT '(' exprs ')' {
            destroy($4);
            $2->swap_token_code(TOK_CALL);
            $$ = ($2->adopt($1))->cannibalize($3);
            }
           | TOK_IDENT  {$$ = $1;}
           | '(' expr ')'  {
              destroy($1);destroy($3);
              $$= $2;}
           | expr '[' expr ']' {
             destroy($4);
             $$ = $2->adopt($1,$3);
            }
           | expr '.' TOK_IDENT  {
             $3->swap_token_code(TOK_FIELD);
             $$ = $2->adopt($1,$3);}
           | constant
           ;
allocator  : TOK_NEW TOK_IDENT '('')' {
            destroy($3); destroy($4);
            $2->swap_token_code(TOK_TYPEID);
            $$ = $1->adopt($2);
            }
           | TOK_NEW TOK_STRING '(' expr ')'{
             destroy($1); destroy($3); destroy($5);
             $2->swap_token_code(TOK_NEWSTRING);
             $$ = $2->adopt($4);
             }
           | TOK_NEW basetype '[' expr ']'{
             destroy($3);destroy($5);
             $1->swap_token_code(TOK_NEWARRAY);
             $$ = $1->adopt($2,$4);
             }
           ;
exprs      : expr moreexprs  {
             astree* dummy = new astree ('{',{0,0,0}, "}");
             $$ = dummy->adopt($1)->cannibalize($2);}
           | {$$ = nullptr;}
           ;
moreexprs  : moreexprs ',' expr {
             destroy($2);
             $$ = $1->adopt($3);} 
           |{$$ = new astree ('{',{0,0,0}, "}");}
           ;
constant   :TOK_INTCON    {$$ = $1;}
           |TOK_CHARCON   {$$ = $1;}
           |TOK_STRINGCON {$$ = $1;}
           |TOK_NULL      {$$ = $1;}
           ;
%%
const char* parser::get_tname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}
