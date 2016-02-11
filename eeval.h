//
//  eeval
//  version 1.0
//
//  a math expression evaluator
//
//  eeval.h
//
//  eeval header
//
//  Copyright (c) 2016 Paolo Bertani - Kalei S.r.l.
//  Licensed under the FreeBSD 2-clause license
//



#ifndef eeval_main_h
#define eeval_main_h



#include <stdbool.h>
#include <inttypes.h>



// build settings


// TEST SUITE

#ifndef eeval_test
#define eeval_test true     // build with test suite (constant is overridden by makefile)
#endif


// FLOATING POINT EXCEPTIONS CATCHING

// leave to true (default) to catch exceptions on math operations (ex. overflows)
#define eeval_catch_fp_exceptions true


// PRECEDENCE OF UNARY MINUS OPERATOR

// leave to true (default) to give unitary minus highest precedece like in most programming languages
// set to false to give lowest precedence like in math notation (see README.md for details)
#define eeval_unary_minus_has_highest_precedence true




// tokens

enum EEToken
{
    ETBlk,   // white space, tab, newline...
    ETErr,   // unrecognized token (error)
    ETEof,   // end of string (null termination character)
    ETSum,   // +
    ETSub,   // -
    ETMul,   // *
    ETDiv,   // /
    ETExc,   // ^ exponentiation
    ETFct,   // ! factorial
    ETSin,   // sin(r)
    ETCos,   // cos(r)
    ETTan,   // tan(r)
    ETASi,   // arcsin(n)
    ETACo,   // arccos(n)
    ETATa,   // arctan(n)
    ETFac,   // fact(n) - factorial, equivalent to n!
    ETExp,   // exp(n) - equivalent to e^n
    ETPow,   // pow(b,n) - equivalent to b^n
    ETLog,   // log(b, n) logarithm of n with base b - or log(n) natural logarithm of n
    ETMax,   // max(n1, n2, n3...) maximum of 1 or more numbers
    ETMin,   // min(n1, n2, n3...) minimum of 1 or more numbers
    ETAvg,   // average(n1, n2, n3...) or avg(n1, ...) average of 1 or more numbers
    ETrbo,   // round bracket open  (round bracket count increases)
    ETrbc,   // round bracket close (round bracket count decreases)
    ETcom,   // comma - argument separator inside functions
    ETVal    // a number in scientific notation (1 .1 0.1 1.2E-3) or `e` (euler number) or `pi`
};
typedef enum EEToken EEToken;



enum EEvalStatus
{
    EEvalFailure = 0,
    EEvalSuccess = 1
};
typedef enum EEvalStatus EEvalStatus;



struct EEvaluation
{
    const char  *expression;
    const char  *cursor;
    double      result;
    int64_t     roundBracketsCount;
    const char  *error;
};
typedef struct EEvaluation EEvaluation;



// Public

EEvalStatus EEvaluate    ( EEvaluation *eval, const char *expression, double *result );
void        EEPrintError ( EEvaluation *eval );



// Private

double      EEvalAddends        ( EEvaluation *eval, int64_t breakOnRoundBracketsCount, bool breakOnETEof, bool breakOnETcom, EEToken *tokenThatCausedBreak );
double      EEvalFactors        ( EEvaluation *eval, double leftValue, EEToken op, bool isExponent, EEToken *leftOp );
double      EEvalFunction       ( EEvaluation *eval, EEToken func );
double      EEvalExponentiation ( EEvaluation *eval, double base, EEToken *rightOp );
double      EEvalFactorial      ( EEvaluation *eval, double value, EEToken *rightOp );
double      EEvalToken          ( EEvaluation *eval, EEToken *token );
double      EEvalPlusToken      ( EEvaluation *eval, EEToken *token );
double      EEvalValue          ( EEvaluation *eval );



// exception catcher

#if eeval_catch_fp_exceptions
#define eexception(n) (isnan(n)||(n)==HUGE_VAL||(n)==INFINITY||(n)==-HUGE_VAL||(n)==-INFINITY)
#else
#define eexception(n) (false)
#endif



// Test suite included ?

#if eeval_test == true
void        EEvalExecuteTests ();
void        EEValTest       ( int lineNumber, EEvalStatus expectedStatus, double expectedResult, char *expression );
#endif
#endif