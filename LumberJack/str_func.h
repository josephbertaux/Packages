#ifndef STR_FUNC_H_
#define STR_FUNC_H_

//Standard includes
#include <stdio.h> //standard IO
#include <string.h> //string manipulations
#include <math.h> //alternate math functions
#include <fstream> //writing output to files
#include <iostream> //writing output to console
#include <string>
#include <vector>

//Root includes
#include "TTree.h"

//Local includes

using namespace std;

template <class T>
class NamedVariable
{
public:
	string name;
	T* address;
};

template <class T>
class NamedFunction
{
public:
	string name;
	T (*func)(T*);
};

template <class T>
class FunctionNode;

template <class T>
class FunctionNode
{
public:
	NamedFunction<T>* self;

	int num_children;
	FunctionNode<T>* children;
	T* child_vals;

	T Evaluate()
	{
		for(int i = 0; i < num_children; i++)
		{
			child_vals[i] = children[i].Evaluate();
		}
		return self->func(child_vals);
	}
};

vector<string> GetExpressionArgs(string expr)
{
	vector<string> args = {};

	string temp = "";
	int i = 0;

	//remove spaces from expr
	for(i = 0; i < (int)expr.length(); i++)
	{
		if(expr.substr(i,1) == " ")continue;

		temp += expr.substr(i,1);
	}
	expr = temp;

	temp = "";
	int depth = 0;
	for(i = 0; i < (int)expr.length(); i++)
	{
		if(expr.substr(i, 1) == ")")
		{
			depth--;
		}
		if(depth >= 1)
		{
			if(depth == 1 and expr.substr(i,1) == "," and temp.length() > 0)
			{
				args.push_back(temp);
				temp = "";
			}
			else
			{
				temp += expr.substr(i,1);
			}
		}
		if(expr.substr(i, 1) == "(")
		{
			depth++;
		}
	}
	if(temp.length() > 0)
	{
		args.push_back(temp);
	}

	return args;
}

vector<string> GetExpressionVars(string expr)
{
	vector<string> vars = {};
	
	vector<string> args = GetExpressionArgs(expr);
	if(0 < (int)args.size())
	{
		int i = 0;
		int j = 0;
		int k = 0;
		bool found = false;

		for(i = 0; i < (int)args.size(); i++)
		{
			vector<string> child_vars = GetExpressionVars(args[i]);
			for(j = 0; j < (int)child_vars.size(); j++)
			{
				found = false;
				for(k = 0; k < (int)vars.size(); k++)
				{
					if(vars[k] == child_vars[j])
					{
						found = true;
					}
				}
				if(!found)vars.push_back(child_vars[j]);
			}
		}
	}
	else
	{
		vars.push_back(expr);
	}

	return vars;
}

template <class T>
FunctionNode<T> ParseExpression(string expr, vector<NamedFunction<T>*> named_funcs, vector<NamedVariable<T>*> named_vars)
{
	//The string expr should only consist of function names (the function could return a variable or constant), parenthesis, and commas

	FunctionNode<T> fn;

	int i;
	bool found;
	string name = "";
	vector<string> child_exprs = {};

	//parse the string for the first time to determine the outermost function name and the depth of the child expressions
	for(i = 0; i < (int)expr.length(); i++)
	{
		if(expr.substr(i, 1) == " ")continue; //skip spaces
		if(expr.substr(i, 1) == "(")break;

		name += expr.substr(i, 1);
	}

	child_exprs = GetExpressionArgs(expr);
	fn.num_children = (int)child_exprs.size();

	if(fn.num_children > 0) //There are arguments, so this is truly a function
	{
		//Find the function with the corresponding name
		found = false;
		for(i = 0; i < (int)named_funcs.size(); i++)
		{
			if(named_funcs[i]->name == name)
			{
				found = true;
				fn.self = named_funcs[i];
				break;
			}
		}
		if(!found)
		{
			cout << "Argument '" << name << "' has arguments" << endl;
			cout << "However, no function '" << name << "' was found in list of named functions" << endl;
		}

		//Now assign the children function nodes
		fn.child_vals = new T[fn.num_children];
		fn.children = new FunctionNode<T>[fn.num_children];

		for(i = 0; i < fn.num_children; i++)
		{
			fn.children[i] = ParseExpression(child_exprs[i], named_funcs, named_vars);
		}
	}
	else //if it has no children, it is a variable, and 'name' stores its name
	{
		//Find the identity function
		found = false;
		for(i = 0; i < (int)named_funcs.size(); i++)
		{
			if(named_funcs[i]->name == "I")
			{
				found = true;
				fn.self = named_funcs[i];
				break;
			}
		}
		if(!found)
		{
			cout << "Identity function 'I' not contained in list of named functions" << endl;
		}

		fn.children = 0x0;

		//find the variable of the same name name, and set its address to child vals
		//change to correspond to branches for ROOT functionality
		found = false;
		for(i = 0; i < (int)named_vars.size(); i++)
		{
			if(named_vars[i]->name == name)
			{
				fn.child_vals = named_vars[i]->address;
				found = true;
			}
		}
		if(!found)
		{
			cout << "Argument '" << name << "' has no arguments" << endl;
			cout << "However, no variable '" << name << "' was found in list of named variables" << endl;
		}
	}

	return fn;
}

