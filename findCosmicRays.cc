#include <iostream>
#include <vector>
#include <math.h>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <random>
//root
#include "TTree.h"
#include "TChain.h"
#include "TFile.h"
#include "TH2.h"
#include "TH2D.h"
#include "TEllipse.h" 
#include "TMarker.h" 
#include "TStyle.h" 
#include "TCanvas.h"
//anita
#include "RawAnitaEvent.h"
#include "RawAnitaHeader.h"
#include "UsefulAdu5Pat.h"
#include "Adu5Pat.h"
#include "CalibratedAnitaEvent.h"
#include "UsefulAnitaEvent.h"
//cosmin's stuff
#include "AnitaEventSummary.h"
#include "AnalysisConfig.h" 
#include "UCFilters.h" 
#include "PeakFinder.h" 
#include "FilterStrategy.h" 
#include "Correlator.h" 
#include "Analyzer.h" 
#include "WaveformCombiner.h"

using namespace std;

/*

  Ben Rotter - January 2017 - University of Hawaii at Manoa

  
  I need to find the cosmic rays in the ANITA3 data set!

  This shouldn't be complicated.  I'll just recreate the cuts that Abby made to her data set.

  1) Defining sample:
  a) RF triggers only
  b) WAIS pulser cut (distance and trigTimeNs)
  c) McM pulser cut  (distance and trigTimeNs)
  d) Payload Blasts  (>15 channels with Vpp > 400mV)
  e) Surf Saturated Events (Vpp > 1.5V in more than three channels)


  2) Then do CW filtering (sine wave subtraction maybe?)

  3) Then do interferometry and get coherantly summed waveform for peak
  a) Make a cut on interferometric map peak?

  4) Then do a correlation between that summed waveform and the template
  a) make a cut on that to get ~1000 events

  5) Clustering!

  That should leave me with a dozen entries!  Lets go!

  
  Things to remember:

  * This needs to be done quickly, so it needs to be able to be run on the cluster
       just do one run per core for now seems easiest
  * I need to store the number of entries cut in each step
  * I should be blind to polarity (basically just do a random  *=-1 to each to blind?

 */


