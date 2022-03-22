//Standard includes
#include <iostream>
#include <fstream>
#include <vector>

//Root inlcudes
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
#include "LumberJack.h"
#include "str_func.h"

using namespace std;

LumberJack :: LumberJack()
{
	//Change so that these are read from a config file

	tree_list_file = "training_files.list"; //List file that names the .root files used for training
	variable_list_file = "training_variables.list"; //List file that contains the names of training variables and the corresponding expressions


	signal_filename = "sgnl.root"; //File that stores the tree containing signal events
	background_filename = "bkgd.root";	//File that stores the tree containing background events
	training_output_filename = "training_out.root";

	weights_filename = "weights";
	data_loader_filename = "dataset";

	TString factory_options = "V:!Silent:Color:Transformations=I:DrawProgressBar:AnalysisType=Classification";
	TString dataset_options = "SplitMode=Random";
	TString method_options = "";

	TCut sgnl_cut = "";
	TCut bkgd_cut = "";

	Float_t mu = 1.82;
	Float_t sigma = 0.05;
}


void LumberJack :: ReadVariables(vector<string> *var_names, vector<string> *var_exprs, vector<string> *branch_names)
{
	int i, j, k;
	bool b;

	//Read the training variables list file
	//obrain the variable names and expressions
	var_names->clear();
	var_exprs->clear();
	branch_names->clear();

	string line;
	string var_name;
	string var_expr;
	ifstream var_name_list(variable_list_file);
	while(true)
	{
		var_name_list >> line;
		if(var_name_list.eof())break;

		i = 0;

		var_name = "";
		while(i < (int)line.length())
		{
			if(line.substr(i, 1) == "=")break;

			var_name += line.substr(i, 1);
			i++;
		}
		
		i++;

		var_expr = "";
		while(i < (int)line.length())
		{
			var_expr += line.substr(i, 1);
			i++;
		}

		var_names->push_back(var_name);
		var_exprs->push_back(var_expr);
	}

	//Now that we have the names and expressions of the training variables, determine which branches we'll need from the expressions
	vector<string> vars = {};
	for(i = 0; i < (int)var_exprs->size(); i++)
	{
		vars = GetExpressionVars((*var_exprs)[i]);
		for(j = 0; j < (int)vars.size(); j++)
		{
			b = false;
			for(k = 0; k < (int)branch_names->size(); k++)
			{
				if((*branch_names)[k] == vars[j])b = true;
			}
			if(!b)branch_names->push_back(vars[j]);
		}
	}
}

