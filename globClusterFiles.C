#include "AnitaEventSummary.h"
#include <sys/stat.h>

void globClusterFiles(){


  TChain *resultTree = new TChain("headTree","headTree");

  
  stringstream name;
  struct stat buffer;   
  for (int run=130; run<384; run++) {
    name.str("");
    name << "/home/brotter/nfsShared/results/findCosmicRays/" << run << ".root";

    if (stat (name.str().c_str(), &buffer) == 0) {
      resultTree->Add(name.str().c_str()); }
  }


  TH2D *hInterVsHilb = new TH2D("hInterVsHilb","Interferometric Peak vs Hilbert Peak;Interferometric Peak (?); Hilbert Peak (mV)",   150,0,0.15,  100,0,100);

  

  //Lets make the summary object that I can shove into the output tree
  AnitaEventSummary *eventSummary = NULL;
  resultTree->SetBranchAddress("eventSummary",&eventSummary);
  

  int lenEntries = resultTree->GetEntries();
  cout << "Found " << lenEntries << " Entries" << endl;

  for (int entry=0; entry< lenEntries; entry++) {
    if (entry%10000 == 0) {
      cout << entry << "/" << lenEntries << "\r";
      fflush(stdout);
    }

    resultTree->GetEntry(entry);
    hInterVsHilb->Fill(eventSummary->peak[0][0].value,eventSummary->coherent[0][0].peakHilbert);
  }

  hInterVsHilb->Draw("colz");
 
  return;
 
}

    
