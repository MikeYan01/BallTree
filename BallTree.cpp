#include "BallTree.h"
#include "Utility.h"
#include <iostream>
#include <cmath>
using namespace std;

BallTreeNode::BallTreeNode() {
	data = 0;
	center = 0;
	radius = 0;
	D = 0;
	d = 0;
	left = 0;
	right = 0;
	list_rid.page = -1;
	list_rid.slot = -1;
}

BallTreeNode::~BallTreeNode() {
	delete[] data;
}

BallTreeNode::BallTreeNode(float **&_data, int _D, int _d) {
	data = new float*[_D];

	for (int i = 0; i < _D; i++) {
		data[i] = new float[_d+1];
		for (int j = 0; j < _d+1; j++)
			data[i][j] = _data[i][j];
	}
	D = _D;
	d = _d;
	left = 0;
	right = 0;
	getCenter();
	getRadius();
}

void BallTreeNode::getCenter() {
	center = new float[d];
	for (int i = 0; i < d; i++) center[i] = 0;
	for (int i = 0; i < d; i++) 
		for (int j = 0; j < D; j++)
			center[i] += data[j][i];
		
	for (int i = 0; i < d; i++) center[i] /= D;
}

void BallTreeNode::getRadius() {
	float max = 0;
	float temp = 0;
	for (int i = 0; i < D; i++) {
		for (int j = 0; j < d; j++)
			temp += (data[i][j] - center[j])*(data[i][j] - center[j]);

		if (temp > max) max = temp;
		temp = 0;
	}
	radius = sqrt(max);
}

float distance(float *a, float *b, int _d) {
	float res = 0.0;
	for (int i = 0; i < _d; i++)
		res += (a[i] - b[i])*(a[i] - b[i]);
	return sqrt(res);
}

BallTree::BallTree() {
	root = 0;
}

BallTree::~BallTree() {
	delete root;
}

bool BallTree::buildTree(int n, int d, float **&data) {
	MakeBallTree(root, n, d, data);
	return true;
}

bool BallTree::MakeBallTree(BallTreeNode *& node, int n, int d, float **&data) {
	node = new BallTreeNode(data, n, d);
	if (n < N0) return true;

	int Lnum = 0;
	int Rnum = 0;
	int Aindex = 0;
	int Bindex = 0;

	// make ball tree split
	int x = 0;
	float max = 0;
	for (int i = 0; i < n; i++) {
		float res = distance(data[i], data[x], d);
		if (res > max) {
			max = res;
			Aindex = i;
		}
	}

	max = 0;
	for (int i = 0; i < n; i++) {
		float res = distance(data[i], data[Aindex], d);
		if (res > max) {
			max = res;
			Bindex = i;
		}
	}

	float **left;
	float **right;

	left = new float*[n];
	right = new float*[n];
	for (int i = 0; i < n; ++i) {
		left[i] = new float[d+1];
		right[i] = new float[d+1];
	}

	for (int i = 0; i < n; ++i) {
		float distanceA = distance(data[i], data[Aindex], d);
		float distanceB = distance(data[i], data[Bindex], d);

		if (distanceA < distanceB) {
			for (int j = 0; j < d+1; j++) left[Lnum][j] = data[i][j];
			Lnum++;
		}
		else {
			for (int j = 0; j < d+1; j++) right[Rnum][j] = data[i][j];
			Rnum++;
		}

	}
	MakeBallTree(node->left, Lnum, d, left);
	MakeBallTree(node->right, Rnum, d, right);
	return true;
}

void preOrderSetNum(BallTreeNode* &root, int & num){
	if (root == 0) return;
	root->node_num = num;
	num++;

	preOrderSetNum(root->left, num);
	preOrderSetNum(root->right, num);
}

