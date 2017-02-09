#include <iostream>
#include <vector>
#include <math.h>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <random>
//root
#include "TTree.h"
#include "TChain.h"
#include "TFile.h"
#include "TH2.h"
#include "TH2D.h"
#include "TEllipse.h" 
#include "TMarker.h" 
#include "TStyle.h" 
#include "TCanvas.h"
//anita
#include "RawAnitaEvent.h"
#include "RawAnitaHeader.h"
#include "UsefulAdu5Pat.h"
#include "Adu5Pat.h"
#include "CalibratedAnitaEvent.h"
#include "UsefulAnitaEvent.h"
//cosmin's stuff
#include "AnitaEventSummary.h"
#include "AnalysisConfig.h" 
#include "UCFilters.h" 
#include "PeakFinder.h" 
#include "FilterStrategy.h" 
#include "Correlator.h" 
#include "Analyzer.h" 
#include "WaveformCombiner.h"
#include "AnitaVersion.h"

using namespace std;


double cartesianDist(Double_t A[3], Double_t B[3]) {
  return TMath::Sqrt( pow(A[0]-B[0],2) + pow(A[1]-B[1],2) + pow(A[2]-B[2],2) );

}

double addQuad(double A, double B){
  
  return TMath::Sqrt(pow(A,2) + pow(B,2));
    
}


//
//Ben Rotter - Feb 2017 - UH Manoa
//
//
//
//
//
//
//just like clusterer.C, except it can be run piecemeal becuase I don't want to learn threading
//also probably the only one I'll be updating
//
//
//run just clusterClusterer.C to do it all
//
//basically, for every point, determine the "closeness" to the nearest neighbor somehow.
//
// also have to rewrite it as an actual code because otherwise it eats all memory
//


