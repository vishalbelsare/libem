/*********************************************************************************************************/

       /* EXPECTATION MAXIMIZATION ALGORITHM (code library)
	CYBERPOINT INTERNATIONAL, LLC
	Written by Elizabeth Garbee, Summer 2012 */

/**********************************************************************************************************/

//Support function declarations - see code for detailed descriptions. You don't need to really pay attention to these, as they are 
//the pieces that work behind the scenes for Kmeans.
int ParseCSV(char *file_name, double *data, int n, int m);
double euclid_distance(int m, double *p1, double *p2);
void all_distances(int m, int n, int k, double *X, double *centroid, double *distance_out);
double calc_total_distance(int m, int n, int k, double *X, double *centroids, int *cluster_assignment_index);
void choose_all_clusters_from_distances(int m, int n, int k, double *X, double *distance_array, int *cluster_assignment_index);
void calc_cluster_centroids(int m, int n, int k, double *X, int *cluster_assignment_index, double *new_cluster_centroid);
void get_cluster_member_count(int n, int k, int *cluster_assignment_index, int *cluster_member_count);
void deprecated_update_delta_score_table(int m, int n, int k, double *X, int *cluster_assignment_cur, double *cluster_centroid, int *cluster_member_count, double*point_move_score_table, int cc);
void perform_move (int m, int n, int k, double *X, int *cluster_assignment, double *cluster_centroid, int *cluster_member_count, int move_point, int move_target_cluster);
void cluster_diag(int m, int n, int k, double *X, int *cluster_assignment_index, double *cluster_centroid);
void copy_assignment_array(int n, int *src, int *tgt);
int assignment_change_count (int n, int a[], int b[]);

//Kmeans function declaration
double * kmeans(int dim, double *X, int n, int k);

//headers
#include <iostream>
#include <ostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <vector>
#include <istream>
#include <list>
#include <numeric>
#include <functional>
#include <algorithm>
#include "time.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// EM specific header
#include "EM_Algorithm.h"

//#define statements - change #debug to 1 if you want to see EM's calculations as it goes
#define sqr(x) ((x)*(x))
#define MAX_CLUSTERS 5
#define MAX_ITERATIONS 100
#define BIG_double (INFINITY)
#define debug 0
#define MAX_LINE_SIZE 1000

using namespace std;

int main(int argc, char *argv[])
{
	int i, k, m, n;
	if (argc != 4) cout << " Usage: <exec_name> <data_file> <num_dimensions> <num_data_points> <num_clusters>" << endl;
	int errno = 0;

	sscanf(argv[2],"%d", &m);
	sscanf(argv[3],"%d", &n);
	sscanf(argv[4],"%d", &k);

	//error checking
	if (errno != 0) 
	{
		cout << "Invalid inputs" << endl;
	}

	//reading in and parsing the data
	double * data = new double[n*m];
	ParseCSV(argv[1], data, n, m);
	if (ParseCSV(argv[1], data, n, m) != 1)
	{
		
		return 0;
	}
	
	// create vectors that hold pointers to the EM result matrices
	vector<Matrix *> sigma_vector;
	for (i = 0; i < k; i++)
	{
		Matrix*p = new Matrix(m,m);
		sigma_vector.push_back(p);
	}

	//create matrices for EM to fill
	Matrix mu_local(k,m);
	Matrix Pk_matrix(1,k);

	// run the EM function
	EM(n, m, k, data, sigma_vector, mu_local, Pk_matrix);

	//print out results
	cout << "The matrix of mu's approximated by the EM algorithm is " << endl;
	mu_local.print();
	cout << "The matrix of Pk's approximated by the EM algorithm is " << endl;
	Pk_matrix.print();
	
	for (i = 0; i < k; i++)
	{
		cout << "The " << i << " -th covariance matrix approximated by the EM algorithm is " << endl;
		sigma_vector[i]->print();
		delete sigma_vector[i];
		
	}
	delete[] data;
	mu_local.clear();
	Pk_matrix.clear();
	if (debug) cout << "I got to the end of main just fine." << endl;
}

/*************************************************************************************************************/
/** SUPPORT FUNCTIONS **
**************************************************************************************************************/

/*
	ParseCSV is a function that takes in your comma delineated data and parses it according to parameters given at the command line (see the README). 
	This is how you first define the three crucial parameters for the Kmeans and EM approximations:
		m - integer representing the dimensionality of the data
		n - number of data points
		k - how many clusters you want Kmeans to find
	return 1 for success, zero for error
*/

