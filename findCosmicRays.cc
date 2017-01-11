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

  if (argc != 3) {
    cout << "Usage: " << argv[0] << " [run] [output base filename]" << endl;
    return -1;
  }
  else {
    runNum = atoi(argv[1]);
    outFileName = argv[2];
    cout << "Hello!  Let us do some physics mate" << endl;
    cout << "Using run " << runNum << " and outfile " << outFileName << endl;
  }




  stringstream name;
  //okay lets start by grabbing the header files
  TChain *headTree = new TChain("headTree","headTree");  
  TChain *eventTree = new TChain("eventTree","eventTree");
  //I also need the gps files to know where the heck ANITA is!
  TChain *patTree = new TChain("adu5PatTree","adu5PatTree");  

  char* dataDir = getenv("ANITA3_DATA");
  name.str("");
  name << dataDir << "run" << runNum << "/headFile" << runNum << ".root";
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

  //also make a Tree of event headers that pass cuts
  name.str("");
  name << outFileName << ".root";
  TFile *outFile = TFile::Open(name.str().c_str(),"recreate");
  TTree *outTree = new TTree("headTree","headTree");

  //Lets make the summary object that I can shove into the output tree
  AnitaEventSummary * eventSummary = new AnitaEventSummary; 

  outTree->Branch("eventSummary",&eventSummary);



  //**counters for cuts
  //global count for how many cuts an event fails
  int globalFailCut = 0;
  //selection (quality) cuts
  int rfTrigCut = 0;
  int waisPulserCut = 0;
  int mcmPulserCut = 0;
  int blastCut = 0;
  int surfSaturation = 0;
  
  
  name.str("");
  name << outFileName << "_filtOutFile.root";
  TFile *filterOutFile = TFile::Open(name.str().c_str(),"recreate");
 
  //**loop through entries
  numEntries=10000;
  for (int entry=0; entry<numEntries; entry++) {
    if (entry%1==0) {
      cout << entry << "/" << numEntries << "\r";
      fflush(stdout);
    }
    //get all the pointers set right
    headTree->GetEntry(entry);
    eventTree->GetEntry(entry);
    patTree->GetEntry(entry);


    //***I can do some quick cuts before I have to do much work
    //a) rf trigs only
    if ((head->trigType&0x0F) != 1){
      rfTrigCut++;
      globalFailCut++;
    }
    //d) payload blasts
    //  more than 15 channels withVpp > 400 counts
    //e) surf saturation
    //  the max is greater than 1500ADC for any point that isn't a clock (this is wrong but fast)
    int saturatedCounter = 0;
    int blastCounter = 0;
    for (int surf=0; surf<12; surf++) {
      for (int chan=0; chan<8; chan++) {
	int index = surf*9 + chan;
	int Vpp = event->xMax[index] - event->xMin[index];
	if ( Vpp > 1500 ) saturatedCounter++;
	if ( Vpp > 400  ) blastCounter++;
      }
    }
    if (saturatedCounter > 3) {
      surfSaturation++;
      globalFailCut++;
    }
    if (blastCounter > 15) {
      blastCut++;
      globalFailCut++;
    }


    //Then create the gps object (needed to do the wais cuts)
    UsefulAdu5Pat *usefulPat = new UsefulAdu5Pat(pat);


    //***More selection cuts!
    //b) Wais divide pulser cut
    double waisDist = usefulPat->getDistanceFromSource(AnitaLocations::LATITUDE_WAIS,
						       AnitaLocations::LONGITUDE_WAIS,
						       AnitaLocations::ALTITUDE_WAIS);
    if (waisDist < 1000) {
      double waisNs = usefulPat->getTriggerTimeNsFromSource(AnitaLocations::LATITUDE_WAIS,
							    AnitaLocations::LONGITUDE_WAIS,
							    AnitaLocations::ALTITUDE_WAIS);
      if ( abs(waisNs - head->triggerTimeNs) < 1000 ) {
	waisPulserCut++;
	globalFailCut++;
      }
    }

    //c) McM pulser cut (just if I'm close to mcmurdo for now...)
    double mcmDist = usefulPat->getDistanceFromSource(AnitaLocations::LATITUDE_LDB,
						      AnitaLocations::LONGITUDE_LDB,
						      AnitaLocations::ALTITUDE_LDB);
    if (mcmDist < 1000) {
      mcmPulserCut++;
      globalFailCut++;
    }
    

   

    //**Okay thats all the selection cuts, lets calibrate the waveform
    UsefulAnitaEvent *usefulEvent = new UsefulAnitaEvent(event,WaveCalType::kFull,head);

    //make a filter strategy I guess?

    FilterStrategy strategy(filterOutFile);
    strategy.addOperation(new UCorrelator::SineSubtractFilter(0.05, 0, 4)); 

    //2) then filter the event and get a FilteredAnitaEvent back
    FilteredAnitaEvent *filteredEvent = new FilteredAnitaEvent(usefulEvent, &strategy, pat, head,true);
    //and a configuration for the analysis
    UCorrelator::AnalysisConfig config; 
    //and an analyzer object
    UCorrelator::Analyzer analyzer(&config, true); ;
    //clear the eventSummary so that I can fill it up with the analyzer
    eventSummary->zeroInternals();
    //3) then analyze the filtered event!
    analyzer.analyze(filteredEvent, eventSummary); 

    //Lets figure out which was the trigger (H=0, V=1, also defaults to H)
    int whichTrig =  eventSummary->flags.isVPolTrigger;
    
    //From there we can get the peak theta and peak phi bins of the map
    double peakPhi = eventSummary->peak[whichTrig][0].phi;
    double peakTheta = eventSummary->peak[whichTrig][0].theta;

    //now I guess we have a bunch more info to make cuts on!
    
      //    UCorrelator::WaveformCombiner *combiner = new UCorrelator::WaveformCombiner();
      //    combiner->combine(peakPhi,peakTheta,filteredEvent);
      //    const AnalysisWaveform *coherent = combiner->getCoherent();

    outFile->cd();
    outTree->Fill();

    delete filteredEvent;
    delete usefulEvent;
    delete usefulPat;
    
  }

  outFile->cd();
  outTree->Write();
  outFile->Close();

  filterOutFile->Close();

  cout << "Physics complete!  See ya later buddy :)" << endl;

  return 1;
  
}


  
