/*
//@HEADER
// ******************************************************************** 
// Tramonto: A molecular theory code for structured and uniform fluids
//                 Copyright (2006) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2.1
// of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.
// ********************************************************************
//@HEADER
*/

/*
 *  FILE: dft_newton.c
 *
 *  This file solves the dft problem using Newton's method.
 */
#include "dft_newton.h"
void do_numerical_jacobian(double **);

/*******************************************************************************/
int solve_problem(double **x, double **x2)
/*
 * x and x2 in dft_main are allocated [Nunk_per_node][Nnodes_box]
 * x2 only relevent for Lbinodal calculations, so can mostly be ignored
 */
{
  int iter,iunk,i;
  double **xOwned, **x2Owned;
  int loc_inode,inode_box,itmp;

  /* Construct dft_Linprobmgr with information on number of unknowns*/
  linsolver_setup_control();

  /* Give Nodal Row and Column maps */
  (void) dft_linprobmgr_setnodalrowmap(LinProbMgr_manager, Nnodes_per_proc, L2G_node);
  (void) dft_linprobmgr_setnodalcolmap(LinProbMgr_manager, Nnodes_box     , B2G_node);

  /* send solver manager information about mesh coarsening */
  dft_linprobmgr_setcoarsenednodeslist(LinProbMgr_manager, Nnodes_coarse_loc, List_coarse_nodes);

  /* Linprobmgr can now set up its own numbering scheme, set up unknown-based Maps */
  (void) dft_linprobmgr_finalizeblockstructure(LinProbMgr_manager);

/* PRINT STATEMENTS FOR DEBUG OF NONUNIQUE GLOBAL TO BOX COORD MAPS */
/*for (loc_inode=0;loc_inode<Nnodes_per_proc;loc_inode++){
if (L2G_node[loc_inode]==254) printf("Proc=%d owns global node 254 (local coord=%d boc coord=%d) \n", Proc,loc_inode,L2B_node[loc_inode]);
}
for (inode_box=0;inode_box<Nnodes_box;inode_box++){
if (B2G_node[inode_box]==254) printf("Proc=%d sees global node 254 (box coord=%d ) \n", Proc,inode_box);
}*/
  /* Set initial guess on owned nodes and reconcile ghost nodes using importr2c */
  xOwned = (double **) array_alloc(2, Nunk_per_node, Nnodes_per_proc, sizeof(double));
  set_initial_guess(Iguess1, xOwned);

/* PRINT STATEMENTS FOR DEBUG OF NONUNIQUE GLOBAL TO BOX COORD MAPS */
/*for (inode_box=0;inode_box<Nnodes_box;inode_box++){
if (B2G_node[inode_box]==254) printf("after calling set_inital guess: Proc=%d inode_box=%d B2G_node=%d xOwned=%g\n",
  Proc,inode_box,B2G_node[inode_box],xOwned[0][inode_box]);
}*/
  (void) dft_linprobmgr_importr2c(LinProbMgr_manager, xOwned, x);

/* PRINT STATEMENTS FOR DEBUG OF NONUNIQUE GLOBAL TO BOX COORD MAPS */
/*for (inode_box=0;inode_box<Nnodes_box;inode_box++){
if (B2G_node[inode_box]==254) printf("after calling importr2c: Proc=%d inode_box=%d B2G_node=%d x=%g\n",
  Proc,inode_box,B2G_node[inode_box],x[0][inode_box]);
}*/

  /* If requested, write out initial guess */
   if (Iwrite == VERBOSE) print_profile_box(x,"rho_init.dat");

  /* Do same for second solution vector when Lbinodal is true */
  if (Lbinodal) {
    x2Owned = (double **) array_alloc(2, Nunk_per_node, Nnodes_per_proc, sizeof(double));
    set_initial_guess(BINODAL_FLAG, x2Owned);
    (void) dft_linprobmgr_importr2c(LinProbMgr_manager, x2Owned, x2);
    if (Iwrite == VERBOSE) print_profile_box(x2,"rho_init2.dat");
  }

#ifdef HAVE_NOXLOCA
  NOXLOCA_Solver(x, xOwned, x2Owned);
#else
  if (Loca.method != -1)
    iter = solve_continuation(x, x2);
  else
     iter = newton_solver(x, NULL);
#endif

  safe_free((void **) &xOwned);
  if (Lbinodal)  safe_free((void **) &x2Owned);

  /* Call the destructor for the dft_Linprobmgr */
  dft_linprobmgr_destruct(LinProbMgr_manager);

  /* Call the destructor for the dft_ParameterList */
  dft_parameterlist_destruct(ParameterList_list);

  return(iter);
}

