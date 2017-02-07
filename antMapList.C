#include "AnitaEventSummary.h"
#include "AntarcticaMapPlotter.h"

TGraph* loadBases(){


  cout<<"initializing base list..."<<endl;
  std::string nameString;
  double input_lat;
  double input_lon;
  char lat_sign; //A character: this one should always be 'S'.
  char lon_sign; //A character: either 'E' or 'W'
  int misc_field;
  int ctr=0;
  ifstream base_file;
  char filename[150];
  char *pEnv;

  TGraph *bases = new TGraph();
  bases->SetTitle("antarcticBases");
  bases->SetName("antarcticBases");

  pEnv = getenv("ANITA_ANALYSIS_AG_BASE");
  sprintf(filename,"%s/baseLocations/all_base_locations_new.txt",pEnv);
  base_file.open(filename);
  if (!base_file.is_open())
    {
      std::cerr<<"Error!  Could not open "
	       <<("all_base_locations_new.txt")<<" to read in base locations!\n";
    } //end if
  while (base_file >> nameString >> input_lat >> lat_sign >> input_lon >> lon_sign >> misc_field )
    {

      if (lat_sign == 'S')
	input_lat *= -1.;
      if (lon_sign == 'W')
	input_lon = (input_lon*-1.);//+360;
      
      cout << "lat:" << input_lat << " lon:" << input_lon << endl;
      bases->SetPoint(bases->GetN(),input_lat,input_lon);
      ctr++;
    }
  
  //now do pseudo bases

  TGraph *pbases = new TGraph();
  pbases->SetTitle("pseudoBases");
  pbases->SetName("pseudoBases");


  char filenamePseudo[150];
    sprintf(filenamePseudo,"%s/baseLocations/pseudoBases.txt",pEnv);
  //std::string filenamePseudo="/rh5stuff/64bit/src/anita/analysis/agoodhue/baseLocations/pseudoBases.txt";
  ifstream base_filePseudo;
  base_filePseudo.open(filenamePseudo);
  if (!base_filePseudo.is_open())
    {
      std::cerr<<"Error!  Could not open "
	       <<("pseudoBases.txt")<<" to read in base locations!\n";
    } //end if
  while (base_filePseudo >> nameString >> input_lat >> lat_sign >> input_lon >> lon_sign >> misc_field )
    {
      
      if (lat_sign == 'S')
	input_lat *= -1.;
      if (lon_sign == 'W')
	input_lon = (input_lon*-1.);//+360;
      pbases->SetPoint(pbases->GetN(),input_lon,input_lat);
      ctr++;
    }

  //now there is room here to add my own hard-coded bases or traverses like stephen did
  cout<<"done initializing base list"<<endl;

  return bases;

}



void antMapList() {

  
  TChain *resultTree = new TChain("headTree","headTree");

  TGraph* basesIn = loadBases();

  
  stringstream name;
  for (int run=130; run<440; run++) {
    name.str("");
    name << "/Volumes/ANITA3Data/bigAnalysisFiles/findCosmicRays/01.19.17_22h/" << run << ".root"; 
    resultTree->Add(name.str().c_str());
  }
  
  AnitaEventSummary *eventSummary = NULL;
  resultTree->SetBranchAddress("eventSummary",&eventSummary);
 

  AntarcticaMapPlotter *aMap = new AntarcticaMapPlotter("mapPlotter","mapPlotter",1000,1000);
  aMap->addTGraph("bases","bases");
  TGraph *bases = aMap->getCurrentTGraph();
  bases->SetMarkerStyle(4);
  bases->SetMarkerSize(0.5);
  bases->SetMarkerColor(kRed);
  double x,y;
  for (int i=0; i<basesIn->GetN(); i++) {
    aMap->getRelXYFromLatLong(basesIn->GetX()[i],basesIn->GetY()[i],x,y);
    bases->SetPoint(i,x,y);
  }


  aMap->addHistogram("pointMap","pointMap",1000,1000);



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
    double fisherVal = fisher0 + var0*fisher1 + var1*fisher2;

    if (eventSummary->flags.pulser !=0) {
      if (fisherVal > 0) pulsesPassing++;
      else pulsesFailing++;
    }

    if (eventSummary->flags.pulser == 0 && eventSummary->flags.isHPolTrigger == 1 && fisherVal > 0) {
      double lat = eventSummary->peak[0][0].latitude;
      double lon = eventSummary->peak[0][0].longitude;
      if (lat < -999 || lon < -999) continue;
      
      aMap->Fill(lat,lon);
      passing++;
    }
    
  }

  cout << "passing: " << passing << endl;
  cout << "pulsesPassing: " << pulsesPassing << " | failing: " << pulsesFailing << endl;

  TCanvas *c1 = new TCanvas("c1","c1",1000,800);
  c1->cd();
  aMap->DrawTGraph("p");
  c1->cd();
  aMap->DrawHist("colz");


  return;
}


