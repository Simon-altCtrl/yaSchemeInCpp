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
