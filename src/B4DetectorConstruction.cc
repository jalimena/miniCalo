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
// $Id: B4DetectorConstruction.cc 101905 2016-12-07 11:34:39Z gunter $
// 
/// \file B4DetectorConstruction.cc
/// \brief Implementation of the B4DetectorConstruction class

#include "B4DetectorConstruction.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4RotationMatrix.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4AutoDelete.hh"
#include "G4UserLimits.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4PropagatorInField.hh"

#include "sensorContainer.h"

#include <cstdlib>

static G4double epsilon=0.0*mm;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ThreadLocal 
G4GlobalMagFieldMessenger* B4DetectorConstruction::fMagFieldMessenger = nullptr; 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

template<class T>
static G4String createString(const T& i){
	std::stringstream ss;
	ss << i;
	std::string number=ss.str();
	return number;
}


B4DetectorConstruction::B4DetectorConstruction()
: G4VUserDetectorConstruction(),
  fCheckOverlaps(true),
  defaultMaterial(0),
  absorberMaterial(0),
  gapMaterial(0)

{

}

G4VPhysicalVolume* B4DetectorConstruction::Construct()
{
	//DefineGeometry(homogenous_ecal_only);
	//DefineGeometry(hcal_only_irregular);
	DefineGeometry(homogenous_no_tracker);
	// Define materials
	DefineMaterials();

	// Define volumes
	return DefineVolumes();
}