void setPageSlot(BallTreeNode* &root) {
	if (root -> left == 0 && root -> right == 0) {
		root->left_rid.page = -1;
		root->left_rid.slot = -1;
		root->right_rid.page = -1;
		root->right_rid.slot = -1;
		return;
	}

	if (root->left != 0) {
		root->left_rid.page = ((int)((root->left) -> node_num)) / (int)(PAGE_SIZE / ((root->d + 10) * sizeof(float)));
		root->left_rid.slot = ((int)((root->left) -> node_num)) % (int)(PAGE_SIZE / ((root->d + 10) * sizeof(float)));
	}

	if(root->right != 0) {
		root->right_rid.page = ((int)((root->right) -> node_num)) / (int)(PAGE_SIZE / ((root->d + 10) * sizeof(float)));
		root->right_rid.slot = ((int)((root->right) -> node_num)) % (int)(PAGE_SIZE / ((root->d + 10) * sizeof(float)));
	}

	setPageSlot(root -> left);
	setPageSlot(root -> right);
}

void findMax(BallTreeNode* &root, int &max) {
	if (root == 0) return;
	if (root->node_num>max) max = root->node_num;
	findMax(root->left, max); 
	findMax(root->right, max); 
}

void writeOnePage(BallTreeNode* &root, float cache[PAGE_SIZE / sizeof(float)], int &slot_num, int &page_num) {
	if (root == 0) return;
	if (((int)((root->node_num)) / (int)(PAGE_SIZE / ((root->d + 10) * sizeof(float)))) == page_num) {
		slot_num = (int)(((root->node_num)) % (int)(PAGE_SIZE / ((root->d + 10) * sizeof(float))));
		
		int k;
		for (k = 0 ; k < root -> d; k++) cache[slot_num*node_slot_size+k] = root->center[k]; 
		cache[slot_num*node_slot_size+0+k] = root->radius; 
		cache[slot_num*node_slot_size+1+k] = root->D; 
		cache[slot_num*node_slot_size+2+k] = root->d; 
		cache[slot_num*node_slot_size+3+k] = root->node_num; 
		cache[slot_num*node_slot_size+4+k] = root->left_rid.page; 
		cache[slot_num*node_slot_size+5+k] = root->left_rid.slot; 
		cache[slot_num*node_slot_size+6+k] = root->right_rid.page; 
		cache[slot_num*node_slot_size+7+k] = root->right_rid.slot; 
		cache[slot_num*node_slot_size+8+k] = root->list_rid.page;
		cache[slot_num*node_slot_size+9+k] = root->list_rid.slot; 	
	}
	writeOnePage(root->left, cache, slot_num, page_num);
	writeOnePage(root->right, cache, slot_num, page_num);
}

void writeIndex(BallTreeNode* &root, FILE* & index_bin) {
	// set node num
	int num = 0;
	preOrderSetNum(root, num);

	setPageSlot(root);

	float cache[PAGE_SIZE/sizeof(float)];
	for (int i = 0; i < PAGE_SIZE / sizeof(float); i++) cache[i] = 0;

	int max;
	findMax(root, max);

	int slot_num = 0;
	for (int i = 0; i <= max / (int)(PAGE_SIZE / ((root->d + 10) * sizeof(float))); i++) {
		slot_num = 0;
		writeOnePage(root, cache, slot_num, i);
		fwrite(cache, sizeof(float), PAGE_SIZE/sizeof(float), index_bin);
	}
}

