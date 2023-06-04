# big_numbers
implementation of bigger precision (up to 8kb) integers to perform high performance parallelized computing. Still WIP.

The objective is for it to work with both 64 and 32 bit systems. 

Currently the only implemented operations are: Addition, Subtraction, Multiplication, (Integer) Division, Raise to nth Power, nth Root, Left and Right bit-shift, Basic Modular Arithmetics.

You can initialize a new number to 0 with:
 big_number <varname> = {0};
You may also initialize a number either as the result of an arithmetic (not-in-place) operation or by copying an already existing number with copy().
Lastly, to compare 2 numbers you can use the compare() function.
  
Sorry for the lack of proper documentation but a lot of components may change as this is still wip.
 
# install
 just download bignumbers.c and include it into your code, no additional library is required
 
