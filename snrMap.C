#include "AnitaEventSummary.h"

void snrMap() {


  TCanvas *c1 = new TCanvas("c1","c1",1000,800);
  
  TChain *inTree = new TChain("headTree","headTree");

  string dataDir = "/Volumes/ANITA3Data/bigAnalysisFiles/findCosmicRays/01.19.17_22h";

  stringstream name;
  for (int run=130; run<440; run++) {
 
    c1->Clear();
   
    name.str("");
    name << dataDir << "/" << run << ".root";
    inTree->Add(name.str().c_str());

  }

  TF1 *cutLine = new TF1("cutLine","[0]*x + [1]",0,0.4);
  cutLine->SetParameter(0,-(0.1/.005));
  cutLine->SetParameter(1,0.1);
  cutLine->SetLineColor(kRed);

  AnitaEventSummary * eventSummary = NULL;
  inTree->SetBranchAddress("eventSummary",&eventSummary);
  
    
  TProfile2D *psnrMap = new TProfile2D("psnrMap","SNR map;Deconvolved Hilbert Peak; Map Peak",
			      400,0,0.04,     400,0,0.4);
  
  int lenEntries = inTree->GetEntries();
  for (int entry=0; entry<lenEntries; entry++) {
    if (entry % 10000 == 0) cout << entry << "/" << lenEntries << endl;
    inTree->GetEntry(entry);
    
    if (eventSummary->flags.isHPolTrigger == 0) continue;
    if (eventSummary->flags.isPayloadBlast == 1) continue;
    if (eventSummary->peak[0][0].masked == 1) continue;
    if (eventSummary->flags.pulser == 0 ) continue;
    
    double X = eventSummary->deconvolved[0][0].peakHilbert;
    double Y = eventSummary->peak[0][0].value;
    
    psnrMap->Fill(X,Y,eventSummary->coherent[0][0].snr);
    
    
  }
  

    c1->cd();
    psnrMap->Draw("colz");

}
