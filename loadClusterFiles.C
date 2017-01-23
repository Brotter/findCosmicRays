#include "AnitaEventSummary.h"
#include <sys/stat.h>

void loadClusterFiles(string dataDir="0"){

  if (dataDir == "0") {
    dataDir = "01.19.17_22h";
  }

  TChain *resultTree = new TChain("headTree","headTree");

  
  stringstream name;
  struct stat buffer;   
  for (int run=130; run<170; run++) {
    name.str("");
    name << "/Volumes/ANITA3Data/bigAnalysisFiles/findCosmicRays/" << dataDir << "/" << run << "_corrected.root";

    if (stat (name.str().c_str(), &buffer) == 0) {
      resultTree->Add(name.str().c_str()); }
  }

 
  return;
 
}

    
