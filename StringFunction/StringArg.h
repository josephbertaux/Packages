#include <string>

template <typename T>
class StringArg
{
public:
	virtual std::string Print();

	virtual T Evaluate();
	virtual StringArg* Derivative(std::string);
	virtual void Simplify();
};
