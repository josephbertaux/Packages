#include "StringArg.h"

#include <string>
#include <vector>

template <typename T>
class BaseFunction
{
public:

	virtual std::string Write();

	virtual T Evaluate(std::vector<StringArg<T>*>);
	virtual StringArg* Derivative(int);
	virtual StringArg* Simplify();
};
