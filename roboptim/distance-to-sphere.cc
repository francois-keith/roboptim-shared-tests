// Copyright (C) 2011-2013 by Florent Lamiraux, Thomas Moulard, AIST, CNRS.
//
// This file is part of the roboptim.
//
// roboptim is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// roboptim is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with roboptim.  If not, see <http://www.gnu.org/licenses/>.


#include "common.hh"

#include <roboptim/core/sum-of-c1-squares.hh>

namespace roboptim
{
  namespace distanceToSphere
  {
    struct ExpectedResult
    {
      static const double x0[];
      static const double fx0;
      static const double x[];
      static const double fx;
    };
    const double ExpectedResult::x0[] = {0., 0.};
    const double ExpectedResult::fx0  = 4.8974713057829096;
    const double ExpectedResult::x[]  = {-1.5, -1.2};
    const double ExpectedResult::fx   = 1.0;

    /// Distance between a point on unit sphere and another point in R^3
    template <typename T>
    struct F : public GenericDifferentiableFunction<T>
    {
      ROBOPTIM_DIFFERENTIABLE_FUNCTION_FWD_TYPEDEFS_
      (GenericDifferentiableFunction<T>);

      explicit F () : GenericDifferentiableFunction<T>
		      (2, 3,
		       "vector between unit sphere and point (x,y,z)"),
		      point_ (3)
      {
	sphericalCoordinates (point_, ExpectedResult::x[0],
                              ExpectedResult::x[1]);
	point_ *= 2.;
      }

      ~F () throw ()
      {}

      void impl_compute (result_t& result, const argument_t& x) const throw ();
      void impl_gradient (gradient_t& gradient, const argument_t& x,
                          size_type functionId = 0) const throw ();


      static void sphericalCoordinates (result_t& res, double theta, double phi)
      {
        res (0) = cos(theta) * cos(phi);
        res (1) = sin(theta) * cos(phi);
        res (2) = sin(phi);
      }
      result_t point_;
    };

    template <typename T>
    void
    F<T>::impl_compute (result_t& result, const argument_t& x) const throw ()
    {
      result.setZero ();
      double theta = x[0];
      double phi = x[1];
      sphericalCoordinates (result, theta, phi);
      result -= point_;
    }

    template <>
    void
    F<EigenMatrixSparse>::impl_gradient
    (gradient_t& grad, const argument_t& x, size_type functionId)
      const throw ()
    {
      grad.setZero ();

      double theta = x[0];
      double phi = x[1];

      switch (functionId)
        {
        case 0:
          grad.insert (0) = -sin(theta) * cos(phi);
          grad.insert (1) = -cos(theta) * sin(phi);
          break;
        case 1:
          grad.insert (0) = cos(theta) * cos(phi);
          grad.insert (1) = -sin(theta) * sin(phi);
          break;
        case 2:
          grad.insert (0) = 0.;
          grad.insert (1) = cos(phi);
          break;
        default:
          abort();
        }
    }

    template <typename T>
    void
    F<T>::impl_gradient
    (gradient_t& grad, const argument_t& x, size_type functionId)
      const throw ()
    {
      grad.setZero ();

      double theta = x[0];
      double phi = x[1];

      switch (functionId)
        {
        case 0:
          grad[0] = -sin(theta) * cos(phi);
          grad[1] = -cos(theta) * sin(phi);
          break;
        case 1:
          grad[0] = cos(theta) * cos(phi);
          grad[1] = -sin(theta) * sin(phi);
          break;
        case 2:
          grad[0] = 0.;
          grad[1] = cos(phi);
          break;
        default:
          abort();
        }
    }

  } // namespace distanceToSphere
} // namespace roboptim


BOOST_FIXTURE_TEST_SUITE (distanceToSphere, TestSuiteConfiguration)

BOOST_AUTO_TEST_CASE (distanceToSphere_problem1)
{
  using namespace roboptim;
  using namespace roboptim::distanceToSphere;

  // Tolerances for Boost checks.
  double f0_tol = 1e-6;
  double x_tol = 1e-5;
  double f_tol = 1e-4;

  // Build problem.
  boost::shared_ptr <F<functionType_t> > f (new F<functionType_t> ());
  GenericSumOfC1Squares<functionType_t> soq (f, "");

  solver_t::problem_t problem (soq);

  // Load starting point
  F<functionType_t>::argument_t x (2);
  x << ExpectedResult::x0[0], ExpectedResult::x0[1];
  problem.startingPoint () = x;

  // Bounds on theta \in [-Pi/2, Pi/2]
  problem.argumentBounds ()[0] = Function::makeInterval (-M_PI_2, M_PI_2);

  // Bounds on phi \in [-Pi, Pi]
  problem.argumentBounds ()[1] = Function::makeInterval (-M_PI, M_PI);

  BOOST_CHECK_SMALL_OR_CLOSE (soq (x)[0], ExpectedResult::fx0, f0_tol);

  // Initialize solver.
  SolverFactory<solver_t> factory (SOLVER_NAME, problem);
  solver_t& solver = factory ();

  // Add an optimization logger
  OptimizationLogger<solver_t> logger
    (solver, "/tmp/roboptim-shared-tests/" SOLVER_NAME "/distance-to-sphere");

  // Set optional log file for debugging
  SET_LOG_FILE (solver);

  // Compute the minimum and retrieve the result.
  solver_t::result_t res = solver.minimum ();

  // Display solver information.
  std::cout << solver << std::endl;

  // Process the result
  PROCESS_RESULT();
}

BOOST_AUTO_TEST_SUITE_END ()