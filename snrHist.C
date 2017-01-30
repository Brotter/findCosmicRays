#include "AnitaEventSummary.h"

void snrHist() {


  TChain *inTree = new TChain("headTree","headTree");

  string dataDir = "/Volumes/ANITA3Data/bigAnalysisFiles/findCosmicRays/01.19.17_22h";

  stringstream name;
  for (int run=130; run<440; run++) {
 
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
  
  TH1D *hSNR = new TH1D("hSNR","Pulser SNR;SNR;Occupancy",1000,0,10.0);
  TH1D *hSNR_corr = new TH1D("hSNR_corr","Pulser SNR;SNR;Occupancy",1000,0,10.0);
  
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
    
    hSNR->Fill(eventSummary->coherent[0][0].snr);
    hSNR_corr->Fill(eventSummary->coherent[0][0].snr/TMath::Sqrt(10.));
  
  }


  TCanvas *c1 = new TCanvas("c1","c1",1000,800);
  
  hSNR->Draw();
  hSNR_corr->SetLineColor(kRed);
  hSNR_corr->Draw("same");

}
