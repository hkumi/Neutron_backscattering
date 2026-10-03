#include "stepping.hh"
#include "G4Neutron.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include <cmath>

MySteppingAction::MySteppingAction(MyEventAction *eventAction)
{
  fEventAction = eventAction;
}

MySteppingAction::~MySteppingAction()
{}

void MySteppingAction::UserSteppingAction(const G4Step *step)
{
  G4Track* track = step->GetTrack();

  // 1) only neutrons
  if (track->GetDefinition() != G4Neutron::Definition()) return;

  // 2) only at the moment the neutron ENTERS a new volume
  G4StepPoint* post = step->GetPostStepPoint();
  if (post->GetStepStatus() != fGeomBoundary) return;
  G4VPhysicalVolume* nextPV = post->GetPhysicalVolume();
  if (!nextPV) return;                       // leaving the world
  G4LogicalVolume* nextLV = nextPV->GetLogicalVolume();

  const auto* det = static_cast<const DetectorConstruction*>(
      G4RunManager::GetRunManager()->GetUserDetectorConstruction());
  G4LogicalVolume* backScorer  = det->GetScoringVolume();    // fScoringVolume_1
  G4LogicalVolume* transScorer = det->GetScoringVolume_2();  // fScoringVolume_2

  // 3) which scorer, and is it going the right way?
  G4double dirZ = post->GetMomentumDirection().z();
  G4int id;                                   // 0 = backscattered, 1 = transmitted
  if      (nextLV == backScorer  && dirZ < 0.) id = 0;   // coming back toward source
  else if (nextLV == transScorer && dirZ > 0.) id = 1;   // went through the wall
  else return;

  // 4) record (energy at the boundary = energy when it arrives)
  G4double E = post->GetKineticEnergy();
  G4int evt  = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

  G4AnalysisManager *man = G4AnalysisManager::Instance();
  man->FillH1(id, E/MeV);                       // energy in MeV

  man->FillNtupleDColumn(id, 0, E/MeV);
  man->FillNtupleDColumn(id, 1, std::fabs(dirZ));              // cos(angle to wall normal)
  man->FillNtupleDColumn(id, 2, post->GetPosition().x()/cm);
  man->FillNtupleDColumn(id, 3, post->GetPosition().y()/cm);
  man->FillNtupleDColumn(id, 4, track->GetTrackLength()/m);
  man->FillNtupleDColumn(id, 5, post->GetGlobalTime()/ns);
  man->FillNtupleIColumn(id, 6, evt);
  man->FillNtupleIColumn(id, 7, track->GetTrackID());
  man->FillNtupleIColumn(id, 8, track->GetParentID());       // 0 = primary neutron
  man->AddNtupleRow(id);

  // 5) stop it here so it can never be counted twice
  track->SetTrackStatus(fStopAndKill);
}
