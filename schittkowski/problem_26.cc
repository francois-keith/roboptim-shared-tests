// Copyright (C) 2014 by Thomas Moulard, AIST, CNRS.
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

namespace roboptim
{
  namespace schittkowski
  {
    namespace problem26
    {
      struct ExpectedResult
      {
	static const double f0;

	static const double alpha;
	static const double beta;
	static const double a;

	static const double x[];
	static const double fx;
      };
      const double ExpectedResult::f0 = 21.16;

      const double ExpectedResult::alpha = std::sqrt (139. / 108.);
      const double ExpectedResult::beta = 61. / 54.;
      const double ExpectedResult::a =
	std::pow (alpha - beta, 1. / 3.)
	- std::pow (alpha + beta, 1. / 3.) - 2. / 3.;

      // another local minima is {a, a, a} where
      const double ExpectedResult::x[] = {1., 1., 1.};

      const double ExpectedResult::fx = 0.;

      template <typename T>
      class F : public GenericDifferentiableFunction<T>
      {
      public:
	ROBOPTIM_DIFFERENTIABLE_FUNCTION_FWD_TYPEDEFS_
	(GenericDifferentiableFunction<T>);

	explicit F () throw ();
	void
	impl_compute (result_t& result, const argument_t& x) const throw ();
	void
	impl_gradient (gradient_t& grad, const argument_t& x, size_type)
	  const throw ();
      };

      template <typename T>
      F<T>::F () throw ()
	: GenericDifferentiableFunction<T>
	  (3, 1, "(x₀ - x₁)² + (x₁ - x₃)⁴")
      {}

      template <typename T>
      void
      F<T>::impl_compute (result_t& result, const argument_t& x)
	const throw ()
      {
	result[0] = std::pow(x[0] - x[1], 2) + std::pow (x[1] - x[2], 4);
      }

      template <>
      void
      F<EigenMatrixSparse>::impl_gradient
      (gradient_t& grad, const argument_t& x, size_type)
	const throw ()
      {
	grad.insert (0) = 2. * x[0] - 2. * x[1];
	grad.insert (1) = -2. * x[0] + 2. * x[1] + 4 * std::pow(x[1] - x[2], 3);
	grad.insert (2) = -4. * std::pow (x[1] - x[2], 3);
      }

      template <typename T>
      void
      F<T>::impl_gradient (gradient_t& grad, const argument_t& x, size_type)
	const throw ()
      {
	grad[0] = 2. * x[0] - 2. * x[1];
	grad[1] = -2. * x[0] + 2. * x[1] + 4 * std::pow(x[1] - x[2], 3);
	grad[2] = -4. * std::pow (x[1] - x[2], 3);
      }

      template <typename T>
      class G : public GenericDifferentiableFunction<T>
      {
      public:
	ROBOPTIM_DIFFERENTIABLE_FUNCTION_FWD_TYPEDEFS_
	(GenericDifferentiableFunction<T>);

	explicit G () throw ();
	void
	impl_compute (result_t& result, const argument_t& x) const throw ();
	void
	impl_gradient (gradient_t& grad, const argument_t& x, size_type)
	  const throw ();
      };

      template <typename T>
      G<T>::G () throw ()
	: GenericDifferentiableFunction<T>
	  (3, 1, "(1 + x₁²) x₀ + x₂⁴ - 3")
      {}

      template <typename T>
      void
      G<T>::impl_compute (result_t& result, const argument_t& x)
	const throw ()
      {
	result[0] = (1 + x[1] * x[1]) * x[0] + std::pow (x[2], 4.) - 3.;
      }

      template <>
      void
      G<EigenMatrixSparse>::impl_gradient
      (gradient_t& grad, const argument_t& x, size_type)
	const throw ()
      {
	grad.insert (0) = x[1] * x[1] + 1;
	grad.insert (1) = 2 * x[0] * x[1];
	grad.insert (2) = 4 * std::pow(x[2], 3);
      }

      template <typename T>
      void
      G<T>::impl_gradient (gradient_t& grad, const argument_t& x, size_type)
	const throw ()
      {
	grad[0] = x[1] * x[1] + 1;
	grad[1] = 2 * x[0] * x[1];
	grad[2] = 4 * std::pow (x[2], 3.);
      }
    } // end of namespace problem26.
  } // end of namespace schittkowski.
} // end of namespace roboptim.

BOOST_FIXTURE_TEST_SUITE (schittkowski, TestSuiteConfiguration)

BOOST_AUTO_TEST_CASE (schittkowski_problem26)
{
  using namespace roboptim;
  using namespace roboptim::schittkowski::problem26;

  // Tolerances for Boost checks.
  double f0_tol = 1e-4;
  double x_tol = 1e-4;
  double f_tol = 1e-4;

  // Build problem.
  F<functionType_t> f;
  solver_t::problem_t problem (f);

  boost::shared_ptr<G<functionType_t> > g =
    boost::make_shared<G<functionType_t> > ();
  problem.addConstraint (g, G<functionType_t>::makeInterval (0., 0.));

  F<functionType_t>::argument_t x (3);
  x << -2.6, 2, 2;
  problem.startingPoint () = x;

  BOOST_CHECK_SMALL_OR_CLOSE (f (x)[0], ExpectedResult::f0, f0_tol);

  std::cout << f.inputSize () << std::endl;
  std::cout << problem.function ().inputSize () << std::endl;

  // Initialize solver.
  SolverFactory<solver_t> factory (SOLVER_NAME, problem);
  solver_t& solver = factory ();
  OptimizationLogger<solver_t> logger
    (solver,
     "/tmp/roboptim-shared-tests/" SOLVER_NAME "/schittkowski/problem-26");

  // Set optional log file for debugging
  SET_LOG_FILE(solver);

  std::cout << f.inputSize () << std::endl;
  std::cout << problem.function ().inputSize () << std::endl;

  // Compute the minimum and retrieve the result.
  solver_t::result_t res = solver.minimum ();

  std::cout << f.inputSize () << std::endl;
  std::cout << problem.function ().inputSize () << std::endl;

  // Display solver information.
  std::cout << solver << std::endl;

  // Process the result
  PROCESS_RESULT();
}

BOOST_AUTO_TEST_SUITE_END ()