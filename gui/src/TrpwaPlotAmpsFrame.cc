/*
 * TrpwaWaveSelectFrame.cc
 *
 *  Created on: Aug 26, 2010
 *      Author: Promme
 */

#include "TrpwaPlotAmpsFrame.h"
#include <sstream>
#include <iostream>
#include "TGButtonGroup.h"
#include "TGButton.h"
#include "TGTab.h"
#include "TGLabel.h"
#include "TrpwaCommonTools.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TChain.h"
//#include "TFitBin.h"
#include "plotIntensity.h"
#include "plotPhase.h"
#include "plotCoherence.h"
#include "plotAllIntensities.h"
#include "plotSpinTotals.h"
#include "fitResult.h"
#include "TGTextEntry.h"
#include "TGMsgBox.h"
#include "TAxis.h"
#include "TGFileDialog.h"

using namespace std;
using namespace TrpwaCommonTools;
using namespace rpwa;

#ifndef TRPWAPLOTAMPSFRAME_CC_
#define TRPWAPLOTAMPSFRAME_CC_

TrpwaPlotAmpsFrame::TrpwaPlotAmpsFrame(
		vector<string> fit_result_paths, // paths containing fit results (all file with the ending .root will be used
		vector<string> fit_result_titles,           // titles of the fits (must be unique)
		vector<string> fit_result_descriptions      // descriptions of the fits
	): TGTransientFrame(gClient->GetRoot(),0,600,600, kVerticalFrame){
	available_fit_results.clear();
	selected_fit_results.clear();
	available_waves.clear();
	available_colors.clear();
	Create_available_colors();
	current_wave = "";
	current_anchor_wave = "";
	current_fit_result = NULL;
	masscutlow  = 0;
	masscuthigh = 0;
	for (int ipad = 0; ipad < 5; ipad++){
		plotted_graphs[ipad] = NULL;
		plotted_most_likely_graphs[ipad] = NULL;
	}
	draw_most_likely = true;
	if (fit_result_descriptions.size() == fit_result_paths.size() && fit_result_paths.size() == fit_result_titles.size()){
		for (unsigned int i = 0; i < fit_result_paths.size(); i++){
			//if (i < 5) continue;
			//cout << fit_result_paths[i] << endl;
			if (!Add_Fit_Result(fit_result_paths[i], fit_result_titles[i], fit_result_descriptions[i])){
				cout << " Warning: problems adding fit results from " << fit_result_paths[i] << endl;
			}
		}
		Build();
		fClient->WaitFor(this); // wait till the user closes this window
	} else {
		cout << " Error in TrpwaPlotAmpsFrame::TrpwaPlotAmpsFrame(): wrong input paramters to constructor! " << endl;
		this->CloseWindow();
	}
}

TrpwaPlotAmpsFrame::~TrpwaPlotAmpsFrame(){
	for (Tfilemapit it = available_fit_results.begin(); it != available_fit_results.end(); it++){
		//it->second->Close();
		delete it->second;
	}
	Cleanup();
}

