#include "AnitaEventSummary.h"

void makeGif(string dataDir){


  const int numFrames = 1000;


  TCanvas *c1 = new TCanvas("c1","c1",1000,800);
  
  TChain *inTree = new TChain("headTree","headTree");


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
  
    
  TH2D *hHilbVsMap = new TH2D("hHilbVsMap","Deconvolved vs Map Peaks;Deconvolved Hilbert Peak; Map Peak",
			      400,0,0.04,     400,0,0.4);
  
  TGraph *gWaisPulses = new TGraph();
  gWaisPulses->SetName("gWaisPulses");
  gWaisPulses->SetMarkerStyle(3);

  int numCutPulses = 0;
  int numPulses = 0;
  TGraph *gCutPulses = new TGraph();
  gWaisPulses->SetName("gCutPulses");
  gWaisPulses->SetMarkerStyle(3);
  
  
  int lenEntries = inTree->GetEntries();
    
  int numPassingCuts = 0;
  TGraph *gPassingCuts = new TGraph();
  gPassingCuts->SetName("gPassingCuts");

  int entriesPerFrame = lenEntries/numFrames;
  for (int frame=0; frame < numFrames; frame++) {
    c1->Clear();

    cout << frame << "/" << numFrames << endl;
    int startEntry = frame*entriesPerFrame;
    int stopEntry = (frame+1)*entriesPerFrame;

    for (int entry=startEntry; entry<stopEntry; entry++) {
      inTree->GetEntry(entry);
      
      if (eventSummary->flags.isHPolTrigger == 0) continue;
      if (eventSummary->flags.isPayloadBlast == 1) continue;
      if (eventSummary->peak[0][0].masked == 1) continue;
      
      double X = eventSummary->deconvolved[0][0].peakHilbert;
      double Y = eventSummary->peak[0][0].value;


      if (eventSummary->flags.pulser == 0) {
	hHilbVsMap->Fill(X,Y);
	if ( cutLine->Eval(X) < Y ) {
	  numPassingCuts++;
	  gPassingCuts->SetPoint(gPassingCuts->GetN(),eventSummary->eventNumber,
				 X); }
      }
      else {
	gWaisPulses->SetPoint(gWaisPulses->GetN(),X,Y); 
	numPulses++;
	if ( cutLine->Eval(X) > Y ) {
	  gCutPulses->SetPoint(gCutPulses->GetN(),eventSummary->eventNumber,X);
	  numCutPulses++;
	}
      }
      
      

    }
    

    c1->cd();

    name.str("");
    name << "Interferometric Peak vs Hilbert Peak (run " << eventSummary->run << ")";
    name << "; Interferometric Peak; Hilbert Peak (mv)";
    hHilbVsMap->SetTitle(name.str().c_str());
    hHilbVsMap->SetMaximum(100);
    hHilbVsMap->SetMinimum(0);
    hHilbVsMap->Draw("colz");
    cutLine->Draw("same");
    if (gWaisPulses->GetN() > 0) {
      gWaisPulses->Draw("pSame"); }
    
    name.str("");
    name << "gifImages/" << frame << ".png";
    c1->SaveAs(name.str().c_str());

    gWaisPulses->Set(0);
    hHilbVsMap->Reset();

  cout << " Num Passing Cuts " << numPassingCuts << endl;
  cout << " Num Pulser events cut " << numCutPulses << "/" << numPulses << "(" << float(numCutPulses)/numPulses << ")" << endl;
  }


  TFile *outFile = TFile::Open("passingCuts.root","recreate");
  gPassingCuts->Write();
  gCutPulses->Write();
  outFile->Close();


}
