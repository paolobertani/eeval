//
//  eeval
//  version 1.0
//
//  a math expression evaluator
//
//  eeval.c
//
//  eeval functions
//
//  Copyright (c) 2016 Paolo Bertani - Kalei S.r.l.
//  Licensed under the FreeBSD 2-clause license
//



#include "eeval.h"

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <inttypes.h>



// Evaluates an expression.
// The function returns a status of success or failure
// The result is in `*result`

EEvalStatus EEvaluate( EEvaluation *eval,       // the EEvaluation structure
                       const char  *expression, // the expression as a null terminated C string
                       double      *result )    // RETURN: the result of the evaluation
{
    eval->expression = eval->cursor = expression;
    eval->roundBracketsCount = 0;
    eval->result = 0;
    eval->error = NULL;

    eval->result = EEvalAddends( eval, -1, true, false, NULL );

    *result = eval->result;

    if( eval->error )
    {
        *result = 0;
        return EEvalFailure;
    }
    else
    {
        eval->error = "";
        return EEvalSuccess;
    }
}



// Utility function to print the error after an
// evaluation failed.
// Prints the error description, the expression
// and a caret under the expression approximately
// where the error occurred.

void EEPrintError( EEvaluation *eval )
{
    if( eval->error && strlen( eval->error ) > 0 )
    {
        fprintf( stderr, "%s\n", eval->error );
        fprintf( stderr, "%s\n", eval->expression );
        fprintf( stderr, "%*c^\n", (int)( eval->cursor - eval->expression ), ' ' );
    }
}



// ***********************
// PRIVATE FUNCTIONS BELOW
// ***********************



// Evaluates a single value or expression A0 or
// sequence of 2 or more addends:
// A1 - A2 [ + A3 [ - A4 ... ] ]
// Addends can be a single values or expressions with
// higher precedence. In the second case the expression is evaluated first.
// "breakOn" parameters define cases where the function must exit.

double EEvalAddends( EEvaluation *eval,
                    int64_t breakOnRoundBracketsCount, // If open brackets count goes down to this count then exit;
                    bool breakOnETEof,                 // exit if the end of the string '\0' is met;
                    bool breakOnETcom,                 // exit if a comma is met;
                    EEToken *tokenThatCausedBreak )    // if pointer is not null the token/symbol that caused the function to exit.
{
    EEToken leftOp;
    EEToken rightOp;
    double  value;
    double  result;

    // Let's pretend we already computed
    // 0 + ...

    result = 0;
    rightOp = ETSum;

    do
    {
        leftOp = rightOp;

        // [ Each addend A is treated as a (potential and higher-precedence)
        //   multiplication and evaluated as 1 * A with the function below ]
        value = EEvalFactors( eval, 1, ETMul, false, &rightOp );
        if( eval->error ) return 0;

        result = leftOp == ETSum ? ( result + value ) : ( result - value );

        // ...and go on as long there are sums ands subs.
    }
    while( rightOp == ETSum || rightOp == ETSub );

    // A round close bracket:
    // check for negative count.

    if( rightOp == ETrbc )
    {
        eval->roundBracketsCount--;
        if( eval->roundBracketsCount < 0 )
        {
            eval->error = "unexpected close round bracket";
            return 0;
        }
    }

    // Returns the token that caused the function to exit

    if( tokenThatCausedBreak )
    {
        *tokenThatCausedBreak = rightOp;
    }

    // Check if must exit

    if( ( eval->roundBracketsCount == breakOnRoundBracketsCount ) || ( breakOnETEof && rightOp == ETEof ) || ( breakOnETcom && rightOp == ETcom ) )
    {
        if( eexception(result) )
        {
            eval->error = "result is complex or too big";
            return 0;
        }

        return result;
    }

    // If not it's an error.

    switch( rightOp )
    {
        case ETEof:
            eval->error = "unexpected end of expression";
            break;

        case ETrbc:
            eval->error = "unexpected close round bracket";
            break;

        case ETcom:
            eval->error = "unexpeced comma";
            break;

        default:
            eval->error = "unexpeced symbol";
            break;
    }

    return 0;
}



// Evaluates a sequence of 1 or more multiplies or divisions
// F1 [ * F2  [ / F3 [ * F4 ... ] ] ]
// Where Fn is a value or a higher precedence expression.

