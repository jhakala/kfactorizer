#define kfactorizer_cxx
#include "kfactorizer.h"

void kfactorizer::Loop(string outputfilename, float energyCutValue, int quitAfter, int reportEvery) {
  bool debug                  = false ;
  bool doDepthOneHist         = false ;
  bool doClusteringComparison = true ;
  bool doOccupancy            = false ;
  bool doAvgEnergy            = false ;
  bool doSingleEventHists     = false ;
  bool dodETdEta              = true  ;

  //    fChain->SetBranchStatus("*",0);  // disable all branches
  //    fChain->SetBranchStatus("branchname",1);  // activate branchname
  if (fChain == 0) return;

  Long64_t nentries = fChain->GetEntriesFast();

  Long64_t nbytes = 0, nb = 0;
  TFile* outfile = new TFile(outputfilename.c_str(), "RECREATE");
  outfile->cd();
  // make a 2d hist to show some rechit clusters

  TH2D* etaPhiHistD1 = new TH2D("etaPhiHistD1", "etaPhiHistD1", 65, -32.5, 32.5, 75, 0, 75);

  TH1D* ratiosOneCell  = new TH1D("ratiosOneCell"  ,  "ratiosOneCell"  ,  100 , 0   , 1  );
  TH1D* ratiosTwoByTwo = new TH1D("ratiosTwoByTwo" ,  "ratiosTwoByTwo" ,  100 , 0   , 1  );
  TH1I* occupancyVsEta = new TH1I("occupancyVsEta" ,  "occupancyVsEta" ,  65  , -32.5 , 32.5 );
  TH1F* avgEnergyVsEta = new TH1F("avgEnergyVsEta" ,  "avgEnergyVsEta" ,  65  , -32.5 , 32.5 );
  TProfile* energyProfVsEta = new TProfile("energyProfVsEta" , "energyProfVsEta" , 65, -32.5, 32.5);
  TProfile* dETdEta =         new TProfile("dETdEta"         , "dETdEta"         , 65, -32.5, 32.5);

  float eventArray[65][75]; 
  float ratio;
  float topLeftBox;
  float topRightBox;
  float bottomLeftBox;
  float bottomRightBox;
  float mostEnergeticBox;
  bool isTopRight;
  bool isTopLeft;
  bool isBottomRight;
  bool isBottomLeft;
  int nRechits[65];
  float totalEnergy[65];
  int nRechitsInThisPhiRing;
  float N=0;
  float ringEtSum;

  std::fill( std::begin( nRechits ), std::end( nRechits ), 0 );
  std::fill( std::begin( totalEnergy ), std::end( totalEnergy ), 0. );
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

    //cout << endl;
    //cout << "In entry number " << jentry << ":" << endl;
    for(size_t iRechit = 0; iRechit != kenergies->size(); ++iRechit) {
      //cout << "  rechit number " << iRechit << ":" << endl;
      //cout << "    energy: " << kenergies->at(iRechit) << endl; 
      //cout << "    time:   " << ktimes->at(iRechit)    << endl; 
      //cout << "    ieta:   " << kietas->at(iRechit)    << endl; 
      //cout << "    iphi:   " << kiphis->at(iRechit)    << endl; 
      //cout << "    depth:  " << kdepths->at(iRechit)   << endl; 
      if (doDepthOneHist) {
        if (kdepths->at(iRechit)==1) {
          etaPhiHistD1->SetBinContent(
              etaPhiHistD1->GetXaxis()->FindBin(kietas->at(iRechit)),
              etaPhiHistD1->GetYaxis()->FindBin(kiphis->at(iRechit)),
              kenergies->at(iRechit)
              );
        }
      }
      eventArray[kietas->at(iRechit)+32][kiphis->at(iRechit)] += kenergies->at(iRechit);
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
          ringEtSum += getEt(eventArray[x][y], x-32); 
          if (doClusteringComparison) {
            compareClustering(x, y, eventArray, ratio, ratiosOneCell, ratiosTwoByTwo, energyProfVsEta,
                topLeftBox, topRightBox, bottomLeftBox, bottomRightBox, mostEnergeticBox, 
                isTopRight, isTopLeft, isBottomRight, isBottomLeft,
                nRechits, totalEnergy, nRechitsInThisPhiRing );
          }
        }
      }
      if (doOccupancy) {
        occupancyVsEta->SetBinContent(x+1, occupancyVsEta->GetBinContent(x+1)+nRechitsInThisPhiRing);
        if (debug && nRechitsInThisPhiRing>0) cout << " filled " << nRechitsInThisPhiRing << " into eta bin  " << x-32 << endl;
      } 
      if (dodETdEta) { 
        dETdEta->Fill(x-32, ringEtSum / getDeta(x-32));
      }

    }
    if (jentry == quitAfter) break;
  }
  if (doAvgEnergy) {
    for (short iBin=0; iBin<65; ++iBin) {
      if (nRechits[iBin]>0) {
        N = (float)nRechits[iBin];
        if (debug) {
          //cout << "totalEnergy[" << iBin << "] = " << totalEnergy[iBin] << endl;
          //cout << "nRechits[" << iBin << "] = " << nRechits[iBin] << endl;
          //cout << "(float)nRechits[" << iBin << "] = " << (float)nRechits[iBin] << endl;
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
  if (doClusteringComparison) {
    ratiosOneCell->Write();
    ratiosTwoByTwo->Write();
  }
  if (doOccupancy) { 
    occupancyVsEta->Write();
  }
  if (dodETdEta) { 
    dETdEta->Write();
  }
  outfile->Close();
}

void kfactorizer::compareClustering(short x, short y, float eventArray[65][75], float ratio, TH1D* ratiosOneCell, TH1D* ratiosTwoByTwo, TProfile* energyProfVsEta,
                                    float topLeftBox, float topRightBox, float bottomLeftBox, float bottomRightBox, float mostEnergeticBox, 
                                    bool isTopRight, bool isTopLeft, bool isBottomRight, bool isBottomLeft,
                                    int nRechits[65], float totalEnergy[65], int nRechitsInThisPhiRing, bool debug                                    ) {
  ratio=    eventArray[x][y] / 
    ( eventArray[x-2][y+2] + eventArray[x-1][y+2] + eventArray[x][y+2] + eventArray[x+1][y+2] + eventArray[x+2][y+2]
      + eventArray[x-2][y+1] + eventArray[x-1][y+1] + eventArray[x][y+1] + eventArray[x+1][y+1] + eventArray[x+2][y+1]
      + eventArray[x-2][y]   + eventArray[x-1][y]   + eventArray[x][y]   + eventArray[x+1][y]   + eventArray[x+2][y]
      + eventArray[x-2][y-1] + eventArray[x-1][y-1] + eventArray[x][y-1] + eventArray[x+1][y-1] + eventArray[x+2][y-1]
      + eventArray[x-2][y-2] + eventArray[x-1][y-2] + eventArray[x][y-2] + eventArray[x+1][y-2] + eventArray[x+2][y-2] );
  ratiosOneCell->Fill(ratio);     

  std::vector<std::pair<short, short>> neighbors = GetNeighbors(x-32, y);
  topLeftBox       = eventArray[x][y];
  topRightBox      = eventArray[x][y];
  bottomLeftBox    = eventArray[x][y];
  bottomRightBox   = eventArray[x][y];
  isTopLeft     =  true;
  isTopRight    = false;
  isBottomLeft  = false;
  isBottomRight = false;
  double maxNeighbor = 0;
  if(debug) std::cout << "Checking neighbors for channel with ieta=" << x-32 << ", iphi=" << y << std::endl;
  for(unsigned short iNeighbor = 0; iNeighbor < neighbors.size(); ++iNeighbor) {
    if (debug) {
      std::cout << "neighbor " << iNeighbor << " has (ieta, iphi) = (" << neighbors[iNeighbor].first << ", " << neighbors[iNeighbor].second << ")" << std::endl;
      std::cout << "neighbor " << iNeighbor << " has energy = " << eventArray[neighbors[iNeighbor].first][neighbors[iNeighbor].second] << std::endl;
    }
    if(neighbors[iNeighbor].first+32 <= x && neighbors[iNeighbor].second >= y) topLeftBox      += eventArray[neighbors[iNeighbor].first+32][neighbors[iNeighbor].second];
    if(neighbors[iNeighbor].first+32 >= x && neighbors[iNeighbor].second >= y) topRightBox     += eventArray[neighbors[iNeighbor].first+32][neighbors[iNeighbor].second];
    if(neighbors[iNeighbor].first+32 <= x && neighbors[iNeighbor].second <= y) bottomLeftBox   += eventArray[neighbors[iNeighbor].first+32][neighbors[iNeighbor].second];
    if(neighbors[iNeighbor].first+32 >= x && neighbors[iNeighbor].second <= y) bottomRightBox  += eventArray[neighbors[iNeighbor].first+32][neighbors[iNeighbor].second];
    if (eventArray[neighbors[iNeighbor].first+32][neighbors[iNeighbor].second] > maxNeighbor) maxNeighbor = eventArray[neighbors[iNeighbor].first+32][neighbors[iNeighbor].second];
  }
  if (debug) {
    std::cout << " TL=" << topLeftBox << ", TR=" << topRightBox << ", BL=" << bottomLeftBox << ", BR=" << bottomRightBox << std::endl;
    std::cout << " dEta for this channel is" << getDeta( x-32 ) << std::endl;
  }
  mostEnergeticBox = topLeftBox;
  if (topRightBox > mostEnergeticBox) { 
    isTopLeft=false; isTopRight=true;
    mostEnergeticBox = topRightBox;
  }

  if (bottomLeftBox > mostEnergeticBox) { 
    isTopLeft=false; isTopRight=false; isBottomLeft=true;
    mostEnergeticBox = bottomLeftBox;
  }

  if (bottomRightBox > mostEnergeticBox) { 
    isTopLeft=false; isTopRight=false; isBottomLeft=false; isBottomRight=true;
    mostEnergeticBox = bottomRightBox;
  }
  if ( eventArray[x][y] > maxNeighbor ) {
    totalEnergy[x]= totalEnergy[x] + mostEnergeticBox;
    if(debug) cout << "totalEnergy[" << x << "] = " << totalEnergy[x] << endl;
    nRechits[x] = nRechits[x]+1;
    energyProfVsEta->Fill(x-32, mostEnergeticBox);
    if (debug) cout << "filled " << mostEnergeticBox << " into the profile at eta = " << x-32 << endl;
    if (mostEnergeticBox > 40 && mostEnergeticBox < 60) {
      ++nRechitsInThisPhiRing;
      if (isTopLeft)     {eventArray[x-1][y+1]=0; eventArray[x][y+1]=0; eventArray[x-1][y]=0; eventArray[x][y]=0;}
      if (isTopRight)    {eventArray[x][y+1]=0; eventArray[x+1][y+1]=0; eventArray[x][y]=0; eventArray[x+1][y]=0;}
      if (isBottomLeft)  {eventArray[x-1][y]=0; eventArray[x][y]=0; eventArray[x-1][y-1]=0; eventArray[x][y-1]=0;}
      if (isBottomRight) {eventArray[x][y]=0; eventArray[x+1][y]=0; eventArray[x][y-1]=0; eventArray[x+1][y-1]=0;}
      if (debug) cout << " phi " << y << " had a rechit with energy in box = " << mostEnergeticBox << endl;
    }
  }


  ratio=  mostEnergeticBox / 
    ( eventArray[x-2][y+2] + eventArray[x-1][y+2] + eventArray[x][y+2] + eventArray[x+1][y+2] + eventArray[x+2][y+2]
      + eventArray[x-2][y+1] + eventArray[x-1][y+1] + eventArray[x][y+1] + eventArray[x+1][y+1] + eventArray[x+2][y+1]
      + eventArray[x-2][y]   + eventArray[x-1][y]   + eventArray[x][y]   + eventArray[x+1][y]   + eventArray[x+2][y]
      + eventArray[x-2][y-1] + eventArray[x-1][y-1] + eventArray[x][y-1] + eventArray[x+1][y-1] + eventArray[x+2][y-1]
      + eventArray[x-2][y-2] + eventArray[x-1][y-2] + eventArray[x][y-2] + eventArray[x+1][y-2] + eventArray[x+2][y-2] );
  ratiosTwoByTwo->Fill(ratio);
}