/****************************************************************************/
/****************************************************************************/
void box2owned(double** xBox, double** xOwned) {
  /* Routine to restrict 2d box array to 2d node array */
  /* This is the opposite of importr2c, and strips ghost nodes */
  int iunk, inode, ibox;

  for (ibox=0; ibox<Nnodes_box; ibox++) {
    inode = B2L_node[ibox];
    if (inode!=-1) {
      for (iunk=0; iunk<Nunk_per_node; iunk++)
        xOwned[iunk][inode] = xBox[iunk][ibox];
    }
  }
}

/****************************************************************************/
/****************************************************************************/
int newton_solver(double** x, void* con_ptr) {

  int iter=0;
  int iunk, ibox;
  int converged=FALSE, converged2=TRUE;
  char filename[20]="matrix.dat";
  double start_t;
  double** delta_x;
  delta_x = (double **) array_alloc(2, Nunk_per_node, Nnodes_box, sizeof(double));

  do {
    iter++;

    (void) dft_linprobmgr_initializeproblemvalues(LinProbMgr_manager);

    /* Call Matrix and Residual Fill routine, resid_only_flag=FALSE)*/
    start_t=MPI_Wtime();
    fill_resid_and_matrix_control(x, iter,FALSE); 
    if (iter==1) Time_fill_first=MPI_Wtime()-start_t;
    else         Time_fill_av+=(MPI_Wtime()-start_t);
    /*fill_test(x, FALSE);*/

    start_t=MPI_Wtime();
    (void) dft_linprobmgr_finalizeproblemvalues(LinProbMgr_manager);
    if (Iwrite != NO_SCREEN) print_resid_norm(iter);
    (void) dft_linprobmgr_setupsolver(LinProbMgr_manager);
    if (iter==1) Time_manager_first=MPI_Wtime()-start_t;
    else         Time_manager_av+=(MPI_Wtime()-start_t);
#ifdef NUMERICAL_JACOBIAN
/*   do_numerical_jacobian(x);*/
#endif
    start_t=MPI_Wtime();
    (void) dft_linprobmgr_solve(LinProbMgr_manager);
    if (iter==1) Time_linsolver_first=MPI_Wtime()-start_t;
    else         Time_linsolver_av+=(MPI_Wtime()-start_t);
    
    /* I am assuming getLhs returns box coordinates (e.g. Column Map)!! */
    (void) dft_linprobmgr_getlhs(LinProbMgr_manager, delta_x);

    /*
      for (iunk=0; iunk<Nunk_per_node; iunk++)
      for (ibox=0; ibox<Nnodes_box; ibox++) {
      printf("delta_x[%d][%d] = %g\n", iunk, ibox,delta_x[iunk][ibox]);
      }
    */
  /*dft_linprobmgr_writeMatrix(LinProbMgr_manager,filename,NULL,NULL);*/
    
    if (con_ptr != NULL) converged2 =
      continuation_hook_conwrap(x, delta_x, con_ptr, Newton_rel_tol, Newton_abs_tol);

    /* Do: x += delta_x, and check for convergence */
    converged = update_solution(x, delta_x, iter);
    if (converged) fix_symmetries(x);




  } while (iter < Max_Newton_iter && (!converged || !converged2));

  if (!converged || !converged2) {
    if (Proc==0 && Iwrite!=NO_SCREEN) printf("\tNewton Solver: Failed to converge in %d iterations\n",iter);
    iter = -iter;
  }
  else
    if (Proc==0 && Iwrite!=NO_SCREEN) printf("\tNewton Solver: Successful convergence in %d iterations\n",iter);

  safe_free((void **) &delta_x);
  return iter;
}