void TrpwaPlotAmpsFrame::Build(){
	// set the root layout for the objects to draw
    gROOT->SetStyle("Plain");
    // this style is set to fit my phd thesis style:
    if (1){
		gStyle->SetTitleFont(10*13+2,"xyz");
		gStyle->SetTitleSize(0.06, "xyz");
		gStyle->SetTitleOffset(1.3,"y");
		gStyle->SetLabelFont(10*13+2,"xyz");
		gStyle->SetLabelSize(0.06,"xyz");
		gStyle->SetLabelOffset(0.009,"xyz");
		gStyle->SetPadBottomMargin(0.16);
		gStyle->SetPadTopMargin(0.16);
		gStyle->SetPadLeftMargin(0.16);
		gStyle->SetPadRightMargin(0.16);
		//gStyle->SetOptTitle(0);
		//gStyle->SetOptStat(0);
    }
    //gStyle->SetOptTitle(kFALSE);

    //gStyle->SetOptStat(0);
    //gStyle->SetOptStat(111); // to see histogram statistics in plots
    //gStyle->SetOptFit(111);  // to see fit output in plots
    /*
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetPadBorderMode(0);
    gStyle->SetPadColor(0);
    gStyle->SetCanvasColor(0);
    gStyle->SetFillColor(0);
    gStyle->SetTitleFillColor(0);*/

    gROOT->ForceStyle();

    // group for root file selection
	TGGroupFrame* frame_rootfile_selections = new TGGroupFrame(this, " select fits ", kHorizontalFrame);

	TGVerticalFrame* subframe_available_fits = new TGVerticalFrame(frame_rootfile_selections);
	subframe_available_fits->AddFrame(
				new TGLabel(subframe_available_fits, "available fit results"));
	// drop down box with available fit results
	box_available_fits = new TGComboBox(subframe_available_fits);
	box_available_fits->Connect("Selected(Int_t)", "TrpwaPlotAmpsFrame", this, "Add_Fit(Int_t)");
	box_available_fits->AddEntry("NONE",-1);
	for (Tfilemapit it = available_fit_results.begin(); it != available_fit_results.end(); it++){
		box_available_fits->AddEntry(it->first.c_str(), (long int) it->second);
	}

	box_available_fits->SetHeight(20);
	subframe_available_fits->AddFrame(box_available_fits, new TGLayoutHints(kLHintsTop | kLHintsLeft |
			kLHintsExpandX,1,1,1,1));

	TGVerticalFrame* subframe_selected_fits = new TGVerticalFrame(frame_rootfile_selections);
	subframe_selected_fits->AddFrame(
				new TGLabel(subframe_selected_fits, "list of selected fit results"));
	box_selected_fits = new TGComboBox(subframe_selected_fits);
	box_selected_fits->Connect("Selected(Int_t)", "TrpwaPlotAmpsFrame", this, "Remove_Fit(Int_t)");
	box_selected_fits->AddEntry("NONE",-1);
	box_selected_fits->SetHeight(20);
	subframe_selected_fits->AddFrame(box_selected_fits, new TGLayoutHints(kLHintsTop | kLHintsLeft |
			kLHintsExpandX,1,1,1,1));

	frame_rootfile_selections->AddFrame(subframe_available_fits, new TGLayoutHints(kLHintsTop | kLHintsLeft |
			kLHintsExpandX,1,1,1,1));
	frame_rootfile_selections->AddFrame(subframe_selected_fits, new TGLayoutHints(kLHintsTop | kLHintsLeft |
			kLHintsExpandX,1,1,1,1));

	// button to draw all intensities (external script will be called)
	TGTextButton* plot_all_button = new TGTextButton(this, new TGHotString(" draw all waves "));
	plot_all_button->Connect("Clicked()","TrpwaPlotAmpsFrame",this,"Plot_All_selected()");
	// Drawing of the spin totals only
	TGTextButton* plot_spin_totals_button = new TGTextButton(this, new TGHotString(" draw all wave's spin totals "));
	plot_spin_totals_button->Connect("Clicked()","TrpwaPlotAmpsFrame",this,"Plot_All_selected_Spin_totals()");

	// group with buttons to select specific (anchor) waves
	TGGroupFrame* frame_partial_wave_selections = new TGGroupFrame(this, " select partial waves ", kHorizontalFrame);
	TGVerticalFrame* subframe_partial_wave_selections = new TGVerticalFrame(frame_partial_wave_selections);
	subframe_partial_wave_selections->AddFrame(
					new TGLabel(subframe_partial_wave_selections, "available partial waves"));
	// drop down box with available waves given by the selected root files
	box_available_waves = new TGComboBox(subframe_partial_wave_selections);
	box_available_waves->Connect("Selected(Int_t)", "TrpwaPlotAmpsFrame", this, "Select_Wave(Int_t)");
	box_available_waves->AddEntry("NONE",-1);
	box_available_waves->SetHeight(20);
	subframe_partial_wave_selections->AddFrame(box_available_waves, new TGLayoutHints(kLHintsTop | kLHintsLeft |
			kLHintsExpandX,1,1,1,1));

	TGVerticalFrame* subframe_anchor_wave_selections = new TGVerticalFrame(frame_partial_wave_selections);
		subframe_anchor_wave_selections->AddFrame(
						new TGLabel(subframe_anchor_wave_selections, "selected anchor wave"));
	box_available_anchor_waves = new TGComboBox(subframe_anchor_wave_selections);
	box_available_anchor_waves->Connect("Selected(Int_t)", "TrpwaPlotAmpsFrame", this, "Select_Anchor_Wave(Int_t)");
	box_available_anchor_waves->AddEntry("NONE",-1);
	box_available_anchor_waves->SetHeight(20);
	subframe_anchor_wave_selections->AddFrame(box_available_anchor_waves, new TGLayoutHints(kLHintsTop | kLHintsLeft |
			kLHintsExpandX,1,1,1,1));

	frame_partial_wave_selections->AddFrame(subframe_partial_wave_selections, new TGLayoutHints(kLHintsTop | kLHintsLeft |
			kLHintsExpandX,1,1,1,1));
	frame_partial_wave_selections->AddFrame(subframe_anchor_wave_selections, new TGLayoutHints(kLHintsTop | kLHintsLeft |
			kLHintsExpandX,1,1,1,1));

	button_show_most_likely = new TGCheckButton(this, new TGHotString("plot most likely solution only"));
	// if use normalization not given then it is assumed not to be available
	if (!draw_most_likely){
		button_show_most_likely->SetState(kButtonUp);
	} else {
		button_show_most_likely->SetState(kButtonDown);
	}
	button_show_most_likely->Connect("Clicked()","TrpwaPlotAmpsFrame",this,"Set_show_most_likely()");


	// button to plot the selection
	TGTextButton* plot_button = new TGTextButton(this, new TGHotString(" draw selected waves "));
	plot_button->Connect("Clicked()","TrpwaPlotAmpsFrame",this,"Plot_selected_wave()");

	// canvas where the results are drawn to
	TRootEmbeddedCanvas* frame_selected_waves = new TRootEmbeddedCanvas(" canvas_selected_waves ",
			this, 1000, 600, kSunkenFrame|kDoubleBorder , 0xffffff);
	canvas_selected_waves = frame_selected_waves->GetCanvas();
	canvas_selected_waves->Divide(2,2);

	// slider to select a mass range for all 4 plots
	slider_mass_range = new TGDoubleHSlider(this);
	slider_mass_range->Connect("Released()","TrpwaPlotAmpsFrame",this,"Set_Mass_range()");
	slider_mass_range->SetPosition(0.,1.);

	// button to save the plot
	TGTextButton* save_plot_button = new TGTextButton(this, new TGHotString(" save current plot "));
	save_plot_button->Connect("Clicked()","TrpwaPlotAmpsFrame",this,"Save_plot()");

	this->AddFrame(frame_rootfile_selections, new TGLayoutHints(kLHintsTop | kLHintsLeft |
			kLHintsExpandX,1,1,1,1));
	this->AddFrame(plot_all_button, new TGLayoutHints(kLHintsTop | kLHintsLeft |
			kLHintsExpandX,1,1,1,1));
	this->AddFrame(plot_spin_totals_button, new TGLayoutHints(kLHintsTop | kLHintsLeft |
				kLHintsExpandX,1,1,1,1));
	this->AddFrame(frame_partial_wave_selections, new TGLayoutHints(kLHintsTop | kLHintsLeft |
			kLHintsExpandX,1,1,1,1));
	this->AddFrame(button_show_most_likely,new TGLayoutHints(kLHintsCenterX,1,1,1,1));
	this->AddFrame(plot_button, new TGLayoutHints(kLHintsTop | kLHintsLeft |
			kLHintsExpandX,1,1,1,1));
	this->AddFrame(frame_selected_waves);
	this->AddFrame(slider_mass_range, new TGLayoutHints(kLHintsTop | kLHintsLeft |
				kLHintsExpandX,1,1,1,1));
	this->AddFrame(save_plot_button, new TGLayoutHints(kLHintsTop | kLHintsLeft |
			kLHintsExpandX,1,1,1,1));

	SetWindowName("Inspect Results");
	Resize(GetDefaultSize());
	SetWMSize(fWidth, fHeight);
	CenterOnParent();
	MapSubwindows();
	MapWindow();
}

