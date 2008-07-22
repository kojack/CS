/*
  Copyright (C) 2008 by Julian Mautner

  This application is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This application is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this application; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CSCLOUDDYNAMICS_PLUGIN_H__
#define __CSCLOUDDYNAMICS_PLUGIN_H__

#include <csgeom/vector3.h>
#include "imesh/clouds.h"
#include "csCloudsUtils.h"

/**
Supervisor-class implementation:
This class represents a three dimensional voxel-grid of user specific size.
Each parcel is a cube (simplifies calculations!) of size dx * dx * dx (m_fGridSize).
The whole simulation updates each timestep the entire grid. Output of the simulation
is the condensed water mixing ratio of each parcel.
*/
class csCloudsDynamics : public scfImplementation1<csCloudsDynamics, iCloudsDynamics>
{
private:
	/**
	From each field there are two instances, because each time-step all of
	them have to be advected, while the last vaild state has to be mantained.
	Therefore iAcutalIndex gives the index of the last updated instance 
	and iLastIndex gives the other one. Both are either	0 or 1. 
	At the end of each timestep those two are going to be swapped.
	*/
	UINT						m_iLastIndex;
	UINT						m_iActualIndex;
	/**
	Temperature, pressure and both mixing ratios are definded at the center
	of each voxel. Indexing is therefore as always f(x, y, z)
	*/
	csRef<iField3<float>>		m_arfPotTemperature[2];				// T, potential temperature
	csRef<iField3<float>>		m_arfCondWaterMixingRatios[2];		// qc
	csRef<iField3<float>>		m_arfWaterVaporMixingRatios[2];		// qv
	csRef<iField3<csVector3>>	m_arvForceField;
	/**
	Velocity is defined at the boundaries of each cell. Half-way index
	notation is used in consequence. These fields are of size N + 1
	*/
	csRef<iField3<csVector3>>	m_arvVelocityField[2];				// u

	//This rotation-field is defined at Cell-Centers!
	csRef<iField3<csVector3>>	m_arvRotVelField;					// rot(u)

	float						m_fTimeStep;
	UINT						m_iGridSizeX;
	UINT						m_iGridSizeY;
	UINT						m_iGridSizeZ;
	float						m_fGridScale;						// dx
	float						m_fInvGridScale;					// 1 / dx

	//====================================================//
	//            USER SPECIFIC VARIABLES				  //
	//====================================================//
	//Epslion for the vorticityConfinement-Force calculaion
	float						m_fVCEpsilon;						// _e
	//Inverse of reference virtual potential temperature
	float						m_fInvRefVirtPotTemp;				// _Tp
	//Scaling factor for condensed water in buoyant-force-calculaion
	float						m_fCondWaterScaleFactor;			// _fqc
	//Acceleration due to gravitation
	csVector3					m_vGravitationAcc;					// _g
	//Condensation-Rate
	float						m_fCondensationRate;				// _C
	//Preasure at sealevel
	float						m_fRefPressure;						// _p0
	//Temperature Lapse rate
	float						m_fTempLapseRate;					// _G
	//Temperature at sea-level
	float						m_fRefTemperature;					// _T0
	//Latent heat of vaporization of water
	float						m_fLatentHeat;						// _L
	//Ideal gas konstant for dry air
	float						m_fIdealGasConstant;				// _R
	//Specific heat capacity (dry air, constant pressure)
	float						m_fSpecificHeatCapacity;			// _cp
	//Ambient temperature
	float						m_fAmbientTemperature;				// _TA
	//Initial-value for condensed water mixing ratio
	float						m_fInitCondWaterMixingRatio;
	//Initial-value for water vapor mixing ratio
	float						m_fInitWaterVaporMixingRatio;
	//====================================================//
	
	//Interpolates the velocity (from boundaries)
	inline const csVector3 GetVelocityOfCellCenter(const csRef<iField3<csVector3>>& rField, 
												   const UINT x, const UINT y, const UINT z)
	{
		return 0.5f * csVector3(rField->GetValue(x, y, z).x + rField->GetValue(x + 1, y, z).x,
								rField->GetValue(x, y, z).y + rField->GetValue(x, y + 1, z).y,
								rField->GetValue(x, y, z).z + rField->GetValue(x, y, z + 1).z);
	}

	//Calculates the rotation of the velocity field u, and stores it in arvRotVelField
	//O(n^3)
	inline void ComputeRotationField()
	{
		for(UINT x = 0; x < m_iGridSizeX; ++x)
		{
			for(UINT y = 0; y < m_iGridSizeY; ++y)
			{
				for(UINT z = 0; z < m_iGridSizeZ; ++z)
					m_arvRotVelField->SetValue(CalcRotation(m_arvVelocityField[m_iActualIndex], x, y, z, m_fGridScale), x, y, z);
			}
		}
	}
	//Computes for each cell the buoyant and the vorticity confinement force
	//O(n^3)
	inline void ComputeForceField()
	{
		for(UINT x = 0; x < m_iGridSizeX; ++x)
		{
			for(UINT y = 0; y < m_iGridSizeY; ++y)
			{
				for(UINT z = 0; z < m_iGridSizeZ; ++z)
				{
					const csVector3 vForce = ComputeBuoyantForce(x, y, z) + ComputeVorticityConfinement(x, y, z);
					m_arvForceField->SetValue(vForce, x, y, z);
				}
			}
		}
	}
	//Returns the vorticity confinement force of a certain parcel depending on rot(u), dx, _e
	const csVector3 ComputeVorticityConfinement(const UINT x, const UINT y, const UINT z);
	//Returns the buoyant force of a certain parcel depending on _g, qc, T, _Tp, _fqc
	const csVector3 ComputeBuoyantForce(const UINT x, const UINT y, const UINT z);

