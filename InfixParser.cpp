// Al Ashby
// Jade Sissel
// Alandric Jones
// CS303
// Project 1-A

#include "InfixParser.h"
#include "ExpressionError.h"
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;

// Opening parentheses
const string Infix_Parser::OPEN = "([{";
// Closing parentheses
const string Infix_Parser::CLOSE = ")]}";

// If the next digit should be negative
bool neg;
// What the next digit should be incremented by
int incr;
// What the next digit should be decremented by
int decr;

// Right-hand integer of an expression to be evaluated
int rhs;
// Left-hand integer of an expression to be evaluated
int lhs;

// Last closing parenthesis that's been processed.
// If == '0', that means none has been processed or the parenthetical expression has been evaluated
char closedParen = '0';
// Stores the value of the next binary operator to compare to the binary operator ontop of the stack
string nextBool;
// If last processed token was an operator
bool lastOptr;
// If the last token in the expression has been evaluated
bool expEnd = false;

/** Take in an infix expression and output the result as an integer.
If the expression contains boolean operators, the function will output 0 for false and 1 for true.
@param expression The infix expression
@return The result of the function as an integer
@throws exception if invalid expressionthrow Expression_Error
*/
int Infix_Parser::eval(const string& expression)
{
	// Eliminate whitespace from expression
	string exp = removeWhite(expression);

	// Iterate through tokens of the expression
	for (int i = 0; i < exp.size(); i++) {
		if (operands.empty()) {
			// Throw Expression_Error if expression begins with closing parenthesis
			if (exp[i] == ')' || exp[i] == ']' || exp[i] == '}')
				throw Expression_Error("Expression can’t start with a closing parenthesis @ char: " + i);
			if (exp[i] != '(' && exp[i] != '[' && exp[i] != '{' && exp[i] != '!' && exp[i] != '+'
				&& exp[i] != '-' && !isdigit(exp[i]))
			// Throw Expression_Error if expression begins with unary operator
				throw Expression_Error("Expression can’t start with an operator @ char: " + i);
		}

		// If token is a priority 6 or 7 operator, push it onto operator stack
		if (exp[i] == '*' || exp[i] == '/' || exp[i] == '%' || exp[i] == '^')
		{
			operators.push(exp[i]);
			lastOptr = true;
		}

		// If token is a digit, push the entire integer onto the operand stack
		if (isdigit(exp[i])) {
			i = getDigits(exp, i);
			// If the integer follows the '!' operator, pop the operand stack and push the appropriate value of !integer
			if (!operators.empty()) {
				if (operators.top() == '!') {
					switch (operands.top()) {
					case 0: operands.pop(); operands.push(1); break;
					default: operands.pop(); operands.push(0); break;
					}
					operators.pop();
				}
			}
			// If '^' follows the integer, evaluate the following digits and sequential powers
			if (exp[i + 1] == '^')
				// Increase the iteration to one after the last evaluated digit
				i = processExp(exp, i + 1);
			// If the integer follows priority 6 operators, evaluate with the top two numbers of the operand stack
			if (!operators.empty()) {
				if (exp[i + 1] != '^' &&
					(operators.top() == '*' || operators.top() == '/' || operators.top() == '%'))
					processP6(i);
			}
			lastOptr = false;
		}

		// If the token is '-', evaluate the following tokens to decipher its subtraction, decrement, or negative status
		if (exp[i] == '-')
			// Increase the iteration to the last processed token
			i = minusParser(exp, i);
		// If the token is '+', evaluate the following tokens to decipher its addition or increment status
		if (exp[i] == '+')
			// Increase the iteration to the last processed token
			i = plusParser(exp, i);

		if (OPEN.find(exp[i]) != string::npos) {
			// If the token is an open parenthesis following a negation or increment/decrement operation,
			// push unique operators to the operator stack and the net increment to ensure evaluation after
			// parenthetical expression is resolved
			if (neg) {
				operators.push('$'); // unique character used to make sure its negated as soon as evaluation is over
			}
			if (decr != 0 || incr != 0) {
				operators.push('@'); 
				operands.push(incr - decr);
			}

			// Reset negative, decrement, and increment variables
			neg = false;
			decr = incr = 0;

			// Push open parenthesis to both the operator stack and the boolean operator stack
			operators.push(exp[i]);
			switch (exp[i]) {
			case '(': bools.push("("); break;
			case '[': bools.push("["); break;
			case '{': bools.push("{"); break;
			}
			lastOptr = true;
		}

		if (CLOSE.find(exp[i]) != string::npos) {
			// Throw Expression_Error if an unary operator is followed by a closing parenthesis
			if (lastOptr)
				throw Expression_Error("Closing parenthesis cannot follow operator @ char: " + i);

			closedParen = exp[i];
			
			// Evaluate the expression in the parenthesis up until the opening parenthesis
			process(i);

			// If '^' follows the closing parenthesis, evaluate the following powers with the
			// evaluated answer
			if (exp[i + 1] == '^')
				i = processExp(exp, i + 1);

			// If '!', '^', or priority 6 operators did not get processed due to being
			// followed by an opening parenthesis, process them now
			if (!operators.empty()) {
				// 
				if (operators.top() == '!') {
					switch (operands.top()) {
					case 0: operands.pop(); operands.push(1); break;
					default: operands.pop(); operands.push(0); break;
					}
					operators.pop();
				}
				if (!operators.empty() && operators.top() == '^')
					processExp(i);
				if (!operators.empty() && (operators.top() == '*' || operators.top() == '/' || operators.top() == '%'))
					processP6(i);
			}

			closedParen = '0';
		}

		if (exp[i] == '>' || exp[i] == '<' || exp[i] == '!' || exp[i] == '=' || exp[i] == '|' || exp[i] == '&') {

			// If next operator is a binary operator, evaluate expression before it
			process(i);

			i = getBools(exp, i);
			// If the next binary operator has lower or equal priority than the one
			// on the top of the binary operator stack, evaluate expression before it
			// Otherwise, just push the binary operator onto the binary operator stack
			if (!bools.empty()) {
				if (binary_prec[distance(binary_ops, find(binary_ops, binary_ops+9, nextBool))] <=
					binary_prec[distance(binary_ops, find(binary_ops, binary_ops+9, bools.top()))])
					process(i);
			}
			if (nextBool != "")
				bools.push(nextBool);
			lastOptr = true;
		}
		}

	expEnd = true;

	// Throw Expression_Error if the last token in the expression was an operator
	if (lastOptr)
		throw Expression_Error("Expression cannot end with operator @ char: " + exp.size());

	// If the operators or binary operators stacks are not empty, process them from top to bottom
	if (!operators.empty() || !bools.empty())
		process(exp.size());



	// If the given expression was not empty, return the top of the operand stack
	int num;
	if (!operands.empty())
		num = operands.top();
	else
		num = 0;
	reset();

	return num;
}