bool TrpwaPlotAmpsFrame::Add_Fit_Result(string fit_result_path, // path containing .root files with fit result trees
			string fit_result_title,			// (unique) title of the fit
			string fit_result_description){		// description of the fit
	bool result(true);
	if (!DirExists(fit_result_path)) return false;
	if (available_fit_results.find(fit_result_title) == available_fit_results.end()){
		// search for .root files in this path
		vector<string> rootfiles;
		if (!(GetDir(fit_result_path, rootfiles, ".root", false) > 0)) result = false;
		TTree* fitresulttree = new TChain(fit_result_title.c_str());
	   	for (vector<string>::iterator it = rootfiles.begin(); it != rootfiles.end(); it++){
			if (!((TChain*)fitresulttree)->AddFile((fit_result_path+"/"+(*it)).c_str(), -1, "pwa")){
				cout << " Error in TrpwaPlotAmpsFrame::Add_Fit_Result(): Could not add file " << fit_result_path+"/"+(*it) << endl;
			} else {
				//cout << " added " << fit_result_path+"/"+(*it) << endl;
			}
		}
		//cout << " found " << fitresulttree->GetEntries() << " entries " << endl;
		available_fit_results[fit_result_title] = fitresulttree;
		//cout << " tree name is " << fitresulttree->GetName() << endl;
	} else {
		cout << " Error in TrpwaPlotAmpsFrame::Add_Fit_Result(): Fit result with the title " << fit_result_title << " does already exist! " << endl;
		return false;
	}
	return result;
}

// will be called when an available fit file was selected
// an item will be added to the list of selected fit files
void TrpwaPlotAmpsFrame::Add_Fit(int pFitFile){
	if (pFitFile > 0){
		// get the name and add the entry to the list of selected fits
		TTree* selected_fit = (TTree*) pFitFile;
		string fitname = selected_fit->GetName();
		if (selected_fit_results.find(fitname)==selected_fit_results.end()){
			selected_fit_results[fitname]=selected_fit;
			vector<int> _one_fitresult;
			_one_fitresult.push_back(selected_fit_results.size()-1);
			TGIconLBEntry* entry = new TGIconLBEntry(box_selected_fits, pFitFile, fitname.c_str(), Get_Icon(_one_fitresult, false));
			box_selected_fits->AddEntry(entry,new TGLayoutHints(kLHintsTop | kLHintsLeft |
									kLHintsExpandX,1,1,1,1));
			//box_selected_fits->AddEntry(fitname.c_str(), pFitFile);
			current_fit_result = selected_fit;
			box_selected_fits->Select(pFitFile, false);

			/*
			TImage *img = TImage::Create();
			img->FillRectangle("#FFFFFF", 0, 0, 100, 100);
			img->BeginPaint();
			img->DrawCircle(50, 50, 35, "#FF0000", -1);
			img->EndPaint();
			img->Scale(16,16); // resize to create a smaller icon
			// now create a TGPicture from our image
			const TGPicture *pic = gClient->GetPicturePool()->GetPicture("pic_name", img->GetPixmap(), img->GetMask());
			TGIconLBEntry* entry = new TGIconLBEntry(box_available_waves, -10, "testentry", pic);//, UInt_t w = 0, Style_t s = 0, UInt_t options = kHorizontalFrame, Pixel_t back = GetWhitePixel())
			box_available_waves->AddEntry(entry,new TGLayoutHints(kLHintsTop | kLHintsLeft |
					kLHintsExpandX,1,1,1,1));
			delete img;*/

			// search now for waves to add to the list of waves
			vector<string> wavelist = Scan_Fit_Result(current_fit_result);
			for (vector<string>::iterator it = wavelist.begin(); it != wavelist.end(); it++){
				available_waves[*it].push_back((int)selected_fit_results.size()-1);
			}
			// renew the drop down boxes
			box_available_waves->RemoveAll();
			box_available_anchor_waves->RemoveAll();
			current_anchor_wave = "";
			current_wave = "";
			int wavecounter(0);
			for (Twavemapit it = available_waves.begin(); it != available_waves.end(); it++){
				wavecounter++; // index is not used
				TGIconLBEntry* entry = new TGIconLBEntry(box_available_waves, wavecounter, it->first.c_str(), Get_Icon(it->second));
				box_available_waves->AddEntry(entry,new TGLayoutHints(kLHintsTop | kLHintsLeft |
						kLHintsExpandX,1,1,1,1));
				entry = new TGIconLBEntry(box_available_anchor_waves, wavecounter, it->first.c_str(), Get_Icon(it->second));
				box_available_anchor_waves->AddEntry(entry,new TGLayoutHints(kLHintsTop | kLHintsLeft |
				kLHintsExpandX,1,1,1,1));
			}
		}
	//selected_fit_results[];
	//cout << pFitFile << endl;
	//cout << " calling add fit " << endl;
	}
}

// available colors for index markers
//vector<TColor_struct> available_colors;

// creates some available colors for markers in available_colors
void TrpwaPlotAmpsFrame::Create_available_colors(){
	if (available_colors.size() > 0) return; // initialize only once
	TColor_struct color;
	for (int i = 0; i < 50; i++){ // thus colors will repeat 50 times
		color.rootcolorindex = kBlack;
		color.colorhexcode   = "#000000";
		available_colors.push_back(color);
		color.rootcolorindex = kRed;
		color.colorhexcode   = "#FF0000";
		available_colors.push_back(color);
		color.rootcolorindex = kGreen;
		color.colorhexcode   = "#00FF00";
		available_colors.push_back(color);
		color.rootcolorindex = kBlue;
		color.colorhexcode   = "#0000FF";
		available_colors.push_back(color);
	}
}

