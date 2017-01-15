Deliverables for 08348 (Languages and their Compilers) Assessed Coursework
-------------------------------------------------------------------------

Alexander C Whitehead, Department of Computer Science.  A.C.Whitehead@2014.hull.ac.uk 483446


Introduction
------------

The files in this directory are supplied as deliverables for the 08348 (Languages and their Compilers) Assessed
Coursework.
This includes a compiler written to compile SPL into ANSI C and tests which can be performed upon the compiler.


Inventory
---------

README.txt		-- This file

spl.bnf		-- A text file containing the BNF of the compiler
spl.l		-- This is the lex/flex file of the compiler
spl.y		-- This is the bison/yacc file of the compiler
spl.c		-- This is the C main program used in compiling the compiler

Tests1		-- These are simple test files for the SPL language
Tests2		-- These are advanced test files for the SPL language
foldingTest.SPL		-- This is a test file for the compiler optimisation

runLexer.bat		-- This only builds the lexical analysis section of the compiler
runDebugCompiler.bat		-- This builds the compiler in debug mode
runCompiler.bat		-- This builds the full compiler
runOptimisedCompiler.bat		-- This builds the full compiler with optimisations

runLexerTests1Manually.bat		-- This builds the lexical analysis section of the compiler and attempts to
					-- execute the contents of Tests1 on it manually
runLexerTests1.bat		-- This builds the lexical analysis section of the compiler and attempts to execute
				-- the contents of Tests1 on it automatically
runDebugTests1Manually.bat		-- This builds the compiler in debug mode and attempts to execute the
					-- contents of Tests1 on it manually
runDebugTests1.bat		-- This builds the compiler in debug mode and attempts to execute the contents of
				-- Tests1 on it automatically
runTests1Manually.bat		-- This builds the full compiler and attempts to execute the contents of Tests1 on
				-- it manually
runTests1.bat		-- This builds the full compiler and attempts to execute the contents of Tests1 on it
			-- automatically
runTests1Manually.bat		-- This builds the full compiler and attempts to execute the contents of Tests1 on
				-- it manually
runTests1.bat		-- This builds the full compiler and attempts to execute the contents of Tests1 on it
			-- automatically
runTests1OptimisedManually.bat		-- This builds the full compiler with optimisations and attempts to
					-- execute the contents of Tests1 on it manually
runTests1Optimised.bat		-- This builds the full compiler with optimisations and attempts to execute the
				-- contents of Tests1 on it automatically

runLexerTests2Manually.bat		-- This builds the lexical analysis section of the compiler and attempts to
					-- execute the contents of Tests2 on it manually
runLexerTests2.bat		-- This builds the lexical analysis section of the compiler and attempts to execute
				-- the contents of Tests2 on it automatically
runDebugTests2Manually.bat		-- This builds the compiler in debug mode and attempts to execute the
					-- contents of Tests2 on it manually
runDebugTests2.bat		-- This builds the compiler in debug mode and attempts to execute the contents of
				-- Tests2 on it automatically
runTests2Manually.bat		-- This builds the full compiler and attempts to execute the contents of Tests2 on
				-- it manually
runTests2.bat		-- This builds the full compiler and attempts to execute the contents of Tests2 on it
			-- automatically
runTests2OptimisedManually.bat		-- This builds the full compiler with optimisations and attempts to
					-- execute the contents of Tests2 on it manually
runOptimisedTests2.bat		-- This builds the full compiler with optimisations and attempts to execute the
				-- contents of Tests2 on it automatically

runFoldingTest.bat		-- This builds the full compiler with optimisations and attempts to execute 
				-- foldingTest.spl with it

a-output.txt		--This is the output from the compiler when it is run against test a
b-output.txt		--This is the output from the compiler when it is run against test b
c-output.txt		--This is the output from the compiler when it is run against test c
d-output.txt		--This is the output from the compiler when it is run against test d
e-output.txt		--This is the output from the compiler when it is run against test e

483446-tokens.txt		--This contains the tokens output from the lexer when acting upon tests a through
				-- e
483446-parse.txt		--This contains the results from all previous output files plus the output from the
				-- parser when acting upon tests a through e
483446-tree.txt			--This contains the results from all previous output files plus the output from the
				-- parser when acting upon tests a through e while in debug mode
483446-code.txt			--This contains the results from all previous output files plus the output from the
				-- compiler when acting upon tests a through e
483446-results.txt		--These are the results of tests conducted upon the compiler


Options
-------

Numerical casts are allowed. However, data will be lost.
Character casts are not allowed, as this can lead to unintended characters being printed.

The re-declaration of program identifiers is allowed, as the program identifiers are generally not used for
anything.

Integer overflows are not allowed, as this can lead to difficult to diagnose errors. An example of this can famously
be found in the game Civilisation.

Character arithmetic including integers and floats is not allowed, as it can lead to unexpected results
including the printing of incorrect or invalid characters.

For loops can be incremented by both characters and floats. This can be used incorrectly by novices but does
also allow for more complex programs when used correctly.


Optimisation
------------

The compiler has an optional optimisation which can be invoked by declaring OPTIMISATION as a command line argument
when compiling the compiler.
The optimisation can also be invoked by using one of the above batch files which dictates that it will compile the
compiler with its optimisation included.

The optimisation which has been included is constant folding which should assimilate large expressions into smaller
expressions or constants at compile time.
This should speed real time execution up dramatically and reduce memory usage.


Test Results
------------

All other text files contained within this directory are test results which can be used to evaluate the
effectiveness of the compiler

