/* This file was automatically generated.  Do not edit! */
void calc_init_lambda(double **xInBox);
#include <stdio.h>
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
extern double Rho_t;
#define NCOMP_MAX 5
extern double Eps_ff[NCOMP_MAX][NCOMP_MAX];
#define NEQ_TYPE       13 
extern int Restart_field[NEQ_TYPE];
extern double **Vext;
extern int **Zero_density_TF;
#define SCF_CONSTR	   9
void calc_init_SCFfield(double **xInBox);
extern double Rho_b[NCOMP_MAX];
extern double VEXT_MAX;
#define SCF_FIELD	  10
#define DENSITY        0
extern int Phys2Unk_first[NEQ_TYPE];
extern int Ncomp;
extern int *L2B_node;
extern int Nnodes_per_proc;
void setup_polymer_SCF_field(double **xInBox,int iguess);
extern int Phys2Nunk[NEQ_TYPE];
void calc_init_polymer_G_SCF(double **xInBox);
#define G_CHAIN       11 
#define INIT_GUESS_FLAG  2
extern int ***Poly_to_Unk;
extern int *Unk_to_Bond;
extern int *Unk_to_Seg;
extern int *Unk_to_Poly;
extern int Nbonds;
double CMS_Resid_Bulk_GCHAIN(int iunk,int pol_num,int jseg,int unk_B,int inode_box,int jnode_box,int nunk,int *unk,double weight,double **x);
double CMS_Resid_GCHAIN(int iunk,int pol_num,int jseg,int unk_B,int inode_box,int jnode_box,int nunk,int *unk,double weight,double **x);
#define NBOND_MAX 4
#define NMER_MAX     100
#define FALSE 0
#define TRUE  1
extern int **Nbond;
extern int ***Bonds;
extern int Nmer[NCOMP_MAX];
extern int Geqn_start[NCOMP_MAX];
void calc_init_polymer_G_SCF(double **xInBox);
double load_Chain_Geqns_SCF(int func_type_field,int Njacobian_types, int Njacobian_sums,
							void (*funcArray_Jac[3])(int,int,int,int,int,int,int,int,int *,double,double **),
							double (*fp_ResidG)(int,int,int,int,int,int,int,int *,double,double **),
							double (*fp_ResidG_Bulk)(int,int,int,int,int,int,int,int *,double,double **),
							int iunk, int loc_inode, int inode_box, 
							int *ijk_box, int izone, double **x,
							int resid_only_flag);