int ParseCSV(char *file_name, double *data, int n, int m)
{
	char buffer[MAX_LINE_SIZE];
	FILE *f = fopen(file_name, "r");
	if (f == NULL) 
	{
		cout << "Could not open file" << endl;
		return 0;
	}
	memset(buffer, 0, MAX_LINE_SIZE);
	int row = 0;
	int cols = 0;
	while (fgets(buffer,MAX_LINE_SIZE,f) != NULL)
	{
		if (buffer[MAX_LINE_SIZE - 1] != 0) 
		{
			cout << "Max line size exceeded at zero relative line " << row << endl;
			return 0;
		}

		int errno = 0;
		char *ptok = strtok(buffer, ",");
		if (ptok) sscanf(ptok, "%lf", &data[row*m]);
		if (errno != 0)
		{
			cout << "Could not convert data at index " << row << " and " << cols << endl;
			return 0;
		}
		
		for (cols = 1; cols < m; cols++)
		{
			sscanf(strtok(NULL, ","), "%lf", &data[row*m + cols]);
			
			if (errno != 0) 
			{
				cout << "Could not convert data at index " << row << " and " << cols << endl;
				return 0;
			}
		}
		
		row++;
		memset(buffer, 0, MAX_LINE_SIZE);
	}
	return 1;
}


/* 
	euclid_distance is a function that does just that - it calculates the euclidean distance between two points. This
	is the method used to assign data points to clusters in kmeans; the aim is to assign each point to the "closest" cluster
	centroid.
		input - the dimensionality of the data, and two double*s representing point one and point 2
		output - double which stores the distance
*/

double euclid_distance(int m, double *p1, double *p2)
{	
	double distance_sum = 0;
	for (int ii = 0; ii < m; ii++)
		distance_sum += pow(p1[ii] - p2[ii],2);
	return sqrt(distance_sum);
	if (debug) cout << "this iteration's distance sum is " << distance_sum << endl;
}

/* 
	all_distances calculates distances from the centroids you initialized to every data point.
		input - dimensionality, number of data points, number of clusters, double*s containing your data, cluster centroids and the distance calculation
		output - void 
*/

void all_distances(int m, int n, int k, double *X, double *centroid, double *distance_out)
{
	for (int ii = 0; ii < n; ii++) // for each data point
	{
		for (int jj = 0; jj < k; jj++) // for each cluster
		{
			distance_out[ii*k + jj] = euclid_distance(m, &X[ii*m], &centroid[jj*m]);
		}
	}
}

/* 
	calc_total_distance computes the total distance from all their respective data points for all the clusters
	you initialized. This function also initializes the array that serves as the index of cluster assignments 
	for each point (i.e. which cluster each point "belongs" to on this iteration).
		input - double*s containing your data, initial centroids
		output - double 
	point with a cluster assignment of -1 is ignored
*/

double calc_total_distance(int m, int n, int k, double *X, double *centroids, int *cluster_assignment_index)
{
	double tot_D = 0;
	for (int ii = 0; ii < n; ii++) // for each data point
	{
		int active_cluster = cluster_assignment_index[ii]; // which cluster it's in
		// sum distance
		if (active_cluster != -1) 
			tot_D += euclid_distance(m, &X[ii*m], &centroids[active_cluster*m]);
	}
	return tot_D;
}

/* 
	choose_all_clusters_from_distances is the function that reassigns clusters based on distance to data points - this
	is the piece that smooshes clusters around to keep minimizing the distance between clusters and their data.
		input - data, the array that holds the distances, and the assignment index
		output - void 
*/

void choose_all_clusters_from_distances(int m, int n, int k, double *X, double *distance_array, int *cluster_assignment_index)
{
	for (int ii = 0; ii < n; ii++) // for each data point
	{
		int best_index = -1;
		double closest_distance = -1;

		// for each cluster
		for (int jj = 0; jj < k; jj++)
		{
			// distance between point and centroid
			double cur_distance = distance_array[ii*k + jj];
			if ((closest_distance < 0) || (cur_distance < closest_distance))
			{
				best_index = jj;
				closest_distance = cur_distance;
			}
		}
		// store in the array
		cluster_assignment_index[ii] = best_index;
	}
}

/* 
	calc_cluster_centroids is the function that actually recalculates the values for centroids based on their reassignment, in order
	to ensure that the cluster centroids are still the means of the data that belong to them. This is also where the double* that
	holds the new cluster centroids is assigned and filled in.
		input - data, assignment index
		output - void
*/

