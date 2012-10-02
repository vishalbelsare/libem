/*********************************************************************************
# Copyright (c) 2012, CyberPoint International, LLC
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the CyberPoint International, LLC nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL CYBERPOINT INTERNATIONAL, LLC BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************/


/*! \file EM_Algorithm.h
*   \brief Function prototypes for EM algorithm routines
*/
#ifndef EM_AGORITHM_H
#define EM_ALGORITHM_H

/* EXPECTATION MAXIMIZATION ALGORITHM
   Written by Elizabeth Garbee, Summer 2012 */

#include "Matrix.h"

using namespace std;

/************************************************************************************************
** EM FUNCTION DECLARATIONS **
************************************************************************************************/

double estep(int n, int m, int k, double *X,  Matrix &p_nk_matrix, vector<Matrix *> &sigma_matrix, Matrix &mu_matrix, Matrix &Pk_matrix);

bool mstep(int n, int m, int k, double *X, Matrix &p_nk_matrix, Matrix *sigma_matrix, Matrix &mu_matrix, Matrix &Pk_matrix);

void EM(int n, int m, int k, double *X, vector<Matrix*> &sigma_matrix, Matrix &mu_matrix, Matrix &Pks);

int ParseCSV(char *file_name, double *data, int n, int m);

#endif //EM_ALGORITHM_HEADER

