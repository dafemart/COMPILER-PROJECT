/* $Id: lexer.l,v 1.7 2016-10-27 19:45:59-07 - - $ */

%{

#include "lyutils.h"

#define YY_USER_ACTION  { lexer::advance(); }

int yylval_token (int symbol) {
   yylval = new astree (symbol, lexer::lloc, yytext);
   fprintf(tok_str_file_inter::tok_file_, 
                           "  %2zd %2zd.%-3.3zd  %3i %-14s (%s)\n",
                           yylval->lloc.filenr, yylval->lloc.linenr,
                           yylval->lloc.offset, yylval->symbol,
                           parser::get_tname(yylval->symbol), yytext);
   tok_str_file_inter::set.intern(yytext);
   return symbol;
}

%}

%option 8bit
%option debug
%option nodefault
%option noinput
%option nounput
%option noyywrap
%option warn
/*%option verbose*/

LETTER          [A-Za-z_]
DIGIT           [0-9]
MANTISSA        ({DIGIT}+)
EXPONENT        ([Ee][+-]?{DIGIT}+)
NUMBER          ({MANTISSA})
NOTNUMBER       ({MANTISSA}[Ee][+-]?)
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
CHARCON         ('([^\\\n]|\\[\\"0nt])')
STRINGCON       (\"([^\\"\n]|\\[\\"0nt])*\")
BADIDENT        (({DIGIT})+{IDENT})
MISSINGSQUOTE   ('([^\\\n]|\\[\\"0nt]))
BADCHARESC      ('(\\[^\\\n])')
BADCHARCON      ({MISSINGSQUOTE}|{BADCHARESC})
MISSINGQUOTE    (\"([^\\"\n]|\\[\\"0nt])*)
BADESCAPE       (\"([^\\"\n])*(\\[^\\"\n])([^\\"\n])*\")      
BADSTRINGCON    ({MISSINGQUOTE}|{BADESCAPE})

%%

"#".*           { lexer::include(); }
[ \t]+          { }
\n              { lexer::newline(); }

"if"            {return yylval_token(TOK_IF);}
"else"          {return yylval_token(TOK_ELSE);}
"while"         {return yylval_token(TOK_WHILE);}
"return"        {return yylval_token(TOK_RETURN);}
"int"           {return yylval_token(TOK_INT);}
"string"        {return yylval_token(TOK_STRING);}
"struct"        {return yylval_token(TOK_STRUCT);}
"void"          {return yylval_token(TOK_VOID);}
"new"           {return yylval_token(TOK_NEW);}
"null"          {return yylval_token(TOK_NULL);}

"="             { return yylval_token ('='); }
"+"             { return yylval_token ('+'); }
"-"             { return yylval_token ('-'); }
"*"             { return yylval_token ('*'); }
"/"             { return yylval_token ('/'); }
"^"             { return yylval_token ('^'); }
"("             { return yylval_token ('('); }
")"             { return yylval_token (')'); }
"["             { return yylval_token ('['); }
"]"             { return yylval_token (']'); }
"{"             { return yylval_token ('{'); }
"}"             { return yylval_token ('}'); }
";"             { return yylval_token (';'); }
"<"             { return yylval_token ('<'); }
"%"             { return yylval_token ('%'); }
"!"             { return yylval_token ('!'); }
"."             { return yylval_token ('.'); }
","             { return yylval_token (','); }
"<"             { return yylval_token ('<'); } 
">"             { return yylval_token ('>'); }
"=="            { return yylval_token (TOK_EQ); }
"!="            { return yylval_token (TOK_NE); }
"<="            { return yylval_token (TOK_LE); }
">="            { return yylval_token (TOK_GE); }
"[]"            { return yylval_token (TOK_ARRAY); }

{IDENT}         { return yylval_token (TOK_IDENT);}
{NUMBER}        { return yylval_token (TOK_INTCON);}
{CHARCON}       { return yylval_token (TOK_CHARCON);}
{STRINGCON}     { return yylval_token (TOK_STRINGCON);}


{NOTNUMBER}     { lexer::badtoken (yytext);}
{BADIDENT}      { lexer::badtoken (yytext);}
{BADCHARCON}    { lexer::badtoken (yytext);}
{BADSTRINGCON}  { lexer::badtoken (yytext);}
.               { lexer::badchar (*yytext);}
%%

