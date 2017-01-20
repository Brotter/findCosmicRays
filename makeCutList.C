#include "AnitaEventSummary.h"

void makeCutList(string dataDir="0") {

  /*

    The "Hilbert Peak vs Interferometric Map Peak" graph has a really clear separation between
    the WAIS pulser events and the whatever thermals in run 342.
    
    Make a cut on them.  Use that cut once again when you get all the data

   */

  if (dataDir == "0") {
    cout << "Using default directory 01.19.17_22h" << endl;
    dataDir = "01.19.17_22h";
  }


  TChain *inTree = new TChain("headTree","headTree");

  stringstream name;
  for (int run=130; run<194; run++) {
    name.str("");
    name << "/home/brotter/nfsShared/results/findCosmicRays/" << dataDir << "/" << run << ".root";
    inTree->Add(name.str().c_str());
  }


  AnitaEventSummary * eventSummary = NULL;
  inTree->SetBranchAddress("eventSummary",&eventSummary);


  TH2D *hHilbVsMap = new TH2D("hHilbVsMap"," Interferometric Peak vs Hilbert Peak; Interferometric Peak (?); Hilbert Peak (mv)",
			      150,0,0.15,     100,0,100);


  AntarcticaMapPlotter *aMap = new AntarcticaMapPlotter("mapPlotter","mapPlotter",1000,1000);
  aMap->addHistogram("pointMap","pointMap",1000,1000);


  //draw cut line
  TF1 *normalToFit = new TF1("normalToFit","[0]*x + [1]",0,0.15);
  normalToFit->SetParameter(0,-7455.77);
  normalToFit->SetParameter(1,425);
  normalToFit->SetLineColor(kBlue);
  normalToFit->Draw("same");


  int numPassingCuts = 0;

  int lenEntries = inTree->GetEntries();  
  for (int entry=0; entry<lenEntries; entry++) {
    if (entry% 1000 == 0) cout << entry << "/" << lenEntries << endl;
    inTree->GetEntry(entry);

    int whichPol = eventSummary->flags.isVPolTrigger;

    double X = eventSummary->peak[whichPol][0].value;
    double Y = eventSummary->coherent[whichPol][0].peakHilbert;
    hHilbVsMap->Fill(X,Y);
    
    if (normalToFit->Eval(X) < Y) {
      //      cout << eventSummary->eventNumber << endl;
      numPassingCuts++;
      aMap->setCurrentHistogram("pointMap");
      aMap->Fill(eventSummary->peak[whichPol][0].latitude,eventSummary->peak[0][0].longitude);


    }


  }

  TCanvas *c1 = new TCanvas("c1","c1",1000,800);
  c1->Divide(2,1);

  c1->cd(1);
  aMap->DrawHist("colz");

  c1->cd(2);
  hHilbVsMap->Draw("colz");
  normalToFit->Draw("same");

  cout << "numPassingCuts: " << numPassingCuts << endl;

  return;


}