void  B4DetectorConstruction::DefineGeometry(geometry geo){


    calorSizeXY  = 35.2*cm;

	if(geo == standard){
		calorThickness=2000*mm;

	    layerGranularity.clear();
	    layerSplitGranularity.clear();
		nofEELayers = 10;
		nofHB=15;
		for(int i=0;i<nofEELayers+nofHB;i++){
			G4double granularity=8;
			if(i>3)
				granularity=6;
			if(i>7){
				granularity=4;
			}
			if(i>15)
				granularity=2;
			layerGranularity.push_back(granularity);
			if(granularity<2)
				layerSplitGranularity.push_back(0);
			else
				layerSplitGranularity.push_back(2*granularity);
		}

		layerThicknessEE=15*mm;
		layerThicknessHB=(calorThickness-nofEELayers*layerThicknessEE)/(float)nofHB; //100*mm;

	}
	else if(geo == homogenous || geo == homogenous_ecal_only){
		if (geo == homogenous)
			calorThickness=2000*mm;
		else
			calorThickness=300*mm;

	    layerGranularity.clear();
	    layerSplitGranularity.clear();
		nofEELayers = 10;
		nofHB=20;
		for(int i=0;i<nofEELayers+nofHB;i++){
			G4double granularity=10;
			if(i==27)
				granularity=8;
			if(i==28)
				granularity=2;
			if(i==29)
				granularity=1;
			layerGranularity.push_back(granularity);

			layerSplitGranularity.push_back(-granularity/2);
		}

		layerThicknessEE=80*mm * calorThickness/2000*mm;
		layerThicknessHB=(calorThickness-nofEELayers*layerThicknessEE)/(float)nofHB; //100*mm;

	}
	else if(geo == ecal_only){
		calorThickness=200*mm;

		layerGranularity.clear();
		layerSplitGranularity.clear();
		nofEELayers = 10;
		nofHB=0;
		for(int i=0;i<nofEELayers+nofHB;i++){
			G4double granularity=10;

			layerGranularity.push_back(granularity);

			layerSplitGranularity.push_back(int(granularity/2));
		}

		layerThicknessEE=20*mm;
		layerThicknessHB=(calorThickness-nofEELayers*layerThicknessEE)/(float)nofHB; //100*mm;

	}
	else if(geo == hcal_only_irregular || geo == ecal_only_irregular){

		if(geo== hcal_only_irregular)
			calorThickness=2000*mm;
		else
			calorThickness=250*mm;

		layerGranularity.clear();
		layerSplitGranularity.clear();
		nofEELayers = 0;
		nofHB=20;
		for(int i=0;i<nofEELayers+nofHB;i++){
			G4double granularity=1;
			if(i==0)
				granularity=8;
			if(i==1)
				granularity=12;
			if(i>=2)
				granularity=16;
			if(i>=4)
				granularity=12;
			if(i>=8)
				granularity=8;
			if(i>=12)
				granularity=4;
			if(i>=14)
				granularity=2;

			G4double splitgranularity=granularity;

			if(granularity==16)
				splitgranularity=10;
			if(granularity==12)
				splitgranularity=8;


			layerGranularity.push_back(granularity);
			if(granularity<2)
				layerSplitGranularity.push_back(0);
			else
				layerSplitGranularity.push_back(splitgranularity);
		}

		layerThicknessEE=15*mm;
		layerThicknessHB=(calorThickness-nofEELayers*layerThicknessEE)/(float)nofHB; //100*mm;

	}
	else if(geo == homogenous_no_tracker){
	    calorThickness=2500*mm;//CHANGE

	    layerGranularity.clear();
	    layerSplitGranularity.clear();
	    nofEELayers = 60;
	    nofHB=0;
	    for(int i=0;i<nofEELayers+nofHB;i++){
	        G4double granularity=30;
	        layerGranularity.push_back(granularity);
	        layerSplitGranularity.push_back(-granularity/2);
	    }

	    layerThicknessEE=calorThickness / (float)nofEELayers;//26*8.903*mm ;//* calorThickness/2000*mm; //26 radiation lengths like CMS (23.2cm
	    layerThicknessHB=0;//(calorThickness-nofEELayers*layerThicknessEE)/(float)nofHB;  //the rest 1'768, about 8.7 nuclear int lengths

	    if(nofHB<1)
	        calorThickness=layerThicknessEE*(float)nofEELayers;
	    noTrackLayers = 0;


	    calorSizeXY  = 100*cm;

	}
	else if(geo == homogenous_muons){
	    calorThickness=2000*mm;

	    layerGranularity.clear();
	    layerSplitGranularity.clear();
	    nofEELayers = 50;
	    nofHB=0;
	    for(int i=0;i<nofEELayers+nofHB;i++){
	        G4double granularity=32;
	        layerGranularity.push_back(granularity);
	        layerSplitGranularity.push_back(-granularity/2);
	    }

	    layerThicknessEE=2000*mm / (float)nofEELayers;//26*8.903*mm ;//* calorThickness/2000*mm; //26 radiation lengths like CMS (23.2cm
	    layerThicknessHB=(calorThickness-nofEELayers*layerThicknessEE)/(float)nofHB;  //the rest 1'768, about 8.7 nuclear int lengths

	    if(nofHB<1)
	        calorThickness=layerThicknessEE*(float)nofEELayers;
	    noTrackLayers = 0;


	    calorSizeXY  = 35.2*cm;

	}
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B4DetectorConstruction::~B4DetectorConstruction()
{ 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


/*
 * creates a single sandwich tile in a layer
 */
G4VPhysicalVolume* B4DetectorConstruction::createSandwich(G4LogicalVolume* layerLV,
		G4double dx,
		G4double dy,
		G4double dz,
		G4ThreeVector position,
		G4String name,
		G4double absorberfraction,
		G4VPhysicalVolume*& absorber,
		G4Material* material,
		G4double rotation){

	auto absdz=absorberfraction*dz;
	auto gapdz=(1-absorberfraction)*dz;

	//auto layerThickness = absoThickness + gapThickness;

	auto sandwichS   = new G4Box("Sandwich_"+name,           // its name
			dx/2-epsilon, dy/2-epsilon, dz/2-epsilon); // its size

	auto sandwichLV  = new G4LogicalVolume(
			sandwichS,           // its solid
			defaultMaterial,  // its material
			"Sandwich_"+name);         // its name

	G4RotationMatrix * rot =0;
	if(rotation){
	    //rotation part:
	    rot = new G4RotationMatrix();
	    rot->set( G4ThreeVector(1,0,0),  rotation );//delta in 2pi?
	}

	//
	// Absorber, technically this is empty
	//
	if(absorberfraction>0){
	    auto absorberS
	    = new G4Box("Abso_"+name,            // its name
	            dx/2-2*epsilon, dy/2-2*epsilon, absdz/2-2*epsilon); // its size

	    auto absorberLV
	    = new G4LogicalVolume(
	            absorberS,        // its solid
	            absorberMaterial, // its material
	            "Abso_"+name);          // its name

	    absorber
	    = new G4PVPlacement(
	            rot,                //  rotation
	            G4ThreeVector(0., 0., -(absdz+gapdz)/2.+absdz/2.), // its position
	            absorberLV,       // its logical volume
	            "Abso_"+name,           // its name
	            sandwichLV,          // its mother  volume
	            false,            // no boolean operation
	            0,                // copy number
	            fCheckOverlaps);  // checking overlaps

	}

	G4Material* gapmaterial=material;
	if(!gapmaterial)
	    gapmaterial=gapMaterial;
	//
	// Gap
	//
	auto gapS
	= new G4Box("Gap_"+name,             // its name
			dx/2-2*epsilon, dy/2-2*epsilon, gapdz/2-2*epsilon); // its size

	auto gapLV
	= new G4LogicalVolume(
			gapS,             // its solid
			gapmaterial,      // its material
			"Gap_"+name);           // its name


    G4double maxStep = dz/20.;
    G4double maxTime = 2.*s;
    G4UserLimits* stepLimit = new G4UserLimits(maxStep,DBL_MAX,maxTime);
    gapLV->SetUserLimits(stepLimit);


	auto activeMaterial
	= new G4PVPlacement(
	        rot,                // no rotation
			G4ThreeVector(0., 0., absdz/2.), // its position
			gapLV,            // its logical volume
			"Gap_"+name,            // its name
			sandwichLV,          // its mother  volume
			false,            // no boolean operation
			0,                // copy number
			fCheckOverlaps);  // checking overlaps

	//place the sandwich

	//auto sandwichPV =
			new G4PVPlacement(
			        rot,                // no rotation
				position, // its position
				sandwichLV,       // its logical volume
				"Sandwich_"+name,           // its name
				layerLV,          // its mother  volume
				false,            // no boolean operation
				0,                // copy number
				fCheckOverlaps);  // checking overlaps

	return activeMaterial;


}

G4VPhysicalVolume* B4DetectorConstruction::createLayer(G4LogicalVolume * caloLV,
		G4double thickness,
		G4int granularity, G4double absfraction,G4ThreeVector position,
		G4String name, int layernumber, G4double calibration, G4int    nsmallsensorsrow,
		G4Material* material,
		G4double sizexy, bool istracker){

    if(!sizexy)
        sizexy=calorSizeXY;

    position += G4ThreeVector(0,0,thickness/2.);

    auto layerS   = new G4Box("Layer_"+name,           // its name
            sizexy/2, sizexy/2, thickness/2); // its size

    auto layerLV  = new G4LogicalVolume(
            layerS,           // its solid
            defaultMaterial,  // its material
			"Layer_"+name);         // its name

    //if(layernumber<0){

    //needs to be put into any LV
        G4double maxStep = thickness/20.;
        G4double maxTime = 2.*s;
        G4UserLimits* stepLimit = new G4UserLimits(maxStep,DBL_MAX,maxTime);
        layerLV->SetUserLimits(stepLimit);

    //}
	auto layerPV = new G4PVPlacement(
			0,                // no rotation
			position, // its position
			layerLV,       // its logical volume
			"Layer_"+name,           // its name
			caloLV,          // its mother  volume
			false,            // no boolean operation
			0,                // copy number
			fCheckOverlaps);  // checking overlaps


	G4double coarsedivider=(G4double)granularity;
	G4double largesensordxy=sizexy/coarsedivider;
	G4double smallsensordxy=sizexy/2./(double)nsmallsensorsrow;

			//largesensordxy/4;
	if(nsmallsensorsrow<0){
    	nsmallsensorsrow=granularity/2;
    	smallsensordxy=largesensordxy;
	}
	if(granularity<2)
		nsmallsensorsrow=0;


	//divide into 4 areas:
	// LG  HG
	// LG  LG
	//
	// LG: low granularity
	// HG: high granularity


	auto placeSensors = [] (
			G4ThreeVector startcorner,
			bool small,
			G4double sensorsize,
			G4double Thickness,
			int gran,
			G4ThreeVector pos,
			G4String lname,
			std::vector<sensorContainer >* acells,
			G4LogicalVolume* layerlogV,
			B4DetectorConstruction* drec,
			G4ThreeVector patentpos,
			G4double absfractio, int laynum, G4double calib, G4Material* sw_material, bool istrk) {
		for(int xi=0;xi<gran;xi++){
			G4double posx=startcorner.x()+sensorsize/2+sensorsize*(G4double)xi;
			for(int yi=0;yi<gran;yi++){
				G4double posy=startcorner.y()+sensorsize/2+sensorsize*(G4double)yi;
				if(!small && posy>pos.y() && posx > pos.x()){
					continue; //here are the small sensors
				}
				auto sandwichposition=G4ThreeVector(posx,posy,pos.z());


				G4VPhysicalVolume * absorber=0;
				auto activesensor=drec->createSandwich(layerlogV,sensorsize,sensorsize,
						Thickness,sandwichposition,
						lname+"_sensor_"+createString(xi)+"_"+createString(yi),
						absfractio,absorber,sw_material);

				sensorContainer sensordesc(activesensor,
						sensorsize,Thickness,sensorsize*sensorsize,
						patentpos.x()+posx,
						patentpos.y()+posy,
						patentpos.z(),laynum,absorber,istrk);
				G4cout << "created sensor with ID "<< sensordesc.getGlobalDetID() <<" sizexy "<< sensorsize <<"mm at "<< sandwichposition << G4endl;
				sensordesc.setEnergyscalefactor(calib);
				acells->push_back(sensordesc);
			}

		}
	};

	G4ThreeVector lowerleftcorner=G4ThreeVector(
			0-sizexy/2,
			0-sizexy/2,
			0);




	placeSensors(lowerleftcorner, true,largesensordxy,thickness,
	        granularity,G4ThreeVector(0,0,0),name,&activecells_,layerLV,this,
	        position,absfraction,layernumber,calibration,material,istracker);

	G4cout << "layer position="<<position <<G4endl;

	return layerPV;

}

void B4DetectorConstruction::createCalo(G4LogicalVolume * caloLV,G4ThreeVector position,G4String name){

    //CHANGE
	G4double absorberFractionEE=0.95;//1e-5;
	G4double absorberFractionHB=absorberFractionEE;
	G4double calibrationEE=1;
	G4double calibrationHB=1;

//define the geometries


	G4double lastzpos=-layerThicknessEE;
	for(int i=0;i<nofEELayers+nofHB;i++){
		int granularity=layerGranularity.at(i);
		int splitgranularity = layerSplitGranularity.at(i);
		G4double absfraction=absorberFractionEE;
		G4double thickness=layerThicknessEE;
		G4double calibration=calibrationEE;

		if(i>=nofEELayers){
			absfraction=absorberFractionHB;
			thickness=layerThicknessHB;
			calibration=calibrationHB;
		}
		G4ThreeVector createatposition=G4ThreeVector(0,0,lastzpos+thickness)+position;
		createLayer(
				caloLV,thickness,
				granularity,
				absfraction,
				createatposition,
				name+"layer"+createString(i),i,1,splitgranularity);//calibration);
		G4cout << "created layer "<<  i<<" at "<< createatposition << G4endl;
		lastzpos+=thickness;
	}

	G4cout << "Calo: created " << activecells_.size() << " sensors"<<std::endl;

	lastzpos=((float)noTrackLayers)*(-5*cm);
	float originpoint = ((float)noTrackLayers+1)*(-5*cm);

	for(int i=0;i<noTrackLayers;i++){

	    int granularity=2*32;
	    int splitgranularity = -1;
	    G4double absfraction=0;
	    G4double thickness=0.3*mm;
	    G4double calibration=1;
	    G4double layersizexy=calorSizeXY;// * (1. - (lastzpos)/originpoint);
	    G4cout << "layersizexy " << layersizexy << G4endl;

	    G4ThreeVector createatposition=G4ThreeVector(0,0,lastzpos+thickness)+position;

	    createLayer(
	            caloLV,thickness,
	            granularity,
	            absfraction,
	            createatposition,
	            name+"tracker"+createString(i),-100+i,1,splitgranularity,
	            trackerMaterial, layersizexy,true);//calibration);
	    G4cout << "created tracker "<<  i<<" at "<< createatposition << G4endl;
	    lastzpos+=5*cm;

	}


}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B4DetectorConstruction::DefineMaterials()
{ 
	// Lead material defined using NIST Manager
	auto nistManager = G4NistManager::Instance();
	nistManager->FindOrBuildMaterial("G4_Pb");
	nistManager->FindOrBuildMaterial("G4_PbWO4");
    nistManager->FindOrBuildMaterial("G4_Si");
    nistManager->FindOrBuildMaterial("G4_Fe");

	//nistManager->ListMaterials("all");

	// Liquid argon material
	G4double a;  // mass of a mole;
	G4double z;  // z=mean number of protons;
	G4double density;
	new G4Material("liquidArgon", z=18., a= 39.95*g/mole, density= 1.390*g/cm3);
	// The argon by NIST Manager is a gas with a different density

	// Vacuum
	new G4Material("Galactic", z=1., a=1.01*g/mole,density= universe_mean_density,
			kStateGas, 2.73*kelvin, 3.e-18*pascal);

	// Print materials
	//G4cout << *(G4Material::GetMaterialTable()) << G4endl;


	//CHANGE
	// Get materials
	defaultMaterial = G4Material::GetMaterial("Galactic");
	absorberMaterial = G4Material::GetMaterial("G4_Fe");//G4_Pb
	gapMaterial = G4Material::GetMaterial("G4_PbWO4");//G4_PbWO4
	trackerMaterial = G4Material::GetMaterial("G4_Si");

	if ( ! defaultMaterial || ! absorberMaterial || ! gapMaterial || !trackerMaterial ) {
		G4ExceptionDescription msg;
		msg << "Cannot retrieve materials already defined.";
		G4Exception("B4DetectorConstruction::DefineVolumes()",
				"MyCode0001", FatalException, msg);
	}

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* B4DetectorConstruction::DefineVolumes()
{
	// Geometry parameters






    //auto calorThickness = nofEELayers * layerThicknessEE + nofHB*layerThicknessHB;
	auto worldSizeXY = 1.2 * calorSizeXY;
	G4double worldSizeZ  = 4 * m;



	//
	// World
	//
	auto worldS
	= new G4Box("World",           // its name
			worldSizeXY/2, worldSizeXY/2, worldSizeZ); // its size

	auto worldLV
	= new G4LogicalVolume(
			worldS,           // its solid
			defaultMaterial,  // its material
			"World");         // its name


	G4double maxStep = 1.0*mm;
	G4double maxTime = 20.*s;
	G4UserLimits* stepLimit = new G4UserLimits(maxStep,DBL_MAX,maxTime);
	worldLV->SetUserLimits(stepLimit);


	auto worldPV
	= new G4PVPlacement(
			0,                // no rotation
			G4ThreeVector(),  // at (0,0,0)
			worldLV,          // its logical volume
			"World",          // its name
			0,                // its mother  volume
			false,            // no boolean operation
			0,                // copy number
			fCheckOverlaps);  // checking overlaps

	//
	// Calorimeter
	//
	createCalo(worldLV,G4ThreeVector(0,0,0),"");



	G4cout << "created in total "<< activecells_.size()<<" sensors" <<G4endl;

	//
	// Visualization attributes
	//
	worldLV->SetVisAttributes (G4VisAttributes::GetInvisible());

	G4double tgreen=0;
	G4double tred=1;
    G4double cgreen=0;
    G4double cred=1;
	float ncalo=0,ntracker=0;
	for(auto& v: activecells_){
	    if(v.isTracker()) ntracker++;
	    else ncalo++;}

	for(auto& v: activecells_){
	    auto col = G4Colour(1,1,0);
	    if(v.isTracker()){
	        col = G4Colour(tred, tgreen,0);
	        tred-= 1./ntracker;
	        tgreen += 1./ntracker;
	    }
	    else{
	        col = G4Colour(cred, cgreen,0);
	        cred-= 1./ncalo;
	        cgreen += 1./ncalo;
	    }

	    auto simpleBoxVisAtt= new G4VisAttributes(col);
	    simpleBoxVisAtt->SetVisibility(true);
        simpleBoxVisAtt->SetForceSolid(true);
		v.getVol()->GetLogicalVolume()->SetVisAttributes(simpleBoxVisAtt);
	}
	//
	// Always return the physical World
	//
	return worldPV;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B4DetectorConstruction::ConstructSDandField()
{ 
	// Create global magnetic field messenger.
	// Uniform magnetic field is then created automatically if
	// the field value is not zero.
	G4ThreeVector fieldValue(0,0,0);
	fMagFieldMessenger = new G4GlobalMagFieldMessenger(fieldValue);
	fMagFieldMessenger->SetVerboseLevel(2);

	// Register the field messenger for deleting
	G4AutoDelete::Register(fMagFieldMessenger);


	auto* fieldprop = G4TransportationManager::GetTransportationManager()->GetPropagatorInField();
	fieldprop->SetMaxLoopCount(10) ;


}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
