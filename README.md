# Producer/Consumer with Semaphores

Video Link: https://drive.google.com/file/d/1-SCHYJ1HbdPqinuRxyd48VSKd9K7AExx/view?usp=sharing

Presentation Link: https://docs.google.com/presentation/d/1X2yaYPMaDpI3ggbROT0SNFUwm1HzOkEd8x2o7_j7imM/edit?usp=sharing

Presentation Link:

## Overview

This project is an extension of the Pr4: Producer/Consumer with a twist: the code only uses semaphores. This means that all mutex locks have been converted to binary semaphores. Additionally, the application tackles the Producer/Consumer problem proposed by Dijkstra and an implementation of read/write locks.

## Major Changes to Code Structure

All `mutex` locks are now converted to binary semaphores.

`ledger.cpp` now contains a `reader()` method for the producer threads to run separately from the `worker()` threads.

`ledger.h` has a global constant variable `MAX` for setting the size of the bounded buffer. Additionally, the `Ledger` structure now also contains the `reader` attribute that indicates which reader thread parsed that particular instruction.

`bank.cpp` now contains a `check_balance` method which returns the amount of money in the specified account number.

In `bank.h`, the `Account` structure now has a semaphores `write_lock` and `read_lock` for implementing read/write locks in `check_balance`.

## How to Run

Compile the code by running `make` in your terminal.

Then to execute the program, run:

    $ ./bank_app <num_each> <ledger_file>

`<num_each>` indicates the number of `worker()` threads and `reader()` threads _separately_. For example, if `<num_each>` is 2, then there will be 2 `worker()` threads and two `reader()` threads.

`<ledger_file>` still must be a .txt file with each line in the file following the format:

    FROM_ID TO_ID AMOUNT MODE

`FROM_ID` is the account ID we would like to act on. `TO_ID` is only used on transfers and is the account ID to transfer money to. `AMOUNT` is the amount to deposit, withdraw, or transfer. Finally, `MODE` is the action where:

    0 => Deposit
    1 => Withdraw
    2 => Transfer
    3 => Check Balance

## Debugging

I have provided a .vscode folder for VSCode debugging. The `args` field can be changed to whatever ledger text file you would like to debug and the number of worker/reader threads. Otherwise, you may debug as you normally would with:

    $ gdb PROGRAM_NAME

## References

https://pages.cs.wisc.edu/~remzi/OSTEP/threads-sema.pdf
