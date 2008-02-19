/* This file was automatically generated.  Do not edit! */
double jac_rho_bar(int junk,int jnode_box,double **x);
double resid_rho_bar(int junk,int jnode_box,double **x);
double resid_and_Jac_sten_fill_sum_Ncomp(int sten_type,double **x,int iunk,int loc_inode,int inode_box,int izone,int *ijk_box,int resid_only_flag,int jzone_flag,double(*fp_prefactor)(int,int,int *),double(*fp_resid)(int,int,double **),double(*fp_jacobian)(int,int,double **));
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
extern void *LinProbMgr_manager;
#define MF_VARIABLE  2
extern int Type_attr;
#define FALSE 0
#if !defined(_CON_CONST_H_)
#define _CON_CONST_H_
#endif
#if !defined(FALSE) && !defined(_CON_CONST_H_)
#define FALSE 0
#endif
extern int *L2B_node;
double load_mean_field(int sten_type,int iunk,int loc_inode,int icomp,int izone,int *ijk_box,double **x,int resid_only_flag);
