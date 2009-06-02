/*
 *  matrixoutputcommand.cpp
 *  Mothur
 *
 *  Created by Sarah Westcott on 5/20/09.
 *  Copyright 2009 Schloss Lab UMASS Amherst. All rights reserved.
 *
 */

#include "matrixoutputcommand.h"
#include "sharedjabund.h"
#include "sharedsorabund.h"
#include "sharedjclass.h"
#include "sharedsorclass.h"
#include "sharedjest.h"
#include "sharedsorest.h"
#include "sharedthetayc.h"
#include "sharedthetan.h"
#include "sharedmorisitahorn.h"
#include "sharedbraycurtis.h"


//**********************************************************************************************************************

MatrixOutputCommand::MatrixOutputCommand(){
	try {
		globaldata = GlobalData::getInstance();
		validCalculator = new ValidCalculators();
			
		int i;
		for (i=0; i<globaldata->Estimators.size(); i++) {
			if (validCalculator->isValidCalculator("matrix", globaldata->Estimators[i]) == true) { 
				if (globaldata->Estimators[i] == "jabund") { 	
					matrixCalculators.push_back(new JAbund());
				}else if (globaldata->Estimators[i] == "sorabund") { 
					matrixCalculators.push_back(new SorAbund());
				}else if (globaldata->Estimators[i] == "jclass") { 
					matrixCalculators.push_back(new Jclass());
				}else if (globaldata->Estimators[i] == "sorclass") { 
					matrixCalculators.push_back(new SorClass());
				}else if (globaldata->Estimators[i] == "jest") { 
					matrixCalculators.push_back(new Jest());
				}else if (globaldata->Estimators[i] == "sorest") { 
					matrixCalculators.push_back(new SorEst());
				}else if (globaldata->Estimators[i] == "thetayc") { 
					matrixCalculators.push_back(new ThetaYC());
				}else if (globaldata->Estimators[i] == "thetan") { 
					matrixCalculators.push_back(new ThetaN());
				}else if (globaldata->Estimators[i] == "morisitahorn") { 
					matrixCalculators.push_back(new MorHorn());
				}else if (globaldata->Estimators[i] == "braycurtis") { 
					matrixCalculators.push_back(new BrayCurtis());
				}
			}
		}
		
		//reset calc for next command
		globaldata->setCalc("");

	}
	catch(exception& e) {
		cout << "Standard Error: " << e.what() << " has occurred in the MatrixOutputCommand class Function MatrixOutputCommand. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	catch(...) {
		cout << "An unknown error has occurred in the MatrixOutputCommand class function MatrixOutputCommand. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}	
}
//**********************************************************************************************************************

MatrixOutputCommand::~MatrixOutputCommand(){
	delete input;
	delete read;
}

//**********************************************************************************************************************

int MatrixOutputCommand::execute(){
	try {
		int count = 1;	
				
		//if the users entered no valid calculators don't execute command
		if (matrixCalculators.size() == 0) { cout << "No valid calculators." << endl; return 0; }

		//you have groups
		read = new ReadOTUFile(globaldata->inputFileName);	
		read->read(&*globaldata); 
			
		input = globaldata->ginput;
		lookup = input->getSharedRAbundVectors();
		vector<SharedRAbundVector*> lastLookup = lookup;
		
		//if the users enters label "0.06" and there is no "0.06" in their file use the next lowest label.
		set<string> processedLabels;
		set<string> userLabels = globaldata->labels;
				
		if (lookup.size() < 2) { cout << "You have not provided enough valid groups.  I cannot run the command." << endl; return 0;}
		
		numGroups = globaldata->Groups.size();
				
		//as long as you are not at the end of the file or done wih the lines you want
		while((lookup[0] != NULL) && ((globaldata->allLines == 1) || (userLabels.size() != 0))) {
		
			if(globaldata->allLines == 1 || globaldata->lines.count(count) == 1 || globaldata->labels.count(lookup[0]->getLabel()) == 1){			
				cout << lookup[0]->getLabel() << '\t' << count << endl;
				process(lookup);
				
				processedLabels.insert(lookup[0]->getLabel());
				userLabels.erase(lookup[0]->getLabel());
			}
			
			if ((anyLabelsToProcess(lookup[0]->getLabel(), userLabels, "") == true) && (processedLabels.count(lastLookup[0]->getLabel()) != 1)) {
				cout << lastLookup[0]->getLabel() << '\t' << count << endl;
				process(lastLookup);
				
				processedLabels.insert(lastLookup[0]->getLabel());
				userLabels.erase(lastLookup[0]->getLabel());
			}

			//prevent memory leak
			if (count != 1) { for (int i = 0; i < lastLookup.size(); i++) {  delete lastLookup[i];  } }
			lastLookup = lookup;			
			
			//get next line to process
			lookup = input->getSharedRAbundVectors();
			count++;
		}
		
		//output error messages about any remaining user labels
		set<string>::iterator it;
		bool needToRun = false;
		for (it = userLabels.begin(); it != userLabels.end(); it++) {  
			cout << "Your file does not include the label "<< *it; 
			if (processedLabels.count(lastLookup[0]->getLabel()) != 1) {
				cout << ". I will use " << lastLookup[0]->getLabel() << "." << endl;
				needToRun = true;
			}else {
				cout << ". Please refer to " << lastLookup[0]->getLabel() << "." << endl;
			}
		}
		
		//run last line if you need to
		if (needToRun == true)  {
			cout << lastLookup[0]->getLabel() << '\t' << count << endl;
			process(lastLookup);		
		}
		
		for (int i = 0; i < lastLookup.size(); i++) {  delete lastLookup[i];  }

		
		//reset groups parameter
		globaldata->Groups.clear();  globaldata->setGroups("");

		return 0;
	}
	catch(exception& e) {
		cout << "Standard Error: " << e.what() << " has occurred in the MatrixOutputCommand class Function execute. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	catch(...) {
		cout << "An unknown error has occurred in the MatrixOutputCommand class function execute. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}		
}
/***********************************************************/
void MatrixOutputCommand::printSims(ostream& out) {
	try {
		
		//output column headers
		out << simMatrix.size() << endl;
		
		for (int m = 0; m < simMatrix.size(); m++)	{
			out << lookup[m]->getGroup() << '\t';
			for (int n = 0; n < m; n++)	{
				out << simMatrix[m][n] << '\t'; 
			}
			out << endl;
		}

	}
	catch(exception& e) {
		cout << "Standard Error: " << e.what() << " has occurred in the MatrixOutputCommand class Function printSims. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	catch(...) {
		cout << "An unknown error has occurred in the MatrixOutputCommand class function printSims. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}		
}
/***********************************************************/
void MatrixOutputCommand::process(vector<SharedRAbundVector*> thisLookup){
	try {
	
				EstOutput data;
				vector<SharedRAbundVector*> subset;

				//for each calculator												
				for(int i = 0 ; i < matrixCalculators.size(); i++) {
					
					//initialize simMatrix
					simMatrix.clear();
					simMatrix.resize(numGroups);
					for (int m = 0; m < simMatrix.size(); m++)	{
						for (int j = 0; j < simMatrix.size(); j++)	{
							simMatrix[m].push_back(0.0);
						}
					}
				
					for (int k = 0; k < thisLookup.size(); k++) { 
						for (int l = k; l < thisLookup.size(); l++) {
							if (k != l) { //we dont need to similiarity of a groups to itself
								//get estimated similarity between 2 groups
								
								subset.clear(); //clear out old pair of sharedrabunds
								//add new pair of sharedrabunds
								subset.push_back(thisLookup[k]); subset.push_back(thisLookup[l]); 
								
								data = matrixCalculators[i]->getValues(subset); //saves the calculator outputs
								//save values in similarity matrix
								simMatrix[k][l] = 1.0 - data[0];  //convert similiarity to distance
								simMatrix[l][k] = 1.0 - data[0];  //convert similiarity to distance
							}
						}
					}
					
					exportFileName = getRootName(globaldata->inputFileName) + matrixCalculators[i]->getName() + "." + thisLookup[0]->getLabel() + ".dist";
					openOutputFile(exportFileName, out);
					printSims(out);
					out.close();
					
				}

	
		
	}
	catch(exception& e) {
		cout << "Standard Error: " << e.what() << " has occurred in the MatrixOutputCommand class Function process. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	catch(...) {
		cout << "An unknown error has occurred in the MatrixOutputCommand class function process. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}		
}
/***********************************************************/



