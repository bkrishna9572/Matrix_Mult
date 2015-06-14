
#include<iostream>
#include<fstream>
#include<string>
#include<chrono>
#include <amp.h>
using namespace std;
using namespace concurrency;
int a[10][10], b[10][10], c[10][10], j = 0, i = 0, k = 0,bMatrix[100],aMatrix[100];
class Matrix{
	
public:
	 std::chrono::time_point<std::chrono::high_resolution_clock> getTime(){
		 return std::chrono::high_resolution_clock::now();
	}
	void readInput(){
		string line;
		ifstream ipFile("C:\\users\\Krishna\\Desktop\\numbers.txt");
		j = 0;
		while (getline(ipFile, line)){
			if (j < 10){
				a[i][j] = stoi(line);
				j++;
			}
			else
			{
				j = 0;
				a[++i][j++] = stoi(line);
			}
		}
		for (i = 0; i < 10; i++)
			for (j = 0; j < 10; j++)
				b[i][j] = a[i][j];
	}

	void print(int x[10][10]){
		for (i = 0; i < 10; i++){
			for (j = 0; j < 10; j++)
				cout << x[i][j] << "\t";
			cout << "\n";
		}
	}

	void add(){
		for (i = 0; i < 10; i++){
			for (j = 0; j < 10; j++)
				cout << a[i][j] + a[i][j] << "\t";
			cout << "\n";

		}
		
	}

	void multiply(){
		
		for (i = 0; i < 10; i++)
			for (j = 0; j < 10; j++)
				for (k = 0; k < 10; k++)
					c[i][j] += a[i][k] * b[k][j];
		
		for (i = 0; i < 10; i++){
			for (j = 0; j < 10; j++)
				cout << c[i][j]<< "\t";
			cout << "\n";

		}
	}
	
};

class Amp_file{

public:
	
	void readInput(){
		string line;
		ifstream ipFile("C:\\users\\Krishna\\Desktop\\numbers.txt");
		j = 0;
		while (getline(ipFile, line)){
			bMatrix[j] = stoi(line);
		}
		for (int i = 0; i < 100;i++)
		aMatrix[i] = bMatrix[i];
	}
	void MultiplyWithTiling()
	{
		static const int TS = 2;

		// The raw data.
		int aMatrix[] = { 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8 };
		int bMatrix[] = { 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8 };
		int productMatrix[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		// Create the array_view objects.
		array_view<int, 2> a(4, 4, aMatrix);
		array_view<int, 2> b(4, 4, bMatrix);
		array_view<int, 2> product(4, 4, productMatrix);
		
		// Call parallel_for_each by using  2x2 tiles.
		parallel_for_each(product.extent.tile< TS, TS >(),
			[=](tiled_index< TS, TS> t_idx) restrict(amp)
		{
			// Get the location of the thread relative to the tile (row, col) and the entire array_view (rowGlobal, colGlobal).
			int row = t_idx.local[0];
			int col = t_idx.local[1];
			int rowGlobal = t_idx.global[0];
			int colGlobal = t_idx.global[1];
			int sum = 0;

			// Given a 4x4 matrix and a 2x2 tile size, this loop executes twice for each thread.
			// For the first tile and the first loop, it copies a into locA and e into locB.
			// For the first tile and the second loop, it copies b into locA and g into locB.
			for (int i = 0; i < 4; i += TS) {
				tile_static int locA[TS][TS];
				tile_static int locB[TS][TS];
				locA[row][col] = a(rowGlobal, col + i);
				locB[row][col] = b(row + i, colGlobal);
				// The threads in the tile all wait here until locA and locB are filled.
				t_idx.barrier.wait();


				// Return the product for the thread. The sum is retained across
				// both iterations of the loop, in effect adding the two products
				// together, for example, a*e.
				for (int k = 0; k < TS; k++) {
					sum += locA[row][k] * locB[k][col];
				}

				// All threads must wait until the sums are calculated. If any threads
				// moved ahead, the values in locA and locB would change.      
				t_idx.barrier.wait();
				// Now go on to the next iteration of the loop.          
			}

			// After both iterations of the loop, copy the sum to the product variable by using the global location.
			product[t_idx.global] = sum;
		});

		// Copy the contents of product back to the productMatrix variable.
		product.synchronize();
		
		for (int row = 0; row < 4; row++) {
			for (int col = 0; col < 4; col++) {
				// The results are available from both the product and productMatrix variables.
				//std::cout << productMatrix[row*3 + col] << "  ";
				std::cout << product(row, col) << "  ";
			}
			std::cout << "\n";
		}
	}
};


int main()
{
	cout << "Enter 1 for sequential operation and 2 for parallel operation";
	int choice;
	cin >> choice;
	std::chrono::time_point<std::chrono::high_resolution_clock> add_start, add_end,mul_start,mul_end,par_start,par_end;
	std::chrono::microseconds dur,mul_dur,par_dur;
	switch (choice)
	{
	case 1:
		Matrix matrix;
		matrix.readInput();
		add_start=matrix.getTime();
		matrix.add();
		add_end = matrix.getTime();
		dur = std::chrono::duration_cast<std::chrono::microseconds>(add_end - add_start);
		cout << "\naddition time = " << dur.count()<<"\n";
		mul_start = matrix.getTime();
		matrix.multiply();
		mul_end = matrix.getTime();
		mul_dur = std::chrono::duration_cast<std::chrono::microseconds>(mul_end - mul_start);
		cout << "\nmultiplication time = " << mul_dur.count() << "\n";
		break;
	case 2:
		Amp_file af;
		af.readInput();
		par_start = matrix.getTime();
		af.MultiplyWithTiling();
		par_end = matrix.getTime();
		par_dur = std::chrono::duration_cast<std::chrono::microseconds>(par_end - par_start);
		cout << "\nparallel multiplication time = " << par_dur.count();
		break;
	default:
		break;
	}
	return 0;
}