const TGPicture* TrpwaPlotAmpsFrame::Get_Icon(vector<int>& fit_references, bool divide){
	if ((divide && selected_fit_results.size() == 0) || (!divide && fit_references.size() < 1)){
		cout << " Errot in TrpwaPlotAmpsFrame::Get_Icon(): no fit results are given to refer to! " << endl;
		return NULL;
	}
	TImage *img = TImage::Create();
	if (divide)
		img->FillRectangle("#FFFFFF", 0, 0, 10 * selected_fit_results.size(), 12);
	else
		img->FillRectangle("#FFFFFF", 0, 0, 10 * fit_references.size(), 12);
	img->BeginPaint();
	for (unsigned int i = 0; i < fit_references.size(); i++){
		int posx;
		if (divide)
			posx = 10 * fit_references[i] + 5;
		else
			posx = 10 * i + 5;
		//cout << " drawing " << posx << " " << available_colors[fit_references[i]].colorhexcode << endl;
		img->DrawCircle(posx, 6, 4, available_colors[fit_references[i]].colorhexcode.c_str(), -1);
		//img->Scale(16,16); // resize to create a smaller icon
	}
	img->EndPaint();
	// create an arbitrary name for the new icon in the pool of pictures
	stringstream pic_name;
	static int counter;
	pic_name << "generated_reference_icon_" << counter++;
	const TGPicture *pic = gClient->GetPicturePool()->GetPicture(pic_name.str().c_str(), img->GetPixmap(), img->GetMask());
	delete img;
	return pic;
}


// will be called when an added fit file was selected
// if the user confirms the selected item will be removed from the list
void TrpwaPlotAmpsFrame::Remove_Fit(int pFitFile){
	cout << pFitFile << endl;
	cout << " call to remove a fit is not implemented yet" << endl;
}

// a wave will be selected from the list of available waves
void TrpwaPlotAmpsFrame::Select_Wave(int pWave){
	//cout << pWave << endl;
	//cout << " calling select wave " << endl;
	TGTextLBEntry *filePointer = (TGTextLBEntry *)box_available_waves->GetSelectedEntry();
	string selected_wave = filePointer->GetTitle();
	if (available_waves.find(selected_wave)!=available_waves.end()){
		cout << " selected wave is " << selected_wave << endl;
		current_wave = selected_wave;
	} else {
		current_wave = "";
	}
}

// a wave will be selected for the anchor wave (phase determination)
void TrpwaPlotAmpsFrame::Select_Anchor_Wave(int pWave){
	//string selected_wave = box_available_anchor_waves->GetTextEntry()->GetText();
	TGTextLBEntry *filePointer = (TGTextLBEntry *)box_available_anchor_waves->GetSelectedEntry();
	string selected_wave = filePointer->GetTitle();
	if (available_waves.find(selected_wave)!=available_waves.end()){
		cout << " selected anchor wave is " << selected_wave << endl;
		current_anchor_wave = selected_wave;
	} else {
		current_anchor_wave = "";
	}
}

vector<string>& TrpwaPlotAmpsFrame::Scan_Fit_Result(TTree* fit_results, string branchName){
	vector<string>* result = new vector<string>();
	if (fit_results <= 0) return *result;
	cout << " scanning " << fit_results->GetName() << " for waves " << endl;
	double totIntensity(0);
	int nbinelements(0);
	const double intensityThr      = 0;            // threshold for total intensity in mass bin
	int nmbBinsAboveThr = 0;
	map<string, int> wavelist; // map with counts of all available waves
	map<string, double> totIntensities; // total intensities per wave name
	fitResult* massBin = new fitResult();
	fit_results->SetBranchAddress(branchName.c_str(), &massBin);
	for (int imassbin = 0; imassbin < fit_results->GetEntries(); imassbin++){
		fit_results->GetEntry(imassbin);
		nbinelements++;
		const double binIntensity = massBin->intensity();
		totIntensity += binIntensity;
		for (unsigned iwave = 0; iwave < massBin->waveNames().size(); iwave++){
			wavelist[massBin->waveNames()[iwave]]++;
			// found a new wave when the specific counter is 1
			if (wavelist[massBin->waveNames()[iwave]] == 1){
			}
			// calculate the total wave intensities
			// warning, several fit results per bin and wave name lead to a wrong calculation
			if (binIntensity > intensityThr) {
				totIntensities[massBin->waveNames()[iwave]]+=massBin->intensity(iwave);
				if (iwave == 0)
					++nmbBinsAboveThr;
			}
		}
	}

	for (map<string, int>::iterator it = wavelist.begin(); it != wavelist.end(); it++){
		result->push_back(it->first);
	}


	return *result;
}

void TrpwaPlotAmpsFrame::Plot_All_selected(){
	/*
	for (Tfilemapit it = selected_fit_results.begin(); it != selected_fit_results.end(); it++){
		TTree* selected_tree = (TTree*)it->second;
		cout << " Drawing all results from " << it->first << ". Please be patient... " << endl;
		plotAllIntensities(selected_tree, true);
		cout << " done " << endl;
		//box_available_fits->AddEntry(it->first.c_str(), (long int) it->second);
	}*/
	if (current_fit_result > 0){
		cout << " Drawing all results from " << current_fit_result->GetName() << ". Please be patient... " << endl;
		plotAllIntensities(current_fit_result, true);
		cout << " done " << endl;
	} else {
		cout <<  " no fit selected " << endl;
	}
}

void TrpwaPlotAmpsFrame::Plot_All_selected_Spin_totals(){
	cout << " plotSpinTotals() macro does not work yet properly and may crash! " << endl;
	//return;
	if (current_fit_result > 0){
		cout << " Drawing all spin totals from " << current_fit_result->GetName() << ". Please be patient... " << endl;
		plotSpinTotals(current_fit_result);
		cout << " done " << endl;
	} else {
		cout <<  " no fit selected " << endl;
	}
}

