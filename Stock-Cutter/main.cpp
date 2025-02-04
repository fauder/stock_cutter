#include "stdafx.h"

#include "csv_io.h"
#include "gen_pttn.h"
#include "solver.h"
using namespace std;

/*
	Author: Thomas Loke
	Development started: 4th February 2016
	Acknowledgements: Uses open-source linear programming libraries from http://www.coin-or.org/ - specifically CBC and its dependencies. Distributed under the Eclipse Public License.
*/

/*
	...Cause God is in control...
*/

void Exit()
{
#ifdef _DEBUG
	string userInput;
	cout << "Type something and hit ENTER to exit... ";
	cin >> userInput;
#endif // _DEBUG
	exit( EXIT_FAILURE );
}

int main(int argc, char* argv[])
{
	if( argc < 2 )
	{
		cerr << "Not enough cmd line arguments provided!\n";
		Exit();
	}

#ifdef _DEBUG
	for( auto i = 0; i < argc; i++ )
		cout << "Cmd line arg " << i << ": " << argv[ i ] << "\n";
#endif // _DEBUG


	const auto fileName = string( argv[ 1 ] );
	if( !ifstream( fileName + ".csv" ) )
	{
		cerr << "Specified csv file (without extension): \"" << fileName << "\" could not be opened!\n";
		Exit();
	}

	int i;
	bool uniform_weight = true;
	// Load input file
	int problem_ct = 0;
	csv_io csv;
	/*string root;
	cout << "Please enter the name (no spaces allowed) of the .csv file that you wish to examine (without the .csv extension): ";
	cin >> root;
	csv.setfilename(root);*/
	csv.setfilename( fileName );
	while (csv.getproblem()) {
		++problem_ct;
		int N, M, L;
		M = csv.M;
		L = csv.L;
		cout << "\nProblem #" << problem_ct << " loaded successfully!" << endl;
		cout << "Performing consistency checks..." << endl;
		if (L <= 0) {
			cerr << "Inconsistency: Length of stock material " << L << " must be a positive integer." << endl;
			Exit();
		}
		if (M == 0) {
			cerr << "Inconsistency: Problem has no orders associated with it." << endl;
			Exit();
		}
		for (i = 0; i < M; ++i) {
			if (csv.Lv[i] <= 0 || csv.Lv[i] > L) {
				cerr << "Inconsistency: Order width " << csv.Lv[i] << " does not satisfy 0 < value <= " << L << "." << endl;
				Exit();
			}
			if (csv.qv[i] <= 0) {
				cerr << "Inconsistency: Order quantity " << csv.qv[i] << " must be a positive integer." << endl;
				Exit();
			}
		}
		cout << "Consistency checks passed!\n" << endl;

		cout << "Generating maximal patterns..." << endl;
		// Sort Lv, qv first
		csv.sort();
		// Initialize generator
		gen_pttn gen_obj;
		gen_obj.Lv = csv.Lv;
		gen_obj.M = M;
		gen_obj.L = L;
		// Run generator
		gen_obj.run();
		N = gen_obj.S.size();
		cout << "Maximal patterns generated! Quantity: " << N << "\n" << endl;
		
		cout << "Solving the associated IP..." << endl;
		// Initialize solver
		solver sol_obj;
		sol_obj.setdim(N, M);
		sol_obj.setvars(gen_obj.S, csv.qv, uniform_weight);
		// Run solver
		vector<int> xv;
		double objv;
		sol_obj.run(xv, objv);
		cout << "IP has been solved! Final objective function value: " << objv << "\n" << endl;
		
		cout << "Writing solution to output file..." << endl;
		// Write solution to output .csv file
		csv.writesolution(xv, gen_obj.S);
		cout << "Solution written to file!" << endl;
		// Reset variables
		csv.reset();
	}
	cout << "\nProgram is done executing! Have fun!! :)" << endl;
#ifdef _DEBUG
	string userInput;
	cout << "Type something and hit ENTER to exit... ";
	cin >> userInput;
#endif // _DEBUG
	return 0;
}