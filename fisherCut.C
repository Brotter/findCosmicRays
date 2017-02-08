#include "AnitaEventSummary.h"

void fisherCut() {

  
  TChain *resultTree = new TChain("headTree","headTree");
  char* resultsDir = getenv("ANITA3_RESULTSDIR");

  TChain *gpsTree = new TChain("adu5PatTree","adu5PatTree");
  char* dataDir = getenv("ANITA3_DATA");

  stringstream name;
  for (int run=130; run<440; run++) {
    name.str("");
    name << resultsDir << run << ".root";
    resultTree->Add(name.str().c_str());

    name.str("");
    name << dataDir << "run" << run << "/gpsEvent" << run << ".root";
    gpsTree->Add(name.str().c_str());
  }
  
  AnitaEventSummary *eventSummary = NULL;
  resultTree->SetBranchAddress("eventSummary",&eventSummary);

  Adu5Pat *pat = NULL;
  gpsTree->SetBranchAddress("pat",&pat);
  gpsTree->BuildIndex("eventNumber");

  TFile *outFile = TFile::Open("fisherCut.root","recreate");
  TTree *outTree = new TTree("fisherCut","fisherCut");
  outTree->Branch("eventSummary",&eventSummary);
  outTree->Branch("pat",&pat);
  double fisherValue;
  outTree->Branch("fisherValue",&fisherValue,"fisherValue/D");
  

  int lenEntries = resultTree->GetEntries();

  //fisher discriminants (F0 + F1*a0 + F2*a1)
  //from TMVABen.C getting like 1e-4 noise rejection
  double fisher0 = -2.52904e0;
  double fisher1 =  4.61243e1;
  double fisher2 =  6.03295e-3;

  int passing = 0;
  int pulsesPassing = 0;
  int pulsesFailing = 0;
  
  for (int entry=0; entry<lenEntries; entry++) {
    if (entry % 100000 == 0) cout << entry << "/" << lenEntries << "(" << passing << ")" << endl;
    resultTree->GetEntry(entry);
    
    double var0 = eventSummary->peak[0][0].value;
    double var1 = eventSummary->coherent[0][0].peakHilbert;
    fisherValue = fisher0 + var0*fisher1 + var1*fisher2;

    if (eventSummary->flags.pulser !=0) {
      if (fisherValue > 0) pulsesPassing++;
      else pulsesFailing++;
    }

    if (eventSummary->flags.pulser == 0 && eventSummary->flags.isHPolTrigger == 1 && fisherValue > 0) {
      double lat = eventSummary->peak[0][0].latitude;
      double lon = eventSummary->peak[0][0].longitude;
      if (lat < -999 || lon < -999) continue;
    
      gpsTree->GetEntry(gpsTree->GetEntryNumberWithBestIndex(eventSummary->eventNumber));
      outTree->Fill();
      passing++;
    }
    
  }

  cout << "passing: " << passing << endl;
  cout << "pulsesPassing: " << pulsesPassing << " | failing: " << pulsesFailing << endl;

  outFile->cd();
  outTree->Write();
  outFile->Close();

  return;
}