double EEvalFactors( EEvaluation *eval,
                     double      leftValue, // The value (already fetched) on the left to be multiplied(divided);
                     EEToken     op,        // is it multiply or divide;
                     bool        isExponent,// is an exponent being evaluated ?
                     EEToken     *leftOp )  // RETURN: factors are over, this is the next operator (token).
{
    EEToken token,
            nextOp;

    double  rightValue;
    double  sign;

    do
    {
        rightValue = EEvalToken( eval, &token );
        if( eval->error ) return 0;

        // Unary minus or plus ?
        // store the sign and get the next token

        if( token == ETSub )
        {
            sign = -1;
            rightValue = EEvalToken( eval, &token );
            if( eval->error ) return 0;
        }
        else if( token == ETSum )
        {
            sign = 1;
            rightValue = EEvalToken( eval, &token );
            if( eval->error ) return 0;
        }
        else
        {
            sign = 1;
        }

        // Open round bracket?
        // The expression between brackets is evaluated.

        if( token == ETrbo )
        {
            eval->roundBracketsCount++;

            rightValue = EEvalAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;

            token = ETVal;
        }

        // A function ?

        if( token == ETCos || token == ETSin || token == ETTan || token == ETASi || token == ETACo || token == ETATa || token == ETFac || token == ETLog || token == ETExp || token == ETPow || token == ETMax || token == ETMin || token == ETAvg )
        {
            rightValue = EEvalFunction( eval, token );
            if( eval->error ) return 0;

            token = ETVal;
        }

        // Excluded previous cases then
        // the token must be a number.

        if( token != ETVal )
        {
            eval->error = "expected value";
            return 0;
        }

        // Get beforehand the next token
        // to see if it's an exponential or factorial operator

        EEvalToken( eval, &nextOp );
        if( eval->error ) return 0;

        // Unary minus precedence (highest/lowest) affects this section of code

        if( nextOp == ETFct )
        {
            #if eeval_unary_minus_has_highest_precedence
                rightValue = EEvalFactorial( eval, rightValue * sign, &nextOp );
                sign = 1;
            #else
                rightValue = EEvalFactorial( eval, rightValue, &nextOp );
            #endif
            if( eval->error ) return 0;
        }

        if( nextOp == ETExc )
        {
            #if eeval_unary_minus_has_highest_precedence
                rightValue = EEvalExponentiation( eval, rightValue * sign, &nextOp );
                sign = 1;
            #else
                rightValue = EEvalExponentiation( eval, rightValue, &nextOp );
            #endif
            if( eval->error ) return 0;
        }

        // multiplication/division is finally
        // calculated

        if( op == ETMul )
        {
            leftValue = leftValue * rightValue * sign;
        }
        else
        {
            if( rightValue == 0 )
            {
                eval->error = "division by zero";
                return 0;
            }
            leftValue = leftValue / rightValue * sign;
        }

        if( eexception( leftValue ) )
        {
            eval->error = "result is too big";
            return 0;
        }

        // The next operator has already been fetched.

        op = nextOp;

        // Go on as long multiply or division operators are met...
        // ...unless an exponent is evaluated
        // (because exponentiation ^ operator have higher precedence)
    }
    while( ( op == ETMul || op == ETDiv ) && ! isExponent );


    *leftOp = op;

    return leftValue;
}



// Evaluates the expession(s) (comma separated if multiple)
// inside the round brackets then computes the function
// specified by the token `func`.

