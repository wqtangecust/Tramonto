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

/* ----------------------------------------------------------------------
dft_thermo_wjdc.c:
Calculate the thermodynamic properties of chain contributions for a 
Wertheim-JD-Chapman bonded fluid.  Note that we use the routine WTC_thermo_precalc
that is found in the file dft_thermo_wtc.c to compute bulk segment and cavity
function variables .
-------------------------------------------------------------------------*/
#include "dft_thermo_wjdc.h"

/****************************************************************************/
/* WJDC_thermo_precalc: call all routines needed to process bulk properties of
                          WJDC functionals */
void WJDC_thermo_precalc(char *output_file1)
{ 
  compute_bulk_nonlocal_wjdc_properties(output_file1);
  return;
}
/*******************************************************************************/
/* compute_bulk_nonlocal_wjdc_properties: compute some additional bulk properties we
   need to carry around the calculation. */
void compute_bulk_nonlocal_wjdc_properties(char *output_file1)
{
  int i,loc_inode,loc_i,inode_box,inode,ijk[3],icomp,jcomp,idim,iunk,printproc;
  int ibond,jbond,index,iseg,jseg,pol_num,bond_num,type_jseg,nloop,iloop;
  int array_val[NMER_MAX*NBOND_MAX],array_fill,count_fill,test,power;
  double vol,area,x_dist,field,sten_sum[4];
  double field_hs,field_att,field_chain;
  FILE *fp2=NULL;

  if (Proc==0 && output_file1 !=NULL) printproc = TRUE;
  else printproc=FALSE;
  if (printproc) {
    if( (fp2 = fopen(output_file1,"a+"))==NULL) {
      printf("Can't open file %s\n", output_file1);
      exit(1);
    }
  }

  /* (1) compute bulk field variable -- note the variable explicitly solved in the residual
     equations is exp(D-beta*Vext) or exp(D) in the bulk. */

  for (iseg=0;iseg<Nseg_tot;iseg++){
    field=0.0;
    icomp=Unk2Comp[iseg];

  /* First include the Hard Sphere parts */
     sten_sum[0]=1.;      
     sten_sum[1]= HS_diam[icomp]/2.;      
     sten_sum[2]=(4.*PI)*POW_DOUBLE_INT(HS_diam[icomp]/2.,2);      
     sten_sum[3]=(4.*PI/3.)*POW_DOUBLE_INT(HS_diam[icomp]/2.,3);       
     for (i=0;i<4;i++){         /* summing over terms in the FMT hs functional */
        field -= Dphi_Drhobar_b[i]*sten_sum[i]*Fac_overlap_hs[icomp];      
     }

  /* Now include Attractions */
    if (Type_attr != NONE){
       for (jcomp=0; jcomp<Ncomp;jcomp++){
          field -= Avdw[icomp][jcomp]*Rho_b[jcomp];
       }
    }

  /* Now include Coulomb parts */
    /*do this later*/

  /* Now include Bonding terms - note that this is identical to WTC theory */
     field += chain_term(iseg,icomp,Rho_seg_b);

     Field_WJDC_b[icomp]=exp(field);
     if(printproc) fprintf(fp2,"iseg=%d field=%9.6f FIELD_WJDC=%9.6f\n",iseg,field,Field_WJDC_b[icomp]);
  } /* end of bulk field calculations */

  /* (2) compute bulk G - chain propogator values.  Note that we need to start at the ends of 
     the chains (linear or branched and work toward the opposite end of the chain carefully. */

  for (ibond=0;ibond<Nbonds;ibond++) array_val[ibond]=FALSE;
  array_fill=FALSE;
  count_fill=0;

  while (array_fill==FALSE){
     for (ibond=0;ibond<Nbonds;ibond++){
        iseg=Unk_to_Seg[ibond];
        icomp=Unk2Comp[iseg];
        pol_num=Unk_to_Poly[ibond];
        bond_num=Unk_to_Bond[ibond];
        if (array_val[ibond]==FALSE){
           test=TRUE;  /* assume we will compute a bulk G */
           jseg=Bonds[pol_num][iseg][bond_num];
           if (jseg != -1 ){   /* may need to skip this G if we don't have all information yet  -
                                  always compute G for end segments flagged with -1 value */
              for (jbond=0;jbond<Nbond[pol_num][jseg];jbond++){
                 if (Bonds[pol_num][jseg][jbond] != iseg){ /* check all jbonds to see if we have necessary info */
                    index=Poly_to_Unk[pol_num][jseg][jbond];
                    if (array_val[index]==FALSE) test=FALSE;
                 }
              }
           }
           if (test==TRUE){     /* compute a bulk G */
              if (jseg == -1){
                  G_WJDC_b[ibond]=Field_WJDC_b[icomp]; /* end segment is simple */
              }
              else{
                  icomp=Unk2Comp[iseg];
                  jcomp=Unk2Comp[jseg];
                  G_WJDC_b[ibond]=Field_WJDC_b[icomp]*
                                  y_cav(Sigma_ff[icomp][icomp],Sigma_ff[jcomp][jcomp],Xi_cav_b[2],Xi_cav_b[3]);
           
                  for (jbond=0;jbond<Nbond[pol_num][jseg];jbond++){
                     if (Bonds[pol_num][jseg][jbond] != iseg){ 
                          G_WJDC_b[ibond]*=G_WJDC_b[Poly_to_Unk[pol_num][jseg][jbond]];
                     }
                  }
                  power=-(Nbond[pol_num][jseg]-2); /* this is 0 for a linear chain for all interal segments */
                  if (power != 0){
                         G_WJDC_b[ibond]*=POW_DOUBLE_INT(Field_WJDC_b[jcomp],power);
                  }
              }
              count_fill++;
              array_val[ibond]=TRUE;
              if (printproc)  fprintf(fp2,"ibond=%d  G_b=%9.6f\n",ibond,G_WJDC_b[ibond]);
           }
        }
     }
     if (count_fill==Nbonds) array_fill=TRUE;
  }

  if (printproc) fclose(fp2);

  return;
}
/*********************************************************************************************/
/*chempot_chain_wjdc- Here compute "Chain" chemical potentials for use with WJDC functionals.  */
void chempot_chain_wjdc(double *rho,double *betamu_chain)
{
   int iseg,ibond,unk_G,pol_num,printproc,icomp;
   double mu_chain,gproduct;
 
   if (Proc==0 && Iwrite==VERBOSE) printproc = TRUE;
   else printproc=FALSE;
    
   if (printproc){
         printf("chain chemical potentials are printed for every segment...\n");
         printf("they should be identical for every segment on a given chain ... \n");
   }

   for (iseg=0;iseg<Nseg_tot;iseg++){
      icomp=Unk2Comp[iseg];
      mu_chain=0.0;

      /* density term */
      mu_chain += log(rho[iseg]);

      /* field term */
      mu_chain += log(Field_WJDC_b[icomp]);

      /* bonding term */
      gproduct=1.0;
      for (ibond=0;ibond<Nbonds_SegAll[iseg];ibond++){
             unk_G=Poly_to_Unk_SegAll[iseg][ibond];
             pol_num=Unk_to_Poly[unk_G];
             gproduct *=G_WJDC_b[unk_G];
      }
      mu_chain -= log(gproduct);
      if (printproc) printf("iseg=%d pol_num=%d  mu_chain=%9.6f\n",iseg,pol_num,mu_chain);
      betamu_chain[pol_num]=mu_chain;
   } 
   return;
}
/*********************************************************************************************/
