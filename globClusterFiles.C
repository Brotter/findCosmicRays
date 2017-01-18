#include "AnitaEventSummary.h"
#include <sys/stat.h>

void globClusterFiles(){


  TChain *resultTree = new TChain("headTree","headTree");

  
  stringstream name;
  struct stat buffer;   
  for (int run=130; run<193; run++) {
    name.str("");
    name << "/home/brotter/nfsShared/results/findCosmicRays/" << run << ".root";

    if (stat (name.str().c_str(), &buffer) == 0) {
      resultTree->Add(name.str().c_str()); }
  }


  TH2D *hInterVsHilbH = new TH2D("hInterVsHilbH","Interferometric Peak vs Hilbert Peak (Hpol);Interferometric Peak (?); Hilbert Peak (mV)",   150,0,0.15,  100,0,100);

  TH2D *hInterVsHilbV = new TH2D("hInterVsHilbV","Interferometric Peak vs Hilbert Peak (Vpol);Interferometric Peak (?); Hilbert Peak (mV)",   150,0,0.15,  100,0,100);

  TH2D *hWaisPulses = new TH2D("hWaisPulses","Interferometric Peak vs Hilbert Peak (WAIS Pulses);Interferometric Peak (?); Hilbert Peak (mV)",   150,0,0.15,  100,0,100);


  TH2D *hLDBPulses = new TH2D("hLDBPulses","Interferometric Peak vs Hilbert Peak (LDB Pulses);Interferometric Peak (?); Hilbert Peak (mV)",   150,0,0.15,  100,0,100);

  

  //Lets make the summary object that I can shove into the output tree
  AnitaEventSummary *eventSummary = NULL;
  resultTree->SetBranchAddress("eventSummary",&eventSummary);
  

  int lenEntries = resultTree->GetEntries();
  cout << "Found " << lenEntries << " Entries" << endl;

  int skippedEvents = 0;

  for (int entry=0; entry< lenEntries; entry++) {
    if (entry%10000 == 0) {
      cout << entry << "/" << lenEntries << "\r";
      fflush(stdout);
    }

    resultTree->GetEntry(entry);

    //cuts
    if ( (eventSummary->flags.isPayloadBlast == 1) ||
	 (eventSummary->flags.isGood == 0) ||
	 (eventSummary->peak[0][0].masked == 1) ) {
      skippedEvents++;
      continue;
    }

    
    //split into pulser and non-pulser
    

    if (eventSummary->flags.pulser == 1) {
      hWaisPulses->Fill(eventSummary->peak[0][0].value,eventSummary->coherent[0][0].peakHilbert);
    }
    else if (eventSummary->flags.pulser == 2) {
      hLDBPulses->Fill(eventSummary->peak[0][0].value,eventSummary->coherent[0][0].peakHilbert);
    }
    else {
      if (eventSummary->flags.isHPolTrigger == 1) {
	hInterVsHilbH->Fill(eventSummary->peak[0][0].value,eventSummary->coherent[0][0].peakHilbert); }
      if (eventSummary->flags.isVPolTrigger == 1) {
	hInterVsHilbV->Fill(eventSummary->peak[1][0].value,eventSummary->coherent[1][0].peakHilbert); }
    }

  }

  cout << "Skipped " << skippedEvents << " events from cuts" << endl;

  TFile *outFile = TFile::Open("globClusterFiles.root","recreate");

  hWaisPulses->Write();
  hLDBPulses->Write();
  hInterVsHilbH->Write();
  hInterVsHilbV->Write();

  outFile->Close();
 
  return;
 
}

    
