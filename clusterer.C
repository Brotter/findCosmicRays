#include "AnitaEventSummary.h"
#include "AnitaVersion.h"


double cartesianDist(Double_t A[3], Double_t B[3]) {
  return TMath::Sqrt( pow(A[0]-B[0],2) + pow(A[1]-B[1],2) + pow(A[2]-B[2],2) );

}

double addQuad(double A, double B){
  
  return TMath::Sqrt(pow(A,2) + pow(B,2));
    
}

//
//Ben Rotter - Feb 2017 - UH Manoa
//
//
//
//depreciated... use clusterClusterer.C
//
//
//
//
//



void clusterer(){

  //need to do this all the time now :(
  AnitaVersion::set(3);


  cout << "Hello!  Lets cluster some events!" << endl;

  //basically, for every point, determine the "closeness" to the nearest neighbor somehow.

  
  TChain *resultTree = new TChain("fisherCut","fisherCut");

  resultTree->Add("fisherCut.root");
  
  AnitaEventSummary *eventSummary = NULL;
  resultTree->SetBranchAddress("eventSummary",&eventSummary);
  int lenEntries = resultTree->GetEntries();

  cout << "Got the things that passed cuts, there were " << lenEntries << " of them" << endl;


  cout << "loading the gps and indexing it which takes awhile sorry" << endl;
  //UsefulAdu5Pat has a nice thing for taking a lat/lon and turning it back into an angle
  TChain *gpsTree = new TChain("adu5PatTree","adu5PatTree");
  char* dataDir = getenv("ANITA3_DATA");
  stringstream name;
  for (int run=130; run<440; run++) {
    name.str("");
    name << dataDir << "run" << run << "/gpsEvent" << run << ".root";
    gpsTree->Add(name.str().c_str());
  }

  Adu5Pat *pat = NULL;
  gpsTree->SetBranchAddress("pat",&pat);
  gpsTree->BuildIndex("eventNumber");
  cout << "loaded gps event files and built index" << endl;



  
  TH1D *hPhiDist = new TH1D("hPhiDist","Phi;root sum square angular separation;occupancy",
			    361,-180,180);
  TH1D *hThetaDist = new TH1D("hThetaDist","Theta;root sum square angular separation;occupancy",
			      361,-180,180);
  
  TGraph *gCloseEvs = new TGraph();
  gCloseEvs->SetName("gCloseEvs");
  gCloseEvs->SetTitle("gCloseEvs");

  
  TFile *outFile = TFile::Open("clusterer.root","recreate");
  double currPhi,currTheta,closestPhi,closestTheta,closestQuad,lat,lon,alt;
  int run,eventNumber,closestEv;
  TTree *outTree = new TTree("clusterTree","clusterTree");
  outTree->Branch("currPhi",&currPhi,"currPhi/D");
  outTree->Branch("currTheta",&currTheta,"currTheta/D");
  outTree->Branch("closestPhi",&closestPhi,"closestPhi/D");
  outTree->Branch("closestTheta",&closestTheta,"closestTheta/D");
  outTree->Branch("closestQuad",&closestQuad,"closestQuad/D");
  outTree->Branch("lat",&lat,"lat/D");
  outTree->Branch("lon",&lon,"lon/D");
  outTree->Branch("alt",&alt,"alt/D");
  outTree->Branch("run",&run,"run/I");
  outTree->Branch("eventNumber",&eventNumber,"eventNumber/I");
  outTree->Branch("closestEv",&closestEv,"closestEv/I");






  
  for (int entry=0; entry<lenEntries; entry++) {
    resultTree->GetEntry(entry);
    eventNumber = eventSummary->eventNumber;
    int run = eventSummary->run;
    
    gpsTree->GetEntry(gpsTree->GetEntryNumberWithBestIndex(eventNumber));

    UsefulAdu5Pat *usefulPat = new UsefulAdu5Pat(pat);

    closestPhi = 999;
    closestTheta = 999;
    closestQuad = addQuad(closestPhi,closestTheta);
    closestEv = -999;

    currPhi = eventSummary->peak[0][0].phi;
    currTheta = eventSummary->peak[0][0].theta;

    if (entry%1 == 0) cout << entry << "/" << lenEntries << " (" << eventNumber << ")" << endl;

    for (int entry2=0; entry2<lenEntries; entry2++) {
      if (entry == entry2) continue; // don't want to compare it to itself! :P
      resultTree->GetEntry(entry2);

      double otherPhi = -999;
      double otherTheta=-999;

      usefulPat->getThetaAndPhiWave(eventSummary->peak[0][0].longitude,
				    eventSummary->peak[0][0].latitude,
				    eventSummary->peak[0][0].altitude,
				    otherTheta,otherPhi);
  
      otherPhi *= (180./TMath::Pi());
      otherTheta *= (180./TMath::Pi());

      double phiDiff = currPhi - otherPhi;
      while (phiDiff > 180) phiDiff -= 180;
      while (phiDiff < -180) phiDiff += 180;
      double thetaDiff = currTheta - otherTheta;
      while (thetaDiff > 180) thetaDiff -= 180;
      while (thetaDiff < -180) thetaDiff += 180;

      //basically how "close" are they, which should take into account the errors for each
  
      double phiSigma = 1;
      double thetaSigma = 1;
  

      double quadDiff = addQuad(phiDiff/phiSigma,thetaDiff/thetaSigma); 


      if (closestQuad > quadDiff ) { 
	//	cout << entry2 < " || " << closestQuad << " | " << currPhi << " - " << otherPhi << " = " << phiDiff << " " << currTheta << " - " << otherTheta << " = " << thetaDiff << " | " <<  quadDiff << endl;

	closestPhi = phiDiff;
	closestTheta = thetaDiff;
	closestQuad = addQuad(phiDiff,thetaDiff);
	closestEv = eventSummary->eventNumber;
      }

    }

    printf("closest: %d(%d) -> %d : %f %f\n",eventNumber,run,closestEv,closestPhi,closestTheta);
    //    cout << "closest: " << closestEv << ": " << closestPhi << " | " << closestTheta << endl;
    
    gCloseEvs->SetPoint(entry,eventNumber,closestEv);
    hPhiDist->Fill(closestPhi);
    hThetaDist->Fill(closestTheta);
    
    outTree->Fill();

    delete usefulPat;

  }

  outFile->cd();
  outTree->Write();
  gCloseEvs->Write();
  hPhiDist->Write();
  hThetaDist->Write();
  outFile->Close();
  

}
