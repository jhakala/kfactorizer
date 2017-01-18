//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Oct 31 09:34:08 2016 by ROOT version 6.07/07
// from TTree ktree/ktree
// found on file: /Users/johakala/kfactorytree.root
//////////////////////////////////////////////////////////

#ifndef kfactorizer_h
#define kfactorizer_h

#include <iostream>

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TProfile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"
#include <utility>
#include <math.h>

class kfactorizer {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain
   short up;
   short down;

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   vector<float>   *kenergies;
   vector<float>   *ktimes;
   vector<short>   *kietas;
   vector<short>   *kiphis;
   vector<short>   *kdepths;

   // List of branches
   TBranch        *b_kenergies;   //!
   TBranch        *b_ktimes;   //!
   TBranch        *b_kietas;   //!
   TBranch        *b_kiphis;   //!
   TBranch        *b_kdepths;   //!

   kfactorizer(TTree *tree=0);
   virtual ~kfactorizer();
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(string outputfilename, float energyCutValue, int quitAfter = -1, int reportEvery = 10000);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
   float getDeta(short ieta);
   float getAbsEta(short ieta);
   float getEt(float kenergy, short ieta);

   float edges[30] = {
     0.0000,
     0.0870,
     0.1740,
     0.2610,
     0.3480,
     0.4350,
     0.5220,
     0.6090,
     0.6950,
     0.7830,
     0.8700,
     0.9570,
     1.0440,
     1.1310,
     1.2180,
     1.3050,
     1.3920,
     1.4790,
     1.5660,
     1.6530,
     1.7400,
     1.8300,
     1.9300,
     2.0430,
     2.1720,
     2.3220,
     2.5000,
     2.6500,
     2.8680,
     3.0000
   };
};

#endif

#ifdef kfactorizer_cxx
kfactorizer::kfactorizer(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("/Users/johakala/kfactorytree.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("/Users/johakala/kfactorytree.root");
      }
      TDirectory * dir = (TDirectory*)f->Get("/Users/johakala/kfactorytree.root:/kfactorize");
      dir->GetObject("ktree",tree);

   }
   Init(tree);
}

kfactorizer::~kfactorizer()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t kfactorizer::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t kfactorizer::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void kfactorizer::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   kenergies = 0;
   ktimes = 0;
   kietas = 0;
   kiphis = 0;
   kdepths = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("kenergies", &kenergies, &b_kenergies);
   fChain->SetBranchAddress("ktimes", &ktimes, &b_ktimes);
   fChain->SetBranchAddress("kietas", &kietas, &b_kietas);
   fChain->SetBranchAddress("kiphis", &kiphis, &b_kiphis);
   fChain->SetBranchAddress("kdepths", &kdepths, &b_kdepths);
   Notify();
}

Bool_t kfactorizer::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void kfactorizer::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}

float kfactorizer::getDeta(short ieta) { 
  return edges[std::abs(ieta)]-edges[std::abs(ieta)-1];
}
float kfactorizer::getAbsEta(short ieta) { 
  return (edges[std::abs(ieta)]+edges[std::abs(ieta)-1])/2.;
}
float kfactorizer::getEt(float kenergy, short ieta) { 
  return kenergy / cosh(getAbsEta(ieta));
}
#endif // #ifdef kfactorizer_cxx
