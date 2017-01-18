#define kfactorizer_cxx
#include "kfactorizer.h"

void kfactorizer::Loop(string outputfilename, float energyCutValue, int quitAfter, int reportEvery) {
  bool debug                  = false ;
  bool doDepthOneHist         = false ;
  bool doOccupancy            = false ;
  bool doAvgEnergy            = false ;
  bool doSingleEventHists     = false ;
  bool dodETdEta              = true  ;

  if (fChain == 0) return;

  Long64_t nentries = fChain->GetEntriesFast();

  Long64_t nbytes = 0, nb = 0;
  TFile* outfile = new TFile(outputfilename.c_str(), "RECREATE");
  outfile->cd();

  // A 2d histogram to show some rechits in ieta-iphi space
  TH2D* etaPhiHistD1 = new TH2D("etaPhiHistD1", "etaPhiHistD1", 65, -32.5, 32.5, 75, 0, 75);

  // Some histograms of rechit occupancy and energy
  TH1I* occupancyVsEta = new TH1I("occupancyVsEta" ,  "occupancyVsEta" ,  65  , -32.5 , 32.5 );
  TH1F* avgEnergyVsEta = new TH1F("avgEnergyVsEta" ,  "avgEnergyVsEta" ,  65  , -32.5 , 32.5 );

  // Some profiles of rechit energies
  TProfile* energyProfVsEta = new TProfile("energyProfVsEta" , "energyProfVsEta" , 65, -32.5, 32.5);
  TProfile* dETdEta         = new TProfile("dETdEta"         , "dETdEta"         , 65, -32.5, 32.5);

  float eventArray[65][75]; 
  float ratio;
  int nRechits[65];
  float totalEnergy[65];
  int nRechitsInThisPhiRing;
  float N=0;
  float ringEtSum;
  short binOffset = 32;

  std::fill( std::begin( nRechits ), std::end( nRechits ), 0 );
  std::fill( std::begin( totalEnergy ), std::end( totalEnergy ), 0. );
  
  cout << "kfactorizer is beginning to process " << fChain->GetName() << endl;
  if (quitAfter >= 0) cout << "  only processing the first " << quitAfter << " entries." << endl;
  for (Long64_t jentry=0; jentry<nentries;++jentry) {
    if (jentry % reportEvery == 0) cout << "processing entry " << jentry << endl;
    for (short iStrip=0; iStrip<65; ++iStrip) {
      std::fill( std::begin( eventArray[iStrip] ), std::end( eventArray[iStrip] ), 0. );
    }
    ratio=0;
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    // if (Cut(ientry) < 0) continue;

    if (debug) cout << "In entry number " << jentry << ":" << endl;
    for(size_t iRechit = 0; iRechit != kenergies->size(); ++iRechit) {
      if (debug) {
        cout << "  rechit number " << iRechit << ":" << endl;
        cout << "    energy: " << kenergies->at(iRechit) << endl; 
        cout << "    time:   " << ktimes->at(iRechit)    << endl; 
        cout << "    ieta:   " << kietas->at(iRechit)    << endl; 
        cout << "    iphi:   " << kiphis->at(iRechit)    << endl; 
        cout << "    depth:  " << kdepths->at(iRechit)   << endl; 
      }
      if (doDepthOneHist) {
        if (kdepths->at(iRechit)==1) {
          etaPhiHistD1->SetBinContent(
              etaPhiHistD1->GetXaxis()->FindBin(kietas->at(iRechit)),
              etaPhiHistD1->GetYaxis()->FindBin(kiphis->at(iRechit)),
              kenergies->at(iRechit)
              );
        }
      }
      eventArray[kietas->at(iRechit)+binOffset][kiphis->at(iRechit)] += kenergies->at(iRechit);
    }
    //cout << endl;
    if (doSingleEventHists) {
      TH2D* tmpHist = new TH2D("tmpHist", "tmpHist", 65, -32, 32, 75, 0, 75);
      for (short x=0; x<65; ++x){
        for (short y=0; y<75; ++y){
          tmpHist->SetBinContent(x, y, eventArray[x][y]);
        }
      }
      tmpHist->Write();
      delete tmpHist;
    }
    for (short x=0; x<65; ++x){
      ringEtSum = 0;
      nRechitsInThisPhiRing = 0;
      for (short y=0; y<75; ++y){
        if (eventArray[x][y]>energyCutValue) {
          ringEtSum += getEt(eventArray[x][y], x-binOffset); 
        }
      }
      if (doOccupancy) {
        occupancyVsEta->SetBinContent(x+1, occupancyVsEta->GetBinContent(x+1)+nRechitsInThisPhiRing);
        if (debug && nRechitsInThisPhiRing>0) cout << " filled " << nRechitsInThisPhiRing 
                                                   << " into eta bin  " << x-binOffset << endl;
      } 
      if (dodETdEta) { 
        dETdEta->Fill(x-binOffset, ringEtSum / getDeta(x-binOffset));
      }

    }
    if (jentry == quitAfter) break;
  }
  if (doAvgEnergy) {
    for (short iBin=0; iBin<65; ++iBin) {
      if (nRechits[iBin]>0) {
        N = (float)nRechits[iBin];
        if (debug) {
          cout << "totalEnergy[" << iBin << "]/((float)nRechits[" << iBin << "]) = " << totalEnergy[iBin]/(float)N << endl;
        }
      }
      if (N>0){
        avgEnergyVsEta->SetBinContent(iBin+1, totalEnergy[iBin]/N);
      }
    }
    avgEnergyVsEta->Write();
    energyProfVsEta->Write();
  }
  if (doDepthOneHist){
    etaPhiHistD1->Write();
  }
  if (doOccupancy) { 
    occupancyVsEta->Write();
  }
  if (dodETdEta) { 
    dETdEta->Write();
  }
  outfile->Close();
  cout << "Completed output file is " << outfile->GetName() << endl;
}