void TrpwaPlotAmpsFrame::Save_plot(){
	TGFileInfo fileinfo;
	const char* filetypes[] = {"pdf","*.pdf","eps","*.eps","gif","*.gif","png","*.png","root","*.root", 0, 0};
	fileinfo.fFileTypes = filetypes;
	stringstream filename;
	filename << current_wave;
	if (current_anchor_wave != "")
		filename << "_vs_" << current_anchor_wave;
	char *cstr = new char [filename.str().size()+1];
	strcpy (cstr, filename.str().c_str());
	fileinfo.fFilename = cstr;
	TGFileDialog* filedialog = new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fileinfo);
	if (filedialog && fileinfo.fFilename) {
		filename.str("");
		filename << fileinfo.fFilename;
		filename << "." << fileinfo.fFileTypes[fileinfo.fFileTypeIdx];
		canvas_selected_waves->Print(filename.str().c_str());
	}
	/*
	int returncode;
	stringstream filename;
	filename << current_wave;
	if (current_anchor_wave != "")
		filename << "_vs_" << current_anchor_wave;
	TGMsgBox* userrespondbox = new TGMsgBox(gClient->GetRoot(), this, "save current output",
			("Do you want to save this plot as\n"+filename.str()+".pdf/.root?").c_str(),
			kMBIconQuestion, (kMBYes | kMBNo), &returncode);
	if (!userrespondbox) cout << " this will be not executed " << endl; // to prevent compiler warnings
	if (returncode == kMBYes){
		canvas_selected_waves->Print((filename.str()+".pdf").c_str());
		canvas_selected_waves->Print((filename.str()+".root").c_str());
	}*/
}

// I could try to copy the fitResult class but I prefer to store
// only the for me relevant values
struct Twavebin_info {
	double massBinCenter;
	double logLikelihood;
	double waveIntensityA;
	double waveIntensityB;
	double waveIntensityErrA;
	double waveIntensityErrB;
	double phase;
	double phaseErr;
	double coherence;
	double coherenceErr;
};

