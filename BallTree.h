// #include "BallTreeNode.h"
#ifndef __BALL_TREE_H
#define __BALL_TREE_H

struct rid {
	public:
	int page;
	int slot;
	rid(int _page = -1,int _slot = -1) {
		page = _page;
		slot = _slot;
	}
};

struct Query {
	float index;
	float maxProduct;
	Query(float in = -1, float max = -1) {
		index = in;
		maxProduct = max;
	}
	void set(float in, float max) {
		index = in;
		maxProduct = max;
	}
};

struct BallTreeNode {
	float **data;
	float *center;
	float radius;
	int D;
	int d;
	int node_num;
	BallTreeNode* left;
	BallTreeNode* right;
	BallTreeNode();
	~BallTreeNode();
	BallTreeNode(float **&_data, int _D, int _d);
	void getCenter();
	void getRadius();

	rid left_rid, right_rid, list_rid;
};

class BallTree {
	private:
		BallTreeNode * root;
	public:
		BallTree();
		~BallTree();

		BallTreeNode* getNode() {
			return root;
		}
		bool buildTree(int n, int d, float**&data);
		bool MakeBallTree(BallTreeNode*&ball, int n, int d, float **&data);
		bool MakeBallTreeSplit(float **&data, int n, int d, int &a, int &b);
		void print();

		bool storeTree(const char* indexPath, const char* dataPath);
		bool restoreTree(const char* indexPath);

		int mipSearch(int d, float* query, const char* indexPath, const char* dataPath);
};

#endif