/****************************************************************************/
/****************************************************************************/
int update_solution(double** x, double** delta_x, int iter) {
/* Routine to update solution vector x using delta_x and 
 * to test for convergence of Newton's method.
 * 
 * Note: Modifications to Newton's method, including damping, have not yet
 *       been translated form previous update_solution.
 *       iter  value may be used in some damping methods
 */

  int iunk, ibox, inode,inodeG,ijk[3],go_update,idim;
  double updateNorm=0.0, temp,frac_min,frac;
  char *yo = "newupdate solution";
     


   /* Certain unknowns - specifically densities and Gs in CMS DFT cannot be less than 0.
      Here we locate problems, and scale the entire update vector to prevent this from 
      happening. */
  frac_min=1.0;
  for (ibox=0; ibox<Nnodes_box; ibox++) { /* find minimum update fraction in entire domain */
    for (iunk=0; iunk<Nunk_per_node; iunk++){
      if ( (Unk2Phys[iunk]==G_CHAIN  && Pol_Sym[iunk-Phys2Unk_first[G_CHAIN]] == -1) ||
           (Unk2Phys[iunk]==DENSITY && (!(Type_poly==WTC) || (Pol_Sym_Seg[iunk-Phys2Unk_first[DENSITY]] ==-1) )) ){
         if(x[iunk][ibox]+delta_x[iunk][ibox]<0.0){
             frac = AZ_MIN(1.0,x[iunk][ibox]/(-delta_x[iunk][ibox]));
             frac = AZ_MAX(frac,Min_update_frac);
         } 
         else{
             frac=1.0;
         }

         if (frac<frac_min) frac_min=frac;
      }
    }
  }

  frac_min=gmin_double(frac_min);
  if (Proc==0 && Iwrite != NO_SCREEN)
      printf("\tUPDATE FRAC = %g percent\n",frac_min*100);

  for (ibox=0; ibox<Nnodes_box; ibox++) {

    /* Increment updateNorm only for owned nodes (inode=-1 for ghosts) */
    inode = B2L_node[ibox];
    if (inode != -1) {
      for (iunk=0; iunk<Nunk_per_node; iunk++) {
        temp =(frac_min*delta_x[iunk][ibox])/(Newton_rel_tol*x[iunk][ibox] + Newton_abs_tol);
        updateNorm +=  temp*temp;
      }
    }

  /* For some cases, we need to be able to keep the solution values at the boundaries constant
     and set equal to the values that are read in from a file.  Do not update the solution 
     vector at these points */
    inodeG=L2G_node[inode]; 
    node_to_ijk(inodeG,ijk);
    go_update=TRUE;
    for (idim=0; idim<Ndim;idim++){
       if (  (ijk[idim]==0 && Type_bc[idim][0] == LAST_NODE_RESTART) ||
          (ijk[idim]==Nodes_x[idim]-1 && Type_bc[idim][1] == LAST_NODE_RESTART)) go_update=FALSE;
    }
   
    /* Update all solution componenets */
    if (go_update){
    for (iunk=0; iunk<Nunk_per_node; iunk++){
      if ((  (Unk2Phys[iunk]==DENSITY && 
                  (!(Type_poly==WTC) || (Pol_Sym_Seg[iunk-Phys2Unk_first[DENSITY]] ==-1) )) || 
            (Unk2Phys[iunk]==G_CHAIN && Pol_Sym[iunk-Phys2Unk_first[G_CHAIN]] == -1) || 
            Unk2Phys[iunk]==CMS_FIELD || Unk2Phys[iunk]==WJDC_FIELD ||
            (Unk2Phys[iunk]==BONDWTC  && Pol_Sym[iunk-Phys2Unk_first[BONDWTC]] == -1 )|| 
             Unk2Phys[iunk]==CAVWTC) && 
            x[iunk][ibox]+frac_min*delta_x[iunk][ibox] <1.e-15){

            x[iunk][ibox]=0.1*x[iunk][ibox];
      }
      else if ((iunk==Phys2Unk_first[HSRHOBAR] || iunk==(Phys2Unk_first[CAVWTC]+1)) && 
              x[iunk][ibox]+frac_min*delta_x[iunk][ibox] > 1.0){
              x[iunk][ibox]+=0.5*(1.0-x[iunk][ibox]);
      }
      else{
         x[iunk][ibox] += frac_min*delta_x[iunk][ibox];
      }
    }
    }
  }

  updateNorm = sqrt(gsum_double(updateNorm));

  if (Proc==0 && Iwrite != NO_SCREEN)
    printf("\n\t\t%s: Weighted norm of update vector =  %g\n", yo, updateNorm);

  if (updateNorm > 1.0) return(FALSE);
  else                  return(TRUE);

}
/*****************************************************************************************************/
void fix_symmetries(double **x)
{
  int ibox,iunk;
  for (ibox=0; ibox<Nnodes_box; ibox++) {
    for (iunk=0; iunk<Nunk_per_node; iunk++){

      if (Lseg_densities && Unk2Phys[iunk]==DENSITY && Pol_Sym_Seg[iunk-Phys2Unk_first[DENSITY]] != -1){
         x[iunk][ibox] = x[Phys2Unk_first[DENSITY]+Pol_Sym_Seg[iunk-Phys2Unk_first[DENSITY]]][ibox];
      }
      else if (Lseg_densities && Unk2Phys[iunk]==BONDWTC && Pol_Sym[iunk-Phys2Unk_first[BONDWTC]] != -1){
         x[iunk][ibox] = x[Phys2Unk_first[BONDWTC]+Pol_Sym[iunk-Phys2Unk_first[BONDWTC]]][ibox];
      }
      else if (!Lseg_densities && Unk2Phys[iunk]==G_CHAIN && Pol_Sym[iunk-Phys2Unk_first[G_CHAIN]] != -1){
         x[iunk][ibox] = x[Phys2Unk_first[G_CHAIN]+Pol_Sym[iunk-Phys2Unk_first[G_CHAIN]]][ibox];
      }

    }
  }
return;
}
/*****************************************************************************************************/
#ifdef NUMERICAL_JACOBIAN
void do_numerical_jacobian(double **x)
/* This routine compares the analytic and numerical jacobians for the     */
/* purpose of checking the accuracy of an analytic Jacobian. It is only   */
/* written for serial runs and will be slow, so only for small problems.  */
/* Three files are created: ja, jn, jd  containing the analytic jacobian, */
/* numerical jacobian, and the difference between them, printed as the    */
/* row, column, and value.                                                */
{
  double **full=NULL, del;
  int i, j, N=Nunk_per_node*Nnodes, count=0,iunk,junk,jnode,inode,c,read_next;
  char filename[20];
  FILE *ifp, *ifp2, *ifp3;
  int ia,ja,count_same=0,count_diff=0;
  double coef_ij,diff,error,fac;
  double **resid, **resid_tmp;
  resid = (double **) array_alloc(2, Nunk_per_node, Nnodes_per_proc, sizeof(double));
  resid_tmp = (double **) array_alloc(2, Nunk_per_node, Nnodes_per_proc, sizeof(double));

  dft_linprobmgr_getrhs(LinProbMgr_manager, resid);

  /* print out analytic matrix in MSR format */
  sprintf(filename, "ja%0d",Proc);
  dft_linprobmgr_writeMatrix(LinProbMgr_manager,filename,NULL,NULL);

  /* compute numerical jacobian by repeated residual fill calls */

  full = (double **) array_alloc(2,N,N,sizeof(double));
  if (full==NULL){printf("Not enough memory for full numerical jacobian\n"); exit(-1);}

  for (iunk=0;iunk<Nunk_per_node;iunk++){
    for (inode=0; inode<Nnodes; inode++) {
      i=inode+Nnodes*iunk; /* Physics Based Ordering */
      /*i=iunk+Nunk_per_node*inode;*/  /* Nodal Based Ordering */
/*      del=1.e-6*fabs(x[iunk][inode])+1.e-12;*/

      if (fabs(x[iunk][inode]) > 1.e-2) fac=1.e-6;
      else if (fabs(x[iunk][inode]) > 1.e-3) fac=1.e-5;
      else if (fabs(x[iunk][inode]) > 1.e-5) fac=1.e-3;
      else if (fabs(x[iunk][inode]) > 1.e-7) fac=1.e-1;
      else fac=1.0;

      del=fac*fabs(x[iunk][inode]);
      if (del<1.e-08) del+= 1.e-08;
      x[iunk][inode] += del;

      for (junk=0; junk<Nunk_per_node; junk++) 
          for (jnode=0; jnode<Nnodes_per_proc; jnode++) resid_tmp[junk][jnode] = 0.0;

      (void) dft_linprobmgr_initializeproblemvalues(LinProbMgr_manager);
      fill_resid_and_matrix_control(x,1,TRUE);
      dft_linprobmgr_getrhs(LinProbMgr_manager, resid_tmp);

      for (junk=0; junk<Nunk_per_node; junk++){ 
      for (jnode=0; jnode<Nnodes_per_proc; jnode++){ 
          j=jnode+Nnodes*junk; /* Physics Based Ordering */
          /*j=junk+Nunk_per_node*jnode;*/  /* Nodal Based Ordering */
          full[j][i] = (resid[junk][jnode] - resid_tmp[junk][jnode])/del;
/*if (junk==0 && iunk==3 && jnode==10 && inode==0){
   printf("del=%g  x_shift=%g  x=%g  resid_tmp=%g  resid=%g  full=%g\n",
           del,x[iunk][inode],x[iunk][inode]-del,resid_tmp[junk][jnode],resid[junk][jnode],full[j][i]);
}*/
      }}
      x[iunk][inode] -= del;
      if (count==100 || i==N-1){
        if (Iwrite != NO_SCREEN)printf("Proc: %d :: ith row=%d, out of %d rows.\n",Proc,i,N-1);
        count=0;
      } count++;
    }
  }
  if (Iwrite != NO_SCREEN)printf("Proc: %d finished computing numerical jacobian !!\n",Proc);

  /* print out nonzero entries of numerical jacobian */
  sprintf(filename, "jn%0d",Proc);
  ifp = fopen(filename,"w");

  sprintf(filename, "ja%0d",Proc);
  ifp2 = fopen(filename,"r");
  for (i=0;i<2;i++) while ((c=getc(ifp2)) != EOF && c !='\n') ; /* skip first two lines of ja0 */


  sprintf(filename, "jdiff%0d",Proc);
  ifp3 = fopen(filename,"w");

  read_next=TRUE;
  for (i=0; i<Nnodes*Nunk_per_node; i++) {
    for (j=0; j<Nnodes*Nunk_per_node; j++) {
      if (read_next){
            fscanf(ifp2,"%d %d %lf",&ia,&ja,&coef_ij);
            read_next=FALSE;
      }
      if (ia-1==i && ja-1==j){
          diff=fabs((full[i][j]-coef_ij));
          error=100*fabs((full[i][j]-coef_ij)/full[i][j]);
          if (diff > 0.001 && error>1.){ 
               fprintf(ifp3,"%d  (node=%d iunk=%d) |  %d (node=%d iunk=%d) | diff=%g | error=%g %\n",
               i,i-Nnodes*(int)(i/Nnodes),i/Nnodes,j,j-Nnodes*(int)(j/Nnodes),j/Nnodes,diff,error);
               count_diff++;
          }
          else count_same++;
          read_next=TRUE;
      }
      else{
         if (fabs(full[i][j])>1.0e-6){
if (i==2) printf("i=%d j=%d  full[i][j]=%g\n",i,j,full[i][j]);
            printf("nonzero matrix coefficient only in numerical jacobian: i=%d  j=%d coefficient=%g\n",i,j,full[i][j]);
            diff=fabs((full[i][j]));
            fprintf(ifp3,"%d  (node=%d iunk=%d) |  %d (node=%d iunk=%d) | diff=%g | error=100%\n",
               i,i-Nnodes*(int)(i/Nnodes),i/Nnodes,j,j-Nnodes*(int)(j/Nnodes),j/Nnodes,diff);
            count_diff++;
         }
         else count_same++;
      }
      if (fabs(full[i][j]) > 1.0e-6) fprintf(ifp,"%d  %d   %g\n",i,j,full[i][j]);
    }
  }
  fclose(ifp);
  fclose(ifp2);
  fclose(ifp3);
  printf("numerical jacobian statistics:\n");
  printf("number of matrix coefficients that are the same=%d\n",count_same);
  printf("number of matrix coefficients that are different=%d\n",count_diff);
  printf("See jdiff0 for summary of matrix coefficients where differences\n");
  printf("between analytical and numerical results are greater than 1%. \n\n");
  printf("KILLING CODE AT END OF NUMERICAL JACOBIAN\n");
  exit(0);
}
#endif
/****************************************************************************/