void writeData(FILE* &data_bin, BallTreeNode* &root, int &page_num, int &slot_num, float cache[PAGE_SIZE/sizeof(float)]) {
	if (root == 0) return;
	
	root->list_rid.page= -1;
	root->list_rid.slot = -1;
	
	if ((root->left == 0 && root -> right == 0) || root->D <= N0) {
		if ((int)((root->d + 1) * N0 * (slot_num+1)) < PAGE_SIZE / sizeof(float)) {
			for (int i = 0; i < root -> D; i++)
				for (int j = 0; j < root -> d+1; j++) 
					cache[(root->d+1) * N0 * (slot_num)+ i * (root->d+1) + j] = root->data[i][j];

			root->list_rid.page = page_num;
			root->list_rid.slot = slot_num;
			slot_num++;
		} else {
			fwrite(cache, sizeof(float), PAGE_SIZE/sizeof(float), data_bin);
			slot_num = 0;
			page_num++;
			if ((int)((root->d + 1) * N0 * (slot_num+1)) < PAGE_SIZE / sizeof(float)) {
			for (int i = 0; i < root -> D; i++)
				for (int j = 0; j < root -> d+1; j++) 
					cache[(root->d+1) * N0 * (slot_num)+ i * (root->d+1) + j] = root->data[i][j];
				root->list_rid.page= page_num;
				root->list_rid.slot = slot_num;
				slot_num++;
			}
		}
	}
	writeData(data_bin, root->left, page_num, slot_num, cache);
	writeData(data_bin, root->right, page_num, slot_num, cache);
}

bool BallTree::storeTree(const char * indexPath, const char * dataPath) {	
	int page_num = 0;
	int slot_num = 0;
	float cache[PAGE_SIZE / sizeof(float)];
	for (int i = 0; i < PAGE_SIZE / sizeof(float); i++) cache[i] = 0;

	FILE * index_bin = fopen(indexPath, "wb"); 
	FILE * data_bin = fopen(dataPath, "wb");
	
	writeData(data_bin, root, page_num, slot_num, cache);
	fwrite(cache, sizeof(float), PAGE_SIZE/sizeof(float), data_bin);
	writeIndex(root,index_bin);

	fclose(data_bin);
	fclose(index_bin);
	return true;
}

void preOrderBuildTree(BallTreeNode* &root, FILE * &index_bin, float cache[PAGE_SIZE/sizeof(float)], const char* dataPath) {
	if (root->D > N0) {
		// construct left node
		root->left = new BallTreeNode;
		int pageoffset = root->left_rid.page * PAGE_SIZE;
		int slot = root->left_rid.slot * ball_num_f;
		
		fseek(index_bin, pageoffset, SEEK_SET);
		fread(cache, sizeof(float), PAGE_SIZE / sizeof(float), index_bin);
		
		root->left->center= new float[root->d]; 
		int k;
		for (k = 0; k < root -> d; k++) root->left->center[k] = cache[k+slot];
	
		root->left->radius = cache[k+0+slot];
		root->left->D = cache[k+1+slot];
		root->left->d = cache[k+2+slot];
		root->left->node_num = cache[k+3+slot];
		root->left->left_rid.page = cache[k+4+slot];
		root->left->left_rid.slot = cache[k+5+slot];
		root->left->right_rid.page = cache[k+6+slot];
		root->left->right_rid.slot = cache[k+7+slot];
		root->left->list_rid.page = cache[k+8+slot];
		root->left->list_rid.slot = cache[k+9+slot];
		
		// construct left node
		root->right = new BallTreeNode;
		pageoffset = root->right_rid.page * PAGE_SIZE;
		slot = root->right_rid.slot * ball_num_f;
		
		fseek(index_bin,pageoffset, SEEK_SET);
		fread(cache, sizeof(float), PAGE_SIZE / sizeof(float), index_bin);
		
		root->right->center= new float[root->d];
			
		for(k = 0; k < root->d; k++) root->right->center[k] = cache[k+slot];
		
		root->right->radius=cache[k+0+slot];
		root->right->D=cache[k+1+slot];
		root->right->d=cache[k+2+slot];
		root->right->node_num=cache[k+3+slot];
		root->right->left_rid.page=cache[k+4+slot];
		root->right->left_rid.slot=cache[k+5+slot];
		root->right->right_rid.page=cache[k+6+slot];
		root->right->right_rid.slot=cache[k+7+slot];
		root->right->list_rid.page =cache[k+8+slot];
		root->right->list_rid.slot=cache[k+9+slot];
	} else {
		root->data = new float* [root->D];
		int pageoffeset = 0;
		int slotoffeset = 0;
		pageoffeset = root->list_rid.page * PAGE_SIZE;
		slotoffeset = root->list_rid.slot * dot_num * N0;
		
		float cache[PAGE_SIZE/sizeof(float)];
		for (int i = 0; i < PAGE_SIZE / sizeof(float); i++) cache[i] = 0;

		FILE *data = fopen(dataPath, "rb");
		fseek(data, pageoffeset, SEEK_SET);
		fread(cache, sizeof(float), PAGE_SIZE/sizeof(float), data);

		for (int i = 0; i < root->D; i++) {
			root->data[i] = new float[root->d + 1];
			for (int j = 0; j < root->d + 1; j++)
				root->data[i][j] = cache[slotoffeset + i * dot_num + j];
		}
		fclose(data);
	}
}

