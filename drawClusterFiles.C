void drawClusterFiles() {

  TChain *resultTree = new TChain("headTree","headTree");

  
  stringstream name;
  for (int run=130; run<440; run++) {
    name.str("");
    name << "/Volumes/ANITA3Data/bigAnalysisFiles/findCosmicRays/01.19.17_22h/" << run << ".root";
    resultTree->Add(name.str().c_str());
  }

  cout << resultTree->GetEntries() << endl;

  TH2D *hNoise = new TH2D("hNoise","hNoise",400,0,0.04,400,0,0.4);
  TH2D *hPulsers = new TH2D("hPulsers","hPulsers",400,0,0.04,400,0,0.4);


  resultTree->Draw("peak[1][0].value:deconvolved[1][0].peakHilbert >> hNoise","flags.pulser == 0 && flags.isVPolTrigger == 1 && flags.isPayloadBlast == 0", "");
  resultTree->Draw("peak[1][0].value:deconvolved[1][0].peakHilbert >> hPulsers","flags.pulser != 0 && flags.isVPolTrigger == 1 && peak[1][0].masked == 0", "");
							  

  TExec *ex1 = new TExec("ex1","gStyle->SetPalette(1);");
  TExec *ex2 = new TExec("ex2","gStyle->SetPalette(53);");
    
  TCanvas *c1 = new TCanvas("c1","c1",1000,800);
  c1->SetLogz();

  ex1->Draw();
  hNoise->Draw("colz");
  ex2->Draw();
  hPulsers->Draw("colzSame");

  c1->SaveAs("allPulses.png");

}
