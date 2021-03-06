/**
 *  Project: HipGISAXS (High-Performance GISAXS)
 *
 *  File: hipgisaxs_main.hpp
 *  Created: Jun 11, 2012
 *
 *  Author: Abhinav Sarje <asarje@lbl.gov>
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
#include <woo/comm/multi_node_comm.hpp>

#include <common/typedefs.hpp>
#include <common/globals.hpp>
#include <config/hig_input.hpp>
#include <model/common.hpp>
#include <model/layer.hpp>
#include <model/qgrid.hpp>
#include <ff/ff.hpp>
#include <sf/sf.hpp>
#include <image/image.hpp>

namespace hig {

  /* This class provides high level functions for all the
   * components underneath in the HipGISAXS package. */

  class HipGISAXS {
    private:
      real_t freq_;
      real_t k0_;

      int num_layers_;
      int num_structures_;

      RefractiveIndex substrate_refindex_;    // can these be avoided? ...
      RefractiveIndex single_layer_refindex_;    // or can these be filled while reading input? ...
      real_t single_layer_thickness_;

      complex_t dns2_;   // find a better name ...

      vector3_t vol_;      /* represents the illuminated volume */
      vector3_t cell_;    /* stores the domain size in each dimension */
      RotMatrix_t rot_;
      MultiLayer multilayer_; 

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

      unsigned int nrow_;    /* Image size */
      unsigned int ncol_;
      unsigned int nqx_;      /* number of q-points along x */
      unsigned int nqy_;      /* number of q-points along y */
      unsigned int nqz_;      /* number of q-points along z */
      unsigned int nqz_extended_;  /* number of q-points along z in case of gisaxs */

//      complex_t* fc_;        /* fresnel coefficients */
//      FormFactor ff_;        /* form factor object */
//      StructureFactor sf_;   /* structure factor object */

      #ifdef USE_MPI
        woo::MultiNode multi_node_; /* for multi node communication */
      #endif
      woo::comm_t root_comm_;     /* the universe */
      woo::comm_t sim_comm_;      /* communicator for simulations */

      bool init();  /* global initialization for all runs */
      //bool init_steepest_fit(real_t);  /* init for steepest descent fitting */
      bool run_init(real_t, real_t, real_t, SampleRotation&);   /* init for a single run */
      bool run_gisaxs(real_t, real_t, real_t, real_t, real_t*&,
                      #ifdef USE_MPI
                        woo::comm_t,
                      #endif
                      int c = 0);
                    /* a single GISAXS run */

      /* wrapper over sf function */
      bool structure_factor(StructureFactor&, std::string, vector3_t&, Lattice*&, 
                    vector3_t&, vector3_t&, RotMatrix_t &,
                    std::shared_ptr<Paracrystal>, std::shared_ptr<PercusYevick>
                  #ifdef USE_MPI
                    , std::string
                  #endif
                  );

      /* wrapper over ff function */
      bool form_factor(FormFactor&, ShapeName, std::string, shape_param_list_t&, vector3_t&,
                  real_t, real_t, RotMatrix_t &
                  #ifdef USE_MPI
                    , std::string
                  #endif
                  );

      bool layer_qgrid_qz(real_t, complex_t);
      bool compute_propagation_coefficients(real_t, complex_t*&, complex_t*&,
                  complex_t*&, complex_t*&, complex_t*&, complex_t*&,
                  complex_t*&, complex_t*&, complex_t*&, complex_t*&);
      bool compute_fresnel_coefficients_embedded(real_t, complex_t*&);
      bool compute_fresnel_coefficients_top_buried(real_t, complex_t*&);

      bool compute_rotation_matrix_x(real_t, vector3_t&, vector3_t&, vector3_t&);
      bool compute_rotation_matrix_y(real_t, vector3_t&, vector3_t&, vector3_t&);
      bool compute_rotation_matrix_z(real_t, vector3_t&, vector3_t&, vector3_t&);

      void save_gisaxs(real_t *final_data, std::string output);
      bool gaussian_smearing(real_t*&, real_t);

      bool normalize(real_t*&, unsigned int);

      bool illuminated_volume(real_t, real_t, int, RefractiveIndex);
      bool spatial_distribution(structure_iterator_t, real_t, int, int&, int&, real_t*&);
      bool orientation_distribution(structure_iterator_t, real_t*, int &, int, real_t*&, real_t *&);
      bool generate_repetition_range(unsigned int, unsigned int, int, std::vector<unsigned int>&);
      bool construct_repetition_distribution(const GrainRepetitions&, int, std::vector<vector3_t>&);
      bool construct_scaling_distribution(std::vector<StatisticType>, 
                    vector3_t, vector3_t,
                    std::vector<int>,
                    std::vector<vector3_t>&,
                    std::vector<real_t>&); 

      /* some functions just for testing and debugging */
      bool write_qgrid(char* filename);
      bool read_form_factor(FormFactor&, const char* filename);
      void printfc(const char*, complex_t*, unsigned int);
      void printfr(const char*, real_t*, unsigned int);

      real_t gaussian(real_t x, real_t mean, real_t sigma) {
        return (1/(sigma*SQRT_2PI_)*std::exp(-(x-mean)*(x-mean)/(2*sigma*sigma)));
      } // gaussian()

      real_t gaussian3d(vector3_t x, vector3_t mean, vector3_t sigma) {
        vector3_t t1 = (x-mean) * (x-mean);
        vector3_t t2 = sigma * sigma;
        real_t xx = 0;
        real_t kk = 1;
        int ndim = 0;
        for(int i = 0; i < 3; ++ i) {
          if(sigma[i] > 0) {
            xx += t1[i] / t2[i];
            kk *= sigma[i];
            ++ ndim;
          } // if
        } // for
        return (1. / (std::pow(SQRT_2PI_, ndim) * kk) * std::exp(-0.5 * xx));
      } // gaussian3d()

      real_t cauchy(real_t x, real_t l, real_t s) {
        return 1.0 / (PI_ * s * (1.0 + ((x - l) / s) * ((x - l) / s)));
      } // cauchy()

    public:
      HipGISAXS(int, char**);
      ~HipGISAXS();

      bool construct_input(const char* filename) {
        return HiGInput::instance().construct_input_config(filename);
      } // construct_input()

      /* loops over all configs and computes GISAXS for each */
      bool run_all_gisaxs(int = 0, int = 0, int = 0);

      /* fitting related */

      /* update parameters given a map:key->value */
      bool update_params(const map_t&);

      bool fit_init();
      bool compute_gisaxs(real_t*&, std::string = "");

      bool is_master() {
        #ifdef USE_MPI
          return multi_node_.is_master(sim_comm_);
        #else
          return true;
        #endif
      } // is_master()
      int rank() {
        #ifdef USE_MPI
          return multi_node_.rank(sim_comm_);
        #else
          return 0;
        #endif
      } // is_master()

      // fitting related ... TODO: improve
      std::vector <std::string> fit_param_keys() const {
        return HiGInput::instance().fit_param_keys();
      } // get_fit_param_keys()
      std::vector <std::pair <real_t, real_t> > fit_param_limits() const {
        return HiGInput::instance().fit_param_limits();
      } // get_fit_param_keys()
      real_vec_t fit_param_step_values() const {
        return HiGInput::instance().fit_param_step_values();
      } // fit_param_step_values()
      unsigned int nqx() const { return nqx_; }
      unsigned int nqy() const { return nqy_; }
      unsigned int nqz() const { return nqz_; }
      unsigned int ncol() const { return ncol_; }
      unsigned int nrow() const { return nrow_; }

      string_t reference_data_path(int i) const {
        return HiGInput::instance().reference_data_path(i);
      } // reference_data_path()
      string_t reference_data_mask(int i) const {
        return HiGInput::instance().reference_data_mask(i);
      } // reference_data_mask()

      int num_fit_params() const { return HiGInput::instance().num_fit_params(); }
      std::vector<real_t> fit_param_init_values() const {
        return HiGInput::instance().fit_param_init_values();
      } // fit_param_init_values()

      bool override_qregion(unsigned int n_par, unsigned int n_ver, unsigned int i);

      #ifdef USE_MPI
        woo::MultiNode* multi_node_comm() { return &multi_node_; }
        bool update_sim_comm(woo::comm_t comm) { sim_comm_ = comm; return true; }
      #endif

      // debug stuff
      bool check_finite(real_t*, unsigned int);
      bool check_finite(complex_t*, unsigned int);

      //template <typename ErrorFunction>
      //real_t compute_gisaxs_error(const ErrorFunction&, real_t*);
      /*template <typename ErrorFunction>
      real_t compute_gisaxs_error(const ErrorFunction& err_func, real_t* ref_data) {
        #ifdef USE_MPI
          // this is for the whole comm world
          const char* world_comm = "world";
          bool master = multi_node_.is_master(world_comm);
        #else
          bool master = true;
        #endif

        if(!init()) return -1.0;
        real_t error = -1.0;

        int num_alphai = 0, num_phi = 0, num_tilt = 0;;
        real_t alphai_min, alphai_max, alphai_step;
        HiGInput::instance().scattering_alphai(alphai_min, alphai_max, alphai_step);
        if(alphai_max < alphai_min) alphai_max = alphai_min;
        if(alphai_min == alphai_max || alphai_step == 0) num_alphai = 1;
        else num_alphai = (alphai_max - alphai_min) / alphai_step + 1;
        real_t phi_min, phi_max, phi_step;
        HiGInput::instance().scattering_inplanerot(phi_min, phi_max, phi_step);
        if(phi_step == 0) num_phi = 1;
        else num_phi = (phi_max - phi_min) / phi_step + 1;
        real_t tilt_min, tilt_max, tilt_step;
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
        real_t alpha_i = alphai_min;
        real_t alphai = alpha_i * PI_ / 180;
        real_t phi_rad = phi_min * PI_ / 180;
        real_t tilt_rad = tilt_min * PI_ / 180;
        if(master) std::cout << "-- Computing GISAXS ... " << std::flush;
        real_t* final_data = NULL;
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
      //bool fit_steepest_descent(real_t, real_t, real_t, real_t,
      //              real_t, real_t, real_t, unsigned int,
      //              MPI::Intracomm&, int = 0, int = 0, int = 0);
      //real_t compute_cut_fit_error(real_t*, real_t*, real_t);

  }; // class HipGISAXS

} // namespace hig

#endif /* _HIPGISAXS_MAIN_HPP_ */
