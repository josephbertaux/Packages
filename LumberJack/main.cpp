//Standard includes
#include <iostream>
#include <string>
#include <vector>

#include "LumberJack.h"


using namespace std;

int main()
{
	LumberJack lj;

	/*
	vector<string> var_names, var_exprs, branch_names;
	lj.ReadVariables(&var_names, &var_exprs, &branch_names);

	cout << "var_names:" << endl;
	for(int i = 0; i < (int)var_names.size(); i++)
	{
		cout << var_names[i] << endl;
	}

	cout << endl;
	cout << "var_exprs:" << endl;
	for(int i = 0; i < (int)var_exprs.size(); i++)
	{
		cout << var_exprs[i] << endl;
	}

	cout << endl;
	cout << "branch_names:" << endl;
	for(int i = 0; i < (int)branch_names.size(); i++)
	{
		cout << branch_names[i] << endl;
	}
	*/

	lj.MakeTrainingTrees();
	lj.TrainBDT();
	lj.Show();
	lj.Check("D0_mass", -1.0);
	return 0;
}
