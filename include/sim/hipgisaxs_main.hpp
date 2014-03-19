/**
 *  Project: HipGISAXS (High-Performance GISAXS)
 *
 *  File: hipgisaxs_main.hpp
 *  Created: Jun 11, 2012
 *  Modified: Sun 26 Jan 2014 10:06:15 AM PST
 *
 *  Author: Abhinav Sarje <asarje@lbl.gov>
 *  Developers: Slim Chourou <stchourou@lbl.gov>
 *              Abhinav Sarje <asarje@lbl.gov>
 *              Elaine Chan <erchan@lbl.gov>
 *              Alexander Hexemer <ahexemer@lbl.gov>
 *              Xiaoye Li <xsli@lbl.gov>
 *
 *  Licensing: The HipGISAXS software is only available to be downloaded and
 *  used by employees of academic research institutions, not-for-profit
 *  research laboratories, or governmental research facilities. Please read the
 *  accompanying LICENSE file before downloading the software. By downloading
 *  the software, you are agreeing to be bound by the terms of this
 *  NON-COMMERCIAL END USER LICENSE AGREEMENT.
 */

#ifndef _HIPGISAXS_MAIN_HPP_
#define _HIPGISAXS_MAIN_HPP_

#include <complex>
#ifdef USE_MPI
#include <mpi.h>
#include <woo/comm/multi_node_comm.hpp>
#endif

#include <common/typedefs.hpp>
#include <common/globals.hpp>
#include <config/hig_input.hpp>
#include <model/common.hpp>
#include <model/qgrid.hpp>
#include <ff/ff.hpp>
#include <sf/sf.hpp>
#include <image/image.hpp>

namespace hig {

	/* This class provides high level functions for all the
	 * components underneath in the HipGISAXS package. */

	class HipGISAXS {
		private:
			float_t freq_;
			float_t k0_;

			int num_layers_;
			int num_structures_;

			RefractiveIndex substrate_refindex_;		// can these be avoided? ...
			RefractiveIndex single_layer_refindex_;		// or can these be filled while reading input? ...
			float_t single_layer_thickness_;

			complex_t dns2_; 	// find a better name ...

			vector3_t vol_;			/* represents the illuminated volume */
			vector3_t cell_;		/* stores the domain size in each dimension */

			class SampleRotation {
				friend class HipGISAXS;

				private:
					vector3_t r1_, r2_, r3_;

					SampleRotation():
						r1_(0.0, 0.0, 0.0),
						r2_(0.0, 0.0, 0.0),
						r3_(0.0, 0.0, 0.0) {
					} //SampleRotation()
			}; // rotation_matrix_;

			unsigned int nqx_;			/* number of q-points along x */
			unsigned int nqy_;			/* number of q-points along y */
			unsigned int nqz_;			/* number of q-points along z */
			unsigned int nqz_extended_;	/* number of q-points along z in case of gisaxs */

//			complex_t* fc_;				/* fresnel coefficients */
//			FormFactor ff_;				/* form factor object */
//			StructureFactor sf_;		/* structure factor object */

			#ifdef USE_MPI
				woo::MultiNode multi_node_;	/* for multi node communication */
			#endif

			bool init();	/* global initialization for all runs */
			//bool init_steepest_fit(float_t);	/* init for steepest descent fitting */
			bool run_init(float_t, float_t, float_t, SampleRotation&); 	/* init for a single run */
			bool run_gisaxs(float_t, float_t, float_t, float_t, float_t*&, const char*, int c = 0);
										/* a single GISAXS run */

			/* wrapper over sf function */
			bool structure_factor(StructureFactor&, std::string, vector3_t&, Lattice*&, vector3_t&,
									vector3_t&, vector3_t&, vector3_t&
									#ifdef USE_MPI
										, const char*
									#endif
									);

			/* wrapper over ff function */
			bool form_factor(FormFactor&, ShapeName, std::string, shape_param_list_t&, vector3_t&,
									float_t, float_t, vector3_t&, vector3_t&, vector3_t&
									#ifdef USE_MPI
										, const char*
									#endif
									);

			bool layer_qgrid_qz(float_t, complex_t);
			bool compute_propagation_coefficients(float_t, complex_t*&, complex_t*&,
									complex_t*&, complex_t*&, complex_t*&, complex_t*&,
									complex_t*&, complex_t*&, complex_t*&, complex_t*&);
			bool compute_fresnel_coefficients_embedded(float_t, complex_t*&);
			bool compute_fresnel_coefficients_top_buried(float_t, complex_t*&);

			bool compute_rotation_matrix_x(float_t, vector3_t&, vector3_t&, vector3_t&);
			bool compute_rotation_matrix_y(float_t, vector3_t&, vector3_t&, vector3_t&);
			bool compute_rotation_matrix_z(float_t, vector3_t&, vector3_t&, vector3_t&);