void LumberJack :: MakeTrainingTrees()
{
	//Reads in the trees from tree_list_file
	//Copies the values of the specified branches to a local tree
	//Writes the local tree to the combined_tree_file

	//make some dummy variables
	int i, j, k;
	bool b;
	
	//Read the training variables list file and determine the variable names, expressions, and needed branches
	vector<string> var_names, var_exprs, branch_names;
	ReadVariables(&var_names, &var_exprs, &branch_names);

	Float_t var_vals[(int)var_names.size()];
	Float_t branch_vals[(int)branch_names.size()];

	//Prepare trees for training
	TTree* sgnl_tree = new TTree("Signal", "Signal");
	TTree* bkgd_tree = new TTree("Background", "Background");
	for(i = 0; i < (int)var_names.size(); i++)
	{
		sgnl_tree->Branch(var_names[i].c_str(), &(var_vals[i]));
		bkgd_tree->Branch(var_names[i].c_str(), &(var_vals[i]));
	}

	//Local signal and background trees have been prepared, now prepare to read in data
	//Need variables to store mass and truth information
	//mass is kind of a special case; more on this when we read in the trees
	Float_t m = 0;
	Float_t * m_ptr = &m;

	vector<Int_t> * hist_1 = 0;
	vector<Int_t> * hist_2 = 0;

	vector<Float_t> * hist_1_pT = 0;
	vector<Float_t> * hist_2_pT = 0;

	//Also dummy indices
	Long64_t n;

	CommonFuncs<Float_t> cf;
	FunctionNode<Float_t> var_funcs[(int)var_names.size()];

	string tree_filename;
	ifstream tree_filename_list(tree_list_file);
	while(true)
	{
		tree_filename_list >> tree_filename;
		if(tree_filename_list.eof())
		{
			break;
		}

		TFile* tree_file(TFile::Open(tree_filename.c_str()));
		TTree* r = (TTree*)tree_file->Get("DecayTree");
		if(r == 0x0)
		{
			cout << "Did not get 'DecayTree' from file: ";
			cout << tree_filename << endl;
			cout << "Check filename spelling and that it is in the expected directory" << endl;
			cout << endl;

			continue;
		}
		r->SetBranchStatus("*", 0);

		//Address the branches we want to use for analysis
		for(i = 0; i < (int)branch_names.size(); i++)
		{
			//Find and address the corresponding branch of the read tree
			if(r->GetBranch(branch_names[i].c_str()) == 0x0)
			{
				cout << "Could not find branch '";
				cout << branch_names[i];
				cout << "' from 'DecayTree' in file: ";
				cout << tree_filename << endl;
				cout << "Check branchname spelling and that it is in the tree" << endl;
				cout << endl;

				continue;
			}

			r->SetBranchStatus(branch_names[i].c_str(), 1);
			r->SetBranchAddress(branch_names[i].c_str(), &(branch_vals[i]));
		}

		for(i = 0; i < (int)var_names.size(); i++)
		{
			var_funcs[i] = ParseExpression(var_exprs[i], cf.common_funcs, r);
		}

		//Address the branches for mass and truth information
		r->SetBranchStatus("D0_mass", 1);				r->SetBranchAddress("D0_mass", m_ptr);
		r->SetBranchStatus("track_1_true_track_history_PDG_ID", 1);	r->SetBranchAddress("track_1_true_track_history_PDG_ID", &hist_1);
		r->SetBranchStatus("track_2_true_track_history_PDG_ID", 1);	r->SetBranchAddress("track_2_true_track_history_PDG_ID", &hist_2);
		r->SetBranchStatus("track_1_true_track_history_pT", 1);		r->SetBranchAddress("track_1_true_track_history_pT", &hist_1_pT);
		r->SetBranchStatus("track_2_true_track_history_pT", 1);		r->SetBranchAddress("track_2_true_track_history_pT", &hist_2_pT);

		//Now that the branches of r have been addressed, fill the signal and background trees accordingly
		for(n = 0; n < r->GetEntriesFast(); n++)
		{
			r->GetEntry(n);

			for(i = 0; i < (int)var_names.size(); i++)
			{
				var_vals[i] = var_funcs[i].Evaluate();
			}

			b = false;
			if(1.7 < *m_ptr and *m_ptr < 2.2)//(mu - 3.0 * sigma < *m_ptr and *m_ptr < mu + 3.0 * sigma) //Sideband check//change for 3-sigma values
			{
				//Between sidebands, do truth check
				i = -1;
				for(k = 0; k < (int)hist_1->size(); k++)
				{
					if(abs(hist_1->at(k)) == 421) //If a track_1 parent is a D0(bar)
					{
						i = k;
						break;
					}
				}
				
				j = -1;
				for(k = 0; k < (int)hist_2->size(); k++)
				{
					if(abs(hist_2->at(k)) == 421) //If a track_2 parent is a D0(bar)
					{
						j = k;
						break;
					}
				}

				if(i > -1 and j > -1) //Both tracks have a D0(bar) parent
				{
					if(hist_1_pT->at(i) == hist_2_pT->at(j)) //They are the same D0(bar) (exact same transverse momentum)
					{
						b = true;
					}
				}

			}

			if(b)
			{
				sgnl_tree->Fill();
			}
			else
			{
				bkgd_tree->Fill();
			}
		}

		r->ResetBranchAddresses();
	}

	//Finished reading in events from all the trees, write signal and background to files
	TFile* sgnl_file(TFile::Open(signal_filename, "RECREATE"));
	sgnl_tree->Write();
	sgnl_file->Write();
	sgnl_file->Close();

	TFile* bkgd_file(TFile::Open(background_filename, "RECREATE"));
	bkgd_tree->Write();
	bkgd_file->Write();
	bkgd_file->Close();
}


void LumberJack :: TrainBDT()
{
	TMVA::Tools::Instance();

	TFile* out_file(TFile::Open(training_output_filename, "RECREATE"));
	TMVA::Factory* factory = new TMVA::Factory(weights_filename, out_file, factory_options);
	TMVA::DataLoader* data_loader = new TMVA::DataLoader(data_loader_filename);

	//Read in the variables we'll need
	//The values in the training tree are already the needed expressions
	//Thus we only need the variable names tod find the branches again
	int i = 0;
	string line;
	string var_name;
	ifstream var_name_list(variable_list_file);
	while(true)
	{
		var_name_list >> line;
		if(var_name_list.eof())break;

		i = 0;
		var_name = "";
		while(i < (int)line.length())
		{
			if(line.substr(i, 1) == "=")break;

			var_name += line.substr(i, 1);
			i++;
		}

		data_loader->AddVariable(var_name.c_str(), 'F');
	}


	TFile* sgnl_file(TFile::Open(signal_filename));
	TTree* sgnl_tree = (TTree*)sgnl_file->Get("Signal");
	data_loader->AddSignalTree(sgnl_tree, 1.0);
	//can't close the file until training is done; otherwise the program segfaults

	TFile* bkgd_file(TFile::Open(background_filename));
	TTree* bkgd_tree = (TTree*)bkgd_file->Get("Background");
	data_loader->AddBackgroundTree(bkgd_tree, 1.0);
	//can't close the file until training is done; otherwise the program segfaults

	data_loader->PrepareTrainingAndTestTree(sgnl_cut, bkgd_cut, dataset_options);
	factory->BookMethod(data_loader, TMVA::Types::kBDT, "BDT", method_options);

	factory->TrainAllMethods();
	factory->TestAllMethods();
	factory->EvaluateAllMethods();

	out_file->Close();
}

