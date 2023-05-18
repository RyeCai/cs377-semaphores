# Producer/Consumer with Semaphores

## Overview

This project is an extension of the Pr4: Producer/Consumer with a twist: the code only uses semaphores. This means that all mutex locks have been converted to binary semaphores. Additionally,
This project has the programmer tackle the Producer/Consumer problem proposed by Dijkstra using Semaphores in C/C++. The programmer will need to solve concurrency and ordering issues related to ledger

## Learning Objectives

## Instructions

InitBank method argument `num_workers` now dictates `num_workers` of reader/producer threads and `num_workers` of worker/consumer threads

## References

https://pages.cs.wisc.edu/~remzi/OSTEP/threads-sema.pdf

<!-- TODO
- Convert all mutex locks into binary semaphore locks
- Create a bounded buffer instead of a ledger list
- Create separate producer and consumer threads
- Add a CheckBalance()
- Have multiple threads read from text file-->
