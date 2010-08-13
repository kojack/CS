/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __CS_IMESH_IK_H__
#define __CS_IMESH_IK_H__

/**\file
 * Inverse Kinematics animation nodes for an animated mesh.
 */

#include "csutil/scf_interface.h"

#include "imesh/skeleton2anim.h"

struct iMovable;
struct iCamera;
class csOrthoTransform;

namespace CS {
namespace Mesh {

struct iAnimatedMesh;

} // namespace Mesh
} // namespace CS

/**\addtogroup meshplugins
 * @{ */

namespace CS {
namespace Animation {

struct iBodyChain;
struct iBodySkeleton;
struct iSkeletonRagdollNode2;

/// Identifier for an effector within an Inverse Kinematics animation node
typedef unsigned int EffectorID;

struct iSkeletonIKNodeFactory2;

/**
 * A class to manage the creation and deletion of Inverse Kinematics animation 
 * node factories.
 */
struct iSkeletonIKManager2 : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iSkeletonIKManager2, 1, 0, 0);

  /**
   * Create a new Inverse Kinematics animation node factory.
   */
  virtual iSkeletonIKNodeFactory2* CreateAnimNodeFactory
    (const char *name, CS::Animation::iBodySkeleton* skeleton) = 0;

  /**
   * Find the Inverse Kinematics animation node factory with the given name.
   */
  virtual iSkeletonIKNodeFactory2* FindAnimNodeFactory
    (const char* name) const = 0;

  /**
   * Delete all Inverse Kinematics animation node factories.
   */
  virtual void ClearAnimNodeFactories () = 0;
};

/**
 * Factory for the Inverse Kinematics animation node. With Inverse Kinematics, you can
 * generate an animation for a sub-part of the skeleton (a CS::Animation::iBodyChain) by
 * specifying some geometrical constraints on some effectors placed on the skeleton. This
 * can be used eg to grab an object or to avoid foot sliding problems within a locomotion
 * system.
 *
 * You must first define some effectors, ie some points on the skeleton. Then in the
 * CS::Animation::iSkeletonIKNode2, you will be able to define some geometric constraints
 * on these effectors.
 *
 * \sa iSkeletonIKPhysicalNodeFactory2
 */
struct iSkeletonIKNodeFactory2 : public iSkeletonAnimNodeFactory2
{
  SCF_INTERFACE(CS::Animation::iSkeletonIKNodeFactory2, 1, 0, 0);

  /**
   * Set the child animation node of this node. The IK controller will
   * add its control on top of the animation of the child node. This child
   * node is not mandatory.
   */
  virtual void SetChildNode (CS::Animation::iSkeletonAnimNodeFactory2* node) = 0;

  /**
   * Return the child animation node of this node.
   */
  virtual CS::Animation::iSkeletonAnimNodeFactory2* GetChildNode () const = 0;

  /**
   * Clear the child animation node of this node.
   */
  virtual void ClearChildNode () = 0;

  /**
   * Add an effector to this factory.
   * \param chain The sub-part of the skeleton that is controlled by this node when
   * the effector is constrained.
   * \param bone The bone where sits the effector.
   * \param transform The position of the effector, relatively to the bone.
   */
  virtual CS::Animation::EffectorID AddEffector (CS::Animation::iBodyChain* chain,
						 BoneID bone,
						 csOrthoTransform& transform) = 0;

  /**
   * Remove the given effector
   */
  virtual void RemoveEffector (CS::Animation::EffectorID effector) = 0;

  // TODO: listeners
};

/**
 * An animation node that generates an animation for a sub-part of the skeleton (a
 * CS::Animation::iBodyChain) by specifying some geometrical constraints on the
 * effectors placed on the skeleton.
 *
 * This node is inactive until there are some constraints on some effector. The
 * effectors are defined by iSkeletonIKNodeFactory2::AddEffector().
 *
 * \sa iSkeletonIKPhysicalNode2
 */
struct iSkeletonIKNode2 : public iSkeletonAnimNode2
{
  SCF_INTERFACE(CS::Animation::iSkeletonIKNode2, 1, 0, 0);

  /**
   * Add a constraint on the given effector so that it sticks to the given world transform.
   * \param effector ID of the effector
   * \param transform Transform to stick to, in world coordinates
   */
  virtual void AddConstraint (CS::Animation::EffectorID effector,
			      csOrthoTransform& transform) = 0;

