// Al Ashby
// Jade Sissel
// Alandric Jones
// CS303
// Project 1-A

#pragma once
#include <string>
#include <stack>
using namespace std;

class Infix_Parser {
	// Public member functions
public:
	/** Take in an infix expression and output the result as an integer.
	If the expression contains boolean operators, the function will output 0 for false and 1 for true.
	@param expression The infix expression
	@return The result of the function as an integer
	@throws Expression_Error if invalid expression
	*/
	int eval(const string& expression);

	// Private member functions
private:
	/** Take in the expression at a certain index and evaluates the sequential '-'s
	for decrement, negative, or subtraction status
	@param expression The infix expression, index the current spot of iteration
	@return The new place in iteration of the expression
	*/
	int minusParser(string expression, int index);

	/** Take in the expression at a certain index and evaluates the sequential '+'s
	for increment or addition statuc
	@param expression The infix expression, index the current spot of iteration
	@return The new place in iteration of the expression
	@throw Expression_Error if invalid use of '+'
	*/
	int plusParser(string expression, int index);

	/** Processes the operators and operands in the operators, operands, and binary operators stacks
	until the beginning of the expression or until opening parenthesis is reached.
	@param index The current iteration spot of the expression
	@return None
	@throw Expression_Error if mismatched parentheses
	*/
	void process(int index);

	/** Sets the left-hand side and right-hand side operands for evaluation.
	@param index Current point of iteration
	@return None
	@throw Expression_Error if not enough operands for an operator
	*/
	void getLHSRHS(int index);

	/** Evaluates priority 6 operators on top of the operators stack.
	@param index The current point of iteration of the expression
	@return None
	@throws Expression_Error if division by zero
	*/
	void processP6(int index);

	/** Take in an infix expression at a current index and process the current and following sequential exponents.
	@param expression The infix expression; index The Current point of iteration
	@return The new point of iteration for the expression
	@throws Expression_Error if expression begins with '^'
	*/
	int processExp(string expression, int index);

	/** Evaluates an exponent if '^' is on top of the stack and has yet to be processed due
	to either a following opening parenthesis or another '^'.
	@ param index Current place of iteration
	@ return None
	*/
	void processExp(int index);

	/** Takes an infix expression at a certain location and pushes the evaluated number onto the operands stack.
	@ param expression The infix expression; index The current point of iteration
	@ return The new point of iteration
	*/
	int getDigits(string expression, int index);

	/** Takes an expression at the current point of iteration and
	pushes the appropriate binary operation onto the binary operators stack.
	@param expression The infix expression; index The current point of iteration
	@return The new point of iteration
	@throws Expression_Error for invalid binary operators
	*/
	int getBools(string expression, int index);

	/** Takes an expression and removes all instances of the whitespace character ' '.
	@param expression The infix expression
	@return The new expression without spaces
	*/
	string removeWhite(string expression);

	/** Resets all the variables to original values.
	@return None
	*/
	void Infix_Parser::reset();

	// Data fields
	stack<string> bools;
	stack<int> operands;
	stack<char> operators;

	static const string OPEN;
	static const string CLOSE;

	string binary_ops[9] = { "!", ">", ">=", "<", "<=", "==", "!=", "&&", "||" };
	int binary_prec[9] = { 0 , 1, 1, 1, 1, 2, 2, 3, 3 };
};