/*****************************************************************************************************/
void print_resid_norm(int iter)
{
  int iunk, j;
  double norm=0.0;
  double **f;
  FILE *ifp;
  char filename[20]="Resid2.dat";

  if (Proc==0 && Iwrite==VERBOSE) ifp=fopen(filename,"w+");
  f = (double **) array_alloc(2, Nunk_per_node, Nnodes_per_proc, sizeof(double));
  dft_linprobmgr_getrhs(LinProbMgr_manager, f);


  for (j=0; j< Nnodes_per_proc; j++) {
    for (iunk=0; iunk<Nunk_per_node; iunk++) {
       norm += f[iunk][j] * f[iunk][j];
       if(Proc==0 && Iwrite==VERBOSE) fprintf(ifp," %d  %d  %14.11f  %14.11f\n",iunk,L2G_node[j],f[iunk][j],norm);
    }
  }
  if (Proc==0 && Iwrite==VERBOSE) fclose(ifp);

  safe_free((void **) &f);
  norm = gsum_double(norm);
  if (Proc==0) printf("\t\tResidual norm at iteration %d = %g\n",iter, sqrt(norm));
  return;
}
/*****************************************************************************************************/

void fill_test(double **x, int flag) 
/* Quick test problem for matrix loading, solve, and Newton's method */
/* For the first unknown at a node, x[0]=inode 
 * for subsequent unknowns at that node, x[iunk]^2=x[iunk-1]
 * So, when run at 3 unknowns per node, global node 16 should have
 * solutions 16, +-4, +-2, for the three unknowns at that node
 */
{
  int loc_inode, inode_box, iunk, inode;
  double f;

  for (loc_inode=0; loc_inode< Nnodes_per_proc; loc_inode++) {
    inode = L2G_node[loc_inode];
    inode_box = L2B_node[loc_inode];
    iunk = 0;

    f = x[iunk][inode_box] - inode - 1;
    dft_linprobmgr_insertrhsvalue(LinProbMgr_manager, iunk, loc_inode, -f);
    dft_linprobmgr_insertonematrixvalue(LinProbMgr_manager,iunk,loc_inode,iunk,inode_box,1.0);

    /* For iunk 1 and higher, the square of the unknow is the previous value */
    for (iunk=1; iunk<Nunk_per_node; iunk++) {
      f = x[iunk][inode_box]*x[iunk][inode_box] - x[iunk-1][inode_box];
      dft_linprobmgr_insertrhsvalue(LinProbMgr_manager, iunk, loc_inode, -f);
      dft_linprobmgr_insertonematrixvalue(LinProbMgr_manager,iunk,loc_inode,iunk,inode_box, 2*x[iunk][inode_box]);
      dft_linprobmgr_insertonematrixvalue(LinProbMgr_manager,iunk,loc_inode,iunk-1,inode_box, -1.0);
    }
  } 
}