/** Take in the expression at a certain index and evaluates the sequential '-'s
for decrement, negative, or subtraction status
@param expression The infix expression, index the current spot of iteration
@return The new place in iteration of the expression
*/
int Infix_Parser::minusParser(string expression, int index) {
	int minusnum = 0;
	// Count number of sequential '-'
	for (int i = index; expression[i] == '-'; i++)
		minusnum++;

	// If the expression begins with '-' or '-' is following another operator
	if (index == 0 || lastOptr) {
		// If the amount of '-'s is even, then they should all be processed as '--'
		if (minusnum % 2 == 0)
			decr += minusnum / 2;
		// If the amount of '-' is odd, then one should be processed as negative and the remaining as '--'
		else {
			neg = true;
			decr += (minusnum - 1) / 2;
		}
	}
	//Otherwise
	else {
		// If the amount of '-'s is even, one should be pushed onto the operator stack, one should be processed as negative,
		// and the remaining as '--'
		if (minusnum % 2 == 0) {
			operators.push('-');
			neg = true;
			decr += (minusnum - 2) / 2;
		}
		// If the amount of '-'s is odd, then one should be pushed onto the operator stack and the rest processed as '--'
		else {
			operators.push('-');
			decr += (minusnum - 1) / 2;
		}
	}
	lastOptr = true;
	return (index + minusnum - 1);
}

