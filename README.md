# SME-MD5

This project is an MD5 bruteforcer implemented in SME. There are 5 different
implementations:

- A simple SME implementation, in which the entire computation is made within a
  clock cycle
- A compact SME version, in which the loop body of the MD5 function is reused,
  thus resulting in lower space usage and a higher clockrate.
- A pipelined SME version, in which each iteration of the loop body is
  destributed to its own process, thus increasing the clockrate.
- An OpenMP C implementation for comparison.
- An OpenCL implementation for comparison.

Each of the OpenMP C and the OpenCL implementations have a corresponding
Makefile for compiling.

The SME versions depend on the SME project, which can be found at

https://github.com/kenkendk/sme