void TrpwaPlotAmpsFrame::Plot_selected_wave(){
	const string branchName = "fitResult_v2";
	canvas_selected_waves->cd(1); gPad->Clear();
	canvas_selected_waves->cd(2); gPad->Clear();
	canvas_selected_waves->cd(3); gPad->Clear();
	canvas_selected_waves->cd(4); gPad->Clear();
	for (int ipad = 0; ipad < 5; ipad++){
		if (plotted_graphs[ipad]){
			plotted_graphs[ipad]->Clear();
			delete plotted_graphs[ipad];
			plotted_graphs[ipad] = NULL;
		}
		if (plotted_most_likely_graphs[ipad]){
			plotted_most_likely_graphs[ipad]->Clear();
			delete plotted_graphs[ipad];
			plotted_most_likely_graphs[ipad]=NULL;
		}
	}
	masscutlow = -1.;
	masscuthigh = -1.;
	slider_mass_range->SetPosition(0.,1.);
	string drawsame("");
	if (current_wave == "") return;
	int iwave(0);
	static int some_name_counter(0); // due to shitty root implementation for unique names
	stringstream some_name;
	for (Tfilemapit it = selected_fit_results.begin(); it != selected_fit_results.end(); it++){
		TTree* selected_tree = (TTree*)it->second;
		fitResult* massBin = new fitResult();
		int icolor = available_colors[iwave].rootcolorindex;
		iwave++;
		// search for the wave indexes and the valid mass bins
		// as well as for the smallest log likelihood in this bin
		selected_tree->SetBranchAddress(branchName.c_str(), &massBin);
		double masscutlowA(-1.), masscutlowB(-1.), masscuthighA(-1.), masscuthighB(-1.);
		int indexA(-1), indexB(-1);
		cout << " scanning fit result " << selected_tree->GetName() << endl;
		stringstream selectExpr;
		// this is not a nice construct:
		// a map of all available results for both waves
		// for the smallest log likelihood
		map < double, Twavebin_info > smallest_log_likes;
		for (int ibin = 0; ibin < selected_tree->GetEntries(); ibin++){
			selected_tree->GetEntry(ibin);
			double mass = massBin->massBinCenter();
			int _indexA = -1;
			if (current_wave != "")
				_indexA = massBin->waveIndex(current_wave);
			int _indexB = -1;
			if (current_anchor_wave != "")
				_indexB = massBin->waveIndex(current_anchor_wave);
			// find the smallest log likelihood per result and mass bin
			double loglike = massBin->logLikelihood();
			// hopefully doubles are initialized to 0 if this is the first entry!
			Twavebin_info* bin_info;
			if (smallest_log_likes.find(mass) == smallest_log_likes.end()){
				bin_info = &smallest_log_likes[mass]; // creates a new entry
				bin_info->logLikelihood = 0; // in principal it should be initialized already
			} else {
				bin_info = &smallest_log_likes[mass];
			}
			if (bin_info->logLikelihood > loglike){
				bin_info->logLikelihood = loglike;
				bin_info->massBinCenter = mass/1000.;
				bin_info->phase = 0;
				bin_info->phaseErr = 0;
				bin_info->coherence = 0;
				bin_info->coherenceErr = 0;
				if (_indexA != -1){
					bin_info->waveIntensityA =
							massBin->intensity   ((unsigned int)_indexA);
					bin_info->waveIntensityErrA =
							massBin->intensityErr((unsigned int)_indexA);
				}
				if (_indexB != -1){
					bin_info->waveIntensityB =
							massBin->intensity   ((unsigned int)_indexB);
					bin_info->waveIntensityErrB =
							massBin->intensityErr((unsigned int)_indexB);
				}
				if (_indexA != -1 && _indexB != -1){
					bin_info->phase        =
							massBin->phase       ((unsigned int) _indexA, (unsigned int) _indexB);
					bin_info->phaseErr     =
							massBin->phaseErr    ((unsigned int) _indexA, (unsigned int) _indexB);
					bin_info->coherence    =
							massBin->coherence   ((unsigned int) _indexA, (unsigned int) _indexB);
					bin_info->coherenceErr =
							massBin->coherenceErr((unsigned int) _indexA, (unsigned int) _indexB);
				}
			}
			if (_indexA != -1){
				if (indexA == -1) indexA = _indexA;
				if (indexA != _indexA){
					cout << " Warning in TrpwaPlotAmpsFrame::Plot_selected_wave():";
					cout << "wave index differs for " << current_wave;
					cout << ". Excluding from selection! " << endl;
					selectExpr << "massBinCenter() != " << mass << " && ";
				}
				if (masscutlowA  == -1.) masscutlowA  = mass;
				if (masscuthighA == -1.) masscuthighA = mass;
				if (mass < masscutlowA ) masscutlowA  = mass;
				if (mass > masscuthighA) masscuthighA = mass;
			}
			if (_indexB != -1){
				if (indexB == -1) indexB = _indexB;
				if (indexB != _indexB){
					cout << " Warning in TrpwaPlotAmpsFrame::Plot_selected_wave():";
					cout << "wave index differs for " << current_anchor_wave;
					cout << ". Excluding from selection! " << endl;
					selectExpr << "massBinCenter() != " << mass << " && ";
				}
				if (masscutlowB  == -1.) masscutlowB  = mass;
				if (masscuthighB == -1.) masscuthighB = mass;
				if (mass < masscutlowB ) masscutlowB  = mass;
				if (mass > masscuthighB) masscuthighB = mass;
			}
		}
		masscutlow = 0;
		masscuthigh = 0;
		if (indexA == -1) {
			cout << current_wave << " not found in " << selected_tree->GetName() << endl;
			continue;
		}
		masscutlow  = masscutlowA;
		masscuthigh = masscuthighA;

		if ((masscutlow != -1.) && (masscuthigh != -1.)){
			selectExpr << "(massBinCenter() >= "<< masscutlow << ") && (massBinCenter() <= " << masscuthigh << ")";
		} else {
			cout << " mass range for " << current_wave << " in " << selected_tree->GetName() << " is bad! " << endl;
			continue;
		}
		if (current_wave == "") continue; // should never happen due to cuts above

		// wave A intensity
		canvas_selected_waves->cd(1);
		TMultiGraph* _graph_pad1 =
		plotIntensity(selected_tree, indexA, false, icolor, false, "", "APZ", 1, 0,
				selectExpr.str(), branchName);
		if (!plotted_graphs[1]){
			plotted_graphs[1] = _graph_pad1;
		} else {
			plotted_graphs[1]->SetTitle(_graph_pad1->GetTitle());
			plotted_graphs[1]->Add(_graph_pad1);//, "APZ");
		}
		if (1){
			// plot the most likely solution
			int nbins = smallest_log_likes.size();
			double* x_val = new double[nbins];
			double* y_val = new double[nbins];
			double* y_val_err = new double[nbins];
			cout << " size of most likely graph is " << nbins;
			nbins = 0;
			for (map < double, Twavebin_info >::iterator it = smallest_log_likes.begin();
					it != smallest_log_likes.end(); it++){
				x_val[nbins] = it->second.massBinCenter;
				if (masscutlow/1000. <= x_val[nbins] && x_val[nbins] <= masscuthigh/1000.){
					y_val[nbins] = it->second.waveIntensityA;
					y_val_err[nbins] = it->second.waveIntensityErrA;
				    nbins++;
				}
			}
			cout << " filtered " << nbins << " events " << endl;
			TGraphErrors* most_likely = new TGraphErrors(nbins, x_val, y_val, NULL, y_val_err);
			some_name.str(""); some_name << "graph_number_" << some_name_counter++;
			most_likely->SetName(some_name.str().c_str());
			most_likely->SetMarkerColor(icolor);
			most_likely->SetMarkerStyle(21);
			most_likely->SetMarkerSize(0.5);
			int ipad = 1;
			if (!plotted_most_likely_graphs[ipad]){
				plotted_most_likely_graphs[ipad] = new TMultiGraph();
				some_name.str(""); some_name << "graph_number_" << some_name_counter++;
				plotted_most_likely_graphs[ipad]->SetName(some_name.str().c_str());
				plotted_most_likely_graphs[ipad]->Add(most_likely);//, "APZ");
				plotted_most_likely_graphs[ipad]->SetTitle(plotted_graphs[ipad]->GetTitle());
				gPad->Clear();
				plotted_most_likely_graphs[ipad]->Draw("APZ");
				plotted_most_likely_graphs[ipad]->GetXaxis()->SetTitle(plotted_graphs[ipad]->GetXaxis()->GetTitle());
				plotted_most_likely_graphs[ipad]->GetYaxis()->SetTitle(plotted_graphs[ipad]->GetYaxis()->GetTitle());
			} else {
				plotted_most_likely_graphs[ipad]->Add(most_likely);//, "APZ");
				gPad->Clear();
				plotted_most_likely_graphs[ipad]->Draw("APZ");
			}
		}
		gPad->Clear();
		plotted_graphs[1]->Draw("APZ");
		gPad->Update();

		if (current_anchor_wave == "" || indexB == -1){
			cout << " no anchor wave found " << endl;
			continue;
		}
		// determine the corresponding ranges
		if (masscutlowB  > masscutlow ) masscutlow  = masscutlowB;
		if (masscuthighB < masscuthigh) masscuthigh = masscuthighB;
		if ((masscutlow != -1.) && (masscuthigh != -1.)){
			selectExpr << "&& (massBinCenter() >= "<< masscutlow << ") && (massBinCenter() <= " << masscuthigh << ")";
		} else {
			cout << " mass range for " << current_anchor_wave << " in " << selected_tree->GetName() << " is bad! " << endl;
			continue;
		}
		// wave A - wave B phase angle
		canvas_selected_waves->cd(2);
		TMultiGraph* _graph_pad2 =
		plotPhase(selected_tree, indexA, indexB, false, icolor, false, "", "APZ",
				selectExpr.str(), branchName);
		if (!plotted_graphs[2]){
			plotted_graphs[2] = _graph_pad2;
		} else {
			plotted_graphs[2]->SetTitle(_graph_pad2->GetTitle());
			plotted_graphs[2]->Add(_graph_pad2);//, "APZ");
		}
		if (1){
			// plot the most likely solution
			int nbins = smallest_log_likes.size();
			double* x_val = new double[nbins];
			double* y_val = new double[nbins];
			double* y_val_err = new double[nbins];
			nbins = 0;
			for (map < double, Twavebin_info >::iterator it = smallest_log_likes.begin();
					it != smallest_log_likes.end(); it++){
				x_val[nbins] = it->second.massBinCenter;
				if (masscutlow/1000. <= x_val[nbins] && x_val[nbins] <= masscuthigh/1000.){
					y_val[nbins] = it->second.phase;
					y_val_err[nbins] = it->second.phaseErr;
				    nbins++;
				}
			}
			TGraphErrors* most_likely = new TGraphErrors(nbins, x_val, y_val, NULL, y_val_err);
			Rectify_phase(most_likely);
			some_name.str(""); some_name << "graph_number_" << some_name_counter++;
			most_likely->SetName(some_name.str().c_str());
			most_likely->SetMarkerColor(icolor);
			most_likely->SetMarkerStyle(21);
			most_likely->SetMarkerSize(0.5);
			int ipad = 2;
			if (!plotted_most_likely_graphs[ipad]){
				plotted_most_likely_graphs[ipad] = new TMultiGraph();
				some_name.str(""); some_name << "graph_number_" << some_name_counter++;
				plotted_most_likely_graphs[ipad]->SetName(some_name.str().c_str());
				plotted_most_likely_graphs[ipad]->Add(most_likely);//, "APZ");
				plotted_most_likely_graphs[ipad]->SetTitle(plotted_graphs[ipad]->GetTitle());
				gPad->Clear();
				plotted_most_likely_graphs[ipad]->Draw("APZ");
				plotted_most_likely_graphs[ipad]->GetXaxis()->SetTitle(plotted_graphs[ipad]->GetXaxis()->GetTitle());
				plotted_most_likely_graphs[ipad]->GetYaxis()->SetTitle(plotted_graphs[ipad]->GetYaxis()->GetTitle());
			} else {
				plotted_most_likely_graphs[ipad]->Add(most_likely);//, "APZ");
				gPad->Clear();
				plotted_most_likely_graphs[ipad]->Draw("APZ");
			}
		}
		gPad->Clear();
		plotted_graphs[2]->Draw("APZ");
		gPad->Update();

		// wave B intensity
		canvas_selected_waves->cd(3);
		TMultiGraph* _graph_pad3 =
		plotIntensity(selected_tree, indexB, false, icolor, false, "", "APZ", 1, 0,
				selectExpr.str(), branchName);
		if (!plotted_graphs[3]){
			plotted_graphs[3] = _graph_pad3;
		} else {
			plotted_graphs[3]->SetTitle(_graph_pad3->GetTitle());
			plotted_graphs[3]->Add(_graph_pad3);//, "APZ");
		}
		if (1){
			// plot the most likely solution
			int nbins = smallest_log_likes.size();
			double* x_val = new double[nbins];
			double* y_val = new double[nbins];
			double* y_val_err = new double[nbins];
			nbins = 0;
			for (map < double, Twavebin_info >::iterator it = smallest_log_likes.begin();
					it != smallest_log_likes.end(); it++){
				x_val[nbins] = it->second.massBinCenter;
				if (masscutlow/1000. <= x_val[nbins] && x_val[nbins] <= masscuthigh/1000.){
					y_val[nbins] = it->second.waveIntensityB;
					y_val_err[nbins] = it->second.waveIntensityErrB;
				    nbins++;
				}
			}
			TGraphErrors* most_likely = new TGraphErrors(nbins, x_val, y_val, NULL, y_val_err);
			some_name.str(""); some_name << "graph_number_" << some_name_counter++;
			most_likely->SetName(some_name.str().c_str());
			most_likely->SetMarkerColor(icolor);
			most_likely->SetMarkerStyle(21);
			most_likely->SetMarkerSize(0.5);
			int ipad = 3;
			if (!plotted_most_likely_graphs[ipad]){
				plotted_most_likely_graphs[ipad] = new TMultiGraph();
				some_name.str(""); some_name << "graph_number_" << some_name_counter++;
				plotted_most_likely_graphs[ipad]->SetName(some_name.str().c_str());
				plotted_most_likely_graphs[ipad]->Add(most_likely);//, "APZ");
				plotted_most_likely_graphs[ipad]->SetTitle(plotted_graphs[ipad]->GetTitle());
				gPad->Clear();
				plotted_most_likely_graphs[ipad]->Draw("APZ");
				plotted_most_likely_graphs[ipad]->GetXaxis()->SetTitle(plotted_graphs[ipad]->GetXaxis()->GetTitle());
				plotted_most_likely_graphs[ipad]->GetYaxis()->SetTitle(plotted_graphs[ipad]->GetYaxis()->GetTitle());
			} else {
				plotted_most_likely_graphs[ipad]->Add(most_likely);//, "APZ");
				gPad->Clear();
				plotted_most_likely_graphs[ipad]->Draw("APZ");
			}
		}
		gPad->Clear();
		plotted_graphs[3]->Draw("APZ");
		gPad->Update();

		// wave A - wave B coherence
		canvas_selected_waves->cd(4);
		TMultiGraph* _graph_pad4 =
		plotCoherence(selected_tree, indexA, indexB, false, icolor, false, "", "APZ",
		              selectExpr.str(), branchName);
		if (!plotted_graphs[4]){
			plotted_graphs[4] = _graph_pad4;
		} else {
			plotted_graphs[4]->SetTitle(_graph_pad4->GetTitle());
			plotted_graphs[4]->Add(_graph_pad4);//, "APZ");
		}
		if (1){
			// plot the most likely solution
			int nbins = smallest_log_likes.size();
			double* x_val = new double[nbins];
			double* y_val = new double[nbins];
			double* y_val_err = new double[nbins];
			nbins = 0;
			for (map < double, Twavebin_info >::iterator it = smallest_log_likes.begin();
					it != smallest_log_likes.end(); it++){
				x_val[nbins] = it->second.massBinCenter;
				if (masscutlow/1000. <= x_val[nbins] && x_val[nbins] <= masscuthigh/1000.){
					y_val[nbins] = it->second.coherence;
					y_val_err[nbins] = it->second.coherenceErr;
				    nbins++;
				}
			}
			TGraphErrors* most_likely = new TGraphErrors(nbins, x_val, y_val, NULL, y_val_err);
			some_name.str(""); some_name << "graph_number_" << some_name_counter++;
			most_likely->SetName(some_name.str().c_str());
			most_likely->SetMarkerColor(icolor);
			most_likely->SetMarkerStyle(21);
			most_likely->SetMarkerSize(0.5);
			int ipad = 4;
			if (!plotted_most_likely_graphs[ipad]){
				plotted_most_likely_graphs[ipad] = new TMultiGraph();
				some_name.str(""); some_name << "graph_number_" << some_name_counter++;
				plotted_most_likely_graphs[ipad]->SetName(some_name.str().c_str());
				plotted_most_likely_graphs[ipad]->Add(most_likely);//, "APZ");
				plotted_most_likely_graphs[ipad]->SetTitle(plotted_graphs[ipad]->GetTitle());
				gPad->Clear();
				plotted_most_likely_graphs[ipad]->Draw("APZ");
				plotted_most_likely_graphs[ipad]->GetXaxis()->SetTitle(plotted_graphs[ipad]->GetXaxis()->GetTitle());
				plotted_most_likely_graphs[ipad]->GetYaxis()->SetTitle(plotted_graphs[ipad]->GetYaxis()->GetTitle());
			} else {
				plotted_most_likely_graphs[ipad]->Add(most_likely);//, "APZ");
				gPad->Clear();
				plotted_most_likely_graphs[ipad]->Draw("APZ");
			}
		}
		gPad->Clear();
		plotted_graphs[4]->Draw("APZ");
		gPad->Update();
	}
	Set_Mass_range();
}