/** Take in the expression at a certain index and evaluates the sequential '+'s
for increment or addition statuc
@param expression The infix expression, index the current spot of iteration
@return The new place in iteration of the expression
@throw Expression_Error if invalid use of '+'
*/
int Infix_Parser::plusParser(string expression, int index)
{
	int plusnum = 0;
	// Count number of sequential '+'
	for (int i = index; expression[i] == '+'; i++)
		plusnum++;
	// If the expression begins with '+' or '+' is following another operator
	if (index == 0 || lastOptr) {
		// If the number of '+' is even, process them as '++'
		if (plusnum % 2 == 0)
			incr += plusnum / 2;
		// If the number of '+' is odd, throw Expression_Error as operator '+'
		// cannot begin an expression/follow another operator
		else
			throw Expression_Error("Insufficient amount of digits about '+' operator @ char: " + index);
	}

	//Otherwise 
	else {
		// If the number of '+' is even, throw Expression_Error as operator '+'
		// cannot begin an expression/follow another operator.
		// Likewise, '++' needs to follow an operator or begin an expression
		if (plusnum % 2 == 0) 
			throw Expression_Error("Insufficient amount of digits about '+' operator @ char: " + index);
		// If the number of '+' is odd, push one onto the operator stack and process the rest as '++'
		else {
			operators.push('+');
			incr += (plusnum - 1) / 2;
		}
	}
	lastOptr = true;
	return (index + plusnum - 1);
}

/** Processes the operators and operands in the operators, operands, and binary operators stacks
until the beginning of the expression or until opening parenthesis is reached.
@param index The current iteration spot of the expression
@return None
@throw Expression_Error if mismatched parentheses
*/
void Infix_Parser::process(int index){
	if (!operators.empty()) {
		char optr = operators.top();
		// Don't pop off opening parentheses if their closing parenthesis hasn't been processed
		if (closedParen != '0')
			operators.pop();
		else if (optr != '(' && optr != '[' && optr != '{')
			operators.pop();
		// Evaluate the operators of the operator stack while there are still operators
		// to be evaluated or have not reached the opening parenthesis
		while (optr != '(' && optr != '[' && optr != '{' ) {
			// 
			getLHSRHS(index);
			switch (optr) {
			case '+': operands.push(lhs + rhs); break;
			case '-': operands.push(lhs - rhs); break;
			}
			if (!operators.empty()) {
				optr = operators.top();
				// Don't pop off opening parentheses if their closing parenthesis hasn't been processed
				if (closedParen != '0')
					operators.pop();
				else if (optr != '(' && optr != '[' && optr != '{')
					operators.pop();
			}
			else
				break;
		}
		// If a closing parenthesis has been processed but an opening parenthesis hasn't
		// throw Expression_Error
		if ((closedParen != '0') && (OPEN.find(optr) != CLOSE.find(closedParen)))
			throw Expression_Error("Mismatched closing parenthesis @ char: " + index);
		// If a closing parenthesis has been processed but does not match the processed opening parenthesis,
		// throw Expression_Error
		else if ((optr == '(' || optr == '[' || optr == '{') && expEnd)
			throw Expression_Error("Unclosed parenthesis @ char: " + index);
	}
	// Evaluate the binary operators of the binary operator stack while there are still operators
	// to be evaluated or have not reached the opening parenthesis
	if (!bools.empty()) {
		string boolean = bools.top();
		// Do not remove opening parenthesis if still present in unary operators
		if (closedParen != '0')
			bools.pop();
		else if (boolean != "(" && boolean != "[" && boolean != "{")
			bools.pop();
		while (boolean != "(" && boolean != "[" && boolean != "{") {
			getLHSRHS(index);
			if (boolean == "<")
				operands.push(lhs < rhs);
			else if (boolean == "<=")
				operands.push(lhs <= rhs);
			else if (boolean == ">")
				operands.push(lhs > rhs);
			else if (boolean == ">=")
				operands.push(lhs >= rhs);
			else if (boolean == "==")
				operands.push(lhs == rhs);
			else if (boolean == "!=")
				operands.push(lhs != rhs);
			else if (boolean == "&&")
				operands.push(lhs && rhs);
			else if (boolean == "||")
				operands.push(lhs || rhs);
			if (!bools.empty()) {
				boolean = bools.top();
				// Don't pop off opening parentheses if their closing parenthesis hasn't been processed
				if (closedParen != '0')
					bools.pop();
				else if ( boolean != "(" && boolean != "[" && boolean != "{")
					bools.pop();
			}
			else
				break;
		}

		// If a closing parenthesis has been processed but an opening parenthesis hasn't
		// throw Expression_Error
		if (closedParen != '0' && (OPEN.find(boolean[0]) != CLOSE.find(closedParen)))
			throw Expression_Error("Mismatched closing parenthesis @ char: " + index);

		// If a closing parenthesis has been processed but does not match the processed opening parenthesis,
		// throw Expression_Error
		else if ((boolean == "(" || boolean == "[" || boolean == "{") && expEnd)
			throw Expression_Error("Unclosed parenthesis @ char: " + index);
	}

	// If special operators for negating or incrementing/decrementing are on top of the operators stack
	// process them and push answer to operand stack
	if (!operators.empty()) {
		if (operators.top() == '@') {
			getLHSRHS(index);
			operands.push(lhs + rhs);
		}
		if (operators.top() == '$') {
			int top = operands.top();
			operands.pop();
			operands.push(top * -1);
		}
	}
}

