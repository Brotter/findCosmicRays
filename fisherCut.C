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
  //  double fisher0 = -2.52904e0;
  //  double fisher1 =  4.61243e1;
  //  double fisher2 =  6.03295e-3;

  double fisher0 = -4.24578e0;
  double fisher1 =  5.05512e1;
  double fisher2 =  3.61643e-2;
  double fisher3 =  4.16148e-2;


  int passing = 0;
  int waisPassing = 0;
  int waisTotal = 0;
  int ldbPassing = 0;
  int ldbTotal = 0;
  
  const int numFailCodes = 6;
  bool currFailCode[numFailCodes] = {0};
  int waisCutCode[numFailCodes] = {0};
  int ldbCutCode[numFailCodes] = {0};

  for (int entry=0; entry<lenEntries; entry++) {
    if (entry % 100000 == 0) cout << entry << "/" << lenEntries << "(" << passing << ")" << endl;
    resultTree->GetEntry(entry);

    if (eventSummary->flags.pulser==1) waisTotal++;
    if (eventSummary->flags.pulser==2) ldbTotal++;
    
    double var0 = eventSummary->peak[0][0].value;
    double var1 = eventSummary->coherent[0][0].peakHilbert;
    double var2 = TMath::Sqrt(pow(eventSummary->coherent[0][0].Q,2) + pow(eventSummary->coherent[0][0].U,2) ) / eventSummary->coherent[0][0].I;
    fisherValue = fisher0 + var0*fisher1 + var1*fisher2 + var2*fisher3;

    memset(currFailCode, 0, sizeof(currFailCode));

    if (!(eventSummary->flags.isHPolTrigger == 1))  currFailCode[0] = true;
    if (!(fisherValue > -1))                        currFailCode[1] = true;
    if (!(eventSummary->flags.isPayloadBlast == 0)) currFailCode[2] = true;
    if (!(eventSummary->peak[0][0].value > 0 && eventSummary->flags.isRF==1)) currFailCode[3] = true;
    if (!(eventSummary->peak[0][0].theta < 60 && eventSummary->peak[0][0].theta > -60)) currFailCode[4] = true;
    if (eventSummary->coherent[0][0].peakFrequency[0] > 0.45 && 
	  eventSummary->coherent[0][0].peakFrequency[0] < 0.47 &&
	  eventSummary->coherent[0][0].peakPower[0] > 16.2 &&
	  eventSummary->coherent[0][0].peakPower[0] < 17.4)  currFailCode[5] = true;


    if (eventSummary->flags.pulser == 0) {
      if (*(int*)currFailCode == 0) {
	double lat = eventSummary->peak[0][0].latitude;
	double lon = eventSummary->peak[0][0].longitude;
	if (lat < -999 || lon < -999) continue;
	
	gpsTree->GetEntry(gpsTree->GetEntryNumberWithBestIndex(eventSummary->eventNumber));
	outTree->Fill();
	passing++;
      }
    }
    else if (eventSummary->flags.pulser == 1) {
      if (*(int*)currFailCode == 0) waisPassing++;
      for (int failCode=0; failCode<numFailCodes; failCode++) { waisCutCode[failCode] += (int)currFailCode[failCode]; } }
    else if (eventSummary->flags.pulser == 2) {
      if (*(int*)currFailCode == 0) ldbPassing++;
      for (int failCode=0; failCode<numFailCodes; failCode++) { ldbCutCode[failCode] += (int)currFailCode[failCode]; } }
    
  }

  cout << "Noise events passing: " << passing << endl;
  cout << "Wais Passing: " << waisPassing << " / " << waisTotal << "(" << float(waisPassing)/waisTotal << ")" << endl;
  for (int failCode=0; failCode<numFailCodes; failCode++) cout << " " << waisCutCode[failCode];
  cout << endl;
  cout << "Ldb Passing: " << ldbCutCode[0] << " / " << ldbTotal << "(" << float(ldbPassing)/ldbTotal << ")" << endl;
  for (int failCode=0; failCode<numFailCodes; failCode++) cout << " " << ldbCutCode[failCode];
  cout << endl;
  

  outFile->cd();
  outTree->Write();
  outFile->Close();

  return;
}