template <class T>
FunctionNode<T> ParseExpression(string expr, vector<NamedFunction<T>*> named_funcs, TTree* tree)
{
	//The string expr should only consist of function names (the function could return a variable or constant), parenthesis, and commas

	FunctionNode<T> fn;

	int i;
	bool found;
	string name = "";
	vector<string> child_exprs = {};

	//parse the string for the first time to determine the outermost function name and the depth of the child expressions
	for(i = 0; i < (int)expr.length(); i++)
	{
		if(expr.substr(i, 1) == " ")continue; //skip spaces
		if(expr.substr(i, 1) == "(")break;

		name += expr.substr(i, 1);
	}

	child_exprs = GetExpressionArgs(expr);
	fn.num_children = (int)child_exprs.size();

	if(fn.num_children > 0) //There are arguments, so this is truly a function
	{
		//Find the function with the corresponding name
		found = false;
		for(i = 0; i < (int)named_funcs.size(); i++)
		{
			if(named_funcs[i]->name == name)
			{
				found = true;
				fn.self = named_funcs[i];
				break;
			}
		}
		if(!found)
		{
			cout << "Argument '" << name << "' has arguments" << endl;
			cout << "However, no function '" << name << "' was found in list of named functions" << endl;
		}

		//Now assign the children function nodes
		fn.child_vals = new T[fn.num_children];
		fn.children = new FunctionNode<T>[fn.num_children];

		for(i = 0; i < fn.num_children; i++)
		{
			fn.children[i] = ParseExpression(child_exprs[i], named_funcs, tree);
		}
	}
	else //if it has no children, it is a variable, and 'name' stores its name
	{
		//Find the identity function
		found = false;
		for(i = 0; i < (int)named_funcs.size(); i++)
		{
			if(named_funcs[i]->name == "I")
			{
				found = true;
				fn.self = named_funcs[i];
				break;
			}
		}
		if(!found)
		{
			cout << "Identity function 'I' not contained in list of named functions" << endl;
		}

		fn.children = 0x0;

		//find the variable of the same name name, and set its address to child vals
		//change to correspond to branches for ROOT functionality
		found = false;
		if(tree->GetBranch(name.c_str()) != 0x0)
		{
			fn.child_vals = (T*)(tree->GetBranch(name.c_str())->GetAddress());
		}
		else
		{
			cout << "Argument '" << name << "' has no arguments" << endl;
			cout << "However, no branch '" << name << "' was found in passed tree" << endl;
		}
	}

	return fn;
}

template <class T>
class CommonFuncs
{
public:
	vector<NamedFunction<T>*> common_funcs;

	NamedFunction<T> named_i;	static T I(T* args){return args[0];}

	NamedFunction<T> named_add;	static T ADD(T* args){return args[0] + args[1];}
	NamedFunction<T> named_sub;	static T SUB(T* args){return args[0] - args[1];}

	NamedFunction<T> named_mul;	static T MUL(T* args){return args[0] * args[1];}
	NamedFunction<T> named_div;	static T DIV(T* args){return args[0] / args[1];}

	NamedFunction<T> named_log;	static T EXP(T* args){return exp(args[0]);}
	NamedFunction<T> named_exp;	static T LOG(T* args){return log(args[0]);}

	NamedFunction<T> named_abs;	static T ABS(T* args){return abs(args[0]);}

	CommonFuncs()
	{
		common_funcs = {};

		named_i.name = "I";	named_i.func = &I;	common_funcs.push_back(&named_i);

		named_add.name = "ADD";	named_add.func = &ADD;	common_funcs.push_back(&named_add);
		named_sub.name = "SUB";	named_sub.func = &SUB;	common_funcs.push_back(&named_sub);

		named_mul.name = "MUL";	named_mul.func = &MUL;	common_funcs.push_back(&named_mul);
		named_div.name = "DIV";	named_div.func = &DIV;	common_funcs.push_back(&named_div);

		named_log.name = "LOG";	named_log.func = &LOG;	common_funcs.push_back(&named_log);
		named_exp.name = "EXP";	named_exp.func = &EXP;	common_funcs.push_back(&named_exp);

		named_abs.name = "ABS"; named_abs.func = &ABS;	common_funcs.push_back(&named_abs);
	}
};

#endif