	//Advects temperature, mixing ratios and velocity itself
	//O(n^3)
	void AdvectAllQuantities();

	//Add accelerating forces (Buoyant and VoricityConfinement) to u
	//O(n^3)
	void AddAcceleratingForces();

	//After this method was invoked all boundarycondition on u ar satisfied
	void SatisfyVelocityBoundaryCond();

	//Frees all reserved memory
	inline void FreeReservedMemory()
	{
		m_iGridSizeX = m_iGridSizeY = m_iGridSizeZ = 0;
		m_arfCondWaterMixingRatios[0].Invalidate(); m_arfCondWaterMixingRatios[1].Invalidate();
		m_arfWaterVaporMixingRatios[0].Invalidate(); m_arfWaterVaporMixingRatios[1].Invalidate();
		m_arfPotTemperature[0].Invalidate(); m_arfPotTemperature[1].Invalidate();
		m_arvVelocityField[0].Invalidate(); m_arvVelocityField[1].Invalidate();
		m_arvRotVelField.Invalidate();
		m_arvForceField.Invalidate();
	}

	//For all userspecific values there are standard once too, which are set here!
	inline void SetStandardValues()
	{
		SetGridScale(1.f);
		SetCondensedWaterScaleFactor(1.f);
		SetGravityAcceleration(csVector3(0.f, -9.81f, 0.f));
		SetVorticityConfinementForceEpsilon(0.01f);
		SetReferenceVirtPotTemperature(290.f);
		SetTempLapseRate(0.01f);									//10 K/km = 0.01 K/m
		SetReferenceTemperature(290.f);
		SetReferencePressure(101300.f);								//1.013 bar = 101300 N/m�
		SetIdealGasConstant(287.f);									//287 J/(kg K)
		SetLatentHeat(2.501f);										//2.501 J/kg
		SetSpecificHeatCapacity(1005.f);							//1005 J/(kg K)
		SetAmbientTemperature(290.f);
		SetInitialCondWaterMixingRatio(0.0f);
		SetInitialWaterVaporMixingRatio(0.8f);
	}

	//swaps actualIndex and LastIndex
	inline void SwapFieldIndizes()
	{
		m_iActualIndex ^= m_iLastIndex ^= m_iActualIndex ^= m_iLastIndex;
	}

public:
	csCloudsDynamics(iBase* pParent) : scfImplementationType(this, pParent), m_iLastIndex(1), m_iActualIndex(0),
		m_iGridSizeX(0), m_iGridSizeY(0), m_iGridSizeZ(0)
	{
		SetStandardValues();
	}
	~csCloudsDynamics() {}

	//This method MUST be called at least once!
	virtual inline void SetGridSize(const UINT x, const UINT y, const UINT z);

	//Configuration-Setter
	virtual inline void SetGridScale(const float dx) {m_fGridScale = dx; m_fInvGridScale = 1.f / dx;}
	virtual inline void SetCondensedWaterScaleFactor(const float fqc) {m_fCondWaterScaleFactor = fqc;}
	virtual inline void SetGravityAcceleration(const csVector3& vG) {m_vGravitationAcc = vG;}
	virtual inline void SetVorticityConfinementForceEpsilon(const float e) {m_fVCEpsilon = e;}
	virtual inline void SetReferenceVirtPotTemperature(const float T) {m_fInvRefVirtPotTemp = 1.f / T;}
	virtual inline void SetTempLapseRate(const float G) {m_fTempLapseRate = G;}
	virtual inline void SetReferenceTemperature(const float T) {m_fRefTemperature = T;}
	virtual inline void SetReferencePressure(const float p) {m_fRefPressure = p;}
	virtual inline void SetIdealGasConstant(const float R) {m_fIdealGasConstant = R;}
	virtual inline void SetLatentHeat(const float L) {m_fLatentHeat = L;}
	virtual inline void SetSpecificHeatCapacity(const float cp) {m_fSpecificHeatCapacity = cp;}
	virtual inline void SetAmbientTemperature(const float T) {m_fAmbientTemperature = T;}
	virtual inline void SetInitialCondWaterMixingRatio(const float qc) {m_fInitCondWaterMixingRatio = qc;}
	virtual inline void SetInitialWaterVaporMixingRatio(const float qv) {m_fInitWaterVaporMixingRatio = qv;}

	//Computes N steps of the entire simulation. If iStepCount == 0, then an entire timestep
	//is calculated
	virtual const bool DoComputationSteps(const UINT iStepCount, const float fTime = 0.f);

	//Returns the simulation output!
	virtual inline const csRef<iField3<float>>& GetCondWaterMixingRatios() const
	{
		//Always when an entire timestep was done, the acutal-index becomes the last-index
		//So the lastindex fields are those of the LAST COMPLETLY DONE TIMESTEP!
		return m_arfCondWaterMixingRatios[m_iLastIndex];
	}
};

#endif // __CSCLOUDDYNAMICS_PLUGIN_H__