void calc_cluster_centroids(int m, int n, int k, double *X, int *cluster_assignment_index, double *new_cluster_centroid)
{
	for (int b = 0; b < k; b++)
		if (debug) printf("\n%f\n", new_cluster_centroid[b]);
	int cluster_member_count[k];
	// initialize cluster centroid coordinate sums to zero
	for (int ii = 0; ii < k; ii++)
	{
		for (int jj = 0; jj < m; jj++)
		{
			new_cluster_centroid[ii*m + jj] = 0;
		}
	}
	// sum all points for every point
	for (int ii = 0; ii < n; ii++)
	{
		// which cluster it's in
		int active_cluster = cluster_assignment_index[ii];
		// sum point coordinates for finding centroid
		for (int jj = 0; jj < m; jj++)
			new_cluster_centroid[active_cluster*m + jj] += X[ii*m + jj];
	}
	// divide each coordinate sum by number of members to find mean(centroid) for each cluster
	for (int ii = 0; ii < k; ii++)
	{
		get_cluster_member_count(n, k, cluster_assignment_index, cluster_member_count);
		if (cluster_member_count[ii] == 0)
			cout << "Warning! Empty cluster. \n" << ii << endl;
		// for each dimension
		for (int jj = 0; jj < m; jj++)
			new_cluster_centroid[ii*m + jj] /= cluster_member_count[ii];
			// warning!! will divide by zero here for any empty clusters
	}
}

/* 
	get_cluster_member_count takes the newly computed cluster centroids and basically takes a survey of how
	many points belong to each cluster. This is where the int* representing the number of data points for 
	every cluster is initialized and filled in.
		input - assignment index
		output - void 
*/
void get_cluster_member_count(int n, int k, int *cluster_assignment_index, int * cluster_member_count)
{
	// initialize cluster member counts
	for (int ii = 0; ii < k; ii++)
		cluster_member_count[ii] = 0;
	// count members of each cluster
	for (int ii = 0; ii < n; ii++)
		cluster_member_count[cluster_assignment_index[ii]]++;

}

/* 
	deprecated_update_delta_score_table is the first step in reassigning points to the clusters that are now closest to them - it basically
	creates a table of which clusters need to be moved and fills in that table. Not all points will need to be reassigned after
	each iteration, so this function keeps track of the ones that do. 
		input - data, current cluster assignment, current cluster centroids, member count
		output - void 
*/
void deprecated_update_delta_score_table(int m, int n, int k, double *X, int *cluster_assignment_cur, double *cluster_centroid, int *cluster_member_count, double *point_move_score_table, int cc)
{
	// for each point both in and not in the cluster
	for (int ii = 0; ii < n; ii++)
	{
		double dist_sum = 0;
		for (int kk = 0; kk < m; kk++)
		{
			double axis_dist = X[ii*m + kk] - cluster_centroid[cc*m + kk];
			dist_sum += sqr(axis_dist);
		}
		double mult = ((double)cluster_member_count[cc] / (cluster_member_count[cc] + ((cluster_assignment_cur[ii]==cc) ? -1 : +1)));
		point_move_score_table[ii*m + cc] = dist_sum * mult;
	}
}

/* 
	perform_move is the piece that actually smooshes around the clusters - and yes, smooshes is a technical term.
		input - data, cluster assignments, cluster centroids, and member counts
		output - void 
*/

void perform_move (int m, int n, int k, double *X, int *cluster_assignment, double *cluster_centroid, int *cluster_member_count, int move_point, int move_target_cluster)
{
	int cluster_old = cluster_assignment[move_point];
	int cluster_new = move_target_cluster;
	// update cluster assignment array
	cluster_assignment[move_point] = cluster_new;
	// update cluster count array
	cluster_member_count[cluster_old]--;
	cluster_member_count[cluster_new]++;

	if (cluster_member_count[cluster_old] <= 1)
		cout << "Warning! can't handle single-member clusters \n" << endl;
	// update centroid array
	for (int ii = 0; ii < m; ii++)
	{	
		cluster_centroid[cluster_old*m + ii] -= (X[move_point*m + ii] - cluster_centroid[cluster_old*m + ii]) / cluster_member_count[cluster_old];
		cluster_centroid[cluster_new*m + ii] += (X[move_point*m + ii] - cluster_centroid[cluster_new*m + ii]) / cluster_member_count[cluster_new];
	}
}

/* 
	cluster_diag gets the current cluster member count and centroids and prints them out for the user after each iteration.
		input - data, assignment index, centroids
		output - void 
*/

void cluster_diag(int m, int n, int k, double *X, int *cluster_assignment_index, double *cluster_centroid)
{
	int cluster_member_count[MAX_CLUSTERS];
	get_cluster_member_count(n, k, cluster_assignment_index, cluster_member_count);
	if (debug) cout << "  Final clusters \n" << endl;
	for (int ii = 0; ii < k; ii++)
	{
		if (debug) printf("cluster %d:  members: %8d\n", ii, cluster_member_count[ii]);
		printf(" ( ");
		for (int jj = 0; jj < m; jj++)
		{
			printf("%lf ",cluster_centroid[ii*m + jj]);
			if (jj != m-1)
				printf(", ");
		}
		printf(" ) \n\n ");
		
		fflush(stdout);
	}
	printf("member list \n");
	for (int ii = 0; ii < n; ii++)
	{
		printf(" %d, %d \n", ii, cluster_assignment_index[ii]);
	}
	cout << "--------------------------" << endl;
} 

