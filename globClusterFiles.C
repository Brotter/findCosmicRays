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

  TH2D *hWaisPulses = new TH2D("hWaisPulses","Interferometric Peak vs Hilbert Peak (WAIS Pulses);Interferometric Peak (?); Hilbert Peak (mV)",   150,0,0.15,  100,0,100);


  TH2D *hLDBPulses = new TH2D("hLDBPulses","Interferometric Peak vs Hilbert Peak (WAIS Pulses);Interferometric Peak (?); Hilbert Peak (mV)",   150,0,0.15,  100,0,100);

  

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
      hLDB->Fill(eventSummary->peak[0][0].value,eventSummary->coherent[0][0].peakHilbert);
    }
    else {
      hInterVsHilb->Fill(eventSummary->peak[0][0].value,eventSummary->coherent[0][0].peakHilbert);
    }

  }

  cout << "Skipped " << skippedEvents << " events from cuts" << endl;

  hInterVsHilb->Draw("colz");
 
  return;
 
}

    
