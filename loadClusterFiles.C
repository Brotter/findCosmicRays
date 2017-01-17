#include "AnitaEventSummary.h"
#include <sys/stat.h>

void loadClusterFiles(){


  TChain *resultTree = new TChain("headTree","headTree");

  
  stringstream name;
  struct stat buffer;   
  for (int run=130; run<384; run++) {
    name.str("");
    name << "/home/brotter/nfsShared/results/findCosmicRays/" << run << ".root";

    if (stat (name.str().c_str(), &buffer) == 0) {
      resultTree->Add(name.str().c_str()); }
  }

 
  return;
 
}

    
