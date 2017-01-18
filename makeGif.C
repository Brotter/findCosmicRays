void makeGif(string dataDir){


  TCanvas *c1 = new TCanvas("c1","c1",1000,800);
  

  stringstream name;
  for (int run=130; run<384; run++) {
 
    c1->Clear();
   
    name.str("");
    name << dataDir << "/" << run << ".root";
    TFile* inFile = TFile::Open(name.str().c_str());
    
    if (inFile == NULL) {
      cout << "run " << run << " does not exist" << endl;
      continue;
    }

    TTree *inTree = (TTree*)inFile->Get("headTree");


    AnitaEventSummary * eventSummary = NULL;
    inTree->SetBranchAddress("eventSummary",&eventSummary);
    
    
    TH2D *hHilbVsMap = new TH2D("hHilbVsMap"," Interferometric Peak vs Hilbert Peak; Interferometric Peak (?); Hilbert Peak (mv)",
				150,0,0.15,     100,0,100);
    
    TGraph *gWaisPulses = new TGraph();
    gWaisPulses->SetName("gWaisPulses");
    gWaisPulses->SetMarkerStyle(3);


    int lenEntries = inTree->GetEntries();
    
    for (int entry=0; entry<lenEntries; entry++) {
      if (entry% 1000 == 0) cout << entry << "/" << lenEntries << endl;
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
    gWaisPulses->Draw("pSame");
    
    name.str("");
    c1->SaveAs("output.gif+");


    delete inTree;
    inFile->Close();
    delete gWaisPulses;
    delete hHilbVsMap;

  }



}