double EEvalFunction( EEvaluation *eval, EEToken func )
{
    double   result,
             result2;

    uint16_t count;

    EEToken  tokenThatCausedBreak,
             token;

    // Eat an open round bracket and count it

    EEvalToken( eval, &token );
    if( eval->error ) return 0;

    if( token != ETrbo )
    {
        eval->error = "expected open round bracket after function name";
        return 0;
    }

    eval->roundBracketsCount++;

    switch( func )
    {
        case ETSin:
            result = EEvalAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = sin( result );
            break;

        case ETCos:
            result = EEvalAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = cos( result );
            break;

        case ETTan:
            result = EEvalAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = tan( result );
            break;

        case ETASi:
            result = EEvalAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = asin( result );
            break;

        case ETACo:
            result = EEvalAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = acos( result );
            break;

        case ETATa:
            result = EEvalAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = atan( result );
            break;

        case ETFac:
            result = EEvalAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            if( result < 0 )
            {
                eval->error = "attempt to evaluate factorial of negative number";
            }
            else
            {
                result = tgamma( 1 + result );
            }
            break;

        case ETExp:
            result = EEvalAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = exp( result );
            break;

        case ETPow:
            result = EEvalAddends( eval, -1, false, true, NULL );
            if( eval->error ) return 0;
            result2 = EEvalAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
            if( eval->error ) return 0;
            result = pow( result, result2 );
            break;

        case ETLog:
            result = EEvalAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
            if( eval->error ) return 0;
            if( tokenThatCausedBreak == ETrbc )
            {
                // log(n) with one parameter
                result = log( result );
            }
            else
            {
                result2 = EEvalAddends( eval, eval->roundBracketsCount - 1, false, false, NULL );
                if( eval->error ) return 0;
                result = log( result2 ) / log( result );
            }
            break;

        case ETMax:
            result = EEvalAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
            if( eval->error ) return 0;
            while( tokenThatCausedBreak == ETcom )
            {
                result2 = EEvalAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
                if( eval->error ) return 0;

                if( result2 > result )
                {
                    result = result2;
                }
            }
            break;

        case ETMin:
            result = EEvalAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
            if( eval->error ) return 0;
            while( tokenThatCausedBreak == ETcom )
            {
                result2 = EEvalAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
                if( eval->error ) return 0;

                if( result2 < result )
                {
                    result = result2;
                }
            }
            break;

        case ETAvg:
            result = EEvalAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
            if( eval->error ) return 0;
            count = 1;
            while( tokenThatCausedBreak == ETcom )
            {
                result2 = EEvalAddends( eval, eval->roundBracketsCount - 1, false, true, &tokenThatCausedBreak );
                if( eval->error ) return 0;

                result += result2;
                count++;
            }
            result = result / (double)count;
            break;

        default:
            result = 0;
            break;
    }

    if( eexception( result ) )
    {
        eval->error = "result is complex or too big";
        return 0;
    }

    return result;
}



// Evaluates an exponentiation.

double EEvalExponentiation( EEvaluation *eval,
                            double      base,      // The base has already been fetched;
                            EEToken     *rightOp ) // RETURN: the token (operator) that follows.
{
    double exponent,
           result;

    exponent = EEvalFactors( eval, 1, ETMul, true, rightOp );
    if( eval->error ) return 0;

    result = pow( base, exponent );
    if( eexception( result ) )
    {
        eval->error = "result is complex or too big";
        return 0;
    }

    return result;
}



// Evaluates a factorial using the Gamma function.

double EEvalFactorial( EEvaluation *eval,
                       double      value,     // The value to compute has already been fetched;
                       EEToken     *rightOp ) // RETURN: the token (operator) that follows.
{
    double result;

    if( value < 0 )
    {
        eval->error = "attempt to evaluate factorial of negative number";
        *rightOp = ETErr;
        return 0;
    }

    result = tgamma( value + 1 );

    if( eexception( result ) )
    {
        eval->error = "result is complex or too big";
        return 0;
    }

    EEvalToken( eval, rightOp );
    if( eval->error ) return 0;

    return result;
}



// Parses the next token and advances the cursor.
// The function returns a number if the token is a value or a constant.
// Whitespace is ignored.

