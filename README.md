# yaSchemeInCpp
Yet another scheme interpreter in C++

It uses 64-bit integers to implement Scheme's integer type, which allows it to handle larger numbers (like factorial of 13) without overflow.
```plaintext

>>> (define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))                                                     
<Lambda>                                                                                                                
>>> (fact 13)                                                                                                           
6227020800                                                                                                              
>>>
```

Next:
1. Use ADT class replace 64bit int to implement int in Scheme which supports bigger value.
2. Use smart pointer to address memory leak.
--------------------
The code in yaSchemeInCpp.cpp implements a simple Scheme interpreter in C++. Here's a breakdown of how it works:
1. Data Structures
•	cell_type Enum: Defines the types of cells that can exist in the interpreter: Symbol, Number, List, Proc, and Lambda.
•	CCELL Class: A placeholder class with a union for different data types, but it is not used in the main implementation.
•	cell Struct: Represents a Lisp value. It can hold different types of values (symbols, numbers, lists, procedures, and lambdas). It includes:
•	type: The type of the cell.
•	val: The string value for symbols and numbers.
•	list: A vector of cells for lists.
•	proc: A function pointer for procedures.
•	env: A pointer to the environment where the cell was defined.
•	environment Struct: Represents a mapping of symbols to cells and can chain to an outer environment. It includes:
•	env_: A map of symbol names to cells.
•	outer_: A pointer to an outer environment.
2. Primitive Procedures
The code defines several primitive procedures for basic arithmetic and list operations:
•	proc_add, proc_sub, proc_mul, proc_div: Perform addition, subtraction, multiplication, and division.
•	proc_greater, proc_less, proc_less_equal: Comparison operations.
•	proc_length, proc_nullp, proc_car, proc_cdr, proc_append, proc_cons, proc_list: List operations.
3. Global Environment
•	add_globals Function: Initializes the global environment with predefined symbols and primitive procedures.
4. Evaluation
•	eval Function: Evaluates a Lisp expression in a given environment. It handles:
•	Symbols: Looks up the symbol in the environment.
•	Numbers: Returns the number itself.
•	Lists: Evaluates special forms (quote, if, set!, define, lambda, begin) and procedures.
5. Parsing
•	tokenize Function: Converts a string into a list of tokens.
