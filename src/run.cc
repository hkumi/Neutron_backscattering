#include "run.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include <cmath>

MyRunAction::MyRunAction()
{
  G4AnalysisManager *man = G4AnalysisManager::Instance();
  man->SetNtupleMerging(true);
  man->SetVerboseLevel(1);

  // Energy spectra in MeV, with log-spaced bins from 1e-10 MeV (0.1 meV) to 30 MeV
  // (draw in ROOT with a log x axis to see thermal and fast neutrons together)
  man->CreateH1("Eback",  "Backscattered neutrons;Energy (MeV);counts",
                115, 1.e-10, 30., "none", "none", "log");                        // H1 id 0
  man->CreateH1("Etrans", "Transmitted neutrons;Energy (MeV);counts",
                115, 1.e-10, 30., "none", "none", "log");                        // H1 id 1

  // One ntuple per scorer, all quantities of the same neutron in one row
  const char* names[2] = {"Backscattered", "Transmitted"};               // ntuple id 0, 1
  for (G4int i = 0; i < 2; ++i) {
    man->CreateNtuple(names[i], names[i]);
    man->CreateNtupleDColumn("Energy_MeV");
    man->CreateNtupleDColumn("CosTheta");
    man->CreateNtupleDColumn("x_cm");
    man->CreateNtupleDColumn("y_cm");
    man->CreateNtupleDColumn("TrackLength_m");
    man->CreateNtupleDColumn("Time_ns");
    man->CreateNtupleIColumn("EventID");
    man->CreateNtupleIColumn("TrackID");
    man->CreateNtupleIColumn("ParentID");
    man->FinishNtuple();
  }
}

MyRunAction::~MyRunAction()
{}

void MyRunAction::BeginOfRunAction(const G4Run* run)
{
  G4AnalysisManager *man = G4AnalysisManager::Instance();
  std::stringstream strRunID;
  strRunID << run->GetRunID();
  man->OpenFile("output" + strRunID.str() + ".root");
}

void MyRunAction::EndOfRunAction(const G4Run* run)
{
  G4AnalysisManager *man = G4AnalysisManager::Instance();
  man->Write();

  if (isMaster) {
    G4int N = run->GetNumberOfEvent();
    if (N > 0) {
      G4double nBack  = man->GetH1(0)->entries();
      G4double nTrans = man->GetH1(1)->entries();
      G4cout << "\n========== RESULTS ==========\n"
             << " Primary neutrons    : " << N << "\n"
             << " Backscattered       : " << nBack  << "  -> "
             << nBack/N  << " +/- " << std::sqrt(nBack)/N  << " per primary\n"
             << " Transmitted         : " << nTrans << "  -> "
             << nTrans/N << " +/- " << std::sqrt(nTrans)/N << " per primary\n"
             << "=============================\n" << G4endl;
    }
  }
  man->CloseFile();
}
