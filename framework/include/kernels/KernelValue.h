//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef KERNELVALUE_H
#define KERNELVALUE_H

// local includes
#include "Kernel.h"

// Forward Declarations
class KernelValue;

template <>
InputParameters validParams<KernelValue>();

/**
 * The KernelValue class is responsible for calculating the residuals in form:
 *
 *  JxW[_qp] * _value[_qp] * _test[_i][_qp]
 *
 */
class KernelValue : public Kernel
{
public:
  /**
   * Factory constructor initializes all internal references needed for residual computation.
   *
   * @param parameters The parameters object for holding additional parameters for kernels and
   * derived kernels
   */
  KernelValue(const InputParameters & parameters);

  virtual void computeResidual() override;

  virtual void computeJacobian() override;

  virtual void computeOffDiagJacobian(MooseVariableFEBase & jvar) override;

  using Kernel::computeOffDiagJacobian;

protected:
  /**
   * Called before forming the residual for an element
   */
  virtual Real precomputeQpResidual() = 0;

  /**
   * Called before forming the jacobian for an element
   */
  virtual Real precomputeQpJacobian();

  virtual Real computeQpResidual() final;
};

#endif // KERNELVALUE_H
