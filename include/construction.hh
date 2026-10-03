#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4Tubs.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4GenericMessenger.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"

//#include "detector.hh"
class G4LogicalVolume;
class G4Material;
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:

    DetectorConstruction();
   ~DetectorConstruction();

    G4LogicalVolume *GetScoringVolume() const {return fScoringVolume_1;}
    G4LogicalVolume *GetScoringVolume_2() const {return fScoringVolume_2;}
   

 
    virtual G4VPhysicalVolume* Construct();
    void ConstructScorer_back(G4double Pos_PPAC_1);
    void ConstructScorer_front(G4double Pos_PPAC_2);
    void waterwall(G4double position1);   
    void Rock( G4double position);

private:
     G4Box  *sBox,*FoShield, *TShield,*fShield,*sShield,*sScore,*solidDetector;
     G4Box *fShield1;
     G4Tubs *solidScintillator;
     G4VPhysicalVolume *ffPShield, *FoPShield,*fPShield,*TPShield;
     G4VPhysicalVolume *fPShield1;
     G4LogicalVolume   *fLBox,*ffLShield, *FoLShield,*fLShield,*TLShield,*fLScore, *logicScintillator;
     G4LogicalVolume   *fLShield1;
     G4LogicalVolume*   logicDetector;
     G4LogicalVolume   *fScoringVolume_1;
     G4LogicalVolume   *fScoringVolume_2;
   
     G4double       mat_thickness;

     G4VPhysicalVolume *fPBox, *physScintillator;

     G4double           ShThick,ThThick,FohThick;
     G4double            fhThick,fhThick1;

     G4double           fBoxSize;
     G4Material*        fMaterial;     
     G4Material *Air, *Aerogel,  *polyethylene, *NaI, *CF4 ;
     G4Material *siliconMaterial,*rock, *water;
     G4Element  *Na, *I, *C,*N,*O,*F,*Al,*Si,*Fe;
     G4Material *Vacc;     

     void DefineMaterials();
     virtual void ConstructSDandField();
     G4GenericMessenger *fMessenger;

     // wall settings, can be changed from a macro BEFORE /run/initialize:
     //   /wall/rockThickness 0.3 m
     //   /wall/waterThickness 0.5 m
     //   /wall/withWater true
     G4double fRockThickness;
     G4double fWaterThickness;
     G4bool   fWithWater;
     G4bool isPPAC, isScintillator;   
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


#endif