  /**
   * Add a constraint on the given effector so that it sticks to the given transform of the
   * iMovable.
   * \param effector ID of the effector
   * \param target The iMovable to stick to.
   * \param offset Offset transform to the iMovable, in the iMovable local coordinates.
   */
  virtual void AddConstraint (CS::Animation::EffectorID effector,
			      iMovable* target, const csOrthoTransform& offset) = 0;

  /**
   * Add a constraint on the given effector so that it sticks to the given transform of the
   * iCamera.
   * \param effector ID of the effector
   * \param target The iCamera to stick to.
   * \param offset Offset transform to the iCamera, in the iCamera local coordinates.
   */
  virtual void AddConstraint (CS::Animation::EffectorID effector,
			      iCamera* target, const csOrthoTransform& offset) = 0;

  /**
   * Remove the constraint on the given effector. This animation node won't be active anymore
   * if there are no more constraints.
   */
  virtual void RemoveConstraint (CS::Animation::EffectorID effector) = 0;
};

/**
 * An implementation of the CS::Animation::iSkeletonIKNodeFactory2 based on physical
 * simulation. This node will use a CS::Animation::iSkeletonRagdollNode2 and apply
 * physical constraints on the rigid bodies created by the ragdoll node.
 *
 * This IK method has the advantage to be able to be physically accurate. It has for
 * example the uncommon capability to manage the collisions with the body and the
 * environment. This IK method is however not suited in applications where you need
 * the IK solution to be independant of the history of the motion. This method is also
 * less efficient than traditionial IK algorithms.
 *
 * \warning The current implementation does not care about the rotational component
 * of the constraints.
 *
 * \sa iSkeletonIKNodeFactory2
 */
struct iSkeletonIKPhysicalNodeFactory2 : public iSkeletonIKNodeFactory2
{
  SCF_INTERFACE(CS::Animation::iSkeletonIKPhysicalNodeFactory2, 1, 0, 0);

  /**
   * Set whether or not the CS::Animation::iBodyChain controlled by this node have to be
   * reset to their previous dynamic state when there are no more constraints on the chain.
   * Default value is true.
   *
   * The body chain is indeed put in the CS::Animation::STATE_DYNAMIC state when there is
   * at least one active constraint (see CS::Animation::iSkeletonRagdollNode2::SetBodyChainState()).
   */
  virtual void SetChainAutoReset (bool reset) = 0;

  /**
   * Get whether or not the CS::Animation::iBodyChain controlled by this node have to be
   * reset to their previous dynamic state when there are no more constraints on the chain.
   *
   * The body chain is indeed put in the CS::Animation::STATE_DYNAMIC state when there is
   * at least one active constraint (see CS::Animation::iSkeletonRagdollNode2::SetBodyChainState()).
   */
  virtual bool GetChainAutoReset () const = 0;
};

/**
 * An implementation of the CS::Animation::iSkeletonIKNode2 based on physical
 * simulation. This node will use a CS::Animation::iSkeletonRagdollNode2 and apply
 * physical constraints on the rigid bodies created by the ragdoll node.
 *
 * This IK method has the advantage to be able to be physically accurate. It has for
 * example the uncommon capability to manage the collisions with the body and the
 * environment. This IK method is however not suited in applications where you need
 * the IK solution to be independant of the history of the motion. This method is also
 * less efficient than traditionial IK algorithms.
 *
 * \warning The current implementation does not care about the rotational component
 * of the constraints.
 *
 * \sa iSkeletonIKNode2
 */
struct iSkeletonIKPhysicalNode2 : public iSkeletonIKNode2
{
  SCF_INTERFACE(CS::Animation::iSkeletonIKPhysicalNode2, 1, 0, 0);

  /**
   * Set the ragdoll node to be used by this node. The ragdoll node has to be active
   * somewhere inside the animation blending tree (but its effective position has no importance).
   */
  virtual void SetRagdollNode (CS::Animation::iSkeletonRagdollNode2* ragdollNode) = 0;

  /**
   * Get the ragdoll node to be used by this node.
   */
  virtual CS::Animation::iSkeletonRagdollNode2* GetRagdollNode () const = 0;
};

} // namespace Animation
} // namespace CS

/** @} */

#endif //__CS_IMESH_IK_H__
