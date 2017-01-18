#include "AnitaEventSummary.h"

void makeGif(string dataDir){


  const int numFrames = 1000;


  TCanvas *c1 = new TCanvas("c1","c1",1000,800);
  
  TChain *inTree = new TChain("headTree","headTree");


  stringstream name;
  for (int run=130; run<384; run++) {
 
    c1->Clear();
   
    name.str("");
    name << dataDir << "/" << run << ".root";
    inTree->Add(name.str().c_str());

  }



  AnitaEventSummary * eventSummary = NULL;
  inTree->SetBranchAddress("eventSummary",&eventSummary);
  
    
  TH2D *hHilbVsMap = new TH2D("hHilbVsMap"," Interferometric Peak vs Hilbert Peak; Interferometric Peak (?); Hilbert Peak (mv)",
			      150,0,0.15,     100,0,100);
  
  TGraph *gWaisPulses = new TGraph();
  gWaisPulses->SetName("gWaisPulses");
  gWaisPulses->SetMarkerStyle(3);
  
  
  int lenEntries = inTree->GetEntries();
    
  int entriesPerFrame = lenEntries/numFrames;
  for (int frame=0; frame < numFrames; frame++) {
    c1->Clear();

    cout << frame << "/" << numFrames << endl;
    int startEntry = frame*numFrames;
    int stopEntry = (frame+1)*numFrames;

    for (int entry=startEntry; entry<stopEntry; entry++) {
    inTree->GetEntry(entry);
      
      if (eventSummary->flags.pulser == 0) {
	hHilbVsMap->Fill(eventSummary->peak[0][0].value,eventSummary->coherent[0][0].peakHilbert); }
      else if (eventSummary->flags.pulser == 1) {
	gWaisPulses->SetPoint(gWaisPulses->GetN(),
			      eventSummary->peak[0][0].value,
			      eventSummary->coherent[0][0].peakHilbert); }

    }
    

    c1->cd();
    hHilbVsMap->Draw("colz");
    if (gWaisPulses->GetN() > 0) {
      gWaisPulses->Draw("pSame"); }
    
    name.str("");
    name << "gifImages/" << frame << ".png";
    c1->SaveAs(name.str().c_str());

    gWaisPulses->Set(0);
    hHilbVsMap->Reset();

  }



}
