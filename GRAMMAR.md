# Gravel's Grammar
Here, the Gravel grammar will be explained using EBNF

---

```ebnf
program     = sentence* ;
sentence    = scho
            | end
            | namespace
            | if
            | fun_def
            | fun_call
            | return
            | inferred_var
            | explicit_var
            | variable_mod
            | repeat
            ;

letter      = "a" .. "z" | "A" .. "Z" ;
digit       = "0" .. "9" ;

condition   = value, '==', value ;
operation   = value, ('+' | '-' | '*' | '/' | '%'), value ;
negation    = '-', value ;

name_char   = letter | digit | "_" ;
name        = name_char+ ;

text_char   = letter | digit | "_" | "-" | " " | "." | "!" | "?" | "," ;
text_literal= '"', text_char*, '"' ;

value       = fun_call
            | text_literal
            | name
            | operation
            | negation
            ;

end         = 'end' ;

scho        = 'scho', '(', character, ')' ;

namespace   = 'namespace', name, sentence*, end ;

if          = 'if', condition, sentence*,
              (elseif | else | end) ;

elseif      = 'elseif', condition, sentence*,
              (elseif | else | end) ;

else        = 'else', sentence*, end ;

args        = '(', [type, name, {',', type, name}], ')' ;

type        = 'int'
            | 'char'   (*Coming soon*)
            | 'float'  (*Coming soon*)
            | 'string' (*Coming soon*)
            ;

fun_def     = 'fun', name, args, [type], sentence*, end ;

fun_call    = name, '(', [value, {',', value}], ')' ;

return      = 'return', value ;

inferred_var= 'val', name, ':=', value ;
explicit_var= type, name, '=', value ;

variable_mod= name, '=', value ;

repeat      = 'repeat', value, sentence*, end ;

```