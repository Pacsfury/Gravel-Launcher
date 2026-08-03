# Gravel's Grammar
Here, the Gravel grammar will be explained using EBNF

---

```ebnf
program     = sentence*
sentence    = scho
            | end
            | namespace
            | if | elseif | else
            | fun_def
            | fun_call
            | return
            | inferred_var
            | explicit_var
            | variable_mod
            | repeat
            ;

letter      = "a" ... "z" | "A" ... "Z" ;
digit       = "0" ... "9" ;

letter      = [a-zA-Z] ;
digit       = [0-9] ;
character   = letter | digit | "_" | "-" | " ";
string      = character+ (*Can also be operations, conditions...*)

value       = fun_call
            | string
            | character
            | letter
            | digit
            ;

end         = 'end'

scho        = 'scho', '(', character, ')' ;

namespace   = 'namespace', string, sentence*, end;

if          = 'if', string, sentence*, 
            | elseif
            | else
            | end
            ;

elseif      = 'elseif', string, sentence*,
            | elseif
            | else
            | end
            ;

else        = 'else', sentence*, 'end';

args        = '(', (type, string)*, ')';

type        = int
            | char   (*Coming soon*)
            | float  (*Coming soon*)
            | string (*Coming soon*)

fun_def     = 'fun', string, args, type?, sentence*, end;

fun_call    = string, '(', args*, ')';

return      = 'return', value;

inferrer_var= 'val', string, ':=', value;
explicit_var= type, string, '=', value;

variable_mod= string, '=', string;

repeat      = 'repeat', digit | string, sentence*, end;


```