int main(int argc, char** argv) {


  int runNum;
  string outFileName;
  int lenEntries = -1;

  if (argc==3) {
    runNum = atoi(argv[1]);
    outFileName = argv[2];
    cout << "Hello!  Let us do some physics mate" << endl;
    cout << "Using run " << runNum << " and outfile " << outFileName << endl;
  }
  else if (argc==4) {
    runNum = atoi(argv[1]);
    outFileName = argv[2];
    lenEntries = atoi(argv[3]);
    cout << "Hello!  Let us do some physics mate" << endl;
    cout << "Using run " << runNum << " and outfile " << outFileName << endl;
    cout << "Only doing " << lenEntries << " of the first entries" << endl;
  }
  else {
    cout << "Usage: " << argv[0] << " [run] [output base filename] [opt: num entries]" << endl;
    return -1;
  }



  stringstream name;
  //okay lets start by grabbing the header files
  TChain *headTree = new TChain("headTree","headTree");  
  TChain *eventTree = new TChain("eventTree","eventTree");
  //I also need the gps files to know where the heck ANITA is!
  TChain *patTree = new TChain("adu5PatTree","adu5PatTree");  

  char* dataDir = getenv("ANITA3_DATA");
  name.str("");
  //  name << dataDir << "run" << runNum << "/headFile" << runNum << ".root";
  name << dataDir << "run" << runNum << "/decimatedHeadFile" << runNum << ".root";
  headTree->Add(name.str().c_str());
  
  name.str("");
  name << dataDir << "run" << runNum << "/eventFile" << runNum << ".root";
  eventTree->Add(name.str().c_str());
  
  name.str("");
  name << dataDir << "run" << runNum << "/gpsEvent" << runNum << ".root";
  patTree->Add(name.str().c_str());

  RawAnitaHeader *head = NULL;
  headTree->SetBranchAddress("header",&head);
  
  RawAnitaEvent *event = NULL;
  eventTree->SetBranchAddress("event",&event);

  Adu5Pat *pat = NULL;
  patTree->SetBranchAddress("pat",&pat);

  int numEntries = headTree->GetEntries();

  cout << "I found " << numEntries << " header entries" << endl;
  cout << "I found " << eventTree->GetEntries() << " event entries" << endl;
  cout << "I found " << patTree->GetEntries() << " gps entries" << endl;

  //using the decimated data, so I need to grab the correct entries!
  eventTree->BuildIndex("eventNumber");
  patTree->BuildIndex("eventNumber");
  

  //also make a Tree of event headers that pass cuts
  name.str("");
  name << outFileName << ".root";
  TFile *outFile = TFile::Open(name.str().c_str(),"recreate");
  outFile->cd();
  TTree *outTree = new TTree("headTree","headTree");

  //Lets make the summary object that I can shove into the output tree
  AnitaEventSummary * eventSummary = new AnitaEventSummary; 
  outTree->Branch("eventSummary",&eventSummary);
  


  //Make a filter strategy
  //  with a debug file
  //  name.str("");
  //  name << outFileName << "_filtOutFile.root";
  //  TFile *filterOutFile = TFile::Open(name.str().c_str(),"recreate"); 
  //  FilterStrategy strategy(filterOutFile);
  //  without a debug file
  FilterStrategy strategy;
  //Add the actual Filters
  //  with the sine subtract alghorithm
  FilterOperation *sineSub = new UCorrelator::SineSubtractFilter(0.05, 0, 4);
  strategy.addOperation(sineSub);
  //  with abby's list of filtering
  //  UCorrelator::applyAbbysFilterStrategy(&strategy);


  //and a configuration for the analysis
  UCorrelator::AnalysisConfig config; 
  //set the response to my "single" response
  config.response_option = UCorrelator::AnalysisConfig::ResponseOption_t::ResponseSingleBRotter;
  //and create an analyzer object (need to create it every time I guess)
  UCorrelator::Analyzer *analyzer = new UCorrelator::Analyzer(&config); ;
  
  
  //**loop through entries
  //option to have less entries! (-1 is the default, so in case you don't specify)
  if (lenEntries == -1) lenEntries = numEntries;

  int entryIndex=0;

  for (int entry=0; entry<lenEntries; entry++) {
    //    if (entry%1==0) {
      cout << entry << "/" << lenEntries << "(" << entryIndex << ")\r";
      fflush(stdout);
      //    }
    //get all the pointers set right
    headTree->GetEntry(entry);
    entryIndex = eventTree->GetEntryNumberWithBestIndex(head->eventNumber);
    eventTree->GetEntry(entryIndex);
    patTree->GetEntry(entryIndex);


    //1) Calibrate the waveform
    UsefulAnitaEvent *usefulEvent = new UsefulAnitaEvent(event,WaveCalType::kFull,head);

    //2) then filter the event and get a FilteredAnitaEvent back
    FilteredAnitaEvent *filteredEvent = new FilteredAnitaEvent(usefulEvent, &strategy, pat, head,true);

    //clear the eventSummary so that I can fill it up with the analyzer
    eventSummary->zeroInternals();
    //3) then analyze the filtered event!
    analyzer->analyze(filteredEvent, eventSummary); 

    //Lets figure out which was the trigger (H=0, V=1, also defaults to H)
    int whichTrig =  eventSummary->flags.isVPolTrigger;

    outFile->cd();
    outTree->Fill();

    delete filteredEvent;
    delete usefulEvent;
    //    analyzer->clearInteractiveMemory();
  }


  outFile->cd();
  outTree->Write();
  outFile->Close();

  delete eventSummary;

  //  cout << "Physics complete!  See ya later buddy :)" << endl;

  return 1;
  
}


  