void TrpwaPlotAmpsFrame::Set_show_most_likely(){
	draw_most_likely = button_show_most_likely->GetState();
	Set_Mass_range();
}

void TrpwaPlotAmpsFrame::Set_Mass_range(){
	float mass_min, mass_max;
	slider_mass_range->GetPosition(mass_min, mass_max);
	mass_min = mass_min * (masscuthigh-masscutlow) + masscutlow;
	mass_max = mass_max * (masscuthigh-masscutlow) + masscutlow;
	mass_min /= 1000.; // drawn in GeV
	mass_max /= 1000.;
	//cout << " setting from "<< mass_min << " to " << mass_max << endl;
	for (int ipad = 0; ipad < 5; ipad++){
		if (!draw_most_likely && plotted_graphs[ipad]){
			canvas_selected_waves->cd(ipad);
			gPad->Clear();
			plotted_graphs[ipad]->Draw("APZ");
			plotted_graphs[ipad]->GetXaxis()->SetRangeUser(mass_min, mass_max);
			gPad->Update();
		}
		if (draw_most_likely && plotted_most_likely_graphs[ipad]){
			canvas_selected_waves->cd(ipad);
			gPad->Clear();
			plotted_most_likely_graphs[ipad]->Draw("APZ");
			plotted_most_likely_graphs[ipad]->GetXaxis()->SetRangeUser(mass_min, mass_max);
			gPad->Update();
		}
	}
}

