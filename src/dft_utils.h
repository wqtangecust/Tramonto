/* This file was automatically generated.  Do not edit! */
#include <stdio.h>
void print_to_file_comp(FILE *fp,int icomp,double val,char *var_label,int first);
void print_to_file(FILE *fp,double val,char *var_label,int first);
void print_to_screen_comp(int icomp,double val,char *var_label);
void print_to_screen(double val,char *var_label);
#include <stdlib.h>
#include <math.h>
#if defined(HAS_VALUES_H)
#include <values.h>
#include <unistd.h>
#include <string.h>
#endif
#include "mpi.h"
#include "az_aztec.h"
#include "rf_allo.h"
#include "dft_basic_lin_prob_mgr_wrapper.h"
#include "dft_poly_lin_prob_mgr_wrapper.h"
#include "dft_hardsphere_lin_prob_mgr_wrapper.h"
#include "Tramonto_ConfigDefs.h"
extern int Nrho_bar_s;
extern int Ndim;
void solutionVec_to_nOrdering(double *rhoBar_SVOrdering,double *n);
extern int Nnodes;
#define GLOBAL 2
extern int Nnodes_box;
#define BOX 0
extern int Nnodes_per_proc;
#define LOCAL_N 1
extern int Nunk_per_node;
#define MATRIX_FILL_NODAL 1   /* set to zero for physics based ordering */
int loc_find(int iunk,int inode,int flag);
#define NMER_MAX     100
#define NBOND_MAX 4
extern double G_WJDC_b[NMER_MAX *NBOND_MAX];
#define WJDC         3
#define CMS          0
extern int Type_poly;
#define G_CHAIN        2 
extern double Field_WJDC_b[NMER_MAX];
#define WJDC_FIELD     8
#define CMS_FIELD      1
extern double BondWTC_RTF[NMER_MAX *NMER_MAX];
extern double BondWTC_LBB[NMER_MAX *NMER_MAX];
extern double BondWTC_b[NMER_MAX *NMER_MAX];
#define BONDWTC        7
extern double Xi_cav_RTF[4];
extern double Xi_cav_LBB[4];
extern double Xi_cav_b[4];
#define CAVWTC         6
#define NCOMP_MAX 5
extern double Betamu_RTF[NCOMP_MAX];
extern double Betamu_LBB[NCOMP_MAX];
extern double VEXT_MAX;
#define DIFFUSION      5
extern double Elec_pot_RTF;
extern double Elec_pot_LBB;
#define POISSON        3
extern double Rhobar_b_RTF[10];
extern double Rhobar_b_LBB[10];
extern double Rhobar_b[10];
#define HSRHOBAR       4
extern double Rho_b_RTF[NCOMP_MAX];
extern double Rho_coex[2];
extern double Rho_b_LBB[NCOMP_MAX];
extern double Rho_b[NCOMP_MAX];
extern double Rho_seg_RTF[NMER_MAX];
extern double Rho_seg_LBB[NMER_MAX];
extern int Lsteady_state;
#define NEQ_TYPE       11 
extern int Phys2Unk_first[NEQ_TYPE];
extern double Rho_seg_b[NMER_MAX];
extern int Lseg_densities;
#define DENSITY        0
extern int Unk2Phys[3 *NCOMP_MAX+NMER_MAX+NMER_MAX *NMER_MAX+13];
double constant_boundary(int iunk,int jnode_box);
extern int *Mesh_coarsen_flag;
extern int Nwall;
extern int Mesh_coarsening;
extern int Nzone;
extern int Coarser_jac;
int find_jzone(int izone,int inode_box);
#define NWALL_MAX 600 
extern int WallType[NWALL_MAX];
extern int **Lsemiperm;
extern int **Wall_elems;
int node_box_to_elem_box_reflect(int inode_box,int local_node,int *reflect_flag);
int node_box_to_elem_box_reflect(int inode_box,int local_node,int *reflect_flag);
extern int Nnodes_per_el_V;
double HW_boundary_weight(int icomp,int ilist,double *hw_weight,int inode_box,int *reflect_flag);