/* 
	copy_assignment_array simply copies the assignment array (which point "belongs" to which cluster)
	so you can use it for the next iteration
		input - source and target
		output - void 
*/ 

void copy_assignment_array(int n, int *src, int *tgt)
{
	for (int ii = 0; ii < n; ii++)
		tgt[ii] = src[ii];
}

/*
	This keeps track of how many cluster assignments have changed.
*/

int assignment_change_count (int n, int a[], int b[])
{
	int change_count = 0;
	for (int ii = 0; ii < n; ii++)
		if (a[ii] != b[ii])
			change_count++;
	return change_count;
}

/******************************************************************************************************************
** K MEANS **

m = dimension of data
double *X = pointer to data
int n = number of elements
int k = number of clusters
double *cluster centroid = initial cluster centroids
int *cluster_assignment_final = output

The piece that you take from Kmeans to help the EM approximation is the double * cluster_centroid. 

*******************************************************************************************************************/
double * kmeans(int m, double *X, int n, int k)


{
	//holds the computed cluster_centroids to pass to EM later
    	double *cluster_centroid = new double[m*k];
	//for each data point, the distance to each centroid
	double *dist = new double[n*k];
	

	if (debug) printf("%p \n",dist);
	int *cluster_assignment_cur = new int[n];
	if (debug) printf("%p \n",cluster_assignment_cur);
	int *cluster_assignment_prev = new int[n];
	double *point_move_score = new double[n*k];

	if (!dist || !cluster_assignment_cur || !cluster_assignment_prev || !point_move_score)
		cout << "Error allocating arrays. \n" << endl;
		
	// give the initial cluster centroids some values
	
    	srand( time(NULL) );
    	for (int i = 0; i < k; i++)
	{
		int row = rand() % n;
		if (row >= n) 
			row = n-1;
		for (int j = 0; j < m; j++)
		{
        		cluster_centroid[i*m + j] = X[row*m + j];
		}
	}
	// initial setup
	all_distances(m, n, k, X, cluster_centroid, dist);
	choose_all_clusters_from_distances(m, n, k, X, dist, cluster_assignment_cur);
	copy_assignment_array(n, cluster_assignment_cur, cluster_assignment_prev);

	// batch update
	double prev_totD = -1;

	//printf("1: \n%lf\n", prev_totD);
	int batch_iteration = 0;
	
	while (batch_iteration < MAX_ITERATIONS)
	{
		if (debug) printf("batch iteration %d \n", batch_iteration);
		//printf("2: \n%lf\n", prev_totD);
		
		cluster_diag(m, n, k, X, cluster_assignment_cur, cluster_centroid);
		//printf("i've returned unscathed(?) from cluster diag \n");
		//printf("2.5: \n%lf\n", prev_totD);
		// update cluster centroids

		calc_cluster_centroids(m, n, k, X, cluster_assignment_cur, cluster_centroid);

		// deal with empty clusters
		// see if we've failed to improve
		
		//printf("3: \n%lf\n", prev_totD);

		double totD = calc_total_distance(m, n, k, X, cluster_centroid, cluster_assignment_cur);
		//printf("4: \n%lf\n", prev_totD);
		//printf("totD: %lf, prev_totD: %lf\n", totD, prev_totD);
/*
		if ((prev_totD >= 0) && (totD > prev_totD))
			// failed to improve - this solution is worse than the last one
			{
				// go back to the old assignments
				copy_assignment_array(n, cluster_assignment_prev, cluster_assignment_cur);
				// recalculate centroids
				calc_cluster_centroids(m, n, k, X, cluster_assignment_cur, cluster_centroid);
				if (debug) printf(" negative progress made on this step - iteration completed (%.2f) \n", totD-prev_totD);
				// done with this phase
				//break;
			}
*/
		// save previous step
		
		// smoosh points around to nearest cluster
		all_distances(m, n, k, X, cluster_centroid, dist);
		choose_all_clusters_from_distances(m, n, k, X, dist, cluster_assignment_cur);

		int change_count = assignment_change_count(n, cluster_assignment_cur, cluster_assignment_prev);
		if (debug) printf("batch iteration:%3d  dimension:%u  change count:%9d  totD:%16.2f totD-prev_totD:%17.2f\n", batch_iteration, 1, change_count, totD, totD-prev_totD);

		prev_totD = totD;
		batch_iteration++;

		// done with this phase if nothing has changed
		if (change_count == 0)
		{
			if (debug) cout << "No change made on this step - reached convergence. \n" << endl;
			break;
		}
		copy_assignment_array(n, cluster_assignment_cur, cluster_assignment_prev);

	}

	if (debug) cluster_diag(m, n, k, X, cluster_assignment_cur, cluster_centroid);

	if (debug) printf("%p \n",dist);
	delete[] dist; 
	if (debug) printf("%p \n",cluster_assignment_cur);
	delete[] cluster_assignment_cur;
	delete[] cluster_assignment_prev;
	delete[] point_move_score;

    	return cluster_centroid;
	
}



