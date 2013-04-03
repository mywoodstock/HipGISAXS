/**
 *  Project: HipGISAXS (High-Performance GISAXS)
 *
 *  File: ff_num_mic.hpp
 *  Created: Apr 02, 2013
 *  Modified: Wed 03 Apr 2013 09:47:05 AM PDT
 *
 *  Author: Abhinav Sarje <asarje@lbl.gov>
 */
	
#ifndef __NUMERIC_FF_MIC_HPP__
#define __NUMERIC_FF_MIC_HPP__
	
#include "typedefs.hpp"
	
namespace hig {
	
	/**
	 * Class for computing Form Factor in either single or double precision on a CPU + MIC node
	 * TODO: this should be merged with the pure CPU version
	 */
	class NumericFormFactorM {
		public:

			NumericFormFactorM();
			~NumericFormFactorM();

			bool init();	// TODO ...
	
		private:
			void form_factor_kernel(float_t*, float_t*, scomplex_t*, float_t*,
									unsigned int, unsigned int, unsigned int, unsigned int,
									unsigned int, unsigned int, unsigned int, unsigned int,
									unsigned int, unsigned int, unsigned int, unsigned int,
									scomplex_t*);

			__attribute__((target(mic:0)))
			float2_t compute_fq(float, float2_t, float2_t);

			__attribute__((target(mic:0)))
			double2_t compute_fq(double, double2_t, double2_t);

			void reduction_kernel(unsigned int, unsigned int, unsigned int,	unsigned int,
									unsigned int, unsigned int, unsigned int, unsigned int,
									unsigned int, unsigned int, unsigned int, unsigned int,
									unsigned int, unsigned int, unsigned int, unsigned int,
									scomplex_t*, scomplex_t*);

			void compute_hyperblock_size(int, int, int, int, unsigned int&, unsigned int&, unsigned int&,
									unsigned int&
									#ifdef FINDBLOCK
										, const int, const int, const int, const int
									#endif
									);

			void move_to_main_ff(scomplex_t*,
									unsigned int, unsigned int, unsigned int, unsigned int,
									unsigned int, unsigned int, unsigned int, unsigned int,
									unsigned int, unsigned int, unsigned int, unsigned int,
									complex_t*);

		public:

			unsigned int compute_form_factor(int, float_vec_t&, complex_t*&,
									float_t*, int, float_t*, int, complex_t* qz, int,
									float_t&, float_t&, float_t&
									#ifdef FINDBLOCK
										, const int, const int, const int, const int
									#endif
									);
			
	}; // class NumericFormFactorM
		
} // namespace hig
		
#endif // __NUMERIC_FF_MIC_HPP__