#include "physics.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "NeutronHPphysics.hh"
#include "G4EmStandardPhysics.hh"

#include "G4BosonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4ShortLivedConstructor.hh"

PhysicsList::PhysicsList()
:G4VModularPhysicsList()
{
  SetVerboseLevel(1);

  // extra units
  new G4UnitDefinition("millielectronVolt", "meV", "Energy", 1.e-3*eV);
  new G4UnitDefinition("mm2/g",  "mm2/g", "Surface/Mass", mm2/g);
  new G4UnitDefinition("um2/mg", "um2/mg","Surface/Mass", um*um/mg);

  // mandatory for G4NuclideTable
  const G4double meanLife = 1*nanosecond, halfLife = meanLife*std::log(2);
  G4NuclideTable::GetInstance()->SetThresholdOfHalfLife(halfLife);

  // Neutron physics (high precision, < 20 MeV, thermal scattering ON by default)
  RegisterPhysics(new NeutronHPphysics("neutronHP"));

  // Electromagnetic physics (recoil protons, gammas from capture, ...)
  RegisterPhysics(new G4EmStandardPhysics());

  // Optical physics removed: there is no light production in this setup.
}

PhysicsList::~PhysicsList()
{ }

void PhysicsList::ConstructParticle()
{
  G4BosonConstructor  pBosonConstructor;
  pBosonConstructor.ConstructParticle();

  G4LeptonConstructor pLeptonConstructor;
  pLeptonConstructor.ConstructParticle();

  G4MesonConstructor pMesonConstructor;
  pMesonConstructor.ConstructParticle();

  G4BaryonConstructor pBaryonConstructor;
  pBaryonConstructor.ConstructParticle();

  G4IonConstructor pIonConstructor;
  pIonConstructor.ConstructParticle();

  G4ShortLivedConstructor pShortLivedConstructor;
  pShortLivedConstructor.ConstructParticle();
}

void PhysicsList::SetCuts()
{
  // Geant4 default production cuts (0.7 mm)
  G4VUserPhysicsList::SetCuts();
}
