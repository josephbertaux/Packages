#ifndef LUMBERJACK_H
#define LUMBERJACK_H

//Standard includes
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

//Root includes
#include "TMVA/DataLoader.h"
#include "TMVA/Reader.h"
#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"

#include "TROOT.h"
#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"

//Local includes

class LumberJack
{
private:
	const char* tree_list_file;
	const char* branch_list_file; //List file that contains the names of branches used for training and analysis
	const char* variable_list_file; //List file that contains the names of training variables and the corresponding expressions

	
	const char* signal_filename; //File that stores the tree containing signal events
	const char* background_filename;	//File that stores the tree containing background events
	const char* training_output_filename;
	
	const char* weights_filename;
	const char* data_loader_filename;

	TString factory_options;
	TString dataset_options;
	TString method_options;

	TCut sgnl_cut;
	TCut bkgd_cut;

	Float_t mu;
	Float_t sigma;
public:
	LumberJack(void);

	void ReadVariables(std::vector<std::string> *var_names, std::vector<std::string> *var_exprs, std::vector<std::string> *branch_names);

	void MakeTrainingTrees(void);

	void TrainBDT(void);

	void Show(void);

	void Check(const char* hist_branch, Float_t cut_val);
};

#endif//LUMBERJACK_H
