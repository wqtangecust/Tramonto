/* This file was automatically generated.  Do not edit! */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if defined(HAS_VALUES_H)
#include <values.h>
#include <unistd.h>
#endif
#include "mpi.h"
#include "az_aztec.h"
#include "rf_allo.h"
#include "dft_basic_lin_prob_mgr_wrapper.h"
#include "dft_poly_lin_prob_mgr_wrapper.h"
#include "dft_hardsphere_lin_prob_mgr_wrapper.h"
#include "Tramonto_ConfigDefs.h"
extern int Lprint_gofr;
void print_vext(double **vext,char *output_file);
int element_to_node(int ielement);
int node_box_to_elem_box_reflect(int inode_box,int local_node,int *reflect_flag);
int node_to_elem(int inode_all,int local_node,int *reflect_flag);
#define TRUE  1
#if !defined(_CON_CONST_H_)
#define _CON_CONST_H_
#endif
#if !defined(TRUE) && !defined(_CON_CONST_H_)
#define TRUE  1
#endif
#define PERIODIC     1
#define NDIM_MAX  3
extern int Type_bc[NDIM_MAX][2];
extern int Nelements;
void print_charge_vol(double *charge_els,char *output_file);
void print_freen_profile_1D(double *freen_profile_1D,char *output_file);
#define FALSE 0
#if !defined(FALSE) && !defined(_CON_CONST_H_)
#define FALSE 0
#endif
void print_charge_surf(double **charge_w_sum,char *output_file);
void print_Nodes_to_zone(int *node_to_zone,char *output_file);
void print_zeroTF(int **zero_TF,char *output_file);
extern double Size_x[NDIM_MAX];
#define NWALL_MAX 600 
extern double WallPos[NDIM_MAX][NWALL_MAX];
extern int Nwall;
#define WTC          3
void print_gofr(char *output_file6);
extern int Ntype_mer;
#define NCOMP_MAX 5
#define NBLOCK_MAX   5
extern int Nmer_t[NCOMP_MAX][NBLOCK_MAX];
#if defined(DEC_ALPHA)
#define POW_DOUBLE_INT powi
#endif
#if !(defined(DEC_ALPHA))
#define POW_DOUBLE_INT pow
#endif
extern int ***Poly_to_Unk;
extern int Geqn_start[NCOMP_MAX];
extern int **Nbond;
#define NMER_MAX     100
extern int Type_mer[NCOMP_MAX][NMER_MAX];
extern int Nmer[NCOMP_MAX];
extern int Npol_comp;
#define NONE       -1
#define NONE      -1
#define NONE -1
#define CMS_G          2 
extern double VEXT_MAX;
#define NEQ_TYPE       8
extern int Phys2Unk_first[NEQ_TYPE];
extern int Unk2Phys[3 *NCOMP_MAX+NMER_MAX+NMER_MAX *NMER_MAX+13];
extern double Esize_x[NDIM_MAX];
void node_to_ijk(int node,int *ijk);
extern double Charge_f[NCOMP_MAX];
extern double Rho_b[NCOMP_MAX];
extern double Temp_elec;
#define PI    M_PI
#define COULOMB      1
extern int Ipot_ff_c;
extern int Npoisson;
#define BONDWTC       7
#define CAVWTC     6
#define HSRHOBAR       4
#define CMS_FIELD      1
#define DIFFUSION      5
#define POISSON        3
extern int Phys2Nunk[NEQ_TYPE];
#define DENSITY        0
#define VERBOSE      3 
extern int Iwrite;
#define CMS_SCFT     2
#define CMS          0
extern int Type_poly;
void print_profile(char *output_file4);
void print_profile_box(double **x,char *outfile);
extern double *Vext_old;
extern int Num_Proc;
extern double **Vext;
extern int Ncomp;
void collect_vext_old();
extern double *X_old;
void safe_free(void **ptr);
void safe_free(void **ptr);
extern int *Comm_offset_unk;
extern int *Comm_unk_proc;
extern int *Comm_offset_node;
extern int *Comm_node_proc;
extern int *L2G_node;
extern int Nunknowns;
extern int Proc;
#if defined(DEBUG)
extern int Proc;
#endif
extern int *L2B_node;
#if defined(__STDC__)
void *array_alloc(int numdim,...);
#endif
void *array_alloc(int numdim,...);
#if !(defined(__STDC__))
void *array_alloc(...);
#endif
extern int Nunk_per_node;
extern int Nnodes_per_proc;
extern int Nodes_x[NDIM_MAX];
extern int Nodes_x_old[NDIM_MAX];
extern int Ndim;
extern int Nnodes;
extern int Nodes_old;
void collect_x_old(double **x);