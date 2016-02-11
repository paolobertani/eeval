//
//  eeval
//  version 1.0
//
//  a math expression evaluator
//
//  eeval_test.c
//
//  eeval test suite
//
//  Copyright (c) 2016 Paolo Bertani - Kalei S.r.l.
//  Licensed under the FreeBSD 2-clause license
//



#include "eeval.h"

#if eeval_test == true

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>



//
// Execute all tests.
//

void EEvalExecuteTests()
{
    double b,
           e,
           r;

    // Plus and minus (unary/binary) mixing cases

    EEValTest( __LINE__, EEvalSuccess, 2,       "+2" );         // plus as unary operator
    EEValTest( __LINE__, EEvalSuccess, 0,       "2+-2" );       // plus as binary operator, minus as unary: 2 + ( -2 )
    EEValTest( __LINE__, EEvalSuccess, 0,       "2-+2" );       // vice-versa: 2 - ( -2 )
    EEValTest( __LINE__, EEvalSuccess, 4,       "2--2" );       // minus as both binary and unary operator 2 - ( -2 )
    EEValTest( __LINE__, EEvalSuccess, 0,       "+2-(+2)" );    // leading plus
    EEValTest( __LINE__, EEvalSuccess, 6,       "+2*(+3)" );    //
    EEValTest( __LINE__, EEvalSuccess, -3,      "1*-3" );       //
    EEValTest( __LINE__, EEvalSuccess, 6,       "2*+3" );       //
    EEValTest( __LINE__, EEvalFailure, 0,       "-+3" );        // *
    EEValTest( __LINE__, EEvalFailure, 0,       "+-3" );        // *
    EEValTest( __LINE__, EEvalFailure, 0,       "2++2" );       // * two plus as consecutive binary and unary operators not allowed
    EEValTest( __LINE__, EEvalFailure, 0,       "2---2" );      // * three minus ? not allowed
    EEValTest( __LINE__, EEvalFailure, 0,       "--2" );        // * beginning with two minus ? no, a value is expected

    // Single numbers

    EEValTest( __LINE__, EEvalSuccess, 2,       "2" );
    EEValTest( __LINE__, EEvalSuccess, 2,       "02" );
    EEValTest( __LINE__, EEvalSuccess, .2,      ".2" );
    EEValTest( __LINE__, EEvalSuccess, -.2,     "-.2" );
    EEValTest( __LINE__, EEvalSuccess, 1234,    "1234" );
    EEValTest( __LINE__, EEvalSuccess, 12.34,   "12.34" );
    EEValTest( __LINE__, EEvalSuccess, 1200,    "12E2" );
    EEValTest( __LINE__, EEvalSuccess, 0.12,    "12E-2" );
    EEValTest( __LINE__, EEvalSuccess, 12,      "12E0" );
    EEValTest( __LINE__, EEvalSuccess, 254,     "0xfE" );
    EEValTest( __LINE__, EEvalFailure, 0,       "12a0" );
    EEValTest( __LINE__, EEvalFailure, 0,       "12E2.5");      // * decimal exponent not allowed
    EEValTest( __LINE__, EEvalFailure, 0,       ".-2" );        // * not a number

    // Round brackets

    EEValTest( __LINE__, EEvalSuccess, 1,       "(1)" );
    EEValTest( __LINE__, EEvalSuccess, 42,      "1+(2*(3+(4+5+6))-1)+6" );
    EEValTest( __LINE__, EEvalSuccess, 1,       "(((((((((((1)))))))))))" );
    EEValTest( __LINE__, EEvalSuccess, -1,      "-(((((((((((1)))))))))))" );
    EEValTest( __LINE__, EEvalSuccess, 1,       "+(((((((((((1)))))))))))" );
    EEValTest( __LINE__, EEvalSuccess, -1,      "+(((((((((((-1)))))))))))" );
    EEValTest( __LINE__, EEvalSuccess, 1,       "-(((((((((((-1)))))))))))" );
    EEValTest( __LINE__, EEvalFailure, 0,       "+2*(+-3)" );                   // *
    EEValTest( __LINE__, EEvalFailure, 0,       "1+(2*(3+(4+5+6))-1+6" );       // * missing close bracket
    EEValTest( __LINE__, EEvalFailure, 0,       "1+(2*(3+(4+5+6))-1))+6" );     // * too many close brackets
    EEValTest( __LINE__, EEvalFailure, 0,       "1+()" );                       // * empty expression
    EEValTest( __LINE__, EEvalFailure, 0,       ".(((((((((((1)))))))))))" );   // *

    // Constants

    EEValTest( __LINE__, EEvalSuccess, -M_PI,   "-pi" );
    EEValTest( __LINE__, EEvalSuccess, exp(1),  "e" );

    // Functions

    EEValTest( __LINE__, EEvalSuccess, pow(6,5),        "pow(6,5)" );
    EEValTest( __LINE__, EEvalSuccess, exp(2.5),        "exp(2.5)" );
    EEValTest( __LINE__, EEvalSuccess, log(3)/log(2),   "log(2,3)" );   // base is the first parameter
    EEValTest( __LINE__, EEvalSuccess, log(3),          "log(e,3)" );
    EEValTest( __LINE__, EEvalSuccess, log(4),          "log(4)" );     // log with one parameter (base e)
    EEValTest( __LINE__, EEvalSuccess, sin(M_PI*.3),    "sin(pi*.3)" );
    EEValTest( __LINE__, EEvalSuccess, cos(M_PI*.3),    "cos(pi*.3)" );
    EEValTest( __LINE__, EEvalSuccess, tan(M_PI*.3),    "tan(pi*.3)" );
    EEValTest( __LINE__, EEvalSuccess, asin(.123),      "asin(.123)" );
    EEValTest( __LINE__, EEvalSuccess, acos(.123),      "acos(.123)" );
    EEValTest( __LINE__, EEvalSuccess, atan(.123),      "atan(.123)" );
    EEValTest( __LINE__, EEvalSuccess, 3,       "max(-1,2,3)" );
    EEValTest( __LINE__, EEvalSuccess, -1,      "min(-1,2,3)" );
    EEValTest( __LINE__, EEvalSuccess, 2,       "average(1,2,3)" );
    EEValTest( __LINE__, EEvalSuccess, 20,      "avg(10,20,30)" );
    EEValTest( __LINE__, EEvalSuccess, 3,       "max(3)" );
    EEValTest( __LINE__, EEvalSuccess, -1,      "min(-1)" );
    EEValTest( __LINE__, EEvalSuccess, 2,       "average(2)" );
    EEValTest( __LINE__, EEvalSuccess, 6.2,     "avg(6.2)" );
    EEValTest( __LINE__, EEvalFailure, 0,       "pow()" );      // * empty function
    EEValTest( __LINE__, EEvalFailure, 0,       "exp()" );      // * empty function
    EEValTest( __LINE__, EEvalFailure, 0,       "log()" );      // * empty function
    EEValTest( __LINE__, EEvalFailure, 0,       "sin()" );      // * empty function
    EEValTest( __LINE__, EEvalFailure, 0,       "cos()" );      // * empty function
    EEValTest( __LINE__, EEvalFailure, 0,       "tan()" );      // * empty function
    EEValTest( __LINE__, EEvalFailure, 0,       "asin()" );     // * empty function
    EEValTest( __LINE__, EEvalFailure, 0,       "acos()" );     // * empty function
    EEValTest( __LINE__, EEvalFailure, 0,       "atan()" );     // * empty function
    EEValTest( __LINE__, EEvalFailure, 0,       "max()" );      // * empty function
    EEValTest( __LINE__, EEvalFailure, 0,       "min()" );      // * empty function
    EEValTest( __LINE__, EEvalFailure, 0,       "average()" );  // * empty function
    EEValTest( __LINE__, EEvalFailure, 0,       "avg()" );      // * empty function
    EEValTest( __LINE__, EEvalFailure, 0,       "pow(1,2,3)" ); // * too many parameters
    EEValTest( __LINE__, EEvalFailure, 0,       "exp(1,2,3)" ); // * too many parameters
    EEValTest( __LINE__, EEvalFailure, 0,       "log(1,2,3)" ); // * too many parameters
    EEValTest( __LINE__, EEvalFailure, 0,       "sin(4,5)" );   // * too many parameters
    EEValTest( __LINE__, EEvalFailure, 0,       "cos(6,7)" );   // * too many parameters
    EEValTest( __LINE__, EEvalFailure, 0,       "tan(8,9)" );   // * too many parameters
    EEValTest( __LINE__, EEvalFailure, 0,       "asin(10,0)" ); // * too many parameters
    EEValTest( __LINE__, EEvalFailure, 0,       "acos(1,2)" );  // * too many parameters
    EEValTest( __LINE__, EEvalFailure, 0,       "atan(3,4)" );  // * too many parameters

    // Factorial

    r = tgamma(1+3.456);

    EEValTest( __LINE__, EEvalSuccess, 24,      "4!" );
    EEValTest( __LINE__, EEvalSuccess, 24,      "+4!" );
    EEValTest( __LINE__, EEvalSuccess, 1,       "0!" );
    EEValTest( __LINE__, EEvalSuccess, r,       "3.456!");      // gamma function
    EEValTest( __LINE__, EEvalSuccess, -24,     "-(4!)" );
    EEValTest( __LINE__, EEvalSuccess, 24,      "fact(4)" );
    EEValTest( __LINE__, EEvalSuccess, 1,       "fact(0)" );
    EEValTest( __LINE__, EEvalSuccess, r,       "fact(3.456)");
    EEValTest( __LINE__, EEvalSuccess, -24,     "-fact(4)" );
    EEValTest( __LINE__, EEvalFailure, 0,       "(-4)!" );      // * factorial of negative number
    EEValTest( __LINE__, EEvalFailure, 0,       "!" );          // *
    EEValTest( __LINE__, EEvalFailure, 0,       "fact(-4)" );   // * factorial of negative number
    EEValTest( __LINE__, EEvalFailure, 0,       "fact()" );     // *
    EEValTest( __LINE__, EEvalFailure, 0,       "fact(1,2)" );  // *

    // Exponentiation

    b = 2;
    e = 1;
    e = -e / 3;
    r = pow(b,e);

    EEValTest( __LINE__, EEvalSuccess, 8,               "2^3" );
    EEValTest( __LINE__, EEvalSuccess, pow(2,3.2),      "2^3.2" );
    EEValTest( __LINE__, EEvalSuccess, pow(2,81),       "2^3^4" );
    EEValTest( __LINE__, EEvalSuccess, -27,             "(-3)^3" );
    EEValTest( __LINE__, EEvalSuccess, r,               "2^(-1/3)" );
    EEValTest( __LINE__, EEvalSuccess, 0.5/3,           "2^-1/3" );
    EEValTest( __LINE__, EEvalSuccess, .5/3,            "(2^-1)/3" );
    EEValTest( __LINE__, EEvalSuccess, 0.5/3+1,         "2^-1/3+1" );
    EEValTest( __LINE__, EEvalSuccess, -pow(2,-0.5),    "-1*2^(-1/2)" );
    EEValTest( __LINE__, EEvalFailure, 0,               "^3" );             // *
    EEValTest( __LINE__, EEvalFailure, 0,               "3^" );             // *
    EEValTest( __LINE__, EEvalFailure, 0,               "^" );              // *

    // Equivalent forms

    b = exp(1);
    e = 3.5;
    r = pow(b,e)-exp(3.5);

    EEValTest( __LINE__, EEvalSuccess, 0,   "e        -  exp(1)" );
    EEValTest( __LINE__, EEvalSuccess, r,   "e^3.5    -  exp(3.5)" );   // result slightly different from 0 due to double internal representation
    EEValTest( __LINE__, EEvalSuccess, 0,   "log(3.2) -  log(e,3.2)" );
    EEValTest( __LINE__, EEvalSuccess, 0,   "1.234!   -  fact(1.234)" );
    EEValTest( __LINE__, EEvalSuccess, 0,   "1.2^3.4  -  pow(1.2,3.4)" );

    // Operator precedencedence

    EEValTest( __LINE__, EEvalSuccess, 14,  "2+3*4" );  // + < *
    EEValTest( __LINE__, EEvalSuccess, 19,  "1+2*3^2" );// + < * < ^
    EEValTest( __LINE__, EEvalSuccess, 10,  "1+3^2" );  //
    EEValTest( __LINE__, EEvalSuccess, 15,  "2+3*4+1" );//
    EEValTest( __LINE__, EEvalSuccess, 20,  "1+2*3^2+1");
    EEValTest( __LINE__, EEvalSuccess, 11,  "1+3^2+1" );
    EEValTest( __LINE__, EEvalSuccess, 24,  "2^3*3" );
    EEValTest( __LINE__, EEvalSuccess, 64,  "2^3!" );   // ^ < !
    EEValTest( __LINE__, EEvalSuccess, -6,  "2*-3" );   // unary minus > *
    EEValTest( __LINE__, EEvalSuccess, -1.5,"3/-2" );   // unary minus > /
    EEValTest( __LINE__, EEvalSuccess,1/9.0,"3^-2" );   // unary minus > ^

    // Unary minus precedence

    #if eeval_unary_minus_has_highest_precedence
    // Unary minus has highest precedence
    EEValTest( __LINE__, EEvalSuccess,  9,  "-3^2" );   // (-3)^2
    EEValTest( __LINE__, EEvalSuccess, .25, "2^-2" );   //
    EEValTest( __LINE__, EEvalSuccess,  9,  "5+-2^2" ); // unary minus has always highest precedence
    EEValTest( __LINE__, EEvalSuccess,  4,  "-2^2" );   //
    EEValTest( __LINE__, EEvalFailure,  0,  "-3!" );    // * (-3)!
    #else
    // Unary minus has lowest precedence (with exceptions)
    EEValTest( __LINE__, EEvalSuccess, -9,  "-3^2" );   // -(3^2)
    EEValTest( __LINE__, EEvalSuccess, .25, "2^-2" );   // to make sense unary minus has highest precedence after a binary operator but...
    EEValTest( __LINE__, EEvalSuccess,  1,  "5+-2^2" ); // ...has lowest precedence after `+`
    EEValTest( __LINE__, EEvalSuccess, -4,  "-2^2" );   // -(2^.5)
    EEValTest( __LINE__, EEvalSuccess, -6,  "-3!" );    // -(3!)
    #endif


    // Whitespace (with some of the above)

    b = 2;
    e = 1;
    e = -e / 3;
    r = pow(b,e);

    EEValTest( __LINE__, EEvalSuccess, 2,           "  +  2  " );
    EEValTest( __LINE__, EEvalSuccess, 0,           "2+ - 2" );
    EEValTest( __LINE__, EEvalSuccess, 0,           "2- +2" );
    EEValTest( __LINE__, EEvalSuccess, 42,          "1+\t(2*(3 +\n\n( 4 +5+6) )-1)+6" );
    EEValTest( __LINE__, EEvalSuccess, 1,           "((((((  ((( (( 1)))  ))) ))) ))" );
    EEValTest( __LINE__, EEvalSuccess, -1,          "  -  ((( (((( (((( 1)))))))))))" );
    EEValTest( __LINE__, EEvalSuccess, asin(.123),  "asin   (.123  )" );
    EEValTest( __LINE__, EEvalSuccess, acos(.123),  "acos(  .123)" );
    EEValTest( __LINE__, EEvalSuccess, atan(.123),  "atan(.123  )" );
    EEValTest( __LINE__, EEvalSuccess, 3,           "max  (-1,  2,3)" );
    EEValTest( __LINE__, EEvalSuccess, -1,          "   min(-1,2 ,3   ) " );
    EEValTest( __LINE__, EEvalSuccess, 2,           "average  (1, 2, 3)" );
    EEValTest( __LINE__, EEvalSuccess, 24,          "4  !" );
    EEValTest( __LINE__, EEvalSuccess, 1,           "  0 ! " );
    EEValTest( __LINE__, EEvalSuccess, -24,         "-( 4 !)" );
    EEValTest( __LINE__, EEvalSuccess, r,           "  2  ^(  -1 / 3)" );
    EEValTest( __LINE__, EEvalSuccess, 0.5/3,       " 2 ^ -1 / 3" );
    EEValTest( __LINE__, EEvalSuccess, .5/3,        "(2 ^ -1 \n\n) / 3" );
    EEValTest( __LINE__, EEvalSuccess, 0.5/3+1,     "2^-1/3+1" );
    EEValTest( __LINE__, EEvalFailure, 0,           "2+  +2" );   // *

    // Complicate expressions - tested with http://developer.wolframalpha.com/widgetbuilder/

    EEValTest( __LINE__, EEvalSuccess, 0.999449080234467150824,    ".2^sin(log(e,3)*4*pi/8!)" );
    EEValTest( __LINE__, EEvalSuccess, 2.417851639229258349412E24, "2^3^4-sin((pi*4!)/0.333)" );
    EEValTest( __LINE__, EEvalSuccess, 2.940653537774626349957,    "log(6,atan((pi*4!)/0.333)*123.987)" );

    // Common exceptions (always catched, always raise error)

    EEValTest( __LINE__, EEvalFailure, 0,   "1/0" );    // * division by zero
    EEValTest( __LINE__, EEvalFailure, 0,   "(-1)!" );  // * negative factorial

    // Exceptions catched with eeval_catch_fp_exceptions set to true

    #if eeval_catch_fp_exceptions
    EEValTest( __LINE__, EEvalFailure, 0, "(-2)^(-1/2)" );                          // * complex
    EEValTest( __LINE__, EEvalFailure, 0, "(-3)^3.5" );                             // * complex
    EEValTest( __LINE__, EEvalFailure, 0, "pow(-2,-1/2)");                          // * complex
    EEValTest( __LINE__, EEvalFailure, 0, "(-2)^0.5" );                             // * complex
    EEValTest( __LINE__, EEvalFailure, 0, "log(-6,atan((pi*4!)/0.333)*123.987)" );  // * complex
    EEValTest( __LINE__, EEvalFailure, 0, "9^9^9" );                                // * huge
    EEValTest( __LINE__, EEvalFailure, 0, "-(9^9^9)" );                             // * huge
    EEValTest( __LINE__, EEvalFailure, 0, "average(-9^9^9,9^9^9" );                 // * huge
    EEValTest( __LINE__, EEvalFailure, 0, "max(-(9^9^9),9^9^9" );                   // * huge
    EEValTest( __LINE__, EEvalFailure, 0, "min(-(9^9^9),9^9^9" );                   // * huge
    EEValTest( __LINE__, EEvalFailure, 0, "pow(9,pow(9,9))" );                      // * huge
    #endif

    // All tests passed

    printf( "All tests passed\n");
}



//
// Test function: compare expected status and result with those generated by EEvaluate().
//

void EEValTest( int lineNumber, EEvalStatus expectedStatus, double expectedResult, char *expression )
{
    EEvaluation eval;
    EEvalStatus status;
    double      result;

    status = EEvaluate( &eval, expression, &result );

    if( status == expectedStatus && result == expectedResult ) return;

    printf( "Test at line number %d failed\n\n", lineNumber );
    printf( "Expression: %s\n\n", expression );
    printf( "Expected status is: %s\n", expectedStatus == EEvalSuccess ? "success" : "failure" );
    printf( "Test     status is: %s\n\n",       status == EEvalSuccess ? "success" : "failure" );
    printf( "Expected result is: %f\n", expectedResult );
    printf( "Test     result is: %f\n\n", result );
    if( status == EEvalFailure )
    {
        printf( "Error:\n" );
        EEPrintError( &eval );
        printf( "\n" );
    }

    exit( 1 );
}
#endif