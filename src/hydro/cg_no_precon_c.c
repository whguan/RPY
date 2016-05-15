/*******************************************************************************
!
!  Content: Intel MKL RCI CG (Conjugate Gradient method) C example without
!           both preconditioner and user-defined stopping criteria
!
!*******************************************************************************/

#include <stdio.h>
#include "mkl_rci.h"
#include "mkl_blas.h"
#include "mkl_spblas.h"
#include "mkl_service.h"

/*---------------------------------------------------------------------------*/
/*  Example program for solving symmetric positive definite system of equations.*/
/*  Simplest case: no preconditioning and no the user-defined stopping tests.*/
/*---------------------------------------------------------------------------*/
int KrylovFMM(double *solution, double *ShellSphs, double *rhs, MKL_INT n)
{
  /*---------------------------------------------------------------------------*/
  /* Define arrays for the upper triangle of the coefficient matrix and rhs vector */
  /* Compressed sparse row storage is used for sparse representation           */
  /*---------------------------------------------------------------------------*/
  MKL_INT rci_request, itercount, expected_itercount = 20, i;
  
  /*---------------------------------------------------------------------------*/
 // double expected_sol[8] = { 1.E0, 0.E0, 1.E0, 0.E0, 1.E0, 0.E0, 1.E0, 0.E0 };
  /*---------------------------------------------------------------------------*/
  /* Some additional variables to use with the RCI (P)CG solver                */
  /*---------------------------------------------------------------------------*/
  MKL_INT ipar[128];
  double euclidean_norm, dpar[128] ;
  double eone = -1.E0;
  MKL_INT ione = 1;
  double *tmp;
  tmp = (double*)calloc(4*n, sizeof(double));

  /*---------------------------------------------------------------------------*/
  /* Initialize the initial guess                                              */
  /*---------------------------------------------------------------------------*/
  for (i = 0; i < n; i++)
    solution[i] = 1.E0;
  /*---------------------------------------------------------------------------*/
  /* Initialize the solver                                                     */
  /*---------------------------------------------------------------------------*/
  dcg_init (&n, solution, rhs, &rci_request, &ipar, &dpar, tmp);
  if (rci_request != 0)
    goto failure;
  /*---------------------------------------------------------------------------*/
  /* Set the desired parameters:                                               */
  /* LOGICAL parameters:                                                       */
  /* do residual stopping test                                                 */
  /* do not request for the user defined stopping test                         */
  /* DOUBLE parameters                                                         */
  /* set the relative tolerance to 1.0D-5 instead of default value 1.0D-6      */
  /*---------------------------------------------------------------------------*/
  ipar[8] = 1;
  ipar[9] = 0;
  dpar[0] = 1.E-5;
  /*---------------------------------------------------------------------------*/
  /* Check the correctness and consistency of the newly set parameters         */
  /*---------------------------------------------------------------------------*/
  dcg_check (&n, solution, rhs, &rci_request, ipar, dpar, tmp);
  if (rci_request != 0)
    goto failure;
  /*---------------------------------------------------------------------------*/
  /* Compute the solution by RCI (P)CG solver without preconditioning          */
  /* Reverse Communications starts here                                        */
  /*---------------------------------------------------------------------------*/
rci:dcg (&n, solution, rhs, &rci_request, ipar, dpar, tmp);
  /*---------------------------------------------------------------------------*/
  /* If rci_request=0, then the solution was found with the required precision */
  /*---------------------------------------------------------------------------*/
  if (rci_request == 0)
    goto getsln;
  /*---------------------------------------------------------------------------*/
  /* If rci_request=1, then compute the vector A*tmp[0]                        */
  /* and put the result in vector tmp[n]                                       */
  /*---------------------------------------------------------------------------*/
  if (rci_request == 1)
    {
      //mkl_dcsrsymv (&tr, &n, a, ia, ja, tmp, &tmp[n]);
	//Note : charge = tmp[0:n] 
	// RPY(int nparts, double *ploc, double *pcharge, double *RPY)
        RPY(n/3, ShellSphs, tmp, &tmp[n]); // SPMV by 4 calls of  FMM
 	goto rci;
    }
  /*---------------------------------------------------------------------------*/
  /* If rci_request=anything else, then dcg subroutine failed                  */
  /* to compute the solution vector: solution[n]                               */
  /*---------------------------------------------------------------------------*/
  goto failure;
  /*---------------------------------------------------------------------------*/
  /* Reverse Communication ends here                                           */
  /* Get the current iteration number into itercount                           */
  /*---------------------------------------------------------------------------*/
getsln:dcg_get (&n, solution, rhs, &rci_request, &ipar[0], &dpar[0], tmp, &itercount);
  /*---------------------------------------------------------------------------*/
  /* Print solution vector: solution[n] and number of iterations: itercount    */
  /*---------------------------------------------------------------------------*/
  printf ("The system has been solved\n");
  printf ("The following solution obtained\n");
  for (i = 0; i < n / 2; i++)
    printf ("%6.3f  ", solution[i]);
  printf ("\n");
  for (i = n / 2; i < n; i++)
    printf ("%6.3f  ", solution[i]);
/*  printf ("\nExpected solution is\n");
  for (i = 0; i < n / 2; i++)
    {
      printf ("%6.3f  ", expected_sol[i]);
      expected_sol[i] -= solution[i];
    }
  printf ("\n");
  for (i = n / 2; i < n; i++)
    {
      printf ("%6.3f  ", expected_sol[i]);
      expected_sol[i] -= solution[i];
    }
  */  
  printf ("\nNumber of iterations: %d\n", itercount);
 // i = 1;
 // euclidean_norm = dnrm2 (&n, expected_sol, &i);

  /*-------------------------------------------------------------------------*/
  /* Release internal MKL memory that might be used for computations         */
  /* NOTE: It is important to call the routine below to avoid memory leaks   */
  /* unless you disable MKL Memory Manager                                   */
  /*-------------------------------------------------------------------------*/
  MKL_Free_Buffers ();

  if (itercount <= expected_itercount)// && euclidean_norm < 1.0e-12)
    {
      printf ("This example has successfully PASSED through all steps of computation!\n");
      return 0;
    }
  else
    {
      printf ("This example may have FAILED as either the number of iterations differs\n");
      printf ("from the expected number of iterations %d, or the ", expected_itercount);
      printf ("computed solution\ndiffers much from the expected solution ");
     // printf ("(Euclidean norm is %e), or both.\n", euclidean_norm);
      return 1;
    }
  /*-------------------------------------------------------------------------*/
  /* Release internal MKL memory that might be used for computations         */
  /* NOTE: It is important to call the routine below to avoid memory leaks   */
  /* unless you disable MKL Memory Manager                                   */
  /*-------------------------------------------------------------------------*/
failure:printf ("This example FAILED as the solver has returned the ERROR code %d", rci_request);
  MKL_Free_Buffers ();
  free(tmp);
  return 1;
}