double EEvalToken( EEvaluation *eval,
                   EEToken     *token ) // RETURN: the token.
{
    EEToken  t;
    double   v;

    t = ETBlk;
    v = 0;

    while( t == ETBlk )
    {
        if( ( *eval->cursor >= '0' && *eval->cursor <= '9' ) || *eval->cursor == '.' )
        {
            v = EEvalValue( eval );
            if( eval->error )
            {
                t = ETErr;
                return t;
            }
            else
            {
                t = ETVal;
            }
            break;
        }
        else
        {
            switch( *eval->cursor )
            {
                case '\n':
                case '\r':
                case '\t':
                case ' ':
                    t = ETBlk;
                    eval->cursor++;
                    break;

                case '+':
                    EEvalPlusToken( eval, &t);
                    break;

                case '-':
                    t = ETSub;
                    eval->cursor++;
                    break;

                case '*':
                    t = ETMul;
                    eval->cursor++;
                    break;

                case '/':
                    t = ETDiv;
                    eval->cursor++;
                    break;

                case '^':
                    t = ETExc;
                    eval->cursor++;
                    break;

                case '!':
                    t = ETFct;
                    eval->cursor++;
                    break;

                case '(':
                    t = ETrbo;
                    eval->cursor++;
                    break;

                case ')':
                    t = ETrbc;
                    eval->cursor++;
                    break;

                case '\0':
                    t = ETEof;
                    eval->cursor++;
                    break;

                case ',':
                    t = ETcom;
                    eval->cursor++;
                    break;

                case 'e':
                    if( strncmp( eval->cursor, "exp", 3 ) == 0 )
                    {
                        t = ETExp;
                        eval->cursor += 3;
                    }
                    else
                    {
                        v = exp( 1 );
                        t = ETVal;
                        eval->cursor++;
                    }
                    break;

                case 'f':
                    if( strncmp( eval->cursor, "fact", 4 ) == 0 )
                    {
                        t = ETFac;
                        eval->cursor += 4;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 'p':
                    if( strncmp( eval->cursor, "pi", 2 ) == 0 )
                    {
                        v = M_PI;
                        t = ETVal;
                        eval->cursor += 2;
                    }
                    else if( strncmp( eval->cursor, "pow", 3 ) == 0 )
                    {
                        t = ETPow;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 'c':
                    if( strncmp( eval->cursor, "cos", 3 ) == 0 )
                    {
                        t = ETCos;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 's':
                    if( strncmp( eval->cursor, "sin", 3 ) == 0 )
                    {
                        t = ETSin;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 't':
                    if( strncmp( eval->cursor, "tan", 3 ) == 0 )
                    {
                        t = ETTan;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 'l':
                    if( strncmp( eval->cursor, "log", 3 ) == 0 )
                    {
                        t = ETLog;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 'm':
                    if( strncmp( eval->cursor, "max", 3 ) == 0 )
                    {
                        t = ETMax;
                        eval->cursor += 3;
                    }
                    else if( strncmp( eval->cursor, "min", 3 ) == 0 )
                    {
                        t = ETMin;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;

                case 'a':
                    if( strncmp( eval->cursor, "asin", 4 ) == 0 )
                    {
                        t = ETASi;
                        eval->cursor += 4;
                    }
                    else if( strncmp( eval->cursor, "acos", 4 ) == 0 )
                    {
                        t = ETACo;
                        eval->cursor += 4;
                    }
                    else if( strncmp( eval->cursor, "atan", 4 ) == 0 )
                    {
                        t = ETATa;
                        eval->cursor += 4;
                    }
                    else if( strncmp( eval->cursor, "average", 7 ) == 0 )
                    {
                        t = ETAvg;
                        eval->cursor += 7;
                    }
                    else if( strncmp( eval->cursor, "avg", 3 ) == 0 )
                    {
                        t = ETAvg;
                        eval->cursor += 3;
                    }
                    else
                    {
                        t = ETErr;
                    }
                    break;


                default:
                    t = ETErr;
                    break;
            }
        }
    }

    if( t == ETErr )
    {
        eval->error = "unexpected symbol";
    }

    *token = t;

    return v;
}



// Parses what follows a (already fetched) plus token
// ensuring that two consecutive plus are not present.
// Expressions such as 2++2 (binary plus
// followed by unitary plus) are not allowed.
// Advances the cursor.
// Always returns 0.

double EEvalPlusToken( EEvaluation *eval, EEToken *token )
{
    do
    {
        eval->cursor++;
    } while( *eval->cursor == ' ' || *eval->cursor == '\n' || *eval->cursor == '\r' || *eval->cursor == '\t' );

    if( *eval->cursor == '+' )
    {
        *token = ETErr;
    }
    else
    {
        *token = ETSum;
    }

    return 0;
}



// Parses a number and advances the cursor.
// The cursor is positioned after an eventually
// `+` or `-` operator that comes before the value.

double EEvalValue( EEvaluation *eval )
{
    char   *endptr;
    double value;

    value = strtod( eval->cursor, &endptr );

    if( endptr == eval->cursor )
    {
        eval->error = "expected value";
        value = 0;
    }
    else
    {
        eval->cursor = endptr;

        if( eexception( value ) )
        {
            eval->error = "value is too big";
            return 0;
        }
    }

    return value;
}