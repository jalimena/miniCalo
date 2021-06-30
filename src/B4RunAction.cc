//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: B4RunAction.cc 100946 2016-11-03 11:28:08Z gcosmo $
//
/// \file B4RunAction.cc
/// \brief Implementation of the B4RunAction class

#include "B4RunAction.hh"
#include "B4Analysis.hh"
#include "G4ParticleGun.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "B4PrimaryGeneratorAction.hh"
#include <stdlib.h>
#include "B4aEventAction.hh"

#include "G4ParticleGun.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B4RunAction::B4RunAction(B4PartGeneratorBase *gen, B4aEventAction* ev, G4String fname)
 : G4UserRunAction()
{ 
    generator_=gen;
    fname_=fname;
    eventact_=ev;

    if(!eventact_->checkConstruct())
        throw std::logic_error("first set all event action pointers B4RunAction::B4RunAction");

  // set printing event number per each event
  G4RunManager::GetRunManager()->SetPrintProgress(1);     

  // Create analysis manager
  // The choice of analysis technology is done via selectin of a namespace
  // in B4Analysis.hh
  auto analysisManager = G4AnalysisManager::Instance();
  G4cout << "Using " << analysisManager->GetType() << G4endl;

  // Create directories 
  //analysisManager->SetHistoDirectoryName("histograms");
  //analysisManager->SetNtupleDirectoryName("ntuple");
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetNtupleMerging(true);
    // Note: merging ntuples is available only with Root output

  // Book histograms, ntuple
  //
  

  // Creating ntuple
  //
  analysisManager->CreateNtuple("B4", "B4");


  analysisManager->CreateNtupleDColumn("in_px");
  analysisManager->CreateNtupleDColumn("in_py");
  analysisManager->CreateNtupleDColumn("in_pz");

  analysisManager->CreateNtupleDColumn("in_polx");
  analysisManager->CreateNtupleDColumn("in_poly");
  analysisManager->CreateNtupleDColumn("in_polz");

  analysisManager->CreateNtupleDColumn("out_px");
  analysisManager->CreateNtupleDColumn("out_py");
  analysisManager->CreateNtupleDColumn("out_pz");

  analysisManager->CreateNtupleDColumn("out_polx");
  analysisManager->CreateNtupleDColumn("out_poly");
  analysisManager->CreateNtupleDColumn("out_polz");



  analysisManager->FinishNtuple();

  G4cout << "run action initialised" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B4RunAction::~B4RunAction()
{
  delete G4AnalysisManager::Instance();  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B4RunAction::BeginOfRunAction(const G4Run* /*run*/)
{ 
  //inform the runManager to save random number seed
  //G4RunManager::GetRunManager()->SetRando//mNumberStore(true);
  
  // Get analysis manager
  auto analysisManager = G4AnalysisManager::Instance();

  eventact_->checkConstruct();
  // Open an output file
  //
  G4String fileName = fname_;
  analysisManager->OpenFile(fileName);
  eventact_->nevents_=0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B4RunAction::EndOfRunAction(const G4Run* /*run*/)
{
  // print histogram statistics
  //
    auto analysisManager = G4AnalysisManager::Instance();


  analysisManager->Write();
  analysisManager->CloseFile();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