void LumberJack :: Show()
{
	if(!gROOT->IsBatch())
	{
		TMVA::TMVAGui(training_output_filename);
	}
	else
	{
		cout << "gROOT isn't batch :^(" << endl;
	}
}

void LumberJack :: Check(const char* hist_branch, Float_t cut_val)
{
	//Apply the produced BDT to classify the training data and plot the mass distribution

	//Dummy variables
	int i;
	Long64_t n;

	//Prepare the histogram
	TH1F* sgnl_hist = new TH1F("SignalDist", hist_branch, 20, 1.7, 2.2);
	TH1F* bkgd_hist = new TH1F("BackgroundDist", hist_branch, 20, 1.7, 2.2);

	//Read the training variables list file
	//obrain the variable names and expressions
	vector<string> var_names, var_exprs, branch_names;
	ReadVariables(&var_names, &var_exprs, &branch_names);

	Float_t var_vals[(int)var_names.size()];
	Float_t branch_vals[(int)branch_names.size()];
	Float_t hist_val;
	Float_t *hist_ptr = &hist_val;

	//Prepare the data reader
	TMVA::Tools::Instance();
	TMVA::Reader* data_reader = new TMVA::Reader("V:Color:!Silent");
	for(i = 0; i < (int)var_names.size(); i++)
	{
		data_reader->AddVariable(var_names[i].c_str(), &(var_vals[i]));
	}
	data_reader->BookMVA("BDT", "dataset/weights/weights_BDT.weights.xml"); //change to use variable names later

	CommonFuncs<Float_t> cf;
	FunctionNode<Float_t> var_funcs[(int)var_names.size()];

	string tree_filename;
	ifstream tree_filename_list(tree_list_file);
	while(true)
	{
		tree_filename_list >> tree_filename;
		if(tree_filename_list.eof())
		{
			break;
		}

		TFile* tree_file(TFile::Open(tree_filename.c_str()));
		TTree* r = (TTree*)tree_file->Get("DecayTree");
		if(r == 0x0)
		{
			cout << "Did not get 'DecayTree' from file: ";
			cout << tree_filename << endl;
			cout << "Check filename spelling and that it is in the expected directory" << endl;
			cout << endl;

			continue;
		}
		r->SetBranchStatus("*", 0);

		//Address the branches we want to use for analysis
		for(i = 0; i < (int)branch_names.size(); i++)
		{
			//Find and address the corresponding branch of the read tree
			if(r->GetBranch(branch_names[i].c_str()) == 0x0)
			{
				cout << "Could not find branch '";
				cout << branch_names[i];
				cout << "' from 'DecayTree' in file: ";
				cout << tree_filename << endl;
				cout << "Check branchname spelling and that it is in the tree" << endl;
				cout << endl;

				continue;
			}

			r->SetBranchStatus(branch_names[i].c_str(), 1);
			r->SetBranchAddress(branch_names[i].c_str(), &(branch_vals[i]));
		}

		//Address the branch we want to histogram
		if(r->GetBranchStatus(hist_branch))hist_ptr = (Float_t*)((r->GetBranch(hist_branch))->GetAddress());//There is a possibility it is already used to compute a training variable
		
		r->SetBranchStatus(hist_branch, 1);
		r->SetBranchAddress(hist_branch, hist_ptr);
		
		for(i = 0; i < (int)var_names.size(); i++)
		{
			var_funcs[i] = ParseExpression(var_exprs[i], cf.common_funcs, r);
		}

		for(n = 0; n < r->GetEntriesFast(); n++)
		{
			r->GetEntry(n);
		
			for(i = 0; i < (int)var_names.size(); i++)
			{
				var_vals[i] = var_funcs[i].Evaluate();
			}
			
			if(data_reader->EvaluateMVA("BDT") > cut_val)
			{
				sgnl_hist->Fill(*hist_ptr);
			}
			else
			{
				bkgd_hist->Fill(*hist_ptr);
			}
		}
	}

	TFile* sgnl_file(TFile::Open("sgnl_hist.root", "RECREATE"));
	sgnl_hist->Write();
	sgnl_file->Write();
	sgnl_file->Close();

	TFile* bkgd_file(TFile::Open("bkgd_hist.root", "RECREATE"));
	bkgd_hist->Write();
	bkgd_file->Write();
	bkgd_file->Close();

	//Draw the histograms
	//sgnl_hist->Draw();
	//bkgd_hist->Draw("same");
}