void clusterClusterer(string outFileName, int numCores=1, int core=0){


  //need to do this all the time now :(
  AnitaVersion::set(3);


  cout << "Hello!  Lets cluster some events!" << endl;
  cout << "Core " << core << "/" << numCores << endl;


  string fisherCutName = "fisherCut.root";

  TFile *resultFile = TFile::Open("fisherCut.root");
  TTree *resultTree = (TTree*)resultFile->Get("fisherCut");

  AnitaEventSummary *eventSummary = NULL;
  resultTree->SetBranchAddress("eventSummary",&eventSummary);
  int lenEntries = resultTree->GetEntries();

  Adu5Pat *pat = NULL;
  resultTree->SetBranchAddress("pat",&pat);

  cout << "Got the things that passed cuts, there were " << lenEntries << " of them" << endl;

  //UsefulAdu5Pat has a nice thing for taking a lat/lon and turning it back into an angle
  //this should be included in the fisherCut resultTree now... but just check to make sure
  //  char* dataDir = getenv("ANITA3_DATA");
  //  stringstream name;
  //  if (gpsTree->GetEntries() == 0) {
  //  cout << "didn't find the gps stuff in the resultTree so I'm loading it all" << endl;
  //  for (int run=130; run<440; run++) {
  //    name.str("");
  //    name << dataDir << "run" << run << "/gpsEvent" << run << ".root";
  //      gpsTree->Add(name.str().c_str());
  //  }
  //  }
  
  TH1D *hPhiDist = new TH1D("hPhiDist","Phi;root sum square angular separation;occupancy",
			    361,-180,180);
  TH1D *hThetaDist = new TH1D("hThetaDist","Theta;root sum square angular separation;occupancy",
			      361,-180,180);
  
  TGraph *gCloseEvs = new TGraph();
  gCloseEvs->SetName("gCloseEvs");
  gCloseEvs->SetTitle("gCloseEvs");

  
  TFile *outFile = TFile::Open(outFileName.c_str(),"recreate");
  double currPhi,currTheta,closestPhi,closestTheta,closestQuad,lat,lon,alt;
  int run,eventNumber,closestEv;
  TTree *outTree = new TTree("clusterTree","clusterTree");
  outTree->Branch("currPhi",&currPhi,"currPhi/D");
  outTree->Branch("currTheta",&currTheta,"currTheta/D");
  outTree->Branch("closestPhi",&closestPhi,"closestPhi/D");
  outTree->Branch("closestTheta",&closestTheta,"closestTheta/D");
  outTree->Branch("closestQuad",&closestQuad,"closestQuad/D");
  outTree->Branch("lat",&lat,"lat/D");
  outTree->Branch("lon",&lon,"lon/D");
  outTree->Branch("alt",&alt,"alt/D");
  outTree->Branch("run",&run,"run/I");
  outTree->Branch("eventNumber",&eventNumber,"eventNumber/I");
  outTree->Branch("closestEv",&closestEv,"closestEv/I");



  //separate into cores
  int numEvPerCore = lenEntries/numCores;
  int startEntry = core*numEvPerCore;
  int stopEntry  = (core+1)*numEvPerCore;
  if (core == numCores-1) stopEntry = lenEntries; //make sure you get the last ones :)
  

  for (int entry=startEntry; entry<stopEntry; entry++) {
    resultTree->GetEntry(entry);
    eventNumber = eventSummary->eventNumber;
    run = eventSummary->run;
    
    UsefulAdu5Pat *usefulPat = new UsefulAdu5Pat(pat);

    closestPhi = 999;
    closestTheta = 999;
    closestQuad = addQuad(closestPhi,closestTheta);
    closestEv = -999;

    currPhi = eventSummary->peak[0][0].phi;
    currTheta = eventSummary->peak[0][0].theta;

    if (entry%1 == 0) cout << entry << "/" << lenEntries << " (" << eventNumber << ")" << endl;

    //still gotta loop over them all to find nearest neighbor though obnoxiously
    for (int entry2=0; entry2<lenEntries; entry2++) {
      if (entry == entry2) continue; // don't want to compare it to itself! :P
      resultTree->GetEntry(entry2);

      double otherPhi = -999;
      double otherTheta=-999;

      usefulPat->getThetaAndPhiWave(eventSummary->peak[0][0].longitude,
				    eventSummary->peak[0][0].latitude,
				    eventSummary->peak[0][0].altitude,
				    otherTheta,otherPhi);
  
      otherPhi *= (180./TMath::Pi());
      otherTheta *= (180./TMath::Pi());

      double phiDiff = currPhi - otherPhi;
      while (phiDiff > 180) phiDiff -= 180;
      while (phiDiff < -180) phiDiff += 180;
      double thetaDiff = currTheta - otherTheta;
      while (thetaDiff > 180) thetaDiff -= 180;
      while (thetaDiff < -180) thetaDiff += 180;

      //basically how "close" are they, which should take into account the errors for each
  
      double phiSigma = 1;
      double thetaSigma = 1;
  

      double quadDiff = addQuad(phiDiff/phiSigma,thetaDiff/thetaSigma); 


      if (closestQuad > quadDiff ) { 
	//	cout << entry2 < " || " << closestQuad << " | " << currPhi << " - " << otherPhi << " = " << phiDiff << " " << currTheta << " - " << otherTheta << " = " << thetaDiff << " | " <<  quadDiff << endl;

	closestPhi = phiDiff;
	closestTheta = thetaDiff;
	closestQuad = addQuad(phiDiff,thetaDiff);
	closestEv = eventSummary->eventNumber;
      }

    }

    printf("closest: %d(%d) -> %d : %f %f\n",eventNumber,run,closestEv,closestPhi,closestTheta);
    //    cout << "closest: " << closestEv << ": " << closestPhi << " | " << closestTheta << endl;
    
    gCloseEvs->SetPoint(entry,eventNumber,closestEv);
    hPhiDist->Fill(closestPhi);
    hThetaDist->Fill(closestTheta);
    
    outTree->Fill();

    delete usefulPat;

  }

  outFile->cd();
  outTree->Write();
  gCloseEvs->Write();
  hPhiDist->Write();
  hThetaDist->Write();
  outFile->Close();
  

}


int main(int argc, char** argv) {

  int numCores,core;
  string outFileName;
  if (argc == 4) {
    outFileName = argv[1];
    numCores = atoi(argv[2]);
    core = atoi(argv[3]);
  }
  else {
    cout << "Usage: " << argv[0] << " [outFileName] [numCores] [core]" << endl;
    return -1;
  }

  clusterClusterer(outFileName,numCores,core);

  return 1;

}
  
