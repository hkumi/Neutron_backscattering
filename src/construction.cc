#include "construction.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"
#include "CLHEP/Units/SystemOfUnits.h"
#include <algorithm>

// ====================== FIXED SETTINGS ======================
static const G4double kWallWidth      = 5.0*m;   // wide = "flat infinite wall"
static const G4double kScorerWidth    = 8.0*m;   // larger than the wall
static const G4double kScorerThick    = 1.0*mm;  // thin plane, just counts
static const G4double kBackScorerZ    = -20.*cm; // behind the source (source at -10 cm)
// Coordinate system: the FRONT face of the rock is at z = 0.
//
// Rock thickness, water thickness and water on/off are set from the macro
// (before /run/initialize), e.g.
//   /wall/rockThickness 0.3 m
//   /wall/withWater true
// ============================================================

DetectorConstruction::DetectorConstruction()
{
  // default values (used when the macro does not set them)
  fRockThickness  = 0.5*m;
  fWaterThickness = 0.5*m;
  fWithWater      = true;

  // macro commands: /wall/...
  fMessenger = new G4GenericMessenger(this, "/wall/", "Rock + water wall settings");

  fMessenger->DeclarePropertyWithUnit("rockThickness", "m", fRockThickness,
                                      "Thickness of the rock layer")
            .SetStates(G4State_PreInit);
  fMessenger->DeclarePropertyWithUnit("waterThickness", "m", fWaterThickness,
                                      "Thickness of the water layer")
            .SetStates(G4State_PreInit);
  fMessenger->DeclareProperty("withWater", fWithWater,
                              "true = water behind the rock, false = rock only")
            .SetStates(G4State_PreInit);

  DefineMaterials();
}

DetectorConstruction::~DetectorConstruction()
{
  delete fMessenger;
}

void DetectorConstruction::DefineMaterials()
{
  G4NistManager *nist = G4NistManager::Instance();
  G4int ncomponents, natoms;
  G4double massfraction;

  G4double Vdens = 1.e-25*g/cm3;
  G4double Vpres = 1.e-19*pascal;
  G4double Vtemp = 0.1*kelvin;

  G4double a, z;
  C  = nist->FindOrBuildElement("C");
  N  = new G4Element("Nitrogen","N",7.,14.007*g/mole);
  O  = new G4Element("Oxygen","O",8.,15.999*g/mole);
  F  = new G4Element("Fluorine","F",9.,18.998*g/mole);
  Al = new G4Element("Aluminium","Al",13.,26.982*g/mole);
  Si = new G4Element("Silicon","Si",z=14.,a=28.085*g/mole);
  Fe = new G4Element("Iron","Fe",z=26.,a=55.85*g/mole);

  // Water. The element NAME "TS_H_of_Water" switches on the thermal
  // scattering data (S(alpha,beta)) for hydrogen bound in water.
  G4Element* H_water = new G4Element("TS_H_of_Water","H",1.,1.0079*g/mole);
  water = new G4Material("Water", 1.0*g/cm3, ncomponents=2,
                         kStateLiquid, 293.15*kelvin);
  water->AddElement(H_water, natoms=2);
  water->AddElement(O, natoms=1);

  // Vacuum (world and scorers)
  Vacc = new G4Material("Galactic", z=1, a=1.01*g/mole, Vdens, kStateGas, Vtemp, Vpres);

  // Air (not used at the moment)
  Air = new G4Material("air", 1.290*mg/cm3, ncomponents=2, kStateGas, 293*kelvin, 1*atmosphere);
  Air->AddElement(N, massfraction=70.*perCent);
  Air->AddElement(O, massfraction=30.*perCent);

  // Polyethylene (not used at the moment)
  G4Element* Hpe = new G4Element("TS_H_of_Polyethylene","H",1,1.0079*g/mole);
  G4Element* Cpe = new G4Element("Carbon","C",6,12.01*g/mole);
  polyethylene = new G4Material("polyethylene", 0.93*g/cm3, ncomponents=2,
                                kStateSolid, 293*kelvin, 1*atmosphere);
  polyethylene->AddElement(Hpe, natoms=4);
  polyethylene->AddElement(Cpe, natoms=2);

  // Rock (dry). NOTE for thesis: 11.9% C stands in for the remaining
  // crustal elements (Ca, Na, Mg, K...). Carbon is a moderator, so either
  // replace it with the real elements or justify it and cite the source.
  rock = new G4Material("rock", 2.8*g/cm3, 5);
  rock->AddElement(O,  massfraction=46.1*perCent);
  rock->AddElement(Si, massfraction=28.2*perCent);
  rock->AddElement(Al, massfraction=8.2*perCent);
  rock->AddElement(Fe, massfraction=5.6*perCent);
  rock->AddElement(C,  massfraction=11.9*perCent);
}