/** Evaluates priority 6 operators on top of the operators stack.
@param index The current point of iteration of the expression
@return None
@throws Expression_Error if division by zero
*/
void Infix_Parser::processP6(int index)
{
	char op = operators.top();
	operators.pop();
	getLHSRHS(index);
	switch (op){
	case '*' : operands.push(lhs*rhs); break;
		// If the operator is / and the right-hand side operator is 0, throw excepton
	case '/': if (rhs == 0) throw Expression_Error("Division by zero @ char: " + index); else operands.push(lhs / rhs); break;
	case '%' : operands.push(lhs%rhs); break;
	}
}

/** Take in an infix expression at a current index and process the current and following sequential exponents.
@param expression The infix expression; index The Current point of iteration
@return The new point of iteration for the expression
@throws Expression_Error if expression begins with '^'
*/
int Infix_Parser::processExp(string expression, int index){
	lastOptr = true;
	int i = index;
	// If the expression begins with '^', throw Expression_Error
	if (index == 0)
		throw Expression_Error("Cannot begi expression with operator @ char: " + index);
	// If a '+' follows the '^',  evaluate any increments
	if (expression[i+1] == '+')
		i = plusParser(expression, i+1);
	// If a '-' follows the '^', evaluate any decrements/negations
	if (expression[i+1] == '-')
		i = minusParser(expression, i+1);
	// If a digit follows the '^', process the entire number
	if (isdigit(expression[i+1]))
		i = getDigits(expression, i+1);
	// If another '^' or an opening parenthesis follows the evaluated digit
	// push '^' onto the operator stack and return
	if (expression[i+1] == '^' || expression[i+1] == '(' || expression[i+1] == '[' || expression[i+1] == '{'){
		operators.push('^');
		return i;
	}
	// Evaluate power
	getLHSRHS(i);
	operands.push(pow(lhs,rhs));
	return i;
}

/** Evaluates an exponent if '^' is on top of the stack and has yet to be processed due
to either a following opening parenthesis or another '^'.
@ param index Current place of iteration
@ return None
*/
void Infix_Parser::processExp(int index){
	getLHSRHS(index);
	operands.push(pow(lhs,rhs));
	operators.pop();
}

