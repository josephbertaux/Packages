#include "StringArg.h"
#include "BaseFunction.h"

#include <string>
#include <vector>

template <typename T>
class StringFunction : StringArg<T>
{
private:
	vector<StringArg<T>*> args;
	BaseFunction* base;

public:
	StringFunction(BaseFunction* b, vector<StringArg<T>*> a)
	{
		base = b;
		args = a;
	}

	template <typename TArgs...>
	StringFunction(BaseFunction* b, StringArg<T>* a, TArgs... targs)
	{
		vector<StringArg<T>*> args = {a};

		StringFunction(b, args, targs);
	}

	template <typename TArgs...>
	StringFunction(BaseFunction* b, vector<StringArg<T>*> args, StringArg<T>* a, TArgs... targs)
	{
		args.push_back(a);

		StringFunction(b, args, targs);
	}


	T Evaluate() override
	{
		return base->Evaluate(args);
	}

	StringArg* Derivative(std::string var_name) override
	{
		for(uint u = 0; u < args.size(); u++)
		{
			
		}
	}
};
