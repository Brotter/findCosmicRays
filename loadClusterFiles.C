#include "AnitaEventSummary.h"
#include <sys/stat.h>

void loadClusterFiles(string dataDir="0"){

  if (dataDir == "0") {
    dataDir = "/Volumes/ANITA3Data/bigAnalysisFiles/findCosmicRays/01.19.17_22h";
    //    dataDir = "01.23.17_10h";
  }

  gROOT->ProcessLine(".x setupProof.C");

  TChain *resultTree = new TChain("headTree","headTree");

  stringstream name;
  struct stat buffer;   
  for (int run=130; run<440; run++) {
    name.str("");
    name <<  dataDir << "/" << run << ".root";

    if (stat (name.str().c_str(), &buffer) == 0) {
      resultTree->Add(name.str().c_str()); }
  }

  resultTree->SetProof();
 
  return;
 
}

    