			void save_gisaxs(float_t *final_data, std::string output);

			bool illuminated_volume(float_t, float_t, int, RefractiveIndex);
			bool spatial_distribution(structure_iterator_t, float_t, int, int&, int&, float_t*&);
			bool orientation_distribution(structure_iterator_t, float_t*, int, int, float_t*&);

			/* some functions just for testing and debugging */
			bool write_qgrid(char* filename);
			bool read_form_factor(FormFactor&, const char* filename);
			void printfc(const char*, complex_t*, unsigned int);
			void printfr(const char*, float_t*, unsigned int);

		public:
			HipGISAXS(int, char**);
			~HipGISAXS();

			bool construct_input(char* filename) {
				return HiGInput::instance().construct_input_config(filename);
			} // construct_input()

			/* loops over all configs and computes GISAXS for each */
			bool run_all_gisaxs(int = 0, int = 0, int = 0);

			/* fitting related */

			/* update parameters given a map:key->value */
			bool update_params(const map_t&);

			bool fit_init();
			bool compute_gisaxs(float_t*&);

			// temporary fitting related ...
			std::vector <std::string> get_fit_param_keys() const {
				return HiGInput::instance().get_fit_param_keys();
			} // get_fit_param_keys()
			std::vector <std::pair <float_t, float_t> > get_fit_param_limits() const {
				return HiGInput::instance().get_fit_param_limits();
			} // get_fit_param_keys()
			unsigned int nqx() const { return nqx_; }
			unsigned int nqy() const { return nqy_; }
			unsigned int nqz() const { return nqz_; }


			//template <typename ErrorFunction>
			//float_t compute_gisaxs_error(const ErrorFunction&, float_t*);
			/*template <typename ErrorFunction>
			float_t compute_gisaxs_error(const ErrorFunction& err_func, float_t* ref_data) {
				#ifdef USE_MPI
					// this is for the whole comm world
					const char* world_comm = "world";
					bool master = multi_node_.is_master(world_comm);
				#else
					bool master = true;
				#endif

				if(!init()) return -1.0;
				float_t error = -1.0;

				int num_alphai = 0, num_phi = 0, num_tilt = 0;;
				float_t alphai_min, alphai_max, alphai_step;
				HiGInput::instance().scattering_alphai(alphai_min, alphai_max, alphai_step);
				if(alphai_max < alphai_min) alphai_max = alphai_min;
				if(alphai_min == alphai_max || alphai_step == 0) num_alphai = 1;
				else num_alphai = (alphai_max - alphai_min) / alphai_step + 1;
				float_t phi_min, phi_max, phi_step;
				HiGInput::instance().scattering_inplanerot(phi_min, phi_max, phi_step);
				if(phi_step == 0) num_phi = 1;
				else num_phi = (phi_max - phi_min) / phi_step + 1;
				float_t tilt_min, tilt_max, tilt_step;
				HiGInput::instance().scattering_tilt(tilt_min, tilt_max, tilt_step);
				if(tilt_step == 0) num_tilt = 1;
				else num_tilt = (tilt_max - tilt_min) / tilt_step + 1;
				if(num_alphai > 1 || num_phi > 1 || num_tilt > 1) {
					if(master)
						std::cerr << "error: currently you can simulate only for single "
									<< "alpha_i, phi and tilt angles"
									<< std::endl;
					return -1.0;
				} // if

				woo::BoostChronoTimer sim_timer;
				sim_timer.start();
				float_t alpha_i = alphai_min;
				float_t alphai = alpha_i * PI_ / 180;
				float_t phi_rad = phi_min * PI_ / 180;
				float_t tilt_rad = tilt_min * PI_ / 180;
				if(master) std::cout << "-- Computing GISAXS ... " << std::flush;
				float_t* final_data = NULL;
				// run a gisaxs simulation
				if(!run_gisaxs(alpha_i, alphai, phi_rad, tilt_rad, final_data,
							#ifdef USE_MPI
								world_comm,
							#endif
							0)) {
					if(master) std::cerr << "error: could not finish successfully" << std::endl;
					return -1.0;
				} // if
				if(master) std::cout << "error ... " << std::flush;
				error = err_func(final_data, ref_data, nqx_, nqy_, nqz_);
				sim_timer.stop();
				if(master) std::cout << "[" << sim_timer.elapsed_msec() << " ms.]" << std::endl;

				return error;
			} // HipGISAXS::compute_gisaxs_error()*/


			/* temporary: does the initial 1D fitting */
			//bool fit_steepest_descent(float_t, float_t, float_t, float_t,
			//							float_t, float_t, float_t, unsigned int,
			//							MPI::Intracomm&, int = 0, int = 0, int = 0);
			//float_t compute_cut_fit_error(float_t*, float_t*, float_t);

	}; // class HipGISAXS

} // namespace hig

#endif /* _HIPGISAXS_MAIN_HPP_ */