// Backscatter scorer (fScoringVolume_1): thin vacuum plane behind the source
void DetectorConstruction::ConstructScorer_back(G4double Pos_PPAC_1)
{
  auto sScore_1 = new G4Box("sScore_1", kScorerWidth/2, kScorerWidth/2, kScorerThick/2);
  auto fLScore_1 = new G4LogicalVolume(sScore_1, Vacc, "fLScore_1");
  new G4PVPlacement(0, G4ThreeVector(0., 0., Pos_PPAC_1),
                    fLScore_1, "ScorerBack", fLBox, false, 0, true);
  fScoringVolume_1 = fLScore_1;
}

// Transmission scorer (fScoringVolume_2): thin vacuum plane behind the wall
void DetectorConstruction::ConstructScorer_front(G4double Pos_PPAC_2)
{
  auto sScore_2 = new G4Box("sScore_2", kScorerWidth/2, kScorerWidth/2, kScorerThick/2);
  auto fLScore_2 = new G4LogicalVolume(sScore_2, Vacc, "fLScore_2");
  new G4PVPlacement(0, G4ThreeVector(0., 0., Pos_PPAC_2),
                    fLScore_2, "ScorerTrans", fLBox, false, 0, true);
  fScoringVolume_2 = fLScore_2;
}

void DetectorConstruction::Rock(G4double position)
{
  G4Box* rockbox = new G4Box("rockbox", kWallWidth/2, kWallWidth/2, fRockThickness/2);
  G4LogicalVolume* rockVolume = new G4LogicalVolume(rockbox, rock, "Rock");
  new G4PVPlacement(0, G4ThreeVector(0., 0., position),
                    rockVolume, "Rock", fLBox, false, 0, true);

  G4VisAttributes* blue = new G4VisAttributes(G4Colour::Blue());
  blue->SetVisibility(true);
  blue->SetForceAuxEdgeVisible(true);
  rockVolume->SetVisAttributes(blue);
}

void DetectorConstruction::waterwall(G4double position1)
{
  G4Box* waterbox = new G4Box("waterbox", kWallWidth/2, kWallWidth/2, fWaterThickness/2);
  G4LogicalVolume* waterVolume = new G4LogicalVolume(waterbox, water, "Water");
  new G4PVPlacement(0, G4ThreeVector(0., 0., position1),
                    waterVolume, "Water", fLBox, false, 0, true);

  G4VisAttributes* red = new G4VisAttributes(G4Colour::Red());
  red->SetVisibility(true);
  red->SetForceAuxEdgeVisible(true);
  waterVolume->SetVisAttributes(red);
}

G4VPhysicalVolume *DetectorConstruction::Construct()
{
  fBoxSize = 10*m;

  sBox  = new G4Box("world", fBoxSize/2, fBoxSize/2, fBoxSize/2);
  fLBox = new G4LogicalVolume(sBox, Vacc, "World");
  fPBox = new G4PVPlacement(0, G4ThreeVector(), fLBox, "World", 0, false, 0);

  G4cout << "\n=== WALL: rock " << fRockThickness/cm << " cm"
         << (fWithWater ? " + water " : " (no water)");
  if (fWithWater) G4cout << fWaterThickness/cm << " cm";
  G4cout << " ===\n" << G4endl;

  // Rock: front face at z = 0
  Rock(fRockThickness/2);
  G4double zBackOfWall = fRockThickness;

  // Water: directly behind the rock (no gap)
  if (fWithWater) {
    waterwall(fRockThickness + fWaterThickness/2);
    zBackOfWall += fWaterThickness;
  }

  // Transmission scorer 1 cm behind the last layer
  ConstructScorer_front(zBackOfWall + 1.*cm);

  // Backscatter scorer behind the source (source is at z = -10 cm in run.mac)
  ConstructScorer_back(kBackScorerZ);

  return fPBox;
}

void DetectorConstruction::ConstructSDandField()
{
  // No sensitive detectors needed: scoring is done in the SteppingAction.
}