/** Sets the left-hand side and right-hand side operands for evaluation.
@param index Current point of iteration
@return None
@throw Expression_Error if not enough operands for an operator
*/
void Infix_Parser::getLHSRHS(int index)
{
	if (operands.empty())
		throw Expression_Error("Two few operands about operator @ char: " + index);
	rhs = operands.top();
	operands.pop();
	if (operands.empty())
		throw Expression_Error("Two few operands about operator @ char: " + index);
	lhs = operands.top();
	operands.pop();

}

/** Takes an infix expression at a certain location and pushes the evaluated number onto the operands stack.
@ param expression The infix expression; index The current point of iteration
@ return The new point of iteration
*/
int Infix_Parser::getDigits(string expression, int index){
	int i = index;
	string exp = expression;
	// Erase any preceding tokens in the expression
	if (index != 0)
		exp.erase(0, index);
	istringstream tokens(exp);
  	char next_digit;
	int num;
	// Read in tokens and, while they are still digits, read them into one integer
  	while ((tokens >> next_digit) && isdigit(next_digit)) {
		tokens.putback(next_digit);
		tokens >> num;
		i++;
	}
	// Increase/decrease the integer by the current value of the increment/decrement variables
	num += incr;
	num -= decr;
	// If the next number is to be negative, convert it to such
	if (neg)
		num *= -1;
	// Push the number
	operands.push(num);
	// Reset the negative/increment/decrement variables
	neg = false;
	incr = decr = 0;
	return i-1;
}

/** Takes an expression at the current point of iteration and
pushes the appropriate binary operation onto the binary operators stack.
@param expression The infix expression; index The current point of iteration
@return The new point of iteration
@throws Expression_Error
*/
int Infix_Parser::getBools(string expression, int index) {
	int i = index;
	// If an operator was processed just before the current token, throw Expression_Error
	if (lastOptr && (expression[i] != '!' || expression[i+1] == '='))
		throw Expression_Error("Two binary operators in a row @ char: " + i);
	if (expression[i] == '<' || expression[i] == '>' || expression[i] == '=' || expression[i] == '!') {
		// If the following token is '=', push the comparator version of the binary operator onto the binary operator stack
		if (expression[i + 1] == '=') {
			switch (expression[i]) {
			case '<': nextBool = "<="; break;
			case '>': nextBool = ">="; break;
			case '=': nextBool = "=="; break;
			case '!': nextBool = "!="; break;
			}
			return i + 1;
		}
		else {
			switch (expression[i]) {
			case '<': nextBool = "<"; break;
			case '>': nextBool = ">"; break;
			case '!': operators.push('!'); break;
				// If only a single '=' is read, throw Expression_Error
			case '=': throw Expression_Error("Assignment operator '=' used at @ char: " + i); break;
			}
			return i;
		}
	}
	if (expression[i] == '&') {
		if (expression[i + 1] == '&') {
			nextBool = "&&";
			return i + 1;
		}
		// If only a single '&' is read, throw Expression_Error
		else
			throw Expression_Error("Incomplete boolean AND '&&' @ char: " + index);
	}
	if (expression[i] == '|') {
		if (expression[i + 1] == '|') {
			nextBool = "||";
			return i + 1;
		}
		// If only a single '|' is read, throw Expression_Error
		else
			throw Expression_Error("Incomplete boolean OR '||' @ char: " + index);
	}
}

/** Takes an expression and removes all instances of the whitespace character ' '.
@param expression The infix expression
@return The new expression without spaces
*/
string Infix_Parser::removeWhite(string expression) {
	char next_char;
	string exp;
	istringstream tokens(expression);
	// Read in tokens and, if they're not ' ', add them to the new expression
	while (tokens >> next_char) {
		if (next_char != ' ')
			exp += next_char;
	}
	return exp;
}

/** Resets all the variables to original values.
@return None
*/
void Infix_Parser::reset() {
	while (!operands.empty()) {
		operands.pop();
	}
	neg = false;
	incr = decr = rhs = lhs = 0;
	closedParen = '0';
	nextBool.clear();
	lastOptr = false;
	expEnd = false;
}