/*****************************************************************************************************************/
/** EM ALGORITHM **

mu_k = the K number of means, each with a vector of length M
sigma_k = the K number of covariance matrices, each of size M x M
P(k) = the fraction of all data points at some position x, where x is the m-dimensional position vector
p_nk = the K number of probabilities for each of the N data points
n = total number of data points
k = total number of multivariate Gaussians
x = M dimensional position vector
x_n = observed position
data_point = an individual data point
gaussian = a specific gaussian (distribution)
weight = the probability of finding a point at position x_n
density = multivariate gaussian density

*******************************************************************************************************************/
/*
	The E step of the EM algorithm is the piece that computes the likelihood of a particular model, and the 
	individual probabilites (p_nk) that each data point (n) belongs to a cluster (k). Be aware that the following
	calculations are done in the log space to avoid underflow.
	
	The first four arguments are the number of data points, dimensionality, number of clusters, and double* with the actual data. The
	last four arguments are (in the following order):
		a reference to the matrix that stores all the p_nk probabilities
		a vector of matrix pointers that holds the covariances for each cluster
		a reference to the matrix that stores all the means
		a reference to the matrix that stores all the Pks
	These last four get passed between estep and mstep - the types stay the same for the EM function, but the names
	then correspond to the structures the caller initialized. That's how you actually get the results from EM.
*/
double estep(int n, int m, int k, double *X,  Matrix &p_nk_matrix, vector<Matrix *> &sigma_matrix, Matrix &mu_matrix, Matrix &Pk_matrix)
{
	
	if (debug) cout << "p_nk matrix";
	if (debug) p_nk_matrix.print();
	if (debug) cout << "data";

	//initialize likelihood
	double likelihood = 0;
	
	//initialize variables
	int pi = 3.141592653;
	int data_point;
	int gaussian;
	int count = 0;
	for (data_point = 0; data_point < n; data_point++)
	{
		if (debug) cout << "1:beginning iteration " << data_point << " of " << n << endl;
		//initialize the x matrix, which holds the data passed in from double*X
		if (debug) printf("i am initing matrix iteration %d \n", count);
		count++;
		fflush(stdout);
		Matrix x(1,m);
		
		//initialize the P_xn to zero to start
		double P_xn = 0;

		for (int dim = 0; dim < m; dim++)
		{	
			//for(data_point = 0; data_point < n; data_point++)
			//{
				if (debug) cout << "trying to assign data " << X[m*dim + data_point] << " to location " << dim << " by " << data_point << endl;
				//put the data stored in the double* in the x matrix you just created
				x.update(X[m*data_point + dim],0,dim);
			//}
		}

		//Matrix x_row(1,m);
		//for (data_point = 0; data_point < n; data_point++)
		//{
			//for (int dim = 0; dim < m; dim++)
			//{
				//x_row.update(x.getValue(data_point,dim),0,dim);
			//}
		//}
		//if (debug) x_row.print();
		//here's where the estep calculation begins:
			//initialize the weight, z max and log densities	
		double weight_d;
		//z max is the maximum cluster weighted density for the data point under any gaussian
		double z_max = 0;
		double log_densities[k];

		//fill in the mu matrix
		for (gaussian = 0; gaussian < k; gaussian++)
		{
			Matrix mu_matrix_row(1,m);
			
			for (int dim = 0; dim < m; dim++)
			{
				double temp = mu_matrix.getValue(gaussian,dim);
				mu_matrix_row.update(temp,0,dim);
			}
	
			if (debug) cout << "2:beginning iteration 1 of n \n" << endl;
			//integer for debugging purposes
			int currStep = 0;
			if (debug) cout << currStep << endl; currStep++;
			if (debug) cout << "x row count is " << x.rowCount() << endl;
			if (debug) cout << "x col count is " << x.colCount() << endl;
			if (debug) cout << "mu_matrix row count is " << mu_matrix.rowCount() << endl;
			if (debug) cout << "mu_matrix col count is " << mu_matrix.colCount() << endl;
			if (debug) cout << "data" << endl;
			if (debug) cout << "x" << endl;
			if (debug) x.print();
			if (debug) cout << "mu matrix" << endl;
			if (debug) mu_matrix.print();

			/***** BEGIN eq 16.1.8 in NR3 *****/

			//x - mu
			Matrix& difference_row = x.subtract(mu_matrix_row);
			if (debug) difference_row.print();

			//sigma^-1
			if (debug) cout << currStep << endl; currStep++;

			Matrix &sigma_inv = sigma_matrix[gaussian]->inv();
			
			if (debug) printf("sigma_inv \n");
			if (debug) sigma_inv.print();
			//det(sigma)
			double determinant = sigma_matrix[gaussian]->det();
			
			//make a column representation of the difference in preparation for matrix multiplication
			Matrix difference_column(m,1);
			
			for (int i = 0; i < m; i++)
			{
				difference_column.update(difference_row.getValue(0,i),i,0);
			}

			Matrix term1 = sigma_inv.dot(difference_column);
			if (debug) printf("difference_column \n");
			if (debug) difference_column.print();
			if (debug) printf("term1 \n");
			if (debug) term1.print();
			//(x - mu) * sigma^-1
			
			if (debug) cout << "multiplication" << endl; 
			
			if (debug) cout << currStep << endl; currStep++;
			if (debug) cout << "yet another multiplication" << endl;

			//(x - mu) * sigma^-1 * (x - mu)
			Matrix &term2 = difference_row.dot(term1);
			if (debug) printf("term2 \n");
			if (debug) term2.print();

			if (debug) cout << currStep << endl; currStep++;
			double term2_d = 1;
			//pull out the double from the above matrix multiplication
			//for (data_point = 0; data_point < n; data_point++)
			//{
				//for (gaussian = 0; gaussian < k; gaussian++)
				//{
			term2_d = term2.getValue(0,0);
				//}
			//}
		

			//exp
			double log_unnorm_density = (-.5 * term2_d);


			double term3 = pow(2*pi, 0.5 * m);

			double term4 = pow(determinant, .5);
			// log norm factor is the normalization constant for the density functions
			double log_norm_factor = log(term3 * term4);

			//calculate multivariate gaussian density
			// log density is the log of the density function for the kth gaussian evaluated on the nth data point
			double log_density = log_unnorm_density - log_norm_factor;

			if (debug) printf("Pk_matrix \n");
			if (debug) Pk_matrix.print();
			//log sum exp trick
			// temp1 is the cluster weight for the current gaussian
			double temp1 = Pk_matrix.getValue(0,gaussian);
			if (debug) printf("temp 1 is %lf \n", Pk_matrix.getValue(0,gaussian));
			//temp2 is the log of the cluster weight for the current gaussian
			double temp2 = log(temp1);
			if (debug) printf("temp2 is %lf \n", log(temp1));
			//current z is the log of the density function times the cluster weight
			double current_z = temp2 + log_density;

			if (gaussian == 0 || current_z > z_max) z_max = current_z;
			
			//log_densities[gaussian] = log(Pk_matrix.getValue(0,gaussian)) + (log_density);
		
			/***** END 16.1.8 in NR3 *****/

			//calculate p_nk = density * Pk / weight
			p_nk_matrix.update(current_z,data_point,gaussian);
			if (debug) printf("p_nk_matrix \n");
			//if (debug) p_nk_matrix.print();
			
		//} 

		//P_xn calculation in log space
		//for(gaussian = 0; gaussian < k; gaussian++)
		//{
			
		//}


		// re-normalize per-gaussian point densities
		//for (gaussian = 0; gaussian < k; gaussian++)
		//{
			//if (debug) cout << "p_nk_matrix.getValue(data_point,gaussian) is " << p_nk_matrix.getValue(data_point,gaussian) << endl;
			//if (debug) cout << "that -log(P_xn) is " << p_nk_matrix.getValue(data_point,gaussian) - log(P_xn) << endl;

			
			delete &sigma_inv;			
		} // end gaussian
	
		for (gaussian = 0; gaussian < k; gaussian++)
		{
			P_xn += exp(p_nk_matrix.getValue(data_point, gaussian) - z_max);
		}
		if (debug) cout << "P_xn is " << P_xn << endl;
		//log of total density for data point
		double tempa = log(P_xn);
		if (debug) cout << "log Pxn is " << log(P_xn) << endl;
		double log_P_xn = tempa + z_max;
		if (debug) cout << "log Pxn plus z_max is " << log_P_xn << endl;
		for (gaussian = 0; gaussian < k; gaussian++)
		{
			//normalize the probabilities per cluster for data point
			p_nk_matrix.update(p_nk_matrix.getValue(data_point,gaussian)-log_P_xn,data_point,gaussian);
		}
		

		//calculate the likelihood of this model
		likelihood += log_P_xn;
		if (debug) cout << "The likelihood for this iteration is " << likelihood << endl;
		
	} // end data+point
	return likelihood;
}

