#include <iostream>

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "Randomize.hh"

#include "construction.hh"
#include "physics.hh"
#include "action.hh"

int main(int argc, char** argv)
{
    // If no macro is given (./sim), open the graphics window
    G4UIExecutive *ui = nullptr;
    if (argc == 1)
    {
       ui = new G4UIExecutive(argc, argv);
    }

    // Run manager + the three mandatory pieces
    G4RunManager *runManager = new G4RunManager();
    runManager->SetUserInitialization(new DetectorConstruction());
    runManager->SetUserInitialization(new PhysicsList());
    runManager->SetUserInitialization(new MyActionInitialization());

    // Visualization
    G4VisManager *visManager = new G4VisExecutive();
    visManager->Initialize();

    G4UImanager *UImanager = G4UImanager::GetUIpointer();

    if (ui)
    {
       // Window mode:  ./sim          -> only show the geometry (vis.mac)
       UImanager->ApplyCommand("/control/execute vis.mac");
       ui->SessionStart();
       delete ui;
    }
    else
    {
       // Batch mode:   ./sim run.mac  -> no graphics, full run
       G4String command  = "/control/execute ";   // the space at the end matters!
       G4String fileName = argv[1];
       UImanager->ApplyCommand(command + fileName);
    }

    // Job termination
    delete visManager;
    delete runManager;
    return 0;
}