bool BallTree::restoreTree(const char * indexPath) {
	FILE * index_bin = fopen(indexPath, "rb");

	float cache[PAGE_SIZE/sizeof(float)];
	for (int i = 0; i < PAGE_SIZE / sizeof(float); i++) cache[i] = 0;
	
	fread(cache, sizeof(float), PAGE_SIZE / sizeof(float), index_bin);

	BallTreeNode *head = new BallTreeNode;
	head->center= new float[dimension];

	int k;
	for (k = 0; k < dimension; k++) head->center[k] = cache[k];
	
	head->radius = cache[k+0];
	head->D = cache[k+1];
	head->d = cache[k+2];
	head->node_num = cache[k+3];
	head->left_rid.page = cache[k+4];
	head->left_rid.slot = cache[k+5];
	head->right_rid.page = cache[k+6];
	head->right_rid.slot = cache[k+7];
	head->list_rid.page = cache[k+8];
	head->list_rid.slot = cache[k+9];
	root = head;
	
	fclose(index_bin);
	return true;
}

float maxMIP(float * query, BallTreeNode*node, int d){
	float result = 0;

	// sum up inner product
	for (int i = 0; i < d; i++) result += query[i] * node->center[i];

	float q = 0;
	for (int i = 0; i < d; i++) q += query[i] * query[i];

	result += sqrt(q) * node->radius;
	return result;
} 

void TreeSearch(float* &query, BallTreeNode* &node, int d, Query& QueryResult, float cache[PAGE_SIZE/sizeof(float)], FILE * &index_bin, const char* dataPath) {
	preOrderBuildTree(node, index_bin, cache, dataPath);
	if (QueryResult.maxProduct < maxMIP(query,node,d)) {
		if (node -> D <= 20) {
			for (int i = 0; i < node->D && node->data != 0; i++) {
				float temp = 0;
				for (int j = 0; j < d; j++) temp += node -> data[i][j] * query[j];
				if (temp > QueryResult.maxProduct) {
					float index = node -> data[i][d];
					QueryResult.set(index, temp);
				}
			}
			return;
		}

		float leftbound = maxMIP(query, node->left, d);
		float rightbound = maxMIP(query, node->right, d);
		if (leftbound < rightbound) {
			TreeSearch(query, node->right, d, QueryResult, cache, index_bin, dataPath);
			TreeSearch(query, node->left, d, QueryResult, cache, index_bin, dataPath);
		} else {
			TreeSearch(query, node->left, d, QueryResult, cache, index_bin, dataPath);
			TreeSearch(query, node->right, d, QueryResult, cache, index_bin, dataPath);
		}
	}
}

int BallTree::mipSearch(int d, float * query, const char* indexPath, const char* dataPath) {
	FILE * index_bin = fopen(indexPath, "rb");
	float cache[PAGE_SIZE/sizeof(float)];							
	for (int i = 0; i < PAGE_SIZE / sizeof(float); i++) cache[i] = 0;

	Query QueryResult; 
	TreeSearch(query, root, d, QueryResult, cache, index_bin, dataPath);
	fclose(index_bin);
	return (int)QueryResult.index;
}