/*
	The M step is where the mu's, sigma's and Pk's are adjusted and re-calculated. This function takes the same arguments of estep.
*/

bool mstep(int n, int m, int k, double *X, Matrix &p_nk_matrix, vector<Matrix *> &sigma_matrix, Matrix &mu_matrix, Matrix &Pk_matrix)
{
	
	
	//if (debug) printf("mu_hat \n");
	//if (debug) mu_hat.print();
	Matrix Pk_hat(1,k);

	for (int gaussian = 0; gaussian < k; gaussian++)
	{	
		//initialize the matrices that hold the mstep approximations
		Matrix sigma_hat(m,m);
		Matrix mu_hat(1,m);

		//initialize the normalization factor - this will be the sum of the densities for each data point for the current gaussian
		double norm_factor = 0;

		//initialize the array of doubles of length m that represents the data
		double x[m];
		if (debug) cout << "double x[m] " << x << endl;
		

		//do the mu calculation point by point
		for (int data_point = 0; data_point < n; data_point++)
		{
			
			for (int dim = 0; dim < m; dim++)
			{
				x[dim] = X[m*data_point + dim]*exp(p_nk_matrix.getValue(data_point,gaussian));
			}

			//sum up all the individual mu calculations
			if (debug) printf("mu hat addition \n");

			mu_hat.add(x, m, 0);
			

			//TODO: see if we need to deal w/ underflow here

			//calculate the normalization factor
			if (debug) cout << "pnk value for norm factor calc is " << p_nk_matrix.getValue(data_point,gaussian) << endl;
			norm_factor += exp(p_nk_matrix.getValue(data_point,gaussian));

			if (debug) cout << "norm factor is " << norm_factor << endl;
		}

		//fill in the mu hat matrix with your new mu calculations, adjusted by the normalization factor
		for (int dim = 0; dim < m; dim++)
		{
			mu_hat.update(mu_hat.getValue(0,dim)/norm_factor,0,dim);
		}

		//calculate the new covariances
		for (int data_point = 0; data_point < n; data_point++)
		{
			//fill in x vector for this data_point
			for (int dim = 0; dim < m; dim++)
			{
				x[dim] = X[m*data_point + dim];
			}
			//initialize the x_m matrix
			Matrix x_m(1,m);

			//fill it
			//if (debug) printf("x_m addition \n");
			x_m.add(x,m,0);
	
			//row representation of x - mu for matrix multiplication			
			Matrix difference_row = x_m.subtract(mu_hat);

			//column representation of x - mu for matrix multiplication
			Matrix difference_column(m,1);
			if (debug) printf("difference_column \n");
			if (debug) difference_column.print();
			for (int i = 0; i < m; i++)
			{
				difference_column.update(difference_row.getValue(0,i),i,0);
			}
			
			//magic tensor product calculation
			for (int i = 0; i < m; i++)
			{
				for (int j = 0; j < m; j++)
				{
					double temp1 = difference_row.getValue(0,i) * difference_column.getValue(j,0)*exp(p_nk_matrix.getValue(data_point,gaussian));
				
					sigma_hat.update(sigma_hat.getValue(i,j) + temp1, i, j);

				}
			}
		}//end data point

		//rest of the sigma calculation, adjusted by the normalization factor
		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < m; j++)
			{
				sigma_hat.update(sigma_hat.getValue(i,j)/norm_factor,i,j);
			}
		}
		double determinant = sigma_matrix[gaussian]->det();
		if (determinant < 0) return false;
		//calculate the new Pk's, also adjusted by the normalization factor
		//for (int data_point = 0; data_point < n; data_point++)
		//{
			//double temp1 = 1.0 / data_point;
			//double temp2 = temp1 * norm_factor;
			
		Pk_hat.update(norm_factor/n,0,gaussian);
			
		//}
		if (debug) cout << "pk hat matrix" << endl;
		if (debug) Pk_hat.print();
		//assign sigma_hat to sigma_matrix[gaussian]

		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < m; j++)
			{
				sigma_matrix[gaussian]->update(sigma_hat.getValue(i,j), i, j);
			}
		}

		//assign mu_hat to mu_matrix
		//for (int i = 0; i < k; i++)
		//{
		for (int dim = 0; dim < m; dim++)
		{
			mu_matrix.update(mu_hat.getValue(0,dim),gaussian,dim);

		}
		//}
		

		//assign Pk_hat to Pk_matrix
		//for (int i = 0; i < m; i++)
		//{
		Pk_matrix.update(Pk_hat.getValue(0,gaussian),0,gaussian);
		//}

	} //end gaussian
	double sum = 0;
	for (int i = 0; i < k; i++)
	{	
		sum += Pk_matrix.getValue(0,i);
	}

	for (int i = 0; i < k; i++)
	{	
		Pk_matrix.update(Pk_matrix.getValue(0,i)/sum,0,i);
	}
	return true;
}