void TrpwaPlotAmpsFrame::Rectify_phase(TGraphErrors* graph_phase){
	if (!graph_phase) return;
	int npoints = graph_phase->GetN();
	double* m = graph_phase->GetX();
	double* phase = graph_phase->GetY();
	double* phase_err = graph_phase->GetEY();
	int istart = 0;
	// first iteration: find the closest point to 0 degree (taking the error into account)
	for (int i = 0; i < npoints; i++){
		if (fabs(phase[i])+phase_err[i] < fabs(phase[istart])+phase_err[istart]){
			istart = i;
		}
	}
	// make the point not jump in phase
	for (int i = istart+1; i < npoints; i++){
		do{
			// define the "direction" to correct phase into
			double phase_diff = phase[i-1]-phase[i];
			int dir = (phase_diff < 0) ? -1 : +1;
			// check that a change in phase of 360° would effect
			// a point to sit nearer to the previous one
			phase_diff = fabs(phase_diff);
			if (phase_diff < 180) break;
			// put the point nearer to the previous one
			phase[i] = phase[i]+dir*360;
			// set also the point of the graph
			graph_phase->SetPoint(i, m[i], phase[i]);
		}while (1); // continue to move points in case of many phase jumps
	}
	// now the other direction
	for (int i = istart-1; i > -1; i--){
		do{
			// define the "direction" to correct phase into
			double phase_diff = phase[i+1]-phase[i];
			int dir = (phase_diff < 0) ? -1 : +1;
			// check that a change in phase of 360° would effect
			// a point to sit nearer to the previous one
			phase_diff = fabs(phase_diff);
			if (phase_diff < 180) break;
			// put the point nearer to the previous one
			phase[i] = phase[i]+dir*360;
			// set also the point of the graph
			graph_phase->SetPoint(i, m[i], phase[i]);
		}while (1); // continue to move points in case of many phase jumps
	}
}


#endif /* TRPWAPLOTAMPSFRAME_CC_ */
