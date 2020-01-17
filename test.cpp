#include <iostream>
#include <fstream>
using namespace std;

#define N0 20
#define PAGE_SIZE 65536
#define dot_num 51
#define rid_num (N0*2)
#define ball_num_f 60
#define dimension 50
#define ball_Node_num (int)(PAGE_SIZE/((root->d+1)*N0*(slot_num)))
#define node_slot_size ball_num_f

#include "BallTree.cpp"
#include "Utility.cpp"

const char rawDataPath[256] = "Netflix/src/dataset.txt";
const char queryPath[256] = "Netflix/src/query.txt";
const char indexPath[256] = "Netflix/index/index.bin";
const char dataPath[256] = "Netflix/data.bin";
const char outputPath[256] = "Netflix/dst/answer.txt";

int main() {
	const int n = 17770;
	const int d = 50;
	const int qn = 1000;
	
	// build a tree and store it to disk
	BallTree tree1;

	float** data = 0;
	float** query = 0; 

	read_data(n, d, data, rawDataPath);
	
	tree1.buildTree(n, d, data);
	cout << "Building completed!" << endl;
	
	tree1.storeTree(indexPath, dataPath);
	cout << "Storing completed!" << endl;
	
	// restore a tree from disk and read into memory
	BallTree tree2;
	tree2.restoreTree(indexPath);
	cout << "Restoring completed!" << endl;
	
	if (!read_data(qn, d, query, queryPath));
		FILE* fout = fopen(outputPath, "w");
	
	if (!fout) {
		printf("Can't open %s!\n", outputPath);
		return 1;
	}
	
	for (int i = 0; i < qn; i++) {
		int index = tree2.mipSearch(d, query[i], indexPath, dataPath);
		fprintf(fout, "%d\n", index);
	}
	
	fclose(fout);
	cout << "Search completed!" << endl;
	
	// recycle memory
	for (int i = 0; i < n; i++) delete[] data[i];
	for (int i = 0; i < qn; i++) delete[] query[i];
	return 0;
}