/*
	Here is where the whole EM algorithm comes together. First, you set up the initial conditions (noting that now, instead
	of the arguments estep and mstep pass to each other, we're now passing in the structures your caller initialized). Then you
	call kmeans to get initial cluster centroid (mu) guesses, and then iterate between the estep and mstep until convergence.
	EM doesn't actually return anything - instead, you're filling the containers you created when you called the function, and 
	when EM has "finished," you can use/print out your final approximations independent of the function.
*/
void EM(int n, int m, int k, double *X, vector<Matrix*> &sigma_matrix, Matrix &mu_matrix, Matrix &Pks)

{
	int iterations;
	//epsilon is the convergence criteria - the smaller epsilon, the narrower the convergence
	double epsilon = .001;
	int counter = 0;
	//initialize the p_nk matrix
	Matrix p_nk_matrix(n,k);

	if (debug) cout << "n is " << n;
	if (debug) cout << "\nm is: " << m << endl;
	if (debug) cout << "matrix data made \n";
	

	//initialize likelihoods with dummy fillers
	double new_likelihood = 0;	
	double old_likelihood = 0;
	
	//take the cluster centroids from kmeans as initial mus 
	if (debug) printf("i will call kmeans \n");
	fflush(stdout);
	double *kmeans_mu = kmeans(m, X, n, k); 
	
	if (debug) printf("i called kmeans \n");
	fflush(stdout);
	if (kmeans_mu == 0)
		return;

	//initialize array of identity covariance matrices, 1 per k
	for(int gaussian = 0; gaussian < k; gaussian++)
	{
		for (int j = 0; j < m; j++)
		{
			sigma_matrix[gaussian]->update(1.0,j,j);
			
		}
	}

	//initialize matrix of mus from kmeans the first time
	for (int i = 0; i < k; i++)
	{
		for (int j = 0; j < m; j++)
		{
			mu_matrix.update(kmeans_mu[i*m + j],i,j);
			if (debug) cout << "assigning, k is " << k << ", kmeans_mu[i] is " << kmeans_mu[i] << " at dimensions i (" << i << ") and j (" << j << ")\n";
		}
	}

	//initialize Pks
	for (int gaussian = 0; gaussian < k; gaussian++)
	{
		double term1 = 1.0 / k;
		Pks.update(term1,0,gaussian);
		
	}
	if (debug) printf("i initialized matrices successfully \n");
	fflush (stdout);

	new_likelihood = estep(n, m, k, X, p_nk_matrix, sigma_matrix, mu_matrix, Pks);
	if (debug) cout << "new likelihood is " << new_likelihood << endl;
	//main loop of EM - this is where the magic happens!
	while (fabs(new_likelihood - old_likelihood) > epsilon)
	{
		if (debug) cout << "HERE " << counter << endl;
		if (debug) cout << "new likelihood is " << new_likelihood << endl;
		
		if (debug) cout << "old likelihood is " << old_likelihood << endl;
		old_likelihood = new_likelihood;
		if ( mstep(n, m, k, X, p_nk_matrix, sigma_matrix, mu_matrix, Pks) == false) 
		{
			cout << "Found singular matrix - terminated." << endl;
			break;
		}
		
		new_likelihood = estep(n, m, k, X, p_nk_matrix, sigma_matrix, mu_matrix, Pks);
		
		//if (debug) printf("i finished estep \n");
		fflush (stdout);

		
		//if (debug) printf("i finished mstep \n");
		
		

		fflush(stdout);
		if (debug) cout << " " << counter << " iteration's pnk matrix is " << endl;
		if (debug) p_nk_matrix.print();
		if (debug) cout << " " << counter << " iteration's mu matrix is " << endl;
		if (debug) mu_matrix.print();
		if (debug) cout << " " << counter << " iteration's sigma matrix is " << endl;
		for (int gaussian = 0; gaussian < k; gaussian++)
		{
			for (int j = 0; j < m; j++)
			{
				if (debug) sigma_matrix[gaussian]->print();
			}
		}
		if (debug) cout << " " << counter << " iteration's Pk matrix is " << endl;
		if (debug) Pks.print();

		counter++;
	}
	delete[] kmeans_mu;
	if (debug) cout << "last new likelihood is " << new_likelihood << endl;	
	if (debug) cout << "last old likelihood is " << old_likelihood << endl;
	cout << "Total number of iterations completed by the EM Algorithm is \n" << counter << endl;
}
