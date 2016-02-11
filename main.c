//
//  eeval
//  version 1.0
//
//  a math expression evaluator
//
//  main.c
//
//  eeval implementation as a command line tool
//
//  Copyright (c) 2016 Paolo Bertani - Kalei S.r.l.
//  Licensed under the FreeBSD 2-clause license
//



#include "eeval.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// Sample (dead) code that shows how to embed `eeval` in your program

void demo()
{
    // eeval structure:
    EEvaluation eval;

    // input:
    const char  *expr = "-.02+pi+(3*sin(average(1,2,3)^-.03E2))"; // the expression

    // output:
    double      result; // result of the evaluation
    EEvalStatus status; // evaluation succedded or failed
    const char  *error; // description of error

    // execute evaluation
    status = EEvaluate( &eval, expr, &result );

    // expression evaluation failed ?
    if( status == EEvalFailure )
    {
        error = eval.error;
        // or...
        EEPrintError( &eval );
        // ...
    }
}



// Evaluates the expressin passed as parameter
// or perform self-test if invoked with "-t".

int main(int argc, const char * argv[])
{
    EEvaluation eval;
    double      result;

    long int    precision;
    char        *endptr;

    precision = 3; // default

    const char *usage =
    "\n"
    "usage:\n"
    "\n"
    "eeval [[-p prec] 'expr']\n"
    "\n"
    "where expr is the expression to evaluate\n"
    "and optional prec is the number of decimal digits\n"
    "to be printed in the output (between 0 and 20 included)\n"
    "\n"
    "when invoked from the shell it's recomended\n"
    "to place the expression between 'single' quotes\n"
    "\n"
    "if invoked without parameters usage and license info is printed\n"
    "\n"
    "supported operator are:\n"
    "\n"
    "+ plus\n"
    "- minus\n"
    "* multiplication\n"
    "/ division\n"
    "^ exponentiation\n"
    "! factorial (using Gamma function)\n"
    "\n"
    "supported function are:\n"
    "\n"
    "sin(r)  sine\n"
    "cos(r)  cosine\n"
    "tan(r)  tangent\n"
    "asin(n) arcsin\n"
    "acos(n) arccos\n"
    "atan(n) arctan\n"
    "fact(n) factorial of n; equivalent to n!\n"
    "exp(n) equivalent to e^n\n"
    "pow(b, n) equivalent to b^n\n"
    "log(n) natural logarithm of n (base e)\n"
    "log(b, n) logarithm of n with base b\n"
    "max(n1, n2, n3, ...) maximum of one or more numbers\n"
    "min(n1, n2, n3, ...) minimum of one or more numbers\n"
    "average(n1, n2, ...) average of one or more numbers\n"
    "avg(n1, n2, ...) abbreviated form of the above\n"
    "\n"
    "numbers can be expressed as follows:\n"
    "\n"
    "0.123  or  .123  or  12.3E-2  etc..\n"
    "\n"
    "recognized constants are:\n"
    "\n"
    "e  euler number\n"
    "pi Pi\n"
    "\n"
    "use round brackets to nest expressions\n"
    "whitespace, tabs and newlines are ignored\n"
    "\n"
    "execute self-test with:\n"
    "eeval -t\n"
    "(must have been built with test unit)\n"
    "\n"
    "-------------------------------------------------------------------------------\n"
    "\n"
    "FreeBSD 2-clause license\n"
    "\n"
    "Copyright (c) 2016, Paolo Bertani - Kalei S.r.l.\n"
    "All rights reserved.\n"
    "\n"
    "Redistribution and use in source and binary forms, with or without\n"
    "modification, are permitted provided that the following conditions are met:\n"
    "\n"
    "1. Redistributions of source code must retain the above copyright notice, this\n"
    "   list of conditions and the following disclaimer.\n"
    "2. Redistributions in binary form must reproduce the above copyright notice,\n"
    "   this list of conditions and the following disclaimer in the documentation\n"
    "   and/or other materials provided with the distribution.\n"
    "\n"
    "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND\n"
    "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"
    "WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n"
    "DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR\n"
    "ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n"
    "(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n"
    "LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND\n"
    "ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
    "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
    "SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
    "\n"
    "-------------------------------------------------------------------------------\n"
    "\n";

    if( argc == 2 || argc == 4 )
    {
        // Three arguments are passed so it is expected to find `-p` option
        // followed by the required precision.

        if( argc == 4 )
        {
            if( strncmp( argv[1], "-p", 3 ) == 0)
            {
                precision = strtol( argv[ 2 ], &endptr, 10 );
                if( endptr == argv[2] || *endptr != '\0' )
                {
                    fprintf( stderr, "value specified for precision parameter is not a integer number\n" );
                    exit( 1 );
                }
                if( precision < 0 || precision > 20 )
                {
                    fprintf( stderr, "value specified for precision parameter must be between 0 and 20 (included)\n" );
                    exit( 1 );
                }
            }
            else
            {
                fprintf( stderr, "%s", usage );
                exit( 1 );
            }
        }

        // Requested self-test ? Execute and exit.

        if( strncmp( argv[1], "-t", 3 ) == 0 )
        {
            if( eeval_test )
            {
                #if eeval_test == true
                EEvalExecuteTests();
                #endif
                exit( 0 );
            }
            else
            {
                printf( "Test unit not available\n" );
                exit( 1 );
            }
        }

        // The passed expression is evaluated.
        // If evaluation succeeds the result is printed.
        // If fails then prints the error.

        if( EEvaluate( &eval, argv[ argc - 1 ], &result ) == EEvalSuccess )
        {
            printf( "%.*f\n", (int)precision, result );
        }
        else
        {
            EEPrintError( &eval );
            exit( 1 );
        }
    }
    else
    {
        fprintf( stderr, "%s", usage );
        exit( 1 );
    }
}